#ifndef PASS_H
#define PASS_H

#include <map>

#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Error.h>
#include <clang/AST/Type.h>
#include <clang/AST/Decl.h>

#include "intorstring.h"

namespace TLC {

    class ThunkDefinition;

    /// Base class for all passes in the TLC pipeline.
    class Pass {
    public:
        enum Stage {
            Builder,
            EntryExit,
            Decoration,
        };

        virtual ~Pass() = default;

        static std::map<std::string, Pass *> &passMap(Stage stage);

    public:
        inline Stage stage() const {
            return _stage;
        }
        inline int type() const {
            return _type;
        }
        inline const std::string &id() const {
            return _id;
        }

    public:
        /// Test the ThunkDefinition to see if it is valid for this pass.
        /// \param td The ThunkDefinition to test.
        /// \param args The pass arguments for further processing.
        virtual bool test(const ThunkDefinition &thunk,
                          llvm::SmallVectorImpl<IntOrString> &args) const = 0;

        /// Run the pass on the ThunkDefinition.
        /// \param td The ThunkDefinition to run the pass on.
        /// \param args The pass arguments from testing or hint declaration.
        virtual llvm::Error begin(ThunkDefinition &thunk,
                                  const llvm::SmallVectorImpl<IntOrString> &args) = 0;

        /// End the pass on the ThunkDefinition.
        /// \param td The ThunkDefinition to end the pass on.
        virtual llvm::Error end(ThunkDefinition &thunk) = 0;

    protected:
        Pass(Stage stage, int type, std::string id)
            : _stage(stage), _type(type), _id(std::move(id)) {
        }

        Stage _stage;
        int _type;
        std::string _id;
    };


    /// Base class for all builder passes in the TLC pipeline, for implementing the skeleton of the
    /// thunk function with the correct signature and body.
    class BuilderPass : public Pass {
    public:
        enum Type {
            PT_Standard,
            PT_LibcPrintf,
            PT_LibcVPrintf,
            PT_LibcScanf,
            PT_LibcVScanf,
        };
        BuilderPass(Type type, std::string id) : Pass(Stage::Builder, type, std::move(id)) {
        }

    public:
        llvm::Error end(ThunkDefinition &td) override {
            return llvm::Error::success();
        }
    };


    /// Base class for all thunk passes in the TLC pipeline, for implementing core thunk procedures
    /// in the entry and exit code for the thunk function.
    class EntryExitPass : public Pass {
    public:
        enum Type {
            PT_CallbackSubstituter,
        };
        EntryExitPass(Type type, std::string id) : Pass(Stage::EntryExit, type, std::move(id)) {
        }
    };


    /// Base class for all decoration passes in the TLC pipeline, for implementing additional
    /// decoration procedures for the thunk function.
    class DecorationPass : public Pass {
    public:
        enum Type {
            PT_GetProcAddress,
        };
        DecorationPass(Type type, std::string id) : Pass(Stage::Decoration, type, std::move(id)) {
        }
    };


    /// Helper class for registering passes in the TLC pipeline.
    template <class T>
    class PassRegistration {
        static_assert(std::is_base_of<Pass, T>::value, "T must be a subclass of Pass");

    public:
        template <class... Args>
        PassRegistration(Args &&...args) {
            static T instance(std::forward<Args>(args)...);
            Pass::passMap(instance.stage()).insert(std::make_pair(instance.id(), &instance));
        }
    };

}

#endif // PASS_H