
/* Specs for the Golden Plaza by Blink
**
** for use in RoninMUD
**
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:39 $
$Header: /home/ronin/cvs/ronin/spec.golden.c,v 2.0.0.1 2004/02/05 16:10:39 ronin Exp $
$Id: spec.golden.c,v 2.0.0.1 2004/02/05 16:10:39 ronin Exp $
$Name:  $
$Log: spec.golden.c,v $
Revision 2.0.0.1  2004/02/05 16:10:39  ronin
Reinitialization of cvs archives

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
#include "enchant.h"
#include "act.h"
#include "spec_assign.h"
#include "mob.spells.h"

int gp_yogi_defender(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd==CMD_SOUTH && GET_LEVEL(ch)<20) {
    act("$n says 'You cannot go past this point.'",0,mob,0,ch,TO_ROOM);
    act("You step in front of $N, blocking $S way.",0,mob,0,ch,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

int gp_ninja_guardian(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd==CMD_NORTH && GET_LEVEL(ch)<30) {
    act("$n says 'You must kill me to get by here.'",0,mob,0,ch,TO_ROOM);
    act("You step in front of $N, blocking $S way.",0,mob,0,ch,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

int gp_elite_bushi(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd==CMD_SOUTH && GET_LEVEL(ch)<40) {
    act("$n says 'You must get past me first!'",0,mob,0,ch,TO_ROOM);
    act("You step in front of $N, blocking $S way.",0,mob,0,ch,TO_CHAR);
    return TRUE;
 }
  return FALSE;
}

int gp_dragon_guardian(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd==CMD_NORTH && GET_LEVEL(ch)<51) {
    act("$n says 'You are not worthy enough to pass.'",0,mob,0,ch,TO_ROOM);
    act("You step in front of $N, blocking $S way.",0,mob,0,ch,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

int gp_master_pagoda(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(10)) {
    act("The Master of the Pagoda summons a storm of raining sleet!",0,mob,0,0,TO_ROOM);
    act("You summon a storm of raining sleet!",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      damage(mob,vict,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  if(chance(10)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("The Master of the Pagoda headbutts $N!",0,mob,0,vict,TO_NOTVICT);
    act("You headbutt $N!",0,mob,0,vict,TO_CHAR);
    act("$N slammed with a huge headbutt from the Master of the Pagoda.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  return FALSE;
}

int gp_chun_lui(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(5)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Chun Lui summons a magical ninja star and hurls it at $N!",0,mob,0,vict,TO_NOTVICT);
    act("You throw a magical star at $N.",0,mob,0,vict,TO_CHAR);
    act("You are sliced by one of Chun Lui's magical ninja stars!",0,mob,0,vict,TO_VICT);
    damage(mob,vict,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(10)) {
    act("Chun Lui emits a tremendous wave of magical power.",0,mob,0,0,TO_ROOM);
    act("You emit a wave of your power!",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      act("You are knocked back by Chun Lui's magical might!",0,mob,0,vict,TO_VICT);
      damage(mob,vict,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  if(chance(50)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    if(GET_CLASS(vict)!=CLASS_MAGIC_USER &&
       GET_CLASS(vict)!=CLASS_THIEF &&
       GET_CLASS(vict)!=CLASS_COMMANDO) return FALSE;
    act("Chun Lui slams $N with his mallet.",0,mob,0,vict,TO_NOTVICT);
    act("You slam $N with your mallet!",0,mob,0,vict,TO_CHAR);
    act("Chun Lui slams you with his mallet!",0,mob,0,vict,TO_VICT);
    damage(mob,vict,550,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  return FALSE;
}

int gp_rhoden_clandba(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;
  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(50)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    if(GET_CLASS(vict)!=CLASS_BARD &&
       GET_CLASS(vict)!=CLASS_CLERIC) return FALSE;
    act("Rhoden summons a huge fireball, and hurls it at $N!",0,mob,0,vict,TO_NOTVICT);
    act("You hurl a huge fireball at $N.",0,mob,0,vict,TO_CHAR);
    act("Rhoden hurls a huge fireball at you.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(10))   {
    act("Rhoden summons a magical fire, scorching the room!",0,mob,0,0,TO_ROOM);
    act("You summon a magical fire, scorching the room.",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      act("Your flesh begins to peel due to Rhodens deadly spell!",0,mob,0,vict,TO_VICT);
      damage(mob,vict,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }
  return FALSE;
}

int gp_lotus(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(GET_HIT(mob)<2000 && CHAR_VIRTUAL_ROOM(mob)!=11770) {
    act("$n laughs manically, turns around and disappears!",0,mob,0,0,TO_ROOM);
    act("You laugh manically, turn around and disappear!",0,mob,0,0,TO_CHAR);
    stop_fighting(mob);
    char_from_room(mob);
    char_to_room(mob,real_room(11770));
    return FALSE;
  }

  if(chance(20)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Lotus lands a roundhouse kick to the side of $N's head.",0,mob,0,vict,TO_NOTVICT);
    act("You roundhouse kick $N.",0,mob,0,vict,TO_CHAR);
    act("Lotus lands a roundhouse kick to the side of your head.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,800,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(20)) {
    act("Lotus expunges a chokingly thick gas!",0,mob,0,0,TO_ROOM);
    act("You expunge some BAD gas ;)!",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      act("You begin to choke as you inhale the deadly gas.",0,mob,0,vict,TO_VICT);
      damage(mob,vict,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  if(chance(75)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    if(GET_CLASS(vict)!=CLASS_PALADIN &&
       GET_CLASS(vict)!=CLASS_NINJA) return FALSE;
    act("Lotus slips behind $N and slices $S throat.",0,mob,0,vict,TO_NOTVICT);
    act("You slit $N's throat.",0,mob,0,vict,TO_CHAR);
    act("Lotus disappears for a moment.  You feel something slit your throat.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,10000,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }
  return FALSE;
}

/*When Lotus mob 11700 teleports, she drops obj 11708.
Then you need to take this object to room 11708 and drop it,
as well as drop obj 11709, then these two objects will
disappear and obj 11710 will be left there.
*/

/*when item #11710 is held and used in room# 11706 it will teleport
everyone in there to room #11765*/


int gp_mallet(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  char buf[MAX_INPUT_LENGTH];
  CHAR *tel_ch,*tel_char_next;

  if(!ch) return FALSE;
  if(cmd!=CMD_USE) return FALSE;
  if(obj->equipped_by!=ch) return FALSE;
  one_argument(argument, buf);
  if(!isname(buf,OBJ_NAME(obj))) return FALSE;
  if(ch!=obj->equipped_by) return FALSE;
  if(CHAR_VIRTUAL_ROOM(ch)!=11722) return FALSE;

  act("$n takes the mallet and hits the gong.  The gong echos throughout the room.",0,ch,0,0,TO_ROOM);
  act("You take the mallet and hit the gong.  The gong echos throughout the room.",0,ch,0,0,TO_CHAR);

  for(tel_ch = world[real_room(11722)].people;tel_ch;tel_ch = tel_char_next) {
    tel_char_next = tel_ch->next_in_room;
    act("When you're ears stop ringing - you notice you aren't where you use to be.\n\r",0,tel_ch,0,0,TO_CHAR);
    char_from_room(tel_ch);
    char_to_room(tel_ch, real_room(11733));
    do_look(tel_ch,"",CMD_LOOK);
  }

  send_to_room("The mallet slowly turns into a fine black powder and disperses.\n\r",CHAR_REAL_ROOM(ch));
  extract_obj(obj);
  return TRUE;
}

#define PLAZA_TELEPORT_FROM 11706
#define PLAZA_TELEPORT_TO   11765

int gp_mirror(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  char buf[MAX_INPUT_LENGTH];
  CHAR *tel_ch,*tel_char_next;

  if(!ch) return FALSE;
  if(cmd!=CMD_USE) return FALSE;
  if(obj->equipped_by!=ch) return FALSE;
  one_argument(argument, buf);
  if(!isname(buf,OBJ_NAME(obj))) return FALSE;
  if(ch!=obj->equipped_by) return FALSE;
  if(CHAR_VIRTUAL_ROOM(ch)!=PLAZA_TELEPORT_FROM) return FALSE;

  send_to_room("The golden light reflects off the mirror and tears a magical rift in mid air.\n\r",CHAR_REAL_ROOM(ch));

  for(tel_ch = world[real_room(PLAZA_TELEPORT_FROM)].people;tel_ch;tel_ch = tel_char_next) {
    tel_char_next = tel_ch->next_in_room;
    act("You slowly are sucked into the magical rift.\n\r",TRUE,tel_ch,0,0,TO_CHAR);
    char_from_room(tel_ch);
    char_to_room(tel_ch, real_room(PLAZA_TELEPORT_TO));
    do_look(tel_ch,"",CMD_LOOK);
  }

  send_to_room("The mirror slowly turns into a fine black powder and disperses.\n\r",CHAR_REAL_ROOM(ch));
  extract_obj(obj);
  return TRUE;
}

/*
11706 11713 11716 11714 11715 11717 11718
decay in 45 min
*/

int gp_decay_gear(OBJ *obj, CHAR *ch, int cmd, char *argument) {

  if(cmd!=MSG_TICK) return FALSE;

  if(OBJ_SPEC(obj)==0) {
    if(obj->carried_by)
      if(!IS_NPC(obj->carried_by)) OBJ_SPEC(obj)=45;
    if(obj->equipped_by)
      if(!IS_NPC(obj->equipped_by)) OBJ_SPEC(obj)=45;
    return FALSE;
  }

  if(OBJ_SPEC(obj)>0) {
    OBJ_SPEC(obj)--;
    if(OBJ_SPEC(obj)==0) OBJ_SPEC(obj)--; /* -1 it disintegrates */
    if(obj->equipped_by)
      act("Your $p weakens slightly.",0,obj->equipped_by,obj,0,TO_CHAR);
    if(obj->carried_by)
      act("Your $p weakens slightly.",0,obj->carried_by,obj,0,TO_CHAR);
    return FALSE;
  }

  if(OBJ_SPEC(obj)<0) {
    if(obj->equipped_by)
      act("Your $p turns into a fine black powder and disappears.",0,obj->equipped_by,obj,0,TO_CHAR);
    if(obj->carried_by)
      act("Your $p turns into a fine black powder and disappears.",0,obj->carried_by,obj,0,TO_CHAR);
    extract_obj(obj);
    return FALSE;
  }
  return FALSE;
}

#define MIRROR_ROOM 11708

int gp_mirror_make(OBJ *obj ,CHAR *ch, int cmd, char *argument) {
  int mirror=FALSE, flask=FALSE;
  OBJ *obj2 = NULL;
  char buf[MAX_INPUT_LENGTH];

  if (cmd != CMD_DROP) return FALSE;  // Check for the Drop Command First.  If not, abort.
  
  if(!ch) return FALSE;
  if(CHAR_REAL_ROOM(ch) == NOWHERE ) return FALSE; // Validate that the CH isnt in NOWHERE, which will cause the next command to crash.
  if(V_ROOM(ch) !=MIRROR_ROOM) return FALSE;

  if(obj->carried_by!=ch) return FALSE;
  one_argument (argument, buf);
  if (!isname(buf,OBJ_NAME(obj))) return FALSE;

  if(V_OBJ(obj) == 11708) {
    mirror=TRUE;
    if((obj2=get_obj_room(11709,MIRROR_ROOM))) flask=TRUE;
  }

  if(V_OBJ(obj) == 11709) {
    flask=TRUE;
    if((obj2=get_obj_room(11708,MIRROR_ROOM))) mirror=TRUE;
  }

  if(mirror && flask) {
    act("$n drops $p.", 1, ch, obj, 0, TO_ROOM);
    act("You drop $p.", 1, ch, obj, 0, TO_CHAR);
    extract_obj(obj);
    act("The flask of tears and the mirror magically fuse together\n\rleaving a dragon-like mirror in its place.",1,ch,0,0,TO_ROOM);
    act("The flask of tears and the mirror magically fuse together\n\rleaving a dragon-like mirror in its place.",1,ch,0,0,TO_CHAR);
    if (obj2) extract_obj(obj2);
    obj2=read_object(11710,VIRTUAL);
    obj_to_room(obj2,real_room(MIRROR_ROOM));
    return TRUE;
  }
  return FALSE;
}

void assign_golden_plaza(void) {
  assign_mob(11705,gp_yogi_defender);
  assign_mob(11706,gp_ninja_guardian);
  assign_mob(11708,gp_elite_bushi);
  assign_mob(11710,gp_dragon_guardian);
  assign_mob(11712,gp_master_pagoda);
  assign_mob(11702,gp_chun_lui);
  assign_mob(11701,gp_rhoden_clandba);
  assign_mob(11700,gp_lotus);
  assign_obj(11710,gp_mirror);
  assign_obj(11704,gp_mallet);
  assign_obj(11708,gp_mirror_make);
  assign_obj(11709,gp_mirror_make);
  assign_obj(11706,gp_decay_gear);
  assign_obj(11713,gp_decay_gear);
  assign_obj(11716,gp_decay_gear);
  assign_obj(11714,gp_decay_gear);
  assign_obj(11715,gp_decay_gear);
  assign_obj(11717,gp_decay_gear);
  assign_obj(11718,gp_decay_gear);
}


