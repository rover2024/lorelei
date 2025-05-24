#include "pass.h"

namespace TLC {

    static std::map<std::string, Pass *> _globalPassMap;

    Pass::Pass(const std::string &identifier): m_identifier(identifier) {
        _globalPassMap[identifier] = this;
    }

    Pass::~Pass() {
        _globalPassMap.erase(m_identifier);
    }

    const std::map<std::string, Pass *> &Pass::globalPassMap() {
        return _globalPassMap;
    }

}