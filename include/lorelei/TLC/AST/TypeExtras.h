#ifndef TYPEEXTRAS_H
#define TYPEEXTRAS_H

#include <clang/AST/Type.h>

#include <lorelei/TLC/Global.h>

namespace TLC {

    LORELIBTLC_EXPORT std::string getTypeString(const clang::QualType &type);

    LORELIBTLC_EXPORT std::string getTypeStringDecompound(const clang::QualType &type);

    LORELIBTLC_EXPORT std::string getTypeStringWithName(const clang::QualType &type,
                                                        const std::string &name);
}

#endif // TYPEEXTRAS_H
