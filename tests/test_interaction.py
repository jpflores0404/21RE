"""Tests for using (E) and inspecting (F) objects, and the journal."""
import unittest

from client.screens import ART_DIR
from game.level import load_level
from game.session import RoomSession
from game.team import Team


def stand(session, x, y, facing):
    """Put the player at (x, y) facing a direction."""
    session.player.x, session.player.y, session.player.facing = x, y, facing


class GalleryTests(unittest.TestCase):
    def setUp(self):
        level = load_level("records_wing")
        self.team = Team(level.task_ids())
        self.session = RoomSession(level.rooms[1])

    def test_use_painting_shows_text_without_clue(self):
        stand(self.session, 6, 2, "up")  # The Watcher is at (6, 1)
        result = self.session.interact(self.team)
        self.assertEqual(result.title, "Painting: The Watcher")
        self.assertEqual(self.session.clues, [])

    def test_inspect_painting_saves_clue_once(self):
        stand(self.session, 6, 2, "up")
        first = self.session.inspect(self.team)
        self.assertEqual(first.art, "painting_watcher")
        self.session.inspect(self.team)
        self.assertEqual(len(self.session.clues), 1)

    def test_container_opens_once_and_gives_item(self):
        stand(self.session, 41, 9, "up")  # curator's desk at (41, 8)
        closed = self.session.inspect(self.team)
        self.assertIn("closed", closed.lines[0])

        self.session.interact(self.team)
        self.assertEqual(self.session.items, ["Curator's Note"])
        again = self.session.interact(self.team)
        self.assertEqual(again.lines, ["The drawer is empty now."])
        self.assertEqual(self.session.items, ["Curator's Note"])

        note = self.session.inspect(self.team)
        self.assertEqual(note.art, "curator_note")
        self.assertEqual(len(self.session.clues), 1)

    def test_big_object_is_the_same_from_either_tile(self):
        stand(self.session, 42, 9, "up")  # second tile of the desk
        self.assertEqual(self.session.faced_name(), "Curator's Desk")

    def test_nothing_to_use_on_open_floor(self):
        stand(self.session, 3, 11, "right")
        self.assertIsNone(self.session.interact(self.team))


class ArchiveTests(unittest.TestCase):
    def test_plain_bookshelf_has_a_description(self):
        level = load_level("records_wing")
        session = RoomSession(level.rooms[2])
        stand(session, 6, 3, "left")  # shelf at (5, 3) has no object entry
        self.assertEqual(session.interact(Team([])).title, "Bookshelf")


class AssetTests(unittest.TestCase):
    def test_all_art_files_exist(self):
        for room in load_level("records_wing").rooms.values():
            for obj in room.objects:
                for key in ("art", "reveal_art"):
                    if key in obj:
                        path = ART_DIR / f"{obj[key]}.txt"
                        self.assertTrue(path.exists(), f"{obj['id']}: missing {path.name}")


if __name__ == "__main__":
    unittest.main()
