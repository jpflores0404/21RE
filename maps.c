#include "game.h"

static const char layouts[3][2][MAP_HEIGHT][MAP_WIDTH + 1] = {
    {
        {
            "#################################################",
            "#...............................................#",
            "#...............................................#",
            "#...............................................#",
            "#...............................................#",
            "#...............................................#",
            "#...............................................#",
            "###########.########################.############",
            "#......................###......................#",
            "#......................###......................#",
            "#......................###......................#",
            "#......................###......................#",
            "#......................###......................E",
            "#......................###......................#",
            "#......................###......................#",
            "#......................###......................#",
            "#################################################",
        },
        {
            "#################################################",
            "#..............###........................#######",
            "#..............###........................#######",
            "#..............###........................#######",
            "#.........................................#######",
            "#..............###........................#######",
            "#..............###........................#######",
            "#..............##################################",
            "#..............##################################",
            "#..............######...........................#",
            "#..............######...........................#",
            "#..............######...........................#",
            "#...............................................#",
            "#..............######...........................#",
            "#..............######...........................#",
            "#..............######...........................#",
            "##################################E##############",
        },
    },
    {
        {
            "#################################################",
            "#....................############################",
            "#....................############################",
            "#....................############################",
            "#....................############################",
            "#....................#########..................#",
            "#....................#########..................#",
            "#########.####################..................#",
            "#########.####################..................E",
            "#....................#########..................#",
            "#....................#########..................#",
            "#....................#########..................#",
            "#...............................................#",
            "#....................#########..................#",
            "#....................#########..................#",
            "#....................#########..................#",
            "#################################################",
        },
        {
            "#################################################",
            "#..................##############################",
            "#..................##############################",
            "#..................##############################",
            "#..................##############################",
            "#..................##############################",
            "##############.##################################",
            "############.......................##############",
            "############.......................##############",
            "############.......................##############",
            "############.......................##############",
            "###############################.#################",
            "############################....................#",
            "############################....................#",
            "############################....................E",
            "############################....................#",
            "#################################################",
        },
    },
    {
        {
            "#################################################",
            "#...............##...............##.............#",
            "#...............##...............##.............#",
            "#...............##...............##.............#",
            "#...............##...............##.............E",
            "#................................##.............#",
            "#...............##...............##.............#",
            "#...............##...............##.............#",
            "#...............##...............##.............#",
            "#...............##...............##.............#",
            "#...............##...............##.............#",
            "#...............##..............................#",
            "#...............##...............##.............#",
            "#...............##...............##.............#",
            "#...............##...............##.............#",
            "#...............##...............##.............#",
            "#################################################",
        },
        {
            "#################################################",
            "#..............##................################",
            "#..............##................################",
            "#................................################",
            "#..............##................################",
            "#..............##................################",
            "#..............##................################",
            "#################................################",
            "#################................################",
            "#################................################",
            "#..............##................################",
            "#..............##................################",
            "#................................################",
            "E..............##................################",
            "#..............##................################",
            "#..............##................################",
            "#################################################",
        },
    },
};

typedef struct {
    int map, player, x, y;
    char symbol;
    const char *description;
} Object;

static const Object objects[] = {
    {1, 0, 6, 2, 'A', "Eye painting: the watcher bears the number 6."},
    {1, 0, 23, 4, 'B', "Lantern painting: the light bears the number 2."},
    {1, 0, 41, 2, 'C', "Key painting: the means of passage bears the number 4."},
    {1, 0, 7, 12, 'D', "Equation desk: X = 14 - 9. Y = 3 x 3. Z = 8 + 4. Ask your partner for the order."},
    {1, 0, 40, 11, 'H', "The hatch has a three-digit keypad."},
    {1, 0, 3, 4, 'i', "Directory: Gallery spans the north. Equation Study is southwest; Hatch Antechamber is southeast."},
    {1, 0, 18, 14, 'j', "Study notebook: the result of an equation can have more than one digit."},
    {1, 0, 29, 14, 'k', "Antechamber plaque: each operator must use their own exit control."},

    {1, 1, 7, 7, 'N', "Three Blanks: first comes the light, then the means of passage, and finally the watcher."},
    {1, 1, 39, 3, 'L', "Reading shelf: describe your discoveries to the operator in the Gallery."},
    {1, 1, 26, 12, 'K', "The cabinet has a three-digit padlock."},
    {1, 1, 3, 12, 'i', "Directory: Archive is west. Reading Room is northeast; Cabinet Office is southeast."},
    {1, 1, 21, 2, 'j', "Reading desk: an ordering clue needs the other room's symbols and numbers."},
    {1, 1, 43, 14, 'k', "Office record: two operators share responsibility for the release."},

    {2, 0, 5, 3, 'A', "Statue A faces east. It is away from the uniquely cracked tile."},
    {2, 0, 15, 4, 'B', "Statue B faces north, directly beside the uniquely cracked tile."},
    {2, 0, 5, 12, 'C', "Statue C faces north, far from the uniquely cracked tile."},
    {2, 0, 15, 5, '*', "A single cracked floor tile marks the service-latch location."},
    {2, 0, 39, 7, 'R', "Receiver rule: A begins at two. B is one above A. C is one above B."},
    {2, 0, 3, 5, 'i', "Directory: Statue Hall is northwest. Service Corridor is below; its sealed passage leads east to the Receiver Chamber."},
    {2, 0, 16, 14, 'j', "Service record: the receiver chamber is reached through the sealed service passage."},
    {2, 0, 44, 13, 'k', "Receiver label: only this operator sees the alignment feedback."},

    {2, 1, 8, 3, 'B', "Blueprint: use the north-facing statue nearest the uniquely cracked floor tile."},
    {2, 1, 24, 8, 'D', "The dial panel has three channels, A, B and C, each numbered from 0 to 4."},
    {2, 1, 40, 13, 'T', "The test console sends the current settings to the other operator's receiver."},
    {2, 1, 3, 2, 'i', "Directory: Follow the staggered rooms: Blueprint Office northwest, Dial Workshop central, Test Room southeast."},
    {2, 1, 14, 9, 'j', "Workshop note: write the three dial values in A B C order."},
    {2, 1, 30, 14, 'k', "Test-room notice: an unsuccessful test is harmless. Adjust the settings and try again."},

    {3, 0, 8, 3, 'M', "Maintenance note: red and blue valves are excluded from routine clearance. Ask which line is required."},
    {3, 0, 22, 3, '1', "Valve 1: Red, Supply line."},
    {3, 0, 28, 6, '2', "Valve 2: Blue, Coolant line."},
    {3, 0, 22, 10, '3', "Valve 3: Yellow, Feed line."},
    {3, 0, 29, 13, '4', "Valve 4: White, Return line."},
    {3, 0, 41, 8, 'A', "The authorization terminal confirms containment clearance."},
    {3, 0, 3, 12, 'i', "Directory: Laboratory is west, Valve Room central, Authorization Office east. The connecting doors are staggered."},
    {3, 0, 19, 14, 'j', "Valve-room notice: match the line name to the instruction in the other room."},
    {3, 0, 45, 13, 'k', "Authorization record: clearance and both release controls are required before leaving."},

    {3, 1, 8, 4, 'R', "Maintenance record: for routine clearance, operate the Return line. Feed lines must stay closed."},
    {3, 1, 25, 8, 'V', "Equipment status: ventilation is normal. The routine clearance procedure uses the valve room."},
    {3, 1, 7, 12, 'S', "The mechanical safety release requires containment clearance first."},
    {3, 1, 3, 2, 'i', "Directory: Equipment Room is the eastern hub. Maintenance Records is northwest; Safety Control is southwest."},
    {3, 1, 20, 13, 'j', "Equipment log: ask the laboratory operator to read the full valve labels."},
    {3, 1, 12, 14, 'k', "Safety notice: both operators must finish their release controls and use their own exits."}
};

char map_cell(int map, int player, int x, int y, int passage)
{
    if (map < 1 || map > 3 || player < 0 || player > 1 ||
        x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return '#';

    if (map == 2 && player == 0 && x == 29 && y == 12)
        return passage ? '.' : 'P';

    size_t count = sizeof objects / sizeof objects[0];
    for (size_t i = 0; i < count; ++i) {
        const Object *object = &objects[i];
        if (object->map == map && object->player == player &&
            object->x == x && object->y == y) return object->symbol;
    }
    return layouts[map - 1][player][y][x];
}

const char *map_room(int map, int player, int x, int y)
{
    static const char *names[3][2][3] = {
        {{"Gallery", "Equation Study", "Hatch Antechamber"},
         {"Archive", "Reading Room", "Cabinet Office"}},
        {{"Statue Hall", "Service Corridor", "Receiver Chamber"},
         {"Blueprint Office", "Dial Workshop", "Test Room"}},
        {{"Laboratory", "Valve Room", "Authorization Office"},
         {"Maintenance Records", "Equipment Room", "Safety Control"}}
    };
    if (map < 1 || map > 3 || player < 0 || player > 1) return "Unknown room";
    int room;
    if (map == 1 && player == 0) room = y < 8 ? 0 : x < 25 ? 1 : 2;
    else if (map == 1) room = x < 18 ? 0 : y < 8 ? 1 : 2;
    else if (map == 2 && player == 0) room = x >= 30 ? 2 : y < 9 ? 0 : 1;
    else if (map == 2) room = y < 7 ? 0 : y < 12 ? 1 : 2;
    else if (player == 0) room = x < 18 ? 0 : x < 35 ? 1 : 2;
    else room = x >= 15 ? 1 : y < 8 ? 0 : 2;
    return names[map - 1][player][room];
}

const char *map_description(int map, int player, char symbol)
{
    if (symbol == 'P') return "The service passage is sealed. A statue mechanism may open it.";
    if (symbol == 'E') return "Each operator has an exit control.";
    size_t count = sizeof objects / sizeof objects[0];
    for (size_t i = 0; i < count; ++i) {
        const Object *object = &objects[i];
        if (object->map == map && object->player == player && object->symbol == symbol)
            return object->description;
    }
    return "Nothing to inspect directly ahead. Face a marked object and press E.";
}
