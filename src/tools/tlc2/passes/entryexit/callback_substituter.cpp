#include "pass.h"

using namespace clang;
using namespace llvm;

namespace TLC {

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
        return false;
    }

    Error CallbackSubstituterPass::begin(ThunkDefinition &thunk,
                                         const SmallVectorImpl<IntOrString> &args) {
        return Error::success();
    }

    Error CallbackSubstituterPass::end(ThunkDefinition &thunk) {
        return Error::success();
    }


    /// \brief Pass registrations.
    static PassRegistration<CallbackSubstituterPass> PR_callback_substituter;

}