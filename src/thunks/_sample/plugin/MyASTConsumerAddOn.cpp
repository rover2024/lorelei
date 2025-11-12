#include "MyASTConsumerAddOn.h"

#include <llvm/Support/raw_ostream.h>

namespace plugin {

    void MyASTConsumerAddOn::initialize(clang::ASTContext &ctx) {
    }

    void MyASTConsumerAddOn::handleTranslationUnit(clang::ASTContext &ctx) {
    }

}