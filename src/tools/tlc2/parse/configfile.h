#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <string>
#include <filesystem>
#include <set>

#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Error.h>

namespace TLC {

    class ConfigFile {
    public:
        ConfigFile() = default;
        ~ConfigFile() = default;

    public:
        llvm::Error load(const std::filesystem::path &path);

        const std::set<std::string> &symbols() const {
            return _symbols;
        }

    protected:
        std::set<std::string> _symbols;
    };

}

#endif // CONFIGFILE_H