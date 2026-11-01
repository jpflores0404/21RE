# cJSON source dependency

Pinned release: cJSON v1.7.19 from https://github.com/DaveGamble/cJSON/tree/v1.7.19.

Files `cJSON.c`, `cJSON.h` and `cJSON.LICENSE` were fetched from that exact tag. The approved October 31 milestone vendors these sources and statically compiles them into the application. No package installation or external runtime library is required.

Build with `CJSON_NESTING_LIMIT=8`. Application framing, message lengths, duplicate/unknown fields, schema and game-action authorization remain the application's responsibility. cJSON parsing alone is not gameplay validation. Preserve the upstream MIT license when redistributing.
