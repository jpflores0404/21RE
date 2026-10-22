# 2R1E — Two Rooms, One Exit: Progress Report — Dev Week 2

## A. Report Information

| Field | Value |
|---|---|
| Section | __________ |
| Game title | 2R1E — Two Rooms, One Exit |
| Dev week | ☐ Week 1 ☑ Week 2 ☐ Week 3 ☐ Week 4 |
| Implementation phase | ☑ Phase 2 Core Build, Movement Testing & Gameplay Polish |
| Check-in dates | November 4, 9 and 11 |
| Reporting period | October 29 to November 11 |
| Target progress for this week | 50% |
| Actual overall progress | **50%** |
| Status | ☑ On track |
| Prepared by | M1 |

### Team members

| Code | Name | Role |
|---|---|---|
| M1 | | Project Lead / Game Systems |
| M2 | | Networking Engineer |
| M3 | | Level & Puzzle Designer |
| M4 | | UI/UX, Art & QA/Docs |

---

## B. Summary

The game now runs. From the main menu a player can enter either room of
Map 1. They can walk around with wall and furniture collision, face objects,
read RPG-style dialogue, open containers for items, view ASCII close-ups, and
save clues in a journal. Task stations are placed on the map but only show a
"coming in Week 3" message for now. A separate network prototype can send
messages between two terminals. **24 automated tests pass.**

---

## C. Phase Checklist (Phase 2 — Definition of done)

| Phase item | Done? | Notes |
|---|---|---|
| `python3 main.py` opens the menu and starts Map 1 | ☑ | |
| Both rooms fully walkable; every object can be used and inspected | ☑ | Checked by tests and a playtest |
| Journal records clues and items | ☑ | |
| Movement and interaction tests pass | ☑ | 24/24 |
| Two terminals can exchange messages (network prototype) | ☑ | `tests/test_network.py` |

---

## D. Task Progress per Member

| Task # | Task | Owner | Due | Status | % done | Evidence |
|---|---|---|---|---|---|---|
| 2.1 | Tile legend in code | M1 | Nov 4 | ☑ Done | 100 | `game/tiles.py` |
| 2.2 | Map loader and checks | M1 | Nov 4 | ☑ Done | 100 | `game/game_map.py` |
| 2.3 | Movement, collision, facing | M1 | Nov 4 | ☑ Done | 100 | `try_move()` |
| 2.4 | Map 1 text files | M3 | Nov 4 | ☑ Done | 100 | `assets/maps/records_wing/*.txt` |
| 2.5 | App shell and menu | M4 | Nov 4 | ☑ Done | 100 | `main.py`, `client/app.py`, `client/menu.py` |
| 2.6 | Map, sprite, and status bar drawing | M4 | Nov 4 | ☑ Done | 100 | `client/game_screen.py` |
| 2.7 | Socket prototype | M2 | Nov 4 | ☑ Done | 100 | `client/network.py` |
| 2.8 | Movement tests | M4 + M1 | Nov 9 | ☑ Done | 100 | `tests/test_movement.py` (15 tests) |
| 2.9 | Object loader with checks | M1 | Nov 9 | ☑ Done | 100 | `game/level.py` |
| 2.10 | Map 1 object files | M3 | Nov 9 | ☑ Done | 100 | `*_objects.json` |
| 2.11 | Room session (E, F, containers, journal) | M1 | Nov 9 | ☑ Done | 100 | `game/session.py` |
| 2.12 | Dialogue box, side panel, hint line | M4 | Nov 9 | ☑ Done | 100 | `client/dialogue.py` |
| 2.13 | Message format and tests | M2 | Nov 9 | ☑ Done | 100 | `tests/test_network.py` |
| 2.14 | Close-up ASCII art | M4 + M3 | Nov 11 | ☑ Done | 100 | `assets/art/` (9 pictures) |
| 2.15 | Close-up and journal screens | M4 | Nov 11 | ☑ Done | 100 | `client/screens.py` |
| 2.16 | Interaction tests | M4 | Nov 11 | ☑ Done | 100 | `tests/test_interaction.py` |
| 2.17 | Gameplay polish | M1 + M4 | Nov 11 | ☑ Done | 100 | Highlighted faced tile, bump messages, instant Esc |
| 2.18 | Playtest both rooms | M3 | Nov 11 | ☑ Done | 100 | Section F |
| 2.19 | LocalLink / NetLink | M2 | Nov 11 | ☑ Done | 100 | `client/network.py` (not used by the game yet) |

### Individual contributions
**M1:** map loader, movement rules, level/object loader, room session. Hours: ____
**M2:** `LocalLink` / `NetLink` prototype with a background reader thread; host/join on localhost; tests. Hours: ____
**M3:** converted the sketches to map files, wrote every object's text and clue, placed the 8 task stations (`%` tiles), playtest. Hours: ____
**M4:** curses shell, all drawing, dialogue, close-ups, journal, ASCII art, tests. Hours: ____

---

## E. Demonstration

| # | What we show | How | Presenter |
|---|---|---|---|
| 1 | Menu, then walk around the Gallery and bump into walls | `python3 main.py` → Play as Player 1 | M4 |
| 2 | Pedestal gives the Staff Keycard; curator's desk opens once | Face the object, press E | M1 |
| 3 | Close-up of a painting saves a clue; open the journal | F, then I | M3 |
| 4 | Tab to the Archive room (testing shortcut) | Tab | M3 |
| 5 | Run the tests | `python3 -m unittest -v` | M4 |
| 6 | Network prototype: two computers exchange a message | `python3 -m unittest tests.test_network -v` | M2 |

---

## F. Testing

| Item | Result |
|---|---|
| Unit tests | **24 passed / 0 failed** |
| New tests this week | Movement (7), map loading (4), Records Wing maps (4: closed in, flood fill, objects reachable), interaction (7), network (2) |
| Manual tests | Walked along every wall of both rooms; used and inspected every object |
| Platforms tested | ☑ macOS ☐ Linux (fill in if tested) |

### Bugs found

| ID | Description | Severity | Owner | Status |
|---|---|---|---|---|
| BUG-01 | Writing to the bottom-right cell crashed curses | High | M4 | Fixed: safe `write()` helper |
| BUG-02 | Esc took 1 second to respond | Low | M4 | Fixed: `ESCDELAY=25` |
| BUG-03 | A typo in object coordinates made an invisible object | Med | M1 | Fixed: the loader now checks positions are solid tiles |

---

## G. Issues, Blockers and Changes to the Plan

| Issue | Effect | Action | Owner |
|---|---|---|---|
| None this week | All Phase 2 tasks were finished on time | Network code stays separate from the game until Week 3, as planned | M2 |

**Design changes:** objects are plain dictionaries read from JSON, which keeps
the code simple. Task stations use a new `%` "control panel" tile.

---

## H. Plan for the Next Check-ins (Nov 16, 18 — target 70%)

| Task # | Task | Owner | Due |
|---|---|---|---|
| 3.1 | Task system (task kind, needs item / needs partner task) | M1 | Nov 16 |
| 3.2 | Mini-games: wiring, swipe, download/upload, keypad, breakers | M4 | Nov 16 |
| 3.3 | Task answers and lock text in JSON | M3 | Nov 16 |
| 3.4–3.5 | Host/Join menu; connect the network to the game | M2 + M4 | Nov 16 |
| 3.6 | Task tests | M4 | Nov 16 |
| 3.7–3.9 | Clue sharing, shared TOTAL TASKS bar and exit, disconnects | M1 + M2 | Nov 18 |
| 3.10–3.12 | Map 2, win screen, two-player playtest | M3 + M4 | Nov 18 |

## I. Sign-off

| Member | Signature | Date |
|---|---|---|
| M1 | | |
| M2 | | |
| M3 | | |
| M4 | | |

**Instructor remarks:**

________________________________________________________________________
