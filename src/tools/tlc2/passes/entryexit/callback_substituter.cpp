#include "pass.h"

#include <set>
#include <sstream>

#include "typerep.h"
#include "thunkdefinition.h"
#include "common.h"
#include "analyzer.h"

using namespace clang;
using namespace llvm;

namespace TLC {

    class CallbackContextBuilder {
    public:
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
        };

        Var rootVar;

        static bool hasFP(const QualType &qt) {
            SmallVector<QualType> stack;
            stack.push_back(qt);

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
                    auto recType = type->getAs<RecordType>();
                    for (const auto &T : recType->getDecl()->fields()) {
                        stack.push_back(T->getType());
                    }
                    continue;
                }
            }
            return false;
        }

        static Var buildVar(QualType qt, std::set<std::string> &visitedTypes) {
            bool is_ptr = false;
            if (qt->isPointerType()) {
                auto pointee = qt->getPointeeType();
                if (pointee->isPointerType()) {
                    if (hasFP(pointee->getPointeeType())) {
                        return {};
                    }
                }
                is_ptr = true;
                qt = pointee;
            }
            if (qt->isArrayType()) {
                if (hasFP(qt->getAsArrayTypeUnsafe()->getElementType())) {
                    return {};
                }
            }
            if (qt->isUnionType()) {
                if (hasFP(qt)) {
                    return {};
                }
            }
            if (qt->isRecordType()) {
                auto recType = qt->getAs<RecordType>();
                Var var(is_ptr ? Var::Pointer : Var::Field);
                for (const auto &T : recType->getDecl()->fields()) {
                    auto type = T->getType().getCanonicalType();
                    if (type->isFunctionPointerType()) {
                        var.fps[T->getNameAsString()] = type->getPointeeType().getCanonicalType();
                        continue;
                    }

                    auto typeString = getTypeString(type);
                    if (visitedTypes.count(typeString)) {
                        if (!hasFP(type)) {
                            continue;
                        }
                        return {};
                    }
                    auto it = visitedTypes.insert(typeString);

                    auto field = buildVar(type, visitedTypes);

                    visitedTypes.erase(it.first);

                    if (field.type == Var::NotInterested) {
                        continue;
                    }
                    if (field.type == Var::NotSupported) {
                        return {};
                    }
                    var.fields.insert(std::make_pair(T->getNameAsString(), std::move(field)));
                }
                if (!var.fields.empty() || !var.fps.empty()) {
                    return var;
                }
            }
            return Var(Var::NotInterested);
        }

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
                // parent = "parent"
                // __auto_type p_field = &p_parent->field;
                // __auto_type p_field = p_parent->field;
                // parent = ""
                // __auto_type p_field_ctx = field;
                ss << indent(level) << "__auto_type p_" << name << " = "
                   << (field.type == Var::Field ? "&" : "")
                   << (parent.empty() ? std::string() : ("p_" + parent + "->")) << name << ";\n";

                // parent = "parent"
                // __auto_type p_field_ctx = &p_parent_ctx->field;
                // parent = ""
                // __auto_type p_field_ctx = &ctx.field;
                ss << indent(level) << "__auto_type p_" << name << "_ctx = &"
                   << (parent.empty() ? std::string("ctx.") : ("p_" + parent + "_ctx->")) + name
                   << ";\n";

                // if (1) {
                // if (p_field) {
                ss << indent(level) << "if (" << (field.type == Var::Field ? "1" : ("p_" + name))
                   << ") {\n";

                std::string newAncestorName = ancestorName + "____" + name;
                ss << buildInitSource(isGCB, field, name, newAncestorName, allocators, level + 1);

                // }
                ss << indent(level) << "}\n";
            }
            for (const auto &fp : var.fps) {
                const auto &name = fp.first;
                std::string allocatorName = ancestorName + "____" + name + "_xx_ThunkAlloc";
                ss << indent(level)
                   << (isGCB ? "LoreLib_CallbackContext_init("
                             : "LoreLib_CallbackContext_init_HCB(")
                   << (parent.empty() ? "ctx." : ("p_" + parent + "_ctx->")) << name << ", "
                   << (parent.empty() ? name : ("p_" + parent + "->" + name)) << ", "
                   << allocatorName << ");\n";
                allocators[allocatorName] = fp.second;
            }
            return ss.str();
        }

        static std::string buildFiniSource(const Var &var, const std::string &parent, int level) {
            std::stringstream ss;
            for (const auto &pair : var.fields) {
                auto &name = pair.first;
                auto &field = pair.second;

                // parent = "parent"
                // __auto_type p_field = &p_parent->field;
                // __auto_type p_field = p_parent->field;
                // parent = ""
                // __auto_type p_field_ctx = field;
                ss << indent(level) << "__auto_type p_" << name << " = "
                   << (field.type == Var::Field ? "&" : "")
                   << (parent.empty() ? std::string() : ("p_" + parent + "->")) << name << ";\n";

                // parent = "parent"
                // __auto_type p_field_ctx = &p_parent_ctx->field;
                // parent = ""
                // __auto_type p_field_ctx = &ctx.field;
                ss << indent(level) << "__auto_type p_" << name << "_ctx = &"
                   << (parent.empty() ? std::string("ctx.") : ("p_" + parent + "_ctx->")) + name
                   << ";\n";

                // if (1) {
                // if (p_field) {
                ss << indent(level) << "if (" << (field.type == Var::Field ? "1" : ("p_" + name))
                   << ") {\n";

                ss << buildFiniSource(field, name, level + 1);

                // }
                ss << indent(level) << "}\n";
            }
            for (const auto &fp : var.fps) {
                const auto &name = fp.first;
                ss << indent(level) << "LoreLib_CallbackContext_fini("
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
                ss << indent(level + 1) << "struct LoreLib_CallbackContext " << fpName << ";\n";
            }
            ss << indent(level) << "} " << name << ";";
            return ss.str();
        }

        CallbackContextBuilder(const FunctionTypeView &FTV) {
            int i = 0;
            Var var(Var::Field);
            for (const auto &argType : FTV.argTypes()) {
                auto type = argType.getCanonicalType();
                auto name = "arg" + std::to_string(++i);
                if (type->isFunctionPointerType()) {
                    var.fps[name] = type->getPointeeType().getCanonicalType();
                    continue;
                }

                std::set<std::string> visitedTypes;
                auto argVar = buildVar(type, visitedTypes);
                if (argVar.type == Var::NotInterested) {
                    continue;
                }
                if (argVar.type == Var::NotSupported) {
                    return;
                }
                var.fields.insert(std::make_pair(name, std::move(argVar)));
            }
            if (!var.fields.empty() || !var.fps.empty()) {
                rootVar = std::move(var);
            } else {
                rootVar = Var(Var::NotInterested);
            }
        }
    };

    class CallbackSubstituterPass : public EntryExitPass {
    public:
        CallbackSubstituterPass() : EntryExitPass(PT_CallbackSubstituter, "callback_substituter") {
        }
        ~CallbackSubstituterPass() = default;

    public:
        bool test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const override;
        Error begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) override;
        Error end(ThunkDefinition &thunk) override;
    };

    bool CallbackSubstituterPass::test(const ThunkDefinition &thunk,
                                       SmallVectorImpl<IntOrString> &args) const {
        return true;
    }

    Error CallbackSubstituterPass::begin(ThunkDefinition &thunk,
                                         const SmallVectorImpl<IntOrString> &args) {
        auto typeView = thunk.view();

        auto thunkRetType = typeView.returnType();
        auto thunkArgTypes = typeView.argTypes();
        bool isGuest = thunk.type() == ThunkDefinition::GuestCallbackThunk;

        FunctionDefinition &TP =
            thunk.function(isGuest ? FunctionDefinition::GTP : FunctionDefinition::HTP);

        std::string ID = "callback_substituter";

        CallbackContextBuilder builder(typeView);
        switch (builder.rootVar.type) {
            case CallbackContextBuilder::Var::NotInterested:
                return Error::success();

            case CallbackContextBuilder::Var::NotSupported:
                TP.bodyForward().push_back(ID, "    // ERROR: unsupported callback type\n");
                return Error::success();

            default:
                break;
        }

        std::string ctxStructName = TP.rep().name() + "_xx_LocalContext";

        std::map<std::string, QualType> thunksToGen;

        std::string initSource;
        initSource =
            builder.buildInitSource(!isGuest, builder.rootVar, {}, TP.rep().name(), thunksToGen, 2);

        std::string finiSource;
        finiSource = builder.buildFiniSource(builder.rootVar, {}, 2);

        std::string structSource;
        structSource = builder.buildStructSource(builder.rootVar, {}, ctxStructName, 0);

        std::string allocatorsSource;
        {
            auto analyzer = thunk.parent();

            std::stringstream ss;
            for (const auto &pair : thunksToGen) {
                const auto &name = pair.first;
                auto type = pair.second.getCanonicalType();

                auto callbackHandler = analyzer->callbackName(getTypeString(type), isGuest);
                if (callbackHandler.empty()) {
                    return createStringError(std::errc::not_supported,
                                             "callback handler not found for type: %s",
                                             getTypeString(type).c_str());
                }

                ss << (isGuest ? "LORELIB_HCB_THUNK_ALLOCATOR" : "LORELIB_GCB_THUNK_ALLOCATOR")
                   << "(" << name << ", " << (isGuest ? "__GTP_HCB_" : "__HTP_GCB_")
                   << callbackHandler << ")\n";
            }
            allocatorsSource = ss.str();
        }

        // forward source
        {
            std::stringstream ss;
            ss << "#ifdef LORELIB_CALLBACK_REPLACE\n"
               << "    struct " << ctxStructName << " ctx;\n"
               << "    {\n"
               << initSource //
               << "    }\n"
               << "#endif\n";
            TP.bodyForward().push_front(ID, ss.str());
        }

        // backward source
        {
            std::stringstream ss;
            ss << "#ifdef LORELIB_CALLBACK_REPLACE\n"
               << "    {\n"
               << finiSource //
               << "    }\n"
               << "#endif\n";
            TP.bodyBackward().push_back(ID, ss.str());
        }

        // global forward source
        std::array sources = {
            std::string("#ifdef LORELIB_CALLBACK_REPLACE"),
            allocatorsSource,
            structSource,
            std::string("#endif"),
            std::string(),
        };
        thunk.sourceForward(isGuest).push_back(ID, llvm::join(sources, "\n"));

        return Error::success();
    }

    Error CallbackSubstituterPass::end(ThunkDefinition &thunk) {
        return Error::success();
    }


    /// \brief Pass registrations.
    static PassRegistration<CallbackSubstituterPass> PR_callback_substituter;

}