#include "pass.h"

#include <set>
#include <sstream>

#include "typerep.h"
#include "thunkdefinition.h"
#include "common.h"
#include "analyzer.h"

using namespace clang;
using namespace llvm;

namespace TLC {

    enum Side {
        GuestSide,
        HostSide,
    };

    enum ConverterDirection {
        GuestToHost,
        HostToGuest,
    };

    struct Converter {
        Converter() {
            data[GuestSide][GuestToHost] = nullptr;
            data[GuestSide][HostToGuest] = nullptr;
            data[HostSide][GuestToHost] = nullptr;
            data[HostSide][HostToGuest] = nullptr;
        }

        std::array<std::array<const clang::FunctionDecl *, 2>, 2> data;
    };

    class TypeConverterPass : public EntryExitPass {
    public:
        TypeConverterPass() : EntryExitPass(PT_TypeConverter, "type_converter") {
        }
        ~TypeConverterPass() = default;

    public:
        void initialize(Analyzer *analyzer) override;
        bool test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const override;
        Error begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) override;
        Error end(ThunkDefinition &thunk) override;

    protected:
        // [ guest_side:0, host_side:1 -> [ g2h:0, h2g:1 -> converter ] ]
        std::map<std::string, Converter> _typeConvs;

        void begin_GuestFunction(std::string ID, ThunkDefinition &thunk) {
            auto typeView = thunk.view();
            auto thunkRetType = typeView.returnType();
            auto thunkArgTypes = typeView.argTypes();

            auto &GTP = thunk.function(FunctionDefinition::GTP);
            auto &HTP_IMPL = thunk.function(FunctionDefinition::HTP_IMPL);

            SmallVector<std::string> guestSideArgConvesions;
            SmallVector<std::string> hostSideArgConvesions;
            for (int i = 0; i < thunkArgTypes.size(); i++) {
                auto argTypeStr = getTypeString(thunkArgTypes[i]);
                auto it = _typeConvs.find(argTypeStr);
                if (it == _typeConvs.end()) {
                    continue;
                }

                auto &conv = it->second.data;
                if (conv[GuestSide][GuestToHost]) {
                    std::stringstream ss;
                    ss << "    arg" << i + 1 << " = "
                       << conv[GuestSide][GuestToHost]->getName().str() << "(" << "arg" << i + 1
                       << ");\n";
                    guestSideArgConvesions.push_back(ss.str());
                }
                if (conv[HostSide][GuestToHost]) {
                    std::stringstream ss;
                    ss << "    arg" << i + 1 << " = "
                       << conv[HostSide][GuestToHost]->getName().str() << "(" << "arg" << i + 1
                       << ");\n";
                    hostSideArgConvesions.push_back(ss.str());
                }
            }

            std::string guestSideRetConversion;
            std::string hostSideRetConversion;
            do {
                auto it = _typeConvs.find(getTypeString(thunkRetType));
                if (it == _typeConvs.end()) {
                    break;
                }
                auto &conv = it->second.data;
                if (conv[GuestSide][HostToGuest]) {
                    std::stringstream ss;
                    ss << "    ret = " << conv[GuestSide][HostToGuest]->getName().str()
                       << "(ret);\n";
                    guestSideRetConversion = ss.str();
                }
                if (conv[HostSide][HostToGuest]) {
                    std::stringstream ss;
                    ss << "    ret = " << conv[HostSide][HostToGuest]->getName().str()
                       << "(ret);\n";
                    hostSideRetConversion = ss.str();
                }
            } while (false);

            if (!guestSideArgConvesions.empty()) {
                GTP.bodyForward().push_back(ID, llvm::join(guestSideArgConvesions, "\n"));
            }
            if (!guestSideRetConversion.empty()) {
                GTP.bodyBackward().push_front(ID, guestSideRetConversion);
            }
            if (!hostSideArgConvesions.empty()) {
                HTP_IMPL.bodyForward().push_back(ID, llvm::join(hostSideArgConvesions, "\n"));
            }
            if (!hostSideRetConversion.empty()) {
                HTP_IMPL.bodyBackward().push_front(ID, hostSideRetConversion);
            }
        }

        void begin_GuestCallback(std::string ID, ThunkDefinition &thunk) {
            // TODO
        }

        void begin_HostCallback(std::string ID, ThunkDefinition &thunk) {
            begin_GuestFunction(ID, thunk);
        }
    };

    void TypeConverterPass::initialize(Analyzer *analyzer) {
        /// STEP: Collect type conversion function declarations
        for (const auto &FD : std::as_const(analyzer->functionDecls())) {
            const auto &name = FD->getName();
            if (char prefix[] = "__TYPECONV_"; name.starts_with(prefix)) {
                auto realName = name.substr(sizeof(prefix) - 1);
                if (char prefix[] = "G_"; realName.starts_with(prefix)) {
                    realName = realName.substr(sizeof(prefix) - 1);
                    if (char prefix[] = "G2H_"; realName.starts_with(prefix)) {
                        // __TYPECONV_G_G2H_xxx
                        realName = realName.substr(sizeof(prefix) - 1);
                        if (FD->param_size() == 1) {
                            auto &firstParam = *FD->param_begin();
                            _typeConvs[getTypeString(firstParam->getType())]
                                .data[GuestSide][GuestToHost] = FD;
                        }
                    } else if (char prefix[] = "H2G_"; realName.starts_with(prefix)) {
                        // __TYPECONV_G_H2G_xxx
                        realName = realName.substr(sizeof(prefix) - 1);
                        if (FD->param_size() == 1) {
                            auto &firstParam = *FD->param_begin();
                            _typeConvs[getTypeString(firstParam->getType())]
                                .data[GuestSide][HostToGuest] = FD;
                        }
                    }
                } else if (char prefix[] = "H_"; realName.starts_with(prefix)) {
                    realName = realName.substr(sizeof(prefix) - 1);
                    if (char prefix[] = "G2H_"; realName.starts_with(prefix)) {
                        // __TYPECONV_H_G2H_xxx
                        realName = realName.substr(sizeof(prefix) - 1);
                        if (FD->param_size() == 1) {
                            auto &firstParam = *FD->param_begin();
                            _typeConvs[getTypeString(firstParam->getType())]
                                .data[HostSide][GuestToHost] = FD;
                        }
                    } else if (char prefix[] = "H2G_"; realName.starts_with(prefix)) {
                        // __TYPECONV_H_H2G_xxx
                        realName = realName.substr(sizeof(prefix) - 1);
                        if (FD->param_size() == 1) {
                            auto &firstParam = *FD->param_begin();
                            _typeConvs[getTypeString(firstParam->getType())]
                                .data[HostSide][HostToGuest] = FD;
                        }
                    }
                }
            }
        }
    }

    bool TypeConverterPass::test(const ThunkDefinition &thunk,
                                 SmallVectorImpl<IntOrString> &args) const {
        return true;
    }

    Error TypeConverterPass::begin(ThunkDefinition &thunk,
                                   const SmallVectorImpl<IntOrString> &args) {
        std::string ID = "type_converter";
        switch (thunk.type()) {
            case ThunkDefinition::GuestFunctionThunk:
                begin_GuestFunction(ID, thunk);
                break;
            case ThunkDefinition::GuestCallbackThunk:
                begin_GuestCallback(ID, thunk);
                break;
            case ThunkDefinition::HostCallbackThunk:
                begin_HostCallback(ID, thunk);
                break;
            default:
                break;
        }
        return Error::success();
    }

    Error TypeConverterPass::end(ThunkDefinition &thunk) {
        return Error::success();
    }


    /// \brief Pass registrations.
    static PassRegistration<TypeConverterPass> PR_callback_substituter;

}