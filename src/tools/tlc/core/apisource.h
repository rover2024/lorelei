#ifndef APISOURCE_H
#define APISOURCE_H

#include <clang/AST/AST.h>

#include "core/functionsource.h"

namespace TLC {

    class PassContext;

    class ApiSource {
    public:
        enum Type {
            Normal,
            GuestCallback,
            HostCallback,
        };

        ApiSource(Type T,                                                           //
                  PassContext *Parent,                                              //
                  llvm::StringRef Name,                                             //
                  const clang::QualType &QT,                                        //
                  const llvm::ArrayRef<const clang::FunctionDecl *> &FunctionDecls, // nullable
                  const clang::FunctionDecl *OrgFunctionDecl,                       // nullable
                  const clang::FunctionDecl *HintFunctionDecl                       // nullable
        );
        ~ApiSource();

        Type getApiType() const;
        PassContext *getParent() const;

        clang::QualType getApiReturnType() const;
        llvm::ArrayRef<clang::QualType> getApiArgumentTypes() const;
        bool getApiVariadic() const;

        /// Properties
        llvm::StringRef getName() const;
        clang::QualType getType() const;
        const clang::FunctionDecl *getFunctionDecl(FunctionSource::Type type) const;
        const clang::FunctionDecl *getOrgFunctionDecl() const;
        const clang::FunctionDecl *getHintFunctionDecl() const;

        struct Annotation {
            llvm::StringRef Name;
            llvm::ArrayRef<std::string> Arguments;
        };
        int getHintAnnotationCount() const;
        Annotation getHintAnnotation(int n) const;

        /// Text data
        void prependForward(llvm::StringRef lines, bool guest);
        void appendForward(llvm::StringRef lines, bool guest);

        void prependBackward(llvm::StringRef lines, bool guest);
        void appendBackward(llvm::StringRef lines, bool guest);

        /// Children
        FunctionSource &getFunctionSource(FunctionSource::Type type);

        /// Result data
        std::string getText(bool guest) const;

        /// Metadata
        llvm::StringRef getAttribute(llvm::StringRef key) const;
        void setAttribute(llvm::StringRef key, llvm::StringRef value);

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        friend class PassContext;
        friend class FunctionSource;
    };

}

#endif // APISOURCE_H