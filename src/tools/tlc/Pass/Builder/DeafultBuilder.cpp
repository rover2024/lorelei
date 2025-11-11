#include "Pass.h"

#include <stdcorelib/str.h>

#include <lorelei/TLCMeta/MetaPass.h>

#include "ProcContext.h"
#include "DocumentContext.h"
#include "BuilderPassCommon.h"

using namespace clang;

namespace TLC {

    class DefaultBuilderPass : public Pass {
    public:
        DefaultBuilderPass() : Pass(Builder, lorethunk::MetaPass::DefaultBuilder) {
        }

        std::string name() const override {
            return "DefaultBuilder";
        }

        bool testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;
        llvm::Error beginHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;
        llvm::Error endHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;
    };

    bool DefaultBuilderPass::testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) {
        return true; // seleted automatically
    }

    llvm::Error DefaultBuilderPass::beginHandleProc(ProcContext &proc,
                                                    std::unique_ptr<ProcMessage> &msg) {
        /// \brief Host function thunks
        /// \example: int foo(int a, double b);
        /// \code
        ///     int  invoke(int a, double b);                           // GTP      (1)
        ///     int  invoke(int a, double b);                           // GTP_IMPL (2)
        ///
        ///     void invoke(void **args, void *ret, void *metadata);    // HTP      (3)
        ///     int  invoke(int a, double b);                           // HTP_IMPL (4)
        /// \endcode

        /// \brief Guest function thunks
        /// \example: int foo(int a, double b);
        /// \code
        ///     int  invoke(void **args, void *ret, void *metadata);    // GTP      (1)
        ///     int  invoke(int a, double b);                           // GTP_IMPL (2)
        ///
        ///     int  invoke(int a, double b);                           // HTP      (3)
        ///     void invoke(int a, double b);                           // HTP_IMPL (4)
        /// \endcode

        /// \brief Guest callback thunks
        /// \example: int (*foo)(int a, double b);
        /// \code
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // GTP      (3)
        ///     int  invoke(void *callback, int a, double b);           // GTP_IMPL (4)
        ///
        ///     int  invoke(int a, double b);                           // HTP      (1)
        ///     int  invoke(void *callback, int a, double b);           // HTP_IMPL (2)
        /// \endcode

        /// \brief Host callback thunks
        /// \example: int (*foo)(int a, double b);
        /// \code
        ///     int  invoke(int a, double b);                           // GTP      (1)
        ///     int  invoke(void *callback, int a, double b);           // GTP_IMPL (2)
        ///
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // HTP      (3)
        ///     int  invoke(void *callback, int a, double b);           // HTP_IMPL (4)
        /// \endcode

        auto &doc = proc.documentContext();
        auto &ast = *doc.ast();
        const auto &real = proc.realFunctionTypeView();
        bool isVoid = real.returnType()->isVoidType();

        QualType voidType = ast.VoidTy;
        QualType pVoidType = ast.getPointerType(voidType);
        QualType ppVoidType = ast.getPointerType(pVoidType);

        std::string key = name();
        FunctionInfo FI = real;
        FunctionInfo CFI = FI;
        CFI.argumentsRef().insert(CFI.argumentsRef().begin(), {pVoidType, "callback"});

        auto &GTP = proc.source(CProcThunkPhase_GTP);
        auto &GTP_IMPL = proc.source(CProcThunkPhase_GTP_IMPL);
        auto &HTP = proc.source(CProcThunkPhase_HTP);
        auto &HTP_IMPL = proc.source(CProcThunkPhase_HTP_IMPL);

        const auto &getMetaProcInvoke = [&](const char *kind, const char *phase) {
            return stdc::formatN("MetaProc<::%1, CProcKind_%2, CProcThunkPhase_%3>::invoke",
                                 proc.name(), kind, phase);
        };
        const auto &getMetaProcCBInvoke = [&](const char *kind, const char *phase) {
            return stdc::formatN("MetaProcCB<%1, CProcKind_%2, CProcThunkPhase_%3>::invoke",
                                 proc.name(), kind, phase);
        };
        const auto &getMetaProcExecInvokeWithCallList = [&](const char *kind) {
            return stdc::formatN("MetaProcExec<::%1, CProcKind_%2>::invoke(args, %3, nullptr);",
                                 proc.name(), kind, isVoid ? "nullptr" : "&ret");
        };
        const auto &getMetaProcCBExecInvokeWithCallList = [&](const char *kind) {
            return stdc::formatN(
                "MetaProcCBExec<%1, CProcKind_%2>::invoke(callback, args, %3, nullptr);",
                proc.name(), kind, isVoid ? "nullptr" : "&ret");
        };

        switch (proc.procKind()) {
            case CProcKind_HostFunction:
            case CProcKind_GuestFunction: {
                bool isHostFunction = proc.procKind() == CProcKind_HostFunction;

                auto &XTP = isHostFunction ? GTP : HTP;
                auto &XTP_IMPL = isHostFunction ? GTP_IMPL : HTP_IMPL;
                auto &YTP = isHostFunction ? HTP : GTP;
                auto &YTP_IMPL = isHostFunction ? HTP_IMPL : GTP_IMPL;

                const char *procKind_str = isHostFunction ? "HostFunction" : "GuestFunction";
                const char *XTP_IMPL_str = isHostFunction ? "GTP_IMPL" : "HTP_IMPL";
                const char *YTP_IMPL_str = isHostFunction ? "HTP_IMPL" : "GTP_IMPL";

                XTP.functionInfo = XTP_IMPL.functionInfo = YTP_IMPL.functionInfo = FI;
                YTP.functionInfo = FI_packedFunctionInfo(ast);

                /// \code
                ///     int invoke(int a, double b) {
                ///         int ret;
                ///         ret = MetaProc<foo, CProcKind_HostFunction,
                ///                             CProcThunkPhase_XTP_IMPL>::invoke(a, b);
                ///         return ret;
                ///     }
                /// \endcode
                XTP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                XTP.body.center.push_back(
                    key, SRC_callListAssign(FI, getMetaProcInvoke(procKind_str, XTP_IMPL_str)));
                XTP.body.epilog.push_back(key, SRC_returnRet(FI));

                /// \code
                ///     int invoke(int a, double b) {
                ///         int ret;
                ///         void *args[] = { &a, &b, };
                ///         ret = MetaProcExec<foo, CProcKind_HostFunction>::invoke(
                ///                   args, &ret, nullptr
                ///         );
                ///         return ret;
                ///     }
                /// \endcode
                XTP_IMPL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                XTP_IMPL.body.prolog.push_back(key, SRC_argPtrListDecl(FI));
                XTP_IMPL.body.center.push_back(
                    key, SRC_asIs(getMetaProcExecInvokeWithCallList(procKind_str)));
                XTP_IMPL.body.epilog.push_back(key, SRC_returnRet(FI));

                /// \code
                ///     void invoke(void **args, void *ret, void *metadata) {
                ///         auto &arg1 = *(int *) args[0];
                ///         auto &arg2 = *(double *) args[1];
                ///         auto &ret_ref = *(int *) ret;
                ///         ret_ref = MetaProc<foo, CProcKind_HostFunction,
                ///                                 CProcThunkPhase_YTP_IMPL>::invoke(arg1, arg2);
                ///     }
                /// \endcode
                YTP.body.prolog.push_back(key, SRC_argPtrListExtractDecl(FI, ast));
                YTP.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
                YTP.body.center.push_back(
                    key, SRC_callListAssign(FI, getMetaProcInvoke(procKind_str, YTP_IMPL_str),
                                            "ret_ref"));

                /// \code
                ///     int invoke(int a, double b) {
                ///         int ret;
                ///         ret = MetaProcExec<foo, CProcKind_HostFunction>::invoke(a, b);
                ///         return ret;
                ///     }
                /// \endcode
                YTP_IMPL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                YTP_IMPL.body.center.push_back(
                    key,
                    SRC_callListAssign(FI, stdc::formatN("MetaProcExec<%1, CProcKind_%2>::invoke",
                                                         proc.name(), procKind_str)));
                YTP_IMPL.body.epilog.push_back(key, SRC_returnRet(FI));
                break;
            }

            case CProcKind_GuestCallback:
            case CProcKind_HostCallback: {
                bool isHostCallback = proc.procKind() == CProcKind_HostCallback;

                auto &XTP = isHostCallback ? GTP : HTP;
                auto &XTP_IMPL = isHostCallback ? GTP_IMPL : HTP_IMPL;
                auto &YTP = isHostCallback ? HTP : GTP;
                auto &YTP_IMPL = isHostCallback ? HTP_IMPL : GTP_IMPL;

                const char *procKind_str = isHostCallback ? "HostCallback" : "GuestCallback";
                const char *XTP_IMPL_str = isHostCallback ? "GTP_IMPL" : "HTP_IMPL";
                const char *YTP_IMPL_str = isHostCallback ? "HTP_IMPL" : "GTP_IMPL";

                XTP.functionInfo = FI;
                YTP.functionInfo = FI_packedCallbackInfo(ast);
                XTP_IMPL.functionInfo = YTP_IMPL.functionInfo = CFI;

                /// \code
                ///     int invoke(void *callback, int a, double b) {
                ///         int ret;
                ///         // get callback
                ///         ret = MetaProcCB<PFN_foo, CProcKind_HostCallback,
                ///                                   CProcThunkPhase_GTP_IMPL>::invoke(
                ///                                        callback, a, b
                ///                                   );
                ///         return ret;
                ///     }
                /// \endcode
                XTP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                XTP.body.prolog.push_back(key, SRC_getCallback(!isHostCallback));
                XTP.body.center.push_back(
                    key, SRC_callListAssign(CFI, getMetaProcCBInvoke(procKind_str, XTP_IMPL_str)));
                XTP.body.epilog.push_back(key, SRC_returnRet(FI));

                /// \code
                ///     int invoke(void *callback, int a, double b) {
                ///         int ret;
                ///         void *args[] = { &a, &b, };
                ///         ret = MetaProcCBExec<PFN_foo,CProcKind_HostCallback>::invoke(
                ///                   callback, args, ret, nullptr
                ///               );
                ///         return ret;
                ///     }
                /// \endcode
                XTP_IMPL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                XTP_IMPL.body.prolog.push_back(key, SRC_argPtrListDecl(FI));
                XTP_IMPL.body.center.push_back(
                    key, SRC_asIs(getMetaProcCBExecInvokeWithCallList(procKind_str)));
                XTP_IMPL.body.epilog.push_back(key, SRC_returnRet(FI));

                /// \code
                ///     void invoke(void *callback, void **args, void *ret, void *metadata) {
                ///         auto &arg1 = *(int *) args[0];
                ///         auto &arg2 = *(double *) args[1];
                ///         auto &ret_ref = *(int *) ret;
                ///         ret_ref = MetaProcCB<PFN_foo, CProcKind_HostCallback,
                ///                                       CProcThunkPhase_HTP_IMPL>::invoke(
                ///                                           callback, arg1, arg2
                ///                                       );
                ///     }
                /// \endcode
                YTP.body.prolog.push_back(key, SRC_argPtrListExtractDecl(FI, ast));
                YTP.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
                YTP.body.center.push_back(
                    key, SRC_callListAssign(CFI, getMetaProcCBInvoke(procKind_str, YTP_IMPL_str),
                                            "ret_ref"));

                /// \code
                ///     int invoke(void *callback, int a, double b) {
                ///         int ret;
                ///         ret = MetaProcCBExec<PFN_foo,
                ///                              CProcKind_HostCallback>::invoke(callback, a, b);
                ///         return ret;
                ///     }
                /// \endcode
                YTP_IMPL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                YTP_IMPL.body.center.push_back(
                    key, SRC_callListAssign(
                             CFI, stdc::formatN("MetaProcCBExec<%1, CProcKind_%2>::invoke",
                                                proc.name(), procKind_str)));
                YTP_IMPL.body.epilog.push_back(key, SRC_returnRet(FI));
                break;
            }

            default:
                break;
        }

        return llvm::Error::success();
    }

    llvm::Error DefaultBuilderPass::endHandleProc(ProcContext &proc,
                                                  std::unique_ptr<ProcMessage> &msg) {
        return llvm::Error::success();
    }

    static PassRegistration<DefaultBuilderPass> PR_defaultBuilder;

}