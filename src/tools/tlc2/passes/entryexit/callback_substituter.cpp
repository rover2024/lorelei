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

        static Var buildVar(QualType qt) {
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
                        var.fps[T->getNameAsString()] = type;
                        continue;
                    }

                    auto field = buildVar(type);
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

        static std::string buildInitSource(const Var &var, const std::string &parent,
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
                // __auto_type p_field_ctx = &ctx->field;
                ss << indent(level) << "__auto_type p_" << name << "_ctx = &"
                   << (parent.empty() ? std::string("ctx") : ("p_" + parent + "_ctx")) + "->" + name
                   << ";\n";

                // if (true) {
                // if (p_field) {
                ss << indent(level) << "if (" << (field.type == Var::Field ? "true" : ("p_" + name))
                   << ") {\n";

                std::string newAncestorName = ancestorName + "____" + name;
                ss << buildInitSource(field, name, newAncestorName, allocators, level + 1);

                // }
                ss << indent(level) << "}";
            }
            for (const auto &fp : var.fps) {
                const auto &name = fp.first;
                std::string allocatorName = ancestorName + "____" + name + "_xx_ThunkAlloc";
                ss << indent(level) << "LoreLib_CallbackContext_init(p_" << parent << "_ctx->"
                   << name << ", p_" << parent << ", " << allocatorName << ");\n";
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
                // __auto_type p_field_ctx = &ctx->field;
                ss << indent(level) << "__auto_type p_" << name << "_ctx = &"
                   << (parent.empty() ? std::string("ctx") : ("p_" + parent + "_ctx")) + "->" + name
                   << ";\n";

                // if (true) {
                // if (p_field) {
                ss << indent(level) << "if (" << (field.type == Var::Field ? "true" : ("p_" + name))
                   << ") {\n";

                ss << buildFiniSource(field, name, level + 1);

                // }
                ss << indent(level) << "}";
            }
            for (const auto &fp : var.fps) {
                const auto &name = fp.first;
                ss << indent(level) << "LoreLib_CallbackContext_fini(p_" << parent << "_ctx->"
                   << name << ");\n";
            }
            return ss.str();
        }

        static std::string buildStructSource(const Var &var, const std::string &name, int level) {
            std::stringstream ss;
            ss << indent(level) << "struct {\n";
            for (const auto &pair : var.fields) {
                auto &fieldName = pair.first;
                auto &field = pair.second;
                ss << buildStructSource(field, fieldName, level + 1);
            }
            for (const auto &fp : var.fps) {
                const auto &fpName = fp.first;
                ss << indent(level + 1) << "struct LoreLib_CallbackContext " << fpName << ";\n";
            }
            ss << indent(level) << "}" << name << ";\n";
            return ss.str();
        }

        CallbackContextBuilder(const FunctionTypeView &FTV) {
            int i = 0;
            Var var(Var::Field);
            for (const auto &argType : FTV.argTypes()) {
                auto type = argType.getCanonicalType();
                auto name = "arg" + std::to_string(++i);
                if (type->isFunctionPointerType()) {
                    var.fps[name] = type;
                    continue;
                }

                auto argVar = buildVar(type);
                if (argVar.type == Var::NotInterested) {
                    continue;
                }
                if (argVar.type == Var::NotSupported) {
                    return;
                }
                var.fields.insert(std::make_pair(name, std::move(argVar)));
            }
            rootVar = std::move(var);
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
        bool isGuest = thunk.type() == ThunkDefinition::HostCallbackThunk;

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
        {
            std::stringstream ss;
            ss << "#ifdef LORELIB_CALLBACK_REPLACE\n"
               << "    struct " << ctxStructName << " ctx;\n"
               << "    " << ctxStructName << "_Init(&ctx"
               << getCallArgsString(thunkArgTypes.size(), true) << ");\n"
               << "#endif\n";
            TP.bodyForward().push_back(ID, ss.str());
        }

        {
            std::stringstream ss;
            ss << "#ifdef LORELIB_CALLBACK_REPLACE\n"
               << "    " << ctxStructName << "_Fini(&ctx);\n"
               << "#endif\n";
            TP.bodyBackward().push_back(ID, ss.str());
        }

        std::map<std::string, QualType> thunksToGen;
        std::string structSource;
        std::string initSource;
        std::string finiSource;

        structSource = builder.buildStructSource(builder.rootVar, ctxStructName, 0);

        {
            std::stringstream ss;
            ss << "static inline void " << ctxStructName << "_Init(" << ctxStructName << " *ctx";
            int i = 0;
            for (const auto &argType : thunkArgTypes) {
                ++i;
                ss << ", __typeof__(" << getTypeString(argType) << ") arg" << i;
            }
            ss << ") {\n";
            ss << builder.buildInitSource(builder.rootVar, {}, TP.rep().name(), thunksToGen, 1);
            ss << "}";
            initSource = ss.str();
        }

        {
            std::stringstream ss;
            ss << "static inline void " << ctxStructName << "_Fini(" << ctxStructName
               << " *ctx) {\n";
            ss << builder.buildFiniSource(builder.rootVar, {}, 1);
            ss << "}";
            finiSource = ss.str();
        }

        std::string allocatorsSource;
        {
            auto analyzer = thunk.parent();

            std::stringstream ss;
            for (const auto &pair : thunksToGen) {
                const auto &name = pair.first;
                const auto &type = pair.second;

                auto callbackHandler =
                    analyzer->callbackName(type.getCanonicalType().getAsString(), isGuest);
                if (callbackHandler.empty()) {
                    return createStringError(
                        std::errc::not_supported,
                        "callback handler not found for type: ", type.getAsString().c_str());
                }

                ss << (isGuest ? "LORELIB_HCB_THUNK_ALLOCATOR" : "LORELIB_GCB_THUNK_ALLOCATOR")
                   << "(" << (isGuest ? "__GTP_HCB_" : "__HTP_GCB_") << callbackHandler << ", "
                   << name << ")\n";
            }
            allocatorsSource = ss.str();
        }

        std::array sources = {
            std::string("#ifdef LORELIB_CALLBACK_REPLACE"),
            allocatorsSource,
            structSource,
            initSource,
            finiSource,
            std::string("#endif"),
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