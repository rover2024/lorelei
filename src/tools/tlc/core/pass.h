#ifndef PASS_H
#define PASS_H

#include <map>

#include <clang/AST/AST.h>

#include "core/apisource.h"

namespace TLC {

    class Pass {
    public:
        virtual ~Pass();

    public:
        enum Priority {
            NoPriority = 0,
            BeginPriority = 1,
            MiddlePriority = 500,
            EndPriority = 1000,
        };

        inline clang::StringRef identifier() const { return m_identifier; }

        virtual bool test(const ApiSource &api, llvm::SmallVectorImpl<std::string> *args, int *priority) const = 0;
        virtual bool start(ApiSource &api, const llvm::ArrayRef<std::string> &args) = 0;
        virtual bool end() = 0;

    public:
        static const std::map<std::string, Pass *> &globalPassMap();

    protected:
        explicit Pass(const std::string &identifier);

    protected:
        std::string m_identifier;
    };

}

#endif // PASS_H