#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "param.h"

ServerParams parse_args(int argc, char *argv[]) {
    ServerParams params = {0, 0, NULL}; // Initialize to 0

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--THREAD_NUM") == 0 || strcmp(argv[i], "-T") == 0) {
            if (i + 1 < argc) {
                params.THREAD_NUM = atoi(argv[++i]);
            } else {
                printf("Error: The key '%s' requires a value.\n", argv[i]);
                printf("Usage: --THREAD_NUM <int> or -T <int>\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "--BUFFER_SIZE") == 0 || strcmp(argv[i], "-B") == 0) {
            if (i + 1 < argc) {
                params.BUFFER_SIZE = atoi(argv[++i]);
            } else {
                printf("Error: The key '%s' requires a value.\n", argv[i]);
                printf("Usage: --BUFFER_SIZE <int> or -B <int>\n");
                exit(1);
            }
        } else if (i + 1 < argc) { // If no key is provided, assume the values are for THREAD_NUM and BUFFER_SIZE
            params.THREAD_NUM = atoi(argv[i]);
            params.BUFFER_SIZE = atoi(argv[++i]);
        } else {
            printf("Error: Invalid argument '%s'.\n", argv[i]);
            printf("Usage: --THREAD_NUM <int> or -T <int> or --BUFFER_SIZE <int> or -B <int> or <int> <int>\n");
            exit(1);
        }
    }

    return params;
}