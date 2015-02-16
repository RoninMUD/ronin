/*
///   Special Procedure Module                Orig. Date 15-02-1994
///                                        Last Modified 28-07-1996 - Ranger
///
///   spec.pagoda.c --- Specials for Golden Pagoda
///
///
///   Coded by Lem of RoninMUD (worm@usacs.rutgers.edu)
///   Using this code is not allowed without permission from originator.
///
///   Mod.    - Assigned ZONE_PAGODAII 26500 for GURU
///           - Changed define PAGODA_GURU using ZONE_PAGODAII   Ranger
///           - added tyrant spec July 28/96                     Ranger
//            - added items specs for wand, sphere and cloak     Quack
//            - added more specs for tyrant & emir Nov 18/96     Quack
*/

/*
$Author: ronin $
$Date: 2004/06/30 19:57:28 $
$Header: /home/ronin/cvs/ronin/spec.pagoda.c,v 2.3 2004/06/30 19:57:28 ronin Exp $
$Id: spec.pagoda.c,v 2.3 2004/06/30 19:57:28 ronin Exp $
$Name:  $
$Log: spec.pagoda.c,v $
Revision 2.3  2004/06/30 19:57:28  ronin
Update of saving throw routine.

Revision 2.2  2004/05/12 13:21:30  ronin
Dice roll prize descriptions will only show up if the player rolling the
quest dice has their quest flag turned on.

Revision 2.1  2004/05/05 20:40:17  ronin
Added prize list with die roll display.

Revision 2.0.0.1  2004/02/05 16:11:10  ronin
Reinitialization of cvs archives

Revision 1.3  2002/03/31 16:45:59  ronin
Added bool to extern bool is_caster(CHAR *CH) to remove default to int
declaration of is_caster.

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
 #include "mob.spells.h"
 /*   external vars  */

 extern CHAR *character_list;
 extern bool is_caster(CHAR *CH);
 extern struct descriptor_data *descriptor_list;
 extern struct spell_info_type spell_info[MAX_SPL_LIST];

int p_emir(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict=NULL;
  if (cmd) return FALSE;

  if (!mob->specials.fighting) return FALSE;

  if(!number(0,3)) {
    if(!(vict=get_random_victim(mob))) return FALSE;
    if (vict!=mob->specials.fighting) spell_vanish (LEVEL_IMM,mob,vict,0);
   }

return FALSE;
}

int p_tyrant(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict=NULL,*next_vict=NULL;
  int num_pcs,dam;

  if (cmd) return FALSE;

  if (!mob->specials.fighting) return FALSE;
  num_pcs=0;
  for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict=vict->next_in_room)
    if((vict!=mob) && (GET_LEVEL(vict)<LEVEL_IMM)) num_pcs=num_pcs+1;

  if(num_pcs>7) {

   act("$n feels there are too many victims here.",1, mob, 0, 0, TO_ROOM);
   act("You feel there are too many victims here.",1, mob, 0, 0, TO_CHAR);
   act("$n utters the words 'mishrak ak thull'!",1, mob, 0, 0, TO_ROOM);
   act("You utter the words 'mishrak ak thull'!",1, mob, 0, 0, TO_CHAR);

   for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict=next_vict) {
    next_vict = vict->next_in_room;
    if (vict!=mob) spell_dispel_sanct (LEVEL_IMM,mob,vict,0);
   }

   act("$n utters the words 'talak ud dhin'!",1, mob, 0, 0, TO_ROOM);
   act("You utter the words 'talak ud dhin'!",1, mob, 0, 0, TO_CHAR);

   for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict=next_vict) {
    next_vict = vict->next_in_room;
    if ((vict!=mob) && (GET_LEVEL(vict)<LEVEL_IMM)) {
      dam = number(60, 100);
      if (saves_spell(vict, SAVING_SPELL,GET_LEVEL(mob)))  dam >>= 1;
      damage(mob, vict, dam, SPELL_COLOUR_SPRAY,DAM_MAGICAL);
    }
   }
  }
/* Petrify any in room except tank */
  if(!number(0,4)) {
    if((vict=get_random_victim(mob))){
      if (vict!=mob->specials.fighting) spell_petrify (LEVEL_IMM,mob,vict,0);
      return FALSE;
    }
  }

/* Drain mana from any caster in room */
  if(!number(0,3)) {
    if((vict=get_random_victim(mob))) {
      if (is_caster(vict)) spell_drain_mana (LEVEL_IMM,mob,vict,0);
      return FALSE;
    }
  }

/* Damage the tank */
  if(!number(0,4)) {
    vict=mob->specials.fighting;
    act("$n gently touches $N who screams in agony.",1, mob, 0,vict,  TO_ROOM);
    act("$n gently touches you and you scream in agony.",1, mob, 0,vict,  TO_VICT);
    act("You touch $N and teach $S the meaning of pain.",1, mob, 0,vict,  TO_CHAR);
    damage(mob, vict, 400, SPELL_CHILL_TOUCH,DAM_COLD);
    return FALSE;
  }

  return FALSE;
}

int p_wand(OBJ *wand, CHAR *ch,int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int i,j;

  if(cmd==MSG_TICK) {
    if(wand->obj_flags.value[3]>0) wand->obj_flags.value[3]--;
    return FALSE;
  }

  if(cmd==MSG_OBJ_ENTERING_GAME) {
    if(is_number(arg)) i=atoi(arg);
    else return FALSE;
    j=wand->obj_flags.value[3]*60; /* Secs left to recharge */
    if(i>j) wand->obj_flags.value[3]=0;
    return FALSE;
  }

  if (!ch) return (FALSE);
  if (cmd!=CMD_USE) return (FALSE);

  one_argument (arg,buf);

  if (!isname (buf,OBJ_NAME(wand))) return (FALSE);
  if (wand != EQ(ch,HOLD)) return (FALSE);

  if(!wand->obj_flags.value[3]) {
    if(chance(75) && CHAR_VIRTUAL_ROOM(ch)!=586) { /* arago room check added july/00 -Sumo */
      act("$p opens a doorway to a different dimension and pulls you in.",FALSE,ch,wand,0,TO_CHAR);
      act("$p opens a doorway to a different dimension and pulls you in.",FALSE,ch,wand,0,TO_ROOM);
      spell_total_recall(LEVEL_IMM-1,ch,ch,SPELL_TYPE_SPELL);
      wand->obj_flags.value[3]=240;
      return (TRUE);
    }
    else {
      act("Your $p shudders violently, and explodes in a shower of magical energy!",FALSE,ch,wand,0,TO_CHAR);
      act("$n's $o shudders violently, and explodes in a shower of magical energy!",FALSE,ch,wand,0,TO_ROOM);
      damage (ch,ch, number(25,75),TYPE_UNDEFINED,DAM_NO_BLOCK);
      if(wand==EQ(ch,HOLD)) extract_obj(unequip_char(ch,HOLD));
      else extract_obj(wand);
      return TRUE;
    }
  }
  act("$p seems powerless.",FALSE,ch,wand,0,TO_CHAR);
  return (TRUE);
}

int p_sphere(OBJ *sphere, CHAR *ch,int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(cmd==MSG_TICK) {
    if(sphere->obj_flags.value[3]>0) sphere->obj_flags.value[3]--;
    return FALSE;
  }
  if(!ch) return (FALSE);
  if(cmd!=CMD_USE) return (FALSE);

  one_argument (arg,buf);

  if (!isname (buf,OBJ_NAME(sphere))) return (FALSE);
  if (sphere!= EQ(ch,WEAR_LIGHT)) return (FALSE);

  if(!sphere->obj_flags.value[3]) {
    act("$p shoots out rays of darklight towards you.",FALSE,ch,sphere,0,TO_CHAR);
    act("$p shoots out rays of darklight towards $n.",FALSE,ch,sphere,0,TO_ROOM);
    spell_layhands(LEVEL_IMM-1,ch,ch,SPELL_TYPE_SPELL);
    sphere->obj_flags.value[3]=120;
    return (TRUE);
  }
  act("$p draws some of your lifeforce into it.", FALSE,ch,sphere,0,TO_CHAR);
  damage (ch,ch, number(10,60),TYPE_UNDEFINED,DAM_NO_BLOCK);
  return (TRUE);
}

int fanatic (CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   void do_say(CHAR *ch, char *argument, int cmd);

   if ((cmd != CMD_SOUTH) && (cmd != CMD_UP)) return FALSE;

   if ((CHAR_REAL_ROOM(mob) == real_room(26475)) && (cmd != CMD_UP))
   {
     do_say(mob,"You are not welcome here!  Go back!", CMD_SAY);
     return TRUE;
   }

   if ((CHAR_REAL_ROOM(mob) == real_room(26476)) && (cmd != CMD_SOUTH))
   {
     do_say(mob,"Only the faithful may pass.", CMD_SAY);
     return TRUE;
   }

  if ((CHAR_REAL_ROOM(mob) == real_room(26477)) && (cmd != CMD_SOUTH))
  {
     do_say(mob,"The uninitiated are not allowed beyond this point!", CMD_SAY);
     return TRUE;
   }

   if ((CHAR_REAL_ROOM(mob) == real_room(26478)) && (cmd != CMD_SOUTH))
   {
     do_say(mob,"A curse on all unbelievers!  You may NOT pass!", CMD_SAY);
     return TRUE;
   }

   if ((CHAR_REAL_ROOM(mob) == real_room(26479)) && (cmd != CMD_SOUTH))
   {
     do_say(mob,"Please, friend, leave us alone.", CMD_SAY);
     return TRUE;
   }

   if ((CHAR_REAL_ROOM(mob) == real_room(26480)) && (cmd != CMD_SOUTH))
   {
     do_say(mob,"Stay back, or I'll ship you to the fourth plane!", CMD_SAY);
     return TRUE;
   }

   if ((CHAR_REAL_ROOM(mob) == real_room(26581)) && (cmd != CMD_SOUTH))
   {
     do_say(mob,"You shall only pass over my dead body!", CMD_SAY);
     return TRUE;
   }

   return FALSE;
 }

int p_dice(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  char arg[MAX_STRING_LENGTH],objname[MAX_STRING_LENGTH];
  int num;
  char *prizes[20] = {
    "Choice of Potion",
    "Walkin/out",
    "+1 Hitroll to a non-quest item",
    "Free Vanity",
    "Free HP or Mana Meta",
    "Subclass Token Roll",
    "+10 HP to a non-quest item",
    "AQ Point Roll",
    "Anti-Rent Item",
    "Walkin/out",
    "Subclass Token Roll",
    "+1 Damroll to a non-quest item",
    "Free Vanity",
    "Choice of Potion",
    "+10 Mana to a non-quest item",
    "AQ Point Roll",
    "Anti-Rent Item",
    "-5 AC to a non-quest item",
    "Free HP or Mana Meta",
    "Subclass Token Roll AND AQ Point Roll"
  };

  if(!ch) return FALSE;
  if(cmd!=CMD_THROW) return FALSE;
  if(GET_POS(ch)<=POSITION_SLEEPING) return FALSE;
  if(!obj->equipped_by) return FALSE;
  if(obj->equipped_by!=ch) return FALSE;

  one_argument(argument,objname);
  if(!*objname) return FALSE;

  if(!isname(objname, OBJ_NAME(obj))) {
    send_to_char("You do not hold that item in your hand.\n\r",ch);
    return TRUE;
  }

  num = number(1,20);
  if(IS_SET(ch->specials.pflag, PLR_QUEST))
    sprintf(arg,"$n's %s rolls a %d. (%s)",OBJ_SHORT(obj),num,prizes[num-1]);
  else
    sprintf(arg,"$n's %s rolls a %d.",OBJ_SHORT(obj),num);
  act(arg,0,ch,0,0,TO_ROOM);

  if(IS_SET(ch->specials.pflag, PLR_QUEST))
    sprintf(arg,"Your %s rolls a %d. (%s)",OBJ_SHORT(obj),num,prizes[num-1]);
  else
    sprintf(arg,"Your %s rolls a %d.",OBJ_SHORT(obj),num);
  act(arg,0,ch,0,0,TO_CHAR);

  if(IS_SET(ch->specials.pflag, PLR_QUEST)) {
    sprintf(arg,"PLRINFO: %s's %s rolls a %d (%s)",GET_NAME(ch),OBJ_SHORT(obj),num,prizes[num-1]);
    wizlog(arg,LEVEL_IMM,4);
  }
  return TRUE;
}

int p_ring(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  if(cmd!=MSG_TICK) return FALSE;
  if(obj->in_room==NOWHERE) return FALSE;

  if(world[obj->in_room].people)
    send_to_room("The Ring of Jal-Pur fades away into nothingness.\n\r",obj->in_room);
  extract_obj(obj);
  return FALSE;
}

#define ZONE_PAGODA   26400
#define ZONE_PAGODAII 26500
#define PAGODA_PROTECTOR   ITEM(ZONE_PAGODA,   75)
#define PAGODA_ADEPT       ITEM(ZONE_PAGODA,   76)
#define PAGODA_DEVOTEE1    ITEM(ZONE_PAGODA,   77)
#define PAGODA_DISCIPLE    ITEM(ZONE_PAGODA,   80)
#define PAGODA_HEALER      ITEM(ZONE_PAGODA,   81)
#define PAGODA_MAGUS       ITEM(ZONE_PAGODA,   82)
#define PAGODA_GURU        ITEM(ZONE_PAGODAII, 83)
#define PAGODA_TYRANT      ITEM(ZONE_PAGODA,    0)
#define PAGODA_EMIR        ITEM(ZONE_PAGODA,    2)
#define PAGODA_WAND        ITEM(ZONE_PAGODA,    2)
#define PAGODA_SPHERE      ITEM(ZONE_PAGODA,    4)
#define PAGODA_CLOAK       ITEM(ZONE_PAGODA,    6)
#define PAGODA_DICE        ITEM(ZONE_PAGODA,   11)
#define PAGODA_RING        ITEM(ZONE_PAGODA,    1)

void assign_pagoda (void) {
  assign_mob(PAGODA_PROTECTOR,   fanatic);
  assign_mob(PAGODA_ADEPT,       fanatic);
  assign_mob(PAGODA_ADEPT,       fanatic);
  assign_mob(PAGODA_DEVOTEE1,    fanatic);
  assign_mob(PAGODA_DISCIPLE,    fanatic);
  assign_mob(PAGODA_HEALER,      fanatic);
  assign_mob(PAGODA_MAGUS,       fanatic);
  assign_mob(PAGODA_GURU,        fanatic);
  assign_mob(PAGODA_TYRANT,      p_tyrant);
  assign_mob(PAGODA_EMIR,        p_emir);
  assign_obj(PAGODA_SPHERE,      p_sphere);
  assign_obj(PAGODA_WAND,        p_wand);
  assign_obj(PAGODA_DICE,        p_dice);
  assign_obj(PAGODA_RING,        p_ring);
}

