#include "ASTMetaContext.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "CommonMatchFinder.h"
#include "TypeExtras.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace TLC {

    template <class T>
    static bool isCDecl(const T *decl) {
        const DeclContext *DC = decl->getDeclContext();
        if (const LinkageSpecDecl *LSD = dyn_cast<LinkageSpecDecl>(DC)) {
            return LSD->getLanguage() == LinkageSpecLanguageIDs::C;
        }

        const LangOptions &LangOpts = decl->getASTContext().getLangOpts();
        if (!LangOpts.CPlusPlus) {
            return true;
        }
        return false;
    }

    template <class T>
    static bool isCLinkage(const T *decl) {
        if (decl->getLanguageLinkage() == CLanguageLinkage) {
            return true;
        }
        return isCDecl(decl);
    }

    void ASTMetaContext::handleTranslationUnit(clang::ASTContext &ast) {
        _ast = &ast;

        const auto &matchCallback = [this](const MatchFinder::MatchResult &Result) {
            if (auto decl = Result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("metaConfig")) {
                MetaConfigItem item(decl);
                if (item.isValid()) {
                    _metaConfigs[item.scope()] = item;
                }
            }
            if (auto decl =
                    Result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("metaProcDesc")) {
                MetaProcDescItem item(decl);
                if (item.isValid()) {
                    _metaProcDescs[item.procDecl()->getName().str()] = item;
                }
            }
            if (auto decl =
                    Result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("metaProcCBDesc")) {
                MetaProcCBDescItem item(decl);
                if (item.isValid()) {
                    _metaProcCBDescs[getTypeString(item.procType().getCanonicalType())] = item;
                }
            }
            if (auto decl = Result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("metaProc")) {
                MetaProcItem item(decl);
                if (item.isValid()) {
                    _metaProcs[item.procKind()][item.thunkPhase()]
                              [item.procDecl()->getNameAsString()] = item;
                }

                if (!_metaProcDecl) {
                    _metaProcDecl = decl->getSpecializedTemplate();
                }
            }
            if (auto decl = Result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("metaProcCB")) {
                MetaProcCBItem item(decl);
                if (item.isValid()) {
                    _metaProcCBs[item.procKind()][item.thunkPhase()]
                                [getTypeString(item.procType().getCanonicalType())] = item;
                }
            }
            if (auto decl = Result.Nodes.getNodeAs<FunctionDecl>("functionDecl")) {
                if (isCLinkage(decl)) {
                    _functions[decl->getNameAsString()] = decl;
                }
            }
            if (auto decl = Result.Nodes.getNodeAs<VarDecl>("varDecl")) {
                if (isCLinkage(decl)) {
                    _vars[decl->getNameAsString()] = decl;
                }
            }
            if (auto decl = Result.Nodes.getNodeAs<TypedefDecl>("typedefDecl")) {
                if (isCDecl(decl)) {
                    if (decl->getUnderlyingType()->isFunctionPointerType()) {
                        _functionPointerTypedefs[decl->getNameAsString()] = decl;
                    }
                }
            }
        };

        CommonMatchFinder matchHandler(matchCallback);
        MatchFinder finder;

        /// STEP: Match \c MetaConfig
        finder.addMatcher(                   //
            classTemplateSpecializationDecl( //
                hasName("MetaConfig")        //
                )
                .bind("metaConfig"),
            &matchHandler);

        /// STEP: Match \c MetaProcDesc
        finder.addMatcher(                   //
            classTemplateSpecializationDecl( //
                hasName("MetaProcDesc")      //
                )
                .bind("metaProcDesc"),
            &matchHandler);

        /// STEP: Match \c MetaProcCBDesc
        finder.addMatcher(                   //
            classTemplateSpecializationDecl( //
                hasName("MetaProcCBDesc")    //
                )
                .bind("metaProcCBDesc"),
            &matchHandler);

        /// STEP: Match \c MetaProc
        finder.addMatcher(                   //
            classTemplateSpecializationDecl( //
                hasName("MetaProc")          //
                )
                .bind("metaProc"),
            &matchHandler);

        /// STEP: Match \c MetaProcCB
        finder.addMatcher(                   //
            classTemplateSpecializationDecl( //
                hasName("MetaProcCB")        //
                )
                .bind("metaProcCB"),
            &matchHandler);

        /// STEP: Match \c FunctionDecl, \c VarDecl and \c TypedefDecl
        finder.addMatcher(functionDecl().bind("functionDecl"), &matchHandler);
        finder.addMatcher(varDecl().bind("varDecl"), &matchHandler);
        finder.addMatcher(typedefDecl().bind("typedefDecl"), &matchHandler);
        finder.matchAST(ast);

        if (!_metaProcDecl) {
            llvm::errs() << "error: \"MetaProc\" declaration not found.\n";
            std::exit(1);
        }
    }

}