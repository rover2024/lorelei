#ifndef LORE_TOOLS_TLCAPI_FUNCTIONINFO_H
#define LORE_TOOLS_TLCAPI_FUNCTIONINFO_H

#include <clang/AST/Decl.h>

#include <lorelei/Tools/ToolUtils/FunctionTypeView.h>

namespace lore::tool {

    /// FunctionInfo - Represents the return type and argument list of a function declaration.
    class FunctionInfo {
    public:
        inline FunctionInfo() = default;

        inline FunctionInfo(const clang::FunctionDecl *FD) {
            m_returnType = FD->getReturnType();
            m_variadic = FD->isVariadic();
            m_args.reserve(FD->param_size());
            for (auto param : FD->parameters()) {
                m_args.emplace_back(param->getType(), param->getName().str());
            }
        }

        FunctionInfo(const FunctionTypeView &view);

        inline FunctionInfo(clang::QualType returnType,
                            llvm::SmallVector<std::pair<clang::QualType, std::string>> args,
                            bool variadic = false)
            : m_returnType(returnType), m_args(std::move(args)), m_variadic(variadic) {
        }

    public:
        clang::QualType returnType() const {
            return m_returnType;
        }

        void setReturnType(clang::QualType type) {
            m_returnType = type;
        }

        llvm::ArrayRef<std::pair<clang::QualType, std::string>> arguments() const {
            return m_args;
        }

        void setArguments(llvm::SmallVector<std::pair<clang::QualType, std::string>> args) {
            m_args = std::move(args);
        }

        llvm::SmallVector<std::pair<clang::QualType, std::string>> &argumentsRef() {
            return m_args;
        }

        clang::QualType argumentType(unsigned i) const {
            return m_args[i].first;
        }

        const std::string &argumentName(unsigned i) const {
            return m_args[i].second;
        }

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

        std::string metaRetType() const {
            return m_metaRetType;
        }

        void setMetaRetType(const std::string &type) {
            m_metaRetType = type;
        }

        bool isVariadic() const {
            return m_variadic;
        }

        void setVariadic(bool variadic) {
            m_variadic = variadic;
        }

        std::string declText(const std::string &name, clang::ASTContext &ast) const;

    protected:
        clang::QualType m_returnType;
        llvm::SmallVector<std::pair<clang::QualType, std::string>> m_args;
        std::map<std::string, std::string> m_metaArgTypes;
        std::string m_metaRetType;
        bool m_variadic = false;
    };

}

#endif // LORE_TOOLS_TLCAPI_FUNCTIONINFO_H
