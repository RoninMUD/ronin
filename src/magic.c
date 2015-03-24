/* ************************************************************************
*  file: magic.c , Implementation of spells.              Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/* Added IMMUNE_FIRE/LIGHTNING/POISON/SLEEP/CHARM/BLINDNESS/DRAIN
   Ranger Oct 96 */

/*
$Author: ronin $
$Date: 2005/01/25 21:48:58 $
$Header: /home/ronin/cvs/ronin/magic.c,v 2.31 2005/01/25 21:48:58 ronin Exp $
$Id: magic.c,v 2.31 2005/01/25 21:48:58 ronin Exp $
$Name:  $
$Log: magic.c,v $
Revision 2.32  2005/05/11 21:37:20  ronin
Added SKILL_EVASION to not be disenchanted in spell_disenchant.

Revision 2.31  2005/01/25 21:48:58  ronin
Added owner to clone spell.

Revision 2.30  2005/01/21 14:55:28  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.29  2004/11/20 02:55:12  ronin
Fixed mana regen amount on quick from 30 to 10.

Revision 2.28  2004/11/19 17:20:33  void
Changed name of lvl 50 ninja spell to mystic swiftness

Revision 2.27  2004/11/19 14:47:21  void
New Level 50 Ninja Spell (adds chance of 3rd hit)

Revision 2.26  2004/11/16 05:11:52  ronin
Chaos 2004 Update

Revision 2.25  2004/11/15 22:29:10  void
Made changes to Blood Lust, Twist, Quick, & Quad

Revision 2.24  2004/11/13 17:17:14  void
Added Mana_Regen to Spell Affects as well as Quick

Revision 2.23  2004/10/21 17:08:22  void
Added level 50 AP spell Blood Lust

Revision 2.22  2004/10/06 12:02:04  ronin
Fix for divine intervention not working in no-magic room.

Revision 2.21  2004/09/29 22:56:12  void
Spell Rush for Commando

Revision 2.20  2004/09/29 15:44:31  void
Some Final changes to Divine Intervention

Revision 2.19  2004/09/28 17:14:05  void
Made some minor changes to Divine Intervention

Revision 2.18  2004/09/28 14:32:15  void
Changed duration of Divine Intervention

Revision 2.17  2004/09/28 14:28:00  void
Added Level 50 Cleric Spell Divine Intervention

Revision 2.16  2004/09/22 21:58:26  void
Added level 50 Mage spell Quick (Allows for 2 casts in a single combat
round)

Revision 2.15  2004/09/13 21:35:37  void
Bard - Haste, Paladin - Improved Fury

Revision 2.14  2004/06/30 19:57:28  ronin
Update of saving throw routine.

Revision 2.13  2004/05/01 11:57:04  ronin
Addition of Owners field in spell_identify.

Revision 2.12  2004/03/09 19:43:29  void
Fixed permanent Bless on Paladins so it can not be disenchanted

Revision 2.11  2004/03/03 13:24:58  ronin
Fix to spell disenchant to prevent crash loops.

Revision 2.10  2004/02/26 19:14:11  void
Fixed Paralyze to make mob switch to the char that successfully paralyzed
said mob

Revision 2.9  2004/02/25 19:45:12  void
Changed Flee code to reflect with hold person code


Revision 2.7  2004/02/23 15:07:42  void
Changed Paralyze to allow characters to paralyze moblies up to 10 levels above
them.

Changed hide to show message if successful or unsuccessful

Revision 2.6  2004/02/19 19:20:22  void
Added Spell Shroud of Existence (Good Align Commando Spell)

Revision 2.5  2004/02/14 13:12:50  void
Fixed a couple errors I made

Revision 2.3  2004/02/13 14:30:01  Void
Changed the Sleep and Paralyze spells to 1 tick duration in Chaos.

Revision 2.2  2004/02/12 21:47:54  Void
Modified Death Spray and Earthquake to affect Impy chars in Chaos

Revision 2.1  2004/02/11 13:54:16  ronin
Change to clone spell to block cloning of anti-rent items.
Addition of braces in hell_fire to remove compile warning.

Revision 2.0.0.1  2004/02/05 16:09:41  ronin
Reinitialization of cvs archives

Revision 1.12 2003/11/03 ronin
Added check_equipment for all GET_ALIGN changes.
COMMENTED OUT until we get a faster processor.

Revision 1.11 2003/04/13 23:37:44  ronin
Addition of pride enchantment check to recall and summon.

Revision 1.10 2003/01/24 23:37:44  ronin
Added dam limits to Power Word Kill, as well as tables for
level differences between ch and vict.  Still a powerful spell.
-Changed disint aquest time from 30 to 25 mins.

Revision 1.9  2002/12/27 23:37:44  ronin
12/27/02 - Addition to disintegrate to make ICE_WALL and ICE_BLOCK
un-disintegratable.

Revision 1.8  2002/10/25 01:28:55  ronin
Check for CHAOSMODE to disable enter messages in 3001.

Revision - Addition to disintegrate to make WALL_THORNS and STATUE_PEACE un-
disintegratable

Revision 1.7  2002/07/24 17:53:41  ronin
Disenchante modified to exclude some skills that use affects.

Revision 1.6  2002/06/05 02:57:16  ronin
Fix to allow bless song to work in battle.

Revision 1.5  2002/04/27 05:46:57  ronin
Addition of gluttony enchantment check to satiate.

Revision 1.4  2002/04/13 14:39:51  ronin
Fix so power word kill can be cast by a mob on a player.

Revision 1.3  2002/03/31 16:35:06  ronin
Added braces to remove ambiguous else warning.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "db.h"
#include "utility.h"
#include "fight.h"
#include "act.h"
#include "cmd.h"
#include "memory.h"
#include "enchant.h"
#include "subclass.h"

/* Extern structures */

extern Room *world;
extern OBJ  *object_list;
extern CHAR *character_list;

#define MOB_ID_HP_ARRAY_MAX    10
#define MOB_ID_DMG_ARRAY_MAX   9
#define MOB_ID_LEVEL_ARRAY_MAX 6
#define MOB_ID_MATT_ARRAY_MAX  4
#define MOB_ID_SPEC_ARRAY_MAX  2

struct scoreboard_data
{
  char killer[80];
  char killed[80];
  char location[80];
  char time_txt[80];
};

extern struct scoreboard_data scores[101];
extern int number_of_kills;
extern struct idname_struct idname[MAX_ID];

/* Extern procedures */
extern char *subclass_res_bits[];
extern int CHAOSMODE;
extern int CHAOSDEATH;
void stop_riding(struct char_data *ch,struct char_data *vict);
void empty_all_containers (CHAR *ch);
void make_statue (struct char_data *ch);
void weight_change_object(OBJ *obj, int weight);
char *str_dup(char *source);
char *one_argument (char*, char*);
int dice(int number, int size);
void stop_follower(CHAR *ch);
void do_look(CHAR *ch, char *argument, int cmd);
extern struct descriptor_data *descriptor_list;

/* Offensive Spells */

void spell_reappear(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
    act("$p reappears into the world.", TRUE, ch, obj, 0, TO_CHAR);
    act("$p reappears into the world.", TRUE, ch, obj, 0, TO_ROOM);
    REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
  }
}

void spell_reveal(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  if ( IS_AFFECTED(victim, AFF_INVISIBLE))
    appear(victim);
}

void spell_paralyze(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
    if (saves_spell(victim, SAVING_PARA,level))
      return;

    if (!IS_MORTAL(victim))
      return;

    if (IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_PARALYSIS))
      return;

    if(GET_LEVEL(victim)-10>GET_LEVEL(ch)) return;

    af.type      = SPELL_PARALYSIS;

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
      af.duration = 1;
    else
      af.duration  = level;
    af.modifier  = +100;
    af.location  = APPLY_AC;
    af.bitvector = AFF_PARALYSIS;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_HITROLL;
    af.modifier = -5;
    affect_to_char(victim, &af);

    act("Your limbs freeze in place",FALSE,victim,0,0,TO_CHAR);
    act("$n is paralyzed!",TRUE,victim,0,0,TO_ROOM);

    if (IS_NPC(victim)) {
      act ("$n gets angry and attacks...",FALSE,victim,0,0,TO_ROOM);
      stop_fighting(victim);
      hit (victim, ch, TYPE_UNDEFINED);
      return;
    }
  } else {
    send_to_char("Someone tries to paralyze you AGAIN!\n\r",victim);
  }
}

void spell_remove_paralysis(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (affected_by_spell(victim, SPELL_PARALYSIS) || affected_by_spell(victim, SPELL_HOLD)) {

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) && ch!=victim) { /* Chaos03 */
      send_to_char("You cannot cast this spell on another player.\n\r",ch);
      return;
    }

    affect_from_char(victim, SPELL_PARALYSIS);
    affect_from_char(victim, SPELL_HOLD);
    send_to_char("You feel much much better!\n\r", victim);
  }
}

/* Updated Nov 25/99 - Ranger */
void spell_magic_missile(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam; /*, dam_each = 10;*/

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = level/2+number(5,10);
/*  dam=number(dam_each>>1, dam_each<<1);*/

  if (saves_spell(victim, SAVING_SPELL,level))
    dam >>= 1;

  damage(ch, victim, dam, SPELL_MAGIC_MISSILE,DAM_MAGICAL);
}

void spell_hell_fire(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  CHAR *tmp, *temp;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  act("$n has summoned the fires of hell to burn here.",TRUE,ch,0,0,TO_ROOM);
  act("You have summoned the fires of hell to burn here.",FALSE,ch,0,0,TO_CHAR);

  for(tmp = world[CHAR_REAL_ROOM(ch)].people;tmp; tmp = temp) {
    temp = tmp->next_in_room;
    if(IS_NPC(tmp) && IS_SET(tmp->specials.immune,IMMUNE_FIRE)) continue;
    if(IS_NPC(ch) && !IS_NPC(tmp) && GET_LEVEL(tmp)>LEVEL_MORT)
      continue;
    if (ch != tmp && ( (IS_NPC(ch) ? !IS_NPC(tmp) : IS_NPC(tmp)) || ROOM_CHAOTIC(CHAR_REAL_ROOM(tmp)) )) {
        damage(ch, tmp, 200, SPELL_HELL_FIRE,DAM_FIRE);
      }
    }

  if (!(IS_EVIL(ch)))
    damage(ch, ch, 200, SPELL_HELL_FIRE,DAM_NO_BLOCK);
}

void spell_death_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  CHAR *temp_victim = NULL, *next_victim = NULL;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  act("You throw out a spray of deadly rays of dark light.", FALSE, ch, 0, 0, TO_CHAR);
  act("There are many dark rays shooting out from $n's hand.", TRUE, ch, 0, 0, TO_ROOM);

  for (temp_victim = world[CHAR_REAL_ROOM(ch)].people; temp_victim; temp_victim = next_victim)
  {
    next_victim = temp_victim->next_in_room;

    if (IS_IMMORTAL(temp_victim)) continue;

    if (ch != temp_victim && ((IS_NPC(ch) ? !IS_NPC(temp_victim) : IS_NPC(temp_victim)) || ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_victim))))
    {
      damage(ch, temp_victim, (level / 2) + 120, SPELL_DEATH_SPRAY, DAM_MAGICAL);
    }
  }
}

/* Updated Nov 25/99 - Ranger */
void spell_chill_touch(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;
  int dam;
/*  int dam_each = 16;*/

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = level/2+number(10,40);
  /*number(dam_each>>1, dam_each<<1);*/

  if (!saves_spell(victim, SAVING_SPELL,level)) {
    af.type      = SPELL_CHILL_TOUCH;
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
      af.duration = 1;
    else
      af.duration  = 6;
    af.modifier  = -1;
    af.location  = APPLY_STR;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_join(victim, &af, TRUE, FALSE);
  } else {
    dam >>= 1;
  }

  damage(ch, victim, dam, SPELL_CHILL_TOUCH,DAM_COLD);
}
/* Updated Nov 25/99 - Ranger */
void spell_burning_hands(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
/*  int dam_each = 25;*/

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = level/2+number(15,50);
  /*number(dam_each>>1, dam_each<<1);*/

  if (saves_spell(victim, SAVING_SPELL,level))
    dam >>= 1;

  if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_FIRE)) dam=0;

  damage(ch, victim, dam, SPELL_BURNING_HANDS,DAM_FIRE);
}

/* Updated Nov 25/99 - Ranger */
void spell_shocking_grasp(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
/*  int dam_each = 40;*/

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = level/2+number(20,80);
  /*number(dam_each>>1, dam_each<<1);*/

  if (saves_spell(victim, SAVING_SPELL,level))
    dam >>= 1;

  if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_ELECTRIC)) dam=0;

  damage(ch, victim, dam, SPELL_SHOCKING_GRASP,DAM_ELECTRIC);
}

/* Updated Nov 25/99 - Ranger */
void spell_lightning_bolt(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
  /*int dam_each = 60;*/

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = level/2 + number(30,120);
  /*number(dam_each>>1, dam_each<<1);*/

  if (saves_spell(victim, SAVING_SPELL,level))
    dam >>= 1;

  if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_ELECTRIC)) dam=0;

  damage(ch, victim, dam, SPELL_LIGHTNING_BOLT,DAM_ELECTRIC);
}

/* Updated Nov 25/99 - Ranger */
void spell_flamestrike(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)  {
  int dam;
/*  int dam_each = 80;*/

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = level/2 + number(50,120);
  /* number(dam_each>>1, dam_each<<1);*/

  if (saves_spell(victim, SAVING_SPELL,level))
    dam >>= 1;
  if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_FIRE)) dam=0;
/*
  act("$n throws a flamestrike out.", TRUE, ch, 0, 0, TO_ROOM);
  act("You throw a flamstrike out.", FALSE, ch, 0, 0, TO_CHAR);
  send_to_char("You are hit by a flamestrike!\n\r", victim);
*/

  damage(ch, victim, dam, SPELL_FLAMESTRIKE,DAM_FIRE);
}

/* Updated Nov 25/99 - Ranger */
void spell_colour_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
  /*int dam_each = 80;*/

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = level/2 + number(35,100);
  /*number(dam_each-20, dam_each+20);*/

  if (saves_spell(victim, SAVING_SPELL,level))
    dam >>= 1;

  damage(ch, victim, dam, SPELL_COLOUR_SPRAY,DAM_MAGICAL);
}

void spell_energy_drain(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam, xp, mana;

  void gain_exp(CHAR *ch, int gain);

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_DRAIN)) {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN,DAM_MAGICAL);
    return;
  }

  if (!saves_spell(victim, SAVING_SPELL,level)) {
    GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch)-200,-1000);
    /* check_equipment(ch);  Linerfix 110203 */
    if (GET_LEVEL(victim) <= 2) {
      damage(ch, victim, 100, SPELL_ENERGY_DRAIN,DAM_MAGICAL); /* Kill the sucker */
    } else {
      if(IS_AFFECTED(victim, AFF_SPHERE) && !breakthrough(ch,victim,BT_SPHERE)) {
        damage(ch, victim, 0, SPELL_ENERGY_DRAIN,DAM_MAGICAL);
        return;
      }
      xp = number(level>>1,level)*500;
      gain_exp(victim, -xp);
      gain_exp(ch,xp/4);
      dam = dice(1,10);

      mana = GET_MANA(victim)>>1;
      GET_MOVE(victim) >>= 1;
      GET_MANA(victim) = mana;

      GET_MANA(ch) += mana>>1;
      GET_HIT(ch) += dam;

/*      send_to_char("Your life energy is drained!\n\r", victim);*/

      damage(ch, victim, dam, SPELL_ENERGY_DRAIN,DAM_NO_BLOCK);
    }
  } else {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN,DAM_MAGICAL); /* Miss */
  }
}
/* Updated Nov 25/99 - Ranger */
void spell_fireball(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
/*  int dam_each = 100;*/

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = level/2 + number(80,140);
  /*umber(dam_each-20, dam_each+20);*/

  if (saves_spell(victim, SAVING_SPELL,level)) dam >>= 1;
  if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_FIRE)) dam=0;

  damage(ch, victim, dam, SPELL_FIREBALL,DAM_FIRE);
}

/* Updated Nov 25/99 - Ranger */
void spell_iceball(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
/*  int dam_each = 130;*/

  dam = level/2 + number(100,160);
  /*number(dam_each-20, dam_each+20);*/

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!IS_NPC(ch) && !IS_NPC(victim)
      && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))
      && !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA) ) {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
    return;
  }

  if ( saves_spell(victim, SAVING_SPELL,level) ) dam >>= 1;

  GET_MOVE(victim) = MAX(GET_MOVE(victim)-30, 0);
/*
  act("$n conjures up an iceball an throws it into $N's face.",
      FALSE, ch, 0, victim, TO_ROOM);
  act("You conjure up an iceball and throw it into $N's face.",
      FALSE, ch, 0, victim, TO_CHAR);
  act("$n throws an  iceball in your face.",
      FALSE, ch, 0, victim, TO_VICT);
*/

  damage(ch, victim, dam, SPELL_ICEBALL,DAM_COLD);
}

void spell_lethal_fire(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  int dam = 0;

  dam = (level / 2) + number(180, 230);

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && !ROOM_ARENA(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);

    return;
  }

  if (saves_spell(victim, SAVING_SPELL, level)) dam /= 2;

  if (IS_NPC(victim) && IS_SET(victim->specials.immune, IMMUNE_FIRE)) dam = 0;
/*
  act("$N screams loudly as blue flames from $n's hands are burning $S body.",
      FALSE, ch, 0, victim, TO_ROOM);
  act("You send fiery flames of blue fire to burn $N to death.",
      FALSE, ch, 0, victim, TO_CHAR);
  send_to_char("You are burnt by the blue fire.\n\r", victim);
*/

  damage(ch, victim, dam, SPELL_LETHAL_FIRE, DAM_FIRE);
}

void spell_thunderball(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = number(800, 1100);

  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
    return;
  }
/*
  act("$N screams loudly as a ball of thunder and lightning hits $S body.",
      FALSE, ch, 0, victim, TO_ROOM);
  act("You conjure up a ball of thunder and throw it at $N.", FALSE, ch, 0, victim, TO_CHAR);
  send_to_char("You scream loudly when a thunderball hits your body.\n\r",
         victim);
*/
  damage(ch, victim, dam, SPELL_THUNDERBALL,DAM_SOUND);
}

void spell_electric_shock(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = number(450, 500);

  if (!IS_NPC(ch) && !IS_NPC(victim) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))
      && !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA) ) {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
    return;
  }

  if (!IS_NPC(victim) && saves_spell(victim, SAVING_SPELL,level) ) dam >>= 1;

  if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_ELECTRIC)) return;
/*
  act("$N screams loudly when $n touches $S body.",FALSE,ch,0,victim,TO_ROOM);
  act("Your hands are full of electricity and you touch $N.",
      FALSE,ch,0,victim, TO_CHAR);
  send_to_char("You scream loudly when 50.000 volts surge through your body.\n\r", victim);
*/

  damage(ch, victim, dam, SPELL_ELECTRIC_SHOCK,DAM_ELECTRIC);
}

void spell_earthquake(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
  CHAR *tmp_victim, *temp;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = dice(1,8)+(IS_NPC(ch) ? 3*level : level);

  send_to_char("The earth trembles beneath your feet!\n\r", ch);
  act("$n makes the earth tremble and shiver...",FALSE, ch, 0, 0, TO_ROOM);

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if(!IS_NPC(tmp_victim) && GET_LEVEL(tmp_victim)>=LEVEL_IMM)
      continue;
    if (ch != tmp_victim &&
  ((IS_NPC(ch) ? !IS_NPC(tmp_victim) : IS_NPC(tmp_victim)) || ROOM_CHAOTIC(CHAR_REAL_ROOM(tmp_victim)) )) {
      damage(ch, tmp_victim, dam, SPELL_EARTHQUAKE,DAM_MAGICAL);
    }
  }
}

void spell_dispel_evil(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (IS_EVIL(ch))
    victim = ch;
  else
    if (IS_GOOD(victim) || IS_NEUTRAL(victim)) {
      dam = 0;
      damage(ch, victim, dam, SPELL_DISPEL_EVIL,DAM_MAGICAL);
      return;
    }

  if(!IS_NPC(ch)) level=MIN(level,30);

  if ((GET_LEVEL(victim) < level) || (victim == ch))
    dam = 180;
  else {
    dam = dice(level,4);
    if ( saves_spell(victim, SAVING_SPELL,level) ) dam >>= 1;
  }
  if(ch==victim) {
    act("Your dispel backfires and targets you!.",0,ch,0,0,TO_CHAR);
    act("$n's dispel backfires and targets $m.",0,ch,0,0,TO_ROOM);
  }

  damage(ch, victim, dam, SPELL_DISPEL_EVIL,DAM_MAGICAL);
}

void spell_dispel_good(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (IS_GOOD(ch))
    victim = ch;
  else
    if (IS_EVIL(victim) || IS_NEUTRAL(victim)) {
      dam = 0;
      damage(ch, victim, dam, SPELL_DISPEL_GOOD,DAM_MAGICAL);
      return;
    }

  if(!IS_NPC(ch)) level=MIN(level,30);

  if ((GET_LEVEL(victim) < level) || (victim == ch))
    dam = 180;
  else {
    dam = dice(level,4);
    if ( saves_spell(victim, SAVING_SPELL,level) ) dam >>= 1;
  }
  if(ch==victim) {
    act("Your dispel backfires and targets you!.",0,ch,0,0,TO_CHAR);
    act("$n's dispel backfires and targets $m.",0,ch,0,0,TO_ROOM);
  }

  damage(ch, victim, dam, SPELL_DISPEL_GOOD,DAM_MAGICAL);
}

void spell_evil_word(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (IS_GOOD(ch)) {
    spell_dispel_good(level, ch, ch, 0);
    return;
  }

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people;tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if (ch != tmp_victim &&
  ((IS_NPC(ch) ? !IS_NPC(tmp_victim) : IS_NPC(tmp_victim)) || ROOM_CHAOTIC(CHAR_REAL_ROOM(tmp_victim))))
      spell_dispel_good(level, ch, tmp_victim, 0);
  }
}

void spell_holy_word(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (IS_EVIL(ch)) {
    spell_dispel_evil(level, ch, ch, 0);
    return;
  }

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if (ch != tmp_victim &&
  ((IS_NPC(ch) ? !IS_NPC(tmp_victim) : IS_NPC(tmp_victim)) || ROOM_CHAOTIC(CHAR_REAL_ROOM(tmp_victim))))
      spell_dispel_evil(level, ch, tmp_victim, 0);
  }
}

void spell_call_lightning(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  int dam;

  extern struct weather_data weather_info;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = dice(MIN(level,30), 8);

  if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
    if (saves_spell(victim, SAVING_SPELL,level))
      dam >>= 1;

    if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_ELECTRIC)) dam=0;

    damage(ch, victim, dam, SPELL_CALL_LIGHTNING,DAM_ELECTRIC);
  }
}

/* Updated Nov 25/99 - Ranger */
void spell_harm(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = GET_HIT(victim) - dice(1,4);

  if (dam < 0)
    dam = 25; /* Kill the suffering bastard */
  else if (saves_spell(victim, SAVING_SPELL,level))
    dam = MIN(50+level/4, dam/2);

  dam = MIN(100+level/2, dam);

  damage(ch, victim, dam, SPELL_HARM,DAM_MAGICAL);
}

void spell_super_harm(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  int dam;

  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't cast such a powerful spell on a player!\n\r", ch);
    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = GET_HIT(victim) - dice(1,4);

  if (dam < 0)
    dam = 25; /* Kill the suffering bastard */
  else if (saves_spell(victim, SAVING_SPELL,level))
    dam = dam/2;
/*
  act("You are harmed by $N!",FALSE,victim,0,ch,TO_CHAR);
  act("You harm $N!",TRUE,ch,0,victim,TO_CHAR);
  act("$n harms $N!",TRUE,ch,0,victim,TO_NOTVICT);
*/

  dam = MIN(dam, ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 450 : 600);

  damage(ch, victim, dam, SPELL_HARM,DAM_MAGICAL);
}

void spell_armor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_ARMOR)) {

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
      send_to_char("You cannot cast this spell on another player.\n\r",ch);
      return;
    }

    af.type      = SPELL_ARMOR;
    af.duration  = 24;
    af.modifier  = -10;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);
    send_to_char("You feel someone protecting you.\n\r", victim);
  }
}

int dt_or_hazard(CHAR *ch);
void spell_teleport(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int to_room;
  extern int top_of_world;      /* ref to the top element of world */

  if(CHAOSMODE) {
    send_to_char("The forces of Chaos absorb the magic.\n\r",ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_BEAM)) {
    send_to_char("You cannot teleport from here.\n\r",ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
  {
      to_room = real_room(number(30001,30031));
  act("$n slowly fades out of existence.", FALSE, ch,0,0,TO_ROOM);
  char_from_room(ch);
  if(ch->specials.fighting)
    stop_fighting(ch);
  char_to_room(ch, to_room);
  act("$n slowly fades into existence.", FALSE, ch,0,0,TO_ROOM);
  do_look(ch, "", CMD_LOOK);
  return;
}
 do {
    to_room = number(0, top_of_world);
  } while (IS_SET(world[to_room].room_flags, PRIVATE) ||
     IS_SET(world[to_room].room_flags, NO_MAGIC) ||
     (IS_SET(world[to_room].room_flags, TUNNEL) && !CHAOSMODE) ||
     IS_SET(world[to_room].room_flags, CHAOTIC) ||
     IS_SET(world[to_room].room_flags, NO_BEAM) ||
     (IS_NPC(ch) && IS_SET(world[to_room].room_flags, SAFE)) ||
     (IS_NPC(ch) && IS_SET(world[to_room].room_flags, DEATH)) ||
     (IS_NPC(ch) && IS_SET(world[to_room].room_flags, HAZARD)) ||
     IS_SET(world[to_room].room_flags, LOCK));

  act("$n slowly fades out of existence.", FALSE, ch,0,0,TO_ROOM);
  char_from_room(ch);
  if(ch->specials.fighting)
    stop_fighting(ch);
  char_to_room(ch, to_room);
  act("$n slowly fades into existence.", FALSE, ch,0,0,TO_ROOM);

  do_look(ch, "", CMD_LOOK);
  dt_or_hazard(ch);
}

void spell_bless(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (obj) {
    if ((5*GET_LEVEL(ch) > GETOBJ_WEIGHT(obj)) &&
  (GET_POS(ch) != POSITION_FIGHTING) &&
  !IS_OBJ_STAT(obj, ITEM_EVIL)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
      act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR);
    }
  } else {
    if (!affected_by_spell(victim, SPELL_BLESS)) {

      if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
        send_to_char("You cannot cast this spell on another player.\n\r",ch);
        return;
      }

      send_to_char("You feel righteous.\n\r", victim);
      af.type      = SPELL_BLESS;
      af.duration  = MAX(6,GET_LEVEL(ch)/5);
      af.modifier  = 1;
      af.location  = APPLY_HITROLL;
      af.bitvector = 0;
      af.bitvector2 = 0;
      affect_to_char(victim, &af);

      af.location = APPLY_SAVING_SPELL;
      af.modifier = -1;                 /* Make better */
      affect_to_char(victim, &af);
    }
  }
}

void spell_blindness(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA)
    && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))
    && !IS_SET(ch->specials.pflag, PLR_NOKILL)
    && !IS_NPC(victim) && !IS_NPC(ch))
  {
    /* if the victim and the ch are players, and it's not an arena, chaos, or they've got nokill on don't do it */
    send_to_char("You can't blind a player here.\n\r", ch);
    return;
  }

  if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_BLINDNESS)) {
    if (CAN_SEE(victim, ch)) {
      act ("$n gets angry and attacks...",FALSE,victim,0,0,TO_ROOM);
      hit (victim,ch,TYPE_UNDEFINED);
    }
    return;
  }

  if (!saves_spell(victim, SAVING_SPELL,level) &&
      !affected_by_spell(victim, SPELL_BLINDNESS)) {
    act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You have been blinded!\n\r", victim);

    af.type      = SPELL_BLINDNESS;
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
      af.duration  = 1;
    else
      af.duration  = 2;
    af.location  = APPLY_HITROLL;
    af.bitvector = AFF_BLIND;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_AC;
    af.modifier = +40; /* Make AC Worse! */
    affect_to_char(victim, &af);
  }

  if (CAN_SEE(victim, ch) && IS_NPC(victim)) {
    if (victim->specials.fighting) return; /* Linerfix 120303 */
    act ("$n gets angry and attacks...",FALSE,victim,0,0,TO_ROOM);
    hit (victim,ch,TYPE_UNDEFINED);
  }
}

void spell_blindness_dust(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  act("$n has thrown a cloud of blinding dust out at you!",
      TRUE, ch, 0, 0, TO_ROOM);
  act("You throw a cloud of blinding dust out!", FALSE, ch, obj, 0, TO_CHAR);

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people;tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if(IS_NPC(ch) && !IS_NPC(tmp_victim) && GET_LEVEL(tmp_victim)>LEVEL_MORT)
      continue;
    if(IS_NPC(ch) && IS_NPC(tmp_victim) && inzone(V_MOB(ch))==inzone(V_MOB(tmp_victim)) )
      continue;
    if(ch==tmp_victim)
      continue;
    spell_blindness(level, ch, tmp_victim, 0);
  }
}

void spell_clone(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
  OBJ *clone;
  char buf[MAX_STRING_LENGTH];

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("The forces of Chaos absorb the magic.\n\r",ch);
    return;
  }

  if (obj) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_CLONE) ||
  (GET_ITEM_TYPE(obj) == ITEM_STAFF) ||
  (GET_ITEM_TYPE(obj) == ITEM_WAND) ||
  (GET_ITEM_TYPE(obj) == ITEM_POTION) ||
  (GET_ITEM_TYPE(obj) == ITEM_BULLET) ||
  (GET_ITEM_TYPE(obj) == ITEM_SCROLL) ||
  (GET_ITEM_TYPE(obj) == ITEM_FIREWEAPON) ||
  (GET_ITEM_TYPE(obj) == ITEM_MISSILE) ||
  (GET_ITEM_TYPE(obj) == ITEM_MONEY) ||
  (GET_ITEM_TYPE(obj) == ITEM_RECIPE) ||
  (GET_ITEM_TYPE(obj) == ITEM_AQ_ORDER) ||
  (GET_ITEM_TYPE(obj) == ITEM_KEY)) {
      act("$p exploded.", FALSE, ch, obj, 0, TO_ROOM);
      act("$p exploded.", FALSE, ch, obj, 0, TO_CHAR);
      extract_obj(obj);

      if (GET_LEVEL(ch) < LEVEL_IMM) {
  dam = number(10,40);
  send_to_char("You hurt yourself!\n\r", ch);
  damage(ch, ch, dam, TYPE_UNDEFINED,DAM_NO_BLOCK);
      }
      return;
    }


    if(obj->item_number==-1 || obj->item_number==0) {
       send_to_char("You failed!\n\r",ch);
       return;
    }

    clone = read_object(obj->item_number, REAL);
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_LIMITED) ||
        IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RENT)) {
      send_to_char("You failed!\n\r", ch);
      extract_obj(clone);
      return;
    }

    /* New check for a limited number of clones for objs with
       abnormal stats or from vanity zone - Ranger Oct 23/97 */
    if(diff_obj_stats(obj) || (inzone(V_OBJ(obj))==299) || (inzone(V_OBJ(obj))==298) || (inzone(V_OBJ(obj))==294) || (inzone(V_OBJ(obj))==69) || (inzone(V_OBJ(obj))==10)) {
      if((CAN_WEAR(obj,ITEM_WEAR_FINGER)) || (CAN_WEAR(obj,ITEM_WEAR_NECK))
         || (CAN_WEAR(obj,ITEM_WEAR_WRIST))) {
        if(obj_proto_table[obj->item_number].number>6) {
          send_to_char("Only 5 clones of this object can be created.\n\r",ch);
          extract_obj(clone);
          return;
        }
      }
      else {
        if(obj_proto_table[obj->item_number].number>3) {
          send_to_char("Only 2 clones of this object can be created.\n\r",ch);
          extract_obj(clone);
          return;
        }
      }
    }

    clone->obj_flags.wear_flags = obj->obj_flags.wear_flags;
    clone->obj_flags.type_flag = obj->obj_flags.type_flag;
    clone->obj_flags.extra_flags = obj->obj_flags.extra_flags;
    clone->obj_flags.value[0] = obj->obj_flags.value[0];
    clone->obj_flags.value[1] = obj->obj_flags.value[1];
    clone->obj_flags.value[2] = obj->obj_flags.value[2];
    clone->obj_flags.value[3] = obj->obj_flags.value[3];
    clone->obj_flags.weight = obj->obj_flags.weight;
    clone->obj_flags.cost = 0;
    clone->obj_flags.cost_per_day = 100000;
    clone->affected[0].location = obj->affected[0].location;
    clone->affected[0].modifier = obj->affected[0].modifier;
    clone->affected[1].location = obj->affected[1].location;
    clone->affected[1].modifier = obj->affected[1].modifier;
    clone->affected[2].location = obj->affected[2].location;
    clone->affected[2].modifier = obj->affected[2].modifier;
    clone->ownerid[0]=obj->ownerid[0];
    clone->ownerid[1]=obj->ownerid[1];
    clone->ownerid[2]=obj->ownerid[2];
    clone->ownerid[3]=obj->ownerid[3];
    clone->ownerid[4]=obj->ownerid[4];
    clone->ownerid[5]=obj->ownerid[5];
    clone->ownerid[6]=obj->ownerid[6];
    clone->ownerid[7]=obj->ownerid[7];

    clone->name = str_dup(OBJ_NAME(obj));
    clone->description = str_dup(OBJ_DESCRIPTION(obj));
    clone->short_description  = str_dup(OBJ_SHORT(obj));

    SET_BIT(clone->obj_flags.extra_flags, ITEM_MAGIC);
    SET_BIT(clone->obj_flags.extra_flags, ITEM_ANTI_RENT);
    SET_BIT(clone->obj_flags.extra_flags, ITEM_CLONE);

    sprintf(buf,"You create an almost perfect duplicate of %s.\n\r", OBJ_SHORT(clone));
    send_to_char(buf, ch);
    act("$n creates a duplicate of $p.", FALSE, ch, clone, 0, TO_ROOM);
    obj_to_room(clone, CHAR_REAL_ROOM(ch));
  } else {
/*    if(GET_LEVEL(ch)==LEVEL_IMP)
     clone_char(victim);*/
  }
}

void spell_control_weather(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  /* Control Weather is not possible here!!! */
  /* Better/Worse can not be transferred     */
}

void spell_create_food(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  OBJ *tmp_obj;

  CREATE(tmp_obj, OBJ, 1);
  clear_object(tmp_obj);

  tmp_obj->name = str_dup("mushroom");
  tmp_obj->short_description = str_dup("A Magic Mushroom");
  tmp_obj->description =
    str_dup("A really delicious looking magic mushroom lies here.");

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE;
  tmp_obj->obj_flags.value[0] = 5+level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_room(tmp_obj,CHAR_REAL_ROOM(ch));

  tmp_obj->item_number = -1;

  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
}

void spell_create_water(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int water;

  extern struct weather_data weather_info;
  void name_to_drinkcon(OBJ *obj,int type);
  void name_from_drinkcon(OBJ *obj,int type);

  if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
    if ((obj->obj_flags.value[2] != LIQ_WATER)
         && (obj->obj_flags.value[1] != 0)) {
      name_from_drinkcon(obj,obj->obj_flags.value[2]);
      obj->obj_flags.value[2] = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
      water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);
      /* Calculate water it can contain, or water created */
      water = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], water);

      if (water > 0) {
        obj->obj_flags.value[2] = LIQ_WATER;
        obj->obj_flags.value[1] += water;
        name_to_drinkcon(obj, LIQ_WATER);
        act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
      }
    }
  }
}

void spell_cure_blind(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
    send_to_char("You cannot cast this spell on another player.\n\r",ch);
    return;
  }


  if (affected_by_spell(victim, SPELL_BLINDNESS)) {
    affect_from_char(victim, SPELL_BLINDNESS);
    send_to_char("Your vision returns!\n\r", victim);
  }
}

void spell_cure_critic(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) &&
      ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      ((duration_of_spell(victim, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(victim, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", victim);
    return;
  }

  //GET_HIT(victim) = MIN(GET_HIT(victim) + MIN(10 + (level * 5), 75), GET_MAX_HIT(victim));
  magic_heal(victim, SPELL_CURE_CRITIC, MIN(10 + (level * 5), 75), FALSE);
  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
}

void spell_mana_transfer(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
    send_to_char("You cannot cast this spell on another player.\n\r",ch);
    return;
  }

  GET_MANA(victim) += 30;
  send_to_char("You feel better!\n\r", victim);
}

/* void check_equipment(CHAR *ch); */
void spell_holy_bless(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  GET_ALIGNMENT(victim) = MIN(1000, GET_ALIGNMENT(victim)+100);
  /* check_equipment(victim);   Linerfix 110203 */
  send_to_char("You feel Saintly!\n\r", victim);
}

/* void check_equipment(CHAR *ch); */
void spell_evil_bless(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  GET_ALIGNMENT(victim) = MAX(-1000, GET_ALIGNMENT(victim)-100);
  /* check_equipment(victim);   Linerfix 110203 */
  send_to_char("You feel Evil!\n\r", victim);
}

void spell_cure_light_spray(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  void spell_cure_light(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people;tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    spell_cure_light(level, ch, tmp_victim, 0);
  }
}

void spell_cure_serious_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  void spell_cure_serious(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people;tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    spell_cure_serious(level, ch, tmp_victim, 0);
  }
}

void spell_cure_critic_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    spell_cure_critic(level, ch, tmp_victim, 0);
  }
}

void spell_heal_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  void spell_heal(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    spell_heal(level, ch, tmp_victim, 0);
  }
}

void spell_great_miracle(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  void spell_miracle(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    spell_miracle(level, ch, tmp_victim, 0);
  }
}

void spell_cure_serious(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) &&
      ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      ((duration_of_spell(victim, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(victim, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", victim);
    return;
  }

  ///GET_HIT(victim) = MIN(GET_HIT(victim) + MIN(10 + (level * 5), 45), GET_MAX_HIT(victim));
  magic_heal(victim, SPELL_CURE_SERIOUS, MIN(10 + (level * 5), 45), FALSE);
  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
}

void spell_cure_light(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) &&
      ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      ((duration_of_spell(victim, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(victim, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", victim);
    return;
  }

  //GET_HIT(victim) = MIN(GET_HIT(victim) + MIN(10 + (level * 5), 30), GET_MAX_HIT(victim));
  magic_heal(victim, SPELL_CURE_LIGHT, MIN(10 + (level * 5), 30), FALSE);
  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
}

void spell_curse(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (obj) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
    REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);

    /* LOWER ATTACK DICE BY -1 */
    if(obj->obj_flags.type_flag == ITEM_WEAPON) {
      obj->obj_flags.value[2]=MAX(obj->obj_flags.value[2]-1,0);
    /*  obj->obj_flags.value[2]--; */  /* Linerfix 103103 - no negative dice on curse */
    }
    act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
  } else {
    if ( saves_spell(victim, SAVING_SPELL,level) ||
  affected_by_spell(victim, SPELL_CURSE))
      return;

    af.type      = SPELL_CURSE;
    af.duration  = 24*7;       /* 7 Days */
    af.modifier  = -1*((GET_LEVEL(ch)-3)/9);
    af.location  = APPLY_HITROLL;
    af.bitvector = AFF_CURSE;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_PARA;
    af.modifier = ((GET_LEVEL(ch)-3)/9); /* Make worse */
    affect_to_char(victim, &af);

    act("$n briefly reveals a red aura!", FALSE, victim, 0, 0, TO_ROOM);
    act("You feel very uncomfortable.",FALSE,victim,0,0,TO_CHAR);
  }
}

void spell_detect_alignment(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if ( affected_by_spell(victim, SPELL_DETECT_ALIGNMENT) )
    return;

  af.type      = SPELL_DETECT_ALIGNMENT;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_ALIGNMENT;
  af.bitvector2 = 0;

  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n\r", victim);
}

void spell_detect_invisibility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if ( affected_by_spell(victim, SPELL_DETECT_INVISIBLE) )
    return;

  af.type      = SPELL_DETECT_INVISIBLE;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVISIBLE;
  af.bitvector2 = 0;

  affect_to_char(victim, &af);

  send_to_char("Your eyes tingle.\n\r", victim);
}

void spell_detect_magic(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if ( affected_by_spell(victim, SPELL_DETECT_MAGIC) )
    return;

  af.type      = SPELL_DETECT_MAGIC;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;
  af.bitvector2 = 0;

  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n\r", victim);
}

void spell_detect_poison(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {

  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
  send_to_char("You can sense poison in your blood.\n\r", ch);
      else
  send_to_char("You feel healthy.\n\r", ch);
    else
      if (IS_AFFECTED(victim, AFF_POISON)) {
        act("You sense that $E is poisoned.",FALSE,ch,0,victim,TO_CHAR);
      } else {
        act("You sense that $E is healthy.",FALSE,ch,0,victim,TO_CHAR);
      }
  } else { /* It's an object */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3])
        act("Poisonous fumes are revealed.",FALSE, ch, 0, 0, TO_CHAR);
      else
        send_to_char("It looks delicious.\n\r", ch);
    }
  }
}

void spell_enchant_weapon(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int i,hit=1,dam=1,lim1,lim2,lim3,lim4,lim5,lim6;

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_DARK))
    return;

  if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

    for (i=0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
  return;

    SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

    if (level>=18) hit=2;
    if (level>=20) dam=2;
    if (level>=25) hit=3;
    if (level>=28) {hit=3;dam=3;}
    if (level>=40 && level<LEVEL_IMM) {
      /* New rolls for enchant - Ranger Oct 99
      dam hit   lvl 50%  lvl 40% (in between levels linear)
       5   5       5       1
       5   4      10       3
       4   5      15       6
       4   4      45      40
       4   3      19      30
       3   4       4      15
       3   3       2       5*/
                                 /* 50  40*/
      lim1=     level* 4/10-15;  /*  5   1*/
      lim2=lim1+level* 7/10-25;  /* 15   4*/
      lim3=lim2+level* 9/10-30;  /* 30  10*/
      lim4=lim3+level* 5/10+20;  /* 75  50*/
      lim5=lim4-level*11/10+74;  /* 94  80*/
      lim6=lim5-level*11/10+59;  /* 98  95*/

      i=number(1,100);
      hit=3;dam=3;
      if(i<=lim6) {hit=4;dam=3;}
      if(i<=lim5) {hit=3;dam=4;}
      if(i<=lim4) {hit=4;dam=4;}
      if(i<=lim3) {hit=5;dam=4;}
      if(i<=lim2) {hit=4;dam=5;}
      if(i<=lim1) {hit=5;dam=5;}
    }

    if(level>=LEVEL_IMM) {hit=4;dam=4;}
    if(level>=LEVEL_ETE) {hit=5;dam=5;}

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = hit;

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = dam;

    if (IS_GOOD(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_NEUTRAL);
      act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_NEUTRAL);
      act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
    } else {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
      act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
    }
  }
}

void spell_endure(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_ENDURE)) {

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
      send_to_char("You cannot cast this spell on another player.\n\r",ch);
      return;
    }

    af.type      = SPELL_ENDURE;
    af.duration  = 24;
    af.modifier  = -15;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);
    send_to_char("A feeling of protection envelops you.\n\r", victim);
  }
}

void spell_fear(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (IS_NPC(victim) && (GET_CLASS(victim) == CLASS_LESSER_UNDEAD || GET_CLASS(victim) == CLASS_GREATER_UNDEAD))
  {
    act("$n doesn't seem to notice your attempt to frighten $m.", FALSE, victim, 0, 0, TO_ROOM);
    return;
  }

  if ((IS_NPC(victim) && IS_SET(victim->specials.immune, IMMUNE_FEAR)) || affected_by_spell(victim, SPELL_TRANQUILITY)) return;

  if (saves_spell(victim, SAVING_PARA, level)) return;

  send_to_char("You are scared.\n\r", victim);
  act("$n is scared.", TRUE, victim, 0, 0, TO_ROOM);
  do_flee(victim, "", 0);
}

void spell_forget(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!IS_NPC(victim))
    return;

  if (saves_spell(victim, SAVING_SPELL,level))
    return;

  act("$n seems to forget something.",TRUE,victim,0,0,TO_ROOM);
  act("Uh, uhm, what's going on anyway?!!",TRUE,victim,0,0,TO_CHAR);

  clearMemory(victim);
}

void spell_fly(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_FLY)) {

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
      send_to_char("You cannot cast this spell on another player.\n\r",ch);
      return;
    }

    af.type      = SPELL_FLY;
    af.duration  = 12;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_FLY;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);

    if (GET_POS(victim) == POSITION_STANDING)
      GET_POS(victim) = POSITION_FLYING;

    send_to_char("You start to fly.\n\r", victim);
    act("$n starts to fly.", TRUE, victim,0,0,TO_ROOM);
  }
}

void spell_fury(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(victim, SPELL_FURY))
  {
    af.type       = SPELL_FURY;
    if (level >= 50 && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 4;
    else
      af.duration = 2;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = AFF_FURY;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);

    send_to_char("You feel very angry.\n\r", victim);
    act("$n starts snarling and fuming with fury.", FALSE, victim, 0, 0, TO_ROOM);
  }
}

void spell_heal(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (CHAOSMODE &&
      ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  spell_cure_blind(level, ch, victim, obj);

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      ((duration_of_spell(victim, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(victim, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", victim);
    return;
  }

  //GET_HIT(victim) = MIN(GET_HIT(victim) + MIN(level * 5, 200), GET_MAX_HIT(victim));
  magic_heal(victim, SPELL_HEAL, MIN(level * 5, 200), FALSE);
  send_to_char("A warm feeling fills your body.\n\r", victim);

  update_pos(victim);
}

void spell_mana_heal(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if(CHAOSMODE && ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  GET_MANA(victim) += 100;
  if(GET_MANA(victim) >= mana_limit(victim))
    GET_MANA(victim) = mana_limit(victim);
  update_pos( victim );
  send_to_char("You feel slightly regenerated\n\r", victim);
}

void spell_layhands(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) &&
      ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      ((duration_of_spell(victim, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(victim, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", victim);
    return;
  }

  //GET_HIT(victim) = MIN(GET_HIT(victim) + MIN(level * 10, 500), GET_MAX_HIT(victim));
  magic_heal(victim, SPELL_LAYHANDS, MIN(level * 10, 500), FALSE);
  send_to_char("A healing power flows into your body.\n\r", victim);

  update_pos(victim);
}

void spell_hold(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (saves_spell(victim, SAVING_PARA,level))
    return;

  if (!affected_by_spell(victim, SPELL_HOLD)) {
    act("$n is rooted to the ground.",TRUE,victim,0,0,TO_ROOM);
    act("You are rooted to the ground.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_HOLD;
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 1;
    else
      af.duration  = GET_LEVEL(ch);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_HOLD;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void spell_invisibility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (obj) {
    if ( !IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) ) {
      act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
      act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
    }
  } else {              /* Then it is a PC | NPC */
    if (!affected_by_spell(victim, SPELL_INVISIBLE)) {

      if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
        send_to_char("You cannot cast this spell on another player.\n\r",ch);
        return;
      }

      act("$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM);
      send_to_char("You vanish.\n\r", victim);

      af.type      = SPELL_INVISIBLE;
      af.duration  = 24;
      af.modifier  = -40;
      af.location  = APPLY_AC;
      af.bitvector = AFF_INVISIBLE;
      af.bitvector2 = 0;
      affect_to_char(victim, &af);
    }
  }
}

void spell_improved_invisibility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj){
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_IMP_INVISIBLE)) {

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim))) {
      act("$n slowly distorts into a chaotic shadow.", TRUE, victim,0,0,TO_ROOM);
      send_to_char("The forces of chaos distort your image.\n\r", victim);
    }
    else
    {
      act("$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM);
      send_to_char("You vanish.\n\r", victim);
    }

    af.type      = SPELL_IMP_INVISIBLE;
    af.duration  = GET_LEVEL(victim)/10;
    af.modifier  = -40;
    af.location  = APPLY_AC;
    af.bitvector = ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 0 : AFF_IMINV;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void spell_remove_improved_invis(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj){
  if (affected_by_spell(victim, SPELL_IMP_INVISIBLE)) {
    act("$n slowly fades into existence.", TRUE, victim,0,0,TO_ROOM);
    send_to_char("You become visible to the world.\n\r", victim);
    affect_from_char(victim,SPELL_IMP_INVISIBLE);
  }
}

void spell_locate_object(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  OBJ *i;
  char name[256];
  char buf[MAX_STRING_LENGTH];
  int j;

  assert(ch && obj);

  if(!obj) {
    send_to_char("That object does not exist.\n\r",ch);
    return;
  }

  one_argument ((char*)obj, name);

  if(!IS_NPC(ch)) level=MIN(level,50);

  j=level>>1;

  if(IS_SET(ch->specials.pflag, PLR_QUEST)) {
    sprintf(buf,"QSTINFO: %s casts 'locate object' %s",GET_NAME(ch),name);
    wizlog(buf,LEVEL_IMM,7);
  }

  for (i = object_list; i && (j>0); i = i->next)
    if (isname(name, OBJ_NAME(i)) /*&& !IS_SET(i->obj_flags.extra_flags,ITEM_ANTI_AUCTION)*/ &&
        i->obj_flags.type_flag!=ITEM_SC_TOKEN && number(0,1) &&
        !IS_SET(i->obj_flags.extra_flags2,ITEM_NO_LOCATE)) {

      if(i->carried_by) {
        sprintf(buf,"%s carried by %s.\n\r",
    OBJ_SHORT(i),PERS(i->carried_by,ch));
        send_to_char(buf,ch);
      } else if (i->in_obj) {
        if(i->in_obj->in_room==real_room(1201)) continue;
        if(i->in_obj->in_room==real_room(1204)) continue;
        sprintf(buf,"%s in %s.\n\r",OBJ_SHORT(i),
    OBJ_SHORT(i->in_obj));
        send_to_char(buf,ch);
      } else if (i->equipped_by) {
  if (strlen(PERS(i->equipped_by, ch))> 0) {
    sprintf(buf, "%s equipped by %s.\n\r",
      OBJ_SHORT(i), PERS(i->equipped_by, ch));
    send_to_char(buf, ch);
  }
    } else {
      if(i->in_room==real_room(1201)) continue;
      if(i->in_room==real_room(1204)) continue;
        sprintf(buf,"%s in %s.\n\r",OBJ_SHORT(i),
    (i->in_room == NOWHERE ? "Used but uncertain." : world[i->in_room].name));
        send_to_char(buf,ch);
      }
      j--;
    }

  if(j==0)
    send_to_char("You are very confused.\n\r",ch);
  if(j==level>>1)
    send_to_char("No such object.\n\r",ch);
}

void spell_locate_character(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  char buf[MIL];

  if (IS_NPC(ch))
    return;

  if(!victim) {
    send_to_char("Nobody playing by that name.\n\r", ch);
    return;
  }

  if(IS_SET(ch->specials.pflag, PLR_QUEST)) {
    sprintf(buf,"QSTINFO: %s casts 'locate char' %s",GET_NAME(ch),GET_NAME(victim));
    wizlog(buf,LEVEL_IMM,7);
  }

  if (!CAN_SEE(ch, victim) || IS_SET(victim->specials.immune2,IMMUNE_LOCATE)) {
    send_to_char("Nobody playing by that name.\n\r", ch);
    return;
  }

  if (GET_LEVEL(ch)<LEVEL_IMM && IS_AFFECTED(victim, AFF_IMINV) && CHAOSMODE) {
    send_to_char("Nobody playing by that name.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
    sprintf(buf, "%s in %s\n\r", MOB_SHORT(victim),
      world[CHAR_REAL_ROOM(victim)].name);
  else
    sprintf(buf, "%s in %s\n\r", victim->player.name,
      world[CHAR_REAL_ROOM(victim)].name);

  send_to_char(buf, ch);
}
/*
New locate char to list more than one mob - not implemented yet

void spell_locate_character(ubyte level, CHAR *ch, struct char_data *victim, OBJ *obj) {
  register struct char_data *i;
  char buf[MAX_STRING_LENGTH], name[MAX_INPUT_LENGTH];
  int j;

  if (IS_NPC(ch))
    return;

  one_argument ((char*)victim, name);

  j=level>>1;

  for (i = character_list; i && (j>0); i = i->next) {
    if (isname(name, GET_NAME(i)) && number(0,1) && CAN_SEE(ch,i)) {
  if (IS_NPC(i))
    sprintf(buf, "%s in %s\n\r", MOB_SHORT(i),
      world[CHAR_REAL_ROOM(i)].name);
  else
    sprintf(buf, "%s in %s\n\r", GET_NAME(i),
      world[CHAR_REAL_ROOM(i)].name);
    send_to_char(buf, ch);
          j--;
  }
    }

  if(j==0)
    send_to_char("You are very confused.\n\r",ch);
  if(j==level>>1)
    send_to_char("No-one by that name around.\n\r",ch);
}
*/

void spell_poison(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }
  if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_POISON)) return;

  if (victim) {

    if(!saves_spell(victim, SAVING_PARA,level)) {
      af.type = SPELL_POISON;
      if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
        af.duration = 1;
      else
        af.duration = level*2;
      af.modifier = -3;
      af.location = APPLY_STR;
      af.bitvector = AFF_POISON;
      af.bitvector2 = 0;

      affect_join(victim, &af, FALSE, FALSE);
      send_to_char("You feel very sick.\n\r", victim);
    }
  } else { /* Object poison */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      act("$p turns green!",FALSE,ch,obj,0,TO_CHAR);
      obj->obj_flags.value[3] = 1;
    }
  }
}

void spell_poison_smoke(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  act("$n has blown a lot of poisonous smoke out!", TRUE, ch, 0, 0, TO_ROOM);
  act("You blow a lot of poisonous smoke out!", FALSE, ch, obj, 0, TO_CHAR);

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if(IS_NPC(ch) && !IS_NPC(tmp_victim) && GET_LEVEL(tmp_victim)>LEVEL_MORT)
      continue;
    if(IS_NPC(ch) && IS_NPC(tmp_victim) && inzone(V_MOB(ch))==inzone(V_MOB(tmp_victim)) )
      continue;
    if(ch==tmp_victim)
      continue;
    if (!IS_NPC(ch) && !IS_NPC(tmp_victim) && !(!IS_SET(ch->specials.pflag, PLR_NOKILL) || ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))))
      continue;
    spell_poison(level, ch, tmp_victim, 0);
  }
}

void spell_protection_from_evil(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj){
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) ) {

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
      send_to_char("You cannot cast this spell on another player.\n\r",ch);
      return;
    }

    af.type      = SPELL_PROTECT_FROM_EVIL;
    af.duration  = 12;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PROTECT_EVIL;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
    send_to_char("You have a righteous feeling!\n\r", victim);
  }
}

void spell_protection_from_good(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj){
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_GOOD) ) {

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
      send_to_char("You cannot cast this spell on another player.\n\r",ch);
      return;
    }

    af.type      = SPELL_PROTECT_FROM_GOOD;
    af.duration  = 12;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PROTECT_GOOD;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
    send_to_char("A cloak of chaotic energy surrounds you.\n\r", victim);
  }
}

void spell_recharge(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
    int tmpnum = 0;
  if((GET_ITEM_TYPE(obj) == ITEM_STAFF) || (GET_ITEM_TYPE(obj) == ITEM_WAND)) {
      if (GET_CLASS(ch) == CLASS_MAGIC_USER) {
        tmpnum = 60;
      } else {
        tmpnum = 50;
      }

      if (chance(tmpnum)) {
      obj->obj_flags.value[2] = obj->obj_flags.value[1];
      act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
        act("$p glows yellow.",FALSE,ch,obj,0,TO_ROOM);
    } else {
      act("$p exploded.", FALSE, ch, obj, 0, TO_CHAR);
      act("You hurt yourself.", FALSE, ch, obj, 0, TO_CHAR);
      act("$p exploded.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
      if (GET_LEVEL(ch) < LEVEL_IMM) {
        if (GET_HIT(ch) < 30)
          GET_HIT(ch) = 1;
        else
          GET_HIT(ch) -= 30;
      }
    }
  }
}

void spell_remove_curse(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {

  if (obj) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_EVIL) ||
  IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
      act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);

      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
    }
  } else {      /* Then it is a PC | NPC */
    if (affected_by_spell(victim, SPELL_CURSE) ) {

      if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
        send_to_char("You cannot cast this spell on another player.\n\r",ch);
        return;
      }

      act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM);
      act("You feel better.",FALSE,victim,0,0,TO_CHAR);
      affect_from_char(victim, SPELL_CURSE);
    }
  }
}

void spell_remove_poison(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {

  if (victim) {
    if(affected_by_spell(victim,SPELL_POISON)) {

      if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
        send_to_char("You cannot cast this spell on another player.\n\r",ch);
        return;
      }

      affect_from_char(victim,SPELL_POISON);
      act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
      act("$n looks better.",FALSE,victim,0,0,TO_ROOM);
    }
  } else {
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
      act("$p glows briefly.",FALSE,ch,obj,0,TO_CHAR);
    }
  }
}

void spell_sanctuary(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    return;

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
    send_to_char("You cannot cast this spell on another player.\n\r",ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_SANCTUARY)) {
    act("$n is surrounded by a white aura.",TRUE,victim,0,0,TO_ROOM);
    act("You start glowing.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_SANCTUARY;
    af.duration  = level / 5;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SANCTUARY;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void spell_satiate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {

  /* Linerfix adding check for Gluttony affect */
  if(enchanted_by(victim,"Deadly Sin - Gluttony")) {
    act("You are repulsed by the thought of food!",TRUE,victim,0,0,TO_CHAR);
    act("$n is repulsed by the thought of food!",TRUE,victim,0,0,TO_ROOM);
    return;
  }

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && victim && victim->specials.fighting && ch!=victim->specials.fighting && IS_MORTAL(victim->specials.fighting) && victim!=ch) {  /* Chaos03 */
    send_to_char("You cannot cast this spell on one who is already fighting another person!\n\r",ch);
    return;
  }

  act("$n is glowing with a bright light.", TRUE, victim, 0, 0, TO_ROOM);
  act("You are full.", TRUE, victim, 0,0,TO_CHAR);

  if (GET_COND(victim,THIRST) > -1) {
    GET_COND(victim, THIRST) = 24;
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim))) GET_COND(victim, THIRST) = 22;
  }
  if (GET_COND(victim,FULL) > -1) {
    GET_COND(victim, FULL) = 24;
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim))) GET_COND(victim, FULL) = 22;
  }
}

void spell_sleep(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if(IS_NPC(victim) && (IS_SET(victim->specials.immune,IMMUNE_SLEEP) ||
  GET_CLASS(victim)==CLASS_LESSER_UNDEAD || GET_CLASS(victim)==CLASS_GREATER_UNDEAD)) {
    act ("$n gets angry and attacks...",FALSE,victim,0,0,TO_ROOM);
    hit (victim, ch, TYPE_UNDEFINED);
    return;
  }

  if (!saves_spell(victim, SAVING_SPELL,level)) {
    af.type      = SPELL_SLEEP;
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
      af.duration  = 1;
    else
      af.duration  = 4+level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    af.bitvector2 = 0;
    affect_join(victim, &af, FALSE, FALSE);

    if (GET_POS(victim)>POSITION_SLEEPING) {
      act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR);
      act("$n falls asleep.",TRUE,victim,0,0,TO_ROOM);
      GET_POS(victim) = POSITION_SLEEPING;
    }

    return;
  } else if (IS_NPC(victim)) {
    act ("$n gets angry and attacks...",FALSE,victim,0,0,TO_ROOM);
    hit (victim, ch, TYPE_UNDEFINED);
  }
}

void spell_hypnotize(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int percent;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("This isn't going to work, you know!\n\r", ch);
    return;
  }

  if (!IS_NPC(victim)) {
    send_to_char("You can't hypnotize the other players.\n\r", ch);
    return;
  }

  percent = number (0,1);

  if (percent)
    spell_sleep (GET_LEVEL(ch),ch,victim,0);
  else
    spell_charm_person (GET_LEVEL(ch),ch,victim,0);
}

void spell_sphere(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_SPHERE)) {

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
      send_to_char("You cannot cast this spell on another player.\n\r",ch);
      return;
    }

    act("$n is surrounded by a golden sphere.",TRUE,victim,0,0,TO_ROOM);
    act("You are surrounded by a golden sphere.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_SPHERE;
    af.duration  = level / 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SPHERE;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void spell_invulnerability(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_INVUL)) {

    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
      send_to_char("You cannot cast this spell on another player.\n\r",ch);
      return;
    }

    act("$n is surrounded by a powerful sphere.",TRUE,victim,0,0,TO_ROOM);
    act("You are surrounded by a powerful sphere.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_INVUL;
    af.duration  = level / 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_INVUL;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void spell_strength(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  act("You feel stronger.", FALSE, victim,0,0,TO_CHAR);
  if(GET_STR(victim)<100) { /* to prevent looping - Ranger Feb 2001 */
    af.type      = SPELL_STRENGTH;
    af.duration  = level;
    af.modifier  = 1+(level>27);
    af.location  = APPLY_STR;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_join(victim, &af, TRUE, FALSE);
  }
}

void spell_ventriloquate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  /* Not possible!! No argument! */
}

void spell_word_of_recall(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int location;
  int org_room;
  CHAR *vict;
  bool mount;

/* Fix for Sin_Pride to not allow totals or recalls - Liner 041303 */
  if(enchanted_by(victim,"Deadly Sin - Pride")) return;

/* Code changes have been made so that you can recall your charmies
if your not riding them and so that they will total with you if you
are riding them. (For Stables) Ranger April 1996 */

  if (IS_NPC(victim) && !(IS_AFFECTED(victim, AFF_CHARM)) ) return;
  if (victim->specials.rider) return;
  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, CHAOTIC)) {
    send_to_char("Chaotic energies disrupt your magic!.\n\r",ch);
    return;
   }
  mount=FALSE;
  org_room = CHAR_REAL_ROOM(victim);
  if(victim->specials.riding) {
    if (org_room == CHAR_REAL_ROOM(victim->specials.riding)) {
      vict = victim->specials.riding;
      mount=TRUE;
    }
  }

  /* If home locations are later added, a routine would need to be added
to find the home of the charmies master - Ranger April 1996 */
  /*  location = real_room(GET_HOME(ch)); */

  location=real_room(3001);

  if(location == NOWHERE) {
    send_to_char("You are completely lost.\n\r", victim);
    return;
  }

  /* a location has been found. */

  if (mount) act("$n disappears with $s mount.", TRUE, victim, 0, 0, TO_ROOM);
  else act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  if(victim->specials.fighting)
    stop_fighting(victim);
  char_to_room(victim, location);
  /* linkdead players in battle won't return to battle area if recalled - Ranger June 2000 */
  victim->specials.was_in_room=NOWHERE;
  if (mount) {
     char_from_room(vict);
     if(vict->specials.fighting)
       stop_fighting(vict);
     char_to_room(vict,location);
     vict->specials.was_in_room=NOWHERE;
  }
  if(!CHAOSMODE) {
  if(mount) act("$n appears in the middle of the room with $s mount.", TRUE, victim, 0, 0, TO_ROOM);
  else act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  }                    /* Liner - During Chaos, make temple quiet to entry */
  do_look(victim, "",15);
}

void spell_total_recall(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if (tmp_victim != ch && GET_LEVEL(tmp_victim) < LEVEL_IMM)
      spell_word_of_recall(level, ch, tmp_victim, 0);
  }
  spell_word_of_recall(level, ch, ch, 0);
}

void spell_summon(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  sh_int target;
  int percent;
  char buf[MIL];

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_SUM))
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }
  
  /* Fix for Sin_Pride to not allow totals or recalls - Liner 041303 */
  if(enchanted_by(victim,"Deadly Sin - Pride")) {
    send_to_char("You failed.\n\r",ch);
    return;
  }

  if(CHAR_REAL_ROOM(victim)==NOWHERE) {
    send_to_char("You failed.\n\r",ch);
    return;
  }

  if(IS_SET(ch->specials.pflag, PLR_QUEST)) {
    sprintf(buf,"QSTINFO: %s casts 'summon' %s",GET_NAME(ch),GET_NAME(victim));
    wizlog(buf,LEVEL_IMM,7);
  }

  /* Check for mount added - Ranger */
  if (IS_NPC(victim) && (GET_LEVEL(victim) > 15) &&
     !IS_SET(victim->specials.act,ACT_MOUNT)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  /* Immune summon added Aug 28/98 - Ranger */
  if(IS_SET(victim->specials.immune,IMMUNE_SUMMON)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  /* So you can't summon a ridden mount Ranger April 96 */
  if (victim->specials.rider) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  /* New summon failures - Ranger April 2000 */
  percent=100;
  percent-=5*(GET_LEVEL(victim)-GET_LEVEL(ch));
  if(affected_by_spell(victim, SPELL_BLESS)) percent+=5;
  if(affected_by_spell(victim, SPELL_CURSE)) percent-=5;
  percent=MIN(percent,100);
  percent=MAX(0,percent);

  if(!chance(percent) || (ch==victim)) {
    send_to_char("You failed.\n\r",ch);
    return;
  }

  if (!IS_NPC(victim) && (IS_SET(victim->specials.pflag, PLR_KILL) ||
        IS_SET(victim->specials.pflag, PLR_THIEF)) && !CHAOSMODE) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if ((IS_SET(victim->specials.pflag, PLR_NOSUMMON)) && (!IS_NPC(victim))) {
    send_to_char("You failed.\n\r",ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, PRIVATE) ||
      IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, PRIVATE) ||
      IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA) ||
      IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) ||
      IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, CHAOTIC) ||
      (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, TUNNEL) && !CHAOSMODE) ||
      IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE) ||
      (V_ROOM(victim)==10) ) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (IS_NPC(victim) && (saves_spell(victim, SAVING_SPELL,level) ||
       IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE))) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  act("$n disappears suddenly.",TRUE,victim,0,0,TO_ROOM);

  if(victim->specials.riding) stop_riding(victim,victim->specials.riding);

  target = CHAR_REAL_ROOM(ch);
  char_from_room(victim);
  if(victim->specials.fighting)
    stop_fighting(victim);
  char_to_room(victim,target);
  /* linkdead players in battle won't return to battle area if summoned - Ranger June 2000 */
  victim->specials.was_in_room=NOWHERE;

  act("$n arrives suddenly.",TRUE,victim,0,0,TO_ROOM);
  act("$n has summoned you!",FALSE,ch,0,victim,TO_VICT);
  do_look(victim,"",15);
  GET_POS(victim) = POSITION_RESTING;
}

void spell_relocation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  sh_int target;

  if (IS_NPC(victim)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, LAWFULL) ||
      IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, LOCK) ||
      IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, CHAOTIC) ||
      (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, TUNNEL) && !CHAOSMODE) ||
      IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, NO_MAGIC) ||
      IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, PRIVATE) ||
      IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, NO_SUM) ||
      (GET_LEVEL(victim) >= LEVEL_IMM)) {
    send_to_char("You failed.\n\r",ch);
    return;
  }

  act("$n disappears suddenly.",TRUE,ch,0,0,TO_ROOM);

  if(ch->specials.riding) stop_riding(ch,ch->specials.riding);
  if(ch->specials.rider) stop_riding(ch->specials.rider,ch);

  target = CHAR_REAL_ROOM(victim);
  char_from_room(ch);
  if(ch->specials.fighting)
    stop_fighting(ch);
  char_to_room(ch,target);

  act("$n relocates to here.",TRUE,ch,0,0,TO_ROOM);
  act("You relocate to $N!",FALSE,ch,0,victim,TO_CHAR);
  do_look(ch,"",15);
}

void spell_charm_person(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  void add_follower(CHAR *ch, CHAR *leader);
  bool circle_follow(CHAR *ch, CHAR *victim);

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

  if (!IS_NPC(victim)) {
    send_to_char("You can't charm the other players.\n\r", ch);
    return;
  }

  if(IS_NPC(victim) && (IS_SET(victim->specials.immune,IMMUNE_CHARM) ||
  GET_CLASS(victim)==CLASS_LESSER_UNDEAD || GET_CLASS(victim)==CLASS_GREATER_UNDEAD)) return;

  if (GET_MAX_HIT(victim) > 1000) {
    return;
  }

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM) &&
      (level >= GET_LEVEL(victim)) && GET_LEVEL(victim)<31) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }

    if (saves_spell(victim, SAVING_SPELL,level))
      return;

    /* Addition of mob follow limit - Ranger April 98 */
    if(count_mob_followers(ch)>9) {
      send_to_char("You can't control anymore followers.\n\r",ch);
      return;
    }

    if (victim->master)
      stop_follower(victim);


    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;

    if (GET_INT(victim)) af.duration  = 24*18/GET_INT(victim);
    else af.duration  = 24*18;

    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);

    act("Say, isn't $n just adoring?",FALSE,ch,0,victim,TO_VICT);
  }
}

void spell_sense_life(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_SENSE_LIFE)) {
    send_to_char("Your feel your awareness improve.\n\r", ch);

    af.type      = SPELL_SENSE_LIFE;
    af.duration  = 5 * level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SENSE_LIFE;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void spell_vitality(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch!=victim) { /* Chaos03 */
    send_to_char("You cannot cast this spell on another player.\n\r",ch);
    return;
  }

  GET_MOVE(victim) = MIN(move_limit(victim), 32768);

  send_to_char("You feel refreshed!\n\r", victim);
}

void spell_miracle(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, NO_MAGIC))
  {
    send_to_char("The magic of the miracle has been absorbed by your surroundings.\n\r", victim);
    return;
  }

  if (CHAOSMODE &&
      ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      ((duration_of_spell(victim, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(victim, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", victim);
    return;
  }

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
  {
    //GET_HIT(victim) = MIN(GET_HIT(victim) + 1500, GET_MAX_HIT(victim));
    magic_heal(victim, SPELL_MIRACLE, 1500, FALSE);
    send_to_char("The magic of the miracle has been manipulated by the chaos around you.\n\r", victim);
  }
  else
  {
    //GET_HIT(victim) = MIN(GET_HIT(victim) + 2000, GET_MAX_HIT(victim));
    magic_heal(victim, SPELL_MIRACLE, 2000, FALSE);
    send_to_char("Your life has been restored.\n\r", victim);
  }

  signal_char(victim, ch, MSG_MIRACLE, "");
  update_pos(victim);
}

void spell_recover_mana(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {

  if(CHAOSMODE && ch!=victim) {
    send_to_char("You cannot cast this spell on another player.\n\r",ch);
    return;
  }

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim))) {
    GET_MANA(victim) = MIN(GET_MANA(victim)+750,mana_limit(victim));
    send_to_char("The forces of chaos have partially restored your magic.\n\r",victim);
  }
  else {
    GET_MANA(victim) = mana_limit(victim);
    send_to_char("Your magic has been restored.\n\r", victim);
  }
}

/* void check_equipment(CHAR *ch); */
void spell_spirit_levy(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  int heal = 0;

  if (!(GET_ITEM_TYPE(obj) == ITEM_CONTAINER) || OBJ_VALUE3(obj) != 1)
  {
    send_to_char("You failed.\n\r", ch);

    return;
  }

  if (OBJ_COST(obj) == PC_CORPSE && obj->contains)
  {
       send_to_char("The corpse has something in it.\n\r", ch);

       return;
    }

  heal = OBJ_VALUE2(obj) * 3;
  magic_heal(ch, SPELL_SPIRIT_LEVY, OBJ_VALUE2(obj) * 3, FALSE);

  GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - heal);

    send_to_char("You absorb life energy from the dead.\n\r", ch);
    act("$n absorbs life energy from the dead.", TRUE, ch, 0, 0, TO_ROOM);

    extract_obj(obj);
    update_pos(ch);
  }

void spell_legend_lore(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  char buf[256];
  int i, avg, loc, mod, bitfield;
  int much = 0, modify = 0;
  int number(int, int);


  extern char *spells[];

  const char *other_msg[] = { "greatly", "a little", "improve", "weaken" };

   const char *type_msg[] = {
    "",
    "a source of light",  "a scroll",            "a wand",
    "a staff",            "a weapon",            "a ranged weapon",
    "a missile",          "a treasure",          "a piece of armor",
    "a potion",           "clothing",            "an item...",
    "a piece of trash",   "different",           "a carrying container",
    "a piece of paper",   "a drink container",   "a key",
    "food",               "a pile of coins",     "a pen",
    "a boat",             "a bullet",            "a musical instrument",
    "used to pick locks","",""
  };

  const char *apply_msg[] = {
    "none", "strength", "agility", "intelligence", "wisdom", "constitution",
    "", "", "", "true age", "weight", "height", "mental strength",
    "physical toughness", "physical endurance", "", "",
    "protection against physical attacks", "ability to hit your opponent",
    "ability to physically harm your opponent",
    "protection against attacks on your body", "protection against magic",
    "protection against attacks on your body",
    "protection against dragon breath", "protection against magic",
    "ability to move silently", "ability to hide", "ability to steal",
    "ability to backstab", "ability to open locks",
    "ability to kick more effectively", "ability to bash more effectively",
    "ability to rescue your friends",
    "ability to prevent others from fleeing", "ability to force doors open",
    "ability to punch", "ability to parry more attacks",
    "ability to use more attacks", "ability to throw more accurately",
    "ability to dodge attacks", "ability to peek around corners",
    "ability to make food from corpses", "ability to set traps",
    "ability to disarm your opponent", "ability to subdue mounts",
    "ability to circle around in melee", "ability to use more attacks","",""
  };

  const int apply_val[] = {
    0, 3, 3, 3, 3, 3, 0, 0, 0, 6, 10, 10, 30, 30, 30, 0, 0, 6, 4, 4,
    3, 3, 3, 3, 3, 15, 15, 15, 15, 15, 20, 20, 15, 15, 20, 20, 20, 15,
    15, 15, 15, 15, 15, 20, 15, 15, 15
  };

  const char *affect_msg[] = {
    "it gives you nothing.\n",
    "it prevents you from seeing anything.\n",
    "it makes you disappear from the eyes of the common man.\n",
    "it enables you to determine the belief of creatures.\n",
    "it enables you to discover the presence of invisible creatures.\n",
    "it enables you to detect magical auras.\n",
    "it enables you to discover the presence of hidden creatures.\n",
    "it will restrict your free movement.\n",
    "it will protect you from great amounts of physical harm.\n",
    "",
    "",
    "it will curse your mind and your hands to fail where they should not.\n",
    "it will give you a protective aura against some forms of attack.\n",
    "it will poison your blood.\n",
    "it will protect you from the powers of evil creatures.\n",
    "it will paralyse you.\n",
    "it will make you able to see in total darkness.\n",
    "",
    "it will make you fall asleep.\n",
    "",
    "it will make you move more silently.\n",
    "it will enable you to hide from others.\n",
    "",
    "",
    "",
    "it will make you able to fly.\n",
    "",
    "it will make you truly disappear from the face of the earth.\n",
    "it will give you a powerful protective aura against most magic.\n",
    "",
    "it will make you more powerful in combat.\n","",""
  };

  assert(ch && (obj || victim));

  if (obj) {
    send_to_char("Legends indicate...\n\r", ch);
    sprintf(buf, "%s is %s.\n\r", CAP(OBJ_SHORT(obj)),
      type_msg[GET_ITEM_TYPE(obj)]);
    send_to_char (buf,ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item grant the following abilities...\n\r",ch);
      for (bitfield = 1, i=1;bitfield;bitfield = bitfield<<1, i++) {
  if (IS_SET(obj->obj_flags.bitvector, bitfield))
    send_to_char ((char*)affect_msg[i],ch);
      }
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC))
      send_to_char("...magical in its nature.\n\r",ch);

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE))
      send_to_char("...invisible.\n\r",ch);

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_BLESS))
      send_to_char("...blessed with a holy aura.\n\r",ch);

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP))
      send_to_char("...cursed with an evil aura.\n\r",ch);

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MORTAL))
      send_to_char("...not for the mortal man.\n\r",ch);

    i=0;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MALE)) i+=2;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FEMALE)) i+=2;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_WARRIOR)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGIC_USER)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PALADIN)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_ANTIPALADIN)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_NINJA)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_NOMAD)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_AVATAR)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_BARD)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_COMMANDO)) i++;
    if (i) i = i + 3 - number (0,6);
    if (i >= 8) send_to_char("...very restricted in its use.\n\r",ch);
    else if (i > 0) send_to_char("...restricted in its use.\n\r",ch);

    i = 0;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_NEUTRAL)) i++;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD)) i++;
    if (i) i = i + 1 - number (0,2);
    if (i >= 2) send_to_char("...strongly aligned.\n\r",ch);
    else if (i > 0) send_to_char("...aligned.\n\r",ch);

    for (i=0;i<MAX_OBJ_AFFECT;i++) {
      if (obj->affected[i].location && obj->affected[i].modifier) {
  loc = obj->affected[i].location;
  mod = obj->affected[i].modifier;

  if (loc == APPLY_DAMROLL && GET_ITEM_TYPE(obj) == ITEM_WEAPON)
    continue;

  if (loc == APPLY_ARMOR)
    mod = -mod;

         if (mod > 0) modify = 2;
  else if (mod < 0)  modify= 3;
  else return;

  if (mod >= apply_val[loc]) much = 0;
  else much = 1;

  sprintf(buf,"...it will %s your %s %s.\n\r",
    other_msg[modify],apply_msg[loc],other_msg[much]);
  send_to_char(buf, ch);
      }
    }

    switch (GET_ITEM_TYPE(obj)) {
    case ITEM_SCROLL :
    case ITEM_POTION :
      sprintf(buf, "There are following spells on the item...\n\r");
      send_to_char (buf,ch);
      for (i=1;i<4;i++) {
  if (obj->obj_flags.value[i] >= 1) {
    strcat(buf,"...");
    sprinttype(obj->obj_flags.value[i]-1,spells,buf);
    strcat(buf,"\n\r");
    send_to_char(buf, ch);
  }
      }
      break;

    case ITEM_WAND :
    case ITEM_STAFF :
      sprintf(buf, "...has %d charges left of the following spell...\n\r",
        obj->obj_flags.value[1]);
      send_to_char (buf,ch);
      if (obj->obj_flags.value[3] >= 1) {
  sprinttype(obj->obj_flags.value[3]-1,spells,buf);
  strcat(buf,"\n\r");
  send_to_char(buf, ch);
      }
      break;

    case ITEM_WEAPON :
      avg = 0;
      avg = (obj->obj_flags.value[1] +
       obj->obj_flags.value[1]*obj->obj_flags.value[2])>>1;

      for (i=0;i<MAX_OBJ_AFFECT;i++) {
  if (obj->affected[i].location == APPLY_DAMROLL &&
      obj->affected[i].modifier)
    avg = avg + obj->affected[i].modifier;
      }

      if (avg>25)
  send_to_char("This weapon does superb damage.\n\r", ch);
      else if (avg>20)
  send_to_char("This weapon does excellent damage.\n\r", ch);
      else if (avg>15)
  send_to_char("This weapon does good damage.\n\r", ch);
      else if (avg>10)
  send_to_char("This weapon does fair damage.\n\r", ch);
      else if (avg>5)
  send_to_char("This weapon does poor damage.\n\r", ch);
      else send_to_char("This weapon does feeble damage.\n\r", ch);
      break;

    case ITEM_MISSILE :
      avg = (obj->obj_flags.value[1] +
       obj->obj_flags.value[1]*obj->obj_flags.value[2])>>1;

      if (avg>100)
  send_to_char("This missile does superb damage.\n\r", ch);
      else if (avg>50)
  send_to_char("This missile does excellent damage.\n\r", ch);
      else if (avg>25)
  send_to_char("This missile does good damage.\n\r", ch);
      else if (avg>12)
  send_to_char("This missile does fair damage.\n\r", ch);
      else if (avg>6)
  send_to_char("This missile does poor damage.\n\r", ch);
      else send_to_char("This missile does weak damage.\n\r", ch);

      break;

    case ITEM_ARMOR :
      avg = obj->obj_flags.value[0];
      if (avg>14)
  send_to_char("This piece of armour gives superb protection.\n\r", ch);
      else if (avg>11)
  send_to_char("This piece of armour gives excellent protection.\n\r",ch);
      else if (avg>8)
  send_to_char("This piece of armour gives good protection.\n\r", ch);
      else if (avg>5)
  send_to_char("This piece of armour gives fair protection.\n\r", ch);
      else if (avg>2)
  send_to_char("This piece of armour gives poor protection.\n\r", ch);
      else if (avg>0)
  send_to_char("This piece of armour gives weak protection.\n\r", ch);
      else send_to_char("This piece of armour gives no protection at all.\n\r",
      ch);
      break;

    case ITEM_BULLET :
      sprintf(buf, "For the gun with license number %d\n\r",
        obj->obj_flags.value[2]);
      send_to_char(buf, ch);
      break;

    case ITEM_FIREWEAPON :
      sprintf(buf,"License Number: %d\nNumber of bullets: %d\n",
        obj->obj_flags.value[0], obj->obj_flags.value[1]);
      send_to_char(buf, ch);
      avg = (obj->obj_flags.value[2] +
       obj->obj_flags.value[2]*obj->obj_flags.value[3])>>1;

      if (avg>500)
  send_to_char("This ranged weapon does superb damage.\n\r", ch);
      else if (avg>250)
  send_to_char("This ranged weapon does excellent damage.\n\r", ch);
      else if (avg>120)
  send_to_char("This ranged weapon does good damage.\n\r", ch);
      else if (avg>60)
  send_to_char("This ranged weapon does fair damage.\n\r", ch);
      else if (avg>30)
  send_to_char("This ranged weapon does poor damage.\n\r", ch);
      else send_to_char("This ranged weapon does weak damage.\n\r", ch);
      break;
    }
  } else {
    send_to_char("You learn nothing...\n\r", ch);
  }
}


void spell_identify(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
    char buf[256], buf2[256];
    int i;
    int iHP=0;
    int iDMG=0;
    int iLEVEL=0;
    int iMATT=0;
    int iSPEC=0;
    int avgdmg=0;
    struct obj_data *wielded=0;
    bool found;
    char *decay_text[10]=
       {"like new","almost new","fairly new","slightly worn","worn",
        "fairly worn","very worn","slightly cracked","cracked",
        "about to crumble"};
    int val3,max_time;

    struct time_info_data age(CHAR *ch);

    /* Spell Names */
    extern char *spells[];

    /* For Objects */
    extern char *item_types[];
    extern char *extra_bits[];
    extern char *extra_bits2[];
    extern char *apply_types[];
    extern char *affected_bits[];
    extern char *affected_bits2[];

    extern char *pc_class_types[];
    extern char *npc_class_types[];
    extern char *wpn_spc[];
    extern int mob_id_hp_int[];
    extern char *mob_id_hp_text[];
    extern int mob_id_dmg_int[];
    extern char *mob_id_dmg_text[];
    extern int mob_id_level_int[];
    extern char *mob_id_level_text[];
    extern int mob_id_matt_int[];
    extern char *mob_id_matt_text[];
    extern char *mob_id_spec[];

    if (obj) {
        send_to_char("You feel informed...\n\r", ch);

        sprintf(buf, "Object: '%s', Keywords: '%s'",
            (OBJ_SHORT(obj) ? OBJ_SHORT(obj) : "None"), OBJ_NAME(obj));
        strcat(buf,"\n\r");
        send_to_char(buf,ch);

        sprintf(buf, "Type: ");
        sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
        strcat(buf,buf2);
        strcat(buf,"\n\r");
        send_to_char(buf, ch);

        send_to_char("Item is: ", ch);
        sprintbit(obj->obj_flags.extra_flags,extra_bits,buf);
        strcat(buf," ");
        send_to_char(buf,ch);
        sprintbit(obj->obj_flags.extra_flags2,extra_bits2,buf);
        strcat(buf,"\n\r");
        send_to_char(buf,ch);

        if( obj->obj_flags.subclass_res )
        {
          send_to_char("Subclass restrictions: ", ch);
          sprintbit(obj->obj_flags.subclass_res,subclass_res_bits,buf);
          strcat(buf,"\n\r");
          send_to_char(buf,ch);
        }

        if (obj->obj_flags.bitvector) {
            send_to_char("Item has been imbued with: ", ch);
            sprintbit(obj->obj_flags.bitvector,affected_bits,buf);
            strcat(buf,"\n\r");
            send_to_char(buf, ch);
        }

        if (obj->obj_flags.bitvector2) {
            sprintbit(obj->obj_flags.bitvector2,affected_bits2,buf);
            strcat(buf,"\n\r");
            send_to_char(buf, ch);
        }

        strcpy(buf,"Owners: ");
        for(i=0;i<8;i++) {
            if(obj->ownerid[i]>0) {
                sprintf(buf2,"%s ",idname[obj->ownerid[i]].name);
                strcat(buf,CAP(buf2));
            }
        }
        if(strcmp(buf,"Owners: "))
        {
           strcat(buf,"\n\r");
           send_to_char(buf, ch);
        }

        sprintf(buf,"Weight: %d, Value: %d, Rent Cost: %d\n\r",
            GETOBJ_WEIGHT(obj), obj->obj_flags.cost, obj->obj_flags.cost_per_day);
        send_to_char(buf, ch);

        switch (GET_ITEM_TYPE(obj)) {
        case ITEM_RECIPE:
            if (obj->obj_flags.value[0] < 0)
            {
                sprintf(buf, "This recipe makes nothing!");
            }
            else if (obj->obj_flags.value[3] >= 0)
            {
                sprintf(buf, "This recipe is for %s\n\rIngredients:\n\r   %s\n\r   %s\n\r   %s\n\r",
                    obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing",
                    obj->obj_flags.value[1] >= 0 ? real_object(obj->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[1])].short_description : "something" : "nothing",
                    obj->obj_flags.value[2] >= 0 ? real_object(obj->obj_flags.value[2]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[2])].short_description : "something" : "nothing",
                    obj->obj_flags.value[3] >= 0 ? real_object(obj->obj_flags.value[3]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[3])].short_description : "something" : "nothing"
                 );
            }
            else if (obj->obj_flags.value[2] >= 0)
            {
                sprintf(buf, "This recipe is for %s\n\rIngredients:\n\r   %s\n\r   %s\n\r",
                    obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing",
                    obj->obj_flags.value[1] >= 0 ? real_object(obj->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[1])].short_description : "something" : "nothing",
                    obj->obj_flags.value[2] >= 0 ? real_object(obj->obj_flags.value[2]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[2])].short_description : "something" : "nothing"
                );
            }
            else if (obj->obj_flags.value[1] >= 0)
            {
                sprintf(buf, "This recipe is for %s\n\rIngredients:\n\r   %s\n\r",
                    obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing",
                    obj->obj_flags.value[1] >= 0 ? real_object(obj->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[1])].short_description : "something" : "nothing"
                );
            }
            else
            {
                sprintf(buf, "This recipe is for %s, but requires no ingredients\n\r",
                    obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing"
                );
            }

            send_to_char(buf, ch);
            break;

        case ITEM_AQ_ORDER:
            sprintf(buf, "This acquisition order is for the following items:\n\r   %s\n\r   %s\n\r   %s\n\r   %s\n\rIf completed, worth %d quest points.\n\r ", 
                obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing",
                obj->obj_flags.value[1] >= 0 ? real_object(obj->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[1])].short_description : "something" : "nothing",
                obj->obj_flags.value[2] >= 0 ? real_object(obj->obj_flags.value[2]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[2])].short_description : "something" : "nothing",
                obj->obj_flags.value[3] >= 0 ? real_object(obj->obj_flags.value[3]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[3])].short_description : "something" : "nothing", 
                OBJ_SPEC(obj));
            send_to_char(buf, ch);
            break;

        case ITEM_SCROLL:
        case ITEM_POTION:
            sprintf(buf, "Level %d spells of:\n\r",  obj->obj_flags.value[0]);
            send_to_char(buf, ch);

            if (obj->obj_flags.value[1] >= 1) {
                sprinttype(obj->obj_flags.value[1]-1,spells,buf);
                strcat(buf,"\n\r");
                send_to_char(buf, ch);
            }

            if (obj->obj_flags.value[2] >= 1) {
                sprinttype(obj->obj_flags.value[2]-1,spells,buf);
                strcat(buf,"\n\r");
                send_to_char(buf, ch);
            }

            if (obj->obj_flags.value[3] >= 1) {
                sprinttype(obj->obj_flags.value[3]-1,spells,buf);
                strcat(buf,"\n\r");
                send_to_char(buf, ch);
            }

            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf(buf, "Has %d charges, with %d charges left.\n\r",
                obj->obj_flags.value[1],
                obj->obj_flags.value[2]);
            send_to_char(buf, ch);

            sprintf(buf, "Level %d spell of:\n\r",  obj->obj_flags.value[0]);
            send_to_char(buf, ch);

            if (obj->obj_flags.value[3] >= 1) {
                sprinttype(obj->obj_flags.value[3]-1,spells,buf);
                strcat(buf,"\n\r");
                send_to_char(buf, ch);
            }
            break;

        case ITEM_WEAPON:
        case ITEM_2HWEAPON:
            if((obj->obj_flags.value[0] > -1) && (obj->obj_flags.value[0] <= 100))
              sprintf(buf, "Extra: '%s'\n\rDamage Dice is '%dD%d'\n\r",
                  wpn_spc[obj->obj_flags.value[0]],
                  obj->obj_flags.value[1],
                  obj->obj_flags.value[2]);
            else if((obj->obj_flags.value[0] > 300) && (obj->obj_flags.value[0] <= 311))
              sprintf(buf, "Extra: '%s Weapon'\n\rDamage Dice is '%dD%d'\n\r",
                  pc_class_types[obj->obj_flags.value[0]-300],
                  obj->obj_flags.value[1],
                  obj->obj_flags.value[2]);
      else
              sprintf(buf, "Extra: 'Unknown'\n\rDamage Dice is '%dD%d'\n\r",
                  obj->obj_flags.value[1],
                  obj->obj_flags.value[2]);
            send_to_char(buf, ch);
            break;

        case ITEM_TICKET :
            sprintf(buf, "This is ticket # %d for lot # %d.\n\r",
                obj->obj_flags.value[1],
                obj->obj_flags.value[0]);
            send_to_char(buf, ch);
            break;

        case ITEM_MISSILE:
            sprintf(buf, "Damage Dice is '%dD%d'\n\r",
                obj->obj_flags.value[0],
                obj->obj_flags.value[1]);
            send_to_char(buf, ch);
            break;

        case ITEM_ARMOR :
            sprintf(buf, "AC-apply: %d\n\r",
                obj->obj_flags.value[0]);
            send_to_char(buf, ch);
            break;

        case ITEM_BULLET :
            sprintf(buf, "For the gun with License Number: %d\n\r",
                obj->obj_flags.value[2]);
            send_to_char(buf, ch);
            break;

        case ITEM_FIREWEAPON:
            sprintf(buf, "License Number: %d\n\rNumber of bullets left : %d\n\rTodam : %dD%d\n\r",
                obj->obj_flags.value[0],
                obj->obj_flags.value[1],
                obj->obj_flags.value[2],
                obj->obj_flags.value[3]);
            send_to_char(buf, ch);
            break;

        case ITEM_LOCKPICK:
            sprintf(buf, "Has %d picks, with %d picks left.\n\r",
                obj->obj_flags.value[1],
                obj->obj_flags.value[0]);
            send_to_char(buf, ch);
            break;
        }

        if(IS_SET(obj->obj_flags.extra_flags2, ITEM_ALL_DECAY) || IS_SET(obj->obj_flags.extra_flags2, ITEM_EQ_DECAY)) {
            max_time=obj_proto_table[obj->item_number].obj_flags.timer;
            if(max_time)
                val3=9-10*obj->obj_flags.timer/max_time;
            else val3=9;
            if(val3<0) val3=0;
            val3=MIN(val3,9);
            printf_to_char(ch,"This item decays and seems %s.\n\r",decay_text[val3]);
        }

        found = FALSE;

        for (i=0;i<MAX_OBJ_AFFECT;i++) {
            if (obj->affected[i].location != APPLY_NONE) {

                if (!found) {
                    send_to_char("Influences you in the following:\n\r", ch);
                    found = TRUE;
                }

                sprinttype(obj->affected[i].location,apply_types,buf2);
                sprintf(buf,"    Affects : %s by %d\n\r", buf2,obj->affected[i].modifier);
                send_to_char(buf, ch);
            }
        }

       if ((obj->item_number >= 0) && 
	       obj_proto_table[obj->item_number].func)
	   {
         send_to_char("You detect that this item contains a hidden power.\n\r", ch);
       }

    } else if (!IS_NPC(victim)) { /* victim */

        sprintf(buf,"%d Years, %d Months, %d Days, %d Hours old.\n\r",
            age(victim).year, age(victim).month, age(victim).day,
            age(victim).hours);
        send_to_char(buf,ch);

        sprintf(buf,"Height %dcm  Weight %dpounds \n\r", GET_HEIGHT(victim),
            GET_WEIGHT(victim));
        send_to_char(buf,ch);

     } else { /* mob */

        wielded=EQ(victim,WIELD);

        if(wielded)
        {
            avgdmg=((wielded->obj_flags.value[1] + wielded->obj_flags.value[1] * wielded->obj_flags.value[2])/2) + GET_DAMROLL(victim);
        }
        else
        {
            avgdmg=((victim->specials.damnodice + victim->specials.damnodice * victim->specials.damsizedice)/2) + GET_DAMROLL(victim);
        }

        if(IS_SET(victim->specials.affected_by,AFF_FURY)) avgdmg*=2;
        while((mob_id_hp_int[iHP] <= GET_MAX_HIT(victim)) && (iHP < MOB_ID_HP_ARRAY_MAX)) iHP++;
        while((mob_id_dmg_int[iDMG] <= avgdmg) && (iDMG < MOB_ID_DMG_ARRAY_MAX)) iDMG++;
        while((mob_id_level_int[iLEVEL] <= GET_LEVEL(victim)) && (iLEVEL < MOB_ID_LEVEL_ARRAY_MAX)) iLEVEL++;
        while((mob_id_matt_int[iMATT] <= victim->specials.no_att) && (iMATT < MOB_ID_MATT_ARRAY_MAX)) iMATT++;
        if(mob_proto_table[victim->nr].func) iSPEC=1;

        sprintf(buf,"%s appears to be a %s %s; %s is %s%s%s and %s.\n\r",
            GET_SHORT(victim),
            mob_id_dmg_text[iDMG-1],
            npc_class_types[GET_CLASS(victim)],
            HSSH(victim),
            mob_id_matt_text[iMATT-1],
            mob_id_spec[iSPEC],
            mob_id_hp_text[iHP-1],
            mob_id_level_text[iLEVEL-1]);
        send_to_char(buf, ch);
    }
}

void spell_infravision(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (!affected_by_spell(victim, SPELL_INFRAVISION)) {

    if(CHAOSMODE && ch!=victim) { /* Chaos03 */
      send_to_char("You cannot cast this spell on another player.\n\r",ch);
      return;
    }

    af.type      = SPELL_INFRAVISION;
    af.duration  = 30 + GET_LEVEL(ch);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_INFRAVISION;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);
    act("$n's eyes start glowing a bright red.",FALSE,victim,0,0,TO_ROOM);
    send_to_char("Your eyes glow a bright red.\n\r", victim);
  }
}

void spell_animate_dead(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;
  int i, mob_level, r_num;
  CHAR *mob;
  OBJ *temp_obj;
  char nam[100];

  void add_follower(CHAR  *ch, CHAR *leader);

  if (!(GET_ITEM_TYPE(obj) == ITEM_CONTAINER) ||
      (obj->obj_flags.value[3] != 1)) {
    /* Object is not a corpse, or a container.      */
  } else {
    /* Addition of mob follow limit - Ranger April 98 */
    if(count_mob_followers(ch)>0) {
      send_to_char("You can't control anymore followers.\n\r",ch);
      return;
    }
    r_num = real_mobile(2);
    mob_level = MAX(1, obj->obj_flags.value[2]);
    mob = read_mobile(r_num, REAL);
    if(!mob) return;

    switch(number(1,12)) {
    case 1: {
      mob->player.short_descr = str_dup("The Vampire");
      mob->player.name = str_dup("vampire");
    } break;
    case 2: {
      mob->player.short_descr = str_dup("The Zombie");
      mob->player.name = str_dup("zombie");
    } break;
    case 3: {
      mob->player.short_descr = str_dup("The Skeleton");
      mob->player.name = str_dup("skeleton");
    } break;
    case 4: {
      mob->player.short_descr = str_dup("The Wraith");
      mob->player.name = str_dup("wraith");
    } break;
    case 5: {
      mob->player.short_descr = str_dup("The Phantom");
      mob->player.name = str_dup("phantom");
    } break;
    case 6: {
      mob->player.short_descr = str_dup("The Spirit");
      mob->player.name = str_dup("spirit");
    } break;
    case 7: {
      mob->player.short_descr = str_dup("The Ghost");
      mob->player.name = str_dup("ghost");
    } break;
    case 8: {
      mob->player.short_descr = str_dup("The Ghoul");
      mob->player.name = str_dup("ghoul");
    } break;
    case 9: {
      mob->player.short_descr = str_dup("The Undead");
      mob->player.name = str_dup("undead");
    } break;
    case 10: {
      mob->player.short_descr = str_dup("The Wight ");
      mob->player.name = str_dup("wight");
    } break;
    case 11: {
      mob->player.short_descr = str_dup("The Ghast");
      mob->player.name = str_dup("ghast");
    } break;
    default: {
      mob->player.short_descr = str_dup("The Spectre");
      mob->player.name = str_dup("spectre");
    } break;
    } /* switch */

    strcpy(nam, mob->player.short_descr);
    strcat(nam, " is floating here.\n\r");

    mob->player.long_descr = str_dup(nam);

    mob->specials.affected_by = AFF_ANIMATE;
    mob->specials.alignment = -1000;
    GET_LEVEL(mob) = mob_level/2;

    mob->points.hitroll = MAX( 1, mob_level/2);
    mob->points.armor = 0;

    mob->points.max_hit = mob_level*30;
    mob->points.hit = mob->points.max_hit;

    mob->points.damroll = number(1,5);
    mob->specials.damnodice = MAX(1, mob_level/2);
    mob->specials.damsizedice = 2;

    mob->points.gold = 0;
    GET_EXP(mob) = 0;

    mob->specials.position = POSITION_STANDING;
    mob->specials.default_pos = POSITION_STANDING;

    mob->player.sex = 0;
    mob->player.class = 0;

    for (i = 0; i < 5; i++)
      mob->specials.apply_saving_throw[i] = MAX(30-GET_LEVEL(mob), 2);

    char_to_room(mob, CHAR_REAL_ROOM(ch));
    act("$n animates the corpse.",FALSE,ch,0,0,TO_ROOM);
    act("You animate the corpse.",FALSE,ch,0,0,TO_CHAR);
    add_follower(mob,ch);

    af.type      = SPELL_CHARM_PERSON;
    if(CHAOSMODE)
      af.duration = 5;
    else
      af.duration  = GET_LEVEL(ch);
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    af.bitvector2 = 0;
    affect_to_char(mob, &af);

    while (obj->contains) {
      temp_obj=obj->contains;
      obj_from_obj(temp_obj);
      obj_to_char(temp_obj,mob);
    }
    extract_obj(obj);
  }
}

void spell_power_word_kill (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  int dam, fraction;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n",ch);
    return;
  }

  if (!IS_NPC(ch) && !IS_NPC(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
     send_to_char("You can't cast such a powerful spell on a player.\n",ch);
     return;
  }
/*
  act("$n gestures and a black stream of crackling energy surrounds $N.",
      FALSE,ch,0,vict,TO_NOTVICT);
  act("$n gestures and a black stream of crackling energy surrounds you.",
      FALSE,ch,0,vict,TO_VICT);
  act("You gesture and a black stream of crackling energy surrounds $N.",
      FALSE,ch,0,vict,TO_CHAR);
*/
  if (GET_LEVEL(vict) <= (GET_LEVEL(ch)/2)) {
    dam = GET_MAX_HIT(vict) + 21;
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(vict))) dam = MIN(dam,300);
    if(IS_AFFECTED(vict, AFF_SANCTUARY)) dam*=2;
    damage (ch, vict, dam, SPELL_PW_KILL,DAM_MAGICAL);
  }
  else if (GET_LEVEL(vict) < GET_LEVEL(ch)-10) {
    fraction = GET_LEVEL(ch) - GET_LEVEL(vict)-10;
    fraction = MIN(fraction,14);
    switch(fraction) {
      case 14:
      case 13:
        dam = (GET_MAX_HIT(vict) / 4);
        dam = MIN(dam,2000);
        break;
      case 12:
      case 11:
        dam = (GET_MAX_HIT(vict) / 8);
        dam = MIN(dam,2000);
        break;
      case 10:
      case 9:
        dam = (GET_MAX_HIT(vict) / 12);
        dam = MIN(dam,1600);
        break;
      case 8:
      case 7:
      case 6:
        dam = (GET_MAX_HIT(vict) / 16);
        dam = MIN(dam,1300);
        break;
      case 5:
      case 4:
      case 3:
      case 2:
        dam = (GET_MAX_HIT(vict) / 20);
        dam = MIN(dam,800);
        break;
      case 1:
      case 0:
        dam = (GET_MAX_HIT(vict) / 25);
        dam = MIN(dam,500);
        break;
      default:
        dam = 500;
        break;
    }
 /*   dam = (fraction * GET_MAX_HIT(vict) / 15); */ /* was going WAY too high on some.. lvl 50-32 */
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(vict))) dam = MIN(dam,300);
    if(IS_AFFECTED(vict, AFF_SANCTUARY)) dam*=2;
    damage (ch, vict, dam, SPELL_PW_KILL,DAM_MAGICAL);
  }
  else {
    damage (ch, vict, 0, SPELL_PW_KILL,DAM_MAGICAL);
  }
}

void spell_vampiric_touch (ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  int dam = 0;
  int heal = 0;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n", ch);
    return;
  }

  if (!(IS_EVIL(ch)))
  {
    send_to_char("Your saintly predilections for doing good forbid the use of this spell!\n", ch);
    return;
  }

  if (!ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) &&
      !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA) &&
      (IS_AFFECTED(victim, AFF_CHARM) || (IS_MORTAL(victim) && IS_SET(ch->specials.pflag, PLR_NOKILL))))
  {
    act("You cannot cast this spell on $N.", FALSE, ch, NULL, victim, TO_CHAR);
    return;
  }

  dam = MIN(GET_LEVEL(ch), 50);

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
  {
    dam *= 5;
  }
  else
  {
    dam *= 9;
  }

  if (IS_AFFECTED(victim, AFF_SPHERE) &&
      !breakthrough(ch, victim, BT_SPHERE))
  {
    dam = 0;
  }

  heal = MAX(0, MIN(GET_HIT(victim), IS_AFFECTED(victim, AFF_SANCTUARY) ? (dam / 2) : dam));

  if (heal > 0)
  {
    //GET_HIT(ch) += heal;
    magic_heal(ch, SPELL_VAMPIRIC, heal, TRUE);
    send_to_char("You feel the drained energy flowing into you.\n\r", ch);
  }
  else
  {
    send_to_char("No energy flows into you!\n\r", ch);
  }

  update_pos(ch);

  damage(ch, victim, dam, SPELL_VAMPIRIC, DAM_NO_BLOCK);
}

void spell_conflagration (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  CHAR *tmp, *next;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n",ch);
    return;
  }

  act("$n conjures a handful of flames from thin air\nand throws them at $N.",
      FALSE,ch,0,vict,TO_NOTVICT);
  act("$n conjures a handful of flames from thin air and throws them at you.",
      FALSE,ch,0,vict,TO_VICT);
  act("You conjure a handful of flames from thin air and throw them at $N.",
      FALSE,ch,0,vict,TO_CHAR);

  for (tmp = world[CHAR_REAL_ROOM(ch)].people; tmp; tmp = next) {
    next = tmp->next_in_room;
    if(IS_NPC(tmp) && IS_SET(tmp->specials.immune,IMMUNE_FIRE)) continue;
    if (tmp != vict && tmp != ch) {
      act ("The fires raging around $n burn you as well.",
     FALSE,vict,0,tmp,TO_VICT);
/* Changed following line so conflag would damage other Pcs - Ranger Nov 96
      damage (ch, tmp, 100, SPELL_CONFLAGRATION);
   Changed again so that pc's would get xp killing mobs in the room with
   this loop - Ranger May 99 */
      if(!IS_NPC(tmp)) {
        damage (tmp, tmp, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);
      }
      else
        damage (ch, tmp, 100, TYPE_UNDEFINED,DAM_FIRE);
    }
  }

  if(!IS_SET(vict->specials.immune,IMMUNE_FIRE)){
    send_to_char("You are burning to the soul.\n\r",vict);
    if(IS_NPC(vict)) damage (ch, vict, 1500, TYPE_UNDEFINED,DAM_MAGICAL);
    else damage (ch, vict, 400, TYPE_UNDEFINED,DAM_FIRE);
  }

  send_to_char("The raging fires burn you as well.\n\r",ch);
  damage (ch, ch, 300, TYPE_UNDEFINED,DAM_NO_BLOCK);
}

void spell_mass_invisibility (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  CHAR *tmp, *next;
  char max;

  max = lvl>>1;

  for (tmp = world[CHAR_REAL_ROOM(ch)].people; tmp && max > 1; tmp = next) {
    next = tmp->next_in_room;
    if (IS_MORTAL(tmp) && tmp != ch) {
      spell_invisibility (lvl, ch, tmp, 0);
      max--;
    }
  }
  spell_invisibility (lvl, ch, ch, 0);
}

void spell_convergence (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  int needed, heal;

  needed = MAX(0,GET_MAX_HIT(vict) - GET_HIT(vict));
  heal = MAX(0,MIN(GET_MANA(vict),needed));

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(vict))) {
    heal = MIN(heal,250);
  }

  if(ch!=vict) {
    act("$n draws power from $N to heal $M.",FALSE,ch,0,vict,TO_NOTVICT);
    act("$n draws power from you to heal your wounds.",FALSE,ch,0,vict,TO_VICT);
    act("You draw power from $N to heal $M.",FALSE,ch,0,vict,TO_CHAR);
  }
  else {
    act("$n draws power from $mself to heal $m.",FALSE,ch,0,vict,TO_NOTVICT);
    act("You draw power from yourself to heal your wounds.",FALSE,ch,0,vict,TO_CHAR);
    heal = MAX(0,MIN(GET_MANA(vict)-20,needed));
  }

  GET_HIT(vict) = GET_HIT(vict) + heal;
  GET_MANA(vict) = GET_MANA(vict) - heal;
  send_to_char ("You feel better.\n",vict);
}

void spell_enchant_armour (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  int i, ench=0;

  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC) ||
  IS_SET(obj->obj_flags.extra_flags, ITEM_DARK)) {
      send_to_char ("Nothing happens.\n\r",ch);
      return;
    }

    for (i = 0 ; i < OFILE_MAX_OBJ_AFFECT ; i++)
      if (obj->affected[i].location == APPLY_NONE)
  break;

    if (i >= OFILE_MAX_OBJ_AFFECT) return;

    if (lvl == LEVEL_IMP)
      ench = 5;
    else if(lvl>=40)
      ench = 4;
    else if (lvl >= 30)
      ench = 3;            /* Assumes you get this spell at least on */
    else if (lvl >=26)   /* 21st lvl. */
      ench = 2;
    else if (lvl >= 21)
      ench = 1;

    obj->affected[i].location = APPLY_AC;
    obj->affected[i].modifier = -(ench + 1);
    SET_BIT (obj->obj_flags.extra_flags, ITEM_MAGIC);
    send_to_char ("It glows blue...\n\r", ch);

    if (IS_EVIL(ch) || IS_NEUTRAL(ch)) {
      SET_BIT (obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
      send_to_char ("It glows red...\n\r", ch);
    }

    if (IS_GOOD(ch) || IS_NEUTRAL(ch)) {
      SET_BIT (obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
      send_to_char ("It glows yellow...\n\r", ch);
    }

    if (IS_EVIL(ch) || IS_GOOD(ch))
      SET_BIT (obj->obj_flags.extra_flags, ITEM_ANTI_NEUTRAL);
  }
}

void spell_dispel_magic (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  struct affected_type_5 af;
  int i,mod;
  long allowed_bits =
    AFF_BLIND | AFF_INVISIBLE | AFF_DETECT_MAGIC | AFF_DETECT_ALIGNMENT |
      AFF_DETECT_INVISIBLE | AFF_DETECT_MAGIC | AFF_SENSE_LIFE |
  AFF_HOLD | AFF_SANCTUARY | AFF_CURSE | AFF_PROTECT_EVIL |
          AFF_PROTECT_GOOD | AFF_PARALYSIS | AFF_SLEEP | AFF_CHARM | AFF_IMINV;
  int bit, index;

  mod=-GET_HIT(ch)+1;
  mod=MAX(mod,-50);
  af.type = SPELL_CURSE;
  af.location = APPLY_HIT;
  af.modifier = mod;
  af.duration = 3;
  af.bitvector = 0;
  af.bitvector2 = 0;
  affect_to_char (ch, &af);
  send_to_char ("You feel exhausted beyond your powers.\n\r",ch);

  if (obj) {
    act("$n points at $p and it glows green.",FALSE,ch,obj,0,TO_ROOM);
    act("You point at $p and it glows green.",FALSE,ch,obj,0,TO_CHAR);

    if (!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
      send_to_char ("There's no magic to dispel!.\n\r",ch);
      return;
    }

    if (number(0,1)) {
      send_to_char ("Nothing happens.\n\r",ch);
      return;
    }

    switch (GET_ITEM_TYPE(obj)) {

    case ITEM_POTION:
    case ITEM_SCROLL:
      for (i=1;i<4;i++)
  if (obj->obj_flags.value[i] != -1)
    obj->obj_flags.value[i] = -1;
      return;

    case ITEM_WAND:
    case ITEM_STAFF:
      obj->obj_flags.value[2] = 0;
      return;

    default:
      SET_BIT (obj->obj_flags.extra_flags, ITEM_DARK);
      for (i=0; i < MAX_OBJ_AFFECT; i++)
  if (obj->affected[i].location != APPLY_NONE) {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }
    }
  } else {
    if (saves_spell (vict, SAVING_SPELL,lvl))
      return;

    for (bit = 1,index = number (0, 30);index>0;bit=bit<< 1, index--);

    if (bit & allowed_bits) {
      REMOVE_BIT (vict->specials.affected_by, bit);
      affect_total (vict);
    }
  }
}

/* Confusion makes spellcasting extremely difficult, fighting difficult
   and even walking to a certain direction hard. Remember to add something
   that cures you. Adding it to spell_miracle would be suitable. */

void spell_confusion (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  struct affected_type_5 af;

  if (!affected_by_spell(vict, SPELL_CONFUSION)) {
    af.type = SPELL_CONFUSION;
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(vict)))
      af.duration = 4;
    else
      af.duration = 12;
    af.location = APPLY_WIS;
    af.modifier = -10;
    af.bitvector = AFF_CONFUSION;
    af.bitvector2 = 0;
    affect_to_char (vict, &af);

    af.location = APPLY_INT;
    af.modifier = -10;
    affect_to_char (vict, &af);

    act("$n points $s finger at $N and a blue ray hits $M.",
  FALSE,ch,0,vict,TO_NOTVICT);
    act("$n points $s finger at you and a blue ray hits you.",
  FALSE,ch,0,vict,TO_VICT);
    act("You point your finger at $N and a blue ray hits $M.",
  FALSE,ch,0,vict,TO_CHAR);
    send_to_char ("You can't seem to think straight anymore...\n\r",vict);
    return;
  }
  send_to_char ("Nothing happens.\n\r",ch);
}

#define WALL_THORNS 34
#define STATUE_PEACE 9
#define ICE_WALL 27750
#define ICE_BLOCK 27751

void spell_disintegrate (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  int v_lvl,c_lvl,ind;
  OBJ *tmp, *next, *tmp_c = NULL, *next_c = NULL;
  char buf[MAX_INPUT_LENGTH];
  int dam = 0;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (obj) {
    if(number(0,3) || GET_ITEM_TYPE(obj)==ITEM_CONTAINER ||
       V_OBJ(obj)==WALL_THORNS || V_OBJ(obj)==STATUE_PEACE ||
       V_OBJ(obj)==ICE_WALL || V_OBJ(obj)==ICE_BLOCK ||
       GET_ITEM_TYPE(obj)==ITEM_AQ_ORDER) {
      send_to_char ("Nothing happens.\n\r",ch);
      return;
    }

    act("$n looks at $p, and it crumbles to dust.",FALSE,ch,obj,0,TO_ROOM);
    act("You look at $p, and it crumbles to dust.",FALSE,ch,obj,0,TO_CHAR);
    sprintf(buf,"%s just destroyed %s at %s.",GET_NAME(ch),
            OBJ_SHORT(obj), world[CHAR_REAL_ROOM(ch)].name);
    extract_obj(obj);
    wizlog(buf, LEVEL_WIZ,6);
    log_s(buf);
    return;
  }

  if (!IS_NPC(ch) && !IS_NPC(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(vict))) {
    send_to_char("You can't cast such a powerful spell on a player.\n\r",ch);
    return;
  }

  if(IS_NPC(vict) && IS_SET(vict->specials.immune,IMMUNE_DISINTEGRATE)) {
    send_to_char("Nothing happens??\n\r",ch);
    return;
  }

  v_lvl = GET_LEVEL(vict);
  c_lvl=MIN(lvl,30);
  if (!ROOM_CHAOTIC(CHAR_REAL_ROOM(vict)) &&
      (c_lvl*c_lvl*number(1,10) > v_lvl*v_lvl*number(1,15))) {
    for (ind = 0;ind < MAX_WEAR;ind++)
      if (EQ(vict,ind)) extract_obj(unequip_char (vict, ind));
    for (tmp = vict->carrying;tmp;tmp = next) {
      next = tmp->next_content;
      // updated to unpack containers which might contain an AQ_ORDER
      //   and moves AQ_ORDERs to room - no free way to "quit" them
      if(GET_ITEM_TYPE(tmp)==ITEM_AQ_ORDER) {
        obj_from_char(tmp);
        obj_to_room(tmp, CHAR_REAL_ROOM(vict));
      } else if (GET_ITEM_TYPE(tmp)==ITEM_CONTAINER) {
        for (tmp_c = tmp->contains; tmp_c; tmp_c = next_c) {
          next_c = tmp_c->next_content;
          obj_from_obj(tmp_c);
          obj_to_char(tmp_c, vict);
        }
        extract_obj(tmp);
        next = vict->carrying;
      } else {
        extract_obj(tmp);
      }
    }
    act("$n looks at $N, and $E crumbles to dust.",FALSE,ch,0,vict,TO_ROOM);
    act("You look at $N, and $E crumbles to dust.",FALSE,ch,0,vict,TO_CHAR);
    sprintf(buf,"%s disintegrates %s.",GET_NAME(ch), GET_SHORT(vict));
    wizlog(buf, LEVEL_WIZ,6);
    log_s(buf);

    if(vict->questowner) {
      vict->questowner->questgiver=0;
      vict->questowner->questmob=0;
      vict->questowner->quest_status=QUEST_FAILED;
      vict->questowner->quest_level=0;
      vict->questowner->ver3.time_to_quest=25;
      printf_to_char(vict->questowner,"Your victim has been disintegrated, you have failed your quest! You can start another in %d ticks.\n\r",vict->questowner->ver3.time_to_quest);
      vict->questowner=0;
    }

    extract_char (vict);

  } else {
    act("$n looks at $N, and $E screams in pain.",FALSE,ch,0,vict,TO_ROOM);
    act("You look at $N, and $E screams in pain.",FALSE,ch,0,vict,TO_CHAR);
    dam = c_lvl*number(10,20);
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(vict))) dam = MIN(dam,400);
    damage (ch,vict,dam,SPELL_DISINTEGRATE,DAM_MAGICAL);
  }
}

void spell_searing_orb (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  OBJ *paper, *anot;
  CHAR *tmp, *next;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char ("Please behave here!\n\r",ch);
    return;
  }

  act("$n throws a flaming orb in the air, and the orb explodes...",
      FALSE,ch,0,0,TO_ROOM);
  send_to_char("You throw a flaming orb in the air, and the orb explodes...",
         ch);

  for (paper = world[CHAR_REAL_ROOM(ch)].contents;paper;paper = anot) {
    anot = paper->next_content;
    if ((GET_ITEM_TYPE(paper) == ITEM_SCROLL || GET_ITEM_TYPE(paper) == ITEM_RECIPE) && number(0,5)) {
      act("$p burns in bright and hot flames...",FALSE,ch,paper,0,TO_ROOM);
      act("$p burns in bright and hot flames...",FALSE,ch,paper,0,TO_CHAR);
      extract_obj(paper);
    }
    if (GET_ITEM_TYPE(paper) == ITEM_POTION && number(0,5)) {
      act("$p boils up in steam...",FALSE,ch,paper,0,TO_ROOM);
      act("$p boils up in steam...",FALSE,ch,paper,0,TO_CHAR);
      extract_obj(paper);
    }
  }

    for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp = next) {
        next = tmp->next_in_room;
        for (paper = tmp->carrying;paper;paper = anot) {
            anot = paper->next_content;
            if ((GET_ITEM_TYPE(paper) == ITEM_SCROLL || GET_ITEM_TYPE(paper) == ITEM_RECIPE) && !number(0,5)) {
                act("$p burns in bright and hot flames...",FALSE,ch,paper,0,TO_ROOM);
                act("$p burns in bright and hot flames...",FALSE,ch,paper,0,TO_CHAR);
                extract_obj(paper);
            }

            if (GET_ITEM_TYPE(paper) == ITEM_POTION && !number(0,5)) {
                act("$p boils up in steam...",FALSE,ch,paper,0,TO_ROOM);
                act("$p boils up in steam...",FALSE,ch,paper,0,TO_CHAR);
                extract_obj(paper);
            }
        }

        if (tmp != ch) {
            damage (ch, tmp, number(100,150), SPELL_SEARING_ORB,DAM_FIRE);
        }
    }
}

void look_in_room(CHAR *ch, int vnum);

void spell_clairvoyance (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
/*  int inroom; */
  char buf[MIL];
  void do_look(CHAR *, char *, int);

  act("$n closes $s eyes and concentrates...",FALSE,ch,0,0,TO_ROOM);
  act("You close your eyes and concentrate...",FALSE,ch,0,0,TO_CHAR);

  if(!vict) {
    send_to_char("Your attempt has failed!\n\r",ch);
    return;
  }

  if(IS_SET(ch->specials.pflag, PLR_QUEST)) {
    sprintf(buf,"QSTINFO: %s casts 'clairvoyance' %s",GET_NAME(ch),GET_NAME(vict));
    wizlog(buf,LEVEL_IMM,7);
  }

  if (!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) {
    send_to_char("Your divination attempt on the immortal fails!\n\r",ch);
    return;
  }

  if(IS_NPC(vict) && IS_SET(vict->specials.immune,IMMUNE_CLAIR)) {
    send_to_char("Your attempt has failed!\n\r",ch);
    return;
  }

  if (GET_LEVEL(ch)<LEVEL_IMM && IS_AFFECTED(vict, AFF_IMINV) && CHAOSMODE) {
    send_to_char("Your attempt has failed!\n\r", ch);
    return;
  }

  send_to_char ("You see the world through a hazy mist...\n\r",ch);
/*  send_to_char ("You feel someone else looking through your eyes!\n\r",vict);*/
  look_in_room(ch,world[CHAR_REAL_ROOM(vict)].number);
}

/*
void spell_shapeshift (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj){
  struct enchantment_type_5 ench;
  int eq_pos;
  int claws = 3005;
  OBJ *tmp_obj;

    for (eq_pos=0 ; eq_pos<18 ; eq_pos++)
      if (EQ(ch,eq_pos))
  obj_to_char(unequip_char(ch,eq_pos), ch);
  tmp_obj=read_object(claws,(int)VIRTUAL);
        equip_char(ch, tmp_obj, WIELD);

  act("$n screams in pain as $e transforms into a werewolf.",TRUE,vict,0,0,TO_ROOM);
  act("You scream in pain as you transform into a werewolf.",TRUE,vict,0,0,TO_CHAR);

  memset(&ench,0,sizeof(struct enchantment_type_5));
  ench.name     = str_dup("Wolf lycanthropy");
  enchantment_to_char(vict,&ench,TRUE);
  free(ench.name);
  }


void spell_silence (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj){
  struct enchantment_type_5 ench;

 act("$n tries to scream but no sound escapes $s lips.",TRUE,vict,0,0,TO_ROOM);
 act("You suddenly find yourself unable to make any sound.",TRUE,vict,0,0,TO_CHAR);

  memset(&ench,0,sizeof(struct enchantment_type_5));
  ench.name     = str_dup("Magical silence");
  enchantment_to_char(vict,&ench,TRUE);
  free(ench.name);
 }

void spell_quester (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj){
  struct enchantment_type_5 ench;

 act("$n starts glowing with a faint blue light.",TRUE,vict,0,0,TO_ROOM);
 act("You start glowing with a faint blue light.",TRUE,vict,0,0,TO_CHAR);

  memset(&ench,0,sizeof(struct enchantment_type_5));
  ench.name     = str_dup("Champion Quester");
  enchantment_to_char(vict,&ench,TRUE);
  free(ench.name);
 }
*/

void spell_firebreath (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj){
  struct enchantment_type_5 ench;

  if(CHAOSMODE && ch!=vict) { /* Chaos03 */
    send_to_char("You cannot cast this spell on another player.\n\r",ch);
    return;
  }

  if(ch!=vict) {
    act("$N's eyes burn red and smoke rises from the ground.",FALSE,ch,0,vict,TO_NOTVICT);
    act("Your eyes burn red, and smoke rises from the ground.",FALSE,ch,0,vict,TO_VICT);
    act("$S eyes burn red, and smoke rises from the ground.",FALSE,ch,0,vict,TO_CHAR);
  }
  else {
    act("$N's eyes burn red and smoke rises from the ground.",FALSE,ch,0,vict,TO_NOTVICT);
    act("Your eyes burn red, and smoke rises from the ground.",FALSE,ch,0,vict,TO_VICT);
  }

  memset(&ench,0,sizeof(struct enchantment_type_5));
  ench.name     = str_dup("Fire Breath");
  enchantment_to_char(vict,&ench,TRUE);
  free(ench.name);
}


void spell_regeneration (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj)
{
  ENCH ench;

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch != vict)
  {
    send_to_char("You cannot cast this spell on another player.\n\r",ch);

    return;
  }

  if (ch != vict)
  {
    act("$N's skin turns green, and $E seems to bend toward the sun.",FALSE,ch,0,vict,TO_NOTVICT);
    act("Your skin turns green, and you feel an affinity for the shining sun.",FALSE,ch,0,vict,TO_VICT);
    act("$S skin turns green, and $E seems to bend toward the sun.",FALSE,ch,0,vict,TO_CHAR);
  }
  else
  {
    act("$N's skin turns green, and $E seems to bend toward the sun.",FALSE,ch,0,vict,TO_NOTVICT);
    act("Your skin turns green, and you feel an affinity for the shining sun.",FALSE,0,0,vict,TO_VICT);
  }

  memset(&ench, 0, sizeof(AFF));
  ench.name     = str_dup("Regeneration");
  enchantment_to_char(vict,&ench,TRUE);
  free(ench.name);
}


void spell_dispel_sanct(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (!affected_by_spell(victim, SPELL_SANCTUARY))
  {
    send_to_char("Nothing happens.\n\r",ch);

    return;
  }

  affect_from_char(victim, SPELL_SANCTUARY);
  REMOVE_BIT(victim->specials.affected_by, AFF_SANCTUARY);

  send_to_char("The white aura around your body fades.\n\r", victim);
  act("The white aura around $n's body fades.", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_disenchant(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 *aff,*tmp;

  if (victim->affected)
    {
    for(aff = victim->affected; aff; aff = tmp) {
      tmp=aff->next;
      if(aff) {
        if(aff->type==SKILL_HOSTILE || aff->type==SKILL_DEFEND ||
           aff->type==SKILL_BERSERK || aff->type==SKILL_FADE ||
           aff->type==SKILL_EVASION || /* Used to be Cover */
           aff->type==SKILL_FRENZY || (aff->type==SPELL_BLESS && aff->duration==-1))
          continue;
        affect_from_char(victim, aff->type);
      }
      else break;
    }
    send_to_char("Done.\n\r", ch);
    act("You have been disenchanted by $N!", FALSE, victim, 0, ch, TO_CHAR);
    }
    else {
    send_to_char("That person isn't affected by any spell.\n\r",ch);
 }
}


void spell_petrify(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  char buffer[MAX_INPUT_LENGTH];
  struct descriptor_data *d;
  long ct;
  char *tmstr;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if(ch->specials.fighting && ch->specials.fighting == victim)
     stop_fighting(ch);

  if (victim->specials.fighting) stop_fighting(victim);
  if(signal_char(victim,ch,MSG_STONE,"")) return;
  if(!IS_NPC(victim) && GET_LEVEL(victim)>GET_LEVEL(ch) &&
     IS_SET(victim->new.imm_flags, WIZ_ACTIVE)) {
    act("$n gazes at $N and $e turns to stone.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n gazes at you and $e turn to stone!", FALSE, ch, 0, victim, TO_VICT);
    act("You gaze at $N and you turn to stone!", FALSE, ch, 0, victim, TO_CHAR);
    sprintf(buffer, "%s stoned by %s at %s (%d)", GET_NAME(ch), GET_NAME(victim),
          world[CHAR_REAL_ROOM(ch)].name,world[CHAR_REAL_ROOM(ch)].number);
    wizlog(buffer,LEVEL_IMM,4);
    log_s(buffer);
    deathlog(buffer);
    death_cry(ch);

    if(ch->quest_status==QUEST_RUNNING || ch->quest_status==QUEST_COMPLETED)
      ch->ver3.time_to_quest=30;
    ch->questgiver=0;
    if(ch->questobj) ch->questobj->owned_by=0;
    ch->questobj=0;
    if(ch->questmob) ch->questmob->questowner=0;
    ch->questmob=0;
    ch->quest_status=QUEST_NONE;
    ch->quest_level=0;

    make_statue(ch);
    save_char(ch,NOWHERE);
    extract_char(ch);
    return;
  }
  act("$n gazes at $N and $E turns to stone.", FALSE, ch, 0, victim, TO_NOTVICT);
  act("$n gazes at you and you turn to stone!", FALSE, ch, 0, victim, TO_VICT);
  act("You gaze at $N and $E turns to stone!", FALSE, ch, 0, victim, TO_CHAR);


  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) &&
     !IS_NPC(victim) && GET_LEVEL(victim)<LEVEL_IMM) {
    sprintf(buffer, "%s stoned by %s at %s", GET_NAME(victim), GET_NAME(ch),
            world[CHAR_REAL_ROOM(ch)].name);
    for (d=descriptor_list;d;d=d->next)
      if (!d->connected) act(buffer,0,d->character,0,0,TO_CHAR);

    number_of_kills++;
    if(number_of_kills<100) {
      sprintf(scores[number_of_kills].killer,"%s",(IS_NPC(ch) ? MOB_SHORT(ch) : GET_NAME(ch)));
      sprintf(scores[number_of_kills].killed,"%s",GET_NAME(victim));
      sprintf(scores[number_of_kills].location,"%s",world[CHAR_REAL_ROOM(victim)].name);
      ct = time(0);
      tmstr = asctime(localtime(&ct));
      *(tmstr+strlen(tmstr)-1)='\0';
      sprintf(scores[number_of_kills].time_txt,"%s",tmstr);
      if(number_of_kills==CHAOSDEATH) {
        sprintf(buffer,
          "**** Kill number %d has been reached, we have a winner!!! ****\n\r\n\r",
          CHAOSDEATH);
        send_to_all(buffer);
        send_to_all(buffer); /* yes, twice */
      }
    }
    else {
      number_of_kills=99;
    }
  }
  else {
    sprintf(buffer, "%s stoned by %s at %s (%d)", GET_NAME(victim), GET_NAME(ch),
            world[CHAR_REAL_ROOM(ch)].name,world[CHAR_REAL_ROOM(ch)].number);
    wizlog(buffer, LEVEL_IMM, 3);
  }
  log_s(buffer);
  deathlog(buffer);

  death_cry(victim);
  make_statue(victim);
  save_char(victim,NOWHERE);
  extract_char(victim);
  return;
}


void spell_haste(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(victim, SPELL_HASTE))
  {
    af.type       = SPELL_HASTE;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 2;
    else
      af.duration = 5;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = AFF_DUAL;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);

    send_to_char("Suddenly everything around you seems to slow down to a crawl.\n\r", victim);
    act("$n starts moving with blinding speed.", FALSE, victim, 0, 0, TO_ROOM);
  }
  else
  {
    send_to_char("You're already moving as fast as you can.\n\r", victim);
  }
}


void spell_great_mana(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj)
{
  CHAR *tmp_victim = NULL;
  CHAR *temp = NULL;

  for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp)
  {
    temp = tmp_victim->next_in_room;

    spell_recover_mana(level, ch, tmp_victim, 0);
  }
}

/*
void spell_spiritwrack(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim, *temp;

  assert(ch);
  assert(level <= MAX_LEVEL);

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  act("$n's soft words sound like booming thunder in your ears!",
      TRUE, ch, 0, 0, TO_ROOM);
  act("You utter a word of power!",FALSE,ch,0,0,TO_CHAR);

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people;tmp_victim;tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if ((ch != tmp_victim) || (GET_LEVEL(tmp_victim)<LEVEL_IMM))  {
      damage(ch, tmp_victim, number(20, 40), SPELL_SPIRITWRACK,DAM_MAGICAL);
      act("$n is stunned!",TRUE,tmp_victim,0,0,TO_ROOM);
      GET_POS(tmp_victim) = POSITION_STUNNED;
   }
  }
}

void spell_spectral_blade(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  OBJ *tmp_obj;

  assert(ch);
  assert((level <= MAX_LEVEL));

  CREATE(tmp_obj, OBJ, 1);
  clear_object(tmp_obj);

  tmp_obj->name = str_dup("spectral ghostly blade");
  tmp_obj->short_description = str_dup("a shimmering ghostly blade");
  tmp_obj->description =
    str_dup("A shimmering, transparent ghostly blade lies here.");
  tmp_obj->obj_flags.type_flag = ITEM_WEAPON;
  tmp_obj->obj_flags.timer = 24;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 1;
  tmp_obj->obj_flags.cost_per_day = 1;
  tmp_obj->obj_flags.extra_flags = 0;
  tmp_obj->obj_flags.value[0] = 0;
  tmp_obj->obj_flags.value[1] = 2;
  tmp_obj->obj_flags.value[2] = 8;
  tmp_obj->obj_flags.value[3] = 3;
  tmp_obj->obj_flags.cost_per_day = 1;
  tmp_obj->affected[0].location = APPLY_HITROLL;
  tmp_obj->affected[0].modifier = 3;
  tmp_obj->affected[1].location = APPLY_DAMROLL;
  tmp_obj->affected[1].modifier = 12;
  SET_BIT(tmp_obj->obj_flags.extra_flags, ITEM_MAGIC);
  SET_BIT(tmp_obj->obj_flags.extra_flags, ITEM_ANTI_RENT);
  tmp_obj->next = object_list;
  object_list = tmp_obj;

  tmp_obj->item_number = -1;

  act("$n gestures and $p appears in $s hands.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("You gesture and $p appears in your hands.",TRUE,ch,tmp_obj,0,TO_CHAR);

if (EQ(ch,16)) {
   obj_to_char (unequip_char (ch, 16),ch);
  }
 equip_char(ch, tmp_obj, 16);
}


void spell_doppelganger(ubyte level, CHAR *ch, CHAR *vict, OBJ *obj) {
  char buf[MAX_STRING_LENGTH];
  CHAR *image;
  int i;

  image = read_mobile(ITEM(14000,12),VIRTUAL);
  char_to_room(image,CHAR_REAL_ROOM(vict));

  GET_LEVEL(image)                = GET_LEVEL(vict);
  image->points.max_mana          = GET_MAX_MANA(vict);
  image->points.max_move          = GET_MAX_MOVE(vict);
  image->points.max_hit           = GET_MAX_HIT(vict);
  image->player.description       = MOB_DESCRIPTION(vict);

if(vict->specials.no_att>0) {
    image->specials.no_att         = vict->specials.no_att;
   for (i=0;i<vict->specials.no_att;i++) {
   if(i==MAX_ATTS) break;
    image->specials.att_type[i]    = vict->specials.att_type[i];
    image->specials.att_target[i]  = vict->specials.att_target[i];
    image->specials.att_percent[i] = vict->specials.att_percent[i];
    image->specials.att_spell[i]   = vict->specials.att_spell[i];
        }
}


  GET_HIT(image)                  = GET_MAX_HIT(vict);
  GET_MANA(image)                 = GET_MAX_MANA(vict);
  GET_MOVE(image)                 = GET_MAX_MOVE(vict);
  GET_EXP(image)                  = GET_LEVEL(image)*GET_LEVEL(image)*300;
  GET_SEX(image)                  = GET_SEX(vict);

  image->specials.alignment       = vict->specials.alignment;
  image->points.hitroll           = vict->points.hitroll;
  image->points.damroll           = vict->points.damroll;
  image->points.armor             = vict->points.armor;
  image->specials.damnodice         = vict->specials.damnodice;
  image->specials.damsizedice       = vict->specials.damsizedice;
  for (i = 0; i < 5; i++)
    image->specials.apply_saving_throw[i] = MAX(30-GET_LEVEL(image), 2);

  if(image->player.name)
    free (image->player.name);
  sprintf (buf,"%s doppelganger",CAP(GET_NAME(vict)));
  image->player.name = str_dup(buf);

  if(image->player.short_descr)
    free (image->player.short_descr);
  sprintf (buf,"the doppelganger of %s",CAP(GET_SHORT(vict)));
  image->player.short_descr = str_dup(buf);

  if(image->player.long_descr)
    free (image->player.long_descr);
  sprintf (buf,"The doppelganger of %s",GET_LONG(vict));
  image->player.long_descr = str_dup(buf);

 if (!IS_NPC(vict)) {

  if(image->player.short_descr)
    free (image->player.short_descr);
  sprintf (buf,"the doppelganger of %s",CAP(GET_NAME(vict)));
  image->player.short_descr = str_dup(buf);

  if(image->player.long_descr)
    free (image->player.long_descr);
  sprintf (buf,"The doppelganger of %s",CAP(GET_NAME(vict)));
  strcat(buf," is standing here.\n\r");
  image->player.long_descr = str_dup(buf);
  image->specials.damnodice         = 9;
  image->specials.damsizedice       = 4;
 }

  image->specials.affected_by = vict->specials.affected_by;

  if (GET_CLASS(vict) == CLASS_WARRIOR ||
      GET_CLASS(vict) == CLASS_THIEF ||
      GET_CLASS(vict) == CLASS_NOMAD ||
      GET_CLASS(vict) == CLASS_BARD ||
      GET_CLASS(vict) == CLASS_ANTI_PALADIN ||
      GET_CLASS(vict) == CLASS_PALADIN ||
      GET_CLASS(vict) == CLASS_NINJA ||
      GET_CLASS(vict) == CLASS_AVATAR) {
    SET_BIT(image->specials.affected_by,AFF_DODGE);
  }

  if (GET_CLASS(vict) == CLASS_WARRIOR ||
      GET_CLASS(vict) == CLASS_NINJA ||
      GET_CLASS(vict) == CLASS_THIEF ||
      GET_CLASS(vict) == CLASS_ANTI_PALADIN ||
      GET_CLASS(vict) == CLASS_AVATAR) {
    SET_BIT(image->specials.affected_by,AFF_DUAL);
  }
  if (IS_SET(vict->specials.affected_by,AFF_SANCTUARY))
       SET_BIT(image->specials.affected_by,AFF_SANCTUARY);
  if (IS_SET(vict->specials.affected_by,AFF_INVUL))
       SET_BIT(image->specials.affected_by,AFF_INVUL);
  if (IS_SET(vict->specials.affected_by,AFF_PROTECT_EVIL))
       SET_BIT(image->specials.affected_by,AFF_PROTECT_EVIL);
if (IS_SET(vict->specials.affected_by,AFF_PROTECT_GOOD))
       SET_BIT(image->specials.affected_by,AFF_PROTECT_GOOD);
  if (IS_SET(vict->specials.affected_by,AFF_IMINV))
       SET_BIT(image->specials.affected_by,AFF_IMINV);
  if (IS_SET(vict->specials.affected_by,AFF_FURY))
       SET_BIT(image->specials.affected_by,AFF_FURY);

  REMOVE_BIT(image->specials.affected_by,AFF_BLIND);
  REMOVE_BIT(image->specials.affected_by,AFF_HOLD);
  REMOVE_BIT(image->specials.affected_by,AFF_CURSE);
  REMOVE_BIT(image->specials.affected_by,AFF_POISON);
  REMOVE_BIT(image->specials.affected_by,AFF_PARALYSIS);
  REMOVE_BIT(image->specials.affected_by,AFF_CHARM);
  REMOVE_BIT(image->specials.affected_by,AFF_GROUP);

  image->player.class=GET_CLASS(vict);
if (vict->specials.fighting && vict != ch) {
   stop_fighting(vict);
   set_fighting(image,vict);
   }

 if(vict->specials.fighting && vict == ch) {
   stop_fighting(ch->specials.fighting);
   set_fighting(image,ch->specials.fighting);
   }

act("$n gestures towards $N and creates a doppelganger of $M.",
        FALSE, ch, 0, vict, TO_NOTVICT);
    act("$n gestures and creates a doppelganger of you!", FALSE, ch, 0, vict,
TO_VICT);
 if (vict != ch) {
    act("You gesture towards $N and create a doppelganger of $M",
        FALSE, ch, 0, vict, TO_CHAR);
    return;
   }
  send_to_char("You create a doppelganger of yourself to fool your enemies.\n\r",ch);
  return;
}

void spell_tranquility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!victim->specials.fighting)
    return;

  act("$n whispers a word to $N who frowns and stops fighting.",
      FALSE,ch,0,victim,TO_NOTVICT);
  act("$n whispers a few soothing words.  A feeling of peace fills you.",
      FALSE,ch,0,victim,TO_VICT);
  act("You whisper to $N who frowns and stops fighting.", FALSE,ch,0,victim,TO_CHAR);

  WAIT_STATE(victim, PULSE_VIOLENCE *2);

  stop_fighting(victim->specials.fighting);
  stop_fighting(victim);
}

void spell_leech (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  CHAR *v;
  int ppl = 0,drain = 0,power = 600;

    ppl = count_mortals_room(ch,TRUE);

    if(ppl > 0)
      drain = (int) (power/ppl);
    else
      return;


  act("You feel weaker as $n draws magical energy from you.",
      FALSE,ch,0,vict,TO_NOTVICT);

  act("You feel refreshed as you draw magical energy from the world.",
      FALSE,ch,0,vict,TO_CHAR);

    for(v = world[CHAR_REAL_ROOM(ch)].people;v;v = v->next_in_room)
      if(ch != v && IS_MORTAL(v) && !IS_NPC(v))
  drain_mana_hit_mv(ch,v,drain,0,0,TRUE,FALSE,FALSE);
}


void spell_armageddon(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= MAX_LEVEL));

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = number(1800, 2200);

  if (!IS_NPC(ch) && !IS_NPC(victim)) {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
    return;
  }

  act("$N screams loudly as a spear of pure energy hits $S body.",
      FALSE, ch, 0, victim, TO_ROOM);
  act("You conjure up a spear of energy and throw it at $N.", FALSE, ch, 0, victim, TO_CHAR);
  send_to_char("You scream loudly when a energy spear hits your body.\n\r",
         victim);
  damage(ch, victim, dam, SPELL_ARMAGEDDON,DAM_MAGICAL);
}
*/


void spell_perceive(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if(affected_by_spell(victim, SPELL_PERCEIVE))
  {
    send_to_char("You are already affected by this spell.\n\r",ch);

    return;
  }

  af.type      = SPELL_PERCEIVE;
  af.duration  = 2*level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = 0;
  af.bitvector2 = 0;

  affect_to_char(victim, &af);

  send_to_char("Your eyes glow with unearthly light.\n\r", victim);
}


void spell_quick(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("The forces of Chaos disrupt your magic.\n\r",ch);

    return;
  }

  if (!affected_by_spell(victim, SPELL_QUICK))
  {
    af.type      = SPELL_QUICK;
    af.duration  = 5;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = 0;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);

    send_to_char("Your thoughts begin to race.\n\r", victim);
    act("$n's mind begins to race.",FALSE,ch,0,0,TO_ROOM);
  }
}

void spell_divine_intervention(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("The forces of Chaos disrupt your magic.\n\r",ch);
    return;
  }

  if ( affected_by_spell(victim, SPELL_DIVINE_INTERVENTION) )
    return;

  af.type      = SPELL_DIVINE_INTERVENTION;
  af.duration  = 8;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = 0;
  af.bitvector2 = 0;

  affect_to_char(victim, &af);
  act("$n bestows you with the gods divine protection.\n\r", FALSE, ch, 0, victim, TO_VICT);
}

void spell_rush(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("The forces of Chaos disrupt your magic.\n\r",ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_RUSH))
  {
    af.type      = SPELL_RUSH;
    af.duration  = 3;
    af.modifier  = 5;
    af.location  = APPLY_DAMROLL;
    af.bitvector = 0;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);

    send_to_char("Your pulse begins to race!!\n\r", victim);
    act("$n veins bulge and twist as $s movement speeds up!",FALSE,ch,0,0,TO_ROOM);
  }
}


void spell_blood_lust(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(victim, SPELL_BLOOD_LUST))
  {
     af.type      = SPELL_BLOOD_LUST;
     af.duration  = 3;
     af.modifier  = 0;
     af.location  = APPLY_NONE;
     af.bitvector = 0;
     af.bitvector2 = 0;

    affect_to_char(victim, &af);

    send_to_char("Your body writhes with a gnawing hunger for blood!\n\r", victim);
    act("$n body writhes with a gnawing hunger for blood!",FALSE,ch,0,0,TO_ROOM);
  }
}


void spell_shroud_existence(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  int dam = 0;
  int heal = 0;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n", ch);

    return;
  }

  if (!(IS_GOOD(ch)))
  {
    send_to_char("Your malicious predilections for doing evil forbid the use of this spell!\n", ch);

    return;
  }

  if (!ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) &&
      !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA) &&
      (IS_AFFECTED(victim, AFF_CHARM) || (IS_MORTAL(victim) && IS_SET(ch->specials.pflag, PLR_NOKILL))))
  {
    act("You cannot cast this spell on $N.", FALSE, ch, NULL, victim, TO_CHAR);

    return;
  }

  dam = MIN(GET_LEVEL(ch), 50);

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
  {
    dam *= 5;
  }
  else
  {
    dam *= 9;
  }

  if (IS_AFFECTED(victim, AFF_SPHERE) &&
      !breakthrough(ch, victim, BT_SPHERE))
  {
    dam = 0;
  }

  heal = MAX(0, MIN(GET_HIT(victim), IS_AFFECTED(victim, AFF_SANCTUARY) ? (dam / 2) : dam));

  if (heal > 0)
  {
    //GET_HIT(ch) += heal;
    magic_heal(ch, SPELL_EXISTENCE, heal, FALSE);

    send_to_char("You feel new life flowing into you.\n\r", ch);
  }
  else
  {
    send_to_char("No life flows into you!\n\r", ch);
  }

  update_pos(ch);

  damage(ch, victim, dam, SPELL_EXISTENCE, DAM_NO_BLOCK);
}


void spell_mystic_swiftness(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("The forces of Chaos disrupt your magic.\n\r", ch);

    return;
  }

  if (!affected_by_spell(ch, SPELL_MYSTIC_SWIFTNESS))
  {
    af.type       = SPELL_MYSTIC_SWIFTNESS;
    af.duration   = 4;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;

    affect_to_char(ch, &af);

    send_to_char("Your hands blur with mystical speed!\n\r", ch);
    act("$n's hands blur with mystical speed!", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void spell_wind_slash(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  CHAR *temp_victim = NULL, *next_victim = NULL;
  int dam = 0;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  dam = (level / 2) + number(140, 180);

  send_to_char("You slash the air with rapid strikes, sending blades of wind in all directions.\n\r", ch);
  act("$n slashes the air with rapid strikes, sending blades of wind in all directions.", TRUE, ch, 0, 0, TO_ROOM);

  for (temp_victim = world[CHAR_REAL_ROOM(ch)].people; temp_victim; temp_victim = next_victim)
  {
    next_victim = temp_victim->next_in_room;

    if (IS_IMMORTAL(temp_victim)) continue;

    if (ch != temp_victim && ((IS_NPC(ch) ? !IS_NPC(temp_victim) : IS_NPC(temp_victim)) || ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_victim))))
    {
      damage(ch, temp_victim, dam, SPELL_WIND_SLASH, DAM_MAGICAL);
    }
  }
}
