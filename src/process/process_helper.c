/**
 * @file process_helper.c
 * @brief This file contains helper functions for process management, such as
 * signal handling.
 */
#include "process_helper.h"

/**
 * @brief Signal handler for SIGCHLD.
 *
 * The function handles the SIGCHLD signal, which is sent to a parent process
 * when a child process terminates. It reaps zombie processes by calling waitpid
 * in a loop until there are no more child processes to reap.
 * 
 * From Beej's C Networking book.
 *
 * @param s Signal number (unused).
 */
void sigchld_handler(int s) {
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}