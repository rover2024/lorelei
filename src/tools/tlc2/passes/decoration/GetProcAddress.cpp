#include "pass.h"

#include <sstream>

#include "thunkdefinition.h"
#include "common.h"

using namespace clang;
using namespace llvm;

namespace TLC {

    /// \brief GetProcAddress pass.
    ///
    /// \example Attribute format: "@GetProcAddress:<name_index>"
    /// \code
    ///     void *XXX_GetProcAddress(int a, const char *name)
    ///         __attribute__((annotate("@GetProcAddress:2")));
    /// \endcode

    class GetProcAddressPass : public DecorationPass {
    public:
        GetProcAddressPass() : DecorationPass(PT_GetProcAddress, "GetProcAddress") {
        }
        ~GetProcAddressPass() = default;

    public:
        bool test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const override;
        Error begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) override;
        Error end(ThunkDefinition &thunk) override;
    };

    bool GetProcAddressPass::test(const ThunkDefinition &thunk,
                                  SmallVectorImpl<IntOrString> &args) const {
        if (StringRef(thunk.name()).contains("GetProcAddress") &&
            thunk.qualType()->isFunctionProtoType()) {
            auto FPT = thunk.qualType()->getAs<clang::FunctionProtoType>();
            if (FPT->getNumParams() > 0) {
                auto MaybeNameType = FPT->getParamType(FPT->getNumParams() - 1);
                if (isCharPointerType(MaybeNameType)) {
                    args.clear();
                    args.push_back(FPT->getNumParams());
                    return true;
                }
            }
        }
        return false;
    }

    Error GetProcAddressPass::begin(ThunkDefinition &thunk,
                                    const SmallVectorImpl<IntOrString> &args) {
        if (args.size() != 1) {
            return createStringError(std::errc::invalid_argument, "Invalid argument number");
        }
        auto &GTP = thunk.function(FunctionDefinition::GTP);
        {
            std::stringstream ss;
            ss << "    ret = Lore_ConvertHostProcAddress(arg" << args[0].toInt() << ", ret);\n";
            GTP.bodyBackward().push_back("GetProcAddress", ss.str());
        }
        return Error::success();
    }

    Error GetProcAddressPass::end(ThunkDefinition &thunk) {
        return Error::success();
    }

    /// \brief Pass registrations.
    static PassRegistration<GetProcAddressPass> PR_GetProcAddress;

}