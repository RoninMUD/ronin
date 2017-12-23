/* ************************************************************************
*  file: limits.c , Limit and gain control module.        Part of DIKUMUD *
*  Usage: Procedures controlling gain and limit.                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/* Includes */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "constants.h"
#include "db.h"
#include "limits.h"
#include "utils.h"
#include "spells.h"
#include "cmd.h"
#include "comm.h"
#include "utility.h"
#include "handler.h"
#include "fight.h"
#include "reception.h"
#include "subclass.h"
#include "enchant.h"


/* Defines */

#define MANA_REGEN         0
#define HP_REGEN           1

#define CLUB_GRUNTING_BOAR 3039

#define MAX_TITLE_LENGTH   80

#define READ_TITLE(ch) (GET_SEX(ch) == SEX_MALE ? \
    titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].title_m : \
    titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].title_f)


/* Externs */

extern struct time_info_data age(CHAR *ch);

extern void auto_rent(CHAR *ch);
extern void stop_riding(CHAR *ch, CHAR *vict);
extern void update_pos(CHAR *victim);

extern void check_equipment(CHAR *ch);
extern void raw_kill(CHAR *ch);
extern void aqcard_cleanup(int id);
extern void update_char_objects(CHAR *ch);
extern void extract_obj(OBJ *obj);


/* Prototypes */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6);
int mana_limit(CHAR *ch);
int hit_limit(CHAR *ch);
int move_limit(CHAR *ch);
int equipment_regen(CHAR *ch, int type);
int spell_regen(CHAR *ch, int type);
int point_update_mana(CHAR *ch);
int point_update_hit(CHAR *ch);
int point_update_move(CHAR *ch);
int mana_gain(CHAR *ch);
int hit_gain(CHAR *ch);
int move_gain(CHAR *ch);
void advance_level(CHAR *ch);
void gain_exp(CHAR *ch, int gain);
void gain_exp_regardless(CHAR *ch, int gain);
void gain_condition(CHAR *ch, int condition, int value);
void check_idling(CHAR *ch);
void point_update(void);


/* Functions */

/* When age is < 15, return the value p0 */
/* When age is 15..29, calculate the line between p1 & p2 */
/* When age is 30..44, calculate the line between p2 & p3 */
/* When age is 45..59, calculate the line between p3 & p4 */
/* When age is 60..79, calculate the line between p4 & p5 */
/* When age is >= 80, return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
  if (age < 15)
    return p0;                                           /* < 15   */
  else if (age <= 29)
    return (int) (p1 + (((age - 15) * (p2 - p1)) / 15)); /* 15..29 */
  else if (age <= 44)
    return (int) (p2 + (((age - 30) * (p3 - p2)) / 15)); /* 30..44 */
  else if (age <= 59)
    return (int) (p3 + (((age - 45) * (p4 - p3)) / 15)); /* 45..59 */
  else if (age <= 79)
    return (int) (p4 + (((age - 60) * (p5 - p4)) / 20)); /* 60..79 */
  else
    return p6;                                           /* >= 80  */
}


/* The three MAX functions define a character's Effective maximum, */
/* which is NOT the same as ch->points.max_xxxx !!!                */
int mana_limit(CHAR *ch)
{
  if (!IS_NPC(ch)) return (100 + ch->points.max_mana);
  else return ch->points.max_mana;
}

int hit_limit(CHAR *ch)
{
  if (!IS_NPC(ch)) return ch->points.max_hit;
  else return ch->points.max_hit;
}

int move_limit(CHAR *ch)
{
  if (!IS_NPC(ch)) return (100 + ch->points.max_move);
  else return ch->points.max_move;
}

int equipment_regen(CHAR *ch, int type)
{
  int i = 0;
  int obj = 0;
  int gain = 0;

  for (i = 0; i < MAX_WEAR; i++)
  {
    if (ch->equipment[i])
    {
      for (obj = 0; obj < MAX_OBJ_AFFECT; obj++)
      {
        switch (type)
        {
          case MANA_REGEN:
            if (ch->equipment[i]->affected[obj].location == APPLY_MANA_REGEN)
            {
              gain += ch->equipment[i]->affected[obj].modifier;

              continue;
            }
            break;

          case HP_REGEN:
            if (ch->equipment[i]->affected[obj].location == APPLY_HP_REGEN)
            {
              gain += ch->equipment[i]->affected[obj].modifier;

              continue;
            }
            break;
        }
      }
    }
  }

  return gain;
}

int spell_regen(CHAR *ch, int type)
{
  AFF *af = NULL;
  int gain = 0;

  for (af = ch->affected; af; af = af->next)
  {
    switch (type)
    {
      case MANA_REGEN:
        if (af->location == APPLY_MANA_REGEN) gain += af->modifier;
        break;

      case HP_REGEN:
        if (af->location == APPLY_HP_REGEN) gain += af->modifier;
        break;
    }
  }

  if (affected_by_spell(ch, SKILL_MEDITATE) &&
      (((duration_of_spell(ch, SKILL_MEDITATE) > 9) && CHAOSMODE) || duration_of_spell(ch, SKILL_MEDITATE) > 30))
  {
    gain += 60;
  }

  if (affected_by_spell(ch, SPELL_LUCK))
  {
    gain += 10;
  }

  return gain;
}

int point_update_mana(CHAR *ch)
{
  return mana_gain(ch);
}

int point_update_hit(CHAR *ch)
{
  if (IS_AFFECTED(ch, AFF_POISON))
  {
    damage(ch, ch, 2, SPELL_POISON, DAM_POISON);

    if (CHAR_REAL_ROOM(ch) == NOWHERE) return 0;
  }

  if (affected_by_spell(ch, SPELL_INCENDIARY_CLOUD))
  {
    act("The cloud of fire enveloping you burns you to the core...", FALSE, ch, 0, 0, TO_CHAR);
    damage(ch, ch, 100, SPELL_INCENDIARY_CLOUD, DAM_NO_BLOCK_NO_FLEE);

    if (CHAR_REAL_ROOM(ch) == NOWHERE) return 0;
  }

  return hit_gain(ch);
}

int point_update_move(CHAR *ch)
{
  if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_ARCTIC)
  {
    if (GET_MOVE(ch) <= 0)
    {
      act("The bitter cold chills you to the bone.", FALSE, ch, 0, 0 , TO_CHAR);

      if (GET_HIT(ch) > 3)
      {
        damage(ch, ch, number(1, 4), TYPE_UNDEFINED, DAM_NO_BLOCK);
      }
    }
  }
  else if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_DESERT)
  {
    if (GET_COND(ch, THIRST) >= 0)
    {
      act("You suffer dehydration from the heat.", FALSE, ch, 0, 0 , TO_CHAR);

      if (GET_HIT(ch) > 2)
      {
        damage(ch, ch, number(1, 3), TYPE_UNDEFINED, DAM_NO_BLOCK);
      }
    }

    gain_condition(ch, THIRST, -2);
  }

  return move_gain(ch);
}

int mana_gain(CHAR *ch)
{
  int gain = 0;
  int loss = 0;
  int year = 0;

  if (IS_NPC(ch))
  {
    gain = GET_LEVEL(ch);

    gain += equipment_regen(ch, MANA_REGEN);
    gain += spell_regen(ch, MANA_REGEN);

    if (IS_AFFECTED(ch, AFF_POISON))
    {
      gain /= 16;
    }

    return gain;
  }

  if (!ch->desc) return 0;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_REGEN)) return 0;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, REV_REGEN))
  {
    if (GET_LEVEL(ch) < 16) return 0;
    else return -10;
  }

  if ((GET_COND(ch, FULL) == 0 || GET_COND(ch, THIRST) == 0) &&
      (!check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)))
  {
    gain = 1 - GET_MANA(ch);

    if (age(ch).year > 49) loss = -5;
    else if (age(ch).year > 45) loss = -4;
    else if (age(ch).year > 41) loss = -3;
    else if (age(ch).year > 37) loss = -2;
    else if (age(ch).year > 33) loss = -1;
    else loss = 0;

    gain = MAX(gain, loss);
    gain = MIN(gain, 0);

    gain += equipment_regen(ch, MANA_REGEN);
    gain += spell_regen(ch, MANA_REGEN);

    return gain;
  }

  if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
    /* Dark Pact */
    year = 45; /* 45 is is right in the sweet spot. */
  }
  else {
    year = age(ch).year;
  }

  gain = graf(year, 9, 10, 12, 14, 12, 10, 9);

  switch (GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
      gain += 5;
      break;

    case CLASS_BARD:
      gain += 3;
      break;

    case CLASS_ANTI_PALADIN:
    case CLASS_COMMANDO:
      if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
        /* Dark Pact */
        if (IS_NIGHT || IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), DARK)) {
          gain += 3;
        }
        else {
          gain += 2;
        }
      }

      gain += 2;
      break;

    default:
      break;
  }

  switch (GET_POS(ch))
  {
    case POSITION_SLEEPING:
      gain *= 2;
      break;

    case POSITION_RESTING:
      gain += gain / 2;
      break;

    case POSITION_SITTING:
      gain += gain / 4;
      break;

    case POSITION_FIGHTING:
      gain /= 4;
      break;
  }

  switch (GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
    case CLASS_PALADIN:
    case CLASS_ANTI_PALADIN:
    case CLASS_AVATAR:
    case CLASS_BARD:
    case CLASS_COMMANDO:
    case CLASS_NINJA:
      gain *= 2;
      break;

    default:
      break;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CLUB) &&
      (IS_SET(ch->specials.pflag, PLR_LINERS_LOUNGE) || IS_SET(ch->specials.pflag, PLR_RANGERS_RELIQUARY)))
  {
    gain *= 2;
  }
  else if (CHAR_VIRTUAL_ROOM(ch) == CLUB_GRUNTING_BOAR &&
           GET_LEVEL(ch) < 16)
  {
    gain += gain / 2;
  }

  if (IS_AFFECTED(ch, AFF_POISON))
  {
    /* Combat Zen */
    if (check_subclass(ch, SC_RONIN, 1))
    {
      gain /= 8;
    }
    else
    {
      gain /= 16;
    }
  }

  gain += con_app[GET_CON(ch)].regen;
  gain += equipment_regen(ch, MANA_REGEN);
  gain += spell_regen(ch, MANA_REGEN);

  if (GET_LEVEL(ch) >= 50)
  {
    switch (GET_CLASS(ch))
    {
      case CLASS_MAGIC_USER:
      case CLASS_CLERIC:
        gain += 10;
        break;

      case CLASS_PALADIN:
      case CLASS_ANTI_PALADIN:
      case CLASS_AVATAR:
      case CLASS_BARD:
      case CLASS_COMMANDO:
      case CLASS_NINJA:
        if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
          /* Dark Pact */
          if (IS_NIGHT || IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), DARK)) {
            gain += 5;
          }
          else {
            gain += 2;
          }
        }

        gain += 5;
        break;

      default:
        break;
    }
  }

  return gain;
}

int hit_gain(CHAR *ch)
{
  int gain = 0;
  int loss = 0;
  int year = 0;

  if (IS_NPC(ch))
  {
    switch (GET_POS(ch))
    {
      case POSITION_STANDING:
      case POSITION_FLYING:
      case POSITION_RIDING:
      case POSITION_SWIMMING:
        gain = MIN(GET_LEVEL(ch) * 4, 120);
        break;

      case POSITION_RESTING:
      case POSITION_SITTING:
        gain = MIN(GET_LEVEL(ch) * 5, 150);
        break;

      case POSITION_SLEEPING:
        gain = MIN(GET_LEVEL(ch) * 6, 180);
        break;

      default:
        gain = MIN(GET_LEVEL(ch), 30);
        break;
    }

    if (IS_AFFECTED(ch, AFF_POISON))
    {
      /* Combat Zen */
      if (check_subclass(ch, SC_RONIN, 1))
      {
        gain /= 4;
      }
      else
      {
        gain /= 8;
      }
    }

    gain += equipment_regen(ch, HP_REGEN);
    gain += spell_regen(ch, HP_REGEN);

    return gain;
  }

  if (!ch->desc) return 0;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_REGEN)) return 0;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, REV_REGEN))
  {
    if (GET_LEVEL(ch) < 16) return 0;
    else return -10;
  }

  if ((GET_COND(ch, FULL) == 0 || GET_COND(ch, THIRST) == 0) &&
      (!check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)))
  {
    gain = 1 - GET_HIT(ch);

    if (age(ch).year > 49) loss = -5;
    else if (age(ch).year > 45) loss = -4;
    else if (age(ch).year > 41) loss = -3;
    else if (age(ch).year > 37) loss = -2;
    else if (age(ch).year > 33) loss = -1;
    else loss = 0;

    gain = MAX(gain, loss);
    gain = MIN(gain, 0);

    if (check_subclass(ch, SC_WARLORD, 2) &&
        ch->specials.fighting)
    {
      gain += 50;
    }

    gain += equipment_regen(ch, HP_REGEN);
    gain += spell_regen(ch, HP_REGEN);

    return gain;
  }

  if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
    /* Dark Pact */
    year = 45; /* 45 is is right in the sweet spot. */
  }
  else {
    year = age(ch).year;
  }

  gain = graf(year, 12, 14, 16, 20, 16, 14, 12);

  gain *= 2;

  switch (GET_CLASS(ch))
  {
    case CLASS_NOMAD:
      gain += 4;
      break;

    case CLASS_WARRIOR:
    case CLASS_COMMANDO:
      gain += 3;
      break;

    case CLASS_THIEF:
    case CLASS_ANTI_PALADIN:
    case CLASS_BARD:
    case CLASS_NINJA:
      if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
        /* Dark Pact */
        if (IS_NIGHT || IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), DARK)) {
          gain += 2;
        }
        else {
          gain += 1;
        }
      }

      gain += 2;
      break;

    default:
      break;
  }

  switch (GET_POS(ch))
  {
    case POSITION_SLEEPING:
      gain += gain / 2;
      break;

    case POSITION_RESTING:
      gain += gain / 4;
      break;

    case POSITION_SITTING:
      gain += gain / 8;
      break;

    case POSITION_FIGHTING:
      gain /= 4;
      break;
  }

  switch (GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
      gain /= 2;
      break;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CLUB) &&
      (IS_SET(ch->specials.pflag, PLR_SANES_VOCAL_CLUB) || IS_SET(ch->specials.pflag, PLR_LEMS_LIQOUR_LOUNGE)))
  {
    gain *= 3;
  }

  if (CHAR_VIRTUAL_ROOM(ch) == CLUB_GRUNTING_BOAR &&
      GET_LEVEL(ch) < 16)
  {
    gain += gain / 2;
  }

  if (IS_AFFECTED(ch, AFF_POISON))
  {
    /* Combat Zen */
    if (check_subclass(ch, SC_RONIN, 1))
    {
      gain /= 4;
    }
    else
    {
      gain /= 8;
    }
  }

  if (check_subclass(ch, SC_WARLORD, 3) &&
      ch->specials.fighting)
  {
    gain += 50;
  }
  
  gain += 4 * con_app[GET_CON(ch)].regen;
  gain += equipment_regen(ch, HP_REGEN);
  gain += spell_regen(ch, HP_REGEN);

  if (GET_LEVEL(ch) == 50)
  {
    switch (GET_CLASS(ch))
    {
      case CLASS_THIEF:
      case CLASS_WARRIOR:
      case CLASS_NOMAD:
        gain += 10;
        break;

      case CLASS_NINJA:
      case CLASS_PALADIN:
      case CLASS_ANTI_PALADIN:
      case CLASS_AVATAR:
      case CLASS_BARD:
      case CLASS_COMMANDO:
        if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
          /* Dark Pact */
          if (IS_NIGHT || IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), DARK)) {
            gain += 5;
          }
          else {
            gain += 2;
          }
        }

        gain += 5;
        break;

      default:
        break;
    }
  }

  if (affected_by_spell(ch, SPELL_TRANQUILITY))
  {
    gain = (gain * 12) / 10;
  }

  return gain;
}

int move_gain(CHAR *ch)
{
  int gain = 0;
  int year = 0;

  if (IS_NPC(ch)) return (GET_LEVEL(ch));

  if (!ch->desc) return 0;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_REGEN)) return 0;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, REV_REGEN))
  {
    if (GET_LEVEL(ch) < 16) return 0;
    else return -5;
  }

  if ((GET_COND(ch, FULL) == 0 || GET_COND(ch, THIRST) == 0) &&
      (world[CHAR_REAL_ROOM(ch)].sector_type != SECT_ARCTIC || world[CHAR_REAL_ROOM(ch)].sector_type != SECT_DESERT) &&
      (!check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)))
  {
    return 5;
  }

  if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
    /* Dark Pact */
    year = 45; /* 45 is is right in the sweet spot. */
  }
  else {
    year = age(ch).year;
  }

  gain = graf(year, 18, 21, 24, 26, 24, 21, 18);

  if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
    /* Dark Pact */
    if (IS_NIGHT || IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), DARK)) {
      gain += 10;
    }
    else {
      gain += 5;
    }
  }

  switch (GET_POS(ch))
  {
    case POSITION_SLEEPING:
      gain += gain / 2;
    break;

    case POSITION_RESTING:
      gain += gain / 4;
    break;

    case POSITION_SITTING:
      gain += gain / 8;
    break;
  }

  if (CHAR_VIRTUAL_ROOM(ch) == CLUB_GRUNTING_BOAR &&
      GET_LEVEL(ch) < 16)
  {
    gain += gain / 2;
  }

  if (IS_AFFECTED(ch, AFF_POISON))
  {
    /* Combat Zen */
    if (check_subclass(ch, SC_RONIN, 1))
    {
      gain /= 2;
    }
    else
    {
      gain /= 4;
    }
  }

  if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_ARCTIC)
  {
    gain = GET_CON(ch) - 20;
  }

  return gain;
}

void advance_level(CHAR *ch)
{
  int gain = 0;
  int i = 0;

  switch(GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
      gain = number(3, 7);   /* average 5, was 4 */
      break;

    case CLASS_CLERIC:
      gain = number(5, 9);   /* average 7, was 6 */
      break;

    case CLASS_THIEF:
      gain = number(7, 11);  /* average 9, was 8 */
      break;

    case CLASS_WARRIOR:
      gain = number(10, 14); /* average 12, was 11 */
      break;

    case CLASS_NINJA:
      gain = number(5, 9);   /* average 7, was 6 */
      break;

    case CLASS_NOMAD:
      gain = number(13, 17); /* average 15, was 14 */
      break;

    case CLASS_PALADIN:
      gain = number(8, 12);  /* average 10, was 9 */
      break;

    case CLASS_ANTI_PALADIN:
      gain = number(6, 10);  /* average 8, was 7 */
      break;

    case CLASS_AVATAR:
      gain = number(15, 15); /* average 15 */
      break;

    case CLASS_BARD:
      gain = number(6, 10);  /* average 8, was 6.5 */
      break;

    case CLASS_COMMANDO:
      gain = number(7, 11);  /* average 9, was 8 */
      break;
  }

  ch->points.max_hit += MAX(gain + con_app[GET_CON(ch)].hitp, 1);

  switch (GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
    case CLASS_AVATAR:
    case CLASS_BARD:
      gain = number(2, 6); /* average 4, was 3 */
      break;

    case CLASS_NINJA:
    case CLASS_PALADIN:
    case CLASS_ANTI_PALADIN:
    case CLASS_COMMANDO:
      gain = number(1, 5); /* average 3, was 2 */
      break;

    default:
      gain = 0;
      break;
  }

  ch->points.max_mana += gain;

  ch->points.max_move += 3;

  ch->specials.spells_to_learn += MAX(2, wis_app[GET_WIS(ch)].bonus);

  if (ch->specials.spells_to_learn < 0)
  {
    ch->specials.spells_to_learn = 127;
  }

  if (GET_LEVEL(ch) >= LEVEL_IMM)
  {
    for (i = 0; i < 3; i++)
    {
      ch->specials.conditions[i] = -1;
    }
  }
}

void set_title(CHAR * ch, char *title)
{
  if (title == NULL) title = READ_TITLE(ch);

  if (strlen(title) > MAX_TITLE_LENGTH) title[MAX_TITLE_LENGTH] = '\0';

  if (GET_TITLE(ch) != NULL) free(GET_TITLE(ch));

  GET_TITLE(ch) = str_dup(title);
}

void gain_exp(CHAR *ch, int gain)
{
  char buf[MIL];
  int is_altered = FALSE;

  if (IS_NPC(ch) ||
      (GET_LEVEL(ch) < LEVEL_IMM && 
       GET_LEVEL(ch) > 0))
  {
    if (gain > 0)
    {
      if (!IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
      {
        GET_EXP(ch) += gain;
      }

      /* Log a warning if exp gain is over 25 million. */
      if (gain > 25000000)
      {
        sprintf(buf, "PLRINFO: WARNING %s's exp just increased by %d. (Room %d)",
          GET_NAME(ch), gain, world[CHAR_REAL_ROOM(ch)].number);
        wizlog(buf, LEVEL_SUP, 4);
        log_f("%s", buf);
      }

      if (!IS_NPC(ch) &&
          GET_LEVEL(ch) < LEVEL_MORT)
      {
        while (GET_EXP(ch) >= exp_table[GET_LEVEL(ch) + 1])
        {
          if (GET_LEVEL(ch) >= LEVEL_MORT) break;

          send_to_char("You raise a level!\n\r", ch);

          GET_EXP(ch) -= exp_table[GET_LEVEL(ch) + 1];
          GET_LEVEL(ch)++;
          advance_level(ch);

          is_altered = TRUE;
        }
      }
    }
    else
    {
      GET_EXP(ch) += MAX(gain, -(GET_EXP(ch) / 2));
      GET_EXP(ch) = MAX(GET_EXP(ch), 0);
    }

    if (!IS_NPC(ch) &&
        is_altered &&
        !IS_SET(ch->specials.pflag, PLR_SKIPTITLE))
    {
      set_title(ch, NULL);
    }
  }
}

void gain_exp_regardless(CHAR *ch, int gain)
{
  char buf[MIL];
  bool is_altered = FALSE;

  if (!IS_NPC(ch))
  {
    GET_EXP(ch) += gain;

    if (gain > 0 &&
        GET_LEVEL(ch) < LEVEL_IMP)
    {
      /* Log a warning if exp gain is over 25 million. */
      if (gain > 25000000)
      {
        sprintf(buf, "PLRINFO: WARNING %s's exp just increased by %d. (Room %d)",
          GET_NAME(ch), gain, world[CHAR_REAL_ROOM(ch)].number);
        wizlog(buf, LEVEL_SUP, 4);
        log_f("%s", buf);
      }

      while (GET_EXP(ch) >= exp_table[GET_LEVEL(ch) + 1])
      {
        if (GET_LEVEL(ch) >= LEVEL_IMP) break;

        send_to_char("You raise a level!\n\r", ch);

        GET_EXP(ch) -= exp_table[GET_LEVEL(ch) + 1];
        GET_LEVEL(ch)++;
        advance_level(ch);

        is_altered = TRUE;
      }
    }
    else
    {
      if (gain < 0) GET_EXP(ch) += gain;
      GET_EXP(ch) = MAX(GET_EXP(ch), 0);
    }

    if (!IS_NPC(ch) &&
        is_altered &&
        !IS_SET(ch->specials.pflag, PLR_SKIPTITLE))
    {
      set_title(ch, NULL);
    }
  }
}

void gain_condition(CHAR *ch, int condition, int value)
{
  int was_intoxicated = FALSE;

  if ((!IS_MORTAL(ch) || GET_CLASS(ch) == CLASS_AVATAR) &&
      GET_COND(ch, condition) == -1 &&
      condition != DRUNK) return;

  if (GET_COND(ch, DRUNK) > 0)
  {
    was_intoxicated = TRUE;
  }

  GET_COND(ch, condition) += value;
  GET_COND(ch, condition) = MAX(GET_COND(ch, condition), 0);
  GET_COND(ch, condition) = MIN(GET_COND(ch, condition), 24);

  if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
    /* Dark Pact */
    if (GET_COND(ch, condition) == 0) {
      GET_COND(ch, condition) = -1;
    }
  }

  if (GET_COND(ch, condition) == 0)
  {
    switch (condition)
    {
      case FULL:
        if (age(ch).year > 33)
        {
          send_to_char("You'd better eat something.\n\r", ch);
        }
        else
        {
          send_to_char("You are hungry.\n\r", ch);
        }
        break;

      case THIRST:
        if (age(ch).year > 33)
        {
          send_to_char("You'd better drink something.\n\r", ch);
        }
        else
        {
          send_to_char("You are thirsty.\n\r", ch);
        }
        break;

      case DRUNK:
        if (was_intoxicated)
        {
          send_to_char("You are now sober.\n\r", ch);
        }
        break;
    }
  }
}

void check_idling(CHAR *ch)
{
  if (IS_NPC(ch)) return;

  ch->specials.timer++;

  if (GET_LEVEL(ch) > LEVEL_WIZ) return;
  else if (GET_LEVEL(ch) >= LEVEL_IMM)
  {
    if (ch->specials.timer > 60)
    {
      act("$n is pulled into the black void.", FALSE, ch, 0, 0, TO_ROOM);
      act("You are pulled into the black void.", FALSE, ch, 0, 0, TO_CHAR);

      auto_rent(ch);
    }

    return;
  }
  else
  {
    if (ch->specials.timer > 40)
    {
      auto_rent(ch);
    }
    else if (ch->specials.timer > 20)
    {
      if (ch->specials.was_in_room == NOWHERE &&
          CHAR_REAL_ROOM(ch) != NOWHERE)
      {
        ch->specials.was_in_room = CHAR_REAL_ROOM(ch);

        if (ch->specials.fighting)
        {
          stop_fighting(ch->specials.fighting);
          stop_fighting(ch);
        }

        if (ch->specials.riding)
        {
          stop_riding(ch, ch->specials.riding);
        }

        act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);

        char_from_room(ch);
        char_to_room(ch, 1);
        save_char(ch, NOWHERE);
      }
    }
  }
}

/*
 * Update PCs/NPCs and objects.
 *
 * This function relies on signaling objects that may affect regen before calling this function.
 */
void point_update(void)
{
  char buf[MSL];
  int mana_regen = 0;
  int mana_regen_cap = 0;
  CHAR *ch = NULL;
  CHAR *next_ch = NULL;
  OBJ *obj = NULL;
  OBJ *next_obj = NULL;
  OBJ *obj2 = NULL;
  OBJ *next_obj2 = NULL;

  /* PCs/NPCs */
  for (ch = character_list; ch; ch = next_ch)
  {
    next_ch = ch->next;

    if (IS_SET(ch->specials.affected_by2, AFF_SEVERED))
    {
      act("With a last gasp of breath,\n\r$n dies due to massive lower body trauma.", FALSE, ch, 0, 0, TO_ROOM);

      if (!IS_NPC(ch))
      {
        send_to_char("Your injuries prove too much, and you die.\n\r", ch);
      }

      signal_char(ch, ch, MSG_DEAD, "");
      die(ch);

      continue;
    }

    if (ch->specials.death_timer)
    {
      if (ch->specials.death_timer > 2)
      {
        ch->specials.death_timer--;
      }
      else
      {
        ch->specials.death_timer = 1;

        act("With a last gasp of breath,\n\r$n dies a horrible death.", FALSE, ch, 0, 0, TO_ROOM);

        if (!IS_NPC(ch))
        {
          send_to_char("Your injuries prove too much, and you die.\n\r", ch);
        }

        signal_char(ch, ch, MSG_DEAD, "");
        die(ch);

        continue;
      }
    }

    if (affected_by_spell(ch, SKILL_PRAY))
    {
      GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch) + 400, 1000);

      check_equipment(ch);

      if (GET_ALIGNMENT(ch) >= 1000)
      {
        affect_from_char(ch, SKILL_PRAY);
        send_to_char("You finish your prayers.\n\r", ch);
      }
    }

    if (GET_POS(ch) > POSITION_INCAP)
    {
      mana_regen = point_update_mana(ch);

      if (IS_MORTAL(ch) &&
          GET_POS(ch) == POSITION_FIGHTING &&
          mana_regen > 0)
      {
        switch (GET_CLASS(ch))
        {
          case CLASS_MAGIC_USER:
          case CLASS_CLERIC:
            mana_regen_cap = 120;
            break;

          case CLASS_AVATAR:
          case CLASS_BARD:
          case CLASS_COMMANDO:
            mana_regen_cap = 100;
            break;

          case CLASS_NINJA:
          case CLASS_PALADIN:
          case CLASS_ANTI_PALADIN:
            mana_regen_cap = 90;
            break;

          default:
            mana_regen_cap = 75;
            break;
        }

          mana_regen = MIN(mana_regen, mana_regen_cap - MIN(ch->points.mana_regen_tmp, mana_regen_cap));
          mana_regen = (mana_regen * (100 - ((MAX_RANK - get_rank(ch)) * 5))) / 100;
      }

      GET_HIT(ch) = MIN(MAX(GET_HIT(ch) + point_update_hit(ch), 1), hit_limit(ch));
      GET_MANA(ch) = MIN(MAX(GET_MANA(ch) + mana_regen, 0), mana_limit(ch));
      GET_MOVE(ch) = MIN(MAX(GET_MOVE(ch) + point_update_move(ch), 0), move_limit(ch));

      update_pos(ch);
    }
    else if (GET_POS(ch) == POSITION_INCAP)
    {
      damage(ch, ch, 1, TYPE_SUFFERING, DAM_NO_BLOCK);
    }
    else if (GET_POS(ch) == POSITION_MORTALLYW)
    {
      damage(ch, ch, 2, TYPE_SUFFERING, DAM_NO_BLOCK);
    }

    if (!IS_NPC(ch))
    {
      if (ch->ver3.time_to_quest)
      {
        if (ch->ver3.time_to_quest > 0)
        {
          ch->ver3.time_to_quest--;
        }

        if (ch->ver3.time_to_quest == 0 &&
            ch->quest_status == QUEST_RUNNING)
        {
          if (ch->questmob)
          {
            ch->questmob->questowner = NULL;
          }

          if (ch->questobj)
          {
            if (V_OBJ(ch->questobj) == 35)
            {
              aqcard_cleanup(ch->ver3.id);
            }
            else
            {
              ch->questobj->owned_by = NULL;
            }
          }

          ch->questgiver = NULL;
          ch->questmob = NULL;
          ch->questobj = NULL;
          ch->quest_status = QUEST_FAILED;
          ch->quest_level = 0;
          ch->ver3.time_to_quest = 1;

          sprintf(buf, "Your time has expired, you have failed your quest! You can start another in %d ticks.\n\r", ch->ver3.time_to_quest);
          send_to_char(buf, ch);
        }

        if (ch->ver3.time_to_quest == 1 &&
            (ch->quest_status == QUEST_FAILED || ch->quest_status == QUEST_NONE))
        {
          send_to_char("You can start another quest in one tick.\n\r", ch);
        }
      }

      if (ch->ver3.time_to_quest < 0) ch->ver3.time_to_quest = 0;

      update_char_objects(ch);
      check_idling(ch);
    }

    if (GET_LEVEL(ch) > LEVEL_MORT ||
        GET_CLASS(ch) == CLASS_AVATAR)
    {
      GET_COND(ch, FULL) = -1;
      GET_COND(ch, THIRST) = -1;
      GET_COND(ch, DRUNK) = -1;
    }
    else
    {
      gain_condition(ch, FULL, -1);
      gain_condition(ch, DRUNK, -1);
      gain_condition(ch, THIRST, -1);
    }

    gain_condition(ch, QUAFF, -1);

    if (IS_NPC(ch) &&
        IS_AFFECTED(ch, AFF_ANIMATE) &&
        !IS_AFFECTED(ch, AFF_CHARM) &&
        GET_POS(ch) != POSITION_FIGHTING &&
        CHAR_REAL_ROOM(ch) != NOWHERE)
    {
      act("$n waves happily and disappears in a puff of smoke.", TRUE, ch, 0, 0, TO_ROOM);
      extract_char(ch);
    }
  }

  /* Objects */
  for (obj = object_list; obj; obj = next_obj)
  {
    next_obj = obj->next;

    /* Decaying objects. */
    if (IS_SET(obj->obj_flags.extra_flags2, ITEM_ALL_DECAY) ||
        IS_SET(obj->obj_flags.extra_flags2, ITEM_EQ_DECAY))
    {
      /* Equipped/Carried objects for PCs/NPCs have their timer updated in
         update_char_objects() which is in handler.c */
      if (IS_SET(obj->obj_flags.extra_flags2, ITEM_ALL_DECAY) &&
          !obj->equipped_by &&
          !obj->carried_by &&
          obj->obj_flags.timer > 0)
      {
        obj->obj_flags.timer--;
      }

      if (obj->obj_flags.timer < 1)
      {
        if (obj->carried_by)
        {
          sprintf(buf,"WIZINFO: DECAY - Carried by: %s, Obj: %s", GET_NAME(obj->carried_by), OBJ_SHORT(obj));
          log_f("%s", buf);

          act("$p decays in your inventory.", FALSE, obj->carried_by, obj, 0, TO_CHAR);
        }
        else if (obj->equipped_by)
        {
          sprintf(buf,"WIZINFO: DECAY - Equipped by: %s, Obj: %s", GET_NAME(obj->equipped_by), OBJ_SHORT(obj));
          log_f("%s", buf);

          act("$p decays while it's equipped!", FALSE, obj->equipped_by, obj, 0, TO_CHAR);
        }
        else if (obj->in_room != NOWHERE)
        {
          sprintf(buf,"WIZINFO: DECAY - In Room: %d, Obj: %s", world[obj->in_room].number, OBJ_SHORT(obj));
          log_f("%s", buf);

          if (world[obj->in_room].people)
          {
            act("$p decays, turning to dust.", TRUE, world[obj->in_room].people, obj, 0, TO_ROOM);
            act("$p decays, turning to dust.", TRUE, world[obj->in_room].people, obj, 0, TO_CHAR);
          }
        }

        if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER)
        {
          for (obj2 = obj->contains; obj2; obj2 = next_obj2)
          {
            next_obj2 = obj2->next_content;

            obj_from_obj(obj2);

            if (obj->in_obj) obj_to_obj(obj2, obj->in_obj);
            else
            if (obj->carried_by)
            {
              if (obj2->obj_flags.type_flag == ITEM_MONEY)
              {
                GET_GOLD(obj->carried_by) += obj2->obj_flags.value[0];
              }
              else
              {
                obj_to_char(obj2, obj->carried_by);
              }
            }
            else
            if (obj->equipped_by)
            {
              if (obj2->obj_flags.type_flag == ITEM_MONEY)
              {
                GET_GOLD(obj->equipped_by) += obj2->obj_flags.value[0];
              }
              else
              {
                obj_to_char(obj2, obj->equipped_by);
              }
            }
            else
            if (obj->in_room != NOWHERE)
            {
              obj_to_room(obj2, obj->in_room);
            }
            else
            {
              log_f("WIZINFO: DECAY - Container decayed in NOWHERE.");
            }
          }
        }

        extract_obj(obj);
      }
    }
    /* Decay corpses. */
    else if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER &&
             obj->obj_flags.value[3])
    {
      if (obj->obj_flags.timer > 0)
      {
        obj->obj_flags.timer--;
      }

      if (obj->obj_flags.timer < 1)
      {
        if (obj->carried_by)
        {
          act("$p decays in your hands.", FALSE, obj->carried_by, obj, 0, TO_CHAR);
        }
        else if (obj->equipped_by) /* Foolish immortals... */
        {
          act("$p decays on you.", FALSE, obj->equipped_by, obj, 0, TO_CHAR);
        }
        else if (obj->in_room != NOWHERE)
        {
          /* Don't decay a PC_CORPSE or PC_STATUE if it contains objects. */
          if (obj->contains &&
            (obj->obj_flags.cost == PC_CORPSE || obj->obj_flags.cost == PC_STATUE))
          {
            obj->obj_flags.timer = 9;

            continue;
          }
          else if (world[obj->in_room].people)
          {
            if (obj->obj_flags.cost == PC_STATUE ||
                obj->obj_flags.cost == NPC_STATUE)
            {
              act("$p crumbles to dust.", TRUE, world[obj->in_room].people, obj, 0, TO_ROOM);
              act("$p crumbles to dust.", TRUE, world[obj->in_room].people, obj, 0, TO_CHAR);
            }
            else
            {
              act("A quivering horde of maggots consumes $p.", TRUE, world[obj->in_room].people, obj, 0, TO_ROOM);
              act("A quivering horde of maggots consumes $p.", TRUE, world[obj->in_room].people, obj, 0, TO_CHAR);
            }
          }
        }

        for (obj2 = obj->contains; obj2; obj2 = next_obj2)
        {
          next_obj2 = obj2->next_content;

          obj_from_obj(obj2);

          if (obj->in_obj)
          {
            obj_to_obj(obj2, obj->in_obj);
          }
          else if (obj->carried_by)
          {
            if (obj2->obj_flags.type_flag == ITEM_MONEY)
            {
              GET_GOLD(obj->carried_by) += obj2->obj_flags.value[0];
            }
            else
            {
              obj_to_char(obj2, obj->carried_by);
            }
          }
          else if (obj->equipped_by)
          {
            if (obj2->obj_flags.type_flag == ITEM_MONEY)
            {
              GET_GOLD(obj->equipped_by) += obj2->obj_flags.value[0];
            }
            else
            {
              obj_to_char(obj2, obj->equipped_by);
            }
          }
          else if (obj->in_room != NOWHERE)
          {
            obj_to_room(obj2, obj->in_room);
          }
          else
          {
            log_f("WIZINFO: DECAY - Corpse decayed in NOWHERE.");
          }
        }

        extract_obj(obj);
      }
    }
  }
}
