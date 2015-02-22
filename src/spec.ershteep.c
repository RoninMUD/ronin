/*   spec.ershteep.c - Specs for Ershteep Revamp by Jukebox

     Written by Hemp for RoninMUD
     Last Modification Date: July 19, 2009

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

extern struct time_info_data time_info;

/* Zone */

#define ENFAN_ZONE			100

/* Rooms */

#define CASINO_ROOM			10000
#define RESTAURANT_ROOM		10035
#define BAR_ROOM			10036
#define ORGAN_ROOM			10045
#define ATTIC_ROOM			10049
#define HIDDEN_STAIRS_ROOM	10050
#define COTTAGE_ROOM		10052

/* Mobs */

#define ENFAN_CITIZEN		10001
#define FEMALE_CITIZEN		10002
#define JARVE				10005
#define ENFAN_CHILD			10007
#define PRIEST				10009
#define FYONA				10010
#define GHOST				10012
#define DARK_PRIEST			10013
#define ELAGO				10014
#define MOUSE				10015
#define CALCIFER			10017
#define DEALER				10021

/* Objects */

#define ORGAN				10012
#define WEDDING_BAND		10013
#define KEY					10016
#define GLASS_CIRCLET       10027

/* ======================================================================== */
/* ===============================OBJECT SPECS============================= */
/* ======================================================================== */

/* 'play organ' reveals exit to north */
int ersh_organ(OBJ *organ, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];
  bool bReturn = FALSE;

  switch(cmd)
  {
	case MSG_ZONE_RESET:
      if(world[real_room(ORGAN_ROOM)].dir_option[NORTH]->to_room_r >= 0)
      {/* remove exits */
        world[real_room(ORGAN_ROOM)].dir_option[NORTH]->to_room_r = -1;
        world[real_room(HIDDEN_STAIRS_ROOM)].dir_option[SOUTH]->to_room_r = -1;
        send_to_room("The opening in the stone wall seals itself closed once again.\n\r", real_room(HIDDEN_STAIRS_ROOM));
        send_to_room("The opening in the stone wall seals itself closed once again.\n\r", real_room(ORGAN_ROOM));
	  }
	  break;
	case CMD_UNKNOWN:
	  arg = one_argument(arg, buf);
	  if(V_ROOM(ch)==ORGAN_ROOM && AWAKE(ch) && !strncmp(buf, "play", MAX_INPUT_LENGTH))
	  {
		one_argument(arg, buf);
        if(!*buf) send_to_char("What do you want to play?\n\r",ch);
        else if(strncmp(buf, "organ", MAX_INPUT_LENGTH)) send_to_char("You can't play that.\n\r",ch);
        else if(world[real_room(ORGAN_ROOM)].dir_option[NORTH]->to_room_r > 0)
        {/* if exit exists already */
          act("$n pounds on the keys of the organ but nothing happens.",0,ch,0,0,TO_ROOM);
          act("You gracelessly pound on the keys of the organ but nothing happens.",0,ch,0,0,TO_CHAR);
        } else {/* if exit does not exist already */
          act("$n plays a dissonant chord on the organ.",0,ch,0,0,TO_ROOM);
          act("You play a dissonant chord on the organ.",0,ch,0,0,TO_CHAR);
          send_to_room("Suddenly a section of the stone wall slides sideways, revealing an exit.\n\r", real_room(ORGAN_ROOM));
          send_to_room("Suddenly a section of the stone wall slides sideways, revealing an exit.\n\r", real_room(HIDDEN_STAIRS_ROOM));
          world[real_room(ORGAN_ROOM)].dir_option[NORTH]->to_room_r = real_room(HIDDEN_STAIRS_ROOM);
          world[real_room(HIDDEN_STAIRS_ROOM)].dir_option[SOUTH]->to_room_r = real_room(ORGAN_ROOM);
	    }
	    bReturn = TRUE;
	  }
	  break;
	default:
	  break;
  }
  return bReturn;
}

/* heals some HP */
int ersh_wedding_band(OBJ *band, CHAR *ch, int cmd, char *arg)
{
  ch=band->equipped_by;

  if(ch && cmd==MSG_TICK && !IS_NPC(ch) && (band==EQ(ch,WEAR_FINGER_L) || band==EQ(ch,WEAR_FINGER_R)) && chance(7))
  {
    act("$n's gold wedding band shines brightly for a moment.",0,ch,0,0,TO_ROOM);
    act("Your gold wedding band shines brightly for a moment.",0,ch,0,0,TO_CHAR);
    spell_heal(GET_LEVEL(ch), ch, ch, 0);
  }
  return FALSE;
}


/* key shatter */
int ersh_key(OBJ *key, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];
  bool bReturn = FALSE;
  ch=key->carried_by;

  if(ch && cmd==CMD_UNLOCK && V_ROOM(ch)==COTTAGE_ROOM)
  {
	one_argument(arg,buf);
	if(*buf && !strncmp(buf, "grate", MAX_INPUT_LENGTH))
	{
      do_unlock(ch, buf, 0);
      act("The skeleton key in $n's hand shatters as $e unlocks the grate.",0,ch,0,0,TO_ROOM);
      act("The skeleton key shatters in your hand as you unlock the grate.",0,ch,0,0,TO_CHAR);
      extract_obj(key);
      bReturn = TRUE;
    }
  }
  return bReturn;
}

/* circlet shatter */
int ersh_glass_circlet(OBJ *circlet, CHAR *ch, int cmd, char *arg)
{
  int counter = number(53,61);
  ch=circlet->equipped_by;

  if(ch && cmd==MSG_TICK && !IS_NPC(ch))
  {
	if(circlet==EQ(ch,WEAR_WRIST_L))
	{
	  OBJ_SPEC(circlet)--;
	  if(OBJ_SPEC(circlet)<=0)
	  {
		if(chance(3))
		{
		  act("$n's $P suddenly shatters, cutting $s wrist.",0,ch,0,circlet,TO_ROOM);
	      act("Your $P suddenly shatters, cutting your wrist.",0,ch,0,circlet,TO_CHAR);
	      damage(ch,ch,number(13,37),TYPE_UNDEFINED,DAM_NO_BLOCK);
	      extract_obj(unequip_char(ch,WEAR_WRIST_L));
	    } else {
		  OBJ_SPEC(circlet)=counter;
	    }
	  }
    }
	if(circlet==EQ(ch,WEAR_WRIST_R))
	{
	  OBJ_SPEC(circlet)--;
	  if(OBJ_SPEC(circlet)<=0)
	  {
		if(chance(3))
		{
		  act("$n's $P suddenly shatters, cutting $s wrist.",0,ch,0,circlet,TO_ROOM);
	      act("Your $P suddenly shatters, cutting your wrist.",0,ch,0,circlet,TO_CHAR);
	      damage(ch,ch,number(13,37),TYPE_UNDEFINED,DAM_NO_BLOCK);
	      extract_obj(unequip_char(ch,WEAR_WRIST_R));
	    } else {
		  OBJ_SPEC(circlet)=counter;
	    }
	  }
    }
  }
  return FALSE;
}

/* ======================================================================== */
/* ================================ROOM SPECS============================== */
/* ======================================================================== */

/* throw players shorter than 175cm back into the bar */
int ersh_casino(int room, CHAR *ch, int cmd, char *arg)
{

  if(cmd==MSG_ENTER && V_ROOM(ch)==CASINO_ROOM && IS_MORTAL(ch) && !IS_NPC(ch) && (GET_HEIGHT(ch) < 175))
  {
    act("The pit-boss whistles to a bouncer to show the midget $n the door.",0,ch,0,0,TO_ROOM);
    send_to_char("\n\rThe pit-boss identifies you as a midget and whistles for a bouncer.",ch);
    act("A bouncer catches $n by the scruff of the neck and escorts $m out.",0,ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch,real_room(BAR_ROOM));
    act("A bouncer drags $n into the room and throws $m against the bar.",0,ch,0,0,TO_ROOM);
    send_to_char("\n\rA bouncer drags you from the casino and throws you against the bar. *OUCH!*\n\n\r",ch);
    damage(ch,ch,31,TYPE_UNDEFINED,DAM_NO_BLOCK);
	do_look(ch,"",CMD_LOOK);
  }
  return FALSE;
}

/* ======================================================================== */
/* ===============================MOBILE SPECS============================= */
/* ======================================================================== */

/* damage buffer accompanied by flavorful commentary */
int ersh_female_citizen(CHAR *fcit, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;

  if(cmd==MSG_MOBACT && fcit->specials.fighting && chance(14))
  {
    vict = fcit->specials.fighting;
    do_say(fcit,"Start your brawling in some other city, we're a peaceful people!",CMD_SAY);
    act("$n slaps $N in the face.",0,fcit,0,vict,TO_NOTVICT);
    act("$n slaps you in the face.",0,fcit,0,vict,TO_VICT);
    damage(fcit,vict,23,TYPE_UNDEFINED,DAM_SKILL);
  }
  return FALSE;
}

/* damage all fighting mortals accompanied by flavorful commentary */
int ersh_male_citizen(CHAR *mcit, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;

  if(cmd==MSG_MOBACT && mcit->specials.fighting && chance(14))
  {
    do_say(mcit,"Get out of our peaceful city you troublemaker!",CMD_SAY);
    act("$n twirls around in a frenzy.",0,mcit,0,0,TO_NOTVICT);
    act("You twirl around in a frenzy.",0,mcit,0,0,TO_CHAR);
    for(vict = world[CHAR_REAL_ROOM(mcit)].people; vict; vict = next_vict)
    {
      next_vict = vict->next_in_room;
      if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
      act("$N is clipped by a flailing limb as $n spins wildly around the room.",0,mcit,0,vict,TO_NOTVICT);
      act("You are clipped by a flailing limb as $n spins wildly around the room.",0,mcit,0,vict,TO_VICT);
      damage(mcit,vict,17,TYPE_UNDEFINED,DAM_SKILL);
    }
  }
  return FALSE;
}

/* damage specs accompanied by flavorful commentary */
int ersh_child(CHAR *child, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;

  if(cmd==MSG_MOBACT && child->specials.fighting)
  {
    switch(number(0,19))
    {
      case 0:
      case 1:
      case 2:/* 15% kick random */
        vict = get_random_victim_fighting(child);
        if (!vict) break;
        do_say(child,"Go cause trouble in some other city you wayfarer!",CMD_SAY);
        act("$n runs up to $N and kicks $m in the shins.",0,child,0,vict,TO_NOTVICT);
        act("$n kicks you in the shins.",0,child,0,vict,TO_VICT);
        damage(child,vict,31,TYPE_UNDEFINED,DAM_NO_BLOCK);
		break;
      case 3:
      case 4:
      case 5:/* 15% punch and lag buffer */
        vict = child->specials.fighting;
        if (!vict) break;
        act("$n punches $N in the groin.",0,child,0,vict,TO_NOTVICT);
        act("$n punches you in the groin.",0,child,0,vict,TO_VICT);
        damage(child,vict,41,TYPE_UNDEFINED,DAM_SKILL);
        WAIT_STATE(vict,PULSE_VIOLENCE*3);
        break;
      default:
        break;
    }
  }
  return FALSE;
}

/* assorted specs accompanied by flavorful commentary */
int ersh_dealer(CHAR *dealer, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  bool bReturn = FALSE;

  switch (cmd)
  {
    case MSG_MOBACT:
      if(dealer->specials.fighting && chance(10))
      {
        vict = get_random_victim_fighting(dealer);
        if (!vict) break;
        do_say(dealer,"I think I've had just about enough of your childish antics.",CMD_SAY);
        act("$n whistles to the bouncers hidden in the shadows.",0,dealer,0,0,TO_ROOM);
        act("The casino bouncers ruthlessly beat $N to a pulp.",0,dealer,0,vict,TO_NOTVICT);
        act("The casino bouncers ruthlessly beat you to a pulp.",0,dealer,0,vict,TO_VICT);
        damage(dealer,vict,41,TYPE_UNDEFINED,DAM_SKILL);
        WAIT_STATE(vict,PULSE_VIOLENCE*3);
      }
      break;
    case CMD_BASH:/* blocks bash command, damages char and makes them sit */
	  act("$N tries to bash $n.",0,dealer,0,ch,TO_NOTVICT);
	  act("You try to bash $n.",0,dealer,0,ch,TO_VICT);
	  act("$n shoves $N to the ground.",0,dealer,0,ch,TO_NOTVICT);
	  act("$n shoves you to the ground.",0,dealer,0,ch,TO_VICT);
	  damage(dealer,ch,number(5,11)*2,TYPE_UNDEFINED,DAM_NO_BLOCK);
	  GET_POS(ch)=POSITION_SITTING;
      bReturn = TRUE;
      break;
    case CMD_PUMMEL:/* blocks pummel command, damages char and makes them sit */
      act("$n nimbly dodges a foolish pummel attempt from $N.",0,dealer,0,ch,TO_NOTVICT);
      act("$n nimbly dodges your foolish pummel attempt.",0,dealer,0,ch,TO_VICT);
      do_say(dealer,"I'm afraid I don't stand for that kind of behavior chump.",CMD_SAY);
      act("$n forcefully shoves $N to the ground.",0,dealer,0,ch,TO_NOTVICT);
      act("$n forcefully shoves you to the ground.",0,dealer,0,ch,TO_VICT);
      damage(dealer,ch,number(7,17)*2,TYPE_UNDEFINED,DAM_NO_BLOCK);
      GET_POS(ch)=POSITION_SITTING;
      bReturn = TRUE;
      break;
    case CMD_SAY:
    case CMD_WHISPER:
    case CMD_GOSSIP:
    case CMD_SHOUT:
    case CMD_YELL:/* punish patrons for talking while in casino */
      do_say(dealer,"Conspiring to cheat the casino eh? Is that what's going on here?",CMD_SAY);
      act("$n fans out a deck of cards and hurls them at $N like darts.",0,dealer,0,ch,TO_NOTVICT);
      act("$n fans out a deck of cards and hurls them at your chest like darts.",0,dealer,0,ch,TO_VICT);
      damage(dealer,ch,number(3,13)*3,TYPE_UNDEFINED,DAM_SKILL);
      break;
    default:
      break;
  }
  return bReturn;
}

/* drain and heal specs */
int ersh_ghost(CHAR *ghost, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;

  if(cmd==MSG_MOBACT && ghost->specials.fighting)
  {
    switch (number(0,19))
    {
      case 0:
      case 1:
      case 2:/* 15% drain HP and mana */
        vict = get_random_victim_fighting(ghost);
        if (!vict) break;
        act("$n reaches out an ethereal hand to $N's heart.",0,ghost,0,vict,TO_NOTVICT);
        act("$n grabs for your heart with an ethereal hand.",0,ghost,0,vict,TO_VICT);
        damage(ghost,vict,23,TYPE_UNDEFINED,DAM_NO_BLOCK);
        GET_MANA(vict)=MAX(GET_MANA(vict)-17,0);
        break;
      case 3:
      case 4:
      case 5:
      case 6:/* 20% heal */
        act("The ethereal mist composing $n swirls and reforms anew.",0,ghost,0,0,TO_ROOM);
        GET_HIT(ghost)=MIN(GET_HIT(ghost)+53,GET_MAX_HIT(ghost));
        break;
      default:
        break;
    }
  }
  return FALSE;
}

/* burn on entry and cast, fight specs */
int ersh_calcifer(CHAR *calcifer, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;

  switch (cmd)
  {
    case MSG_ENTER:
    case CMD_CAST:
      if(ch && IS_MORTAL(ch))
      {
        act("$n brightens and the floor ignites with blinding white flames.",0,calcifer,0,0,TO_ROOM);
        for(vict = world[CHAR_REAL_ROOM(calcifer)].people; vict; vict = next_vict)
        {
          next_vict = vict->next_in_room;
          if(!vict || IS_NPC(vict) || !IS_MORTAL(vict)) continue;
          act("The white-hot flames lick up your legs, charring your skin.",0,calcifer,0,vict,TO_VICT);
          damage(calcifer,vict,53,TYPE_UNDEFINED,DAM_FIRE);
        }
      }
      break;
    case MSG_MOBACT:
      if(calcifer->specials.fighting)
      {
        switch (number(0,19))
        {
          case 0:
          case 1:
          case 2:/* 15% fireball random */
            vict = get_random_victim_fighting(calcifer);
            if (!vict) break;
            act("$n conjures a giant fireball and hurls it at $N.",0,calcifer,0,vict,TO_NOTVICT);
            act("$n conjures a giant fireball and hurls it at you.",0,calcifer,0,vict,TO_VICT);
            damage(calcifer,vict,number(53,101),TYPE_UNDEFINED,DAM_FIRE);
		    break;
		  case 3:
          case 4:/* 10% burn all */
            act("$n brightens and the floor ignites with blinding white flames.",0,calcifer,0,vict,TO_ROOM);
            for(vict = world[CHAR_REAL_ROOM(calcifer)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if(!vict || IS_NPC(vict) || !IS_MORTAL(vict)) continue;
              act("The white-hot flames lick up your legs, charring your skin.",0,calcifer,0,vict,TO_VICT);
              damage(calcifer,vict,47,TYPE_UNDEFINED,DAM_FIRE);
            }
			break;
          default:
            break;
	    }
      }
    default:
      break;
  }
  return FALSE;
}

/* Spousal Defense System (TM), fight specs */
int ersh_jarve(CHAR *jarve, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *temp, *next;
  bool bCheck = TRUE;
  bool bFound = FALSE;

  if(cmd==MSG_ZONE_RESET && !bCheck) bCheck=TRUE;
  if(cmd==MSG_MOBACT && jarve->specials.fighting)
  {
    switch (number(0,11))
    {
      case 0:
      case 1:/* 16% damage and lag tank */
        vict = jarve->specials.fighting;
        if (!vict) break;
        act("$n rams $N with his enormous belly, pinning $M against the wall.",0,jarve,0,vict,TO_NOTVICT);
        act("$n rams you with his enormous belly, pinning you against the wall.",0,jarve,0,vict,TO_VICT);
        damage(jarve,vict,61,TYPE_UNDEFINED,DAM_SKILL);
        WAIT_STATE(vict,PULSE_VIOLENCE*3);
        break;
      case 3:
      case 4:/* 16% stop fight with random */
        vict = get_random_victim_fighting(jarve);
        if (!vict) break;
        act("$n picks up the nearest chair and hurls it at $N.",0,jarve,0,vict,TO_NOTVICT);
        act("$n picks up the nearest chair and hurls it at you.",0,jarve,0,vict,TO_VICT);
        damage(jarve,vict,41,TYPE_UNDEFINED,DAM_SKILL);
        stop_fighting(vict);
        WAIT_STATE(vict,PULSE_VIOLENCE*2);
        break;
	  default:
        break;
    }

	if(bCheck && jarve->equipment[WEAR_FINGER_L] && V_OBJ(jarve->equipment[WEAR_FINGER_L])==WEDDING_BAND && V_ROOM(jarve)==RESTAURANT_ROOM)
	{/* find Fyona, if alive and not-fighting, bring in to assist */
      for(temp = world[real_room(BAR_ROOM)].people; temp; temp = next)
      {
        next = temp->next_in_room;
        if(!IS_NPC(temp) || (V_MOB(temp)!=FYONA) || temp->specials.fighting) continue;
        if(IS_SET(EXIT(temp, WEST)->exit_info, EX_CLOSED)) do_move_keyword(temp,"curtain",CMD_MOVE);
        act("$n charges out of the bar.",0,temp,0,0,TO_ROOM);
        char_from_room(temp);
        char_to_room(temp, real_room(RESTAURANT_ROOM));
        bFound=TRUE;
        break;
      }
      if(bFound)
      {
		act("$n comes charging into the restaurant.",0,temp,0,0,TO_ROOM);
	    do_say(jarve,"Hey hon, this bugger's messing up our restaurant!",CMD_SAY);
        hit(temp,jarve->specials.fighting,TYPE_HIT);
	  }
      bCheck=FALSE;
    }
  }
  return FALSE;
}

/* Spousal Defense System (TM), fight specs */
int ersh_fyona(CHAR *fyona, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *temp, *next;
  struct affected_type_5 af;
  bool bCheck = TRUE;
  bool bFound = FALSE;

  if(cmd==MSG_ZONE_RESET && !bCheck) bCheck=TRUE;
  if(cmd==MSG_MOBACT && fyona->specials.fighting)
  {
     if(chance(20))
     {/* 20% mug throw -> 25% blindness */
        vict = fyona->specials.fighting;
         
        if (vict)
        {
           do_say(fyona,"Get outta here!",CMD_SAY);
           act("$n picks up a mug from behind the counter and lobs it at $N's head.",0,fyona,0,vict,TO_NOTVICT);
           act("$n picks up a mug from behind the counter and lobs it at your head.",0,fyona,0,vict,TO_VICT);
           damage(fyona,vict,31,TYPE_UNDEFINED,DAM_NO_BLOCK);
  
           if(!affected_by_spell(vict,SPELL_BLINDNESS) && chance(25))
           {
              act("The mug shatters as it hits $N's head, sending glass shards into $S eyes.",0,fyona,0,vict,TO_NOTVICT);
              act("The mug shatters as it hits you, sending glass shards into your eyes.",0,fyona,0,vict,TO_VICT);
              af.type = SPELL_BLINDNESS;
              af.location = APPLY_HITROLL;
              af.modifier = -3;
              af.bitvector = AFF_BLIND;
              af.bitvector2 = 0;
              af.duration = 3;
              affect_to_char(vict,&af);
              act("$N seems to be blinded!",0,fyona,0,vict,TO_NOTVICT);
              act("You have been blinded!",0,fyona,0,vict,TO_VICT);
           }
        }
     }

     if(bCheck && fyona->equipment[WEAR_FINGER_L] && 
        V_OBJ(fyona->equipment[WEAR_FINGER_L])==WEDDING_BAND && 
        V_ROOM(fyona)==BAR_ROOM)
     {/* find Jarve, if alive and not-fighting, bring in to assist */
        for(temp = world[real_room(RESTAURANT_ROOM)].people; temp; temp = next)
        {
           next = temp->next_in_room;
           if(!IS_NPC(temp) || (V_MOB(temp)!=JARVE) || temp->specials.fighting) continue;
           if(IS_SET(EXIT(temp, EAST)->exit_info, EX_CLOSED)) do_move_keyword(temp,"curtain",CMD_MOVE);
           act("$n charges out of the restaurant.",0,temp,0,0,TO_ROOM);
           char_from_room(temp);
           char_to_room(temp, real_room(BAR_ROOM));
           bFound=TRUE;
           break;
        }
      
        if(bFound)
        {
           act("$n comes barreling into the bar.",0,temp,0,0,TO_ROOM);
	   do_say(fyona,"My love, you have come to protect me!",CMD_SAY);
           if (fyona->specials.fighting) hit(temp,fyona->specials.fighting,TYPE_HIT);
	}
	bCheck=FALSE;
     }
  }
  return FALSE;
}

/* alchemist potion specs */
int ersh_elago(CHAR *elago, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict, *temp, *next, *mouse;
  struct affected_type_5 af;
  int mouse_count;

  if(cmd==MSG_MOBACT && elago->specials.fighting)
  {
    switch (number(0,35))
    {
      case 0:
      case 1:
      case 2:/* 8% paralyze and -2 damage potion random */
        vict = get_random_victim_fighting(elago);
        if (!vict) break;
        act("$n throws a potion at $N's feet which explodes, releasing a toxic green cloud.",0,elago,0,vict,TO_NOTVICT);
        act("$n throws a potion at your feet which explodes, releasing a toxic green cloud.",0,elago,0,vict,TO_VICT);
        if(!(affected_by_spell(vict,SPELL_PARALYSIS)))
        {
          af.type = SPELL_PARALYSIS;
          af.duration = 1;
          af.location = APPLY_DAMROLL;
          af.modifier = -2;
          af.bitvector = AFF_PARALYSIS;
          af.bitvector2 = 0;
          affect_to_char(vict,&af);
	      act("$N is paralyzed!",0,elago,0,vict,TO_NOTVICT);
	      act("Your limbs freeze in place.",0,elago,0,vict,TO_VICT);
	    }
        break;
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:/* 14% mutant mouse creation */
        mouse_count = 0;
        for(temp = world[real_room(ATTIC_ROOM)].people; temp; temp = next)
        {/* count mice */
          next = temp->next_in_room;
          if (IS_NPC(temp) && V_MOB(temp)==MOUSE) mouse_count++;
        }
        if(mouse_count<=5)
        {
          act("$n throws a crimson potion at an unsuspecting mouse, filling the room with smoke.",0,elago,0,0,TO_ROOM);
          send_to_room("As the smoke dissipates the form of an enormous mouse comes into view.\n\r",CHAR_REAL_ROOM(elago));
          vict = elago->specials.fighting;
          if (!vict) break;
          mouse = read_mobile(MOUSE, VIRTUAL);
          char_to_room(mouse, real_room(ATTIC_ROOM));
          hit(mouse,vict,TYPE_HIT);
        }
        break;
      case 8:
      case 9:
      case 10:
      case 11:/* 11% pink elephant charge all */
        act("$n throws a bright pink potion at the ground which shatters on impact.",0,elago,0,0,TO_ROOM);
        act("A pink cloud erupts from the broken beaker and takes the form of an elephant.",0,elago,0,0,TO_ROOM);
        act("The corporeal pink elephant charges around the room wildly.",0,elago,0,0,TO_ROOM);
        for(vict = world[CHAR_REAL_ROOM(elago)].people; vict; vict = next_vict)
        {
          next_vict = vict->next_in_room;
          if(!vict || IS_NPC(vict) || !IS_MORTAL(vict)) continue;
          act("The elephant's trunk whips around, catching you in the stomach.",0,elago,0,vict,TO_VICT);
          damage(elago,vict,61,TYPE_UNDEFINED,DAM_MAGICAL);
        }
        break;
      case 12:
      case 13:
      case 14:
      case 15:/* 11% damage and lag potion random */
        vict = get_random_victim_fighting(elago);
        if (!vict) break;
        act("$n throws a pale white potion at $N, which shatters on impact.",0,elago,0,vict,TO_NOTVICT);
        act("$n throws a pale white potion at you, which shatters on impact.",0,elago,0,vict,TO_VICT);
        act("Wispy tendrils burst forth from the broken beaker, ensnaring $N!",0,elago,0,vict,TO_NOTVICT);
        act("Wispy tendrils burst forth from the broken beaker, ensnaring you!",0,elago,0,vict,TO_VICT);
        damage(elago,vict,number(11,31),TYPE_UNDEFINED,DAM_MAGICAL);
        WAIT_STATE(vict,PULSE_VIOLENCE*2);
        break;
      case 16:
      case 17:
      case 18:/* 8% sanctuary */
        act("$n quaffs an azure potion.",0,elago,0,0,TO_ROOM);
        if(!(affected_by_spell(elago,SPELL_SANCTUARY)))
        {
          af.type = SPELL_SANCTUARY;
          af.duration = 2;
          af.location = APPLY_DAMROLL;
          af.modifier = 2;
          af.bitvector = AFF_SANCTUARY;
          af.bitvector2 = 0;
          affect_to_char(elago,&af);
          act("$n is surrounded by a white aura.",0,elago,0,0,TO_ROOM);
	    }
        break;
      case 19:
      case 20:
      case 21:
      case 22:/* 11% heal */
        act("$n quaffs a periwinkle-blue potion.",0,elago,0,0,TO_ROOM);
        GET_HIT(elago)=MIN(GET_HIT(elago) + 179, GET_MAX_HIT(elago));
        break;
      default:
        break;
    }
  }
  return FALSE;
}

/* response and damage specs accompanied by flavorful commentary */
int ersh_priest(CHAR *priest, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  struct affected_type_5 af;
  bool bReturn = FALSE;

  if(priest->specials.fighting)
  {
	switch(cmd)
    {
      case CMD_PUMMEL:
        do_say(priest,"There will be no brawling in the church!",CMD_SAY);
        act("As $N makes a move to pummel, $E is blasted by a holy light.",0,priest,0,ch,TO_NOTVICT);
        act("As you make a move to pummel, you are blasted by a holy light.",0,priest,0,ch,TO_VICT);
        damage(priest,ch,41,TYPE_UNDEFINED,DAM_MAGICAL);
        bReturn = TRUE;
        break;
      case MSG_MOBACT:
        switch(number(0,16))
        {
	      case 0:
	      case 1:
	        do_say(priest,"This is a place of worship you heathen!",CMD_SAY);
      	    break;
          case 2:
    	  case 3:/* 12% damage and blind random evil char */
    	    vict = get_random_victim_fighting(priest);
            if (!vict) break;
            if(!IS_EVIL(vict))
            {
      		  do_say(priest,"Your evilness belongs elsewhere fiend.",CMD_SAY);
      		  act("A blindingly white lightning bolt slams into $N's chest.",0,priest,0,vict,TO_NOTVICT);
      		  act("A blindingly white lightning bolt strikes you in the chest.",0,priest,0,vict,TO_VICT);
      		  damage(priest,vict,67,TYPE_UNDEFINED,DAM_MAGICAL);
              if(!affected_by_spell(vict,SPELL_BLINDNESS))
              {
		        af.type = SPELL_BLINDNESS;
		        af.location = APPLY_HITROLL;
		        af.modifier = -3;
		        af.bitvector = AFF_BLIND;
		        af.bitvector2 = 0;
		        af.duration = 3;
		        affect_to_char(vict,&af);
                act("$N seems to be blinded!",0,priest,0,vict,TO_NOTVICT);
			    act("You have been blinded!",0,priest,0,vict,TO_VICT);
			  }
		    }
      		break;
      	  default:
      	    break;
	    }
      default:
        break;
    }
  }
  return bReturn;
}

/* damage specs */
int ersh_dark_priest(CHAR *dpriest, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  struct affected_type_5 af;

  if(cmd==MSG_MOBACT && dpriest->specials.fighting)
  {
    vict = get_random_victim_fighting(dpriest);
    if (!vict) return FALSE;

    switch (number(0,23))
    {
      case 0:
      case 1:
      case 2:
      case 3:/* 17% purple orb random vict */
        act("$n conjures a swirling purple orb and throws it at $N.",0,dpriest,0,vict,TO_NOTVICT);
        act("$n conjures a swirling purple orb and hurls it at you.",0,dpriest,0,vict,TO_VICT);
        damage(dpriest,vict,61,TYPE_UNDEFINED,DAM_COLD);
		break;
  	  case 4:
      case 5:
      case 6:/* 13% chill touch random vict */
		if(IS_NIGHT)
        {
          do_say(dpriest,"Allow me to introduce you to my brethren.",CMD_SAY);
          act("$n utters an incantation that reanimates the corpses in the walls!",0,dpriest,0,vict,TO_ROOM);
          act("The rotting corpses ruthlessly maul $N.",0,dpriest,0,vict,TO_NOTVICT);
          act("The rotting corpses ruthlessly maul you.",0,dpriest,0,vict,TO_VICT);
          damage(dpriest,vict,31,TYPE_UNDEFINED,DAM_NO_BLOCK);
          if(!affected_by_spell(vict,SPELL_CHILL_TOUCH))
          {
            af.type = SPELL_CHILL_TOUCH;
            af.duration = 2;
            af.modifier = -2;
            af.location = APPLY_STR;
            af.bitvector = 0;
            af.bitvector2 = 0;
            affect_to_char(vict,&af);
            act("The rotting corpses' hits chill $N who suddenly seem less lively.",0,dpriest,0,vict,TO_NOTVICT);
            act("You feel your life flowing away as the rotting corpses' hits chill you.",0,dpriest,0,vict,TO_VICT);
	  	  }
	  	  act("$n's undead puppets return to their coffins, lifeless once more.",0,dpriest,0,vict,TO_ROOM);
		}
	    break;
      default:
        break;
	}
  }
  return FALSE;
}

/* assignments */
void assign_ershteep(void) {

  /* Objects */
  assign_obj(ORGAN,				ersh_organ);
  assign_obj(WEDDING_BAND,		ersh_wedding_band);
  assign_obj(KEY,				ersh_key);
  assign_obj(GLASS_CIRCLET,		ersh_glass_circlet);
  /* Rooms */
  assign_room(CASINO_ROOM,		ersh_casino);
  /* Mobs */
  assign_mob(FEMALE_CITIZEN,	ersh_female_citizen);
  assign_mob(ENFAN_CITIZEN,		ersh_male_citizen);
  assign_mob(ENFAN_CHILD,		ersh_child);
  assign_mob(DEALER,			ersh_dealer);
  assign_mob(GHOST,				ersh_ghost);
  assign_mob(CALCIFER,			ersh_calcifer);
  assign_mob(JARVE,				ersh_jarve);
  assign_mob(FYONA,				ersh_fyona);
  assign_mob(ELAGO,				ersh_elago);
  assign_mob(PRIEST,			ersh_priest);
  assign_mob(DARK_PRIEST,		ersh_dark_priest);
}
