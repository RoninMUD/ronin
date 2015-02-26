/**********************************************************************/
/****                                                              ****/
/****          Specs for Barovia & Castle of Ravenloft             ****/
/****                                                              ****/
/**********************************************************************/

/****************************************************************************
 *               Special procedure module for Barovia                        *
 *                                                                           *
 *                       by: J Lasilla (Ervin)                               *
 *             modified by:  Vishnu, Slash, and Balinor                      *
 *                                                                           *
 *             contact Vishnu at : clay@nano.engr.mun.ca                     *
 ****************************************************************************/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:13 $
$Header: /home/ronin/cvs/ronin/spec.barovia.c,v 2.0.0.1 2004/02/05 16:10:13 ronin Exp $
$Id: spec.barovia.c,v 2.0.0.1 2004/02/05 16:10:13 ronin Exp $
$Name:  $
$Log: spec.barovia.c,v $
Revision 2.0.0.1  2004/02/05 16:10:13  ronin
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
#include "spec_assign.h"
extern struct time_info_data time_info;
extern int CHAOSMODE;
/* Make one npc rescue another */

int bar_do_npc_rescue (CHAR *ch_hero, CHAR *ch_victim) {
  CHAR *ch_bad_guy ;

  for (ch_bad_guy=world[CHAR_REAL_ROOM(ch_hero)].people;
       ch_bad_guy && (ch_bad_guy->specials.fighting != ch_victim);
       ch_bad_guy=ch_bad_guy->next_in_room)  ;
  if (ch_bad_guy) {
    if (ch_bad_guy == ch_hero)
      return FALSE ; /* NO WAY I'll rescue the one I'm fighting! */
    act ("You bravely rescue $N.\n\r", FALSE, ch_hero, 0, ch_victim, TO_CHAR) ;
    act("You are rescued by $N, your subservient minion.\n\r",
	FALSE, ch_victim, 0, ch_hero, TO_CHAR);
    act("With a scream of rage $n throws $mself in front of $N.", FALSE, ch_hero, 0, ch_victim, TO_NOTVICT);

    if (ch_bad_guy->specials.fighting)
      stop_fighting(ch_bad_guy);
    if (ch_hero->specials.fighting)
      stop_fighting(ch_hero);

    set_fighting(ch_hero, ch_bad_guy);
    set_fighting(ch_bad_guy, ch_hero);
    return TRUE ;
  }
  return FALSE ;
}

/* Check if an npc is in the same room as the one checking */

CHAR *bar_find_npc (CHAR *chAtChar, char *pszName,
		    int iLen) {
  CHAR *ch ;
  if(chAtChar)
   for (ch = world[CHAR_REAL_ROOM(chAtChar)].people ; ch ; ch = ch->next_in_room)
    if (IS_NPC(ch))
      if (!strncmp (pszName, ch->player.short_descr, iLen))
	return (ch) ;

  return NULL ;
}

/* Make vampires rescue Strahd if he is there and fighting */

int vampire(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *strahd ;

  if(!mob)
    return (FALSE);
  if (!AWAKE(mob))
    return FALSE ;

  if (cmd)
    return FALSE;
  if ((strahd = bar_find_npc (ch, "Strahd", 6)))
    {
      while (strahd->specials.fighting != mob->specials.fighting)
	{
	  bar_do_npc_rescue (mob, strahd) ;
	  return FALSE;
	}
    }
  if(mob->specials.fighting && (CHAR_REAL_ROOM(mob->specials.fighting) == CHAR_REAL_ROOM(mob)))
    {
      if(number(0,2) == 0){
	act("$n bites you on the neck !",1,mob,0,mob->specials.fighting,TO_VICT);
	act("$n bites $N on the neck !",1,mob,0,mob->specials.fighting,TO_NOTVICT);
	damage(mob,mob->specials.fighting,GET_HIT(mob->specials.fighting)/5+40,TYPE_UNDEFINED,DAM_NO_BLOCK);
	return FALSE;
      }
    }
  return FALSE;
}

int strahd(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

  CHAR *victim, *new_mob, *temp;
  char num,buf[MAX_INPUT_LENGTH];
  OBJ *wield;
  static char sttaus = 0, tries = 0;


  if(cmd==MSG_TICK) {
    if (!mob->specials.fighting) {
      if((sttaus == 4) && (GET_HIT(mob) > 2400) && (CHAR_REAL_ROOM(mob) == real_room(11426))) {
        sttaus = 0;
        tries = 0;
        return FALSE;
      }
      if(GET_HIT(mob)==GET_MAX_HIT(mob) && CHAR_REAL_ROOM(mob) == real_room(11426)) {
        sttaus = 0;
        tries = 0;
        return FALSE;
      }
    }
    return FALSE;
  }

  if(cmd!=MSG_MOBACT && cmd != MSG_DIE) return FALSE;
  if(cmd==MSG_MOBACT && !mob->specials.fighting) return FALSE;

  /* Summon some help ! (zombie & ghoul) */

  if(sttaus == 0){
    sttaus = 1;
    act("$n makes a strange gesture, and a portal opens !", TRUE, mob,0, 0, TO_ROOM);

    if(!(new_mob = read_mobile(real_mobile(11322), REAL)))
      return(FALSE);
    char_to_room(new_mob, CHAR_REAL_ROOM(mob));
    act("a zombie jumps through the portal !", FALSE, mob, 0, new_mob, TO_ROOM);

    if(!(new_mob = read_mobile(real_mobile(11323), REAL)))
      return(TRUE);
    char_to_room(new_mob, CHAR_REAL_ROOM(mob));
    act("a ghoul jumps through the portal !", FALSE, mob, 0, new_mob, TO_ROOM);

    if(!(new_mob = read_mobile(real_mobile(11322), REAL)))
      return(FALSE);
    char_to_room(new_mob, CHAR_REAL_ROOM(mob));
    act("a zombie jumps through the portal !", FALSE, mob, 0, new_mob, TO_ROOM);

    if(!(new_mob = read_mobile(real_mobile(11323), REAL)))
      return(TRUE);
    char_to_room(new_mob, CHAR_REAL_ROOM(mob));
    act("a ghoul jumps through the portal !", FALSE, mob, 0, new_mob, TO_ROOM);

    return FALSE;
  }


  /* Need a miracle ? */

  if((GET_HIT(mob) < 600) && (sttaus == 1) ) {
    act("$n utters the words 'he titul iparem piolo'",1,mob,0,0,TO_ROOM);
    act("$n seems to be completely healed again !'",1,mob,0,0,TO_ROOM);
    /* cast_miracle(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, mob, 0); */
    GET_HIT(mob) = GET_MAX_HIT(mob); 
    sttaus = 2;
    return FALSE;
  }

  /* Need a teleport ? */

  if((GET_HIT(mob) < 750) && sttaus == 2) {   /* 2 ghouls to keep players company after Strahd leaves */
    sttaus = 3;
    if(!(new_mob = read_mobile(real_mobile(11323), REAL)))
      return(FALSE);
    char_to_room(new_mob, CHAR_REAL_ROOM(mob));

    if(!(new_mob = read_mobile(real_mobile(11323), REAL)))
      return(FALSE);
    char_to_room(new_mob, CHAR_REAL_ROOM(mob));

    if(!(new_mob = read_mobile(real_mobile(11323), REAL)))
      return(FALSE);
    char_to_room(new_mob, CHAR_REAL_ROOM(mob));

    if(!(new_mob = read_mobile(real_mobile(11323), REAL)))
      return(FALSE);
    char_to_room(new_mob, CHAR_REAL_ROOM(mob));

    act("$n makes a strange gesture, and a portal opens !", TRUE,mob,0, 0, TO_ROOM);
    act("$N jumps through the portal !", FALSE, mob, 0, new_mob, TO_ROOM);
    act("$N jumps through the portal !", FALSE, mob, 0, new_mob, TO_ROOM);
  }

  if(sttaus == 3)
    {
    act("$n says 'So you fools thought you could kill me !'",FALSE,mob,0,0,TO_ROOM);
    act("$n utters the words 'japo ooois'",FALSE,mob,0,0,TO_ROOM);
    act("$n disappears in a puff of smoke",FALSE,mob,0,0,TO_ROOM);
    char_from_room(mob);
    if(number(0,1) == 0)
      char_to_room(mob,real_room(11438));                 /* The room with coffins */
    else
      char_to_room(mob,real_room(11439));                 /* The other room with coffins */
    act("$n arrives in a puff of smoke.",FALSE,mob,0,0,TO_ROOM);
    sttaus = 4;
    return FALSE;
    }


/* New disarm attack - Jan 97 - Ranger */

  if(!number(0,3) && mob->specials.fighting) {
    if((victim = get_random_victim(mob))) {
      if(EQ(victim, WIELD)) {
        wield = victim->equipment[WIELD];
        if(V_OBJ(wield)==11523) return FALSE;
        sprintf(buf,"WIZINFO: %s disarms %s's %s",GET_NAME(mob),GET_NAME(victim),OBJ_SHORT(wield));
        log_s(buf);
        wield->log=1;
        obj_to_char(unequip_char(victim, WIELD), mob);
        act("$n bites you on the arm, and takes your weapon ! ",1,mob,0,victim,TO_VICT);
        act("$n bites $N on the arm and takes $S weapon !",1,mob,0,victim,TO_NOTVICT);
        act("You bite $N on the arm and take $S weapon",1,mob,0,victim,TO_CHAR);
        save_char(victim, NOWHERE);
        return FALSE;
      }
    }
  }

  /* Blind/sleep all clerics ? Tried enough ? Iceball them !  */

  if(number(0,3) != 3)
    {
    for(victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = temp) {
      temp = victim->next_in_room;  /* Added temp - Ranger June 96 */
      if(((GET_CLASS(victim) == CLASS_CLERIC) ||
	  (GET_CLASS(victim) == CLASS_PALADIN)) &&
	 (GET_LEVEL(victim) < LEVEL_IMM))
        {
	if(!affected_by_spell(victim,SPELL_BLINDNESS && !affected_by_spell(victim,SPELL_SLEEP)))
          {
	  switch(tries)
            {
	    case 0:
	    case 1:
	    case 2:
	      act("$n utters the words 'nak opois'",1,mob,0,0,TO_ROOM);
	      cast_blindness(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
	      tries++;
	      return FALSE;
	    case 3:
	    case 4:
	      act("$n says 'You poor clerics disgust me !'",1,mob,0,0,TO_ROOM);
	      act("$n utters the words 'jaun taka aliin'",1,mob,0,0,TO_ROOM);
	      cast_sleep(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
	      tries++;
	      return FALSE;
	    default:
	      act("$n utters the words 'va hajai tahat tu'",1,mob,0,0,TO_ROOM);
	      act("$n conjures an iceball and throws it at $N",FALSE,mob,0,victim,TO_NOTVICT);
	      act("$n conjures an iceball and throws it at you",FALSE,mob,0,victim,TO_VICT);
	      cast_iceball(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
	      act("$n cackles with insane glee!",1,mob,0,0,TO_ROOM);
	      return FALSE;
	    }
          }
        }
      }
    }
  /* Fear on the tanker ? */

  if(number(0,3)==0)
    {
    if((victim = mob->specials.fighting))
      {
      act("$n utters the words 'jahaal avetaa'",1,mob,0,0,TO_ROOM);
      cast_fear(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
      return FALSE;
      }
    }

  /* Cast nasty spells on other people ! */

  for(victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = temp ) {
    temp = victim->next_in_room; /* Added temp - Ranger June 96 */

    if((!IS_NPC(victim)) && (number(0,3)==0) && (GET_LEVEL(victim) < LEVEL_IMM))
      {
      num=number(0,14);
      switch(num)
        {
        case 0:
  	  act("$n utters the words 'nak opois'",1,mob,0,0,TO_ROOM);
	  cast_blindness(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
          break;
        case 1:
  	  act("$n utters the words 'jaun taka aliin'",1,mob,0,0,TO_ROOM);
	  cast_sleep(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
          break;
        case 2:
  	  act("$n utters the words 'vahav oimap ois'",1,mob,0,0,TO_ROOM);
	  cast_chill_touch(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
	  break;
        case 3:
	  act("$n utters the words 'tu ltam unille'",1,mob,0,0,TO_ROOM);
	  cast_fireball(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
          break;
        case 4:
  	  act("$n utters the words 'va hajai tahat tu'",1,mob,0,0,TO_ROOM);
	  cast_iceball(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
          break;
        case 5:
	  act("$n utters the words 'lo ppus pel lit'",1,mob,0,0,TO_ROOM);
	  cast_energy_drain(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
	  break;
        case 6:
	  act("$n utters the words 'jahaal avetaa'",1,mob,0,0,TO_ROOM);
	  cast_fear(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
          break;
        case 7:
	  act("$n utters the words 'kokokr oppajam ahti'",1,mob,0,0,TO_ROOM);
	  cast_paralyze(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, victim, 0);
          break;
        default:
	  if(mob->specials.fighting)
            {
	    act("$n bites you on the neck, and drains your blood! You feel weaker.",1,mob,0,victim,TO_VICT);
	    act("$n bites $N on the neck !",1,mob,0,victim,TO_NOTVICT);
	    GET_MANA(victim)=MAX(0,GET_MANA(victim)-50);
	    damage(mob,victim,GET_HIT(mob->specials.fighting)/4+80,TYPE_UNDEFINED,DAM_NO_BLOCK);
	    }
	  break;
        }
      }
    }
  return FALSE;
}

int soth(CHAR *s, CHAR *ch, int cmd, char *arg)
{
  CHAR *victim,*temp;
  static char warning = 0;

  if (cmd) return FALSE;
  if(!s) return FALSE;
  if(CHAR_REAL_ROOM(s)==NOWHERE) return FALSE;

  if(GET_POS(s)==POSITION_STANDING) {
    warning = 0;
    if(number(0,6)==0)
      act("$n swears loudly, and shatters objects to pieces with his blows.",FALSE,s,0,0,TO_ROOM);
    return FALSE;
  }

  victim = s->specials.fighting;
  if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s))) {
    if(warning == 1) {
      act("$n reaches out his hand, and a black claw tears your heart !",1,s,0,victim,TO_VICT);
      act("$n reaches out his hand, and a black claw tears $Ns heart !",1,s,0,victim,TO_NOTVICT);
      damage(s,victim,370,TYPE_UNDEFINED,DAM_NO_BLOCK);
      warning = 0;
      return FALSE;
    }

    if((number(0,1)==0) && (GET_HIT(s) < 3000) && (warning == 0)) {
      warning = 1;
      act("$n utters words for a powerful spell ' tap poloit sulahtee !'.",FALSE,s,0,0,TO_ROOM);
      return FALSE;
    }
    if(number(0,3) != 3)
      for(victim = world[CHAR_REAL_ROOM(s)].people; victim && (GET_HIT(s) > 3000); victim = temp ) {
        temp = victim->next_in_room; /* Added temp - Ranger June 96 */
	if(((GET_CLASS(victim) == CLASS_CLERIC) ||
	    (GET_CLASS(victim) == CLASS_PALADIN)) &&
	   (GET_LEVEL(victim) < LEVEL_IMM))
	  {
	    act("$n mutters 'Stupid healers!'",1,s,0,0,TO_ROOM);
	    act("$n smashes you in the side of the head with his gauntlet !",1,s,0,victim,TO_VICT);
	    act("$n smashes $N with a vicious backhand !",1,s,0,victim,TO_NOTVICT);
	    damage(s,victim,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
	  }
      }
  }
  return FALSE;
}


int ghoul(CHAR *gh, CHAR *ch, int cmd, char *arg)
{
  if(cmd) return FALSE;

  if(gh->specials.fighting)
    if((number(0,3) == 0) && (!affected_by_spell(gh->specials.fighting,SPELL_PARALYSIS))){
      send_to_char("You feel your limbs stiffening.\n\r",gh->specials.fighting);
      cast_paralyze(30, gh, "", SPELL_TYPE_SPELL,gh->specials.fighting , 0);
    }
  return FALSE;
}

int b_zombie(CHAR *zombie, CHAR *ch, int cmd, char *arg) {
  if (cmd) return FALSE;

  if(zombie->specials.fighting)
    if((number(0,3) == 0)) {
      send_to_char("The zombie touches you with its withered hand.\n\r",zombie->specials.fighting);
      cast_chill_touch(30, zombie, "", SPELL_TYPE_SPELL,zombie->specials.fighting , 0);
    }
  return FALSE;
}

/* Decaying eq */

int decaying_eq(OBJ *ob, CHAR *ch, int cmd, char *argument)
{
  int n = 0, i = -1;

  if(cmd != CMD_NORTH &&
     cmd != CMD_SOUTH &&
     cmd != CMD_EAST  &&
     cmd != CMD_WEST  &&
     cmd != CMD_UP    &&
     cmd != CMD_DOWN)
    return FALSE;

  if(number(0,250) == 0 && ob->equipped_by)
    {
    for(n=0;n<MAX_WEAR;n++)
       {
       if (ob == ch->equipment[n])
           i = n;
       }
    if (i>-1)
      {
      if(ob->obj_flags.value[0] == 0)
        {   /* Destroy item */
        act("Your $o crumbles to dust.",FALSE,ch,ob,0,TO_CHAR);
        act("$n's $o crumbles to dust.",FALSE,ch,ob,0,TO_ROOM);
        ob = unequip_char(ch,i);
        extract_obj(ob);
        }
      else
        {
        ob = unequip_char(ch,i);
        ob->obj_flags.value[0] -= 1;/* Worsen effecting armor */
        equip_char(ch,ob,i);
        act("Your $o seems to decay a little.",FALSE,ch,ob,0,TO_CHAR);
        }
      }
    }
  return FALSE;
}


/* Soth's sword */
int is_shop(CHAR *mob);

int black_rose_sword(OBJ *obj,CHAR *ch, int cmd, char *argument)
{
  CHAR *vic;
  char arg[MAX_STRING_LENGTH];

  if(cmd != CMD_KILL) return FALSE;
  /* Added safe room check - Ranger Sept 97 */
  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(!ch->specials.fighting &&ch->equipment[WIELD]  == obj)
    {

    /* Check that the target is valid */

    one_argument(argument, arg);

    if(!*arg) {
      act("$n's sword twists in $s hand, and cuts into $s flesh !",FALSE,ch,0,ch,TO_NOTVICT);
      send_to_char("Your sword twists in your hand, and cuts your flesh !\n\r",ch);
      damage(ch,ch,number(15,40),TYPE_UNDEFINED,DAM_NO_BLOCK);
      return TRUE;
    }

    if (!(vic = get_char_room_vis(ch, arg))) {
      send_to_char("Your sword humms violently, as the target isn't here !\n\r",ch);
      return TRUE;
    }

    if(!IS_NPC(vic) && !IS_NPC(ch)){
      send_to_char("Your sword refuses to attack other players !\n\r",ch);
      return TRUE;
    }

    if(ch==vic) {
      act("$n's sword twists in $s hand, and cuts into $s flesh !",FALSE,ch,0,ch,TO_NOTVICT);
      send_to_char("Your sword twists in your hand, and cuts your flesh !\n\r",ch);
      damage(ch,ch,number(15,40),TYPE_UNDEFINED,DAM_NO_BLOCK);
      return TRUE;
    }

    if((GET_LEVEL(ch)-GET_LEVEL(vic)) > 20) return FALSE;

    if(is_shop(vic)) return FALSE;
    /* Cast the spell on him */

    if(number(0,40) != 0) return FALSE;

    act("A black cloud from the sword of Black Rose envelopes $n !",FALSE,ch,0,0,TO_ROOM);
    send_to_char("A black cloud from your sword envelopes you !\n\r",ch);
    if(obj->obj_flags.value[2]<13) {
      send_to_char("You cough and choke in the smoke.\n\r",ch);
      damage(ch,ch,50,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    else
    cast_fury(30, ch, "", SPELL_TYPE_SPELL, ch, 0);
  }
  return FALSE;
}

/* Bracelet of Magic */

int bracelet_of_magic(OBJ *obj, CHAR *ch, int cmd, char *argument)
{
  if(cmd != MSG_TICK)
    return FALSE;
  if(obj->equipped_by)
    {
      if(GET_HIT(obj->equipped_by)>-1) {
        GET_HIT(obj->equipped_by) = MIN( GET_MAX_HIT(obj->equipped_by),
                         GET_HIT(obj->equipped_by)+10);
        GET_MANA(obj->equipped_by) = MIN( GET_MAX_MANA(obj->equipped_by),
                         GET_MANA(obj->equipped_by)+10);
      }
    }
  return FALSE;
}

int barovian(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  static char verse = 0;
  unsigned char num;
  char *vers[] = {
    "$n sings 'Along the coast road, by the headland'",
    "$n sings 'the early lights of winter glow'",
    "$n sings 'I'll pour a cup to you my darling'",
    "$n sings 'Raise it up - say Cheerio.'",
    "$n sniffs sadly.",
    "$n says 'it's such a nice song.'"
    };
  char *cmt[] = {
    "$n says 'Let's sing a nice song !'",
    "$n mumbles something to himself.",
    "$n burps loudly.",
    "$n says 'Hello strangers, have a drink with me !'",
    "$n drinks beer from a mug."
    };


  if (cmd) return FALSE;

  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(verse == 0)
      {
	if(number(0,2)==0)
	  {
	    num = number(0,7);
	    if(num==0) verse = 1;
	    if(num<4)
	      act(cmt[num],FALSE,mob,0,0,TO_ROOM);
	    else
	      act(cmt[4],FALSE,mob,0,0,TO_ROOM);
	  }
	return FALSE;
      }

    if(number(0,2)==0){
      if(verse<6)
	act(vers[verse-1],FALSE,mob,0,0,TO_ROOM);
      else
	{
	  act(vers[5],FALSE,mob,0,0,TO_ROOM);
	  verse = 0;
	}
      verse++;
    }
  }
  return FALSE;
}



int gipsy(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  char n;
  if (cmd) return FALSE;

  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(number(0,3)==0)
      {
	n = number(0,2);
	switch(n){
	case 0:
	  act("$n dances around the campfire.",FALSE,mob,0,0,TO_ROOM);
	  break;
	case 1:
	  act("$n sings a beautiful song.",FALSE,mob,0,0,TO_ROOM);
	  break;
	default:
	  act("$n sips wine from a flask.",FALSE,mob,0,0,TO_ROOM);
	  break;
	}
      }
  }
  return FALSE;
}


int child(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  if (cmd) return FALSE;

  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(number(0,50)==0){
      act("$n says 'Hi ! Did you bring any gifts for me ?",FALSE,mob,0,0,TO_ROOM);
      act("$n smiles happily.",FALSE,mob,0,0,TO_ROOM);
    }
  }
  return FALSE;
}


int seeress(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  int num;
  if (cmd) return FALSE;

  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(number(0,2)==0){
      num = number(0,5);
      switch(num){
      case 0:
	act("$n says 'Strahd will cast thunderball on you'",FALSE,mob,0,0,TO_ROOM);
	break;
      case 1:
	act("$n says 'Vampires will suck you dry'",FALSE,mob,0,0,TO_ROOM);
	break;
      case 2:
	act("$n says 'Lord Soth will impale you'",FALSE,mob,0,0,TO_ROOM);
	break;
      case 3:
	act("$n says 'Ghouls will eat your entrails'",FALSE,mob,0,0,TO_ROOM);
	break;
      case 4:
	act("$n says 'Zombies will tear you apart'",FALSE,mob,0,0,TO_ROOM);
	break;
      case 5:
	act("$n says 'Ghosts will suck out your soul'",FALSE,mob,0,0,TO_ROOM);
	break;
      default:
	break;
      }
    }
  }
  return FALSE;
}

int vampire_guard(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  if (cmd == CMD_UP) {
    send_to_char("Vampire blocks your way !\n\r",ch);
    act("$n blocks $N, and stops $N from going up !",FALSE,mob,0,ch,TO_ROOM);
    return TRUE;
  }

  if (cmd) return FALSE;

  if (GET_POS(mob)!=POSITION_FIGHTING)
    return FALSE;

  if(mob->specials.fighting && (CHAR_REAL_ROOM(mob->specials.fighting) == CHAR_REAL_ROOM(mob))){
    if(number(0,2) == 0){
      act("$n bites you on the neck !",1,mob,0,mob->specials.fighting,TO_VICT);
      act("$n bites $N on the neck !",1,mob,0,mob->specials.fighting,TO_NOTVICT);
      damage(mob,mob->specials.fighting,GET_HIT(mob->specials.fighting)/5+40,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
  }
  return FALSE;
}

int ghost_guard(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  if (cmd == CMD_UP || cmd == CMD_NORTH || cmd == CMD_EAST) {
    send_to_char("The ghost blocks your way !\n\r",ch);
    act("The ghost blocks $n, and stops $s from passing !",FALSE,ch,0,0,TO_ROOM);
    act("The ghost howls 'My master is not expecting visitors !'",FALSE,ch,0,0,TO_ROOM);
    return TRUE;
  }

  return FALSE;
}

int jander(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  if (cmd) return FALSE;

  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(number(0,6)==0){
      act("$n tries to plant some flovers here, but they whither away quickly.",FALSE,mob,0,0,TO_ROOM);
    }
    return FALSE;
  }
  if(mob->specials.fighting && (CHAR_REAL_ROOM(mob->specials.fighting) == CHAR_REAL_ROOM(mob))){
    if(number(0,2) == 0){
      act("$n bites you on the neck !",1,mob,0,mob->specials.fighting,TO_VICT);
      act("$n bites $N on the neck !",1,mob,0,mob->specials.fighting,TO_NOTVICT);
      damage(mob,mob->specials.fighting,GET_HIT(mob->specials.fighting)/4+40,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
  }
  return FALSE;
}

/* ******************** End of Barovia & Castle of Ravenloft ****************** */


#define BAR_ZONE      11300
#define BAR_BAROVIAN  ITEM(BAR_ZONE,31)
#define BAR_JANDER    ITEM(BAR_ZONE,27)
#define BAR_GUARD     ITEM(BAR_ZONE,30)
#define BAR_VAMPIRE   ITEM(BAR_ZONE,25)
#define BAR_GIPSY     ITEM(BAR_ZONE,19)
#define BAR_SEERESS   ITEM(BAR_ZONE,20)
#define BAR_ABEL      ITEM(BAR_ZONE,12)
#define BAR_ALYS      ITEM(BAR_ZONE,13)
#define BAR_SEERESS   ITEM(BAR_ZONE,20)
#define BAR_STRAHD    ITEM(BAR_ZONE,26)
#define BAR_SOTH      ITEM(BAR_ZONE,24)
#define BAR_GHOUL     ITEM(BAR_ZONE,23)

#define BAR_SWORD      ITEM(BAR_ZONE,17)
#define BAR_BRACELET   ITEM(BAR_ZONE,14)
#define BAR_DECAY1     ITEM(BAR_ZONE,20)
#define BAR_DECAY2     ITEM(BAR_ZONE,21)
#define BAR_DECAY3     ITEM(BAR_ZONE,22)
#define BAR_DECAY4     ITEM(BAR_ZONE,23)
#define BAR_DECAY5     ITEM(BAR_ZONE,24)
#define BAR_DECAY6     ITEM(BAR_ZONE,25)
#define BAR_DECAY7     ITEM(BAR_ZONE,26)

void assign_barovia (void) {
  assign_mob(BAR_BAROVIAN, barovian);
  assign_mob(BAR_JANDER,    jander);
  assign_mob(BAR_GUARD ,    vampire_guard);
  assign_mob(BAR_VAMPIRE,  vampire);
  assign_mob(BAR_GIPSY  ,   gipsy);
  assign_mob(BAR_SEERESS,   seeress);
  assign_mob(BAR_ABEL   ,   child);
  assign_mob(BAR_ALYS   ,   child);
  assign_mob(BAR_STRAHD ,   strahd);
  assign_mob(BAR_SOTH   ,   soth);
  assign_mob(BAR_GHOUL  ,   ghoul);
  assign_mob(11322      ,   b_zombie);

  assign_obj(BAR_SWORD   ,  black_rose_sword);
  assign_obj(BAR_BRACELET, bracelet_of_magic);
  assign_obj(BAR_DECAY1  ,  decaying_eq);
  assign_obj(BAR_DECAY2  ,  decaying_eq);
  assign_obj(BAR_DECAY3  ,  decaying_eq);
  assign_obj(BAR_DECAY4  ,  decaying_eq);
  assign_obj(BAR_DECAY5  ,  decaying_eq);
  assign_obj(BAR_DECAY6  ,  decaying_eq);
  assign_obj(BAR_DECAY7  ,  decaying_eq);
}
