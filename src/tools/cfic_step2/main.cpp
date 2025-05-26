#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <org_file> <pp_file> [output_file]" << std::endl;
        return 1;
    }

    // Read preprocessed file
    std::string pp_str;
    {
        fs::path pp_path(argv[2]);
        std::ifstream pp_file(pp_path);
        if (!pp_file.is_open()) {
            std::cout << "Failed to open \"" << pp_path << "\"" << std::endl;
            return EXIT_FAILURE;
        }
        pp_str = {std::istreambuf_iterator<char>(pp_file), std::istreambuf_iterator<char>()};
    }

    std::vector<std::string> macros;
    {
        static std::regex re(R"(/\*__LORELEI_MACRO_BEGIN__\*/([\s\S]+?)/\*__LORELEI_MACRO_END__\*/)");

        auto begin = std::sregex_iterator(pp_str.begin(), pp_str.end(), re);
        auto end = std::sregex_iterator();

        for (std::sregex_iterator it = begin; it != end; ++it) {
            std::smatch match = *it;
            std::string captured = match.str(1);
            macros.push_back(captured);
        }
    }

    // Read original file
    std::string new_str;
    {
        fs::path org_path(argv[1]);
        std::ifstream org_file(org_path);
        if (!org_file.is_open()) {
            std::cout << "Failed to open \"" << org_path << "\"" << std::endl;
            return EXIT_FAILURE;
        }
        std::string org_str = {std::istreambuf_iterator<char>(org_file), std::istreambuf_iterator<char>()};

        static std::regex re(R"(/\*__LORELEI_MACRO_BEGIN__\*/([\s\S]+?)/\*__LORELEI_MACRO_END__\*/)");
        std::smatch match;

        auto cur = org_str.cbegin();
        auto end = org_str.cend();

        int index = 0; // 当前替换项索引

        while (std::regex_search(cur, end, match, re, std::regex_constants::match_default)) {
            // 添加当前匹配前的内容
            new_str.append(cur, match[0].first);

            // 如果有对应替换项就用它，否则保留原内容
            if (index < static_cast<int>(macros.size())) {
                new_str += macros[index++];
            } else {
                new_str += match.str(0);
            }

            // 移动指针到本次匹配之后
            cur = match[0].second;
        }

        // 添加最后剩余部分
        new_str.append(cur, end);
    }

    std::ostream *out;
    std::fstream output_file;
    if (argc >= 4) {
        fs::path output_path(argv[3]);
        output_file.open(output_path, std::ios::out);
        if (!output_file.is_open()) {
            std::cout << "Failed to open output file" << std::endl;
            return EXIT_FAILURE;
        }
        out = &output_file;
    } else {
        out = &std::cout;
    }
    *out << new_str;

    return EXIT_SUCCESS;
}