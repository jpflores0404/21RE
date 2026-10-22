"""One player's state inside their room, and what E (use) and F (inspect) do.

You always act on the tile directly in front of you (the way you are facing),
like classic top-down RPGs:
    E (use)      read text, open containers, start tasks, use the exit
    F (inspect)  close-up view; reading details here saves clues in your journal
"""
from game.game_map import DIRECTIONS, Player, try_move


class Result:
    """What the screen should show after the player presses E or F."""

    def __init__(self, title, lines, art=None, task=None, escape=False):
        self.title = title
        self.lines = list(lines)
        self.art = art        # name of a close-up picture in assets/art/
        self.task = task      # a task object: the screen should start its mini-game
        self.escape = escape  # True: the player walks out through the exit


class RoomSession:
    def __init__(self, room):
        self.room = room
        self.player = Player(*room.game_map.spawn)
        self.items = []
        self.clues = []
        self.opened = set()  # ids of containers already opened

    def move(self, direction):
        return try_move(self.room.game_map, self.player, direction)

    def faced_position(self):
        dx, dy = DIRECTIONS[self.player.facing]
        return self.player.x + dx, self.player.y + dy

    def faced_object(self):
        return self.room.at.get(self.faced_position())

    def faced_name(self):
        """Name of the thing in front of the player, or None if there is nothing to use."""
        obj = self.faced_object()
        if obj:
            return obj["name"]
        tile = self.room.game_map.tile(*self.faced_position())
        if tile and "about" in tile:
            return tile["name"].title()
        return None

    def add_clue(self, clue):
        """Save a clue in the journal. Returns False if we already had it."""
        if clue in self.clues:
            return False
        self.clues.append(clue)
        return True

    # ---- E: use -----------------------------------------------------------

    def interact(self, team):
        obj = self.faced_object()
        if obj is None:
            return self.plain_tile()
        if obj["kind"] == "container":
            return self.open_container(obj)
        if obj["kind"] == "task":
            return self.use_task(obj, team)
        if obj["kind"] == "exit":
            return self.use_exit(obj, team)
        return Result(obj["name"], obj["text"])

    def plain_tile(self):
        # Furniture with no object entry (e.g. ordinary bookshelves) still
        # says something, so the room feels alive.
        tile = self.room.game_map.tile(*self.faced_position())
        if tile and "about" in tile:
            return Result(tile["name"].title(), [tile["about"]])
        return None

    def open_container(self, obj):
        if obj["id"] in self.opened:
            return Result(obj["name"], obj.get("after", ["There is nothing else here."]))
        self.opened.add(obj["id"])
        self.items.append(obj["item"])
        lines = obj["text"] + [f"* You got: {obj['item']}! *"]
        if "reveal_art" in obj:
            lines.append("(Press F to read it.)")
        return Result(obj["name"], lines)

    def use_task(self, task, team):
        if team.is_done(task["id"]):
            return Result(task["name"], ["* Task complete. *"])
        if "needs_item" in task and task["needs_item"] not in self.items:
            return Result(task["name"], task["text"] + [task["locked_text"]])
        if "needs_task" in task and not team.is_done(task["needs_task"]):
            return Result(task["name"], task["text"] + [task["locked_text"]])
        return Result(task["name"], task["text"], task=task)

    def use_exit(self, obj, team):
        if not team.all_done():
            count = f"Team tasks: {team.done_count()}/{len(team.task_ids)}."
            return Result(obj["name"], obj["text"] + [count])
        return Result(obj["name"], ["The exit is open! You step through..."], escape=True)

    # ---- F: inspect -------------------------------------------------------

    def inspect(self, team):
        obj = self.faced_object()
        if obj is None:
            return self.plain_tile()

        art = obj.get("art")
        if obj["kind"] == "container":
            if obj["id"] not in self.opened:
                return Result(obj["name"], ["It's closed. Press E to open it."], art=art)
            art = obj.get("reveal_art", art)

        lines = list(obj.get("inspect", obj["text"]))
        if obj["kind"] == "exit":
            lines.append(f"Team tasks: {team.done_count()}/{len(team.task_ids)}.")
        if "clue" in obj and self.add_clue(obj["clue"]):
            lines.append("* New clue saved in your journal (press I). *")
        return Result(obj["name"], lines, art=art)
