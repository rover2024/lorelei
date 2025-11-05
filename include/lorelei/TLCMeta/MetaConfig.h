#ifndef METACONFIG_H
#define METACONFIG_H

namespace lorethunk {

    enum MetaConfigScope {
        MCS_Builtin = 0,
        MCS_User,
    };

    template <MetaConfigScope Scope>
    struct MetaConfig {};

}

#endif // METACONFIG_H
