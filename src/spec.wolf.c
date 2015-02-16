 /* ************************************************************************
  *  file: spec_procs.c , Special module.                   Part of DIKUMUD *
  *  Usage: Procedures handling special procedures for object/room/mobile   *
  *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
  ************************************************************************* */

/*
$Author: ronin $
$Date: 2004/02/05 16:11:35 $
$Header: /home/ronin/cvs/ronin/spec.wolf.c,v 2.0.0.1 2004/02/05 16:11:35 ronin Exp $
$Id: spec.wolf.c,v 2.0.0.1 2004/02/05 16:11:35 ronin Exp $
$Name:  $
$Log: spec.wolf.c,v $
Revision 2.0.0.1  2004/02/05 16:11:35  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:16  ronin
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
 #include "reception.h"
#include "spec_assign.h"
 /*   external vars  */

 extern struct room_data *world;
 extern struct mob_proto *mob_proto_table;
 extern struct obj_proto *obj_proto_table;
 extern CHAR *character_list;
 extern struct descriptor_data *descriptor_list;
 extern struct time_info_data time_info;
 extern struct spell_info_type spell_info[MAX_SPL_LIST];

 /**********************************************************************\
 |* Special procedures for Wolf Cave by James Yung                     *|
 \**********************************************************************/
int wolf_chew_toy(CHAR *mob)
{
   OBJ *i,*temp,*next_obj,*tmp;

   if (GET_POS(mob) != POSITION_FIGHTING)
     {
     for (i=world[CHAR_REAL_ROOM(mob)].contents;i;i=tmp) {
       tmp = i->next_content; /* Added tmp - Ranger June 96 */
       if (GET_ITEM_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3])
         {
         act("$n is eating a corpse.", FALSE, mob, 0, 0, TO_ROOM);
         for (temp=i->contains;temp;temp=next_obj)
           {
	   next_obj = temp->next_content;
	   obj_from_obj(temp);
	   obj_to_room(temp, CHAR_REAL_ROOM(mob));
           }
         extract_obj(i);
         return TRUE;
         }
       }
     }
   return FALSE;
}
int wolf_options(CHAR *mob, CHAR *vict)
{
  OBJ *wield;
  int start,stop;
  char buf[MAX_INPUT_LENGTH];
  if (isname("guard", GET_NAME(mob)))
    {
    start = 1; stop = 3;
    }
  else if (isname("lord", GET_NAME(mob)))
    {
    start = 2; stop = 6;
    }
  else if (isname("king", GET_NAME(mob)))
    {
    start = 5; stop = 7;
    }
  else
    {
    start=99;
    stop =99;
    }

  switch(number(start,stop))
     {
     case 1: /* Guard */
	act("$n hits $N hard with $s claw!", 1, mob, 0, vict, TO_NOTVICT);
	act("$n hits you hard with $s claw!", 1, mob, 0, vict, TO_VICT);
	damage(mob, vict, number(5,10), TYPE_UNDEFINED,DAM_SKILL);
	cast_poison(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
	return FALSE;
     case 2:/* Guard & Lord*/
	act("$n bites $N's arm!", 1, mob, 0, vict, TO_NOTVICT);
	act("$n bites your arm!", 1, mob, 0, vict, TO_VICT);
	damage(mob, vict, number(10,20), TYPE_UNDEFINED,DAM_SKILL);
	return FALSE;
      case 3:/* Guard & Lord*/
	act("$n bites $N's leg!", 1, mob, 0, vict, TO_NOTVICT);
	act("$n bites your leg!", 1, mob, 0, vict, TO_VICT);
	damage(mob, vict, number(25,35), TYPE_UNDEFINED,DAM_SKILL);
	WAIT_STATE(vict, PULSE_VIOLENCE*2);
	return FALSE;
     case 4: /* Lord */
	act("$n bites $N's foot!", 1, mob, 0, vict, TO_NOTVICT);
	act("$n bites your foot!", 1, mob, 0, vict, TO_VICT);
	damage(mob, vict, number(5,10), TYPE_UNDEFINED,DAM_SKILL);
	if (GET_MOVE(vict) > 10)
	  GET_MOVE(vict) -= 10;
	else
	  GET_MOVE(vict) = 0;
	return FALSE;
     case 5: /* King & Lord */
        act("$n claws $N's hand!", 1, mob, 0, vict, TO_NOTVICT);
	act("$n claws your hand!", 1, mob, 0, vict, TO_VICT);
	damage(mob, vict, number(15,30), TYPE_UNDEFINED,DAM_SKILL);
	if (vict->equipment[WIELD])
	 wield = vict->equipment[WIELD];
	else return FALSE;
	act("$N drop $S weapon.", 1, mob, 0, vict, TO_NOTVICT);
	act("You drop your weapon.", 1, mob, 0, vict, TO_VICT);
	/* Disarm log added by Solmyr 2009 */
	sprintf(buf, "WIZINFO: %s disarms %s's %s at %d", GET_NAME(mob), GET_NAME(vict), OBJ_SHORT(wield), world[CHAR_REAL_ROOM(mob)].number);
	log_f(buf);
	wield->log = TRUE;
	unequip_char(vict, WIELD);
	obj_to_room(wield, CHAR_REAL_ROOM(vict));
	return FALSE;
     case 6: /* Lord & King */
	act("$n bites $N's neck with $s huge mouth and sucks $S blood!", 1, mob, 0, vict, TO_NOTVICT);
	act("$n bites your neck with $s huge mouth and sucks your blood!", 1, mob, 0, vict, TO_VICT);
	damage(mob, vict, number(15,25), TYPE_UNDEFINED,DAM_SKILL);
	if (GET_MOVE(vict) > 20)
	  GET_MOVE(vict) -= 20;
	else
	  GET_MOVE(vict) = 0;
	if (GET_MANA(vict) > 40)
	  GET_MANA(vict) -= 40;
	else
	  GET_MANA(vict) = 0;
	return FALSE;
      case 7: /*King*/
	act("$n opens $s huge mouth!", 1, mob, 0, vict, TO_ROOM);
	act("There is a BIG fireball shooting from $n's mouth towards $N!",
	     1, mob, 0, vict, TO_NOTVICT);
	act("$N enveloped in flames!", 1, mob, 0, vict, TO_NOTVICT);
	act("There is a BIG fireball shooting from $n's mouth towards YOU!",
	     1, mob, 0, vict, TO_VICT);
	act("You enveloped in flames!", 1, mob, 0, vict, TO_VICT);
	damage(mob, vict, number(40,60), TYPE_UNDEFINED,DAM_FIRE);
	return FALSE;
      case 99:
        return FALSE;
      }
   return FALSE;
}
 int wolf_guard(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {

   CHAR *vict;

   if (cmd || !AWAKE(mob)) return FALSE;

   if(wolf_chew_toy(mob))
       return(FALSE);
   else
     {
     for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room)
      if (vict->specials.fighting==mob && number(0,2)==0)
	break;

     if (!vict) return FALSE;
     return(wolf_options(mob,vict));
     }
   return FALSE;
 }

 int wolf_lord(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict;

   if (cmd || !AWAKE(mob)) return FALSE;

   if(wolf_chew_toy(mob))
       return(FALSE);
   else {
     for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room)
      if (vict->specials.fighting==mob && number(0,1)==0)
	break;

     if (!vict) return FALSE;

     return(wolf_options(mob,vict));
   }
   return FALSE;
 }
int wolf_priest(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {

   CHAR *vict;

   if (cmd || !AWAKE(mob)) return FALSE;

   for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room) {
    if (vict) {
     if (IS_NPC(vict) && (GET_HIT(vict) != hit_limit(vict))) {
      if (mob_proto_table[vict->nr].virtual > 9500 &&
	  mob_proto_table[vict->nr].virtual < 9507) {
       act("$n takes the first aid box from $s pocket and cures $N.",
	   1, mob, 0, vict, TO_NOTVICT);
       if (GET_POS(mob) == POSITION_FIGHTING) {
	if ((40 + GET_HIT(vict)) > hit_limit(vict))
	  GET_HIT(vict) = hit_limit(vict);
	else
	  GET_HIT(vict) += 40;
        }
       else {
	 if ((70 + GET_HIT(vict)) > hit_limit(vict))
	   GET_HIT(vict) = hit_limit(vict);
	 else
	   GET_HIT(vict) += 70;
       }
       update_pos(vict);
      }
     }
    }
   }
   return FALSE;
 }

 int wolf_queen(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict, *tmp;
   int minhp=0;

   if (cmd || !AWAKE(mob)) return FALSE;

   if(wolf_chew_toy(mob))
     return(FALSE);
   else {
     vict = mob->specials.fighting;
     if (vict) {
      for (tmp=world[CHAR_REAL_ROOM(mob)].people;tmp;tmp=tmp->next_in_room) {
       if (tmp->specials.fighting==mob && (GET_HIT(tmp) < minhp)) {
	 minhp = GET_HIT(tmp);
	 vict=tmp;
       }
      }
      if ((vict != mob->specials.fighting) &&
	  (GET_POS(vict) > POSITION_DEAD) &&
	  (number(1,5) < 3)) {
	act("$n chooses $s mind and hits $N.", 1, mob, 0, vict, TO_NOTVICT);
	stop_fighting(mob);
	hit(mob, vict, TYPE_UNDEFINED);
      }
     }
   }
   return FALSE;
 }

 int wolf_king(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict;

   if (cmd || !AWAKE(mob)) return FALSE;

   if(wolf_chew_toy(mob))
     return(FALSE);
   else {
     for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room)
      if (vict->specials.fighting==mob && number(0,1)==0)
	break;

     if (!vict) return FALSE;

     return(wolf_options(mob,vict));
     }
   return FALSE;
 }


 /**********************************************************************\
 |* End Of the Special procedures for Wolf Cave                        *|
 \**********************************************************************/

#define WOLF_ZONE    9500
#define WOLF_GUARD   ITEM(WOLF_ZONE,2)
#define WOLF_LORD    ITEM(WOLF_ZONE,3)
#define WOLF_PRIEST  ITEM(WOLF_ZONE,6)
#define WOLF_QUEEN   ITEM(WOLF_ZONE,5)
#define WOLF_KING    ITEM(WOLF_ZONE,4)

void assign_wolf (void) {

  assign_mob(WOLF_GUARD , wolf_guard);
  assign_mob(WOLF_LORD  , wolf_lord);
  assign_mob(WOLF_PRIEST, wolf_priest);
  assign_mob(WOLF_QUEEN , wolf_queen);
  assign_mob(WOLF_KING  , wolf_king);
}

