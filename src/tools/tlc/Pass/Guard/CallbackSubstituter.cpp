#include "Pass.h"

#include <sstream>

#include <stdcorelib/str.h>

#include <lorelei/TLCMeta/MetaPass.h>

#include "ProcContext.h"
#include "DocumentContext.h"
#include "PassCommon.h"
#include "BuilderPassCommon.h"

using namespace clang;

namespace TLC {

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

                //  Function pointer
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

    static std::string buildInitSource(bool isGCB, const Var &var, const std::string &parent,
                                       const std::string &ancestorName,
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
            ss << buildInitSource(isGCB, field, name, newAncestorName, allocators, level + 1);

            ss << indent(level) << "}\n";
        }
        for (const auto &fp : var.fps) {
            const auto &name = fp.first;
            std::string allocatorName = ancestorName + "____" + name + "_xx_ThunkAlloc";
            ss << indent(level)
               << (isGCB ? "CallbackContext_init<true>(" : "CallbackContext_init<false>(")
               << (parent.empty() ? "ctx." : ("p_" + parent + "_ctx->")) << name << ", (void *&) "
               << (parent.empty() ? name : ("p_" + parent + "->" + name)) << ", allocCallbackTrampoline<" << allocatorName
               << "::invoke>);\n";
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

    class CallbackSubstituterMessage : public ProcMessage {
    public:
        CallbackSubstituterMessage(Var var) : var(std::move(var)) {
        }

        Var var;
    };

    class CallbackSubstituterPass : public Pass {
    public:
        CallbackSubstituterPass() : Pass(Guard, lorethunk::MetaPass::CallbackSubstituter) {
        }

        std::string name() const override {
            return "CallbackSubstituter";
        }

        bool testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;
        llvm::Error beginHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;
        llvm::Error endHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;

    protected:
        std::map<std::string, const ClassTemplateSpecializationDecl *> _metaProcArgFilters;
        std::map<std::string, const ClassTemplateSpecializationDecl *> _metaProcRetFilters;

        std::map<std::string, Var> _varCache;

        Var buildFunctionVar(const FunctionInfo &info) {
            int i = 0;
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
                auto it = _varCache.find(typeString);
                if (it != _varCache.end()) {
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
                    _varCache.insert(std::make_pair(typeString, argVar));
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

    bool CallbackSubstituterPass::testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) {
        FunctionInfo FI = proc.realFunctionTypeView();
        auto var = buildFunctionVar(FI);
        if (var.type == Var::NotInterested) {
            return false;
        }
        msg = std::make_unique<CallbackSubstituterMessage>(std::move(var));
        return true;
    }

    llvm::Error CallbackSubstituterPass::beginHandleProc(ProcContext &proc,
                                                         std::unique_ptr<ProcMessage> &msg) {
        auto &message = static_cast<CallbackSubstituterMessage &>(*msg);

        auto &doc = proc.documentContext();
        auto &ast = *doc.ast();
        const auto &real = proc.realFunctionTypeView();

        std::string key = name();
        FunctionInfo FI = real;

        auto &GTP = proc.source(CProcThunkPhase_GTP);
        auto &GTP_IMPL = proc.source(CProcThunkPhase_GTP_IMPL);
        auto &HTP = proc.source(CProcThunkPhase_HTP);
        auto &HTP_IMPL = proc.source(CProcThunkPhase_HTP_IMPL);

        static const char *notSupportedError = "#error \"unsupported callback type\"";

        {
            const char *thunk_ProcKind_str;
            const char *thunk_phase_str;
            const char *metaProc_str;
            bool isHost;
            switch (proc.procKind()) {
                case CProcKind_HostFunction:
                case CProcKind_HostCallback:
                    thunk_ProcKind_str = "GuestCallback";
                    thunk_phase_str = "HTP";
                    isHost = true;
                    break;
                case CProcKind_GuestFunction:
                case CProcKind_GuestCallback:
                    thunk_ProcKind_str = "HostCallback";
                    thunk_phase_str = "GTP";
                    isHost = false;
                    break;
                default:
                    break;
            }

            auto &XTP = isHost ? GTP : HTP;
            auto &XTP_IMPL = isHost ? GTP_IMPL : HTP_IMPL;
            auto &YTP = isHost ? HTP : GTP;
            auto &YTP_IMPL = isHost ? HTP_IMPL : GTP_IMPL;

            if (message.var.type == Var::NotSupported) {
                std::stringstream ss;
                ss << "#ifdef LORETHUNK_CALLBACK_REPLACE\n" << notSupportedError << "\n#endif\n";
                YTP.body.forward.push_back(key, ss.str());
                return llvm::Error::success();
            }

            std::string ctxStructName = proc.name() + "_xx_LocalContext";
            std::map<std::string, QualType> thunksToGen;

            std::string initSource =
                buildInitSource(isHost, message.var, {}, proc.name(), thunksToGen, 2);
            std::string finiSource = buildFiniSource(message.var, {}, 2);
            std::string structSource = buildStructSource(message.var, {}, ctxStructName, 0);

            std::string thunkUsingDecls;
            for (const auto &it : std::as_const(thunksToGen)) {
                auto typeStr =
                    getTypeString(doc.ast()->getPointerType(it.second.getCanonicalType()));
                auto &callbacks = doc.callbacks();
                const auto &cbInfo = callbacks.find(typeStr);
                if (cbInfo == callbacks.end()) {
                    return llvm::createStringError(std::errc::not_supported,
                                                   "%s callback not found for type: %s",
                                                   isHost ? "host" : "guest", typeStr.c_str());
                }
                thunkUsingDecls += "    using " + it.first + " = MetaProcCB<" + cbInfo->first +
                                   ", CProcKind_" + thunk_ProcKind_str + ", CProcThunkPhase_" +
                                   thunk_phase_str + ">;\n";
            }

            // forward source
            {
                std::stringstream ss;
                ss << "#ifdef LORETHUNK_CALLBACK_REPLACE\n"
                   << thunkUsingDecls //
                   << "\n"
                   << "    struct " << ctxStructName << " ctx;\n"
                   << "    {\n"
                   << initSource //
                   << "    }\n"
                   << "#endif\n";
                YTP.body.forward.push_front(key, ss.str());
            }

            // backward source
            {
                std::stringstream ss;
                ss << "#ifdef LORETHUNK_CALLBACK_REPLACE\n"
                   << "    {\n"
                   << finiSource //
                   << "    }\n"
                   << "#endif\n";
                YTP.body.backward.push_back(key, ss.str());
            }

            // global forward source
            std::array sources = {
                std::string("#ifdef LORETHUNK_CALLBACK_REPLACE"),
                structSource,
                std::string("#endif"),
                std::string(),
            };
            YTP.head.push_back(key, llvm::join(sources, "\n"));
        }

        return llvm::Error::success();
    }

    llvm::Error CallbackSubstituterPass::endHandleProc(ProcContext &proc,
                                                       std::unique_ptr<ProcMessage> &msg) {
        return llvm::Error::success();
    }

    static PassRegistration<CallbackSubstituterPass> PR_CallbackSubstituter;

}