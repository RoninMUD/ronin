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
int special(struct char_data *ch, int cmd, char *arg);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list);
void stop_follower(struct char_data *ch);
void do_special_move(struct char_data *ch, char *arg, int cmd);

void dt_cry(CHAR *ch)
{
  int door;

  if(CHAR_REAL_ROOM(ch)==NOWHERE) return; /* Ranger - June 96 */
  act("Your body stiffens as you hear $n scream in agony.",FALSE, ch,0,0,TO_ROOM);

  for(door = 0; door <= 5; door++) {
    if(CAN_GO(ch, door))
      send_to_room("Your body stiffens as you hear someone scream in agony.\n\r",
                    world[CHAR_REAL_ROOM(ch)].dir_option[door]->to_room_r);
  }
}

int dt_or_hazard(CHAR *ch) {
  struct extra_descr_data *tmp_descr;
  int extra=0;
  char buf[MIL];

  if((IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, HAZARD) && number(0,1)) &&
      GET_LEVEL(ch) < LEVEL_IMM) {
    if(!IS_NPC(ch) && IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, HAZARD)) {
      sprintf(buf, "WIZINFO: %s fell into a HAZARD at %s (%d)", GET_NAME(ch),
              world[CHAR_REAL_ROOM(ch)].name,world[CHAR_REAL_ROOM(ch)].number);
      log_s(buf);
      deathlog(buf);
      wizlog(buf,LEVEL_IMM,3);
    }

    // signal character and rider with MSG_AUTORENT, which is a terrible hack
    // to have dynamic enchantments remove themselves and not save affects
    // permanently in raw_kill

    if(ch->specials.riding) {
      signal_char(ch->specials.riding, ch->specials.riding, MSG_AUTORENT, "");
      raw_kill(ch->specials.riding);
    }

    signal_char(ch, ch, MSG_AUTORENT, "");
    raw_kill(ch);
    return TRUE;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, DEATH) && GET_LEVEL(ch)<LEVEL_IMM) {
    if(!IS_NPC(ch)) {
      sprintf(buf, "WIZINFO: %s fell into a DT at %s (%d)", GET_NAME(ch),
              world[CHAR_REAL_ROOM(ch)].name,world[CHAR_REAL_ROOM(ch)].number);
      log_s(buf);
      deathlog(buf);
      wizlog(buf,LEVEL_IMM,3);
    }
    if(ch->specials.riding) {
      SET_BIT(ch->specials.riding->specials.affected_by2,AFF2_IMMINENT_DEATH);
      ch->specials.riding->specials.death_timer=2;
    }
    SET_BIT(ch->specials.affected_by2,AFF2_IMMINENT_DEATH);
    ch->specials.death_timer=2;

    /* Send death_text to character */
    tmp_descr=world[CHAR_REAL_ROOM(ch)].ex_description;
    while(tmp_descr && !extra) {
      if(!strcmp(tmp_descr->keyword,"death_text")) {
        if(tmp_descr->description) {
          send_to_char(tmp_descr->description,ch);
          extra=1;
        }
      }
      else {
        tmp_descr = tmp_descr->next;
      }
    }
    if(!extra)
      send_to_char("....\n\rDarkness begins to decend around you....\n\rYour time of death is near....\n\r",ch);

    dt_cry(ch);
    return TRUE;
  }

  return FALSE;
}

int special(struct char_data *ch, int cmd, char *arg);
int signal_room(int room, CHAR *ch, int cmd, char *arg);
int do_simple_move(struct char_data *ch, int cmd, int spec_check)
/*
Assumes:
  1. That there is no master and no followers.
  2. That the direction exists.

Returns:
  1 : If success.
  0 : If failure.
 -1 : If dead.
*/
{
  char buf[MSL];

  /* Check for special routines (North is 1) */
  if (spec_check && special(ch, cmd + 1, "")) {
    return FALSE;
  }

  /* Check for Pray. */
  if (!IS_IMMORTAL(ch) &&
      affected_by_spell(ch, SKILL_PRAY)) {
    send_to_char("You are deep in prayer, unable to follow.\n\r", ch);

    return FALSE;
  }

  /* Check for Meditation. */
  if (!IS_IMMORTAL(ch) &&
      (affected_by_spell(ch, SKILL_MEDITATE) &&
      (duration_of_spell(ch, SKILL_MEDITATE) > (CHAOSMODE ? 9 : 27)))) {
    send_to_char("You are deep in meditation, unable to follow.\n\r", ch);

    return FALSE;
  }

  if (!world[CHAR_REAL_ROOM(ch)].dir_option[cmd] ||
       (world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r == NOWHERE) ||
       (world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r == real_room(0))) return FALSE;

  /* Check for Wall of Thorns. */
  if (!IS_IMMORTAL(ch) &&
      (get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch)) ||
       get_obj_room(WALL_THORNS, world[world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r].number))) {
    send_to_char("A wall of thorns blocks your way. Ouch!\n\r", ch);

    damage(ch, ch, 30, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

    return FALSE;
  }

  /* Check for tunnel flee escape. */
  if (IS_SET(world[world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r].room_flags, TUNNEL) &&
      IS_MORTAL(ch) &&
      (count_mortals_real_room(world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r) > 0) &&
      !CHAOSMODE) {
    send_to_char("It's too narrow to go there.\n\r", ch);

    return FALSE;
  }

  /* Check if mount can move. */
  if (GET_MOUNT(ch) &&
      (GET_POS(GET_MOUNT(ch)) != POSITION_FLYING) &&
      (GET_POS(GET_MOUNT(ch)) != POSITION_STANDING) &&
      (GET_POS(GET_MOUNT(ch)) != POSITION_SWIMMING)) {
    send_to_char("Your mount is unable to move.\n\r", ch);

    return FALSE;
  }

  /* Check if exit is a flying room. */
  if (IS_SET(world[world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r].room_flags, FLYING) &&
      !IS_IMMORTAL(ch)) {
    if (cmd == UP) {
      if (GET_MOUNT(ch)) {
        if (!IS_SET(GET_ACT(GET_MOUNT(ch)), ACT_FLY)) {
          send_to_char("Your mount isn't capable of flight.\n\r", ch);

          return FALSE;
        }
        if (!IS_AFFECTED(GET_MOUNT(ch), AFF_FLY)) {
          send_to_char("Your mount isn't flying.\n\r", ch);

          return FALSE;
        }
      }
      else if (IS_MORTAL(ch)) {
        send_to_char("You need a flying mount to gain altitude.\n\r", ch);

        return FALSE;
      }
    }

    if (GET_MOUNT(ch)) {
      if (!IS_AFFECTED(GET_MOUNT(ch), AFF_FLY)) {
        send_to_char("Your mount isn't flying.\n\r", ch);

        return FALSE;
      }
    }
    else {
      if (!IS_AFFECTED(ch, AFF_FLY)) {
        send_to_char("You are not flying.\n\r", ch);

        if (GET_RIDER(ch)) {
          send_to_char("Your mount is not flying.\n\r", GET_RIDER(ch));
        }

        return FALSE;
      }
    }
  }

  /* Check to see if they have a boat, or can swim. */
  if ((world[CHAR_REAL_ROOM(ch)].sector_type == SECT_WATER_NOSWIM ||
       world[world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r].sector_type == SECT_WATER_NOSWIM) &&
      !IS_IMMORTAL(ch)) {
    bool has_boat = FALSE;

    /* If they're flying, or they're a ninja, they have a boat. */
    if (IS_AFFECTED(ch, AFF_FLY) || GET_CLASS(ch) == CLASS_NINJA) has_boat = TRUE;

    /* Check if they are carrying a boat. */
    for (struct obj_data *tmp_obj = ch->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
      if (OBJ_TYPE(tmp_obj) == ITEM_BOAT) {
        has_boat = TRUE;
        break;
      }
    }

    /* Check if their boots are a boat. */
    if (EQ(ch, WEAR_FEET) && OBJ_TYPE(EQ(ch, WEAR_FEET)) == ITEM_BOAT) has_boat = TRUE;

    /* Check if their mount has a boat, or can fly. etc. */
    if (!has_boat && GET_MOUNT(ch)) {
      for (struct obj_data *tmp_obj = GET_MOUNT(ch)->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
        if (OBJ_TYPE(tmp_obj) == ITEM_BOAT) {
          has_boat = TRUE;
          break;
        }
      }

      if (IS_AFFECTED(GET_MOUNT(ch), AFF_FLY)) has_boat = TRUE;
      if (IS_SET(GET_ACT(GET_MOUNT(ch)), ACT_FLY)) has_boat = TRUE;
      if (EQ(GET_MOUNT(ch), WEAR_FEET) && OBJ_TYPE(EQ(GET_MOUNT(ch), WEAR_FEET)) == ITEM_BOAT) has_boat = TRUE;
    }

    if (!has_boat) {
      if (GET_MOUNT(ch)) {
        send_to_char("Your mount needs a boat to go there.\n\r", ch);
      }
      else {
        send_to_char("You need a boat to go there.\n\r", ch);
      }

      return FALSE;
    }
  }

  /* Calculate encumberance modifier. */
  int carrying_weight = IS_CARRYING_W(ch);
  int can_carry_weight = CAN_CARRY_W(ch);

  int encumb_modifier = 1;

  if (carrying_weight >= 3 * can_carry_weight)
    encumb_modifier = 6;
  else if (carrying_weight >= 5 * can_carry_weight / 2)
    encumb_modifier = 5;
  else if (carrying_weight >= 2 * can_carry_weight)
    encumb_modifier = 4;
  else if (carrying_weight >= 3 * can_carry_weight / 2)
    encumb_modifier = 3;
  else if (carrying_weight >= can_carry_weight)
    encumb_modifier = 2;

  int need_movement = encumb_modifier * (movement_loss[world[CHAR_REAL_ROOM(ch)].sector_type] +
                      movement_loss[world[world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r].sector_type]) / 2;

  /* Handle riding. */
  if (GET_MOUNT(ch)) {
    need_movement = 1;

    if (IS_SET(GET_ACT(GET_MOUNT(ch)), ACT_MOUNT)) {
      need_movement = 3;

      /* Check for bridle. */
      if (EQ(GET_MOUNT(ch), WEAR_NECK_1)) {
        if (V_OBJ(EQ(GET_MOUNT(ch), WEAR_NECK_1)) == 3900) need_movement--;
      }
      else if (EQ(GET_MOUNT(ch), WEAR_NECK_2)) {
        if (V_OBJ(EQ(GET_MOUNT(ch), WEAR_NECK_2)) == 3900) need_movement--;
      }

      /* Check for saddle. */
      if (EQ(GET_MOUNT(ch), WEAR_BODY)) {
        if (V_OBJ(EQ(GET_MOUNT(ch), WEAR_BODY)) == 3904) need_movement--;
      }
    }
  }

  /* Check for Paralaysis and Hold Person. */
  if (!IS_IMMORTAL(ch) &&
      !GET_MOUNT(ch) &&
      (IS_AFFECTED(ch, AFF_PARALYSIS) || IS_AFFECTED(ch, AFF_HOLD)) &&
      /* Combat Zen */
      !(IS_MORTAL(ch) && check_subclass(ch, SC_RONIN, 3) && chance(50))) {
    send_to_char("You are paralyzed! You can't move!\n\r", ch);

    return FALSE;
  }

  // Prestige Perk 23
  if (GET_PRESTIGE_PERK(ch) >= 23) {
    need_movement = MAX(1, need_movement - 1);
  }

  /* Check for exhaustion. */
  if (IS_MORTAL(ch) && (GET_MOVE(ch) < need_movement)) {
    if (GET_MASTER(ch)) {
      send_to_char("You are too exhausted to follow.\n\r", ch);
    }
    else {
      send_to_char("You are too exhausted.\n\r", ch);
    }

    return FALSE;
  }

  // Prestige Perk 7
  if (GET_PRESTIGE_PERK(ch) >= 7) {
    if ((IS_SET(world[world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r].room_flags, HAZARD) ||
         IS_SET(world[world[CHAR_REAL_ROOM(ch)].dir_option[cmd]->to_room_r].room_flags, DEATH)) &&
        chance(50)) {
      send_to_char("You avoid certain death at the last moment and are momentarily paralyzed with fear.\n\r", ch);

      GET_MOVE(ch) = 0;

      return FALSE;
    }
  }

  /* Subtract their movement points. */
  if (IS_MORTAL(ch)) {
    GET_MOVE(ch) -= need_movement;
  }

  if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_WATER_NOSWIM &&
      !IS_IMMORTAL(ch) &&
      GET_CLASS(ch) == CLASS_NINJA &&
      !IS_AFFECTED(ch, AFF_FLY) &&
      GET_POS(ch) != POSITION_RIDING) {
    GET_POS(ch) = POSITION_SWIMMING;
  }

  if (IS_AFFECTED(ch, AFF_FLY)) {
    if (ch->player.poofout && (GET_LEVEL(ch) < LEVEL_IMM)) {
      snprintf(buf, sizeof(buf), "%s %s.", ch->player.poofout, dirs[cmd]);
    }
    else {
      snprintf(buf, sizeof(buf), "$n flies %s.", dirs[cmd]);
    }

    act(buf, 2, ch, 0, 0, TO_ROOM);
  }
  else if (GET_POS(ch) == POSITION_SWIMMING) {
    if (ch->player.poofout && (GET_LEVEL(ch) < LEVEL_IMM)) {
      snprintf(buf, sizeof(buf), "%s %s.", ch->player.poofout, dirs[cmd]);
    }
    else {
      snprintf(buf, sizeof(buf), "$n swims %s.", dirs[cmd]);
    }

    act(buf, 2, ch, 0, 0, TO_ROOM);
  }
  else if (GET_MOUNT(ch)) {   
    if (GET_POS(GET_MOUNT(ch)) == POSITION_FLYING) {
      snprintf(buf, sizeof(buf), "$n flies %s.", dirs[cmd]);
    }
    else {
      snprintf(buf, sizeof(buf), "$n rides %s.", dirs[cmd]);
    }

    act(buf, 2, ch, 0, 0, TO_ROOM);
  }
  else if (GET_RIDER(ch)) {
    for (struct char_data *tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
      if (CAN_SEE(tmp_ch, GET_RIDER(ch)) && (tmp_ch != GET_RIDER(ch))) {
        if (GET_POS(ch) == POSITION_FLYING) {
          snprintf(buf, sizeof(buf), "$n flies %s.", dirs[cmd]);
        }
        else {
          snprintf(buf, sizeof(buf), "$n leaves %s.", dirs[cmd]);
        }

        act(buf, 2, ch, 0, tmp_ch, TO_VICT);
      }
    }
  }
  else {
    if (!IS_IMMORTAL(ch) && (GET_HIT(ch) < (hit_limit(ch) / 10))) {
      snprintf(buf, sizeof(buf), "$n crawls %s.", dirs[cmd]);
    }
    else {
      if (ch->player.poofout && (GET_LEVEL(ch) < LEVEL_IMM)) {
        snprintf(buf, sizeof(buf), "%s %s.", ch->player.poofout, dirs[cmd]);
      }
      else {
        snprintf(buf, sizeof(buf), "$n leaves %s.", dirs[cmd]);
      }
    }

    if (IS_AFFECTED(ch, AFF_SNEAK)) {
      act(buf, 2, ch, 0, 0, TO_GROUP);
    }
    else {
      act(buf, 2, ch, 0, 0, TO_ROOM);
    }
  }

  /* If adding hunting, or scanning, or blood trail, etc.,
     it should probably be added here. */

  int was_in = CHAR_REAL_ROOM(ch);

  /* Signal the room the character is moving from. */
  if (signal_room(CHAR_REAL_ROOM(ch), ch, MSG_LEAVE, "")) return FALSE;

  /* In case MSG_LEAVE kills. */
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return FALSE;

  /* Move the character from the old room. */
  char_from_room(ch);

  /* Move the character to the new room. */
  char_to_room(ch, world[was_in].dir_option[cmd]->to_room_r);

  /* Move their mount. */
  if (GET_MOUNT(ch)) {
    char_from_room(GET_MOUNT(ch));
    char_to_room(GET_MOUNT(ch), world[was_in].dir_option[cmd]->to_room_r);
  }

  /* Show arrival message, as approrpriate. */
  if (ch->player.poofout && (GET_LEVEL(ch) < LEVEL_IMM)) {
    snprintf(buf, sizeof(buf), "%s %s.", ch->player.poofout, dirs[cmd]);
  }
  else {
    snprintf(buf, sizeof(buf), "$n has arrived.");
  }

  if (GET_RIDER(ch)) {
    for (struct char_data *tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
      if (CAN_SEE(tmp_ch, GET_RIDER(ch)) && (tmp_ch != GET_RIDER(ch))) {
        act(buf, 2, ch, 0, tmp_ch, TO_VICT);
      }
    }
  }
  else {
    if (IS_AFFECTED(ch, AFF_SNEAK)) {
      for (CHAR *temp_ch = world[CHAR_REAL_ROOM(ch)].people, *next_ch; temp_ch; temp_ch = next_ch) {
        next_ch = temp_ch->next_in_room;

        if ((temp_ch == ch) || (!IS_IMMORTAL(ch) && !SAME_GROUP(temp_ch, ch))) continue;

        act(buf, 2, ch, 0, 0, TO_CHAR);
      }
    }
    else {
      act(buf, 2, ch, 0, 0, TO_ROOM);
    }
  }

  /* Do look do look for the character. */
  do_look(ch, "\0", CMD_LOOK);

  /* Do look for the mount. */
  if (GET_MOUNT(ch)) {
    do_look(GET_MOUNT(ch), "\0", CMD_LOOK);
  }

  /* Signal the room the character just moved to. */
  if (signal_room(CHAR_REAL_ROOM(ch), ch, MSG_ENTER, "")) return FALSE;

  /* In case MSG_ENTER kills. */
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return FALSE;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, MOVETRAP) && !IS_IMMORTAL(ch)) {
    send_to_char("\n\rYour movement points have been drained by the surroundings.\n\r", ch);

    GET_MOVE(ch) = 0;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, MANADRAIN) && !IS_IMMORTAL(ch)) {
    send_to_char("\n\rYour mana has been drained by the surroundings.\n\r", ch);

    GET_MANA(ch) = 0;
  }

  if (dt_or_hazard(ch)) return -1;

  /* Check for Trap. */
  int trap_check = number(1, 110);

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, TRAP) &&
      (GET_POS(ch) != POSITION_FLYING) &&
      ((IS_NPC(ch) && (trap_check > (GET_LEVEL(ch) * 2))) ||
       (IS_MORTAL(ch) && IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) && (trap_check > (GET_LEVEL(ch) * 2))) ||
       (IS_MORTAL(ch) && CHAOSMODE && (trap_check > GET_LEVEL(ch))))) {
    act("$n fell into a trap!", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You fell into a trap!\n\r", ch);

    REMOVE_BIT(world[CHAR_REAL_ROOM(ch)].room_flags, TRAP);

    GET_HIT(ch) = (4 * GET_HIT(ch)) / 5;

    return TRUE;
  }

  return TRUE;
}


void do_move(struct char_data *ch, char *argument, int cmd) {
  --cmd;

  if (IS_SET(ch->specials.affected_by2, AFF2_SEVERED)) {
    send_to_char("Move without legs? How?\n\r", ch);

    return;
  }

  if (ch->specials.death_timer == 2) {
    send_to_char("You are near certain death, you can't move!\n\r", ch);

    return;
  }

  if (!world[CHAR_REAL_ROOM(ch)].dir_option[cmd] ||
      EXIT(ch, cmd)->to_room_r == NOWHERE ||
      EXIT(ch, cmd)->to_room_r == real_room(0)) {
    send_to_char("Alas, you cannot go that way...\n\r", ch);

    return;
  }

  if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLIMB) ||
      IS_SET(EXIT(ch, cmd)->exit_info, EX_JUMP) ||
      IS_SET(EXIT(ch, cmd)->exit_info, EX_CRAWL) ||
      IS_SET(EXIT(ch, cmd)->exit_info, EX_ENTER)) {
    send_to_char("Alas, you cannot go that way...\n\r", ch);

    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) && ch->master && CHAR_REAL_ROOM(ch) == CHAR_REAL_ROOM(ch->master)) {
    send_to_char("The thought of leaving your master makes you weep.\n\r", ch);
    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);

    return;
  }

  if (IS_NPC(ch) && ch->specials.rider && CHAR_REAL_ROOM(ch) == CHAR_REAL_ROOM(ch->specials.rider)) {
    send_to_char("You don't want to leave your master.\n\r", ch);

    return;
  }

  if (!IS_IMMORTAL(ch) && (get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch) || get_obj_room(WALL_THORNS, EXIT(ch, cmd)->to_room_v)))) {
    send_to_char("A wall of thorns blocks your way. Ouch!\n\r", ch);

    damage(ch, ch, 30, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

    return;
  }

  if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLOSED)) {
    if (EXIT(ch, cmd)->keyword) {
      printf_to_char(ch, "The %s seems to be closed.\n\r", fname(EXIT(ch, cmd)->keyword));
    }
    else {
      send_to_char("It seems to be closed.\n\r", ch);
    }

    return;
  }

  if (IS_SET(world[EXIT(ch, cmd)->to_room_r].room_flags, TUNNEL) &&
      IS_MORTAL(ch) &&
      (count_mortals_real_room(EXIT(ch, cmd)->to_room_r) > 0) &&
      !CHAOSMODE) {
    send_to_char("It's too narrow to go there.\n\r", ch);

    return;
  }

  int was_in = CHAR_REAL_ROOM(ch);

  /* Move the character. */
  if (do_simple_move(ch, cmd, 1) == 1) {
    if (!ch) {
      log_f("WARNING: ch doesn't exist after a movement");

      return;
    }

    if (ch->followers) {
      /* If success, move followers. */
      for (struct follow_type *temp_follower = ch->followers, *next_follower = NULL; temp_follower; temp_follower = next_follower) {
        next_follower = temp_follower->next;

        if ((was_in != CHAR_REAL_ROOM(temp_follower->follower)) || (GET_POS(temp_follower->follower) < POSITION_STANDING)) continue;

        act("You follow $N.", FALSE, temp_follower->follower, 0, ch, TO_CHAR);
        cmd++;
        send_to_char("\n\r", temp_follower->follower);
        do_move(temp_follower->follower, argument, cmd);
        cmd--;
      }
    }
  }
}

void look_in_room(CHAR *ch, int vnum);

void do_peek(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  int keyword_no;
  int was_in, percent;
  const char * const keywords[]= {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "",  /* Look at '' case */
    "\n" };

  if (!ch->desc || !ch->skills)
    return;

  if ((GET_CLASS(ch) != CLASS_THIEF) && (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_NOMAD) && (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_BARD) && (GET_CLASS(ch) != CLASS_AVATAR) &&
      (GET_LEVEL(ch) < LEVEL_IMM)) {
    send_to_char("Leave this job for the others.\n\r", ch);
    return;
  }

  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char("You can't see anything but stars!\n\r", ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char("You can't see anything, you're sleeping!\n\r", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see a damn thing, you're blind!\n\r", ch);
  else if ( IS_DARK(CHAR_REAL_ROOM(ch)) && !IS_AFFECTED(ch,AFF_INFRAVISION) )
    send_to_char("It is pitch black...\n\r", ch);
  else {
    arg1[0] = 0; /* null terminate the thing */
    sscanf(argument,"%s",arg1);
    keyword_no = search_block(arg1, keywords, FALSE); /* Partiel Match */

    /*    if ((keyword_no == -1) && *arg1) {
        keyword_no = 7;
        strcpy(arg2, arg1);
        }
    */

    switch(keyword_no) {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 : {

      if (EXIT(ch, keyword_no) &&
        world[CHAR_REAL_ROOM(ch)].dir_option[keyword_no]->to_room_r != -1 &&
        world[CHAR_REAL_ROOM(ch)].dir_option[keyword_no]->to_room_r != real_room(0) &&
        !IS_SET(EXIT(ch, keyword_no)->exit_info, EX_JUMP) &&
        !IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLIMB) &&
        !IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CRAWL) &&
        !IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ENTER) &&
        !IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED)) {
      send_to_char("You try to peek.\n\r", ch);

  /* NO_PEEK - Ranger Sept 97 */
      if(IS_SET(world[world[CHAR_REAL_ROOM(ch)].dir_option[keyword_no]->to_room_r].room_flags,NO_PEEK)) {
    send_to_char("Something blocks your vision.\n\r",ch);
    return;
      }
      percent = number(1, 85); /* removing fail on peek, basically */

      if (percent > ch->skills[SKILL_PEEK].learned)
        { return; }
      if (IS_DARK(world[CHAR_REAL_ROOM(ch)].dir_option[keyword_no]->to_room_r) &&
          !IS_AFFECTED(ch,AFF_INFRAVISION) ) {
        send_to_char("It is pitch black...\n\r", ch);
        return;
      }

      was_in = CHAR_REAL_ROOM(ch);
  look_in_room(ch, world[world[was_in].dir_option[keyword_no]->to_room_r].number);

      } else {
      send_to_char("Nothing special there...\n\r", ch);
      }
    }
      break;

    case 6 : { send_to_char("Select a direction please.\n\r", ch); }
      break;

    case -1 :
      send_to_char("Sorry, I didn't understand that!\n\r", ch);
      break;
    }
  }
}

int find_door(struct char_data *ch, char *type, char *dir)
{
  int door;
  const char * const dirs[] =
    {
      "north",
      "east",
      "south",
      "west",
      "up",
      "down",
      "\n"
    };

  if (*dir) /* a direction was specified */
    {
      if ((door = search_block(dir, dirs, FALSE)) == -1) /* Partial Match */
      {
        send_to_char("That's not a direction.\n\r", ch);
        return(-1);
      }

      if (EXIT(ch, door))
      if (EXIT(ch, door)->keyword)
        if (isname(type, EXIT(ch, door)->keyword))
          return(door);
        else
          {
            printf_to_char(ch, "I see no %s there.\n\r", type);
            return(-1);
          }
      else
        return(door);
      else
      {
        send_to_char(
                   "I really don't see how you can close anything there.\n\r", ch);
        return(-1);
      }
    }
  else /* try to locate the keyword */
    {
      for (door = 0; door <= 5; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->keyword)
          if (isname(type, EXIT(ch, door)->keyword))
            return(door);

      printf_to_char(ch, "I see no %s here.\n\r", type);
      return(-1);
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
int has_key(struct char_data *ch, int key)
{
  struct obj_data *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (obj_proto_table[o->item_number].virtual == key)
      return 1;

  if (ch->equipment[HOLD])
    if (obj_proto_table[ch->equipment[HOLD]->item_number].virtual == key)
      return 1;

  return 0;
}

OBJ *find_keychain( CHAR* ch )
{
  OBJ *chain = get_obj_in_list_num( real_object( KEY_RING ), ch->carrying );
  if( !chain )
    if( EQ(ch, HOLD) && V_OBJ( EQ(ch, HOLD ) ) == KEY_RING )
      chain = EQ(ch, HOLD );

  return chain;
}

bool keys_from_ring( OBJ *chain, CHAR *ch, char *argument, int cmd )
{
  CHAR *victim = chain->carried_by;
  if( !victim )
    victim = chain->equipped_by;

  struct obj_data *carrying = NULL;
  struct obj_data *next_item = NULL;

  for( carrying = chain->contains; carrying; carrying = next_item )
  {
    next_item = carrying->next_content;
    obj_from_obj( carrying );
    obj_to_char( carrying, victim );
    if( signal_object( carrying, ch, cmd, argument ) )
      return TRUE;
  }

  return FALSE;
}

bool keys_to_ring( OBJ* chain)
{
  CHAR *victim = chain->carried_by;
  if( !victim )
    victim = chain->equipped_by;

  struct obj_data *carrying = NULL;
  struct obj_data *next_item = NULL;

  bool fiddled = FALSE;


  bool moved_object = FALSE;
  do
  {
    moved_object = FALSE;
    for( carrying = victim->carrying; carrying; carrying = next_item )
    {
      next_item = carrying->next_content;

      if( carrying->obj_flags.type_flag == ITEM_KEY )
      {
        obj_from_char( carrying );
        obj_to_obj( carrying, chain );
        fiddled = TRUE;
        moved_object = TRUE;
        break;
      }
    }
  } while( moved_object );

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
    if( keys_from_ring( chain, ch, argument, cmd ) )
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
    if( keys_from_ring( chain, ch, argument, cmd ) )
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


void do_wake(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *tmp_char;
  char arg[MAX_STRING_LENGTH];

  one_argument(argument, arg);

  if (*arg) {
    if (GET_POS(ch) == POSITION_SLEEPING) {
      act("You can't wake people up if you are asleep yourself!",
        FALSE, ch, 0, 0, TO_CHAR);
    }
    else {
      tmp_char = get_char_room_vis(ch, arg);
      if (tmp_char) {
        if (tmp_char == ch) {
          act("If you want to wake yourself up, just type 'wake'",
            FALSE, ch, 0, 0, TO_CHAR);
        }
        else {
          if (GET_POS(tmp_char) == POSITION_SLEEPING) {
            if (IS_AFFECTED(tmp_char, AFF_SLEEP) && !ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
              act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR);
            }
            else {
              if (IS_AFFECTED(tmp_char, AFF_SLEEP)) {
                affect_from_char(tmp_char, SPELL_SLEEP);
              }
              act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
              GET_POS(tmp_char) = POSITION_SITTING;
              act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
            }
          }
          else {
            act("$N is already awake.", FALSE, ch, 0, tmp_char, TO_CHAR);
          }
        }
      }
      else {
        send_to_char("You do not see that person here.\n\r", ch);
      }
    }
  }
  else {
    if (IS_AFFECTED(ch, AFF_SLEEP)) {
      send_to_char("You can't wake up!\n\r", ch);
    }
    else {
      if (GET_POS(ch) > POSITION_SLEEPING)
        send_to_char("You are already awake...\n\r", ch);
      else {
        send_to_char("You wake, and sit up.\n\r", ch);
        act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SITTING;
      }
    }
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


void stop_riding(struct char_data *ch,struct char_data *vict) {
  GET_POS(ch)=POSITION_STANDING;
  ch->specials.riding=0;
  if(vict) {
    vict->specials.rider=0;
    if(IS_AFFECTED(vict, AFF_CHARM))
    { } /* Perhaps some descript for stables here later - Ranger */
    else stop_follower(vict);
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
    for(k = ch->master->followers; k && k->next && (k->next->follower != ch); k = k->next)  ;

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

const char * special_move_str[][4] = {
  {"climb", "climbs", " ", " " },
  {"jump", "jumps", " ", " " },
  {"crawl", "crawls", " in ", " through " },
  {"enter", "enters", " ", " "},
};

const int special_move_mv[] = {
  6,
  10,
  3,
  3,
};

void do_special_move(struct char_data *ch, char *arg, int cmd) {
  char buf[MSL];

  int dir_type = -1;
  bool up_down = FALSE;

  switch (cmd) {
    case CMD_CLIMB:
      dir_type = DIR_TYPE_CLIMB;
      up_down = TRUE;
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
      sprintf(buf, "WIZINFO: Invalid cmd sent to do_special_move, cmd = %d", cmd);
      wizlog(buf, LEVEL_WIZ, 6);
      return;
      break;
  }

  if (dir_type < 0) {
    sprintf(buf, "WIZINFO: Invalid dir type in do_special_move, dir_type = %d", dir_type);
    wizlog(buf, LEVEL_WIZ, 6);

    return;
  }

  char type[MIL];
  char dir[MIL];

  argument_interpreter(arg, type, dir);

  if (!*type) {
    snprintf(buf, sizeof(buf), "%s%s", special_move_str[dir_type][DIR_SINGULAR], special_move_str[dir_type][DIR_ADVERB_PRE]);
    CAP(buf);
    strcat(buf, "what?\n\r");
    send_to_char(buf, ch);

    return;
  }

  int door = -1;

  if ((door = find_door(ch, type, dir)) < 0) return;

  bool can_go = FALSE;

  switch (dir_type) {
    case DIR_TYPE_CLIMB:
      if (IS_SET(EXIT(ch, door)->exit_info, EX_CLIMB)) can_go = TRUE;
      break;
    case DIR_TYPE_JUMP:
      if (IS_SET(EXIT(ch, door)->exit_info, EX_JUMP)) can_go = TRUE;
      break;
    case DIR_TYPE_CRAWL:
      if (IS_SET(EXIT(ch, door)->exit_info, EX_CRAWL)) can_go = TRUE;
      break;
    case DIR_TYPE_ENTER:
      if (IS_SET(EXIT(ch, door)->exit_info, EX_ENTER)) can_go = TRUE;
      break;
  }

  if (!can_go) {
    snprintf(buf, sizeof(buf), "You can't %s%sthat.\n\r", special_move_str[dir_type][DIR_SINGULAR], special_move_str[dir_type][DIR_ADVERB_PRE]);
    send_to_char(buf, ch);

    return;
  }

  int move_cost = special_move_mv[dir_type];

  // Prestige Perk 23
  if (GET_PRESTIGE_PERK(ch) >= 23) {
    move_cost = MAX(1, move_cost - 1);
  }

  if (GET_MOVE(ch) < move_cost && !IS_NPC(ch)) {
    send_to_char("You are too exhausted.\n\r", ch);

    return;
  }

  if (GET_POS(ch) == POSITION_RIDING) {
    send_to_char("You must dismount first.\n\r", ch);

    return;
  }

  if (!EXIT(ch, door)->keyword) {
    sprintf(buf, "WIZINFO: Error in do_special_move, no keyword on exit, room %d", CHAR_VIRTUAL_ROOM(ch));
    wizlog(buf, LEVEL_WIZ, 6);

    return;
  }

  if (!IS_IMMORTAL(ch) && (get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch) || get_obj_room(WALL_THORNS, EXIT(ch, door)->to_room_v)))) {
    send_to_char("A wall of thorns blocks your way. Ouch!\n\r", ch);

    damage(ch, ch, 30, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

    return;
  }  

  int other_room = EXIT(ch, door)->to_room_r;

  if (other_room == NOWHERE || other_room == real_room(0)) {
    sprintf(buf, "WIZINFO: Error in do_special_move, exit points to NULL or VOID, room %d", CHAR_VIRTUAL_ROOM(ch));
    wizlog(buf, LEVEL_WIZ, 6);

    return;
  }

  if (IS_SET(world[other_room].room_flags, TUNNEL) && IS_MORTAL(ch) && (count_mortals_real_room(other_room)) > 0 && !CHAOSMODE) {
    send_to_char("It's too narrow to go there.\n\r", ch);

    return;
  }

  if (IS_SET(world[other_room].room_flags, FLYING) && !IS_AFFECTED(ch, AFF_FLY) && IS_MORTAL(ch) && !CHAOSMODE) {
    send_to_char("You are not flying.\n\r", ch);

    return;
  }

  // Prestige Perk 7
  if (GET_PRESTIGE_PERK(ch) >= 7) {
    if ((IS_SET(world[other_room].room_flags, HAZARD) ||
         IS_SET(world[other_room].room_flags, DEATH)) &&
        chance(50)) {
      send_to_char("You avoid certain death at the last moment and are momentarily paralyzed with fear.\n\r", ch);

      GET_MOVE(ch) = 0;

      return;
    }
  }

  GET_MOVE(ch) -= move_cost;

  if (up_down && (door == UP || door == DOWN)) {
    snprintf(buf, sizeof(buf), "$n %s %s the $F.", special_move_str[dir_type][DIR_PLURAL], dirs[door]);
    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
      act(buf, 2, ch, 0, EXIT(ch, door)->keyword, TO_ROOM);
    }
    else {
      act(buf, 2, ch, 0, EXIT(ch, door)->keyword, TO_GROUP);
    }
    snprintf(buf, sizeof(buf), "You %s %s the $F.", special_move_str[dir_type][DIR_SINGULAR], dirs[door]);
    act(buf, 0, ch, 0, EXIT(ch, door)->keyword, TO_CHAR);
  }
  else {
    snprintf(buf, sizeof(buf), "$n %s%sthe $F.", special_move_str[dir_type][DIR_PLURAL], special_move_str[dir_type][DIR_ADVERB_POST]);
    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
      act(buf, 0, ch, 0, EXIT(ch, door)->keyword, TO_ROOM);
    }
    else {
      act(buf, 2, ch, 0, EXIT(ch, door)->keyword, TO_GROUP);
    }
    snprintf(buf, sizeof(buf), "You %s%sthe $F.", special_move_str[dir_type][DIR_SINGULAR], special_move_str[dir_type][DIR_ADVERB_POST]);
    act(buf, 0, ch, 0, EXIT(ch, door)->keyword, TO_CHAR);
  }

  char_from_room(ch);
  char_to_room(ch, other_room);

  snprintf(buf, sizeof(buf), "$n as arrived.");
  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    for (CHAR *temp_ch = world[CHAR_REAL_ROOM(ch)].people, *next_ch; temp_ch; temp_ch = next_ch) {
      next_ch = temp_ch->next_in_room;

      if ((temp_ch == ch) || (!IS_IMMORTAL(ch) && !SAME_GROUP(temp_ch, ch))) continue;

      act(buf, 2, ch, 0, 0, TO_CHAR);
    }
  }
  else {
    act(buf, 2, ch, 0, 0, TO_ROOM);
  }

  do_look(ch, "\0", CMD_LOOK);

  if (signal_room(CHAR_REAL_ROOM(ch), ch, MSG_ENTER, "\0")) return;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, MOVETRAP)) GET_MOVE(ch) = 0;

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
