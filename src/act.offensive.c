/**************************************************************************
*  file: act.offensive.c , Implementation of commands.    Part of DIKUMUD *
*  Usage : Offensive commands.                                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "modify.h"
#include "utility.h"
#include "fight.h"
#include "act.h"
#include "cmd.h"
#include "mob.spells.h"
#include "subclass.h"
#include "enchant.h"

extern int CHAOSMODE;
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct room_data *world;
extern struct dex_app_type dex_app[];
extern struct int_app_type int_app[];
extern struct wis_app_type wis_app[];

int MAX_PRAC(CHAR *ch);
void raw_kill(struct char_data *ch);
int calc_position_damage(int position, int dam);
int stack_position(CHAR *ch, int target_position);


int impair_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg) {
  return FALSE;
}


/* Do not use this function on victims; it is designed for skill users only. */
void skill_wait(CHAR *ch, int skill, int wait) {
  if (IS_IMPLEMENTOR(ch) || (CHAR_REAL_ROOM(ch) == NOWHERE) || (wait < 1)) return;

  /* Quick Recovery */
  if ((wait > 1) &&
      check_subclass(ch, SC_ROGUE, 5) &&
      chance(number(50, 75))) {
    wait -= 1;
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * wait);
}


void auto_learn_skill(CHAR *ch, int skill) {
  if (GET_LEARNED(ch, skill) < 80) {
    GET_LEARNED(ch, skill) = MIN(GET_LEARNED(ch, skill) + 2, 80);
  }
}


void do_block(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_AVATAR)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_BLOCK)) {
    affect_from_char(ch, SKILL_BLOCK);

    send_to_char("You will now let your victim flee.\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_BLOCK)) {
    send_to_char("You failed to concentrate on blocking your enemies.\n\r", ch);
  }
  else {
    af.type = SKILL_BLOCK;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You will now block your enemies if they flee.\n\r", ch);
  }
}


void do_hit(CHAR *ch, char *argument, int cmd) {
  char arg[MSL];
  char buf[MSL];
  CHAR *victim = NULL;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Hit who?\n\r", ch);

    return;
  }

  victim = get_char_room_vis(ch, arg);

  if ((victim == ch) && IS_NPC(ch)) {
    victim = get_mortal_room_vis(ch, arg);
  }

  if (ch->bot.misses >= 20) {
    sprintf(buf, "WARNING: %s has 20 kill/hit misses", GET_NAME(ch));
    log_s(buf);

    ch->bot.misses = 0;
  }

  if (!victim) {
    send_to_char("They aren't here.\n\r", ch);

    ch->bot.misses++;

    return;
  }

  if (victim == ch) {
    send_to_char("You hit yourself... OUCH!\n\r", ch);
    act("$n hits $mself, and says 'OUCH!'", FALSE, ch, 0, victim, TO_ROOM);

    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) && (GET_MASTER(ch) == victim)) {
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) &&
      GET_WEAPON(ch) &&
      IS_SET(OBJ_EXTRA_FLAGS(GET_WEAPON(ch)), ITEM_ANTI_MORTAL)) {
    send_to_char("Perhaps you shouldn't be using an ANTI-MORTAL weapon.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if ((GET_POS(ch) < POSITION_STANDING) || (GET_OPPONENT(ch) == victim)) {
    send_to_char("You do the best you can!\n\r", ch);

    if (!IS_NPC(ch)) {
      ch->bot.misses++;
    }

    return;
  }

  /* Magic Weapon */
  if (((CHAOSMODE && !GET_OPPONENT(victim)) || IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CHAOTIC) || IS_NPC(victim)) &&
      GET_WEAPON(ch) &&
      (OBJ_VALUE0(GET_WEAPON(ch)) != 0) &&
      (OBJ_VALUE0(GET_WEAPON(ch)) <= 20) &&
      !number(0, 3)) {
    switch (OBJ_VALUE0(GET_WEAPON(ch))) {
      case 1:
        spell_blindness(30, ch, victim, 0);
        break;
      case 2:
        if (!IS_NPC(ch)) break;
        spell_poison(30, ch, victim, 0);
        break;
      case 3:
        if (chance(90)) break; /* 2.5% chance on kill. */
        spell_vampiric_touch(30, ch, victim, 0);
        break;
      case 4:
        spell_chill_touch(30, ch, victim, 0);
        break;
      case 5:
        spell_forget(30, ch, victim, 0);
        break;
      case 6:
        spell_curse(30, ch, victim, 0);
        break;
      case 7:
        if (chance(96)) break; /* 1% chance on kill. */
        spell_mana_transfer(30, victim, ch, 0);
        break;
      case 8: /* TODO: Add energy drain. */
        break;
      case 9:
        if (chance(60)) break; /* 15% chance on kill. */
        spell_power_word_kill(GET_LEVEL(ch), ch, victim, 0);
        break;
      default:
        break;
    }
  }

  hit(ch, victim, TYPE_UNDEFINED);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}


void do_kill(CHAR *ch, char *argument, int cmd) {
  char arg[MSL];
  char buf[MSL];
  CHAR *victim = NULL;
  OBJ *weapon = NULL;

  one_argument(argument, arg);

  if ((weapon = GET_WEAPON(ch))) {
    if (OBJ_ACTION(weapon)) {
      if (*arg && (victim = get_char_room_vis(ch, arg))) {
        sprintf(buf, "%s", OBJ_ACTION(weapon));

        act(buf, FALSE, ch, 0, victim, TO_NOTVICT);
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
      }
      else {
        if (OBJ_ACTION_NT(weapon)) {
          sprintf(buf, "%s", OBJ_ACTION_NT(weapon));
        }
        else {
          sprintf(buf, "%s", OBJ_ACTION(weapon));
        }

        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
      }
    }
  }

  if ((GET_CLASS(ch) == CLASS_NINJA) && (weapon = EQ(ch, HOLD))) {
    if ((GET_ITEM_TYPE(weapon) == ITEM_WEAPON) && OBJ_ACTION(weapon)) {
      if (*arg && (victim = get_char_room_vis(ch, arg))) {
        sprintf(buf, "%s", OBJ_ACTION(weapon));

        act(buf, FALSE, ch, 0, victim, TO_NOTVICT);
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
      }
      else {
        if (OBJ_ACTION_NT(weapon)) {
          sprintf(buf, "%s", OBJ_ACTION_NT(weapon));
        }
        else {
          sprintf(buf, "%s", OBJ_ACTION(weapon));
        }

        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
      }
    }
  }

  if (IS_IMPLEMENTOR(ch)) {
    if (!*arg) {
      send_to_char("Kill who?\n\r", ch);

      return;
    }

    if (!(victim = get_char_room_vis(ch, arg))) {
      send_to_char("They aren't here.\n\r", ch);

      return;
    }

    if (ch == victim) {
      send_to_char("Your mother would be so sad... :(\n\r", ch);

      return;
    }

    act("You chop $M to pieces! Ah! The blood!", FALSE, ch, 0, victim, TO_CHAR);
    act("$N chops you to pieces!", FALSE, ch, 0, victim, TO_VICT);
    act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);

    signal_char(victim, ch, MSG_DIE, "");
    divide_experience(ch, victim, TRUE);
    raw_kill(victim);

    return;
  }

  do_hit(ch, argument, 0);
}


void do_wound(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  char number[MIL];
  CHAR *victim = NULL;
  int dmg = 0;

  if (IS_NPC(ch)) return;

  if ((GET_LEVEL(ch) < LEVEL_SUP) && !IS_SET(GET_IMM_FLAGS(ch), WIZ_CREATE)) {
    send_to_char("You need a CREATE flag to use this command.\n\r", ch);

    return;
  }

  argument_interpreter(argument, name, number);

  if (!*name) {
    send_to_char("Usage: wound <name> <damage>.\n\r", ch);

    return;
  }

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Eh!!? That person isn't here you know.\n\r", ch);

    return;
  }

  if (!*number) {
    dmg = GET_MAX_HIT(victim) / 10;
  }
  else {
    dmg = atoi(number);
  }

  sprintf(name, "WIZINFO: %s wounds %s", GET_NAME(ch), GET_NAME(victim));
  log_s(name);

  act("You gesture towards $N, tearing away some of $S lifeforce!\n\r", TRUE, ch, 0, victim, TO_CHAR);
  act("$n gestures towards you and drains away some of your lifeforce!\n\r", TRUE, ch, 0, victim, TO_VICT);
  act("$n gestures slightly towards $N who screams in pain!\n\r", TRUE, ch, 0, victim, TO_NOTVICT);

  GET_HIT(victim) = MAX(0, GET_HIT(victim) - dmg);
}


void do_spin_kick(CHAR *ch, char *argument, int cmd) {
  CHAR *tmp_victim = NULL;
  CHAR *next_victim = NULL;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_CLASS(ch) != CLASS_PALADIN)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  check = number(1, 101) - GET_DEX_APP(ch);

  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 10;
  }

  if (((GET_LEARNED(ch, SKILL_KICK) < MAX_PRAC(ch)) && (check > GET_LEARNED(ch, SKILL_KICK)) && (check > GET_LEARNED(ch, SKILL_SPIN_KICK))) ||
      (check > GET_LEARNED(ch, SKILL_SPIN_KICK))) {
    if (!GET_MOUNT(ch)) {
      send_to_char("You try to do a spin-kick, but fail and hit your head on the ground.\n\r", ch);
      act("$n tries to do a spin-kick, but fails and falls on the ground.", FALSE, ch, 0, 0, TO_ROOM);

      GET_POS(ch) = POSITION_SITTING;
    }
    else {
      send_to_char("You try to do a spin-kick, but fail.\n\r", ch);
      act("$n tries to do a spin-kick, but fails.", FALSE, ch, 0, 0, TO_ROOM);
    }

    skill_wait(ch, SKILL_SPIN_KICK, 3);
  }
  else {
    send_to_char("Your spin-kick has generated a big whirl.\n\r", ch);
    act("$n's spin-kick has generated a big whirl.", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim) {
      next_victim = tmp_victim->next_in_room;

      if ((tmp_victim == ch) ||
          (IS_NPC(tmp_victim) && (GET_RIDER(tmp_victim) == ch)) ||
          (IS_MORTAL(tmp_victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) ||
          (IS_MORTAL(tmp_victim) && GET_OPPONENT(tmp_victim) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))) {
        continue;
      }

      act("You have been kicked by $n.", FALSE, ch, 0, tmp_victim, TO_VICT);

      damage(ch, tmp_victim, calc_position_damage(GET_POS(tmp_victim), MIN(GET_LEVEL(ch) * 2, 60)), TYPE_UNDEFINED, DAM_SKILL);
    }

    auto_learn_skill(ch, SKILL_SPIN_KICK);

    skill_wait(ch, SKILL_SPIN_KICK, 4);
  }
}


void do_backstab(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_AVATAR)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Backstab who?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("How can you sneak up on yourself!?\n\r", ch);
    return;
  }

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon to backstab someone.\n\r", ch);
    return;
  }

  if ((get_weapon_type(GET_WEAPON(ch)) != TYPE_PIERCE) &&
      (get_weapon_type(GET_WEAPON(ch)) != TYPE_STING)) {
    send_to_char("Only pointy weapons can be used for backstabbing.\n\r", ch);
    return;
  }

  /* Assassinate */
  if (GET_OPPONENT(victim))
  {
    if (!GET_LEARNED(ch, SKILL_ASSASSINATE)) {
      send_to_char("You can't backstab someone engaged in combat; they're too alert!\n\r", ch);
      return;
    }

    if (IS_MORTAL(ch) &&
        CAN_SEE(victim, ch) &&
        !affected_by_spell(ch, AFF_SNEAK) &&
        !affected_by_spell(ch, SPELL_IMP_INVISIBLE) &&
        !affected_by_spell(ch, SPELL_BLACKMANTLE)) {
      act("Maybe if $E couldn't see you, or in the cover of darkness...", FALSE, ch, 0, victim, TO_CHAR);
      return;
    }
  }

  check = (number(1, 151) - GET_DEX_APP(ch));

  if (!CAN_SEE(victim, ch) ||
      affected_by_spell(ch, AFF_SNEAK) ||
      affected_by_spell(ch, SPELL_INVISIBLE) ||
      affected_by_spell(ch, SPELL_IMP_INVISIBLE)) {
    check -= 5;
  }

  if (affected_by_spell(ch, SKILL_VEHEMENCE) ||
      check_subclass(ch, SC_INFIDEL, 1)) {
    check -= (5 + (GET_DEX_APP(ch) / 2));
  }

  if (IS_IMMUNE(victim, IMMUNE_BACKSTAB) ||
      (AWAKE(victim) && (check > GET_LEARNED(ch, SKILL_BACKSTAB)))) {
    damage(ch, victim, 0, SKILL_BACKSTAB, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_BACKSTAB, 2);
  }
  else {
    /* Assassinate */
    if (GET_OPPONENT(victim)) {
      hit(ch, victim, SKILL_ASSASSINATE);
    }
    else {
      hit(ch, victim, SKILL_BACKSTAB);
    }

    auto_learn_skill(ch, SKILL_BACKSTAB);

    skill_wait(ch, SKILL_BACKSTAB, 3);
  }
}

void do_assassinate(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  char buf[MIL];
  int dir = NOWHERE;
  int room = NOWHERE;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      (GET_LEVEL(ch) < 45)) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  if (GET_MOUNT(ch)) {
    send_to_char("You must dismount first.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      !IS_AFFECTED(ch, AFF_SNEAK) &&
      !affected_by_spell(ch, SPELL_INVISIBLE) &&
      !affected_by_spell(ch, SPELL_IMP_INVISIBLE)) {
    send_to_char("You need to be sneaking or invisible to succeed.\n\r", ch);

    return;
  }

  argument = one_argument(argument, name);

  if (!*name) {
    send_to_char("Assassinate who?\n\r", ch);

    return;
  }

  one_argument(argument, buf);

  if (!*buf) {
    send_to_char("What direction?\n\r", ch);

    return;
  }

  if (is_abbrev(buf, "north")) dir = CMD_NORTH;
  else if (is_abbrev(buf, "east")) dir = CMD_EAST;
  else if (is_abbrev(buf, "south")) dir = CMD_SOUTH;
  else if (is_abbrev(buf, "west")) dir = CMD_WEST;
  else if (is_abbrev(buf, "up")) dir = CMD_UP;
  else if (is_abbrev(buf, "down")) dir = CMD_DOWN;

  if (dir == NOWHERE) {
    send_to_char("What direction!?\n\r", ch);

    return;
  }

  check = number(1, 111) - GET_DEX_APP(ch);

  if (check_subclass(ch, SC_INFIDEL, 1)) {
    check -= 5;
  }

  if (check > GET_LEARNED(ch, SKILL_ASSASSINATE)) {
    send_to_char("You fail your assassination attempt.\n\r", ch);

    skill_wait(ch, SKILL_ASSASSINATE, 2);

    return;
  }

  room = CHAR_REAL_ROOM(ch);

  do_move(ch, "", dir);

  if (room != CHAR_REAL_ROOM(ch)) {
    do_backstab(ch, name, CMD_BACKSTAB);
  }
}


void do_ambush(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  char name[256];
  int percent;

  if (!ch->skills)
    return;

  one_argument(argument, name);

  if ((GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_LEVEL(ch) < LEVEL_IMM)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Ambush who?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("How can you sneak up on yourself?\n\r", ch);
    return;
  }

  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon for it to succeed.\n\r", ch);
    return;
  }

  if (victim->specials.fighting) {
    send_to_char("You can't ambush a fighting person, too alert!\n\r", ch);
    return;
  }

  percent = number(1, 151) - GET_DEX_APP(ch); /* 101% is a complete failure */

  if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_FIELD) percent = (percent - 2); /* field advantage */
  if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_HILLS) percent = (percent - 3); /* hills advantage */
  if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_MOUNTAIN) percent = (percent - 5); /* mtn advantage */
  if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_FOREST) percent = (percent - 7); /* forest advantage */

  if ((AWAKE(victim) && (percent > ch->skills[SKILL_AMBUSH].learned)) ||
      IS_SET(victim->specials.immune, IMMUNE_AMBUSH)) {
    act("You try to ambush $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to ambush you, but fails.", FALSE, victim, 0, ch, TO_CHAR);
    act("$n tries to ambush $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_AMBUSH, DAM_NO_BLOCK);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
  }
  else {
    hit(ch, victim, SKILL_AMBUSH);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  }


}


void do_assault(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int modifier = 0;
  bool assault = FALSE;
  bool dual_assault = FALSE;

  if (!GET_SKILLS(ch)) return;

  one_argument(arg, name);

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_COMMANDO)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Assault who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("Would that qualify as domestic abuse?\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim)) {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (!IS_WEAPON(GET_WEAPON(ch))) {
    send_to_char("You need to wield a weapon for your assault to succeed.\n\r", ch);

    return;
  }

  modifier -= GET_DEX_APP(ch);

  if (affected_by_spell(ch, SPELL_BLUR)) {
    modifier -= GET_LEVEL(ch) / 5;
  }

  if (IS_NPC(victim) && !IS_IMMUNE(victim, IMMUNE_ASSAULT)) {
    if (AWAKE(victim)) {
      if ((number(1, 151) + modifier) <= GET_LEARNED(ch, SKILL_ASSAULT)) {
        assault = TRUE;
      }

      /* Dual Assault */
      if (check_subclass(ch, SC_RONIN, 3)) {
        if ((number(1, 151) + modifier) <= GET_LEARNED(ch, SKILL_ASSAULT)) {
          dual_assault = TRUE;
        }
      }
    }
    else {
      assault = TRUE;

      /* Dual Assault */
      if (check_subclass(ch, SC_RONIN, 3)) {
        dual_assault = TRUE;
      }
    }
  }

  if (!assault) {
    act("You try to assault $N, but fail and $N fights back.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to assault you, but fails and you fight back.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to assault $N, but fails and $N fights back.", FALSE, ch, 0, victim, TO_NOTVICT);
  }
  else {
    hit(ch, victim, SKILL_ASSAULT);

    auto_learn_skill(ch, SKILL_ASSAULT);
  }

  /* Dual Assault */
  if (check_subclass(ch, SC_RONIN, 3)) {
    if (!dual_assault) {
      act("You try to assault $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n tries to assault you, but fails.", FALSE, ch, 0, victim, TO_VICT);
      act("$n tries to assault $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    else {
      /* A bit of a hack, but this bypasses the check for the 2nd weapon and Mystic Swiftness. */
      qhit(ch, victim, SKILL_ASSAULT);
    }
  }

  if (!assault) {
    hit(victim, ch, TYPE_UNDEFINED);
  }

  skill_wait(ch, SKILL_ASSAULT, (assault || dual_assault) ? 3 : 2);
}


void do_circle(CHAR *ch, char *argument, int cmd) {
  CHAR *victim = NULL;
  char name[MIL];
  int check = 0;
  int set_pos = 0;

  if (!GET_SKILLS(ch)) return;

  one_argument(argument, name);

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_AVATAR)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
    send_to_char("Circle who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You briefly consider spinning around in a circle forever...\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim)) {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon for your circle to succeed.\n\r", ch);

    return;
  }

  if ((get_weapon_type(GET_WEAPON(ch)) != TYPE_PIERCE) &&
      (get_weapon_type(GET_WEAPON(ch)) != TYPE_STING)) {
    send_to_char("Only piercing weapons can be used to stab someone in the back.\n\r", ch);

    return;
  }

  check = number(1, 191) - GET_DEX_APP(ch);

  if (GET_POS(victim) < POSITION_RESTING) {
    check -= 50;
  }

  if (GET_CLASS(ch) == CLASS_THIEF) {
    check -= GET_LEVEL(ch) / 2;
  }

  if (affected_by_spell(ch, SKILL_VEHEMENCE)) {
    check -= 5 + (GET_DEX_APP(ch) / 2);
  }

  set_pos = stack_position(victim, POSITION_RESTING);

  if ((check > GET_LEARNED(ch, SKILL_CIRCLE)) || (IS_NPC(victim) && IS_IMMUNE2(victim, IMMUNE_CIRCLE))) {
    act("$n slips quietly into the shadows, but $N notices as $e appears behind $M.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n disappears from sight, but you notice as $e appears behind you.", FALSE, ch, 0, victim, TO_VICT);
    act("You slip quietly into the shadows, but $N notices as you appear behind $M.", FALSE, ch, 0, victim, TO_CHAR);

    damage(ch, victim, 0, SKILL_CIRCLE, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_CIRCLE, 2);
  }
  else {
    act("$n vanishes into the shadows, suddenly appearing behind $N.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n disappears into the shadows, vanishing completely from sight.", FALSE, ch, 0, victim, TO_VICT);
    act("You slip into the shadows and vanish, suddenly appearing behind $N.", FALSE, ch, 0, victim, TO_CHAR);

    hit(ch, victim, SKILL_CIRCLE);

    auto_learn_skill(ch, SKILL_CIRCLE);

    skill_wait(ch, SKILL_CIRCLE, 3);
  }

  /* Trip */
  if ((CHAR_REAL_ROOM(victim) != NOWHERE) &&
      check_sc_access(ch, SKILL_TRIP) &&
      affected_by_spell(ch, SKILL_TRIP)) {
    check = number(1, 101) - GET_DEX_APP(ch);

    if (affected_by_spell(ch, SKILL_VEHEMENCE)) {
      check -= 5 + (GET_DEX_APP(ch) / 2);
    }

    if (check <= GET_LEARNED(ch, SKILL_TRIP)) {
      act("You trip $N, causing $M to become off-balanced.", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n trips you, causing you to become off-balanced.", FALSE, ch, NULL, victim, TO_VICT);
      act("$n trips $N, causing $M to become off-balanced.", FALSE, ch, NULL, victim, TO_NOTVICT);

      if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = set_pos;
      }
    }
  }
}


void do_order(struct char_data *ch, char *argument, int cmd) {
  char name[100], message[256];
  char buf[256];
  bool found = FALSE;
  int org_room;
  struct char_data *victim;
  struct follow_type *k, *temp;

  half_chop(argument, name, 100, message, 256);

  if (!*name || !*message)
    send_to_char("Order who to do what?\n\r", ch);
  else if (!(victim = get_char_room_vis(ch, name)) &&
           str_cmp("follower", name) && str_cmp("followers", name))
           send_to_char("That person isn't here.\n\r", ch);
  else if (ch == victim)
    send_to_char("You obviously suffer from schizophrenia.\n\r", ch);
  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not approve of you giving orders.\n\r", ch);
      return;
    }
    if (victim) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, victim, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);

      if (((victim->master == ch && IS_AFFECTED(victim, AFF_CHARM)) ||
        (victim->specials.rider == ch)) && IS_NPC(victim)) {
        send_to_char("Ok.\n\r", ch);
        if (strncmp(message, "flex", 4) && (!strncmp(message, "fl", 2) || !strncmp(message, "fle", 3) || !strncmp(message, "flee", 4))) {
          act("The thought of running away makes $n burst into tears.", 0, victim, 0, 0, TO_ROOM);
          act("The thought of running away makes you burst into tears.", 0, victim, 0, 0, TO_CHAR);
          return;
        }
        command_interpreter(victim, message);
      }
      else {
        act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
      }
    }
    else {  /* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, victim, TO_ROOM);

      org_room = CHAR_REAL_ROOM(ch);

      for (k = ch->followers; k; k = temp) {
        temp = k->next; /* added temp - Ranger June 96 */
        if (org_room == CHAR_REAL_ROOM(k->follower))
          if (IS_AFFECTED(k->follower, AFF_CHARM)) {
            found = TRUE;
            if (strncmp(message, "flex", 4) && (!strncmp(message, "fl", 2) || !strncmp(message, "fle", 3) || !strncmp(message, "flee", 4))) {
              act("The thought of running away makes $n burst into tears.", 0, k->follower, 0, 0, TO_ROOM);
              act("The thought of running away makes you burst into tears.", 0, k->follower, 0, 0, TO_CHAR);
              continue;
            }
            command_interpreter(k->follower, message);
          }
      }
      if (found)
        send_to_char("Ok.\n\r", ch);
      else
        send_to_char("Nobody here is a loyal subject of yours!\n\r", ch);
    }
  }
}


#define WALL_THORNS 34
void do_flee(struct char_data *ch, char *argument, int cmd) {
  int i, attempt, loose, die, diff;
  struct char_data *victim = NULL, *vict, *vict_n;
  int org_room;
  bool rider, mount;
  OBJ *wall = 0;

  void gain_exp(struct char_data *ch, int gain);
  int special(struct char_data *ch, int cmd, char *arg);

  rider = FALSE;
  mount = FALSE;
  /* Added check for when rider flees, mount flees, mount flees rider flees
     Ranger April 96  Whole flee thing changed */

  if (IS_AFFECTED(ch, AFF_PARALYSIS) || (IS_AFFECTED(ch, AFF_HOLD) && chance(25))) {
    act("$n tries to flee but seems to be unable to move.", TRUE, ch, 0, 0, TO_ROOM);
    act("You try to flee but are paralyzed and can't move!", TRUE, ch, 0, 0, TO_CHAR);
    return;
  }

  if (CHAOSMODE && chance(25) && ch->specials.fighting && (!check_subclass(ch->specials.fighting, SC_WARLORD, 1))) {  /* Chaos03 */
    act("$n tries to flee but fails.", TRUE, ch, 0, 0, TO_ROOM);
    act("You try to flee but fail!", TRUE, ch, 0, 0, TO_CHAR);
    return;
  }


  if (affected_by_spell(ch, SKILL_BERSERK) ||
      affected_by_spell(ch, SKILL_FRENZY) ||
      affected_by_spell(ch, SKILL_HOSTILE)) {
    act("$n tries to flee but fails.", TRUE, ch, 0, 0, TO_ROOM);
    act("You try to flee but fail!", TRUE, ch, 0, 0, TO_CHAR);
    return;
  }

  if ((wall = get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch)))) {
    send_to_char("The wall of thorns blocks your way. Ouch!\n\r", ch);
    damage(ch, ch, 30, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
    return;
  }

  if (!(ch->specials.fighting)) {
    for (i = 0; i < 6; i++) {
      attempt = number(0, 5);  /* Select a random direction */
      if (CAN_GO(ch, attempt) &&
          !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, DEATH)) {
        act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
        org_room = CHAR_REAL_ROOM(ch);
        mount = FALSE;
        rider = FALSE;
        victim = NULL;
        if (ch->specials.riding) {
          if (org_room == CHAR_REAL_ROOM(ch->specials.riding)) {
            victim = ch->specials.riding;
            mount = TRUE;
          }
        }
        if (ch->specials.rider) {
          victim = ch->specials.rider;
          rider = TRUE;
        }

        if ((die = do_simple_move(ch, attempt, FALSE, 1)) == 1) {
          /* The escape has succeded */
          send_to_char("You flee head over heels.\n\r", ch);
          if ((rider) || (mount)) {
            char_from_room(victim);
            char_to_room(victim, CHAR_REAL_ROOM(ch));
            do_look(victim, "\0", 15);
            if (victim->specials.fighting) {
              if (victim->specials.fighting->specials.fighting == victim)
                stop_fighting(victim->specials.fighting);
              stop_fighting(victim);
            }
          }
          if (!IS_NPC(ch) && IS_SET(world[org_room].room_flags, CHAOTIC)) {
            drain_mana_hit_mv(ch, ch, GET_MANA(ch) / 10, GET_HIT(ch) / 10, GET_MOVE(ch) / 10, FALSE, FALSE, FALSE);
            send_to_char("The gods of Chaos rip some of your lifeforce from you!\n\r", ch);
          }
          return;
        }
        else {
          if (!die) act("$n tries to flee, but is too exhausted!",
                        TRUE, ch, 0, 0, TO_ROOM);
          return;
        }
      }
    } /* for */
    /* No exits was found */
    send_to_char("PANIC! You couldn't escape!\n\r", ch);
    return;
  }

  /* FIGHTING */
  for (i = 0; i < 6; i++) {
    attempt = number(0, 5);
    if (CAN_GO(ch, attempt) &&
        !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, DEATH)) {
      if (!IS_NPC(ch->specials.fighting) && !ch->specials.rider &&
          affected_by_spell(ch->specials.fighting, SKILL_BLOCK) &&
          ((number(1, 101) < ch->specials.fighting->skills[SKILL_BLOCK].learned) ||
          (check_subclass(ch->specials.fighting, SC_WARLORD, 1) && chance(90)))) {
        act("$N tried to flee but $n blocked $S way!",
            FALSE, ch->specials.fighting, 0, ch, TO_NOTVICT);
        act("You tried to flee but $N blocked your way!",
            FALSE, ch, 0, ch->specials.fighting, TO_CHAR);
        act("$N tried to flee but you blocked $S way!",
            FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
        return;
      }
      act("$n panics and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
      org_room = CHAR_REAL_ROOM(ch);
      mount = FALSE;
      rider = FALSE;
      victim = NULL;

      if (ch->specials.riding) {
        if (org_room == CHAR_REAL_ROOM(ch->specials.riding)) {
          victim = ch->specials.riding;
          mount = TRUE;
        }
      }
      if (ch->specials.rider) {
        victim = ch->specials.rider;
        rider = TRUE;
      }

      /* Liner Jan 03, adding flee exp loss tables to vary exp loss depending on level
      differences in ch and ch->specials.fighting */

      if ((die = do_simple_move(ch, attempt, FALSE, 1)) == 1) {
        /* The escape has succeeded */
        if (ch->specials.fighting) { /* Another check */
          if (GET_LEVEL(ch) > GET_LEVEL(ch->specials.fighting)) {
            diff = (GET_LEVEL(ch) - GET_LEVEL(ch->specials.fighting));
            diff = diff / 10; /* gives integer from 0-5 of difference in levels */
            switch (diff) {
              case 0: /* 1-4 levels below, lose 1/5 normal */
                loose = (GET_EXP(ch->specials.fighting) / 50);
                break;
              case 1: /* 5-14 levels below, lose 1/10 normal */
                loose = (GET_EXP(ch->specials.fighting) / 100);
                break;
              case 2: /* 15-24 levels below, lose 1/50 normal */
                loose = (GET_EXP(ch->specials.fighting) / 500);
                break;
              case 3: /* 25-34 levels below, lose 1/100 normal */
                loose = (GET_EXP(ch->specials.fighting) / 1000);
                break;
              case 4: /* 35-44 levels below, lose 1/500 normal */
                loose = (GET_EXP(ch->specials.fighting) / 5000);
                break;
              case 5: /* 45-49 levels below, lose 1/1000 normal */
                loose = (GET_EXP(ch->specials.fighting) / 10000);
                break;
              default:
                loose = 0;
                break;
            }
          }
          else loose = (GET_EXP(ch->specials.fighting) / 10);
          if (IS_SET(world[org_room].room_flags, CHAOTIC)) loose = 0;
          if (GET_LEVEL(ch) < 15) loose = 0;   /* flee below level 15, no exp loss */

          if (!IS_NPC(ch) && IS_NPC(ch->specials.fighting)) {
            if (loose > (GET_EXP(ch) / 2)) { /* if loss is greater than half of ch's xp */
              loose = MIN(loose, GET_EXP(ch) / 2);
              gain_exp(ch, -loose); /* loss is now no greater than half of ch's xp */
            }
            else {
              gain_exp(ch, -loose); /* loss is less than half of ch's xp, no worries */
            }
            gain_exp(ch->specials.fighting, loose); /* exp is now retainable */
          }

          /* Insert later when using hunting system       */
          /* ch->specials.fighting->specials.hunting = ch */

          for (vict = world[org_room].people; vict; vict = vict_n) {
            vict_n = vict->next_in_room;
            if (vict->specials.fighting == ch)
              stop_fighting(vict);
          }
          stop_fighting(ch);
        }
        send_to_char("You flee head over heels.\n\r", ch);

        if ((rider) || (mount)) {
          char_from_room(victim);
          char_to_room(victim, CHAR_REAL_ROOM(ch));
          do_look(victim, "\0", 15);
          if (victim->specials.fighting) {
            if (victim->specials.fighting->specials.fighting == victim)
              stop_fighting(victim->specials.fighting);
            stop_fighting(victim);
          }
        }
        return;
      }
      else {
        if (!die) act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
        return;
      }
    }
  } /* for */
  /* No exits was found */
  send_to_char("PANIC! You couldn't escape!\n\r", ch);
}

#define QGII_GRUUMSH 1060

void do_pummel(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_COMMANDO)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
    send_to_char("Pummel who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim)) {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (!IS_WEAPON(GET_WEAPON(ch))) {
    send_to_char("You need to wield a weapon for your pummel to succeed.\n\r", ch);

    return;
  }

  check = number(1, 121) - GET_DEX_APP(ch);

  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 10;
  }

  if (((GET_LEARNED(ch, SKILL_BASH) < MAX_PRAC(ch)) && (check > GET_LEARNED(ch, SKILL_BASH)) && (check > GET_LEARNED(ch, SKILL_PUMMEL))) ||
      (check > GET_LEARNED(ch, SKILL_PUMMEL))) {
    act("You try to pummel $N, but miss.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n tried to pummel you, but missed.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n tried to pummel $N, but missed.", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_PUMMEL, DAM_NO_BLOCK);
  }
  else {
    if ((IS_NPC(victim) && IS_IMMUNE(victim, IMMUNE_PUMMEL)) ||
        !breakthrough(ch, victim, BT_INVUL)) {
      act("You pummel $N, but your pummel has no effect!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n pummels you, but $s pummel has no effect!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n pummels $N, but $s pummel has no effect!", FALSE, ch, NULL, victim, TO_NOTVICT);

      damage(ch, victim, 0, SKILL_PUMMEL, DAM_NO_BLOCK);
    }
    else {
      OBJ *hands = NULL;

      set_pos = stack_position(victim, POSITION_STUNNED);

      if ((NULL != (hands = EQ(ch, WEAR_HANDS))) &&
          (QGII_GRUUMSH == V_OBJ(hands)) &&
          chance(15)) {
        set_pos = POSITION_MORTALLYW;
      }

      act("You pummel $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n pummels you, and you are stunned now!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n pummels $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_NOTVICT);

      damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_PUMMEL, DAM_PHYSICAL);

      if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = set_pos;

        /* Can't use skill_wait() since this applies to victim. */
        WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
      }

      auto_learn_skill(ch, SKILL_PUMMEL);
    }
  }

  /* Trusty Steed */
  if ((CHAR_REAL_ROOM(victim) != NOWHERE) &&
      affected_by_spell(ch, SKILL_TRUSTY_STEED) &&
      breakthrough(ch, victim, BT_INVUL)) {
    check = number(1, 121) - GET_WIS_APP(ch);

    if (check <= GET_LEARNED(ch, SKILL_TRUSTY_STEED)) {
      set_pos = stack_position(victim, POSITION_RESTING);

      act("You summon forth your trusty steed and it tramples $N with spiritual energy!", 0, ch, 0, victim, TO_CHAR);
      act("$n summons forth $s trusty steed and it tramples you with spiritual energy!", 0, ch, 0, victim, TO_VICT);
      act("$n summons forth $s trusty steed and it tramples $N with spiritual energy!", 0, ch, 0, victim, TO_NOTVICT);

      damage(ch, victim, calc_position_damage(GET_POS(victim), lround(GET_LEVEL(ch) * 1.5)), SKILL_TRUSTY_STEED, DAM_PHYSICAL);

      if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = set_pos;
      }
    }
  }

  skill_wait(ch, SKILL_PUMMEL, 2);
}


void do_bash(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_CLERIC) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_COMMANDO)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) == CLASS_CLERIC) &&
      (GET_LEVEL(ch) < 35)) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
    send_to_char("Bash who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You're feeling a little... bash-ful...\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim)) {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (!IS_WEAPON(GET_WEAPON(ch))) {
    send_to_char("You need to wield a weapon for your bash to succeed.\n\r", ch);

    return;
  }

  check = number(1, 101) - GET_DEX_APP(ch);

  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 10;
  }

  if (check > GET_LEARNED(ch, SKILL_BASH)) {
    damage(ch, victim, 0, SKILL_BASH, DAM_NO_BLOCK);
  }
  else {
    if ((IS_NPC(victim) && IS_IMMUNE(victim, IMMUNE_PUMMEL)) ||
        !breakthrough(ch, victim, BT_INVUL)) {
      damage(ch, victim, 0, SKILL_BASH, DAM_NO_BLOCK);
    }
    else {
      set_pos = stack_position(victim, POSITION_RESTING);

      damage(ch, victim, calc_position_damage(GET_POS(victim), number(1, GET_LEVEL(ch))), SKILL_BASH, DAM_PHYSICAL);

      auto_learn_skill(ch, SKILL_BASH);

      if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = set_pos;

        /* Can't use skill_wait() since this applies to victim. */
        WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
      }
    }
  }

  skill_wait(ch, SKILL_BASH, 2);
}


void do_punch(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!GET_SKILLS(ch)) return;

  one_argument(arg, name);

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_AVATAR)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
    send_to_char("Punch who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You aren't feeling well... are you?\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim)) {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  check = number(1, 101) - GET_DEX_APP(ch) - (GET_LEVEL(ch) / 20);

  if (check > GET_LEARNED(ch, SKILL_PUNCH)) {
    damage(ch, victim, 0, SKILL_PUNCH, DAM_NO_BLOCK);
  }
  else {
    if ((IS_NPC(victim) && IS_IMMUNE(victim, IMMUNE_PUNCH)) ||
        !breakthrough(ch, victim, BT_INVUL)) {
      damage(ch, victim, 0, SKILL_PUNCH, DAM_NO_BLOCK);
    }
    else {
      set_pos = stack_position(victim, POSITION_SITTING);

      damage(ch, victim, calc_position_damage(GET_POS(victim), GET_LEVEL(ch) * 2), SKILL_PUNCH, DAM_PHYSICAL);

      auto_learn_skill(ch, SKILL_PUNCH);

      if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = set_pos;

        /* Can't use skill_wait() since this applies to victim. */
        WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
      }
    }
  }

  skill_wait(ch, SKILL_PUNCH, 2);
}


void do_rescue(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  CHAR *tmp_ch = NULL;
  int check;

  if (!GET_SKILLS(ch)) return;

  if (CHAOSMODE) {
    send_to_char("Rescue? During Chaos!? It'd be better to just commit suicide...\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {

    send_to_char("Who do you want to rescue?\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_COMMANDO) &&
      (IS_MORTAL(victim) && (GET_LEVEL(victim) > 15))) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("What about fleeing instead?\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch) == victim) {
    send_to_char("How can you rescue someone you are trying to kill?\n\r", ch);

    return;
  }

  for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch && (GET_OPPONENT(tmp_ch) != victim); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if (affected_by_spell(ch, SKILL_HOSTILE)) {
    send_to_char("Your hostility prevents the rescue.\n\r", ch);

    return;
  }

  check = (IS_MORTAL(victim) && (GET_LEVEL(victim) < 16)) ? 0 : number(1, 101);

  if (check > GET_LEARNED(ch, SKILL_RESCUE)) {
    send_to_char("You fail the rescue.\n\r", ch);
  }
  else {
    send_to_char("Banzai! To the rescue...\n\r", ch);
    act("You are rescued by $n! You are confused!", FALSE, ch, 0, victim, TO_VICT);
    act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);

    if (GET_OPPONENT(victim) == tmp_ch) {
      stop_fighting(victim);
    }

    if (GET_OPPONENT(tmp_ch)) {
      stop_fighting(tmp_ch);
    }

    if (GET_OPPONENT(ch)) {
      stop_fighting(ch);
    }

    set_fighting(ch, tmp_ch);
    set_fighting(tmp_ch, ch);

    auto_learn_skill(ch, SKILL_RESCUE);

    /* Can't use skill_wait() since this applies to victim. */
    if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
      WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
    }
  }
}


void do_assist(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  CHAR *tmp_ch = NULL;

  if (CHAOSMODE) {
    send_to_char("Assist? All you can think about is KILLING!\n\r", ch);

    return;
  }

  if ((GET_POS(ch) == POSITION_FIGHTING) || GET_OPPONENT(ch)) {
    send_to_char("You are fighting already!\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Who do you want to assist?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You're doing the best you can...\n\r", ch);

    return;
  }

  for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch && (GET_OPPONENT(tmp_ch) != victim); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  send_to_char("You join the fight!\n\r", ch);
  act("$n assists you!", FALSE, ch, 0, victim, TO_VICT);
  act("$n assists $N.", FALSE, ch, 0, victim, TO_NOTVICT);

  set_fighting(ch, tmp_ch);
}


void do_kick(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_COMMANDO)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
    send_to_char("Kick who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You're here to kick ass, not yourself...\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim)) {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  check = ((10 - (GET_AC(victim) / 10)) * 2) + number(1, 101) - GET_DEX_APP(ch);

  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 10;
  }

  if (check > GET_LEARNED(ch, SKILL_KICK)) {
    damage(ch, victim, 0, SKILL_KICK, DAM_NO_BLOCK);
  }
  else {
    if ((IS_NPC(victim) && IS_IMMUNE(victim, IMMUNE_KICK)) ||
        !breakthrough(ch, victim, BT_INVUL)) {
      damage(ch, victim, 0, SKILL_KICK, DAM_NO_BLOCK);
    }
    else {
      damage(ch, victim, calc_position_damage(GET_POS(victim), MIN(GET_LEVEL(ch), 30) * 2), SKILL_KICK, DAM_PHYSICAL);

      auto_learn_skill(ch, SKILL_KICK);

      /* Can't use skill_wait() since this applies to victim. */
      if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
        WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(2, 3) : 3));
      }
    }
  }

  skill_wait(ch, SKILL_KICK, 2);
}


void do_disarm(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  char name[256];
  int percent;
  struct obj_data *wield = 0;

  if ((GET_CLASS(ch) != CLASS_NOMAD) && (GET_LEVEL(ch) < LEVEL_IMM) &&
      (GET_CLASS(ch) != CLASS_COMMANDO) &&
      (GET_CLASS(ch) != CLASS_PALADIN) && (GET_CLASS(ch) != CLASS_AVATAR)) {
    send_to_char("You better leave this job to the others.\n\r", ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    }
    else {
      send_to_char("Disarm who?\n\r", ch);
      return;
    }
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!IS_NPC(ch) && !IS_NPC(victim) &&
      IS_SET(ch->specials.pflag, PLR_NOKILL) &&
      !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA) &&
      !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
      !CHAOSMODE &&
      (!IS_SET(victim->specials.pflag, PLR_THIEF) ||
      !IS_SET(victim->specials.pflag, PLR_KILL))) {
    send_to_char("You can't attack other players.\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  if (victim->equipment[WIELD])
    wield = victim->equipment[WIELD];
  else {
    send_to_char("Disarm a person who is not wielding anything??\n\r", ch);
    return;
  }

  percent = number(1, 200) - GET_DEX_APP(ch); /* 101% is a complete failure */

  if (GET_LEVEL(victim) < GET_LEVEL(ch))
    percent = percent - (GET_LEVEL(ch) - GET_LEVEL(victim));

  if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    percent = number(1, 800);

  if (V_OBJ(wield) == 11523 || (ch->skills ? (percent > ch->skills[SKILL_DISARM].learned) ||
    (IS_SET(victim->specials.act, ACT_ARM) && IS_NPC(victim)) ||
    ((GET_LEVEL(victim) - GET_LEVEL(ch)) > 5) || GET_LEVEL(victim) > 30 : percent > 100)) {
    act("You tried to kick off $N's weapon, but failed!",
        FALSE, ch, 0, victim, TO_CHAR);
    act("$N tried to kick off your weapon, but failed!",
        FALSE, victim, 0, ch, TO_CHAR);
    act("$n tried to kick off $N's weapon, but failed!",
        FALSE, ch, 0, victim, TO_NOTVICT);
    hit(ch, victim, TYPE_UNDEFINED);
  }
  else {
    act("Your beautiful side-kick has kicked off $N's weapon!",
        FALSE, ch, 0, victim, TO_CHAR);
    act("$N kicked off your weapon!",
        FALSE, victim, 0, ch, TO_CHAR);
    act("$n kicked off $N's weapon by a beautiful side-kick!",
        FALSE, ch, 0, victim, TO_NOTVICT);

    if (ch->skills ? ch->skills[SKILL_DISARM].learned < 85 : FALSE)
      ch->skills[SKILL_DISARM].learned += 2;

    unequip_char(victim, WIELD);
    if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC)) obj_to_char(wield, victim);
    else {
      log_f("WIZLOG: %s disarms %s's %s (Room %d).", GET_NAME(ch), GET_NAME(victim), OBJ_SHORT(wield), world[CHAR_REAL_ROOM(victim)].number);
      obj_to_room(wield, CHAR_REAL_ROOM(victim));
      wield->log = 1;
    }
    save_char(victim, NOWHERE);
    hit(ch, victim, TYPE_UNDEFINED);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


void do_disembowel(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  one_argument(argument, name);

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_NOMAD)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (((GET_CLASS(ch) != CLASS_WARRIOR) && (GET_LEVEL(ch) < 40)) ||
      ((GET_CLASS(ch) != CLASS_NOMAD) && (GET_LEVEL(ch) < 20))) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
    send_to_char("Disembowel who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You might want to contact the suicide hotline...\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim)) {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (!IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(victim))) {
    send_to_char("You can't disembowel another player.\n\r", ch);

    return;
  }

  if (!IS_WEAPON(GET_WEAPON(ch))) {
    send_to_char("You need to wield a weapon for your disembowel to succeed.\n\r", ch);

    return;
  }

  if ((get_weapon_type(GET_WEAPON(ch)) == TYPE_HIT) ||
      (get_weapon_type(GET_WEAPON(ch)) == TYPE_BLUDGEON) ||
      (get_weapon_type(GET_WEAPON(ch)) == TYPE_CRUSH)) {
    send_to_char("You need a weapon with an edge or a point to disembowel someone.\n\r", ch);

    return;
  }

  check = number(1, 221) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_DISEMBOWEL)) {
    act("You try to disembowel $N, but stumble over your own feet! $N attacks back!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to disembowel you, but stumbles over $s own feet! You attack back!", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to disembowel $N, but stumbles over $s own feet! $N attacks back!", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(victim, ch, ((2 * GET_LEVEL(victim)) + 20), TYPE_UNDEFINED, DAM_SKILL);
  }
  else {
    if ((IS_NPC(victim) && IS_IMMUNE(ch, IMMUNE_DISEMBOWEL)) ||
        (GET_HIT(victim) > (GET_MAX_HIT(victim) / 10))) {
      act("$N avoids your pitiful attack and strikes back at you!", FALSE, ch, 0, victim, TO_CHAR);
      act("You avoid $n's pitiful attack and strike back at $m!", FALSE, ch, 0, victim, TO_VICT);
      act("$N avoids $n's pitiful attack and strikes back at $m!", FALSE, ch, 0, victim, TO_NOTVICT);

      damage(victim, ch, ((2 * GET_LEVEL(victim)) + 20), TYPE_UNDEFINED, DAM_SKILL);
    }
    else {
      act("Your savage attack causes $N's innards to spill out!", FALSE, ch, 0, victim, TO_CHAR);
      act("$n's savage attack causes your innards to spill out!", FALSE, ch, 0, victim, TO_VICT);
      act("$n's savage attack causes $N's innards to spill out!", FALSE, ch, 0, victim, TO_NOTVICT);

      damage(ch, victim, ((GET_MAX_HIT(victim) * 100) / 7), SKILL_DISEMBOWEL, DAM_SKILL);

      auto_learn_skill(ch, SKILL_DISEMBOWEL);

      if (IS_NPC(victim)) {
        SET_BIT(GET_IMMUNE(ch), IMMUNE_DISEMBOWEL);
      }
    }
  }

  skill_wait(ch, SKILL_DISEMBOWEL, 2);
}


void do_backflip(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  one_argument(argument, name);

  if (IS_MORTAL(ch) && (GET_CLASS(ch) != CLASS_BARD)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_LEVEL(ch) < 20) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
    send_to_char("Who do you want to flip over?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("This could prove very interesting...\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim)) {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  check = number(1, 111) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_BACKFLIP)) {
    act("As you spring, $N knocks you down!", FALSE, ch, 0, victim, TO_CHAR);
    act("As $n springs, you knock $m down!", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to spring over $N, but is knocked hard on $s back.", FALSE, ch, 0, victim, TO_NOTVICT);
  }
  else {
    act("With a mighty leap, you spring over $N!", 0, ch, 0, victim, TO_CHAR);
    act("$n crouches low and springs over you!", 0, ch, 0, victim, TO_VICT);
    act("With a mighty leap, $n springs over $N and lands behind $M!", 0, ch, 0, victim, TO_NOTVICT);

    hit(ch, victim, SKILL_BACKFLIP);

    auto_learn_skill(ch, SKILL_BACKFLIP);
  }

  skill_wait(ch, SKILL_BACKFLIP, 2);
}


void do_cunning(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (GET_CLASS(ch) != CLASS_THIEF) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_LEVEL(ch) < 50) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_CUNNING)) {
    affect_from_char(ch, SKILL_CUNNING);

    send_to_char("You relinquish your focus and feel notably less cunning.\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_CUNNING)) {
    send_to_char("You aren't feeling particularly cunning at the moment.\n\r", ch);
  }
  else {
    af.type = SKILL_CUNNING;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You focus on exploiting any weakness in your enemies' defenses and grow more cunning in the process.\n\r", ch);
  }
}

void do_coin_toss(CHAR *ch, char *argument, int cmd) {
  CHAR *tmp_victim = NULL;
  CHAR *next_victim = NULL;
  char arg2[MIL];
  int check = 0;
  int wager = 0;
  int min_wager = 0;
  int max_wager = 0;
  int num_mobs = 0;
  int scattered_coins = 0;
  double factor = 0.0;
  int dmg = 0;

  if (!GET_SKILLS(ch) || (GET_LEVEL(ch) < 1)) return;

  if (IS_MORTAL(ch) && (GET_CLASS(ch) != CLASS_THIEF)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  one_argument(argument, arg2);

  wager = atoi(arg2);

  min_wager = (GET_LEVEL(ch) * 100);
  max_wager = (GET_LEVEL(ch) * 1000);

  if (wager < min_wager) {
    wager = min_wager;
  }
  else if (wager > max_wager) {
    wager = max_wager;
  }

  if (IS_MORTAL(ch) && (wager > GET_GOLD(ch))) {
    send_to_char("You don't have enough coins for such a dastardly attack!\n\r", ch);

    return;
  }

  GET_GOLD(ch) = MAX((GET_GOLD(ch) - wager), 0);
  num_mobs = count_mobs_real_room(CHAR_REAL_ROOM(ch));
  scattered_coins = ((wager * number(10, 25)) / 100);

  check = (number(1, 101) - GET_DEX_APP(ch));

  if (affected_by_spell(ch, SKILL_VEHEMENCE)) {
    check -= (5 + (GET_DEX_APP(ch) / 2));
  }

  if (check > GET_LEARNED(ch, SKILL_COIN_TOSS)) {
    send_to_char("You toss some coins in the air, somehow missing all of your targets.\n\r", ch);
    act("$n tosses some coins in the air, scattering them about carelessly.", FALSE, ch, 0, 0, TO_ROOM);

    if (num_mobs > 0) {
      scattered_coins /= num_mobs;

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim) {
        next_victim = tmp_victim->next_in_room;

        if (IS_MOB(tmp_victim) && (scattered_coins > 0)) {
          act("You snatch some of the coins thrown by $n!", FALSE, ch, 0, tmp_victim, TO_VICT);

          GET_GOLD(tmp_victim) += scattered_coins;
        }
      }
    }
    else {
      send_to_char("Nobody was hit by your coins and they are hopelessly lost in the chaos.\n\r", ch);
    }
  }
  else {
    send_to_char("You toss a hail of coins about you with deadly precision.\n\r", ch);
    act("$n tosses a hail of coins about $m with deadly precision.", FALSE, ch, 0, 0, TO_ROOM);

    if (num_mobs > 0) {
      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim) {
        next_victim = tmp_victim->next_in_room;

        if ((tmp_victim == ch) ||
          (IS_IMMORTAL(tmp_victim)) ||
          (IS_NPC(tmp_victim) && (GET_RIDER(tmp_victim) == ch)) ||
          (IS_MORTAL(tmp_victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) ||
          (IS_MORTAL(tmp_victim) && GET_OPPONENT(tmp_victim) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))) {
          continue;
        }

        act("You are battered by a hail of coins thrown by $n!", FALSE, ch, 0, tmp_victim, TO_VICT);

        if ((wager >= min_wager) && (wager <= max_wager)) {
          factor = (wager / max_wager);
        }

        dmg = MAX(lround(((GET_LEVEL(ch) * 2) * factor)), 10);

        damage(ch, tmp_victim, calc_position_damage(GET_POS(tmp_victim), dmg), TYPE_UNDEFINED, DAM_SKILL);
      }
    }
    else {
      send_to_char("Nobody was hit by your coins and they are hopelessly lost in the chaos.\n\r", ch);
    }
  }

  skill_wait(ch, SKILL_COIN_TOSS, 2);
}
