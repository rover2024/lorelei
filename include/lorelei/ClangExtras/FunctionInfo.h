// SPDX-License-Identifier: MIT

#ifndef LORE_CLANGEXTRAS_FUNCTIONINFO_H
#define LORE_CLANGEXTRAS_FUNCTIONINFO_H

#include <clang/AST/Decl.h>

#include <lorelei/ClangExtras/FunctionTypeView.h>

namespace lore::tool {

    /// FunctionInfo - A mutable model of a function's signature: return type and named arguments.
    ///
    /// Unlike FunctionTypeView it keeps argument names and a couple of side-channel "meta" type
    /// strings that the thunk passes attach to the return value and individual arguments.
    class FunctionInfo {
    public:
        inline FunctionInfo() = default;

        /// Builds from a function declaration, capturing each parameter's type and name.
        inline FunctionInfo(const clang::FunctionDecl *FD) {
            m_returnType = FD->getReturnType();
            m_variadic = FD->isVariadic();
            m_args.reserve(FD->param_size());
            for (auto param : FD->parameters()) {
                m_args.emplace_back(param->getType(), param->getName().str());
            }
        }

        /// Builds from a FunctionTypeView. Argument names are left empty.
        FunctionInfo(const FunctionTypeView &view);

        /// Builds from an explicit return type, (type, name) argument list, and variadic flag.
        inline FunctionInfo(clang::QualType returnType,
                            llvm::SmallVector<std::pair<clang::QualType, std::string>> args,
                            bool variadic = false)
            : m_returnType(returnType), m_args(std::move(args)), m_variadic(variadic) {
        }

    public:
        /// The return type.
        clang::QualType returnType() const {
            return m_returnType;
        }
        void setReturnType(clang::QualType type) {
            m_returnType = type;
        }

        /// The (type, name) pairs of the arguments.
        llvm::ArrayRef<std::pair<clang::QualType, std::string>> arguments() const {
            return m_args;
        }
        void setArguments(llvm::SmallVector<std::pair<clang::QualType, std::string>> args) {
            m_args = std::move(args);
        }
        /// Mutable access to the argument list (so passes can rewrite it in place).
        llvm::SmallVector<std::pair<clang::QualType, std::string>> &argumentsRef() {
            return m_args;
        }

        /// The type of the \a i-th argument.
        clang::QualType argumentType(unsigned i) const {
            return m_args[i].first;
        }
        /// The name of the \a i-th argument.
        const std::string &argumentName(unsigned i) const {
            return m_args[i].second;
        }

        /// The meta (reflected) type string attached to argument \a name, or empty if unset.
        std::string metaArgumentType(const std::string &name) const {
            auto it = m_metaArgTypes.find(name);
            if (it == m_metaArgTypes.end()) {
                return {};
            }
            return it->second;
        }
        void setMetaArgumentType(const std::string &name, const std::string &type) {
            m_metaArgTypes[name] = type;
        }

        /// The meta (reflected) type string attached to the return value.
        std::string metaRetType() const {
            return m_metaRetType;
        }
        void setMetaRetType(const std::string &type) {
            m_metaRetType = type;
        }

        /// Whether the function is variadic.
        bool isVariadic() const {
            return m_variadic;
        }
        void setVariadic(bool variadic) {
            m_variadic = variadic;
        }

        /// Renders a C declaration of this function named \a name.
        std::string declText(const std::string &name, clang::ASTContext &ast) const;

    protected:
        clang::QualType m_returnType;
        llvm::SmallVector<std::pair<clang::QualType, std::string>> m_args;
        std::map<std::string, std::string> m_metaArgTypes; // argument name -> meta type string
        std::string m_metaRetType;
        bool m_variadic = false;
    };

}

#endif // LORE_CLANGEXTRAS_FUNCTIONINFO_H
