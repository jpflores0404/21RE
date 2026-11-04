#include "demo.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif

int add_character(Line *line, int key)
{
    if (key == '\n' || key == '\r') {
        if (line->too_long) line->used = 0;
        line->text[line->used] = '\0';
        line->used = 0;
        line->too_long = 0;
        return 1;
    }
    if (key < 32 || key > 126 || line->used == sizeof line->text - 1) {
        line->too_long = 1;
    } else if (!line->too_long) {
        line->text[line->used++] = (char)key;
    }
    return 0;
}

/* Linux keeps normal terminal line editing. Windows echoes console keys here. */
int input_line(Line *line)
{
    for (int i = 0; i < LINE_SIZE; ++i) {
#ifdef _WIN32
        if (!_kbhit()) return 0;
        int key = _getch();
        if (key == 0 || key == 224) {
            _getch();
            continue;
        }
        if (key == '\b') {
            if (line->used > 0) {
                --line->used;
                fputs("\b \b", stdout);
                fflush(stdout);
            }
            continue;
        }
        if (key == '\r') putchar('\n');
        else if (key >= 32 && key <= 126 && !line->too_long &&
                 line->used < sizeof line->text - 1) putchar(key);
        fflush(stdout);
#else
        fd_set readers;
        struct timeval wait = {0, 0};
        unsigned char key;
        FD_ZERO(&readers);
        FD_SET(STDIN_FILENO, &readers);
        int ready = select(STDIN_FILENO + 1, &readers, NULL, NULL, &wait);
        if (ready <= 0) return ready;
        if (read(STDIN_FILENO, &key, 1) != 1) return -1;
#endif
        if (add_character(line, key)) return 1;
    }
    return 0;
}

int read_prompt(char *text, size_t capacity)
{
    if (!fgets(text, (int)capacity, stdin)) return 0;
    char *newline = strchr(text, '\n');
    if (!newline) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { }
        text[0] = '\0';
    } else {
        *newline = '\0';
    }
    return 1;
}

void show_screen(const char *screen)
{
    printf("\n%s\n", screen);
    fflush(stdout);
}
