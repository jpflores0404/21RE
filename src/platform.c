#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static HANDLE original_out, active_out, input_handle;
static DWORD old_input;
static int saved;
static volatile LONG stop_flag;
static BOOL WINAPI stop_handler(DWORD event)
{
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
    InterlockedExchange(&stop_flag, 1);
    return TRUE;
}
int platform_stopping(void) { return InterlockedCompareExchange(&stop_flag, 0, 0) != 0; }
uint64_t platform_millis(void) { return (uint64_t)GetTickCount64(); }
void platform_sleep(int milliseconds) { Sleep((DWORD)milliseconds); }
int term_open(void)
{
    DWORD output_mode;
    CONSOLE_CURSOR_INFO cursor = {25, FALSE};
    original_out = GetStdHandle(STD_OUTPUT_HANDLE);
    input_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (!GetConsoleMode(input_handle, &old_input)) return 0;
    saved = 1;
    active_out = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    if (active_out == INVALID_HANDLE_VALUE) { active_out = NULL; return 0; }
    if (!GetConsoleMode(active_out, &output_mode) ||
        !SetConsoleMode(active_out, output_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) ||
        !SetConsoleMode(input_handle, (old_input | ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_PROCESSED_INPUT)
            & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_QUICK_EDIT_MODE)) ||
        !SetConsoleCursorInfo(active_out, &cursor) || !SetConsoleCtrlHandler(stop_handler, TRUE) ||
        !SetConsoleActiveScreenBuffer(active_out)) return 0;
    return 1;
}
void term_close(void)
{
    if (saved) SetConsoleMode(input_handle, old_input);
    if (original_out) SetConsoleActiveScreenBuffer(original_out);
    if (active_out) { CloseHandle(active_out); active_out = NULL; }
    SetConsoleCtrlHandler(stop_handler, FALSE);
    saved = 0;
}
int term_size(int *width, int *height)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(active_out, &info)) return 0;
    *width = info.srWindow.Right - info.srWindow.Left + 1;
    *height = info.srWindow.Bottom - info.srWindow.Top + 1;
    return 1;
}
int term_key(int timeout_ms)
{
    INPUT_RECORD record;
    DWORD count, status = WaitForSingleObject(input_handle, (DWORD)timeout_ms);
    if (status == WAIT_TIMEOUT) return 0;
    if (status != WAIT_OBJECT_0 || !ReadConsoleInputA(input_handle, &record, 1, &count)) return -1;
    if (count != 1 || record.EventType != KEY_EVENT || !record.Event.KeyEvent.bKeyDown) return 0;
    switch (record.Event.KeyEvent.wVirtualKeyCode) {
        case VK_UP: return KEY_UP; case VK_DOWN: return KEY_DOWN;
        case VK_LEFT: return KEY_LEFT; case VK_RIGHT: return KEY_RIGHT;
        case VK_ESCAPE: return KEY_ESCAPE; case VK_RETURN: return KEY_ENTER;
        case VK_BACK: return KEY_BACKSPACE;
        /* shortcut: one move per key-down record; expand repeat counts if holding-key tests require it. */
        default: return (unsigned char)record.Event.KeyEvent.uChar.AsciiChar;
    }
}
int term_draw(const char *text)
{
    DWORD written;
    const char *clear = "\033[H\033[2J";
    DWORD length = (DWORD)strlen(text);
    return WriteConsoleA(active_out, clear, (DWORD)strlen(clear), &written, NULL) &&
        WriteConsoleA(active_out, text, length, &written, NULL) && written == length;
}
int platform_executable(char *path, int capacity)
{
    DWORD n = GetModuleFileNameA(NULL, path, (DWORD)capacity);
    return n > 0 && n < (DWORD)capacity;
}
int platform_spawn(Child *child, const char *executable, const char *address, int port, const char *session)
{
    STARTUPINFOA startup;
    PROCESS_INFORMATION process;
    char command[4096];
    int length = snprintf(command, sizeof command, "\"%s\" --server %s %d %s", executable, address, port, session);
    if (length < 0 || length >= (int)sizeof command) return 0;
    memset(&startup, 0, sizeof startup); startup.cb = sizeof startup;
    memset(&process, 0, sizeof process);
    if (!CreateProcessA(executable, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startup, &process)) return 0;
    CloseHandle(process.hThread);
    child->handle = (intptr_t)process.hProcess; child->id = process.dwProcessId;
    return 1;
}
int platform_child_running(Child *child)
{
    return child->handle && WaitForSingleObject((HANDLE)child->handle, 0) == WAIT_TIMEOUT;
}
void platform_child_stop(Child *child)
{
    if (!child->handle) return;
    if (WaitForSingleObject((HANDLE)child->handle, 1000) == WAIT_TIMEOUT) {
        TerminateProcess((HANDLE)child->handle, 0);
        WaitForSingleObject((HANDLE)child->handle, 1000);
    }
    CloseHandle((HANDLE)child->handle); child->handle = 0;
}
#else
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
static struct termios old_input;
static int saved;
static volatile sig_atomic_t stop_flag;
static void stop_handler(int event) { (void)event; stop_flag = 1; }
int platform_stopping(void) { return stop_flag != 0; }
uint64_t platform_millis(void)
{
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) return 0;
    return (uint64_t)time.tv_sec * 1000 + (uint64_t)time.tv_nsec / 1000000;
}
void platform_sleep(int milliseconds)
{
    struct timespec time = {milliseconds / 1000, (milliseconds % 1000) * 1000000L};
    while (nanosleep(&time, &time) < 0 && errno == EINTR) { }
}
int term_open(void)
{
    struct termios mode;
    struct sigaction action;
    if (!isatty(0) || !isatty(1) || tcgetattr(0, &old_input) != 0) return 0;
    mode = old_input;
    mode.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    mode.c_lflag |= ISIG;
    mode.c_iflag &= (tcflag_t)~(IXON | ICRNL);
    mode.c_cc[VMIN] = 0; mode.c_cc[VTIME] = 0;
    memset(&action, 0, sizeof action); action.sa_handler = stop_handler;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) || sigaction(SIGTERM, &action, NULL) ||
        sigaction(SIGQUIT, &action, NULL) || sigaction(SIGTSTP, &action, NULL) ||
        tcsetattr(0, TCSANOW, &mode)) return 0;
    saved = 1;
    return fputs("\033[?1049h\033[?25l", stdout) >= 0 && fflush(stdout) == 0;
}
void term_close(void)
{
    if (!saved) return;
    tcsetattr(0, TCSANOW, &old_input);
    fputs("\033[?25h\033[?1049l", stdout); fflush(stdout); saved = 0;
}
int term_size(int *width, int *height)
{
    struct winsize size;
    if (ioctl(1, TIOCGWINSZ, &size) || !size.ws_col || !size.ws_row) return 0;
    *width = size.ws_col; *height = size.ws_row; return 1;
}
static int read_byte(int timeout_ms)
{
    fd_set set;
    struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    unsigned char byte;
    int result;
    FD_ZERO(&set); FD_SET(0, &set);
    result = select(1, &set, NULL, NULL, &timeout);
    if (result < 0) return errno == EINTR ? 0 : -1;
    if (!result) return 0;
    result = (int)read(0, &byte, 1);
    if (result < 0 && errno == EINTR) return 0;
    return result == 1 ? byte : -1;
}
int term_key(int timeout_ms)
{
    int key = read_byte(timeout_ms);
    if (key == 13 || key == 10) return KEY_ENTER;
    if (key == 127 || key == 8) return KEY_BACKSPACE;
    if (key != 27) return key;
    key = read_byte(20);
    if (key == '[' || key == 'O') {
        int final = 0, i;
        for (i = 0; i < 16; ++i) {
            key = read_byte(20);
            if (key <= 0) break;
            if (key >= 0x40 && key <= 0x7e) { final = key; break; }
        }
        switch (final) {
            case 'A': return KEY_UP; case 'B': return KEY_DOWN;
            case 'C': return KEY_RIGHT; case 'D': return KEY_LEFT;
            default: return 0;
        }
    }
    return KEY_ESCAPE;
}
int term_draw(const char *text)
{
    return fputs("\033[H\033[2J", stdout) >= 0 && fputs(text, stdout) >= 0 && fflush(stdout) == 0;
}
int platform_executable(char *path, int capacity)
{
    ssize_t length = readlink("/proc/self/exe", path, (size_t)capacity - 1);
    if (length < 1 || length >= capacity - 1) return 0;
    path[length] = '\0'; return 1;
}
int platform_spawn(Child *child, const char *executable, const char *address, int port, const char *session)
{
    char number[8];
    pid_t pid;
    snprintf(number, sizeof number, "%d", port);
    pid = fork();
    if (pid < 0) return 0;
    if (pid == 0) {
        int null_file = open("/dev/null", O_RDWR);
        if (null_file >= 0) { dup2(null_file, 0); dup2(null_file, 1); dup2(null_file, 2); if (null_file > 2) close(null_file); }
        execl(executable, executable, "--server", address, number, session, (char *)NULL);
        _exit(127);
    }
    child->handle = (intptr_t)pid; child->id = (unsigned long)pid; return 1;
}
int platform_child_running(Child *child)
{
    int status;
    pid_t result;
    if (!child->handle) return 0;
    result = waitpid((pid_t)child->handle, &status, WNOHANG);
    if (result == 0) return 1;
    if (result < 0 && errno == EINTR) return 1;
    child->handle = 0; return 0;
}
void platform_child_stop(Child *child)
{
    int i, status;
    if (!child->handle) return;
    for (i = 0; i < 20 && platform_child_running(child); ++i) platform_sleep(50);
    if (!child->handle) return;
    kill((pid_t)child->handle, SIGTERM);
    for (i = 0; i < 20 && platform_child_running(child); ++i) platform_sleep(50);
    if (!child->handle) return;
    kill((pid_t)child->handle, SIGKILL);
    while (waitpid((pid_t)child->handle, &status, 0) < 0 && errno == EINTR) { }
    child->handle = 0;
}
#endif
