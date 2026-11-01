# Campaign sockets and protocol feature record

Approved by John on October 31, 2026 through approval of `docs/plans/2026-10-31_MAPS_1_2_MILESTONE.md`. Implemented with Codex assistance. Student authorship, partner review and teach-back remain pending. This record covers the networking bundle; the campaign rules and terminal UI have their own records.

## Concept and player experience

A TCP socket is one end of an ordered byte stream between programs. It does not preserve message boundaries. The client sends intentions as one JSON object followed by newline. The server owns the actual game state and sends each client only its permitted view. A received valid JSON object still requires schema and game-rule validation.

Simple example: writing `ab` and later `c\n` can arrive in separate reads, but the receiver keeps `ab` until the newline completes `abc`. In the project, a partial `{"type":"move"` waits in the connection buffer until the remaining fields and newline arrive. Multiple newline-delimited actions in one read are processed separately.

The host starts an owned background server through the application layer. The server binds the selected IPv4 endpoint, assigns two slots, sends a welcome with a session identity, and waits for both players' ready actions. A third client is rejected. Quitting or disconnecting ends this session clearly, and a fresh host starts a new one. There is no automatic reconnection or persistent session restoration.

## Files and interfaces

| File | Purpose and callers |
|---|---|
| `src/net.c` | Windows Winsock/POSIX socket adapter used by server and client |
| `src/protocol.c` | Strict bounded action decoder and JSON-line encoder used at network boundaries |
| `src/server.c` | One authoritative event loop; called by the executable's `--server` mode |
| `tests/test_protocol.c` | Exported `protocol_tests`, called by the executable self-test entry point |
| `tests/integration_campaign.py` | Independent real socket clients driving the actual server, called by a developer/test command |

`net.h`, `protocol.h`, `game.h` and `app.h` are the shared contracts. Their ownership and the vendored cJSON source are covered by the overall implementation record. No original Python runtime files are modified by this bundle. JSON is parsed by the pinned cJSON dependency, while this bundle validates allowed fields and actions.

## Socket functions

| Function | Parameters and result | Work, state and failures |
|---|---|---|
| `retry_error` | No parameters, true for retryable native errors | Maps would-block/interrupted/in-progress errors to a common convention; reads error state only after failed operations |
| `net_init` | No parameters, success/failure | Starts Winsock 2.2 on Windows; POSIX needs no startup |
| `net_cleanup` | No parameters, no result | Balances the Winsock startup in its owning process |
| `net_close` | Socket, no result | Closes a valid OS socket; invalid sentinel is harmless |
| `net_nonblocking` | Socket, success/failure | Sets `FIONBIO` on Windows or preserves existing flags while adding `O_NONBLOCK` on POSIX |
| `address_for` | IPv4 text, port, output sockaddr; success/failure | Validates port 1–65535, converts numeric IPv4 and network-byte-order port; no DNS or shell use |
| `net_listen` | IPv4 text and port; listener or invalid sentinel | Creates, binds, listens and makes nonblocking; Windows exclusive address ownership, POSIX restart-friendly reuse; closes on setup failure |
| `net_accept` | Listener; accepted nonblocking socket or invalid sentinel | Accepts one peer and closes it if nonblocking setup fails |
| `net_connect` | IPv4 text, port, timeout; connection or invalid sentinel | Starts nonblocking connect, waits for write readiness and checks `SO_ERROR`; closes on invalid endpoint, timeout or failure |
| `net_read` | Socket, destination and byte capacity; bytes/zero/-1/-2 | Receives up to bounded capacity; zero means closed, -1 retry later, -2 fatal; rejects zero/oversized capacities |
| `net_write` | Socket, source and byte count; bytes/zero/-1/-2 | Sends potentially only part of the bytes; POSIX suppresses broken-pipe signals with `MSG_NOSIGNAL`; caller retains unsent suffix |
| `net_wait` | Socket list, wanted writes, count, timeout and result arrays | Rebuilds read/write sets, bounds descriptor use, waits with select, marks readiness; -1 fatal, interrupted wait treated as no readiness |

The adapters neither mutate game state nor interpret puzzle answers. There are no worker threads. Windows socket handles are normalized through `intptr_t`; POSIX descriptors are checked before entering `FD_SET`.

## Protocol functions

| Function | Input and output | Important checks and ownership |
|---|---|---|
| `printable` | JSON value and character limit -> true/false | Requires a string with printable ASCII and bounded decoded length |
| `integer` | JSON value and inclusive range -> true/false | Rejects fractions, wrong types and out-of-range values before use |
| `fields` | Object and exact field-name set -> true/false | Rejects missing/extra fields and duplicate root keys with a small seen bitmask |
| `protocol_action` | Complete line bytes and length -> owned cJSON object or NULL | Bounds line at 1024, rejects raw controls/non-ASCII and Unicode escapes, bounds nesting before parsing, requires full JSON consumption, validates each exact action schema |
| `protocol_print` | JSON value, destination and capacity -> newline-inclusive length or zero | Serializes with cJSON, bounds the result, copies text plus newline and terminator, frees temporary string; caller retains its original JSON value |

Actions are ready, quit, move, interact, submit, dial, test and chat. Identity is never an accepted client field. Numeric map IDs range 1–3, dial IDs 0–2 and dial values 0–4. Answers are at most 64 printable ASCII characters, object IDs 32 and player messages 120. Unsupported JSON Unicode escapes are rejected rather than allowing embedded NUL/control bytes to bypass C string checks.

For example, `{"type":"move","map":1,"direction":"up"}` is structurally accepted; `{"type":"ready","player":2}` is rejected. A structurally accepted move at a wall or stale map is still handled by the authoritative rule layer. The network decoder cannot by itself establish that a game action is legal.

## Server functions and state

Each Connection owns a socket, an input buffer of 1025 bytes and pending output of at most 65536 bytes. The extra input byte permits a full 1024-byte action plus newline. View serialization is capped at 16384 bytes. Queue limits prevent a slow peer from consuming unlimited memory.

| Function | Inputs/result and callers | Execution and failure behavior |
|---|---|---|
| `server_stop` | OS callback, sets stop flag | Handles Ctrl+C/Ctrl+Break on Windows or SIGINT/SIGTERM on POSIX; no cleanup inside callback |
| `should_stop` | No inputs, stop flag | Read by the loop; Windows uses atomic Interlocked operations |
| `queue` | Connection and JSON -> success/failure | Encodes and appends a complete frame if output capacity permits; never drops a partly transmitted frame |
| `notice` | Connection, type and message -> success/failure | Allocates a small JSON notification, queues it, then deletes its temporary JSON |
| `views` | Both connections and Game -> success/failure | Calls `game_view` separately for each connected identity, queues, deletes views; does not send the other player's view |
| `close_connection` | Connection, no result | Closes socket, clears buffer lengths/state and restores invalid sentinel |
| `end_session` | Both connections and message | Appends a session-end notice after existing output so partially sent frames remain valid; closes an over-capacity connection rather than corrupting the stream |
| `consume` | Connections, identity and Game -> continue/end | Extracts at most 16 complete lines per turn, rejects malformed input with error, dispatches accepted intentions, broadcasts permitted views, treats quit or capacity failure as session end |
| `reject_third` | New socket, no result | Tries bounded partial writes of a clear rejection for at most 100 ms, then closes it |
| `server_run` | Address, port and session string -> process status | Initializes sockets and state, binds, accepts, waits/read/writes with one owner, exits idle lobby after 60 seconds, drains final output for at most 300 ms, and releases its owned resources |

The loop preserves incomplete action bytes and unsent view suffixes using `memmove`. It processes bounded work per connection per turn. Pending buffered complete lines request an immediate next select cycle, preventing buffered data from being stranded while no new bytes arrive. A connection close or fatal send causes one explicit session end. The final 300 ms drain is bounded; an unresponsive peer can be closed before receiving the final notice.

## Tests and evidence

`protocol_tests` iterates accepted and rejected examples, checks oversize rejection and serialization capacity, prints a count and returns the failure count. Zero means pass. It changes only local objects and buffers. It does not open a socket or prove rendering.

The integration driver creates an owned server subprocess with hidden-window creation on Windows. `Peer.__init__` connects and verifies welcome/session; `Peer.next` reconstructs lines and selects expected message types; `Peer.send` writes encoded intentions; `Peer.close` releases its socket. `start_server` chooses a test port, starts and waits for its own process; `update` consumes one view per player; `act` supplies the player's map and performs a synchronized action/update; `approach` finds a real path from the received room rows and sends each movement/facing/interaction action; `campaign` drives both maps and the unfinished third preview and releases every owned test process/socket in a finally block. The `server_command` helper selects direct native launch or an explicit `--wsl` launch through docker-desktop; this mode lets the Windows standard-library driver test the actual Linux server over forwarded localhost. `lifecycle_faults` checks abrupt disconnect and an oversized incomplete line in fresh owned sessions. Its command-line block accepts an executable path and the optional `--wsl` testing flag.

The integration route checks all four Map 1 clue objects, word reconstruction, wrong/correct cabinet and hatch answers, private notes, both-player exits, the statue passage, dials, Map 3 stopping condition, malformed messages, forged identity/outcomes, duplicate keys, fragmented/coalesced frames, a busy port, third join and peer quit. Real socket checks are still distinct from interactive terminal play and physical two-computer LAN acceptance.

Compilation evidence: on October 31 the networking, protocol, server and protocol-test source files individually compiled for both Linux musl and Windows targets with C17, `-Wall -Wextra -Wpedantic -Werror`. A standalone Linux protocol-test executable ran through WSL with 31 passed and zero failed, including exact 64-character answer acceptance and 65-character rejection. The combined Linux binary then passed `python tests/integration_campaign.py --wsl releases/linux-x86_64/2r1e`: the complete two-client socket campaign through Maps 1 and 2 into the unfinished Map 3, private notes, fragmented/coalesced frames, invalid/forged/stale actions, busy-port rejection, third join, normal quit, abrupt disconnect and oversized incomplete-line termination. The Windows driver used forwarded localhost to the real Linux server. This is automated socket evidence, not human terminal play or two-computer LAN acceptance. Slow-output capacity and every possible partial-write scheduling pattern are protected by bounded code and review but not exhaustively forced by this driver.

## Worked trace and professor explanation

Final November 2 reruns pass against both the native Windows executable and the rebuilt Linux executable. The integration helper now also runs `slow_peer(binary)`: it connects two players, makes the second socket's receive buffer small, stops draining that peer, and continues sending one ready action at a time while reading the first peer's updates. The loop has a deadline and action cap. Queue exhaustion must end the session and the server must exit successfully within a bounded wait. An initiating connection can receive EOF instead of the final notice when a view cannot be queued; the UI reports the closed connection clearly. All sockets and the owned test process are released in `finally`. `Peer.next(kind)` accepts either one message kind or a tuple for that test. This proves bounded backpressure handling, not every short-write/retry schedule.

Player 2 sends a cabinet answer -> the socket adapter receives bytes -> consume waits for newline -> protocol_action checks the exact schema -> game_action verifies map, identity, facing, reachability and puzzle prerequisites -> only the server opens the cabinet -> game_view creates each permitted response -> queue retains the bytes until partial writes finish -> the client renders the approved result.

"TCP sends a stream, so our code reconstructs complete bounded JSON lines. The server validates player actions and owns the shared state. Both terminal clients receive only their permitted views, and nonblocking I/O keeps a slow client from freezing the referee."

## Student review still required

Both developers should predict whether a split action changes state before its newline, trace a wrong-player cabinet request, and explain why an incomplete outgoing frame cannot be discarded. They should distinguish parsing from schema checks and game authorization. Actual authorship, partner review, code changes made by students and teach-back must be recorded when they occur; this generated note is not proof of understanding.
