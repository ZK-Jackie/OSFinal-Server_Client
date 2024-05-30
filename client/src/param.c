#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "param.h"
#include "global.h"


ClientParams parse_args(int argc, char *argv[]) {
    ClientParams params = {-1, -1, -1}; // Initialize to 0
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--thread") == 0 ||
            strcmp(argv[i], "-T") == 0 ||
            strcmp(argv[i], "--threads") == 0 ||
            strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                params.threadNum = atoi(argv[++i]);
            } else {
                printf("Error: The key '%s' requires a value.\n", argv[i]);
                printf("Usage: --thread <int> or -T <int>\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "--workload") == 0 ||
                   strcmp(argv[i], "-W") == 0 ||
                   strcmp(argv[i], "-w") == 0) {
            if (i + 1 < argc) {
                params.workload = atoi(argv[++i]);
            } else {
                printf("Error: The key '%s' requires a value.\n", argv[i]);
                printf("Usage: --workload <int> or -W <int>\n");
                exit(EXIT_FAILURE);
            }
        }else if (strcmp(argv[i], "--delay") == 0 ||
                  strcmp(argv[i], "-D") == 0 ||
                  strcmp(argv[i], "-d") == 0) {
            if (i + 1 < argc) {
                params.sleepTime = atoi(argv[++i]);
            } else {
                printf("Error: The key '%s' requires a value.\n", argv[i]);
                printf("Usage: --server <string> or -S <string>\n");
                exit(EXIT_FAILURE);
            }
        } else if (i + 1 < argc) {
            // If no key is provided, assume the values are for threadNum and workload
            params.threadNum = atoi(argv[i]);
            params.workload = atoi(argv[++i]);
        } else {
            printf("Error: Invalid argument '%s'.\n", argv[i]);
            printf("Usage:\n"
                   "\t--thread <int> or -T <int>\n"
                   "\t--workload <int> or -W <int>\n");
            exit(EXIT_FAILURE);
        }
    }
    if (argc == 1) {
        printf("OS-Simple Web Server-Client. Original by GDOUYJ.compsci-Jackie.\n\n"
               "Usage: ./client\t[--thread <int> | -T <int>]\n"
               "\t\t\t[--workload <int> | -W <int>]\n"
               "\t\t\t[--delay <int> or -D <int>]\n\n"
               "  -T number of threads to use, default is 5\n"
               "  -W workload to use, '0' for concurrent request and '1' for FIFO, default is 0\n"
               "  -D sleep seconds between requests, default is 2\n\n");
        exit(EXIT_SUCCESS);
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