#pragma once
#include <vector>
#include <string>

struct Command {
    std::vector<char*> args;
};

struct ParsedLine {
    std::vector<char*> args;
    bool background;
    bool redirect_out;
    bool redirect_in;
    bool redirect_err;
    bool err_to_out;
    std::string out_file;
    std::string in_file;
    std::string err_file;
};

std::vector<Command> parse_pipeline(const std::string& line);
ParsedLine parse_line(const std::string& line);
