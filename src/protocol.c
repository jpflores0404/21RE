#include "protocol.h"
#include <string.h>

static int printable(const cJSON *item, size_t limit)
{
    const unsigned char *text;
    if (!cJSON_IsString(item) || item->valuestring == NULL || strlen(item->valuestring) > limit) return 0;
    text = (const unsigned char *)item->valuestring;
    while (*text) { if (*text < 32 || *text > 126) return 0; ++text; }
    return 1;
}

static int integer(const cJSON *item, int minimum, int maximum)
{
    return cJSON_IsNumber(item) && item->valuedouble >= minimum && item->valuedouble <= maximum &&
        item->valuedouble == (double)item->valueint;
}

static int fields(const cJSON *object, const char *const *names, int count)
{
    const cJSON *item;
    int seen = 0, i;
    cJSON_ArrayForEach(item, object) {
        int matched = 0;
        if (item->string == NULL) return 0;
        for (i = 0; i < count; ++i) if (strcmp(item->string, names[i]) == 0) {
            if (seen & (1 << i)) return 0;
            seen |= 1 << i; matched = 1; break;
        }
        if (!matched) return 0;
    }
    return seen == (1 << count) - 1;
}

cJSON *protocol_action(const char *line, size_t length)
{
    char copy[ACTION_LIMIT + 1];
    const char *end;
    cJSON *object, *type;
    size_t i;
    int depth = 0, quoted = 0, escaped = 0, valid = 0;
    const char *const basic[] = { "type" };
    const char *const mapped[] = { "type", "map" };
    const char *const moved[] = { "type", "map", "direction" };
    const char *const submitted[] = { "type", "map", "object", "answer" };
    const char *const dialed[] = { "type", "map", "dial", "value" };
    const char *const chatted[] = { "type", "text" };
    if (line == NULL || length == 0 || length > ACTION_LIMIT) return NULL;
    for (i = 0; i < length; ++i) {
        unsigned char ch = (unsigned char)line[i];
        if (ch < 32 || ch > 126) return NULL;
        if (escaped) { if (ch == 'u') return NULL; escaped = 0; continue; }
        if (quoted && ch == '\\') { escaped = 1; continue; }
        if (ch == '"') quoted = !quoted;
        if (!quoted) {
            if (ch == '{' || ch == '[') { if (++depth > 8) return NULL; }
            if (ch == '}' || ch == ']') { if (--depth < 0) return NULL; }
        }
    }
    memcpy(copy, line, length); copy[length] = '\0';
    object = cJSON_ParseWithLengthOpts(copy, length + 1, &end, 1);
    if (!cJSON_IsObject(object)) { cJSON_Delete(object); return NULL; }
    type = cJSON_GetObjectItemCaseSensitive(object, "type");
    if (!printable(type, 16)) goto rejected;
    if (strcmp(type->valuestring, "ready") == 0 || strcmp(type->valuestring, "quit") == 0)
        valid = fields(object, basic, 1);
    else if (strcmp(type->valuestring, "chat") == 0)
        valid = fields(object, chatted, 2) && printable(cJSON_GetObjectItemCaseSensitive(object, "text"), 120);
    else {
        if (!integer(cJSON_GetObjectItemCaseSensitive(object, "map"), 1, 3)) goto rejected;
        if (strcmp(type->valuestring, "interact") == 0 || strcmp(type->valuestring, "test") == 0)
            valid = fields(object, mapped, 2);
        else if (strcmp(type->valuestring, "move") == 0) {
            cJSON *direction = cJSON_GetObjectItemCaseSensitive(object, "direction");
            valid = fields(object, moved, 3) && printable(direction, 5) &&
                (strcmp(direction->valuestring, "up") == 0 || strcmp(direction->valuestring, "right") == 0 ||
                 strcmp(direction->valuestring, "down") == 0 || strcmp(direction->valuestring, "left") == 0);
        } else if (strcmp(type->valuestring, "submit") == 0)
            valid = fields(object, submitted, 4) && printable(cJSON_GetObjectItemCaseSensitive(object, "object"), 32) &&
                printable(cJSON_GetObjectItemCaseSensitive(object, "answer"), 64);
        else if (strcmp(type->valuestring, "dial") == 0)
            valid = fields(object, dialed, 4) && integer(cJSON_GetObjectItemCaseSensitive(object, "dial"), 0, 2) &&
                integer(cJSON_GetObjectItemCaseSensitive(object, "value"), 0, 4);
    }
    if (valid) return object;
rejected:
    cJSON_Delete(object);
    return NULL;
}

int protocol_print(const cJSON *message, char *buffer, size_t capacity)
{
    char *text = cJSON_PrintUnformatted(message);
    size_t length;
    if (text == NULL || buffer == NULL || capacity == 0) { cJSON_free(text); return 0; }
    length = strlen(text);
    if (length + 2 > capacity || length > VIEW_LIMIT) { cJSON_free(text); return 0; }
    memcpy(buffer, text, length); buffer[length] = '\n'; buffer[length + 1] = '\0';
    cJSON_free(text);
    return (int)(length + 1);
}
