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

extern struct room_data *world;
extern struct char_data *character_list;
extern int CHAOSMODE;
extern struct dex_app_type dex_app[];
extern struct wis_app_type wis_app[26];
extern struct obj_data  *object_list;

int calc_position_damage(int position, int dam);

void do_meditate(CHAR *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if(!ch->skills) return;

  if(!check_sc_access(ch,SKILL_MEDITATE)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }
  /* Hemp - no lag skill, no point in a failure if practiced */
  if(number(0,85)>ch->skills[SKILL_MEDITATE].learned) {
    send_to_char("You failed to focus your thoughts.\n\r",ch);
    return;
  }
  if(ch->specials.fighting) {
    send_to_char("You cannot meditate while fighting!\n\r",ch);
    return;
  }
  if(affected_by_spell(ch, SKILL_MEDITATE)) return;

  af.type      = SKILL_MEDITATE;
  if(CHAOSMODE)
    af.duration = 12;
  else
    af.duration = 33;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = 0;
  af.bitvector2 = 0;
  affect_to_char(ch, &af);
  act("You gaze inward and focus on healing.",1,ch,0,0,TO_CHAR);
  act("$n enters a deep trance.",1,ch,0,0,TO_ROOM);
}

void do_protect(CHAR *ch, char *argument, int cmd) {
  char buf[MAX_INPUT_LENGTH];
  CHAR *vict;

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_PROTECT)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  one_argument(argument,buf);
  if(!*buf) {
    send_to_char("Protect who?\n\r",ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("That player is not here.\n\r", ch);
    return;
  }
  if(IS_NPC(vict)) {
    send_to_char("Protect a mob? Impossible!\n\r",ch);
    return;
  }
  if(ch==vict) {
    act("You just protect yourself.",0,ch,0,0,TO_CHAR);
    act("$n just protects $mself.",0,ch,0,0,TO_NOTVICT);
    if(ch->specials.protecting) {
      act("$N stops protecting you.",0,ch->specials.protecting,0,ch,TO_CHAR);
      ch->specials.protecting->specials.protect_by=0;
    }
    ch->specials.protecting=0;
    return;
  }

  if(affected_by_spell(ch, SKILL_BERSERK)) {
    send_to_char("You can't protect anyone while berserked.\n\r",ch);
    return;
  }

  if(vict->specials.protecting) {
    act("$N is already protecting someone.",0,ch,0,vict,TO_CHAR);
    return;
  }
  if(ch->specials.protecting) {
    act("$N stops protecting you.",0,ch->specials.protecting,0,ch,TO_CHAR);
    act("You stop protecting $n.",0,ch->specials.protecting,0,ch,TO_VICT);
    ch->specials.protecting->specials.protect_by=0;
    ch->specials.protecting=0;
  }

  if((number(0,130)>ch->skills[SKILL_PROTECT].learned)  || (CHAOSMODE)) {
    act("You failed to start protecting $N.",0,ch,0,vict,TO_CHAR);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }
  act("You start trying to protect $N.",0,ch,0,vict,TO_CHAR);
  act("$n starts trying to protect you.",0,ch,0,vict,TO_VICT);
  act("$n starts trying to protect $N.",0,ch,0,vict,TO_NOTVICT);
  ch->specials.protecting=vict;
  vict->specials.protect_by=ch;
}

void do_retreat(struct char_data *ch, char *argument, int cmd) {
  int i, attempt, die;
  struct char_data *victim=NULL;
  int org_room;
  bool rider,mount;

  int special(struct char_data *ch, int cmd, char *arg);

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_RETREAT)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  rider=FALSE;
  mount=FALSE;

  if(IS_AFFECTED(ch, AFF_PARALYSIS) || IS_AFFECTED(ch, AFF_HOLD)) {
    act("$n tries to retreat but seems to be unable to move.",TRUE,ch,0,0,TO_ROOM);
    act("You try to retreat but are paralyzed and can't move!",TRUE,ch,0,0,TO_CHAR);
    return;
  }

  attempt=number(1,86); /* shouldn't fail if practiced */
  if(attempt > ch->skills[SKILL_RETREAT].learned) {
    act("You try to rapidly retreat but fail.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n tries to rapidly retreat but fails.", FALSE, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }

  if(!(ch->specials.fighting)) {
    for (i=0; i<10; i++) {
      attempt = number(0, 5);  /* Select a random direction */
      if(CAN_GO(ch, attempt) &&
         !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, DEATH)) {
        act("$n attempts to retreat.", TRUE, ch, 0, 0, TO_ROOM);
        org_room = CHAR_REAL_ROOM(ch);
        mount=FALSE;
        rider=FALSE;
        victim=NULL;
        if(ch->specials.riding) {
          if(org_room == CHAR_REAL_ROOM(ch->specials.riding)) {
            victim = ch->specials.riding;
            mount=TRUE;
          }
        }
        if(ch->specials.rider) {
          victim=ch->specials.rider;
          rider=TRUE;
        }

        if ((die = do_simple_move(ch, attempt, FALSE,1))== 1) {
          /* The escape has succeded */
          send_to_char("You retreat as fast as you are able.\n\r", ch);
          if( (rider) || (mount) ) {
            char_from_room(victim);
            char_to_room(victim, CHAR_REAL_ROOM(ch));
            do_look(victim, "\0",15);
            if(victim->specials.fighting) {
              if(victim->specials.fighting->specials.fighting == victim)
                stop_fighting(victim->specials.fighting);
              stop_fighting(victim);
            }
          }
          return;
        } else {
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
  for (i=0; i<10; i++) {
    attempt = number(0, 5);
    if(CAN_GO(ch, attempt) &&
       !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, DEATH)) {
      if(!IS_NPC(ch->specials.fighting) && !ch->specials.rider &&
         IS_SET(ch->specials.fighting->specials.pflag, PLR_BLOCK) &&
         ( (number(1, 130) < ch->specials.fighting->skills[SKILL_BLOCK].learned) ||
           (check_subclass(ch->specials.fighting,SC_WARLORD,1) && chance(80)) ) ) {
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
      mount=FALSE;
      rider=FALSE;
      victim=NULL;

      if(ch->specials.riding) {
        if(org_room == CHAR_REAL_ROOM(ch->specials.riding)) {
          victim = ch->specials.riding;
          mount=TRUE;
        }
      }
      if (ch->specials.rider) {
        victim=ch->specials.rider;
        rider=TRUE;
      }

      if((die = do_simple_move(ch, attempt, FALSE,1))== 1) {
        /* The escape has succeded */
        if(ch->specials.fighting) { /* Another check */

        /* Insert later when using huntig system        */
        /* ch->specials.fighting->specials.hunting = ch */

          if(ch->specials.fighting->specials.fighting == ch)
             stop_fighting(ch->specials.fighting);
          stop_fighting(ch);
        }
        send_to_char("You retreat as fast as you are able.\n\r", ch);

        if( (rider) || (mount) ) {
          char_from_room(victim);
          char_to_room(victim, CHAR_REAL_ROOM(ch));
          do_look(victim, "\0",15);
          if(victim->specials.fighting) {
            if(victim->specials.fighting->specials.fighting == victim)
              stop_fighting(victim->specials.fighting);
            stop_fighting(victim);
          }
        }
        return;
      } else {
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

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_BACKFIST))
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
      send_to_char("Backfist who?\n\r", ch);
      return;
    }
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }
  else if (IS_MORTAL(ch) &&
           IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);
    return;
  }
  else if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  check = number(1, 121) - dex_app[GET_DEX(ch)].prac_bonus;

  if (affected_by_spell(ch, SPELL_BLUR))
  {
    check -= (GET_LEVEL(ch) / 10);
  }

  if (check > ch->skills[SKILL_BACKFIST].learned)
  {
    act("You try to backfist $N but fail.", 0, ch, 0, victim, TO_CHAR);
    act("$n tries to backfist $N but fails.", 0, ch, 0, victim, TO_NOTVICT);
    act("$n tries to backfist you but fails.", 0, ch, 0, victim, TO_VICT);
    damage(ch, victim, 0, SKILL_BACKFIST, DAM_SKILL);

    if (!IS_IMPLEMENTOR(victim))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    if ((IS_NPC(victim) && IS_SET(victim->specials.immune2, SKILL_BACKFIST)) ||
        (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL)))
    {
      act("You backfist $N, but your backfist has no effect!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n backfists you, but $s backfist has no effect!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n backfists $N, but $s backfist has no effect!", FALSE, ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, 0, SKILL_BACKFIST, DAM_NO_BLOCK);
    }
    else
    {
      act("With a sickening crunch you hit $N with a huge backfist.", 0, ch, 0, victim, TO_CHAR);
      act("With a sickening crunch $n hits $N with a huge backfist.", 0, ch, 0, victim, TO_NOTVICT);
      act("With a sickening crunch $n hits you with a huge backfist.", 0, ch, 0, victim, TO_VICT);
      damage(ch, victim, calc_position_damage(GET_POS(victim), (GET_LEVEL(ch) * 5) / 4), SKILL_BACKFIST, DAM_SKILL);

      if (CHAR_REAL_ROOM(victim) != NOWHERE &&
          !IS_IMPLEMENTOR(victim))
      {
        if (GET_POS(victim) > POSITION_STUNNED)
        {
          GET_POS(victim) = POSITION_STUNNED;
        }

        if (CHAOSMODE)
        {
          WAIT_STATE(victim, PULSE_VIOLENCE * number(1, 2));
        }
        else
        {
          WAIT_STATE(victim, PULSE_VIOLENCE * 2);
        }
      }
    }

    if (!IS_IMPLEMENTOR(victim))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
}

void do_pray(CHAR *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if(!ch->skills) return;

  if(!check_sc_access(ch,SKILL_PRAY)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }
/* Hemp - no lag skill, no point in a failure if practiced */

  if(number(0,85)>ch->skills[SKILL_PRAY].learned) {
    send_to_char("You failed to focus your thoughts in prayer.\n\r",ch);
    return;
  }

  if(ch->specials.fighting) {
    send_to_char("You would do better to swear than to pray right now.\n\r",ch);
    return;
  }
  if(affected_by_spell(ch, SKILL_PRAY)) return;

  af.type      = SKILL_PRAY;
  af.duration  = -1;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = 0;
  af.bitvector2 = 0;
  affect_to_char(ch, &af);
  act("You bow your head and begin your prayer.",1,ch,0,0,TO_CHAR);
  act("$n bows $s head and begins praying.",1,ch,0,0,TO_ROOM);
}

void do_evade(CHAR *ch, char *argument, int cmd) {
  int recover=70;                          /* Linerfix */

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_EVADE)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  if(!ch->specials.fighting) {
    send_to_char("You aren't fighting anyone.\n\r",ch);
    return;
  }

  if(number(0,121)>ch->skills[SKILL_EVADE].learned) {
    act("You failed to evade $N.",0,ch,0,ch->specials.fighting,TO_CHAR);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }

  act("You quickly evade $N, and slip out of combat with $M!",0,ch,0,ch->specials.fighting,TO_CHAR);
  act("$n quickly evades you and slips out of combat.",0,ch,0,ch->specials.fighting,TO_VICT);
  act("$n quickly evades $N and slips out of combat with $M!",0,ch,0,ch->specials.fighting,TO_NOTVICT);
  if(ch->specials.fighting->specials.fighting==ch)
    stop_fighting(ch->specials.fighting);
  stop_fighting(ch);
  recover=number(50,75);                               /* Linerfix */
  if(check_subclass(ch,SC_BANDIT,4) && chance(recover)  && (!CHAOSMODE)) {
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
  }
  else {
  if(CHAOSMODE)
    WAIT_STATE(ch, PULSE_VIOLENCE*1);
  else
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
  }
}

void do_trip(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  struct affected_type_5 af;
  char name[256];
  int percent;
  int recover=70;                          /* Linerfix */

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_TRIP)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Trip who?\n\r", ch);
     return;
   }
  }

  if ((GET_LEVEL(victim) > LEVEL_MORT) && (!IS_NPC(victim))) {
    send_to_char("It is not a good idea to attack an Immortal!\n\r", ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE)&& (!CHAOSMODE)) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  percent=number(1,131)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */

  if (percent > ch->skills[SKILL_TRIP].learned ||
      (GET_LEVEL(ch)<GET_LEVEL(victim)-5) ||
      (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch,victim,BT_INVUL)) ){
    act("$n tries to trip $N but fails.",0,ch,0,victim,TO_NOTVICT);
    act("$n tries to trip you but fails.",0,ch,0,victim,TO_VICT);
    act("You try to trip $N but fail.",0,ch,0,victim,TO_CHAR);
    damage(ch, victim, 0, SKILL_TRIP,DAM_NO_BLOCK);
    if(!ch->specials.riding) GET_POS(ch) = POSITION_SITTING;
    recover=number(50,75);                               /* Linerfix */
    if(check_subclass(ch,SC_BANDIT,4) && chance(recover)) {
      WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }
    else {
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    }

    return;
  }

  act("$n sticks out $s foot and trips $N.",0,ch,0,victim,TO_NOTVICT);
  act("$n sticks out $s foot and trips you.",0,ch,0,victim,TO_VICT);
  act("You stick out your foot and trip $N.",0,ch,0,victim,TO_CHAR);

  if(!affected_by_spell(victim, SKILL_TRIP)) {
    af.type      = SKILL_TRIP;
    af.duration  = 0;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
  damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_TRIP,DAM_NO_BLOCK);
  GET_POS(victim)=POSITION_STUNNED;
  recover=number(50,75);                               /* Linerfix */
  if(check_subclass(ch,SC_BANDIT,4) && chance(recover)) {
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
  }
  else {
  WAIT_STATE(ch, PULSE_VIOLENCE*3);
  }
}

void do_tigerkick(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;

  if (!ch->skills) return;

  one_argument(arg, name);

  if (!check_sc_access(ch, SKILL_TIGERKICK))
  {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if (!(victim = get_char_room_vis(ch, name)))
  {
    if (ch->specials.fighting)
    {
      victim = ch->specials.fighting;
    }
    else
    {
      send_to_char("Tigerkick who?\n\r", ch);
      return;
    }
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }
  else if (IS_MORTAL(ch) &&
           IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);
    return;
  }
  else if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  check = number(1, 111) - dex_app[GET_DEX(ch)].prac_bonus;

  if (affected_by_spell(ch, SPELL_BLUR))
  {
    check -= (GET_LEVEL(ch) / 10);
  }

  if (check > ch->skills[SKILL_TIGERKICK].learned)
  {
    act("You try to tigerkick $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to tigerkick you, but misses.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to tigerkick $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_TIGERKICK, DAM_NO_BLOCK);

    if (!IS_IMPLEMENTOR(ch))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    act("You drive your foot into $N's chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n drives $s foot into your chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_VICT);
    act("$n drives $s foot into $N's chest with the ferocity of a tiger!", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, calc_position_damage(GET_POS(victim), (GET_LEVEL(ch) * 7) / 4), SKILL_TIGERKICK, DAM_NO_BLOCK);

    if (CHAR_REAL_ROOM(victim) != NOWHERE &&
        !IS_IMPLEMENTOR(victim))
    {
      if (GET_POS(victim) > POSITION_RESTING)
      {
        GET_POS(victim) = POSITION_RESTING;
      }

      if (CHAOSMODE)
      {
        WAIT_STATE(victim, PULSE_VIOLENCE * number(1, 2));
      }
      else
      {
        WAIT_STATE(victim, PULSE_VIOLENCE * 2);
      }
    }

    if (!IS_IMPLEMENTOR(ch))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
}

/* Portions of scan taken from a snippet (do_scan) by sjmaster@mit.edu */
void list_scanned_chars(struct char_data *list, struct char_data *ch, int distance, int door) {
  struct char_data *i;
  int count = 0;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  const char *how_far[] = {
    "close to the",
    "a ways to the",
    "far off to the"
  };
  const char *dir_name[]= {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down"};

  for (i = list; i; i = i->next_in_room)
    if (CAN_SEE(ch, i)) count++;
  if (!count) return;

  buf[0]='\0';
  for (i = list; i; i = i->next_in_room) {
    if (!CAN_SEE(ch, i)) continue;
    if (!*buf)
      sprintf(buf, "You see %s", (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)) );
    else
      sprintf(buf, "%s%s", buf, (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)));
    if (--count > 1)
      strcat(buf, ", ");
    else if (count == 1)
      strcat(buf, " and ");
    else {
      sprintf(buf2, " %s %s.\n\r", how_far[distance], dir_name[door]);
      strcat(buf, buf2);
    }

  }
  send_to_char(buf, ch);
}

void do_scan(CHAR *ch, char *argument, int cmd) {
  int d,room,exit_room,distance;
  const char *dir_name[]= {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down"};

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_SCAN)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You can't see a thing, you're blind!\n\r", ch);
    return;
  }

  send_to_char("You quickly scan the area.\n\r",ch);

  for (d=0;d<6;d++) { /* n,s,e,w,u,d */
    room=CHAR_REAL_ROOM(ch);
    if(!CAN_GO(ch,d)) continue;
    if (number(0,121) > ch->skills[SKILL_SCAN].learned) {
      printf_to_char(ch,"You failed to scan %s.\n\r",dir_name[d]);
      continue;
    }
    for(distance = 0 ; distance < 3; distance++) {
      if(!world[room].dir_option[d]) break;
      exit_room=world[room].dir_option[d]->to_room_r;
      if(exit_room==NOWHERE || exit_room==real_room(0) ||
         IS_SET(world[room].dir_option[d]->exit_info,EX_CLOSED) ||
         IS_SET(world[room].dir_option[d]->exit_info,EX_CRAWL) ||
         IS_SET(world[room].dir_option[d]->exit_info,EX_ENTER) ||
         IS_SET(world[room].dir_option[d]->exit_info,EX_JUMP) ||
         IS_SET(world[room].dir_option[d]->exit_info,EX_CLIMB))
        break;
      if(world[exit_room].people)
        list_scanned_chars(world[exit_room].people, ch, distance,d);
      room=exit_room;
    }
  }
}

void do_camp(struct char_data *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if(!ch->skills) return;

  if(!check_sc_access(ch,SKILL_CAMP)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }
  if(affected_by_spell(ch, SKILL_CAMP) || IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CLUB)) {
    send_to_char("There is already a camp here.\n\r",ch);
    return;
  }
  if(ch->specials.riding) {
    send_to_char("You must dismount first.\n\r",ch);
    return;
  }

  if(number(0,130)>ch->skills[SKILL_CAMP].learned) {
    send_to_char("You failed to set up camp.\n\r",ch);
    return;
  }

  if(count_mobs_real_room_except_followers(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't camp here, there is at least one mob!\n\r",ch);
    return;
  }

  af.type      = SKILL_CAMP;
  af.duration  = -1;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = 0;
  af.bitvector2 = 0;
  affect_to_char(ch, &af);
  GET_POS(ch)=POSITION_RESTING;
  SET_BIT(world[CHAR_REAL_ROOM(ch)].room_flags,CLUB);
  act("You quickly set up a camp here, then sit down and rest.",1,ch,0,0,TO_CHAR);
  act("$n quickly sets up a camp here, then sits down and rests.",1,ch,0,0,TO_ROOM);
}

void do_blitz(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  char name[256];
  int percent;

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_BLITZ)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Blitz who?\n\r", ch);
     return;
   }
  }

  if ((GET_LEVEL(victim) > LEVEL_MORT) && (!IS_NPC(victim))) {
    send_to_char("It is not a good idea to attack an Immortal!\n\r", ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE)&& (!CHAOSMODE)) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  percent=number(1,131)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */

  if(percent > ch->skills[SKILL_BLITZ].learned) {
    hit(ch, victim,SKILL_BLITZ);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }
  hit(ch,victim,SKILL_BLITZ);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_lunge(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  char name[256];
  int percent;

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_LUNGE)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon for it to be successful.\n\r",ch);
    return;
  }

/*  if (ch->equipment[WIELD]->obj_flags.value[3] != 11) {
    send_to_char("Only piercing weapons can be used for lunging.\n\r",ch);
    return;
  }
*/

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Lunge at who?\n\r", ch);
     return;
   }
  }

  if ((GET_LEVEL(victim) > LEVEL_MORT) && (!IS_NPC(victim))) {
    send_to_char("It is not a good idea to attack an Immortal!\n\r", ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE)&& (!CHAOSMODE)) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  percent=number(1,131)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */
  if(victim->specials.fighting)
    percent -= 5;

  if( (AWAKE(victim) && (percent > ch->skills[SKILL_LUNGE].learned)) ||
      IS_SET(victim->specials.immune,IMMUNE_ASSAULT) ) {
    act("You try to lunge at $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to lunge at you, but fails.", FALSE, victim, 0, ch, TO_CHAR);
    act("$n tries to lunge at $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_LUNGE,DAM_NO_BLOCK);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }
  hit(ch,victim,SKILL_LUNGE);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_fade(struct char_data *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  /* Level dependent hitroll penalty */
  int hitroll_fademod;
  if(!ch->skills) return;

  if(!check_sc_access(ch,SKILL_FADE)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }
  if(affected_by_spell(ch, SKILL_FADE)) {
    send_to_char("You are blended as much as possible already.\n\r",ch);
    return;
  }
  if(ch->specials.riding) {
    send_to_char("You must dismount first.\n\r",ch);
    return;
  }
/* Hemp - no lag skill, no point in failure if practiced */

  if(number(0,85)>ch->skills[SKILL_FADE].learned) {
    send_to_char("You failed to blend in.\n\r",ch);
    return;
  }

  /* Set the hitroll penalty based on character level */
  if ( GET_LEVEL(ch) < 35 ) { /* 30-34 */
    hitroll_fademod = -5;
  } else {
    if ( GET_LEVEL(ch) < 40 ) { /* 35-39 */
      hitroll_fademod = -4;
      } else {
        if ( GET_LEVEL(ch) < 45 ) { /* 40-44 */
          hitroll_fademod = -3;
          } else {
            if ( GET_LEVEL(ch) < 50 ) { /* 45-49 */
              hitroll_fademod = -2;
              } else {
                if ( GET_LEVEL(ch) == 50 ) { /* 50 */
                  hitroll_fademod = -1;
                  } else { /* Greater than 50 */
                    hitroll_fademod = 0;
                  }
              }

          }
    }
  }

  af.type      = SKILL_FADE;
  af.duration  = -1;
  /*af.modifier  = -20;
  af.location  = APPLY_AC;*/
  af.bitvector = 0;
  af.bitvector2 = 0;
/*  affect_to_char(ch, &af);*/
  af.modifier  = hitroll_fademod;
  af.location  = APPLY_HITROLL;
  affect_to_char(ch, &af);

  act("You quickly fade in with your surroundings.",1,ch,0,0,TO_CHAR);
/*  act("$n quickly fades in with $s surroundings.",1,ch,0,0,TO_ROOM);*/
}

void do_switch(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  char name[256];
  int percent;

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_SWITCH)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  if(!ch->specials.fighting) {
    send_to_char("You aren't fighting anything.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Switch to who?\n\r", ch);
    return;
  }

  if ((GET_LEVEL(victim) > LEVEL_MORT) && (!IS_NPC(victim))) {
    send_to_char("It is not a good idea to attack an Immortal!\n\r", ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE)&& (!CHAOSMODE)) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

/*  if(victim==ch->specials.fighting) {
    act("You are already fighting $N.",0,ch,0,victim,TO_CHAR);
    return;
  }
*/

  percent=number(1,111); /* 101% is a complete failure */

  if( (percent > ch->skills[SKILL_SWITCH].learned) ||
      (IS_AFFECTED(ch,AFF_FURY) && GET_CLASS(ch)==CLASS_WARRIOR) ||
      (IS_AFFECTED(ch,AFF_FURY) && GET_CLASS(ch)==CLASS_PALADIN && chance(50)) ) {
    send_to_char("You failed to switch.\n\r",ch);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }
  act("$n switches $s fight to $N",0,ch,0,victim,TO_NOTVICT);
  act("You switch your fight to $N!",0,ch,0,victim,TO_CHAR);
  act("$n switches $s fight to you!",0,ch,0,victim,TO_VICT);
  stop_fighting(ch);
  set_fighting(ch,victim);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_flank(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  char name[256];
  int percent;

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_FLANK)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Flank who?\n\r", ch);
     return;
   }
  }

  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon for it to be successful.\n\r",ch);
    return;
  }

  if ((GET_LEVEL(victim) > LEVEL_MORT) && (!IS_NPC(victim))) {
    send_to_char("It is not a good idea to attack an Immortal!\n\r", ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE)&& (!CHAOSMODE)) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  percent=number(1,131)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */

  if(percent > ch->skills[SKILL_FLANK].learned) {
    hit(ch, victim,SKILL_FLANK);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }
  hit(ch,victim,SKILL_FLANK);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_impair(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int percent;
  struct obj_data *wield = 0;
  int recover=70;                          /* Linerfix */

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_IMPAIR)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon for it to be successful.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Impair who?\n\r", ch);
     return;
   }
  }

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE)&& (!CHAOSMODE)) {
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
    send_to_char("Impair a person who is not wielding anything??\n\r", ch);
    return;
  }

  percent= number(1,175)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */

  if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    percent = number(1, 600)-dex_app[GET_DEX(ch)].prac_bonus;

  if (V_OBJ(wield)==11523 || (ch->skills ? (percent > ch->skills[SKILL_IMPAIR].learned) ||
      (IS_SET(victim->specials.act, ACT_ARM) && IS_NPC(victim)) ||
      ((GET_LEVEL(victim)-GET_LEVEL(ch))>5) || GET_LEVEL(victim)>45 : percent > 100) ) {
     act("You try to impair $N's ability to hit, but fail!",FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to impair your ability to hit, but fails!",FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to impair $N's ability to hit, but fails!",FALSE, ch, 0, victim, TO_NOTVICT);
    hit(ch, victim, TYPE_UNDEFINED);
  } else {
     act("You quickly drive your weapon into $N.\n\r$N howls in pain and drops $S weapon.",FALSE, ch, 0,victim, TO_CHAR);
     act("$n quickly drives $s weapon into you.\n\rYou howl in pain and drop your weapon.",FALSE, ch, 0,victim, TO_VICT);
     act("$n quickly drives $s weapon into $N.\n\r$N howls in pain and drops $S weapon.",FALSE, ch, 0, victim, TO_NOTVICT);
     unequip_char(victim, WIELD);
     if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC)) obj_to_char(wield, victim);
     else {
      log_f("WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(ch),
             GET_NAME(victim),OBJ_NAME(wield),world[CHAR_REAL_ROOM(victim)].number);
      obj_to_room(wield, CHAR_REAL_ROOM(victim));
      wield->log=1;
     }
     save_char(victim,NOWHERE);
     hit(ch, victim, TYPE_UNDEFINED);
  }
    recover=number(50,75);                               /* Linerfix */
    if(check_subclass(ch,SC_BANDIT,4) && chance(recover)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
    }
    else {
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }
}

void do_sweep(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *temp, *tmp_victim;
  char name[256];
  int percent;

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_SWEEP)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon for it to be successful.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  percent=number(1,131)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */

  if(percent > ch->skills[SKILL_SWEEP].learned) {
    act("You try to sweep around the room with your weapon, but fail.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n tries to sweep around the room with $s weapon, but fail.", FALSE, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    return;
  }

  act("You deftly sweep with your weapon.", FALSE, ch, 0, 0, TO_CHAR);
  act("$n deftly sweeps with $s weapon.", FALSE, ch, 0, 0, TO_ROOM);
  for(tmp_victim=world[CHAR_REAL_ROOM(ch)].people;tmp_victim;tmp_victim=temp) {
    temp = tmp_victim->next_in_room;
    /* Added check so you don't kill your mount - Ranger April 96*/
    if(ch!=tmp_victim && IS_NPC(tmp_victim)&& tmp_victim->specials.rider!=ch)
      hit(ch, tmp_victim,TYPE_UNDEFINED);
    if(ch!=tmp_victim && !IS_NPC(tmp_victim) && IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
      hit(ch, tmp_victim,TYPE_UNDEFINED);

    if (ch!=tmp_victim && IS_MORTAL(tmp_victim) && (CHAOSMODE) && !tmp_victim->specials.fighting) {  /* Chaos03 */
      hit(ch, tmp_victim,TYPE_UNDEFINED);
    }

    if(CHAR_REAL_ROOM(tmp_victim)!=NOWHERE && IS_NPC(tmp_victim)) WAIT_STATE(tmp_victim, PULSE_VIOLENCE*2);
  }                                   /* Linerfix 03/02, check for IS_NPC also to give 2 rnd lag */
  WAIT_STATE(ch, PULSE_VIOLENCE*4);
}

void do_hostile(CHAR *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if(!ch->skills) return;

  if(!check_sc_access(ch,SKILL_HOSTILE)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }
  if(ch->specials.fighting) {
    send_to_char("You can't change your stance while fighting.\n\r",ch);
    return;
  }
  if(!affected_by_spell(ch, SKILL_HOSTILE)) {
    if(number(0,120)>ch->skills[SKILL_HOSTILE].learned) {
      send_to_char("You failed to get into the hostile stance.\n\r",ch);
      WAIT_STATE(ch,PULSE_VIOLENCE);
      return;
    }
    af.type      = SKILL_HOSTILE;
    af.duration  = -1;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
    act("You place yourself in a hostile stance.",1,ch,0,0,TO_CHAR);
    act("$n places $mself in a hostile stance.",1,ch,0,0,TO_ROOM);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
  else {
    affect_from_char(ch,SKILL_HOSTILE);
    act("$n places $mself in a normal stance.",1,ch,0,0,TO_ROOM);
    act("You place yourself in a normal stance.",1,ch,0,0,TO_CHAR);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
}

void do_defend(CHAR *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if(!ch->skills) return;

  if(!check_sc_access(ch,SKILL_DEFEND)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }
  if(ch->specials.fighting) {
    send_to_char("You can't change your stance while fighting.\n\r",ch);
    return;
  }
  if(!affected_by_spell(ch, SKILL_DEFEND)) {
    if(number(0,120)>ch->skills[SKILL_DEFEND].learned) {
      send_to_char("You failed to get into the defensive stance.\n\r",ch);
      WAIT_STATE(ch,PULSE_VIOLENCE);
      return;
    }
    af.type      = SKILL_DEFEND;
    af.duration  = -1;
    af.modifier  = -100;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
    act("You place yourself in a defensive stance.",1,ch,0,0,TO_CHAR);
    act("$n places $mself in a defensive stance.",1,ch,0,0,TO_ROOM);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
  else {
    affect_from_char(ch,SKILL_DEFEND);
    act("$n places $mself in a normal stance.",1,ch,0,0,TO_ROOM);
    act("You place yourself in a normal stance.",1,ch,0,0,TO_CHAR);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
}

void do_move(struct char_data *ch, char *argument, int cmd);
void do_assassinate(struct char_data *ch, char *argument, int cmd) {
  int room,dir=-1,percent;
  char buf[MIL],name[MIL];

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_ASSASSINATE)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  argument=one_argument(argument,name);
  if(!*name) {
    send_to_char("Assassinate who?\n\r",ch);
    return;
  }
  one_argument(argument,buf);
  if(!*buf) {
    send_to_char("What direction?\n\r",ch);
    return;
  }

  if(is_abbrev(buf,"north")) dir=CMD_NORTH;
  if(is_abbrev(buf,"east")) dir=CMD_EAST;
  if(is_abbrev(buf,"south")) dir=CMD_SOUTH;
  if(is_abbrev(buf,"west")) dir=CMD_WEST;
  if(is_abbrev(buf,"up")) dir=CMD_UP;
  if(is_abbrev(buf,"down")) dir=CMD_DOWN;

  if(dir==-1) {
    send_to_char("What direction?\n\r",ch);
    return;
  }

  if(ch->specials.riding) {
    send_to_char("Not while riding.\n\r",ch);
    return;
  }

  if(!IS_AFFECTED(ch, AFF_SNEAK)) {
    send_to_char("You need to be sneaking.\n\r",ch);
    return;
  }

  percent=number(1,110);
  if(percent > ch->skills[SKILL_ASSASSINATE].learned) {
    act("You fail your assassination attempt.", FALSE, ch, 0, 0, TO_CHAR);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }

  room=CHAR_REAL_ROOM(ch);
  do_move(ch, "", dir);

  if(room!=CHAR_REAL_ROOM(ch))
    do_backstab(ch,name,CMD_BACKSTAB);
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

  check = number(1, 131) - dex_app[GET_DEX(ch)].prac_bonus;

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

      if (CHAR_REAL_ROOM(victim) != NOWHERE &&
          (IS_NPC(victim) || GET_LEVEL(victim) < LEVEL_IMP))
      {
        if (GET_POS(victim) > POSITION_SITTING)
        {
          GET_POS(victim) = POSITION_SITTING;
        }
      }
    }

    if (IS_NPC(ch) ||
        GET_LEVEL(ch) < LEVEL_IMP)
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
}


void do_charge(struct char_data *ch, char *argument, int cmd) {
  CHAR *leader,*victim;
  struct follow_type *Followers=NULL;
  int percent;
  char name[256];

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_CHARGE)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  if(ch->specials.fighting) {
    send_to_char("You are already fighting.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if(!(victim = get_char_room_vis(ch, name))) {
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
    act("$N is already fighting someone.",0,ch,0,victim,TO_CHAR);
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

  percent=number(1,131);

  if(percent > ch->skills[SKILL_CHARGE].learned) {
    act("You try to lead a charge against $N but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to lead a charge against you, but fails.", FALSE, victim, 0, ch, TO_CHAR);
    act("$n tries to lead a charge against $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_CHARGE,DAM_NO_BLOCK);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }

  /* find the leader */
  if((!IS_AFFECTED(ch,AFF_GROUP) || ch->master==NULL)) {
    leader = ch;
  }
  else {
    if(!IS_AFFECTED(ch->master,AFF_GROUP))
      leader=ch;
    else
      leader=ch->master;
  }

  act("You lead a charge against $N!", FALSE, ch, 0, victim, TO_CHAR);
  act("$N leads a charge against you!", FALSE, victim, 0, ch, TO_CHAR);
  act("$n leads a charge against $N!", FALSE, ch, 0, victim, TO_NOTVICT);

  /* charger hits */
  hit(ch,victim,SKILL_CHARGE);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);

  if(!victim || CHAR_REAL_ROOM(victim)==NOWHERE) return;

  /* Go through the followers */
  for (Followers=leader->followers;Followers; Followers=Followers->next)
  {
    if (Followers->follower->desc) /*make sure they're connected*/
    {
       if(IS_AFFECTED(Followers->follower,AFF_GROUP) &&
          CHAR_REAL_ROOM(Followers->follower) == CHAR_REAL_ROOM(ch) && Followers->follower!=ch)
       {
         send_to_char("You join in the charge!\n\r",Followers->follower);
         hit(Followers->follower,victim,SKILL_CHARGE);
         if(!victim || CHAR_REAL_ROOM(victim)==NOWHERE) return;
       }
    }
  }
  /* Have leader charge */
  if(CHAR_REAL_ROOM(leader) == CHAR_REAL_ROOM(ch) && leader!=ch) {
    send_to_char("You join in the charge!\n\r",leader);
    hit(leader,victim,SKILL_CHARGE);
  }
}

void do_headbutt(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int wait = 0;

  if (!ch->skills) return;

  if (IS_MORTAL(ch) &&
      !check_sc_access(ch, SKILL_HEADBUTT))
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
      send_to_char("Headbutt who?\n\r", ch);
      return;
    }
  }

  if (victim == ch)
  {
    send_to_char("This could prove very interesting...\n\r", ch);
    return;
  }
  else if (IS_MORTAL(ch) &&
           IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);
    return;
  }
  else if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  check = number(1, 131) - dex_app[GET_DEX(ch)].prac_bonus;

  if (check > ch->skills[SKILL_HEADBUTT].learned)
  {
    act("You miss $N with your headbutt.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n misses you with $s headbutt.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n misses $N with $s headbutt.", FALSE, ch, NULL, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_HEADBUTT, DAM_NO_BLOCK);

    if (!IS_IMPLEMENTOR(victim))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
    {
      act("You slam your forehead into $N's face, but $E isn't even phased.", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n slams his forehead into your face, but you aren't even phased.", FALSE, ch, NULL, victim, TO_VICT);
      act("$n slams $s forehead into $N's face, but $E isn't even phased.", FALSE, ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, 0, SKILL_HEADBUTT, DAM_NO_BLOCK);
    }
    else
    {
      act("You slam your forehead into $N's face!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n slams his forehead into your face!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n slams $s forehead into $N's face!", FALSE, ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, calc_position_damage(GET_POS(victim), 200), SKILL_HEADBUTT, DAM_NO_BLOCK);

      if (CHAR_REAL_ROOM(victim) != NOWHERE &&
          !IS_IMPLEMENTOR(victim))
      {
        if (GET_POS(victim) > POSITION_SITTING)
        {
          GET_POS(victim) = POSITION_SITTING;
        }
      }
    }

    if (!IS_IMPLEMENTOR(victim))
    {
      wait = number(1, 8);
      wait = ((wait == 1) ? 1 : ((wait == 2 || wait == 3) ? 2 : 3));

      WAIT_STATE(ch, PULSE_VIOLENCE * wait);
    }
  }
}

void do_banzai(struct char_data *ch, char *arg, int cmd)
{
  CHAR *victim;
  char name[256];
  int check = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_BANZAI))
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
      send_to_char("Banzai charge who?\n\r", ch);
      return;
    }
  }

  if (GET_MANA(ch) < 75)
  {
    send_to_char("You do not have enough mana.\n\r", ch);
    return;
  }

  check = number(1, 101) - dex_app[GET_DEX(ch)].prac_bonus;

  if (check > ch->skills[SKILL_BANZAI].learned)
  {
    act("You try to banzai charge $N but fail.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to banzai charge you but fails.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to banzai charge $N but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_BANZAI, DAM_NO_BLOCK);

    GET_MANA(ch) = MAX(GET_MANA(ch) - 37, 0);

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
  }
  else
  {
    act("With a primal yell, you banzai charge $N.", FALSE, ch, 0, victim, TO_CHAR);
    act("With a primal yell, $n banzai charges you.", FALSE, ch, 0, victim, TO_VICT);
    act("With a primal yell, $n banzai charges $N.", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, number(200, 300), SKILL_BANZAI, DAM_SKILL);
    hit(ch, victim, SKILL_BANZAI);

    GET_MANA(ch) = MAX(GET_MANA(ch) - 75, 0);

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
  }
}

void do_mantra(struct char_data *ch, char *arg, int cmd)
{
  CHAR *victim;
  char name[256];
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
  else
  {
    if (!(victim = get_char_room_vis(ch, name)))
    {
      send_to_char("Chant your mantra to who?\n\r", ch);
      return;
    }
  }

  if (GET_MANA(ch) < 120)
  {
    send_to_char("You do not have enough mana.\n\r", ch);
    return;
  }

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      ((duration_of_spell(victim, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(victim, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The power of the skill fails to heal your degenerated body.\n\r", victim);
    return;
  }

  if (CHAOSMODE && victim != ch)
  {
    send_to_char("You cannot perform this skill on another player during chaos.\n\r", ch);
    return;
  }

  check = number(1, 101) - wis_app[GET_WIS(ch)].bonus;

  if (check > ch->skills[SKILL_MANTRA].learned)
  {
    if (victim != ch)
    {
      act("You chant your mantra to $N, but nothing happens.", FALSE, ch, 0,victim, TO_CHAR);
      act("$n chants $s mantra to you, but nothing happens.", FALSE, ch, 0,victim, TO_VICT);
      act("$n chants $s mantra to $N, but nothing happens.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    else
    {
      act("You chant softly to yourself with no noticeable affect.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n chants softly to $mself with no noticeable affect.", FALSE, ch, 0, 0, TO_ROOM);
    }

    GET_MANA(ch) = MAX(GET_MANA(ch) - 60, 0);

    WAIT_STATE(ch, PULSE_VIOLENCE);
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
      act("You chant softly, healing your spirit and giving yourself life.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n chants softly, healing $s spirit and giving $mself life.", FALSE, ch, 0, 0, TO_ROOM);
    }

    GET_MANA(ch) = MAX(GET_MANA(ch) - 120, 0);
    GET_HIT(victim) = MIN(GET_HIT(victim) + 500, GET_MAX_HIT(victim));

    heal = (GET_LEVEL(ch) + (5 * wis_app[GET_WIS(ch)].bonus));

    for (tmp_af = victim->affected; tmp_af; tmp_af = af_next)
    {
      af_next = tmp_af->next;

      if (tmp_af->type == SKILL_MANTRA)
      {
        if (tmp_af->modifier >= heal) affect_from_char(victim, SKILL_MANTRA);
      }
    }

    af.type       = SKILL_MANTRA;
    af.duration   = 10;
    af.modifier   = heal;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);

    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
}

void do_frenzy(CHAR *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if(!ch->skills) return;

  if(!check_sc_access(ch,SKILL_FRENZY)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }
  if(ch->specials.fighting) {
    send_to_char("You can't change your mindset while fighting.\n\r",ch);
    return;
  }
  if(!affected_by_spell(ch, SKILL_FRENZY)) {
    if(number(0,120)>ch->skills[SKILL_FRENZY].learned) {
      send_to_char("You failed to get into the frenzied mindset.\n\r",ch);
      WAIT_STATE(ch,PULSE_VIOLENCE);
      return;
    }
    af.type      = SKILL_FRENZY;
    af.duration  = -1;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
    act("You go into a frenzy.",1,ch,0,0,TO_CHAR);
    act("$n goes into a frenzy.",1,ch,0,0,TO_ROOM);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
  else {
    affect_from_char(ch,SKILL_FRENZY);
    act("$n calms down.",1,ch,0,0,TO_ROOM);
    act("You calm down.",1,ch,0,0,TO_CHAR);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
}

void do_berserk(CHAR *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if(!ch->skills) return;

  if(!check_sc_access(ch,SKILL_BERSERK)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }
  if(ch->specials.fighting) {
    send_to_char("You can't change your mindset while fighting.\n\r",ch);
    return;
  }
  if(!affected_by_spell(ch, SKILL_BERSERK)) {
    if(number(0,120)>ch->skills[SKILL_BERSERK].learned) {
      send_to_char("You failed to get into a berserk mindset.\n\r",ch);
      WAIT_STATE(ch,PULSE_VIOLENCE);
      return;
    }
    af.type      = SKILL_BERSERK;
    af.duration  = -1;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
    act("You go berserk!",1,ch,0,0,TO_CHAR);
    act("$n goes berserk.",1,ch,0,0,TO_ROOM);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
  else {
    affect_from_char(ch,SKILL_BERSERK);
    act("$n calms down.",1,ch,0,0,TO_ROOM);
    act("You calm down.",1,ch,0,0,TO_CHAR);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
}

void do_scalp(CHAR *ch, char *argument, int cmd) {
  struct obj_data *corpse=0,*scalp;
  char obj_name[240],scalp_name[240],buf[MIL];

  if(!check_sc_access(ch,SKILL_SCALP)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  argument = one_argument(argument, obj_name);
  if(!(corpse = get_obj_in_list_vis(ch, obj_name, world[CHAR_REAL_ROOM(ch)].contents))){
    send_to_char("Scalp what?\n\r", ch);
    return;
  }

  if(GET_ITEM_TYPE(corpse)!=ITEM_CONTAINER && corpse->obj_flags.value[3]!=1) {
    send_to_char("That isn't a corpse.\n\r",ch);
    return;
  }

  if(!IS_NPC(ch) && (corpse->obj_flags.cost==PC_CORPSE || corpse->obj_flags.cost==CHAOS_CORPSE)) {
    send_to_char("The thought of scalping a fellow adventurer makes you cringe!\n\r",ch);
    return;
  }

  if(corpse->obj_flags.cost==PC_STATUE || corpse->obj_flags.cost==NPC_STATUE) {
    send_to_char("You find it impossible to chip away at the stone.\n\r",ch);
    return;
  }

  /* check to see if corpse is scalpable */
  if(!CORPSE_HAS_SCALP(corpse)) {
    send_to_char("That corpse has no scalp.\n\r",ch);
    return;
  }

  if(IS_SET(corpse->obj_flags.extra_flags2, ITEM_NO_SCALP)) {
    send_to_char("The scalp has already been removed.\n\r",ch);
    return;
  }

  if(GET_LEVEL(ch) < LEVEL_IMM) {
    if (!ch->equipment[WIELD]) {
      send_to_char("Not with your bare hands.\r\n", ch);
      return;
    }
    switch (ch->equipment[WIELD]->obj_flags.value[3]) {
      case 0  :/* TYPE WHIP */
      case 1  :/* TYPE WHIP */
      case 2  :/* TYPE WHIP */
      case 4  :/* TYPE WHIP */
      case 5  :/* TYPE_STING */
      case 6  :/* TYPE_CRUSH */
      case 7  :/* TYPE_BLUDGEON */
        send_to_char("You can't scalp with that type of weapon.\n\r",ch);
        return;
        break;
    }
  }

  if(!IS_NPC(ch) && number(0,120)>ch->skills[SKILL_SCALP].learned) {
    send_to_char("You failed.\n\r",ch);
    WAIT_STATE(ch,PULSE_VIOLENCE);
    return;
  }

  CREATE(scalp, struct obj_data, 1);
  clear_object(scalp);
  scalp->item_number = NOWHERE;
  scalp->in_room = NOWHERE;
  scalp->name = str_dup("scalp");
  half_chop(corpse->description,buf,MIL,scalp_name,240);
  sprintf(buf,"scalp %s",scalp_name);
  scalp->description = str_dup(buf);
  half_chop(corpse->short_description,buf,MIL,scalp_name,240);
  sprintf(buf,"scalp %s",scalp_name);
  scalp->short_description = str_dup(buf);
  scalp->obj_flags.timer=10;

  scalp->obj_flags.type_flag    = ITEM_SCALP;
  scalp->obj_flags.wear_flags   = ITEM_TAKE+ITEM_WEAR_HEAD;
  scalp->obj_flags.extra_flags  = ITEM_ANTI_RENT;
  scalp->obj_flags.extra_flags2 = ITEM_ALL_DECAY;
  scalp->obj_flags.material     = corpse->obj_flags.material;
  scalp->obj_flags.value[0]     = 0;
  scalp->obj_flags.value[1]     = 0;
  scalp->obj_flags.value[2]     = 0;
  scalp->obj_flags.value[3]     = 0;
  scalp->obj_flags.weight       = corpse->obj_flags.weight/10;
  scalp->obj_flags.cost         = corpse->obj_flags.cost_per_day;

  scalp->next = object_list;
  object_list = scalp;
  obj_to_room(scalp, CHAR_REAL_ROOM(ch));

  send_to_char("You strip the scalp off the corpse.\n\r", ch);
  act("$n strips the scalp off the corpse.", TRUE, ch, 0,0, TO_ROOM);

  /* make corpse unscalpable */
  SET_BIT(corpse->obj_flags.extra_flags2, ITEM_NO_SCALP);
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

  percent=number(1,131)-dex_app[GET_DEX(ch)].prac_bonus;
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

void do_execute(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int percent,hp_percent;

  if(!ch->skills) return;
  if(!check_sc_access(ch,SKILL_EXECUTE)) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Execute who?\n\r", ch);
     return;
   }
  }

  if (victim == ch) {
    send_to_char("Bet that would feel great..\n\r", ch);
    return;
  }

  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon for it to succeed.\n\r",ch);
    return;
  }

  if (ch->equipment[WIELD]->obj_flags.value[3] != 3) {
    send_to_char("Only slashing weapons can be used to execute.\n\r",ch);
    return;
  }

  if(!IS_NPC(victim)) {
    send_to_char("You can't use this on other players.\n\r",ch);
    return;
  }

  percent=number(1,131)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */
  hp_percent=GET_HIT(victim)*100/GET_MAX_HIT(victim)+80;

  if((percent > ch->skills[SKILL_EXECUTE].learned) || chance(hp_percent) ||
     (IS_NPC(victim) && IS_SET(victim->specials.immune2,IMMUNE_EXECUTE))) {
    act("You try to execute $N, but $E jumps back!", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to execute you, but you jump back!", FALSE, victim, 0, ch, TO_CHAR);
    act("$n tries to execute $N, but $E jumps back!", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$N avoids your pitiful attack and strikes back at you!", FALSE, ch, 0, victim, TO_CHAR);
    act("You avoid $N's pitiful attack and strike back at $M!", FALSE, victim, 0, ch, TO_CHAR);
    act("$N avoids $n's pitiful attack and strikes back at $m!", FALSE, ch, 0, victim, TO_NOTVICT);
    hit(victim, ch, TYPE_UNDEFINED);
    if(CHAR_REAL_ROOM(ch)!=NOWHERE) WAIT_STATE(ch, PULSE_VIOLENCE*3);
    return;
  }

  act("You scream 'THERE CAN BE ONLY ONE!' and cut $N's head off!", FALSE, ch, 0, victim, TO_CHAR);
  act("$n screams 'THERE CAN BE ONLY ONE!' and cuts your head off!", FALSE, ch, 0, victim, TO_VICT);
  act("$n screams 'THERE CAN BE ONLY ONE!' and cut $N's head off!", FALSE, ch, 0, victim, TO_NOTVICT);
  damage(ch,victim,32000,TYPE_UNDEFINED,DAM_NO_BLOCK);
  WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

void do_clobber(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!ch->skills) return;

  if (!check_sc_access(ch, SKILL_CLOBBER))
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
      send_to_char("Clobber who?\n\r", ch);
      return;
    }
  }

  if (victim == ch)
  {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }
  else if (IS_MORTAL(ch) &&
           IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);
    return;
  }
  else if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  check = number(1, 131) - dex_app[GET_DEX(ch)].prac_bonus;

  if (check > ch->skills[SKILL_CLOBBER].learned)
  {
    act("Your attempt to clobber $N misses everything but the air.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n flails about wildly, missing you with $s erratic swinging.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n flails about wildly, missing $N with $s erratic swinging.", FALSE, ch, NULL, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_CLOBBER, DAM_NO_BLOCK);

    if (!IS_IMPLEMENTOR(ch))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
    {
      act("You clobber $N, but $E doesn't seem to mind!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n tries to clobber you, but it feels more like a massage!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n clobbers $N, but $E doesn't seem to mind!", FALSE, ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, 0, SKILL_CLOBBER, DAM_NO_BLOCK);
    }
    else
    {
      act("You clobber $N with windmilling fists!", FALSE, ch, NULL, victim, TO_CHAR);
      act("You're clobbered by $n's windmilling fists!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n clobbers $N with $s windmilling fists!", FALSE, ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, calc_position_damage(GET_POS(victim), number(GET_LEVEL(ch), GET_LEVEL(ch) * 4)), SKILL_CLOBBER, DAM_NO_BLOCK);

      if (CHAR_REAL_ROOM(victim) != NOWHERE &&
          !IS_IMPLEMENTOR(victim))
      {
        switch (number(1, 10))
        {
            case 1: /* 20% chance of 2.50x damage on next hit. */
            case 2:
              set_pos = POSITION_MORTALLYW;
              break;

            case 3: /* 40% chance of 2.33x damage on next hit. */
            case 4: 
            case 5:
            case 6:
              set_pos = POSITION_INCAP;
              break;

            default: /* 40% chance of 2.00x damage on next hit. */
              set_pos = POSITION_STUNNED;
              break;
        }
        
        GET_POS(victim) = set_pos;
      }
    }

    if (!IS_IMPLEMENTOR(ch))
    {
      if (chance(15))
      {
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
      }
      else
      {
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
      }
    }
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

  if (!affected_by_spell(ch, SKILL_VICTIMIZE))
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
  else
  {
    affect_from_char(ch, SKILL_VICTIMIZE);

    send_to_char("You decide to show a modicum of mercy to your victims.\n\r",ch);
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

  if (!affected_by_spell(ch, SKILL_SHADOW_WALK))
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
  else
  {
    affect_from_char(ch, SKILL_SHADOW_WALK);

    send_to_char("You will no longer slip into the shadows to attack your victims.\n\r",ch);
  }
}
