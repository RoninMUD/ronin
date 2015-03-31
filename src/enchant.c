
/*
$Author: ronin $
$Date: 2005/01/21 14:55:27 $
$Header: /home/ronin/cvs/ronin/enchant.c,v 2.3 2005/01/21 14:55:27 ronin Exp $
$Id: enchant.c,v 2.3 2005/01/21 14:55:27 ronin Exp $
$Name:  $
$Log: enchant.c,v $
Revision 2.3  2005/01/21 14:55:27  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.2  2004/03/02 20:54:08  ronin
Fix to prevent loss of rank in a CHAOTIC room.  Bug introducted when
trying to fix rank loss at near death.

Revision 2.1  2004/02/25 19:40:16  ronin
Change of MSG_DIE to MSG_DEAD to prevent near deaths from causing
rank loss.

Revision 2.0.0.1  2004/02/05 16:09:11  ronin
Reinitialization of cvs archives

Revision 1.4  2002/05/29 04:53:24  ronin
Activation of enchantments for hell.

Revision 1.3  2002/05/09 20:15:04  ronin
Added hell enchantments for saving through rent.

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/

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

extern struct time_info_data time_info;
extern CHAR *character_list;
extern struct room_data *world;
extern int hit_limit(CHAR *ch);
extern int mana_limit(CHAR *ch);
extern int move_limit(CHAR *ch);
extern int hit_gain(CHAR *ch);
extern int mana_gain(CHAR *ch);
extern int move_gain(CHAR *ch);

ENCH *enchantments;
void command_interpreter(CHAR *ch, char *arg);
void update_pos(CHAR *victim);

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
    if (!enchanted_by(ch, "The title of Swashbuckler"))
    {
      strcat(arg, "Squire ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Squire") &&
        !enchanted_by(ch, "The title of Swashbuckler") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.15 * hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.15 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }
  return FALSE;
}


int swashbuckler_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The title of Knight"))
    {
      strcat(arg, "Swashbuckler ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Swashbuckler") &&
        !enchanted_by(ch, "The title of Knight") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.075 * hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075 * move_gain(ch));
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
    if (enchanted_by(ch, "The title of Knight") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.075  *hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075 * move_gain(ch));
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
    if (!enchanted_by(ch, "The title of Justiciar"))
    {
      strcat(arg, "First Sword ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of First Sword") &&
        !enchanted_by(ch, "The title of Justiciar") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.1 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }
  return FALSE;
}


int justiciar_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The title of Lord/Lady"))
    {
      strcat(arg, "Justiciar ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Justiciar") &&
        !enchanted_by(ch, "The title of Lord/Lady") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (enchanted_by(ch, "The title of Lord/Lady") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (!enchanted_by(ch, "The title of Forester"))
    {
      strcat(arg, "Wanderer ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Wanderer") &&
        !enchanted_by(ch, "The title of Forester") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.15 * hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.15 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int forester_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The title of Tamer"))
    {
      strcat(arg, "Forester ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Forester") &&
        !enchanted_by(ch, "The title of Tamer") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.075 * hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075 * move_gain(ch));
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
    if (enchanted_by(ch, "The title of Tamer") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.075 * hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075 * move_gain(ch));
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
    if (!enchanted_by(ch, "The title of Warlock"))
    {
      strcat(arg, "Apprentice ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Apprentice") &&
        !enchanted_by(ch, "The title of Warlock") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.1 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int warlock_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The title of Sorcerer"))
    {
      strcat(arg, "Warlock ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Warlock") &&
        !enchanted_by(ch, "The title of Sorcerer") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (enchanted_by(ch, "The title of Sorcerer") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (!enchanted_by(ch, "The title of Poet"))
    {
      strcat(arg, "Minstrel ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Minstrel") &&
        !enchanted_by(ch, "The title of Poet") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.1 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int poet_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The title of Conductor"))
    {
      strcat(arg, "Poet ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Poet") &&
        !enchanted_by(ch, "The title of Conductor") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (enchanted_by(ch, "The title of Conductor") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (!enchanted_by(ch, "The rank of Commodore"))
    {
      strcat(arg, "Private ");

      return TRUE;
    }

    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The rank of Private") &&
        !enchanted_by(ch, "The rank of Commodore") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.1 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int commodore_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The rank of Commander"))
    {
      strcat(arg, "Commodore ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The rank of Commodore") &&
        !enchanted_by(ch, "The rank of Commander") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (enchanted_by(ch, "The rank of Commander") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (!enchanted_by(ch, "The status of Brigand"))
    {
      strcat(arg, "Highwayman ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The status of Highwayman") &&
        !enchanted_by(ch, "The status of Brigand") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.15 * hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.15 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}

int brigand_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The status of Assassin"))
    {
      strcat(arg, "Brigand ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The status of Brigand") &&
        !enchanted_by(ch, "The status of Assassin") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.075 * hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075 * move_gain(ch));
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
    if (enchanted_by(ch, "The status of Assassin") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.075 * hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075 * move_gain(ch));
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
    if (!enchanted_by(ch, "The title of Dark Warder"))
    {
      strcat(arg, "Evil Minion ");
      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Minion of Darkness") &&
        !enchanted_by(ch, "The title of Dark Warder") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.1 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int darkwarder_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The title of Dark Lord/Lady"))
    {
      strcat(arg, "Dark Warder ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Dark Warder") &&
        !enchanted_by(ch, "The title of Dark Lord/Lady") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (enchanted_by(ch, "The title of Dark Lord/Lady") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (!enchanted_by(ch, "The title of Shinobi"))
    {
      strcat(arg, "Tsume ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Tsume") &&
        !enchanted_by(ch, "The title of Shinobi") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.1 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int shinobi_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The title of Shogun"))
    {
      strcat(arg, "Shinobi ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Shinobi") &&
        !enchanted_by(ch, "The title of Shogun") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (enchanted_by(ch, "The title of Shogun") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (!enchanted_by(ch, "The title of Bishop"))
    {
      strcat(arg, "Acolyte ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Acolyte") &&
        !enchanted_by(ch, "The title of Bishop") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.1 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


int bishop_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg)
{
  if (cmd == MSG_SHOW_PRETITLE)
  {
    if (!enchanted_by(ch, "The title of Prophet"))
    {
      strcat(arg, "Bishop ");

      return TRUE;
    }
    else return FALSE;
  }

  if (cmd == MSG_DEAD)
  {
    if (enchanted_by(ch, "The title of Bishop") &&
        !enchanted_by(ch, "The title of Prophet") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
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
    if (enchanted_by(ch, "The title of Prophet") &&
        !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC))
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
    GET_HIT(ch) = MIN(hit_limit(ch), GET_HIT(ch) + 0.05 * hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05 * mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05 * move_gain(ch));
    ench->duration = -1;

    return FALSE;
  }

  return FALSE;
}


void assign_enchantments(void)
{
  log_s("Defining Enchantments");
  enchantments = (ENCH*)calloc(TOTAL_ENCHANTMENTS, sizeof(ENCH));

/*         Name                               Enchant #            Dur,Mod, Location,              Bitvector            , Bitvect2, Function */
  ENCHANTO("Remort"                         , ENCHANT_REMORTV2    , -1,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, remortv2_enchantment);
  ENCHANTO("Immortalis' Grace"              , ENCHANT_IMM_GRACE   , -1,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, imm_grace_enchantment); /* Project Broadsword */

  ENCHANTO("Common Cold"                    , ENCHANT_COLD        , 20,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, cold_enchantment);
  ENCHANTO("Fire Breath"                    , ENCHANT_FIREBREATH  ,  6,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, firebreath_enchantment);
  ENCHANTO("Regeneration"                   , ENCHANT_REGENERATION, 24,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, regeneration_enchantment);

  ENCHANTO("The title of Squire"            , ENCHANT_SQUIRE      , -1,  1, APPLY_HITROLL         , AFF_NONE            , AFF_NONE, squire_enchantment);
  ENCHANTO("The title of Swashbuckler"      , ENCHANT_SWASHBUCKLER, -1,  5, APPLY_SKILL_BLOCK     , AFF_NONE            , AFF_NONE, swashbuckler_enchantment);
  ENCHANTO("The title of Knight"            , ENCHANT_KNIGHT      , -1,  5, APPLY_SKILL_DUAL      , AFF_SENSE_LIFE      , AFF_NONE, knight_enchantment);

  ENCHANTO("The title of First Sword"       , ENCHANT_FIRSTSWORD  , -1,  1, APPLY_HITROLL         , AFF_DETECT_ALIGNMENT, AFF_NONE, firstsword_enchantment);
  ENCHANTO("The title of Justiciar"         , ENCHANT_JUSTICIAR   , -1,  5, APPLY_SKILL_BLOCK     , AFF_NONE            , AFF_NONE, justiciar_enchantment);
  ENCHANTO("The title of Lord/Lady"         , ENCHANT_LORDLADY    , -1,  5, APPLY_SKILL_PUMMEL    , AFF_NONE            , AFF_NONE, lordlady_enchantment);

  ENCHANTO("The title of Apprentice"        , ENCHANT_APPRENTICE  , -1,  0, APPLY_NONE            , AFF_DETECT_INVISIBLE, AFF_NONE, apprentice_enchantment);
  ENCHANTO("The title of Warlock"           , ENCHANT_WARLOCK     , -1,  1, APPLY_HITROLL         , AFF_DETECT_MAGIC    , AFF_NONE, warlock_enchantment);
  ENCHANTO("The title of Sorcerer"          , ENCHANT_SORCERER    , -1,  1, APPLY_DAMROLL         , AFF_FLY             , AFF_NONE, sorcerer_enchantment);

  ENCHANTO("The rank of Private"            , ENCHANT_PRIVATE     , -1,  5, APPLY_SKILL_DUAL      , AFF_NONE            , AFF_NONE, private_enchantment);
  ENCHANTO("The rank of Commodore"          , ENCHANT_COMMODORE   , -1,  5, APPLY_SKILL_ASSAULT   , AFF_NONE            , AFF_NONE, commodore_enchantment);
  ENCHANTO("The rank of Commander"          , ENCHANT_COMMANDER   , -1,  5, APPLY_SKILL_TRIPLE    , AFF_NONE            , AFF_NONE, commander_enchantment);

  ENCHANTO("The title of Wanderer"          , ENCHANT_WANDERER    , -1,  5, APPLY_SKILL_AMBUSH    , AFF_DETECT_MAGIC    , AFF_NONE, wanderer_enchantment);
  ENCHANTO("The title of Forester"          , ENCHANT_FORESTER    , -1,  1, APPLY_DAMROLL         , AFF_INFRAVISION     , AFF_NONE, forester_enchantment);
  ENCHANTO("The title of Tamer"             , ENCHANT_TAMER       , -1,  5, APPLY_SKILL_DISEMBOWEL, AFF_DETECT_INVISIBLE, AFF_NONE, tamer_enchantment);

  ENCHANTO("The title of Tsume"             , ENCHANT_TSUME       , -1,  0, APPLY_NONE            , AFF_INFRAVISION     , AFF_NONE, tsume_enchantment);
  ENCHANTO("The title of Shinobi"           , ENCHANT_SHINOBI     , -1,  5, APPLY_SKILL_PUMMEL    , AFF_SENSE_LIFE      , AFF_NONE, shinobi_enchantment);
  ENCHANTO("The title of Shogun"            , ENCHANT_SHOGUN      , -1,  5, APPLY_SKILL_ASSAULT   , AFF_NONE            , AFF_NONE, shogun_enchantment);

  ENCHANTO("The title of Minstrel"          , ENCHANT_TRAVELER    , -1,  1, APPLY_HITROLL         , AFF_NONE            , AFF_NONE, minstrel_enchantment);
  ENCHANTO("The title of Poet"              , ENCHANT_POET        , -1,  1, APPLY_DAMROLL         , AFF_SENSE_LIFE      , AFF_NONE, poet_enchantment);
  ENCHANTO("The title of Conductor"         , ENCHANT_CONDUCTOR   , -1,  5, APPLY_SKILL_BACKFLIP  , AFF_INFRAVISION     , AFF_NONE, conductor_enchantment);

  ENCHANTO("The title of Minion of Darkness", ENCHANT_MINION      , -1,  5, APPLY_SKILL_BACKSTAB  , AFF_NONE            , AFF_NONE, minion_enchantment);
  ENCHANTO("The title of Dark Warder"       , ENCHANT_DARKWARDER  , -1,  0, APPLY_NONE            , AFF_INFRAVISION     , AFF_NONE, darkwarder_enchantment);
  ENCHANTO("The title of Dark Lord/Lady"    , ENCHANT_DARKLORDLADY, -1,  1, APPLY_DAMROLL         , AFF_NONE            , AFF_NONE, darklordlady_enchantment);

  ENCHANTO("The title of Acolyte"           , ENCHANT_ACOLYTE     , -1,  0, APPLY_NONE            , AFF_SPHERE          , AFF_NONE, acolyte_enchantment);
  ENCHANTO("The title of Bishop"            , ENCHANT_BISHOP      , -1,  1, APPLY_HITROLL         , AFF_DETECT_ALIGNMENT, AFF_NONE, bishop_enchantment);
  ENCHANTO("The title of Prophet"           , ENCHANT_PROPHET     , -1,  1, APPLY_DAMROLL         , AFF_SENSE_LIFE      , AFF_NONE, prophet_enchantment);

  ENCHANTO("The status of Highwayman"       , ENCHANT_HIGHWAYMAN  , -1,  0, APPLY_NONE            , AFF_SNEAK           , AFF_NONE, highwayman_enchantment);
  ENCHANTO("The status of Brigand"          , ENCHANT_BRIGAND     , -1,  1, APPLY_HITROLL         , AFF_SENSE_LIFE      , AFF_NONE, brigand_enchantment);
  ENCHANTO("The status of Assassin"         , ENCHANT_ASSASSIN    , -1,  5, APPLY_SKILL_CIRCLE    , AFF_INFRAVISION     , AFF_NONE, assassin_enchantment);

  ENCHANTO("Deadly Sin - Wrath"             , ENCHANT_WRATH       , 66,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_wrath);
  ENCHANTO("Deadly Sin - Envy"              , ENCHANT_ENVY        , 66,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_envy);
  ENCHANTO("Deadly Sin - Lust"              , ENCHANT_LUST        , 66,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_lust);
  ENCHANTO("Deadly Sin - Pride"             , ENCHANT_PRIDE       , 66,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_pride);
  ENCHANTO("Deadly Sin - Avarice"           , ENCHANT_AVARICE     , 66,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_avarice);
  ENCHANTO("Deadly Sin - Gluttony"          , ENCHANT_GLUTTONY    , 66,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_gluttony);
  ENCHANTO("Deadly Sin - Sloth"             , ENCHANT_SLOTH       , 66,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, sin_sloth);
  ENCHANTO("Greasy Palms"                   , ENCHANT_GREASY      ,  6,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, greasy_palms);
  ENCHANTO("Red Death"                      , ENCHANT_REDDEATH    , 30,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, red_death);
  ENCHANTO("Lizard Lycanthropy"             , ENCHANT_LIZARD      , 30,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, lizard_bite);

#ifdef TEST_SITE
  ENCHANTO("Toxic Fumes"                    , ENCHANT_TOXICFUMES  , -1,  0, APPLY_NONE            , AFF_NONE            , AFF_NONE, toxic_fumes_ench);
#endif
  ENCHANTO("Frightful Presence"             , ENCHANT_FRIGHTFUL   , 10, -5, APPLY_HITROLL         , AFF_NONE            , AFF_NONE, frightful_presence);
  ENCHANTO("The Curse of the Lich"          , ENCHANT_LICH        ,  5, -5, APPLY_HITROLL         , AFF_NONE            , AFF_NONE, lich_curse);
/* These 3 were added by Quack for trial in early 1997.
  ENCHANTO("Magical Silence"                , ENCHANT_SILENCE     ,  2,  0, APPLY_NONE           , AFF_SNEAK            , AFF_NONE, silence_enchantment);
  ENCHANTO("Wolf Lycanthropy"               , ENCHANT_SHAPESHIFT  ,  2,  0, APPLY_NONE           , AFF_INFRAVISION      , AFF_NONE, shapeshift_enchantment);
  ENCHANTO("Champion Quester"               , ENCHANT_QUESTER     , -1,  3, APPLY_DAMROLL        , AFF_SANCTUARY        , AFF_NONE, quester_enchantment);*/
}


/* Note: Name takes precedence over enchantment number, unless the name isn't in the global enchantment list. */
ENCH *get_enchantment(ENCH *enchantment, int must_find)
{
  ENCH *ench;
  int index = -1;
  int i = 0;

  if (!enchantment->name)
  {
    index = enchantment->type;
  }
  else
  {
    for (i = 0; i < TOTAL_ENCHANTMENTS; i++)
    {
      if (!enchantments[i].name || !enchantment->name) continue;

      if (!strcmp(enchantments[i].name, enchantment->name)) 
      {
        index = i;
      }
    }
  }

  if (index == -1 && must_find) return NULL;

  CREATE(ench, ENCH, 1);

  if (index != -1)
  {
    if (!enchantment->name)
    {
      ench->name = str_dup(enchantments[index].name);
    }
    else
    {
      ench->name = str_dup(enchantment->name);
    }

    if (!enchantment->duration)
    {
      ench->duration = enchantments[index].duration;
    }
    else
    {
      ench->duration = enchantment->duration;
    }

    ench->location = enchantments[index].location;
    ench->modifier = enchantments[index].modifier;
    ench->bitvector = enchantments[index].bitvector;
    ench->bitvector2 = enchantments[index].bitvector2;
    ench->type = index;
    ench->func = enchantments[index].func;
    ench->duration = enchantments[index].duration;
  }
  else
  {
    ench->name = str_dup(enchantment->name);
    ench->location = enchantment->location;
    ench->modifier = enchantment->modifier;
    ench->bitvector = enchantment->bitvector;
    ench->bitvector2 = enchantment->bitvector2;
    ench->type = 0;
    ench->func = enchantment->func;
    ench->duration = enchantment->duration;
  }

  return ench;
}


void enchantment_to_char(CHAR *victim, ENCH *enchantment, int must_find)
{
  ENCH *new_ench;
  ENCH *ench;
  ENCH *next;
  ENCH *tmp_ench;

  new_ench = get_enchantment(enchantment, must_find);

  if (!new_ench) return;

  /* If there's an enchantment already there, it replaces it. */
  for (ench = victim->enchantments; ench; ench = next)
  {
    next = ench->next;

    if (!strcmp(ench->name, new_ench->name))
    {
      enchantment_remove(victim, ench, TRUE);
      ench = NULL;
    }
  }

  if (!victim->enchantments)
  {
    victim->enchantments = new_ench;
  }
  else
  {
    for (tmp_ench = victim->enchantments; tmp_ench->next; tmp_ench = tmp_ench->next);

    tmp_ench->next = new_ench;
  }

  affect_modify(victim, new_ench->location, new_ench->modifier, new_ench->bitvector, new_ench->bitvector2, TRUE);
  affect_total(victim);
  check_equipment(victim);
}


void enchantment_remove(CHAR *victim, ENCH *enchantment, int tolog)
{
  ENCH **previous;
  ENCH *ench = NULL;
  ENCH *next = NULL;
  char buf[MSL];

  affect_modify(victim, enchantment->location, enchantment->modifier, enchantment->bitvector, enchantment->bitvector2, FALSE);

  /* Find it in the linked list, and remove it. */
  previous = &victim->enchantments;
  for (ench = victim->enchantments; ench; ench = next)
  {
    next = ench->next;

    if (ench == enchantment)
    {
      *previous = next;

      if (tolog)
      {
        sprintf(buf, "PLRINFO: %s just had enchantment %s removed.", GET_NAME(victim), ench->name);
        log_s(buf);
      }

      if (enchantment->name)
      {
        free(enchantment->name);
      }
      free(enchantment);

      affect_total(victim);
      check_equipment(victim);
    }
    else
    {
      previous = &ench->next;
    }
  }
}


int enchanted_by(CHAR *ch, char *enchantment_name)
{
  ENCH *ench = NULL;

  for (ench = ch->enchantments; ench; ench = ench->next)
  {
    if (!strcmp(ench->name, enchantment_name))
    {
      return TRUE;
    }
  }

  return FALSE;
}


int enchanted_by_type(CHAR *ch, int type)
{
  ENCH *ench = NULL;

  for (ench = ch->enchantments; ench; ench = ench->next)
  {
    if (ench->type == type)
    {
      return TRUE;
    }
  }

  return FALSE;
}


int get_rank(CHAR *ch)
{
  ENCH *ench = NULL;
  int rank = 0;

  for (ench = ch->enchantments; ench; ench = ench->next)
  {
    switch (ench->type)
    {
      case ENCHANT_SQUIRE:
      case ENCHANT_FIRSTSWORD:
      case ENCHANT_APPRENTICE:
      case ENCHANT_PRIVATE:
      case ENCHANT_WANDERER:
      case ENCHANT_TSUME:
      case ENCHANT_TRAVELER:
      case ENCHANT_MINION:
      case ENCHANT_ACOLYTE:
      case ENCHANT_HIGHWAYMAN:
        if (rank < 1) rank = 1;
        break;
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
        if (rank < 2) rank = 2;
        break;
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
        if (rank < 3) rank = 3;
        break;
    }
  }

  return rank;
}


/* Project Epee */
char *get_rank_name(CHAR *ch)
{
  ENCH *ench = NULL;
  int rank = 0;

  rank = get_rank(ch);

  if (rank >= 1 && rank <= 3)
  {
    for (ench = ch->enchantments; ench; ench = ench->next)
    {
      switch (ench->type)
      {
        case ENCHANT_SQUIRE:
          if (rank == 1) return "Squire";
          break;
        case ENCHANT_FIRSTSWORD:
          if (rank == 1) return "First Sword";
          break;
        case ENCHANT_APPRENTICE:
          if (rank == 1) return "Apprentice";
          break;
        case ENCHANT_PRIVATE:
          if (rank == 1) return "Private";
          break;
        case ENCHANT_WANDERER:
          if (rank == 1) return "Wanderer";
          break;
        case ENCHANT_TSUME:
          if (rank == 1) return "Tsume";
          break;
        case ENCHANT_TRAVELER:
          if (rank == 1) return "Minstrel";
          break;
        case ENCHANT_MINION:
          if (rank == 1) return "Evil Minion";
          break;
        case ENCHANT_ACOLYTE:
          if (rank == 1) return "Acolyte";
          break;
        case ENCHANT_HIGHWAYMAN:
          if (rank == 1) return "Highwayman";
          break;
        case ENCHANT_SWASHBUCKLER:
          if (rank == 2) return "Swashbuckler";
          break;
        case ENCHANT_JUSTICIAR:
          if (rank == 2) return "Justiciar";
          break;
        case ENCHANT_WARLOCK:
          if (rank == 2) return "Warlock";
          break;
        case ENCHANT_COMMODORE:
          if (rank == 2) return "Commodore";
          break;
        case ENCHANT_FORESTER:
          if (rank == 2) return "Forester";
          break;
        case ENCHANT_SHINOBI:
          if (rank == 2) return "Shinobi";
          break;
        case ENCHANT_POET:
          if (rank == 2) return "Poet";
          break;
        case ENCHANT_DARKWARDER:
          if (rank == 2) return "Dark Warder";
          break;
        case ENCHANT_BISHOP:
          if (rank == 2) return "Bishop";
          break;
        case ENCHANT_BRIGAND:
          if (rank == 2) return "Brigand";
          break;
        case ENCHANT_KNIGHT:
          if (rank == 3) return "Knight";
          break;
        case ENCHANT_LORDLADY:
          if (rank == 3)
          {
            if (GET_SEX(ch) == SEX_FEMALE) return "Lady";
            else return "Lord";
          }
          break;
        case ENCHANT_SORCERER:
          if (rank == 3) return "Sorcerer";
          break;
        case ENCHANT_COMMANDER:
          if (rank == 3) return "Commander";
          break;
        case ENCHANT_TAMER:
          if (rank == 3) return "Tamer";
          break;
        case ENCHANT_SHOGUN:
          if (rank == 3) return "Shogun";
          break;
        case ENCHANT_CONDUCTOR:
          if (rank == 3) return "Conductor";
          break;
        case ENCHANT_DARKLORDLADY:
          if (rank == 3)
          {
            if (GET_SEX(ch) == SEX_FEMALE) return "Dark Lady";
            else return "Dark Lord";
          }
          break;
        case ENCHANT_PROPHET:
          if (rank == 3) return "Prophet";
          break;
        case ENCHANT_ASSASSIN:
          if (rank == 3) return "Assassin";
          break;
      }
    }
  }

  return "None";
}
