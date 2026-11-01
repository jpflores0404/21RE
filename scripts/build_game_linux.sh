#!/bin/sh
set -eu
root=$(CDPATH= cd -P "$(dirname "$0")/.." && pwd)
cd "$root"
if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != x86_64 ]; then
    printf '%s\n' 'This release build script targets Linux x86_64.' >&2
    exit 1
fi
if ! command -v cc >/dev/null 2>&1; then
    printf '%s\n' 'A Linux C compiler named cc is required.' >&2
    exit 1
fi
mkdir -p releases/linux-x86_64
cc -std=c17 -Wall -Wextra -Wpedantic -Werror -O2 -DCJSON_NESTING_LIMIT=8 \
    src/main.c src/platform.c src/net.c src/protocol.c src/content.c src/game.c \
    src/server.c src/client.c tests/test_game.c tests/test_protocol.c third_party/cJSON.c \
    -lm -o releases/linux-x86_64/2r1e
./releases/linux-x86_64/2r1e --self-test
printf '%s\n' 'Linux executable: releases/linux-x86_64/2r1e'
