# 2R1E — Two Rooms, One Exit: Progress Report — Dev Week 4 (Final)

## A. Report Information

| Field | Value |
|---|---|
| Section | __________ |
| Game title | 2R1E — Two Rooms, One Exit |
| Dev week | ☐ Week 1 ☐ Week 2 ☐ Week 3 ☑ Week 4 |
| Implementation phase | ☑ Phase 4 Integration, Testing, Documentation & Submission |
| Check-in dates | November 23 and 25 |
| Reporting period | November 19 to November 25 |
| Target progress for this week | 100% |
| Actual overall progress | **100%** |
| Status | ☑ Complete |
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

The game is complete. Two players can finish Map 1 *The Records Wing* on one
computer or over LAN. Each player has 4 Among Us style tasks, and the team
shares one TOTAL TASKS bar. Players work together through shared clues and
the new **chat (T)**. Both open bugs from Week 3 are fixed. All documentation
is written: README, user manual, test report, design documents, and all four
progress reports. The final source code is packaged as the submission zip.
**33 automated tests pass.**

---

## C. Phase Checklist (Phase 4 — Definition of done)

| Phase item | Done? | Notes |
|---|---|---|
| Fully playable by two players from start to finish | ☑ | Map 1 (Map 2 dropped in Week 3) |
| All tests pass; no known crash bugs | ☑ | 33/33; see `docs/test_report.md` |
| All documents complete | ☑ | See section E |
| Zip tested by unzipping into a new folder and running it | ☑ | |

---

## D. Task Progress per Member

| Task # | Task | Owner | Due | Status | % done | Evidence |
|---|---|---|---|---|---|---|
| 4.1 | Chat between players (replaced Map 2 puzzles) | M2 | Nov 23 | ☑ Done | 100 | T key; `chat()` in `client/game_screen.py` |
| 4.2 | Feature freeze and merge | M1 | Nov 23 | ☑ Done | 100 | |
| 4.3 | Full regression test | M4 | Nov 23 | ☑ Done | 100 | `docs/test_report.md` |
| 4.4 | Network stress check | M2 | Nov 23 | ☑ Done | 100 | Partner quits mid-game → "Your partner left" screen; wrong IP / port in use show an error message |
| 4.5 | Difficulty tuning | M3 | Nov 23 | ☑ Done | 100 | Shorter objective text; clearer lock messages |
| 4.6 | Bug fixing | All | Nov 25 | ☑ Done | 100 | BUG-07 and BUG-08 fixed |
| 4.7 | Documentation | M4 + all | Nov 25 | ☑ Done | 100 | `README.md`, `docs/` |
| 4.8 | Presentation and demo script | M1 + M3 | Nov 25 | ☑ Done | 100 | Demo steps in section E |
| 4.9 | Package the submission zip | M1 | Nov 25 | ☑ Done | 100 | |
| 4.10 | Final two-player playthrough | M2 + M3 | Nov 25 | ☑ Done | 100 | Escape time: ________ |

### Individual contributions (whole project)
**M1:** concept, architecture, game rules (`game/`), task and exit logic, integration, packaging. Hours: ____
**M2:** networking research and protocol, `NetLink`, host/join, disconnects, chat, network tests. Hours: ____
**M3:** both maps, all objects, clues and puzzles, task data, playtests, difficulty tuning, level design doc. Hours: ____
**M4:** all screens and mini-games, ASCII art, tests, test plan and report, README and user manual. Hours: ____

---

## E. Demonstration (final presentation)

| # | What we show | Presenter |
|---|---|---|
| 1 | Concept and the Among Us task idea (`docs/design/concept.md`) | M1 |
| 2 | Host and join over LAN; chat "hello" | M2 |
| 3 | Player 1 does Fix Wiring and Swipe Card; Player 2 does Breakers and Download | M4 / M3 |
| 4 | Cross-room codes: share clues, enter 246 and 1259 | M3 |
| 5 | Upload unlocks, 8/8, both escape, win screen | All |
| 6 | Code tour: `game/` rules vs `client/` screens; run the tests | M1 |

**Documents in the submission:** `README.md`, `docs/user_manual.md`,
`docs/test_plan.md`, `docs/test_report.md`, `docs/design/*`
(concept, maps, puzzles, protocol, architecture...),
`docs/IMPLEMENTATION_PLAN.md`, `docs/progress_reports/*`, `docs/credits.md`.

---

## F. Testing

| Item | Result |
|---|---|
| Unit tests | **33 passed / 0 failed** |
| Regression | Full playthrough on one computer and over LAN; every mini-game with wrong inputs |
| Platforms tested | ☑ macOS ☑ Two game windows over LAN on one computer ☐ Linux (fill in) ☐ Two computers over LAN (fill in) |

### Bugs

| ID | Description | Severity | Owner | Status |
|---|---|---|---|---|
| BUG-07 | Long objective text past the dialogue box edge | Low | M4 | **Fixed:** text is cut to the box width, and the objectives are shorter |
| BUG-08 | Win banner's first line shifted by one column | Low | M4 | **Fixed:** the banner is centred as one block |

No known open bugs.

---

## G. Issues and Changes to the Plan

| Change | Reason |
|---|---|
| Map 2 dropped (decided in Week 3) | Kept time for a polished Map 1, testing and documentation |
| Chat added in Week 4 instead | Finished task 3.7 and makes LAN play easier |

**Known limitations:** macOS/Linux only (`curses`); the terminal must be at
least 80 x 24; LAN play needs both computers on the same network with port
5050 open; keypad answers are stored in plain JSON files.

---

## H. Final Submission

| Item | Value |
|---|---|
| File | `Section_2R1E.zip` (complete source code + documentation) |
| Run | `python3 main.py` |
| Tests | `python3 -m unittest -v` |

## I. Sign-off

| Member | Signature | Date |
|---|---|---|
| M1 | | |
| M2 | | |
| M3 | | |
| M4 | | |

**Instructor remarks:**

________________________________________________________________________
