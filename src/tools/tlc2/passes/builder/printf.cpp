#include "pass.h"

#include <sstream>

#include <clang/AST/Attr.h>

#include "thunkdefinition.h"
#include "common.h"

using namespace clang;
using namespace llvm;

namespace TLC {

    template <bool PrependCallback = false>
    static FunctionDeclRep declRep(std::string name, const QualType &thunkRetType,
                                   ArrayRef<QualType> thunkArgTypes, int fixedArgCount) {
        SmallVector<TypeRep> argTypes;
        SmallVector<std::string> argNames;
        if constexpr (PrependCallback) {
            argTypes.push_back("void *");
            argNames.push_back("callback");
        }
        for (int i = 0; i < fixedArgCount; ++i) {
            argTypes.push_back(thunkArgTypes[i]);
            argNames.push_back("arg" + std::to_string(i + 1));
        }
        argTypes.push_back("struct LORE_VARG_ENTRY *");
        argNames.push_back("vargs1");

        FunctionTypeRep newTypeRep;
        newTypeRep.setReturnType(thunkRetType);
        newTypeRep.setArgTypes(argTypes);

        return FunctionDeclRep(std::move(newTypeRep), name, std::move(argNames));
    }

    static void begin_GuestFunction(std::string ID, ThunkDefinition &thunk, int fmtIdx, int vargIdx,
                                    bool scanf, bool va = false) {
        auto helperFunc = scanf ? "Lore_ExtractScanFArgs" : "Lore_ExtractPrintFArgs";
        auto typeView = thunk.view();

        auto thunkRetType = typeView.returnType();
        auto thunkArgTypes = typeView.argTypes();
        bool isVoid = thunkRetType->isVoidType();
        int fixedArgCount = va ? (thunkArgTypes.size() - 1) : thunkArgTypes.size();

        LIST_FDS(thunk);

        // GTP
        if (!GTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }

                ss << "    struct LORE_VARG_ENTRY vargs1[100];\n";
                if (va) {
                    ss << "    " << helperFunc << "(arg" << fmtIdx << ", arg" << vargIdx
                       << ", vargs1);\n";
                } else {
                    ss << "    {\n";
                    ss << "        va_list ap;\n";
                    ss << "        va_start(ap, arg" << vargIdx - 1 << ");\n";
                    ss << "        " << helperFunc << "(arg" << fmtIdx << ", ap, vargs1);\n";
                    ss << "        va_end(ap);\n";
                    ss << "    }\n";
                }
                GTP.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << GTP_IMPL.rep().name() << "(" + getCallArgsString(fixedArgCount)
                   << ", vargs1);\n";
                GTP.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                GTP.epilog().push_back(ID, ss.str());
            }
        }

        // GTP_IMPL
        if (!GTP_IMPL.rep().decl()) {
            // params
            GTP_IMPL.setRep(
                declRep(GTP_IMPL.rep().name(), thunkRetType, thunkArgTypes, fixedArgCount));

            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }

                ss << "    void *args[] = {\n";
                for (int i = 0; i < fixedArgCount; ++i) {
                    ss << "        (void *) &arg" << i + 1 << ",\n";
                }
                ss << "    };\n";

                ss << "    void *metadata[] = {\n";
                ss << "        (void *) &vargs1,\n";
                ss << "    };\n";

                GTP_IMPL.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    LORELIB_INVOKE_HTP(LORELIB_HTP(" << thunk.name() << "), args, "
                   << (isVoid ? "NULL" : "&ret") << ", metadata);\n";
                GTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    return ret;\n";
                }
                GTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }

        // HTP
        if (!HTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                for (int i = 0; i < fixedArgCount; ++i) {
                    ss << "    __auto_type arg" << i + 1 //
                       << " = *(__typeof__(" << getTypeString(thunkArgTypes[i]) << ") *) args[" << i
                       << "];\n";
                }
                ss << "    __auto_type vargs1 = *(struct LORE_VARG_ENTRY **) metadata[0];\n";
                if (!isVoid) {
                    ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(thunkRetType)
                       << ") *) ret;\n";
                }
                HTP.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "*ret_ref = ";
                }
                ss << HTP_IMPL.rep().name() << "(" << getCallArgsString(fixedArgCount)
                   << ", vargs1);\n";
                HTP.body().push_back(ID, ss.str());
            }
        }

        // HTP_IMPL
        if (!HTP_IMPL.rep().decl()) {
            // params
            HTP_IMPL.setRep(
                declRep(HTP_IMPL.rep().name(), thunkRetType, thunkArgTypes, fixedArgCount));

            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }

                ss << "    struct LORE_VARG_ENTRY argv1[] = {\n";
                for (int i = 0; i < fixedArgCount; ++i) {
                    ss << "        LORE_VARG(arg" << i + 1 << "),\n";
                }
                ss << "    };\n";
                ss << "    struct LORE_VARG_ENTRY va_ret = {.type = "
                   << (isVoid ? "0" : ("LORE_VARG_TYPE_ID(" + getTypeString(thunkRetType) + ")"))
                   << "};\n";
                HTP_IMPL.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    " << (va ? "Lore_VAFmtCallV" : "Lore_VAFmtCall") << "(LORELIB_API("
                   << thunk.name()
                   << "), sizeof(argv1) / sizeof(struct LORE_VARG_ENTRY), argv1, -1, vargs1, "
                      "&va_ret);\n";
                if (!isVoid) {
                    ss << "    ret = LORE_VARG_VALUE(" << getTypeString(thunkRetType)
                       << ", va_ret);\n";
                }
                HTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                HTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }
    }

    static void begin_GuestCallback(std::string ID, ThunkDefinition &thunk, int fmtIdx, int vargIdx,
                                    bool scanf, bool va = false) {
        auto helperFunc = scanf ? "Lore_ExtractScanFArgs" : "Lore_ExtractPrintFArgs";
        auto typeView = thunk.view();

        auto thunkRetType = typeView.returnType();
        auto thunkArgTypes = typeView.argTypes();
        bool isVoid = thunkRetType->isVoidType();
        int fixedArgCount = va ? (thunkArgTypes.size() - 1) : thunkArgTypes.size();

        LIST_FDS(thunk);

        // GTP
        if (!GTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                for (int i = 0; i < fixedArgCount; ++i) {
                    ss << "    __auto_type arg" << i + 1 //
                       << " = *(__typeof__(" << getTypeString(thunkArgTypes[i]) << ") *) args[" << i
                       << "];\n";
                }
                ss << "    __auto_type vargs1 = *(struct LORE_VARG_ENTRY **) metadata[0];\n";
                if (!isVoid) {
                    ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(thunkRetType)
                       << ") *) ret;\n";
                }
                GTP.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "*ret_ref = ";
                }
                ss << GTP_IMPL.rep().name() << "(callback" << getCallArgsString(fixedArgCount, true)
                   << ", vargs1);\n";
                GTP.body().push_back(ID, ss.str());
            }
        }

        // GTP_IMPL
        if (!GTP_IMPL.rep().decl()) {
            // params
            GTP_IMPL.setRep(
                declRep<true>(GTP_IMPL.rep().name(), thunkRetType, thunkArgTypes, fixedArgCount));

            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }

                ss << "    struct LORE_VARG_ENTRY argv1[] = {\n";
                for (int i = 0; i < fixedArgCount; ++i) {
                    ss << "        LORE_VARG(arg" << i + 1 << "),\n";
                }
                ss << "    };\n";
                ss << "    struct LORE_VARG_ENTRY va_ret = {.type = "
                   << (isVoid ? "0" : ("LORE_VARG_TYPE_ID(" + getTypeString(thunkRetType) + ")"))
                   << "};\n";
                GTP_IMPL.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    " << (va ? "Lore_VAFmtCallV" : "Lore_VAFmtCall")
                   << "(callback, sizeof(argv1) / sizeof(struct LORE_VARG_ENTRY), argv1, -1, "
                      "vargs1, &va_ret);\n";
                if (!isVoid) {
                    ss << "    ret = LORE_VARG_VALUE(" << getTypeString(thunkRetType)
                       << ", va_ret);\n";
                }
                GTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                GTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }

        // HTP
        if (!HTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                ss << "#ifdef LORELIB_CALLBACK_REPLACE\n"
                   << "    LORELIB_GET_LAST_GCB(callback);\n"
                   << "#else\n"
                   << "    void *callback = LORELIB_LAST_GCB;\n"
                   << "#endif\n";
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }

                ss << "    struct LORE_VARG_ENTRY vargs1[100];\n";
                if (va) {
                    ss << "    " << helperFunc << "(arg" << fmtIdx << ", arg" << vargIdx
                       << ", vargs1);\n";
                } else {
                    ss << "    {\n";
                    ss << "        va_list ap;\n";
                    ss << "        va_start(ap, arg" << vargIdx - 1 << ");\n";
                    ss << "        " << helperFunc << "(arg" << fmtIdx << ", ap, vargs1);\n";
                    ss << "        va_end(ap);\n";
                    ss << "    }\n";
                }
                HTP.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << HTP_IMPL.rep().name() << "(callback" + getCallArgsString(fixedArgCount, true)
                   << ", vargs1);\n";
                HTP.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                HTP.epilog().push_back(ID, ss.str());
            }
        }

        // HTP_IMPL
        if (!HTP_IMPL.rep().decl()) {
            // params
            HTP_IMPL.setRep(
                declRep(HTP_IMPL.rep().name(), thunkRetType, thunkArgTypes, fixedArgCount));

            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }

                ss << "    void *args[] = {\n";
                for (int i = 0; i < fixedArgCount; ++i) {
                    ss << "        (void *) &arg" << i + 1 << ",\n";
                }
                ss << "    };\n";

                ss << "    void *metadata[] = {\n";
                ss << "        (void *) &vargs1,\n";
                ss << "    };\n";

                HTP_IMPL.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    LORELIB_INVOKE_GCB(LORELIB_GCB(" << thunk.name() << "), callback, args, "
                   << (isVoid ? "NULL" : "&ret") << ", metadata);\n";
                HTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    return ret;\n";
                }
                HTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }
    }

    static void begin_HostCallback(std::string ID, ThunkDefinition &thunk, int fmtIdx, int vargIdx,
                                   bool scanf, bool va = false) {
        auto helperFunc = scanf ? "Lore_ExtractScanFArgs" : "Lore_ExtractPrintFArgs";
        auto typeView = thunk.view();

        auto thunkRetType = typeView.returnType();
        auto thunkArgTypes = typeView.argTypes();
        bool isVoid = thunkRetType->isVoidType();
        int fixedArgCount = va ? (thunkArgTypes.size() - 1) : thunkArgTypes.size();

        LIST_FDS(thunk);

        // GTP
        if (!GTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                ss << "#ifdef LORELIB_CALLBACK_REPLACE\n"
                   << "    LORELIB_GET_LAST_HCB(callback);\n"
                   << "#else\n"
                   << "    void *callback = LORELIB_LAST_HCB;\n"
                   << "#endif\n";
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }

                ss << "    struct LORE_VARG_ENTRY vargs1[100];\n";
                if (va) {
                    ss << "    " << helperFunc << "(arg" << fmtIdx << ", arg" << vargIdx
                       << ", vargs1);\n";
                } else {
                    ss << "    {\n";
                    ss << "        va_list ap;\n";
                    ss << "        va_start(ap, arg" << vargIdx - 1 << ");\n";
                    ss << "        " << helperFunc << "(arg" << fmtIdx << ", ap, vargs1);\n";
                    ss << "        va_end(ap);\n";
                    ss << "    }\n";
                }
                GTP.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << GTP_IMPL.rep().name() << "(callback" + getCallArgsString(fixedArgCount, true)
                   << ", vargs1);\n";
                GTP.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                GTP.epilog().push_back(ID, ss.str());
            }
        }

        // GTP_IMPL
        if (!GTP_IMPL.rep().decl()) {
            // params
            GTP_IMPL.setRep(
                declRep(GTP_IMPL.rep().name(), thunkRetType, thunkArgTypes, fixedArgCount));

            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }

                ss << "    void *args[] = {\n";
                for (int i = 0; i < fixedArgCount; ++i) {
                    ss << "        (void *) &arg" << i + 1 << ",\n";
                }
                ss << "    };\n";

                ss << "    void *metadata[] = {\n";
                ss << "        (void *) &vargs1,\n";
                ss << "    };\n";

                GTP_IMPL.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    LORELIB_INVOKE_HCB(LORELIB_HCB(" << thunk.name() << "), callback, args, "
                   << (isVoid ? "NULL" : "&ret") << ", metadata);\n";
                GTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    return ret;\n";
                }
                GTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }

        // HTP
        if (!HTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                for (int i = 0; i < fixedArgCount; ++i) {
                    ss << "    __auto_type arg" << i + 1 //
                       << " = *(__typeof__(" << getTypeString(thunkArgTypes[i]) << ") *) args[" << i
                       << "];\n";
                }
                ss << "    __auto_type vargs1 = *(struct LORE_VARG_ENTRY **) metadata[0];\n";
                if (!isVoid) {
                    ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(thunkRetType)
                       << ") *) ret;\n";
                }
                HTP.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "*ret_ref = ";
                }
                ss << HTP_IMPL.rep().name() << "(callback" << getCallArgsString(fixedArgCount, true)
                   << ", vargs1);\n";
                HTP.body().push_back(ID, ss.str());
            }
        }

        // HTP_IMPL
        if (!HTP_IMPL.rep().decl()) {
            // params
            {
                HTP_IMPL.setRep(declRep<true>(HTP_IMPL.rep().name(), thunkRetType, thunkArgTypes,
                                              fixedArgCount));
            }

            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }

                ss << "    struct LORE_VARG_ENTRY argv1[] = {\n";
                for (int i = 0; i < fixedArgCount; ++i) {
                    ss << "        LORE_VARG(arg" << i + 1 << "),\n";
                }
                ss << "    };\n";
                ss << "    struct LORE_VARG_ENTRY va_ret = {.type = "
                   << (isVoid ? "0" : ("LORE_VARG_TYPE_ID(" + getTypeString(thunkRetType) + ")"))
                   << "};\n";
                HTP_IMPL.prolog().push_front(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    " << (va ? "Lore_VAFmtCallV" : "Lore_VAFmtCall")
                   << "(callback, sizeof(argv1) / sizeof(struct LORE_VARG_ENTRY), argv1, -1, "
                      "vargs1, &va_ret);\n";
                if (!isVoid) {
                    ss << "    ret = LORE_VARG_VALUE(" << getTypeString(thunkRetType)
                       << ", va_ret);\n";
                }
                HTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                HTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }
    }

    /// \class PrintfPass
    class PrintfPass : public BuilderPass {
    public:
        PrintfPass() : BuilderPass(PT_LibcPrintf, "printf") {
        }
        ~PrintfPass() = default;

    public:
        bool test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const override;
        Error begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) override;
        Error end(ThunkDefinition &thunk) override;
    };

    bool PrintfPass::test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const {
        auto FD = thunk.declOrHint();

        if (FD) {
            // Check if the FD has the "printf" attribute
            if (auto Attr = FD->getAttr<clang::FormatAttr>(); Attr) {
                auto AttrType = Attr->getType();
                if (AttrType->getName() == "printf") {
                    auto formatIdx = Attr->getFormatIdx();
                    auto firstArg = Attr->getFirstArg();
                    if (formatIdx > 0 && firstArg > 0) {
                        args.clear();
                        args.push_back(formatIdx);
                        args.push_back(firstArg);
                        return true;
                    }
                }
            }
        }

        if (StringRef(thunk.name()).contains("printf") && thunk.qualType()->isFunctionProtoType()) {
            auto FPT = thunk.qualType()->getAs<clang::FunctionProtoType>();
            if (FPT->isVariadic() && FPT->getNumParams() > 0) {
                auto MaybeFmtParam = FPT->getParamType(FPT->getNumParams() - 1);
                if (isCharPointerType(MaybeFmtParam)) {
                    args.clear();
                    args.push_back(FPT->getNumParams());
                    args.push_back(FPT->getNumParams() + 1);
                    return true;
                }
            }
        }
        return false;
    }

    Error PrintfPass::begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) {
        if (args.size() != 2) {
            return createStringError(std::errc::invalid_argument, "Invalid argument number");
        }

        auto FirstArg = args[0].toInt();
        auto SecondArg = args[1].toInt();

        std::string ID = "builder/printf";
        switch (thunk.type()) {
            case ThunkDefinition::GuestFunctionThunk:
                begin_GuestFunction(ID, thunk, FirstArg, SecondArg, false);
                break;
            case ThunkDefinition::GuestCallbackThunk:
                begin_GuestCallback(ID, thunk, FirstArg, SecondArg, false);
                break;
            case ThunkDefinition::HostCallbackThunk:
                begin_HostCallback(ID, thunk, FirstArg, SecondArg, false);
                break;
            default:
                break;
        }
        return Error::success();
    }

    Error PrintfPass::end(ThunkDefinition &thunk) {
        return Error::success();
    }

    /// \class VPrintfPass
    class VPrintfPass : public BuilderPass {
    public:
        VPrintfPass() : BuilderPass(PT_LibcVPrintf, "vprintf") {
        }
        ~VPrintfPass() = default;

    public:
        bool test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const override;
        Error begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) override;
        Error end(ThunkDefinition &thunk) override;
    };

    bool VPrintfPass::test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const {
        auto FD = thunk.declOrHint();

        if (FD) {
            // Check if the FD has the "printf" attribute
            if (auto Attr = FD->getAttr<clang::FormatAttr>(); Attr) {
                auto AttrType = Attr->getType();
                if (AttrType->getName() == "printf") {
                    auto FormatIdx = Attr->getFormatIdx();
                    auto FirstArg = Attr->getFirstArg();
                    if (FormatIdx > 0 && FirstArg == 0) {
                        args.clear();
                        args.push_back(FormatIdx);
                        args.push_back(FD->param_size());
                        return true;
                    }
                }
            }
        }

        if (StringRef(thunk.name()).contains("printf") && thunk.qualType()->isFunctionProtoType()) {
            auto FPT = thunk.qualType()->getAs<clang::FunctionProtoType>();
            if (FPT->getNumParams() > 1) {
                auto MaybeVaListParam = FPT->getParamType(FPT->getNumParams() - 1);
                if (getTypeString(MaybeVaListParam) == "va_list") {
                    auto MaybeFmtParam = FPT->getParamType(FPT->getNumParams() - 2);
                    if (isCharPointerType(MaybeFmtParam)) {
                        args.clear();
                        args.push_back(FPT->getNumParams() - 1);
                        args.push_back(FPT->getNumParams());
                        return true;
                    }
                }
            }
        }
        return false;
    }

    Error VPrintfPass::begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) {
        if (args.size() != 2) {
            return createStringError(std::errc::invalid_argument, "Invalid argument number");
        }

        auto FirstArg = args[0].toInt();
        auto SecondArg = args[1].toInt();

        std::string ID = "builder/vprintf";
        switch (thunk.type()) {
            case ThunkDefinition::GuestFunctionThunk:
                begin_GuestFunction(ID, thunk, FirstArg, SecondArg, false, true);
                break;
            case ThunkDefinition::GuestCallbackThunk:
                begin_GuestCallback(ID, thunk, FirstArg, SecondArg, false, true);
                break;
            case ThunkDefinition::HostCallbackThunk:
                begin_HostCallback(ID, thunk, FirstArg, SecondArg, false, true);
                break;
            default:
                break;
        }
        return Error::success();
    }

    Error VPrintfPass::end(ThunkDefinition &thunk) {
        return Error::success();
    }

    /// \class ScanfPass
    class ScanfPass : public BuilderPass {
    public:
        ScanfPass() : BuilderPass(PT_LibcScanf, "scanf") {
        }
        ~ScanfPass() = default;

    public:
        bool test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const override;
        Error begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) override;
        Error end(ThunkDefinition &thunk) override;
    };

    bool ScanfPass::test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const {
        auto FD = thunk.declOrHint();

        if (FD) {
            // Check if the FD has the "scanf" attribute
            if (auto Attr = FD->getAttr<clang::FormatAttr>(); Attr) {
                auto AttrType = Attr->getType();
                if (AttrType->getName() == "scanf") {
                    auto FormatIdx = Attr->getFormatIdx();
                    auto FirstArg = Attr->getFirstArg();
                    if (FormatIdx > 0 && FirstArg > 0) {
                        args.clear();
                        args.push_back(FormatIdx);
                        args.push_back(FirstArg);
                        return true;
                    }
                }
            }
        }

        if (StringRef(thunk.name()).contains("scanf") && thunk.qualType()->isFunctionProtoType()) {
            auto FPT = thunk.qualType()->getAs<clang::FunctionProtoType>();
            if (FPT->isVariadic() && FPT->getNumParams() > 0) {
                auto MaybeFmtParam = FPT->getParamType(FPT->getNumParams() - 1);
                if (isCharPointerType(MaybeFmtParam)) {
                    args.clear();
                    args.push_back(FPT->getNumParams());
                    args.push_back((FPT->getNumParams() + 1));
                    return true;
                }
            }
        }
        return false;
    }

    Error ScanfPass::begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) {
        if (args.size() != 2) {
            return createStringError(std::errc::invalid_argument, "Invalid argument number");
        }

        auto FirstArg = args[0].toInt();
        auto SecondArg = args[1].toInt();

        std::string ID = "builder/scanf";
        switch (thunk.type()) {
            case ThunkDefinition::GuestFunctionThunk:
                begin_GuestFunction(ID, thunk, FirstArg, SecondArg, true);
                break;
            case ThunkDefinition::GuestCallbackThunk:
                begin_GuestCallback(ID, thunk, FirstArg, SecondArg, true);
                break;
            case ThunkDefinition::HostCallbackThunk:
                begin_HostCallback(ID, thunk, FirstArg, SecondArg, true);
                break;
            default:
                break;
        }
        return Error::success();
    }

    Error ScanfPass::end(ThunkDefinition &thunk) {
        return Error::success();
    }

    /// \class VScanfPass
    class VScanfPass : public BuilderPass {
    public:
        VScanfPass() : BuilderPass(PT_LibcVScanf, "vscanf") {
        }
        ~VScanfPass() = default;

    public:
        bool test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const override;
        Error begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) override;
        Error end(ThunkDefinition &thunk) override;
    };

    bool VScanfPass::test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const {
        auto FD = thunk.declOrHint();

        if (FD) {
            // Check if the FD has the "scanf" attribute
            if (auto Attr = FD->getAttr<clang::FormatAttr>(); Attr) {
                auto AttrType = Attr->getType();
                if (AttrType->getName() == "scanf") {
                    auto FormatIdx = Attr->getFormatIdx();
                    auto FirstArg = Attr->getFirstArg();
                    if (FormatIdx > 0 && FirstArg == 0) {
                        args.clear();
                        args.push_back(FormatIdx);
                        args.push_back(FD->param_size());
                        return true;
                    }
                }
            }
        }

        if (StringRef(thunk.name()).contains("scanf") && thunk.qualType()->isFunctionProtoType()) {
            auto FPT = thunk.qualType()->getAs<clang::FunctionProtoType>();
            if (FPT->getNumParams() > 1) {
                auto MaybeVaListParam = FPT->getParamType(FPT->getNumParams() - 1);
                if (getTypeString(MaybeVaListParam) == "va_list") {
                    auto MaybeFmtParam = FPT->getParamType(FPT->getNumParams() - 2);
                    if (isCharPointerType(MaybeFmtParam)) {
                        args.clear();
                        args.push_back(FPT->getNumParams() - 1);
                        args.push_back(FPT->getNumParams());
                        return true;
                    }
                }
            }
        }
        return false;
    }

    Error VScanfPass::begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) {
        if (args.size() != 2) {
            return createStringError(std::errc::invalid_argument, "Invalid argument number");
        }

        auto FirstArg = args[0].toInt();
        auto SecondArg = args[1].toInt();

        std::string ID = "builder/vscanf";
        switch (thunk.type()) {
            case ThunkDefinition::GuestFunctionThunk:
                begin_GuestFunction(ID, thunk, FirstArg, SecondArg, true, true);
                break;
            case ThunkDefinition::GuestCallbackThunk:
                begin_GuestCallback(ID, thunk, FirstArg, SecondArg, true, true);
                break;
            case ThunkDefinition::HostCallbackThunk:
                begin_HostCallback(ID, thunk, FirstArg, SecondArg, true, true);
                break;
            default:
                break;
        }
        return Error::success();
    }

    Error VScanfPass::end(ThunkDefinition &thunk) {
        return Error::success();
    }

    /// \brief Pass registrations.
    static PassRegistration<PrintfPass> PR_printf;
    static PassRegistration<VPrintfPass> PR_vprintf;
    static PassRegistration<ScanfPass> PR_scanf;
    static PassRegistration<VScanfPass> PR_vscanf;

}