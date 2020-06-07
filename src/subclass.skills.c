/*
** subclass.skills.c
**   Routines for subclass skills.
**   Skill# defines are in spells.h
**   List of skills/class in constants.c
**   skill name in spell.parser.c
**   skill wear off in constants.c
**   subclass check in subclass.c
**
** Written for RoninMUD by Ranger
**
** Do not distribute without permission.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
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
#include "cmd.h"
#include "utility.h"
#include "act.h"
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"
#include "subclass.h"
#include "enchant.h"

int calc_position_damage(int position, int dam);
int stack_position(CHAR *ch, int target_position);
void do_move(struct char_data *ch, char *argument, int cmd);
void skill_wait(CHAR *ch, int skill, int wait);


void do_awareness(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_AWARENESS)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_AWARENESS)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_AWARENESS);

    send_to_char("You relax and your sense of awareness fades.\n\r", ch);
    act("$n relaxes and $s sense of awareness fades.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_AWARENESS)) {
    send_to_char("You are overwhelmed by your surroundings, nearly feeling less aware than before.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_AWARENESS);

  send_to_char("You become more aware of your surroundings, watching for danger.\n\r", ch);
  act("$n becomes more aware of $s surroundings, watching for danger.", TRUE, ch, 0, 0, TO_ROOM);
}


void do_meditate(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_MEDITATE)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch)) {
    send_to_char("You can't meditate while fighting!\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_MEDITATE)) {
    send_to_char("You're unable to meditate again for a while.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_MEDITATE)) {
    send_to_char("You failed to focus your thoughts.\n\r", ch);

    return;
  }

  affect_apply(ch, SKILL_MEDITATE, (CHAOSMODE ? 12 : 32), 0, 0, 0, 0);

  send_to_char("You gaze inward and focus on healing.\n\r", ch);
  act("$n enters a deep trance.", TRUE, ch, 0, 0, TO_ROOM);
}


void do_protect(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_PROTECT)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (CHAOSMODE) {
    send_to_char("Protect someone during Chaos? Might as well just quit now...\n\r", ch);

    return;
  }

  char buf[MIL];

  one_argument(arg, buf);

  if (!*buf) {
    send_to_char("Protect who?\n\r", ch);

    return;
  }

  CHAR *victim = victim = get_char_room_vis(ch, buf);

  if (!victim) {
    send_to_char("That player is not here.\n\r", ch);

    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("Protect a potential enemy? Impossible!\n\r", ch);

    return;
  }

  if (IS_IMMORTAL(victim)) {
    send_to_char("The gods laugh at the mere thought.\n\r", ch);

    return;
  }

  if (ch == victim) {
    send_to_char("You now protect only yourself.\n\r", ch);
    act("$n now protects only $mself.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_NOTVICT);

    if (GET_PROTECTEE(ch)) {
      act("$n stops protecting you.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_VICT);

      GET_PROTECTOR(GET_PROTECTEE(ch)) = NULL;
    }

    GET_PROTECTEE(ch) = NULL;

    return;
  }

  /* Berserk */
  if (affected_by_spell(ch, SKILL_BERSERK)) {
    send_to_char("You can't protect anyone while you're berserking!\n\r", ch);

    return;
  }

  if (GET_PROTECTEE(victim)) {
    act("$N is already protecting someone.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if (GET_PROTECTEE(ch)) {
    act("You stop protecting $N.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_CHAR);
    act("$n stops protecting you.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_VICT);
    act("$n stops protecting $N.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_NOTVICT);

    GET_PROTECTOR(GET_PROTECTEE(ch)) = NULL;
    GET_PROTECTEE(ch) = NULL;
  }

  if ((number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_PROTECT))) {
    act("You failed to start protecting $N.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  act("You start trying to protect $N.", FALSE, ch, 0, victim, TO_CHAR);
  act("$n starts trying to protect you.", FALSE, ch, 0, victim, TO_VICT);
  act("$n starts trying to protect $N.", FALSE, ch, 0, victim, TO_NOTVICT);

  GET_PROTECTEE(ch) = victim;
  GET_PROTECTOR(victim) = ch;
}


void do_backfist(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_BACKFIST)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char buf[MIL];

  one_argument(arg, buf);

  CHAR *victim = get_char_room_vis(ch, buf);

  if (!victim && IS_ALIVE(GET_OPPONENT(ch)) && SAME_ROOM(ch, GET_OPPONENT(ch))) {
    victim = GET_OPPONENT(ch);
  }

  if (!victim) {
    send_to_char("Backfist who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("That sounds incredibly painful...\n\r", ch);

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

  int check = number(1, 121) - GET_DEX_APP(ch);

  /* Blur*/
  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 7;
  }

  if (check > GET_LEARNED(ch, SKILL_BACKFIST)) {
    act("You try to backfist $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to backfist $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n tries to backfist you, but misses.", FALSE, ch, 0, victim, TO_VICT);

    damage(ch, victim, 0, SKILL_BACKFIST, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_BACKFIST, 2);

    return;
  }

  int set_pos = stack_position(victim, POSITION_STUNNED);

  act("With a sickening crunch you hit $N with a huge backfist.", FALSE, ch, 0, victim, TO_CHAR);
  act("With a sickening crunch $n hits $N with a huge backfist.", FALSE, ch, 0, victim, TO_NOTVICT);
  act("With a sickening crunch $n hits you with a huge backfist.", FALSE, ch, 0, victim, TO_VICT);

  double multi = number(150, 175) / 100;

  damage(ch, victim, calc_position_damage(GET_POS(victim), lround(GET_LEVEL(ch) * multi)), SKILL_BACKFIST, DAM_PHYSICAL);

  if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;

    WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
  }

  skill_wait(ch, SKILL_BACKFIST, 2);
}


void do_pray(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) && (GET_CLASS(ch) != CLASS_PALADIN)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_LEVEL(ch) < 40) {
    send_to_char("You are not high enough level to use that skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch)) {
    send_to_char("You would do better to swear than to pray right now.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_PRAY)) {
    send_to_char("You're already praying.\n\r", ch);

    return;
  }

  if (GET_ALIGNMENT(ch) == 1000) {
    send_to_char("You are already saintly.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_PRAY)) {
    send_to_char("You failed to focus your thoughts in prayer.\n\r", ch);

    return;
  }

  affect_apply(ch, SKILL_PRAY, -1, 0, 0, 0, 0);

  send_to_char("You bow your head and begin your prayer.\n\r", ch);
  act("$n bows $s head and begins praying.", TRUE, ch, 0, 0, TO_ROOM);
}


void do_tigerkick(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_TIGERKICK)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char buf[MIL];

  one_argument(arg, buf);

  CHAR *victim = get_char_room_vis(ch, buf);

  if (!victim && IS_ALIVE(GET_OPPONENT(ch)) && SAME_ROOM(ch, GET_OPPONENT(ch))) {
    victim = GET_OPPONENT(ch);
  }

  if (!victim) {
    send_to_char("Tigerkick who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("That would be physically impossible...\n\r", ch);

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

  int check = number(1, 121) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_TIGERKICK)) {
    act("You try to tigerkick $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to tigerkick you, but misses.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to tigerkick $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_TIGERKICK, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_TIGERKICK, 2);

    return;
  }

  if (!breakthrough(ch, victim, SKILL_TIGERKICK, BT_INVUL)) {
    act("$N seems completely unaffected by your beautiful tigerkick.", FALSE, ch, 0, victim, TO_CHAR);
    act("You feel completely unaffected by $n's beautiful tigerkick.", FALSE, ch, 0, victim, TO_VICT);
    act("$N seems completely unaffected by $n's beautiful tigerkick.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_TIGERKICK, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_TIGERKICK, 2);

    return;
  }

  int set_pos = stack_position(victim, POSITION_STUNNED);

  act("You drive your foot into $N's chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_CHAR);
  act("$n drives $s foot into your chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_VICT);
  act("$n drives $s foot into $N's chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_NOTVICT);

  damage(ch, victim, calc_position_damage(GET_POS(victim), lround(GET_LEVEL(ch) * 1.75)), SKILL_TIGERKICK, DAM_PHYSICAL);

  GET_MANA(ch) = MIN(GET_MAX_MANA(ch), (GET_MANA(ch) + MIN(MAX((GET_MAX_HIT(victim) / 5000), 2), 10)));

  if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;

    WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
  }

  skill_wait(ch, SKILL_TIGERKICK, 2);
}


/* Portions of scan taken from a snippet (do_scan) by sjmaster@mit.edu */
void list_scanned_chars(CHAR *list, CHAR *ch, int distance, int door) {
  CHAR *i = NULL;
  int count = 0;
  bool start = FALSE;

  const char *how_far[] = {
    "close to the",
    "a ways to the",
    "far off to the"
  };

  const char *dir_name[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down"
  };

  for (i = list; i; i = i->next_in_room) {
    if (CAN_SEE(ch, i)) count++;
  }

  if (!count) return;

  for (i = list; i; i = i->next_in_room) {
    if (!CAN_SEE(ch, i)) continue;

    count--;

    if (start) {
      printf_to_char(ch, "You see %s", (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)));
    }
    else {
      printf_to_char(ch, "%s", (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)));
    }

    if (count > 1) {
      printf_to_char(ch, ", ");
    }
    else if (count == 1) {
      printf_to_char(ch, " and ");
    }
    else {
      printf_to_char(ch, " %s %s.\n\r", how_far[distance], dir_name[door]);
    }
  }
}


/* Portions of scan taken from a snippet (do_scan) by sjmaster@mit.edu */
void do_scan(CHAR *ch, char *arg, int cmd) {
  int dir = NOWHERE;
  int room = NOWHERE;
  int exit_room = NOWHERE;
  int distance = 0;

  const char *dir_name[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down"
  };

  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF) &&
      !check_subclass(ch, SC_TRAPPER, 1)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if ((GET_CLASS(ch) == CLASS_THIEF) && GET_LEVEL(ch) < 35) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You can't see a thing! You're blind!\n\r", ch);

    return;
  }

  send_to_char("You quickly scan the area.\n\r", ch);

  for (dir = 0; dir < 6; dir++) {
    room = CHAR_REAL_ROOM(ch);

    if (!CAN_GO(ch, dir)) continue;

    exit_room = world[room].dir_option[dir]->to_room_r;

    if ((number(1, 121) > GET_LEARNED(ch, SKILL_SCAN)) ||
        ((exit_room != NOWHERE) && (exit_room != real_room(0)) && IS_SET(NO_PEEK, ROOM_FLAGS(exit_room)))) {
      printf_to_char(ch, "You failed to scan %s.\n\r", dir_name[dir]);

      continue;
    }

    for (distance = 0; distance < 3; distance++) {
      if (!world[room].dir_option[dir]) break;

      exit_room = world[room].dir_option[dir]->to_room_r;

      if ((exit_room == NOWHERE) ||
          (exit_room == real_room(0)) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_CLOSED) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_CRAWL) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_ENTER) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_JUMP) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_CLIMB)) {
        break;
      }

      if (world[exit_room].people) {
        list_scanned_chars(world[exit_room].people, ch, distance, dir);
      }

      room = exit_room;
    }
  }
}


void do_camp(CHAR *ch, char *arg, int cmd) {
  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_CAMP)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) == CLASS_BARD) &&
      (GET_LEVEL(ch) < 35)) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_CAMP)) {
    send_to_char("You're already camping.\n\r", ch);

    return;
  }

  if (IS_SET(CHAR_ROOM_FLAGS(ch), CLUB)) {
    send_to_char("There is already a camp here.\n\r", ch);

    return;
  }

  if (GET_MOUNT(ch)) {
    send_to_char("You must dismount first.\n\r", ch);

    return;
  }

  if (count_mobs_real_room_except_followers(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't camp when there are potential enemies in the room!\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_CAMP)) {
    send_to_char("You failed to set up camp.\n\r", ch);

    return;
  }

  affect_apply(ch, SKILL_CAMP, -1, 0, 0, 0, 0);

  GET_POS(ch) = POSITION_RESTING;

  SET_BIT(CHAR_ROOM_FLAGS(ch), CLUB);

  send_to_char("You quickly set up a camp here, then sit down and rest.\n\r", ch);
  act("$n quickly sets up a camp here, then sits down and rests.", FALSE, ch, 0, 0, TO_ROOM);
}

void do_lunge(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_LUNGE)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim) {
    send_to_char("Lunge at who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You aren't flexible enough for that maneuver...\n\r", ch);

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
    send_to_char("You need to wield a weapon for your lunge to succeed.\n\r", ch);

    return;
  }

  int check = number(1, 131) - GET_DEX_APP(ch);

  /* Bonus for lunging at a target already in combat. */
  if (GET_OPPONENT(victim)) {
    check -= 5;
  }

  if (AWAKE(victim) && (check > GET_LEARNED(ch, SKILL_LUNGE))) {
    act("You try to lunge at $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to lunge at you, but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to lunge at $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_LUNGE, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_LUNGE, 2);

    return;
  }

  hit(ch, victim, SKILL_LUNGE);

  skill_wait(ch, SKILL_LUNGE, 2);
}


void do_vehemence(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_VEHEMENCE)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch)) {
    send_to_char("You can't shake off your feeling of vehemence while in combat.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_VEHEMENCE)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_VEHEMENCE);

    send_to_char("Your vehemence diminishes.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_VEHEMENCE)) {
    send_to_char("You are unable to fill yourself with vehemence.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_VEHEMENCE);

  send_to_char("You feel a sudden rush of vehemence wash over you.\n\r", ch);
}


void do_smite(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_SMITE)) {
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
    send_to_char("Smite who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("Have you considered the possibility that you might be insane?\n\r", ch);

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
    send_to_char("You need to wield a weapon for your smite to succeed.\n\r", ch);

    return;
  }

  int check = number(1, 121) - MAX(GET_STR_TO_HIT(ch), GET_WIS_APP(ch));

  if (check > GET_LEARNED(ch, SKILL_SMITE)) {
    act("You try to smite $N, but your concentration falters.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to smite you, but $s concentration falters.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to smite $N, but $s concentration falters.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_SMITE, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_SMITE, 2);

    return;
  }

  if (GET_OPPONENT(ch) && (victim != GET_OPPONENT(ch))) {
    act("You call upon your divine energy and switch the focus of your attacks to $N.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n calls upon $s divine energy and switches the focus of $s attacks to you!", FALSE, ch, 0, victim, TO_VICT);
    act("$n calls upon $s divine energy and switches the focus of $s attacks to $N.", FALSE, ch, 0, victim, TO_NOTVICT);
  }
  else {
    act("You raise your weapon high to smite $N with holy vengeance!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n raises $s weapon high to smite you with holy vengence!", FALSE, ch, 0, victim, TO_VICT);
    act("$n raises $s weapon high to smite $N with holy vengeance!", FALSE, ch, 0, victim, TO_NOTVICT);
  }

  stop_fighting(ch);
  set_fighting(ch, victim);

  hit(ch, victim, SKILL_SMITE);

  GET_POS(victim) = stack_position(victim, POSITION_STUNNED);

  /* Trusty Steed */
  if (SAME_ROOM(ch, victim) && IS_SET(GET_TOGGLES(ch), TOG_TRUSTY_STEED) && check_sc_access(ch, SKILL_TRUSTY_STEED)) {
    check = number(1, 121) - GET_WIS_APP(ch);

    if ((check <= GET_LEARNED(ch, SKILL_TRUSTY_STEED)) && breakthrough(ch, victim, SKILL_TRUSTY_STEED, BT_INVUL)) {
      int set_pos = stack_position(victim, POSITION_SITTING);

      act("You summon forth your trusty steed and it tramples $N with spiritual energy!", FALSE, ch, 0, victim, TO_CHAR);
      act("$n summons forth $s trusty steed and it tramples you with spiritual energy!", FALSE, ch, 0, victim, TO_VICT);
      act("$n summons forth $s trusty steed and it tramples $N with spiritual energy!", FALSE, ch, 0, victim, TO_NOTVICT);

      damage(ch, victim, calc_position_damage(GET_POS(victim), lround(GET_LEVEL(ch) * 1.5)), SKILL_TRUSTY_STEED, DAM_PHYSICAL);

      if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = set_pos;
      }
    }
  }

  skill_wait(ch, SKILL_SMITE, 2);
}


int maim_enchantment(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (cmd == MSG_DAMAGED) {
    if (!*arg || strcasecmp(arg, "SKILL_MAIM")) return FALSE;

    send_to_char("You cry out in pain as your maimed body suffers another blow!\n\r", ch);
    act("$n cries out in pain as $s maimed body suffers another blow!", FALSE, ch, 0, 0, TO_ROOM);

    return FALSE;
  }

  return FALSE;
}

void do_flank(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_FLANK)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim) {
    send_to_char("Flank who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("Yeah... right... flank yourself...\n\r", ch);

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
    send_to_char("You need to wield a weapon for your flank to succeed.\n\r", ch);

    return;
  }

  int check = number(1, 131) - GET_DEX_APP(ch);

  if (AWAKE(victim) && (check > GET_LEARNED(ch, SKILL_FLANK))) {
    act("You try to flank $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to flank you, but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to flank $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_FLANK, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_FLANK, 2);

    return;
  }

  if (!GET_OPPONENT(ch)) {
    hit(ch, victim, SKILL_FLANK);
  }
  else if (GET_OPPONENT(ch) && (victim != GET_OPPONENT(ch))) {
    act("You turn suddenly, flanking $N deftly and swiftly.", 0, ch, 0, victim, TO_CHAR);
    act("$n turns suddenly, flanking $N deftly and swiftly.", 0, ch, 0, victim, TO_VICT);
    act("$n turns suddenly, flanking $N deftly and swiftly.", 0, ch, 0, victim, TO_NOTVICT);

    stop_fighting(ch);
    set_fighting(ch, victim);
  }
  else if (GET_OPPONENT(ch) && (victim == GET_OPPONENT(ch))) {
    act("You reposition yourself so that you to flank $N.", 0, ch, 0, victim, TO_CHAR);
    act("$n repositions $mself so that $e flanks you.", 0, ch, 0, victim, TO_VICT);
    act("$n repositions $mself so that $e flanks $N.", 0, ch, 0, victim, TO_NOTVICT);
  }

  /* Maim */
  if (SAME_ROOM(ch, victim) && check_sc_access(ch, SKILL_MAIM) && !affected_by_spell(victim, SKILL_MAIM)) {
    check = number(1, 131) - MAX(GET_STR_TO_HIT(ch), GET_DEX_APP(ch));

    if (check <= GET_LEARNED(ch, SKILL_MAIM)) {
      act("You savagely maim $N, making $M susceptible to extra damage!", FALSE, ch, 0, victim, TO_CHAR);
      act("$n savagely maims you, making you susceptible to extra damage!", FALSE, ch, 0, victim, TO_VICT);
      act("$n savagely maim $N, making $M susceptible to extra damage!", FALSE, ch, 0, victim, TO_NOTVICT);

      enchantment_apply(victim, FALSE, "Savaged (Maim)", SKILL_MAIM, -1, 0, (GET_LEVEL(ch) / 4), 0, 0, 0, maim_enchantment);
    }
  }

  skill_wait(ch, SKILL_FLANK, 2);
}


void do_zeal(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_ZEAL)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon for your zeal to be successful.\n\r", ch);

    return;
  }

  int check = number(1, 101) - MAX(GET_STR_TO_HIT(ch), GET_WIS_APP(ch));

  if (check > GET_LEARNED(ch, SKILL_ZEAL)) {
    act("You fail in your attempt to invoke divine wrath upon your foes.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n fails in an attempt to invoke divine wrath upon $s foes.", FALSE, ch, 0, 0, TO_ROOM);

    skill_wait(ch, SKILL_ZEAL, 2);

    return;
  }

  act("You are empowered by a sense of divine zeal as you tear into your foes.", FALSE, ch, 0, 0, TO_CHAR);
  act("$n is empowered by divine zeal and cleaves through $s foes.", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_victim = world[CHAR_REAL_ROOM(ch)].people, *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if (IS_IMMORTAL(temp_victim) ||
        (temp_victim == ch) ||
        (IS_NPC(temp_victim) && (GET_RIDER(temp_victim) == ch)) ||
        (IS_MORTAL(temp_victim) && (!ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_victim)) || GET_OPPONENT(temp_victim)))) continue;

    hit(ch, temp_victim, SKILL_ZEAL);

    if (CHAR_REAL_ROOM(temp_victim) != NOWHERE) {
      WAIT_STATE(temp_victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
    }
  }

  skill_wait(ch, SKILL_ZEAL, 2);
}


void do_hostile(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_HOSTILE)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch)) {
    send_to_char("You can't change your stance while fighting.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_HOSTILE)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_HOSTILE);

    send_to_char("You place yourself in a normal stance.\n\r", ch);
    act("$n places $mself in a normal stance.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_HOSTILE)) {
    send_to_char("You failed to get into the hostile stance.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_HOSTILE);

  send_to_char("You place yourself in a hostile stance.\n\r", ch);
  act("$n places $mself in a hostile stance.", TRUE, ch, 0, 0, TO_ROOM);
}


void do_defend(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_DEFEND)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch)) {
    send_to_char("You can't change your stance while fighting.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_DEFEND)) {
    affect_from_char(ch, SKILL_DEFEND);

    send_to_char("You place yourself in a normal stance.\n\r", ch);
    act("$n places $mself in a normal stance.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_DEFEND)) {
    send_to_char("You failed to get into a defensive stance.\n\r", ch);

    return;
  }

  affect_apply(ch, SKILL_DEFEND, -1, -100, APPLY_AC, 0, 0);

  send_to_char("You place yourself in a defensive stance.\n\r", ch);
  act("$n places $mself in a defensive stance.", TRUE, ch, 0, 0, TO_ROOM);
}


void do_batter(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim;
  int check;

  if (!ch->skills) return;

  if (IS_MORTAL(ch) &&
      !check_sc_access(ch, SKILL_BATTER))
  {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)))
  {
    if (ch->specials.fighting)
    {
      victim = ch->specials.fighting;
    }
    else
    {
      send_to_char("Batter who?\n\r", ch);
      return;
    }
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }
  else if (IS_MORTAL(ch) &&
           !IS_NPC(victim) &&
           GET_LEVEL(victim) >= LEVEL_IMM)
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);
    return;
  }
  else if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE) &&
           !CHAOSMODE)
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  check = number(1, 131) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_BATTER))
  {
    act("$n tried to batter $N, but misses.", FALSE, ch, NULL, victim, TO_NOTVICT);
    act("$N tried to batter you, but misses.", FALSE, ch, NULL, victim, TO_VICT);
    act("You try to batter $N, but miss.", FALSE, ch, NULL, victim, TO_CHAR);
    damage(ch, victim, 0, SKILL_BATTER, DAM_NO_BLOCK);

    if (IS_NPC(ch) ||
        GET_LEVEL(ch) < LEVEL_IMP)
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    if ((IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, SKILL_BATTER, BT_INVUL)))
    {
      act("You pound on $N with your fists, but your battering has no effect.", FALSE, ch, 0, victim, TO_CHAR);
      act("$N pounds on you with $s fists, but $s battering has no effect.", FALSE, ch, 0, victim, TO_VICT);
      act("$n pounds on $N with $s fists, but $s battering has no effect.", FALSE, ch, 0, victim, TO_NOTVICT);

      damage(ch, victim, 0, SKILL_BATTER, DAM_NO_BLOCK);
    }
    else
    {
      act("You pound on $N with your fists.", FALSE, ch, 0, victim, TO_CHAR);
      act("$N pounds on you with $s fists.", FALSE, ch, 0, victim, TO_VICT);
      act("$n pounds on $N with $s fists.", FALSE, ch, 0, victim, TO_NOTVICT);

      damage(ch, victim, calc_position_damage(GET_POS(victim), GET_LEVEL(ch) * 2), SKILL_BATTER, DAM_PHYSICAL);

      if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim))
      {
        if (GET_POS(victim) > POSITION_SITTING)
        {
          GET_POS(victim) = POSITION_SITTING;
        }
      }
    }

    if (!IS_IMPLEMENTOR(ch))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
}


void do_headbutt(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_HEADBUTT)) {
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
    send_to_char("Headbutt who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("How could you possibly headbutt yourself?\n\r", ch);

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

  int check = number(1, 121) - MAX(GET_STR_TO_HIT(ch), GET_DEX_APP(ch));

  if (check > GET_LEARNED(ch, SKILL_HEADBUTT)) {
    act("You miss $N with your headbutt.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n misses you with $s headbutt.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n misses $N with $s headbutt.", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_HEADBUTT, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_HEADBUTT, 2);

    return;
  }

  if (!breakthrough(ch, victim, SKILL_HEADBUTT, BT_INVUL)) {
    act("You slam your forehead into $N's face, but $E isn't even phased.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n slams his forehead into your face, but you aren't even phased.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n slams $s forehead into $N's face, but $E isn't even phased.", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_HEADBUTT, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_HEADBUTT, 2);

    return;
  }

  int set_pos = stack_position(victim, POSITION_STUNNED);

  act("You slam your forehead into $N's face!", FALSE, ch, NULL, victim, TO_CHAR);
  act("$n slams his forehead into your face!", FALSE, ch, NULL, victim, TO_VICT);
  act("$n slams $s forehead into $N's face!", FALSE, ch, NULL, victim, TO_NOTVICT);

  damage(ch, victim, calc_position_damage(GET_POS(victim), GET_LEVEL(ch) * 4), SKILL_HEADBUTT, DAM_PHYSICAL);

  if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;
  }

  skill_wait(ch, SKILL_HEADBUTT, 2);
}


void do_banzai(CHAR *ch, char *arg, int cmd) {
  const int mana_cost = 75;

  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_BANZAI)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && (GET_MANA(ch) < mana_cost)) {
    send_to_char("You do not have enough mana.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim && IS_ALIVE(GET_OPPONENT(ch)) && SAME_ROOM(ch, GET_OPPONENT(ch))) {
    victim = GET_OPPONENT(ch);
  }

  if (!victim) {
    send_to_char("Banzai charge who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You've had better ideas...\n\r", ch);

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

  /* The Shogun title increases "concentration" rate. */
  if (enchanted_by_type(ch, ENCHANT_SHOGUN)) {
    check -= 5;
  }

  if (check > GET_LEARNED(ch, SKILL_BANZAI)) {
    act("You try to banzai charge $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to banzai charge you, but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to banzai charge $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_BANZAI, DAM_NO_BLOCK);

    if (ch) {
      GET_MANA(ch) -= mana_cost / 2;
    }

    skill_wait(ch, SKILL_BANZAI, 1);

    return;
  }

  if (GET_OPPONENT(ch) && (victim != GET_OPPONENT(ch))) {
    stop_fighting(ch);
    set_fighting(ch, victim);
  }

  act("With a primal yell, you banzai charge $N!", FALSE, ch, 0, victim, TO_CHAR);
  act("With a primal yell, $n banzai charges you!", FALSE, ch, 0, victim, TO_VICT);
  act("With a primal yell, $n banzai charges $N!", FALSE, ch, 0, victim, TO_NOTVICT);

  damage(ch, victim, number(200, 300), SKILL_BANZAI, DAM_PHYSICAL);

  if (SAME_ROOM(victim, ch)) {
    check = number(1, 101) - GET_DEX_APP(ch);

    /* The Shogun title increases "concentration" rate. */
    if (enchanted_by_type(ch, ENCHANT_SHOGUN)) {
      check -= 5;
    }

    if (check <= GET_LEARNED(ch, SKILL_BANZAI)) {
      hit(ch, victim, SKILL_BANZAI);
    }
  }

  if (ch) {
    GET_MANA(ch) -= mana_cost;
  }

  skill_wait(ch, SKILL_BANZAI, 1);
}


void do_mantra(CHAR *ch, char *arg, int cmd) {
  const int mana_cost = 120;

  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_MANTRA)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && (GET_MANA(ch) < mana_cost)) {
    send_to_char("You can't summon enough energy to chant your mantra.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  CHAR *victim = get_char_room_vis(ch, name);

  if (!victim && !(victim = ch)) {
    send_to_char("Chant your mantra to who?\n\r", ch);

    return;
  }

  if (CHAOSMODE && (victim != ch)) {
    send_to_char("You cannot perform this skill on another player during Chaos.\n\r", ch);

    return;
  }

  int check = number(1, 101) - ((GET_INT_CONC(ch) + GET_WIS_CONC(ch)) / 10);

  /* Inner Peace */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_MYSTIC, 2)) {
    check -= 5;
  }

  /* The Shogun title increases "concentration" rate. */
  if (enchanted_by_type(ch, ENCHANT_SHOGUN)) {
    check -= 5;
  }

  /* Take 50% of the mana cost before skill check. */
  GET_MANA(ch) -= mana_cost / 2;

  if ((check > GET_LEARNED(ch, SKILL_MANTRA)) ||
      (affected_by_spell(victim, SPELL_DEGENERATE) && (duration_of_spell(victim, SPELL_DEGENERATE) > (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 9 : 27)))) {
    if (victim != ch) {
      act("You chant your mantra to $N, but nothing happens.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n chants $s mantra to you, but nothing happens.", FALSE, ch, 0, victim, TO_VICT);
      act("$n chants $s mantra to $N, but nothing happens.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    else {
      send_to_char("You chant softly to yourself with no noticeable effect.\n\r", ch);
      act("$n chants softly to $mself with no noticeable effect.", FALSE, ch, 0, 0, TO_ROOM);
    }

    skill_wait(ch, SKILL_MANTRA, 1);

    return;
  }

  /* Take 50% of the mana cost after skill check. */
  GET_MANA(ch) -= mana_cost / 2;

  if (victim != ch) {
    act("You chant softly to $N, healing $S spirit and giving $M life.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n chants softly to you, healing your spirit and giving you life.", FALSE, ch, 0, victim, TO_VICT);
    act("$n chants softly to $N, healing $S spirit and giving $M life.", FALSE, ch, 0, victim, TO_NOTVICT);
  }
  else {
    act("You chant softly, healing your spirit and giving yourself life.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n chants softly, healing $s spirit and giving $mself life.", FALSE, ch, 0, 0, TO_ROOM);
  }

  magic_heal(victim, SKILL_MANTRA, 500, FALSE);

  int modifier = lround((GET_LEVEL(ch) + GET_WIS_APP(ch)) * 1.5);

  /* Check if victim is already affected by Mantra. If so, keep the higher modifier. */
  for (AFF *temp_af = victim->affected; temp_af; temp_af = temp_af->next) {
    if (temp_af->type == SKILL_MANTRA) {
      modifier = MAX(temp_af->modifier, modifier);

      affect_from_char(victim, SKILL_MANTRA);

      break;
    }
  }

  /* TODO: Convert to Enchant using MSG_ROUND. */
  affect_apply(victim, SKILL_MANTRA, 10, modifier, 0, 0, 0);

  skill_wait(ch, SKILL_MANTRA, 1);
}


void do_frenzy(CHAR *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_FRENZY)) {
    send_to_char("You do not have this skill.\n\r", ch);
    return;
  }
  if (ch->specials.fighting) {
    send_to_char("You can't change your mindset while fighting.\n\r", ch);
    return;
  }
  if (!affected_by_spell(ch, SKILL_FRENZY)) {
    if (number(0, 120) > ch->skills[SKILL_FRENZY].learned) {
      send_to_char("You failed to get into the frenzied mindset.\n\r", ch);
      WAIT_STATE(ch, PULSE_VIOLENCE);
      return;
    }
    af.type = SKILL_FRENZY;
    af.duration = -1;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_DUAL;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
    act("You go into a frenzy.", 1, ch, 0, 0, TO_CHAR);
    act("$n goes into a frenzy.", 1, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
  else {
    affect_from_char(ch, SKILL_FRENZY);
    act("$n calms down.", 1, ch, 0, 0, TO_ROOM);
    act("You calm down.", 1, ch, 0, 0, TO_CHAR);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
}


void do_berserk(CHAR *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_BERSERK)) {
    send_to_char("You do not have this skill.\n\r", ch);
    return;
  }
  if (ch->specials.fighting) {
    send_to_char("You can't change your mindset while fighting.\n\r", ch);
    return;
  }
  if (!affected_by_spell(ch, SKILL_BERSERK)) {
    if (number(0, 120) > ch->skills[SKILL_BERSERK].learned) {
      send_to_char("You failed to get into a berserk mindset.\n\r", ch);
      WAIT_STATE(ch, PULSE_VIOLENCE);
      return;
    }
    af.type = SKILL_BERSERK;
    af.duration = -1;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_DUAL;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
    act("You go berserk!", 1, ch, 0, 0, TO_CHAR);
    act("$n goes berserk.", 1, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
  else {
    affect_from_char(ch, SKILL_BERSERK);
    act("$n calms down.", 1, ch, 0, 0, TO_ROOM);
    act("You calm down.", 1, ch, 0, 0, TO_CHAR);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
}


void do_trophy(CHAR *ch, char *arg, int cmd) {
  const int mana_cost = 10;

  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_TROPHY)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  char name[MIL];

  one_argument(arg, name);

  OBJ *corpse = corpse = get_obj_in_list_vis(ch, name, world[CHAR_REAL_ROOM(ch)].contents);

  if (!corpse) {
    send_to_char("Collect a trophy from what?\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch)) {
    send_to_char("Not with your bare hands.\r\n", ch);

    return;
  }

  if ((get_weapon_type(GET_WEAPON(ch)) == TYPE_HIT) ||
      (get_weapon_type(GET_WEAPON(ch)) == TYPE_BLUDGEON) ||
      (get_weapon_type(GET_WEAPON(ch)) == TYPE_WHIP) ||
      (get_weapon_type(GET_WEAPON(ch)) == TYPE_CRUSH)) {
    send_to_char("You need a weapon with an edge or a point in order to craft a trophy.\n\r", ch);

    return;
  }

  if (GET_MANA(ch) < mana_cost) {
    send_to_char("You can't summon enough energy to craft a trophy.\n\r", ch);

    return;
  }

  if (!IS_CORPSE(corpse)) {
    send_to_char("That isn't a corpse.\n\r", ch);

    return;
  }

  if (IS_PC_CORPSE(corpse)) {
    send_to_char("The thought of collecting a trophy from a fellow adventurer makes you cringe!\n\r", ch);

    return;
  }

  if (IS_STATUE(corpse)) {
    send_to_char("You find it impossible to chip away at the stone.\n\r", ch);

    return;
  }

  if (!CORPSE_HAS_TROPHY(corpse)) {
    send_to_char("That corpse has no suitable 'parts' for a trophy.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_TROPHY)) {
    send_to_char("You failed to craft a trophy from the corpse.\n\r", ch);

    return;
  }

  /* Change the material type to the hilariously appropriate CLASS_BLOB to
     represent that its no longer suitable for making trophies. */
  OBJ_MATERIAL(corpse) = CLASS_BLOB;

  CHAR *group_leader = (GET_MASTER(ch) ? GET_MASTER(ch) : ch);

  int num_group_members = 1;

  /* Count the number of characters in the player's group. */
  for (FOL *follower = group_leader->followers; follower; follower = follower->next) {
    CHAR *temp_ch = follower->follower;

    if (!temp_ch || !SAME_ROOM(ch, temp_ch) || !SAME_GROUP(group_leader, temp_ch) || affected_by_spell(temp_ch, SKILL_TROPHY)) continue;

    num_group_members++;
  }

  act("You craft a trophy from the $p.", FALSE, ch, corpse, 0, TO_CHAR);
  act("$n crafts a trophy from the $p.", FALSE, ch, corpse, 0, TO_ROOM);

  skill_wait(ch, SKILL_TROPHY, 1);

  /* Apply the affects to the player. */
  if (!affected_by_spell(ch, SKILL_TROPHY)) {
    act("You are inspired by the trophy's impressive quality.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n is inspired by the trophy's impressive quality.", FALSE, ch, 0, 0, TO_ROOM);

    affect_apply(ch, SKILL_TROPHY, 10, 2, APPLY_HITROLL, 0, 0);
    affect_apply(ch, SKILL_TROPHY, 10, 2, APPLY_DAMROLL, 0, 0);

    GET_MANA(ch) -= mana_cost;
  }

  if (GET_MANA(ch) < mana_cost) {
    act("You feel exhausted from the effort of crafting your trophy.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n looks exhausted after crafting $s trophy.", FALSE, ch, 0, 0, TO_ROOM);

    return;
  }

  /* Apply the affects to the group leader. */
  if ((group_leader != ch) && SAME_ROOM(group_leader, ch) && SAME_GROUP(ch, group_leader) && !affected_by_spell(group_leader, SKILL_TROPHY)) {
    act("You are inspired by the trophy's impressive quality.", FALSE, group_leader, 0, 0, TO_CHAR);
    act("$n is inspired by the trophy's impressive quality.", FALSE, group_leader, 0, 0, TO_ROOM);

    affect_apply(group_leader, SKILL_TROPHY, 10, 2, APPLY_HITROLL, 0, 0);
    affect_apply(group_leader, SKILL_TROPHY, 10, 2, APPLY_DAMROLL, 0, 0);

    GET_MANA(ch) -= mana_cost;
  }

  /* Loop through all of the members in group_leader's group and apply the affects to them. */
  for (FOL *follower = group_leader->followers; follower; follower = follower->next) {
    if (GET_MANA(ch) < mana_cost) {
      act("You feel exhausted from the effort of crafting your trophy.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n looks exhausted after crafting $s trophy.", FALSE, ch, 0, 0, TO_ROOM);

      return;
    }

    CHAR *group_member = follower->follower;

    if (!group_member || (group_member == ch) || !SAME_ROOM(group_member, ch) || !SAME_GROUP(ch, group_leader) || affected_by_spell(group_member, SKILL_TROPHY)) continue;

    act("You are inspired by the trophy's impressive quality.", FALSE, group_member, 0, 0, TO_CHAR);
    act("$n is inspired by the trophy's impressive quality.", FALSE, group_member, 0, 0, TO_ROOM);

    affect_apply(group_member, SKILL_TROPHY, 10, 2, APPLY_HITROLL, 0, 0);
    affect_apply(group_member, SKILL_TROPHY, 10, 2, APPLY_DAMROLL, 0, 0);

    GET_MANA(ch) -= mana_cost;
  }
}


void do_clobber(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_CLOBBER)) {
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
    send_to_char("Clobber who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("You feel like a crazy person just thinking about it...\n\r", ch);

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

  int check = number(1, 121) - MAX(GET_STR_TO_HIT(ch), GET_DEX_APP(ch));

  if (check > GET_LEARNED(ch, SKILL_CLOBBER)) {
    act("Your attempt to clobber $N misses everything but the air.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n flails about wildly, missing you with $s erratic swinging.", FALSE, ch, 0, victim, TO_VICT);
    act("$n flails about wildly, missing $N with $s erratic swinging.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_CLOBBER, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_CLOBBER, 2);

    return;
  }

  int target_pos = GET_POS(victim);

  int rnd = number(1, 100);

  /* 25% Mortally Wounded */
  if (rnd <= 25) {
    target_pos = POSITION_MORTALLYW;
  }
  /* 25% Incapacitated */
  else if (rnd <= 50) {
    target_pos = POSITION_INCAP;
  }
  /* 50% Stunned */
  else {
    target_pos = POSITION_STUNNED;
  }

  int set_pos = stack_position(victim, target_pos);

  act("You clobber $N with windmilling fists!", FALSE, ch, 0, victim, TO_CHAR);
  act("You're clobbered by $n's windmilling fists!", FALSE, ch, 0, victim, TO_VICT);
  act("$n clobbers $N with $s windmilling fists!", FALSE, ch, 0, victim, TO_NOTVICT);

  damage(ch, victim, calc_position_damage(GET_POS(victim), number(GET_LEVEL(ch), GET_LEVEL(ch) * 4)), SKILL_CLOBBER, DAM_PHYSICAL);

  if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;
  }

  skill_wait(ch, SKILL_CLOBBER, 2);
}


void do_victimize(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_VICTIMIZE)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_VICTIMIZE)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_VICTIMIZE);

    send_to_char("You decide to show a modicum of mercy to your victims.\n\r", ch);

    return;
  }

  if (IS_GOOD(ch)) {
    send_to_char("You are much too friendly to do that.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_VICTIMIZE)) {
    send_to_char("You can't come up with any good taunts or threats.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_VICTIMIZE);

  send_to_char("You will now take every opportunity to victimize your foe.\n\r", ch);
}


void do_shadowstep(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_SHADOWSTEP)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_SHADOWSTEP)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_SHADOWSTEP);

    send_to_char("You will no longer slip into the shadows to attack your victims.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_SHADOWSTEP)) {
    send_to_char("You can't seem to blend in with the shadows.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_SHADOWSTEP);

  send_to_char("You will now slip into the shadows to attack your victims.\n\r", ch);
}


void do_evasion(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_EVASION)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch)) {
    send_to_char("You can't focus well enough to do that while fighting.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_EVASION)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_EVASION);

    send_to_char("You become less evasive and set yourself into a normal fighting stance.\n\r", ch);
    act("$n becomes less evasive and sets $mself into a normal fighting stance.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_EVASION)) {
    send_to_char("You failed to focus on evading attacks.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_EVASION);

  send_to_char("You focus on evading attacks and fighting defensively.\n\r", ch);
  act("$n focuses on evading attacks and fighting defensively.", TRUE, ch, 0, 0, TO_ROOM);
}


void do_dirty_tricks(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_DIRTY_TRICKS)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_DIRTY_TRICKS)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_DIRTY_TRICKS);

    send_to_char("You'll fight fair... for now.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_DIRTY_TRICKS)) {
    send_to_char("You have trouble coming up with a strategy for fighting dirty.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_DIRTY_TRICKS);

  send_to_char("You will start using dirty tricks to get the upper hand in combat.\n\r", ch);
}


void do_trip(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_TRIP)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_TRIP)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_TRIP);

    send_to_char("You will no longer try to trip your enemies during combat.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_TRIP)) {
    send_to_char("You don't feel quite skilled enough to trip up your enemies.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_TRIP);

  send_to_char("You will try to trip your enemies when you circle around them.\n\r", ch);
}


void do_trusty_steed(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_TRUSTY_STEED)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_TRUSTY_STEED)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_TRUSTY_STEED);

    send_to_char("You will no longer summon your trusty steed to aid you in battle.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_TRUSTY_STEED)) {
    send_to_char("You fail to establish a spirit link with your trusty steed in the astral plane.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_TRUSTY_STEED);

  send_to_char("You will summon your trusty steed when pummeling or smiting your foes.\n\r", ch);
}


void do_snipe(CHAR *ch, char *arg, int cmd) {
  if (!ch || !GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_SNIPE)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (IS_SET(GET_TOGGLES(ch), TOG_SNIPE)) {
    REMOVE_BIT(GET_TOGGLES(ch), TOG_SNIPE);

    send_to_char("You will no longer attempt to snipe opponents when they are weakened.\n\r", ch);

    return;
  }

  if (number(1, SKILL_MAX_PRAC) > GET_LEARNED(ch, SKILL_SNIPE)) {
    send_to_char("You're unable to concetrate well enough to make accurate headshots.\n\r", ch);

    return;
  }

  SET_BIT(GET_TOGGLES(ch), TOG_SNIPE);

  send_to_char("You'll now take advantage of your opponents weakness and attempt to snipe.\n\r", ch);
}
