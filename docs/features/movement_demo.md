# Feature explanation for the C movement demo

Status: implemented and Windows compiled; pure movement checks and a controlled native Windows console probe passed. Human terminal acceptance, Linux acceptance and both developers' teach-back are pending unless separately recorded after this note.

Approval: John Francis Espiritu approved the assistant writing the demo and notes on October 27, 2026, Asia/Manila. Approved scope: WASD marker movement across the current terminal, bounds, resize, Q exit and terminal restoration. Implementation assistance: Codex. Developer review: pending. This is an approved early demo alongside Week 1 design work, not approval for the campaign port or networking.

## Purpose and learning example

The screen contains a player marker and an otherwise open bordered play area. The player can move immediately without pressing Enter. It demonstrates how a C process receives terminal input, changes state and writes a new view. It does not demonstrate sockets, threads, synchronization between computers or an authoritative multiplayer server.

State means values remembered while a program runs. A struct groups related values. `Player` contains integers `x` and `y`. Coordinates start at the upper left; increasing x moves right and increasing y moves down. A pointer such as `Player *player` lets a function modify the caller's struct. A return value reports whether an operation worked or changed something.

Simple example: a counter of 5 becomes 6 when incremented. Project example: player `(5,5)` receives D, so the candidate position becomes `(6,5)`. Bounds are applied before the candidate replaces the player. At x=78 in an 80-column terminal, D would reach the right border, so the position remains x=78 and the movement function returns 0.

The inherited Python `try_move` was read before this work. Both implementations compute a candidate position and reject blocked movement. This C demo has terminal boundaries only; it does not load the Python maps or preserve the prototype's facing/object behavior. It is a separate local exercise, not a replacement for existing callers.

## Files and callers

| File | Purpose and changes | Caller or reader |
|---|---|---|
| `src/movement_demo.c` | New standalone C entry point, movement rules, Windows/POSIX terminal branches, rendering and 14 self-checks | C compiler; user launches resulting executable |
| `docs/MOVEMENT_DEMO.md` | New actual Windows commands, proposed Linux commands, controls and demonstration checklist | Both developers and presenter |
| `docs/features/movement_demo.md` | This complete feature explanation and evidence record | Both developers and reviewer |
| `submissions/movement-demo/movement_demo.exe` | Generated local native Windows executable; directory was already ignored | User or test command; rebuild when source changes |
| `submissions/movement-demo/movement_demo.obj` | Generated intermediate object file in the same ignored directory | MSVC linker; not a source deliverable |

No existing Python functions, fixtures, imports, configuration or public interfaces were changed. There are no new dependencies or build-system files. The single source file keeps this first exercise easy to build; separate modules should be introduced only when the approved game actually needs them.

## Common functions explained

### playable

Parameters: integer width and height, for example 80 and 24. Returns 1 if width is at least 20 and height at least 6, otherwise 0. It changes no state. The minimum prevents the header and play area from collapsing. `keep_inside`, `move_player`, `render` and self-checks call it. A small terminal is an expected condition, not a crash.

### keep_inside

Parameters: pointer to a valid Player plus width and height. Returns nothing. For a usable terminal it clamps x to 1 through width-2 and y to 2 through height-2. For a too-small terminal it leaves coordinates unchanged until usable dimensions return. The first row is the header, row 1 is the top boundary and the last row is the bottom boundary. Called by movement, resizing and self-checks. Example: `(78,22)` becomes `(18,4)` after shrinking to 20 by 6. There is no allocation or external input parsing here.

### move_player

Parameters: player pointer, integer key and terminal dimensions. Returns 1 only when position changes, otherwise 0. It copies the original player into `next`, rejects too-small dimensions, maps WASD or uppercase equivalents to one coordinate change, rejects other keys, clamps the candidate and compares it with the original. Only a changed valid candidate is copied back. Called by the main loop and self-checks. Q is handled by main, not interpreted as movement. `keep_inside` supplies its only bounds logic.

### render

Parameters: player value and current positive terminal dimensions. Returns success or failure. Allocates one row of width bytes, returning failure if allocation fails. The loop writes each row as spaces, adds the header/borders and player when the area is playable, or shows an enlargement prompt when small. Each write uses the platform `write_at`. It deliberately skips the bottom-right cell to avoid terminal scrolling, frees the row and flushes output. It does not change player state. Called by main only after initial sizing, resizing or movement, rather than repainting on every input timeout. On a write failure main closes the terminal and reports failure. Dynamic row allocation is bounded by the terminal's reported width; no gameplay-sized unbounded buffers or format strings are accepted.

### self_test

Takes no parameters and returns `EXIT_SUCCESS` only if all 14 checks pass. It builds example Player structs and calls real common movement functions. The local CHECK macro prints the failing line and condition, then increments a failure count; it remains active in release builds. It tests each direction, uppercase keys, invalid input, all four edges, shrinking, pausing for small dimensions, enlargement and minimum dimensions. It changes only local test state and never opens a terminal. Called by main for the exact `--self-test` argument. It does not test native input, screen drawing, signal handlers, Linux compilation or LAN behavior.

### main

Parameters: argument count and argument strings from the OS. Returns a process exit status. The exact `--self-test` argument runs pure checks; extra/unknown arguments print usage and fail. Interactive startup opens the platform terminal or prints a readable error after cleanup. Its loop checks stop requests, reads current dimensions, clamps on resize, renders only when dirty, waits briefly for input, handles Q and delegates WASD to `move_player`. First usable screen position is approximately its center. Key input or terminal I/O failure breaks with a failing exit status. All normal loop exits call `terminal_close`. There is no busy loop: platform input waits for up to 50 milliseconds, allowing resize and stop requests to be noticed even without keys.

## Windows branch functions explained

The Windows branch is selected by the compiler's `_WIN32` macro. It uses Windows headers only inside that branch. Saved handles identify the original screen, temporary screen and console input. Input mode is saved before modification. Handles are OS-managed references, not C memory pointers to free.

### request_stop

Windows console callback taking an event number, returning TRUE for Ctrl+C or Ctrl+Break and FALSE for unrelated events. For handled events it atomically sets the stop flag using `InterlockedExchange`. The OS calls it, potentially outside the main execution path. It does not render or clean up in the callback; main notices the request and closes normally. Forced process termination or closing the host window is not a guaranteed cleanup path.

### terminal_open

Takes no parameters; returns success or failure. Gets standard input/output handles, verifies interactive console input and screen output, saves input mode and creates a separate console screen buffer. It disables line/echo/Quick Edit modes, enables resize events, hides the demo cursor, registers the stop callback and activates the temporary screen. Each OS operation is checked. A failure returns immediately; main calls cleanup using the saved flags/handles. The original screen is preserved instead of being cleared. Called only by main; this is the platform setup step, not an application framework.

### terminal_close

Takes no parameters and returns nothing. Restores saved input mode, reactivates the original output buffer, closes the temporary handle once and unregisters the callback. Called by main after normal exit or startup failure. Checks on saved handles/flags make partial setup safe. Cleanup calls are best effort if the terminal itself has been destroyed. It does not close standard handles owned by the host.

### terminal_size

Takes pointers to width and height, returns success or failure. Queries the active buffer information and calculates visible width/height from its window rectangle. Also stores the rectangle so drawing includes any viewport offset. It changes only those outputs and cached screen metadata. Called by main each loop; a failed query ends the demo cleanly.

### write_at

Takes x/y viewport coordinates, text bytes and explicit length. Returns success only if the exact number of bytes is written. Converts viewport coordinates to buffer coordinates and calls `WriteConsoleOutputCharacterA`, which writes directly without advancing a text cursor. The caller supplies a row bounded by current width. Called by `render`; a resize race or output failure propagates to main rather than being ignored.

### read_key

Takes no parameters. Returns an unsigned ASCII key, 0 for timeout/non-key events, or -1 for an input error. Waits up to 50 milliseconds, reads one console input record and returns key-down characters only. Resize and other records are consumed without moving the player. Native repeat behavior generates movement events; the demo does not implement a game-speed simulation. Called by main after rendering. Normal non-key events do not fail the program.

### stopping

Takes no parameters and returns whether a stop request exists. `InterlockedCompareExchange` reads the shared stop flag without changing it. Main calls it at the loop condition. This only coordinates console cancellation; it is not a multiplayer synchronization feature.

## Linux branch functions explained

The alternative branch uses POSIX terminal and descriptor APIs. It is present for portability but has not been compiled or manually accepted in this Windows session. The build defines the POSIX feature level before including system headers.

### request_stop

Signal callback taking an integer signal number and returning nothing. It only sets a `volatile sig_atomic_t` flag. The OS calls it for SIGINT or SIGTERM; main later handles shutdown. Cleanup is outside the handler because terminal I/O and most library work are unsafe inside a signal handler.

### terminal_open

Takes no parameters and returns success or failure. Requires input and output to be terminals, saves `termios` settings, disables canonical line input/echo and software flow control, and sets immediate read controls. It registers SIGINT/SIGTERM handlers, applies the new settings, enters the alternate screen and hides the cursor. On failure it returns 0; main calls cleanup. It retains signal processing so Ctrl+C reaches the handler. Only a successful settings change marks the input mode as saved.

### terminal_close

Takes no parameters and returns nothing. If input mode was changed, restores original settings, shows the cursor, leaves the alternate screen, flushes output and clears its saved-state flag. Called by main for startup failure and ordinary exit. SIGKILL or terminal destruction cannot guarantee cleanup. It does not allocate/free memory or persist game state.

### terminal_size

Takes width/height pointers and returns success or failure. Uses `ioctl` with `TIOCGWINSZ` to read columns and rows. Rejects failed queries and zero dimensions before filling outputs. Called each main-loop iteration, so no special resize handler mutates gameplay state.

### write_at

Takes x/y, text and bounded length; returns success or failure. Converts zero-based coordinates into a terminal cursor escape sequence, then writes the exact row bytes. It reports formatting or output errors. Called only by `render`. The demo assumes a terminal supporting the ordinary cursor/alternate-screen escape sequences; unsupported terminal types need actual acceptance testing.

### read_key

Takes no parameters and returns a byte, 0 for timeout/interrupted wait or -1 for an error/EOF. Builds a fresh read descriptor set and 50-millisecond timeout, waits with `select`, then reads one byte when available. A signal interruption returns control to main so its stop flag is checked. Unexpected EOF ends the demo instead of looping forever. Arrow-key escape sequences are not decoded; WASD is the approved control scheme.

### stopping

Takes no parameters; returns whether the signal-safe stop flag is set. Called by main's loop. No threads, queues or locks are created.

## Trace and OS connection

Input D -> platform `read_key` returns `'d'` -> `move_player` builds `(x+1,y)` -> `keep_inside` clamps within the current terminal -> changed player marks the display dirty -> `render` writes the new marker. In this local exercise there is no network message, server validation or partner update. Input Z produces no state change and no redraw.

The OS provides a process, terminal input/output handles or descriptors, and event waiting. The 50-millisecond wait prevents constant CPU polling while preserving responsiveness. The player struct is ordinary memory owned by this process. Proposed professor explanation: "Our first C prototype reads keyboard events without waiting for Enter and updates the player's coordinates within terminal bounds. Platform-specific terminal code handles Windows and Linux differences, while the movement rule is tested without opening a terminal."

## Actual checks and remaining acceptance

October 27, 2026, Windows: the existing MSVC 14.50.35717 compiler built C17 source with `/W4 /WX /TC`, with no warnings. Running `submissions/movement-demo/movement_demo.exe --self-test` printed `Movement self-test passed (14 checks).` Exact command appears in the run guide. Later source edits must be rebuilt before this evidence is reused.

The final source was rebuilt and all 14 checks passed again. Unknown argument `--invalid` returned exit code 1 with usage; launch with redirected handles returned exit code 1 with the interactive-terminal requirement. This verifies those failure paths without opening a player-facing window.

A separate temporary Python ctypes probe created a hidden native Windows console, launched the real executable attached to it, injected actual input records and read the active screen buffer. It checked WASD positions, each boundary, shrink to 40x12 and clamp to `(38,10)`, a 10x4 viewport enlargement prompt with movement paused, enlargement with position retained, Q exit code 0, saved input mode restoration and original screen restoration. A second launch received an actual console Ctrl+C event and exited successfully with modes/screen restored. The probe first tried a 10-column screen-buffer resize, which Windows rejected with error 87 because of native console size constraints; the harness was corrected to resize the viewport instead. No game source change was needed for that harness error.

Probe helper and raw result are at `%TEMP%/21re-movement-probe-20261027/probe.py` and `result.json`; a copy of the successful result is in ignored `submissions/movement-demo/windows-console-probe.json`. This temporary verification helper is not part of the shipped game. It contains typed Windows structure declarations, handle/view/key helpers and a bounded wait for the expected marker, then drives those checks. It is controlled console evidence rather than human visual acceptance, performance measurement or a Windows Terminal playtest.

Pure checks prove coordinate and boundary rules only; the native probe adds OS input/output evidence. Human terminal feel and holding keys still need interactive acceptance. Linux build/run is pending; no two-computer or LAN claim applies. Common startup failure is running inside a redirected editor output panel rather than a real console. A missing executable means rebuild using an installed C compiler, not run the inherited Python command.

## Developer attempt and teach back

Predict the result of D at `(78,22)` in an 80x24 terminal, then trace the actual function. Change the marker in `render` after reviewing the source and rebuild. Explain why coordinates are clamped after a resize and why cleanup occurs outside signal/console handlers. The partner should review and explain those steps without reading this answer aloud.

| Developer | Execution explanation | Valid and blocked input trace | Small source change | Evidence |
|---|---|---|---|---|
| John Francis Espiritu | Pending | Pending | Pending | Not yet recorded |
| Partner name to confirm | Pending | Pending | Pending | Not yet recorded |

No developer comprehension, human review or oral demonstration is claimed. Next dependent step requires its own approval and acceptance; this demo does not approve a server or campaign implementation.

## October 28 Linux comparison and launcher addition

John requested Linux C support and a detailed explanation against Windows. The existing shared C file already contains the POSIX implementation; no C runtime function was changed or duplicated. The added file `scripts/run_movement_linux.sh` checks Linux and `cc`, finds the repository relative to itself, creates the existing ignored output location, compiles with C17 warnings treated as errors, and execs the resulting program with the supplied arguments. It defines no shell functions. A non-Linux environment or missing compiler prints a message and returns one; compilation failure stops the script through `set -e`. `set -u` rejects unset shell variables. Inputs are positional executable arguments; quoted paths preserve spaces; it makes no game-state changes itself.

The new `docs/LINUX_WINDOWS_MOVEMENT_GUIDE.md` explains each common and platform function, headers versus libraries, keyboard flow, control flags, drawing, cleanup, tests, known differences, and practice questions. Its Word export is a study artifact. `docs/MOVEMENT_DEMO.md` links the guide and launcher. Existing source callers and Windows build commands remain unchanged.

The added root `.gitattributes` specifies LF line endings only for `scripts/run_movement_linux.sh`. This prevents a Windows Git checkout from converting that Linux shell script into CRLF text. It defines no functions and changes no C compilation or game behavior. `git check-attr` verifies the effective text/eol settings; it does not test runtime input.

On October 28, Linux `sh -n` passed for the launcher. Running it under the available docker-desktop WSL distribution produced the expected missing-compiler diagnostic and exit code one. No `cc`, `gcc`, or `clang` was present there. No compiler was installed and the Docker Linux service was unavailable. The Windows executable's 14 movement checks passed again. Linux C compilation, Linux rendering and human Linux acceptance remain unverified. An independent source review found no obvious ordinary WASD/Q compile or runtime blocker; that review is not an actual Linux build.

The review also identified actual input limitations: Linux arrow sequences can end in uppercase A or D and be interpreted as movement, and Ctrl+Z/SIGQUIT do not use the cleanup path. The study guide explains these accurately. WASD/Q and the handled SIGINT/SIGTERM paths remain the intended Linux prototype contract. No student understanding is inferred from producing these explanations.
