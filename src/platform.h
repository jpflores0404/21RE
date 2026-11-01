#ifndef PLATFORM_H
#define PLATFORM_H
#include <stdint.h>
typedef struct { intptr_t handle; unsigned long id; } Child;
enum { KEY_UP=256, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_ESCAPE, KEY_ENTER, KEY_BACKSPACE };
int term_open(void);
void term_close(void);
int term_size(int *width, int *height);
int term_key(int timeout_ms);
int term_draw(const char *text);
int platform_stopping(void);
uint64_t platform_millis(void);
void platform_sleep(int milliseconds);
int platform_executable(char *path, int capacity);
int platform_spawn(Child *child, const char *executable, const char *address, int port, const char *session);
int platform_child_running(Child *child);
void platform_child_stop(Child *child);
#endif
