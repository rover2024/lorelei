#include "Pass.h"

#include <stdcorelib/str.h>

#include <lorelei/TLCMeta/MetaPass.h>

#include "ProcContext.h"
#include "DocumentContext.h"
#include "PassCommon.h"
#include "BuilderPassCommon.h"

using namespace clang;

namespace TLC {

    class GetProcAddressProcMessage : public ProcMessage {
    public:
        GetProcAddressProcMessage(int nameIndex) : nameIndex(nameIndex) {
        }

        int nameIndex;
    };

    class GetProcAddressPass : public Pass {
    public:
        GetProcAddressPass() : Pass(Misc, lorethunk::MetaPass::GetProcAddress) {
        }

        std::string name() const override {
            return "GetProcAddress";
        }

        bool testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) const override;
        llvm::Error beginHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;
        llvm::Error endHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;

    protected:
        bool _scanf;
        bool _hasVAList;
    };

    static inline bool isCharPointerType(const clang::QualType &type) {
        return type->isPointerType() && type->getPointeeType()->isCharType();
    }

    bool GetProcAddressPass::testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) const {
        bool passWithDefaultArgs = false;

        // Check pass desc
        if (int passArgs[1]; PASS_getIntegralArgumentsInTemplate(passArgs, proc, id())) {
            if (passArgs[0] < 0) {
                passWithDefaultArgs = true;
            } else {
                msg = std::make_unique<GetProcAddressProcMessage>(passArgs[0]);
                return true;
            }
        }

        // Check function name and arguments
        if (passWithDefaultArgs || StringRef(proc.name()).contains("GetProcAddress")) {
            auto view = proc.realFunctionTypeView();
            auto argTypes = view.argTypes();
            if (!argTypes.empty()) {
                auto MaybeNameType = argTypes.back();
                if (isCharPointerType(MaybeNameType)) {
                    msg = std::make_unique<GetProcAddressProcMessage>(argTypes.size());
                    return true;
                }
            }
        }
        return false;
    }

    llvm::Error GetProcAddressPass::beginHandleProc(ProcContext &proc,
                                                    std::unique_ptr<ProcMessage> &msg) {
        if (proc.procKind() != CProcKind_HostFunction) {
            return llvm::Error::success();
        }

        auto &message = static_cast<GetProcAddressProcMessage &>(*msg.get());
        int nameIndex = message.nameIndex;

        std::string key = name();

        auto &GTP = proc.source(CProcThunkPhase_GTP);
        GTP.body.backward.push_back(key, SRC_asIs("ret = proc::client->convertHostProcAddress(" +
                                                  GTP.functionInfo.argumentName(nameIndex - 1) +
                                                  ", ret);"));
        return llvm::Error::success();
    }

    llvm::Error GetProcAddressPass::endHandleProc(ProcContext &proc,
                                                  std::unique_ptr<ProcMessage> &msg) {
        return llvm::Error::success();
    }

    static PassRegistration<GetProcAddressPass> PR_GetProcAddress;

}