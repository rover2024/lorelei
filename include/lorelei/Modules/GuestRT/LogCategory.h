// SPDX-License-Identifier: MIT

#ifndef LORE_MODULES_GUESTRT_LOGCATEGORY_H
#define LORE_MODULES_GUESTRT_LOGCATEGORY_H

#include <lorelei/Support/Logging.h>

namespace lore::log {

    /// The guest runtime's logging category ("lorelei.guest-rt"). Use it in the member form, e.g.
    /// lore::log::logger().loreWarning(...), so every record carries this category and the host
    /// sink renders it as a "name: " prefix, keeping the origin out of the message text. Records
    /// from the thunk context, which runs inside the guest runtime, use it too. logger().name()
    /// gives the bare string for the guest runtime's pre-callback bootstrap diagnostics.
    inline LogCategory &logger() {
        static LogCategory category("lorelei.guest-rt");
        return category;
    }

}

#endif // LORE_MODULES_GUESTRT_LOGCATEGORY_H
