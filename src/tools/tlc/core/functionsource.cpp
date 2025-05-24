#include "functionsource.h"

#include <llvm/ADT/StringExtras.h>

#include "apisource.h"
#include "common.h"

using namespace clang;

namespace TLC {

    class FunctionSource::Impl {
    public:
        Impl(FunctionSource *decl, Type T, ApiSource *Parent, StringRef Name, const FunctionDecl *FunctionDecl)
            : _decl(decl), FunctionType(T), Parent(Parent), Name(Name), FD(FunctionDecl) {
            if (FD) {
                ReturnValue = {getTypeString(FD->getReturnType()), {}};

                int i = 0;
                for (const auto &Arg : FD->parameters()) {
                    Arguments.push_back({getTypeString(Arg->getType()), {}, {}});
                }

                if (FD->isVariadic())
                    Variadic = true;
            }
        }

        FunctionSource *_decl;

        Type FunctionType;
        ApiSource *Parent;

        // Metadata
        std::string Name;
        const FunctionDecl *FD;

        // Function Data
        SmallVector<FunctionSource::Param, 6> Arguments;
        FunctionSource::Param ReturnValue;
        bool Variadic = false;

        // Texts
        std::string Prolog;
        std::string Body;
        std::string Epilog;
        std::list<std::string> ForwardSources;
        std::list<std::string> BackwardSources;

        std::string getFunctionSignature(StringRef name = {}) const {
            std::string FuncSignature;
            FuncSignature += "__typeof__(" + (FD ? getTypeString(FD->getReturnType()) : ReturnValue.TypeName) + ")";
            SmallVector<std::string> ArgTexts;
            {
                int i = 0;
                for (const auto &Arg : Arguments) {
                    ++i;
                    std::string Text =
                        "__typeof__(" + (Arg.MetadataTypeName.empty() ? Arg.TypeName : Arg.MetadataTypeName) + ")";
                    Text += " ";
                    if (Arg.Name.empty())
                        Text += "arg" + std::to_string(i);
                    else
                        Text += Arg.Name;
                    ArgTexts.emplace_back(Text);
                }
            }
            if (Variadic) {
                ArgTexts.emplace_back("...");
            }
            FuncSignature += " ";
            FuncSignature += (name.empty() ? Name : name.str()) + "(" + llvm::join(ArgTexts, ", ") + ")";
            return FuncSignature;
        }

        std::string getContent() const {
            std::array Sources{
                std::string("    // Prologue"),
                Prolog,
                std::string("    // Forward Codes"),
                llvm::join(ForwardSources, "\n"),
                std::string("    // Body"),
                Body, //
                std::string("    // Backward Codes"),
                llvm::join(BackwardSources, "\n"),
                std::string("    // Epilogue"),
                Epilog,
            };
            return llvm::join(Sources, "\n");
        }

        std::string getText() const {
            return "static " + getFunctionSignature() + " {\n" + getContent() + "\n}\n";
        }
    };

    FunctionSource::FunctionSource(Type T, ApiSource *Parent, StringRef Name, const FunctionDecl *FunctionDecl)
        : _impl(std::make_unique<Impl>(this, T, Parent, Name, FunctionDecl)) {
    }

    FunctionSource::~FunctionSource() = default;

    FunctionSource::Type FunctionSource::getFunctionType() const {
        return _impl->FunctionType;
    }

    ApiSource *FunctionSource::getParent() const {
        return _impl->Parent;
    }

    StringRef FunctionSource::getName() const {
        return _impl->Name;
    }

    const FunctionDecl *FunctionSource::getFunctionDecl() const {
        return _impl->FD;
    }

    ArrayRef<FunctionSource::Param> FunctionSource::getArguments() const {
        return _impl->Arguments;
    }

    void FunctionSource::setArguments(const ArrayRef<Param> &args) {
        _impl->Arguments = {args.begin(), args.end()};
    }

    const FunctionSource::Param &FunctionSource::getReturnValue() const {
        return _impl->ReturnValue;
    }

    void FunctionSource::setReturnValue(const Param &returnValue) {
        _impl->ReturnValue = returnValue;
    }

    bool FunctionSource::isVariadic() const {
        return _impl->Variadic;
    }

    void FunctionSource::setVariadic(bool variadic) const {
        _impl->Variadic = variadic;
    }

    std::string FunctionSource::getFunctionSignature(llvm::StringRef name) const {
        return _impl->getFunctionSignature(name);
    }

    StringRef FunctionSource::getProlog() const {
        return _impl->Prolog;
    }

    void FunctionSource::setProlog(StringRef lines) {
        _impl->Prolog = lines;
    }

    StringRef FunctionSource::getEpilog() const {
        return _impl->Epilog;
    }

    void FunctionSource::setEpilog(StringRef lines) {
        _impl->Epilog = lines;
    }

    StringRef FunctionSource::getBody() const {
        return _impl->Body;
    }

    void FunctionSource::setBody(StringRef lines) {
        _impl->Body = lines;
    }

    int FunctionSource::forwardCount() const {
        return _impl->ForwardSources.size();
    }

    void FunctionSource::prependForward(StringRef lines) {
        _impl->ForwardSources.push_front((lines.ends_with("\n") ? lines.drop_back() : lines).str());
    }

    void FunctionSource::appendForward(StringRef lines) {
        _impl->ForwardSources.push_back((lines.ends_with("\n") ? lines.drop_back() : lines).str());
    }

    int FunctionSource::backwardCount() const {
        return _impl->BackwardSources.size();
    }

    void FunctionSource::prependBackward(StringRef lines) {
        _impl->BackwardSources.push_front((lines.ends_with("\n") ? lines.drop_back() : lines).str());
    }

    void FunctionSource::appendBackward(StringRef lines) {
        _impl->BackwardSources.push_back((lines.ends_with("\n") ? lines.drop_back() : lines).str());
    }

    bool FunctionSource::isEmpty() const {
        return _impl->Prolog.empty() && _impl->ForwardSources.empty() && _impl->Body.empty() &&
               _impl->BackwardSources.empty() && _impl->Epilog.empty();
    }

    std::string FunctionSource::getContent() const {
        return _impl->getContent();
    }

    std::string FunctionSource::getText() const {
        return _impl->getText();
    }

}