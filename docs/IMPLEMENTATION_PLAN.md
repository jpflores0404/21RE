# 2R1E — Two Rooms, One Exit: Implementation Plan

**Project:** 2R1E (Two Rooms, One Exit), a two-player co-op escape game played in the terminal
**Gameplay in one line:** like the tasks in *Among Us*. Each player is locked in a different room with a task list. Some tasks need clues or items from the partner's room. When the shared **TOTAL TASKS** bar reaches 8/8 the exit opens, and both players must walk out to win.
**Language / Stack:** Python 3.10+ standard library only (`curses`, `socket`, `threading`, `json`, `unittest`)
**Final submission:** `Section_2R1E.zip` (complete source code + documentation), due before the presentation date
**Team size:** 4

---

## 1. Grading Checkpoints

| Phase | Dev Week | Check-in dates | Target progress | Theme |
|---|---|---|---|---|
| Phase 1 | Week 1 | Mon Oct 26, Wed Oct 28 | **20%** | Pre-production: map planning, task and puzzle planning, architecture. No implementation yet. |
| Phase 2 | Week 2 | Wed Nov 4, Mon Nov 9, Wed Nov 11 | **50%** | Core build: map loading, movement, collision, interaction, dialogue. Movement testing and gameplay polish. |
| Phase 3 | Week 3 | Mon Nov 16, Wed Nov 18 | **70%** | Tasks and multiplayer: task system, mini-games, cross-room puzzles, LAN two-player, clue sharing, exit and win. |
| Phase 4 | Week 4 | Mon Nov 23, Wed Nov 25 | **100%** | Chat, integration, full playtests, bug fixing, documentation, packaging the zip, presentation prep. |

**Rule of thumb:** each item below is due *at the start* of the class on the listed date, so it can be shown during the check-in. Work that is not shown at a check-in does not count toward that week's percentage.

---

## 2. Team Roles

Fill in names. Every member owns one area but helps in others during testing weeks.

| Code | Role | Owns | Folder(s) / files |
|---|---|---|---|
| **M1** | Project Lead / Game Systems | Game rules: map loader, movement, interaction, task rules, team progress, merging, final zip | `game/`, `main.py` |
| **M2** | Networking Engineer | LAN host/join, message format, sending task progress, clues and chat, disconnects | `client/network.py`, `client/lobby.py` |
| **M3** | Level & Puzzle Designer | Map layouts, object and task station placement, cross-room puzzles, clue text, difficulty tuning | `assets/maps/`, `docs/design/` puzzle docs |
| **M4** | UI/UX, Art & QA/Docs | Curses screens, menus, dialogue box, mini-games, close-up art, tests, test plan, user manual | `client/` (except network/lobby), `assets/art/`, `tests/`, `docs/` |

| Member | Name | Role |
|---|---|---|
| M1 | ______________________ | Project Lead / Game Systems |
| M2 | ______________________ | Networking Engineer |
| M3 | ______________________ | Level & Puzzle Designer |
| M4 | ______________________ | UI/UX, Art & QA/Docs |

### 2.1 Target folder structure

```
21re/
  main.py                 start the game
  game/                   rules only (no curses, no socket)
    tiles.py              tile legend
    game_map.py           map loader, Player, try_move()
    level.py              loads a level: rooms, objects, tasks
    session.py            one player's room: E (use), F (inspect), journal
    team.py               shared task progress, escapes, win check
  client/                 everything the player sees
    app.py, menu.py       app shell and main menu
    game_screen.py        map, side panel, dialogue, key handling
    tasks.py              mini-games
    screens.py            close-ups, journal, win screen
    dialogue.py, ui.py    dialogue paging, drawing helpers
    network.py, lobby.py  LAN link, host/join screens
  assets/maps/records_wing/   map .txt files, *_objects.json, level.json
  assets/art/             ASCII close-up pictures
  tests/                  unittest tests
  docs/                   plan, reports, design, manual, test report
```

---

## 3. Phase 1 — Pre-Production (Dev Week 1, target 20%)

**Check-ins:** Mon Oct 26 and Wed Oct 28
**Goal:** Everyone agrees on *what* is being built before any code is written. By Oct 28 the team should be able to explain both rooms, all 8 tasks, and how the two players help each other, using text documents only.

> **No game implementation this week.** Allowed: repository setup, an empty folder structure, and design documents. Not allowed yet: movement code, screens, or networking code.

### 3.1 Tasks and due dates

| # | Task | Owner | Due | Output |
|---|---|---|---|---|
| 1.1 | Game concept: premise, task-list idea, TOTAL TASKS bar, win condition, controls draft | M1 | Oct 26 | `docs/design/concept.md` |
| 1.2 | Set up repository, branch rules, empty folder structure (2.1), `README.md` skeleton | M1 | Oct 26 | Empty `game/`, `client/`, `assets/`, `tests/`, `docs/` |
| 1.3 | Sketch Map 1 *The Records Wing*: Gallery Room (P1) and Archive Room (P2) in text | M3 | Oct 26 | `docs/design/map1_sketch.txt` |
| 1.4 | Tile legend (wall, floor, carpet, door, exit, furniture, task station) and which tiles are walkable | M3 + M1 | Oct 26 | `docs/design/tiles.md` |
| 1.5 | Research curses on macOS/Linux; confirm the 80 x 24 layout; list limitations | M4 | Oct 26 | `docs/design/ui_research.md` |
| 1.6 | Research LAN options (TCP, host/join, port, threads); choose the networking model | M2 | Oct 26 | `docs/design/network_research.md` |
| 1.7 | Task and puzzle flow for Map 1: 4 tasks per player, cross-room puzzles, answers (see 3.3) | M3 | Oct 28 | `docs/design/puzzle_flow_map1.md` |
| 1.8 | Map 2 concept (stretch goal; built only if Map 1 is finished early) | M3 | Oct 28 | `docs/design/map2_concept.md` |
| 1.9 | Wireframes: menu, game screen (map, side panel, dialogue, key bar), close-up, journal, each mini-game | M4 | Oct 28 | `docs/design/wireframes.md` |
| 1.10 | Network message draft: task, clue, chat, escaped, bye | M2 | Oct 28 | `docs/design/protocol.md` |
| 1.11 | Architecture diagram: `game/` (rules) vs `client/` (screens) vs `client/network.py` (link) | M1 | Oct 28 | `docs/design/architecture.md` |
| 1.12 | Object data format (JSON fields: id, name, at, kind, text, clue, item, task, needs) | M1 + M3 | Oct 28 | `docs/design/objects_format.md` |
| 1.13 | Test plan outline: what is tested each week and how | M4 | Oct 28 | `docs/test_plan.md` |

### 3.2 Guide: Map planning

1. Draw each room on an **at most 50 x 15** character grid so it fits beside the side panel in an 80 x 24 terminal.
2. Use one character per tile. Start with walls (`#`), then floors (`.`), then furniture and task stations (`%`).
3. Mark exactly **one spawn point** (`@`) per room.
4. Every floor tile must be reachable from the spawn. Trace paths with a pencil to check.
5. Every object and task station must be placed **against a walkable tile**, so the player can stand next to it and face it.
6. Place the exit (`=`) at the same spot on the bottom wall of both rooms. Both exits stay sealed until the team finishes every task. This sells the *Two Rooms, One Exit* idea.
7. Spread task stations around the room so players have to walk, like in *Among Us*. Add 1–2 "red herring" objects (things to look at that are not clues) so the room does not feel empty.
8. Label each object on the sketch with a short id (`painting_key`, `g_wires`, `a_download`). These ids become the JSON ids later.

### 3.3 Guide: Task and puzzle planning

Each player gets **4 tasks** (8 in total). Write every task as: **station → mini-game → what it needs → who has the info**.

Mix three kinds of task so both players stay busy and must talk:

| Kind | Example | Why |
|---|---|---|
| Solo task | Fix Wiring, Reset Breakers | Easy start; each player can make progress alone |
| Needs an item | Swipe Card needs the Staff Keycard hidden under the pedestal | Rewards exploring the room |
| Needs the partner | Upload Data only works after the partner finishes Download Data; keypad codes need clues from both rooms | Forces teamwork |

Planned mini-games (one simple function each, written in Phase 3):

| Mini-game | How it plays |
|---|---|
| Wires | Pick a wire on the left, connect it to the same colour on the right |
| Swipe | Press SPACE while the moving card is inside the green zone |
| Download / Upload | Stand at the terminal while the progress bar fills |
| Keypad | Type a code; wrong codes show ACCESS DENIED |
| Breakers | Press the switches 1–10 in order on a shuffled board |

Task and puzzle checklist:

- [ ] Each player has exactly 4 tasks; the TOTAL TASKS bar counts all 8.
- [ ] At least one task in each room cannot be finished with that room's clues alone.
- [ ] Every clue is found by a **specific action** (E to use/open, F to inspect).
- [ ] Every code has a written answer and a written hint.
- [ ] No dead ends: items are never used up and tasks can be retried.
- [ ] Target play time: 10–15 minutes for first-time players.

### 3.4 Definition of done (20%)

- [ ] Concept, map sketches, tile legend, task flow, wireframes, message draft, and architecture documents are in the repo.
- [ ] Every member can explain all 8 tasks and both cross-room codes.
- [ ] Task board (Trello, GitHub Projects, or a shared sheet) lists all Phase 2 tasks.

---

## 4. Phase 2 — Core Build, Movement Testing & Gameplay Polish (Dev Week 2, target 50%)

**Check-ins:** Wed Nov 4, Mon Nov 9, Wed Nov 11
**Goal:** A single-player build where one player can walk around each room of Map 1, bump into walls, face objects, read dialogue, inspect close-ups, and keep a journal. Task stations exist on the map but their mini-games come in Phase 3. Networking is prototyped separately.

### 4.1 Tasks and due dates

| # | Task | Owner | Due | Output |
|---|---|---|---|---|
| 2.1 | Tile legend in code: character → name, walkable, colour, bump message | M1 | Nov 4 | `game/tiles.py` |
| 2.2 | Map loader and checks: reject unknown characters, missing/duplicate spawn | M1 | Nov 4 | `game/game_map.py` |
| 2.3 | Player movement with collision and facing direction (`try_move`) | M1 | Nov 4 | `game/game_map.py` |
| 2.4 | Convert the Map 1 sketches into `gallery.txt` and `archive.txt` | M3 | Nov 4 | `assets/maps/records_wing/` |
| 2.5 | Curses app shell: main menu, start game, back to menu, quit, terminal restored on crash | M4 | Nov 4 | `main.py`, `client/app.py`, `client/menu.py` |
| 2.6 | Draw the map, player sprite, and status bar | M4 | Nov 4 | `client/game_screen.py`, `client/ui.py` |
| 2.7 | Socket prototype: host and join on the same LAN, send/receive JSON lines | M2 | Nov 4 | `client/network.py` (not used by the game yet) |
| 2.8 | **Movement tests:** walls block, objects block, spawn found, every floor reachable, rooms closed in | M4 + M1 | Nov 9 | `tests/test_movement.py` |
| 2.9 | Level and object loader with checks (each object sits on a solid tile) | M1 | Nov 9 | `game/level.py` |
| 2.10 | Write all Map 1 object files: text, inspect lines, clues, items, task stations | M3 | Nov 9 | `gallery_objects.json`, `archive_objects.json` |
| 2.11 | Room session: use (E), inspect (F), containers that open once, journal of clues and items | M1 | Nov 9 | `game/session.py` |
| 2.12 | Dialogue box with paging, side panel, hint line, key bar | M4 | Nov 9 | `client/dialogue.py`, `client/game_screen.py` |
| 2.13 | Message format and network tests (send, receive, disconnect gives `bye`) | M2 | Nov 9 | `tests/test_network.py` |
| 2.14 | ASCII close-up art: paintings, notes, document, keypad, exit panel | M4 + M3 | Nov 11 | `assets/art/*.txt` |
| 2.15 | Close-up (inspect) screen and journal screen | M4 | Nov 11 | `client/screens.py` |
| 2.16 | **Interaction tests:** use vs inspect, containers, clues saved once, 2-tile objects | M4 | Nov 11 | `tests/test_interaction.py` |
| 2.17 | **Gameplay polish pass:** highlight faced object, bump messages, instant Esc, readable colours, room intro text | M1 + M4 | Nov 11 | Polished single-player build |
| 2.18 | Playtest both rooms (Tab switches rooms for testing); log every issue | M3 | Nov 11 | Notes in the Week 2 progress report |
| 2.19 | Connection layer: `LocalLink` (one computer) and `NetLink` (LAN, peer to peer) with the same `send()` / `receive()` | M2 | Nov 11 | `client/network.py` (not yet wired into the game) |

### 4.2 Guide: Movement implementation and testing

1. Keep **all rules in `game/`** and **all drawing in `client/`**. `try_move()` must not import curses, so it can be unit tested.
2. Movement flow: key → direction → `try_move(game_map, player, direction)` → returns `(moved, message)` → screen redraws.
3. If the target tile is not walkable, **do not move the player**, only turn them to face it. This is how players "select" objects and task stations.
4. Movement test cases (one `unittest` test each):
   - Moving into a wall does not change position.
   - Moving onto floor works and counts a step.
   - Moving into furniture turns the player to face it.
   - The player can never leave the map.
   - Each map has exactly one spawn; unknown characters are rejected.
   - Each room is fully closed in by walls.
   - Every floor tile is reachable from the spawn (flood fill).
   - Every object has a walkable tile next to it.
5. Run `python3 -m unittest -v` before every push.

### 4.3 Guide: Gameplay polish checklist

- [ ] The faced tile is highlighted and its name appears in the hint line.
- [ ] Every object has at least one line of text; nothing says "TODO".
- [ ] Bump messages are short and varied ("A solid wall.", "The bookshelf won't budge.").
- [ ] Dialogue never runs past the box; long text pages correctly.
- [ ] Colours are readable on dark and light terminal themes.
- [ ] The game works in an 80 x 24 terminal and shows a warning if the window is too small.
- [ ] Ctrl+C, Q and Esc never leave the terminal broken.

### 4.4 Definition of done (50%)

- [ ] `python3 main.py` opens the menu and starts Map 1.
- [ ] Both rooms are fully walkable and every object can be used and inspected.
- [ ] The journal records clues and items.
- [ ] Movement, interaction and network tests pass.
- [ ] Two terminals can exchange messages with the network prototype.

---

## 5. Phase 3 — Tasks & Multiplayer (Dev Week 3, target 70%)

**Check-ins:** Mon Nov 16, Wed Nov 18
**Goal:** Two players on two computers (or two terminals) can play Map 1 together: finish their tasks, share clues, fill the TOTAL TASKS bar, open the exit and win.

### 5.1 Tasks and due dates

| # | Task | Owner | Due | Output |
|---|---|---|---|---|
| 3.1 | Task system: `task` objects, `needs_item`, `needs_task`, shared team progress | M1 | Nov 16 | `game/team.py`, `use_task()` in `game/session.py` |
| 3.2 | Mini-games: wires, swipe, download, upload, keypad, breakers | M4 | Nov 16 | `client/tasks.py` |
| 3.3 | Map 1 answers, hints and lock messages in JSON | M3 | Nov 16 | `*_objects.json` |
| 3.4 | Host / Join screens: host shows its IP and waits; join asks for the IP | M2 + M4 | Nov 16 | `client/lobby.py`, `client/menu.py` |
| 3.5 | Connect the network to the game: read messages every 0.1 s without freezing | M2 + M1 | Nov 16 | `handle_messages()` in `client/game_screen.py` |
| 3.6 | **Task tests:** task starts, needs item, needs partner task, cannot redo, codes match clues | M4 | Nov 16 | `tests/test_tasks.py` |
| 3.7 | Share a clue from the journal; partner chat (T key) | M2 + M4 | Nov 18 | Journal → Enter; chat line on the game screen |
| 3.8 | Shared TOTAL TASKS bar, exit opens at 8/8, win when both players escape | M1 + M2 | Nov 18 | `Team.has_won()` |
| 3.9 | Disconnect handling: "Your partner left the game", back to menu without crashing | M2 | Nov 18 | `bye` message in `client/network.py` |
| 3.10 | Map 2 build (stretch goal; only if 3.1–3.9 are done) | M3 + M4 | Nov 18 | `assets/maps/<map2_id>/` |
| 3.11 | Win screen with escape time | M4 | Nov 18 | `show_win()` in `client/screens.py` |
| 3.12 | Two-player playtest of Map 1; record solve time and where players got stuck | M3 + all | Nov 18 | Notes in the Week 3 progress report |

### 5.2 Guide: Task implementation

1. Put task data in JSON, not code. Example:
   ```json
   { "id": "g_upload", "name": "Gallery Terminal", "at": [[37, 6]],
     "kind": "task", "task": "upload", "title": "Upload Data",
     "needs_task": "a_download", "seconds": 6,
     "locked_text": "NO DATA TO UPLOAD. Your partner must download the records in the Archive first." }
   ```
2. `game/session.py` decides **whether** a task can start (already done? item missing? partner task missing?). `client/tasks.py` only plays the mini-game and returns True (finished) or False (left early).
3. When a mini-game returns True: mark the task done in `Team`, then `send({"type": "task", "id": ...})` so the partner's bar updates too.
4. Task stations are drawn yellow while open and green when done, so players can see what is left.
5. Keypad answers live in the JSON. Test that each answer matches its clues (for example, the cabinet code is built from the painting numbers in the archive's order).
6. Test every task: starts when ready, refuses when locked, cannot be done twice.

### 5.3 Guide: Networking implementation

1. **Model:** peer to peer. The host's game listens on port **5050**; the joiner connects to the host's IP. After that both sides are equal.
2. **Each computer runs its own room.** Only shared things go over the network: finished tasks, shared clues, chat and escapes. Player positions are never sent, because the partner cannot see the other room.
3. **Transport:** TCP, one JSON object per line (`\n` at the end). A background thread reads the socket into a `queue.Queue`; the game loop checks the queue every 0.1 s (`stdscr.timeout(100)`), so the screen never freezes.
4. **One-computer mode** uses `LocalLink`, which has the same `send()` / `receive()` as `NetLink`. The game loop does not need to know which one it has.
5. Messages:

   | Message | Example | When |
   |---|---|---|
   | `task` | `{"type":"task","id":"a_download"}` | A player finishes a task |
   | `clue` | `{"type":"clue","to":2,"text":"..."}` | Shared from the journal |
   | `chat` | `{"type":"chat","from":1,"text":"..."}` | T key |
   | `escaped` | `{"type":"escaped","player":1}` | A player walks out of the open exit |
   | `bye` | `{"type":"bye"}` | Made locally when the connection closes |

6. Test on one machine first (`127.0.0.1`), then on two machines on the same Wi-Fi. Make sure the firewall allows port 5050.
7. Show a clear message for "port already in use" and "could not connect", never a crash.

### 5.4 Definition of done (70%)

- [ ] Two players can host/join and play Map 1 together from start to win.
- [ ] All 8 tasks and their mini-games work; locked tasks explain what is missing.
- [ ] Clue sharing works, and each player only sees their own room's clues unless they are shared.
- [ ] The TOTAL TASKS bar is the same on both screens.
- [ ] Task tests pass.

---

## 6. Phase 4 — Integration, Testing, Documentation & Submission (Dev Week 4, target 100%)

**Check-ins:** Mon Nov 23, Wed Nov 25
**Goal:** A finished, stable, documented game zipped as `Section_2R1E.zip` and ready to present.

### 6.1 Tasks and due dates

| # | Task | Owner | Due | Output |
|---|---|---|---|---|
| 4.1 | Finish any Phase 3 leftovers (for example chat), or Map 2 tasks if Map 2 was started | M2 / M3 | Nov 23 | Complete feature set |
| 4.2 | Feature freeze: merge all branches, fix merge conflicts, `main` becomes the release branch | M1 | Nov 23 | Stable `main` |
| 4.3 | Full regression test: all unit tests + manual test plan on macOS and Linux | M4 | Nov 23 | `docs/test_report.md` |
| 4.4 | Network stress check: partner quits mid-game, wrong IP, port in use, slow Wi-Fi | M2 | Nov 23 | Fixes + notes in the test report |
| 4.5 | Difficulty tuning from playtest notes: hint text, clue wording, lock messages | M3 | Nov 23 | Updated object files |
| 4.6 | Bug fix sprint: all open bugs closed | All | Nov 25 | Bug list with status |
| 4.7 | Documentation (see 6.3) | M4 (lead), all contribute | Nov 25 | `README.md`, `docs/` |
| 4.8 | Presentation slides and live demo script (who says what, backup video) | M1 + M3 | Nov 25 | Demo steps in the Week 4 report |
| 4.9 | Package `Section_2R1E.zip`, unzip in a clean folder, and run it | M1 | Nov 25 | Final zip |
| 4.10 | Final two-player playthrough, recorded as a backup | M2 + M3 | Nov 25 | Demo video |

### 6.2 Guide: Final testing

1. Unzip the submission into a fresh folder and run `python3 main.py`. If it fails there, it fails for the grader.
2. Run `python3 -m unittest -v`; all tests must pass.
3. Play from start to win as two players on two computers, without any testing shortcut.
4. Try to break it: small window, key mashing, quit mid-game, wrong IP, very long codes and chat messages.
5. Log every bug with: steps, expected result, actual result, severity, owner, status.

### 6.3 Guide: Documentation contents

| Document | Contents | Owner |
|---|---|---|
| `README.md` | What the game is, requirements, how to run, how to run tests, controls, folder guide | M4 |
| `docs/user_manual.md` | Story, how to host/join, controls, the task list, tips (answers marked as spoilers) | M4 |
| `docs/design/` | Concept, maps, tiles, task flow, wireframes, protocol, architecture, object format | M1 + M3 |
| `docs/test_plan.md` + `docs/test_report.md` | Test cases and results | M4 |
| `docs/progress_reports/` | All four weekly progress reports | M1 |
| `docs/credits.md` | Team members, roles and contributions | M1 |

### 6.4 Guide: Packaging the zip

```bash
# from the folder that contains 21re/
find 21re -name "__pycache__" -type d -prune -exec rm -rf {} +
zip -r Section_2R1E.zip 21re -x "*.DS_Store" "*/.git/*"
```

Replace `Section` with your actual section (for example `BSCS3A_2R1E.zip`). Unzip it in a new folder and run the game once before submitting.

### 6.5 Definition of done (100%)

- [ ] Map 1 is fully playable by two players from start to win, on one computer and over LAN.
- [ ] Chat, clue sharing and disconnect handling work.
- [ ] All tests pass; no known crash bugs.
- [ ] All documents in 6.3 are complete.
- [ ] `Section_2R1E.zip` is tested in a clean folder and submitted before the presentation date.

---

## 7. Per-Member Summary of Deadlines

### M1 — Project Lead / Game Systems
| Due | Tasks |
|---|---|
| Oct 26 | 1.1 Concept, 1.2 Repo setup, 1.4 Tile legend (with M3) |
| Oct 28 | 1.11 Architecture, 1.12 Object data format (with M3) |
| Nov 4 | 2.1 Tiles, 2.2 Map loader, 2.3 Movement |
| Nov 9 | 2.8 Movement tests (with M4), 2.9 Level loader, 2.11 Room session |
| Nov 11 | 2.17 Gameplay polish (with M4) |
| Nov 16 | 3.1 Task system, 3.5 Network in the game loop (with M2) |
| Nov 18 | 3.8 TOTAL TASKS bar, exit and win (with M2) |
| Nov 23 | 4.2 Feature freeze and merge |
| Nov 25 | 4.6 Bug fixes, 4.8 Presentation (with M3), 4.9 Package zip |

### M2 — Networking Engineer
| Due | Tasks |
|---|---|
| Oct 26 | 1.6 Networking research |
| Oct 28 | 1.10 Message draft |
| Nov 4 | 2.7 Socket prototype |
| Nov 9 | 2.13 Message format and network tests |
| Nov 11 | 2.19 LocalLink / NetLink |
| Nov 16 | 3.4 Host/Join screens (with M4), 3.5 Network in the game loop (with M1) |
| Nov 18 | 3.7 Clue sharing and chat (with M4), 3.8 Shared progress (with M1), 3.9 Disconnects |
| Nov 23 | 4.1 Phase 3 leftovers, 4.4 Network stress check |
| Nov 25 | 4.6 Bug fixes, 4.10 Final playthrough (with M3) |

### M3 — Level & Puzzle Designer
| Due | Tasks |
|---|---|
| Oct 26 | 1.3 Map 1 sketches, 1.4 Tile legend (with M1) |
| Oct 28 | 1.7 Task and puzzle flow, 1.8 Map 2 concept, 1.12 Object format (with M1) |
| Nov 4 | 2.4 Map 1 text files |
| Nov 9 | 2.10 Map 1 object files |
| Nov 11 | 2.14 Close-up art (with M4), 2.18 Week 2 playtest |
| Nov 16 | 3.3 Answers, hints and lock messages |
| Nov 18 | 3.10 Map 2 (stretch, with M4), 3.12 Two-player playtest |
| Nov 23 | 4.1 Map 2 tasks (only if started), 4.5 Difficulty tuning |
| Nov 25 | 4.6 Bug fixes, 4.8 Presentation (with M1), 4.10 Final playthrough (with M2) |

### M4 — UI/UX, Art & QA/Docs
| Due | Tasks |
|---|---|
| Oct 26 | 1.5 Curses research |
| Oct 28 | 1.9 Wireframes, 1.13 Test plan outline |
| Nov 4 | 2.5 App shell and menu, 2.6 Map drawing |
| Nov 9 | 2.8 Movement tests (with M1), 2.12 Dialogue and side panel |
| Nov 11 | 2.14 Close-up art (with M3), 2.15 Close-up and journal screens, 2.16 Interaction tests, 2.17 Polish (with M1) |
| Nov 16 | 3.2 Mini-games, 3.4 Host/Join screens (with M2), 3.6 Task tests |
| Nov 18 | 3.7 Journal sharing and chat line (with M2), 3.10 Map 2 art (stretch, with M3), 3.11 Win screen |
| Nov 23 | 4.3 Regression testing |
| Nov 25 | 4.6 Bug fixes, 4.7 Documentation |

---

## 8. Team Working Rules

### 8.1 Git workflow
- `main` must always run. Never push broken code to `main`.
- One branch per task: `feature/2.3-movement`, `fix/dialogue-overflow`, `docs/user-manual`.
- Open a pull request; at least **one other member** reviews before merging.
- Commit messages say what changed: `Add wall collision to try_move`.
- Pull `main` every day before starting work.

### 8.2 Coding standards
- Python 3.10+, standard library only (the grader should not need `pip install`).
- **Keep the code simple:** plain functions and dictionaries first; a class only when it holds state (`GameMap`, `Player`, `RoomSession`, `Team`, the links). Short functions, clear names, no clever tricks.
- `game/` never imports `curses` or `socket`. `client/` never decides game rules.
- Level content (maps, objects, tasks, clues, answers) lives in `assets/`, not in Python code.
- Each new rule gets at least one test in `tests/`.
- Short docstring at the top of every module explaining what it is for.

### 8.3 Communication
- Short daily update in the group chat: *done yesterday / doing today / blocked by*.
- Team meeting the day **before** each check-in to fill in the progress report (`docs/PROGRESS_REPORT_TEMPLATE.md`) and rehearse the demo.
- If a task will miss its date, tell M1 at least **2 days early** so work can be moved.

---

## 9. Risks and Backup Plans

| Risk | Likelihood | Impact | Backup plan |
|---|---|---|---|
| Networking takes longer than planned | High | High | "Play on one computer" mode (Tab switches players) uses the same game loop, so the full task list can still be demoed on one machine. |
| School Wi-Fi blocks LAN connections | Medium | High | Demo on a phone hotspot, or two game windows on one laptop via `127.0.0.1`. Record a backup video. |
| Mini-games take longer than planned | Medium | Medium | Every mini-game is one small function; start with keypad and download (simplest), add the rest after. |
| Terminal too small or different on a grader's machine | Medium | Medium | Size check with a clear message; the 80 x 24 minimum is in the README. |
| Map 2 not finished | High | Low | Map 2 is a stretch goal. Map 1 must be complete and polished first; Map 2 is dropped if it would cut into Week 4 testing. |
| Member unavailable for a week | Low | High | Every task has a co-owner in section 7; documents and code reviews keep knowledge shared. |
| Merge conflicts close to the deadline | Medium | Medium | Feature freeze on Nov 23; small, frequent pull requests. |
