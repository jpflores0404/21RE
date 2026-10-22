# Game Concept — 2R1E: Two Rooms, One Exit

**Task 1.1 · Owner: M1 · Due: Oct 26**

## Premise
Two people wake up in two different sealed rooms of an old records building:
the **Gallery Room** and the **Archive Room**. They cannot see each other, but
both rooms share **one exit**. A note on each door says: *FINISH ALL TASKS.*

## Core idea (inspired by Among Us tasks)
- Each player has their own **task list** (4 tasks each, 8 in total).
- Every finished task fills one shared **TOTAL TASKS** bar.
- Each task is a short **mini-game**: fix wiring, swipe a card, download/upload
  data, enter a keypad code, reset breakers.
- Some tasks are **impossible alone**. One player has the numbers, the other has
  the order. One player downloads data, the other uploads it.
- When the bar is full, the exit opens. **Both** players must walk out to win.

## Players and rooms
| Player | Room | Starts at |
|---|---|---|
| Player 1 | Gallery Room (paintings, curator's office) | west side of the gallery |
| Player 2 | Archive Room (bookshelves, vault, clerk's desk) | middle of the archive |

## Win and lose conditions
- **Win:** all 8 tasks done, and both players go through the exit.
- **No lose state.** The game shows the escape time at the end, so teams can
  try to beat it.

## How the game is played
- **One computer:** both players share the keyboard, and Tab switches rooms.
- **LAN:** one player hosts and the other joins with the host's IP address.

## Controls (draft)
| Key | Action |
|---|---|
| W A S D | Move. Walking into something faces it. |
| E | Use what you face |
| F | Look closely; reading details saves clues |
| I | Journal (items, clues, share clue with partner) |
| T | Chat (LAN) |
| Tab | Switch player (one computer) |
| Q / Esc | Back |

## Platform
Terminal game in Python 3.10+ using only the standard library (`curses` for
the screen, `socket` for LAN), so graders do not need to install anything.
