#pragma once

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
void setup_signals();