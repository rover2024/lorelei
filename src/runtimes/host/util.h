#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

namespace Util {

    void StringReplaceAll(std::string &str, const std::string &from, const std::string &to);

    std::vector<std::string_view> SplitString(const std::string &str, const std::string &delim);

    std::string JoinString(const std::vector<std::string> &vec, const std::string &delim);
}

#endif // UTIL_H