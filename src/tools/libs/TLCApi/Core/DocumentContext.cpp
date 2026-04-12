// SPDX-License-Identifier: MIT

#include <lorelei/Tools/TLCApi/Core/DocumentContext.h>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <llvm/Support/raw_ostream.h>

#include <lorelei/Tools/ToolUtils/TypeUtils.h>

namespace lore::tool::TLC {

    bool DocumentContext::beginSourceFileAction(clang::CompilerInstance &CI) {
        (void) CI;
        return true;
    }

    void DocumentContext::handleTranslationUnit(clang::ASTContext &ast) {
        m_ast = &ast;
        m_functionDecls[ProcSnippet::Function].clear();
        m_functionDecls[ProcSnippet::Callback].clear();
        m_varDecls.clear();

        auto *tu = ast.getTranslationUnitDecl();
        if (!tu) {
            return;
        }

        for (const auto *decl : tu->decls()) {
            if (const auto *fd = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
                if (!fd->getIdentifier()) {
                    continue;
                }
                const std::string name = fd->getNameAsString();
                if (!name.empty()) {
                    m_functionDecls[ProcSnippet::Function][name] = fd;
                    m_functionDecls[ProcSnippet::Callback][name] = fd;
                }
                continue;
            }

            if (const auto *vd = llvm::dyn_cast<clang::VarDecl>(decl)) {
                if (!vd->getIdentifier()) {
                    continue;
                }
                const std::string name = vd->getNameAsString();
                if (!name.empty()) {
                    m_varDecls[name] = vd;
                }
                continue;
            }
        }
    }

    void DocumentContext::endSourceFileAction() {
        for (auto &kindArr : m_procs) {
            for (auto &dirMap : kindArr) {
                dirMap.clear();
            }
        }

        if (!m_ast) {
            return;
        }

        const auto appendFunctionByName = [&](const std::string &name, ProcSnippet::Direction direction) {
            auto it = m_functionDecls[ProcSnippet::Function].find(name);
            if (it == m_functionDecls[ProcSnippet::Function].end()) {
                return;
            }

            ProcSnippet::ProcSource source;
            source.functionInfo = FunctionInfo(it->second);
            m_procs[ProcSnippet::Function][direction][name] = std::move(source);
        };

        // Interested function seeds are indexed by direction (GuestToHost / HostToGuest).
        for (int i = ProcSnippet::GuestToHost; i < ProcSnippet::NumDirections; ++i) {
            auto direction = static_cast<ProcSnippet::Direction>(i);
            for (const auto &name : m_interestedProcData.functions[i]) {
                appendFunctionByName(name, direction);
            }
        }

        const auto callbackDirection =
            m_mode == Guest ? ProcSnippet::HostToGuest : ProcSnippet::GuestToHost;

        const auto resolveCallbackTypeBySignature = [&](const std::string &signature)
            -> std::optional<clang::QualType> {
            for (const auto &[_, fd] : m_functionDecls[ProcSnippet::Function]) {
                auto fp = m_ast->getPointerType(fd->getType().getCanonicalType());
                if (getTypeString(fp) == signature) {
                    return fp;
                }
            }

            for (const auto &[_, vd] : m_varDecls) {
                auto type = vd->getType().getCanonicalType();
                if (!type->isFunctionPointerType()) {
                    continue;
                }
                if (getTypeString(type) == signature) {
                    return type;
                }
            }

            return std::nullopt;
        };

        for (const auto &[signature, callbackName] : m_interestedProcData.callbacks) {
            ProcSnippet::ProcSource source;

            if (auto fpType = resolveCallbackTypeBySignature(signature)) {
                source.functionInfo = FunctionInfo(FunctionTypeView(*fpType));
            }

            auto key = callbackName.empty() ? signature : callbackName;
            m_procs[ProcSnippet::Callback][callbackDirection][key] = std::move(source);
        }
    }

    void DocumentContext::generateOutput(llvm::raw_ostream &os) {
        os << m_source.head.toRawText();

        const auto emitProcMap = [&](ProcSnippet::Kind kind, ProcSnippet::Direction direction,
                                     const char *kindName, const char *directionName) {
            for (const auto &[name, proc] : m_procs[kind][direction]) {
                os << "\n// ===== " << kindName << " / " << directionName << " / " << name
                   << " =====\n";

                const auto &info = proc.functionInfo;
                if (!info.returnType().isNull() && m_ast) {
                    os << info.declText(name, *m_ast) << " {\n";
                    os << proc.body.prolog.toRawText();
                    os << proc.body.forward.toRawText();
                    os << proc.body.center.toRawText();
                    os << proc.body.backward.toRawText();
                    os << proc.body.epilog.toRawText();
                    os << "}\n";
                } else {
                    os << "// unresolved function type\n";
                    os << proc.head.toRawText();
                    os << proc.body.prolog.toRawText();
                    os << proc.body.forward.toRawText();
                    os << proc.body.center.toRawText();
                    os << proc.body.backward.toRawText();
                    os << proc.body.epilog.toRawText();
                    os << proc.tail.toRawText();
                }
            }
        };

        emitProcMap(ProcSnippet::Function, ProcSnippet::GuestToHost, "Function", "GuestToHost");
        emitProcMap(ProcSnippet::Function, ProcSnippet::HostToGuest, "Function", "HostToGuest");
        emitProcMap(ProcSnippet::Callback, ProcSnippet::GuestToHost, "Callback", "GuestToHost");
        emitProcMap(ProcSnippet::Callback, ProcSnippet::HostToGuest, "Callback", "HostToGuest");

        os << m_source.tail.toRawText();
    }

}
