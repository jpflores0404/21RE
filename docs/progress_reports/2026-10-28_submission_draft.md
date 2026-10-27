# Week 1 check in draft for October 28 2026

Prepared October 27, 2026, for the upcoming check-in. John Francis Espiritu and partner, name pending. Professor and section pending. Professor's target: 20 percent. Measured overall completion: not assessed against an approved weighted checklist.

## Actual work and evidence

- The game plan was revised to C, Windows-first development/testing and Linux acceptance. The planning revision was pushed as `f312f56` on October 27.
- A detailed design draft was prepared with the game description, six room sketches, sample puzzles for all three maps, solutions, state changes, recovery rules, technical direction and verification plan. Proposed maps and puzzles are not implemented features. See `docs/deliverables/2026-10-28_GAME_DESIGN_DRAFT.docx`.
- John explicitly approved an assistant-written first movement demo and notes. `src/movement_demo.c` implements WASD, terminal bounds, resizing and clean exit. This is newly written C source; the inherited Python game's features remain separate reference material.
- The existing MSVC 14.50.35717 compiler built the demo as C17 with `/W4 /WX /TC`, without installation. The real executable's 14 movement checks passed. A controlled native Windows console probe verified real key input, all four boundaries, resize clamping, pause in a small viewport, and Q/Ctrl+C restoration. These are controlled automated checks, not a human playtest.

## Presentation plan

Use the commands in `docs/MOVEMENT_DEMO.md`. Maximize the terminal, launch the native demo, move in four directions, demonstrate a boundary and resize, quit and show ordinary terminal input again. Add the actual presenter's name, terminal and observed result after rehearsal or presentation. Do not prefill those checks as completed.

## Learning and review

Implementation assistance: Codex, explicitly requested for this initial demo. Developer review and partner review: pending. Both developers must trace input, x/y state, movement validation, redraw and cleanup; then predict a blocked move and explain resize behavior. See `docs/features/movement_demo.md`. Notes alone do not establish student understanding.

## Remaining work

Human terminal rehearsal, Linux compilation, developer teach-back, final clue/layout review, server/client networking and all C campaign puzzles remain pending. The new draft is a design deliverable, not evidence of a playable three-map campaign. No new completion percentage has been assigned.

## Deviations and next step

The early movement demo is an explicitly approved exception to the prior Week 1 planning-only proposal. Its scope does not authorize the campaign port. The next step is developer review and a small approved map/collision or platform step with a checkable acceptance condition.

This draft must be reviewed by the two developers, supplemented with genuine presentation evidence and completed identification fields before submission.
