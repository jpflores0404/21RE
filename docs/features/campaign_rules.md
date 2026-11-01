# Campaign rules and six room layouts

Implemented October 31, 2026 with AI assistance under John's explicit approval of `docs/plans/2026-10-31_MAPS_1_2_MILESTONE.md`. This feature implements server-owned rules and content for Maps 1 and 2 and the unfinished Map 3 preview. Student review and both developers' teach-back remain pending. This note does not establish networking or terminal acceptance by itself.

## Player experience

Two players explore different rooms. Player 1 discovers painting values and equations; Player 2 reconstructs the Three Blanks ordering note and opens the cabinet. Combining their clues unlocks the hatch. Both players activate their own exits before either advances.

Map 2's correct statue opens a real collision passage into the receiver chamber. Only Player 1 sees the receiver rule and alignment feedback. Player 2 adjusts and tests the dials. Both exits then advance to walkable Map 3, whose equipment and exit are explicitly unfinished. There is no gas, final synchronization or campaign victory in these rules.

## Concepts and worked example

State is information remembered between actions. The `Game` struct owns map number, puzzle flags and player records. A player record contains position, facing, discovered notes and the object currently opened for input. The server chooses which record belongs to a socket; actions do not choose the player identity.

Simple example: changing a door flag from closed to open changes what later actions allow. Project example: Player 2 submits cabinet answer `246` while facing cabinet K after the ordering note has been reconstructed. `submit` verifies ownership, facing, active interaction and stage, sets `cabinet`, records the instruction in Player 2's notes, and closes that prompt. Player 1's view still contains none of the cabinet's instruction. They must communicate it. Trying `259` at the hatch before this stage returns a prerequisite message without setting hatch authorization.

Coordinates start at zero in the top-left. Facing values are 0 north, 1 east, 2 south and 3 west. A blocked move still changes facing so the player can select the object in front. The server tests real map tiles; the client does not approve its own position.

## Files and callers

| File | Purpose and responsibility | Caller |
|---|---|---|
| `src/content.c` | Six 29x11 room layouts, fixed object positions and room names | Game movement/view and layout tests |
| `src/game.c` | Authoritative actions, puzzle state, notes, prompts and private JSON views | Server action loop and pure tests |
| `tests/test_game.c` | Deterministic rule, state, privacy and reachability checks | Application self-test entry point |
| `docs/features/campaign_rules.md` | This explanation, actual evidence and remaining learning tasks | Developers and reviewer |

The shared `src/game.h` declares the interface. The parent implementation added `active_object` so a view does not reveal a prompt merely because a player faces an object. Vendored cJSON supplies JSON allocation and serialization; this feature does not implement a JSON parser. The inherited Python game and standalone C movement demo remain separate earlier checkpoints.

## Content functions and static data

`TileObject` groups x, y and a display symbol. Six constant object arrays hold the compact room sketches. Object letters are solid interaction tiles; `.` and `@` are walkable, as is the distinctive cracked floor `*`. Each room has spawn 1,1 and exit control 14,10. Map 2's Exhibition partition occupies column 20, with passage 20,5 initially solid. No function mutates the constant object arrays.

### content_room

Inputs: map 1 through 3, player 0 or 1, an output array of eleven 30-byte rows, and passage-open flag. Output: one on success, zero for an invalid map/player/null array. It creates boundary walls and interior floor, chooses the proper object array, builds the Exhibition partition if needed, places the objects, and writes the spawn and exit. Every row is null-terminated. Opening the passage changes its actual tile to floor. Callers are movement, facing lookup, `game_view` and layout tests. No filesystem load or allocation occurs. Example: map 2/player 0/passage 0 keeps row 5 column 20 solid; passage 1 changes it to `.`.

### content_room_name

Inputs: map and player. Returns a pointer to the corresponding static room name or `Unknown room` for invalid inputs. It changes no state and allocates no memory. The view builder calls it; callers must not free its returned string.

## Game functions

### field

Inputs: JSON object and field name. Returns a string pointer owned by the JSON tree if that field is a string, otherwise an empty static string. Callers are dispatch, movement and submission. It changes no state and does not free JSON values. This allows direct rule tests to reject missing or wrong-typed strings safely even before the separate protocol validator is involved.

### number

Inputs: JSON object, name and output integer pointer. Returns one only when the value is a finite small whole number within the explicit guard range; otherwise zero. Only on success does it write the output. It guards values before integer use. Callers are stage and dial validation. The protocol validator applies narrower field-specific limits as well.

### message

Inputs: game pointer, valid internal player index, feedback text and accepted result. It copies bounded text into that player's feedback and returns the supplied result. This lets rejected actions report why they were rejected without mutating puzzle flags. Nearly every rule branch calls it. The server should send refreshed views after rejected game actions too, since feedback can change.

### note

Inputs: game, player and discovered text. Returns nothing. It compares against existing notes, ignores duplicates, and copies new text only while the fixed note array has capacity. It uses bounded strings. Interaction and successful submissions call it. Example: inspecting the equation desk twice adds one note. Notes are cleared on map transition, and each player's view includes only their own notes.

### start_room

Input: game with a valid current map. Returns nothing. For both players it restores spawn/facing, clears exit readiness, note count and opened prompt, and sets a map-appropriate introduction. It preserves connection readiness and completed earlier puzzle flags. `game_init`, the ready gate and joint exit transition call it. This centralizes transition cleanup so a previous exit-ready flag cannot advance another map.

### game_init

Input: valid writable Game pointer. Returns nothing. Clears the structure, selects Map 1, initializes both rooms, and sets waiting feedback. Server startup and tests call it. Puzzle state and readiness begin false. The caller must provide valid storage; this is an internal C API, not a network pointer.

### facing_tile

Inputs: game and internal player. Returns the tile directly in front, or wall for an invalid facing or out-of-bounds target. It builds the current room, uses the direction delta arrays and checks bounds before indexing. It changes no state. `object_at_face` calls it. This connects authoritative coordinates to object selection rather than trusting a client-supplied location.

### object_at_face

Inputs: game and player. Returns a static object identifier inferred from that player's current map and facing tile. It distinguishes the same letter on different maps. It changes no state. Interaction, submission and calibration call it. Map 3 returns harmless preview identifiers only. A missing object returns an empty string.

### interact

Inputs: game and player. Returns one for a handled interaction and zero for a rejected prerequisite. It closes the old active prompt before selecting the current object. Ordinary clue objects write a private note and feedback. Input objects open only the appropriate prompt. The correct statue sets passage-open. Exits require the map's authorization/calibration flag and both exit-ready flags before incrementing the map and calling `start_room`. Map 3 always leaves the exit sealed. Wrong statues are harmless. Unknown floor/wall interactions give readable feedback. `game_action` calls it for E intentions.

### submit

Inputs: game, player and typed JSON action containing object and answer. Returns one for accepted progress, zero for incorrect/illegal submissions. Before comparing an answer, it requires the object to match both the facing tile and previously opened interaction. It then checks map, ownership and prerequisites. The Three Blanks accepts either the displayed words or their numbered choices. Cabinet/hatch success sets one flag, closes the prompt and records only permitted notes. Wrong answers preserve all solved flags and keep input retryable. The server must never accept a client's claim that a puzzle is solved.

### calibration

Inputs: game, player, action and whether this is a Test rather than a dial adjustment. It first requires Map 2 Player 2 at an opened calibration control. Dial actions validate integer index 0-2 and value 0-4. Test rejects an unopened passage. A wrong configuration computes below/aligned/above feedback for Player 1 only; Player 2 gets `Test sent`. A correct configuration enables the joint exits and reports generic success to both. It does not include target values in Player 2's view. `game_action` calls it for dial/test intentions.

### game_action

Inputs: Game pointer, internal socket-assigned player index and JSON action. Returns accepted/rejected status. It rejects invalid pointers/player indices/nonobjects. Readiness is idempotent and starts only after both players are ready. Chat validates 1-120 printable ASCII bytes and prefixes the socket-assigned player name. Gameplay requires a started session and matching current map. Movement changes facing first, clears opened prompts, tests bounds and walkability, and then changes position or reports collision. Other known types dispatch to their rule owners; unknown declarations such as `victory` are rejected. The server and tests call it. Quit/disconnect lifecycle belongs to the server rather than these pure rules.

### prompt

Inputs: game and player. Returns a newly allocated cJSON object for that player's active prompt, or NULL when initial allocation fails. It creates `none`, `words`, `answer` or `dials` according to map, ownership, active object and prerequisites. Word choices are vocabulary, not numeric puzzle answers. Dial values are the operator's current settings, not target answers. `game_view` owns and eventually frees the resulting tree through its caller. No prompt opens merely by walking toward an object.

### game_view

Inputs: game and player. Returns a newly allocated JSON view, or NULL for invalid input/allocation failure. It selects only the current player's room, removes the static spawn marker so rendering can place the current player, and serializes position/facing, private feedback/notes, shared player messages, objective and permitted prompt. The caller deletes the JSON tree after serialization. `milestone_complete` becomes true only in Map 3, and no `victory` state is produced. No partner room, answer-key field, undiscovered equation or unrevealed cabinet instruction is serialized.

## Test helpers and checks

`tests/test_game.c` has no networking or terminal imports. Its CHECK macro increments check/failure counts and prints the failed expression and line; checks are not disabled by NDEBUG.

| Function | Inputs and result | Important state and control flow |
|---|---|---|
| `action` | Game/player/JSON text -> accepted result | Parses test JSON, checks allocation, calls real game_action and deletes the request |
| `face` | Game/player/object coordinates -> no result | Places a test fixture next to the chosen tile and clears its prompt; this is test setup, not a client capability |
| `use` | Game/player/object coordinates -> accepted result | Uses face then sends an actual current-stage interaction |
| `view_text` | Game/player -> allocated serialized view | Calls real game_view, checks allocation, deletes tree; caller frees text |
| `layout_checks` | No arguments/result | Breadth-first traversal with fixed arrays checks dimensions, one spawn, floor reachability and interaction adjacency in all six layouts, closed/open passage cases |
| `game_tests` | No arguments -> EXIT_SUCCESS on pass, EXIT_FAILURE on failure | Runs layouts, readiness, movement, stage rejection, private-note tests, wrong answers, all puzzles, both exit orders, preview stop and chat validation |

The layout traversal visits each reachable floor cell at most once. It explicitly permits the sealed receiver chamber to be unreachable before the passage opens and requires it afterward. Rule tests use direct fixture coordinates to isolate interaction behavior; an actual socket/client walkthrough must separately prove that players navigate there using movement requests.

## Actual verification

October 31, 2026: Zig 0.17.0 targeting native Windows compiled `src/game.c`, `src/content.c` and `tests/test_game.c` with C17, `-Wall -Wextra -Wpedantic -Werror` and no warnings. A small temporary test entry point in ignored `submissions/game_test_main.c` linked those files with vendored cJSON 1.7.19 and called the exported `game_tests`. It printed `Campaign rule tests passed (3207 checks).` The large count includes per-tile layout assertions, not 3207 distinct gameplay scenarios.

This evidence proves pure deterministic rules on that Windows build. Parent integration verification must separately record Linux, actual two-client socket actions, terminal input/rendering, error recovery and published binary results. No independent human puzzle fairness, physical LAN, oral explanation or student-authorship claim is made by this note.

The exported test result was then aligned with the application entry point: zero means success and nonzero means failure. The updated files cross-compiled for static Linux x86_64 musl with the same C17 warnings, and the Linux test executable ran under WSL with `Campaign rule tests passed (3207 checks).` and exit zero. Windows Application Control blocked launching the rebuilt temporary Zig Windows test executable after this return-convention change; the application integration's native MSVC build remains the relevant follow-up Windows check. The Linux test does not establish interactive terminal or LAN acceptance.

## Operating Systems connection and teach back

These files hold ordinary memory in the authoritative server process. They contain no socket or terminal calls. The OS concept demonstrated by the surrounding system is process isolation and one owner of shared game state; the pure functions make those rules testable without OS input.

Professor explanation: "The server owns the player positions and puzzle flags. Clients request an action, and the server checks the current map, facing object, player ownership and prerequisites before updating state. It then builds a different permitted view for each player."

Before marking learning complete, each developer should predict a blocked move, a hatch attempt before cabinet opening, wrong calibration feedback, and the result of one player activating an exit twice. Trace the actual function and changed fields, run the focused checks and explain why the partner does not receive hidden clues. Reviewer names, actual attempts and dates remain pending until performed.

Known limits: fixed compact maps and fixed puzzle values are intentional milestone content. Map 3 is deliberately unfinished. This module does not perform protocol framing or terminal cleanup; those belong to separate owners and tests. Proposed changes beyond the approved milestone require their own scope decision.
