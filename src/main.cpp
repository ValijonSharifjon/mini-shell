#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

struct Command {
    std::vector<char*> args;
};

std::vector<Command> parse_pipeline(const std::string& line) {
    std::vector<Command> commands;
    std::stringstream ss(line);
    std::string segment;

    // Разбиваем строку по символу '|'
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

void execute_single_command(const std::vector<char*>& args) {
    pid_t pid = fork();

    if (pid == 0) {
        // === ДОЧЕРНИЙ ПРОЦЕСС ===
        execvp(args[0], const_cast<char**>(args.data()));
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        // === РОДИТЕЛЬСКИЙ ПРОЦЕСС ===
        waitpid(pid, nullptr, 0);
    } else {
        perror("fork");
    }
}

void execute_pipeline(std::vector<Command>& commands) {
    int num_commands = commands.size();

    // Добавляем nullptr в конец каждого массива аргументов
    // (требование execvp)
    for (auto& cmd : commands) {
        cmd.args.push_back(nullptr);
    }

    // Если команда одна, pipe не нужен
    if (num_commands == 1) {
        execute_single_command(commands[0].args);
        return;
    }

     // ШАГ 1: Создание труб (pipes)

    int num_pipes = num_commands - 1;  // Для N команд нужно N-1 труб
    int pipefds[num_pipes][2];         // Массив файловых дескрипторов

    // Создаём все трубы
    for (int i = 0; i < num_pipes; ++i) {
        if (pipe(pipefds[i]) == -1) {
            perror("pipe");
            return;
        }
        // pipefds[i][0] - для чтения (read end)
        // pipefds[i][1] - для записи (write end)
    }

    // ШАГ 2: Запуск дочерних процессов

    for (int i = 0; i < num_commands; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
             // Если это НЕ первая команда, читаем из предыдущей трубы
            if (i > 0) {
                // Перенаправляем stdin на чтение из трубы
                dup2(pipefds[i - 1][0], STDIN_FILENO);
            }

            if (i < num_commands - 1) {
                // Перенаправляем stdout на запись в трубу
                dup2(pipefds[i][1], STDOUT_FILENO);
            }

            // После dup2 нам больше не нужны оригинальные дескрипторы
            // Дочерний процесс должен закрыть все трубы
            for (int j = 0; j < num_pipes; j++) {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            // Выполняем команду
            execvp(commands[i].args[0], commands[i].args.data());
            
            // Если execvp вернулся - произошла ошибка
            perror("execvp");
            exit(1);
        } else if (pid < 0) {
            perror("fork");
            return;
        }
    }

    for (int i = 0; i < num_pipes; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    for (int i = 0; i < num_commands; i++) {
        wait(nullptr);
    }
}

int main() {
    while (true) {
        std::cout<<"myshell>> ";
        std::string line;

         // Читаем строку от пользователя
        if (!std::getline(std::cin, line)) {
            std::cout<< "\nexit\n";
            break;
        }

        if (line.empty()) continue;

        // Проверяем, есть ли pipe в команде
        if (line.find('|') != std::string::npos) {
            // Есть pipe - парсим и выполняем pipeline
            std::vector<Command> commands = parse_pipeline(line);
            execute_pipeline(commands);

            for (auto& cmd : commands) {
                for (char* arg : cmd.args) {
                    if (arg != nullptr) {
                        free(arg);
                    }
                }
            }
        } else {
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
    }
    
    return 0;
}