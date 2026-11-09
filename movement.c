#include "game.h"

#include <stdio.h>
#include <string.h>

void move_player(Game *game, int number, char key)
{
    if (number < 0 || number > 1 || game->finished) return;

    int dx = 0, dy = 0;
    switch (key) {
    case 'w': case 'W': dy = -1; break;
    case 's': case 'S': dy = 1; break;
    case 'a': case 'A': dx = -1; break;
    case 'd': case 'D': dx = 1; break;
    default: return;
    }

    Player *player = &game->players[number];
    player->dx = dx;
    player->dy = dy;
    player->prompt = PROMPT_NONE;
    char tile = map_cell(game->map, number, player->x + dx,
                         player->y + dy, game->passage);
    if (tile == '.' || tile == '*') {
        player->x += dx;
        player->y += dy;
    }
}

char facing_object(const Game *game, int number)
{
    if (number < 0 || number > 1) return '#';
    const Player *player = &game->players[number];
    return map_cell(game->map, number, player->x + player->dx,
                    player->y + player->dy, game->passage);
}

int control_key(Controls *controls, int key, char command[TEXT_SIZE])
{
    command[0] = '\0';
    if (key == KEY_ESCAPE) {
        controls->mode = MODE_PLAY;
        controls->length = 0;
        controls->text[0] = '\0';
        strcpy(command, "cancel");
        return 1;
    }

    if (controls->mode == MODE_ANSWER) {
        if (key == KEY_ENTER) {
            snprintf(command, TEXT_SIZE, "answer %.150s", controls->text);
            controls->mode = MODE_PLAY;
            controls->length = 0;
            controls->text[0] = '\0';
            return 1;
        }
        if (key == KEY_BACKSPACE && controls->length > 0) {
            controls->text[--controls->length] = '\0';
        } else if (key >= 32 && key <= 126 && controls->length < 150) {
            controls->text[controls->length++] = (char)key;
            controls->text[controls->length] = '\0';
        }
        return 0;
    }

    switch (key) {
    case 'q': case 'Q': return -1;
    case 'w': case 'W': case 'a': case 'A':
    case 's': case 'S': case 'd': case 'D': case 'e': case 'E':
        command[0] = (char)key;
        command[1] = '\0';
        return 1;
    default: return 0;
    }
}
