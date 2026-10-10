/*
** subclass.spells.c
**   Routines for subclass spells.
**   Spell# defines are in spells.h
**   SPELLO assignments and cast names are in spell_parser.c
**   Spell wear off messages are in constants.c
**   Sc check in subclass.c
**   spell proc declarations in subclass.h
**
** Written for RoninMUD by Ranger
**
** Do not distribute without permission.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "cmd.h"
#include "utility.h"
#include "act.h"
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"
#include "subclass.h"
#include "enchant.h"
#include "aff_ench.h"

void spell_rally(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_RALLY)) {
    print_spell_messages(victim, SPELL_RALLY);

    aff_apply(victim, SPELL_RALLY, 5, 5, APPLY_HITROLL, 0, 0);
  }
}

void cast_luck(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_luck(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_luck, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_luck(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_LUCK)) {
    print_spell_messages(victim, SPELL_LUCK);

    aff_apply(victim, SPELL_LUCK, 5, 0, 0, 0, 0);
  }
}

void spell_warchant(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  /* A PC's charmed follower is treated as being on its master's side. */
  CHAR *side_ch = victim;

  if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM) && GET_MASTER(victim) && !IS_NPC(GET_MASTER(victim))) {
    side_ch = GET_MASTER(victim);
  }

  if (SAME_GROUP_EX(side_ch, ch, TRUE)) {
    if (!aff_affected_by(victim, SPELL_WAR_CHANT)) {
      print_spell_messages(victim, SPELL_WAR_CHANT);

      aff_apply(victim, SPELL_WAR_CHANT, 5, -2, APPLY_SAVING_ALL, 0, 0);
    }
  }
  else if (side_ch != victim) {
    /* Another party's charmed follower: no effect. */
  }
  else {
    if ((IS_NPC(victim) || (IS_MORTAL(victim) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))) && !aff_affected_by(victim, SPELL_WAR_CHANT_DEBUFF)) {
      print_spell_messages(victim, SPELL_WAR_CHANT_DEBUFF);

      aff_apply(victim, SPELL_WAR_CHANT_DEBUFF, 5, -4, APPLY_HITROLL, 0, 0);
    }
  }
}

void spell_camaraderie(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_CAMARADERIE)) {
    print_spell_messages(victim, SPELL_CAMARADERIE);

    aff_apply(victim, SPELL_CAMARADERIE, 5, 0, 0, 0, 0);
  }
}

void cast_cloud_confusion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_cloud_confusion(level, ch, 0, 0);
      break;
  }
}

void spell_cloud_confusion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

  send_to_char("You whirl around, engulfing the area with a thick black cloud.\n\r", ch);
  act("$n whirls around, engulfing the area with a thick black cloud.", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_victim; temp_victim; temp_victim = next_victim) {
    next_victim = temp_victim->next_in_room;

    if ((temp_victim == ch) || IS_IMMORTAL(temp_victim)) continue;
    if (IS_NPC(temp_victim) && IS_NPC(ch) && (!IS_AFFECTED(temp_victim, AFF_CHARM) || (GET_MASTER(temp_victim) == ch))) continue;
    if (!IS_NPC(temp_victim) && !IS_NPC(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) continue;
    if (IS_NPC(temp_victim) && !IS_NPC(ch) && ((GET_MASTER(temp_victim) == ch) || (IS_AFFECTED(temp_victim, AFF_CHARM) && GET_MASTER(temp_victim) && !IS_NPC(GET_MASTER(temp_victim))))) continue;
    if (aff_affected_by(temp_victim, SPELL_CLOUD_CONFUSION)) continue;

    print_spell_messages(temp_victim, SPELL_CLOUD_CONFUSION);

    aff_apply(temp_victim, SPELL_CLOUD_CONFUSION, dice(1, 5) + 1, IS_NIGHT ? -(number(6, 9)) : -(number(5, 8)), APPLY_HITROLL, 0, 0);
  }
}

void cast_rage(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_rage(level, ch, victim, 0);

      if (!IS_NPC(ch) && (GET_CLASS(ch) == CLASS_ANTI_PALADIN)) {
        spell_blood_lust(level, ch, ch, 0);
      }
      break;

    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_rage(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_rage, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_rage(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_RAGE)) {
    print_spell_messages(victim, SPELL_RAGE);

    aff_apply(victim, SPELL_RAGE, 5, 0, 0, 0, AFF2_RAGE);
  }
}

void cast_righteousness(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_righteousness(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_righteousness, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_righteousness(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_RIGHTEOUSNESS)) {
    print_spell_messages(victim, SPELL_RIGHTEOUSNESS);

    aff_apply(victim, SPELL_RIGHTEOUSNESS, 10, 0, 0, 0, 0);
  }
}

void cast_debilitate(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_debilitate(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_debilitate, level, 0);
      break;
  }
}

void spell_debilitate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM)) return;

  if (!aff_affected_by(victim, SPELL_DEBILITATE)) {
    print_spell_messages(victim, SPELL_DEBILITATE);

    aff_apply(victim, SPELL_DEBILITATE, 5, -5, APPLY_HITROLL, 0, 0);
    aff_apply(victim, SPELL_DEBILITATE, 5, 10, APPLY_ARMOR, 0, 0);
  }
}

void cast_wrath_ancients(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_wrath_ancients(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_wrath_ancients, level, 0);
      break;
  }
}

void spell_wrath_ancients(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_WRATH_ANCIENTS, level, number(1800, 2000), DAM_MAGICAL, SAVING_NONE);
}

void cast_might(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_might(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_might, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_might(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_MIGHT)) {
    print_spell_messages(victim, SPELL_MIGHT);

    affect_apply(victim, SPELL_MIGHT, 20, 1, APPLY_HITROLL, 0, 0);
    affect_apply(victim, SPELL_MIGHT, 20, 3, APPLY_DAMROLL, 0, 0);
  }
}

void cast_mirror_image(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_mirror_image(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_mirror_image, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

static int mirror_image_enchant(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (!ench || !ch) return FALSE;

  if (cmd == MSG_SHOW_AFFECT_TEXT) {
    int image_count = MAX(0, ench->temp[0]);

    switch (image_count) {
      case 0:
      case 1:
        act("......$n is surrounded by a shifting mirror image.", FALSE, ch, 0, signaler, TO_VICT);
        break;
      case 2:
        act("......$n is surrounded by two shifting mirror images.", FALSE, ch, 0, signaler, TO_VICT);
        break;
      case 3:
        act("......$n is surrounded by three shifting mirror images.", FALSE, ch, 0, signaler, TO_VICT);
        break;
      case 4:
        act("......$n is surrounded by four shifting mirror images.", FALSE, ch, 0, signaler, TO_VICT);
        break;
      case 5:
        act("......$n is surrounded by five shifting mirror images.", FALSE, ch, 0, signaler, TO_VICT);
        break;
      default:
        act("......$n is surrounded by shifting mirror images.", FALSE, ch, 0, signaler, TO_VICT);
        break;
    }

    return FALSE;
  }

  return FALSE;
}

void spell_mirror_image(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int image_count = 1;

  if (check_subclass(ch, SC_ENCHANTER, 5))      image_count = 5;
  else if (check_subclass(ch, SC_ENCHANTER, 4)) image_count = 4;
  else if (check_subclass(ch, SC_ENCHANTER, 3)) image_count = 3;
  else if (check_subclass(ch, SC_ENCHANTER, 2)) image_count = 2;

  ENCH *existing = ench_get_from_char(victim, ENCH_NAME_MIRROR_IMAGE, 0);

  if (existing && (existing->temp[0] >= image_count)) {
    existing->duration = 50;
    if (victim == ch) {
      act("Your mirror images shimmer and steady.", FALSE, victim, 0, 0, TO_CHAR);
    }
    else {
      act("You weave a fresh set of mirror images around $N.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n weaves a fresh set of mirror images around you.", FALSE, ch, 0, victim, TO_VICT);
      act("$n weaves a fresh set of mirror images around $N.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
    return;
  }

  if (victim == ch) {
    print_spell_messages(victim, SPELL_MIRROR_IMAGE);
  }
  else {
    act("You weave a shifting illusion around $N.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n weaves a shifting illusion around you.", FALSE, ch, 0, victim, TO_VICT);
    act("$n weaves a shifting illusion around $N.", FALSE, ch, 0, victim, TO_NOTVICT);
  }

  ench_apply(victim, TRUE, ENCH_NAME_MIRROR_IMAGE, 0, 50, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, mirror_image_enchant);

  ENCH *mirror_ench = ench_get_from_char(victim, ENCH_NAME_MIRROR_IMAGE, 0);
  if (mirror_ench) mirror_ench->temp[0] = image_count;
}

void cast_distortion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_distortion(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_distortion, level, 0);
      break;
  }
}

void spell_distortion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (IS_AFFECTED(victim, AFF_SPHERE) && !aff_affected_by(victim, SPELL_DISTORTION)) {
    print_spell_messages(victim, SPELL_DISTORTION);

    aff_apply(victim, SPELL_DISTORTION, 2, 0, 0, 0, 0);
  }
}

void cast_iron_skin(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_iron_skin(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_iron_skin, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_iron_skin(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_IRON_SKIN)) {
    print_spell_messages(victim, SPELL_IRON_SKIN);

    aff_apply(victim, SPELL_IRON_SKIN, 10, ch == victim ? -30 : -30, APPLY_ARMOR, 0, 0);
  }
}

void cast_frostbolt(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_frostbolt(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_frostbolt, level, 0);
      break;
  }
}

void spell_frostbolt(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (!IS_IMMORTAL(victim) && !IS_IMMUNE2(victim, IMMUNE2_COLD)) {
    GET_MOVE(victim) = MAX(GET_MOVE(victim) - 100, 0);
  }

  damage_spell(ch, victim, SPELL_FROSTBOLT, level, 350, DAM_COLD, SAVING_NONE);
}

void cast_orb_protection(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_orb_protection(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_orb_protection, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_orb_protection(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(ch, SPELL_ORB_PROTECTION)) {
    print_spell_messages(victim, SPELL_ORB_PROTECTION);

    aff_apply(victim, SPELL_ORB_PROTECTION, 10, 0, 0, 0, 0);
  }
}

void cast_meteor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_meteor(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_meteor, level, 0);
      break;
  }
}

void spell_meteor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  /* Archmage SC1: Meteor - 60 + (5 * INT mod) damage. */
  damage_spell(ch, victim, SPELL_METEOR, level, 60 + (5 * GET_INT_BONUS(ch)), DAM_MAGICAL, SAVING_NONE);
}

/* Archmage SC3: Twinspell. Passively splashes a copy of a damage spell onto a second nearby hostile that is already
 * fighting the caster or another player. The copy rolls its own saving throw and goes through damage() normally.
 * Called from damage_spell(), which skips it for AoE casts. */
static bool twinspell_active = FALSE;

void twinspell_splash(CHAR *ch, CHAR *victim, int spell, int level, int base_dmg, int dmg_type, int saving_throw) {
  if (twinspell_active) return;

  if (!IS_MORTAL(ch) || !check_subclass(ch, SC_ARCHMAGE, 3)) return;

  /* The primary hit may have killed or moved the caster. */
  if (!IS_ALIVE(ch) || (victim == ch) || (base_dmg <= 0)) return;

  /* Pick one eligible hostile at random: in the room, not the primary victim, and already fighting the caster or
   * any other player (Archmages are rarely the one being hit, so engagement with anyone's PC counts). */
  CHAR *twin = NULL;
  int candidates = 0;

  for (CHAR *temp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); temp_ch; temp_ch = temp_ch->next_in_room) {
    if ((temp_ch == ch) || (temp_ch == victim) || IS_IMMORTAL(temp_ch)) continue;
    if (!IS_ALIVE(temp_ch) || SAME_GROUP(temp_ch, ch)) continue;

    CHAR *opp = GET_OPPONENT(temp_ch);

    if (!opp || IS_IMMORTAL(opp)) continue;

    /* Never splash the caster's own charmed followers. */
    if (IS_NPC(temp_ch) && IS_AFFECTED(temp_ch, AFF_CHARM) && GET_MASTER(temp_ch) && SAME_GROUP(GET_MASTER(temp_ch), ch)) continue;

    if (IS_NPC(temp_ch)) {
      /* An NPC is a valid twin if it is fighting the caster or any other player. */
      if ((opp != ch) && IS_NPC(opp)) continue;
    }
    else {
      /* Never splash onto players outside of chaotic/arena rooms; that would flag the caster as a killer. */
      if (!ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && !ROOM_ARENA(CHAR_REAL_ROOM(ch))) continue;

      /* A player is only a valid twin if they are fighting the caster or the caster's group. */
      if ((opp != ch) && !SAME_GROUP(opp, ch)) continue;
    }

    if (number(1, ++candidates) == 1) {
      twin = temp_ch;
    }
  }

  if (!twin) return;

  act("Your spell splits in two, its twin hits $N!", FALSE, ch, 0, twin, TO_CHAR);
  act("$n's spell splits in two, its twin hits you!", FALSE, ch, 0, twin, TO_VICT);
  act("$n's spell splits in two, its twin hits $N!", FALSE, ch, 0, twin, TO_NOTVICT);

  twinspell_active = TRUE;
  damage_spell(ch, twin, spell, level, base_dmg, dmg_type, saving_throw);
  twinspell_active = FALSE;
}

void cast_wall_thorns(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_wall_thorns(level, ch, 0, 0);
      break;
  }
}

void spell_wall_thorns(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (IS_MORTAL(ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ((ROOM_ZONE(CHAR_REAL_ROOM(ch)) == 30) || (ROOM_ZONE(CHAR_REAL_ROOM(ch)) == 31))) {
    send_to_char("Your druidic magic does not appear to function in this area.\n\r", ch);

    return;
  }

  OBJ *wall_of_thorns = get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch));

  if (wall_of_thorns) {
    send_to_room("The thick wall of brambles recedes, allowing passage to the surrounding world.\n\r", CHAR_REAL_ROOM(ch));

    extract_obj(wall_of_thorns);
  }
  else {
    wall_of_thorns = read_object(WALL_THORNS, VIRTUAL);

    if (wall_of_thorns) {
      OBJ_SPEC(wall_of_thorns) = level;
      OBJ_TIMER(wall_of_thorns) = 5;

      obj_to_room(wall_of_thorns, CHAR_REAL_ROOM(ch));

      send_to_room("A thick wall of razor-sharp brambles envelops the area!\n\r", CHAR_REAL_ROOM(ch));
    }
  }
}

void cast_rejuvenation(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_rejuvenation(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_rejuvenation, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_rejuvenation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  print_spell_messages(victim, SPELL_REJUVENATION);

  magic_heal(ch, victim, SPELL_REJUVENATION, 400, FALSE);
}

void cast_divine_wind(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_divine_wind(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_divine_wind, level, 0);
      break;
  }
}

void spell_divine_wind(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int stack_position(CHAR *ch, int target_position);

  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  int set_pos = stack_position(victim, POSITION_RESTING);

  damage_spell(ch, victim, SPELL_DIVINE_WIND, level, 300, DAM_MAGICAL, SAVING_NONE);

  if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;
  }
}

void cast_blackmantle(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_blackmantle(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_blackmantle, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_blackmantle(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(ch, SPELL_BLACKMANTLE)) {
    print_spell_messages(victim, SPELL_BLACKMANTLE);

    aff_apply(victim, SPELL_BLACKMANTLE, 9, 0, 0, 0, 0);
  }
}

void cast_rimefang(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_rimefang(level, ch, 0, 0);
      break;
  }
}

void spell_rimefang(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  int dam = number(5, 7) * GET_LEVEL(ch);

  /* Increased damage at night. */
  if (IS_NIGHT) {
    dam += number(GET_LEVEL(ch) / 2, GET_LEVEL(ch));
  }

  /* Increased damage during winter. */
  if ((time_info.month <= 2) || (time_info.month >= 15)) {
    dam += number(GET_LEVEL(ch) / 2, GET_LEVEL(ch));
  }

  send_to_char("An aura of frost starts to form around you.\n\r", ch);
  act("An aura of frost starts to form around $n.", FALSE, ch, 0, 0, TO_ROOM);

  int duration = GET_LEVEL(ch) / 10;

  for (CHAR *temp_vict = world[CHAR_REAL_ROOM(ch)].people, *next_vict; temp_vict; temp_vict = next_vict) {
    next_vict = temp_vict->next_in_room;

    if ((temp_vict == ch) || IS_IMMORTAL(temp_vict) || (IS_NPC(ch) && IS_NPC(temp_vict))) continue;

    if (IS_NPC(temp_vict) || ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_vict))) {
      act("$n sends a wall of jagged ice cascading towards you!", FALSE, ch, 0, temp_vict, TO_VICT);

      if (!IS_SET(GET_IMMUNE(temp_vict), IMMUNE_PARALYSIS) &&
          !IS_AFFECTED(temp_vict, AFF_PARALYSIS)) {
        aff_apply(temp_vict, SPELL_PARALYSIS, duration, 100, APPLY_ARMOR, AFF_PARALYSIS, 0);
        aff_apply(temp_vict, SPELL_PARALYSIS, duration, -5, APPLY_HITROLL, AFF_PARALYSIS, 0);

        send_to_char("Your limbs freeze in place.\n\r", temp_vict);
        act("$n is paralyzed!", FALSE, temp_vict, 0, 0, TO_ROOM);

        if (IS_NPC(temp_vict)) {
          MOB_ATT_TIMER(temp_vict) = MAX(MOB_ATT_TIMER(temp_vict), 2);
        }
        else {
          WAIT_STATE(temp_vict, PULSE_VIOLENCE * 2);
        }
      }

      dam = damage(ch, temp_vict, dam, SPELL_RIMEFANG, DAM_COLD);
    }
  }
}

void cast_passdoor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  char buf[MIL];
  int dir = 0;

  one_argument(arg, buf);

  if (!*buf)
  {
    send_to_char("What direction?\n\r",ch);
    return;
  }

  if (is_abbrev(buf, "north")) dir = NORTH;
  else if (is_abbrev(buf, "east")) dir = EAST;
  else if (is_abbrev(buf, "south")) dir = SOUTH;
  else if (is_abbrev(buf, "west")) dir = WEST;
  else if (is_abbrev(buf, "up")) dir = UP;
  else if (is_abbrev(buf, "down")) dir = DOWN;
  else
  {
    send_to_char("What direction?\n\r", ch);
    return;
  }

  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_passdoor(level, ch, dir);
      break;
    default:
      log_f("Wrong type called in passdoor!");
      break;
  }
}

void spell_passdoor(ubyte level, CHAR *ch, int door)
{
  char buf[MIL];
  int to_room;

  if (ch->specials.riding)
  {
    send_to_char("You must dismount first.\n\r", ch);
    return;
  }


  if (EXIT(ch, door) && (EXIT(ch, door)->to_room_r != NOWHERE) &&
      (EXIT(ch, door)->to_room_v != 0) &&
      !IS_SET(EXIT(ch, door)->exit_info, EX_CRAWL) &&
      !IS_SET(EXIT(ch, door)->exit_info, EX_JUMP) &&
      !IS_SET(EXIT(ch, door)->exit_info, EX_ENTER) &&
      !IS_SET(EXIT(ch, door)->exit_info, EX_CLIMB))
  {
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
    {
      send_to_char("That direction is open already.\n\r", ch);
      return;
    }

    sprintf(buf, "You slowly phase your body through the %s.\n\r", EXIT(ch, door)->keyword);
    send_to_char(buf, ch);
    sprintf(buf, "$n slowly phases $s body through the %s.", EXIT(ch, door)->keyword);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    to_room = EXIT(ch, door)->to_room_r;
    char_from_room(ch);
    char_to_room(ch, to_room);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    do_look(ch, "", CMD_LOOK);

    GET_POS(ch) = POSITION_STUNNED;
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);

    signal_room(CHAR_REAL_ROOM(ch), ch, MSG_ENTER, "");
  }
  else
  {
    send_to_char("There is nothing in that direction.\n\r", ch);
    return;
  }
}

void cast_desecrate(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_desecrate(level, ch, 0, obj);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_desecrate, level, AOE_TAR_OBJS);
      break;
  }
}

void spell_desecrate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!IS_CORPSE(obj)) {
    send_to_char("You must target a corpse.\n\r", ch);

    return;
  }

  if ((OBJ_COST(obj) == PC_CORPSE) && OBJ_CONTAINS(obj)) {
    send_to_char("The corpse has something in it.\n\r", ch);

    return;
  }

  int mob_level = OBJ_VALUE2(obj);

  if (!aff_affected_by(ch, SPELL_DESECRATE)) {
    aff_apply(ch, SPELL_DESECRATE, 7, -(mob_level / 2), APPLY_ARMOR, 0, 0);
    aff_apply(ch, SPELL_DESECRATE, 7, mob_level, APPLY_HP_REGEN, 0, 0);
  }

  act("You perform vile acts upon $p, desecrating it.", FALSE, ch, obj, 0, TO_CHAR);
  act("$n performs vile acts upon $p, desecrating it.", FALSE, ch, obj, 0, TO_ROOM);

  if (IS_MORTAL(ch) && (mob_level >= (GET_LEVEL(ch) - 10))) {
    send_to_room("Some blood and gore is left behind after the ritual is complete.\n\r", CHAR_REAL_ROOM(ch));

    ROOM_BLOOD(CHAR_REAL_ROOM(ch)) = MIN(ROOM_BLOOD(CHAR_REAL_ROOM(ch)) + 1, 10);
  }

  /* Get all the stuff from the corpse and put it on the ground. */
  for (OBJ *temp_obj = OBJ_CONTAINS(obj), *next_obj; temp_obj; temp_obj = next_obj) {
    next_obj = temp_obj->next_content;

    obj_from_obj(temp_obj);
    obj_to_room(temp_obj, CHAR_REAL_ROOM(victim));
  }

  extract_obj(obj);

  check_equipment(ch);
}

int aid_enchantment(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (cmd == MSG_REMOVE_ENCH) {
    char *saveptr, *name = strtok_r(ench->name, " ", &saveptr);

    printf_to_char(ch, "You feel the inspiration of %s words fade.\n\r", name ? name : "Someone's");

    return FALSE;
  }

  return FALSE;
}

void spell_aid(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  char buf[MIL];

  snprintf(buf, sizeof(buf), "%s's Aid", GET_DISP_NAME(ch));

  if (!ench_enchanted_by(victim, buf, 0)) {
    ENCH aid_ench = { 0 };

    aid_ench.name = buf;
    aid_ench.duration = 5;
    aid_ench.interval = ENCH_INTERVAL_TICK;
    aid_ench.temp[0] = 10;
    aid_ench.metadata = ENCH_APPLY_DMG_PCT;
    aid_ench.func = aid_enchantment;

    ench_to_char(victim, &aid_ench, FALSE);

    if (ch == victim) {
      send_to_char("You feel inspired by your words!\n\r", victim);
    }
    else {
      act("You feel inspired by $n's words!", FALSE, ch, 0, victim, TO_VICT);
    }
  }
}

void cast_magic_weapon(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_magic_weapon(level, ch, victim, 0);
      break;
  }
}

static int magic_weapon_enchant(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (!ench || !ch) return FALSE;

  if (cmd == MSG_SHOW_AFFECT_TEXT) {
    act("......$n's weapon crackles with arcane power.", FALSE, ch, 0, signaler, TO_VICT);
    return FALSE;
  }
  if (cmd == MSG_REMOVE_ENCH) {
    send_to_char("The arcane power fades from your weapon.\n\r", ch);
    return FALSE;
  }
  if (cmd == MSG_MOBACT) {
    CHAR *opp = GET_OPPONENT(ch);

    if (opp && SAME_ROOM(ch, opp) && GET_WEAPON(ch) && chance(10 + (2 * ench->temp[0]))) {
      weapon_attack_proc(ch, opp, GET_WEAPON(ch));
    }
  }
  return FALSE;
}

void spell_magic_weapon(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!victim) victim = ch;

  if (!GET_WEAPON(victim)) {
    send_to_char("There's no wielded weapon to enchant.\n\r", ch);
    return;
  }

  print_spell_messages(victim, SPELL_MAGIC_WEAPON);

  /* Duration counts ticks; the proc runs off MSG_MOBACT, which every enchant function still receives. */
  ench_apply(victim, TRUE, ENCH_NAME_MAGIC_WEAPON, 0, 50 + GET_WIS_BONUS(ch), ENCH_INTERVAL_ROUND,
             10 + (2 * GET_INT_BONUS(ch)), APPLY_DAMROLL, 0, 0, magic_weapon_enchant);

  ENCH *mw = ench_get_from_char(victim, ENCH_NAME_MAGIC_WEAPON, 0);
  if (mw) mw->temp[0] = GET_WIS_BONUS(ch);
}

static int familiar_type_from_arg(char *arg) {
  char buf[MIL];

  one_argument(arg, buf);

  if (!*buf) return -1;
  if (is_abbrev(buf, "sword")) return 0;
  if (is_abbrev(buf, "sentinel")) return 1;
  if (is_abbrev(buf, "homunculus")) return 2;

  return -1;
}

#define FAMILIAR_SWORD       0
#define FAMILIAR_SENTINEL    1
#define FAMILIAR_HOMUNCULUS  2

void familiar_fade_message(CHAR *ch, int type);
static void familiar_summon_message(CHAR *ch, int type);
static const char * const familiar_names[] = { "ethereal sword", "sentinel familiar", "tiny homunculus" };

int familiar_enchantment(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (!ench || !ch) return FALSE;

  if (cmd == MSG_SHOW_AFFECT_TEXT) {
    switch (ench->temp[0]) {
      case FAMILIAR_SWORD:
        act("......an ethereal sword dances in the air around $n.", FALSE, ch, 0, signaler, TO_VICT);
        break;
      case FAMILIAR_SENTINEL:
        act("......a sentinel familiar stands at the ready by $n's side.", FALSE, ch, 0, signaler, TO_VICT);
        break;
      case FAMILIAR_HOMUNCULUS:
        act("......a tiny homunculus whispers and peers about from $n's shoulder.", FALSE, ch, 0, signaler, TO_VICT);
        break;
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH) {
    familiar_fade_message(ch, ench->temp[0]);
    return FALSE;
  }

  if (cmd == MSG_ROUND) {
    CHAR *opp = GET_OPPONENT(ch);

    if (!opp || !SAME_ROOM(ch, opp)) return FALSE;

    switch (ench->temp[0]) {
      case FAMILIAR_SWORD: {
        int sword_dmg = MAX(1, GET_DAMROLL(ch));

        /* Messages first: if the hit kills, opp is gone and $N would print literally. */
        act("An ethereal sword slices across $N in a flashing arc!", FALSE, ch, 0, opp, TO_NOTVICT);
        act("An ethereal sword slices across you in a flashing arc!", FALSE, ch, 0, opp, TO_VICT);
        act("You send an ethereal sword slicing across $N in a flashing arc!", FALSE, ch, 0, opp, TO_CHAR);

        damage(ch, opp, sword_dmg, TYPE_UNDEFINED, DAM_MAGICAL);
        break;
      }

      case FAMILIAR_HOMUNCULUS: {
        int spec_chance = number(5, MAX(5, GET_WIS_BONUS(ch)));
        if (!chance(spec_chance)) return FALSE;

        int mana_restore = number(5,20);
        int pilfer_damage = 50 * (GET_INT_BONUS(ch) + 5);

        act("A tiny homunculus darts up $N's back and plucks a glowing mote from $S aura, cackling shrilly!", FALSE, ch, 0, opp, TO_NOTVICT);
        act("A tiny homunculus darts up your back and plucks a glowing mote from your aura, cackling shrilly!", FALSE, ch, 0, opp, TO_VICT);
        act("Your homunculus darts up $N's back and plucks a glowing mote from $S aura, cackling shrilly!", FALSE, ch, 0, opp, TO_CHAR);

        /* Restore mana to the group (SAME_GROUP is TRUE for ch itself). */
        for (CHAR *room_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); room_ch; room_ch = room_ch->next_in_room) {
          if (IS_NPC(room_ch) || !IS_MORTAL(room_ch) || !SAME_GROUP(ch, room_ch)) continue;

          int mana_gain = MIN(GET_MAX_MANA(room_ch) - GET_MANA(room_ch), mana_restore);

          if (mana_gain > 0) {
            GET_MANA(room_ch) += mana_gain;

            send_to_char("The homunculus flicks the stolen mote into the air; it bursts over you, restoring some mana.\n\r", room_ch);
          }
        }

        /* Damage last, so a kill can't affect anything above. */
        damage(ch, opp, pilfer_damage, TYPE_UNDEFINED, DAM_MAGICAL);
        break;
      }
    }

    return FALSE;
  }

  return FALSE;
}

void cast_familiar(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_familiar(level, ch, arg, victim, 0);
      break;
  }
}

void familiar_fade_message(CHAR *ch, int type) {
  switch (type) {
    case FAMILIAR_SWORD:
      act("The dancing sword around $n fades, then vanishes with a faint pop.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Your dancing sword fades away.\n\r", ch);
      break;
    case FAMILIAR_SENTINEL:
      act("The sentinel familiar around $n collapses into itself until nothing remains.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Your sentinel familiar fades away.\n\r", ch);
      break;
    case FAMILIAR_HOMUNCULUS:
      act("The homunculus hovering around $n squeaks once and dissolves into motes of light.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Your homunculus familiar fades away.\n\r", ch);
      break;
  }
}

static void familiar_summon_message(CHAR *ch, int type) {
  switch (type) {
    case FAMILIAR_SWORD:
      act("An ethereal sword blinks into being and begins to dance around $n.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You conjure an ethereal sword, which begins to dance around you.\n\r", ch);
      break;
    case FAMILIAR_SENTINEL:
      act("A sentinel familiar coalesces beside $n, standing at the ready.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You call forth a sentinel familiar to stand guard at your side.\n\r", ch);
      break;
    case FAMILIAR_HOMUNCULUS:
      act("A tiny homunculus pops into existence on $n's shoulder, peering about.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You summon a tiny homunculus, which scrambles onto your shoulder.\n\r", ch);
      break;
  }
}

void spell_familiar(ubyte level, CHAR *ch, char *arg, CHAR *victim, OBJ *obj) {
  if (!IS_ALIVE(ch)) return;

  int familiar_type = familiar_type_from_arg(arg);

  if (familiar_type < 0) {
    send_to_char("Choose a familiar form: sword, sentinel, or homunculus.\n\r", ch);
    return;
  }

  const int duration = 40 + GET_INT_BONUS(ch) + GET_WIS_BONUS(ch);
  const int sentinel_hp = MAX(500, (GET_LEVEL(ch) * 20) + ((GET_INT_BONUS(ch) + GET_WIS_BONUS(ch)) * 40));

  ENCH *existing = ench_get_from_char(ch, ENCH_NAME_FAMILIAR, 0);

  /* Same familiar: renew it. */
  if (existing && (existing->temp[0] == familiar_type)) {
    char buf[256];

    existing->duration = MAX(existing->duration, duration);

    if (familiar_type == FAMILIAR_SENTINEL) {
      existing->temp[1] = sentinel_hp;   /* recasting recharges a worn-down sentinel */
    }

    snprintf(buf, sizeof(buf), "You renew your %s.\n\r", familiar_names[familiar_type]);
    send_to_char(buf, ch);
    return;
  }

  /* Different familiar: dismiss the old one first. */
  if (existing) {
    familiar_fade_message(ch, existing->temp[0]);
  }

  print_spell_messages(ch, SPELL_FAMILIAR);

  ench_apply(ch, TRUE, ENCH_NAME_FAMILIAR, SPELL_FAMILIAR, duration, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, familiar_enchantment);

  ENCH *fam = ench_get_from_char(ch, ENCH_NAME_FAMILIAR, 0);

  if (fam) {
    fam->temp[0] = familiar_type;
    fam->temp[1] = (familiar_type == FAMILIAR_SENTINEL) ? sentinel_hp : 0;
  }

  familiar_summon_message(ch, familiar_type);
}

/* A summon bound by magic (animated dead, greater elementals, Charm Person). Bought pets are charmed but not bound. */
static bool is_bound_summon(CHAR *mob) {
  if (!mob || !IS_NPC(mob) || !IS_AFFECTED(mob, AFF_CHARM)) return FALSE;

  return IS_AFFECTED(mob, AFF_ANIMATE) || is_greater_elemental(mob) || affected_by_spell(mob, SPELL_CHARM_PERSON);
}

CHAR *binding_mastery_owner(CHAR *summon) {
  if (!summon || !summon->master || !is_bound_summon(summon)) return NULL;

  CHAR *owner = summon->master;

  if (IS_NPC(owner) || !IS_MORTAL(owner) || !SAME_ROOM(summon, owner)) return NULL;
  if (!check_subclass(owner, SC_ENCHANTER, 5)) return NULL;

  return owner;
}

void cast_tether(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_tether(level, ch, arg, victim, obj);
      break;
  }
}

void spell_tether(ubyte level, CHAR *ch, char *arg, CHAR *victim, OBJ *obj) {
  if (!IS_ALIVE(ch)) return;

  int called = 0;

  for (FOL *fol = ch->followers, *next_fol; fol; fol = next_fol) {
    next_fol = fol->next;

    CHAR *summon = fol->follower;

    if (!summon || (summon->master != ch) || !is_bound_summon(summon)) continue;
    if (SAME_ROOM(summon, ch) || (CHAR_REAL_ROOM(summon) == NOWHERE)) continue;

    /* Disengage the summon and anything fighting it before it leaves. */
    if (GET_OPPONENT(summon)) stop_fighting(summon);

    for (CHAR *temp = ROOM_PEOPLE(CHAR_REAL_ROOM(summon)); temp; temp = temp->next_in_room) {
      if (GET_OPPONENT(temp) == summon) stop_fighting(temp);
    }

    act("$n is yanked away along a glowing tether.", FALSE, summon, 0, 0, TO_ROOM);
    char_from_room(summon);
    char_to_room(summon, CHAR_REAL_ROOM(ch));
    act("$n arrives, drawn along $N's tether.", FALSE, summon, 0, ch, TO_NOTVICT);

    called++;
  }

  if (called)
    send_to_char("Your bound servants are drawn to your side.\n\r", ch);
  else
    send_to_char("None of your bound servants are beyond your reach.\n\r", ch);
}

#define SLIME_MAX_STACKS        10

bool is_greater_elemental(CHAR *ch) {
  return ch && IS_NPC(ch) && ench_enchanted_by(ch, ENCH_NAME_GREATER_ELEMENTAL, 0);
}

static CHAR *find_greater_elemental(CHAR *ch) {
  for (FOL *fol = ch->followers; fol; fol = fol->next) {
    if (is_greater_elemental(fol->follower)) return fol->follower;
  }

  return NULL;
}

int elemental_vnum_from_arg(char *arg) {
  char buf[MIL];

  one_argument(arg, buf);

  if (!*buf) return -1;
  if (is_abbrev(buf, "slime")) return GREATER_ELEMENTAL_SLIME;
  if (is_abbrev(buf, "earth")) return GREATER_ELEMENTAL_EARTH;
  if (is_abbrev(buf, "air"))   return GREATER_ELEMENTAL_AIR;
  if (is_abbrev(buf, "void"))  return GREATER_ELEMENTAL_VOID;

  return -1;
}

/* Slime: stacking poison DoT on the victim, modeled on poison_dart_tick. */
int corrosive_slime_tick(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (!ench || !ch) return FALSE;

  if (cmd == MSG_SHOW_AFFECT_TEXT) {
    act("......$n is coated in corrosive slime.", FALSE, ch, 0, signaler, TO_VICT);
    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH) {
    act("The slime coating $n sloughs away.", FALSE, ch, 0, 0, TO_ROOM);
    return FALSE;
  }

  if (cmd != MSG_ROUND) return FALSE;

  int dam = ench->temp[0] * ench->temp[1];                 /* stacks * per-stack damage */
  if (dam >= GET_HIT(ch)) dam = MAX(0, GET_HIT(ch) - 1);   /* never kill (and extract) mid-signal, same as the dart */

  if (dam > 0) {
    act("Corrosive slime eats away at $n.", FALSE, ch, 0, 0, TO_ROOM);
    act("The corrosive slime eats away at you!", FALSE, ch, 0, 0, TO_CHAR);
    damage(ch, ch, dam, TYPE_UNDEFINED, DAM_POISON);
  }

  return FALSE;
}

static void apply_corrosive_slime(CHAR *victim, int per_stack) {
  ENCH *slime = ench_get_from_char(victim, ENCH_NAME_CORROSIVE_SLIME, 0);

  if (!slime) {
    ench_apply(victim, TRUE, ENCH_NAME_CORROSIVE_SLIME, 0, 40, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, corrosive_slime_tick);
    slime = ench_get_from_char(victim, ENCH_NAME_CORROSIVE_SLIME, 0);
    if (slime) {
      slime->temp[0] = 1;
      slime->temp[1] = per_stack;
    }
    return;
  }

  int stacks = MIN(slime->temp[0] + 1, SLIME_MAX_STACKS);
  slime->temp[0] = stacks;
  slime->temp[1] = per_stack;
}

/* One ending for every case (expiry, orphaned, dismissed, destroyed).
   extract_char() drops carried and worn items in the room and leaves no corpse. */
static void elemental_end(CHAR *mob, bool destroyed) {
  switch (V_MOB(mob)) {
    case GREATER_ELEMENTAL_SLIME:
      act(destroyed ? "$n is torn apart, its slime splattering and sizzling away to nothing."
                    : "$n loses cohesion as the magic binding it fades, and the slime slumps into a puddle that quickly evaporates.",
          FALSE, mob, 0, 0, TO_ROOM);
      break;
    case GREATER_ELEMENTAL_EARTH:
      act(destroyed ? "The animated $n shatters and crumbles to the ground, its binding magic broken."
                    : "The animated $n crumbles to the ground as the magic that supported it fades.",
          FALSE, mob, 0, 0, TO_ROOM);
      break;
    case GREATER_ELEMENTAL_AIR:
      act(destroyed ? "$n is shredded into scattered wisps that die away on the wind."
                    : "$n unravels into a last, dying gust as the magic that bound it fades.",
          FALSE, mob, 0, 0, TO_ROOM);
      break;
    case GREATER_ELEMENTAL_VOID:
      act(destroyed ? "$n is ripped apart, its shadowy form dissolving into nothingness."
                    : "$n folds in upon itself and is swallowed by shadow as the magic that sustains it fades.",
          FALSE, mob, 0, 0, TO_ROOM);
      break;
  }

  extract_char(mob);
}

/* Mob spec for all four vnums. Melee comes from the OLC stat block;
   this only adds the form-specific extras while the elemental is fighting. */
int greater_elemental_spec(CHAR *mob, CHAR *signaler, int cmd, char *arg) {
  /* Destroyed: flavor text, no corpse. Must return TRUE so damage() skips normal death on a mob we just extracted. */
  if (cmd == MSG_DEAD) {
    elemental_end(mob, TRUE);
    return TRUE;
  }

  if (cmd != MSG_ROUND) return FALSE;

  ENCH *ench = ench_get_from_char(mob, ENCH_NAME_GREATER_ELEMENTAL, 0);
  CHAR *master = mob->master;

  /* Expired (temp[0] counts down rounds), or orphaned (master died / quit / charm broken). */
  if (!ench || !master || !IS_AFFECTED(mob, AFF_CHARM) || (--ench->temp[0] <= 0)) {
    elemental_end(mob, FALSE);
    return FALSE;
  }

  /* temp[1] counts down rounds spent fighting without the master in the room; only matters while in combat. */
  if (mob->specials.fighting) {
    if (SAME_ROOM(mob, master)) {
      ench->temp[1] = 5;   /* reset to 5 rounds if master is present */
    }
    else if (--ench->temp[1] <= 0) {
      elemental_end(mob, FALSE);
      return FALSE;
    }
  }

  /* Supplement only: no free hits unless the elemental itself is in combat. NPC targets only. */
  CHAR *opp = GET_OPPONENT(mob);

  if (!opp || !SAME_ROOM(mob, opp) || !IS_NPC(opp)) return FALSE;

  switch (V_MOB(mob)) {
    case GREATER_ELEMENTAL_SLIME:
      if (chance(40)) {
        act("$n hurls a glob of caustic slime over $N!", FALSE, mob, 0, opp, TO_NOTVICT);
        act("$n hurls a glob of caustic slime over you!", FALSE, mob, 0, opp, TO_VICT);
        apply_corrosive_slime(opp, 10 + GET_INT_BONUS(master));

        if (chance(20 + GET_WIS_BONUS(master)) && !ench_enchanted_by(opp, ENCH_NAME_SLUGGISHNESS, 0)) {
          ench_apply(opp, TRUE, ENCH_NAME_SLUGGISHNESS, 0, 5, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, sluggishness_enchant);
          act("The clinging slime slows $N's movements.", FALSE, mob, 0, opp, TO_NOTVICT);
          act("The clinging slime slows your movements.", FALSE, mob, 0, opp, TO_VICT);
        }
      }
      break;

    case GREATER_ELEMENTAL_EARTH:   /* melee already does the damage; the spec is only the stun */
      if (chance(15 + GET_WIS_BONUS(master))) {
        GET_POS(opp) = stack_position(opp, POSITION_STUNNED);
        skill_wait_victim(opp, SPELL_GREATER_ELEMENTAL, 1);

        act("$n slams the ground, and the shockwave leaves $N reeling!", FALSE, mob, 0, opp, TO_NOTVICT);
        act("$n slams the ground, and the shockwave leaves $N reeling!", FALSE, mob, 0, opp, TO_VICT);
      }
      break;

    case GREATER_ELEMENTAL_AIR:
      if (chance(35)) {
        act("$n buffets $N with a torrent of icy winds!", FALSE, mob, 0, opp, TO_NOTVICT);
        act("$n buffets you with a torrent of icy winds!", FALSE, mob, 0, opp, TO_VICT);
        damage(mob, opp, GET_LEVEL(master) + (5 * GET_INT_BONUS(master)), TYPE_UNDEFINED, DAM_MAGICAL);

        if ((CHAR_REAL_ROOM(opp) != NOWHERE) && chance(20 + GET_WIS_BONUS(master)) && !ench_enchanted_by(opp, ENCH_NAME_CONFUSION, 0)) {
          ench_apply(opp, TRUE, ENCH_NAME_CONFUSION, 0, 8, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, confusion_enchant);
          act("$N staggers, buffeted into confusion.", FALSE, mob, 0, opp, TO_NOTVICT);
          act("You are staggered, buffeted into confusion.", FALSE, mob, 0, opp, TO_VICT);
        }
      }
      break;

    case GREATER_ELEMENTAL_VOID:
      if (chance(30)) {
        act("$n spears $N with a lance of living shadow!", FALSE, mob, 0, opp, TO_NOTVICT);
        act("$n spears you with a lance of living shadow!", FALSE, mob, 0, opp, TO_VICT);
        damage(mob, opp, GET_LEVEL(master) + (10 * GET_INT_BONUS(master)), TYPE_UNDEFINED, DAM_MAGICAL);

        if ((CHAR_REAL_ROOM(opp) != NOWHERE) && chance(35 + GET_WIS_BONUS(master)) && !ench_enchanted_by(opp, ENCH_NAME_EXPOSED, 0)) {
          ench_apply(opp, TRUE, ENCH_NAME_EXPOSED, 0, 4, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, exposed_enchant);
          act("The shadows strip away $N's protections!", FALSE, mob, 0, opp, TO_NOTVICT);
          act("The shadows strip away your protections!", FALSE, mob, 0, opp, TO_VICT);
        }
      }
      break;
  }

  return FALSE;
}

void cast_greater_elemental(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_greater_elemental(level, ch, arg, victim, 0);
      break;
  }
}

void spell_greater_elemental(ubyte level, CHAR *ch, char *arg, CHAR *victim, OBJ *obj) {
  if (!IS_ALIVE(ch)) return;

  int vnum = elemental_vnum_from_arg(arg);

  if (vnum < 0) {
    send_to_char("Choose an elemental form: slime, earth, air, or void.\n\r", ch);
    return;
  }

  const int duration = 80 + GET_INT_BONUS(ch) + GET_WIS_BONUS(ch);   /* rounds */

  CHAR *existing = find_greater_elemental(ch);

  if (existing) {
    if (V_MOB(existing) == vnum) {    /* same form: renew */
      ENCH *old = ench_get_from_char(existing, ENCH_NAME_GREATER_ELEMENTAL, 0);

      if (old) {
        old->temp[0] = MAX(old->temp[0], duration);
        old->temp[1] = 5;   /* reset the combat-without-master counter */
      }

      act("Your elemental surges with renewed power.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }

    elemental_end(existing, FALSE);  /* different form: dismiss the old one */
  }

  CHAR *elem = read_mobile(vnum, VIRTUAL);

  if (!elem) {
    log_f("BUG: spell_greater_elemental: missing mob prototype %d", vnum);
    send_to_char("The elemental plane doesn't answer. Poke an immortal.\n\r", ch);
    return;
  }

  char_to_room(elem, CHAR_REAL_ROOM(ch));
  add_follower(elem, ch);
  aff_apply(elem, SPELL_CHARM_PERSON, level, 0, 0, AFF_CHARM, 0);

  ench_apply(elem, TRUE, ENCH_NAME_GREATER_ELEMENTAL, SPELL_GREATER_ELEMENTAL, -1, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, 0);

  ENCH *ench = ench_get_from_char(elem, ENCH_NAME_GREATER_ELEMENTAL, 0);
  if (ench) {
    ench->temp[0] = duration;
    ench->temp[1] = 5;   /* reset the combat-without-master counter */
  }

  act("$n calls forth a $N!", FALSE, ch, 0, elem, TO_ROOM);
  act("You call forth a $N!", FALSE, ch, 0, elem, TO_CHAR);
}

void cast_magic_armament(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_magic_armament(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_magic_armament, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_magic_armament(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_MAGIC_ARMAMENT)) {
    print_spell_messages(victim, SPELL_MAGIC_ARMAMENT);

    aff_apply(victim, SPELL_MAGIC_ARMAMENT, 5, 0, 0, 0, 0);
  }
}

void cast_fortification(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_fortification(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_fortification, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_fortification(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_FORTIFICATION)) {
    print_spell_messages(victim, SPELL_FORTIFICATION);

    aff_apply(victim, SPELL_FORTIFICATION, level / 4, 0, 0, AFF_SANCTUARY, AFF2_FORTIFICATION);
  }
}

void cast_disrupt_sanct(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_disrupt_sanct(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_disrupt_sanct, level, 0);
      break;
  }
}

void spell_disrupt_sanct(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  if (!ench_enchanted_by(victim, ENCH_NAME_DISRUPT_SANCT, 0) && IS_AFFECTED(victim, AFF_SANCTUARY)) {
    const int duration = 10 + GET_WIS_BONUS(ch);

    print_spell_messages(victim, SPELL_DISRUPT_SANCT);

    ench_apply(victim, TRUE, ENCH_NAME_DISRUPT_SANCT, 0, duration, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, 0);
  }
}

void cast_wrath_of_god(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_wrath_of_god(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_wrath_of_god, level, 0);
      break;
  }
}

void spell_wrath_of_god(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  const int hitroll_step = 1, damroll_step = 10;
  const int hitroll_max = -1, damroll_max = -50;

  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  bool found = FALSE;

  for (AFF *aff = victim->affected; aff; aff = aff->next) {
    if (aff->type == SPELL_WRATH_OF_GOD) {
      found = TRUE;

      if (aff->location == APPLY_HITROLL) {
        aff->modifier = MAX(aff->modifier - hitroll_step, hitroll_max);
      }
      else if (aff->location == APPLY_DAMROLL) {
        aff->modifier = MAX(aff->modifier - damroll_step, damroll_max);
      }
    }
  }

  if (!found) {
    aff_apply(victim, SPELL_WRATH_OF_GOD, 2, -hitroll_step, APPLY_HITROLL, 0, 0);
    aff_apply(victim, SPELL_WRATH_OF_GOD, 2, -damroll_step, APPLY_DAMROLL, 0, 0);
  }

  damage_spell(ch, victim, SPELL_WRATH_OF_GOD, level, 600, DAM_MAGICAL, SAVING_NONE);
}

void cast_power_of_devotion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_power_of_devotion(level, ch, victim, 0);

      char buf[MIL];

      arg = one_argument(arg, buf);

      while (*buf) {
        if (GET_MANA(ch) < spell_info[SPELL_POWER_OF_DEVOTION].min_usesmana) {
          send_to_char("You can't summon enough energy to continue casting your spell.\n\r", ch);

          break;
        }

        victim = get_char_room_vis(ch, buf);

        if (victim) {
          GET_MANA(ch) -= spell_info[SPELL_POWER_OF_DEVOTION].min_usesmana;

          spell_power_of_devotion(level, ch, victim, 0);
        }
        else {
          send_to_char("Nobody here by that name.\n\r", ch);
        }

        arg = one_argument(arg, buf);
      }
      break;

    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_power_of_devotion(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_power_of_devotion, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_power_of_devotion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_POWER_OF_DEVOTION)) {
    print_spell_messages(victim, SPELL_POWER_OF_DEVOTION);

    aff_apply(victim, SPELL_POWER_OF_DEVOTION, level / 4, -15, APPLY_ARMOR, AFF_SANCTUARY, 0);
    aff_apply(victim, SPELL_POWER_OF_DEVOTION, level / 4, 3, APPLY_DAMROLL, AFF_SANCTUARY, 0);
    aff_apply(victim, SPELL_POWER_OF_DEVOTION, level / 4, 25, APPLY_HP_REGEN, AFF_SANCTUARY, 0);
    aff_apply(victim, SPELL_POWER_OF_DEVOTION, level / 4, 5, APPLY_MANA_REGEN, AFF_SANCTUARY, 0);
    aff_apply(victim, SPELL_POWER_OF_DEVOTION, level / 4, -1, APPLY_SAVING_ALL, AFF_SANCTUARY, 0);
  }
}

void cast_power_of_faith(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim == ch) {
        act("The power of your faith heals your body.", FALSE, ch, 0, 0, TO_CHAR);
        act("The power of $n's faith heals $s body.", FALSE, ch, 0, 0, TO_ROOM);
      }
      else {
        act("The power of your faith heals $N's body.", FALSE, ch, 0, victim, TO_CHAR);
        act("The power of $n's faith heals your body.", FALSE, ch, 0, victim, TO_VICT);
        act("The power of $n's faith heals $N's body.", FALSE, ch, 0, victim, TO_NOTVICT);
      }

      spell_power_of_faith(level, ch, victim, 0);
      break;

    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_power_of_faith(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_power_of_faith, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_power_of_faith(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  const int MIN_MANA = 0, MAX_MANA = 75, MANA_STEP = 1;
  const int MIN_HEAL = 375, MAX_HEAL = 1500, HEAL_STEP = 15;

  int heal = MIN_HEAL, mana = MIN_MANA;

  while ((heal < (GET_MAX_HIT(victim) - GET_HIT(victim))) && (heal < MAX_HEAL)) {
    heal += HEAL_STEP;
    mana += MANA_STEP;
  }

  GET_MANA(ch) -= MIN(mana, MAX_MANA);

  magic_heal(ch, victim, SPELL_POWER_OF_FAITH, heal, FALSE);

  /* Crusader SC2: Focus */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_CRUSADER, 2)) {
    GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch) + 10, 1000);
  }
}

void cast_devastation( ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_devastation(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_devastation, level, 0);
      break;
  }
}

void spell_devastation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage_spell(ch, victim, SPELL_DEVASTATION, level, 1000, DAM_MAGICAL, SAVING_NONE);

  if (SAME_ROOM(ch, victim)) {
    act("$n chases $s devastating spell with a deadly strike.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("Still reeling from the spell, you are caught off guard as $n strikes at you!", FALSE, ch, 0, victim, TO_VICT);
    act("You chase your devastating spell with a deadly strike.", FALSE, ch, 0, victim, TO_CHAR);
    hit(ch, victim, TYPE_UNDEFINED);
    /* Only apply Exposed if the victim is still alive/active. */
    if (GET_POS(victim) > POSITION_DEAD) {
      /* Apply or refresh Exposed enchant: 10 + INT bonus rounds.
         If already exposed with equal/longer duration, leave it alone. */
      int new_dur = 10 + GET_INT_BONUS(ch);
      int existing = ench_duration(victim, ENCH_NAME_EXPOSED, 0);

      if (existing < new_dur) {
        ench_apply(victim, TRUE, ENCH_NAME_EXPOSED, 0, new_dur, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, exposed_enchant);
      }
    }
  }
}

/* Exposed: Invul Breakthrough enchant handler */
int exposed_enchant(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (cmd == MSG_SHOW_AFFECT_TEXT) {
    if (!ench || !ch || !signaler) return FALSE;
    act("......$n is exposed!", FALSE, ch, 0, signaler, TO_VICT);
    return FALSE;
  }

  if (cmd == MSG_ROUND) {
    /* No per-round action needed for the breakthrough enchant itself. */
    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH) {
    if (!ench || !ch) return FALSE;
    act("$n is no longer exposed.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You feel invulnerable once more.\n\r", ch);
    return FALSE;
  }
  return FALSE;
}

void cast_incendiary_cloud(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_incendiary_cloud(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_incendiary_cloud, level, 0);
      break;
  }
}

int incendiary_cloud_enchant(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (cmd == MSG_REMOVE_ENCH) {
    if (!ench || !ch) return FALSE;

    print_spell_wear_off_message(ch, SPELL_INCENDIARY_CLOUD);

    return FALSE;
  }

  if (cmd == MSG_SHOW_AFFECT_TEXT) {
    if (!ench || !ch || !signaler) return FALSE;

    act("......$n is enveloped by a huge ball of flame!", FALSE, ch, 0, signaler, TO_VICT);

    return FALSE;
  }

  if (cmd == MSG_ROUND) {
    if (!ench || !ch) return FALSE;

    send_to_char("The cloud of fire enveloping you burns you to the core...\n\r", ch);
    act("The cloud of fire enveloping $n burns $m to the core...", FALSE, ch, 0, 0, TO_ROOM);

    int base = 150;
    int int_bonus = 0;

    /* If the caster stored their INT bonus on the enchant, use it to scale damage. */
    if (ench) int_bonus = ench->temp[0];

    int dmg = base + (int_bonus * 5); /* +5 damage per INT bonus point */

    /* Don't kill the character, otherwise EXP is lost. */
    if (GET_HIT(ch) <= dmg) {
      dmg = GET_HIT(ch) - 1;
    }

    int set_pos = POSITION_STANDING;

    /* Don't consume position during combat. */
    if (((GET_POS(ch) == POSITION_FIGHTING) || (GET_POS(ch) <= POSITION_STUNNED)) && (GET_OPPONENT(ch) && SAME_ROOM(ch, GET_OPPONENT(ch)))) {
      set_pos = GET_POS(ch);
    }

    damage(ch, ch, dmg, TYPE_UNDEFINED, DAM_FIRE);

    GET_POS(ch) = MIN(GET_POS(ch), set_pos);

    return FALSE;
  }

  return FALSE;
}

void spell_incendiary_cloud(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  /* Double the initial damage if the target was already affected. */
  if (ench_enchanted_by(victim, ENCH_NAME_INCENDIARY_CLOUD, 0)) {
    damage_spell(ch, victim, SPELL_INCENDIARY_CLOUD, level, 1000, DAM_FIRE, SAVING_NONE);
  }
  else {
    damage_spell(ch, victim, SPELL_INCENDIARY_CLOUD, level, 500, DAM_FIRE, SAVING_NONE);
  }

  ench_apply(victim, TRUE, ENCH_NAME_INCENDIARY_CLOUD, SPELL_INCENDIARY_CLOUD, 10, ENCH_INTERVAL_ROUND, -4, APPLY_HITROLL, AFF_BLIND, 0, incendiary_cloud_enchant);

  /* Record the caster's INT bonus on the enchant so the enchant handler can use it each round. */
  ENCH *e = ench_get_from_char(victim, ENCH_NAME_INCENDIARY_CLOUD, 0);
  if (e) {
    e->temp[0] = GET_INT_BONUS(ch);
  }
}

void cast_tremor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_tremor(level, ch, 0, 0);
      break;
  }
}

void spell_tremor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, 0, NO_CAST_SAFE_ROOM)) return;

  send_to_room("The ground begins to shake and heave.\n\r", CHAR_REAL_ROOM(ch));

  for (CHAR *temp_vict = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_vict; temp_vict; temp_vict = next_vict) {
    next_vict = temp_vict->next_in_room;

    if ((temp_vict == ch) || IS_IMMORTAL(temp_vict) || (IS_NPC(ch) && IS_NPC(temp_vict))) continue;

    if (IS_NPC(temp_vict) || ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_vict))) {
      damage(ch, temp_vict, 400, TYPE_UNDEFINED, DAM_MAGICAL);

      if (CHAR_REAL_ROOM(temp_vict) != NOWHERE) {
        GET_POS(temp_vict) = POSITION_RESTING;

        ench_apply(temp_vict, TRUE, ENCH_NAME_TREMOR, SPELL_TREMOR, 10, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, 0);
      }
    }
  }
}

void cast_blur(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_blur(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_blur, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_blur(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_BLUR)) {
    print_spell_messages(victim, SPELL_BLUR);

    aff_apply(victim, SPELL_BLUR, level / 5, 0, 0, 0, 0);
  }
}

void cast_tranquility(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_tranquility(level, ch, 0, 0);
      break;
  }
}

void spell_tranquility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  const int tranquility_duration = 6;

  for (CHAR *temp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_ch; temp_ch; temp_ch = next_ch) {
    next_ch = temp_ch->next_in_room;

    if (SAME_GROUP_EX(temp_ch, ch, TRUE)) {
      print_spell_messages(temp_ch, SPELL_TRANQUILITY);

      aff_from_char(temp_ch, SPELL_TRANQUILITY);
      aff_apply(temp_ch, SPELL_TRANQUILITY, tranquility_duration, 0, 0, 0, 0);
      apply_tranquility_inner_peace(temp_ch, ch, tranquility_duration);
    }
  }
}

int wither_enchantment(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (cmd == MSG_SHOW_AFFECT_TEXT) {
    if (!ench || !ch || !signaler) return FALSE;

    act("......$n writhes and shudders as $s body is wracked with pain!", FALSE, ch, 0, signaler, TO_VICT);

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH) {
    if (!ench || !ch) return FALSE;
    print_spell_wear_off_message(ch, SPELL_WITHER);

    return FALSE;
  }

  if (cmd == MSG_ROUND) {
    if (!ench || !ch) return FALSE;

    const int wither_affect_types[] = {
    SPELL_BLINDNESS,
    SPELL_CURSE,
    SPELL_PARALYSIS,
    SPELL_POISON
    };

    const int wither_pulse_damage = 75;
    const int wither_effect_chance = 25;
    const int wither_effect_reduction = 5;

    /* Pulse every 3 rounds. */
    if (ench->duration % 3 != 0) return FALSE;

    send_to_char("You shudder as your body is wracked with pain!\n\r", ch);
    act("$n shudders as $s body is wracked with pain!", TRUE, ch, 0, 0, TO_ROOM);

    int dam = wither_pulse_damage;

    int caster_level = MIN(MAX(ench->temp[0], 45), LEVEL_MORT);

    int num_applied_effects = 0;

    for (int i = 0; i < NUMELEMS(wither_affect_types) - 1; i++) {
      if (aff_affected_by(ch, wither_affect_types[i])) {
        num_applied_effects++;
      }
    }

    int effect_chance = MAX(0, (wither_effect_chance - (num_applied_effects * wither_effect_reduction)));

    if (chance(effect_chance)) {
      int effect_type = get_random_eligible_affect(ch, wither_affect_types, NUMELEMS(wither_affect_types));

      if (effect_type > 0) {
        switch (effect_type) {
          case SPELL_BLINDNESS:
            if (IS_NPC(ch) && IS_IMMUNE(ch, IMMUNE_BLINDNESS)) {
              dam += 50;
            }
            else {
              int duration = ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 1 : 2;

              aff_apply(ch, SPELL_BLINDNESS, duration, 0, 0, AFF_BLIND, 0);
              aff_apply(ch, SPELL_BLINDNESS, duration, -4, APPLY_HITROLL, 0, 0);
              aff_apply(ch, SPELL_BLINDNESS, duration, 40, APPLY_ARMOR, 0, 0);

              act("You have been blinded!", FALSE, ch, 0, 0, TO_CHAR);
              act("$n seems to be blinded!", TRUE, ch, 0, 0, TO_ROOM);
            }
            break;

          case SPELL_CURSE:
            if (IS_NPC(ch) && IS_IMMUNE2(ch, IMMUNE2_CURSE)) {
              dam += 50;
            }
            else {
              int duration = ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 1 : 2;

              aff_apply(ch, SPELL_CURSE, duration, 0, 0, AFF_CURSE, 0);
              aff_apply(ch, SPELL_CURSE, duration, -((caster_level - 3) / 9), APPLY_HITROLL, 0, 0);
              aff_apply(ch, SPELL_CURSE, duration, ((caster_level - 3) / 9), APPLY_SAVING_PARA, 0, 0);

              act("You feel very uncomfortable.", FALSE, ch, 0, 0, TO_CHAR);
              act("$n briefly reveals a red aura!", TRUE, ch, 0, 0, TO_ROOM);
            }
            break;

          case SPELL_PARALYSIS:
            if (IS_NPC(ch) && IS_IMMUNE(ch, IMMUNE_PARALYSIS)) {
              dam += 50;
            }
            else {
              int duration = ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 1 : caster_level;

              aff_apply(ch, SPELL_PARALYSIS, duration, 0, 0, AFF_PARALYSIS, 0);
              aff_apply(ch, SPELL_PARALYSIS, duration, -2, APPLY_HITROLL, 0, 0);
              aff_apply(ch, SPELL_PARALYSIS, duration, 20, APPLY_ARMOR, 0, 0);

              act("Your limbs freeze in place!", FALSE, ch, 0, 0, TO_CHAR);
              act("$n is paralyzed!", TRUE, ch, 0, 0, TO_ROOM);
            }
            break;

          case SPELL_POISON:
            if (IS_NPC(ch) && IS_IMMUNE(ch, IMMUNE_POISON)) {
              dam += 50;
            }
            else {
              int duration = ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 1 : caster_level;

              aff_apply(ch, SPELL_POISON, duration, -3, APPLY_STR, AFF_POISON, 0);

              act("You feel very sick.", FALSE, ch, 0, 0, TO_CHAR);
              act("$n looks very sick.", TRUE, ch, 0, 0, TO_ROOM);
            }
            break;
        }
      }
      else {
        dam += 100;
      }
    }

    dam = MIN(dam, GET_HIT(ch) - 1);

    /* Record the character's initial position. */
    int set_pos = GET_POS(ch);

    damage(ch, ch, dam, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

    /* Restore the character's initial position if it's less than the current position. */
    GET_POS(ch) = MIN(GET_POS(ch), set_pos);

    return FALSE;
  }

  return FALSE;
}

void cast_wither(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_wither(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_wither, level, 0);
      break;
  }
}

void spell_wither(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!spell_check_cast_ok(ch, victim, NO_CAST_SAFE_ROOM | NO_CAST_OTHER_PC_NOKILL_FLAG_ON)) return;

  damage(ch, victim, 350, SPELL_WITHER, DAM_MAGICAL);

  if (IS_ALIVE(victim)) {
    /* Duration is 1 less than "expected" because we want the last pulse to happen when duration is 0. */
    ench_apply(victim, TRUE, "Wither", SPELL_WITHER, 11, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, wither_enchantment);
  }
}

void cast_shadow_wraith(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_shadow_wraith(level, ch, victim, 0);
      break;
  }
}

void spell_shadow_wraith(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int num_shadows = 0;

  if (affected_by_spell(ch, SPELL_SHADOW_WRAITH)) {
    num_shadows = (MAX(1, (duration_of_spell(ch, SPELL_SHADOW_WRAITH) - 1)) / 10) + 1;
  }

  if ((num_shadows >= 4) || ((num_shadows >= 2) && (GET_SC_LEVEL(ch) < 4)) || ((num_shadows >= 3) && (GET_SC_LEVEL(ch) < 5))) {
    send_to_char("You are already controlling the maximum number of shadows you can maintain.\n\r", ch);

    return;
  }

  int extra_mana_cost = num_shadows * 20;

  if (GET_MANA(ch) < extra_mana_cost) {
    send_to_char("You can't summon enough energy to manifest another shadow.\n\r", ch);

    /* Refund base mana cost, since they couldn't muster the full cost of the spell. */
    GET_MANA(ch) += spell_info[SPELL_SHADOW_WRAITH].min_usesmana;

    return;
  }

  GET_MANA(ch) -= extra_mana_cost;

  /* Apply the affect. This has to be done the old-school way, due to using affect_join() below. */

  AFF aff = { 0 };

  aff.type = SPELL_SHADOW_WRAITH;
  aff.duration = 10;

  aff_join(ch, &aff, FALSE, FALSE);

  if (!aff_affected_by(ch, SPELL_SHADOW_WRAITH)) return;

  num_shadows = (MAX(1, (aff_duration(ch, SPELL_SHADOW_WRAITH) - 1)) / 10) + 1;

  switch (num_shadows) {
    case 1:
      send_to_char("Your shadow stretches to the north.\n\r", ch);
      act("$n's shadow stretches to the north.", TRUE, ch, 0, 0, TO_ROOM);
      break;

    case 2:
      send_to_char("Your shadow stretches to the south.\n\r", ch);
      act("$n's shadow stretches to the south.", TRUE, ch, 0, 0, TO_ROOM);
      break;

    case 3:
      send_to_char("Your shadow stretches to the east.\n\r", ch);
      act("$n's shadow stretches to the east.", TRUE, ch, 0, 0, TO_ROOM);
      break;

    case 4:
      send_to_char("Your shadow stretches to the west.\n\r", ch);
      act("$n's shadow stretches to the west.", TRUE, ch, 0, 0, TO_ROOM);
      break;
  }
}

void cast_dusk_requiem(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_dusk_requiem(level, ch, victim, 0);
      break;
  }
}

void spell_dusk_requiem(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!affected_by_spell(ch, SPELL_SHADOW_WRAITH)) {
    send_to_char("Your requiem fails to draw power from the fleeting shadows.\n\r", ch);
    act("$n's requiem fails to draw power from the fleeting shadows.", FALSE, ch, 0, 0, TO_ROOM);

    return;
  }

  int num_shadows = (MAX(1, (aff_duration(ch, SPELL_SHADOW_WRAITH) - 1)) / 10) + 1;

  int dam = MIN(350 * 4, 350 * num_shadows);

  /* If level <= 0, inflict double damage. */
  if (!level) {
    dam *= 2;
  }

  damage(ch, victim, dam, SPELL_DUSK_REQUIEM, DAM_MAGICAL);

  /* We only want to reduce duration if the spell was cast, not if Shadow Wraith is expiring. */
  if (level > 0) {
    for (AFF *tmp_af = ch->affected, *next_af; tmp_af; tmp_af = next_af) {
      next_af = tmp_af->next;

      if (tmp_af->type == SPELL_SHADOW_WRAITH) {
        if (num_shadows <= 1) {
          affect_from_char(ch, SPELL_SHADOW_WRAITH);
        }
        else {
          tmp_af->duration = (num_shadows - 1) * 10;
        }
        break;
      }
    }
  }
}

void cast_divine_hammer(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_divine_hammer(level, ch, victim, 0);
      break;
  }
}

int judgement_strike_enchantment(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (cmd == CMD_UNKNOWN) {
    if (!ench || !ch || (ch != signaler) || IS_NPC(ch)) return FALSE;

    char cmd[MIL] = { 0 }, name[MIL] = { 0 };

    argument_interpreter(arg, cmd, name);

    if (!(*cmd) || !is_abbrev(cmd, "judgement-strike")) return FALSE;

    CHAR *victim = NULL, *opponent = NULL;

    if (GET_OPPONENT(ch) && (IS_NPC(GET_OPPONENT(ch)) || (IS_MORTAL(GET_OPPONENT(ch)) && ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))) && SAME_ROOM(GET_OPPONENT(ch), ch)) {
      opponent = GET_OPPONENT(ch);
    }

    if (*name) {
      victim = get_char_room_vis(ch, name);

      if (!victim) {
        if (is_abbrev("area", name) || is_abbrev("room", name)) {
          victim = NULL;
        }
        else if (is_abbrev("opponent", name) || is_abbrev("victim", name)) {
          victim = opponent;
        }
        else if (is_abbrev("self", name) || is_abbrev("me", name)) {
          victim = ch;
        }
      }
    }
    else if (opponent) {
      victim = opponent;
    }

    if (!victim) {
      printf_to_char(ch, "The %s energy contained in your divine hammer envelops the area in %s light!\n\r",
        IS_GOOD(ch) ? "holy" : IS_EVIL(ch) ? "unholy" : "spiritual",
        IS_GOOD(ch) ? "radiant" : IS_EVIL(ch) ? "lambent" : "scintillating");

      printf_to_room_except(CHAR_REAL_ROOM(ch), ch, "The %s energy contained in %s's divine hammer envelops the area in %s light!\n\r",
        IS_GOOD(ch) ? "holy" : IS_EVIL(ch) ? "unholy" : "spiritual",
        GET_DISP_NAME(ch),
        IS_GOOD(ch) ? "radiant" : IS_EVIL(ch) ? "lambent" : "scintillating");

      for (CHAR *temp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_ch = NULL; temp_ch; temp_ch = next_ch) {
        next_ch = temp_ch->next_in_room;

        if ((temp_ch == ch) || ((temp_ch != GET_OPPONENT(ch)) && IS_MORTAL(temp_ch) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))) {
          send_to_char("Divine energy restores some of your health.\n\r", temp_ch);

          magic_heal(ch, temp_ch, SPELL_DIVINE_HAMMER, 450, FALSE);
        }
      }
    }
    else if ((victim == ch) || ((victim != GET_OPPONENT(ch)) && IS_MORTAL(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))) {
      if (victim == ch) {
        act("$n channels the energy from $s divine hammer into $s soul, healing $mself.", FALSE, ch, 0, victim, TO_ROOM);
        act("You channel the energy from your divine hammer into your soul, healing yourself.", FALSE, ch, 0, victim, TO_CHAR);
      }
      else {
        act("$n channels the energy from $s divine hammer into $N's soul, healing $M!", FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n channels the energy from $s divine hammer into your soul, healing you!", FALSE, ch, 0, victim, TO_VICT);
        act("You channel the energy from your divine hammer into $N's soul, healing $M!", FALSE, ch, 0, victim, TO_CHAR);
      }

      magic_heal(ch, victim, SPELL_DIVINE_HAMMER, 900, FALSE);
    }
    else {
      if (!IS_IMMORTAL(ch) && IS_IMMORTAL(victim)) {
        send_to_char("Your god is offended by your request to harm another immortal!\n\r", ch);

        damage(ch, ch, 100, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

        return TRUE;
      }

      act("$n smites $N with $s divine hammer, crushing $M into the earth!", FALSE, ch, 0, victim, TO_NOTVICT);
      act("$n smites you with $s divine hammer, crushing you into the earth!", FALSE, ch, 0, victim, TO_VICT);
      act("You smite $N with your divine hammer, crushing $M into the earth!", FALSE, ch, 0, victim, TO_CHAR);

      damage(ch, victim, 900, TYPE_UNDEFINED, DAM_MAGICAL);
    }

    ench_from_char(ch, ENCH_NAME_JUDGEMENT_STRIKE, 0, FALSE);

    return TRUE;
  }

  if (cmd == MSG_ROUND) {
    if (!ench || !ch) return FALSE;

    switch (ench->duration) {
      case 3:
        send_to_char("The energy within your divine hammer ebbs as it begins to fade.\n\r", ch);
        break;

      case 2:
        send_to_char("The energy within your divine hammer diminishes further.\n\r", ch);
        break;

      case 1:
        send_to_char("The energy within your divine hammer is nearly exhausted.\n\r", ch);
        break;

      default:
        if (ench->duration % 5 == 0) {
          send_to_char("You are reminded of the great power of your divine hammer as it looms above you.\n\r", ch);
        }
        break;
    }

    return FALSE;
  }

  if (cmd == MSG_REMOVE_ENCH) {
    if (!ench || !ch) return FALSE;

    act("$n's divine hammer fades, restoring some of $s energy.", TRUE, ch, 0, 0, TO_ROOM);
    act("Your divine hammer fades, restoring some of your energy.", FALSE, ch, 0, 0, TO_CHAR);

    magic_heal(ch, ch, SPELL_DIVINE_HAMMER, 300, FALSE);

    GET_MANA(ch) = MIN(GET_MANA(ch) + 50, GET_MAX_MANA(ch));

    return FALSE;
  }

  return FALSE;
}

int divine_hammer_enchantment(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  struct attack_data_t {
    int attack_type;
    int immunity;
  };

  const struct attack_data_t hammer_attack_data[] = {
    { TYPE_CRUSH, IMMUNE_CRUSH },
    { TYPE_HACK, IMMUNE_HACK },
    { TYPE_PIERCE, IMMUNE_PIERCE }
  };

  if (cmd == MSG_VIOLENCE) {
    if (!ench || !ch || !signaler || (ch == signaler)) return FALSE;

    static OBJ *hammer = NULL;

    if (!hammer) {
      CREATE(hammer, OBJ, 1);

      clear_object(hammer);

      OBJ_GET_NAME(hammer) = strdup("hammer");
      OBJ_GET_SHORT(hammer) = strdup("a divine hammer");
      OBJ_TYPE(hammer) = ITEM_WEAPON;
    }

    int num_dice = 5;
    int size_dice = 5;
    int dam_bonus = 5;

    if ((ench->duration == 40) || (ench->duration == 20)) {
      num_dice += 1;
      size_dice += 1;
      dam_bonus += 1;
    }

    int attack_type = TYPE_UNDEFINED;

    for (int i = 0; (attack_type <= 0) && (i < NUMELEMS(hammer_attack_data)); i++) {
      if (!IS_IMMUNE(signaler, hammer_attack_data[i].immunity)) {
        attack_type = hammer_attack_data[i].attack_type;
      }
    }

    OBJ_VALUE(hammer, 1) = num_dice;
    OBJ_VALUE(hammer, 2) = size_dice;
    OBJ_VALUE(hammer, 3) = attack_type;

    int dam = calc_hit_damage(ch, GET_OPPONENT(ch), hammer, dam_bonus, RND_RND);

    OBJ *temp_weapon = EQ(ch, WIELD);

    EQ(ch, WIELD) = hammer;

    if (attack_type == TYPE_UNDEFINED) {
      act("$n smashes $N with a godly blow from $s divine hammer!", FALSE, ch, 0, signaler, TO_NOTVICT);
      act("$n smashes you with a godly blow from $s divine hammer!", FALSE, ch, 0, signaler, TO_VICT);
      act("You smash $N with a godly blow from your divine hammer!", FALSE, ch, 0, signaler, TO_CHAR);
    }

    damage(ch, GET_OPPONENT(ch), dam, attack_type, DAM_PHYSICAL);

    EQ(ch, WIELD) = temp_weapon;

    ench->duration--;

    if (ench->duration <= 0) {
      ench_remove(ch, ench, FALSE);

      act("$n's divine hammer rises into the sky, fully charged for one final strike!", TRUE, ch, 0, 0, TO_ROOM);
      act("Your divine hammer rises into the sky, fully charged for one final strike!", FALSE, ch, 0, 0, TO_CHAR);

      ench_apply(ch, FALSE, ENCH_NAME_JUDGEMENT_STRIKE, 0, 20, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, judgement_strike_enchantment);
    }
    else if (ench->duration == 40) {
      act("$n's divine hammer glows bright, bathing $m in holy light.", TRUE, ch, 0, 0, TO_ROOM);
      act("Your divine hammer glows bright, bathing you in holy light.", FALSE, ch, 0, 0, TO_CHAR);
    }
    else if (ench->duration == 20) {
      act("$n's divine hammer glows brilliantly, showering $e with motes of mystic energy.", TRUE, ch, 0, 0, TO_ROOM);
      act("Your divine hammer glows brilliantly, showering you with motes of mystic energy.", FALSE, ch, 0, 0, TO_CHAR);
    }

    return FALSE;
  }

  if (cmd == CMD_UNKNOWN) {
    if (!ench || !ch || (ch != signaler)) return FALSE;

    char cmd[MIL], name[MIL];

    argument_interpreter(arg, cmd, name);

    if (!(*cmd) || !is_abbrev(cmd, "judgement-strike")) return FALSE;

    printf_to_char(ch, "Your divine hammer thrums with %s energy, but is not yet fully charged.\n\r",
      IS_GOOD(ch) ? "holy" : IS_EVIL(ch) ? "unholy" : "spiritual");

    return TRUE;
  }

  return FALSE;
}

void spell_divine_hammer(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!ench_enchanted_by(victim, ENCH_NAME_DIVINE_HAMMER, 0)) {
    print_spell_messages(victim, SPELL_DIVINE_HAMMER);

    ench_apply(victim, FALSE, ENCH_NAME_DIVINE_HAMMER, 0, 60, ENCH_INTERVAL_USER, 0, 0, 0, 0, divine_hammer_enchantment);
  }
}
