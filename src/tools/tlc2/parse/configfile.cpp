#include "configfile.h"

#include <sstream>

#include <llvm/Support/MemoryBuffer.h>

using namespace llvm;

namespace TLC {

    Error ConfigFile::load(const std::filesystem::path &path) {
        auto buffer = MemoryBuffer::getFile(path.string());
        if (std::error_code ec = buffer.getError()) {
            return createStringError(ec, "Failed to open file: %s", ec.message().c_str());
        }

        decltype(_symbols) symbols;
        {
            std::istringstream iss(buffer->get()->getBuffer().str());
            std::string line;
            int i = 0;
            while (std::getline(iss, line)) {
                line = StringRef(line).trim().str();
                if (line.empty()) {
                    continue;
                }
                symbols.insert(line);
            }
        }
        _symbols = std::move(symbols);
        return Error::success();
    }

}