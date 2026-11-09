#ifndef GAME_H
#define GAME_H

#include <stddef.h>
#include <stdint.h>

#define MAP_WIDTH 49
#define MAP_HEIGHT 17
#define TEXT_SIZE 192
#define CLUE_SIZE 256
#define SCREEN_SIZE 4096

enum { PROMPT_NONE, PROMPT_WORDS, PROMPT_CABINET, PROMPT_HATCH, PROMPT_DIALS };
enum { MODE_PLAY, MODE_ANSWER };
enum { KEY_ESCAPE = 256, KEY_ENTER, KEY_BACKSPACE };

typedef struct {
    int x, y, dx, dy, at_exit, prompt;
    unsigned int reply;
    char clue[CLUE_SIZE];
} Player;

typedef struct {
    int map, words, cabinet, hatch, passage, calibrated;
    int clearance, authorization, release, finished;
    int dials[3];
    Player players[2];
} Game;

typedef struct {
    int map, prompt, finished;
    unsigned int reply;
    char screen[SCREEN_SIZE];
} View;

typedef struct {
    int mode, prompt;
    char text[TEXT_SIZE];
    size_t length;
} Controls;

typedef intptr_t Socket;
#define NO_SOCKET ((Socket)-1)
typedef struct {
    Socket socket;
    char incoming[8192], outgoing[8192], latest[SCREEN_SIZE + 64];
    size_t received, queued, sent, latest_size;
} Peer;

char map_cell(int map, int player, int x, int y, int passage);
const char *map_room(int map, int player, int x, int y);
const char *map_description(int map, int player, char object);

void move_player(Game *game, int player, char key);
char facing_object(const Game *game, int player);
int control_key(Controls *controls, int key, char command[TEXT_SIZE]);

void start_game(Game *game);
void apply_action(Game *game, int player, const char *command);

int terminal_open(void);
void terminal_close(void);
int terminal_key(int timeout_ms);
int terminal_size(int *width, int *height);
int terminal_draw(const char *text);
int terminal_stopping(void);

int net_start(void);
void net_stop(void);
int valid_address(const char *address);
int parse_port(const char *text, unsigned short *port);
void local_addresses(char *text, size_t capacity);
const char *net_error(void);
Socket socket_listen(const char *address, unsigned short port);
Socket socket_accept(Socket listener);
Socket socket_connect(const char *address, unsigned short port);
void socket_close(Socket socket);
void peer_open(Peer *peer, Socket socket);
void peer_close(Peer *peer);
int peer_pump(Peer *peer);
int peer_line(Peer *peer, char line[TEXT_SIZE]);
int peer_queue(Peer *peer, const char *text);
int peer_screen(Peer *peer, const View *view);

void make_view(const Game *game, int player, View *view);
int show_view(const View *view, const Controls *controls);

#endif
