/*
		Specs for The Zan Kingdom, by Mitch
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

/* Room defines */

#define FIELD_HOLE  989
#define GUARD_ROOM  925
#define HOLE_TARGET 901

/* Mob defines */

#define ZAN_GUARD   915
#define FARMER      920

/* Room specs */

int zan_hole(int room, CHAR *ch, int cmd, char *arg)
{
	if(!ch) return FALSE;
	if(cmd == CMD_DOWN)
	{
		act("You step near the hole and accidentally slip into it, falling head over heels.",1,ch,0,0, TO_CHAR);
		act("As $n gets near the hole $e falls right into it, screaming loudly.",1,ch,0,0, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, real_room(HOLE_TARGET));
		do_look(ch, "", CMD_LOOK);
		act("A soft thud can be heard as $n falls down the hole and hits the ground.",1,ch,0,0, TO_ROOM);
		return TRUE;
	}
	return FALSE;
}

/* Mob specs */

int zan_guard(CHAR *guard, CHAR *ch, int cmd, char *arg)
{
	if(cmd == CMD_SOUTH)
	{
		if(!ch || (IS_NPC(ch))) return FALSE;
		act("$n silently steps in $N's way and gives $M a nasty look.",0,guard,0,ch, TO_NOTVICT);
		act("$n silently steps in your way and gives you a nasty look.",0,guard,0,ch, TO_VICT);
		return TRUE;
	}
	return FALSE;
}

int zan_farmer(CHAR *farmer, CHAR *ch, int cmd, char *arg)
{
	char buf[MIL];
	
	if((cmd == MSG_MOBACT) && count_mortals_room(farmer,TRUE))
	{
		if(chance(50))
		{
			switch(number(1,9))
			{
				case 1:
					sprintf(buf,"say Aunt Matilda, since when do you wear pink?");
					break;
				case 2:
					sprintf(buf,"say The giant ants stole my precious poopsie!");
					break;
				case 3:
					sprintf(buf,"say Why does danger end in anger?  It should start with it.");
					break;
				case 4:
					sprintf(buf,"boggle");
					break;
				case 5:
					sprintf(buf,"say My favorite jellybean is the corn ones...");
					break;
				case 6:
					sprintf(buf,"say Ladies, there is plenty of me to go around, no need to fight.");
					break;
				case 7:
					sprintf(buf,"say Where is that Waldo boy?  He dresses in the most outrageous things and I still can never find him.");
					break;
				case 8:
					sprintf(buf,"say My dear, why don't you");
					break;
				case 9:
					sprintf(buf,"say Green? I ain't got a green.");
					break;						
			}
			command_interpreter(farmer, buf);
			return FALSE;
		}
	}
	return FALSE;
}

/* Spec assigns */

void assign_zan(void)
{
	assign_room(FIELD_HOLE , zan_hole);
	assign_mob(ZAN_GUARD   , zan_guard);
	assign_mob(FARMER      , zan_farmer);
}
