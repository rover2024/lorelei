#ifndef MYASTCONSUMERADDON_H
#define MYASTCONSUMERADDON_H

#include <lorelei/TLC/AST/ASTConsumerAddOn.h>

namespace plugin {

    class MyASTConsumerAddOn : public TLC::ASTConsumerAddOn {
    public:
        MyASTConsumerAddOn() {
        }

        void initialize(TLC::DocumentContext &doc) override;
        void handleTranslationUnit(TLC::DocumentContext &doc) override;
    };

}

#endif // MYASTCONSUMERADDON_H
