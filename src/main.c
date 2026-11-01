#include "app.h"
#include "game.h"
#include "platform.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    char executable[2048];
    if (argc == 2 && strcmp(argv[1], "--self-test") == 0) {
        int rules = game_tests(), protocol = protocol_tests();
        return rules || protocol ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    if (argc == 5 && strcmp(argv[1], "--server") == 0) {
        char *end;
        long port = strtol(argv[3], &end, 10);
        if (!*argv[3] || *end || port < 1024 || port > 65535 || strlen(argv[4]) > 64) return EXIT_FAILURE;
        return server_run(argv[2], (int)port, argv[4]);
    }
    if (argc != 1) { fputs("Usage: 2r1e [--self-test | --server IPv4 PORT SESSION]\n", stderr); return EXIT_FAILURE; }
    if (!platform_executable(executable, sizeof executable)) { fputs("Cannot locate this executable.\n", stderr); return EXIT_FAILURE; }
    return client_run(executable);
}
