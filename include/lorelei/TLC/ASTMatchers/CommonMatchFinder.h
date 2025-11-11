#ifndef COMMONMATCHFINDER_H
#define COMMONMATCHFINDER_H

#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace TLC {

    class CommonMatchFinder : public clang::ast_matchers::MatchFinder::MatchCallback {
    public:
        explicit CommonMatchFinder(
            std::function<void(const clang::ast_matchers::MatchFinder::MatchResult &)> callback)
            : _callback(callback) {
        }

        void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override {
            _callback(Result);
        }

    private:
        std::function<void(const clang::ast_matchers::MatchFinder::MatchResult &)> _callback;
    };

}

#endif // COMMONMATCHFINDER_H
