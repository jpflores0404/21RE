"""What both players share: the team's finished tasks, and who has escaped.

Like the TOTAL TASKS bar in Among Us, every task either player finishes
fills the same team bar. When it is full, the exit opens.

In a LAN game each computer keeps its own Team. They stay the same because
each side sends a message when something changes (see client/network.py):
    {"type": "task", "id": "a_download"}    a task was finished
    {"type": "escaped", "player": 1}        a player walked out of the exit
"""


class Team:
    def __init__(self, task_ids):
        self.task_ids = list(task_ids)
        self.done = set()     # ids of finished tasks
        self.escaped = set()  # ids of players who walked out of the exit

    def finish(self, task_id):
        self.done.add(task_id)

    def is_done(self, task_id):
        return task_id in self.done

    def done_count(self):
        return len([task_id for task_id in self.task_ids if task_id in self.done])

    def all_done(self):
        return self.done_count() == len(self.task_ids)

    def escape(self, player_id):
        self.escaped.add(player_id)

    def has_won(self):
        return self.all_done() and 1 in self.escaped and 2 in self.escaped

    def apply(self, message):
        """Update the team from a message sent by the partner."""
        if message["type"] == "task":
            self.finish(message["id"])
        elif message["type"] == "escaped":
            self.escape(message["player"])
