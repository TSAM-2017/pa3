#include <stdlib.h>

#include "http_server.h"



int main(int argc, char *argv[]) {
    if (argc < 1) {
        error_out("Too few arguments, need http port")
    }

    int port = atoi(argv[1]);
    HE *server = malloc(sizeof(HE));
    set_up(server, port);
    loop_accepting(server);
    shut_down(server);
    free(server);

    return 0;
}
