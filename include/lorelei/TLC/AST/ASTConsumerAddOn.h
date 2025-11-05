#ifndef ASTCONSUMERADDON_H
#define ASTCONSUMERADDON_H

namespace clang {
    class ASTContext;
}

namespace TLC {

    class ASTConsumerAddOn {
    public:
        virtual ~ASTConsumerAddOn() = default;

        /// Called at \c ASTConsumer::Initialize.
        virtual void initialize(clang::ASTContext &ctx) {};

        /// Called at \c ASTConsumer::HandleTranslationUnit.
        virtual void handleTranslationUnit(clang::ASTContext &ctx) {};
    };

}

#endif // ASTCONSUMERADDON_H
