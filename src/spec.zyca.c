/*
  spec.zyca.c - Specs for Zyca City (Revamp)

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 04/13/2012
*/

/* Includes */
/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ronin Includes */
#include "structs.h"
#include "utils.h"
#include "act.h"
#include "cmd.h"
#include "comm.h"
#include "fight.h"
#include "spec_assign.h"
#include "spells.h"
#include "subclass.h"

/* Defines */
#define ZYCA_ZONE      10900

/* Mobile Defines */
#define ZYCA_PRINCE    10901
#define ZYCA_QUEEN     10902
#define ZYCA_KING      10903
#define ZYCA_BODYGUARD 10905
#define ZYCA_ZYCA      10907

/* Mobile Specs */
int zyca_prince_queen(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL;

  if (cmd != MSG_MOBACT) return FALSE;

  if (!mob->specials.fighting)
  {
    if (!number(0, 3))
    {
      act("$n sighs loudly!", FALSE, mob, NULL, NULL, TO_ROOM);
    }

    return FALSE;
  }

  if (!(vict = mob->specials.fighting)) return FALSE;

  if (vict && CHAR_REAL_ROOM(vict) == CHAR_REAL_ROOM(mob) && !number(0, 2))
  {
    switch (number(0, 3))
    {
      case 0:
        act("$n slaps $N hard.", TRUE, mob, NULL, vict, TO_NOTVICT);
        act("$n slaps you hard.", TRUE, mob, 0, vict, TO_VICT);
        cast_curse(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
        break;

      case 1:
        act("$n grins evilly.", TRUE, mob, NULL, NULL, TO_ROOM);
        cast_blindness(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
        break;

      case 2:
        act("$n smiles happily.", TRUE, mob, NULL, NULL, TO_ROOM);
        cast_heal(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, mob, NULL);
        break;

      case 3:
        act("$n glares icily at $N.", TRUE, mob, NULL, vict, TO_NOTVICT);
        act("$n glares icily at you; you feel cold to your bones.", TRUE, mob, NULL, vict, TO_VICT);
        cast_lightning_bolt(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
        break;

      default:
        break;
    }
  }

  return FALSE;
}

int zyca_king(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL;
  CHAR *bodyguard = NULL;

  if (cmd != MSG_MOBACT) return FALSE;

  if (!mob->specials.fighting)
  {
    if (!number(0, 7))
    {
      act("$n is counting his gold coins!", FALSE, mob, NULL, NULL, TO_ROOM);
    }

    return FALSE;
  }

  if (!(vict = mob->specials.fighting)) return FALSE;

  if (vict && CHAR_REAL_ROOM(vict) == CHAR_REAL_ROOM(mob) && !number(0, 2))
  {
    switch (number(0, 1))
    {
      case 0:
        act("$n shouts 'HELP!'", FALSE, mob, NULL, NULL, TO_ROOM);

        for (bodyguard = world[CHAR_REAL_ROOM(mob)].people; bodyguard; bodyguard = bodyguard->next_in_room)
        {
          if (!IS_NPC(bodyguard) || bodyguard == mob || bodyguard->specials.fighting || (V_MOB(bodyguard) < ZYCA_ZONE || V_MOB(bodyguard) > ZYCA_ZONE + 99)) continue;

          act("$n leaps into action and assists $s king!", TRUE, bodyguard, NULL, NULL, TO_ROOM);
          set_fighting(bodyguard, mob->specials.fighting);
        }
        break;

      case 1:
        act("$n says, 'Never felt so good!'", TRUE, mob, NULL, NULL, TO_ROOM);
        cast_heal(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, mob, NULL);
        break;

      default:
        break;
    }
  }

  return FALSE;
}

int zyca_bodyguard(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL;

  if (cmd != MSG_MOBACT) return FALSE;

  if (!mob->specials.fighting) return FALSE;

  for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room)
  {
     if (IS_MORTAL(vict) && vict->specials.fighting && IS_NPC(vict->specials.fighting) && (V_MOB(vict->specials.fighting) == ZYCA_KING || vict == mob->specials.fighting) && !number(0, 2)) break;
  }

  if (!vict) return FALSE;

  act("$n circles to $N's back, and kicks $M very hard.", TRUE, mob, NULL, vict, TO_NOTVICT);
  act("$n circles to your back and kicks you very hard.", FALSE, mob, NULL, vict, TO_VICT);
  damage(mob, vict, number(30, 40), TYPE_UNDEFINED, DAM_NO_BLOCK);

  return FALSE;
}

/* Re-written by Hemp and Night on or around 2012-04-13 to make a viable level 50 XP mob. */
int zyca(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL;
  CHAR *next_vict = NULL;

  if (cmd != MSG_MOBACT) return FALSE;

  if (!mob->specials.fighting) return FALSE;

  if ((GET_POS(mob) == POSITION_FIGHTING) || chance(25))
  {
    if (!(vict = get_random_victim(mob))) return FALSE;

    act("$n screams, 'Yaf hpz ohifl ay Zyca!'", TRUE, mob, NULL, NULL, TO_ROOM);

    switch (dice(2, 4))
    {
      case 2:/* least likely: frost bolt */
        cast_frostbolt(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
        break;

      case 3:/* paralyze */
        cast_paralyze(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
        break;

      case 4:/* poison */
        cast_poison(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
        break;

      case 5:/* most likely: room blindness */
        for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
        {
          next_vict = vict->next_in_room;

          if (IS_MORTAL(vict))
          {
            cast_blindness(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
          }
        }
        break;

      case 6:/* curse */
        cast_curse(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
        break;

      case 7:/* orb of protection */
        cast_orb_protection(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, mob, NULL);
        break;

      case 8:/* least likely: vampiric touch */
        cast_vampiric_touch(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
        break;

      default:
        break;
    }
  }

  return FALSE;
}

/* Spec Assignment */
void assign_zyca(void)
{
  assign_mob(ZYCA_PRINCE,    zyca_prince_queen);
  assign_mob(ZYCA_QUEEN,     zyca_prince_queen);
  assign_mob(ZYCA_KING,      zyca_king);
  assign_mob(ZYCA_BODYGUARD, zyca_bodyguard);
  assign_mob(ZYCA_ZYCA,      zyca);
}
