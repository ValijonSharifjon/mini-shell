#include "executor.h"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "builtins.h"
#include "jobs.h"

static void execute_single(const std::vector<char*>& args) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], const_cast<char**>(args.data()));
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, nullptr, 0);
    } else {
        perror("fork");
    }
}

void execute_pipeline(std::vector<Command>& commands) {
    int num_commands = commands.size();

    for (auto& cmd : commands) {
        cmd.args.push_back(nullptr);
    }

    if (num_commands == 1) {
        execute_single(commands[0].args);
        return;
    }

    int num_pipes = num_commands - 1;
    int pipefds[num_pipes][2];

    for (int i = 0; i < num_pipes; ++i) {
        if (pipe(pipefds[i]) == -1) {
            perror("pipe");
            return;
        }
    }

    for (int i = 0; i < num_commands; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            if (i > 0) dup2(pipefds[i - 1][0], STDIN_FILENO);
            if (i < num_commands - 1) dup2(pipefds[i][1], STDOUT_FILENO);

            for (int j = 0; j < num_pipes; j++) {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            execvp(commands[i].args[0], commands[i].args.data());
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

void execute_command(ParsedLine& parsed, const std::string& line) {
    if (handle_builtin(parsed.args)) return;

    bool redirect_out = false, redirect_in = false, redirect_err = false,
         err_to_out = false;
    std::string out_file, in_file, err_file;
    std::vector<char*> clean_args;

    for (size_t i = 0; i < parsed.args.size(); ++i) {
        if (strcmp(parsed.args[i], ">") == 0) {
            redirect_out = true;
            out_file = parsed.args[++i];
        } else if (strcmp(parsed.args[i], "<") == 0) {
            redirect_in = true;
            in_file = parsed.args[++i];
        } else if (strcmp(parsed.args[i], "2>") == 0) {
            redirect_err = true;
            err_file = parsed.args[++i];
        } else if (strcmp(parsed.args[i], "2>&1") == 0) {
            err_to_out = true;
        } else {
            clean_args.push_back(parsed.args[i]);
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
            int fd = open(out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open output");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (redirect_err) {
            int fd = open(err_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
        if (!parsed.background) {
            foreground_pid = pid;
            add_job(pid, line);

            int status;
            waitpid(pid, &status, WUNTRACED);

            if (WIFSTOPPED(status)) {
                job_list.back().state = Job::STOPPED;
                std::cout << "\n[" << job_list.back().job_id << "]+ Stopped    "
                          << line << "\n";
            } else {
                job_list.back().state = Job::DONE;
                clean_jobs();
            }
            foreground_pid = -1;
        } else {
            add_job(pid, line);
            printf("[%d] %d\n", job_list.back().job_id, pid);
        }
    } else {
        perror("fork");
    }
}