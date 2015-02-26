 /* ************************************************************************
  *  file: spec_procs.c , Special module.                   Part of DIKUMUD *
  *  Usage: Procedures handling special procedures for object/room/mobile   *
  *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
  ************************************************************************* */

/*
$Author: ronin $
$Date: 2004/06/30 19:57:28 $
$Header: /home/ronin/cvs/ronin/spec.grail.c,v 2.1 2004/06/30 19:57:28 ronin Exp $
$Id: spec.grail.c,v 2.1 2004/06/30 19:57:28 ronin Exp $
$Name:  $
$Log: spec.grail.c,v $
Revision 2.1  2004/06/30 19:57:28  ronin
Update of saving throw routine.

Revision 2.0.0.1  2004/02/05 16:10:40  ronin
Reinitialization of cvs archives

Revision 1.3  2003/11/02  liner
Addition of log for objects lost to rust monster.

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
 #include "reception.h"
 #include "spec_assign.h"
 /*   external vars  */

 extern CHAR *character_list;
 extern struct descriptor_data *descriptor_list;
 extern struct time_info_data time_info;
 extern struct spell_info_type spell_info[MAX_SPL_LIST];

 void npc_steal(CHAR *ch,CHAR *victim)
 {
   int gold;

   if(IS_NPC(victim)) return;
   if(GET_LEVEL(victim)>LEVEL_MORT) return;

   if (AWAKE(victim) && (number(0,GET_LEVEL(ch)) < 3)) {
    act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
    act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
   } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
    if (gold > 0) {
     GET_GOLD(ch) += gold;
     GET_GOLD(victim) -= gold;
    }
   }
 }

 int thief(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *cons;

   if(cmd) return FALSE;

   if(GET_POS(mob)!=POSITION_STANDING)return FALSE;

   for(cons = world[CHAR_REAL_ROOM(mob)].people; cons; cons = cons->next_in_room )
     if((!IS_NPC(cons)) && (GET_LEVEL(cons)<LEVEL_IMM) && (number(1,3)==1))
       npc_steal(mob,cons);

   return FALSE;
 }

 int rust_monster(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   int damaged;
   char buf[MAX_INPUT_LENGTH];
   OBJ *eq = 0;
   CHAR *victim;

   if (cmd) return FALSE;

   for(victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = victim->next_in_room )
    if((!IS_NPC(victim)) && (GET_LEVEL(victim)<LEVEL_IMM) && (number(1,2)==1))

   if (!saves_spell(victim, SAVING_BREATH,GET_LEVEL(mob))) {
    damaged = number(0,17);
    if (victim->equipment[damaged])
     eq=victim->equipment[damaged];
    else return FALSE;
    act("$N eats your $o.", 0, victim, eq, mob, TO_CHAR);
    unequip_char(victim, damaged);
    extract_obj(eq);

    if(IS_MORTAL(victim)) {  /* Linerfix 110203 */
      sprintf(buf,"ObjLog: [ %s had %s extracted at %d ]",GET_NAME(victim),OBJ_SHORT(eq),world[CHAR_REAL_ROOM(victim)].number);
      log_s(buf);
    }
  }
   return FALSE;
 }

 int snake(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   if(cmd) return FALSE;

   if(GET_POS(mob)!=POSITION_FIGHTING) return FALSE;

   if (mob->specials.fighting &&
     (CHAR_REAL_ROOM(mob->specials.fighting) == CHAR_REAL_ROOM(mob)) &&
     (number(0,32-GET_LEVEL(mob))==0)) {
     act("$n bites $N!", 1, mob, 0, mob->specials.fighting, TO_NOTVICT);
     act("$n bites you!", 1, mob, 0, mob->specials.fighting, TO_VICT);
     cast_poison( GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, mob->specials.fighting, 0);
   }
   return FALSE;
 }

 /**********************************************************************\
 |* Special procedures for Holy Grail                                  *|
 |* Designed by Ni         Coded by NoName                             *|
 \**********************************************************************/

 int crowd(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {

   if (cmd) return FALSE;

   switch (number(0,5)) {
    case 0:
     do_say(mob, "Burn her, Burn her!", 0);
     break;
   case 1:
     do_say(mob, "She turned me into a Newt.", 0);
     break;
   case 2:
     do_say(mob, "If she has the same weight as a duck then she is a witch!", 0);
     break;
   default:
     break;
   }

   return FALSE;
 }

 int french_guard(CHAR *mob,CHAR *ch,int cmd,char *arg)
 {

   if (cmd) return FALSE;

   switch (number(0,7)) {
   case 0:
     do_say(mob, "I fart in your general direction dirty english Kniggets!", 0);
     break;
   case 1:
     do_say(mob, "I unclod my nose in your direction!", 0);
     break;
   case 2:
     do_say(mob, "You don't frighten us, English pig dogs!", 0);
     break;
   default:
     break;
   }
   return FALSE;
 }

 int man(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {

   if (cmd) return FALSE;

   switch (number(0,2)) {
   case 0:
     do_say(mob, "Bring out your dead!", 0);
     break;
   default:
     break;
   }
   return FALSE;
 }

 int knight(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   void do_say(CHAR *ch, char *argument, int cmd);

   if (cmd != CMD_NORTH) return FALSE;
   if (GET_LEVEL(ch)<25) return FALSE;

   if (CHAR_REAL_ROOM(mob) == real_room(8609)) {
     do_say(mob,"You shall not pass unless you fight me!", CMD_SAY);
     return TRUE;
   }

   if (CHAR_REAL_ROOM(ch) == real_room(8644)) {
     do_say(mob,"To get past here, you must cut down", CMD_SAY);
     do_say(mob,"the mightiest tree in the forest with a HERRING!", CMD_SAY);
   }
   return FALSE;
 }

 int tim_enchanter(CHAR *mob,CHAR *ch,int cmd,char *arg)
 {
   CHAR *vict;

   if (cmd) return FALSE;

   if (GET_POS(mob) != POSITION_FIGHTING) return FALSE;

   if (!mob->specials.fighting) return FALSE;

   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room)
    if (vict->specials.fighting==mob && number(0,5)==0)
      break;

   if (!vict) return FALSE;

   act("$n points $s old staff towards $N.", 1, mob, 0, vict, TO_NOTVICT);
   act("$n points $s old staff towards you.", 1, mob, 0, vict, TO_VICT);

   cast_fireball(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
   return FALSE;
 }

 /**********************************************************************\
 |* End Of the Special procedures for Holy Grail                        *|
 \**********************************************************************/

#define GRAIL_ZONE    8600
#define GRAIL_CROWD   ITEM(GRAIL_ZONE, 5)
#define GRAIL_MAN     ITEM(GRAIL_ZONE,16)
#define GRAIL_KNIGHT1 ITEM(GRAIL_ZONE, 1)
#define GRAIL_KNIGHT2 ITEM(GRAIL_ZONE, 6)
#define GRAIL_TIM     ITEM(GRAIL_ZONE,14)
#define GRAIL_GUARD   ITEM(GRAIL_ZONE, 4)

void assign_grail (void) {

  assign_mob(GRAIL_CROWD  , crowd);
  assign_mob(GRAIL_MAN    , man);
  assign_mob(GRAIL_KNIGHT1, knight);
  assign_mob(GRAIL_KNIGHT2, knight);
  assign_mob(GRAIL_TIM    , tim_enchanter);
  assign_mob(GRAIL_GUARD  , french_guard);

  assign_mob(5708,rust_monster);

  assign_mob(4000,snake);
  assign_mob(4001,snake);
  assign_mob(4053,snake);

  assign_mob(4103,thief);
  assign_mob(3145,thief);
  assign_mob(4102,snake);
  assign_mob(1506,snake);
  assign_mob(1507,snake);
  assign_mob(1508,snake);

  /* SEWERS */
  assign_mob(7006,snake);

  /* FOREST */
  assign_mob(6113,snake);
  assign_mob(6114,snake);
}


