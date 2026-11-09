#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "game.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HANDLE original_output, game_output, keyboard;
static DWORD original_input_mode;
static int input_saved;
static volatile LONG stop_requested;

static BOOL WINAPI request_stop(DWORD event)
{
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
    InterlockedExchange(&stop_requested, 1);
    return TRUE;
}

int terminal_stopping(void)
{
    return InterlockedCompareExchange(&stop_requested, 0, 0) != 0;
}

int terminal_open(void)
{
    DWORD output_mode;
    CONSOLE_CURSOR_INFO cursor = {25, FALSE};
    original_output = GetStdHandle(STD_OUTPUT_HANDLE);
    keyboard = GetStdHandle(STD_INPUT_HANDLE);
    if (!GetConsoleMode(keyboard, &original_input_mode)) return 0;
    input_saved = 1;

    game_output = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    if (game_output == INVALID_HANDLE_VALUE) {
        game_output = NULL;
        return 0;
    }
    if (!GetConsoleMode(game_output, &output_mode) ||
        !SetConsoleMode(game_output, output_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) ||
        !SetConsoleMode(keyboard,
            (original_input_mode | ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_PROCESSED_INPUT)
            & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_QUICK_EDIT_MODE)) ||
        !SetConsoleCursorInfo(game_output, &cursor) ||
        !SetConsoleCtrlHandler(request_stop, TRUE) ||
        !SetConsoleActiveScreenBuffer(game_output)) return 0;
    return 1;
}

void terminal_close(void)
{
    if (input_saved) SetConsoleMode(keyboard, original_input_mode);
    if (original_output) SetConsoleActiveScreenBuffer(original_output);
    if (game_output) CloseHandle(game_output);
    SetConsoleCtrlHandler(request_stop, FALSE);
    game_output = NULL;
    input_saved = 0;
}

int terminal_size(int *width, int *height)
{
    CONSOLE_SCREEN_BUFFER_INFO size;
    if (!GetConsoleScreenBufferInfo(game_output, &size)) return 0;
    *width = size.srWindow.Right - size.srWindow.Left + 1;
    *height = size.srWindow.Bottom - size.srWindow.Top + 1;
    return 1;
}

int terminal_key(int timeout_ms)
{
    INPUT_RECORD record;
    DWORD count;
    DWORD ready = WaitForSingleObject(keyboard, (DWORD)timeout_ms);
    if (ready == WAIT_TIMEOUT) return 0;
    if (ready != WAIT_OBJECT_0 || !ReadConsoleInputA(keyboard, &record, 1, &count)) return -1;
    if (count != 1 || record.EventType != KEY_EVENT || !record.Event.KeyEvent.bKeyDown) return 0;

    switch (record.Event.KeyEvent.wVirtualKeyCode) {
    case VK_ESCAPE: return KEY_ESCAPE;
    case VK_RETURN: return KEY_ENTER;
    case VK_BACK: return KEY_BACKSPACE;
    case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT: return 0;
    default: return (unsigned char)record.Event.KeyEvent.uChar.AsciiChar;
    }
}

int terminal_draw(const char *text)
{
    const char *clear = "\033[H\033[2J";
    DWORD written;
    DWORD length = (DWORD)strlen(text);
    return WriteConsoleA(game_output, clear, (DWORD)strlen(clear), &written, NULL) &&
        written == (DWORD)strlen(clear) &&
        WriteConsoleA(game_output, text, length, &written, NULL) && written == length;
}

#else
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

static struct termios original_input;
static int input_saved;
static volatile sig_atomic_t stop_requested;

static void request_stop(int event)
{
    (void)event;
    stop_requested = 1;
}

int terminal_stopping(void)
{
    return stop_requested != 0;
}

int terminal_open(void)
{
    struct termios mode;
    struct sigaction action;
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) ||
        tcgetattr(STDIN_FILENO, &original_input) != 0) return 0;

    mode = original_input;
    mode.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    mode.c_lflag |= ISIG;
    mode.c_iflag &= (tcflag_t)~(IXON | ICRNL);
    mode.c_cc[VMIN] = 0;
    mode.c_cc[VTIME] = 0;
    memset(&action, 0, sizeof action);
    action.sa_handler = request_stop;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) || sigaction(SIGTERM, &action, NULL) ||
        sigaction(SIGQUIT, &action, NULL) || sigaction(SIGTSTP, &action, NULL) ||
        tcsetattr(STDIN_FILENO, TCSANOW, &mode)) return 0;
    input_saved = 1;
    return fputs("\033[?1049h\033[?25l", stdout) >= 0 && fflush(stdout) == 0;
}

void terminal_close(void)
{
    if (!input_saved) return;
    tcsetattr(STDIN_FILENO, TCSANOW, &original_input);
    fputs("\033[?25h\033[?1049l", stdout);
    fflush(stdout);
    input_saved = 0;
}

int terminal_size(int *width, int *height)
{
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0 || !size.ws_col || !size.ws_row) return 0;
    *width = size.ws_col;
    *height = size.ws_row;
    return 1;
}

static int read_byte(int timeout_ms)
{
    fd_set readers;
    struct timeval wait = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    unsigned char key;
    FD_ZERO(&readers);
    FD_SET(STDIN_FILENO, &readers);
    int ready = select(STDIN_FILENO + 1, &readers, NULL, NULL, &wait);
    if (ready < 0) return errno == EINTR ? 0 : -1;
    if (!ready) return 0;
    int count = (int)read(STDIN_FILENO, &key, 1);
    if (count < 0 && errno == EINTR) return 0;
    return count == 1 ? key : -1;
}

int terminal_key(int timeout_ms)
{
    int key = read_byte(timeout_ms);
    if (key == '\r' || key == '\n') return KEY_ENTER;
    if (key == 127 || key == '\b') return KEY_BACKSPACE;
    if (key != 27) return key;

    key = read_byte(20);
    if (key != '[' && key != 'O') return KEY_ESCAPE;
    /* Consume terminal key sequences so their letters cannot become movement. */
    for (int i = 0; i < 16; ++i) {
        key = read_byte(20);
        if (key <= 0 || (key >= 0x40 && key <= 0x7e)) break;
    }
    return 0;
}

int terminal_draw(const char *text)
{
    return fputs("\033[H\033[2J", stdout) >= 0 && fputs(text, stdout) >= 0 && fflush(stdout) == 0;
}
#endif
