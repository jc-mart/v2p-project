#include "file_handler.h"

// TODO make sure in the main function to ensure that a log directory is present
int log_rtt(const double *data[], const int data_size, const time_t *time) {
    // Convert epoch time to mmdd_hh:mm:ss
    char buf[FILENAMESIZE];
    strftime(buf, FILENAMESIZE, "./logs/rtt_%m%d_%H%M%S", localtime(&time));

    FILE *file = fopen(buf, "w");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    fprintf(file,
            "rtt for %s\n \
            pass no., time (ms)\n",
            buf + 4);
    for (int i = 0; i < data_size; i++) {
        fprintf(file, "%d, %f\n", i, data[i]);
    }
    fclose(file);

    return 0;
}

int create_dir(const char *directory) {
    struct stat info = {0}; // This is the same as using memset()

    if (stat(directory, &info) == -1) { // Directory doesn't exist
        if (mkdir(directory, URW_GR_OR) == -1) {
            perror("mkdir");
            return -1;
        }
    }
    
    return 0;
}