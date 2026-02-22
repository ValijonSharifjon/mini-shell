#include "signals.h"

#include <signal.h>
#include <sys/wait.h>

#include <iostream>

#include "jobs.h"

void sigint_handler(int sig) {
    (void)sig;
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGINT);
        foreground_pid = -1;
    }

    std::cout << "\nmyshell>>";
    std::cout.flush();
}

void sigtstp_handler(int sig) {
    (void)sig;

    if (foreground_pid > 0) {
        kill(foreground_pid, SIGSTOP);
    }
}

void sigchld_handler(int sig) {
    (void)sig;
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (auto& job : job_list) {
            if (job.pid == pid) {
                job.state = Job::DONE;
                std::cout << "\n[" << job.job_id << "]+ Done    " << job.command
                          << "\n";
            }
        }
    }
    clean_jobs();
}

void setup_signals() {
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGCHLD, sigchld_handler);
    signal(SIGQUIT, SIG_IGN);
}