// SPDX-License-Identifier: MIT

#include "NextLibrary.h"

namespace lore::utils {

    std::string resolveNextLibrary(const std::string &next, const char *ownPath) {
        if (next.find('/') == std::string::npos) {
            return next;
        }
        if (!next.empty() && next.front() == '/') {
            return next;
        }
        std::string dir(ownPath ? ownPath : "");
        auto slash = dir.find_last_of('/');
        dir = (slash == std::string::npos) ? std::string(".") : dir.substr(0, slash);
        return dir + "/" + next;
    }

    std::string nextLibraryByName(const char *ownPath, bool hostThunk) {
        std::string base(ownPath ? ownPath : "");
        if (auto slash = base.find_last_of('/'); slash != std::string::npos) {
            base = base.substr(slash + 1);
        }
        // Strip the rightmost ".so" and anything after it, so a versioned soname such as "libz.so.1"
        // yields "libz".
        if (auto so = base.rfind(".so"); so != std::string::npos) {
            base.resize(so);
        }
        if (hostThunk) {
            if (const std::string s = "_HTL";
                base.size() >= s.size() && base.compare(base.size() - s.size(), s.size(), s) == 0) {
                base.resize(base.size() - s.size());
            }
            return base + ".so";
        }
        return base + "_HTL.so";
    }

}
