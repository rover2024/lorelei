#ifndef ANALYZER_H
#define ANALYZER_H

#include <clang/AST/AST.h>
#include <clang/Frontend/CompilerInstance.h>

#include "configfile.h"
#include "thunkdefinition.h"

namespace TLC {

    class Analyzer {
    public:
        /// \param ci  The related compiler instance.
        /// \param fds The set of function declarations in AST.
        /// \param vds The set of variable declarations in AST.
        /// \param cfg The configuration file to use for analysis.
        Analyzer(clang::CompilerInstance &CI, llvm::ArrayRef<const clang::FunctionDecl *> FDs,
                 llvm::ArrayRef<const clang::VarDecl *> VDs, const ConfigFile &cfg);

    public:
        void analyze();
        void generateHeader(llvm::raw_ostream &os) const;
        void generateSource(llvm::raw_ostream &os) const;

    public:
        clang::CompilerInstance &CI() const {
            return _ci;
        }
        const std::set<const clang::FunctionDecl *> &functionDecls() const {
            return _fds;
        }
        const std::set<const clang::VarDecl *> &varDecls() const {
            return _vds;
        }
        const ConfigFile &config() const {
            return _config;
        }
        std::map<std::string, ThunkDefinition> &thunks(ThunkDefinition::Type type) {
            return _thunks[type];
        }
        const std::map<std::string, ThunkDefinition> &thunks(ThunkDefinition::Type type) const {
            return _thunks[type];
        }

        std::filesystem::path inputFile() const;

        std::string callbackName(const std::string &type, bool guest) const {
            auto &map = _callbackNameIndexes[!guest];
            auto it = map.find(type);
            if (it == map.end()) {
                return {};
            }
            return it->second;
        }

    protected:
        clang::CompilerInstance &_ci;
        std::set<const clang::FunctionDecl *> _fds;
        std::set<const clang::VarDecl *> _vds;
        const ConfigFile &_config;

        // initialized in constructor
        std::array<std::map<std::string, ThunkDefinition>, ThunkDefinition::NumTypes> _thunks;
        std::array<std::map<std::string, std::string>, 2>
            _callbackNameIndexes; // array[ guest ? 0 : 1, map[ type -> name ] ]
        std::map<std::string, const clang::VarDecl *> _vars;
        std::set<std::string> _missingSymbols;
    };

}

#endif // ANALYZER_H