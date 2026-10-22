"""Tests for loading maps and moving. Run all tests with: python3 -m unittest -v"""
import unittest

from game.game_map import GameMap, MapError, Player, try_move
from game.level import load_level

SMALL_MAP = [
    "#####",
    "#@.P#",
    "#...#",
    "#####",
]


class MovementTests(unittest.TestCase):
    def setUp(self):
        self.map = GameMap(SMALL_MAP)
        self.player = Player(*self.map.spawn)

    def test_spawn_found_and_becomes_floor(self):
        self.assertEqual(self.map.spawn, (1, 1))
        self.assertTrue(self.map.can_walk(1, 1))

    def test_move_onto_floor(self):
        moved, _ = try_move(self.map, self.player, "right")
        self.assertTrue(moved)
        self.assertEqual((self.player.x, self.player.y), (2, 1))
        self.assertEqual(self.player.steps, 1)

    def test_wall_blocks_movement(self):
        moved, message = try_move(self.map, self.player, "up")
        self.assertFalse(moved)
        self.assertIn("wall", message)
        self.assertEqual((self.player.x, self.player.y), (1, 1))

    def test_blocked_move_still_turns_player(self):
        try_move(self.map, self.player, "left")
        self.assertEqual(self.player.facing, "left")

    def test_object_blocks_movement(self):
        try_move(self.map, self.player, "right")
        moved, _ = try_move(self.map, self.player, "right")
        self.assertFalse(moved)
        self.assertEqual((self.player.x, self.player.y), (2, 1))

    def test_cannot_leave_the_map(self):
        open_map = GameMap(["@."])
        player = Player(*open_map.spawn)
        moved, _ = try_move(open_map, player, "left")
        self.assertFalse(moved)

    def test_unknown_direction_rejected(self):
        moved, _ = try_move(self.map, self.player, "diagonal")
        self.assertFalse(moved)


class MapLoadingTests(unittest.TestCase):
    def test_unknown_tile_rejected(self):
        with self.assertRaises(MapError):
            GameMap(["#@X#"])

    def test_missing_spawn_rejected(self):
        with self.assertRaises(MapError):
            GameMap(["#..#"])

    def test_two_spawns_rejected(self):
        with self.assertRaises(MapError):
            GameMap(["@.@"])

    def test_short_rows_filled_with_void(self):
        game_map = GameMap(["####", "#@"])
        self.assertEqual(game_map.width, 4)
        self.assertFalse(game_map.can_walk(3, 1))


class RecordsWingMapTests(unittest.TestCase):
    def setUp(self):
        self.level = load_level("records_wing")

    def test_both_rooms_load(self):
        self.assertEqual(sorted(self.level.rooms), [1, 2])

    def test_rooms_are_closed_in(self):
        # No walkable tile on the edge means players can never walk off the map.
        for room in self.level.rooms.values():
            m = room.game_map
            for y in range(m.height):
                for x in range(m.width):
                    on_edge = x in (0, m.width - 1) or y in (0, m.height - 1)
                    if on_edge:
                        self.assertFalse(m.can_walk(x, y), f"{room.name} open at {(x, y)}")

    def test_every_floor_tile_reachable_from_spawn(self):
        # Flood fill: catches areas that were walled off by mistake.
        for room in self.level.rooms.values():
            m = room.game_map
            seen = {m.spawn}
            todo = [m.spawn]
            while todo:
                x, y = todo.pop()
                for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                    if (nx, ny) not in seen and m.can_walk(nx, ny):
                        seen.add((nx, ny))
                        todo.append((nx, ny))
            walkable = {(x, y) for y in range(m.height) for x in range(m.width) if m.can_walk(x, y)}
            self.assertEqual(walkable, seen, f"{room.name} has tiles you cannot reach")

    def test_every_object_can_be_reached(self):
        # Each object needs at least one walkable tile next to it to stand on.
        for room in self.level.rooms.values():
            for obj in room.objects:
                spots = [(x + dx, y + dy) for x, y in obj["at"]
                         for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
                self.assertTrue(any(room.game_map.can_walk(*s) for s in spots), obj["id"])


if __name__ == "__main__":
    unittest.main()
