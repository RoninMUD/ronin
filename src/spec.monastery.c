/*
///   Special Procedure Module                   Orig. Date    15-02-1994
///                                              Last Modified 20-04-1996
///
///   spec.monastery.c --- Specials for The Old Monastery
///
///   Designed and coded by Kafka of (kafka@modeemi.cs.tut.fi)
///   Using this code is not allowed without permission from originator.
///
///   Modified March 22, 1998 - Overhaul of checks for what the mobs
///   trigger on. Previously added a check for mounts on the pit. Ranger
*/

/*
$Author: ronin $
$Date: 2005/01/21 14:55:29 $
$Header: /home/ronin/cvs/ronin/spec.monastery.c,v 2.1 2005/01/21 14:55:29 ronin Exp $
$Id: spec.monastery.c,v 2.1 2005/01/21 14:55:29 ronin Exp $
$Name:  $
$Log: spec.monastery.c,v $
Revision 2.1  2005/01/21 14:55:29  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.0.0.1  2004/02/05 16:11:04  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "cmd.h"
#include "fight.h"
#include "spec_assign.h"

void stop_fighting (CHAR *);
void hit (CHAR *, CHAR *, int);
char *one_argument(char *, char *);

#define MONK_ZONE      12900

#define IN_THE_RING    ITEM(MONK_ZONE,50)

#define RING           ITEM(MONK_ZONE,5)
#define SHARD          ITEM(MONK_ZONE,6)
#define FUNG_TAI       ITEM(MONK_ZONE,1)

#define DAEMON         ITEM(MONK_ZONE,10)
#define GREAT          ITEM(MONK_ZONE,5)
#define WIND           ITEM(MONK_ZONE,2)
#define ULTIMATE       ITEM(MONK_ZONE,4)
#define HAZE           ITEM(MONK_ZONE,8)

#define CRYSTAL_ROOM   ITEM(MONK_ZONE,50)
#define PIT_1          ITEM(MONK_ZONE,0)
#define PIT_2          ITEM(MONK_ZONE,1)
#define PIT_3          ITEM(MONK_ZONE,2)

#define CRYSTAL_RING   ITEM(MONK_ZONE,5)
#define SHARD          ITEM(MONK_ZONE,6)

#define MAIDEN         ITEM(MONK_ZONE,7)

void stun (CHAR *ch, CHAR *vict) {
  act ("$n stuns you with a mighty blow on the head.",
       FALSE, ch, 0, vict, TO_VICT);
  act ("$n stuns $N with a mighty blow on $S head.",
       FALSE, ch, 0, vict, TO_NOTVICT);
  act ("You hit $N with a stunning blow. $E will be out for a while.",
       FALSE, ch, 0, vict, TO_CHAR);

  GET_POS(vict) = POSITION_STUNNED;

  if (GET_OPPONENT(vict))
    stop_fighting (vict);
  if (GET_OPPONENT(ch) && GET_OPPONENT(ch) == vict)
    stop_fighting (ch);

  WAIT_STATE (vict, 3*PULSE_VIOLENCE);
}

void vanish (CHAR *ch) {
  CHAR *t;

  act ("$n vanishes into thin air from right before your eyes.",
       FALSE, ch, 0, 0, TO_ROOM);
  send_to_char ("You hide yourself from the eyes of the attackers.\n", ch);
  stop_fighting (ch);

  for (t = world[CHAR_REAL_ROOM(ch)].people; t; t = t->next_in_room) {
    if (GET_OPPONENT(t) && GET_OPPONENT(t) == ch) {
      stop_fighting (t);
      send_to_char ("You can't fight someone you can't see.\n", t);
    }
  }

  if (!(t = get_random_victim (ch)))
    return;

  act ("$n appears just next to you. $e attacks you with blind fury.",
       FALSE, ch, 0, t, TO_VICT);
  act ("$n appears in the midst of you and attacks $N with blind fury.",
       FALSE, ch, 0, t, TO_NOTVICT);
  act ("You appear just next to the unexpecting victims.",
       FALSE, ch, 0, t, TO_CHAR);
  hit (ch, t, TYPE_UNDEFINED);
}

int Ultimate (CHAR *ul, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  static char *actions [] = {
    "$n changes his stance to another yoga position.",
    "$n writes something in the dust on the floor in front of him.",
    "$n seems to have reached enlightment, but soon sighs in vain.",
    "$n tortures himself with endless physical and mental excercises.",
    "$n bids you to join him in meditation.",
    "$n becons you closer and whispers 'The Pit is dangerous for you.'",
    "$n looks south for a long time and shivers frightfully."
  };

  if (cmd == CMD_NORTH) {
    act ("$n pushes you back as you try to walk north.",
	 FALSE, ul, 0, ch, TO_VICT);
    act ("$n pushes $N back as $E tries to walk north.",
	 FALSE, ul, 0, ch, TO_NOTVICT);
    act ("You push $N back as $E tries to walk north.",
	 FALSE, ul, 0, ch, TO_CHAR);

    return TRUE;
  }

  /* MOBACT behaviour below */
  if (cmd!=MSG_MOBACT) return FALSE;

  if (!GET_OPPONENT(ul) && !number (0,7)) {
    act (actions[number (0,6)], FALSE, ul, 0, 0, TO_ROOM);
    return FALSE;
  }

  switch (number (1,9)) {
  case 1:
  case 2:
  case 3:
    act("$n touches himself gently and heals some of $s wounds.",
	FALSE, ul, 0, 0, TO_ROOM);
    send_to_char ("You touch yourself with a healing hand.\n", ul);
    GET_HIT(ul) += 500;
    return FALSE;

  case 4:
    vanish (ul);
    break;

  case 5:
  case 6:
    if (!(vict = get_random_victim (ul)))
      return FALSE;
    stun (ul, vict);
    break;
  default:
    break;
  }

  return FALSE;
}

int
Great (CHAR *gr, CHAR *ch, int cmd, char *arg) {
  CHAR *tch;
  CHAR *wind;
  int wind_nr;

  static char *work [] = {
    "$n turns to another page from the book $e is reading.",
    "$n wipes the sweat off of $s brow.",
    "$n sighs deeply and shakes $s head.",
    "$n writes some notes about the text."
  };

  wind_nr = real_mobile (WIND);

  if (cmd!=MSG_MOBACT) return FALSE;

  if ((tch = GET_OPPONENT(gr)) && mob_proto_table[wind_nr].number < 2) {
    wind = read_mobile (wind_nr, REAL);
    char_to_room (wind, CHAR_REAL_ROOM(gr));

    act ("$n frowns at you and makes a few strange gestures.",
	FALSE, gr, 0, tch, TO_VICT);
    act ("$n frowns at $N and makes a few strange gestures.",
	FALSE, gr, 0, tch, TO_NOTVICT);
    act ("You frown at $N and summon $N to aid you in the battle.",
	FALSE, gr, 0, wind, TO_CHAR);
    act ("$n appears in a loud clap of thunder.", FALSE, wind, 0, 0, TO_ROOM);
    hit (wind, tch, TYPE_UNDEFINED);

    return FALSE;
  }

  if (!number (0,9))
    act (work[number (0,3)], FALSE, gr, 0, 0, TO_ROOM);

  return FALSE;
}

int
Maiden (CHAR *maiden, CHAR *ch, int cmd, char *arg) {
  CHAR *haze;
  int haze_nr;

  haze_nr = real_mobile (HAZE);

  if (cmd)
    return FALSE;

  if (GET_OPPONENT(maiden) && !get_ch_room (HAZE, CHAR_REAL_ROOM(maiden))) {
    haze = read_mobile (haze_nr, REAL);
    char_to_room (haze, CHAR_REAL_ROOM(maiden));
    act ("$n appears in a blinding flash of lightning to protect $N.",
	FALSE, haze, 0, maiden, TO_ROOM);
    hit (haze, GET_OPPONENT(maiden), TYPE_UNDEFINED);
    return FALSE;
  }

  return FALSE;
}

int
CrystalRoom (int room, CHAR *ch, int cmd, char *arg) {
  OBJ *ring;
  int target = -1;
  int crystal;
  void do_look (CHAR *, char *, int);

  if(!ch)
    return FALSE;

  crystal = real_object(RING);

  /* For resetting the room */
  if (cmd == CMD_ROOMLOCK && GET_LEVEL(ch) >= LEVEL_WIZ) {
    ROOM_SPEC(room) = 0;
    return TRUE;
  }

  if(!IS_MORTAL(ch))
    return FALSE;

  if (count_mortals_room(ch,TRUE) == 0) {
    ROOM_SPEC(room) = 0;
    return FALSE;
  }

  if(get_ch_room(DAEMON, room))
    return FALSE;

  if (number (0,6))
    return FALSE;

  if ((ring = get_obj_num(crystal)))
    {
      if (ring->carried_by)
	target = CHAR_REAL_ROOM(ring->carried_by);
      else
	if (ring->equipped_by)
	  target = CHAR_REAL_ROOM(ring->equipped_by);
	else
	  if (ring->in_room)
	    target = ring->in_room;
    }

  if (target == -1)
    return FALSE;

  ROOM_SPEC(room) = 0;
  send_to_char("You are freed from your crystalline prison.\n\r",ch);
  char_from_room(ch);
  char_to_room(ch, target);
  act ("$n appears from $p.",TRUE,ch,ring,0,TO_ROOM);
  do_look(ch, "", 0);
  return FALSE;
}

int
CrystalRing (OBJ *ring, CHAR *ch, int cmd, char *arg) {
  int found = 0;
  int num = 0;
  int crystal_room = 0;
  struct char_data *v,*daemon=0;
  struct affected_type_5 af;
  void do_look (CHAR *, char *, int);

  if(!ch)
    return(FALSE);

  if(cmd==MSG_OBJ_ENTERING_GAME) return FALSE;

  crystal_room = real_room(IN_THE_RING);

  for(v=world[real_room(IN_THE_RING)].people;v;v=v->next_in_room) {
    if(IS_NPC(v) && V_MOB(v)==DAEMON) daemon=v;
    if (IS_MORTAL(v)) {
      num++;
    }
  }

  if (num > 0) {
    return FALSE;
  }

  if (ROOM_SPEC(crystal_room))
    return FALSE;

  if (ring == EQ(ch, WEAR_FINGER_R))
    found = WEAR_FINGER_R;
  else
    if (ring == EQ(ch, WEAR_FINGER_L))
      found = WEAR_FINGER_L;
    else
      return FALSE;

  if (GET_OPPONENT(ch))
    return FALSE;

  if (!number(0,79)) {
    act("$p in your finger suddenly grows larger and is sucking you in.",
	FALSE,ch,ring,0,TO_CHAR);
    act("$n starts to shrink and falls in to $p.",FALSE,ch,ring,0,TO_ROOM);
    if(daemon) GET_HIT(daemon)=GET_MAX_HIT(daemon);

    unequip_char (ch, found);
    obj_to_room (ring,CHAR_REAL_ROOM(ch));
    ROOM_SPEC(crystal_room) = 1;
    char_from_room (ch);
    char_to_room (ch ,crystal_room);

    af.type = SPELL_INVISIBLE;
    af.duration = 5;
    af.bitvector = AFF_IMINV;
    af.bitvector2 = 0;
    af.location = APPLY_NONE;
    af.modifier = 0;
    affect_to_char (ch, &af);
    do_look(ch, "", CMD_LOOK);
    return TRUE;
  }
  return FALSE;
}

int BottomlessPit (int room, CHAR *ch, int cmd, char *arg) {
  OBJ *tobj, *next;
  void do_look(CHAR *, char *, int);
  int pit1, pit2, pit3;
  char buf[1000];
  if(!ch)
    return(FALSE);

  pit1 = real_room(ITEM(MONK_ZONE,1));
  pit2 = real_room(ITEM(MONK_ZONE,2));
  pit3 = real_room(ITEM(MONK_ZONE,3));

  for (tobj = world[room].contents; tobj; tobj = next) {
    next = tobj->next_content;
    act("$p falls deeper into the pit, past you.",FALSE,ch,tobj,0,TO_ROOM);
    act("$p falls deeper into the pit, past you.",FALSE,ch,tobj,0,TO_CHAR);
    extract_obj(tobj);
  }

  if (!IS_MORTAL(ch)) return FALSE;

  if (IS_AFFECTED(ch, AFF_FLY) || number(1,20) < GET_DEX(ch))
    return FALSE;

  if(ch->specials.riding) {
    if(IS_AFFECTED(ch->specials.riding,AFF_FLY)) return FALSE;
  }

  if (cmd == CMD_NORTH || cmd == CMD_EAST || cmd == CMD_SOUTH ||
      cmd == CMD_WEST  || cmd == CMD_DOWN || cmd == CMD_UP) {
    act("$n screams loudly as $s hands come off the rope, and $e falls.",
	FALSE,ch,0,0,TO_ROOM);
    send_to_char("Your can't hold on to the rope, and you fall down...\n",ch);

    switch (V_ROOM(ch)) {
    case ITEM(MONK_ZONE,0):
      char_from_room(ch);
      char_to_room(ch, pit1);
      do_look(ch,"",0);
      act("$n screams loudly as $e falls down, but manages to grab a hold.",
	  FALSE,ch,0,0,TO_ROOM);
      send_to_char("You manage to grab on to one of the ropes...\n",ch);
      break;
    case ITEM(MONK_ZONE,1):
      char_from_room(ch);
      char_to_room(ch, pit2);
      do_look(ch,"",0);
      act("$n screams loudly as $e falls down, but manages to grab a hold.",
	  FALSE,ch,0,0,TO_ROOM);
      send_to_char("You manage to grab on to one of the ropes...\n",ch);
      break;
    case ITEM(MONK_ZONE,2):
      char_from_room(ch);
      char_to_room(ch, pit3);
      do_look(ch,"",0);
      send_to_char("You fall, fall and fall...and your spirit is dislodged\n",
		   ch);
      send_to_char("from your body... It travels back to the world of men.\n",
		   ch);

      sprintf (buf, "%s hit a DT in %s (%d).",
               GET_NAME(ch), world[CHAR_REAL_ROOM(ch)].name,
               CHAR_VIRTUAL_ROOM(ch));
      wizlog(buf, LEVEL_WIZ, 4);/* wizlog and save_char - Sumo Jan98 */
      log_f(buf);
      raw_kill(ch);
      break;
    }
    return TRUE;
  }
  return FALSE;
}

/*
 *************************************************************************
 Like a wand of wonder, the shard will use one of its four powers randomly
 *************************************************************************
 */

#ifndef MN_SHARD_MAX_POWER
#define MN_SHARD_MAX_POWER   30
#endif

int
Vulcanite (OBJ *shard, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int set, power[] = {7,7,5,20};

  if (!shard) return FALSE;

  if (cmd == MSG_TICK)
  {
    if ((OBJ_SPEC(shard) > 0) && chance(75))
    {
      OBJ_SPEC(shard) -= 1;
    }
  }

  if (!IS_MORTAL(ch))
    return FALSE;

  if (!(shard == EQ(ch, HOLD)))
    return FALSE;

  if(!number(0,44) && OBJ_SPEC(shard) < MN_SHARD_MAX_POWER) {
    OBJ_SPEC(shard) += 1;
    act ("You can see $p pulse once, like gaining new life.",
	 FALSE,ch,shard,0,TO_ROOM);
    act ("You can see $p pulse once, like gaining new life.",
	 FALSE,ch,shard,0,TO_CHAR);
  }

  if (cmd == CMD_USE) {
    one_argument(arg, buf);
    if (!isname(buf, OBJ_NAME(shard)))
      return FALSE;

    set = number(0,3);
    if (OBJ_SPEC(shard) < power[set]) {
      act ("$p doesn't show any signs of having any power at all.",
	   FALSE,ch,shard,0,TO_CHAR);
      return TRUE;
    }

    act("$n waves $p in front of $m in a large circle.",
	FALSE,ch,shard,0,TO_ROOM);
    act("You wave $p in front of you in a large circle.",
	FALSE,ch,shard,0,TO_CHAR);
    act("$p pulses, and a strange feeling spreads all over you.",
	FALSE,ch,shard,0,TO_CHAR);

    OBJ_SPEC(shard) -= power[set];

    switch (set) {
    case 0:
      spell_heal(30,ch,ch,0);
      spell_heal(30,ch,ch,0);
      return TRUE;
    case 1:
      spell_sanctuary(30,ch,ch,0);
      spell_armor(30,ch,ch,0);
      return TRUE;
    case 2:
      spell_strength (30,ch,ch,0);
      spell_strength (30,ch,ch,0);
      return TRUE;
    case 3:
      spell_fury(30,ch,ch,0);
      return TRUE;
    }
  }
  return FALSE;
}

void
fung_sting(CHAR *ch, CHAR *vict, OBJ *dg, int w_align, int v_align, int pos)
{
  int dmg;
  if (!IS_NPC(vict)) return;

  if(!number(0,50000)) {
    if(dg->obj_flags.value[1]>1 && dg->obj_flags.value[2]>1) {
      act("$p seems to dull a little.",FALSE,ch,dg,0,TO_CHAR);
      if(number(0,1)) dg->obj_flags.value[1] = dg->obj_flags.value[1]-1;
      else  dg->obj_flags.value[2] = dg->obj_flags.value[2]-1;
      return;
    }
    if(dg->obj_flags.value[1]>1 && dg->obj_flags.value[2]==1) {
      act("$p seems to dull a little.",FALSE,ch,dg,0,TO_CHAR);
      dg->obj_flags.value[1] = dg->obj_flags.value[1]-1;
      return;
    }
    if(dg->obj_flags.value[1]==1 && dg->obj_flags.value[2]>1) {
      act("$p seems to dull a little.",FALSE,ch,dg,0,TO_CHAR);
      dg->obj_flags.value[2] = dg->obj_flags.value[2]-1;
      return;
    }
  }

  if(dg->obj_flags.value[1]==1 && dg->obj_flags.value[2]==1) dmg=0;
  else dmg=GET_LEVEL(ch)>>2;

  if (v_align > 350) {
      act("$p stings you viciously.",FALSE,ch,dg,vict,TO_VICT);
      act("$p stings $N viciously.",FALSE,ch,dg,vict,TO_NOTVICT);
      act("You sting $N with your sharp dagger.",FALSE,ch,dg,vict,TO_CHAR);
      if (GET_LEVEL(vict)<55) damage(ch,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
  } else if (v_align > -350) {
      act("$p stings you viciously.",FALSE,ch,dg,vict,TO_VICT);
      act("$p stings $N viciously.",FALSE,ch,dg,vict,TO_NOTVICT);
      act("You sting $N with your sharp dagger.",FALSE,ch,dg,vict,TO_CHAR);
      if (GET_LEVEL(vict)<55) damage(ch,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
  }
/*  unequip_char (ch, pos);
  dg->obj_flags.value[1] = 4;
  dg->obj_flags.value[2] = 7;
  equip_char (ch, dg, pos);*/
}

int
Dagger (OBJ *dg, CHAR *ch, int cmd, char *arg) {
  int w_align, v_align, found;
  CHAR *vict=NULL;
  char buf[MAX_INPUT_LENGTH];
  int dagger_nr = 0;
  OBJ *dg2 = NULL;
  int second_dagger = 0;

  if(!ch)
     return (FALSE);

  dagger_nr = real_object (FUNG_TAI);

  if (!IS_MORTAL(ch))
    return FALSE;

  if (dg == EQ(ch, WIELD) && dg->item_number == dagger_nr)
    found = WIELD;
  else if (dg == EQ(ch, HOLD) && dg->item_number == dagger_nr)
    found = HOLD;
  else return FALSE;

  switch (cmd) {
  case CMD_NORTH:
  case CMD_SOUTH:
  case CMD_EAST:
  case CMD_WEST:
  case CMD_UP:
  case CMD_DOWN:
       return FALSE;
  case CMD_FLEE:
    act ("$p prevents you from fleeing. It wants to fight.",
	 FALSE,ch,dg,0,TO_CHAR);
    return TRUE;
  case CMD_CONSIDER:
    send_to_char("Now where did that chicken go?\n\r", ch);
    return TRUE;
  case CMD_SLEEP:
    send_to_char("You can't get to sleep, something is bothering you.\n\r",ch);
    return TRUE;
  case CMD_REMOVE:
    one_argument (arg, buf);
    if (isname(buf, OBJ_NAME(dg))) {
      send_to_char("You can't do that! Something doesn't let you.\n\r",ch);
      return TRUE;
    }
  }

  /*
   * damage code modified by Shun 04/17/96 to prevent crashes under linux
   */

  if (cmd != CMD_USE || !(vict = GET_OPPONENT(ch)))
    return FALSE;

  one_argument(arg,buf);

  if(!isname(buf, OBJ_NAME(dg)))
    return FALSE;

  v_align = GET_ALIGNMENT(vict);
  w_align = GET_ALIGNMENT(ch);

  if (v_align < w_align && GET_LEVEL(vict) > GET_LEVEL(ch)) {
    switch (number (1,4)) {
    case 1 :
      act("$p seems dulled by the presence of $N.",FALSE,ch,dg,vict,TO_CHAR);
      unequip_char (ch, found);
      dg->obj_flags.value[1] = 1;
      dg->obj_flags.value[2] = 1;
      equip_char (ch, dg, found);
      return TRUE;

    case 2 :
      act("$p drops strangely from your grasp.",FALSE,ch,dg,0,TO_CHAR);
      act("$p drops strangely from $n's graps.",FALSE,ch,dg,0,TO_ROOM);
      obj_to_room(unequip_char(ch,found), CHAR_REAL_ROOM(ch));
      return TRUE;

    case 3 :
      act("$p bursts into bright flames for a moment.",
	  FALSE,ch,dg,0,TO_CHAR);
      act("$p bursts into bright flames for a moment.",
	  FALSE,ch,dg,0,TO_ROOM);
      spell_blindness(30,ch,ch,0);
      return TRUE;

    case 4 :
      act("$p shocks you with a tingling jolt of lightning.",
	  FALSE,ch,dg,0,TO_CHAR);
      act("$p shocks $n with a tingling jolt of lightning.",
	  FALSE,ch,dg,0,TO_ROOM);
      damage(ch,ch,50,TYPE_UNDEFINED,DAM_ELECTRIC);
      GET_POS(ch) = POSITION_STUNNED;
      return TRUE;
    }
  }

  if(found == WIELD) {
    dg2 = EQ(ch, HOLD);
    if(!dg2)
	second_dagger = 0;
    else if(dg2->item_number == dagger_nr)
	second_dagger = 1;
  }

  fung_sting(ch, vict, dg, w_align, v_align, found);

  /* Gotta make sure first fung didn't kill the mob - Ranger */
  vict=NULL;
  if(second_dagger && dg2) {
     vict = GET_OPPONENT(ch);
     if (vict) fung_sting(ch, vict, dg2, w_align, v_align, HOLD);
  }
  return TRUE;
}

int daemon_spec(CHAR *daemon, CHAR *ch, int cmd, char *arg) {
  CHAR *tel_ch,*tel_char_next;
  OBJ *tel_obj,*tel_obj_next;

  if(cmd==MSG_DIE) {
    act("A flash of light consumes $n as he expands into reality!",TRUE,daemon,0,0,TO_ROOM);
    act("A flash of light consumes you as you expand into reality!",TRUE,daemon,0,0,TO_CHAR);
    for (tel_ch = world[real_room(12950)].people;tel_ch;tel_ch =tel_char_next) {
      tel_char_next = tel_ch->next_in_room;
      char_from_room (tel_ch);
      char_to_room(tel_ch, real_room(3014));
    }

    for (tel_obj = world[real_room(12950)].contents;tel_obj;tel_obj = tel_obj_next) {
      tel_obj_next = tel_obj->next_content;
      obj_from_room (tel_obj);
      obj_to_room (tel_obj, real_room(3014));
    }
  }
  return FALSE;
}

 /**********************************************************************\
 |* End Of the Special procedures for Monastery                        *|
 \**********************************************************************/
void assign_monastery (void) {
  assign_room(CRYSTAL_ROOM, CrystalRoom);
  assign_room(PIT_1, BottomlessPit);
  assign_room(PIT_2, BottomlessPit);
  assign_room(PIT_3, BottomlessPit);

  assign_mob(ULTIMATE, Ultimate);
  assign_mob(DAEMON,daemon_spec);
  assign_mob(GREAT, Great);
  assign_mob(MAIDEN, Maiden);

  assign_obj(CRYSTAL_RING, CrystalRing);
/*  assign_obj(FUNG_TAI ,  Dagger); Commented by Ranger Sept 98 */
  assign_obj(SHARD ,  Vulcanite);

}
