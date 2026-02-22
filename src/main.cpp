#include <iostream>
#include <sstream>
#include "parser.h"
#include "executor.h"
#include "signals.h"
#include "jobs.h"

int main() {
    setup_signals();

    while (true) {
        std::cout<<"myshell>> ";
        std::string line;

        if (!std::getline(std::cin, line)) {
            std::cout<< "\nexit\n";
            break;
        }

        if (line.empty()) continue;

        if (line.find('|') != std::string::npos) {
            std::vector<Command> commands = parse_pipeline(line);
            execute_pipeline(commands);

            for (auto& cmd : commands) {
                for (char* arg : cmd.args) {
                    if (arg != nullptr) free(arg);
                }
            }
        } else {
            ParsedLine parsed = parse_line(line);
            if (parsed.args.empty()) continue;
            
            execute_command(parsed, line);

            for (char* arg : parsed.args) {
                free(arg);
            }
        }
    }
    
    return 0;
}