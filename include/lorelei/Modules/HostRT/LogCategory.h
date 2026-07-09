// SPDX-License-Identifier: MIT

#ifndef LORE_MODULES_HOSTRT_LOGCATEGORY_H
#define LORE_MODULES_HOSTRT_LOGCATEGORY_H

#include <lorelei/Support/Logging.h>

namespace lore::log {

    /// The host runtime's logging category ("lorelei.host-rt"). Use it in the member form, e.g.
    /// lore::log::logger().loreCritical(...), so every record carries this category. Records from
    /// the thunk context, which runs inside the host runtime, use it too. The host sink renders the
    /// category of each record it prints, host-native and guest-forwarded alike, as a "name: "
    /// prefix, keeping the origin out of the message text.
    inline LogCategory &logger() {
        static LogCategory category("lorelei.host-rt");
        return category;
    }

}

#endif // LORE_MODULES_HOSTRT_LOGCATEGORY_H
