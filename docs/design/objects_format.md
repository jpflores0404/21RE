# Object Data Format

**Task 1.12 · Owners: M1 + M3 · Due: Oct 28**

Each room has `<room>_objects.json`: a list of objects.

```json
{
  "id": "g_code",
  "name": "Display Case",
  "at": [[46, 12]],
  "kind": "task",
  "task": "code",
  "title": "Unlock Display Case",
  "text": ["A glass display case with a 4-digit keypad."],
  "answer": "1259",
  "hint": "Use the curator's note. Your partner knows the order."
}
```

| Field | Used by | Meaning |
|---|---|---|
| id | all | unique name |
| name | all | shown in the hint line and dialogue title |
| at | all | list of `[x, y]` tiles (a 2-tile desk has 2 entries) |
| kind | all | `examine`, `container`, `task`, `exit` |
| text | all | lines shown when pressing E |
| art / inspect / clue | optional | close-up picture, close-up lines, journal clue |
| item / after / reveal_art | container | item given, text once empty, art once opened |
| task / title | task | mini-game type and the name in the task list |
| answer / hint | code task | keypad answer and hint |
| needs_item / needs_task / locked_text | task | what is required first, and what to say if it is missing |
| seconds | download/upload | how long the transfer takes |
