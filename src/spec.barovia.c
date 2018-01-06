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

#ifndef TEST_SITE
    if(number(0,40) != 0) return FALSE;
#endif

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

#define UBER_STRAHD       11300
#define NAMELESS_ONE      11301
#define SUPER_ZOMBIE         11302
#define NOTSOSUPER_ZOMBIE    11303
#define STRAHD_SPAWNROOM        11400
#define STRAHD_BEDCHAMBER         11499
#define STRAHD_TORTURECHAMBER         11431
#define UBER_BOM          11300
#define PENTACLE          11301
#define BAVARIA_BOTTOM          11300
#define BAVARIA_TOP         11347

extern void mob_do(CHAR* mob, char *arg);

int uber_strahd(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL, *next_vict = NULL, *nameless = NULL, *zombie = NULL;
  char buf[MAX_STRING_LENGTH];
  int num_specs = 0, drain = 0, num_dispels = 0, tele_room = 0, eq_slot = 0, num_charms = 0, zombie_count = 0;
  OBJ *charm_obj = NULL, *obj = NULL;
  static int form = 0;
  struct affected_type_5 af;

  switch (cmd) {
    case MSG_TICK:
      // regenerate 10% of HP every tick when not fighting
      if (!uber->specials.fighting) {
        if (GET_HIT(uber) < GET_MAX_HIT(uber)) {
          GET_HIT(uber) = MIN(( GET_HIT(uber) + ( GET_MAX_HIT(uber) / 10 ) ), GET_MAX_HIT(uber));
          sprintf(buf, "Bathed in an aura of darkness, %s regenerates some of %s wounds.\n\r", GET_SHORT(uber), HSHR(uber) );
          send_to_room(buf, CHAR_REAL_ROOM(uber));
        } else if (uber->carrying) {
        //drop all "borrowed" equipment on the ground if full on HP
          mob_do(uber, "yawn");
          do_say(uber, "Filthy mortal trinkets, blech.", CMD_SAY);
          mob_do(uber, "drop all");
        } else {
          // equip an uber-bom if he doesn't have one
          obj=EQ(uber, WEAR_WRIST_L);
          if (obj && ( V_OBJ(obj) != UBER_BOM )) obj_to_char( unequip_char(uber, WEAR_WRIST_L), uber );
          else if (!obj) {
            obj=read_object(UBER_BOM, VIRTUAL);
            if(chance(90)) REMOVE_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_EVIL);
            if(chance(20)) REMOVE_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_GOOD);
            if(chance(33)) REMOVE_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_PALADIN);
            if(chance(33)) REMOVE_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_BARD);
            equip_char(uber, obj, WEAR_WRIST_L);
          }
        }
      } else {
        if (!affected_by_spell(uber, SPELL_BLACKMANTLE) && chance(20) ) {
          af.type = SPELL_BLACKMANTLE;
          af.duration = 1;
          af.modifier = 0;
          af.location = 0;
          af.bitvector = 0;
          af.bitvector2 = 0;
          affect_to_char(uber, &af);
          act("$n becomes enveloped in a swirling cloud of darkness!", TRUE, uber, 0, 0, TO_ROOM);
        }
      }
      break;
    case CMD_SONG:
      if (ch && IS_MORTAL(ch) && chance(20) && ch->equipment[HOLD]->obj_flags.type_flag == ITEM_MUSICAL) {
        do_say(uber, "Tatyana, is that you!?! You always had the most beautiful voice; at last, time and circumstance have brought you back to me!", CMD_SAY);
        sprintf(buf, "%s swirls %s cloak around %s and %s disappears.", GET_SHORT(uber), HSHR(uber), GET_NAME(ch), HSSH(ch) );
        act("$n swirls $s cloak around $N and they both disappear.  $n returns immediately, alone.",0,uber,0,ch,TO_NOTVICT);
        act("$n swirls $s cloak around you and your surroundings vanish.",0,uber,0,ch,TO_CHAR);
        if (GET_SEX(ch) == SEX_MALE) {
          //trans to The Torture Chamber
          do_shout(uber, "What - you're a man?!? You disgusting catfish, I'll deal with you later!", CMD_SHOUT);
          stop_fighting(ch);
          char_from_room(ch);
          char_to_room(ch, real_room(STRAHD_TORTURECHAMBER));
          do_look(ch, "", CMD_LOOK);
          WAIT_STATE(ch, PULSE_VIOLENCE*5);
        } else {
          //protect the lady, trans to bedchamber and lock door
          do_shout(uber, "Nothing will get between us this time my love, I will keep you safe.", CMD_SHOUT);
          stop_fighting(ch);
          char_from_room(ch);
          char_to_room(ch, real_room(STRAHD_BEDCHAMBER));
          if ( !IS_SET( world[real_room(STRAHD_BEDCHAMBER)].dir_option[EAST]->exit_info, EX_CLOSED ) ) {
            //if door isn't closed, close it
            SET_BIT(world[real_room(STRAHD_BEDCHAMBER)].dir_option[EAST]->exit_info, EX_CLOSED);
          }
          if ( !IS_SET( world[real_room(STRAHD_BEDCHAMBER)].dir_option[EAST]->exit_info, EX_LOCKED ) ) {
            //if door isn't locked, lock it
            SET_BIT(world[real_room(STRAHD_BEDCHAMBER)].dir_option[EAST]->exit_info, EX_LOCKED);
          }
          //now from the other side
          if ( !IS_SET( world[real_room(STRAHD_SPAWNROOM)].dir_option[WEST]->exit_info, EX_CLOSED ) ) {
            //if door isn't closed, close it
            SET_BIT(world[real_room(STRAHD_SPAWNROOM)].dir_option[WEST]->exit_info, EX_CLOSED);
          }
          if ( !IS_SET( world[real_room(STRAHD_SPAWNROOM)].dir_option[WEST]->exit_info, EX_LOCKED ) ) {
            //if door isn't locked, lock it
            SET_BIT(world[real_room(STRAHD_SPAWNROOM)].dir_option[WEST]->exit_info, EX_LOCKED);
          }
          do_look(ch, "", CMD_LOOK);
          WAIT_STATE(ch, PULSE_VIOLENCE*2);
        }
      }
      break;
    case CMD_CAST:
      if (ch && IS_MORTAL(ch) && chance(50)) {
        if (chance(25)) do_say(uber, "Aha, I too have a taste for magic!", CMD_SAY);
        act("$n moves with blinding speed and is upon $N instantly, sinking $s fangs deep into $N's neck.",0,uber,0,ch,TO_NOTVICT);
        act("You move with blinding speed and descend upon $N instantly, sinking your fangs deep into $S neck.",0,uber,0,ch,TO_CHAR);
        act("The last syllable of your spell barely leaves your lips before $n descends upon you and sinks $s fangs deep into your neck!",0,uber,0,ch,TO_VICT);
        drain = number (100,185);
        //ignore sanc/orb_of_protection etc.; do 'drain' dmg, steal 50 mana, and 0 mv
        drain_mana_hit_mv(uber, ch, 50, drain, 0, TRUE, TRUE, FALSE);
        //at night restore additional HP to strahd
        if(IS_NIGHT) GET_HIT(uber) = MIN( GET_MAX_HIT(uber), GET_HIT(uber) + 250);
      }
      break;
    case CMD_ASSIST:
      //if using "assist" command, and 'bewitched' (CHARMed), assist Strahd instead
      if (ch && !ch->specials.fighting && affected_by_spell(ch, SPELL_CHARM_PERSON) && uber->specials.fighting) {
        send_to_char("You're right, the handsome Count needs your help, attack!\n\r", ch);
        vict = uber->specials.fighting;
        hit(ch,vict,TYPE_HIT);
        return TRUE;
      }
      break;
    case MSG_MOBACT:
      if (!uber->specials.fighting) break;
      if (uber && form == 0 && ( GET_HIT(uber) < ( GET_MAX_HIT(uber) / 20 ) ) ) {
        nameless = read_mobile(NAMELESS_ONE, VIRTUAL);
        if (nameless) {
          char_to_room(nameless, CHAR_REAL_ROOM(uber));
          sprintf(buf, "NOOOOOOOOOOO! %s, we struck an accord, the pact was signed in blood, this cannot end now!", GET_SHORT(nameless));
          do_yell(uber, buf, CMD_YELL);
          sprintf(buf, "The air ripples and suddenly an enormous black shadow is here, it extends a withered skeletal hand toward %s.\n\r", GET_SHORT(uber));
          send_to_room(buf, CHAR_REAL_ROOM(uber));
          sprintf(buf, "%s's wounds ooze putrid black oil for a moment before reforming anew.\n\r%s seems to be completely healed!\n\n\r", GET_SHORT(uber), GET_SHORT(uber));
          send_to_room(buf, CHAR_REAL_ROOM(uber));
          do_shout(nameless, "`bNames have power, and you have used mine Strahd, do not dare use it again or I will deliver you an eternity of torment.", CMD_SHOUT);
          act("$n disappears in an explosion of putrid black gunk.", TRUE, nameless, 0, 0, TO_ROOM);
          for(vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
          {
            next_vict = vict->next_in_room;
            if ( IS_NPC(vict) || !IS_MORTAL(vict) ) continue;
            damage(nameless,vict,number(200,800),TYPE_UNDEFINED,DAM_POISON);
            stop_fighting(vict);
          }
          extract_char(nameless);
          GET_HIT(uber) = GET_MAX_HIT(uber);
          form = 1;
        }
      }
      num_specs = number(0,2);
      if (IS_NIGHT) num_specs++;
      if (form == 1) num_specs++;
      if ((affected_by_spell(uber, SPELL_DISRUPT_SANCT) || !IS_SET( uber->specials.affected_by, AFF_SANCTUARY)) && !affected_by_spell(uber, SPELL_SANCTUARY)) {
        sprintf(buf, "%s growls angrily, frustratingly exposed and vulnerable here in %s sanctuary.\n\r", GET_SHORT(uber), HSHR(uber));
        send_to_room(buf, CHAR_REAL_ROOM(uber));
        num_specs+=2;
      }
      bool charming_msg = TRUE;
      for (int i = 0; i < num_specs; i++) {
        switch (number(0,9)) {
          case 0:
          case 1:
          //vampire wiles: charms target, if already charmed: characters attempt to hand over a piece of gear to Strahd
            num_charms = number(1,2);
            if (form == 1) num_charms++;
            if (charming_msg) {
              sprintf(buf, "The whites of %s's eyes darken, and an intense wave of dazzling charisma exudes from %s.\n\r", GET_SHORT(uber), HMHR(uber));
              send_to_room(buf, CHAR_REAL_ROOM(uber));
              charming_msg = FALSE;
            }

            for (int j = 0; j < num_charms; j++)
            {
              vict = get_random_victim(uber);
              if (vict && IS_MORTAL(vict) && ( GET_CON(vict) < 20 || chance(25) ) ) {
                if(!affected_by_spell( vict, SPELL_CHARM_PERSON )) {
                  af.type = SPELL_CHARM_PERSON;
                  af.location = 0;
                  af.duration = 1;
                  af.modifier = 0;
                  af.bitvector  = AFF_CHARM;
                  af.bitvector2 = 0;
                  affect_to_char(vict, &af);
                  act("You're bewitched by $n's charms.",0,uber,0,vict,TO_VICT);
                  act("$N is bewitched by $n's charms.",0,uber,0,vict,TO_NOTVICT);
                } else {
                  //victim is already charmed, time to give away an item
                  eq_slot = number(0, MAX_WEAR-1);
                  if ( EQ(vict, eq_slot) ) {
                    charm_obj = unequip_char(vict, eq_slot);
                    obj_to_char(charm_obj, uber);
                    check_equipment(vict);
                    save_char(vict, NOWHERE);
                    act("$N sighs longingly, then hastily removes $p and gives it to $n in adoration.",0,uber,charm_obj,vict,TO_NOTVICT);
                    act("$N sighs longingly, then hastily removes $p and gives it to you in adoration.",0,uber,charm_obj,vict,TO_CHAR);
                    act("You sigh longingly, then hastily remove $p and give it to $n in adoration.",0,uber,charm_obj,vict,TO_VICT);
                    sprintf(buf, "WIZINFO: %s gave %s to %s in %d", GET_NAME(vict), OBJ_SHORT(charm_obj), GET_SHORT(uber), V_ROOM(vict) );
                    log_s(buf);
                  }
                }
                stop_fighting(vict);
              }
            }
            break;
          case 2:
          //sleep & impy a target (non-tank) for 1 tick
            vict = get_random_victim(uber);
            if (vict && IS_MORTAL(vict) && vict != uber->specials.fighting) {
              sprintf(buf, "whisper %s This is a taste of the afterlife, a whiff of existence on the other side; to be gone from here and in an eternal slumber where none of your friends can reach you.", GET_NAME(vict));
              mob_do(uber, buf);
              if(!affected_by_spell( vict, SPELL_SLEEP )) {
                af.type = SPELL_SLEEP;
                af.location = 0;
                af.duration = 0;
                if (form == 1) af.duration++;
                af.modifier = 0;
                af.bitvector  = 0;
                af.bitvector2 = 0;
                affect_to_char(vict, &af);
              }
              if(!affected_by_spell( vict, SPELL_IMP_INVISIBLE )) {
                af.type = SPELL_IMP_INVISIBLE;
                af.location = 0;
                af.duration = 0;
                if (IS_NIGHT) af.duration++;
                af.modifier = 0;
                af.bitvector  = 0;
                af.bitvector2 = 0;
                affect_to_char(vict, &af);
                act("$n disappears without a trace, as if $e never existed.", TRUE, vict,0,0,TO_ROOM);
              }
              stop_fighting(vict);
              GET_POS(vict) = POSITION_SLEEPING;
            }
            break;
          case 3:
            //summon super-zombie: stun spec and "brain eating" ( chill touch with massive -INT )
            zombie_count = 0;
            for(vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
            {// count zombies
              next_vict = vict->next_in_room;
              if (IS_NPC(vict) && V_MOB(vict)==SUPER_ZOMBIE) zombie_count++;
            }
            if(zombie_count<3)
            {
              do_yell(uber, "Arise my friend, we have guests who need your attention!", CMD_YELL);
              zombie = read_mobile(SUPER_ZOMBIE, VIRTUAL);
              if (zombie) char_to_room(zombie, CHAR_REAL_ROOM(uber));
              vict = get_random_victim(uber);
              if (vict && IS_MORTAL(vict)) hit(zombie,vict,TYPE_HIT);
            }
            break;
          case 4:
          case 5:
          case 6:
          //room damage & blind & -5 STR
            act("$n sends a wave of dark energy swirling through the room, sapping light and strength from the world.",0,uber,0,0,TO_ROOM);
            act("You send a wave of dark energy into the fray.",0,uber,0,0,TO_CHAR);
            for( vict=world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict )
            {
              next_vict = vict->next_in_room;
              if( vict == uber || affected_by_spell( vict, SPELL_SLEEP )) continue;
              if( ( !IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM ) || ( IS_NPC(vict) && V_MOB(vict)==SUPER_ZOMBIE ) ) continue;
              damage(uber, vict, number(100,300), TYPE_UNDEFINED, DAM_NO_BLOCK);
              if(!affected_by_spell( vict, SPELL_BLINDNESS )) {
                af.type = SPELL_BLINDNESS;
                af.duration = 1;
                if (IS_NIGHT) af.duration++;
                af.location = APPLY_STR;
                af.modifier = -5;
                af.bitvector  = AFF_BLIND;
                af.bitvector2 = 0;
                affect_to_char(vict, &af);
              }
            }
            break;
          case 7:
          case 8:
            //dispel some folk
            num_dispels = 1;
            if (IS_NIGHT) num_dispels++;
            if (form == 1) num_dispels++;
            for (int j = 0; j < num_dispels; j++)
            {
              vict = get_random_victim(uber);
              if (vict && IS_MORTAL(vict) && !affected_by_spell( vict, SPELL_CHARM_PERSON ) && !affected_by_spell( vict, SPELL_SLEEP ) && !affected_by_spell( vict, SPELL_CHILL_TOUCH )) {
              //don't dispel people that are: bewitched, sleep-impy, or brain-eaten
                act("$n makes a contemptuous gesture in $N's direction while mumbling an incantation, 'tizsiztsalis megis a gyengeketum'.",0,uber,0,vict,TO_NOTVICT);
                act("You make a contemptuous gesture in $N's direction while mumbling an incantation, 'tizsiztsalis megis a gyengeketum'.",0,uber,0,vict,TO_CHAR);
                act("$n makes a contemptuous gesture in your direction while mumbling an incantation, 'tizsiztsalis megis a gyengeketum'.",0,uber,0,vict,TO_VICT);
                spell_disenchant( GET_LEVEL(uber), uber, vict, 0 );
              }
            }
            break;
          case 9:
            //trans someone to Barovia
            tele_room = real_room( number(BAVARIA_BOTTOM, BAVARIA_TOP) );
            vict = get_random_victim(uber);
            if (vict && IS_MORTAL(vict) && tele_room) {
              do_say(uber, "You bore me, go bother the village folk.", CMD_SAY);
              act("$n glances piteously at $N, who disappears.",0,uber,0,vict,TO_NOTVICT);
              act("You glances piteously at $N, who disappears.",0,uber,0,vict,TO_CHAR);
              act("$n glances piteously at you, and suddenly you're elsewhere.",0,uber,0,vict,TO_VICT);
              stop_fighting(vict);
              char_from_room(vict);
              char_to_room(vict, tele_room);
              do_look(vict, "", CMD_LOOK);
            }
            break;
          default:
            break;
        }
      }
      break;
    case MSG_DIE:
      // on boss death reward AQP
      sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
      send_to_room(buf, CHAR_REAL_ROOM(uber));
      for(vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if ( IS_NPC(vict) || !IS_MORTAL(vict) ) continue;
        int reward = 10;
        sprintf(buf, "You are awarded with %d quest %s for the kill.\n\r", reward, reward > 1 ? "points" : "point");
        send_to_char(buf, vict);
        vict->ver3.quest_points += reward;
      }
      break;
    default:
      break;
  }
  return FALSE;
}


int super_zombie(CHAR *zombie, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL, *next_vict = NULL, *vict1 = NULL, *vict2 = NULL;
  char buf[MAX_STRING_LENGTH];
  struct affected_type_5 af;

  if(cmd == MSG_MOBACT && zombie->specials.fighting) {
    switch (number(0,5)) {
      case 0:
        //stun two people
        vict1=get_random_victim(zombie);
        vict2=get_random_victim(zombie);

        int failcount = 0;
        while(failcount < 10 && (vict2==vict1))
        {
          vict2=get_random_victim(zombie);
          failcount++;
        }

        if(vict1 != vict2 && failcount < 10) {
          for(vict = world[CHAR_REAL_ROOM(zombie)].people; vict; vict = next_vict)
          {
            next_vict = vict->next_in_room;
            if ( IS_NPC(vict) ) continue;
            //this is a little awkward here, no clean way for messaging the different people appropriately, writing our own logic rather than use act()
            if (vict != vict1 && vict != vict2 && vict != zombie) {
            //result for witnesses
              sprintf(buf, "%s lifts %s off the floor in one swift movement, then proceeds to bludgeon %s repeatedly with its human cudgel.\n\r", GET_SHORT(zombie), GET_NAME(vict1), GET_NAME(vict2));
              send_to_char(buf, vict);
            } else if (vict == vict1) {
            //result for "bludgeoner"
              sprintf(buf, "%s lifts you off the floor in one swift movement, then proceeds to bludgeon %s, swinging you around like a human cudgel.\n\r", GET_SHORT(zombie), GET_NAME(vict2));
              send_to_char(buf, vict);
              damage(zombie,vict,number(200,400),TYPE_UNDEFINED,DAM_PHYSICAL);
              stop_fighting(vict);
              WAIT_STATE(vict, 2*PULSE_VIOLENCE);
            } else if (vict == vict2) {
            //result for "the bludgeoned"
              sprintf(buf, "%s lifts %s off the floor in one swift movement, then proceeds to bludgeon you with %s body, swinging %s like a human cudgel.\n\r", GET_SHORT(zombie), GET_NAME(vict1), HSHR(vict1), HMHR(vict1));
              send_to_char(buf, vict);
              damage(zombie,vict2,number(400,1000),TYPE_UNDEFINED,DAM_PHYSICAL);
              WAIT_STATE(vict, 3*PULSE_VIOLENCE);
            } else {
            //result for zombie
              sprintf(buf, "You lift %s off the floor in one swift movement, swing %s like a human cudgel, then proceed to bludgeon %s.\n\r", GET_NAME(vict1), HMHR(vict1), GET_NAME(vict2));
              send_to_char(buf, vict);
            }
          }
        }
        break;
      case 1:
      case 2:
        //eat someone's brains
        vict = get_random_victim(zombie);
        if(vict && IS_MORTAL(vict)) {
          if(chance(25)) do_shout(zombie, "BRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAINS!", CMD_SHOUT);
          act("$n lunges forward with surprising agility, puts $N in a headlock, and calmly begins to eat $S brains.",0,zombie,0,vict,TO_NOTVICT);
          act("You lunge forward with surprising agility, put $N in a headlock, and calmly begin to eat $S brains.",0,zombie,0,vict,TO_CHAR);
          act("$n lunges forward with surprising agility, puts you in a headlock, and calmly begins to eat your brains.\n\rYou feel... weird.",0,zombie,0,vict,TO_VICT);
          if(!affected_by_spell( vict, SPELL_CHILL_TOUCH )) {
            af.type = SPELL_CHILL_TOUCH;
            af.duration = 2;
            af.location = APPLY_INT;
            af.modifier = -10;
            af.bitvector  = 0;
            af.bitvector2 = 0;
            affect_to_char(vict, &af);
          }
          damage(zombie,vict,number(200,500),TYPE_UNDEFINED,DAM_NO_BLOCK);
        }
        break;
      case 3:
        //do nothing
      case 4:
      case 5:
      default:
        break;
    }
  }
  return FALSE;
}


int uber_bom(OBJ *obj, CHAR *ch, int cmd, char *argument)
{
  if (cmd == MSG_TICK) {
    if (obj->equipped_by) {
      if (GET_HIT(obj->equipped_by)>-1) {
        //bom: range of 8-13 "hidden" regen instead of flat 10, doubles in CLUB, +5 in DARK
        int regen = number(8,13);
        if (IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(obj->equipped_by)), CLUB)) regen*=2;
        if (IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(obj->equipped_by)), DARK)) regen+=5;
        GET_HIT(obj->equipped_by) = MIN( GET_MAX_HIT(obj->equipped_by),
        GET_HIT(obj->equipped_by)+regen);
        GET_MANA(obj->equipped_by) = MIN( GET_MAX_MANA(obj->equipped_by),
        GET_MANA(obj->equipped_by)+regen);
      }
    }
  }
  return FALSE;
}

int strahd_pentacle(OBJ *pentacle, CHAR *ch, int cmd, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  struct affected_type_5 af;
  CHAR *zombie = NULL;

  //at sunset, iterate the OBJ_SPEC, mark that the sunset hour was already counted with obj_flags.timer boolean
  ch = pentacle->equipped_by;
  if (cmd == MSG_TICK && ch) {
    if (time_info.hours == 21) {
      if (pentacle->obj_flags.timer == 0) {
        if (OBJ_SPEC(pentacle) < 20) {
          send_to_char("A pulse of cold energy surges from the pentacle as the sun sets.\n\r", ch);
          OBJ_SPEC(pentacle) = MIN(20, OBJ_SPEC(pentacle) + 1);
        } else {
          send_to_char("Twilight falls, but the pentacle cannot contain any more of the dusk's power.\n\r", ch);
        }
        pentacle->obj_flags.timer = 1;
      }
    } else {
      //any other hour than sunset, reset the boolean "flag" to 0
      if (pentacle->obj_flags.timer != 0) {
        pentacle->obj_flags.timer = 0;
      }
    }
  }

  if (cmd == CMD_USE && ch) {
    one_argument(argument, buf);
    if (isname(buf, OBJ_NAME(pentacle)) && (pentacle == EQ(ch, HOLD))) {
      if (OBJ_SPEC(pentacle) >= 10) {
        if (count_mob_followers(ch) > 1) {
          send_to_char("You don't seem capable of controlling more denizens of the dead at this time.\n\r",ch);
        } else {
          OBJ_SPEC(pentacle)-=10;
          act("A blue unearthly light imbues $n's pentacle as $e channels the power of the dead.",0,ch,0,NULL,TO_ROOM);
          act("Your pentacle glows with an unearthly blue light and becomes ice cold as you channel the power of the dead.",0,ch,0,NULL,TO_CHAR);
          zombie = read_mobile(NOTSOSUPER_ZOMBIE, VIRTUAL);
          char_to_room(zombie, CHAR_REAL_ROOM(ch));
          send_to_room("An incredibly dimwitted zombie appears from nowhere, eager to serve its master.\n\r",CHAR_REAL_ROOM(ch));
          add_follower(zombie, ch);

          af.type        = SPELL_CHARM_PERSON;
          af.duration    = 5;
          af.modifier    = 0;
          af.location    = 0;
          af.bitvector   = AFF_CHARM;
          af.bitvector2  = 0;
          affect_to_char(zombie, &af);
        }
      } else {
        send_to_char("How does one use a pentacle that hasn't been enchanted by the twilight?\n\r", ch);
      }
      return TRUE;
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
  assign_mob(SUPER_ZOMBIE, super_zombie);
  assign_mob(UBER_STRAHD, uber_strahd);

  assign_obj(BAR_SWORD   ,  black_rose_sword);
  assign_obj(BAR_BRACELET, bracelet_of_magic);
  assign_obj(29612       , bracelet_of_magic);
  assign_obj(BAR_DECAY1  ,  decaying_eq);
  assign_obj(BAR_DECAY2  ,  decaying_eq);
  assign_obj(BAR_DECAY3  ,  decaying_eq);
  assign_obj(BAR_DECAY4  ,  decaying_eq);
  assign_obj(BAR_DECAY5  ,  decaying_eq);
  assign_obj(BAR_DECAY6  ,  decaying_eq);
  assign_obj(BAR_DECAY7  ,  decaying_eq);
  assign_obj(UBER_BOM, uber_bom);
  assign_obj(PENTACLE, strahd_pentacle);
}
