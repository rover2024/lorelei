#ifndef FUNCTIONTYPE_H
#define FUNCTIONTYPE_H

#include <clang/AST/Type.h>

#include <lorelei/TLC/Global.h>

namespace TLC {

    /// \class FunctionTypeView
    /// \brief A raw view of a function QualType.
    class FunctionTypeView {
    public:
        /// Constructs from a \c QualType.
        explicit FunctionTypeView(clang::QualType qt) {
            qt = qt.getCanonicalType();
            if (qt->isFunctionPointerType()) {
                qt = qt->getPointeeType();
            }
            if (qt->isFunctionProtoType()) {
                const clang::FunctionProtoType *FPT = qt->getAs<clang::FunctionProtoType>();
                _returnType = FPT->getReturnType();
                _argTypes = FPT->getParamTypes();
                _variadic = FPT->isVariadic();
            } else if (qt->isFunctionNoProtoType()) {
                const clang::FunctionNoProtoType *FNT = qt->getAs<clang::FunctionNoProtoType>();
                _returnType = FNT->getReturnType();
                _variadic = false;
            } else {
                assert(false);
            }
        }

        /// Constructs from the return type, argument types, and variadic flag.
        FunctionTypeView(const clang::QualType &returnType,
                         llvm::ArrayRef<clang::QualType> argTypes, bool variadic)
            : _returnType(returnType), _argTypes(argTypes), _variadic(variadic) {
        }

        /// Default constructor.
        FunctionTypeView() = default;

    public:
        const clang::QualType &returnType() const {
            return _returnType;
        }
        llvm::ArrayRef<clang::QualType> argTypes() const {
            return _argTypes;
        }
        bool isVariadic() const {
            return _variadic;
        }

        LORELIBTLC_EXPORT clang::QualType buildQualType(clang::ASTContext &ctx) const;

    protected:
        clang::QualType _returnType;
        llvm::ArrayRef<clang::QualType> _argTypes;
        bool _variadic = false;
    };

}

#endif // FUNCTIONTYPE_H
