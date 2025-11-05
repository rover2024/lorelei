#include "FunctionInfo.h"

#include "TypeExtras.h"

namespace TLC {

    FunctionInfo::FunctionInfo(const FunctionTypeView &view) {
        _returnType = view.returnType();
        _variadic = view.isVariadic();
        _args.reserve(view.argTypes().size());

        int i = 0;
        for (auto argType : view.argTypes()) {
            i++;
            _args.emplace_back(argType, "arg" + std::to_string(i));
        }
    }

    std::string FunctionInfo::declText(const std::string &name, clang::ASTContext &ast) const {
        const auto &getArgDecl = [&](const std::pair<clang::QualType, std::string> &arg) {
            if (arg.first.isNull()) {
                auto it = _metaArgTypes.find(arg.second);
                if (it != _metaArgTypes.end()) {
                    return it->second + " " + arg.second;
                }
            }
            return getTypeStringWithName(arg.first, arg.second);
        };
        std::string decl =
            (_returnType.isNull() ? _metaRetType : getTypeStringDecompound(_returnType)) + " " +
            name + "(";
        if (_args.size() > 0) {
            decl += getArgDecl(_args[0]);
            for (size_t i = 1; i < _args.size(); i++) {
                decl += ", " + getArgDecl(_args[i]);
            }
        }
        if (_variadic) {
            if (_args.size() > 0) {
                decl += ", ";
            }
            decl += "...";
        }
        decl += ")";
        return decl;
    }

}