/*
///   Special Procedure Module                   Orig. Date 15-11-1994
///
///   spec.quests.c --- Specials for quest items
///
///   Using this code is not allowed without permission from originator.
*/

/*
$Author: ronin $
$Date: 2005/01/21 14:55:29 $
$Header: /home/ronin/cvs/ronin/spec.quests.c,v 2.2 2005/01/21 14:55:29 ronin Exp $
$Id: spec.quests.c,v 2.2 2005/01/21 14:55:29 ronin Exp $
$Name:  $
$Log: spec.quests.c,v $
Revision 2.2  2005/01/21 14:55:29  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.1  2004/10/06 12:15:50  ronin
Changed ugly fur coat to spec more often, but with less damage.

Revision 2.0.0.1  2004/02/05 16:11:12  ronin
Reinitialization of cvs archives

Revision 1.3  2002/05/29 04:56:54  ronin
Addition of whapmaster spec.

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
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"
#include "cmd.h"
#include "act.h"
#include "spec_assign.h"

extern int CHAOSMODE;
void stop_fighting (CHAR *);
void hit (CHAR *, CHAR *, int);
void do_look(CHAR *ch, char *argument, int cmd);
char *one_argument(char *, char *);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);


int Sleeves_of_Healing (OBJ *sleeves, CHAR *ch, int cmd, char *arg) ;
#define QUEST_SLEEVES 27

int Sleeves_of_Healing (OBJ *sleeves, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  int Bool = FALSE;

  switch(cmd)
    {
    case CMD_GRAB:
      if(!ch)
         return FALSE;
      while((*arg == ' ')) arg++;
      if (sleeves == EQ(ch, WEAR_ARMS))
        {
        if ((vict = get_char_room_vis(ch,arg)))
          {
          if(!OBJ_SPEC(sleeves))
            { /* Works */
  	    act("$n reaches out and grabs $N, the $p glow brightly."
	      ,FALSE, ch, sleeves, vict,TO_NOTVICT);
	    act("You reach out and grab $N, your $p glow brightly."
	      ,FALSE, ch, sleeves, vict,TO_CHAR);
	    act("$n grabs you, and $s $p glow brightly."
	      ,FALSE, ch, sleeves, vict,TO_VICT);
            cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            OBJ_SPEC(sleeves) = 10;
            }
          else
            { /* Doesnt work */
   	    act("$n reaches out and grabs $N. Not surprisingly, nothing happens."
	      ,FALSE, ch, sleeves, vict,TO_NOTVICT);
	    act("You reach out and grab $N. Not surprisingly, nothing happens."
	      ,FALSE, ch, sleeves, vict,TO_CHAR);
	    act("$n grabs you, not surprisingly, nothing happens."
	      ,FALSE, ch, sleeves, vict,TO_VICT);
            }
          }
        else
          {
  	  send_to_char("They arent here.\n\r", ch);
          }
          Bool = TRUE;
        }
        break;
    case MSG_TICK:
      if(sleeves->equipped_by)
        {
        if (sleeves == EQ(sleeves->equipped_by, WEAR_ARMS))
          {
          if(OBJ_SPEC(sleeves))
              OBJ_SPEC(sleeves)--;
          else
              {
              GET_HIT(sleeves->equipped_by)+=5;
              if(!number(0,4))
                {
      	        act("$n's $p flash for a brief moment."
  	          ,FALSE, sleeves->equipped_by, sleeves, NULL,TO_NOTVICT);
	        act("Your $p flash for a brief moment."
	          ,FALSE, sleeves->equipped_by, sleeves, NULL,TO_CHAR);
                }
              }
            }
          }
        break;
    }
  return Bool;
}


/* spec.mk4.c
** A part of RoninMUD
**
** Special Procedure Module for
** Mortal Kombat 4
** Written by: Ranger
** Ideas by: Nosferatu
**/


#define HALF_MANA  12621
#define NO_SKILLS  12623
#define HALF_FLEE  12624
#define NO_CAST    12629
#define HALF_MOVE  12631
#define HALF_CONCE  12632
#define ZERO_REGEN 12633
#define HALF_HPS   12635

/* Notes
  1/2 flee only works if the player types flee
  1/2 conc will stop the cast but not take any mana
  zero_regen may allow a tiny amount of regen if the stat is near 0
  1/2 mana, hps and move limit the players regen to 1/2 nat + whatever gear
*/

int mk4_half_flee(OBJ *obj,CHAR *ch,int cmd,char *arg) {
  if(!ch) return FALSE;
  if(ch!=obj->equipped_by) return FALSE;
  if(cmd!=CMD_FLEE) return FALSE;
  if(number(0,1)) {
    act("Your $p prevents you from fleeing.",0,ch,obj,0,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

int mk4_half_conc(OBJ *obj,CHAR *ch,int cmd,char *arg) {
  if(!ch) return FALSE;
  if(ch!=obj->equipped_by) return FALSE;
  if(cmd!=CMD_CAST) return FALSE;
  if(number(0,1)) {
    act("Your $p makes you lose your concentration.",0,ch,obj,0,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

int mk4_no_cast(OBJ *obj,CHAR *ch,int cmd,char *arg) {
  if(!ch) return FALSE;
  if(ch!=obj->equipped_by) return FALSE;
  if(cmd==CMD_CAST) {
    act("Your $p prevents a cast of any kind.",0,ch,obj,0,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

int mk4_no_skills(OBJ *obj,CHAR *ch,int cmd,char *arg) {
  if(!ch) return FALSE;
  if(ch!=obj->equipped_by) return FALSE;
  if(cmd==CMD_PUMMEL        ||
     cmd==CMD_CIRCLE        ||
     cmd==CMD_PUNCH         ||
     cmd==CMD_PEEK          ||
     cmd==CMD_KNOCK         ||
     cmd==CMD_SUBDUE        ||
     cmd==CMD_DISARM        ||
     cmd==CMD_TRAP          ||
     cmd==CMD_BUTCHER       ||
     cmd==CMD_THROW         ||
     cmd==CMD_BLOCK         ||
     cmd==CMD_SNEAK         ||
     cmd==CMD_HIDE          ||
     cmd==CMD_BACKSTAB      ||
     cmd==CMD_PICK          ||
     cmd==CMD_STEAL         ||
     cmd==CMD_BASH          ||
     cmd==CMD_RESCUE        ||
     cmd==CMD_KICK          ||
     cmd==CMD_ASSAULT       ||
     cmd==CMD_SPIN          ||
     cmd==CMD_DISEMBOWEL    ||
     cmd==CMD_BACKFLIP ) {
    act("Your $p prevents any kind of skill.",0,ch,obj,0,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

int mk4_zero_regen(OBJ *obj,CHAR *chtmp,int cmd,char *arg) {
  CHAR *ch;
  if(!(ch=obj->equipped_by)) return FALSE;
  if(cmd==MSG_TICK) {
    GET_HIT (ch) = MAX(1, GET_HIT (ch) - hit_gain(ch));
    GET_MANA(ch) = MAX(1, GET_MANA(ch) - mana_gain(ch));
    GET_MOVE(ch) = MAX(1, GET_MOVE(ch) - move_gain(ch));
  }
  return FALSE;
}

int mk4_half_mana(OBJ *obj,CHAR *chtmp,int cmd,char *arg) {
  int limit;
  CHAR *ch;
  if(!(ch=obj->equipped_by)) return FALSE;
  if(cmd==MSG_MOBACT) {
    limit=GET_MAX_MANA(ch)-ch->points.max_mana/2-mana_gain(ch);
    GET_MANA(ch)=MIN(GET_MANA(ch),limit);
  }
  return FALSE;
}

int mk4_half_hps(OBJ *obj,CHAR *chtmp,int cmd,char *arg) {
  int limit;
  CHAR *ch;
  if(!(ch=obj->equipped_by)) return FALSE;
  if(cmd==MSG_MOBACT) {
    limit=GET_MAX_HIT(ch)-ch->points.max_hit/2-hit_gain(ch);
    GET_HIT(ch)=MIN(GET_HIT(ch),limit);
  }
  return FALSE;
}

int mk4_half_move(OBJ *obj,CHAR *chtmp,int cmd,char *arg) {
  int limit;
  CHAR *ch;
  if(!(ch=obj->equipped_by)) return FALSE;
  if(cmd==MSG_MOBACT) {
    limit=GET_MAX_MOVE(ch)-ch->points.max_move/2-move_gain(ch);
    GET_MOVE(ch)=MIN(GET_MOVE(ch),limit);
  }
  return FALSE;
}

#define UGLY_COAT  6958
#define TACKY_BELT 6959
#define PURPLE_HAT 6960

int tacky_belt(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  CHAR *vic;
  char arg[MAX_STRING_LENGTH];

  if(cmd != CMD_KILL) return FALSE;
  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(ch!=obj->equipped_by) return FALSE;

  if(obj->equipped_by->specials.fighting) return FALSE;

  one_argument(argument, arg);
  if(!*arg) return FALSE;

  if (!(vic = get_char_room_vis(ch, arg)))
   return FALSE;

  if(!IS_NPC(vic) && !IS_NPC(ch)) {
    send_to_char("Your belt refuses to attack other players !\n\r",ch);
    return TRUE;
  }

  if(number(0,29)) return FALSE;

  act("$N is blinded by the bright glow of $n's hideously tacky gold belt!",FALSE,ch,0,vic,TO_ROOM);
  act("A blinding light streams from your belt!\n\r",FALSE,ch,0,vic,TO_CHAR);
  act("$n blinds you with $s tacky gold belt!\n\r",FALSE,ch,0,vic,TO_VICT);
  spell_blindness(30, ch, vic, 0);

  return FALSE;
}

int purple_hat(OBJ *obj, CHAR *ch, int cmd, char *argument) {

  if(cmd != MSG_TICK) return FALSE;

  if(!obj->equipped_by) return FALSE;

  if(number(0,29)) return FALSE;

  act("$n adjusts $s bright purple plumed hat and blinks once.",FALSE,obj->equipped_by,0,0,TO_ROOM);
  act("You fiddle with your hat and blink.",FALSE,obj->equipped_by,0,0,TO_CHAR);
  spell_cure_blind(30, obj->equipped_by, obj->equipped_by, 0);
  return FALSE;
}

int ugly_coat(OBJ *obj, CHAR *ch, int cmd, char *argument) {

  if(cmd != MSG_TICK) return FALSE;
  if(!obj->equipped_by) return FALSE;
  if(!obj->equipped_by->specials.fighting) return FALSE;

  if(number(0,19)) return FALSE;

  act("$N looks at $n's hideously ugly coat and trembles in terror.",FALSE,obj->equipped_by,0,obj->equipped_by->specials.fighting,TO_NOTVICT);
  act("$N looks at your hideously ugly coat and trembles in terror.",FALSE,obj->equipped_by,0,obj->equipped_by->specials.fighting,TO_CHAR);
  act("You look at $n's hideously ugly coat and tremble in terror.",FALSE,obj->equipped_by,0,obj->equipped_by->specials.fighting,TO_VICT);
  damage(obj->equipped_by, obj->equipped_by->specials.fighting, 20, TYPE_UNDEFINED,DAM_NO_BLOCK);
  return FALSE;
}

int hemp_farmers(CHAR *mob,CHAR *ch,int cmd, char *argument) {
  char arg[MAX_INPUT_LENGTH];
  CHAR *vict=0;
  OBJ *obj=0;
  if(cmd!=CMD_UNKNOWN) return FALSE;
  if(!ch) return FALSE;

  argument = one_argument (argument, arg);
  if(!*arg) return FALSE;
  if(strcmp(arg,"hug")) return FALSE;
  one_argument(argument,arg);
  if(!*arg) return FALSE;
  generic_find(arg,FIND_CHAR_ROOM,ch,&vict,&obj);
  if(!vict) return FALSE;
  if(vict!=mob) return FALSE;
  obj=read_object(V_MOB(mob),VIRTUAL);
  if(!obj) return FALSE;
  obj_to_char(obj,ch);
  act("$n hugs $N and gets a card!",0,ch,0,mob,TO_NOTVICT);
  act("You hug $N and get a card!",0,ch,0,mob,TO_CHAR);
  act("$n hugs you and gets a card!",0,ch,0,mob,TO_VICT);
  return TRUE;
}

#define XALTH_MOB1 18910
#define XALTH_MOB2 18909

int xalth_mob(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict=0,*tmp;
  OBJ *obj;
  int i,to_room;
  extern int top_of_world;

  if(cmd==MSG_TICK) { /* mob beam routine */
    if(chance(66)) return FALSE;
    do {
      to_room = number(0, top_of_world);
    } while (IS_SET(world[to_room].room_flags, PRIVATE) ||
             IS_SET(world[to_room].room_flags, CHAOTIC) ||
             IS_SET(world[to_room].room_flags, NO_BEAM) ||
             (IS_NPC(ch) && IS_SET(world[to_room].room_flags, SAFE)) ||
             (IS_NPC(ch) && IS_SET(world[to_room].room_flags, DEATH)) ||
             (IS_NPC(ch) && IS_SET(world[to_room].room_flags, HAZARD)) ||
             IS_SET(world[to_room].room_flags, LOCK));

    if(V_MOB(mob)==XALTH_MOB1)
      act("$n says 'You fools will never catch me.", FALSE,mob,0,0,TO_ROOM);
    else
      act("$n cackles loudly.", FALSE,mob,0,0,TO_ROOM);
    act("$n disappears in a puff of smoke.", FALSE,mob,0,0,TO_ROOM);

    char_from_room(mob);
    if(mob->specials.fighting)
      stop_fighting(mob);
    char_to_room(mob, to_room);
    act("$n slowly fades into existence.", FALSE, mob,0,0,TO_ROOM);
    do_look(mob, "", CMD_LOOK);
  } /* end of beam routine */

  if(V_MOB(mob)==XALTH_MOB1) { /* block of certain attacks */
    switch(cmd) {
      case CMD_PUMMEL:
      case CMD_CIRCLE:
      case CMD_PUNCH:
      case CMD_BACKSTAB:
      case CMD_BASH:
      case CMD_KICK:
      case CMD_ASSAULT:
      case CMD_SPIN:
      case CMD_DISEMBOWEL:
      case CMD_BACKFLIP:
        act("$n laughs at your pitiful attack attempt.",0,mob,0,ch,TO_VICT);
        return TRUE;
        break;
    }
  } /* end of attack block */

  if(cmd) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(V_MOB(mob)==XALTH_MOB1 && GET_HIT(mob)<2300) {
    vict=read_mobile(XALTH_MOB2,VIRTUAL);
    if(vict) char_to_room(vict,CHAR_REAL_ROOM(mob));
    act("$n cackles gleefully as $e takes the form of $N.", 1, mob, 0, vict, TO_ROOM);
    for(i=0; i<MAX_WEAR; i++) {
      obj=mob->equipment[i];
      if(obj) {
        unequip_char(mob,i);
        equip_char(vict,obj,i);
      }
    }

    /*tank*/
    tmp=mob->specials.fighting;
    if(tmp->specials.fighting==mob) {
      stop_fighting(tmp);
      set_fighting(vict,tmp);
    }

    for(tmp=world[CHAR_REAL_ROOM(mob)].people;tmp;tmp=tmp->next_in_room) {
      if(tmp->specials.fighting==mob) {
        stop_fighting(tmp);
        set_fighting(tmp,vict);
      }
    }
    stop_fighting(mob);
    extract_char(mob);
    return FALSE;
  }

  return FALSE;
}

#define TARION_HELMET 6973
int helmet_tarion(OBJ *ta, CHAR *ch, int cmd, char *arg) {
  CHAR *tmp, *tch;
  char buf[1000];

  if(cmd == MSG_DIE) {
    if(!(tch=ta->equipped_by)) return FALSE;
    if(IS_MOB(tch)) return FALSE;

    if(IS_SET(world[CHAR_REAL_ROOM(tch)].room_flags, CHAOTIC)) return FALSE;

    act ("As $n lies dying, $p explodes in a torrent of blue light!\n\r$p disappears.", TRUE, tch, ta, 0, TO_ROOM);
    send_to_char ("As you are about to expire, the cavalier's helmet explodes around you\n\rand there is only blackness...\n\r",tch);

    GET_HIT (tch) = 5;
    GET_MANA (tch) = 1;
    GET_MOVE (tch) = 5;
    tch->points.max_hit +=11;
    tch->points.max_mana +=11;

    for (tmp = world[CHAR_REAL_ROOM(tch)].people;tmp;tmp=tmp->next_in_room)
      if (GET_OPPONENT(tmp) == tch)
        stop_fighting (tmp);

    sprintf (buf, "%s rescued %s from %s [%d].", OBJ_SHORT(ta),GET_NAME(tch),world[CHAR_REAL_ROOM(tch)].name,CHAR_VIRTUAL_ROOM(tch));
    wizlog (buf, LEVEL_WIZ, 6);
    log_s(buf);

    stop_fighting (tch);
    unequip_char (tch, WEAR_HEAD);
    obj_to_char (ta, tch);
    extract_obj (ta);

    spell_word_of_recall (GET_LEVEL(tch), tch, tch, 0);
    return TRUE;
  }
  return FALSE;
}

#define CUPID_DIAPER 6979

int cupid_diaper(OBJ *diaper, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*user,*next_vict;
  struct affected_type_5 af;

  if(cmd!=MSG_TICK) return FALSE;
  if(!(user=diaper->equipped_by)) return FALSE;

  if(chance(5)) {
    act("A foul smelling green cloud emits from $n's diaper!",0,user,0,0,TO_ROOM);
    act("A foul smelling green cloud emits from your diaper!",0,user,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(user)].people;vict;vict = next_vict) {
      next_vict = vict->next_in_room;
      if( (IS_MOB(vict) && !IS_SET(vict->specials.immune, IMMUNE_POISON)) || (IS_MORTAL(vict) && IS_GROUPED(user,vict))) {
        af.type = SPELL_POISON;
        af.duration = 30;
        af.modifier = -3;
        af.location = APPLY_STR;
        af.bitvector = AFF_POISON;
        af.bitvector2 = 0;
        affect_join(vict, &af, FALSE, FALSE);
        send_to_char("You feel very sick.\n\r", vict);
        damage(vict,vict,number(0,50),TYPE_UNDEFINED,DAM_NO_BLOCK);
      }
    }/*for*/
  }/*chance*/
  return FALSE;
}

int WHAP_SPEC(OBJ *WHAPMASTER, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*user;
  int SPECS, dam, i;

      if(cmd!=MSG_MOBACT) return FALSE;
      if(!(user=WHAPMASTER->equipped_by)) return FALSE;
      if(!user->specials.fighting) return FALSE;
      if(chance(80)) return FALSE;

      act("The Whapmaster erupts in a fury of Whapping!",0,user,0,0,TO_ROOM);
      act("The Whapmaster erupts in a fury of Whapping!",0,user,0,0,TO_CHAR);

      SPECS = number(1,6);
      vict = user->specials.fighting;
      for(i=1;i<=SPECS;i++) {
        dam = number(1,12) + number(1,12) + user->points.damroll;
        damage(user,vict,dam,TYPE_BLUDGEON,DAM_NO_BLOCK);
        if(!vict || CHAR_REAL_ROOM(vict)==NOWHERE)
          break;
      }
  return FALSE;
}


int object_tracking(OBJ *trackobj, CHAR *ch, int cmd, char *arg) {  /* Linerfix 110303 */
  char buf[MAX_INPUT_LENGTH];

  if(!cmd==MSG_OBJ_ENTERING_GAME) return FALSE;

  if(trackobj->log!=2) {
    act("$p will now be tracked.",FALSE,0,trackobj,0,TO_ROOM);
    sprintf(buf,"ObjLog: %s has entered the game.",OBJ_SHORT(trackobj));
    log_s(buf);
    trackobj->log=2;
    return FALSE;
  }
  return FALSE;
}


#define POINTY_HAT 26214

int pointy_hat_spec(OBJ *hat,CHAR *ch, int cmd, char *argument) {
  CHAR *wearer;
  int aphavg=1000, apmavg=1000,
      bahavg=1200, bamavg=1200,
      clhavg=800,  clmavg=2000,
      cohavg=1500, comavg=700,
      muhavg=400,  mumavg=1637,
      nihavg=1450, nimavg=600,
      nohavg=2300, nomavg=100,
      pahavg=1300, pamavg=1200,
      thhavg=1700, thmavg=100,
      wahavg=1873, wamavg=100;
  int diffhp=0, diffmana=0;

  /* if hat is not equipped on Tick, reset hp and mana on it to 0 */
  if(cmd==MSG_TICK && !hat->equipped_by) {
    hat->affected[0].modifier = 0; /* change hp aff to 0 */
    hat->affected[1].modifier = 0; /* change mana aff to 0 */
    return FALSE;
  }

  /* if wearer is not set to the person equipping the hat, stop spec */
  if(!(wearer=hat->equipped_by)) return FALSE;

  /* if there is no wearer pointer, stop spec */
  if(!wearer) return FALSE;

  /* if it's tick and equipped, enter spec */
  if(cmd==MSG_TICK && hat->equipped_by) {

    /* if wearer isn't a god or mortal, stop spec */
    if(IS_NPC(wearer)) return FALSE;

    /* check class of the wearer for conditions */
    switch(GET_CLASS(wearer)) {

      case CLASS_ANTI_PALADIN:
        /* if Ap, get difference in max hp and mana from the Ap avg */
        diffhp = aphavg - GET_MAX_HIT(wearer);
        diffmana = apmavg - GET_MAX_MANA(wearer);
        break;

      case CLASS_BARD:
        diffhp = bahavg - GET_MAX_HIT(wearer);
        diffmana = bamavg - GET_MAX_MANA(wearer);
        break;

      case CLASS_CLERIC:
        diffhp = clhavg - GET_MAX_HIT(wearer);
        diffmana = clmavg - GET_MAX_MANA(wearer);
        break;

      case CLASS_COMMANDO:
        diffhp = cohavg - GET_MAX_HIT(wearer);
        diffmana = comavg - GET_MAX_MANA(wearer);
        break;

      case CLASS_MAGIC_USER:
        diffhp = muhavg - GET_MAX_HIT(wearer);
        diffmana = mumavg - GET_MAX_MANA(wearer);
        break;

      case CLASS_NINJA:
        diffhp = nihavg - GET_MAX_HIT(wearer);
        diffmana = nimavg - GET_MAX_MANA(wearer);
        break;

      case CLASS_NOMAD:
        diffhp = nohavg - GET_MAX_HIT(wearer);
        diffmana = nomavg - GET_MAX_MANA(wearer);
        break;

      case CLASS_PALADIN:
        diffhp = pahavg - GET_MAX_HIT(wearer);
        diffmana = pamavg - GET_MAX_MANA(wearer);
        break;

      case CLASS_THIEF:
        diffhp = thhavg - GET_MAX_HIT(wearer);
        diffmana = thmavg - GET_MAX_MANA(wearer);
        break;

      case CLASS_WARRIOR:
        diffhp = wahavg - GET_MAX_HIT(wearer);
        diffmana = wamavg - GET_MAX_MANA(wearer);
        break;

      default:
        break;
    } /* end switch */

    unequip_char(wearer,WEAR_HEAD); /* remove the thing */

    /* change hp aff to the hp difference added to current hp aff */
    hat->affected[0].modifier = (hat->affected[0].modifier + diffhp);

    /* change mana aff to the mana difference added to current mana aff */
    hat->affected[1].modifier = (hat->affected[1].modifier + diffmana);

    equip_char(wearer,hat,WEAR_HEAD); /* re-wear it */

    return FALSE;

  } /* end if Tick */

  return FALSE;

} /* end of int */

int bracelet_of_magic(OBJ *obj, CHAR *ch, int cmd, char *argument);
#define INFINITY_TATTOO 6976

/* Christmas Quest 2009 specs
** 2009-11-30
** Ideas/Code by: Hemp
*/

#define HEMP_SANTA			26200
#define HEMP_ELF				26201
#define HEMP_REINDEER		26202
#define HEMP_GRINCH			26203

int hemp_santa(CHAR *santa, CHAR *vict, int cmd, char *buf)
{
	int iLevel = 0;
	struct affected_type_5 af;

	if(cmd==MSG_MOBACT && santa->specials.fighting)
	{
		vict = santa->specials.fighting;
		iLevel = GET_LEVEL(vict);
		switch(iLevel)
		{
			case 40:
			case 41:
			case 42:
				if(chance(3))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						if(chance(50))
						{
							sprintf(buf, "%s, I think you belong on the naughty list.", GET_NAME(vict));
							do_say(santa, buf, CMD_SAY);
							GET_ALIGNMENT(vict) = -700;
							check_equipment(vict);
						}
						else
						{
							sprintf(buf, "%s, I think you belong on the nice list.", GET_NAME(vict));
							do_say(santa, buf, CMD_SAY);
							GET_ALIGNMENT(vict) = 700;
							check_equipment(vict);
						}
					}
				}// END: re-align vict
				if(chance(5))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						act("$n swings a sack full of coal at $N's head.", FALSE, santa, NULL, vict, TO_NOTVICT);
						act("You black out as $n's sack of coal smashes into the side of your head!", FALSE, santa, NULL, vict, TO_VICT);
						damage(santa, vict, 225, TYPE_UNDEFINED, DAM_NO_BLOCK);
						WAIT_STATE(vict, PULSE_VIOLENCE * 2);
						if(!affected_by_spell(vict, SPELL_BLINDNESS))
						{
							af.type = SPELL_BLINDNESS;
							af.location = APPLY_HITROLL;
							af.modifier = -4;
							af.bitvector = AFF_BLIND;
							af.bitvector2 = 0;
							af.duration = 8;
							affect_to_char(vict, &af);
						}
					}
				}// END: blind/stun vict
				if(chance(7))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						do_say(santa, "Get back to work elf!", CMD_SAY);
						spell_teleport(GET_LEVEL(santa), vict, NULL, NULL);
					}
				}// END: teleport vict
				break;
			case 43:
			case 44:
			case 45:
			case 46:
				if(chance(7))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						if(chance(50))
						{
							sprintf(buf, "%s, I think you belong on the naughty list.", GET_NAME(vict));
							do_say(santa, buf, CMD_SAY);
							GET_ALIGNMENT(vict) = -850;
							check_equipment(vict);
						}
						else
						{
							sprintf(buf, "%s, I think you belong on the nice list.", GET_NAME(vict));
							do_say(santa, buf, CMD_SAY);
							GET_ALIGNMENT(vict) = 850;
							check_equipment(vict);
						}
					}
				}// END: re-align vict
				if(chance(11))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						act("$n swings a sack full of coal at $N's head.", FALSE, santa, NULL, vict, TO_NOTVICT);
						act("You black out as $n's sack of coal smashes into the side of your head!", FALSE, santa, NULL, vict, TO_VICT);
						damage(santa, vict, 350, TYPE_UNDEFINED, DAM_NO_BLOCK);
						if(GET_POS(vict) > POSITION_STUNNED)
							GET_POS(vict) = POSITION_STUNNED;
						WAIT_STATE(vict, PULSE_VIOLENCE * 2);
						if(!affected_by_spell(vict, SPELL_BLINDNESS))
						{
							af.type = SPELL_BLINDNESS;
							af.location = APPLY_HITROLL;
							af.modifier = -4;
							af.bitvector = AFF_BLIND;
							af.bitvector2 = 0;
							af.duration = 8;
							affect_to_char(vict, &af);
						}
					}
				}// END: blind/stun vict
				if(chance(11))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						do_say(santa, "Get back to work elf!", CMD_SAY);
						spell_teleport(GET_LEVEL(santa), vict, NULL, NULL);
					}
				}// END: teleport vict
				break;
			case 47:
			case 48:
			case 49:
			case 50:
				if(chance(11))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						if(chance(50))
						{
							sprintf(buf, "%s, I think you belong on the naughty list.", GET_NAME(vict));
							do_say(santa, buf, CMD_SAY);
							GET_ALIGNMENT(vict) = -1000;
							check_equipment(vict);
						}
						else
						{
							sprintf(buf, "%s, I think you belong on the nice list.", GET_NAME(vict));
							do_say(santa, buf, CMD_SAY);
							GET_ALIGNMENT(vict) = 1000;
							check_equipment(vict);
						}
					}
				}// END: re-align vict
				if(chance(13))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						act("$n swings a sack full of coal at $N's head.", FALSE, santa, NULL, vict, TO_NOTVICT);
						act("You black out as $n's sack of coal smashes into the side of your head!", FALSE, santa, NULL, vict, TO_VICT);
						damage(santa, vict, 450, TYPE_UNDEFINED, DAM_NO_BLOCK);
						if(vict==santa->specials.fighting && GET_POS(vict) > POSITION_STUNNED)
							GET_POS(vict) = POSITION_STUNNED;
						else if(GET_POS(vict) > POSITION_SITTING)
							GET_POS(vict) = POSITION_SITTING;
						WAIT_STATE(vict, PULSE_VIOLENCE * 3);
						if(!affected_by_spell(vict, SPELL_BLINDNESS))
						{
							af.type = SPELL_BLINDNESS;
							af.location = APPLY_HITROLL;
							af.modifier = -4;
							af.bitvector = AFF_BLIND;
							af.bitvector2 = 0;
							af.duration = 8;
							affect_to_char(vict, &af);
						}
					}
				}// END: blind/stun vict
				if(chance(13))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						do_say(santa, "Get back to work elf!", CMD_SAY);
						spell_teleport(GET_LEVEL(santa), vict, NULL, NULL);
					}
				}// END: teleport vict
				break;
			default:
				if(chance(5))
				{
					vict = get_random_victim_fighting(santa);
					if(vict)
					{
						act("$n swings a sack full of coal at $N's head.", FALSE, santa, NULL, vict, TO_NOTVICT);
						act("You black out as $n's sack of coal smashes into the side of your head!", FALSE, santa, NULL, vict, TO_VICT);
						damage(santa, vict, 150, TYPE_UNDEFINED, DAM_NO_BLOCK);
						if(GET_POS(vict) > POSITION_STUNNED)
							GET_POS(vict) = POSITION_STUNNED;
						if(!affected_by_spell(vict, SPELL_BLINDNESS))
						{
							af.type = SPELL_BLINDNESS;
							af.location = APPLY_HITROLL;
							af.modifier = -4;
							af.bitvector = AFF_BLIND;
							af.bitvector2 = 0;
							af.duration = 3;
							affect_to_char(vict, &af);
						}
					}
				}// END: blind/stun vict
				break;
		}
	}

	if(cmd==CMD_CAST && vict)
	{
		iLevel = GET_LEVEL(vict);
		if(iLevel > 46 && chance(85))
		{
			do_say(santa, "I have magic of my own you know.", CMD_SAY);
			act("$n throws a handful of shimmering dust into the air.", FALSE, santa, NULL, NULL, TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(santa)].people; vict; vict = vict->next_in_room)
			{
				if(!IS_MORTAL(vict)) continue;
				send_to_char("You choke as you inhale the strange northern dust.", vict);
				damage(vict, vict, 100, TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			act("$n inhales the dust deeply with a satisfied grin.", FALSE, santa, NULL, NULL, TO_ROOM);
			GET_HIT(santa) = MIN(GET_MAX_HIT(santa), GET_HIT(santa) + 500);
		}
		else if(iLevel > 42 && chance(60))
		{
			do_say(santa, "I have magic of my own you know.", CMD_SAY);
			act("$n throws a handful of shimmering dust into the air.", FALSE, santa, NULL, NULL, TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(santa)].people; vict; vict = vict->next_in_room)
			{
				if(!IS_MORTAL(vict)) continue;
				send_to_char("You choke as you inhale the strange northern dust.", vict);
				damage(vict, vict, 100, TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			act("$n inhales the dust deeply with a satisfied grin.", FALSE, santa, NULL, NULL, TO_ROOM);
			GET_HIT(santa) = MIN(GET_MAX_HIT(santa), GET_HIT(santa) + 400);
		}
		else if(iLevel > 39 && chance(35))
		{
			do_say(santa, "I have magic of my own you know.", CMD_SAY);
			act("$n throws a handful of shimmering dust into the air.", FALSE, santa, NULL, NULL, TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(santa)].people; vict; vict = vict->next_in_room)
			{
				if(!IS_MORTAL(vict)) continue;
				send_to_char("You choke as you inhale the strange northern dust.", vict);
				damage(vict, vict, 100, TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			act("$n inhales the dust deeply with a satisfied grin.", FALSE, santa, NULL, NULL, TO_ROOM);
			GET_HIT(santa) = MIN(GET_MAX_HIT(santa), GET_HIT(santa) + 300);
		}
	}
	return FALSE;
}/* END hemp_santa() */

int hemp_elf(CHAR *elf, CHAR *vict, int cmd, char *buf)
{
	int iLevel = 0;
	struct affected_type_5 af;

	if(cmd==MSG_MOBACT && elf->specials.fighting)
	{
		vict = elf->specials.fighting;
		iLevel = GET_LEVEL(vict);
		switch(iLevel)
		{
			case 40:
			case 41:
			case 42:
				if(chance(3))
				{
					do_say(elf, "Ack! I have to get back to the shoppe before Santa notices I'm gone!", CMD_SAY);
					spell_teleport(GET_LEVEL(elf), elf, NULL, NULL);
				}// END: teleport
				if(chance(7))
				{
					vict = get_random_victim_fighting(elf);
					if(!vict) break;
					sprintf(buf, "%s, I'm sick of making toys, you do it.", GET_NAME(vict));
					do_say(elf, buf, CMD_SAY);
					act("$n sprinkles some strange dust over $N, who suddenly starts making toys.", FALSE, elf, NULL, vict, TO_NOTVICT);
					act("$n sprinkles some strange dust over you, suddenly you find yourself making toys.", FALSE, elf, NULL, vict, TO_VICT);
					if(vict==elf->specials.fighting)
						stop_fighting(elf);
					stop_fighting(vict);
					WAIT_STATE(vict, PULSE_VIOLENCE * 5);
				}// END: stop vict fighting
				break;
			case 43:
			case 44:
			case 45:
			case 46:
				if(chance(5))
				{
					do_say(elf, "Ack! I have to get back to the shoppe before Santa notices I'm gone!", CMD_SAY);
					spell_teleport(GET_LEVEL(elf), elf, NULL, NULL);
				}// END: teleport
				if(chance(10))
				{
					vict = get_random_victim_fighting(elf);
					if(!vict) break;
					sprintf(buf, "%s, I'm sick of making toys, you do it.", GET_NAME(vict));
					do_say(elf, buf, CMD_SAY);
					act("$n sprinkles some strange dust over $N, who suddenly starts making toys.", FALSE, elf, NULL, vict, TO_NOTVICT);
					act("$n sprinkles some strange dust over you, suddenly you find yourself making toys.", FALSE, elf, NULL, vict, TO_VICT);
					if(vict==elf->specials.fighting)
						stop_fighting(elf);
					stop_fighting(vict);
					WAIT_STATE(vict, PULSE_VIOLENCE * 5);
				}// END: stop vict fighting
				break;
			case 47:
			case 48:
			case 49:
			case 50:
				if(chance(7))
				{
					do_say(elf, "Ack! I have to get back to the shoppe before Santa notices I'm gone!", CMD_SAY);
					spell_teleport(GET_LEVEL(elf), elf, NULL, NULL);
				}// END: teleport
				if(chance(12))
				{
					vict = get_random_victim_fighting(elf);
					if(!vict) break;
					sprintf(buf, "%s, I'm sick of making toys, you do it.", GET_NAME(vict));
					do_say(elf, buf, CMD_SAY);
					act("$n sprinkles some strange dust over $N, who suddenly starts making toys.", FALSE, elf, NULL, vict, TO_NOTVICT);
					act("$n sprinkles some strange dust over you, suddenly you find yourself making toys.", FALSE, elf, NULL, vict, TO_VICT);
					if(vict==elf->specials.fighting)
						stop_fighting(elf);
					stop_fighting(vict);
					WAIT_STATE(vict, PULSE_VIOLENCE * 7);
				}// END: stop vict fighting
				break;
			default:
				if(chance(5))
				{
					vict = get_random_victim_fighting(elf);
					if(!vict) break;
					sprintf(buf, "%s, I'm sick of making toys, you do it.", GET_NAME(vict));
					do_say(elf, buf, CMD_SAY);
					act("$n sprinkles some strange dust over $N, who suddenly starts making toys.", FALSE, elf, NULL, vict, TO_NOTVICT);
					act("$n sprinkles some strange dust over you, suddenly you find yourself making toys.", FALSE, elf, NULL, vict, TO_VICT);
					if(vict==elf->specials.fighting)
						stop_fighting(elf);
					stop_fighting(vict);
					WAIT_STATE(vict, PULSE_VIOLENCE * 3);
				}// END: stop vict fighting
				break;
		}
	}

	if(cmd==CMD_CAST && vict)
	{
		iLevel = GET_LEVEL(vict);
		if(iLevel > 46 && chance(85))
		{
			do_say(elf, "We elves have our own magic from the North!", CMD_SAY);
			if(chance(50))
			{
				act("$n summons a blizzard to bombard $N, burying $m in snow.", FALSE, elf, NULL, vict, TO_NOTVICT);
				act("$n summons a blizzard, drowning you in snow!", FALSE, elf, NULL, vict, TO_VICT);
				stop_fighting(vict);
				WAIT_STATE(vict, PULSE_VIOLENCE * 3);
			}
			else
			{
				act("$n summons a brilliant aurora to blind $N.", FALSE, elf, NULL, vict, TO_NOTVICT);
				act("$n summons a brilliant aurora, blinding you!", FALSE, elf, NULL, vict, TO_VICT);
				if(!affected_by_spell(vict, SPELL_BLINDNESS))
				{
					af.type = SPELL_BLINDNESS;
					af.location = APPLY_HITROLL;
					af.modifier = -4;
					af.bitvector = AFF_BLIND;
					af.bitvector2 = 0;
					af.duration = 8;
					affect_to_char(vict, &af);
				}
			}
		}
		else if(iLevel > 42 && chance(60))
		{
			do_say(elf, "We elves have our own magic from the North!", CMD_SAY);
			if(chance(50))
			{
				act("$n summons a blizzard to bombard $N.", FALSE, elf, NULL, vict, TO_NOTVICT);
				act("$n summons a blizzard which drowns you in snow!", FALSE, elf, NULL, vict, TO_VICT);
				stop_fighting(vict);
				WAIT_STATE(vict, PULSE_VIOLENCE * 2);
			}
			else
			{
				act("$n summons a brilliant aurora to blind $N.", FALSE, elf, NULL, vict, TO_NOTVICT);
				act("$n summons a brilliant aurora, blinding you!", FALSE, elf, NULL, vict, TO_VICT);
				if(!affected_by_spell(vict, SPELL_BLINDNESS))
				{
					af.type = SPELL_BLINDNESS;
					af.location = APPLY_HITROLL;
					af.modifier = -4;
					af.bitvector = AFF_BLIND;
					af.bitvector2 = 0;
					af.duration = 5;
					affect_to_char(vict, &af);
				}
			}
		}
		else if(iLevel > 39 && chance(35))
		{
			do_say(elf, "We elves have our own magic from the North!", CMD_SAY);
			if(chance(50))
			{
				act("$n summons a blizzard to bombard $N.", FALSE, elf, NULL, vict, TO_NOTVICT);
				act("$n summons a blizzard which drowns you in snow!", FALSE, elf, NULL, vict, TO_VICT);
				stop_fighting(vict);
				WAIT_STATE(vict, PULSE_VIOLENCE);
			}
			else
			{
				act("$n summons a brilliant aurora to blind $N.", FALSE, elf, NULL, vict, TO_NOTVICT);
				act("$n summons a brilliant aurora, blinding you!", FALSE, elf, NULL, vict, TO_VICT);
				if(!affected_by_spell(vict, SPELL_BLINDNESS))
				{
					af.type = SPELL_BLINDNESS;
					af.location = APPLY_HITROLL;
					af.modifier = -4;
					af.bitvector = AFF_BLIND;
					af.bitvector2 = 0;
					af.duration = 2;
					affect_to_char(vict, &af);
				}
			}
		}
	}
	return FALSE;
}/* END hemp_elf() */

int hemp_reindeer(CHAR *reindeer, CHAR *vict, int cmd, char *buf)
{
	int iLevel = 0;
	int iShield = 0;

	if(cmd==MSG_MOBACT && reindeer->specials.fighting)
	{
		vict = reindeer->specials.fighting;
		iLevel = GET_LEVEL(vict);
		switch(iLevel)
		{
			case 40:
			case 41:
			case 42:
				if(chance(3))
				{
					act("$n's ears prick up suddenly at the sound of Santa's whistle.", FALSE, reindeer, NULL, NULL, TO_ROOM);
					spell_teleport(GET_LEVEL(reindeer), reindeer, NULL, NULL);
				}// END: teleport
				if(chance(5))
				{
					vict = get_random_victim_fighting(reindeer);
					if(vict)
					{
						act("$n kicks $N who vanishes on impact.", FALSE, reindeer, NULL, vict, TO_NOTVICT);
						act("$n kicks you, you vanish as the magical hooves impact your chest.", FALSE, reindeer, NULL, vict, TO_VICT);
						damage(reindeer, vict, 100, TYPE_UNDEFINED, DAM_NO_BLOCK);
						spell_teleport(GET_LEVEL(reindeer), vict, NULL, NULL);
						WAIT_STATE(vict, PULSE_VIOLENCE * 2);
					}
				}// END: damage/stun/teleport vict
				if(chance(5))
				{
					vict = get_random_victim_fighting(reindeer);
					if(vict)
					{
						act("$n charges at $N and spears $m with $S velvety antlers.", FALSE, reindeer, NULL, vict, TO_NOTVICT);
						act("$n charges at you and spears your belly with $S velvety antlers.", FALSE, reindeer, NULL, vict, TO_VICT);
						damage(reindeer, vict, 200, TYPE_UNDEFINED, DAM_NO_BLOCK);
					}
				}// END: damage vict
				break;
			case 43:
			case 44:
			case 45:
			case 46:
				if(chance(5))
				{
					act("$n's ears prick up suddenly at the sound of Santa's whistle.", FALSE, reindeer, NULL, NULL, TO_ROOM);
					spell_teleport(GET_LEVEL(reindeer), reindeer, NULL, NULL);
				}// END: teleport
				if(chance(7))
				{
					vict = get_random_victim_fighting(reindeer);
					if(vict)
					{
						act("$n kicks $N who vanishes on impact.", FALSE, reindeer, NULL, vict, TO_NOTVICT);
						act("$n kicks you, you vanish as the magical hooves impact your chest.", FALSE, reindeer, NULL, vict, TO_VICT);
						damage(reindeer, vict, 200, TYPE_UNDEFINED, DAM_NO_BLOCK);
						spell_teleport(GET_LEVEL(reindeer), vict, NULL, NULL);
						WAIT_STATE(vict, PULSE_VIOLENCE * 2);
					}
				}// END: damage/stun/teleport vict
				if(chance(7))
				{
					vict = get_random_victim_fighting(reindeer);
					if(vict)
					{
						act("$n charges at $N and spears $m with $S velvety antlers.", FALSE, reindeer, NULL, vict, TO_NOTVICT);
						act("$n charges at you and spears your belly with $S velvety antlers.", FALSE, reindeer, NULL, vict, TO_VICT);
						damage(reindeer, vict, 300, TYPE_UNDEFINED, DAM_NO_BLOCK);
					}
				}// END: damage vict
				break;
			case 47:
			case 48:
			case 49:
			case 50:
				if(chance(7))
				{
					act("$n's ears prick up suddenly at the sound of Santa's whistle.", FALSE, reindeer, NULL, NULL, TO_ROOM);
					spell_teleport(GET_LEVEL(reindeer), reindeer, NULL, NULL);
				}// END: teleport
				if(chance(7))
				{
					vict = get_random_victim_fighting(reindeer);
					if(vict)
					{
						act("$n kicks $N who vanishes on impact.", FALSE, reindeer, NULL, vict, TO_NOTVICT);
						act("$n kicks you, you vanish as the magical hooves impact your chest.", FALSE, reindeer, NULL, vict, TO_VICT);
						damage(reindeer, vict, 300, TYPE_UNDEFINED, DAM_NO_BLOCK);
						spell_teleport(GET_LEVEL(reindeer), vict, NULL, NULL);
						WAIT_STATE(vict, PULSE_VIOLENCE * 2);
					}
				}// END: damage/stun/teleport vict
				if(chance(11))
				{
					vict = get_random_victim_fighting(reindeer);
					if(vict)
					{
						act("$n charges at $N and spears $m with $S velvety antlers.", FALSE, reindeer, NULL, vict, TO_NOTVICT);
						act("$n charges at you and spears your belly with $S velvety antlers.", FALSE, reindeer, NULL, vict, TO_VICT);
						damage(reindeer, vict, 400, TYPE_UNDEFINED, DAM_NO_BLOCK);
					}
				}// END: damage vict
				break;
			default:
				if(chance(5))
				{
					vict = get_random_victim_fighting(reindeer);
					if(vict)
					{
						act("$n charges at $N and spears $m with $S velvety antlers.", FALSE, reindeer, NULL, vict, TO_NOTVICT);
						act("$n charges at you and spears your belly with $S velvety antlers.", FALSE, reindeer, NULL, vict, TO_VICT);
						damage(reindeer, vict, 100, TYPE_UNDEFINED, DAM_NO_BLOCK);
					}
				}// END: damage vict
				break;
		}
	}

	if(cmd==MSG_TICK)
	{
		if(IS_SET(reindeer->specials.act, ACT_SHIELD))
		{
			act("$n's fur loses its silvery sheen.", FALSE, reindeer, NULL, NULL, TO_ROOM);
			REMOVE_BIT(reindeer->specials.act, ACT_SHIELD);
			iShield = 1;
		}
		else
		{
			if(iShield == 0)
			{
				act("$n's fur seems to gain a silvery sheen.", FALSE, reindeer, NULL, NULL, TO_ROOM);
				SET_BIT(reindeer->specials.act, ACT_SHIELD);
			}
			else
				iShield = 0;
		}
	}
	return FALSE;
}/* END hemp_reindeer() */

int hemp_grinch(CHAR *grinch, CHAR *vict, int cmd, char *buf)
{

	return FALSE;
}/* END hemp_grinch() */

/* END - Christmas Quest 2009 specs */

void assign_quests (void) {
  int i;
  assign_mob(XALTH_MOB1, xalth_mob);
  assign_mob(XALTH_MOB2, xalth_mob);
  assign_obj(QUEST_SLEEVES,Sleeves_of_Healing);
  assign_obj(HALF_MANA , mk4_half_mana);
  assign_obj(NO_SKILLS , mk4_no_skills);
  assign_obj(HALF_FLEE , mk4_half_flee);
  assign_obj(NO_CAST   , mk4_no_cast);
  assign_obj(HALF_MOVE , mk4_half_move);
  assign_obj(HALF_CONCE , mk4_half_conc);
  assign_obj(ZERO_REGEN, mk4_zero_regen);
  assign_obj(HALF_HPS  , mk4_half_hps);
  assign_obj(UGLY_COAT , ugly_coat);
  assign_obj(TACKY_BELT, tacky_belt);
  assign_obj(PURPLE_HAT, purple_hat);
  assign_obj(TARION_HELMET,helmet_tarion);
  for(i = 26220; i < 26281; i++)
    assign_mob(i, hemp_farmers);
  assign_obj(INFINITY_TATTOO, bracelet_of_magic);
  assign_obj(CUPID_DIAPER,cupid_diaper);
  assign_obj(6987,WHAP_SPEC);
  assign_obj(208, object_tracking);
  assign_obj(POINTY_HAT, pointy_hat_spec);

  assign_mob(HEMP_SANTA, hemp_santa);
  assign_mob(HEMP_ELF, hemp_elf);
  assign_mob(HEMP_REINDEER, hemp_reindeer);
	assign_mob(HEMP_GRINCH, hemp_grinch);
}

