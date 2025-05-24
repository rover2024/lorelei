#ifndef FUNCTIONSOURCE_H
#define FUNCTIONSOURCE_H

#include <clang/AST/AST.h>

namespace TLC {

    class ApiSource;

    class FunctionSource {
    public:
        enum Type {
            GTP,
            GTP_IMPL,
            HTP,
            HTP_IMPL,
        };

        FunctionSource(Type T,                                 //
                       ApiSource *Parent,                      //
                       llvm::StringRef Name,                   //
                       const clang::FunctionDecl *FunctionDecl // nullable
        );
        ~FunctionSource();

        Type getFunctionType() const;
        ApiSource *getParent() const;

        /// Properties
        llvm::StringRef getName() const;
        const clang::FunctionDecl *getFunctionDecl() const;

        /// Function literal data, used if declaration is not available
        struct Param {
            std::string TypeName;
            std::string Name;
            std::string MetadataTypeName; // if valid, the `TypeName` will store the original type name
        };
        clang::ArrayRef<Param> getArguments() const;
        void setArguments(const llvm::ArrayRef<Param> &args);

        const Param &getReturnValue() const;
        void setReturnValue(const Param &returnValue);

        bool isVariadic() const;
        void setVariadic(bool variadic) const;

        std::string getFunctionSignature(llvm::StringRef name) const;

        /// Text data
        llvm::StringRef getProlog() const;
        void setProlog(llvm::StringRef lines);

        llvm::StringRef getEpilog() const;
        void setEpilog(llvm::StringRef lines);

        llvm::StringRef getBody() const;
        void setBody(llvm::StringRef lines);

        int forwardCount() const;
        void prependForward(llvm::StringRef lines);
        void appendForward(llvm::StringRef lines);

        int backwardCount() const;
        void prependBackward(llvm::StringRef lines);
        void appendBackward(llvm::StringRef lines);

        bool isEmpty() const;

        // Result data
        std::string getContent() const;
        std::string getText() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        friend class ApiSource;
    };

}

#endif // FUNCTIONSOURCE_H