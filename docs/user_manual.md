# 2R1E — Two Rooms, One Exit: User Manual

## 1. The story
You wake up on a cold floor in the **Records Wing**. The door is sealed, and a
note says **FINISH ALL TASKS**. Somewhere on the other side of the wall your
partner woke up in a different room. Neither of you can see the other's room,
but you share **one exit**.

## 2. What you need
- A Mac or Linux computer with Python 3.10 or newer (`python3 --version`)
- A terminal window of at least 80 x 24 characters
- For LAN play: two computers on the same Wi-Fi

## 3. Starting the game
1. Unzip the folder and open a terminal inside it.
2. Type `python3 main.py` and press Enter.
3. Choose an option with W/S and press Enter:

| Option | Use it when |
|---|---|
| **Play on this computer** | Two people share one keyboard. Press **Tab** to hand over to the other player. |
| **Host LAN game** | You are Player 1 (Gallery). Read your IP address to your partner and wait. |
| **Join LAN game** | You are Player 2 (Archive). Type the host's IP address and press Enter. |
| **How to play** | Shows the rules and controls. |

## 4. Controls
| Key | What it does |
|---|---|
| W A S D or arrows | Walk. Walk into something to face it. |
| E | Use what you face: read it, open it, start a task, go through the exit |
| F | Look closely. Details you read are saved as clues. |
| I | Journal. W/S picks a clue; **Enter sends it to your partner**. |
| T | Chat (LAN games) |
| Tab | Switch player (one-computer games) |
| H | Help |
| Q / Esc | Close a box, leave a task, or return to the menu |

## 5. The screen
```
 TOTAL TASKS [#####...............] 2/8          <- team progress (both rooms)
 +---- map ----------------------+  +- PLAYER 1 -+
 |  @ = you   yellow = task      |  | YOUR TASKS |  <- [x] done, [ ] to do
 |  green = task done            |  | [x] ...    |     dim = waiting on partner
 +-------------------------------+  +------------+
 +- dialogue box ---------------------------------+
  ! Wiring Panel  [E] Use  [F] Inspect           <- what you are facing
```

## 6. Tasks
Find the **yellow** stations on your map and press E.

| Mini-game | How to play |
|---|---|
| **Fix Wiring** | W/S to choose a wire on the left, Enter, then choose the same colour on the right, Enter. Connect all 4. |
| **Swipe Card** | A marker runs along the track. Press SPACE while it is inside the green zone. |
| **Download / Upload Data** | Press E to start, then wait for the bar to fill. Leaving early cancels it. |
| **Keypad Code** | Type the digits and press Enter. Backspace deletes. |
| **Reset Breakers** | WASD to move, Enter to press. Press the switches 1, 2, 3 ... 10 in order. A mistake resets them all. |

Some tasks are locked:
- **Swipe Card** needs a keycard that is hidden in the gallery.
- **Upload Data** works only after your partner finishes **Download Data**.
- The **keypads** need numbers from one room and the order from the other room.
  Talk to each other, or share clues from your journal!

## 7. Winning
When the TOTAL TASKS bar shows **8/8**, the exits turn green. Both players walk
to the exit (`=`) and press E. When both are out, you win, and the game shows
your escape time.

## 8. Troubleshooting
| Problem | Fix |
|---|---|
| "Terminal too small" | Make the window bigger (at least 80 x 24). |
| Join says "Could not connect" | Check the IP address. Make sure the host is on the waiting screen and both computers use the same Wi-Fi. Some school networks block this; try a phone hotspot. |
| Host says "Could not open port 5050" | Another copy of the game is already hosting. Close it. |
| "Your partner left the game" | The other player quit or lost the connection. Start a new game. |

## 9. Hints (spoilers!)
<details><summary>Show the answers</summary>

- Keycard: under the **Empty Pedestal** in the gallery.
- Archive Cabinet: the paintings are Lantern 2, Key 4, Watcher 6. The order is
  "light, passage, watcher", so the code is **246**.
- Display Case: X = 5, Y = 9, Z = 12. The logbook order is Z, X, Y, so the code is **1259**.
</details>
