#include "jobs.h"
#include <iostream>
#include <algorithm>
#include <sys/wait.h>
#include <signal.h>

std::vector<Job> job_list;
pid_t foreground_pid = -1;
int next_job_id = 1;

void add_job(pid_t pid, const std::string command) {
    Job job;
    job.job_id = next_job_id++;
    job.pid = pid;
    job.command = command;
    job.state = Job::RUNNING;
    job_list.push_back(job);
}

Job* find_job(int job_id) {
    for (auto& job : job_list) {
        if (job.job_id == job_id) return &job;
    }

    return nullptr;
}

void clean_jobs() {
    job_list.erase(
        std::remove_if(job_list.begin(), job_list.end(), 
            [](const Job& j) { return j.state == Job::DONE; }),
        job_list.end()
    );
}

void cmd_jobs() {
    for (auto& job : job_list) {
        std::string state_str;
        if (job.state == Job::RUNNING) state_str = "Running";
        else if (job.state == Job::STOPPED) state_str = "Stopped";
        else state_str = "Done";

        std::cout << "[" << job.job_id << "]  " << state_str << "    " << job.command << "\n";
    }
}

void cmd_bg(int job_id) {
    Job* job = find_job(job_id);
    if (!job) {
        std::cerr << "bg: job " << job_id << " not found\n";
        return;
    }

    kill(job->pid, SIGCONT);
    job->state = Job::RUNNING;
    std::cout << "[" << job->job_id << "]+ " << job->command << " &\n";
}

void cmd_fg(int job_id) {
    Job* job = find_job(job_id);
    if (!job) {
        std::cerr << "fg: job " << job_id << " not found\n";
        return;
    }
    kill(job->pid, SIGCONT);
    foreground_pid = job->pid;
    std::cout << job->command << "\n";
    waitpid(job->pid, nullptr, 0);
    foreground_pid = -1;
    job->state = Job::DONE;
    clean_jobs();
}