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

/*
$Author: ronin $
$Date: 2005/01/21 14:55:30 $
$Header: /home/ronin/cvs/ronin/subclass.skills.c,v 2.4 2005/01/21 14:55:30 ronin Exp $
$Id: subclass.skills.c,v 2.4 2005/01/21 14:55:30 ronin Exp $
$Name:  $
$Log: subclass.skills.c,v $
Revision 2.4  2005/01/21 14:55:30  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.3  2004/11/16 05:05:03  ronin
Chaos 2004 Update.

Revision 2.2  2004/10/11 19:25:41  piggy
Changed Fade hitroll penalty to be based on level, and remove AC bonus.

Revision 2.1  2004/05/02 13:12:45  ronin
Fix to stop assassinate from jumping over blocking mobs.

Revision 2.0.0.1  2004/02/05 16:11:51  ronin
Reinitialization of cvs archives


Revision 6-Nov-03 Ranger
Added disarm log to impair.

Revision - 17-Oct-03 Liner
mantra - check for degenerate before healing.
-adding checks for chaosmode to stop healing things.

Revision - 03-Dec-02 Ranger
switch - increased chance of a furied Pa to switch
charge - fixed problem if successful charge lead, rest of group will charge

Revision - 27-Nov-02 Ranger
  charge: can't charge if mob is already fighting, fix to have leader charge,
          some message changes

Revision 1.6  2002/07/24 17:55:01  ronin
Small fix to trip to check if victim is already affected by trip.
Fix to reorder the logic of some checks in some skills so that
messages received by the play makes more sense.
Additional messages added to the protect skill.

Revision 1.5  2002/06/18 14:32:20  ronin
Adding divide_experience before raw_kill to ensure proper quest
completion.  Addition of flag within divide_experience to force
amount to 0 if required.

Revision 1.4  2002/04/16 18:00:34  ronin
Addition of IMMUNE_EXECUTE

Revision 1.3  2002/03/31 07:54:09  ronin
Fix of sweep skill affecting PCs in the room.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

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
extern struct wis_app_type wis_app[26];
extern struct obj_data  *object_list;

int calc_position_damage(int position, int dam);
int stack_position(CHAR *ch, int target_position);
void do_move(struct char_data *ch, char *argument, int cmd);
void skill_wait(CHAR *ch, int skill, int wait);


void do_meditate(CHAR *ch, char *argument, int cmd)
{
  AFF af;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_MEDITATE))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch))
  {
    send_to_char("You cannot meditate while fighting!\n\r", ch);

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
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);

    send_to_char("You gaze inward and focus on healing.\n\r", ch);
    act("$n enters a deep trance.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void do_protect(CHAR *ch, char *argument, int cmd)
{
  char buf[MIL];
  CHAR *victim = NULL;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_PROTECT))
  {
    send_to_char("You do not have this skill.\n\r", ch);

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
    send_to_char("Protect a mob? Impossible!\n\r", ch);

    return;
  }

  if (ch == victim)
  {
    send_to_char("You just protect yourself.\n\r", ch);
    act("$n just protects $mself.", FALSE, ch, 0, 0, TO_NOTVICT);

    if (ch->specials.protecting)
    {
      act("$N stops protecting you.", FALSE, ch->specials.protecting, 0, ch, TO_CHAR);

      ch->specials.protecting->specials.protect_by = 0;
    }

    ch->specials.protecting = 0;

    return;
  }

  if (affected_by_spell(ch, SKILL_BERSERK))
  {
    send_to_char("You can't protect anyone while berserked.\n\r", ch);

    return;
  }

  if (victim->specials.protecting)
  {
    act("$N is already protecting someone.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if (ch->specials.protecting)
  {
    act("You stop protecting $N.", FALSE, ch, 0, ch->specials.protecting, TO_CHAR);
    act("$n stops protecting you.", FALSE, ch, 0, ch->specials.protecting, TO_VICT);

    ch->specials.protecting->specials.protect_by = 0;
    ch->specials.protecting = 0;
  }

  if (number(1, 131) > GET_LEARNED(ch, SKILL_PROTECT) || CHAOSMODE)
  {
    act("You failed to start protecting $N.", FALSE, ch, 0, victim, TO_CHAR);

    skill_wait(ch, SKILL_PROTECT, 1);
  }
  else
  {
    act("You start trying to protect $N.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n starts trying to protect you.", FALSE, ch, 0, victim, TO_VICT);
    act("$n starts trying to protect $N.", FALSE, ch, 0, victim, TO_NOTVICT);

    ch->specials.protecting = victim;
    victim->specials.protect_by = ch;

    skill_wait(ch, SKILL_PROTECT, 1);
  }
}


void do_retreat(struct char_data *ch, char *argument, int cmd) {
  int i, attempt, die;
  struct char_data *victim = NULL;
  int org_room;
  bool rider, mount;

  int special(struct char_data *ch, int cmd, char *arg);

  if (!ch->skills) return;
  if (!check_sc_access(ch, SKILL_RETREAT)) {
    send_to_char("You do not have this skill.\n\r", ch);
    return;
  }

  rider = FALSE;
  mount = FALSE;

  if (IS_AFFECTED(ch, AFF_PARALYSIS) || IS_AFFECTED(ch, AFF_HOLD)) {
    act("$n tries to retreat but seems to be unable to move.", TRUE, ch, 0, 0, TO_ROOM);
    act("You try to retreat but are paralyzed and can't move!", TRUE, ch, 0, 0, TO_CHAR);
    return;
  }

  attempt = number(1, 86); /* shouldn't fail if practiced */
  if (attempt > ch->skills[SKILL_RETREAT].learned) {
    act("You try to rapidly retreat but fail.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n tries to rapidly retreat but fails.", FALSE, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }

  if (!(ch->specials.fighting)) {
    for (i = 0; i < 10; i++) {
      attempt = number(0, 5);  /* Select a random direction */
      if (CAN_GO(ch, attempt) &&
          !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, DEATH)) {
        act("$n attempts to retreat.", TRUE, ch, 0, 0, TO_ROOM);
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
          send_to_char("You retreat as fast as you are able.\n\r", ch);
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
          if (!die) act("$n tries to retreat, but is too exhausted!",
                        TRUE, ch, 0, 0, TO_ROOM);
          return;
        }
      }
    } /* for */
    /* No exits was found */
    send_to_char("ACK! There isn't anywhere to retreat to!\n\r", ch);
    return;
  }

  /* FIGHTING */
  for (i = 0; i < 10; i++) {
    attempt = number(0, 5);
    if (CAN_GO(ch, attempt) &&
        !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, DEATH)) {
      if (!IS_NPC(ch->specials.fighting) && !ch->specials.rider &&
          affected_by_spell(ch->specials.fighting, SKILL_BLOCK) &&
          ((number(1, 130) < ch->specials.fighting->skills[SKILL_BLOCK].learned) ||
          (check_subclass(ch->specials.fighting, SC_WARLORD, 1) && chance(80)))) {
        act("$N tried to retreat but $n blocked $S way!",
            FALSE, ch->specials.fighting, 0, ch, TO_NOTVICT);
        act("You tried to retreat but $N blocked your way!",
            FALSE, ch, 0, ch->specials.fighting, TO_CHAR);
        act("$N tried to retreat but you blocked $S way!",
            FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
        return;
      }
      act("$n attempts to retreat.", TRUE, ch, 0, 0, TO_ROOM);
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
        if (ch->specials.fighting) { /* Another check */

        /* Insert later when using huntig system        */
        /* ch->specials.fighting->specials.hunting = ch */

          if (ch->specials.fighting->specials.fighting == ch)
            stop_fighting(ch->specials.fighting);
          stop_fighting(ch);
        }
        send_to_char("You retreat as fast as you are able.\n\r", ch);

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
        if (!die) act("$n tries to retreat, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
        return;
      }
    }
  } /* for */
  /* No exits was found */
  send_to_char("ACK! There isn't anywhere to retreat to!\n\r", ch);
}


void do_backfist(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_BACKFIST))
  {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch)))
  {
    send_to_char("Backfist who?\n\r", ch);

    return;
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  check = number(1, 121) - GET_DEX_APP(ch);

  if (affected_by_spell(ch, SPELL_BLUR))
  {
    check -= (GET_LEVEL(ch) / 10);
  }

  if (check > GET_LEARNED(ch, SKILL_BACKFIST))
  {
    act("You try to backfist $N but fail.", 0, ch, 0, victim, TO_CHAR);
    act("$n tries to backfist $N but fails.", 0, ch, 0, victim, TO_NOTVICT);
    act("$n tries to backfist you but fails.", 0, ch, 0, victim, TO_VICT);

    damage(ch, victim, 0, SKILL_BACKFIST, DAM_SKILL);

    skill_wait(ch, SKILL_BACKFIST, 2);
  }
  else
  {
    if (AWAKE(victim) &&
        ((IS_NPC(victim) && IS_SET(victim->specials.immune2, SKILL_BACKFIST)) ||
        (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))))
    {
      act("You backfist $N, but your backfist has no effect!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n backfists you, but $s backfist has no effect!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n backfists $N, but $s backfist has no effect!", FALSE, ch, NULL, victim, TO_NOTVICT);

      damage(ch, victim, 0, SKILL_BACKFIST, DAM_NO_BLOCK);

      skill_wait(ch, SKILL_BACKFIST, 2);
    }
    else
    {
      set_pos = stack_position(victim, POSITION_STUNNED);

      act("With a sickening crunch you hit $N with a huge backfist.", 0, ch, 0, victim, TO_CHAR);
      act("With a sickening crunch $n hits $N with a huge backfist.", 0, ch, 0, victim, TO_NOTVICT);
      act("With a sickening crunch $n hits you with a huge backfist.", 0, ch, 0, victim, TO_VICT);

      damage(ch, victim, calc_position_damage(GET_POS(victim), (GET_LEVEL(ch) * 5) / 4), SKILL_BACKFIST, DAM_SKILL);

      if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim))
      {
        GET_POS(victim) = set_pos;

        if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim))
        {
          GET_POS(victim) = set_pos;

          /* Can't use skill_wait() since this applies to victim. */
          WAIT_STATE(victim, PULSE_VIOLENCE * CHAOSMODE ? number(1, 2) : 2);
        }
      }

      skill_wait(ch, SKILL_BACKFIST, 2);
    }
  }
}


void do_pray(CHAR *ch, char *argument, int cmd)
{
  AFF af;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_PRAY))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch))
  {
    send_to_char("You would do better to swear than to pray right now.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_PRAY))
  {
    send_to_char("You're already praying.\n\r", ch);

    return;
  }

  if (number(0, 85) > GET_LEARNED(ch, SKILL_PRAY))
  {
    send_to_char("You failed to focus your thoughts in prayer.\n\r", ch);
  }
  else
  {
    af.type = SKILL_PRAY;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);

    send_to_char("You bow your head and begin your prayer.\n\r", ch);
    act("$n bows $s head and begins praying.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


//void do_evade(CHAR *ch, char *argument, int cmd) {
//  int recover=70;                          /* Linerfix */
//
//  if(!ch->skills) return;
//  if(!check_sc_access(ch,SKILL_EVADE)) {
//    send_to_char("You do not have this skill.\n\r",ch);
//    return;
//  }
//
//  if(!ch->specials.fighting) {
//    send_to_char("You aren't fighting anyone.\n\r",ch);
//    return;
//  }
//
//  if(number(0,121)>ch->skills[SKILL_EVADE].learned) {
//    act("You failed to evade $N.",0,ch,0,ch->specials.fighting,TO_CHAR);
//    WAIT_STATE(ch, PULSE_VIOLENCE);
//    return;
//  }
//
//  act("You quickly evade $N, and slip out of combat with $M!",0,ch,0,ch->specials.fighting,TO_CHAR);
//  act("$n quickly evades you and slips out of combat.",0,ch,0,ch->specials.fighting,TO_VICT);
//  act("$n quickly evades $N and slips out of combat with $M!",0,ch,0,ch->specials.fighting,TO_NOTVICT);
//  if(ch->specials.fighting->specials.fighting==ch)
//    stop_fighting(ch->specials.fighting);
//  stop_fighting(ch);
//  recover=number(50,75);                               /* Linerfix */
//  if(check_subclass(ch,SC_BANDIT,4) && chance(recover)  && (!CHAOSMODE)) {
//    WAIT_STATE(ch, PULSE_VIOLENCE*2);
//  }
//  else {
//  if(CHAOSMODE)
//    WAIT_STATE(ch, PULSE_VIOLENCE*1);
//  else
//    WAIT_STATE(ch, PULSE_VIOLENCE*3);
//  }
//}

//void do_trip(struct char_data *ch, char *argument, int cmd) {
//  struct char_data *victim;
//  struct affected_type_5 af;
//  char name[256];
//  int percent;
//  int recover=70;                          /* Linerfix */
//
//  if(!ch->skills) return;
//  if(!check_sc_access(ch,SKILL_TRIP)) {
//    send_to_char("You do not have this skill.\n\r",ch);
//    return;
//  }
//
//  one_argument(argument, name);
//
//  if (!(victim = get_char_room_vis(ch, name))) {
//   if (ch->specials.fighting) {
//     victim = ch->specials.fighting;
//   } else {
//     send_to_char("Trip who?\n\r", ch);
//     return;
//   }
//  }
//
//  if ((GET_LEVEL(victim) > LEVEL_MORT) && (!IS_NPC(victim))) {
//    send_to_char("It is not a good idea to attack an Immortal!\n\r", ch);
//    return;
//  }
//
//  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE)&& (!CHAOSMODE)) {
//    send_to_char("Behave yourself here please!\n\r", ch);
//    return;
//  }
//
//  if (victim == ch) {
//    send_to_char("Aren't we funny today...\n\r", ch);
//    return;
//  }
//
//  percent=number(1,131)-GET_DEX_APP(ch); /* 101% is a complete failure */
//
//  if (percent > ch->skills[SKILL_TRIP].learned ||
//      (GET_LEVEL(ch)<GET_LEVEL(victim)-5) ||
//      (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch,victim,BT_INVUL)) ){
//    act("$n tries to trip $N but fails.",0,ch,0,victim,TO_NOTVICT);
//    act("$n tries to trip you but fails.",0,ch,0,victim,TO_VICT);
//    act("You try to trip $N but fail.",0,ch,0,victim,TO_CHAR);
//    damage(ch, victim, 0, SKILL_TRIP,DAM_NO_BLOCK);
//    if(!ch->specials.riding) GET_POS(ch) = POSITION_SITTING;
//    recover=number(50,75);                               /* Linerfix */
//    if(check_subclass(ch,SC_BANDIT,4) && chance(recover)) {
//      WAIT_STATE(ch, PULSE_VIOLENCE*2);
//    }
//    else {
//    WAIT_STATE(ch, PULSE_VIOLENCE*3);
//    }
//
//    return;
//  }
//
//  act("$n sticks out $s foot and trips $N.",0,ch,0,victim,TO_NOTVICT);
//  act("$n sticks out $s foot and trips you.",0,ch,0,victim,TO_VICT);
//  act("You stick out your foot and trip $N.",0,ch,0,victim,TO_CHAR);
//
//  if(!affected_by_spell(victim, SKILL_TRIP)) {
//    af.type      = SKILL_TRIP;
//    af.duration  = 0;
//    af.modifier  = 0;
//    af.location  = APPLY_NONE;
//    af.bitvector = 0;
//    af.bitvector2 = 0;
//    affect_to_char(victim, &af);
//  }
//  damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_TRIP,DAM_NO_BLOCK);
//  GET_POS(victim)=POSITION_STUNNED;
//  recover=number(50,75);                               /* Linerfix */
//  if(check_subclass(ch,SC_BANDIT,4) && chance(recover)) {
//    WAIT_STATE(ch, PULSE_VIOLENCE*2);
//  }
//  else {
//  WAIT_STATE(ch, PULSE_VIOLENCE*3);
//  }
//}


void do_tigerkick(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_TIGERKICK))
  {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch)))
  {
    send_to_char("Tigerkick who?\n\r", ch);

    return;
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);

    return;
  }
  else if (IS_MORTAL(ch) && IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }
  else if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  check = number(1, 111) - GET_DEX_APP(ch);

  if (affected_by_spell(ch, SPELL_BLUR))
  {
    check -= (GET_LEVEL(ch) / 10);
  }

  if (check > GET_LEARNED(ch, SKILL_TIGERKICK))
  {
    act("You try to tigerkick $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to tigerkick you, but misses.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to tigerkick $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_TIGERKICK, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_TIGERKICK, 2);
  }
  else
  {
    set_pos = stack_position(victim, POSITION_RESTING);

    act("You drive your foot into $N's chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n drives $s foot into your chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_VICT);
    act("$n drives $s foot into $N's chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, calc_position_damage(GET_POS(victim), (GET_LEVEL(ch) * 7) / 4), SKILL_TIGERKICK, DAM_NO_BLOCK);

    if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim))
    {
      GET_POS(victim) = set_pos;

      if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim))
      {
        GET_POS(victim) = set_pos;

        /* Can't use skill_wait() since this applies to victim. */
        WAIT_STATE(victim, PULSE_VIOLENCE * CHAOSMODE ? number(1, 2) : 2);
      }
    }

    skill_wait(ch, SKILL_TIGERKICK, 2);
  }
}


/* Portions of scan taken from a snippet (do_scan) by sjmaster@mit.edu */
void list_scanned_chars(CHAR *list, CHAR *ch, int distance, int door)
{
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

  for (i = list; i; i = i->next_in_room)
  {
    if (CAN_SEE(ch, i)) count++;
  }

  if (!count) return;

  buf[0] = '\0';
  for (i = list; i; i = i->next_in_room)
  {
    if (!CAN_SEE(ch, i)) continue;

    count--;

    if (!*buf)
      sprintf(buf, "You see %s", (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)));
    else
      sprintf(buf, "%s%s", buf, (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)));
    if (count > 1)
      strcat(buf, ", ");
    else if (count == 1)
      strcat(buf, " and ");
    else
    {
      sprintf(buf2, " %s %s.\n\r", how_far[distance], dir_name[door]);
      strcat(buf, buf2);
    }

  }

  send_to_char(buf, ch);
}


/* Portions of scan taken from a snippet (do_scan) by sjmaster@mit.edu */
void do_scan(CHAR *ch, char *argument, int cmd)
{
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

  if (!ch->skills) return;

  if (GET_CLASS(ch) != CLASS_THIEF && IS_MORTAL(ch))
  {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (GET_LEVEL(ch) < 40)
  {
    send_to_char("You are not high enough level to use that skill.\n\r", ch);

    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND))
  {
    send_to_char("You can't see a thing, you're blind!\n\r", ch);

    return;
  }

  send_to_char("You quickly scan the area.\n\r", ch);

  for (dir = 0; dir < 6; dir++)
  {
    room = CHAR_REAL_ROOM(ch);

    if (!CAN_GO(ch, dir)) continue;

    if (number(0, 121) > GET_LEARNED(ch, SKILL_SCAN))
    {
      printf_to_char(ch, "You failed to scan %s.\n\r", dir_name[dir]);

      continue;
    }

    for (distance = 0; distance < 3; distance++)
    {
      if (!world[room].dir_option[dir]) break;

      exit_room = world[room].dir_option[dir]->to_room_r;

      if (exit_room == NOWHERE || exit_room == real_room(0) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_CLOSED) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_CRAWL) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_ENTER) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_JUMP) ||
          IS_SET(world[room].dir_option[dir]->exit_info, EX_CLIMB))
      {
        break;
      }

      if (world[exit_room].people)
      {
        list_scanned_chars(world[exit_room].people, ch, distance, dir);
      }

      room = exit_room;
    }
  }
}


void do_camp(struct char_data *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_CAMP)) {
    send_to_char("You do not have this skill.\n\r", ch);
    return;
  }
  if (affected_by_spell(ch, SKILL_CAMP) || IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CLUB)) {
    send_to_char("There is already a camp here.\n\r", ch);
    return;
  }
  if (ch->specials.riding) {
    send_to_char("You must dismount first.\n\r", ch);
    return;
  }

  if (number(0, 130) > ch->skills[SKILL_CAMP].learned) {
    send_to_char("You failed to set up camp.\n\r", ch);
    return;
  }

  if (count_mobs_real_room_except_followers(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't camp here, there is at least one mob!\n\r", ch);
    return;
  }

  af.type = SKILL_CAMP;
  af.duration = -1;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  af.bitvector2 = 0;
  affect_to_char(ch, &af);
  GET_POS(ch) = POSITION_RESTING;
  SET_BIT(world[CHAR_REAL_ROOM(ch)].room_flags, CLUB);
  act("You quickly set up a camp here, then sit down and rest.", 1, ch, 0, 0, TO_CHAR);
  act("$n quickly sets up a camp here, then sits down and rests.", 1, ch, 0, 0, TO_ROOM);
}


void do_blitz(CHAR *ch, char *argument, int cmd)
{
  CHAR* victim = NULL;
  char name[MIL];
  int check = 0;

  if (!ch->skills) return;

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

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);

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


void do_lunge(CHAR *ch, char *argument, int cmd)
{
  CHAR *victim = NULL;
  char name[MIL];
  int check = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_LUNGE))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch))
  {
    send_to_char("You need to wield a weapon for it to be successful.\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch)))
  {
    send_to_char("Lunge at who?\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);

    return;
  }

  check = number(1, 131) - GET_DEX_APP(ch);

  /* Bonus for lunging at a target already in combat. */
  if (GET_OPPONENT(victim))
  {
    check -= 5;
  }

  if (check > GET_LEARNED(ch, SKILL_LUNGE))
  {
    act("You try to lunge at $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to lunge at you, but fails.", FALSE, victim, 0, ch, TO_CHAR);
    act("$n tries to lunge at $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_LUNGE, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_LUNGE, 2);
  }
  else
  {
    hit(ch, victim, SKILL_LUNGE);

    skill_wait(ch, SKILL_LUNGE, 2);
  }
}


void do_fade(CHAR *ch, char *argument, int cmd)
{
  int modifier = 0;
  AFF af;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_FADE))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_MOUNT(ch))
  {
    send_to_char("You must dismount first.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_FADE))
  {
    send_to_char("You emerge from the background and into view.\n\r", ch);

    affect_from_char(ch, SKILL_FADE);

    return;
  }

  if (number(0, 85) > GET_LEARNED(ch, SKILL_FADE))
  {
    send_to_char("You failed to fade into the background.\n\r", ch);
  }
  else
  {
    if (GET_LEVEL(ch) < 35)
    {
      modifier = -5;
    }
    else if (GET_LEVEL(ch) < 40)
    {
      modifier = -4;
    }
    else if (GET_LEVEL(ch) < 45)
    {
      modifier = -3;
    }
    else if (GET_LEVEL(ch) < 50)
    {
      modifier = -2;
    }
    else
    {
      modifier = -1;
    }

    af.type = SKILL_FADE;
    af.duration = -1;
    af.bitvector = 0;
    af.bitvector2 = 0;
    af.modifier = modifier;
    af.location = APPLY_HITROLL;
    affect_to_char(ch, &af);

    send_to_char("You quietly fade into the background and blend in with your surroundings.\n\r", ch);
    act("$n quietly fades into the background and blends in with $s surroundings.", TRUE, ch, 0, 0, TO_ROOM);

    skill_wait(ch, SKILL_FADE, 1);
  }
}


void do_vehemence(CHAR *ch, char *argument, int cmd)
{
  AFF af;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_VEHEMENCE))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch))
  {
    send_to_char("You can't shake off your feeling of vehemence while in combat.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_VEHEMENCE))
  {
    send_to_char("Your vehemence diminishes.\n\r", ch);

    affect_from_char(ch, SKILL_VEHEMENCE);

    return;
  }

  if (number(0, 85) > GET_LEARNED(ch, SKILL_VEHEMENCE))
  {
    send_to_char("You are unable to fill yourself with a sense of vehemence.\n\r", ch);
  }
  else
  {
    af.type = SKILL_VEHEMENCE;
    af.duration = -1;
    af.bitvector = 0;
    af.bitvector2 = 0;
    af.modifier = 0;
    af.location = 0;
    affect_to_char(ch, &af);

    send_to_char("You feel a sudden rush of vehemence wash over you.\n\r", ch);
  }
}


void do_switch(CHAR *ch, char *argument, int cmd)
{
  CHAR *victim = NULL;
  char name[MIL];
  int check = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_SWITCH))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (!GET_OPPONENT(ch))
  {
    send_to_char("You aren't fighting anything.\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name)))
  {
    send_to_char("Switch to who?\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);

    return;
  }

  check = number(1, 111) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_SWITCH) ||
      (IS_AFFECTED(ch, AFF_FURY) && GET_CLASS(ch) == CLASS_WARRIOR) ||
      (IS_AFFECTED(ch, AFF_FURY) && GET_CLASS(ch) == CLASS_PALADIN && chance(50)))
  {
    send_to_char("You failed to switch.\n\r", ch);

    skill_wait(ch, SKILL_SWITCH, 2);
  }
  else
  {
    act("$n switches $s fight to $N", 0, ch, 0, victim, TO_NOTVICT);
    act("You switch your fight to $N!", 0, ch, 0, victim, TO_CHAR);
    act("$n switches $s fight to you!", 0, ch, 0, victim, TO_VICT);

    stop_fighting(ch);
    set_fighting(ch, victim);

    skill_wait(ch, SKILL_SWITCH, 2);
  }
}


void do_flank(CHAR *ch, char *argument, int cmd)
{
  CHAR *victim = NULL;
  char name[MIL];
  int check = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_FLANK))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name)))
  {
    send_to_char("Flank who?\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch))
  {
    send_to_char("You need to wield a weapon for it to be successful.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);

    return;
  }

  check = number(1, 131) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_FLANK))
  {
    act("You try to flank $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to flank you, but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to flank $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_FLANK, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_FLANK, 2);
  }
  else
  {
    hit(ch, victim, SKILL_FLANK);

    skill_wait(ch, SKILL_FLANK, 2);
  }
}


void do_sweep(CHAR *ch, char *argument, int cmd)
{
  CHAR *temp = NULL;
  CHAR *tmp_victim = NULL;
  char name[MIL];
  int check = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_SWEEP))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (!GET_WEAPON(ch))
  {
    send_to_char("You need to wield a weapon for it to be successful.\n\r", ch);

    return;
  }

  one_argument(argument, name);

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  check = number(1, 131) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_SWEEP))
  {
    act("You try to sweep around the room with your weapon, but fail.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n tries to sweep around the room with $s weapon, but fail.", FALSE, ch, 0, 0, TO_ROOM);

    skill_wait(ch, SKILL_SWEEP, 3);

    return;
  }
  else
  {
    act("You deftly sweep with your weapon.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n deftly sweeps with $s weapon.", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp)
    {
      temp = tmp_victim->next_in_room;

      if (tmp_victim == ch) continue;

      if (IS_NPC(tmp_victim) && GET_RIDER(tmp_victim) != ch)
      {
        hit(ch, tmp_victim, TYPE_UNDEFINED);

        skill_wait(tmp_victim, SKILL_SWEEP, 2);
      }
      else if (IS_MORTAL(tmp_victim) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && !GET_OPPONENT(tmp_victim))
      {
        hit(ch, tmp_victim, TYPE_UNDEFINED);
      }
    }

    skill_wait(ch, SKILL_SWEEP, 4);
  }
}


void do_hostile(CHAR *ch, char *argument, int cmd)
{
  int check = 0;
  AFF af;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_HOSTILE))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch))
  {
    send_to_char("You can't change your stance while fighting.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_HOSTILE))
  {
    affect_from_char(ch, SKILL_HOSTILE);

    send_to_char("You place yourself in a normal stance.\n\r", ch);
    act("$n places $mself in a normal stance.", TRUE, ch, 0, 0, TO_ROOM);

    skill_wait(ch, SKILL_HOSTILE, 1);

    return;
  }

  check = number(1, 121) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_HOSTILE))
  {
    send_to_char("You failed to get into the hostile stance.\n\r", ch);

    skill_wait(ch, SKILL_HOSTILE, 1);
  }
  else
  {
    af.type = SKILL_HOSTILE;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);

    send_to_char("You place yourself in a hostile stance.", ch);
    act("$n places $mself in a hostile stance.", TRUE, ch, 0, 0, TO_ROOM);

    skill_wait(ch, SKILL_HOSTILE, 1);
  }
}


void do_defend(CHAR *ch, char *argument, int cmd)
{
  int check = 0;
  AFF af;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_DEFEND))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (GET_OPPONENT(ch))
  {
    send_to_char("You can't change your stance while fighting.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_DEFEND))
  {
    affect_from_char(ch, SKILL_DEFEND);

    send_to_char("You place yourself in a normal stance.\n\r", ch);
    act("$n places $mself in a normal stance.", TRUE, ch, 0, 0, TO_ROOM);

    skill_wait(ch, SKILL_DEFEND, 1);

    return;
  }

  check = number(1, 121) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_DEFEND))
  {
    send_to_char("You failed to get into the defensive stance.\n\r", ch);

    skill_wait(ch, SKILL_DEFEND, 1);
  }
  else
  {
    af.type = SKILL_DEFEND;
    af.duration = -1;
    af.modifier = -100;
    af.location = APPLY_AC;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);

    send_to_char("You place yourself in a defensive stance.", ch);
    act("$n places $mself in a defensive stance.", TRUE, ch, 0, 0, TO_ROOM);

    skill_wait(ch, SKILL_DEFEND, 1);
  }
}


void do_assassinate(CHAR *ch, char *argument, int cmd)
{
  char name[MIL];
  char buf[MIL];
  int dir = NOWHERE;
  int room = NOWHERE;
  int check = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_ASSASSINATE))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  argument = one_argument(argument, name);

  if (!*name)
  {
    send_to_char("Assassinate who?\n\r", ch);

    return;
  }

  one_argument(argument, buf);

  if (!*buf)
  {
    send_to_char("What direction?\n\r", ch);

    return;
  }

  if (is_abbrev(buf, "north")) dir = CMD_NORTH;
  else if (is_abbrev(buf, "east")) dir = CMD_EAST;
  else if (is_abbrev(buf, "south")) dir = CMD_SOUTH;
  else if (is_abbrev(buf, "west")) dir = CMD_WEST;
  else if (is_abbrev(buf, "up")) dir = CMD_UP;
  else if (is_abbrev(buf, "down")) dir = CMD_DOWN;

  if (dir == NOWHERE)
  {
    send_to_char("What direction!?\n\r", ch);

    return;
  }

  if (GET_MOUNT(ch))
  {
    send_to_char("You must dismount first.\n\r", ch);

    return;
  }

  if (!IS_AFFECTED(ch, AFF_SNEAK))
  {
    send_to_char("You need to be sneaking.\n\r", ch);

    return;
  }

  check = number(1, 111) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_ASSASSINATE))
  {
    send_to_char("You fail your assassination attempt.\n\r", ch);

    skill_wait(ch, SKILL_ASSASSINATE, 2);

    return;
  }

  room = CHAR_REAL_ROOM(ch);

  do_move(ch, "", dir);

  if (room != CHAR_REAL_ROOM(ch))
  {
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

      damage(ch, victim, calc_position_damage(GET_POS(victim), GET_LEVEL(ch) * 2), SKILL_BATTER, DAM_NO_BLOCK);

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


void do_headbutt(CHAR *ch, char *arg, int cmd)
{
  CHAR *victim = NULL;
  char name[MIL];
  int check = 0;
  int set_pos = 0;
  int wait = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_HEADBUTT))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch)))
  {
    send_to_char("Headbutt who?\n\r", ch);

    return;
  }

  if (victim == ch)
  {
    send_to_char("This could prove very interesting...\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  check = number(1, 131) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_HEADBUTT))
  {
    act("You miss $N with your headbutt.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n misses you with $s headbutt.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n misses $N with $s headbutt.", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_HEADBUTT, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_HEADBUTT, 2);
  }
  else
  {
    if (AWAKE(victim) && IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
    {
      act("You slam your forehead into $N's face, but $E isn't even phased.", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n slams his forehead into your face, but you aren't even phased.", FALSE, ch, NULL, victim, TO_VICT);
      act("$n slams $s forehead into $N's face, but $E isn't even phased.", FALSE, ch, NULL, victim, TO_NOTVICT);

      damage(ch, victim, 0, SKILL_HEADBUTT, DAM_NO_BLOCK);
    }
    else
    {
      set_pos = stack_position(victim, POSITION_STUNNED);

      act("You slam your forehead into $N's face!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n slams his forehead into your face!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n slams $s forehead into $N's face!", FALSE, ch, NULL, victim, TO_NOTVICT);

      damage(ch, victim, calc_position_damage(GET_POS(victim), 200), SKILL_HEADBUTT, DAM_NO_BLOCK);

      if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim))
      {
        GET_POS(victim) = set_pos;
      }
    }

    wait = number(1, 8);
    wait = ((wait == 1) ? 1 : ((wait == 2 || wait == 3) ? 2 : 3));

    skill_wait(ch, SKILL_HEADBUTT, wait);
  }
}


void do_banzai(CHAR *ch, char *arg, int cmd)
{
  CHAR *victim = NULL;
  char name[MIL];
  int check = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_BANZAI))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch)))
  {
    send_to_char("Banzai charge who?\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && GET_MANA(ch) < 75)
  {
    send_to_char("You do not have enough mana.\n\r", ch);

    return;
  }

  check = number(1, 101) - GET_DEX_APP(ch);

  if (AWAKE(victim) && check > GET_LEARNED(ch, SKILL_BANZAI))
  {
    act("You try to banzai charge $N but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to banzai charge you but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to banzai charge $N but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_BANZAI, DAM_NO_BLOCK);

    GET_MANA(ch) = MAX(GET_MANA(ch) - 37, 0);

    skill_wait(ch, SKILL_BANZAI, 2);
  }
  else
  {
    act("With a primal yell, you banzai charge $N.", FALSE, ch, 0, victim, TO_CHAR);
    act("With a primal yell, $n banzai charges you.", FALSE, ch, 0, victim, TO_VICT);
    act("With a primal yell, $n banzai charges $N.", FALSE, ch, 0, victim, TO_NOTVICT);

    damage(ch, victim, number(200, 300), SKILL_BANZAI, DAM_SKILL);
    hit(ch, victim, SKILL_BANZAI);

    GET_MANA(ch) = MAX(GET_MANA(ch) - 75, 0);

    skill_wait(ch, SKILL_BANZAI, 2);
  }
}


void do_mantra(CHAR *ch, char *arg, int cmd)
{
  CHAR *victim = NULL;
  char name[MIL];
  int check = 0;
  int heal = 0;
  AFF *tmp_af = NULL;
  AFF *af_next = NULL;
  AFF af;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_MANTRA))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!*name)
  {
    victim = ch;
  }
  else if (!(victim = get_char_room_vis(ch, name)))
  {
    send_to_char("Chant your mantra to who?\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && GET_MANA(ch) < 120)
  {
    send_to_char("You do not have enough mana.\n\r", ch);

    return;
  }

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      (duration_of_spell(victim, SPELL_DEGENERATE) > 27 || (duration_of_spell(victim, SPELL_DEGENERATE) > 9 && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The power of the skill fails to heal your degenerated body.\n\r", victim);

    return;
  }

  if (CHAOSMODE && victim != ch)
  {
    send_to_char("You cannot perform this skill on another player during chaos.\n\r", ch);

    return;
  }

  check = number(1, 101) - GET_WIS_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_MANTRA))
  {
    if (victim != ch)
    {
      act("You chant your mantra to $N, but nothing happens.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n chants $s mantra to you, but nothing happens.", FALSE, ch, 0, victim, TO_VICT);
      act("$n chants $s mantra to $N, but nothing happens.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    else
    {
      send_to_char("You chant softly to yourself with no noticeable affect.\n\r", ch);
      act("$n chants softly to $mself with no noticeable affect.", FALSE, ch, 0, 0, TO_ROOM);
    }

    GET_MANA(ch) = MAX(GET_MANA(ch) - 60, 0);

    skill_wait(ch, SKILL_MANTRA, 2);
  }
  else
  {
    if (victim != ch)
    {
      act("You chant softly to $N, healing $S spirit and giving $M life.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n chants softly to you, healing your spirit and giving you life.", FALSE, ch, 0, victim, TO_VICT);
      act("$n chants softly to $N, healing $S spirit and giving $M life.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    else
    {
      send_to_char("You chant softly, healing your spirit and giving yourself life.\n\r", ch);
      act("$n chants softly, healing $s spirit and giving $mself life.", FALSE, ch, 0, 0, TO_ROOM);
    }

    GET_MANA(ch) = MAX(GET_MANA(ch) - 120, 0);
    //GET_HIT(victim) = MIN(GET_HIT(victim) + 500, GET_MAX_HIT(victim));
    magic_heal(victim, SKILL_MANTRA, 500, FALSE);

    heal = (GET_LEVEL(ch) + (5 * GET_WIS_APP(ch)));

    for (tmp_af = victim->affected; tmp_af; tmp_af = af_next)
    {
      af_next = tmp_af->next;

      if (tmp_af->type == SKILL_MANTRA && tmp_af->modifier >= heal)
      {
        affect_from_char(victim, SKILL_MANTRA);
      }
    }

    af.type = SKILL_MANTRA;
    af.duration = 10;
    af.modifier = heal;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = 0;

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


void do_trophy(CHAR *ch, char *argument, int cmd)
{
  OBJ *corpse = NULL;
  OBJ *trophy = NULL;
  char obj_name[MSL];
  char trophy_name[MSL];
  char buf[MIL];
  int check = 0;

  if (!check_sc_access(ch, SKILL_TROPHY))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  argument = one_argument(argument, obj_name);

  if (!(corpse = get_obj_in_list_vis(ch, obj_name, world[CHAR_REAL_ROOM(ch)].contents)))
  {
    send_to_char("Collect a trophy from what?\n\r", ch);

    return;
  }

  if (GET_ITEM_TYPE(corpse) != ITEM_CONTAINER && OBJ_VALUE3(corpse) != 1)
  {
    send_to_char("That isn't a corpse.\n\r", ch);

    return;
  }

  if (!IS_NPC(ch) && (OBJ_COST(corpse) == PC_CORPSE || OBJ_COST(corpse) == CHAOS_CORPSE))
  {
    send_to_char("The thought of collecting a trophy from a fellow adventurer makes you cringe!\n\r", ch);

    return;
  }

  if (OBJ_COST(corpse) == PC_STATUE || OBJ_COST(corpse) == NPC_STATUE)
  {
    send_to_char("You find it impossible to chip away at the stone.\n\r", ch);

    return;
  }

  if (!CORPSE_HAS_TROPHY(corpse))
  {
    send_to_char("That corpse has no suitable 'parts' for a trophy.\n\r", ch);

    return;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS2(corpse), ITEM_NO_TROPHY))
  {
    send_to_char("A trophy has already been collected from the corpse.\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && !GET_WEAPON(ch))
  {
    send_to_char("Not with your bare hands.\r\n", ch);

    return;
  }

  check = number(1, 85);

  if (!IS_NPC(ch) && check > GET_LEARNED(ch, SKILL_TROPHY))
  {
    send_to_char("You failed to harvest a trophy from the corpse.\n\r", ch);

    if (!IS_IMPLEMENTOR(ch))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE);
    }
  }
  else
  {
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

    SET_BIT(OBJ_EXTRA_FLAGS(corpse), ITEM_NO_TROPHY);
  }
}


/* Disabled
void do_throatstrike(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int percent,hp_percent;

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_THROATSTRIKE)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Throatstrike who?\n\r", ch);
     return;
   }
  }

  if (victim == ch) {
    send_to_char("Bet that would feel great..\n\r", ch);
    return;
  }

  if (ch->equipment[WIELD] && ch->equipment[HOLD]) {
    send_to_char("You need at least one free hand for it to succeed.\n\r",ch);
    return;
  }

  if(!IS_NPC(victim)) {
    send_to_char("You can't use this on other players.\n\r",ch);
    return;
  }

  percent=number(1,131)-GET_DEX_APP(ch);
  hp_percent=GET_HIT(victim)*100/GET_MAX_HIT(victim)+50;

  if ((percent > ch->skills[SKILL_THROATSTRIKE].learned) || chance(hp_percent) ||
      (IS_NPC(victim) && IS_SET(victim->specials.immune2,IMMUNE_THROATSTRIKE))) {
    act("You try to throatstrike $N, but miss by inches!", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to throatstrike you, but misses by inches!", FALSE, victim, 0, ch, TO_CHAR);
    act("$n tries to throatstrike $N, but missed by inches!", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$N avoids your pitiful attack and strikes back at you!", FALSE, ch, 0, victim, TO_CHAR);
    act("You avoid $N's pitiful attack and strike back at $M!", FALSE, victim, 0, ch, TO_CHAR);
    act("$N avoids $n's pitiful attack and strikes back at $m!", FALSE, ch, 0, victim, TO_NOTVICT);
    if(IS_NPC(victim) && IS_MORTAL(ch)) SET_BIT(victim->specials.immune2,IMMUNE_THROATSTRIKE);
    stop_fighting(ch);
    stop_fighting(victim);
    set_fighting(ch,victim);
    hit(victim, ch, TYPE_UNDEFINED);
    if(CHAR_REAL_ROOM(ch)!=NOWHERE) WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }

  act("Your fingers sink into $N's throat, killing $M instantly!", FALSE, ch, 0, victim, TO_CHAR);
  act("$N's fingers sink into your throat, killing you instantly!", FALSE, victim, 0, ch, TO_CHAR);
  act("$n's fingers sink into $N's throat, killing $M instantly!", FALSE, ch, 0, victim, TO_NOTVICT);
  divide_experience(ch,victim,1);
  raw_kill(victim);
  WAIT_STATE(ch, PULSE_VIOLENCE);
}
*/


//void do_execute(struct char_data *ch, char *argument, int cmd)
//{
//  struct char_data *victim;
//  char name[256];
//  int percent,hp_percent;
//
//  if(!ch->skills) return;
//  if(!check_sc_access(ch,SKILL_EXECUTE)) {
//    send_to_char("You do not have this skill.\n\r",ch);
//    return;
//  }
//
//  one_argument(argument, name);
//
//  if (!(victim = get_char_room_vis(ch, name))) {
//   if (ch->specials.fighting) {
//     victim = ch->specials.fighting;
//   } else {
//     send_to_char("Execute who?\n\r", ch);
//     return;
//   }
//  }
//
//  if (victim == ch) {
//    send_to_char("Bet that would feel great..\n\r", ch);
//    return;
//  }
//
//  if (!ch->equipment[WIELD]) {
//    send_to_char("You need to wield a weapon for it to succeed.\n\r",ch);
//    return;
//  }
//
//  if (ch->equipment[WIELD]->obj_flags.value[3] != 3) {
//    send_to_char("Only slashing weapons can be used to execute.\n\r",ch);
//    return;
//  }
//
//  if(!IS_NPC(victim)) {
//    send_to_char("You can't use this on other players.\n\r",ch);
//    return;
//  }
//
//  percent=number(1,131)-GET_DEX_APP(ch); /* 101% is a complete failure */
//  hp_percent=GET_HIT(victim)*100/GET_MAX_HIT(victim)+80;
//
//  if((percent > ch->skills[SKILL_EXECUTE].learned) || chance(hp_percent) ||
//     (IS_NPC(victim) && IS_SET(victim->specials.immune2,IMMUNE_EXECUTE))) {
//    act("You try to execute $N, but $E jumps back!", FALSE, ch, 0, victim, TO_CHAR);
//    act("$N tries to execute you, but you jump back!", FALSE, victim, 0, ch, TO_CHAR);
//    act("$n tries to execute $N, but $E jumps back!", FALSE, ch, 0, victim, TO_NOTVICT);
//    act("$N avoids your pitiful attack and strikes back at you!", FALSE, ch, 0, victim, TO_CHAR);
//    act("You avoid $N's pitiful attack and strike back at $M!", FALSE, victim, 0, ch, TO_CHAR);
//    act("$N avoids $n's pitiful attack and strikes back at $m!", FALSE, ch, 0, victim, TO_NOTVICT);
//    hit(victim, ch, TYPE_UNDEFINED);
//    if(CHAR_REAL_ROOM(ch)!=NOWHERE) WAIT_STATE(ch, PULSE_VIOLENCE*3);
//    return;
//  }
//
//  act("You scream 'THERE CAN BE ONLY ONE!' and cut $N's head off!", FALSE, ch, 0, victim, TO_CHAR);
//  act("$n screams 'THERE CAN BE ONLY ONE!' and cuts your head off!", FALSE, ch, 0, victim, TO_VICT);
//  act("$n screams 'THERE CAN BE ONLY ONE!' and cut $N's head off!", FALSE, ch, 0, victim, TO_NOTVICT);
//  damage(ch,victim,32000,TYPE_UNDEFINED,DAM_NO_BLOCK);
//  WAIT_STATE(ch, PULSE_VIOLENCE*3);
//}


void do_clobber(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int num = 0;
  int set_pos = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_CLOBBER))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  one_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name)) && !(victim = GET_OPPONENT(ch)))
  {
    send_to_char("Clobber who?\n\r", ch);

    return;
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);

    return;
  }

  if (IS_MORTAL(ch) && IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  check = number(1, 131) - GET_DEX_APP(ch);

  if (check > GET_LEARNED(ch, SKILL_CLOBBER))
  {
    act("Your attempt to clobber $N misses everything but the air.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n flails about wildly, missing you with $s erratic swinging.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n flails about wildly, missing $N with $s erratic swinging.", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_CLOBBER, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_CLOBBER, 2);

    return;
  }

  if (AWAKE(victim) && IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
  {
    act("You clobber $N, but $E doesn't seem to mind!", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n tries to clobber you, but it feels more like a massage!", FALSE, ch, NULL, victim, TO_VICT);
    act("$n clobbers $N, but $E doesn't seem to mind!", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, 0, SKILL_CLOBBER, DAM_NO_BLOCK);

    skill_wait(ch, SKILL_CLOBBER, chance(15) ? 3 : 2);
  }
  else
  {
    num = number(1, 10);

    if (num <= 2) /* 20% chance of 2.50x damage on next hit. */
    {
      set_pos = POSITION_MORTALLYW;
    }
    else if (num <= 6) /* 40% chance of 2.33x damage on next hit. */
    {
      set_pos = POSITION_INCAP;
    }
    else /* 40% chance of 2.00x damage on next hit. */
    {
      set_pos = POSITION_STUNNED;
    }

    set_pos = stack_position(victim, set_pos);

    act("You clobber $N with windmilling fists!", FALSE, ch, NULL, victim, TO_CHAR);
    act("You're clobbered by $n's windmilling fists!", FALSE, ch, NULL, victim, TO_VICT);
    act("$n clobbers $N with $s windmilling fists!", FALSE, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, calc_position_damage(GET_POS(victim), number(GET_LEVEL(ch), GET_LEVEL(ch) * 4)), SKILL_CLOBBER, DAM_NO_BLOCK);

    if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim))
    {
      GET_POS(victim) = set_pos;
    }

    skill_wait(ch, SKILL_CLOBBER, chance(15) ? 3 : 2);
  }
}


void do_victimize(CHAR *ch, char *argument, int cmd)
{
  AFF af;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_VICTIMIZE))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_VICTIMIZE))
  {
    affect_from_char(ch, SKILL_VICTIMIZE);

    send_to_char("You decide to show a modicum of mercy to your victims.\n\r", ch);
  }
  else
  {
    af.type = SKILL_VICTIMIZE;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = 0;

    affect_to_char(ch, &af);

    send_to_char("You will now take every opportunity to victimize your foe.\n\r", ch);
  }
}


void do_shadow_walk(CHAR *ch, char *argument, int cmd)
{
  AFF af;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_SHADOW_WALK))
  {
    send_to_char("You do not have this skill.\n\r", ch);

    return;
  }

  if (affected_by_spell(ch, SKILL_SHADOW_WALK))
  {
    affect_from_char(ch, SKILL_SHADOW_WALK);

    send_to_char("You will no longer slip into the shadows to attack your victims.\n\r", ch);
  }
  else
  {
    af.type = SKILL_SHADOW_WALK;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = 0;

    affect_to_char(ch, &af);

    send_to_char("You will now slip into the shadows to attack your victims.\n\r", ch);
  }
}


void do_evasion(CHAR *ch, char *argument, int cmd)
{
  AFF af;

  if (!ch->skills) return;

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
    act("You become less evasive and set yourself into a normal fighting stance.", TRUE, ch, 0, 0, TO_CHAR);
    act("$n becomes less evasive and sets $mself into a normal fighting stance.", TRUE, ch, 0, 0, TO_ROOM);

    affect_from_char(ch, SKILL_EVASION);

    skill_wait(ch, SKILL_EVASION, 1);

    return;
  }

  if (number(1, 85) > GET_LEARNED(ch, SKILL_EVASION))
  {
    send_to_char("You failed to focus on evading attacks.\n\r", ch);

    skill_wait(ch, SKILL_EVASION, 1);

    return;
  }
  else
  {
    act("You focus on evading attacks and fighting defensively.", TRUE, ch, 0, 0, TO_CHAR);
    act("$n focuses on evading attacks and fighting defensively.", TRUE, ch, 0, 0, TO_ROOM);

    af.type = SKILL_EVASION;
    af.duration = -1;
    af.modifier = ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? -4 : -5;
    af.location = APPLY_HITROLL;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);

    skill_wait(ch, SKILL_EVASION, 1);
  }
}
