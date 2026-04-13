#include "ProcSnippet.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>

#include "DocumentContext.h"

namespace lore::tool::TLC {

    std::string ProcSnippet::text(Phase phase, bool hasDecl) const {
        const auto &src = m_sources[phase];

        if (src.functionInfo.returnType().isNull()) {
            return {};
        }

        auto &ast = document().ast();
        std::string out;
        if (hasDecl) {
            out += src.functionInfo.declText(m_name, ast);
            out += ";\n";
            return out;
        }

        out += src.head.toRawText();
        out += src.functionInfo.declText(m_name, ast);
        out += " {\n";
        out += src.body.prolog.toRawText();
        out += src.body.forward.toRawText();
        out += src.body.center.toRawText();
        out += src.body.backward.toRawText();
        out += src.body.epilog.toRawText();
        out += "}\n";
        out += src.tail.toRawText();
        return out;
    }

    void ProcSnippet::initialize(const std::string &nameHint) {
        if (isFunction()) {
            assert(m_functionDecl != nullptr);

            if (!nameHint.empty()) {
                m_name = nameHint;
            } else {
                m_name = m_functionDecl->getNameAsString();
            }

            auto fp = document().ast().getPointerType(m_functionDecl->getType().getCanonicalType());
            m_functionPointerType = fp;
        } else {
            assert(m_functionPointerType.has_value());

            if (!nameHint.empty()) {
                m_name = nameHint;
            } else if (const auto *typedefType =
                           (*m_functionPointerType)->getAs<clang::TypedefType>()) {
                m_name = typedefType->getDecl()->getNameAsString();
            }
        }

        if (m_desc.has_value() && m_desc->overlayType.has_value()) {
            m_realFunctionPointerType = *m_desc->overlayType;
        } else if (m_functionPointerType.has_value()) {
            m_realFunctionPointerType = *m_functionPointerType;
        }

        if (!m_realFunctionPointerType.isNull()) {
            m_realFunctionTypeView = FunctionTypeView(m_realFunctionPointerType);
        }
    }

}
