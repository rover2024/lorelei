#ifndef LORE_TOOLS_TLCAPI_FUNCTIONTYPEVIEW_H
#define LORE_TOOLS_TLCAPI_FUNCTIONTYPEVIEW_H

#include <clang/AST/Type.h>

namespace lore::tool {

    /// \class FunctionTypeView
    /// \brief A raw view of a function QualType.
    class FunctionTypeView {
    public:
        /// Constructs from a \c QualType.
        explicit FunctionTypeView(clang::QualType T) {
            T = T.getCanonicalType();
            if (T->isFunctionPointerType()) {
                T = T->getPointeeType();
            }
            if (T->isFunctionProtoType()) {
                const clang::FunctionProtoType *FPT = T->getAs<clang::FunctionProtoType>();
                m_returnType = FPT->getReturnType();
                m_argTypes = FPT->getParamTypes();
                m_variadic = FPT->isVariadic();
            } else if (T->isFunctionNoProtoType()) {
                const clang::FunctionNoProtoType *FNT = T->getAs<clang::FunctionNoProtoType>();
                m_returnType = FNT->getReturnType();
                m_variadic = false;
            } else {
                assert(false);
            }
        }

        /// Constructs from the return type, argument types, and variadic flag.
        FunctionTypeView(const clang::QualType &returnType,
                         llvm::ArrayRef<clang::QualType> argTypes, bool variadic)
            : m_returnType(returnType), m_argTypes(argTypes), m_variadic(variadic) {
        }

        /// Default constructor.
        FunctionTypeView() = default;

    public:
        const clang::QualType &returnType() const {
            return m_returnType;
        }
        llvm::ArrayRef<clang::QualType> argTypes() const {
            return m_argTypes;
        }
        bool isVariadic() const {
            return m_variadic;
        }

        clang::QualType buildQualType(clang::ASTContext &ctx) const;

    protected:
        clang::QualType m_returnType;
        llvm::ArrayRef<clang::QualType> m_argTypes;
        bool m_variadic = false;
    };

}

#endif // LORE_TOOLS_TLCAPI_FUNCTIONTYPEVIEW_H
