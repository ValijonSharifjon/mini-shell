#include "builtins.h"

#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "jobs.h"

bool handle_builtin(const std::vector<char*>& args) {
    if (args.empty()) return false;

    std::string command = args[0];

    if (command == "exit") {
        exit(0);
    } else if (command == "cd") {
        if (args.size() < 2) {
            std::cerr << "cd: missing argument\n";
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return true;
    } else if (command == "pwd") {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            std::cout << cwd << "\n";
        } else {
            perror("pwd");
        }
        return true;
    } else if (command == "jobs") {
        cmd_jobs();
        return true;
    } else if (command == "fg") {
        int job_id = (args.size() > 1) ? atoi(args[1] + 1) : 1;
        cmd_fg(job_id);
        return true;
    } else if (command == "bg") {
        int job_id = (args.size() > 1) ? atoi(args[1] + 1) : 1;
        cmd_bg(job_id);
        return true;
    }

    return false;
}