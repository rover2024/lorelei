#ifndef LORE_TOOLS_TLCAPI_PROC_SNIPPET_H
#define LORE_TOOLS_TLCAPI_PROC_SNIPPET_H

#include <cassert>
#include <string>
#include <vector>

#include <clang/AST/DeclTemplate.h>

#include <lorelei/Tools/ToolUtils/SourceLineList.h>
#include <lorelei/Tools/ToolUtils/FunctionInfo.h>
#include <lorelei/Tools/TLCApi/Global.h>
#include <lorelei/Tools/TLCApi/Utils/ManifestStatistics.h>

namespace lore::tool::TLC {

    class DocumentContext;

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
            NumKinds,
        };

        enum Direction {
            GuestToHost,
            HostToGuest,
            NumDirections,
        };

        enum Phase {
            Entry,
            Caller,
            NumPhases,
        };

        struct PassInfo {
            int id = -1;
            clang::QualType type;
        };

        struct Desc {
            std::optional<clang::QualType> overlayType;
            std::optional<PassInfo> builderPass;
            llvm::SmallVector<PassInfo, 10> passes;
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
            Kind kind, Direction direction, const clang::FunctionDecl *FD, std::string nameHint,
            std::optional<Desc> desc,
            std::array<const clang::ClassTemplateSpecializationDecl *, NumPhases> definitions,
            DocumentContext &documentContext)
            : m_kind(kind), m_direction(direction), m_functionDecl(FD), m_desc(std::move(desc)),
              m_definitions(std::move(definitions)), m_doc(&documentContext) {
            assert(isFunction());
            initialize(nameHint);
        }

        /// Constructs from a function pointer type
        ProcSnippet(
            Kind kind, Direction direction, clang::QualType functionPointerType,
            std::optional<Desc> desc,
            std::array<const clang::ClassTemplateSpecializationDecl *, NumPhases> definitions,
            DocumentContext &documentContext)
            : m_kind(kind), m_direction(direction),
              m_functionPointerType(std::move(functionPointerType)), m_desc(std::move(desc)),
              m_definitions(std::move(definitions)), m_doc(&documentContext) {
            assert(isCallback());
            initialize({});
        }

        ProcSnippet(const ProcSnippet &) = delete;
        ProcSnippet &operator=(const ProcSnippet &) = delete;
        ProcSnippet(ProcSnippet &&) = default;
        ProcSnippet &operator=(ProcSnippet &&) = default;

    public:
        inline Kind kind() const {
            return m_kind;
        }
        inline Direction direction() const {
            return m_direction;
        }
        inline bool isFunction() const {
            return m_kind == Function;
        }
        inline bool isCallback() const {
            return m_kind == Callback;
        }
        inline DocumentContext &document() const {
            assert(m_doc);
            return *m_doc;
        }

        // FunctionDecl or FunctionPointerType that initializes this instance
        inline const clang::FunctionDecl *functionDecl() const {
            return m_functionDecl;
        }
        inline const std::optional<clang::QualType> functionPointerType() const {
            return m_functionPointerType;
        }

        /// Optional user-provided \c ProcFnDesc<> or \c ProcCbDesc<>
        inline const std::optional<Desc> &desc() const {
            return m_desc;
        }

        /// Optional user-provided \c ProcFn<> or \c ProcCb<>
        bool hasDefinition(Phase phase) const {
            return m_definitions[phase] != nullptr;
        }
        inline const clang::ClassTemplateSpecializationDecl *definition(Phase phase) const {
            return m_definitions[phase];
        }

        const std::string &name() const {
            return m_name;
        }

        /// Real normalized type information
        clang::QualType realFunctionPointerType() const {
            return m_realFunctionPointerType;
        }
        FunctionTypeView realFunctionTypeView() const {
            return m_realFunctionTypeView;
        }

        /// Sources to generate.
        const ProcSource &source(Phase phase) const {
            return m_sources[phase];
        }
        ProcSource &source(Phase phase) {
            return m_sources[phase];
        }

        /// Merged source text.
        std::string text(Phase phase, bool hasDecl) const;

    protected:
        void initialize(const std::string &nameHint);

        Kind m_kind = Function;
        Direction m_direction = GuestToHost;
        const clang::FunctionDecl *m_functionDecl = nullptr;
        std::optional<clang::QualType> m_functionPointerType;
        std::optional<Desc> m_desc;
        std::array<const clang::ClassTemplateSpecializationDecl *, NumPhases> m_definitions;
        DocumentContext *m_doc = nullptr;

        // Initialized in initialize()
        std::string m_name;
        clang::QualType m_realFunctionPointerType;
        FunctionTypeView m_realFunctionTypeView;

        // Generated by passes
        std::array<ProcSource, NumPhases> m_sources;
    };

}

#endif // LORE_TOOLS_TLCAPI_PROC_SNIPPET_H
