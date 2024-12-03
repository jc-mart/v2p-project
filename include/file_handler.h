#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define FILENAMESIZE 40
#define PERMISSIONS 0701

int log_rtt(const double data[], const int data_size, const char dir[],
            const int dir_size, const time_t *time);
int create_dir(const char *directory);

#endif
