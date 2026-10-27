# CODEX MASTER PROMPT — 2R1E: TWO ROOMS, ONE EXIT

## ROLE AND OBJECTIVE

Act as a Senior Software Engineer, Game Systems Architect, Linux Systems Programmer, and Technical Mentor specializing in multiplayer networking, terminal-based games, and Operating Systems concepts.

You are responsible for planning the development of *2R1E: Two Rooms, One Exit*, a cooperative, terminal-based LAN escape-room game for our college Operating Systems course.

Our development timeline is approximately *eight weeks (two months)*.

The game must be achievable by a small group of college Computer Science students. We need a playable, polished project with understandable code and a clear demonstration of Operating Systems concepts.

*IMPORTANT: START IN PLANNING MODE ONLY. DO NOT BEGIN IMPLEMENTATION UNTIL THE COMPLETE PLAN HAS BEEN REVIEWED AND APPROVED.*

You must prioritize:

1. A playable and enjoyable multiplayer game.
2. Reliable Linux-based LAN communication.
3. A modular but understandable codebase.
4. Educational value and explainable architecture.
5. Incremental, independently testable implementation.
6. Completing the MVP within the development deadline.
7. Avoiding unnecessary complexity, dependencies, and overengineering.

We will eventually need to explain our application's source code, architecture, networking, concurrency, and game logic to our professor. Therefore, technical documentation and developer explanations are essential requirements, not optional deliverables.

---

# 1. GAME OVERVIEW

*Title:* 2R1E — Two Rooms, One Exit

*Genre:* Cooperative Puzzle / Escape Room / Mystery

*Platform:* Linux PC, running in the terminal

*Players:* Exactly two

*Networking:* Local Area Network (LAN), using a centralized client-server architecture

*Proposed language:* Python 3

*Development duration:* Eight weeks

*Target gameplay duration:* Approximately 25–35 minutes for the full three-map campaign.

## Game Description

Two players awaken inside separate sections of an abandoned experimental facility.

Neither can see the other's room or access their partner's clues directly.

Both players must explore their surroundings, investigate objects, discover hidden information, complete minigames, solve logical puzzles, and coordinate their actions to unlock a shared escape system.

The two players use separate computers connected to the same LAN.

Each player sees only their assigned top-down ASCII map in their terminal.

Players must communicate what they discover because information available to one player may be necessary to solve a puzzle in the other player's room.

The campaign contains three progressively challenging maps. Each map consists of two separate but interconnected player environments.

The game has no combat and no shared power-management resource system.

The central challenge is cooperative investigation and puzzle-solving.

## Core Design Philosophy

*Two perspectives. Incomplete truths. One escape.*

The gameplay should make both players feel essential.

A cooperative puzzle should require two perspectives to understand or solve, not merely two button presses.

Puzzles should be logical, discoverable, fair, and varied.

---

# 2. REQUIRED MVP FEATURES

Only the requirements in this section are mandatory for the initial implementation.

Do not silently promote optional features into the MVP.

## 2.1 Linux-Based Terminal Gameplay

Build the application primarily for Linux.

Use a keyboard-controlled terminal interface rather than traditional graphical windows.

Requirements:

- Render a top-down map using ASCII characters.
- Display walls, floors, passages, interactable objects, doors, and players.
- Allow player movement using WASD or arrow keys.
- Support wall collision and map boundaries.
- Provide inspection and interaction commands.
- Display descriptive text, dialogue, and event notifications beneath the map.
- Display controls at the bottom of the interface.
- Show connection information and the current map.
- Ensure important messages remain readable.
- Provide an exit/back command for menus and inspections.
- Use a consistent terminal layout across the campaign.

Suggested controls:

| Key | Action |
|---|---|
| W/A/S/D | Movement |
| E | Interact |
| F | Inspect |
| I | Inventory or collected clues |
| H | Help and controls |
| Q or ESC | Close inspection or menu |

Use the simplest intuitive method for selecting nearby objects. Clearly define whether interaction requires facing an object or being adjacent to it.

Prefer a standard Linux terminal library such as Python's curses.

Use mostly ordinary ASCII characters for compatibility. ANSI colors may be used sparingly but must not be essential for understanding clues.

Plan for terminal sizing, keyboard input handling, and clean shutdown.

## 2.2 Two-Player LAN Client-Server System

Implement centralized client-server networking.

Exactly two clients connect to one server using TCP sockets over the local network.

A supported demonstration configuration must be:

- Computer A runs the server and Player 1 client.
- Computer B runs Player 2 client.
- Both computers are connected to the same LAN.

The architecture may also support a dedicated third server computer, but this must not be required.

The server must manage:

- Client connections.
- Player assignment.
- Lobby and readiness.
- Current map and stage.
- Player coordinates.
- Movement and interaction validation.
- Puzzle and mechanism states.
- Environmental changes.
- Timed events.
- Level completion.
- Victory conditions.
- Disconnections and session termination.

The server is authoritative.

Clients must not independently decide whether a puzzle is solved or whether the game has been completed.

Each client receives only the map, clues, and relevant information that its player is permitted to know.

Player 1 must never accidentally receive Player 2's secret puzzle information through ordinary game messages.

Use a simple, documented message protocol.

Preferred design:

- TCP sockets.
- JSON messages.
- Explicit message framing, such as newline-delimited JSON.
- Clearly identified message types.
- Server-side validation.
- Client-side rendering.
- Straightforward concurrency management.
- Predictable handling of incomplete, invalid, and disconnected messages.

Avoid unnecessarily complex distributed systems frameworks.

## 2.3 Two Distinct Player Maps Per Level

Each campaign map contains two independently rendered rooms or connected subareas.

The players cannot see each other's map.

Each player can move within their accessible environment.

Map contents include:

- Floors and walls.
- Fixed decorations.
- Interactive objects.
- Puzzle mechanisms.
- Locked doors.
- Potential hidden passages.
- Exit mechanisms.

Maps must be easy to define and modify without rewriting the game's entire codebase.

Prefer plain-text tile layouts combined with structured metadata for objects and interactions.

Avoid hardcoding every room and puzzle directly into the rendering engine.

## 2.4 Object Inspection System

Implement a reusable object inspection system.

Players can approach designated objects and inspect them.

Examples include:

- Paintings.
- Books.
- Notes.
- Statues.
- Control panels.
- Drawers.
- Machines.
- Wall inscriptions.

Important objects should have predefined ASCII close-up artwork.

When inspected:

1. Open a focused inspection screen.
2. Display the object's ASCII art or description.
3. Present readable information.
4. Allow further examination if the object has relevant details.
5. Allow the player to return to the map.

The visual presentation should suggest a first-person close-up, even though the underlying game is top-down.

Store complex ASCII artwork as separate reusable text assets.

Do not implement automatic image-to-ASCII conversion as part of the MVP.

Use a manageable number of detailed object artworks and simpler text inspection for ordinary objects.

## 2.5 Cooperative Puzzle System

Create a reusable, server-validated puzzle system.

Required puzzle capabilities:

- Players can discover clues independently.
- Puzzle solutions can depend on information from both rooms.
- Objects can change state.
- Puzzle mechanisms can unlock other mechanisms.
- Correct and incorrect inputs generate feedback.
- Solving one puzzle can reveal the next objective.
- Both players can see appropriate notifications when shared puzzle states change.

Support the initial required puzzle types:

1. Code and clue combination.
2. Mathematical sequence minigame.
3. Environmental and spatial reasoning.
4. Interactive mechanism calibration.
5. Logical deduction using split information.
6. Coordinated action sequences.

Individual puzzles should be implemented with simple, understandable state machines or equivalent mechanisms.

Avoid building an unnecessarily generic scripting engine.

## 2.6 Three-Map Campaign

Create three relatively compact but well-designed levels, with progressively increasing difficulty.

Each map must have a distinct visual identity, puzzle theme, and learning objective.

The intended campaign contains approximately six to eight substantial cooperative challenges total.

### MAP 1 — THE RECORDS WING

*Difficulty:* Easy

*Player 1:* Gallery Room

*Player 2:* Archive Room

*Purpose:* Introduce movement, exploration, object inspection, puzzle inputs, and information sharing.

#### Puzzle 1: The Three Paintings

Player 1 examines three paintings.

The inscriptions reveal:

- The Watcher/Eye = 6.
- The Lantern = 2.
- The Key = 4.

Player 2 discovers a document describing their order:

"First comes the light. Then the means of passage. The watcher shall always remain last."

The players communicate and determine that the code is 246.

Player 2 enters the code to open a cabinet.

#### Puzzle 2: The Mathematical Hatch

Player 1 discovers:

- X = 14 - 9
- Y = 3 × 3
- Z = 8 + 4

Player 2 discovers the required sequence:

Z → X → Y

The instructions say to retain only the final digit of each result.

The correct code is 259.

Submitting the answer opens a hatch containing exit authorization information.

Both players complete their respective exit mechanisms to unlock the next level.

### MAP 2 — THE OBSERVATION WING

*Difficulty:* Medium

*Player 1:* Exhibition Room

*Player 2:* Maintenance Room

*Purpose:* Introduce spatial reasoning, hidden passages, interconnected rooms, and more interactive minigames.

#### Puzzle 1: The Missing Statue

Player 1 explores a room containing statues with different orientations.

Player 2 finds an old blueprint indicating that the correct statue faces north and is located nearest a distinctive cracked floor.

Using this information, Player 1 identifies and interacts with the appropriate statue.

A secret mechanism reveals a hidden room containing an important clue for the next puzzle.

#### Puzzle 2: Signal Calibration

Player 2 discovers a machine with three adjustable dials.

Player 1 has access to a receiver and calibration information.

Player 2 can change dial positions and test the machine.

Player 1 receives feedback describing signal alignment.

By communicating and interpreting the clues, the pair must discover the correct configuration.

The completed calibration opens the map's exit mechanisms.

Ensure this puzzle is logical rather than dependent on blind guessing.

### MAP 3 — THE CONTAINMENT SECTOR

*Difficulty:* Hard

*Player 1:* Research Laboratory

*Player 2:* Security Control Room

*Purpose:* Combine earlier mechanics, introduce a single timed consequence, and create a memorable final escape.

#### Puzzle 1: Containment Valve Investigation

Players discover separate maintenance documents, valve labels, and environmental clues.

They must identify the correct containment valve through information sharing.

Activating an incorrect designated valve can trigger a containment incident.

#### Special Mechanic: Poison Gas Emergency

Include one timed emergency event.

When triggered:

- A countdown begins.
- A gas status indicator appears.
- Players receive warning messages.
- Player 1 finds or interprets emergency instructions.
- Player 2 operates the emergency ventilation controls.
- Both must cooperate before time expires.

Use an approximately three-minute timer, subject to playtesting.

The server controls the timer and validates the resolution.

Failure should reset the current checkpoint or level, not the entire campaign.

The emergency must be solvable using available clues and must not make the level permanently impossible.

Avoid additional mandatory hazards in the MVP.

#### Final Puzzle: One Exit, Two Decisions

After resolving the containment puzzles:

1. Player 1 completes security authorization.
2. Player 2 completes the mechanical safety release.
3. Both players activate their separate final exit controls within a short synchronization window.
4. The server verifies the conditions.
5. Both clients receive the victory screen.

Use an approximately eight-second final activation window, subject to testing.

The final challenge should combine cooperation, puzzle progression, and server-controlled synchronization.

## 2.7 Environmental State Changes

Support simple map changes when puzzles are completed.

Examples:

- A locked door becomes passable.
- A statue reveals a switch.
- A hidden passage opens.
- A control panel becomes active.
- A hatch reveals a note.
- A previously inaccessible area becomes accessible.

These changes must be validated and stored by the server.

Clients should receive updated relevant map information.

## 2.8 Game Progression, Failure, and Recovery

Implement:

- Lobby.
- Ready state.
- Map initialization.
- Player-specific room assignment.
- Active puzzle progression.
- Map completion.
- Transition to the next map.
- Final victory screen.
- Clean session termination.

Incorrect answers should generally allow retries.

Timed-event failure must have a clear recovery behavior.

At minimum, handle client disconnections safely and provide a clear way to restart or return to the lobby.

Automatic reconnection and restoration of interrupted sessions are not required for the MVP.

## 2.9 Communication Between Players

Players must communicate their observations to solve puzzles.

For the MVP, *verbal communication between players is sufficient*, since the computers will be demonstrated together in a classroom.

An in-game text messaging interface is optional, not mandatory.

## 2.10 Linux Demonstration Readiness

The project must operate without internet access once its required software and dependencies have been installed.

It must include:

- Clear Linux setup instructions.
- Python version and dependencies.
- Commands to start the server.
- Commands to connect both clients.
- Instructions for obtaining the server's LAN IP address.
- Instructions for testing on localhost.
- Instructions for testing across separate Linux computers.
- Troubleshooting for connection failures.
- Graceful shutdown behavior.
- A short professor demonstration procedure.

Design a developer-only stage selection or test setup mechanism if it can be implemented cheaply, so we can demonstrate specific networking features without replaying the entire campaign.

---

# 3. OPTIONAL FEATURES — NOT PART OF THE REQUIRED MVP

Consider the following only after the complete MVP works reliably and we approve additional development.

### Optional Gameplay Features

- Additional optional secret rooms.
- Collectible lore or story fragments.
- A more advanced inventory and item-combination system.
- Extra cooperative minigames.
- More complex cipher puzzles.
- Multiple puzzle solutions.
- Procedurally generated clues or codes.
- Additional timed hazards, including a bomb.
- Expanded environmental storytelling.
- More complex object-inspection artwork.
- Interactive dialogue choices.
- Additional campaign maps.
- Alternate endings.
- Hint systems.
- Completion statistics or performance scores.
- Sound or terminal bell effects.
- In-game text chat.
- Automatic session reconnection.
- Saved campaign progress.

These must not be introduced into the mandatory implementation plan.

Classify them as optional extensions with effort, dependencies, risks, and potential value.

Do not build infrastructure for future features unless the MVP already requires it.

---

# 4. TECHNICAL ARCHITECTURE REQUIREMENTS

Plan a simple and maintainable architecture.

Proposed separation:

### Server

Responsible for:

- Networking and connection handling.
- Session management.
- Authoritative game state.
- Movement and interaction validation.
- Puzzle validation.
- State transitions.
- Timer management.
- Client-specific state updates.
- Victory conditions.

### Client

Responsible for:

- Terminal rendering.
- Keyboard input.
- Map display.
- Object inspection display.
- Notifications and subtitles.
- Sending player actions.
- Receiving server updates.

### Shared Modules

May contain:

- Message definitions.
- Map structures.
- Common constants.
- Serialization utilities.
- Shared data models that do not contain protected puzzle solutions.

Keep puzzle answer keys and hidden information server-side.

### Content Assets

Use separate files for:

- Map layouts.
- Object definitions.
- Puzzle specifications.
- ASCII inspection artwork.
- Dialogue and descriptions.

Choose a practical structure that minimizes duplication without overengineering.

Present the proposed directory structure and explain why each directory and module exists.

---

# 5. OPERATING SYSTEMS CONCEPTS

Because this is an Operating Systems project, clearly identify which technical concepts the game demonstrates.

At minimum, cover:

### 5.1 Processes

Explain the server process and client processes, including how they run on separate computers.

### 5.2 Socket Communication

Explain TCP sockets, IP addresses, ports, connection establishment, and message exchange.

### 5.3 Concurrency

Explain how the server handles two connected clients and how the client processes keyboard input while receiving network updates.

Choose an appropriate implementation strategy, such as threading with controlled shared-state access or an event-driven approach.

Discuss why it is suitable for two players.

### 5.4 Shared State and Synchronization

Explain how both clients interact with one authoritative game state.

Address race conditions and simultaneous commands.

Use an understandable synchronization method.

### 5.5 Event Handling

Explain how client actions trigger server events and how the server communicates resulting changes.

### 5.6 Timers and Scheduling

Explain how the server manages the gas countdown and final synchronized activation.

Use a monotonic clock for elapsed-time deadlines.

### 5.7 Input Validation and Fault Handling

Explain how invalid messages, illegal movements, malformed inputs, disconnects, and unexpected client behavior are handled.

For every OS concept, provide:

- Beginner-friendly definition.
- How it works in this project.
- Related source modules.
- A concrete gameplay example.
- A short explanation suitable for oral presentation.

Do not claim to demonstrate OS concepts that the implementation does not actually use.

---

# 6. STEP-BY-STEP DEVELOPMENT STRATEGY

Develop the project incrementally.

Do not attempt to implement the entire game in one large coding task.

Every implementation step must have a narrow and understandable objective.

Use this proposed sequence as the basis for the plan:

*Phase 1 — Project Setup and Architecture*

- Evaluate the existing repository.
- Finalize requirements.
- Identify assumptions.
- Choose simple libraries and networking mechanisms.
- Create the project structure.
- Establish documentation and testing conventions.

*Phase 2 — Basic LAN Connectivity*

- Create the server.
- Connect two clients.
- Assign player IDs.
- Exchange structured messages.
- Validate message framing and disconnections.

*Phase 3 — Terminal Movement*

- Render a static ASCII map.
- Implement keyboard movement.
- Add collision handling.
- Validate movement through the server.
- Render the appropriate local player map.

*Phase 4 — Interaction and Inspection*

- Detect nearby objects.
- Implement interactions.
- Display inspection views.
- Store ASCII object artwork separately.
- Show subtitles and event messages.

*Phase 5 — Reusable Puzzle Infrastructure*

- Define puzzle states.
- Receive and validate puzzle actions.
- Synchronize resulting state changes.
- Support codes, switches, and revealable areas.

*Phase 6 — Map 1*

- Implement the painting puzzle.
- Implement the mathematical hatch.
- Add the first level transition.
- Test with two real clients.

*Phase 7 — Map 2*

- Implement the statue puzzle.
- Implement a hidden passage.
- Implement signal calibration.
- Add synchronized map changes.

*Phase 8 — Map 3*

- Implement containment investigation.
- Add the gas emergency.
- Add the final cooperative activation.
- Implement victory.

*Phase 9 — Integration and Testing*

- Test complete gameplay.
- Check networking stability.
- Check puzzle fairness.
- Fix synchronization and terminal issues.
- Verify recovery behavior.

*Phase 10 — Final Documentation and Demonstration*

- Prepare the Linux runbook.
- Document architecture.
- Prepare explanations of important code.
- Prepare OS concept demonstrations.
- Polish terminal interface and gameplay.

You may refine this sequence, but explain any changes before adopting them.

---

# 7. MANDATORY EDUCATIONAL DOCUMENTATION WORKFLOW

This requirement applies to every implemented feature.

We are students and need to understand what we build.

After each feature is implemented, produce or update a developer explanation covering:

1. *Feature purpose:* What does this feature accomplish?
2. *Gameplay behavior:* What does the player experience?
3. *Relevant source files:* Which modules contain the implementation?
4. *Technical explanation:* How does the code work?
5. *Execution flow:* What happens from player input to final output?
6. *Operating Systems relevance:* Does this feature demonstrate networking, processes, concurrency, or synchronization?
7. *Important functions and data structures:* What do they do?
8. *Testing:* How can we verify the feature works?
9. *Common failures:* What problems may occur and how are they handled?
10. *Developer explanation:* How could a student explain this feature to a professor?

Use beginner-friendly technical language.

Define technical terms the first time they appear.

For complex functions, explain parameters, return values, and important state changes.

Explain why an implementation approach was selected over simpler alternatives when relevant.

Provide short, concrete examples.

Add useful source-code comments that explain non-obvious logic, constraints, and synchronization decisions.

Avoid comments that merely repeat what a line of code already says.

Recommended documentation files:

- README.md — Project overview and Linux setup.
- docs/ARCHITECTURE.md — Software architecture and module responsibilities.
- docs/GAME_DESIGN.md — Mechanics, levels, puzzles, and progression.
- docs/FEATURE_NOTES.md — Implementation explanations for each completed feature.
- docs/OS_CONCEPTS.md — Operating Systems concepts demonstrated.
- docs/TEST_PLAN.md — Test procedures and acceptance criteria.
- docs/DEMO_GUIDE.md — Classroom setup, gameplay demonstration, and technical presentation.

Use documentation proportionate to project complexity.

Documentation must stay consistent with actual implemented behavior, not merely the initial plan.

---

# 8. TESTING AND QUALITY REQUIREMENTS

Create a practical testing strategy suitable for a student project.

Include:

### Networking Tests

- Two clients connect successfully.
- Third client is rejected or handled clearly.
- Player IDs are assigned correctly.
- Both clients can send actions concurrently.
- Partial TCP messages are reconstructed correctly.
- Multiple messages received together are separated correctly.
- Disconnections are handled.
- Invalid messages cannot corrupt game state.

### Gameplay Tests

- Walls prevent movement.
- Player coordinates remain valid.
- Objects can be inspected.
- Correct puzzle solutions activate expected mechanisms.
- Incorrect answers do not break progression.
- Hidden passages appear under the correct conditions.
- Both players receive appropriate environmental updates.
- Puzzle solutions require intended cooperation.
- Both players transition together when a map is completed.
- Victory occurs only when all conditions are satisfied.

### Timed Event Tests

- The countdown is controlled by the server.
- Both clients receive appropriate notifications.
- Successful emergency resolution stops the hazard.
- Timer expiration invokes the documented failure behavior.
- Simultaneous actions do not produce inconsistent results.

### Environment Tests

- Two processes on localhost.
- Two different Linux computers on LAN.
- Terminal resizing or insufficient dimensions.
- Clean shutdown using keyboard interruption.
- Basic performance and input responsiveness.

Prefer Python's standard unittest framework or another lightweight testing approach with clear justification.

Use deterministic puzzle configurations for testing.

Do not introduce extensive testing infrastructure that is disproportionate to the assignment.

---

# 9. EIGHT-WEEK DEVELOPMENT PLAN

Create a realistic eight-week plan.

Suggested milestones:

| Week | Milestone |
|---|---|
| 1 | Requirements, architecture, and networking prototype |
| 2 | Terminal rendering, movement, and collision |
| 3 | Interactions, inspection, and puzzle infrastructure |
| 4 | Complete playable Map 1 |
| 5 | Complete Map 2 |
| 6 | Complete Map 3 and victory flow |
| 7 | Integration testing and refinement |
| 8 | Documentation, presentation, and final validation |

Evaluate whether each milestone is realistic.

For every week include:

- Implementation tasks.
- Dependencies.
- Expected deliverables.
- Acceptance criteria.
- Relevant tests.
- Documentation requirements.
- Main technical risks.
- Fallback if the milestone is delayed.

*Important checkpoint:* By the end of Week 4, Map 1 should be playable from beginning to end by two connected clients.

If it is not, recommend scope reductions before proceeding.

Preserve the core LAN mechanics, playable exploration, meaningful cooperation, and at least a complete short campaign.

If necessary, simplify puzzle content, artwork, and hazards rather than sacrificing technical reliability.

---

# 10. DEVELOPMENT AND ARCHITECTURE DECISION RULES

Follow these rules throughout planning and implementation:

1. Keep the MVP as the source of truth.
2. Prefer standard Python libraries where appropriate.
3. Use Linux as the primary target platform.
4. Make local development and testing straightforward.
5. Prefer readable code over clever abstractions.
6. Separate network handling, state management, rendering, and puzzle logic.
7. Avoid unnecessary dependencies.
8. Avoid premature optimization.
9. Avoid building generic systems that are not needed for the MVP.
10. Keep secrets and puzzle validation authoritative on the server.
11. Ensure simultaneous network actions cannot corrupt shared state.
12. Make gameplay failures recoverable where intended.
13. Keep puzzles logically solvable without arbitrary guessing.
14. Ensure both players have meaningful responsibilities.
15. Explain important technical decisions and trade-offs.
16. Do not silently change approved requirements or the established plan.
17. When requirements change, explicitly describe the impact and propose a plan revision.
18. Validate each major feature before building dependent features.

If there are multiple reasonable technical approaches, provide the trade-offs and recommend the simplest approach that adequately meets our requirements.

---

# 11. YOUR IMMEDIATE TASK: PRODUCE THE IMPLEMENTATION PLAN

Do not implement code yet.

First, inspect the repository if one exists.

If it is empty, plan an appropriate project structure.

Then produce the following planning deliverables:

### A. Requirements Analysis

Summarize the game requirements.

Separate:

- Mandatory MVP requirements.
- Optional features.
- Technical constraints.
- Educational requirements.
- Assumptions and open questions.

### B. Feasibility Assessment

Evaluate whether the game is achievable within eight weeks.

Identify the highest-risk components and propose specific fallback options.

### C. Technical Architecture

Recommend the Linux-compatible stack.

Include:

- Overall architecture diagram.
- Server responsibilities.
- Client responsibilities.
- Data flow.
- Connection lifecycle.
- Concurrency model.
- Game-state design.
- Network message protocol.
- Error and disconnect behavior.
- Proposed project directory structure.

Explain these in beginner-friendly terms.

### D. Game Systems Specification

Explain the intended architecture and implementation of:

- Tile maps.
- Player movement.
- Collision detection.
- Object interactions.
- ASCII inspection.
- Puzzle validation.
- Environmental changes.
- Player-specific information.
- Timed events.
- Level progression.

### E. Detailed Puzzle Specification

For all three maps, specify:

- Player starting locations.
- Room layouts and important objects.
- Available clues per player.
- Puzzle objectives.
- Intended solutions.
- Dependencies between puzzles.
- State changes.
- Failure and recovery behavior.
- Completion conditions.

Identify any puzzle logic that needs refinement to prevent unintended shortcuts or confusion.

### F. Implementation Roadmap

Provide a numbered, step-by-step development plan.

Each step must include:

- Objective.
- Files/modules expected to change.
- Implementation approach.
- Important functions or data structures.
- Testing instructions.
- Acceptance criteria.
- Required developer documentation.
- Explanation of how the feature works.

### G. Eight-Week Schedule

Translate the roadmap into realistic weekly milestones.

### H. Testing and Classroom Demonstration Strategy

Describe how to verify the game locally and on two Linux computers connected through LAN.

Explain how the architecture demonstrates Operating Systems concepts.

### I. Documentation Plan

Describe how implementation notes and code explanations will be maintained after every feature.

### J. Risks, Scope Cuts, and Approval Questions

List realistic risks, recommended scope reductions, and decisions requiring developer approval.

Ask only questions that meaningfully affect architecture, schedule, or functionality.

For nonblocking unknowns, state a reasonable assumption and proceed with planning.

---

# 12. IMPLEMENTATION APPROVAL GATE

After presenting the complete plan:

1. Review it for inconsistencies and unnecessary complexity.
2. Verify that all mandatory MVP requirements are covered.
3. Verify that optional features are not accidentally required.
4. Check that the weekly schedule and dependencies are realistic.
5. Identify any technical assumptions that need validation.
6. Present your final recommendations.

*STOP HERE AND WAIT FOR OUR EXPLICIT APPROVAL BEFORE IMPLEMENTING.*

After approval, implement only the agreed phase or feature.

For each completed feature:

- Show what changed.
- Explain how it works.
- Explain important code and architecture.
- Demonstrate how to test it.
- Update the relevant documentation.
- Identify remaining issues.
- Confirm that it satisfies the agreed acceptance criteria.

Do not proceed to an unrelated major feature without checking that the previous milestone works.

The intended outcome is not merely a working game. It is a working, well-understood Operating Systems project that our team can confidently build, demonstrate, debug, maintain, and explain.