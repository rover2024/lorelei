// SPDX-License-Identifier: MIT

#include <array>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include <lorelei/TLCApi/Pass.h>
#include <lorelei/TLCApi/ProcSnippet.h>
#include <lorelei/TLCApi/DocumentContext.h>
#include <lorelei/ThunkInterface/PassTags.h>
#include <lorelei/ClangExtras/TypeUtils.h>

#include "Utils/PassCodeTemplates.h"

using namespace clang;

namespace lore::tool::TLC {

    namespace {

        std::string indent(int level) {
            return std::string(level * 4, ' ');
        }

        // CallbackTree - The callbacks reachable from one proc argument, as a tree.
        //
        // Each node is an aggregate: \c callbacks are its direct function-pointer members and
        // \c children are nested aggregate members to recurse into. \c reach records how a node is
        // reached from its parent (held by value, or behind a pointer that must be null-checked).
        // \c status distinguishes a subtree that carries callbacks from one that carries none, or one
        // whose shape cannot be marshalled (a callback behind a double pointer, in an array, ...).
        struct CallbackTree {
            enum class Reach { ByValue, ByPointer };
            enum class Status { HasCallbacks, NoCallbacks, Unsupported };

            Status status = Status::Unsupported;
            Reach reach = Reach::ByValue;
            std::map<std::string, QualType> callbacks; // member name -> callee function type
            std::map<std::string, CallbackTree> children;
            // Out-parameter callbacks: an argument of type `callback (**)(...)` that the callee fills
            // in. Only collected at the top (argument) level; the value is the callee function type.
            std::map<std::string, QualType> outCallbacks;

            CallbackTree() = default;
            explicit CallbackTree(Status status) : status(status) {
            }

            static CallbackTree unsupported() {
                return CallbackTree(Status::Unsupported);
            }
            static CallbackTree noCallbacks() {
                return CallbackTree(Status::NoCallbacks);
            }

            // True when \a T, fully walked through pointers, arrays and nested records, contains any
            // function pointer at all.
            static bool containsFunctionPointer(QualType T) {
                SmallVector<QualType> stack{T};
                std::set<std::string> visited;
                while (!stack.empty()) {
                    auto type = stack.pop_back_val().getCanonicalType();
                    while (type->isPointerType() || type->isArrayType()) {
                        type = type->isPointerType()
                                   ? type->getPointeeType()
                                   : type->getAsArrayTypeUnsafe()->getElementType();
                    }
                    type = type.getCanonicalType();

                    // Stop at recursive record types so the walk terminates.
                    if (!visited.insert(getTypeString(type)).second) {
                        continue;
                    }
                    if (type->isFunctionProtoType() || type->isFunctionNoProtoType()) {
                        return true;
                    }
                    if (type->isRecordType()) {
                        for (const auto *field : type->getAs<RecordType>()->getDecl()->fields()) {
                            stack.push_back(field->getType());
                        }
                    }
                }
                return false;
            }

            // Build the tree for the type \a T. A record contributes its function-pointer fields as
            // callbacks and recurses into the rest; a callback reached through more than one level of
            // indirection (a double pointer, an array or union of function pointers) is unsupported.
            static CallbackTree build(QualType T, std::set<std::string> &visited) {
                bool throughPointer = false;
                if (T->isPointerType()) {
                    // A pointer to a pointer that leads to a callback cannot be marshalled here.
                    if (auto pointee = T->getPointeeType(); pointee->isPointerType() &&
                                                            containsFunctionPointer(
                                                                pointee->getPointeeType())) {
                        return unsupported();
                    }
                    throughPointer = true;
                    T = T->getPointeeType();
                }
                if (T->isArrayType() &&
                    containsFunctionPointer(T->getAsArrayTypeUnsafe()->getElementType())) {
                    return unsupported();
                }
                if (T->isUnionType() && containsFunctionPointer(T)) {
                    return unsupported();
                }
                if (!T->isRecordType()) {
                    return noCallbacks();
                }

                CallbackTree node(Status::HasCallbacks);
                node.reach = throughPointer ? Reach::ByPointer : Reach::ByValue;
                for (const auto *field : T->getAs<RecordType>()->getDecl()->fields()) {
                    const auto type = field->getType().getCanonicalType();
                    const auto fieldName = field->getNameAsString();

                    if (type->isFunctionPointerType()) {
                        node.callbacks.emplace(fieldName, type->getPointeeType().getCanonicalType());
                        continue;
                    }

                    const auto typeStr = getTypeString(type);
                    if (!visited.insert(typeStr).second) {
                        // Already on the path: a cycle that carries a callback is unsupported.
                        if (containsFunctionPointer(type)) {
                            return unsupported();
                        }
                        continue;
                    }
                    auto child = build(type, visited);
                    visited.erase(typeStr);

                    if (child.status == Status::Unsupported) {
                        return unsupported();
                    }
                    if (child.status == Status::HasCallbacks) {
                        node.children.emplace(fieldName, std::move(child));
                    }
                }

                if (node.callbacks.empty() && node.children.empty()) {
                    return noCallbacks();
                }
                return node;
            }
        };

        // The three emitters below all walk the tree the same way. At each node they navigate to a
        // child by declaring a local pointer to it, named after the child's full path (the `____`
        // joined ancestor chain) so that members repeated at different depths never collide. `valExpr`
        // / `ctxExpr` are the prefixes to reach a member's value and its CallbackContext from the
        // current node ("" / "ctx." at the top, "p_PATH->" / "p_PATH_ctx->" inside a child).

        // Emit the substitution that runs before the call: wrap each foreign callback pointer in a
        // local-callable trampoline (CallbackContext_init records the original so it can be restored).
        std::string emitInit(const CallbackTree &node, const std::string &valExpr,
                             const std::string &ctxExpr, const std::string &idPath, bool guestCallback,
                             std::map<std::string, QualType> &allocators, int level) {
            std::ostringstream out;
            const auto pad = indent(level);
            for (const auto &[name, child] : node.children) {
                const auto childId = idPath + "____" + name;
                const auto var = "p_" + childId;
                const bool byValue = child.reach == CallbackTree::Reach::ByValue;
                out << pad << "auto " << var << " = " << (byValue ? "&" : "") << valExpr << name
                    << ";\n";
                out << pad << "auto " << var << "_ctx = &" << ctxExpr << name << ";\n";
                out << pad << "if (" << (byValue ? "1" : var) << ") {\n";
                out << emitInit(child, var + "->", var + "_ctx->", childId, guestCallback, allocators,
                                level + 1);
                out << pad << "}\n";
            }
            for (const auto &[name, calleeType] : node.callbacks) {
                const auto allocator = idPath + "____" + name + "_xx_ThunkAlloc";
                out << pad << "CallbackContext_init<" << (guestCallback ? "true" : "false") << ">("
                    << ctxExpr << name << ", (void *&) " << valExpr << name
                    << ", allocCallbackTrampoline<" << allocator << "::invoke>);\n";
                allocators.emplace(allocator, calleeType);
            }
            return out.str();
        }

        // Emit the cleanup that runs after the call: restore each callback pointer the matching
        // CallbackContext saved, so the caller's argument is left untouched.
        std::string emitFini(const CallbackTree &node, const std::string &valExpr,
                             const std::string &ctxExpr, const std::string &idPath, int level) {
            std::ostringstream out;
            const auto pad = indent(level);
            for (const auto &[name, child] : node.children) {
                const auto childId = idPath + "____" + name;
                const auto var = "p_" + childId;
                const bool byValue = child.reach == CallbackTree::Reach::ByValue;
                out << pad << "auto " << var << " = " << (byValue ? "&" : "") << valExpr << name
                    << ";\n";
                out << pad << "auto " << var << "_ctx = &" << ctxExpr << name << ";\n";
                out << pad << "if (" << (byValue ? "1" : var) << ") {\n";
                out << emitFini(child, var + "->", var + "_ctx->", childId, level + 1);
                out << pad << "}\n";
            }
            for (const auto &[name, calleeType] : node.callbacks) {
                out << pad << "CallbackContext_fini(" << ctxExpr << name << ");\n";
            }
            return out.str();
        }

        // Emit the context struct that mirrors the tree: a CallbackContext per callback and a nested
        // struct per child. Member names are scoped to their struct, so they need no path qualifier.
        std::string emitContextStruct(const CallbackTree &node, const std::string &member,
                                      const std::string &typeName, int level) {
            std::ostringstream out;
            const auto pad = indent(level);
            out << pad << "struct " << typeName << " {\n";
            for (const auto &[name, child] : node.children) {
                out << emitContextStruct(child, name, {}, level + 1) << "\n";
            }
            for (const auto &[name, calleeType] : node.callbacks) {
                out << indent(level + 1) << "struct CallbackContext " << name << ";\n";
            }
            out << pad << "} " << member << ";";
            return out.str();
        }

    } // namespace

    class CallbackSubstituterMessage : public PassMessage {
    public:
        explicit CallbackSubstituterMessage(CallbackTree tree) : tree(std::move(tree)) {
        }

        CallbackTree tree;
    };

    class CallbackSubstituterPass : public Pass {
    public:
        CallbackSubstituterPass() : Pass(Guard, lore::thunk::pass::ID_CallbackSubstituter) {
        }

        std::string name() const override {
            return "CallbackSubstituter";
        }

        bool testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        llvm::Error beginHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        llvm::Error endHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;

    protected:
        std::map<std::string, CallbackTree> m_treeCache;

        // The whole argument list as one synthetic aggregate: direct callback arguments are its
        // callbacks, struct arguments its children. Returns NoCallbacks when nothing needs wrapping,
        // Unsupported when any argument carries an unmarshallable callback shape.
        CallbackTree buildArgumentTree(const FunctionInfo &info) {
            CallbackTree top(CallbackTree::Status::HasCallbacks);
            for (const auto &[type, name] : info.arguments()) {
                const auto canon = type.getCanonicalType();
                if (canon->isFunctionPointerType()) {
                    top.callbacks.emplace(name, canon->getPointeeType().getCanonicalType());
                    continue;
                }

                // `callback (**)(...)`: an out parameter the callee fills with a callback.
                if (canon->isPointerType() &&
                    canon->getPointeeType()->isFunctionPointerType()) {
                    top.outCallbacks.emplace(
                        name, canon->getPointeeType()->getPointeeType().getCanonicalType());
                    continue;
                }

                const auto typeStr = getTypeString(canon);
                auto it = m_treeCache.find(typeStr);
                if (it == m_treeCache.end()) {
                    std::set<std::string> visited;
                    it = m_treeCache.emplace(typeStr, CallbackTree::build(canon, visited)).first;
                }

                const auto &argTree = it->second;
                if (argTree.status == CallbackTree::Status::Unsupported) {
                    return CallbackTree::unsupported();
                }
                if (argTree.status == CallbackTree::Status::HasCallbacks) {
                    top.children.emplace(name, argTree);
                }
            }

            if (top.callbacks.empty() && top.children.empty() && top.outCallbacks.empty()) {
                return CallbackTree::noCallbacks();
            }
            return top;
        }
    };

    bool CallbackSubstituterPass::testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) {
        auto tree = buildArgumentTree(proc.realFunctionTypeView());
        if (tree.status == CallbackTree::Status::NoCallbacks) {
            return false;
        }
        msg = std::make_unique<CallbackSubstituterMessage>(std::move(tree));
        return true;
    }

    llvm::Error CallbackSubstituterPass::beginHandleProc(ProcSnippet &proc,
                                                         std::unique_ptr<PassMessage> &msg) {
        const auto &tree = static_cast<CallbackSubstituterMessage &>(*msg).tree;

        auto &doc = proc.document();
        const auto key = name();

        const bool isHost = doc.mode() == DocumentContext::Host;
        const bool isG2H = proc.direction() == ProcSnippet::GuestToHost;

        // Substitution is built in the Adapt layer (where Entry has already unpacked the arguments
        // into typed form). In callbacks (passed in, possibly nested in structs) are wrapped on the
        // receiver side; out callbacks (filled in through a pointer-to-callback) on the caller side.
        // For a guest-to-host call the receiver is the host and the caller the guest. Only the side
        // matching this document is the real source; the other's writes are discarded.
        auto &ADP = proc.source(ProcSnippet::Adapt);
        ProcSnippet::ProcSource discard;
        auto &receiverADP = (isG2H == isHost) ? ADP : discard;
        auto &callerADP = (isG2H == isHost) ? discard : ADP;

        // A guest-to-host call carries guest callbacks in (the host calls back into the guest) and
        // host callbacks out (the guest calls what the host hands back); host-to-guest is the mirror.
        const char *inDirection = isG2H ? "HostToGuest" : "GuestToHost";
        const char *outDirection = isG2H ? "GuestToHost" : "HostToGuest";
        const bool inGuestCallback = isG2H;
        const bool outGuestCallback = !isG2H;

        if (tree.status == CallbackTree::Status::Unsupported) {
            receiverADP.body.forward.push_back(
                key, "#ifdef LORE_THUNK_CALLBACK_REPLACE\n#error \"unsupported callback type\"\n#endif\n");
            return llvm::Error::success();
        }

        // Resolve the ProcCb<> alias the manifest registered for a callee function type.
        const auto resolveAlias = [&](const QualType &calleeType) -> llvm::Expected<std::string> {
            const auto ptrType = doc.ast().getPointerType(calleeType.getCanonicalType());
            const auto it = doc.callbackTypes().find(getTypeString(ptrType.getCanonicalType()));
            if (it == doc.callbackTypes().end()) {
                return llvm::createStringError(std::errc::not_supported,
                                               "callback not found for type: %s",
                                               getTypeString(ptrType).c_str());
            }
            return it->second.name;
        };

        // In callbacks: wrap before the call and restore after (the trampoline persists, so the
        // receiver can still call it later; only the caller's argument is left untouched).
        if (!tree.callbacks.empty() || !tree.children.empty()) {
            const auto ctxStructName = proc.name() + "_xx_LocalContext";
            std::map<std::string, QualType> allocators;
            const auto initSource =
                emitInit(tree, {}, "ctx.", proc.name(), inGuestCallback, allocators, 2);
            const auto finiSource = emitFini(tree, {}, "ctx.", proc.name(), 2);
            const auto structSource = emitContextStruct(tree, {}, ctxStructName, 0);

            std::string usingDecls;
            for (const auto &[allocator, calleeType] : allocators) {
                auto alias = resolveAlias(calleeType);
                if (!alias) {
                    return alias.takeError();
                }
                usingDecls += "    using " + allocator + " = ProcCb<" + *alias + ", " + inDirection +
                              ", Entry>;\n";
            }

            std::ostringstream forwardSS;
            forwardSS << "#ifdef LORE_THUNK_CALLBACK_REPLACE\n"
                      << usingDecls << "\n"
                      << "    struct " << ctxStructName << " ctx;\n"
                      << "    {\n"
                      << initSource << "    }\n"
                      << "#endif\n";
            receiverADP.body.forward.push_front(key, forwardSS.str());

            std::ostringstream backwardSS;
            backwardSS << "#ifdef LORE_THUNK_CALLBACK_REPLACE\n"
                       << "    {\n"
                       << finiSource << "    }\n"
                       << "#endif\n";
            receiverADP.body.backward.push_back(key, backwardSS.str());

            std::array sources = {
                std::string("#ifdef LORE_THUNK_CALLBACK_REPLACE"),
                structSource,
                std::string("#endif"),
                std::string(),
            };
            receiverADP.head.push_back(key, llvm::join(sources, "\n"));
        }

        // Out callbacks: after the call the callee has filled *arg with a callback; hand the caller a
        // callable pointer for it in the opposite direction and keep that value (no restore). If it is
        // a stub we handed the callee earlier, CallbackContext_init's unwrap reverts it to the
        // original, so a set then get round-trips to the caller's own function.
        if (!tree.outCallbacks.empty()) {
            std::ostringstream ss;
            ss << "#ifdef LORE_THUNK_CALLBACK_REPLACE\n    {\n";
            for (const auto &[name, calleeType] : tree.outCallbacks) {
                auto alias = resolveAlias(calleeType);
                if (!alias) {
                    return alias.takeError();
                }
                const auto allocator = proc.name() + "____" + name + "_xx_OutThunkAlloc";
                ss << "        using " << allocator << " = ProcCb<" << *alias << ", " << outDirection
                   << ", Entry>;\n";
                ss << "        struct CallbackContext _xx_out_" << name << ";\n";
                ss << "        CallbackContext_init<" << (outGuestCallback ? "true" : "false")
                   << ">(_xx_out_" << name << ", (void *&) *" << name << ", allocCallbackTrampoline<"
                   << allocator << "::invoke>);\n";
            }
            ss << "    }\n#endif\n";
            callerADP.body.backward.push_back(key, ss.str());
        }

        return llvm::Error::success();
    }

    llvm::Error CallbackSubstituterPass::endHandleProc(ProcSnippet &proc,
                                                       std::unique_ptr<PassMessage> &msg) {
        return llvm::Error::success();
    }

    static llvm::Registry<Pass>::Add<CallbackSubstituterPass>
        PR_CallbackSubstituter("CallbackSubstituter", {});

}
