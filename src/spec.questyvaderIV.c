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

#define MAX_LENGTH 1000

/*
Questy Vader IV - Specs by Sane

when someone dies in rooms 26600 through 26650 they are transfered to room 26201 with 1 hp 1 mana and full mvs

Doors between rooms (26608 and 26638) and (26614 and 26617) close when someone goes through them
*/

#define DEATH_ROOM        26601
#define ZONE_FIRST_ROOM   26600
#define ZONE_LAST_ROOM    26650

#define DOOR_1_ROOM_1     26608
#define DOOR_1_ROOM_2     26638
#define DOOR_2_ROOM_1     26614
#define DOOR_2_ROOM_2     26617

#define DOOR_1_ROOM_1_EXIT  DOWN
#define DOOR_1_ROOM_2_EXIT  UP
#define DOOR_2_ROOM_1_EXIT  UP
#define DOOR_2_ROOM_2_EXIT  DOWN

int getDoorExit(int room)
{
    switch (room)
    {
        case DOOR_1_ROOM_1:
            return DOOR_1_ROOM_1_EXIT;
            break;
        case DOOR_1_ROOM_2:
            return DOOR_1_ROOM_2_EXIT;
            break;
         case DOOR_2_ROOM_1:
            return DOOR_2_ROOM_1_EXIT;
            break;
         case DOOR_2_ROOM_2:
            return DOOR_2_ROOM_2_EXIT;
            break;
    }
    return 0;
}

int getOtherRoom(int room)
{
    switch (room)
    {
        case DOOR_1_ROOM_1:
            return DOOR_1_ROOM_2;
            break;
        case DOOR_1_ROOM_2:
            return DOOR_1_ROOM_1;
            break;
         case DOOR_2_ROOM_1:
            return DOOR_2_ROOM_2;
            break;
         case DOOR_2_ROOM_2:
            return DOOR_2_ROOM_1;
            break;
     }
    return 0;
}

int vaderIV_die(int room, CHAR *ch, int cmd, char *arg)
{
    CHAR *tmp, *opp;
    char buf[MAX_LENGTH];
    int this_room, other_room, door;

    if (!ch) return FALSE;

    if (cmd == MSG_DIE)
    {
        if (IS_NPC(ch)) return FALSE;
        //Stop everyone from fighting him
        for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp=tmp->next_in_room)
            if (GET_OPPONENT(tmp) == ch)
                stop_fighting(tmp);

        act("$n screams as if his very life force is leaving $m, but it is suddenly silent as $e disappears.", FALSE, ch, 0, opp, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, real_room(DEATH_ROOM));
        GET_HIT(ch) = 1;
        GET_MANA(ch) = 0;
        GET_MOVE(ch) = GET_MAX_MOVE(ch);
        /*do_look(ch,"",0);*/
        act("You howl in pain as your life force tears from your body, but then everything is calm as you find yourself in new surroundings.", FALSE, ch, 0, 0, TO_CHAR);
        act("You hear a scream as a mist rolls in and deposits $n.", FALSE, ch, 0, 0, TO_ROOM);

        /*return true so the game stops processing the death*/
        return TRUE;
    }
    else if (cmd == MSG_ENTER)
    {
send_to_world("shit\n\r");
        this_room = world[room].number;
        other_room = getOtherRoom(this_room);

        if (this_room == DOOR_1_ROOM_1 || this_room == DOOR_1_ROOM_2
         || this_room == DOOR_2_ROOM_1 || this_room == DOOR_2_ROOM_2)
        {
            door = getDoorExit(this_room);
            SET_BIT(world[room].dir_option[door]->exit_info, EX_CLOSED);
            SET_BIT(world[world[room].dir_option[door]->to_room_r].dir_option[getDoorExit(other_room)]->exit_info, EX_CLOSED);
            sprintf(buf, "The door closes behind %s.\n\r", GET_NAME(ch));
            send_to_char("The door closes behind you.\n\r", ch);
            send_to_room(buf, this_room);
            send_to_room(buf, other_room);
        } 
    }
    return FALSE;
}

void assign_questyvaderIV(void)
{
    int i;

    for (i = ZONE_FIRST_ROOM; i <= ZONE_LAST_ROOM; ++i)
    {
        assign_room(i, vaderIV_die);
    }
}

