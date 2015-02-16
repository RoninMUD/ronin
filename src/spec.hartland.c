/*
		Specs for Hartland, by Arodtanjoe
		Written by Solmyr for RoninMUD
*/

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

#define BOUNDARY_WARDEN 14204
#define ROAD_ROOM_ONE   14225
#define ROAD_ROOM_TWO   14226
#define COURTYARD       14227

int hartland_block(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	if((cmd == CMD_WEST) && (world[CHAR_REAL_ROOM(mob)].number == ROAD_ROOM_ONE))
	{
		if(IS_MORTAL(ch))
		{
			act("$n places $mself in $N's way.",0,mob,0,ch, TO_NOTVICT);
			act("$n places $mself in your way.",0,mob,0,ch, TO_VICT);
			act("You place yourself in $N's way.",0,mob,0,ch, TO_CHAR);
			do_say(mob, "You are not allowed to see the First Councellor", CMD_SAY);
			return TRUE;
		}
		return FALSE;
	}
	if((cmd == CMD_SOUTH) && (world[CHAR_REAL_ROOM(mob)].number == ROAD_ROOM_TWO))
	{
		if(IS_MORTAL(ch))
		{
			act("$n places $mself in $N's way.",0,mob,0,ch, TO_NOTVICT);
			act("$n places $mself in your way.",0,mob,0,ch, TO_VICT);
			act("You place yourself in $N's way.",0,mob,0,ch, TO_CHAR);
			do_say(mob, "You are not allowed to see the First Councellor", CMD_SAY);
			return TRUE;
		}
		return FALSE;
	}
	if(((cmd == CMD_EAST) || (cmd == CMD_SOUTH) || (cmd == CMD_WEST)) && (world[CHAR_REAL_ROOM(mob)].number == COURTYARD))
	{
		if(IS_MORTAL(ch))
		{
			act("$n places $mself in $N's way.",0,mob,0,ch, TO_NOTVICT);
			act("$n places $mself in your way.",0,mob,0,ch, TO_VICT);
			act("You place yourself in $N's way.",0,mob,0,ch, TO_CHAR);
			do_say(mob, "You are not allowed to see the First Councellor", CMD_SAY);
			return TRUE;
		}
	}
	return FALSE;
}

void assign_hartland(void)
{
	assign_mob(BOUNDARY_WARDEN, hartland_block);
}
