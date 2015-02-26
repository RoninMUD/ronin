#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "cmd.h"
#include "utility.h"
#include "act.h"
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"
extern struct time_info_data time_info;

#define MAX_LENGTH 1000

/*
Questy Vader II - Specs by Sane

having item number 26200 in your inv allows you to enter mirror in room 26233

having item number 26201 in you inv allows you to enter mirror in room 26212

when someone picks up items 26200 and 26201 a chaos message saying <whoever> has the Flag of the <which ever side>

when someone dies in rooms 26201 through 26242 they are transfered to room 26200 with 1 hp 1 mana and full mvs

when some dies in rooms 26251 through 26259 then are transfered to room 26250 with 1 hp 1 mana and full mvs
*/

#define MIRROR_FLAG_OBJ_1   26200
#define MIRROR_ROOM_1       26233
#define TEAM_OBJ_1          26202
#define FLAG_ROOM_1         26212

#define MIRROR_FLAG_OBJ_2   26201
#define MIRROR_ROOM_2       26212
#define TEAM_OBJ_2          26203
#define FLAG_ROOM_2         26233

#define DEATH_ROOM_1        26200
#define ZONE_FIRST_ROOM_1   26201
#define ZONE_LAST_ROOM_1    26242

#define DEATH_ROOM_2        26250
#define ZONE_FIRST_ROOM_2   26251
#define ZONE_LAST_ROOM_2    26259

#define TALKY_MOB           26200

OBJ* get_carrying_obj (struct char_data *ch, int virtual) {
  struct obj_data *o;
  for (o=ch->carrying;o;o=o->next_content)
    if (V_OBJ(o) == virtual) return o;
  return NULL;
}

int CTF_mirror_room(int room, CHAR *ch, int cmd, char *arg, int obj)
{
    CHAR *mob;
    OBJ *objptr;
    char buf[MAX_LENGTH];

    /* only if someone did it */
    if (!ch) return FALSE;

    if ((cmd == CMD_ENTER) && !(str_cmp(arg, " mirror")) )
    {
        objptr = get_carrying_obj(ch, obj);
        if (!objptr)
        {
            //They don't have the flag, don't let them in
            act("You can't seem to enter this portal!", FALSE, ch, 0, 0, TO_CHAR);
            return TRUE;
        }

        mob = get_ch_zone(TALKY_MOB, 262) ;
        if (mob)
        {
            sprintf(buf, "%s has captured %s!\n\r", GET_NAME(ch), OBJ_SHORT(objptr));
            do_chaos(mob, buf, CMD_CHAOS);
        }
        GET_HIT(ch) = GET_MAX_HIT(ch);
        GET_MANA(ch) = GET_MAX_MANA(ch);
        GET_MOVE(ch) = GET_MAX_MOVE(ch);
        act("You feel refreshed!", FALSE, ch, 0, 0, TO_CHAR);
    }

    return FALSE;
}

int CTF_die(int room, CHAR *ch, int cmd, char *arg, int transRoom)
{
    CHAR *tmp, *mob;
    OBJ *obj;
    char buf[MAX_LENGTH];

    if (!ch) return FALSE;
    if (IS_NPC(ch)) return FALSE;

    if (cmd == MSG_CORPSE)
    {
        //If they've got the flag, drop it in the room
        obj = ch->carrying;
        while (obj)
        {
            if ((V_OBJ(obj) == MIRROR_FLAG_OBJ_1 || V_OBJ(obj) == MIRROR_FLAG_OBJ_2) && obj->carried_by == ch)
            {
                mob = get_ch_zone(TALKY_MOB, 262) ;
                if (mob)
                {
                    sprintf(buf, "%s died carrying %s at %s!\n\r", GET_NAME(ch), OBJ_SHORT(obj), world[room].name);
                    do_chaos(mob, buf, CMD_CHAOS);
                    obj_from_char(obj);
                    obj_to_room(obj, CHAR_REAL_ROOM(ch));
                }
            }
            obj = obj->next;
        }
        //Stop everyone from fighting him
        for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp=tmp->next_in_room)
            if (GET_OPPONENT(tmp) == ch)
                stop_fighting(tmp);

        act("$n screams as if his very life force is leaving $m, but it is suddenly silent as $e disappears.", FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, real_room(transRoom));
        GET_HIT(ch) = 1;
        GET_MANA(ch) = 0;
        GET_MOVE(ch) = GET_MAX_MOVE(ch);
        /*do_look(ch,"",0);*/
        act("You howl in pain as your life force tears from your body, but then everything is calm as you find yourself in new surroundings.", FALSE, ch, 0, 0, TO_CHAR);
        act("You hear a scream as a mist rolls in and deposits $n.", FALSE, ch, 0, 0, TO_ROOM);

        /*return true so the game stops processing the death*/
        return TRUE;
    }

    return FALSE;
}

int CTF_room_1(int room, CHAR *ch, int cmd, char *arg)
{
    if (world[room].number == MIRROR_ROOM_1)
    {
        if (CTF_mirror_room(room, ch, cmd, arg, MIRROR_FLAG_OBJ_1))
            return TRUE;
    }

    if (world[room].number == MIRROR_ROOM_2)
    {
        if (CTF_mirror_room(room, ch, cmd, arg, MIRROR_FLAG_OBJ_2))
            return TRUE;
    }

    return CTF_die(room, ch, cmd, arg, DEATH_ROOM_1);
}

int CTF_room_2(int room, CHAR *ch, int cmd, char *arg)
{
    return CTF_die(room, ch, cmd, arg, DEATH_ROOM_2);
}

int flag_pickup(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
    CHAR *mob;
    char buf[MAX_LENGTH];

    if (cmd == MSG_GET)
    {
        mob = get_ch_zone(TALKY_MOB, 262);
        if ((V_OBJ(obj) == MIRROR_FLAG_OBJ_1 && count_carrying_obj(ch, TEAM_OBJ_1))
            || (V_OBJ(obj) == MIRROR_FLAG_OBJ_2 && count_carrying_obj(ch, TEAM_OBJ_2)))
        {
            if (mob)
            {
                sprintf(buf, "%s returned the %s", !IS_NPC(ch) ? MOB_NAME(ch) : MOB_SHORT(ch), OBJ_SHORT(obj));
                do_chaos(mob, buf, CMD_CHAOS);
            }
            obj_from_room(obj);
            obj_to_room(obj, real_room(V_OBJ(obj) == MIRROR_FLAG_OBJ_1 ? FLAG_ROOM_1 : FLAG_ROOM_2));
            return TRUE;
        }
        else
        {
            if (mob)
            {
                sprintf(buf, "%s has picked up the %s!", !IS_NPC(ch) ? MOB_NAME(ch) : MOB_SHORT(ch), OBJ_SHORT(obj));
                do_chaos(mob, buf, CMD_CHAOS);
            }
        }
    }

    return FALSE;
}

void assign_questyvaderII(void)
{
    int i;

    for (i = ZONE_FIRST_ROOM_1; i <= ZONE_LAST_ROOM_1; ++i)
    {
        assign_room(i, CTF_room_1);
    }
    for (i = ZONE_FIRST_ROOM_2; i <= ZONE_LAST_ROOM_2; ++i)
    {
        assign_room(i, CTF_room_2);
    }

    assign_obj(MIRROR_FLAG_OBJ_1, flag_pickup);
    assign_obj(MIRROR_FLAG_OBJ_2, flag_pickup);
}



