#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

typedef struct { int x, y; } Player;

static int playable(int width, int height)
{
    return width >= 20 && height >= 6;
}

static void keep_inside(Player *player, int width, int height)
{
    if (!playable(width, height)) return;
    if (player->x < 1) player->x = 1;
    if (player->x > width - 2) player->x = width - 2;
    if (player->y < 2) player->y = 2;
    if (player->y > height - 2) player->y = height - 2;
}

static int move_player(Player *player, int key, int width, int height)
{
    Player next = *player;
    if (!playable(width, height)) return 0;
    switch (key) {
        case 'w': case 'W': --next.y; break;
        case 'a': case 'A': --next.x; break;
        case 's': case 'S': ++next.y; break;
        case 'd': case 'D': ++next.x; break;
        default: return 0;
    }
    keep_inside(&next, width, height);
    if (next.x == player->x && next.y == player->y) return 0;
    *player = next;
    return 1;
}

#ifdef _WIN32
static HANDLE original_output = INVALID_HANDLE_VALUE;
static HANDLE demo_output = INVALID_HANDLE_VALUE;
static HANDLE console_input = INVALID_HANDLE_VALUE;
static DWORD original_input_mode;
static int input_mode_saved;
static volatile LONG stop_requested;
static CONSOLE_SCREEN_BUFFER_INFO screen;

static BOOL WINAPI request_stop(DWORD event)
{
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
    InterlockedExchange(&stop_requested, 1);
    return TRUE;
}

static void terminal_close(void)
{
    if (input_mode_saved) SetConsoleMode(console_input, original_input_mode);
    if (original_output != INVALID_HANDLE_VALUE)
        SetConsoleActiveScreenBuffer(original_output);
    if (demo_output != INVALID_HANDLE_VALUE) {
        CloseHandle(demo_output);
        demo_output = INVALID_HANDLE_VALUE;
    }
    SetConsoleCtrlHandler(request_stop, FALSE);
}

static int terminal_open(void)
{
    CONSOLE_CURSOR_INFO cursor = { 25, FALSE };
    CONSOLE_SCREEN_BUFFER_INFO original_screen;
    original_output = GetStdHandle(STD_OUTPUT_HANDLE);
    console_input = GetStdHandle(STD_INPUT_HANDLE);
    if (!GetConsoleMode(console_input, &original_input_mode) ||
        !GetConsoleScreenBufferInfo(original_output, &original_screen)) return 0;
    input_mode_saved = 1;
    demo_output = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    if (demo_output == INVALID_HANDLE_VALUE) return 0;
    if (!SetConsoleMode(console_input,
        (original_input_mode | ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS)
        & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_QUICK_EDIT_MODE))) return 0;
    if (!SetConsoleCursorInfo(demo_output, &cursor)) return 0;
    if (!SetConsoleCtrlHandler(request_stop, TRUE)) return 0;
    return SetConsoleActiveScreenBuffer(demo_output) != 0;
}

static int terminal_size(int *width, int *height)
{
    if (!GetConsoleScreenBufferInfo(demo_output, &screen)) return 0;
    *width = screen.srWindow.Right - screen.srWindow.Left + 1;
    *height = screen.srWindow.Bottom - screen.srWindow.Top + 1;
    return 1;
}

static int write_at(int x, int y, const char *text, int length)
{
    DWORD written;
    COORD at = { (SHORT)(screen.srWindow.Left + x),
                 (SHORT)(screen.srWindow.Top + y) };
    return WriteConsoleOutputCharacterA(demo_output, text, (DWORD)length,
        at, &written) && written == (DWORD)length;
}

static int read_key(void)
{
    INPUT_RECORD record;
    DWORD count;
    DWORD wait_result = WaitForSingleObject(console_input, 50);
    if (wait_result == WAIT_TIMEOUT) return 0;
    if (wait_result != WAIT_OBJECT_0 ||
        !ReadConsoleInputA(console_input, &record, 1, &count) || count != 1) return -1;
    if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown)
        return (unsigned char)record.Event.KeyEvent.uChar.AsciiChar;
    return 0;
}

static int stopping(void)
{
    return InterlockedCompareExchange(&stop_requested, 0, 0) != 0;
}
#else
static struct termios original_termios;
static int input_mode_saved;
static volatile sig_atomic_t stop_requested;

static void request_stop(int event)
{
    (void)event;
    stop_requested = 1;
}

static void terminal_close(void)
{
    if (input_mode_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
        fputs("\033[?25h\033[?1049l", stdout);
        fflush(stdout);
        input_mode_saved = 0;
    }
}

static int terminal_open(void)
{
    struct termios input;
    struct sigaction action;
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) ||
        tcgetattr(STDIN_FILENO, &original_termios) != 0) return 0;
    input = original_termios;
    input.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    input.c_iflag &= (tcflag_t)~(IXON | ICRNL);
    input.c_cc[VMIN] = 0;
    input.c_cc[VTIME] = 0;
    memset(&action, 0, sizeof action);
    action.sa_handler = request_stop;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) != 0 ||
        sigaction(SIGTERM, &action, NULL) != 0) return 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &input) != 0) return 0;
    input_mode_saved = 1;
    fputs("\033[?1049h\033[?25l", stdout);
    return fflush(stdout) == 0;
}

static int terminal_size(int *width, int *height)
{
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0 ||
        size.ws_col == 0 || size.ws_row == 0) return 0;
    *width = size.ws_col;
    *height = size.ws_row;
    return 1;
}

static int write_at(int x, int y, const char *text, int length)
{
    return printf("\033[%d;%dH", y + 1, x + 1) >= 0 &&
        fwrite(text, 1, (size_t)length, stdout) == (size_t)length;
}

static int read_key(void)
{
    fd_set inputs;
    struct timeval timeout = { 0, 50000 };
    unsigned char key;
    int ready;
    ssize_t count;
    FD_ZERO(&inputs);
    FD_SET(STDIN_FILENO, &inputs);
    ready = select(STDIN_FILENO + 1, &inputs, NULL, NULL, &timeout);
    if (ready < 0) return errno == EINTR ? 0 : -1;
    if (ready == 0) return 0;
    count = read(STDIN_FILENO, &key, 1);
    if (count < 0 && errno == EINTR) return 0;
    return count == 1 ? key : -1;
}

static int stopping(void)
{
    return stop_requested != 0;
}
#endif

static int render(Player player, int width, int height)
{
    char *row;
    int y, ok = 1;
    const char *title = "2R1E | WASD move | Q quit";
    const char *small = "Enlarge terminal";
    int title_length = (int)strlen(title);
    row = malloc((size_t)width);
    if (row == NULL) return 0;
    for (y = 0; y < height && ok; ++y) {
        memset(row, ' ', (size_t)width);
        if (playable(width, height)) {
            if (y == 1 || y == height - 1) memset(row, '#', (size_t)width);
            else if (y >= 2) { row[0] = '#'; row[width - 1] = '#'; }
            if (y == player.y) row[player.x] = '@';
        }
        if (y == 0) {
            const char *label = playable(width, height) ? title : small;
            int length = playable(width, height) ? title_length : (int)strlen(small);
            if (length > width) length = width;
            memcpy(row, label, (size_t)length);
        }
        /* Avoid writing the bottom-right cell, which can scroll POSIX terminals. */
        ok = write_at(0, y, row, y == height - 1 ? width - 1 : width);
    }
    free(row);
    return ok && fflush(stdout) == 0;
}

static int self_test(void)
{
    Player player = { 5, 5 };
    int failures = 0;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL at line %d: %s\n", __LINE__, #condition); ++failures; } } while (0)
    CHECK(move_player(&player, 'w', 80, 24) && player.x == 5 && player.y == 4);
    CHECK(move_player(&player, 'A', 80, 24) && player.x == 4 && player.y == 4);
    CHECK(move_player(&player, 's', 80, 24) && player.y == 5);
    CHECK(move_player(&player, 'D', 80, 24) && player.x == 5);
    CHECK(!move_player(&player, 'z', 80, 24) && player.x == 5 && player.y == 5);
    player.x = 1; player.y = 2;
    CHECK(!move_player(&player, 'a', 80, 24) && player.x == 1);
    CHECK(!move_player(&player, 'w', 80, 24) && player.y == 2);
    player.x = 78; player.y = 22;
    CHECK(!move_player(&player, 'd', 80, 24) && player.x == 78);
    CHECK(!move_player(&player, 's', 80, 24) && player.y == 22);
    keep_inside(&player, 20, 6);
    CHECK(player.x == 18 && player.y == 4);
    CHECK(!move_player(&player, 'w', 19, 5) && player.x == 18 && player.y == 4);
    keep_inside(&player, 80, 24);
    CHECK(player.x == 18 && player.y == 4);
    player.x = -1; player.y = -1;
    keep_inside(&player, 20, 6);
    CHECK(player.x == 1 && player.y == 2);
    CHECK(!playable(19, 6) && !playable(20, 5) && playable(20, 6));
#undef CHECK
    if (failures == 0) puts("Movement self-test passed (14 checks).");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv)
{
    Player player = { 1, 2 };
    int width = 0, height = 0, old_width = 0, old_height = 0;
    int key, dirty = 1, result = EXIT_SUCCESS;
    if (argc == 2 && strcmp(argv[1], "--self-test") == 0) return self_test();
    if (argc != 1) {
        fputs("Usage: movement_demo [--self-test]\n", stderr);
        return EXIT_FAILURE;
    }
    if (!terminal_open()) {
        terminal_close();
        fputs("Cannot open an interactive terminal. Run in Windows Terminal or a Linux terminal.\n", stderr);
        return EXIT_FAILURE;
    }
    while (!stopping()) {
        if (!terminal_size(&width, &height)) { result = EXIT_FAILURE; break; }
        if (width != old_width || height != old_height) {
            if (old_width == 0) { player.x = width / 2; player.y = height / 2; }
            keep_inside(&player, width, height);
            old_width = width; old_height = height; dirty = 1;
        }
        if (dirty && !render(player, width, height)) { result = EXIT_FAILURE; break; }
        dirty = 0;
        key = read_key();
        if (key < 0) { result = EXIT_FAILURE; break; }
        if (key == 'q' || key == 'Q') break;
        if (move_player(&player, key, width, height)) dirty = 1;
    }
    terminal_close();
    if (result != EXIT_SUCCESS) fputs("Terminal input/output failed; demo stopped.\n", stderr);
    return result;
}
