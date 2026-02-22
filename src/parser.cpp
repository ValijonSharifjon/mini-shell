#include "parser.h"
#include <sstream>
#include <cstring>
#include <cstdlib>

std::vector<Command> parse_pipeline(const std::string& line) {
    std::vector<Command> commands;
    std::stringstream ss(line);
    std::string segment;

    while (std::getline(ss, segment, '|')) {
        Command cmd;
        std::stringstream seg_ss(segment);
        std::string token;

        while (seg_ss >> token) {
            cmd.args.push_back(strdup(token.c_str()));
        }
        
        if (!cmd.args.empty()) {
            commands.push_back(cmd);
        }
    }

    return commands;
}

ParsedLine parse_line(const std::string& line) {
    ParsedLine result;
    result.background = false;
    result.redirect_out = false;
    result.redirect_in = false;
    result.redirect_err = false;
    result.err_to_out = false;

    std::stringstream ss(line);
    std::string token;

    while (ss >> token) {
        result.args.push_back(strdup(token.c_str()));
    }

    if (!result.args.empty() && strcmp(result.args.back(), "&") == 0) {
        result.background = true;
        free(result.args.back());
        result.args.pop_back();
    }

    return result;
}