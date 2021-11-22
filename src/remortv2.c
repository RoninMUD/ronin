/*
  remortv2.c - Specs for Remort v2, by Night

  Written by Alan K. Miles for RoninMUD
*/

/* Includes */
/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ronin Includes */
#include "structs.h"
#include "aff_ench.h"
#include "cmd.h"
#include "comm.h"
#include "constants.h"
#include "db.h"
#include "enchant.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "meta.h"
#include "spec_assign.h"
#include "utils.h"

#include "remortv2.h"

/* Simulate a character advancing levels and gaining average hit/mana/move points every time. */
void rv2_sim_advance_level(CHAR *ch) {
  int gain = 0;

  switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
      gain = 5;
      break;

    case CLASS_CLERIC:
      gain = 7;
      break;

    case CLASS_THIEF:
      gain = 9;
      break;

    case CLASS_WARRIOR:
      gain = 12;
      break;

    case CLASS_NINJA:
      gain = 7;
      break;

    case CLASS_NOMAD:
      gain = 15;
      break;

    case CLASS_PALADIN:
      gain = 10;
      break;

    case CLASS_ANTI_PALADIN:
      gain = 8;
      break;

    case CLASS_AVATAR:
      gain = 15;
      break;

    case CLASS_BARD:
      gain = 8;
      break;

    case CLASS_COMMANDO:
      gain = 9;
      break;
  }

  adjust_hit_mana_move(ch, META_HIT, gain);

  gain = 0;

  switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
    case CLASS_AVATAR:
    case CLASS_BARD:
      gain = 4;
      break;

    case CLASS_NINJA:
    case CLASS_PALADIN:
    case CLASS_ANTI_PALADIN:
    case CLASS_COMMANDO:
      gain = 3;
      break;
  }

  adjust_hit_mana_move(ch, META_MANA, gain);

  gain = 0;

  adjust_hit_mana_move(ch, META_MOVE, 3);
}

/* Adjust a character's remort experience. Don't use this directly unless you know what you're doing. */
int rv2_adjust_remort_exp(CHAR *ch, int exp) {
  GET_REMORT_EXP(ch) += exp;

  /* Did the player finish the remort process? */
  if (GET_REMORT_EXP(ch) <= 0) {
    GET_REMORT_EXP(ch) = 0;

    send_to_char("Congratulations! You have completed the remort process!\n\r", ch);

    printf_to_world_except(ch, "%s has completed the remort process! All hail %s!\n\r", GET_DISP_NAME(ch), GET_DISP_NAME(ch));

    rv2_remove_enchant(ch);
  }

  return exp;
}

/* Calculate a character's remort multiplier. */
int rv2_calc_remort_mult(CHAR *ch) {
  int mult = RV2_EXP_MULTIPLIER;

  if (!ch) return mult;

  /* Attribute-based increases. */

  if (GET_OSTR(ch) >= 18) {
    mult += 1;

    if (GET_OADD(ch) >= 50) mult += 1;
    if (GET_OADD(ch) == 100) mult += 2;
  }

  if (GET_ODEX(ch) >= 18) mult += 1;
  if (GET_OINT(ch) >= 18) mult += 1;
  if (GET_OWIS(ch) >= 18) mult += 1;
  if (GET_OCON(ch) >= 18) mult += 1;

  /* Level-based increases. */

  if (GET_LEVEL(ch) >= 30) mult += 1;
  if (GET_LEVEL(ch) >= 40) mult += 1;
  if (GET_LEVEL(ch) >= 45) mult += 2;
  if (GET_LEVEL(ch) >= 50) mult += 3;

  /* Stat-based increase */

  int prestige_hit = 0, prestige_mana = 0;

  if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
    prestige_hit = GET_PRESTIGE(ch) * (PRESTIGE_HIT_GAIN + PRESTIGE_MANA_GAIN);
    prestige_mana = 0;
  }
  else {
    prestige_hit = GET_PRESTIGE(ch) * PRESTIGE_HIT_GAIN;
    prestige_mana = GET_PRESTIGE(ch) * PRESTIGE_MANA_GAIN;
  }

  int hp_min = 0, hp_step = 0, mana_min = 0, mana_step = 0;

  if ((GET_CLASS(ch) == CLASS_NOMAD) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_THIEF)) {
    hp_min = 500;
    hp_step = 300;
    mana_min = 0;
    mana_step = 0;
  }
  else {
    hp_min = 500;
    hp_step = 250;
    mana_min = 500;
    mana_step = 250;
  }

  /* Compensate for Prestige stats. */
  if ((prestige_hit >= 0) && (hp_min > 0) && (hp_step > 0)) {
    for (int adjusted_hp = GET_NAT_HIT(ch) - prestige_hit - hp_min, temp_hp = 0, mod = 1; (temp_hp + hp_step < adjusted_hp) && (mult < RV2_MAX_EXP_MULTIPLIER); temp_hp += hp_step) {
      mod *= 2;
      mult += mod;
    }
  }

  if ((prestige_mana >= 0) && (mana_min > 0) && (mana_step > 0)) {
    for (int adjusted_mana = GET_NAT_MANA(ch) - prestige_mana - mana_min, temp_mana = 0, mod = 1; (temp_mana + mana_step < adjusted_mana) && (mult < RV2_MAX_EXP_MULTIPLIER); temp_mana += mana_step) {
      mod *= 2;
      mult += mod;
    }
  }

  mult = MAX(MIN(mult, RV2_MAX_EXP_MULTIPLIER), RV2_EXP_MULTIPLIER);

  // Prestige Perk 1
  if (GET_PRESTIGE_PERK(ch) >= 1) {
    mult += 1;
  }

  return mult;
}

/* Give the character remort experience if they deserve it and return how much experience was given. */
int rv2_gain_remort_exp(CHAR *ch, int exp) {
  if (GET_REMORT_EXP(ch) <= 0) return 0;

  /* Limit the experience gain. */
  if (exp > RV2_MAX_EXP_GAIN) {
    exp = RV2_MAX_EXP_GAIN;
  }

  /* Multiply the experience gained by the character's remort multiplier. */
  exp *= rv2_calc_remort_mult(ch);

  /* Limit experience gain to the character's available remort experience pool. */
  if (exp > GET_REMORT_EXP(ch)) {
    exp = (int)GET_REMORT_EXP(ch);
  }

  /* Deduct the experience from the character's remort experience pool. */
  rv2_adjust_remort_exp(ch, -exp);

  /* Give the experience to the character. */
  gain_exp(ch, exp);

  return exp;
}

/* Return the remort experience pool of a character, based on multiple factors. */
long long int rv2_meta_sim(CHAR *ch) {
  CHAR *sim = NULL;
  int i = 0, j = 0, stat_total = 0;
  long long int exp = 0;

#ifdef TEST_SITE
  char buf[MSL];
#endif

  if (GET_LEVEL(ch) < 6) return 0;

  /* Create a sim character. */
  CREATE(sim, CHAR, 1);
  clear_char(sim);
  reset_char(sim);

  /* Set the sim character to level 0 and set it to the same class as the character. */
  GET_LEVEL(sim) = 0;
  GET_CLASS(sim) = GET_CLASS(ch);

  /* Set the sim character's points to the base for a level 0 character. */
  sim->points.max_hit = 10;
  sim->specials.org_hit = 10;
  sim->points.max_mana = 0;
  sim->specials.org_mana = 0;
  sim->points.max_move = 0;
  sim->specials.org_move = 0;

  /* Set the sim character's stats to the average stat to be used for simulation. */
  GET_OSTR(sim) = MIN(GET_OSTR(ch), RV2_SIM_AVG_STAT);
  GET_OINT(sim) = MIN(GET_OINT(ch), RV2_SIM_AVG_STAT);
  GET_OWIS(sim) = MIN(GET_OWIS(ch), RV2_SIM_AVG_STAT);
  GET_ODEX(sim) = MIN(GET_ODEX(ch), RV2_SIM_AVG_STAT);
  GET_OCON(sim) = MIN(GET_OCON(ch), RV2_SIM_AVG_STAT);
  GET_OADD(sim) = 0;

  affect_total(sim);

  /* Calculate experience required for the character's level and the average point gains from such levels. */
  for (i = 0; GET_LEVEL(sim) < LEVEL_MORT && GET_LEVEL(sim) < GET_LEVEL(ch); i++) {
    exp += rv2_adjust_remort_exp(sim, exp_table[GET_LEVEL(sim) + 1]);

    rv2_sim_advance_level(sim);
    GET_LEVEL(sim)++;

    affect_total(sim);
  }

  adjust_hit_mana_move(sim, META_HIT, (GET_LEVEL(sim) * 3));

  affect_total(sim);

#ifdef TEST_SITE

  snprintf(buf, sizeof(buf), "Level EXP to Pool........ %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  snprintf(buf, sizeof(buf), "Base Hit:  %5d, Target Hit:  %5d\n\r",
    get_max_stat(sim, META_HIT), get_max_stat(ch, META_HIT));
  send_to_char(buf, ch);

#endif

  /* Calculate the average number of hit point metas required to match the character's hit points, based on a 4.5 point average. */
  i = 0;
  exp = 0;
  if (get_max_stat(sim, META_HIT) < get_max_stat(ch, META_HIT)) {
    for (i = 0; get_max_stat(sim, META_HIT) < get_max_stat(ch, META_HIT); i++) {
      exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_HIT));

      if (i % 2) {
        adjust_hit_mana_move(sim, META_HIT, 4);
      }
      else {
        adjust_hit_mana_move(sim, META_HIT, 5);
      }

      affect_total(sim);
    }
  }

#ifdef TEST_SITE

  snprintf(buf, sizeof(buf), "Avg. No. of Hit Metas.... %d\n\r", i);
  send_to_char(buf, ch);

  snprintf(buf, sizeof(buf), "Hit Meta EXP to Pool..... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  snprintf(buf, sizeof(buf), "Base Mana: %5d, Target Mana: %5d\n\r",
    get_max_stat(sim, META_MANA), get_max_stat(ch, META_MANA));
  send_to_char(buf, ch);

#endif

  /* Calculate the average number of mana metas required to match the character's mana, based on a 4.5 point average. */
  i = 0;
  exp = 0;
  if (get_max_stat(sim, META_MANA) < get_max_stat(ch, META_MANA)) {
    for (i = 0; get_max_stat(sim, META_MANA) < get_max_stat(ch, META_MANA); i++) {
      exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_MANA));

      if (i % 2) {
        adjust_hit_mana_move(sim, META_MANA, 4);
      }
      else {
        adjust_hit_mana_move(sim, META_MANA, 5);
      }

      affect_total(sim);
    }
  }

#ifdef TEST_SITE

  snprintf(buf, sizeof(buf), "Avg. No. of Mana Metas... %d\n\r", i);
  send_to_char(buf, ch);

  snprintf(buf, sizeof(buf), "Mana Meta EXP to Pool.... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  snprintf(buf, sizeof(buf), "Base Move: %5d, Target Move: %5d\n\r",
    get_max_stat(sim, META_MOVE), get_max_stat(ch, META_MOVE));
  send_to_char(buf, ch);

#endif

  /* Calculate the average number of move metas required to match the character's movement points. */
  i = 0;
  exp = 0;
  if (get_max_stat(sim, META_MOVE) < get_max_stat(ch, META_MOVE)) {
    for (i = 0; get_max_stat(sim, META_MOVE) < get_max_stat(ch, META_MOVE); i++) {
      if (((get_max_stat(sim, META_MOVE)) + 30) <= get_max_stat(ch, META_MOVE)) {
        exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_MOVE));

        adjust_hit_mana_move(sim, META_MOVE, 30);
      }
      else {
        exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_MOVE_UP_1));

        adjust_hit_mana_move(sim, META_MOVE_UP_1, 1);
      }

      affect_total(sim);
    }
  }

#ifdef TEST_SITE

  snprintf(buf, sizeof(buf), "Move Meta EXP to Pool.... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  /* Calculate the character's stat total. */
  i = 0;
  exp = 0;
  for (i = META_STR; i <= META_CON; i++) {
    stat_total += get_ability(ch, i);
  }

  /* Calculate the number of metas required to match the character's stats if their stat total is above 65. */
  i = 0;
  exp = 0;
  if (stat_total > (RV2_SIM_AVG_STAT * 5) || get_ability(ch, META_STR_ADD) > 0) {
    for (i = META_STR; i <= META_CON; i++) {
      if (get_ability(sim, i) < RV2_SIM_AVG_STAT) {
        for (j = 0; get_ability(sim, i) < RV2_SIM_AVG_STAT; j++) {
          exp += rv2_adjust_remort_exp(sim, -meta_cost(sim, i));

          adjust_ability(sim, i, 1);

          affect_total(sim);
        }
      }
      else {
        for (j = 0; get_ability(sim, i) < get_ability(ch, i); j++) {
          exp += rv2_adjust_remort_exp(sim, meta_cost(sim, i));

          adjust_ability(sim, i, 1);

          affect_total(sim);
        }
      }
    }

    /* Calculate the amount of experience invested in STR_ADD metas. */
    if (get_ability(ch, META_STR_ADD) > 0) {
      for (i = 0; get_ability(sim, META_STR_ADD) < get_ability(ch, META_STR_ADD); i++) {
        exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_STR_ADD));

        adjust_ability(sim, META_STR_ADD, 1);

        affect_total(sim);
      }
    }
  }

#ifdef TEST_SITE

  snprintf(buf, sizeof(buf), "Stat Meta EXP to Pool.... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  /* Calculate the amount of experience invested in ranks. */
  i = 0;
  exp = 0;
  if (get_rank(ch) > 0) {
    for (i = 0; i < get_rank(ch); i++) {
      exp += rv2_adjust_remort_exp(sim, 5000000 * (i + 1));
    }
  }

#ifdef TEST_SITE

  snprintf(buf, sizeof(buf), "Rank EXP to Pool......... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  /* Add the character's current experience pool to the remort experience pool. */
  exp = 0;
  if (GET_EXP(ch) > 0) {
    exp = rv2_adjust_remort_exp(sim, GET_EXP(ch));
  }

#ifdef TEST_SITE

  snprintf(buf, sizeof(buf), "Normal EXP to Pool....... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  /* Add the character's current death experience pool to the remort experience pool. */
  exp = 0;
  if (GET_DEATH_EXP(ch) > 0) {
    exp = rv2_adjust_remort_exp(sim, GET_DEATH_EXP(ch));
  }

#ifdef TEST_SITE

  snprintf(buf, sizeof(buf), "Death EXP to Pool........ %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

#ifdef TEST_SITE

  snprintf(buf, sizeof(buf), "Remort v2 EXP Pool....... %ld\n\r", GET_REMORT_EXP(sim));
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  snprintf(buf, sizeof(buf), "Avg. Stat Used........... %d\n\r", RV2_SIM_AVG_STAT);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  send_to_char("Simulated Final Stats\n\r", ch);
  send_to_char("---------------------\n\r", ch);
  snprintf(buf, sizeof(buf), "Level... %d\n\r", GET_LEVEL(sim));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Class... %s\n\r", GET_CLASS_NAME(ch));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Rank.... %d\n\r", get_rank(ch));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Hit..... %d\n\r", GET_MAX_HIT(sim));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Mana.... %d\n\r", GET_MAX_MANA(sim));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Move.... %d\n\r", GET_MAX_MOVE(sim));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Str..... %d\n\r", GET_OSTR(sim));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Int..... %d\n\r", GET_OINT(sim));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Wis..... %d\n\r", GET_OWIS(sim));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Dex..... %d\n\r", GET_ODEX(sim));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Con..... %d\n\r", GET_OCON(sim));
  send_to_char(buf, ch);
  snprintf(buf, sizeof(buf), "Add..... %d\n\r", GET_OADD(sim));
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  exp = GET_REMORT_EXP(sim);

  free_char(sim);

  return exp;
}

/* Misc Functions */

/* Utility function to make things a little cleaner down below. */
int rv2_get_ability_by_apply(CHAR *ch, int ability_apply) {
  int value = 0;

  switch (ability_apply) {
    case APPLY_STR:
      value = GET_OSTR(ch);
      break;

    case APPLY_DEX:
      value = GET_ODEX(ch);
      break;

    case APPLY_INT:
      value = GET_OINT(ch);
      break;

    case APPLY_WIS:
      value = GET_OWIS(ch);
      break;

    case APPLY_CON:
      value = GET_OCON(ch);
      break;
  }

  return value;
}

/* Extract remort token. */
void rv2_extract_token(OBJ *obj) {
  if (!obj) return;

  if (OBJ_CARRIED_BY(obj)) {
    printf_to_char(OBJ_CARRIED_BY(obj), "%s vanishes from existence.\n\r", OBJ_SHORT(obj));

    obj_from_char(obj);
  }
  else if (OBJ_IN_ROOM(obj)) {
    printf_to_room(OBJ_IN_ROOM(obj), "%s vanishes from existence.\n\r", OBJ_SHORT(obj));

    obj_from_room(obj);
  }

  extract_obj(obj);
}

/* Determines what remort tokens, if any, a character has and what their choices are based on those tokens. */
struct rv2_token_info rv2_get_token_info(CHAR *ch) {
  struct rv2_token_info token_info = { 0 };

  token_info.class_num = GET_CLASS(ch);

  for (OBJ *tmp_obj = GET_CARRYING(ch); tmp_obj; tmp_obj = OBJ_NEXT_CONTENT(tmp_obj)) {
    if (V_OBJ(tmp_obj) < RV2_TOKEN_MIN_VNUM || V_OBJ(tmp_obj) > RV2_TOKEN_MAX_VNUM) continue;
    if (OBJ_VALUE0(tmp_obj) != GET_ID(ch)) continue;
    if (token_info.alignment_token && (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_GOOD || V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_EVIL)) continue;
    if (token_info.gender_token && (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_MALE || V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_FEMALE)) continue;
    if (token_info.class_token && (V_OBJ(tmp_obj) >= RV2_OBJ_TOKEN_MAGE && V_OBJ(tmp_obj) <= RV2_OBJ_TOKEN_COMMANDO)) continue;

    if (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_GOOD || V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_EVIL) {
      token_info.alignment_token = tmp_obj;
    }
    else if (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_MALE || V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_FEMALE) {
      token_info.gender_token = tmp_obj;
    }
    else if (V_OBJ(tmp_obj) >= RV2_OBJ_TOKEN_MAGE && V_OBJ(tmp_obj) <= RV2_OBJ_TOKEN_COMMANDO) {
      token_info.class_token = tmp_obj;
    }
  }

  if (token_info.alignment_token) {
    if (V_OBJ(token_info.alignment_token) == RV2_OBJ_TOKEN_GOOD) {
      token_info.alignment = 500;
      strncpy(token_info.alignment_name, "good", sizeof(token_info.gender_name));
    }
    else if (V_OBJ(token_info.alignment_token) == RV2_OBJ_TOKEN_EVIL) {
      token_info.alignment = -500;
      strncpy(token_info.alignment_name, "evil", sizeof(token_info.gender_name));
    }
  }

  if (token_info.gender_token) {
    if (V_OBJ(token_info.gender_token) == RV2_OBJ_TOKEN_MALE) {
      token_info.gender = SEX_MALE;
      strncpy(token_info.gender_name, "male", sizeof(token_info.gender_name));
    }
    else if (V_OBJ(token_info.gender_token) == RV2_OBJ_TOKEN_FEMALE) {
      token_info.gender = SEX_FEMALE;
      strncpy(token_info.gender_name, "female", sizeof(token_info.gender_name));
    }
  }

  if (token_info.class_token) {
    if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_MAGE) {
      token_info.class_num = CLASS_MAGIC_USER;
      strncpy(token_info.class_name, "mage", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_CLERIC) {
      token_info.class_num = CLASS_CLERIC;
      strncpy(token_info.class_name, "cleric", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_THIEF) {
      token_info.class_num = CLASS_THIEF;
      strncpy(token_info.class_name, "thief", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_WARRIOR) {
      token_info.class_num = CLASS_WARRIOR;
      strncpy(token_info.class_name, "warrior", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_NINJA) {
      token_info.class_num = CLASS_NINJA;
      strncpy(token_info.class_name, "ninja", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_NOMAD) {
      token_info.class_num = CLASS_NOMAD;
      strncpy(token_info.class_name, "nomad", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_PALADIN) {
      token_info.class_num = CLASS_PALADIN;
      strncpy(token_info.class_name, "paladin", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_ANTI_PALADIN) {
      token_info.class_num = CLASS_ANTI_PALADIN;
      strncpy(token_info.class_name, "anti-paladin", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_BARD) {
      token_info.class_num = CLASS_BARD;
      strncpy(token_info.class_name, "bard", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_COMMANDO) {
      token_info.class_num = CLASS_COMMANDO;
      strncpy(token_info.class_name, "commando", sizeof(token_info.class_name));
    }
  }

  return token_info;
}

/* Determines if a character has the right tokens to remort. If not, prints some messages. */
int rv2_token_check(CHAR *ch, struct rv2_token_info token_info) {
  if (!token_info.gender_token || !token_info.class_token) {
    if (!token_info.gender_token && !token_info.class_token) {
      send_to_char("`qA pensive frown is seen briefly on Immortalis' face as he considers your request.\n\r", ch);
      send_to_char("He whispers, 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation without first making the necessary preparations.'\n\r", ch);
      send_to_char("He continues, 'If you do wish to proceed, I humbly request you at least purchase `kgender`q and `nclass`q tokens of your own choosing.'\n\r", ch);
    }
    else if (token_info.gender_token && !token_info.class_token) {
      send_to_char("`qImmortalis gives you a pensive glance as he calmly places a token into the palm of your hand.\n\r", ch);
      printf_to_char(ch, "He whispers, 'A `k%s`q it shall be, but what of the `nclass`q?'\n\r",
        token_info.gender_name);
    }
    else if (!token_info.gender_token && token_info.class_token) {
      send_to_char("`qImmortalis gives you a wistful smile as he gently places a token into the palm of your hand.\n\r", ch);
      printf_to_char(ch, "He whispers, 'A `n%s`q it shall be, but what of the `kgender`q?'\n\r",
        token_info.class_name);
    }

    return FALSE;
  }

  return TRUE;
}

/* Calculate the character's remort experience pool, their QP and SCP, and any fees they must pay in order to remort. */
struct rv2_remort_info rv2_appraise(CHAR *ch) {
  struct rv2_remort_info remort_info = { 0 };

  remort_info.exp = rv2_meta_sim(ch);

  /* Calculate the amount of QP invested in the character's stats. */
  for (int i = APPLY_STR; i <= APPLY_CON; i++) {
    if (rv2_get_ability_by_apply(ch, i) < 19) continue;

    for (int j = 19; j <= rv2_get_ability_by_apply(ch, i); j++) {
      remort_info.qp += (j - 18) * 200;
    }
  }

  remort_info.qp += GET_QP(ch); /* Add in their current QP pool. */

  /* Calculate the amount of SCP invested in the character's subclass levels. */
  for (int i = 1; i <= GET_SC_LEVEL(ch); i++) {
    remort_info.scp += i * 70;
  }

  remort_info.scp += GET_SCP(ch); /* Add in their current SCP pool. */

  /* Determine the fee to be charged for the remort process (if any). */
  if (!FREEMORT) {
    if (remort_info.exp > RV2_MIN_EXP_COST) {
      long long int tmp = RV2_MAX_EXP_COST;
      int chunk = 0;

      if (remort_info.exp > tmp) {
        remort_info.qp_fee = RV2_MAX_QP_COST;
        remort_info.scp_fee = RV2_MAX_SCP_COST;
      }
      else {
        tmp = (remort_info.exp - RV2_MIN_EXP_COST);
        chunk = ((RV2_MAX_EXP_COST - RV2_MIN_EXP_COST) / RV2_MAX_QP_COST);

        for (remort_info.qp_fee = 0; tmp > chunk; tmp -= chunk, remort_info.qp_fee++);

        tmp = (remort_info.exp - RV2_MIN_EXP_COST);
        chunk = ((RV2_MAX_EXP_COST - RV2_MIN_EXP_COST) / RV2_MAX_SCP_COST);

        for (remort_info.scp_fee = 0; tmp > chunk; tmp -= chunk, remort_info.scp_fee++);
      }
    }

    // Prestige Perk 13
    if (GET_PRESTIGE_PERK(ch) >= 13) {
      remort_info.qp_fee *= 0.9;
      remort_info.scp_fee *= 0.9;
    }
  }

  return remort_info;
}

/* Print some messages based on the results of appraise. */
void rv2_appraise_message(CHAR *ch, struct rv2_remort_info remort_info) {
  long long int cost = RV2_MIN_EXP_COST;

  if (FREEMORT) {
    send_to_char("Immortalis whispers, 'To transcend this existence, I will require no level of compensation, as today celebrates the efforts of the fallen on behalf of us all.'\n\r", ch);
    printf_to_char(ch, "He continues, 'For my part however, I will return to you a sum of %lld experience points, %d quest points, and %d subclass points.'\n\r",
      remort_info.exp, remort_info.qp, remort_info.scp);
  }
  else if (remort_info.exp > cost) {
    send_to_char("Immortalis whispers 'Commensurate to your achievements, so too shall be the measure of my fee.'\n\r", ch);

    if (remort_info.qp_fee > remort_info.qp || remort_info.scp_fee > remort_info.scp) {
      printf_to_char(ch, "Immortalis whispers, 'To transcend this existence, I will require compensation in the form of %d quest points and %d subclass points, which I see you do not possess.'\n\r",
        remort_info.qp_fee, remort_info.scp_fee);
      printf_to_char(ch, "He continues, 'Were you to acquire it however, I would return to you a sum of %lld experience points.'\n\r",
        remort_info.exp);
    }
    else {
      printf_to_char(ch, "Immortalis whispers, 'To transcend this existence, I will require compensation in the form of %d quest points and %d subclass points.'\n\r",
        remort_info.qp_fee, remort_info.scp_fee);
      printf_to_char(ch, "He continues, 'For my part, I will return to you a sum of %lld experience points, %d quest points, and %d subclass points.'\n\r",
        remort_info.exp, MAX(0, remort_info.qp - remort_info.qp_fee), MAX(0, remort_info.scp - remort_info.scp_fee));
    }
  }
  else {
    send_to_char("Immortalis whispers, 'To transcend this existence, I will require no level of compensation, as your achievements do not yet merit excessive effort on my behalf.'\n\r", ch);
    printf_to_char(ch, "He continues, 'For my part however, I will return to you a sum of %lld experience points, %d quest points, and %d subclass points.'\n\r",
      remort_info.exp, remort_info.qp, remort_info.scp);
  }
}

/* Add the remort enchant. */
void rv2_add_enchant(CHAR *ch) {
  ENCH remort_ench = { 0 };

  remort_ench.type = ENCHANT_REMORTV2;

  enchantment_to_char(ch, &remort_ench, TRUE);
}

/* Remove the remort enchant. */
void rv2_remove_enchant(CHAR *ch) {
  ENCH *remort_ench = ench_get_from_char(ch, NULL, ENCHANT_REMORTV2);

  if (remort_ench) {
    send_to_char("You are no longer affected by Remort.\n\r", ch);

    ench_remove(ch, remort_ench, FALSE);
  }
}

/* Mobile Specs */

/* Responsible for the core of the remort system. */
int rv2_mob_spec_immortalis(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  char buf[MSL];
  struct rv2_remort_info remort_info;
  struct rv2_token_info token_info;

  /* Buying tokens is handled as a spec due to shopkeeper limitations and a few other requirements. */
  if (cmd == CMD_BUY) {
    if (!ch || IS_NPC(ch)) return FALSE;

    if (CHAR_VIRTUAL_ROOM(mob) != RV2_WLD_SANCTUM_ETERNITY) return FALSE;

    if (GET_LEVEL(ch) < 6) {
      send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request.'\n\r", ch);
      send_to_char("Immortalis whispers 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste.'\n\r", ch);
      send_to_char("He continues, 'If you do indeed endeavor to transcend this existence and into the next, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

      return TRUE;
    }

    if (IS_IMMORTAL(ch)) {
      send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

      return TRUE;
    }

    one_argument(arg, buf);

    int vnum = -1;

    if (!str_cmp(buf, "good")) vnum = RV2_OBJ_TOKEN_GOOD;
    else if (!str_cmp(buf, "evil")) vnum = RV2_OBJ_TOKEN_EVIL;
    else if (!str_cmp(buf, "male")) vnum = RV2_OBJ_TOKEN_MALE;
    else if (!str_cmp(buf, "female")) vnum = RV2_OBJ_TOKEN_FEMALE;
    else if (!str_cmp(buf, "mage")) vnum = RV2_OBJ_TOKEN_MAGE;
    else if (!str_cmp(buf, "cleric")) vnum = RV2_OBJ_TOKEN_CLERIC;
    else if (!str_cmp(buf, "thief")) vnum = RV2_OBJ_TOKEN_THIEF;
    else if (!str_cmp(buf, "warrior")) vnum = RV2_OBJ_TOKEN_WARRIOR;
    else if (!str_cmp(buf, "ninja")) vnum = RV2_OBJ_TOKEN_NINJA;
    else if (!str_cmp(buf, "nomad")) vnum = RV2_OBJ_TOKEN_NOMAD;
    else if (!str_cmp(buf, "paladin")) vnum = RV2_OBJ_TOKEN_PALADIN;
    else if (!str_cmp(buf, "anti-paladin")) vnum = RV2_OBJ_TOKEN_ANTI_PALADIN;
    else if (!str_cmp(buf, "bard")) vnum = RV2_OBJ_TOKEN_BARD;
    else if (!str_cmp(buf, "commando")) vnum = RV2_OBJ_TOKEN_COMMANDO;
    else {
      send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request.\n\r", ch);
      send_to_char("Immortalis whispers, 'While I appreciate your enthusiasm, I simply cannot provide that which you request.'\n\r", ch);

      return TRUE;
    }

    /* Ensure that a character can't buy more than one of each type of token. */
    bool deny = FALSE;

    for (OBJ *tmp_obj = GET_CARRYING(ch); tmp_obj && !deny; tmp_obj = OBJ_NEXT_CONTENT(tmp_obj)) {
      if (V_OBJ(tmp_obj) < RV2_TOKEN_MIN_VNUM || V_OBJ(tmp_obj) > RV2_TOKEN_MAX_VNUM) continue;

      if (((vnum == RV2_OBJ_TOKEN_GOOD || vnum == RV2_OBJ_TOKEN_EVIL) && (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_GOOD || V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_EVIL)) ||
        ((vnum == RV2_OBJ_TOKEN_MALE || vnum == RV2_OBJ_TOKEN_FEMALE) && (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_MALE || V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_FEMALE)) ||
        ((vnum >= RV2_OBJ_TOKEN_MAGE && vnum <= RV2_OBJ_TOKEN_COMMANDO) && (V_OBJ(tmp_obj) >= RV2_OBJ_TOKEN_MAGE && V_OBJ(tmp_obj) <= RV2_OBJ_TOKEN_COMMANDO))) {
        deny = TRUE;
      }
    }

    if (deny) {
      send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request.\n\r", ch);
      send_to_char("Immortalis whispers, 'While I appreciate your enthusiasm, I simply cannot allow you more than a single token of that variety.\n\r", ch);

      return TRUE;
    }

    /* Load up the token and give it the character. */
    OBJ *token = read_object(vnum, VIRTUAL);

    if (!token) {
      snprintf(buf, sizeof(buf), "WIZINFO: Error loading object (#%d) in rv2_mob_spec_immortalis(), cmd == CMD_BUY.", vnum);

      wizlog(buf, LEVEL_SUP, 5);
      log_f("%s", buf);

      return TRUE;
    }

    /* Set the token's Value 0 to the unique ID of the character a safety precaution, and set its timer to 10. */
    OBJ_VALUE0(token) = GET_ID(ch);
    OBJ_TIMER(token) = 10;

    obj_to_char(token, ch);

    /* Find out what tokens the character has now. */
    token_info = rv2_get_token_info(ch);

    if (token_info.alignment_token) {
      send_to_char("`qImmortalis gives you an understanding nod as he calmly places a token into the palm of your hand.\n\r", ch);
      printf_to_char(ch, "He whispers, 'I see you have chosen the path of `l%s`q.'\n\r",
        token_info.alignment_name);
    }

    /* If the character has the proper tokens for a remort, print out the next steps. */
    if (rv2_token_check(ch, token_info)) {
      send_to_char("Immortalis places yet another token into your hand and a faint clink is briefly heard as he clasps his hands over yours, sealing the glimmering metal inside.\n\r", ch);
      printf_to_char(ch, "He whispers, 'A `k%s`q `n%s`q is it? Consider wisely, for this path leads in only a singular direction.'\n\r",
        token_info.gender_name, token_info.class_name);

      /* Calculate the player's remort info and print out some messages if needed. */
      remort_info = rv2_appraise(ch);

      rv2_appraise_message(ch, remort_info);

      /* The player can remort, so we'll tell them such. */
      if (remort_info.qp_fee <= remort_info.qp && remort_info.scp_fee <= remort_info.scp) {
        send_to_char("Immortalis whispers, 'If you see merit in my offer, you need only issue the command `jremort commit`q and I shall see it done.'\n\r", ch);
        send_to_char("He continues, 'However, if you were mistaken in your choices and do not wish to undergo this transformation, you may simply walk away.'\n\r", ch);
        send_to_char("He concludes, 'Or, to make it more official, issue the command `iremort cancel`q and I will proceed as though this conversation never happened.'\n\r", ch);
      }
    }

    return TRUE;
  }

  if (cmd == CMD_LIST) {
    if (!ch || IS_NPC(ch)) return FALSE;

    if (CHAR_VIRTUAL_ROOM(mob) != RV2_WLD_SANCTUM_ETERNITY) return FALSE;

    if (GET_LEVEL(ch) < 6) {
      send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request.\n\r", ch);
      send_to_char("Immortalis whispers, 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste.'\n\r", ch);
      send_to_char("He continues, 'If you do indeed endeavor to transcend this existence and into the next, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

      return TRUE;
    }

    if (IS_IMMORTAL(ch)) {
      send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

      return TRUE;
    }

    send_to_char("You can buy:\n\r", ch);

    int count = 0;

    for (int vnum = RV2_TOKEN_MIN_VNUM; vnum <= RV2_TOKEN_MAX_VNUM; vnum++) {
      OBJ *token = read_object(vnum, VIRTUAL);

      if (!token) {
        snprintf(buf, sizeof(buf), "WIZINFO: Error loading object (#%d) in rv2_mob_spec_immortalis(), cmd == CMD_LIST.", vnum);

        wizlog(buf, LEVEL_SUP, 5);
        log_f("%s", buf);

        return FALSE;
      }

      count++;

      printf_to_char(ch, "%s\n\r", OBJ_SHORT(token));

      extract_obj(token);
    }

    if (!count) {
      send_to_char("Nothing.\n\r", ch);
    }

    return TRUE;
  }

  if (cmd == CMD_UNKNOWN) {
    if (!ch || IS_NPC(ch)) return FALSE;

    if (CHAR_VIRTUAL_ROOM(mob) != RV2_WLD_SANCTUM_ETERNITY) return FALSE;

    char arg1[MIL], arg2[MIL];

    two_arguments(arg, arg1, arg2);

    /* The player can type "appraise" to see what their remort experience pool would be, and what it would cost them to remort. */
    if (!str_cmp(arg1, "appraise") || (!str_cmp(arg1, "remort") && !str_cmp(arg2, "appraise"))) {
      if (GET_LEVEL(ch) < 6) {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request.\n\r", ch);
        send_to_char("Immortalis whispers, 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste.'\n\r", ch);
        send_to_char("He continues, 'If you do indeed endeavor to proceed further, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

        return TRUE;
      }

      if (IS_IMMORTAL(ch)) {
        send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

        return TRUE;
      }

      /* Calculate the player's remort info and print out some messages if needed. */
      remort_info = rv2_appraise(ch);

      rv2_appraise_message(ch, remort_info);

      return TRUE;
    }

    /* Cancel the player's intention of remorting by clearing their inventory of any remort tokens purchased. */
    if (!str_cmp(arg1, "cancel") || (!str_cmp(arg1, "remort") && !str_cmp(arg2, "cancel"))) {
      if (GET_LEVEL(ch) < 6) {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request.\n\r", ch);
        send_to_char("Immortalis whispers, 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste.'\n\r", ch);
        send_to_char("He continues, 'If you do indeed endeavor to proceed further, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

        return TRUE;
      }

      if (IS_IMMORTAL(ch)) {
        send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

        return TRUE;
      }

      send_to_char("A slight nod of reverent understanding is made as \n\r", ch);
      send_to_char("Immortalis whispers, 'As you wish. If you do endeavour to proceed down the path of transcendence in the future, you need only inquire.'\n\r", ch);

      for (OBJ *temp_obj = GET_CARRYING(ch), *next_obj = NULL; temp_obj; temp_obj = next_obj) {
        next_obj = OBJ_NEXT_CONTENT(temp_obj);

        if (V_OBJ(temp_obj) < RV2_TOKEN_MIN_VNUM || V_OBJ(temp_obj) > RV2_TOKEN_MAX_VNUM) continue;

        rv2_extract_token(temp_obj);
      }

      return TRUE;
    }

    /* Enable or Disable earning of remort experience. */
    if (!str_cmp(arg1, "remort") && (!str_cmp(arg2, "enable") || !str_cmp(arg2, "disable"))) {
      if (IS_IMMORTAL(ch)) {
        send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

        return TRUE;
      }

      if (GET_REMORT_EXP(ch) <= 0) {
        send_to_char("Immortalis' brow furrows in obvious confusion at your request.\n\r", ch);
        send_to_char("He whispers, 'Alas, any echoes of your past experiences have faded to but a distant memory. As such, I cannot grant your request.'\n\r", ch);

        return TRUE;
      }

      if (!str_cmp(arg2, "enable")) {
        REMOVE_BIT(GET_PFLAG(ch), PLR_REMORT_DISABLED);

        send_to_char("Immortalis nods and whispers, 'As you wish. You will now gain additional experience augmented by the echoes of your past.'\n\r", ch);

        return TRUE;
      }

      if (!str_cmp(arg2, "disable")) {
        SET_BIT(GET_PFLAG(ch), PLR_REMORT_DISABLED);

        send_to_char("Immortalis nods and whispers, 'As you wish. You will no longer gain additional experience augmented by the echoes of your past.'\n\r", ch);

        return TRUE;
      }

      return TRUE;
    }

    /* The character is serious about remorting. Let's do it. */
    if (!str_cmp(arg1, "remort") && !str_cmp(arg2, "commit")) {
      if (GET_LEVEL(ch) < 6) {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request.\n\r", ch);
        send_to_char("Immortalis whispers, 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste.'\n\r", ch);
        send_to_char("He continues, 'If you do indeed endeavor to proceed further, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

        return TRUE;
      }

      if (IS_IMMORTAL(ch)) {
        send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

        return TRUE;
      }

      /* Calculate the character's remort experience pool and what it's going to cost them to remort. */
      remort_info = rv2_appraise(ch);

      if (remort_info.qp_fee > remort_info.qp || remort_info.scp_fee > remort_info.scp) {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request.\n\r", ch);
        send_to_char("Immortalis whispers, 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation without the proper level of compensation for my efforts.'\n\r", ch);

        return TRUE;
      }

      /* Check the character's tokens and make sure they have the ones required for the remort process. */
      token_info = rv2_get_token_info(ch);

      /* Did the character fail the token check? */
      if (!rv2_token_check(ch, token_info)) return TRUE;

      /* Log the remort and record it as a death to save additional stats. */
      GET_BEEN_KILLED(ch)++;
      if (GET_DEATH_LIMIT(ch)) GET_DEATH_LIMIT(ch)++;

      snprintf(buf, sizeof(buf),
        "WIZINFO: Remort committed for %s with %dhp %dmn %dmv, granting %lld remort XP (total: %lld) for %d QP and %d SCP.",
        GET_NAME(ch),
        GET_NAT_HIT(ch),
        GET_NAT_MANA(ch),
        GET_NAT_MOVE(ch),
        remort_info.exp,
        remort_info.exp + GET_REMORT_EXP(ch),
        remort_info.qp_fee,
        remort_info.scp_fee
      );

      wizlog(buf, LEVEL_SUP, 5);
      log_f("%s", buf);

      death_list(ch);

      /* Junk the tokens, as we're good to go. */
      if (token_info.alignment_token) {
        signal_object(token_info.alignment_token, ch, MSG_OBJ_JUNKED, "");
      }

      if (token_info.gender_token) {
        signal_object(token_info.gender_token, ch, MSG_OBJ_JUNKED, "");
      }

      if (token_info.class_token) {
        signal_object(token_info.class_token, ch, MSG_OBJ_JUNKED, "");
      }

      /* Get rid of any remort enchant the character currently has. */
      ench_from_char(ch, "Remort", 0, FALSE);

      send_to_char("Immortalis makes a simple gesture, and your equipped items vanish, only to re-materialize in your hands!\n\r", ch);

      /* Unequip the character to make sure worn items don't cause problems during the remort process. */
      for (int eq_pos = 0; eq_pos < MAX_WEAR; eq_pos++) {
        if (EQ(ch, eq_pos)) {
          obj_to_char(unequip_char(ch, eq_pos), ch);
        }
      }

      for (ENCH *ench = ch->enchantments, *next_ench = NULL; ench; ench = next_ench) {
        next_ench = ench->next;

        switch (ench->type) {
          case ENCHANT_SQUIRE:
          case ENCHANT_FIRSTSWORD:
          case ENCHANT_APPRENTICE:
          case ENCHANT_PRIVATE:
          case ENCHANT_WANDERER:
          case ENCHANT_TSUME:
          case ENCHANT_MINSTREL:
          case ENCHANT_MINION:
          case ENCHANT_ACOLYTE:
          case ENCHANT_HIGHWAYMAN:
          case ENCHANT_SWASHBUCKLER:
          case ENCHANT_JUSTICIAR:
          case ENCHANT_WARLOCK:
          case ENCHANT_COMMODORE:
          case ENCHANT_FORESTER:
          case ENCHANT_SHINOBI:
          case ENCHANT_POET:
          case ENCHANT_DARKWARDER:
          case ENCHANT_BISHOP:
          case ENCHANT_BRIGAND:
          case ENCHANT_KNIGHT:
          case ENCHANT_LORDLADY:
          case ENCHANT_SORCERER:
          case ENCHANT_COMMANDER:
          case ENCHANT_TAMER:
          case ENCHANT_SHOGUN:
          case ENCHANT_CONDUCTOR:
          case ENCHANT_DARKLORDLADY:
          case ENCHANT_PROPHET:
          case ENCHANT_ASSASSIN:
            ench_remove(ch, ench, FALSE);
            break;
        }
      }

      for (AFF *aff = ch->affected, *next_aff = NULL; aff; aff = next_aff) {
        next_aff = aff->next;

        aff_remove(ch, aff);
      }

      GET_TOGGLES(ch) = 0;

      GET_ALIGNMENT(ch) = token_info.alignment;
      GET_SEX(ch) = token_info.gender;
      GET_CLASS(ch) = token_info.class_num;

      GET_SC(ch) = 0;
      GET_SC_LEVEL(ch) = 0;

      GET_BIRTH(ch) -= ((long)SECS_PER_MUD_YEAR * (long)(17 - GET_AGE(ch)));

      GET_LEVEL(ch) = 1;
      GET_EXP(ch) = 0;

      if (GET_DEATH_EXP(ch)) {
        GET_DEATH_EXP(ch) = 0;

        ENCH *imm_grace_ench = ench_get_from_char(ch, NULL, ENCHANT_IMM_GRACE);

        if (imm_grace_ench) {
          ench_remove(ch, imm_grace_ench, FALSE);
        }
      }

      if (!IS_SET(GET_PFLAG(ch), PLR_SKIPTITLE)) {
        set_title(ch, NULL);
      }

      GET_OSTR(ch) = 13;
      GET_ODEX(ch) = 13;
      GET_OCON(ch) = 13;
      GET_OINT(ch) = 13;
      GET_OWIS(ch) = 13;
      GET_OADD(ch) = 0;

      GET_MAX_HIT_POINTS(ch) = 10;
      GET_MAX_MANA_POINTS(ch) = 0;
      GET_MAX_MOVE_POINTS(ch) = 0;

      affect_total(ch);

      if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
        GET_MAX_HIT_POINTS(ch) += GET_PRESTIGE(ch) * 12;
      }
      else {
        GET_MAX_HIT_POINTS(ch) += GET_PRESTIGE(ch) * 8;
      }

      if (!((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD))) {
        GET_MAX_MANA_POINTS(ch) += GET_PRESTIGE(ch) * 4;
      }

      GET_PRAC(ch) = 0;

      advance_level(ch);

      GET_AFF(ch) = 0;
      GET_AFF2(ch) = 0;

      for (int skill = 0; skill < MAX_SKILLS5; skill++) {
        GET_LEARNED(ch, skill) = 0;
      }

      for (int save = 0; save < MAX_SAVE; save++) {
        GET_SAVING_THROW(ch, save) = 0;
      }

      GET_HIT(ch) = hit_limit(ch);
      GET_MANA(ch) = mana_limit(ch);
      GET_MOVE(ch) = move_limit(ch);

      switch (GET_CLASS(ch)) {
        case CLASS_THIEF:
          GET_LEARNED(ch, SKILL_SNEAK) = 10;
          GET_LEARNED(ch, SKILL_HIDE) = 5;
          GET_LEARNED(ch, SKILL_STEAL) = 15;
          GET_LEARNED(ch, SKILL_BACKSTAB) = 10;
          GET_LEARNED(ch, SKILL_PICK_LOCK) = 10;
          break;

        case CLASS_PALADIN:
          if (token_info.alignment == 0) {
            GET_ALIGNMENT(ch) = 500;
          }
          break;

        case CLASS_ANTI_PALADIN:
          if (token_info.alignment == 0) {
            GET_ALIGNMENT(ch) = -500;
          }
          break;
      }

      GET_COND(ch, THIRST) = 24;
      GET_COND(ch, FULL) = 24;
      GET_COND(ch, DRUNK) = 0;

      GET_QP(ch) = MAX(0, remort_info.qp - remort_info.qp_fee);
      GET_SCP(ch) = MAX(0, remort_info.scp - remort_info.scp_fee);

      GET_REMORT_EXP(ch) += remort_info.exp;

      if (GET_REMORT_EXP(ch) <= 0) {
        GET_REMORT_EXP(ch) = 0;

        snprintf(buf, sizeof(buf), "WIZINFO: Remort EXP Overflow! [%s]", GET_NAME(ch));

        wizlog(buf, LEVEL_SUP, 5);
        log_f("%s", buf);
      }
      else {
        rv2_add_enchant(ch);
      }

      affect_total(ch);

      save_char(ch, NOWHERE);

      send_to_char("You feel an overwhelming sensation that can only be described as a troubling mix of death and new life.\n\r", ch);
      send_to_char("The entire force of your being is ripped seamlessly from within as a feeling of transcendence courses through your entire body.\n\r", ch);
      send_to_char("Immortalis' visage breaks into jubilance for but a brief moment as he smiles.\n\r", ch);
      send_to_char("He whispers, 'Your will is done. I wish you a pleasant journey and eagerly hope your new existence is a most fruitful endeavor.'\n\r", ch);

      return TRUE;
    }

    if (!str_cmp(arg1, "remort")) {
      send_to_char("Immortalis raises his eyebrow at you questioningly.\n\r", ch);
      send_to_char("He whispers, 'I think I misheard your request. I can help you with the following `jremort`q commands: appraise, cancel, enable, disable, commit.\n\r", ch);

      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

/* Object Specs */

/* Decrement the counter on tokens every tick and prevent the tokens from getting "lost" from a character's inventory. */
int rv2_obj_spec_token(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_TICK) {
    if (OBJ_TIMER(obj)) {
      OBJ_TIMER(obj)--;
    }

    if (OBJ_TIMER(obj) <= 0) {
      rv2_extract_token(obj);
    }

    return FALSE;
  }

  if (cmd == MSG_OBJ_GIVEN || cmd == MSG_OBJ_DROPPED || cmd == MSG_OBJ_PUT || cmd == MSG_OBJ_DONATED || cmd == MSG_OBJ_JUNKED) {
    rv2_extract_token(obj);

    return TRUE;
  }

  return FALSE;
}

/* Assign Specs */
void assign_remortv2(void) {
  assign_mob(RV2_MOB_IMMORTALIS, rv2_mob_spec_immortalis);

  for (int vnum = RV2_TOKEN_MIN_VNUM; vnum <= RV2_TOKEN_MAX_VNUM; vnum++) {
    assign_obj(vnum, rv2_obj_spec_token);
  }
}
