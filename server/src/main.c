#include <stdio.h>

void run(int argc, char *argv[]);


int main(int argc, char *argv[]) {
    printf("Simple web server is starting...\n");
    run(argc, argv);
    printf("Simple web server stop.\n");
    return 0;
}
