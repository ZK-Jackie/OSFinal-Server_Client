#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "param.h"

// Usage 提示
const char USAGE[] = "\nUsage:\n"
                    "\tSet the number of threads and buffer size:\n"
                    "\t\t<int> <int>\n"
                    "\t\tif no key is provided, the first value is assumed to be the number of threads and the second value is assumed to be the buffer size\n"
                    "\tSet the number of threads:\n"
                    "\t\t--threads <int> or -T <int>\n"
                    "\tSet the buffer size:\n"
                    "\t\t--buffer <int> or -B <int> or <int> <int>\n"
                    "\n\tif any value is not provided, the default value is 5 threads and 10 buffer size\n";


/**
 * 转换命令行参数
 * @param argc
 * @param argv
 * @return
 */
ServerParams parseArgs(int argc, char *argv[]) {
    ServerParams params = {0, 0, NULL}; // Initialize to 0
    if (argc == 1) {
        printf("%s", USAGE);
        exit(1);
    }

    char *strtolEndPtr;  // 辅助检查参数有效性
    char isErr = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--threads") == 0 ||
            strcmp(argv[i], "-T") == 0 ||
            strcmp(argv[i], "-t") == 0 ||
            strcmp(argv[i], "--thread") == 0){
            if (i + 1 < argc) {
                params.THREAD_NUM = (int)strtol(argv[++i], &strtolEndPtr, 10);
                if (*strtolEndPtr != '\0' || errno == ERANGE || errno == EINVAL) {
                    printf("Error: Invalid argument '%s'.\n", argv[i]);
                    ++isErr;
                }
            } else {
                printf("Error: Invalid argument '%s'.\n", argv[i]);
                ++isErr;
            }
        } else if (strcmp(argv[i], "--buffer") == 0 ||
                    strcmp(argv[i], "-B") == 0 ||
                    strcmp(argv[i], "-b") == 0) {
            if (i + 1 < argc) {
                params.BUFFER_SIZE = (int)strtol(argv[++i], &strtolEndPtr, 10);
                if (*strtolEndPtr != '\0' || errno == ERANGE || errno == EINVAL) {
                    printf("Error: Invalid argument '%s'.\n", argv[i]);
                    ++isErr;
                }
            } else {
                printf("Error: Invalid argument '%s'.\n", argv[i]);
                ++isErr;
            }
        } else if (i + 1 < argc) { // If no key is provided, assume the values are for THREAD_NUM and BUFFER_SIZE
            params.THREAD_NUM = (int)strtol(argv[i], &strtolEndPtr, 10);
            if (*strtolEndPtr != '\0' || errno == ERANGE || errno == EINVAL) {
                printf("Error: Invalid argument '%s'.\n", argv[i]);
                ++isErr;
            }
            params.BUFFER_SIZE = (int)strtol(argv[++i], &strtolEndPtr, 10);
            if (*strtolEndPtr != '\0' || errno == ERANGE || errno == EINVAL) {
                printf("Error: Invalid argument '%s'.\n", argv[i]);
                ++isErr;
            }
        } else {
            printf("Error: Invalid argument '%s'.\n", argv[i]);
            ++isErr;
        }
    }
    if (isErr) {
        printf("%s", USAGE);
        exit(1);
    }

    return params;
}