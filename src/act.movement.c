/**************************************************************************
*  file: act.movement.c , Implementation of commands      Part of DIKUMUD *
*  Usage : Movement commands, close/open & lock/unlock doors.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "utility.h"
#include "fight.h"
#include "act.h"
#include "cmd.h"
#include "subclass.h"

/* external functs */

int hit_limit(struct char_data *ch);
int special(CHAR *ch, int cmd, char *arg);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list);
void stop_follower(struct char_data *ch);
void do_special_move(struct char_data *ch, char *arg, int cmd);

void dt_cry(CHAR *ch) {
  if (!ch || CHAR_REAL_ROOM(ch) == NOWHERE) return;

  act("Your body stiffens as you hear $n scream in agony.", FALSE, ch, 0, 0, TO_ROOM);

  for (int dir = NORTH; dir <= DOWN; dir++) {
    if (CAN_GO(ch, dir)) {
      send_to_room("Your body stiffens as you hear someone scream in agony.\n\r", EXIT(ch, dir)->to_room_r);
    }
  }
}

int dt_or_hazard(CHAR *ch) {
  char buf[MSL];

  if (!IS_IMMORTAL(ch) && IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), HAZARD) && chance(50)) {
    if (!IS_NPC(ch)) {
      snprintf(buf, sizeof(buf), "WIZINFO: %s fell into a HAZARD at %s (%d)",
        GET_NAME(ch), ROOM_NAME(CHAR_REAL_ROOM(ch)), ROOM_VNUM(CHAR_REAL_ROOM(ch)));
      wizlog(buf, LEVEL_IMM, 3);
      log_s(buf);
      deathlog(buf);
    }

    // signal character and rider with MSG_AUTORENT, which is a terrible hack
    // to have dynamic enchantments remove themselves and not save affects
    // permanently in raw_kill

    if (GET_MOUNT(ch)) {
      signal_char(GET_MOUNT(ch), GET_MOUNT(ch), MSG_AUTORENT, "");

      raw_kill(GET_MOUNT(ch));
    }

    signal_char(ch, ch, MSG_AUTORENT, "");

    raw_kill(ch);

    return TRUE;
  }

  if (!IS_IMMORTAL(ch) && IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), DEATH)) {
    if (!IS_NPC(ch)) {
      snprintf(buf, sizeof(buf), "WIZINFO: %s fell into a DT at %s (%d)",
        GET_NAME(ch), ROOM_NAME(CHAR_REAL_ROOM(ch)), ROOM_VNUM(CHAR_REAL_ROOM(ch)));
      wizlog(buf, LEVEL_IMM, 3);
      log_s(buf);
      deathlog(buf);
    }

    if (GET_MOUNT(ch)) {
      SET_BIT(GET_AFF2(GET_MOUNT(ch)), AFF2_IMMINENT_DEATH);
      GET_DEATH_TIMER(GET_MOUNT(ch)) = 2;
    }

    SET_BIT(GET_AFF2(ch), AFF2_IMMINENT_DEATH);
    GET_DEATH_TIMER(ch) = 2;

    bool found_death_text = FALSE;

    for (struct extra_descr_data *extra_desc = world[CHAR_REAL_ROOM(ch)].ex_description; extra_desc; extra_desc = extra_desc->next) {
      if (!strcmp(extra_desc->keyword, "death_text") && extra_desc->description) {
        found_death_text = TRUE;

        send_to_char(extra_desc->description, ch);

        break;
      }
    }

    if (!found_death_text) {
      send_to_char("....\n\rDarkness begins to decend around you....\n\rYour time of death is near....\n\r", ch);
    }

    dt_cry(ch);

    return TRUE;
  }

  return FALSE;
}

/*
Move a character (and their mount) in the direction specified; does not move followers.

Returns 1 if movement succeeded, 0 if movement failed, -1 if movement killed the character.
*/
int move_char(CHAR *ch, int dir, bool spec_check) {
  char buf[MSL];

  /* Sanity checks. */
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE) || (dir < NORTH) || (dir > DOWN) || !world[CHAR_REAL_ROOM(ch)].dir_option[dir]) return FALSE;

  /* Record the destination room. */
  int dest_room_rnum = EXIT(ch, dir)->to_room_r;

  /* Check that the exit leads to a valid room. */
  if ((dest_room_rnum == NOWHERE) || (dest_room_rnum == real_room(0))) return FALSE;

  /* Check for invalid movement positions. */
  if (GET_POS(ch) <= POSITION_STUNNED) {
    send_to_char("You are in pretty bad shape, unable to do anything!\n\r", ch);

    return FALSE;
  }
  else if (GET_POS(ch) == POSITION_SLEEPING) {
    send_to_char("In your dreams, or what?\n\r", ch);

    return FALSE;
  }
  else if (GET_POS(ch) == POSITION_RESTING) {
    send_to_char("Nah... You feel too relaxed to do that...\n\r", ch);

    return FALSE;
  }

  if (!IS_IMMORTAL(ch)) {
    /* Check for Pray. */
    if (affected_by_spell(ch, SKILL_PRAY)) {
      send_to_char("You are deep in prayer, unable to follow.\n\r", ch);

      return FALSE;
    }

    /* Check for Meditation. */
    if (affected_by_spell(ch, SKILL_MEDITATE) && (duration_of_spell(ch, SKILL_MEDITATE) >= 10)) {
      send_to_char("You are deep in meditation, unable to follow.\n\r", ch);

      return FALSE;
    }

    /* Check for Paralaysis and Hold Person. */
    if (!GET_MOUNT(ch) && (IS_AFFECTED(ch, AFF_PARALYSIS) || IS_AFFECTED(ch, AFF_HOLD))) {
      /* Ronin SC3: Combat Zen - 50% chance to be able to move, even if paralyzed or held. */
      if (!(IS_MORTAL(ch) && check_subclass(ch, SC_RONIN, 3) && chance(50))) {
        send_to_char("You are paralyzed!  You can't move!\n\r", ch);

        return FALSE;
      }
    }

    /* Check if mount can move. */
    if (GET_MOUNT(ch) && ((GET_POS(GET_MOUNT(ch)) < POSITION_STANDING) || (IS_AFFECTED(ch, AFF_PARALYSIS) || IS_AFFECTED(ch, AFF_HOLD)))) {
      send_to_char("Your mount is unable to move.\n\r", ch);

      return FALSE;
    }

    /* Check for tunnel. */
    if (IS_SET(ROOM_FLAGS(dest_room_rnum), TUNNEL) && (count_mortals_real_room(dest_room_rnum) > 0) && !CHAOSMODE) {
      send_to_char("It's too narrow to go there.\n\r", ch);

      return FALSE;
    }

    /* Check if exit is a flying room. */
    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), FLYING) || IS_SET(ROOM_FLAGS(dest_room_rnum), FLYING)) {
      if (dir == UP) {
        if (GET_MOUNT(ch)) {
          if (!IS_SET(GET_ACT(GET_MOUNT(ch)), ACT_FLY)) {
            send_to_char("Your mount isn't capable of flight.\n\r", ch);

            return FALSE;
          }
          else if (!IS_AFFECTED(GET_MOUNT(ch), AFF_FLY)) {
            send_to_char("Your mount isn't flying.\n\r", ch);

            return FALSE;
          }
        }
        else if (IS_MORTAL(ch)) {
          send_to_char("You need a flying mount to gain altitude.\n\r", ch);

          return FALSE;
        }
      }
      else {
        if (GET_MOUNT(ch) && !IS_AFFECTED(GET_MOUNT(ch), AFF_FLY)) {
          send_to_char("Your mount isn't flying.\n\r", ch);

          return FALSE;
        }
        else if (!IS_AFFECTED(ch, AFF_FLY)) {
          send_to_char("You are not flying.\n\r", ch);

          return FALSE;
        }
      }
    }

    /* Check to see if they have a boat, or can swim. */
    if (!IS_IMMORTAL(ch) && ((ROOM_SECTOR_TYPE(CHAR_REAL_ROOM(ch)) == SECT_WATER_NOSWIM) || (ROOM_SECTOR_TYPE(dest_room_rnum) == SECT_WATER_NOSWIM))) {
      if (GET_MOUNT(ch) && !HAS_BOAT(GET_MOUNT(ch))) {
        send_to_char("Your mount needs a boat to go there.\n\r", ch);

        return FALSE;
      }
      else if (!HAS_BOAT(ch)) {
        send_to_char("You need a boat to go there.\n\r", ch);

        return FALSE;
      }
    }
  }

  /* Calculate encumberance factor. */
  int encumberance_factor = 1, carry_weight = IS_CARRYING_W(ch), max_carry_weight = CAN_CARRY_W(ch);

  if (carry_weight >= (3 * max_carry_weight)) {
    encumberance_factor = 6;
  }
  else if (carry_weight >= (5 * max_carry_weight) / 2) {
    encumberance_factor = 5;
  }
  else if (carry_weight >= (2 * max_carry_weight)) {
    encumberance_factor = 4;
  }
  else if (carry_weight >= (3 * max_carry_weight) / 2) {
    encumberance_factor = 3;
  }
  else if (carry_weight >= max_carry_weight) {
    encumberance_factor = 2;
  }

  int move_points_needed = encumberance_factor * (movement_loss[ROOM_SECTOR_TYPE(CHAR_REAL_ROOM(ch))] + movement_loss[ROOM_SECTOR_TYPE(dest_room_rnum)]) / 2;

  /* Handle riding. */
  if (GET_MOUNT(ch)) {
    move_points_needed = 3;

    const int BRIDLE = 3900, SADDLE = 3904;

    /* Check for bridle. */
    if ((EQ(GET_MOUNT(ch), WEAR_NECK_1) && (V_OBJ(EQ(GET_MOUNT(ch), WEAR_NECK_1)) == BRIDLE)) || (EQ(GET_MOUNT(ch), WEAR_NECK_2) && (V_OBJ(EQ(GET_MOUNT(ch), WEAR_NECK_2)) == BRIDLE))) {
      move_points_needed--;
    }

    /* Check for saddle. */
    if (EQ(GET_MOUNT(ch), WEAR_BODY) && (V_OBJ(EQ(GET_MOUNT(ch), WEAR_BODY)) == SADDLE)) {
      move_points_needed--;
    }

    move_points_needed = MAX(move_points_needed, 1);
  }

  // Prestige Perk 23
  if (GET_PRESTIGE_PERK(ch) >= 23) {
    move_points_needed = MAX(move_points_needed - 1, 1);
  }

  /* Check for exhaustion. */
  if (IS_MORTAL(ch) && (GET_MOVE(ch) < move_points_needed)) {
    if (GET_MASTER(ch)) {
      send_to_char("You are too exhausted to follow.\n\r", ch);

      return FALSE;
    }
    else {
      send_to_char("You are too exhausted.\n\r", ch);

      return FALSE;
    }
  }

  /* Druid SC3: Wall of Thorns - Blocks NPC movement and PC movement in chaotic rooms. */
  if (IS_NPC(ch) || (IS_MORTAL(ch) && (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) || ROOM_CHAOTIC(dest_room_rnum)))) {
    OBJ *thorns_here = get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch));
    OBJ *thorns_there = get_obj_room(WALL_THORNS, ROOM_VNUM(dest_room_rnum));

    if (thorns_here || thorns_there) {
      OBJ *thorns = thorns_here ? thorns_here : thorns_there;

      if (ROOM_SAFE(OBJ_IN_ROOM(thorns))) {
        send_to_char("A wall of thorns blocks your way.\n\r", ch);
      }
      else {
        send_to_char("A wall of thorns blocks your way.  Ouch!\n\r", ch);

        damage(ch, ch, MIN(GET_HIT(ch) - 1, OBJ_SPEC(thorns)), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
      }

      return FALSE;
    }
  }

  // Prestige Perk 7
  if (GET_PRESTIGE_PERK(ch) >= 7) {
    if (IS_MORTAL(ch) && (IS_SET(ROOM_FLAGS(dest_room_rnum), HAZARD) || IS_SET(ROOM_FLAGS(dest_room_rnum), DEATH)) && chance(50)) {
      send_to_char("You avoid certain death at the last instant and are momentarily paralyzed with fear.\n\r", ch);

      GET_MOVE(ch) = 0;

      return FALSE;
    }
  }

  /* Call room special prior to movement. */
  if (spec_check) {
    /* Check room special result. */
    if (special(ch, dir + 1, "")) return FALSE;

    /* Check if the room special killed the character. */
    if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return -1;
  }

  /* Deduct movement points from character. */
  if (IS_MORTAL(ch)) {
    GET_MOVE(ch) -= move_points_needed;
  }

  /* Determine movement text. */
  if (GET_POOFOUT(ch) && !IS_IMMORTAL(ch)) {
    snprintf(buf, sizeof(buf), "%s", GET_POOFOUT(ch));
  }
  else {
    char verb[MIL];

    switch (GET_POS(ch)) {
      case POSITION_FLYING:
        snprintf(verb, sizeof(verb), "flies");
        break;

      case POSITION_RIDING:
        snprintf(verb, sizeof(verb), "rides");
        break;

      case POSITION_SWIMMING:
        snprintf(verb, sizeof(verb), "swims");
        break;

      default:
        if (!IS_IMMORTAL(ch) && (GET_HIT(ch) < (hit_limit(ch) / 10))) {
          snprintf(verb, sizeof(verb), "crawls");
        }
        else {
          snprintf(verb, sizeof(verb), "leaves");
        }
        break;
    }

    snprintf(buf, sizeof(buf), "$n %s %s.", verb, dirs[dir]);
  }

  /* If sneaking, only send movement text to group; otherwise, send to room. */
  act(buf, COMM_ACT_HIDE_SUPERBRF, ch, 0, 0, IS_AFFECTED(ch, AFF_SNEAK) ? TO_GROUP : TO_ROOM);

  /* Signal the room the character is moving from. */
  if (signal_room(CHAR_REAL_ROOM(ch), ch, MSG_LEAVE, "")) return FALSE;

  /* In case MSG_LEAVE kills the character. */
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return -1;

  /* Move the character. */
  char_from_room(ch);
  char_to_room(ch, dest_room_rnum);

  do_look(ch, "", CMD_LOOK);

  /* Move their mount, if they have one. */
  if (GET_MOUNT(ch)) {
    char_from_room(GET_MOUNT(ch));
    char_to_room(GET_MOUNT(ch), dest_room_rnum);
  }

  /* Show arrival message, as approrpriate. */
  if (GET_POOFIN(ch) && (GET_LEVEL(ch) < LEVEL_IMM)) {
    snprintf(buf, sizeof(buf), "%s", GET_POOFIN(ch));
  }
  else {
    snprintf(buf, sizeof(buf), "$n has arrived.");
  }

  if (GET_RIDER(ch)) {
    for (CHAR *tmp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); tmp_ch; tmp_ch = tmp_ch->next_in_room) {
      if (CAN_SEE(tmp_ch, GET_RIDER(ch)) && (tmp_ch != GET_RIDER(ch))) {
        act(buf, COMM_ACT_HIDE_SUPERBRF, ch, 0, tmp_ch, TO_VICT);
      }
    }
  }
  else {
    /* If sneaking, only send movement text to group. Otherwise, send to room. */
    act(buf, COMM_ACT_HIDE_SUPERBRF, ch, 0, 0, (IS_AFFECTED(ch, AFF_SNEAK) ? TO_GROUP : TO_ROOM));
  }

  /* Signal the room the character just moved to. */
  if (signal_room(CHAR_REAL_ROOM(ch), ch, MSG_ENTER, "")) return FALSE;

  /* Check if signal_room killed the character.  */
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return FALSE;

  /* Check for move trap. */
  if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), MOVE_TRAP) && !IS_IMMORTAL(ch)) {
    send_to_char("\n\rYour movement points have been drained by the surroundings.\n\r", ch);

    GET_MOVE(ch) = 0;
  }

  /* Check for mana drain. */
  if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), MANA_DRAIN) && !IS_IMMORTAL(ch)) {
    send_to_char("\n\rYour mana has been drained by the surroundings.\n\r", ch);

    GET_MANA(ch) = 0;
  }

  /* Check for death trap/hazard. */
  if (dt_or_hazard(ch)) return -1;

  /* Check for trap rooms. */
  int trap_check = number(1, 110);

  if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), TRAP) &&
      (GET_POS(ch) != POSITION_FLYING) &&
      ((IS_NPC(ch) && (trap_check > (GET_LEVEL(ch) * 2))) ||
       (IS_MORTAL(ch) && IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CHAOTIC) && (trap_check > (GET_LEVEL(ch) * 2))) ||
       (IS_MORTAL(ch) && CHAOSMODE && (trap_check > GET_LEVEL(ch))))) {
    send_to_char("You fell into a trap!\n\r", ch);
    act("$n fell into a trap!", FALSE, ch, 0, 0, TO_ROOM);

    REMOVE_BIT(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), TRAP);

    GET_HIT(ch) = (4 * GET_HIT(ch)) / 5;

    return TRUE;
  }

  return TRUE;
}


void do_move(CHAR *ch, char *argument, int cmd) {
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return;

  /* Directions are 1 less than their respective commands. */
  int dir = cmd - 1;

  /* Sanity check. */
  if ((dir < NORTH) || (dir > DOWN)) return;

  /* Check for severed. */
  if (IS_SET(GET_AFF2(ch), AFF2_SEVERED)) {
    send_to_char("Move without legs?  How!?\n\r", ch);

    return;
  }

  /* Check for death timer. */
  if (GET_DEATH_TIMER(ch) == 2) {
    send_to_char("You are near certain death; you can't move!\n\r", ch);

    return;
  }

  /* Check that the exit leads to a valid room, and that it's not a special exit type. */
  if (!EXIT(ch, dir) ||
      EXIT(ch, dir)->to_room_r == NOWHERE ||
      EXIT(ch, dir)->to_room_r == real_room(0) ||
      IS_SET(EXIT(ch, dir)->exit_info, EX_CLIMB) ||
      IS_SET(EXIT(ch, dir)->exit_info, EX_JUMP) ||
      IS_SET(EXIT(ch, dir)->exit_info, EX_CRAWL) ||
      IS_SET(EXIT(ch, dir)->exit_info, EX_ENTER)) {
    send_to_char("Alas, you cannot go that way...\n\r", ch);

    return;
  }

  /* Check for charm. */
  if (IS_AFFECTED(ch, AFF_CHARM) && GET_MASTER(ch) && CHAR_REAL_ROOM(ch) == CHAR_REAL_ROOM(GET_MASTER(ch))) {
    send_to_char("The thought of leaving your master makes you weep.\n\r", ch);
    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);

    return;
  }

  /* Check for mount. */
  if (IS_NPC(ch) && GET_RIDER(ch) && CHAR_REAL_ROOM(ch) == CHAR_REAL_ROOM(GET_RIDER(ch))) {
    send_to_char("You don't want to leave your master.\n\r", ch);

    return;
  }

  /* Check if the exit is closed. */
  if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)) {
    if (EXIT(ch, dir)->keyword) {
      printf_to_char(ch, "The %s seems to be closed.\n\r", fname(EXIT(ch, dir)->keyword));
    }
    else {
      send_to_char("It seems to be closed.\n\r", ch);
    }

    return;
  }

  /* Check for tunnel. */
  if (IS_MORTAL(ch) && IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room_r), TUNNEL) && (count_mortals_real_room(EXIT(ch, dir)->to_room_r) > 0) && !CHAOSMODE) {
    send_to_char("It's too narrow to go there.\n\r", ch);

    return;
  }

  /* Record the original room. */
  int orig_room_rnum = CHAR_REAL_ROOM(ch);

  /* Move the character. */
  if (move_char(ch, dir, TRUE) == TRUE) {
    /* Move the character's followers. */
    for (FOL *temp_follower = ch->followers, *next_follower = NULL; temp_follower; temp_follower = next_follower) {
      next_follower = temp_follower->next;

      if ((CHAR_REAL_ROOM(temp_follower->follower) == orig_room_rnum) && (GET_POS(temp_follower->follower) >= POSITION_STANDING)) {
        act("You follow $N.\n\r", FALSE, temp_follower->follower, 0, ch, TO_CHAR);

        do_move(temp_follower->follower, argument, cmd);
      }
    }
  }
}

void look_in_room(CHAR *ch, int vnum);

void do_peek(struct char_data *ch, char *argument, int cmd) {
  const char *keywords[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "",
    "\n"
  };

  if (!ch || !ch->desc || !ch->skills) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_AVATAR) &&
      (GET_CLASS(ch) != CLASS_BARD)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You can't see a thing; you're blind!\n\r", ch);

    return;
  }

  if (!IS_IMMORTAL(ch) && IS_DARK(CHAR_REAL_ROOM(ch)) && !IS_AFFECTED(ch, AFF_INFRAVISION)) {
    send_to_char("You're surrounded by darkness...\n\r", ch);

    return;
  }

  char arg[MSL];

  one_argument(argument, arg);

  int keyword_idx = search_block(arg, keywords, FALSE);

  switch (keyword_idx) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    if (EXIT(ch, keyword_idx) &&
        world[CHAR_REAL_ROOM(ch)].dir_option[keyword_idx]->to_room_r != -1 &&
        world[CHAR_REAL_ROOM(ch)].dir_option[keyword_idx]->to_room_r != real_room(0) &&
        !IS_SET(EXIT(ch, keyword_idx)->exit_info, EX_CLIMB) &&
        !IS_SET(EXIT(ch, keyword_idx)->exit_info, EX_JUMP) &&
        !IS_SET(EXIT(ch, keyword_idx)->exit_info, EX_CRAWL) &&
        !IS_SET(EXIT(ch, keyword_idx)->exit_info, EX_ENTER) &&
        !IS_SET(EXIT(ch, keyword_idx)->exit_info, EX_CLOSED)) {
      send_to_char("You try to peek.\n\r", ch);

      if (number(1, 85) > GET_LEARNED(ch, SKILL_PEEK)) return;

      if (IS_MORTAL(ch) && IS_SET(world[world[CHAR_REAL_ROOM(ch)].dir_option[keyword_idx]->to_room_r].room_flags, NO_PEEK)) {
        send_to_char("Something blocks your vision.\n\r", ch);

        return;
      }

      look_in_room(ch, world[world[CHAR_REAL_ROOM(ch)].dir_option[keyword_idx]->to_room_r].number);
    }
    else {
      send_to_char("Nothing special there...\n\r", ch);
    }
    break;

    case 6:
      send_to_char("You must choose a direction.\n\r", ch);
      break;

    case -1:
      send_to_char("Peek where?\n\r", ch);
      break;
  }
}

int find_door(struct char_data *ch, char *type, char *dir) {
  const char *dirs[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "\n"
  };

  if (*dir) {
    /* a direction was specified */
    int door_idx = search_block(dir, dirs, FALSE);

    if (door_idx == -1) {
      send_to_char("That's not a direction.\n\r", ch);

      return -1;
    }

    if (EXIT(ch, door_idx)) {
      if (EXIT(ch, door_idx)->keyword) {
        if (isname(type, EXIT(ch, door_idx)->keyword)) {
          return door_idx;
        }
        else {
          printf_to_char(ch, "I see no %s there.\n\r", type);

          return -1;
        }
      }
      else {
        return door_idx;
      }
    }
    else {
      send_to_char("I really don't see how you can close anything there.\n\r", ch);

      return -1;
    }
  }
  else {
    /* try to locate the keyword */
    for (int door = NORTH; door <= DOWN; door++) {
      if (EXIT(ch, door) && EXIT(ch, door)->keyword && isname(type, EXIT(ch, door)->keyword)) {
        return door;
      }
    }

    printf_to_char(ch, "I see no %s here.\n\r", type);

    return -1;
  }
}


void do_open(struct char_data *ch, char *argument, int cmd)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct room_direction_data *back;
  struct obj_data *obj;
  struct char_data *victim;

  argument_interpreter(argument, type, dir);

  if (!*type)
    send_to_char("Open what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
                  ch, &victim, &obj))

    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("But it's already open!\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
      send_to_char("You can't do that.\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("It seems to be locked.\n\r", ch);
    else
      {
      REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
      send_to_char("Ok.\n\r", ch);
      act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  else if ((door = find_door(ch, type, dir)) >= 0) {
    /* perhaps it is a door */
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's impossible, I'm afraid.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("It's already open!\n\r", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("It seems to be locked.\n\r", ch);
    else {
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
      if (EXIT(ch, door)->keyword)
        act("$n opens the $F.", FALSE, ch, 0, EXIT(ch, door)->keyword,TO_ROOM);
      else
        act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Ok.\n\r", ch);
      /* now for opening the OTHER side of the door! */
      if ((other_room = EXIT(ch, door)->to_room_r) != NOWHERE)
        if ((back = world[other_room].dir_option[rev_dir[door]]))
          if (back->to_room_r == CHAR_REAL_ROOM(ch)) {
            REMOVE_BIT(back->exit_info, EX_CLOSED);
            if(back->keyword) {
              sprintf(buf,"The %s is opened from the other side.\n\r",fname(back->keyword));
              send_to_room(buf, EXIT(ch, door)->to_room_r);
            }
            else
              send_to_room("The door is opened from the other side.\n\r",EXIT(ch, door)->to_room_r);
          }
    }
  }
}


void do_close(struct char_data *ch, char *argument, int cmd)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct room_direction_data *back;
  struct obj_data *obj;
  struct char_data *victim;


  argument_interpreter(argument, type, dir);

  if (!*type)
    send_to_char("Close what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
                  ch, &victim, &obj))

    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("But it's already closed!\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
      send_to_char("That's impossible.\n\r", ch);
    else
      {
      SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
      send_to_char("Ok.\n\r", ch);
      act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  else if ((door = find_door(ch, type, dir)) >= 0) {
    /* Or a door */
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("It's already closed!\n\r", ch);
    else {
      SET_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
      if (EXIT(ch, door)->keyword)
        act("$n closes the $F.", 0, ch, 0, EXIT(ch, door)->keyword,TO_ROOM);
      else
        act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Ok.\n\r", ch);
      /* now for closing the other side, too */
      if ((other_room = EXIT(ch, door)->to_room_r) != NOWHERE)
        if ((back = world[other_room].dir_option[rev_dir[door]]))
          if (back->to_room_r == CHAR_REAL_ROOM(ch)) {
            SET_BIT(back->exit_info, EX_CLOSED);
            if (back->keyword) {
                sprintf(buf,"The %s closes quietly.\n\r", back->keyword);
                send_to_room(buf, EXIT(ch, door)->to_room_r);
            }
            else
              send_to_room("The door closes quietly.\n\r",EXIT(ch, door)->to_room_r);
          }
    }
  }
}


#define KEY_RING 16539

bool has_key(CHAR *ch, int key_vnum) {
  if (!ch || !key_vnum) return FALSE;

  if (EQ(ch, HOLD) && (V_OBJ(EQ(ch, HOLD)) == key_vnum)) {
    return TRUE;
  }

  for (OBJ *temp_obj = GET_CARRYING(ch); temp_obj; temp_obj = OBJ_NEXT_CONTENT(temp_obj)) {
    if (V_OBJ(temp_obj) == key_vnum) {
      return TRUE;
    }
  }

  return FALSE;
}


OBJ *find_keychain(CHAR* ch) {
  if (!ch) return NULL;

  if (EQ(ch, HOLD) && (V_OBJ(EQ(ch, HOLD)) == KEY_RING)) {
    return EQ(ch, HOLD);
  }

  return get_obj_in_list_num(real_object(KEY_RING), GET_CARRYING(ch));
}


bool keys_from_ring(OBJ *chain, char *argument, int cmd) {
  if (!chain || (V_OBJ(chain) != KEY_RING)) return FALSE;

  CHAR *owner = OBJ_EQUIPPED_BY(chain) ? OBJ_EQUIPPED_BY(chain) : OBJ_CARRIED_BY(chain) ? OBJ_CARRIED_BY(chain) : NULL;

  if (owner) {
    for (OBJ *temp_obj = OBJ_CONTAINS(chain), *next_obj; temp_obj; temp_obj = next_obj) {
      next_obj = OBJ_NEXT_CONTENT(temp_obj);

      obj_from_obj(temp_obj);
      obj_to_char(temp_obj, owner);

      if (signal_object(temp_obj, owner, cmd, argument)) {
        return TRUE;
      }
    }
  }

  return FALSE;
}


bool keys_to_ring(OBJ *chain) {
  bool fiddled = FALSE;

  if (!chain || (V_OBJ(chain) != KEY_RING)) return FALSE;

  CHAR *owner = OBJ_EQUIPPED_BY(chain) ? OBJ_EQUIPPED_BY(chain) : OBJ_CARRIED_BY(chain) ? OBJ_CARRIED_BY(chain) : NULL;

  if (owner) {
    for (OBJ *temp_obj = GET_CARRYING(owner), *next_obj; temp_obj; temp_obj = next_obj) {
      next_obj = OBJ_NEXT_CONTENT(temp_obj);

      if (OBJ_TYPE(temp_obj) == ITEM_KEY) {
        fiddled = TRUE;

        obj_from_char(temp_obj);
        obj_to_obj(temp_obj, chain);
      }
    }
  }

  return fiddled;
}


void do_lock(struct char_data *ch, char *argument, int cmd)
{
  int door, other_room;
  char buf[MAX_INPUT_LENGTH], type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back;
  struct obj_data *obj;
  struct char_data *victim;

  if(CHAOSMODE) {
    send_to_char("You feel too chaotic to lock anything!\n\r",ch);
    return;
  }

  OBJ *chain = find_keychain( ch );

  if( chain )
    if( keys_from_ring( chain, argument, cmd ) )
      goto cleanup;

  argument_interpreter(argument, type, dir);

  if (!*type)
    send_to_char("Lock what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
                  ch, &victim, &obj))

    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Maybe you should close it first...\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("That thing can't be locked.\n\r", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("You don't seem to have the proper key.\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("It is locked already.\n\r", ch);
    else
      {
      if (GET_LEVEL(ch) >= LEVEL_IMM) {
        sprintf (buf, "WIZINFO: %s locked %s", GET_NAME(ch), OBJ_SHORT(obj));
        wizlog(buf, GET_LEVEL(ch)+1, 5);
        log_s(buf);
      }
      SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act("$n locks $p - 'click', it says.", FALSE, ch, obj, 0, TO_ROOM);
      }
  else if ((door = find_door(ch, type, dir)) >= 0) {
    /* a door, perhaps */
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("You have to close it first, I'm afraid.\n\r", ch);
    else if (EXIT(ch, door)->key < 0)
      send_to_char("You can't find any keyholes.\n\r", ch);
    else if (!has_key(ch, EXIT(ch, door)->key))
      send_to_char("You don't have the proper key.\n\r", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("It's already locked!\n\r", ch);
    else {
      if (GET_LEVEL(ch) >= LEVEL_IMM) {
        sprintf (buf, "WIZINFO: %s locked a door at %d",
               GET_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
        wizlog(buf, GET_LEVEL(ch)+1, 5);
        log_s(buf);
      }
      SET_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
        act("$n locks the $F.", 0, ch, 0,  EXIT(ch, door)->keyword,
            TO_ROOM);
      else
        act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*Click*\n\r", ch);
      /* now for locking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room_r) != NOWHERE)
        if ((back = world[other_room].dir_option[rev_dir[door]]))
          if (back->to_room_r == CHAR_REAL_ROOM(ch))
            SET_BIT(back->exit_info, EX_LOCKED);
    }
  }

cleanup:
  if( chain )
    keys_to_ring( chain );
}


void do_unlock(struct char_data *ch, char *argument, int cmd)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  char buf[250];
  struct room_direction_data *back;
  struct obj_data *obj;
  struct char_data *victim;

  OBJ *chain = find_keychain( ch );

  if( chain )
    if( keys_from_ring( chain, argument, cmd ) )
      goto cleanup;

  argument_interpreter(argument, type, dir);

  if (!*type)
    send_to_char("Unlock what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
                  ch, &victim, &obj))

    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Silly - it ain't even closed!\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("You don't seem to have the proper key.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all.\n\r", ch);
    else
      {
      if (GET_LEVEL(ch) >= LEVEL_IMM) {
        sprintf (buf, "WIZINFO: %s unlocked %s", GET_NAME(ch), OBJ_SHORT(obj));
        wizlog(buf, GET_LEVEL(ch)+1, 5);
        log_s(buf);
      }
      REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  else if ((door = find_door(ch, type, dir)) >= 0) {
    /* it is a door */
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("Heck.. it ain't even closed!\n\r", ch);
    else if (EXIT(ch, door)->key < 0)
      send_to_char("You can't seem to spot any keyholes.\n\r", ch);
    else if (!has_key(ch, EXIT(ch, door)->key))
      send_to_char("You do not have the proper key for that.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("It's already unlocked, it seems.\n\r", ch);
    else {
      if (GET_LEVEL(ch) >= LEVEL_IMM) {
        sprintf (buf, "WIZINFO: %s unlocked a door at %d",
               GET_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
        wizlog(buf, GET_LEVEL(ch)+1, 5);
        log_s(buf);
      }
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
        act("$n unlocks the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
            TO_ROOM);
      else
        act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*click*\n\r", ch);
      /* now for unlocking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room_r) != NOWHERE)
        if ((back = world[other_room].dir_option[rev_dir[door]]))
          if (back->to_room_r == CHAR_REAL_ROOM(ch))
            REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
  }

cleanup:
  if( chain )
    keys_to_ring( chain );
}

void do_pick(struct char_data *ch, char *argument, int cmd) {
  byte percent;
  int door, other_room,row,col=0;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  char buf[250];
  struct room_direction_data *back;
  struct obj_data *obj;
  struct char_data *victim;

              /* Lock Level  PICK  NOP 10  15  20  25  30   Player Lvl*/
  int pick_percent[12][7] = { {100,  0, 20, 10,  5,  2,  1},  /*  1-4   */
                              {100,  0, 30, 20, 10,  5,  2},  /*  5-9   */
                              {100,  0, 50, 30, 20, 10,  5},  /* 10-14  */
                              {100,  0, 55, 50, 30, 20, 10},  /* 15-19  */
                              {100,  0, 60, 55, 50, 30, 20},  /* 20-24  */
                              {100,  0, 65, 60, 55, 50, 30},  /* 25-29  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 30-34  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 35-39  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 40-44  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 45-49  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 50-54  */
                              {100,100,100,100,100,100,100} };/* 55-IMP */

  if(!ch->skills) return;

  if ((GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_BARD) &&
      (GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_AVATAR) &&
      (GET_LEVEL(ch) < LEVEL_IMM) && (!CHAOSMODE)) { /* if CHAOSMODE, allow anyone to pick, for now */
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  argument_interpreter(argument, type, dir);

  /* Redid entire logic on pick and added lock levels - Ranger July 96 */

  percent=number(1,101); /* 101% is a complete failure */

  if (!*type) {
    send_to_char("Pick what?\n\r", ch);
    return;
  }

  /* If object */
  if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {

    if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
      send_to_char("That's not a container.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
      send_to_char("Silly - it ain't even closed!\n\r", ch);
      return;
    }
    if (obj->obj_flags.value[2] < 0) {
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
      send_to_char("Oho! This thing is NOT locked!\n\r", ch);
      return;
    }
    if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
      send_to_char("Your skills don't meet the challenge.\n\r", ch);
      return;
    }
    if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF) || (GET_LEVEL(ch) < LEVEL_IMP)) {
      send_to_char("It resists your attempts at picking it.\n\r", ch);
      return;
    }

    if (GET_LEVEL(ch) >= LEVEL_IMM) {
      sprintf (buf, "WIZINFO: %s picked %s", GET_NAME(ch), OBJ_SHORT(obj));
      wizlog(buf, GET_LEVEL(ch), 5);
      log_s(buf);
    }

    REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
    send_to_char("*Click*\n\r", ch);
    act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
    return;
  }


  /* For Doors */
  if ((door = find_door(ch, type, dir)) >= 0) {

    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
      send_to_char("That's absurd.\n\r", ch);
      return;
    }
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      send_to_char("You realize that the door is already open.\n\r", ch);
      return;
    }
    if (EXIT(ch, door)->key < 0) {
      send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
      return;
    }
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
      send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
      return;
    }
    if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
      send_to_char("Your skills don't meet the challenge.\n\r", ch);
      return;
    }

    row=GET_LEVEL(ch)/5;

    if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)) col=1;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_10))   col=2;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_15))   col=3;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_20))   col=4;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_25))   col=5;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_30))   col=6;

    if(col>1) {
      if (ch->equipment[HOLD]) {
         if (ch->equipment[HOLD]->obj_flags.type_flag == ITEM_LOCKPICK) {
           obj=ch->equipment[HOLD];
         }
         else {
           send_to_char("You need a lockpick to open this lock.\n\r",ch);
           return;
         }
      } else {
         send_to_char("You need a lockpick to open this lock.\n\r",ch);
         return;
      }
    }


    if (number(1,100)<=pick_percent[row][col]) {

      if (GET_LEVEL(ch) >= LEVEL_IMM) {
         sprintf (buf, "WIZINFO: %s picked a door at %d", GET_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
         wizlog(buf, GET_LEVEL(ch), 5);
         log_s(buf);
      }

      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
        act("$n skillfully picks the lock of the $F.", 0, ch, 0,
            EXIT(ch, door)->keyword, TO_ROOM);
      else
        act("$n picks the lock of the.", TRUE, ch, 0, 0, TO_ROOM);

      send_to_char("The lock quickly yields to your skills.\n\r", ch);
      if(col>1) {
         obj->obj_flags.value[0]=obj->obj_flags.value[1];
         send_to_char("The lockpick restores itself!\n\r",ch);
      }

      /* now for unlocking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room_r) != NOWHERE)
         if ((back = world[other_room].dir_option[rev_dir[door]]))
          if (back->to_room_r == CHAR_REAL_ROOM(ch))
            REMOVE_BIT(back->exit_info, EX_LOCKED);
      return;
    }

    if (col==1) {
      send_to_char("It resists your attempts at picking it.\n\r",ch);
      return;
    }

    if (col>1) {
      send_to_char("You were unable to pick the lock.\n\r",ch);
      obj->obj_flags.value[0]=obj->obj_flags.value[0]-1;
      if(obj->obj_flags.value[0]<=0) {
        send_to_char("The lockpick disintegrates in your hand.\n\r",ch);
        unequip_char (obj->equipped_by, HOLD);
        extract_obj(obj);
      } else send_to_char("Your lockpick becomes a little weaker.\n\r",ch);
      return;
    }
  } /* if door */
}

void do_knock(struct char_data *ch, char *argument, int cmd) {
  byte percent;
  int door, other_room,row,col=0;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  char buf[250];
              /* Lock Level  PICK  NOP 10  15  20  25  30   Player Lvl*/
  int knock_percent[12][7] = { {100,  0, 20, 10,  5,  2,  1},  /*  1-4   */
                              {100,  0, 30, 20, 10,  5,  2},  /*  5-9   */
                              {100,  0, 50, 30, 20, 10,  5},  /* 10-14  */
                              {100,  0, 55, 50, 30, 20, 10},  /* 15-19  */
                              {100,  0, 60, 55, 50, 30, 20},  /* 20-24  */
                              {100,  0, 65, 60, 55, 50, 30},  /* 25-29  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 30-34  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 35-39  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 40-44  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 45-49  */
                              {100,  0, 70, 65, 60, 55, 50},  /* 50-54  */
                              {100,100,100,100,100,100,100} };/* 55-IMP */

  struct room_direction_data *back;

  /* Added lock levels - redid logic - Ranger July 96 */

  if(!ch->skills)
    return;

  argument_interpreter(argument, type, dir);

  percent=number(1,101); /* 101% is a complete failure */

  if ((GET_CLASS(ch) != CLASS_WARRIOR) && (GET_LEVEL(ch) < LEVEL_IMM))
    {
      send_to_char("Leave the hard job to the warrior.\n\r", ch);
      return;
    }

  if (GET_MOVE(ch) < 30)
    {
      send_to_char("You are too tired, take some rests first.\n\r", ch);
      return;
    }

  if (!*type) {
    send_to_char("Knock what?\n\r", ch);
    return;
  }

  if ((door = find_door(ch, type, dir)) >= 0) {

     if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
      send_to_char("That's absurd.\n\r", ch);
        return;
     }
     if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      send_to_char("You realize that the door is already open.\n\r", ch);
        return;
     }
     if (EXIT(ch, door)->key < 0) {
      send_to_char("You knock on the door very hard, but nothing seems to happen.\n\r", ch);
        return;
     }
     if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
      send_to_char("You knock on the door very hard, but nothing seems to happen.\n\r", ch);
        return;
     }
     if (percent > (ch->skills[SKILL_KNOCK].learned)) {
       send_to_char("Your skills were not up to the challenge.\n\r", ch);
       if (GET_MOVE(ch) > 20)
         GET_MOVE(ch) -= 20;
       else
         GET_MOVE(ch) = 0;
       return;
     }

    row=GET_LEVEL(ch)/5;

    if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)) col=1;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_10))   col=2;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_15))   col=3;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_20))   col=4;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_25))   col=5;
    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCK_30))   col=6;

/*    sprintf(buf,"col=%d  row=%d  knock per=%d\n\r",col,row,knock_percent[row][col]);
    send_to_char(buf,ch);*/

    if (number(1,100)<=knock_percent[row][col]) {
        if (GET_LEVEL(ch) > LEVEL_IMM) {
          sprintf (buf, "WIZINFO: %s knocked a door open at %d",
                 GET_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
          wizlog(buf, GET_LEVEL(ch)+1, 5);
          log_s(buf);
        }

        REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
        if (ch->skills[SKILL_KNOCK].learned < 80)
          ch->skills[SKILL_KNOCK].learned += 2;

        if (EXIT(ch, door)->keyword)
          act("$n knocks on the door loudly and breaks the lock of the $F.", 0, ch, 0,
            EXIT(ch, door)->keyword, TO_ROOM);
        else
          act("$n breaks the lock.", TRUE, ch, 0, 0, TO_ROOM);
        send_to_char("You break the lock.\n\r", ch);
        /* now for unlocking the other side, too */
        if ((other_room = EXIT(ch, door)->to_room_r) != NOWHERE)
          if ((back = world[other_room].dir_option[rev_dir[door]]))
            if (back->to_room_r == CHAR_REAL_ROOM(ch))
            REMOVE_BIT(back->exit_info, EX_LOCKED);

        if (GET_MOVE(ch) > 30) GET_MOVE(ch) -= 30;
        else       GET_MOVE(ch) = 0;

        return;
    }
    if (col==1) send_to_char("You realize it is impossible to break this lock.\n\r",ch);
    else send_to_char("You were unable to break the lock.\n\r",ch);
  }
}

void do_enter(struct char_data *ch, char *argument, int cmd) {
  int door;
  char buf[MAX_INPUT_LENGTH], tmp[MAX_STRING_LENGTH];

  void do_move(struct char_data *ch, char *argument, int cmd);

  one_argument(argument, buf);

  if (*buf)  /* an argument was supplied, search for door keyword */
    {
      for (door = 0; door <= 5; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->keyword)
          if (!str_cmp(EXIT(ch, door)->keyword, buf))
            {
            if(IS_SET(EXIT(ch, door)->exit_info, EX_ENTER))
              do_special_move(ch, buf, CMD_ENTER);
            else
              do_move(ch, "", ++door);
            return;
            }
      sprintf(tmp, "There is no %s here.\n\r", buf);
      send_to_char(tmp, ch);
    }
  else

    if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, INDOORS))
      send_to_char("You are already indoors.\n\r", ch);
    else
      {
      /* try to locate an entrance */
      for (door = 0; door <= 5; door++)
        if (EXIT(ch, door))
          if (EXIT(ch, door)->to_room_r != NOWHERE)
            if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
              IS_SET(world[EXIT(ch, door)->to_room_r].room_flags,
                   INDOORS))
            {
              do_move(ch, "", ++door);
              return;
            }
      send_to_char("You can't seem to find anything to enter.\n\r", ch);
      }
}


void do_leave(struct char_data *ch, char *argument, int cmd)
{
  int door;

  void do_move(struct char_data *ch, char *argument, int cmd);

  if (!IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, INDOORS))
    send_to_char("You are outside.. where do you want to go?\n\r", ch);
  else
    {
      for (door = 0; door <= 5; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->to_room_r != NOWHERE)
          if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
            !IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, INDOORS))
            {
            do_move(ch, "", ++door);
            return;
            }
      send_to_char("I see no obvious exits to the outside.\n\r", ch);
    }
}


void do_stand(struct char_data *ch, char *argument, int cmd)
{

  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    act("You are already standing.",FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_SITTING      : {
    act("You stand up.", FALSE, ch,0,0,TO_CHAR);
    act("$n clambers on $s feet.",TRUE, ch, 0, 0, TO_ROOM);
    if (IS_AFFECTED(ch, AFF_FLY))
      GET_POS(ch) = POSITION_FLYING;
    else
      GET_POS(ch) = POSITION_STANDING;
  } break;
  case POSITION_RESTING      : {
    act("You stop resting, and stand up.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    if (IS_AFFECTED(ch, AFF_FLY))
      GET_POS(ch) = POSITION_FLYING;
    else
      GET_POS(ch) = POSITION_STANDING;
  } break;
  case POSITION_SLEEPING : {
    act("You have to wake up first!", FALSE, ch, 0,0,TO_CHAR);
  } break;
  case POSITION_FIGHTING : {
    act("Do you not consider fighting as standing?",FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_FLYING : {
    act("You are flying.",FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_RIDING :{
    act("You must dismount first.", FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_SWIMMING :{
    act("But you are swimming!", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  default : {
    act("You stop floating around, and put your feet on the ground.",
      FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and puts $s feet on the ground.",
      TRUE, ch, 0, 0, TO_ROOM);
  } break;
  }
}


void do_sit(struct char_data *ch, char *argument, int cmd)
{

  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    act("You sit down.", FALSE, ch, 0,0, TO_CHAR);
    act("$n sits down.", FALSE, ch, 0,0, TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  case POSITION_SITTING      : {
    send_to_char("You'r sitting already.\n\r", ch);
  } break;
  case POSITION_RESTING      : {
    act("You stop resting, and sit up.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops resting.", TRUE, ch, 0,0,TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  case POSITION_SLEEPING : {
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_FIGHTING : {
    act("Sit down while fighting? are you alright?", FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_FLYING : {
    act("You stop flying and sit down.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops flying and sits down.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SITTING; /* Ranger April 96 */
  } break;
  case POSITION_RIDING :{
    act("You must dismount first.", FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_SWIMMING :{
    act("But you are swimming!", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  default : {
    act("You stop floating around, and sit down.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops floating around, and sits down.", TRUE, ch,0,0,TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  }
}


void do_rest(struct char_data *ch, char *argument, int cmd) {

  if(ch->specials.fighting) {
    act("Rest while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
  } break;
  case POSITION_SITTING : {
    act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
  } break;
  case POSITION_RESTING : {
    act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_SLEEPING : {
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_FIGHTING : {
    act("Rest while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_FLYING : {
    act("You stop flying and rest.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops flying and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
  } break;
  case POSITION_RIDING :{
    act("You must dismount first.", FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_SWIMMING :{
    act("But you are swimming!", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  default : {
    act("You stop floating around, and stop to rest your tired bones.",
      FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and rests.", FALSE, ch, 0,0, TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  }
}


void do_sleep(struct char_data *ch, char *argument, int cmd) {

  switch(GET_POS(ch)) {
  case POSITION_STANDING :
  case POSITION_SITTING  :
  case POSITION_RESTING  : {
    send_to_char("You go to sleep.\n\r", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;
  } break;
  case POSITION_SLEEPING : {
    send_to_char("You are already sound asleep.\n\r", ch);
  } break;
  case POSITION_FIGHTING : {
    send_to_char("Sleep while fighting? are you alright?\n\r", ch);
  } break;
  case POSITION_FLYING : {
    act("You stop flying and go to sleep.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops flying and lies down to sleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;
  } break;
  case POSITION_RIDING :{
    act("You must dismount first.", FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_SWIMMING :{
    act("But you are swimming!", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  default : {
    act("You stop floating around, and lie down to sleep.",
      FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and lie down to sleep.",
      TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;
  } break;
  }
}


void do_wake(struct char_data *ch, char *argument, int cmd) {
  char buf[MIL];

  one_argument(argument, buf);

  if (!(*buf)) {
    if (IS_AFFECTED(ch, AFF_SLEEP)) {
      send_to_char("You can't wake up!\n\r", ch);

      return;
    }

    if (GET_POS(ch) != POSITION_SLEEPING) {
      send_to_char("You are already awake...\n\r", ch);

      return;
    }

    AFF *meditate = get_affect_from_char(ch, SKILL_MEDITATE);

    if (meditate && meditate->duration >= 10) {
      meditate->duration = 9;

      send_to_char("You wake from your meditation, and sit up.\n\r", ch);
      act("$n awakens from $s meditation.", TRUE, ch, 0, 0, TO_ROOM);
    }
    else {
      send_to_char("You wake, and sit up.\n\r", ch);
      act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    }

    GET_POS(ch) = POSITION_SITTING;
  }
  else {
    CHAR *victim = get_char_room_vis(ch, buf);

    if (victim != ch && GET_POS(ch) == POSITION_SLEEPING) {
      send_to_char("You can't wake someone up if you are asleep yourself!\n\r", ch);

      return;
    }

    if (victim == ch) {
      do_wake(ch, "", CMD_WAKE);

      return;
    }

    if (!victim) {
      send_to_char("You do not see that person here.\n\r", ch);

      return;
    }

    if (!IS_IMMORTAL(ch) && ((IS_AFFECTED(victim, AFF_SLEEP) && !ROOM_SAFE(CHAR_REAL_ROOM(ch))) || (affected_by_spell(victim, SKILL_MEDITATE) && (duration_of_spell(victim, SKILL_MEDITATE) >= 10)))) {
      act("You can't wake $M up!", FALSE, ch, 0, victim, TO_CHAR);

      return;
    }

    if (GET_POS(victim) != POSITION_SLEEPING) {
      act("$N is already awake.", FALSE, ch, 0, victim, TO_CHAR);

      return;
    }

    if (IS_AFFECTED(victim, AFF_SLEEP)) {
      affect_from_char(victim, SPELL_SLEEP);
    }

    act("You wake $M up.", FALSE, ch, 0, victim, TO_CHAR);
    act("You are awakened by $n.", FALSE, ch, 0, victim, TO_VICT);

    GET_POS(victim) = POSITION_SITTING;
  }
}


void do_follow(struct char_data *ch, char *argument, int cmd)
{
  char name[160];
  struct char_data *leader;

  void add_follower(struct char_data *ch, struct char_data *leader);

  if(!IS_NPC(ch) &&
     (CHAOSMODE || IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))) {
    send_to_char("You feel too chaotic to follow anyone.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if (*name) {
    if (!(leader = get_char_room_vis(ch, name))) {
      send_to_char("I see no person by that name here!\n\r", ch);
      return;
    }
  } else {
    send_to_char("Who do you wish to follow?\n\r", ch);
    return;
  }

/*  if (GET_POS(leader)== POSITION_RIDING) {
    send_to_char("You can't follow a person who is riding.\n\r", ch);
    return;
  } Want leader to be able to mount a stable mob - Ranger */

  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {

    act("But you only feel like following $N!",
      FALSE, ch, 0, ch->master, TO_CHAR);

  } else { /* Not Charmed follow person */

    if (leader == ch) {
      if (!ch->master) {
      send_to_char("You are already following yourself.\n\r",
                 ch);
      return;
      }
      stop_follower(ch);
    } else {
      if (circle_follow(ch, leader)) {
      act("Sorry, but following in 'loops' is not allowed", FALSE, ch, 0, 0, TO_CHAR);
      return;
      }
      if (ch->master)
      stop_follower(ch);

      add_follower(ch, leader);
      if (IS_AFFECTED(ch, AFF_GROUP))
      REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);

    }
  }
}

void do_subdue(struct char_data *ch, char *argument, int cmd)
{
  char arg[100];
  struct char_data *victim;
  int percent;

  if(!ch->skills)
    return;

  if ((GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_LEVEL(ch) < LEVEL_IMM)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  one_argument(argument,arg);
  if (*arg) {

    if (GET_MOVE(ch) < 30 ) {
      send_to_char("You are too tired to subdue anything.\n\r", ch);
      return;
    }

    victim = get_char_room_vis(ch,arg);
    if (victim) {
      if (!IS_NPC(victim)) {
      send_to_char("You can't subdue another player!\n\r",ch);
      return;
      }
      if(!IS_AFFECTED(victim, AFF_SUBDUE))
      {
        send_to_char("You can't subdue that!\n\r",ch);
        return;
      }

      percent = number(1,130);

      if (GET_LEVEL(victim) > GET_LEVEL(ch) || (percent > (ch->skills[SKILL_SUBDUE].learned)))
      {
        send_to_char("You fail.\n\r",ch);
        GET_MOVE(ch)-=30;
        return;
      }
      if (IS_SET(victim->specials.act, ACT_SUBDUE))
      {
        send_to_char("It's already been subdued!\n\r",ch);
        return;
      }
      else
      {
        act("You subdue $N.",FALSE,ch,0,victim,TO_CHAR);
        act("$n subdues $N.",FALSE,ch,0,victim,TO_NOTVICT);
        GET_MOVE(ch)-=30;
        SET_BIT(victim->specials.act, ACT_SUBDUE);
        if (ch->skills[SKILL_SUBDUE].learned < 85)
          ch->skills[SKILL_SUBDUE].learned += 2;
      }
    }
    else
      send_to_char("It is not here.\n\r",ch);
  }
  else
    send_to_char("Subdue who?\n\r",ch);
}


void stop_riding(CHAR *ch, CHAR *victim) {
  GET_POS(ch) = POSITION_STANDING;

  GET_MOUNT(ch) = 0;

  if (victim) {
    GET_RIDER(victim) = 0;

    if (!IS_AFFECTED(victim, AFF_CHARM)) {
      stop_follower(victim);
    }
  }
}

void do_ride(struct char_data *ch, char *argument, int cmd)
{
  char arg[100];
  struct char_data *victim;
  void add_follower(struct char_data *ch,struct char_data *leader);

  one_argument(argument,arg);

  if (GET_POS(ch)==POSITION_RIDING) {
    send_to_char("You are already riding something.\n\r",ch);
    return;
  }
  if (*arg) {
    victim=get_char_room_vis(ch,arg);
    if (victim) {
      if (!IS_NPC(victim)) {
      send_to_char("You can't ride PC.\n",ch);
      return;
      }
      if (!IS_SET(victim->specials.act, ACT_SUBDUE)) {
      send_to_char("It's still a wild creature.\n\r",ch);
      hit(victim, ch, 0);
      return;
      }

      if (victim->master != ch && victim->master != 0) {
      send_to_char("You can't ride other player's pet.\n\r",ch);
      return;
      }

      /* New if for stables - Ranger APril 96 */
      if ((GET_LEVEL(victim) > GET_LEVEL(ch)) ||
        (!IS_SET(victim->specials.act, ACT_MOUNT) &&
           (GET_CLASS(ch) != CLASS_NOMAD))) {
      send_to_char("It refuses to carry you.\n\r",ch);
      return;
      } else {
      act("You start riding $N.",FALSE,ch,0,victim,TO_CHAR);
      act("$n starts riding $N.",FALSE,ch,0,victim,TO_NOTVICT);
      GET_POS(ch)=POSITION_RIDING;
      ch->specials.riding=victim;
      if(victim->master && !IS_AFFECTED(victim, AFF_CHARM)) stop_follower(victim);
      if(!IS_AFFECTED(victim, AFF_CHARM)) add_follower(victim,ch);
      victim->specials.rider=ch;
      return;
      }
    } else
      send_to_char("Thats not here.\n\r",ch);
  }
  else
    send_to_char("Ride what?\n\r",ch);
}

/* A whole new command for stables - Ranger April 96 */
void do_free(struct char_data *ch, char *argument, int cmd) {
  char arg[MIL];
  struct char_data *victim=NULL;
  struct follow_type *k;
  int org_room;

  one_argument(argument, arg);

  if (GET_POS(ch) == POSITION_RIDING)      {
     send_to_char("Please dismount first.\n\r",ch);
     return;
  }

  if (*arg) {
     victim=get_char_room_vis(ch,arg);
     if (!victim) {
        send_to_char("Thats not here.\n\r", ch);
      return;
     }
  }
  else {
     org_room = CHAR_REAL_ROOM(ch);
     for (k=ch->followers; k; k= k->next) {
        if (org_room == CHAR_REAL_ROOM(k->follower)) {
          if (IS_NPC(k->follower) && (k->follower->master==ch) &&
            (IS_SET(k->follower->specials.act, ACT_MOUNT))) victim = k->follower;
      }
     }
     if (!victim) {
        send_to_char("You have nothing to free.\n\r", ch);
      return;
     }
  }

  if(!IS_NPC(victim)) return;

  if (victim->master!=ch) {
    send_to_char("That isn't yours to free.\n\r",ch);
    return;
  }

  if (GET_POS(victim)==POSITION_FIGHTING) {
     send_to_char("It seems a little busy at this time.\n\r",ch);
     return;
  }

  act("You dismiss $N back to the stables.",FALSE,ch,0,victim,TO_CHAR);
  act("$n dismisses $N back to the stables.",TRUE,ch,0,victim,TO_NOTVICT);
  stop_follower(victim);
/*  char_from_room(victim);*/
  extract_char(victim);
  return;
}

void do_dismount(struct char_data *ch, char *argument, int cmd) {
  char arg[100];
  struct char_data *victim=NULL;

  one_argument(argument, arg);

  if(GET_POS(ch) != POSITION_RIDING)      {
    send_to_char("But You are not riding anything.\n\r",ch);
    return;
  }

  if(*arg) {
    victim=get_char_room_vis(ch,arg);
    if (!victim) {
      send_to_char("That's not here.\n\r", ch);
      return;
    }
  }
  else {
    victim=ch->specials.riding;
    if(!victim) {
      GET_POS(ch)=POSITION_RESTING;
      act("You suddenly realise you weren't riding anything, and fall to the ground.",FALSE,ch,0,0,TO_CHAR);
      act("$n suddenly realises $e wasn't riding anything, and falls to the ground.",FALSE,ch,0,0,TO_NOTVICT);
      return;
    }
  }

  if (victim->specials.rider!=ch) {
    send_to_char("You aren't riding that.\n\r",ch);
    return;
  } else {
    act("You dismount $N.",FALSE,ch,0,victim,TO_CHAR);
    act("$n dismounts $N.",FALSE,ch,0,victim,TO_NOTVICT);
    stop_riding(ch,victim);
    return;
  }
}

/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
  struct char_data *k;

  for(k=victim; k; k=k->master) {
    if (k == ch)
      return(TRUE);
  }

  return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if(!ch->master)
    return;

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    /* For freeing - Ranger April 96 */
    if (!IS_SET(ch->specials.act, ACT_MOUNT)) {
       act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
       act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
       act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    }
    if (affected_by_spell(ch, SPELL_CHARM_PERSON))
      affect_from_char(ch, SPELL_CHARM_PERSON);
    if (ch->specials.rider) {
      stop_riding(ch->specials.rider,ch); /* Stop riding calls stop_fol*/
      return;
    }
  } else {
    if (ch->specials.rider) {
      stop_riding(ch->specials.rider,ch);
      return;
    }
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
  }

  if (ch->master->followers->follower == ch) { /* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
  } else { /* locate follower who is not head of list */
    for(k = ch->master->followers; k && k->next && k->next->follower && (k->next->follower != ch); k = k->next)  ;

    if (k) {
      j = k->next;
      if (j) {
        k->next = j->next;
        free(j);
      }
    }
  }

  ch->master = 0;
  REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k=ch->followers; k; k=j) {
    j = k->next;
    if(k->follower->master)
      stop_follower(k->follower);
  }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
  struct follow_type *k;

  if (ch->master)
    return;

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

/* do_move_keyword do_jump do_climb do_crawl
   by Ranger - Oct 96 */

void do_move_keyword(struct char_data *ch, char *argument, int cmd)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct room_direction_data *back;

  argument_interpreter(argument, type, dir);

  if (!*type) {
    send_to_char("Move what?\n\r", ch);
    return;
  }

  if((door = find_door(ch, type, dir)) >= 0) {
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_MOVE)) {
      send_to_char("That's impossible, I'm afraid.\n\r", ch);
    }
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      SET_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
      if (EXIT(ch, door)->keyword) {
        act("$n moves the $F closed.", 0, ch, 0, EXIT(ch, door)->keyword,TO_ROOM);
        act("You move the $F closed.", 0, ch, 0, EXIT(ch, door)->keyword,TO_CHAR);
      }
      else {
        sprintf(buf,"WIZINFO: Error in do_move_keyword, no keyword on exit, room %d",CHAR_VIRTUAL_ROOM(ch));
        wizlog(buf, LEVEL_WIZ, 6);
        return;
      }
      if ((other_room = EXIT(ch, door)->to_room_r) != NOWHERE) {
        if ((back = world[other_room].dir_option[rev_dir[door]])) {
          if (back->to_room_r == CHAR_REAL_ROOM(ch)) {
            SET_BIT(back->exit_info, EX_CLOSED);
            if(back->keyword) {
              sprintf(buf,"The %s moves quietly closed.\n\r", back->keyword);
              send_to_room(buf, EXIT(ch, door)->to_room_r);
            }
            else {
              sprintf(buf,"WIZINFO: Error in do_move_keyword, no keyword on other side of exit, room %d",CHAR_VIRTUAL_ROOM(ch));
              wizlog(buf, LEVEL_WIZ, 6);
              return;
            }
          }
        }
      }
    }
    else {
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
      if (EXIT(ch, door)->keyword) {
        act("$n moves the $F open.", FALSE, ch, 0, EXIT(ch, door)->keyword,TO_ROOM);
        act("You move the $F open.", FALSE, ch, 0, EXIT(ch, door)->keyword,TO_CHAR);
      }
      else {
        sprintf(buf,"WIZINFO: Error in do_move_keyword, no keyword on exit, room %d",CHAR_VIRTUAL_ROOM(ch));
        wizlog(buf, LEVEL_WIZ, 6);
        return;
      }

      if ((other_room = EXIT(ch, door)->to_room_r) != NOWHERE) {
        if ((back = world[other_room].dir_option[rev_dir[door]])) {
          if (back->to_room_r == CHAR_REAL_ROOM(ch)) {
            REMOVE_BIT(back->exit_info, EX_CLOSED);
            if(back->keyword) {
              sprintf(buf,"The %s moves open from the other side.\n\r",fname(back->keyword));
              send_to_room(buf, EXIT(ch, door)->to_room_r);
            }
            else {
              sprintf(buf,"WIZINFO: Error in do_move_keyword, no keyword on other side of exit, room %d",CHAR_VIRTUAL_ROOM(ch));
              wizlog(buf, LEVEL_WIZ, 6);
              return;
            }
          }
        }
      }
    }
  }
}


#define DIR_TYPE_CLIMB  0
#define DIR_TYPE_JUMP   1
#define DIR_TYPE_CRAWL  2
#define DIR_TYPE_ENTER  3

#define DIR_SINGULAR    0
#define DIR_PLURAL      1
#define DIR_ADVERB_PRE  2
#define DIR_ADVERB_POST 3

const char *special_move_str[][4] = {
  {"climb", "climbs", "", "" },
  {"jump", "jumps", "", "" },
  {"crawl", "crawls", " in", " through" },
  {"enter", "enters", "", ""},
};

const int special_move_cost[] = {
  6,
  10,
  3,
  3,
};

void do_special_move(struct char_data *ch, char *arg, int cmd) {
  int dir_type = -1;

  switch (cmd) {
    case CMD_CLIMB:
      dir_type = DIR_TYPE_CLIMB;
      break;

    case CMD_JUMP:
      dir_type = DIR_TYPE_JUMP;
      break;

    case CMD_CRAWL:
      dir_type = DIR_TYPE_CRAWL;
      break;

    case CMD_ENTER:
      dir_type = DIR_TYPE_ENTER;
      break;

    default:
      wizlog_f(LEVEL_WIZ, 6, "WIZINFO: Invalid cmd sent to do_special_move, cmd = %d", cmd);

      return;
      break;
  }

  if ((dir_type < 0) || (dir_type > NUMELEMS(special_move_str))) {
    wizlog_f(LEVEL_WIZ, 6, "WIZINFO: Invalid dir type in do_special_move, dir_type = %d", dir_type);

    return;
  }

  char type[MIL], dir[MIL];

  argument_interpreter(arg, type, dir);

  if (!*type) {
    char buf[MIL];

    snprintf(buf, sizeof(buf), "%s%s what?\n\r", special_move_str[dir_type][DIR_SINGULAR], special_move_str[dir_type][DIR_ADVERB_PRE]);

    send_to_char(CAP(buf), ch);

    return;
  }

  int door = find_door(ch, type, dir);

  if (door < 0) return;

  bool can_go = FALSE, up_down = FALSE;

  switch (dir_type) {
    case DIR_TYPE_CLIMB:
      if (IS_SET(EXIT(ch, door)->exit_info, EX_CLIMB)) {
        can_go = TRUE;

        if ((door == UP) || (door == DOWN)) {
          up_down = TRUE;
        }
      }
      break;

    case DIR_TYPE_JUMP:
      if (IS_SET(EXIT(ch, door)->exit_info, EX_JUMP)) {
        can_go = TRUE;

        if ((door == UP) || (door == DOWN)) {
          up_down = TRUE;
        }
      }
      break;

    case DIR_TYPE_CRAWL:
      if (IS_SET(EXIT(ch, door)->exit_info, EX_CRAWL)) {
        can_go = TRUE;
      }
      break;

    case DIR_TYPE_ENTER:
      if (IS_SET(EXIT(ch, door)->exit_info, EX_ENTER)) {
        can_go = TRUE;
      }
      break;
  }

  if (!can_go) {
    printf_to_char(ch, "You can't %s%s that.\n\r", special_move_str[dir_type][DIR_SINGULAR], special_move_str[dir_type][DIR_ADVERB_PRE]);

    return;
  }

  int move_cost = special_move_cost[dir_type];

  // Prestige Perk 23
  if (GET_PRESTIGE_PERK(ch) >= 23) {
    move_cost = MAX(1, move_cost - 1);
  }

  if (IS_MORTAL(ch) && (GET_MOVE(ch) < move_cost)) {
    printf_to_char(ch, "You are too exhausted.\n\r");

    return;
  }

  if (GET_POS(ch) == POSITION_RIDING) {
    printf_to_char(ch, "You must dismount first.\n\r");

    return;
  }

  if (!EXIT(ch, door)->keyword) {
    wizlog_f(LEVEL_WIZ, 6, "WIZINFO: Error in do_special_move, no keyword on exit, room %d", CHAR_VIRTUAL_ROOM(ch));

    return;
  }

  if ((EXIT(ch, door)->to_room_r == NOWHERE) || (EXIT(ch, door)->to_room_r == real_room(0))) {
    wizlog_f(LEVEL_WIZ, 6, "WIZINFO: Error in do_special_move, exit points to NULL or VOID, room %d", CHAR_VIRTUAL_ROOM(ch));

    return;
  }

  if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, TUNNEL) && IS_MORTAL(ch) && (count_mortals_real_room(EXIT(ch, door)->to_room_r) > 0) && !CHAOSMODE) {
    printf_to_char(ch, "It's too narrow to go there.\n\r");

    return;
  }

  if (((world[CHAR_REAL_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) || (world[EXIT(ch, door)->to_room_r].sector_type == SECT_WATER_NOSWIM)) && !IS_IMMORTAL(ch) && !HAS_BOAT(ch)) {
    printf_to_char(ch, "You need a boat to go there.\n\r");

    return;
  }

  if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, FLYING) && !IS_AFFECTED(ch, AFF_FLY) && IS_MORTAL(ch) && !CHAOSMODE) {
    printf_to_char(ch, "You are not flying.\n\r");

    return;
  }

  /* Druid SC3: Wall of Thorns - Blocks NPC movement and PC movement in chaotic rooms. */
  if (IS_NPC(ch) || (IS_MORTAL(ch) && (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) || ROOM_CHAOTIC(EXIT(ch, door)->to_room_r)))) {
    OBJ *thorns_here = get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch));
    OBJ *thorns_there = get_obj_room(WALL_THORNS, ROOM_VNUM(EXIT(ch, door)->to_room_r));

    if (thorns_here || thorns_there) {
      OBJ *thorns = thorns_here ? thorns_here : thorns_there;

      if (ROOM_SAFE(OBJ_IN_ROOM(thorns))) {
        send_to_char("A wall of thorns blocks your way.\n\r", ch);
      }
      else {
        send_to_char("A wall of thorns blocks your way.  Ouch!\n\r", ch);

        damage(ch, ch, MIN(GET_HIT(ch) - 1, OBJ_SPEC(thorns)), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
      }

      return;
    }
  }

  // Prestige Perk 7
  if (GET_PRESTIGE_PERK(ch) >= 7) {
    if ((IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, HAZARD) || IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, DEATH)) && chance(50)) {
      printf_to_char(ch, "You avoid certain death at the last instant and are momentarily paralyzed with fear.\n\r");

      GET_MOVE(ch) = 0;

      return;
    }
  }

  GET_MOVE(ch) -= move_cost;

  act_f(FALSE, ch, 0, EXIT(ch, door)->keyword, TO_CHAR,
    "You %s%s%s the $F.",
    special_move_str[dir_type][DIR_SINGULAR],
    (up_down ? " " : ""),
    (up_down ? dirs[door] : special_move_str[dir_type][DIR_ADVERB_POST]));

  act_f(COMM_ACT_HIDE_SUPERBRF, ch, 0, EXIT(ch, door)->keyword, (IS_AFFECTED(ch, AFF_SNEAK) ? TO_GROUP : TO_ROOM),
    "$n %s%s%s the $F.",
    special_move_str[dir_type][DIR_PLURAL],
    (up_down ? " " : ""),
    (up_down ? dirs[door] : special_move_str[dir_type][DIR_ADVERB_POST]));

  /* Signal the room the character is moving from. */
  if (signal_room(CHAR_REAL_ROOM(ch), ch, MSG_LEAVE, "")) return;

  /* In case MSG_LEAVE kills. */
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return;

  int was_in = CHAR_REAL_ROOM(ch);

  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[door]->to_room_r);

  act("$n has arrived.", COMM_ACT_HIDE_SUPERBRF, ch, 0, 0, (IS_AFFECTED(ch, AFF_SNEAK) ? TO_GROUP : TO_ROOM));

  do_look(ch, "", CMD_LOOK);

  if (signal_room(CHAR_REAL_ROOM(ch), ch, MSG_ENTER, "")) return;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, MOVE_TRAP)) {
    GET_MOVE(ch) = 0;
  }

  dt_or_hazard(ch);
}

void do_climb(struct char_data *ch, char *argument, int cmd) {
  do_special_move(ch, argument, cmd);
}

void do_jump(struct char_data *ch, char *argument, int cmd) {
  do_special_move(ch, argument, cmd);
}

void do_crawl(struct char_data *ch, char *argument, int cmd) {
  do_special_move(ch, argument, cmd);
}
