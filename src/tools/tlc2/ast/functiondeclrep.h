#ifndef FUNCTIONDECLREP_H
#define FUNCTIONDECLREP_H

#include <optional>

#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Error.h>
#include <clang/AST/Type.h>
#include <clang/AST/Decl.h>

#include "typerep.h"

namespace TLC {

    /// \class FunctionDeclRep
    /// \brief A representation of a function pointer type or a function declaration.
    class FunctionDeclRep {
    public:
        FunctionDeclRep() = default;

        /// Construct from a FunctionTypeRep.
        FunctionDeclRep(FunctionTypeRep typeRep, std::string name,
                        llvm::SmallVector<std::string> argNames)
            : _typeRep(std::move(typeRep)), _name(std::move(name)), _argNames(std::move(argNames)) {
        }

        /// Construct from a FunctionDecl.
        FunctionDeclRep(const clang::FunctionDecl *FD)
            : _qt(FD->getType()), _fd(FD), _typeRep(FD), _name(FD->getNameAsString()),
              _argNames([](const clang::FunctionDecl *FD) {
                  llvm::SmallVector<std::string> argNames;
                  for (const auto &param : FD->parameters()) {
                      argNames.push_back(param->getNameAsString());
                  }
                  return argNames;
              }(FD)) {
        }

        /// Construct from a QualType.
        /// \note Only FunctionProtoType and FunctionNoProtoType or their pointers are valid input
        /// types.
        static llvm::Expected<FunctionDeclRep>
            fromQualType(const clang::QualType &QT, std::string name,
                         llvm::SmallVector<std::string> argNames = {});

    public:
        const std::optional<clang::QualType> &qualType() const {
            return _qt;
        }
        const FunctionTypeRep &typeRep() const {
            return _typeRep;
        }
        const clang::FunctionDecl *decl() const {
            return _fd;
        }
        const std::string &name() const {
            return _name;
        }
        llvm::ArrayRef<std::string> argNames() const {
            return _argNames;
        }
        const std::string &argName(int i) const {
            static const std::string empty;
            return (i >= _argNames.size() || i < 0) ? empty : _argNames[i];
        }

    protected:
        std::optional<clang::QualType> _qt;
        const clang::FunctionDecl *_fd = nullptr;
        FunctionTypeRep _typeRep;
        std::string _name;
        llvm::SmallVector<std::string> _argNames;
    };

}

#endif // FUNCTIONDECLREP_H