#include "MetaProcItem.h"

using namespace clang;

namespace TLC {

    static inline bool isValidProcKindAndThunkPhase(int procKind, int thunkPhase) {
        return (procKind >= CProcKind_HostFunction && procKind < CProcKind_NumProcKind) &&
               (thunkPhase >= CProcThunkPhase_GTP && thunkPhase < CProcThunkPhase_NumThunkPhase);
    }

    void MetaProcItemBase::postAssign() {
    }

    void MetaProcItem::tryAssign(const clang::ClassTemplateSpecializationDecl *decl) {
        if (decl->getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
            return;
        }
        auto &tArgs = decl->getTemplateArgs();
        if (tArgs.size() != 3) {
            return;
        }
        auto &tArg = tArgs[0];
        if (tArg.getKind() != TemplateArgument::ArgKind::Declaration ||
            tArgs[1].getKind() != TemplateArgument::ArgKind::Integral ||
            tArgs[2].getKind() != TemplateArgument::ArgKind::Integral) {
            return;
        }
        auto tArgDecl = tArg.getAsDecl();
        if (tArgDecl->getKind() != Decl::Kind::Function) {
            return;
        }
        auto procKind = tArgs[1].getAsIntegral().getExtValue();
        auto thunkPhase = tArgs[2].getAsIntegral().getExtValue();
        if (!isValidProcKindAndThunkPhase(procKind, thunkPhase)) {
            return;
        }
        _decl = decl;
        _procDecl = tArgDecl->getAsFunction();
        _procKind = static_cast<CProcKind>(procKind);
        _thunkPhase = static_cast<CProcThunkPhase>(thunkPhase);

        postAssign();
    }

    void MetaProcCBItem::tryAssign(const clang::ClassTemplateSpecializationDecl *decl) {
        if (decl->getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
            return;
        }
        auto &tArgs = decl->getTemplateArgs();
        if (tArgs.size() != 3) {
            return;
        }
        auto &tArg = tArgs[0];
        if (tArg.getKind() != TemplateArgument::ArgKind::Type ||
            tArgs[1].getKind() != TemplateArgument::ArgKind::Integral ||
            tArgs[2].getKind() != TemplateArgument::ArgKind::Integral) {
            return;
        }
        auto tArgType = tArg.getAsType();
        if (!tArgType.getCanonicalType()->isFunctionPointerType()) {
            return;
        }
        auto procKind = tArgs[1].getAsIntegral().getExtValue();
        auto thunkPhase = tArgs[2].getAsIntegral().getExtValue();
        if (!isValidProcKindAndThunkPhase(procKind, thunkPhase)) {
            return;
        }
        _decl = decl;
        _procType = tArgType;
        _procKind = static_cast<CProcKind>(procKind);
        _thunkPhase = static_cast<CProcThunkPhase>(thunkPhase);

        postAssign();
    }

}