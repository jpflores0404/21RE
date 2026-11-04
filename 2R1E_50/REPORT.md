# Classroom prototype report

Francis approved the separate C17 prototype with Enter-based controls, a combined host and Player 1, and the two existing map puzzle chains. Codex supplied the implementation with AI assistance. Team review and oral rehearsal are pending. The label “50” identifies the Maps 1 and 2 demonstration scope.

## What the two computers do

The host calls `socket`, `bind`, and `listen`, then `accept` waits for the second player. `bind` gives the server TCP port `5050` on the host. The joining PC converts the host’s LAN IPv4 address into a socket address and calls `connect`. Once `accept` and `connect` return, each program has one connected socket. The host closes its listening socket and keeps the connected one. Both ends use `send` and `recv` on that same connection.

`host_game` holds one `Game` struct in memory. When the host presses Enter, it applies the command to Player 1. When a command comes from the socket, it applies it to Player 2. For example, `1 d` tells the host that Player 2 pressed `d` on Map 1. The `1` is the map number. The program already knows the sender is Player 2 because the command came from the joining socket.

TCP carries bytes in order. `receive_line` checks whether a byte is ready and adds it to a small buffer. The newline tells the program that the command is complete. The host sends the guest a map number, the screen’s text lines, and a line containing just `.` to mark the end of the screen. `join_game` collects those lines before printing the screen. `send_text` keeps sending if a call only writes part of the text.

Both programs alternate checking the keyboard and socket. The socket check waits at most 20 milliseconds for the next byte before the loop checks the keyboard again. One player can send a message while the other is deciding what to do. The prototype uses the school’s trusted LAN and connects one guest per session.

## Follow one command

1. Player 2 types `d` and presses Enter. `input_line` reads keyboard input and `add_character` recognizes the end of the command.
2. `join_game` adds the current map number and calls `send_text`.
3. `host_game` receives the line and calls `play_command` with player index `1`. Internally, `0` is Player 1 and `1` is Player 2.
4. `play_command` sets the facing direction east, then asks `tile_at` what is to the right. A floor tile lets it add one to `x`. Facing still changes if an object blocks the move, so `e` can inspect it.
5. `refresh` calls `draw_screen` separately for each player and sends the guest their room and clues when their screen changes.

The same rule function handles answers and chat. `answer` checks the facing tile, player, and prerequisite before accepting a code. `calibrate` handles three dial values and gives alignment feedback to the receiver player. `use_exit` records each player’s exit and changes maps only after both use it.

## Files and functions

| File | Functions and data to study |
|---|---|
| `demo.h` | `Line` stores an unfinished input line. `Player` holds position, facing, clue text, and exit status. `Game` holds both players, current map, puzzle flags, dials, and latest chat. `Socket` gives both platforms one handle type. The header also declares the limits and function interfaces. |
| `main.c` | `main` opens the menu and starts or joins a socket session. `host_game` owns state and routes both players’ commands. `join_game` sends commands and displays the host’s screen. `refresh` draws both views and only prints or sends a changed one. `quitting` recognizes `q` and `Q`. |
| `net.c` | `net_start` and `net_stop` initialize and release Winsock on Windows. `host_player` binds port 5050 and accepts one guest. `join_host` parses the address and connects. `close_socket` releases an open handle. `limit_send_wait` sets a timeout for a stalled send. `send_text` sends the complete buffer or reports failure. `receive_line` returns `1` for a line, `0` while waiting, or `-1` for a closed or failed connection. |
| `terminal.c` | `read_prompt` reads the menu and address. `input_line` collects Enter-based commands without waiting for a player to type before checking the socket. Linux uses `select` and `read` on normal terminal input. Windows uses `_kbhit` and `_getch`. `add_character` collects printable characters up to the line limit and discards invalid or oversized lines. `show_screen` prints and flushes a spaced text screen. |
| `game.c` | `rooms` contains the four ASCII maps. `tile_at` returns the room tile or a wall for a position outside the grid. `reset_players` sets the spawn position and facing. `start_game` initializes Map 1. `inspect` reads clues and opens the statue passage. `answer` checks the word, cabinet, and hatch solutions. `calibrate` sets and tests the dials. `use_exit` records both exits and advances or finishes. `play_command` handles a command from either player. `draw_screen` formats a player’s room, clue, chat, and controls. |

The approval kept the puzzle chains and simplified the interface to commands. Map 1 still uses the Three Blanks clue, three paintings, cabinet, and hatch. Map 2 still requires the correct statue to open a real passage and both players to cooperate on calibration. Player 2 sets the three dials together with `set` and checks them with `test`. The program keeps the latest clue and chat on screen, and `e` can reread a nearby object.

## Verification and remaining work

- Compiled the Linux x86_64 executable with Zig 0.17.0 targeting static musl using C17 and warnings as errors. Ran the actual Linux file under WSL with two program instances in separate pseudo-terminals.
- Selected Host and Join, exchanged messages both ways, and completed both maps. The walkthrough checked that clues stay with their player, wrong answers can be retried, the hatch waits for the cabinet, the statue opens the passage, the receiver sees private calibration feedback, and both exits are required. Both programs returned to the menu and quit successfully.
- Built the native Windows executable with MSVC C17 and warnings as errors. Ran its Host and Join console paths against controlled TCP peers. Checked movement, chat, a command split across multiple sends, an oversized line followed by a valid one, disconnect, and clean quit.
- Physical two-PC school LAN play, a native GCC build on a school PC, and the team’s own rehearsal remain to be checked. The Linux file is the executable included in the repository. The Windows file is a local practice build.

## One paragraph for the professor

Our 2R1E demo is a two-player escape game written in C. One computer starts as Host and listens on TCP port 5050. The second computer connects using the host’s LAN IP address. We use `send` and `recv` to exchange short text commands. The host runs the game rules for both players, checks their moves and puzzle answers, and sends each player the view of their own room. Players share clues to solve the cabinet and hatch in Map 1, then open a passage and calibrate a signal in Map 2. Both players must use their exits to finish. We ran both maps over a real socket connection on one machine, and the school LAN demonstration is our next check.
