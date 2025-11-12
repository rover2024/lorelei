#include <llvm/ADT/StringRef.h>

#include <lorelei/TLC/Plugin/ASTActionPlugin.h>

#include "MyASTConsumerAddOn.h"
#include "MyPass.h"

namespace plugin {

    class MyASTActionPlugin : public TLC::ASTActionPlugin {
    public:
        std::string key() const override {
            return "sample";
        }

        std::unique_ptr<TLC::ASTConsumerAddOn>
            createASTConsumerAddOn(clang::CompilerInstance &CI, llvm::StringRef inFile) override {
            return std::make_unique<MyASTConsumerAddOn>();
        }

    public:
        TLC::PassRegistration<MyPass> pass;
    };

}

TLC_PLUGIN_EXPORT(plugin::MyASTActionPlugin)