#ifndef TYPEREP_H
#define TYPEREP_H

#include <clang/AST/Type.h>

namespace TLC {

    /// \class TypeRep
    /// \brief A common representation of a type。
    class TypeRep {
    public:
        enum Type {
            TypeSpec,
            String,
        };

        inline TypeRep() : TypeRep(std::string()) {
        }

        /// Construct from a qualified type.
        inline TypeRep(const clang::QualType &QT) {
            _type = TypeSpec;
            new (&_storage.qt) clang::QualType(QT);
        }

        /// Construct from a string.
        inline TypeRep(std::string s) {
            _type = String;
            new (&_storage.s) std::string(std::move(s));
        }
        inline TypeRep(const char *s) : TypeRep(std::string(s)) {
        }

        /// Destructor.
        inline ~TypeRep() {
            destroy();
        }

        /// Copy constructor.
        inline TypeRep(const TypeRep &RHS) {
            copyConstruct(RHS);
        }

        /// Move constructor.
        inline TypeRep(TypeRep &&RHS) {
            moveConstruct(RHS);
        }

        /// Copy assignment operator.
        inline TypeRep &operator=(const TypeRep &RHS) {
            if (this != &RHS) {
                destroy();
                copyConstruct(RHS);
            }
            return *this;
        }

        /// Move assignment operator.
        inline TypeRep &operator=(TypeRep &&RHS) {
            if (this != &RHS) {
                destroy();
                moveConstruct(RHS);
            }
            return *this;
        }

    public:
        inline Type type() const {
            return static_cast<Type>(_type);
        }
        inline bool isTypeSpec() const {
            return _type == TypeSpec;
        }
        inline bool isString() const {
            return _type == String;
        }
        inline const clang::QualType &toQualType() const {
            return _storage.qt;
        }
        inline const std::string &toString() const {
            return _storage.s;
        }
        inline std::string &toString() {
            return _storage.s;
        }

        std::string getAsString() const;

    protected:
        inline void copyConstruct(const TypeRep &RHS) {
            _type = RHS._type;
            if (_type == TypeSpec) {
                new (&_storage.qt) clang::QualType(RHS._storage.qt);
            } else if (_type == String) {
                new (&_storage.s) std::string(RHS._storage.s);
            }
        }

        inline void moveConstruct(TypeRep &RHS) {
            _type = RHS._type;
            if (_type == TypeSpec) {
                new (&_storage.qt) clang::QualType(std::move(RHS._storage.qt));
            } else if (_type == String) {
                new (&_storage.s) std::string(std::move(RHS._storage.s));
            }
        }

        inline void destroy() {
            if (_type == String) {
                _storage.s.~basic_string();
            } else {
                _storage.qt.~QualType();
            }
        }

        union Storage {
            clang::QualType qt;
            std::string s;
            Storage() {};
            ~Storage() {};
        } _storage;
        int _type;
    };


    /// \class FunctionTypeRep
    /// \brief A common representation of a function type。
    class FunctionTypeRep {
    public:
        FunctionTypeRep() {
        }

        /// Construct from all the necessary information.
        FunctionTypeRep(const TypeRep &returnType, llvm::SmallVector<TypeRep> argTypes,
                        bool variadic)
            : _returnType(returnType), _argTypes(std::move(argTypes)), _variadic(variadic) {
        }

        /// Construct from a FunctionNoProtoType.
        FunctionTypeRep(const clang::FunctionNoProtoType *FNT);

        /// Construct from a FunctionProtoType.
        FunctionTypeRep(const clang::FunctionProtoType *FTP);

        /// Construct from a FunctionDecl.
        FunctionTypeRep(const clang::FunctionDecl *FD);

    public:
        const TypeRep &returnType() const {
            return _returnType;
        }
        void setReturnType(TypeRep returnType) {
            _returnType = std::move(returnType);
        }

        const llvm::SmallVector<TypeRep> &argTypes() const {
            return _argTypes;
        }
        void setArgTypes(llvm::SmallVector<TypeRep> argTypes) {
            _argTypes = std::move(argTypes);
        }

        bool isVariadic() const {
            return _variadic;
        }
        void setVariadic(bool variadic) {
            _variadic = variadic;
        }

    protected:
        TypeRep _returnType;
        llvm::SmallVector<TypeRep> _argTypes;
        bool _variadic = false;
    };


    /// \class FunctionTypeView
    /// \brief A raw view of a function QualType.
    class FunctionTypeView {
    public:
        FunctionTypeView(const clang::QualType &qt) {
            if (qt->isFunctionProtoType()) {
                const clang::FunctionProtoType *FPT = qt->getAs<clang::FunctionProtoType>();
                _returnType = FPT->getReturnType();
                _argTypes = FPT->getParamTypes();
                _variadic = FPT->isVariadic();
            } else if (qt->isFunctionNoProtoType()) {
                const clang::FunctionNoProtoType *FNT = qt->getAs<clang::FunctionNoProtoType>();
                _returnType = FNT->getReturnType();
                _variadic = false;
            }
        }

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

    protected:
        clang::QualType _returnType;
        llvm::ArrayRef<clang::QualType> _argTypes;
        bool _variadic;
    };

}

#endif // TYPEREP_H
