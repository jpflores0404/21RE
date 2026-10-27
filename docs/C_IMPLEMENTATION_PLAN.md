# C implementation plan revision

October 28 status update: John subsequently approved the first standalone C movement demo and notes, limited to WASD, terminal bounds, resizing and clean exit. See [the run guide](MOVEMENT_DEMO.md) and [feature record](features/movement_demo.md). Windows checks passed; Linux compilation remains unverified. This does not approve the full campaign port. The dated proposal below records the original planning boundary.

Decision recorded October 27, 2026, Asia/Manila. John Francis Espiritu reported the professor's requirement to develop the game in **C** and requested **Windows-first testing with Linux support**. He authorized this documentation revision and its GitHub push. This does not authorize implementing the port, installing tools, or reducing the campaign.

## Authority and baseline

This revision supersedes Python-specific implementation proposals and the inherited four-person, single-map plan. Preserve the original master prompt and historical audit as records; their Python requirements describe the earlier baseline. The team remains two developers. The existing Python game remains a reference prototype, not a C build or evidence that the revised requirements are complete.

The target remains one authoritative server, exactly two clients, TCP over LAN, private player views, three cooperative maps, retryable wrong answers, checkpoint recovery, one gas emergency, and shared final victory. Target playtime remains approximately 25–35 minutes, subject to real playtests. Optional chat and decorative work remain deferred. The C migration adds workload; percentages and dates remain grading targets, not measured completion or guaranteed delivery.

## Language, platforms, and tools

- Use C17, small functions, explicit structs/enums, bounded buffers, and clear ownership. Keep rules independent of networking and terminal code. Do not translate Python classes line for line.
- Windows is the first development/test environment. Propose GCC through MinGW-w64 for native Windows builds and GCC on Linux. Confirm available tools before writing build instructions. No compiler or package installation is authorized by this plan update.
- Propose a small CMake build for separate server, client, and test executables, with warnings enabled. Select and approve the build setup before creating it. Source must compile as C, not C++.
- Linux remains a required acceptance target. WSL can provide an intermediate Linux build/test environment if separately installed with permission; it does not replace two-computer Linux LAN testing.
- Use the C standard library plus small platform-specific adapters. Standard C alone does not provide TCP sockets, immediate terminal input, or a portable monotonic clock.
- Keep JSON-line TCP framing as the proposed contract. C has no standard JSON parser. Before implementation, review a small maintained C parser, its license, limits and ownership model, then obtain dependency approval. Do not write a general JSON parser just to avoid a dependency. Review asset JSON compatibility at the same time; preserve existing text maps/art where useful.

## Proposed module boundaries

These paths are proposals, not files already implemented.

| Area | Responsibility |
|---|---|
| `src/game/` | Map loading, collision, puzzle rules, progression and authoritative state; no OS/terminal imports |
| `src/server/` | Lobby, assigned identities, action validation, permitted views and one state-owning event loop |
| `src/client/` | Input intentions, received views, terminal rendering and feedback; no answer keys |
| `src/shared/` | Bounded framing and validated message structures; no puzzle solutions |
| `src/platform/` | Only the socket, terminal, clock and shutdown differences needed for Windows/Linux |
| `tests/` | C rule/protocol tests and controlled connection probes, kept distinct from inherited Python tests |
| `assets/` | Reviewed reusable maps and art; answer-bearing metadata loaded only by the server |

Use one small server loop with nonblocking sockets and bounded work. Proposed socket readiness uses `select` for this two-client scale. Rebuild readiness sets each iteration; respect platform-specific handle types, errors and limits. Retain incomplete reads and unsent bytes, cap message/output sizes, handle disconnects and prevent a slow client from blocking the other client or deadlines. Do not require threads merely because the Python prototype used them.

| Concern | Windows | Linux |
|---|---|---|
| TCP lifecycle | Winsock startup/cleanup, `SOCKET`, `closesocket`, Winsock errors and nonblocking mode; link Winsock | POSIX sockets, file descriptors, `close`, errno and nonblocking mode; handle broken-pipe writes safely |
| Terminal | Windows Console input events; enable supported virtual-terminal output and restore original modes | `termios` input and terminal escape output; restore original modes |
| Time | Monotonic elapsed time using `QueryPerformanceCounter` | `clock_gettime(CLOCK_MONOTONIC)` |
| Shutdown | Request loop exit through an appropriate console handler; perform cleanup in normal execution | Signal handler requests loop exit; perform cleanup outside the handler |

Normalize keys and errors at these boundaries. Keep a short input wait so network changes and timers remain visible in every screen. Handle resize, a terminal smaller than 80x24, EOF, unsupported console modes and startup failures clearly. Avoid `system("cls")`, shell-driven rendering, Windows-only assumptions in game rules, and a permanent blocking `scanf` gameplay loop.

Alternative: ncurses on Linux with PDCurses on Windows could reduce manual terminal handling, but adds two library/build integrations. The recommended initial proposal is the small native adapter above. Confirm it with a tiny approved input/render/cleanup exercise before building the full UI. If that exercise exposes excessive complexity, review the library option before changing dependencies.

## Campaign preserved

| Map | Required cooperative chain | Details still to finalize before implementation |
|---|---|---|
| Records Wing | Gallery painting numbers plus Archive ordering clue produce `246`; equations and partner's Z-X-Y final-digit instruction produce hatch code `259`; both players exit together | Review inherited layouts, object positions, clue wording and any retained solo tasks. Existing Python `1259` is a known mismatch, not the intended answer |
| Observation Wing | Blueprint identifies the north-facing statue nearest a uniquely marked cracked floor; correct interaction opens a real passage; receiver clue helps partner calibrate controls | Exact room grids, unique statue placement, passage tiles and final calibration clue/values. Proposed A=2, B=A+1, C=B+1 is not yet accepted content |
| Containment | Split valve clues identify a safe action; one designated wrong valve triggers gas; partner instructions enable ventilation; authorization and safety release enable final joint activation | Exact labels, clue text, emergency sequence, checkpoint snapshot/reset fields and boundary behavior |

Gas uses an approximately 180-second server deadline. Final activations use an approximately eight-second server window. Wrong answers remain retryable; expiry resets the current approved checkpoint/level for both players; a missed final window allows retry. Exact timing rules require an approved specification and boundary tests. No client decides success or receives partner secrets or answer keys.

## Revised grading phases

| Phase/check-ins, 2026 | Planned work and acceptance |
|---|---|
| Week 1: Oct 26/28, target 20% | Record C/Windows/Linux revision; study inherited movement/network flow; agree platform/build/parser proposals, campaign dependencies and two-developer roles. Paper-test clues and identify Linux access. Deliver a reviewed plan and honest report, not a claim of a working C game |
| Week 2: Nov 4/9/11, target 50% | After explicit approval, build small C steps: compiler smoke test, pure movement test, platform socket/clock/terminal exercises, bounded protocol, two-client authoritative lobby, private views, and complete Map 1. Windows localhost first, then an early Linux compile/rule/socket check before dependent campaign work. Nov 11 gate requires two actual clients completing Map 1 |
| Week 3: Nov 16/18, target 70% | After Map 1 acceptance and phase approval, add compact Map 2, Map 3, environmental changes, gas recovery and final activation. Verify reset fields and exact timer boundaries with an injected test time. Check both platforms as each adapter-dependent feature lands |
| Week 4: Nov 23/25, target 100% | Freeze features; run complete Windows regression and Linux terminal/LAN acceptance; fix failures, measure playtime, extract/build/run a clean package, rehearse source explanations and record actual evidence |

The added language migration puts this schedule at greater risk. If the early C/platform or Map 1 gates fail, bring evidence and request an explicit scope/deadline decision. Do not silently drop required maps, validation, Linux support or student learning. No new implementation phase is approved by this revision.

## Small-step workflow and verification

1. Both developers review this revision. Agree one small function/group, its implementer/reviewer and its acceptance check. Rotate roles and use concept, example, developer attempt, review, test and teach-back.
2. Establish reproducible Windows build/test commands only after the toolchain is confirmed. Build a minimal Linux counterpart early. Record compiler, OS, terminal and actual commands/results.
3. Test pure C rules without a terminal: movement/collision, reachability, prerequisites, wrong-answer retry, permitted views, transitions, reset and timer boundaries.
4. Test malformed and oversized messages, lengths/types, invalid IDs, ownership, stale stages, fragmented/coalesced reads, partial writes, third join, disconnect, shutdown and bounded queues. In C, also check buffer lengths, allocation failures, cleanup paths and memory lifetime. Use sanitizers where the selected compiler supports them; record unsupported checks.
5. On Windows run one server plus two client processes on localhost, including resize and modal-screen updates. Later run the full campaign on two Linux machines over LAN, including firewall/network failures and no-internet operation. Localhost, WSL and Windows evidence must be labeled separately.
6. Each implementation includes a feature note accounting for every changed file/function and an honest check-in report. Distinguish inherited Python behavior, newly written C behavior, tests run and developer understanding. Never report a grading target as achieved without accepted evidence.
7. Before implementation, update the shared repository instructions to reflect C under explicit approval, preserving the student-led workflow. Do not treat their historical Python-specific guidance as authority to implement the new game in Python. Keep original master requirements intact and link this dated revision instead.

## References and status

Platform reference documentation: [Winsock initialization](https://learn.microsoft.com/en-us/windows/win32/winsock/initializing-winsock), [Windows console I/O](https://github.com/MicrosoftDocs/Console-Docs/blob/main/docs/high-level-console-input-and-output-functions.md), [Linux select](https://man7.org/linux/man-pages/man2/select.2.html), [Linux terminal input](https://man7.org/linux/man-pages/man3/termios.3.html), [Linux monotonic clocks](https://man7.org/linux/man-pages/man3/clock_gettime.3.html).

Approved requirement change: C implementation, Windows-first testing, Linux support, and documentation push, requested by John on October 27. Proposed implementation choices above still require review before code changes. No C source, compiler installation, Windows gameplay, Linux build or LAN acceptance is established by this document.
