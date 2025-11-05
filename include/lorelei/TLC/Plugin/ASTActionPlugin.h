#ifndef TLCPLUGIN_H
#define TLCPLUGIN_H

#include <memory>

#include <llvm/ADT/StringRef.h>

#include <lorelei/TLC/Plugin/BasePlugin.h>
#include <lorelei/TLC/AST/ASTConsumerAddOn.h>

namespace clang {
    class CompilerInstance;
}

namespace TLC {

    /// ThunkGenPlugin - A plugin for the TLC ThunkGen command.
    /// \endcode
    class ASTActionPlugin : public BasePlugin {
    public:
        /// Called at \c FrontendAction::BeginSourceFileAction.
        /// \return True to continue processing, false to stop.
        virtual bool beginSourceFileAction(clang::CompilerInstance &CI) {
            return true;
        }

        /// Called at \c FrontendAction::EndSourceFileAction.
        virtual void endSourceFileAction() {};

        /// Called at \c FrontendAction::createASTConsumer.
        /// \return A nullable \c ASTConsumerAddOn object.
        virtual std::unique_ptr<ASTConsumerAddOn>
            createASTConsumerAddOn(clang::CompilerInstance &CI, llvm::StringRef inFile) = 0;
    };

}

#endif // TLCPLUGIN_H
