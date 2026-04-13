#include <lorelei/Base/Support/StringExtras.h>
#include <lorelei/Tools/TLCApi/Core/Pass.h>
#include <lorelei/Tools/TLCApi/Core/ProcSnippet.h>
#include <lorelei/Tools/TLCApi/Core/DocumentContext.h>
#include <lorelei/Tools/ThunkInterface/PassTags.h>

#include <clang/AST/Attr.h>

#include "Utils/PassCodeTemplates.h"
#include "Utils/PassUtils.h"

using namespace clang;

namespace lore::tool::TLC {

    class LibCFormatMessage : public PassMessage {
    public:
        LibCFormatMessage(int fmtIdx, int vargIdx) : fmtIdx(fmtIdx), vargIdx(vargIdx) {
        }

        int fmtIdx;
        int vargIdx;
    };

    class LibCFormatPass : public Pass {
    public:
        LibCFormatPass(int id, bool scanf, bool hasVAList)
            : Pass(Builder, id), m_scanf(scanf), m_hasVAList(hasVAList) {
        }

        std::string name() const override {
            return "LibCFormat";
        }

        bool testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        llvm::Error beginHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        llvm::Error endHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;

    protected:
        bool m_scanf;
        bool m_hasVAList;
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

    bool LibCFormatPass::testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) {
        bool passWithDefaultArgs = false;

        // Check pass desc
        if (int passArgs[2]; PASS_getIntegralArgumentsInPass(passArgs, proc, true, id())) {
            if (passArgs[0] < 0 || passArgs[1] < 0) {
                passWithDefaultArgs = true;
            } else {
                msg = std::make_unique<LibCFormatMessage>(passArgs[0], passArgs[1]);
                return true;
            }
        }

        const char *styleName = m_scanf ? "scanf" : "printf";

        // Check format attribute
        if (!m_hasVAList && proc.isFunction() &&
            !(proc.desc() && proc.desc()->overlayType.has_value())) {
            auto FD = proc.functionDecl();
            if (FD) {
                if (auto attr = FD->getAttr<clang::FormatAttr>(); attr) {
                    auto attrType = attr->getType();
                    if (attrType->getName() == styleName) {
                        auto fmtIdx = attr->getFormatIdx();
                        auto vargIdx = attr->getFirstArg();
                        if (fmtIdx > 0 && vargIdx == 0) {
                            msg = std::make_unique<LibCFormatMessage>(fmtIdx, vargIdx);
                            return true;
                        }
                    }
                }
            }
        }

        // Check function name and arguments
        if (passWithDefaultArgs || StringRef(proc.name()).ends_with(styleName)) {
            auto view = proc.realFunctionTypeView();
            auto argTypes = view.argTypes();
            if (m_hasVAList) {
                if (!view.isVariadic() && argTypes.size() > 1) {
                    auto maybeVAListType = argTypes.back();
                    if (getTypeString(maybeVAListType) == "va_list") {
                        auto maybeFmtParam = argTypes[argTypes.size() - 2];
                        if (isCharPointerType(maybeFmtParam)) {
                            msg = std::make_unique<LibCFormatMessage>(argTypes.size() - 1,
                                                                      argTypes.size());
                            return true;
                        }
                    }
                }
            } else {
                if (view.isVariadic() && !argTypes.empty()) {
                    auto maybeFmtParam = argTypes.back();
                    if (isCharPointerType(maybeFmtParam)) {
                        msg = std::make_unique<LibCFormatMessage>(argTypes.size(),
                                                                  argTypes.size() + 1);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    llvm::Error LibCFormatPass::beginHandleProc(ProcSnippet &proc,
                                                std::unique_ptr<PassMessage> &msg) {
        /// \brief Guest-to-Host variadic function thunks.
        /// \example: int (v)printf(const char *fmt, ... | va_list);
        /// \code
        ///     int  invoke(const char *fmt, ... | va_list);            // Guest Entry  (1)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Guest Caller (2)
        ///
        ///     void invoke(void **args, void *ret, void *metadata);    // Host Entry   (3)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Host Caller  (4)
        /// \endcode

        /// \brief Host-to-Guest variadic function thunks.
        /// \example: int (v)printf(const char *fmt, ... | va_list);
        /// \code
        ///     void invoke(void **args, void *ret, void *metadata);    // Guest Entry  (1)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Guest Caller (2)
        ///
        ///     int  invoke(const char *fmt, ... | va_list);            // Host Entry   (3)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Host Caller  (4)
        /// \endcode

        /// \brief Host-to-Guest variadic callback thunks.
        /// \example: int (*PFN_printf_like)(const char *fmt, ... | va_list);
        /// \code
        ///     int  invoke(const char *fmt, ... | va_list);            // Guest Entry  (1)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Guest Caller (2)
        ///
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // Host Entry   (3)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Host Caller  (4)
        /// \endcode

        /// \brief Guest-to-Host variadic callback thunks.
        /// \example: int (*PFN_printf_like)(const char *fmt, ... | va_list);
        /// \code
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // Guest Entry  (1)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Guest Caller (2)
        ///
        ///     int  invoke(const char *fmt, ... | va_list);            // Host Entry   (3)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Host Caller  (4)
        /// \endcode

        /// \brief Callback variadic thunks are analogous to function thunks.
        /// \details Callback path prepends a `void *callback` argument and uses
        ///          `ProcCb<...>` in place of `ProcFn<...>` for dispatch.
        auto message = static_cast<LibCFormatMessage &>(*msg.get());
        int fmtIdx = message.fmtIdx;
        int vargIdx = message.vargIdx;

        auto &doc = proc.document();
        auto &ast = doc.ast();
        const auto &real = proc.realFunctionTypeView();
        bool isVoid = real.returnType()->isVoidType();

        QualType voidType = ast.VoidTy;
        QualType pVoidType = ast.getPointerType(voidType);

        std::string key = name();
        FunctionInfo FI = real;
        FunctionInfo NFI = FI; // normalized FI
        if (m_hasVAList) {
            NFI.argumentsRef().pop_back();
        } else {
            NFI.setVariadic(false);
        }
        NFI.argumentsRef().push_back({{}, "vargs"});
        NFI.setMetaArgumentType("vargs", "CVargEntry *");

        FunctionInfo CNFI = NFI; // callback normalized FI
        CNFI.argumentsRef().insert(CNFI.argumentsRef().begin(), {pVoidType, "callback"});

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

        const char *procDirectionStr = isG2H ? "GuestToHost" : "HostToGuest";
        const auto &getProcFnCallerInvoke = [&]() {
            return formatN("ProcFn<%1, %2, Caller>::invoke", proc.name(), procDirectionStr);
        };
        const auto &getProcCbCallerInvoke = [&]() {
            return formatN("ProcCb<%1, %2, Caller>::invoke", proc.name(), procDirectionStr);
        };
        const auto &getProcFnExecInvokeWithCallList = [&]() {
            return formatN("ProcFn<%1, %2, Exec>::invoke(args, %3, nullptr);", proc.name(),
                           procDirectionStr, isVoid ? "nullptr" : "&ret");
        };
        const auto &getProcCbExecInvokeWithCallList = [&]() {
            return formatN("ProcCb<%1, %2, Exec>::invoke(callback, args, %3, nullptr);",
                           proc.name(), procDirectionStr, isVoid ? "nullptr" : "&ret");
        };

        if (auto &src = doc.source(); !src.properties.count("libc_format")) {
            src.properties["libc_format"] = "1";
            src.head.push_back(
                key, "#include <lorelei/Base/PassThrough/ThunkTools/VariadicAdaptor.h>\n");
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

        auto formatStyleToken = m_scanf ? "ScanF" : "PrintF";
        auto callHelperName = m_hasVAList ? "vcall" : "call";
        auto formatName = FI.argumentName(fmtIdx - 1);
        const auto &getVAListName = [&]() {
            return m_hasVAList ? FI.argumentName(vargIdx - 1) : "ap";
        };
        const auto &getExtractStatment = [&](const std::string &vaListName) {
            return formatN(
                "lore::VariadicAdaptor::extract(lore::VariadicAdaptor::%1, %2, %3, vargs);",
                formatStyleToken, formatName, vaListName);
        };
        const auto &getProcCallHelperAssign = [&]() {
            return formatN("lore::VariadicAdaptor::%1(ProcFn<%2, %3, Exec>::get(), "
                           "sizeof(argv1) / sizeof(argv1[0]), argv1, -1, vargs, &vret);",
                           callHelperName, proc.name(), procDirectionStr);
        };
        const auto &getProcCBCallHelperAssign = [&]() {
            return formatN("lore::VariadicAdaptor::%1(callback, "
                           "sizeof(argv1) / sizeof(argv1[0]), argv1, -1, vargs, &vret);",
                           callHelperName);
        };
        const auto &getVRetDecl = [&]() { return "CVargEntry vret;"; };
        const auto &getVRetInit = [&]() {
            if (isVoid) {
                return std::string("vret.type = 0;");
            }
            return std::string("vret.type = CVargTypeID(ret);");
        };
        const auto &getVRetValueAssign = [&]() {
            if (isVoid) {
                return std::string();
            }
            return formatN("ret = CVargValue(%1, vret);", getTypeString(FI.returnType()));
        };

        auto lastArgToken = FI.argumentName(vargIdx - 2);
        int fixedArgCount = (m_hasVAList ? (vargIdx - 1) : vargIdx) - 1;

        if (proc.isFunction()) {
            XENT.functionInfo = FI;
            XCAL.functionInfo = NFI;
            YENT.functionInfo = FI_packedFunctionInfo(ast);
            YCAL.functionInfo = NFI;

            /// \code
            ///     // printf
            ///     int invoke(const char *fmt, ...) {
            ///         int ret;
            ///         CVargEntry vargs[LORETHUNK_CONFIG_VARG_MAX];
            ///         {
            ///             va_list ap;
            ///             va_start(ap, fmt);
            ///             lore::VariadicAdaptor::extract(
            ///                 lore::VariadicAdaptor::PrintF, fmt, ap, vargs
            ///             );
            ///             va_end(ap);
            ///         }
            ///         ret = ProcFn<printf, GuestToHost, Caller>::invoke(fmt, vargs);
            ///         return ret;
            ///     }
            ///
            ///     // vprintf
            ///     int invoke(const char *fmt, va_list ap) {
            ///         int ret;
            ///         CVargEntry vargs[LORETHUNK_CONFIG_VARG_MAX];
            ///         lore::VariadicAdaptor::extract(
            ///             lore::VariadicAdaptor::PrintF, fmt, ap, vargs
            ///         );
            ///         ret = ProcFn<vprintf, GuestToHost, Caller>::invoke(fmt, vargs);
            ///         return ret;
            ///     }
            /// \endcode
            XENT.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XENT.body.prolog.push_back(key,
                                       SRC_asIs("CVargEntry vargs[LORE_THUNK_CONFIG_VARG_MAX];"));
            if (m_hasVAList) {
                XENT.body.center.push_back(key, SRC_asIs(getExtractStatment(getVAListName())));
            } else {
                XENT.body.center.push_back(key,
                                           addVAStartEnd(lastArgToken, getExtractStatment("ap")));
            }
            XENT.body.center.push_back(key, SRC_callListAssign(NFI, getProcFnCallerInvoke()));
            XENT.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \code
            ///     // printf | vprintf
            ///     int invoke(const char *fmt, CVargEntry *vargs) {
            ///         int ret;
            ///         void *args[] = { &fmt, &vargs };
            ///         ret = ProcFn<printf, GuestToHost, Exec>::invoke(args, &ret, nullptr);
            ///         return ret;
            ///     }
            /// \endcode
            XCAL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XCAL.body.prolog.push_back(key, SRC_argPtrListDecl(NFI));
            XCAL.body.center.push_back(key, SRC_asIs(getProcFnExecInvokeWithCallList()));
            XCAL.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \code
            ///     void invoke(void **args, void *ret, void *metadata) {
            ///         auto &arg1 = *(const char **) args[0];
            ///         auto &vargs = *(CVargEntry **) args[1];
            ///         auto &ret_ref = *(int *) ret;
            ///         ret_ref = ProcFn<printf, GuestToHost, Caller>::invoke(arg1, vargs);
            ///     }
            /// \endcode
            YENT.body.prolog.push_back(key, SRC_argPtrListExtractDecl(NFI, ast));
            YENT.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
            YENT.body.center.push_back(key,
                                       SRC_callListAssign(NFI, getProcFnCallerInvoke(), "ret_ref"));

            /// \code
            ///     int invoke(const char *fmt, CVargEntry *vargs) {
            ///         int ret;
            ///         CVargEntry argv1[] = {
            ///             CVargGet(fmt),
            ///         };
            ///         CVargEntry vret;
            ///         vret.type = CVargTypeID(ret);
            ///         lore::VariadicAdaptor::call(
            ///             ProcFn<printf, GuestToHost, Exec>::get(),
            ///             sizeof(argv1) / sizeof(argv1[0]),
            ///             argv1,
            ///             -1,
            ///             vargs,
            ///             &vret
            ///         );
            ///         ret = CVargValue(int, vret);
            ///         return ret;
            ///     }
            /// \endcode
            YCAL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            YCAL.body.prolog.push_back(key, SRC_fixedArgEntriesDecl(FI, fixedArgCount));
            YCAL.body.prolog.push_back(key, SRC_asIs(getVRetDecl()));
            YCAL.body.prolog.push_back(key, SRC_asIs(getVRetInit()));
            YCAL.body.center.push_back(key, SRC_asIs(getProcCallHelperAssign()));
            YCAL.body.center.push_back(key, SRC_asIs(getVRetValueAssign()));
            YCAL.body.epilog.push_back(key, SRC_returnRet(FI));
        } else {
            XENT.functionInfo = FI;
            XCAL.functionInfo = CNFI;
            YENT.functionInfo = FI_packedCallbackInfo(ast);
            YCAL.functionInfo = CNFI;

            /// \code
            ///     // printf
            ///     int invoke(const char *fmt, ...) {
            ///         int ret;
            ///         // get callback
            ///         CVargEntry vargs[LORETHUNK_CONFIG_VARG_MAX];
            ///         {
            ///             va_list ap;
            ///             va_start(ap, fmt);
            ///             lore::VariadicAdaptor::extract(
            ///                 lore::VariadicAdaptor::PrintF, fmt, ap, vargs
            ///             );
            ///             va_end(ap);
            ///         }
            ///         ret = ProcCb<PFN_printf_like, HostToGuest, Caller>::invoke(
            ///             callback, fmt, vargs
            ///         );
            ///         return ret;
            ///     }
            /// \endcode
            XENT.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XENT.body.prolog.push_back(key, SRC_getCallback(!isG2H));
            XENT.body.prolog.push_back(key,
                                       SRC_asIs("CVargEntry vargs[LORETHUNK_CONFIG_VARG_MAX];"));
            if (m_hasVAList) {
                XENT.body.center.push_back(key, SRC_asIs(getExtractStatment(getVAListName())));
            } else {
                XENT.body.center.push_back(key,
                                           addVAStartEnd(lastArgToken, getExtractStatment("ap")));
            }
            XENT.body.center.push_back(key, SRC_callListAssign(CNFI, getProcCbCallerInvoke()));
            XENT.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \code
            ///     int invoke(void *callback, const char *fmt, CVargEntry *vargs) {
            ///         int ret;
            ///         void *args[] = { &fmt, &vargs };
            ///         ret = ProcCb<PFN_printf_like, HostToGuest, Exec>::invoke(
            ///             callback, args, &ret, nullptr
            ///         );
            ///         return ret;
            ///     }
            /// \endcode
            XCAL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XCAL.body.prolog.push_back(key, SRC_argPtrListDecl(NFI));
            XCAL.body.center.push_back(key, SRC_asIs(getProcCbExecInvokeWithCallList()));
            XCAL.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \code
            ///     void invoke(void *callback, void **args, void *ret, void *metadata) {
            ///         auto &arg1 = *(const char **) args[0];
            ///         auto &vargs = *(CVargEntry **) args[1];
            ///         auto &ret_ref = *(int *) ret;
            ///         ret_ref = ProcCb<PFN_printf_like, HostToGuest, Caller>::invoke(
            ///             callback, arg1, vargs
            ///         );
            ///     }
            /// \endcode
            YENT.body.prolog.push_back(key, SRC_argPtrListExtractDecl(NFI, ast));
            YENT.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
            YENT.body.center.push_back(
                key, SRC_callListAssign(CNFI, getProcCbCallerInvoke(), "ret_ref"));

            /// \code
            ///     int invoke(void *callback, const char *fmt, CVargEntry *vargs) {
            ///         int ret;
            ///         CVargEntry argv1[] = {
            ///             CVargGet(fmt),
            ///         };
            ///         CVargEntry vret;
            ///         vret.type = CVargTypeID(ret);
            ///         lore::VariadicAdaptor::call(
            ///             callback,
            ///             sizeof(argv1) / sizeof(argv1[0]),
            ///             argv1,
            ///             -1,
            ///             vargs,
            ///             &vret
            ///         );
            ///         ret = CVargValue(int, vret);
            ///         return ret;
            ///     }
            /// \endcode
            YCAL.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            YCAL.body.prolog.push_back(key, SRC_fixedArgEntriesDecl(FI, fixedArgCount));
            YCAL.body.prolog.push_back(key, SRC_asIs(getVRetDecl()));
            YCAL.body.prolog.push_back(key, SRC_asIs(getVRetInit()));
            YCAL.body.center.push_back(key, SRC_asIs(getProcCBCallHelperAssign()));
            YCAL.body.center.push_back(key, SRC_asIs(getVRetValueAssign()));
            YCAL.body.epilog.push_back(key, SRC_returnRet(FI));
        }

        return llvm::Error::success();
    }

    llvm::Error LibCFormatPass::endHandleProc(ProcSnippet &proc,
                                              std::unique_ptr<PassMessage> &msg) {
        return llvm::Error::success();
    }

    class PrintFPass : public LibCFormatPass {
    public:
        PrintFPass() : LibCFormatPass(lore::thunk::pass::ID_printf, false, false) {
        }
    };
    class VPrintFPass : public LibCFormatPass {
    public:
        VPrintFPass() : LibCFormatPass(lore::thunk::pass::ID_vprintf, false, true) {
        }
    };
    class ScanFPass : public LibCFormatPass {
    public:
        ScanFPass() : LibCFormatPass(lore::thunk::pass::ID_scanf, true, false) {
        }
    };
    class VScanFPass : public LibCFormatPass {
    public:
        VScanFPass() : LibCFormatPass(lore::thunk::pass::ID_vscanf, true, true) {
        }
    };

    static llvm::Registry<Pass>::Add<PrintFPass> PR_printf("printf", {});
    static llvm::Registry<Pass>::Add<VPrintFPass> PR_vprintf("vprintf", {});
    static llvm::Registry<Pass>::Add<ScanFPass> PR_scanf("scanf", {});
    static llvm::Registry<Pass>::Add<VScanFPass> PR_vscanf("vscanf", {});

}
