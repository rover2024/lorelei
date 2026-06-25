#ifndef LORE_CLANGEXTRAS_SOURCELINELIST_H
#define LORE_CLANGEXTRAS_SOURCELINELIST_H

#include <list>
#include <string>

namespace lore::tool {

    /// SourceLineList - An ordered collection of generated source lines.
    ///
    /// Each line carries an id tag (used by passes to locate or group the lines they emitted) and
    /// the line text. Lines stay in insertion order and can be rendered back into a single,
    /// newline-terminated string with toRawText().
    ///
    /// The backing container is a template parameter, so callers can swap the sequence type (for
    /// example \c std::vector) without changing this interface. It defaults to std::list, whose
    /// iterators and references stay valid across insertion at either end.
    template <template <class...> class ListType = std::list, class... ListMods>
    class SourceLineList {
    public:
        /// One source line: an identifier tag plus its text.
        struct Line {
            std::string id;
            std::string text;
        };
        using LineList = ListType<Line, ListMods...>;

        SourceLineList() = default;

        /// Appends a line. Empty-text lines are ignored.
        void push_back(Line line) {
            if (line.text.empty())
                return;
            m_lines.insert(m_lines.end(), std::move(line));
        }

        /// @overload
        void push_back(std::string id, std::string text) {
            push_back({std::move(id), std::move(text)});
        }

        /// Prepends a line. Empty-text lines are ignored.
        void push_front(Line line) {
            if (line.text.empty())
                return;
            m_lines.insert(m_lines.begin(), std::move(line));
        }

        /// @overload
        void push_front(std::string id, std::string text) {
            push_front({std::move(id), std::move(text)});
        }

        /// The underlying lines, in insertion order.
        const LineList &lines() const {
            return m_lines;
        }
        LineList &lines() {
            return m_lines;
        }

        /// Whether no line has been added yet.
        bool empty() const {
            return m_lines.empty();
        }

        /// Concatenates every line, terminating each with a newline if it lacks one.
        std::string toRawText() const {
            std::string result;
            for (const Line &line : m_lines) {
                result += line.text;
                if (!line.text.empty() && line.text.back() != '\n')
                    result += '\n';
            }
            return result;
        }

    protected:
        LineList m_lines;
    };

}

#endif // LORE_CLANGEXTRAS_SOURCELINELIST_H
