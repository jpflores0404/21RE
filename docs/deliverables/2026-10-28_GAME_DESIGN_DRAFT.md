# 2R1E Game Design Draft

## Week 1 submission for the Operating Systems project

Submission date: October 28, 2026. Prepared on October 27, 2026. Version 0.1 for discussion and review.

Developer A: John Francis Espiritu. Developer B: name to be supplied by the team. Course section and professor: to be supplied before submission.

This document presents our proposed game, three map drafts, sample cooperative puzzles, technical direction, and first prototype scope. Our Week 1 checkpoint has a 20 percent target. That target is not a measured claim that 20 percent of the finished C game has been accepted.

The professor requires C. We will develop and test first on Windows while retaining Linux as the intended final demonstration platform. The existing Python prototype is reference material. The map layouts and new puzzle details below are proposals for implementation and playtesting, not completed game features.

## Game overview

2R1E stands for Two Rooms One Exit. It is a two-player cooperative terminal escape game set inside a sealed research facility. Each player occupies a different room and sees different evidence. A clue in one room explains an object or control in the other. Players must describe what they see, combine information, and operate their own controls to escape together.

The tone is suspenseful rather than combat-focused. The terminal uses readable ASCII rooms, short environmental descriptions, and clear feedback. Increasing difficulty comes from combining clues and coordinating actions. The required campaign has three maps and approximately six main cooperative challenges, plus one conditional gas emergency.

| Item | Proposed specification |
|---|---|
| Genre | Cooperative puzzle escape game with suspense |
| Players | Exactly two human players |
| Language | C17 |
| Presentation | Text terminal with ASCII maps and object inspection |
| First testing environment | Native Windows terminal |
| Final target | Linux terminal on two computers connected over LAN |
| Network model | One authoritative server and two clients |
| Intended duration | Approximately 25 to 35 minutes after playtesting |
| Team | Two developers with rotating implementation and review roles |

## Story and player objective

Two people regain consciousness in separate rooms of a closed research facility. The facility has entered lockdown, and ordinary exits are sealed. Records suggest that the building's access systems were deliberately divided between two operators. Neither player has enough information or control to escape alone.

The Records Wing introduces the facility's symbols and access procedures. The Observation Wing conceals a passage and requires coordinated calibration. The Containment Sector contains the final authorization and safety controls, with a possible gas incident if a dangerous valve is selected.

The players win only when both complete the final release conditions. The server advances both players through the campaign together. Reaching an earlier map's exit does not count as winning the entire game. There is no combat, enemy AI, weapon system, procedural generation, or paid online service in the required scope.

\pagebreak

## Core gameplay and terminal interface

The repeated play sequence is explore, inspect, communicate, attempt an action, read feedback, and observe the resulting room change. Each clue should have an identifiable source and a reason to share it. Important objects should be discoverable without testing every tile at random.

| Control | Intended campaign behavior |
|---|---|
| W A S D | Move one tile; a blocked move may still change facing |
| E | Interact with the object directly ahead |
| F | Inspect details and record discovered information |
| I | Read the player's discovered clues |
| Enter | Confirm a selected control or answer |
| Escape | Close a view or cancel an unfinished input |
| Q | Leave through a clear confirmation or return path |

These are proposed campaign controls. The first movement demo supports WASD and Q only. Its player marker occupies the available terminal area without campaign objects. Fullscreen means using the current terminal viewport, which the presenter can maximize; it does not require changing the operating system's display mode.

For the campaign, reserve space for the current map and room, player identity, objective, interaction feedback, and controls. During gas, show the remaining time and emergency state on every active view. Text labels must explain status even when colors are unavailable. The initial minimum campaign terminal target is 80 columns by 24 rows; smaller windows should show a resize message instead of drawing out of bounds.

## Shared map notation

The following sketches are compact design diagrams. They are not production asset files. Coordinates start at column zero and row zero in the upper-left. Objects occupy solid tiles and are used from an adjacent walkable tile. Each proposed room is 29 columns by 11 rows, including its boundary. A larger terminal may center it without changing game coordinates.

| Symbol | Meaning |
|---|---|
| # | Solid wall |
| . | Walkable floor |
| @ | Initial player position |
| E | Sealed exit control, used from the tile above |
| A B C D and other letters | Solid object, identified in that room's object list |
| P | Hidden passage wall segment before it opens |
| * | Cracked floor marker that remains walkable |

Every sketch includes one spawn and an exit. Each object has an intended reachable neighboring tile. The Observation Exhibition has a sealed chamber; its interior is intentionally unreachable before the passage opens. Reachability and interaction adjacency must be tested again when these sketches become actual map assets.

## Campaign progression

Records Wing paintings -> mathematical hatch -> both room exit controls -> Observation statue -> calibration -> both room exit controls -> Containment valve -> authorization and safety release -> synchronized final activation -> victory for both.

A designated valve mistake branches into gas recovery and then returns to the valve objective. This is the only planned timed emergency. An otherwise correct playthrough can avoid it.

\pagebreak

## Map 1 The Records Wing

Difficulty: introductory. Intended playtime: approximately 7 to 9 minutes, not yet measured. Player 1 occupies the Gallery and Player 2 occupies the Archive. This map teaches navigation, inspection, sharing information, and entering a code supported by evidence.

### Gallery room draft

```text
#############################
#@..........................#
#....A.......B.......C......#
#...........................#
#...........................#
#...........................#
#.......D............H......#
#...........................#
#...........................#
#...........................#
##############E##############
```

| Object | Position | Player 1 can discover |
|---|---|---|
| A Eye painting | 5 2 | A watcher symbol with inscription 6 |
| B Lantern painting | 13 2 | A light symbol with inscription 2 |
| C Key painting | 21 2 | A passage symbol with inscription 4 |
| D Equation desk | 8 6 | X equals 14 minus 9, Y equals 3 times 3, Z equals 8 plus 4 |
| H Mathematical hatch | 21 6 | Three-digit input, initially locked |
| E Exit control | 14 10 | Sealed until hatch authorization is obtained |

### Archive room draft

```text
#############################
#@..........................#
#.....N.............K.......#
#...........................#
#...........................#
#...........................#
#.......L...................#
#...........................#
#...........................#
#...........................#
##############E##############
```

| Object | Position | Player 2 can discover |
|---|---|---|
| N Ordering note | 6 2 | The required order of the three painting symbols |
| K Locked cabinet | 20 2 | Three-digit input; opens after the painting code |
| L Archive shelf | 8 6 | Short story detail; clearly identified as background |
| E Exit control | 14 10 | Sealed until the team obtains hatch authorization |

### Puzzle 1 The Three Paintings

Player 1 sees the symbol-to-number mapping but does not see the order. Player 2 reads: "First comes the light. Then the means of passage. The watcher shall always remain last." The note does not contain the numbers.

The players associate light with Lantern, passage with Key, and watcher with Eye. Player 2 enters 246 at cabinet K. On success, the cabinet opens and exposes the instruction for the next puzzle. On an incorrect code, it displays a short rejection and permits another attempt. No item or attempt is consumed.

A sample exchange is: Player 2 asks for the numbers on the Lantern, Key, and Eye. Player 1 replies 2, 4, and 6. Player 2 enters the code and reads the new instruction aloud. The dependency requires useful contributions from both rooms.

\pagebreak

## Map 1 puzzle flow and acceptance

### Puzzle 2 The Mathematical Hatch

After the cabinet opens, Player 2 reads: "Use Z, then X, then Y. Keep only the final digit of each result." Player 1 can inspect the equations on desk D and operate hatch H.

The worked solution is X = 14 - 9 = 5, Y = 3 x 3 = 9, and Z = 8 + 4 = 12. In Z-X-Y order, retain 2, 5, and 9. Player 1 enters 259. The answer is not 1259 because the instruction explicitly asks for the final digit of each result.

The hatch opens and grants the team exit authorization. Both players must use their own exit controls. The first activation marks that player ready to leave but leaves exploration available. When both are ready, the server loads the next map for both at the same time. There is no exit synchronization timer on this introductory map.

### Player information and state changes

Player 1 initially knows the painting values and equations. Player 2 initially knows the painting order. Only the opened cabinet reveals the hatch sequence. The server stores the cabinet-open flag, hatch authorization, and each player's exit-ready flag. The client receives visible consequences without answer keys or unrevealed partner clues.

The planned state order is cabinet locked -> cabinet open -> hatch authorized -> first player ready -> both ready -> Observation Wing. Entering 259 before the cabinet stage is active should return a prerequisite message rather than skipping progression.

### Hints and recovery

First painting hint: "Describe the symbols, not just the numbers." A stronger hint explicitly connects light to Lantern. First hatch hint: "Twelve has two digits; the instruction asks you to keep one." Hint wording is proposed and should be tested with students unfamiliar with the answers.

Wrong codes preserve the room and all discovered clues. A player may close a keypad without resetting progress. Repeated inspection should not duplicate clue entries. A repeated correct submission should give already-completed feedback without granting duplicate rewards.

### Acceptance checks for implementation

- All initial objects have a reachable interaction tile from the room's spawn.
- The code 246 opens only the cabinet and reveals the hatch instruction.
- The code 259 succeeds only at the active hatch; 1259 is rejected.
- Each room contributes essential information to the intended solution.
- One exit activation does not advance either client by itself.
- Both ready flags advance both players exactly once.

### Design choices still open

This draft uses the two main cooperative puzzles. Inherited wiring, swipe-card, breaker, and download tasks are not required by this draft's chain. Whether to retain any as optional content needs an explicit scope decision. They must not delay the C networking and campaign requirements.

\pagebreak

## Map 2 The Observation Wing

Difficulty: intermediate. Intended playtime: approximately 8 to 11 minutes, not yet measured. Player 1 occupies the Exhibition Room and Player 2 occupies Maintenance. The theme is interpreting a partner's blueprint and using feedback from a receiver the operator cannot see.

### Exhibition room draft

```text
#############################
#@..................#.......#
#...................#.......#
#....A.......B......#...R...#
#............*......#.......#
#...................P.......#
#...................#.......#
#....C..............#.......#
#...................#.......#
#...................#.......#
##############E##############
```

| Object | Position | Function |
|---|---|---|
| A Statue A | 5 3 | Faces east |
| B Statue B | 13 3 | Faces north; intended correct statue |
| C Statue C | 5 7 | Faces north, farther from the crack |
| * Cracked floor | 13 4 | Unique visible floor marker |
| P Passage segment | 20 5 | Becomes walkable after the correct statue |
| R Receiver | 24 3 | Inside the sealed chamber |
| E Exit control | 14 10 | Enabled only after calibration |

### Maintenance room draft

```text
#############################
#@..........................#
#.....B.....................#
#...................D.......#
#...........................#
#...........................#
#...................T.......#
#...........................#
#...........................#
#...........................#
##############E##############
```

| Object | Position | Function |
|---|---|---|
| B Blueprint | 6 2 | Identifies the statue by orientation and floor marker |
| D Calibration controls | 20 3 | Three dials A B C, each with values 0 through 4 |
| T Test switch | 20 6 | Submits the current dial positions |
| E Exit control | 14 10 | Enabled after verified calibration |

### Puzzle 3 The Missing Statue

Player 2's blueprint reads: "The service latch is beneath the north-facing statue nearest the marked cracked tile." Player 1 sees the statue orientations and cracked floor, but does not have the blueprint rule. Statue B is north-facing and one tile from the crack. Statue C also faces north but is farther away, so only B satisfies the full clue.

Player 1 interacts with B. The server changes passage P from wall to floor, and Player 1 can enter the previously sealed chamber. A wrong statue produces a harmless mechanical click and a retryable message. This puzzle changes actual collision geometry, not only descriptive text.

\pagebreak

## Map 2 calibration and acceptance

### Puzzle 4 Signal Calibration

The chamber receiver displays: "Channel A begins at two. Channel B is one above A. Channel C is one above B." These exact values and wording are proposed sample content. Player 1 reads the rule and sees receiver feedback; Player 2 has the labeled dials and test switch.

The worked solution is A = 2, B = 3, C = 4. Controls show each dial's current value and allow individual adjustment without a speed requirement. Player 2 chooses values and activates Test. Before the passage is open, testing reports that the receiver connection is unavailable and cannot solve the puzzle early.

On a test, Player 1 receives one message for each channel: below target, aligned, or above target. Player 2 sees only "Test sent; ask the receiver operator for the result." A test such as 1, 3, 4 produces "A below target; B aligned; C aligned" on Player 1's receiver. This reinforces communication while the written rule prevents blind guessing from being the intended strategy.

When all channels align, both players see calibration accepted, both exits become available, and both players use their exit controls to advance. Incorrect settings remain editable and do not damage the receiver. There is no shared power resource and no second emergency timer.

### State and information boundaries

Server state includes passage-open, the three dial values, last permitted receiver feedback, calibration-complete, and exit-ready flags. The Maintenance client receives control values but not the target answers. The Exhibition client receives the discovered receiver clue and feedback. Ordinary source access in a public educational repository cannot conceal solutions from someone who reads the implementation; runtime views still follow the intended player boundaries.

### Hints and accessibility

A statue hint reminds players that both orientation and distance matter. A calibration hint says: "Start with A, then calculate the next two channels." Statue orientation must be written as North or East during inspection, not implied only through a small glyph. Dial names and values must remain readable without color.

### Acceptance checks for implementation

- Exactly one statue satisfies the blueprint and cracked-floor relationship.
- Before unlocking, there is no route into the receiver chamber.
- Opening P creates a continuous reachable path to a tile adjacent to R.
- Wrong statues do not consume progress or create a dead end.
- Each dial accepts only values zero through four.
- Tests before passage activation cannot complete calibration.
- Configuration 2, 3, 4 succeeds; a near miss gives accurate per-channel feedback.
- Both clients advance only after calibration and both exit activations.

### Design choices still open

The three small integer dials are the recommended first version. Animated tuning meters or a timing minigame add implementation work without improving the core information-sharing requirement. Paper-test the clue with both developers before approving final values.

\pagebreak

## Map 3 The Containment Sector

Difficulty: advanced. Intended playtime: approximately 9 to 12 minutes, not yet measured. Player 1 occupies the Research Laboratory and Player 2 occupies Security Control. Both players combine maintenance records, recover from one possible emergency, and coordinate a final release.

### Research Laboratory draft

```text
#############################
#@..........................#
#.....M.....................#
#...........................#
#...........................#
#....1....2....3....4.......#
#...........................#
#...........................#
#.....C.................A...#
#...........................#
##############E##############
```

| Object | Position | Function |
|---|---|---|
| M Maintenance note | 6 2 | Excludes red and blue valves from routine clearance |
| 1 Red valve | 5 5 | Labelled Bypass |
| 2 Blue valve | 10 5 | Labelled Exhaust |
| 3 Yellow valve | 15 5 | Labelled Feed; designated incident trigger |
| 4 White valve | 20 5 | Labelled Return; correct clearance valve |
| C Emergency card | 6 8 | Gives the ventilation sequence before or during gas |
| A Authorization terminal | 24 8 | Enables Player 1 clearance after the safe valve |
| E Final exit control | 14 10 | Records Player 1's final activation |

### Security Control draft

```text
#############################
#@..........................#
#.....R.....................#
#...................V.......#
#...........................#
#...........................#
#...........................#
#...................S.......#
#...........................#
#...........................#
##############E##############
```

| Object | Position | Function |
|---|---|---|
| R Maintenance record | 6 2 | Identifies the required return-line function |
| V Ventilation panel | 20 3 | Intake Filter Exhaust sequence controls |
| S Mechanical safety release | 20 7 | Enables Player 2 clearance after the safe valve |
| E Final exit control | 14 10 | Records Player 2's final activation |

### Puzzle 5 Containment Valve Investigation

Player 1 reads: "Red and blue valves are excluded from routine clearance." The room shows each valve's color and function in text. Player 2 reads: "For routine clearance, operate the return line. Feed lines must remain closed." Player 2 cannot see which colored valve has that label.

Combining the documents identifies the white Return valve, number 4. Interacting with it grants containment clearance. Red or blue gives a safe rejection without changing progress. Yellow Feed triggers the gas incident. Colors supplement the printed numbers and names, so the solution does not rely on color recognition.

\pagebreak

## Map 3 emergency and final escape

### Conditional challenge Poison Gas Emergency

The designated wrong valve starts a server-controlled 180-second countdown. Both rooms show a gas warning and remaining time. Player 1's emergency card reads: "Ventilate in this order: INTAKE, FILTER, EXHAUST." The card is accessible from the initial room and remains available during gas. Player 2 operates these named controls on panel V.

A correct three-step sequence stops gas. An incorrect step clears only the partial sequence, gives feedback, and allows a new attempt while the original deadline continues. Reopening the panel, repeating the valve, or sending an invalid action does not extend the deadline.

After successful ventilation, the players still need the correct Return valve before final clearance. Proposed rule: the dangerous valve is isolated after recovery and cannot start a second incident during that attempt. The incident is conditional, so a safe successful run does not need to trigger it merely to unlock the exit.

If time expires, reset Map 3 for both players: restore spawn positions, close clearance, clear final activation times and partial ventilation input, and restore the initial valve state. Earlier maps remain completed. Restore the written clues to their original reachable locations. This makes the current level the checkpoint. The renewed attempt may trigger one incident again.

Use a monotonic clock, which measures elapsed time without depending on changes to the wall-clock date. The server compares action processing time with the deadline. Proposed boundary: an action processed at or after the gas deadline is too late, and reset happens first. These exact rules remain subject to review and playtesting.

### Puzzle 6 One Exit Two Decisions

With containment clearance granted and gas inactive, Player 1 activates authorization terminal A. Player 2 activates mechanical release S. Both prerequisites must be satisfied before either final exit attempt counts.

The first valid final exit activation begins an eight-second window. The second player must activate their own exit within that window. Proposed boundary: a difference of exactly eight seconds succeeds; a larger difference fails. Repeated presses from the same player do not extend the active window. Expiry clears both attempts and displays a retry message. An activation after expiry may begin a fresh window once the old attempt has been cleared.

Only the server confirms final success and sends victory to both clients. A valid final activation cannot succeed during gas, before valve clearance, or before either release prerequisite. For example, activations at elapsed times 100 and 107 seconds succeed; times 100 and 109 require another attempt.

### Acceptance checks for implementation

Test the safe Return valve, harmless wrong valves, incident trigger, incorrect ventilation sequence, exact gas deadline, successful ventilation, one incident per attempt, full Map 3 reset, missing release prerequisites, both activation orders, duplicate presses, and eight-second boundary. Verify that timer updates remain visible while reading a clue or using a panel.

\pagebreak

## Technical design and Operating Systems concepts

The final session consists of three processes: a server and Player 1 client on Computer A, and Player 2 client on Computer B. A process is a running program with its own memory. The server is the referee: it owns positions, puzzle flags, deadlines, and progression. Clients request actions and display only the information permitted for their player.

TCP sockets carry framed messages over LAN. TCP supplies a stream of bytes, so receiving one message may require several reads, and one read may contain several messages. The planned JSON-line framing therefore keeps incomplete input, validates bounded complete messages, and preserves unsent output. A small C JSON dependency still needs review and approval before implementation.

One server event loop is the proposed owner of mutable game state. It checks socket readiness, processes bounded work, updates deadlines, and sends results. This demonstrates concurrency without requiring several threads to modify the same state. The implementation should reject wrong message types, forged identities, impossible movement, out-of-stage actions, and client declarations of success.

| OS concept | Planned game example | Evidence needed |
|---|---|---|
| Processes | Separate server and two client programs | Show the three running instances |
| Terminal input and output | Immediate keys, redraw and restored console settings | Trace input to movement and demonstrate clean exit |
| Socket communication | LAN action and view messages | Two-computer test and message explanation |
| Concurrency | Both clients act while timers continue | Controlled simultaneous actions and deadline test |
| Synchronization | One state owner and joint exits | Explain invariants and test both action orders |
| Timers | Gas deadline and final activation window | Deterministic boundary tests and live demo |
| Fault handling | Invalid input or disconnected client | Clear rejection, cleanup, and explicit session end |

Windows and Linux share rules but need different low-level console and socket calls. Proposed adapters use Winsock and Windows Console input on Windows, and POSIX sockets and termios on Linux. An early Linux build must check portability before the full campaign is built. Windows success alone cannot prove Linux acceptance.

## Failure handling and usability

Wrong answers are retryable. A bad network message must not mutate game state or crash the session. If a client disconnects, the server informs the remaining player and ends the session clearly; automatic reconnection is outside the initial scope. Startup errors should explain a busy port, invalid address, missing assets, or unsupported terminal.

The game must restore console settings and cursor state when it exits through supported quit and interrupt paths. Room geometry should not stretch with terminal size. A resized campaign viewport should redraw or show a clear minimum-size notice. Player identity, object names, clues, and warnings must remain readable without color.

\pagebreak

## Week 1 prototype and demonstration plan

The first C prototype is deliberately narrow: one visible player marker, WASD movement, terminal boundaries, resizing behavior, and Q to quit. It does not claim to implement maps, networking, collision with furniture, inspection, or puzzles. It provides an initial input -> state update -> terminal output example that both developers can trace.

A position contains a column x and row y. If a player at 10,5 presses D, the proposed candidate is 11,5. The movement function accepts it only if it remains inside the playable border. At the right edge, D leaves the position unchanged. Rendering then draws the marker at the accepted position.

On October 27, the first demo compiled as C17 using the existing Windows MSVC compiler with warnings treated as errors. All 14 pure movement checks passed. A controlled native Windows console check also verified actual key input, the four boundaries, resizing, and restoration after Q and Ctrl+C. Human presentation rehearsal, Linux compilation, and both students' code explanations remain pending. These checks establish the small demo only, not campaign completion.

The intended presentation sequence is to open a terminal, maximize it, run the compiled demo, press W A S D, demonstrate a blocked border movement, resize the terminal, and quit with Q. After quitting, type a normal command to show that the terminal is usable. Actual build commands and results belong in the accompanying movement guide and evidence notes after verification.

| Week 1 material | Status in this design submission |
|---|---|
| Game description and campaign structure | Documented draft |
| Six room sketches and object lists | Proposed layouts for review |
| Puzzle clues and worked solutions | Draft specifications, not playtested |
| C and Windows-first direction | Requested October 27 |
| First movement demo | Separate small deliverable; consult its actual verification record |
| LAN server and campaign puzzles in C | Future implementation |
| Linux and two-machine acceptance | Future verification |
| Both developers explaining the code | Must be demonstrated; notes alone are insufficient |

## Development milestones

| Check-ins in 2026 | Target | Planned focus |
|---|---|---|
| October 26 and 28 | 20 percent | Game design, reviewed scope, C platform direction, first movement prototype |
| November 4 9 and 11 | 50 percent | Authoritative two-client connection and accepted Map 1 |
| November 16 and 18 | 70 percent | Map 2, compact Map 3, emergency recovery and final exit |
| November 23 and 25 | 100 percent | Linux LAN acceptance, fixes, packaging and oral demonstration |

These are the professor's checkpoint targets. The C migration increases workload, so a missed early gate requires an explicit scope or schedule discussion. We will not report 100 percent while required Linux, campaign, or learning criteria remain unmet.

## Team workflow and attribution

John Francis Espiritu and the second developer will rotate implementation and review roles. For each small approved step, the implementer explains the function inputs, outputs, state changes, and failure cases. The reviewer predicts the result of a concrete example, runs the relevant check, and traces the actual code.

AI assistance is used for planning, drafting, examples, review, and the explicitly requested initial demo. Both students remain responsible for understanding and presenting the work. Existing Python code is inherited reference material and must not be described as newly written C functionality.

\pagebreak

## Verification plan and decisions for review

Before implementing each map, verify every reachable floor region, interaction location, and dependency. The sealed Observation chamber is the only intentionally inaccessible starting region in these sketches. Test that opening its passage changes walkability correctly. Map loading must reject invalid dimensions, missing spawns, unknown symbols, and unsafe metadata.

For rules, use small tests without a terminal. Cover correct and incorrect answers, prerequisites, duplicate actions, private information, both-player progression, gas recovery, reset completeness, and timing boundaries. For C, include bounds and cleanup checks and use memory diagnostics supported by the selected compiler.

For connections, test fragmented and combined messages, oversized input, invalid fields, a third client, disconnects, and incomplete writes. For terminal behavior, test small windows, resize, repeat keys, reading a clue during a timer, and normal and interrupted shutdown. Final acceptance requires a clean build and complete campaign on two Linux computers over LAN.

### Review questions

1. Are the three maps and six primary cooperative challenges acceptable for the course scope?
2. Are the proposed calibration values, valve clues, and ventilation sequence clear and fair?
3. Is resetting only Map 3 after gas expiry acceptable?
4. Is the conditional emergency acceptable when a safe route avoids triggering it?
5. What Linux machines or environment will be available for the final LAN demonstration?
6. What checklist and weighting should establish measured completion against the checkpoint targets?

### Known risks and planned response

The largest risk is completing a C port and three-map campaign while both students learn the implementation. Keep rooms compact, defer optional minigames and chat, and verify networking before dependent content. If the timeline becomes infeasible, request an explicit revision rather than silently removing required behavior.

The proposed answers are visible here because this is a development document. Player-facing clues must reveal only the relevant information during play. A public source repository is not a secure place to hide solutions from readers, but the running clients should still receive only permitted views.

The duration estimates, difficulty levels, clue fairness, and final layouts require playtesting. This draft supplies concrete examples for review; it does not assert that the campaign is implemented, balanced, or approved.

### Source and revision record

This draft follows the original three-map campaign in docs/MASTER_PROMPT.md, the baseline findings in docs/REPOSITORY_AUDIT.md, and the October 27 revision in docs/C_IMPLEMENTATION_PLAN.md. The older Python-specific plans remain historical references. John requested this submission draft and approved the narrow assistant-written movement demo on October 27, 2026.

Before submission, supply the partner name, section, professor, and any actual demonstration evidence. Review the puzzle proposals together. Do not copy unverified completion claims from the inherited weekly report drafts.
