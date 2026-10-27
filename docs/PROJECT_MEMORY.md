# Project memory

## October 28 Linux movement study

John requested Linux-compatible C movement code and a detailed Windows comparison for the professor explanation. The maintained `src/movement_demo.c` already contains its POSIX branch, so it was reused without duplicating or changing runtime functions. Added `scripts/run_movement_linux.sh` to build/run it with an existing Linux compiler, and `LINUX_WINDOWS_MOVEMENT_GUIDE.md` plus its Word study copy to explain libraries, functions, controls, terminal modes, rendering and cleanup.

The available docker-desktop WSL distribution runs Linux but has no cc/gcc/clang on its command path; Docker's Linux engine was stopped. Launcher syntax passed and its missing-compiler error path returned one as expected. Windows shared movement tests passed again. Linux C compilation/runtime remains unverified; no compiler was installed. Source review identified Linux escape-sequence keys and suspend/SIGQUIT cleanup as existing limitations, explained in the guide. No student teach-back or Linux acceptance is claimed.

## October 27 C revision and first movement demo

John reported that the professor requires C and requested Windows-first testing with Linux support. `C_IMPLEMENTATION_PLAN.md` supersedes Python-specific future proposals. Its documentation revision was pushed to origin/main as `f312f56`; the original master prompt and historical audit remain records of the earlier baseline.

John requested a detailed October 28 design submission and explicitly approved the assistant writing the first narrow movement demo and explanatory notes. Approved scope: WASD marker movement across the current terminal viewport, boundaries, resize handling, Q exit and terminal restoration; no networking or campaign implementation. This is an explicit early implementation exception to the older Phase 1 no-runtime-work proposal, limited to this demo.

New source: `src/movement_demo.c`. Run guide: `MOVEMENT_DEMO.md`. Complete feature note: `features/movement_demo.md`. The existing MSVC compiler built C17 with warnings treated as errors; 14 movement checks and a controlled native Windows console probe passed. Linux build, human Windows Terminal acceptance and student teach-back remain unverified. The local executable and probe results are in the already ignored `submissions/movement-demo/` directory. No tool installation was needed.

Submission draft: `deliverables/2026-10-28_GAME_DESIGN_DRAFT.docx`, with editable Markdown alongside it. The draft contains proposed room sketches, clue text, solutions, recovery and final-release rules. New puzzle refinements are proposals, not approved runtime content. Do not equate the 20% checkpoint target with measured accepted campaign completion.

The sections below preserve the October 23 review and may describe the earlier Python baseline. Read the C revision above first for current language and implementation authorization.

Last reviewed October 23, 2026, Asia/Manila. This file records current decisions; `MASTER_PROMPT.md` preserves the supplied original verbatim.

## People and purpose

- Explicit instructions supplied by **John Francis Espiritu**.
- Two developers. Partner name awaits confirmation. The baseline commit names John Paul S. Flores as author; this is attribution, not a confirmed allocation of work.
- College Operating Systems project. Both students must be able to explain the source, demonstrate the OS concepts actually used, and make small changes themselves.
- AI should mentor and review. Teach and implement one approved small step at a time. Preserve accurate AI attribution; do not represent inherited implementation as newly student-written code.
- Each implementation needs beginner notes accounting for every changed file and function, tests, examples, and teach-back by both developers.

## Preserved source and baseline

- Repository: https://github.com/jpflores0404/21RE.git
- Cloned directly into this project folder on October 23, 2026.
- Reviewed commit: `6bfedc825c126c387559d62646a7ba79f5789163`, branch `main`.
- One commit was present across the fetched branches. Author/committer date: October 23, 2026, 01:10:06 +08:00. It contains code, tests, design documents, and all four reports.
- Commit includes `Co-Authored-By: Claude Opus 5.5`. This is evidence of declared AI assistance, not a complete account of how the code was produced or understood.
- No existing `AGENTS.md` was found in the cloned repository. The root `AGENTS.md` is the single shared instruction file added for both developers.
- `docs/MASTER_PROMPT.md` is a byte-for-byte copy of John's pasted attachment. Do not silently rewrite it as requirements evolve.

## Original requirements to preserve unless explicitly revised

Python 3, Linux terminal, standard libraries, two separate LAN clients and one authoritative server. Server on Computer A alongside Player 1, Player 2 on Computer B. JSON over framed TCP, server validation, per-player views, movement/collision, object inspection, cooperative puzzles, three-map campaign, synchronized transitions, recovery, and one timed gas event. Approximately 25-35 minutes of gameplay. Eight-week original development horizon.

Map 1 has painting code `246` and the final-digit mathematical hatch code `259`. Map 2 is the Observation Wing with statue/hidden-passage and calibration puzzles. Map 3 is Containment with valve investigation, approximately three-minute gas incident, checkpoint recovery, and approximately eight-second final activation. Chat and advanced inventory/reconnection are optional.

## Observed implementation, not target behavior

The inherited code is a compact single-map, eight-task variant with peer-to-peer completion messages, client-side validation and answer keys, a local room-switching mode, and chat. It uses math code `1259`. It has no authoritative server, Map 2/3 campaign, gas emergency, or final timed synchronization.

The inherited `IMPLEMENTATION_PLAN.md` has four grading phases, four unnamed team roles, and October 26-November 25 check-ins. John confirmed those grading dates on October 23, so the calendar difference from the eight-week prompt is now an explicit requirement revision. Four-person staffing and the reduced campaign remain unresolved deviations. The inherited plan is a historical reference, not authorization for further scope changes.

Confirmed grading schedule: Dev Week 1, Oct 26/28, 20%; Dev Week 2, Nov 4/9/11, 50%; Dev Week 3, Nov 16/18, 70%; Dev Week 4, Nov 23/25, 100%. These are grading targets, not measured completion. Dev Week 2 spans more than one calendar week.

## Review results and limits

- `REPOSITORY_AUDIT.md` contains findings, source references, and current verification.
- Windows Python 3.14.3: native test discovery ran 15 real movement/map tests successfully; two other test modules could not import because `_curses` is missing. This is an environment/test-coupling limitation.
- Controlled headless run: all 33 existing tests passed with an in-memory replacement of `client.screens` supplying only its real `ART_DIR` path. Actual game rules, sockets, and assets were used. Terminal rendering was not executed.
- Additional probes confirmed forged completion/escape acceptance, unhandled malformed message schemas, loading of both rooms' answers, correct fragmented/coalesced line reconstruction, and malformed JSON causing `bye`.
- Interactive Linux/macOS rendering, resize, real two-process gameplay, two-computer LAN, timing, and full playthrough were not verified. WSL lists only Docker Desktop, not a general Linux development distribution.
- Original reports assert completed future weeks and contain blank evidence fields. They are marked unverified imported drafts. Do not submit them as history.

## Authorized work and next decision

Authorized now: clone, inspect, compare against the prompt, save memory, add instructions, provide a learning guide and proposed two-developer roadmap, and correct documentation status labels. Runtime code and content remain unchanged.

`LEARNING_ROADMAP.md` is a proposal awaiting approval. Start with baseline explanation and tests; then approve one networking step. Do not rebuild the whole game in one task.

Blocking before substantial implementation: approve the first narrow phase and the proposed architecture correction. Keep the original campaign as the target unless an explicit reduced scope is approved. The professor's calendar has been confirmed; the implementation roadmap maps that calendar to small learning steps and calls out feasibility risks. No deadline was invented during this review.

## Working document map

| Need | Read |
|---|---|
| Shared rules | `../AGENTS.md` |
| Original requirements | `MASTER_PROMPT.md` |
| Deviations and verified baseline | `REPOSITORY_AUDIT.md` |
| Proposed phases, weekly milestones, function-sized steps | `LEARNING_ROADMAP.md` |
| Existing files/functions explained | `CODE_GUIDE.md` |
| Per-feature teaching record | `FEATURE_NOTE_TEMPLATE.md` |
| Genuine weekly evidence | `PROGRESS_REPORT_TEMPLATE.md` |

Record later approvals here with date, approver, exact step/scope, and acceptance criteria. No implementation approval has been recorded as of this review.
