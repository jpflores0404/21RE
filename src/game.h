#ifndef GAME_H
#define GAME_H
#include "../third_party/cJSON.h"
#define ROOM_W 29
#define ROOM_H 11
#define NOTE_COUNT 16
typedef struct { int x, y, facing; int ready, exit_ready; char notes[NOTE_COUNT][256]; int note_count; char feedback[512]; char active_object[32]; } GamePlayer;
typedef struct { int map, started, words, cabinet, hatch, passage, calibrated; int dials[3]; GamePlayer players[2]; char chat[160]; } Game;
/* Facing: 0 north, 1 east, 2 south, 3 west. Player IDs are zero based internally. */
void game_init(Game *game);
int game_action(Game *game, int player, const cJSON *action);
cJSON *game_view(const Game *game, int player);
int content_room(int map, int player, char rows[ROOM_H][ROOM_W + 1], int passage);
const char *content_room_name(int map, int player);
int game_tests(void);
#endif
