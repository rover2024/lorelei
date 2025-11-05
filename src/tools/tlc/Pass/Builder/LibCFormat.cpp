#include "Pass.h"

#include <clang/AST/Attr.h>

#include <stdcorelib/str.h>

#include <lorelei/TLCMeta/MetaPass.h>

#include "ProcContext.h"
#include "DocumentContext.h"
#include "PassCommon.h"
#include "BuilderPassCommon.h"

using namespace clang;

namespace TLC {

    class LibCFormatProcMessage : public ProcMessage {
    public:
        LibCFormatProcMessage(int fmtIdx, int vargIdx) : fmtIdx(fmtIdx), vargIdx(vargIdx) {
        }

        int fmtIdx;
        int vargIdx;
    };

    class LibCFormatPass : public Pass {
    public:
        LibCFormatPass(int id, bool scanf, bool hasVAList)
            : Pass(Builder, id), _scanf(scanf), _hasVAList(hasVAList) {
        }

        std::string name() const override {
            return "LibCFormat";
        }

        bool testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) const override;
        llvm::Error beginHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;
        llvm::Error endHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;

    protected:
        bool _scanf;
        bool _hasVAList;
    };

    static inline bool isCharPointerType(const clang::QualType &type) {
        return type->isPointerType() && type->getPointeeType()->isCharType();
    }

    static inline std::string SRC_fixedArgEntriesDecl(const FunctionInfo &info, int fixedCount,
                                                      int indent = 4) {
        std::string res;
        res += std::string(indent, ' ') + "CVargEntry argv1[] = {\n";
        indent += 4;
        for (int i = 0; i < fixedCount; ++i) {
            res += std::string(indent, ' ') + "CVargGet(" + info.arguments()[i].second + "),\n";
        }
        indent -= 4;
        res += std::string(indent, ' ') + "};\n";
        return res;
    }

    bool LibCFormatPass::testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) const {
        bool passWithDefaultArgs = false;

        // Check pass desc
        if (int passArgs[2]; PASS_getPassIntegralTemplates(passArgs, proc, id())) {
            if (passArgs[0] < 0 || passArgs[1] < 0) {
                passWithDefaultArgs = true;
            } else {
                msg = std::make_unique<LibCFormatProcMessage>(passArgs[0], passArgs[1]);
                return true;
            }
        }

        const char *styleName = _scanf ? "scanf" : "printf";

        // Check format attribute
        if (!_hasVAList && proc.isFunction() && !proc.overlayType()) {
            auto FD = proc.functionDecl();
            if (auto attr = FD->getAttr<clang::FormatAttr>(); attr) {
                auto attrType = attr->getType();
                if (attrType->getName() == styleName) {
                    auto fmtIdx = attr->getFormatIdx();
                    auto vargIdx = attr->getFirstArg();
                    if (fmtIdx > 0 && vargIdx == 0) {
                        msg = std::make_unique<LibCFormatProcMessage>(fmtIdx, vargIdx);
                        return true;
                    }
                }
            }
        }

        // Check function name and arguments
        if (passWithDefaultArgs || StringRef(proc.name()).ends_with(styleName)) {
            auto view = proc.realFunctionTypeView();
            auto argTypes = view.argTypes();
            if (_hasVAList) {
                if (!view.isVariadic() && argTypes.size() > 1) {
                    auto maybeVAListType = argTypes.back();
                    if (getTypeString(maybeVAListType) == "va_list") {
                        auto maybeFmtParam = argTypes[argTypes.size() - 2];
                        if (isCharPointerType(maybeFmtParam)) {
                            msg = std::make_unique<LibCFormatProcMessage>(argTypes.size() - 1,
                                                                          argTypes.size());
                            return true;
                        }
                    }
                }
            } else {
                if (view.isVariadic() && argTypes.size() > 0) {
                    auto maybeFmtParam = argTypes.back();
                    if (isCharPointerType(maybeFmtParam)) {
                        msg = std::make_unique<LibCFormatProcMessage>(argTypes.size(),
                                                                      argTypes.size() + 1);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    llvm::Error LibCFormatPass::beginHandleProc(ProcContext &proc,
                                                std::unique_ptr<ProcMessage> &msg) {
        /// \brief Host function thunks
        /// \example: int (v)printf(const char *fmt, ... | va_list);
        /// \code
        ///     int  invoke(const char *fmt, ... | va_list);            // GTP      (1)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // GTP_IMPL (2)
        ///
        ///     void invoke(void **args, void *ret, void *metadata);    // HTP      (3)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // HTP_IMPL (4)
        /// \endcode

        /// \brief Guest function thunks
        /// \example: int (v)printf(const char *fmt, ... | va_list);
        /// \code
        ///     int  invoke(void **args, void *ret, void *metadata);    // GTP      (1)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // GTP_IMPL (2)
        ///
        ///     int  invoke(const char *fmt, ... | va_list);            // HTP      (3)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // HTP_IMPL (4)
        /// \endcode

        /// \brief Guest callback thunks
        /// \example: int (*foo)(int a, double b);
        /// \code
        ///     int  invoke(const char *fmt, ... | va_list);            // GTP      (1)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // GTP_IMPL (2)
        ///
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // HTP      (3)
        ///     int  invoke(void *callback,
        //                  const char *fmt, CVargEntry *vargs);        // HTP_IMPL (4)
        /// \endcode

        /// \brief Host callback thunks
        /// \example: int (*foo)(int a, double b);
        /// \code
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // GTP      (1)
        ///     int  invoke(void *callback,
        //                  const char *fmt, CVargEntry *vargs);        // GTP_IMPL (2)
        ///
        ///     int  invoke(const char *fmt, ... | va_list);            // HTP      (3)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // HTP_IMPL (4)
        /// \endcode

        auto message = static_cast<LibCFormatProcMessage *>(msg.get());
        int fmtIdx = message->fmtIdx;
        int vargIdx = message->vargIdx;

        auto &doc = proc.documentContext();
        auto &ast = *doc.ast();
        const auto &real = proc.realFunctionTypeView();
        bool isVoid = real.returnType()->isVoidType();

        QualType voidType = ast.VoidTy;
        QualType pVoidType = ast.getPointerType(voidType);
        QualType ppVoidType = ast.getPointerType(pVoidType);

        std::string key = name();
        FunctionInfo FI = real;
        FunctionInfo NFI = FI; // normalized FI
        if (_hasVAList) {
            NFI.argumentsRef().pop_back();
        } else {
            NFI.setVariadic(false);
        }
        NFI.argumentsRef().push_back({{}, "vargs"});
        NFI.setMetaArgumentType("vargs", "CVargEntry *");
        FunctionInfo CNFI = NFI; // callback normalized FI
        CNFI.argumentsRef().insert(CNFI.argumentsRef().begin(), {pVoidType, "callback"});

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
        const auto &getMetaProcBridgeInvokeWithCallList = [&](const char *kind) {
            return stdc::formatN("MetaProcBridge<::%1, CProcKind_%2>::invoke(args, %3, nullptr);",
                                 proc.name(), kind, isVoid ? "nullptr" : "&ret");
        };
        const auto &getMetaProcCBBridgeInvokeWithCallList = [&](const char *kind) {
            return stdc::formatN(
                "MetaProcCBBridge<%1, CProcKind_%2>::invoke(callback, args, %3, nullptr);",
                proc.name(), kind, isVoid ? "nullptr" : "&ret");
        };

        // Add includes
        if (auto &src = doc.source(); !src.properties.count("libc_format")) {
            src.properties["libc_format"] = "1";
            src.head.push_back(key, "#include <lorelei/Core/ThunkTools/VariadicAdaptor.h>\n"
                                    "#include <lorelei-c/Core/VariadicAdaptor_c.h>\n");
        }

        const auto &addVAStartEnd = [](const std::string &fixedArgToken, const std::string &content,
                                       int indent = 4) {
            std::string result;
            result += std::string(indent, ' ') + "{\n";
            indent += 4;
            result += std::string(indent, ' ') + "va_list ap;\n";
            result += std::string(indent, ' ') + "va_start(ap, " + fixedArgToken + ");\n";
            result += std::string(indent, ' ') + content + "\n";
            result += std::string(indent, ' ') + "va_end(ap);\n";
            indent -= 4;
            result += std::string(indent, ' ') + "}\n";
            return result;
        };

        auto formatStyleToken = _scanf ? "FS_scanf" : "FS_printf";
        auto callHelperName = _hasVAList ? "vcall" : "call";
        auto formatName = FI.argumentName(fmtIdx - 1);
        const auto &getVAListName = [&]() {
            return _hasVAList ? FI.argumentName(vargIdx - 1) : "ap";
        };
        const auto &getExtractStatment = [&](const std::string &vaListName) {
            return stdc::formatN(
                "lore::VariadicAdaptor::extract(lore::VariadicAdaptor::%1, %2, %3, vargs);",
                formatStyleToken, formatName, vaListName);
        };
        const auto &getProcCallHelperAssign = [&](const char *kind) {
            return stdc::formatN(
                "lore::VariadicAdaptor::%1(MetaProcBridge<%2, CProcKind_%3>::get(), "
                "sizeof(argv1) / sizeof(argv1[0]), argv1, -1, vargs, &vret);",
                callHelperName, proc.name(), kind);
        };
        const auto &getProcCBCallHelperAssign = [&]() {
            return stdc::formatN("lore::VariadicAdaptor::%1(callback, "
                                 "sizeof(argv1) / sizeof(argv1[0]), argv1, -1, vargs, &vret);",
                                 callHelperName);
        };
        const auto &getVretDecl = [&]() {
            return "CVargEntry vret;";
        };
        const auto &getVRetInit = [&]() {
            if (isVoid)
                return "vret.type = 0;";
            return "vret.type = CVargTypeID(ret);";
        };
        const auto &getVRetValueAssign = [&]() {
            if (isVoid)
                return std::string();
            return stdc::formatN("ret = CVargValue(%1, vret);", getTypeString(FI.returnType()));
        };

        auto lastArgToken = FI.argumentName(vargIdx - 2);
        int fixedArgCount = (_hasVAList ? (vargIdx - 1) : vargIdx) - 1;

        switch (proc.procKind()) {
            case CProcKind_HostFunction: {
                bool isHostFunction = proc.procKind() == CProcKind_HostFunction;

                auto &XTP = isHostFunction ? GTP : HTP;
                auto &XTP_IMPL = isHostFunction ? GTP_IMPL : HTP_IMPL;
                auto &YTP = isHostFunction ? HTP : GTP;
                auto &YTP_IMPL = isHostFunction ? HTP_IMPL : GTP_IMPL;

                const char *procKind_str = isHostFunction ? "HostFunction" : "GuestFunction";
                const char *XTP_IMPL_str = isHostFunction ? "GTP_IMPL" : "HTP_IMPL";
                const char *YTP_IMPL_str = isHostFunction ? "HTP_IMPL" : "GTP_IMPL";

                XTP.functionInfo = FI;
                YTP.functionInfo = FI_bridgeFunctionInfo(ast);
                XTP_IMPL.functionInfo = YTP_IMPL.functionInfo = NFI;

                /// \code
                ///     // printf
                ///     int invoke(const char *fmt, ...) {
                ///         int ret;
                ///         CVargEntry vargs[LORE_CONFIG_VARG_MAX];
                ///         {
                ///             va_list ap;
                ///             va_start(ap, fmt);
                ///             VariadicAdaptor::extract(VariadicAdaptor::FS_printf,
                ///                                      fmt, ap, vargs);
                ///             va_end(ap);
                ///         }
                ///         ret = MetaProc<printf, CProcKind_HostFunction,
                ///                                CProcThunkPhase_GTP_IMPL>::invoke(fmt, vargs);
                ///         return ret;
                ///     }
                ///
                ///     // vprintf
                ///     int invoke(const char *fmt, va_list ap) {
                ///         int ret;
                ///         CVargEntry vargs[LORE_CONFIG_VARG_MAX];
                ///         VariadicAdaptor::extract(VariadicAdaptor::FS_printf, fmt, ap, vargs);
                ///         ret = MetaProc<vprintf, CProcKind_HostFunction,
                ///                                 CProcThunkPhase_GTP_IMPL>::invoke(fmt, vargs);
                ///         return ret;
                ///     }
                /// \endcode
                XTP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                XTP.body.prolog.push_back(key, SRC_asIs("CVargEntry vargs[LORE_CONFIG_VARG_MAX];"));
                if (_hasVAList) {
                    XTP.body.center.push_back(key, SRC_asIs(getExtractStatment(getVAListName())));
                } else {
                    XTP.body.center.push_back(
                        key, addVAStartEnd(lastArgToken, getExtractStatment("ap")));
                }
                XTP.body.center.push_back(
                    key, SRC_callListAssign(NFI, getMetaProcInvoke(procKind_str, XTP_IMPL_str)));
                XTP.body.epilog.push_back(key, SRC_returnRet(FI));

                /// \code
                ///     // printf | vprintf
                ///     int invoke(const char *fmt, CVargEntry *vargs) {
                ///         int ret;
                ///         void *args[] = { &fmt, &vargs };
                ///         ret = MetaProcBridge<printf | vprintf,
                ///                              CProcKind_HostFunction>::invoke(
                ///                                  args, &ret, nullptr
                ///                              );
                ///         return ret;
                ///     }
                /// \endcode
                XTP_IMPL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                XTP_IMPL.body.prolog.push_back(key, SRC_argPtrListDecl(NFI));
                XTP_IMPL.body.center.push_back(
                    key, SRC_asIs(getMetaProcBridgeInvokeWithCallList(procKind_str)));
                XTP_IMPL.body.epilog.push_back(key, SRC_returnRet(FI));

                /// \code
                ///     void invoke(void **args, void *ret, void *metadata) {
                ///         auto &arg1 = *(const char **) args[0];
                ///         auto &vargs = *(CVargEntry **) args[1];
                ///         auto &ret_ref = *(int *) ret;
                ///         ret_ref = MetaProc<printf | vprintf,
                //                             CProcKind_HostFunction,
                ///                            CProcThunkPhase_HTP_IMPL>::invoke(arg1, vargs);
                ///     }
                /// \endcode
                YTP.body.prolog.push_back(key, SRC_argPtrListExtractDecl(NFI, ast));
                YTP.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
                YTP.body.center.push_back(
                    key, SRC_callListAssign(NFI, getMetaProcInvoke(procKind_str, YTP_IMPL_str),
                                            "ret_ref"));

                /// \code
                ///     int invoke(const char *fmt, CVargEntry *vargs) {
                ///         int ret;
                ///         CVArgEntry argv1[] = {
                ///             CVargGet(fmt),
                ///         };
                ///         CVargEntry vret;
                ///         vret.type = CVargTypeID(ret);
                ///         VariadicAdaptor::call | VariadicAdaptor::vcall (
                ///             MetaProcBridge<printf | vprintf,
                ///                            CProcKind_HostFunction>::get(),
                ///             sizeof(argv1) / sizeof(argv1[0]),
                ///             argv1,
                ///             -1,
                ///             vargs,
                ///             &vret
                ///         );
                ///         ret = CVargValue(int, va_ret);
                ///         return ret;
                ///     }
                /// \endcode
                YTP_IMPL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                YTP_IMPL.body.prolog.push_back(key, SRC_fixedArgEntriesDecl(FI, fixedArgCount));
                YTP_IMPL.body.prolog.push_back(key, SRC_asIs(getVretDecl()));
                YTP_IMPL.body.prolog.push_back(key, SRC_asIs(getVRetInit()));
                YTP_IMPL.body.center.push_back(key,
                                               SRC_asIs(getProcCallHelperAssign(procKind_str)));
                YTP_IMPL.body.center.push_back(key, SRC_asIs(getVRetValueAssign()));
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
                YTP.functionInfo = FI_bridgeCallbackInfo(ast);
                XTP_IMPL.functionInfo = YTP_IMPL.functionInfo = CNFI;

                /// \code
                ///     // printf
                ///     int invoke(const char *fmt, ...) {
                ///         int ret;
                ///         // get callback
                ///         CVargEntry vargs[LORE_CONFIG_VARG_MAX];
                ///         {
                ///             va_list ap;
                ///             va_start(ap, fmt);
                ///             VariadicAdaptor::extract(VariadicAdaptor::FS_printf,
                ///                                      fmt, ap, vargs);
                ///             va_end(ap);
                ///         }
                ///         ret = MetaProcCB<printf, CProcKind_HostCallback,
                ///                                  CProcThunkPhase_GTP_IMPL>::invoke(
                ///                                      callback, fmt, vargs
                ///                                  );
                ///         return ret;
                ///     }
                ///
                ///     // vprintf
                ///     int invoke(const char *fmt, va_list ap) {
                ///         int ret;
                ///         CVargEntry vargs[LORE_CONFIG_VARG_MAX];
                ///         VariadicAdaptor::extract(VariadicAdaptor::FS_printf, fmt, ap, vargs);
                ///         ret = MetaProcCB<vprintf, CProcKind_HostCallback,
                ///                                   CProcThunkPhase_GTP_IMPL>::invoke(
                ///                                       callback, fmt, vargs
                ///                                   );
                ///         return ret;
                ///     }
                /// \endcode
                XTP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                XTP.body.prolog.push_back(key, SRC_getCallback(!isHostCallback));
                XTP.body.prolog.push_back(key, SRC_asIs("CVargEntry vargs[LORE_CONFIG_VARG_MAX];"));
                if (_hasVAList) {
                    XTP.body.center.push_back(key, SRC_asIs(getExtractStatment(getVAListName())));
                } else {
                    XTP.body.center.push_back(
                        key, addVAStartEnd(lastArgToken, getExtractStatment("ap")));
                }
                XTP.body.center.push_back(
                    key, SRC_callListAssign(CNFI, getMetaProcCBInvoke(procKind_str, XTP_IMPL_str)));
                XTP.body.epilog.push_back(key, SRC_returnRet(FI));

                /// \code
                ///     // printf | vprintf
                ///     int invoke(void *callback, const char *fmt, CVargEntry *vargs) {
                ///         int ret;
                ///         void *args[] = { &fmt, &vargs, };
                ///         ret = MetaProcCBBridge<printf | vprintf,
                ///                                CProcKind_HostCallback>::invoke(
                ///                                    callback, args, &ret, nullptr
                ///                                );
                ///         return ret;
                ///     }
                /// \endcode
                XTP_IMPL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                XTP_IMPL.body.prolog.push_back(key, SRC_argPtrListDecl(NFI));
                XTP_IMPL.body.center.push_back(
                    key, SRC_asIs(getMetaProcCBBridgeInvokeWithCallList(procKind_str)));
                XTP_IMPL.body.epilog.push_back(key, SRC_returnRet(FI));

                /// \code
                ///     void invoke(void *callback, void **args, void *ret, void *metadata) {
                ///         auto &arg1 = *(const char **) args[0];
                ///         auto &vargs = *(CVargEntry **) args[1];
                ///         auto &ret_ref = *(int *) ret;
                ///         ret_ref = MetaProcCB<printf | vprintf,
                ///                            CProcKind_HostCallback,
                ///                            CProcThunkPhase_HTP_IMPL>::invoke(
                ///                                callback, arg1, vargs
                ///                            );
                ///     }
                /// \endcode
                YTP.body.prolog.push_back(key, SRC_argPtrListExtractDecl(NFI, ast));
                YTP.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
                YTP.body.center.push_back(
                    key, SRC_callListAssign(CNFI, getMetaProcCBInvoke(procKind_str, YTP_IMPL_str),
                                            "ret_ref"));

                /// \code
                ///     void invoke(void *callback, const char *fmt, CVargEntry *vargs) {
                ///         int ret;
                ///         CVArgEntry argv1[] = {
                ///             CVargGet(fmt),
                ///         };
                ///         CVargEntry vret;
                ///         vret.type = CVargTypeID(ret);
                ///         VariadicAdaptor::call | VariadicAdaptor::vcall (
                ///             callback,
                ///             sizeof(argv1) / sizeof(argv1[0]),
                ///             argv1,
                ///             -1,
                ///             vargs,
                ///             &vret
                ///         );
                ///         ret = CVargValue(int, va_ret);
                ///         return ret;
                ///     }
                /// \endcode
                YTP_IMPL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
                YTP_IMPL.body.prolog.push_back(key, SRC_fixedArgEntriesDecl(FI, fixedArgCount));
                YTP_IMPL.body.prolog.push_back(key, SRC_asIs(getVretDecl()  ));
                YTP_IMPL.body.prolog.push_back(key, SRC_asIs(getVRetInit()));
                YTP_IMPL.body.center.push_back(key, SRC_asIs(getProcCBCallHelperAssign()));
                YTP_IMPL.body.center.push_back(key, SRC_asIs(getVRetValueAssign()));
                YTP_IMPL.body.epilog.push_back(key, SRC_returnRet(FI));
                break;
            }

            default:
                break;
        };

        return llvm::Error::success();
    }

    llvm::Error LibCFormatPass::endHandleProc(ProcContext &proc,
                                              std::unique_ptr<ProcMessage> &msg) {
        return llvm::Error::success();
    }

    static PassRegistration<LibCFormatPass> PR_printf(lorethunk::MetaPass::Printf, false, false);
    static PassRegistration<LibCFormatPass> PR_vprintf(lorethunk::MetaPass::VPrintf, false, true);
    static PassRegistration<LibCFormatPass> PR_scanf(lorethunk::MetaPass::Scanf, true, false);
    static PassRegistration<LibCFormatPass> PR_vscanf(lorethunk::MetaPass::VScanf, true, true);

}