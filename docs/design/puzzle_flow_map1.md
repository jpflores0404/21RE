# Task & Puzzle Flow — Map 1: The Records Wing

> Inherited baseline reference, reviewed October 23, 2026. This describes the current alternate design, not approval of deviations from the master. Read [the audit](../REPOSITORY_AUDIT.md) and [proposed two-developer roadmap](../LEARNING_ROADMAP.md) before implementation. John confirmed the four grading phases; original four-person staffing and reduced campaign are not the current plan.

**Task 1.7 · Owner: M3 · Due: Oct 28**

Every task is written as: **where → mini-game → what it needs → who has the info**.

## Player 1 — Gallery (4 tasks)
| # | Task | Station | Mini-game | Needs |
|---|---|---|---|---|
| G1 | Fix Wiring | Wiring Panel | Connect 4 coloured wires to the same colour | nothing (solo) |
| G2 | Swipe Card | Card Reader | Press SPACE while the card is in the green zone | **Staff Keycard**, hidden under the pedestal |
| G3 | Unlock Display Case | Display Case | 4-digit keypad | Curator's note (P1) + **order from P2's logbook** |
| G4 | Upload Data | Gallery Terminal | Progress bar, must not leave | **P2 must finish Download Data first** |

## Player 2 — Archive (4 tasks)
| # | Task | Station | Mini-game | Needs |
|---|---|---|---|---|
| A1 | Fix Wiring | Wiring Panel | Same as G1 (shared "common task") | nothing |
| A2 | Download Data | Archive Terminal | Progress bar, must not leave | nothing |
| A3 | Reset Breakers | Fuse Box | Press switches 1–10 in order (shuffled) | nothing |
| A4 | Open Cabinet | Archive Cabinet | 3-digit keypad | **painting numbers from P1** + order from the archive document |

## Cross-room puzzles (answers — SPOILERS)
**Cabinet code (A4) = 246**
- P1 inspects the paintings: Watcher = 6, Key = 4, Lantern = 2.
- P2 reads the archive document: *"First comes the light. Then the means of
  passage. The watcher shall always remain last."* → Lantern, Key, Watcher.
- Hint (archivist's note in a loose binder): *the code hides in the gallery paintings.*

**Display case code (G3) = 1259**
- P1 opens the curator's desk: X = 14 − 9 = 5, Y = 3 × 3 = 9, Z = 8 + 4 = 12.
- P2 reads the clerk's logbook: *Z first, then X, then Y* → 12, 5, 9.

**Download → Upload (A2 → G4)**
- The gallery terminal says *NO DATA TO UPLOAD* until P2 finishes the download.

## Flow chart
```
 P1 Gallery                                   P2 Archive
 ----------                                   ----------
 pedestal -> Staff Keycard -> G2 Swipe        A1 Wiring     A3 Breakers
 G1 Wiring
 paintings (2,4,6) ------- share -----------> archive document (order) -> A4 Cabinet 246
 curator note (X,Y,Z) <--- share ------------ clerk logbook (Z,X,Y)
   -> G3 Display Case 1259
 G4 Upload  <------------- must finish first - A2 Download
                 \                          /
                  ---> TOTAL TASKS 8/8 <----
                         exit opens
                both players walk out -> WIN
```

## Checklist
- [x] No cross-room lock can be solved with one room's clues alone.
- [x] Every clue comes from a specific action (E to open, F to inspect).
- [x] Every puzzle has a written answer and a hint.
- [x] Nothing can be lost or used up, so there are no dead ends.
- [x] Target time: 10–15 minutes for new players.
