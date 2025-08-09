#include "pass.h"

#include <sstream>

#include <clang/AST/Attr.h>

#include "thunkdefinition.h"
#include "common.h"

using namespace clang;
using namespace llvm;

namespace TLC {

    static void begin_GuestFunction(std::string ID, ThunkDefinition &thunk) {
        auto typeView = thunk.view();

        auto thunkRetType = typeView.returnType();
        auto thunkArgTypes = typeView.argTypes();
        bool isVoid = thunkRetType->isVoidType();

        LIST_FDS(thunk);

        // GTP
        if (!GTP.rep().decl()) {
            // prolog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                GTP.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << GTP_IMPL.rep().name() << "(" + getCallArgsString(thunkArgTypes.size())
                   << ");\n";
                GTP.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                GTP.epilog().push_back(ID, ss.str());
            }
        }

        // GTP_IMPL
        if (!GTP_IMPL.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }
                ss << "    void *args[] = {\n";
                for (int i = 0; i < thunkArgTypes.size(); ++i) {
                    ss << "        (void *) &arg" << i + 1 << ",\n";
                }
                ss << "    };\n";

                GTP_IMPL.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    LORELIB_INVOKE_HTP(LORELIB_HTP(" << thunk.name() << "), args, "
                   << (isVoid ? "NULL" : "&ret") << ", NULL);\n";
                GTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    return ret;\n";
                }
                GTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }

        // HTP
        if (!HTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                for (int i = 0; i < thunkArgTypes.size(); ++i) {
                    ss << "    __auto_type arg" << i + 1 //
                       << " = *(__typeof__(" << getTypeString(thunkArgTypes[i]) << ") *) args[" << i
                       << "];\n";
                }
                if (!isVoid) {
                    ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(thunkRetType)
                       << ") *) ret;\n";
                }
                HTP.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "*ret_ref = ";
                }
                ss << HTP_IMPL.rep().name() << "(" << getCallArgsString(thunkArgTypes.size())
                   << ");\n";
                HTP.body().push_back(ID, ss.str());
            }
        }

        // HTP_IMPL
        if (!HTP_IMPL.rep().decl()) {
            // prolog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                HTP_IMPL.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << "LORELIB_API(" << thunk.name() << ")("
                   << getCallArgsString(thunkArgTypes.size()) << ");\n";
                HTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                HTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }
    }

    static void begin_GuestCallback(std::string ID, ThunkDefinition &thunk) {
        auto typeView = thunk.view();

        auto thunkRetType = typeView.returnType();
        auto thunkArgTypes = typeView.argTypes();
        bool isVoid = thunkRetType->isVoidType();

        LIST_FDS(thunk);

        // GTP
        if (!GTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                for (int i = 0; i < thunkArgTypes.size(); ++i) {
                    ss << "    __auto_type arg" << i + 1 << //
                        " = *(__typeof__(" << getTypeString(thunkArgTypes[i]) << ") *) args[" << i
                       << "];\n";
                }
                if (!isVoid) {
                    ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(thunkRetType)
                       << ") *) ret;\n";
                }
                GTP.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "*ret_ref = ";
                }
                ss << GTP_IMPL.rep().name() << "(callback"
                   << getCallArgsString(thunkArgTypes.size(), true) << ");\n";

                GTP.body().push_back(ID, ss.str());
            }
        }

        // GTP_IMPL
        if (!GTP_IMPL.rep().decl()) {
            // prolog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                GTP_IMPL.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << "((__typeof__(" << getTypeString(thunk.qualType()) << ") *) callback)("
                   << getCallArgsString(thunkArgTypes.size()) << ");\n";
                GTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                GTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }

        // HTP
        if (!HTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                ss << "#ifdef LORELIB_CALLBACK_REPLACE\n"
                   << "    LORELIB_GET_LAST_GCB(callback);\n"
                   << "#else\n"
                   << "    void *callback = LORELIB_LAST_GCB;\n"
                   << "#endif\n";
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }
                HTP.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << HTP_IMPL.rep().name() << "(callback"
                   << getCallArgsString(thunkArgTypes.size(), true) << ");\n";

                HTP.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                HTP.epilog().push_back(ID, ss.str());
            }
        }

        // HTP_IMPL
        if (!HTP_IMPL.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }
                ss << "    void *args[] = {\n";
                for (int i = 0; i < thunkArgTypes.size(); ++i) {
                    ss << "        (void *) &arg" << i + 1 << ",\n";
                }
                ss << "    };\n";
                HTP_IMPL.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    LORELIB_INVOKE_GCB(LORELIB_GCB(" << thunk.name() << "), callback, args, "
                   << (isVoid ? "NULL" : "&ret") << ", NULL);\n";
                HTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                HTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }
    }

    static void begin_HostCallback(std::string ID, ThunkDefinition &thunk) {
        auto typeView = thunk.view();

        auto thunkRetType = typeView.returnType();
        auto thunkArgTypes = typeView.argTypes();
        bool isVoid = thunkRetType->isVoidType();

        LIST_FDS(thunk);

        // GTP
        if (!GTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                ss << "#ifdef LORELIB_CALLBACK_REPLACE\n"
                   << "    LORELIB_GET_LAST_HCB(callback);\n"
                   << "#else\n"
                   << "    void *callback = LORELIB_LAST_HCB;\n"
                   << "#endif\n";
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }
                GTP.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << GTP_IMPL.rep().name() << "(callback"
                   << getCallArgsString(thunkArgTypes.size(), true) << ");\n";

                GTP.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                GTP.epilog().push_back(ID, ss.str());
            }
        }

        // GTP_IMPL
        if (!GTP_IMPL.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                if (!isVoid) {
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                }
                ss << "    void *args[] = {\n";
                for (int i = 0; i < thunkArgTypes.size(); ++i) {
                    ss << "        (void *) &arg" << i + 1 << ",\n";
                }
                ss << "    };\n";

                GTP_IMPL.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    LORELIB_INVOKE_HCB(LORELIB_HCB(" << thunk.name() << "), callback, args, "
                   << (isVoid ? "NULL" : "&ret") << ", NULL);\n";
                GTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                GTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }

        // HTP
        if (!HTP.rep().decl()) {
            // prolog
            {
                std::stringstream ss;
                for (int i = 0; i < thunkArgTypes.size(); ++i) {
                    ss << "    __auto_type arg" << i + 1 << //
                        " = *(__typeof__(" << getTypeString(thunkArgTypes[i]) << ") *) args[" << i
                       << "];\n";
                }
                if (!isVoid) {
                    ss << "    __auto_type ret_ref = (__typeof__(" << getTypeString(thunkRetType)
                       << ") *) ret;\n";
                }
                HTP.prolog().push_back(ID, ss.str());
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "*ret_ref = ";
                }
                ss << HTP_IMPL.rep().name() << "(callback"
                   << getCallArgsString(thunkArgTypes.size(), true) << ");\n";

                HTP.body().push_back(ID, ss.str());
            }
        }

        // HTP_IMPL
        if (!HTP_IMPL.rep().decl()) {
            // prolog
            {
                if (!isVoid) {
                    std::stringstream ss;
                    ss << "    __typeof__(" << getTypeString(thunkRetType) << ") ret;\n";
                    HTP_IMPL.prolog().push_back(ID, ss.str());
                }
            }

            // body
            {
                std::stringstream ss;
                ss << "    ";
                if (!isVoid) {
                    ss << "ret = ";
                }
                ss << "((__typeof__(" << getTypeString(thunk.qualType()) << ") *) callback)("
                   << getCallArgsString(thunkArgTypes.size()) << ");\n";

                HTP_IMPL.body().push_back(ID, ss.str());
            }

            // epilog
            if (!isVoid) {
                std::stringstream ss;
                ss << "    return ret;\n";
                HTP_IMPL.epilog().push_back(ID, ss.str());
            }
        }
    }

    /// \class StandardPass
    class StandardPass : public BuilderPass {
    public:
        StandardPass() : BuilderPass(PT_Standard, "standard") {
        }
        ~StandardPass() = default;

    public:
        bool test(const ThunkDefinition &thunk, SmallVectorImpl<IntOrString> &args) const override;
        Error begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) override;
        Error end(ThunkDefinition &thunk) override;
    };

    bool StandardPass::test(const ThunkDefinition &thunk,
                            SmallVectorImpl<IntOrString> &args) const {
        return false;
    }

    Error StandardPass::begin(ThunkDefinition &thunk, const SmallVectorImpl<IntOrString> &args) {
        (void) args;

        std::string ID = "standard";
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

    Error StandardPass::end(ThunkDefinition &thunk) {
        return Error::success();
    }

    /// \brief Pass registrations.
    static PassRegistration<StandardPass> PR_standard;

}