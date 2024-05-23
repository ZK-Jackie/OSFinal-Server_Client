#include "utils/param.h"

void run(ServerParams params);


int main(int argc, char *argv[]) {
    run(parse_args(argc, argv));
    return 0;
}
