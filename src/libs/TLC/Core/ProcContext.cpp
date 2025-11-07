#include "ProcContext.h"

#include "DocumentContext.h"
#include "TypeExtras.h"

#include <stdcorelib/str.h>

using namespace clang;

namespace TLC {

    std::string ProcContext::text(CProcThunkPhase phase, bool decl, clang::ASTContext &ast) const {
        auto &source = _sources[phase];

        const char *phaseStr;
        switch (phase) {
            case CProcThunkPhase_GTP:
                phaseStr = "GTP";
                break;
            case CProcThunkPhase_GTP_IMPL:
                phaseStr = "GTP_IMPL";
                break;
            case CProcThunkPhase_HTP:
                phaseStr = "HTP";
                break;
            case CProcThunkPhase_HTP_IMPL:
                phaseStr = "HTP_IMPL";
                break;
            default:
                break;
        }

        const char *kindStr;
        switch (_procKind) {
            case CProcKind_HostFunction:
                kindStr = "HostFunction";
                break;
            case CProcKind_GuestFunction:
                kindStr = "GuestFunction";
                break;
            case CProcKind_HostCallback:
                kindStr = "HostCallback";
                break;
            case CProcKind_GuestCallback:
                kindStr = "GuestCallback";
                break;
            default:
                break;
        }

        std::string res;
        if (decl) {
            res += "template <>\n";
            res += stdc::formatN("struct %1<%2, CProcKind_%3, CProcThunkPhase_%4> {\n",
                                 isFunction() ? "MetaProc" : "MetaProcCB",
                                 isFunction() ? ("::" + _name) : _name, kindStr, phaseStr);
            res += "    _PROC " + source.functionInfo.declText("invoke", ast) + ";\n";
            res += "};\n";
        } else {
            res += source.head.toRawText();
            res += source.functionInfo.declText(
                       stdc::formatN("%1<%2, CProcKind_%3, CProcThunkPhase_%4>::\ninvoke",
                                     isFunction() ? "MetaProc" : "MetaProcCB",
                                     isFunction() ? ("::" + _name) : _name, kindStr, phaseStr),
                       ast) +
                   " {\n";
            res += "    // prolog\n";
            res += source.body.prolog.toRawText();
            res += "    // forward\n";
            res += source.body.forward.toRawText();
            res += "    // center\n";
            res += source.body.center.toRawText();
            res += "    // backward\n";
            res += source.body.backward.toRawText();
            res += "    // epilog\n";
            res += source.body.epilog.toRawText();
            res += "}\n";
            res += source.tail.toRawText();
        }
        return res;
    }

    void ProcContext::initialize() {
        bool isFunc = isFunction();
        if (isFunc) {
            assert(_functionDecl);
            _functionPointerType = _documentContext.ast()->getPointerType(_functionDecl->getType());
            _name = _functionDecl->getName();

            // get desc
            if (auto it = _documentContext.procDescs().find(_name);
                it != _documentContext.procDescs().end()) {
                _desc = &it->second;
            }

            // get definitions
            for (int phase = CProcThunkPhase_GTP; phase < CProcThunkPhase_NumThunkPhase; ++phase) {
                auto &map = _documentContext.procs(_procKind, static_cast<CProcThunkPhase>(phase));
                auto it = map.find(_name);
                if (it != map.end()) {
                    _definitions[phase] = &it->second;
                }
            }
        } else {
            if (!_nameHint.empty()) {
                _name = _nameHint;
            } else if (auto typedefType = _functionPointerType->getAs<TypedefType>()) {
                _name = typedefType->getDecl()->getName();
            }

            std::string typeStr = getTypeString(_functionPointerType.getCanonicalType());

            // get desc
            if (auto it = _documentContext.procCBDescs().find(typeStr);
                it != _documentContext.procCBDescs().end()) {
                _desc = &it->second;
            }

            // get definitions
            for (int phase = CProcThunkPhase_GTP; phase < CProcThunkPhase_NumThunkPhase; ++phase) {
                auto &map =
                    _documentContext.procCBs(_procKind, static_cast<CProcThunkPhase>(phase));
                auto it = map.find(typeStr);
                if (it != map.end()) {
                    _definitions[phase] = &it->second;
                }
            }
        }

        if (_desc) {
            if (_desc->builderPass().has_value()) {
                _builderID = _desc->builderPass().value().get().id();
            }
            if (_desc->overlayType()) {
                _overlayType = _desc->overlayType().value();
            }
        }

        if (_overlayType) {
            _realFunctionPointerType = _overlayType.value();
        } else {
            _realFunctionPointerType = _functionPointerType;
        }
        _realFunctionTypeView = FunctionTypeView(_realFunctionPointerType);
    }

}