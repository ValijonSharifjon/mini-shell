#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

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

        std::string command = args[0];

        if (command == "exit") {
            break;
        } else if (command == "cd") {
            if (args.size() < 2) {
                std::cerr << "cd: missing argument\n";
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd");
                }
            }
        } else if (command == "pwd") {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                std::cout << cwd << "\n";
            } else {
                 perror("pwd");
            }
        } else {
            bool redirect_out = false;
            bool redirect_in = false;
            bool redirect_err = false;
            bool err_to_out = false;

            std::string out_file, in_file, err_file;
            std::vector<char*> clean_args;

            for (size_t i = 0; i < args.size(); ++i) {
                if (strcmp(args[i], ">") == 0) {
                    redirect_out = true;
                    out_file = args[i + 1];
                    i++;
                }
                else if (strcmp(args[i], "<") == 0) {
                    redirect_in = true;
                    in_file = args[i + 1];
                    i++;
                }
                else if (strcmp(args[i], "2>") == 0) {
                    redirect_err = true;
                    err_file = args[i + 1];
                    i++;
                } else if (strcmp(args[i], "2>&1") == 0) {
                    err_to_out = true;
                }
                else {
                    clean_args.push_back(args[i]);
                }
            }

            clean_args.push_back(nullptr);

            pid_t pid = fork();

            if (pid == 0) {
                if (redirect_in) {
                    int fd = open(in_file.c_str(), O_RDONLY);
                    if (fd < 0) {
                        perror("open input");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                if (redirect_out) {
                    int fd = open(
                        out_file.c_str(),
                        O_WRONLY | O_CREAT | O_TRUNC,
                        0644
                    );
                    if (fd < 0) {
                        perror("open output");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                if (redirect_err) {
                     int fd = open(
                        err_file.c_str(),
                        O_WRONLY | O_CREAT | O_TRUNC,
                        0644
                    );
                    if (fd < 0) {
                        perror("open error");
                        exit(1);
                    }
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                }

                if (err_to_out) {
                    dup2(STDOUT_FILENO, STDERR_FILENO);
                }

                execvp(clean_args[0], clean_args.data());
                perror("execvp");
                exit(1);
            } else if (pid > 0) {
                waitpid(pid, nullptr, 0);
            } else {
                perror("fork");
            }
        }

        for (char* arg : args) {
            free(arg);
        }
    }
    
    return 0;
}