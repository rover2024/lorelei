#include "analyzer.h"

#include <filesystem>

#include "common.h"
#include "pass.h"

using namespace clang;
using namespace llvm;

namespace fs = std::filesystem;

namespace TLC {

    struct FuncThunkData {
        QualType type;
        const FunctionDecl *decl = nullptr;
        const FunctionDecl *hintDecl = nullptr;
        std::array<const FunctionDecl *, FunctionDefinition::NumTypes> thunks = {};

        std::array<FunctionDeclRep, FunctionDefinition::NumTypes>
            toFunctionDeclReps(std::string name, ThunkDefinition::Type thunkType) const {
            std::array<FunctionDeclRep, FunctionDefinition::NumTypes> res;

            /// \brief Guest function thunks
            /// \example: int foo(int a, double b);
            /// \code
            ///     void  __GTP_foo(int a, double b);                           // GTP      (1)
            ///     void ___GTP_foo(int a, double b);                           // GTP_IMPL (2)
            ///
            ///     void  __HTP_foo(void **args, void **ret, void **metadata);  // HTP      (3)
            ///     void ___HTP_foo(int a, double b);                           // HTP_IMPL (4)
            /// \endcode

            /// \brief Guest callback thunks
            /// \example: int (*foo)(int a, double b);
            /// \code
            ///     void  __GTP_GCB_foo(void *callback, void **args,
            ///                         void **ret,     void **metadata);       // GTP      (3)
            ///     void ___GTP_GCB_foo(void *callback, int a, double b);       // GTP_IMPL (4)
            ///
            ///     void  __HTP_GCB_foo(int a, double b);                       // HTP      (1)
            ///     void ___HTP_GCB_foo(int a, double b);                       // HTP_IMPL (2)
            /// \endcode

            /// \brief Host callback thunks
            /// \example: void (*foo)(int a, double b);
            /// \code
            ///     void  __GTP_HCB_foo(int a, double b);                       // GTP      (1)
            ///     void ___GTP_HCB_foo(int a, double b);                       // GTP_IMPL (2)
            ///
            ///     void  __HTP_HCB_foo(void *callback, void **args,
            ///                         void **ret,     void **metadata);       // HTP      (3)
            ///     void ___HTP_HCB_foo(void *callback, int a, double b);       // HTP_IMPL (4)
            /// \endcode

            if (auto &target = res[FunctionDefinition::GTP]; thunks[FunctionDefinition::GTP]) {
                target = thunks[FunctionDefinition::GTP];
            } else {
                switch (thunkType) {
                    case ThunkDefinition::GuestFunctionThunk: {
                        target = FunctionDeclRep::fromQualType(type, "__GTP_" + name).get();
                        break;
                    }
                    case ThunkDefinition::GuestCallbackThunk: {
                        target = FunctionDeclRep(FunctionTypeRep("void",
                                                                 {
                                                                     "void *", /*callback*/
                                                                     "void **" /*args*/,
                                                                     "void *" /*ret*/,
                                                                     "void **" /*metadata*/,
                                                                 },
                                                                 false),
                                                 "__GTP_GCB_" + name,
                                                 {
                                                     "callback",
                                                     "args",
                                                     "ret",
                                                     "metadata",
                                                 });
                        break;
                    }
                    case ThunkDefinition::HostCallbackThunk: {
                        target = FunctionDeclRep::fromQualType(type, "__GTP_HCB_" + name).get();
                        break;
                    }
                    default:
                        break;
                }
            }

            if (auto &target = res[FunctionDefinition::GTP_IMPL];
                thunks[FunctionDefinition::GTP_IMPL]) {
                target = thunks[FunctionDefinition::GTP_IMPL];
            } else {
                switch (thunkType) {
                    case ThunkDefinition::GuestFunctionThunk: {
                        target = FunctionDeclRep::fromQualType(type, "___GTP_" + name).get();
                        break;
                    }
                    case ThunkDefinition::GuestCallbackThunk: {
                        auto FTV = FunctionTypeView(type);
                        SmallVector<TypeRep> argTypes;
                        SmallVector<std::string> argNames;
                        argTypes.push_back("void *");
                        argNames.push_back("callback");
                        int i = 0;
                        for (const auto &T : FTV.argTypes()) {
                            argTypes.push_back(TypeRep(T));
                            argNames.push_back("arg" + std::to_string(++i));
                        }
                        target =
                            FunctionDeclRep(FunctionTypeRep(FTV.returnType(), std::move(argTypes),
                                                            FTV.isVariadic()),
                                            "___GTP_GCB_" + name, argNames);
                        break;
                    }
                    case ThunkDefinition::HostCallbackThunk: {
                        target = FunctionDeclRep::fromQualType(type, "___GTP_HCB_" + name).get();
                        break;
                    }
                    default:
                        break;
                }
            }

            if (auto &target = res[FunctionDefinition::HTP]; thunks[FunctionDefinition::HTP]) {
                target = thunks[FunctionDefinition::HTP];
            } else {
                switch (thunkType) {
                    case ThunkDefinition::GuestFunctionThunk: {
                        target = FunctionDeclRep(FunctionTypeRep("void",
                                                                 {
                                                                     "void **" /*args*/,
                                                                     "void *" /*ret*/,
                                                                     "void **" /*metadata*/,
                                                                 },
                                                                 false),
                                                 "__HTP_" + name,
                                                 {
                                                     "args",
                                                     "ret",
                                                     "metadata",
                                                 });
                        break;
                    }
                    case ThunkDefinition::GuestCallbackThunk: {
                        target = FunctionDeclRep::fromQualType(type, "__HTP_GCB_" + name).get();
                        break;
                    }
                    case ThunkDefinition::HostCallbackThunk: {
                        target = FunctionDeclRep(FunctionTypeRep("void",
                                                                 {
                                                                     "void *" /*callback*/,
                                                                     "void **" /*args*/,
                                                                     "void *" /*ret*/,
                                                                     "void **" /*metadata*/,
                                                                 },
                                                                 false),
                                                 "__HTP_HCB_" + name,
                                                 {
                                                     "callback",
                                                     "args",
                                                     "ret",
                                                     "metadata",
                                                 });
                        break;
                    }
                    default:
                        break;
                }
            }

            if (auto &target = res[FunctionDefinition::HTP_IMPL];
                thunks[FunctionDefinition::HTP_IMPL]) {
                target = thunks[FunctionDefinition::HTP_IMPL];
            } else {
                switch (thunkType) {
                    case ThunkDefinition::GuestFunctionThunk: {
                        target = FunctionDeclRep::fromQualType(type, "___HTP_" + name).get();
                        break;
                    }
                    case ThunkDefinition::GuestCallbackThunk: {
                        target = FunctionDeclRep::fromQualType(type, "___HTP_GCB_" + name).get();
                        break;
                    }
                    case ThunkDefinition::HostCallbackThunk: {
                        auto FTV = FunctionTypeView(type);
                        SmallVector<TypeRep> argTypes;
                        SmallVector<std::string> argNames;
                        argTypes.push_back("void *");
                        argNames.push_back("callback");
                        int i = 0;
                        for (const auto &T : FTV.argTypes()) {
                            argTypes.push_back(TypeRep(T));
                            argNames.push_back("arg" + std::to_string(++i));
                        }
                        target =
                            FunctionDeclRep(FunctionTypeRep(FTV.returnType(), std::move(argTypes),
                                                            FTV.isVariadic()),
                                            "___HTP_HCB_" + name, argNames);
                        break;
                    }
                    default:
                        break;
                }
            }
            return res;
        }
    };

    static std::array<FunctionDeclRep, FunctionDefinition::NumTypes>
        buildGuestCallbackReps(const QualType &type, const std::string &name) {
        FuncThunkData data;
        data.type = type;
        return data.toFunctionDeclReps(name, ThunkDefinition::GuestCallbackThunk);
    }

    static std::array<FunctionDeclRep, FunctionDefinition::NumTypes>
        buildHostCallbackReps(const QualType &type, const std::string &name) {
        FuncThunkData data;
        data.type = type;
        return data.toFunctionDeclReps(name, ThunkDefinition::HostCallbackThunk);
    }

    struct VarThunkData {
        const VarDecl *Var = nullptr;
    };

    Analyzer::Analyzer(CompilerInstance &CI, ArrayRef<const FunctionDecl *> FDs,
                       ArrayRef<const VarDecl *> VDs, const ConfigFile &cfg)
        : _ci(CI), _fds(FDs.begin(), FDs.end()), _vds(VDs.begin(), VDs.end()), _config(cfg) {
        auto symbols = cfg.symbols();

        /// STEP: Collect function declarations
        // array[ thunk type, map{ function name -> ThunkData } ]
        std::array<std::map<std::string, FuncThunkData>, ThunkDefinition::NumTypes>
            declaredFuncMaps;
        for (const auto &FD : std::as_const(_fds)) {
            if (!(FD->getLinkageInternal() == Linkage::External &&
                  FD->getStorageClass() != SC_Static)) {
                continue;
            }

            const auto &name = FD->getName().str();
            auto it = symbols.find(name);
            if (it != symbols.end()) {
                auto &target = declaredFuncMaps[ThunkDefinition::GuestFunctionThunk][name];
                target.type = FD->getType();
                target.decl = FD;
                symbols.erase(it);
            }
        }

        /// STEP: Collect variable declarations
        // map{ variable name -> ThunkData }
        std::map<std::string, VarThunkData> declaredVarMap;
        for (const auto &VD : std::as_const(_vds)) {
            const auto &name = VD->getName().str();

            auto it = symbols.find(name);
            if (it != symbols.end()) {
                declaredVarMap[name].Var = VD;

                // Add function pointer variables to GuestFunctionThunk map
                if (auto vType = VD->getType().getCanonicalType(); isFunctionPointerType(vType)) {
                    // Prepend "__FPVAR_" to the variable name to create a new function name
                    auto &target =
                        declaredFuncMaps[ThunkDefinition::GuestFunctionThunk]["__FPVAR_" + name];
                    target.type = vType->getPointeeType();
                }
                symbols.erase(it);
            }
        }

        /// STEP: Collect callbacks in function arguments and return types
        std::map<std::string, QualType> fpTypes;
        {
            std::set<std::string> visitedTypes;
            for (const auto &pair :
                 std::as_const(declaredFuncMaps[ThunkDefinition::GuestFunctionThunk])) {
                FunctionTypeView rep(pair.second.type);

                SmallVector<QualType> stack;
                stack.push_back(rep.returnType());
                for (const auto &argType : rep.argTypes()) {
                    stack.push_back(argType);
                }
                while (!stack.empty()) {
                    auto type = stack.pop_back_val().getCanonicalType();
                    while (true) {
                        if (type->isPointerType()) {
                            type = type->getPointeeType();
                            continue;
                        }
                        if (type->isArrayType()) {
                            type = type->getAsArrayTypeUnsafe()->getElementType();
                            continue;
                        }
                        break;
                    }

                    type = type.getCanonicalType();
                    auto Name = getTypeString(type);
                    if (visitedTypes.count(Name)) {
                        continue;
                    }
                    visitedTypes.insert(Name);

                    //  Function pointer
                    if (type->isFunctionProtoType()) {
                        auto FPT = type->getAs<FunctionProtoType>();
                        if (auto RetType = FPT->getReturnType(); !RetType->isVoidType())
                            stack.push_back(FPT->getReturnType());
                        for (const auto &T : FPT->param_types()) {
                            stack.push_back(T);
                        }
                        fpTypes[Name] = type;
                        continue;
                    }
                    if (type->isFunctionNoProtoType()) {
                        auto FNT = type->getAs<FunctionNoProtoType>();
                        stack.push_back(FNT->getReturnType());
                        fpTypes[Name] = type;
                        continue;
                    }
                    if (type->isRecordType()) {
                        auto recType = type->getAs<RecordType>();
                        for (const auto &T : recType->getDecl()->fields()) {
                            stack.push_back(T->getType());
                        }
                        continue;
                    }
                }
            }
        }

        /// STEP: Collect hint declarations
        for (const auto &FD : std::as_const(_fds)) {
            const auto &name = FD->getName();

            // Hint function must start with "__HINT_"
            if (char prefix[] = "__HINT_"; name.starts_with(prefix)) {
                auto realName = name.substr(sizeof(prefix) - 1);
                auto typeName = getTypeString(FD->getType().getCanonicalType());
                if (char prefix[] = "GCB_"; realName.starts_with("GCB_")) {
                    // __HINT_GCB_xxx
                    realName = realName.substr(sizeof(prefix) - 1);
                    if (fpTypes.count(typeName))
                        declaredFuncMaps[ThunkDefinition::GuestCallbackThunk][realName.str()]
                            .hintDecl = FD;
                } else if (char prefix[] = "HCB_"; realName.starts_with("HCB_")) {
                    // __HINT_HCB_xxx
                    realName = realName.substr(sizeof(prefix) - 1);
                    if (fpTypes.count(typeName))
                        declaredFuncMaps[ThunkDefinition::HostCallbackThunk][realName.str()]
                            .hintDecl = FD;
                } else {
                    // __HINT_xxx
                    auto it =
                        declaredFuncMaps[ThunkDefinition::GuestFunctionThunk].find(realName.str());
                    if (it != declaredFuncMaps[ThunkDefinition::GuestFunctionThunk].end()) {
                        it->second.hintDecl = FD;
                    }
                }
            }
        }

        /// STEP: Collect existing thunk definitions
        for (const auto &FD : std::as_const(_fds)) {
            const auto &name = FD->getName();

            // thunks
            StringRef thunkName;
            FunctionDefinition::Type thunkType;
            if (char prefix[] = "__GTP_"; name.starts_with(prefix)) {
                thunkName = name.substr(sizeof(prefix) - 1);
                thunkType = FunctionDefinition::GTP;
            } else if (char prefix[] = "___GTP_"; name.starts_with(prefix)) {
                thunkName = name.substr(sizeof(prefix) - 1);
                thunkType = FunctionDefinition::GTP_IMPL;
            } else if (char prefix[] = "__HTP_"; name.starts_with(prefix)) {
                thunkName = name.substr(sizeof(prefix) - 1);
                thunkType = FunctionDefinition::HTP;
            } else if (char prefix[] = "___HTP_"; name.starts_with(prefix)) {
                thunkName = name.substr(sizeof(prefix) - 1);
                thunkType = FunctionDefinition::HTP_IMPL;
            }

            if (!thunkName.empty()) {
                if (char prefix[] = "GCB_"; thunkName.starts_with(prefix)) {
                    thunkName = thunkName.substr(sizeof(prefix) - 1);
                    auto &map = declaredFuncMaps[ThunkDefinition::GuestCallbackThunk];
                    auto it = map.find(thunkName.str());
                    if (it != map.end()) {
                        it->second.thunks[thunkType] = FD;
                    }
                } else if (char prefix[] = "HCB_"; thunkName.starts_with(prefix)) {
                    thunkName = thunkName.substr(sizeof(prefix) - 1);
                    auto &map = declaredFuncMaps[ThunkDefinition::HostCallbackThunk];
                    auto it = map.find(thunkName.str());
                    if (it != map.end()) {
                        it->second.thunks[thunkType] = FD;
                    }
                } else {
                    auto &map = declaredFuncMaps[ThunkDefinition::GuestFunctionThunk];
                    auto it = map.find(thunkName.str());
                    if (it != map.end()) {
                        it->second.thunks[thunkType] = FD;
                    }
                }
            }
        }

        /// STEP: Create \c ThunkDefinition instances
        {
            // Guest function thunks
            const auto &sourceMap = declaredFuncMaps[ThunkDefinition::GuestFunctionThunk];
            auto &targetMap = _thunks[ThunkDefinition::GuestFunctionThunk];
            for (auto &pair : sourceMap) {
                const auto &name = pair.first;
                const auto &data = pair.second;
                targetMap.insert(std::make_pair(
                    name, ThunkDefinition(
                              ThunkDefinition::GuestFunctionThunk, this, name, data.type,
                              data.toFunctionDeclReps(name, ThunkDefinition::GuestFunctionThunk),
                              data.hintDecl ? data.hintDecl : data.decl)));
            }
        }
        {
            // Guest callback thunks
            std::map<std::string, std::string> declaredTypeNames;
            const auto &sourceMap = declaredFuncMaps[ThunkDefinition::GuestCallbackThunk];
            auto &targetMap = _thunks[ThunkDefinition::GuestCallbackThunk];
            for (auto &pair : sourceMap) {
                const auto &name = pair.first;
                const auto &data = pair.second;
                targetMap.insert(std::make_pair(
                    name, ThunkDefinition(
                              ThunkDefinition::GuestCallbackThunk, this, name, data.type,
                              data.toFunctionDeclReps(name, ThunkDefinition::GuestCallbackThunk),
                              data.hintDecl ? data.hintDecl : data.decl)));
                declaredTypeNames.insert(
                    std::make_pair(getTypeString(data.type.getCanonicalType()), name));
            }

            // Remaining function pointer types
            int i = 0;
            for (const auto &pair : std::as_const(fpTypes)) {
                i++;

                const auto &typeName = pair.first;
                const auto &type = pair.second;
                if (declaredTypeNames.count(typeName)) {
                    continue;
                }
                std::string name = "__UnknownCallback_" + std::to_string(i);
                targetMap.insert(std::make_pair(
                    name, ThunkDefinition(ThunkDefinition::GuestCallbackThunk, this, name, type,
                                          buildGuestCallbackReps(type, name), nullptr)));
                declaredTypeNames.insert(std::make_pair(typeName, name));
            }

            _callbackNameIndexes[0] = std::move(declaredTypeNames);
        }
        {
            // Host callback thunks
            std::map<std::string, std::string> declaredTypeNames;
            const auto &sourceMap = declaredFuncMaps[ThunkDefinition::HostCallbackThunk];
            auto &targetMap = _thunks[ThunkDefinition::HostCallbackThunk];
            for (auto &pair : sourceMap) {
                const auto &name = pair.first;
                const auto &data = pair.second;
                targetMap.insert(std::make_pair(
                    name, ThunkDefinition(
                              ThunkDefinition::HostCallbackThunk, this, name, data.type,
                              data.toFunctionDeclReps(name, ThunkDefinition::HostCallbackThunk),
                              data.hintDecl ? data.hintDecl : data.decl)));
                declaredTypeNames.insert(
                    std::make_pair(getTypeString(data.type.getCanonicalType()), name));
            }

            // Remaining function pointer types
            int i = 0;
            for (const auto &pair : std::as_const(fpTypes)) {
                i++;

                const auto &typeName = pair.first;
                const auto &type = pair.second;
                if (declaredTypeNames.count(typeName)) {
                    continue;
                }
                std::string name = "__UnknownCallback_" + std::to_string(i);
                targetMap.insert(std::make_pair(
                    name, ThunkDefinition(ThunkDefinition::HostCallbackThunk, this, name, type,
                                          buildHostCallbackReps(type, name), nullptr)));
                declaredTypeNames.insert(std::make_pair(typeName, name));
            }

            _callbackNameIndexes[1] = std::move(declaredTypeNames);
        }

        /// STEP: Collect function pointer variables
        for (const auto &pair : std::as_const(declaredVarMap)) {
            const auto &name = pair.first;
            const auto &VD = pair.second.Var;
            _vars.insert(std::make_pair(pair.first, VD));
        }

        /// STEP: Collect missing symbols
        _missingSymbols = std::move(symbols);
    }

    struct PassData {
        Pass *pass = nullptr;
        SmallVector<IntOrString> args;

        template <bool Unique>
        static void runPasses(std::map<std::string, Pass *> &passMap,
                              std::map<std::string, ThunkDefinition> &thunks,
                              Pass *defaultPass = nullptr) {
            for (auto &pair : thunks) {
                auto &name = pair.first;
                auto &thunk = pair.second;

                std::map<int, PassData> result;
                auto annotations = thunk.annotations();
                if (!annotations.empty()) {
                    // Collect passes specified in annotations
                    for (const auto &anno : annotations) {
                        auto it = passMap.find(anno.name);
                        if (it != passMap.end()) {
                            PassData passData;
                            passData.pass = it->second;
                            passData.args = anno.arguments;
                            result[passData.pass->type()] = std::move(passData);
                            if constexpr (Unique)
                                break;
                        }
                    }
                } else {
                    // Test and collect related passes
                    for (const auto &pair : std::as_const(passMap)) {
                        auto pass = pair.second;
                        SmallVector<IntOrString> args;
                        if (pass->test(thunk, args)) {
                            PassData passData;
                            passData.pass = pass;
                            passData.args = std::move(args);
                            result[passData.pass->type()] = std::move(passData);
                            if constexpr (Unique)
                                break;
                        }
                    }
                }

                if (result.empty() && defaultPass) {
                    result[defaultPass->type()] = PassData{defaultPass, {}};
                }

                // Run passes
                for (const auto &pair : std::as_const(result)) {
                    auto &passData = pair.second;
                    if (auto err = passData.pass->begin(thunk, passData.args); err) {
                        llvm::errs()
                            << llvm::format("error: in API \"%s\", failed to start pass @%s: %s\n",
                                            thunk.name().c_str(), passData.pass->id().data(),
                                            llvm::toString(std::move(err)).c_str());
                        std::exit(-1);
                    }
                }
                for (auto it = result.rbegin(); it != result.rend(); it++) {
                    auto &passData = it->second;
                    if (auto err = passData.pass->end(thunk); err) {
                        llvm::errs()
                            << llvm::format("error: in API \"%s\", failed to end pass @%s: %s\n",
                                            thunk.name().c_str(), passData.pass->id().data(),
                                            llvm::toString(std::move(err)).c_str());
                        std::exit(-1);
                    }
                }
            }
        }
    };

    void Analyzer::analyze() {
        auto defaultBuilderPass = Pass::passMap(Pass::Builder).at("standard");
        for (int type = ThunkDefinition::GuestFunctionThunk; type < ThunkDefinition::NumTypes;
             type++) {
            /// STEP: run builder passes, default pass is "standard"
            PassData::runPasses<true>(Pass::passMap(Pass::Builder), _thunks[type],
                                      defaultBuilderPass);

            /// STEP: run entry/exit passes
            PassData::runPasses<false>(Pass::passMap(Pass::EntryExit), _thunks[type]);

            /// STEP: run decoration passes
            PassData::runPasses<false>(Pass::passMap(Pass::Decoration), _thunks[type]);
        }
    }

    static constexpr const char GENERATED_WARNING[] =
        R"(/****************************************************************************
** Lorelei thunk library code from reading C file '%s'
**
** Created by: Lorelei Thunk Library compiler
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/
)";

    void Analyzer::generateHeader(raw_ostream &os) const {
        auto inputFileName = inputFile().filename().string();

        char *warning;
        auto res = asprintf(&warning, GENERATED_WARNING, inputFileName.c_str());
        assert(res > 0);
        os << warning;
        free(warning);

        /// STEP: Generate include guard
        os << "#pragma once\n\n";

        /// STEP: Generate guest function names
        os << "//\n// Declare Apis\n//\n";
        os << "#define LORELIB_API_FOREACH(F)";
        for (const auto &pair : _thunks[ThunkDefinition::GuestFunctionThunk]) {
            os << " \\\n    F(" << pair.first << ")";
        }
        os << "\n\n";

        /// STEP: Generate guest callback names and prototypes
        os << "//\n// Declare Guest Callbacks\n//\n";
        os << "#define LORELIB_GCB_FOREACH(F)";
        for (const auto &pair : _thunks[ThunkDefinition::GuestCallbackThunk]) {
            os << " \\\n    F(" << pair.first << ", \"" << getTypeString(pair.second.qualType())
               << "\")";
        }
        os << "\n\n";

        /// STEP: Generate host callback names and prototypes
        os << "//\n// Declare Host Callbacks\n//\n";
        os << "#define LORELIB_HCB_FOREACH(F)";
        for (const auto &pair : _thunks[ThunkDefinition::HostCallbackThunk]) {
            os << " \\\n    F(" << pair.first << ", \"" << getTypeString(pair.second.qualType())
               << "\")";
        }
        os << "\n\n";
    }

    void Analyzer::generateSource(raw_ostream &os) const {
        auto inputFileName = inputFile().filename().string();

        char *warning;
        auto res = asprintf(&warning, GENERATED_WARNING, inputFileName.c_str());
        assert(res > 0);

        os << warning;
        free(warning);

        os << "#include \"" << inputFileName << "\"\n";
        os << "\n";
        os << "#include <lorelib_common/callback.h>\n";
        os << "\n";

        os << "//\n// Generated(Guest)\n//\n";
        os << "#if defined(LORELIB_VISUAL) || defined(LORELIB_GTL_BUILD)\n";

        /// STEP: Generate variables section
        // TODO

        /// STEP: Generate guest function thunks
        for (const auto &pair : _thunks[ThunkDefinition::GuestFunctionThunk]) {
            auto &thunk = pair.second;
            os << thunk.text(true) << "\n";
        }
        for (const auto &pair : _thunks[ThunkDefinition::GuestCallbackThunk]) {
            auto &thunk = pair.second;
            os << thunk.text(true) << "\n";
        }
        for (const auto &pair : _thunks[ThunkDefinition::GuestCallbackThunk]) {
            auto &thunk = pair.second;
            os << thunk.text(true) << "\n";
        }
        os << "\n";

        /// STEP: Generate variables definitions
        for (const auto &pair : std::as_const(_vars)) {
            auto &VD = pair.second;
            os << "__typeof__(" << getTypeString(VD->getType()) << ") " << VD->getNameAsString();
            if (auto vType = VD->getType().getCanonicalType(); isFunctionPointerType(vType)) {
                os << " = " << "__GTP_" << "__FPVAR_" << VD->getNameAsString();
            }
            os << ";\n";
        }
        os << "\n";

        /// STEP: Generate aliases for guest thunks for export
        for (const auto &pair : _thunks[ThunkDefinition::GuestFunctionThunk]) {
            auto &thunk = pair.second;
            auto &GTP = thunk.function(FunctionDefinition::GTP);
            os << "LORELIB_EXPORT " << GTP.declText(thunk.name()) //
               << " __attribute__((alias(\"" << GTP.rep().name() << "\")));\n";
        }
        os << "\n";

        os << "#endif\n";
        os << "\n";

        os << "//\n// Generated(Host)\n//\n";
        os << "#if defined(LORELIB_VISUAL) || defined(LORELIB_HTL_BUILD)\n";

        /// STEP: Generate host function thunks
        for (const auto &pair : _thunks[ThunkDefinition::GuestFunctionThunk]) {
            auto &thunk = pair.second;
            os << thunk.text(false) << "\n";
        }
        for (const auto &pair : _thunks[ThunkDefinition::GuestCallbackThunk]) {
            auto &thunk = pair.second;
            os << thunk.text(false) << "\n";
        }
        for (const auto &pair : _thunks[ThunkDefinition::HostCallbackThunk]) {
            auto &thunk = pair.second;
            os << thunk.text(false) << "\n";
        }
        os << "\n";

        /// STEP: Generate aliases for host thunks for export
        for (const auto &pair : _thunks[ThunkDefinition::GuestFunctionThunk]) {
            auto &thunk = pair.second;
            auto &HTP = thunk.function(FunctionDefinition::HTP);
            os << "LORELIB_EXPORT " << HTP.declText("_HTP_" + thunk.name()) //
               << " __attribute__((alias(\"" << HTP.rep().name() << "\")));\n";
        }
        os << "#endif\n";
        os << "\n";

        /// STEP: Generate missing function comments
        os << "//\n// Missing Function Declarations\n//\n";
        for (const auto &item : std::as_const(_missingSymbols)) {
            os << "// " << item << "\n";
        }
    }

    std::filesystem::path Analyzer::inputFile() const {
        auto &SM = _ci.getSourceManager();
        return fs::path(SM.getFileEntryRefForID(SM.getMainFileID())->getName().str());
    }

}