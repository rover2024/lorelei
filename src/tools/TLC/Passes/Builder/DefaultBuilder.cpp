// SPDX-License-Identifier: MIT

#include <lorelei/Support/StringExtras.h>
#include <lorelei/TLCApi/Pass.h>
#include <lorelei/TLCApi/ProcSnippet.h>
#include <lorelei/TLCApi/DocumentContext.h>
#include <lorelei/ThunkInterface/PassTags.h>

#include "Utils/PassCodeTemplates.h"

using namespace clang;

namespace lore::tool::TLC {

    class DefaultBuilderPass : public Pass {
    public:
        DefaultBuilderPass() : Pass(Builder, lore::thunk::pass::ID_DefaultBuilder) {
        }

        std::string name() const override {
            return "DefaultBuilder";
        }

        bool testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        void beginHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        void endHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
    };

    bool DefaultBuilderPass::testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) {
        return true; // selected automatically; the default builder handles every proc
    }

    void DefaultBuilderPass::beginHandleProc(ProcSnippet &proc,
                                                    std::unique_ptr<PassMessage> &msg) {
        /// \brief Guest-to-Host function thunks
        /// \example: int foo(int a, double b);
        /// \code
        ///     int  invoke(int a, double b);                           // Guest Entry  (1)
        ///     int  invoke(int a, double b);                           // Guest Adapt  (2)
        ///     int  invoke(int a, double b);                           // Guest Caller (3)
        ///
        ///     void invoke(void **args, void *ret, void *metadata);    // Host Entry   (4)
        ///     int  invoke(int a, double b);                           // Host Adapt   (5)
        ///     int  invoke(int a, double b);                           // Host Caller  (6)
        /// \endcode

        /// \brief Host-to-Guest function thunks
        /// \example: int foo(int a, double b);
        /// \code
        ///     int  invoke(void **args, void *ret, void *metadata);    // Guest Entry  (1)
        ///     int  invoke(int a, double b);                           // Guest Adapt  (2)
        ///     int  invoke(int a, double b);                           // Guest Caller (3)
        ///
        ///     int  invoke(int a, double b);                           // Host Entry   (4)
        ///     int  invoke(int a, double b);                           // Host Adapt   (5)
        ///     void invoke(int a, double b);                           // Host Caller  (6)
        /// \endcode

        /// \brief Host-to-Guest callback thunks
        /// \example: int (*foo)(int a, double b);
        /// \code
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // Guest Entry  (1)
        ///     int  invoke(void *callback, int a, double b);           // Guest Adapt  (2)
        ///     int  invoke(void *callback, int a, double b);           // Guest Caller (3)
        ///
        ///     int  invoke(int a, double b);                           // Host Entry   (4)
        ///     int  invoke(void *callback, int a, double b);           // Host Adapt   (5)
        ///     int  invoke(void *callback, int a, double b);           // Host Caller  (6)
        /// \endcode

        /// \brief Guest-to-Host callback thunks
        /// \example: int (*foo)(int a, double b);
        /// \code
        ///     int  invoke(int a, double b);                           // Guest Entry  (1)
        ///     int  invoke(void *callback, int a, double b);           // Guest Adapt  (2)
        ///     int  invoke(void *callback, int a, double b);           // Guest Caller (3)
        ///
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // Host Entry   (4)
        ///     int  invoke(void *callback, int a, double b);           // Host Adapt   (5)
        ///     int  invoke(void *callback, int a, double b);           // Host Caller  (6)
        /// \endcode

        auto &doc = proc.document();
        auto &ast = doc.ast();
        const auto &real = proc.realFunctionTypeView();
        bool isVoid = real.returnType()->isVoidType();

        QualType voidType = ast.VoidTy;
        QualType pVoidType = ast.getPointerType(voidType);

        std::string key = name();
        FunctionInfo FI = real;
        FunctionInfo CFI = FI; // callback FI: same signature with a leading `void *callback`
        CFI.argumentsRef().insert(CFI.argumentsRef().begin(), {pVoidType, "callback"});

        bool isHost = doc.mode() == DocumentContext::Host;
        bool isG2H = proc.direction() == ProcSnippet::GuestToHost;

        auto &ENT = proc.source(ProcSnippet::Entry);
        auto &ADP = proc.source(ProcSnippet::Adapt);
        auto &CAL = proc.source(ProcSnippet::Caller);
        ProcSnippet::ProcSource emptyENT;
        ProcSnippet::ProcSource emptyADP;
        ProcSnippet::ProcSource emptyCAL;

        // This pass only emits one side per run (guest or host). The G*/H* aliases route the live
        // ProcSource to whichever side matches the document mode and discard the other into a throw-
        // away `empty*`. X* is then the sender side and Y* the receiver side for this direction.
        auto &GENT = isHost ? emptyENT : ENT;
        auto &GADP = isHost ? emptyADP : ADP;
        auto &GCAL = isHost ? emptyCAL : CAL;
        auto &HENT = isHost ? ENT : emptyENT;
        auto &HADP = isHost ? ADP : emptyADP;
        auto &HCAL = isHost ? CAL : emptyCAL;

        auto &XENT = isG2H ? GENT : HENT;
        auto &XADP = isG2H ? GADP : HADP;
        auto &XCAL = isG2H ? GCAL : HCAL;
        auto &YENT = isG2H ? HENT : GENT;
        auto &YADP = isG2H ? HADP : GADP;
        auto &YCAL = isG2H ? HCAL : GCAL;

        const char *procKindStr = isG2H ? "GuestToHost" : "HostToGuest";
        const auto &getProcFnAdaptInvoke = [&]() {
            return formatN("ProcFn<%1, %2, Adapt>::invoke", proc.name(), procKindStr);
        };
        const auto &getProcCbAdaptInvoke = [&]() {
            return formatN("ProcCb<%1, %2, Adapt>::invoke", proc.name(), procKindStr);
        };
        const auto &getProcFnCallerInvoke = [&]() {
            return formatN("ProcFn<%1, %2, Caller>::invoke", proc.name(), procKindStr);
        };
        const auto &getProcCbCallerInvoke = [&]() {
            return formatN("ProcCb<%1, %2, Caller>::invoke", proc.name(), procKindStr);
        };
        const auto &getProcFnExecInvokeWithCallList = [&]() {
            return formatN("ProcFn<%1, %2, Exec>::invoke(args, %3, nullptr);", proc.name(),
                           procKindStr, isVoid ? "nullptr" : "&ret");
        };
        const auto &getProcCbExecInvokeWithCallList = [&]() {
            return formatN("ProcCb<%1, %2, Exec>::invoke(callback, args, %3, nullptr);",
                           proc.name(), procKindStr, isVoid ? "nullptr" : "&ret");
        };

        // Adapt is a typed pass-through between Entry and Caller; non-Builder passes (callback
        // substitution, type/handle filtering, GetProcAddress) inject into its forward/backward so
        // that Entry stays pure (un)marshalling.

        if (proc.isFunction()) {
            XENT.functionInfo = XADP.functionInfo = XCAL.functionInfo = YADP.functionInfo =
                YCAL.functionInfo = FI;
            YENT.functionInfo = FI_packedFunctionInfo(ast);

            /// \example: A guest-to-host function \a foo
            /// \code
            ///     int foo(int a, double b);
            /// \endcode

            /// \example: Entry (sender)
            /// \code
            ///     int invoke(int a, double b) {
            ///         int ret;
            ///         ret = ProcFn<foo, GuestToHost, Adapt>::invoke(a, b);
            ///         return ret;
            ///     }
            /// \endcode
            XENT.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XENT.body.center.push_back(key, SRC_callListAssign(FI, getProcFnAdaptInvoke()));
            XENT.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Adapt (sender)
            /// \code
            ///     int invoke(int a, double b) {
            ///         int ret;
            ///         ret = ProcFn<foo, GuestToHost, Caller>::invoke(a, b);
            ///         return ret;
            ///     }
            /// \endcode
            XADP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XADP.body.center.push_back(key, SRC_callListAssign(FI, getProcFnCallerInvoke()));
            XADP.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller (sender)
            /// \code
            ///     int invoke(int a, double b) {
            ///         int ret;
            ///         void *args[] = { &a, &b, };
            ///         ret = ProcFn<foo, GuestToHost, Exec>::invoke(args, &ret, nullptr);
            ///         return ret;
            ///     }
            /// \endcode
            XCAL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XCAL.body.prolog.push_back(key, SRC_argPtrListDecl(FI));
            XCAL.body.center.push_back(key, SRC_asIs(getProcFnExecInvokeWithCallList()));
            XCAL.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Entry (receiver)
            /// \code
            ///     void invoke(void **args, void *ret, void *metadata) {
            ///         auto &arg1 = *(int *) args[0];
            ///         auto &arg2 = *(double *) args[1];
            ///         auto &ret_ref = *(int *) ret;
            ///         ret_ref = ProcFn<foo, GuestToHost, Adapt>::invoke(arg1, arg2);
            ///     }
            /// \endcode
            YENT.body.prolog.push_back(key, SRC_argPtrListExtractDecl(FI, ast));
            YENT.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
            YENT.body.center.push_back(key,
                                       SRC_callListAssign(FI, getProcFnAdaptInvoke(), "ret_ref"));

            /// \example: Adapt (receiver)
            /// \code
            ///     int invoke(int a, double b) {
            ///         int ret;
            ///         ret = ProcFn<foo, GuestToHost, Caller>::invoke(a, b);
            ///         return ret;
            ///     }
            /// \endcode
            YADP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            YADP.body.center.push_back(key, SRC_callListAssign(FI, getProcFnCallerInvoke()));
            YADP.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller (receiver)
            /// \code
            ///     int invoke(int a, double b) {
            ///         int ret;
            ///         ret = ProcFn<foo, GuestToHost, Exec>::invoke(a, b);
            ///         return ret;
            ///     }
            /// \endcode
            YCAL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            YCAL.body.center.push_back(
                key, SRC_callListAssign(
                         FI, formatN("ProcFn<%1, %2, Exec>::invoke", proc.name(), procKindStr)));
            YCAL.body.epilog.push_back(key, SRC_returnRet(FI));
        } else {
            XENT.functionInfo = FI;
            YENT.functionInfo = FI_packedCallbackInfo(ast);
            XADP.functionInfo = XCAL.functionInfo = YADP.functionInfo = YCAL.functionInfo = CFI;

            /// \example: A host-to-guest callback \a PFN_foo
            /// \code
            ///     int (*PFN_foo)(int a, double b);
            /// \endcode

            /// \example: Entry (sender)
            /// \code
            ///     int invoke(void *callback, int a, double b) {
            ///         int ret;
            ///         // get callback
            ///         ret = ProcCb<PFN_foo, HostToGuest, Adapt>::invoke(callback, a, b);
            ///         return ret;
            ///     }
            /// \endcode
            XENT.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XENT.body.prolog.push_back(key, SRC_getCallback(!isG2H));
            XENT.body.center.push_back(key, SRC_callListAssign(CFI, getProcCbAdaptInvoke()));
            XENT.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Adapt (sender)
            /// \code
            ///     int invoke(void *callback, int a, double b) {
            ///         int ret;
            ///         ret = ProcCb<PFN_foo, HostToGuest, Caller>::invoke(callback, a, b);
            ///         return ret;
            ///     }
            /// \endcode
            XADP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XADP.body.center.push_back(key, SRC_callListAssign(CFI, getProcCbCallerInvoke()));
            XADP.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller (sender)
            /// \code
            ///     int invoke(void *callback, int a, double b) {
            ///         int ret;
            ///         void *args[] = { &a, &b, };
            ///         ret = ProcCb<PFN_foo, HostToGuest, Exec>::invoke(
            //              callback, args, ret, nullptr
            ///         );
            ///         return ret;
            ///     }
            /// \endcode
            XCAL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XCAL.body.prolog.push_back(key, SRC_argPtrListDecl(FI));
            XCAL.body.center.push_back(key, SRC_asIs(getProcCbExecInvokeWithCallList()));
            XCAL.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Entry (receiver)
            /// \code
            ///     void invoke(void *callback, void **args, void *ret, void *metadata) {
            ///         auto &arg1 = *(int *) args[0];
            ///         auto &arg2 = *(double *) args[1];
            ///         auto &ret_ref = *(int *) ret;
            ///         ret_ref = ProcCb<PFN_foo, HostToGuest, Adapt>::invoke(
            ///             callback, arg1, arg2
            ///         );
            ///     }
            /// \endcode
            YENT.body.prolog.push_back(key, SRC_argPtrListExtractDecl(FI, ast));
            YENT.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
            YENT.body.center.push_back(key,
                                       SRC_callListAssign(CFI, getProcCbAdaptInvoke(), "ret_ref"));

            /// \example: Adapt (receiver)
            /// \code
            ///     int invoke(void *callback, int a, double b) {
            ///         int ret;
            ///         ret = ProcCb<PFN_foo, HostToGuest, Caller>::invoke(callback, a, b);
            ///         return ret;
            ///     }
            /// \endcode
            YADP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            YADP.body.center.push_back(key, SRC_callListAssign(CFI, getProcCbCallerInvoke()));
            YADP.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller (receiver)
            /// \code
            ///     int invoke(void *callback, int a, double b) {
            ///         int ret;
            ///         ret = ProcCb<PFN_foo, HostToGuest, Exec>::invoke(callback, a, b);
            ///         return ret;
            ///     }
            /// \endcode
            YCAL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            YCAL.body.center.push_back(
                key, SRC_callListAssign(
                         CFI, formatN("ProcCb<%1, %2, Exec>::invoke", proc.name(), procKindStr)));
            YCAL.body.epilog.push_back(key, SRC_returnRet(FI));
        }
    }

    void DefaultBuilderPass::endHandleProc(ProcSnippet &proc,
                                                  std::unique_ptr<PassMessage> &msg) {
    }

    static llvm::Registry<Pass>::Add<DefaultBuilderPass> PR_DefaultBuilder("DefaultBuilder", {});

}
