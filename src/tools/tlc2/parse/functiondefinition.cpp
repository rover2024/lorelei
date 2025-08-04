#include "functiondefinition.h"

#include <llvm/ADT/StringExtras.h>

#include "common.h"

using namespace clang;
using namespace llvm;

namespace TLC {

    std::string FunctionDefinition::declText(StringRef name) const {
        auto &strRep = _rep.typeRep();

        std::string result;
        result += "__typeof__(" + strRep.returnType().getAsString() + ")";
        SmallVector<std::string> argTexts;
        {
            int i = 0;
            for (const auto &argType : strRep.argTypes()) {
                auto name = _rep.argName(i);
                if (name.empty()) {
                    name = "arg" + std::to_string(i + 1);
                }
                argTexts.emplace_back("__typeof__(" + argType.getAsString() + ") " + name);
                i++;
            }
        }
        if (strRep.isVariadic()) {
            argTexts.emplace_back("...");
        }
        result += " ";
        result += (name.empty() ? _rep.name() : name.str());
        result += "(" + llvm::join(argTexts, ", ") + ")";
        return result;
    }

    std::string FunctionDefinition::content() const {
        std::array Sources{
            std::string("    // Prologue"),       prolog().toRawText(),
            std::string("    // Forward Codes"),  bodyForward().toRawText(),
            std::string("    // Body"),           body().toRawText(),
            std::string("    // Backward Codes"), bodyBackward().toRawText(),
            std::string("    // Epilogue"),       epilog().toRawText(),
        };
        return llvm::join(Sources, "\n");
    }

    std::string FunctionDefinition::text() const {
        return "static " + declText({}) + " {\n" + content() + "\n}";
    }

}