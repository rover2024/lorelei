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
        const auto &procTemplateName = isFunction() ? "ProcFn" : "ProcCb";
        const auto &procName = isFunction() ? ("::" + m_name) : m_name;
        const auto &directionName = (m_direction == GuestToHost) ? "GuestToHost" : "HostToGuest";
        const auto &phaseName = (phase == Entry) ? "Entry" : "Caller";

        std::string out;
        if (hasDecl) {
            out += "template <>\n";
            out += "struct ";
            out += procTemplateName;
            out += "<";
            out += procName;
            out += ", ";
            out += directionName;
            out += ", ";
            out += phaseName;
            out += "> {\n";
            out += "    _PROC ";
            out += src.functionInfo.declText("invoke", ast);
            out += ";\n";
            out += "};\n";
            return out;
        }

        out += src.head.toRawText();
        out += src.functionInfo.declText(procTemplateName + ("<" + procName + ", " + directionName +
                                                             ", " + phaseName + ">::\ninvoke"),
                                         ast);
        out += " {\n";
        out += "    // prolog\n";
        out += src.body.prolog.toRawText();
        out += "    // forward\n";
        out += src.body.forward.toRawText();
        out += "    // center\n";
        out += src.body.center.toRawText();
        out += "    // backward\n";
        out += src.body.backward.toRawText();
        out += "    // epilog\n";
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
