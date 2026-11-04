#include "demo.h"

#include <stdio.h>
#include <string.h>

static int quitting(const char *command)
{
    return strcmp(command, "q") == 0 || strcmp(command, "Q") == 0;
}

static int refresh(const Game *game, Socket partner, char previous[2][SCREEN_SIZE])
{
    char screen[SCREEN_SIZE];
    draw_screen(game, 0, screen);
    if (strcmp(screen, previous[0]) != 0) {
        show_screen(screen);
        strcpy(previous[0], screen);
    }

    draw_screen(game, 1, screen);
    if (strcmp(screen, previous[1]) != 0) {
        char message[SCREEN_SIZE + 8];
        /* Map number, screen lines, then a line containing only a dot. */
        snprintf(message, sizeof message, "%d\n%s.\n", game->map, screen);
        if (!send_text(partner, message)) return 0;
        strcpy(previous[1], screen);
    }
    return 1;
}

static void host_game(Socket partner)
{
    Game game;
    Line keyboard = {0}, network = {0};
    char previous[2][SCREEN_SIZE] = {{0}};
    start_game(&game);
    while (refresh(&game, partner, previous)) {
        if (game.finished) {
            send_text(partner, "done\n");
            return;
        }

        int local = input_line(&keyboard);
        if (local < 0 || (local == 1 && quitting(keyboard.text))) {
            send_text(partner, "quit\n");
            return;
        }
        if (local == 1) play_command(&game, 0, keyboard.text);

        int remote = receive_line(partner, &network);
        if (remote < 0 || (remote == 1 && strcmp(network.text, "quit") == 0)) break;
        if (remote == 1 && network.text[0] == '0' + game.map && network.text[1] == ' ')
            play_command(&game, 1, network.text + 2);
    }
    puts("Player 2 disconnected. Session ended.");
}

static void join_game(Socket host)
{
    Line keyboard = {0}, network = {0};
    char screen[SCREEN_SIZE] = "";
    size_t used = 0;
    int map = 0, incoming_map = 0;

    puts("Connected. Waiting for your room...");
    for (;;) {
        int local = input_line(&keyboard);
        if (local < 0 || (local == 1 && quitting(keyboard.text))) {
            send_text(host, "quit\n");
            return;
        }
        if (local == 1 && map != 0) {
            char message[LINE_SIZE];
            snprintf(message, sizeof message, "%d %.156s\n", map, keyboard.text);
            if (!send_text(host, message)) break;
        }

        int remote = receive_line(host, &network);
        if (remote < 0) break;
        if (remote == 0) continue;

        if (incoming_map == 0) {
            if (strcmp(network.text, "done") == 0) return;
            if (strcmp(network.text, "quit") == 0) break;
            if (strcmp(network.text, "1") == 0 || strcmp(network.text, "2") == 0)
                incoming_map = network.text[0] - '0';
            continue;
        }
        if (strcmp(network.text, ".") == 0) {
            show_screen(screen);
            map = incoming_map;
            incoming_map = 0;
            used = 0;
            screen[0] = '\0';
            continue;
        }
        size_t length = strlen(network.text);
        if (used + length + 2 > sizeof screen) break;
        memcpy(screen + used, network.text, length);
        used += length;
        screen[used++] = '\n';
        screen[used] = '\0';
    }
    puts("Host disconnected. Session ended.");
}

int main(void)
{
    char choice[LINE_SIZE];
    /* Keep menu reads from taking bytes that the live keyboard loop needs. */
    setvbuf(stdin, NULL, _IONBF, 0);
    if (!net_start()) {
        fputs("Could not start sockets.\n", stderr);
        return 1;
    }

    for (;;) {
        puts("\n2R1E 50\n\n1. Host game\n2. Join game\n3. Quit\n");
        printf("Choice (Enter to confirm): ");
        fflush(stdout);
        if (!read_prompt(choice, sizeof choice) || strcmp(choice, "3") == 0) break;

        Socket partner;
        if (strcmp(choice, "1") == 0) {
            partner = host_player();
            if (partner == NO_SOCKET) puts("Could not host. Check whether port 5050 is in use.");
            else host_game(partner);
        } else if (strcmp(choice, "2") == 0) {
            char address[LINE_SIZE];
            printf("Host's IPv4 address: ");
            fflush(stdout);
            if (!read_prompt(address, sizeof address)) break;
            partner = join_host(address);
            if (partner == NO_SOCKET) puts("Could not join. Check the host IP and TCP port 5050.");
            else join_game(partner);
        } else {
            puts("Choose 1, 2, or 3.");
            continue;
        }
        close_socket(partner);
    }
    net_stop();
    return 0;
}
