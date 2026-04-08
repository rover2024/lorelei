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
            Section(const std::string &name) : m_name(name) {
            }

            /// Check if a key exists
            bool contains(const std::string &key) const {
                return m_keyIndex.find(key) != m_keyIndex.end();
            }

            /// Get string value
            std::optional<std::string> getString(const std::string &key) const {
                auto it = m_keyIndex.find(key);
                if (it == m_keyIndex.end())
                    return std::nullopt;
                return m_keyValues[it->second].second;
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
                return m_name;
            }

            /// Get number of keys
            size_t size() const {
                return m_keyValues.size();
            }

            /// Check if section is empty
            bool empty() const {
                return m_keyValues.empty();
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
                    : m_keyValues(keyValues), m_index(index) {
                }
                const value_type &operator*() const {
                    return (*m_keyValues)[m_index];
                }
                const value_type &operator->() const {
                    return (*m_keyValues)[m_index];
                }
                iterator &operator++() {
                    ++m_index;
                    return *this;
                }
                iterator operator++(int) {
                    iterator tmp = *this;
                    ++m_index;
                    return tmp;
                }
                iterator &operator--() {
                    --m_index;
                    return *this;
                }
                iterator operator--(int) {
                    iterator tmp = *this;
                    --m_index;
                    return tmp;
                }
                iterator &operator+=(difference_type n) {
                    m_index += n;
                    return *this;
                }
                iterator &operator-=(difference_type n) {
                    m_index -= n;
                    return *this;
                }
                iterator operator+(difference_type n) const {
                    return iterator(m_keyValues, m_index + n);
                }
                friend iterator operator+(difference_type n, const iterator &it) {
                    return iterator(it.m_keyValues, it.m_index + n);
                }
                iterator operator-(difference_type n) const {
                    return iterator(m_keyValues, m_index - n);
                }
                difference_type operator-(const iterator &other) const {
                    return m_index - other.m_index;
                }
                value_type operator[](difference_type n) const {
                    return (*m_keyValues)[m_index + n];
                }
                bool operator==(const iterator &other) const {
                    return m_index == other.m_index;
                }
                bool operator!=(const iterator &other) const {
                    return m_index != other.m_index;
                }
                bool operator<(const iterator &other) const {
                    return m_index < other.m_index;
                }
                bool operator>(const iterator &other) const {
                    return m_index > other.m_index;
                }
                bool operator<=(const iterator &other) const {
                    return m_index <= other.m_index;
                }
                bool operator>=(const iterator &other) const {
                    return m_index >= other.m_index;
                }

            private:
                const std::vector<std::pair<std::string, std::string>> *m_keyValues = nullptr;
                size_t m_index = 0;
            };

            iterator begin() const {
                return iterator(&m_keyValues, 0);
            }
            iterator end() const {
                return iterator(&m_keyValues, m_keyValues.size());
            }

            /// Get all keys
            std::vector<std::string> keys() const {
                std::vector<std::string> result;
                result.reserve(m_keyValues.size());
                for (const auto &[key, value] : m_keyValues) {
                    result.push_back(key);
                }
                return result;
            }

            /// Get all key-value pairs
            const std::vector<std::pair<std::string, std::string>> &items() const {
                return m_keyValues;
            }

            size_t count() const {
                return m_keyValues.size();
            }

        private:
            std::vector<std::pair<std::string, std::string>>
                m_keyValues;                                    // Store key-value pairs in order
            std::unordered_map<std::string, size_t> m_keyIndex; // Map key to index in vector
            std::string m_name;

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
                : m_sections(sections), m_index(index) {
            }
            reference operator*() const {
                return (*m_sections)[m_index];
            }

            pointer operator->() const {
                return &(*m_sections)[m_index];
            }
            iterator &operator++() {
                ++m_index;
                return *this;
            }
            iterator operator++(int) {
                iterator tmp = *this;
                ++m_index;
                return tmp;
            }
            iterator &operator--() {
                --m_index;
                return *this;
            }
            iterator operator--(int) {
                iterator tmp = *this;
                --m_index;
                return tmp;
            }

            iterator &operator+=(difference_type n) {
                m_index += n;
                return *this;
            }
            iterator &operator-=(difference_type n) {
                m_index -= n;
                return *this;
            }
            iterator operator+(difference_type n) const {
                return iterator(m_sections, m_index + n);
            }
            friend iterator operator+(difference_type n, const iterator &it) {
                return iterator(it.m_sections, it.m_index + n);
            }
            iterator operator-(difference_type n) const {
                return iterator(m_sections, m_index - n);
            }
            difference_type operator-(const iterator &other) const {
                return m_index - other.m_index;
            }
            reference operator[](difference_type n) const {
                return (*m_sections)[m_index + n];
            }
            bool operator==(const iterator &other) const {
                return m_index == other.m_index;
            }
            bool operator!=(const iterator &other) const {
                return m_index != other.m_index;
            }
            bool operator<(const iterator &other) const {
                return m_index < other.m_index;
            }
            bool operator>(const iterator &other) const {
                return m_index > other.m_index;
            }
            bool operator<=(const iterator &other) const {
                return m_index <= other.m_index;
            }
            bool operator>=(const iterator &other) const {
                return m_index >= other.m_index;
            }

        private:
            std::vector<Section> *m_sections;
            size_t m_index;
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
                : m_sections(sections), m_index(index) {
            }

            reference operator*() const {
                return (*m_sections)[m_index];
            }

            pointer operator->() const {
                return &(*m_sections)[m_index];
            }

            const_iterator &operator++() {
                ++m_index;
                return *this;
            }
            const_iterator operator++(int) {
                const_iterator tmp = *this;
                ++m_index;
                return tmp;
            }
            const_iterator &operator--() {
                --m_index;
                return *this;
            }
            const_iterator operator--(int) {
                const_iterator tmp = *this;
                --m_index;
                return tmp;
            }

            const_iterator &operator+=(difference_type n) {
                m_index += n;
                return *this;
            }
            const_iterator &operator-=(difference_type n) {
                m_index -= n;
                return *this;
            }

            const_iterator operator+(difference_type n) const {
                return const_iterator(m_sections, m_index + n);
            }

            friend const_iterator operator+(difference_type n, const const_iterator &it) {
                return const_iterator(it.m_sections, it.m_index + n);
            }

            const_iterator operator-(difference_type n) const {
                return const_iterator(m_sections, m_index - n);
            }

            difference_type operator-(const const_iterator &other) const {
                return m_index - other.m_index;
            }

            reference operator[](difference_type n) const {
                return (*m_sections)[m_index + n];
            }

            bool operator==(const const_iterator &other) const {
                return m_index == other.m_index;
            }
            bool operator!=(const const_iterator &other) const {
                return m_index != other.m_index;
            }
            bool operator<(const const_iterator &other) const {
                return m_index < other.m_index;
            }
            bool operator>(const const_iterator &other) const {
                return m_index > other.m_index;
            }
            bool operator<=(const const_iterator &other) const {
                return m_index <= other.m_index;
            }
            bool operator>=(const const_iterator &other) const {
                return m_index >= other.m_index;
            }

        private:
            const std::vector<Section> *m_sections;
            size_t m_index;
        };

    public:
        ConfigFile();

        /// Parse a file and return the result
        ParseResult load(const std::filesystem::path &filename);

        /// Check if a section exists
        bool contains(const std::string &section) const {
            return m_sectionIndex.find(section) != m_sectionIndex.end();
        }

        /// Get a section by name (returns nullopt if not found)
        std::optional<std::reference_wrapper<Section>> get(const std::string &section) {
            auto it = m_sectionIndex.find(section);
            if (it == m_sectionIndex.end())
                return std::nullopt;
            return std::ref(m_sections[it->second]);
        }

        /// Const version of get
        std::optional<std::reference_wrapper<const Section>> get(const std::string &section) const {
            auto it = m_sectionIndex.find(section);
            if (it == m_sectionIndex.end())
                return std::nullopt;
            return std::cref(m_sections[it->second]);
        }

        /// Get global section
        Section &global() {
            return m_sections[0];
        }
        const Section &global() const {
            return m_sections[0];
        }

        /// Get section by index (no bounds checking)
        Section &operator[](size_t index) {
            return m_sections[index];
        }
        const Section &operator[](size_t index) const {
            return m_sections[index];
        }

        /// Get section by index with bounds checking
        std::optional<std::reference_wrapper<Section>> at(size_t index) {
            if (index < m_sections.size()) {
                return std::ref(m_sections[index]);
            }
            return std::nullopt;
        }
        std::optional<std::reference_wrapper<const Section>> at(size_t index) const {
            if (index < m_sections.size()) {
                return std::cref(m_sections[index]);
            }
            return std::nullopt;
        }

        /// Iterators
        iterator begin() {
            return iterator(&m_sections, 0);
        }
        iterator end() {
            return iterator(&m_sections, m_sections.size());
        }
        const_iterator begin() const {
            return const_iterator(&m_sections, 0);
        }
        const_iterator end() const {
            return const_iterator(&m_sections, m_sections.size());
        }
        const_iterator cbegin() const {
            return const_iterator(&m_sections, 0);
        }
        const_iterator cend() const {
            return const_iterator(&m_sections, m_sections.size());
        }

        /// Find a section by name using iterator
        iterator find(const std::string &sectionName) {
            auto it = m_sectionIndex.find(sectionName);
            if (it != m_sectionIndex.end()) {
                return iterator(&m_sections, it->second);
            }
            return end();
        }

        const_iterator find(const std::string &sectionName) const {
            auto it = m_sectionIndex.find(sectionName);
            if (it != m_sectionIndex.end()) {
                return const_iterator(&m_sections, it->second);
            }
            return cend();
        }

        /// Get number of sections
        size_t count() const {
            return m_sections.size();
        }

        /// Clear all configuration (keep global section)
        void clear();

    private:
        std::vector<Section> m_sections;                        // All sections stored in order
        std::unordered_map<std::string, size_t> m_sectionIndex; // Map section name to index
        size_t m_currentSectionIndex = 0;                       // Index of current section
        std::vector<std::filesystem::path> m_baseDirStack;      // Include resolution context stack

        // Get or create a section
        size_t getOrCreateSection(const std::string &sectionName);

        // Parse a single line
        ParseResult parseLine(const std::string &originalLine, const std::filesystem::path &file,
                              int lineNum);
    };

}

#endif // LORE_BASE_CORELIB_CONFIGFILE_H
