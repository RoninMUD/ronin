 /* ************************************************************************
  *  file: spec_procs.c , Special module.                   Part of DIKUMUD *
  *  Usage: Procedures handling special procedures for object/room/mobile   *
  *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
  ************************************************************************* */

/*

	Revamp by Solmyr 2007-2009
	
$Author: ronin $
$Date: 2005/01/21 14:55:28 $
$Header: /home/ronin/cvs/ronin/spec.abyss.c,v 2.3 2005/01/21 14:55:28 ronin Exp $
$Id: spec.abyss.c,v 2.3 2005/01/21 14:55:28 ronin Exp $
$Name:  $
$Log: spec.abyss.c,v $

Revision 2.3  2005/01/21 14:55:28  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.2  2004/11/16 04:59:07  ronin
Typo fix.

Revision 2.1  2004/04/29 11:49:25  ronin
Fix to AFF_HOLD spec to make it removable by remove_paralysis.

Revision 2.0.0.1  2004/02/05 16:10:11  ronin
Reinitialization of cvs archives


Revision 6-Nov-03 Ranger
Added room number to disarm log.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
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
 #include "fight.h"
 #include "act.h"
 #include "enchant.h"
 #include "reception.h"
 #include "spec_assign.h"
 /*   external vars  */

 extern CHAR *character_list;
 extern struct descriptor_data *descriptor_list;
 extern struct time_info_data time_info;
 extern struct spell_info_type spell_info[MAX_SPL_LIST];

#define ABYSS_ZONE      25000
#define ABYSS_LICH      25000
#define ABYSS_KEFTAB    25001
#define ABYSS_CRIMSON   25002
#define ABYSS_GIANT     25006
#define ABYSS_HYDRA     25009
#define ABYSS_KRAKEN    25010
#define ABYSS_TREANT    25011
#define ABYSS_GITH      25012
#define ABYSS_KALAS     25013
#define ABYSS_KNIGHT    25014
#define ABYSS_KEEPER    25017
#define ABYSS_WATER     25018
#define ABYSS_EARTH     25019
#define ABYSS_AIR       25020
#define ABYSS_FIRE      25021
#define ABYSS_LORD      25040

#define FIXED_PORTAL    25039
#define BROKEN_PORTAL   25041
#define ICOSAHEDRON     25042
#define CUBE            25043
#define OCTAHEDRON      25044
#define TETRAHEDRON     25045
#define DODECAHEDRON    25046
#define BONE_KEY        25048
#define ABYSS_STAFF     25071
#define ABYSS_WINDOW    25040
#define ABYSS_APPARATUS 25070

#define LICH_ROOM       25050
#define PORTAL_ROOM     25051
#define TWISTED1_ROOM   25066
#define TWISTED2_ROOM   25067
/* #define PORTAL_TARGET 25700 */
#define WINDOW_TARGET   25013

 /*********************************************************************
 *  Special procedures for abyss                                      *
 **********************************************************************/

void abyss_punch(CHAR *mob,  CHAR *vict)
{
  act("$n punches $N's head with a blow!", 1, mob, 0, vict, TO_NOTVICT);
  act("$n punches your head with a blow!", 1, mob, 0, vict, TO_VICT);
  if(V_MOB(mob) == ABYSS_LORD)
  	damage(mob, vict, number(120,200), TYPE_UNDEFINED, DAM_SKILL);
  else
		damage(mob, vict, number(60,100), TYPE_UNDEFINED, DAM_SKILL);
  WAIT_STATE(vict,PULSE_VIOLENCE*3);
}

void abyss_bash(CHAR *mob, CHAR *vict)
{
  act("$n runs quickly towards $N and knocks $M down!", 1, mob, 0, vict, TO_NOTVICT);
  act("$n runs quickly towards YOU! BANG!!!!!!!!!", 1, mob, 0, vict, TO_VICT);
  if(V_MOB(mob) == ABYSS_LORD)
  	damage(mob, vict, number(160,240), TYPE_UNDEFINED, DAM_SKILL);
  else
	  damage(mob, vict, number(80,120), TYPE_UNDEFINED,DAM_SKILL);
  GET_POS(vict) = POSITION_RESTING;
  WAIT_STATE(vict,PULSE_VIOLENCE*3);
}

void abyss_disarm(CHAR *mob, CHAR *vict)
{
  char buf[MAX_INPUT_LENGTH];
  OBJ *wield=NULL;
  act("$n makes a full-circle-kick and kicks $N's hand", 1, mob, 0, vict, TO_NOTVICT);
  act("$n makes a full-circle-kick and kicks your hand!", 1, mob, 0, vict, TO_VICT);
  if(V_MOB(mob) == ABYSS_LORD)
  	damage(mob, vict, number(60,100), TYPE_UNDEFINED, DAM_SKILL);
  else
  	damage(mob, vict, number(30,50), TYPE_UNDEFINED,DAM_SKILL);
  if (vict->equipment[WIELD])
    wield = vict->equipment[WIELD];
  else
    return;
  if(V_OBJ(wield)==11523) return;
  act("$N drop $S weapon.", 1,mob , 0, vict, TO_NOTVICT);
  act("You drop your weapon.", 1,mob , 0, vict, TO_VICT);
  sprintf(buf,"WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(mob),GET_NAME(vict),OBJ_SHORT(wield),world[CHAR_REAL_ROOM(vict)].number);
  log_s(buf);
  unequip_char(vict, WIELD);
  obj_to_room(wield, CHAR_REAL_ROOM(vict));
  wield->log=1;
  save_char(vict,NOWHERE);
}


int gatekeeper(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	
   if(cmd!=CMD_ENTER) return FALSE;

   one_argument (arg, buf);
   if(!*buf) return FALSE;
   if(strncmp(buf, "window", 6)) return FALSE;
   
   do_say(mob,"Go Away!!!!",CMD_SAY);
   return TRUE;
}

int treant(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	
	if(cmd!=CMD_NORTH) return FALSE;

	send_to_char("A twisted tree blocks your way.\n\r", ch);

	return TRUE;
}

int twisted_forest(int room, CHAR *ch, int cmd, char *arg)
{
	if(!ch) return FALSE;
	if(!MSG_LEAVE) return FALSE;
	if(!IS_MORTAL(ch)) return FALSE;
	if(chance(85)) return FALSE;
	send_to_char("A large branch takes a swing at you!\n\r", ch);
	
	if(CHAR_REAL_ROOM(ch) != NOWHERE)
		damage(ch, ch, number(50,200), TYPE_UNDEFINED, DAM_NO_BLOCK);
	
	return TRUE;
}

int githyanki(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict;
	struct affected_type_5 af;

	if(cmd != MSG_MOBACT) return FALSE;
	if(GET_POS(mob) != POSITION_FIGHTING) return FALSE;
	if(!mob->specials.fighting) return FALSE;

	vict = mob->specials.fighting;
	if (!vict) return FALSE;

	if (number(0,1)==0)
	{
		act("$n grins evilly towards $N.",1,mob,0,vict,TO_NOTVICT);
		act("$n grins evilly towards YOU!",1,mob,0,vict,TO_VICT);
		cast_fireball(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
	}
	else
	{
		act("$n throws some bandages towards $N!",1,mob,0,vict,TO_NOTVICT);
		act("$n throws some bandages towards YOU!",1,mob,0,vict,TO_VICT);
		send_to_char("Your body is wraped in bandages and you can't move!\n\r",vict);
		damage(mob,vict,number(30,40),TYPE_UNDEFINED,DAM_NO_BLOCK);

		if (!IS_AFFECTED(vict, AFF_HOLD))
		{
			af.type      = SPELL_HOLD;
			af.duration  = GET_LEVEL(mob);
			af.modifier  = 0;
			af.location  = APPLY_NONE;
			af.bitvector = AFF_HOLD;
			af.bitvector2 = 0;
			affect_to_char(vict, &af);
		}
		WAIT_STATE(vict,PULSE_VIOLENCE*3);
	}
	return FALSE;
}

int hydra(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict, *temp;

	if(cmd != MSG_MOBACT) return FALSE;
	if(!AWAKE(mob)) return FALSE;

	if (GET_POS(mob) != POSITION_FIGHTING)	return FALSE;

	if(chance(15))
	{
		act("$n charges and bites everyone with $s eight heads.",1,mob,0,0,TO_ROOM);
		for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = temp)
		{
			temp = vict->next_in_room;
			if(IS_NPC(vict) || !IS_MORTAL(vict))
				continue;
			
			act("$n bites you!\n\r",0,mob,0,vict, TO_VICT);
			damage(mob,vict,number(60,100),TYPE_UNDEFINED,DAM_NO_BLOCK);
   	 }
   	 return FALSE;
	}

	if (number(0,1) == 1) return FALSE;
	act("$n breathes a dangerous stream of fire.", 1, mob, 0, 0, TO_ROOM);

	for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = temp)
	{
		temp = vict->next_in_room;
		if (!IS_NPC(vict) && IS_MORTAL(vict))
    {
			send_to_char("You are burnt by the fire!\n\r", vict);
			damage(mob, vict, number(90,120), TYPE_UNDEFINED, DAM_FIRE);
		}
	}
	return FALSE;
}

int storm_giant(CHAR *mob,CHAR *ch,int cmd, char *arg)
{
	CHAR *vict;

	if (cmd != MSG_MOBACT) return FALSE;
	if (!mob->specials.fighting) return FALSE;

	vict = get_random_victim_fighting(mob);
	if(!vict) return FALSE;
	act("$n throws a lighting towards $N!",1,mob,0,vict, TO_NOTVICT);
	act("$n throws a lighting towards YOU!",1,mob,0,vict, TO_VICT);
	damage(mob, vict, number(50,80), TYPE_UNDEFINED, DAM_ELECTRIC);

	return FALSE;
}

int water_prince(CHAR *mob,CHAR *ch,int cmd,char *arg)
{
	CHAR *vict,*temp;
	
	if(cmd != MSG_MOBACT) return FALSE;
	if(!mob->specials.fighting) return FALSE;

	if (chance(50))
	{
		act("$n summons a large wave that almost drowns you.", 1, mob, 0, 0, TO_ROOM);
		for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp)
		{
			temp = vict->next_in_room;
			if(IS_NPC(vict) || !IS_MORTAL(vict))
				continue;
			damage(mob, vict, number(150,400), TYPE_UNDEFINED,DAM_MAGICAL);
		}
	}
	return FALSE;
}

int earth_king(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;

	if(cmd != MSG_MOBACT) return FALSE;
	if(!mob->specials.fighting) return FALSE;
	if(chance(50)) return FALSE;
	switch(number(0,1))
	{
		case 0:
		{
 			vict=get_random_victim_fighting(mob);
 			if(!vict) return FALSE;
 			act("$n throws a large boulder at $N.",1,mob,0,vict,TO_NOTVICT);
 			act("$n throws a large boulder at you!",1,mob,0,vict,TO_VICT);
 			act("You throw a large boulder at $N.",1,mob,0,vict,TO_CHAR);
 			damage(mob,vict,number(200,250),TYPE_UNDEFINED,DAM_NO_BLOCK);
 			break;
 		}
		case 1:
		{
			act("$n starts running around the room!",1,mob,0,vict,TO_ROOM);
			for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(IS_NPC(vict) || !IS_MORTAL(vict))
				continue;				
				act("$n runs at you and knocks you down!",1,mob,0,vict,TO_VICT);
				damage(mob,vict,number(110,150),TYPE_UNDEFINED,DAM_NO_BLOCK);
				GET_POS(vict) = POSITION_SITTING;
				WAIT_STATE(vict,PULSE_VIOLENCE*3);
			}
			break;
		}
	}
	return FALSE;
}

int air_lord(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
	CHAR *vict;

	if (cmd!=MSG_MOBACT) return FALSE;
	if (!mob->specials.fighting) return FALSE;

	switch(number(1,10))
	{
		case 1:
		case 2:
		case 3:
		case 4:
		{
			vict = get_random_victim_fighting(mob);
			if(!vict) return FALSE;
			act("A large branch flies through the air and hits $N on the head!",1,mob,0,vict, TO_NOTVICT);
			act("A large branch flies through the air and hits you on the head!",1,mob,0,vict, TO_VICT);
			act("A branch is thrown by your powerful winds right onto $N's head.",1,mob,0,vict, TO_CHAR);
			damage(mob, vict, 200, TYPE_UNDEFINED, DAM_NO_BLOCK);
			break;
		}
		
		case 5:
		case 6:
		case 7:
		case 8:
		{
			vict = get_random_victim_fighting(mob);
			if(!vict) return FALSE;
			act("$n pulls $N up, and throws $M back on the ground!",1,mob,0,vict, TO_NOTVICT);
			act("$n lifts YOU up! BANG!!!!!!!!!!!!!!",1,mob,0,vict, TO_VICT);
			damage(mob, vict, number(100,150), TYPE_UNDEFINED, DAM_NO_BLOCK);
			GET_POS(vict) = POSITION_STUNNED;
			WAIT_STATE(vict, PULSE_VIOLENCE*3);
			break;
		}
  
		case 9:
		case 10:
		{
			vict = get_random_victim_fighting(mob);
			if(!vict) return FALSE;
			act("You are blinded by pieces of moss and bark that pierce your eyes.",1,mob,0,vict, TO_VICT);
			act("$N's eyes are pierced by the debree from the tornado.",1,mob,0,vict, TO_NOTVICT);
			act("You blind $N.",1,mob,0,vict, TO_CHAR);
			spell_blindness(GET_LEVEL(mob), mob, vict, 0);
			break;
		}
	}
  return FALSE;
}

int fire_sultan(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
	CHAR *vict,*temp;

	if(cmd != MSG_MOBACT) return FALSE;
	if(!mob->specials.fighting) return FALSE;

	if (number(0,1) == 1) return FALSE;

	act("$n conjures a huge fireball and drops it on the ground!", 1, mob, 0, 0, TO_ROOM);
	act("The huge fireball explodes!!!!!!!!!!!!",1,mob,0,0,TO_ROOM);

	for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp)
	{
		temp=vict->next_in_room;
		if(IS_NPC(vict) || !IS_MORTAL(vict))
			continue;
		send_to_char("You are burnt by some pieces of the fireball!\n\r",vict);
		damage(mob, vict, number(80,100), TYPE_UNDEFINED,DAM_FIRE);
	}
	return FALSE;
}

int death_knight(CHAR *mob,CHAR *ch,int cmd,char *arg)
{
	CHAR *vict;

	if(cmd != MSG_MOBACT) return FALSE;
	if(!mob->specials.fighting) return FALSE;

	vict = get_random_victim_fighting(mob);
	if(!vict) return FALSE;
	switch(number(1,3))
	{
		case 1:
		{
			abyss_punch(mob,vict);
			break;
		}
		case 2:
		{
			abyss_bash(mob,vict);
			break;
		}
		case 3:
		{
			abyss_disarm(mob,vict);
			break;
		}
	}
	return FALSE;
}

int keftab(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict,*next_vict;
	
	if(cmd!=MSG_MOBACT) return FALSE;
	if(!mob->specials.fighting) return FALSE;
	
	if(chance(25)) {
		act("Large quantities of disgusting green slime are splattered everywhere!",0,mob,0,0,TO_ROOM);
	  act("You feel your flesh burn from the contact with the slime.",0,mob,0,0,TO_ROOM);
	  for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
	  {
	  	next_vict = vict->next_in_room;
	  	if(!IS_MORTAL(vict))
	  		continue;
	  	damage(mob,vict,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
	  }
	}
	return FALSE;
}

int kalas(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
	CHAR *vict,*temp;

	if(cmd != MSG_MOBACT) return FALSE;
	if(!mob->specials.fighting) return FALSE;

	if (number(0,1)==0)
	{
		if (number(0,2)==0)
		{
			act("$n's spin-kick has generated a big whirl.", FALSE, mob, 0, 0, TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = temp)
			{
				temp = vict->next_in_room;
				if(IS_NPC(vict) || !IS_MORTAL(vict))
					continue;
				act("You have been kicked by $n!",0,mob,0,vict,TO_VICT);
				damage(mob, vict, number(80,120), TYPE_UNDEFINED,DAM_SKILL);
			}
		}
		else
		return FALSE;
	}
	else
	{
		vict = get_random_victim_fighting(mob);
		if(!vict) return FALSE;
		switch(number(1,3))
		{
			case 1:
				abyss_punch(mob,vict);
				break;
			case 2:
				abyss_bash(mob,vict);
				break;
			case 3:
				abyss_disarm(mob,vict);
				break;
		}
	}
return FALSE;
}

int crimson_death(CHAR *mob,CHAR *ch,int cmd,char *arg)
{
	CHAR *vict,*temp;

	if(cmd != MSG_MOBACT) return FALSE;
	if(!mob->specials.fighting) return FALSE;

	if(number(0,1)==0)
	{
		if(number(0,1)==0)
		{
			act("$n is so angry and he hits on the ground very hard!", FALSE, mob, 0, 0, TO_ROOM);
			act("The earth trembles and shivers!", FALSE, mob, 0, 0, TO_ROOM);
			for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp)
			{
				temp = vict->next_in_room;
				if(!IS_MORTAL(vict))
					continue;
				act("There is a pain coming from your feet and you fall down!",0,mob,0,vict,TO_VICT);
				damage(mob, vict, number(240,300), TYPE_UNDEFINED, DAM_NO_BLOCK);
				GET_POS(vict) = POSITION_SITTING;
				WAIT_STATE(vict,PULSE_VIOLENCE*3);
			}
		}
		else
			return FALSE;
	}
	else
	{
		vict = get_random_victim_fighting(mob);
		if(!vict) return FALSE;
		switch(number(1,2))
		{
			case 1:
			{
				act("A stream of fire shoots from $n's eye towards $N!", 1, mob, 0, vict, TO_NOTVICT);
				act("A stream of fire shoots from $n's eye towards YOU! Ouch!!!", 1, mob, 0, vict, TO_VICT);
				damage(mob, vict, number(260,300), TYPE_UNDEFINED,DAM_NO_BLOCK);
				WAIT_STATE(vict,PULSE_VIOLENCE*3);
				break;
			}
			case 2:
			{
				abyss_disarm(mob,vict);
				break;
			}
		}
	}
	return FALSE;
}

int kraken(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
	OBJ *wield=0;
	CHAR *vict,*temp;
	char buf[MAX_INPUT_LENGTH];

	if(cmd != MSG_MOBACT) return FALSE;
	if(!mob->specials.fighting) return FALSE;

	if (number(0,1)==0)
	{
		if (number(0,1)==0)
		{
			act("$n shouts loudly and $s voice hits right on your heart!\n\r", FALSE, mob, 0, 0, TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = temp)
			{
				temp = vict->next_in_room;
				if(IS_NPC(vict) || !IS_MORTAL(vict))
					continue;
				act("There is a pain coming from your heart!",0,mob,0,vict,TO_VICT);
				damage(mob, vict, number(150,200), TYPE_UNDEFINED, DAM_NO_BLOCK);
				WAIT_STATE(vict,PULSE_VIOLENCE*3);
			}
		}
		else
			return FALSE;
	}
	else
	{
		switch(number(1,3))
		{
			case 1:
			{
				vict = get_random_victim_fighting(mob);
				if(!vict) return FALSE;
				act("$n grabs $N with one tentacle and hits $M with another!", 1, mob, 0, vict, TO_NOTVICT);
				act("$n grabs you with one tentacle and hits YOU with another!", 1, mob, 0, vict, TO_VICT);
				damage(mob, vict, number(250,300), TYPE_UNDEFINED,DAM_NO_BLOCK);
				WAIT_STATE(vict,PULSE_VIOLENCE*3);
				break;
			}
			case 2:
			{
				vict = get_random_victim_fighting(mob);
				if(!vict) return FALSE;
				act("$n jumps high into the sky and falls on $N", 1, mob, 0, vict, TO_NOTVICT);
				act("$n jumps high into the sky and falls on YOU!", 1, mob, 0, vict, TO_VICT);
				damage(mob, vict, number(120,150), TYPE_UNDEFINED,DAM_NO_BLOCK);
				if(vict->equipment[WIELD])
					wield = vict->equipment[WIELD];
				else
					return FALSE;
					
				act("$N drops $S weapon.", 1, mob, 0, vict, TO_NOTVICT);
				act("You drop your weapon.", 1, mob, 0, vict, TO_VICT);
				sprintf(buf,"WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(mob),GET_NAME(vict),OBJ_SHORT(wield),world[CHAR_REAL_ROOM(vict)].number);
            log_s(buf);
				unequip_char(vict, WIELD);
				obj_to_room(wield, CHAR_REAL_ROOM(vict));
				GET_POS(vict)=POSITION_SITTING;
				WAIT_STATE(vict,PULSE_VIOLENCE*3);
				break;
			}
			case 3:
			{
				vict=get_random_victim_fighting(mob);
				if(!vict) return FALSE;
				act("$n reaches out and catches $N with $s giant tentacles.",1,mob,0,vict,TO_NOTVICT);
				act("$n wraps $s tentacles around you.",1,mob,0,vict,TO_VICT);
				act("You gently squeeze the crap out of $N.",1,mob,0,vict,TO_CHAR);
				damage(mob,vict,number(300,400),TYPE_UNDEFINED,DAM_NO_BLOCK);
				break;
			}
		}
	}
	return FALSE;
}

int lich_curse(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);

int demi_lich(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL, *next_vict = NULL;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

	switch(number(1,4))
	{
		case 1:
		{
   		act("Green rays shoot through the room as $n raises $s staff.\n\r",0,mob,0,0,TO_ROOM);
    	for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict = next_vict)
    	{
      	next_vict=vict->next_in_room;
      	if(!IS_MORTAL(vict)) continue;
      	spell_poison(GET_LEVEL(mob),mob,vict,0);
    	}
    	break;
  	}
  
  	case 2:
		{
    	vict=get_random_victim_fighting(mob);
    	if(!vict) return FALSE;
    	act("$n thrusts $s skeletal hand right through $N's ribcage!",1,mob,0,vict,TO_NOTVICT);
    	act("$n thrusts $s skeletal hand right through your chest!",1,mob,0,vict,TO_VICT);
    	act("Haha, That'll show $M.",1,mob,0,vict,TO_CHAR);
    	damage(mob,vict,number(350,500),TYPE_UNDEFINED,DAM_NO_BLOCK);
    	break;
  	}
  
    case 3:
        {
 /*       act("Streams of fire shoot from $n's eye sockets.\n\rThe walls start to glow.\n\r",1,mob,0,vict,TO_ROOM);
        for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict = next_vict)
        {
            next_vict=vict->next_in_room;
                    if(!IS_MORTAL(vict))
                        continue;
            if(!IS_AFFECTED(vict, AFF_SPHERE) || breakthrough(ch,vict,BT_SPHERE))
            {
                if(!enchanted_by(vict, "The Curse of the Lich"))
                {
                    CREATE(tmp_enchantment, ENCH, 1);
                    tmp_enchantment->name     = str_dup("The Curse of the Lich");
                    tmp_enchantment->duration = 5;
                    tmp_enchantment->func     = lich_curse;
                    enchantment_to_char(vict, tmp_enchantment, FALSE);
                    send_to_char("The tension of the room puts a strain on you.\n\r",vict);
                }
            }
        }*/
        break;
    }
  
 		case 4:
  	{
   		act("$n's staff glows as $e flails it around the room.\n\r",1,mob,0,vict,TO_ROOM);
    	for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict = next_vict)
    	{
      	next_vict = vict->next_in_room;
      	if(!IS_MORTAL(vict))
      		continue;
      	switch(number(1,4))
      	{
        	case 1:
        	{
        		act("$n's staff hits you on the shoulder.",1,mob,0,vict,TO_VICT);
        		damage(mob,vict,80,TYPE_UNDEFINED,DAM_NO_BLOCK);
						break;
					}
 	      	case 2:
 	      	{
 	      		act("You stagger as $n's staff hits you on the head.",1,mob,0,vict,TO_VICT);
 	      		damage(mob,vict,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
 	      		break;
 	      	}
 	      	case 3:
 	      	{
 	      		act("You feel your knee shatter as $n's staff hits you.",1,mob,0,vict,TO_VICT);
 	      		damage(mob,vict,160,TYPE_UNDEFINED,DAM_NO_BLOCK);
 	      		break;
 	      	}
 	      	case 4:
 	      	{
 	      		act("$n's staff hits you right on the hip.",1,mob,0,vict,TO_VICT);
 	      		damage(mob,vict,120,TYPE_UNDEFINED,DAM_NO_BLOCK);
 	      		break;
 	      	}
	      }
			}
		}
		break;
	}
	return FALSE;
}

int lich_curse(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {

  if(cmd==MSG_REMOVE_ENCH)
  {
    send_to_char("The Demi-Lich releases his hold.\n\r",ench_ch);
    return FALSE;
  }
  return FALSE;
}

int bone_staff(OBJ *obj, CHAR *ch, int cmd, char *argument)
{
  CHAR *vict, *next_vict, *carrier;
	
	carrier = obj->equipped_by;
	if(!carrier) return FALSE;
  if(!obj->equipped_by) return FALSE;
  if(carrier != obj->equipped_by) return FALSE;
  if(chance(2) && cmd == MSG_MOBACT && carrier->specials.fighting)
  {
    act("$n's staff shoots green rays all over the room.\n\r",0,carrier,0,0,TO_ROOM);
    act("Your staff shivers in your hand as it shoots poisonous rays all over the room.\n\r",0,carrier,0,0,TO_CHAR);
    for(vict = world[CHAR_REAL_ROOM(carrier)].people; vict; vict = next_vict)
    {
      next_vict=vict->next_in_room;
      if(!IS_MORTAL(vict))
      	continue;
      spell_poison(GET_LEVEL(carrier),carrier,vict,0);
    }
  }
  return FALSE;
}

/* Spec to snap bone key when used */

int bone_key(int room, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	int bone_key;
	OBJ *key;
	
	bone_key = real_object(BONE_KEY);		
	
	if(cmd == CMD_UNLOCK)
	{
		if(!ch) return FALSE;
		one_argument(arg, buf);
		if(!*buf) return FALSE;
		if(strncmp(buf, "door", 4)) return FALSE;
		if(!(key = get_obj_in_list_num(bone_key, ch->carrying))) return FALSE;
		
		send_to_char("The key of bone snaps off as you unlock the door.\n\r", ch);
		REMOVE_BIT(world[room].dir_option[SOUTH]->exit_info, EX_LOCKED);
		REMOVE_BIT(world[real_room(PORTAL_ROOM)].dir_option[NORTH]->exit_info, EX_LOCKED);
		act("The key of bone snaps off as $n unlocks the door.",0,ch,0,0, TO_ROOM);
		obj_from_char(key);
		extract_obj(key);
		return TRUE;
	}
	return FALSE;
}

/* spec to replace object 25041 with 25039 */
/*
int vault_portal_swap(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  int element = FALSE;
  OBJ *obj2;
  char arg[MSL];

  if (cmd!=CMD_UNKNOWN) return FALSE;
  if (!ch) return FALSE;
  if (V_ROOM(ch)!=PORTAL_ROOM) return FALSE;

  argument=one_argument(argument, arg);
  if(!*arg) return FALSE;
  if(!is_abbrev(argument,"insert")) return FALSE;
  one_argument(argument, arg);
  if(!*arg) {
    send_to_char("insert what?\n\r",ch);
    return TRUE;
  }
  string_to_lower(arg);
  if(strcmp(arg,"element aperture")) return FALSE;


  for(obj2 = ch->carrying;obj2;obj2=obj2->next_content) {
    if (V_OBJ(obj2) == DODECAHEDRON) element = TRUE;
  }

  if(!element) return FALSE;

  act("You insert the element into the aperture.\n\r",0,ch,0,0,TO_CHAR);
  act("$n inserts the element into the aperture.\n\r",0,ch,0,0,TO_ROOM);
  act("The $p's image turns.\n\r",0,ch,obj,0,TO_CHAR);
  act("The $p's image turns.\n\r",0,ch,obj,0,TO_ROOM);
  extract_obj(obj);
  extract_obj(obj_from_char(obj2));
  obj2=read_object(FIXED_PORTAL,VIRTUAL);
  obj_to_room(obj2,PORTAL_ROOM);
  return FALSE;
}
*/
/* Spec to transfer people from 25051 to 25700 */
/*
int vault_portal(OBJ *portal, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(!ch) return(FALSE);

  if (cmd != CMD_ENTER || !AWAKE (ch)) return FALSE;

  one_argument(arg,buf);
  if (!isname (buf, OBJ_NAME(portal))) return FALSE;

  if(GET_POS(ch)==POSITION_RIDING) {
     send_to_char("Dismount first.\n\r",ch);
     return TRUE;

  }

  act ("$n enters the $p.\n\r", TRUE, ch, portal, 0, TO_ROOM);
  send_to_char ("You enter the portal.\n\r",ch);
  char_from_room (ch);
  char_to_room (ch, real_room(PORTAL_TARGET));
  act ("$n arrives through the portal.\n\r",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  return TRUE;
}
*/

/* Spec to transfer people from 25011 to 25013 */

int gate_window(OBJ *window, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(!ch) return(FALSE);
  if (cmd != CMD_ENTER || !AWAKE (ch)) return FALSE;

  one_argument(arg,buf);
  if (!isname(buf, OBJ_NAME(window))) return FALSE;

  if(GET_POS(ch)==POSITION_RIDING)
  {
     send_to_char("Dismount first.\n\r",ch);
     return TRUE;
  }

  act("$n enters the $p.", TRUE, ch, window, 0, TO_ROOM);
  send_to_char ("You enter the window.\n\r",ch);
  char_from_room (ch);
  char_to_room (ch, real_room(WINDOW_TARGET));
  act ("$n arrives through the window.\n\r",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  return TRUE;
}

/* Spec to transmute 4 items into one, inside object 25070 */

int abyss_apparatus(OBJ *apparatus, CHAR *ch, int cmd, char *argument) {
  OBJ *obj2, *obj2_next;
  int elmwater=0,elmearth=0,elmwind=0,elmfire=0;
  char arg[MSL];

  if(cmd!=CMD_UNKNOWN) return FALSE;
  if(!ch) return FALSE;
  if(!AWAKE(ch)) return FALSE;
  if(ch!=apparatus->carried_by) return FALSE;

  argument=one_argument(argument, arg);
  if(!*arg) return FALSE;
  if(!is_abbrev(arg,"transmute")) return FALSE;

  for(obj2 = apparatus->contains;obj2;obj2=obj2->next_content)
  {
    if(obj2 && V_OBJ(obj2)==ICOSAHEDRON) elmwater=1;
    if(obj2 && V_OBJ(obj2)==CUBE) elmearth=1;
    if(obj2 && V_OBJ(obj2)==OCTAHEDRON) elmwind=1;
    if(obj2 && V_OBJ(obj2)==TETRAHEDRON) elmfire=1;

  }

  if(!elmwater || !elmearth || !elmwind || !elmfire)
  {
    send_to_char("You attempt to activate the apparatus, but nothing happens.\n\r",ch);
    return TRUE;
  }

  if(!IS_SET(apparatus->obj_flags.value[1], CONT_CLOSED)) return FALSE;

  for(obj2 = apparatus->contains;obj2;obj2=obj2_next)
  {
    obj2_next=obj2->next_content;
    if(V_OBJ(obj2) == ICOSAHEDRON || V_OBJ(obj2) == CUBE || V_OBJ(obj2) == OCTAHEDRON || V_OBJ(obj2) == TETRAHEDRON)
    {
      obj_from_obj(obj2);
      extract_obj(obj2);
		}
  }

  act("When you activate the Apparatus, it begins to shake, and strange sounds can be heard from within.\n\r",0,ch,0,0,TO_CHAR);
  act("As $n activates $s Alchemist's Apparatus, it begins to percolate energetically!\n\r",0,ch,0,0,TO_ROOM);
  obj2=read_object(DODECAHEDRON,VIRTUAL);
  obj_to_obj(obj2,apparatus);

  return FALSE;

}
 /**********************************************************************\
 |* End Of the Special procedures for Abyss                            *|
 \**********************************************************************/

void assign_abyss(void)
{
  assign_room(TWISTED1_ROOM  , twisted_forest);
  assign_room(TWISTED2_ROOM  , twisted_forest);
  assign_room(LICH_ROOM      , bone_key);
  assign_mob(ABYSS_KEFTAB    , keftab);
  assign_mob(ABYSS_CRIMSON   , crimson_death);
  assign_mob(ABYSS_GIANT     , storm_giant);
  assign_mob(ABYSS_GITH      , githyanki);
  assign_mob(ABYSS_HYDRA     , hydra);
  assign_mob(ABYSS_KRAKEN    , kraken);
  assign_mob(ABYSS_KALAS     , kalas);
  assign_mob(ABYSS_KNIGHT    , death_knight);
	assign_mob(ABYSS_LORD      , death_knight);
  assign_mob(ABYSS_KEEPER    , gatekeeper);
  assign_mob(ABYSS_TREANT    , treant);
  assign_mob(ABYSS_WATER     , water_prince);
  assign_mob(ABYSS_EARTH     , earth_king);
  assign_mob(ABYSS_AIR       , air_lord);
  assign_mob(ABYSS_FIRE      , fire_sultan);
	assign_mob(ABYSS_LICH      , demi_lich);
	assign_obj(ABYSS_STAFF     , bone_staff);
/*assign_obj(BROKEN_PORTAL   , vault_portal_swap);*/
/*assign_obj(FIXED_PORTAL    , vault_portal); */
  assign_obj(ABYSS_WINDOW    , gate_window);
  assign_obj(ABYSS_APPARATUS , abyss_apparatus);
}
