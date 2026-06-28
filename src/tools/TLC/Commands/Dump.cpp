// SPDX-License-Identifier: MIT

#include <string>
#include <map>
#include <set>
#include <vector>
#include <filesystem>
#include <system_error>
#include <algorithm>
#include <cctype>
#include <sstream>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <lorelei/Support/ConfigFile.h>
#include <lorelei/ClangExtras/CommonMatchFinder.h>
#include <lorelei/ClangExtras/TypeUtils.h>
#include <lorelei/ClangExtras/DeclUtils.h>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

namespace cl = llvm::cl;

namespace lore::tool::command::dump {

    struct GlobalContext {
        SmallString<128> initialCwd;
        std::string outputPath;
        std::string configPath;
        lore::ConfigFile config;

        struct RequestedFunction {
            std::set<std::string> sections;
        };

        struct ResolvedFunction {
            std::set<std::string> headerPaths;
            std::set<std::string> nonHeaderPaths;
        };

        struct CapturedType {
            std::string originalText;
            bool pointerLike = false;
            std::set<std::string> headerPaths;
            std::set<std::string> nonHeaderPaths;
            std::set<std::string> dependencyNames;
        };

        struct CapturedParam {
            std::string name;
            CapturedType type;
        };

        struct CapturedFunction {
            std::string sourcePath;
            std::string sourceLocation;
            CapturedType returnType;
            std::vector<CapturedParam> params;
            bool variadic = false;
        };

        std::map<std::string, RequestedFunction> requestedFunctions;
        std::map<std::string, ResolvedFunction> resolvedFunctions;
        std::map<std::string, CapturedFunction> capturedFunctions;

        std::string outBuffer;
    };

    static GlobalContext &g_ctx() {
        static GlobalContext instance;
        return instance;
    }

    static llvm::Error parseOptions(int &argc, const char **argv, cl::OptionCategory &cat,
                                    const char *overview = nullptr) {
        cl::ResetAllOptionOccurrences();
        cl::HideUnrelatedOptions(cat);

        std::string err;
        llvm::raw_string_ostream os(err);
        if (!cl::ParseCommandLineOptions(argc, argv, overview, &os)) {
            return llvm::make_error<llvm::StringError>(err, llvm::inconvertibleErrorCode());
        }
        cl::PrintOptionValues();
        return llvm::Error::success();
    }

    static std::string normalizePath(llvm::StringRef path) {
        std::filesystem::path fsPath(path.str());
        if (fsPath.is_relative()) {
            fsPath = std::filesystem::path(std::string(g_ctx().initialCwd.str())) / fsPath;
        }
        return fsPath.lexically_normal().string();
    }

    static std::string normalizeCompileCommandPath(const CompileCommand &command) {
        std::filesystem::path fsPath(command.Filename);
        if (fsPath.is_relative()) {
            fsPath = std::filesystem::path(command.Directory) / fsPath;
        }
        return fsPath.lexically_normal().string();
    }

    static std::string lowerExtension(llvm::StringRef path) {
        std::string ext = llvm::sys::path::extension(path).str();
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char ch) { return std::tolower(ch); });
        return ext;
    }

    static bool isLikelyHeaderPath(llvm::StringRef path) {
        std::string ext = lowerExtension(path);
        return ext == ".h" || ext == ".hh" || ext == ".hpp" || ext == ".hxx" || ext == ".h++" ||
               ext == ".inc" || ext == ".inl" || ext == ".tcc";
    }

    static bool isSupportedSourcePath(llvm::StringRef path) {
        std::string ext = lowerExtension(path);
        return ext == ".c" || ext == ".cc" || ext == ".cpp" || ext == ".cxx" || ext == ".c++" ||
               ext == ".m" || ext == ".mm";
    }

    static std::string escapeForInclude(llvm::StringRef path) {
        std::string out;
        out.reserve(path.size());
        for (char ch : path) {
            if (ch == '\\' || ch == '"') {
                out.push_back('\\');
            }
            out.push_back(ch);
        }
        return out;
    }

    static void appendReason(std::string &text, llvm::StringRef reason) {
        if (!text.empty()) {
            text += "; ";
        }
        text += reason.str();
    }

    static void collectRequestedFunctionsFromSection(llvm::StringRef sectionName) {
        auto sectionOpt = g_ctx().config.get(sectionName.str());
        if (!sectionOpt) {
            return;
        }

        for (const auto &[name, _] : sectionOpt->get()) {
            (void) _;
            if (name.empty()) {
                continue;
            }
            g_ctx().requestedFunctions[name].sections.insert(sectionName.str());
        }
    }

    static void loadRequestedFunctions() {
        g_ctx().requestedFunctions.clear();
        collectRequestedFunctionsFromSection("Function");
        collectRequestedFunctionsFromSection("Guest Function");
    }

    static void mergeSets(std::set<std::string> &dst, const std::set<std::string> &src) {
        dst.insert(src.begin(), src.end());
    }

    static void addNamedDeclDependency(GlobalContext::CapturedType &out, const NamedDecl *decl,
                                       SourceManager &sm) {
        if (!decl) {
            return;
        }

        auto name = decl->getQualifiedNameAsString();
        if (!name.empty()) {
            out.dependencyNames.insert(name);
        }

        SourceLocation loc = sm.getExpansionLoc(decl->getLocation());
        if (loc.isInvalid()) {
            return;
        }

        llvm::StringRef fileName = sm.getFilename(loc);
        if (fileName.empty()) {
            return;
        }

        std::string normalizedPath = normalizePath(fileName);
        if (isLikelyHeaderPath(normalizedPath)) {
            out.headerPaths.insert(normalizedPath);
        } else {
            out.nonHeaderPaths.insert(normalizedPath);
        }
    }

    static void collectTypeDependencies(GlobalContext::CapturedType &out, QualType type,
                                        SourceManager &sm) {
        if (type.isNull()) {
            return;
        }

        // Peel off sugar that does not change which declarations the type depends on, so the
        // checks below see the underlying typedef/enum/record/pointer.
        while (true) {
            if (const auto *adjustedType = type->getAs<AdjustedType>()) {
                type = adjustedType->getOriginalType();
                continue;
            }
            if (const auto *decayedType = type->getAs<DecayedType>()) {
                type = decayedType->getOriginalType();
                continue;
            }
            if (const auto *parenType = type->getAs<ParenType>()) {
                type = parenType->getInnerType();
                continue;
            }
            if (const auto *attrType = type->getAs<AttributedType>()) {
                type = attrType->getModifiedType();
                continue;
            }
            if (const auto *macroType = type->getAs<MacroQualifiedType>()) {
                type = macroType->getUnderlyingType();
                continue;
            }
            if (const auto *elabType = type->getAs<ElaboratedType>()) {
                type = elabType->getNamedType();
                continue;
            }
            break;
        }

        if (type->isBuiltinType() || type->isVoidType()) {
            return;
        }

        if (const auto *typedefType = type->getAs<TypedefType>()) {
            addNamedDeclDependency(out, typedefType->getDecl(), sm);
            return;
        }

        if (const auto *usingType = llvm::dyn_cast<UsingType>(type.getTypePtr())) {
            addNamedDeclDependency(out, usingType->getFoundDecl(), sm);
            return;
        }

        if (const auto *enumType = type->getAs<EnumType>()) {
            addNamedDeclDependency(out, enumType->getDecl(), sm);
            return;
        }

        if (const auto *recordType = type->getAs<RecordType>()) {
            addNamedDeclDependency(out, recordType->getDecl(), sm);
            return;
        }

        if (type->isPointerType()) {
            collectTypeDependencies(out, type->getPointeeType(), sm);
            return;
        }

        if (type->isReferenceType()) {
            collectTypeDependencies(out, type->getPointeeType(), sm);
            return;
        }

        if (type->isArrayType()) {
            collectTypeDependencies(out, type->getAsArrayTypeUnsafe()->getElementType(), sm);
            return;
        }

        if (const auto *funcType = type->getAs<FunctionProtoType>()) {
            collectTypeDependencies(out, funcType->getReturnType(), sm);
            for (auto paramType : funcType->getParamTypes()) {
                collectTypeDependencies(out, paramType, sm);
            }
            return;
        }

        if (const auto *funcType = type->getAs<FunctionNoProtoType>()) {
            collectTypeDependencies(out, funcType->getReturnType(), sm);
            return;
        }
    }

    static GlobalContext::CapturedType captureType(QualType type, SourceManager &sm) {
        GlobalContext::CapturedType out;
        out.originalText = getTypeStringDecompound(type);
        out.pointerLike = type->isPointerType();
        collectTypeDependencies(out, type, sm);
        return out;
    }

    static GlobalContext::CapturedFunction captureFunction(const FunctionDecl &decl,
                                                           SourceManager &sm) {
        GlobalContext::CapturedFunction out;

        SourceLocation loc = sm.getExpansionLoc(decl.getLocation());
        out.sourceLocation = loc.isValid() ? loc.printToString(sm) : std::string();
        if (loc.isValid()) {
            llvm::StringRef fileName = sm.getFilename(loc);
            if (!fileName.empty()) {
                out.sourcePath = normalizePath(fileName);
            }
        }

        out.returnType = captureType(decl.getReturnType(), sm);
        out.variadic = decl.isVariadic();

        size_t index = 0;
        for (const auto *param : decl.parameters()) {
            index++;
            GlobalContext::CapturedParam capturedParam;
            capturedParam.name = param->getNameAsString();
            if (capturedParam.name.empty()) {
                capturedParam.name = "arg" + std::to_string(index);
            }
            capturedParam.type = captureType(param->getType(), sm);
            out.params.push_back(std::move(capturedParam));
        }

        return out;
    }

    class MyASTConsumer : public ASTConsumer {
    public:
        void HandleTranslationUnit(ASTContext &ast) override {
            auto &sm = ast.getSourceManager();

            const auto matchCallback = [&sm](const MatchFinder::MatchResult &result) {
                const auto *decl = result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
                if (!decl || !isCLinkage(decl)) {
                    return;
                }

                auto requestedIt = g_ctx().requestedFunctions.find(decl->getNameAsString());
                if (requestedIt == g_ctx().requestedFunctions.end()) {
                    return;
                }

                SourceLocation loc = sm.getExpansionLoc(decl->getLocation());
                if (loc.isInvalid()) {
                    return;
                }

                llvm::StringRef fileName = sm.getFilename(loc);
                if (fileName.empty()) {
                    return;
                }

                std::string normalizedPath = normalizePath(fileName);
                auto &resolved = g_ctx().resolvedFunctions[decl->getNameAsString()];
                if (isLikelyHeaderPath(normalizedPath)) {
                    resolved.headerPaths.insert(normalizedPath);
                } else {
                    resolved.nonHeaderPaths.insert(normalizedPath);
                    if (!g_ctx().capturedFunctions.count(decl->getNameAsString())) {
                        g_ctx().capturedFunctions[decl->getNameAsString()] =
                            captureFunction(*decl, sm);
                    }
                }
            };

            tool::CommonMatchFinder matchHandler(matchCallback);
            MatchFinder finder;
            finder.addMatcher(functionDecl().bind("functionDecl"), &matchHandler);
            finder.matchAST(ast);
        }
    };

    class MyASTFrontendAction : public ASTFrontendAction {
    public:
        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &, StringRef) override {
            return std::make_unique<MyASTConsumer>();
        }
    };

    static void appendFunctionListComment(llvm::raw_ostream &out, llvm::StringRef title,
                                          const std::vector<std::string> &lines) {
        out << "/*\n";
        out << " * " << title << "\n";
        for (const auto &line : lines) {
            out << " * - " << line << "\n";
        }
        out << " */\n";
    }

    static std::string joinSet(const std::set<std::string> &items) {
        std::string out;
        bool first = true;
        for (const auto &item : items) {
            if (!first) {
                out += ", ";
            }
            out += item;
            first = false;
        }
        return out;
    }

    static std::string describeFunction(const std::string &name) {
        const auto &requested = g_ctx().requestedFunctions.at(name);
        return name + " [" + joinSet(requested.sections) + "]";
    }

    static std::string formatTypeTextWithName(llvm::StringRef typeText, llvm::StringRef name) {
        std::string out = typeText.str();
        if (!out.empty() && !llvm::StringRef(out).ends_with("*")) {
            out += " ";
        }
        out += name.str();
        return out;
    }

    struct FallbackTypeDecision {
        bool allowed = false;
        bool rewrittenToVoidPtr = false;
        std::string emittedTypeText;
        std::string reason;
    };

    static bool isSafeBuiltinPointerBaseToken(llvm::StringRef token) {
        static const std::set<std::string> safeTokens = {
            "void",      "char",      "short",     "int",       "long",      "float",
            "double",    "_Bool",     "bool",      "size_t",    "ssize_t",   "ptrdiff_t",
            "intptr_t",  "uintptr_t", "int8_t",    "uint8_t",   "int16_t",   "uint16_t",
            "int32_t",   "uint32_t",  "int64_t",   "uint64_t",  "intmax_t",  "uintmax_t",
            "wchar_t",
        };
        return safeTokens.count(token.str()) > 0;
    }

    static bool looksLikeUnresolvedNamedPointerType(llvm::StringRef typeText) {
        std::string normalized;
        normalized.reserve(typeText.size());
        for (char ch : typeText) {
            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
                normalized.push_back(ch);
            } else {
                normalized.push_back(' ');
            }
        }

        std::istringstream iss(normalized);
        std::string token;
        while (iss >> token) {
            if (token == "const" || token == "volatile" || token == "restrict" ||
                token == "__restrict" || token == "__restrict__" || token == "signed" ||
                token == "unsigned" || token == "struct" || token == "union" ||
                token == "enum" || token == "__typeof__") {
                continue;
            }
            return !isSafeBuiltinPointerBaseToken(token);
        }
        return false;
    }

    static FallbackTypeDecision decideFallbackType(const GlobalContext::CapturedType &type,
                                                   const std::set<std::string> &knownHeaders,
                                                   bool isReturnType) {
        FallbackTypeDecision decision;

        bool hasUnknownHeaderDeps = false;
        for (const auto &headerPath : type.headerPaths) {
            if (!knownHeaders.count(headerPath)) {
                hasUnknownHeaderDeps = true;
                break;
            }
        }
        bool hasNonHeaderDeps = !type.nonHeaderPaths.empty();
        bool hasUnresolvedNamedPointer = type.pointerLike && type.headerPaths.empty() &&
                                         type.nonHeaderPaths.empty() &&
                                         looksLikeUnresolvedNamedPointerType(type.originalText);
        bool fullyKnown = !hasUnknownHeaderDeps && !hasNonHeaderDeps;

        if (fullyKnown && !hasUnresolvedNamedPointer) {
            decision.allowed = true;
            decision.emittedTypeText = type.originalText;
            return decision;
        }

        std::string reason;
        if (hasUnknownHeaderDeps) {
            appendReason(reason, "requires types from headers not already included: " +
                                     joinSet(type.headerPaths));
        }
        if (hasNonHeaderDeps) {
            appendReason(reason, "references types only seen in non-header files: " +
                                     joinSet(type.nonHeaderPaths));
        }
        if (hasUnresolvedNamedPointer) {
            appendReason(reason, "pointee type could not be resolved to an included header");
        }

        // A pointer whose pointee we cannot reach is still emittable as an opaque `void *`:
        // the value round-trips even if its declared type is unavailable here.
        if (type.pointerLike) {
            decision.allowed = true;
            decision.rewrittenToVoidPtr = true;
            decision.emittedTypeText = "void *";
            decision.reason = reason;
            return decision;
        }

        decision.allowed = false;
        if (reason.empty()) {
            reason = isReturnType ? "unsupported return type" : "unsupported parameter type";
        }
        decision.reason = reason;
        return decision;
    }

    struct FallbackDeclDecision {
        bool allowed = false;
        std::string declText;
        std::vector<std::string> notes;
        std::vector<std::string> errors;
    };

    static FallbackDeclDecision buildFallbackDeclDecision(const std::string &name,
                                                          const GlobalContext::CapturedFunction &fn,
                                                          const std::set<std::string> &knownHeaders) {
        FallbackDeclDecision out;

        auto returnDecision = decideFallbackType(fn.returnType, knownHeaders, true);
        if (!returnDecision.allowed) {
            out.errors.push_back("return type `" + fn.returnType.originalText + "` cannot be emitted: " +
                                 returnDecision.reason);
            return out;
        }
        if (returnDecision.rewrittenToVoidPtr) {
            out.notes.push_back("return type rewritten: `" + fn.returnType.originalText +
                                "` -> `void *` because " + returnDecision.reason);
        }

        std::ostringstream decl;
        decl << returnDecision.emittedTypeText << " " << name << "(";
        bool first = true;

        for (const auto &param : fn.params) {
            auto paramDecision = decideFallbackType(param.type, knownHeaders, false);
            if (!paramDecision.allowed) {
                out.errors.push_back("parameter `" + param.name + "` of type `" +
                                     param.type.originalText + "` cannot be emitted: " +
                                     paramDecision.reason);
                return out;
            }

            if (!first) {
                decl << ", ";
            }
            decl << formatTypeTextWithName(paramDecision.emittedTypeText, param.name);
            first = false;

            if (paramDecision.rewrittenToVoidPtr) {
                out.notes.push_back("parameter `" + param.name + "` rewritten: `" +
                                    param.type.originalText + "` -> `void *` because " +
                                    paramDecision.reason);
            }
        }

        if (fn.variadic) {
            if (!first) {
                decl << ", ";
            }
            decl << "...";
            first = false;
        }

        if (first) {
            decl << "void";
        }
        decl << ");";

        out.allowed = true;
        out.declText = decl.str();
        return out;
    }

    static void buildOutput() {
        std::map<std::string, std::vector<std::string>> headerToFunctions;
        std::vector<std::string> missingFunctions;
        std::vector<std::string> nonHeaderOnlyFunctions;
        struct FallbackDeclOutput {
            std::string functionName;
            std::string sourcePath;
            std::string sourceLocation;
            std::string declText;
            std::vector<std::string> notes;
            std::vector<std::string> errors;
        };
        std::vector<FallbackDeclOutput> fallbackDecls;

        for (const auto &[name, requested] : g_ctx().requestedFunctions) {
            (void) requested;
            auto resolvedIt = g_ctx().resolvedFunctions.find(name);
            if (resolvedIt == g_ctx().resolvedFunctions.end() || resolvedIt->second.headerPaths.empty()) {
                if (resolvedIt != g_ctx().resolvedFunctions.end() &&
                    !resolvedIt->second.nonHeaderPaths.empty()) {
                    std::string line = describeFunction(name) + " found only in non-header files: " +
                                       joinSet(resolvedIt->second.nonHeaderPaths);
                    nonHeaderOnlyFunctions.push_back(std::move(line));
                } else {
                    missingFunctions.push_back(describeFunction(name) + " missing");
                }
                continue;
            }

            std::string description = describeFunction(name);
            if (!resolvedIt->second.nonHeaderPaths.empty()) {
                description += " (also seen in non-header files)";
            }
            for (const auto &headerPath : resolvedIt->second.headerPaths) {
                headerToFunctions[headerPath].push_back(description);
            }
        }

        std::set<std::string> knownHeaders;
        for (const auto &[headerPath, _] : headerToFunctions) {
            knownHeaders.insert(headerPath);
        }

        for (const auto &[name, requested] : g_ctx().requestedFunctions) {
            (void) requested;
            auto resolvedIt = g_ctx().resolvedFunctions.find(name);
            if (resolvedIt == g_ctx().resolvedFunctions.end() || !resolvedIt->second.headerPaths.empty()) {
                continue;
            }

            auto capturedIt = g_ctx().capturedFunctions.find(name);
            if (capturedIt == g_ctx().capturedFunctions.end()) {
                continue;
            }

            auto decision = buildFallbackDeclDecision(name, capturedIt->second, knownHeaders);
            FallbackDeclOutput output;
            output.functionName = name;
            output.sourcePath = capturedIt->second.sourcePath;
            output.sourceLocation = capturedIt->second.sourceLocation;
            output.notes = std::move(decision.notes);
            output.errors = std::move(decision.errors);
            if (decision.allowed) {
                output.declText = std::move(decision.declText);
            }
            fallbackDecls.push_back(std::move(output));
        }

        llvm::raw_string_ostream out(g_ctx().outBuffer);
        out << "#pragma once\n\n";
        out << "/*\n";
        out << " * Auto-generated by LoreTLC dump.\n";
        out << " * Config: " << g_ctx().configPath << "\n";
        out << " */\n\n";

        if (!missingFunctions.empty()) {
            std::sort(missingFunctions.begin(), missingFunctions.end());
            appendFunctionListComment(out, "Missing configured functions:", missingFunctions);
            out << "\n";
        }

        if (!nonHeaderOnlyFunctions.empty()) {
            std::sort(nonHeaderOnlyFunctions.begin(), nonHeaderOnlyFunctions.end());
            appendFunctionListComment(out, "Configured functions found only in non-header files:",
                                      nonHeaderOnlyFunctions);
            out << "\n";
        }

        for (auto &[headerPath, functions] : headerToFunctions) {
            std::sort(functions.begin(), functions.end());
            appendFunctionListComment(out, "Functions declared in this header:", functions);
            out << "#include \"" << escapeForInclude(headerPath) << "\"\n\n";
        }

        if (!fallbackDecls.empty()) {
            std::sort(fallbackDecls.begin(), fallbackDecls.end(),
                      [](const FallbackDeclOutput &lhs, const FallbackDeclOutput &rhs) {
                          return lhs.functionName < rhs.functionName;
                      });

            out << "/*\n";
            out << " * Fallback declarations synthesized from non-header source files.\n";
            out << " * These are emitted after all includes so they may reuse already included types.\n";
            out << " */\n\n";

            for (const auto &fallback : fallbackDecls) {
                out << "/*\n";
                out << " * Fallback for " << fallback.functionName << "\n";
                if (!fallback.sourcePath.empty()) {
                    out << " * Source: " << fallback.sourcePath << "\n";
                }
                if (!fallback.sourceLocation.empty()) {
                    out << " * Source location: " << fallback.sourceLocation << "\n";
                }
                if (fallback.notes.empty()) {
                    out << " * Rewrites: none\n";
                } else {
                    out << " * Rewrites:\n";
                    for (const auto &note : fallback.notes) {
                        out << " * - " << note << "\n";
                    }
                }
                if (!fallback.errors.empty()) {
                    out << " * Status: cannot synthesize declaration\n";
                    for (const auto &error : fallback.errors) {
                        out << " * - " << error << "\n";
                    }
                } else {
                    out << " * Status: declaration synthesized below\n";
                }
                out << " */\n";

                if (fallback.errors.empty()) {
                    out << fallback.declText << "\n\n";
                } else {
                    out << "\n";
                }
            }
        }
    }

    const char *name = "dump";

    const char *help = "Dump include headers for configured functions";

    int main(int argc, char *argv[]) {
        static cl::OptionCategory myOptionCat("Lorelei TLC - Dump");
        static cl::opt<std::string> configOption("c", cl::desc("Specify thunk config file"),
                                                 cl::value_desc("config file"),
                                                 cl::Required, cl::cat(myOptionCat));
        static cl::opt<std::string> outputOption("o", cl::desc("Specify output file"),
                                                 cl::value_desc("output file"),
                                                 cl::cat(myOptionCat));
        static cl::opt<std::string> buildPathOption("p", cl::desc("Build path"), cl::Required,
                                                    cl::cat(myOptionCat));
        static cl::list<std::string> sourcePathsOption(cl::Positional,
                                                       cl::desc("[source0] [... sourceN]"),
                                                       cl::ZeroOrMore, cl::cat(myOptionCat));

        if (auto err = parseOptions(argc, const_cast<const char **>(argv), myOptionCat); err) {
            llvm::errs() << err;
            return 1;
        }

        if (std::error_code ec = llvm::sys::fs::current_path(g_ctx().initialCwd)) {
            llvm::errs() << "Failed to get current path: " << ec.message() << "\n";
            return 1;
        }

        g_ctx().outputPath = outputOption.getValue();
        g_ctx().configPath = configOption.getValue();
        g_ctx().resolvedFunctions.clear();
        g_ctx().outBuffer.clear();

        auto result = g_ctx().config.load(g_ctx().configPath);
        if (!result.success) {
            llvm::errs() << "Failed to load config file " << g_ctx().configPath << ": "
                         << result.errorMessage;
            if (!result.file.empty()) {
                llvm::errs() << " (" << result.file.string() << ":" << result.line << ")";
            }
            llvm::errs() << "\n";
            return 1;
        }
        loadRequestedFunctions();

        std::unique_ptr<CompilationDatabase> compilations;
        if (std::string err;
            !(compilations = CompilationDatabase::autoDetectFromDirectory(buildPathOption, err))) {
            llvm::errs() << "Error while trying to load a compilation database:\n"
                         << err << "Running without flags.\n";
            return 1;
        }

        std::vector<std::string> sourcePathList;
        if (!sourcePathsOption.empty()) {
            sourcePathList.assign(sourcePathsOption.begin(), sourcePathsOption.end());
        } else {
            // Key by normalized path so each source is scanned once; insert keeps the first
            // (original) Filename seen for that path.
            std::map<std::string, std::string> uniqueSources;
            for (const auto &command : compilations->getAllCompileCommands()) {
                std::string normalized = normalizeCompileCommandPath(command);
                if (!isSupportedSourcePath(normalized)) {
                    continue;
                }
                uniqueSources.emplace(normalized, command.Filename);
            }
            for (const auto &[_, sourcePath] : uniqueSources) {
                sourcePathList.push_back(sourcePath);
            }
        }

        sourcePathList.erase(
            std::remove_if(sourcePathList.begin(), sourcePathList.end(),
                           [](const std::string &sourcePath) {
                               return !isSupportedSourcePath(sourcePath);
                           }),
            sourcePathList.end());

        if (sourcePathList.empty()) {
            llvm::errs() << "No source files were selected for scanning.\n";
            return 1;
        }

        ClangTool tool(*compilations, sourcePathList);
        tool.appendArgumentsAdjuster(
            getInsertArgumentAdjuster("-Wno-pragma-once-outside-header",
                                      tooling::ArgumentInsertPosition::END));
        if (int ret = tool.run(newFrontendActionFactory<MyASTFrontendAction>().get()); ret != 0) {
            return ret;
        }

        buildOutput();

        std::error_code ec;
        llvm::raw_fd_ostream out(g_ctx().outputPath.empty() ? "-" : g_ctx().outputPath, ec);
        if (ec) {
            llvm::errs() << "Error occurs opening output file: " << ec.message() << "\n";
            return 1;
        }
        out << g_ctx().outBuffer;
        return 0;
    }

}
