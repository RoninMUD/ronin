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
  if (SAME_GROUP_EX(victim, ch, TRUE)) {
    if (!aff_affected_by(victim, SPELL_WAR_CHANT)) {
      print_spell_messages(victim, SPELL_WAR_CHANT);

      aff_apply(victim, SPELL_WAR_CHANT, 5, -2, APPLY_SAVING_ALL, 0, 0);
    }
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

void cast_blade_barrier(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_blade_barrier(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_blade_barrier, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_blade_barrier(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_BLADE_BARRIER)) {
    print_spell_messages(victim, SPELL_BLADE_BARRIER);

    aff_apply(victim, SPELL_BLADE_BARRIER, 5, 0, 0, 0, 0);
  }
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

    aff_apply(victim, SPELL_IRON_SKIN, 10, ch == victim ? -20 : -10, APPLY_ARMOR, 0, 0);
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

  damage_spell(ch, victim, SPELL_METEOR, level, number(40, 60), DAM_MAGICAL, SAVING_NONE);
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
          !IS_AFFECTED(temp_vict, AFF_PARALYSIS) &&
          ((GET_LEVEL(ch) + 10) >= GET_LEVEL(temp_vict)) &&
          !saves_spell(temp_vict, SAVING_PARA, (level + 10))) {
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
      spell_desecrate(level, ch, victim, 0);
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

void cast_engage(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_engage(level, ch, victim, 0);
      break;
  }
}

void spell_engage(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  int wpn_extra(OBJ *wielded, CHAR *ch, CHAR *victim, int mode);

  int dam, w_type, str_index;
  OBJ *wielded = NULL;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }
  
  if (!(wielded=ch->equipment[WIELD]))
  {
    send_to_char("You must wield a weapon.\n\r", ch);
    return;
  }

  switch (wielded->obj_flags.value[3])
  {
    case 0 :
    case 1 :
    case 2 : w_type = TYPE_WHIP; break;
    case 3 : w_type = TYPE_SLASH; break;
    case 4 : w_type = TYPE_WHIP; break;
    case 5 : w_type = TYPE_STING; break;
    case 6 : w_type = TYPE_CRUSH; break;
    case 7 : w_type = TYPE_BLUDGEON; break;
    case 8 : w_type = TYPE_CLAW; break;
    case 9 :
    case 10:
    case 11: w_type = TYPE_PIERCE; break;
    case 12: w_type = TYPE_HACK; break;
    case 13: w_type = TYPE_CHOP; break;
    case 14: w_type = TYPE_SLICE; break;
    default: w_type = TYPE_HIT; break;
  }

  /* magic weapon */
  if((ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) || IS_NPC(victim)) &&
     (wielded->obj_flags.value[0] != 0) && 
     (wielded->obj_flags.value[0] <= 20) && 
     !number(0,2)) {
    switch((int)wielded->obj_flags.value[0]) {
      case 1:
        spell_blindness(30, ch, victim, 0);
        break;
      case 2:
        if(!(IS_NPC(ch))) break;
        spell_poison(30, ch, victim, 0);
        break;
      case 3:
        spell_vampiric_touch(30, ch, victim, 0);
        break;
      case 4:
        spell_chill_touch(30, ch, victim, 0);
        break;
      case 5:
        spell_forget(30, ch, victim, 0);
        break;
      case 6:
        spell_curse(30, ch, victim, 0);
        break;
      case 7:
      case 8:
      default:
        break;
    }
  }
  act("Your $p leaps from your hand and attacks $N before returning",0,ch,wielded,victim,TO_CHAR);
  act("$n's $p leaps from $s hand and attacks $N before returning",0,ch,wielded,victim,TO_NOTVICT);
  act("$n's $p leaps from $s hand and attacks you before returning",0,ch,wielded,victim,TO_VICT);

  str_index=MIN(STRENGTH_APPLY_INDEX(ch),OSTRENGTH_APPLY_INDEX(ch));
  dam = str_app[str_index].todam;
  if (wielded->obj_flags.type_flag == ITEM_2H_WEAPON) dam = (dam*3)/2;
  dam += GET_DAMROLL(ch);
  dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
  dam += wpn_extra(wielded,ch,victim,RND_RND);

  if(GET_POS(victim) < POSITION_FIGHTING)
    dam *= 1+(POSITION_FIGHTING-GET_POS(victim))/3;
  dam = MAX(1, dam);  /* Not less than 0 damage */

  dam*=5; /* 5x damage for engage */

  if(affected_by_spell(ch, SKILL_DEFEND))
    dam=1;

  damage(ch, victim, dam, w_type,DAM_PHYSICAL);
}

void cast_ethereal_nature(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj) {
  switch (type) {
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
      spell_ethereal_nature(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      aoe_spell(ch, spell_ethereal_nature, level, AOE_TAR_ALL_CHARS);
      break;
  }
}

void spell_ethereal_nature(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!aff_affected_by(victim, SPELL_ETHEREAL_NATURE)) {
    print_spell_messages(victim, SPELL_ETHEREAL_NATURE);

    aff_apply(victim, SPELL_ETHEREAL_NATURE, 30, 0, 0, 0, 0);
  }
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

  if (!aff_affected_by(victim, SPELL_DISRUPT_SANCT) && IS_AFFECTED(victim, AFF_SANCTUARY)) {
    print_spell_messages(victim, SPELL_DISRUPT_SANCT);

    aff_apply(victim, SPELL_DISRUPT_SANCT, 0, 0, 0, 0, 0);
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
    case SPELL_TYPE_POTION:
      victim = ch;
    case SPELL_TYPE_SPELL:
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

  if (victim == ch) {
    act("The power of your faith heals your body.", FALSE, ch, 0, 0, TO_CHAR);
    act("The power of $n's faith heals $s body.", FALSE, ch, 0, 0, TO_ROOM);
  }
  else {
    act("The power of your faith heals $N's body.", FALSE, ch, 0, victim, TO_CHAR);
    act("The power of $n's faith heals your body.", FALSE, ch, 0, victim, TO_VICT);
    act("The power of $n's faith heals $N's body.", FALSE, ch, 0, victim, TO_NOTVICT);
  }

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

    int dmg = 150;

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

  ench_apply(victim, TRUE, ENCH_NAME_INCENDIARY_CLOUD, SPELL_INCENDIARY_CLOUD, 10, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, incendiary_cloud_enchant);

  damage_spell(ch, victim, SPELL_INCENDIARY_CLOUD, level, 500, DAM_FIRE, SAVING_NONE);
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
  for (CHAR *temp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_ch; temp_ch; temp_ch = next_ch) {
    next_ch = temp_ch->next_in_room;

    if (SAME_GROUP_EX(temp_ch, ch, TRUE)) {
      print_spell_messages(temp_ch, SPELL_TRANQUILITY);

      aff_from_char(temp_ch, SPELL_TRANQUILITY);

      aff_apply(temp_ch, SPELL_TRANQUILITY, 6, 3, APPLY_HITROLL, 0, 0);
      aff_apply(temp_ch, SPELL_TRANQUILITY, 6, 3, APPLY_DAMROLL, 0, 0);
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

    printf_to_char(ch, "DEBUG :: ench->duration = %d\n\r", ench->duration);

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
