#ifndef BUILDERPASSCOMMON_H
#define BUILDERPASSCOMMON_H

#include <llvm/ADT/StringExtras.h>
#include <clang/AST/ASTContext.h>

#include <lorelei/TLC/AST/FunctionInfo.h>
#include <lorelei/TLC/AST/TypeExtras.h>

namespace TLC {

    static inline FunctionInfo FI_bridgeFunctionInfo(clang::ASTContext &ast) {
        clang::QualType voidType = ast.VoidTy;
        clang::QualType pVoidType = ast.getPointerType(voidType);
        clang::QualType ppVoidType = ast.getPointerType(pVoidType);
        return {
            voidType,
            {
              {ppVoidType, "args"},
              {pVoidType, "ret"},
              {pVoidType, "metadata"},
              },
        };
    }

    static inline FunctionInfo FI_bridgeCallbackInfo(clang::ASTContext &ast) {
        clang::QualType voidType = ast.VoidTy;
        clang::QualType pVoidType = ast.getPointerType(voidType);
        clang::QualType ppVoidType = ast.getPointerType(pVoidType);
        return {
            voidType,
            {
              {pVoidType, "callback"},
              {ppVoidType, "args"},
              {pVoidType, "ret"},
              {pVoidType, "metadata"},
              },
        };
    }

    static inline std::string SRC_asIs(const std::string &s, int indent = 4) {
        return std::string(indent, ' ') + s + "\n";
    }

    // [indent] decltype(<ret_type>) ret;
    static inline std::string SRC_emptyReturnDecl(const FunctionInfo &info, clang::ASTContext &ast,
                                                  int indent = 4) {
        if (info.returnType()->isVoidType())
            return {};
        return std::string(indent, ' ') + getTypeStringWithName(info.returnType(), "ret") + ";\n";
    }

    // [indent]
    static inline std::string SRC_getCallback(int indent = 4) {
        std::string ret;
        ret += std::string(indent - 4, ' ') + "#ifdef LORETHUNK_CALLBACK_REPLACE\n";
        ret += std::string(indent, ' ') + "LORETHUNK_GET_LAST_GCB(callback);\n";
        ret += std::string(indent - 4, ' ') + "#else\n";
        ret += std::string(indent, ' ') + "void *callback = LORETHUNK_LAST_GCB;\n";
        ret += std::string(indent - 4, ' ') + "#endif\n";
        return ret;
    }

    // arg1, arg2, ...
    static inline std::string SRC_callList(const FunctionInfo &info) {
        const auto &args = info.arguments();
        if (args.size() == 0)
            return {};
        std::string ret = args[0].second;
        for (size_t i = 1; i < args.size(); i++) {
            ret += ", " + args[i].second;
        }
        return ret;
    }

    // [indent] <lval> = arg1, arg2, ...;
    static inline std::string SRC_callListAssign(const FunctionInfo &info, const std::string &func,
                                                 const std::string &lval = "ret", int indent = 4) {
        return std::string(indent, ' ') + (!info.returnType()->isVoidType() ? (lval + " = ") : "") +
               func + "(" + SRC_callList(info) + ");\n";
    }

    // [indent] void *args[] = {&arg1, &arg2, ...};
    static inline std::string SRC_argPtrListDecl(const FunctionInfo &info, int indent = 4) {
        std::string res = std::string(indent, ' ') + "void *args[] = {\n";
        for (int i = 0; i < info.arguments().size(); ++i) {
            res += std::string(indent + 4, ' ') + "(void *) &" + info.arguments()[i].second + ",\n";
        }
        res += std::string(indent, ' ') + "};\n";
        return res;
    }

    // [indent] return ret;
    static inline std::string SRC_returnRet(const FunctionInfo &info, int indent = 4) {
        if (info.returnType()->isVoidType())
            return {};
        return std::string(indent, ' ') + "return ret;\n";
    }

    // [indent] auto arg1 = *(typeof(arg1) *) args[1]; ... auto argN = *(typeof(argN) *) args[N];
    static inline std::string SRC_argPtrListExtractDecl(const FunctionInfo &info,
                                                        clang::ASTContext &ast, int N = -1,
                                                        int indent = 4) {
        std::string res;
        auto args = info.arguments();
        N = (N < 0) ? args.size() : N;
        for (size_t i = 0; i < N; ++i) {
            res += std::string(indent, ' ') + "auto &" + args[i].second + " = *(" +
                   (args[i].first.isNull() ? (info.metaArgumentType(args[i].second) + " *")
                                           : getTypeString(ast.getPointerType(args[i].first))) +
                   ") args[" + std::to_string(i) + "];\n";
        }
        return res;
    }

    // [indent] auto &ret_ref = *(decltype(<ret_type>) *) ret;
    static inline std::string SRC_retExtractDecl(const FunctionInfo &info, clang::ASTContext &ast,
                                                 int indent = 4) {
        if (info.returnType()->isVoidType())
            return {};
        return std::string(indent, ' ') + "auto &ret_ref = *(" +
               getTypeString(ast.getPointerType(info.returnType())) + ") ret;\n";
    }

}

#endif // BUILDERPASSCOMMON_H
