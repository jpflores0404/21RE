# Shared C module contracts

John approved the October 31 Maps 1 and 2 plan and AI-assisted implementation. This note accounts for shared declarations and build/distribution configuration owned by the integrating agent. Actual test evidence belongs in the milestone report; student teach-back remains pending.

## Header files

`src/game.h` defines the dimensions, `GamePlayer` and `Game` state structs, and declarations for pure rules/content. `GamePlayer` stores coordinates, facing, readiness, discovered notes, private feedback and the active object selected by interaction. `Game` stores the current map and shared puzzle flags/dials. Internally players use indices zero/one; displayed IDs use one/two. `game_init` initializes state; `game_action` applies a socket-associated player's requested action and returns acceptance; `game_view` allocates only that player's permitted JSON view and its caller must free it. `content_room` fills a bounded map grid; `content_room_name` supplies a static room label. `game_tests` runs focused rule checks.

`src/protocol.h` sets action/view/output bounds and declares `protocol_action`, `protocol_print` and `protocol_tests`. A line is validated before any game-state mutation. Parsed cJSON trees are owned by the caller. Printing must respect capacity and append the newline framing delimiter. These declarations are shared by client, server and tests, so changes must remain compatible across all callers.

`src/net.h` defines a portable `Socket` representation and invalid sentinel. `net_init`/`net_cleanup` own platform network startup, `net_listen` binds a server, `net_connect` bounds connection waiting, and `net_accept` returns a nonblocking peer. `net_read`/`net_write` normalize byte counts, would-block and fatal errors. `net_wait` reports readiness; `net_nonblocking` sets mode; `net_close` releases an owned socket. Network buffers remain in client/server modules, not inside hidden global shared gameplay state.

`src/platform.h` defines normalized special keys and `Child` ownership. Terminal functions open/close, size, read and draw. Clock/sleep helpers support bounded waiting. Executable lookup and spawn functions start the owned referee copy; child status/stop functions operate only on that owned process. `platform_stopping` exposes a shutdown request. These signatures let main/client code remain independent of Windows handles or POSIX descriptors.

`src/app.h` declares the two application modes: `server_run` owns an authoritative session, and `client_run` owns the menu/player UI. Main dispatches these functions. These are declarations, not another copy of the implementation.

## Configuration and reference files

`third_party/cJSON.c`, `third_party/cJSON.h` and `third_party/cJSON.LICENSE` are unmodified vendored cJSON v1.7.19 sources/license. `third_party/README.md` records the pin and nesting-limit build requirement. The parser is a source dependency, not a runtime package installation; the application still validates schema and actions.

`.gitignore` excludes Windows executables, objects and local build intermediates. `.gitattributes` preserves LF endings for shell scripts. Neither changes runtime game state. Linux release files are explicitly reviewed and recorded with executable mode; no Windows build output belongs in GitHub.

`AGENTS.md` and the approved plan record the C implementation authority and the precise Map 3 stop. The original master prompt remains historical. The run guide explains commands and actual gameplay; the check-in report distinguishes inherited code, new C work, controlled tests and unverified student acceptance.

## Worked trace and review exercise

A client sends a move action with the current map and a direction. Protocol validation produces a bounded action; the connection supplies its player index. The game applies only that player's legal movement, and the server serializes separately filtered views. A client-supplied player ID or outcome is not an authority. Predict a move into a wall, find the return value and facing change in the actual implementation, then explain which fields appear in each client's view.

No student understanding, signatures or physical two-machine acceptance is inferred from these contracts. Review the implementation and its tests beside this note.
