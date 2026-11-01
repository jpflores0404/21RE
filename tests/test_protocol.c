#include "../src/protocol.h"
#include <stdio.h>
#include <string.h>

int protocol_tests(void)
{
    static const char *valid[] = {
        "{\"type\":\"ready\"}", "{\"type\":\"quit\"}",
        "{\"type\":\"move\",\"map\":1,\"direction\":\"up\"}",
        "{\"type\":\"interact\",\"map\":3}",
        "{\"type\":\"submit\",\"map\":1,\"object\":\"cabinet\",\"answer\":\"246\"}",
        "{\"type\":\"dial\",\"map\":2,\"dial\":0,\"value\":4}",
        "{\"type\":\"test\",\"map\":2}", "{\"type\":\"chat\",\"text\":\"Lantern is 2\"}"
    };
    static const char *invalid[] = {
        "[]", "{}", "null", "true", "{", "{\"type\":\"ready\"}x",
        "{\"type\":\"ready\",\"type\":\"quit\"}", "{\"Type\":\"ready\"}",
        "{\"type\":\"ready\",\"player\":2}", "{\"type\":\"victory\"}",
        "{\"type\":\"move\",\"map\":1.5,\"direction\":\"up\"}",
        "{\"type\":\"move\",\"map\":0,\"direction\":\"up\"}",
        "{\"type\":\"move\",\"map\":1,\"direction\":\"diagonal\"}",
        "{\"type\":\"dial\",\"map\":2,\"dial\":3,\"value\":2}",
        "{\"type\":\"chat\",\"text\":\"hello\\nworld\"}",
        "{\"type\":\"chat\",\"text\":\"\\u0000escape\"}",
        "{\"type\":\"chat\",\"text\":\"\\u001b[2J\"}",
        "{\"type\":\"chat\",\"text\":{\"x\":{\"x\":{\"x\":{\"x\":{\"x\":{\"x\":{\"x\":{\"x\":{}}}}}}}}}}"
    };
    size_t i;
    int failures = 0, checks = 0;
    char oversized[ACTION_LIMIT + 2], printed[128], answer[66], bounded[256];
    cJSON *json;
    for (i = 0; i < sizeof valid / sizeof valid[0]; ++i) {
        json = protocol_action(valid[i], strlen(valid[i])); ++checks;
        if (!json) { ++failures; fprintf(stderr, "Protocol rejected valid sample %zu\n", i); }
        cJSON_Delete(json);
    }
    for (i = 0; i < sizeof invalid / sizeof invalid[0]; ++i) {
        json = protocol_action(invalid[i], strlen(invalid[i])); ++checks;
        if (json) { ++failures; fprintf(stderr, "Protocol accepted invalid sample %zu\n", i); }
        cJSON_Delete(json);
    }
    memset(oversized, ' ', sizeof oversized);
    json = protocol_action(oversized, sizeof oversized); ++checks;
    if (json) ++failures;
    cJSON_Delete(json);
    memset(answer, 'a', 64); answer[64] = '\0';
    snprintf(bounded, sizeof bounded, "{\"type\":\"submit\",\"map\":1,\"object\":\"cabinet\",\"answer\":\"%s\"}", answer);
    json = protocol_action(bounded, strlen(bounded)); ++checks;
    if (!json) ++failures;
    cJSON_Delete(json);
    answer[64] = 'a'; answer[65] = '\0';
    snprintf(bounded, sizeof bounded, "{\"type\":\"submit\",\"map\":1,\"object\":\"cabinet\",\"answer\":\"%s\"}", answer);
    json = protocol_action(bounded, strlen(bounded)); ++checks;
    if (json) ++failures;
    cJSON_Delete(json);
    json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "ready"); ++checks;
    if (protocol_print(json, printed, sizeof printed) != 17 || strcmp(printed, "{\"type\":\"ready\"}\n") != 0) ++failures;
    ++checks;
    if (protocol_print(json, printed, 2) != 0) ++failures;
    cJSON_Delete(json);
    printf("Protocol checks: %d passed, %d failed.\n", checks - failures, failures);
    return failures;
}
