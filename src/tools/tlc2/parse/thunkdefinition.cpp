#include "thunkdefinition.h"

#include <charconv>

#include <clang/AST/Attr.h>

#include "common.h"

using namespace clang;
using namespace llvm;

namespace TLC {

    ThunkDefinition::ThunkDefinition(Type type, Analyzer *parent, std::string name,
                                     clang::QualType QT,
                                     std::array<FunctionDeclRep, FunctionDefinition::NumTypes> reps,
                                     const clang::FunctionDecl *hint)
        : _type(type), _parent(parent), _name(std::move(name)), _qualType(std::move(QT)),
          _fds({
              FunctionDefinition(FunctionDefinition::GTP, this,
                                 std::move(reps[FunctionDefinition::GTP])),
              FunctionDefinition(FunctionDefinition::GTP_IMPL, this,
                                 std::move(reps[FunctionDefinition::GTP_IMPL])),
              FunctionDefinition(FunctionDefinition::HTP, this,
                                 std::move(reps[FunctionDefinition::HTP])),
              FunctionDefinition(FunctionDefinition::HTP_IMPL, this,
                                 std::move(reps[FunctionDefinition::HTP_IMPL])),
          }),
          _hint(hint) //
    {
        /// STEP: Parse pass annotations
        if (hint) {
            SmallVector<StringRef> annotated;
            if (auto attr = hint->getAttr<clang::AnnotateAttr>(); attr) {
                attr->getAnnotation().split(annotated, ';', -1, false);
            }
            for (auto theAttr : std::as_const(annotated)) {
                // Pass annotations must start with "@", e.g.: @foo:1,2
                if (!theAttr.starts_with("@")) {
                    continue;
                }
                theAttr = theAttr.substr(1);

                Annotation anno;
                auto colonIndex = theAttr.find(':');
                if (colonIndex != StringRef::npos) {
                    anno.name = theAttr.substr(0, colonIndex);

                    StringRef passArgsStr = theAttr.substr(colonIndex + 1);
                    SmallVector<StringRef> passArgs;
                    passArgsStr.split(passArgs, ',');
                    for (auto arg : std::as_const(passArgs)) {
                        anno.arguments.push_back(IntOrString::fromAnyString(arg.str()));
                    }
                } else {
                    anno.name = theAttr;
                }
                _annotations.push_back(std::move(anno));
            }
        }
    }

    std::string ThunkDefinition::text(bool guest, bool decl) const {
        auto &TP = _fds[guest ? FunctionDefinition::GTP : FunctionDefinition::HTP];
        auto &TP_IMPL = _fds[guest ? FunctionDefinition::GTP_IMPL : FunctionDefinition::HTP_IMPL];
        if (decl) {
            return "static " + TP_IMPL.declText({}) + ";\n" + "static " + TP.declText({}) + ";";
        }
        std::array Sources{
            sourceForward(guest).toRawText(),
            TP_IMPL.rep().decl() ? std::string() : TP_IMPL.text(),
            std::string(),
            TP.rep().decl() ? std::string() : TP.text(),
            sourceBackward(guest).toRawText(),
        };
        return llvm::join(Sources, "\n");
    }

}