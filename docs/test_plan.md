# Test Plan

> Inherited baseline reference, reviewed October 23, 2026. This describes the current alternate design, not approval of deviations from the master. Read [the audit](REPOSITORY_AUDIT.md) and [proposed two-developer roadmap](LEARNING_ROADMAP.md) before implementation. John confirmed the four grading phases; original four-person staffing and reduced campaign are not the current plan.

**Task 1.13 · Owner: M4 · Due: Oct 28**

Automated tests use `unittest` (built in): `python3 -m unittest -v`.

| Week | Area | Automated tests | Manual tests |
|---|---|---|---|
| 2 | Map loading | unknown tile, missing/double spawn, short rows | open each map in the game |
| 2 | Movement | floor ok, walls/objects/doors block, edges, facing turns | walk every wall of both rooms |
| 2 | Map design | rooms closed in, every floor reachable (flood fill), every object reachable | — |
| 2 | Interaction | E text, F clue saved once, container opens once, 2-tile objects | read every object |
| 2 | Network prototype | messages sent/received on localhost, disconnect gives "bye" | two terminals |
| 3 | Tasks | task starts, needs item, needs partner task, can't redo, codes match clues | play each mini-game, wrong inputs |
| 3 | Team / exit | sealed until 8/8, win only when both escape | full playthrough on one computer |
| 3 | LAN | — | host + join on two laptops, chat, clue share, quit mid-game |
| 4 | Regression | all of the above | full playthrough on macOS and Linux, small window, key mashing |

Bug report format: **ID · steps · expected · actual · severity (High/Med/Low) · owner · status**.
