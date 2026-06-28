// SPDX-License-Identifier: MIT

#include <lorelei/Support/StringExtras.h>
#include <lorelei/TLCApi/Pass.h>
#include <lorelei/TLCApi/ProcSnippet.h>
#include <lorelei/TLCApi/DocumentContext.h>
#include <lorelei/ThunkInterface/PassTags.h>

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

        // Check format attribute. A printf/scanf-style function is usually annotated with
        // __attribute__((format(printf, fmtIdx, firstToCheck))). firstToCheck is the 1-based
        // position of the first variadic argument, or 0 for a va_list function (which has no
        // variadic arguments to type-check). This lets the builders recognise such functions
        // (SDL_LogMessage, glibc printf / vprintf, ...) without a manifest descriptor. Note that
        // some libraries (SDL) omit the attribute on their va_list forms, which then still need a
        // name match or an explicit vprintf/vscanf tag.
        if (proc.isFunction() && !(proc.desc() && proc.desc()->overlayType.has_value())) {
            auto FD = proc.functionDecl();
            if (FD) {
                if (auto attr = FD->getAttr<clang::FormatAttr>(); attr) {
                    if (attr->getType()->getName() == styleName) {
                        auto fmtIdx = attr->getFormatIdx();
                        auto vargIdx = attr->getFirstArg();
                        if (!m_hasVAList && fmtIdx > 0 && vargIdx > 0) {
                            // `...`: the attribute carries the variadic position directly.
                            msg = std::make_unique<LibCFormatMessage>(fmtIdx, vargIdx);
                            return true;
                        }
                        if (m_hasVAList && fmtIdx > 0 && vargIdx == 0) {
                            // va_list: the list immediately follows the format string.
                            msg = std::make_unique<LibCFormatMessage>(fmtIdx, fmtIdx + 1);
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
                    bool isVAList = getTypeString(maybeVAListType) == "va_list";
                    // The view's argument types are canonical, so on riscv64 the va_list parameter
                    // shows up as a plain void* and the string match above fails. Recover it from
                    // the function declaration's sugared last parameter, where the va_list typedef
                    // is still present. A callback proc has no FunctionDecl and keeps the old
                    // behavior (a va_list-taking callback is unsupported).
                    if (!isVAList && proc.functionDecl()) {
                        auto params = proc.functionDecl()->parameters();
                        if (params.size() == argTypes.size() && !params.empty()) {
                            isVAList =
                                isVaListType(proc.document().ast(), params.back()->getOriginalType());
                        }
                    }
                    if (isVAList) {
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
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Guest Adapt  (2)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Guest Caller (3)
        ///
        ///     void invoke(void **args, void *ret, void *metadata);    // Host Entry   (4)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Host Adapt   (5)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Host Caller  (6)
        /// \endcode

        /// \brief Host-to-Guest variadic function thunks.
        /// \example: int (v)printf(const char *fmt, ... | va_list);
        /// \code
        ///     void invoke(void **args, void *ret, void *metadata);    // Guest Entry  (1)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Guest Adapt  (2)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Guest Caller (3)
        ///
        ///     int  invoke(const char *fmt, ... | va_list);            // Host Entry   (4)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Host Adapt   (5)
        ///     int  invoke(const char *fmt, CVargEntry *vargs);        // Host Caller  (6)
        /// \endcode

        /// \brief Host-to-Guest variadic callback thunks.
        /// \example: int (*PFN_printf_like)(const char *fmt, ... | va_list);
        /// \code
        ///     int  invoke(const char *fmt, ... | va_list);            // Guest Entry  (1)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Guest Adapt  (2)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Guest Caller (3)
        ///
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // Host Entry   (4)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Host Adapt   (5)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Host Caller  (6)
        /// \endcode

        /// \brief Guest-to-Host variadic callback thunks.
        /// \example: int (*PFN_printf_like)(const char *fmt, ... | va_list);
        /// \code
        ///     void invoke(void *callback, void **args,
        ///                 void *ret,      void *metadata);            // Guest Entry  (1)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Guest Adapt  (2)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Guest Caller (3)
        ///
        ///     int  invoke(const char *fmt, ... | va_list);            // Host Entry   (4)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Host Adapt   (5)
        ///     int  invoke(void *callback,
        ///                 const char *fmt, CVargEntry *vargs);        // Host Caller  (6)
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
        // Normalized FI: replace the variadic tail (`...` or the va_list parameter) with a single
        // packed `CVargEntry *vargs`, so the cross-side signature is fixed and marshallable.
        FunctionInfo NFI = FI;
        if (m_hasVAList) {
            NFI.argumentsRef().pop_back();
        } else {
            NFI.setVariadic(false);
        }
        NFI.argumentsRef().push_back({{}, "vargs"});
        NFI.setMetaArgumentType("vargs", "CVargEntry *");

        FunctionInfo CNFI = NFI; // callback normalized FI: NFI with a leading `void *callback`
        CNFI.argumentsRef().insert(CNFI.argumentsRef().begin(), {pVoidType, "callback"});

        bool isHost = doc.mode() == DocumentContext::Host;
        bool isG2H = proc.direction() == ProcSnippet::GuestToHost;

        auto &ENT = proc.source(ProcSnippet::Entry);
        auto &ADP = proc.source(ProcSnippet::Adapt);
        auto &CAL = proc.source(ProcSnippet::Caller);
        ProcSnippet::ProcSource emptyENT;
        ProcSnippet::ProcSource emptyADP;
        ProcSnippet::ProcSource emptyCAL;

        // Route the live ProcSource to the side matching the document mode (the other side gets a
        // throw-away `empty*`); X* is the sender side and Y* the receiver side. See DefaultBuilder.
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

        const char *procDirectionStr = isG2H ? "GuestToHost" : "HostToGuest";
        const auto &getProcFnAdaptInvoke = [&]() {
            return formatN("ProcFn<%1, %2, Adapt>::invoke", proc.name(), procDirectionStr);
        };
        const auto &getProcCbAdaptInvoke = [&]() {
            return formatN("ProcCb<%1, %2, Adapt>::invoke", proc.name(), procDirectionStr);
        };
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

        // Adapt is a typed pass-through between Entry and Caller (here the normalized,
        // varg-carrying signature); non-Builder passes inject into its forward/backward.

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
        // The fixed arguments are everything before vargIdx (the `...` position for printf, or the
        // va_list argument for vprintf), so there are vargIdx - 1 of them in both cases. The format
        // string is one of them and must be forwarded, so it has to be included here.
        int fixedArgCount = vargIdx - 1;

        if (proc.isFunction()) {
            XENT.functionInfo = FI;
            XADP.functionInfo = XCAL.functionInfo = NFI;
            YENT.functionInfo = FI_packedFunctionInfo(ast);
            YADP.functionInfo = YCAL.functionInfo = NFI;

            /// \example: Entry (sender)
            /// \code
            ///     // printf
            ///     int invoke(const char *fmt, ...) {
            ///         int ret;
            ///         CVargEntry vargs[LORE_THUNK_VARG_MAX];
            ///         {
            ///             va_list ap;
            ///             va_start(ap, fmt);
            ///             lore::VariadicAdaptor::extract(
            ///                 lore::VariadicAdaptor::PrintF, fmt, ap, vargs
            ///             );
            ///             va_end(ap);
            ///         }
            ///         ret = ProcFn<printf, GuestToHost, Adapt>::invoke(fmt, vargs);
            ///         return ret;
            ///     }
            ///
            ///     // vprintf
            ///     int invoke(const char *fmt, va_list ap) {
            ///         int ret;
            ///         CVargEntry vargs[LORE_THUNK_VARG_MAX];
            ///         lore::VariadicAdaptor::extract(
            ///             lore::VariadicAdaptor::PrintF, fmt, ap, vargs
            ///         );
            ///         ret = ProcFn<vprintf, GuestToHost, Adapt>::invoke(fmt, vargs);
            ///         return ret;
            ///     }
            /// \endcode
            XENT.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XENT.body.prolog.push_back(key, SRC_asIs("CVargEntry vargs[LORE_THUNK_VARG_MAX];"));
            if (m_hasVAList) {
                XENT.body.center.push_back(key, SRC_asIs(getExtractStatment(getVAListName())));
            } else {
                XENT.body.center.push_back(key,
                                           addVAStartEnd(lastArgToken, getExtractStatment("ap")));
            }
            XENT.body.center.push_back(key, SRC_callListAssign(NFI, getProcFnAdaptInvoke()));
            XENT.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Adapt (sender)
            /// \code
            ///     int invoke(const char *fmt, CVargEntry *vargs) {
            ///         int ret;
            ///         ret = ProcFn<printf, GuestToHost, Caller>::invoke(fmt, vargs);
            ///         return ret;
            ///     }
            /// \endcode
            XADP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XADP.body.center.push_back(key, SRC_callListAssign(NFI, getProcFnCallerInvoke()));
            XADP.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller (sender)
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

            /// \example: Entry (receiver)
            /// \code
            ///     void invoke(void **args, void *ret, void *metadata) {
            ///         auto &arg1 = *(const char **) args[0];
            ///         auto &vargs = *(CVargEntry **) args[1];
            ///         auto &ret_ref = *(int *) ret;
            ///         ret_ref = ProcFn<printf, GuestToHost, Adapt>::invoke(arg1, vargs);
            ///     }
            /// \endcode
            YENT.body.prolog.push_back(key, SRC_argPtrListExtractDecl(NFI, ast));
            YENT.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
            YENT.body.center.push_back(key,
                                       SRC_callListAssign(NFI, getProcFnAdaptInvoke(), "ret_ref"));

            /// \example: Adapt (receiver)
            /// \code
            ///     int invoke(const char *fmt, CVargEntry *vargs) {
            ///         int ret;
            ///         ret = ProcFn<printf, GuestToHost, Caller>::invoke(fmt, vargs);
            ///         return ret;
            ///     }
            /// \endcode
            YADP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            YADP.body.center.push_back(key, SRC_callListAssign(NFI, getProcFnCallerInvoke()));
            YADP.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller (receiver)
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
            XADP.functionInfo = XCAL.functionInfo = CNFI;
            YENT.functionInfo = FI_packedCallbackInfo(ast);
            YADP.functionInfo = YCAL.functionInfo = CNFI;

            /// \example: Entry (sender)
            /// \code
            ///     // printf
            ///     int invoke(const char *fmt, ...) {
            ///         int ret;
            ///         // get callback
            ///         CVargEntry vargs[LORE_THUNK_VARG_MAX];
            ///         {
            ///             va_list ap;
            ///             va_start(ap, fmt);
            ///             lore::VariadicAdaptor::extract(
            ///                 lore::VariadicAdaptor::PrintF, fmt, ap, vargs
            ///             );
            ///             va_end(ap);
            ///         }
            ///         ret = ProcCb<PFN_printf_like, HostToGuest, Adapt>::invoke(
            ///             callback, fmt, vargs
            ///         );
            ///         return ret;
            ///     }
            /// \endcode
            XENT.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XENT.body.prolog.push_back(key, SRC_getCallback(!isG2H));
            XENT.body.prolog.push_back(key, SRC_asIs("CVargEntry vargs[LORE_THUNK_VARG_MAX];"));
            if (m_hasVAList) {
                XENT.body.center.push_back(key, SRC_asIs(getExtractStatment(getVAListName())));
            } else {
                XENT.body.center.push_back(key,
                                           addVAStartEnd(lastArgToken, getExtractStatment("ap")));
            }
            XENT.body.center.push_back(key, SRC_callListAssign(CNFI, getProcCbAdaptInvoke()));
            XENT.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Adapt (sender)
            /// \code
            ///     int invoke(void *callback, const char *fmt, CVargEntry *vargs) {
            ///         int ret;
            ///         ret = ProcCb<PFN_printf_like, HostToGuest, Caller>::invoke(callback, fmt, vargs);
            ///         return ret;
            ///     }
            /// \endcode
            XADP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            XADP.body.center.push_back(key, SRC_callListAssign(CNFI, getProcCbCallerInvoke()));
            XADP.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller (sender)
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

            /// \example: Entry (receiver)
            /// \code
            ///     void invoke(void *callback, void **args, void *ret, void *metadata) {
            ///         auto &arg1 = *(const char **) args[0];
            ///         auto &vargs = *(CVargEntry **) args[1];
            ///         auto &ret_ref = *(int *) ret;
            ///         ret_ref = ProcCb<PFN_printf_like, HostToGuest, Adapt>::invoke(
            ///             callback, arg1, vargs
            ///         );
            ///     }
            /// \endcode
            YENT.body.prolog.push_back(key, SRC_argPtrListExtractDecl(NFI, ast));
            YENT.body.prolog.push_back(key, SRC_retExtractDecl(FI, ast));
            YENT.body.center.push_back(key,
                                       SRC_callListAssign(CNFI, getProcCbAdaptInvoke(), "ret_ref"));

            /// \example: Adapt (receiver)
            /// \code
            ///     int invoke(void *callback, const char *fmt, CVargEntry *vargs) {
            ///         int ret;
            ///         ret = ProcCb<PFN_printf_like, HostToGuest, Caller>::invoke(callback, fmt, vargs);
            ///         return ret;
            ///     }
            /// \endcode
            YADP.body.prolog.push_back(key, SRC_emptyReturnDecl(FI, ast));
            YADP.body.center.push_back(key, SRC_callListAssign(CNFI, getProcCbCallerInvoke()));
            YADP.body.epilog.push_back(key, SRC_returnRet(FI));

            /// \example: Caller (receiver)
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
