#ifndef LORE_BASE_CORELIB_CONFIGFILE_H
#define LORE_BASE_CORELIB_CONFIGFILE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <filesystem>
#include <cstring>

#include <lorelei/Base/Support/Global.h>

namespace lore {

    /// ConfigFile - File reader for an INI-style configuration file, supports including other
    /// configuration files.
    class LORESUPPORT_EXPORT ConfigFile {
    public:
        /// Global section name constant
        static inline const std::string GlobalSectionName = "$global$";

        /// Represents the result of parsing a configuration file.
        struct ParseResult {
            bool success;
            std::string errorMessage;
            std::filesystem::path file;
            int line;

            ParseResult(bool s, const std::string &msg = "", const std::filesystem::path &f = "",
                        int l = 0)
                : success(s), errorMessage(msg), file(f), line(l) {
            }
            static ParseResult ok() {
                return ParseResult(true);
            }
            static ParseResult error(const std::string &msg, const std::filesystem::path &f = "",
                                     int l = 0) {
                return ParseResult(false, msg, f, l);
            }
        };

        /// Represents a section in the configuration file.
        class LORESUPPORT_EXPORT Section {
        public:
            Section(const std::string &name) : _name(name) {
            }

            /// Check if a key exists
            bool contains(const std::string &key) const {
                return _keyIndex.find(key) != _keyIndex.end();
            }

            /// Get string value
            std::optional<std::string> getString(const std::string &key) const {
                auto it = _keyIndex.find(key);
                if (it == _keyIndex.end())
                    return std::nullopt;
                return _keyValues[it->second].second;
            }

            /// Get string value with default
            std::string getString(const std::string &key, const std::string &defaultValue) const {
                return getString(key).value_or(defaultValue);
            }

            /// Get integer value (using strtol)
            std::optional<int> getInt(const std::string &key) const;

            /// Get integer value with default
            int getInt(const std::string &key, int defaultValue) const {
                return getInt(key).value_or(defaultValue);
            }

            /// Get double value (using strtod)
            std::optional<double> getDouble(const std::string &key) const;

            /// Get double value with default
            double getDouble(const std::string &key, double defaultValue) const {
                return getDouble(key).value_or(defaultValue);
            }

            /// Get boolean value
            std::optional<bool> getBool(const std::string &key) const;

            /// Get boolean value with default
            bool getBool(const std::string &key, bool defaultValue) const {
                return getBool(key).value_or(defaultValue);
            }

            /// Get section name
            const std::string &name() const {
                return _name;
            }

            /// Get number of keys
            size_t size() const {
                return _keyValues.size();
            }

            /// Check if section is empty
            bool empty() const {
                return _keyValues.empty();
            }

            /// Random access iterator for key-value pairs
            class iterator {
            public:
                using iterator_category = std::random_access_iterator_tag;
                using value_type = std::pair<std::string, std::string>;
                using difference_type = std::ptrdiff_t;
                using pointer = value_type *;
                using reference = value_type &;

                iterator() = default;
                iterator(const std::vector<std::pair<std::string, std::string>> *keyValues,
                         size_t index)
                    : _keyValues(keyValues), _index(index) {
                }
                const value_type &operator*() const {
                    return (*_keyValues)[_index];
                }
                const value_type &operator->() const {
                    return (*_keyValues)[_index];
                }
                iterator &operator++() {
                    ++_index;
                    return *this;
                }
                iterator operator++(int) {
                    iterator tmp = *this;
                    ++_index;
                    return tmp;
                }
                iterator &operator--() {
                    --_index;
                    return *this;
                }
                iterator operator--(int) {
                    iterator tmp = *this;
                    --_index;
                    return tmp;
                }
                iterator &operator+=(difference_type n) {
                    _index += n;
                    return *this;
                }
                iterator &operator-=(difference_type n) {
                    _index -= n;
                    return *this;
                }
                iterator operator+(difference_type n) const {
                    return iterator(_keyValues, _index + n);
                }
                friend iterator operator+(difference_type n, const iterator &it) {
                    return iterator(it._keyValues, it._index + n);
                }
                iterator operator-(difference_type n) const {
                    return iterator(_keyValues, _index - n);
                }
                difference_type operator-(const iterator &other) const {
                    return _index - other._index;
                }
                value_type operator[](difference_type n) const {
                    return (*_keyValues)[_index + n];
                }
                bool operator==(const iterator &other) const {
                    return _index == other._index;
                }
                bool operator!=(const iterator &other) const {
                    return _index != other._index;
                }
                bool operator<(const iterator &other) const {
                    return _index < other._index;
                }
                bool operator>(const iterator &other) const {
                    return _index > other._index;
                }
                bool operator<=(const iterator &other) const {
                    return _index <= other._index;
                }
                bool operator>=(const iterator &other) const {
                    return _index >= other._index;
                }

            private:
                const std::vector<std::pair<std::string, std::string>> *_keyValues = nullptr;
                size_t _index = 0;
            };

            iterator begin() const {
                return iterator(&_keyValues, 0);
            }
            iterator end() const {
                return iterator(&_keyValues, _keyValues.size());
            }

            /// Get all keys
            std::vector<std::string> keys() const {
                std::vector<std::string> result;
                result.reserve(_keyValues.size());
                for (const auto &[key, value] : _keyValues) {
                    result.push_back(key);
                }
                return result;
            }

            /// Get all key-value pairs
            const std::vector<std::pair<std::string, std::string>> &items() const {
                return _keyValues;
            }

            size_t count() const {
                return _keyValues.size();
            }

        private:
            std::vector<std::pair<std::string, std::string>>
                _keyValues;                                    // Store key-value pairs in order
            std::unordered_map<std::string, size_t> _keyIndex; // Map key to index in vector
            std::string _name;

            void addKeyValue(const std::string &key, const std::string &value);

            friend class ConfigFile;
        };

        /// Iterator for ConfigFile sections
        class iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = Section;
            using difference_type = std::ptrdiff_t;
            using pointer = Section *;
            using reference = Section &;

            iterator(std::vector<Section> *sections, size_t index)
                : _sections(sections), _index(index) {
            }
            reference operator*() const {
                return (*_sections)[_index];
            }

            pointer operator->() const {
                return &(*_sections)[_index];
            }
            iterator &operator++() {
                ++_index;
                return *this;
            }
            iterator operator++(int) {
                iterator tmp = *this;
                ++_index;
                return tmp;
            }
            iterator &operator--() {
                --_index;
                return *this;
            }
            iterator operator--(int) {
                iterator tmp = *this;
                --_index;
                return tmp;
            }

            iterator &operator+=(difference_type n) {
                _index += n;
                return *this;
            }
            iterator &operator-=(difference_type n) {
                _index -= n;
                return *this;
            }
            iterator operator+(difference_type n) const {
                return iterator(_sections, _index + n);
            }
            friend iterator operator+(difference_type n, const iterator &it) {
                return iterator(it._sections, it._index + n);
            }
            iterator operator-(difference_type n) const {
                return iterator(_sections, _index - n);
            }
            difference_type operator-(const iterator &other) const {
                return _index - other._index;
            }
            reference operator[](difference_type n) const {
                return (*_sections)[_index + n];
            }
            bool operator==(const iterator &other) const {
                return _index == other._index;
            }
            bool operator!=(const iterator &other) const {
                return _index != other._index;
            }
            bool operator<(const iterator &other) const {
                return _index < other._index;
            }
            bool operator>(const iterator &other) const {
                return _index > other._index;
            }
            bool operator<=(const iterator &other) const {
                return _index <= other._index;
            }
            bool operator>=(const iterator &other) const {
                return _index >= other._index;
            }

        private:
            std::vector<Section> *_sections;
            size_t _index;
        };

        /// Const iterator for ConfigFile sections.
        class const_iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = const Section;
            using difference_type = std::ptrdiff_t;
            using pointer = const Section *;
            using reference = const Section &;

            const_iterator(const std::vector<Section> *sections, size_t index)
                : _sections(sections), _index(index) {
            }

            reference operator*() const {
                return (*_sections)[_index];
            }

            pointer operator->() const {
                return &(*_sections)[_index];
            }

            const_iterator &operator++() {
                ++_index;
                return *this;
            }
            const_iterator operator++(int) {
                const_iterator tmp = *this;
                ++_index;
                return tmp;
            }
            const_iterator &operator--() {
                --_index;
                return *this;
            }
            const_iterator operator--(int) {
                const_iterator tmp = *this;
                --_index;
                return tmp;
            }

            const_iterator &operator+=(difference_type n) {
                _index += n;
                return *this;
            }
            const_iterator &operator-=(difference_type n) {
                _index -= n;
                return *this;
            }

            const_iterator operator+(difference_type n) const {
                return const_iterator(_sections, _index + n);
            }

            friend const_iterator operator+(difference_type n, const const_iterator &it) {
                return const_iterator(it._sections, it._index + n);
            }

            const_iterator operator-(difference_type n) const {
                return const_iterator(_sections, _index - n);
            }

            difference_type operator-(const const_iterator &other) const {
                return _index - other._index;
            }

            reference operator[](difference_type n) const {
                return (*_sections)[_index + n];
            }

            bool operator==(const const_iterator &other) const {
                return _index == other._index;
            }
            bool operator!=(const const_iterator &other) const {
                return _index != other._index;
            }
            bool operator<(const const_iterator &other) const {
                return _index < other._index;
            }
            bool operator>(const const_iterator &other) const {
                return _index > other._index;
            }
            bool operator<=(const const_iterator &other) const {
                return _index <= other._index;
            }
            bool operator>=(const const_iterator &other) const {
                return _index >= other._index;
            }

        private:
            const std::vector<Section> *_sections;
            size_t _index;
        };

    public:
        ConfigFile();

        /// Parse a file and return the result
        ParseResult load(const std::filesystem::path &filename);

        /// Check if a section exists
        bool contains(const std::string &section) const {
            return _sectionIndex.find(section) != _sectionIndex.end();
        }

        /// Get a section by name (returns nullopt if not found)
        std::optional<std::reference_wrapper<Section>> get(const std::string &section) {
            auto it = _sectionIndex.find(section);
            if (it == _sectionIndex.end())
                return std::nullopt;
            return std::ref(_sections[it->second]);
        }

        /// Const version of get
        std::optional<std::reference_wrapper<const Section>> get(const std::string &section) const {
            auto it = _sectionIndex.find(section);
            if (it == _sectionIndex.end())
                return std::nullopt;
            return std::cref(_sections[it->second]);
        }

        /// Get global section
        Section &global() {
            return _sections[0];
        }
        const Section &global() const {
            return _sections[0];
        }

        /// Get section by index (no bounds checking)
        Section &operator[](size_t index) {
            return _sections[index];
        }
        const Section &operator[](size_t index) const {
            return _sections[index];
        }

        /// Get section by index with bounds checking
        std::optional<std::reference_wrapper<Section>> at(size_t index) {
            if (index < _sections.size()) {
                return std::ref(_sections[index]);
            }
            return std::nullopt;
        }
        std::optional<std::reference_wrapper<const Section>> at(size_t index) const {
            if (index < _sections.size()) {
                return std::cref(_sections[index]);
            }
            return std::nullopt;
        }

        /// Iterators
        iterator begin() {
            return iterator(&_sections, 0);
        }
        iterator end() {
            return iterator(&_sections, _sections.size());
        }
        const_iterator begin() const {
            return const_iterator(&_sections, 0);
        }
        const_iterator end() const {
            return const_iterator(&_sections, _sections.size());
        }
        const_iterator cbegin() const {
            return const_iterator(&_sections, 0);
        }
        const_iterator cend() const {
            return const_iterator(&_sections, _sections.size());
        }

        /// Find a section by name using iterator
        iterator find(const std::string &sectionName) {
            auto it = _sectionIndex.find(sectionName);
            if (it != _sectionIndex.end()) {
                return iterator(&_sections, it->second);
            }
            return end();
        }

        const_iterator find(const std::string &sectionName) const {
            auto it = _sectionIndex.find(sectionName);
            if (it != _sectionIndex.end()) {
                return const_iterator(&_sections, it->second);
            }
            return cend();
        }

        /// Get number of sections
        size_t count() const {
            return _sections.size();
        }

        /// Clear all configuration (keep global section)
        void clear();

    private:
        std::vector<Section> _sections;                        // All sections stored in order
        std::unordered_map<std::string, size_t> _sectionIndex; // Map section name to index
        size_t _currentSectionIndex = 0;                       // Index of current section
        std::filesystem::path _baseDir; // Base directory for include resolution

        // Get or create a section
        size_t getOrCreateSection(const std::string &sectionName);

        // Parse a single line
        ParseResult parseLine(const std::string &originalLine, const std::filesystem::path &file,
                              int lineNum);
    };

}

#endif // LORE_BASE_CORELIB_CONFIGFILE_H
