/* ************************************************************************
*  file: spells1.c , handling of magic.                   Part of DIKUMUD *
*  Usage : Procedures handling all offensive magic.                       *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
**************************************************************************/

/*
$Author: void $
$Date: 2004/11/19 17:20:33 $
$Header: /home/ronin/cvs/ronin/spells.c,v 2.8 2004/11/19 17:20:33 void Exp $
$Id: spells.c,v 2.8 2004/11/19 17:20:33 void Exp $
$Name:  $
$Log: spells.c,v $
Revision 2.8  2004/11/19 17:20:33  void
Changed name of lvl 50 ninja spell to mystic swiftness

Revision 2.7  2004/11/19 14:47:21  void
New Level 50 Ninja Spell (adds chance of 3rd hit)

Revision 2.6  2004/10/21 17:08:22  void
Added level 50 AP spell Blood Lust

Revision 2.5  2004/09/29 22:56:13  void
Spell Rush for Commando

Revision 2.4  2004/09/28 14:28:01  void
Added Level 50 Cleric Spell Divine Intervention

Revision 2.3  2004/09/22 21:58:26  void
Added level 50 Mage spell Quick (Allows for 2 casts in a single combat
round)

Revision 2.2  2004/06/30 19:57:28  ronin
Update of saving throw routine.

Revision 2.1  2004/02/19 19:20:22  void
Added Spell Shroud of Existence (Good Align Commando Spell)

Revision 2.0.0.1  2004/02/05 16:11:45  ronin
Reinitialization of cvs archives

Revision 1.3  2002/10/27 11:59:36  ronin
Vent fixed to show player rank, also fixed
to show who is casting to imm+.

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "utility.h"
#include "limits.h"
#include "subclass.h"

/* Global data */

extern int CHAOSMODE;
extern struct room_data *world;
extern struct char_data *character_list;

void magic_heal(CHAR *ch, int spell, int heal, bool overheal)
{
  if (!ch) return;

  if (!IS_NPC(ch) && check_subclass(ch, SC_BANDIT, 3))
  {
    heal = heal + (heal * number(10, 20)) / 100;
  }

  if (!overheal)
  {
    heal = MIN(heal, GET_MAX_HIT(ch) - GET_HIT(ch));
  }

  if ((spell != SKILL_MANTRA) &&
      affected_by_spell(ch, SPELL_DEGENERATE) &&
      ((duration_of_spell(ch, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(ch, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))))
  {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", ch);
    return;
  }

  GET_HIT(ch) = GET_HIT(ch) + heal;
}

/* Extern functions */

void cast_burning_hands(ubyte level,CHAR *ch,char *arg,int type,
			CHAR *victim, OBJ *tar_obj )
/* Mana: 17 Damage: 18-25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_burning_hands(level, ch, victim, 0);
    break;
  default :
    log_f("Serious screw-up in burning hands!");
    break;
  }
}

void cast_hell_fire(ubyte level, CHAR *ch, char *arg, int type,
		    CHAR *victim, OBJ *tar_obj )
/* Mana: 130 Damage: 200 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_hell_fire(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in hell fire!");
    break;
  }
}

void cast_death_spray(ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *victim, OBJ *tar_obj )
/* Mana: 80  Minus 100 hps on mobs */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_death_spray(level, ch, victim, 0);
    break;
  default:
    log_f("Wrong type called in death spray!");
    break;
  }
}

void cast_holy_word(ubyte level, CHAR *ch, char *arg, int type,
		    CHAR *victim, OBJ *tar_obj )
/* Mana: 80 Damage: 130 on Evil */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_holy_word(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in holy word!");
    break;
  }
}

void cast_evil_word(ubyte level, CHAR *ch, char *arg, int type,
		    CHAR *victim, OBJ *tar_obj )
/* Mana: 80 Damage: 130 on Good */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_evil_word(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in evil word!");
    break;
  }
}

void cast_call_lightning(ubyte level, CHAR *ch, char *arg, int type,
			 CHAR *victim, OBJ *tar_obj )
/* Mana: 15 Damage: (level)d8 */
{
  CHAR *tmp;
  extern struct weather_data weather_info;

  switch (type) {
  case SPELL_TYPE_SPELL:
    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING))
      spell_call_lightning(level, ch, victim, 0);
    else
      send_to_char("You fail to call upon the lightning from the sky!\n\r",ch);
    break;
  case SPELL_TYPE_POTION:
    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING))
      spell_call_lightning(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
      if(victim)
	spell_call_lightning(level, ch, victim, 0);
      else if (!tar_obj) spell_call_lightning(level, ch, ch, 0);
    }
    break;
  case SPELL_TYPE_STAFF:
    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING))
      for (victim = world[CHAR_REAL_ROOM(ch)].people ;
	   victim ; victim = tmp ) {
	tmp = victim->next_in_room;
	if (victim != ch) spell_call_lightning(level, ch, victim, 0);
      }
    break;
  default :
    log_f("Wrong type called in call lightning!");
    break;
  }
}

void cast_chill_touch( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *victim, OBJ *tar_obj )
/* Mana: 15 Damage: 11-16 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_chill_touch(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in chill touch!");
    break;
  }
}

void cast_shocking_grasp( ubyte level, CHAR *ch, char *arg, int type,
			 CHAR *victim, OBJ *tar_obj )
/* Mana: 19 Damage: 33-40 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_shocking_grasp(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in shocking grasp!");
    break;
  }
}

void cast_colour_spray( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *victim, OBJ *tar_obj )
/* Mana: 23 Damage: 80 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_colour_spray(level, ch, victim, 0);
    break;
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_colour_spray(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in colour spray!");
    break;
  }
}

void cast_earthquake( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *victim, OBJ *tar_obj )
/* Mana: 15 Damage: 1d8+level */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_STAFF:
    spell_earthquake(level, ch, 0, 0);
    break;
  default :
    log_f("Wrong type called in earthquake!");
    break;
  }
}

void cast_energy_drain( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *victim, OBJ *tar_obj )
/* Mana: 35  Gain exp from others */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_energy_drain(level, ch, victim, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_energy_drain(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (victim = world[CHAR_REAL_ROOM(ch)].people ;
	 victim ; victim = victim->next_in_room )
      if(victim != ch)
	spell_energy_drain(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in energy drain!");
    break;
  }
}

void cast_fireball( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *victim, OBJ *tar_obj )
/* Mana: 25 Damage: 100 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_fireball(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in fireball!");
    break;
  }
}


void cast_iceball( ubyte level, CHAR *ch, char *arg, int type,
		  CHAR *victim, OBJ *tar_obj )
/* Mana: 30 Damage: 130 */
{
  CHAR *victim_n;
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_iceball(level, ch, victim, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (victim = world[CHAR_REAL_ROOM(ch)].people ;
	 victim ; victim = victim_n) {
      victim_n=victim->next_in_room;
      if(victim == ch) continue;
      if(!IS_NPC(victim) && GET_LEVEL(victim)>=LEVEL_IMM) continue;
      spell_iceball(level, ch, victim, 0);
    }
    break;
  default :
    log_f("Wrong type called in iceball!");
    break;
  }
}

void cast_harm( ubyte level, CHAR *ch, char *arg, int type,
	       CHAR *victim, OBJ *tar_obj )
/* Mana: 35 Leaves all but 1d4 hps max Damage: 100 */
{
  CHAR *temp;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_harm(level, ch, victim, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_harm(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF: /* Added temp - Ranger June 96 */
    for (victim = world[CHAR_REAL_ROOM(ch)].people ; victim ; victim = temp) {
      temp = victim->next_in_room;
      if(victim != ch) spell_harm(level, ch, victim, 0);
    }
    break;
  default :
    log_f("Wrong type called in harm!");
    break;
  }
}

void cast_super_harm( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *victim, OBJ *tar_obj )
/* Mana: 100 Leaves all but 1d4 hps max Damage: 600 */
{
  CHAR *temp;
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_super_harm(level, ch, victim, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_super_harm(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (victim = world[CHAR_REAL_ROOM(ch)].people ;
	 victim ; victim = temp) {
      temp = victim->next_in_room;
      if(victim != ch) spell_super_harm(level, ch, victim, 0);
    }
    break;
  default :
    log_f("Wrong type called in super harm!");
    break;
  }
}

void cast_lightning_bolt( ubyte level, CHAR *ch, char *arg, int type,
			 CHAR *victim, OBJ *tar_obj )
/* Mana: 20 Damage: 50-60 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_lightning_bolt(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in lightning bolt!");
    break;
  }
}

void cast_flamestrike( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *victim, OBJ *tar_obj )
/* Mana: 25 Damage: 80 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_flamestrike(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in flamestrike!");
    break;
  }
}

void cast_lethal_fire( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *victim, OBJ *tar_obj )
/* Mana: 50 Damage: 180-230 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_lethal_fire(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in lethal fire!");
    break;
  }
}


void cast_thunderball( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *victim, OBJ *tar_obj )
/* Mana: 200 Damage: 800-1100 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_thunderball(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in thunderball!");
    break;
  }
}

void cast_electric_shock( ubyte level, CHAR *ch, char *arg, int type,
			 CHAR *victim, OBJ *tar_obj )
/* Mana: 100 Damage: 450-500 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    if (victim)
      spell_electric_shock(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in electric shock!");
    break;
  }
}

void cast_magic_missile( ubyte level, CHAR *ch, char *arg, int type,
			CHAR *victim, OBJ *tar_obj )
/* Mana: 5 Damage: 6-10 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_magic_missile(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in magic missile!");
    break;
  }
}

void cast_mass_invisibility (ubyte level, CHAR *ch, char *arg,
			     int type, CHAR *victim,OBJ *obj)
/* Mana: 100  Works on all pcs in room */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_STAFF:
    spell_mass_invisibility (level, ch, 0, 0);
    break;
  default:
    log_f("cast_mass_invisibility called with wrong type!");
    break;
  }
}

void cast_power_word_kill (ubyte level,CHAR *ch, char *arg, int type,
			   CHAR *victim, OBJ *obj)
/* Mana: 100 Can kill victims weeker than caster */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
    spell_power_word_kill(level, ch, victim, 0);
    break;
  default:
    log_f("cast_power_word_kill called with wrong type!");
    break;
  }
}

void cast_dispel_magic (ubyte level, CHAR *ch, char *arg, int type,
			CHAR *victim, OBJ *obj)
/* Mana: 20 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_SCROLL:
    if (victim || obj)
      spell_dispel_magic(level,ch,victim,obj);
    break;
  default:
    log_f("cast_dispel_magic called with wrong type!");
    break;
  }
}

void cast_convergence (ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *victim, OBJ *obj)
/* Mana: 20 Uses targets mana to heal him/her */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_convergence(level,ch,victim,0);
    break;
  case SPELL_TYPE_POTION:
    spell_convergence(level,ch,ch,0);
  default:
    log_f("cast_convergence called with wrong type!");
    break;
  }
}

void cast_enchant_armour (ubyte level, CHAR *ch, char *arg, int type,
			  CHAR *victim, OBJ *obj)
/* Mana: 100 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
    spell_enchant_armour (level,ch,0,obj);
    break;
  default:
    log_f("cast_enchant_armour called with wrong type!");
    break;
  }
}

void cast_disintegrate (ubyte level, CHAR *ch, char *arg, int type,
			CHAR *victim, OBJ *obj)
/* Mana: 150 Damage: 10-20(level) also destroys objects */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    if (victim || obj)
      spell_disintegrate (level,ch,victim,obj);
    break;
  default:
    log_f("cast_disintegrate called with wrong type!");
    break;
  }
}

void cast_confusion (ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *victim, OBJ *obj)
/* Mana: 20 Current only 31+ can cast */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_confusion(level,ch,victim,0);
    break;
  case SPELL_TYPE_POTION:
    spell_confusion(level,ch,ch,0);
    break;
  default:
    log_f("cast_confusion called with wrong type!");
    break;
  }
}

void cast_conflagration (ubyte level, CHAR *ch, char *arg, int type,
			 CHAR *victim, OBJ *obj)
/* Mana: 200 Damage: 100 May also affect pcs in room */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    spell_conflagration (level, ch, victim, 0);
    break;
  default:
    log_f("cast_conflagration called with wrong type!");
    break;
  }
}

void cast_vampiric_touch (ubyte level, CHAR *ch, char *arg, int type,
			  CHAR *victim, OBJ *obj)
/* Mana: 50  Drains energy and transfers to caster */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_vampiric_touch(level,ch,victim,0);
    break;
  default:
    log_f("cast_vampiric_touch with wrong type!");
    break;
  }
}

void cast_searing_orb (ubyte level, CHAR *ch, char *arg,
		       int type, CHAR *victim, OBJ *obj)
/* Mana: 60 Destroy flamable objects in area */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
    spell_searing_orb(level,ch,0,0);
    break;
  default:
    log_f("cast_searing_orb with wrong type!");
    break;
  }
}

void cast_clairvoyance (ubyte level, CHAR *ch, char *arg, int type,
			CHAR *victim, OBJ *obj)
/* Mana: 35 See through anothers eyes */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
    spell_clairvoyance (level,ch,victim,0);
    break;
  default:
    log_f("cast_clairvoyance with wrong type!");
    break;
  }
}

void cast_animate_dead(ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 */
{
  struct obj_data *o, *o_next;

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
    if(!tar_obj) return;
    spell_animate_dead(level, ch, 0,tar_obj);
    break;
  case SPELL_TYPE_STAFF:
    for (o = world[CHAR_REAL_ROOM(ch)].contents;o;o = o_next) {
      o_next = o->next_content;
      spell_animate_dead (level, ch, 0, o);
    }
  default :
    log_f("Wrong type called in Animate Dead!");
    break;
  }
}

void cast_spirit_levy(ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 15 Absorb energy from corpse */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_spirit_levy(level, ch, 0,tar_obj);
    break;

  default :
    log_f("Wrong type called in Spirit Levy!");
    break;
  }
}


void cast_armor(ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 5 affect ac by 10 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (ch != tar_ch)
      act("$N is protected by your deity.", FALSE, ch, 0, tar_ch, TO_CHAR);
    spell_armor(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_armor(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_armor(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in armor!");
    break;
  }
}

void cast_endure(ubyte level, CHAR *ch, char *arg, int type,
		 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 10 affect ac by 15 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (ch != tar_ch)
      act("$N is protected by your deity.", FALSE, ch, 0, tar_ch, TO_CHAR);
    spell_endure(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_endure(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_endure(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in endure!");
    break;
  }
}

void cast_teleport( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  struct char_data *o_next;

  switch (type) {
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:
    if (!tar_ch)
      tar_ch = ch;
    spell_teleport(level, ch, tar_ch, 0);
    break;

  case SPELL_TYPE_WAND:
    if(!tar_ch) return;
    spell_teleport(level, ch, tar_ch, 0);
    break;

  case SPELL_TYPE_STAFF:
    for (tar_ch=world[CHAR_REAL_ROOM(ch)].people;tar_ch;tar_ch=o_next) {
      o_next = tar_ch->next_in_room;
      if (tar_ch != ch)
	spell_teleport(level, ch, tar_ch, 0);
    }
    spell_teleport(level, ch, ch, 0);
    break;

  default :
    log_f("Wrong type called in teleport!");
    break;
  }
}

void cast_firebreath( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 120 Six breaths of fireball */
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    if(GET_POS(tar_ch) == POSITION_FIGHTING)
      send_to_char("Your target won't stay still long enough to draw the runes around.\n\r", ch);
    else
      {
        if(ch!=tar_ch) {
  	  act("$n draws the sigil of fire around $N's feet.",FALSE,ch,0,tar_ch,TO_NOTVICT);
	  act("$n draws the sigil of fire around your feet.",FALSE,ch,0,tar_ch,TO_VICT);
	  act("You draw the sigil of fire around $N's feet.",FALSE,ch,0,tar_ch,TO_CHAR);
        }
        else {
  	  act("$n draws the sigil of fire around $s feet.",FALSE,ch,0,tar_ch,TO_NOTVICT);
	  act("You draw the sigil of fire around your feet.",FALSE,ch,0,0,TO_CHAR);
        }
	spell_firebreath(level,ch,tar_ch,0);
      }
    break;
  case SPELL_TYPE_POTION:
    act("$n looks like $e has indigestion.",FALSE,ch,0,ch,TO_NOTVICT);
    act("It burns in your stomach.",FALSE,ch,0,ch,TO_CHAR);
    spell_firebreath(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    act("As $n reads the scroll, the spirit of fire lifts from the page and\n\renvelops $N.",FALSE,ch,0,tar_ch,TO_NOTVICT);
    act("$n reads the scroll, and the spirit of fire envelopes you.",FALSE,ch,0,tar_ch,TO_VICT);
    act("You release the spirit of fire from the scroll.",FALSE,ch,0,tar_ch,TO_CHAR);
    spell_firebreath(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in firebreath!");
    break;
  }
}

void cast_bless( ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 5 Bless victim or object Hit +1 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) ) {
	send_to_char("Nothing seems to happen.\n\r", ch);
	return;
      }
      spell_bless(level,ch,0,tar_obj);
    } else {              /* Then it is a PC | NPC */
      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING)) {
	send_to_char("Nothing seems to happen.\n\r", ch);
	return;
      }
      spell_bless(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_BLESS) ||
	(GET_POS(ch) == POSITION_FIGHTING))
      return;
    spell_bless(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
	return;
      spell_bless(level,ch,0,tar_obj);

    } else {              /* Then it is a PC | NPC */

      if (!tar_ch) tar_ch = ch;

      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING))
	return;
      spell_bless(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
	return;
      spell_bless(level,ch,0,tar_obj);

    } else {              /* Then it is a PC | NPC */

      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING))
	return;
      spell_bless(level,ch,tar_ch,0);
    }
    break;
  default :
    log_f("Wrong type called in bless!");
    break;
  }
}

void cast_blindness( ubyte level, CHAR *ch, char *arg, int type,
		    CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 15 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( IS_AFFECTED(tar_ch, AFF_BLIND) ){
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_blindness(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if(IS_AFFECTED(ch, AFF_BLIND)) return;
    spell_blindness(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( IS_AFFECTED(ch, AFF_BLIND) )
      return;
    spell_blindness(level,ch,ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( IS_AFFECTED(ch, AFF_BLIND) )
      return;
    spell_blindness(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, AFF_BLIND)))
	  spell_blindness(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in blindness!");
    break;
  }
}

void cast_clone( ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 100 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    /* if (tar_ch) {
       sprintf(buf, "You create a duplicate of %s.\n\r", GET_NAME(tar_ch));
       send_to_char(buf, ch);
       sprintf(buf, "%%s creates a duplicate of %s,\n\r", GET_NAME(tar_ch));

       spell_clone(level,ch,tar_ch,0);
       } else {
       sprintf(buf, "You create a duplicate of %s %s.\n\r",SANA(tar_obj),OBJ_SHORT(tar_obj));
       send_to_char(buf, ch);
       sprintf(buf, "%%s creates a duplicate of %s %s,\n\r",SANA(tar_obj),OBJ_SHORT(tar_obj));
       */
    spell_clone(level,ch,0,tar_obj);
    break;


  default :
    log_f("Wrong type called in clone!");
    break;
  }
  /* MISSING REST OF SWITCH -- POTION, SCROLL, WAND */
}

void cast_control_weather( ubyte level, CHAR *ch, char *arg, int type,
			  CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  char buffer[MAX_STRING_LENGTH];
  extern struct weather_data weather_info;
  level=MIN(level,30);
  switch (type) {
  case SPELL_TYPE_SPELL:

    one_argument(arg,buffer);

    if (strcmp("better",buffer) && strcmp("worse",buffer))
      {
	send_to_char("Do you want it to get better or worse?\n\r",ch);
	return;
      }

    if(!strcmp("better",buffer))
      weather_info.change+=(dice(((level)/2),4));
    else
      weather_info.change-=(dice(((level)/2),4));
    break;
  default :
    log_f("Wrong type called in control weather!");
    break;
  }
}

void cast_create_food( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 5 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    act("$n magically creates a mushroom.",FALSE, ch, 0, 0, TO_ROOM);
    spell_create_food(level,ch,0,0);
    break;
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_STAFF:
    spell_create_food(level,ch,0,0);
    break;
  default :
    log_f("Wrong type called in create food!");
    break;
  }
}

void cast_create_water( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 5 */
{
  switch (type) {
  case SPELL_TYPE_WAND:
    if (tar_obj)
      {
	if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON)
	  return;
      }
    else
      return;
     spell_create_water(level, ch, 0, tar_obj);
    break;

  case SPELL_TYPE_SPELL:
    if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
      send_to_char("It is unable to hold water.\n\r", ch);
      return;
    }
    spell_create_water(level,ch,0,tar_obj);
    break;

  default :
    log_f("Wrong type called in create water!");
    break;
  }
}

void cast_blindness_dust( ubyte level, CHAR *ch, char *arg, int type,
			 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 20 Blind everyone in room */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_blindness_dust(level,ch,0,0);
    break;
  default :
    log_f("Wrong type called in blindness dust!");
    break;
  }
}

void cast_poison_smoke( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 20 Poison everyone in room */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_poison_smoke(level,ch,0,0);
    break;
  default :
    log_f("Wrong type called in poison smoke!");
    break;
  }
}

void cast_hypnotize( ubyte level, CHAR *ch, char *arg, int type,
		    CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 40 Sleep or charm victim */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_hypnotize(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in hyptonize!");
    break;
  }
}

void cast_reappear( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 20 For use on objects */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_reappear(level,ch,tar_ch,tar_obj);
    break;
  default :
    log_f("Wrong type called in reappear!");
    break;
  }
}

void cast_reveal( ubyte level, CHAR *ch, char *arg, int type,
		 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 30 For use on pcs */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_reveal(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in reveal!");
    break;
  }
}

void cast_relocation( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 Relocate to victim */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (!CAN_SEE(ch, tar_ch)) {
      send_to_char("No-one by that name here!\n\r", ch);
      return;
    }
    spell_relocation(level,ch,tar_ch,0);
    break;
  default:
    log_f("Wrong type called in relocation!");
    break;
  }
}

void cast_cure_blind( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 15 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_blind(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_blind(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_cure_blind(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in cure blind!");
    break;
  }
}

void cast_cure_critic( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 Cure 30 hps */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
    spell_cure_critic(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_critic(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_cure_critic(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in cure critic!");
    break;
  }
}

void cast_cure_light_spray( ubyte level, CHAR *ch, char *arg, int type,
			   CHAR *victim, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_light_spray(level,ch,0,0);
    break;
  default :
    log_f("Wrong type called in cure critic spray!");
    break;
  }
}

void cast_great_miracle( ubyte level, CHAR *ch, char *arg, int type,
			CHAR *victim, OBJ *tar_obj )
/* Mana: 200 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    spell_great_miracle(level,ch,0,0);
    break;
  default :
    log_f("Wrong type called in great miracle!");
    break;
  }
}

void cast_heal_spray( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *victim, OBJ *tar_obj )
/* Mana: 100 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_heal_spray(level,ch,0,0);
    break;
  default :
    log_f("Wrong type called in heal spray!");
    break;
  }
}

void cast_cure_serious_spray( ubyte level, CHAR *ch, char *arg, int type,
			     CHAR *victim, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_serious_spray(level,ch,0,0);
    break;
  default :
    log_f("Wrong type called in cure serious spray!");
    break;
  }
}

void cast_cure_critic_spray( ubyte level, CHAR *ch, char *arg, int type,
			    CHAR *victim, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_critic_spray(level,ch,0,0);
    break;
  default :
    log_f("Wrong type called in cure critic spray!");
    break;
  }
}

void cast_cure_serious( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 18 Cure 12 hps */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_serious(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_serious(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_cure_serious(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in cure serious!");
    break;
  }
}

void cast_fear( ubyte level, CHAR *ch, char *arg, int type,
	       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 15 */
{
  CHAR *temp;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fear(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_STAFF: /* Added temp - Ranger June 96 */
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = temp) {
       temp = tar_ch->next_in_room;
       if (tar_ch != ch) spell_fear(level,ch,tar_ch,0);
    }
    break;
  default :
    log_f("Wrong type called in fear!");
    break;
  }
}

void cast_forget( ubyte level, CHAR *ch, char *arg, int type,
		 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 15 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_forget(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in forget!");
    break;
  }
}

void cast_fly( ubyte level, CHAR *ch, char *arg, int type,
	      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_FLY) ){
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_fly(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_FLY) )
      return;
    spell_fly(level,ch,ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( affected_by_spell(tar_ch, SPELL_FLY) )
      return;
    spell_fly(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_fly (level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in fly!");
    break;
  }
}

void cast_infravision( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 20 See in the dark */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_INFRAVISION) ){
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    if (ch != tar_ch)
      act("$N seems to be glowing with a bright light.",
	  FALSE, ch, 0, tar_ch, TO_CHAR);

    spell_infravision(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_INFRAVISION) )
      return;
    spell_infravision(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (!tar_ch) tar_ch = ch;
    if ( affected_by_spell(tar_ch, SPELL_INFRAVISION) )
      return;
    spell_infravision(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_WAND:
    if ( affected_by_spell(tar_ch, SPELL_INFRAVISION) )
      return;
    spell_infravision(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in infravision!");
    break;
  }
}

void cast_vitality( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 Restore mv points */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_vitality(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_vitality(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_vitality(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in vitality!");
    break;
  }
}

void cast_miracle( ubyte level, CHAR *ch, char *arg, int type,
		  CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 100 Restore victims hps */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_miracle(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_miracle(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_miracle(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in miracle!");
    break;
  }
}

void cast_fury(ubyte level, CHAR *ch, char *arg, int type, CHAR *tar_ch, OBJ *tar_obj)
/* Mana: 60  Double damage for hits */
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
      spell_fury(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_fury(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = world[CHAR_REAL_ROOM(ch)].people; tar_ch; tar_ch = tar_ch->next_in_room)
        spell_fury(level, ch, tar_ch, 0);
      break;
    default:
      log_f("Wrong type called in fury!");
      break;
  }
}

void cast_mana_transfer( ubyte level, CHAR *ch, char *arg, int type,
			CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_mana_transfer(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in mana transfer!");
    break;
  }
}

void cast_holy_bless( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 30 Fix alignment */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_holy_bless(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_holy_bless(level,ch,ch,0);
    break;
  default :
    log_f("Wrong type called in holy bless!");
    break;
  }
}

void cast_evil_bless( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 30 Fix alignment */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_evil_bless(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_evil_bless(level,ch,ch,0);
    break;
  default :
    log_f("Wrong type called in evil bless!");
    break;
  }
}

void cast_satiate( ubyte level, CHAR *ch, char *arg, int type,
		  CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 10 Fill your stomach */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_satiate(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_satiate(level,ch,ch,0);
    break;
  default :
    log_f("Wrong type called in satiate!");
    break;
  }
}

void cast_cure_light( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 15 Cure 10 hps */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_light(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_light(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_cure_light(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in cure light!");
    break;
  }
}

void cast_curse( ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 20 Reduce hit by 1 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj)   /* It is an object */
      spell_curse(level,ch,0,tar_obj);
    else {              /* Then it is a PC | NPC */
      spell_curse(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_POTION:
    spell_curse(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)   /* It is an object */
      spell_curse(level,ch,0,tar_obj);
    else {              /* Then it is a PC | NPC */
      if (!tar_ch) tar_ch = ch;
      spell_curse(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_curse(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in curse!");
    break;
  }
}

void cast_detect_alignment( ubyte level, CHAR *ch, char *arg, int type,
			   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_DETECT_ALIGNMENT) ){
      send_to_char("Nothing seems to happen.\n\r", tar_ch);
      return;
    }
    spell_detect_alignment(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_DETECT_ALIGNMENT) )
      return;
    spell_detect_alignment(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if(!(IS_AFFECTED(tar_ch, SPELL_DETECT_ALIGNMENT)))
	spell_detect_alignment(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in detect alignment!");
    break;
  }
}

void cast_detect_invisibility( ubyte level, CHAR *ch, char *arg, int type,
			      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_DETECT_INVISIBLE) ){
      send_to_char("Nothing seems to happen.\n\r", tar_ch);
      return;
    }
    spell_detect_invisibility(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_DETECT_INVISIBLE) )
      return;
    spell_detect_invisibility(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if(!(IS_AFFECTED(tar_ch, SPELL_DETECT_INVISIBLE)))
	spell_detect_invisibility(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in detect invisibility!");
    break;
  }
}

void cast_detect_magic( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 5 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_DETECT_MAGIC) ){
      send_to_char("Nothing seems to happen.\n\r", tar_ch);
      return;
    }
    spell_detect_magic(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_DETECT_MAGIC) )
      return;
    spell_detect_magic(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (!(IS_AFFECTED(tar_ch, SPELL_DETECT_MAGIC)))
	spell_detect_magic(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in detect magic!");
    break;
  }
}

void cast_recover_mana( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_recover_mana(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_recover_mana(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
	  spell_recover_mana(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in recover mana!");
    break;
  }
}

void cast_detect_poison( ubyte level, CHAR *ch, char *arg, int type,
			CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 5 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_detect_poison(level, ch, tar_ch,tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_detect_poison(level, ch, ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {
      spell_detect_poison(level, ch, 0, tar_obj);
      return;
    }
    if (!tar_ch) tar_ch = ch;
    spell_detect_poison(level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in detect poison!");
    break;
  }
}

void cast_dispel_evil( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_dispel_evil(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_dispel_evil(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_dispel_evil(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    spell_dispel_evil(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_dispel_evil(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in dispel evil!");
    break;
  }
}

void cast_dispel_good( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_dispel_good(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_dispel_good(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_dispel_good(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    spell_dispel_good(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_dispel_good(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in dispel good!");
    break;
  }
}


void cast_enchant_weapon( ubyte level, CHAR *ch, char *arg, int type,
			 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_enchant_weapon(level, ch, 0,tar_obj);
    break;

  case SPELL_TYPE_SCROLL:
    if(!tar_obj) return;
    spell_enchant_weapon(level, ch, 0,tar_obj);
    break;
  default :
    log_f("Wrong type called in enchant weapon!");
    break;
  }
}


void cast_heal( ubyte level, CHAR *ch, char *arg, int type,
	       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 Cures 100 hps */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if(ch==tar_ch) {
      act("$n heals $mself.", FALSE, ch, 0, 0, TO_ROOM);
      act("You heal yourself.", FALSE, ch, 0, 0, TO_CHAR);
    }
    else {
      act("$n heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
      act("You heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
    }
    spell_heal(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_heal(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_heal(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in heal!");
    break;
  }
}

/* Cures 100 mana */
void cast_mana_heal(ubyte level, CHAR *ch, char *arg, int type, CHAR *tar_ch, OBJ *tar_obj)
{
	switch(type)
	{
		case SPELL_TYPE_SPELL:
		{
			if(ch == tar_ch)
			{
				act("$n restores $mself slightly.",0,ch,0,0, TO_ROOM);
				act("You restore yourself slightly.",0,ch,0,0, TO_CHAR);
			}
			else
			{
				act("$n restores $N slightly.",0,ch,0,tar_ch, TO_NOTVICT);
				act("You restore $N slightly.",0,ch,0,tar_ch, TO_CHAR);
			}
			spell_mana_heal(level, ch, tar_ch, 0);
			break;
		}

		case SPELL_TYPE_POTION:
		{
			spell_mana_heal(level, ch, ch, 0);
			break;
		}

		case SPELL_TYPE_STAFF:
		{
			for(tar_ch = world[CHAR_REAL_ROOM(ch)].people; tar_ch; tar_ch = tar_ch->next_in_room)
				spell_mana_heal(level, ch, tar_ch, 0);
			break;
		}
		default :
		{
			log_f("Wrong type called in mana_heal!");
			break;
		}
	}
}

/*
void cast_shapeshift( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
          act("$n falls down on the ground, clutching $s stomach.",FALSE,ch,0,tar_ch,TO_NOTVICT);
	  act("You fall down on the ground, clutching your stomach.",FALSE,ch,0,0,TO_CHAR);
          spell_shapeshift(level,ch,tar_ch,0);
          break;

  case SPELL_TYPE_SCROLL:
    act("As $n reads the scroll, the spirit of the wolf lifts from the page and\n\renvelops $N.",FALSE,ch,0,tar_ch,TO_NOTVICT);
    act("$n reads the scroll, and the spirit of the wolf envelopes you.",FALSE,ch,0,tar_ch,TO_VICT);
    act("You release the wolf spirit from the scroll.",FALSE,ch,0,tar_ch,TO_CHAR);
    spell_shapeshift(level,ch,tar_ch,0);
    break;

  default :
    log_f("Wrong type called in shapeshift!");
    break;
  }
 }

void cast_silence( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  	case SPELL_TYPE_SPELL:
        if(ch!=tar_ch) {
          act("$n makes a choking gesture towards $N.",FALSE,ch,0,tar_ch,TO_NOTVICT);
	  act("$n makes a choking gesture towards you.",FALSE,ch,0,tar_ch,TO_VICT);
	  act("You make a choking gesture towards $N's throat.",FALSE,ch,0,tar_ch,TO_CHAR);
        }
        else {
          act("$n draws a few symbols on $s forehead.",FALSE,ch,0,tar_ch,TO_NOTVICT);
	  act("You draw the symbols of stealth on your forehead.",FALSE,ch,0,0,TO_CHAR);
        }
	spell_silence(level,ch,tar_ch,0);
        break;

    default :
    log_f("Wrong type called in silence!");
    break;
   }
  }


void cast_quester( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  	case SPELL_TYPE_SPELL:
          act("$n touches $N gently on the forehead.",FALSE,ch,0,tar_ch,TO_NOTVICT);
	  act("$n touches you gently on the forehead.",FALSE,ch,0,tar_ch,TO_VICT);
	  act("You touch $N gently on the forehead.",FALSE,ch,0,tar_ch,TO_CHAR);
	spell_quester(level,ch,tar_ch,0);
        break;

    default :
    log_f("Wrong type called in quest champion!");
    break;
   }
  }
*/

void cast_regeneration( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 100 Regen of mana and hps */
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    if(GET_POS(tar_ch) == POSITION_FIGHTING)
      send_to_char("Your target won't stay still long enough to draw the runes around.\n\r", ch);
    else
      {
        if(ch!=tar_ch) {
          act("$n draws the sigil of growth around $N's feet.",FALSE,ch,0,tar_ch,TO_NOTVICT);
	  act("$n draws the sigil of growth around your feet.",FALSE,ch,0,tar_ch,TO_VICT);
	  act("You draw the sigil of growth around $N's feet.",FALSE,ch,0,tar_ch,TO_CHAR);
        }
        else {
          act("$n draws the sigil of growth around $s feet.",FALSE,ch,0,tar_ch,TO_NOTVICT);
	  act("You draw the sigil of growth around your feet.",FALSE,ch,0,0,TO_CHAR);
        }
	spell_regeneration(level,ch,tar_ch,0);
      }
    break;
  case SPELL_TYPE_POTION:
    spell_regeneration(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    act("As $n reads the scroll, the spirit of the forest lifts from the page and\n\renvelops $N.",FALSE,ch,0,tar_ch,TO_NOTVICT);
    act("$n reads the scroll, and the spirit of the forest envelopes you.",FALSE,ch,0,tar_ch,TO_VICT);
    act("You release the spirit of the forest from the scroll.",FALSE,ch,0,tar_ch,TO_CHAR);
    spell_regeneration(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in regeneration!");
    break;
  }
}
void cast_layhands( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 100 10*level hps cured */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    act("$n lays hands on $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
    act("You lay hands on $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
    spell_layhands(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
    spell_layhands(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
         tar_ch ; tar_ch = tar_ch->next_in_room)
         spell_layhands(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in layhands!");
    break;
  }
}
void cast_hold( ubyte level, CHAR *ch, char *arg, int type,
	       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 15 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_hold(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_hold(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj)
      return;
    if(!tar_ch) tar_ch = ch;
    spell_hold(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_hold(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in hold!");
    break;
  }
}



void cast_invisibility( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj) {
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE) )
	send_to_char("Nothing new seems to happen.\n\r", ch);
      else
	spell_invisibility(level, ch, 0, tar_obj);
    } else { /* tar_ch */
      if ( IS_AFFECTED(tar_ch, AFF_INVISIBLE) )
	send_to_char("Nothing new seems to happen.\n\r", ch);
      else
	spell_invisibility(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_POTION:
    if (!IS_AFFECTED(ch, AFF_INVISIBLE) )
      spell_invisibility(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {
      if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
	spell_invisibility(level, ch, 0, tar_obj);
    } else { /* tar_ch */
      if (!tar_ch) tar_ch = ch;

      if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
	spell_invisibility(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) {
      if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
	spell_invisibility(level, ch, 0, tar_obj);
    } else { /* tar_ch */
      if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
	spell_invisibility(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
	spell_invisibility(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in invisibility!");
    break;
  }
}


void cast_imp_invisibility( ubyte level, CHAR *ch, char *arg, int type,
			   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 30 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( IS_AFFECTED(tar_ch, AFF_IMINV) )
      send_to_char("Nothing new seems to happen.\n\r", ch);
    else
      spell_improved_invisibility(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      send_to_char("The forces of chaos negate your potion!\n\r",ch);
    else
      spell_improved_invisibility (level, ch, ch, 0);
    break;
  default :
    log_f("Wrong type called in imp-invisibility!");
    break;
  }
}

void cast_remove_improved_invis( ubyte level, CHAR *ch, char *arg, int type,
			   CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (!IS_AFFECTED(tar_ch, AFF_IMINV) )
      send_to_char("Nothing new seems to happen.\n\r", ch);
    else
      spell_remove_improved_invis(level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in imp-invisibility!");
    break;
  }
}



void cast_locate_object( ubyte level, CHAR *ch, char *arg, int type,
			CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 20 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_locate_object(level, ch, 0, tar_obj);
    break;
  default :
    log_f("Wrong type called in locate object!");
    break;
  }
}

void cast_paralyze( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_paralyze(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_paralyze(level,ch,ch,0);
    break;
  default :
    log_f("Wrong type called in paralyze!");
    break;
  }
}


void cast_poison( ubyte level, CHAR *ch, char *arg, int type,
		 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 10 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_poison(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_poison(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
	      spell_poison(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in poison!");
    break;
  }
}


void cast_protection_from_evil( ubyte level, CHAR *ch, char *arg, int type,
			       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 30 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_protection_from_evil(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_protection_from_evil(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj) return;
    if(!tar_ch) tar_ch = ch;
    spell_protection_from_evil(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_protection_from_evil(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in protection from evil!");
    break;
  }
}


void cast_protection_from_good( ubyte level, CHAR *ch, char *arg, int type,
			       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 30 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_protection_from_good(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_protection_from_good(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj) return;
    if(!tar_ch) tar_ch = ch;
    spell_protection_from_good(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_protection_from_good(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in protection from good!");
    break;
  }
}

void cast_remove_curse( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_remove_curse(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_remove_curse(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj) {
      spell_remove_curse(level, ch, 0, tar_obj);
      return;
    }
    if(!tar_ch) tar_ch = ch;
    spell_remove_curse(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_remove_curse(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in remove curse!");
    break;
  }
}

void cast_remove_paralysis( ubyte level, CHAR *ch, char *arg, int type,
			   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 35 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_remove_paralysis(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_remove_paralysis(level, ch, ch, 0);
    break;
  default :
    log_f("Wrong type called in remove paralysis!");
    break;
  }
}


void cast_remove_poison( ubyte level, CHAR *ch, char *arg, int type,
			CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_remove_poison(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_remove_poison(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_remove_poison(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in remove poison!");
    break;
  }
}



void cast_sanctuary( ubyte level, CHAR *ch, char *arg, int type,
		    CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_sanctuary(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_sanctuary(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj)
      return;
    if(!tar_ch) tar_ch = ch;
    spell_sanctuary(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_sanctuary(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in sanctuary!");
    break;
  }
}


void cast_sphere( ubyte level, CHAR *ch, char *arg, int type,
		 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_sphere(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_sphere(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj)
      return;
    if(!tar_ch) tar_ch = ch;
    spell_sphere(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_sphere(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in sphere!");
    break;
  }
}

void cast_invulnerability( ubyte level, CHAR *ch, char *arg, int type,
			  CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_invulnerability(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_invulnerability(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj)
      return;
    if(!tar_ch) tar_ch = ch;
    spell_invulnerability(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_invulnerability(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in invulnerability!");
    break;
  }
}

void cast_sleep( ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_sleep(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_sleep(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_sleep(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if(tar_obj) return;
    spell_sleep(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_sleep(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in sleep!");
    break;
  }
}


void cast_strength( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 10 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_strength(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_strength(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_strength(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_strength(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in strength!");
    break;
  }
}


void cast_ventriloquate( ubyte level, CHAR *ch, char *arg, int type,
			CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 5 */
{
  CHAR *tmp_ch;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char buf4[MAX_STRING_LENGTH];

  if (type != SPELL_TYPE_SPELL) {
    log_f("Attempt to ventriloquate by non-cast-spell.");
    return;
  }
  for(; *arg && (*arg == ' '); arg++);
  if (tar_obj) {
    sprintf(buf1, "The %s says '%s'\n\r", fname(OBJ_NAME(tar_obj)), arg);
    sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
	    fname(OBJ_NAME(tar_obj)), arg);
    sprintf(buf4, "%s makes it sound like the %s says '%s'.\n\r", GET_NAME(ch),
            fname(OBJ_NAME(tar_obj)), arg);
  }	else {
    sprintf(buf1, "%s says '%s'\n\r", PERS(tar_ch,tar_ch), arg);
    sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
	    GET_NAME(tar_ch), arg);
    sprintf(buf4, "%s makes it sound like %s says '%s'.\n\r", GET_NAME(ch),
            GET_NAME(tar_ch), arg);
  }

  sprintf(buf3, "Someone says, '%s'\n\r", arg);

  for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
       tmp_ch = tmp_ch->next_in_room) {

    if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
      if (!IS_MORTAL(tmp_ch)) {
        send_to_char(buf4, tmp_ch);
      } else {
      if ( saves_spell(tmp_ch, SAVING_SPELL,level) )
	send_to_char(buf2, tmp_ch);
      else
	send_to_char(buf1, tmp_ch);
      }
    } else {
      if (tmp_ch == tar_ch) {
        if (!IS_MORTAL(tmp_ch)) {
          sprintf(buf3, "%s makes you say '%s'\n\r", GET_NAME(ch), arg);
          send_to_char(buf3, tar_ch);
    } else {
	send_to_char(buf3, tar_ch);
    }
  }
}
  }
}



void cast_word_of_recall( ubyte level, CHAR *ch, char *arg, int type,
			 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  CHAR *temp;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_word_of_recall(level, ch, ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_word_of_recall(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_word_of_recall(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if(tar_obj) return;
    spell_word_of_recall(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF: /* Added temp - Ranger June 96 */
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ; tar_ch ; tar_ch = temp) {
      temp = tar_ch->next_in_room;
      if (tar_ch != ch) spell_word_of_recall(level,ch,tar_ch,0);
    }
    spell_word_of_recall(level,ch,ch,0);
    break;
  default :
    log_f("Wrong type called in word of recall!");
    break;
  }
}


void cast_total_recall( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 30 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_STAFF:
    spell_total_recall(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_total_recall(level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in word of recall!");
    break;
  }
}


void cast_summon( ubyte level, CHAR *ch, char *arg, int type,
		 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_summon(level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in summon!");
    break;
  }
}

void cast_locate_character( ubyte level, CHAR *ch, char *arg, int type,
			   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 20 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_locate_character(level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in locate character!");
    break;
  }
}


void cast_charm_person( ubyte level, CHAR *ch, char *arg, int type,
		       CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 50 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_charm_person(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(!tar_ch) return;
    spell_charm_person(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if ((tar_ch != ch) && (IS_NPC(tar_ch)))
	spell_charm_person(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in charm person!");
    break;
  }
}

void cast_sense_life( ubyte level, CHAR *ch, char *arg, int type,
		     CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 20 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_sense_life(level, ch, ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_sense_life(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[CHAR_REAL_ROOM(ch)].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_sense_life(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in sense life!");
    break;
  }
}


void cast_identify( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 25 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_identify(level,ch,tar_ch,tar_obj);
    break;

  case SPELL_TYPE_SCROLL:
    spell_identify(level, ch, tar_ch, tar_obj);
    break;
  default :
    log_f("Wrong type called in identify!");
    break;
  }
}


void cast_legend_lore( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_legend_lore(level,ch,tar_ch,tar_obj);
    break;

  case SPELL_TYPE_SCROLL:
    spell_legend_lore(level, ch, tar_ch, tar_obj);
    break;
  default :
    log_f("Wrong type called in legend lore!");
    break;
  }
}


void cast_recharge( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 30 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_recharge(level,ch,tar_ch,tar_obj);
    break;

  default :
    log_f("Wrong type called in recharge!");
    break;
  }
}

/* Dispel sanct - Ranger Sept 96 */
void cast_dispel_sanct( ubyte level, CHAR *ch, char *arg, int type,
		   CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_SPELL:
    spell_dispel_sanct(level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in dispel_sanct!");
    break;
  }
}
void cast_disenchant( ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_disenchant (level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_disenchant (level, ch, ch, 0);
    break;
  default :
    log_f("Wrong type called in Disenchant!");
    break;
  }
}


void cast_petrify( ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
{
  if(!IS_NPC(ch) && GET_LEVEL(ch)<LEVEL_WIZ) {
    send_to_char("Your level isn't high enough to cast this spell.\n\r",ch);
    return;
  }
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_petrify (level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in Petrify!");
    break;
  }
}


void cast_haste(ubyte level, CHAR *ch, char *arg, int type, CHAR *tar_ch, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_haste (level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_haste(level, ch, ch,0);
      break;
    default:
      log_f("Wrong type called in haste!");
      break;
  }
}

void cast_great_mana( ubyte level, CHAR *ch, char *arg, int type,
			CHAR *victim, OBJ *tar_obj )
/* Mana: 10 - God Spell Only*/
{
  if(GET_LEVEL(ch)<LEVEL_IMM) {
    send_to_char("This spell is for gods only!\n\r",ch);
    return;
  }
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_great_mana(level,ch,0,0);
    break;
  default :
    log_f("Wrong type called in great mana!");
    break;
  }
}

/*
void cast_spiritwrack( ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_STAFF:
  case SPELL_TYPE_SPELL:
    spell_spiritwrack (level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in spiritwrack!");
    break;
  }
}

void cast_spectral_blade( ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_spectral_blade (level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in Spectral Blade!");
    break;
  }
}


void cast_doppelganger( ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_doppelganger (level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in Doppelganger!");
    break;
  }
}


void cast_armageddon( ubyte level, CHAR *ch, char *arg, int type,
		      CHAR *victim, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    if(victim)
      spell_armageddon(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in armageddon!");
    break;
  }
}


void cast_tranquility( ubyte level, CHAR *ch, char *arg, int type,
                   CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_tranquility (level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in tranquility!");
    break;
  }
}


void cast_leech( ubyte level, CHAR *ch, char *arg, int type,
                   CHAR *tar_ch, OBJ *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_leech (level, ch, tar_ch, 0);
    break;
  default :
    log_f("Wrong type called in Leech!");
    break;
  }
}
*/
void cast_perceive(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj ) {
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_perceive(level, ch, victim, 0);
    break;
  default :
    log_f("Wrong type called in perceive!");
    break;
  }
}

void cast_shroud_existence (ubyte level, CHAR *ch, char *arg, int type,
			  CHAR *victim, OBJ *obj)
/* Mana: 50  Drains energy and transfers to caster */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
    spell_shroud_existence(level,ch,victim,0);
    break;
  default:
    log_f("cast_shroud_existence with wrong type!");
    break;
  }
}

void cast_quick(ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 100 allows to cast twice in 1 round */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_quick (level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_quick(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_quick(level,ch,tar_ch,0);
    break;
  default :
    log_f("Wrong type called in quick!");
    break;
  }
}

void cast_divine_intervention( ubyte level, CHAR *ch, char *arg, int type,
			      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 500 */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_DIVINE_INTERVENTION) ){
      send_to_char("Nothing seems to happen.\n\r", tar_ch);
      return;
    }
    spell_divine_intervention(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    break;
  case SPELL_TYPE_STAFF:
    break;
  default :
    log_f("Wrong type called in divine intervention!");
    break;
  }
}

void cast_rush(ubyte level, CHAR *ch, char *arg, int type,
		CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 100  */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_rush (level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    break;
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    break;
  default :
    log_f("Wrong type called in rush!");
    break;
  }
}

void cast_blood_lust(ubyte level, CHAR *ch, char *arg, int type,
                      CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 100  */
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_blood_lust (level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    break;
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    break;
  default :
    log_f("Wrong type called in blood lust!");
    break;
  }
}

void cast_mystic_swiftness(ubyte level, CHAR *ch, char *arg, int type,
                 CHAR *tar_ch, OBJ *tar_obj )
/* Mana: 100  */
{
 switch (type) {
  case SPELL_TYPE_SPELL:
     spell_mystic_swiftness (level, ch, tar_ch, 0);
     break;
  case SPELL_TYPE_POTION:
     break;
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
     break;
  default :
    log_f("Wrong type called in mystic_swiftness!");
    break;
  }
}

/* Mana: 100  */
void cast_wind_slash(ubyte level, CHAR *ch, char *arg, int type, CHAR *tar_ch, OBJ *tar_obj)
{
 switch (type) {
  case SPELL_TYPE_SPELL:
     spell_wind_slash(level, ch, tar_ch, 0);
     break;
  case SPELL_TYPE_POTION:
     break;
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
     break;
  default :
    log_f("Wrong type called in wind_slash!");
    break;
  }
}
