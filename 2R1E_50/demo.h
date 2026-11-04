#ifndef DEMO_H
#define DEMO_H

#include <stddef.h>
#include <stdint.h>

#define LINE_SIZE 160
#define SCREEN_SIZE 1600
#define PORT 5050

typedef intptr_t Socket;
#define NO_SOCKET ((Socket)-1)

typedef struct {
    char text[LINE_SIZE];
    size_t used;
    int too_long;
} Line;

typedef struct {
    int x, y, dx, dy, at_exit;
    char clue[LINE_SIZE];
} Player;

typedef struct {
    int map, words, cabinet, hatch, passage, calibrated, finished;
    int dials[3];
    Player player[2];
    char chat[LINE_SIZE];
} Game;

int net_start(void);
void net_stop(void);
Socket host_player(void);
Socket join_host(const char *address);
void close_socket(Socket socket);
int send_text(Socket socket, const char *text);
int receive_line(Socket socket, Line *line);

int add_character(Line *line, int key);
int input_line(Line *line);
int read_prompt(char *text, size_t capacity);
void show_screen(const char *screen);

void start_game(Game *game);
void play_command(Game *game, int player, const char *command);
void draw_screen(const Game *game, int player, char screen[SCREEN_SIZE]);

#endif
