/* ************************************************************************
*  file: limits.c , Limit and gain control module.        Part of DIKUMUD *
*  Usage: Procedures controlling gain and limit.                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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
#include "aff_ench.h"

/* Defines */

#define MANA_REGEN         0
#define HP_REGEN           1

#define CLUB_GRUNTING_BOAR 3039

#define MAX_TITLE_LENGTH   80

#define READ_TITLE(ch) (GET_SEX(ch) == SEX_MALE ? titles[GET_CLASS(ch) - 1][GET_LEVEL(ch)].title_m :  titles[GET_CLASS(ch) - 1][GET_LEVEL(ch)].title_f)


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
void set_title(CHAR *ch, char *title);
void gain_exp(CHAR *ch, int gain);
void gain_exp_regardless(CHAR *ch, int gain);
void gain_condition(CHAR *ch, int condition, int value);
void check_idling(CHAR *ch);
void point_update(void);

/* Constants */
const int optimal_age = 45;

const double rank_regen_non_caster[] = {
  1.15,  /* Rank 1 */
  1.075, /* Rank 2 */
  1.075  /* Rank 3 */
};

const double rank_regen_caster[] = {
  1.1,  /* Rank 1 */
  1.05, /* Rank 2 */
  1.05  /* Rank 3 */
};

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

/**
* @brief Get character's effective maximum hit points.
*
* @param[in] ch The character.
*
* @return The character's effective maximum hit points.
*/
int hit_limit(CHAR *ch) {
  return GET_MAX_HIT_POINTS(ch);
}

/**
* @brief Get character's effective maximum mana points.
*
* Note: Not the same as ch->points.max_mana for players!
*
* @param[in] ch The character.
*
* @return The character's effective maximum mana points.
*/
int mana_limit(CHAR *ch) {
  return GET_MAX_MANA_POINTS(ch) + (!IS_NPC(ch) ? 100 : 0);
}

/**
 * @brief Get character's effective maximum movement points.
 *
 * Note: Not the same as ch->points.max_move for players!
 *
 * @param[in] ch The character.
 *
 * @return The character's effective maximum movement points.
 */
int move_limit(CHAR *ch) {
  return GET_MAX_MOVE_POINTS(ch) + (!IS_NPC(ch) ? 100 : 0);
}

int equipment_regen(CHAR *ch, int type) {
  if (!ch) return 0;

  int gain = 0;

  for (int eq_pos = 0; eq_pos < MAX_WEAR; eq_pos++) {
    if (EQ(ch, eq_pos)) {
      for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
        switch (type) {
          case MANA_REGEN:
            if (OBJ_AFF_LOC(EQ(ch, eq_pos), i) == APPLY_MANA_REGEN) {
              gain += OBJ_AFF_MOD(EQ(ch, eq_pos), i);
            }
            break;

          case HP_REGEN:
            if (OBJ_AFF_LOC(EQ(ch, eq_pos), i) == APPLY_HP_REGEN) {
              gain += OBJ_AFF_MOD(EQ(ch, eq_pos), i);
            }
            break;
        }
      }
    }
  }

  return gain;
}

int spell_regen(CHAR *ch, int type) {
  if (!ch) return 0;

  int gain = 0;

  for (AFF *aff = ch->affected; aff; aff = aff->next) {
    switch (type) {
      case MANA_REGEN:
        if (aff->location == APPLY_MANA_REGEN) {
          gain += aff->modifier;
        }
        break;

      case HP_REGEN:
        if (aff->location == APPLY_HP_REGEN) {
          gain += aff->modifier;
        }
        break;
    }
  }

  if (affected_by_spell(ch, SPELL_LUCK)) {
    switch (type) {
      case MANA_REGEN:
        gain += 15;
        break;

      case HP_REGEN:
        gain += 75;
        break;
    }
  }

  return gain;
}

int point_update_mana(CHAR *ch) {
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return 0;

  return mana_gain(ch);
}

int point_update_hit(CHAR *ch) {
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return 0;

  if (IS_AFFECTED(ch, AFF_POISON))   {
    damage(ch, ch, 2, SPELL_POISON, DAM_POISON);

    if (CHAR_REAL_ROOM(ch) == NOWHERE) return 0;
  }

  /* Old Incendiary Cloud (used by some specs, etc.) */
  if (affected_by_spell(ch, SPELL_INCENDIARY_CLOUD))   {
    send_to_char("The cloud of fire enveloping you burns you to the core...\n\r", ch);

    damage(ch, ch, 100, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

    if (CHAR_REAL_ROOM(ch) == NOWHERE) return 0;
  }

  return hit_gain(ch);
}

int point_update_move(CHAR *ch) {
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return 0;

  switch (ROOM_SECTOR_TYPE(CHAR_REAL_ROOM(ch))) {
    case SECT_ARCTIC:
      if (GET_MOVE(ch) <= 0) {
        send_to_char("The bitter cold chills you to the bone.\n\r", ch);

        if (GET_HIT(ch) > 4) {
          damage(ch, ch, number(1, 4), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
        }
      }
      break;

    case SECT_DESERT:
      if (GET_COND(ch, THIRST) >= 0) {
        send_to_char("You suffer dehydration from the heat.\n\r", ch);

        if (GET_HIT(ch) > 4) {
          damage(ch, ch, number(1, 4), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
        }
      }

      gain_condition(ch, THIRST, -2);
      break;
  }

  return move_gain(ch);
}

int mana_gain(CHAR *ch) {
  if (!ch) return 0;

  int gain = 0;

  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);

    if (IS_AFFECTED(ch, AFF_POISON)) {
      gain /= 16;
    }

    gain += equipment_regen(ch, MANA_REGEN);

    gain += spell_regen(ch, MANA_REGEN);
  }
  else {
    if (IS_NPC(ch)) return GET_LEVEL(ch);

    if (!GET_DESCRIPTOR(ch)) return 0;

    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), NO_REGEN)) return 0;

    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), REV_REGEN)) {
      if (GET_LEVEL(ch) <= 15) {
        return 0;
      }
      else {
        return -5;
      }
    }

    int year = age(ch).year;

    /* Dark Pact - Provides optimal age-based regen. */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
      year = optimal_age;
    }

    gain = graf(year, 10, 12, 14, 16, 14, 12, 10);

    /* Base class regen. */
    switch (GET_CLASS(ch)) {
      case CLASS_MAGIC_USER:
      case CLASS_CLERIC:
        gain += 5;
        break;

      case CLASS_NINJA:
      case CLASS_AVATAR:
      case CLASS_BARD:
        gain += 3;
        break;

      case CLASS_PALADIN:
      case CLASS_ANTI_PALADIN:
      case CLASS_COMMANDO:
        gain += 2;
        break;
    }

    /* Dark Pact - Increases base class regen. */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
      if (IS_NIGHT || IS_SET(CHAR_ROOM_FLAGS(ch), DARK)) {
        gain += 3;
      }
      else {
        gain += 2;
      }
    }

    /* Class-based modifier. */
    switch (GET_CLASS(ch)) {
      case CLASS_MAGIC_USER:
      case CLASS_CLERIC:
      case CLASS_NINJA:
      case CLASS_PALADIN:
      case CLASS_ANTI_PALADIN:
      case CLASS_AVATAR:
      case CLASS_BARD:
      case CLASS_COMMANDO:
        gain *= 2;
        break;
    }

    /* Position-based modifier. */
    switch (GET_POS(ch)) {
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

    /* Level 50 regen. */
    if (GET_LEVEL(ch) >= 50) {
      switch (GET_CLASS(ch)) {
        case CLASS_MAGIC_USER:
        case CLASS_CLERIC:
          gain += 10;
          break;

        case CLASS_NINJA:
        case CLASS_PALADIN:
        case CLASS_ANTI_PALADIN:
        case CLASS_AVATAR:
        case CLASS_BARD:
        case CLASS_COMMANDO:
          gain += 5;
          break;
      }

      /* Dark Pact - Increases level 50 regen. */
      if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
        if (IS_NIGHT || IS_SET(CHAR_ROOM_FLAGS(ch), DARK)) {
          gain += 5;
        }
        else {
          gain += 2;
        }
      }
    }

    /* Bathed in Blood */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_DEFILER, 5)) {
      if ((CHAR_REAL_ROOM(ch) != NOWHERE) && ROOM_BLOOD(CHAR_REAL_ROOM(ch))) {
        double multi = 1.0 + (0.2 * (double)ROOM_BLOOD(CHAR_REAL_ROOM(ch)));

        if (multi > 2.0) multi = 2.0;

        gain *= multi;
      }
    }

    if (gain > 0) {
      /* Calculate regeneration from ranks. */
      if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
        for (int i = 0; (i < get_rank(ch)) && (i < NUMELEMS(rank_regen_non_caster)); i++) {
          gain *= rank_regen_non_caster[i];
        }
      }
      else {
        for (int i = 0; (i < get_rank(ch)) && (i < NUMELEMS(rank_regen_caster)); i++) {
          gain *= rank_regen_caster[i];
        }
      }

      // Prestige Perk 8
      if (GET_PRESTIGE_PERK(ch) >= 8) {
        gain *= 1.05;
      }
    }

    /* Meditate */
    if (affected_by_spell(ch, SKILL_MEDITATE) && (duration_of_spell(ch, SKILL_MEDITATE) >= 10)) {
      gain *= 2;
    }

    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CLUB) || ((CHAR_VIRTUAL_ROOM(ch) == CLUB_GRUNTING_BOAR) && (GET_LEVEL(ch) <= 15))) {
      gain *= 2;
    }

    /* Druid SC3: Wall of Thorns - Increases regeneration by 1/3rd when sitting, resting, or sleeping. */
    if (!GET_OPPONENT(ch) && (GET_POS(ch) <= POSITION_SITTING) && (GET_POS(ch) >= POSITION_SLEEPING) && get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch))) {
      gain *= 1.3333;
    }

    /* Dark Pact - Bypasses hunger-based regen. */
    if (!(IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch))) {
      /* Hunger reduces positive gain to 1/10th normal. */
      if ((gain > 0) && ((GET_COND(ch, FULL) == 0 || GET_COND(ch, THIRST) == 0))) {
        gain /= 10;
      }
    }

    if (IS_AFFECTED(ch, AFF_POISON)) {
      /* Combat Zen */
      if (IS_MORTAL(ch) && check_subclass(ch, SC_RONIN, 3)) {
        gain /= 8;
      }
      else {
        gain /= 16;
      }
    }

    /* Constitution modifier. */
    gain += con_app[GET_CON(ch)].regen;

    gain += equipment_regen(ch, MANA_REGEN);

    gain += spell_regen(ch, MANA_REGEN);

    /* Inner Peace */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_MYSTIC, 2)) {
      gain += 10;
    }

    if (gain > 0) {
      /* Limit mana regen in combat. */
      if (IS_MORTAL(ch) && GET_OPPONENT(ch) && SAME_ROOM(ch, GET_OPPONENT(ch))) {
        int mana_regen_cap = 75;

        switch (GET_CLASS(ch)) {
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
        }

        /* Inner Peace */
        if (IS_MORTAL(ch) && check_subclass(ch, SC_MYSTIC, 2)) {
          mana_regen_cap += 10;
        }

        // Prestige Perk 24
        if (GET_PRESTIGE_PERK(ch) >= 24) {
          if (mana_regen_cap > 0) {
            mana_regen_cap += 5;
          }
        }

        gain = MIN(gain, mana_regen_cap);
      }
    }
  }

  return gain;
}

int hit_gain(CHAR *ch) {
  if (!ch) return 0;

  int gain = 0;

  /* NPCs */
  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);

    switch (GET_POS(ch)) {
      case POSITION_STANDING:
      case POSITION_FLYING:
      case POSITION_RIDING:
      case POSITION_SWIMMING:
        gain = MIN(gain * 4, 120);
        break;

      case POSITION_RESTING:
      case POSITION_SITTING:
        gain = MIN(gain * 5, 150);
        break;

      case POSITION_SLEEPING:
        gain = MIN(gain * 6, 180);
        break;

      default:
        gain = MIN(gain, 30);
        break;
    }

    if (IS_AFFECTED(ch, AFF_POISON)) {
      gain /= 8;
    }

    gain += equipment_regen(ch, HP_REGEN);

    gain += spell_regen(ch, HP_REGEN);
  }
  else {
    if (!GET_DESCRIPTOR(ch)) return 0;

    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), NO_REGEN)) return 0;

    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), REV_REGEN)) {
      if (GET_LEVEL(ch) <= 15) {
        return 0;
      }
      else {
        return -5;
      }
    }

    int year = age(ch).year;

    /* Dark Pact - Provides optimal age-based regen. */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
      year = optimal_age;
    }

    /* Age-based regen. */
    gain = graf(year, 13, 16, 19, 22, 19, 16, 13);

    /* Base class regen. */
    switch (GET_CLASS(ch)) {
      case CLASS_THIEF:
      case CLASS_WARRIOR:
      case CLASS_NOMAD:
        gain += 6;
        break;

      case CLASS_NINJA:
      case CLASS_PALADIN:
      case CLASS_ANTI_PALADIN:
      case CLASS_AVATAR:
      case CLASS_BARD:
      case CLASS_COMMANDO:
        gain += 2;
        break;
    }

    /* Dark Pact - Increases base class regen. */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
      if (IS_NIGHT || IS_SET(CHAR_ROOM_FLAGS(ch), DARK)) {
        gain += 4;
      }
      else {
        gain += 2;
      }
    }

    /* Base multiplier. */
    gain *= 2;

    /* Class-based modifier. */
    switch (GET_CLASS(ch)) {
      case CLASS_MAGIC_USER:
      case CLASS_CLERIC:
        gain -= gain / 3;
        break;
    }

    /* Position-based modifier. */
    switch (GET_POS(ch)) {
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
        gain /= 3;
        break;
    }

    /* Level 50 regen. */
    if (GET_LEVEL(ch) >= 50) {
      switch (GET_CLASS(ch)) {
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
          gain += 5;
          break;
      }

      /* Dark Pact - Increases level 50 regen. */
      if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
        if (IS_NIGHT || IS_SET(CHAR_ROOM_FLAGS(ch), DARK)) {
          gain += 5;
        }
        else {
          gain += 2;
        }
      }
    }

    /* Bathed in Blood */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_DEFILER, 5)) {
      if ((CHAR_REAL_ROOM(ch) != NOWHERE) && ROOM_BLOOD(CHAR_REAL_ROOM(ch))) {
        double multi = 1.0 + (0.2 * ROOM_BLOOD(CHAR_REAL_ROOM(ch)));

        if (multi > 2.0) multi = 2.0;

        gain *= multi;
      }
    }

    /* Tranquility */
    if (affected_by_spell(ch, SPELL_TRANQUILITY)) {
      gain *= 1.25;
    }

    if (gain > 0) {
      /* Calculate regeneration from ranks. */
      if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
        for (int i = 0; (i < get_rank(ch)) && (i < NUMELEMS(rank_regen_non_caster)); i++) {
          gain *= rank_regen_non_caster[i];
        }
      }
      else {
        for (int i = 0; (i < get_rank(ch)) && (i < NUMELEMS(rank_regen_caster)); i++) {
          gain *= rank_regen_caster[i];
        }
      }

      // Prestige Perk 8
      if (GET_PRESTIGE_PERK(ch) >= 8) {
        gain *= 1.05;
      }
    }

    /* Meditate */
    if (affected_by_spell(ch, SKILL_MEDITATE) && (duration_of_spell(ch, SKILL_MEDITATE) >= 10)) {
      gain *= 2;
    }

    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CLUB) || ((CHAR_VIRTUAL_ROOM(ch) == CLUB_GRUNTING_BOAR) && (GET_LEVEL(ch) <= 15))) {
      gain *= 2;
    }

    /* Druid SC3: Wall of Thorns - Increases regeneration by 1/3rd when sitting, resting, or sleeping. */
    if (!GET_OPPONENT(ch) && (GET_POS(ch) <= POSITION_SITTING) && (GET_POS(ch) >= POSITION_SLEEPING) && get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch))) {
      gain *= 1.3333;
    }

    /* Druid SC1: Degenerate - 1/4 hit point regen for 5 ticks after using Degenerate. */
    if ((ench_enchanted_by(ch, 0, ENCHANT_DEGENERATE) && (ench_duration(ch, 0, ENCHANT_DEGENERATE) >= 15))) {
      gain /= 4;
    }

    /* Dark Pact - Bypasses hunger-based regen. */
    if (!(IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch))) {
      /* Hunger reduces positive gain to 1/10th normal. */
      if ((gain > 0) && ((GET_COND(ch, FULL) == 0 || GET_COND(ch, THIRST) == 0))) {
        gain /= 10;
      }
    }

    if (IS_AFFECTED(ch, AFF_POISON)) {
      /* Combat Zen */
      if (IS_MORTAL(ch) && check_subclass(ch, SC_RONIN, 3)) {
        gain /= 4;
      }
      else {
        gain /= 8;
      }
    }

    /* Constitution modifier. */
    gain += 4 * con_app[GET_CON(ch)].regen;

    gain += equipment_regen(ch, HP_REGEN);

    gain += spell_regen(ch, HP_REGEN);

    /* Awareness */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 1)) {
      gain += 2 * GET_LEVEL(ch);
    }
  }

  return gain;
}

int move_gain(CHAR *ch) {
  if (!ch) return 0;

  int gain = 0;

  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);
  }
  else {
    if (!GET_DESCRIPTOR(ch)) return 0;

    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), NO_REGEN)) return 0;

    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), REV_REGEN)) {
      if (GET_LEVEL(ch) <= 15) {
        return 0;
      }
      else {
        return -5;
      }
    }

    int year = age(ch).year;

    /* Dark Pact - Increases age-based regen. */
    if (check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
      year = optimal_age;
    }

    /* Age-based regen. */
    gain = graf(year, 18, 21, 24, 27, 24, 21, 18);

    /* Dark Pact - Increases base class regen. */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
      if (IS_NIGHT || IS_SET(CHAR_ROOM_FLAGS(ch), DARK)) {
        gain += 10;
      }
      else {
        gain += 5;
      }
    }

    /* Position-based modifier. */
    switch (GET_POS(ch)) {
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

    if (gain > 0) {
      /* Calculate regeneration from ranks. */
      if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
        for (int i = 0; (i < get_rank(ch)) && (i < NUMELEMS(rank_regen_non_caster)); i++) {
          gain *= rank_regen_non_caster[i];
        }
      }
      else {
        for (int i = 0; (i < get_rank(ch)) && (i < NUMELEMS(rank_regen_caster)); i++) {
          gain *= rank_regen_caster[i];
        }
      }

      // Prestige Perk 8
      if (GET_PRESTIGE_PERK(ch) >= 8) {
        gain *= 1.05;
      }
    }

    /* Meditate */
    if (affected_by_spell(ch, SKILL_MEDITATE) && (duration_of_spell(ch, SKILL_MEDITATE) >= 10)) {
      gain *= 2;
    }

    if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CLUB) || ((CHAR_VIRTUAL_ROOM(ch) == CLUB_GRUNTING_BOAR) && (GET_LEVEL(ch) <= 15))) {
      gain *= 2;
    }

    /* Druid SC3: Wall of Thorns - Increases regeneration by 1/3rd when sitting, resting, or sleeping. */
    if (!GET_OPPONENT(ch) && (GET_POS(ch) <= POSITION_SITTING) && (GET_POS(ch) >= POSITION_SLEEPING) && get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch))) {
      gain *= 1.3333;
    }

    /* Dark Pact - Bypasses hunger-based regen. */
    if (!(IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch))) {
      /* Hunger reduces positive gain to 1/10th normal. */
      if ((gain > 0) && ((GET_COND(ch, FULL) == 0 || GET_COND(ch, THIRST) == 0))) {
        gain /= 10;

        /* Gain a small amount of movement points, even if hungry, as long as the pre-hunger gain was positive. */
        gain = MAX(gain, 5);
      }
    }

    if (IS_AFFECTED(ch, AFF_POISON)) {
      /* Combat Zen */
      if (IS_MORTAL(ch) && check_subclass(ch, SC_RONIN, 3)) {
        gain /= 2;
      }
      else {
        gain /= 4;
      }
    }

    /* Constitution modifier. */
    gain += con_app[GET_CON(ch)].regen;
  }

  return gain;
}

void advance_level(CHAR *ch) {
  int hit_gain = con_app[MAX(GET_CON(ch), 18)].hitp;

  switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
      hit_gain += number(3, 7);   /* average 5 */
      break;

    case CLASS_CLERIC:
      hit_gain += number(5, 9);   /* average 7 */
      break;

    case CLASS_THIEF:
      hit_gain += number(8, 12);  /* average 10 */
      break;

    case CLASS_WARRIOR:
      hit_gain += number(10, 14); /* average 12 */
      break;

    case CLASS_NINJA:
      hit_gain += number(6, 10);  /* average 8 */
      break;

    case CLASS_NOMAD:
      hit_gain += number(13, 17); /* average 15 */
      break;

    case CLASS_PALADIN:
      hit_gain += number(7, 11);  /* average 9 */
      break;

    case CLASS_ANTI_PALADIN:
      hit_gain += number(6, 10);  /* average 8 */
      break;

    case CLASS_AVATAR:
      hit_gain += number(15, 15); /* average 15 */
      break;

    case CLASS_BARD:
      hit_gain += number(5, 9);   /* average 7 */
      break;

    case CLASS_COMMANDO:
      hit_gain += number(7, 11);  /* average 9 */
      break;
  }

  int mana_gain = 0;

  switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
    case CLASS_AVATAR:
    case CLASS_BARD:
      mana_gain += number(2, 6); /* average 4 */
      break;

    case CLASS_NINJA:
    case CLASS_PALADIN:
    case CLASS_ANTI_PALADIN:
    case CLASS_COMMANDO:
      mana_gain += number(1, 5); /* average 3 */
      break;
  }

  GET_MAX_HIT_POINTS(ch) = MIN(GET_MAX_HIT_POINTS(ch) + hit_gain, SHRT_MAX);
  GET_MAX_MANA_POINTS(ch) = MIN(GET_MAX_MANA_POINTS(ch) + mana_gain, SHRT_MAX);
  GET_MAX_MOVE_POINTS(ch) = MIN(GET_MAX_MOVE_POINTS(ch) + 3, SHRT_MAX);
  GET_PRAC(ch) = MIN(GET_PRAC(ch) + 1 + wis_app[MAX(GET_WIS(ch), 18)].bonus, SCHAR_MAX);
}

void set_title(CHAR * ch, char *title) {
  if (!title) {
    title = READ_TITLE(ch);
  }

  if (strlen(title) > MAX_TITLE_LENGTH) {
    title[MAX_TITLE_LENGTH] = '\0';
  }

  if (GET_TITLE(ch)) {
    free(GET_TITLE(ch));
    GET_TITLE(ch) = NULL;
  }

  GET_TITLE(ch) = str_dup(title);
}

void gain_exp(CHAR *ch, int gain) {
  char buf[MIL];
  int is_altered = FALSE;

  if (IS_NPC(ch) || (GET_LEVEL(ch) < LEVEL_IMM && GET_LEVEL(ch) > 0)) {
    if (gain > 0) {
      if (!IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC)) {
        GET_EXP(ch) += gain;
      }

      /* Log a warning if exp gain is over 25 million. */
      if (gain > 25000000) {
        sprintf(buf, "PLRINFO: WARNING %s's exp just increased by %d. (Room %d)",
          GET_NAME(ch), gain, world[CHAR_REAL_ROOM(ch)].number);
        wizlog(buf, LEVEL_SUP, 4);
        log_f("%s", buf);
      }

      if (!IS_NPC(ch) &&
        GET_LEVEL(ch) < LEVEL_MORT) {
        while (GET_EXP(ch) >= exp_table[GET_LEVEL(ch) + 1]) {
          if (GET_LEVEL(ch) >= LEVEL_MORT) break;

          send_to_char("You raise a level!\n\r", ch);

          GET_EXP(ch) -= exp_table[GET_LEVEL(ch) + 1];
          GET_LEVEL(ch)++;
          advance_level(ch);

          is_altered = TRUE;
        }
      }
    }
    else {
      GET_EXP(ch) += MAX(gain, -(GET_EXP(ch) / 2));
      GET_EXP(ch) = MAX(GET_EXP(ch), 0);
    }

    if (!IS_NPC(ch) &&
      is_altered &&
      !IS_SET(ch->specials.pflag, PLR_SKIPTITLE)) {
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

void gain_condition(CHAR *ch, int condition, int value) {
  if (!ch || IS_NPC(ch) || ((condition < 0) || (condition > MAX_COND)) || (value == 0)) return;

  int was_intoxicated = GET_COND(ch, DRUNK) > 0;

  GET_COND(ch, condition) = MIN(MAX(GET_COND(ch, condition) + value, 0), 24);

  if ((condition == FULL) || (condition == THIRST)) {
    bool free_lunch = FALSE;

    if (IS_IMMORTAL(ch) || (GET_CLASS(ch) == CLASS_AVATAR)) {
      free_lunch = TRUE;
    }
    // Dark Pact
    else if (IS_MORTAL(ch) && check_subclass(ch, SC_INFIDEL, 1) && IS_EVIL(ch)) {
      free_lunch = TRUE;
    }
    // Prestige Perk 26
    else if (GET_PRESTIGE_PERK(ch) >= 26) {
      free_lunch = TRUE;
    }

    if (free_lunch) {
      GET_COND(ch, condition) = -1;
    }
  }

  if (GET_COND(ch, condition) == 0) {
    switch (condition) {
      case DRUNK:
        if (was_intoxicated) {
          send_to_char("You are now sober.\n\r", ch);
        }
        break;

      case FULL:
        if (age(ch).year > 33) {
          send_to_char("You'd better eat something.\n\r", ch);
        }
        else {
          send_to_char("You are hungry.\n\r", ch);
        }
        break;

      case THIRST:
        if (age(ch).year > 33) {
          send_to_char("You'd better drink something.\n\r", ch);
        }
        else {
          send_to_char("You are thirsty.\n\r", ch);
        }
        break;
    }
  }
}

void check_idling(CHAR *ch) {
  if (!ch || IS_NPC(ch)) return;

  GET_TIMER(ch)++;

  if (IS_MORTAL(ch)) {
    if (GET_TIMER(ch) > 40) {
      auto_rent(ch);
    }
    else if (GET_TIMER(ch) > 20) {
      if ((GET_WAS_IN_ROOM(ch) == NOWHERE) && (CHAR_REAL_ROOM(ch) != NOWHERE)) {
        GET_WAS_IN_ROOM(ch) = CHAR_REAL_ROOM(ch);

        if (GET_OPPONENT(ch)) {
          stop_fighting(GET_OPPONENT(ch));
          stop_fighting(ch);
        }

        if (GET_MOUNT(ch)) {
          stop_riding(ch, GET_MOUNT(ch));
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
void point_update(void) {
  /* PCs/NPCs */
  for (CHAR *ch = character_list, *next_ch = NULL; ch; ch = next_ch) {
    next_ch = ch->next;

    if (IS_SET(GET_AFF2(ch), AFF2_SEVERED)) {
      act("With a last gasp of breath, $n dies due to massive lower body trauma.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Your injuries prove too much, and you die.\n\r", ch);

      signal_char(ch, ch, MSG_DEAD, "");

      die(ch);

      continue;
    }

    if (GET_DEATH_TIMER(ch)) {
      if (GET_DEATH_TIMER(ch) > 2) {
        GET_DEATH_TIMER(ch)--;
      }
      else {
        GET_DEATH_TIMER(ch) = 1;

        act("With a last gasp of breath, $n dies a horrible death.", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("Your injuries prove too much, and you die.\n\r", ch);

        signal_char(ch, ch, MSG_DEAD, "");

        die(ch);

        continue;
      }
    }

    if (affected_by_spell(ch, SKILL_PRAY)) {
      GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch) + 400, 1000);

      check_equipment(ch);

      if (GET_ALIGNMENT(ch) >= 1000) {
        affect_from_char(ch, SKILL_PRAY);

        send_to_char("You finish your prayers.\n\r", ch);
      }
    }

    if (GET_POS(ch) > POSITION_INCAP) {
      /* Calculate base regeneration. */
      int hit_regen = point_update_hit(ch);
      int mana_regen = point_update_mana(ch);
      int move_regen = point_update_move(ch);

      GET_HIT(ch) = MIN(MAX(GET_HIT(ch) + hit_regen, 1), hit_limit(ch));
      GET_MANA(ch) = MIN(MAX(GET_MANA(ch) + mana_regen, 0), mana_limit(ch));
      GET_MOVE(ch) = MIN(MAX(GET_MOVE(ch) + move_regen, 0), move_limit(ch));

      update_pos(ch);
    }

    if (GET_POS(ch) == POSITION_INCAP) {
      damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NO_BLOCK);
      if (DISPOSED(ch)) continue;

    }
    else if (GET_POS(ch) == POSITION_MORTALLYW) {
      damage(ch, ch, 2, TYPE_UNDEFINED, DAM_NO_BLOCK);
      if (DISPOSED(ch)) continue;
    }

    if (!IS_NPC(ch)) {
      if (GET_QUEST_TIMER(ch) > 1) {
        GET_QUEST_TIMER(ch)--;

        if ((GET_QUEST_TIMER(ch) == 1) && ((GET_QUEST_STATUS(ch) == QUEST_NONE) || (GET_QUEST_STATUS(ch) == QUEST_FAILED))) {
          send_to_char("You can start another quest in one tick.\n\r", ch);
        }
      }
      else {
        GET_QUEST_TIMER(ch) = 0;

        if (GET_QUEST_STATUS(ch) == QUEST_RUNNING) {
          if (GET_QUEST_MOB(ch)) {
            GET_QUEST_OWNER(GET_QUEST_MOB(ch)) = NULL;
          }

          if (GET_QUEST_OBJ(ch)) {
            const int aqcard_vnum = 35;

            if (V_OBJ(GET_QUEST_OBJ(ch)) == aqcard_vnum) {
              aqcard_cleanup(GET_ID(ch));
            }
            else {
              OBJ_OWNED_BY(GET_QUEST_OBJ(ch)) = NULL;
            }
          }

          GET_QUEST_GIVER(ch) = NULL;
          GET_QUEST_MOB(ch) = NULL;
          GET_QUEST_OBJ(ch) = NULL;
          GET_QUEST_LEVEL(ch) = 0;
          GET_QUEST_STATUS(ch) = QUEST_FAILED;
          GET_QUEST_TIMER(ch) = 1;

          printf_to_char(ch, "Your time has expired, you have failed your quest! You can start another in %d ticks.\n\r", GET_QUEST_TIMER(ch));
        }
      }

      update_char_objects(ch);

      check_idling(ch);
    }

    gain_condition(ch, DRUNK, -1);
    gain_condition(ch, FULL, -1);
    gain_condition(ch, THIRST, -1);
    gain_condition(ch, QUAFF, -1);

    // Prestige Perk 26
    if (GET_PRESTIGE_PERK(ch) >= 26) {
      gain_condition(ch, QUAFF, -1);
    }

    if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_ANIMATE) && !IS_AFFECTED(ch, AFF_CHARM) && (GET_POS(ch) != POSITION_FIGHTING) && (CHAR_REAL_ROOM(ch) != NOWHERE)) {
      act("$n waves happily and disappears in a puff of smoke.", TRUE, ch, 0, 0, TO_ROOM);

      extract_char(ch);
    }
  }

  /* Objects */
  for (OBJ *obj = object_list, *next_obj; obj; obj = next_obj) {
    next_obj = obj->next;

    bool extract = FALSE;

    if (IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_ALL_DECAY) || IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_EQ_DECAY)) {
      /* Note: ALL_DECAY objects that are equipped or carried decay in update_char_objects() instead of here. */
      if (IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_ALL_DECAY) && (OBJ_TIMER(obj) > 0) && !OBJ_CARRIED_BY(obj) && !OBJ_EQUIPPED_BY(obj)) {
        OBJ_TIMER(obj)--;
      }

      if (OBJ_TIMER(obj) <= 0) {
        if (OBJ_CARRIED_BY(obj)) {
          log_f("WIZINFO: DECAY - Carried by: %s, Obj: %s", GET_NAME(OBJ_CARRIED_BY(obj)), OBJ_SHORT(obj));

          act("$p decays in your inventory.", FALSE, OBJ_CARRIED_BY(obj), obj, 0, TO_CHAR);
        }
        else if (OBJ_EQUIPPED_BY(obj)) {
          log_f("WIZINFO: DECAY - Equipped by: %s, Obj: %s", GET_NAME(OBJ_EQUIPPED_BY(obj)), OBJ_SHORT(obj));

          act("$p decays while it's equipped!", FALSE, OBJ_EQUIPPED_BY(obj), obj, 0, TO_CHAR);
        }
        else if (OBJ_IN_ROOM(obj) != NOWHERE) {
          log_f("WIZINFO: DECAY - In Room: %d, Obj: %s", ROOM_VNUM(OBJ_IN_ROOM(obj)), OBJ_SHORT(obj));

          if (ROOM_PEOPLE(OBJ_IN_ROOM(obj))) {
            act("$p decays, turning to dust.", TRUE, ROOM_PEOPLE(OBJ_IN_ROOM(obj)), obj, 0, TO_CHAR);
            act("$p decays, turning to dust.", TRUE, ROOM_PEOPLE(OBJ_IN_ROOM(obj)), obj, 0, TO_ROOM);
          }
        }

        extract = TRUE;
      }
    }
    else if (IS_STATUE(obj) || IS_CORPSE(obj)) {
      if (OBJ_TIMER(obj) > 0) {
        if (!IS_PC_STATUE(obj) && !IS_PC_CORPSE(obj)) {
          OBJ_TIMER(obj)--;
        }
      }

      if (OBJ_TIMER(obj) <= 0) {
        if (OBJ_CARRIED_BY(obj)) {
          if (IS_STATUE(obj)) {
            act("$p crumbles to dust in your hands.", FALSE, OBJ_CARRIED_BY(obj), obj, 0, TO_CHAR);
          }
          else {
            act("$p decays in your hands.", FALSE, OBJ_CARRIED_BY(obj), obj, 0, TO_CHAR);
          }
        }
        else if (OBJ_EQUIPPED_BY(obj)) {
          if (IS_STATUE(obj)) {
            act("$p crumbles to dust and falls off of you.", FALSE, OBJ_EQUIPPED_BY(obj), obj, 0, TO_CHAR);
          }
          else {
            act("$p decays on you.", FALSE, OBJ_EQUIPPED_BY(obj), obj, 0, TO_CHAR);
          }
        }
        else if (OBJ_IN_ROOM(obj) != NOWHERE) {
          if (ROOM_PEOPLE(OBJ_IN_ROOM(obj))) {
            if (IS_STATUE(obj)) {
              act("$p crumbles to dust.", TRUE, ROOM_PEOPLE(OBJ_IN_ROOM(obj)), obj, 0, TO_CHAR);
              act("$p crumbles to dust.", TRUE, ROOM_PEOPLE(OBJ_IN_ROOM(obj)), obj, 0, TO_ROOM);
            }
            else {
              act("A quivering horde of maggots consumes $p.", TRUE, ROOM_PEOPLE(OBJ_IN_ROOM(obj)), obj, 0, TO_CHAR);
              act("A quivering horde of maggots consumes $p.", TRUE, ROOM_PEOPLE(OBJ_IN_ROOM(obj)), obj, 0, TO_ROOM);
            }
          }
        }

        extract = TRUE;
      }
    }

    if (extract) {
      if ((OBJ_TYPE(obj) == ITEM_CONTAINER) || IS_STATUE(obj) || IS_CORPSE(obj)) {
        for (OBJ *temp_obj = OBJ_CONTAINS(obj), *temp_obj_next; temp_obj; temp_obj = temp_obj_next) {
          temp_obj_next = OBJ_NEXT_CONTENT(temp_obj);

          obj_from_obj(temp_obj);

          if (OBJ_IN_OBJ(obj)) {
            obj_to_obj(temp_obj, OBJ_IN_OBJ(obj));
          }
          else {
            CHAR *temp_ch = NULL;

            if ((temp_ch = OBJ_CARRIED_BY(obj)) || (temp_ch = OBJ_EQUIPPED_BY(obj))) {
              if (OBJ_TYPE(temp_obj) == ITEM_MONEY) {
                int gold = 0;

                if (OBJ_TYPE(temp_obj) == ITEM_MONEY) {
                  gold = OBJ_VALUE(temp_obj, 0);

                  /* Prevent gold overflow. */
                  if (INT_MAX - GET_GOLD(temp_ch) < gold) {
                    gold = INT_MAX - GET_GOLD(temp_ch);
                  }
                }

                GET_GOLD(temp_ch) += gold;
              }
              else {
                obj_to_char(temp_obj, temp_ch);
              }
            }
            else if (OBJ_IN_ROOM(obj) != NOWHERE) {
              obj_to_room(temp_obj, OBJ_IN_ROOM(obj));
            }
            else {
              if (!IS_STATUE(obj) && !IS_CORPSE(obj)) {
                log_f("WIZINFO: DECAY - Container decayed in NOWHERE.");
              }
            }
          }
        }
      }

      extract_obj(obj);
    }
  }
}
