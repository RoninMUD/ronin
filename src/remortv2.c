/*
  remortv2.c - Specs for Remort v2, by Night

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 4/24/2012
*/

/* Includes */
/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ronin Includes */
#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "act.h"
#include "cmd.h"
#include "comm.h"
#include "db.h"
#include "enchant.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "meta.h"
#include "spec_assign.h"
#include "spells.h"
#include "time.h"
#include "remortv2.h"

/* Externs */
extern void advance_level(CHAR *ch);
extern void death_list(CHAR *ch);
extern void imm_grace_remove_enchant(CHAR *ch);

/* Function used to simulate a character advancing levels and gaining the average points every time. */
void rv2_sim_advance_level(CHAR *ch)
{
  int gain = 0;

  switch (GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
      gain = 5;   /* average 5, was 4 */
    break;

    case CLASS_CLERIC:
      gain = 7;   /* average 7, was 6 */
    break;

    case CLASS_THIEF:
      gain = 9;  /* average 9, was 8 */
    break;

    case CLASS_WARRIOR:
      gain = 12; /* average 12, was 11 */
    break;

    case CLASS_NINJA:
      gain = 7;   /* average 7, was 6 */
    break;

    case CLASS_NOMAD:
      gain = 15; /* average 15, was 14 */
    break;

    case CLASS_PALADIN:
      gain = 10;  /* average 10, was 9 */
    break;

    case CLASS_ANTI_PALADIN:
      gain = 8;  /* average 8, was 7 */
    break;

    case CLASS_AVATAR:
      gain = 15; /* average 15 */
    break;

    case CLASS_BARD:
      gain = 8;  /* average 8, was 6.5 */
    break;

    case CLASS_COMMANDO:
      gain = 9;  /* average 9, was 8 */
    break;

    default:
      gain = 0;
    break;
  }

  adjust_hit_mana_move(ch, META_HIT, gain);

  gain = 0;

  switch (GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
    case CLASS_AVATAR:
    case CLASS_BARD:
      gain = 4; /* average 4, was 3 */
    break;

    case CLASS_NINJA:
    case CLASS_PALADIN:
    case CLASS_ANTI_PALADIN:
    case CLASS_COMMANDO:
      gain = 3; /* average 3, was 2 */
    break;

    default:
      gain = 0;
    break;
  }

  adjust_hit_mana_move(ch, META_MANA, gain);

  adjust_hit_mana_move(ch, META_MOVE, 3);
}

/* Function used to maniplulate remort experience. Don't use this unless you know what you're doing. */
int rv2_adjust_remort_exp(CHAR *ch, int exp)
{
  char buf[MSL];

  GET_REMORT_EXP(ch) += exp;

  /* Did the player finish the remort process? */
  if (!GET_REMORT_EXP(ch))
  {
    GET_REMORT_EXP(ch) = 0;

    send_to_char("Congratulations! You have completed the remort process!\n\r", ch);

    sprintf(buf, "%s has completed the remort process! All hail %s!\n\r", PERS(ch, ch), PERS(ch, ch));
    send_to_world_except(buf, ch);

    rv2_remove_enchant(ch);
  }

  return exp;
}

int rv2_calc_remort_mult(CHAR *ch) {
  int result = RV2_EXP_MULTIPLIER;
  int min, step, adjust;

  if (!ch) return result;

  /* attribute based increases */

  if (GET_OSTR(ch) >= 18) {
    result += 1;

    if (GET_OADD(ch) >= 50) result += 1;
    if (GET_OADD(ch) == 100) result += 2;
  }

  if (GET_ODEX(ch) >= 18) result += 1;
  if (GET_OINT(ch) >= 18) result += 1;
  if (GET_OWIS(ch) >= 18) result += 1;
  if (GET_OCON(ch) >= 18) result += 1;

  /* level based increases */

  if (GET_LEVEL(ch) >= 30) result += 1;
  if (GET_LEVEL(ch) >= 40) result += 1;
  if (GET_LEVEL(ch) >= 45) result += 2;
  if (GET_LEVEL(ch) >= 50) result += 3;

  /* stat based increase */

  if (GET_CLASS(ch) == CLASS_NOMAD || GET_CLASS(ch) == CLASS_WARRIOR || GET_CLASS(ch) == CLASS_THIEF) {
    min  = 1000;
    step = 250;
  } else {
    min  = 500;
    step = 250;
  }

  int prestige_hit = 0;

  if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
    prestige_hit = GET_PRESTIGE(ch) * (PRESTIGE_HIT_GAIN + PRESTIGE_MANA_GAIN);
  }
  else {
    prestige_hit = GET_PRESTIGE(ch) * PRESTIGE_HIT_GAIN;
  }

  adjust = MAX(0, GET_NAT_HIT(ch) - min - prestige_hit) / step;
  if (adjust > 0) result += (1 << (adjust - 1));

  int prestige_mana = 0;

  if (!((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD))) {
    prestige_mana = GET_PRESTIGE(ch) * PRESTIGE_MANA_GAIN;
  }

  adjust = MAX(0, GET_NAT_MANA(ch) - min - prestige_mana) / step;
  if (adjust > 0) result += (1 << (adjust - 1));

  result = MIN(result, 24);

  // Prestige Perk 1
  if (GET_PRESTIGE_PERK(ch) >= 1) result += 1;

  return result;
}

/* Give the player remort experience (if they deserve it), and return how much experience was given. */
int rv2_gain_remort_exp(CHAR *ch, int exp)
{
  if (!GET_REMORT_EXP(ch)) return 0;

  if (exp > RV2_MAX_EXP_GAIN) exp = RV2_MAX_EXP_GAIN;

  exp *= rv2_calc_remort_mult(ch);

  if (exp > GET_REMORT_EXP(ch)) exp = (int)GET_REMORT_EXP(ch);

  rv2_adjust_remort_exp(ch, -exp); /* Deduct the experience from the remort experience pool. */
  gain_exp(ch, exp); /* Give the remort experience to the player. */

  return exp;
}

/* This function will return the remort experience pool of a player, based on multiple factors (mostly metas). */
long long int rv2_meta_sim(CHAR *ch)
{
#ifdef TEST_SITE
  char buf[MSL];
#endif
  CHAR *sim = NULL;
  int i = 0;
  int j = 0;
  int stat_total = 0;
  long long int exp = 0;

  if (GET_LEVEL(ch) < 6) return 0;

  /* Create a sim character. */
  CREATE(sim, CHAR, 1);
  clear_char(sim);
  reset_char(sim);

  /* Set the sim character to level 0 (it'll get leveled up below) and set it to the same class as the player. */
  GET_LEVEL(sim) = 0;
  GET_CLASS(sim) = GET_CLASS(ch);

  /* Set the sim character's points to the base for a level 1 (it'll get leveled up below). */
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

  /* Calculate experience required for the player's level, and the average point gains from such levels. */
  for (i = 0; GET_LEVEL(sim) < LEVEL_MORT && GET_LEVEL(sim) < GET_LEVEL(ch); i++)
  {
    exp += rv2_adjust_remort_exp(sim, exp_table[GET_LEVEL(sim) + 1]);

    rv2_sim_advance_level(sim);
    GET_LEVEL(sim)++;

    affect_total(sim);
  }

  /* To prevent a "free exp" loophole, assume 18 Constitution stat (leveling gear) for HP purposes. */
  adjust_hit_mana_move(sim, META_HIT, (GET_LEVEL(sim) * con_app[18].hitp));

  affect_total(sim);

#ifdef TEST_SITE

  sprintf(buf, "Level EXP to Pool........ %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  sprintf(buf, "Base Hit:  %5d, Target Hit:  %5d\n\r",
    get_max_stat(sim, META_HIT), get_max_stat(ch, META_HIT));
  send_to_char(buf, ch);

#endif

  /* Calculate the average number of HP metas required to match the player's HP, based on a 4.5 point average. */
  i = 0;
  exp = 0;
  if (get_max_stat(sim, META_HIT) < get_max_stat(ch, META_HIT))
  {
    for (i = 0; get_max_stat(sim, META_HIT) < get_max_stat(ch, META_HIT); i++)
    {
      exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_HIT));

      if (i % 2)
      {
        adjust_hit_mana_move(sim, META_HIT, 4);
      }
      else
      {
        adjust_hit_mana_move(sim, META_HIT, 5);
      }

      affect_total(sim);
    }
  }

#ifdef TEST_SITE

  sprintf(buf, "Avg. No. of Hit Metas.... %d\n\r", i);
  send_to_char(buf, ch);

  sprintf(buf, "Hit Meta EXP to Pool..... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  sprintf(buf, "Base Mana: %5d, Target Mana: %5d\n\r",
    get_max_stat(sim, META_MANA), get_max_stat(ch, META_MANA));
  send_to_char(buf, ch);

#endif

  /* Calculate the average number of mana metas required to match the player's mana, based on a 4.5 point average. */
  i = 0;
  exp = 0;
  if (get_max_stat(sim, META_MANA) < get_max_stat(ch, META_MANA))
  {
    for (i = 0; get_max_stat(sim, META_MANA) < get_max_stat(ch, META_MANA); i++)
    {
      exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_MANA));

      if (i % 2)
      {
        adjust_hit_mana_move(sim, META_MANA, 4);
      }
      else
      {
        adjust_hit_mana_move(sim, META_MANA, 5);
      }

      affect_total(sim);
    }
  }

#ifdef TEST_SITE

  sprintf(buf, "Avg. No. of Mana Metas... %d\n\r", i);
  send_to_char(buf, ch);

  sprintf(buf, "Mana Meta EXP to Pool.... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  sprintf(buf, "Base Move: %5d, Target Move: %5d\n\r",
    get_max_stat(sim, META_MOVE), get_max_stat(ch, META_MOVE));
  send_to_char(buf, ch);

#endif

  /* Calculate the average number of move metas required to match the player's movement points. */
  i = 0;
  exp = 0;
  if (get_max_stat(sim, META_MOVE) < get_max_stat(ch, META_MOVE))
  {
    for (i = 0; get_max_stat(sim, META_MOVE) < get_max_stat(ch, META_MOVE); i++)
    {
      if (((get_max_stat(sim, META_MOVE)) + 30) <= get_max_stat(ch, META_MOVE))
      {
        exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_MOVE));

        adjust_hit_mana_move(sim, META_MOVE, 30);
      }
      else
      {
        exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_MOVE_UP_1));

        adjust_hit_mana_move(sim, META_MOVE_UP_1, 1);
      }

      affect_total(sim);
    }
  }

#ifdef TEST_SITE

  sprintf(buf, "Move Meta EXP to Pool.... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  /* Calculate the player's stat total. */
  i = 0;
  exp = 0;
  for (i = META_STR; i <= META_CON; i++)
  {
    stat_total += get_ability(ch, i);
  }

  /* Calculate the number of metas required to match the player's stats if their stat total is above 65. */
  i = 0;
  exp = 0;
  if (stat_total > (RV2_SIM_AVG_STAT * 5) || get_ability(ch, META_STR_ADD))
  {
    for (i = META_STR; i <= META_CON; i++)
    {
      if (get_ability(sim, i) < RV2_SIM_AVG_STAT)
      {
        for (j = 0; get_ability(sim, i) < RV2_SIM_AVG_STAT; j++)
        {
          exp += rv2_adjust_remort_exp(sim, -meta_cost(sim, i));

          adjust_ability(sim, i, 1);

          affect_total(sim);
        }
      }
      else
      {
        for (j = 0; get_ability(sim, i) < get_ability(ch, i); j++)
        {
          exp += rv2_adjust_remort_exp(sim, meta_cost(sim, i));

          adjust_ability(sim, i, 1);

          affect_total(sim);
        }
      }
    }

    /* Calculate the amount of experience invested in STR_ADD metas. */
    if (get_ability(ch, META_STR_ADD))
    {
      for (i = 0; get_ability(sim, META_STR_ADD) < get_ability(ch, META_STR_ADD); i++)
      {
        exp += rv2_adjust_remort_exp(sim, meta_cost(sim, META_STR_ADD));

        adjust_ability(sim, META_STR_ADD, 1);

        affect_total(sim);
      }
    }
  }

#ifdef TEST_SITE

  sprintf(buf, "Stat Meta EXP to Pool.... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  /* Calculate the amount of experience invested in ranks. */
  i = 0;
  exp = 0;
  if (get_rank(ch))
  {
    for (i = 0; i < get_rank(ch); i++)
    {
      exp += rv2_adjust_remort_exp(sim, 5000000 * (i + 1));
    }
  }

#ifdef TEST_SITE

  sprintf(buf, "Rank EXP to Pool......... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  /* Add the player's current experience pool to the remort experience pool. */
  exp = 0;
  if (GET_EXP(ch))
  {
    exp = rv2_adjust_remort_exp(sim, GET_EXP(ch));
  }

#ifdef TEST_SITE

  sprintf(buf, "Normal EXP to Pool....... %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  /* Add the player's current death experience pool to the remort experience pool. */
  exp = 0;
  if (GET_DEATH_EXP(ch))
  {
    exp = rv2_adjust_remort_exp(sim, GET_DEATH_EXP(ch));
  }

#ifdef TEST_SITE

  sprintf(buf, "Death EXP to Pool........ %lld\n\r", exp);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

#ifdef TEST_SITE

  sprintf(buf, "Remort v2 EXP Pool....... %lld\n\r", GET_REMORT_EXP(sim));
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  sprintf(buf, "Avg. Stat Used........... %d\n\r", RV2_SIM_AVG_STAT);
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

  send_to_char("Simulated Final Stats\n\r", ch);
  send_to_char("---------------------\n\r", ch);
  sprintf(buf, "Level... %d\n\r", GET_LEVEL(sim));
  send_to_char(buf, ch);
  sprintf(buf, "Class... %s\n\r", GET_CLASS_NAME(ch));
  send_to_char(buf, ch);
  sprintf(buf, "Rank.... %d\n\r", get_rank(ch));
  send_to_char(buf, ch);
  sprintf(buf, "Hit..... %d\n\r", GET_MAX_HIT(sim));
  send_to_char(buf, ch);
  sprintf(buf, "Mana.... %d\n\r", GET_MAX_MANA(sim));
  send_to_char(buf, ch);
  sprintf(buf, "Move.... %d\n\r", GET_MAX_MOVE(sim));
  send_to_char(buf, ch);
  sprintf(buf, "Str..... %d\n\r", GET_OSTR(sim));
  send_to_char(buf, ch);
  sprintf(buf, "Int..... %d\n\r", GET_OINT(sim));
  send_to_char(buf, ch);
  sprintf(buf, "Wis..... %d\n\r", GET_OWIS(sim));
  send_to_char(buf, ch);
  sprintf(buf, "Dex..... %d\n\r", GET_ODEX(sim));
  send_to_char(buf, ch);
  sprintf(buf, "Con..... %d\n\r", GET_OCON(sim));
  send_to_char(buf, ch);
  sprintf(buf, "Add..... %d\n\r", GET_OADD(sim));
  send_to_char(buf, ch);

  send_to_char("\n\r", ch);

#endif

  exp = GET_REMORT_EXP(sim);

  free_char(sim);

  return exp;
}

/* Misc Functions */

/* Just a utility function to make things a little cleaner down below. */
int rv2_get_ability_by_apply(CHAR *ch, int ability_apply)
{
  int value = 0;

  switch (ability_apply)
  {
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

/* Used to extract remort tokens. */
void rv2_extract_token(OBJ *obj)
{
  char buf[MSL];

  sprintf(buf, "%s vanishes from existence.\n\r", OBJ_SHORT(obj));

  if (obj->carried_by)
  {
    send_to_char(buf, obj->carried_by);

    obj_from_char(obj);
    extract_obj(obj);
  }
  else if (obj->in_room)
  {
    send_to_room(buf, obj->in_room);

    obj_from_room(obj);
    extract_obj(obj);
  }
}

/* Determines what remort tokens (if any) a player has, and what their choices are based on those tokens. */
struct rv2_token_info rv2_get_token_info(CHAR *ch)
{
  OBJ *tmp_obj = NULL;
  struct rv2_token_info token_info;

  /* token_info.name_token = NULL; */
  token_info.gender_token = NULL;
  token_info.class_token = NULL;
  token_info.class_num = -1;
  token_info.gender = 0;
  strncpy(token_info.gender_name, "", sizeof(token_info.gender_name));
  strncpy(token_info.class_name, "", sizeof(token_info.class_name));

  for (tmp_obj = ch->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
  {
    if (V_OBJ(tmp_obj) < RV2_TOKEN_MIN_VNUM || V_OBJ(tmp_obj) > RV2_TOKEN_MAX_VNUM) continue;
    if (OBJ_VALUE0(tmp_obj) != GET_ID(ch)) continue;
    /* if (name_token && V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_NAME) continue; */
    if (token_info.gender_token && (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_MALE || V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_FEMALE)) continue;
    if (token_info.class_token && (V_OBJ(tmp_obj) >= RV2_OBJ_TOKEN_AVATAR && V_OBJ(tmp_obj) <= RV2_OBJ_TOKEN_COMMANDO)) continue;

    /*
    if (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_NAME && OBJ_VALUE1(tmp_obj) == TRUE)
    {
      name_token = tmp_obj;
    }
    else
    */
    if (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_MALE || V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_FEMALE)
    {
      token_info.gender_token = tmp_obj;
    }
    else if (V_OBJ(tmp_obj) >= RV2_OBJ_TOKEN_AVATAR && V_OBJ(tmp_obj) <= RV2_OBJ_TOKEN_COMMANDO)
    {
      token_info.class_token = tmp_obj;
    }
  }

  if (token_info.gender_token)
  {
    if (V_OBJ(token_info.gender_token) == RV2_OBJ_TOKEN_MALE)
    {
      token_info.gender = SEX_MALE;
      strncpy(token_info.gender_name, "male", sizeof(token_info.gender_name));
    }
    else if (V_OBJ(token_info.gender_token) == RV2_OBJ_TOKEN_FEMALE)
    {
      token_info.gender = SEX_FEMALE;
      strncpy(token_info.gender_name, "female", sizeof(token_info.gender_name));
    }
  }

  if (token_info.class_token)
  {
    if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_AVATAR)
    {
      token_info.class_num = CLASS_AVATAR;
      strncpy(token_info.class_name, "avatar", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_MAGE)
    {
      token_info.class_num = CLASS_MAGIC_USER;
      strncpy(token_info.class_name, "mage", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_CLERIC)
    {
      token_info.class_num = CLASS_CLERIC;
      strncpy(token_info.class_name, "cleric", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_THIEF)
    {
      token_info.class_num = CLASS_THIEF;
      strncpy(token_info.class_name, "thief", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_WARRIOR)
    {
      token_info.class_num = CLASS_WARRIOR;
      strncpy(token_info.class_name, "warrior", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_NINJA)
    {
      token_info.class_num = CLASS_NINJA;
      strncpy(token_info.class_name, "ninja", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_NOMAD)
    {
      token_info.class_num = CLASS_NOMAD;
      strncpy(token_info.class_name, "nomad", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_PALADIN)
    {
      token_info.class_num = CLASS_PALADIN;
      strncpy(token_info.class_name, "paladin", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_ANTI_PALADIN)
    {
      token_info.class_num = CLASS_ANTI_PALADIN;
      strncpy(token_info.class_name, "anti-paladin", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_BARD)
    {
      token_info.class_num = CLASS_BARD;
      strncpy(token_info.class_name, "bard", sizeof(token_info.class_name));
    }
    else if (V_OBJ(token_info.class_token) == RV2_OBJ_TOKEN_COMMANDO)
    {
      token_info.class_num = CLASS_COMMANDO;
      strncpy(token_info.class_name, "commando", sizeof(token_info.class_name));
    }
  }

  return token_info;
}

/* Determines if a character has the right tokens to remort. If not, prints some messages. */
int rv2_token_check(CHAR *ch, struct rv2_token_info token_info)
{
  char buf[MSL];

  if (!token_info.gender_token || !token_info.class_token)
  {
    if (!token_info.gender_token && !token_info.class_token)
    {
      send_to_char("`qA pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation without first making the necessary preparations. If you do wish to proceed, I humbly request you at least purchase gender and class tokens of your own choosing.'\n\r", ch);
    }
    else if (token_info.gender_token && !token_info.class_token)
    {
      sprintf(buf, "`qImmortalis gives you a pensive glance as he calmly places a token into the palm of your hand and whispers 'A `k%s`q it shall be, but what of the `nclass`q?'\n\r",
        token_info.gender_name);
      send_to_char(buf, ch);
    }
    else if (!token_info.gender_token && token_info.class_token)
    {
      sprintf(buf, "`qImmortalis gives you a wistful smile as he gently places a token into the palm of your hand and whispers 'A `n%s`q it shall be, but what of the `kgender`q?'\n\r",
        token_info.class_name);
      send_to_char(buf, ch);
    }

    return FALSE;
  }

  return TRUE;
}

/* Used to calculate the character's remort experience pool, their QP and SCP, and any fees they must pay in order to remort. */
struct rv2_remort_info rv2_appraise(CHAR *ch)
{
  int i = 0;
  int j = 0;
  long long int tmp = 0;
  long long int chunk = 0;
  struct rv2_remort_info remort_info;

  remort_info.exp = 0;
  remort_info.qp = 0;
  remort_info.scp = 0;
  remort_info.qp_fee = 0;
  remort_info.scp_fee = 0;

  remort_info.exp = rv2_meta_sim(ch);

  /* Calculate the amount of QP invested in the player's stats. */
  for (i = APPLY_STR; i <= APPLY_CON; i++)
  {
    if (rv2_get_ability_by_apply(ch, i) < 19) continue;

    for (j = 19; j <= rv2_get_ability_by_apply(ch, i); j++)
    {
      remort_info.qp += (j - 18) * 200;
    }
  }

  remort_info.qp += GET_QP(ch); /* Add in their current QP pool. */

  /* Calculate the amount of SCP invested in the player's subclass levels. */
  for (i = 1; i <= GET_SC_LEVEL(ch); i++)
  {
    remort_info.scp += i * 70;
  }

  remort_info.scp += GET_SCP(ch); /* Add in their current SCP pool. */

  /* Determine the fee to be charged for the remort process (if any). */
  tmp = RV2_MIN_EXP_COST;
  if (remort_info.exp > tmp)
  {
    tmp = RV2_MAX_EXP_COST;
    if (remort_info.exp > tmp)
    {
      remort_info.qp_fee = RV2_MAX_QP_COST;
      remort_info.scp_fee = RV2_MAX_SCP_COST;
    }
    else
    {
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

  return remort_info;
}

/* Print some messages based on the results of appraise (above). */
void rv2_appraise_message(CHAR *ch, struct rv2_remort_info remort_info)
{
  char buf[MSL];
  long long int tmp = 0;

  tmp = RV2_MIN_EXP_COST;
  if (remort_info.exp > tmp)
  {
    if (remort_info.qp_fee > remort_info.qp || remort_info.scp_fee > remort_info.scp)
    {
      sprintf(buf, "Immortalis whispers 'Commensurate to your achievements, so too shall be the measure of my fee. To transcend this existence, I will require compensation in the form of %d quest points and %d subclass points, which I see you do not possess. Were you to acquire it however, I would return to you a sum of %lld experience points.'\n\r",
        remort_info.qp_fee, remort_info.scp_fee, remort_info.exp);
      send_to_char(buf, ch);
    }
    else
    {
      sprintf(buf, "Immortalis whispers 'Commensurate to your achievements, so too shall be the measure of my fee. To transcend this existence, I will require compensation in the form of %d quest points and %d subclass points. For my part, I will return to you a sum of %lld experience points, %d quest points and %d subclass points, for you to spend at your leisure.'\n\r",
        remort_info.qp_fee, remort_info.scp_fee, remort_info.exp, MAX(0, remort_info.qp - remort_info.qp_fee), MAX(0, remort_info.scp - remort_info.scp_fee));
      send_to_char(buf, ch);
    }
  }
  else
  {
    sprintf(buf, "Immortalis whispers 'Commensurate to your achievements, so too shall be the measure of my fee. To transcend this existence, I will require no level of compensation, as your achievements do not yet merit excessive effort on my behalf. For my part however, I will return to you a sum of %lld experience points, %d quest points and %d subclass points.'\n\r",
      remort_info.exp, remort_info.qp, remort_info.scp);
    send_to_char(buf, ch);
  }
}

/* Remove the remort v2 enchant. */
void rv2_remove_enchant(CHAR *ch)
{
  ENCH *ench = NULL;

  for (ench = ch->enchantments; ench; ench = ench->next)
  {
    if (ench->type == ENCHANT_REMORTV2)
    {
      enchantment_remove(ch, ench, FALSE);
    }
  }
}

/* Add the remort v2 enchant. */
void rv2_add_enchant(CHAR *ch)
{
  char buf[MSL];
  ENCH ench;

  sprintf(buf, "Remort");
  ench.name = buf;
  enchantment_to_char(ch, &ench, TRUE);
}

/* Mobile Specs */

/* Responsible for the core of the remort system. */
int rv2_mob_spec_immortalis(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  char buf[MSL];
  OBJ *token = NULL;
  OBJ *tmp_obj = NULL;
  OBJ *next_obj = NULL;
  int vnum = 0;
  int i = 0;
  long age_year = 17;
  struct rv2_remort_info remort_info;
  struct rv2_token_info token_info;
  ENCH *ench = NULL;
  ENCH *ench_next = NULL;
  AFF *aff = NULL;
  AFF *aff_next = NULL;

  /* Buying tokens is handled as a spec due to shopkeeper limitations and a few other requirements. */
  if (cmd == CMD_BUY)
  {
    if (!ch || IS_NPC(ch)) return FALSE;

    if (CHAR_VIRTUAL_ROOM(mob) != RV2_WLD_SANCTUM_ETERNITY) return FALSE;

    if (GET_LEVEL(ch) < 6)
    {
      send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste. If you do indeed endeavor to transcend this existence and into the next, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

      return TRUE;
    }
    else if (IS_IMMORTAL(ch))
    {
      send_to_char("Consult with a SUP+ about remorting as an immortal.\n\r", ch);

      return TRUE;
    }

    one_argument(arg, buf);

    if (!str_cmp(buf, "name")) vnum = RV2_OBJ_TOKEN_NAME;
    else if (!str_cmp(buf, "male")) vnum = RV2_OBJ_TOKEN_MALE;
    else if (!str_cmp(buf, "female")) vnum = RV2_OBJ_TOKEN_FEMALE;
    else if (!str_cmp(buf, "avatar")) vnum = RV2_OBJ_TOKEN_AVATAR;
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
    else
    {
      send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot provide that which you request.'\n\r", ch);

      return TRUE;
    }

    /* We aren't allowing renames or avatars at this time, but it could be added in the future with little work. */
    if (vnum == RV2_OBJ_TOKEN_NAME || vnum == RV2_OBJ_TOKEN_AVATAR || vnum == RV2_OBJ_TOKEN_NOMAD)
    {
      send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot provide that which you request.'\n\r", ch);

      return TRUE;
    }

    /* Ensure that a player can't buy more than one of each type of token. */
    for (tmp_obj = ch->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
    {
      if (V_OBJ(tmp_obj) < RV2_TOKEN_MIN_VNUM || V_OBJ(tmp_obj) > RV2_TOKEN_MAX_VNUM) continue;

      /*
      if (vnum == RV2_OBJ_TOKEN_NAME &&
          V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_NAME)
      {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you more than a single token of that variety\n\r", ch);

        return TRUE;
      }
      else
      */
      if ((vnum == RV2_OBJ_TOKEN_MALE || vnum == RV2_OBJ_TOKEN_FEMALE) &&
          (V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_MALE || V_OBJ(tmp_obj) == RV2_OBJ_TOKEN_FEMALE))
      {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you more than a single token of that variety.\n\r", ch);

        return TRUE;
      }
      else
      if ((vnum >= RV2_OBJ_TOKEN_AVATAR && vnum <= RV2_OBJ_TOKEN_COMMANDO) &&
          (V_OBJ(tmp_obj) >= RV2_OBJ_TOKEN_AVATAR && V_OBJ(tmp_obj) <= RV2_OBJ_TOKEN_COMMANDO))
      {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you more than a single token of that variety.\n\r", ch);

        return TRUE;
      }
    }

    /* Load up a token to give to the player. */
    if (!(token = read_object(vnum, VIRTUAL)))
    {
      send_to_char("There was a problem loading the token. Please report this.\n\r", ch);
      sprintf(buf, "WIZINFO: Error loading object (#%d) in rv2_mob_spec_immortalis(), cmd == CMD_LIST", vnum);
      wizlog(buf, LEVEL_SUP, 5);
      log_f("%s", buf);

      return TRUE;
    }

    /* Set the token's Value 0 to the unique ID of the player (just a safety precaution), and set its timer to 10 ticks. */
    OBJ_VALUE0(token) = GET_ID(ch);
    OBJ_TIMER(token) = 10;

    obj_to_char(token, ch);

    /* Find out what tokens the player has now. */
    token_info = rv2_get_token_info(ch);

    /* If the player has the proper tokens for a remort, print out the next steps. */
    if (rv2_token_check(ch, token_info))
    {
      sprintf(buf, "Immortalis places yet another token into your hand and a faint clink is briefly heard as he clasps his hands over yours, sealing the glimmering metal inside. He whispers 'A `k%s`q `n%s`q is it? Consider wisely, for this path leads in only a singular direction.'\n\r",
        token_info.gender_name, token_info.class_name);
      send_to_char(buf, ch);
      send_to_char("\n\r", ch);

      /* Calculate the player's remort info and print out some messages if needed. */
      remort_info = rv2_appraise(ch);
      rv2_appraise_message(ch, remort_info);

      /* The player can remort, so we'll tell them such. */
      if (remort_info.qp_fee <= remort_info.qp && remort_info.scp_fee <= remort_info.scp)
      {
        send_to_char("\n\r", ch);
        send_to_char("`qImmortalis whispers 'If you see merit in my offer, you need only issue the command `jremort commit`q and I shall see it done.'\n\r", ch);
        send_to_char("\n\r", ch);
        send_to_char("`qImmortalis whispers 'However, if you were mistaken in your choices and do not wish to undergo this transformation, you may simply walk away. Or, to make it more official, issue the command `icancel`q and I will proceed as though this conversation never happened.'\n\r", ch);
      }
    }

    return TRUE;
  }
  else if (cmd == CMD_LIST) /* Listing tokens is handled as a spec, mostly due to shopkeeper limitations. */
  {
    if (!ch || IS_NPC(ch)) return FALSE;

    if (CHAR_VIRTUAL_ROOM(mob) != RV2_WLD_SANCTUM_ETERNITY) return FALSE;

    if (GET_LEVEL(ch) < 6)
    {
      send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste. If you do indeed endeavor to transcend this existence and into the next, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

      return TRUE;
    }
    else if (IS_IMMORTAL(ch))
    {
      send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

      return TRUE;
    }

    send_to_char("You can buy:\n\r", ch);

    for (vnum = RV2_TOKEN_MIN_VNUM; vnum <= RV2_TOKEN_MAX_VNUM; vnum++)
    {
      /* Modify the following if statement if we ever allow renames or avatars through the remort process. */
      if (vnum == RV2_OBJ_TOKEN_NAME || vnum == RV2_OBJ_TOKEN_AVATAR || vnum == RV2_OBJ_TOKEN_NOMAD) continue;

      if (!(token = read_object(vnum, VIRTUAL)))
      {
        send_to_char("There was a problem loading the token. Please report this.\n\r", ch);
        sprintf(buf, "WIZINFO: Error loading object (#%d) in rv2_mob_spec_immortalis(), cmd == CMD_LIST", vnum);
        wizlog(buf, LEVEL_SUP, 5);
        log_f("%s", buf);

        return FALSE;
      }

      sprintf(buf, "%s\n\r", OBJ_SHORT(token));
      send_to_char(buf, ch);

      extract_obj(token);
    }

    if (!vnum)
    {
      send_to_char("Nothing.\n\r", ch);
    }

    return TRUE;
  }
  else if (cmd == CMD_UNKNOWN) /* Handle some "custom" commands. */
  {
    if (!ch || IS_NPC(ch)) return FALSE;

    if (CHAR_VIRTUAL_ROOM(mob) != RV2_WLD_SANCTUM_ETERNITY) return FALSE;

    /* The character can type "appraise" to see what their remort experience pool would be, and what it would cost them to remort. */
    if (!str_cmp(arg, "appraise"))
    {
      if (GET_LEVEL(ch) < 6)
      {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste. If you do indeed endeavor to proceed further, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

        return TRUE;
      }
      else if (IS_IMMORTAL(ch))
      {
        send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

        return TRUE;
      }

      /* Calculate the player's remort info and print out some messages if needed. */
      remort_info = rv2_appraise(ch);
      rv2_appraise_message(ch, remort_info);

      return TRUE;
    }
    else if (!str_cmp(arg, "remort commit")) /* The player is serious about remorting. Let's do it. */
    {
      if (GET_LEVEL(ch) < 6)
      {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste. If you do indeed endeavor to transcend this existence and into the next, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

        return TRUE;
      }
      else if (IS_IMMORTAL(ch))
      {
        send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

        return TRUE;
      }

      /* Calculate the player's remort experience pool and what it's going to cost them to remort. */
      remort_info = rv2_appraise(ch);

      if (remort_info.qp_fee > remort_info.qp || remort_info.scp_fee > remort_info.scp)
      {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation without the proper level of compensation for my efforts.'\n\r", ch);

        return TRUE;
      }

      /* Check the player's tokens and make sure they have the ones required for the remort process. */
      token_info = rv2_get_token_info(ch);

      /* Did the player fail the token check? */
      if (!rv2_token_check(ch, token_info)) return TRUE;

      GET_BEEN_KILLED(ch)++;
      if (GET_DEATH_LIMIT(ch)) GET_DEATH_LIMIT(ch)++;

      /* Log the remort. */
      sprintf(buf,
        "WIZINFO: Remort committed for %s with %dhp %dmn %dmv, granting %lld remort exp (Total: %lld) in (#%d).",  
        GET_NAME(ch),
        GET_NAT_HIT(ch),
        GET_NAT_MANA(ch),
        GET_NAT_MOVE(ch),
        remort_info.exp,
        remort_info.exp + GET_REMORT_EXP(ch),
        CHAR_VIRTUAL_ROOM(ch));
      wizlog(buf, LEVEL_SUP, 5);
      log_f("%s", buf);
      death_list(ch);

      /* Junk the tokens, as we're good to go. */
      /*
      if (token_info.name_token)
      {
        signal_object(token_info.name_token, ch, MSG_OBJ_JUNKED, "");
      }
      */

      if (token_info.gender_token)
      {
        signal_object(token_info.gender_token, ch, MSG_OBJ_JUNKED, "");
      }

      if (token_info.class_token)
      {
        signal_object(token_info.class_token, ch, MSG_OBJ_JUNKED, "");
      }

      /* Get rid of any remort enchant the player currently has. */
      rv2_remove_enchant(ch);

      send_to_char("\n\r", ch);
      send_to_char("Immortalis makes a simple gesture and your equipped items vanish, only to re-materialize in your hands!\n\r", ch);

      /* Unequip the character to make sure worn items don't cause problems during the remort process. */
      for (i = 0; i < MAX_WEAR; i++)
      {
        if (ch->equipment[i])
        {
          obj_to_char(unequip_char(ch, i), ch);
        }
      }

      /* Remove the player's rank enchantment, if they have one. */
      for (ench = ch->enchantments; ench; ench = ench_next)
      {
        ench_next = ench->next;

        switch (ench->type)
        {
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
            enchantment_remove(ch, ench, FALSE);
            break;
        }
      }

      /* Remove the player's affects. */
      for (aff = ch->affected; aff; aff = aff_next)
      {
        aff_next = aff->next;
        affect_remove(ch, aff);
      }

      /* Set the player up as their chosen gender and class, and reset their vitals to that of a new player. */
      GET_SEX(ch) = token_info.gender;
      GET_CLASS(ch) = token_info.class_num;
      GET_ALIGNMENT(ch) = 0;
      age_year = 17;
      age_year -= GET_AGE(ch);
      ch->player.time.birth -= ((long)SECS_PER_MUD_YEAR * (long)age_year);
      GET_SC(ch) = 0;
      GET_SC_LEVEL(ch) = 0;

      /* This section pulled (almost) directly from do_class() in act.wizard.c */

      GET_LEVEL(ch) = 1;
      GET_EXP(ch) = 1;

      if (GET_DEATH_EXP(ch))
      {
        GET_DEATH_EXP(ch) = 0;

        imm_grace_remove_enchant(ch);
      }

      if (!IS_SET(ch->specials.pflag, PLR_SKIPTITLE))
      {
        set_title(ch, NULL);
      }

      GET_OSTR(ch) = 13;
      GET_ODEX(ch) = 13;
      GET_OCON(ch) = 13;
      GET_OINT(ch) = 13;
      GET_OWIS(ch) = 13;
      GET_OADD(ch) = 0;

      ch->points.max_hit = 10;
      ch->points.max_mana = 0;
      ch->points.max_move = 0;

      if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
        ch->points.max_hit += GET_PRESTIGE(ch) * 12;
      }
      else {
        ch->points.max_hit += GET_PRESTIGE(ch) * 8;
      }

      if (!((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD))) {
        ch->points.max_mana += GET_PRESTIGE(ch) * 4;
      }

      ch->specials.spells_to_learn = 0;

      advance_level(ch);

      ch->specials.affected_by = 0;
      ch->specials.affected_by2 = 0;

      for (i = 0; i <= (MAX_SKILLS5 - 1); i++)
      {
        ch->skills[i].learned = 0;
      }

      for (i = 0; i < 5; i++)
      {
        ch->specials.apply_saving_throw[i] = 0;
      }

      GET_HIT(ch) = hit_limit(ch);
      GET_MANA(ch) = mana_limit(ch);
      GET_MOVE(ch) = move_limit(ch);

      switch (GET_CLASS(ch))
      {
        case CLASS_MAGIC_USER:
          break;

        case CLASS_CLERIC:
          break;

        case CLASS_THIEF:
          ch->skills[SKILL_SNEAK].learned = 10;
          ch->skills[SKILL_HIDE].learned =  5;
          ch->skills[SKILL_STEAL].learned = 15;
          ch->skills[SKILL_BACKSTAB].learned = 10;
          ch->skills[SKILL_PICK_LOCK].learned = 10;
          break;

        case CLASS_WARRIOR:
          break;

        case CLASS_NINJA:
          break;

        case CLASS_NOMAD:
          break;

        case CLASS_PALADIN:
          GET_ALIGNMENT(ch) = 500;
          break;

        case CLASS_ANTI_PALADIN:
          GET_ALIGNMENT(ch) = -500;
          break;

        case CLASS_AVATAR:
          break;

        case CLASS_BARD:
          break;

        case CLASS_COMMANDO:
          break;
      }

      if (GET_CLASS(ch) == CLASS_AVATAR)
      {
        GET_COND(ch, THIRST) = -1;
        GET_COND(ch, FULL) = -1;
        GET_COND(ch, DRUNK) = -1;
      }
      else
      {
        GET_COND(ch, THIRST) = 24;
        GET_COND(ch, FULL) = 24;
        GET_COND(ch, DRUNK) = 0;
      }

      GET_QP(ch) = MAX(0, remort_info.qp - remort_info.qp_fee);
      GET_SCP(ch) = MAX(0, remort_info.scp - remort_info.scp_fee);

      GET_REMORT_EXP(ch) += remort_info.exp;

      if (!GET_REMORT_EXP(ch))
      {
        GET_REMORT_EXP(ch) = 0;

        sprintf(buf, "WIZINFO: Remort EXP Overflow! [%s]", GET_NAME(ch));
        wizlog(buf, LEVEL_SUP, 5);
        log_f("%s", buf);
      }
      else
      {
        rv2_add_enchant(ch);
      }

      affect_total(ch);

      save_char(ch, NOWHERE);

      /* End do_class() section. */

      send_to_char("\n\r", ch);
      send_to_char("`qYou feel an overwhelming sensation that can only be described as a troubling mix of death and new life. The entire force of your being is ripped seamlessly from within as a feeling of transcendence coarses through your entire body.\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Immortalis' visage breaks into jubilance for but a brief moment as he smiles, whispering 'Your will is done. I wish you a pleasant journey and eagerly hope your new existence is a most fruitful endeavor.'\n\r", ch);

      return TRUE;
    }
    else if (!str_cmp(arg, "cancel")) /* Cancel the player's intention of remorting by clearing their inventory of any remort tokens purchased. */
    {
      if (GET_LEVEL(ch) < 6)
      {
        send_to_char("A pensive frown is seen briefly on Immortalis' face as he considers your request and then whispers 'While I appreciate your enthusiasm, I simply cannot allow you to undergo this transformation with such haste. If you do indeed endeavor to transcend this existence and into the next, return to me after you have achieved level 6 and we shall discuss these matters further.'\n\r", ch);

        return TRUE;
      }
      else if (IS_IMMORTAL(ch))
      {
        send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

        return TRUE;
      }

      send_to_char("A slight nod of reverent understanding is made as Immortalis whispers 'As you wish. If you do endeavour to proceed down the path of transcendence in the future, you need only inquire.'\n\r", ch);
      send_to_char("\n\r", ch);

      for (tmp_obj = ch->carrying; tmp_obj; tmp_obj = next_obj)
      {
        next_obj = tmp_obj->next_content;

        if (V_OBJ(tmp_obj) < RV2_TOKEN_MIN_VNUM || V_OBJ(tmp_obj) > RV2_TOKEN_MAX_VNUM) continue;

        rv2_extract_token(tmp_obj);
      }

      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

/* Object Specs */

/* Decrement the counter on tokens every tick and prevent the tokens from getting "lost" from a player's inventory. */
int rv2_obj_spec_token(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  if (cmd == MSG_TICK)
  {
    if (obj->obj_flags.timer)
    {
      obj->obj_flags.timer--;
    }

    if (!(obj->obj_flags.timer))
    {
      rv2_extract_token(obj);
    }

    return FALSE;
  }
  else if (cmd == MSG_GAVE_OBJ ||
           cmd == MSG_OBJ_DROPPED ||
           cmd == MSG_OBJ_PUT ||
           cmd == MSG_OBJ_DONATED ||
           cmd == MSG_OBJ_JUNKED)
  {
    rv2_extract_token(obj);

    return TRUE;
  }

  return FALSE;
}

/* Assign Specs */
void assign_remortv2(void)
{
  int vnum = 0;

  assign_mob(RV2_MOB_IMMORTALIS, rv2_mob_spec_immortalis);

  for (vnum = RV2_TOKEN_MIN_VNUM; vnum <= RV2_TOKEN_MAX_VNUM; vnum++)
  {
    assign_obj(vnum, rv2_obj_spec_token);
  }
}
