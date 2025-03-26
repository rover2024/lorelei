#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <set>
#include <filesystem>

namespace fs = std::filesystem;

static const std::set<std::string> source_exts{
    ".c", ".cc", ".cxx", ".cpp", ".m", ".mm",
};
static const std::set<std::string> ignored_include{
    "stdc-predef.h",
};

// 判断文件路径是否在项目目录内
static bool is_descendant(const fs::path &file, const fs::path &dir) {
    try {
        auto relative = fs::relative(file, dir);
        return !relative.empty() && *relative.begin() != "..";
    } catch (...) {
        return false;
    }
}

// 命令行参数解析结果
struct Config {
    fs::path input;
    fs::path project_dir;
    fs::path output;
};

static void printHelp(const char *path) {
    std::cout << "Preprocessor Filter\n";
    std::cout << "Usage: " << std::filesystem::path(path).filename().string()
              << " <input> [-d <project dir>] [-o <output>]\n";
    exit(EXIT_SUCCESS);
}

// 手写命令行参数解析
static Config parse_args(int argc, char *argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.size() == 0) {
        printHelp(argv[0]);
    }

    Config cfg;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-o") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing argument for -o\n";
                exit(EXIT_FAILURE);
            }
            cfg.output = fs::absolute(args[++i]);
            continue;
        }
        if (args[i] == "-d") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing argument for -d\n";
                exit(EXIT_FAILURE);
            }
            cfg.project_dir = fs::absolute(args[++i]);
            continue;
        }

        if (cfg.input.empty()) {
            cfg.input = fs::absolute(args[i]);
            continue;
        }
    }

    if (cfg.input.empty()) {
        std::cerr << "Missing input\n";
        exit(EXIT_FAILURE);
    }
    return cfg;
}

// 主处理函数
void process_file(const Config &cfg) {
    // 读取输入文件
    std::ifstream fin(cfg.input);
    if (!fin) {
        std::cerr << cfg.input << ": failed to open input file" << "\n";
        exit(EXIT_FAILURE);
    }

    // 准备输出
    std::ostream *out = &std::cout;
    std::ofstream fout;
    if (!cfg.output.empty()) {
        fout.open(cfg.output);
        if (!fout) {
            std::cerr << cfg.output << "failed to open output file" << "\n";
            exit(EXIT_FAILURE);
        }
        out = &fout;
    }

    // 预处理正则表达式
    std::regex re("^# (\\d+) \"([^\"]+)\"(( \\d+)*)$");
    int level = 0;
    std::string line;

    while (std::getline(fin, line)) {
        std::smatch match;
        if (std::regex_search(line, match, re)) {
            // 解析预处理指令
            std::string filename = match[2];
            fs::path file_path(filename);

            /*
                https://stackoverflow.com/questions/5370539/what-is-the-meaning-of-lines-starting-with-a-hash-sign-and-number-like-1-a-c
                https://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html

                # line-number "source-file" [flags]
                1 - Start of a new file
                2 - Returning to previous file
                3 - Following text comes from a system header file (#include <> vs #include "")
                4 - Following text should be treated as being wrapped in an implicit extern "C" block."
            */

            // 处理flags
            std::set<int> flags;
            std::string flags_str = match[3];
            std::istringstream iss(flags_str);
            int num;
            while (iss >> num)
                flags.insert(num);

            bool is_system = flags.count(3);
            if (flags.count(1)) {
                // 新文件开始
                if (level > 0) {
                    level++;
                } else {
                    bool ignore;
                    if (filename.front() == '<') {
                        // <built-in> <command-line> 不忽略
                        ignore = false;
                    } else {
                        // 源文件不忽略
                        ignore = !source_exts.count(file_path.extension());

                        // 工程目录内的一律不忽略
                        if (!cfg.project_dir.empty()) {
                            ignore = ignore && !is_descendant(file_path, cfg.project_dir);
                        }
                    }

                    if (ignore) {
                        // 生成 include 语句，忽略全部内容
                        if (!ignored_include.count(file_path.filename())) {
                            *out << (is_system ? "#include <" : "#include \"") << filename
                                 << (is_system ? ">\n" : "\"\n");
                        }
                        level = 1;
                    }
                }
            } else if (flags.count(2)) {
                // 返回上级文件
                if (level > 0)
                    level--;
            }
            continue;
        }

        // 仅输出当前 level 为 0 的内容
        if (level == 0) {
            *out << line << "\n";
        }
    }
}

int main(int argc, char *argv[]) {
    Config cfg = parse_args(argc, argv);

    // 验证项目目录
    if (!cfg.project_dir.empty() && !fs::is_directory(cfg.project_dir)) {
        std::cerr << cfg.project_dir << ": invalid project directory" << "\n";
        return EXIT_FAILURE;
    }

    process_file(cfg);
    return EXIT_SUCCESS;
}