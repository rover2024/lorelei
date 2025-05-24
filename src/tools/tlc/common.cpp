#include "common.h"

namespace TLC {

    std::string replace(std::string str, const std::string &from, const std::string &to) {
        if (from.empty())
            return str;

        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // 跳过已替换的部分
        }
        return str;
    }

    std::string getTypeString(const clang::QualType &type) {
        auto res = type.getAsString();
        res = replace(res, "struct __va_list_tag[1]", "va_list");
        res = replace(res, "struct __va_list_tag *", "va_list");
        return res;
    }

    std::string getTypeStringAsDecl(const clang::QualType &type) {
        auto res = getTypeString(type);
        if (type->isFunctionNoProtoType() || type->isFunctionProtoType() || type->isArrayType()) {
            return "__typeof__(" + res + ")";
        }
        return res;
    }

    std::string getCallArgsString(int n, bool PrefixComma) {
        std::string s;
        if (n == 0) {
            return {};
        }
        s = PrefixComma ? ", arg1" : "arg1";
        for (int i = 2; i <= n; i++) {
            s += ", arg" + std::to_string(i);
        }
        return s;
    }
}