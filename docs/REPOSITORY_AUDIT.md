# Repository audit against the master prompt

Reviewed October 23, 2026. Baseline `6bfedc825c126c387559d62646a7ba79f5789163`. This report describes the inherited code; the review changes documentation only.

## Overall finding

The repository is a useful, compact single-map prototype, but it does not implement the master campaign or its authoritative networking architecture. Existing tests mainly validate the repository's alternate design. Passing them does not establish conformance to the master prompt.

John Francis Espiritu subsequently confirmed the repository's grading dates. The four-phase calendar therefore replaces the original eight-week horizon by explicit instruction. Team size is two. Other architecture/content changes have not been approved in this conversation, and no prior approval record was found in the cloned files. Absence of a record does not prove that no discussion happened elsewhere.

## Requirement comparison

Source line references below refer to the reviewed commit before documentation banners were added.

| Master requirement | Observed implementation and evidence | Assessment |
|---|---|---|
| Linux terminal, Python, ordinary ASCII, standard libraries | `main.py:6-21`, `client/ui.py`, text maps and art, no external runtime dependencies declared | Stack aligned; Linux execution unverified |
| WASD/arrows, collision, facing/interaction, inspection, messages, help/back | `game/game_map.py:71`, `game/session.py`, `client/game_screen.py:138`, `client/screens.py`, `client/menu.py` | Substantially present; UI needs live acceptance |
| Connection information and current map visible | Host lobby shows guessed IP; main status shows LAN/local and player, panel shows room, not full server/map details | Partial |
| Exactly two clients connected to one authoritative server | Host accepts one peer; after connection both independently validate their own room and maintain a `Team` | Major architectural deviation |
| Server assigns player IDs; lobby/readiness | IDs chosen from host/join menu in `client/app.py:23-25`; connection immediately starts play, no readiness exchange | Missing required lifecycle |
| Server validates movement, interactions, answers, completion and victory | `RoomSession.move`, `client/tasks.py:257-280`, `client/game_screen.py:171-188`, `Team.apply` operate locally | Major authority gap |
| Per-player permitted information, protected answer keys | `load_level()` loads both room object files; client task reads `answer` | Boundary not implemented |
| TCP + JSON with framing | `NetLink.read_loop`, `send` use JSON lines; actual fragmented/coalesced reads passed controlled probes | Transport aligned |
| Invalid/disconnected messages handled safely | Invalid JSON becomes `bye`; valid JSON with invalid structure reaches unchecked consumers | Partial; crash cases confirmed |
| Two distinct rooms per map | Records Wing contains Gallery and Archive | Present for Map 1 only |
| Plain text maps + object metadata + separate ASCII close-ups | `assets/maps/records_wing/`, `assets/art/`, loader checks | Aligned and reusable |
| Paintings produce code 246 | Archive cabinet uses `246` | Aligned puzzle core |
| Mathematical hatch uses final digit of each result, Z-X-Y => 259 | Gallery display case uses full concatenation `1259`; `tests/test_tasks.py:54` asserts it | Explicit gameplay deviation, including test expectation |
| Three compact levels, approximately 6-8 substantial cooperative challenges, 25-35 minutes | One level with eight tasks, several solo; docs target 10-15 minutes | Campaign/content and pacing deviation; duration not playtested here |
| Map 2 Observation Wing: statue, hidden passage, signal calibration | No implementation; concept instead proposes optional Boiler Floor | Missing and substituted concept |
| Map 3 Containment: valves, gas, recovery, final synchronized exit | No implementation or assets | Missing |
| Environmental map changes managed by server | Local completed-task colors and exit eligibility; no changing walkable passages/hatches | Partial visual state only |
| Both players transition through campaign, then final victory | `START_LEVEL` fixed to Records Wing, local team win ends session | Single-map victory only |
| Monotonic server countdown, roughly 180 s gas and 8 s final window | No required timers; client transfer/escape time uses `time.time()` | Missing required timer/scheduling demonstration |
| Incorrect answers retry; disconnect safely returns to lobby/menu | Local keypad retries, `bye` noticed in main loop; modal screens defer messages | Partial |
| Verbal communication sufficient; chat optional | Chat, journal clue sharing and local Tab mode exist | Extra scope; not a substitute for required campaign |
| Linux runbook, server/client startup, localhost and two-Linux-machine demo | README/manual describe peer Host/Join, no standalone authoritative server instructions; Linux/LAN checks blank | Incomplete for original architecture |
| Beginner notes per feature/function and actual OS concepts | Module comments and overview docs exist; feature notes and systematic walkthrough absent at baseline | Educational workflow incomplete |
| Eight-week plan | Four grading phases in inherited docs | Resolved calendar revision: John confirmed the four phases |
| Current team has two developers | Plan/credits/reports contain four unnamed roles | Documentation staffing deviation; current instructions corrected |
| Incremental validated development with approval | One fetched commit contains the implementation and reports; no approval/step history available | Incremental process cannot be established from this history |

## Findings requiring implementation approval

### A1. A peer can declare success without performing the actions

`client/game_screen.py:177-183` marks completion locally and sends an outcome. `game/team.py:37-42` trusts it. A controlled direct probe applied every task ID and two escape messages to a fresh team; `has_won()` became true without moving, inspecting, solving, or checking a player's room. A peer can claim the other player's tasks or escape identity.

This contradicts master sections 2.2, 4 and 5.4. Correct the shared authority boundary, rather than adding isolated checks to each mini-game. Proposed correction: clients send action requests; a small server validates legal stage, player ownership, position and answer, then publishes approved outcomes. This is consequential work and is not implemented by this audit.

### A2. Both rooms and answer keys are in each client's loaded level

`client/app.py:37` loads the full level. `game/level.py:76-81` loads both rooms and their object metadata. `client/tasks.py:258` obtains the answer for local comparison. A probe confirmed answers `g_code=1259` and `a_code=246` in the loaded data.

This is observed local access, not proof that ordinary network messages automatically send every secret to the partner. The standard message list contains outcomes/shared clues. Nevertheless, server-only validation and restricted client views are absent. A future client view must exclude answers and the partner's unrevealed content. Source files in a public coursework repository will remain readable by developers; do not claim runtime separation hides publicly published solutions from someone browsing the source.

### A3. JSON parsing is not message validation

`client/network.py:54-55` enqueues any decoded JSON. `client/game_screen.py:117` and `game/team.py:39-42` assume dictionaries and required keys. Controlled probes produced:

| Input to `Team.apply` | Result |
|---|---|
| `{}` | Unhandled `KeyError` |
| `[]` | Unhandled `TypeError` |
| `{"type":"task"}` | Unhandled `KeyError` |
| `{"type":"task","id":[]}` | Unhandled `TypeError` |

An actual socket probe sent `[]\n`, and the inbox returned a list rather than rejecting it. No message-length or queue bound exists in this path. A faulty or unexpected peer can crash message processing or accumulate memory. Add a small explicit validator at the future server/client input boundaries; distinguish malformed JSON, invalid message shape and illegal gameplay actions.

### A4. Modal screens stop applying network events

The main loop calls `handle_messages` at `client/game_screen.py:84`. Inspection, journal, help, chat input and mini-games run separate keyboard loops. `wait_for_key` blocks at `client/screens.py:24-30`; `run_task` transfers control to a mini-game. The socket thread can keep queuing messages, but the main loop cannot apply them until the modal function returns.

This is a source-derived finding, not a live curses reproduction. The inherited claim that updates always apply every 0.1 seconds is too broad. It matters especially before adding gas timers. Keep authoritative time advancing on the server and process client updates in every screen through a small common polling approach. Do not introduce a UI framework to solve it.

### A5. Progress narratives and verification claims lack historical evidence

The fetched history contains one commit dated October 23, 2026, 01:10:06 +08:00. All four reports are already present. Week 1 claims no code was written through October 28; Week 4 claims 100%, final packaging, and clean-folder testing through November 25. Those dates were future dates when reviewed. Blank names, hours, signatures and playtest timings remain.

The Week 2 report refers to `tests/test_network.py`, which does not exist; socket tests are in `tests/test_tasks.py`. Week 4's own environment checklist leaves Linux and separate-computer LAN unchecked. Its submission zip claim is not substantiated by a tracked zip; an external/ignored artifact may exist but was not verified.

Treat these files as imported draft narratives. Labels were added while preserving their text. This review cannot establish how long development took outside Git and does not infer misconduct. It also cannot convert inherited code into evidence of a student's weekly implementation or comprehension.

The commit names John Paul S. Flores and includes a Claude Opus 5.5 coauthor trailer. Preserve this attribution. Going forward, describe the code as an inherited AI-assisted baseline where applicable, and document the students' actual implementation, debugging, review and learning. Removing attribution or manufacturing weekly history would not demonstrate understanding.

## What is worth keeping

The architecture is small enough to study: production modules are 1-344 lines, with the two largest mixing game-loop/drawing and mini-game input/rendering. This is not a reason for a broad rewrite. Plain dictionaries, short rule methods, text/JSON content, separate art, `unittest`, explicit facing selection and a thread-safe receive queue are useful foundations.

Map validation, collision, inspect-versus-use behavior, duplicate clue prevention, task prerequisites and retryable inputs have meaningful checks. Reuse these after understanding them. Move authority only as required; do not build a generic puzzle interpreter, dependency graph framework, database, web service or external networking stack.

## Verification performed on October 23

| Check | Observed result | Limit |
|---|---|---|
| Git baseline, files, branches and history | One main/origin-main commit; clean initial checkout | Does not establish off-repository history |
| Original prompt preservation | SHA256 matches attachment: `2354389408841EE5F6A54F2C1D5C65221689EE1DE0724FFFC8B6E3EF169D8CDA` | Confirms exact copy only |
| Native `python -m unittest -v`, Windows Python 3.14.3 | 15 movement/map tests pass; two test modules cannot import `_curses`; runner reports 17 entries including import failures | Expected platform limitation; no claim of native Windows support |
| Controlled headless existing suite | 33/33 pass; only `client.screens` replaced in memory with a module exposing the real asset directory constant | Not an unchanged native suite run; no terminal/UI code executed |
| Actual `NetLink` socket-pair probes | Fragmented JSON line waits for completion; two coalesced lines delivered separately; invalid JSON emits `bye` | One-process local transport probe, not two-computer LAN |
| Valid-JSON invalid-schema probe | `[]` delivered unchecked; direct consumers raise errors on missing/wrong fields | Confirms fault, not a remediation test |
| Forged completion/escape probe | Fresh team reaches win without gameplay | Confirms missing authority |
| Content probe | Both rooms and both code answers loaded | Does not prove automatic network leak |
| Independent read-only reviewer | Agreed on authority, secrets, validation, modal update, campaign and reporting findings | Additional code review, not manual gameplay |

The headless test command created an in-memory `client.screens` module with `ART_DIR = Path.cwd() / 'assets' / 'art'`, inserted it into `sys.modules`, then called normal unittest discovery. That bypassed only the tests' import-time dependency on curses for a path constant. No repository test or runtime file was edited. The tests use real rules, assets and localhost sockets. Re-run the ordinary command on supported Linux before claiming the standard suite passes there.

Not performed: installs, interactive curses gameplay, Linux/macOS rendering, terminal resizing, a real two-process game session, two Linux computers over LAN, campaign playthrough, measured solve time, packaging acceptance, or runtime fixes. WSL inventory shows Docker Desktop only. No general Linux environment was installed or configured.

Final documentation checks: local Markdown links resolve; all 152 current Python function/method/callback definitions have named coverage in the code guide, with the guide also reviewed against source; the four original report narratives are preserved beneath added warnings; there is one root `AGENTS.md`; and tracked runtime, tests, assets and ignore configuration match HEAD under Git's checkout normalization. A second documentation review found one incorrect `Game.use` reference in the roadmap, corrected to module-level `client.game_screen.use`. No gameplay change was made. The named-coverage check alone does not establish the quality of every explanation or student comprehension.

## Recommended next decision

Approve the roadmap's Phase 1 learning/baseline work first. Before Phase 2, approve the small authoritative-server contract. Use the confirmed grading dates, retain the master campaign as the unmodified target, and explicitly decide scope at the listed feasibility gates. No implementation was approved by producing this audit.
