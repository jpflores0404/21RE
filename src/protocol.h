#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stddef.h>
#include "../third_party/cJSON.h"
#define ACTION_LIMIT 1024
#define VIEW_LIMIT 16384
#define OUTPUT_LIMIT 65536
/* Complete line without newline; rejects duplicate/unknown keys, wrong types, trailing bytes and controls. */
cJSON *protocol_action(const char *line, size_t length);
int protocol_print(const cJSON *message, char *buffer, size_t capacity);
int protocol_tests(void);
#endif
