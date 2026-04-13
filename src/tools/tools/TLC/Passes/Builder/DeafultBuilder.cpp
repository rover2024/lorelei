#include <lorelei/Base/Support/StringExtras.h>
#include <lorelei/Tools/TLCApi/Core/Pass.h>
#include <lorelei/Tools/TLCApi/Core/ProcSnippet.h>
#include <lorelei/Tools/TLCApi/Core/DocumentContext.h>
#include <lorelei/Tools/ThunkInterface/PassTags.h>

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
        llvm::Error beginHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        llvm::Error endHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
    };

    bool DefaultBuilderPass::testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) {
        return true; // seleted automatically
    }

    llvm::Error DefaultBuilderPass::beginHandleProc(ProcSnippet &proc,
                                                    std::unique_ptr<PassMessage> &msg) {
        /// \brief Guest-to-Host function thunks
        /// \example: int foo(int a, double b);
        /// \code
        ///     int  invoke(int a, double b);                           // Guest Entry  (1)
        ///     int  invoke(int a, double b);                           // Guest Caller (2)
        ///
        ///     void invoke(void **args, void *ret, void *metadata);    // Host Entry   (3)
        ///     int  invoke(int a, double b);                           // Host Caller  (4)
        /// \endcode

        /// \brief Host-to-Guest function thunks
        /// \example: int foo(int a, double b);
        /// \code
        ///     int  invoke(void **args, void *ret, void *metadata);    // Guest Entry  (1)
        ///     int  invoke(int a, double b);                           // Guest Caller (2)
        ///
        ///     int  invoke(int a, double b);                           // Host Entry   (3)
        ///     void invoke(int a, double b);                           // Host Caller  (4)
        /// \endcode

        /// \brief Host-to-Guest callback thunks
        /// \example: int (*foo)(int a, double b);
        /// \code
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // GTP      (3)
        ///     int  invoke(void *callback, int a, double b);           // GTP_IMPL (4)
        ///
        ///     int  invoke(int a, double b);                           // HTP      (1)
        ///     int  invoke(void *callback, int a, double b);           // HTP_IMPL (2)
        /// \endcode

        /// \brief Guest-to-Host callback thunks
        /// \example: int (*foo)(int a, double b);
        /// \code
        ///     int  invoke(int a, double b);                           // GTP      (1)
        ///     int  invoke(void *callback, int a, double b);           // GTP_IMPL (2)
        ///
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // HTP      (3)
        ///     int  invoke(void *callback, int a, double b);           // HTP_IMPL (4)
        /// \endcode

        auto &doc = proc.document();
        auto &ast = doc.ast();
        const auto &real = proc.realFunctionTypeView();
        bool isVoid = real.returnType()->isVoidType();

        QualType voidType = ast.VoidTy;
        QualType pVoidType = ast.getPointerType(voidType);
        QualType ppVoidType = ast.getPointerType(pVoidType);

        std::string key = name();
        FunctionInfo FI = real;
        FunctionInfo CFI = FI;
        CFI.argumentsRef().insert(CFI.argumentsRef().begin(), {pVoidType, "callback"});

        bool isHost = doc.mode() == DocumentContext::Host;
        bool isG2H = proc.direction() == ProcSnippet::GuestToHost;

        auto &ENT = proc.source(ProcSnippet::Entry);
        auto &CAL = proc.source(ProcSnippet::Caller);
        ProcSnippet::ProcSource emptyENT;
        ProcSnippet::ProcSource emptyCAL;

        auto &GENT = isHost ? emptyENT : ENT;
        auto &GCAL = isHost ? emptyCAL : CAL;
        auto &HENT = isHost ? ENT : emptyENT;
        auto &HCAL = isHost ? CAL : emptyCAL;

        auto &XENT = isG2H ? GENT : HENT;
        auto &XCAL = isG2H ? GCAL : HCAL;
        auto &YENT = isG2H ? HENT : GENT;
        auto &YCAL = isG2H ? HCAL : GCAL;

        const char *procKindStr = isG2H ? "GuestToHost" : "HostToGuest";
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

        if (proc.isFunction()) {
            XENT.functionInfo = XCAL.functionInfo = YCAL.functionInfo = FI;
            YENT.functionInfo = FI_packedFunctionInfo(ast);

            /// \example: A guest-to-host function \a foo
            /// \code
            ///     int foo(int a, double b);
            /// \endcode

            /// \example: Entry
            /// \code
            ///     int invoke(int a, double b) {
            ///         int ret;
            ///         ret = ProcFn<foo, GuestToHost, Caller>::invoke(a, b);
            ///         return ret;
            ///     }
            /// \endcode
            XENT.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XENT.body.center.push_back(key, SRC_callListAssign(FI, getProcFnCallerInvoke()));
            XENT.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller
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

            /// \example: Entry
            /// \code
            ///     void invoke(void **args, void *ret, void *metadata) {
            ///         auto &arg1 = *(int *) args[0];
            ///         auto &arg2 = *(double *) args[1];
            ///         auto &ret_ref = *(int *) ret;
            ///         ret_ref = ProcFn<foo, GuestToHost, Caller>::invoke(arg1, arg2);
            ///     }
            /// \endcode
            YENT.body.prolog.push_back(key, SRC_argPtrListExtractDecl(FI, ast));
            YENT.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
            YENT.body.center.push_back(key,
                                       SRC_callListAssign(FI, getProcFnCallerInvoke(), "ret_ref"));

            /// \example: Caller
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
            XCAL.functionInfo = YCAL.functionInfo = CFI;

            /// \example: A host-to-guest callback \a PFN_foo
            /// \code
            ///     int (*PFN_foo)(int a, double b);
            /// \endcode

            /// \example: Entry
            /// \code
            ///     int invoke(void *callback, int a, double b) {
            ///         int ret;
            ///         // get callback
            ///         ret = ProcCb<PFN_foo, HostToGuest, Caller>::invoke(callback, a, b);
            ///         return ret;
            ///     }
            /// \endcode
            XENT.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XENT.body.prolog.push_back(key, SRC_getCallback(!isG2H));
            XENT.body.center.push_back(key, SRC_callListAssign(CFI, getProcCbCallerInvoke()));
            XENT.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller
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

            /// \example: Entry
            /// \code
            ///     void invoke(void *callback, void **args, void *ret, void *metadata) {
            ///         auto &arg1 = *(int *) args[0];
            ///         auto &arg2 = *(double *) args[1];
            ///         auto &ret_ref = *(int *) ret;
            ///         ret_ref = ProcCb<PFN_foo, HostToGuest, Caller>::invoke(
            ///             callback, arg1, arg2
            ///         );
            ///     }
            /// \endcode
            YENT.body.prolog.push_back(key, SRC_argPtrListExtractDecl(FI, ast));
            YENT.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
            YENT.body.center.push_back(key,
                                       SRC_callListAssign(CFI, getProcCbCallerInvoke(), "ret_ref"));

            /// \example: Caller
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

        return llvm::Error::success();
    }

    llvm::Error DefaultBuilderPass::endHandleProc(ProcSnippet &proc,
                                                  std::unique_ptr<PassMessage> &msg) {
        return llvm::Error::success();
    }

    static llvm::Registry<Pass>::Add<DefaultBuilderPass> PR_DefaultBuilder("DefaultBuilder", {});

}
