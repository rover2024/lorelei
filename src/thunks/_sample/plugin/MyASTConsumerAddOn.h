#ifndef MYASTCONSUMERADDON_H
#define MYASTCONSUMERADDON_H

#include <lorelei/TLC/AST/ASTConsumerAddOn.h>

namespace plugin {

    class MyASTConsumerAddOn : public TLC::ASTConsumerAddOn {
    public:
        MyASTConsumerAddOn() {
        }

        void initialize(clang::ASTContext &ctx) override;
        void handleTranslationUnit(clang::ASTContext &ctx) override;
    };

}

#endif // MYASTCONSUMERADDON_H
