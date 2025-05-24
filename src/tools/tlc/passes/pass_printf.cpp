#include <sstream>

#include <clang/AST/AST.h>
#include <clang/AST/Attr.h>

#include "core/pass.h"

#include "common.h"

using namespace clang;

namespace TLC {

    static void start_Normal(ApiSource &Source, int FirstArg, bool scanf, bool va = false) {
        auto HelperFunc = scanf ? "Lore_ExtractScanFArgs" : "Lore_ExtractPrintFArgs";

        auto ApiReturnType = Source.getApiReturnType();
        auto ApiArgTypes = Source.getApiArgumentTypes();

        bool isVoid = ApiReturnType->isVoidType();

        auto &GTP = Source.getFunctionSource(FunctionSource::GTP);
        auto &GTP_IMPL = Source.getFunctionSource(FunctionSource::GTP_IMPL);
        auto &HTP = Source.getFunctionSource(FunctionSource::HTP);
        auto &HTP_IMPL = Source.getFunctionSource(FunctionSource::HTP_IMPL);

        // GTP
        if (!GTP.getFunctionDecl()) {
            // prolog
            {
                std::stringstream ss;
                ss << "    struct LORE_VARG_ENTRY vargs1[100];\n";
                if (va) {
                    ss << "    " << HelperFunc << "(arg" << FirstArg << ", arg" << FirstArg + 1 << ", vargs1);\n";
                } else {
                    ss << "    {\n";
                    ss << "        va_list ap;\n";
                    ss << "        va_start(ap, arg" << FirstArg << ");\n";
                    ss << "        " << HelperFunc << "(arg" << FirstArg << ", ap, vargs1);\n";
                    ss << "        va_end(ap);\n";
                    ss << "    }\n";
                }
                GTP.prependForward(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << GTP_IMPL.getName().str() << "(" + getCallArgsString(ApiArgTypes.size()) << ", vargs1);\n";
                GTP.setBody(ss.str());
            }
        }

        // GTP_IMPL
        if (!GTP_IMPL.getFunctionDecl()) {
            // params
            {
                GTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                SmallVector<FunctionSource::Param> Args;
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                }
                Args.push_back({"struct LORE_VARG_ENTRY *", "vargs1", {}});
                GTP_IMPL.setArguments(Args);
            }

            // prolog
            {
                std::stringstream ss;

                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                }

                ss << "    void *args[] = {\n";
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    ss << "        (void *) &arg" << i + 1 << ",\n";
                }
                ss << "    };\n";

                ss << "    void *metadata[] = {\n";
                ss << "        (void *) &vargs1,\n";
                ss << "    };\n";

                GTP_IMPL.setProlog(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    LORELIB_INVOKE_HTP(LORELIB_HTP(" << Source.getName().str() << "), args, "
                   << (isVoid ? "NULL" : "&ret") << ", metadata);\n";
                GTP_IMPL.setBody(ss.str());
            }

            // epilog
            {

                std::stringstream ss;
                if (!isVoid) {
                    ss << "    return ret;\n";
                }
                GTP_IMPL.setEpilog(ss.str());
            }
        }

        // HTP
        if (!HTP.getFunctionDecl()) {
            // prolog
            {
                std::stringstream ss;
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    ss << "    __auto_type arg" << i + 1 //
                       << " = *(__typeof__(" << getTypeString(ApiArgTypes[i]) << ") *) args[" << i << "];\n";
                }
                ss << "    __auto_type vargs1 = (struct LORE_VARG_ENTRY *) metadata[0];\n";
                if (!isVoid) {
                    ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(ApiReturnType) << ") *) ret;\n";
                }
                HTP.setProlog(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "*ret_ref = ";
                }
                ss << HTP_IMPL.getName().str() << "(" << getCallArgsString(ApiArgTypes.size()) << ", vargs1);\n";
                HTP.setBody(ss.str());
            }
        }

        // HTP_IMPL
        if (!HTP_IMPL.getFunctionDecl()) {
            // params
            {
                HTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                SmallVector<FunctionSource::Param> Args;
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                }
                Args.push_back({"struct LORE_VARG_ENTRY *", "vargs1", {}});
                HTP_IMPL.setArguments(Args);
            }

            // prolog
            {
                std::stringstream ss;
                ss << "    struct LORE_VARG_ENTRY argv1[] = {\n";
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    ss << "        LORE_VARG(arg" << i + 1 << "),\n";
                }
                ss << "    };\n";
                ss << "    struct LORE_VARG_ENTRY va_ret = {.type = "
                   << (isVoid ? "0" : ("LORE_VARG_TYPE_ID(" + getTypeString(ApiReturnType) + ")")) << "};\n";
                HTP_IMPL.prependForward(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    " << (va ? "Lore_VAFmtCallV" : "Lore_VAFmtCall") << "(LORELIB_API(" << Source.getName().str()
                   << "), sizeof(argv1) / sizeof(struct LORE_VARG_ENTRY), argv1, -1, vargs1, &va_ret);\n";
                if (!isVoid) {
                    ss << "    ret = LORE_VARG_VALUE(" << getTypeString(ApiReturnType) << ", va_ret);\n";
                }
                HTP_IMPL.setBody(ss.str());
            }
        }
    }

    static void start_GuestCallback(ApiSource &Source, int FirstArg, bool scanf, bool va = false) {
        auto HelperFunc = scanf ? "Lore_ExtractScanFArgs" : "Lore_ExtractPrintFArgs";

        auto ApiReturnType = Source.getApiReturnType();
        auto ApiArgTypes = Source.getApiArgumentTypes();

        bool isVoid = ApiReturnType->isVoidType();

        auto &GTP = Source.getFunctionSource(FunctionSource::GTP);
        auto &GTP_IMPL = Source.getFunctionSource(FunctionSource::GTP_IMPL);
        auto &HTP = Source.getFunctionSource(FunctionSource::HTP);
        auto &HTP_IMPL = Source.getFunctionSource(FunctionSource::HTP_IMPL);

        // GTP
        if (!GTP.getFunctionDecl()) {
            // prolog
            {
                std::stringstream ss;
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    ss << "    __auto_type arg" << i + 1 //
                       << " = *(__typeof__(" << getTypeString(ApiArgTypes[i]) << ") *) args[" << i << "];\n";
                }
                ss << "    __auto_type vargs1 = (struct LORE_VARG_ENTRY *) metadata[0];\n";
                if (!isVoid) {
                    ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(ApiReturnType) << ") *) ret;\n";
                }
                GTP.setProlog(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "*ret_ref = ";
                }
                ss << GTP_IMPL.getName().str() << "(callback" << getCallArgsString(ApiArgTypes.size(), true)
                   << ", vargs1);\n";
                GTP.setBody(ss.str());
            }
        }

        // GTP_IMPL
        if (!GTP_IMPL.getFunctionDecl()) {
            // params
            {
                GTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                SmallVector<FunctionSource::Param> Args;
                Args.push_back({"void *", "callback", {}});
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                }
                Args.push_back({"struct LORE_VARG_ENTRY *", "vargs1", {}});
                GTP_IMPL.setArguments(Args);
            }

            // prolog
            {
                std::stringstream ss;
                ss << "    struct LORE_VARG_ENTRY argv1[] = {\n";
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    ss << "        LORE_VARG(arg" << i + 1 << "),\n";
                }
                ss << "    };\n";
                ss << "    struct LORE_VARG_ENTRY va_ret = {.type = "
                   << (isVoid ? "0" : ("LORE_VARG_TYPE_ID(" + getTypeString(ApiReturnType) + ")")) << "};\n";
                GTP_IMPL.prependForward(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    " << (va ? "Lore_VAFmtCallV" : "Lore_VAFmtCall")
                   << "(callback, sizeof(argv1) / sizeof(struct LORE_VARG_ENTRY), argv1, -1, vargs1, &va_ret);\n";
                if (!isVoid) {
                    ss << "    ret = LORE_VARG_VALUE(" << getTypeString(ApiReturnType) << ", va_ret);\n";
                }
                GTP_IMPL.setBody(ss.str());
            }
        }

        // HTP
        if (!HTP.getFunctionDecl()) {
            // prolog
            {
                std::stringstream ss;
                ss << "    struct LORE_VARG_ENTRY vargs1[100];\n";
                if (va) {
                    ss << "    " << HelperFunc << "(arg" << FirstArg << ", arg" << FirstArg + 1 << ", vargs1);\n";
                } else {
                    ss << "    {\n";
                    ss << "        va_list ap;\n";
                    ss << "        va_start(ap, arg" << FirstArg << ");\n";
                    ss << "        " << HelperFunc << "(arg" << FirstArg << ", ap, vargs1);\n";
                    ss << "        va_end(ap);\n";
                    ss << "    }\n";
                }
                HTP.prependForward(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << HTP_IMPL.getName().str() << "(" + getCallArgsString(ApiArgTypes.size()) << ", vargs1);\n";
                HTP.setBody(ss.str());
            }
        }

        // HTP_IMPL
        if (!HTP_IMPL.getFunctionDecl()) {
            // params
            {
                HTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                SmallVector<FunctionSource::Param> Args;
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                }
                Args.push_back({"struct LORE_VARG_ENTRY *", "vargs1", {}});
                HTP_IMPL.setArguments(Args);
            }

            // prolog
            {
                std::stringstream ss;

                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                }

                ss << "    void *args[] = {\n";
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    ss << "        (void *) &arg" << i + 1 << ",\n";
                }
                ss << "    };\n";

                ss << "    void *metadata[] = {\n";
                ss << "        (void *) &vargs1,\n";
                ss << "    };\n";

                HTP_IMPL.setProlog(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    LORELIB_INVOKE_GCB(LORELIB_GCB(" << Source.getName().str() << "), LORELIB_LAST_GCB, args, "
                   << (isVoid ? "NULL" : "&ret") << ", metadata);\n";
                HTP_IMPL.setBody(ss.str());
            }

            // epilog
            {

                std::stringstream ss;
                if (!isVoid) {
                    ss << "    return ret;\n";
                }
                HTP_IMPL.setEpilog(ss.str());
            }
        }
    }

    static void start_HostCallback(ApiSource &Source, int FirstArg, bool scanf, bool va = false) {
        auto HelperFunc = scanf ? "Lore_ExtractScanFArgs" : "Lore_ExtractPrintFArgs";

        auto ApiReturnType = Source.getApiReturnType();
        auto ApiArgTypes = Source.getApiArgumentTypes();

        bool isVoid = ApiReturnType->isVoidType();

        auto &GTP = Source.getFunctionSource(FunctionSource::GTP);
        auto &GTP_IMPL = Source.getFunctionSource(FunctionSource::GTP_IMPL);
        auto &HTP = Source.getFunctionSource(FunctionSource::HTP);
        auto &HTP_IMPL = Source.getFunctionSource(FunctionSource::HTP_IMPL);

        // GTP
        if (!GTP.getFunctionDecl()) {
            // prolog
            {
                std::stringstream ss;
                ss << "    struct LORE_VARG_ENTRY vargs1[100];\n";
                if (va) {
                    ss << "    " << HelperFunc << "(arg" << FirstArg << ", arg" << FirstArg + 1 << ", vargs1);\n";
                } else {
                    ss << "    {\n";
                    ss << "        va_list ap;\n";
                    ss << "        va_start(ap, arg" << FirstArg << ");\n";
                    ss << "        " << HelperFunc << "(arg" << FirstArg << ", ap, vargs1);\n";
                    ss << "        va_end(ap);\n";
                    ss << "    }\n";
                }
                GTP.prependForward(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << GTP_IMPL.getName().str() << "(" + getCallArgsString(ApiArgTypes.size()) << ", vargs1);\n";
                GTP.setBody(ss.str());
            }
        }

        // GTP_IMPL
        if (!GTP_IMPL.getFunctionDecl()) {
            // params
            {
                GTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                SmallVector<FunctionSource::Param> Args;
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                }
                Args.push_back({"struct LORE_VARG_ENTRY *", "vargs1", {}});
                GTP_IMPL.setArguments(Args);
            }

            // prolog
            {
                std::stringstream ss;

                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                }

                ss << "    void *args[] = {\n";
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    ss << "        (void *) &arg" << i + 1 << ",\n";
                }
                ss << "    };\n";

                ss << "    void *metadata[] = {\n";
                ss << "        (void *) &vargs1,\n";
                ss << "    };\n";

                GTP_IMPL.setProlog(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    LORELIB_INVOKE_HCB(LORELIB_HCB(" << Source.getName().str() << "), LORELIB_LAST_HCB, args, "
                   << (isVoid ? "NULL" : "&ret") << ", metadata);\n";
                GTP_IMPL.setBody(ss.str());
            }

            // epilog
            {

                std::stringstream ss;
                if (!isVoid) {
                    ss << "    return ret;\n";
                }
                GTP_IMPL.setEpilog(ss.str());
            }
        }

        // HTP
        if (!HTP.getFunctionDecl()) {
            // prolog
            {
                std::stringstream ss;
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    ss << "    __auto_type arg" << i + 1 //
                       << " = *(__typeof__(" << getTypeString(ApiArgTypes[i]) << ") *) args[" << i << "];\n";
                }
                ss << "    __auto_type vargs1 = (struct LORE_VARG_ENTRY *) metadata[0];\n";
                if (!isVoid) {
                    ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(ApiReturnType) << ") *) ret;\n";
                }
                HTP.setProlog(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "*ret_ref = ";
                }
                ss << HTP_IMPL.getName().str() << "(callback" << getCallArgsString(ApiArgTypes.size(), true)
                   << ", vargs1);\n";
                HTP.setBody(ss.str());
            }
        }

        // HTP_IMPL
        if (!HTP_IMPL.getFunctionDecl()) {
            // params
            {
                HTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                SmallVector<FunctionSource::Param> Args;
                Args.push_back({"void *", "callback", {}});
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                }
                Args.push_back({"struct LORE_VARG_ENTRY *", "vargs1", {}});
                HTP_IMPL.setArguments(Args);
            }

            // prolog
            {
                std::stringstream ss;
                ss << "    struct LORE_VARG_ENTRY argv1[] = {\n";
                for (int i = 0; i < ApiArgTypes.size(); ++i) {
                    ss << "        LORE_VARG(arg" << i + 1 << "),\n";
                }
                ss << "    };\n";
                ss << "    struct LORE_VARG_ENTRY va_ret = {.type = "
                   << (isVoid ? "0" : ("LORE_VARG_TYPE_ID(" + getTypeString(ApiReturnType) + ")")) << "};\n";
                HTP_IMPL.prependForward(ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    " << (va ? "Lore_VAFmtCallV" : "Lore_VAFmtCall")
                   << "(callback, sizeof(argv1) / sizeof(struct LORE_VARG_ENTRY), argv1, -1, vargs1, &va_ret);\n";
                if (!isVoid) {
                    ss << "    ret = LORE_VARG_VALUE(" << getTypeString(ApiReturnType) << ", va_ret);\n";
                }
                HTP_IMPL.setBody(ss.str());
            }
        }
    }

    class Pass_printf : public Pass {
    public:
        Pass_printf() : Pass("printf") {
        }

    public:
        bool test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const override;

        bool start(ApiSource &api, const llvm::ArrayRef<std::string> &args) override;
        bool end() override;
    };

    bool Pass_printf::test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const {
        auto FD = api.getOrgFunctionDecl();

        if (FD) {
            // Check if the FD has the "printf" attribute
            if (auto Attr = FD->getAttr<clang::FormatAttr>(); Attr) {
                auto AttrType = Attr->getType();
                if (AttrType->getName() == "printf") {
                    args->clear();
                    args->push_back(std::to_string(Attr->getFormatIdx()));
                    args->push_back(std::to_string(Attr->getFirstArg()));
                    *priority = BeginPriority;
                    return true;
                }
            }
        }

        if (api.getName().contains("printf") && api.getType()->isFunctionProtoType()) {
            auto FPT = api.getType()->getAs<clang::FunctionProtoType>();
            if (FPT->isVariadic() && FPT->getNumParams() > 0) {
                auto MaybeFmtParam = FPT->getParamType(FPT->getNumParams() - 1);
                if (isCharPointerType(MaybeFmtParam)) {
                    args->clear();
                    args->push_back(std::to_string(FPT->getNumParams()));
                    args->push_back(std::to_string(FPT->getNumParams() + 1));
                    *priority = BeginPriority;
                    return true;
                }
            }
        }
        return false;
    }

    bool Pass_printf::start(ApiSource &api, const llvm::ArrayRef<std::string> &args) {
        if (args.size() != 2) {
            return false;
        }

        auto FirstArg = std::atoi(args[0].c_str());
        auto SecondArg = std::atoi(args[1].c_str());

        (void) SecondArg;

        switch (api.getApiType()) {
            case ApiSource::Normal:
                start_Normal(api, FirstArg, false);
                break;
            case ApiSource::GuestCallback:
                start_GuestCallback(api, FirstArg, false);
                break;
            case ApiSource::HostCallback:
                start_HostCallback(api, FirstArg, false);
                break;
            default:
                return false;
        }
        return true;
    }

    bool Pass_printf::end() {
        return true;
    }

    class Pass_vprintf : public Pass {
    public:
        Pass_vprintf() : Pass("vprintf") {
        }

    public:
        bool test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const override;

        bool start(ApiSource &api, const llvm::ArrayRef<std::string> &args) override;
        bool end() override;
    };

    bool Pass_vprintf::test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const {
        if (api.getName().contains("printf") && api.getType()->isFunctionProtoType()) {
            auto FPT = api.getType()->getAs<clang::FunctionProtoType>();
            if (FPT->getNumParams() > 1) {
                auto MaybeVaListParam = FPT->getParamType(FPT->getNumParams() - 1);
                if (getTypeString(MaybeVaListParam) == "va_list") {
                    auto MaybeFmtParam = FPT->getParamType(FPT->getNumParams() - 2);
                    if (isCharPointerType(MaybeFmtParam)) {
                        args->clear();
                        args->push_back(std::to_string(FPT->getNumParams() - 1));
                        args->push_back(std::to_string(FPT->getNumParams()));
                        *priority = BeginPriority;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Pass_vprintf::start(ApiSource &api, const llvm::ArrayRef<std::string> &args) {
        if (args.size() != 2) {
            return false;
        }

        auto FirstArg = std::atoi(args[0].c_str());
        auto SecondArg = std::atoi(args[1].c_str());

        (void) SecondArg;

        switch (api.getApiType()) {
            case ApiSource::Normal:
                start_Normal(api, FirstArg, false, true);
                break;
            case ApiSource::GuestCallback:
                start_GuestCallback(api, FirstArg, false, true);
                break;
            case ApiSource::HostCallback:
                start_HostCallback(api, FirstArg, false, true);
                break;
            default:
                return false;
        }
        return true;
    }

    bool Pass_vprintf::end() {
        return true;
    }

    class Pass_scanf : public Pass {
    public:
        Pass_scanf() : Pass("scanf") {
        }

    public:
        bool test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const override;

        bool start(ApiSource &api, const llvm::ArrayRef<std::string> &args) override;
        bool end() override;
    };

    bool Pass_scanf::test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const {
        auto FD = api.getOrgFunctionDecl();

        if (FD) {
            // Check if the FD has the "scanf" attribute
            if (auto Attr = FD->getAttr<clang::FormatAttr>(); Attr) {
                auto AttrType = Attr->getType();
                if (AttrType->getName() == "scanf") {
                    args->clear();
                    args->push_back(std::to_string(Attr->getFormatIdx()));
                    args->push_back(std::to_string(Attr->getFirstArg()));
                    *priority = BeginPriority;
                    return true;
                }
            }
        }

        if (api.getName().contains("scanf") && api.getType()->isFunctionProtoType()) {
            auto FPT = api.getType()->getAs<clang::FunctionProtoType>();
            if (FPT->isVariadic() && FPT->getNumParams() > 0) {
                auto MaybeFmtParam = FPT->getParamType(FPT->getNumParams() - 1);
                if (isCharPointerType(MaybeFmtParam)) {
                    args->clear();
                    args->push_back(std::to_string(FPT->getNumParams()));
                    args->push_back(std::to_string(FPT->getNumParams() + 1));
                    *priority = BeginPriority;
                    return true;
                }
            }
        }
        return false;
    }

    bool Pass_scanf::start(ApiSource &api, const llvm::ArrayRef<std::string> &args) {
        if (args.size() != 2) {
            return false;
        }

        auto FirstArg = std::atoi(args[0].c_str());
        auto SecondArg = std::atoi(args[1].c_str());

        (void) SecondArg;

        switch (api.getApiType()) {
            case ApiSource::Normal:
                start_Normal(api, FirstArg, true);
                break;
            case ApiSource::GuestCallback:
                start_GuestCallback(api, FirstArg, true);
                break;
            case ApiSource::HostCallback:
                start_HostCallback(api, FirstArg, true);
                break;
            default:
                return false;
        }
        return true;
    }

    bool Pass_scanf::end() {
        return true;
    }

    class Pass_vscanf : public Pass {
    public:
        Pass_vscanf() : Pass("vscanf") {
        }

    public:
        bool test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const override;

        bool start(ApiSource &api, const llvm::ArrayRef<std::string> &args) override;
        bool end() override;
    };

    bool Pass_vscanf::test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const {
        if (api.getName().contains("scanf") && api.getType()->isFunctionProtoType()) {
            auto FPT = api.getType()->getAs<clang::FunctionProtoType>();
            if (FPT->getNumParams() > 1) {
                auto MaybeVaListParam = FPT->getParamType(FPT->getNumParams() - 1);
                if (getTypeString(MaybeVaListParam) == "va_list") {
                    auto MaybeFmtParam = FPT->getParamType(FPT->getNumParams() - 2);
                    if (isCharPointerType(MaybeFmtParam)) {
                        args->clear();
                        args->push_back(std::to_string(FPT->getNumParams() - 1));
                        args->push_back(std::to_string(FPT->getNumParams()));
                        *priority = BeginPriority;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Pass_vscanf::start(ApiSource &api, const llvm::ArrayRef<std::string> &args) {
        if (args.size() != 2) {
            return false;
        }

        auto FirstArg = std::atoi(args[0].c_str());
        auto SecondArg = std::atoi(args[1].c_str());

        (void) SecondArg;

        switch (api.getApiType()) {
            case ApiSource::Normal:
                start_Normal(api, FirstArg, true, true);
                break;
            case ApiSource::GuestCallback:
                start_GuestCallback(api, FirstArg, true, true);
                break;
            case ApiSource::HostCallback:
                start_HostCallback(api, FirstArg, true, true);
                break;
            default:
                return false;
        }
        return true;
    }

    bool Pass_vscanf::end() {
        return true;
    }

}

static TLC::Pass_printf pass_printf;
static TLC::Pass_vprintf pass_vprintf;

static TLC::Pass_scanf pass_scanf;
static TLC::Pass_vscanf pass_vscanf;