#include <sstream>

#include <lorelei/Tools/TLCApi/Core/Pass.h>
#include <lorelei/Tools/TLCApi/Core/ProcSnippet.h>
#include <lorelei/Tools/TLCApi/Core/DocumentContext.h>
#include <lorelei/Tools/ThunkInterface/PassTags.h>
#include <lorelei/Tools/ToolUtils/TypeUtils.h>

#include "Utils/PassCodeTemplates.h"

using namespace clang;

namespace lore::tool::TLC {

    struct Var {
        enum Type {
            Field,
            Pointer,
            NotInterested,
            NotSupported,
        };
        Type type;
        std::map<std::string, Var> fields;
        std::map<std::string, QualType> fps;

        Var(Type type = NotSupported) : type(type) {
        }

        static bool hasFunctionPointer(const QualType &T) {
            SmallVector<QualType> stack;
            stack.push_back(T);

            std::set<std::string> visitedTypes;
            while (!stack.empty()) {
                auto type = stack.pop_back_val().getCanonicalType();
                while (true) {
                    if (type->isPointerType()) {
                        type = type->getPointeeType();
                        continue;
                    }
                    if (type->isArrayType()) {
                        type = type->getAsArrayTypeUnsafe()->getElementType();
                        continue;
                    }
                    break;
                }

                type = type.getCanonicalType();
                auto Name = getTypeString(type);
                if (visitedTypes.count(Name)) {
                    continue;
                }
                visitedTypes.insert(Name);

                if (type->isFunctionProtoType()) {
                    return true;
                }
                if (type->isFunctionNoProtoType()) {
                    return true;
                }
                if (type->isRecordType()) {
                    auto recordType = type->getAs<RecordType>();
                    for (const auto &fieldDecl : recordType->getDecl()->fields()) {
                        stack.push_back(fieldDecl->getType());
                    }
                    continue;
                }
            }
            return false;
        }

        static Var build(QualType T, std::set<std::string> &visitedTypes) {
            bool is_ptr = false;
            if (T->isPointerType()) {
                auto pointee = T->getPointeeType();
                if (pointee->isPointerType()) {
                    if (hasFunctionPointer(pointee->getPointeeType())) {
                        return {};
                    }
                }
                is_ptr = true;
                T = pointee;
            }
            if (T->isArrayType()) {
                if (hasFunctionPointer(T->getAsArrayTypeUnsafe()->getElementType())) {
                    return {};
                }
            }
            if (T->isUnionType()) {
                if (hasFunctionPointer(T)) {
                    return {};
                }
            }
            if (T->isRecordType()) {
                auto recType = T->getAs<RecordType>();
                Var var(is_ptr ? Var::Pointer : Var::Field);
                for (const auto &fieldDecl : recType->getDecl()->fields()) {
                    auto type = fieldDecl->getType().getCanonicalType();
                    if (type->isFunctionPointerType()) {
                        var.fps[fieldDecl->getNameAsString()] =
                            type->getPointeeType().getCanonicalType();
                        continue;
                    }

                    auto typeString = getTypeString(type);
                    if (visitedTypes.count(typeString)) {
                        if (!hasFunctionPointer(type)) {
                            continue;
                        }
                        return {};
                    }
                    auto it = visitedTypes.insert(typeString);

                    auto field = build(type, visitedTypes);
                    visitedTypes.erase(it.first);
                    if (field.type == Var::NotInterested) {
                        continue;
                    }
                    if (field.type == Var::NotSupported) {
                        return {};
                    }
                    var.fields.insert(
                        std::make_pair(fieldDecl->getNameAsString(), std::move(field)));
                }
                if (!var.fields.empty() || !var.fps.empty()) {
                    return var;
                }
            }
            return Var(Var::NotInterested);
        }
    };

    static std::string indent(int level) {
        return std::string(level * 4, ' ');
    }

    static std::string buildInitSource(bool isGuestCallback, const Var &var,
                                       const std::string &parent, const std::string &ancestorName,
                                       std::map<std::string, QualType> &allocators, int level) {
        std::stringstream ss;
        for (const auto &pair : var.fields) {
            auto &name = pair.first;
            auto &field = pair.second;
            ss << indent(level) << "auto p_" << name << " = "
               << (field.type == Var::Field ? "&" : "")
               << (parent.empty() ? std::string() : ("p_" + parent + "->")) << name << ";\n";

            ss << indent(level) << "auto p_" << name << "_ctx = &"
               << (parent.empty() ? std::string("ctx.") : ("p_" + parent + "_ctx->")) + name
               << ";\n";

            ss << indent(level) << "if (" << (field.type == Var::Field ? "1" : ("p_" + name))
               << ") {\n";

            std::string newAncestorName = ancestorName + "____" + name;
            ss << buildInitSource(isGuestCallback, field, name, newAncestorName, allocators,
                                  level + 1);

            ss << indent(level) << "}\n";
        }
        for (const auto &fp : var.fps) {
            const auto &name = fp.first;
            std::string allocatorName = ancestorName + "____" + name + "_xx_ThunkAlloc";
            ss << indent(level)
               << (isGuestCallback ? "CallbackContext_init<true>(" : "CallbackContext_init<false>(")
               << (parent.empty() ? "ctx." : ("p_" + parent + "_ctx->")) << name << ", (void *&) "
               << (parent.empty() ? name : ("p_" + parent + "->" + name))
               << ", allocCallbackTrampoline<" << allocatorName << "::invoke>);\n";
            allocators[allocatorName] = fp.second;
        }
        return ss.str();
    }

    static std::string buildFiniSource(const Var &var, const std::string &parent, int level) {
        std::stringstream ss;
        for (const auto &pair : var.fields) {
            auto &name = pair.first;
            auto &field = pair.second;

            ss << indent(level) << "auto p_" << name << " = "
               << (field.type == Var::Field ? "&" : "")
               << (parent.empty() ? std::string() : ("p_" + parent + "->")) << name << ";\n";

            ss << indent(level) << "auto p_" << name << "_ctx = &"
               << (parent.empty() ? std::string("ctx.") : ("p_" + parent + "_ctx->")) + name
               << ";\n";

            ss << indent(level) << "if (" << (field.type == Var::Field ? "1" : ("p_" + name))
               << ") {\n";

            ss << buildFiniSource(field, name, level + 1);

            ss << indent(level) << "}\n";
        }
        for (const auto &fp : var.fps) {
            const auto &name = fp.first;
            ss << indent(level) << "CallbackContext_fini("
               << (parent.empty() ? "ctx." : ("p_" + parent + "_ctx->")) << name << ");\n";
        }
        return ss.str();
    }

    static std::string buildStructSource(const Var &var, const std::string &name,
                                         const std::string &typeName, int level) {
        std::stringstream ss;
        ss << indent(level) << "struct " << typeName << " {\n";
        for (const auto &pair : var.fields) {
            auto &fieldName = pair.first;
            auto &field = pair.second;
            ss << buildStructSource(field, fieldName, {}, level + 1) << "\n";
        }
        for (const auto &fp : var.fps) {
            const auto &fpName = fp.first;
            ss << indent(level + 1) << "struct CallbackContext " << fpName << ";\n";
        }
        ss << indent(level) << "} " << name << ";";
        return ss.str();
    }

    class CallbackSubstituterMessage : public PassMessage {
    public:
        CallbackSubstituterMessage(Var var) : var(std::move(var)) {
        }

        Var var;
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
        std::map<std::string, Var> m_varCache;

        Var buildFunctionVar(const FunctionInfo &info) {
            Var var(Var::Field);
            for (const auto &arg : info.arguments()) {
                const auto &type = arg.first.getCanonicalType();
                const auto &name = arg.second;
                if (type->isFunctionPointerType()) {
                    var.fps[name] = type->getPointeeType().getCanonicalType();
                    continue;
                }

                Var argVar;
                std::string typeString = getTypeString(type);
                auto it = m_varCache.find(typeString);
                if (it != m_varCache.end()) {
                    if (it->second.type == Var::NotInterested) {
                        continue;
                    }
                    argVar = it->second;
                    if (argVar.type == Var::NotSupported) {
                        return {};
                    }
                } else {
                    std::set<std::string> visitedTypes;
                    argVar = Var::build(type, visitedTypes);
                    m_varCache.insert(std::make_pair(typeString, argVar));
                    if (argVar.type == Var::NotInterested) {
                        continue;
                    }
                    if (argVar.type == Var::NotSupported) {
                        return {};
                    }
                }
                var.fields.insert(std::make_pair(name, std::move(argVar)));
            }

            if (!var.fields.empty() || !var.fps.empty()) {
                return var;
            }
            return Var(Var::NotInterested);
        }
    };

    bool CallbackSubstituterPass::testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) {
        FunctionInfo FI = proc.realFunctionTypeView();
        auto var = buildFunctionVar(FI);
        if (var.type == Var::NotInterested) {
            return false;
        }
        msg = std::make_unique<CallbackSubstituterMessage>(std::move(var));
        return true;
    }

    llvm::Error CallbackSubstituterPass::beginHandleProc(ProcSnippet &proc,
                                                         std::unique_ptr<PassMessage> &msg) {
        auto &message = static_cast<CallbackSubstituterMessage &>(*msg);

        auto &doc = proc.document();
        std::string key = name();

        bool isHost = doc.mode() == DocumentContext::Host;
        bool isG2H = proc.direction() == ProcSnippet::GuestToHost;

        auto &ENT = proc.source(ProcSnippet::Entry);
        auto &CAL = proc.source(ProcSnippet::Caller);
        ProcSnippet::ProcSource emptyENT;
        ProcSnippet::ProcSource emptyCAL;

        auto &GENT = isHost ? emptyENT : ENT;
        auto &GCAL = isHost ? emptyCAL : CAL;
        auto &HENT = isHost ? ENT : emptyENT;
        auto &HCAL = isHost ? CAL : emptyCAL;

        auto &YENT = isG2H ? HENT : GENT;

        static const char *notSupportedError = "#error \"unsupported callback type\"";

        const char *callbackDirectionStr = isG2H ? "HostToGuest" : "GuestToHost";
        bool isGuestCallback = isG2H;

        if (message.var.type == Var::NotSupported) {
            std::stringstream ss;
            ss << "#ifdef LORE_THUNK_CALLBACK_REPLACE\n" << notSupportedError << "\n#endif\n";
            YENT.body.forward.push_back(key, ss.str());
            return llvm::Error::success();
        }

        std::string ctxStructName = proc.name() + "_xx_LocalContext";
        std::map<std::string, QualType> thunksToGen;

        std::string initSource =
            buildInitSource(isGuestCallback, message.var, {}, proc.name(), thunksToGen, 2);
        std::string finiSource = buildFiniSource(message.var, {}, 2);
        std::string structSource = buildStructSource(message.var, {}, ctxStructName, 0);

        const auto &findCallbackAliasByType =
            [&](const QualType &callbackFnType) -> std::optional<std::string> {
            auto callbackFnPtrType = doc.ast().getPointerType(callbackFnType.getCanonicalType());
            const auto targetTypeStr = getTypeString(callbackFnPtrType.getCanonicalType());

            for (int direction = ProcSnippet::GuestToHost; direction < ProcSnippet::NumDirections;
                 ++direction) {
                const auto &callbacks = doc.procs(ProcSnippet::Callback,
                                                  static_cast<ProcSnippet::Direction>(direction));
                for (const auto &[_, callbackProc] : callbacks) {
                    if (getTypeString(callbackProc.realFunctionPointerType().getCanonicalType()) ==
                        targetTypeStr) {
                        return callbackProc.name();
                    }
                }
            }
            return std::nullopt;
        };

        std::string thunkUsingDecls;
        for (const auto &it : std::as_const(thunksToGen)) {
            auto callbackAlias = findCallbackAliasByType(it.second);
            if (!callbackAlias.has_value()) {
                auto typeStr =
                    getTypeString(doc.ast().getPointerType(it.second.getCanonicalType()));
                return llvm::createStringError(std::errc::not_supported,
                                               "callback not found for type: %s", typeStr.c_str());
            }
            thunkUsingDecls += "    using " + it.first + " = ProcCb<" + callbackAlias.value() +
                               ", " + callbackDirectionStr + ", Entry>;\n";
        }

        {
            std::stringstream ss;
            ss << "#ifdef LORE_THUNK_CALLBACK_REPLACE\n"
               << thunkUsingDecls << "\n"
               << "    struct " << ctxStructName << " ctx;\n"
               << "    {\n"
               << initSource << "    }\n"
               << "#endif\n";
            YENT.body.forward.push_front(key, ss.str());
        }

        {
            std::stringstream ss;
            ss << "#ifdef LORE_THUNK_CALLBACK_REPLACE\n"
               << "    {\n"
               << finiSource << "    }\n"
               << "#endif\n";
            YENT.body.backward.push_back(key, ss.str());
        }

        std::array sources = {
            std::string("#ifdef LORE_THUNK_CALLBACK_REPLACE"),
            structSource,
            std::string("#endif"),
            std::string(),
        };
        YENT.head.push_back(key, llvm::join(sources, "\n"));

        (void) GCAL;
        (void) HCAL;
        return llvm::Error::success();
    }

    llvm::Error CallbackSubstituterPass::endHandleProc(ProcSnippet &proc,
                                                       std::unique_ptr<PassMessage> &msg) {
        return llvm::Error::success();
    }

    static llvm::Registry<Pass>::Add<CallbackSubstituterPass>
        PR_CallbackSubstituter("CallbackSubstituter", {});

}
