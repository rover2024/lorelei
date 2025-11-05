#ifndef MYPASS_H
#define MYPASS_H

#include <lorelei/TLC/Core/Pass.h>

#include <lorelei/TLCMeta/MetaPass.h>

namespace plugin {

    class MyPass : public TLC::Pass {
    public:
        MyPass() : TLC::Pass(Misc, lorethunk::MetaPass::UserID + 1) {
        }

        ~MyPass() = default;

    public:
        std::string name() const override {
            return "plugin";
        }

        llvm::Error beginHandleProc(TLC::ProcContext &proc,
                                    std::unique_ptr<TLC::ProcMessage> &msg) override;
        llvm::Error endHandleProc(TLC::ProcContext &proc,
                                  std::unique_ptr<TLC::ProcMessage> &msg) override;
    };

}

#endif // MYPASS_H
