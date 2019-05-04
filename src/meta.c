/*
  meta.c - Specs for Meta, re-written by Night

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 2/5/2012
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "act.h"
#include "cmd.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "spec_assign.h"
#include "time.h"
#include "meta.h"

int get_max_stat(CHAR *ch, int stat)
{
  int value = 0;

  switch (stat)
  {
    case META_HIT:
      value = ch->specials.org_hit;

      if (GET_PRESTIGE(ch)) {
        if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
          value -= GET_PRESTIGE(ch) * (PRESTIGE_HIT_GAIN + PRESTIGE_MANA_GAIN);
        }
        else {
          value -= GET_PRESTIGE(ch) * PRESTIGE_HIT_GAIN;
        }
      }
      break;

    case META_MANA:
      value = ch->specials.org_mana;

      if (!((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD))) {
        value -= GET_PRESTIGE(ch) * PRESTIGE_MANA_GAIN;
      }
      break;

    case META_MOVE:
      value = ch->specials.org_move;
      break;
  }

  return value;
}

int get_ability(CHAR *ch, int ability)
{
  int value = -1;
  
  switch (ability)
  {
    case META_STR:
      value = MIN(18, GET_OSTR(ch));
      break;

    case META_INT:
      value = MIN(18, GET_OINT(ch));
      break;

    case META_WIS:
      value = MIN(18, GET_OWIS(ch));
      break;

    case META_DEX:
      value = MIN(18, GET_ODEX(ch));
      break;

    case META_CON:
      value = MIN(18, GET_OCON(ch));
      break;

    case META_STR_ADD:
      if (GET_OSTR(ch) > 18)
      {
        value = META_MAX_STR_ADD;
      }
      else
      {
        value = GET_OADD(ch);
      }
      break;
  }

  return value;
}

void adjust_hit_mana_move(CHAR *ch, int stat, int amt)
{
  switch (stat)
  {
    case META_HIT:
      ch->points.max_hit += MIN(META_MAX_HIT_MANA_MOVE - ch->points.max_hit, amt);
      break;

    case META_MANA:
      ch->points.max_mana += MIN(META_MAX_HIT_MANA_MOVE - ch->points.max_mana, amt);
      break;

    case META_MOVE:
    case META_MOVE_UP_1:
    case META_MOVE_DOWN_1:
      ch->points.max_move += MIN(META_MAX_HIT_MANA_MOVE - ch->points.max_move, amt);
      break;
  }
}

void adjust_ability(CHAR *ch, int ability, int amt)
{
  switch (ability)
  {
    case META_STR:
      GET_OSTR(ch) += MIN(META_MAX_ABILITY - GET_OSTR(ch), amt);
      break;

    case META_INT:
      GET_OINT(ch) += MIN(META_MAX_ABILITY - GET_OINT(ch), amt);
      break;

    case META_WIS:
      GET_OWIS(ch) += MIN(META_MAX_ABILITY - GET_OWIS(ch), amt);
      break;

    case META_DEX:
      GET_ODEX(ch) += MIN(META_MAX_ABILITY - GET_ODEX(ch), amt);
      break;

    case META_CON:
      GET_OCON(ch) += MIN(META_MAX_ABILITY - GET_OCON(ch), amt);
      break;

    case META_STR_ADD:
      GET_OADD(ch) += MIN(META_MAX_STR_ADD - GET_OADD(ch), amt);
      break;
  }
}

int hit_mana_cost(CHAR *ch, int stat)
{
  int price = 0;

  const int hit_mana_cost_matrix[2][11] =
  {
    /* MU,  Cl,  Th,  Wa,  Ni,  No,  Pa,  AP,  Av,  Ba,  Co */
    { 150, 250, 400, 500, 300, 700, 300, 300,   0, 300, 300 }, /* Hit */
    { 300, 300, 100, 100, 200, 100, 200, 200,   0, 250, 200 }  /* Mana */
  };

  const float cost_mult[2][11] =
  {
    /* MU,  Cl,  Th,  Wa,  Ni,  No,  Pa,  AP,  Av,  Ba,  Co */
    {   1,   1, .95,  .9,   1,  .8,   1,   1,   1,   1,   1 }, /* Hit */
    {   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1 }  /* Mana */
  };

  if (stat == META_HIT ||
      stat == META_MANA)
  {
    price = (get_max_stat(ch, stat) - hit_mana_cost_matrix[stat - 1][GET_CLASS(ch) - 1]);
    price *= cost_mult[stat - 1][GET_CLASS(ch) - 1];
    price = (1000000 * (MAX(-1, (price / 100)) + 3));
  }

  return price;
}

int str_add_cost(CHAR *ch)
{
  int price = 0;

  if (GET_OADD(ch) < 50)
    price = 1000000;
  else
  if (GET_OADD(ch) < 75)
    price = 2000000;
  else
  if (GET_OADD(ch) < 90)
    price = 4000000;
  else
  if (GET_OADD(ch) < 99)
    price = 8000000;
  else
  if (GET_OADD(ch) == 99)
    price = 18000000;

  return price;
}

int increase_age_cost(CHAR *ch)
{
  int price = 0;
  int years = 0;
  int i = 0;
  OBJ *eq[MAX_WEAR];

  for (i = 0; i < MAX_WEAR; i++)
  {
    if (ch->equipment[i])
      eq[i] = unequip_char(ch, i);
    else
      eq[i] = NULL;
  }

  years = age(ch).year;

  for (i = 0; i < WIELD; i++)
    if (eq[i])
      equip_char(ch, eq[i], i);
  if (eq[HOLD])
    equip_char(ch, eq[HOLD], HOLD);
  if (eq[WIELD])
    equip_char(ch, eq[WIELD], WIELD);

  if (years < 20)
    price = 1500000;
  else
  if (years < 25)
    price = 2000000;
  else
  if (years < 30)
    price = 2500000;
  else
  if (years < 35)
    price = 3000000;
  else
  if (years < 40)
    price = 4000000;
  else
    price = 5000000;

  return price;
}

int decrease_age_cost(CHAR *ch)
{
  int price = 0;
  int years = 0;
  int i = 0;
  OBJ *eq[MAX_WEAR];

  for (i = 0; i < MAX_WEAR; i++)
  {
    if (ch->equipment[i])
      eq[i] = unequip_char(ch, i);
    else
      eq[i] = NULL;
  }

  years = age(ch).year;

  for (i = 0; i < WIELD; i++)
    if (eq[i])
      equip_char (ch, eq[i], i);
  if (eq[HOLD])
    equip_char(ch, eq[HOLD], HOLD);
  if (eq[WIELD])
    equip_char(ch, eq[WIELD], WIELD);

  if (years < 60)
    price = 5000000;
  else
  if (years < 75)
    price = 4000000;
  else
  if (years < 90)
    price = 3000000;
  else
  if (years < 110)
    price = 2000000;
  else
  if (years < 130)
    price = 1500000;
  else
    price = 1000000;

  return price;
}

int get_meta_adjust(int choice, int bribe)
{
  int adjust = 0;

  switch (choice)
  {
    case META_HIT:
      adjust = bribe ? number(5, 10) : number(2, 7);
      break;

    case META_MANA:
      adjust = bribe ? number(5, 10) : number(2, 7);
      break;

    case META_MOVE:
      adjust = 30;
      break;

    case META_PRAC:
      adjust = number(3, 4);
      break;

    case META_STR:
    case META_INT:
    case META_WIS:
    case META_DEX:
    case META_CON:
      adjust = 1;
      break;

    case META_STR_ADD:
      adjust = 1;
      break;

    case META_AGE_UP:
      adjust = (-1 * (SECS_PER_MUD_YEAR * (5 + number(0, 4))));
      break;

    case META_AGE_DOWN:
      adjust = (SECS_PER_MUD_YEAR * (5 + number(0, 4)));
      break;

    case META_MOVE_UP_1:
      adjust = 1;
      break;

    case META_MOVE_DOWN_1:
      adjust = -1;
      break;

    default:
      adjust = 0;
      break;
  }

  return adjust;
}

int meta_cost(CHAR *ch, int choice)
{
  int cost = 0;

  const int ability_cost_matrix[] =
  {
    /* Exp */ /* Stat */
    0, /* Never - 1   */
    0, /* Never - 2   */
    0, /* Never - 3   */
    100000,   /*  3   */
    100000,   /*  4   */
    100000,   /*  5   */
    100000,   /*  6   */
    100000,   /*  7   */
    200000,   /*  8   */
    250000,   /*  9   */
    300000,   /* 10   */
    400000,   /* 11   */
    700000,   /* 12   */
    1000000,  /* 13   */
    2000000,  /* 14   */
    3000000,  /* 15   */
    5000000,  /* 16   */
    10000000, /* 17   */
    0 /* Never - 18   */
  };

  switch (choice)
  {
    case META_HIT:
    case META_MANA:
      cost = hit_mana_cost(ch, choice);
      break;

    case META_MOVE:
      cost = META_MOVE_COST;
      break;

    case META_PRAC:
      cost = META_PRAC_COST;
      break;

    case META_STR:
    case META_INT:
    case META_WIS:
    case META_DEX:
    case META_CON:
      cost = ability_cost_matrix[get_ability(ch, choice)];
      break;

    case META_STR_ADD:
      cost = str_add_cost(ch);
      break;

    case META_AGE_UP:
      cost = increase_age_cost(ch);
      break;

    case META_AGE_DOWN:
      cost = decrease_age_cost(ch);
      break;

    case META_MOVE_UP_1:
      cost = META_MOVE_UP1_COST;
      break;

    case META_MOVE_DOWN_1:
      cost = META_MOVE_DOWN1_COST;
      break;

    default:
      cost = 0;
      break;
  }

  return cost;
}

int meta(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  char buf[MSL];
  int bribe = FALSE;
  int choice = 0;
  int exp = 0;
  int gold = 0;
  int check = 0;
  int max = 0;
  int adjust = 0;

  if (cmd == MSG_TICK)
  {
    if (chance(10))
    {
      if (chance(50))
      {
        do_say(mob, "For the right price I might be able to help you a little more.", CMD_SAY);
      }
      else
      {
        do_say(mob, "Twice the experience in gold is all I ask.", CMD_SAY);
      }
    }

    return FALSE;
  }

  if (cmd == CMD_LIST)
  {
    if (!IS_MORTAL(ch) ||
        GET_LEVEL(ch) < 6)
    {
      send_to_char("The Metaphysician tells you, 'You probably shouldn't buy anything.'\n\r", ch);
      return TRUE;
    }

    sprintf(buf, "The Metaphysician tells you 'You have %d experience available.'\n\r", GET_EXP(ch));
    send_to_char(buf, ch);

    sprintf(buf, "\n\r\
Listed on a sign are the Metaphysician's prices in experience:\n\r\
 1) Hit Points        %d\n\r\
 2) Mana              %d\n\r\
 3) +30 Movement      %d\n\r\
 4) Practice Sessions %d\n\r\
 5) Strength          %d\n\r\
 6) Intelligence      %d\n\r\
 7) Wisdom            %d\n\r\
 8) Dexterity         %d\n\r\
 9) Constitution      %d\n\r\
10) Strength Percent  %d\n\r\
11) Increase Age      %d and %d coins\n\r\
12) Decrease Age      %d and %d coins\n\r\
13) +1 Movement       %d\n\r\
14) -1 Movement       %d\n\r",
      meta_cost(ch, META_HIT),
      meta_cost(ch, META_MANA),
      meta_cost(ch, META_MOVE),
      meta_cost(ch, META_PRAC),
      meta_cost(ch, META_STR),
      meta_cost(ch, META_INT),
      meta_cost(ch, META_WIS),
      meta_cost(ch, META_DEX),
      meta_cost(ch, META_CON),
      meta_cost(ch, META_STR_ADD),
      meta_cost(ch, META_AGE_UP), meta_cost(ch, META_AGE_UP),
      meta_cost(ch, META_AGE_DOWN), meta_cost(ch, META_AGE_DOWN),
      meta_cost(ch, META_MOVE_UP_1),
      meta_cost(ch, META_MOVE_DOWN_1));
    send_to_char(buf, ch);

    return TRUE;
  }

  if (cmd == CMD_UNKNOWN)
  {
    arg = one_argument(arg, buf);

    if (str_cmp(buf, "bribe")) return FALSE;

    bribe = TRUE;

    cmd = CMD_BUY;
  }

  if (cmd == CMD_BUY)
  {
    arg = one_argument(arg, buf);

    if (!*buf)
    {
      if (bribe)
      {
        send_to_char("The Metaphysician tells you 'What are you trying to bribe me for?'\n\r", ch);
      }
      else
      {
        send_to_char("The Metaphysician tells you 'Buy what?'\n\r", ch);
      }

      return TRUE;
    }

    if (!IS_MORTAL(ch) ||
        GET_LEVEL(ch) < 6)
    {
      send_to_char("The Metaphysician tells you, 'You probably shouldn't buy anything.'\n\r", ch);
      return TRUE;
    }

    choice = atoi(buf);

    if (bribe && (choice != META_HIT) && (choice != META_MANA))
    {
      send_to_char("The Metaphysician tells you 'I don't take bribes for that.'\n\r", ch);
      return TRUE;
    }

    switch (choice)
    {
      case META_HIT:
        check = GET_MAX_HIT(ch);
        max = META_MAX_HIT_MANA_MOVE;
        break;

      case META_MANA:
        check = GET_MAX_MANA(ch);
        max = META_MAX_HIT_MANA_MOVE;
        break;

      case META_MOVE:
        check = GET_MAX_MOVE(ch);
        max = META_MAX_HIT_MANA_MOVE;
        break;

      case META_PRAC:
        check = ch->specials.spells_to_learn;
        max = META_MAX_PRAC;
        break;

      case META_STR:
      case META_INT:
      case META_WIS:
      case META_DEX:
      case META_CON:
        check = get_ability(ch, choice);
        max = META_MAX_ABILITY;
        break;

      case META_STR_ADD:
        if (ch->abilities.str < 18)
        {
          send_to_char("The Metaphysician tells you 'Get your normal strength fixed first!'\n\r", ch);
          return TRUE;
        }

        check = ch->abilities.str_add;
        max = META_MAX_STR_ADD;
        break;

      case META_AGE_UP:
        check = 0; /* Warning - No min/max check. */
        max = 1; /* Warning - No min/max check. */
        gold = meta_cost(ch, choice);
        break;

      case META_AGE_DOWN:
        check = 0; /* Warning - No min/max check. */
        max = 1; /* Warning - No min/max check. */
        gold = meta_cost(ch, choice);
        break;

      case META_MOVE_UP_1:
        check = GET_MAX_MOVE(ch);
        max = META_MAX_HIT_MANA_MOVE;
        break;

      case META_MOVE_DOWN_1:
        check = 0; /* Warning - No min/max check. */
        max = 1; /* Warning - No min/max check. */
        break;

      default:
        send_to_char("The Metaphysician asks you 'What do you mean?'\n\r", ch);
        return TRUE;
        break;
    }

    exp = meta_cost(ch, choice);

    if (check >= max)
    {
      send_to_char("The Metaphysician tells you 'Sorry, but I can't help you with that anymore.'\n\r", ch);

      return TRUE;
    }

    if (exp > 0 && GET_EXP(ch) < exp)
    {
      send_to_char("The Metaphysician tells you 'Sorry, but you can't pay me for that!'\n\r", ch);

      return TRUE;
    }

    if (bribe)
    {
      gold = exp * 2;
    }

    // Prestige Perk 10
    if (GET_PRESTIGE_PERK(ch) >= 10) {
      gold *= 0.95;
    }

    if (gold > 0 && GET_GOLD(ch) < gold)
    {
      send_to_char("The Metaphysician tells you 'Sorry, you don't have enough gold for that!\n\r", ch);

      return TRUE;
    }

    bool free_bribe = FALSE;

    // Prestige Perk 24
    if ((GET_PRESTIGE_PERK(ch) >= 24) && chance(2) && ((choice == META_HIT) || (choice == META_MANA))) {
      free_bribe = TRUE;

      send_to_char("The Metaphysician tells you 'Congratulations, you got a free bribe!'\n\r", ch);
    }

    adjust = get_meta_adjust(choice, (bribe || free_bribe));

    if (choice == META_MOVE_UP_1 || choice == META_MOVE_DOWN_1)
    {
      choice = META_MOVE;
    }

    int half_price_chance = 2;

    // Prestige Perk 3
    if (GET_PRESTIGE_PERK(ch) >= 3) {
      half_price_chance += 1;
    }

    if (chance(half_price_chance))
    {
      send_to_char("The Metaphysician tells you 'Congratulations, you get this one half-price!'\n\r", ch);

      exp /= 2;
      gold /= 2;
    }

    send_to_char("The Metaphysician tells you 'Pleasure doing business with you. Thank you!'\n\r", ch);

    if (exp > 0)
    {
      ch->points.exp -= exp;
    }

    if (gold > 0)
    {
      ch->points.gold -= gold;
    }

    switch (choice)
    {
      case META_HIT:
      case META_MANA:
      case META_MOVE:
      case META_MOVE_UP_1:
      case META_MOVE_DOWN_1:
        adjust_hit_mana_move(ch, choice, adjust);
        break;

      case META_PRAC:
        ch->specials.spells_to_learn = MIN(ch->specials.spells_to_learn + adjust, 127);
         break;

      case META_STR:
      case META_INT:
      case META_WIS:
      case META_DEX:
      case META_CON:
      case META_STR_ADD:
        adjust_ability(ch, choice, adjust);
        ch->tmpabilities = ch->abilities;
        break;

      case META_AGE_UP:
      case META_AGE_DOWN:
        ch->player.time.birth += adjust;
        break;
    }

    send_to_char("The Metaphysician grabs you and hits your head on the floor!\n\r", ch);
    act("The Metaphysician grabs $n and hits $s head on the floor.", FALSE, ch, NULL, NULL, TO_ROOM);

    if (ch->bot.meta_update + 600 > time(0))
    {
      ch->bot.meta_number++;
      ch->bot.meta_amount += exp;

      if (ch->bot.meta_number > 1)
      {
        sprintf(buf,"WARNING: %s has meta'd %d times with %d exp in less than 10 mins.",
                GET_NAME(ch), ch->bot.meta_number, ch->bot.meta_amount);
        log_f("%s", buf);
      }
    }

    ch->bot.meta_update = time(0);

    affect_total(ch);

    save_char(ch, NOWHERE);

    return TRUE;
  }

  return FALSE;
}

/* Assign Specs */
void assign_meta(void)
{
  assign_mob(12, meta);
}

