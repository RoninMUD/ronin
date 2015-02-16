/*

	Specs for Subterra by Ursum
	Written by Solmyr for RoninMUD
	
*/

/* Includes */

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

/* Defines */

#define SCORPION_ONE  11906
#define SCORPION_TWO  11907
#define PILLBUG       11908
#define TIGER         11910
#define CONSTRICTOR   11911
#define BUMBLEBEE     11912
#define LESTODON      11914

#define PILLBUG_WEST  11938
#define PILLBUG_SOUTH 11940
#define PILLBUG_EAST  11942
#define PILLBUG_NORTH 11944

#define SCORPION_ROOM 11945
#define ALTAR_ROOM    11946
#define POOL_ROOM     11947

#define PH_ITEM_ONE   11903
#define PH_ITEM_TWO   11904
#define PH_ITEM_THREE 11905
#define PH_ITEM_FOUR  11906

/* Extern structs */

struct char_data *get_random_vict(struct char_data *ch);

int subterra_pillbug(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	int which_room, new_room;
	
	if(cmd!=MSG_MOBACT) return FALSE;
	if(!mob->specials.fighting) return FALSE;
	
	switch (V_ROOM(mob))
	{
		case SCORPION_ROOM:
			if(chance(20))
			{
				which_room = number(1,4);
				switch (which_room)
				{
					case 1:
						new_room = PILLBUG_NORTH;
						act("The rolly-polly Pillbug curls into a ball and rolls north.",1,mob,0,0,TO_ROOM);
						break;
					
					case 2:
						new_room = PILLBUG_EAST;
						act("The rolly-polly Pillbug curls into a ball and rolls east.",1,mob,0,0,TO_ROOM);
						break;
					
					case 3:
						new_room = PILLBUG_SOUTH;
						act("The rolly-polly Pillbug curls into a ball and rolls south.",1,mob,0,0,TO_ROOM);
						break;
						
					case 4:
						new_room = PILLBUG_WEST;
						act("The rolly-polly Pillbug curls into a ball and rolls west.",1,mob,0,0,TO_ROOM);
						break;
				}
				char_from_room(mob);
				char_to_room(mob, real_room(new_room));
			}
			break;
			
		case PILLBUG_NORTH:
		case PILLBUG_EAST:
		case PILLBUG_SOUTH:
		case PILLBUG_WEST:				
			act("The rolly-polly Pillbug curls into a ball and rolls back to the center of the tunnels.",1,mob,0,0,TO_ROOM);
			char_from_room(mob);
			char_to_room(mob, real_room(SCORPION_ROOM));
			break;
	}
	return FALSE;
}

int subterra_scorpion_room(int room, CHAR *ch, int cmd, char *arg)
{
	CHAR *mob;
	CHAR *victim;
	OBJ *obj;
	char buf[MIL], targ[MIL];
	
	if(!cmd) return FALSE;
	if(!ch) return FALSE;
	if(V_ROOM(ch) != SCORPION_ROOM) return FALSE;
	
	if(cmd == MSG_SPELL_CAST)
	{
		argument_interpreter(arg, buf, targ);
		if(*targ)
		{
			one_argument(targ, buf);
			if(!(victim = get_char_room_vis(ch, buf))) return FALSE;
			if((V_MOB(victim) == SCORPION_ONE) || (V_MOB(victim) == SCORPION_TWO))
			{
				if(chance(20))
				{	
					act("$N is struck by $n's stinger and the neurotoxin freezes $M in place.",1,mob,0,ch,TO_NOTVICT);
					act("You are struck by $n's stinger and your brain slows and clouds until you cannot move.",1,mob,0,ch,TO_VICT);
					damage(mob,ch,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
					WAIT_STATE(ch, PULSE_VIOLENCE*4);
				}
			}			
		}
	}
	if(cmd == CMD_USE)
	{
    if(EQ(ch,HOLD))  obj = EQ(ch,HOLD);
    if(!EQ(ch,HOLD) || EQ(ch,HOLD) != obj) return FALSE;
    one_argument (arg, buf);
    if(!isname(buf, OBJ_NAME(obj))) return FALSE;
    if(!(GET_ITEM_TYPE(obj) == ITEM_STAFF) || (GET_ITEM_TYPE(obj) == ITEM_WAND)) return FALSE;
		send_to_char("Your staff feels powerless in this room",ch);
		act("$n attempts to use $p but an aura in the room prevents it.",0,ch,obj,0,TO_ROOM);
	}
	return FALSE;
}

int subterra_yellow_scorpion(CHAR *scorpion, CHAR *ch, int cmd, char *arg)
{
	CHAR *victim;
	struct affected_type_5 af;
	
	if(cmd == MSG_ZONE_RESET)
	{
  	if(affected_by_spell(victim, SPELL_FORTIFICATION)) return FALSE;
  	af.type      = SPELL_FORTIFICATION;
  	af.duration  = -1;
  	af.modifier  = 0;
  	af.location  = APPLY_NONE;
  	af.bitvector = 0;
  	af.bitvector2 = 0;
  	affect_to_char(scorpion, &af);
	}
	return FALSE;
}

int subterra_tiger(CHAR *tiger, CHAR *ch, int cmd, char *arg)
{
	CHAR *victim;
	
	if(!ch) return FALSE;
	if(cmd == MSG_MOBACT && !tiger->specials.fighting)
	{
		victim = get_random_vict(tiger);
		act("You fell into an ambush by $n.",0,tiger,0,ch,TO_VICT);
		act("$N fell into a cunning ambush by $n.",0,tiger,0,ch,TO_NOTVICT);
		damage(tiger, victim, 2500, TYPE_UNDEFINED, DAM_NO_BLOCK);
		set_fighting(tiger, victim);
	}
	return FALSE;
}

int subterra_constrictor(CHAR *snake, CHAR *ch, int cmd, char *arg)
{
	CHAR *victim;
	static char status = 0;
	
	if(cmd == MSG_MOBACT && snake->specials.fighting)
	{
		victim = snake->specials.fighting;
		switch (status)
		{
		case 0:
			act("$n wraps around $N, tightening its grip.",0,snake,0,victim,TO_NOTVICT);
			act("You begin to suffocate as $n's grip grows even tighter.",0,snake,0,victim,TO_VICT);
			damage(snake, victim, 500, TYPE_UNDEFINED, DAM_NO_BLOCK);
			status = 1;	
			break;
		case 1:
			act("$n wraps around $N, tightening its grip.",0,snake,0,victim,TO_NOTVICT);
			act("You begin to suffocate as $n's grip grows even tighter.",0,snake,0,victim,TO_VICT);
			if(GET_CON(victim) >= 20)
				damage(snake, victim, 500, TYPE_UNDEFINED, DAM_NO_BLOCK);
			else
				damage(snake, victim, 1000, TYPE_UNDEFINED, DAM_NO_BLOCK);
			status = 2;
			break;
		case 2:
			act("$n wraps around $N, tightening its grip.",0,snake,0,victim,TO_NOTVICT);
			act("You begin to suffocate as $n's grip grows even tighter.",0,snake,0,victim,TO_VICT);
			if(GET_CON(victim) >= 20)
				damage(snake, victim, 500, TYPE_UNDEFINED, DAM_NO_BLOCK);
			else
				damage(snake, victim, 2000, TYPE_UNDEFINED, DAM_NO_BLOCK);
			status = 3;
			break;
		case 3:
			act("$n wraps around $N, tightening its grip.",0,snake,0,victim,TO_NOTVICT);
			act("You begin to suffocate as $n's grip grows even tighter.",0,snake,0,victim,TO_VICT);
			if(GET_CON(victim) >= 20)
				damage(snake, victim, 500, TYPE_UNDEFINED, DAM_NO_BLOCK);
			else
				damage(snake, victim, 3000, TYPE_UNDEFINED, DAM_NO_BLOCK);
			status = 4;
			break;
		case 4:
			act("$n wraps around $N, tightening its grip.",0,snake,0,victim,TO_NOTVICT);
			act("You begin to suffocate as $n's grip grows even tighter.",0,snake,0,victim,TO_VICT);
			if(GET_CON(victim) >= 20)
				damage(snake, victim, 500, TYPE_UNDEFINED, DAM_NO_BLOCK);
			else
				damage(snake, victim, 4000, TYPE_UNDEFINED, DAM_NO_BLOCK);
			status = 4;
			break;
		case 5:
			act("$n wraps around $N, tightening its grip.",0,snake,0,victim,TO_NOTVICT);
			act("You begin to suffocate as $n's grip grows even tighter.",0,snake,0,victim,TO_VICT);
			if(GET_CON(victim) >= 20)
				damage(snake, victim, 500, TYPE_UNDEFINED, DAM_NO_BLOCK);
			else
				damage(snake, victim, 5000, TYPE_UNDEFINED, DAM_NO_BLOCK);
			status = 4;
			break;
		}
	}
	return FALSE;
}

int subterra_bumblebee(CHAR *bee, CHAR *ch, int cmd, char *arg)
{
	CHAR *victim;
	
	if(cmd == MSG_MOBACT && bee->specials.fighting && chance(30))
	{
		victim = get_random_victim_fighting(bee);
		act("$n drives its stinger deep into $N's flesh and collapses lifeless to the ground.",0,bee,0,victim,TO_NOTVICT);
		act("$n drives its stinger deep into your flesh and collapses lifeless to the ground.",0,bee,0,victim,TO_VICT);
		/* kill her here and leave corpse */
		damage(bee, victim, 1000, TYPE_UNDEFINED, DAM_NO_BLOCK);
	}
	return FALSE;
}

int subterra_lestodon(CHAR *lestodon, CHAR *ch, int cmd, char *arg)
{
	char buf[MIL];
	
	if(!ch) return FALSE;
	
	if(cmd == CMD_KILL)
	{
		one_argument(arg, buf);
		if(!buf) return FALSE;
		if(!isname(buf,GET_SHORT(lestodon))) return FALSE;
		send_to_char("A playful feminine voice echoes in your head: \"are you sure you want to do that? it's so adorable!\"",ch);
		return TRUE;
	}
	return FALSE;
}

void assign_subterra()
{
	assign_room(SCORPION_ROOM , subterra_scorpion_room);

	assign_mob(PILLBUG        , subterra_pillbug);
	assign_mob(SCORPION_ONE   , subterra_yellow_scorpion);
	assign_mob(TIGER          , subterra_tiger);
	assign_mob(CONSTRICTOR    , subterra_constrictor);
	assign_mob(BUMBLEBEE      , subterra_bumblebee);
	assign_mob(LESTODON       , subterra_lestodon);
}
