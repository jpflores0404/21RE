#include "app.h"
#include "game.h"
#include "net.h"
#include "platform.h"
#include "protocol.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Socket socket;
    char incoming[VIEW_LIMIT + 1], outgoing[OUTPUT_LIMIT];
    size_t in_count, out_count, out_sent;
    cJSON *view;
    int welcomed, ended, mode, pending_open, selected_dial;
    unsigned long view_generation;
    char message[256], input[121], object[33];
    const char *expected_session;
} Client;

static const char *string_field(const cJSON *object, const char *name)
{
    const cJSON *value = cJSON_GetObjectItemCaseSensitive(object, name);
    return cJSON_IsString(value) ? value->valuestring : "";
}
static int number_field(const cJSON *object, const char *name)
{
    const cJSON *value = cJSON_GetObjectItemCaseSensitive(object, name);
    return cJSON_IsNumber(value) ? value->valueint : 0;
}
static int text_safe(const cJSON *value, size_t limit)
{
    const unsigned char *text;
    size_t i;
    if (!cJSON_IsString(value) || strlen(value->valuestring) > limit) return 0;
    text = (const unsigned char *)value->valuestring;
    for (i = 0; text[i]; ++i) if ((text[i] < 32 && text[i] != '\n') || text[i] > 126) return 0;
    return 1;
}
static int integer_field(const cJSON *object, const char *name, int low, int high)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, name);
    return cJSON_IsNumber(item) && item->valuedouble >= low && item->valuedouble <= high && item->valuedouble == item->valueint;
}
static int valid_view(const cJSON *view)
{
    const cJSON *rows = cJSON_GetObjectItemCaseSensitive(view, "rows");
    const cJSON *notes = cJSON_GetObjectItemCaseSensitive(view, "notes");
    const cJSON *prompt = cJSON_GetObjectItemCaseSensitive(view, "prompt");
    const cJSON *item;
    const char *kind;
    if (!integer_field(view,"player",1,2) || !integer_field(view,"map",1,3) ||
        !integer_field(view,"x",0,ROOM_W-1) || !integer_field(view,"y",0,ROOM_H-1) ||
        !integer_field(view,"facing",0,3) || !cJSON_IsBool(cJSON_GetObjectItemCaseSensitive(view,"started")) ||
        !cJSON_IsBool(cJSON_GetObjectItemCaseSensitive(view,"milestone_complete")) ||
        !text_safe(cJSON_GetObjectItemCaseSensitive(view,"room"),80) ||
        !text_safe(cJSON_GetObjectItemCaseSensitive(view,"feedback"),512) ||
        !text_safe(cJSON_GetObjectItemCaseSensitive(view,"objective"),512) ||
        !text_safe(cJSON_GetObjectItemCaseSensitive(view,"chat"),160) ||
        !cJSON_IsArray(rows) || cJSON_GetArraySize(rows)!=ROOM_H || !cJSON_IsArray(notes) ||
        cJSON_GetArraySize(notes)>NOTE_COUNT || !cJSON_IsObject(prompt)) return 0;
    cJSON_ArrayForEach(item,rows) if (!text_safe(item,ROOM_W) || strlen(item->valuestring)!=ROOM_W || strchr(item->valuestring,'\n')) return 0;
    cJSON_ArrayForEach(item,notes) if (!text_safe(item,256)) return 0;
    if (!text_safe(cJSON_GetObjectItemCaseSensitive(prompt,"kind"),16)) return 0;
    kind=string_field(prompt,"kind");
    if (strcmp(kind,"none") && strcmp(kind,"words") && strcmp(kind,"answer") && strcmp(kind,"dials")) return 0;
    if (strcmp(kind,"none")) {
        if (!text_safe(cJSON_GetObjectItemCaseSensitive(prompt,"object"),32) ||
            !text_safe(cJSON_GetObjectItemCaseSensitive(prompt,"title"),80) ||
            !text_safe(cJSON_GetObjectItemCaseSensitive(prompt,"text"),1024)) return 0;
    }
    if (!strcmp(kind,"dials")) {
        const cJSON *values=cJSON_GetObjectItemCaseSensitive(prompt,"values");
        if (!cJSON_IsArray(values) || cJSON_GetArraySize(values)!=3) return 0;
        cJSON_ArrayForEach(item,values) if (!cJSON_IsNumber(item) || item->valuedouble<0 || item->valuedouble>4 || item->valuedouble!=item->valueint) return 0;
    }
    if (!strcmp(kind,"words")) {
        const cJSON *options=cJSON_GetObjectItemCaseSensitive(prompt,"options");
        if (!cJSON_IsArray(options) || cJSON_GetArraySize(options)>12) return 0;
        cJSON_ArrayForEach(item,options) if (!text_safe(item,64)) return 0;
    }
    return 1;
}
static int queue(Client *client, cJSON *action)
{
    char line[ACTION_LIMIT+2];
    int length;
    if (!action) return 0;
    length=protocol_print(action,line,sizeof line);
    cJSON_Delete(action);
    if (length<=0 || client->out_count+(size_t)length>sizeof client->outgoing) return 0;
    memcpy(client->outgoing+client->out_count,line,(size_t)length); client->out_count+=(size_t)length;
    return 1;
}
static cJSON *action(Client *client, const char *type, int map)
{
    cJSON *message=cJSON_CreateObject();
    if (!message || !cJSON_AddStringToObject(message,"type",type) ||
        (map && !cJSON_AddNumberToObject(message,"map",number_field(client->view,"map")))) {
        cJSON_Delete(message); return NULL;
    }
    return message;
}
static void end_client(Client *client, const char *message)
{
    client->ended=1;
    snprintf(client->message,sizeof client->message,"%s",message);
}
static int receive_message(Client *client, const char *line, size_t length)
{
    const char *end;
    cJSON *message=cJSON_ParseWithLengthOpts(line,length+1,&end,1);
    const char *type;
    if (!message || !cJSON_IsObject(message)) { cJSON_Delete(message); return 0; }
    type=string_field(message,"type");
    if (!strcmp(type,"welcome")) {
        if (client->welcomed || !integer_field(message,"player",1,2) ||
            !text_safe(cJSON_GetObjectItemCaseSensitive(message,"session"),64) ||
            (client->expected_session && strcmp(client->expected_session,string_field(message,"session")))) {
            cJSON_Delete(message); return 0;
        }
        client->welcomed=1;
        if (!queue(client,action(client,"ready",0))) { cJSON_Delete(message); return 0; }
    } else if (!strcmp(type,"view")) {
        if (!client->welcomed || !valid_view(message)) { cJSON_Delete(message); return 0; }
        if (client->view && number_field(client->view,"map")!=number_field(message,"map")) {
            client->mode=0;client->pending_open=0;client->input[0]='\0';
        }
        cJSON_Delete(client->view);client->view=message;++client->view_generation;
        if (client->pending_open) {
            const cJSON *prompt=cJSON_GetObjectItemCaseSensitive(message,"prompt");
            const char *kind=string_field(prompt,"kind");
            if (strcmp(kind,"none")) {
                client->mode=!strcmp(kind,"dials")?3:2;client->pending_open=0;
                snprintf(client->object,sizeof client->object,"%s",string_field(prompt,"object"));
                client->input[0]='\0';
            }
        }
        return 1;
    } else if (!strcmp(type,"session_end") || !strcmp(type,"error")) {
        const cJSON *reason=cJSON_GetObjectItemCaseSensitive(message,"message");
        end_client(client,text_safe(reason,255)?reason->valuestring:"The session ended.");
    } else { cJSON_Delete(message); return 0; }
    cJSON_Delete(message);return 1;
}
static int pump(Client *client)
{
    int count, turns=0;
    while (client->out_sent<client->out_count) {
        count=net_write(client->socket,client->outgoing+client->out_sent,client->out_count-client->out_sent);
        if (count==-1) break;
        if (count<=0) return 0;
        client->out_sent+=(size_t)count;
    }
    if (client->out_sent==client->out_count) client->out_sent=client->out_count=0;
    while (turns++<16) {
        char *newline=memchr(client->incoming,'\n',client->in_count);
        if (newline) {
            size_t length=(size_t)(newline-client->incoming);
            client->incoming[length]='\0';
            if (!receive_message(client,client->incoming,length)) return 0;
            memmove(client->incoming,newline+1,client->in_count-length-1);
            client->in_count-=length+1;
            if (client->ended) return 1;
            continue;
        }
        if (client->in_count==VIEW_LIMIT) return 0;
        count=net_read(client->socket,client->incoming+client->in_count,VIEW_LIMIT-client->in_count);
        if (count==-1) break;
        if (count<=0) return 0;
        client->in_count+=(size_t)count;client->incoming[client->in_count]='\0';
    }
    return 1;
}
static void append(char *buffer, size_t capacity, const char *format, ...)
{
    size_t length=strlen(buffer);
    va_list args;
    if (length>=capacity-1) return;
    va_start(args,format);vsnprintf(buffer+length,capacity-length,format,args);va_end(args);
}
static int show(const char *text)
{
    int width,height,x=0,y=0;
    char clipped[24576];size_t length=0;
    if (!term_size(&width,&height)) return 0;
    while (*text && y<height-1 && length<sizeof clipped-2) {
        if (*text=='\n') { clipped[length++]='\n';++y;x=0; }
        else {
            if (x>=width-1) {clipped[length++]='\n';++y;x=0;if(y>=height-1)break;}
            clipped[length++]=*text;++x;
        }
        ++text;
    }
    clipped[length]='\0';return term_draw(clipped);
}
static const char *tutorial(void)
{
    return "TUTORIAL\n\nTwo people, two different rooms. Share clues to progress.\n\n"
        "WASD / arrows: move. Walking into an object changes your facing.\n"
        "E: interact with the object directly ahead. I: discovered notes.\n"
        "T: send a short message to your partner.\n"
        "Enter: confirm. Escape: close a view or cancel input. Q: leave game.\n\n"
        "Numbers and instructions are split between rooms. Read objects with E.\n"
        "Wrong answers can be retried. Both players must activate their own exit.\n"
        "For word choices, enter three option numbers separated by spaces.\n"
        "For dials, choose channel 1/2/3, adjust with +/- and Enter to test.\n\n"
        "This milestone finishes Maps 1 and 2. Map 3 is a walkable preview.\n"
        "It does not yet contain gas, final puzzles or campaign victory.\n\nEscape: back\n";
}
static void draw_game(Client *client)
{
    char text[24576]="";
    const cJSON *view=client->view,*item;
    int i,width,height;
    if (!term_size(&width,&height)) {end_client(client,"Terminal size query failed.");return;}
    if (width<80 || height<24) {show("Enlarge terminal to at least 80 columns by 24 rows.\nNetwork updates continue. Q: leave\n");return;}
    if (!view) {show("Connecting to the session... Q: cancel\n");return;}
    append(text,sizeof text,"2R1E | Player %d | Map %d | %s | Facing %s\n",number_field(view,"player"),number_field(view,"map"),string_field(view,"room"),
        (const char *[]) {"North","East","South","West"}[number_field(view,"facing")]);
    append(text,sizeof text,"%s\n",string_field(view,"objective"));
    if (client->mode==1) {
        append(text,sizeof text,"\nDISCOVERED NOTES (Escape: back)\n");
        cJSON_ArrayForEach(item,cJSON_GetObjectItemCaseSensitive(view,"notes")) append(text,sizeof text,"- %s\n",item->valuestring);
    } else if (client->mode==5) append(text,sizeof text,"%s",tutorial());
    else {
        const cJSON *rows=cJSON_GetObjectItemCaseSensitive(view,"rows");
        for (i=0;i<ROOM_H && client->mode!=2 && client->mode!=3;++i) {
            char row[ROOM_W+1];snprintf(row,sizeof row,"%s",cJSON_GetArrayItem(rows,i)->valuestring);
            if (i==number_field(view,"y")) row[number_field(view,"x")]='@';
            append(text,sizeof text,"%s\n",row);
        }
        append(text,sizeof text,"%s\n",string_field(view,"feedback"));
        if (client->mode==2 || client->mode==3) {
            const cJSON *prompt=cJSON_GetObjectItemCaseSensitive(view,"prompt");
            append(text,sizeof text,"%s: %s\n",string_field(prompt,"title"),string_field(prompt,"text"));
            if (client->mode==2) {
                int n=0;cJSON_ArrayForEach(item,cJSON_GetObjectItemCaseSensitive(prompt,"options")) append(text,sizeof text,"%d=%s ",++n,item->valuestring);
                append(text,sizeof text,"\nAnswer: %s_ (Enter submit / Esc cancel)\n",client->input);
            } else {
                const cJSON *values=cJSON_GetObjectItemCaseSensitive(prompt,"values");
                append(text,sizeof text,"A=%d B=%d C=%d | selected %c | 1/2/3 select, +/- adjust, Enter test\n",
                    cJSON_GetArrayItem(values,0)->valueint,cJSON_GetArrayItem(values,1)->valueint,cJSON_GetArrayItem(values,2)->valueint,'A'+client->selected_dial);
            }
        } else if (client->mode==4) append(text,sizeof text,"Message: %s_ (Enter send / Esc cancel)\n",client->input);
        else append(text,sizeof text,"WASD move | E use | I notes | T message | H help | Q leave\n");
    }
    append(text,sizeof text,"Partner messages: %s\n",string_field(view,"chat"));
    if (!show(text)) end_client(client,"Terminal drawing failed.");
}
static int input_key(Client *client, int key)
{
    cJSON *message;
    if (key==KEY_ESCAPE) {client->mode=0;client->pending_open=0;client->input[0]='\0';return 1;}
    if (client->mode==2 || client->mode==4) {
        size_t length=strlen(client->input),limit=client->mode==4?120:64;
        if (key==KEY_BACKSPACE && length) client->input[length-1]='\0';
        else if (key==KEY_ENTER) {
            message=action(client,client->mode==4?"chat":"submit",client->mode==4?0:1);
            if (!message) return 0;
            if (client->mode==2 && !cJSON_AddStringToObject(message,"object",client->object)) {cJSON_Delete(message);return 0;}
            if (!cJSON_AddStringToObject(message,client->mode==4?"text":"answer",client->input)) {cJSON_Delete(message);return 0;}
            if (!queue(client,message)) return 0;
            client->input[0]='\0';client->mode=0;
        } else if (key>=32 && key<=126 && length<limit) {client->input[length]=(char)key;client->input[length+1]='\0';}
        return 1;
    }
    if (key=='q' || key=='Q') {queue(client,action(client,"quit",0));end_client(client,"You left the session.");return 1;}
    if (client->mode==3) {
        if (key>='1' && key<='3') client->selected_dial=key-'1';
        else if (key==KEY_ENTER) return queue(client,action(client,"test",1));
        else if (key=='+' || key=='-' || key=='=') {
            const cJSON *values=cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(client->view,"prompt"),"values");
            const cJSON *value=cJSON_GetArrayItem(values,client->selected_dial);
            int next;
            if (!cJSON_IsNumber(value)) return 0;
            next=value->valueint+(key=='-'?-1:1);if(next<0)next=0;if(next>4)next=4;
            message=action(client,"dial",1);if(!message)return 0;
            cJSON_AddNumberToObject(message,"dial",client->selected_dial);cJSON_AddNumberToObject(message,"value",next);
            return queue(client,message);
        }
        return 1;
    }
    if (key=='i'||key=='I') {client->mode=1;return 1;}
    if (key=='h'||key=='H') {client->mode=5;return 1;}
    if (key=='t'||key=='T') {client->mode=4;client->input[0]='\0';return 1;}
    if (client->mode) return 1;
    if (key=='e'||key=='E') {client->pending_open=1;return queue(client,action(client,"interact",1));}
    {
        const char *direction=NULL;
        if(key=='w'||key=='W'||key==KEY_UP)direction="up";
        if(key=='d'||key=='D'||key==KEY_RIGHT)direction="right";
        if(key=='s'||key=='S'||key==KEY_DOWN)direction="down";
        if(key=='a'||key=='A'||key==KEY_LEFT)direction="left";
        if(direction) {client->pending_open=0;message=action(client,"move",1);if(!message)return 0;cJSON_AddStringToObject(message,"direction",direction);return queue(client,message);}
    }
    return 1;
}
static int ask(const char *title, char *value, size_t capacity)
{
    int key;
    while (!platform_stopping()) {
        char text[512];size_t length=strlen(value);
        snprintf(text,sizeof text,"%s\n\n%s_\n\nEnter: confirm | Escape: cancel | Backspace: edit\n",title,value);
        if (!show(text)) return 0;
        key=term_key(50);
        if(key<0||key==KEY_ESCAPE)return 0;
        if(key==KEY_ENTER)return 1;
        if(key==KEY_BACKSPACE&&length)value[length-1]='\0';
        else if(key>=32&&key<=126&&length+1<capacity){value[length]=(char)key;value[length+1]='\0';}
    }
    return 0;
}
static void notice(const char *message)
{
    char text[512];snprintf(text,sizeof text,"%s\n\nEnter or Escape: return to menu\n",message);show(text);
    while(!platform_stopping()){int key=term_key(50);if(key<0||key==KEY_ENTER||key==KEY_ESCAPE)break;}
}
static void session(const char *executable, int hosting)
{
    Client client;
    Child child={0,0};
    char address[32]="127.0.0.1",number[8]="5050",session_id[64],*end;
    long port;
    uint64_t deadline;
    int key,width=0,height=0,old_width=0,old_height=0;
    memset(&client,0,sizeof client);client.socket=NO_SOCKET;
    if(!ask(hosting?"Host bind IPv4 (127.0.0.1 same device; 0.0.0.0 enables LAN)":"Join host IPv4 address",address,sizeof address)||
       !ask("TCP port (1024 through 65535)",number,sizeof number))return;
    port=strtol(number,&end,10);
    if(!*number||*end||port<1024||port>65535){notice("Invalid port.");return;}
    {const char *p=address;while(*p){if((*p<'0'||*p>'9')&&*p!='.'){notice("Use a numeric IPv4 address.");return;}++p;}}
    if(hosting){
        snprintf(session_id,sizeof session_id,"host-%llu",(unsigned long long)platform_millis());
        if(!platform_spawn(&child,executable,address,(int)port,session_id)){notice("Could not start the background server.");return;}
        client.expected_session=session_id;
        deadline=platform_millis()+4000;
        do {
            platform_sleep(50);
            if(!platform_child_running(&child))break;
            client.socket=net_connect(!strcmp(address,"0.0.0.0")?"127.0.0.1":address,(int)port,150);
        }while(client.socket==NO_SOCKET&&platform_millis()<deadline&&!platform_stopping());
    }else client.socket=net_connect(address,(int)port,2000);
    if(client.socket==NO_SOCKET){platform_child_stop(&child);notice("Connection failed. Check address/port, busy port and server availability.");return;}
    deadline=platform_millis()+5000;
    while(!client.ended&&!platform_stopping()){
        unsigned long before=client.view_generation;
        if(!pump(&client)){end_client(&client,"Connection closed or an invalid server message was received.");break;}
        if(!client.welcomed&&platform_millis()>deadline){end_client(&client,"Server handshake timed out.");break;}
        if(!term_size(&width,&height)){end_client(&client,"Terminal size query failed.");break;}
        if(before!=client.view_generation||width!=old_width||height!=old_height){draw_game(&client);old_width=width;old_height=height;}
        key=term_key(25);
        if(key<0){end_client(&client,"Terminal input failed.");break;}
        if(key){
            if(!client.view||width<80||height<24){if(key=='q'||key=='Q'){if(client.welcomed)queue(&client,action(&client,"quit",0));end_client(&client,"You left the session.");}}
            else if(!input_key(&client,key)){end_client(&client,"Input queue could not accept this action.");}
            draw_game(&client);
        }
    }
    /* Give a normal quit one bounded opportunity to reach the referee. */
    deadline=platform_millis()+150;
    while(client.out_sent<client.out_count&&platform_millis()<deadline){if(!pump(&client))break;platform_sleep(5);}
    net_close(client.socket);cJSON_Delete(client.view);platform_child_stop(&child);
    if(!platform_stopping())notice(client.message[0]?client.message:"Session ended.");
}
int client_run(const char *executable)
{
    int selected=0,key,result=0,previous=-1,old_width=0,old_height=0;
    const char *entries[]={"Host game","Join game","Tutorial","Quit"};
    if(!net_init()){fputs("Socket initialization failed.\n",stderr);return 1;}
    if(!term_open()){term_close();net_cleanup();fputs("Run in an interactive Windows or Linux terminal.\n",stderr);return 1;}
    while(!platform_stopping()){
        char text[1024]="2R1E | TWO ROOMS ONE EXIT\n\nMaps 1 and 2 milestone\n\n";int i;
        for(i=0;i<4;++i)append(text,sizeof text,"%s %s\n",i==selected?">":" ",entries[i]);
        append(text,sizeof text,"\nW/S or arrows: select | Enter: confirm | Q: quit\n\nHost and Join use two visible terminals.\nStart with 127.0.0.1 and port 5050 on this computer.\n");
        {
            int width,height;
            if(!term_size(&width,&height)){result=1;break;}
            if(previous!=selected||old_width!=width||old_height!=height){
                if(!show(text)){result=1;break;}
                previous=selected;old_width=width;old_height=height;
            }
        }
        key=term_key(100);
        if(key<0){result=1;break;}
        if(key=='q'||key=='Q')break;
        if(key=='w'||key=='W'||key==KEY_UP)selected=(selected+3)%4;
        if(key=='s'||key=='S'||key==KEY_DOWN)selected=(selected+1)%4;
        if(key==KEY_ENTER){
            if(selected==3)break;
            if(selected==2){show(tutorial());while(!platform_stopping()){key=term_key(50);if(key<0||key==KEY_ESCAPE)break;}}
            else session(executable,selected==0);
            previous=-1;
        }
    }
    term_close();net_cleanup();return result;
}
