#include "apisource.h"

#include <clang/AST/Attr.h>

#include <llvm/ADT/StringExtras.h>

#include "common.h"

using namespace clang;

namespace TLC {

    class ApiSource::Impl {
    public:
        Impl(ApiSource *decl, Type T, PassContext *Parent, llvm::StringRef Name, const QualType &QT,
             const ArrayRef<const FunctionDecl *> &FDs, const FunctionDecl *OrgFunctionDecl,
             const FunctionDecl *HintFunctionDecl)
            : _decl(decl), ApiType(T), Parent(Parent), Name(Name), QT(QT), OrgFunctionDecl(OrgFunctionDecl),
              HintFunctionDecl(HintFunctionDecl) {

            // Copy function decls
            for (int i = 0; i < std::min(FDs.size(), FunctionDecls.size()); ++i) {
                FunctionDecls[i] = FDs[i];
            }

            // Get types
            if (QT->isFunctionNoProtoType()) {
                auto *FT = QT->getAs<FunctionProtoType>();
                ApiReturnType = FT->getReturnType();
            } else if (QT->isFunctionProtoType()) {
                auto *FT = QT->getAs<FunctionProtoType>();
                ApiReturnType = FT->getReturnType();
                for (int i = 0; i < FT->getNumParams(); ++i) {
                    ApiArgumentTypes.push_back(FT->getParamType(i));
                }
                if (FT->isVariadic()) {
                    ApiVariadic = true;
                }
            } else {
                // unreachable
                assert(false);
            }

            // Extract annotations from hint function
            if (HintFunctionDecl) {
                SmallVector<StringRef> AnnotatedPasses;
                if (auto Attr = HintFunctionDecl->getAttr<clang::AnnotateAttr>(); Attr) {
                    Attr->getAnnotation().split(AnnotatedPasses, ';', -1, false);
                }
                for (auto TheAnno : std::as_const(AnnotatedPasses)) {
                    // Pass annotations must start with "@", e.g.: @foo:1,2
                    if (!TheAnno.starts_with("@")) {
                        continue;
                    }
                    TheAnno = TheAnno.substr(1);

                    AnnotationImpl Anno;
                    auto colonIndex = TheAnno.find(':');
                    if (colonIndex != StringRef::npos) {
                        Anno.Name = TheAnno.substr(0, colonIndex);

                        StringRef PassArgsStr = TheAnno.substr(colonIndex + 1);
                        SmallVector<StringRef> PassArgs;
                        PassArgsStr.split(PassArgs, ',');
                        for (auto Arg : std::as_const(PassArgs)) {
                            Anno.Arguments.push_back(Arg.str());
                        }
                    } else {
                        Anno.Name = TheAnno;
                    }
                    HintAnnotations.push_back(std::move(Anno));
                }
            }

            std::string NamePrefix;
            switch (T) {
                case GuestCallback: {
                    NamePrefix = "GCB_";
                    break;
                }
                case HostCallback: {
                    NamePrefix = "HCB_";
                    break;
                }
                default:
                    break;
            }

            // Create children
            Functions[FunctionSource::GTP].Source = std::make_shared<FunctionSource>( //
                FunctionSource::GTP,                                                  //
                _decl,                                                                //
                "__GTP_" + NamePrefix + Name.str(),                                   //
                FunctionDecls[FunctionSource::GTP]                                    //
            );
            Functions[FunctionSource::GTP_IMPL].Source = std::make_shared<FunctionSource>( //
                FunctionSource::GTP_IMPL,                                                  //
                _decl,                                                                     //
                "___GTP_" + NamePrefix + Name.str(),                                       //
                FunctionDecls[FunctionSource::GTP_IMPL]                                    //
            );
            Functions[FunctionSource::HTP].Source = std::make_shared<FunctionSource>( //
                FunctionSource::HTP,                                                  //
                _decl,                                                                //
                "__HTP_" + NamePrefix + Name.str(),                                   //
                FunctionDecls[FunctionSource::HTP]                                    //
            );
            Functions[FunctionSource::HTP_IMPL].Source = std::make_shared<FunctionSource>( //
                FunctionSource::HTP_IMPL,                                                  //
                _decl,                                                                     //
                "___HTP_" + NamePrefix + Name.str(),                                       //
                FunctionDecls[FunctionSource::HTP_IMPL]                                    //
            );

            switch (T) {
                case Normal: {
                    // The GTP has the origin FD
                    auto &GTP = Functions[FunctionSource::GTP].Source;
                    assert(OrgFunctionDecl);
                    if (!GTP->getFunctionDecl()) {
                        GTP->setReturnValue({getTypeString(OrgFunctionDecl->getReturnType()), {}, {}});
                        SmallVector<FunctionSource::Param> Args;
                        for (int i = 0; i < OrgFunctionDecl->getNumParams(); ++i) {
                            Args.push_back({getTypeString(OrgFunctionDecl->getParamDecl(i)->getType()), {}, {}});
                        }
                        GTP->setArguments(Args);

                        if (OrgFunctionDecl->isVariadic()) {
                            GTP->setVariadic(true);
                        }
                    }

                    // The HTP should be "void (void **args, voi *ret, void **metadata)"
                    auto &HTP = Functions[FunctionSource::HTP].Source;
                    if (!HTP->getFunctionDecl()) {
                        HTP->setReturnValue({"void", {}});
                        HTP->setArguments({
                            {"void **", "args", {}},
                            {"void *",  "ret",  {}},
                            {"void **", "metadata", {}},
                        });
                    }
                    break;
                }
                case GuestCallback: {
                    // The GTP should be "void (void *callback, void **args, voi *ret, void **metadata)"
                    auto &GTP = Functions[FunctionSource::GTP].Source;
                    if (!GTP->getFunctionDecl()) {
                        GTP->setReturnValue({"void", {}});
                        GTP->setArguments({
                            {"void *",  "callback", {}},
                            {"void **", "args",     {}},
                            {"void *",  "ret",      {}},
                            {"void **", "metadata", {}},
                        });
                    }

                    // The HTP should be the same as the callback type
                    auto &HTP = Functions[FunctionSource::HTP].Source;
                    if (!HTP->getFunctionDecl()) {
                        HTP->setReturnValue({getTypeString(ApiReturnType), {}, {}});

                        SmallVector<FunctionSource::Param> Args;
                        for (int i = 0; i < ApiArgumentTypes.size(); ++i) {
                            Args.push_back({getTypeString(ApiArgumentTypes[i]), {}, {}});
                        }
                        HTP->setArguments(Args);

                        if (ApiVariadic) {
                            HTP->setVariadic(true);
                        }
                    }
                    break;
                }
                case HostCallback: {
                    // The GTP should be the same as the callback type
                    auto &GTP = Functions[FunctionSource::GTP].Source;
                    if (!GTP->getFunctionDecl()) {
                        GTP->setReturnValue({getTypeString(ApiReturnType), {}, {}});
                        SmallVector<FunctionSource::Param> Args;
                        for (int i = 0; i < ApiArgumentTypes.size(); ++i) {
                            Args.push_back({getTypeString(ApiArgumentTypes[i]), {}, {}});
                        }
                        GTP->setArguments(Args);

                        if (ApiVariadic) {
                            GTP->setVariadic(true);
                        }
                    }

                    // The HTP should be "void (void *callback, void **args, voi *ret, void **metadata)"
                    auto &HTP = Functions[FunctionSource::HTP].Source;
                    if (!HTP->getFunctionDecl()) {
                        HTP->setReturnValue({"void", {}});
                        HTP->setArguments({
                            {"void *",  "callback", {}},
                            {"void **", "args",     {}},
                            {"void *",  "ret",      {}},
                            {"void **", "metadata", {}},
                        });
                    }
                    break;
                }
            }
        }

        ApiSource *_decl;

        Type ApiType;
        PassContext *Parent;

        QualType ApiReturnType;
        llvm::SmallVector<QualType> ApiArgumentTypes;
        bool ApiVariadic = false;

        // Function Data
        std::string Name;
        QualType QT;
        std::array<const FunctionDecl *, FunctionSource::HTP_IMPL + 1> FunctionDecls{};
        const FunctionDecl *OrgFunctionDecl;
        const FunctionDecl *HintFunctionDecl;

        struct AnnotationImpl {
            std::string Name;
            SmallVector<std::string> Arguments;
        };
        SmallVector<AnnotationImpl> HintAnnotations;

        // Texts
        std::array<std::list<std::string>, 2> ForwardSources;
        std::array<std::list<std::string>, 2> BackwardSources;

        // Metadata
        std::map<std::string, std::string> Attributes;

        // Children
        struct FunctionData {
            std::shared_ptr<FunctionSource> Source;
        };
        std::array<FunctionData, FunctionSource::HTP_IMPL + 1> Functions;

        std::string getText(bool guest) const {
            auto &TP = Functions[guest ? FunctionSource::GTP : FunctionSource::HTP].Source;
            auto &TP_IMPL = Functions[guest ? FunctionSource::GTP_IMPL : FunctionSource::HTP_IMPL].Source;
            std::array Sources{
                llvm::join(ForwardSources[!!!guest], "\n"),
                TP_IMPL->getFunctionDecl() ? "" : TP_IMPL->getText(),
                TP->getFunctionDecl() ? "" : TP->getText(),
                llvm::join(BackwardSources[!!!guest], "\n"),
            };
            return llvm::join(Sources, "\n");
        }
    };

    ApiSource::ApiSource(Type T, PassContext *Parent, llvm::StringRef Name, const QualType &QT,
                         const ArrayRef<const FunctionDecl *> &FunctionDecls,
                         const clang::FunctionDecl *OrgFunctionDecl, const FunctionDecl *HintFunctionDecl)
        : _impl(std::make_unique<Impl>(this, T, Parent, Name, QT, FunctionDecls, OrgFunctionDecl, HintFunctionDecl)) {
    }

    ApiSource::~ApiSource() = default;

    ApiSource::Type ApiSource::getApiType() const {
        return _impl->ApiType;
    }

    PassContext *ApiSource::getParent() const {
        return _impl->Parent;
    }

    clang::QualType ApiSource::getApiReturnType() const {
        return _impl->ApiReturnType;
    }

    llvm::ArrayRef<clang::QualType> ApiSource::getApiArgumentTypes() const {
        return _impl->ApiArgumentTypes;
    }

    bool ApiSource::getApiVariadic() const {
        return _impl->ApiVariadic;
    }

    StringRef ApiSource::getName() const {
        return _impl->Name;
    }

    QualType ApiSource::getType() const {
        return _impl->QT;
    }

    const clang::FunctionDecl *ApiSource::getFunctionDecl(FunctionSource::Type type) const {
        return _impl->FunctionDecls[type];
    }

    const clang::FunctionDecl *ApiSource::getOrgFunctionDecl() const {
        return _impl->OrgFunctionDecl;
    }

    const FunctionDecl *ApiSource::getHintFunctionDecl() const {
        return _impl->HintFunctionDecl;
    }

    int ApiSource::getHintAnnotationCount() const {
        return int(_impl->HintAnnotations.size());
    }

    ApiSource::Annotation ApiSource::getHintAnnotation(int n) const {
        auto &Anno = _impl->HintAnnotations[n];
        return {Anno.Name, Anno.Arguments};
    }

    void ApiSource::prependForward(StringRef lines, bool guest) {
        _impl->ForwardSources[!!!guest].push_front((lines.ends_with("\n") ? lines.drop_back() : lines).str());
    }

    void ApiSource::appendForward(StringRef lines, bool guest) {
        _impl->ForwardSources[!!!guest].push_back((lines.ends_with("\n") ? lines.drop_back() : lines).str());
    }

    void ApiSource::prependBackward(StringRef lines, bool guest) {
        _impl->BackwardSources[!!!guest].push_front((lines.ends_with("\n") ? lines.drop_back() : lines).str());
    }

    void ApiSource::appendBackward(StringRef lines, bool guest) {
        _impl->BackwardSources[!!!guest].push_back((lines.ends_with("\n") ? lines.drop_back() : lines).str());
    }

    FunctionSource &ApiSource::getFunctionSource(FunctionSource::Type type) {
        return *_impl->Functions[type].Source;
    }

    std::string ApiSource::getText(bool guest) const {
        return _impl->getText(guest);
    }

    StringRef ApiSource::getAttribute(StringRef key) const {
        auto it = _impl->Attributes.find(key.str());
        if (it == _impl->Attributes.end()) {
            return {};
        }
        return it->second;
    }

    void ApiSource::setAttribute(StringRef key, StringRef value) {
        if (value.empty()) {
            _impl->Attributes.erase(key.str());
            return;
        }
        _impl->Attributes[key.str()] = value;
    }

}