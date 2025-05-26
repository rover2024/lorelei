#include "passcontext.h"

#include <sstream>

#include <llvm/Support/Format.h>
#include <llvm/ADT/StringExtras.h>

#include "pass.h"

#include "common.h"

using namespace clang;

static constexpr const char GENERATED_WARNING[] =
    R"(/****************************************************************************
** Lorelei thunk library code from reading C file '%s'
**
** Created by: Lorelei Thunk Library compiler
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/
)";

namespace TLC {

    class PassContext::Impl {
    public:
        Impl(PassContext *decl, const std::set<std::string> &FunctionNames,
             const llvm::ArrayRef<const FunctionDecl *> &FunctionDecls, SourceManager *SourceMgr,
             const LangOptions &LangOpts, ASTContext *Context, llvm::StringRef InputFileName,
             llvm::StringRef HeaderFileName, llvm::StringRef SourceFileName)
            : _decl(decl), FunctionDecls(FunctionDecls), SourceMgr(SourceMgr), LangOpts(LangOpts), Context(Context),
              InputFileName(InputFileName), HeaderFileName(HeaderFileName), SourceFileName(SourceFileName) {
            ApiNames = FunctionNames;
        }

        void Begin();
        void RunPass();
        void End();

        PassContext *_decl;

        // Children
        struct ApiData {
            std::map<std::string, std::shared_ptr<ApiSource>> SourceMap;
        };
        std::array<ApiData, ApiSource::HostCallback + 1> Apis;

        // AST
        std::set<std::string> ApiNames;

        SmallVector<const FunctionDecl *> FunctionDecls;
        SourceManager *SourceMgr;
        LangOptions LangOpts;
        ASTContext *Context;

        // File info
        std::string InputFileName;
        std::string HeaderFileName;
        std::string SourceFileName;

        // Result
        std::string ResultDefs;
        std::string ResultImpl;
    };

    PassContext::PassContext(const std::set<std::string> &FunctionNames,
                             const ArrayRef<const FunctionDecl *> &FunctionDecls, SourceManager *SourceMgr,
                             const LangOptions &LangOpts, ASTContext *Context, llvm::StringRef InputFileName,
                             llvm::StringRef HeaderFileName, llvm::StringRef SourceFileName)
        : _impl(std::make_unique<Impl>(this, FunctionNames, FunctionDecls, SourceMgr, LangOpts, Context, InputFileName,
                                       HeaderFileName, SourceFileName)) {
    }

    PassContext::~PassContext() = default;

    void PassContext::Impl::Begin() {
        struct ThunkData {
            const FunctionDecl *Api = nullptr; // only Normal
            const FunctionDecl *Hint = nullptr;
            std::array<const FunctionDecl *, FunctionSource::HTP_IMPL + 1> Thunks;
        };
        std::array<std::map<std::string, ThunkData>, ApiSource::HostCallback + 1> DeclaredThunkMap;

        // collect Api declarations
        for (const auto &FD : std::as_const(FunctionDecls)) {
            const auto &FuncName = FD->getName().str();
            if (ApiNames.count(FuncName)) {
                auto &TD = DeclaredThunkMap[ApiSource::Normal][FuncName];
                TD.Api = FD;
            }
        }

        // collect callbacks
        std::map<std::string, QualType> FPTypes;
        {
            std::set<std::string> VisitedTypes;
            for (const auto &pair : std::as_const(DeclaredThunkMap[ApiSource::Normal])) {
                auto &FD = pair.second.Api;

                llvm::SmallVector<QualType> Stack;
                Stack.push_back(FD->getReturnType());
                for (const auto &Param : FD->parameters()) {
                    Stack.push_back(Param->getType());
                }
                while (!Stack.empty()) {
                    auto Type = Stack.pop_back_val().getCanonicalType();
                    while (true) {
                        if (Type->isPointerType()) {
                            Type = Type->getPointeeType();
                            continue;
                        }
                        if (Type->isArrayType()) {
                            Type = Type->getAsArrayTypeUnsafe()->getElementType();
                            continue;
                        }
                        break;
                    }

                    Type = Type.getCanonicalType();
                    auto Name = getTypeString(Type);
                    if (VisitedTypes.count(Name)) {
                        continue;
                    }
                    VisitedTypes.insert(Name);

                    //  Function pointer
                    if (Type->isFunctionProtoType()) {
                        auto FPT = Type->getAs<FunctionProtoType>();
                        if (auto RetType = FPT->getReturnType(); !RetType->isVoidType())
                            Stack.push_back(FPT->getReturnType());
                        for (const auto &T : FPT->param_types()) {
                            Stack.push_back(T);
                        }
                        FPTypes[Name] = Type;
                        continue;
                    }
                    if (Type->isFunctionNoProtoType()) {
                        auto FPT = Type->getAs<FunctionNoProtoType>();
                        Stack.push_back(FPT->getReturnType());
                        FPTypes[Name] = Type;
                        continue;
                    }
                    if (Type->isRecordType()) {
                        auto RecType = Type->getAs<RecordType>();
                        for (const auto &T : RecType->getDecl()->fields()) {
                            Stack.push_back(T->getType());
                        }
                        continue;
                    }
                }
            }
        }

        // collect hints
        for (const auto &FD : std::as_const(FunctionDecls)) {
            const auto &FuncName = FD->getName();

            // hint
            if (FuncName.starts_with("__HINT_")) {
                auto RealFuncName = FuncName.substr(7);
                auto TypeName = getTypeString(FD->getType().getCanonicalType());
                if (RealFuncName.starts_with("GCB_")) {
                    RealFuncName = RealFuncName.substr(4);
                    if (FPTypes.count(TypeName))
                        DeclaredThunkMap[ApiSource::GuestCallback][RealFuncName.str()].Hint = FD;
                } else if (RealFuncName.starts_with("HCB_")) {
                    RealFuncName = RealFuncName.substr(4);
                    if (FPTypes.count(TypeName))
                        DeclaredThunkMap[ApiSource::HostCallback][RealFuncName.str()].Hint = FD;
                } else {
                    auto it = DeclaredThunkMap[ApiSource::Normal].find(RealFuncName.str());
                    if (it != DeclaredThunkMap[ApiSource::Normal].end()) {
                        it->second.Hint = FD;
                    }
                }
            }
        }

        for (const auto &FD : std::as_const(FunctionDecls)) {
            const auto &FuncName = FD->getName();

            // thunks
            StringRef ThunkName;
            FunctionSource::Type ThunkType;
            if (FuncName.starts_with("__GTP_")) {
                ThunkName = FuncName.substr(6);
                ThunkType = FunctionSource::GTP;
            } else if (FuncName.starts_with("___GTP_")) {
                ThunkName = FuncName.substr(7);
                ThunkType = FunctionSource::GTP_IMPL;
            } else if (FuncName.starts_with("__HTP_")) {
                ThunkName = FuncName.substr(6);
                ThunkType = FunctionSource::HTP;
            } else if (FuncName.starts_with("___HTP_")) {
                ThunkName = FuncName.substr(7);
                ThunkType = FunctionSource::HTP_IMPL;
            }

            if (!ThunkName.empty()) {
                if (ThunkName.starts_with("GCB_")) {
                    ThunkName = ThunkName.substr(4);
                    auto it = DeclaredThunkMap[ApiSource::GuestCallback].find(ThunkName.str());
                    if (it != DeclaredThunkMap[ApiSource::GuestCallback].end()) {
                        it->second.Thunks[ThunkType] = FD;
                    }
                } else if (ThunkName.starts_with("HCB_")) {
                    ThunkName = ThunkName.substr(4);
                    auto it = DeclaredThunkMap[ApiSource::HostCallback].find(ThunkName.str());
                    if (it != DeclaredThunkMap[ApiSource::HostCallback].end()) {
                        it->second.Thunks[ThunkType] = FD;
                    }
                } else {
                    auto it = DeclaredThunkMap[ApiSource::Normal].find(ThunkName.str());
                    if (it != DeclaredThunkMap[ApiSource::Normal].end()) {
                        it->second.Thunks[ThunkType] = FD;
                    }
                }
            }
        }

        // NOTE: we now only care about Normal and GuestCallback apis
        {
            // api
            auto &map = Apis[ApiSource::Normal].SourceMap;
            for (const auto &pair : std::as_const(DeclaredThunkMap[ApiSource::Normal])) {
                auto &TD = pair.second;
                const auto &FD = TD.Api;
                const auto &FuncName = FD->getName().str();
                map[FuncName] = std::make_shared<ApiSource>( //
                    ApiSource::Normal,                       //
                    _decl,                                   //
                    FuncName,                                //
                    FD->getType().getCanonicalType(),        //
                    TD.Thunks,                               //
                    FD,                                      //
                    TD.Hint                                  //
                );
            }
        }

        std::set<std::string> DeclaredGuestCallbackName;
        {
            // guest callbacks with hint
            auto &map = Apis[ApiSource::GuestCallback].SourceMap;
            for (const auto &pair : std::as_const(DeclaredThunkMap[ApiSource::GuestCallback])) {
                auto &TD = pair.second;
                const auto &FD = TD.Hint;
                const auto &FuncName = FD->getName().str();
                map[FuncName] = std::make_shared<ApiSource>( //
                    ApiSource::GuestCallback,                //
                    _decl,                                   //
                    FuncName,                                //
                    FD->getType().getCanonicalType(),        //
                    TD.Thunks,                               //
                    nullptr,                                 //
                    TD.Hint                                  //
                );
                DeclaredGuestCallbackName.insert(getTypeString(FD->getType().getCanonicalType()));
            }
        }
        {
            // undeclared guest callbacks
            int i = 0;
            auto &map = Apis[ApiSource::GuestCallback].SourceMap;
            for (const auto &pair : std::as_const(FPTypes)) {
                i++;
                auto &FPName = pair.first;
                auto &QT = pair.second;
                if (DeclaredGuestCallbackName.count(FPName)) {
                    continue;
                }
                std::string Name = "unknown_callback_" + std::to_string(i);
                map[Name] = std::make_shared<ApiSource>( //
                    ApiSource::GuestCallback,            //
                    _decl,                               //
                    Name,                                //
                    QT,                                  //
                    ArrayRef<const FunctionDecl *>(),    //
                    nullptr,                             //
                    nullptr                              //
                );
            }
        }
    }

    void PassContext::Impl::RunPass() {
        auto &decl = *_decl;
        auto &PassMap = TLC::Pass::globalPassMap();
        for (int AT = ApiSource::Normal; AT < ApiSource::HostCallback + 1; AT++) {
            for (auto &pair : Apis[AT].SourceMap) {
                const auto &ApiName = pair.first;
                auto &ApiSrc = *pair.second;

                using PassArgPair = std::pair<TLC::Pass *, SmallVector<std::string, 4>>;
                SmallVector<PassArgPair> PassList;

                // add standard pass
                if (auto it = PassMap.find("std"); it == PassMap.end()) {
                    llvm::errs() << llvm::format("error: standard pass not found\n");
                    std::exit(-1);
                } else {
                    PassList.push_back({it->second, {}});
                }

                // add extra pass
                if (int HintCount = ApiSrc.getHintAnnotationCount(); HintCount == 0) {
                    // auto select pass
                    SmallVector<PassArgPair> TestedNoPriorityPassList;
                    std::map<int, PassArgPair> TestedPriorityPassMap;
                    for (auto &pair : PassMap) {
                        auto P = pair.second;
                        SmallVector<std::string, 4> Args;
                        int Priority = TLC::Pass::NoPriority;
                        if (!P->test(ApiSrc, &Args, &Priority)) {
                            continue;
                        }

                        if (Priority == TLC::Pass::NoPriority) {
                            TestedNoPriorityPassList.push_back({P, Args});
                            continue;
                        }

                        if (auto Result = TestedPriorityPassMap.insert(std::make_pair(Priority, PassArgPair{P, Args}));
                            !Result.second) {
                            llvm::errs() << llvm::format(
                                "warning: in API \"%s\", priority conflict between pass @%s and pass @%s "
                                "during pass auto-selection\n",
                                ApiName.c_str(), P->identifier().data(),
                                Result.first->second.first->identifier().data());
                            continue;
                        }
                    }

                    for (auto &item : TestedNoPriorityPassList) {
                        PassList.push_back(item);
                    }
                    for (auto &pair : TestedPriorityPassMap) {
                        PassList.push_back(pair.second);
                    }
                } else {
                    // annotated pass
                    for (int i = 0; i < HintCount; ++i) {
                        const auto &Hint = ApiSrc.getHintAnnotation(i);
                        auto it = PassMap.find(Hint.Name.str());
                        if (it == PassMap.end()) {
                            llvm::errs() << llvm::format("error: in API \"%s\", unknown pass @%s\n", ApiName.c_str(),
                                                         Hint.Name.data());
                            std::exit(-1);
                        }
                        PassList.push_back({it->second, SmallVector<std::string, 4>{Hint.Arguments}});
                    }
                }

                // run pass
                for (const auto &pair : PassList) {
                    auto P = pair.first;
                    if (!P->start(ApiSrc, pair.second)) {
                        llvm::errs() << llvm::format("error: in API \"%s\", failed to start pass @%s\n",
                                                     ApiName.c_str(), P->identifier().data());
                        std::exit(-1);
                    }
                }
                for (auto it = PassList.rbegin(); it != PassList.rend(); ++it) {
                    auto P = it->first;
                    if (!P->end()) {
                        llvm::errs() << llvm::format("error: in API \"%s\", failed to end pass @%s\n", ApiName.c_str(),
                                                     P->identifier().data());
                        std::exit(-1);
                    }
                }
            }
        }
    }

    void PassContext::Impl::End() {
        // Implement the rest of the ApiSources
        for (const auto &pair : Apis[ApiSource::Normal].SourceMap) {
            auto &Source = *pair.second;
            auto ApiReturnType = Source.getApiReturnType();
            auto ApiArgTypes = Source.getApiArgumentTypes();
            auto ApiVariadic = Source.getApiVariadic();

            bool isVoid = ApiReturnType->isVoidType();

            auto &GTP = Source.getFunctionSource(FunctionSource::GTP);
            auto &GTP_IMPL = Source.getFunctionSource(FunctionSource::GTP_IMPL);
            auto &HTP = Source.getFunctionSource(FunctionSource::HTP);
            auto &HTP_IMPL = Source.getFunctionSource(FunctionSource::HTP_IMPL);

            // GTP
            {
                if (GTP.getProlog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                        GTP.setProlog(ss.str());
                    }
                }

                if (GTP.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    ";
                    if (!isVoid) {
                        ss << "ret = ";
                    }
                    ss << GTP_IMPL.getName().str() << "(" + getCallArgsString(ApiArgTypes.size()) << ");\n";
                    GTP.setBody(ss.str());
                }

                if (GTP.getEpilog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    return ret;\n";
                        GTP.setEpilog(ss.str());
                    }
                }
            }

            // GTP_IMPL
            if (!GTP_IMPL.getFunctionDecl()) {
                // Fill in missing signature information
                if (GTP_IMPL.getReturnValue().TypeName.empty()) {
                    GTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                    SmallVector<FunctionSource::Param> Args;
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                    }
                    GTP_IMPL.setArguments(Args);

                    if (ApiVariadic) {
                        GTP_IMPL.setVariadic(true);
                    }
                }

                if (GTP_IMPL.getProlog().empty()) {
                    std::stringstream ss;

                    if (!isVoid) {
                        ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                    }
                    ss << "    void *args[] = {\n";
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        ss << "        (void *) &arg" << i + 1 << ",\n";
                    }
                    ss << "    };\n";

                    GTP_IMPL.setProlog(ss.str());
                }

                if (GTP_IMPL.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    LORELIB_INVOKE_HTP(LORELIB_HTP(" << Source.getName().str() << "), args, "
                       << (isVoid ? "NULL" : "&ret") << ", NULL);\n";
                    GTP_IMPL.setBody(ss.str());
                }

                if (GTP_IMPL.getEpilog().empty()) {
                    std::stringstream ss;
                    if (!isVoid) {
                        ss << "    return ret;\n";
                    }
                    GTP_IMPL.setEpilog(ss.str());
                }
            }

            // HTP
            if (!HTP.getFunctionDecl()) {
                if (HTP.getProlog().empty()) {
                    std::stringstream ss;
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        ss << "    __auto_type arg" << i + 1 //
                           << " = *(__typeof__(" << getTypeString(ApiArgTypes[i]) << ") *) args[" << i << "];\n";
                    }
                    if (!isVoid) {
                        ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(ApiReturnType) << ") *) ret;\n";
                    }
                    HTP.setProlog(ss.str());
                }

                if (HTP.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    ";
                    if (!isVoid) {
                        ss << "*ret_ref = ";
                    }
                    ss << HTP_IMPL.getName().str() << "(" << getCallArgsString(ApiArgTypes.size()) << ");\n";
                    HTP.setBody(ss.str());
                }
            }

            // HTP_IMPL
            if (!HTP_IMPL.getFunctionDecl()) {
                // Fill in missing signature information
                if (HTP_IMPL.getReturnValue().TypeName.empty()) {
                    HTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                    SmallVector<FunctionSource::Param> Args;
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                    }
                    HTP_IMPL.setArguments(Args);

                    if (ApiVariadic) {
                        HTP_IMPL.setVariadic(true);
                    }
                }

                if (HTP_IMPL.getProlog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                        HTP_IMPL.setProlog(ss.str());
                    }
                }

                if (HTP_IMPL.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    ";
                    if (!isVoid) {
                        ss << "ret = ";
                    }
                    ss << "LORELIB_API(" << Source.getName().str() << ")(" << getCallArgsString(ApiArgTypes.size())
                       << ");\n";
                    HTP_IMPL.setBody(ss.str());
                }

                if (HTP_IMPL.getEpilog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    return ret;\n";
                        HTP_IMPL.setEpilog(ss.str());
                    }
                }
            }
        }

        for (const auto &pair : Apis[ApiSource::GuestCallback].SourceMap) {
            auto &Source = *pair.second;
            auto ApiReturnType = Source.getApiReturnType();
            auto ApiArgTypes = Source.getApiArgumentTypes();
            auto ApiVariadic = Source.getApiVariadic();

            bool isVoid = ApiReturnType->isVoidType();

            auto &GTP = Source.getFunctionSource(FunctionSource::GTP);
            auto &GTP_IMPL = Source.getFunctionSource(FunctionSource::GTP_IMPL);
            auto &HTP = Source.getFunctionSource(FunctionSource::HTP);
            auto &HTP_IMPL = Source.getFunctionSource(FunctionSource::HTP_IMPL);

            // GTP
            if (!GTP.getFunctionDecl()) {
                if (GTP.getProlog().empty()) {
                    std::stringstream ss;
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        ss << "    __auto_type arg" << i + 1 << //
                            " = *(__typeof__(" << getTypeString(ApiArgTypes[i]) << ") *) args[" << i << "];\n";
                    }
                    if (!isVoid) {
                        ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(ApiReturnType) << ") *) ret;\n";
                    }
                    GTP.setProlog(ss.str());
                }

                if (GTP.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    ";
                    if (!isVoid) {
                        ss << "*ret_ref = ";
                    }
                    ss << GTP_IMPL.getName().str() << "(callback" << getCallArgsString(ApiArgTypes.size(), true)
                       << ");\n";

                    GTP.setBody(ss.str());
                }
            }

            // GTP_IMPL
            if (!GTP_IMPL.getFunctionDecl()) {
                // Fill in missing signature information
                if (GTP_IMPL.getReturnValue().TypeName.empty()) {
                    GTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                    SmallVector<FunctionSource::Param> Args;
                    Args.push_back({"void *", "callback", {}});
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        Args.push_back({getTypeString(ApiArgTypes[i]), "arg" + std::to_string(i + 1), {}});
                    }
                    GTP_IMPL.setArguments(Args);

                    if (ApiVariadic) {
                        GTP_IMPL.setVariadic(true);
                    }
                }

                if (GTP_IMPL.getProlog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                        GTP_IMPL.setProlog(ss.str());
                    }
                }

                if (GTP_IMPL.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    ";
                    if (!isVoid) {
                        ss << "ret = ";
                    }
                    ss << "((__typeof__(" << getTypeString(Source.getType()) << ") *) callback)("
                       << getCallArgsString(ApiArgTypes.size()) << ");\n";
                    GTP_IMPL.setBody(ss.str());
                }

                if (GTP_IMPL.getEpilog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    return ret;\n";
                        GTP_IMPL.setEpilog(ss.str());
                    }
                }
            }

            // HTP
            if (!HTP.getFunctionDecl()) {
                // Fill in missing signature information
                if (HTP.getReturnValue().TypeName.empty()) {
                    HTP.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                    SmallVector<FunctionSource::Param> Args;
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                    }
                    HTP.setArguments(Args);

                    if (ApiVariadic) {
                        HTP.setVariadic(true);
                    }
                }

                if (HTP.getProlog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                        HTP.setProlog(ss.str());
                    }
                }

                if (HTP.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    ";
                    if (!isVoid) {
                        ss << "ret = ";
                    }
                    ss << HTP_IMPL.getName().str() << "(" << getCallArgsString(ApiArgTypes.size()) << ");\n";

                    HTP.setBody(ss.str());
                }

                if (HTP.getEpilog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    return ret;\n";
                        HTP.setEpilog(ss.str());
                    }
                }
            }

            // HTP_IMPL
            if (!HTP_IMPL.getFunctionDecl()) {
                // Fill in missing signature information
                if (HTP_IMPL.getReturnValue().TypeName.empty()) {
                    HTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                    SmallVector<FunctionSource::Param> Args;
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                    }
                    HTP_IMPL.setArguments(Args);

                    if (ApiVariadic) {
                        HTP_IMPL.setVariadic(true);
                    }
                }

                if (HTP_IMPL.getProlog().empty()) {
                    std::stringstream ss;

                    if (!isVoid) {
                        ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                    }
                    ss << "    void *args[] = {\n";
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        ss << "        (void *) &arg" << i + 1 << ",\n";
                    }
                    ss << "    };\n";

                    HTP_IMPL.setProlog(ss.str());
                }

                if (HTP_IMPL.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    LORELIB_INVOKE_GCB(LORELIB_GCB(" << Source.getName().str()
                       << "), LORELIB_LAST_GCB, args, " << (isVoid ? "NULL" : "&ret") << ", NULL);\n";
                    HTP_IMPL.setBody(ss.str());
                }

                if (HTP_IMPL.getEpilog().empty()) {
                    std::stringstream ss;
                    if (!isVoid) {
                        ss << "    return ret;\n";
                    }
                    HTP_IMPL.setEpilog(ss.str());
                }
            }
        }

        for (const auto &pair : Apis[ApiSource::HostCallback].SourceMap) {
            auto &Source = *pair.second;
            auto ApiReturnType = Source.getApiReturnType();
            auto ApiArgTypes = Source.getApiArgumentTypes();
            auto ApiVariadic = Source.getApiVariadic();

            bool isVoid = ApiReturnType->isVoidType();

            auto &GTP = Source.getFunctionSource(FunctionSource::GTP);
            auto &GTP_IMPL = Source.getFunctionSource(FunctionSource::GTP_IMPL);
            auto &HTP = Source.getFunctionSource(FunctionSource::HTP);
            auto &HTP_IMPL = Source.getFunctionSource(FunctionSource::HTP_IMPL);

            // GTP
            if (!GTP.getFunctionDecl()) {
                // Fill in missing signature information
                if (GTP.getReturnValue().TypeName.empty()) {
                    GTP.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                    SmallVector<FunctionSource::Param> Args;
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                    }
                    GTP.setArguments(Args);

                    if (ApiVariadic) {
                        GTP.setVariadic(true);
                    }
                }

                if (GTP.getProlog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                        GTP.setProlog(ss.str());
                    }
                }

                if (GTP.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    ";
                    if (!isVoid) {
                        ss << "ret = ";
                    }
                    ss << GTP_IMPL.getName().str() << "(" << getCallArgsString(ApiArgTypes.size()) << ");\n";

                    GTP.setBody(ss.str());
                }

                if (GTP.getEpilog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    return ret;\n";
                        GTP.setEpilog(ss.str());
                    }
                }
            }

            // GTP_IMPL
            if (!GTP_IMPL.getFunctionDecl()) {
                // Fill in missing signature information
                if (GTP_IMPL.getReturnValue().TypeName.empty()) {
                    GTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                    SmallVector<FunctionSource::Param> Args;
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        Args.push_back({getTypeString(ApiArgTypes[i]), {}, {}});
                    }
                    GTP_IMPL.setArguments(Args);

                    if (ApiVariadic) {
                        GTP_IMPL.setVariadic(true);
                    }
                }

                if (GTP_IMPL.getProlog().empty()) {
                    std::stringstream ss;

                    if (!isVoid) {
                        ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                    }
                    ss << "    void *args[] = {\n";
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        ss << "        (void *) &arg" << i + 1 << ",\n";
                    }
                    ss << "    };\n";

                    GTP_IMPL.setProlog(ss.str());
                }

                if (GTP_IMPL.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    LORELIB_INVOKE_HCB(LORELIB_HCB(" << Source.getName().str()
                       << "), LORELIB_LAST_HCB, args, " << (isVoid ? "NULL" : "&ret") << ", NULL);\n";
                    GTP_IMPL.setBody(ss.str());
                }

                if (GTP_IMPL.getEpilog().empty()) {
                    std::stringstream ss;
                    if (!isVoid) {
                        ss << "    return ret;\n";
                    }
                    GTP_IMPL.setEpilog(ss.str());
                }
            }

            // HTP
            if (!HTP.getFunctionDecl()) {
                if (HTP.getProlog().empty()) {
                    std::stringstream ss;
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        ss << "    __auto_type arg" << i + 1 << //
                            " = *(__typeof__(" << getTypeString(ApiArgTypes[i]) << ") *) args[" << i << "];\n";
                    }
                    if (!isVoid) {
                        ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(ApiReturnType) << ") *) ret;\n";
                    }
                    HTP.setProlog(ss.str());
                }

                if (HTP.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    ";
                    if (!isVoid) {
                        ss << "*ret_ref = ";
                    }
                    ss << HTP_IMPL.getName().str() << "(callback" << getCallArgsString(ApiArgTypes.size(), true)
                       << ");\n";

                    HTP.setBody(ss.str());
                }
            }

            // HTP_IMPL
            if (!HTP_IMPL.getFunctionDecl()) {
                // Fill in missing signature information
                if (HTP_IMPL.getReturnValue().TypeName.empty()) {
                    HTP_IMPL.setReturnValue({getTypeString(ApiReturnType), {}, {}});

                    SmallVector<FunctionSource::Param> Args;
                    Args.push_back({"void *", "callback", {}});
                    for (int i = 0; i < ApiArgTypes.size(); ++i) {
                        Args.push_back({getTypeString(ApiArgTypes[i]), "arg" + std::to_string(i + 1), {}});
                    }
                    HTP_IMPL.setArguments(Args);

                    if (ApiVariadic) {
                        HTP_IMPL.setVariadic(true);
                    }
                }

                if (HTP_IMPL.getProlog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    __typeof__(" << getTypeString(ApiReturnType) << ") ret;\n";
                        HTP_IMPL.setProlog(ss.str());
                    }
                }

                if (HTP_IMPL.getBody().empty()) {
                    std::stringstream ss;
                    ss << "    ";
                    if (!isVoid) {
                        ss << "ret = ";
                    }
                    ss << "((__typeof__(" << getTypeString(Source.getType()) << ") *) callback)("
                       << getCallArgsString(ApiArgTypes.size()) << ");\n";

                    HTP_IMPL.setBody(ss.str());
                }

                if (HTP_IMPL.getEpilog().empty()) {
                    if (!isVoid) {
                        std::stringstream ss;
                        ss << "    return ret;\n";
                        HTP_IMPL.setEpilog(ss.str());
                    }
                }
            }
        }

        // Generate defs
        {
            std::stringstream ss;

            char *warning;

            auto res = asprintf(&warning, GENERATED_WARNING, InputFileName.c_str());
            assert(res > 0);

            ss << warning;
            free(warning);

            ss << "#pragma once\n\n";

            ss << "//\n// Declare Apis\n//\n";
            ss << "#define LORELIB_API_FOREACH(F)";
            for (const auto &Item : Apis[ApiSource::Normal].SourceMap) {
                ss << " \\\n    F(" << Item.first << ")";
            }
            ss << "\n\n";

            ss << "//\n// Declare Guest Callbacks\n//\n";
            ss << "#define LORELIB_GCB_FOREACH(F)";
            for (const auto &Item : Apis[ApiSource::GuestCallback].SourceMap) {
                ss << " \\\n    F(" << Item.first << ", \"" << getTypeString(Item.second->getType()) << "\")";
            }
            ss << "\n\n";

            ss << "//\n// Declare Host Callbacks\n//\n";
            ss << "#define LORELIB_HCB_FOREACH(F)";
            for (const auto &Item : Apis[ApiSource::HostCallback].SourceMap) {
                ss << " \\\n    F(" << Item.first << ", \"" << getTypeString(Item.second->getType()) << "\")";
            }
            ss << "\n\n";

            ResultDefs = ss.str();
        }

        // Generate impl
        {
            std::stringstream ss;

            char *warning;

            auto res = asprintf(&warning, GENERATED_WARNING, InputFileName.c_str());
            assert(res > 0);

            ss << warning;
            free(warning);

            ss << "#if !defined(LORELIB_VISUAL) && !defined(LORELIB_BUILD)\n";
            ss << "#  include \"" << InputFileName << "\"\n";
            ss << "#endif\n";
            ss << "\n";
            ss << "#include <lorelib_common/callback.h>\n";
            ss << "\n";

            ss << "//\n// Generated(Guest)\n//\n";
            ss << "#if defined(LORELIB_VISUAL) || defined(LORELIB_GTL_BUILD)\n";
            for (const auto &pair : Apis[ApiSource::Normal].SourceMap) {
                auto &Source = *pair.second;
                ss << Source.getText(true) << "\n";
            }
            for (const auto &pair : Apis[ApiSource::GuestCallback].SourceMap) {
                auto &Source = *pair.second;
                ss << Source.getText(true) << "\n";
            }
            for (const auto &pair : Apis[ApiSource::HostCallback].SourceMap) {
                auto &Source = *pair.second;
                ss << Source.getText(true) << "\n";
            }
            for (const auto &pair : Apis[ApiSource::Normal].SourceMap) {
                auto &Source = *pair.second;
                auto &Function = Source.getFunctionSource(FunctionSource::GTP);
                ss << "LORELIB_EXPORT " << Function.getFunctionSignature(Source.getName()) //
                   << " __attribute__((alias(\"" << Function.getName().str() << "\")));\n";
            }
            ss << "#endif\n";
            ss << "\n";


            ss << "//\n// Generated(Host)\n//\n";
            ss << "#if defined(LORELIB_VISUAL) || defined(LORELIB_HTL_BUILD)\n";
            for (const auto &pair : Apis[ApiSource::Normal].SourceMap) {
                auto &Source = *pair.second;
                ss << Source.getText(false) << "\n";
            }
            for (const auto &pair : Apis[ApiSource::GuestCallback].SourceMap) {
                auto &Source = *pair.second;
                ss << Source.getText(false) << "\n";
            }
            for (const auto &pair : Apis[ApiSource::HostCallback].SourceMap) {
                auto &Source = *pair.second;
                ss << Source.getText(false) << "\n";
            }
            for (const auto &pair : Apis[ApiSource::Normal].SourceMap) {
                auto &Source = *pair.second;
                auto &Function = Source.getFunctionSource(FunctionSource::HTP);
                ss << "LORELIB_EXPORT " << Function.getFunctionSignature("_HTP_" + Source.getName().str()) //
                   << " __attribute__((alias(\"" << Function.getName().str() << "\")));\n";
            }
            ss << "#endif\n";
            ss << "\n";

            // Generate missing function declarations
            ss << "//\n// Missing Function Declarations\n//\n";
            for (const auto &Item : std::as_const(ApiNames)) {
                if (!Apis[ApiSource::Normal].SourceMap.count(Item)) {
                    ss << "// " << Item << "\n";
                }
            }
            ResultImpl = ss.str();
        }
    }

    clang::SourceManager &PassContext::getSourceManager() const {
        return *_impl->SourceMgr;
    }

    const clang::LangOptions &PassContext::getLangOpts() const {
        return _impl->LangOpts;
    }

    clang::ASTContext &PassContext::getASTContext() const {
        return *_impl->Context;
    }

    std::set<std::string> PassContext::getApiNameSet(ApiSource::Type type) const {
        std::set<std::string> Result;
        for (const auto &pair : _impl->Apis[type].SourceMap) {
            Result.insert(pair.first);
        }
        return Result;
    }

    ApiSource *PassContext::getApiSource(ApiSource::Type type, const std::string &name) {
        auto &Map = _impl->Apis[type].SourceMap;
        auto it = Map.find(name);
        if (it == Map.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    PassContext::ResultText PassContext::Run() {
        auto &impl = *_impl;
        impl.Begin();
        impl.RunPass();
        impl.End();
        return {impl.ResultDefs, impl.ResultImpl};
    }

}