#include "game.h"

#include <stdio.h>
#include <string.h>

static void tell(Game *game, int player, const char *text)
{
    snprintf(game->players[player].clue, CLUE_SIZE, "%s", text);
}

static void enter_map(Game *game)
{
    for (int i = 0; i < 2; ++i) {
        game->players[i] = (Player){0};
        game->players[i].x = 1;
        game->players[i].y = 1;
        game->players[i].dy = 1;
        tell(game, i, "Explore your rooms. Face a marked object and press E. Tell your partner what you find.");
    }
}

void start_game(Game *game)
{
    *game = (Game){0};
    game->map = 1;
    enter_map(game);
}

static int normalize(const char *input, char output[TEXT_SIZE])
{
    size_t length = 0;
    int space = 0;
    for (size_t i = 0; input[i]; ++i) {
        unsigned char letter = (unsigned char)input[i];
        if (letter == ' ') {
            if (length > 0) space = 1;
            continue;
        }
        if (letter < 32 || letter > 126) return 0;
        if (letter >= 'A' && letter <= 'Z') letter += 'a' - 'A';
        if (length + (size_t)space + 1 >= TEXT_SIZE) return 0;
        if (space) output[length++] = ' ';
        output[length++] = (char)letter;
        space = 0;
    }
    output[length] = '\0';
    return 1;
}

static void use_exit(Game *game, int player)
{
    int open;
    if (game->map == 1) open = game->hatch;
    else if (game->map == 2) open = game->calibrated;
    else open = game->clearance && game->authorization && game->release;
    if (!open) {
        tell(game, player, "Exit locked. Complete your team's puzzles and release controls first.");
        return;
    }

    game->players[player].at_exit = 1;
    tell(game, player, "Your exit is ready. Your partner must also use their exit control.");
    if (!game->players[0].at_exit || !game->players[1].at_exit) return;

    if (game->map < 3) {
        ++game->map;
        enter_map(game);
    } else {
        game->finished = 1;
        for (int i = 0; i < 2; ++i) {
            game->players[i].prompt = PROMPT_NONE;
            tell(game, i, "Both operators escaped. The facility is open. Press Enter to return to the main menu.");
        }
    }
}

static void inspect(Game *game, int player)
{
    Player *operator = &game->players[player];
    char object = facing_object(game, player);
    operator->prompt = PROMPT_NONE;

    if (object == 'E') {
        use_exit(game, player);
        return;
    }
    const char *text = map_description(game->map, player, object);

    if (game->map == 1 && player == 1 && object == 'N') {
        if (game->words) text = "Order restored: light, means of passage, watcher.";
        else {
            operator->prompt = PROMPT_WORDS;
            text = "First comes the ___. Then the means of ___. The ___ remains last. Clues: banishes shadows, a way through, a silent guardian. Type the three missing words in order.";
        }
    } else if (game->map == 1 && player == 1 && object == 'K') {
        if (game->cabinet) text = "Cabinet note: use Z, X, Y. Keep only the final digit of each result.";
        else if (!game->words) text = "Reconstruct the Three Blanks note first.";
        else {
            operator->prompt = PROMPT_CABINET;
            text = "Cabinet padlock: type the three painting digits in the note's order.";
        }
    } else if (game->map == 1 && player == 0 && object == 'H') {
        if (game->hatch) text = "Hatch authorized. Both operators can use their own exit controls.";
        else if (!game->cabinet) text = "Hatch inactive. Your partner must open the cabinet first.";
        else {
            operator->prompt = PROMPT_HATCH;
            text = "Hatch keypad: combine your equations with your partner's instruction. Type three digits.";
        }
    } else if (game->map == 2 && player == 0 && object == 'B') {
        game->passage = 1;
        text = "Statue B faces north beside the crack. The passage in the east wall opens.";
    } else if (game->map == 2 && player == 1 && (object == 'D' || object == 'T')) {
        operator->prompt = PROMPT_DIALS;
        text = "Signal calibration: type A B C as three digits from 0 to 4 separated by spaces. Enter tests them. Your partner sees the receiver feedback.";
    } else if (game->map == 3 && player == 0 && object >= '1' && object <= '4') {
        if (object == '4') {
            game->clearance = 1;
            text = "White Return valve accepted. Containment clearance granted. Activate authorization and the mechanical release.";
        } else {
            snprintf(operator->clue, CLUE_SIZE, "%s This valve is excluded from routine clearance. Compare the labels with your partner's record and try again.", text);
            return;
        }
    } else if (game->map == 3 && player == 0 && object == 'A') {
        if (!game->clearance) text = "Authorization locked. Obtain containment clearance first.";
        else {
            game->authorization = 1;
            text = "Authorization accepted. Ask your partner to operate the mechanical safety release.";
        }
    } else if (game->map == 3 && player == 1 && object == 'S') {
        if (!game->clearance) text = "Mechanical release locked. Your partner must obtain containment clearance.";
        else {
            game->release = 1;
            text = "Mechanical safety released. Both operators must finish their controls and use their own exits.";
        }
    }
    tell(game, player, text);
}

static void calibrate(Game *game, int player, const char *text)
{
    if (strlen(text) != 5 || text[1] != ' ' || text[3] != ' ') {
        tell(game, player, "Type three single digits from 0 to 4, separated by spaces.");
        return;
    }
    for (int i = 0; i < 3; ++i) {
        if (text[2 * i] < '0' || text[2 * i] > '4') {
            tell(game, player, "Each dial must be from 0 to 4.");
            return;
        }
    }
    for (int i = 0; i < 3; ++i) game->dials[i] = text[2 * i] - '0';
    if (!game->passage) {
        tell(game, player, "Receiver unavailable. Your partner must open the passage first.");
        return;
    }

    const char *feedback[3];
    int aligned = 1;
    for (int i = 0; i < 3; ++i) {
        int difference = game->dials[i] - (i + 2);
        if (difference != 0) aligned = 0;
        feedback[i] = difference < 0 ? "below target" : difference > 0 ? "above target" : "aligned";
    }
    snprintf(game->players[0].clue, CLUE_SIZE, "Receiver feedback: A %s, B %s, C %s.",
             feedback[0], feedback[1], feedback[2]);
    if (aligned) {
        game->calibrated = 1;
        game->players[player].prompt = PROMPT_NONE;
        tell(game, player, "Calibration accepted. Both operators can use their exit controls.");
    } else {
        tell(game, player, "Test sent. Ask your partner for the receiver feedback, then adjust your answer.");
    }
}

static void submit_answer(Game *game, int player, const char *input)
{
    Player *operator = &game->players[player];
    char object = facing_object(game, player);
    char text[TEXT_SIZE];
    if (!normalize(input, text)) {
        tell(game, player, "Answer is too long or contains unsupported characters.");
        return;
    }
    if (game->map == 1 && player == 1 && object == 'N' && operator->prompt == PROMPT_WORDS) {
        if (strcmp(text, "light passage watcher") != 0) {
            tell(game, player, "Try again. Use the words for the shining start, the way through, and the watching guardian.");
            return;
        }
        game->words = 1;
        tell(game, player, "Order restored: light, means of passage, watcher. Inspect the cabinet next.");
    } else if (game->map == 1 && player == 1 && object == 'K' &&
               operator->prompt == PROMPT_CABINET && game->words) {
        if (strcmp(text, "246") != 0) {
            tell(game, player, "Wrong cabinet code. Compare your note's order with the painting numbers.");
            return;
        }
        game->cabinet = 1;
        tell(game, player, "Cabinet opened. Use Z, X, Y. Keep only the final digit of each result.");
    } else if (game->map == 1 && player == 0 && object == 'H' &&
               operator->prompt == PROMPT_HATCH && game->cabinet) {
        if (strcmp(text, "259") != 0) {
            tell(game, player, "Wrong hatch code. Use the final digit of each result in your partner's order.");
            return;
        }
        game->hatch = 1;
        tell(game, player, "Hatch authorized. Both operators must use their own exit controls.");
    } else if (game->map == 2 && player == 1 && (object == 'D' || object == 'T') &&
               operator->prompt == PROMPT_DIALS) {
        calibrate(game, player, text);
        return;
    } else {
        tell(game, player, "Press E while facing the permitted puzzle before submitting an answer.");
        return;
    }
    operator->prompt = PROMPT_NONE;
}

void apply_action(Game *game, int player, const char *command)
{
    if (!game || player < 0 || player > 1 || !command || game->finished) return;
    if (strlen(command) == 1 && strchr("wasdWASD", command[0])) {
        move_player(game, player, command[0]);
    } else if (strcmp(command, "e") == 0 || strcmp(command, "E") == 0) {
        inspect(game, player);
    } else if (strcmp(command, "cancel") == 0) {
        game->players[player].prompt = PROMPT_NONE;
    } else if (strncmp(command, "answer ", 7) == 0) {
        submit_answer(game, player, command + 7);
    } else {
        tell(game, player, "Unknown action. Use WASD or E.");
    }
}
