#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "param.h"
#include "global.h"
const char USAGE[] = "\nUsage:\n"
                     "\tSet the number of threads, workload and sleep time:\n"
                        "\t\t<int> <int> <int>\n"
                        "\t\tif no key is provided, assume the values are for number of thread, workload and sleep time\n"
                     "\tSet the number of threads:\n"
                        "\t\t--threads <int> or -T <int>\n"
                     "\tSet the workload:\n"
                        "\t\t--workload <int> or -W <int>\n"
                        "\t\t 0 for concurrent request and 1 for FIFO\n"
                     "\tSet the sleep time between requests:\n"
                        "\t\t--delay <int> or -D <int>\n"
                        "\t\tsleep time is an integer in seconds\n"
                    "\tif any key is not provided, defaulting:"
                    "\t\t- the number of threads to 5"
                    "\t\t- workload to 0"
                    "\t\t- sleep time to 2\n";


int parseInt(char *str) {
    char *strtolEndPtr;
    int num = (int)strtol(str, &strtolEndPtr, 10);
    if (*strtolEndPtr != '\0' || errno == ERANGE || errno == EINVAL) {
        printf("Error: Invalid argument '%s'.\n", str);
        exit(1);
    }
    return num;
}


ClientParams parseArgs(int argc, char *argv[]) {
    ClientParams params = {-1, -1, -1}; // Initialize to 0
    if (argc == 1) {
        printf("%s", USAGE);
        exit(1);
    }

    char isErr = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--thread") == 0 ||
            strcmp(argv[i], "-T") == 0 ||
            strcmp(argv[i], "--threads") == 0 ||
            strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                params.threadNum = parseInt(argv[++i]);
            } else {
                printf("Error: Invalid argument '%s'.\n", argv[i]);
                ++isErr;
            }
        } else if (strcmp(argv[i], "--workload") == 0 ||
                   strcmp(argv[i], "-W") == 0 ||
                   strcmp(argv[i], "-w") == 0) {
            if (i + 1 < argc) {
                params.workload = parseInt(argv[++i]);
            } else {
                printf("Error: Invalid argument '%s'.\n", argv[i]);
                ++isErr;
            }
        }else if (strcmp(argv[i], "--delay") == 0 ||
                  strcmp(argv[i], "-D") == 0 ||
                  strcmp(argv[i], "-d") == 0) {
            if (i + 1 < argc) {
                params.sleepTime = parseInt(argv[++i]);
            } else {
                printf("Error: Invalid argument '%s'.\n", argv[i]);
                ++isErr;
            }
        } else if (i + 2 < argc) {
            // If no key is provided, assume the values are for threadNum and workload
            params.threadNum = parseInt(argv[1]);
            params.workload = parseInt(argv[2]);
            params.sleepTime = parseInt(argv[3]);
        } else {
            printf("Error: Invalid argument '%s'.\n", argv[i]);
            ++isErr;
        }
    }
    if (params.threadNum == -1) {
        logger.info("Thread number not provided, defaulting to 5", LOG_WARNING);
        params.threadNum = 5;
    }
    if (params.workload == -1) {
        logger.info("Workload not provided, defaulting to 0", LOG_WARNING);
        params.workload = 0;
    }
    if (params.sleepTime == -1) {
        logger.info("Sleep time between requests not provided, defaulting to 2(seconds)", LOG_WARNING);
        params.sleepTime = 2;
    }

    return params;
}