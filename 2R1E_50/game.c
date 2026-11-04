#include "demo.h"

#include <stdio.h>
#include <string.h>

#define WIDTH 29
#define HEIGHT 11

/* Each level has one room per player. Row y, column x. */
static const char rooms[4][HEIGHT][WIDTH + 1] = {
    { /* Gallery */
        "#############################",
        "#...........................#",
        "#....A.......B.......C......#",
        "#...........................#",
        "#...........................#",
        "#...........................#",
        "#.......D............H......#",
        "#...........................#",
        "#...........................#",
        "#...........................#",
        "##############E##############",
    },
    { /* Archive */
        "#############################",
        "#...........................#",
        "#.....N.............K.......#",
        "#...........................#",
        "#...........................#",
        "#...........................#",
        "#.......L...................#",
        "#...........................#",
        "#...........................#",
        "#...........................#",
        "##############E##############",
    },
    { /* Exhibition */
        "#############################",
        "#...................#.......#",
        "#...................#.......#",
        "#....A.......B......#...R...#",
        "#............*......#.......#",
        "#...................P.......#",
        "#...................#.......#",
        "#....C..............#.......#",
        "#...................#.......#",
        "#...................#.......#",
        "##############E##############",
    },
    { /* Maintenance */
        "#############################",
        "#...........................#",
        "#.....B.....................#",
        "#...................D.......#",
        "#...........................#",
        "#...........................#",
        "#...................T.......#",
        "#...........................#",
        "#...........................#",
        "#...........................#",
        "##############E##############",
    }
};

static const char *room_names[] = {"Gallery", "Archive", "Exhibition", "Maintenance"};

static char tile_at(const Game *game, int player, int x, int y)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return '#';
    char tile = rooms[(game->map - 1) * 2 + player][y][x];
    if (tile == 'P' && game->passage) return '.';
    return tile;
}

static void reset_players(Game *game)
{
    for (int i = 0; i < 2; ++i) {
        Player *player = &game->player[i];
        *player = (Player){0};
        player->x = 1;
        player->y = 1;
        player->dy = 1;
        strcpy(player->clue, "Walk up to a marked object, face it, then enter e.");
    }
}

void start_game(Game *game)
{
    *game = (Game){0};
    game->map = 1;
    reset_players(game);
}

static const char *use_exit(Game *game, int number)
{
    if ((game->map == 1 && !game->hatch) ||
        (game->map == 2 && !game->calibrated))
        return "Exit locked. Finish the room puzzles first.";
    game->player[number].at_exit = 1;
    if (!game->player[0].at_exit || !game->player[1].at_exit)
        return "Exit ready. Your partner needs to use their exit too.";
    if (game->map == 2) {
        game->finished = 1;
        return "Both players escaped. Maps 1 and 2 complete!";
    }
    game->map = 2;
    reset_players(game);
    return "You reached Map 2. Look for the blueprint and statues.";
}

static const char *inspect(Game *game, int player, char tile)
{
    if (tile == 'E') return use_exit(game, player);

    if (game->map == 1 && player == 0) {
        switch (tile) {
        case 'A': return "Eye painting: the watcher has number 6.";
        case 'B': return "Lantern painting: the light has number 2.";
        case 'C': return "Key painting: the means of passage has number 4.";
        case 'D': return "Equation desk: X = 14 - 9. Y = 3 x 3. Z = 8 + 4. Ask for the order.";
        case 'H': return game->cabinet ? "Hatch: enter answer followed by the three digits."
                                      : "Hatch locked. Your partner must open the cabinet first.";
        }
    } else if (game->map == 1) {
        switch (tile) {
        case 'N':
            if (game->words) return "Order: light, means of passage, watcher.";
            return "Three Blanks: what shines, what lets you through, and what watches?\n"
                   "Choices: 1 light, 2 passage, 3 watcher, 4 silence. Use answer plus three numbers.";
        case 'K':
            if (game->cabinet) return "Cabinet note: use Z, X, Y. Keep only the last digit of each.";
            return game->words ? "Cabinet: use answer followed by the three painting digits."
                               : "Reconstruct the Three Blanks note first.";
        case 'L': return "Archive shelf: share discoveries with your partner.";
        }
    } else if (player == 0) {
        switch (tile) {
        case 'A': return "Statue A faces east. Nothing opens.";
        case 'B':
            game->passage = 1;
            return "Statue B faces north beside the crack. The passage opens.";
        case 'C': return "Statue C faces north but is far from the crack.";
        case 'P': return "The passage is sealed. Check the statues.";
        case 'R': return "Receiver rule: A is 2, B is one above A, C is one above B.";
        }
    } else {
        switch (tile) {
        case 'B': return "Blueprint: use the north-facing statue closest to the cracked tile (*).";
        case 'D':
        case 'T': return "Calibration: use set A B C with values 0 to 4, then test.";
        }
    }
    return "Nothing to inspect here. Face a marked object.";
}

static const char *answer(Game *game, int player, char tile, const char *text)
{
    if (game->map == 1 && player == 1 && tile == 'N') {
        if (strcmp(text, "1 2 3") != 0) return "Try the Three Blanks again.";
        game->words = 1;
        return "Order restored: light, means of passage, watcher.";
    }
    if (game->map == 1 && player == 1 && tile == 'K' && game->words) {
        if (strcmp(text, "246") != 0) return "Wrong cabinet code. Compare the note and paintings.";
        game->cabinet = 1;
        return "Cabinet opened. Use Z, X, Y. Keep only the last digit of each.";
    }
    if (game->map == 1 && player == 0 && tile == 'H' && game->cabinet) {
        if (strcmp(text, "259") != 0) return "Wrong hatch code. Check the last digits and order.";
        game->hatch = 1;
        return "Hatch open. Both players can use their exits.";
    }
    return "Face the right puzzle and finish its earlier clues first.";
}

static const char *calibrate(Game *game, const char *command)
{
    if (strncmp(command, "set ", 4) == 0) {
        const char *values = command + 4;
        const char *hint = "Use three single digits from 0 to 4, like set 0 1 0.";
        if (strlen(values) != 5 || values[1] != ' ' || values[3] != ' ') return hint;
        for (int i = 0; i < 3; ++i) {
            char digit = values[2 * i];
            if (digit < '0' || digit > '4') return hint;
        }
        for (int i = 0; i < 3; ++i) game->dials[i] = values[2 * i] - '0';
        return "Dials updated. Enter test to send them to the receiver.";
    }
    if (!game->passage) return "Your partner needs to open the receiver passage first.";

    const char *feedback[3];
    int aligned = 1;
    for (int i = 0; i < 3; ++i) {
        int difference = game->dials[i] - (i + 2);
        if (difference != 0) aligned = 0;
        feedback[i] = difference < 0 ? "low" : difference > 0 ? "high" : "aligned";
    }
    snprintf(game->player[0].clue, sizeof game->player[0].clue,
             "Receiver: A %s, B %s, C %s.", feedback[0], feedback[1], feedback[2]);
    if (!aligned) return "Test sent. Ask your partner for the receiver feedback.";
    game->calibrated = 1;
    return "Signal aligned. Both players can use their exits.";
}

void play_command(Game *game, int number, const char *command)
{
    Player *player = &game->player[number];
    const char *feedback;
    if (strncmp(command, "say ", 4) == 0) {
        snprintf(game->chat, sizeof game->chat, "Player %d: %.140s", number + 1, command + 4);
        return;
    }

    if (strlen(command) == 1) {
        int dx = 0, dy = 0;
        switch (command[0]) {
        case 'w': case 'W': dy = -1; break;
        case 's': case 'S': dy = 1; break;
        case 'a': case 'A': dx = -1; break;
        case 'd': case 'D': dx = 1; break;
        }
        if (dx || dy) {
            player->dx = dx;
            player->dy = dy;
            char next = tile_at(game, number, player->x + dx, player->y + dy);
            if (next == '.' || next == '*') {
                player->x += dx;
                player->y += dy;
            }
            return;
        }
    }

    char tile = tile_at(game, number, player->x + player->dx, player->y + player->dy);
    if (strcmp(command, "e") == 0 || strcmp(command, "E") == 0)
        feedback = inspect(game, number, tile);
    else if (strncmp(command, "answer ", 7) == 0)
        feedback = answer(game, number, tile, command + 7);
    else if (strncmp(command, "set ", 4) == 0 || strcmp(command, "test") == 0)
        feedback = game->map == 2 && number == 1 && (tile == 'D' || tile == 'T')
                   ? calibrate(game, command) : "Face the dial panel or test switch first.";
    else
        feedback = "Unknown or too long. Use the commands below.";
    snprintf(player->clue, sizeof player->clue, "%s", feedback);
}

void draw_screen(const Game *game, int number, char screen[SCREEN_SIZE])
{
    const Player *player = &game->player[number];
    const char *room = room_names[(game->map - 1) * 2 + number];
    const char *facing = player->dx < 0 ? "west" : player->dx > 0 ? "east" :
                         player->dy < 0 ? "north" : "south";
    char drawing[HEIGHT * (WIDTH + 1) + 1];
    int index = 0;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x)
            drawing[index++] = x == player->x && y == player->y
                               ? '@' : tile_at(game, number, x, y);
        drawing[index++] = '\n';
    }
    drawing[index] = '\0';

    char dials[80] = "";
    if (game->map == 2 && number == 1)
        snprintf(dials, sizeof dials, "Dials: %d %d %d    set A B C / test\n",
                 game->dials[0], game->dials[1], game->dials[2]);
    snprintf(screen, SCREEN_SIZE,
             "2R1E 50  |  Map %d  |  Player %d  |  %s\n"
             "Facing %s   @ you   # wall   E exit\n\n"
             "%s\nClue: %s\n\nChat: %s\n\n%s"
             "w/a/s/d move   e inspect   answer TEXT   say TEXT   q quit\n"
             "Press Enter after each command.\n",
             game->map, number + 1, room, facing, drawing,
             game->finished ? "Both players escaped. Maps 1 and 2 complete!" : player->clue,
             game->chat[0] ? game->chat : "(no messages)", dials);
}
