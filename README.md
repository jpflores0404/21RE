> **Current direction, October 28, 2026:** The professor requires C, with Windows-first testing and Linux acceptance. See [the C implementation plan](docs/C_IMPLEMENTATION_PLAN.md). The first standalone C movement demo is implemented; the full campaign port is future work. The Python run commands below describe the inherited prototype.

# 2R1E: Two Rooms, One Exit

## First C movement demo and Week 1 submission

The [C demo](src/movement_demo.c) supports WASD movement, terminal boundaries, resizing and clean exit. See the [build and run guide](docs/MOVEMENT_DEMO.md) and [Linux and Windows code explanation](docs/LINUX_WINDOWS_MOVEMENT_GUIDE.md). Windows checks passed; Linux compilation and terminal acceptance remain unverified. On Linux with a C compiler, use `sh scripts/run_movement_linux.sh --self-test` and then `sh scripts/run_movement_linux.sh`.

The [game design draft](docs/deliverables/2026-10-28_GAME_DESIGN_DRAFT.docx) contains map sketches and proposed puzzles. The [Word code guide](docs/deliverables/C_MOVEMENT_LINUX_WINDOWS_GUIDE.docx) explains the movement source. The [actual Week 1 report draft](docs/progress_reports/2026-10-28_submission_draft.md) separates implemented behavior from future work. These drafts still need developer review and missing identification fields before submission.

## Inherited Python prototype

> **Review status, October 23, 2026:** the code below is an inherited single-map, peer-to-peer variant. It does not yet meet the original authoritative-server, three-map master plan. John Francis Espiritu confirmed four grading phases ending November 25, and the team has two developers. Runtime code has not been changed by this review.

Start with [shared developer instructions](AGENTS.md) and [project memory](docs/PROJECT_MEMORY.md). Read the [repository audit](docs/REPOSITORY_AUDIT.md) for verified deviations, the [proposed learning roadmap](docs/LEARNING_ROADMAP.md) for small steps and confirmed check-ins, and the [beginner code guide](docs/CODE_GUIDE.md) to study existing functions. The [original master prompt](docs/MASTER_PROMPT.md) is preserved verbatim. Implementation phases still need explicit approval. Existing Week 1-4 reports are unverified imported drafts, not evidence of completed weekly work.

A two-player co-op escape room that runs in the terminal. Each player wakes up in
a different room. Like in **Among Us**, each player has a **task list**, and every
finished task fills the shared **TOTAL TASKS** bar. Some tasks need a clue or a
finished task from the partner's room, so the players must talk to each other.
When the bar is full, the exit opens and **both** players must walk out to win.

## Requirements

- macOS or Linux
- Python 3.10+ (standard library only, nothing to install)
- A terminal of at least **80 x 24** characters

## Run

```bash
python3 main.py              # play
python3 -m unittest -v       # run the tests
```

### Ways to play

| Menu option | How it works |
|---|---|
| Play on this computer | Both players share one keyboard. **Tab** switches between the Gallery and the Archive. |
| Host LAN game | You are Player 1 (Gallery). The screen shows your IP address for your partner. |
| Join LAN game | You are Player 2 (Archive). Type the host's IP address. Leave it empty to join a game hosted on this same computer. |

Both computers must be on the same Wi-Fi. The game uses TCP port **5050**.

## Controls

| Key | Action |
|---|---|
| W A S D / arrows | Move. Walk into something to face it. |
| E | Use what you face: read it, open it, start a task, go through the exit |
| F | Look closely. Details you read are saved as clues. |
| I | Journal: items and clues. Press **Enter** on a clue to send it to your partner. |
| T | Chat with your partner (LAN games) |
| Tab | Switch player (games on one computer) |
| H | How to play |
| Q / Esc | Close a box, leave a task, or go back to the menu |

## Tasks (Map 1: The Records Wing)

| Player 1: Gallery | Mini-game | Needs |
|---|---|---|
| Fix Wiring | Join each wire to the same colour | — |
| Swipe Card | Press SPACE while the card is in the green zone | Staff Keycard (search the gallery) |
| Unlock Display Case | Type a 4-digit code | Curator's note + order from the Archive logbook |
| Upload Data | Start the upload and wait | Partner's **Download Data** |

| Player 2: Archive | Mini-game | Needs |
|---|---|---|
| Fix Wiring | Join each wire to the same colour | — |
| Download Data | Start the download and wait | — |
| Reset Breakers | Press switches 1 to 10 in order | — |
| Open Cabinet | Type a 3-digit code | Painting numbers from the Gallery + order from the archive document |

Task stations glow **yellow** on the map and turn **green** when done.

## Documentation

| File | Contents |
|---|---|
| `docs/user_manual.md` | How to play, controls, tasks, troubleshooting, hints |
| `docs/test_plan.md`, `docs/test_report.md` | What we tested and the results |
| `docs/design/` | Concept, map sketches, tile legend, task & puzzle flow, wireframes, network protocol, architecture |
| `docs/IMPLEMENTATION_PLAN.md` | Inherited four-role plan, retained as a historical reference |
| `docs/LEARNING_ROADMAP.md` | Proposed two-developer learning plan using the confirmed four grading phases |
| `docs/PROGRESS_REPORT_TEMPLATE.md` | Actual check-in evidence, implementation, tests and learning |
| `docs/FEATURE_NOTE_TEMPLATE.md` | Required explanations for every changed file/function |
| `docs/progress_reports/` | Unverified imported report drafts (Weeks 1-4) |
| `docs/credits.md` | Team members and contributions |

## Project structure

```
main.py                 Starts the game and restores the terminal when it ends
game/                   Game rules only: no drawing, no networking (easy to test)
  tiles.py              What each map character means
  game_map.py           Loading a map, and the movement rule
  level.py              Loading a level folder (maps + objects JSON)
  session.py            One player's state and what E / F do
  team.py               Shared state: finished tasks and who escaped
client/                 Everything you see and press
  app.py                Menu -> game -> menu
  menu.py               Main menu and How to Play
  lobby.py              Host / Join screens for LAN games
  network.py            LocalLink (one computer) and NetLink (LAN) messages
  game_screen.py        The main game loop and in-game screen
  tasks.py              The Among Us style mini-games
  screens.py            Close-up, journal, chat input, messages, win screen
  dialogue.py           Splits long text into dialogue-box pages
  sprites.py            The ASCII player character
  ui.py                 Colours, keys and drawing helpers
assets/maps/records_wing/   level.json, two maps, and two objects files
assets/art/             ASCII close-up pictures
tests/                  Movement, interaction, task, and network tests
```

## How it fits together

1. `game_screen.py` reads a key and calls the rules in `game/`, for example
   `session.move("up")` or `session.interact(team)`.
2. The rules return a `Result` saying what to show: text, a close-up, a task
   to start, or "the player escaped".
3. When something happens that the partner must know about (a finished task, a
   shared clue, a chat message, an escape), the game sends a small message:
   `link.send({"type": "task", "id": "g_wires"})`.
4. Every 0.1 s the game reads incoming messages with `link.receive()` and
   updates the shared `Team`. On one computer, `LocalLink` returns the messages
   straight back. Over LAN, `NetLink` sends them as one line of JSON each.

## Adding content

**A new object.** Add an entry to the room's `_objects.json`. Its position must
be a solid tile on the map (wall, furniture, or `%` control panel):

```json
{ "id": "g_wires", "name": "Wiring Panel", "at": [[40, 1]], "kind": "task",
  "task": "wires", "title": "Fix Wiring", "text": ["Four wires are loose."] }
```

- **Kinds:** `examine`, `container` (needs `item`), `task`, `exit`.
- **Task types:** `wires`, `swipe`, `download`, `upload`, `code` (needs
  `answer`), `breaker`.
- **Optional fields:** `art`, `inspect`, `clue`, `reveal_art`, `after`,
  `needs_item` / `needs_task` (with `locked_text`), `hint`, `seconds`.

**A new mini-game.** Write a `play_<name>(stdscr, task)` function in
`client/tasks.py` that returns True when finished. Add it to the `games`
dictionary in `run_task()` and to `TASK_TYPES` in `game/level.py`.
