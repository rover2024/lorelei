#ifndef ASTCONSUMERADDON_H
#define ASTCONSUMERADDON_H

namespace TLC {

    class DocumentContext;

    class ASTConsumerAddOn {
    public:
        virtual ~ASTConsumerAddOn() = default;

        /// Called at \c ASTConsumer::Initialize.
        virtual void initialize(DocumentContext &doc) {};

        /// Called at \c ASTConsumer::HandleTranslationUnit.
        virtual void handleTranslationUnit(DocumentContext &doc) {};
    };

}

#endif // ASTCONSUMERADDON_H
