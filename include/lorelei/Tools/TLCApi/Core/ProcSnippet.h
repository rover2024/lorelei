#ifndef LORE_TOOLS_TLCAPI_PROC_SNIPPET_H
#define LORE_TOOLS_TLCAPI_PROC_SNIPPET_H

#include <cassert>

#include <clang/AST/DeclTemplate.h>

#include <lorelei/Tools/ToolUtils/SourceLineList.h>
#include <lorelei/Tools/ToolUtils/FunctionInfo.h>
#include <lorelei/Tools/TLCApi/Global.h>

namespace lore::tool::TLC {

    class DocumentContext;

    /// ProcMessage - Per-pass temporary message object exchanged between
    /// `testProc`, `beginHandleProc`, and `endHandleProc`.
    class LORETLCAPI_EXPORT ProcMessage {
    public:
        virtual ~ProcMessage() = default;
    };

    /// ProcSnippet - Canonical per-proc model used by TLC generate passes.
    ///
    /// A `ProcSnippet` instance represents one thunk procedure candidate and owns:
    /// 1. Identity and source binding:
    ///    - Function mode: points to a concrete `clang::FunctionDecl`.
    ///    - Callback mode: stores a canonical function-pointer `QualType`.
    /// 2. Per-phase metadata:
    ///    - Descriptor metadata (`Desc`) parsed from ProcDesc specializations
    ///      (builder selection, overlay type, extra pass tags).
    ///    - Optional user-provided proc definitions for each phase.
    /// 3. Generation buffers:
    ///    - Structured text fragments (`ProcSource`) to be filled by passes,
    ///      then emitted by the command layer.
    ///
    /// Lifecycle:
    /// 1. Constructed by the command pipeline after AST collection.
    /// 2. `initialize()` resolves stable fields such as canonical type/name.
    /// 3. Builder/Guard/Misc passes mutate `m_sources`.
    /// 4. Command layer serializes `m_sources` into the output translation unit.
    ///
    /// Design note:
    /// `ProcSnippet` is intentionally a data-centric object. It does not perform
    /// AST matching by itself; AST discovery is expected to happen outside
    /// (typically in command/main), and only normalized inputs are injected here.
    class LORETLCAPI_EXPORT ProcSnippet {
    public:
        enum Kind {
            Function,
            Callback,
        };

        enum Phase {
            Entry,
            Caller,
            NumPhases,
        };

        struct Desc {
            std::optional<int> builderID;
            std::optional<clang::QualType> overlayType;
            std::optional<clang::QualType> builderPass;
            llvm::SmallVector<clang::QualType, 10> passes;
        };

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

        /// Constructs from a function declaration
        ProcSnippet(
            Kind kind, const clang::FunctionDecl *FD, std::string nameHint,
            std::array<Desc, NumPhases> descs,
            std::array<const clang::ClassTemplateSpecializationDecl *, NumPhases> definitions,
            DocumentContext &documentContext)
            : m_kind(kind), m_functionDecl(FD), m_descs(std::move(descs)),
              m_definitions(std::move(definitions)), m_doc(documentContext) {
            assert(isFunction());
            initialize(nameHint);
        }

        /// Constructs from a function pointer type
        ProcSnippet(
            Kind kind, clang::QualType functionPointerType, std::array<Desc, NumPhases> descs,
            std::array<const clang::ClassTemplateSpecializationDecl *, NumPhases> definitions,
            DocumentContext &documentContext)
            : m_kind(kind), m_functionPointerType(std::move(functionPointerType)),
              m_descs(std::move(descs)), m_definitions(std::move(definitions)),
              m_doc(documentContext) {
            assert(isCallback());
            initialize({});
        }

        ProcSnippet(const ProcSnippet &) = delete;
        ProcSnippet &operator=(const ProcSnippet &) = delete;

    public:
        inline Kind kind() const {
            return m_kind;
        }
        inline bool isFunction() const {
            return m_kind == Function;
        }
        inline bool isCallback() const {
            return m_kind == Callback;
        }
        inline DocumentContext &document() const {
            return m_doc;
        }
        inline const clang::FunctionDecl *functionDecl() const {
            return m_functionDecl;
        }
        inline clang::QualType functionPointerType() const {
            return m_functionPointerType;
        }
        bool hasDefinition(Phase phase) const {
            return m_definitions[phase] != nullptr;
        }
        inline const clang::ClassTemplateSpecializationDecl *definition(Phase phase) const {
            return m_definitions[phase];
        }

    protected:
        void initialize(const std::string &nameHint);

        Kind m_kind;
        const clang::FunctionDecl *m_functionDecl = nullptr;
        clang::QualType m_functionPointerType;
        DocumentContext &m_doc;

        /// Initialized in \c initialize()
        std::string m_name;
        std::array<Desc, NumPhases> m_descs; // Entry/Caller
        std::array<const clang::ClassTemplateSpecializationDecl *, NumPhases>
            m_definitions; // Entry/Caller
        clang::QualType m_realFunctionPointerType;
        FunctionTypeView m_realFunctionTypeView;

        /// Generated by passes
        std::array<ProcSource, NumPhases> m_sources; // Entry/Caller
    };

}

#endif // LORE_TOOLS_TLCAPI_PROC_SNIPPET_H
