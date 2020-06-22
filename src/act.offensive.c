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
#include "constants.h"
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

void raw_kill(struct char_data *ch);
int calc_position_damage(int position, int dam);
int stack_position(CHAR *ch, int target_position);


/* Note: Do not use this function on victims; it is designed for skill users only. */
void skill_wait(CHAR *ch, int skill, int wait) {
  if (IS_IMPLEMENTOR(ch) || (CHAR_REAL_ROOM(ch) == NOWHERE) || !wait) return;

  /* Quick Recovery */
  if ((wait > 1) && (IS_MORTAL(ch) && check_subclass(ch, SC_ROGUE, 5)) && chance(number(50, 75))) {
    wait -= 1;
  }

  WAIT_STATE(ch, (PULSE_VIOLENCE * wait));
}


void auto_learn_skill(CHAR *ch, int skill) {
  if (GET_LEARNED(ch, skill) < (SKILL_MAX_PRAC - 5)) {
    GET_LEARNED(ch, skill) = MIN(GET_LEARNED(ch, skill) + 2, (SKILL_MAX_PRAC - 5));
  }
}


void do_hit(CHAR *ch, char *argument, int cmd) {
  char name[MSL];

  one_argument(argument, name);

  if (!*name) {
    send_to_char("Hit who?\n\r", ch);

    return;
  }

  CHAR *victim = get_char_room_vis(ch, name);

  if ((victim == ch) && IS_NPC(ch)) {
    victim = get_mortal_room_vis(ch, name);
  }

  if (ch->bot.misses >= 20) {
    log_f("WARNING: %s has 20+ kill/hit misses.", GET_NAME(ch));

    ch->bot.misses = 0;
  }

  if (!victim) {
    send_to_char("They aren't here.\n\r", ch);

    if (!IS_NPC(ch)) {
      ch->bot.misses++;
    }

    return;
  }

  if (victim == ch) {
    send_to_char("You hit yourself...  OUCH!\n\r", ch);
    act("$n hits $mself and says, 'OUCH!'", FALSE, ch, 0, victim, TO_ROOM);

    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) && (GET_MASTER(ch) == victim)) {
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) &&
      GET_WEAPON(ch) &&
      IS_SET(OBJ_EXTRA_FLAGS(GET_WEAPON(ch)), ITEM_ANTI_MORTAL)) {
    send_to_char("Perhaps you shouldn't be using an ANTI-MORTAL weapon!\n\r", ch);

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
  if (GET_WEAPON(ch) &&
      (OBJ_VALUE(GET_WEAPON(ch), 0) != 0) &&
      (OBJ_VALUE(GET_WEAPON(ch), 0) <= 20) &&
      ((CHAOSMODE && !GET_OPPONENT(victim)) || IS_SET(CHAR_ROOM_FLAGS(ch), CHAOTIC) || IS_NPC(victim)) &&
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
        if (chance(90)) break;
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
        if (chance(96)) break;
        spell_mana_transfer(30, victim, ch, 0);
        break;
      case 8: /* TODO: Add energy drain. */
        break;
      case 9:
        if (chance(60)) break;
        spell_power_word_kill(GET_LEVEL(ch), ch, victim, 0);
        break;
    }
  }

  hit(ch, victim, TYPE_UNDEFINED);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}


void do_kill(CHAR *ch, char *argument, int cmd) {
  char buf[MSL], name[MIL];

  one_argument(argument, name);

  CHAR *victim = get_char_room_vis(ch, name);

  /* Print action messages. */
  for (int eq_slot = WIELD; eq_slot <= HOLD; eq_slot++) {
    OBJ *weapon = EQ(ch, eq_slot);

    if (!weapon || !IS_WEAPON(weapon)) continue;

    if (victim && OBJ_ACTION(weapon)) {
      snprintf(buf, sizeof(buf), "%s", OBJ_ACTION(weapon));

      act(buf, FALSE, ch, 0, victim, TO_NOTVICT);
      act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
    else if (!victim && OBJ_ACTION_NT(weapon)) {
      snprintf(buf, sizeof(buf), "%s", OBJ_ACTION_NT(weapon));

      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
    else if (!victim && OBJ_ACTION(weapon)) {
      snprintf(buf, sizeof(buf), "%s", OBJ_ACTION(weapon));

      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }

    if (IS_NPC(ch) || (GET_CLASS(ch) != CLASS_NINJA)) break;
  }

  /* Implementor raw kill. */
  if (IS_IMPLEMENTOR(ch)) {
    if (!*name) {
      send_to_char("Kill who?\n\r", ch);

      return;
    }

    if (!victim) {
      send_to_char("They aren't here.\n\r", ch);

      return;
    }

    if (victim == ch) {
      send_to_char("Your mother would be so sad... :(\n\r", ch);

      return;
    }

    act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, victim, TO_CHAR);
    act("$N chops you to pieces!", FALSE, ch, 0, victim, TO_VICT);
    act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);

    signal_char(victim, ch, MSG_DIE, "");

    divide_experience(ch, victim, TRUE);

    raw_kill(victim);

    return;
  }

  do_hit(ch, argument, CMD_KILL);
}


void do_wound(CHAR *ch, char *argument, int cmd) {
  if (IS_NPC(ch)) return;

  if (!IS_SET(GET_IMM_FLAGS(ch), WIZ_CREATE) && (GET_LEVEL(ch) < LEVEL_SUP)) {
    send_to_char("You need a CREATE flag to use this command.\n\r", ch);

    return;
  }

  char name[MIL], number[MIL];

  argument_interpreter(argument, name, number);

  if (!*name) {
    send_to_char("Usage: wound <name> <damage>\n\r", ch);

    return;
  }

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim) {
    send_to_char("Eh!? That person isn't here you know.\n\r", ch);

    return;
  }

  int dmg = 0;

  if (!*number) {
    dmg = GET_MAX_HIT(victim) / 10;
  }
  else {
    dmg = atoi(number);
  }

  log_f("WIZINFO: %s wounds %s", GET_NAME(ch), GET_NAME(victim));

  act("You gesture towards $N, tearing away some of $S lifeforce!", TRUE, ch, 0, victim, TO_CHAR);
  act("$n gestures towards you and drains away some of your lifeforce!", TRUE, ch, 0, victim, TO_VICT);
  act("$n gestures slightly towards $N, who screams in pain!", TRUE, ch, 0, victim, TO_NOTVICT);

  GET_HIT(victim) = MAX(0, GET_HIT(victim) - dmg);
}


void do_block(CHAR *ch, char *argument, int cmd) {
  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_AVATAR) &&
      ((GET_CLASS(ch) == CLASS_NOMAD) && !check_subclass(ch, SC_RANGER, 3))) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_BLOCK)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_BLOCK);

    send_to_char("You will now let your victim flee.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_BLOCK)) {
    send_to_char("You failed to concentrate on blocking your enemies.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_BLOCK);

  send_to_char("You will now block your enemies if they flee.\n\r", ch);
}


void do_spin_kick(CHAR *ch, char *argument, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

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

  int check = number(1, 101) - GET_DEX_APP(ch);

  /* Juggernaut */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 4)) {
    check -= 10;
  }

  /* Blur */
  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 7;
  }

  if (check > GET_LEARNED(ch, SKILL_SPIN_KICK)) {
    if (!GET_MOUNT(ch)) {
      send_to_char("You try to do a spin-kick, but fail and hit your head on the ground.\n\r", ch);
      act("$n tries to do a spin-kick, but fails and hits $s head on the ground.", FALSE, ch, 0, 0, TO_ROOM);

      GET_POS(ch) = POSITION_SITTING;
    }
    else {
      send_to_char("You try to do a spin-kick, but fail.\n\r", ch);
      act("$n tries to do a spin-kick, but fails.", FALSE, ch, 0, 0, TO_ROOM);
    }

    int set_wait = 3;

    /* Juggernaut */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 4)) {
      set_wait = 2;
    }

    skill_wait(ch, SKILL_SPIN_KICK, set_wait);

    return;
  }

  send_to_char("Your spin-kick has generated a big whirl.\n\r", ch);
  act("$n's spin-kick has generated a big whirl.", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_victim = world[CHAR_REAL_ROOM(ch)].people, *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) ||
        (IS_NPC(temp_victim) && (GET_RIDER(temp_victim) == ch)) ||
        (IS_MORTAL(temp_victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) ||
        (IS_MORTAL(temp_victim) && GET_OPPONENT(temp_victim) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))) {
      continue;
    }

    act("You have been kicked by $n.", FALSE, ch, 0, temp_victim, TO_VICT);

    if (IS_IMMUNE(temp_victim, IMMUNE_KICK)) {
      damage(ch, temp_victim, 0, SKILL_SPIN_KICK, DAM_NO_BLOCK);
    }
    else {
      damage(ch, temp_victim, calc_position_damage(GET_POS(temp_victim), (GET_LEVEL(ch) * 2)), SKILL_SPIN_KICK, DAM_PHYSICAL);
    }
  }

  auto_learn_skill(ch, SKILL_SPIN_KICK);

  int set_wait = 4;

  /* Juggernaut */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 4)) {
    set_wait = 2;
  }

  skill_wait(ch, SKILL_SPIN_KICK, set_wait);
}


void do_backstab(CHAR *ch, char *argument, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_AVATAR)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(argument, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim) {
    send_to_char("Backstab who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("How can you sneak up on yourself!?\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon to backstab someone.\n\r", ch);

    return;
  }

  if ((get_weapon_type(GET_WEAPON(ch)) != TYPE_PIERCE) &&
      (get_weapon_type(GET_WEAPON(ch)) != TYPE_STING)) {
    send_to_char("Only pointed weapons can be used for backstabbing.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(victim)) {
    if (!GET_LEARNED(ch, SKILL_ASSASSINATE)) {
      send_to_char("You can't backstab someone engaged in combat!  They're too alert!\n\r", ch);

      return;
    }

    /* Assassinate */
    if (CAN_SEE(victim, ch) &&
        !IS_AFFECTED(ch, AFF_SNEAK) &&
        !IS_AFFECTED(ch, AFF_IMINV) &&
        !affected_by_spell(ch, SPELL_BLACKMANTLE)) {
      act("Maybe if $E couldn't see you, or in the cover of darkness...", FALSE, ch, 0, victim, TO_CHAR);

      return;
    }

    if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_ASSASSINATE)) {
      damage(ch, victim, 0, SKILL_BACKSTAB, DAM_NO_BLOCK);

      return;
    }
  }

  int check = number(1, 151) - GET_DEX_APP(ch);

  /* Bonus for being unseen. */
  if (!CAN_SEE(victim, ch) ||
      IS_AFFECTED(ch, AFF_SNEAK) ||
      IS_AFFECTED(ch, AFF_IMINV)) {
    check -= 5;
  }

  /* Vehemence */
  if (IS_SET(GET_TOGGLES(ch), TOG_VEHEMENCE)) {
    check -= (5 + (GET_DEX_APP(ch) / 2));
  }

  /* Dark Pact */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
    check -= (5 + (GET_DEX_APP(ch) / 2));
  }

  if ((AWAKE(victim) && (check > GET_LEARNED(ch, SKILL_BACKSTAB))) || IS_IMMUNE(victim, IMMUNE_BACKSTAB)) {
    damage(ch, victim, 0, SKILL_BACKSTAB, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_BACKSTAB, 2);

    return;
  }

  auto_learn_skill(ch, SKILL_BACKSTAB);

  hit(ch, victim, SKILL_BACKSTAB);

  /* Bathed in Blood */
  if (SAME_ROOM(ch, victim)) {
    int room_blood_level = MIN(((CHAR_REAL_ROOM(ch) != NOWHERE) ? ROOM_BLOOD(CHAR_REAL_ROOM(ch)) : 0), 10);

    if (IS_MORTAL(ch) && check_subclass(ch, SC_DEFILER, 5) && chance(10 + room_blood_level)) {
      act("As you drive your weapon into $N's back, $S life energy flows into you.", FALSE, ch, 0, victim, TO_CHAR);
      act("As $n drives $s weapon into your back, your life energy flows into $m.", FALSE, ch, 0, victim, TO_VICT);
      act("As $n drives $s weapon into $N's back, $N's life energy flows into $n.", FALSE, ch, 0, victim, TO_NOTVICT);

      spell_vampiric_touch(GET_LEVEL(ch), ch, victim, 0);
    }
  }

  /* Close Combat */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_BANDIT, 4) && chance(50 + GET_DEX_APP(ch))) {
    enchantment_apply(ch, FALSE, "+3 Hitroll (Close Combat)", SKILL_CLOSE_COMBAT, 3, ENCH_INTERVAL_ROUND, 3, APPLY_HITROLL, 0, 0, 0);
    enchantment_apply(ch, FALSE, "+3 Damroll (Close Combat)", SKILL_CLOSE_COMBAT, 3, ENCH_INTERVAL_ROUND, 3, APPLY_DAMROLL, 0, 0, 0);
  }

  skill_wait(ch, SKILL_BACKSTAB, 3);
}


void do_assassinate(CHAR *ch, char *argument, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) == CLASS_ANTI_PALADIN) &&
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
      !IS_AFFECTED(ch, AFF_INVISIBLE) &&
      !IS_AFFECTED(ch, AFF_IMINV)) {
    send_to_char("You need to be sneaking or invisible to succeed.\n\r", ch);

    return;
  }

  char name[MIL], direction[MIL];

  argument_interpreter(argument, name, direction);

  if (!*name) {
    send_to_char("Assassinate who?\n\r", ch);

    return;
  }

  if (!*direction) {
    send_to_char("What direction?\n\r", ch);

    return;
  }

  int dir = NOWHERE;

  if (is_abbrev(direction, "north")) dir = CMD_NORTH;
  else if (is_abbrev(direction, "east")) dir = CMD_EAST;
  else if (is_abbrev(direction, "south")) dir = CMD_SOUTH;
  else if (is_abbrev(direction, "west")) dir = CMD_WEST;
  else if (is_abbrev(direction, "up")) dir = CMD_UP;
  else if (is_abbrev(direction, "down")) dir = CMD_DOWN;

  if (!dir) {
    send_to_char("What direction!?\n\r", ch);

    return;
  }

  int check = number(1, 111) - GET_DEX_APP(ch);

  /* Dark Pact */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
    check -= 5;
  }

  if (check > GET_LEARNED(ch, SKILL_ASSASSINATE)) {
    send_to_char("You fail your assassination attempt.\n\r", ch);

    skill_wait(ch, SKILL_ASSASSINATE, 2);

    return;
  }

  int orig_room = CHAR_REAL_ROOM(ch);

  do_move(ch, "", dir);

  if (CHAR_REAL_ROOM(ch) != orig_room) {
    do_backstab(ch, name, CMD_BACKSTAB);
  }
}


void do_ambush(struct char_data *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_NOMAD)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim) {
    send_to_char("Ambush who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("How can you sneak up on yourself!?\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon for your ambush to succeed.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(victim)) {
    send_to_char("You can't ambush a fighting person, they're too alert!\n\r", ch);

    return;
  }

  int check = number(1, 151) - GET_DEX_APP(ch);

  /* Sector type bonus. */
  switch (world[CHAR_REAL_ROOM(ch)].sector_type) {
    case SECT_FIELD:
      check -= 2;
      break;
    case SECT_FOREST:
      check -= 7;
      break;
    case SECT_HILLS:
      check -= 3;
      break;
    case SECT_MOUNTAIN:
      check -= 5;
      break;
  }

  if ((AWAKE(victim) && (check > GET_LEARNED(ch, SKILL_AMBUSH))) || IS_IMMUNE(victim, IMMUNE_AMBUSH)) {
    act("You try to ambush $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to ambush you, but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to ambush $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_AMBUSH, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_AMBUSH, 2);

    return;
  }

  auto_learn_skill(ch, SKILL_AMBUSH);

  hit(ch, victim, SKILL_AMBUSH);

  skill_wait(ch, SKILL_AMBUSH, 3);
}


void do_assault(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_COMMANDO)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim) {
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

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon for your assault to succeed.\n\r", ch);

    return;
  }

  int check = number(1, 151) - GET_DEX_APP(ch);

  /* Bonus for assaulting a target already in combat. */
  if (GET_OPPONENT(victim)) {
    check -= 5;
  }

  /* Blur */
  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= (GET_LEVEL(ch) / 7);
  }

  if ((AWAKE(victim) && (check > GET_LEARNED(ch, SKILL_ASSAULT))) || IS_IMMUNE(victim, IMMUNE_ASSAULT)) {
    act("You try to assault $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to assault you, but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to assault $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_ASSAULT, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_ASSAULT, 3);

    return;
  }

  auto_learn_skill(ch, SKILL_ASSAULT);

  hit(ch, victim, SKILL_ASSAULT);

  skill_wait(ch, SKILL_ASSAULT, 3);
}


void do_circle(CHAR *ch, char *argument, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  char name[MIL];

  one_argument(argument, name);

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_AVATAR)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim && IS_ALIVE(GET_OPPONENT(ch)) && SAME_ROOM(ch, GET_OPPONENT(ch))) {
    victim = GET_OPPONENT(ch);
  }

  if (!victim) {
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
    send_to_char("Only pointed weapons can be used to stab someone in the back.\n\r", ch);

    return;
  }

  int check = number(1, 191) - GET_DEX_APP(ch);

  /* Position bonus. */
  if (GET_POS(victim) < POSITION_FIGHTING) {
    check -= 50;
  }

  /* Class bonus. */
  if (GET_CLASS(ch) == CLASS_THIEF) {
    check -= GET_LEVEL(ch) / 2;
  }

  /* Vehemence */
  if (IS_SET(GET_TOGGLES(ch), TOG_VEHEMENCE) && check_sc_access(ch, SKILL_VEHEMENCE)) {
    check -= 5 + (GET_DEX_APP(ch) / 2);
  }

  /* Fade */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_BANDIT, 2) && !count_attackers(ch)) {
    check -= 10;
  }

  if ((check > GET_LEARNED(ch, SKILL_CIRCLE)) || IS_IMMUNE2(victim, IMMUNE2_CIRCLE)) {
    act("$n slips quietly into the shadows, but $N notices as $e appears behind $M.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n disappears from sight, but you notice as $e appears behind you.", FALSE, ch, 0, victim, TO_VICT);
    act("You slip quietly into the shadows, but $N notices as you appear behind $M.", FALSE, ch, 0, victim, TO_CHAR);

    damage(ch, victim, 0, SKILL_CIRCLE, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_CIRCLE, 2);

    return;
  }

  auto_learn_skill(ch, SKILL_CIRCLE);

  act("$n vanishes into the shadows, suddenly appearing behind $N.", FALSE, ch, 0, victim, TO_NOTVICT);
  act("$n disappears into the shadows, vanishing completely from sight.", FALSE, ch, 0, victim, TO_VICT);
  act("You slip into the shadows and vanish, suddenly appearing behind $N.", FALSE, ch, 0, victim, TO_CHAR);

  hit(ch, victim, SKILL_CIRCLE);

  /* Trip */
  if (SAME_ROOM(ch, victim) && check_sc_access(ch, SKILL_TRIP) && IS_SET(GET_TOGGLES(ch), TOG_TRIP)) {
    check = number(1, 111) - GET_DEX_APP(ch);

    /* Vehemence */
    if (IS_SET(GET_TOGGLES(ch), TOG_VEHEMENCE) && check_sc_access(ch, SKILL_VEHEMENCE)) {
      check -= 5 + (GET_DEX_APP(ch) / 2);
    }

    if (check <= GET_LEARNED(ch, SKILL_TRIP)) {
      act("You trip $N, causing $M to become off-balanced.", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n trips you, causing you to become off-balanced.", FALSE, ch, NULL, victim, TO_VICT);
      act("$n trips $N, causing $M to become off-balanced.", FALSE, ch, NULL, victim, TO_NOTVICT);

      if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = stack_position(victim, POSITION_STUNNED);
      }
    }
  }

  /* Close Combat */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_BANDIT, 4) && chance(50 + GET_DEX_APP(ch))) {
    enchantment_apply(ch, FALSE, "+3 Hitroll (Close Combat)", SKILL_CLOSE_COMBAT, 3, ENCH_INTERVAL_ROUND, 3, APPLY_HITROLL, 0, 0, 0);
    enchantment_apply(ch, FALSE, "+3 Damroll (Close Combat)", SKILL_CLOSE_COMBAT, 3, ENCH_INTERVAL_ROUND, 3, APPLY_DAMROLL, 0, 0, 0);
  }

  skill_wait(ch, SKILL_CIRCLE, 3);
}


void do_order(struct char_data *ch, char *argument, int cmd) {
  char name[100], message[256];
  char buf[MSL];
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

  if (IS_AFFECTED(ch, AFF_PARALYSIS) || (IS_AFFECTED(ch, AFF_HOLD) && chance(25))) {
    act("You try to flee, but are paralyzed and can't move!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n tries to flee, but seems to be unable to move.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  if (enchanted_by(ch, "Staggering (Tremor)") && chance(95)) {
    act("You try to flee, but your tremoring causes you to stumble and stagger!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n tries to flee, but $s tremoring causes $m to stumble and stagger.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  if (affected_by_spell(ch, SKILL_BERSERK) || affected_by_spell(ch, SKILL_FRENZY)) {
    act("You try to flee, but fail!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n tries to flee, but fails.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  if ((wall = get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch)))) {
    act("A wall of thorns blocks your way.  Ouch!", FALSE, ch, 0, 0, TO_CHAR);

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

        if ((die = do_simple_move(ch, attempt, 1)) == 1) {
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
        !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, DEATH) &&
        (!IS_NPC(ch) || (IS_NPC(ch) && !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, NO_MOB)))) {
      CHAR *blocker = GET_OPPONENT(ch);

      if (!GET_RIDER(ch) && !IS_NPC(blocker) && IS_SET(GET_TOGGLES(blocker), TOG_BLOCK)) {
        int block_check = number(1, 111);
        int block_skill = GET_LEARNED(blocker, SKILL_BLOCK) + GET_DEX_APP(blocker);
        int auto_block_chance = 0;

        /* Iron Fist */
        if (IS_MORTAL(blocker) && check_subclass(blocker, SC_WARLORD, 3)) {
          block_skill += (GET_LEVEL(blocker) / 5);
          auto_block_chance = 90;
        }

        if ((block_check <= block_skill) || (auto_block_chance && chance(auto_block_chance))) {
          act("You tried to flee, but $N blocked your way!", FALSE, ch, 0, blocker, TO_CHAR);
          act("$n tried to flee, but you blocked $s way!", FALSE, ch, 0, blocker, TO_VICT);
          act("$n tried to flee, but $N blocked $s way!", FALSE, ch, 0, blocker, TO_NOTVICT);

          return;
        }
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

      if ((die = do_simple_move(ch, attempt, 1)) == 1) {
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


#define QGII_GRUUMSH   1060 // The Spiked Gauntlet of Gruumsh
#define JEAROM_GRUUMSH 29374

void do_pummel(CHAR *ch, char *arg, int cmd) {

  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_COMMANDO)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim && IS_ALIVE(GET_OPPONENT(ch)) && SAME_ROOM(ch, GET_OPPONENT(ch))) {
    victim = GET_OPPONENT(ch);
  }

  if (!victim) {
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

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon for your pummel to succeed.\n\r", ch);

    return;
  }

  int check = number(1, 121) - GET_DEX_APP(ch);

  /* Blur */
  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 7;
  }

  if (check > GET_LEARNED(ch, SKILL_PUMMEL)) {
    act("You try to pummel $N, but miss.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n tried to pummel you, but missed.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n tried to pummel $N, but missed.", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_PUMMEL, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_PUMMEL, 2);

    return;
  }

  if (IS_IMMUNE(victim, IMMUNE_PUMMEL) || !breakthrough(ch, victim, SKILL_PUMMEL, BT_INVUL)) {
    act("You pummel $N, but your pummel has no effect!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n pummels you, but $s pummel has no effect!", FALSE, ch, 0, victim, TO_VICT);
    act("$n pummels $N, but $s pummel has no effect!", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_PUMMEL, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_PUMMEL, 2);

    return;
  }

  auto_learn_skill(ch, SKILL_PUMMEL);

  int set_pos = stack_position(victim, POSITION_STUNNED);

  /* The Spiked Gauntlet of Gruumsh + vanities */
  if (EQ(ch, WEAR_HANDS) && chance(15)) {
    switch (V_OBJ(EQ(ch, WEAR_HANDS))) {
      case QGII_GRUUMSH:
      case JEAROM_GRUUMSH:
        set_pos = POSITION_MORTALLYW;
        break;
    }
  }

  act("You pummel $N, and $N is stunned now!", FALSE, ch, 0, victim, TO_CHAR);
  act("$n pummels you, and you are stunned now!", FALSE, ch, 0, victim, TO_VICT);
  act("$n pummels $N, and $N is stunned now!", FALSE, ch, 0, victim, TO_NOTVICT);

  damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_PUMMEL, DAM_PHYSICAL);

  /* Hidden Blade */
  if (SAME_ROOM(ch, victim) &&
      ((GET_CLASS(ch) == CLASS_ANTI_PALADIN) && (GET_LEVEL(ch) >= 40)) &&
      ((number(1, SKILL_MAX_PRAC) <= GET_LEARNED(ch, SKILL_HIDDEN_BLADE)) && chance(25 + GET_DEX_APP(ch)))) {
    act("You drive a hidden blade deep into $N's gut!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n drives a hidden blade deep into your gut!", FALSE, ch, 0, victim, TO_VICT);
    act("$n drives a hidden blade deep into $N's gut!", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, calc_position_damage(GET_POS(victim), GET_LEVEL(ch) * 2), SKILL_HIDDEN_BLADE, DAM_PHYSICAL);
  }

  if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;

    WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
  }

  /* Trusty Steed */
  if (SAME_ROOM(ch, victim) && check_sc_access(ch, SKILL_TRUSTY_STEED) && IS_SET(GET_TOGGLES(ch), TOG_TRUSTY_STEED)) {
    check = number(1, 121) - GET_WIS_APP(ch);

    if ((check <= GET_LEARNED(ch, SKILL_TRUSTY_STEED)) && breakthrough(ch, victim, SKILL_TRUSTY_STEED, BT_INVUL)) {
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
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_CLERIC) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_COMMANDO) &&
      (GET_CLASS(ch) != CLASS_WARRIOR)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) == CLASS_CLERIC) &&
      (GET_LEVEL(ch) < 35)) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim && IS_ALIVE(GET_OPPONENT(ch)) && SAME_ROOM(ch, GET_OPPONENT(ch))) {
    victim = GET_OPPONENT(ch);
  }

  if (!victim) {
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

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon for your bash to succeed.\n\r", ch);

    return;
  }

  int check = number(1, 101) - MAX(GET_STR_TO_HIT(ch), GET_DEX_APP(ch));

  if ((check > GET_LEARNED(ch, SKILL_BASH)) || IS_IMMUNE2(victim, IMMUNE2_BASH) || !breakthrough(ch, victim, SKILL_BASH, BT_INVUL)) {
    damage(ch, victim, 0, SKILL_BASH, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_BASH, 2);

    return;
  }

  auto_learn_skill(ch, SKILL_BASH);

  int set_pos = stack_position(victim, POSITION_RESTING);

  damage(ch, victim, calc_position_damage(GET_POS(victim), number(1, GET_LEVEL(ch))), SKILL_BASH, DAM_PHYSICAL);

  if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;

    WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
  }

  skill_wait(ch, SKILL_BASH, 2);
}


void do_punch(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_AVATAR)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim && IS_ALIVE(GET_OPPONENT(ch)) && SAME_ROOM(ch, GET_OPPONENT(ch))) {
    victim = GET_OPPONENT(ch);
  }

  if (!victim) {
    send_to_char("Punch who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You aren't feeling well, are you?\n\r", ch);

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

  int check = number(1, 101) - MAX(GET_STR_TO_HIT(ch), GET_DEX_APP(ch));

  if (check > GET_LEARNED(ch, SKILL_PUNCH) || IS_IMMUNE(victim, IMMUNE_PUNCH) || !breakthrough(ch, victim, SKILL_PUNCH, BT_INVUL)) {
    damage(ch, victim, 0, SKILL_PUNCH, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_PUNCH, 2);

    return;
  }

  auto_learn_skill(ch, SKILL_PUNCH);

  int set_pos = stack_position(victim, POSITION_SITTING);

  /* Iron Fist */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 3) && chance(33)) {
    set_pos = stack_position(victim, POSITION_STUNNED);

    damage(ch, victim, calc_position_damage(GET_POS(victim), GET_LEVEL(ch) * 4), SKILL_PUNCH, DAM_PHYSICAL);

    act("Your iron fist hits $N with devastating effect!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n's iron fist hits you with devastating effect!", FALSE, ch, 0, victim, TO_VICT);
    act("$n's iron fist hits $N with devastating effect!", FALSE, ch, 0, victim, TO_NOTVICT);
  }
  /* Punch */
  else {
    damage(ch, victim, calc_position_damage(GET_POS(victim), GET_LEVEL(ch) * 2), SKILL_PUNCH, DAM_PHYSICAL);
  }

  if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;

    WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
  }

  skill_wait(ch, SKILL_PUNCH, 2);
}


void do_rescue(CHAR *ch, char *argument, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (CHAOSMODE) {
    send_to_char("Rescue? During Chaos!?  It'd be better to just commit suicide...\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(argument, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim) {
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

  CHAR *attacker = NULL;

  for (attacker = world[CHAR_REAL_ROOM(ch)].people; attacker && (GET_OPPONENT(attacker) != victim); attacker = attacker->next_in_room);

  if (!attacker) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  int check = number(1, 101);

  /* Anyone can rescue a player that is level 15 or below. Protect grants automatic success. */
  if (IS_MORTAL(ch) && (GET_LEVEL(victim) > 15) && !check_subclass(ch, SC_WARLORD, 2)) {
    if (IS_SET(GET_TOGGLES(ch), TOG_HOSTILE)) {
      send_to_char("Your hostility prevents the rescue.\n\r", ch);

      return;
    }

    if (check > GET_LEARNED(ch, SKILL_RESCUE)) {
      send_to_char("You fail the rescue.\n\r", ch);

      return;
    }
  }

  auto_learn_skill(ch, SKILL_RESCUE);

  act("Banzai!  To the rescue...", FALSE, ch, 0, victim, TO_CHAR);
  act("You are rescued by $n!  You are confused!", FALSE, ch, 0, victim, TO_VICT);
  act("$n heroically rescues $N!", FALSE, ch, 0, victim, TO_NOTVICT);

  if (GET_OPPONENT(victim) == attacker) {
    stop_fighting(victim);
  }

  if (GET_OPPONENT(attacker) == victim) {
    stop_fighting(attacker);
  }

  if (GET_OPPONENT(ch)) {
    stop_fighting(ch);
  }

  set_fighting(ch, attacker);
  set_fighting(attacker, ch);

  if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
    WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
  }
}


void do_assist(CHAR *ch, char *argument, int cmd) {
  if (CHAOSMODE) {
    send_to_char("Assist?  All you can think about is KILLING!\n\r", ch);

    return;
  }

  if ((GET_POS(ch) == POSITION_FIGHTING) || GET_OPPONENT(ch)) {
    send_to_char("You are fighting already!\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(argument, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim) {
    send_to_char("Who do you want to assist?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You're doing the best you can...\n\r", ch);

    return;
  }

  CHAR *attacker = NULL;

  for (attacker = world[CHAR_REAL_ROOM(ch)].people; attacker && (GET_OPPONENT(attacker) != victim); attacker = attacker->next_in_room);

  if (!attacker) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  send_to_char("You join the fight!\n\r", ch);
  act("$n assists you!", FALSE, ch, 0, victim, TO_VICT);
  act("$n assists $N.", FALSE, ch, 0, victim, TO_NOTVICT);

  set_fighting(ch, attacker);
}


void do_kick(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_COMMANDO)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim && IS_ALIVE(GET_OPPONENT(ch)) && SAME_ROOM(ch, victim)) {
    victim = GET_OPPONENT(ch);
  }

  if (!victim) {
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

  int check = number(1, 101) - GET_DEX_APP(ch);

  /* Juggernaut */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 4)) {
    check -= 5;
  }

  /* Blur */
  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 7;
  }

  if ((check > GET_LEARNED(ch, SKILL_KICK)) || IS_IMMUNE(victim, IMMUNE_KICK) || !breakthrough(ch, victim, SKILL_KICK, BT_INVUL)) {
    damage(ch, victim, 0, SKILL_KICK, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_KICK, 2);

    return;
  }

  auto_learn_skill(ch, SKILL_KICK);

  damage(ch, victim, calc_position_damage(GET_POS(victim), (GET_LEVEL(ch) * 2)), SKILL_KICK, DAM_PHYSICAL);

  if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
    WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(2, 3) : 3));
  }

  skill_wait(ch, SKILL_KICK, 2);
}


void do_disarm(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  char name[256];
  int percent;
  struct obj_data *wield = 0;

  if ((GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_CLASS(ch) != CLASS_COMMANDO) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_AVATAR)) {
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
    (IS_MORTAL(ch) && check_subclass(victim, SC_GLADIATOR, 2)) || /* Tactician */
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


void do_disembowel(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_NOMAD)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      ((GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) &&
      (GET_LEVEL(ch) < 40)) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim && !(victim = GET_OPPONENT(ch))) {
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
    send_to_char("You can't disembowel another player!\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon for your disembowel to succeed.\n\r", ch);

    return;
  }

  if ((get_weapon_type(GET_WEAPON(ch)) == TYPE_HIT) ||
      (get_weapon_type(GET_WEAPON(ch)) == TYPE_BLUDGEON) ||
      (get_weapon_type(GET_WEAPON(ch)) == TYPE_CRUSH)) {
    send_to_char("You need a weapon with an edge or a point to disembowel someone.\n\r", ch);

    return;
  }

  int check = number(1, 131) - MAX(GET_STR_TO_HIT(ch), GET_DEX_APP(ch));

  if (IS_IMMUNE(victim, IMMUNE_DISEMBOWEL) || (check > GET_LEARNED(ch, SKILL_DISEMBOWEL))) {
    act("$N completely avoids your attempt to spill $S guts.", FALSE, ch, 0, victim, TO_CHAR);
    act("You completely avoid $N's attempt to spill your guts.", FALSE, ch, 0, victim, TO_VICT);
    act("$N completely avoids $n's attempt to spill $S guts.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_DISEMBOWEL, DAM_PHYSICAL);

    skill_wait(ch, SKILL_DISEMBOWEL, 3);

    return;
  }

  int dam = number(GET_LEVEL(ch) / 10, GET_LEVEL(ch) / 5) * calc_hit_damage(ch, victim, GET_WEAPON(ch), 0, RND_RND);

  if ((GET_HIT(victim) > dam) && (GET_HIT(victim) > (GET_MAX_HIT(victim) * 0.3))) {
    act("You attempt to spill $N's guts, but $E fends you off.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n attempts to spill your guts, but you fend $m off.", FALSE, ch, 0, victim, TO_VICT);
    act("$n attempts to spill $N's guts, but $N fends $n off.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_DISEMBOWEL, DAM_PHYSICAL);

    skill_wait(ch, SKILL_DISEMBOWEL, 3);

    return;
  }

  auto_learn_skill(ch, SKILL_DISEMBOWEL);

  act("Your savage attack causes $N's guts to spill out!", FALSE, ch, 0, victim, TO_CHAR);
  act("$n's savage attack causes your guts to spill out!", FALSE, ch, 0, victim, TO_VICT);
  act("$n's savage attack causes $N's guts to spill out!", FALSE, ch, 0, victim, TO_NOTVICT);

  damage(ch, victim, dam, SKILL_DISEMBOWEL, DAM_PHYSICAL);

  skill_wait(ch, SKILL_DISEMBOWEL, 3);
}


/*
void do_backflip(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_BARD)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) == CLASS_BARD) &&
      (GET_LEVEL(ch) < 20)) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim && IS_ALIVE(GET_OPPONENT(ch)) && SAME_ROOM(ch, GET_OPPONENT(ch))) {
    victim = GET_OPPONENT(ch);
  }

  if (!victim) {
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

  int check = number(1, 111) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_BACKFLIP)) {
    act("As you spring, $N knocks you down!", FALSE, ch, 0, victim, TO_CHAR);
    act("As $n springs, you knock $m down!", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to spring over $N, but is knocked hard on $s back.", FALSE, ch, 0, victim, TO_NOTVICT);

    skill_wait(ch, SKILL_BACKFLIP, 2);

    return;
  }

  auto_learn_skill(ch, SKILL_BACKFLIP);

  act("With a mighty leap, you spring over $N!", 0, ch, 0, victim, TO_CHAR);
  act("$n crouches low and springs over you!", 0, ch, 0, victim, TO_VICT);
  act("With a mighty leap, $n springs over $N and lands behind $M!", 0, ch, 0, victim, TO_NOTVICT);

  hit(ch, victim, SKILL_BACKFLIP);

  skill_wait(ch, SKILL_BACKFLIP, 2);
}
*/

void do_cunning(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) == CLASS_THIEF) &&
      (GET_LEVEL(ch) < 50)) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_CUNNING)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_CUNNING);

    send_to_char("You relinquish your focus and feel notably less cunning.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_CUNNING)) {
    send_to_char("You aren't feeling particularly cunning at the moment.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_CUNNING);

  send_to_char("You focus on exploiting any weakness in your enemies' defenses and grow more cunning in the process.\n\r", ch);
}


void do_coin_toss(CHAR *ch, char *argument, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  char amount[MIL];

  one_argument(argument, amount);

  int wager = atoi(amount);

  int min_wager = (GET_LEVEL(ch) * 100);
  int max_wager = (GET_LEVEL(ch) * 1000);

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

  int num_mobs = count_mobs_real_room(CHAR_REAL_ROOM(ch));
  int scattered_coins = ((wager * number(10, 25)) / 100);

  int check = number(1, 101) - GET_DEX_APP(ch);

  /* Vehemence */
  if (IS_SET(GET_TOGGLES(ch), TOG_VEHEMENCE)) {
    check -= (5 + (GET_DEX_APP(ch) / 2));
  }

  if (check > GET_LEARNED(ch, SKILL_COIN_TOSS)) {
    send_to_char("You toss some coins in the air, somehow missing all of your targets.\n\r", ch);
    act("$n tosses some coins in the air, scattering them about carelessly.", FALSE, ch, 0, 0, TO_ROOM);

    if (num_mobs > 0) {
      scattered_coins /= num_mobs;

      for (CHAR *temp_victim = world[CHAR_REAL_ROOM(ch)].people, *next_victim; temp_victim; temp_victim = next_victim) {
        next_victim = temp_victim->next_in_room;

        if (IS_MOB(temp_victim) && (scattered_coins > 0)) {
          act("You snatch some of the coins thrown by $n!", FALSE, ch, 0, temp_victim, TO_VICT);

          GET_GOLD(temp_victim) += scattered_coins;
        }
      }
    }
    else {
      send_to_char("Nobody was hit by your coins and they are hopelessly lost in the chaos.\n\r", ch);
    }

    skill_wait(ch, SKILL_COIN_TOSS, 2);

    return;
  }

  send_to_char("You toss a hail of coins about you with deadly precision.\n\r", ch);
  act("$n tosses a hail of coins about $m with deadly precision.", FALSE, ch, 0, 0, TO_ROOM);

  if (num_mobs > 0) {
    for (CHAR *temp_victim = world[CHAR_REAL_ROOM(ch)].people, *next_victim; temp_victim; temp_victim = next_victim) {
      next_victim = temp_victim->next_in_room;

      if ((temp_victim == ch) ||
          (IS_IMMORTAL(temp_victim)) ||
          (IS_NPC(temp_victim) && (GET_RIDER(temp_victim) == ch)) ||
          (IS_MORTAL(temp_victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) ||
          (IS_MORTAL(temp_victim) && GET_OPPONENT(temp_victim) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))) {
        continue;
      }

      act("You are battered by a hail of coins thrown by $n!", FALSE, ch, 0, temp_victim, TO_VICT);

      double factor = 0.0;

      if ((wager >= min_wager) && (wager <= max_wager)) {
        factor = (wager / max_wager);
      }

      int dmg = MAX(lround(((GET_LEVEL(ch) * 2) * factor)), 10);

      damage(ch, temp_victim, calc_position_damage(GET_POS(temp_victim), dmg), SKILL_COIN_TOSS, DAM_PHYSICAL);
    }
  }
  else {
    send_to_char("Nobody was hit by your coins and they are hopelessly lost in the chaos.\n\r", ch);
  }

  skill_wait(ch, SKILL_COIN_TOSS, 2);
}
