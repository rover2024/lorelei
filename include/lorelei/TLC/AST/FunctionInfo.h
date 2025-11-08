#ifndef FUNCTIONDECLINFO_H
#define FUNCTIONDECLINFO_H

#include <clang/AST/Decl.h>

#include <lorelei/TLC/AST/FunctionTypeView.h>

namespace TLC {

    /// FunctionInfo - Represents the return type and argument list of a function declaration.
    class LORELIBTLC_EXPORT FunctionInfo {
    public:
        inline FunctionInfo() = default;

        inline FunctionInfo(const clang::FunctionDecl *decl) {
            _returnType = decl->getReturnType();
            _variadic = decl->isVariadic();
            _args.reserve(decl->param_size());
            for (auto param : decl->parameters()) {
                _args.emplace_back(param->getType(), param->getName().str());
            }
        }

        FunctionInfo(const FunctionTypeView &view);

        inline FunctionInfo(clang::QualType returnType,
                            llvm::SmallVector<std::pair<clang::QualType, std::string>> args,
                            bool variadic = false)
            : _returnType(returnType), _args(std::move(args)), _variadic(variadic) {
        }

    public:
        clang::QualType returnType() const {
            return _returnType;
        }

        void setReturnType(clang::QualType type) {
            _returnType = type;
        }

        llvm::ArrayRef<std::pair<clang::QualType, std::string>> arguments() const {
            return _args;
        }

        void setArguments(llvm::SmallVector<std::pair<clang::QualType, std::string>> args) {
            _args = std::move(args);
        }

        llvm::SmallVector<std::pair<clang::QualType, std::string>> &argumentsRef() {
            return _args;
        }

        clang::QualType argumentType(unsigned i) const {
            return _args[i].first;
        }

        const std::string &argumentName(unsigned i) const {
            return _args[i].second;
        }

        std::string metaArgumentType(const std::string &name) const {
            auto it = _metaArgTypes.find(name);
            if (it == _metaArgTypes.end()) {
                return {};
            }
            return it->second;
        }
        void setMetaArgumentType(const std::string &name, const std::string &type) {
            _metaArgTypes[name] = type;
        }

        std::string metaRetType() const {
            return _metaRetType;
        }

        void setMetaRetType(const std::string &type) {
            _metaRetType = type;
        }

        bool isVariadic() const {
            return _variadic;
        }

        void setVariadic(bool variadic) {
            _variadic = variadic;
        }

        std::string declText(const std::string &name, clang::ASTContext &ast) const;

    protected:
        clang::QualType _returnType;
        llvm::SmallVector<std::pair<clang::QualType, std::string>> _args;
        std::map<std::string, std::string> _metaArgTypes;
        std::string _metaRetType;
        bool _variadic = false;
    };

}

#endif // FUNCTIONDECLINFO_H
