#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    while (true) {
        std::cout<<"myshell>> ";

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout<< "\nexit\n";
            break;
        }

        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<char*> args;

        while (ss >> token) {
            args.push_back(strdup(token.c_str()));
        }

        pid_t pid = fork();

        if (pid == 0) {
            execvp(args[0], args.data());
            perror("execvp");
            exit(1);
        } else if (pid > 0) {
            waitpid(pid, nullptr, 0);
        } else {
            perror("fork");
        }

        for (char* arg : args) {
            free(arg);
        }
    }
    
    return 0;
}