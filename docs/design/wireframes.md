# Screen Wireframes (80 x 24)

**Task 1.9 · Owner: M4 · Due: Oct 28**

## Main menu
```
                              ____  ____  _ _____
                             |___ \|  _ \/ | ____|      (title art)
                              TWO ROOMS, ONE EXIT

   (P1)         > Play on this computer (Tab switches player) <       (P2)
                  Host LAN game  (you are Player 1 - Gallery)
                  Join LAN game  (you are Player 2 - Archive)
                                  How to play
                                     Quit
 [W/S] Choose  [Enter] Select  [Q] Quit
```

## Game screen
```
row 0  | TOTAL TASKS [#####...............] 2/8              LAN  Player 1 |
rows   | ##################################  | +- PLAYER 1 -----------+   |
1-15   | #.....P.....P.....P......%......#   | |   (player sprite)    |   |
       | #..@..........o.................#   | |    Gallery Room      |   |
       |   MAP (50 wide)                     | | YOUR TASKS           |   |
       |                                     | | [x] Fix Wiring       |   |
       |                                     | | [ ] Swipe Card       |   |
       | ###############=################    | | Items 1  Clues 2     |   |
rows   | +- Curator's Desk ---------------------------------------------+ |
16-21  | | You slide open the desk drawer...                    [E] Next | |
row 22 |  ! Wiring Panel  [E] Use  [F] Inspect                            |
row 23 | WASD Move  E Use  F Inspect  I Journal  T Chat  H Help  Q Menu   |
```
- Task stations are **yellow** until done, then **green**.
- The tile the player faces is highlighted (reverse colours).

## Close-up (F)
Full screen: the ASCII art of the object, then its description, then
"* New clue saved *" in green.

## Journal (I)
ITEMS line, then a CLUES list with a cursor. Enter = share the selected clue
with the partner.

## Task mini-games
| Mini-game | Layout |
|---|---|
| Wiring | 4 colour labels on the left, 4 on the right, `>>` cursor, connected wires drawn as L-shaped lines |
| Swipe | Card picture, a 40-cell track with a green zone, a moving `#` marker |
| Download/Upload | Two folder icons, a flying `[#]` file, a progress bar, "Estimated time" |
| Keypad | Code display `_ _ _`, a 3x3+1 keypad, red "ACCESS DENIED" |
| Breakers | 2 x 5 grid of numbered switches; switches already pressed turn green |

## Win screen
Big "You Escaped!" title, escape time, "press any key".
