"""The tile legend: what each character in a map file means.

Maps are plain text files. Every character is one tile, for example '#' is a
wall and '.' is floor. To add a new kind of tile, add one line here.

Each tile is a small dictionary:
    name   what the tile is called
    walk   True if the player can stand on it
    style  how it is coloured on screen: wall, floor, decor, object, door
    show   character drawn on screen (only needed if it differs from the file)
    bump   message shown when the player walks into it
    about  text shown when the player presses E on it (only for plain furniture)
"""

SPAWN = "@"  # where the player starts; it becomes floor when the map loads
FLOOR = "."

TILES = {
    "#": {"name": "wall", "walk": False, "style": "wall", "bump": "A solid wall blocks your way."},
    ".": {"name": "floor", "walk": True, "style": "floor"},
    ",": {"name": "carpet", "walk": True, "style": "decor"},
    "/": {"name": "doorway", "walk": True, "style": "floor", "show": "."},
    "+": {"name": "locked door", "walk": False, "style": "door",
          "bump": "The door is locked tight.", "about": "The door is locked tight."},
    "=": {"name": "exit door", "walk": False, "style": "door",
          "bump": "The exit is sealed.", "about": "The exit is sealed."},
    "P": {"name": "painting", "walk": False, "style": "object",
          "bump": "A framed painting hangs on the wall.", "about": "A framed painting."},
    "|": {"name": "bookshelf", "walk": False, "style": "object",
          "bump": "A tall bookshelf blocks the way.",
          "about": "Rows of crumbling binders. Nothing stands out."},
    "T": {"name": "reading table", "walk": False, "style": "object",
          "bump": "A heavy reading table is in the way.", "about": "A heavy wooden table."},
    "D": {"name": "desk", "walk": False, "style": "object",
          "bump": "A cluttered desk blocks your path.", "about": "A cluttered desk."},
    "C": {"name": "cabinet", "walk": False, "style": "object",
          "bump": "A metal cabinet stands here.", "about": "A metal cabinet. It won't open."},
    "b": {"name": "bench", "walk": False, "style": "object",
          "bump": "A viewing bench is in the way.", "about": "A viewing bench."},
    "o": {"name": "pedestal", "walk": False, "style": "object",
          "bump": "A stone pedestal.", "about": "A stone pedestal."},
    "%": {"name": "control panel", "walk": False, "style": "object",
          "bump": "A control panel is in the way.", "about": "A control panel. Its screen is dark."},
    # Void fills in short map rows. It is never walkable, so it acts like a wall.
    " ": {"name": "void", "walk": False, "style": "void", "bump": "There is nothing there."},
}
