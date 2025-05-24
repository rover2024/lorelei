#include <sstream>

#include <clang/AST/AST.h>
#include <clang/AST/Attr.h>

#include "core/pass.h"

#include "common.h"

namespace TLC {

    class Pass_std : public Pass {
    public:
        Pass_std();
        ~Pass_std();

    public:
        bool test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const override;

        bool start(ApiSource &api, const llvm::ArrayRef<std::string> &args) override;
        bool end() override;
    };

    Pass_std::Pass_std() : Pass("std") {
    }

    Pass_std::~Pass_std() {
    }

    bool Pass_std::test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const {
        return false;

    }

    bool Pass_std::start(ApiSource &api, const llvm::ArrayRef<std::string> &args){
        return true;

    }

    bool Pass_std::end() {
        return true;
    }

}

static TLC::Pass_std pass_std;