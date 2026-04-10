#include "GuestThunkContext.h"

#include <dlfcn.h>

#include <map>
#include <string>

#include <lorelei/Base/Support/Logging.h>

#include "GuestSyscallClient.h"

namespace lore::mod {

    namespace {

        static constexpr uintptr_t kFunctionTrampolineMagicSign = 0x1234567890ABCDEFULL;

    }

    GuestThunkContext::~GuestThunkContext() {
        for (auto *table : m_fdgTrampolineTables) {
            if (table) {
                FunctionTrampolineTable::destroy(table);
            }
        }
        if (m_htlHandle) {
            std::ignore = GuestSyscallClient::freeLibrary(m_htlHandle);
        }
    }

    void GuestThunkContext::initialize() {
        /// STEP: get thunk name
        const char *modulePath = nullptr;
        Dl_info selfInfo;
        if (!dladdr(m_staticContext, &selfInfo)) {
            loreCritical("[GTL] failed to get thunk library name");
            std::abort();
        }
        modulePath = selfInfo.dli_fname;

        /// STEP: load host thunk library
        const char *htlPath;
        {
            auto info = GuestSyscallClient::getThunkInfo(modulePath, false);
            if (!info.forward) {
                loreCritical("[GTL] %1: failed to get thunk info", modulePath);
                std::abort();
            }
            htlPath = info.forward->hostThunk;
            m_htlHandle = GuestSyscallClient::loadLibrary(htlPath, RTLD_NOW);
        }

        if (!m_htlHandle) {
            loreCritical("[GTL] %1: failed to load HTL", htlPath);
            std::abort();
        }

        /// STEP: exchange context
        {
            void *exchangeFunc =
                GuestSyscallClient::getProcAddress(m_htlHandle, "__Lore_ExchangeContext");
            if (!exchangeFunc) {
                loreCritical("[GTL] %1: failed to get init proc", htlPath);
                std::abort();
            }
            void *args[] = {
                m_staticContext,
            };
            GuestSyscallClient::invokeFormat(exchangeFunc, "v_p", args, nullptr);
        }

        /// STEP: fill FDG list with FunctionTrampoline
        if (m_staticContext->FDGs) {
            std::map<std::string, void *> guestToHostCallbackThunkByName;
            {
                auto callbackEntries =
                    m_staticContext->guestProcs[CProcKind_Callback][CProcDirection_GuestToHost];
                for (size_t i = 0; i < callbackEntries.size; ++i) {
                    const auto &entry = callbackEntries.arr[i];
                    assert(entry.name != nullptr);
                    assert(entry.addr != nullptr);
                    guestToHostCallbackThunkByName[entry.name] = entry.addr;
                }
            }

            for (size_t i = 0; i < m_staticContext->numFDGs; ++i) {
                auto &fdgInfo = m_staticContext->FDGs[i];
                assert(fdgInfo.signature != nullptr);
                if (fdgInfo.count == 0) {
                    continue;
                }
                assert(fdgInfo.pProcs != nullptr);

                void *target = nullptr;
                if (auto it = guestToHostCallbackThunkByName.find(fdgInfo.signature);
                    it != guestToHostCallbackThunkByName.end()) {
                    target = it->second;
                }
                if (!target) {
                    loreCritical("[GTL] %1: failed to find guest-to-host callback entry for FDG "
                                 "signature %2",
                                 modulePath, fdgInfo.signature);
                    std::abort();
                }

                auto *table = FunctionTrampolineTable::create(static_cast<size_t>(fdgInfo.count),
                                                              target,
                                                              kFunctionTrampolineMagicSign);
                assert(table != nullptr);
                m_fdgTrampolineTables.push_back(table);
                for (int j = 0; j < fdgInfo.count; ++j) {
                    auto *pair = fdgInfo.pProcs[j];
                    if (!pair) {
                        continue;
                    }
                    auto &tramp = table->trampoline[j];
                    tramp.saved_function = pair->hostAddr;
                    pair->guestTramp = static_cast<void *>(tramp.thunk_instr);
                }
            }
        }
    }

}
