#include "MyASTConsumerAddOn.h"

#include <llvm/Support/raw_ostream.h>

namespace plugin {

    void MyASTConsumerAddOn::initialize(TLC::DocumentContext &doc) {
    }

    void MyASTConsumerAddOn::handleTranslationUnit(TLC::DocumentContext &doc) {
    }

}