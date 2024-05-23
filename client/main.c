#include "utils/param.h"

void run(ClientParams params);


int main(int argc, char *argv[]) {
    run(parse_args(argc, argv));
    return 0;
}