#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <filesystem>
#include <cstring>

#include <lorelei/Utils/Global.h>

namespace lore {

    class LOREUTILS_EXPORT ConfigFile {
    public:
        // Global section name constant
        static inline const std::string GlobalSectionName = "$global$";

        // Parse result structure
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

        class LOREUTILS_EXPORT Section {
        public:
            Section(const std::string &name) : _name(name) {
            }

            // Add a key-value pair
            void addKeyValue(const std::string &key, const std::string &value);

            // Check if a key exists
            bool contains(const std::string &key) const {
                return _keyIndex.find(key) != _keyIndex.end();
            }

            // Get string value
            std::optional<std::string> getString(const std::string &key) const {
                auto it = _keyIndex.find(key);
                if (it == _keyIndex.end())
                    return std::nullopt;
                return _keyValues[it->second].second;
            }

            // Get string value with default
            std::string getString(const std::string &key, const std::string &defaultValue) const {
                return getString(key).value_or(defaultValue);
            }

            // Get integer value (using strtol)
            std::optional<int> getInt(const std::string &key) const;

            // Get integer value with default
            int getInt(const std::string &key, int defaultValue) const {
                return getInt(key).value_or(defaultValue);
            }

            // Get double value (using strtod)
            std::optional<double> getDouble(const std::string &key) const;

            // Get double value with default
            double getDouble(const std::string &key, double defaultValue) const {
                return getDouble(key).value_or(defaultValue);
            }

            // Get boolean value
            std::optional<bool> getBool(const std::string &key) const;

            // Get boolean value with default
            bool getBool(const std::string &key, bool defaultValue) const {
                return getBool(key).value_or(defaultValue);
            }

            // Get section name
            const std::string &name() const {
                return _name;
            }

            // Get number of keys
            size_t size() const {
                return _keyValues.size();
            }

            // Check if section is empty
            bool empty() const {
                return _keyValues.empty();
            }

            // Random access iterator for key-value pairs
            class Iterator {
            public:
                using iterator_category = std::random_access_iterator_tag;
                using value_type = std::pair<std::string, std::string>;
                using difference_type = std::ptrdiff_t;
                using pointer = value_type *;
                using reference = value_type &;

                Iterator() = default;
                Iterator(const std::vector<std::pair<std::string, std::string>> *keyValues,
                         size_t index)
                    : _keyValues(keyValues), _index(index) {
                }

                const value_type &operator*() const {
                    return (*_keyValues)[_index];
                }

                const value_type &operator->() const {
                    return (*_keyValues)[_index];
                }

                Iterator &operator++() {
                    ++_index;
                    return *this;
                }
                Iterator operator++(int) {
                    Iterator tmp = *this;
                    ++_index;
                    return tmp;
                }
                Iterator &operator--() {
                    --_index;
                    return *this;
                }
                Iterator operator--(int) {
                    Iterator tmp = *this;
                    --_index;
                    return tmp;
                }

                Iterator &operator+=(difference_type n) {
                    _index += n;
                    return *this;
                }
                Iterator &operator-=(difference_type n) {
                    _index -= n;
                    return *this;
                }

                Iterator operator+(difference_type n) const {
                    return Iterator(_keyValues, _index + n);
                }

                friend Iterator operator+(difference_type n, const Iterator &it) {
                    return Iterator(it._keyValues, it._index + n);
                }

                Iterator operator-(difference_type n) const {
                    return Iterator(_keyValues, _index - n);
                }

                difference_type operator-(const Iterator &other) const {
                    return _index - other._index;
                }

                value_type operator[](difference_type n) const {
                    return (*_keyValues)[_index + n];
                }

                bool operator==(const Iterator &other) const {
                    return _index == other._index;
                }
                bool operator!=(const Iterator &other) const {
                    return _index != other._index;
                }
                bool operator<(const Iterator &other) const {
                    return _index < other._index;
                }
                bool operator>(const Iterator &other) const {
                    return _index > other._index;
                }
                bool operator<=(const Iterator &other) const {
                    return _index <= other._index;
                }
                bool operator>=(const Iterator &other) const {
                    return _index >= other._index;
                }

            private:
                const std::vector<std::pair<std::string, std::string>> *_keyValues = nullptr;
                size_t _index = 0;
            };

            Iterator begin() const {
                return Iterator(&_keyValues, 0);
            }
            Iterator end() const {
                return Iterator(&_keyValues, _keyValues.size());
            }

            // Get all keys
            std::vector<std::string> keys() const {
                std::vector<std::string> result;
                result.reserve(_keyValues.size());
                for (const auto &[key, value] : _keyValues) {
                    result.push_back(key);
                }
                return result;
            }

            // Get all key-value pairs
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
        };

        // ConfigFile iterator
        class Iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = Section;
            using difference_type = std::ptrdiff_t;
            using pointer = Section *;
            using reference = Section &;

            Iterator(std::vector<Section> *sections, size_t index)
                : _sections(sections), _index(index) {
            }

            reference operator*() const {
                return (*_sections)[_index];
            }

            pointer operator->() const {
                return &(*_sections)[_index];
            }

            Iterator &operator++() {
                ++_index;
                return *this;
            }
            Iterator operator++(int) {
                Iterator tmp = *this;
                ++_index;
                return tmp;
            }
            Iterator &operator--() {
                --_index;
                return *this;
            }
            Iterator operator--(int) {
                Iterator tmp = *this;
                --_index;
                return tmp;
            }

            Iterator &operator+=(difference_type n) {
                _index += n;
                return *this;
            }
            Iterator &operator-=(difference_type n) {
                _index -= n;
                return *this;
            }

            Iterator operator+(difference_type n) const {
                return Iterator(_sections, _index + n);
            }

            friend Iterator operator+(difference_type n, const Iterator &it) {
                return Iterator(it._sections, it._index + n);
            }

            Iterator operator-(difference_type n) const {
                return Iterator(_sections, _index - n);
            }

            difference_type operator-(const Iterator &other) const {
                return _index - other._index;
            }

            reference operator[](difference_type n) const {
                return (*_sections)[_index + n];
            }

            bool operator==(const Iterator &other) const {
                return _index == other._index;
            }
            bool operator!=(const Iterator &other) const {
                return _index != other._index;
            }
            bool operator<(const Iterator &other) const {
                return _index < other._index;
            }
            bool operator>(const Iterator &other) const {
                return _index > other._index;
            }
            bool operator<=(const Iterator &other) const {
                return _index <= other._index;
            }
            bool operator>=(const Iterator &other) const {
                return _index >= other._index;
            }

        private:
            std::vector<Section> *_sections;
            size_t _index;
        };

        // Const iterator
        class ConstIterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = const Section;
            using difference_type = std::ptrdiff_t;
            using pointer = const Section *;
            using reference = const Section &;

            ConstIterator(const std::vector<Section> *sections, size_t index)
                : _sections(sections), _index(index) {
            }

            reference operator*() const {
                return (*_sections)[_index];
            }

            pointer operator->() const {
                return &(*_sections)[_index];
            }

            ConstIterator &operator++() {
                ++_index;
                return *this;
            }
            ConstIterator operator++(int) {
                ConstIterator tmp = *this;
                ++_index;
                return tmp;
            }
            ConstIterator &operator--() {
                --_index;
                return *this;
            }
            ConstIterator operator--(int) {
                ConstIterator tmp = *this;
                --_index;
                return tmp;
            }

            ConstIterator &operator+=(difference_type n) {
                _index += n;
                return *this;
            }
            ConstIterator &operator-=(difference_type n) {
                _index -= n;
                return *this;
            }

            ConstIterator operator+(difference_type n) const {
                return ConstIterator(_sections, _index + n);
            }

            friend ConstIterator operator+(difference_type n, const ConstIterator &it) {
                return ConstIterator(it._sections, it._index + n);
            }

            ConstIterator operator-(difference_type n) const {
                return ConstIterator(_sections, _index - n);
            }

            difference_type operator-(const ConstIterator &other) const {
                return _index - other._index;
            }

            reference operator[](difference_type n) const {
                return (*_sections)[_index + n];
            }

            bool operator==(const ConstIterator &other) const {
                return _index == other._index;
            }
            bool operator!=(const ConstIterator &other) const {
                return _index != other._index;
            }
            bool operator<(const ConstIterator &other) const {
                return _index < other._index;
            }
            bool operator>(const ConstIterator &other) const {
                return _index > other._index;
            }
            bool operator<=(const ConstIterator &other) const {
                return _index <= other._index;
            }
            bool operator>=(const ConstIterator &other) const {
                return _index >= other._index;
            }

        private:
            const std::vector<Section> *_sections;
            size_t _index;
        };

    public:
        ConfigFile();

        // Parse a file and return the result
        ParseResult load(const std::filesystem::path &filename);

        // Check if a section exists
        bool contains(const std::string &section) const {
            return _sectionIndex.find(section) != _sectionIndex.end();
        }

        // Get a section by name (returns nullopt if not found)
        std::optional<std::reference_wrapper<Section>> get(const std::string &section) {
            auto it = _sectionIndex.find(section);
            if (it == _sectionIndex.end())
                return std::nullopt;
            return std::ref(_sections[it->second]);
        }

        // Const version of get
        std::optional<std::reference_wrapper<const Section>> get(const std::string &section) const {
            auto it = _sectionIndex.find(section);
            if (it == _sectionIndex.end())
                return std::nullopt;
            return std::cref(_sections[it->second]);
        }

        // Get global section
        Section &global() {
            return _sections[0];
        }
        const Section &global() const {
            return _sections[0];
        }

        // Get section by index (no bounds checking)
        Section &operator[](size_t index) {
            return _sections[index];
        }
        const Section &operator[](size_t index) const {
            return _sections[index];
        }

        // Get section by index with bounds checking
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

        // Iterators
        Iterator begin() {
            return Iterator(&_sections, 0);
        }
        Iterator end() {
            return Iterator(&_sections, _sections.size());
        }

        ConstIterator begin() const {
            return ConstIterator(&_sections, 0);
        }
        ConstIterator end() const {
            return ConstIterator(&_sections, _sections.size());
        }

        ConstIterator cbegin() const {
            return ConstIterator(&_sections, 0);
        }
        ConstIterator cend() const {
            return ConstIterator(&_sections, _sections.size());
        }

        // Find a section by name using iterator
        Iterator find(const std::string &sectionName) {
            auto it = _sectionIndex.find(sectionName);
            if (it != _sectionIndex.end()) {
                return Iterator(&_sections, it->second);
            }
            return end();
        }

        ConstIterator find(const std::string &sectionName) const {
            auto it = _sectionIndex.find(sectionName);
            if (it != _sectionIndex.end()) {
                return ConstIterator(&_sections, it->second);
            }
            return cend();
        }

        // Get number of sections
        size_t count() const {
            return _sections.size();
        }

        // Clear all configuration (keep global section)
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

#endif // CONFIGFILE_H
