#include "Pass.h"

namespace TLC {

    std::map<int, Pass *> &Pass::passMap(Phase phase) {
        static std::map<int, Pass *> passes[3];
        return passes[phase];
    }

}