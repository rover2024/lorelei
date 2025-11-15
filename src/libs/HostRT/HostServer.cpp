#include "HostServer.h"

#include <vector>
#include <cstdlib>
#include <cstring>

#include <dlfcn.h>
#include <limits.h>

#ifdef __linux__
#  include <link.h>
#endif

#include <stdcorelib/support/logging.h>

#include <lorelei/Core/Connect/SyscallClient.h>
#include <lorelei/Core/ThunkTools/VariadicAdaptor.h>

#include "Timing.h"

namespace lore {

    using ReqID = SyscallClient<>::RequestID;

    using Convention = Client<>::Convention;

    static HostServer *m_instance = nullptr;

    struct ThreadContext {
        std::vector<ClientTask *> tasks;
    };

    static thread_local ThreadContext thread_ctx;

    static void before_call(void *proc, int conv, void *opaque) {
        timing_last_tick = rdtsc();
    }

    static void after_call(void *proc, int conv, void *opaque) {
        timing_ticks += rdtsc() - timing_last_tick;
    }

    static void prepare_arg_entry(char fmt, CVargEntry *entry, void *arg) {
        switch (fmt) {
            case 'c':
                entry->type = CVargType_Char;
                entry->c = *(char *) arg;
                break;
            case 'C':
                entry->type = CVargType_UChar;
                entry->uc = *(unsigned char *) arg;
                break;
            case 's':
                entry->type = CVargType_Short;
                entry->s = *(short *) arg;
                break;
            case 'S':
                entry->type = CVargType_UShort;
                entry->us = *(unsigned short *) arg;
                break;
            case 'i':
                entry->type = CVargType_Int;
                entry->i = *(int *) arg;
                break;
            case 'I':
                entry->type = CVargType_UInt;
                entry->u = *(unsigned int *) arg;
                break;
            case 'l':
                entry->type = CVargType_Long;
                entry->l = *(long *) arg;
                break;
            case 'u':
                entry->type = CVargType_ULong;
                entry->ul = *(unsigned long *) arg;
                break;
            case 'L':
                entry->type = CVargType_LongLong;
                entry->ll = *(long long *) arg;
                break;
            case 'U':
                entry->type = CVargType_ULongLong;
                entry->ull = *(unsigned long long *) arg;
                break;
            case 'f':
                entry->type = CVargType_Float;
                entry->f = *(float *) arg;
                break;
            case 'F':
                entry->type = CVargType_Double;
                entry->d = *(double *) arg;
                break;
            case 'p':
                entry->type = CVargType_Pointer;
                entry->p = *(void **) arg;
                break;
            default:
                entry->type = CVargType_Void;
                break;
        }
    }

    static void prepare_ret_entry(char fmt, CVargEntry *entry) {
        switch (fmt) {
            case 'c':
                entry->type = CVargType_Char;
                break;
            case 'C':
                entry->type = CVargType_UChar;
                break;
            case 's':
                entry->type = CVargType_Short;
                break;
            case 'S':
                entry->type = CVargType_UShort;
                break;
            case 'i':
                entry->type = CVargType_Int;
                break;
            case 'I':
                entry->type = CVargType_UInt;
                break;
            case 'l':
                entry->type = CVargType_Long;
                break;
            case 'u':
                entry->type = CVargType_ULong;
                break;
            case 'L':
                entry->type = CVargType_LongLong;
                break;
            case 'U':
                entry->type = CVargType_ULongLong;
                break;
            case 'f':
                entry->type = CVargType_Float;
                break;
            case 'F':
                entry->type = CVargType_Double;
                break;
            case 'p':
                entry->type = CVargType_Pointer;
                break;
            default:
                entry->type = CVargType_Void;
                break;
        }
    }

    // fmt: <ret>_<a1><a2>...
    static void do_fmt_call(void *func, const char *fmt, void **args, void *ret) {
        const auto len = std::strlen(fmt);

        CVargEntry vret;
        prepare_ret_entry(fmt[0], &vret);

        auto vargs = (CVargEntry *) alloca(sizeof(CVargEntry) * (len - 2));
        for (int i = 0; i < len - 2; i++) {
            auto fmt_char = fmt[i + 2];
            prepare_arg_entry(fmt_char, &vargs[i], args[i]);
        }
        VariadicAdaptor::call(func, len - 2, vargs, 0, nullptr, &vret);

        if (ret) {
            switch (vret.type) {
                case CVargType_Char:
                    *(char *) ret = vret.c;
                    break;
                case CVargType_UChar:
                    *(unsigned char *) ret = vret.uc;
                    break;
                case CVargType_Short:
                    *(short *) ret = vret.s;
                    break;
                case CVargType_UShort:
                    *(unsigned short *) ret = vret.us;
                    break;
                case CVargType_Int:
                    *(int *) ret = vret.i;
                    break;
                case CVargType_UInt:
                    *(unsigned int *) ret = vret.u;
                    break;
                case CVargType_Long:
                    *(long *) ret = vret.l;
                    break;
                case CVargType_ULong:
                    *(unsigned long *) ret = vret.ul;
                    break;
                case CVargType_LongLong:
                    *(long long *) ret = vret.ll;
                    break;
                case CVargType_ULongLong:
                    *(unsigned long long *) ret = vret.ull;
                    break;
                case CVargType_Float:
                    *(float *) ret = vret.f;
                    break;
                case CVargType_Double:
                    *(double *) ret = vret.d;
                    break;
                case CVargType_Pointer:
                    *(void **) ret = vret.p;
                    break;
                default:
                    break;
            }
        }
    }

    static const char *path_get_name(const char *path) {
        const char *slashPos = strrchr(path, '/');
        if (!slashPos) {
            return path;
        }
        return slashPos + 1;
    }

    static void get_library_name(char *buffer, const char *path) {
        const char *start = path_get_name(path);
        const char *end = NULL;

        int len = strlen(path);
        for (const char *p = path + len - 3; p >= start; --p) {
            if (strncasecmp(p, ".so", 3) == 0) {
                end = p;
                break;
            }
        }
        if (!end) {
            end = path + len;
        }
        memcpy(buffer, start, end - start);
        buffer[end - start] = '\0';
    }

    HostServer::HostServer() {
        if (m_instance) {
            fprintf(stderr, "HostServer can only be instantiated once!!!\n");
            std::abort();
        }
        m_instance = this;
    }

    HostServer::~HostServer() {
        m_instance = nullptr;
    }

    HostServer *HostServer::instance() {
        return m_instance;
    }

    static HostServer::RunTaskEntry s_runTaskEntry = nullptr;

    HostServer::RunTaskEntry HostServer::runTaskEntry() {
        return s_runTaskEntry;
    }

    void HostServer::setRunTaskEntry(RunTaskEntry runTask) {
        s_runTaskEntry = runTask;
    }

    uint64_t HostServer::dispatch_impl(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3,
                                       uint64_t a4, uint64_t a5, uint64_t a6) {
        uint64_t &sub_id = a1;
        switch (sub_id) {
            // check health
            case ReqID::REQUEST_CHECK_CONNECTION: {
                return 0;
            }

            case ReqID::REQUEST_LOG_MESSAGE: {
                auto level = (int) (uintptr_t) a2;

                struct CLogContext {
                    int line;
                    const char *file;
                    const char *function;
                    const char *category;
                };
                auto context = (const CLogContext *) a3;
                auto msg = (const char *) a4;

                // align
                stdc::LogContext new_ctx(context->file, context->line, context->function,
                                         context->category);
                stdc::Logger(new_ctx).log(level, msg);
                return 0;
            }

            // load library
            case ReqID::REQUEST_LOAD_LIBRARY: {
                auto a = (void **) a2;
                auto ret = (void **) a3;

                auto path = (const char *) a[0];
                auto flags = (int) (uintptr_t) a[1];
                *ret = dlopen(path, flags);
                printf("[HRT] load library: %s\n", path);
                return 0;
            }

            // free library
            case ReqID::REQUEST_FREE_LIBRARY: {
                auto a = (void **) a2;
                auto ret = (int *) a3;

                auto handle = a[0];
                if (struct link_map * lm; dlinfo(handle, RTLD_DI_LINKMAP, &lm) == 0) {
                    if (lm->l_name && lm->l_name[0] != '\0') {
                        printf("[HRT] free library: %s", lm->l_name);
                    }
                }
                *ret = dlclose(handle);
                return 0;
            }

            // get proc address
            case ReqID::REQUEST_GET_PROC_ADDRESS: {
                auto a = (void **) a2;
                auto ret = (void **) a3;

                auto handle = a[0];
                auto name = (const char *) a[1];
                void *sym = dlsym(handle, name);
                if (!sym) {
                    sym = dlsym(RTLD_DEFAULT, name);
                }
                *ret = sym;
                return 0;
            }

            // get error message
            case ReqID::REQUEST_GET_ERROR_MESSAGE: {
                auto ret = (char **) a2;
                *ret = dlerror();
                return 0;
            }

            // get module path
            case ReqID::REQUEST_GET_MODULE_PATH: {
                auto a = (void **) a2;
                auto ret = (char **) a3;

                void *opaque = a[0];
                auto isHandle = (bool) (intptr_t) a[1];
                if (isHandle) {
#ifdef __linux__
                    struct link_map *lm;
                    if (dlinfo(opaque, RTLD_DI_LINKMAP, &lm) == 0) {
                        if (lm->l_name && lm->l_name[0] != '\0') {
                            *ret = lm->l_name;
                        } else {
                            *ret = nullptr;
                        }
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
                        *ret = (char *) info.dli_fname;
                    }
                }
                return 0;
            }

            // invoke proc
            case ReqID::REQUEST_INVOKE_PROC: {
                auto proc = (void *) a2;
                auto conv = (int) (uintptr_t) a3;
                auto opaque = (void **) a4;
                auto task = (ClientTask *) a5;

                auto &tasks = thread_ctx.tasks;
                tasks.push_back(task);

                switch (conv) {
                    case Convention::CONV_STANDARD: {
                        using Func = void (*)(void * /*args*/, void * /*ret*/, void * /*metadata*/);
                        auto func = (Func) proc;
                        auto args = opaque[0];
                        auto ret = opaque[1];
                        auto metadata = opaque[2];
                        before_call(proc, conv, opaque);
                        func(args, ret, metadata);
                        after_call(proc, conv, opaque);
                        break;
                    }

                    case Convention::CONV_STANDARD_CALLBACK: {
                        using Func = void (*)(void * /*callback*/, void * /*args*/, void * /*ret*/,
                                              void * /*metadata*/);
                        auto func = (Func) proc;
                        auto callback = opaque[0];
                        auto args = opaque[1];
                        auto ret = opaque[2];
                        auto metadata = opaque[3];
                        before_call(proc, conv, opaque);
                        func(callback, args, ret, metadata);
                        after_call(proc, conv, opaque);
                        break;
                    }

                    case Convention::CONV_FORMAT: {
                        auto fmt = (const char *) opaque[0];
                        auto args = (void **) opaque[1];
                        auto ret = (void *) opaque[2];
                        before_call(proc, conv, opaque);
                        do_fmt_call(proc, fmt, args, ret);
                        after_call(proc, conv, opaque);
                        break;
                    }

                    case Convention::CONV_THREAD_ENTRY: {
                        using Func = void *(*) (void * /*arg*/);
                        auto func = (Func) proc;
                        auto arg = opaque[0];
                        auto ret = (void **) opaque[1];
                        before_call(proc, conv, opaque);
                        *ret = func(arg);
                        after_call(proc, conv, opaque);
                        break;
                    }

                    default:
                        break;
                }

                tasks.pop_back();
                return 0;
            }

            // get thunk info
            case ReqID::REQUEST_GET_THUNK_INFO: {
                auto a = (void **) a2;
                auto ret = (CThunkInfo *) a3;

                auto path = (const char *) a[0];
                auto isReserve = (bool) (uintptr_t) a[1];

                char nameBuf[PATH_MAX];
                get_library_name(nameBuf, path);

                std::string name(nameBuf);
                if (stdc::ends_with(name, "_HTL")) {
                    name = name.substr(0, name.size() - 4);
                }

                auto &ctx = thread_ctx;
                if (isReserve) {
                    auto it = _config.reversedThunk(name);
                    if (!it) {
                        ret->reversed = nullptr;
                    } else {
                        ret->reversed = (CReversedThunkInfo *) &it.value().get().cinfo();
                    }
                } else {
                    auto it = _config.forwardThunk(name);
                    if (!it) {
                        ret->forward = nullptr;
                    } else {
                        ret->forward = (CForwardThunkInfo *) &it.value().get().cinfo();
                    }
                }
                return 0;
            }

            default: {
                break;
            }
        }
        return -1;
    }

    ClientTask *HostServer::currentTask_impl() const {
        auto &tasks = thread_ctx.tasks;
        if (tasks.empty()) {
            return nullptr;
        }
        return tasks.back();
    }

    uint64_t HostServer::runTask_impl() {
        assert(s_runTaskEntry);
        after_call(nullptr, Convention::CONV_STANDARD, nullptr);
        uint64_t ret = s_runTaskEntry(currentTask());
        before_call(nullptr, Convention::CONV_STANDARD, nullptr);
        return ret;
    }

}