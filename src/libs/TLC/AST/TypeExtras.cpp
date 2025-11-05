#include "TypeExtras.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>

using namespace clang;

namespace TLC {

    static std::string replace(std::string str, const std::string &from, const std::string &to) {
        if (from.empty())
            return str;

        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return str;
    }

    std::string getTypeString(const QualType &type) {
        auto res = type.getAsString();
        res = replace(res, "struct __va_list_tag[1]", "va_list");
        res = replace(res, "struct __va_list_tag *", "va_list");
        return res;
    }

    static bool isCompound(QualType type) {
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
        return type->isArrayType() || type->isFunctionType();
    }

    std::string getTypeStringDecompound(const QualType &type) {
        std::string ret;
        if (isCompound(type)) {
            ret = "decltype(" + getTypeString(type) + ")";
        } else {
            ret = getTypeString(type);
        }
        return ret;
    }

    std::string getTypeStringWithName(const QualType &type, const std::string &name) {
        auto typeStr = getTypeStringDecompound(type);
        if (!StringRef(typeStr).ends_with("*"))
            typeStr += " ";
        return typeStr + name;
    }

}