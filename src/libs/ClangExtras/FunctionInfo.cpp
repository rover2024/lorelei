// SPDX-License-Identifier: MIT

#include "FunctionInfo.h"

#include "TypeUtils.h"

namespace lore::tool {

    FunctionInfo::FunctionInfo(const FunctionTypeView &view) {
        m_returnType = view.returnType();
        m_variadic = view.isVariadic();
        const auto argTypes = view.argTypes();
        m_args.reserve(argTypes.size());
        // Synthesize 1-based names ("arg1", "arg2", ...) since a type view carries no names.
        for (size_t i = 0; i < argTypes.size(); i++) {
            m_args.emplace_back(argTypes[i], "arg" + std::to_string(i + 1));
        }
    }

    std::string FunctionInfo::declText(const std::string &name, clang::ASTContext &ast) const {
        const auto &getArgDecl = [&](const std::pair<clang::QualType, std::string> &arg) {
            // A null QualType marks a "meta" argument whose spelling is carried as a raw string in
            // m_metaArgTypes (keyed by the arg name) rather than as a real AST type.
            if (arg.first.isNull()) {
                auto it = m_metaArgTypes.find(arg.second);
                if (it != m_metaArgTypes.end()) {
                    return it->second + " " + arg.second;
                }
            }
            return getTypeStringWithName(arg.first, arg.second);
        };
        // Same convention as the args: a null return type falls back to the meta spelling string.
        std::string decl =
            (m_returnType.isNull() ? m_metaRetType : getTypeStringDecompound(m_returnType)) + " " +
            name + "(";
        if (!m_args.empty()) {
            decl += getArgDecl(m_args[0]);
            for (size_t i = 1; i < m_args.size(); i++) {
                decl += ", " + getArgDecl(m_args[i]);
            }
        }
        if (m_variadic) {
            if (!m_args.empty()) {
                decl += ", ";
            }
            decl += "...";
        }
        decl += ")";
        return decl;
    }

}
