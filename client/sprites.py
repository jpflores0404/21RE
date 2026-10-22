"""ASCII character sprite shown in the side panel, one pose per facing direction.

The body is 4 rows; a 5th row of legs alternates every step to give a simple
two-frame walking animation.
"""

BODIES = {
    "down": [
        r"  ___  ",
        r" [___] ",
        r" (o.o) ",
        r" /|_|\ ",
    ],
    "up": [
        r"  ___  ",
        r" [___] ",
        r" (   ) ",
        r" /|_|\ ",
    ],
    "left": [
        r"  ___  ",
        r"=[___] ",
        r" (o  ) ",
        r"  |_|\ ",
    ],
    "right": [
        r"  ___  ",
        r" [___]=",
        r" (  o) ",
        r" /|_|  ",
    ],
}

LEGS = [r"  / \  ", r"  | |  "]


def get_sprite(facing, frame):
    return BODIES.get(facing, BODIES["down"]) + [LEGS[frame % len(LEGS)]]
