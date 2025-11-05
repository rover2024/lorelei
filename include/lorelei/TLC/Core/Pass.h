#ifndef PASS_H
#define PASS_H

#include <map>
#include <string>
#include <memory>

#include <clang/AST/ASTContext.h>
#include <llvm/Support/Error.h>

#include <lorelei/TLC/Global.h>
#include <lorelei/TLC/Core/ProcMessage.h>

namespace TLC {

    class DocumentContext;

    class ProcContext;

    class LORELIBTLC_EXPORT Pass {
    public:
        enum Phase {
            Builder,
            Guard,
            Misc,
        };

        virtual ~Pass() = default;

        static std::map<int, Pass *> &passMap(Phase phase);

    public:
        inline Phase phase() const {
            return _phase;
        }

        inline int id() const {
            return _id;
        }

        virtual std::string name() const = 0;

    public:
        /// Called  at \c ASTConsumer::HandleTranslationUnit when the source file action is being
        /// executed.
        /// \param doc The document context.
        virtual void handleTranslationUnit(DocumentContext &doc) {};

        /// Called after the source file action is executed, before any pass is executed for the
        /// document.
        /// \param doc The document context.
        virtual void beginProcessDocument(DocumentContext &doc) {};

        /// Called after the source file action is executed, after all passes are executed for the
        /// document.
        /// \param doc The document context.
        virtual void endProcessDocument(DocumentContext &doc) {};

        /// Test if the pass should be executed for the given proc.
        /// \param proc The proc context.
        /// \param msg The proc message to be generated, will be used when the pass is executed.
        /// \return True if the pass should be executed, false otherwise.
        virtual bool testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) const {
            return false;
        }

        /// Execute the pass for the given proc.
        /// \param proc The proc context.
        /// \param msg The proc message generated at the test phase.
        /// \return An error if the pass fails, \c llvm::Error::success() otherwise.
        virtual llvm::Error beginHandleProc(ProcContext &proc,
                                            std::unique_ptr<ProcMessage> &msg) = 0;

        /// End executing the pass for the given proc.
        /// \param proc The proc context.
        /// \param msg The proc message generated at the test phase.
        /// \return An error if the pass fails, \c llvm::Error::success() otherwise.
        virtual llvm::Error endHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) = 0;

    protected:
        inline Pass(Phase phase, int id) : _phase(phase), _id(id) {
        }

        Phase _phase;
        int _id;
    };

    /// PassRegistration - Helper class for registering passes in the TLC pipeline.
    /// \tparam T The pass type to be registered.
    /// \note A builtin pass is registered as a static global instance, while a plugin pass should
    /// be registered as a non-static member of the plugin instance.
    template <class T>
    class PassRegistration {
        static_assert(std::is_base_of<Pass, T>::value, "T must be a subclass of Pass");

    public:
        template <class... Args>
        PassRegistration(Args &&...args) : instance(std::forward<Args>(args)...) {
            Pass::passMap(instance.phase()).insert(std::make_pair(instance.id(), &instance));
        }

        ~PassRegistration() {
            Pass::passMap(instance.phase()).erase(instance.id());
        }

        T instance;
    };

}

#endif // PASS_H
