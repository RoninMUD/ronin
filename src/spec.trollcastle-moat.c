/**********************************************************************/
/****                                                              ****/
/****          Specs for Troll Castle - Moat                       ****/
/****                                                              ****/
/**********************************************************************/

/****************************************************************************
*               Special procedure module for  Troll_Castle-Moat
*
*
*
*                       by: Robert Upshall (Ankh)
*
*           special thanks: Rick(Ranger) for syntax checking
*
 ****************************************************************************/

/*
$Author: ronin $
$Date: 2004/02/05 16:11:24 $
$Header: /home/ronin/cvs/ronin/spec.trollcastle-moat.c,v 2.0.0.1 2004/02/05 16:11:24 ronin Exp $
$Id: spec.trollcastle-moat.c,v 2.0.0.1 2004/02/05 16:11:24 ronin Exp $
$Name:  $
$Log: spec.trollcastle-moat.c,v $
Revision 2.0.0.1  2004/02/05 16:11:24  ronin
Reinitialization of cvs archives

Revision 1.5  2002/05/29 04:57:45  ronin
Yet another fix for lizard assist spec.

Revision 1.4  2002/05/13 12:59:15  ronin
Fix for lizardman assist spec.

Revision 1.3  2002/05/09 20:16:07  ronin
Fix to randomize mobs targets better.

Revision 1.2  2002/04/15 03:26:36  ronin
Update of acts and small changes to spec percentages.

Revision 1.1  2002/04/13 14:41:05  ronin
spec.troll.c replaced with spec.trollvillage.c.  spec.trollcastle-moat.c
specs for a new area.

$State: Exp $
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

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
#include "spec_assign.h"
#include "act.h"
#include "enchant.h"

extern struct time_info_data time_info;

int troll_lizardapprentice(CHAR *mob, CHAR *ch, int cmd, char *arg) {

   CHAR *tch, *fighting, *vic, *buff, *murder, *thief;

   /* set fighting, murder and thief pointers */
   fighting = 0;
   vic = 0;
   buff = 0;
   murder = 0;
   thief = 0;

   if (cmd || !AWAKE(mob) || (GET_POS(mob) == POSITION_FIGHTING))
     return FALSE;

   for (tch=world[CHAR_REAL_ROOM(mob)].people; tch; tch = tch->next_in_room)
   {
      if(GET_LEVEL(tch) > 50)
         continue;

      if(IS_MOB(tch))
        continue;

      if (!fighting && tch->specials.fighting)
      {
         vic = tch->specials.fighting;
         buff = vic->specials.fighting;
         fighting = buff;
      }
      else if (!murder && IS_SET(tch->specials.pflag, PLR_KILL))
      {
         murder = tch;
      }
      else if (!thief && IS_SET(tch->specials.pflag, PLR_THIEF))
      {
         thief = tch;
      }
   }

   /* lizard apprentice assist mob */
   if (fighting)
   {
     act("$n hisses 'This is a peaceful moat!  Aggression must be punished!'", FALSE, mob, 0, fighting, TO_ROOM);
     hit(mob, fighting, TYPE_UNDEFINED);
     return(FALSE);
   }
   /* lizard apprentice attacks murderer */
   else if (murder)
   {
     act("$n hisses 'This is a lawful moat!  Attack!!!!!!'", FALSE, mob, 0, murder, TO_ROOM);
     hit(mob, murder, TYPE_UNDEFINED);
     return(FALSE);
   }
   /* lizard apprentice attacks thief */
   else if (thief)
   {
     act("$n hisses 'Stealing will not be tolerated in this moat!  Attack!!!!!!'", FALSE, mob, 0, thief, TO_ROOM);
     hit(mob, thief, TYPE_UNDEFINED);
     return(FALSE);
   }
   /* lizard apprentice hisses */
   else if (GET_POS(mob)!=POSITION_FIGHTING)
   {
     if(!number(0,10))
       act("$n hisses quietly.",FALSE,mob,0,0,TO_ROOM);
     return (FALSE);
   }

   return FALSE;
}

int troll_lizardshaman(CHAR *typik, CHAR *ch, int cmd, char *arg) {

CHAR *victim,*temp;

if (cmd)
  return FALSE;

/* Typik practices spells */
if (GET_POS(typik)!=POSITION_FIGHTING) {
      if(!number(0,2)) {
    act("$n waves his arms in a strange motion.",FALSE,typik,0,0,TO_ROOM);
    if (!number(0,1))
       act("A small impish creature dances out of the mud.",FALSE,typik,0,0,TO_ROOM);
    else
         act("A large mud arm grows from the wall and scratches $n's back.",FALSE,typik,0,0,TO_ROOM);
  }
}

/* Typik floods room with mud */
   if ((GET_POS(typik) == POSITION_FIGHTING) && (!number(0,15))){

   act("$n waves his arms in a strange motion.",FALSE,typik,0,0,TO_ROOM);
   act("The lair suddenly fills with a thick mud.",FALSE,typik,0,0,TO_ROOM);

      for(victim = world[CHAR_REAL_ROOM(typik)].people; victim; victim = temp)
      {
      temp = victim->next_in_room;

         if(GET_LEVEL(victim) > 50)
            continue;

	 if(IS_MOB(victim))
	    continue;

      if(victim != typik){ /* mud blinds all in room */
         act("The thick muds gets into your eyes.",1,typik,0,victim,TO_VICT);
         spell_blindness(GET_LEVEL(typik), typik, victim, 0);
      }

      if(!number(0,2)) { /* mud forces victim out of room */
        act("A massive wall of mud pushes $N out of the room.",1,typik,0,victim,TO_NOTVICT);
        act("A massive wall of mud pushes you out of the room.",1,typik,0,victim,TO_VICT);
        char_from_room(victim);
        char_to_room(victim,real_room(7798));
        act("$n is washed into the room on a giant wave of mud.",FALSE,victim,0,0,TO_NOTVICT);
        act("The wall of mud has displaced you.",1,victim,0,victim,TO_VICT);
      }
   }
   return FALSE;
}

/* Typik random fightings specs */
   for(victim = world[CHAR_REAL_ROOM(typik)].people; victim; victim = temp)
   {

  temp = victim->next_in_room;

      if(GET_LEVEL(victim) > 50)
         continue;

      if(IS_MOB(victim))
     continue;

   /* Typik randomly sticks peoples heads into the mud wall */
      if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(typik)))
      {
         if(!number(0,15))
	 {
            act("$n says '$N, your face offends me!'",FALSE,typik,0,victim,TO_ROOM);
       act("$n grabs you by the neck and places your face into the mud wall!",1,typik,0,victim,TO_VICT);
       act("$n grabs $N by the neck and sticks his head into the mud wall!",1,typik,0,victim,TO_NOTVICT);
       damage(typik,victim,(number(1,10)*27),TYPE_UNDEFINED,DAM_NO_BLOCK);
            act("The thick muds gets into your eyes.",1,typik,0,victim,TO_VICT);
       spell_blindness(GET_LEVEL(typik), typik, victim, 0);
       /* stun/pummel vitvim here */
       GET_POS(victim)=POSITION_STUNNED;
       WAIT_STATE(victim, 2*PULSE_VIOLENCE); /* 2 rounds */
       return FALSE;
     }
   }

   /* Typik summons a arm of mud to choke random people */
      if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(typik)))
      {
         if(!number(0,9))
	 {
       act("$n waves his arms in a strange motion.",FALSE,typik,0,victim,TO_ROOM);
       act("A large arm of mud grows out of the lair wall!'",FALSE,typik,0,victim,TO_ROOM);
       act("The large arm of mud graps you by the neck and starts to choke you!",1,typik,0,victim,TO_VICT);
       act("The large arm of mud starts to choke $N!",1,typik,0,victim,TO_NOTVICT);
       damage(typik,victim,(number(1,10)*51),TYPE_UNDEFINED,DAM_NO_BLOCK);
       return FALSE;
     }
   }

   /* Typik creates a glob of mud to fall on random people */
      if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(typik)))
      {
         if(!number(0,11))
	 {
       act("$n waves his arms in a strange motion.",FALSE,typik,0,victim,TO_ROOM);
       act("A heavy glob of mud falls from the ceiling onto your head!",1,typik,0,victim,TO_VICT);
       act("A large glob of mub falls from the ceiling onto $N's head!",1,typik,0,victim,TO_NOTVICT);
       damage(typik,victim,(number(7,15)*7),TYPE_UNDEFINED,DAM_NO_BLOCK);
       /* stun/pummel vitvim here */
       GET_POS(victim)=POSITION_STUNNED;
       WAIT_STATE(victim, 2*PULSE_VIOLENCE); /* 2 rounds */
       return FALSE;
     }
   }

   /* Typik creates a mudhole under random people */
      if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(typik)))
      {
         if(!number(0,5))
	 {
       act("$n waves his arms in a strange motion.",FALSE,typik,0,victim,TO_ROOM);
       act("The ground beneath your feet liquefies!",1,typik,0,victim,TO_VICT);
       act("The ground beneath $N's feet liquefies!",1,typik,0,victim,TO_NOTVICT);
       /* if not flying sink in hole */
       if (!IS_AFFECTED(victim,AFF_FLY))
       {
          act("You sink into the mudhole!",1,typik,0,victim,TO_VICT);
               act("$N sinks into the mudhole!",1,typik,0,victim,TO_NOTVICT);
          WAIT_STATE(victim, 3*PULSE_VIOLENCE); /* 3 rounds */
       }
       return FALSE;
     }
   }

}

return FALSE;
}

void reset_kudzu(OBJ *kudzu)
{
	kudzu->affected[0].modifier = 1; //Location 0 should be set as DAMROLL
	kudzu->affected[1].modifier = 5; //Location 1 should be set as HP_REGEN
	kudzu->affected[2].modifier = 0; //Location 2 should be set as MV
	kudzu->obj_flags.value[0] = 0;
	kudzu->obj_flags.value[1] = 0;
}

void update_kudzu(OBJ *kudzu, int bSlowDecay)
{
	int hprgn_mod = 0;
	int ac_mod = 0;

	if(bSlowDecay)
	{
		/* --- Decay Timer Modifications */
		if(kudzu->affected[2].modifier <= -8000 && chance(75))
		// 1/4 decay rate when MV is <= -8000
			kudzu->obj_flags.timer += 1;
		else if(kudzu->affected[2].modifier <= -4000 && chance(50))
		// 1/2 decay rate when MV is <= -4000 and > -8000
			kudzu->obj_flags.timer += 1;
		else if(kudzu->affected[2].modifier <= -2000 && chance(25))
		// 3/4 decay rate when MV is <= -2000 and > -4000
			kudzu->obj_flags.timer += 1;
	}

	/* --- Damage Modifications */
	if(kudzu->affected[2].modifier <= -10000)
		kudzu->affected[0].modifier = 5;
	else if(kudzu->affected[2].modifier <= -4000)
		kudzu->affected[0].modifier = 4;
	else if(kudzu->affected[2].modifier <= -1000)
		kudzu->affected[0].modifier = 3;
	else if(kudzu->affected[2].modifier <= -400)
		kudzu->affected[0].modifier = 2;
	else
		kudzu->affected[0].modifier = 1;

	/* --- HP_Regen Modifications */
	hprgn_mod = fabs(kudzu->affected[2].modifier/131);
	kudzu->affected[1].modifier = 5 + hprgn_mod;

	/* --- AC Modifications */
	ac_mod = MIN(25,fabs(kudzu->affected[2].modifier/353)); //AC maxed at 25, Hemp: 2015-03-05
	kudzu->obj_flags.value[0] = ac_mod;
}

int knot_of_kudzu(OBJ *kudzu, CHAR *ch, int cmd, char *argument)
{
	struct affected_type_5 af;
	int j = 0;
	int iChance = 0;
	int iMaxRN = 0;

	if(!(ch=kudzu->equipped_by)) return FALSE;

	switch(cmd)
	{
		case MSG_BEING_REMOVED:
			//Reset stats if kudzu is removed
			if(kudzu==EQ(ch, WEAR_WRIST_L))
			{
				unequip_char(ch, WEAR_WRIST_L);
				reset_kudzu(kudzu);
				equip_char(ch, kudzu, WEAR_WRIST_L);
			}
			if(kudzu==EQ(ch, WEAR_WRIST_R))
			{
				unequip_char(ch, WEAR_WRIST_R);
				reset_kudzu(kudzu);
				equip_char(ch, kudzu, WEAR_WRIST_R);
			}
			break;
		case MSG_MOBACT:
			if(kudzu==EQ(ch, WEAR_WRIST_L) && ch->specials.fighting)
			{
				if(kudzu->affected[2].modifier <= -8000)
				{
					iChance = 5;
				  iMaxRN = 23;
				}
				else if(kudzu->affected[2].modifier <= -4000)
				{
					iChance = 3;
					iMaxRN = 17;
				}
				else if(kudzu->affected[2].modifier <= -2000)
				{
					iChance = 2;
					iMaxRN = 13;
				}
				else
				{
					iChance = 1;
					iMaxRN = 11;
				}

				if(iChance > number(0,1000))
				{
					act("A shoot of kudzu on $n's left arm gets sliced off amid the fray.", FALSE, ch, NULL, NULL, TO_ROOM);
					act("A shoot of kudzu on your left arm gets sliced off amid the fray.", FALSE, ch, NULL, NULL, TO_CHAR);
					j = number(3, iMaxRN);
					unequip_char(ch, WEAR_WRIST_L);
					kudzu->affected[2].modifier += j;
					update_kudzu(kudzu, 0);
					equip_char(ch, kudzu, WEAR_WRIST_L);
				}
			}/* end WEAR_WRIST_L */
			if(kudzu==EQ(ch, WEAR_WRIST_R) && ch->specials.fighting)
			{
				if(kudzu->affected[2].modifier <= -8000)
				{
					iChance = 5;
				  iMaxRN = 23;
				}
				else if(kudzu->affected[2].modifier <= -4000)
				{
					iChance = 3;
					iMaxRN = 17;
				}
				else if(kudzu->affected[2].modifier <= -2000)
				{
					iChance = 2;
					iMaxRN = 13;
				}
				else
				{
					iChance = 1;
					iMaxRN = 11;
				}

				if(iChance > number(0,1000))
				{
					act("A shoot of kudzu on $n's right arm gets sliced off amid the fray.", FALSE, ch, NULL, NULL, TO_ROOM);
					act("A shoot of kudzu on your right arm gets sliced off amid the fray.", FALSE, ch, NULL, NULL, TO_CHAR);
					j = number(3, iMaxRN);
					unequip_char(ch, WEAR_WRIST_R);
					kudzu->affected[2].modifier += j;
					update_kudzu(kudzu, 0);
					equip_char(ch, kudzu, WEAR_WRIST_R);
				}
			}/* end WEAR_WRIST_R */
			break;
		case MSG_TICK:
			if(time_info.month <= 2 || time_info.month >= 15)
			{/* winter - no growth / cold shatter */
				if(world[CHAR_REAL_ROOM(ch)].sector_type == SECT_ARCTIC ||
					world[CHAR_REAL_ROOM(ch)].sector_type == SECT_MOUNTAIN)
				{
					if(kudzu==EQ(ch, WEAR_WRIST_L) && chance(3))
					{
						send_to_char("The exterior kudzu vines on your left arm freeze and shatter like glass in the cold.\n\r", ch);
						j = number(3,23);
						unequip_char(ch, WEAR_WRIST_L);
						kudzu->affected[2].modifier += j;
						update_kudzu(kudzu, 0);
						equip_char(ch, kudzu, WEAR_WRIST_L);
					}/* end WEAR_WRIST_L */
					if(kudzu==EQ(ch, WEAR_WRIST_R) && chance(3))
					{
						send_to_char("The exterior kudzu vines on your right arm freeze and shatter like glass in the cold.\n\r", ch);
						j = number(3,23);
						unequip_char(ch, WEAR_WRIST_R);
						kudzu->affected[2].modifier += j;
						update_kudzu(kudzu, 0);
						equip_char(ch, kudzu, WEAR_WRIST_R);
					}/* end WEAR_WRIST_R */
				}
				break;
			}
			else if(time_info.month == 4)
			{/* spring (The Grand Struggle) - accelerated growth */
				if(kudzu==EQ(ch, WEAR_WRIST_L) && chance(7))
				{
					send_to_char("With the mighty awakening of spring, a small bud appears on the kudzu vine.\n\r", ch);
					j = number(2,7);
					unequip_char(ch, WEAR_WRIST_L);
					kudzu->affected[2].modifier -= j;
					update_kudzu(kudzu, 1);
					equip_char(ch, kudzu, WEAR_WRIST_L);
				}/* end WEAR_WRIST_L */
				if(kudzu==EQ(ch, WEAR_WRIST_R) && chance(7))
				{
					send_to_char("With the mighty awakening of spring, a small bud appears on the kudzu vine.\n\r", ch);
					j = number(2,7);
					unequip_char(ch, WEAR_WRIST_R);
					kudzu->affected[2].modifier -= j;
					update_kudzu(kudzu, 1);
					equip_char(ch, kudzu, WEAR_WRIST_R);
				}/* end WEAR_WRIST_R */
			}
			else if(time_info.month >= 8 && time_info.month <= 10)
			{/* summer - wither */
				if(IS_DAY && world[CHAR_REAL_ROOM(ch)].sector_type == SECT_DESERT)
				{
					if(GET_COND(ch, THIRST) < 5)
						iChance = 47;
					else
						iChance = 23;
					if(kudzu==EQ(ch, WEAR_WRIST_L) && chance(iChance))
					{
						send_to_char("Vines of kudzu fall to the ground, withered by the scorching summer heat.\n\r", ch);
						j = number(3,11);
						unequip_char(ch, WEAR_WRIST_L);
						kudzu->affected[2].modifier += j;
						update_kudzu(kudzu, 0);
						equip_char(ch, kudzu, WEAR_WRIST_L);
					}/* end WEAR_WRIST_L */
					if(kudzu==EQ(ch, WEAR_WRIST_R) && chance(iChance))
					{
						send_to_char("Vines of kudzu fall to the ground, withered by the scorching summer heat.\n\r", ch);
						j = number(3,11);
						unequip_char(ch, WEAR_WRIST_R);
						kudzu->affected[2].modifier += j;
						update_kudzu(kudzu, 0);
						equip_char(ch, kudzu, WEAR_WRIST_R);
					}/* end WEAR_WRIST_R */
					break;
				}
			}

			if(kudzu==EQ(ch, WEAR_WRIST_L))
			{
				if(ch->specials.timer >= 20 || OBJ_SPEC(kudzu) > 0)
				{/* poison idle wearers */
					act("A restless kudzu vine suddenly pierces $n's skin.", FALSE, ch, NULL, NULL, TO_ROOM);
					act("A restless kudzu vine suddenly pierces your skin.", FALSE, ch, NULL, NULL, TO_CHAR);
					if(OBJ_SPEC(kudzu) <= 0)
						OBJ_SPEC(kudzu) = 7;
					else
						OBJ_SPEC(kudzu)--;
					af.type = SPELL_POISON;
					af.duration = 20;
					af.location = APPLY_STR;
					af.modifier = -2;
					af.bitvector = AFF_POISON;
					af.bitvector2 = 0;
					affect_join(ch, &af, FALSE, FALSE);
					send_to_char("You feel very sick.\n\r",ch);
				}

				if(world[CHAR_REAL_ROOM(ch)].sector_type == SECT_INSIDE ||
					IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, INDOORS))
				{/* indoors */
					if(IS_DAY)
						j = number(0,1);
					else
						j = 0;
				}
				else
				{/* outdoors */
					if(IS_DAY)
						j = number(0,3);
					else
						j = number(0,1);
				}

				if(affected_by_spell(ch, SPELL_POISON) && chance(42))
				{
					j = -number(0,5);
					send_to_char("Poison in your veins seeps into your left arm's knot of kudzu, slowing its growth.\n\r", ch);
				}
				else if(enchanted_by(ch, "Regeneration") && IS_DAY && GET_COND(ch, FULL) >= 5 && chance(42))
				{
					j++;
					send_to_char("Wrapped lovingly around your chlorophyll infused skin, the kudzu grows.\n\r", ch);
				}
				else if(GET_COND(ch, FULL) < 5 && j > 0)
					j--;

				//Update kudzu
				if(j > 0)
				{
					unequip_char(ch, WEAR_WRIST_L);
					kudzu->affected[2].modifier -= j;
					update_kudzu(kudzu, 1);
					equip_char(ch, kudzu, WEAR_WRIST_L);
					act("The knot of kudzu on $n's left arm tightens as new vine nodes sprout.", FALSE, ch, NULL, NULL, TO_ROOM);
					act("The knot of kudzu on your left arm tightens as new vine nodes sprout.", FALSE, ch, NULL, NULL, TO_CHAR);
				}
				else if(j < 0)
				{
					unequip_char(ch, WEAR_WRIST_L);
					kudzu->affected[2].modifier += fabs(j);
					update_kudzu(kudzu, 0);
					equip_char(ch, kudzu, WEAR_WRIST_L);
					act("The knot of kudzu on $n's left arm loosens.", FALSE, ch, NULL, NULL, TO_ROOM);
					act("The knot of kudzu on your left arm withers slightly, loosening its grip.", FALSE, ch, NULL, NULL, TO_CHAR);
				}
			}/* end WEAR_WRIST_L */
			if(kudzu==EQ(ch, WEAR_WRIST_R))
			{
				if(ch->specials.timer >= 20 || OBJ_SPEC(kudzu) > 0)
				{/* poison idle wearers */
					act("A restless kudzu vine suddenly pierces $n's skin.", FALSE, ch, NULL, NULL, TO_ROOM);
					act("A restless kudzu vine suddenly pierces your skin.", FALSE, ch, NULL, NULL, TO_CHAR);
					if(OBJ_SPEC(kudzu) <= 0)
						OBJ_SPEC(kudzu) = 7;
					else
						OBJ_SPEC(kudzu)--;
					af.type = SPELL_POISON;
					af.duration = 20;
					af.location = APPLY_STR;
					af.modifier = -2;
					af.bitvector = AFF_POISON;
					af.bitvector2 = 0;
					affect_join(ch, &af, FALSE, FALSE);
					send_to_char("You feel very sick.\n\r",ch);
				}

				if(world[CHAR_REAL_ROOM(ch)].sector_type == SECT_INSIDE ||
					IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, INDOORS))
				{/* indoors */
					if(IS_DAY)
						j = number(0,1);
					else
						j = 0;
				}
				else
				{/* outdoors */
					if(IS_DAY)
						j = number(0,3);
					else
						j = number(0,1);
				}

				if(affected_by_spell(ch, SPELL_POISON) && chance(42))
				{
					j = -number(0,5);
					send_to_char("Poison in your veins seeps into your right arm's knot of kudzu, slowing its growth.\n\r", ch);
				}
				else if(enchanted_by(ch, "Regeneration") && IS_DAY && GET_COND(ch, FULL) >= 5 && chance(42))
				{
					j++;
					send_to_char("Wrapped lovingly around your chlorophyll infused skin, the kudzu grows.\n\r", ch);
				}
				else if(GET_COND(ch, FULL) < 5 && j > 0)
					j--;

				//Update kudzu
				if(j > 0)
				{
					unequip_char(ch, WEAR_WRIST_R);
					kudzu->affected[2].modifier -= j;
					update_kudzu(kudzu, 1);
					equip_char(ch, kudzu, WEAR_WRIST_R);
					act("The knot of kudzu on $n's right arm tightens as new vine nodes sprout.", FALSE, ch, NULL, NULL, TO_ROOM);
					act("The knot of kudzu on your right arm tightens as new vine nodes sprout.", FALSE, ch, NULL, NULL, TO_CHAR);
				}
				else if(j < 0)
				{
					unequip_char(ch, WEAR_WRIST_R);
					kudzu->affected[2].modifier += fabs(j);
					update_kudzu(kudzu, 0);
					equip_char(ch, kudzu, WEAR_WRIST_R);
					act("The knot of kudzu on $n's right arm loosens.", FALSE, ch, NULL, NULL, TO_ROOM);
					act("The knot of kudzu on your right arm withers slightly, loosening its grip.", FALSE, ch, NULL, NULL, TO_CHAR);
				}
			}/* end WEAR_WRIST_R */
			break;
		default:
			break;
  }/* end switch */
  return FALSE;
}/* end int kudzu_bracelet() */

/* ************************ End of Troll Castle - Moat ******************** */

#define TROLL_ZONE                      7700
#define TROLL_LIZARDAPPRENTICE          ITEM(TROLL_ZONE,1)
#define TROLL_TYPIK                     ITEM(TROLL_ZONE,3)
#define KNOT_OF_KUDZU					7704

void assign_trollcastle_moat (void) {
  assign_mob(TROLL_LIZARDAPPRENTICE    ,  troll_lizardapprentice);
  assign_mob(TROLL_TYPIK               ,  troll_lizardshaman);
  assign_obj(KNOT_OF_KUDZU, knot_of_kudzu);
}
