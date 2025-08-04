#ifndef THUNKDEFINITION_H
#define THUNKDEFINITION_H

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/ImmutableList.h>

#include "functiondefinition.h"
#include "intorstring.h"

namespace TLC {

    class Analyzer;

    class ThunkDefinition {
    public:
        enum Type {
            /// A thunk that calls a host function
            GuestFunctionThunk,

            /// A thunk that calls a guest function
            GuestCallbackThunk,

            /// A thunk that calls a host function
            HostCallbackThunk,

            /// \internal
            NumTypes,
        };

        ThunkDefinition(Type type, Analyzer *parent, std::string name, clang::QualType QT,
                        std::array<FunctionDeclRep, FunctionDefinition::NumTypes> reps,
                        const clang::FunctionDecl *hint);
        ~ThunkDefinition() = default;

    public:
        Type type() const {
            return _type;
        }
        Analyzer *parent() const {
            return _parent;
        }
        const std::string &name() const {
            return _name;
        }
        clang::QualType qualType() const {
            return _qualType;
        }
        FunctionTypeView view() const {
            return _qualType;
        }
        FunctionDefinition &function(FunctionDefinition::Type type) {
            return _fds[type];
        }
        const FunctionDefinition &function(FunctionDefinition::Type type) const {
            return _fds[type];
        }
        const clang::FunctionDecl *hint() const {
            return _hint;
        }
        const clang::FunctionDecl *declOrHint() const {
            if (_type == GuestFunctionThunk) {
                return _fds[FunctionDefinition::GTP].rep().decl();
            }
            return _hint;
        }

        struct Annotation {
            std::string name;
            llvm::SmallVector<IntOrString> arguments;
        };
        llvm::ArrayRef<Annotation> annotations() const {
            return _annotations;
        }

        std::string text(bool guest) const;

        const IntOrString &attr(llvm::StringRef key) const {
            auto it = _attrs.find(key);
            if (it == _attrs.end()) {
                static IntOrString empty;
                return empty;
            }
            return it->second;
        }
        void setAttr(llvm::StringRef key, const IntOrString &value) {
            _attrs[key] = value;
        }

    public:
        /// The following methods returns a reference to the source lines, each element is the line
        /// ID and the source code.
        auto &sourceForward(bool guest) {
            return _sources[!guest][0];
        }
        auto &sourceBackward(bool guest) {
            return _sources[!guest][1];
        }

        /// Const version of the above methods.
        const auto &sourceForward(bool guest) const {
            return _sources[!guest][0];
        }
        const auto &sourceBackward(bool guest) const {
            return _sources[!guest][1];
        }

    protected:
        Type _type;
        Analyzer *_parent;
        std::string _name;
        clang::QualType _qualType;
        std::array<FunctionDefinition, FunctionDefinition::NumTypes> _fds;
        const clang::FunctionDecl *_hint;
        std::array<std::array<SourceLines<std::list>, 2>, 2>
            _sources; // array[ guest ? 0 : 1, array[ forward ? 0 : 1, lines ] ]
        llvm::SmallVector<Annotation> _annotations;
        llvm::StringMap<IntOrString> _attrs;
    };

#define LIST_FDS(td)                                                                               \
    auto &GTP = (td).function(FunctionDefinition::GTP);                                            \
    auto &GTP_IMPL = (td).function(FunctionDefinition::GTP_IMPL);                                  \
    auto &HTP = (td).function(FunctionDefinition::HTP);                                            \
    auto &HTP_IMPL = (td).function(FunctionDefinition::HTP_IMPL);

}

#endif // THUNKDEFINITION_H