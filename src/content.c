#include "game.h"
#include <string.h>

typedef struct { int x, y; char symbol; } TileObject;

static const TileObject gallery[] = {{5,2,'A'},{13,2,'B'},{21,2,'C'},{8,6,'D'},{21,6,'H'}};
static const TileObject archive[] = {{6,2,'N'},{20,2,'K'},{8,6,'L'}};
static const TileObject exhibition[] = {{5,3,'A'},{13,3,'B'},{5,7,'C'},{13,4,'*'},{24,3,'R'}};
static const TileObject maintenance[] = {{6,2,'B'},{20,3,'D'},{20,6,'T'}};
static const TileObject laboratory[] = {{6,2,'M'},{5,5,'1'},{10,5,'2'},{15,5,'3'},{20,5,'4'},{6,8,'C'},{24,8,'A'}};
static const TileObject security[] = {{6,2,'R'},{20,3,'V'},{20,7,'S'}};

int content_room(int map, int player, char rows[ROOM_H][ROOM_W + 1], int passage)
{
    const TileObject *objects;
    size_t count, i;
    int x, y;
    if (map < 1 || map > 3 || player < 0 || player > 1 || rows == NULL) return 0;
    for (y = 0; y < ROOM_H; ++y) {
        for (x = 0; x < ROOM_W; ++x)
            rows[y][x] = y == 0 || y == ROOM_H-1 || x == 0 || x == ROOM_W-1 ? '#' : '.';
        rows[y][ROOM_W] = '\0';
    }
    if (map == 1 && player == 0) { objects=gallery; count=sizeof gallery/sizeof *gallery; }
    else if (map == 1) { objects=archive; count=sizeof archive/sizeof *archive; }
    else if (map == 2 && player == 0) { objects=exhibition; count=sizeof exhibition/sizeof *exhibition; }
    else if (map == 2) { objects=maintenance; count=sizeof maintenance/sizeof *maintenance; }
    else if (player == 0) { objects=laboratory; count=sizeof laboratory/sizeof *laboratory; }
    else { objects=security; count=sizeof security/sizeof *security; }
    if (map == 2 && player == 0) {
        for (y=1; y<ROOM_H-1; ++y) rows[y][20]='#';
        rows[5][20]=passage ? '.' : 'P';
    }
    for (i=0; i<count; ++i) rows[objects[i].y][objects[i].x]=objects[i].symbol;
    rows[1][1]='@';
    rows[ROOM_H-1][14]='E';
    return 1;
}

const char *content_room_name(int map, int player)
{
    static const char *names[3][2]={{"Gallery","Archive"},{"Exhibition","Maintenance"},{"Research Laboratory","Security Control"}};
    if (map < 1 || map > 3 || player < 0 || player > 1) return "Unknown room";
    return names[map-1][player];
}
