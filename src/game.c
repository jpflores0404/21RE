#include "game.h"
#include <stdio.h>
#include <string.h>

static const int dx[4]={0,1,0,-1}, dy[4]={-1,0,1,0};

static const char *field(const cJSON *object, const char *name)
{
    const cJSON *value=cJSON_GetObjectItemCaseSensitive(object,name);
    return cJSON_IsString(value) ? value->valuestring : "";
}

static int number(const cJSON *object, const char *name, int *result)
{
    const cJSON *value=cJSON_GetObjectItemCaseSensitive(object,name);
    if (!cJSON_IsNumber(value) || value->valuedouble < -100000 || value->valuedouble > 100000 || value->valuedouble != (double)value->valueint) return 0;
    *result=value->valueint;
    return 1;
}

static int message(Game *game, int player, const char *text, int accepted)
{
    snprintf(game->players[player].feedback,sizeof game->players[player].feedback,"%s",text);
    return accepted;
}

static void note(Game *game, int player, const char *text)
{
    GamePlayer *p=&game->players[player];
    int i;
    for (i=0;i<p->note_count;++i) if (strcmp(p->notes[i],text)==0) return;
    if (p->note_count<NOTE_COUNT) {
        snprintf(p->notes[p->note_count],sizeof p->notes[0],"%s",text);
        ++p->note_count;
    }
}

static void start_room(Game *game)
{
    int player;
    for (player=0;player<2;++player) {
        GamePlayer *p=&game->players[player];
        p->x=1; p->y=1; p->facing=2; p->exit_ready=0; p->note_count=0;
        p->active_object[0]='\0';
        message(game,player,game->map==3 ? "Maps 1-2 milestone complete. Containment puzzles are under development." : "Explore your room with WASD. Face an object and press E. Share clues with T; read discovered notes with I.",1);
    }
}

void game_init(Game *game)
{
    memset(game,0,sizeof *game);
    game->map=1;
    start_room(game);
    message(game,0,"Waiting for both players to be ready.",1);
    message(game,1,"Waiting for both players to be ready.",1);
}

static char facing_tile(const Game *game, int player)
{
    const GamePlayer *p=&game->players[player];
    char rows[ROOM_H][ROOM_W+1];
    int x,y;
    if (p->facing<0 || p->facing>3 || !content_room(game->map,player,rows,game->passage)) return '#';
    x=p->x+dx[p->facing]; y=p->y+dy[p->facing];
    return x>=0 && x<ROOM_W && y>=0 && y<ROOM_H ? rows[y][x] : '#';
}

static const char *object_at_face(const Game *game, int player)
{
    char tile=facing_tile(game,player);
    if (tile=='E') return "exit";
    if (game->map==1) {
        if (player==0) {
            switch (tile) { case 'A':return "eye"; case 'B':return "lantern"; case 'C':return "key"; case 'D':return "equations"; case 'H':return "hatch"; default:return ""; }
        }
        switch (tile) { case 'N':return "order_note"; case 'K':return "cabinet"; case 'L':return "shelf"; default:return ""; }
    }
    if (game->map==2) {
        if (player==0) {
            switch (tile) { case 'A':return "statue_a"; case 'B':return "statue_b"; case 'C':return "statue_c"; case 'R':return "receiver"; case 'P':return "passage"; default:return ""; }
        }
        switch (tile) { case 'B':return "blueprint"; case 'D':return "dials"; case 'T':return "test_switch"; default:return ""; }
    }
    return tile=='.' || tile=='#' || tile=='@' ? "" : "preview";
}

static int interact(Game *game, int player)
{
    GamePlayer *p=&game->players[player];
    const char *object=object_at_face(game,player), *text="";
    p->active_object[0]='\0';
    if (strcmp(object,"exit")==0) {
        if (game->map==3) return message(game,player,"Containment exit sealed. This is the unfinished Map 3 preview, not campaign victory.",1);
        if (!(game->map==1 ? game->hatch : game->calibrated)) return message(game,player,"Exit sealed. Complete your team's current puzzles first.",0);
        p->exit_ready=1;
        message(game,player,"Exit ready. Waiting for your partner's exit control.",1);
        if (game->players[0].exit_ready && game->players[1].exit_ready) { ++game->map; start_room(game); }
        return 1;
    }
    if (game->map==3) return message(game,player,object[0] ? "Containment equipment is a preview. Valve, gas and final-release puzzles are not implemented in this milestone." : "Containment preview: walk around and inspect marked equipment with E.",1);
    if (strcmp(object,"eye")==0) text="Eye painting: The Watcher / Eye bears the number 6.";
    else if (strcmp(object,"lantern")==0) text="Lantern painting: The Lantern / light bears the number 2.";
    else if (strcmp(object,"key")==0) text="Key painting: The Key / means of passage bears the number 4.";
    else if (strcmp(object,"equations")==0) text="Equation desk: X = 14 - 9; Y = 3 x 3; Z = 8 + 4. Your partner may know the required order.";
    else if (strcmp(object,"order_note")==0) {
        if (game->words) text="Ordering note: First comes the light. Then the means of passage. The watcher shall always remain last.";
        else text="Three Blanks: choose words to complete the ordering note. Think of something that banishes shadows, a way through, and a silent guardian. Hint: use the displayed word choices.";
        snprintf(p->active_object,sizeof p->active_object,"%s",object);
    }
    else if (strcmp(object,"cabinet")==0) {
        if (game->cabinet) text="Cabinet instruction: Use Z, then X, then Y. Keep only the final digit of each result.";
        else if (!game->words) return message(game,player,"The cabinet label says to reconstruct the ordering note first.",0);
        else { snprintf(p->active_object,sizeof p->active_object,"%s",object); return message(game,player,"Cabinet padlock: enter the three-digit code using your partner's painting numbers.",1); }
    }
    else if (strcmp(object,"hatch")==0) {
        if (game->hatch) return message(game,player,"Hatch open. Authorization granted; both players must use their own exits.",1);
        if (!game->cabinet) return message(game,player,"Hatch inactive. Your partner must open the cabinet first.",0);
        snprintf(p->active_object,sizeof p->active_object,"%s",object);
        return message(game,player,"Mathematical hatch: enter the three-digit code derived from both rooms' information.",1);
    }
    else if (strcmp(object,"shelf")==0) text="Archive shelf: Records describe two operators sharing responsibility for every release.";
    else if (strcmp(object,"blueprint")==0) text="Blueprint: The service latch is beneath the north-facing statue nearest the uniquely cracked floor tile.";
    else if (strcmp(object,"statue_a")==0) text="Statue A faces East. Its mechanism clicks harmlessly. It is not the correct service latch.";
    else if (strcmp(object,"statue_b")==0) {
        game->passage=1;
        text="Statue B faces North, beside the cracked floor. The hidden passage opens in the east wall.";
    }
    else if (strcmp(object,"statue_c")==0) text="Statue C faces North, far from the cracked floor. Its mechanism clicks harmlessly.";
    else if (strcmp(object,"passage")==0) text="A sealed passage in the east wall. A service latch must open it.";
    else if (strcmp(object,"receiver")==0) text="Receiver rule: Channel A begins at two. Channel B is one above A. Channel C is one above B. Ask your partner to adjust and test the dials.";
    else if (strcmp(object,"dials")==0 || strcmp(object,"test_switch")==0) {
        snprintf(p->active_object,sizeof p->active_object,"%s",object);
        return message(game,player,"Calibration controls: adjust A, B and C from 0 to 4, then Test. Your partner sees receiver feedback.",1);
    }
    else return message(game,player,"Nothing to interact with directly ahead. Face a marked object with WASD and press E.",1);
    if (text[0]) { note(game,player,text); return message(game,player,text,1); }
    return 1;
}

static int submit(Game *game,int player,const cJSON *action)
{
    const char *object=field(action,"object"), *answer=field(action,"answer");
    GamePlayer *p=&game->players[player];
    if (!object[0] || strcmp(object,object_at_face(game,player))!=0 || strcmp(object,p->active_object)!=0)
        return message(game,player,"Use E on the permitted nearby object before submitting.",0);
    if (game->map==1 && player==1 && strcmp(object,"order_note")==0) {
        if (strcmp(answer,"light passage watcher")!=0 && strcmp(answer,"1 2 3")!=0) return message(game,player,"Try again. Use light for the shining start, passage for the way through, and watcher for the final guardian.",0);
        game->words=1;
        note(game,player,"Ordering note: First comes the light. Then the means of passage. The watcher shall always remain last.");
        p->active_object[0]='\0';
        return message(game,player,"Ordering note reconstructed. Share the symbol order and inspect the cabinet.",1);
    }
    if (game->map==1 && player==1 && strcmp(object,"cabinet")==0 && game->words) {
        if (strcmp(answer,"246")!=0) return message(game,player,"Code rejected. Compare the completed note with your partner's painting numbers.",0);
        game->cabinet=1; p->active_object[0]='\0';
        note(game,player,"Cabinet instruction: Use Z, then X, then Y. Keep only the final digit of each result.");
        return message(game,player,"Cabinet opened: Use Z, then X, then Y. Keep only the final digit of each result.",1);
    }
    if (game->map==1 && player==0 && strcmp(object,"hatch")==0 && game->cabinet) {
        if (strcmp(answer,"259")!=0) return message(game,player,"Code rejected. Use the final digit of each equation result in your partner's order.",0);
        game->hatch=1; p->active_object[0]='\0';
        return message(game,player,"Hatch authorized. Both players must activate their own exit controls.",1);
    }
    return message(game,player,"This submission is not legal at the current stage.",0);
}

static int calibration(Game *game,int player,const cJSON *action,int test)
{
    GamePlayer *p=&game->players[player];
    const char *object=object_at_face(game,player);
    int dial,value,i;
    const int targets[3]={2,3,4};
    char feedback[256];
    const char *states[3];
    if (game->map!=2 || player!=1 || (strcmp(object,"dials")!=0 && strcmp(object,"test_switch")!=0) || strcmp(p->active_object,object)!=0)
        return message(game,player,"Use E at your calibration controls first.",0);
    if (!test) {
        if (!number(action,"dial",&dial) || !number(action,"value",&value) || dial<0 || dial>2 || value<0 || value>4)
            return message(game,player,"Dial index must be 0-2 and value must be 0-4.",0);
        game->dials[dial]=value;
        return message(game,player,"Dial adjusted. Test the settings and ask your partner for receiver feedback.",1);
    }
    if (!game->passage) return message(game,player,"Receiver unavailable. Your partner must find the passage first.",0);
    for (i=0;i<3;++i) states[i]=game->dials[i]<targets[i] ? "below target" : game->dials[i]>targets[i] ? "above target" : "aligned";
    snprintf(feedback,sizeof feedback,"Receiver feedback: A %s; B %s; C %s.",states[0],states[1],states[2]);
    message(game,0,feedback,1);
    if (game->dials[0]==2 && game->dials[1]==3 && game->dials[2]==4) {
        game->calibrated=1;
        message(game,0,"All channels aligned. Calibration accepted; both exit controls are enabled.",1);
        return message(game,player,"Calibration accepted. Both players must use their own exit controls.",1);
    }
    return message(game,player,"Test sent. Ask the receiver operator for the result.",1);
}

int game_action(Game *game,int player,const cJSON *action)
{
    const char *type;
    GamePlayer *p;
    int map,direction,x,y;
    char rows[ROOM_H][ROOM_W+1];
    if (!game || player<0 || player>1 || !cJSON_IsObject(action)) return 0;
    p=&game->players[player]; type=field(action,"type");
    if (strcmp(type,"ready")==0) {
        p->ready=1;
        if (!game->started && game->players[0].ready && game->players[1].ready) { game->started=1; start_room(game); }
        return 1;
    }
    if (strcmp(type,"chat")==0) {
        const char *text=field(action,"text"); size_t i,length=strlen(text);
        if (!length || length>120) return message(game,player,"Message must contain 1-120 printable ASCII characters.",0);
        for (i=0;i<length;++i) if ((unsigned char)text[i]<32 || (unsigned char)text[i]>126) return message(game,player,"Message contains unsupported characters.",0);
        snprintf(game->chat,sizeof game->chat,"Player %d: %s",player+1,text); return 1;
    }
    if (!game->started) return message(game,player,"Waiting for both players to be ready.",0);
    if (!number(action,"map",&map) || map!=game->map) return message(game,player,"That action belongs to an old or invalid map stage.",0);
    if (strcmp(type,"move")==0) {
        const char *name=field(action,"direction");
        direction=strcmp(name,"up")==0 ? 0 : strcmp(name,"right")==0 ? 1 : strcmp(name,"down")==0 ? 2 : strcmp(name,"left")==0 ? 3 : -1;
        if (direction<0) return message(game,player,"Unknown movement direction.",0);
        p->facing=direction; p->active_object[0]='\0';
        x=p->x+dx[direction]; y=p->y+dy[direction];
        if (!content_room(game->map,player,rows,game->passage)) return 0;
        if (x<0 || x>=ROOM_W || y<0 || y>=ROOM_H || (rows[y][x]!='.' && rows[y][x]!='@' && rows[y][x]!='*')) return message(game,player,"Blocked. You are now facing that tile; press E to inspect it.",1);
        p->x=x; p->y=y; return message(game,player,"",1);
    }
    if (strcmp(type,"interact")==0) return interact(game,player);
    if (strcmp(type,"submit")==0) return submit(game,player,action);
    if (strcmp(type,"dial")==0) return calibration(game,player,action,0);
    if (strcmp(type,"test")==0) return calibration(game,player,action,1);
    return message(game,player,"Unknown or forbidden action.",0);
}

static cJSON *prompt(const Game *game,int player)
{
    const GamePlayer *p=&game->players[player];
    const char *object=p->active_object,*kind="none",*title="",*text="";
    cJSON *result=cJSON_CreateObject(),*options;
    if (!result) return NULL;
    if (game->map==1 && player==1 && strcmp(object,"order_note")==0 && !game->words) {
        kind="words"; title="The Riddle of the Three Blanks";
        text="First comes the _____. Then the means of _____. The _____ shall always remain last.\nI banish shadows: choose the shining start.\nI bring you through: choose the way of passage.\nI silently guard the end: choose the watcher.\nChoose three words in order, or enter their numbers separated by spaces.";
    }
    else if (game->map==1 && player==1 && strcmp(object,"cabinet")==0 && game->words && !game->cabinet) { kind="answer"; title="Cabinet padlock"; text="Enter the three painting digits in the reconstructed note's order."; }
    else if (game->map==1 && player==0 && strcmp(object,"hatch")==0 && game->cabinet && !game->hatch) { kind="answer"; title="Mathematical hatch"; text="Combine your equations with your partner's instruction. Enter three digits."; }
    else if (game->map==2 && player==1 && (strcmp(object,"dials")==0 || strcmp(object,"test_switch")==0)) { kind="dials"; title="Signal calibration"; text="Set each channel from 0 to 4, then Test. Only your partner receives alignment feedback."; }
    cJSON_AddStringToObject(result,"kind",kind); cJSON_AddStringToObject(result,"object",object);
    cJSON_AddStringToObject(result,"title",title); cJSON_AddStringToObject(result,"text",text);
    if (strcmp(kind,"words")==0) {
        const char *words[6]={"light","passage","watcher","shadow","lock","silence"}; int i;
        options=cJSON_AddArrayToObject(result,"options");
        for (i=0;i<6;++i) cJSON_AddItemToArray(options,cJSON_CreateString(words[i]));
    }
    if (strcmp(kind,"dials")==0) cJSON_AddItemToObject(result,"values",cJSON_CreateIntArray(game->dials,3));
    return result;
}

cJSON *game_view(const Game *game,int player)
{
    const GamePlayer *p;
    cJSON *view,*array;
    char rows[ROOM_H][ROOM_W+1];
    int i;
    if (!game || player<0 || player>1) return NULL;
    p=&game->players[player];
    if (!content_room(game->map,player,rows,game->passage)) return NULL;
    view=cJSON_CreateObject(); if (!view) return NULL;
    cJSON_AddStringToObject(view,"type","view"); cJSON_AddNumberToObject(view,"player",player+1);
    cJSON_AddNumberToObject(view,"map",game->map); cJSON_AddBoolToObject(view,"started",game->started);
    cJSON_AddStringToObject(view,"room",content_room_name(game->map,player));
    array=cJSON_AddArrayToObject(view,"rows");
    for (i=0;i<ROOM_H;++i) { if (i==1) rows[i][1]='.'; cJSON_AddItemToArray(array,cJSON_CreateString(rows[i])); }
    cJSON_AddNumberToObject(view,"x",p->x); cJSON_AddNumberToObject(view,"y",p->y); cJSON_AddNumberToObject(view,"facing",p->facing);
    cJSON_AddStringToObject(view,"feedback",p->feedback); cJSON_AddStringToObject(view,"chat",game->chat);
    array=cJSON_AddArrayToObject(view,"notes");
    for (i=0;i<p->note_count;++i) cJSON_AddItemToArray(array,cJSON_CreateString(p->notes[i]));
    cJSON_AddItemToObject(view,"prompt",prompt(game,player));
    cJSON_AddStringToObject(view,"objective",game->map==3 ? "Maps 1-2 milestone complete. Map 3 puzzles are under development." : game->map==1 ? "Share clues, open the cabinet and hatch, then use both exits." : "Find the passage, calibrate the signal, then use both exits.");
    cJSON_AddBoolToObject(view,"milestone_complete",game->map==3);
    return view;
}
