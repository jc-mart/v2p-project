#ifndef PROCESS_HELPER_H
#define PROCESS_HELPER_H

#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

void sigchld_handler(int s);

#endif