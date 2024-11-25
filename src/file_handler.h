#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stdio.h>
#include <sys/stat.h>

#define FILENAMESIZE 23
#define URW_GR_OR 0644

int log_rtt(const double *data[], const int data_size, const time_t *time);
int create_dir(const char *directory);

#endif
