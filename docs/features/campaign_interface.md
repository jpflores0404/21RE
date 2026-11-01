# Campaign interface and platform feature note

Approval: John approved `docs/plans/2026-10-31_MAPS_1_2_MILESTONE.md` on October 31, 2026. Implementation assistance: Codex subagent, with root-agent integration review. Student authorship, partner review and teach-back are pending until actually performed. This note accounts for interface-owned files; game rules and networking have separate notes.

## Player experience and boundaries

The new application has Host, Join, Tutorial and Quit entries. Host launches a background referee process and connects a local client. Join opens another client against the chosen numeric IPv4 address and port. Two terminals therefore correspond to two players while the server is a separate process. Same-computer defaults are 127.0.0.1 and port 5050. Hosting on 0.0.0.0 explicitly exposes the listener to available network interfaces; later physical-LAN testing is a separate acceptance step.

The interface draws only the permitted server view. It never loads puzzle answer metadata. WASD or arrows requests movement; E requests interaction; I displays discovered notes; T enters a bounded message; H opens tutorial help; Q leaves outside a text-entry dialog. Enter confirms and Escape closes a view. The word puzzle accepts three option numbers separated by spaces. The dial UI selects A/B/C using 1/2/3 and requests changes with plus/minus. Only the server decides whether a puzzle is solved.

The display uses the current terminal viewport with an 80x24 minimum for gameplay. A small window pauses player actions except quit while the network continues polling. Long clue text wraps. Answer and dial dialogs use a dedicated view instead of squeezing controls underneath the room. Map 3 is clearly an unfinished preview, not final victory.

## Concepts and one worked example

A process is an independently running program. A handle or descriptor identifies an OS resource belonging to a process. A socket is a communication endpoint. The UI sends an intention; receiving a view means the referee has chosen the state visible to that player.

Simple example: a function that receives a pointer can update the caller's value. In this client the server view is a JSON tree owned by `Client`. A new accepted tree replaces the old one and increments `view_generation`. The draw loop compares the generation counter instead of pointer addresses, because an allocator can reuse an address after an older tree is freed.

Project trace: at map 1, D creates `{type:move,map:1,direction:right}`. `queue` serializes it with a newline, `pump` sends all bytes including partial-send retries, and the server validates it. A subsequent view supplies the approved x/y/facing and room. `receive_message` accepts the view, increments its generation, and `draw_game` places `@` at the approved coordinate. A wall movement can change facing without position. The client does not decide success locally.

## Files

| File | Purpose and caller |
|---|---|
| `src/main.c` | New executable entry and strict command dispatch; called by the C runtime |
| `src/platform.c` | Terminal, keyboard, monotonic clock and owned-child process operations; called by the UI and server timing |
| `src/client.c` | Menu, configuration, session transport pump and views; called by `main` |
| `scripts/build_game_windows.ps1` | Native MSVC build and pure checks; run by the developer |
| `scripts/build_game_linux.sh` | Native Linux x86_64 build and pure checks; run on a suitable Linux machine |
| `docs/features/campaign_interface.md` | This explanation, review and evidence record |

The shared header declarations were established by the root agent. The original `src/movement_demo.c` and its existing executables remain separate learning checkpoints. New Windows output is in the ignored `submissions/game/` folder. Source-controlled Linux output is `releases/linux-x86_64/2r1e`.

## Entry function

`main(int argc, char **argv)` returns a process success/failure status. With exactly `--self-test`, it runs `game_tests` and `protocol_tests`, both using zero for success. With `--server IPv4 PORT SESSION`, it parses the port and requires 1024 through 65535, then runs the referee. Unknown arguments fail with usage. Normal startup resolves the current executable path and calls `client_run`. No shell command is built from a player's text.

## Platform functions

Each platform implementation supplies the same public operations. The compiler includes the Windows or POSIX branch, not both. Functions returning int use nonzero success unless a keyboard return convention is explicitly noted.

| Function | Parameters and result | State and execution |
|---|---|---|
| `stop_handler` | Windows event -> BOOL; Linux signal number -> void | For supported shutdown events only, sets the stop flag. Windows uses Interlocked operations; Linux uses `sig_atomic_t`. No rendering or cleanup occurs in the callback |
| `platform_stopping` | No parameters -> whether stop requested | Reads the appropriate stop flag; called by menu/session waits |
| `platform_millis` | No parameters -> unsigned elapsed milliseconds | Windows `GetTickCount64`, Linux `clock_gettime(CLOCK_MONOTONIC)`; no wall-clock dependency |
| `platform_sleep` | Milliseconds -> void | Windows Sleep or POSIX nanosleep; interrupted POSIX sleeps retry with the remaining time |
| `term_open` | No parameters -> success/failure | Validates interactive input, saves mode, opens a separate/alternate screen, hides cursor, disables line/echo input, registers shutdown handlers; caller cleans partial setup on failure |
| `term_close` | No parameters -> void | Best-effort restores input and prior screen, releases only the owned Windows screen handle, clears saved flag. Called by `client_run` on all normal exit/startup-failure paths |
| `term_size` | Width/height pointers -> success/failure | Windows visible screen-buffer window or Linux TIOCGWINSZ; writes outputs only after a successful query |
| `read_byte` | POSIX-only timeout milliseconds -> byte, zero timeout/interruption, minus one failure | Recreates select set/timeout, waits for standard input, reads one byte. EOF/failure does not spin |
| `term_key` | Timeout milliseconds -> ASCII byte, normalized KEY enum, zero no key, minus one error | Windows filters key-down records and maps arrow/Enter/Escape/Backspace. POSIX consumes bounded CSI/SS3 sequences as a unit, maps arrow finals and discards unknown sequences. This prevents an Up-arrow trailing A from becoming the A movement command |
| `term_draw` | Null-terminated UI-owned text -> success/failure | Clears/homes the active view and writes text, checking output errors. Windows enables virtual terminal processing on its temporary buffer; Linux flushes stdio |
| `platform_executable` | Output path buffer and capacity -> success/failure | Windows GetModuleFileName or Linux readlink of `/proc/self/exe`; rejects truncation before spawning a server |
| `platform_spawn` | Owned Child pointer, executable path, numeric address, port, session -> success/failure | Windows CreateProcess with CREATE_NO_WINDOW; Linux fork/exec with standard streams redirected to `/dev/null`. Saves only this child's process identity/handle |
| `platform_child_running` | Child pointer -> still running | Windows checks the owned handle; Linux waitpid with WNOHANG also reaps exited children. Clears Linux handle after exit; interruption may be retried |
| `platform_child_stop` | Child pointer -> void | Gives normal session exit a bounded chance to finish, then stops only the saved owned child. Windows closes its process handle; Linux waits/reaps and uses TERM then KILL only if needed. Joining players own no child |

Windows cursor setup hides the cursor in a new buffer, preserving the original buffer. The output mode enables ANSI processing only for the new buffer. Linux setup explicitly retains ISIG; handles SIGINT, SIGTERM, SIGQUIT and SIGTSTP through a normal shutdown request. Ctrl+Z exits this prototype cleanly rather than implementing a suspend/resume lifecycle. Forced termination or destroyed terminal resources can still prevent restoration.

Windows key-repeat counts are intentionally not expanded: one key-down record causes one movement request. The source marks this limit with a `shortcut:` comment. POSIX key sequences are bounded to 16 bytes with short waits; a complete general-purpose terminal input parser is outside this milestone.

## Client state and helper functions

`Client` owns its socket, bounded incoming/outgoing arrays and counts, current view JSON tree, welcome/end flags, dialog mode, pending-interaction flag, selected dial, view generation, feedback and typed input. Outgoing ownership ends when all bytes are sent. A previous view is deleted only when a validated replacement exists. The expected session pointer is used only during the lifetime of the hosting session's local token buffer.

| Function | Parameters and result | Purpose, branches and callers |
|---|---|---|
| `string_field` | JSON object and field name -> string or empty string | Typed string lookup for display; no mutation; used throughout UI |
| `number_field` | JSON object and field -> integer or zero | Read validated numeric values without raw unchecked dereferences |
| `text_safe` | JSON value and maximum bytes -> valid/invalid | Requires a bounded string; rejects non-ASCII and terminal controls except newline. Prevents network text from injecting terminal escape operations |
| `integer_field` | Object, field, low/high -> valid/invalid | Checks number type, range and integer equality before use in coordinates or selection |
| `valid_view` | Parsed JSON view -> valid/invalid | Requires identity/map/position/facing, boolean flags, bounded display strings, exactly eleven 29-character rows, bounded notes, known prompt kind, valid word choices and three dial values where needed. Called before replacing the UI view |
| `queue` | Client and owned action JSON -> success/failure | Serializes with protocol_print, deletes the supplied tree on all paths, rejects capacity overflow, appends bytes for partial-write pumping |
| `action` | Client, type string, whether map required -> newly allocated action or NULL | Builds type and current map fields. Caller adds typed payload fields and transfers ownership to queue |
| `end_client` | Client and reason -> void | Sets the ended flag and copies a bounded readable reason; main session loop then cleans up |
| `receive_message` | Client, complete line and byte count -> success/failure | Parses a bounded message. Welcome validates player/session and queues ready; a host rejects a different server's startup token. View validates and replaces state, increments generation, resets dialogs on map transition and opens a pending object prompt. Error/session_end stores reason. Unknown shapes fail |
| `pump` | Client -> transport success/failure | Sends pending bytes without losing unsent remainder; processes at most sixteen receive/parse turns. Retains incomplete lines, extracts coalesced messages and rejects oversized frames or closed/fatal sockets. Runs in every active in-session view |
| `append` | Output buffer/capacity plus controlled format -> void | Uses current length and bounded vsnprintf; UI formatting never treats partner text as a format string |
| `show` | UI text -> drawing success/failure | Reads current dimensions, wraps to width minus one and caps at height minus one; avoids the bottom-right scrolling cell. Called by all screens |
| `tutorial` | No parameters -> static text | Defines controls, split clues, retries, joint exits and the unfinished Map 3 boundary. Used before joining and as in-game help |
| `draw_game` | Client -> void | Shows a minimum-size notice or current identity/objective/map. Notes/help are overlays; answer/dials use dedicated views; normal play shows room, feedback and controls. Copies room rows before placing marker, so stored server rows remain intact. A drawing error ends the client |
| `input_key` | Client and normalized key -> success/failure | Escape closes/cancels locally. Answer/chat modes bound printable input, support backspace and submit only on Enter. Q quits outside text entry. Dial mode sends bounded channel adjustments/test. I/H/T select views; E queues interaction; directions queue intentions. No local answer checking |
| `ask` | Title, editable value buffer and capacity -> confirmed/canceled | Before connecting, accepts bounded editable address/port text, Enter confirms, Escape/terminal error cancels. Quit characters can be typed into fields and are not global shortcuts |
| `notice` | Bounded message -> void | Displays a result/failure and waits for Enter/Escape or stop. Used after a session is already disconnected, not while a live socket needs pumping |
| `session` | Executable path and host/join flag -> void | Asks numeric IPv4/port, starts an owned background server for Host, connects with bounded timeouts, checks welcome nonce, runs transport/input/render loop. Handshake has a finite deadline. Small windows preserve polling. On exit, briefly flushes pending quit, closes socket/deletes view/stops only owned child and reports reason |
| `client_run` | Executable path -> process success/failure | Initializes sockets and terminal, runs the four-entry main menu and tutorial/session dispatch, and restores resources on exit or startup failure |

Dialog modes are local display state, not server puzzle state. Closing a keypad with Escape does not unlock anything. Typed Q is valid text inside an answer/message, so it is not accidentally interpreted as quit. A joining client cannot shut down an unrelated server process; it owns only its socket.

## Build scripts

The scripts define no functions. The Windows script finds the repository relative to itself, verifies the known local MSVC setup, creates `submissions/game/`, compiles the explicit source list as C17 with `/W4 /WX`, links Winsock, runs `--self-test` and fails on errors. A `finally` block restores the prior working directory. The script does not install MSVC or alter system configuration. On another Windows computer, use its installed compiler setup or the documented equivalent cl invocation.

The Linux shell script requires Linux x86_64 and an installed `cc`, resolves the repository, creates the release directory, compiles the explicit source list with warnings as errors and the bounded JSON nesting definition, links the system math library, and runs `--self-test`. `set -eu` stops failed or unset-variable paths. Its executable is a native Linux binary; it is not the Windows .exe renamed. Cross-compiling with the existing portable Zig toolchain is an additional root-agent build path, not an assertion that `cc` is installed in WSL.

## Actual verification and limits

November 2 final verification supersedes the earlier blocked status below: native MSVC builds with `/W4 /WX` and passes 3207 rule checks plus 31 protocol checks. The final Windows executable passes the full two-client socket campaign and ten controlled native-console assertion groups including Q, Ctrl+C, terminal restoration and owned-server cleanup. The final Linux executable passes the same pure/socket checks, with separate two-PTY UI evidence in `campaign_linux_terminal_checks.md`. No policy was changed or bypassed.

October 31: the first full native Windows MSVC C17 build completed with `/W4 /WX`, and 3207 game-rule checks plus 29 protocol checks passed. Integration review found that comparing view pointers could miss updates if allocation addresses were reused. The code now uses an incremented view generation counter. Long clues now wrap, and answer/dial views reserve sufficient screen space. Input before the initial view cannot accidentally send a map-zero movement action.

A subsequent native build with those corrections completed without compiler warnings, but launching that newest executable was rejected by Windows Application Control. No protection was disabled, bypassed or reconfigured. Therefore current-source Windows compilation is verified, while rerunning its latest executable requires root-agent/user acceptance of the OS policy state. Consult the overall milestone report for any later successful verified run or Linux/terminal results; do not infer those from this note alone.

Pure rule/protocol checks do not prove the menu, drawing, background launch or human usability. Native console and Linux PTY session checks should exercise Host/Join, both welcome identities, partner updates while notes/answers are open, chat, resize, word choice and dials, disconnect, and owned-server cleanup. Physical two-computer LAN and human player acceptance remain separate claims.

## Student prediction and teach back

Before running, predict what happens when D targets a wall, Escape closes a wrong answer, a partner disconnects while the notes screen is open, or the terminal shrinks below 80x24. Locate the input action, network pump, view validation and drawing functions, and trace the corresponding message. Explain why the client never decides that an answer is correct.

Both John and the partner must independently demonstrate a valid and rejected input trace and make a small reviewed source change. No signatures, individual hours or successful teach-back are recorded by producing this note. The suggested professor explanation is: our two clients send intentions through TCP to one referee process; a responsive terminal loop applies only the player's permitted server view, and an owned server process is started/cleaned up by the host.
