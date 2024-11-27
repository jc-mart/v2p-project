#ifndef PROCESS_HELPER_H
#define PROCESS_HELPER_H

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

void sigchld_handler(int s);

#endif
