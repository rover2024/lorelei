#include "FunctionInfo.h"

#include "TypeUtils.h"

namespace lore::tool {

    FunctionInfo::FunctionInfo(const FunctionTypeView &view) {
        m_returnType = view.returnType();
        m_variadic = view.isVariadic();
        m_args.reserve(view.argTypes().size());

        int i = 0;
        for (auto argType : view.argTypes()) {
            i++;
            m_args.emplace_back(argType, "arg" + std::to_string(i));
        }
    }

    std::string FunctionInfo::declText(const std::string &name, clang::ASTContext &ast) const {
        const auto &getArgDecl = [&](const std::pair<clang::QualType, std::string> &arg) {
            if (arg.first.isNull()) {
                auto it = m_metaArgTypes.find(arg.second);
                if (it != m_metaArgTypes.end()) {
                    return it->second + " " + arg.second;
                }
            }
            return getTypeStringWithName(arg.first, arg.second);
        };
        std::string decl =
            (m_returnType.isNull() ? m_metaRetType : getTypeStringDecompound(m_returnType)) + " " +
            name + "(";
        if (m_args.size() > 0) {
            decl += getArgDecl(m_args[0]);
            for (size_t i = 1; i < m_args.size(); i++) {
                decl += ", " + getArgDecl(m_args[i]);
            }
        }
        if (m_variadic) {
            if (m_args.size() > 0) {
                decl += ", ";
            }
            decl += "...";
        }
        decl += ")";
        return decl;
    }

}
