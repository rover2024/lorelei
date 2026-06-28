// SPDX-License-Identifier: MIT

#include <lorelei/Support/StringExtras.h>
#include <lorelei/TLCApi/Pass.h>
#include <lorelei/TLCApi/ProcSnippet.h>
#include <lorelei/TLCApi/DocumentContext.h>
#include <lorelei/ThunkInterface/PassTags.h>

#include "Utils/PassUtils.h"
#include "Utils/PassCodeTemplates.h"

using namespace clang;

namespace lore::tool::TLC {

    class GetProcAddressMessage : public PassMessage {
    public:
        GetProcAddressMessage(int nameIndex) : nameIndex(nameIndex) {
        }

        int nameIndex;
    };

    class GetProcAddressPass : public Pass {
    public:
        GetProcAddressPass() : Pass(Misc, lore::thunk::pass::ID_GetProcAddress) {
        }

        std::string name() const override {
            return "GetProcAddress";
        }

        bool testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        llvm::Error beginHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        llvm::Error endHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
    };

    static inline bool isCharPointerType(const clang::QualType &type) {
        return type->isPointerType() && type->getPointeeType()->isCharType();
    }

    bool GetProcAddressPass::testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) {
        bool passWithDefaultArgs = false;

        // Check pass descriptor
        if (int passArgs[1]; PASS_getIntegralArgumentsInPass(passArgs, proc, false, id())) {
            if (passArgs[0] < 0) {
                passWithDefaultArgs = true;
            } else {
                msg = std::make_unique<GetProcAddressMessage>(passArgs[0]);
                return true;
            }
        }

        // Check function name and arguments
        if (passWithDefaultArgs || StringRef(proc.name()).contains("GetProcAddress")) {
            auto view = proc.realFunctionTypeView();
            auto argTypes = view.argTypes();
            if (!argTypes.empty()) {
                // The proc-name string is conventionally the last parameter (e.g. the `name` of
                // *GetProcAddress); recognise it by its char-pointer type.
                auto maybeNameType = argTypes.back();
                if (isCharPointerType(maybeNameType)) {
                    msg = std::make_unique<GetProcAddressMessage>(argTypes.size());
                    return true;
                }
            }
        }
        return false;
    }

    llvm::Error GetProcAddressPass::beginHandleProc(ProcSnippet &proc,
                                                    std::unique_ptr<PassMessage> &msg) {
        if (proc.kind() != ProcSnippet::Function && proc.direction() != ProcSnippet::GuestToHost) {
            return llvm::Error::success();
        }

        if (proc.document().mode() != DocumentContext::Guest) {
            return llvm::Error::success();
        }

        auto &message = static_cast<GetProcAddressMessage &>(*msg.get());
        int nameIndex = message.nameIndex;

        std::string key = name();

        // Rewrite the returned host proc address in the guest Adapt layer's backward section.
        auto &GADP = proc.source(ProcSnippet::Adapt);
        GADP.body.backward.push_back(
            key,
            SRC_asIs("ret = (decltype(ret)) mod::GuestClient::convertHostProcAddress((const char *) " +
                     GADP.functionInfo.argumentName(nameIndex - 1) + ", (void *) ret);"));
        return llvm::Error::success();
    }

    llvm::Error GetProcAddressPass::endHandleProc(ProcSnippet &proc,
                                                  std::unique_ptr<PassMessage> &msg) {
        return llvm::Error::success();
    }

    static llvm::Registry<Pass>::Add<GetProcAddressPass> PR_GetProcAddress("GetProcAddress", {});

}
