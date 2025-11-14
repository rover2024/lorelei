#include "HostServer.h"

#include <cstdlib>
#include <cstdio>
#include <set>

#include <stdcorelib/support/logging.h>
#include <stdcorelib/support/sharedlibrary.h>
#include <stdcorelib/console.h>

#include "Timing.h"

namespace lore {
#ifdef __x86_64__
#  define _ARCH "x86_64"
#elif defined(__aarch64__)
#  define _ARCH "aarch64"
#elif defined(__riscv)
#  define _ARCH "riscv64"
#endif

    static const char kSystemLibraryDir[] = "/usr/lib/" _ARCH "-linux-gnu";

    static void logCallback(int level, const stdc::LogContext &ctx, const std::string_view &s);

    struct LOREHOSTRT_EXPORT HostRuntime {
        int level = stdc::Logger::Information;
        FILE *logFile = nullptr;
        stdc::Logger::LogCallback defaultLogCallback = nullptr;

        // The syscall server instance
        HostServer server;

        HostRuntime() {
            if (auto level_str = std::getenv("LORELEI_HOST_LOG_LEVEL"); level_str) {
                level = std::atoi(level_str);
                if (level == 0) {
                    level = stdc::Logger::Information;
                }
            }

            if (auto log_file_str = std::getenv("LORELEI_HOST_LOG_FILE"); log_file_str) {
                logFile = std::fopen(log_file_str, "w");
            }

            defaultLogCallback = stdc::Logger::logCallback();
            stdc::Logger::setLogCallback(logCallback);

            /// STEP: get directory and files
            std::filesystem::path rootDir;
            std::filesystem::path runtimePath =
                stdc::SharedLibrary::locateLibraryPath((void *) logCallback).parent_path();
            if (auto root_dir_str = std::getenv("LORELEI_ROOT"); root_dir_str) {
                rootDir = root_dir_str;
            } else {
                auto thisDir = runtimePath;
                do {
                    auto dirName = thisDir.filename().string();
                    if (dirName == "lib" || dirName == "lib64") {
                        rootDir = thisDir.parent_path();
                        break;
                    }
                    if (!thisDir.has_parent_path()) {
                        break;
                    }
                    thisDir = thisDir.parent_path();
                } while (true);
            }
            if (rootDir.empty()) {
                rootDir = runtimePath;
            }

            std::filesystem::path guestRootDir;
            if (auto guest_root_dir_str = std::getenv("LORELEI_GUEST_ROOT"); guest_root_dir_str) {
                guestRootDir = guest_root_dir_str;
            } else {
                guestRootDir = rootDir;
            }

            /// STEP: load config
            std::filesystem::path configPath;
            if (auto config_file_str = std::getenv("LORELEI_THUNKS_CONFIG"); config_file_str) {
                configPath = config_file_str;
            } else {
                configPath = rootDir / "etc" / "lorelei" / "thunks.json";
            }

            const char *sysLibDir;
            if (auto sysLibDir_str = std::getenv("LORELEI_SYSLIB_DIR"); sysLibDir_str) {
                sysLibDir = sysLibDir_str;
            } else {
                sysLibDir = kSystemLibraryDir;
            }

            std::map<std::string, std::string> configVars = {
                {"ROOT",       rootDir.string()                                          },
                {"GUEST_ROOT", guestRootDir.string()                                     },
                {"HOME",       std::getenv("HOME")                                       },
                {"GTL_DIR",    (guestRootDir / "lib" / "x86_64-loreg-linux-gnu").string()},
                {"HTL_DIR",    (rootDir / "lib" / _ARCH "-loreh-linux-gnu").string()     },
                {"ARCH",       _ARCH                                                     },
                {"SYSLIB",     sysLibDir                                                 },
            };
            if (auto config_vars_str = std::getenv("LORELEI_THUNKS_CONFIG_VARIABLES");
                config_vars_str) {
                auto var_strs = stdc::split(std::string_view(config_vars_str), ";");
                for (auto var_str : var_strs) {
                    auto var_name_end = var_str.find('=');
                    if (var_name_end == std::string::npos) {
                        continue;
                    }
                    auto var_name = var_str.substr(0, var_name_end);
                    auto var_value = var_str.substr(var_name_end + 1);
                    configVars[std::string(var_name)] = var_value;
                }
            }

            ThunkInfoConfig config;
            if (config.load(configPath, configVars)) {
                // Build LD_LIBRARY_PATH
                {
                    std::vector<std::string> pathList;
                    std::set<std::string> pathSet;
                    const auto &addPath = [&pathList, &pathSet](const std::filesystem::path &path) {
                        if (std::filesystem::is_directory(path)) {
                            std::string canonicalPath = std::filesystem::canonical(path).string();
                            if (!pathSet.count(canonicalPath)) {
                                pathList.push_back(canonicalPath);
                                pathSet.insert(canonicalPath);
                            }
                        }
                    };

                    addPath(rootDir / "lib");

                    auto orgPathListStr = std::getenv("LD_LIBRARY_PATH");
                    if (orgPathListStr) {
                        for (const auto &item :
                             stdc::split(std::string_view(orgPathListStr), ":")) {
                            addPath(std::filesystem::path(item));
                        }
                    }

                    for (const auto &item : config.forwardThunks()) {
                        addPath(std::filesystem::path(item.hostLibrary).parent_path());
                    }
                    setenv("LD_LIBRARY_PATH", stdc::join(pathList, ":").c_str(), true);
                }
                server.setConfig(std::move(config));
            }
        }

        ~HostRuntime() {
            if (logFile) {
                std::fclose(logFile);
            }
        }
    };

    LOREHOSTRT_EXPORT HostRuntime runtime_instance;

    LOREHOSTRT_EXPORT thread_local void *thread_last_callback;

    static void logCallback(int level, const stdc::LogContext &ctx, const std::string_view &s) {
        if (level >= runtime_instance.level) {
            if (runtime_instance.logFile) {
                std::fprintf(runtime_instance.logFile, "%s\n", s.data());
            } else {
                runtime_instance.defaultLogCallback(level, ctx, s);
            }
        }
    }

    thread_local uint64_t timing_ticks = 0;
    thread_local uint64_t timing_last_tick = 0;
    thread_local uint64_t timing_total_ticks = 0;

}

// implementation of "EmulatorIntegration"
#include "EmulatorIntegration.h"

uint64_t LOREHOSTRT_dispatchSyscall(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3,
                                    uint64_t a4, uint64_t a5, uint64_t a6) {
    return lore::runtime_instance.server.dispatch(num, a1, a2, a3, a4, a5, a6);
}

void LOREHOSTRT_setRunTaskEntry(void *runTask) {
    lore::HostServer::setRunTaskEntry((lore::HostServer::RunTaskEntry) runTask);
}

void LOREHOSTRT_notifyThreadEntry() {
    using namespace lore;
    timing_last_tick = rdtsc();
}

void LOREHOSTRT_notifyThreadExit() {
    using namespace lore;
    uint64_t t = rdtsc();
    timing_total_ticks = t - timing_total_ticks;
    stdcInfoF("TICKS: host=%lu, total=%lu\n", timing_ticks, timing_total_ticks);
}


// implementation of "MultiThreading"
#include "MultiThreading.h"

struct LORE_HOST_THREAD_CONTEXT {
    const pthread_attr_t *last_attr;
    pthread_t last_tid;
};

using PFN_GetThreadContext = LORE_HOST_THREAD_CONTEXT *(*)(void);

static PFN_GetThreadContext s_getThreadContext = nullptr;

void LOREHOSTRT_setGetThreadContext(void *getter) {
    s_getThreadContext = (PFN_GetThreadContext) getter;
}

int LOREHOSTRT_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                              void *(*start_routine)(void *), void *arg) {
    auto &thread_ctx = *s_getThreadContext();
    thread_ctx.last_attr = attr;

    int ret;
    lore::runtime_instance.server.runTaskPthreadCreate(thread, (void *) attr,
                                                       (void *) start_routine, arg, &ret);
    *thread = thread_ctx.last_tid;
    return ret;
}

void LOREHOSTRT_pthread_exit(void *ret) {
    lore::runtime_instance.server.runTaskPthreadExit(ret);
    __builtin_unreachable();
}