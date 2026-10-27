# 2R1E — Two Rooms, One Exit: Progress Report — Dev Week 1

> **UNVERIFIED IMPORTED DRAFT, not a completed weekly report.** This file was already present in the October 23, 2026 baseline, before the check-in dates described below. Its completion percentages, contributions, meetings, tests, packaging and sign-offs have not been established by this review. Original wording is retained as a reference. Use [the actual-report template](../PROGRESS_REPORT_TEMPLATE.md), record work when it happens, and consult [the audit](../REPOSITORY_AUDIT.md).

## A. Report Information

| Field | Value |
|---|---|
| Section | __________ |
| Game title | 2R1E — Two Rooms, One Exit |
| Dev week | ☑ Week 1 ☐ Week 2 ☐ Week 3 ☐ Week 4 |
| Implementation phase | ☑ Phase 1 Pre-Production |
| Check-in dates | October 26 and October 28 |
| Reporting period | October 22 to October 28 |
| Target progress for this week | 20% |
| Actual overall progress | **20%** |
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

The team finished pre-production. We agreed on the concept: a two-player
co-op escape room with **Among Us style tasks**. Each player has 4 tasks, and a
shared TOTAL TASKS bar opens the exit when all 8 are done. Map 1 (*The Records
Wing*) is sketched for both rooms. Every task and cross-room puzzle has a
written answer and hint. The screens, network messages and code structure are
designed. As planned, no game code was written this week.

---

## C. Phase Checklist (Phase 1 — Definition of done)

| Phase item | Done? | Notes |
|---|---|---|
| Concept, map sketches, tile legend, puzzle flow, wireframes, protocol, architecture in repo | ☑ | `docs/design/` |
| Every member can explain the full Map 1 puzzle chain | ☑ | Walked through it in the team meeting |
| Task board lists all Phase 2 tasks | ☑ | Section 4 of the Implementation Plan |

---

## D. Task Progress per Member

| Task # | Task | Owner | Due | Status | % done | Evidence |
|---|---|---|---|---|---|---|
| 1.1 | Game concept | M1 | Oct 26 | ☑ Done | 100 | `docs/design/concept.md` |
| 1.2 | Repository and folder setup | M1 | Oct 26 | ☑ Done | 100 | Empty `game/ client/ assets/ tests/ docs/` |
| 1.3 | Map 1 sketches (Gallery + Archive) | M3 | Oct 26 | ☑ Done | 100 | `docs/design/map1_sketch.txt` |
| 1.4 | Tile legend | M3 + M1 | Oct 26 | ☑ Done | 100 | `docs/design/tiles.md` |
| 1.5 | curses research | M4 | Oct 26 | ☑ Done | 100 | `docs/design/ui_research.md` |
| 1.6 | Networking research | M2 | Oct 26 | ☑ Done | 100 | `docs/design/network_research.md` |
| 1.7 | Map 1 task & puzzle flow | M3 | Oct 28 | ☑ Done | 100 | `docs/design/puzzle_flow_map1.md` |
| 1.8 | Map 2 concept | M3 | Oct 28 | ☑ Done | 100 | `docs/design/map2_concept.md` |
| 1.9 | Screen wireframes | M4 | Oct 28 | ☑ Done | 100 | `docs/design/wireframes.md` |
| 1.10 | Network protocol draft | M2 | Oct 28 | ☑ Done | 100 | `docs/design/protocol.md` |
| 1.11 | Architecture diagram | M1 | Oct 28 | ☑ Done | 100 | `docs/design/architecture.md` |
| 1.12 | Object data format | M1 + M3 | Oct 28 | ☑ Done | 100 | `docs/design/objects_format.md` |
| 1.13 | Test plan outline | M4 | Oct 28 | ☑ Done | 100 | `docs/test_plan.md` |

### Individual contributions
**M1:** concept, repo setup, architecture, object format (with M3). Hours: ____
**M2:** networking research (chose TCP + JSON lines, peer to peer), protocol draft. Hours: ____
**M3:** both room maps, tile legend (with M1), 8 tasks with the cross-room puzzles, Map 2 concept. Hours: ____
**M4:** curses research, all screen and mini-game wireframes, test plan. Hours: ____

---

## E. Demonstration

| # | What we show | How | Presenter |
|---|---|---|---|
| 1 | Concept and the "Among Us task" idea | `concept.md` | M1 |
| 2 | Both room maps and where each task station is | `map1_sketch.txt` | M3 |
| 3 | The flow chart of cross-room puzzles (codes 246 and 1259, download → upload) | `puzzle_flow_map1.md` | M3 |
| 4 | Game screen and mini-game wireframes | `wireframes.md` | M4 |
| 5 | Message list and why only shared events are sent | `protocol.md` | M2 |

---

## F. Testing

| Item | Result |
|---|---|
| Unit tests | ☑ N/A (Week 1, no code yet) |
| Manual checks | Paper playtest of the puzzle chain: ______________ (who tested, time taken) |
| Platforms tested | — |

### Bugs found
None (no code yet).

---

## G. Issues, Blockers and Changes to the Plan

| Issue | Effect on schedule | Action | Owner |
|---|---|---|---|
| School Wi-Fi may block LAN traffic | Could block the Week 3 demo | Backup: phone hotspot, or two terminals with 127.0.0.1 | M2 |

**Design changes this week:** the puzzles became **8 Among Us style tasks**
(4 per player) with a shared TOTAL TASKS bar, instead of one long puzzle chain.
Map 2 is a stretch goal only.

---

## H. Plan for the Next Check-ins (Nov 4, 9, 11 — target 50%)

| Task # | Task | Owner | Due |
|---|---|---|---|
| 2.1–2.3 | Tiles, map loader, movement | M1 | Nov 4 |
| 2.4 | Map text files | M3 | Nov 4 |
| 2.5–2.6 | Menu, map drawing | M4 | Nov 4 |
| 2.7 | Socket prototype | M2 | Nov 4 |
| 2.8–2.13 | Tests, objects, interaction, dialogue, protocol | All | Nov 9 |
| 2.14–2.19 | Art, close-ups, journal, polish, playtest | All | Nov 11 |

## I. Sign-off

| Member | Signature | Date |
|---|---|---|
| M1 | | |
| M2 | | |
| M3 | | |
| M4 | | |

**Instructor remarks:**

________________________________________________________________________
