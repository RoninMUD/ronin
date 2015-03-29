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

extern int CHAOSMODE;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct dex_app_type dex_app[];
extern struct wis_app_type wis_app[];
extern struct obj_data  *object_list;

int calc_position_damage(int position, int dam);
int stack_position(CHAR *ch, int target_position);
void do_move(struct char_data *ch, char *argument, int cmd);
void skill_wait(CHAR *ch, int skill, int wait);


void do_meditate(CHAR *ch, char *argument, int cmd)
{
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_MEDITATE))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch))
  {
    send_to_char("You can't meditate while fighting!\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_MEDITATE))
  {
    send_to_char("You're already meditating.\n\r", ch);

    return;
  }

  if (number(0, 85) > GET_LEARNED(ch, SKILL_MEDITATE))
  {
    send_to_char("You failed to focus your thoughts.\n\r", ch);
  }
  else
  {
    af.type = SKILL_MEDITATE;
    af.duration = CHAOSMODE ? 12 : 33;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You gaze inward and focus on healing.\n\r", ch);
    act("$n enters a deep trance.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void do_protect(CHAR *ch, char *argument, int cmd)
{
  char buf[MIL];
  CHAR *victim = NULL;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_PROTECT))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (CHAOSMODE)
  {
    send_to_char("Protect someone during Chaos? Might as well just quit now...\n\r", ch);

    return;
  }

  one_argument(argument, buf);

  if (!*buf)
  {
    send_to_char("Protect who?\n\r", ch);

    return;
  }

  if (!(victim = get_char_room_vis(ch, buf)))
  {
    send_to_char("That player is not here.\n\r", ch);

    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Protect a potential enemy? Impossible!\n\r", ch);

    return;
  }

  if (ch == victim)
  {
    send_to_char("You just protect yourself.\n\r", ch);
    act("$n just protects $mself.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_NOTVICT);

    if (GET_PROTECTEE(ch))
    {
      act("$n stops protecting you.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_VICT);

      GET_PROTECTOR(GET_PROTECTEE(ch)) = NULL;
    }

    GET_PROTECTEE(ch) = NULL;

    return;
  }

  if (affected_by_spell(ch, SKILL_BERSERK))
  {
    send_to_char("You can't protect anyone while berserked.\n\r", ch);

    return;
  }

  if (GET_PROTECTEE(victim))
  {
    act("$N is already protecting someone.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if (GET_PROTECTEE(ch))
  {
    act("You stop protecting $N.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_CHAR);
    act("$n stops protecting you.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_VICT);
    act("$n stops protecting $N.", FALSE, ch, 0, GET_PROTECTEE(ch), TO_NOTVICT);

    GET_PROTECTOR(GET_PROTECTEE(ch)) = NULL;
    GET_PROTECTEE(ch) = NULL;
  }

  if ((number(1, 131) > GET_LEARNED(ch, SKILL_PROTECT)))
  {
    act("You failed to start protecting $N.", FALSE, ch, 0, victim, TO_CHAR);
  }
  else
  {
    act("You start trying to protect $N.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n starts trying to protect you.", FALSE, ch, 0, victim, TO_VICT);
    act("$n starts trying to protect $N.", FALSE, ch, 0, victim, TO_NOTVICT);

    GET_PROTECTEE(ch) = victim;
    GET_PROTECTOR(victim) = ch;
  }
}


void do_backfist(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_BACKFIST)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
    send_to_char("Backfist who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("That seems incredibly painful...\n\r", ch);

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

  check = number(1, 121) - GET_DEX_APP(ch);

  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 10;
  }

  if (check > GET_LEARNED(ch, SKILL_BACKFIST)) {
    act("You try to backfist $N but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to backfist $N but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n tries to backfist you but fails.", FALSE, ch, 0, victim, TO_VICT);

    damage(ch, victim, 0, SKILL_BACKFIST, DAM_SKILL);

    skill_wait(ch, SKILL_BACKFIST, 2);
  }
  else {
    set_pos = stack_position(victim, POSITION_STUNNED);

    act("With a sickening crunch you hit $N with a huge backfist.", FALSE, ch, 0, victim, TO_CHAR);
    act("With a sickening crunch $n hits $N with a huge backfist.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("With a sickening crunch $n hits you with a huge backfist.", FALSE, ch, 0, victim, TO_VICT);

    damage(ch, victim, calc_position_damage(GET_POS(victim), lround(GET_LEVEL(ch) * 1.25)), SKILL_BACKFIST, DAM_PHYSICAL);

    if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
      GET_POS(victim) = set_pos;

      /* Can't use skill_wait() since this applies to victim. */
      WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
    }

    skill_wait(ch, SKILL_BACKFIST, 2);
  }
}


void do_pray(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

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

  if (number(0, 85) > GET_LEARNED(ch, SKILL_PRAY)) {
    send_to_char("You failed to focus your thoughts in prayer.\n\r", ch);
  }
  else {
    af.type = SKILL_PRAY;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You bow your head and begin your prayer.\n\r", ch);
    act("$n bows $s head and begins praying.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void do_tigerkick(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_TIGERKICK)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
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

  check = number(1, 121) - GET_DEX_APP(ch);

  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= GET_LEVEL(ch) / 10;
  }

  if (check > GET_LEARNED(ch, SKILL_TIGERKICK)) {
    act("You try to tigerkick $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to tigerkick you, but misses.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to tigerkick $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_TIGERKICK, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_TIGERKICK, 2);
  }
  else {
    if (!breakthrough(ch, victim, BT_INVUL)) {
      act("$N seems completely affected by your beautiful tigerkick.", FALSE, ch, 0, victim, TO_CHAR);
      act("You feel completely affected by $n's beautiful tigerkick.", FALSE, ch, 0, victim, TO_VICT);
      act("$N seems completely affected by $n's beautiful tigerkick.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    else {
      set_pos = stack_position(victim, POSITION_STUNNED);

      act("You drive your foot into $N's chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_CHAR);
      act("$n drives $s foot into your chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_VICT);
      act("$n drives $s foot into $N's chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_NOTVICT);

      damage(ch, victim, calc_position_damage(GET_POS(victim), lround(GET_LEVEL(ch) * 1.75)), SKILL_TIGERKICK, DAM_PHYSICAL);

      GET_MANA(ch) = MIN(GET_MANA(ch) + MIN(MAX((GET_MAX_HIT(victim) / 5000), 2), 10), GET_MAX_MANA(ch));

      if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = set_pos;

        /* Can't use skill_wait() since this applies to victim. */
        WAIT_STATE(victim, PULSE_VIOLENCE * (CHAOSMODE ? number(1, 2) : 2));
      }

      skill_wait(ch, SKILL_TIGERKICK, 2);
    }
  }
}


/* Portions of scan taken from a snippet (do_scan) by sjmaster@mit.edu */
void list_scanned_chars(CHAR *list, CHAR *ch, int distance, int door) {
  CHAR *i = NULL;
  int count = 0;
  char buf[MSL];
  char buf2[MIL];

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

  buf[0] = '\0';
  for (i = list; i; i = i->next_in_room) {
    if (!CAN_SEE(ch, i)) continue;

    count--;

    if (!*buf) {
      sprintf(buf, "You see %s", (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)));
    }
    else {
      sprintf(buf, "%s%s", buf, (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)));
    }

    if (count > 1) {
      strcat(buf, ", ");
    }
    else if (count == 1) {
      strcat(buf, " and ");
    }
    else {
      sprintf(buf2, " %s %s.\n\r", how_far[distance], dir_name[door]);
      strcat(buf, buf2);
    }
  }

  send_to_char(buf, ch);
}


/* Portions of scan taken from a snippet (do_scan) by sjmaster@mit.edu */
void do_scan(CHAR *ch, char *argument, int cmd) {
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

  if (IS_MORTAL(ch) && (GET_CLASS(ch) != CLASS_THIEF)) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_LEVEL(ch) < 40) {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You can't see a thing, you're blind!\n\r", ch);

    return;
  }

  send_to_char("You quickly scan the area.\n\r", ch);

  for (dir = 0; dir < 6; dir++) {
    room = CHAR_REAL_ROOM(ch);

    if (!CAN_GO(ch, dir)) continue;

    if (number(1, 121) > GET_LEARNED(ch, SKILL_SCAN)) {
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


void do_camp(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_CAMP) &&
      GET_CLASS(ch) != CLASS_BARD) {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if ((GET_CLASS(ch) == CLASS_BARD) && (GET_LEVEL(ch) < 35))
  {
    send_to_char("You don't know this skill yet.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_CAMP)) {
    send_to_char("You're already camping.\n\r", ch);

    return;
  }

  if (IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CLUB)) {
    send_to_char("There is already a camp here.\n\r", ch);

    return;
  }

  if (GET_MOUNT(ch)) {
    send_to_char("You must dismount first.\n\r", ch);

    return;
  }

  if (count_mobs_real_room_except_followers(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't camp when there are enemies in the room!\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_CAMP)) {
    send_to_char("You failed to set up camp.\n\r", ch);
  }
  else {
    af.type = SKILL_CAMP;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    GET_POS(ch) = POSITION_RESTING;

    SET_BIT(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CLUB);

    send_to_char("You quickly set up a camp here, then sit down and rest.\n\r", ch);
    act("$n quickly sets up a camp here, then sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void do_blitz(CHAR *ch, char *argument, int cmd)
{
  char name[MIL];
  CHAR* victim = NULL;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_BLITZ))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch)))
  {
    send_to_char("Blitz who?\n\r", ch);

    return;
  }

  if (victim == ch)
  {
    send_to_char("Why would you even consider such a thing?\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  check = number(1, 131) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_BLITZ))
  {
    act("You try to blitz $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to blitz you, but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to blitz $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_BLITZ, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_BLITZ, 2);
  }
  else
  {
    hit(ch, victim, SKILL_BLITZ);

    skill_wait(ch, SKILL_BLITZ, 2);
  }
}


void do_lunge(CHAR *ch, char *argument, int cmd) {
  CHAR *victim = NULL;
  char name[MIL];
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_LUNGE)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
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

  check = number(1, 131) - GET_DEX_APP(ch);

  /* Bonus for lunging at a target already in combat. */
  if (GET_OPPONENT(victim)) {
    check -= 5;
  }

  if (AWAKE(victim) && (check > GET_LEARNED(ch, SKILL_LUNGE))) {
    act("You try to lunge at $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to lunge at you, but fails.", FALSE, victim, 0, ch, TO_CHAR);
    act("$n tries to lunge at $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_LUNGE, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_LUNGE, 2);
  }
  else {
    hit(ch, victim, SKILL_LUNGE);

    skill_wait(ch, SKILL_LUNGE, 2);
  }
}


void do_fade(CHAR *ch, char *argument, int cmd) {
  int modifier = 0;
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_FADE)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_FADE)) {
    affect_from_char(ch, SKILL_FADE);

    send_to_char("You emerge from the background and into view.\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_FADE)) {
    send_to_char("You failed to fade into the background.\n\r", ch);
  }
  else {
    if (GET_LEVEL(ch) < 35) {
      modifier = -5;
    }
    else if (GET_LEVEL(ch) < 40) {
      modifier = -4;
    }
    else if (GET_LEVEL(ch) < 45) {
      modifier = -3;
    }
    else if (GET_LEVEL(ch) < 50) {
      modifier = -2;
    }
    else {
      modifier = -1;
    }

    af.type = SKILL_FADE;
    af.duration = -1;
    af.modifier = modifier;
    af.location = APPLY_HITROLL;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You quietly fade into the background and blend in with your surroundings.\n\r", ch);
    act("$n quietly fades into the background and blends in with $s surroundings.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void do_vehemence(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_VEHEMENCE)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch)) {
    send_to_char("You can't shake off your feeling of vehemence while in combat.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_VEHEMENCE)) {
    affect_from_char(ch, SKILL_VEHEMENCE);

    send_to_char("Your vehemence diminishes.\n\r", ch);

    return;
  }

  if (number(0, 85) > GET_LEARNED(ch, SKILL_VEHEMENCE)) {
    send_to_char("You are unable to fill yourself with a sense of vehemence.\n\r", ch);
  }
  else {
    af.type = SKILL_VEHEMENCE;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You feel a sudden rush of vehemence wash over you.\n\r", ch);
  }
}


void do_switch(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_SWITCH)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (!GET_OPPONENT(ch)) {
    send_to_char("You aren't fighting anything.\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Switch to who?\n\r", ch);

    return;
  }

  if (victim == ch) {
    send_to_char("Maybe you should surrender instead...\n\r", ch);

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

  if ((check > GET_LEARNED(ch, SKILL_SWITCH)) || IS_AFFECTED(ch, AFF_FURY)) {
    send_to_char("You failed to switch.\n\r", ch);

    skill_wait(ch, SKILL_SWITCH, 2);
  }
  else {
    act("$n switches $s fight to $N", 0, ch, 0, victim, TO_NOTVICT);
    act("You switch your fight to $N!", 0, ch, 0, victim, TO_CHAR);
    act("$n switches $s fight to you!", 0, ch, 0, victim, TO_VICT);

    stop_fighting(ch);
    set_fighting(ch, victim);

    skill_wait(ch, SKILL_SWITCH, 2);
  }
}


void do_smite(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_SMITE)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
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

  check = number(1, 111) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_SMITE)) {
    act("You try to smite smite $M but your concentration falters.", 0, ch, 0, victim, TO_CHAR);
    act("$n tries to smite you but $s concentration falters.", 0, ch, 0, victim, TO_VICT);
    act("$n tries to smite $N but $s concentration falters.", 0, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_TRUSTY_STEED, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_SMITE, 2);
  }
  else {
    if (victim != GET_OPPONENT(ch)) {
      act("You call upon your divine energy and switch the focus of your attacks to $N.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n calls upon $s divine energy and switches the focus of $s attacks to you!", FALSE, ch, 0, victim, TO_VICT);
      act("$n calls upon $s divine energy and switches the focus of $s attacks to $N.", FALSE, ch, 0, victim, TO_NOTVICT);
    }

    stop_fighting(ch);
    set_fighting(ch, victim);

    act("You raise your weapon high to smite $M with holy vengeance!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n raises $s weapon high to smite you with holy vengence!", FALSE, ch, 0, victim, TO_VICT);
    act("$n raises $s weapon high to smite $N with holy vengeance!", FALSE, ch, 0, victim, TO_NOTVICT);

    hit(ch, victim, TYPE_UNDEFINED);

    if ((CHAR_REAL_ROOM(victim) != NOWHERE) && breakthrough(ch, victim, BT_INVUL)) {
      GET_POS(victim) = stack_position(victim, POSITION_RESTING);
    }

    skill_wait(ch, SKILL_SMITE, 2);
  }

  /* Trusty Steed */
  if ((CHAR_REAL_ROOM(victim) != NOWHERE) &&
      affected_by_spell(ch, SKILL_TRUSTY_STEED) &&
      breakthrough(ch, victim, BT_INVUL)) {
    check = number(1, 121) - GET_WIS_APP(ch);

    if (check <= GET_LEARNED(ch, SKILL_TRUSTY_STEED)) {
      set_pos = stack_position(victim, POSITION_SITTING);

      act("You summon forth your trusty steed and it tramples $N with spiritual energy!", FALSE, ch, 0, victim, TO_CHAR);
      act("$n summons forth $s trusty steed and it tramples you with spiritual energy!", FALSE, ch, 0, victim, TO_VICT);
      act("$n summons forth $s trusty steed and it tramples $N with spiritual energy!", FALSE, ch, 0, victim, TO_NOTVICT);

      damage(ch, victim, calc_position_damage(GET_POS(victim), lround(GET_LEVEL(ch) * 1.5)), SKILL_TRUSTY_STEED, DAM_NO_BLOCK);

      GET_POS(victim) = set_pos;
    }
  }
}


void do_flank(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_FLANK)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
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

  check = number(1, 131) - GET_DEX_APP(ch);

  if (AWAKE(victim) && (check > GET_LEARNED(ch, SKILL_FLANK))) {
    act("You try to flank $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to flank you, but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to flank $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_FLANK, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_FLANK, 2);
  }
  else {
    hit(ch, victim, SKILL_FLANK);

    skill_wait(ch, SKILL_FLANK, 2);
  }
}


/* Used to be Sweep */
void do_zeal(CHAR *ch, char *argument, int cmd) {
  CHAR *tmp_victim = NULL;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_ZEAL)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch)) {
    send_to_char("You need to wield a weapon for it to be successful.\n\r", ch);

    return;
  }

  check = number(1, 101) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_ZEAL)) {
    act("You fail in your attempt to invoke divine wrath upon your foes.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n fails in an attempt to invoke divine wrath upon $s foes.", FALSE, ch, 0, 0, TO_ROOM);

    skill_wait(ch, SKILL_ZEAL, 2);
  }
  else {
    act("You are empowered by a sense of divine zeal as you tear into your foes.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n is empowered by divine zeal and cleaves through $s foes.", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = tmp_victim->next_in_room) {
      if (tmp_victim == ch) continue;

      if (IS_NPC(tmp_victim) && GET_RIDER(tmp_victim) != ch) {
        hit(ch, tmp_victim, TYPE_UNDEFINED);

        skill_wait(tmp_victim, SKILL_ZEAL, 2);

        continue;
      }

      if (IS_MORTAL(tmp_victim) &&
          ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) &&
          !GET_OPPONENT(tmp_victim)) {
        hit(ch, tmp_victim, TYPE_UNDEFINED);

        continue;
      }
    }

    skill_wait(ch, SKILL_ZEAL, 3);
  }
}


void do_hostile(CHAR *ch, char *argument, int cmd) {
  int check = 0;
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_HOSTILE)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch)) {
    send_to_char("You can't change your stance while fighting.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_HOSTILE)) {
    affect_from_char(ch, SKILL_HOSTILE);

    send_to_char("You place yourself in a normal stance.\n\r", ch);
    act("$n places $mself in a normal stance.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  check = number(1, 121) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_HOSTILE)) {
    send_to_char("You failed to get into the hostile stance.\n\r", ch);
  }
  else {
    af.type = SKILL_HOSTILE;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You place yourself in a hostile stance.", ch);
    act("$n places $mself in a hostile stance.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void do_defend(CHAR *ch, char *argument, int cmd) {
  int check = 0;
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_DEFEND)) {
    send_to_char("You do not have this skill.\n\r", ch);

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

  check = number(1, 121) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_DEFEND)) {
    send_to_char("You failed to get into a defensive stance.\n\r", ch);
  }
  else {
    af.type = SKILL_DEFEND;
    af.duration = -1;
    af.modifier = -100;
    af.location = APPLY_AC;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You place yourself in a defensive stance.", ch);
    act("$n places $mself in a defensive stance.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void do_assassinate(CHAR *ch, char *argument, int cmd) {
  char name[MIL];
  char buf[MIL];
  int dir = NOWHERE;
  int room = NOWHERE;
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_ASSASSINATE)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_MOUNT(ch)) {
    send_to_char("You must dismount first.\n\r", ch);

    return;
  }

  if (!affected_by_spell(ch, AFF_HIDE) &&
      !affected_by_spell(ch, AFF_SNEAK) &&
      !affected_by_spell(ch, SPELL_INVISIBLE) &&
      !affected_by_spell(ch, SPELL_IMP_INVISIBLE)) {
    send_to_char("You need to be hiding, sneaking or invisible to succeed.\n\r", ch);

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


void do_batter(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim;
  int check;

  if (!ch->skills) return;

  if (IS_MORTAL(ch) &&
      !check_sc_access(ch, SKILL_BATTER))
  {
    send_to_char("You do not have this skill.\n\r", ch);
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

  if (check > ch->skills[SKILL_BATTER].learned)
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
    if ((IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL)))
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


void do_charge(struct char_data *ch, char *argument, int cmd) {
  CHAR *leader, *victim;
  struct follow_type *Followers = NULL;
  int percent;
  //int hps;
  char name[256];

  if (!ch->skills) return;
  if (!check_sc_access(ch, SKILL_CHARGE)) {
    send_to_char("You do not have this skill.\n\r", ch);
    return;
  }

  if (ch->specials.fighting) {
    send_to_char("You are already fighting.\n\r", ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Charge who?\n\r", ch);
    return;
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }
  else if (victim->specials.fighting)
  {
    act("$N is already fighting someone.", 0, ch, 0, victim, TO_CHAR);
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

  percent = number(1, 131);

  if (percent > ch->skills[SKILL_CHARGE].learned) {
    act("You try to lead a charge against $N but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to lead a charge against you, but fails.", FALSE, victim, 0, ch, TO_CHAR);
    act("$n tries to lead a charge against $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_CHARGE, DAM_NO_BLOCK);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    return;
  }

  /* find the leader */
  if ((!IS_AFFECTED(ch, AFF_GROUP) || ch->master == NULL)) {
    leader = ch;
  }
  else {
    if (!IS_AFFECTED(ch->master, AFF_GROUP))
      leader = ch;
    else
      leader = ch->master;
  }

  act("You lead a charge against $N!", FALSE, ch, 0, victim, TO_CHAR);
  act("$N leads a charge against you!", FALSE, victim, 0, ch, TO_CHAR);
  act("$n leads a charge against $N!", FALSE, ch, 0, victim, TO_NOTVICT);

  /* charger hits */
  //hps = GET_HIT(victim);
  hit(ch, victim, SKILL_CHARGE);
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

  if (!victim || CHAR_REAL_ROOM(victim) == NOWHERE) return;
  /*if(GET_HIT(victim)>=hps) return;*/ /* this is a check for a missed charge */

  /* Go through the followers */
  for (Followers = leader->followers; Followers; Followers = Followers->next)
  {
    if (Followers->follower->desc) /*make sure they're connected*/
    {
      if (IS_AFFECTED(Followers->follower, AFF_GROUP) &&
          CHAR_REAL_ROOM(Followers->follower) == CHAR_REAL_ROOM(ch) && Followers->follower != ch)
      {
        send_to_char("You join in the charge!\n\r", Followers->follower);
        hit(Followers->follower, victim, SKILL_CHARGE);
        if (!victim || CHAR_REAL_ROOM(victim) == NOWHERE) return;
      }
    }
  }
  /* Have leader charge */
  if (CHAR_REAL_ROOM(leader) == CHAR_REAL_ROOM(ch) && leader != ch) {
    send_to_char("You join in the charge!\n\r", leader);
    hit(leader, victim, SKILL_CHARGE);
  }
}


void do_headbutt(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;
  int wait = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_HEADBUTT)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
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

  check = number(1, 131) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_HEADBUTT)) {
    act("You miss $N with your headbutt.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n misses you with $s headbutt.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n misses $N with $s headbutt.", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_HEADBUTT, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_HEADBUTT, 2);
  }
  else {
    if (!breakthrough(ch, victim, BT_INVUL)) {
      act("You slam your forehead into $N's face, but $E isn't even phased.", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n slams his forehead into your face, but you aren't even phased.", FALSE, ch, NULL, victim, TO_VICT);
      act("$n slams $s forehead into $N's face, but $E isn't even phased.", FALSE, ch, NULL, victim, TO_NOTVICT);

      damage(ch, victim, 0, SKILL_HEADBUTT, DAM_NO_BLOCK);
    }
    else {
      set_pos = stack_position(victim, POSITION_STUNNED);

      act("You slam your forehead into $N's face!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n slams his forehead into your face!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n slams $s forehead into $N's face!", FALSE, ch, NULL, victim, TO_NOTVICT);

      damage(ch, victim, calc_position_damage(GET_POS(victim), 200), SKILL_HEADBUTT, DAM_PHYSICAL);

      /* Can't use skill_wait() since this applies to victim. */
      if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = set_pos;
      }
    }

    wait = number(1, 8);
    wait = ((wait == 1) ? 1 : ((wait == 2 || wait == 3) ? 2 : 3));

    skill_wait(ch, SKILL_HEADBUTT, wait);
  }
}


void do_banzai(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int mana_cost = 75;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_BANZAI)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && (GET_MANA(ch) < mana_cost)) {
    send_to_char("You do not have enough mana.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
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

  check = number(1, 101) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_BANZAI)) {
    act("You try to banzai charge $N but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to banzai charge you but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to banzai charge $N but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_BANZAI, DAM_NO_BLOCK);

    GET_MANA(ch) = MAX(GET_MANA(ch) - (mana_cost / 2), 0);

    skill_wait(ch, SKILL_BANZAI, 2);
  }
  else {
    act("With a primal yell, you banzai charge $N.", FALSE, ch, 0, victim, TO_CHAR);
    act("With a primal yell, $n banzai charges you.", FALSE, ch, 0, victim, TO_VICT);
    act("With a primal yell, $n banzai charges $N.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, number(200, 300), SKILL_BANZAI, DAM_SKILL);

    if (SAME_ROOM(victim, ch)) {
      hit(ch, victim, SKILL_BANZAI);
    }

    GET_MANA(ch) = MAX(GET_MANA(ch) - mana_cost, 0);

    skill_wait(ch, SKILL_BANZAI, 1);
  }
}


void do_mantra(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int mana_cost = 120;
  int modifier = 0;
  AFF *tmp_af = NULL;
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_MANTRA)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && GET_MANA(ch) < mana_cost) {
    send_to_char("You do not have enough mana.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!*name) {
    victim = ch;
  }
  else if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Chant your mantra to who?\n\r", ch);

    return;
  }

  if (CHAOSMODE && (victim != ch)) {
    send_to_char("You cannot perform this skill on another player during Chaos.\n\r", ch);

    return;
  }

  check = number(1, 101) - GET_WIS_APP(ch);

  /* Mantra automatically fails when used on a degenerated target. */
  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      (duration_of_spell(victim, SPELL_DEGENERATE) > (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 9 : 27))) {
    check = 256;
  }

  if (check > GET_LEARNED(ch, SKILL_MANTRA)) {
    if (victim != ch) {
      act("You chant your mantra to $N, but nothing happens.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n chants $s mantra to you, but nothing happens.", FALSE, ch, 0, victim, TO_VICT);
      act("$n chants $s mantra to $N, but nothing happens.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    else {
      send_to_char("You chant softly to yourself with no noticeable affect.\n\r", ch);
      act("$n chants softly to $mself with no noticeable affect.", FALSE, ch, 0, 0, TO_ROOM);
    }

    GET_MANA(ch) = MAX(GET_MANA(ch) - (mana_cost / 2), 0);

    skill_wait(ch, SKILL_MANTRA, 2);
  }
  else {
    if (victim != ch) {
      act("You chant softly to $N, healing $S spirit and giving $M life.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n chants softly to you, healing your spirit and giving you life.", FALSE, ch, 0, victim, TO_VICT);
      act("$n chants softly to $N, healing $S spirit and giving $M life.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    else {
      send_to_char("You chant softly, healing your spirit and giving yourself life.\n\r", ch);
      act("$n chants softly, healing $s spirit and giving $mself life.", FALSE, ch, 0, 0, TO_ROOM);
    }

    GET_MANA(ch) = MAX(GET_MANA(ch) - mana_cost, 0);

    magic_heal(victim, SKILL_MANTRA, 500, FALSE);

    modifier = (GET_LEVEL(ch) + (5 * GET_WIS_APP(ch)));

    for (tmp_af = victim->affected; tmp_af; tmp_af = tmp_af->next) {
      if (tmp_af->type == SKILL_MANTRA && tmp_af->modifier >= modifier) {
        affect_from_char(victim, SKILL_MANTRA);
        break;
      }
    }

    af.type = SKILL_MANTRA;
    af.duration = 10;
    af.modifier = modifier;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(victim, &af);

    skill_wait(ch, SKILL_MANTRA, 2);
  }
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
    af.location = APPLY_NONE;
    af.bitvector = 0;
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
    af.location = APPLY_NONE;
    af.bitvector = 0;
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


void do_trophy(CHAR *ch, char *argument, int cmd) {
  OBJ *corpse = NULL;
  OBJ *trophy = NULL;
  char obj_name[MSL];
  char trophy_name[MSL];
  char buf[MIL];

  if (!check_sc_access(ch, SKILL_TROPHY)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && !GET_WEAPON(ch)) {
    send_to_char("Not with your bare hands.\r\n", ch);

    return;
  }

  argument = one_argument(argument, obj_name);

  if (!(corpse = get_obj_in_list_vis(ch, obj_name, world[CHAR_REAL_ROOM(ch)].contents))) {
    send_to_char("Collect a trophy from what?\n\r", ch);

    return;
  }

  if (GET_ITEM_TYPE(corpse) != ITEM_CONTAINER && OBJ_VALUE3(corpse) != 1) {
    send_to_char("That isn't a corpse.\n\r", ch);

    return;
  }

  if (!IS_NPC(ch) && (OBJ_COST(corpse) == PC_CORPSE || OBJ_COST(corpse) == CHAOS_CORPSE)) {
    send_to_char("The thought of collecting a trophy from a fellow adventurer makes you cringe!\n\r", ch);

    return;
  }

  if (OBJ_COST(corpse) == PC_STATUE || OBJ_COST(corpse) == NPC_STATUE) {
    send_to_char("You find it impossible to chip away at the stone.\n\r", ch);

    return;
  }

  if (!CORPSE_HAS_TROPHY(corpse)) {
    send_to_char("That corpse has no suitable 'parts' for a trophy.\n\r", ch);

    return;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS2(corpse), ITEM_NO_TROPHY)) {
    send_to_char("A trophy has already been collected from the corpse.\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_TROPHY)) {
    send_to_char("You failed to harvest a trophy from the corpse.\n\r", ch);
  }
  else {
    CREATE(trophy, struct obj_data, 1);
    clear_object(trophy);
    trophy->item_number = NOWHERE;
    trophy->in_room = NOWHERE;
    trophy->name = str_dup("trophy");
    half_chop(corpse->description, buf, MIL, trophy_name, MSL);
    sprintf(buf, "A gruesome trophy %s", trophy_name);
    trophy->description = str_dup(buf);
    half_chop(corpse->short_description, buf, MIL, trophy_name, MSL);
    sprintf(buf, "A gruesome trophy %s", trophy_name);
    trophy->short_description = str_dup(buf);
    trophy->obj_flags.timer = 10;
    trophy->obj_flags.type_flag = ITEM_TROPHY;
    trophy->obj_flags.wear_flags = ITEM_TAKE + ITEM_WEAR_HEAD;
    trophy->obj_flags.extra_flags = ITEM_ANTI_RENT;
    trophy->obj_flags.extra_flags2 = ITEM_ALL_DECAY;
    trophy->obj_flags.material = corpse->obj_flags.material;
    trophy->obj_flags.value[0] = 0;
    trophy->obj_flags.value[1] = 0;
    trophy->obj_flags.value[2] = 0;
    trophy->obj_flags.value[3] = 0;
    trophy->obj_flags.weight = corpse->obj_flags.weight / 10;
    trophy->obj_flags.cost = corpse->obj_flags.cost_per_day;

    trophy->next = object_list;
    object_list = trophy;
    obj_to_room(trophy, CHAR_REAL_ROOM(ch));

    send_to_char("You harvest a trophy from the corpse.\n\r", ch);
    act("$n harvests a trophy from $p.", TRUE, ch, corpse, 0, TO_ROOM);

    SET_BIT(OBJ_EXTRA_FLAGS2(corpse), ITEM_NO_TROPHY);
  }
}


void do_clobber(CHAR *ch, char *arg, int cmd) {
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int num = 0;
  int set_pos = 0;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_CLOBBER)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch))) {
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

  check = number(1, 131) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_CLOBBER)) {
    act("Your attempt to clobber $N misses everything but the air.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n flails about wildly, missing you with $s erratic swinging.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n flails about wildly, missing $N with $s erratic swinging.", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_CLOBBER, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_CLOBBER, 2);

    return;
  }

  if (!breakthrough(ch, victim, BT_INVUL)) {
    act("You clobber $N, but $E doesn't seem to mind!", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n tries to clobber you, but it feels more like a massage!", FALSE, ch, NULL, victim, TO_VICT);
    act("$n clobbers $N, but $E doesn't seem to mind!", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_CLOBBER, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_CLOBBER, chance(15) ? 3 : 2);
  }
  else {
    num = number(1, 10);

    /* 20% chance of 2.50x damage on next hit. */
    if (num <= 2) {
      set_pos = POSITION_MORTALLYW;
    }
    /* 40% chance of 2.33x damage on next hit. */
    else if (num <= 6) {
      set_pos = POSITION_INCAP;
    }
    /* 40% chance of 2.00x damage on next hit. */
    else {
      set_pos = POSITION_STUNNED;
    }

    set_pos = stack_position(victim, set_pos);

    act("You clobber $N with windmilling fists!", FALSE, ch, NULL, victim, TO_CHAR);
    act("You're clobbered by $n's windmilling fists!", FALSE, ch, NULL, victim, TO_VICT);
    act("$n clobbers $N with $s windmilling fists!", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, calc_position_damage(GET_POS(victim), number(GET_LEVEL(ch), GET_LEVEL(ch) * 4)), SKILL_CLOBBER, DAM_PHYSICAL);

    if ((CHAR_REAL_ROOM(victim) != NOWHERE) && !IS_IMPLEMENTOR(victim)) {
      GET_POS(victim) = set_pos;
    }

    skill_wait(ch, SKILL_CLOBBER, chance(15) ? 3 : 2);
  }
}


void do_victimize(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_VICTIMIZE)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_VICTIMIZE)) {
    affect_from_char(ch, SKILL_VICTIMIZE);

    send_to_char("You decide to show a modicum of mercy to your victims.\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_VICTIMIZE)) {
    send_to_char("You can't come up with any good taunts or threats.\n\r", ch);
  }
  else {
    af.type = SKILL_VICTIMIZE;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You will now take every opportunity to victimize your foe.\n\r", ch);
  }
}


void do_shadow_walk(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_SHADOW_WALK)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_SHADOW_WALK)) {
    affect_from_char(ch, SKILL_SHADOW_WALK);

    send_to_char("You will no longer slip into the shadows to attack your victims.\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_SHADOW_WALK)) {
    send_to_char("You can't seem to blend in with the shadows.\n\r", ch);
  }
  else {
    af.type = SKILL_SHADOW_WALK;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You will now slip into the shadows to attack your victims.\n\r", ch);
  }
}


void do_evasion(CHAR *ch, char *argument, int cmd)
{
  int check = 0;
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_EVASION))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch))
  {
    send_to_char("You can't focus well enough to do that while fighting.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_EVASION))
  {
    affect_from_char(ch, SKILL_EVASION);

    send_to_char("You become less evasive and set yourself into a normal fighting stance.\n\r", ch);
    act("$n becomes less evasive and sets $mself into a normal fighting stance.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  check = number(1, 121) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_EVASION)) {
    send_to_char("You failed to focus on evading attacks.\n\r", ch);
  }
  else
  {
    af.type = SKILL_EVASION;
    af.duration = -1;
    af.modifier = ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? -4 : -5;
    af.location = APPLY_HITROLL;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You focus on evading attacks and fighting defensively.\n\r", ch);
    act("$n focuses on evading attacks and fighting defensively.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void do_dirty_tricks(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_DIRTY_TRICKS)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_DIRTY_TRICKS)) {
    affect_from_char(ch, SKILL_DIRTY_TRICKS);

    send_to_char("You'll fight fair... for now.\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_DIRTY_TRICKS)) {
    send_to_char("You have trouble coming up with a strategy for fighting dirty.\n\r", ch);
  }
  else {
    af.type = SKILL_DIRTY_TRICKS;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You will start using dirty tricks to get the upper hand in combat.\n\r", ch);
  }
}


void do_trip(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_TRIP)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_TRIP)) {
    affect_from_char(ch, SKILL_TRIP);

    send_to_char("You will no longer try to trip your enemies during combat.\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_TRIP)) {
    send_to_char("You don't feel quite skilled enough to trip up your enemies.\n\r", ch);
  }
  else {
    af.type = SKILL_TRIP;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You will try to trip your enemies when you circle around them.\n\r", ch);
  }
}


void do_trusty_steed(CHAR *ch, char *argument, int cmd) {
  AFF af;

  if (!GET_SKILLS(ch)) return;

  if (!check_sc_access(ch, SKILL_TRUSTY_STEED)) {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_TRUSTY_STEED)) {
    affect_from_char(ch, SKILL_TRUSTY_STEED);

    send_to_char("You will no longer summon your trusty steed to aid you in battle.\n\r", ch);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_TRUSTY_STEED)) {
    send_to_char("You fail to establish a spirit link with your trusty steed in the astral plane.\n\r", ch);
  }
  else {
    af.type = SKILL_TRUSTY_STEED;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_NONE;
    af.bitvector2 = AFF_NONE;

    affect_to_char(ch, &af);

    send_to_char("You will summon your trusty steed when pummeling or smiting your foes.\n\r", ch);
  }
}
