#include <sstream>

#include <clang/AST/AST.h>
#include <clang/AST/Attr.h>

#include "core/pass.h"

#include "common.h"

namespace TLC {

    class Pass_GetProcAddress : public Pass {
    public:
        Pass_GetProcAddress();
        ~Pass_GetProcAddress();

    public:
        bool test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const override;

        bool start(ApiSource &api, const llvm::ArrayRef<std::string> &args) override;
        bool end() override;
    };

    Pass_GetProcAddress::Pass_GetProcAddress() : Pass("GetProcAddress") {
    }

    Pass_GetProcAddress::~Pass_GetProcAddress() {
    }

    bool Pass_GetProcAddress::test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args,
                                   int *priority) const {
        if (api.getName().contains("GetProcAddress") && api.getType()->isFunctionProtoType()) {
            auto FPT = api.getType()->getAs<clang::FunctionProtoType>();
            if (FPT->getNumParams() > 0) {
                auto MaybeNameType = FPT->getParamType(FPT->getNumParams() - 1);
                if (isCharPointerType(MaybeNameType)) {
                    args->clear();
                    args->push_back(std::to_string(FPT->getNumParams()));
                    *priority = NoPriority;
                    return true;
                }
            }
        }
        return false;
    }

    bool Pass_GetProcAddress::start(ApiSource &api, const llvm::ArrayRef<std::string> &args) {
        if (args.size() != 1) {
            return false;
        }

        auto NameArg = std::atoi(args[0].c_str());
        
        auto &GTP = api.getFunctionSource(FunctionSource::GTP);
        {
            std::stringstream ss;
            ss << "    ret = Lore_ConvertHostProcAddress(arg" << NameArg << ", ret);\n";
            GTP.appendBackward(ss.str());
        }

        return true;
    }

    bool Pass_GetProcAddress::end() {
        return true;
    }

}

static TLC::Pass_GetProcAddress pass_GetProcAddress;