/* ************************************************************************
*  file: spells.c , handling of magic.                    Part of DIKUMUD *
*  Usage : Procedures handling all offensive magic.                       *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
**************************************************************************/

#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "structs.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "utility.h"
#include "utils.h"
#include "spells.h"
#include "subclass.h"
#include "fight.h"
#include "enchant.h"
#include "aff_ench.h"

/**
 * @brief Print spell messages.
 *
 * Most simple spells should call this function to print the canned spell
 *   messages defined in spell_parser.c in assign_spell_text().  More complex
 *   messages should be handled in the spell code itself.
 *
 * @param[in] victim The target of the spell.
 * @param[in] spell The spell number.
 */
void print_spell_messages(CHAR *victim, int spell) {
  if (!victim || !spell || (spell >= MAX_SPL_LIST)) return;

  if (strlen(spell_text[spell].to_char_msg)) {
    act(spell_text[spell].to_char_msg, FALSE, victim, 0, 0, TO_CHAR);
  }

  if (strlen(spell_text[spell].to_room_msg)) {
    act(spell_text[spell].to_room_msg, TRUE, victim, 0, 0, TO_ROOM);
  }
}

/**
 * @brief Print spell wear-off message.
 *
 * @param[in] victim The character affected by the spell.
 * @param[in] spell The spell number.
 */
void print_spell_wear_off_message(CHAR *victim, int spell) {
  if (!victim || !spell || (spell >= MAX_SPL_LIST)) return;

  if (*spell_text[spell].wear_off_msg) {
    printf_to_char(victim, "%s\n\r", spell_text[spell].wear_off_msg);
  }
}

bool spell_check_cast_ok(CHAR *ch, CHAR *victim, int spl_chk_flags) {
  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return FALSE;

  bool cast_ok = TRUE;

  if (IS_SET(spl_chk_flags, NO_CAST_SAFE_ROOM) && ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    cast_ok = FALSE;
  }
  else if (IS_SET(spl_chk_flags, NO_CAST_SELF) && (victim == ch)) {
    send_to_char("You can't cast this spell upon yourself.\n\r", ch);

    cast_ok = FALSE;
  }
  else if (IS_SET(spl_chk_flags, NO_CAST_OTHER_PC) &&
    victim && !IS_NPC(victim) && !IS_NPC(ch) && (victim != ch)) {
    send_to_char("You can't cast this spell on another player.\n\r", ch);

    cast_ok = FALSE;
  }
  else if (IS_SET(spl_chk_flags, NO_CAST_OTHER_PC_NOT_IN_GROUP) &&
    victim && !SAME_GROUP(victim, ch)) {
    send_to_char("You can't cast this spell on non-group members.\n\r", ch);

    cast_ok = FALSE;
  }
  else if (IS_SET(spl_chk_flags, NO_CAST_OTHER_PC_NOKILL_FLAG_ON) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && !ROOM_ARENA(CHAR_REAL_ROOM(ch)) &&
    victim && !IS_NPC(victim) && !IS_NPC(ch) && (victim != ch) &&
    (IS_SET(GET_PFLAG(ch), PLR_NOKILL) || !IS_SET(GET_PFLAG(victim), PLR_KILL) || !IS_SET(GET_PFLAG(victim), PLR_THIEF))) {
    send_to_char("You can't attack other players.\n\r", ch);

    cast_ok = FALSE;
  }
  else if (IS_SET(spl_chk_flags, NO_CAST_OTHER_PC_IN_ARENA_ROOM) && ROOM_ARENA(CHAR_REAL_ROOM(ch)) &&
    victim && !IS_NPC(victim) && !IS_NPC(ch) && (victim != ch)) {
    send_to_char("You can't cast this spell on another player here.\n\r", ch);

    cast_ok = FALSE;
  }
  else if (IS_SET(spl_chk_flags, NO_CAST_OTHER_PC_IN_CHAOTIC_ROOM) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) &&
    victim && !IS_NPC(victim) && !IS_NPC(ch) && (victim != ch)) {
    send_to_char("The chaos around you prevents this spell from being cast on another player.\n\r", ch);

    cast_ok = FALSE;
  }
  else if (IS_SET(spl_chk_flags, NO_CAST_CHAOSMODE) && CHAOSMODE) {
    send_to_char("The forces of Chaos absorb the magic.\n\r", ch);

    cast_ok = FALSE;
  }

  return cast_ok;
}

/**
 * @brief Generic damage spell function.
 *
 * Most simple damage spells should call this function.
 *
 * @param[in] ch The caster of the spell.
 * @param[in] victim The target of the spell.
 * @param[in] level The caster level of the spell.
 * @param[in] spell The spell number.
 * @param[in] dmg The damage to inflict.
 * @param[in] dmg_type The damage type.
 * @param[in] saving_throw The saving throw to check, if any.
 *
 * @example Use SAVING_NONE or -1 if there is no saving throw.
 *
 * @return The actual damage inflicted by the spell, or -1 if the cast was aborted.
 */
int damage_spell(CHAR *ch, CHAR *victim, int spell, int level, int dmg, int dmg_type, int saving_throw) {
  if (!ch || !victim || !spell) return -1;

  /* Sanity check for negative level. */
  level = MAX(level, 1);

  /* Sanity check for undefined dmg_type. */
  if (dmg_type <= 0) {
    dmg_type = TYPE_UNDEFINED;
  }

  /* Check saving throw, if any. Damage is reduced by 1/2 on a successful saving throw. */
  if ((dmg > 0) && (saving_throw >= SAVING_PARA) && (saving_throw <= SAVING_SPELL) && saves_spell(victim, saving_throw, level)) {
    dmg /= 2;
  }

  /* Call damage() to inflict the damage. */
  return damage(ch, victim, dmg, spell, dmg_type);
}

/**
 * @brief Cast a single-target spell as an area of effect.
 */
void aoe_spell(CHAR *ch, void (*func)(ubyte level, CHAR *ch, CHAR *victim, OBJ *tar_obj), int level, int aoe_flags) {
  if (!ch || !func || (CHAR_REAL_ROOM(ch) == NOWHERE)) return;

  if (!IS_SET(aoe_flags, AOE_TAR_OBJS_ONLY)) {
    for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
      next_victim = temp_victim->next_in_room;

      if ((temp_victim == ch) || (IS_IMMORTAL(temp_victim) && !IS_IMMORTAL(ch))) continue;

      if (!IS_SET(aoe_flags, AOE_TAR_ALL_CHARS)) {
        if (IS_SET(aoe_flags, AOE_TAR_GROUP) && !SAME_GROUP(temp_victim, ch)) continue;
        if (IS_SET(aoe_flags, AOE_TAR_PCS_ONLY) && IS_NPC(temp_victim)) continue;
        if (IS_SET(aoe_flags, AOE_TAR_NPCS_ONLY) && !IS_NPC(temp_victim)) continue;
        if (!IS_SET(aoe_flags, AOE_TAR_OTHER_PCS) && !IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;
        if (!IS_SET(aoe_flags, AOE_TAR_OTHER_NPCS) && IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
      }

      func(level, ch, temp_victim, 0);
    }

    if (IS_SET(aoe_flags, AOE_TAR_CH | AOE_TAR_ALL_CHARS)) {
      func(level, ch, ch, 0);
    }
  }

  if (IS_SET(aoe_flags, AOE_TAR_OBJS)) {
    for (OBJ *temp_obj = ROOM_CONTENTS(CHAR_REAL_ROOM(ch)), *next_obj; temp_obj; temp_obj = next_obj) {
      next_obj = temp_obj->next_content;

      func(level, ch, 0, temp_obj);
    }
  }
}

/**
 * @brief Generic magic heal function.
 *
 * Most magical healing should call this function.
 *
 * @param[in] ch The caster of the spell.
 * @param[in] victim The target of the spell.
 * @param[in] spell The spell number.
 * @param[in] heal The amount of hit points to restore.
 * @param[in] overheal Set to TRUE to allow overhealing.
 *
 * @example If ch is NULL then some checks for the caster may not apply, which
 *   could cause unexpected behavior.  Generally only spells "cast" by objects
 *   or affects that provide healing over time should specify NULL for ch.
 *
 * @return The actual healing stored by the spell.
 */
int magic_heal(CHAR *ch, CHAR *victim, int spell, int heal, bool overheal) {
  if (!victim) return 0;

  int hp_limit = GET_MAX_HIT(victim);

  if (ch) {
    /* Druid SC4: Shapeshift: Elemental Form - Magical healing caused by the Druid is increased by 20%. */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_DRUID, 4) && ench_enchanted_by(ch, ENCH_NAME_ELEMENTAL_FORM, 0)) {
      heal *= 1.2;
    }

    /* Druid SC5: Shapeshift: Dragon Form - Magical healing caused by the Druid is reduced by 20%. */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_DRUID, 5) && ench_enchanted_by(ch, ENCH_NAME_DRAGON_FORM, 0)) {
      heal *= 0.8;
    }
  }

  /* Adrenaline Rush */
  if (IS_MORTAL(victim)) {
    if (check_subclass(victim, SC_BANDIT, 3)) {
      heal += (heal * number(10, 15)) / 100;
    }

    /* Degenerate */
    ENCH *degen = ench_get_from_char(victim, 0, ENCHANT_DEGENERATE);

    if (degen && (degen->duration >= 15)) {
      send_to_char("The magic of the spell heals only a fraction your degenerated body.\n\r", victim);

      heal /= 4;
      hp_limit /= 4;
    }
  }

  /* Restore hit points to the victim. */
  if (heal > 0) {
    GET_HIT(victim) += heal;

    /* Limit healing, as appropriate. */
    if (!overheal) {
      GET_HIT(victim) = MIN(GET_HIT(victim), hp_limit);
    }

    update_pos(victim);
  }

  return heal;
}

void cast_burning_hands(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_burning_hands(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_burning_hands, level, 0);
      break;
  }
}

void cast_hell_fire(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_hell_fire(level, ch, 0, 0);
      break;
  }
}

void cast_death_spray(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_death_spray(level, ch, 0, 0);
      break;
  }
}

void cast_holy_word(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_holy_word(level, ch, 0, 0);
      break;
  }
}

void cast_evil_word(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_evil_word(level, ch, 0, 0);
      break;
  }
}

void cast_call_lightning(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_call_lightning(level, ch, victim, 0);
      break;
  }
}

void cast_chill_touch(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_chill_touch(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_chill_touch, level, 0);
      break;
  }
}

void cast_shocking_grasp(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_shocking_grasp(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_shocking_grasp, level, 0);
      break;
  }
}

void cast_color_spray(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_color_spray(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_color_spray, level, 0);
      break;
  }
}

void cast_earthquake(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_earthquake(level, ch, 0, 0);
      break;
  }
}

void cast_energy_drain(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_energy_drain(level, ch, victim, 0);
      break;
  }
}

void cast_fireball(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_fireball(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_fireball, level, 0);
      break;
  }
}

void cast_iceball(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_iceball(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_iceball, level, 0);
      break;
  }
}

void cast_harm(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_harm(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_harm, level, 0);
      break;
  }
}

void cast_super_harm(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_super_harm(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_super_harm, level, 0);
      break;
  }
}

void cast_lightning_bolt(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_lightning_bolt(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_lightning_bolt, level, 0);
      break;
  }
}

void cast_flamestrike(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_flamestrike(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_flamestrike, level, 0);
      break;
  }
}

void cast_lethal_fire(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_lethal_fire(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_lethal_fire, level, 0);
      break;
  }
}

void cast_thunderball(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_thunderball(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_thunderball, level, 0);
      break;
  }
}

void cast_electric_shock(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_electric_shock(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_electric_shock, level, 0);
      break;
  }
}

void cast_magic_missile(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_magic_missile(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_magic_missile, level, 0);
      break;
  }
}

void cast_power_word_kill(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_power_word_kill(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_power_word_kill, level, 0);
      break;
  }
}

void cast_dispel_magic(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_dispel_magic(level, ch, victim, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_dispel_magic, level, AOE_TAR_OBJS);
      break;
  }
}

void cast_convergence(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_convergence(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_convergence, level, 0);
      break;
  }
}

void cast_enchant_armor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_enchant_armor(level, ch, 0, obj);
      break;
  }
}

void cast_disintegrate(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_disintegrate(level, ch, victim, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_disintegrate, level, AOE_TAR_OBJS);
      break;
  }
}

void cast_conflagration(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_conflagration(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_conflagration, level, 0);
      break;
  }
}

void cast_vampiric_touch(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_vampiric_touch(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_vampiric_touch, level, 0);
      break;
  }
}

void cast_searing_orb(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_searing_orb(level, ch, 0, 0);
      break;
  }
}

void cast_clairvoyance(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_clairvoyance(level, ch, victim, 0);
      break;
  }
}

void cast_animate_dead(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_animate_dead(level, ch, 0, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_animate_dead, level, AOE_TAR_OBJS_ONLY);
      break;
  }
}

void cast_spirit_levy(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_spirit_levy(level, ch, 0, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_spirit_levy, level, AOE_TAR_OBJS_ONLY);
      break;
  }
}

void cast_armor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_armor(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_armor, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_endure(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_endure(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_endure, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_teleport(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_teleport(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_teleport, level, AOE_TAR_CH | AOE_TAR_GROUP);
      break;
  }
}

void cast_firebreath(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_firebreath(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_firebreath, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_bless(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_bless(level, ch, victim, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_bless, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_blindness(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_blindness(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_blindness, level, 0);
      break;
  }
}

void cast_clone(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_clone(level, ch, victim, obj);
      break;

  }
}

void cast_control_weather(ubyte level, CHAR *ch, char *arg, int type, CHAR *tar_ch, OBJ *tar_obj) {
  char buf[MIL];

  level = MIN(level, 30);

  switch (type) {
    case SPELL_TYPE_SPELL:
      one_argument(arg, buf);

      if (is_abbrev(buf, "better")) {
        send_to_room("The weather takes a turn for the better.\n\r", CHAR_REAL_ROOM(ch));

        weather_info.change += (dice(((level) / 2), 4));
      }
      else if (is_abbrev(buf, "worse")) {
        send_to_room("The weather takes a turn for the worse.\n\r", CHAR_REAL_ROOM(ch));

        weather_info.change -= (dice(((level) / 2), 4));
      }
      else {
        send_to_char("Do you want the weather to get better or worse?\n\r", ch);
      }
      break;
  }
}

void cast_create_food(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_create_food(level, ch, 0, 0);
      break;
  }
}

void cast_create_water(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_create_water(level, ch, 0, obj);
      break;
  }
}

void cast_blindness_dust(ubyte level, CHAR *ch, char *arg, int type, CHAR *tar_ch, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_blindness_dust(level, ch, 0, 0);
      break;
  }
}

void cast_poison_smoke(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_poison_smoke(level, ch, 0, 0);
      break;
  }
}

void cast_hypnotize(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_hypnotize(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_hypnotize, level, 0);
      break;
  }
}

void cast_reappear(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_reappear(level, ch, 0, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_reappear, level, AOE_TAR_OBJS_ONLY);
      break;
  }
}

void cast_reveal(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_reveal(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_reveal, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_relocation(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_relocation(level, ch, victim, 0);
      break;
  }
}

void cast_cure_blind(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_cure_blind(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_cure_blind, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_cure_critic(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_cure_critic(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_cure_critic, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_cure_light_spray(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_cure_light_spray(level, ch, 0, 0);
      break;
  }
}

void cast_great_miracle(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_great_miracle(level, ch, 0, 0);
      break;
  }
}

void cast_heal_spray(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_heal_spray(level, ch, 0, 0);
      break;
  }
}

void cast_cure_serious_spray(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_cure_serious_spray(level, ch, 0, 0);
      break;
  }
}

void cast_cure_critic_spray(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_cure_critic_spray(level, ch, 0, 0);
      break;
  }
}

void cast_cure_serious(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_cure_serious(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_cure_serious, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_fear(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_fear(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_fear, level, 0);
      break;
  }
}

void cast_forget(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_forget(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_forget, level, AOE_TAR_NPCS_ONLY);
      break;
  }
}

void cast_fly(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_fly(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_fly, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_infravision(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_infravision(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_infravision, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_vitality(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_vitality(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_vitality, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_miracle(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_miracle(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_miracle, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_fury(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_fury(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_fury, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_mana_transfer(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_mana_transfer(level, ch, victim, 0);
      break;
  }
}

void cast_holy_bless(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_holy_bless(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_holy_bless, level, AOE_TAR_CH | AOE_TAR_GROUP);
      break;
  }
}

void cast_evil_bless(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_evil_bless(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_evil_bless, level, AOE_TAR_CH | AOE_TAR_GROUP);
      break;
  }
}

void cast_satiate(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_satiate(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_satiate, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_cure_light(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_cure_light(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_cure_light, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_curse(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_curse(level, ch, victim, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_curse, level, 0);
      break;
  }
}

void cast_detect_alignment(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_detect_alignment(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_detect_alignment, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_detect_invisibility(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_detect_invisibility(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_detect_invisibility, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_detect_magic(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_detect_magic(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_detect_magic, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_recover_mana(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_recover_mana(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_recover_mana, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_detect_poison(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_detect_poison(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_detect_poison, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_dispel_evil(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_dispel_evil(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_dispel_evil, level, 0);
      break;
  }
}

void cast_dispel_good(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_dispel_good(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_dispel_good, level, 0);
      break;
  }
}


void cast_enchant_weapon(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_enchant_weapon(level, ch, 0, obj);
      break;
  }
}

void cast_heal(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim == ch) {
        act("You heal yourself.", FALSE, victim, 0, 0, TO_CHAR);
        act("$n heals $mself.", FALSE, victim, 0, 0, TO_ROOM);
      }
      else {
        act("You heal $N.", FALSE, ch, 0, victim, TO_CHAR);
        act("$n heals $N.", FALSE, ch, 0, victim, TO_NOTVICT);
      }

      spell_heal(level, ch, victim, 0);
      break;

    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_heal(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_heal, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_mana_heal(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim == ch) {
        act("You restore yourself slightly.", 0, victim, 0, 0, TO_CHAR);
        act("$n restores $mself slightly.", 0, victim, 0, 0, TO_ROOM);
      }
      else {
        act("You restore $N slightly.", 0, ch, 0, victim, TO_CHAR);
        act("$n restores $N slightly.", 0, ch, 0, victim, TO_NOTVICT);
      }

      spell_mana_heal(level, ch, victim, 0);
      break;

    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_mana_heal(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_mana_heal, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_regeneration(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_regeneration(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_regeneration, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_lay_hands(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim == ch) {
        act("You lay your hands on yourself.", FALSE, ch, 0, victim, TO_CHAR);
        act("$n lays $s hands on $mself.", FALSE, ch, 0, victim, TO_ROOM);
      }
      else {
        act("You lay your hands on $N.", FALSE, ch, 0, victim, TO_CHAR);
        act("$n lays $s hands on $N.", FALSE, ch, 0, victim, TO_NOTVICT);
      }

      spell_lay_hands(level, ch, victim, 0);
      break;

    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_lay_hands(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_lay_hands, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_hold(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_hold(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_hold, level, 0);
      break;
  }
}

void cast_invisibility(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_invisibility(level, ch, victim, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_invisibility, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_imp_invisibility(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
        send_to_char("The forces of chaos negate your potion!\n\r", ch);

        return;
      }

      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_improved_invisibility(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_improved_invisibility, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_remove_improved_invis(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_remove_improved_invis(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_remove_improved_invis, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_locate_object(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_locate_object(level, ch, 0, obj);
      break;
  }
}

void cast_paralyze(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_paralyze(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_paralyze, level, 0);
      break;
  }
}

void cast_poison(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_poison(level, ch, victim, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_poison, level, 0);
      break;
  }
}

void cast_protection_from_evil(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_protection_from_evil(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_protection_from_evil, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_protection_from_good(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_protection_from_good(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_protection_from_good, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_remove_curse(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_remove_curse(level, ch, victim, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_remove_curse, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_remove_paralysis(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_remove_paralysis(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_remove_paralysis, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_remove_poison(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_remove_poison(level, ch, victim, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_remove_poison, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_sanctuary(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_sanctuary(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_sanctuary, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_sphere(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_sphere(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_sphere, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_invulnerability(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_invulnerability(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_invulnerability, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_sleep(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_sleep(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_sleep, level, 0);
      break;
  }
}

void cast_strength(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_strength(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_strength, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_ventriloquate(ubyte level, CHAR *ch, char *arg, int type, CHAR *tar_ch, OBJ *tar_obj) {
  CHAR *tmp_ch;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char buf4[MAX_STRING_LENGTH];

  if (type != SPELL_TYPE_SPELL) {
    log_f("Attempt to ventriloquate by non-cast-spell.");
    return;
  }

  for (; *arg && (*arg == ' '); arg++);

  if (tar_obj) {
    sprintf(buf1, "The %s says '%s'\n\r", fname(OBJ_NAME(tar_obj)), arg);
    sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
      fname(OBJ_NAME(tar_obj)), arg);
    sprintf(buf4, "%s makes it sound like the %s says '%s'.\n\r", GET_NAME(ch),
      fname(OBJ_NAME(tar_obj)), arg);
  }
  else {
    sprintf(buf1, "%s says '%s'\n\r", PERS(tar_ch, tar_ch), arg);
    sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
      GET_NAME(tar_ch), arg);
    sprintf(buf4, "%s makes it sound like %s says '%s'.\n\r", GET_NAME(ch),
      GET_NAME(tar_ch), arg);
  }

  sprintf(buf3, "Someone says, '%s'\n\r", arg);

  for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
    tmp_ch = tmp_ch->next_in_room) {

    if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
      if (!IS_MORTAL(tmp_ch)) {
        send_to_char(buf4, tmp_ch);
      }
      else {
        if (saves_spell(tmp_ch, SAVING_SPELL, level))
          send_to_char(buf2, tmp_ch);
        else
          send_to_char(buf1, tmp_ch);
      }
    }
    else {
      if (tmp_ch == tar_ch) {
        if (!IS_MORTAL(tmp_ch)) {
          sprintf(buf3, "%s makes you say '%s'\n\r", GET_NAME(ch), arg);
          send_to_char(buf3, tar_ch);
        }
        else {
          send_to_char(buf3, tar_ch);
        }
      }
    }
  }
}

void cast_word_of_recall(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_word_of_recall(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_word_of_recall, level, AOE_TAR_CH | AOE_TAR_PCS_ONLY);
      break;
  }
}

void cast_total_recall(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_total_recall(level, ch, 0, 0);
      break;
  }
}

void cast_summon(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_summon(level, ch, victim, 0);
      break;
  }
}

void cast_locate_character(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_locate_character(level, ch, victim, 0);
      break;
  }
}

void cast_charm_person(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_charm_person(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_charm_person, level, AOE_TAR_NPCS_ONLY);
      break;
  }
}

void cast_sense_life(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_sense_life(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_sense_life, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_identify(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_identify(level, ch, victim, obj);
      break;
  }
}

void cast_recharge(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_recharge(level, ch, 0, obj);
      break;
  }
}

void cast_dispel_sanct(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_dispel_sanct(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_dispel_sanct, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_disenchant(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_disenchant(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_disenchant, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_petrify(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  if (!IS_NPC(ch) && (GET_LEVEL(ch) < LEVEL_WIZ)) {
    send_to_char("Your level isn't high enough to cast this spell.\n\r", ch);

    return;
  }

  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_petrify(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_petrify, level, 0);
      break;
  }
}

void cast_haste(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_haste(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_haste, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_great_mana(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  if (IS_MORTAL(ch)) {
    send_to_char("This spell is for gods only!\n\r", ch);

    return;
  }

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_great_mana(level, ch, 0, 0);
      break;
  }
}

void cast_perceive(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_perceive(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_perceive, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_quick(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_quick(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_quick, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_divine_intervention(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_divine_intervention(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_divine_intervention, level, AOE_TAR_CH | AOE_TAR_PCS_ONLY);
      break;
  }
}

void cast_rush(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_rush(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_rush, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_blood_lust(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_blood_lust(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_blood_lust, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_mystic_swiftness(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_mystic_swiftness(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_mystic_swiftness, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void cast_wind_slash(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_wind_slash(level, ch, 0, 0);
      break;
  }
}
