// SPDX-License-Identifier: MIT

#ifndef LORE_CLANGEXTRAS_COMMONMATCHFINDER_H
#define LORE_CLANGEXTRAS_COMMONMATCHFINDER_H

#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace lore::tool {

    /// CommonMatchFinder - An AST MatchFinder callback that forwards every match to a
    /// \c std::function.
    ///
    /// Lets a call site hand in a lambda instead of subclassing MatchCallback for each matcher.
    class CommonMatchFinder : public clang::ast_matchers::MatchFinder::MatchCallback {
    public:
        explicit CommonMatchFinder(
            std::function<void(const clang::ast_matchers::MatchFinder::MatchResult &)> &&callback)
            : m_callback(std::move(callback)) {
        }

        void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override {
            m_callback(Result);
        }

    private:
        std::function<void(const clang::ast_matchers::MatchFinder::MatchResult &)> m_callback;
    };

}

#endif // LORE_CLANGEXTRAS_COMMONMATCHFINDER_H
