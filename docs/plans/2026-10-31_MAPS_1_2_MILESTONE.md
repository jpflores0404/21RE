# Maps 1 and 2 milestone implementation proposal

Prepared October 31, 2026, Asia/Manila. Status: explicitly approved by John in this chat with "approve" on October 31. Implementation and final controlled verification are recorded in the [November 2 report](../progress_reports/2026-11-02_maps_1_2_milestone.md). Human acceptance and teach-back remain pending. The proposal below is the implementation contract; completion claims require the acceptance evidence.

## Goal and authority

John requests a C milestone with three maps, localhost socket communication between two visible terminal players, a main menu, tutorial, completed Maps 1 and 2 puzzles, and an unfinished Map 3. Native Windows builds are for local use only; publish the Linux executable, source, tests and necessary documentation to GitHub after verification. Preserve the existing Windows demo executable locally.

This revises the older roadmap's milestone boundary: John wants Maps 1 and 2 to define the **50 percent milestone**, rather than the old Map 1 at 50 / Map 2 at 70 split. Treat this as a named scope/checklist, not an independently measured fraction of all development effort or professor acceptance.

Puzzle reference: [John's Google Doc](https://docs.google.com/document/d/1n1D6RwnGbuIFZLbJZibmuNqlMBP-6M8pRATalRgAizQ/edit), retrieved October 31. It specifies paintings 246, hatch 259, the statue/passage, calibration 2/3/4, and proposed bracketed refinements. The source document stays unchanged. The earlier master prompt and audit remain historical references where the C revision and this scope supersede them.

The repository requires concrete plan approval before substantial implementation. John's current request explicitly asks for AI implementation and subagents, so the proposed implementation will be AI-assisted and documented honestly; developer review and teach-back remain pending until actually demonstrated.

## Investigation findings

- C currently implements only the standalone movement demo. The prior Linux binary was built and tested under WSL; some older documentation predates that verification.
- Python assets contain the alternate eight-task, 50x15 Map 1 and mathematical answer 1259. Do not copy those rules into the new campaign.
- The submission draft contains six consistent compact 29x11 room sketches. Reuse their layout ideas and test them as real C content.
- The C demo supplies reusable terminal setup/restoration and coordinate concepts, but no facing, world collision, interaction, network, lobby or campaign state.
- Its Linux reader mishandles arrow escape sequences. Address that during terminal input extraction so menus and answer entry do not inherit accidental movement.
- Native MSVC and the temporary Zig cross-compiler are available. Reuse them; no system package installation is proposed.

## Proposed player experience

One new executable, `2r1e`, with Host, Join, Tutorial and Quit menu entries. Windows produces the corresponding local `2r1e.exe`.

Terminal A chooses Host. The program starts a background copy of itself in server mode, then connects Player 1 over TCP. Terminal B chooses Join at 127.0.0.1 and connects Player 2. There are two visible player terminals and three OS processes: one referee and two clients. Also expose a standalone `--server` mode for testing and explanation.

This preserves the original authoritative-server model. An alternative embedding the referee inside the host UI loop avoids child-process launch code, but couples server lifecycle to terminal rendering. Recommend the separate background server. Start it directly with OS process APIs, not string-built shell commands. Save its process identity and shut down only the child started by this host. Surface bind/startup errors and bound startup waits.

Default to loopback for same-device demonstrations. Permit explicit IPv4 address/port configuration for later LAN use. Separate-computer LAN acceptance remains pending until actually tested. No internet backend, account system or database is needed.

Controls: WASD moves/changes facing; E inspects or interacts with the facing object; I opens discovered notes; Enter confirms; Escape cancels a view; Q leaves with clear feedback. Provide a small bounded T message input so the two terminals can exchange clue descriptions as well as synchronized game updates. No chat history service or chat enhancements.

Tutorial covers controls, facing, clue sharing, wrong-answer retry and the milestone stop. Main menu/tutorial/notes/answer input must continue processing socket events. A too-small terminal displays a resize message while network handling continues.

## Puzzle and content contract

| Map | Player rooms | Implemented milestone behavior |
|---|---|---|
| Records Wing | Gallery and Archive | Paintings/cabinet 246, mathematical hatch 259, private clues and joint exit |
| Observation Wing | Exhibition and Maintenance | Unique statue selection, real opening passage, receiver/dial calibration 2/3/4, joint exit |
| Containment Sector | Research Laboratory and Security Control | Walkable layouts, harmless object descriptions and visibly unfinished sealed controls |

### Map 1

Player 1 discovers Eye=6, Lantern=2, Key=4 and the equations. Player 2 discovers the ordering note. Cabinet submissions belong to Player 2 and require an adjacent facing interaction. Correct code 246 reveals the Z-X-Y final-digit instruction only to Player 2. Player 1 submits 259 at the hatch after the cabinet stage. Authorization enables both exits. Both exit-ready flags are required for one synchronized transition.

Use the bracketed Three Blanks suggestion as a clue-reconstruction interaction within Player 2's ordering note. Present curated word choices for light, passage and watcher rather than exact free-text grading. The supplied passage poem also suggests door or bridge; the UI must make the intended vocabulary clear and allow retry/hints. This is part of the paintings clue, not an added physical hidden passage or unrelated puzzle engine.

### Map 2

Statue B uniquely satisfies north-facing and nearest the distinctive cracked tile. Wrong statues give harmless feedback. Correct interaction changes passage P to walkable floor and opens the receiver chamber.

Only Player 1 sees the receiver rule and per-channel below/aligned/above feedback. Player 2 operates the three labeled dials with values 0 through 4 and the Test control. Testing cannot solve calibration before the passage is open. Correct configuration 2,3,4 enables exits; both players activate their respective controls to advance together.

### Map 3 stopping condition

Load both Containment rooms after Map 2. Show that Maps 1 and 2 are complete for this milestone and Containment puzzles are under development. Permit walking and harmless inspection. Final controls remain sealed. Do not implement valve resolution, gas, countdowns, final synchronization, or campaign victory yet.

Wrong answers remain retryable. Canceled dialogs preserve progress. Repeated completion and exit actions are harmless. Clear per-map readiness/feedback at transitions. Discovered notes are per player and deduplicated.

## Architecture and dependency decision

Use a small set of C modules with explicit structs and bounded arrays, not a single giant source file or generic puzzle framework. Keep the old movement demo as a learning checkpoint.

| Proposed files | Responsibility |
|---|---|
| `src/main.c` | Entry points and application lifecycle |
| `src/platform.c/.h` | Windows/Linux terminal, sockets and owned server process lifecycle |
| `src/protocol.c/.h` | Bounded JSON-line framing, typed messages and validation |
| `src/content.c/.h` | Six compact room layouts and server-owned object/clue metadata |
| `src/game.c/.h` | Authoritative positions/facing, legal actions, puzzles, progression and permitted views |
| `src/server.c/.h` | Lobby and single state-owning socket event loop |
| `src/client.c/.h` | Menu, tutorial, input, responsive view rendering and interaction dialogs |
| `tests/` | C rule/protocol tests and controlled two-client socket/UI checks |
| `scripts/` | Reproducible native Windows and Linux builds/checks |

Preserve the already proposed JSON-line wire contract. Recommend vendoring a pinned reviewed release of [cJSON](https://github.com/DaveGamble/cJSON), its C source/header and MIT license. Compile it into the application, with no system installation or runtime DLL needed. This is the one proposed new source dependency and is included in the approval request. Review upstream version/security notes before selecting the pin. A simple bounded text command protocol is viable but changes the existing JSON requirement; it is not the recommended option.

Use one nonblocking server `select` loop to own all authoritative game state. Clients send intentions, never declarations of success. The connection determines player identity. Validate message schema, field types, bounds, stage, position, ownership and prerequisites before mutation. Reject duplicate critical fields/trailing junk and unknown message types. Limit nesting before/while parsing rather than trusting arbitrary JSON trees.

Proposed limits: client action line 1024 bytes, answer 64 printable ASCII characters, player message 120 characters, server view 16384 bytes, per-connection pending output 65536 bytes, and at most 16 actions per loop turn. Validate that all actual generated views fit; revising these numeric limits without changing the design is an implementation detail. Retain fragmented input and partial writes, bound queues and waiting, and disconnect a client that exceeds safe capacity. Do not block one player's progress waiting for a slow peer.

Clients receive their permitted current room/view and discovered notes, never answer keys or unrevealed partner clues. Treat chat text as bounded plain text and reject control sequences. Server-only source can remain readable in the educational repository; runtime separation is the boundary being implemented.

Assign exactly two slots, require readiness, clearly reject a third join, and end the session explicitly on player disconnect. Host quit must clean up its owned server; a joining client cannot terminate arbitrary processes. No automatic reconnect or saved sessions in this milestone.

## Execution and review order after approval

1. Record approval, the revised milestone and the AI-assisted implementation scope. Update current planning/instruction pointers for C without rewriting the original master prompt.
2. Establish stable action/view and platform contracts; add the pinned parser, build commands and focused checks.
3. Implement authoritative server and two-client host/join lifecycle, bounded framing and private views. Prove localhost communication before dependent puzzles.
4. Build six validated room layouts, menu/tutorial, movement/facing/collision, interaction and notes. Implement and test Maps 1 and 2's full chains and the Three Blanks refinement.
5. Add the explicit unfinished Map 3 preview. Review all new code and failure paths against the milestone; correct issues within scope.
6. Build/test native Windows and Linux. Keep Windows executables/objects in ignored local outputs. Publish the Linux x86_64 executable with executable file mode, source and documentation after checks pass.
7. Supply a local Windows run command and launch the Windows player application for John if useful. Linux execution is verification, not the program John is asked to play locally.

Use subagents for independent bounded components after shared contracts are fixed, avoid overlapping file edits, and use a separate reviewer for integration/security. The root agent remains responsible for reconciling and inspecting the final result. Do not claim speed or cost savings without measurements.

## Acceptance checklist for this milestone

- Main menu, tutorial and host/join paths work on both platforms.
- Two real socket clients can complete the gameplay on the same device using two visible terminals.
- All six rooms validate; interaction neighbors are reachable; the hidden chamber is sealed before and reachable after opening.
- WASD/facing/collision, E interactions, notes, answer/dial controls and player messages work without leaking hidden data.
- Maps 1 and 2 can be solved from the intended split clues; wrong actions retry and cannot skip prerequisites.
- Both players transition together; Map 3 is explorable but cannot produce endgame victory.
- Invalid/malformed/oversized/fragmented/coalesced network input, forged identity/outcomes, third join, disconnect, busy port and slow output have explicit checks.
- Menu, notes and input dialogs process updates/disconnects; resize, small windows, Q and handled interrupts preserve terminal usability.
- Linux binary and native Windows build pass relevant tests; distinguish pure rules, socket probes, controlled terminal tests and manual acceptance.
- Every changed file/function has the required explanation and actual evidence. Student reviews and teach-back stay pending until performed.
- Git diff is reviewed; no Windows `.exe`, `.obj`, toolchains or temporary probe logs enter the commit. The local Windows executable remains available. Linux binary is included.

Report accepted checklist items and remaining gaps. Do not relabel partial work as accepted 50% merely because the target is 50%. Physical two-computer LAN, full Map 3, gas and final victory remain outside this milestone's demonstrated completion.

## Approval recorded

John explicitly approved this concrete plan in chat on October 31, including the background authoritative server, pinned cJSON source dependency, constrained Three Blanks interaction, revised Maps 1-2 milestone, unfinished Map 3, AI-assisted implementation with subagents, verification, local Windows build and GitHub publication of source/docs/Linux binary only. The approval authorizes implementation but does not establish accepted behavior or student understanding; record actual results in the milestone report.
