#include <stdio.h>

void run(int argc, char *argv[]);


int main(int argc, char *argv[]) {
    printf("** Simple Web Server **\n");
    run(argc, argv);
    printf("Simple web server stop.\n");
    return 0;
}
