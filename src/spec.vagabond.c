/*
  spec.vagabond.c - Specs for Vagabond Cove, by Night

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 12/12/2012
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
#include "constants.h"
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
/* Rooms*/
#define OLD_DRAGON_ROOM          350
#define WATERFALL_ROOM           474
#define ENDLESS_SEA_ROOM         478
#define LAVA_TUBE_ROOM           17501

/* Mobs */
#define OLD_DRAGON_MOB           301

/* Objects */
#define SPYGLASS_OBJ             204
#define TALON_OBJ                300
#define BOOTS_OBJ                301
#define FIRE_WALL_OBJ            305

/* Misc */
#define WATERFALL_DAMAGE         50

#define SPYGLASS_SPEC_CHANCE     70
#define SPYGLASS_DISTANCE        3


#define TALON_SPEC_1_CHANCE      5
#define TALON_SPEC_1_DAMAGE      65
#define TALON_SPEC_2_CHANCE      5
#define TALON_SPEC_2_DAMAGE      150
#define TALON_SPEC_2_DURATION    7
#define TALON_SPEC_2_REGEN       20

#define BOOTS_NORMAL_DAMROLL     2
#define BOOTS_SPEC_CHANCE        5
#define BOOTS_SPEC_DAMROLL       4
#define BOOTS_SPEC_DURATION      4

#define FIRE_WALL_DAMAGE         400

#define OLD_DRAGON_BREATH_CHANCE 40
#define OLD_DRAGON_BREATH_DAMAGE 2000
#define OLD_DRAGON_TAIL_CHANCE   45
#define OLD_DRAGON_TAIL_DAMAGE   100
#define OLD_DRAGON_TAIL_LAG      2
#define OLD_DRAGON_HEAT_MOBACTS  20
#define OLD_DRAGON_HEAT_DAMAGE   30

/* Room Specs */
/* Transport players from WATERFALL_ROOM to ENDLESS_SEA_ROOM, inflicting WATERFALL_DAMAGE damage. */
int waterfall_room(int room, CHAR *ch, int cmd, char *arg)
{
  if (cmd == MSG_ENTER)
  {
    if (!ch || !IS_MORTAL(ch)) return FALSE;

    act("You are swept over the edge of the waterfall!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n is swept over the edge of the waterfall!", TRUE, ch, 0, 0, TO_ROOM);

    char_from_room(ch);
    char_to_room(ch, real_room(WATERFALL_ROOM));

    act("$n plummets into the sea, having fallen from a waterfall far above!", TRUE, ch, 0, 0, TO_ROOM);

    do_look(ch, "", CMD_LOOK);

    damage(ch, ch, WATERFALL_DAMAGE, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

    return FALSE;
  }

  return FALSE;
}

int old_dragon_room(int room, CHAR *ch, int cmd, char *arg)
{
  OBJ *fire_wall = NULL;

  if (cmd == MSG_ZONE_RESET)
  {
    fire_wall = get_obj_room(FIRE_WALL_OBJ, OLD_DRAGON_ROOM);

    if (!fire_wall)
    {
      fire_wall = read_object(FIRE_WALL_OBJ, VIRTUAL);

      if (fire_wall)
      {
        send_to_room("A wall of flames roars life, sealing off the exit to the west.\r\n", room);

        world[room].dir_option[WEST]->to_room_r = real_room(0);

        obj_to_room(fire_wall, room);
      }
    }

    return FALSE;
  }

  return FALSE;
}

/* Mob Specs */
int old_dragon_mob(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *victim = NULL;
  CHAR *next_victim = NULL;
  OBJ *fire_wall = NULL;
  bool heat_up = FALSE;
  bool cool_down = FALSE;

  if (!mob) return FALSE;

  if (cmd == MSG_MOBACT)
  {
    if (GET_OPPONENT(mob) && CHAR_VIRTUAL_ROOM(mob) == OLD_DRAGON_ROOM)
    {
      fire_wall = get_obj_room(FIRE_WALL_OBJ, CHAR_VIRTUAL_ROOM(mob));

      if (!fire_wall)
      {
        fire_wall = read_object(FIRE_WALL_OBJ, VIRTUAL);

        if (fire_wall)
        {
          obj_to_room(fire_wall, CHAR_REAL_ROOM(mob));

          send_to_room("A wall of flames errupts into existence, encircling the chamber!\r\n", CHAR_REAL_ROOM(mob));
        }
      }

      if (fire_wall && !OBJ_SPEC(fire_wall))
      {
        OBJ_SPEC(fire_wall) = TRUE;

        send_to_room("The wall of flames increases in size and intensity, sealing off all exits from the chamber!\r\n", CHAR_REAL_ROOM(mob));
      }
    }
    else if (!GET_OPPONENT(mob) && CHAR_VIRTUAL_ROOM(mob) == OLD_DRAGON_ROOM)
    {
      fire_wall = get_obj_room(FIRE_WALL_OBJ, CHAR_VIRTUAL_ROOM(mob));

      if (fire_wall && OBJ_SPEC(fire_wall))
      {
        OBJ_SPEC(fire_wall) = FALSE;

        send_to_room("The wall of flames encircling the room decreases in intensity, revealing an exit to the north.\n\r", CHAR_REAL_ROOM(mob));
      }
    }

    if (GET_OPPONENT(mob) && GET_BANK(mob) < OLD_DRAGON_HEAT_MOBACTS)
    {
      GET_BANK(mob)++;

      if (GET_BANK(mob) == OLD_DRAGON_HEAT_MOBACTS)
      {
        heat_up = TRUE;
      }
    }
    else if (!GET_OPPONENT(mob) && GET_BANK(mob) > 0)
    {
      GET_BANK(mob)--;

      if (GET_BANK(mob) == 0)
      {
        cool_down = TRUE;
      }
    }

    if (heat_up)
    {
      act("Your scales begin to glow brightly, radiating intense heat!", FALSE, mob, 0, 0, TO_CHAR);
      act("$n's scales begin to glow brightly, radiating intense heat!", FALSE, mob, 0, 0, TO_ROOM);

      if (!mob_proto_table[mob->nr].tagline)
      {
        CREATE(mob_proto_table[mob->nr].tagline, struct tagline_data, 1);

        mob_proto_table[mob->nr].tagline->desc = strdup("......$n's scales glow brightly with intense heat!");
        mob_proto_table[mob->nr].tagline->next = NULL;
      }
    }
    else if (cool_down)
    {
      act("Your scales stop glowing with such intensity.", FALSE, mob, 0, 0, TO_CHAR);
      act("$n's scales stop glowing with such intensity.", FALSE, mob, 0, 0, TO_ROOM);

      if (mob_proto_table[mob->nr].tagline)
      {
        free(mob_proto_table[mob->nr].tagline->desc);
        free(mob_proto_table[mob->nr].tagline);

        mob_proto_table[mob->nr].tagline = NULL;
      }
    }

    if (GET_BANK(mob) == OLD_DRAGON_HEAT_MOBACTS)
    {
      for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_victim)
      {
        next_victim = victim->next_in_room;

        if (victim == mob || IS_IMMORTAL(victim)) continue;

        act("Waves of heat radiate from your scales, searing $N's flesh!", FALSE, mob, 0, victim, TO_CHAR);
        act("Waves of heat radiate from $n's scales, searing your flesh!", FALSE, mob, 0, victim, TO_VICT);
        act("Waves of heat radiate from $n's scales, searing $N's flesh!", FALSE, mob, 0, victim, TO_NOTVICT);

        damage(mob, victim, OLD_DRAGON_HEAT_DAMAGE, TYPE_UNDEFINED, DAM_NO_BLOCK);
      }
    }

    if (GET_OPPONENT(mob))
    {
      if (chance(OLD_DRAGON_BREATH_CHANCE))
      {
        act("You breathe a tremendous gout of fire, sweeping the room with flames!", FALSE, mob, 0, 0, TO_CHAR);
        act("$n breathes a tremendous gout of fire, sweeping the room with flames!", FALSE, mob, 0, 0, TO_ROOM);

        if (count_mortals_room(mob, TRUE) > 0)
        {
          for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_victim)
          {
            next_victim = victim->next_in_room;

            if (victim == mob || IS_IMMORTAL(victim)) continue;

            act("You are engulfed in scorching flames!", FALSE, ch, 0, 0, TO_VICT);
            act("$n is engulfed in scorching flames!", FALSE, ch, 0, 0, TO_NOTVICT);

            damage(mob, victim, (OLD_DRAGON_BREATH_DAMAGE / count_mortals_room(mob, TRUE)), TYPE_UNDEFINED, DAM_NO_BLOCK);
          }
        }
      }
      else if (chance(OLD_DRAGON_TAIL_CHANCE))
      {
        victim = get_random_victim_fighting(mob);

        if (victim)
        {
          act("You impale $N viciously with the spined tip of your tail!", FALSE, mob, 0, victim, TO_CHAR);
          act("$n impales you viciously with the spined tip of $s tail!", FALSE, mob, 0, victim, TO_VICT);
          act("$n impales $N viciously with the spined tip of $s tail!", FALSE, mob, 0, victim, TO_NOTVICT);

          damage(mob, victim, OLD_DRAGON_TAIL_DAMAGE, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

          WAIT_STATE(victim, OLD_DRAGON_TAIL_LAG);

          if (victim != GET_OPPONENT(mob))
          {
            stop_fighting(victim);
          }
        }
      }
    }

    return FALSE;
  }
  else if (cmd == MSG_DIE)
  {
    if (CHAR_VIRTUAL_ROOM(mob) != OLD_DRAGON_ROOM) return FALSE;

    fire_wall = get_obj_room(FIRE_WALL_OBJ, CHAR_VIRTUAL_ROOM(mob));

    if (fire_wall)
    {
      send_to_room("The wall of flames flickers and fades, revealing a hidden lava tube to the west.\r\n", CHAR_REAL_ROOM(mob));

      world[CHAR_REAL_ROOM(mob)].dir_option[WEST]->to_room_r = real_room(LAVA_TUBE_ROOM);

      extract_obj(fire_wall);
    }

    return FALSE;
  }

  return FALSE;
}

void spyglass_list_scanned_chars(CHAR *list, CHAR *ch, int distance, int dir)
{
  const char *how_far[] = {
    "close",
    "a ways",
    "far"
  };

  const char *dir_messages[] = {
    "to the north",
    "to the east",
    "to the south",
    "to the west",
    "above you",
    "below you"
  };

  CHAR *i = NULL;
  char buf[MSL];
  int count = 0, count_limit = 10;

  for (i = list; i && count < count_limit; i = i->next_in_room)
  {
    if (CAN_SEE(ch, i)) count++;
  }

  if (!count) return;

  sprintf(buf, "You see ");

  for (i = list; i && count > 0; i = i->next_in_room)
  {
    if (!CAN_SEE(ch, i)) continue;

    count--;

    sprintf(buf, "%s%s", buf, (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)));

    if (count > 1)
    {
      strcat(buf, ", ");
    }
    else if (count == 1)
    {
      strcat(buf, " and ");
    }
    else
    {
      sprintf(buf, "%s %s %s.\n\r", buf, how_far[distance], dir_messages[dir]);
    }
  }

  send_to_char(buf, ch);
}

int spyglass_obj(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner = NULL;
  char buf[MSL];
  int dir = 0, room = 0, exit_room = 0, distance = 0;

  if (!obj || ch != obj->equipped_by || !(owner = obj->equipped_by)) return FALSE;

  if (cmd != CMD_USE || !AWAKE(owner)) return FALSE;

  arg = one_argument(arg, buf);

  if (!*buf) return FALSE;

  string_to_lower(buf);

  if (!isname(buf, OBJ_NAME(obj))) return FALSE;

  if (IS_AFFECTED(owner, AFF_BLIND))
  {
    send_to_char("You can't see a thing, you're blind!\n\r", owner);
  }
  else
  {
    act("You peer into your spyglass and survey the area around you.", FALSE, owner, 0, 0, TO_CHAR);
    act("$n peers into $s spyglass and surveys the area around $m.", TRUE, owner, 0, 0, TO_ROOM);

    for (dir = NORTH; dir <= DOWN; dir++)
    {
      room = CHAR_REAL_ROOM(owner);

      if (!CAN_GO(owner, dir)) continue;

      if (!chance(SPYGLASS_SPEC_CHANCE))
      {
        printf_to_char(owner, "Your view %s%s was a bit out of focus.\n\r", (dir < UP ? "to the " : ""), dirs[dir]);

        continue;
      }

      for (distance = 0; distance < SPYGLASS_DISTANCE; distance++)
      {
        if (!world[room].dir_option[dir]) break;

        exit_room = world[room].dir_option[dir]->to_room_r;

        if (exit_room == NOWHERE ||
            exit_room == real_room(0) ||
            IS_SET(world[room].dir_option[dir]->exit_info, EX_CLOSED) ||
            IS_SET(world[room].dir_option[dir]->exit_info, EX_CLIMB) ||
            IS_SET(world[room].dir_option[dir]->exit_info, EX_JUMP) ||
            IS_SET(world[room].dir_option[dir]->exit_info, EX_CRAWL) ||
            IS_SET(world[room].dir_option[dir]->exit_info, EX_ENTER))
          break;

        if (world[exit_room].people)
        {
          spyglass_list_scanned_chars(world[exit_room].people, owner, distance, dir);
        }

        room = exit_room;
      }
    }

    WAIT_STATE(owner, PULSE_VIOLENCE);
  }

  return TRUE;
}

int talon_obj(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner = NULL;
  CHAR *victim = NULL;
  CHAR *group_leader = NULL;
  CHAR *group_member = NULL;
  FOL *follower = NULL;
  AFF af;

  if (!obj || !(owner = obj->equipped_by)) return FALSE;

  if (cmd == MSG_TICK)
  {
    if (GET_OPPONENT(owner) && chance(TALON_SPEC_1_CHANCE))
    {
      if (!(victim = GET_OPPONENT(owner))) return FALSE;

      act("Claws of flame project from your talon, raking $N with blazing fury!", FALSE, owner, obj, victim, TO_CHAR);
      act("Claws of flame project from $n's talon, raking you with blazing fury!", FALSE, owner, obj, victim, TO_VICT);
      act("Claws of flame project from $n's talon, raking $N with blazing fury!", FALSE, owner, obj, victim, TO_NOTVICT);

      damage(owner, victim, TALON_SPEC_1_DAMAGE, TYPE_UNDEFINED, DAM_NO_BLOCK);

      GET_MANA(owner) = MIN(GET_MAX_MANA(owner), GET_MANA(owner) + TALON_SPEC_1_DAMAGE);
    }

    if (chance(TALON_SPEC_2_CHANCE))
    {
      act("Your talon flares intensely, searing your flesh!", FALSE, owner, 0, 0, TO_CHAR);
      act("$n's talon flares intensely, searing $s flesh!", FALSE, owner, 0, 0, TO_ROOM);

      damage(owner, owner, TALON_SPEC_2_DAMAGE, TYPE_UNDEFINED, DAM_NO_BLOCK);

      af.type       = SPELL_REGENERATION;
      af.duration   = TALON_SPEC_2_DURATION;
      af.modifier   = TALON_SPEC_2_REGEN;
      af.location   = APPLY_HP_REGEN;
      af.bitvector  = 0;
      af.bitvector2 = 0;

      /* Find the group leader. */
      if (owner->master)
      {
        group_leader = owner->master;
      }
      else /* The acting character is the leader (or there is no group leader). */
      {
        group_leader = owner;
      }

      /* Loop through all of the group members of group_leader's group. */
      for (follower = group_leader->followers; follower; follower = follower->next)
      {
        group_member = follower->follower;

        if (!group_member || group_member == owner || !SAME_ROOM(group_member, owner) || !IS_MORTAL(group_member)) continue;

        if (IS_AFFECTED(group_member, AFF_GROUP) && !affected_by_spell(group_member, SPELL_REGENERATION))
        {
          act("$N is infused with a regenerative power!", FALSE, owner, 0, group_member, TO_CHAR);
          act("You are infused with a regenerative power!", FALSE, owner, 0, group_member, TO_VICT);
          act("$N is infused with a regenerative power!", FALSE, owner, 0, group_member, TO_NOTVICT);

          affect_to_char(group_member, &af);
        }
      }
    }

    return FALSE;
  }

  return FALSE;
}

int boots_obj(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *owner = NULL;

  if (!obj || !(owner = obj->equipped_by)) return FALSE;

  if (EQ(owner, WEAR_FEET) != obj) return FALSE;

  if (cmd == MSG_TICK)
  {
    /* Sanity check. */
    if (OBJ_SPEC(obj) < 0 || OBJ_SPEC(obj) > BOOTS_SPEC_DURATION)
    {
      OBJ_SPEC(obj) = 0;

      unequip_char(owner, WEAR_FEET);

      /* OBJ_AFF0 0 should be set as damroll. */
      OBJ_AFF0(obj).modifier = BOOTS_NORMAL_DAMROLL;

      equip_char(owner, obj, WEAR_FEET);
    }

    if (OBJ_SPEC(obj))
    {
      OBJ_SPEC(obj)--;

      if (!OBJ_SPEC(obj))
      {
        act("Your boots emit a puff of smoke and smolder to a dull crimson color.", FALSE, owner, 0, 0, TO_CHAR);
        act("$n's boots emit a puff of smoke and smolder to a dull crimson color.", FALSE, owner, 0, 0, TO_ROOM);

        OBJ_SPEC(obj) = 0;

        unequip_char(owner, WEAR_FEET);

        /* OBJ_AFF0 0 should be set as damroll. */
        OBJ_AFF0(obj).modifier = BOOTS_NORMAL_DAMROLL;

        equip_char(owner, obj, WEAR_FEET);
      }
    }

    if (!OBJ_SPEC(obj) && chance(BOOTS_SPEC_CHANCE))
    {
      act("Your boots glow white-hot, imbuing you with draconic power!", FALSE, owner, 0, 0, TO_CHAR);
      act("$n's boots glow white-hot, imbuing $m with draconic power!", FALSE, owner, 0, 0, TO_ROOM);

      OBJ_SPEC(obj) = BOOTS_SPEC_DURATION;

      unequip_char(owner, WEAR_FEET);

      /* OBJ_AFF0 0 should be set as damroll. */
      OBJ_AFF0(obj).modifier = BOOTS_SPEC_DAMROLL;

      equip_char(owner, obj, WEAR_FEET);
    }

    return FALSE;
  }
  else if (cmd == MSG_BEING_REMOVED)
  {
    unequip_char(owner, WEAR_FEET);

    /* OBJ_AFF0 0 should be set as damroll. */
    OBJ_AFF0(obj).modifier = BOOTS_NORMAL_DAMROLL;
    OBJ_SPEC(obj) = 0;

    equip_char(owner, obj, WEAR_FEET);

    return FALSE;
  }

  return FALSE;
}

int fire_wall_obj(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  CHAR *dragon = NULL;

  if (!obj) return FALSE;

  if (OBJ_VIRTUAL_ROOM(obj) != OLD_DRAGON_ROOM) return FALSE;

  if (cmd == MSG_ZONE_RESET)
  {
    dragon = get_ch(OLD_DRAGON_MOB, WORLD, OBJ_REAL_ROOM(obj));

    if (OBJ_SPEC(obj) && (!dragon || !GET_OPPONENT(dragon)))
    {
      OBJ_SPEC(obj) = FALSE;
    }

    if (world[OBJ_REAL_ROOM(obj)].dir_option[WEST]->to_room_r == real_room(LAVA_TUBE_ROOM))
    {
      send_to_room("The wall of flames burns intensely, sealing off the exit to the west.\n\r", OBJ_REAL_ROOM(obj));

      world[OBJ_REAL_ROOM(obj)].dir_option[WEST]->to_room_r = real_room(0);
    }

    return FALSE;
  }
  else if (cmd == CMD_WEST || (cmd == CMD_NORTH && OBJ_SPEC(obj)))
  {
    if (!ch) return FALSE;

    if (!AWAKE(ch) || IS_IMMORTAL(ch)) return FALSE;

    act("A massive wall of flames blocks your way and burns you!", FALSE, ch, 0, 0, TO_CHAR);
    act("A massive wall of flames blocks $n's way and burns you!", FALSE, ch, 0, 0, TO_ROOM);

    damage(ch, ch, FIRE_WALL_DAMAGE, TYPE_UNDEFINED, DAM_NO_BLOCK);

    return TRUE;
  }

  return FALSE;
}

/* Assign Specs */
void assign_vagabond(void)
{
  assign_room(WATERFALL_ROOM, waterfall_room);
  assign_room(OLD_DRAGON_ROOM, old_dragon_room);
  assign_mob(OLD_DRAGON_MOB, old_dragon_mob);
  assign_obj(SPYGLASS_OBJ, spyglass_obj);
  assign_obj(TALON_OBJ, talon_obj);
  assign_obj(BOOTS_OBJ, boots_obj);
  assign_obj(FIRE_WALL_OBJ, fire_wall_obj);
}
