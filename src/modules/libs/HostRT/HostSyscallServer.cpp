#include "HostSyscallServer.h"

#include <array>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <mutex>
#include <vector>
#include <cassert>

#include <alloca.h>
#include <dlfcn.h>
#include <limits.h>
#include <pthread.h>

#ifdef __linux__
#  include <link.h>
#endif

#include <lorelei/Base/PassThrough/Core/SyscallPassThrough.h>
#include <lorelei/Base/PassThrough/Core/IClient.h>
#include <lorelei/Base/Support/StringExtras.h>

#include "Invocation.h"

namespace lore::mod {

    namespace {

#ifdef __x86_64__
        constexpr const char *kHostArch = "x86_64";
#elif defined(__aarch64__)
        constexpr const char *kHostArch = "aarch64";
#elif defined(__riscv) && __riscv_xlen == 64
        constexpr const char *kHostArch = "riscv64";
#else
        constexpr const char *kHostArch = "unknown";
#endif

        constexpr const char *kDefaultSysLibFmt = "/usr/lib/%s-linux-gnu";

        static void *g_specialEntries[64] = {};
        static ThunkInfoConfig g_thunkConfig;

        static const char *pathGetName(const char *path) {
            const char *slashPos = std::strrchr(path, '/');
            if (!slashPos) {
                return path;
            }
            return slashPos + 1;
        }

        static void getLibraryName(char *buffer, const char *path) {
            const char *start = pathGetName(path);
            const char *end = nullptr;

            int len = std::strlen(path);
            for (const char *p = path + len - 3; p >= start; --p) {
                if (strncasecmp(p, ".so", 3) == 0) {
                    end = p;
                    break;
                }
            }

            if (!end) {
                end = path + len;
            }
            std::memcpy(buffer, start, end - start);
            buffer[end - start] = '\0';
        }

        static std::map<std::string, std::string> buildConfigVars() {
            std::string root;
            if (const char *rootEnv = std::getenv("LORELEI_ROOT")) {
                root = rootEnv;
            }

            std::string guestRoot = root;
            if (const char *guestRootEnv = std::getenv("LORELEI_GUEST_ROOT")) {
                guestRoot = guestRootEnv;
            }

            char sysLibBuf[PATH_MAX];
            if (const char *sysLibEnv = std::getenv("LORELEI_SYSLIB_DIR")) {
                std::snprintf(sysLibBuf, sizeof(sysLibBuf), "%s", sysLibEnv);
            } else {
                std::snprintf(sysLibBuf, sizeof(sysLibBuf), kDefaultSysLibFmt, kHostArch);
            }

            std::map<std::string, std::string> vars = {
                {"ROOT",       root                                                        },
                {"GUEST_ROOT", guestRoot                                                   },
                {"HOME",       std::getenv("HOME") ? std::getenv("HOME") : ""              },
                {"GTL_DIR",    guestRoot + "/lib/x86_64-loreg-linux-gnu"                   },
                {"HTL_DIR",    root + "/lib/" + std::string(kHostArch) + "-loreh-linux-gnu"},
                {"ARCH",       kHostArch                                                   },
                {"SYSLIB",     sysLibBuf                                                   },
            };

            if (const char *extraVars = std::getenv("LORELEI_THUNKS_CONFIG_VARIABLES");
                extraVars && *extraVars) {
                for (const auto item : split(std::string_view(extraVars), ";")) {
                    const auto eq = item.find('=');
                    if (eq == std::string_view::npos) {
                        continue;
                    }
                    vars[std::string(item.substr(0, eq))] = std::string(item.substr(eq + 1));
                }
            }

            return vars;
        }

        static void ensureThunkConfigLoaded() {
            static std::once_flag configInitFlag;
            static bool loaded = false;
            std::call_once(configInitFlag, [] {
                std::string root;
                if (const char *rootEnv = std::getenv("LORELEI_ROOT")) {
                    root = rootEnv;
                }

                std::filesystem::path configPath;
                if (const char *configEnv = std::getenv("LORELEI_THUNKS_CONFIG")) {
                    configPath = configEnv;
                } else if (!root.empty()) {
                    configPath = std::filesystem::path(root) / "etc" / "lorelei" / "thunks.json";
                } else {
                    configPath = std::filesystem::path("etc") / "lorelei" / "thunks.json";
                }

                loaded = g_thunkConfig.load(configPath, buildConfigVars());
            });
        }

        static CString queryHostAttribute(const char *key) {
            static const std::string hostArch = kHostArch;

            if (!key) {
                return {};
            }

            if (std::strcmp(key, "arch") == 0 || std::strcmp(key, "host_arch") == 0) {
                return {const_cast<char *>(hostArch.c_str()), hostArch.size()};
            }

            if (std::strcmp(key, "thunks_config") == 0) {
                if (const char *path = std::getenv("LORELEI_THUNKS_CONFIG")) {
                    return {const_cast<char *>(path), std::strlen(path)};
                }
            }

            return {};
        }

    }

    uint64_t HostSyscallServer::dispatchSyscall(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3,
                                                uint64_t a4, uint64_t a5, uint64_t a6) {
        using namespace lore;
        using namespace lore::mod;

        assert(num == SyscallPathThroughNumber);

        switch (a1) {
            case SPID_GetHostAttribute: {
                auto a = reinterpret_cast<void **>(a2);
                auto ret = reinterpret_cast<CString *>(a3);
                assert(a);
                assert(ret);
                const auto key = reinterpret_cast<const char *>(a[0]);
                *ret = queryHostAttribute(key);
                return 0;
            }

            case SPID_LogMessage: {
                auto a = reinterpret_cast<void **>(a2);
                assert(a);
                const auto level = static_cast<int>(reinterpret_cast<uintptr_t>(a[0]));
                const auto context = reinterpret_cast<const LogContext *>(a[1]);
                const auto msg = reinterpret_cast<const char *>(a[2]);
                assert(context);
                assert(msg);
                Logger(*context).print(level, msg);
                return 0;
            }

            case SPID_LoadLibrary: {
                auto a = reinterpret_cast<void **>(a2);
                auto ret = reinterpret_cast<void **>(a3);
                assert(a);
                assert(ret);
                const auto path = reinterpret_cast<const char *>(a[0]);
                const auto flags = static_cast<int>(reinterpret_cast<uintptr_t>(a[1]));
                *ret = dlopen(path, flags);
                return 0;
            }

            case SPID_FreeLibrary: {
                auto a = reinterpret_cast<void **>(a2);
                auto ret = reinterpret_cast<int *>(a3);
                assert(a);
                assert(ret);
                *ret = dlclose(a[0]);
                return 0;
            }

            case SPID_GetProcAddress: {
                auto a = reinterpret_cast<void **>(a2);
                auto ret = reinterpret_cast<void **>(a3);
                assert(a);
                assert(ret);

                auto handle = a[0];
                auto name = reinterpret_cast<const char *>(a[1]);
                void *sym = dlsym(handle, name);
                if (!sym) {
                    sym = dlsym(RTLD_DEFAULT, name);
                }
                *ret = sym;
                return 0;
            }

            case SPID_GetErrorMessage: {
                auto ret = reinterpret_cast<char **>(a3);
                assert(ret);
                *ret = dlerror();
                return 0;
            }

            case SPID_GetModulePath: {
                auto a = reinterpret_cast<void **>(a2);
                auto ret = reinterpret_cast<char **>(a3);
                assert(a);
                assert(ret);

                auto opaque = a[0];
                const bool isHandle = static_cast<bool>(reinterpret_cast<intptr_t>(a[1]));

                if (isHandle) {
#ifdef __linux__
                    struct link_map *lm = nullptr;
                    if (dlinfo(opaque, RTLD_DI_LINKMAP, &lm) == 0 && lm && lm->l_name &&
                        lm->l_name[0] != '\0') {
                        *ret = lm->l_name;
                    } else {
                        *ret = nullptr;
                    }
#else
                    *ret = nullptr;
#endif
                } else {
                    Dl_info info;
                    if (dladdr(opaque, &info) == 0) {
                        *ret = nullptr;
                    } else {
                        *ret = const_cast<char *>(info.dli_fname);
                    }
                }
                return 0;
            }

            case SPID_InvokeProc: {
                auto ia = reinterpret_cast<const InvocationArguments *>(a2);
                auto ra_ptr = reinterpret_cast<ReentryArguments **>(a3);
                return Invocation::invoke(ia, ra_ptr);
            }

            case SPID_ResumeInvocation: {
                return Invocation::resume();
            }

            case SPID_GetThunkInfo: {
                auto a = reinterpret_cast<void **>(a2);
                auto ret = reinterpret_cast<CThunkInfo *>(a3);
                assert(a);
                assert(ret);

                ensureThunkConfigLoaded();

                auto path = reinterpret_cast<const char *>(a[0]);
                bool isReverse = static_cast<bool>(reinterpret_cast<uintptr_t>(a[1]));

                char nameBuf[PATH_MAX];
                getLibraryName(nameBuf, path ? path : "");

                std::string name(nameBuf);
                if (str::ends_with(name, "_HTL")) {
                    name = name.substr(0, name.size() - 4);
                }

                if (isReverse) {
                    auto *info = g_thunkConfig.reversedThunk(name);
                    ret->reversed =
                        info ? const_cast<CReversedThunkInfo *>(&info->c_data()) : nullptr;
                } else {
                    auto *info = g_thunkConfig.forwardThunk(name);
                    ret->forward =
                        info ? const_cast<CForwardThunkInfo *>(&info->c_data()) : nullptr;
                }
                return 0;
            }

            case SPID_HeapAlloc: {
                auto size = static_cast<size_t>(a2);
                auto ret = reinterpret_cast<void **>(a3);
                assert(ret);
                *ret = std::malloc(size);
                return 0;
            }

            case SPID_HeapFree: {
                std::free(reinterpret_cast<void *>(a2));
                return 0;
            }

            case SPID_SetSpecialEntry: {
                auto a = reinterpret_cast<void **>(a2);
                assert(a);
                const auto id = static_cast<size_t>(reinterpret_cast<uintptr_t>(a[0]));
                assert(id < std::size(g_specialEntries));
                g_specialEntries[id] = a[1];
                return 0;
            }

            default:
                break;
        }

        return static_cast<uint64_t>(-1);
    }

}
