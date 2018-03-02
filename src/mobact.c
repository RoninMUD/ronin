/* ************************************************************************
*  file: mobact.c , Mobile action module.                 Part of DIKUMUD *
*  Usage: Procedures generating 'intelligent' behavior in the mobiles.    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "constants.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "utility.h"
#include "handler.h"
#include "cmd.h"
#include "act.h"
#include "subclass.h"

void hit(CHAR *ch, CHAR *victim, int type);
void do_kill(CHAR *ch, char *argument, int cmd);

bool check_aggro_target(CHAR *attacker, CHAR *defender) {
  if (IS_NPC(defender) ||
      IS_IMMORTAL(defender) ||
      !CAN_SEE(attacker, defender) ||
      (IS_SET(GET_ACT(attacker), ACT_WIMPY) && AWAKE(defender)) ||
      ((IS_AFFECTED(defender, AFF_SPHERE) || IS_AFFECTED(defender, AFF_INVUL)) && (GET_LEVEL(attacker) <= (GET_LEVEL(defender) - 5)) && !IS_SET(GET_ACT2(attacker), ACT_IGNORE_SPHERE))) {
    return FALSE;
  }

  return TRUE;
}

int mob_act_test_object(OBJ *obj) {
  for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (((OBJ_AFF(obj, i).location == APPLY_AC) && (OBJ_AFF(obj, i).modifier > 0)) ||
        ((((OBJ_AFF(obj, i).location == APPLY_DAMROLL) || (OBJ_AFF(obj, i).location == APPLY_HITROLL))) && (OBJ_AFF(obj, i).modifier < 0))) {
      return FALSE;
    }
  }

  return TRUE;
}

void mobile_activity(CHAR *mob) {
  if (!mob || !IS_MOB(mob) || (CHAR_REAL_ROOM(mob) == NOWHERE) || GET_OPPONENT(mob) || !(AWAKE(mob) || GET_POS(mob) == POSITION_STUNNED)) return;

  /* Scavenger */
  if (IS_SET(GET_ACT(mob), ACT_SCAVENGER)) {
    if ((CHAR_VIRTUAL_ROOM(mob) != ROOM_MORGUE) && world[CHAR_REAL_ROOM(mob)].contents && !number(0, 3)) {
      OBJ *best_obj = NULL;

      for (OBJ *tmp_obj = world[CHAR_REAL_ROOM(mob)].contents; tmp_obj; tmp_obj = tmp_obj->next_content) {
        if (CAN_GET_OBJ(mob, tmp_obj) && !IS_SET(OBJ_WEAR_FLAGS(tmp_obj), ITEM_NO_SCAVENGE) && (OBJ_COST(tmp_obj) > ((best_obj == NULL) ? 1 : OBJ_COST(best_obj)))) {
          best_obj = tmp_obj;
        }
      }

      if (best_obj) {
        obj_from_room(best_obj);
        obj_to_char(best_obj, mob);

        act("$n gets $p.", FALSE, mob, best_obj, 0, TO_ROOM);

        if (mob_act_test_object(best_obj)) {
          switch (OBJ_TYPE(best_obj)) {
            case ITEM_LIGHT:
              wear(mob, best_obj, 0);
              break;
            case ITEM_WEAPON:
            case ITEM_2HWEAPON:
              wear(mob, best_obj, 12);
              break;
            case ITEM_FIREWEAPON:
            case ITEM_KEY:
            case ITEM_MUSICAL:
              wear(mob, best_obj, 13);
              break;
            default:
              do_wear(mob, "all", 0);
              break;
          }
        }
      }
    }
  }

  /* Movement */
  if (!IS_SET(GET_ACT(mob), ACT_SENTINEL) &&
      !GET_RIDER(mob) &&
      !IS_AFFECTED(mob, AFF_HOLD) &&
      (GET_MOVE(mob) > 0) &&
      ((GET_POS(mob) == POSITION_STANDING) || (GET_POS(mob) == POSITION_RIDING) || (GET_POS(mob) == POSITION_FLYING))) {
    int dir = number(0, 45);

    if ((dir  <= DOWN) &&
        CAN_GO(mob, dir) &&
        !IS_SET(world[EXIT(mob, dir)->to_room_r].room_flags, NO_MOB) &&
        !IS_SET(world[EXIT(mob, dir)->to_room_r].room_flags, SAFE) &&
        !IS_SET(world[EXIT(mob, dir)->to_room_r].room_flags, DEATH) &&
        !IS_SET(world[EXIT(mob, dir)->to_room_r].room_flags, HAZARD)) {
      if (GET_LAST_DIR(mob) == dir) {
        GET_LAST_DIR(mob) = NOWHERE;
      }
      else if (!IS_SET(GET_ACT(mob), ACT_STAY_ZONE) || (world[EXIT(mob, dir)->to_room_r].zone == world[CHAR_REAL_ROOM(mob)].zone)) {
        GET_LAST_DIR(mob) = dir;
        do_move(mob, "", ++dir);
      }
    }
  }

  /* Open Doors */
  if (IS_SET(GET_ACT(mob), ACT_OPEN_DOOR)) {
    for (int door = NORTH; door <= DOWN; door++) {
      if (!chance(5) ||
          !EXIT(mob, door) ||
          !IS_SET(EXIT(mob, door)->exit_info, EX_ISDOOR) ||
          !EXIT(mob, door)->keyword) {
        continue;
      }

      char buf[MIL];
      snprintf(buf, sizeof(buf), " %s %s", EXIT(mob, door)->keyword, dirs[door]);

      IS_SET(EXIT(mob, door)->exit_info, EX_CLOSED) ? do_open(mob, buf, CMD_OPEN) : do_close(mob, buf, CMD_CLOSE);
      break;
    }
  }

  /* Aggro */

  /* Check for conditions that prevent aggro/memory. */
  if (CHAR_REAL_ROOM(mob) == NOWHERE ||
      GET_OPPONENT(mob) ||
      !count_mortals_real_room(CHAR_REAL_ROOM(mob)) ||
      IS_AFFECTED(mob, AFF_HOLD) ||
      IS_SET(GET_ACT(mob), ACT_SUBDUE)) {
    return;
  }

  CHAR *aggro_target = NULL;

  if (IS_SET(GET_ACT(mob), ACT_AGGRANDOM)) {
    aggro_target = get_random_victim(mob);
  }
  else {
    const int act_aggro_table[] = {
      ACT_AGGMU,
      ACT_AGGCL,
      ACT_AGGTH,
      ACT_AGGWA,
      ACT_AGGNI,
      ACT_AGGNO,
      ACT_AGGPA,
      ACT_AGGAP,
      ACT_AGGBA,
      ACT_AGGCO,
      ACT_AGGEVIL,
      ACT_AGGGOOD,
      ACT_AGGNEUT
    };

    int tmp_aggro_table[NUMELEMS(act_aggro_table)];

    memset(tmp_aggro_table, 0, NUMELEMS(tmp_aggro_table) * sizeof(int));

    int num_aggro = 0;

    for (int i = 0; i < NUMELEMS(act_aggro_table); i++) {
      if (IS_SET(GET_ACT(mob), act_aggro_table[i])) {
        tmp_aggro_table[i] = act_aggro_table[i];
        num_aggro++;
      }
    }

    if (num_aggro) {
      shuffle_int_array(tmp_aggro_table, num_aggro);

      for (int j = 0; (j < num_aggro) && !aggro_target; j++) {
        for (CHAR *tmp_ch = world[CHAR_REAL_ROOM(mob)].people; tmp_ch && !aggro_target; tmp_ch = tmp_ch->next_in_room) {
          if (!check_aggro_target(mob, tmp_ch)) continue;

          if (((tmp_aggro_table[j] == ACT_AGGMU) && (GET_CLASS(tmp_ch) == CLASS_MAGIC_USER)) ||
              ((tmp_aggro_table[j] == ACT_AGGCL) && (GET_CLASS(tmp_ch) == CLASS_CLERIC)) ||
              ((tmp_aggro_table[j] == ACT_AGGTH) && (GET_CLASS(tmp_ch) == CLASS_THIEF)) ||
              ((tmp_aggro_table[j] == ACT_AGGWA) && (GET_CLASS(tmp_ch) == CLASS_WARRIOR)) ||
              ((tmp_aggro_table[j] == ACT_AGGNI) && (GET_CLASS(tmp_ch) == CLASS_NINJA)) ||
              ((tmp_aggro_table[j] == ACT_AGGNO) && (GET_CLASS(tmp_ch) == CLASS_NOMAD)) ||
              ((tmp_aggro_table[j] == ACT_AGGPA) && (GET_CLASS(tmp_ch) == CLASS_PALADIN)) ||
              ((tmp_aggro_table[j] == ACT_AGGAP) && (GET_CLASS(tmp_ch) == CLASS_ANTI_PALADIN)) ||
              ((tmp_aggro_table[j] == ACT_AGGBA) && (GET_CLASS(tmp_ch) == CLASS_BARD)) ||
              ((tmp_aggro_table[j] == ACT_AGGCO) && (GET_CLASS(tmp_ch) == CLASS_COMMANDO)) ||
              ((tmp_aggro_table[j] == ACT_AGGEVIL) && (IS_EVIL(tmp_ch))) ||
              ((tmp_aggro_table[j] == ACT_AGGGOOD) && (IS_GOOD(tmp_ch))) ||
              ((tmp_aggro_table[j] == ACT_AGGNEUT) && (IS_NEUTRAL(tmp_ch)))) {
            aggro_target = tmp_ch;
          }
        }
      }
    }

    if (!aggro_target && (IS_SET(GET_ACT(mob), ACT_AGGRESSIVE) || IS_SET(GET_ACT(mob), ACT_AGGLEADER))) {
      for (CHAR *tmp_ch = world[CHAR_REAL_ROOM(mob)].people; tmp_ch && !aggro_target; tmp_ch = tmp_ch->next_in_room) {
        if (!check_aggro_target(mob, tmp_ch)) continue;

        aggro_target = tmp_ch;

        if (IS_SET(GET_ACT(mob), ACT_AGGLEADER) && GET_MASTER(aggro_target) && check_aggro_target(mob, GET_MASTER(aggro_target))) {
          aggro_target = GET_MASTER(aggro_target);
        }
      }
    }
  }

  /* Memory */
  if (!aggro_target && IS_SET(GET_ACT(mob), ACT_MEMORY)) {
    for (CHAR *tmp_ch = world[CHAR_REAL_ROOM(mob)].people; tmp_ch && !aggro_target; tmp_ch = tmp_ch->next_in_room) {
      for (MEMtMemoryRec *memory = mob->specials.memory; memory && !aggro_target; memory = memory->next) {
        if ((GET_ID(tmp_ch) != memory->id) || !check_aggro_target(mob, tmp_ch)) continue;

        aggro_target = tmp_ch;

        act("$n screams, 'I remember the likes of you!'", TRUE, mob, 0, 0, TO_ROOM);
      }
    }
  }

  /* If a target was found, attack it. */
  if (aggro_target) {
    if (GET_POS(mob) < POSITION_STANDING) {
      do_stand(mob, "\0", CMD_STAND);
    }

    /* Awareness */
    for (CHAR *tmp_ch = world[CHAR_REAL_ROOM(mob)].people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
      if (IS_MORTAL(tmp_ch) &&
          affected_by_spell(tmp_ch, SKILL_AWARENESS) &&
          !GET_OPPONENT(tmp_ch) &&
          !IS_AFFECTED(tmp_ch, AFF_FURY) &&
          !affected_by_spell(tmp_ch, SKILL_BERSERK) &&
          !affected_by_spell(tmp_ch, SKILL_FRENZY)) {
        act("You notice $N's attack and hit first!", 0, tmp_ch, 0, mob, TO_CHAR);
        act("$n notices your attack and hits first!", 0, tmp_ch, 0, mob, TO_VICT);
        act("$n notices $N's attack and hits first!", 0, tmp_ch, 0, mob, TO_NOTVICT);

        hit(tmp_ch, mob, TYPE_UNDEFINED);

        return;
      }
    }

    do_kill(mob, GET_NAME(aggro_target), CMD_KILL);
  }
}
