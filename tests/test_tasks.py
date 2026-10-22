"""Tests for tasks, the team's progress, the exit, and sending messages."""
import socket
import time
import unittest

from client.network import LocalLink, check_for_partner, join, start_hosting
from game.level import load_level
from game.session import RoomSession
from game.team import Team
from tests.test_interaction import stand


class TaskTests(unittest.TestCase):
    def setUp(self):
        self.level = load_level("records_wing")
        self.team = Team(self.level.task_ids())
        self.gallery = RoomSession(self.level.rooms[1])
        self.archive = RoomSession(self.level.rooms[2])

    def test_each_player_has_four_tasks(self):
        self.assertEqual(len(self.level.rooms[1].tasks), 4)
        self.assertEqual(len(self.level.rooms[2].tasks), 4)

    def test_wiring_panel_starts_a_task(self):
        stand(self.gallery, 40, 2, "up")  # wiring panel at (40, 1)
        result = self.gallery.interact(self.team)
        self.assertEqual(result.task["task"], "wires")

    def test_swipe_card_needs_the_keycard(self):
        stand(self.gallery, 48, 9, "right")  # card reader at (49, 9)
        self.assertIsNone(self.gallery.interact(self.team).task)

        stand(self.gallery, 18, 12, "up")    # the keycard is under the pedestal
        self.gallery.interact(self.team)
        stand(self.gallery, 48, 9, "right")
        self.assertEqual(self.gallery.interact(self.team).task["id"], "g_swipe")

    def test_upload_waits_for_partner_download(self):
        stand(self.gallery, 37, 7, "up")  # gallery terminal at (37, 6)
        self.assertIsNone(self.gallery.interact(self.team).task)
        self.team.finish("a_download")
        self.assertEqual(self.gallery.interact(self.team).task["id"], "g_upload")

    def test_finished_task_cannot_be_started_again(self):
        self.team.finish("g_wires")
        stand(self.gallery, 40, 2, "up")
        self.assertIsNone(self.gallery.interact(self.team).task)

    def test_codes_match_the_clues(self):
        # Paintings: Lantern 2, Key 4, Watcher 6. Archive order: light, key, watcher.
        # Curator: X=5, Y=9, Z=12. Logbook order: Z, X, Y.
        answers = {task["id"]: task.get("answer") for task in self.level.all_tasks()}
        self.assertEqual(answers["a_code"], "246")
        self.assertEqual(answers["g_code"], "12" + "5" + "9")


class ExitTests(unittest.TestCase):
    def setUp(self):
        self.level = load_level("records_wing")
        self.team = Team(self.level.task_ids())
        self.gallery = RoomSession(self.level.rooms[1])
        stand(self.gallery, 27, 13, "down")  # exit at (27, 14)

    def test_exit_is_sealed_until_all_tasks_are_done(self):
        result = self.gallery.interact(self.team)
        self.assertFalse(result.escape)
        self.assertIn("0/8", result.lines[-1])

    def test_exit_opens_when_all_tasks_are_done(self):
        for task_id in self.level.task_ids():
            self.team.finish(task_id)
        self.assertTrue(self.gallery.interact(self.team).escape)

    def test_team_wins_only_when_both_players_escape(self):
        for task_id in self.level.task_ids():
            self.team.finish(task_id)
        self.team.escape(1)
        self.assertFalse(self.team.has_won())
        self.team.apply({"type": "escaped", "player": 2})
        self.assertTrue(self.team.has_won())


class LinkTests(unittest.TestCase):
    def test_local_link_gives_messages_back(self):
        link = LocalLink()
        link.send({"type": "task", "id": "g_wires"})
        self.assertEqual(link.receive(), [{"type": "task", "id": "g_wires"}])
        self.assertEqual(link.receive(), [])

    def test_two_computers_can_talk(self):
        # Use a free port so the test never clashes with a running game.
        probe = socket.socket()
        probe.bind(("127.0.0.1", 0))
        port = probe.getsockname()[1]
        probe.close()

        server = start_hosting(port)
        joiner = join("127.0.0.1", port)
        host = None
        while host is None:
            host = check_for_partner(server)
        server.close()

        joiner.send({"type": "task", "id": "a_wires"})
        self.assertEqual(wait_for(host), [{"type": "task", "id": "a_wires"}])

        joiner.close()
        self.assertEqual(wait_for(host), [{"type": "bye"}])
        host.close()


def wait_for(link):
    """Wait up to 2 seconds for at least one message."""
    for _ in range(200):
        messages = link.receive()
        if messages:
            return messages
        time.sleep(0.01)
    return []


if __name__ == "__main__":
    unittest.main()
