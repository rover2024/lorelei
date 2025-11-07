#ifndef PROCCONTEXT_H
#define PROCCONTEXT_H

#include <cassert>

#include <lorelei/TLC/Basic/SourceLineList.h>
#include <lorelei/TLC/ASTMetaItem/MetaProcDescItem.h>
#include <lorelei/TLC/ASTMetaItem/MetaProcItem.h>
#include <lorelei/TLC/AST/FunctionInfo.h>

namespace TLC {

    class DocumentContext;

    class LORELIBTLC_EXPORT ProcContext {
    public:
        /// Constructs from a function declaration
        ProcContext(CProcKind procKind, const clang::FunctionDecl *FD, std::string nameHint,
                    DocumentContext &documentContext)
            : _procKind(procKind), _functionDecl(FD), _nameHint(nameHint),
              _documentContext(documentContext) {
            assert(isFunction());
            initialize();
        }

        /// Constructs from a callback type (function pointer type)
        ProcContext(CProcKind procKind, clang::QualType functionPointerType, std::string nameHint,
                    DocumentContext &documentContext)
            : _procKind(procKind), _functionPointerType(std::move(functionPointerType)),
              _nameHint(std::move(nameHint)), _documentContext(documentContext) {
            assert(!isFunction());
            initialize();
        }

        struct FunctionBodySource {
            SourceLineList<> prolog;
            SourceLineList<> forward;
            SourceLineList<> center;
            SourceLineList<> backward;
            SourceLineList<> epilog;
        };

        struct ProcSource {
            FunctionInfo functionInfo;
            SourceLineList<> head;
            FunctionBodySource body;
            SourceLineList<> tail;
        };

    public:
        inline bool isFunction() const {
            return _procKind == CProcKind::CProcKind_HostFunction ||
                   _procKind == CProcKind::CProcKind_GuestFunction;
        }

        /// Returns the parent document context.
        DocumentContext &documentContext() {
            return _documentContext;
        }

        CProcKind procKind() const {
            return _procKind;
        }

        /// Returns the related \c FunctionDecl, null if it's a callback proc.
        const clang::FunctionDecl *functionDecl() const {
            return _functionDecl;
        }

        /// Returns the function pointer type.
        clang::QualType functionPointerType() const {
            return _functionPointerType;
        }

        std::string name() const {
            return _name;
        }

        bool hasDesc() const {
            return _desc != nullptr;
        }

        const MetaProcDescItemBase *desc() const {
            return _desc;
        }

        const MetaProcDescItem *functionDesc() const {
            assert(isFunction());
            return static_cast<const MetaProcDescItem *>(_desc);
        }

        const MetaProcCBDescItem *callbackDesc() const {
            assert(!isFunction());
            return static_cast<const MetaProcCBDescItem *>(_desc);
        }

        bool hasDefinition(CProcThunkPhase thunkPhase) const {
            return _definitions[thunkPhase] != nullptr;
        }

        const MetaProcItem *functionDefinition(CProcThunkPhase thunkPhase) const {
            assert(isFunction());
            return static_cast<const MetaProcItem *>(_definitions[thunkPhase]);
        }

        const MetaProcCBItem *callbackDefinition(CProcThunkPhase thunkPhase) const {
            assert(!isFunction());
            return static_cast<const MetaProcCBItem *>(_definitions[thunkPhase]);
        }

        /// Returns the builder pass ID.
        std::optional<int> builderID() const {
            return _builderID;
        }

        /// Returns the overlay type.
        std::optional<clang::QualType> overlayType() const {
            return _overlayType;
        }

        /// Returns the real function pointer type.
        clang::QualType realFunctionPointerType() const {
            return _realFunctionPointerType;
        }

        /// Returns the real function type.
        FunctionTypeView realFunctionTypeView() const {
            return _realFunctionTypeView;
        }

        /// Sources to generate.
        const ProcSource &source(CProcThunkPhase thunkPhase) const {
            return _sources[thunkPhase];
        }
        ProcSource &source(CProcThunkPhase thunkPhase) {
            return _sources[thunkPhase];
        }

        /// Results.
        std::string text(CProcThunkPhase phase, bool decl, clang::ASTContext &ast) const;

    protected:
        void initialize();

        CProcKind _procKind;
        const clang::FunctionDecl *_functionDecl = nullptr;
        clang::QualType _functionPointerType;
        std::string _nameHint;
        DocumentContext &_documentContext;

        /// Initialized at \c initialize
        std::string _name;
        const MetaProcDescItemBase *_desc = nullptr;
        std::array<const MetaProcItemBase *, CProcThunkPhase_NumThunkPhase> _definitions = {};
        std::optional<int> _builderID;
        std::optional<clang::QualType> _overlayType;
        clang::QualType _realFunctionPointerType;
        FunctionTypeView _realFunctionTypeView;

        /// Generated by passes
        std::array<ProcSource, CProcThunkPhase_NumThunkPhase> _sources;
    };

}

#endif // PROCCONTEXT_H
