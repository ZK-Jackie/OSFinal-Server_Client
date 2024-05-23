#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "param.h"

ClientParams parse_args(int argc, char *argv[]) {
    ClientParams params = {0, 0, NULL}; // Initialize to 0

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--THREAD_NUM") == 0 || strcmp(argv[i], "-T") == 0) {
            if (i + 1 < argc) {
                params.THREAD_NUM = atoi(argv[++i]);
            } else {
                printf("Error: The key '%s' requires a value.\n", argv[i]);
                printf("Usage: --THREAD_NUM <int> or -T <int>\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "--WORKLOAD") == 0 || strcmp(argv[i], "-W") == 0) {
            if (i + 1 < argc) {
                params.WORKLOAD = atoi(argv[++i]);
            } else {
                printf("Error: The key '%s' requires a value.\n", argv[i]);
                printf("Usage: --WORKLOAD <int> or -W <int>\n");
                exit(1);
            }
        } else if (i + 1 < argc) { // If no key is provided, assume the values are for THREAD_NUM and BUFFER_SIZE
            params.THREAD_NUM = atoi(argv[i]);
            params.WORKLOAD = atoi(argv[++i]);
        } else {
            printf("Error: Invalid argument '%s'.\n", argv[i]);
            printf("Usage: --THREAD_NUM <int> or -T <int> or --WORKLOAD <int> or -W <int> or <int> <int>\n");
            exit(1);
        }
    }

    return params;
}