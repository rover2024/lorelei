#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

#include <string>

#include <lorelei/Core/Global.h>

namespace TLC {

    /// BasePlugin - The base class for all TLC plugins.
    /// \note The plugin should export a function as follows for the loader.
    /// \code
    ///     extern "C" TLC::BasePlugin *TLC_PluginInstance();
    /// \endcode
    class BasePlugin {
    public:
        virtual ~BasePlugin() = default;

    public:
        /// Called at the very beginning of the tool, before any file is processed.
        virtual void initialize() {};

        /// Returns the unique key of the plugin.
        virtual std::string key() const = 0;
    };

}

#define TLC_PLUGIN_EXPORT(T)                                                                       \
    extern "C" LORECORE_DECL_EXPORT TLC::BasePlugin *TLC_PluginInstance() {                        \
        static T instance;                                                                         \
        return &instance;                                                                          \
    }

#endif // BASEPLUGIN_H
