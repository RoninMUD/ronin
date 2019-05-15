/*
	Specs for Dionysus, by Dionysus
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
#define KEY_ROOM     21342
#define KEY_ROOM2    21341
#define OTYUGH_ROOM  21327

/* Object defines */
#define WOODEN_KEY   21304
#define BROKEN_SWORD 21305
#define ELVEN_SKULL  21307
#define ELVEN_KEY    21308
#define SWARM        21329

/* Mobile defines */
#define ELVEN_LASS   21327
#define WARRIOR      21328
#define OTYUGH       21332
#define BASILISK     21333

int dio_swarm(OBJ *swarm, CHAR *ch, int cmd, char *arg)
{
       ch = swarm->equipped_by;
       if(ch && cmd==MSG_TICK && chance(GET_AGE(ch)))
               send_to_char("The swarm of angry beetles buzzes angrily in your ear.\r\n", ch);
       return FALSE;
}

int key_crumble(int room, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	int wooden_key;
	OBJ *key;
	
	wooden_key = real_object(WOODEN_KEY);
	
	if(cmd == CMD_UNLOCK)
	{
		if(!ch) return FALSE;
		one_argument(arg,buf);
		if(!*buf) return FALSE;
		if(strncmp(buf,"door", 4)) return FALSE;
		if(!(key = get_obj_in_list_num(wooden_key, ch->carrying))) return FALSE;
		
		send_to_char("The wooden key manages to unlock the door, but snaps off in the lock.\n\r",ch);
		REMOVE_BIT(world[room].dir_option[WEST]->exit_info,EX_LOCKED);
	  REMOVE_BIT(world[real_room(KEY_ROOM2)].dir_option[EAST]->exit_info,EX_LOCKED);
	  act("As $n unlocks the door, the key snaps off in the lock.",FALSE,ch,0,0,TO_ROOM);
		obj_from_char(key);
	  extract_obj (key);
	  return TRUE;
	}
	return FALSE;
}

int elven_lass(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	OBJ *obj;
	OBJ *obj2;
	
	if(cmd==MSG_OBJ_GIVEN)
	{
		if(!ch) return FALSE;
		for(obj = mob->carrying;obj;obj = obj->next_content)
		{
			if(V_OBJ(obj) == ELVEN_SKULL)
			{
		
				act("\n\r$n lets out a loud wail, and tears begin running down her face.",0,mob,0,0,TO_ROOM);
				act("$n says, \"Oh, Isolde! My sweet baby, what have they done to you?\"",0,mob,0,0,TO_ROOM);
				act("\n\rCollecting herself, $n nods to you.\n\r",0,mob,0,0,TO_ROOM);
				act("$n says, \"Thank you, stranger, for bringing me this. My heart breaks,",0,mob,0,0,TO_ROOM);
				act("but at least I know what became of her. Please, take this, it may help you",0,mob,0,0,TO_ROOM);
				act("on your travels.\"\n\r",0,mob,0,0,TO_ROOM);
				
				extract_obj(obj);
				obj2 = read_object(ELVEN_KEY,VIRTUAL);
				obj_to_char(obj2,ch);				
			}
		}
	}
	return FALSE;
}

int warrior(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	OBJ *obj;
	
	if(cmd==MSG_OBJ_GIVEN)
	{
		if(!ch) return FALSE;
		for(obj = mob->carrying;obj;obj = obj->next_content)
		{
			if(V_OBJ(obj) == BROKEN_SWORD)
			{
				act("$n says, \"My father's sword! Thank you, stranger. I pray that someday I can repay your kindness.\"",0,mob,0,0,TO_ROOM);
				extract_obj(obj);
			}
		}
	}
	return FALSE;
}

int otyugh(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	
	if(V_ROOM(mob) != OTYUGH_ROOM) return FALSE;
	if(cmd != CMD_CRAWL) return FALSE;
	
	one_argument (arg, buf);
	if(!*buf) return FALSE;
	if(strncmp(buf,"rubbish", 7)) return FALSE;
	
	act("$n places itself in front of the passage, effectively blocking it.",0,mob,0,0,TO_ROOM);
	return TRUE;
}

int basilisk(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict;
		
	if(!mob->specials.fighting) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;
	
	if(chance(5))
	{
		vict = get_random_victim_fighting(mob);
		if(!vict) return FALSE;
		
		act("$n swings $s head around and makes eye contact with you, turning you to stone.",0,mob,0,vict,TO_VICT);
		act("$n swings $s head around and makes eye contact with $N, turning $M to stone.",0,mob,0,vict,TO_NOTVICT);
		act("You swing your head around and make eye contact with $N, turning $M to stone.",0,mob,0,vict,TO_CHAR);
		spell_petrify(GET_LEVEL(mob), mob, vict, 0);		
	}
	return FALSE;
}

void assign_dionysus(void)
{
	assign_room(KEY_ROOM , key_crumble);
	assign_mob(ELVEN_LASS, elven_lass);
	assign_mob(WARRIOR   , warrior);
	assign_mob(BASILISK  , basilisk);
	assign_mob(OTYUGH    , otyugh);
        assign_obj(SWARM, dio_swarm);	
}
