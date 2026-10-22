# Test Report

**Task 4.3 · Owner: M4**

## 1. Automated tests
Command: `python3 -m unittest -v`. Result: **33 tests, 33 passed, 0 failed.**

| File | Tests | What is checked |
|---|---|---|
| `tests/test_movement.py` | 15 | Walking on floor; walls, objects and map edges block; a blocked move still turns you; bad map files are rejected (unknown tile, missing/double spawn); short rows are padded; both rooms are closed in; every floor tile is reachable (flood fill); every object has a tile to stand on |
| `tests/test_interaction.py` | 7 | E on a painting shows text without a clue; F saves a clue only once; containers open once and give their item; 2-tile objects work from either tile; open floor has nothing to use; plain bookshelves have a description; every art file exists |
| `tests/test_tasks.py` | 11 | 4 tasks per room; a wiring panel starts a task; Swipe Card needs the keycard; Upload waits for the partner's Download; finished tasks cannot be repeated; keypad answers match the clues; exit is sealed until 8/8; exit opens at 8/8; win only when both escape; LocalLink returns messages; two NetLinks talk over localhost and report "bye" on disconnect |

## 2. Manual tests

| # | Test | Expected | Result |
|---|---|---|---|
| M1 | Full game on one computer (Tab), all 8 tasks | Win screen with escape time | Pass |
| M2 | Wrong keypad code | "ACCESS DENIED", input cleared | Pass |
| M3 | Use the gallery terminal before the download | "NO DATA TO UPLOAD..." message | Pass |
| M4 | Exit before 8/8 | "Team tasks: 7/8." and stays sealed | Pass |
| M5 | Share a clue from the journal | Shows in the partner's journal as "From partner: ..." | Pass |
| M6 | Host + Join (two windows, 127.0.0.1) | Both start; a task done by P2 shows "Your partner finished: ..." for P1 | Pass |
| M7 | Chat over LAN | Partner sees "Player 1 says" | Pass |
| M8 | Partner quits mid-game | "Your partner left the game." then the menu | Pass |
| M9 | Terminal smaller than 80 x 24 | "Terminal too small" message, no crash | ______ |
| M10 | Two separate computers over Wi-Fi | Same as M6 | ______ |
| M11 | Linux terminal | Same as M1 | ______ |

## 3. Bugs found during the project

| ID | Description | Severity | Status |
|---|---|---|---|
| BUG-01 | Writing to the bottom-right cell crashed curses | High | Fixed (safe `write()`) |
| BUG-02 | Esc took 1 s to respond | Low | Fixed (`ESCDELAY=25`) |
| BUG-03 | Typo in object coordinates made an invisible object | Med | Fixed (loader checks positions) |
| BUG-05 | Upload could start before the partner's download | High | Fixed (`needs_task`) |
| BUG-06 | Game froze while waiting for network messages | High | Fixed (reader thread + 0.1 s timeout) |
| BUG-07 | Long objective text ran past the dialogue box | Low | Fixed |
| BUG-08 | Win banner's first line was shifted by one column | Low | Fixed |

No known open bugs.
