#ifndef FUNCTIONDEFINITION_H
#define FUNCTIONDEFINITION_H

#include <list>

#include <llvm/ADT/SmallVector.h>
#include <clang/AST/AST.h>

#include "functiondeclrep.h"
#include "sourcelines.h"

namespace TLC {

    class ThunkDefinition;

    class FunctionDefinition {
    public:
        enum Type {
            /// Guest thunk procedure
            /// \code
            ///     ReturnValue __GTP_XXX(Param1 arg1, Param2 arg2, ...) {
            ///         ReturnValue ret;
            ///         ret = ___GTP_XXX(arg1, arg2, ...);
            ///         return ret;
            ///     }
            /// \endcode
            GTP,

            /// Guest thunk implementation procedure
            /// \code
            ///     ReturnValue ___GTP_XXX(Param1 arg1, Param2 arg2, ...) {
            ///         // Invoke host here
            ///     }
            /// \endcode
            GTP_IMPL,

            /// Host thunk procedure
            /// \code
            ///     ReturnValue __HTP_XXX(Param1 arg1, Param2 arg2, ...) {
            ///         ReturnValue ret;
            ///         ret = ___HTP_XXX(arg1, arg2, ...);
            ///         return ret;
            ///     }
            /// \endcode
            HTP,

            /// Host thunk implementation procedure
            /// \code
            ///     ReturnValue ___HTP_XXX(Param1 arg1, Param2 arg2, ...) {
            ///         // Invoke guest here
            ///     }
            /// \endcode
            HTP_IMPL,

            /// \internal
            NumTypes,
        };

        FunctionDefinition(Type type, ThunkDefinition *parent, FunctionDeclRep rep)
            : _type(type), _parent(parent), _rep(std::move(rep)) {
        }
        ~FunctionDefinition() = default;

        inline Type type() const {
            return _type;
        }
        inline ThunkDefinition *parent() const {
            return _parent;
        }
        inline const FunctionDeclRep &rep() const {
            return _rep;
        }
        void setRep(FunctionDeclRep rep) {
            _rep = std::move(rep);
        }

        /// Returns the function signature as a string with the name replaced by the given \a name.
        std::string declText(llvm::StringRef name) const;

        /// Returns the content of the function definition, excluding the function signature.
        std::string content() const;

        /// Returns the text of the function definition, including the function signature and the
        /// function definition content.
        std::string text() const;

        /// The following methods returns a reference to the source lines, each element is the line
        /// ID and the source code.
        ///
        /// The source lines are stored in the following order:
        /// \code
        ///     ReturnValue foo(Param1 arg1, Param2 arg2, ...) {
        ///         // Prolog
        ///         ReturnValue ret;
        ///         entryThunk(&arg1, &arg2, ...);
        ///
        ///         // Body forward
        ///         initialize(...);
        ///
        ///         // Body
        ///         ret = fooImpl(arg1, arg2, ...);
        ///
        ///         // Body backward
        ///         finalize(&ret, ...);
        ///
        ///         // Epilog
        ///         exitThunk(&arg1, &arg2, ...);
        ///         return ret;
        ///     }
        /// \endcode
        inline auto &prolog() {
            return _sources[0];
        }
        inline auto &epilog() {
            return _sources[1];
        }
        inline auto &body() {
            return _sources[2];
        }
        inline auto &bodyForward() {
            return _sources[3];
        }
        inline auto &bodyBackward() {
            return _sources[4];
        }

        /// Const version of the above methods.
        inline const auto &prolog() const {
            return _sources[0];
        }
        inline const auto &epilog() const {
            return _sources[1];
        }
        inline const auto &body() const {
            return _sources[2];
        }
        inline const auto &bodyForward() const {
            return _sources[3];
        }
        inline const auto &bodyBackward() const {
            return _sources[4];
        }

    protected:
        Type _type;
        ThunkDefinition *_parent;
        FunctionDeclRep _rep;
        std::array<SourceLines<std::list>, 5> _sources;
    };

}

#endif // FUNCTIONDEFINITION_H