/**
 * @file file_hander.c
 * @brief This file contains logger code, responsible for documenting data and
 * creating a directory if necessary.
 */
#include "file_handler.h"

/**
 * @brief Logs the round-trip time (RTT) data to a file.
 *
 * The function logs the RTT data to a file with a timestamped filename.
 *
 * @param data Array of RTT data in milliseconds.
 * @param data_size Size of the RTT data array.
 * @param time Pointer to the time_t structure representing the current time.
 * @return 0 on success, -1 on file open failure.
 */
int log_rtt(const double data[], const int data_size, const time_t *time) {
    // Convert epoch time to mmdd_hh:mm:ss
    char buf[FILENAMESIZE];
    strftime(buf, FILENAMESIZE, "./logs/rtt_%m%d_%H%M%S", localtime(time));

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

/**
 * @brief Creates a directory if it does not exist.
 *
 * The function checks if the specified directory exists, and if not, creates
 * it.
 *
 * @param directory Path to the directory to be created.
 * @return 0 on success, -1 on directory creation failure.
 */
int create_dir(const char *directory) {
    struct stat info = {0};  // This is the same as using memset()

    if (stat(directory, &info) == -1) {  // Directory doesn't exist
        if (mkdir(directory, URW_GR_OR) == -1) {
            perror("mkdir");
            return -1;
        }
    }

    return 0;
}
