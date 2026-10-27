# 2R1E — Two Rooms, One Exit: Progress Report — Dev Week 3

> **UNVERIFIED IMPORTED DRAFT, not a completed weekly report.** This file was already present in the October 23, 2026 baseline, before the check-in dates described below. Its completion percentages, contributions, meetings, tests, packaging and sign-offs have not been established by this review. Original wording is retained as a reference. Use [the actual-report template](../PROGRESS_REPORT_TEMPLATE.md), record work when it happens, and consult [the audit](../REPOSITORY_AUDIT.md).

## A. Report Information

| Field | Value |
|---|---|
| Section | __________ |
| Game title | 2R1E — Two Rooms, One Exit |
| Dev week | ☐ Week 1 ☐ Week 2 ☑ Week 3 ☐ Week 4 |
| Implementation phase | ☑ Phase 3 Puzzles & Multiplayer |
| Check-in dates | November 16 and 18 |
| Reporting period | November 12 to November 18 |
| Target progress for this week | 70% |
| Actual overall progress | **70%** |
| Status | ☑ On track (Map 2 moved out of scope; see G) |
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

Map 1 is now playable from start to finish by two players, either on one
computer (Tab switches rooms) or over LAN (host/join). Every task station
starts an **Among Us style mini-game**: Fix Wiring, Swipe Card, Download Data,
Upload Data, Keypad Code, and Reset Breakers. Each finished task fills the
shared TOTAL TASKS bar on both computers. Clues can be sent to the partner
from the journal. When 8/8 tasks are done, the exit turns green, and the game
is won when both players walk out. **33 automated tests pass.**

---

## C. Phase Checklist (Phase 3 — Definition of done)

| Phase item | Done? | Notes |
|---|---|---|
| Two players can host/join and play Map 1 to the win | ☑ | Tested with two terminals on one laptop; two laptops over Wi-Fi: ______ |
| Clue sharing works; each player sees only their own room | ☑ | Journal → Enter |
| Keypad puzzles work; wrong answers give feedback | ☑ | "ACCESS DENIED" |
| Map 2 loads and is walkable | ☐ | Moved out of scope (see G) |
| Task tests pass | ☑ | `tests/test_tasks.py` |

---

## D. Task Progress per Member

| Task # | Task | Owner | Due | Status | % done | Evidence |
|---|---|---|---|---|---|---|
| 3.1 | Task system and shared team state | M1 | Nov 16 | ☑ Done | 100 | `game/team.py`, `use_task()` in `game/session.py` |
| 3.2 | Mini-games (6 types) | M4 | Nov 16 | ☑ Done | 100 | `client/tasks.py` |
| 3.3 | Map 1 answers, hints, lock text | M3 | Nov 16 | ☑ Done | 100 | `*_objects.json` |
| 3.4 | Host / Join screens | M2 + M4 | Nov 16 | ☑ Done | 100 | `client/lobby.py` |
| 3.5 | Network connected to the game | M2 + M1 | Nov 16 | ☑ Done | 100 | `handle_messages()` in `client/game_screen.py` |
| 3.6 | Task tests | M4 | Nov 16 | ☑ Done | 100 | 11 new tests |
| 3.7 | Share clue + partner message panel | M2 + M4 | Nov 18 | ◐ Partly done | 70 | Clue sharing done; **chat (T) moved to Week 4** |
| 3.8 | Shared TOTAL TASKS bar, exit, win | M1 + M2 | Nov 18 | ☑ Done | 100 | `Team.has_won()` |
| 3.9 | Disconnect handling | M2 | Nov 18 | ☑ Done | 100 | "Your partner left the game" screen |
| 3.10 | Map 2 build | M3 + M4 | Nov 18 | ☐ Not started | 0 | Dropped from scope (see G) |
| 3.11 | Win screen | M4 | Nov 18 | ☑ Done | 100 | `show_win()` with escape time |
| 3.12 | Two-player playtest | M3 + all | Nov 18 | ☑ Done | 100 | Section F |

### Individual contributions
**M1:** `Team` (shared tasks and escapes), task requirements, exit rules, game loop. Hours: ____
**M2:** connected `NetLink` to the game; host/join; message handling; disconnects. Hours: ____
**M3:** task data, codes 246 and 1259, lock text; ran the playtests. Hours: ____
**M4:** all 6 mini-games, task list panel, yellow/green task stations, win screen, tests. Hours: ____

---

## E. Demonstration

| # | What we show | How | Presenter |
|---|---|---|---|
| 1 | Host on laptop A, join from laptop B | Menu → Host / Join, type IP | M2 |
| 2 | Fix Wiring and Reset Breakers mini-games | Use the wiring panel / fuse box | M4 |
| 3 | Swipe Card needs the keycard from the pedestal | E on card reader before and after | M1 |
| 4 | Upload is locked until the partner downloads | Archive terminal, then gallery terminal | M3 |
| 5 | Share a painting clue; partner enters 246 | I → Enter; Archive Cabinet keypad | M3 |
| 6 | 8/8, exit turns green, both escape, win screen | Walk to the exits | All |

---

## F. Testing

| Item | Result |
|---|---|
| Unit tests | **33 passed / 0 failed** |
| New tests this week | Task requirements (6), exit & win (3), network host/join and disconnect (2) |
| Playtests | ____ two-player sessions. Average solve time: ________. Where players got stuck: ______________ |
| Platforms tested | ☑ macOS ☑ Two game windows over LAN on one computer ☐ Two computers over LAN (fill in) |

### Bugs found

| ID | Description | Severity | Owner | Status |
|---|---|---|---|---|
| BUG-05 | Upload could be started before the partner's download | High | M1 | Fixed: `needs_task` check |
| BUG-06 | The game froze while waiting for network messages | High | M2 | Fixed: background reader thread + 0.1 s key timeout |
| BUG-07 | Long objective text runs past the right edge of the dialogue box | Low | M4 | **Open** → Week 4 |
| BUG-08 | First line of the "You Escaped!" banner is shifted by one column | Low | M4 | **Open** → Week 4 |

---

## G. Issues, Blockers and Changes to the Plan

| Issue | Effect | Action | Owner |
|---|---|---|---|
| Map 2 would take all of Week 4 | Less time for testing and docs | **Dropped Map 2.** As planned in the risk table, Map 1 must be complete and polished first. The loader supports more maps later with no code changes. | M1 + M3 |
| Chat not finished | 3.7 partly done | Moved to Week 4 (small: reuses the text input from the Join screen) | M2 |

---

## H. Plan for the Final Check-ins (Nov 23, 25 — target 100%)

| Task # | Task | Owner | Due |
|---|---|---|---|
| 4.1 | Chat (T) between players (replaces the Map 2 work) | M2 | Nov 23 |
| 4.2 | Feature freeze and merge | M1 | Nov 23 |
| 4.3 | Full regression test | M4 | Nov 23 |
| 4.4 | Network stress check | M2 | Nov 23 |
| 4.5 | Difficulty tuning from playtests | M3 | Nov 23 |
| 4.6 | Fix BUG-07 and BUG-08, plus any new bugs | All | Nov 25 |
| 4.7–4.10 | Documentation, presentation, zip, final playthrough | All | Nov 25 |

## I. Sign-off

| Member | Signature | Date |
|---|---|---|
| M1 | | |
| M2 | | |
| M3 | | |
| M4 | | |

**Instructor remarks:**

________________________________________________________________________
