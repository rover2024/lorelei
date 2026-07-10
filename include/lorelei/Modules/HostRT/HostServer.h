// SPDX-License-Identifier: MIT

#ifndef LORE_MODULES_HOSTRT_HOSTSERVER_H
#define LORE_MODULES_HOSTRT_HOSTSERVER_H

#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <cassert>

#include <lorelei/DLCall/Protocol.h>
#include <lorelei/DLCall/ThunkDatabase.h>
#include <lorelei/Modules/HostRT/Global.h>

namespace lore::mod {

    /// HostServer - Host-side endpoint of the DLCall bridge.
    ///
    /// Lives in the host process (loaded into QEMU alongside the \c dlcall plugin) and is the
    /// counterpart of \c GuestClient. The guest's \c DR_InvokeProc requests arrive through the
    /// exported \c LoreCommonHostEntry symbol, which dispatches on a \c DLCallSecondaryID to run
    /// host functions on the guest's behalf, drive reentries back into guest code, log, and answer
    /// thunk-database lookups. A single instance is created by the host runtime and reachable via
    /// \c instance().
    class LOREHOSTRT_EXPORT HostServer {
    public:
        HostServer();
        ~HostServer();

        /// The singleton instance. Constructed at host runtime startup.
        static inline HostServer *instance() {
            return self;
        }

        /// Install the thunk database used to resolve guest/host library mappings. Called at host
        /// runtime startup.
        void setThunkDatabase(std::unique_ptr<ThunkDatabase> db);

        /// The installed thunk database.
        inline const ThunkDatabase *thunkDatabase() const {
            assert(m_thunkDatabase != nullptr);
            return m_thunkDatabase.get();
        }

        /// Set up run-time thunk discovery. Builds the initial database from the explicit override JSON
        /// alone (\a overridePath, may be empty), and remembers what discovery needs later: the JSON
        /// ${...} substitution \a vars and the host architecture tag \a hostArch (for the
        /// <arch>-LoreHTL directory). \a autoDiscover false turns off convention discovery, so only the
        /// override JSON applies. Called at host runtime startup.
        void configureThunkDiscovery(const std::map<std::string, std::string> &vars,
                                     const std::filesystem::path &overridePath,
                                     const std::string &hostArch, bool autoDiscover);

        /// Answer a guest DS_GetThunkInfo request. \a path is the guest thunk's own resolved location
        /// for a forward lookup (it reports its dladdr self-path) or a host library for a reversed one.
        /// A forward lookup that misses the database is resolved by convention from that location (see
        /// resolveForwardThunk). Serialized on m_thunkMutex, since it may extend the database and the
        /// guest can issue requests from several threads.
        void getThunkInfo(const char *path, bool isReverse, CThunkInfo *ret);

        /// Host-side reference address, set during host runtime startup. Used to tell host
        /// addresses apart from guest addresses (e.g. by the guard logic in HostThunkContext).
        static void *emuAddr;

        /// Returns true if \a addr is a host address.
        static bool isHostAddressNaive(void *addr);

    public:
        /// Reenter the guest with a specific reentry convention. Called by host-side thunk code
        /// when a host function needs to call back into guest code mid-invocation. Delegates to the
        /// runtime's coroutine-based invocation machinery. The \c reenter* helpers below build the
        /// \c ReentryArguments for each convention and forward here.
        static void reenter(ReentryArguments *ra);

        static inline void reenterStandard(void *proc, void **args, void *ret, void *metadata) {
            ReentryArguments a;
            a.conv = SC_Standard;
            a.standard.proc = proc;
            a.standard.args = (void *) args;
            a.standard.ret = ret;
            a.standard.metadata = metadata;
            reenter(&a);
        }

        static inline void reenterStandardCallback(void *proc, void *callback, void **args,
                                                   void *ret, void *metadata) {
            ReentryArguments a;
            a.conv = SC_StandardCallback;
            a.standardCallback.proc = proc;
            a.standardCallback.callback = callback;
            a.standardCallback.args = (void *) args;
            a.standardCallback.ret = ret;
            a.standardCallback.metadata = metadata;
            reenter(&a);
        }

        static inline void reenterFormat(void *proc, const char *format, void **args, void *ret) {
            ReentryArguments a;
            a.conv = SC_Format;
            a.format.proc = proc;
            a.format.format = format;
            a.format.args = args;
            a.format.ret = ret;
            reenter(&a);
        }

        static inline int reenterThreadCreate(void *thread, void *attr, void *start_routine,
                                              void *arg) {
            int ret;
            ReentryArguments a;
            a.conv = SC_ThreadCreate;
            a.threadCreate.thread = thread;
            a.threadCreate.attr = attr;
            a.threadCreate.start_routine = start_routine;
            a.threadCreate.arg = arg;
            a.threadCreate.ret = &ret;
            reenter(&a);
            return ret;
        }

        static inline void reenterThreadExit(void *ret) {
            ReentryArguments a;
            a.conv = SC_ThreadExit;
            a.threadExit.ret = ret;
            reenter(&a);
        }

    protected:
        // Resolve a forward thunk for a guest thunk's own resolved location \a guestThunkPath (its
        // basename is \a name). Returns a database entry when one exists (from a JSON or an earlier
        // convention hit). Otherwise, for a guest thunk in the standard layout, loads that pack's JSON
        // once and then materializes a convention entry (host thunk
        // <prefix>/lib/<arch>-LoreHTL/<name>_HTL.so). A bare name (from a reversed lookup's candidate)
        // materializes a by-soname entry the guest loader resolves. Assumes m_thunkMutex is held.
        const CForwardThunkInfo *resolveForwardThunk(const char *guestThunkPath,
                                                     const std::string &name);

        std::unique_ptr<ThunkDatabase> m_thunkDatabase;

        // Serializes m_thunkDatabase access: getThunkInfo() may extend it (loadPack /
        // materializeForward) while other guest threads read it.
        std::mutex m_thunkMutex;

        // Retained from configureThunkDiscovery() so resolveForwardThunk() can load a pack's JSON the
        // first time a thunk from it appears.
        std::map<std::string, std::string> m_thunkVars;
        std::string m_hostArch;
        bool m_thunkAutoDiscover = false;
        std::unordered_set<std::string> m_loadedPackPrefixes;

        static HostServer *self;
    };

}

#endif // LORE_MODULES_HOSTRT_HOSTSERVER_H
