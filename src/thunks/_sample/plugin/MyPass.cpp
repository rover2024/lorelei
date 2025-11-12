#include "MyPass.h"

namespace plugin {

    llvm::Error MyPass::beginHandleProc(TLC::ProcContext &proc,
                                        std::unique_ptr<TLC::ProcMessage> &msg) {
        return llvm::Error::success();
    }

    llvm::Error MyPass::endHandleProc(TLC::ProcContext &proc,
                                      std::unique_ptr<TLC::ProcMessage> &msg) {
        return llvm::Error::success();
    }


}