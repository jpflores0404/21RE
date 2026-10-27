# Two-developer learning and implementation roadmap

> October 28 update: [the C implementation revision](C_IMPLEMENTATION_PLAN.md) supersedes the Python-specific future modules and tooling below. The first standalone movement demo was explicitly approved and implemented as an early Week 1 exception; see [its feature record](features/movement_demo.md). The remaining campaign steps still require approval.

**Status: proposed implementation plan, awaiting approval.** The grading calendar below was explicitly confirmed by John Francis Espiritu on October 23, 2026. Approval of dates does not approve code changes or campaign reductions.

## Goal and constraints

Build and understand a Linux terminal escape game with two LAN clients, one authoritative server, private player views, three compact cooperative maps, recoverable failure and a final shared victory. Keep Python 3.10+ and standard libraries. Reuse understandable existing code. Every small implementation includes tests, a file/function explanation and teach-back by both developers.

The master prompt's eight-week horizon is superseded by the confirmed grading calendar. Its gameplay and authority requirements remain the target. No percentage below is an assessment of today's code.

| Grading phase | Confirmed check-ins, 2026 | Professor's target | Meaning here |
|---|---|---|---|
| Dev Week 1 | Oct 26, Oct 28 | 20% | Agree on the game, understand inherited baseline, approve architecture and content contracts |
| Dev Week 2 | Nov 4, Nov 9, Nov 11 | 50% | Build and explain a reliable authoritative two-client Map 1 slice |
| Dev Week 3 | Nov 16, Nov 18 | 70% | Add and explain Map 2 and the smallest complete Map 3 mechanics |
| Dev Week 4 | Nov 23, Nov 25 | 100% | Freeze features, verify Linux/LAN/campaign, close bugs, rehearse and package |

These are four grading phases across about a month. Dev Week 2 spans more than one calendar week. The original eight-week milestone sequence still informs dependencies, but cannot be presented as the approved calendar.

## Feasibility and scope decisions

Three maps plus an authority correction and code-level learning in this time is **high risk** for two developers. The existing movement, assets, inspection and terminal drawing reduce the amount of new work. The architecture correction must happen before adding dependent puzzles. Teaching and real LAN tests must have scheduled time; a large generated replacement would miss the learning goal.

Recommended approach: retain the full target, approve only one phase at a time, make each map small, and treat the following gates as actual stop-and-review points.

- Oct 28: both developers can explain the inherited input/rule/network flow and have approved an action/state protocol. If not, spend the next session on that explanation rather than generating a server.
- Nov 4: two clients connect with assigned identities, ready state and private views. If that fails, defer decorative UI and optional chat work, not validation.
- Nov 11: two actual clients complete Map 1 using server validation. If that fails, request a scope/deadline revision before proceeding to Map 2. The master's end-of-Week-4 Map 1 checkpoint moves earlier because the calendar is shorter.
- Nov 18: all required maps and timed recovery exist and have focused checks. If not, propose an explicitly approved short-campaign scope or deadline change. Do not silently turn Maps 2/3 into stretch goals.
- Nov 23: freeze features. Nov 25 is for fixes, evidence, explanation and final acceptance, not first-time network integration.

Specific fallback proposals, each requiring approval if it changes required scope: fewer decorative objects/artworks; text choices instead of animated dials; compact rooms; fewer optional solo tasks; one short version of each required puzzle. If still infeasible, a polished Map 1 demonstrator can be an honest reduced submission only after scope agreement with the developers and, where required, the professor. Local Tab mode and a backup video are rehearsal aids, not replacements for required LAN evidence.

Optional chat already exists. Leave it alone unless it blocks the approved work. Do not add chat features, advanced inventory, procedural content, save games, auto-reconnect, extra hazards, dependencies or a generic puzzle engine.

## Two developers, shared understanding

Developer A is **John Francis Espiritu**. Developer B is the partner, name to be confirmed. Role allocation below is a proposal, not an assertion about past contributions.

| Phase | A implements first | B implements first | Shared responsibility |
|---|---|---|---|
| 1 | Trace rules/state and document a movement example | Trace socket/thread/UI flow and content dependencies | Teach the other half, review protocol and draw the puzzle flow |
| 2 | Server state and validation | Framing/connection lifecycle and client integration | Review each other's functions, switch ownership for tests and one feature |
| 3 | First Map 2 puzzle and level transitions | Second Map 2 puzzle and first Map 3 rules | Pair on timer/recovery/final window; swap lead on the next puzzle |
| 4 | Game-rule regression, packaging/runbook | LAN/UI/error regression, demo procedure | Both run the game, explain all OS concepts and review evidence |

One person writes a function; the other reviews it. Avoid two branches changing the same function simultaneously. Agree on the message/data contract before splitting client/server work. Neither developer is allowed to become the only person who understands networking or game rules.

## Proposed architecture, not implemented

```text
Computer A                              Computer B
Python server process                   Python Player 2 client
  authoritative session                    keyboard + curses
  positions / puzzle states                     |
  deadlines / campaign                          | framed JSON/TCP
       |                                        |
       +------------ LAN -----------------------+
       |
Python Player 1 client
  keyboard + curses
```

An authoritative server is the referee. A client says "move left" or "submit 259", not "I solved the hatch". The referee checks the rule and then tells that player what changed. Computer A runs two separate processes; Computer B runs one. No internet service is required.

Proposed additions, only after approval:

| Module | Narrow responsibility | Reuse |
|---|---|---|
| `shared/protocol.py` | Decode/validate bounded JSON-line envelopes; encode permitted messages; no answer keys | JSON-lines format and socket lessons from `client/network.py` |
| `server/main.py` | Listen, accept exactly two clients, own socket buffers, dispatch actions, check deadlines, shutdown | Existing socket lifecycle knowledge |
| `server/session.py` | Authoritative players, rooms, puzzle states, legal actions, filtered player views and progression | `game/` movement/loading/session behavior where suitable |
| Existing `game/` | Pure map/rule functions and simple state structures | Keep working collision/inspection code |
| Existing `client/` | Input, view rendering, networking queue, feedback | Reuse curses helpers and artwork |
| Existing `assets/` | Server-loaded maps/object metadata, client-visible art when permitted | Current text and JSON formats |

Keep package marker files simple. This is a proposed addition to the existing structure, not a requirement to rewrite every module. Add a dedicated puzzle module only if a concrete level's rules no longer fit readably in the authoritative session; avoid a framework.

Recommended concurrency model: one server loop owns all authoritative state and uses standard-library `selectors` to check which connections have input/output ready. A selector is a waiter that reports which socket can make progress, so the server does not get stuck waiting on Player 1 while Player 2 acts. Explain this before coding it. Keep per-connection incoming/outgoing buffers, preserve unsent bytes, and handle partial writes. Do not put blocking `sendall` in the state-owning loop. Process bounded work per turn so incoming spam cannot starve deadlines.

Alternative worth considering: one reader thread per socket feeding a queue to one state-owning loop, which builds on the existing client. It is viable but needs thread shutdown and bounded output handling; two reader threads must not mutate state directly. Choose it if both students understand it better and explicitly approve the contract. Do not mix concurrency models merely for novelty.

On the client, keep the existing reader-thread/queue separation if practical, but poll updates during every active screen. A queue safely passes messages between the receiving thread and UI loop. The UI draws only state approved by the server. Puzzle answers are never included in the view. The public repository can contain server source, but the running client must not load the entire answer-bearing level.

## Proposed protocol and lifecycle

Use a small explicit set of dictionaries rather than a messaging framework. Final field names and bounds are decided in Phase 1.

| Direction | Example/intention | Validation |
|---|---|---|
| Server -> client | `welcome`, assigned player 1 or 2 | Client accepts only valid server envelopes |
| Client -> server | `ready` | Bound connection has assigned slot; duplicate ready is harmless |
| Client -> server | `move`, direction `left` | Known direction, active stage, room bounds and collision |
| Client -> server | `inspect` or `interact` | Server uses authoritative position/facing and permitted object |
| Client -> server | `submit`, object ID, input string | Object exists, belongs to player, is reachable/active, prerequisites met, bounded input |
| Server -> client | `view`, permitted room/player/puzzle feedback | Exclude answers, partner's unrevealed clues and hidden objects |
| Server -> client | `error`, readable reason | Reject invalid action without changing state |
| Server -> client | `transition`, `timer`, `victory`, `session_end` | Only server decides these states |

Connection sequence: listen -> accept and assign two slots -> send welcome -> both ready -> initialize Map 1 -> process actions -> transition both players together -> final victory -> clean close. Reject a third connection clearly. If one client disconnects, notify the other, end that session and allow a fresh lobby/session; automatic restoration is optional and deferred. Document whether a new server run is needed in the first accepted slice.

TCP is a byte stream, not a packet-per-message service. Keep incomplete bytes until newline, split several lines from one read, decode complete bounded messages, and validate shape/types before reading fields. Reject excessive lines, invalid UTF-8, impossible IDs, forged outcomes and stale-stage actions. Identity comes from the server's connection slot, not a player ID supplied by a client.

A session can start as plain fields: phase, map ID, player states, puzzle flags, per-player discovered clues, ready set, optional deadline and final activation timestamps. Draw allowed transitions before implementing. Use `time.monotonic()` for elapsed deadlines. Do not persist sessions or build an event database.

## Proposed puzzle specifications

All additions below require approval. Exact compact layouts/coordinates must be drawn and reachability-checked before writing their rules. Preserve the current two-room layout for Map 1 where possible; its exact current spawn is read from each map's `@`, not guessed from documentation.

| Map | Player perspectives and objects | Dependency and intended solution | Change, recovery and completion |
|---|---|---|---|
| 1 Records Wing | P1 Gallery paintings: Eye 6, Lantern 2, Key 4. P2 Archive ordering document and cabinet | P1 describes numbers; P2 applies light, passage, watcher => `246` | Server opens cabinet and exposes the next permitted clue; wrong answer leaves it retryable |
| 1 mathematical hatch | P1 equations X=5, Y=9, Z=12 and hatch; P2 sequence Z-X-Y and instruction to keep final digits | Take final digit: 2,5,9 => `259`, not `1259` | Hatch reveals exit authorization. Both use their exit mechanisms; server transitions both only when conditions hold |
| 2 Observation Wing | P1 Exhibition statues, orientations, uniquely cracked floor, hidden panel. P2 Maintenance blueprint | Blueprint identifies north-facing statue nearest the marked cracked floor; ensure exactly one candidate | Correct statue opens an actual passage to receiver clue. Wrong statue gives feedback without permanent loss |
| 2 calibration | P1 receiver behind passage; P2 three labeled dial controls | Proposed deterministic target: receiver clue specifies A=2, B=A+1, C=B+1 on dials 0-4, so 2,3,4. Have students paper-test before accepting these new values | P2 adjusts/tests; feedback visible appropriately to P1; both exits activate only after calibration. No blind random search required |
| 3 Containment | P1 Lab maintenance instructions and valve labels. P2 Security split maintenance record and ventilation controls | Proposed logical clue pair: one document excludes red/blue labels; the other identifies the remaining valve's function. Draw labels so only one candidate survives | Correct valve enables authorization. A designated incorrect valve triggers the single gas incident; other wrong choices give safe feedback |
| 3 gas emergency | P1 emergency instruction card accessible from spawn; P2 ventilation panel accessible even during incident | Proposed fixed short sequence, e.g. INTAKE -> FILTER -> EXHAUST, explicitly given by P1's card | Approximately 180 s server deadline. Correct sequence stops gas. Wrong inputs retry. Expiry resets current level/checkpoint and both views; all required clues remain accessible |
| 3 final release | P1 authorization; P2 mechanical safety release; separate final controls | Both prerequisites, no active gas, both activations within approximately 8 s | Server records monotonic activations. Missed window clears attempts with feedback and permits retry; only verified success sends victory to both |

Calibration values, valve clues and ventilation sequence are proposed refinements, not facts from the master or existing code. Paper-test both-player information dependence and remove accidental shortcuts before approval. Use compact rooms, clearly marked objects and ordinary text input first. If optional inherited tasks are retained, define their purpose explicitly; they must not replace the required cooperation chain or impose a shared power resource.

## Function-sized implementation steps

For **every row**, first teach the concept, let the developer implement, review the attempt, run the check, and complete a feature note using `FEATURE_NOTE_TEMPLATE.md`. The function names in future modules are suggestions, not existing APIs. Each row is a small reviewed work item; it is not permission to generate its entire phase at once.

### Phase 1: understand and approve, Oct 26 and 28

| Step/objective | Files/functions or structures | Method and learning example | Test and acceptance |
|---|---|---|---|
| 1.1 Establish honest baseline | `REPOSITORY_AUDIT.md`, `PROJECT_MEMORY.md`, Git history, existing tests | Separate inherited behavior, current verified checks and missing requirements | Both developers identify the P2P/server gap and name unverified Linux/LAN tests |
| 1.2 Learn one movement function | `game/game_map.py`: `GameMap`, `Player`, `try_move` | Trace `(1,1)` plus right `(1,0)`; turn against wall without changing position | Developer predicts move result and runs `tests.test_movement`; no new movement engine |
| 1.3 Learn interaction and task flow | `RoomSession.interact`, `inspect`, `use_task`; module-level `client.game_screen.use`; `Team.apply` | Trace E from facing tile to local task success; explain why a peer declaration is not authority | Each student traces a valid interaction and a forged completion without reading a script |
| 1.4 Agree puzzle/data contracts | Map sketches, existing JSON, proposed puzzle table, protocol docs | Draw dependencies and fair split clues; specify state fields/messages and exact layouts | Paper walkthrough finds no dead end; both students explain the selected concurrency approach |
| 1.5 Approve next phase | Project memory decision entry, phase acceptance checklist | Record approved architecture, scope, identities, owner/reviewer and intended tests | Explicit Phase 2 approval. No runtime implementation in Phase 1 |

Phase deliverable: corrected plan, paper campaign, baseline review, teach-back evidence and approval record. Dependency: both developers have access to code and a supported Linux setup is identified. Risk: treating existing code as understood. Fallback: repeat traces, reduce art/UI ambition, and raise schedule pressure early. Documentation: reviewed code guide plus actual phase report, not the imported Week 1 narrative.

### Phase 2: referee first, Nov 4 / 9 / 11

| Step/objective | Files/functions or structures | Method and learning example | Test and acceptance |
|---|---|---|---|
| 2.1 Parse one message | Proposed `shared/protocol.py`: `encode_message`, `decode_message` | Dict -> JSON bytes/newline; bytes -> validated dict. Show `{}` versus valid `move` | Unit cases for non-object JSON, missing/type-invalid fields, invalid UTF-8 and lengths; invalid input cannot reach state mutation |
| 2.2 Frame the TCP stream | Proposed connection buffer in `server/main.py`, test module for protocol | Append bytes, extract complete lines, retain remainder | Split a line across reads, combine two lines, oversize line and EOF tests; no dropped/merged actions |
| 2.3 Start one server and assign two slots | Proposed `accept_client`, connection-slot table, `welcome` | Bind/listen/accept; server binds identity to socket; initial slice may be headless | Two real localhost clients get distinct IDs; third gets clear rejection; bind conflict fails clearly |
| 2.4 Ready and disconnect lifecycle | Proposed `ready` handling, `end_session`, close path | Both ready before start; disconnect ends session and releases sockets | Start cannot occur early; disconnect before/after ready and Ctrl+C are controlled |
| 2.5 Create authoritative state and views | Proposed `server/session.py`: session fields, `view_for(player_id)` | Load private full content only on server; construct permitted room dict | Assert recursively no answer key or partner-only clue in each view; clients no longer call full answer-bearing level load |
| 2.6 Authoritative movement | Proposed `apply_action`; reuse `try_move` on server | Client sends direction only; server checks current session and collision | Wall, edge, unknown direction, stale stage and forged coordinate tests. Both clients render approved results |
| 2.7 Authoritative inspect/interact | Reuse/refine `RoomSession`; adapt `handle_key`, `screens` to server results | Server chooses faced object from its own position, returns permitted text/art reference | No remote object/partner-room action possible; duplicate inspection/container behavior remains correct |
| 2.8 Validate one puzzle answer | Proposed `submit_answer`; `client/tasks.py` becomes input/display for code | Keep answer comparison on server. First use cabinet `246` | Wrong answer retries; correct one changes exactly one puzzle state; clients cannot send trusted `task` completion claims |
| 2.9 Restore mathematical hatch | Map 1 JSON/art, rule handler, `test_tasks` expectations | Explain `% 10` or equivalent final-digit operation as a small worked example; use `259` and reveal authorization | `259` accepted, `1259` rejected, repeated submissions harmless, next clue appears only after prerequisites |
| 2.10 Finish Map 1 and transition rule | Proposed `level_complete`, `advance_level`; two exit states | Both exits plus required puzzles; server transitions both together | One player alone cannot transition; duplicate exit commands harmless; Map 2 view initializes consistently |
| 2.11 Keep updates alive in every screen | `screens`, `menu.show_help`, `tasks`, `run_game`, receive polling helper if needed | Poll input with timeout and apply server updates during modal views; keep curses on main UI thread | Live test disconnect/partner updates during inspection and input; automated state checks plus Linux UI evidence |

Suggested check-in split: Nov 4 demonstrates 2.1-2.4, Nov 9 demonstrates 2.5-2.8, Nov 11 demonstrates 2.9-2.11 and Map 1 from start to finish with two clients. Do not claim 50% merely from inheriting a game that draws a map.

Phase dependency: approved Phase 1 contracts and a Linux terminal for actual UI checks. Deliverable: reliable, explainable server/client Map 1 slice, targeted protocol/state/network tests and private-view assertions. Main risks: partial I/O, authority split, modal update delay and learning pace. Fallback: simplify content/animation and request a scope revision if Map 1 misses the gate. Documentation: one note per accepted step, updated actual architecture/protocol and Linux start commands. A user-approved Linux install is separate if the environment needs one.

### Phase 3: one puzzle at a time, Nov 16 and 18

| Step/objective | Files/functions or structures | Method and learning example | Test and acceptance |
|---|---|---|---|
| 3.1 Add two compact Observation rooms | New `assets/maps/observation_wing/`, existing loader and content tests | Draw spawn, route, statues, cracked tile, hidden destination; reuse ASCII/JSON | Both maps load, ordinary paths/objects reachable, exactly one intended statue fits clues |
| 3.2 Reveal hidden passage | Server puzzle flag, small tile-update rule, per-player view | `identify_statue` changes closed tile to floor and reveals permitted clue | Wrong choice leaves state safe; right choice changes collision/view; repeat action is harmless |
| 3.3 Calibrate and complete Map 2 | Server `set_dial`, `test_calibration`, client simple dial input | Three bounded integers and a deterministic target; feedback split between players | Out-of-range/early actions rejected; solved state opens exits; both clients transition together |
| 3.4 Add Containment rooms and valve logic | New `assets/maps/containment_sector/`, `choose_valve` | Plain label/clue elimination, one hazardous wrong valve, safe retry paths | Clues select unique valve; safety controls/instructions reachable before and during gas |
| 3.5 Add the gas deadline | Server `trigger_gas`, `tick(now)`; client indicator | Store `deadline = monotonic_now + duration`; inject `now` into rule checks for tests | Timer starts once, advances with no player input, both clients warned; controlled time checks avoid three-minute sleeps |
| 3.6 Resolve or reset emergency | `ventilate`, `reset_checkpoint`, per-player state reset | Fixed clue-derived sequence; correct input clears deadline, expiry resets documented current level/checkpoint | Wrong sequence retries, just-before/at-deadline behavior defined, reset clears stale flags and permits completion |
| 3.7 Add final synchronized release | `activate_exit(player, now)`, authorization/safety flags | Record both activations after prerequisites; compare timestamps to proposed eight-second window | Exact boundary, missed window/retry, no-gas precondition, duplicate actions and reversed action order tested |
| 3.8 Send final victory | Server terminal state; reuse `show_win`, client event handling | Only server victory message ends campaign for both | Map 1/2 exits cannot produce final win; final conditions produce one consistent victory |

Suggested split: Nov 16 demonstrates complete Map 2, Nov 18 demonstrates Containment, recovery and final win. This is the tightest phase. Pair on timer/recovery and defer decorative work. Dependency: authoritative Map 1 gate. Deliverables: small complete campaign and deterministic rule tests. Risks: ambiguous clues, stale reset state and timer/UI disagreement. Fallback: request explicit scope/deadline revision; preserve reliable networking, meaningful cooperation and learning. Documentation: annotate every new content field/function, state-transition diagrams, timer boundary examples, and actual playtest notes.

### Phase 4: prove and explain, Nov 23 and 25

| Step/objective | Files/functions or structures | Method and learning example | Test and acceptance |
|---|---|---|---|
| 4.1 Freeze and regress | Targeted tests then full suite; actual `test_report` | Re-run original regressions adapted to approved behavior, new state/network cases | All required checks pass; distinguish product faults from environment limitations |
| 4.2 Two Linux computers on LAN | Actual server/client commands and runbook | Computer A server + P1, B P2; use LAN IP; repeat with no internet service | Full campaign, third join, wrong IP, bind conflict, mid-puzzle disconnect; record OS/Python/terminal and actual results |
| 4.3 Terminal/error acceptance | UI screens, shutdown paths, failure records | Resize, <80x24, no-color readability, wrong answers, key mashing, Ctrl+C | No crash/data corruption; clear feedback; terminal restored; important timer updates remain visible in modal screens |
| 4.4 Fairness and duration | Paper puzzle specs, actual playtest log | Two students test without answer sheet; time and stuck points recorded | Progression possible without guessing; both useful; target 25-35 min measured rather than asserted |
| 4.5 Student code defense | Feature notes and code guide | Each traces key -> message -> validation -> state -> response and a timer boundary; swap presenters | Both can locate and explain every feature, make a small safe change, and explain tests |
| 4.6 Clean package and rehearsal | README, demo guide, credits, zip, latest report | Package source/docs/assets, extract to clean folder and run on supported Linux | Tested package and honest attribution; no completion claim before actual check; no new features |

Nov 23 evidence: frozen campaign, regression output, LAN results and remaining issue list. Nov 25 evidence: verified fixes, tested clean package and both developers' oral demonstration. Dependencies: functioning campaign and two Linux machines. Risks: network isolation, machine availability, packaging omissions, incomplete student understanding. Fallback: authorized hotspot/local rehearsal and clearly marked recorded backup, while disclosing any unmet acceptance condition. Documentation: actual tests, manual, runbook, feature notes, OS explanations, report and contribution review.

## How weekly reports demonstrate real progress

Use `PROGRESS_REPORT_TEMPLATE.md`, one report per actual check-in. Keep the inherited reports as labeled reference drafts. Link real commits only after developers choose to create them; this review does not commit or rewrite history.

Record separately: inherited functionality; concepts studied; functions the students actually implemented/changed this period; checks executed and failures; reviewer feedback; both students' teach-back evidence; next function. Small learning and debugging steps are valid progress when described honestly. Do not split one inherited implementation into fictitious historical weeks.

If the instructor requires a measured percentage, first agree on acceptance items and their weights with the team/instructor. Then report accepted work against that denominator and show the calculation. The 20/50/70/100 milestones are targets, not automatically achieved scores. The original three-map scope cannot be called 100% complete if only Map 1 is accepted without a documented scope revision.

## Classroom demonstration and OS explanations

Proposed final runbook must contain actual accepted commands, not today's nonexistent server command. Expected shape after implementation is `python3 -m server.main` plus `python3 main.py` for each client. On Linux use `hostname -I` or the local network settings to find Computer A's LAN IP, choose the address shared with B, and document port 5050. Rehearse localhost with the server and two clients in separate terminals before separate machines. If the classroom network blocks peer traffic, arrange an approved hotspot in advance.

| OS concept | Beginner definition | Current evidence | Intended demonstration after accepted steps |
|---|---|---|---|
| Process | A running program with its own memory | Two peer programs can run separately | One server process plus two client processes; identify each terminal |
| TCP socket | An endpoint for an ordered byte stream between programs | Real `NetLink` sockets and line framing | Connect by IP/port, split/coalesce messages, observe a controlled disconnect |
| Thread and queue | Another execution path; safe handoff container | Background reader passes messages to UI through `queue.Queue` | Explain why receiving need not freeze typing; distinguish it from another process |
| Concurrency | Several activities can make progress during overlapping time | Current peer input and reader activity | Both clients submit actions while server handles readiness and deadlines |
| Shared state and synchronization | Agreeing who may change one common state and in what order | Current separate peer copies trust messages, not an authoritative solution | One server loop owns state; test both action orders and show invariants hold |
| Event handling | Reacting to an input or notification | E/inspect/task/message dispatch | Action -> rule -> approved update -> each permitted view |
| Timers/scheduling | Checking when a deadline arrives while other work continues | Client `time.time()` animation, no required server timer | Monotonic gas and exit windows continue without client input, including modal views |
| Fault handling | Rejecting bad input and recovering from failures | Map checks and socket errors, but schema gap | Send invalid action, disconnect, retry/reset without corrupt state |

For an oral explanation, define the term, point at the real function, trace one gameplay example, and state its limit. Do not claim this project implements a kernel scheduler, shared-memory IPC between computers, distributed consensus or automatic reconnection.

## Review of this proposal

The order avoids building campaign logic on an untrusted peer model. Existing pure rules/content/drawing remain useful. All mandatory systems have a planned step; optional enhancements stay deferred. Learning notes, two-person review, tests and actual environment evidence are completion criteria throughout. The largest unresolved risk is workload inside the confirmed calendar. No plan can guarantee three-map completion without knowing the developers' pace and verifying the early networking gate.

Next approval should cover **Phase 1 only**, or a specifically named smaller learning session. Approval for Phase 2 must name the concurrency model and authority/view contract. Proposed scope cuts, installs and major plan revisions need their own explicit decision.
