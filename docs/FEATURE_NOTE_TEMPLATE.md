# Feature explanation: [feature name]

Status: draft / implemented / tested / reviewed / both developers demonstrated understanding.

Approval: [who approved which roadmap step and when]. Implementer: [name]. Reviewer: [name]. Actual dates: [dates].

## What the feature does

Explain its purpose and what the player sees. State what was already in the inherited baseline and what this change adds or fixes.

## Concept, then example

Define unfamiliar terms. Start with a small example unrelated to the game, then show the same idea in this feature. Ask the learner to predict the result before running it.

## Files and code explained

Account for every added/changed file, including tests and assets.

| File | Why it exists | What changed | Who calls or loads it |
|---|---|---|---|
| [path] | [purpose] | [specific change] | [caller/loader] |

For every added/changed function or method, including callbacks:

### [file: function name]

- Purpose in plain language:
- Parameters, types, and one actual example:
- Return value and meaning, including `None` or failure:
- State before and after:
- Important statements/blocks in execution order, with branch and loop conditions:
- Callers and functions called:
- Expected errors and recovery:
- Why this approach is simpler or safer than the relevant alternative:
- Test that proves it, plus what the test does not prove:

Explain line by line when requested or when a learner cannot trace a block. Do not replace the real explanation with a pasted function and comments repeating its syntax.

## Trace one player action

Key/input -> message -> authoritative validation -> state change -> permitted update -> drawing. Identify which parts actually exist in this step. Show concrete example values and one rejected input.

## Operating Systems connection

Give the concept's definition, actual source location, a gameplay example, and two sentences suitable for an oral defense. If the feature has no direct OS concept, say so. Distinguish a process, a thread, a queue, a socket, and shared state accurately.

## Testing and failures

Record commands, environment/version, expected and observed results, and actual test date. Include a happy path, relevant boundary/error case, common failure and recovery, and regressions checked. Identify unrun Linux/LAN/UI checks explicitly.

## Let the developer try

1. Predict what [small input] does.
2. Implement [one bounded function or change] after approval.
3. Explain why [edge case] is accepted or rejected.
4. Run [targeted check] and explain the result.

## Teach-back and review

| Developer | Explained execution without reading a prepared answer | Traced valid and invalid inputs | Made a small change | Evidence/date |
|---|---|---|---|---|
| John Francis Espiritu | Pending | Pending | Pending | |
| Partner, name to confirm | Pending | Pending | Pending | |

Unclear concepts, remaining issues, approval deviations, and next small step: [record facts]. Mark completion only after the behavior, checks, notes, and learning criteria have been reviewed.
