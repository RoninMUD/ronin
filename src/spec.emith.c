/*
  spec.emith.c - Specs for Emithoynarthe, by SolmyrNight

  Written by Alan K. Miles for RoninMUD
  Based on Original Specs by Robert Upshall for RoninMUD
  Last Modification Date: 04/28/2003
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
#define GHOST           26702

/* Objects */
#define OAR             26701
#define PLANT           26707
#define CAPE            26709
#define CAPE_SPECIAL    26719
#define CUTLASS         26710

/* Rooms */
#define OUTSIDE_ROCK    26701
#define INSIDE_ROCK     26702
#define BEACH_ROOM      26705
#define BARGE_ROOM      26708
#define SHIP_ROOM       26709

/* Other */
#define BRANCHES_THORNS "branches thorns"


/* Mobile Specs */

/* Ghost summons followers to damage people he is fighting. */
int ghost_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *next_vict, *vict;

  if(cmd == MSG_MOBACT)
  {
    /* Don't spec if not fighting. */
    if(!mob->specials.fighting) return FALSE;

    if(!chance(20)) return FALSE;

    act("$n makes a gesture and his loyal Albatross friends assist $m by attacking in huge numbers!", FALSE, mob, 0, 0, TO_ROOM);

    for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
    {
      next_vict = vict->next_in_room;

      if(vict == mob) continue;
      if(vict->specials.fighting != mob) continue;
      if(!IS_MORTAL(vict) && !IS_NPC(vict)) continue;

      send_to_char("You are assaulted by a swirling mass of feathers and beaks!\r\n", vict);
      damage(mob, vict, number(180, 220), TYPE_UNDEFINED, DAM_NO_BLOCK);
    }

    return FALSE;
  }

  return FALSE;
}


/* Object Specs */

/* 'Row' BARGE_ROOM to SHIP_ROOM and then break the oar. */
int oar_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner;
  char buf[MIL];
  int beach, barge, ship, nowhere;

  beach = real_room(BEACH_ROOM);
  barge = real_room(BARGE_ROOM);
  ship = real_room(SHIP_ROOM);
  nowhere = real_room(0);

  if(cmd == CMD_USE)
  {
    /* Don't spec if no ch. */
    if(!ch) return FALSE;
    /* Don't spec if ch is not awake. */
    if(!AWAKE(ch)) return FALSE;
    /* Don't spec if obj is not equipped by the actor. */
    if(!(owner = obj->equipped_by)) return FALSE;
    /* Don't spec if actor is not the owner. */
    if(ch != owner) return FALSE;

    one_argument(arg, buf);

    /* Return if no target. */
    if(!*buf) return FALSE;
    /* Return if its not OAR. */
    if(!isname(buf, OBJ_NAME(obj))) return FALSE;

    /* Don't spec if not in BARGE_ROOM. */
    if(CHAR_REAL_ROOM(owner) != barge)
    {
      send_to_char("It doesn't look to be of any use here.\r\n", owner);

      return TRUE;
    }
    else if(world[barge].dir_option[SOUTH]->to_room_r == ship)
    {
      act("$n tries to row to shore but the $o breaks in $s's hands against the current!", FALSE, owner, obj, 0, TO_ROOM);
      act("You try to row to shore but the $o breaks in your hands against the current!", FALSE, owner, obj, 0, TO_CHAR);

      extract_obj(obj);

      return TRUE;
    }
    else
    {
      world[beach].dir_option[SOUTH]->to_room_r = nowhere;
      send_to_room("The barge pulls away from the beach.\r\n", beach);

      world[barge].dir_option[NORTH]->to_room_r = nowhere;
      world[barge].dir_option[SOUTH]->to_room_r = ship;
      act("$n rows the barge out to the ship with $s $o.", FALSE, owner, obj, 0, TO_ROOM);
      act("You row the barge out to the ship with $o.", FALSE, owner, obj, 0, TO_CHAR);

      world[ship].dir_option[NORTH]->to_room_r = barge;
      send_to_room("A barge pulls up to the side of the ship.\r\n", ship);

      act("The $o breaks in $n's hands at the last moment!", FALSE, owner, obj, 0, TO_ROOM);
      act("The $o breaks in your hands at the last moment!", FALSE, owner, obj, 0, TO_CHAR);

      extract_obj(obj);

      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

/* If eaten, heals good people 100 hps, if evil damages them for 100 hps. */
int plant_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner;
  OBJ *tmp;
  char buf[MIL];

  if(cmd == CMD_EAT)
  {
    /* Don't spec if no ch. */
    if(!ch) return FALSE;
    /* Don't spec if ch is not awake. */
    if(!AWAKE(ch)) return FALSE;
    /* Don't spec if ch is !mortal. */
    if(!IS_MORTAL(ch)) return FALSE;
    /* Don't spec if obj is not carried by actor. */
    if(!(owner = obj->carried_by)) return FALSE;
    /* Don't spec if actor is not the owner. */
    if(ch != owner) return FALSE;

    one_argument(arg, buf);

    /* Return if no target. */
    if(!*buf) return FALSE;
    /* Return if we don't find an object matching args. */
    if(!(tmp = get_obj_in_list_vis(owner, buf, owner->carrying))) return FALSE;
    /* Return if its not PLANT. */
    if(tmp != obj) return FALSE;

    /* Return if too full. */
    if(GET_COND(owner, FULL) > 20)
    {
      return FALSE;
    }

    act("$n eats $p.", FALSE, owner, tmp, 0, TO_ROOM);
    act("You eat $p.", FALSE, owner, tmp, 0, TO_CHAR);

    GET_COND(owner, FULL) += tmp->obj_flags.value[0];

    extract_obj(tmp);

    /* Heal owner 100 hps if good aligned. */
    if(IS_GOOD(owner))
    {
      act("$n looks better.", TRUE, owner, 0, 0, TO_ROOM);
      send_to_char("You feel better.\r\n", owner);

      GET_HIT(owner) = MIN(GET_MAX_HIT(owner), (GET_HIT(owner) + 100));
    }
    /* Damage owner 100 hps if evil aligned. */
    else if(IS_EVIL(owner))
    {
      act("$n looks sick.", TRUE, owner, 0, 0, TO_ROOM);
      send_to_char("You feel sick.\r\n", owner);

      GET_HIT(owner) = MAX((GET_HIT(owner) - 100), 1);
    }

    if(GET_COND(owner, FULL) > 20)
    {
      send_to_char("You are full.\r\n", owner);
    }

    return TRUE;
  }

  return FALSE;
}

/* Make CAPE take up both neck slots. */
int cape_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner;
  OBJ *tmp, *tmp2;
  char *argument;
  char buf[MIL], buf2[MIL];

  if(cmd == CMD_WEAR)
  {
    /* Don't spec if no ch. */
    if(!ch) return FALSE;
    /* Don't spec if ch is not awake. */
    if(!AWAKE(ch)) return FALSE;
    /* Don't spec if obj is not equipped or carried by actor. */
    if(!(owner = obj->carried_by))
    {
      if(!(owner = obj->equipped_by))
      {
        return FALSE;
      }
    }
    /* Don't spec if actor is not the owner. */
    if(ch != owner) return FALSE;

    argument = arg;
    argument = one_argument(argument, buf);
    argument = one_argument(argument, buf2);

    /* Return if no target. */
    if(!*buf) return FALSE;

    /* Prevent 'wear all' */
    if(isname(buf, "all"))
    {
      act("The weight of $p prevents your actions.", FALSE, owner, obj, 0, TO_CHAR);

      return TRUE;
    }
    /* Do special checking to block special cases involving CAPE. */
    else
    {
      /* Return if we don't find an object matching args. */
      if(!(tmp = get_obj_in_list_vis(owner, buf, owner->carrying))) return FALSE;

      /* Arg targeted CAPE. */
      if(tmp == obj)
      {
        if(!((owner->equipment[WEAR_NECK_1] == 0) && (owner->equipment[WEAR_NECK_2] == 0)))
        {
          act("The weight of the $o prevents anything else from being worn about your neck effectively.", FALSE, owner, obj, 0, TO_CHAR);

          return TRUE;
        }
        else
        {
          return FALSE;
        }
      }
      /* Arg matched some other object that can be worn about the neck. */
      else if(CAN_WEAR(tmp, ITEM_WEAR_NECK))
      {
        if(!*buf2)
        {
          if((tmp2 = owner->equipment[WEAR_NECK_1]) != 0)
          {
            if((V_OBJ(tmp2) == CAPE) || (V_OBJ(tmp2) == CAPE_SPECIAL))
            {
              act("The weight of the $o prevents anything else from being worn about your neck effectively.", FALSE, owner, tmp2, 0, TO_CHAR);

              return TRUE;
            }
          }
          else if((tmp2 = owner->equipment[WEAR_NECK_2]) != 0)
          {
            if((V_OBJ(tmp2) == CAPE) || (V_OBJ(tmp2) == CAPE_SPECIAL))
            {
              act("The weight of the $o prevents anything else from being worn about your neck effectively.", FALSE, owner, tmp2, 0, TO_CHAR);

              return TRUE;
            }
          }
          else
          {
            return FALSE;
          }
        }
        else if(isname(buf2, "neck"))
        {
          if((tmp2 = owner->equipment[WEAR_NECK_1]) != 0)
          {
            if((V_OBJ(tmp2) == CAPE) || (V_OBJ(tmp2) == CAPE_SPECIAL))
            {
              act("The weight of the $o prevents anything else from being worn about your neck effectively.", FALSE, owner, tmp2, 0, TO_CHAR);

              return TRUE;
            }
          }
          else if((tmp2 = owner->equipment[WEAR_NECK_2]) != 0)
          {
            if((V_OBJ(tmp2) == CAPE) || (V_OBJ(tmp2) == CAPE_SPECIAL))
            {
              act("The weight of the $o prevents anything else from being worn about your neck effectively.", FALSE, owner, tmp2, 0, TO_CHAR);

              return TRUE;
            }
          }
          else
          {
            return FALSE;
          }
        }
        else
        {
          return FALSE;
        }
      }
      else
      {
        return FALSE;
      }
    }

    return FALSE;
  }

  return FALSE;
}

/* Cast invisibility on the wielder. */
int cutlass_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner;
  OBJ *tmp;
  char buf[MIL];

  if(cmd == CMD_WIELD)
  {
    /* Don't spec if no ch. */
    if(!ch) return FALSE;
    /* Don't spec if ch is not awake. */
    if(!AWAKE(ch)) return FALSE;
    /* Don't spec if obj is not carried by actor. */
    if(!(owner = obj->carried_by)) return FALSE;
    /* Don't spec if actor is not the owner. */
    if(ch != owner) return FALSE;

    /* Return if already wielding something. */
    if(owner->equipment[WIELD] != 0) return FALSE;

    one_argument(arg, buf);

    /* Return if no target. */
    if(!*buf) return FALSE;
    /* Return if we don't find an object matching args. */
    if(!(tmp = get_obj_in_list_vis(owner, buf, owner->carrying))) return FALSE;
    /* Return if its not CUTLASS. */
    if(tmp != obj) return FALSE;

    /* Give owner invisibility. */
    if(!affected_by_spell(owner, SPELL_INVISIBLE))
    {
      spell_invisibility(50, owner, owner, 0);
    }

    return FALSE;
  }

  return FALSE;
}


/* Room Specs */

/* Branches hurt and poison people moving them. */
int branches_spec(int room, CHAR *ch, int cmd, char *arg)
{
  char buf[MIL];
  struct affected_type_5 af;

  /* Hurt and poison people moving the branches. */
  if(cmd == CMD_MOVE)
  {
    /* Don't spec if no ch. */
    if(!ch) return FALSE;
    /* Don't spec if ch is not awake. */
    if(!AWAKE(ch)) return FALSE;
    /* Don't spec if ch is !mortal. */
    if(!IS_MORTAL(ch)) return FALSE;

    one_argument(arg, buf);

    /* Return if no target. */
    if(!*buf) return FALSE;
    /* Return if its not BRANCHES_THORNS. */
    if(!isname(buf, BRANCHES_THORNS)) return FALSE;

    act("$n hurts $mself on the sharp thorns!", FALSE, ch, 0, 0, TO_ROOM);
    act("You hurt yourself on the sharp thorns!", FALSE, ch, 0, 0, TO_CHAR);

    /* Give ch poison. */
    if(!affected_by_spell(ch, SPELL_POISON))
    {
      af.type = SPELL_POISON;
      af.duration = 60;
      af.modifier = -1;
      af.location = APPLY_STR;
      af.bitvector = 0;
      af.bitvector2 = 0;
      affect_to_char(ch, &af);

      send_to_char("You feel very sick.\r\n", ch);
    }

    damage(ch, ch, (GET_HIT(ch) / 2), TYPE_UNDEFINED, DAM_NO_BLOCK);

    return FALSE;
  }

  return FALSE;
}

/* Reset the barge to the beach at zone reset. */
int barge_spec(int room, CHAR *ch, int cmd, char *arg)
{
  int barge, beach, ship, nowhere;
  OBJ* obj, *obj2;
  beach = real_room(BEACH_ROOM);
  barge = room;
  ship = real_room(SHIP_ROOM);
  nowhere = real_room(0);

  if(cmd == MSG_ZONE_RESET)
  {
    if(world[beach].dir_option[SOUTH]->to_room_r == nowhere)
    {
      world[beach].dir_option[SOUTH]->to_room_r = barge;
      send_to_room("A barge drifts to shore and lodges itself into the sandy beach.\r\n", beach);

      world[barge].dir_option[NORTH]->to_room_r = beach;
      world[barge].dir_option[SOUTH]->to_room_r = nowhere;
      send_to_room("The barge dislodges from the ship and drifts back to shore.\r\n", barge);

      world[ship].dir_option[NORTH]->to_room_r = nowhere;
      send_to_room("The barge dislodges from the ship and drifts back to shore.\r\n", ship);
    }

	/* A spec to make sure the oar loads on zone reset
	added by Solmyr in April 2009 */
	if(!(obj = get_obj_in_list_num(real_object(OAR), world[room].contents)))
	{
		obj2 = read_object(OAR, VIRTUAL);
		obj_to_room(obj2, real_room(BARGE_ROOM));
		return FALSE;
	}
    return FALSE;
  }

  return FALSE;
}


/* Assign Specs */
void assign_emith(void)
{
  assign_mob(GHOST, ghost_spec);

  assign_obj(OAR, oar_spec);
  assign_obj(PLANT, plant_spec);
  assign_obj(CAPE, cape_spec);
  assign_obj(CUTLASS, cutlass_spec);
  assign_obj(CAPE_SPECIAL, cape_spec);

  assign_room(OUTSIDE_ROCK, branches_spec);
  assign_room(INSIDE_ROCK, branches_spec);
  assign_room(BARGE_ROOM, barge_spec);
}

