/*
$Author: ronin $
$Date: 2005/04/27 17:13:30 $
$Header: /home/ronin/cvs/ronin/spec.invasion.c,v 2.3 2005/04/27 17:13:30 ronin Exp $
$Id: spec.invasion.c,v 2.3 2005/04/27 17:13:30 ronin Exp $
$Name:  $
$Log: spec.invasion.c,v $
Revision 2.3  2005/04/27 17:13:30  ronin
Minor changes needed to compile on Slackware 10 for the new machine.

Revision 2.2  2005/01/21 14:55:29  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.1  2004/02/10 20:05:54  ronin
Check added to prevent two non-existent mob vnums from being used in
xykloqtium spec.

Revision 2.0.0.1  2004/02/05 16:10:50  ronin
Reinitialization of cvs archives

Revision 1.6  2003/01/03 17:37:22  ronin
Adding more fixes to tear_room, etc.

Revision 1.5  2003/01/02 16:07:22  ronin
Fixing xyklo and chaos specs.

Revision 1.4  2002/12/27 23:35:22  ronin
Fixing vexlok and ice wall disint specs.

Revision 1.3  2002/12/27 12:33:15  ronin
Fixing various specs, percentages, etc.

Revision 1.2  2002/12/24 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

/*
  spec.invasion.c - Specs for Demon Invasion, by Night

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 12/27/2002
*/

/* System Includes */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Ronin Includes */
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "cmd.h"
#include "comm.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "mob.spells.h"
#include "spec_assign.h"
#include "spells.h"
#include "act.h"
#include "utility.h"

/* Externs */
extern struct time_info_data time_info;
extern int CHAOSMODE;

/* Defines */
/* Rooms*/
#define TEMPLE          3001
#define MARKET_SQUARE   3014

#define ICE_WALL_ROOM   27760
#define STORAGE_ROOM    27778
#define LAVA_HAZARD     27787
#define SEARING_ROOM    27789
#define TEAR_ROOM       27793
#define MAZE_MIN        27794
#define MAZE_MAX        27799

/* Mobiles */
#define BASILISK        27702

#define IMP             27710
#define LEMURE          27711
#define OSYLUTH         27713
#define HELLCAT         27714
#define ELITE_GELUGON   27715
#define DEMON_SPAWN     27717

#define GATE_MIN        27713
#define GATE_MAX        27717

#define MYRDON          27720
#define SHADOWRAITH     27721
#define SHOMED          27722
#define TRYSTA          27723
#define VELXOK          27724
#define STRAM           27725
#define TOHIRI          27726
#define ANISTON         27727
#define LAW             27728
#define CHAOS           27729

#define XYKLOQTIUM      27730

/* Objects */
#define HORN            27723
#define MANTLE          27725
#define CIRCLET         27726
#define TACTICAL        27727
#define FROSTBRAND      27728
#define BLADED_SHIELD   27729

#define LANTERN         27730

#define LOCKPICKS       27740
#define DART            27741
#define BARBARIANS_RAGE 27742
#define FLUTE           27743
#define MAP             27744
#define LOCKET          27745
#define ORB             27746
#define BROKEN_ORB      27747
#define MANDATE         27748
#define SHARD           27749

#define ICE_WALL        27750
#define BROKEN_ICE_WALL 27751
#define VORTEX          27752


/* Mobile Specs */

/* Turn buffer to stone. */
int basilisk_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't spec if mob is not fighting. */
  if(!(vict = mob->specials.fighting)) return FALSE;

  switch(number(0, 39))
  {
    case(0): /* 2.5% Petrify vict. */
    {
      act("$n gazes deep into $N's eyes and turns $M to stone!", TRUE, mob, 0, vict, TO_NOTVICT);
      act("$n gazes deep into your eyes and turns you to stone!", FALSE, mob, 0, vict, TO_VICT);

      spell_petrify(GET_LEVEL(mob), mob, vict, 0);

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

/* Simple message-specs to make Imp's more interesting. */
int imp_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if no one is in the room. */
  if(count_mortals_room(mob, TRUE) < 1) return FALSE;

  switch(number(0, 9))
  {
    case(0):
    {
      act("$n cackles maniacally to $mself.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(1):
    {
      act("$n swears profusely in $s own demonic tongue.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(2):
    {
      act("$n flicks $s tail about, seeking another victim.", TRUE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

/* Simple message-specs to make Lemure's more interesting. */
int lemure_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if no one is in the room. */
  if(count_mortals_room(mob, TRUE) < 1) return FALSE;

  switch(number(0, 9))
  {
    case(0):
    {
      act("$n drools blood from $s gaping mouth.", TRUE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(1):
    {
      act("$n groans in the agony of $s twisted body.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

/* Tail-Sting Spec */
int osyluth_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if the mob is not fighting. */
  if(!mob->specials.fighting) return FALSE;

  /* Don't spec if there is no vict. */
  if(!(vict = get_random_victim_fighting(mob))) return FALSE;
  /* Don't spec if vict is !mortal. */
  if(!IS_MORTAL(vict)) return FALSE;

  switch(number(0, 19))
  {
    case(0):
    case(1): /* Sting the vict. */
    {
      act("$n stings $N with $s tail!", TRUE, mob, 0, vict, TO_NOTVICT);
      act("$n stings you with $s tail!", FALSE, mob, 0, vict, TO_VICT);
      damage(mob, vict, number(8, 24), TYPE_UNDEFINED, DAM_SKILL);

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

/* Simple message-specs to make Hellcat's more interesting. */
int hellcat_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if no one is in the room. */
  if(count_mortals_room(mob, TRUE) < 1) return FALSE;

  switch(number(0, 9))
  {
    case(0):
    {
      act("$n growls menacingly.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(1):
    {
      act("$n sniffs the air, seeking fresh prey.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

int elite_gelugon_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't spec if ch is !mortal or is an NPC. */
  if(!IS_MORTAL(ch) || IS_NPC(ch)) return FALSE;

  /* Block anyone trying to exit down and attack them. */
  if(cmd == CMD_DOWN)
  {
    act("$n blocks $N's way and attacks!", FALSE, mob, 0, ch, TO_NOTVICT);
    act("$n blocks your way and attacks!", TRUE, mob, 0, ch, TO_VICT);
    hit(mob, ch, TYPE_HIT);

    return TRUE;
  }

  return FALSE;
}

/* A Circle-Like Spec */
int myrdon_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  struct affected_type_5 af;

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if the mob is not fighting. */
  if(!(vict = mob->specials.fighting)) return FALSE;

  switch(number(0, 9))
  {
    case(0):
    case(1):
    case(2): /* Kick dirt in eyes for blindness. */
    {
      /* Don't spec if no vict. */
      if(!(vict = get_random_victim_fighting(mob))) return FALSE;
      /* Don't spec if vict is !mortal. */
      if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

      if(!affected_by_spell(vict, SPELL_BLINDNESS) && !IS_AFFECTED(vict, AFF_BLIND))
      {
        act("$n kicks dirt in $N's eyes, effectively blinding $M!", TRUE, mob, 0, vict, TO_NOTVICT);
        act("$n kicks dirt in your eyes, effectively blinding you!", FALSE, mob, 0, vict, TO_VICT);

        /* Give the vict blindness. */
        af.type = SPELL_BLINDNESS;
        af.location = APPLY_HITROLL;
        af.modifier = -4;
        af.duration = 1;
        af.bitvector = AFF_BLIND;
        af.bitvector2 = 0;

        affect_to_char(vict, &af);

        af.location = APPLY_AC;
        af.modifier = +40;

        affect_to_char(vict, &af);
      }
      else
      {
        act("$n kicks dirt in $N's eyes but $E is already blind!", TRUE, mob, 0, vict, TO_NOTVICT);
        act("$n kicks dirt in your eyes but you are already blind!", FALSE, mob, 0, vict, TO_VICT);
      }

      return FALSE;
    }
    case(3):
    case(4):
    case(5):
    case(6): /* Circle Type Spec (Hit) */
    {
      /* Don't spec if vict is !mortal. */
      if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

      act("$n circles behind $N's back and stabs $s weapon into $S back!", TRUE, mob, 0, vict,
TO_NOTVICT);
      act("$n circles behind you and stabs $s weapon into your back!", FALSE, mob, 0, vict, TO_VICT);
      damage(mob, vict, ((dice(mob->specials.damnodice, mob->specials.damsizedice) + mob->points.damroll)
* 4), TYPE_UNDEFINED, DAM_SKILL);

      return FALSE;
    }
    default:
    {
      /* Don't spec if vict is !mortal. */
      if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

      act("$n attempts to circle behind $N's back, but fails.", TRUE, mob, 0, vict, TO_NOTVICT);
      act("$n attempts to circle behind your back, but fails.", FALSE, mob, 0, vict, TO_VICT);

      return FALSE;
    }
  }

  return FALSE;
}

/* Ninja-Style Dual-Hit Spec (Sorta) */
int shadowraith_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;

  if(mob && cmd == MSG_VIOLENCE)
  {
    /* Don't spec if no vict. */
    if(!(vict = mob->specials.fighting)) return FALSE;
    /* Don't spec if vict is !mortal. */
    if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

    hit(mob, vict, TYPE_HIT);

    return FALSE;
  }

  /* Don't waste any more CPU time if it's not a MSG_MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if the mob is not fighting. */
  if(!(mob->specials.fighting)) return FALSE;

  switch(number(0, 9))
  {
    case(0):
    case(1): /* Throw ninja stars. */
    {
      vict = get_random_victim_fighting(mob);

      /* Don't spec if no vict. */
      if(!vict) return FALSE;
      /* Don't spec if vict is !mortal. */
      if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

      act("$n makes a series of quick movements and $N is pelted with throwing stars!", TRUE, mob, 0,
vict, TO_NOTVICT);
      act("$n makes a series of quick movements and you are pelted with throwing stars!", FALSE, mob, 0,
vict, TO_VICT);
      damage(mob, vict, number(80, 180), TYPE_UNDEFINED, DAM_SKILL);

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

/* Berserk Spec */
int shomed_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;

  /* 10% Chance to calm down. */
  if(IS_SET(mob->specials.affected_by, AFF_DUAL) && chance(10))
  {
    REMOVE_BIT(mob->specials.affected_by, AFF_DUAL);

    act("$n falls out of $s berserk frenzy and calms down.", TRUE, mob, 0, 0, TO_ROOM);

    return FALSE;
  }

  /* Don't waste any more CPU time if the mob is not fighting. */
  if(!mob->specials.fighting) return FALSE;

  /* 30% chance to berserk. */
  if(!IS_SET(mob->specials.affected_by, AFF_DUAL) && chance(30))
  {
    SET_BIT(mob->specials.affected_by, AFF_DUAL);

    act("$n works $mself into a berserk frenzy!", TRUE, mob, 0, 0, TO_ROOM);

    return FALSE;
  }

  return FALSE;
}

/* Bard-Like Specs */
int trysta_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  struct affected_type_5 af;

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if the mob is not fighting. */
  if(!mob->specials.fighting) return FALSE;

  switch(number(0, 19))
  {
    case(0): /* Send all mortals in room to Temple of Midgaard. */
    {
      act("$n sings 'Ilu lme ier, hini lme ier...'", FALSE, mob, 0, 0, TO_ROOM);

      for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(vict == mob) continue;
        if(IS_NPC(vict)) continue;
        if(!IS_MORTAL(vict)) continue;

        act("$n disappears!", TRUE, vict, 0, 0, TO_ROOM);
        act("You disappear!", FALSE, vict, 0, 0, TO_CHAR);
        char_from_room(vict);
        char_to_room(vict, real_room(TEMPLE));
        act("$n appears in the room suddenly!", TRUE, vict, 0, 0, TO_ROOM);
        do_look(vict, "", CMD_LOOK);
      }

      return FALSE;
    }
    case(1):
    case(2): /* Give all mortals in room the bad War Chant affect. */
    {
      act("$n sings 'Quen nwalme nin...'", FALSE, mob, 0, 0, TO_ROOM);

      for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(vict == mob) continue;
        if(!IS_MORTAL(vict) && !IS_NPC(vict)) continue;

        if(!affected_by_spell(vict, SPELL_WARCHANT))
        {
          af.type = SPELL_WARCHANT;
          af.duration = 5;
          af.modifier = -4;
          af.location = APPLY_HITROLL;
          af.bitvector = 0;
          af.bitvector2 = 0;
          affect_to_char(vict, &af);

          send_to_char("You feel slightly weaker!\r\n", vict);
        }
      }

      return FALSE;
    }
    case(3):
    case(4):
    case(5): /* Give all mortals in the room the Blindness affect. */
    {
      act("$n sings 'Lumbo mi olos...'", FALSE, mob, 0, 0, TO_ROOM);
      act("Flashes of light fill the room!", TRUE, mob, 0, 0, TO_ROOM);

      for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(vict == mob) continue;
        if(!IS_MORTAL(vict) && !IS_NPC(vict)) continue;

        spell_blindness(GET_LEVEL(mob), mob, vict, 0);
      }

      return FALSE;
    }
    case(6):
    case(7):
    case(8):
    case(9): /* Damage all mortals in the room as if by Color Spray. */
    {
      act("$n sings 'Nuruhuine...'", FALSE, mob, 0, 0, TO_ROOM);
      act("Streams of color spray from $n's finger-tips!", TRUE, mob, 0, 0, TO_ROOM);

      for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(vict == mob) continue;
        if(!IS_MORTAL(vict) && !IS_NPC(vict)) continue;

        damage(mob, vict, (GET_LEVEL(mob) / 2) + number(35, 100), TYPE_UNDEFINED, DAM_MAGICAL);
      }

      return FALSE;
    }
    default: /* Backflip */
    {
      /* Don't spec if no vict. */
      if(!(vict = mob->specials.fighting)) return FALSE;

      act("$n flips into the air and lands behind $N!", TRUE, mob, 0, vict, TO_NOTVICT);
      act("$n flips into the air and lands behind you!", FALSE, mob, 0, vict, TO_VICT);
      hit(mob, vict, TYPE_HIT);

      return FALSE;
    }
  }

  return FALSE;
}

/* Velxok has a mana-shield that keeps him alive forever until hit by DART. */
int velxok_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Make him attack first person into the room. */
  if(cmd == MSG_ENTER)
  {
    /* Don't spec if no ch. */
    if(!ch) return FALSE;
    /* Don't spec if already fighting. */
    if(mob->specials.fighting) return FALSE;
    /* Don't spec if vict is !mortal. */
    if(!IS_MORTAL(ch)) return FALSE;

    do_say(mob, "Intruder!", CMD_SAY);

    hit(mob, ch, TYPE_HIT);

    return FALSE;
  }

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;

  /* Refill mana. */
  GET_MANA(mob) = GET_MAX_MANA(mob);

  /* Set hps to 4000 if not hit by dart yet. */
  if(!IS_SET(mob->specials.act, ACT_MEMORY) && (GET_HIT(mob) <= 4000))  GET_HIT(mob) = 4000;

  /* Don't spec if not fighting. */
  if(!mob->specials.fighting) return FALSE;

  /* Taunt messages based on his power. */
  if(!IS_SET(mob->specials.act, ACT_MEMORY))
  {
    switch(number(0, 9))
    {
      case(0):
      {
        do_say(mob, "Hahaha, you fools.  You cannot defeat one so powerful as me!", CMD_SAY);

        return FALSE;
      }
      case(1):
      {
        do_say(mob, "My magic can sustain me indefinitely!", CMD_SAY);

        return FALSE;
      }
      case(2):
      {
        do_say(mob, "Pathetic.  Watch me rain fire upon your corpses!", CMD_SAY);

        return FALSE;
      }
      default:
      {
        return FALSE;
      }
    }

    return FALSE;
  }
  else
  {
    switch(number(0, 9))
    {
      case(0):
      {
        do_say(mob, "What?!  My powers are waning!", CMD_SAY);

        return FALSE;
      }
      case(1):
      {
        do_say(mob, "This cannot be!  I will not let you defeat me!", CMD_SAY);

        return FALSE;
      }
      case(2):
      {
        do_say(mob, "If you strike me down, I shall become more powerful than you can possibly imagine!",
CMD_SAY);

        return FALSE;
      }
      default:
      {
        return FALSE;
      }
    }

    return FALSE;
  }

  return FALSE;
}

/* Commando Spell Specs (For casting in a NO_MAGIC Room) */
int stram_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  struct affected_type_5 af;

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if the mob is not fighting. */
  if(!(vict = get_random_victim_fighting(mob))) return FALSE;
  /* Don't spec if vict is !mortal. */
  if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

  switch(number(0, 19))
  {
    case(0):
    case(1): /* Incendiary Cloud */
    {
      act("$n makes an arcane gesture and $N is engulfed in a cloud of flames!", TRUE, mob, 0, vict,
TO_NOTVICT);
      act("$n makes an arcane gesture and you are engulfed in a cloud of flames!", FALSE, mob, 0, vict,
TO_VICT);
      damage(mob, vict, 600, TYPE_UNDEFINED, DAM_FIRE);
      if(!affected_by_spell(vict, SPELL_INCENDIARY_CLOUD))
      {
        af.type = SPELL_INCENDIARY_CLOUD;
        af.duration = 8;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = 0;
        af.bitvector2 = 0;
        affect_to_char(vict, &af);
      }

      return FALSE;
    }
    case(2):
    case(3):
    case(4): /* Frost Bolt */
    {
      act("$n makes an arcane gesture and sends bolts of ice at $N!", TRUE, mob, 0, vict, TO_NOTVICT);
      act("$n makes an arcane gesture and sends bolts of ice at you!", FALSE, mob, 0, vict, TO_VICT);
      damage(mob, vict, 350, TYPE_UNDEFINED, DAM_COLD);

      return FALSE;
    }
    case(5):
    case(6):
    case(7):
    case(8):
    case(9): /* Vamp-Touch */
    {
      act("$n reaches out and touches $N, draining some of $S life away!", TRUE, mob, 0, vict,
TO_NOTVICT);
      act("$n reaches out and touches you, draining some of your life away!", FALSE, mob, 0, vict,
TO_VICT);
      damage(mob, vict, (GET_LEVEL(mob) * 8), TYPE_UNDEFINED, DAM_MAGICAL);
      GET_HIT(mob) += (GET_LEVEL(mob) * 8);

      return FALSE;
    }
    case(10):
    case(11):
    case(12):
    case(13):
    case(14):
    case(15): /* Iceball */
    {
      act("$n makes an arcane gesture and sends a ball of ice at $N!", TRUE, mob, 0, vict, TO_NOTVICT);
      act("$n makes an arcane gesture and sends a ball of ice at you!", FALSE, mob, 0, vict, TO_VICT);
      damage(mob, vict, ((GET_LEVEL(mob) / 2) + number(100, 160)), TYPE_UNDEFINED, DAM_COLD);

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

/* Tohiri miras every round unless affected by TRANQUILITY affect (from FLUTE). */
int tohiri_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;

  /* Refill mana. */
  GET_MANA(mob) = GET_MAX_MANA(mob);

  /* Mira if flute has not been played. */
  if(!IS_SET(mob->specials.act, ACT_MEMORY))
  {
    if(GET_HIT(mob) < GET_MAX_HIT(mob))
    {
      act("$n prays to $s gods for a miracle and it is granted!", TRUE, mob, 0, 0, TO_ROOM);
      GET_HIT(mob) = GET_MAX_HIT(mob);

      return FALSE;
    }
    else
    {
      return FALSE;
    }
  }
  else
  {
    REMOVE_BIT(mob->specials.act, ACT_MEMORY);

    act("$n's look of tranquility fades as $e comes to $s senses.", TRUE, mob, 0, 0, TO_ROOM);

    return FALSE;
  }

  return FALSE;
}

/* Blocks characters from going north and throws players into a hazard during fight. */
int aniston_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  OBJ *obj;
  char buf[MAX_STRING_LENGTH];

  if(cmd == MSG_MOBACT)
  {
    /* Spell-shield until 2/3 dead. */
    if(GET_HIT(mob) > (GET_MAX_HIT(mob) / 3))
    {
      if(!IS_SET(mob->specials.act, ACT_SHIELD)) SET_BIT(mob->specials.act, ACT_SHIELD);
    }
    else
    {
      if(IS_SET(mob->specials.act, ACT_SHIELD)) REMOVE_BIT(mob->specials.act, ACT_SHIELD);
    }

    if(mob->specials.fighting && !IS_SET(mob->specials.act, ACT_MEMORY))
    {
      do_say(mob, "I will not fight you!  Go away!", CMD_SAY);
      act("$n recites a scroll of total-recall!", TRUE, mob, 0, 0, TO_ROOM);

      for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(IS_NPC(vict)) continue;

        act("$n disappears!", TRUE, vict, 0, 0, TO_ROOM);
        act("You disappear!", FALSE, vict, 0, 0, TO_CHAR);
        char_from_room(vict);
        char_to_room(vict, real_room(TEMPLE));
        act("$n appears in the room suddenly!", TRUE, vict, 0, 0, TO_ROOM);
        do_look(vict, "", CMD_LOOK);
      }

      return FALSE;
    }

    /* Don't waste any more CPU time if the mob is not fighting. */
    if(!(vict = get_random_victim_fighting(mob))) return FALSE;
    /* Don't throw the tank. */
    if(vict == mob->specials.fighting) return FALSE;
    /* Don't spec if vict is !mortal. */
    if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

    switch(number(0, 19))
    {
      case(0):
      case(1): /* Throw vict into an adjacent hazard. */
      {
        act("$n charges $N and pushes $M off the narrow bridge!", TRUE, mob, 0, vict, TO_NOTVICT);
        act("$n charges you and pushes you off the narrow bridge!", FALSE, mob, 0, vict, TO_VICT);
        stop_fighting(vict);
        if(!IS_AFFECTED(vict, AFF_PARALYSIS))
        {
          GET_POS(vict) = POSITION_STANDING;
          if(chance(50))
            sprintf(buf, "west");
          else
            sprintf(buf, "east");
          command_interpreter(vict, buf);
        }
        else
        {
          char_from_room(vict);
          char_to_room(vict, real_room(LAVA_HAZARD));
          do_look(vict, "", CMD_LOOK);

          send_to_char("Your paralyzation prevents your escape from a fiery death!\r\n", vict);
          damage(vict, vict, 10000, TYPE_UNDEFINED, DAM_NO_BLOCK);
        }

        return FALSE;
      }
      default:
      {
        return FALSE;
      }
    }

    return FALSE;
  }

  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't spec if ch is !mortal or if ch is an NPC. */
  if(IS_NPC(ch)) return FALSE;

  if(cmd == CMD_GIVE)
  {
    arg = one_argument(arg, buf);

    /* Return if no object. */
    if(!*buf) return FALSE;

    /* Proceed with normal do_give() no object matching input. */
    if(!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) return FALSE;

    /* Proceed with normal do_give() if tmp is not LOCKET. */
    if(V_OBJ(obj) != LOCKET) return FALSE;

    one_argument(arg, buf);

    /* Return if no target. */
    if(!*buf) return FALSE;

    /* Proceed with normal do_give() if target is not ANISTON. */
    if(!isname(buf, MOB_NAME(mob))) return FALSE;

    extract_obj(obj);

    if(!IS_SET(mob->specials.act, ACT_MEMORY))
    {
      SET_BIT(mob->specials.act, ACT_MEMORY);
      SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }

    do_say(mob, "You killed my brother... prepare to DIE!", CMD_SAY);
    act("$n drops the locket which tumbles from the bridge into the magma below.", FALSE, mob, 0, 0,
TO_ROOM);

    hit(mob, ch, TYPE_HIT);

    return TRUE;
  }

  /* Don't spec if ch is !mortal. */
  if(!IS_MORTAL(ch) || IS_NPC(ch)) return FALSE;

  /* Attack first person to enter room if LOCKET has been given to mob. */
  if(cmd == MSG_ENTER && IS_SET(mob->specials.act, ACT_MEMORY))
  {
    /* Don't spec if no ch. */
    if(!ch) return FALSE;
    /* Don't spec if already fighting. */
    if(mob->specials.fighting) return FALSE;

    do_say(mob, "My brother is dead and soon you shall join him!", CMD_SAY);

    hit(mob, ch, TYPE_HIT);

    return FALSE;
  }

  /* Do not fight unless affected by AFFECT_LOCKET. */
  if(cmd == CMD_KILL || cmd == CMD_HIT)
  {
    arg = one_argument(arg, buf);

    /* Return if no buf. */
    if(!*buf) return FALSE;

    if(isname(buf, MOB_NAME(mob)) && !IS_SET(mob->specials.act, ACT_MEMORY))
    {
      do_say(mob, "My duty is clear, I cannot be distracted!", CMD_SAY);

      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }

  /* Block anyone from going north. */
  if(cmd == CMD_NORTH)
  {
    act("$n pushes $N back!", TRUE, mob, 0, ch, TO_NOTVICT);
    act("$n pushes you back!", FALSE, mob, 0, ch, TO_VICT);
    do_say(mob, "I will not let you interfere!  This is not a fight for mere mortals!", CMD_SAY);

    return TRUE;
  }

  return FALSE;
}

/* Casts fury when someone enters the room or when it runs out and is fighting. */
int law_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Fury if fighting and not affected by it. */
  if(cmd == MSG_TICK && mob->specials.fighting)
  {
    if(!affected_by_spell(mob, SPELL_FURY))
    {
      spell_fury(GET_LEVEL(mob), mob, mob, 0);
    }

    return FALSE;
  }

  if(cmd == MSG_MOBACT)
  {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);
    return FALSE;
  }

  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't spec if ch is !mortal. */
  if(!IS_MORTAL(ch) && !IS_NPC(ch)) return FALSE;
  /* Only spec when someone enters the room. */
  if(cmd != MSG_ENTER) return FALSE;

  if(!affected_by_spell(mob, SPELL_FURY))
  {
    spell_fury(GET_LEVEL(mob), mob, mob, 0);

    act("$n starts snarling and fuming with rage!", FALSE, mob, 0, 0, TO_ROOM);
  }

  return FALSE;
}

/* Backstabs first person into the room if not already in a fight and casts fury/rage.  Also has a
spirit-levy spec. */
int chaos_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  OBJ *tmp, *next_content, *tmp2, *next_content2;
  struct affected_type_5 af;

  /* Spirit Levy any corpses in the room (pseudo levy). */
  if(cmd == MSG_MOBACT)
  {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);

    for(tmp = world[CHAR_REAL_ROOM(mob)].contents; tmp; tmp = next_content)
    {
      next_content = tmp->next_content;

      if(GET_ITEM_TYPE(tmp) == ITEM_CONTAINER && tmp->obj_flags.value[3] && tmp->obj_flags.cost !=
PC_STATUE && tmp->obj_flags.cost != NPC_STATUE)
      {
        act("$n drains energy from $p.", FALSE, mob, tmp, 0, TO_ROOM);

        for(tmp2 = tmp->contains; tmp2; tmp2 = next_content2)
        {
          next_content2 = tmp2->next_content;

          obj_from_obj(tmp2);
          obj_to_room(tmp2, CHAR_REAL_ROOM(mob));
        }

        extract_obj(tmp);

        GET_HIT(mob) += number(1000, 4000);
      }
    }

    return FALSE;
  }

  /* Rage if fighting and not affected by it. */
  if(cmd == MSG_TICK && mob->specials.fighting)
  {
    if(!affected_by_spell(mob, SPELL_BLACKMANTLE))
    { 
      af.type = SPELL_BLACKMANTLE;
      af.duration = 10;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = 0;
      af.bitvector2 = 0;
      affect_to_char(mob, &af);
  
      act("$n becomes enveloped in a swirling cloud of darkness!", TRUE, mob, 0, 0, TO_ROOM);
    }

    if(!affected_by_spell(mob, SPELL_RAGE))
    {
      af.type = SPELL_RAGE;
      af.duration = 3;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = 0;
      af.bitvector2 = 0;
      affect_to_char(mob, &af);

      act("$n snarls in rage and hatred of all living things!", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  /* Only spec when someone enters the room. */
  if(cmd != MSG_ENTER) return FALSE;
  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't spec if ch is !mortal. */
  if(!IS_MORTAL(ch)) return FALSE;
  /* Don't spec if already in a fight. */
  if(mob->specials.fighting) return FALSE;

  if(!affected_by_spell(mob, SPELL_BLACKMANTLE))
  {
    af.type = SPELL_BLACKMANTLE;
    af.duration = 10;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(mob, &af);

    act("$n becomes enveloped in a swirling cloud of darkness!", TRUE, mob, 0, 0, TO_ROOM);
  }

  if(!affected_by_spell(mob, SPELL_RAGE))
  {
    af.type = SPELL_RAGE;
    af.duration = 3;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(mob, &af);

    act("$n snarls in rage and hatred of all living things!", FALSE, mob, 0, 0, TO_ROOM);
  }

  act("$n plunges $s weapon deep into $N's back!", FALSE, mob, 0, ch, TO_NOTVICT);
  act("As you enter the room, $n plunges $s weapon deep into your back!", FALSE, mob, 0, ch, TO_VICT);
  damage(mob, ch, number(2000, 4000), TYPE_UNDEFINED, DAM_NO_BLOCK);

  return FALSE;
}

/* Insta-Kill people if not carrying BROKEN_ORB.  Throw people, gate in ally and taunt people. */
int xykloqtium_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *tmp_mob;
  OBJ *obj;
  int throw_room = 0,mob_vnum;
  char buf[MAX_STRING_LENGTH];

  if(cmd == CMD_GIVE)
  {
    arg = one_argument(arg, buf);

    /* Return if no object. */
    if(!*buf) return FALSE;

    /* Proceed with normal do_give() no object matching input. */
    if(!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) return FALSE;

    /* Proceed with normal do_give() if tmp is not ORB. */
    if(V_OBJ(obj) != ORB) return FALSE;

    one_argument(arg, buf);

    /* Return if no target. */
    if(!*buf) return FALSE;

    /* Proceed with normal do_give() if target is not XYKLOQTUIM. */
    if(!isname(buf, MOB_NAME(mob))) return FALSE;

    do_say(mob, "Arrgh!  I will not take that foul thing!", CMD_SAY);

    act("$n drops $p upon the round.", TRUE, ch, obj, 0, TO_ROOM);
    act("You drop $p upon the ground.", FALSE, ch, obj, 0, TO_CHAR);

    obj_from_char(obj);
    obj_to_room(obj, CHAR_REAL_ROOM(ch));

    return TRUE;
  }

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;

  /* Refill mana. */
  GET_MANA(mob) = GET_MAX_MANA(mob);

  /* Don't waste any more CPU time if the mob is not fighting. */
  if(!mob->specials.fighting) return FALSE;

  /* Don't spec if there is no vict. */
  if(!(vict = get_random_victim_fighting(mob))) return FALSE;
  /* Don't spec if vict is !mortal. */
  if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

  if(!is_carrying_obj(mob, BROKEN_ORB) && chance(15))
  {
    /* Don't spec if ch is the tank. */
    if(vict == mob->specials.fighting) return FALSE;

    act("$n points $s finger at $N and utters a purely evil word.", TRUE, mob, 0, vict, TO_NOTVICT);
    act("$n points $s finger at you and utters a purely evil word.", FALSE, mob, 0, vict, TO_VICT);

    damage(mob, vict, 10000, TYPE_UNDEFINED, DAM_NO_BLOCK);
  }

  switch(number(0, 19))
  {
    case(0):
    case(1): /* Throw a victim into a random maze room and load a DEMON_SPAWN to hit them. */
    {
      /* Don't spec if ch is the tank. */
      if(vict == mob->specials.fighting) return FALSE;

      /* Find a room other than the one the mob is currently in. */
      do
      {
        throw_room = number(MAZE_MIN, MAZE_MAX);
      } while(throw_room == V_ROOM(mob));

      act("$n reaches out, grabs $N, and throws $M into the swirling mists!", TRUE, mob, 0, vict,
TO_NOTVICT);
      act("$n reaches out, grabs you, and throws you into the swirling mists!", FALSE, mob, 0, vict,
TO_VICT);

      damage(mob, vict, number(80, 160), TYPE_UNDEFINED, DAM_NO_BLOCK);

      /* Throw the character to the throw_room. */
      char_from_room(vict);
      char_to_room(vict, real_room(throw_room));

      tmp_mob = read_mobile(DEMON_SPAWN, VIRTUAL);
      char_to_room(tmp_mob, CHAR_REAL_ROOM(vict));

      do_look(vict, "", CMD_LOOK);

      hit(tmp_mob, vict, TYPE_HIT);

      return FALSE;
    }
    case(2): /* Load a random demon to fight a random mortal in the room. */
    {
      act("$n utters a word of command and gates in a demonic ally!", FALSE, mob, 0, 0, TO_ROOM);

      mob_vnum = number(GATE_MIN, GATE_MAX);
      /* Mobs 27712 and 27716 don't exist, so replace them with DEMON_SPAWN. */
      if((mob_vnum == 27712) || (mob_vnum == 27716))
      {
        mob_vnum = DEMON_SPAWN;
      }
      tmp_mob = read_mobile(mob_vnum, VIRTUAL);
      char_to_room(tmp_mob, CHAR_REAL_ROOM(mob));

      hit(tmp_mob, vict, TYPE_HIT);

      return FALSE;
    }
    case(3):
    case(4): /* Taunt the people in the room. */
    {
      do_say(mob, "Your bones shall fill my belly after I break your soul!", CMD_SAY);

      return FALSE;
    }
    case(5):
    case(6): /* Taunt the people in the room. */
    {
      do_say(mob, "Hahaha... You fools!  You cannot defeat me!", CMD_SAY);

      return FALSE;
    }
    case(7):
    case(8): /* Taunt the people in the room. */
    {
      do_say(mob, "Is that the best you can do?  Pitiful mortals!", CMD_SAY);

      return FALSE;
    }
    case(9):
    case(10): /* Taunt the people in the room. */
    {
      do_say(mob, "I will squeeze the life from your world with my bare hands!", CMD_SAY);

      return FALSE;
    }
    case(11):
    case(12): /* Taunt the people in the room. */
    {
      do_say(mob, "You shall all die a most painfully slow death!", CMD_SAY);

      return FALSE;
    }
    case(13):
    case(14): /* Taunt the people in the room. */
    {
      do_say(mob, "Bow down and serve the true masters of the multiverse!", CMD_SAY);

      return FALSE;
    }
    default:
    {
      act("$n snorts smoke and flames from $s nostrils!", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
  }

  return FALSE;
}

/* Great Miracle, bless, vitality, cure critic in combat.  (Low chances)
   Note: This spec uses fall-through case's. */
int horn_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner, *vict, *next_vict;

  /* Don't waste any more CPU time if it's not a TICK */
  if(cmd != MSG_TICK) return FALSE;
  /* Don't waste any more CPU time if the object is not equipped. */
  if(!(owner = obj->equipped_by)) return FALSE;
  /* Don't waste any more CPU time if ch is not fighting. */
  if(!owner->specials.fighting) return FALSE;

  switch(number(0, 199))
  {
    case(0):  /* .5% Miracle to room. */
    {
      if(GET_MANA(owner) < 100) return FALSE;

      GET_MANA(owner) -= 100;

      act("The tranquil music of the sea fills the air!", FALSE, owner, 0, 0, TO_ROOM);
      act("The tranquil music of the sea fills the air!", FALSE, owner, 0, 0, TO_CHAR);

      for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(!IS_MORTAL(vict)) continue;
        if(IS_NPC(vict)) continue;

        spell_miracle(GET_LEVEL(owner), vict, vict, 0);
      }

      return FALSE;
    }
    case(1):
    case(2): /* 1% Heal to room. */
    {
      if(GET_MANA(owner) < 50) return FALSE;

      GET_MANA(owner) -= 50;

      act("The tranquil music of the sea fills the air!", FALSE, owner, 0, 0, TO_ROOM);
      act("The tranquil music of the sea fills the air!", FALSE, owner, 0, 0, TO_CHAR);

      for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(!IS_MORTAL(vict)) continue;
        if(IS_NPC(vict)) continue;

        spell_heal(GET_LEVEL(owner), vict, vict, 0);
      }

      return FALSE;
    }
    case(3):
    case(4):
    case(5): /* 1.5% Cure Critic to room. */
    {
      if(GET_MANA(owner) < 25) return FALSE;

      GET_MANA(owner) -= 25;

      act("The tranquil music of the sea fills the air!", FALSE, owner, 0, 0, TO_ROOM);
      act("The tranquil music of the sea fills the air!", FALSE, owner, 0, 0, TO_CHAR);

      for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(!IS_MORTAL(vict)) continue;
        if(IS_NPC(vict)) continue;

        spell_cure_critic(GET_LEVEL(owner), vict, vict, 0);
      }

      return FALSE;
    }
    case(6):
    case(7):
    case(8):
    case(9):
    case(10): /* 2.5% Bless to room. */
    {
      if(GET_MANA(owner) < 5) return FALSE;

      GET_MANA(owner) -= 5;

      act("The tranquil music of the sea fills the air!", FALSE, owner, 0, 0, TO_ROOM);
      act("The tranquil music of the sea fills the air!", FALSE, owner, 0, 0, TO_CHAR);

      for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(!IS_MORTAL(vict)) continue;
        if(IS_NPC(vict)) continue;

        spell_bless(GET_LEVEL(owner), vict, vict, 0);
      }

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

/* Change damage on MANTLE randomly.
   Note: This spec uses fall-through case's. */
int mantle_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner;

  /* Don't spec if obj is not equipped. */
  if(!obj->equipped_by) return FALSE;
  /* Don't spec if ch is not the wearer. */
  if(!(owner = obj->equipped_by)) return FALSE;

  if(cmd == MSG_BEING_REMOVED)
  {
    unequip_char(owner, WEAR_ABOUT);

    /* Location 1 should be set as damroll. */
    obj->affected[1].modifier = 2;

    equip_char(owner, obj, WEAR_ABOUT);

    return FALSE;
  }

  /* Don't waste any more CPU time if it's not a TICK */
  if(cmd != MSG_TICK) return FALSE;

  switch(number(0, 19))
  {
    case(0): /* 5% 1 damage. */
    {
      unequip_char(owner, WEAR_ABOUT);

      /* Location 1 should be set as damroll. */
      obj->affected[1].modifier = 1;

      equip_char(owner, obj, WEAR_ABOUT);

      return FALSE;
    }
    case(1):
    case(2):
    case(3):
    case(4):
    case(5):
    case(6):
    case(7):
    case(8):
    case(9):
    case(10):
    case(11):
    case(12):
    case(13):
    case(14):
    case(15):
    case(16): /* 80% 2 damage. */
    {
      unequip_char(owner, WEAR_ABOUT);

      /* Location 1 should be set as damroll. */
      obj->affected[1].modifier = 2;

      equip_char(owner, obj, WEAR_ABOUT);

      return FALSE;
    }
    default: /* 15% 3 damage. */
    {
      unequip_char(owner, WEAR_ABOUT);

      /* Location 1 should be set as damroll. */
      obj->affected[1].modifier = 3;

      equip_char(owner, obj, WEAR_ABOUT);

      return FALSE;
    }
  }

  return FALSE;
}

/* Spec MAX_MANA bonus, amount is based on current alignment. */
int circlet_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner;
  int amount = 0;

  /* Don't waste any more CPU time if it's not a TICK */
  if(cmd != MSG_TICK) return FALSE;
  /* Don't waste any more CPU time if the object is not equipped. */
  if(!(owner = obj->equipped_by)) return FALSE;

  /* Item is Good-Only, but check to make sure spec doesn't happen to non-good people. */
  if(!IS_GOOD(owner)) return FALSE;

  if(GET_ALIGNMENT(owner) < 500)
  {
    amount = 10;
  }
  else if(GET_ALIGNMENT(owner) < 650)
  {
    amount = 15;
  }
  else if(GET_ALIGNMENT(owner) < 800)
  {
    amount = 20;
  }
  else if(GET_ALIGNMENT(owner) < 950)
  {
    amount = 25;
  }
  else
  {
    amount = 30;
  }

  unequip_char(owner, WEAR_HEAD);

  /* Location 1 should be set as damroll. */
  obj->affected[1].modifier = amount;

  equip_char(owner, obj, WEAR_HEAD);

  return FALSE;
}

/* Special spin-kick spec. */
int tactical_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;

  /* Don't waste any more CPU time if cmd is not SPIN. */
  if(cmd != CMD_SPIN) return FALSE;
  /* Don't spec if obj is not worn. */
  if(!(obj->equipped_by)) return FALSE;
  /* Don't spec if ch is not the wearer. */
  if(ch != obj->equipped_by) return FALSE;

  if(!chance(25)) return FALSE;

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && !CHAOSMODE)
  {
    send_to_char("Behave yourself here please!\r\n", ch);

    return TRUE;
  }

  if(ch->specials.riding)
  {
    send_to_char("Dismount first.\r\n", ch);

    return TRUE;
  }

  act("$n whirls about the room kicking everyone in sight!", TRUE, ch, 0, 0, TO_ROOM);
  act("You whirl about the room kicking everyone in sight!", FALSE, ch, 0, 0, TO_CHAR);

  for(vict = world[CHAR_REAL_ROOM(ch)].people; vict; vict = next_vict)
  {
    next_vict = vict->next_in_room;

    if(!IS_MORTAL(vict) && !IS_NPC(vict)) continue;

    if(ch != vict && IS_NPC(vict) && vict->specials.rider != ch)
    {
      act("You have been kicked by $n.", FALSE, ch, 0, vict, TO_VICT);
      damage(ch, vict, (MIN((GET_LEVEL(ch) * 2), 60) * 2), TYPE_UNDEFINED, DAM_SKILL);
    }
    if(ch != vict && !IS_NPC(vict) && IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
    {
      act("You have been kicked by $n.", FALSE, ch, 0, vict, TO_VICT);
      damage(ch, vict, (MIN((GET_LEVEL(ch) * 2), 60) * 2), TYPE_UNDEFINED, DAM_SKILL);
    }
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 4);

  return TRUE;
}

/* Chill-touches on wield and sometimes does damage + chill touch on kill/hit. */
int frostbrand_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *wielder;
  OBJ *tmp;
  char buf[MAX_STRING_LENGTH];
  struct affected_type_5 af;

  /* Requires 20 Natural Str to wield, Spec short chill-touch to ch when frostbrand is wielded. */
  if(cmd == CMD_WIELD)
  {
    one_argument(arg, buf);

    if(!(tmp = get_obj_in_list_vis(ch, buf, ch->carrying))) return FALSE;

    if(tmp != obj) return FALSE;

    if(ch->abilities.str < 20)
    {
      send_to_char("You are too weak to wield such a mighty weapon.\r\n", ch);

      return TRUE;
    }

    if(!affected_by_spell(ch, SPELL_CHILL_TOUCH))
    {
      af.type = SPELL_CHILL_TOUCH;
      af.duration = 1;
      af.modifier = -2;
      af.location = APPLY_STR;
      af.bitvector = 0;
      af.bitvector2 = 0;
      affect_to_char(ch, &af);
    }

    return FALSE;
  }

  /* Don't waste any more CPU time if the object is not equipped. */
  if(!(wielder = obj->equipped_by)) return FALSE;

  /* Return if command is not KILL. */
  if(wielder && cmd == CMD_KILL) {

  one_argument(arg, buf);

  /* Return if no target. */
  if(!*buf) return FALSE;

  /* Return if target is invalid. */
    if(!(vict = get_char_room_vis(wielder, buf))) return FALSE;

  /* Don't spec if vict is !mortal. */
  if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;
    /* Don't spec if wielder is already fighting. */
    if(wielder->specials.fighting) return FALSE;
  /* Don't spec if vict is already fighting. */
  if(vict->specials.fighting) return FALSE;
    if(ch!=wielder || wielder!=obj->equipped_by) return FALSE; /* Linerfix 102803 */

  /* Don't spec other players unless it is a chaotic room or CHAOSMODE. */
    if(!IS_NPC(vict) && !IS_SET(world[CHAR_REAL_ROOM(wielder)].room_flags, CHAOTIC) && !CHAOSMODE) return FALSE;

  switch(number(0, 19))
  {
    case(0):
    case(1): /* Cold damage spec on kill. */
    {
        act("$n's Frostbrand drains the warmth from $N as $e strikes $M!", TRUE, wielder, 0, vict, TO_NOTVICT);
        act("$n's Frostbrand drains the warmth from you as $e strikes you!", FALSE, wielder, 0, vict, TO_VICT);
        act("Your Frostbrand drains the warmth from $N as you strike $M!", FALSE, wielder, 0, vict, TO_CHAR);
        damage(wielder, vict, MAX(((GET_LEVEL(wielder) - GET_LEVEL(vict)) + 100), 100), TYPE_UNDEFINED, DAM_COLD);
        WAIT_STATE(wielder, PULSE_VIOLENCE);

      /* Don't spec Chill Touch to high-level mobs. */
      if(GET_LEVEL(vict) > 45) return FALSE;
      /* 20% of the time it specs Chill Touch. */
      if(!chance(20)) return TRUE;

      if(!affected_by_spell(vict, SPELL_CHILL_TOUCH))
      {
        af.type = SPELL_CHILL_TOUCH;
        af.duration = 1;
        af.modifier = -2;
        af.location = APPLY_STR;
        af.bitvector = 0;
        af.bitvector2 = 0;
        affect_to_char(vict, &af);
      }

      return TRUE;
    }
    default:
    {
      return FALSE;
    }
  }
    return FALSE;
  }
  return FALSE;
}

/* Damages wearer on wear, remove and sometimes in combat.
   Note: This spec uses fall-through case's. */
int bladed_shield_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner, *vict;
  OBJ *tmp;
  char buf[MAX_STRING_LENGTH];

  if(cmd == CMD_WEAR)
  {
    if(ch != obj->carried_by) return FALSE;

    one_argument(arg, buf);

    if(!(tmp = get_obj_in_list_vis(ch, buf, ch->carrying))) return FALSE;

    if(tmp != obj) return FALSE;

    damage(ch, ch, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK);

    return FALSE;
  }

  if(cmd == CMD_REMOVE)
  {
    if(ch != obj->equipped_by) return FALSE;

    one_argument(arg, buf);

    if(!isname(buf,OBJ_NAME(obj))) return FALSE;

    if(!(tmp = ch->equipment[WEAR_SHIELD])) return FALSE;

    if(tmp != obj) return FALSE;

    damage(ch, ch, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK);

    return FALSE;
  }

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't spec if it's not equipped.*/
  if(!(owner = obj->equipped_by)) return FALSE;
  /* Don't waste any more CPU time if ch is not fighting. */
  if(!(vict = owner->specials.fighting)) return FALSE;

  /* Don't spec if vict is !mortal. */
  if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

  switch(number(0, 99))
  {
    case(0): /* 1% Shield cuts wearer. */
    {
      act("$n winces in pain as $s shield slices into $s wrist!", TRUE, owner, 0, 0, TO_ROOM);
      act("You wince in pain as your shield slices into your wrist!", FALSE, owner, 0, 0, TO_CHAR);
      damage(owner, owner, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK);

      return FALSE;
    }
    case(1):
    case(2):
    case(3):
    case(4):
    case(5): /* 5% Shield cuts vict. */
    {
      act("$n lashes out with $s shield and slices $N with it!", TRUE, owner, 0, vict, TO_NOTVICT);
      act("$n lashes out with $s shield and slices you with it!", FALSE, owner, 0, vict, TO_VICT);
      act("You lash out with your shield and slice $N with it!", FALSE, owner, 0, vict, TO_CHAR);
      damage(owner, vict, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK);

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

/* Lantern sets hps to 1 on grab and sometimes does hellfire to room during combat. */
int lantern_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner, *vict, *next_vict;
  OBJ *tmp;
  char buf[MAX_STRING_LENGTH];

  /* Set ch's hps to 1 when lantern is held. */
  if(cmd == CMD_GRAB)
  {
    if(ch != obj->carried_by) return FALSE;

    one_argument(arg, buf);

    if(!(tmp = get_obj_in_list_vis(ch, buf, ch->carrying))) return FALSE;

    if(tmp != obj) return FALSE;

    GET_HIT(ch) = 1;

    return FALSE;
  }

  /* Don't waste any more CPU time if it's not a TICK */
  if(cmd != MSG_TICK) return FALSE;
  /* Don't waste any more CPU time if the object is not equipped. */
  if(!(owner = obj->equipped_by)) return FALSE;
  /* Don't waste any more CPU time if ch is not fighting. */
  if(!(owner->specials.fighting)) return FALSE;

  switch(number(0, 19))
  {
    case(0): /* Hellfire-Like Spec to the wearer's room. */
    {
      act("A shower of burning sparks rains from $n's lantern!", FALSE, owner, 0, 0, TO_ROOM);
      act("A shower of burning sparks rains from your lantern!", FALSE, owner, 0, 0, TO_CHAR);

      for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        if(!IS_MORTAL(vict) && !IS_NPC(vict)) continue;
        if(IS_EVIL(vict)) continue;

        act("$N is burned the rain of sparks!", TRUE, owner, 0, vict, TO_NOTVICT);
        act("You are burned by the rain of sparks!", FALSE, owner, 0, vict, TO_VICT);
        act("$N is burned by the rain of sparks!", TRUE, owner, 0, vict, TO_CHAR);
        if(IS_NPC(vict))
        {
          damage(ch, vict, 100, TYPE_UNDEFINED, DAM_FIRE);
        }
        else if(IS_MORTAL(vict))
        {
          damage(vict, vict, 100, TYPE_UNDEFINED, DAM_FIRE);
        }
      }

      return FALSE;
    }
    default:
    {
      return FALSE;
    }
  }

  return FALSE;
}

/* 'Picks' a normally un-pickable door. */
int lockpicks_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int door = -1;

  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't spec if ch is not the one carrying the LOCKPICKS. */
  if(ch != obj->carried_by) return FALSE;
  /* Don't spec if ch is an NPC. */
  if(IS_NPC(ch)) return FALSE;
  /* Don't waste any more CPU time if cmd is not CMD_PICK. */
  if(cmd != CMD_PICK) return FALSE;

  /* Don't waste any more CPU time if ch is not in the STORAGE_ROOM. */
  if(V_ROOM(ch) != STORAGE_ROOM)
  {
    send_to_char("The lockpicks don't seem to be intended to pick anything here.\r\n", ch);

    return TRUE;
  }

  arg = one_argument(arg, buf1);
  one_argument(arg, buf2);

  /* Return if no target. */
  if(!*buf1) return FALSE;
  /* Return if no direction. */
  if(!*buf2) return FALSE;

  string_to_lower(buf1);
  string_to_lower(buf2);

  if((door = find_door(ch, buf1, buf2)) < 0) return FALSE;

  /* Set the state of the door to unlocked. */
  REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);

  act("$n manages to unlock the mechanism with the Magical Lockpicks.", TRUE, ch, 0, 0, TO_ROOM);
  act("The magic within the lockpicks fades away and they crumble to dust.", TRUE, ch, 0, 0, TO_ROOM);
  act("You manage to unlock the mechanism with the Magical Lockpicks.", FALSE, ch, 0, 0, TO_CHAR);
  act("The magic within the lockpicks fades away and they crumble to dust.", FALSE, ch, 0, 0, TO_CHAR);

  extract_obj(obj);

  return TRUE;
}

/* Used to kill VELXOK when near death and relying on mana shield. */
int dart_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  OBJ *tmp_obj;
  char buf[MAX_STRING_LENGTH];
  int bits;

  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't spec if ch is an NPC. */
  if(IS_NPC(ch)) return FALSE;
  /* Don't waste any more CPU time if ch is not throwing obj. */
  if(cmd != CMD_THROW) return FALSE;

  arg = one_argument(arg, buf);

  /* Return if no target. */
  if(!*buf) return FALSE;

  /* Return if it's not the dart. */
  if(!isname(buf, OBJ_NAME(obj))) return FALSE;

  if(!(ch->equipment[HOLD]) || V_OBJ(ch->equipment[HOLD]) != DART)
  {
    if(ch != obj->carried_by) return FALSE;

    send_to_char("You do not hold the dart in your hand.\r\n", ch);

    return TRUE;
  }

  one_argument(arg, buf);

  /* Find the target. */
  if(!ch->specials.fighting)
  {
    bits = generic_find(arg, FIND_CHAR_ROOM, ch, &vict, &tmp_obj);
  }
  else
  {
    bits = FIND_CHAR_ROOM;
    vict = ch->specials.fighting;
    }

  if(bits)
  {
    if(bits == FIND_CHAR_ROOM)
    {
      if(IS_NPC(vict) && V_MOB(vict) == VELXOK)
      {
        /* Only wounds Velxok if he's at <= 1/2 max_hps. */
        if(GET_HIT(vict) > (GET_MAX_HIT(vict) / 2))
        {
          act("$N is yet too powerful!", TRUE, ch, 0, vict, TO_CHAR);

          return TRUE;
        }

        act("$n throws a small dart at $N which strikes $M in the chest and dissolves!", TRUE, ch, 0,
vict, TO_NOTVICT);
        act("$n throws a small dart at you which strikes you in the chest and dissolves!", FALSE, ch, 0,
vict, TO_VICT);
        act("You throw the Assassin's Dart at $N which strikes $M in the chest and dissolves!", FALSE,
ch, 0, vict, TO_CHAR);

        unequip_char(ch, HOLD);
        extract_obj(obj);

        if(!IS_SET(vict->specials.act, ACT_MEMORY)) SET_BIT(vict->specials.act, ACT_MEMORY);
        if(IS_SET(vict->specials.act, ACT_SHIELD)) REMOVE_BIT(vict->specials.act, ACT_SHIELD);

        act("$n shudders in agony and clutches at his chest!", TRUE, vict, 0, 0, TO_ROOM);

        return TRUE;
      }
      else
      {
        send_to_char("The dart was meant for someone much more powerful.\r\n", ch);

        return TRUE;
      }
    }
    else
    {
      send_to_char("You can't do that.\r\n", ch);

      return TRUE;
    }
  }
  else
  {
    send_to_char("Throw the dart at whom?\r\n", ch);

    return TRUE;
  }

  return TRUE;
}

/* Used to break ICE_WALL. */
int barbarians_rage_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  OBJ *block_on, *block_off;
  char buf[MAX_STRING_LENGTH];
  int wall_on, wall_off;

  wall_on = real_object(ICE_WALL);
  wall_off = real_object(BROKEN_ICE_WALL);

  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't waste any more CPU time if cmd is not CMD_UNKNOWN. */
  if(cmd != CMD_UNKNOWN || !AWAKE(ch)) return FALSE;

  arg = one_argument(arg, buf);

  /* Return if no command. */
  if(!*buf) return FALSE;

  string_to_lower(buf);

  /* Return if command is not 'break'. */
  if(!is_abbrev(buf, "break")) return FALSE;

  if(!(ch->equipment[WIELD]) || V_OBJ(ch->equipment[WIELD]) != BARBARIANS_RAGE)
  {
    if(ch != obj->carried_by) return FALSE;

    send_to_char("You must wield the axe first.\r\n", ch);

    return TRUE;
  }

  one_argument(arg, buf);

  /* Return if no target. */
  if(!*arg)
  {
    send_to_char("Break what?\r\n", ch);

    return TRUE;
  }

  block_on = get_obj_in_list_num(wall_on, world[CHAR_REAL_ROOM(ch)].contents);

  if(!block_on)
  {
    send_to_char("There is nothing here to break effectively.\r\n", ch);

    return TRUE;
  }

  if(!isname(buf, OBJ_NAME(block_on)))
  {
    send_to_char("Breaking that would serve no purpose.\r\n", ch);

    return TRUE;
  }

  act("$n slams $s axe into the wall of ice!", FALSE, ch, 0, 0, TO_ROOM);
  act("Both the wall and the axe shatter to pieces!", FALSE, ch, 0, 0, TO_ROOM);
  act("You slam your axe into the wall of ice!", FALSE, ch, 0, 0, TO_CHAR);
  act("Both the wall and the axe shatter to pieces!", FALSE, ch, 0, 0, TO_CHAR);

  extract_obj(block_on);

  block_off = read_object(wall_off, REAL);
  obj_to_room(block_off, CHAR_REAL_ROOM(ch));

  unequip_char(ch, WIELD);
  extract_obj(obj);

  return TRUE;
}

int flute_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *tmp, *next_in_room;
  char buf[MAX_STRING_LENGTH];

  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't waste any more CPU time if cmd is not CMD_UNKNOWN. */
  if(cmd != CMD_UNKNOWN || !AWAKE(ch)) return FALSE;

  arg = one_argument(arg, buf);

  /* Return if no command. */
  if(!*buf) return FALSE;

  string_to_lower(buf);

  /* Return if command is not 'play'. */
  if(!is_abbrev(buf, "play")) return FALSE;

  if(!(ch->equipment[HOLD])|| V_OBJ(ch->equipment[HOLD]) != FLUTE)
  {
    send_to_char("You must hold the flute first.\r\n", ch);

    return TRUE;
  }

  one_argument(arg, buf);

  /* Return if no target. */
  if(!*buf)
  {
    send_to_char("Play what?\r\n", ch);

    return TRUE;
  }

  /* Return if it's not the FLUTE that is being played. */
  if(!isname(buf, OBJ_NAME(ch->equipment[HOLD])))
  {
    send_to_char("You don't know how to play that.\r\n", ch);

    return TRUE;
  }

  act("A tranquil melody from $n's flute fills the room for a brief moment...", FALSE, ch, 0, 0,
TO_ROOM);
  act("A tranquil melody from your flute fills the room for a brief moment...", FALSE, ch, 0, 0,
TO_CHAR);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

  for(tmp = world[CHAR_REAL_ROOM(ch)].people; tmp; tmp = next_in_room)
  {
    next_in_room = tmp->next_in_room;

    if(IS_NPC(tmp) && V_MOB(tmp) == TOHIRI)
    {
      if(!IS_SET(tmp->specials.act, ACT_MEMORY))
      {
        SET_BIT(tmp->specials.act, ACT_MEMORY);

        act("$n seems to forget about $s worries for the moment as $e is filled with a sense of tranquility.", TRUE, tmp, 0, 0, TO_ROOM);
      }

      return TRUE;
    }
  }

  return TRUE;
}

/* Orb of Banishment breaks Xykloqtium's powers, making him killable. */
int orb_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  OBJ *tmp_obj, *obj2;
  char buf[MAX_STRING_LENGTH];
  int bits;

  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't waste any more CPU time if ch is not throwing obj. */
  if(cmd != CMD_THROW || !AWAKE(ch)) return FALSE;

  arg = one_argument(arg, buf);

  /* Return if no target. */
  if(!*buf) return FALSE;

  /* Return if it's not the orb. */
  if(!isname(buf, OBJ_NAME(obj))) return FALSE;

  if(!(ch->equipment[HOLD]) || V_OBJ(ch->equipment[HOLD]) != ORB)
  {
    if(ch != obj->carried_by) return FALSE;

    send_to_char("You do not hold the orb in your hand.\r\n", ch);

    return TRUE;
  }

  one_argument(arg, buf);

  /* Find the target. */
  if(!ch->specials.fighting)
  {
    bits = generic_find(arg, FIND_CHAR_ROOM, ch, &vict, &tmp_obj);
  }
  else
  {
    bits = FIND_CHAR_ROOM;
    vict = ch->specials.fighting;
    }

  if(bits)
  {
    if(bits == FIND_CHAR_ROOM)
    {
      if(IS_NPC(vict) && V_MOB(vict) == XYKLOQTIUM)
      {
        act("The Orb of Banishment flies from $n's hand and strikes $N, disrupting $S demonic powers!",
FALSE, ch, 0, vict, TO_NOTVICT);
        act("The Orb of Banishment flies from your hand and strikes $N, disrupting $S demonic powers!",
FALSE, ch, 0, vict, TO_CHAR);

        unequip_char(ch, HOLD);
        extract_obj(obj);

        obj2 = read_object(BROKEN_ORB, VIRTUAL);
        obj_to_char(obj2, vict);

        act("$n snarls in rage at having been weakened!", TRUE, vict, 0, 0, TO_ROOM);

        return TRUE;
      }
      else
      {
        act("$n throws The Orb of Banishment at $N, but it seems ineffective.", TRUE, ch, 0, vict,
TO_NOTVICT);
        act("You throw The Orb of Banishment at $N, but it seems ineffective.", FALSE, ch, 0, vict,
TO_CHAR);

        unequip_char(ch, HOLD);
        obj_to_char(obj, vict);

        return TRUE;
      }
    }
    else
    {
      send_to_char("You can't do that.\r\n", ch);

      return TRUE;
    }
  }
  else
  {
    send_to_char("Throw the orb at whom?\r\n", ch);

    return TRUE;
  }

  return TRUE;
}

/* ICE_WALL blocks passage north unless broken with BARBARIANS_RAGE */
int ice_wall_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't spec if ch is !mortal. */
  if(!IS_MORTAL(ch) && !IS_NPC(ch)) return FALSE;
  /* Don't waste any more CPU time if cmd is not CMD_ENTER. */
  if(cmd != CMD_NORTH || !AWAKE(ch)) return FALSE;

  send_to_char("A wall of ice and stone blocks your way.\r\n", ch);

  return TRUE;
}

/* Sends people to MS if they enter the vortex. */
int vortex_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];

  /* Don't spec if there is no ch. */
  if(!ch) return FALSE;
  /* Don't spec if ch is an NPC. */
  if(IS_NPC(ch)) return FALSE;
  /* Don't waste any more CPU time if cmd is not CMD_ENTER. */
  if(cmd != CMD_ENTER || !AWAKE(ch)) return FALSE;

  one_argument(arg, buf);

  /* Return if no argument. */
  if(!*buf) return FALSE;

  /* Return if not correct keyword. */
  if(!isname(buf, OBJ_NAME(obj))) return FALSE;

  /* Return if riding. */
  if(ch->specials.riding)
  {
    send_to_char("Dismount first.\r\n", ch);

    return TRUE;
  }

  act("$n enters the vortex and disappears!", TRUE, ch, 0, 0, TO_ROOM);
  act("You enter the vortex and disappear!", FALSE, ch, 0, 0, TO_CHAR);
  char_from_room(ch);
  char_to_room(ch, real_room(MARKET_SQUARE));
  act("$n appears in the room suddenly!", TRUE, ch, 0, 0, TO_ROOM);
  do_look(ch, "", CMD_LOOK);

  return TRUE;
}

/* Remove BROKEN_ICE_WALL on zone reset. */
int ice_wall_room_spec(int room, CHAR *ch, int cmd, char *arg)
{
  OBJ *obj;
  int wall = 0;

  /* Don't waste any more CPU time if it's not a ZONE_RESET. */
  if(cmd != MSG_ZONE_RESET) return FALSE;

  if(!(wall = real_object(BROKEN_ICE_WALL))) return FALSE;

  if(!(obj = get_obj_in_list_num(wall, world[room].contents))) return FALSE;

  extract_obj(obj);

  return FALSE;
}

/* Searing-orb spec to a room on MOBACT. */
int searing_room_spec(int room, CHAR *ch, int cmd, char *arg)
{
  CHAR *tmp, *next_in_room;
  OBJ *paper, *next_content;

  /* Don't waste any more CPU time if it's not a TICK. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if nobody is in the room. */
  if(!(ch = world[real_room(SEARING_ROOM)].people)) return FALSE;

  act("Searing flames rise from the fiery depths below super-heating the room!", FALSE, ch, 0, 0,
TO_ROOM);
  act("Searing flames rise from the fiery depths below super-heating the room!", FALSE, ch, 0, 0,
TO_CHAR);

  for(paper = world[real_room(SEARING_ROOM)].contents; paper; paper = next_content)
  {
    next_content = paper->next_content;

    if((GET_ITEM_TYPE(paper) == ITEM_SCROLL || GET_ITEM_TYPE(paper) == ITEM_RECIPE) && number(0, 5))
    {
      act("$p burns in bright and hot flames...", FALSE, ch, paper, 0, TO_ROOM);
      act("$p burns in bright and hot flames...", FALSE, ch, paper, 0, TO_CHAR);
      extract_obj(paper);
    }
    if(GET_ITEM_TYPE(paper) == ITEM_POTION && number(0, 5))
    {
      act("$p boils up in steam...", FALSE, ch, paper, 0, TO_ROOM);
      act("$p boils up in steam...", FALSE, ch, paper, 0, TO_CHAR);
      extract_obj(paper);
    }
  }

  for(tmp = world[real_room(SEARING_ROOM)].people; tmp; tmp = next_in_room)
  {
    next_in_room = tmp->next_in_room;

    if(!IS_MORTAL(tmp)) continue;

    for(paper = tmp->carrying; paper; paper = next_content)
    {
      next_content = paper->next_content;

      if((GET_ITEM_TYPE(paper) == ITEM_SCROLL || GET_ITEM_TYPE(paper) == ITEM_RECIPE) && !number(0, 5))
      {
        act("$p burns in bright and hot flames...", FALSE, ch, paper, 0, TO_ROOM);
        act("$p burns in bright and hot flames...", FALSE, ch, paper, 0, TO_CHAR);
        extract_obj(paper);
      }
      if (GET_ITEM_TYPE(paper) == ITEM_POTION && !number(0,5))
      {
        act("$p boils up in steam...", FALSE, ch, paper, 0, TO_ROOM);
        act("$p boils up in steam...", FALSE, ch, paper, 0, TO_CHAR);
        extract_obj(paper);
      }
    }

    if(IS_MORTAL(tmp) && !IS_NPC(tmp))
    {
      damage(tmp, tmp, number(25, 50), SPELL_SEARING_ORB, DAM_FIRE);
    }
  }

  return FALSE;
}

/* Transports people to the Abyss if they have the required objects. */
int tear_room_spec(int room, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  OBJ *tmp, *next_carrying;
  int has_items = FALSE, dest_room = 0;

  if(cmd == MSG_ZONE_RESET)
  {
    world[room].dir_option[NORTH]->to_room_r = real_room(0);
    world[room].dir_option[EAST]->to_room_r = real_room(0);
    world[room].dir_option[SOUTH]->to_room_r = real_room(0);
    world[room].dir_option[UP]->to_room_r = real_room(0);
    world[room].dir_option[DOWN]->to_room_r = real_room(0);
 
    send_to_room("The breach into reality seals itself closed!\r\n", real_room(TEAR_ROOM));
 
    return FALSE;
  }

  /* Don't spec if there's no ch. */
  if(!ch) return FALSE;
  /* Don't spec if ch is an NPC. */
  if(IS_NPC(ch)) return FALSE;

  /* Block the use of items that might transport players out of the room. */
  if(cmd==CMD_QUIT || cmd==CMD_USE || cmd==CMD_QUAFF || cmd==CMD_RECITE || cmd==CMD_DONATE)
  {
    /* Don't spec if ch is !mortal. */
    if(!IS_MORTAL(ch) && !IS_NPC(ch)) return FALSE;
 
    send_to_char("That action has no meaning here...", ch);
 
    return TRUE;
  }

  /* Don't waste any more CPU time if cmd is not MSG_ENTER. */
  if(cmd != MSG_ENTER) return FALSE;

  if(!is_carrying_obj(ch, MAP) || !is_carrying_obj(ch, MANDATE) || !is_carrying_obj(ch, SHARD))
  {
    has_items = FALSE;
  }
  else
  {
    has_items = TRUE;
  }

  /* Extract MAP, MANDATE and SHARD */
  for(tmp = ch->carrying; tmp; tmp = next_carrying)
  {
    next_carrying = tmp->next_content;

    if(tmp && (V_OBJ(tmp) == MAP || V_OBJ(tmp) == MANDATE || V_OBJ(tmp) == SHARD))
    {
      act("$p dissolves!", TRUE, ch, tmp, 0, TO_ROOM);
      act("$p dissolves!", TRUE, ch, tmp, 0, TO_CHAR);

      extract_obj(tmp);
    }
  }

  if(!has_items) return FALSE;

  dest_room = number(MAZE_MIN, MAZE_MAX);

  switch(number(0, 5))
  {
    case(0):
    {
      world[room].dir_option[NORTH]->to_room_r = real_room(dest_room);
      break;
    }
    case(1):
    {
      world[room].dir_option[EAST]->to_room_r = real_room(dest_room);
      break;
    }
    case(2):
    {
      world[room].dir_option[SOUTH]->to_room_r = real_room(dest_room);
      break;
    }
    case(3):
    {
      world[room].dir_option[WEST]->to_room_r = real_room(dest_room);
      break;
    }
    case(4):
    {
      world[room].dir_option[UP]->to_room_r = real_room(dest_room);
      break;
    }
    case(5):
    {
      world[room].dir_option[DOWN]->to_room_r = real_room(dest_room);
      break;
    }
    default:
    {
      /* Do nothing. */
      break;
    }
  }

  send_to_room("A breach into reality has been created!\r\n", real_room(TEAR_ROOM));

  for(vict = world[room].people; vict; vict = next_vict)
  {
    next_vict = vict->next_in_room;

    if(IS_NPC(vict)) continue;

    act("$n disappears!", TRUE, vict, 0, 0, TO_ROOM);
    act("Reality distorts as you travel instantly to another plane of existence!", FALSE, vict, 0, 0,
TO_CHAR);
    char_from_room(vict);
    char_to_room(vict, real_room(dest_room));
    act("$n appears in the room suddenly!", TRUE, vict, 0, 0, TO_ROOM);
    do_look(vict, "", CMD_LOOK);
  }

  return FALSE;
}


/* Assign Specs */
void assign_invasion(void)
{
  assign_mob(BASILISK, basilisk_spec);

  assign_mob(IMP, imp_spec);
  assign_mob(LEMURE, lemure_spec);
  assign_mob(OSYLUTH, osyluth_spec);
  assign_mob(HELLCAT, hellcat_spec);
  assign_mob(ELITE_GELUGON, elite_gelugon_spec);

  assign_mob(MYRDON, myrdon_spec);
  assign_mob(SHADOWRAITH, shadowraith_spec);
  assign_mob(SHOMED, shomed_spec);
  assign_mob(TRYSTA, trysta_spec);
  assign_mob(VELXOK, velxok_spec);
  assign_mob(TOHIRI, tohiri_spec);
  assign_mob(STRAM, stram_spec);
  assign_mob(ANISTON, aniston_spec);
  assign_mob(LAW, law_spec);
  assign_mob(CHAOS, chaos_spec);

  assign_mob(XYKLOQTIUM, xykloqtium_spec);

  assign_obj(HORN, horn_spec);
  assign_obj(MANTLE, mantle_spec);
  assign_obj(CIRCLET, circlet_spec);
  assign_obj(TACTICAL, tactical_spec);
  assign_obj(FROSTBRAND, frostbrand_spec);
  assign_obj(BLADED_SHIELD, bladed_shield_spec);

  assign_obj(LANTERN, lantern_spec);

  assign_obj(LOCKPICKS, lockpicks_spec);
  assign_obj(DART, dart_spec);
  assign_obj(BARBARIANS_RAGE, barbarians_rage_spec);
  assign_obj(FLUTE, flute_spec);
  assign_obj(ORB, orb_spec);

  assign_obj(ICE_WALL, ice_wall_spec);
  assign_obj(VORTEX, vortex_spec);

  assign_room(ICE_WALL_ROOM, ice_wall_room_spec);
  assign_room(SEARING_ROOM, searing_room_spec);
  assign_room(TEAR_ROOM, tear_room_spec);
}

