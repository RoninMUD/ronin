#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"

#include "act.h"
#include "aff_ench.h"
#include "cmd.h"
#include "comm.h"
#include "db.h"
#include "enchant.h"
#include "interpreter.h"
#include "fight.h"
#include "handler.h"
#include "memory.h"
#include "spells.h"
#include "subclass.h"
#include "utility.h"

#define MOB_ID_HP_ARRAY_MAX    10
#define MOB_ID_DMG_ARRAY_MAX   9
#define MOB_ID_LEVEL_ARRAY_MAX 6
#define MOB_ID_MATT_ARRAY_MAX  4
#define MOB_ID_SPEC_ARRAY_MAX  2

void spell_reappear(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_INVISIBLE)) {
    REMOVE_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_INVISIBLE);

    act("$p reappears into the world.", TRUE, ch, obj, 0, TO_CHAR);
    act("$p reappears into the world.", TRUE, ch, obj, 0, TO_ROOM);
  }
}

void spell_reveal(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (IS_AFFECTED(victim, AFF_INVISIBLE)) {
    appear(victim);
  }
}

void spell_paralyze(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (IS_AFFECTED(victim, AFF_PARALYSIS) || aff_affected_by(victim, SPELL_PARALYSIS)) {
    act("$N is already paralyzed!", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if ((!IS_IMMORTAL(victim) || IS_IMMORTAL(ch)) && !IS_IMMUNE(victim, IMMUNE_PARALYSIS) && !saves_spell(victim, SAVING_PARA, level)) {
    if (GET_LEVEL(victim) - 10 <= GET_LEVEL(ch)) {
      print_spell_messages(victim, SPELL_PARALYSIS);

      int duration = ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 1 : level;

      aff_apply(victim, SPELL_PARALYSIS, duration, 100, APPLY_AC, AFF_PARALYSIS, 0);
      aff_apply(victim, SPELL_PARALYSIS, duration, -5, APPLY_HITROLL, AFF_PARALYSIS, 0);
    }
  }
}

void spell_remove_paralysis(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (aff_affected_by(victim, SPELL_PARALYSIS) || aff_affected_by(victim, SPELL_HOLD)) {
    aff_from_char(victim, SPELL_PARALYSIS);
    aff_from_char(victim, SPELL_HOLD);

    if (!IS_AFFECTED(ch, AFF_PARALYSIS) && !IS_AFFECTED(ch, AFF_HOLD)) {
      print_spell_messages(victim, SPELL_REMOVE_PARALYSIS);
    }
  }
}

void spell_magic_missile(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_MAGIC_MISSILE, level, number(5, 10) + (level / 2), DAM_MAGICAL, SAVING_SPELL);
}

void spell_hell_fire(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

  send_to_char("You have summoned the fires of hell to burn here.\n\r", ch);
  act("$n has summoned the fires of hell to burn here.", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
    if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
    if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;

    damage(ch, temp_victim, 200 + (!IS_EVIL(temp_victim) ? GET_LEVEL(ch) : 0), SPELL_HELL_FIRE, DAM_FIRE);
  }
}

void spell_death_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

  send_to_char("You throw out a spray of deadly rays of dark light.\n\r", ch);
  act("There are many dark rays shooting out from $n's hand.", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
    if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
    if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;

    damage(ch, temp_victim, 120 + (level / 2), SPELL_DEATH_SPRAY, DAM_MAGICAL);
  }
}

void spell_chill_touch(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if ((!IS_IMMORTAL(victim) || IS_IMMORTAL(ch)) && !IS_IMMUNE2(victim, IMMUNE2_COLD) && !saves_spell(victim, SAVING_SPELL, level)) {
    print_spell_messages(victim, SPELL_CHILL_TOUCH);

    AFF aff = { 0 };

    aff.type = SPELL_CHILL_TOUCH;
    aff.duration = ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 1 : 6;
    aff.modifier = -1;
    aff.location = APPLY_STR;

    aff_join(victim, &aff, TRUE, FALSE);
  }

  damage_spell(ch, victim, SPELL_CHILL_TOUCH, level, number(10, 40) + (level / 2), DAM_COLD, SAVING_NONE);
}

void spell_burning_hands(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_BURNING_HANDS, level, number(15, 50) + (level / 2), DAM_FIRE, SAVING_SPELL);
}

void spell_shocking_grasp(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_SHOCKING_GRASP, level, number(20, 80) + (level / 2), DAM_ELECTRIC, SAVING_SPELL);
}

void spell_lightning_bolt(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_LIGHTNING_BOLT, level, number(30, 120) + (level / 2), DAM_ELECTRIC, SAVING_SPELL);
}

void spell_flamestrike(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)  {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_FLAMESTRIKE, level, number(50, 120) + (level / 2), DAM_FIRE, SAVING_SPELL);
}

void spell_color_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_COLOR_SPRAY, level, number(35, 100) + (level / 2), DAM_MAGICAL, SAVING_SPELL);
}

void spell_energy_drain(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_SELF | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch) - 200, -1000);

  if ((!IS_IMMORTAL(victim) || IS_IMMORTAL(ch)) && !IS_IMMUNE(victim, IMMUNE_DRAIN) && !saves_spell(victim, SAVING_SPELL, level) &&
      (!IS_AFFECTED(victim, AFF_SPHERE) || breakthrough(ch, victim, SPELL_ENERGY_DRAIN, BT_SPHERE))) {
    int exp = number(level >> 1, level) * 500;

    gain_exp(victim, -exp);
    gain_exp(ch, exp >> 2);

    int mana = GET_MANA(victim) >> 1;

    GET_MANA(victim) -= mana;
    GET_MANA(ch) += mana >> 1;

    int move = GET_MOVE(victim) >> 1;

    GET_MOVE(victim) -= move;
    GET_MOVE(ch) = move >> 1;

    update_pos(ch);

    /* DAM_NO_BLOCK because immunity and sphere were already checked above. */
    damage(ch, victim, dice(1, 10), SPELL_ENERGY_DRAIN, DAM_NO_BLOCK);
  }
  else {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN, DAM_NO_BLOCK);
  }
}

void spell_fireball(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_FIREBALL, level, number(80, 140) + (level / 2), DAM_FIRE, SAVING_SPELL);
}

void spell_iceball(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if ((!IS_IMMORTAL(victim) || IS_IMMORTAL(ch)) && !IS_IMMUNE2(victim, IMMUNE2_COLD) && !saves_spell(victim, SAVING_SPELL, level)) {
    GET_MOVE(victim) = MAX(GET_MOVE(victim) - 30, 0);
  }

  damage_spell(ch, victim, SPELL_ICEBALL, level, number(100, 160) + (level / 2), DAM_COLD, SAVING_SPELL);
}

void spell_lethal_fire(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_LETHAL_FIRE, level, number(180, 230) + (level / 2), DAM_FIRE, SAVING_SPELL);
}

void spell_thunderball(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_THUNDERBALL, level, number(800, 1100), DAM_SOUND, SAVING_NONE);
}

void spell_electric_shock(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_ELECTRIC_SHOCK, level, number(450, 500), DAM_ELECTRIC, SAVING_NONE);
}

void spell_earthquake(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

  send_to_char("The earth trembles beneath your feet!\n\r", ch);
  act("$n makes the earth tremble and shiver...", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
    if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
    if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;

    damage(ch, temp_victim, dice(1, 8) + (IS_NPC(ch) ? 3 * level : level), SPELL_EARTHQUAKE, DAM_MAGICAL);
  }
}

void spell_dispel_evil(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (IS_GOOD(ch)) {
    victim = ch;

    send_to_char("Your dispel backfires and targets you!\n\r", ch);
    act("$n's dispel backfires and targets $m.", FALSE, ch, 0, 0, TO_ROOM);
  }

  int dmg = 0;

  if (IS_GOOD(victim)) {
    dmg = 180;

    if (IS_MORTAL(ch) && (level < GET_LEVEL(victim))) {
      dmg = dice(level, 4);
    }

    /* Templar SC1: Conviction - Triple the damage. */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_TEMPLAR, 1)) {
      dmg *= 3;
    }
  }

  damage_spell(ch, victim, SPELL_DISPEL_GOOD, level, dmg, DAM_MAGICAL, SAVING_SPELL);
}

void spell_dispel_good(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (IS_GOOD(ch)) {
    victim = ch;

    send_to_char("Your dispel backfires and targets you!\n\r", ch);
    act("$n's dispel backfires and targets $m.", FALSE, ch, 0, 0, TO_ROOM);
  }

  int dmg = 0;

  if (IS_GOOD(victim)) {
    dmg = 180;

    if (IS_MORTAL(ch) && (level < GET_LEVEL(victim))) {
      dmg = dice(level, 4);
    }

    /* Templar SC1: Conviction - Triple the damage. */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_TEMPLAR, 1)) {
      dmg *= 3;
    }
  }

  damage_spell(ch, victim, SPELL_DISPEL_GOOD, level, dmg, DAM_MAGICAL, SAVING_SPELL);
}

void spell_evil_word(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

  if (IS_EVIL(ch) && !IS_IMMORTAL(ch)) {
    spell_dispel_good(level, ch, ch, 0);
  }
  else {
    for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
      next_victim = temp_victim->next_in_room;

      if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
      if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
      if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;
      if (IS_EVIL(temp_victim)) continue;

      spell_dispel_good(level, ch, temp_victim, 0);
    }
  }
}

void spell_holy_word(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

  if (IS_GOOD(ch) && !IS_IMMORTAL(ch)) {
    spell_dispel_evil(level, ch, ch, 0);
  }
  else {
    for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
      next_victim = temp_victim->next_in_room;

      if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
      if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
      if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;
      if (IS_GOOD(temp_victim)) continue;

      spell_dispel_evil(level, ch, temp_victim, 0);
    }
  }
}

void spell_call_lightning(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (!IS_OUTSIDE(ch) || (weather_info.sky < SKY_RAINING)) {
    send_to_char("You fail to call lightning from the sky!\n\r", ch);

    return;
  }

  damage_spell(ch, victim, SPELL_CALL_LIGHTNING, level, dice(MIN(level, 30), 8), DAM_ELECTRIC, SAVING_SPELL);
}

void spell_harm(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dmg = 100 + (level / 2);

  if (GET_HIT(victim) > 0) {
    dmg = MIN(GET_HIT(victim) - dice(1, 4), dmg);
  }

  int saving_throw = SAVING_SPELL;

  /* Templar SC1: Conviction - Bypasses saving throw. */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_TEMPLAR, 1)) {
    saving_throw = SAVING_NONE;
  }

  damage_spell(ch, victim, SPELL_HARM, level, dmg, DAM_MAGICAL, saving_throw);
}

void spell_super_harm(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  int dmg = 600;

  if (GET_HIT(victim) > 0) {
    dmg = MIN(GET_HIT(victim) - dice(1, 4), dmg);
  }

  int saving_throw = SAVING_SPELL;

  /* Templar SC1: Conviction - Bypasses saving throw. */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_TEMPLAR, 1)) {
    saving_throw = SAVING_NONE;
  }

  damage_spell(ch, victim, SPELL_SUPER_HARM, level, dmg, DAM_MAGICAL, saving_throw);
}

void spell_armor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_ARMOR)) {
    print_spell_messages(victim, SPELL_ARMOR);

    aff_apply(victim, SPELL_ARMOR, 24, -10, APPLY_AC, 0, 0);
  }
}

void spell_teleport(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dt_or_hazard(CHAR * ch);

  int to_room;

  if (!spell_check_cast_ok(ch, victim, NO_CAST_OTHER_PC_NOT_IN_GROUP)) return;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_BEAM)) {
    send_to_char("You cannot teleport from here.\n\r", ch);
    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC)) {
    to_room = real_room(number(30001, 30031));
    act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    if (ch->specials.fighting)
      stop_fighting(ch);
    char_to_room(ch, to_room);
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
    do_look(ch, "", CMD_LOOK);
    return;
  }
  do {
    to_room = number(0, top_of_world);
  } while (IS_SET(world[to_room].room_flags, PRIVATE) ||
    IS_SET(world[to_room].room_flags, NO_MAGIC) ||
    (IS_SET(world[to_room].room_flags, TUNNEL) && !CHAOSMODE) ||
    IS_SET(world[to_room].room_flags, CHAOTIC) ||
    IS_SET(world[to_room].room_flags, NO_BEAM) ||
    (IS_NPC(ch) && IS_SET(world[to_room].room_flags, SAFE)) ||
    (IS_NPC(ch) && IS_SET(world[to_room].room_flags, DEATH)) ||
    (IS_NPC(ch) && IS_SET(world[to_room].room_flags, HAZARD)) ||
    (IS_NPC(ch) && IS_SET(world[to_room].room_flags, NO_MOB)) ||
    IS_SET(world[to_room].room_flags, LOCK));

  act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  if (ch->specials.fighting)
    stop_fighting(ch);
  char_to_room(ch, to_room);
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);

  do_look(ch, "", CMD_LOOK);
  dt_or_hazard(ch);
}

void spell_bless(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_BLESS)) {
    print_spell_messages(victim, SPELL_BLESS);

    int duration = MAX(GET_LEVEL(ch) / 4, 6);

    aff_apply(victim, SPELL_BLESS, duration, 1, APPLY_HITROLL, 0, 0);
    aff_apply(victim, SPELL_BLESS, duration, -1, APPLY_SAVING_SPELL, 0, 0);
  }
}

void spell_blindness(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (IS_AFFECTED(victim, AFF_BLIND) || aff_affected_by(victim, SPELL_BLINDNESS)) {
    act("$N is already blind!", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if ((!IS_IMMORTAL(victim) || IS_IMMORTAL(ch)) && !IS_IMMUNE(victim, IMMUNE_BLINDNESS) && !saves_spell(victim, SAVING_SPELL, level)) {
    print_spell_messages(victim, SPELL_BLINDNESS);

    aff_apply(victim, SPELL_BLINDNESS, 2, -4, APPLY_HITROLL, AFF_BLIND, 0);
    aff_apply(victim, SPELL_BLINDNESS, 2, 40, APPLY_AC, AFF_BLIND, 0);
  }
}

void spell_blindness_dust(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

  send_to_char("You throw a cloud of blinding dust out!\n\r", ch);
  act("$n has thrown a cloud of blinding dust out at you!", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
    if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
    if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;

    spell_blindness(level, ch, temp_victim, 0);
  }
}

void spell_clone(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  // const int max_clone_count = 8; // remove clone limit

  if (!obj) {
    send_to_char("Nothing like that exists.\n\r", ch);
    return;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_CLONE)) {
    act("$p exploded.", FALSE, ch, obj, 0, TO_CHAR);
    act("$p exploded.", FALSE, ch, obj, 0, TO_ROOM);

    extract_obj(obj);

    if (!IS_IMMORTAL(ch)) {
      send_to_char("You hurt yourself!\n\r", ch);

      damage(ch, ch, number(10, 40), TYPE_UNDEFINED, DAM_NO_BLOCK);
    }

    return;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_LIMITED) ||
      IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_RENT) ||
      (OBJ_RNUM(obj) <= 0) ||
      (OBJ_TYPE(obj) == ITEM_SCROLL) ||
      (OBJ_TYPE(obj) == ITEM_WAND) ||
      (OBJ_TYPE(obj) == ITEM_STAFF) ||
      (OBJ_TYPE(obj) == ITEM_FIREARM) ||
      (OBJ_TYPE(obj) == ITEM_MISSILE) ||
      (OBJ_TYPE(obj) == ITEM_POTION) ||
      (OBJ_TYPE(obj) == ITEM_KEY) ||
      (OBJ_TYPE(obj) == ITEM_MONEY) ||
      (OBJ_TYPE(obj) == ITEM_BULLET) ||
      (OBJ_TYPE(obj) == ITEM_RECIPE) ||
      (OBJ_TYPE(obj) == ITEM_AQ_ORDER)) {
    act("You cannot clone $p.", FALSE, ch, obj, 0, TO_CHAR);

    return;
  }

  /*
  ** Remove clone limit - Shun 20201214

  if (obj_proto_table[OBJ_RNUM(obj)].number >= max_clone_count) {
    printf_to_char(ch, "Only %d clones of this object can be created.\n\r", max_clone_count);

    return;
  }

  */

  OBJ *clone = read_object(OBJ_RNUM(obj), REAL);

  clone->name = str_dup(OBJ_NAME(obj));
  clone->short_description = str_dup(OBJ_SHORT(obj));
  clone->description = str_dup(OBJ_DESCRIPTION(obj));

  OBJ_TYPE(clone) = OBJ_TYPE(obj);
  OBJ_WEAR_FLAGS(clone) = OBJ_WEAR_FLAGS(obj);
  OBJ_EXTRA_FLAGS(clone) = OBJ_EXTRA_FLAGS(obj);
  OBJ_EXTRA_FLAGS2(clone) = OBJ_EXTRA_FLAGS2(obj);
  OBJ_BITS(clone) = OBJ_BITS(obj);
  OBJ_BITS2(clone) = OBJ_BITS2(obj);
  OBJ_WEIGHT(clone) = OBJ_WEIGHT(obj);
  OBJ_COST(clone) = 0;
  OBJ_RENT_COST(clone) = 0;

  for (int i = 0; i < MAX_OBJ_VALUE; i++) {
    OBJ_VALUE(clone, i) = OBJ_VALUE(obj, i);
  }

  for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
    OBJ_AFF_LOC(clone, i) = OBJ_AFF_LOC(obj, i);
    OBJ_AFF_MOD(clone, i) = OBJ_AFF_MOD(obj, i);
  }

  for (int i = 0; i < MAX_OBJ_OWNER_ID; i++) {
    OBJ_OWNER_ID(clone, i) = OBJ_OWNER_ID(obj, i);
  }

  SET_BIT(OBJ_EXTRA_FLAGS(clone), ITEM_CLONE | ITEM_MAGIC | ITEM_ANTI_RENT);

  act("You create an almost perfect duplicate of $p.", FALSE, ch, clone, 0, TO_CHAR);
  act("$n creates a duplicate of $p.", FALSE, ch, clone, 0, TO_ROOM);

  obj_to_room(clone, CHAR_REAL_ROOM(ch));
}

void spell_create_food(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  OBJ *food;

  CREATE(food, OBJ, 1);

  clear_object(food);

  OBJ_GET_NAME(food) = str_dup("mushroom");
  OBJ_GET_SHORT(food) = str_dup("A Magic Mushroom");
  OBJ_GET_DESCRIPTION(food) = str_dup("A really delicious looking magic mushroom lies here.");

  OBJ_TYPE(food) = ITEM_FOOD;
  OBJ_WEAR_FLAGS(food) = ITEM_TAKE;
  OBJ_VALUE(food, 0) = 5 + level;
  OBJ_WEIGHT(food) = 1;

  food->next = object_list;
  object_list = food;

  obj_to_room(food, CHAR_REAL_ROOM(ch));

  act("$p suddenly appears.", FALSE, ch, food, 0, TO_CHAR);
  act("$p suddenly appears.", FALSE, ch, food, 0, TO_ROOM);
}

void spell_create_water(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  void name_from_drinkcon(OBJ * obj, int type);
  void name_to_drinkcon(OBJ *obj, int type);

  if (OBJ_TYPE(obj) == ITEM_DRINKCON) {
    if ((OBJ_VALUE(obj, 1) != 0) && (OBJ_VALUE(obj, 2) != LIQ_WATER)) {
      name_from_drinkcon(obj, OBJ_VALUE(obj, 2));

      OBJ_VALUE(obj, 2) = LIQ_SLIME;

      name_to_drinkcon(obj, LIQ_SLIME);

      act("The liquid inside $p turns green.", FALSE, ch, obj, 0, TO_CHAR);
    }
    else {
      int water = MIN(OBJ_VALUE(obj, 0) - OBJ_VALUE(obj, 1), level * 2 * ((weather_info.sky >= SKY_RAINING) ? 2 : 1));

      if (water > 0) {
        name_from_drinkcon(obj, OBJ_VALUE(obj, 2));

        OBJ_VALUE(obj, 2) = LIQ_WATER;
        OBJ_VALUE(obj, 1) += water;

        name_to_drinkcon(obj, LIQ_WATER);

        act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
      else {
        act("$p is already full.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
  }
  else {
    act("$p is unable to hold liquid.", FALSE, ch, obj, 0, TO_CHAR);
  }
}

void spell_cure_blind(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (aff_affected_by(victim, SPELL_BLINDNESS)) {
    aff_from_char(victim, SPELL_BLINDNESS);

    if (!IS_AFFECTED(victim, AFF_BLIND)) {
      print_spell_messages(victim, SPELL_CURE_BLIND);
    }
  }
}

void spell_mana_transfer(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_MANA_TRANSFER);

  GET_MANA(victim) = MIN(GET_MANA(victim) + 30, SHRT_MAX);
}

void spell_holy_bless(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  /* Templar SC1: Conviction - Allows casting this spell on another character. */
  if ((victim != ch) && !check_subclass(ch, SC_TEMPLAR, 1) && (!IS_MORTAL(victim) || !SAME_GROUP(victim, ch))) {
    act("Your magic seems to have no effect upon $N.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  print_spell_messages(victim, SPELL_HOLY_BLESS);

  int gain = 100;

  /* Templar SC1: Conviction - Twice the alignment change for self, or half for others. */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_TEMPLAR, 1)) {
    if (victim == ch) {
      gain *= 2;
    }
    else {
      gain /= 2;
    }
  }

  GET_ALIGNMENT(victim) = MIN(GET_ALIGNMENT(victim) + gain, 1000);
}

void spell_evil_bless(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  /* Templar SC1: Conviction - Allows casting this spell on another character. */
  if ((victim != ch) && !check_subclass(ch, SC_TEMPLAR, 1) && (!IS_MORTAL(victim) || !SAME_GROUP(victim, ch))) {
    act("Your magic seems to have no effect upon $N.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  print_spell_messages(victim, SPELL_EVIL_BLESS);

  int gain = 100;

  /* Templar SC1: Conviction - Twice the alignment change for self, or half for others. */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_TEMPLAR, 1)) {
    if (victim == ch) {
      gain *= 2;
    }
    else {
      gain /= 2;
    }
  }

  GET_ALIGNMENT(victim) = MAX(GET_ALIGNMENT(victim) - gain, -1000);
}

void spell_curse(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (victim) {
    if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

    if (IS_AFFECTED(victim, AFF_CURSE) || aff_affected_by(victim, SPELL_CURSE)) {
      act("$N is already cursed!", FALSE, ch, 0, victim, TO_CHAR);

      return;
    }

    if (!saves_spell(victim, SAVING_SPELL, level)) {
      print_spell_messages(victim, SPELL_CURSE);

      int duration = 4 * 24 * 7;
      int modifier = MAX(GET_LEVEL(ch) / 10, 1);

      aff_apply(victim, SPELL_CURSE, duration, modifier, APPLY_HITROLL, AFF_CURSE, 0);
      aff_apply(victim, SPELL_CURSE, duration, modifier, APPLY_SAVING_PARA, AFF_CURSE, 0);
    }
  }
  else if (obj) {
    REMOVE_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_BLESS);

    SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_EVIL);
    SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_NO_DROP);

    /* Lower attack dice by -1. */
    if (OBJ_TYPE(obj) == ITEM_WEAPON) {
      OBJ_VALUE(obj, 2) = MAX(OBJ_VALUE(obj, 2) - 1, 0);
    }

    act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    act("$p glows red.", FALSE, ch, obj, 0, TO_ROOM);
  }
}

void spell_detect_alignment(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_DETECT_ALIGNMENT)) {
    print_spell_messages(victim, SPELL_DETECT_ALIGNMENT);

    aff_apply(victim, SPELL_DETECT_ALIGNMENT, level * 5, 0, 0, AFF_DETECT_ALIGNMENT, 0);
  }
}

void spell_detect_invisibility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_DETECT_INVISIBLE)) {
    print_spell_messages(victim, SPELL_DETECT_INVISIBLE);

    aff_apply(victim, SPELL_DETECT_INVISIBLE, level * 5, 0, 0, AFF_DETECT_INVISIBLE, 0);
  }
}

void spell_detect_magic(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_DETECT_MAGIC)) {
    print_spell_messages(victim, SPELL_DETECT_MAGIC);

    aff_apply(victim, SPELL_DETECT_MAGIC, level * 5, 0, 0, AFF_DETECT_MAGIC, 0);
  }
}

void spell_detect_poison(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_DETECT_POISON)) {
    print_spell_messages(victim, SPELL_DETECT_POISON);

    aff_apply(victim, SPELL_DETECT_POISON, level * 5, 0, 0, AFF_DETECT_POISON, 0);
  }
}

void spell_enchant_weapon(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int hitroll_aff_idx = -1, damroll_aff_idx = -1;

  for (int i = 0; (hitroll_aff_idx < 0) && (i < MAX_OBJ_AFFECT); i++) {
    if (OBJ_AFF_LOC(obj, i) == 0) {
      hitroll_aff_idx = i;
    }
  }

  for (int i = 0; (damroll_aff_idx < 0) && (i < MAX_OBJ_AFFECT); i++) {
    if (i == hitroll_aff_idx) continue;

    if (OBJ_AFF_LOC(obj, i) == 0) {
      damroll_aff_idx = i;
    }
  }

  if (!IS_WEAPON(obj) || IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_MAGIC) || (hitroll_aff_idx < 0) || (damroll_aff_idx < 0) || !level) {
    send_to_char("Nothing happens.\n\r", ch);

    return;
  }

  int hitroll = 0, damroll = 0;

  if (level < 20) {
    hitroll = 1;
    damroll = 1;
  }
  else if (level < 25) {
    hitroll = 2;
    damroll = 1;
  }
  else if (level < 30) {
    hitroll = 2;
    damroll = 2;
  }
  else if (level < 35) {
    hitroll = 3;
    damroll = 2;
  }
  else if (level < 40) {
    hitroll = 3;
    damroll = 3;
  }
  else if (level <= 50) {
    struct enchant_weapon_t {
      int chance;
      int hitroll;
      int damroll;
    };

    /* This table contains the weight (chance) and outcome for each possible
     * combination of enchants for a level 50 caster. It MUST have 3 or more
     * elements and SHOULD have an odd number of elements. The scaling performed
     * for < level 50 casters will only work for a 10 level spread! */
    struct enchant_weapon_t enchant_weapon_table[] = {
      /* C, H, D */
      {  2, 3, 3 },
      {  4, 4, 3 },
      { 19, 3, 4 },
      { 45, 4, 4 },
      { 15, 5, 4 },
      { 10, 4, 5 },
      {  5, 5, 5 }
    };

    if (NUMELEMS(enchant_weapon_table) < 3) {
      log_f("WARNING :: spell_enchant_weapon() :: enchant_weapon_table[] has too few elements (%lu)", NUMELEMS(enchant_weapon_table));

      return;
    }

    int index = -1;

    /* Scale weights for level 40-49 casters. */
    if (level < 50) {
      int tmp_sum_weights = 0;

      /* Increase (ascending) weight for lower-tier enchants. */
      for (int i = 0; i < NUMELEMS(enchant_weapon_table) / 2; i++) {
        enchant_weapon_table[i].chance *= (1 + ((50 - level) * 0.2));
        tmp_sum_weights += enchant_weapon_table[i].chance;
      }

      /* Decrease (descending) weight for higher-tier enchants. */
      for (int i = (NUMELEMS(enchant_weapon_table) / 2) + 1; i < NUMELEMS(enchant_weapon_table); i++) {
        enchant_weapon_table[i].chance /= (1 + ((50 - level) * 0.2));
        tmp_sum_weights += enchant_weapon_table[i].chance;
      }

      /* Sanity check. */
      if ((100 - tmp_sum_weights < 0) || (tmp_sum_weights >= 100)) {
        log_f("WARNING :: spell_enchant_weapon() :: tmp_sum_weights is out of range (%d)", tmp_sum_weights);

        return;
      }

      /* Set the weight of middle index of the table to the remainder. */
      enchant_weapon_table[NUMELEMS(enchant_weapon_table) / 2].chance = 100 - tmp_sum_weights;
    }

    /* Sum the weights in the table. */
    int sum_weights = 0;
    for (int i = 0; i < NUMELEMS(enchant_weapon_table); i++) {
      sum_weights += enchant_weapon_table[i].chance;
    }

    /* Find the index of the weighted result in the table. */
    int rnd = number(0, sum_weights - 1);
    for (int i = 0; (index < 0) && (i < NUMELEMS(enchant_weapon_table)); i++) {
      if (rnd < enchant_weapon_table[i].chance) {
        index = i;
      }
      else {
        rnd -= enchant_weapon_table[i].chance;
      }
    }

    /* Sanity check. */
    if ((index < 0) || (index >= NUMELEMS(enchant_weapon_table))) {
      log_f("WARNING :: spell_enchant_weapon() :: index of enchant_weapon_table is out of range (%d)", index);

      return;
    }

    hitroll = enchant_weapon_table[index].hitroll;
    damroll = enchant_weapon_table[index].damroll;
  }
  else if (level < LEVEL_SUP) {
    hitroll = 4;
    damroll = 4;
  }
  else {
    hitroll = 5;
    damroll = 5;
  }

  OBJ_AFF_LOC(obj, hitroll_aff_idx) = APPLY_HITROLL;
  OBJ_AFF_MOD(obj, hitroll_aff_idx) = hitroll;

  OBJ_AFF_LOC(obj, damroll_aff_idx) = APPLY_DAMROLL;
  OBJ_AFF_MOD(obj, damroll_aff_idx) = damroll;

  SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_MAGIC);

  if (IS_GOOD(ch)) {
    act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);

    SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_EVIL | ITEM_ANTI_NEUTRAL);
  }
  else if (IS_EVIL(ch)) {
    act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);

    SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_GOOD | ITEM_ANTI_NEUTRAL);
  }
  else {
    act("$p glows orange.", FALSE, ch, obj, 0, TO_CHAR);

    SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);
  }
}

void spell_endure(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_ENDURE)) {
    print_spell_messages(victim, SPELL_ENDURE);

    aff_apply(victim, SPELL_ENDURE, 24, -15, APPLY_AC, 0, 0);
  }
}

void spell_fear(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if ((GET_CLASS(victim) == CLASS_LESSER_UNDEAD) || (GET_CLASS(victim) == CLASS_GREATER_UNDEAD) || aff_affected_by(victim, SPELL_TRANQUILITY) || ench_enchanted_by(victim, ENCH_NAME_DRAGON_FORM, 0)) {
    act("$N doesn't seem to notice your attempt to frighten $M.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n tries to frighten you, but you're not afraid.", FALSE, ch, 0, victim, TO_VICT);

    return;
  }

  if ((!IS_IMMORTAL(victim) || IS_IMMORTAL(ch)) && !IS_IMMUNE2(victim, IMMUNE2_FEAR) && !saves_spell(victim, SAVING_PARA, level)) {
    print_spell_messages(victim, SPELL_FEAR);

    do_flee(victim, "", 0);
  }
}

void spell_forget(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (IS_NPC(victim)) {
    if (!saves_spell(victim, SAVING_SPELL, level)) {
      print_spell_messages(victim, SPELL_FORGET);

      clearMemory(victim);
    }
  }
  else {
    send_to_char("Nothing happens.\n\r", ch);
  }
}

void spell_fly(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_FLY)) {
    print_spell_messages(victim, SPELL_FLY);

    aff_apply(victim, SPELL_FLY, 12, 0, 0, AFF_FLY, 0);

    update_pos(victim);
  }
}

void spell_fury(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_FURY)) {
    print_spell_messages(victim, SPELL_FURY);

    aff_apply(victim, SPELL_FURY, (level >= 50) ? 4 : 2, 0, 0, AFF_FURY, 0);
  }
}

void spell_mana_heal(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_MANA_HEAL);

  GET_MANA(victim) = MIN(GET_MAX_MANA(victim) + 100, GET_MAX_MANA(victim));
}

void spell_hold(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (IS_AFFECTED(victim, AFF_HOLD)) {
    act("$N is already rooted to the ground.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if (!saves_spell(victim, SAVING_PARA, level)) {
    print_spell_messages(victim, SPELL_HOLD);

    aff_apply(victim, SPELL_HOLD, ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 1 : GET_LEVEL(ch), 0, 0, AFF_HOLD, 0);
  }
}

void spell_invisibility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (victim) {
    if (!aff_affected_by(victim, SPELL_INVISIBLE)) {
      print_spell_messages(victim, SPELL_INVISIBLE);

      aff_apply(victim, SPELL_INVISIBLE, 24, -40, APPLY_AC, AFF_INVISIBLE, 0);
    }
  }
  else if (obj) {
    if (!IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_INVISIBLE)) {
      act("$p turns invisible.", FALSE, ch, obj, 0, TO_CHAR);
      act("$p turns invisible.", TRUE, ch, obj, 0, TO_ROOM);

      SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_INVISIBLE);
    }
  }
}

void spell_improved_invisibility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_IMP_INVISIBLE)) {
    print_spell_messages(victim, SPELL_IMP_INVISIBLE);

    aff_apply(victim, SPELL_IMP_INVISIBLE, level / 10, -40, APPLY_AC, AFF_IMINV, 0);
  }
}

void spell_remove_improved_invis(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (aff_affected_by(victim, SPELL_INVISIBLE) || aff_affected_by(victim, SPELL_IMP_INVISIBLE)) {
    aff_from_char(victim, SPELL_INVISIBLE);
    aff_from_char(victim, SPELL_IMP_INVISIBLE);

    if (!IS_AFFECTED(victim, AFF_INVISIBLE) && !IS_AFFECTED(victim, AFF_IMINV)) {
      print_spell_messages(victim, SPELL_REMOVE_IMP_INVIS);
    }
  }
}

void spell_locate_object(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  bool auction_obj_is_in_auction(OBJ *obj);

  int count = 0;

  if (obj) {
    char name[MIL];

    one_argument((char *)obj, name);

    if (IS_SET(GET_PFLAG(ch), PLR_QUEST)) {
      wizlog_f(LEVEL_IMM, 7, "QSTINFO: %s casts 'locate object' %s", GET_DISP_NAME(ch), name);
    }

    for (OBJ *temp_obj = object_list, *next_obj; temp_obj; temp_obj = next_obj) {
      next_obj = temp_obj->next;

      if (isname(name, OBJ_NAME(temp_obj))) {
        if (IS_SET(OBJ_EXTRA_FLAGS2(temp_obj), ITEM_NO_LOCATE) || (OBJ_TYPE(temp_obj) == ITEM_SC_TOKEN)) continue;
        if (!IS_IMMORTAL(ch) && (OBJ_IN_ROOM(temp_obj) != NOWHERE) && ROOM_ZONE(OBJ_IN_ROOM(temp_obj)) == 12) continue;

        if (OBJ_CARRIED_BY(temp_obj)) {
          if (IS_IMMORTAL(OBJ_CARRIED_BY(temp_obj)) && (GET_LEVEL(ch) < GET_LEVEL(OBJ_CARRIED_BY(temp_obj)))) continue;
          if (!strlen(PERS(OBJ_CARRIED_BY(temp_obj), ch))) continue;

          printf_to_char(ch, "%s carried by %s.\n\r", OBJ_SHORT(temp_obj), PERS(OBJ_CARRIED_BY(temp_obj), ch));
        }
        else if (OBJ_EQUIPPED_BY(temp_obj)) {
          if (IS_IMMORTAL(OBJ_EQUIPPED_BY(temp_obj)) && (GET_LEVEL(ch) < GET_LEVEL(OBJ_EQUIPPED_BY(temp_obj)))) continue;
          if (!strlen(PERS(OBJ_EQUIPPED_BY(temp_obj), ch))) continue;

          printf_to_char(ch, "%s equipped by %s.\n\r", OBJ_SHORT(temp_obj), PERS(OBJ_EQUIPPED_BY(temp_obj), ch));
        }
        else if (OBJ_IN_OBJ(temp_obj)) {
          if ((OBJ_IN_ROOM(OBJ_IN_OBJ(temp_obj)) == ROOM_LOST_TREASURE_TROVE) || (OBJ_IN_ROOM(OBJ_IN_OBJ(temp_obj)) == ROOM_QUESTERS_FORUM)) continue;

          printf_to_char(ch, "%s in %s.\n\r", OBJ_SHORT(temp_obj), OBJ_SHORT(OBJ_IN_OBJ(temp_obj)));
        }
        else {
          int in_room = OBJ_IN_ROOM(temp_obj);

          if (auction_obj_is_in_auction(temp_obj)) {
            in_room = real_room(ROOM_AUCTION_HALL);
          }

          printf_to_char(ch, "%s in %s.\n\r", OBJ_SHORT(temp_obj), (in_room != NOWHERE) ? ROOM_NAME(in_room) : "an unknown location");
        }

        count++;
      }
    }
  }

  if (!count) {
    send_to_char("That object does not exist.\n\r", ch);
  }
}

void spell_locate_character(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int count = 0;

  if (victim) {
    if (IS_SET(GET_PFLAG(ch), PLR_QUEST)) {
      wizlog_f(LEVEL_IMM, 7, "QSTINFO: %s casts 'locate character' %s", GET_DISP_NAME(ch), GET_DISP_NAME(victim));
    }

    if ((!IS_IMMORTAL(victim) || (GET_LEVEL(ch) >= GET_LEVEL(victim))) && !IS_IMMUNE2(victim, IMMUNE2_LOCATE) && !(CHAOSMODE && IS_MORTAL(ch) && IS_AFFECTED(victim, AFF_IMINV))) {
      printf_to_char(ch, "%s in %s\n\r", GET_DISP_NAME(victim), ROOM_NAME(CHAR_REAL_ROOM(victim)));

      count++;
    }
  }

  if (!count) {
    send_to_char("That character does not exist.\n\r", ch);
  }
}

void spell_poison(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (victim) {
    if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

    if ((!IS_IMMORTAL(victim) || IS_IMMORTAL(ch)) && !IS_IMMUNE(victim, IMMUNE_POISON) && !saves_spell(victim, SAVING_PARA, level)) {
      print_spell_messages(victim, SPELL_POISON);

      aff_apply(victim, SPELL_POISON, ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 1 : level * 2, -3, APPLY_STR, AFF_POISON, 0);
    }
  }
  else if (obj) {
    if (((OBJ_TYPE(obj) == ITEM_DRINKCON) || (OBJ_TYPE(obj) == ITEM_FOOD)) && (OBJ_VALUE(obj, 3) != 1)) {
      OBJ_VALUE(obj, 3) = 1;

      act("$p turns green!", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}

void spell_poison_smoke(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

  send_to_char("You blow a lot of poisonous smoke out!\n\r", ch);
  act("$n has blown a lot of poisonous smoke out!", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
    if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
    if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;

    spell_poison(level, ch, temp_victim, 0);
  }
}

void spell_protection_from_evil(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_PROTECT_FROM_EVIL)) {
    print_spell_messages(victim, SPELL_PROTECT_FROM_EVIL);

    aff_apply(victim, SPELL_PROTECT_FROM_EVIL, 12, 0, 0, AFF_PROTECT_EVIL, 0);
  }
}

void spell_protection_from_good(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_PROTECT_FROM_GOOD)) {
    print_spell_messages(victim, SPELL_PROTECT_FROM_GOOD);

    aff_apply(victim, SPELL_PROTECT_FROM_GOOD, 12, 0, 0, AFF_PROTECT_GOOD, 0);
  }
}

void spell_recharge(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if ((OBJ_TYPE(obj) == ITEM_STAFF) || (OBJ_TYPE(obj) == ITEM_WAND)) {
    int check = 50;

    if (GET_CLASS(ch) == CLASS_MAGIC_USER) {
      check += 10;
    }

    if (IS_IMMORTAL(ch) || chance(check)) {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_ROOM);

      OBJ_VALUE(obj, 2) = OBJ_VALUE(obj, 1);
    }
    else {
      act("$p exploded.", FALSE, ch, obj, 0, TO_CHAR);
      act("$p exploded.", FALSE, ch, obj, 0, TO_ROOM);

      extract_obj(obj);

      if (!IS_IMMORTAL(ch)) {
        send_to_char("You hurt yourself.\n\r", ch);

        GET_HIT(ch) = MAX(GET_HIT(ch) - 30, 1);

        update_pos(ch);
      }
    }
  }
}

void spell_remove_curse(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (victim) {
    if (aff_affected_by(victim, SPELL_CURSE)) {
      aff_from_char(victim, SPELL_CURSE);

      if (!IS_AFFECTED(victim, AFF_CURSE)) {
        print_spell_messages(victim, SPELL_REMOVE_CURSE);
      }
    }
  }
  else if (obj) {
    if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_EVIL) || IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_NO_DROP)) {
      act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
      act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_ROOM);

      REMOVE_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_EVIL);
      REMOVE_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_NO_DROP);
    }
  }
}

void spell_remove_poison(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (victim) {
    if (aff_affected_by(victim, SPELL_POISON)) {
      aff_from_char(victim, SPELL_POISON);

      print_spell_messages(victim, SPELL_REMOVE_POISON);
    }
  }
  else if (obj) {
    if (((OBJ_TYPE(obj) == ITEM_DRINKCON) || (OBJ_TYPE(obj) == ITEM_FOOD)) && (OBJ_VALUE(obj, 3) == 1)) {
      OBJ_VALUE(obj, 3) = 0;

      act("$p briefly glows green.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}

void spell_sanctuary(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_SANCTUARY)) {
    print_spell_messages(victim, SPELL_SANCTUARY);

    aff_apply(victim, SPELL_SANCTUARY, level / 5, 0, 0, AFF_SANCTUARY, 0);
  }
}

void spell_satiate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_OTHER_PC_IN_CHAOTIC_ROOM)) return;

  if (IS_MORTAL(victim) && ench_enchanted_by(victim, 0, ENCHANT_GLUTTONY)) {
    send_to_char("You are repulsed by the thought of food!\n\r", victim);
    act("$n is repulsed by the thought of food!", TRUE, victim, 0, 0, TO_ROOM);

    return;
  }

  print_spell_messages(victim, SPELL_SATIATE);

  if (GET_COND(victim, FULL) >= 0) {
    GET_COND(victim, FULL) = ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 12 : 24;
  }

  if (GET_COND(victim, THIRST) >= 0) {
    GET_COND(victim, THIRST) = ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 12 : 24;
  }
}

void spell_sleep(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (IS_AFFECTED(victim, AFF_SLEEP) && (GET_POS(victim) == POSITION_SLEEPING)) {
    act("$N is already affected by magical slumber.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if ((!IS_IMMORTAL(victim) || IS_IMMORTAL(ch)) && !saves_spell(victim, SAVING_SPELL, level)) {
    AFF aff = { 0 };

    aff.type = SPELL_SLEEP;
    aff.duration = ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 1 : 4 + level;
    aff.bitvector = AFF_SLEEP;

    aff_join(victim, &aff, FALSE, FALSE);

    if (GET_POS(victim) > POSITION_SLEEPING) {
      GET_POS(victim) = POSITION_SLEEPING;
    }

    print_spell_messages(victim, SPELL_SLEEP);
  }
}

void spell_hypnotize(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM)) return;

  if (victim == ch) {
    send_to_char("This isn't going to work, you know!\n\r", ch);

    return;
  }

  if (!IS_NPC(victim)) {
    send_to_char("You can't hypnotize other players.\n\r", ch);

    return;
  }

  chance(50) ? spell_sleep(level, ch, victim, 0) : spell_charm_person(level, ch, victim, 0);
}

void spell_sphere(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_SPHERE)) {
    print_spell_messages(victim, SPELL_SPHERE);

    aff_apply(victim, SPELL_SPHERE, level / 2, 0, 0, AFF_SPHERE, 0);
  }
}

void spell_invulnerability(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_INVUL)) {
    print_spell_messages(victim, SPELL_INVUL);

    aff_apply(victim, SPELL_INVUL, level / 2, 0, 0, AFF_INVUL, 0);
  }
}

void spell_strength(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_STRENGTH);

  AFF aff = { 0 };

  aff.type = SPELL_STRENGTH;
  aff.duration = level;
  aff.modifier = 1 + (level >= 28);
  aff.location = APPLY_STR;

  aff_join(victim, &aff, TRUE, FALSE);
}

void spell_word_of_recall(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_OTHER_PC_IN_CHAOTIC_ROOM)) return;

  if (!victim) {
    send_to_char("Your attempt to recall an inanimate object succeeds at nothing.\n\r", ch);
    return;
  }

  if ((IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) || (IS_NPC(victim) && (!IS_AFFECTED(victim, AFF_CHARM) || GET_RIDER(victim)))) return;

  if (IS_MORTAL(victim) && ench_enchanted_by(victim, 0, ENCHANT_PRIDE)) {
    send_to_char("Your pride gets the better of you and you decide to stay put.\n\r", victim);
    act("$n's pride gets the better of $m and $e decides to stay put.", TRUE, victim, 0, 0, TO_ROOM);

    return;
  }

  int to_room = real_room(ROOM_TEMPLE_OF_MIDGAARD);

  if (to_room == NOWHERE) {
    send_to_char("You are completely lost.\n\r", victim);

    return;
  }

  if (GET_MOUNT(victim)) {
    act("$n disappears with $s mount.", TRUE, victim, 0, 0, TO_ROOM);
  }
  else {
    act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  }

  if (GET_OPPONENT(victim)) {
    stop_fighting(victim);
  }

  char_from_room(victim);
  char_to_room(victim, to_room);

  GET_WAS_IN_ROOM(victim) = NOWHERE;

  do_look(victim, "", CMD_LOOK);

  if (GET_MOUNT(victim)) {
    if (GET_OPPONENT(GET_MOUNT(victim))) {
      stop_fighting(GET_MOUNT(victim));
    }

    char_from_room(GET_MOUNT(victim));
    char_to_room(GET_MOUNT(victim), to_room);

    GET_WAS_IN_ROOM(GET_MOUNT(victim)) = NOWHERE;
  }

  if (!CHAOSMODE) {
    if (GET_MOUNT(victim)) {
      act("$n appears in the middle of the room with $s mount.", TRUE, victim, 0, 0, TO_ROOM);
    }
    else {
      act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
    }
  }
}

void spell_total_recall(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim != ch) && !IS_IMMORTAL(temp_victim)) {
      spell_word_of_recall(level, ch, temp_victim, 0);
    }
  }

  spell_word_of_recall(level, ch, ch, 0);
}

void spell_summon(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_CHAOSMODE)) return;

  if (victim == ch) {
    send_to_char("This isn't going to work, you know!\n\r", ch);

    return;
  }

  if (victim && IS_SET(GET_PFLAG(ch), PLR_QUEST)) {
    wizlog_f(LEVEL_IMM, 7, "QSTINFO: %s casts 'summon' %s", GET_DISP_NAME(ch), GET_DISP_NAME(victim));
  }

  int percent = 100 - (5 * (GET_LEVEL(victim) - GET_LEVEL(ch)));

  if (aff_affected_by(victim, SPELL_BLESS)) percent += 5;
  if (aff_affected_by(victim, SPELL_CURSE)) percent -= 5;

  percent = MAX(MIN(percent, 100), 0);

  if ((CHAR_REAL_ROOM(victim) == NOWHERE) ||
      IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(victim)), PRIVATE | LOCK | CHAOTIC | SAFE) ||
      IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), PRIVATE | LOCK | CHAOTIC | ARENA | NO_SUM) ||
      (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), TUNNEL) && !CHAOSMODE) ||
      (IS_IMMORTAL(victim) && (GET_LEVEL(ch) < GET_LEVEL(victim))) ||
      (IS_MORTAL(victim) && IS_SET(GET_PFLAG(victim), PLR_NOSUMMON)) ||
      (IS_MORTAL(victim) && IS_SET(GET_PFLAG(victim), PLR_KILL | PLR_THIEF) && !CHAOSMODE) ||
      (IS_NPC(victim) && IS_IMMUNE(victim, IMMUNE_SUMMON)) ||
      (IS_NPC(victim) && (GET_LEVEL(victim) > 15) && !IS_MOUNT(victim)) ||
      (IS_NPC(victim) && GET_RIDER(victim)) ||
      (IS_NPC(victim) && IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), SAFE)) ||
      (IS_NPC(victim) && saves_spell(victim, SAVING_SPELL, level)) ||
      (V_ROOM(victim) == ROOM_PRISON) ||
      ench_enchanted_by(victim, 0, ENCHANT_PRIDE) ||
      !chance(percent)) {
    act("You failed to summon $N.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  if (GET_MOUNT(victim)) {
    stop_riding(victim, GET_MOUNT(victim));
  }

  if (GET_OPPONENT(victim)) {
    stop_fighting(victim);
  }

  char_from_room(victim);
  char_to_room(victim, CHAR_REAL_ROOM(ch));

  GET_WAS_IN_ROOM(victim) = NOWHERE;

  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  do_look(victim, "", CMD_LOOK);

  GET_POS(victim) = POSITION_RESTING;
}

void spell_relocation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (IS_MORTAL(ch) && ench_enchanted_by(ch, 0, ENCHANT_PRIDE)) {
    send_to_char("Your pride gets the better of you and you decide to stay put.\n\r", ch);
    act("$n's pride gets the better of $m and $e decides to stay put.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  if (victim == ch) {
    send_to_char("This isn't going to work, you know!\n\r", ch);

    return;
  }

  if (IS_NPC(victim) ||
      (CHAR_REAL_ROOM(victim) == NOWHERE) ||
      (ROOM_ZONE(CHAR_REAL_ROOM(victim)) == 0) ||
      ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ||
      IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(victim)), LAWFUL | NO_MAGIC | PRIVATE | LOCK | CHAOTIC | NO_SUM) ||
      (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(victim)), TUNNEL) && !CHAOSMODE) ||
      (IS_IMMORTAL(victim) && (GET_LEVEL(ch) < GET_LEVEL(victim))) ||
      (V_ROOM(victim) == ROOM_PRISON)) {
    send_to_char("You failed.\n\r", ch);

    return;
  }

  act("$n disappears suddenly.", TRUE, ch, 0, 0, TO_ROOM);

  if (GET_MOUNT(ch)) {
    stop_riding(ch, GET_MOUNT(ch));
  }

  if (GET_OPPONENT(ch)) {
    stop_fighting(ch);
  }

  char_from_room(ch);
  char_to_room(ch, CHAR_REAL_ROOM(victim));

  GET_WAS_IN_ROOM(ch) = NOWHERE;

  act("You relocate to $N!", FALSE, ch, 0, victim, TO_CHAR);
  act("$n relocates to here.", TRUE, ch, 0, 0, TO_ROOM);

  do_look(ch, "", CMD_LOOK);
}

void spell_charm_person(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  const int charm_person_max_followers = 10;

  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM)) return;

  if (!IS_NPC(victim)) {
    send_to_char("You can't charm other players.\n\r", ch);

    return;
  }

  if (count_mob_followers(ch) >= charm_person_max_followers) {
    send_to_char("You can't control anymore followers.\n\r", ch);

    return;
  }

  if (circle_follow(victim, ch)) {
    send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);

    return;
  }

  if (IS_AFFECTED(victim, AFF_CHARM)) {
    act("$N has already under someone's influence!", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if ((GET_MAX_HIT(victim) > 1000) || (GET_LEVEL(victim) > MIN(level, 30)) || ((GET_CLASS(victim) == CLASS_LESSER_UNDEAD) || (GET_CLASS(victim) == CLASS_GREATER_UNDEAD))) {
    act("$N looks indifferent.", FALSE, ch, 0, victim, TO_CHAR);

    return;
  }

  if (!IS_IMMUNE(victim, IMMUNE_CHARM) && !saves_spell(victim, SAVING_SPELL, level)) {
    stop_follower(victim);
    add_follower(victim, ch);

    aff_apply(victim, SPELL_CHARM_PERSON, (24 * 18) / (GET_INT(victim) > 0 ? GET_INT(victim) : 1), 0, 0, AFF_CHARM, 0);
  }
}

void spell_sense_life(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_SENSE_LIFE)) {
    aff_apply(victim, SPELL_SENSE_LIFE, 5 * level, 0, 0, AFF_SENSE_LIFE, 0);

    print_spell_messages(victim, SKILL_AWARENESS);
  }
}

void spell_vitality(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_VITALITY);

  GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), SHRT_MAX);
}

void spell_recover_mana(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_RECOVER_MANA);

  GET_MANA(victim) = GET_MAX_MANA(victim);
}

void spell_spirit_levy(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if ((OBJ_TYPE(obj) != ITEM_CONTAINER) || (OBJ_VALUE3(obj) != 1)) {
    send_to_char("You must target a corpse.\n\r", ch);

    return;
  }

  if ((OBJ_COST(obj) == PC_CORPSE) && OBJ_CONTAINS(obj)) {
    send_to_char("The corpse has something in it.\n\r", ch);

    return;
  }

  print_spell_messages(ch, SPELL_SPIRIT_LEVY);

  int mob_level = OBJ_VALUE2(obj);

  int heal = (mob_level * 3);

  /* Defiler SC1: Desecrate */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_DEFILER, 1)) {
    heal += ((heal * number(0, 5)) / 10);
  }

  GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch) - heal, -1000);

  magic_heal(ch, ch, SPELL_SPIRIT_LEVY, heal, FALSE);

  if (IS_MORTAL(ch) && (mob_level >= (GET_LEVEL(ch) - 10))) {
    send_to_room("Some blood and gore is left behind after the ritual is complete.\n\r", CHAR_REAL_ROOM(ch));

    ROOM_BLOOD(CHAR_REAL_ROOM(ch)) = MIN(ROOM_BLOOD(CHAR_REAL_ROOM(ch)) + 1, 10);
  }

  /* Get all the stuff from the corpse and put it on the ground. */
  for (OBJ *temp_obj = OBJ_CONTAINS(obj), *next_obj; temp_obj; temp_obj = next_obj) {
    next_obj = temp_obj->next_content;

    obj_from_obj(temp_obj);
    obj_to_room(temp_obj, CHAR_REAL_ROOM(ch));
  }

  extract_obj(obj);
}

void spell_identify(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  char buf[MSL], buf2[256];
  int i;
  int iHP = 0;
  int iDMG = 0;
  int iLEVEL = 0;
  int iMATT = 0;
  int iSPEC = 0;
  int avgdmg = 0;
  struct obj_data *wielded = 0;
  bool found;
  char *decay_text[10] =
  { "like new","almost new","fairly new","slightly worn","worn",
   "fairly worn","very worn","slightly cracked","cracked",
   "about to crumble" };
  int val3, max_time;

  struct time_info_data age(CHAR * ch);

  if (obj) {
    send_to_char("You feel informed...\n\r", ch);

    sprintf(buf, "Object: '%s', Keywords: '%s'",
      (OBJ_SHORT(obj) ? OBJ_SHORT(obj) : "None"), OBJ_NAME(obj));
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "Type: ");
    //sprinttype(OBJ_TYPE(obj),item_types,buf2);
    snprint_type(buf2, sizeof(buf2), OBJ_TYPE(obj), item_types);
    strcat(buf, buf2);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "Can be worn: ");
    snprint_bits(buf2, sizeof(buf2), OBJ_WEAR_FLAGS(obj), wear_bits);
    strcat(buf, buf2);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    send_to_char("Item is: ", ch);
    //sprintbit(obj->obj_flags.extra_flags,extra_bits,buf);
    snprint_bits(buf, sizeof(buf), obj->obj_flags.extra_flags, extra_bits);
    strcat(buf, " ");
    send_to_char(buf, ch);
    //sprintbit(obj->obj_flags.extra_flags2,extra_bits2,buf);
    snprint_bits(buf, sizeof(buf), obj->obj_flags.extra_flags2, extra_bits2);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    if (obj->obj_flags.subclass_res) {
      send_to_char("Subclass restrictions: ", ch);
      //sprintbit(obj->obj_flags.subclass_res,subclass_res_bits,buf);
      snprint_bits(buf, sizeof(buf), obj->obj_flags.subclass_res, subclass_res_bits);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
    }

    if (obj->obj_flags.bitvector) {
      send_to_char("Item has been imbued with: ", ch);
      //sprintbit(obj->obj_flags.bitvector,affected_bits,buf);
      snprint_bits(buf, sizeof(buf), obj->obj_flags.bitvector, affected_bits);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
    }

    if (obj->obj_flags.bitvector2) {
      //sprintbit(obj->obj_flags.bitvector2,affected_bits2,buf);
      snprint_bits(buf, sizeof(buf), obj->obj_flags.bitvector2, affected_bits2);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
    }

    strcpy(buf, "Owners: ");
    for (i = 0; i < 8; i++) {
      if (obj->ownerid[i] > 0) {
        sprintf(buf2, "%s ", idname[obj->ownerid[i]].name);
        strcat(buf, CAP(buf2));
      }
    }
    if (strcmp(buf, "Owners: ")) {
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
    }

    sprintf(buf, "Weight: %d, Value: %d, Rent Cost: %d, Popped: %d\n\r",
      GETOBJ_WEIGHT(obj), obj->obj_flags.cost, obj->obj_flags.cost_per_day, obj->obj_flags.popped);
    send_to_char(buf, ch);

    switch (OBJ_TYPE(obj)) {
      case ITEM_RECIPE:
        if (obj->obj_flags.value[0] < 0) {
          sprintf(buf, "This recipe makes nothing!");
        }
        else if (obj->obj_flags.value[3] >= 0) {
          sprintf(buf, "This recipe is for %s\n\rIngredients:\n\r   %s\n\r   %s\n\r   %s\n\r",
            obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing",
            obj->obj_flags.value[1] >= 0 ? real_object(obj->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[1])].short_description : "something" : "nothing",
            obj->obj_flags.value[2] >= 0 ? real_object(obj->obj_flags.value[2]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[2])].short_description : "something" : "nothing",
            obj->obj_flags.value[3] >= 0 ? real_object(obj->obj_flags.value[3]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[3])].short_description : "something" : "nothing"
          );
        }
        else if (obj->obj_flags.value[2] >= 0) {
          sprintf(buf, "This recipe is for %s\n\rIngredients:\n\r   %s\n\r   %s\n\r",
            obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing",
            obj->obj_flags.value[1] >= 0 ? real_object(obj->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[1])].short_description : "something" : "nothing",
            obj->obj_flags.value[2] >= 0 ? real_object(obj->obj_flags.value[2]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[2])].short_description : "something" : "nothing"
          );
        }
        else if (obj->obj_flags.value[1] >= 0) {
          sprintf(buf, "This recipe is for %s\n\rIngredients:\n\r   %s\n\r",
            obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing",
            obj->obj_flags.value[1] >= 0 ? real_object(obj->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[1])].short_description : "something" : "nothing"
          );
        }
        else {
          sprintf(buf, "This recipe is for %s, but requires no ingredients\n\r",
            obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing"
          );
        }

        send_to_char(buf, ch);
        break;

      case ITEM_AQ_ORDER:
        sprintf(buf, "This acquisition order is for the following items:\n\r   %s\n\r   %s\n\r   %s\n\r   %s\n\rIf completed, worth %d quest points.\n\r ",
          obj->obj_flags.value[0] >= 0 ? real_object(obj->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[0])].short_description : "something" : "nothing",
          obj->obj_flags.value[1] >= 0 ? real_object(obj->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[1])].short_description : "something" : "nothing",
          obj->obj_flags.value[2] >= 0 ? real_object(obj->obj_flags.value[2]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[2])].short_description : "something" : "nothing",
          obj->obj_flags.value[3] >= 0 ? real_object(obj->obj_flags.value[3]) >= 0 ? obj_proto_table[real_object(obj->obj_flags.value[3])].short_description : "something" : "nothing",
          OBJ_SPEC(obj));
        send_to_char(buf, ch);
        break;

      case ITEM_SCROLL:
      case ITEM_POTION:
        sprintf(buf, "Level %d spells of:\n\r", obj->obj_flags.value[0]);
        send_to_char(buf, ch);

        if (obj->obj_flags.value[1] >= 1) {
          //sprinttype(obj->obj_flags.value[1]-1,spells,buf);
          snprint_type(buf, sizeof(buf), obj->obj_flags.value[1] - 1, (const char *const *const)spells);
          strcat(buf, "\n\r");
          send_to_char(buf, ch);
        }

        if (obj->obj_flags.value[2] >= 1) {
          //sprinttype(obj->obj_flags.value[2]-1,spells,buf);
          snprint_type(buf, sizeof(buf), obj->obj_flags.value[2] - 1, (const char *const *const)spells);
          strcat(buf, "\n\r");
          send_to_char(buf, ch);
        }

        if (obj->obj_flags.value[3] >= 1) {
          //sprinttype(obj->obj_flags.value[3]-1,spells,buf);
          snprint_type(buf, sizeof(buf), obj->obj_flags.value[3] - 1, (const char *const *const)spells);
          strcat(buf, "\n\r");
          send_to_char(buf, ch);
        }

        break;

      case ITEM_WAND:
      case ITEM_STAFF:
        sprintf(buf, "Has %d charges, with %d charges left.\n\r",
          obj->obj_flags.value[1],
          obj->obj_flags.value[2]);
        send_to_char(buf, ch);

        sprintf(buf, "Level %d spell of:\n\r", obj->obj_flags.value[0]);
        send_to_char(buf, ch);

        if (obj->obj_flags.value[3] >= 1) {
          //sprinttype(obj->obj_flags.value[3]-1,spells,buf);
          snprint_type(buf, sizeof(buf), obj->obj_flags.value[3] - 1, (const char *const *const)spells);
          strcat(buf, "\n\r");
          send_to_char(buf, ch);
        }
        break;

      case ITEM_WEAPON:
      case ITEM_2H_WEAPON:
        if ((obj->obj_flags.value[0] > -1) && (obj->obj_flags.value[0] <= 100))
          sprintf(buf, "Extra: '%s'\n\rDamage Dice is '%dD%d'\n\r",
            wpn_spc[obj->obj_flags.value[0]],
            obj->obj_flags.value[1],
            obj->obj_flags.value[2]);
        else if ((obj->obj_flags.value[0] > 300) && (obj->obj_flags.value[0] <= 311))
          sprintf(buf, "Extra: '%s Weapon'\n\rDamage Dice is '%dD%d'\n\r",
            pc_class_types[obj->obj_flags.value[0] - 300],
            obj->obj_flags.value[1],
            obj->obj_flags.value[2]);
        else
          sprintf(buf, "Extra: 'Unknown'\n\rDamage Dice is '%dD%d'\n\r",
            obj->obj_flags.value[1],
            obj->obj_flags.value[2]);
        send_to_char(buf, ch);
        break;

      case ITEM_TICKET:
        sprintf(buf, "This is ticket # %d for lot # %d.\n\r",
          obj->obj_flags.value[1],
          obj->obj_flags.value[0]);
        send_to_char(buf, ch);
        break;

      case ITEM_MISSILE:
        sprintf(buf, "Damage Dice is '%dD%d'\n\r",
          obj->obj_flags.value[0],
          obj->obj_flags.value[1]);
        send_to_char(buf, ch);
        break;

      case ITEM_ARMOR:
        sprintf(buf, "AC-apply: %d\n\r",
          obj->obj_flags.value[0]);
        send_to_char(buf, ch);
        break;

      case ITEM_BULLET:
        sprintf(buf, "For the gun with License Number: %d\n\r",
          obj->obj_flags.value[2]);
        send_to_char(buf, ch);
        break;

      case ITEM_FIREARM:
        sprintf(buf, "License Number: %d\n\rNumber of bullets left : %d\n\rTodam : %dD%d\n\r",
          obj->obj_flags.value[0],
          obj->obj_flags.value[1],
          obj->obj_flags.value[2],
          obj->obj_flags.value[3]);
        send_to_char(buf, ch);
        break;

      case ITEM_LOCKPICK:
        sprintf(buf, "Has %d picks, with %d picks left.\n\r",
          obj->obj_flags.value[1],
          obj->obj_flags.value[0]);
        send_to_char(buf, ch);
        break;
    }

    if (IS_SET(obj->obj_flags.extra_flags2, ITEM_ALL_DECAY) || IS_SET(obj->obj_flags.extra_flags2, ITEM_EQ_DECAY)) {
      max_time = obj->item_number >= 0 ? obj_proto_table[obj->item_number].obj_flags.timer : 0;
      if (max_time)
        val3 = 9 - 10 * obj->obj_flags.timer / max_time;
      else val3 = 9;
      if (val3 < 0) val3 = 0;
      val3 = MIN(val3, 9);
      printf_to_char(ch, "This item decays and seems %s.\n\r", decay_text[val3]);
    }

    found = FALSE;

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if (obj->affected[i].location != APPLY_NONE) {

        if (!found) {
          send_to_char("Influences you in the following:\n\r", ch);
          found = TRUE;
        }

        //sprinttype(obj->affected[i].location,apply_types,buf2);
        snprint_type(buf2, sizeof(buf2), obj->affected[i].location, apply_types);
        sprintf(buf, "    Affects : %s by %d\n\r", buf2, obj->affected[i].modifier);
        send_to_char(buf, ch);
      }
    }

    if ((obj->item_number >= 0) &&
      obj_proto_table[obj->item_number].func) {
      send_to_char("You detect that this item contains a hidden power.\n\r", ch);
    }

  }
  else if (!IS_NPC(victim)) { /* victim */

    sprintf(buf, "%d Years, %d Months, %d Days, %d Hours old.\n\r",
      age(victim).year, age(victim).month, age(victim).day,
      age(victim).hours);
    send_to_char(buf, ch);

    sprintf(buf, "Height %dcm  Weight %dpounds \n\r", GET_HEIGHT(victim),
      GET_WEIGHT(victim));
    send_to_char(buf, ch);

  }
  else { /* mob */

    wielded = EQ(victim, WIELD);

    if (wielded) {
      avgdmg = ((wielded->obj_flags.value[1] + wielded->obj_flags.value[1] * wielded->obj_flags.value[2]) / 2) + GET_DAMROLL(victim);
    }
    else {
      avgdmg = ((victim->specials.damnodice + victim->specials.damnodice * victim->specials.damsizedice) / 2) + GET_DAMROLL(victim);
    }

    if (IS_SET(victim->specials.affected_by, AFF_FURY)) avgdmg *= 2;
    while ((mob_id_hp_int[iHP] <= GET_MAX_HIT(victim)) && (iHP < MOB_ID_HP_ARRAY_MAX)) iHP++;
    while ((mob_id_dmg_int[iDMG] <= avgdmg) && (iDMG < MOB_ID_DMG_ARRAY_MAX)) iDMG++;
    while ((mob_id_level_int[iLEVEL] <= GET_LEVEL(victim)) && (iLEVEL < MOB_ID_LEVEL_ARRAY_MAX)) iLEVEL++;
    while ((mob_id_matt_int[iMATT] <= victim->specials.no_att) && (iMATT < MOB_ID_MATT_ARRAY_MAX)) iMATT++;
    if (mob_proto_table[victim->nr].func) iSPEC = 1;

    sprintf(buf, "%s appears to be a %s %s; %s is %s%s%s and %s.\n\r",
      GET_SHORT(victim),
      mob_id_dmg_text[iDMG - 1],
      npc_class_types[GET_CLASS(victim)],
      HESH(victim),
      mob_id_matt_text[iMATT - 1],
      mob_id_spec[iSPEC],
      mob_id_hp_text[iHP - 1],
      mob_id_level_text[iLEVEL - 1]);
    send_to_char(buf, ch);
  }
}

void spell_infravision(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!affected_by_spell(victim, SPELL_INFRAVISION)) {
    print_spell_messages(victim, SPELL_INFRAVISION);

    aff_apply(victim, SPELL_INFRAVISION, 30 + GET_LEVEL(ch), 0, 0, AFF_INFRAVISION, 0);
  }
}

void spell_animate_dead(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct animate_data_t {
    char *short_descr;
    char *name;
    bool floating;
  };

  struct animate_data_t animate_data[] = {
    { "The Ghast", "ghast", TRUE },
    { "The Ghost", "ghost", TRUE },
    { "The Ghoul", "ghoul", FALSE },
    { "The Phantom", "phantom", TRUE },
    { "The Skeleton", "skeleton", FALSE },
    { "The Spectre", "spectre", TRUE },
    { "The Spirit", "spirit", TRUE },
    { "The Undead", "undead", FALSE },
    { "The Vampire", "vampire", FALSE },
    { "The Wight", "wight", TRUE },
    { "The Wraith", "wraith", TRUE },
    { "The Zombie", "zombie", FALSE }
  };

  const int animate_dead_max_followers = 10;
  const int animate_vnum = 2;

  if (!IS_CORPSE(obj)) {
    send_to_char("You must target a corpse with this spell.\n\r", ch);

    return;
  }

  if (count_mob_followers(ch) >= animate_dead_max_followers) {
    send_to_char("You can't control anymore followers.\n\r", ch);

    return;
  }

  CHAR *animate = read_mobile(animate_vnum, VIRTUAL);

  if (!animate || !NUMELEMS(animate_data)) return;

  int animate_level = MAX(OBJ_VALUE(obj, 2) / 2, 1);

  int animate_idx = number(0, NUMELEMS(animate_data) - 1);

  char long_descr[MIL];

  str_mrg(long_descr, sizeof(long_descr), animate_data[animate_idx].short_descr, animate_data[animate_idx].floating ? " is floating here.\n\r" : " is standing here.\n\r", NULL);

  animate->player.long_descr = strdup(long_descr);
  animate->player.short_descr = strdup(animate_data[animate_idx].short_descr);
  animate->player.name = strdup(animate_data[animate_idx].name);

  GET_SEX(animate) = SEX_NEUTRAL;
  GET_CLASS(animate) = CLASS_LESSER_UNDEAD;
  GET_LEVEL(animate) = animate_level;
  GET_ALIGNMENT(animate) = -1000;
  GET_MAX_HIT_POINTS(animate) = 30 * animate_level;
  GET_HIT(animate) = GET_MAX_HIT(animate);
  GET_AC(animate) = 0;
  GET_EXP(animate) = 0;
  GET_GOLD(animate) = 0;
  GET_HITROLL(animate) = animate_level;
  GET_DAMROLL(animate) = number(1, 5);

  animate->specials.damnodice = animate_level;
  animate->specials.damsizedice = 2;

  if (animate_data[animate_idx].floating) {
    SET_BIT(GET_ACT(animate), ACT_FLY);
  }

  GET_DEFAULT_POSITION(animate) = animate_data[animate_idx].floating ? POSITION_FLYING : POSITION_STANDING;
  GET_POS(animate) = animate_data[animate_idx].floating ? POSITION_FLYING : POSITION_STANDING;

  for (int i = 0; i <= SAVING_SPELL; i++) {
    GET_SAVING_THROW(animate, i) = MAX(30 - GET_LEVEL(animate), 2);
  }

  SET_BIT(GET_AFF(animate), AFF_ANIMATE);

  char_to_room(animate, CHAR_REAL_ROOM(ch));

  act("$n animates the corpse.", FALSE, ch, 0, 0, TO_ROOM);
  act("You animate the corpse.", FALSE, ch, 0, 0, TO_CHAR);

  add_follower(animate, ch);

  aff_apply(animate, SPELL_CHARM_PERSON, level, 0, 0, AFF_CHARM, 0);

  for (OBJ *temp_obj = OBJ_CONTAINS(obj), *next_obj; temp_obj; temp_obj = next_obj) {
    next_obj = temp_obj->next_content;

    obj_from_obj(temp_obj);
    obj_to_char(temp_obj, animate);
  }

  extract_obj(obj);
}

void spell_power_word_kill(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  level = MIN(level, 50);

  if (GET_LEVEL(victim) <= level / 2) {
    damage(ch, victim, 30000, SPELL_PW_KILL, DAM_MAGICAL);
  }
  else if (GET_LEVEL(victim) < level - 10) {
    int dmg = 500;

    int fraction = MIN(level - GET_LEVEL(victim) - 10, 15);

    switch (fraction) {
      case 15:
      case 14:
        dmg = MIN(GET_MAX_HIT(victim) / 4, 2150);
        break;

      case 13:
      case 12:
      case 11:
        dmg = MIN(GET_MAX_HIT(victim) / 8, 1900);
        break;

      case 10:
      case 9:
      case 8:
        dmg = MIN(GET_MAX_HIT(victim) / 12, 1550);
        break;

      case 7:
      case 6:
      case 5:
        dmg = MIN(GET_MAX_HIT(victim) / 16, 1100);
        break;

      case 4:
      case 3:
      case 2:
        dmg = MIN(GET_MAX_HIT(victim) / 20, 750);
        break;

      case 1:
      case 0:
        dmg = MIN(GET_MAX_HIT(victim) / 24, 500);
        break;
    }

    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
      dmg *= 2;
    }

    damage(ch, victim, dmg, SPELL_PW_KILL, DAM_MAGICAL);
  }
  else {
    damage(ch, victim, 0, SPELL_PW_KILL, DAM_NO_BLOCK);
  }
}

void spell_vampiric_touch(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_SELF | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  int align_drain = 50;

  if (GET_ALIGNMENT(ch) > 0) {
    align_drain += ((50 * ((double)GET_ALIGNMENT(ch) / 1000)) * ((double)GET_ALIGNMENT(victim) / 1000));
  }

  GET_ALIGNMENT(ch) = MAX((GET_ALIGNMENT(ch) - align_drain), -1000);

  int dmg = MIN(GET_LEVEL(ch), LEVEL_MORT) * (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 5 : 9);

  if (affected_by_spell(ch, SPELL_DESECRATE)) {
    dmg *= 1.1;
  }

  int heal = damage_spell(ch, victim, SPELL_VAMPIRIC, level, dmg, DAM_MAGICAL, SAVING_NONE);

  if (heal > 0) {
    print_spell_messages(victim, SPELL_VAMPIRIC);

    magic_heal(ch, ch, SPELL_VAMPIRIC, heal, TRUE);
  }
}

void spell_conflagration(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  act("You conjure a handful of flames from thin air and throw them at $N.", FALSE, ch, 0, victim, TO_CHAR);
  act("$n conjures a handful of flames from thin air and throws them at you.", FALSE, ch, 0, victim, TO_VICT);
  act("$n conjures a handful of flames from thin air and throws them at $N.", FALSE, ch, 0, victim, TO_NOTVICT);

  damage(ch, victim, 1500, TYPE_UNDEFINED, DAM_FIRE);

  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
    if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
    if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;

    act("The fires raging around $n burn you as well.", FALSE, victim, 0, temp_victim, TO_VICT);

    damage(ch, temp_victim, 200, TYPE_UNDEFINED, DAM_FIRE);
  }

  send_to_char("The raging fires burn you as well.\n\r", ch);

  damage(ch, ch, 300, TYPE_UNDEFINED, DAM_NO_BLOCK);
}

void spell_convergence(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_CHAOSMODE)) return;

  int heal = 0;

  if (ch != victim) {
    act("You draw power from $N to heal $S wounds.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n draws power from you to heal your wounds.", FALSE, ch, 0, victim, TO_VICT);
    act("$n draws power from $N to heal $S wounds.", FALSE, ch, 0, victim, TO_NOTVICT);

    heal = MAX(MIN(GET_MANA(victim), MAX(GET_MAX_HIT(victim) - GET_HIT(victim), 0)), 0);
  }
  else {
    act("You draw power from yourself to heal your wounds.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n draws power from $mself to heal $s wounds.", FALSE, ch, 0, 0, TO_ROOM);

    heal = MAX(MIN(GET_MANA(victim) - 20, MAX(GET_MAX_HIT(victim) - GET_HIT(victim), 0)), 0);
  }

  if (heal) {
    print_spell_messages(victim, SPELL_CONVERGENCE);

    GET_HIT(victim) = GET_HIT(victim) + heal;
    GET_MANA(victim) = GET_MANA(victim) - heal;

    update_pos(victim);
  }
}

void spell_enchant_armor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int ac_aff_idx = -1;

  for (int i = 0; (ac_aff_idx < 0) && (i < MAX_OBJ_AFFECT); i++) {
    if (OBJ_AFF_LOC(obj, i) == 0) {
      ac_aff_idx = i;
    }
  }

  if ((OBJ_TYPE(obj) != ITEM_ARMOR) || IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_MAGIC) || (ac_aff_idx < 0) || !level) {
    send_to_char("Nothing happens.\n\r", ch);

    return;
  }

  int ac = 0;

  if (level < 20) {
    ac = -1;
  }
  else if (level < 25) {
    ac = -2;
  }
  else if (level < 30) {
    ac = -3;
  }
  else if (level < 40) {
    ac = -4;
  }
  else {
    ac = -5;
  }

  OBJ_AFF_LOC(obj, ac_aff_idx) = APPLY_AC;
  OBJ_AFF_MOD(obj, ac_aff_idx) = ac;

  SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_MAGIC);

  if (IS_GOOD(ch)) {
    SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_EVIL | ITEM_ANTI_NEUTRAL);

    act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
  }
  else if (IS_EVIL(ch)) {
    SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_GOOD | ITEM_ANTI_NEUTRAL);

    act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
  }
  else {
    SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);

    act("$p glows orange.", FALSE, ch, obj, 0, TO_CHAR);
  }
}

void spell_dispel_magic(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (victim) {
    const int dispel_types[] = {
      SPELL_BLINDNESS,
      SPELL_CHARM_PERSON,
      SPELL_CURSE,
      SPELL_DETECT_ALIGNMENT,
      SPELL_DETECT_INVISIBLE,
      SPELL_DETECT_MAGIC,
      SPELL_DETECT_POISON,
      SPELL_FLY,
      SPELL_HOLD,
      SPELL_IMP_INVISIBLE,
      SPELL_INFRAVISION,
      SPELL_INVISIBLE,
      SPELL_PARALYSIS,
      SPELL_PROTECT_FROM_EVIL,
      SPELL_PROTECT_FROM_GOOD,
      SPELL_SENSE_LIFE,
      SPELL_SLEEP
    };

    if (ROOM_SAFE(CHAR_REAL_ROOM(ch)) && IS_NPC(victim) && (!IS_AFFECTED(victim, AFF_CHARM) || (GET_MASTER(victim) != ch))) {
      send_to_char("Behave yourself here please!\n\r", ch);

      return;
    }

    if (!IS_IMMORTAL(victim) && !saves_spell(victim, SAVING_SPELL, level)) {
      int dispel_type = get_random_set_affect(victim, dispel_types, NUMELEMS(dispel_types));

      if ((dispel_type <= 0) || !number(0, 1)) {
        send_to_char("Nothing happens.\n\r", ch);

        return;
      }

      act("You point at $N and $E briefly glows green.", TRUE, ch, 0, victim, TO_CHAR);
      act("$n points at you and you briefly glow green.", TRUE, ch, 0, victim, TO_VICT);
      act("$n points at $N and $E briefly glows green.", TRUE, ch, 0, victim, TO_ROOM);

      if (*spell_text[dispel_type].wear_off_msg) {
        printf_to_char(victim, "%s\n\r", spell_text[dispel_type].wear_off_msg);
      }

      for (AFF *temp_aff = victim->affected, *next_aff; temp_aff; temp_aff = next_aff) {
        next_aff = temp_aff->next;

        if (temp_aff->type == dispel_type) {
          aff_from_char(victim, dispel_type);
        }
      }
    }
  }
  else if (obj) {
    if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_DISPELLED) || IS_SET(OBJ_WEAR_FLAGS(obj), ITEM_QUESTWEAR) || !number(0, 1)) {
      send_to_char("Nothing happens.\n\r", ch);

      return;
    }

    act("You point at $p and it briefly glows green.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n points at $p and it briefly glows green.", FALSE, ch, obj, 0, TO_ROOM);

    switch (OBJ_TYPE(obj)) {
      case ITEM_SCROLL:
      case ITEM_WAND:
      case ITEM_STAFF:
      case ITEM_POTION:
        for (int i = 0; i < MAX_OBJ_VALUE; i++) {
          OBJ_VALUE(obj, i) = 0;
        }
        break;
    }

    for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((OBJ_AFF_LOC(obj, i) != APPLY_HITROLL) && (OBJ_AFF_LOC(obj, i) != APPLY_DAMROLL)) {
        OBJ_AFF_LOC(obj, i) = 0;
      }

      OBJ_AFF_MOD(obj, i) = 0;
    }

    REMOVE_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_MAGIC);

    SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_DISPELLED);
  }
}

void spell_disintegrate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (victim) {
    if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC)) return;

    if (IS_IMMORTAL(victim) || IS_IMMUNE(victim, IMMUNE_DISINTEGRATE)) {
      send_to_char("Nothing happens.\n\r", ch);

      return;
    }

    if (!ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && (MIN(level, 30) * MIN(level, 30) * number(1, 10) > GET_LEVEL(victim) * GET_LEVEL(victim) * number(1, 15))) {
      act("You look at $N, and $E crumbles to dust.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n looks at $N, and $E crumbles to dust.", FALSE, ch, 0, victim, TO_ROOM);

      for (int eq_pos = 0; eq_pos < MAX_WEAR; eq_pos++) {
        if (EQ(victim, eq_pos)) {
          extract_obj(unequip_char(victim, eq_pos));
        }
      }

      for (OBJ *temp_obj = victim->carrying, *next_obj; temp_obj; temp_obj = next_obj) {
        next_obj = temp_obj->next_content;

        if (OBJ_TYPE(temp_obj) == ITEM_CONTAINER) {
          for (OBJ *temp_content = temp_obj->contains, *next_content; temp_content; temp_content = next_content) {
            next_content = temp_content->next_content;

            obj_from_obj(temp_content);
            obj_to_char(temp_content, victim);
          }

          extract_obj(temp_obj);

          next_obj = victim->carrying;
        }
        else if (OBJ_TYPE(temp_obj) == ITEM_AQ_ORDER) {
          obj_from_char(temp_obj);
          obj_to_room(temp_obj, CHAR_REAL_ROOM(victim));
        }
        else {
          extract_obj(temp_obj);
        }
      }

      if (GET_QUEST_OWNER(victim)) {
        GET_QUEST_STATUS(GET_QUEST_OWNER(victim)) = QUEST_FAILED;
        GET_QUEST_GIVER(GET_QUEST_OWNER(victim)) = 0;
        GET_QUEST_MOB(GET_QUEST_OWNER(victim)) = 0;
        GET_QUEST_LEVEL(GET_QUEST_OWNER(victim)) = 0;
        GET_QUEST_TIMER(GET_QUEST_OWNER(victim)) = 2;

        printf_to_char(GET_QUEST_OWNER(victim), "Your victim has been disintegrated, you have failed your quest! You can start another in %d ticks.\n\r", GET_QUEST_TIMER(GET_QUEST_OWNER(victim)));

        GET_QUEST_OWNER(victim) = 0;
      }

      char buf[MSL];

      snprintf(buf, sizeof(buf), "%s disintegrates %s.", GET_DISP_NAME(ch), GET_DISP_NAME(victim));
      wizlog(buf, LEVEL_WIZ, 6);
      log_s(buf);

      extract_char(victim);
    }
    else {
      damage(ch, victim, number(10, 20) * level, SPELL_DISINTEGRATE, DAM_MAGICAL);
    }
  }
  else if (obj) {
    if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

    if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_NO_DISINTEGRATE) || (OBJ_TYPE(obj) == ITEM_CONTAINER) || (OBJ_TYPE(obj) == ITEM_AQ_ORDER)) {
      send_to_char("Nothing happens.\n\r", ch);

      return;
    }

    act("You look at $p, and it crumbles to dust.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n looks at $p, and it crumbles to dust.", FALSE, ch, obj, 0, TO_ROOM);

    char buf[MSL];

    snprintf(buf, sizeof(buf), "%s just destroyed %s at %s.", GET_NAME(ch), OBJ_SHORT(obj), ROOM_NAME(CHAR_REAL_ROOM(ch)));
    wizlog(buf, LEVEL_WIZ, 6);
    log_s(buf);

    extract_obj(obj);
  }
}

void spell_searing_orb(ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  if (!spell_check_cast_ok(ch, vict, NO_CAST_SAFE_ROOM)) return;

  send_to_char("You throw a flaming orb in the air, and the orb explodes...", ch);
  act("$n throws a flaming orb in the air, and the orb explodes...", FALSE, ch, 0, 0, TO_ROOM);

  for (OBJ *temp_obj = ROOM_CONTENTS(CHAR_REAL_ROOM(ch)), *next_obj; temp_obj; temp_obj = next_obj) {
    next_obj = temp_obj->next_content;

    if (((OBJ_TYPE(temp_obj) == ITEM_SCROLL) || (OBJ_TYPE(temp_obj) == ITEM_RECIPE)) && number(0, 5)) {
      act("$p burns in bright and hot flames...", FALSE, ch, temp_obj, 0, TO_ROOM);
      act("$p burns in bright and hot flames...", FALSE, ch, temp_obj, 0, TO_CHAR);

      extract_obj(temp_obj);
    }

    if ((OBJ_TYPE(temp_obj) == ITEM_POTION) && number(0, 5)) {
      act("$p boils up in steam...", FALSE, ch, temp_obj, 0, TO_ROOM);
      act("$p boils up in steam...", FALSE, ch, temp_obj, 0, TO_CHAR);

      extract_obj(temp_obj);
    }
  }

  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
    if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
    if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;

    for (OBJ *temp_obj = temp_victim->carrying, *next_obj; temp_obj; temp_obj = next_obj) {
      next_obj = temp_obj->next_content;

      if (((OBJ_TYPE(temp_obj) == ITEM_SCROLL) || (OBJ_TYPE(temp_obj) == ITEM_RECIPE)) && !number(0, 5)) {
        act("$p burns in bright and hot flames...", FALSE, ch, temp_obj, 0, TO_ROOM);
        act("$p burns in bright and hot flames...", FALSE, ch, temp_obj, 0, TO_CHAR);

        extract_obj(temp_obj);
      }

      if ((OBJ_TYPE(temp_obj) == ITEM_POTION) && !number(0, 5)) {
        act("$p boils up in steam...", FALSE, ch, temp_obj, 0, TO_ROOM);
        act("$p boils up in steam...", FALSE, ch, temp_obj, 0, TO_CHAR);

        extract_obj(temp_obj);
      }
    }

    damage(ch, temp_victim, number(100, 150), SPELL_SEARING_ORB, DAM_FIRE);
  }
}

void spell_clairvoyance(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int count = 0;

  if (victim) {
    if (victim && IS_SET(GET_PFLAG(ch), PLR_QUEST)) {
      wizlog_f(LEVEL_IMM, 7, "QSTINFO: %s casts 'clairvoyance' %s", GET_DISP_NAME(ch), GET_DISP_NAME(victim));
    }

    print_spell_messages(ch, SPELL_CLAIRVOYANCE);

    if (!IS_IMMORTAL(victim) && CAN_SEE(ch, victim)) {
      if (IS_IMMUNE(victim, IMMUNE_CLAIR)) {
        act("The world around $N is shrouded in mist...", FALSE, ch, 0, victim, TO_CHAR);
      }
      else {
        send_to_char("You see the world through a hazy mist...\n\r", ch);

        look_in_room(ch, ROOM_VNUM(CHAR_REAL_ROOM(victim)));
      }

      count++;
    }
  }

  if (!count) {
    send_to_char("Your attempt has failed!\n\r", ch);
  }
}

void spell_firebreath(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_FIREBREATH);

  ench_to_char(victim, ench_get_from_global(0, ENCHANT_FIREBREATH), FALSE);
}

void spell_regeneration(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_REGENERATION);

  ench_to_char(victim, ench_get_from_global(0, ENCHANT_REGENERATION), FALSE);
}

void spell_dispel_sanct(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (aff_affected_by(victim, SPELL_SANCTUARY)) {
    aff_from_char(victim, SPELL_SANCTUARY);
  }
}

void spell_disenchant(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  bool disenchanted = FALSE;

  for (AFF *temp_aff = victim->affected, *next_aff = NULL; temp_aff; temp_aff = next_aff) {
    next_aff = temp_aff->next;

    // skip over subsequent affs with same type, since these
    // are all removed by aff_from_char at once
    while (next_aff && next_aff->type == temp_aff->type) {
      next_aff = next_aff->next;
    }

    if (temp_aff->duration >= 0) {
      aff_from_char(victim, temp_aff->type);

      disenchanted = TRUE;
    }
  }

  if (disenchanted) {
    if (victim == ch) {
      send_to_char("You disenchant yourself.\n\r", ch);
    }
    else {
      act("You disenchant $N.", FALSE, ch, 0, victim, TO_CHAR);
      act("You have been disenchanted by $n!", FALSE, ch, 0, victim, TO_VICT);
    }
  }
  else {
    if (victim == ch) {
      send_to_char("You aren't affected by any magic.\n\r", ch);
    }
    else {
      act("$N isn't affected by any magic.", FALSE, ch, 0, victim, TO_CHAR);
    }
  }
}

void spell_petrify(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM)) return;

  if (signal_char(victim, ch, MSG_STONE, "")) return;

  if (IS_IMMORTAL(victim) && (GET_LEVEL(victim) > GET_LEVEL(ch)) && IS_SET(GET_IMM_FLAGS(victim), WIZ_ACTIVE)) {
    act("You gaze at $N and you turn to stone!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n gazes at you and $e turn to stone!", FALSE, ch, 0, victim, TO_VICT);
    act("$n gazes at $N and $e turns to stone.", FALSE, ch, 0, victim, TO_NOTVICT);

    CHAR *temp = ch;

    ch = victim;
    victim = temp;
  }
  else {
    act("You gaze at $N and $E turns to stone!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n gazes at you and you turn to stone!", FALSE, ch, 0, victim, TO_VICT);
    act("$n gazes at $N and $E turns to stone.", FALSE, ch, 0, victim, TO_NOTVICT);
  }

  if (IS_MORTAL(victim)) {
    char buf[MSL];

    snprintf(buf, sizeof(buf), "%s stoned by %s at %s (%d)", GET_DISP_NAME(victim), GET_DISP_NAME(ch), ROOM_NAME(CHAR_REAL_ROOM(ch)), ROOM_VNUM(CHAR_REAL_ROOM(ch)));

    wizlog(buf, LEVEL_IMM, 3);
    log_s(buf);
    deathlog(buf);
  }

  raw_kill_ex(victim, TRUE);
}

void spell_haste(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_HASTE)) {
    print_spell_messages(victim, SPELL_HASTE);

    aff_apply(victim, SPELL_HASTE, 5, 0, 0, 0, 0);
  }
}

void spell_great_mana(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    spell_recover_mana(level, ch, temp_victim, 0);
  }
}

void spell_perceive(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_PERCEIVE)) {
    print_spell_messages(victim, SPELL_PERCEIVE);

    aff_apply(victim, SPELL_PERCEIVE, level * 2, 0, 0, 0, AFF2_PERCEIVE);
  }
}

void spell_quick(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_QUICK)) {
    print_spell_messages(victim, SPELL_QUICK);

    aff_apply(victim, SPELL_QUICK, 5, 0, 0, 0, 0);
  }
}

void spell_divine_intervention(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_DIVINE_INTERVENTION)) {
    print_spell_messages(victim, SPELL_DIVINE_INTERVENTION);

    aff_apply(victim, SPELL_DIVINE_INTERVENTION, 8, 0, 0, 0, 0);
  }
}

void spell_rush(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_RUSH)) {
    print_spell_messages(victim, SPELL_RUSH);

    aff_apply(victim, SPELL_RUSH, 3, 5, APPLY_DAMROLL, 0, 0);
  }
}

void spell_blood_lust(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_BLOOD_LUST)) {
    print_spell_messages(victim, SPELL_BLOOD_LUST);

    aff_apply(victim, SPELL_BLOOD_LUST, 5, 0, 0, 0, 0);
  }
}

void spell_mystic_swiftness(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_MYSTIC_SWIFTNESS)) {
    print_spell_messages(victim, SPELL_MYSTIC_SWIFTNESS);

    aff_apply(victim, SPELL_MYSTIC_SWIFTNESS, 4, 0, 0, 0, 0);
  }
}

void spell_wind_slash(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM)) return;

  send_to_char("You slash the air with rapid strikes, sending blades of wind in all directions.\n\r", ch);
  act("$n slashes the air with rapid strikes, sending blades of wind in all directions.", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
    if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
    if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;

    damage(ch, temp_victim, number(140, 180) + (level / 2), SPELL_WIND_SLASH, DAM_MAGICAL);
  }
}

void spell_cure_light_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    spell_cure_light(level, ch, temp_victim, 0);
  }
}

void spell_cure_serious_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    spell_cure_serious(level, ch, temp_victim, 0);
  }
}

void spell_cure_critic_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    spell_cure_critic(level, ch, temp_victim, 0);
  }
}

void spell_heal_spray(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    spell_heal(level, ch, temp_victim, 0);
  }
}

void spell_great_miracle(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    spell_miracle(level, ch, temp_victim, 0);
  }
}

void spell_cure_light(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_CURE_LIGHT);

  magic_heal(ch, victim, SPELL_CURE_LIGHT, MIN(10 + (level * 5), 30), FALSE);
}

void spell_cure_serious(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_CURE_SERIOUS);

  magic_heal(ch, victim, SPELL_CURE_SERIOUS, MIN(10 + (level * 5), 45), FALSE);
}

void spell_cure_critic(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_CURE_CRITIC);

  magic_heal(ch, victim, SPELL_CURE_CRITIC, MIN(10 + (level * 5), 75), FALSE);
}

void spell_heal(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_HEAL);

  magic_heal(ch, victim, SPELL_HEAL, MIN(10 + (level * 5), 200), FALSE);

  spell_cure_blind(level, ch, victim, obj);

  /* Crusader SC2: Focus */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_CRUSADER, 2)) {
    GET_ALIGNMENT(ch) = MIN(10 + GET_ALIGNMENT(ch), 1000);
  }
}

void spell_lay_hands(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  const int lay_hands_dispel_types[] = {
    SPELL_BLINDNESS,
    SPELL_PARALYSIS,
    SPELL_HOLD,
    SPELL_CURSE,
    SPELL_POISON,
    SPELL_CHILL_TOUCH,
    SPELL_SLEEP
  };

  int heal = MIN(MAX(level * 10, 250), 500);

  /* Crusader SC3: Panacea */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_CRUSADER, 4)) {
    heal *= 1.5;
  }

  print_spell_messages(victim, SPELL_LAY_HANDS);

  magic_heal(ch, victim, SPELL_LAY_HANDS, heal, FALSE);

  /* Crusader SC3: Panacea */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_CRUSADER, 4)) {
    /* Cast bless on the target. */
    spell_bless(level, ch, victim, 0);

    /* Remove up to two ailments from the lay_hands_dispel_types array, in order. */
    for (int type_idx = 0, count = 0; (type_idx < NUMELEMS(lay_hands_dispel_types)) && (count < 2); type_idx++) {
      if (aff_affected_by(victim, lay_hands_dispel_types[type_idx])) {
        aff_from_char(victim, lay_hands_dispel_types[type_idx]);

        count++;
      }
    }
  }

  /* Crusader SC3: Focus */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_CRUSADER, 2)) {
    GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch) + 10, 1000);
  }
}

void spell_miracle(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (signal_char(victim, ch, MSG_MIRACLE, "")) return;

  print_spell_messages(victim, SPELL_MIRACLE);

  magic_heal(ch, victim, SPELL_MIRACLE, IS_MORTAL(ch) ? 2000 : GET_MAX_HIT(victim), FALSE);
}
