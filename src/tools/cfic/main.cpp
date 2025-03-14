#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/Lexer.h"

#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

static llvm::cl::OptionCategory MyToolCategory("string-modifier options");

static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

class StringLiteralHandler : public MatchFinder::MatchCallback {
public:
    StringLiteralHandler(Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &Result) {
        if (const StringLiteral *SL = Result.Nodes.getNodeAs<StringLiteral>("stringLiteral")) {
            SourceManager &SM = *Result.SourceManager;
            SourceLocation Loc = SL->getBeginLoc();

            // 如果字符串出现在宏展开中，则获取其在展开后的位置
            if (Loc.isMacroID()) {
                Loc = SM.getSpellingLoc(Loc);
            }

            // 获取字符串文本的范围
            CharSourceRange CharRange = CharSourceRange::getTokenRange(SL->getSourceRange());
            StringRef OrigText = Lexer::getSourceText(CharRange, SM, Result.Context->getLangOpts());

            llvm::SmallVector<char> v;
            printf("XX: %s | %s\n", Lexer::getSpelling(Loc, v, SM, Result.Context->getLangOpts()).str().c_str(),
                   v.data());

            // 确保文本以双引号开始和结束，简单判断
            if (OrigText.starts_with("\"") && OrigText.ends_with("\"")) {
                // 在最后一个双引号之前插入感叹号
                std::string ModifiedText = OrigText.drop_back(1).str() + "!" + "\"";
                Rewrite.ReplaceText(CharRange, ModifiedText);
            }
        }
    }

private:
    Rewriter &Rewrite;
};

class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer(Rewriter &R) : Handler(R) {
        Matcher.addMatcher(stringLiteral().bind("stringLiteral"), &Handler); //
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context); //
    }

private:
    StringLiteralHandler Handler;
    MatchFinder Matcher;
};

class MyFrontendAction : public ASTFrontendAction {
public:
    MyFrontendAction() {}

    void EndSourceFileAction() override {
        SourceManager &SM = TheRewriter.getSourceMgr();
        llvm::errs() << "修改后的文件内容：\n";
        TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<MyASTConsumer>(TheRewriter);
    }

private:
    Rewriter TheRewriter;
};

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 0;
    }
    auto &OptionsParser = ExpectedParser.get();

    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}