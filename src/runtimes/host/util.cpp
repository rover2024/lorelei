#include "util.h"

namespace Util {

    void StringReplaceAll(std::string &str, const std::string &from, const std::string &to) {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    std::vector<std::string_view> SplitString(const std::string &str, const std::string &delim) {
        std::vector<std::string_view> tokens;
        std::string::size_type start = 0;
        std::string::size_type end = str.find(delim);
        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + delim.size();
            end = str.find(delim, start);
        }
        tokens.push_back(str.substr(start));
        return tokens;
    }

    std::string JoinString(const std::vector<std::string> &vec, const std::string &delim) {
        if (vec.empty())
            return {};

        size_t length = 0;
        for (const auto &item : vec) {
            length += item.size();
        }
        length += delim.size() * (vec.size() - 1);

        std::string res;
        res.reserve(length);
        for (int i = 0; i < vec.size() - 1; ++i) {
            res.append(vec[i]);
            res.append(delim);
        }
        res.append(vec.back());
        return res;
    }

}