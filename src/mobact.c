/* ************************************************************************
*  file: mobact.c , Mobile action module.                 Part of DIKUMUD *
*  Usage: Procedures generating 'intelligent' behavior in the mobiles.    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "utility.h"
#include "utils.h"
#include "db.h"
#include "constants.h"
#include "comm.h"
#include "handler.h"

#include "act.h"
#include "cmd.h"
#include "fight.h"
#include "spells.h"
#include "subclass.h"

extern CHAR *combat_list;
extern CHAR *combat_next_dude;

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

bool mob_disarm(CHAR *mob, CHAR *victim, bool to_ground) {
  const int WEAPON_WYVERN_SPUR = 11523;

  if (!mob || !victim || (mob == victim) || !IS_NPC(mob) || !IS_MORTAL(victim)) return FALSE;

  OBJ *weapon = EQ(victim, WIELD);

  if (!weapon ||
      (V_OBJ(weapon) == WEAPON_WYVERN_SPUR) ||
      (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(mob, victim, BT_INVUL))) return FALSE;

  /* Tactician */
  if (IS_MORTAL(victim) &&
      check_subclass(victim, SC_GLADIATOR, 3) &&
      SAME_ROOM(mob, victim) &&
      (GET_POS(victim) >= POSITION_FIGHTING)) {
    if (chance(20 + GET_DEX_APP(victim))) {
      act("$N avoids $n's disarm and attacks back!", FALSE, mob, 0, victim, TO_NOTVICT);
      act("$N avoids your disarm and attacks back!", FALSE, mob, 0, victim, TO_CHAR);
      act("You avoid $n's disarm and attack back!", FALSE, mob, 0, victim, TO_VICT);

      hit(victim, mob, TYPE_UNDEFINED);
    }
    else {
      act("$N avoids $n's disarm attempt!", FALSE, mob, 0, victim, TO_NOTVICT);
      act("$N avoids your disarm attempt!", FALSE, mob, 0, victim, TO_CHAR);
      act("You avoid $n's disarm attempt!", FALSE, mob, 0, victim, TO_VICT);
    }

    return FALSE;
  }

  unequip_char(victim, WIELD);

  if (to_ground) {
    char buf[MSL];

    snprintf(buf, sizeof(buf), "WIZINFO: %s disarms %s's %s (Room %d)",
      GET_NAME(mob), GET_NAME(victim), OBJ_NAME(weapon), world[CHAR_REAL_ROOM(victim)].number);
    log_s(buf);

    weapon->log = 1;

    obj_to_room(weapon, CHAR_REAL_ROOM(victim));
  }
  else {
    obj_to_char(weapon, victim);
  }

  save_char(victim, NOWHERE);

  return TRUE;
}

typedef struct mob_attact_t {
  char *attack_name;

  char *failure_to_other;
  char *failure_to_char;
  char *failure_to_vict;

  char *success_to_other;
  char *success_to_char;
  char *success_to_vict;

  char *multi_to_other;
  char *multi_to_char;
  char *multi_to_vict;

  int vict_pos;
  int vict_wait_state;
  
  int mob_attack_timer;
} mob_attact_t;

const mob_attact_t mob_attack_table[] = {
  {0},
  {0},
  /* ATT_KICK */
  {
    .attack_name = "kick",

    .failure_to_other = "$n kicks $N in the solar plexus, rendering $M breathless.",
    .failure_to_char = "Your beautiful full-circle-kick swings way over $N's head.",
    .failure_to_vict = "$n kicks you in the solar plexus, rendering you breathless.",

    .success_to_other = "$n kicks $N in the solar plexus, rendering $M breathless.",
    .success_to_char = "You kick $N in the solar plexus, rendering $M breathless.",
    .success_to_vict = "$n kicks you in the solar plexus, rendering you breathless.",

    .multi_to_other = "$n's spin-kick has generated a big whirl.",
    .multi_to_char = "Your spin-kick has generated a big whirl.",
    .multi_to_vict = "You are kicked by $n.",

    .vict_wait_state = 2,

    .mob_attack_timer = 3
  },
  /* ATT_PUMMEL */
  {
    .attack_name = "pummel",

    .failure_to_other = "$n tried to pummel $N, but missed.",
    .failure_to_char = "You try to pummel $N, but miss and nearly hurt yourself.",
    .failure_to_vict = "$n tried to pummel you, but missed.",

    .success_to_other = "$n pummels $N, and $N is stunned now!",
    .success_to_char = "You pummel $N, and $N is stunned now!",
    .success_to_vict = "$n pummels you, and you are stunned now!",

    .multi_to_other = "$n spins wildly about the room, pummeling like crazy.",
    .multi_to_char = "You spin wildly about the room, pummeling like crazy.",
    .multi_to_vict = "You are pummeled by $n.",

    .vict_pos = POSITION_STUNNED,
    .vict_wait_state = 2,

    .mob_attack_timer = 2
  },
  /* ATT_PUNCH */
  {
    .attack_name = "punch",

    .failure_to_other = "$n tries to punch $N with a mighty swing, but misses.",
    .failure_to_char = "You try to punch $N with a mighty swing, but miss.",
    .failure_to_vict = "$n tries to punch you with a mighty swing, but misses.",

    .success_to_other = "$n punches $N with a blow that sends $M reeling.",
    .success_to_char = "You punch $N with a blow that sends $M reeling.",
    .success_to_vict = "$n punches you with a blow that sends you reeling.",

    .multi_to_other = "$n spins wildly about the room, punching like crazy.",
    .multi_to_char = "You spin wildly about the room, punching like crazy.",
    .multi_to_vict = "You are punched by $n.",

    .vict_pos = POSITION_SITTING,
    .vict_wait_state = 2,

    .mob_attack_timer = 2
  },
  /* ATT_BITE */
  {
    .attack_name = "bite",

    .failure_to_other = "$n bites at $N, missing $M by a breath.",
    .failure_to_char = "You bite at $N, missing $M by a breath.",
    .failure_to_vict = "$n bites at you, missing you by a breath.",

    .success_to_other = "$n takes a bite out of $N.",
    .success_to_char = "You take a bite out of $N.",
    .success_to_vict = "$n takes a bite out of you.",

    .multi_to_other = "$n snaps $s jaws wildly about, biting all around $m.",
    .multi_to_char = "You snap your jaws wildly about, biting all around you.",
    .multi_to_vict = "You are bitten by $n.",

    .vict_wait_state = 1,

    .mob_attack_timer = 2
  },
  /* ATT_CLAW */
  {
    .attack_name = "claw",

    .failure_to_other = "$n tries to shred $N to pieces, but claws the air instead.",
    .failure_to_char = "You try to shred $N to pieces, but you claw the air instead.",
    .failure_to_vict = "$n tries to shred you to pieces, but claws the air instead.",

    .success_to_other = "$n rakes $N with $s claws.",
    .success_to_char = "You rake $N with your claws.",
    .success_to_vict = "$N rakes you with $S claws.",

    .multi_to_other = "$n lashes wildly about, raking everyone with $s claws.",
    .multi_to_char = "You lash wildly about, raking everyone with your claws.",
    .multi_to_vict = "You are clawed by $N.",

    .vict_wait_state = 3,

    .mob_attack_timer = 3
  },
  /* ATT_BASH */
  {
    .attack_name = "bash",

    .failure_to_other = "$N nimbly avoids $n's bash.",
    .failure_to_char = "$N nimbly avoids your bash.",
    .failure_to_vict = "You nimbly avoid $n's bash.",

    .success_to_other = "$n's powerful bash sends $N sprawling.",
    .success_to_char = "Your powerful bash sends $N sprawling.",
    .success_to_vict = "$n's powerful bash sends you sprawling.",

    .multi_to_other = "$n spins wildly about the room, bashing like crazy.",
    .multi_to_char = "You spin wildly about the room, bashing like crazy.",
    .multi_to_vict = "You are bashed by $n.",

    .vict_pos = POSITION_SITTING,
    .vict_wait_state = 2,

    .mob_attack_timer = 2
  },
  /* ATT_TAILSLAM */
  {
    .attack_name = "tailslam",

    .failure_to_other = "$n tries to slam $N with $s tail, but slams the ground with a thump!",
    .failure_to_char = "You try to slam $N with your tail, but slam the ground with a thump!",
    .failure_to_vict = "$n tries to slam you with $s tail, but slams the ground with a thump!",

    .success_to_other = "$n slams $N with $s tail.",
    .success_to_char = "You slam $N with your tail.",
    .success_to_vict = "$n slams you with $s tail.",

    .multi_to_other = "$n's spins $s tail around.",
    .multi_to_char = "You spin your tail around.",
    .multi_to_vict = "You are tailslammed by $n.",

    .vict_pos = POSITION_STUNNED,
    .vict_wait_state = 2,

    .mob_attack_timer = 3
  },
  /* ATT_DISARM */
  {
    .attack_name = "disarm",

    .failure_to_other = "$n tried to kick off $N's weapon, but failed!",
    .failure_to_char = "You tried to kick off $N's weapon, but failed!",
    .failure_to_vict = "$n tried to kick off your weapon, but failed!",

    .success_to_other = "$n kicks off $N's weapon.",
    .success_to_char = "You kick off $N's weapon.",
    .success_to_vict = "$n kicks off your weapon.",

    .multi_to_other = "$n spins wildly about the room.",
    .multi_to_char = "You spin wildly about the room.",
    .multi_to_vict = "$n kicks off your weapon.",

    .mob_attack_timer = 2
  },
  /* ATT_TRAMPLE */
  {
    .attack_name = "trample",

    .failure_to_other = "$n stomps around $N, failing to trample $M into the ground.",
    .failure_to_char = "You stomp around $N, failing to trample $M into the ground.",
    .failure_to_vict = "$n stomps around you, failing to trample you into the ground.",

    .success_to_other = "$n jumps into the air and lands on $N.",
    .success_to_char = "You jump into the air and land on $N.",
    .success_to_vict = "$n jumps into the air and lands on you.",

    .multi_to_other = "$n jumps high in the air and lands on everyone.",
    .multi_to_char = "You jump high in the air and land on everyone.",
    .multi_to_vict = "You are trampled by $n.",

    .vict_wait_state = 2,

    .mob_attack_timer = 2
  },
  {0}
};

CHAR * mob_attack_get_victim(CHAR *mob, int attack_type, int target_type) {
  CHAR *victim = NULL;

  switch (target_type) {
    case TAR_BUFFER:
      victim = GET_OPPONENT(mob);
      break;
    case TAR_RAN_GROUP:
      victim = get_random_victim_fighting(mob);
      break;
    case TAR_RAN_ROOM:
      victim = get_random_victim(mob);
      break;
    case TAR_GROUP:
      if ((attack_type == ATT_SPELL_CAST) || (attack_type == ATT_SPELL_SKILL)) {
        victim = get_random_victim_fighting(mob);
      }
      break;
    case TAR_ROOM:
      if ((attack_type == ATT_SPELL_CAST) || (attack_type == ATT_SPELL_SKILL)) {
        victim = get_random_victim(mob);
      }
      break;
    case TAR_SELF:
      victim = mob;
      break;
    case TAR_LEADER:
      victim = GET_OPPONENT(mob);

      if (GET_MASTER(GET_OPPONENT(mob))) {
        victim = GET_MASTER(GET_OPPONENT(mob));
      }

      if (!SAME_ROOM(mob, victim)) {
        victim = get_random_victim_fighting(mob);
      }
      break;
  }

  return victim;
}

void mob_attack_spell(CHAR *mob, CHAR *victim, int spell, bool use_mana) {
  if (!mob || !IS_NPC(mob) || !victim || (spell <= 0) || (spell >= MAX_SPL_LIST)) return;

  char buf[MIL];

  snprintf(buf, sizeof(buf), "'%s' %s", spells[spell - 1], GET_NAME(victim));

  do_mob_cast(mob, buf, use_mana);
}

int mob_attack_calc_damage(CHAR *mob, CHAR *victim, int attack) {
  if (!mob) return 0;

  int dam = 0;

  switch (attack) {
    case ATT_KICK:
    case ATT_BITE:
    case ATT_CLAW:
      dam = GET_LEVEL(mob);
      break;
    case ATT_PUMMEL:
      dam = 10;
      break;
    case ATT_BASH:
      dam = number(1, GET_LEVEL(mob));
      break;
    case ATT_PUNCH:
    case ATT_TAILSLAM:
    case ATT_TRAMPLE:
      dam = GET_LEVEL(mob) * 2;
      break;
  }

  return dam;
}

void mob_attack_skill_action(CHAR *mob, CHAR *victim, int attack_type, bool multi_target) {
  if (!mob || !IS_NPC(mob) || !victim || !IS_MORTAL(victim) || (attack_type < ATT_KICK) || (attack_type > ATT_TRAMPLE)) return;

  /* Tactician */
  if (IS_MORTAL(victim) &&
      check_subclass(victim, SC_GLADIATOR, 3) &&
      SAME_ROOM(mob, victim) &&
      (GET_POS(victim) >= POSITION_FIGHTING) &&
      chance(70 + GET_DEX_APP(victim))) {
    char buf[MIL];

    snprintf(buf, sizeof(buf), "$N avoids $n's %s and attacks back!", mob_attack_table[attack_type].attack_name);
    act(buf, FALSE, mob, 0, victim, TO_NOTVICT);
    snprintf(buf, sizeof(buf), "$N avoids your %s and attacks back!", mob_attack_table[attack_type].attack_name);
    act(buf, FALSE, mob, 0, victim, TO_CHAR);
    snprintf(buf, sizeof(buf), "You avoid $n's %s and attack back!", mob_attack_table[attack_type].attack_name);
    act(buf, FALSE, mob, 0, victim, TO_VICT);

    hit(victim, mob, TYPE_UNDEFINED);

    return;
  }

  if (attack_type == ATT_DISARM) {
    if (mob_disarm(mob, victim, FALSE)) {
      act(mob_attack_table[attack_type].success_to_other, FALSE, mob, 0, victim, TO_NOTVICT);
      act(mob_attack_table[attack_type].success_to_char, FALSE, mob, 0, victim, TO_CHAR);
      act(mob_attack_table[attack_type].success_to_vict, FALSE, mob, 0, victim, TO_VICT);
    }

    return;
  }

  if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(mob, victim, BT_INVUL)) {
    if (!multi_target) {
      act(mob_attack_table[attack_type].failure_to_other, FALSE, mob, 0, victim, TO_NOTVICT);
      act(mob_attack_table[attack_type].failure_to_char, FALSE, mob, 0, victim, TO_CHAR);
    }

    act(mob_attack_table[attack_type].failure_to_vict, FALSE, mob, 0, victim, TO_VICT);

    damage(mob, victim, 0, TYPE_UNDEFINED, DAM_PHYSICAL);

    return;
  }

  if (!multi_target) {
    act(mob_attack_table[attack_type].success_to_other, FALSE, mob, 0, victim, TO_NOTVICT);
    act(mob_attack_table[attack_type].success_to_char, FALSE, mob, 0, victim, TO_CHAR);
    act(mob_attack_table[attack_type].success_to_vict, FALSE, mob, 0, victim, TO_VICT);
  }
  else {
    act(mob_attack_table[attack_type].multi_to_vict, FALSE, mob, 0, victim, TO_VICT);
  }

  damage(mob, victim, mob_attack_calc_damage(mob, victim, attack_type), TYPE_UNDEFINED, DAM_PHYSICAL);

  int vict_pos = mob_attack_table[attack_type].vict_pos;

  if (vict_pos) {
    GET_POS(victim) = vict_pos;
  }

  int vict_wait_state = mob_attack_table[attack_type].vict_wait_state;

  /* Tactician */
  if (IS_MORTAL(victim) && check_subclass(victim, SC_GLADIATOR, 3)) {
    vict_wait_state = MAX(0, vict_wait_state - 1);
  }

  WAIT_STATE(victim, PULSE_VIOLENCE * vict_wait_state);
}

void mob_attack_skill_single_target(CHAR *mob, CHAR *victim, int attack_type) {
  if (!mob || !IS_NPC(mob) || !victim || !IS_MORTAL(victim) || (attack_type < ATT_KICK) || (attack_type > ATT_TRAMPLE)) return;

  mob_attack_skill_action(mob, victim, attack_type, FALSE);

  MOB_ATTACK_TIMER(mob) = mob_attack_table[attack_type].mob_attack_timer;
}

void mob_attack_skill_multi_target(CHAR *mob, int attack_type, int target_type) {
  if (!mob || !IS_NPC(mob) || (attack_type < ATT_KICK) || (attack_type > ATT_TRAMPLE)) return;

  act(mob_attack_table[attack_type].multi_to_other, FALSE, mob, 0, 0, TO_ROOM);
  act(mob_attack_table[attack_type].multi_to_char, FALSE, mob, 0, 0, TO_CHAR);

  for (CHAR *temp_victim = world[CHAR_REAL_ROOM(mob)].people, *next_victim = NULL; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if (!temp_victim || (temp_victim == mob) || !IS_MORTAL(temp_victim) || ((target_type == TAR_GROUP) && (GET_OPPONENT(temp_victim) != mob))) continue;

    mob_attack_skill_action(mob, temp_victim, attack_type, TRUE);
  }

  MOB_ATTACK_TIMER(mob) = mob_attack_table[attack_type].mob_attack_timer;
}

void mob_attack(CHAR *mob) {
  if (!mob || !IS_NPC(mob) || !MOB_NUM_ATTACKS(mob) || !GET_OPPONENT(mob) || !SAME_ROOM(mob, GET_OPPONENT(mob)) || (GET_POS(mob) < POSITION_FIGHTING)) return;

  if (MOB_ATTACK_TIMER(mob)) {
    MOB_ATTACK_TIMER(mob)--;
    return;
  }

  for (int num = 0; (num < MOB_NUM_ATTACKS(mob)) && !MOB_ATTACK_TIMER(mob); num++) {
    if (!chance(MOB_ATTACK_CHANCE(mob, num))) continue;

    int attack_type = MOB_ATTACK_TYPE(mob, num);
    int target_type = MOB_ATTACK_TARGET(mob, num);

    if ((attack_type <= ATT_UNDEFINED) || (attack_type >= ATT_MAX) || (target_type <= TAR_UNDEFINED) || (target_type >= TAR_MAX)) continue;

    CHAR *victim = mob_attack_get_victim(mob, attack_type, target_type);

    switch (attack_type) {
      /* "Spells" */
      case ATT_SPELL_CAST:
      case ATT_SPELL_SKILL:
        if (victim) {
          mob_attack_spell(mob, victim, MOB_ATTACK_SPELL(mob, num), (ATT_SPELL_CAST ? 1 : 0));
        }
        break;
      /* "Skills" */
      case ATT_KICK:
      case ATT_PUMMEL:
      case ATT_PUNCH:
      case ATT_BITE:
      case ATT_CLAW:
      case ATT_BASH:
      case ATT_TAILSLAM:
      case ATT_DISARM:
      case ATT_TRAMPLE:
        if (victim) {
          mob_attack_skill_single_target(mob, victim, attack_type);
        }
        else if ((target_type == TAR_GROUP) || (target_type == TAR_ROOM)) {
          mob_attack_skill_multi_target(mob, attack_type, target_type);
        }
        break;
    }
  }
}

void perform_mob_attack(void) {
  for (CHAR *ch = combat_list; ch; ch = combat_next_dude) {
    combat_next_dude = ch->next_fighting;

    CHAR *victim = GET_OPPONENT(ch);

    assert(victim);

    if (IS_NPC(ch) && AWAKE(ch) && SAME_ROOM(ch, victim)) {
      mob_attack(ch);
    }
  }
}