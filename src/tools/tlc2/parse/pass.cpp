#include "pass.h"

#include <charconv>

namespace TLC {

    IntOrString IntOrString::fromAnyString(const std::string &s) {
        int i;
        auto result = std::from_chars(s.data(), s.data() + s.size(), i);
        if (result.ec != std::errc()) {
            return s;
        }
        return i;
    }

    std::map<std::string, Pass *> &Pass::passMap(Stage stage) {
        static std::map<std::string, Pass *> passes[3];
        return passes[stage];
    }

    void Pass::initialize(Analyzer *analyzer) {
    }

}