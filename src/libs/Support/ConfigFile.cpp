#include "ConfigFile.h"

#include <cerrno>
#include <algorithm>
#include <limits>
#include <fstream>
#include <cstdlib>

namespace lore {

    namespace fs = std::filesystem;

    // Utility functions
    static std::string trim(const std::string &str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            return "";
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    static bool isWhitespace(char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    static std::string unquoteString(const std::string &str) {
        if (str.length() >= 2 &&
            ((str.front() == '"' && str.back() == '"') || (str.front() == '\'' && str.back() == '\''))) {
            return str.substr(1, str.length() - 2);
        }
        return str;
    }

    // Parse quoted string with escape sequences
    static std::optional<std::string> parseQuotedString(const std::string &str) {
        if (str.empty())
            return str;

        char quoteChar = str[0];
        if (quoteChar != '"' && quoteChar != '\'') {
            return str; // Not a quoted string, return as is
        }

        if (str.length() < 2 || str.back() != quoteChar) {
            return std::nullopt; // Unmatched quotes
        }

        std::string result;
        result.reserve(str.length() - 2);

        for (size_t i = 1; i < str.length() - 1; ++i) {
            if (str[i] == '\\') {
                if (i + 1 < str.length() - 1) {
                    switch (str[i + 1]) {
                        case 'n':
                            result += '\n';
                            break;
                        case 't':
                            result += '\t';
                            break;
                        case 'r':
                            result += '\r';
                            break;
                        case '\\':
                            result += '\\';
                            break;
                        case '"':
                            result += '"';
                            break;
                        case '\'':
                            result += '\'';
                            break;
                        default:
                            return std::nullopt; // Unknown escape sequence
                    }
                    ++i;
                } else {
                    return std::nullopt; // Incomplete escape sequence
                }
            } else {
                result += str[i];
            }
        }

        return result;
    }

    // Remove inline comments (ignore # inside quotes)
    static std::optional<std::string> removeInlineComment(const std::string &line) {
        std::string result;
        bool inQuotes = false;
        char quoteChar = 0;
        bool escaped = false;

        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];

            if (escaped) {
                result += c;
                escaped = false;
                continue;
            }

            if (c == '\\') {
                escaped = true;
                result += c;
                continue;
            }

            if (c == '"' || c == '\'') {
                if (!inQuotes) {
                    inQuotes = true;
                    quoteChar = c;
                } else if (c == quoteChar) {
                    inQuotes = false;
                }
                result += c;
            } else if (c == '#' && !inQuotes) {
                // Found comment start, truncate
                break;
            } else {
                result += c;
            }
        }

        if (inQuotes) {
            return std::nullopt; // Unclosed quotes
        }

        if (escaped) {
            return std::nullopt; // Incomplete escape sequence
        }

        return result;
    }

    void ConfigFile::Section::addKeyValue(const std::string &key, const std::string &value) {
        auto it = m_keyIndex.find(key);
        if (it == m_keyIndex.end()) {
            // New key, add to both vector and map
            m_keyValues.emplace_back(key, value);
            m_keyIndex[key] = m_keyValues.size() - 1;
        } else {
            // Existing key, update value
            m_keyValues[it->second].second = value;
        }
    }
    std::optional<int> ConfigFile::Section::getInt(const std::string &key) const {
        auto strVal = getString(key);
        if (!strVal || strVal->empty())
            return std::nullopt;

        char *endptr;
        errno = 0;
        long result = std::strtol(strVal->c_str(), &endptr, 10);

        if (errno != 0 || *endptr != '\0') {
            return std::nullopt;
        }

        if (result < std::numeric_limits<int>::min() || result > std::numeric_limits<int>::max()) {
            return std::nullopt;
        }
        return static_cast<int>(result);
    }
    std::optional<double> ConfigFile::Section::getDouble(const std::string &key) const {
        auto strVal = getString(key);
        if (!strVal || strVal->empty())
            return std::nullopt;

        char *endptr;
        errno = 0;
        double result = std::strtod(strVal->c_str(), &endptr);

        if (errno != 0 || *endptr != '\0') {
            return std::nullopt;
        }

        return result;
    }
    std::optional<bool> ConfigFile::Section::getBool(const std::string &key) const {
        auto strVal = getString(key);
        if (!strVal || strVal->empty())
            return std::nullopt;

        std::string lowerVal;
        std::transform(strVal->begin(), strVal->end(), std::back_inserter(lowerVal),
                       [](unsigned char c) { return std::tolower(c); });

        if (lowerVal == "true" || lowerVal == "yes" || lowerVal == "on" || lowerVal == "1") {
            return true;
        } else if (lowerVal == "false" || lowerVal == "no" || lowerVal == "off" || lowerVal == "0") {
            return false;
        }

        return std::nullopt;
    }
    size_t ConfigFile::getOrCreateSection(const std::string &sectionName) {
        auto it = m_sectionIndex.find(sectionName);
        if (it != m_sectionIndex.end()) {
            return it->second;
        }

        m_sections.emplace_back(sectionName);
        size_t index = m_sections.size() - 1;
        m_sectionIndex[sectionName] = index;
        return index;
    }
    ConfigFile::ParseResult ConfigFile::parseLine(const std::string &originalLine, const std::filesystem::path &file,
                                                  int lineNum) {
        auto lineOpt = removeInlineComment(originalLine);
        if (!lineOpt) {
            return ParseResult::error("Unclosed quotes or invalid escape sequence", file, lineNum);
        }

        std::string line = trim(*lineOpt);
        if (line.empty())
            return ParseResult::ok();

        // Check for include directive
        if (line.compare(0, 7, "include") == 0) {
            std::string includeFile = trim(line.substr(7));
            if (includeFile.empty()) {
                return ParseResult::error("include directive requires a filename", file, lineNum);
            }

            includeFile = unquoteString(includeFile);

            fs::path includePath =
                (m_baseDirStack.empty() ? fs::path(".") : m_baseDirStack.back()) / includeFile;
            if (!fs::exists(includePath)) {
                return ParseResult::error("Included file not found: " + includePath.string(), file, lineNum);
            }

            return load(includePath.string());
        }

        // Check for section
        if (line.front() == '[' && line.back() == ']') {
            std::string sectionName = trim(line.substr(1, line.length() - 2));
            if (sectionName.empty()) {
                return ParseResult::error("Empty section name", file, lineNum);
            }

            auto parsedSection = parseQuotedString(sectionName);
            if (!parsedSection) {
                return ParseResult::error("Invalid section name format", file, lineNum);
            }

            m_currentSectionIndex = getOrCreateSection(*parsedSection);
            return ParseResult::ok();
        }

        // Parse key-value pair or single key
        size_t equalPos = line.find('=');
        std::string key, value;

        if (equalPos != std::string::npos) {
            key = trim(line.substr(0, equalPos));
            value = trim(line.substr(equalPos + 1));

            if (!value.empty()) {
                size_t valueStart = 0;
                while (valueStart < value.length() && isWhitespace(value[valueStart])) {
                    valueStart++;
                }
                size_t valueEnd = value.length() - 1;
                while (valueEnd > valueStart && isWhitespace(value[valueEnd])) {
                    valueEnd--;
                }
                value = value.substr(valueStart, valueEnd - valueStart + 1);
            }
        } else {
            key = trim(line);
            value = "";
        }

        if (key.empty()) {
            return ParseResult::error("Empty key", file, lineNum);
        }

        auto parsedKey = parseQuotedString(unquoteString(key));
        auto parsedValue = parseQuotedString(unquoteString(value));

        if (!parsedKey) {
            return ParseResult::error("Invalid key format", file, lineNum);
        }
        if (!parsedValue) {
            return ParseResult::error("Invalid value format", file, lineNum);
        }

        m_sections[m_currentSectionIndex].addKeyValue(*parsedKey, *parsedValue);
        return ParseResult::ok();
    }
    ConfigFile::ConfigFile() {
        // Initialize with global section
        m_sections.emplace_back(GlobalSectionName);
        m_sectionIndex[GlobalSectionName] = 0;
        m_currentSectionIndex = 0;
    }
    ConfigFile::ParseResult ConfigFile::load(const std::filesystem::path &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return ParseResult::error("Cannot open file: " + filename.string(), filename, 0);
        }

        auto baseDir = filename.parent_path();
        if (baseDir.empty()) {
            baseDir = ".";
        }
        m_baseDirStack.push_back(std::move(baseDir));

        std::string line;
        int lineNum = 0;

        while (std::getline(file, line)) {
            lineNum++;
            auto result = parseLine(line, filename, lineNum);
            if (!result.success) {
                m_baseDirStack.pop_back();
                clear();
                return result;
            }
        }

        m_baseDirStack.pop_back();
        return ParseResult::ok();
    }
    void ConfigFile::clear() {
        m_sections.clear();
        m_sectionIndex.clear();
        m_baseDirStack.clear();

        // Reinitialize with global section
        m_sections.emplace_back(GlobalSectionName);
        m_sectionIndex[GlobalSectionName] = 0;
        m_currentSectionIndex = 0;
    }

}
