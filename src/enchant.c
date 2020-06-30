/****************************************************************************
*  Enchantments are basically the standard diku 'affect_type' with added    *
*  functions to make them a little more powerful.  This 'power' adds a bit  *
*  of complexity to their handling.  Some tips and traps to look out for:   *
*                                                                           *
*  o  All affects and enchantments get removed at death (except those with  *
*     a duration of -1 (never ending).                                      *
*  o  An enchantment gets EVERY cmd/signal that a player/mob would get.     *
*  o  In general, every message needs a return (FALSE) so that the true     *
*     intent of the message/command will be carried out.                    *
*     Return (TRUE) if you WANT to stop the command from happening.         *
*  o  When you return TRUE, if there's any other objects that intercept the *
*     command and don't return TRUE will still happen.  For example, you    *
*     COULD (if your heart desires) create an item to "stop time" (i.e.     *
*     return TRUE on MSG_TICK) but there's probably a LOT of items ahead of *
*     it that get signalled, so it'll only partially stop time.             *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "utility.h"
#include "cmd.h"
#include "act.h"
#include "enchant.h"
#include "spells.h"
#include "fight.h"
#include "aff_ench.h"

extern int hit_limit(CHAR *ch);
extern int mana_limit(CHAR *ch);
extern int move_limit(CHAR *ch);
extern int hit_gain(CHAR *ch);
extern int mana_gain(CHAR *ch);
extern int move_gain(CHAR *ch);

void command_interpreter(CHAR *ch, char *arg);
void update_pos(CHAR *victim);

ENCH *enchantments;

/* Hell Enchantments */
int sin_wrath(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);
int sin_envy(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);
int sin_lust(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);
int sin_avarice(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);
int sin_pride(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);
int sin_gluttony(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);
int sin_sloth(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);
int red_death(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);
int lizard_bite(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);
int greasy_palms(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);


#ifdef TEST_SITE
/* Digsite Enchantments */
int toxic_fumes_ench(ENCH *ench, CHAR *ench_ch, CHAR *char_in_room, int cmd, char *arg);
#endif

/* Red Dragons Enchantments */
int frightful_presence(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);

/* Abyss Enchantments */
int lich_curse(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);


int remortv2_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg)
{
  return FALSE;
}


int imm_grace_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg)
{
  return FALSE;
}


int cold_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg)
{
  char buf[MIL];

  if (cmd != CMD_CAST && cmd != CMD_RECITE) return FALSE;

  if (!number(0, 5))
  {
    snprintf(buf, sizeof(buf), "cough");
    command_interpreter(enchanted_ch, buf);
  }

  return FALSE;
}


int regeneration_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char*arg)
{
  OBJ *flower = NULL;
  int heal = 0;

  if (cmd == MSG_REMOVE_ENCH)
  {
    act("$n's skin loses its green tint.", TRUE, enchanted_ch, NULL, NULL, TO_ROOM);
    send_to_char("Your skin returns to its normal color.\n\r", enchanted_ch);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    if (!IS_DAY || IS_INDOORS(enchanted_ch)) return FALSE;

    heal = 14 - abs(14 - time_info.hours);

    //GET_HIT(enchanted_ch) = MIN(GET_HIT(enchanted_ch) + heal, hit_limit(enchanted_ch));
    magic_heal(enchanted_ch, SPELL_REGENERATION, heal, FALSE);

    update_pos(enchanted_ch);

    if (!number(0, 20))
    {
      send_to_char("A rose suddenly grows from your nose and falls to the ground.\n\r", enchanted_ch);
      act("$n sprouts a rose, and it drops to the ground.", FALSE, enchanted_ch, NULL, NULL, TO_ROOM);

      flower = read_object(1, REAL);
      obj_to_room(flower, CHAR_REAL_ROOM(enchanted_ch));
    }
  }

  return FALSE;
}


int firebreath_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char*arg)
{
  CHAR *victim = NULL;
  char name[MIL];

  if (cmd == MSG_SHOW_AFFECT_TEXT)
  {
    act("......$n's eyes glow an unearthly red light!", FALSE, enchanted_ch, 0, char_in_room, TO_VICT);

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("The fire in your belly subsides.\n\r", enchanted_ch);
    act("$n's belly quits rumbling and $s eyes stop glowing red.", FALSE, enchanted_ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == CMD_FIREBREATH)
  {
    if (enchanted_ch != char_in_room) return FALSE;

    ench->duration--;

    one_argument(arg, name);

    if (!(victim = get_char_room_vis(enchanted_ch, name)) && !(victim = GET_OPPONENT(enchanted_ch)))
    {
      send_to_char("Without a target, your belch gives you heartburn.\n\r", enchanted_ch);
      act("$n gets a pained look in $s face as he holds in a fiery belch.", TRUE, enchanted_ch, NULL, victim, TO_ROOM);

      damage(enchanted_ch, enchanted_ch, number(GET_LEVEL(enchanted_ch) / 5, GET_LEVEL(enchanted_ch) / 4), TYPE_UNDEFINED, DAM_FIRE);

      return TRUE;
    }

    if (number(1, 110) < MIN(100, 50 + (GET_LEVEL(enchanted_ch) - GET_LEVEL(victim)) * 3))
    {
      send_to_char("Your stomach rumbles and a tremendous belch of fire erupts.", enchanted_ch);
      act("$n's stomach rumbles and a tremendous belch of fire erupts.", TRUE, enchanted_ch, NULL, victim, TO_ROOM);

      act("Your fiery belch envelopes $N, igniting $M into a human torch.", TRUE, enchanted_ch, NULL, victim, TO_CHAR);
      act("You are enveloped by $n's fiery belch and are ignited into a human torch.", TRUE, enchanted_ch, NULL, victim, TO_VICT);
      act("$n's fiery belch envelopes $N, igniting $M into a human torch.", TRUE, enchanted_ch, NULL, victim, TO_NOTVICT);

      damage(enchanted_ch, victim, number(GET_LEVEL(enchanted_ch) * 2, GET_LEVEL(enchanted_ch) * 4), TYPE_UNDEFINED, DAM_FIRE);
    }
    else
    {
      send_to_char("Your stomach rumbles, but only a smoking oily stream of fire emerges.", enchanted_ch);
      act("$n's stomach rumbles, but only a smoking oily stream of fire emerges.", TRUE, enchanted_ch, NULL, victim, TO_ROOM);

      act("Your belch fizzles, only managing to singe $N's hair.", TRUE, enchanted_ch, NULL, victim, TO_CHAR);
      act("$n's fizzling belch only manages to singe your hair.", TRUE, enchanted_ch, NULL, victim, TO_VICT);
      act("$n's belch fizzles, only managing to singe $N's hair.", TRUE, enchanted_ch, NULL, victim, TO_NOTVICT);

      damage(enchanted_ch, victim, number(GET_LEVEL(enchanted_ch), GET_LEVEL(enchanted_ch) * 2), TYPE_UNDEFINED, DAM_FIRE);
    }

    if (ench->duration < 0)
    {
      firebreath_enchantment(ench, enchanted_ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(enchanted_ch, ench, FALSE);
    }

    return TRUE;
  }

  return FALSE;
}

/* Ranks*/

/* Warrior */
int squire_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_SWASHBUCKLER))
    {
      strcat(arg, "Squire ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_SQUIRE) &&
        !enchanted_by_type(ch, ENCHANT_SWASHBUCKLER) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      squire_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your nobility.\n\r", ch);
    act("$n is stripped of $s nobility.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }
  return FALSE;
}


int swashbuckler_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_KNIGHT))
    {
      strcat(arg, "Swashbuckler ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_SWASHBUCKLER) &&
        !enchanted_by_type(ch, ENCHANT_KNIGHT) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      swashbuckler_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int knight_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    strcat(arg, "Knight ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_KNIGHT) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      knight_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


/* Paladin */
int firstsword_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_JUSTICIAR))
    {
      strcat(arg, "First Sword ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_FIRSTSWORD) &&
        !enchanted_by_type(ch, ENCHANT_JUSTICIAR) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      firstsword_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your rank.\n\r", ch);
    act("$n is stripped of $s rank.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }
  return FALSE;
}


int justiciar_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_LORDLADY))
    {
      strcat(arg, "Justiciar ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_JUSTICIAR) &&
        !enchanted_by_type(ch, ENCHANT_LORDLADY) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      justiciar_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }
  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int lordlady_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (GET_SEX(ch) == SEX_MALE)
      strcat(arg, "Lord ");
    else
      strcat(arg, "Lady ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_LORDLADY) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      lordlady_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


/*  Nomad  */
int wanderer_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_FORESTER))
    {
      strcat(arg, "Wanderer ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_WANDERER) &&
        !enchanted_by_type(ch, ENCHANT_FORESTER) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      wanderer_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your wanderer status.\n\r", ch);
    act("$n is stripped of $s wanderer status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int forester_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_TAMER))
    {
      strcat(arg, "Forester ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_FORESTER) &&
        !enchanted_by_type(ch, ENCHANT_TAMER) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      forester_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }
    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int tamer_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    strcat(arg, "Tamer ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_TAMER) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      tamer_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


/* Mage */
int apprentice_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_WARLOCK))
    {
      strcat(arg, "Apprentice ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_APPRENTICE) &&
        !enchanted_by_type(ch, ENCHANT_WARLOCK) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      apprentice_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your apprenticeship.\n\r", ch);
    act("$n is stripped of $s apprenticeship.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int warlock_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_SORCERER))
    {
      strcat(arg, "Warlock ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_WARLOCK) &&
        !enchanted_by_type(ch, ENCHANT_SORCERER) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      warlock_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int sorcerer_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    strcat(arg, "Sorcerer ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_SORCERER) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      sorcerer_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }
  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your fame.\n\r", ch);
    act("$n is stripped of $s fame.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


/* Bard */
int minstrel_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_POET))
    {
      strcat(arg, "Minstrel ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_MINSTREL) &&
        !enchanted_by_type(ch, ENCHANT_POET) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      minstrel_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int poet_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_CONDUCTOR))
    {
      strcat(arg, "Poet ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_POET) &&
        !enchanted_by_type(ch, ENCHANT_CONDUCTOR) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      poet_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }
  return FALSE;
}


int conductor_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    strcat(arg, "Conductor ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_CONDUCTOR) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17)
    {
      conductor_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


/* Commando */
int private_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_COMMODORE))
    {
      strcat(arg, "Private ");

      return TRUE;
    }

    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_PRIVATE) &&
        !enchanted_by_type(ch, ENCHANT_COMMODORE) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      private_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your stripes.\n\r", ch);
    act("$n is stripped of $s stripes.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int commodore_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_COMMANDER))
    {
      strcat(arg, "Commodore ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_COMMODORE) &&
        !enchanted_by_type(ch, ENCHANT_COMMANDER) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      commodore_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int commander_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    strcat(arg, "Commander ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_COMMANDER) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      commander_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


/* Thief */
int highwayman_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_BRIGAND))
    {
      strcat(arg, "Highwayman ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_HIGHWAYMAN) &&
        !enchanted_by_type(ch, ENCHANT_BRIGAND) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      highwayman_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your status.\n\r", ch);
    act("$n is stripped of $s status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}

int brigand_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_ASSASSIN))
    {
      strcat(arg, "Brigand ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_BRIGAND) &&
        !enchanted_by_type(ch, ENCHANT_ASSASSIN) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      brigand_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int assassin_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    strcat(arg, "Assassin ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_ASSASSIN) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      assassin_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


/* Anti-Paladin */
int minion_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_DARKWARDER))
    {
      strcat(arg, "Evil Minion ");
      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_MINION) &&
        !enchanted_by_type(ch, ENCHANT_DARKWARDER) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      minion_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your minionhood.\n\r", ch);
    act("$n is stripped of $s minionhood.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int darkwarder_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_DARKLORDLADY))
    {
      strcat(arg, "Dark Warder ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_DARKWARDER) &&
        !enchanted_by_type(ch, ENCHANT_DARKLORDLADY) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      darkwarder_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int darklordlady_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (GET_SEX(ch) == SEX_MALE)
      strcat(arg, "Dark Lord ");
    else
      strcat(arg, "Dark Lady ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_DARKLORDLADY) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      darklordlady_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


/* Ninja */
int tsume_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_SHINOBI))
    {
      strcat(arg, "Tsume ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_TSUME) &&
        !enchanted_by_type(ch, ENCHANT_SHINOBI) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      tsume_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your honor.\n\r", ch);
    act("$n is stripped of $s honor.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int shinobi_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_SHOGUN))
    {
      strcat(arg, "Shinobi ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_SHINOBI) &&
        !enchanted_by_type(ch, ENCHANT_SHOGUN) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      shinobi_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int shogun_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    strcat(arg, "Shogun ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_SHOGUN) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      shogun_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


/* Cleric */
int acolyte_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_BISHOP))
    {
      strcat(arg, "Acolyte ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_ACOLYTE) &&
        !enchanted_by_type(ch, ENCHANT_BISHOP) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      acolyte_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your position in the church.\n\r", ch);
    act("$n is stripped of $s position in the church.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int bishop_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by_type(ch, ENCHANT_PROPHET))
    {
      strcat(arg, "Bishop ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_BISHOP) &&
        !enchanted_by_type(ch, ENCHANT_PROPHET) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      bishop_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }
  return FALSE;
}


int prophet_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    strcat(arg, "Prophet ");

    return TRUE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by_type(ch, ENCHANT_PROPHET) &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) &&
        (IS_IMMORTAL(char_in_room) || !((GET_PRESTIGE_PERK(ch) >= 17) && chance(10)))) // Prestige Perk 17
    {
      prophet_enchantment(ench, ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ch, ench, TRUE);
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH)
  {
    send_to_char("You are stripped of your ranking status.\n\r", ch);
    act("$n is stripped of $s ranking status.", FALSE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (cmd == MSG_TICK)
  {
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


void assign_enchantments(void) {
  log_s("Defining Enchantments");

  CREATE(enchantments, ENCH, TOTAL_ENCHANTMENTS);

/*         Name                               Type                  Dur, Int, Mod, Loc,                    Bitv                , Bitv2,    Func */
  ENCHANTO("Remort"                         , ENCHANT_REMORTV2    ,  -1,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, remortv2_enchantment);
  ENCHANTO("Immortalis' Grace"              , ENCHANT_IMM_GRACE   ,  -1,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, imm_grace_enchantment);

  ENCHANTO("Common Cold"                    , ENCHANT_COLD        ,  20,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, cold_enchantment);
  ENCHANTO("Fire Breath"                    , ENCHANT_FIREBREATH  ,   6,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, firebreath_enchantment);
  ENCHANTO("Regeneration"                   , ENCHANT_REGENERATION,  24,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, regeneration_enchantment);

  ENCHANTO("The title of Apprentice"        , ENCHANT_APPRENTICE  ,  -1,   0,   0, APPLY_NONE            , AFF_DETECT_INVISIBLE, AFF_NONE, apprentice_enchantment);
  ENCHANTO("The title of Warlock"           , ENCHANT_WARLOCK     ,  -1,   0,   1, APPLY_HITROLL         , AFF_DETECT_MAGIC    , AFF_NONE, warlock_enchantment);
  ENCHANTO("The title of Sorcerer"          , ENCHANT_SORCERER    ,  -1,   0,   1, APPLY_DAMROLL         , AFF_FLY             , AFF_NONE, sorcerer_enchantment);

  ENCHANTO("The title of Acolyte"           , ENCHANT_ACOLYTE     ,  -1,   0,   0, APPLY_NONE            , AFF_SPHERE          , AFF_NONE, acolyte_enchantment);
  ENCHANTO("The title of Bishop"            , ENCHANT_BISHOP      ,  -1,   0,   1, APPLY_HITROLL         , AFF_DETECT_ALIGNMENT, AFF_NONE, bishop_enchantment);
  ENCHANTO("The title of Prophet"           , ENCHANT_PROPHET     ,  -1,   0,   1, APPLY_DAMROLL         , AFF_SENSE_LIFE      , AFF_NONE, prophet_enchantment);

  ENCHANTO("The status of Highwayman"       , ENCHANT_HIGHWAYMAN  ,  -1,   0,   5, APPLY_SKILL_BACKSTAB  , AFF_NONE            , AFF_NONE, highwayman_enchantment);
  ENCHANTO("The status of Brigand"          , ENCHANT_BRIGAND     ,  -1,   0,   1, APPLY_HITROLL         , AFF_SENSE_LIFE      , AFF_NONE, brigand_enchantment);
  ENCHANTO("The status of Assassin"         , ENCHANT_ASSASSIN    ,  -1,   0,   5, APPLY_SKILL_CIRCLE    , AFF_INFRAVISION     , AFF_NONE, assassin_enchantment);

  ENCHANTO("The title of Squire"            , ENCHANT_SQUIRE      ,  -1,   0,   5, APPLY_SKILL_BLOCK     , AFF_NONE            , AFF_NONE, squire_enchantment);
  ENCHANTO("The title of Swashbuckler"      , ENCHANT_SWASHBUCKLER,  -1,   0,   5, APPLY_SKILL_DUAL      , AFF_NONE            , AFF_NONE, swashbuckler_enchantment);
  ENCHANTO("The title of Knight"            , ENCHANT_KNIGHT      ,  -1,   0,   5, APPLY_SKILL_TRIPLE    , AFF_SENSE_LIFE      , AFF_NONE, knight_enchantment);

  ENCHANTO("The title of Tsume"             , ENCHANT_TSUME       ,  -1,   0,   0, APPLY_NONE            , AFF_INFRAVISION     , AFF_NONE, tsume_enchantment);
  ENCHANTO("The title of Shinobi"           , ENCHANT_SHINOBI     ,  -1,   0,   5, APPLY_SKILL_PUMMEL    , AFF_SENSE_LIFE      , AFF_NONE, shinobi_enchantment);
  ENCHANTO("The title of Shogun"            , ENCHANT_SHOGUN      ,  -1,   0,   5, APPLY_SKILL_ASSAULT   , AFF_NONE            , AFF_NONE, shogun_enchantment);

  ENCHANTO("The title of Wanderer"          , ENCHANT_WANDERER    ,  -1,   0,   5, APPLY_SKILL_AMBUSH    , AFF_DETECT_MAGIC    , AFF_NONE, wanderer_enchantment);
  ENCHANTO("The title of Forester"          , ENCHANT_FORESTER    ,  -1,   0,   1, APPLY_DAMROLL         , AFF_INFRAVISION     , AFF_NONE, forester_enchantment);
  ENCHANTO("The title of Tamer"             , ENCHANT_TAMER       ,  -1,   0,   5, APPLY_SKILL_DISEMBOWEL, AFF_DETECT_INVISIBLE, AFF_NONE, tamer_enchantment);

  ENCHANTO("The title of First Sword"       , ENCHANT_FIRSTSWORD  ,  -1,   0,   1, APPLY_HITROLL         , AFF_DETECT_ALIGNMENT, AFF_NONE, firstsword_enchantment);
  ENCHANTO("The title of Justiciar"         , ENCHANT_JUSTICIAR   ,  -1,   0,   5, APPLY_SKILL_BLOCK     , AFF_NONE            , AFF_NONE, justiciar_enchantment);
  ENCHANTO("The title of Lord/Lady"         , ENCHANT_LORDLADY    ,  -1,   0,   5, APPLY_SKILL_PUMMEL    , AFF_NONE            , AFF_NONE, lordlady_enchantment);

  ENCHANTO("The title of Minion of Darkness", ENCHANT_MINION      ,  -1,   0,   5, APPLY_SKILL_BACKSTAB  , AFF_NONE            , AFF_NONE, minion_enchantment);
  ENCHANTO("The title of Dark Warder"       , ENCHANT_DARKWARDER  ,  -1,   0,   0, APPLY_NONE            , AFF_INFRAVISION     , AFF_NONE, darkwarder_enchantment);
  ENCHANTO("The title of Dark Lord/Lady"    , ENCHANT_DARKLORDLADY,  -1,   0,   1, APPLY_DAMROLL         , AFF_NONE            , AFF_NONE, darklordlady_enchantment);

  ENCHANTO("The title of Minstrel"          , ENCHANT_MINSTREL    ,  -1,   0,   1, APPLY_HITROLL         , AFF_NONE            , AFF_NONE, minstrel_enchantment);
  ENCHANTO("The title of Poet"              , ENCHANT_POET        ,  -1,   0,   1, APPLY_DAMROLL         , AFF_SENSE_LIFE      , AFF_NONE, poet_enchantment);
  ENCHANTO("The title of Conductor"         , ENCHANT_CONDUCTOR   ,  -1,   0,   5, APPLY_SKILL_TAUNT     , AFF_INFRAVISION     , AFF_NONE, conductor_enchantment);

  ENCHANTO("The rank of Private"            , ENCHANT_PRIVATE     ,  -1,   0,   5, APPLY_SKILL_DUAL      , AFF_NONE            , AFF_NONE, private_enchantment);
  ENCHANTO("The rank of Commodore"          , ENCHANT_COMMODORE   ,  -1,   0,   5, APPLY_SKILL_ASSAULT   , AFF_NONE            , AFF_NONE, commodore_enchantment);
  ENCHANTO("The rank of Commander"          , ENCHANT_COMMANDER   ,  -1,   0,   5, APPLY_SKILL_TRIPLE    , AFF_NONE            , AFF_NONE, commander_enchantment);

  ENCHANTO("Deadly Sin - Wrath"             , ENCHANT_WRATH       ,  66,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_wrath);
  ENCHANTO("Deadly Sin - Envy"              , ENCHANT_ENVY        ,  66,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_envy);
  ENCHANTO("Deadly Sin - Lust"              , ENCHANT_LUST        ,  66,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_lust);
  ENCHANTO("Deadly Sin - Pride"             , ENCHANT_PRIDE       ,  66,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_pride);
  ENCHANTO("Deadly Sin - Avarice"           , ENCHANT_AVARICE     ,  66,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_avarice);
  ENCHANTO("Deadly Sin - Gluttony"          , ENCHANT_GLUTTONY    ,  66,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_gluttony);
  ENCHANTO("Deadly Sin - Sloth"             , ENCHANT_SLOTH       ,  66,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_sloth);
  ENCHANTO("Greasy Palms"                   , ENCHANT_GREASY      ,   6,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, greasy_palms);
  ENCHANTO("Red Death"                      , ENCHANT_REDDEATH    ,  30,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, red_death);
  ENCHANTO("Lizard Lycanthropy"             , ENCHANT_LIZARD      ,  30,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, lizard_bite);

#ifdef TEST_SITE
  ENCHANTO("Toxic Fumes"                    , ENCHANT_TOXICFUMES  ,  -1,   0,   0, APPLY_NONE            , AFF_NONE            , AFF_NONE, toxic_fumes_ench);
#endif

  ENCHANTO("Frightful Presence"             , ENCHANT_FRIGHTFUL   ,  10,   0,  -5, APPLY_HITROLL         , AFF_NONE            , AFF_NONE, frightful_presence);

  ENCHANTO("The Curse of the Lich"          , ENCHANT_LICH        ,   5,   0,  -5, APPLY_HITROLL         , AFF_NONE            , AFF_NONE, lich_curse);
}

ENCH *get_enchantment_by_name(CHAR *ch, const char *name) {
  return ench_get_from_char(ch, name, 0);
}

ENCH *get_enchantment_by_type(CHAR *ch, int type) {
  return ench_get_from_char(ch, 0, type);
}

int enchanted_by(CHAR *ch, const char *name) {
  return ench_get_from_char(ch, name, 0) ? TRUE : FALSE;
}

int enchanted_by_type(CHAR *ch, int type) {
  return ench_get_from_char(ch, 0, type) ? TRUE : FALSE;
}

void enchantment_to_char(CHAR *victim, ENCH *enchantment, int must_find) {
  if (!victim || !enchantment) return;

  ENCH *ench = NULL;

  if (must_find) {
    ench = ench_get_from_global(enchantment->name, enchantment->type);
  }
  else {
    ench = enchantment;
  }

  if (ench) {
    ench_to_char(victim, ench, TRUE);
  }
}

void enchantment_apply(CHAR *ch, bool overwrite, const char *name, int type, sh_int duration, byte interval, int modifier, byte location, long bitvector, long bitvector2, int(*func)(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)) {
  ench_apply(ch, overwrite, name, type, duration, interval, modifier, location, bitvector, bitvector2, func);
}

void enchantment_remove(CHAR *victim, ENCH *enchantment, int to_log) {
  ench_remove(victim, enchantment, to_log);
}

int get_rank(CHAR *ch) {
  if (!ch) return 0;

  int rank = 0;

  for (ENCH *temp_ench = ch->enchantments; temp_ench; temp_ench = temp_ench->next) {
    switch (temp_ench->type) {
      case ENCHANT_APPRENTICE:
      case ENCHANT_ACOLYTE:
      case ENCHANT_HIGHWAYMAN:
      case ENCHANT_SQUIRE:
      case ENCHANT_TSUME:
      case ENCHANT_WANDERER:
      case ENCHANT_FIRSTSWORD:
      case ENCHANT_MINION:
      case ENCHANT_MINSTREL:
      case ENCHANT_PRIVATE:
        if (rank < 1) {
          rank = 1;
        }
        break;

      case ENCHANT_WARLOCK:
      case ENCHANT_BISHOP:
      case ENCHANT_BRIGAND:
      case ENCHANT_SWASHBUCKLER:
      case ENCHANT_SHINOBI:
      case ENCHANT_FORESTER:
      case ENCHANT_JUSTICIAR:
      case ENCHANT_DARKWARDER:
      case ENCHANT_POET:
      case ENCHANT_COMMODORE:
        if (rank < 2) {
          rank = 2;
        }
        break;

      case ENCHANT_SORCERER:
      case ENCHANT_PROPHET:
      case ENCHANT_ASSASSIN:
      case ENCHANT_KNIGHT:
      case ENCHANT_SHOGUN:
      case ENCHANT_TAMER:
      case ENCHANT_LORDLADY:
      case ENCHANT_DARKLORDLADY:
      case ENCHANT_CONDUCTOR:
      case ENCHANT_COMMANDER:
        if (rank < 3) {
          rank = 3;
        }
        break;
    }
  }

  return rank;
}

char *get_rank_name(CHAR *ch) {
  if (!ch) return NULL;

  struct rank_name_t {
    int class;
    int rank;
    char *name;
    char *name_female;
  };

  const struct rank_name_t rank_names[] = {
    { CLASS_MAGIC_USER,   1, "Apprentice",   NULL },
    { CLASS_MAGIC_USER,   2, "Warlock",      NULL },
    { CLASS_MAGIC_USER,   3, "Sorcerer",     NULL },

    { CLASS_CLERIC,       1, "Acolyte",      NULL },
    { CLASS_CLERIC,       2, "Bishop",       NULL },
    { CLASS_CLERIC,       3, "Apprentice",   NULL },

    { CLASS_THIEF,        1, "Highwayman",   NULL },
    { CLASS_THIEF,        2, "Brigand"  ,    NULL },
    { CLASS_THIEF,        3, "Assassin",     NULL },

    { CLASS_WARRIOR,      1, "Squire",       NULL },
    { CLASS_WARRIOR,      2, "Swashbuckler", NULL },
    { CLASS_WARRIOR,      3, "Knight",       NULL },

    { CLASS_NINJA,        1, "Tsume",        NULL },
    { CLASS_NINJA,        2, "Shinobi",      NULL },
    { CLASS_NINJA,        3, "Shogun",       NULL },

    { CLASS_NOMAD,        1, "Wanderer",     NULL },
    { CLASS_NOMAD,        2, "Forester",     NULL },
    { CLASS_NOMAD,        3, "Tamer",        NULL },

    { CLASS_PALADIN,      1, "First Sword",  NULL },
    { CLASS_PALADIN,      2, "Justiciar",    NULL },
    { CLASS_PALADIN,      3, "Lord",         "Lady" },

    { CLASS_ANTI_PALADIN, 1, "Minion",       NULL },
    { CLASS_ANTI_PALADIN, 2, "Dark Warder",  NULL },
    { CLASS_ANTI_PALADIN, 3, "Dark Lord",    "Dark Lady" },

    { CLASS_BARD,         1, "Minstrel",     NULL },
    { CLASS_BARD,         2, "Poet",         NULL },
    { CLASS_BARD,         3, "Conductor",    NULL },

    { CLASS_COMMANDO,     1, "Private",      NULL },
    { CLASS_COMMANDO,     2, "Commodore",    NULL },
    { CLASS_COMMANDO,     3, "Commander",    NULL },
  };

  int rank = get_rank(ch), rank_name_idx = -1;

  for (int i = 0; (rank_name_idx < 0) && (i < NUMELEMS(rank_names)); i++) {
    if ((rank_names[i].class == GET_CLASS(ch)) && (rank_names[i].rank == rank)) {
      rank_name_idx = i;
    }
  }

  char *rank_name = NULL;

  if (rank_name_idx >= 0) {
    if (GET_SEX(ch) != SEX_FEMALE) {
      rank_name = rank_names[rank_name_idx].name;
    }
    else {
      rank_name = rank_names[rank_name_idx].name_female;
    }
  }
  else {
    rank_name = "None";
  }

  return rank_name;
}
