"""Loads a level: level.json, one map per player, and each room's objects.

A level folder looks like this:
    assets/maps/records_wing/
        level.json              title, and which files belong to each room
        gallery.txt             Player 1's map
        gallery_objects.json    Player 1's objects (paintings, tasks, the exit...)
        archive.txt             Player 2's map
        archive_objects.json    Player 2's objects

Objects stay plain dictionaries, exactly as written in the JSON files.
"""
import json
from pathlib import Path

from game.game_map import load_map

MAPS_DIR = Path(__file__).resolve().parent.parent / "assets" / "maps"

# What each kind of object does when you press E:
#   examine    shows its text
#   container  opens once and gives you an item
#   task       starts a mini-game (see client/tasks.py)
#   exit       the shared exit; it opens when every team task is done
KINDS = ("examine", "container", "task", "exit")

# The mini-games a task can use.
TASK_TYPES = ("wires", "swipe", "download", "upload", "code", "breaker")


class Room:
    def __init__(self, player_id, info, game_map, objects):
        self.player_id = player_id
        self.name = info["name"]
        self.intro = info.get("intro", "")
        self.objective = info.get("objective", "")
        self.game_map = game_map
        self.objects = objects
        self.tasks = [obj for obj in objects if obj["kind"] == "task"]

        # (x, y) -> object, so we can quickly find what is at a tile.
        # A big object (like a 2-tile desk) is listed once per tile.
        self.at = {}
        for obj in objects:
            for x, y in obj["at"]:
                self.at[(x, y)] = obj


class Level:
    def __init__(self, title, rooms):
        self.title = title
        self.rooms = rooms  # player id (1 or 2) -> Room

    def all_tasks(self):
        return [task for room in self.rooms.values() for task in room.tasks]

    def task_ids(self):
        return [task["id"] for task in self.all_tasks()]

    def task_title(self, task_id):
        for task in self.all_tasks():
            if task["id"] == task_id:
                return task["title"]
        return task_id


def read_json(path):
    with open(path, encoding="utf-8") as file:
        return json.load(file)


def load_level(level_id, maps_dir=MAPS_DIR):
    folder = Path(maps_dir) / level_id
    info = read_json(folder / "level.json")

    rooms = {}
    for player_id, room_info in info["rooms"].items():
        game_map = load_map(folder / room_info["map"])
        objects = read_json(folder / room_info["objects"])
        check_objects(objects, game_map)
        rooms[int(player_id)] = Room(int(player_id), room_info, game_map, objects)

    level = Level(info["title"], rooms)
    check_tasks(level)
    return level


def check_objects(objects, game_map):
    """Catch typos in an objects file when the game starts, not in the middle of play."""
    used = {}
    for obj in objects:
        name = obj["id"]
        if obj["kind"] not in KINDS:
            raise ValueError(f"{name}: unknown kind {obj['kind']!r}")
        if obj["kind"] == "container" and "item" not in obj:
            raise ValueError(f"{name}: a container needs an 'item'")
        if obj["kind"] == "task":
            if obj.get("task") not in TASK_TYPES:
                raise ValueError(f"{name}: unknown task type {obj.get('task')!r}")
            if "title" not in obj:
                raise ValueError(f"{name}: a task needs a 'title'")
            if obj["task"] == "code" and not obj.get("answer"):
                raise ValueError(f"{name}: a code task needs an 'answer'")
        for x, y in obj["at"]:
            tile = game_map.tile(x, y)
            if tile is None or tile["walk"]:
                raise ValueError(f"{name}: {(x, y)} must be a solid tile, like a wall or furniture")
            if (x, y) in used:
                raise ValueError(f"{name}: {(x, y)} is already used by {used[(x, y)]}")
            used[(x, y)] = name


def check_tasks(level):
    """Task ids must be unique, and 'needs_task' must name a real task."""
    ids = level.task_ids()
    if len(ids) != len(set(ids)):
        raise ValueError("Two tasks share the same id.")
    for task in level.all_tasks():
        if "needs_task" in task and task["needs_task"] not in ids:
            raise ValueError(f"{task['id']}: needs_task {task['needs_task']!r} does not exist")
