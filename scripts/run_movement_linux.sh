#!/bin/sh
set -eu

if [ "$(uname -s)" != Linux ]; then
    printf '%s\n' 'Run this script inside Linux. Windows uses the MSVC command in docs/MOVEMENT_DEMO.md.' >&2
    exit 1
fi

if ! command -v cc >/dev/null 2>&1; then
    printf '%s\n' 'No C compiler named cc was found. Use a Linux environment with a C compiler installed.' >&2
    exit 1
fi

project_root=$(CDPATH= cd -P "$(dirname "$0")/.." && pwd)
output_dir="$project_root/submissions/movement-demo"
mkdir -p "$output_dir"
cc -std=c17 -Wall -Wextra -Wpedantic -Werror \
    "$project_root/src/movement_demo.c" -o "$output_dir/movement_demo"
exec "$output_dir/movement_demo" "$@"
