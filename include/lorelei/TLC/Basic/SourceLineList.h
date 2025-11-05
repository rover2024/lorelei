#ifndef SOURCELINELIST_H
#define SOURCELINELIST_H

#include <list>
#include <string>

namespace TLC {

    /// \class SourceLineList
    /// \brief A container for source code lines.
    template <template <class...> class ListType = std::list, class... ListMods>
    class SourceLineList {
    public:
        struct Line {
            std::string id;
            std::string text;
        };
        using LineList = ListType<Line, ListMods...>;

        SourceLineList() {
        }

    public:
        void push_back(Line line) {
            if (line.text.empty())
                return;
            _lines.insert(_lines.end(), std::move(line));
        }

        void push_back(std::string id, std::string text) {
            push_back({std::move(id), std::move(text)});
        }

        void push_front(Line line) {
            if (line.text.empty())
                return;
            _lines.insert(_lines.begin(), std::move(line));
        }

        void push_front(std::string id, std::string text) {
            push_front({std::move(id), std::move(text)});
        }

        const LineList &lines() const {
            return _lines;
        }

        LineList &lines() {
            return _lines;
        }

        bool empty() const {
            return _lines.empty();
        }

        std::string toRawText() const {
            if (_lines.empty()) {
                return {};
            }
            std::string result;
            auto last = std::prev(_lines.end());
            for (auto it = _lines.begin(); it != last; it++) {
                const Line &line = *it;
                result += line.text;
                if (line.text.back() != '\n') {
                    result += "\n";
                }
            }
            {
                const Line &line = *last;
                result += line.text;
                if (line.text.back() != '\n') {
                    result += "\n";
                }
            }
            return result;
        }

    protected:
        LineList _lines;
    };

}

#endif // SOURCELINELIST_H
