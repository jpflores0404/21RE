# Shared instructions for 2R1E

These instructions apply to the entire repository and to both developers and their AI assistants.

## Explicit direction from John Francis Espiritu

John Francis Espiritu supplied these requirements on October 23, 2026. The team has **two developers**. His partner must read this file before implementing. These are explicit project instructions, not suggestions inferred by an assistant.

The goal is a playable Linux terminal LAN escape game that both college Operating Systems students can understand, change, test, and explain down to the source code. AI provides guidance, explanations, and review. Developers implement small approved steps and demonstrate their understanding. A finished game without that understanding does not meet the goal.

## Read before work

1. Read `docs/PROJECT_MEMORY.md` for the current baseline, decisions, and unresolved questions.
2. Before changing requirements, puzzles, networking, or scope, read the relevant section of `docs/MASTER_PROMPT.md` and `docs/REPOSITORY_AUDIT.md`.
3. Before proposing implementation, read `docs/LEARNING_ROADMAP.md`. Its implementation steps are a proposal until approved. John confirmed the four grading phases on October 23; those dates replace the original eight-week schedule.
4. Before teaching or modifying an existing module, read its source and the corresponding section of `docs/CODE_GUIDE.md`.
5. After a feature, complete `docs/FEATURE_NOTE_TEMPLATE.md` and the weekly report template. Record evidence from the actual work.

## Authority and scope

- Investigate and plan freely. Substantial implementation requires a concrete reviewed plan and explicit approval from the developers. Implement only the approved step or phase.
- Use this order: understand, inspect, plan, review plan, obtain approval, implement, test, review against the goal, correct within scope, report.
- A later explicit developer instruction and an explicitly approved plan govern the work. Record consequential changes to the master requirements, their tradeoffs, and who approved them.
- If the plan becomes materially wrong, explain the new evidence and proposed revision before changing architecture, scope, dependencies, or behavior.
- Routine low-risk documentation corrections and narrowly scoped unambiguous fixes can proceed when authorized. Never interpret review or planning as permission to implement the campaign.
- Preserve existing work. Inspect Git status before editing and the diff before finishing. Commit, push, install dependencies, deploy, delete content, or rewrite history only when explicitly authorized.

## Student-led implementation

For each approved step:

1. Explain the concept in plain language and define unfamiliar terms.
2. Trace one small example through the existing code. Explain inputs, return values, and changed state.
3. Agree on one function or closely related small group of functions and a checkable acceptance condition.
4. Let the developer write or complete the function. Give hints and small examples; ask before supplying a whole implementation.
5. Review their attempt, explain specific mistakes, and run the narrow relevant checks.
6. Have the other developer review and explain the change without reading an AI answer aloud.
7. Update feature notes and record an honest weekly result before proceeding to a dependent step.

Use Concept -> Simple example -> Project example -> Developer attempt -> Review -> Test -> Teach-back. AI-authored documentation is reference material until the developers review it. Do not claim the developers understand code merely because notes exist.

## Keep the code understandable

- Prefer Python's standard library, plain functions, dictionaries, lists, and small state-holding classes. Reuse working map, collision, inspection, and drawing code where suitable.
- Explain why a new abstraction is necessary. Defer generic puzzle engines, plugin systems, distributed frameworks, elaborate class hierarchies, and optional features unless a specific approved requirement needs them.
- Separate networking, authoritative rules/state, terminal rendering, and content. Keep rule tests independent of terminal imports.
- The intended architecture has one authoritative server and exactly two clients. Clients send intentions, such as movement or an answer. The server validates them, changes state, and sends each player only permitted information.
- Puzzle solutions and partner secrets stay out of client payloads and client-side validation. A public educational repository is not a secret vault; the boundary concerns runtime behavior and messages.
- Validate network message type, fields, types, lengths, identity, ownership, legal actions, and current stage before state changes. Treat valid JSON as untrusted input.
- Use one understandable owner for authoritative state, preferably a single event loop if approved. Explain queue-based thread communication if retained. Use `time.monotonic()` for deadlines.
- Keep wrong answers retryable, disconnections explicit, and timed failure recoverable at the current level/checkpoint.
- Preserve terminal size handling, keyboard navigation, clean shutdown, and readable clues without relying on color alone.

## Notes required for every implementation

Create one feature note using `docs/FEATURE_NOTE_TEMPLATE.md`. Account for **every added or changed file and function**, including tests, assets, configuration, helpers, methods, and callbacks where relevant. Explain purpose, parameters, return values, state changes, important branches/loops, callers, and errors. Use a short worked example. Explain blocks line by line when the students need it; avoid comments that merely repeat syntax.

Include the player's experience, input-to-output flow, actual OS concept demonstrated, test instructions/results, limitations, common failures, and a short professor explanation. Keep source comments focused on reasoning. Each developer must predict the example's result and trace the actual code before marking the learning check complete.

## Two-developer collaboration

- Developer A: **John Francis Espiritu**. Developer B: partner, name to be confirmed by the team.
- Agree who implements and who reviews each small step. Rotate roles so both learn networking, game logic, content, and terminal code.
- Avoid simultaneous edits to the same function. Pair on protocol/state contracts before splitting dependent work.
- Record actual authorship and review. The inherited Git author is John Paul S. Flores; that metadata alone does not establish the current partner assignment or individual feature contributions.
- Preserve attribution and disclose AI assistance accurately. Existing code is an inherited baseline to study, not proof of student authorship or comprehension.

## Weekly reporting and completion

- Use `docs/PROGRESS_REPORT_TEMPLATE.md` for actual dates, tasks, owners, evidence, tests, learning, blockers, and next steps.
- Distinguish inherited baseline, work implemented this week, code studied/reviewed, and future work. Studying an existing feature is learning progress, not newly implementing it.
- Never backdate work, manufacture percentages, signatures, meetings, screenshots, tests, or playtests. The original Week 1-4 files are unverified imported drafts, not completed student reports.
- If a professor requires a percentage, define the approved checklist and denominator, then report accepted items over total items. Keep targets separate from measured results.
- Done means the approved behavior works, relevant checks pass, the diff is reviewed, notes match the code, both developers can explain the feature, and limitations are disclosed.
- Report what changed, important decisions, verification, deviations, and remaining issues. Distinguish unit tests, controlled probes, terminal testing, and two-computer Linux LAN evidence.

## Current implementation boundary

The October 23 review authorized cloning, analysis, project-memory preservation, shared instructions, and a proposed step-by-step plan. John confirmed check-ins of Oct 26/28 at 20%, Nov 4/9/11 at 50%, Nov 16/18 at 70%, and Nov 23/25 at 100%. This did **not** authorize replacing the peer-to-peer design, changing puzzles, or implementing missing maps. The implementation plan and architecture changes still await approval. Update this paragraph when the developers explicitly approve a phase and record that decision in project memory.
