// SPDX-License-Identifier: MIT

#ifndef LORE_TLCAPI_PROCSNIPPET_H
#define LORE_TLCAPI_PROCSNIPPET_H

#include <cassert>
#include <string>
#include <vector>

#include <clang/AST/DeclTemplate.h>

#include <lorelei/ClangExtras/SourceLineList.h>
#include <lorelei/ClangExtras/FunctionInfo.h>
#include <lorelei/DLCall/ProcDefs.h>
#include <lorelei/TLCApi/Global.h>

namespace lore::tool::TLC {

    class DocumentContext;

    /// ProcSnippet - The per-proc model that TLC's generate passes read from and write into.
    ///
    /// Each instance describes one thunk procedure and binds it to its source: a function proc
    /// points at a concrete \c clang::FunctionDecl, while a callback proc holds a canonical
    /// function-pointer \c QualType. Alongside that identity it carries the descriptor metadata
    /// parsed from the manifest's ProcDesc specializations (builder selection, overlay type, extra
    /// pass tags), the optional user-provided per-phase definitions, and the generation buffers
    /// (\c ProcSource) that passes fill in.
    ///
    /// It is deliberately data-centric and performs no AST matching of its own: discovery happens
    /// in the command layer, which injects already-normalized inputs here. The lifecycle is
    /// construct -> initialize() (resolves the canonical type and name) -> Builder/Guard/Misc
    /// passes mutate the sources -> the command layer serializes them into the output translation
    /// unit.
    class LORETLCAPI_EXPORT ProcSnippet {
    public:
        using Kind = lore::thunk::ProcKind;
        using Direction = lore::thunk::ProcDirection;
        using Phase = lore::thunk::ProcPhase;
        using enum lore::thunk::ProcKind;
        using enum lore::thunk::ProcDirection;
        using enum lore::thunk::ProcPhase;

        /// A resolved pass id paired with the type argument it was configured with.
        struct PassInfo {
            int id = -1;
            clang::QualType type;
        };

        /// Descriptor metadata parsed from a ProcDesc specialization.
        struct Desc {
            std::optional<clang::QualType> overlayType;
            std::optional<PassInfo> builderPass;
            llvm::SmallVector<PassInfo, 10> passes;
        };

        /// The ordered line buffers that make up one generated function body.
        struct FunctionBodySource {
            SourceLineList<> prolog;
            SourceLineList<> forward;
            SourceLineList<> center;
            SourceLineList<> backward;
            SourceLineList<> epilog;
        };

        /// All generated text for one phase: a head, the function body, and a tail.
        struct ProcSource {
            FunctionInfo functionInfo;
            SourceLineList<> head;
            FunctionBodySource body;
            SourceLineList<> tail;
        };

        /// Constructs a function proc from a function declaration.
        ProcSnippet(
            Kind kind, Direction direction, const clang::FunctionDecl *FD, std::string nameHint,
            std::optional<Desc> desc,
            std::array<const clang::ClassTemplateSpecializationDecl *, Exec> definitions,
            DocumentContext &documentContext)
            : m_kind(kind), m_direction(direction), m_functionDecl(FD), m_desc(std::move(desc)),
              m_definitions(std::move(definitions)), m_doc(&documentContext) {
            assert(isFunction());
            initialize(nameHint);
        }

        /// Constructs a callback proc from a function-pointer type.
        ProcSnippet(
            Kind kind, Direction direction, clang::QualType functionPointerType,
            std::string nameHint, std::optional<Desc> desc,
            std::array<const clang::ClassTemplateSpecializationDecl *, Exec> definitions,
            DocumentContext &documentContext)
            : m_kind(kind), m_direction(direction),
              m_functionPointerType(std::move(functionPointerType)), m_desc(std::move(desc)),
              m_definitions(std::move(definitions)), m_doc(&documentContext) {
            assert(isCallback());
            initialize(nameHint);
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

        /// The source this snippet was built from: a \c FunctionDecl in function mode, or a
        /// function-pointer \c QualType in callback mode (only one is populated).
        inline const clang::FunctionDecl *functionDecl() const {
            return m_functionDecl;
        }
        inline const std::optional<clang::QualType> functionPointerType() const {
            return m_functionPointerType;
        }

        /// Descriptor metadata from the optional \c ProcFnDesc<> / \c ProcCbDesc<> manifest entry.
        inline const std::optional<Desc> &desc() const {
            return m_desc;
        }

        /// Whether the manifest supplied a \c ProcFn<> / \c ProcCb<> definition for the phase.
        bool hasDefinition(Phase phase) const {
            return m_definitions[phase] != nullptr;
        }
        inline const clang::ClassTemplateSpecializationDecl *definition(Phase phase) const {
            return m_definitions[phase];
        }

        /// The generated thunk symbol name.
        const std::string &name() const {
            return m_name;
        }

        /// The resolved, normalized function-pointer type and a cached view over its signature.
        clang::QualType realFunctionPointerType() const {
            return m_realFunctionPointerType;
        }
        FunctionTypeView realFunctionTypeView() const {
            return m_realFunctionTypeView;
        }

        /// The per-phase generation buffer that passes fill in.
        const ProcSource &source(Phase phase) const {
            return m_sources[phase];
        }
        ProcSource &source(Phase phase) {
            return m_sources[phase];
        }

        /// Renders the phase's buffers into the final proc source text.
        std::string text(Phase phase, bool hasDecl) const;

    protected:
        void initialize(const std::string &nameHint);

        Kind m_kind = Function;
        Direction m_direction = GuestToHost;
        const clang::FunctionDecl *m_functionDecl = nullptr;
        std::optional<clang::QualType> m_functionPointerType;
        std::optional<Desc> m_desc;
        std::array<const clang::ClassTemplateSpecializationDecl *, Exec> m_definitions;
        DocumentContext *m_doc = nullptr;

        // Resolved by initialize().
        std::string m_name;
        clang::QualType m_realFunctionPointerType;
        FunctionTypeView m_realFunctionTypeView;

        // Produced by the passes.
        std::array<ProcSource, Exec> m_sources;
    };

}

#endif // LORE_TLCAPI_PROCSNIPPET_H
