/*
*  file: act.offensive.c , Implementation of commands.    Part of DIKUMUD *
*  Usage : Offensive commands.                                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2005/01/25 21:52:21 $
$Header: /home/ronin/cvs/ronin/act.offensive.c,v 2.15 2005/01/25 21:52:21 ronin Exp $
$Id: act.offensive.c,v 2.15 2005/01/25 21:52:21 ronin Exp $
$Name:  $
$Log: act.offensive.c,v $

Sane - added blame 2008/4/2

Revision 2.15  2005/01/25 21:52:21  ronin
Fix for SKILL_ASSAULT

Revision 2.14  2005/01/21 14:55:26  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.13  2004/11/19 14:53:34  void
Fixed some errors with Cover

Revision 2.12  2004/11/17 19:21:52  void
Added Nomad Skill Cover (1/2 Damage -10 Hitroll)

Revision 2.11  2004/11/16 04:53:56  ronin
Chaos 2004 Update

Revision 2.10  2004/11/15 22:29:10  void
Made changes to Blood Lust, Twist, Quick, & Quad

Revision 2.9  2004/10/21 17:08:22  void
Added level 50 AP spell Blood Lust

Revision 2.8  2004/03/09 20:21:58  void
Changed Backstab to have higher percentages if affected by Hide, Sneak, or
Imp Invis.


Revision 2.6  2004/03/04 13:45:48  ronin
Fix to kick and bash to stop position change with 0 damage.

Revision 2.5  2004/03/04 00:24:39  Void
Fixed Kick to stop position change with missed kick.

Revision 2.4  2004/02/25 19:45:12  Void
Changed Flee code to reflect with hold person code

Revision 2.3  2004/02/19 19:20:22  Void
Added Spell Shroud of Existence (Good Align Commando Spell)

Revision 2.2  2004/02/17 21:48:58  Void
Changed Kick to make opponent go to POSITION_SITTING if successful.

Revision 2.1  2004/02/14 18:18:18  Void
Changed Punch code to be affected by Level.

Revision 2.0.0.1  2004/02/05 16:08:37  ronin
Reinitialization of cvs archives


Revision 6-Nov-03 Ranger
Added disarm log.

Revision 17-Mar-03 Ranger
Addition of breakthrough check to bash and punch to ensure bash/punch
didn't show missed messages but still cause the victim to sit.

Revision 1.10 2003/03/16 14:32:20  ronin
Revision of do_bash and do_punch to make sure vict and ch go to positions.
Position changes moved to fight.c (after invul checks).

Revision 1.9  2003/03/08 14:32:20  ronin
Revision of do_bash to do up to level in dam and make sure vict goes
to resting. - first skill reviews.

Revision 1.8  2003/02/22 14:32:20  ronin
Added advantages to nomads in do_ambush for sector-types, first skill reviews.
Lowered spin rate in do_spinkick.
Raised backflip rate from 85% on second roll to 90% max- first roll is still
85%, prac level very good.  Watch for superbs.  -first skill reviews.

Revision 1.7  2003/02/08 14:32:20  ronin
Added a check in do_hit for charmies wielding ANTI-MORTAL weapons.
May have to limit anti-mortal gear later to anti-rent as well.

Revision 1.6  2003/02/02 14:32:20  ronin
Added a higher chance of hitting circle (equal to assault/backstab)
if victim is stunned, etc.

Revision 1.5  2003/01/24 14:32:20  ronin
Added cases in do_flee for exp-loss tables, players
lose no exp under lvl 15, and lose progressively less
exp with higher level differences, if above level of mob
they're fighting.

Revision 1.4  2003/01/23 14:32:20  ronin
Had to change spell_vampiric in the weapon attacks (case 3)
to a lower % to thwart people from doing mobs without losing
any hp overall for no mana. flee k flee k.

Revision 1.3  2002/06/18 14:32:20  ronin
Adding divide_experience before raw_kill to ensure proper quest
completion.  Addition of flag within divide_experience to force
amount to 0 if required.

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct room_data *world;
extern int CHAOSMODE;
extern struct dex_app_type dex_app[];
extern struct int_app_type int_app[];

void raw_kill(struct char_data *ch);
int calc_position_damage(int position, int dam);
int MAX_PRAC(CHAR *ch);

void do_block(struct char_data *ch, char *argument, int cmd)
{
  if ((GET_CLASS(ch) != CLASS_WARRIOR) && (GET_LEVEL(ch) < LEVEL_IMM) &&
      (GET_CLASS(ch) != CLASS_PALADIN) && (GET_CLASS(ch) != CLASS_AVATAR)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if IS_SET(ch->specials.pflag, PLR_BLOCK) {
    REMOVE_BIT(ch->specials.pflag, PLR_BLOCK);
    send_to_char("You will now let your victim flee.\n\r", ch);
  }
  else {
    SET_BIT(ch->specials.pflag, PLR_BLOCK);
    send_to_char("You will now block your enemies if they flee.\n\r", ch);
  }

}


void do_hit(struct char_data *ch, char *argument, int cmd) {
  char arg[MAX_STRING_LENGTH],buf[MAX_INPUT_LENGTH];
  struct char_data *victim;
  struct obj_data *wielded = 0;
  one_argument(argument, arg);

  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if(victim == ch && IS_NPC(ch)) victim=get_mortal_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
        send_to_char("You hit yourself..OUCH!.\n\r", ch);
        act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
      } else {
        if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
          act("$N is just such a good friend, you simply can't hit $M.",FALSE, ch,0,victim,TO_CHAR);
          return;
        }
        if (IS_AFFECTED(ch, AFF_CHARM) && ch->equipment[WIELD] &&
            IS_SET(ch->equipment[WIELD]->obj_flags.extra_flags, ITEM_ANTI_MORTAL)) { /* Linerfix 020803 */
          send_to_char("Perhaps you shouldn't be using an ANTI-MORTAL weapon.\n\r", ch);
          return;
        }
        if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
          send_to_char("Behave yourself here please!\n\r", ch);
          return;
        }
        if ((GET_POS(ch)>=POSITION_STANDING) && (victim != ch->specials.fighting)) {

          /* magic weapon */
          if (((CHAOSMODE && !victim->specials.fighting) || (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC)) || (IS_NPC(victim))) && /* Chaos03 */
              ch->equipment[WIELD] &&
              ch->equipment[WIELD]->obj_flags.value[0] != 0 &&
              ch->equipment[WIELD]->obj_flags.value[0]<=20 && !number(0,3)) {
            wielded = ch->equipment[WIELD];
            switch((int)wielded->obj_flags.value[0]) {
              case 1:
                spell_blindness(30, ch, victim, 0);
                break;
              case 2:
                if(!(IS_NPC(ch))) break;
                spell_poison(30, ch, victim, 0);
                break;
              case 3:
              if(chance(90)) break; /* makes it overall 2.5% on kill */
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
              if(chance(96)) break; /* overall 1% on kill */
                spell_mana_transfer(30, victim, ch, 0);
                break;
              case 8: /* to add energy drain thing */
                break;
              case 9:
              if(chance(60)) break;
                spell_power_word_kill(GET_LEVEL(ch), ch, victim, 0);
                break;
              default:
                break;
            }
          }
          hit(ch, victim, TYPE_UNDEFINED);
          WAIT_STATE(ch, PULSE_VIOLENCE); /* HVORFOR DET?? (why??) */
        } else {
          send_to_char("You do the best you can!\n\r",ch);
          if(!IS_NPC(ch)) ch->bot.misses++;
        }
      }
    } else {
     send_to_char("They aren't here.\n\r", ch);
     ch->bot.misses++;
    }
  } else {
      send_to_char("Hit who?\n\r", ch);
  }
  if(ch->bot.misses>19) {
    sprintf(buf,"WARNING: %s has 20 kill/hit misses",GET_NAME(ch));
    log_s(buf);
    ch->bot.misses=0;
  }
}

void do_kill(struct char_data *ch, char *argument, int cmd) {
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct char_data *victim;
  struct obj_data *wielded = 0;
  struct obj_data *held = 0;

  one_argument(argument, arg);

  if (ch->equipment[WIELD]) {
    wielded = ch->equipment[WIELD];
    if (OBJ_ACTION(wielded)) {
      victim=0;
      if (*arg) victim = get_char_room_vis(ch, arg);
      if(victim) {
        sprintf(buf, "%s", OBJ_ACTION(wielded));
        act(buf, FALSE,ch, 0,victim,TO_NOTVICT);
        act(buf, FALSE,ch, 0,victim,TO_CHAR);
      } else {
        if(OBJ_ACTION_NT(wielded))
          sprintf(buf, "%s", OBJ_ACTION_NT(wielded));
        else
          sprintf(buf, "%s", OBJ_ACTION(wielded));
        act(buf, FALSE,ch, 0,0,TO_ROOM);
        act(buf, FALSE,ch, 0,0,TO_CHAR);
      }
    }
  }
  if ((ch->equipment[HOLD]) && (GET_CLASS(ch) == CLASS_NINJA)) {
    held = ch->equipment[HOLD];
   if ((held->obj_flags.type_flag == ITEM_WEAPON)
       && (OBJ_ACTION(held))) {
     victim=0;
     if (*arg) victim = get_char_room_vis(ch, arg);
     if(victim) {
       sprintf(buf, "%s", OBJ_ACTION(held));
       act(buf, FALSE,ch, 0,victim,TO_NOTVICT);
       act(buf, FALSE,ch, 0,victim,TO_CHAR);
     } else {
        if(OBJ_ACTION_NT(held))
          sprintf(buf, "%s", OBJ_ACTION_NT(held));
        else
          sprintf(buf, "%s", OBJ_ACTION(held));
        act(buf, FALSE,ch, 0,0,TO_ROOM);
        act(buf, FALSE,ch, 0,0,TO_CHAR);
     }
   }
  }

  if ((GET_LEVEL(ch) < LEVEL_IMP) || IS_NPC(ch)) {
    do_hit(ch, argument, 0);
    return;
  }

  if (!*arg) {
    send_to_char("Kill who?\n\r", ch);
  }
  else {
    if (!(victim = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\n\r", ch);

    else
      if (ch == victim)
        send_to_char("Your mother would be so sad.. :(\n\r", ch);
      else {
       act("You chop $M to pieces! Ah! The blood!", FALSE, ch, 0, victim, TO_CHAR);
       act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
       act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);
       signal_char(victim,ch,MSG_DIE,"");
       divide_experience(ch,victim,1);
       raw_kill(victim);
      }
  }
}

void do_wound(struct char_data *ch, char *argument, int cmd) {
  char usage_text[] = "Usage: wound <name> <damage>.\n\r";
  char number[MAX_INPUT_LENGTH],name[MAX_INPUT_LENGTH];
  CHAR *victim=FALSE;
  unsigned int dmg=1;

  if(IS_NPC(ch)) return;

  if((GET_LEVEL(ch)<LEVEL_SUP) && !IS_SET(ch->new.imm_flags,WIZ_CREATE)){
    send_to_char("You need a CREATE flag to use this command.\n\r",ch);
    return;
  }

  argument_interpreter(argument, name, number);
  if (!*name) {
    send_to_char(usage_text,ch);
    return;
  }

  victim = get_char_room_vis(ch, name);

  if(!victim) {
    send_to_char("Eh!!? That person isn't here you know.\n\r",ch);
    return;
  }

  sprintf(name,"WIZINFO: %s wounds %s",GET_NAME(ch),GET_NAME(victim));
  log_s(name);
  if(!*number) dmg=GET_MAX_HIT(victim)/10;
  else dmg = atoi(number);

  act("$n gestures slightly towards $N who screams in pain!\n\r",TRUE, ch, 0, victim, TO_NOTVICT);
  act("$n gestures towards you and drains away some of your lifeforce!\n\r",TRUE, ch, 0, victim ,TO_VICT);
  act("You gesture towards $N, tearing away some of $S lifeforce!\n\r",TRUE, ch, 0, victim, TO_CHAR);

  /*  damage(victim,victim,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK); */
  GET_HIT(victim) = MAX(0,GET_HIT(victim) - dmg);

}

void do_spin_kick(CHAR *ch, char *argument, int cmd)
{
  char name[MIL];
  CHAR *tmp_victim = NULL;
  CHAR *next_victim = NULL;
  int check = 0;

  if (!ch->skills) return;

  one_argument(argument, name);

  if ((GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      IS_MORTAL(ch))
  {
    send_to_char("You don't know this skill.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  check = number(1, 101) - dex_app[GET_DEX(ch)].prac_bonus;

  if (affected_by_spell(ch, SPELL_BLUR))
  {
    check -= (GET_LEVEL(ch) / 10);
  }

  if ((ch->skills[SKILL_KICK].learned < MAX_PRAC(ch) && (check > ch->skills[SKILL_KICK].learned) && (check > ch->skills[SKILL_SPIN_KICK].learned)) ||
      (check > ch->skills[SKILL_SPIN_KICK].learned))
  {
    if (!ch->specials.riding)
    {
      act("You try to do a spin-kick, but fail and hit your head on the ground.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n tries to do a spin-kick, but fails and falls on the ground.", FALSE, ch, 0, 0, TO_ROOM);

      GET_POS(ch) = POSITION_SITTING;
    }
    else
    {
      act("You try to do a spin-kick, but fail.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n tries to do a spin-kick, but fails.", FALSE, ch, 0, 0, TO_ROOM);
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  }
  else
  {
    act("Your spin-kick has generated a big whirl.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n's spin-kick has generated a big whirl.", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (tmp_victim == ch ||
          (IS_NPC(tmp_victim) && tmp_victim->specials.rider == ch) ||
          (IS_MORTAL(tmp_victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) ||
          (IS_MORTAL(tmp_victim) && GET_OPPONENT(tmp_victim) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))) continue;

      act("You have been kicked by $n.", FALSE, ch, 0, tmp_victim, TO_VICT);

      damage(ch, tmp_victim, calc_position_damage(GET_POS(tmp_victim), MIN(GET_LEVEL(ch) * 2, 60)), TYPE_UNDEFINED, DAM_SKILL);
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * 4);
  }
}

void do_backstab(struct char_data *ch, char *argument, int cmd)
{
  CHAR *victim = NULL;

#ifdef TEST_SITE
  struct char_data *victim2;
#endif

  char name[MIL];

#ifdef TEST_SITE
  char name2[256];
  char argument2[256];
#endif

  int percent = 0;
  int recover = 70; /* Linerfix */

  if (!ch->skills)
    return;

#ifdef TEST_SITE
  argument = one_argument(argument, name);
  argument = one_argument(argument, argument2);
  argument = one_argument(argument, name2);
#else
  one_argument(argument, name);
#endif

  if ((GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_AVATAR) &&
      (GET_LEVEL(ch) < LEVEL_IMM))
  {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if (!(victim = get_char_room_vis(ch, name)))
  {
    send_to_char("Backstab who?\n\r", ch);
    return;
  }

  if (victim == ch)
  {
    send_to_char("How can you sneak up on yourself?\n\r", ch);
    return;
  }

  if (!ch->equipment[WIELD])
  {
    send_to_char("You need to wield a weapon for it to be successful.\n\r",ch);
    return;
  }

  if (ch->equipment[WIELD]->obj_flags.value[3] != 11)
  {
    send_to_char("Only piercing weapons can be used for backstabbing.\n\r",ch);
    return;
  }

  if (victim->specials.fighting && !check_subclass(ch, SC_INFIDEL, 1))
  {
    send_to_char("You can't backstab a fighting person, they're too alert!\n\r", ch);
    return;
  }

#ifdef TEST_SITE
  if (!is_abbrev(argument2, "blame") || (name2[0] == '\0'))
  {
    victim2 = ch;
  }
  else if (!(victim2 = get_char_room_vis(ch, name2)))
  {
    send_to_char("Blame who?\n\r", ch);
    return;
  }

  if (!IS_NPC(victim))
  {
    /* don't allow them to blame another for backstabing a player */
    victim2 = ch;
  }
  else
  {
    /* if the blame victim is a mob, 1/10 the success rate */
    percent=number(1,IS_NPC(victim2) ? 1510 : 151)-int_app[GET_INT(ch)].conc+int_app[GET_INT(victim)].conc;

    if (percent > ch->skills[SKILL_BLAME].learned)
    {
      //Failed blame
      victim2 = ch;
    }
  }
#endif

  percent = number(1 , 151) - dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */

  if (affected_by_spell(ch, AFF_HIDE) ||
      affected_by_spell(ch, AFF_SNEAK) ||
      affected_by_spell(ch, SPELL_IMP_INVISIBLE))
  {
    percent -= 5;
  }

  if ((AWAKE(victim) && (percent > ch->skills[SKILL_BACKSTAB].learned)) ||
       IS_SET(victim->specials.immune, IMMUNE_BACKSTAB))
  {
    damage(ch, victim, 0, SKILL_BACKSTAB, DAM_NO_BLOCK);
#ifdef TEST_SITE
    victim2 = chance(50) ? ch : victim2;
    set_fighting(victim, victim2);
    GET_POS(victim)=POSITION_FIGHTING;
#endif
    recover=number(50,75);                               /* Linerfix */
    if(check_subclass(ch,SC_BANDIT,4) && chance(recover))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE);
    }
    else
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    if (victim->specials.fighting && check_subclass(ch, SC_INFIDEL, 1))
    {
      hit(ch, victim, SKILL_ASSASSINATE);
    }
    else
    {
      hit(ch, victim, SKILL_BACKSTAB);
    }
#ifdef TEST_SITE
    set_fighting(victim, victim2);
#endif
    recover=number(50,75);                               /* Linerfix */
    if(check_subclass(ch,SC_BANDIT,4) && chance(recover))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
    else
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
  }
}

void do_ambush(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int percent;

  if(!ch->skills)
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
    send_to_char("You need to wield a weapon for it to succeed.\n\r",ch);
    return;
  }

  if (victim->specials.fighting) {
    send_to_char("You can't ambush a fighting person, too alert!\n\r", ch);
    return;
  }

  percent=number(1,151)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */

  if(world[CHAR_REAL_ROOM(ch)].sector_type == SECT_FIELD) percent = (percent-2); /* field advantage */
  if(world[CHAR_REAL_ROOM(ch)].sector_type == SECT_HILLS) percent = (percent-3); /* hills advantage */
  if(world[CHAR_REAL_ROOM(ch)].sector_type == SECT_MOUNTAIN) percent = (percent-5); /* mtn advantage */
  if(world[CHAR_REAL_ROOM(ch)].sector_type == SECT_FOREST) percent = (percent-7); /* forest advantage */

  if( (AWAKE(victim) && (percent > ch->skills[SKILL_AMBUSH].learned)) ||
      IS_SET(victim->specials.immune,IMMUNE_AMBUSH) ) {
      act("You try to ambush $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
      act("$N tries to ambush you, but fails.", FALSE, victim, 0, ch, TO_CHAR);
      act("$n tries to ambush $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_AMBUSH,DAM_NO_BLOCK);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
  }
  else {
    hit(ch,victim,SKILL_AMBUSH);
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
  }


}

void do_assault(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int modifier = 0;
  bool assault = FALSE;
  bool dual_assault = FALSE;

  if (!ch->skills) return;

  one_argument(arg, name);

  if (GET_CLASS(ch) != CLASS_NINJA &&
      GET_CLASS(ch) != CLASS_COMMANDO &&
      !IS_IMMORTAL(ch))
  {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if (!(victim = get_char_room_vis(ch, name)))
  {
    send_to_char("Assault who?\n\r", ch);
    return;
  }

  if (victim == ch)
  {
    send_to_char("How can you sneak up on yourself?\n\r", ch);
    return;
  }

  if (IS_MORTAL(ch) &&
      IS_IMMORTAL(victim))
  {
    send_to_char("It's not a good idea to attack an immortal!\n\r", ch);
    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!EQ(ch, WIELD) || !IS_WEAPON(EQ(ch, WIELD)))
  {
    send_to_char("You need to wield a weapon for your assault to succeed.\n\r", ch);
    return;
  }

  modifier -= dex_app[GET_DEX(ch)].prac_bonus;

  if (affected_by_spell(ch, SPELL_BLUR))
  {
    modifier -= GET_LEVEL(ch) / 5;
  }

  if (!IS_SET(victim->specials.immune, IMMUNE_ASSAULT))
  {
    if (AWAKE(victim))
    {
      if (number(1, 151) + modifier <= ch->skills[SKILL_ASSAULT].learned)
      {
        assault = TRUE;
      }

      /* Dual Assault */
      if (check_subclass(ch, SC_RONIN, 1))
      {
        if (number(1, 151) + modifier <= ch->skills[SKILL_ASSAULT].learned)
        {
          dual_assault = TRUE;
        }
      }
    }
    else
    {
      assault = TRUE;

      /* Dual Assault */
      if (check_subclass(ch, SC_RONIN, 1))
      {
        dual_assault = TRUE;
      }
    }
  }

  if (!assault)
  {
    act("You try to assault $N, but fail and $N fights back.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to assault you, but fails and you fight back.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to assault $N, but fails and $N fights back.", FALSE, ch, 0, victim, TO_NOTVICT);
  }
  else
  {
    hit(ch, victim, SKILL_ASSAULT);
  }

  /* Dual Assault */
  if (check_subclass(ch, SC_RONIN, 1))
  {
    if (!dual_assault)
    {
      act("You try to assault $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n tries to assault you, but fails.", FALSE, ch, 0, victim, TO_VICT);
      act("$n tries to assault $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    else
    {
      /* A bit of a hack, but this bypasses the check for the 2nd weapon and Mystic Swiftness. */
      qhit(ch, victim, SKILL_ASSAULT);
    }
  }

  if (!assault)
  {
    hit(victim, ch, TYPE_UNDEFINED);
  }

  if (!IS_IMPLEMENTOR(ch))
  {
    if (assault || dual_assault)
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
    else
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
}

void do_circle(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int percent;
  int recover=70; /* Linerfix */

  if(!ch->skills)
    return;

  one_argument(argument, name);

  if ((GET_CLASS(ch) != CLASS_THIEF) && (GET_CLASS(ch) != CLASS_AVATAR) && (GET_LEVEL(ch) < LEVEL_IMM)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Circle who?\n\r", ch);
     return;
   }
  }

  if (victim == ch) {
    send_to_char("How can you sneak up on yourself?\n\r", ch);
    return;
  }

  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon, to make it a success.\n\r",ch);
    return;
  }

  if (ch->equipment[WIELD]->obj_flags.value[3] != 11) {
    send_to_char("Only piercing weapons can be used for that.\n\r",ch);
    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }
  
  percent=number(1,190)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */
  if(GET_POS(victim) < POSITION_RESTING){ /* if victim is sleep,stunned,incap,mortalw,or dead */
    percent=(percent-49); /* same chance as landing an assault or backstab */
  }
  
  if (GET_CLASS(ch)==CLASS_THIEF){
     percent -= (GET_LEVEL(ch)/2);
  }    

  if ((percent > ch->skills[SKILL_CIRCLE].learned) ||
      IS_SET(victim->specials.immune2,IMMUNE_CIRCLE)) {
    damage(ch, victim, 0, SKILL_CIRCLE,DAM_NO_BLOCK);
    act("$n slips quietly into the shadows, but $N notices as $e appears behind $M.",
      FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n disappears from sight, but you notice as $e appears behind you.",
      FALSE, ch, 0, victim, TO_VICT);
    act("You slip quietly into the shadows, but $N notices as you appear behind $M.",
      FALSE, ch, 0, victim, TO_CHAR);
    recover=number(50,75);                                  /* Linerfix */
    if(check_subclass(ch,SC_BANDIT,4) && chance(recover)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
    }
    else {
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }
  }
  else {
    act("$n vanishes into the shadows, suddenly appearing behind $N.",
      FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n disappears into the shadows, vanishing completely from sight.",
      FALSE, ch, 0, victim, TO_VICT);
    act("You slip into the shadows and vanish, suddenly appearing behind $N.",
      FALSE, ch, 0, victim, TO_CHAR);
    hit(ch,victim,SKILL_CIRCLE);
    recover=number(50,75);                                  /* Linerfix */
    if(check_subclass(ch,SC_BANDIT,4) && chance(recover)) {
      WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }
    else {
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    }
  }
}


void do_order(struct char_data *ch, char *argument, int cmd)
{
  char name[100], message[256];
  char buf[256];
  bool found = FALSE;
  int org_room;
  struct char_data *victim;
  struct follow_type *k,*temp;

  half_chop(argument, name,100,message,256);

  if (!*name || !*message)
    send_to_char("Order who to do what?\n\r", ch);
  else if (!(victim = get_char_room_vis(ch, name)) &&
        str_cmp("follower", name) && str_cmp("followers", name))
    send_to_char("That person isn't here.\n\r", ch);
  else if (ch == victim)
    send_to_char("You obviously suffer from schizophrenia.\n\r", ch);
  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not approve of you giving orders.\n\r",ch);
      return;
    }
    if (victim) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, victim, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);

      if( ( (victim->master==ch && IS_AFFECTED(victim, AFF_CHARM)) ||
            (victim->specials.rider==ch) ) && IS_NPC(victim)) {
        send_to_char("Ok.\n\r", ch);
        if(strncmp(message,"flex",4) && (!strncmp(message,"fl",2) || !strncmp(message,"fle",3) || !strncmp(message,"flee",4))) {
          act("The thought of running away makes $n burst into tears.",0,victim,0,0,TO_ROOM);
          act("The thought of running away makes you burst into tears.",0,victim,0,0,TO_CHAR);
          return;
        }
        command_interpreter(victim, message);
      }
      else {
        act("$n has an indifferent look.", FALSE, victim, 0,0, TO_ROOM);
      }
    } else {  /* This is order "followers" */
        sprintf(buf, "$n issues the order '%s'.", message);
     act(buf, FALSE, ch, 0, victim, TO_ROOM);

     org_room = CHAR_REAL_ROOM(ch);

     for (k = ch->followers; k; k = temp) {
         temp=k->next; /* added temp - Ranger June 96 */
      if (org_room == CHAR_REAL_ROOM(k->follower))
       if (IS_AFFECTED(k->follower, AFF_CHARM)) {
         found = TRUE;
         if(strncmp(message,"flex",4) && (!strncmp(message,"fl",2) || !strncmp(message,"fle",3) || !strncmp(message,"flee",4))) {
          act("The thought of running away makes $n burst into tears.",0,k->follower,0,0,TO_ROOM);
          act("The thought of running away makes you burst into tears.",0,k->follower,0,0,TO_CHAR);
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
  struct char_data *victim=NULL,*vict,*vict_n;
  int org_room;
  bool rider,mount;
  OBJ *wall=0;

  void gain_exp(struct char_data *ch, int gain);
  int special(struct char_data *ch, int cmd, char *arg);

  rider=FALSE;
  mount=FALSE;
  /* Added check for when rider flees, mount flees, mount flees rider flees
     Ranger April 96  Whole flee thing changed */

  if(IS_AFFECTED(ch, AFF_PARALYSIS) || (IS_AFFECTED(ch, AFF_HOLD) && chance(25)) ) {
    act("$n tries to flee but seems to be unable to move.",TRUE,ch,0,0,TO_ROOM);
    act("You try to flee but are paralyzed and can't move!",TRUE,ch,0,0,TO_CHAR);
    return;
  }

  if(CHAOSMODE && chance(25) && ch->specials.fighting && (!check_subclass(ch->specials.fighting,SC_WARLORD,1))) {  /* Chaos03 */
    act("$n tries to flee but fails.",TRUE,ch,0,0,TO_ROOM);
    act("You try to flee but fail!",TRUE,ch,0,0,TO_CHAR);
    return;
  }


  if(affected_by_spell(ch, SKILL_BERSERK) ||
     affected_by_spell(ch, SKILL_FRENZY) ||
     affected_by_spell(ch, SKILL_HOSTILE)) {
    act("$n tries to flee but fails.",TRUE,ch,0,0,TO_ROOM);
    act("You try to flee but fail!",TRUE,ch,0,0,TO_CHAR);
    return;
  }

  if((wall=get_obj_room(WALL_THORNS,CHAR_VIRTUAL_ROOM(ch)))) {
    send_to_char("The wall of thorns blocks your way. Ouch!\n\r",ch);
    damage(ch,ch,30,TYPE_UNDEFINED,DAM_NO_BLOCK_NO_FLEE);
    return;
  }

  if(!(ch->specials.fighting)) {
    for (i=0; i<6; i++) {
      attempt = number(0, 5);  /* Select a random direction */
      if(CAN_GO(ch, attempt) &&
         !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, DEATH)) {
        act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
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
          send_to_char("You flee head over heels.\n\r", ch);
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
          if (!IS_NPC(ch) && IS_SET(world[org_room].room_flags, CHAOTIC)) {
            drain_mana_hit_mv(ch,ch,GET_MANA(ch)/10,GET_HIT(ch)/10,GET_MOVE(ch)/10,FALSE,FALSE,FALSE);
            send_to_char("The gods of Chaos rip some of your lifeforce from you!\n\r",ch);
          }
          return;
        } else {
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
  for (i=0; i<6; i++) {
    attempt = number(0, 5);
    if(CAN_GO(ch, attempt) &&
       !IS_SET(world[EXIT(ch, attempt)->to_room_r].room_flags, DEATH)) {
      if(!IS_NPC(ch->specials.fighting) && !ch->specials.rider &&
         IS_SET(ch->specials.fighting->specials.pflag, PLR_BLOCK) &&
         ( (number(1, 101) < ch->specials.fighting->skills[SKILL_BLOCK].learned) ||
           (check_subclass(ch->specials.fighting,SC_WARLORD,1) && chance(90)) ) ) {
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

/* Liner Jan 03, adding flee exp loss tables to vary exp loss depending on level
differences in ch and ch->specials.fighting */

      if((die = do_simple_move(ch, attempt, FALSE,1))== 1) {
        /* The escape has succeeded */
        if(ch->specials.fighting) { /* Another check */
          if(GET_LEVEL(ch) > GET_LEVEL(ch->specials.fighting)) {
            diff = (GET_LEVEL(ch) - GET_LEVEL(ch->specials.fighting));
            diff = diff/10; /* gives integer from 0-5 of difference in levels */
            switch(diff) {
              case 0: /* 1-4 levels below, lose 1/5 normal */
                loose = (GET_EXP(ch->specials.fighting)/50);
                break;
              case 1: /* 5-14 levels below, lose 1/10 normal */
                loose = (GET_EXP(ch->specials.fighting)/100);
                break;
              case 2: /* 15-24 levels below, lose 1/50 normal */
                loose = (GET_EXP(ch->specials.fighting)/500);
                break;
              case 3: /* 25-34 levels below, lose 1/100 normal */
                loose = (GET_EXP(ch->specials.fighting)/1000);
                break;
              case 4: /* 35-44 levels below, lose 1/500 normal */
                loose = (GET_EXP(ch->specials.fighting)/5000);
                break;
              case 5: /* 45-49 levels below, lose 1/1000 normal */
                loose = (GET_EXP(ch->specials.fighting)/10000);
                break;
              default:
                loose = 0;
                break;
            }
          }
          else loose = (GET_EXP(ch->specials.fighting)/10);
          if(IS_SET(world[org_room].room_flags, CHAOTIC)) loose = 0;
          if(GET_LEVEL(ch) < 15) loose = 0;   /* flee below level 15, no exp loss */

          if(!IS_NPC(ch) && IS_NPC(ch->specials.fighting)) {
            if(loose>(GET_EXP(ch)/2)) { /* if loss is greater than half of ch's xp */
              loose=MIN(loose,GET_EXP(ch)/2);
              gain_exp(ch, -loose); /* loss is now no greater than half of ch's xp */
            }
            else {
              gain_exp(ch, -loose); /* loss is less than half of ch's xp, no worries */
            }
            gain_exp(ch->specials.fighting, loose); /* exp is now retainable */
          }

        /* Insert later when using hunting system       */
        /* ch->specials.fighting->specials.hunting = ch */

          for(vict=world[org_room].people;vict;vict=vict_n) {
            vict_n=vict->next_in_room;
            if(vict->specials.fighting == ch)
             stop_fighting(vict);
          }
          stop_fighting(ch);
        }
        send_to_char("You flee head over heels.\n\r", ch);

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
        if (!die) act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
        return;
      }
    }
  } /* for */
  /* No exits was found */
  send_to_char("PANIC! You couldn't escape!\n\r", ch);
}

void do_pummel(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!ch->skills) return;

  if (GET_CLASS(ch) != CLASS_NINJA &&
      GET_CLASS(ch) != CLASS_PALADIN &&
      GET_CLASS(ch) != CLASS_ANTI_PALADIN &&
      GET_CLASS(ch) != CLASS_COMMANDO &&
      IS_MORTAL(ch))
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
      send_to_char("Pummel who?\n\r", ch);
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

  if (!EQ(ch, WIELD) || !IS_WEAPON(EQ(ch, WIELD)))
  {
    send_to_char("You need to wield a weapon for your pummel to succeed.\n\r", ch);
    return;
  }

  check = number(1, 121) - dex_app[GET_DEX(ch)].prac_bonus;

  if (affected_by_spell(ch, SPELL_BLUR))
  {
    check -= (GET_LEVEL(ch) / 10);
  }

  if ((ch->skills[SKILL_BASH].learned < MAX_PRAC(ch) && (check > ch->skills[SKILL_BASH].learned) && (check > ch->skills[SKILL_PUMMEL].learned)) ||
      check > ch->skills[SKILL_PUMMEL].learned ||
      number(1, 18) > GET_DEX(ch))
  {
    act("You try to pummel $N, but miss.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n tried to pummel you, but missed.", FALSE, ch, NULL, victim, TO_VICT);
    act("$n tried to pummel $N, but missed.", FALSE, ch, NULL, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_PUMMEL, DAM_NO_BLOCK);

    if (!IS_IMPLEMENTOR(ch))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    if ((IS_NPC(victim) && IS_SET(victim->specials.immune, IMMUNE_PUMMEL)) ||
        (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL)))
    {
      act("You pummel $N, but your pummel has no effect!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n pummels you, but $s pummel has no effect!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n pummels $N, but $s pummel has no effect!", FALSE, ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, 0, SKILL_PUMMEL, DAM_NO_BLOCK);
    }
    else
    {
      if ((GET_POS(victim) == POSITION_STUNNED) ||
          (GET_POS(victim) == POSITION_RESTING) ||
          (GET_POS(victim) == POSITION_SITTING))
      {
        set_pos = POSITION_INCAP;
      }
      else if (GET_POS(victim) > POSITION_STUNNED)
      {
        set_pos = POSITION_STUNNED;
      }
      else
      {
        set_pos = GET_POS(victim);
      }

      act("You pummel $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_CHAR);
      act("$n pummels you, and you are stunned now!", FALSE, ch, NULL, victim, TO_VICT);
      act("$n pummels $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_PUMMEL, DAM_NO_BLOCK);

      if (ch->skills[SKILL_PUMMEL].learned < 80)
      {
        ch->skills[SKILL_PUMMEL].learned = MIN(ch->skills[SKILL_PUMMEL].learned + 2, 80);
      }

      if (CHAR_REAL_ROOM(victim) != NOWHERE &&
          !IS_IMPLEMENTOR(victim))
      {
        GET_POS(victim) = set_pos;

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

    if (!IS_IMPLEMENTOR(ch))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
}

void do_bash(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;
  int set_pos = 0;

  if (!ch->skills) return;

  if (GET_CLASS(ch) == CLASS_CLERIC &&
      GET_LEVEL(ch) < 35 &&
      IS_MORTAL(ch))
  {
    send_to_char("You don't know this skill yet.\n\r", ch);
    return;
  }

  if (GET_CLASS(ch) != CLASS_CLERIC &&
      GET_CLASS(ch) != CLASS_WARRIOR &&
      GET_CLASS(ch) != CLASS_NINJA &&
      GET_CLASS(ch) != CLASS_PALADIN &&
      GET_CLASS(ch) != CLASS_ANTI_PALADIN &&
      GET_CLASS(ch) != CLASS_COMMANDO &&
      IS_MORTAL(ch))
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
      send_to_char("Bash who?\n\r", ch);
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

  if (!EQ(ch, WIELD) || !IS_WEAPON(EQ(ch, WIELD)))
  {
    send_to_char("You need to wield a weapon for your bash to succeed.\n\r", ch);
    return;
  }

  check = number(1, 101) - dex_app[GET_DEX(ch)].prac_bonus;

  if (affected_by_spell(ch, SPELL_BLUR))
  {
    check -= (GET_LEVEL(ch) / 10);
  }

  if (check > ch->skills[SKILL_BASH].learned)
  {
    damage(ch, victim, 0, SKILL_BASH, DAM_NO_BLOCK);

    if (!IS_IMPLEMENTOR(victim))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    if ((IS_NPC(victim) && IS_SET(victim->specials.immune, IMMUNE_PUMMEL)) ||
        (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL)))
    {
      damage(ch, victim, 0, SKILL_BASH, DAM_NO_BLOCK);
    }
    else
    {
      if (GET_POS(victim) == POSITION_STUNNED)
      {
        set_pos = POSITION_INCAP;
      }
      else if ((GET_POS(victim) == POSITION_RESTING) ||
               (GET_POS(victim) == POSITION_SITTING))
      {
        set_pos = POSITION_STUNNED;
      }
      else if (GET_POS(victim) > POSITION_RESTING)
      {
        set_pos = POSITION_RESTING;
      }
      else
      {
        set_pos = GET_POS(victim);
      }

      damage(ch, victim, calc_position_damage(GET_POS(victim), number(1, GET_LEVEL(ch))), SKILL_BASH, DAM_NO_BLOCK);

      if (ch->skills[SKILL_BASH].learned < 80)
      {
        ch->skills[SKILL_BASH].learned = MIN(ch->skills[SKILL_BASH].learned + 2, 80);
      }

      if (CHAR_REAL_ROOM(victim) != NOWHERE &&
          !IS_IMPLEMENTOR(victim))
      {
        GET_POS(victim) = set_pos;

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

void do_punch(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check = 0;

  if (!ch->skills) return;

  one_argument(arg, name);

  if ((GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_AVATAR) &&
      IS_MORTAL(ch))
  {
    send_to_char("You better leave all the martial arts to the fighters.\n\r", ch);
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
      send_to_char("Punch who?\n\r", ch);
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

  check = number(1, 101) - dex_app[GET_DEX(ch)].prac_bonus - (GET_LEVEL(ch) / 20);

  if (check > ch->skills[SKILL_PUNCH].learned)
  {
    damage(ch, victim, 0, SKILL_PUNCH, DAM_NO_BLOCK);

    if (!IS_IMPLEMENTOR(ch))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    if ((IS_NPC(victim) && IS_SET(victim->specials.immune, IMMUNE_PUNCH)) ||
        (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL)))
    {
      damage(ch, victim, 0, SKILL_PUNCH, DAM_NO_BLOCK);
    }
    else
    {
      damage(ch, victim, calc_position_damage(GET_POS(victim), GET_LEVEL(ch) * 2), SKILL_PUNCH, DAM_NO_BLOCK);

      if (ch->skills[SKILL_PUNCH].learned < 80)
      {
        ch->skills[SKILL_PUNCH].learned = MIN(ch->skills[SKILL_PUNCH].learned + 2, 80);
      }

      if (CHAR_REAL_ROOM(victim) != NOWHERE &&
          !IS_IMPLEMENTOR(victim))
      {
        if (GET_POS(victim) > POSITION_SITTING)
        {
          GET_POS(victim) = POSITION_SITTING;
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

void do_rescue(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim, *tmp_ch;
  int percent;
  char victim_name[240];

  if(!ch->skills)
    return;

  one_argument(argument, victim_name);

  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Who do you want to rescue?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("What about fleeing instead?\n\r", ch);
    return;
  }

  if (ch->specials.fighting == victim) {
    send_to_char("How can you rescue someone you are trying to kill?\n\r",ch);
    return;
  }

  for (tmp_ch=world[CHAR_REAL_ROOM(ch)].people; tmp_ch &&
       (tmp_ch->specials.fighting != victim);
       tmp_ch=tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  if(affected_by_spell(ch, SKILL_BERSERK) ||
     affected_by_spell(ch, SKILL_FRENZY) ||
     affected_by_spell(ch, SKILL_HOSTILE) || (CHAOSMODE) ) {
    send_to_char("You fail the rescue.\n\r", ch);
    return;
  }

  percent=number(1,101); /* 101% is a complete failure */
  if(GET_LEVEL(victim)<16 && IS_SET(ch->specials.pflag,PLR_DEPUTY))
    percent=0;

  if(GET_LEVEL(victim)>15 && GET_CLASS(ch)!=CLASS_COMMANDO &&
     GET_CLASS(ch)!=CLASS_NOMAD && GET_CLASS(ch)!=CLASS_WARRIOR &&
     GET_CLASS(ch)!=CLASS_PALADIN)
    percent=101;

  if (percent > ch->skills[SKILL_RESCUE].learned) {
    send_to_char("You fail the rescue.\n\r", ch);
    return;
  }
  send_to_char("Banzai! To the rescue...\n\r", ch);
  act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
  act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);

  if ((ch->skills[SKILL_RESCUE].learned < 80) && percent)
    ch->skills[SKILL_RESCUE].learned += 2;

  if (victim->specials.fighting == tmp_ch)
    stop_fighting(victim);
  if (tmp_ch->specials.fighting)
    stop_fighting(tmp_ch);
  if (ch->specials.fighting)
    stop_fighting(ch);
  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);

  WAIT_STATE(victim, 2*PULSE_VIOLENCE);

}


void do_assist(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim, *tmp_ch;
  char victim_name[240];

  if(CHAOSMODE) {
    send_to_char("Assist??  All you can think about is KILLING!!\n\r",ch);
    return;
  }

  one_argument(argument, victim_name);

  if (GET_POS(ch) == POSITION_FIGHTING ||
      ch->specials.fighting) {
    send_to_char("You are fighting already!\n\r", ch);
    return;
  }

  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Who do you want to assist?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("What about fleeing instead?\n\r", ch);
    return;
  }

  for (tmp_ch=world[CHAR_REAL_ROOM(ch)].people; tmp_ch &&
       (tmp_ch->specials.fighting != victim);
       tmp_ch=tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }


  send_to_char("You join the fight!\n\r", ch);
  act("$N assists you!", FALSE, victim, 0, ch, TO_CHAR);
  act("$n assists $N.", FALSE, ch, 0, victim, TO_NOTVICT);

  set_fighting(ch, tmp_ch);
}

void do_kick(CHAR *ch, char *arg, int cmd)
{
  char name[MIL];
  CHAR *victim = NULL;
  int check;

  if (!ch->skills) return;

  one_argument(arg, name);

  if ((GET_CLASS(ch) != CLASS_WARRIOR) &&
      (GET_CLASS(ch) != CLASS_PALADIN) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      (GET_CLASS(ch) != CLASS_COMMANDO) &&
      (GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      IS_MORTAL(ch))
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
      send_to_char("Kick who?\n\r", ch);
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

  check = ((10 - (GET_AC(victim) / 10)) * 2) + number(1, 101) - dex_app[GET_DEX(ch)].prac_bonus;

  if (affected_by_spell(ch, SPELL_BLUR))
  {
    check -= (GET_LEVEL(ch) / 10);
  }

  if (check > ch->skills[SKILL_KICK].learned)
  {
    damage(ch, victim, 0, SKILL_KICK, DAM_NO_BLOCK);

    if (!IS_IMPLEMENTOR(victim))
    {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
  }
  else
  {
    if ((IS_NPC(victim) && IS_SET(victim->specials.immune, IMMUNE_KICK)) ||
        (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL)))
    {
      damage(ch, victim, 0, SKILL_KICK, DAM_NO_BLOCK);
    }
    else
    {
      damage(ch, victim, calc_position_damage(GET_POS(victim), MIN(GET_LEVEL(ch), 30) * 2), SKILL_KICK, DAM_NO_BLOCK);

      if (ch->skills[SKILL_KICK].learned < 80)
      {
        ch->skills[SKILL_KICK].learned = MIN(ch->skills[SKILL_KICK].learned + 2, 80);
      }

      if (CHAR_REAL_ROOM(victim) != NOWHERE &&
          !IS_IMPLEMENTOR(victim))
      {
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
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
  }
}

void do_disarm(struct char_data *ch, char *argument, int cmd)
{
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
   } else {
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

  percent= number(1,200)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */

  if(GET_LEVEL(victim)<GET_LEVEL(ch))
    percent = percent - (GET_LEVEL(ch)-GET_LEVEL(victim));

  if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    percent = number(1, 800);

  if (V_OBJ(wield)==11523 || (ch->skills ? (percent > ch->skills[SKILL_DISARM].learned) ||
      (IS_SET(victim->specials.act, ACT_ARM) && IS_NPC(victim)) ||
      ((GET_LEVEL(victim)-GET_LEVEL(ch))>5) || GET_LEVEL(victim)>30 : percent > 100) ) {
    act("You tried to kick off $N's weapon, but failed!",
     FALSE, ch, 0, victim, TO_CHAR);
    act("$N tried to kick off your weapon, but failed!",
     FALSE, victim, 0, ch, TO_CHAR);
    act("$n tried to kick off $N's weapon, but failed!",
     FALSE, ch, 0, victim, TO_NOTVICT);
    hit(ch, victim, TYPE_UNDEFINED);
  } else {
     act("Your beautiful side-kick has kicked off $N's weapon!",
         FALSE, ch, 0,victim, TO_CHAR);
     act("$N kicked off your weapon!",
         FALSE, victim, 0, ch, TO_CHAR);
     act("$n kicked off $N's weapon by a beautiful side-kick!",
         FALSE, ch, 0, victim, TO_NOTVICT);

     if (ch->skills ? ch->skills[SKILL_DISARM].learned < 85 : FALSE)
       ch->skills[SKILL_DISARM].learned += 2;

     unequip_char(victim, WIELD);
     if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC)) obj_to_char(wield, victim);
     else {
      log_f("WIZLOG: %s disarms %s's %s (Room %d).",GET_NAME(ch),GET_NAME(victim),OBJ_SHORT(wield),world[CHAR_REAL_ROOM(victim)].number);
      obj_to_room(wield, CHAR_REAL_ROOM(victim));
      wield->log=1;
     }
     save_char(victim,NOWHERE);
     hit(ch, victim, TYPE_UNDEFINED);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

/* Added disembowel - Ranger Oct 96 */
void do_disembowel(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int dam;
  int percent,percent2;

  if(!ch->skills) return;

  one_argument(argument, name);

  /* First some basic checks */
  if ((GET_CLASS(ch) != CLASS_NOMAD) && (GET_LEVEL(ch) < LEVEL_IMM)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if(GET_LEVEL(ch)<20) {
    send_to_char("You are not a high enough level.\n\r",ch);
    return;
  }

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Disembowel who?\n\r", ch);
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

  if ((ch->equipment[WIELD]->obj_flags.value[3] != 3) && (ch->equipment[WIELD]->obj_flags.value[3] != 8)) {
    send_to_char("Only slashing or clawing weapons can be used to disembowel.\n\r",ch);
    return;
  }

  if(!IS_NPC(victim)&&!IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, CHAOTIC) ) {
    send_to_char("You can't use this on other players.\n\r",ch);
    return;
  }

  /* Damage to PC if fail */
  dam=2*GET_LEVEL(victim)+20;

  /* Skill roll + DEX check + 50/50 roll */
  percent=number(1,101)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */
  percent2=GET_LEVEL(ch)+25;
  percent2=MIN(percent2,55);
  /* Easy rank addition here - for example for a Nomad
  rank, just add an amount to percent2 to increase chance
  of a disembowel - Ranger */
  if ((percent > ch->skills[SKILL_DISEMBOWEL].learned) ||
      (number(1,18) > GET_DEX(ch)) || number(0,100)>percent2 ) {
    act("You try to disembowel $N, but stumble over your own feet!", FALSE, ch, 0, victim, TO_CHAR);
    act("$N tries to disembowel you, but stumbles over $S own feet!", FALSE, victim, 0, ch, TO_CHAR);
    act("$n tries to disembowel $N, but stumbles over $s own feet!", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$N avoids your pitiful attack and strikes back at you!", FALSE, ch, 0, victim, TO_CHAR);
    act("You avoid $N's pitiful attack and strike back at $M!", FALSE, victim, 0, ch, TO_CHAR);
    act("$N avoids $n's pitiful attack and strikes back at $m!", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(victim, ch, dam, TYPE_UNDEFINED,DAM_SKILL);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }

  /* Skill roll successful, now for some more checks */

  /* Check for ch==tank or no tank */
  if(victim->specials.fighting && victim->specials.fighting!=ch) {
    act("$N avoids your pitiful attack and strikes back at you!", FALSE, ch, 0, victim, TO_CHAR);
    act("You avoid $N's pitiful attack and strike back at $M!", FALSE, victim, 0, ch, TO_CHAR);
    act("$N avoids $n's pitiful attack and strikes back at $m!", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(victim, ch, dam, TYPE_UNDEFINED,DAM_SKILL);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }

  /* Check for immunity and make sure current victim hps<10% of max */
  if((IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_DISEMBOWEL)) ||
     (GET_HIT(victim)>GET_MAX_HIT(victim)/10) ){
    act("$N avoids your pitiful attack and strikes back at you!", FALSE, ch, 0, victim, TO_CHAR);
    act("You avoid $N's pitiful attack and strike back at $M!", FALSE, victim, 0, ch, TO_CHAR);
    act("$N avoids $n's pitiful attack and strikes back at $m!", FALSE, ch, 0, victim, TO_NOTVICT);
    damage(victim, ch, dam, TYPE_UNDEFINED,DAM_SKILL);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }

  act("Your deft slash causes $N's innards to spill out!", FALSE, ch, 0, victim, TO_CHAR);
  act("$N's deft slash causes your innards to spill out!", FALSE, victim, 0, ch, TO_CHAR);
  act("$n's deft slash causes $N's innards to spill out!", FALSE, ch, 0, victim, TO_NOTVICT);
  dam=GET_MAX_HIT(victim)*100/7;
  if(IS_NPC(victim)) SET_BIT(victim->specials.immune,IMMUNE_DISEMBOWEL);
  damage(ch,victim,dam,SKILL_DISEMBOWEL,DAM_SKILL);
  WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_backflip(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  char name[256];
  int percent,percent2;

  if(!ch->skills) return;

  one_argument(argument, name);

  /* First some basic checks */
  if ((GET_CLASS(ch) != CLASS_BARD) && (GET_LEVEL(ch) < LEVEL_IMM)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if(GET_LEVEL(ch)<20) {
    send_to_char("You are not a high enough level.\n\r",ch);
    return;
  }

  if (!(victim = get_char_room_vis(ch, name))) {
   if (ch->specials.fighting) {
     victim = ch->specials.fighting;
   } else {
     send_to_char("Who do you want to flip over?\n\r", ch);
     return;
   }
  }

  if (victim == ch) {
    send_to_char("This could prove very interesting...\n\r", ch);
    return;
  }

  /* Skill roll + DEX check + level roll */
  percent=number(1,101)-dex_app[GET_DEX(ch)].prac_bonus; /* 101% is a complete failure */
  percent2=GET_LEVEL(ch)+50;
  percent2=MIN(percent2,90); /* upped from 85% to 90% max on 2nd roll */
  /* Easy rank addition here - for example for a Bard
  rank, just add an amount to percent2 to increase chance
  of a backflip - Ranger */
  if ((percent > ch->skills[SKILL_BACKFLIP].learned) ||
      (number(1,18) > GET_DEX(ch)) || number(1,100)>percent2 ) {
    act("As you spring, $N knocks you down!",0,ch,0,victim,TO_CHAR);
    act("$n tries to spring over $N, but is knocked hard on $s back.",0,ch,0,victim,TO_NOTVICT);
    act("As $n springs, you knock $m down!",1,ch,0,victim,TO_VICT);
    WAIT_STATE(ch,PULSE_VIOLENCE*2);
    return;
  }

  /* Skill roll successful */

  if(ch->skills[SKILL_BACKFLIP].learned < 80) ch->skills[SKILL_BACKFLIP].learned += 1;
  act("With a mighty leap, you spring over $N!",0,ch,0,victim,TO_CHAR);
  act("$n crouches low and springs over you!",0,ch,0,victim,TO_VICT);
  act("With a mighty leap, $n springs over $N and lands behind $M!",0,ch,0,victim,TO_NOTVICT);
  hit(ch, victim, SKILL_BACKFLIP);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_cover(CHAR *ch, char *argument, int cmd) {
  struct affected_type_5 af;
  if(!ch->skills) return;

  if(GET_CLASS(ch) != CLASS_NOMAD && GET_LEVEL(ch) < 50) {
    send_to_char("You do not have this skill.\n\r",ch);
    return;
  }
  if(ch->specials.fighting) {
    send_to_char("You can't cover yourself while fighting.\n\r",ch);
    return;
  }
  if(!affected_by_spell(ch, SKILL_COVER)) {
    if(number(0,120)>ch->skills[SKILL_COVER].learned) {
      send_to_char("You failed to protect yourself.\n\r",ch);
      WAIT_STATE(ch,PULSE_VIOLENCE);
      return;
    }
    af.type      = SKILL_COVER;
    af.duration  = -1;
    af.modifier  = ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? -4 : -5;
    af.location  = APPLY_HITROLL;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
    act("You place yourself in a protective position.",1,ch,0,0,TO_CHAR);
    act("$n places $mself in a protective position.",1,ch,0,0,TO_ROOM);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
  else {
    affect_from_char(ch,SKILL_COVER);
    act("$n places $mself in a normal position.",1,ch,0,0,TO_ROOM);
    act("You place yourself in a normal position.",1,ch,0,0,TO_CHAR);
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
}


