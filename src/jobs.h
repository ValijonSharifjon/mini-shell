#pragma once
#include <sys/types.h>

#include <string>
#include <vector>

struct Job {
    int job_id;
    pid_t pid;
    std::string command;
    enum { RUNNING, STOPPED, DONE } state;
};

extern std::vector<Job> job_list;
extern pid_t foreground_pid;
extern int next_job_id;

void add_job(pid_t pid, const std::string command);
Job* find_job(int job_id);
void clean_jobs();
void cmd_jobs();
void cmd_bg(int job_id);
void cmd_fg(int job_id);