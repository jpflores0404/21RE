#include "../src/game.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int checks, failures;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"Game check failed at %d: %s\n",__LINE__,#x); ++failures; } } while (0)

static int action(Game *game,int player,const char *json)
{
    cJSON *request=cJSON_Parse(json);
    int result;
    CHECK(request!=NULL);
    if (!request) return 0;
    result=game_action(game,player,request);
    cJSON_Delete(request);
    return result;
}

static void face(Game *game,int player,int x,int y)
{
    game->players[player].x=x;
    game->players[player].y=y==ROOM_H-1 ? y-1 : y+1;
    game->players[player].facing=y==ROOM_H-1 ? 2 : 0;
    game->players[player].active_object[0]='\0';
}

static int use(Game *game,int player,int x,int y)
{
    char json[64];
    face(game,player,x,y);
    snprintf(json,sizeof json,"{\"type\":\"interact\",\"map\":%d}",game->map);
    return action(game,player,json);
}

static char *view_text(Game *game,int player)
{
    cJSON *view=game_view(game,player);
    char *text;
    CHECK(view!=NULL);
    text=cJSON_PrintUnformatted(view); cJSON_Delete(view);
    CHECK(text!=NULL);
    return text;
}

static void layout_checks(void)
{
    int map,player,open;
    for (map=1;map<=3;++map) for (player=0;player<2;++player) for (open=0;open<=1;++open) {
        char rows[ROOM_H][ROOM_W+1];
        int seen[ROOM_H][ROOM_W]={{0}},qx[ROOM_H*ROOM_W],qy[ROOM_H*ROOM_W],head=0,tail=1,x,y,i,spawns=0;
        const int dx[4]={0,1,0,-1},dy[4]={-1,0,1,0};
        CHECK(content_room(map,player,rows,open));
        qx[0]=1;qy[0]=1;seen[1][1]=1;
        for (y=0;y<ROOM_H;++y) {
            CHECK(strlen(rows[y])==ROOM_W);
            for (x=0;x<ROOM_W;++x) if (rows[y][x]=='@') ++spawns;
        }
        CHECK(spawns==1);
        while (head<tail) {
            x=qx[head];y=qy[head++];
            for (i=0;i<4;++i) {
                int nx=x+dx[i],ny=y+dy[i];
                if (nx<0 || nx>=ROOM_W || ny<0 || ny>=ROOM_H || seen[ny][nx]) continue;
                if (rows[ny][nx]!='.' && rows[ny][nx]!='*' && rows[ny][nx]!='@') continue;
                seen[ny][nx]=1;qx[tail]=nx;qy[tail++]=ny;
            }
        }
        for (y=0;y<ROOM_H;++y) for (x=0;x<ROOM_W;++x) {
            int sealed=map==2 && player==0 && !open && x>20;
            if (rows[y][x]=='.' || rows[y][x]=='*' || rows[y][x]=='@') CHECK(seen[y][x] || sealed);
            else if (rows[y][x]!='#') {
                int adjacent=0;
                for (i=0;i<4;++i) {
                    int nx=x+dx[i],ny=y+dy[i];
                    if (nx>=0 && nx<ROOM_W && ny>=0 && ny<ROOM_H && seen[ny][nx]) adjacent=1;
                }
                CHECK(adjacent || sealed);
            }
        }
        if (map==2 && player==0) CHECK(seen[4][24]==open);
    }
}

int game_tests(void)
{
    Game game;
    char *text;
    checks=failures=0;
    layout_checks();
    game_init(&game);
    CHECK(game.map==1 && !game.started);
    CHECK(!action(&game,0,"{\"type\":\"move\",\"map\":1,\"direction\":\"right\"}"));
    CHECK(action(&game,0,"{\"type\":\"ready\"}"));
    CHECK(!game.started);
    CHECK(action(&game,1,"{\"type\":\"ready\"}"));
    CHECK(game.started);
    CHECK(action(&game,0,"{\"type\":\"move\",\"map\":1,\"direction\":\"up\"}"));
    CHECK(game.players[0].x==1 && game.players[0].y==1 && game.players[0].facing==0);
    CHECK(!action(&game,0,"{\"type\":\"move\",\"map\":0,\"direction\":\"right\"}"));
    CHECK(!action(&game,0,"{\"type\":\"move\",\"map\":1,\"direction\":\"teleport\"}"));
    CHECK(!use(&game,0,14,10));
    CHECK(game.map==1 && !game.players[0].exit_ready);
    face(&game,0,8,6);
    text=view_text(&game,0); CHECK(strstr(text,"X = 14")==NULL); free(text);
    CHECK(use(&game,0,8,6));
    CHECK(game.players[0].note_count==1);
    CHECK(use(&game,0,8,6)); CHECK(game.players[0].note_count==1);
    text=view_text(&game,1); CHECK(strstr(text,"X = 14")==NULL); free(text);
    CHECK(!use(&game,0,21,6));
    CHECK(!action(&game,0,"{\"type\":\"submit\",\"map\":1,\"object\":\"hatch\",\"answer\":\"259\"}"));
    CHECK(!game.hatch);
    CHECK(!use(&game,1,20,2));
    CHECK(use(&game,1,6,2));
    CHECK(!action(&game,1,"{\"type\":\"submit\",\"map\":1,\"object\":\"order_note\",\"answer\":\"shadow lock silence\"}"));
    CHECK(!game.words);
    CHECK(action(&game,1,"{\"type\":\"submit\",\"map\":1,\"object\":\"order_note\",\"answer\":\"1 2 3\"}"));
    CHECK(game.words);
    CHECK(use(&game,1,20,2));
    CHECK(!action(&game,1,"{\"type\":\"submit\",\"map\":1,\"object\":\"cabinet\",\"answer\":\"000\"}"));
    CHECK(!game.cabinet);
    CHECK(action(&game,1,"{\"type\":\"submit\",\"map\":1,\"object\":\"cabinet\",\"answer\":\"246\"}"));
    text=view_text(&game,0); CHECK(strstr(text,"Use Z, then X")==NULL); free(text);
    CHECK(use(&game,0,21,6));
    CHECK(!action(&game,0,"{\"type\":\"submit\",\"map\":1,\"object\":\"hatch\",\"answer\":\"1259\"}"));
    CHECK(action(&game,0,"{\"type\":\"submit\",\"map\":1,\"object\":\"hatch\",\"answer\":\"259\"}"));
    CHECK(use(&game,0,14,10)); CHECK(game.map==1);
    CHECK(use(&game,0,14,10)); CHECK(game.map==1);
    CHECK(use(&game,1,14,10)); CHECK(game.map==2);
    CHECK(!game.players[0].exit_ready && !game.players[1].exit_ready && game.players[0].note_count==0);
    CHECK(!action(&game,0,"{\"type\":\"interact\",\"map\":1}"));
    CHECK(use(&game,1,20,3));
    CHECK(!action(&game,1,"{\"type\":\"test\",\"map\":2}"));
    CHECK(!action(&game,1,"{\"type\":\"dial\",\"map\":2,\"dial\":3,\"value\":2}"));
    CHECK(!action(&game,1,"{\"type\":\"dial\",\"map\":2,\"dial\":0,\"value\":5}"));
    CHECK(use(&game,0,5,3)); CHECK(!game.passage);
    CHECK(use(&game,0,5,7)); CHECK(!game.passage);
    CHECK(use(&game,0,13,3)); CHECK(game.passage);
    CHECK(use(&game,0,24,3));
    text=view_text(&game,1); CHECK(strstr(text,"Channel A begins at two")==NULL); free(text);
    CHECK(action(&game,1,"{\"type\":\"dial\",\"map\":2,\"dial\":0,\"value\":1}"));
    CHECK(action(&game,1,"{\"type\":\"dial\",\"map\":2,\"dial\":1,\"value\":3}"));
    CHECK(action(&game,1,"{\"type\":\"dial\",\"map\":2,\"dial\":2,\"value\":4}"));
    CHECK(action(&game,1,"{\"type\":\"test\",\"map\":2}")); CHECK(!game.calibrated);
    CHECK(strstr(game.players[0].feedback,"A below target")!=NULL);
    CHECK(strstr(game.players[1].feedback,"below target")==NULL);
    CHECK(action(&game,1,"{\"type\":\"dial\",\"map\":2,\"dial\":0,\"value\":2}"));
    CHECK(action(&game,1,"{\"type\":\"test\",\"map\":2}")); CHECK(game.calibrated);
    CHECK(use(&game,1,14,10)); CHECK(game.map==2);
    CHECK(use(&game,0,14,10)); CHECK(game.map==3);
    CHECK(use(&game,0,14,10)); CHECK(use(&game,1,14,10)); CHECK(game.map==3);
    CHECK(!action(&game,0,"{\"type\":\"submit\",\"map\":3,\"object\":\"authorization\",\"answer\":\"259\"}"));
    text=view_text(&game,0); CHECK(strstr(text,"\"milestone_complete\":true")!=NULL); CHECK(strstr(text,"under development")!=NULL); free(text);
    CHECK(action(&game,0,"{\"type\":\"chat\",\"text\":\"Ready to compare clues\"}"));
    CHECK(strstr(game.chat,"Player 1: Ready")!=NULL);
    CHECK(!action(&game,1,"{\"type\":\"chat\",\"text\":\"bad\\u001b[2J\"}"));
    CHECK(!action(&game,0,"{\"type\":\"victory\",\"map\":3}"));
    CHECK(game.map==3);
    if (!failures) printf("Campaign rule tests passed (%d checks).\n",checks);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
