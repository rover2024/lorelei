#ifndef METAPASSITEM_H
#define METAPASSITEM_H

#include <clang/AST/Type.h>

#include <lorelei/TLC/Global.h>

namespace TLC {

    class LORELIBTLC_EXPORT MetaPassItem {
    public:
        MetaPassItem() = default;
        explicit MetaPassItem(const clang::QualType &type) {
            tryAssign(type);
        }

    public:
        const clang::QualType &type() const {
            return _type;
        }

        int id() const {
            return _id;
        }

        bool isBuilder() const {
            return _isBuilder;
        }

        bool isValid() const {
            return !_type.isNull();
        }

    protected:
        void tryAssign(const clang::QualType &type);

        int _id = -1;
        bool _isBuilder = false;
        clang::QualType _type;
    };

}

#endif // METAPASSITEM_H
