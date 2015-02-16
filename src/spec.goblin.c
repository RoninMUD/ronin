/*
  spec.goblin.c - Specs for Goblin Kingdom and Marsh Cave, by BobaNight

  Written by Alan K. Miles for RoninMUD

  Last Modification Date: 03/03/2004
*/

/*
$Author: ronin $
$Date: 2004/08/18 13:15:36 $
$Header: /home/ronin/cvs/ronin/spec.goblin.c,v 2.2 2004/08/18 13:15:36 ronin Exp $
$Id: spec.goblin.c,v 2.2 2004/08/18 13:15:36 ronin Exp $
$Name:  $
$Log: spec.goblin.c,v $
Revision 2.2  2004/08/18 13:15:36  ronin
Re-addition of header lines.

Revision 2.1  2004/08/11 16:10:38  ronin
Additional specs for expanded area

Revision 2.0.0.1  2004/02/05 16:10:38  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

/* Includes */

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


/* Defines */

/* Mobiles */
#define DWELLER        4512
#define GRUG           4513
#define CRAB           4514
#define HERDER         4515
#define DRAGON         4516
#define PIXIE          4517
#define ZOMBIE         4518

/* Objects */
#define WONDER         4507
#define PROD           4511

/* Rooms */
#define SILT_START     4543
#define SILT_FINISH    4548
#define DRAGON_LAIR    4541
#define UPPER_TUNNEL   4540
#define ISLAND_START   4557
#define ISLAND_FINISH  4560
#define CRACKED_TUNNEL 4566


/* Mobile Specs */

/* Dwellers randomly enter the rock wall and emerge in a different room.  Plus some flavor text.*/
int dweller_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;

  if(chance(10))
  {
    /* Don't waste any more CPU time if no one is in the zone. */
    if(count_mortals_zone(mob, TRUE) < 1) return FALSE;
    /* Don't spec if mob is fighting. */
    if(mob->specials.fighting) return FALSE;

    act("$n melds into the rock wall and dissapears!", TRUE, mob, 0, 0, TO_ROOM);

    char_from_room(mob);
    char_to_room(mob, real_room(number(SILT_START, SILT_FINISH)));

    act("$n suddenly emerges from the rock wall!", TRUE, mob, 0, 0, TO_ROOM);

    return FALSE;
  }

  /* Don't waste any more CPU time if no one is in the room. */
  if(count_mortals_room(mob, TRUE) < 1) return FALSE;

  /* 10% chance each case. */
  switch(number(0, 9))
  {
    case(0):
    {
      act("$n coughs up a small puff of dust.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(1):
    {
      act("$n pounds the floor with a small stone club.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    default:
    {
      /* Do nothing. */

      return FALSE;
    }
  }

  return FALSE;
}

/* Simple message spec to make Grug more interesting. */
int grug_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if no one is in the room. */
  if(count_mortals_room(mob, TRUE) < 1) return FALSE;
  /* Don't spec if mob is fighting. */
  if(mob->specials.fighting) return FALSE;

  /* 10% chance each case. */
  switch(number(0, 9))
  {
    case(0):
    {
      act("$n mumbles to $mself about the aerodynamics of a flailing goblin.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(1):
    {
      act("$n adjusts $s loincloth and proceeds to scratch $mself...", TRUE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(2):
    {
      act("$n scribbles some complex design on the ground, curses, then blacks out for a moment.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    default:
    {
      /* Do nothing. */

      return FALSE;
    }
  }

  return FALSE;
}

/* Simple message spec to make Crabs more interesting. */
int crab_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if no one is in the room. */
  if(count_mortals_room(mob, TRUE) < 1) return FALSE;

  /* 10% chance each case. */
  switch(number(0, 9))
  {
    case(0):
    {
      act("$n clacks $s pinchers together in a loud snapping display.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(1):
    {
      /* Don't spec if mob is fighting. */
      if(mob->specials.fighting) return FALSE;

      /* Don't spec if mob is not in in a room between ISLAND_START and ISLAND_FINISH. */
      if((V_ROOM(mob) < ISLAND_START) || (V_ROOM(mob) > ISLAND_FINISH)) return FALSE;

      act("$n snips the base of a giant mushroom, catches the stalk, and begins devouring it.", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    default:
    {
      /* Do nothing. */

      return FALSE;
    }
  }

  return FALSE;
}

/* Herder uses his prod if equipped.  Plus some flavor text. */
int herder_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *next, *temp, *vict;
  OBJ *obj;

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if no one is in the room. */
  if(count_mortals_room(mob, TRUE) < 1) return FALSE;

  if(chance(10))
  {
    /* Don't spec if mob is fighting. */
    if(mob->specials.fighting) return FALSE;

    /* Check room for crab; if one is here, zap it. */
    for(temp = world[CHAR_REAL_ROOM(mob)].people; temp; temp = next)
    {
      next = temp->next_in_room;

      if(!IS_NPC(temp)) continue;
      if(V_MOB(temp) != CRAB) continue;

      act("$n notices $N is out of line and gives $M a quick zap with $s prod!", FALSE, mob, 0, temp, TO_ROOM);
      act("$n clacks $s scuttles away in pain and falls into line.", FALSE, temp, 0, 0, TO_ROOM);

      return FALSE;
    }

    return FALSE;
  }

  /* Don't spec if mob is not fighting. */
  if(!(mob->specials.fighting)) return FALSE;

  if(chance(10))
  {
    /* check for prod, if equipped, zap victim */
    if((!(mob->equipment[HOLD])) || (V_OBJ(mob->equipment[HOLD]) != PROD)) return FALSE;

    obj = mob->equipment[HOLD];

    /* Don't spec if there is no vict. */
    if(!(vict = get_random_victim_fighting(mob))) return FALSE;
    /* Don't spec if vict is !mortal. */
    if(!IS_MORTAL(vict) && !IS_NPC(vict)) return FALSE;

    act("$n points $p at $N and utters a mystical word!", FALSE, mob, obj, vict, TO_NOTVICT);
    act("$n points $p at you and utters a mystical word!", FALSE, mob, obj, vict, TO_VICT);

    damage(mob, vict, number(30, 90), SPELL_LIGHTNING_BOLT, DAM_ELECTRIC);

    return FALSE;
  }

  return FALSE;
}

/* Dragon breaths acid on players.  Also blocks players from entering the treasure room to the east of DRAGON_LAIR room. */
int dragon_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *next, *temp, *vict;

  if(cmd == MSG_MOBACT)
  {
    /* Don't spec if mob is not fighting. */
    if(!(vict = mob->specials.fighting)) return FALSE;

    if(chance(10))
    {
      act("$n breaths a cloud of caustic green acid at $N!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n breaths a cloud of caustic green acid at you!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, number(50, 100), TYPE_UNDEFINED, DAM_NO_BLOCK);

      /* Check room for other players and check for splash damage. */
      for(temp = world[CHAR_REAL_ROOM(mob)].people; temp; temp = next)
      {
        next = temp->next_in_room;

        if(temp == vict) continue;

        if(chance(10))
        {
          act("$n suffers some splash damage!", TRUE, temp, 0, 0, TO_ROOM);
          act("You suffer some splash damage!", TRUE, temp, 0, 0, TO_CHAR);

          damage(mob, temp, number(25, 50), TYPE_UNDEFINED, DAM_NO_BLOCK);
        }
      }

      return FALSE;
    }

    return FALSE;
  }

  /* Block eastward movement. */
  if(cmd == CMD_EAST)
  {
    /* Don't spec if there is no ch. */
    if(!ch) return FALSE;
    /* Don't spec if ch is !mortal. */
    if(!IS_MORTAL(ch) && !IS_NPC(ch)) return FALSE;
    /* Don't spec if mob is not in LAIR. */
    if(V_ROOM(mob) != DRAGON_LAIR) return FALSE;

    act("$n blocks $N's way and slams $E down the northern tunnel with $s tail!", FALSE, mob, 0, ch, TO_NOTVICT);
    act("$n blocks your way and slams you down the northern tunnel with $s tail!", FALSE, mob, 0, ch, TO_VICT);

    char_from_room(ch);
    char_to_room(ch, real_room(UPPER_TUNNEL));
    do_look(ch, "", CMD_LOOK);

    act("$n tumbles into the room from the south!", TRUE, ch, 0, 0, TO_ROOM);

    damage(ch, ch, number(40, 60), TYPE_UNDEFINED, DAM_NO_BLOCK);

    return TRUE;
  }

  return FALSE;
}

/* Small damage spec. */
int pixie_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;

  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't spec if mob is not fighting. */
  if(!(vict = mob->specials.fighting)) return FALSE;

  if(chance(10))
  {
    act("$n giggles to $mself as $e quickly flies behind $N and slices $S hamstrings!", TRUE, mob, 0, vict, TO_NOTVICT);
    act("$n giggles to $mself as $e quickly flies behind you and slices your hamstrings!", FALSE, mob, 0, vict, TO_VICT);

    damage(mob, vict, number(20, 40), TYPE_UNDEFINED, DAM_NO_BLOCK);

    GET_POS(vict) = POSITION_SITTING;

    return FALSE;
  }

  return FALSE;
}

/* Simple message spec to make Grug more interesting. */
int zombie_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if no one is in the room. */
  if(count_mortals_room(mob, TRUE) < 1) return FALSE;

  /* 10% chance each case. */
  switch(number(0, 9))
  {
    case(0):
    {
      act("$n moans some curse loudly in ancient dwarven!", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(1):
    {
      act("$n pukes up a gout of black worm-infested blood!", FALSE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    case(2):
    {
      act("$n reaches up and re-adjusts $s broken neck slightly.", TRUE, mob, 0, 0, TO_ROOM);

      return FALSE;
    }
    default:
    {
      /* Do nothing. */

      return FALSE;
    }
  }

  return FALSE;
}


/* Object Specs */

/* Minor mana-regen spec.  33% chance each tick to regen up to 5 mana. */
int wonder_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner;

  /* Don't waste any more CPU time if it's not a TICK */
  if(cmd != MSG_TICK) return FALSE;
  /* Don't waste any more CPU time if obj is not equipped. */
  if(!(owner = obj->equipped_by)) return FALSE;
  /* Don't spec if owner already already has full mana. */
  if(GET_MANA(owner) >= GET_MAX_MANA(owner)) return FALSE;

  if(chance(33))
  {
    act("$p flashes with a pulse of magical energy!", TRUE, owner, obj, 0, TO_ROOM);
    act("$p flashes with a pulse of magical energy!", FALSE, owner, obj, 0, TO_CHAR);

    /* Regen up to 5 mana without going over max. */
    GET_MANA(owner) += MIN((GET_MAX_MANA(owner) - GET_MANA(owner)), 5);

    return FALSE;
  }

  return FALSE;
}


/* Room Specs */

/* Flavor text. */
int cracked_tunnel_spec(int room, CHAR *ch, int cmd, char *arg)
{
  /* Don't waste any more CPU time if it's not a MOBACT. */
  if(cmd != MSG_MOBACT) return FALSE;
  /* Don't waste any more CPU time if no one is in the room. */
  if(!(ch = world[real_room(CRACKED_TUNNEL)].people)) return FALSE;

  if(chance(10))
  {
    send_to_room("Water drains noisily into the cracks in the floor.\r\n", real_room(CRACKED_TUNNEL));
  }

  return FALSE;
}


/* Assign Specs */
void assign_goblin(void)
{
  /* Mobiles */
  assign_mob(DWELLER, dweller_spec);
  assign_mob(GRUG, grug_spec);
  assign_mob(CRAB, crab_spec);
  assign_mob(HERDER, herder_spec);
  assign_mob(DRAGON, dragon_spec);
  assign_mob(PIXIE, pixie_spec);
  assign_mob(ZOMBIE, zombie_spec);

  /* Objects */
  assign_obj(WONDER, wonder_spec);

  /* Rooms */
  assign_room(CRACKED_TUNNEL, cracked_tunnel_spec);
}
