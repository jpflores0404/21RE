"""A room map loaded from a text file, and the rule for walking on it.

There is no drawing code here, so these rules can be unit tested
without opening a game window.
"""
from game.tiles import FLOOR, SPAWN, TILES

# How x and y change for each direction. y grows downward, like screen rows.
DIRECTIONS = {
    "up": (0, -1),
    "down": (0, 1),
    "left": (-1, 0),
    "right": (1, 0),
}


class MapError(ValueError):
    """The map file has a mistake in it."""


class GameMap:
    def __init__(self, rows):
        if not rows:
            raise MapError("Map is empty.")

        # Make every row the same width. Missing characters become void (" ").
        width = max(len(row) for row in rows)
        rows = [row.ljust(width) for row in rows]

        # Find the spawn point and check every character is a known tile.
        self.spawn = None
        for y, row in enumerate(rows):
            for x, ch in enumerate(row):
                if ch == SPAWN:
                    if self.spawn is not None:
                        raise MapError(f"Two spawn points: {self.spawn} and {(x, y)}.")
                    self.spawn = (x, y)
                elif ch not in TILES:
                    raise MapError(f"Unknown tile {ch!r} at {(x, y)}.")
        if self.spawn is None:
            raise MapError("Map has no spawn point '@'.")

        self.rows = [row.replace(SPAWN, FLOOR) for row in rows]
        self.width = width
        self.height = len(rows)

    def tile(self, x, y):
        """Return the tile at (x, y), or None if (x, y) is outside the map."""
        if 0 <= x < self.width and 0 <= y < self.height:
            return TILES[self.rows[y][x]]
        return None

    def can_walk(self, x, y):
        tile = self.tile(x, y)
        return tile is not None and tile["walk"]


def load_map(path):
    with open(path, encoding="utf-8") as file:
        return GameMap(file.read().rstrip().splitlines())


class Player:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.facing = "down"
        self.steps = 0  # used to animate the walking sprite


def try_move(game_map, player, direction):
    """Turn the player to face `direction`, then step forward if the tile is free.

    Returns (moved, message). Walking into a wall or object still turns the
    player. That is how you choose what to interact with.
    """
    if direction not in DIRECTIONS:
        return False, f"Unknown direction: {direction}"

    player.facing = direction
    dx, dy = DIRECTIONS[direction]
    x, y = player.x + dx, player.y + dy

    tile = game_map.tile(x, y)
    if tile is None:
        return False, "You cannot go any further."
    if not tile["walk"]:
        return False, tile.get("bump", "")

    player.x, player.y = x, y
    player.steps += 1
    return True, ""
