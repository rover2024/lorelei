#ifndef SOURCELINES_H
#define SOURCELINES_H

#include <list>
#include <string>

namespace TLC {

    /// \class SourceLines
    /// \brief A container for source code lines.
    template <template <class...> class ListType, class ... ListMods>
    class SourceLines {
    public:
        struct Line {
            std::string id;
            std::string text;
        };
        using LineList = ListType<Line, ListMods...>;

        SourceLines() {
        }

    public:
        void push_back(Line line) {
            _lines.insert(_lines.end(), std::move(line));
        }

        void push_back(std::string id, std::string text) {
            push_back({std::move(id), std::move(text)});
        }

        void push_front(Line line) {
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

        std::string toRawText() const {
            if (_lines.empty()) {
                return {};
            }
            std::string result;
            auto last = std::prev(_lines.end());
            for (auto it = _lines.begin(); it != last; it++) {
                const Line &line = *it;
                result += line.text;
                if (it != last) {
                    result += "\n";
                }
            }
            result += last->text;
            return result;
        }

    protected:
        LineList _lines;
    };

}

#endif // SOURCELINES_H
