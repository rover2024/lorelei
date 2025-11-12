#include "DocumentContext.h"

#include <clang/Basic/SourceManager.h>
#include <clang/AST/Mangle.h>

#include <lorelei/TLCMeta/MetaPass.h>

#include <stdcorelib/path.h>

#include "Pass.h"
#include "TypeExtras.h"

using namespace clang;

namespace TLC {

    struct MetaProcInvoker {
        ClassTemplateDecl *metaProcTDecl;
        TemplateArgument procKindTArg;
        TemplateArgument procThunkPhaseTArg;
        std::unique_ptr<MangleContext> mangleCtx;

        MetaProcInvoker(ClassTemplateDecl *decl, const char *procKind, const char *procThunkPhase) {
            auto &ast = decl->getASTContext();
            auto tArgs = decl->getTemplateParameters()->asArray();
            if (tArgs.size() != 3) {
                llvm::errs() << "error: invalid template parameters count of " << decl->getName()
                             << "\n";
                return;
            }

            EnumDecl *procKindEnumDecl = nullptr;
            EnumDecl *procThunkPhaseEnumDecl = nullptr;
            if (auto nonTypeTArg = dyn_cast<NonTypeTemplateParmDecl>(tArgs[1])) {
                auto argType = nonTypeTArg->getType();
                if (!argType->isEnumeralType()) {
                    llvm::errs() << "error: invalid template parameter type of "
                                 << nonTypeTArg->getName() << "\n";
                    return;
                }
                auto enumDecl = argType->getAs<EnumType>()->getDecl();
                if (enumDecl->getName() != "CProcKind") {
                    llvm::errs() << "error: invalid template parameter type of "
                                 << nonTypeTArg->getName() << "\n";
                    return;
                }
                procKindEnumDecl = enumDecl;
            } else {
                return;
            }

            if (auto nonTypeTArg = dyn_cast<NonTypeTemplateParmDecl>(tArgs[2])) {
                auto argType = nonTypeTArg->getType();
                if (!argType->isEnumeralType()) {
                    llvm::errs() << "error: invalid template parameter type of "
                                 << nonTypeTArg->getName() << "\n";
                    return;
                }
                auto enumDecl = argType->getAs<EnumType>()->getDecl();
                if (enumDecl->getName() != "CProcThunkPhase") {
                    llvm::errs() << "error: invalid template parameter type of "
                                 << nonTypeTArg->getName() << "\n";
                    return;
                }
                procThunkPhaseEnumDecl = enumDecl;
            } else {
                return;
            }

            EnumConstantDecl *procKindECD = nullptr;
            EnumConstantDecl *procThunkPhaseECD = nullptr;
            for (auto *ECD : procKindEnumDecl->enumerators()) {
                if (ECD->getName() == procKind) {
                    procKindECD = ECD;
                    break;
                }
            }
            for (auto *ECD : procThunkPhaseEnumDecl->enumerators()) {
                if (ECD->getName() == procThunkPhase) {
                    procThunkPhaseECD = ECD;
                    break;
                }
            }

            if (!procKindECD || !procThunkPhaseECD) {
                llvm::errs() << "error: failed to find enum constant " << procKind << " or "
                             << procThunkPhase << "\n";
                return;
            }

            metaProcTDecl = decl;
            procKindTArg = TemplateArgument(ast, procKindECD->getInitVal(),
                                            ast.getTypeDeclType(procKindEnumDecl));
            procThunkPhaseTArg = TemplateArgument(ast, procThunkPhaseECD->getInitVal(),
                                                  ast.getTypeDeclType(procThunkPhaseEnumDecl));

            mangleCtx.reset(clang::ItaniumMangleContext::create(ast, ast.getDiagnostics()));
        }

        bool isValid() const {
            return metaProcTDecl;
        }

        std::string getInvokeMangledName(FunctionDecl *FD,
                                         const std::optional<QualType> &overlayType) {
            auto &ast = metaProcTDecl->getASTContext();

            std::array<TemplateArgument, 3> tArgs;
            // tArgs[0] = TemplateArgument(ast.VoidPtrTy);
            // tArgs[1] = TemplateArgument(ast.VoidPtrTy);
            // tArgs[2] = TemplateArgument(ast.VoidPtrTy);
            tArgs[0] = TemplateArgument(FD, ast.getPointerType(FD->getType()));
            tArgs[1] = procKindTArg;
            tArgs[2] = procThunkPhaseTArg;

            // Search for a matching specialization
            void *insertPos = nullptr;
            auto spec = metaProcTDecl->findSpecialization(tArgs, insertPos);
            if (!spec) {
                // No matching specialization found, create a new one
                spec = ClassTemplateSpecializationDecl::Create(
                    ast, metaProcTDecl->getTemplatedDecl()->getTagKind(),
                    metaProcTDecl->getTemplatedDecl()->getDeclContext(),
                    metaProcTDecl->getTemplatedDecl()->getLocation(), metaProcTDecl->getLocation(),
                    metaProcTDecl, tArgs, nullptr);
            }

            if (!spec) {
                llvm::outs()
                    << "Error: Failed to find or create specialization of 'MetaProc' template.\n";
                return {};
            }

            // Find the 'invoke' method in the specialization
            CXXMethodDecl *InvokeMethod = nullptr;
            for (auto *D : spec->decls()) {
                if (auto *Method = dyn_cast<CXXMethodDecl>(D)) {
                    if (Method->getName() == "invoke") {
                        InvokeMethod = Method;
                        break;
                    }
                }
            }

            if (!InvokeMethod) {
                QualType InvokeType =
                    overlayType ? overlayType.value()->getPointeeType() : FD->getType();

                // Create the 'invoke' method declaration
                InvokeMethod = CXXMethodDecl::Create(
                    ast, spec, spec->getLocation(),
                    DeclarationNameInfo(DeclarationName(&ast.Idents.get("invoke")),
                                        spec->getLocation()),
                    InvokeType, ast.getTrivialTypeSourceInfo(InvokeType), StorageClass::SC_Static,
                    false, false, ConstexprSpecKind::Unspecified, spec->getLocation());
                InvokeMethod->setAccess(AS_public);
                InvokeMethod->setImplicit(true);

                if (!overlayType) {
                    InvokeMethod->setParams(FD->parameters());
                } else {
                    FunctionTypeView view(overlayType.value());
                    llvm::SmallVector<ParmVarDecl *, 4> params;
                    params.reserve(view.argTypes().size());
                    for (size_t i = 0; i < view.argTypes().size(); ++i) {
                        const auto &argType = view.argTypes()[i];
                        auto param = ParmVarDecl::Create(
                            ast, InvokeMethod->getDeclContext(), SourceLocation(), SourceLocation(),
                            &ast.Idents.get("arg" + std::to_string(i + 1)), argType,
                            ast.getTrivialTypeSourceInfo(argType), StorageClass::SC_None, nullptr);
                        params.push_back(param);
                    }
                    InvokeMethod->setParams(params);
                }
                spec->addDecl(InvokeMethod);
            }

            if (!InvokeMethod->isUserProvided()) {
                llvm::outs() << "Error: Failed to create user-provided 'invoke' method.\n";
                return {};
            }

            // Mangle the 'invoke' method name
            std::string MangledName;
            llvm::raw_string_ostream Stream(MangledName);
            if (mangleCtx->shouldMangleDeclName(InvokeMethod))
                mangleCtx->mangleName(InvokeMethod, Stream);
            Stream.flush();
            return MangledName;
        }
    };

    void DocumentContext::initialize(const lore::ConfigFile &inputConfig) {
        _inputConfig = inputConfig;
    }

    void DocumentContext::handleTranslationUnit(ASTContext &ast) {
        ASTMetaContext::handleTranslationUnit(ast);

        for (auto &pair : Pass::passMap(Pass::Builder)) {
            pair.second->handleTranslationUnit(*this);
        }
        for (auto &pair : Pass::passMap(Pass::Guard)) {
            pair.second->handleTranslationUnit(*this);
        }
        for (auto &pair : Pass::passMap(Pass::Misc)) {
            pair.second->handleTranslationUnit(*this);
        }
    }

    bool DocumentContext::beginSourceFileAction(clang::CompilerInstance &CI) {
        return true;
    }

    void DocumentContext::endSourceFileAction() {
        /// STEP: Collect information from MetaXXX items
        for (const auto &item : _metaConfigs) {
            if (!item) {
                continue;
            }
            auto &metaConf = item.value();
            if (metaConf.isHost().has_value()) {
                _isHost = metaConf.isHost().value();
            }
            if (metaConf.moduleName().has_value()) {
                _moduleName = metaConf.moduleName().value();
            }
        }

        /// STEP: Collect listed objects
        decltype(_functions) allFunctions;
        decltype(_vars) allVars;
        std::swap(allFunctions, _functions);
        std::swap(allVars, _vars);
        if (auto section = std::as_const(_inputConfig).get("Function")) {
            auto &listed = section->get();
            for (auto sym : std::as_const(listed)) {
                if (auto it = allFunctions.find(sym.first); it != allFunctions.end()) {
                    _functions[sym.first] = it->second;
                } else {
                    _missingFunctions.insert(sym.first);
                }
            }
        }
        if (auto section = std::as_const(_inputConfig).get("Variable")) {
            auto &listed = section->get();
            for (auto sym : std::as_const(listed)) {
                if (auto it = allVars.find(sym.first); it != allVars.end()) {
                    _vars[sym.first] = it->second;
                } else {
                    _missingVars.insert(sym.first);
                }
            }
        }
        if (auto section = std::as_const(_inputConfig).get("Guest Function")) {
            auto &listed = section->get();
            for (auto sym : std::as_const(listed)) {
                if (auto it = allFunctions.find(sym.first); it != allFunctions.end()) {
                    _guestFunctions[sym.first] = it->second;
                } else {
                    _missingFunctions.insert(sym.first);
                }
            }
        }

        // Add typedef declared function pointer types
        std::map<std::string, QualType> knownCallbacks;
        std::map<std::string, std::string> namedCallbackMap; // type string -> name
        for (auto it : std::as_const(_functionPointerTypedefs)) {
            auto &typedefDecl = it.second;
            auto fpType = typedefDecl->getUnderlyingType().getCanonicalType();
            namedCallbackMap[getTypeString(fpType)] = typedefDecl->getNameAsString();
        }

        if (auto section = std::as_const(_inputConfig).get("Callback")) {
            auto &listed = section->get();
            for (auto it : std::as_const(listed)) {
                std::string token = it.first;
                if (token.front() == '&') {
                    std::string orgToken = token;
                    // The callback type is referenced from an existing function
                    token = token.substr(1);
                    auto it = allFunctions.find(token);
                    if (it != allFunctions.end()) {
                        auto FD = it->second;
                        auto fpType = _ast->getPointerType(FD->getType().getCanonicalType());
                        auto fpTypeStr = getTypeString(fpType);
                        if (!namedCallbackMap.count(fpTypeStr)) {
                            namedCallbackMap[fpTypeStr] = "PFN_AUTO_" + token;
                        }
                        knownCallbacks[fpTypeStr] = fpType;
                    } else {
                        _missingCallbacks.insert(orgToken);
                    }
                } else {
                    auto it = _functionPointerTypedefs.find(token);
                    if (it != _functionPointerTypedefs.end()) {
                        auto fpType = it->second->getUnderlyingType().getCanonicalType();
                        auto fpTypeStr = getTypeString(fpType);
                        knownCallbacks[fpTypeStr] = fpType;
                    } else {
                        _missingCallbacks.insert(token);
                    }
                }
            }
        }

        /// STEP: Add callbacks with explicit desc item
        for (auto it : std::as_const(_metaProcCBDescs)) {
            auto &desc = it.second;

            auto type = desc.procType();
            auto fpTypeStr = getTypeString(type.getCanonicalType());
            // may overwrite the name hint from reading the config file
            if (!desc.nameHint().empty()) {
                namedCallbackMap[fpTypeStr] = desc.nameHint();
            } else if (auto typedefType = type->getAs<TypedefType>()) {
                auto name = typedefType->getDecl()->getName();
                namedCallbackMap[fpTypeStr] = name.str();
            }
            knownCallbacks[fpTypeStr] = type;
        }

        const auto &addKnownCallback = [&](const QualType &type, const std::string &nameHint) {
            auto fpTypeStr = getTypeString(type.getCanonicalType());
            if (!namedCallbackMap.count(fpTypeStr)) {
                if (auto typedefType = type->getAs<TypedefType>()) {
                    auto name = typedefType->getDecl()->getName();
                    namedCallbackMap[fpTypeStr] = name.str();
                } else if (!nameHint.empty()) {
                    namedCallbackMap[fpTypeStr] = nameHint;
                }
            }
            knownCallbacks[fpTypeStr] = type;
        };

        /// STEP: Add callback overlay types
        for (auto it : std::as_const(_metaProcCBDescs)) {
            auto &desc = it.second;
            if (!desc.overlayType()) {
                continue;
            }
            std::string nameHint;
            if (auto it = namedCallbackMap.find(getTypeString(desc.procType().getCanonicalType()));
                it != namedCallbackMap.end()) {
                nameHint = "OVERLAY_" + it->second;
            }
            addKnownCallback(desc.overlayType().value(), nameHint);
        }

        /// STEP: Collect callbacks in function arguments and return types
        {
            SmallVector<QualType> stack;
            const auto &pushFunctionView = [&](const FunctionTypeView &func) {
                stack.push_back(func.returnType());
                for (const auto &argType : func.argTypes()) {
                    stack.push_back(argType);
                }
            };
            const auto &pushFunctionDecl = [&](const FunctionDecl *func) {
                FunctionTypeView view(func->getType());
                pushFunctionView(view);

                auto it = _metaProcDescs.find(func->getName().str());
                if (it != _metaProcDescs.end()) {
                    auto &desc = it->second;
                    if (desc.overlayType()) {
                        FunctionTypeView overlayView(desc.overlayType().value());
                        pushFunctionView(overlayView);
                    }
                }
            };

            for (const auto &pair : std::as_const(_functions)) {
                pushFunctionDecl(pair.second);
            }
            for (const auto &pair : std::as_const(_guestFunctions)) {
                pushFunctionDecl(pair.second);
            }
            for (const auto &it : std::as_const(knownCallbacks)) {
                stack.push_back(it.second);
            }

            // Disabled. May break the consistency of guest and host items
            // for (auto kind : {CProcKind_HostCallback, CProcKind_GuestCallback}) {
            //     for (auto &phaseMap : _metaProcCBs.at(kind)) {
            //         for (auto it2 : phaseMap) {
            //             stack.push_back(it2.second.procType());
            //         }
            //     }
            // }

            std::set<std::string> visitedTypes;
            while (!stack.empty()) {
                auto type = stack.pop_back_val().getCanonicalType();
                while (true) {
                    if (type->isFunctionPointerType()) {
                        break;
                    }
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
                auto typeStr = getTypeString(type);
                if (visitedTypes.count(typeStr)) {
                    continue;
                }
                visitedTypes.insert(typeStr);

                if (type->isFunctionPointerType()) {
                    auto functionType = type->getPointeeType();
                    if (functionType->isFunctionProtoType()) {
                        auto FPT = functionType->getAs<FunctionProtoType>();
                        if (auto retType = FPT->getReturnType(); !retType->isVoidType())
                            stack.push_back(FPT->getReturnType());
                        for (const auto &T : FPT->param_types()) {
                            stack.push_back(T);
                        }
                        _callbacks[typeStr] = type;
                        continue;
                    }
                    if (functionType->isFunctionNoProtoType()) {
                        auto FNT = functionType->getAs<FunctionNoProtoType>();
                        stack.push_back(FNT->getReturnType());
                        _callbacks[typeStr] = type;
                        continue;
                    }
                } else if (type->isRecordType()) {
                    auto recType = type->getAs<RecordType>();
                    for (const auto &field : recType->getDecl()->fields()) {
                        stack.push_back(field->getType());
                    }
                    continue;
                }
            }
        }

        /// STEP: Create \c ProcContext instances
        for (auto it : std::as_const(_functions)) {
            auto FD = it.second;
            auto ctx = std::make_unique<ProcContext>(CProcKind_HostFunction, FD,
                                                     FD->getName().str(), *this);
            _procContexts[CProcKind_HostFunction].insert(
                std::make_pair(FD->getName(), std::move(ctx)));
        }
        // for (auto it : std::as_const(_vars)) {
        //     // Add function pointer types from variables
        //     auto &VD = it.second;
        //     auto type = VD->getType();
        //     if (!type.getCanonicalType()->isFunctionPointerType()) {
        //         continue;
        //     }
        //     std::string name = "__FPVAR_" + VD->getName().str();
        //     ProcContext ctx(CProcKind_HostFunction, nullptr, type, name, *this);
        //     _procContexts[CProcKind_HostFunction].insert(std::make_pair(name, ctx));
        // }

        for (auto it : std::as_const(_guestFunctions)) {
            auto FD = it.second;
            auto ctx = std::make_unique<ProcContext>(CProcKind_GuestFunction, FD,
                                                     FD->getName().str(), *this);
            _procContexts[CProcKind_GuestFunction].insert(
                std::make_pair(FD->getName(), std::move(ctx)));
        }
        int numUnknownCallback = 0;
        for (auto it : std::as_const(_callbacks)) {
            auto type = it.second;
            std::string nameHint;
            if (auto it = namedCallbackMap.find(getTypeString(type.getCanonicalType()));
                it != namedCallbackMap.end()) {
                nameHint = it->second;
            } else {
                numUnknownCallback++;
                nameHint = "PFN_UnknownCallback_" + std::to_string(numUnknownCallback);
            }
            {
                auto ctx =
                    std::make_unique<ProcContext>(CProcKind_GuestCallback, type, nameHint, *this);
                _procContexts[CProcKind_GuestCallback].insert(
                    std::make_pair(it.first, std::move(ctx)));
            }
            {
                auto ctx =
                    std::make_unique<ProcContext>(CProcKind_HostCallback, type, nameHint, *this);
                _procContexts[CProcKind_HostCallback].insert(
                    std::make_pair(it.first, std::move(ctx)));
            }
        }

        /// STEP: Run passes
        // begin
        for (auto &pair : Pass::passMap(Pass::Builder)) {
            pair.second->beginProcessDocument(*this);
        }
        for (auto &pair : Pass::passMap(Pass::Guard)) {
            pair.second->beginProcessDocument(*this);
        }
        for (auto &pair : Pass::passMap(Pass::Misc)) {
            pair.second->beginProcessDocument(*this);
        }

        Pass *defaultBuilderPass;
        if (auto it = Pass::passMap(Pass::Builder).find(lorethunk::MetaPass::DefaultBuilder);
            it == Pass::passMap(Pass::Builder).end()) {
            llvm::errs() << "error: No default builder pass found\n";
            std::exit(-1);
        } else {
            defaultBuilderPass = it->second;
        }

        using RunPassTuple = std::tuple<ProcContext *, Pass *, std::unique_ptr<ProcMessage>>;
        llvm::SmallVector<RunPassTuple> passTuples;

        // Builder pass is unique for each proc
        for (int kind = CProcKind_HostFunction; kind < CProcKind_NumProcKind; kind++) {
            for (auto &it : _procContexts[kind]) {
                auto &proc = *it.second;
                // Find builder pass
                Pass *pass = nullptr;
                for (auto &pair : Pass::passMap(Pass::Builder)) {
                    if (pair.first == lorethunk::MetaPass::DefaultBuilder) {
                        continue;
                    }

                    std::unique_ptr<ProcMessage> msg;
                    auto curPass = pair.second;
                    if (curPass->testProc(proc, msg)) {
                        passTuples.emplace_back(&proc, curPass, std::move(msg));
                        pass = curPass;
                        break;
                    }
                }

                if (!pass) {
                    passTuples.emplace_back(&proc, defaultBuilderPass, nullptr);
                }
            }
        }
        for (int kind = CProcKind_HostFunction; kind < CProcKind_NumProcKind; kind++) {
            for (auto &it : _procContexts[kind]) {
                auto &proc = *it.second;
                for (auto &pair : Pass::passMap(Pass::Guard)) {
                    std::unique_ptr<ProcMessage> msg;
                    auto pass = pair.second;
                    if (pass->testProc(proc, msg)) {
                        passTuples.emplace_back(&proc, pass, std::move(msg));
                    }
                }
            }
        }
        for (int kind = CProcKind_HostFunction; kind < CProcKind_NumProcKind; kind++) {
            for (auto &it : _procContexts[kind]) {
                auto &proc = *it.second;
                for (auto &pair : Pass::passMap(Pass::Misc)) {
                    std::unique_ptr<ProcMessage> msg;
                    auto pass = pair.second;
                    if (pass->testProc(proc, msg)) {
                        passTuples.emplace_back(&proc, pass, std::move(msg));
                    }
                }
            }
        }

        for (auto &tuple : passTuples) {
            auto &[proc, pass, msg] = tuple;
            if (auto err = pass->beginHandleProc(*proc, msg); err) {
                llvm::errs() << llvm::format("error: in API \"%s\", failed to begin pass @%s: %s\n",
                                             proc->name().c_str(), pass->name().c_str(),
                                             llvm::toString(std::move(err)).c_str());
                std::exit(-1);
            }
        }
        for (auto it = passTuples.rbegin(); it != passTuples.rend(); it++) {
            auto &[proc, pass, msg] = *it;
            if (auto err = pass->endHandleProc(*proc, msg); err) {
                llvm::errs() << llvm::format("error: in API \"%s\", failed to end pass @%s: %s\n",
                                             proc->name().c_str(), pass->name().c_str(),
                                             llvm::toString(std::move(err)).c_str());
                std::exit(-1);
            }
        }

        // end
        for (auto &pair : Pass::passMap(Pass::Builder)) {
            pair.second->endProcessDocument(*this);
        }
        for (auto &pair : Pass::passMap(Pass::Guard)) {
            pair.second->endProcessDocument(*this);
        }
        for (auto &pair : Pass::passMap(Pass::Misc)) {
            pair.second->endProcessDocument(*this);
        }
    }

    void DocumentContext::generateOutput(llvm::raw_ostream &os) {
        const auto &legend = [](const std::string &name) {
            int eqCount = (75 - name.size()) / 2;
            return "// " + std::string(eqCount, '=') + " " + name + " " + std::string(eqCount, '=');
        };

        auto &ast = *_ast;
        auto &SM = ast.getSourceManager();
        auto mainFilePath = SM.getFileEntryRefForID(SM.getMainFileID())->getName();

        /// STEP: Add macro
        os << "#define LORETHUNK_BUILD\n";
        os << "\n";

        /// STEP: Include necessary headers
        os << "#include <lorelei/TLCMeta/ManifestlGlobal.h>\n";
        os << "#include <lorelei/TLCMeta/ManifestConfig.h>\n";
        os << "#include <lorelei/TLCMeta/ManifestCallbackDefs.h>\n";
        os << "#include <lorelei/TLCMeta/MetaProc.h>\n";
        os << "\n";

        /// STEP: Generate symbol declarations
        os << "extern \"C\" {\n";
        os << "#pragma GCC diagnostic push\n";
        os << "#pragma GCC diagnostic ignored \"-Wattribute-alias\"\n";
        if (_isHost) {
            MetaProcInvoker MPI(_metaProcDecl, "CProcKind_GuestFunction", "CProcThunkPhase_HTP");
            if (!MPI.isValid()) {
                std::exit(1);
            }

            for (auto &it : _procContexts[CProcKind_GuestFunction]) {
                auto &proc = *it.second;
                os << "LORETHUNK_EXPORT "
                   << FunctionInfo(proc.functionDecl()).declText("GTL_" + proc.name(), ast)
                   << "\n    __attribute__((alias(\""
                   << MPI.getInvokeMangledName(const_cast<FunctionDecl *>(proc.functionDecl()),
                                               proc.overlayType())
                   << "\")));\n";
            }
        } else {
            MetaProcInvoker MPI(_metaProcDecl, "CProcKind_HostFunction", "CProcThunkPhase_GTP");
            if (!MPI.isValid()) {
                std::exit(1);
            }

            for (auto &it : _procContexts[CProcKind_HostFunction]) {
                auto &proc = *it.second;
                os << "LORETHUNK_EXPORT "
                   << FunctionInfo(proc.functionDecl()).declText(proc.name(), ast)
                   << "\n    __attribute__((alias(\""
                   << MPI.getInvokeMangledName(const_cast<FunctionDecl *>(proc.functionDecl()),
                                               proc.overlayType())
                   << "\")));\n";
            }
        }
        os << "#pragma GCC diagnostic pop\n";
        os << "}\n\n";

        /// STEP: Generate document head
        os << _source.head.toRawText() << "\n";

        /// STEP: Generate macros
        os << "#define LORETHUNK_MODULE_NAME \"" << _moduleName << "\"\n\n";
        os << "#define LORETHUNK_HOST_FUNCTION_FOREACH(F)";
        for (const auto &pair : _procContexts[CProcKind_HostFunction]) {
            os << " \\\n    F(" << pair.second->name() << ")";
        }
        os << "\n\n";
        os << "#define LORETHUNK_GUEST_FUNCTION_FOREACH(F)";
        for (const auto &pair : _procContexts[CProcKind_GuestFunction]) {
            os << " \\\n    F(" << pair.second->name() << ")";
        }
        os << "\n\n";

        os << "namespace lorethunk {\n\n";
        for (const auto &pair : _procContexts[CProcKind_GuestCallback]) {
            os << "using " << pair.second->name() << " = "
               << getTypeString(pair.second->functionPointerType()) << ";\n";
        }
        os << "\n";
        os << "}\n\n";

        os << "#define LORETHUNK_CALLBACK_FOREACH(F)";
        for (const auto &pair : _procContexts[CProcKind_GuestCallback]) {
            os << " \\\n    F(" << pair.second->name() << ", lorethunk::" << pair.second->name()
               << ")";
        }
        os << "\n\n";

        /// STEP: Generate forward declarations
        os << "namespace lorethunk {\n\n";

        if (_isHost) {
            for (int kind = CProcKind_HostFunction; kind < CProcKind_NumProcKind; kind++) {
                for (auto &it : _procContexts[kind]) {
                    auto &proc = *it.second;
                    os << legend(proc.name()) << "\n";
                    if (!proc.hasDefinition(CProcThunkPhase_HTP_IMPL)) {
                        os << proc.text(CProcThunkPhase_HTP_IMPL, true, ast) << "\n";
                    }
                    if (!proc.hasDefinition(CProcThunkPhase_HTP)) {
                        os << proc.text(CProcThunkPhase_HTP, true, ast) << "\n";
                    }
                }
            }
        } else {
            for (int kind = CProcKind_HostFunction; kind < CProcKind_NumProcKind; kind++) {
                for (auto &it : _procContexts[kind]) {
                    auto &proc = *it.second;
                    os << legend(proc.name()) << "\n";
                    if (!proc.hasDefinition(CProcThunkPhase_GTP_IMPL)) {
                        os << proc.text(CProcThunkPhase_GTP_IMPL, true, ast) << "\n";
                    }
                    if (!proc.hasDefinition(CProcThunkPhase_GTP)) {
                        os << proc.text(CProcThunkPhase_GTP, true, ast) << "\n";
                    }
                }
            }
        }

        os << "}\n\n";

        /// STEP: Include main source file
        os << "#include \"" << stdc::clean_path(mainFilePath.str()) << "\"\n\n";

        /// STEP: Generate definitions
        os << "namespace lorethunk {\n\n";

        if (_isHost) {
            for (int kind = CProcKind_HostFunction; kind < CProcKind_NumProcKind; kind++) {
                for (auto &it : _procContexts[kind]) {
                    auto &proc = *it.second;
                    os << legend(proc.name()) << "\n";
                    if (!proc.hasDefinition(CProcThunkPhase_HTP_IMPL)) {
                        os << proc.text(CProcThunkPhase_HTP_IMPL, false, ast) << "\n";
                    }
                    if (!proc.hasDefinition(CProcThunkPhase_HTP)) {
                        os << proc.text(CProcThunkPhase_HTP, false, ast) << "\n";
                    }
                }
            }
        } else {
            for (int kind = CProcKind_HostFunction; kind < CProcKind_NumProcKind; kind++) {
                for (auto &it : _procContexts[kind]) {
                    auto &proc = *it.second;
                    os << legend(proc.name()) << "\n";
                    if (!proc.hasDefinition(CProcThunkPhase_GTP_IMPL)) {
                        os << proc.text(CProcThunkPhase_GTP_IMPL, false, ast) << "\n";
                    }
                    if (!proc.hasDefinition(CProcThunkPhase_GTP)) {
                        os << proc.text(CProcThunkPhase_GTP, false, ast) << "\n";
                    }
                }
            }
        }

        os << "}\n\n";

        /// STEP: Generate document tail
        os << _source.tail.toRawText() << "\n";

        /// STEP: Generate missing declarations
        os << "//\n// Missing Function Declarations\n//\n";
        for (const auto &item : std::as_const(_missingFunctions)) {
            os << "// " << item << "\n";
        }
        os << "\n";
        os << "//\n// Missing Variable Declarations\n//\n";
        for (const auto &item : std::as_const(_missingVars)) {
            os << "// " << item << "\n";
        }
        os << "\n";
        os << "//\n// Missing Callback Declarations\n//\n";
        for (const auto &item : std::as_const(_missingCallbacks)) {
            os << "// " << item << "\n";
        }
        os << "\n";

        /// STEP: Add internal tail
        if (_isHost) {
            os << "#include <lorelei/TLCMeta/private/ManifestContext_host_impl.inc.h>\n";
        } else {
            os << "#include <lorelei/TLCMeta/private/ManifestContext_guest_impl.inc.h>\n";
        }

        os << "\n";
    }

}