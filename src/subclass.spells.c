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

void cast_rally(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_luck(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in rally!");
      break;
  }
}

void spell_rally(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && victim != ch)
  {
    send_to_char("You cannot cast this spell on another player.\n\r",ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_RALLY))
  {
    send_to_char("You feel rallied!\n\r", victim);

    af.type       = SPELL_RALLY;
    af.duration   = 5;
    af.modifier   = 5;
    af.location   = APPLY_HITROLL;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void cast_luck(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_luck(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_luck(level, ch, ch, 0);
      break;
    default:
      log_f("Wrong type called in luck!");
      break;
  }
}

void spell_luck(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r",ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_LUCK))
  {
    send_to_char("You feel lucky!\n\r", victim);

    af.type       = SPELL_LUCK;
    af.duration   = 5;
    af.modifier   = 0;
    af.location   = 0;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void cast_warchant(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_luck(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in warchant!");
      break;
  }
}

void spell_warchant(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;
  CHAR *leader = NULL;

  if (!affected_by_spell(victim, SPELL_WARCHANT))
  {
    af.type       = SPELL_WARCHANT;
    af.duration   = 5;
    af.bitvector  = 0;
    af.bitvector2 = 0;

    if (!IS_AFFECTED(ch, AFF_GROUP) || !ch->master)
      leader = ch;
    else
      leader = ch->master;

    if (victim == leader || victim->master == leader)
    {
      send_to_char("You feel safer!\n\r", victim);

      af.modifier  = -2;
      af.location  = APPLY_SAVING_PARA;
      affect_to_char(victim, &af);
      af.location  = APPLY_SAVING_ROD;
      affect_to_char(victim, &af);
      af.location  = APPLY_SAVING_PETRI;
      affect_to_char(victim, &af);
      af.location  = APPLY_SAVING_BREATH;
      affect_to_char(victim, &af);
      af.location  = APPLY_SAVING_SPELL;
      affect_to_char(victim, &af);
    }
    else
    {
      send_to_char("You grow weak with panic!\n\r", victim);
      act("$n grows weak with panic!", FALSE, victim, 0, 0, TO_ROOM);

      if (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
        af.modifier = -3;
      else
        af.modifier = -4;
      af.location  = APPLY_HITROLL;
      affect_to_char(victim, &af);
    }
  }
}

void cast_camaraderie(ubyte level, CHAR* ch, char* arg, int type, CHAR* victim, OBJ* tar_obj)
{
    switch (type)
    {
    case SPELL_TYPE_SPELL:
        spell_camaraderie(level, ch, victim, 0);
        break;
    default:
        log_f("Wrong type called in camaraderie!");
        break;
    }
}

void spell_camaraderie(ubyte level, CHAR* ch, CHAR* victim, OBJ* obj)
{
    AFF af;

    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch != victim)
    {
        send_to_char("You cannot cast this spell on another player.\n\r", ch);
        return;
    }

    if (!affected_by_spell(victim, SPELL_CAMARADERIE))
    {
        send_to_char("You feel safety in numbers.\n\r", victim);

        af.type = SPELL_CAMARADERIE;
        af.duration = 5;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = 0;
        af.bitvector2 = 0;
        affect_to_char(victim, &af);
    }
}

void cast_cloud_confusion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cloud_confusion(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong 'type' called in cast_cloud_confusion!");
      break;
  }
}

void spell_cloud_confusion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  CHAR *leader = NULL;

  if (!IS_AFFECTED(ch, AFF_GROUP) || !GET_MASTER(ch)) {
    leader = ch;
  }
  else {
    leader = GET_MASTER(ch);
  }

  send_to_char("You whirl around, engulfing the area with a thick black cloud.\n\r", ch);
  act("$n whirls around, engulfing the area with a thick black cloud.", FALSE, ch, 0, 0, TO_ROOM);

  for (CHAR *temp_vict = world[CHAR_REAL_ROOM(ch)].people, *next_vict; temp_vict; temp_vict = next_vict) {
    next_vict = temp_vict->next_in_room;

    if (IS_IMMORTAL(temp_vict) || (temp_vict == leader) || (GET_MASTER(temp_vict) == leader) || affected_by_spell(temp_vict, SPELL_CLOUD_CONFUSION)) continue;

    send_to_char("You feel disoriented.\n\r", temp_vict);

    affect_apply(temp_vict, SPELL_CLOUD_CONFUSION, (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 1 : (dice(1, 5) + 1)), (IS_NIGHT ? -(number(6, 9)) : -(number(5, 8))), APPLY_HITROLL, 0, 0);
  }
}

void cast_rage(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  if (type == SPELL_TYPE_SPELL) {
    spell_rage(level, ch, ch, 0);

    if (!IS_NPC(ch) && (GET_CLASS(ch) == CLASS_ANTI_PALADIN)) {
      spell_blood_lust(level, ch, ch, 0);
    }
  }
  else {
    log_f("Wrong 'type' called in cast_rage!");
  }
}

void spell_rage(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!affected_by_spell(victim, SPELL_RAGE)) {
    send_to_char("Rage courses through your body!\n\r", victim);
    act("Rage courses through $n's body!", FALSE, victim, 0, 0, TO_ROOM);

    affect_apply(victim, SPELL_RAGE, 5, 0, 0, 0, AFF2_RAGE);
  }
}

void cast_righteousness(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_righteousness(level, ch, ch, 0);
      break;
    default:
      log_f("Wrong 'type' called in cast_righteousness!");
      break;
  }
}

void spell_righteousness(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!affected_by_spell(victim, SPELL_RIGHTEOUSNESS)) {
    send_to_char("You feel righteous!\n\r", victim);

    affect_apply(victim, SPELL_RIGHTEOUSNESS, 10, 0, 0, 0, 0);
  }
}

void cast_debilitate(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_debilitate(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in debilitate!");
      break;
  }
}

void spell_debilitate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_DEBILITATE))
  {
    send_to_char("You are enveloped by a greenish smoke - you feel weaker.\n\r", victim);
    act("$n is enveloped by a greenish smoke.", FALSE, victim, 0, 0, TO_ROOM);

    af.type       = SPELL_DEBILITATE;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 1;
    else
      af.duration = 5;
    af.modifier   = -5;
    af.location   = APPLY_HITROLL;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
    af.modifier   = 10;
    af.location   = APPLY_AC;
    affect_to_char(victim, &af);
  }
}

void cast_wrath_ancients(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_wrath_ancients(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in wrath of ancients!");
      break;
  }
}

void spell_wrath_ancients(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  damage(ch, victim, number(1800, 2000), SPELL_WRATH_ANCIENTS, DAM_MAGICAL);
}

void cast_might(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_might(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong 'type' called in cast_might!");
      break;
  }
}

void spell_might(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!affected_by_spell(ch, SPELL_MIGHT)) {
    send_to_char("You feel more powerful.\n\r", ch);
    act("$n looks more powerful.", TRUE, ch, 0, 0, TO_ROOM);

    affect_apply(ch, SPELL_MIGHT, (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 10 : 20), 1, APPLY_HITROLL, 0, 0);
    affect_apply(ch, SPELL_MIGHT, (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 10 : 20), 3, APPLY_DAMROLL, 0, 0);
  }
}

void cast_clarity(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_clarity(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in clarity!");
      break;
  }
}

void spell_clarity(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_CLARITY))
  {
    send_to_char("You feel a sense of precision!\n\r", ch);

    af.type       = SPELL_CLARITY;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 5;
    else
      af.duration = 10;
    af.modifier   = 0;
    af.location   = 0;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_blade_barrier(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_blade_barrier(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in blade barrier!");
      break;
  }
}

void spell_blade_barrier(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_BLADE_BARRIER))
  {
    send_to_char("You summon thousands of tiny whirling blades to envelope you!\n\r", ch);
    act("The air hums as thousands of tiny whirling blades appear and envelope $n!", TRUE, ch, 0, 0, TO_ROOM);

    af.type       = SPELL_BLADE_BARRIER;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 2;
    else
      af.duration = 5;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_distortion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_distortion(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in distortion!");
      break;
  }
}

void spell_distortion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_DISTORTION) && IS_AFFECTED(victim, AFF_SPHERE))
  {
    send_to_char("Your sphere appears to blur and distort.\n\r", victim);
    act("$n's sphere appears to blur and distort.", FALSE, victim, 0, 0, TO_ROOM);

    af.type       = SPELL_DISTORTION;
    af.duration   = 2;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void cast_iron_skin(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim) {
        spell_iron_skin(level, ch, victim, 0);
      }
      break;
    default:
      log_f("Wrong 'type' called in cast_iron_skin!");
      break;
  }
}

void spell_iron_skin(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!affected_by_spell(victim, SPELL_IRON_SKIN)) {
    send_to_char("You feel your skin harden.\n\r", victim);
    act("$n's skin hardens and turns dark iron in color.", TRUE, victim, 0, 0, TO_ROOM);

    affect_apply(victim, SPELL_IRON_SKIN, (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 5 : 10), ((ch == victim) ? -20 : -10), APPLY_AC, 0, 0);
  }
}

void cast_frostbolt(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim) {
        spell_frostbolt(level, ch, victim, 0);
      }
      break;
    default:
      log_f("Wrong 'type' called in cast_frostbolt!");
      break;
  }
}

void spell_frostbolt(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);

    return;
  }

  GET_MOVE(victim) = MAX(GET_MOVE(victim) - 100, 0);

  damage(ch, victim, 350, SPELL_FROSTBOLT, DAM_COLD);
}

void cast_orb_protection(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_orb_protection(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in orb of protection!");
      break;
  }
}

void spell_orb_protection(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_ORB_PROTECTION))
  {
    send_to_char("A golden orb appears over your head.", ch);
    act("A golden orb appears over $n's head.", TRUE, ch, 0, 0, TO_ROOM);

    af.type       = SPELL_ORB_PROTECTION;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 2;
    else
      af.duration = 10;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_sanctify(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_sanctify(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in sanctify!");
      break;
  }
}

#define STATUE_PEACE 9
void spell_sanctify(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  OBJ *tmp = NULL;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_MOB))
  {
    send_to_char("This area is already safe to rest in.\n\r", ch);
    return;
  }

  if (count_mobs_real_room_except_followers(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("This area can't be made safe, there is a mob here!\n\r", ch);
    return;
  }

  SET_BIT(world[CHAR_REAL_ROOM(ch)].room_flags, NO_MOB);

  tmp = read_object(STATUE_PEACE, VIRTUAL);

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
    tmp->spec_value = 1;
  else
    tmp->spec_value = 10; /* 10 ticks of safety. */

  obj_to_room(tmp, CHAR_REAL_ROOM(ch));

  send_to_room("A statue of peace appears before your eyes!\n\r", CHAR_REAL_ROOM(ch));
}

void cast_meteor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
      if (victim)
        spell_meteor(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in meteor!");
      break;
  }
}

void spell_meteor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  damage(ch, victim, number(40, 60), SPELL_METEOR, DAM_MAGICAL);
}

void cast_wall_thorns(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_wall_thorns(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in wall of thorns!");
      break;
  }
}

#define WALL_THORNS 34
void spell_wall_thorns(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  OBJ *tmp = NULL;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if ((tmp = get_obj_room(WALL_THORNS, CHAR_VIRTUAL_ROOM(ch))))
  {
    send_to_room("The thick wall of brambles recedes, allowing passage to the surrounding world.\n\r", CHAR_REAL_ROOM(ch));
    extract_obj(tmp);
    return;
  }

  tmp = read_object(WALL_THORNS, VIRTUAL);

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
    tmp->spec_value = 1;
  else
    tmp->spec_value = 5; /* 5 ticks of safety. */

  obj_to_room(tmp, CHAR_REAL_ROOM(ch));

  send_to_room("A thick wall of razor sharp brambles bursts forth!\n\r", CHAR_REAL_ROOM(ch));
}

void cast_rejuvenation(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_rejuvenation(level, ch, victim, NULL);
      break;
    default:
      log_f("Wrong type called in rejuvenation!");
      break;
  }
}

void spell_rejuvenation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch != victim) {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);

    return;
  }

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      (duration_of_spell(victim, SPELL_DEGENERATE) > (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 9 : 27))) {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", victim);

    return;
  }

  magic_heal(victim, SPELL_REJUVENATION, 400, FALSE);

  send_to_char("You feel much better!\n\r", victim);

  update_pos(victim);
}

void cast_divine_wind(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_divine_wind(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in divine wind!");
      break;
  }
}

void spell_divine_wind(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int stack_position(CHAR *ch, int target_position);

  int set_pos = 0;

  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  set_pos = stack_position(victim, POSITION_RESTING);

  damage(ch, victim, 300, SPELL_DIVINE_WIND, DAM_MAGICAL);

  if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;
  }
}

void cast_blackmantle(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_blackmantle(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in blackmantle!");
      break;
  }
}

void spell_blackmantle(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_BLACKMANTLE))
  {
    send_to_char("You are surrounded by an eerie mantle of darkness.\n\r", ch);
    act("$n is surrounded by an eerie mantle of darkness.", TRUE, ch, 0, 0, TO_ROOM);

    af.type       = SPELL_BLACKMANTLE;
    af.duration   = ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 3 : 9;
    af.modifier   = 0;
    af.location   = 0;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_rimefang(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_rimefang(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong 'type' called in cast_rimefang!");
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

  for (CHAR *temp_vict = world[CHAR_REAL_ROOM(ch)].people, *next_vict; temp_vict; temp_vict = next_vict) {
    next_vict = temp_vict->next_in_room;

    if ((temp_vict == ch) || IS_IMMORTAL(temp_vict) || (IS_NPC(ch) && IS_NPC(temp_vict))) continue;

    if (IS_NPC(temp_vict) || ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_vict))) {
      act("$n sends a wall of jagged ice cascading towards you!", FALSE, ch, 0, temp_vict, TO_VICT);

      if (!IS_SET(GET_IMMUNE(temp_vict), IMMUNE_PARALYSIS) &&
          !IS_AFFECTED(temp_vict, AFF_PARALYSIS) &&
          ((GET_LEVEL(ch) + 10) >= GET_LEVEL(temp_vict)) &&
          !saves_spell(temp_vict, SAVING_PARA, (level + 10))) {
        affect_apply(temp_vict, SPELL_PARALYSIS, (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 1 : (GET_LEVEL(ch) / 10)), 100, APPLY_AC, AFF_PARALYSIS, 0);
        affect_apply(temp_vict, SPELL_PARALYSIS, (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 1 : (GET_LEVEL(ch) / 10)), -5, APPLY_HITROLL, AFF_PARALYSIS, 0);

        send_to_char("Your limbs freeze in place.\n\r", temp_vict);
        act("$n is paralyzed!", FALSE, temp_vict, 0, 0, TO_ROOM);

        if (IS_NPC(temp_vict)) {
          MOB_ATT_TIMER(temp_vict) = MAX(MOB_ATT_TIMER(temp_vict), 2);
        }
        else {
          WAIT_STATE(temp_vict, PULSE_VIOLENCE * (ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_vict)) ? number(1, 2) : 2));
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
  if (type == SPELL_TYPE_SPELL) {
    spell_desecrate(level, ch, victim, obj);
  }
  else {
    log_f("Wrong 'type' called in: cast_desecrate()");
  }
}

void spell_desecrate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int mob_level = 0;

  if (OBJ_TYPE(obj) != ITEM_CONTAINER || OBJ_VALUE3(obj) != 1) {
    /* Object is not a corpse, or a container. */
    send_to_char("You must target a corpse.\n\r", ch);

    return;
  }

  if (OBJ_COST(obj) == PC_CORPSE && obj->contains) {
    /* The corpse is that of a PC and contains items (prevent griefing). */
    send_to_char("The corpse has something in it.\n\r", ch);

    return;
  }

  mob_level = OBJ_VALUE2(obj);

  if (!affected_by_spell(ch, SPELL_DESECRATE)) {
    affect_apply(ch, SPELL_DESECRATE, ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 3 : 7, -(mob_level / 2), APPLY_AC, 0, 0);
    affect_apply(ch, SPELL_DESECRATE, ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 3 : 7, mob_level, APPLY_HP_REGEN, 0, 0);
  }

  act("You perform vile acts upon $p, desecrating it.", FALSE, ch, obj, 0, TO_CHAR);
  act("$n performs vile acts upon $p, desecrating it.", FALSE, ch, obj, 0, TO_ROOM);

  if (IS_MORTAL(ch) && (mob_level >= (GET_LEVEL(ch) - 10))) {
    send_to_room("Some blood and gore is left behind after the ritual is complete.\n\r", CHAR_REAL_ROOM(ch));

    ROOM_BLOOD(CHAR_REAL_ROOM(ch)) = MIN(ROOM_BLOOD(CHAR_REAL_ROOM(ch)) + 1, 10);
  }

  extract_obj(obj);

  check_equipment(ch);
}

void cast_demonic_thunder(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_demonic_thunder(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in demonic thunder!");
      break;
  }
}

void spell_demonic_thunder(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  damage(ch, victim, 500, SPELL_DEMONIC_THUNDER, DAM_MAGICAL);

  if (affected_by_spell(victim, SPELL_CURSE)) return;
  if (saves_spell(victim, SAVING_SPELL, level + 10)) return;

  af.type       = SPELL_CURSE;
  af.duration   = (24 * 7);
  af.bitvector  = AFF_CURSE;
  af.bitvector2 = 0;

  af.location = APPLY_HITROLL;
  af.modifier   = -((GET_LEVEL(ch) - 3) / 9);
  affect_to_char(victim, &af);

  af.location   = APPLY_SAVING_PARA;
  af.modifier = ((GET_LEVEL(ch) - 3) / 9);
  affect_to_char(victim, &af);

  act("$n briefly reveals a red aura!", FALSE, victim, 0, 0, TO_ROOM);
  act("You feel very uncomfortable.", FALSE, victim, 0, 0, TO_CHAR);
}

void cast_aid(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_aid(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in aid!");
      break;
  }
}

void spell_aid(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_AID))
  {
    send_to_char("You feel powerful!\n\r", victim);

    af.type       = SPELL_AID;
    af.duration   = 5;
    af.modifier   = 5;
    af.location   = APPLY_DAMROLL;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void cast_engage(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_engage(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in engage!");
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
  if(affected_by_spell(ch, SPELL_CLARITY))
    dam=0;

  damage(ch, victim, dam, w_type,DAM_PHYSICAL);
}

void cast_ethereal_nature(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_ethereal_nature(level, ch, 0, 0);
      break;
    default :
      log_f("Wrong type called in ethereal_nature!");
      break;
  }
}

void spell_ethereal_nature(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_ETHEREAL_NATURE))
  {
    send_to_char("You slowly fade into an alternate plane of existence.\n\r", ch);
    act("$n slowly fades into an alternate plane of existence.", TRUE, ch, 0, 0, TO_ROOM);

    af.type       = SPELL_ETHEREAL_NATURE;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 12;
    else
      af.duration = 30;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_magic_armament(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_magic_armament(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in magic armament!");
      break;
  }
}

void spell_magic_armament(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_MAGIC_ARMAMENT))
  {
    send_to_char("You feel the force of magic coursing through your body.\n\r", ch);

    af.type       = SPELL_MAGIC_ARMAMENT;
    af.duration   = 3;
    af.modifier   = dice(2, 5);
    af.location   = APPLY_DAMROLL;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_degenerate(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_degenerate(level, ch, 0, 0);
      break;
    default :
      log_f("Wrong type called in degenerate!");
      break;
  }
}

void spell_degenerate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;
  int tmp = 0;

  if (!affected_by_spell(ch, SPELL_DEGENERATE))
  {
    send_to_char("You eat away at your life force for a few precious mana points.\n\r", ch);
    act("$n eats away at $s life force for a few precious mana points.", TRUE, ch, 0, 0, TO_ROOM);

    af.type       = SPELL_DEGENERATE;
    af.duration   = ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? 12 : 30;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);

    tmp = MIN(MAX(GET_HIT(ch) - 10, 0), (GET_MAX_MANA(ch) - GET_MANA(ch)) / 2);
    GET_HIT(ch) -= tmp;
    GET_MANA(ch) = MIN(GET_MANA(ch) + (tmp * 2), GET_MAX_MANA(ch));
  }
}

void cast_fortification(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_fortification(level, ch, victim, 0);
      break;
    default :
      log_f("Wrong type called in fortification!");
      break;
  }
}

void spell_fortification(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && victim != ch)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_FORTIFICATION))
  {
    send_to_char("You feel able to withstand any attack.\n\r", victim);
    act("$n seems able to withstand any attack.", TRUE, victim, 0, 0, TO_ROOM);

    af.type       = SPELL_FORTIFICATION;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 5;
    else
      af.duration = 10;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void cast_disrupt_sanct(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_disrupt_sanct(level, ch, victim, 0);
      break;
    default :
      log_f("Wrong type called in disrupt sanct!");
      break;
  }
}

void spell_disrupt_sanct(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_DISRUPT_SANCT) &&
      (affected_by_spell(victim, SPELL_SANCTUARY) || IS_AFFECTED(victim, AFF_SANCTUARY)))
  {
    send_to_char("Your white aura seems to fade a little.\n\r", victim);
    act("$n's white aura seems to fade a little.", TRUE, victim, 0, 0, TO_ROOM);

    af.type       = SPELL_DISRUPT_SANCT;
    af.duration   = 0;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }
}

void cast_wrath_of_god(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim) {
        spell_wrath_of_god(level, ch, victim, 0);
      }
      break;
    default:
      log_f("Wrong type called in wrath of god!");
      break;
  }
}

void spell_wrath_of_god(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  AFF af;
  AFF *aff = NULL;
  int hitroll_modifier = 0, damroll_modifier = 0;

  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  damage(ch, victim, 600, SPELL_WRATH_OF_GOD, DAM_MAGICAL);

  if (affected_by_spell(victim, SPELL_WRATH_OF_GOD)) {
    for (aff = victim->affected; aff; aff = aff->next)
    {
      if (aff->type == SPELL_WRATH_OF_GOD)
      {
        if (aff->location == APPLY_HITROLL) {
          hitroll_modifier = MAX(aff->modifier - 1, -5);
        }
        else if (aff->location == APPLY_DAMROLL) {
          damroll_modifier = MAX(aff->modifier - 10, -50);
        }
      }
    }
  }
  else {
    hitroll_modifier = -1;
    damroll_modifier = -10;
  }

  affect_from_char(victim, SPELL_WRATH_OF_GOD);

  af.type       = SPELL_WRATH_OF_GOD;
  af.duration   = 2;
  af.modifier   = hitroll_modifier;
  af.location   = APPLY_HITROLL;
  af.bitvector  = AFF_NONE;
  af.bitvector2 = AFF_NONE;
  affect_to_char(victim, &af);
  af.modifier   = damroll_modifier;
  af.location   = APPLY_DAMROLL;
  affect_to_char(victim, &af);
}

//void cast_focus(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
//  switch (type) {
//    case SPELL_TYPE_SPELL:
//      spell_focus(level, ch, 0, 0);
//      break;
//    default :
//      log_f("Wrong type called in focus!");
//      break;
//  }
//}
//
//void spell_focus(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
//  AFF af;
//
//  if (!affected_by_spell(ch, SPELL_FOCUS)) {
//    send_to_char("You focus your thoughts.\n\r", ch);
//
//    af.type       = SPELL_FOCUS;
//    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
//      af.duration = 5;
//    else
//      af.duration = 10;
//    af.modifier   = 0;
//    af.location   = APPLY_NONE;
//    af.bitvector  = AFF_NONE;
//    af.bitvector2 = AFF_NONE;
//    affect_to_char(ch, &af);
//  }
//}

void cast_power_of_devotion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  char buf[MIL];

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_power_of_devotion(level, ch, victim, 0);

      arg = one_argument(arg, buf);

      while (*buf) {
        if ((GET_MANA(ch) < spell_info[SPELL_POWER_OF_DEVOTION].min_usesmana)) {
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
    default:
      log_f("Wrong type called in power of devotion!");
      break;
  }
}

void spell_power_of_devotion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && victim != ch) {
    send_to_char("Things are too chaotic to cast this spell on another player.\n\r", ch);

    return;
  }

  if (!affected_by_spell(victim, SPELL_POWER_OF_DEVOTION)) {
    send_to_char("You are enveloped in a bright white aura.\n\r", victim);
    act("$n is enveloped in a bright white aura.", TRUE, victim, 0, 0, TO_ROOM);

    int duration = (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? (level / 8) : (level / 4));

    if (!affected_by_spell(victim, SPELL_SANCTUARY) && !IS_AFFECTED(victim, AFF_SANCTUARY)) {
      affect_apply(victim, SPELL_SANCTUARY, duration, 0, 0, AFF_SANCTUARY, 0);
    }

    affect_apply(victim, SPELL_POWER_OF_DEVOTION, duration, -15, APPLY_AC, 0, 0);
    affect_apply(victim, SPELL_POWER_OF_DEVOTION, duration, 3, APPLY_DAMROLL, 0, 0);
    affect_apply(victim, SPELL_POWER_OF_DEVOTION, duration, 25, APPLY_HP_REGEN, 0, 0);
    affect_apply(victim, SPELL_POWER_OF_DEVOTION, duration, 5, APPLY_MANA_REGEN, 0, 0);
    affect_apply(victim, SPELL_POWER_OF_DEVOTION, duration, -1, APPLY_SAVING_PARA, 0, 0);
    affect_apply(victim, SPELL_POWER_OF_DEVOTION, duration, -1, APPLY_SAVING_ROD, 0, 0);
    affect_apply(victim, SPELL_POWER_OF_DEVOTION, duration, -1, APPLY_SAVING_PETRI, 0, 0);
    affect_apply(victim, SPELL_POWER_OF_DEVOTION, duration, -1, APPLY_SAVING_BREATH, 0, 0);
    affect_apply(victim, SPELL_POWER_OF_DEVOTION, duration, -1, APPLY_SAVING_SPELL, 0, 0);
  }
}

void cast_power_of_faith(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim) {
        spell_power_of_faith(level, ch, victim, NULL);
      }
      break;
    default:
      log_f("Wrong type called in power of faith!");
      break;
  }
}

void spell_power_of_faith(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && (ch != victim)) {
    send_to_char("The chaos around you prevents this spell from being cast on another player.\n\r", ch);

    return;
  }

  int heal = 360, mana = 0;

  while ((heal < (GET_MAX_HIT(victim) - GET_HIT(victim))) && (heal < 1200)) {
    heal += 12;
    mana += 1;
  }

  magic_heal(victim, SPELL_POWER_OF_FAITH, heal, FALSE);

  GET_MANA(ch) -= mana;

  if (victim != ch) {
    act("The power of your faith heals $N's body.", FALSE, ch, 0, victim, TO_CHAR);
    act("The power of $n's faith heals your body.", FALSE, ch, 0, victim, TO_VICT);
    act("The power of $n's faith heals $N's body.", FALSE, ch, 0, victim, TO_NOTVICT);
  }
  else {
    act("The power of your faith heals your body.", FALSE, ch, 0, 0, TO_CHAR);
    act("The power of $n's faith heals $s body.", FALSE, ch, 0, 0, TO_ROOM);
  }

  /* Focus */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_CRUSADER, 2)) {
    GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch) + 10, 1000);
  }
}

void cast_devastation( ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim) {
        spell_devastation(level, ch, victim, 0);
      }
      break;
    default:
      log_f("Wrong 'type' called in cast_devastation!");
      break;
  }
}

void spell_devastation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))   {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))   {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  damage(ch, victim, 1000, SPELL_DEVASTATION, DAM_MAGICAL);
}

void cast_incendiary_cloud(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim) {
        spell_incendiary_cloud(level, ch, victim, 0);
      }
      break;

    default:
      log_f("Wrong 'type' called in cast_incendiary_cloud!");
      break;
  }
}

int incendiary_cloud_enchant(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (cmd == MSG_REMOVE_ENCH) {
    send_to_char("The cloud of fire surrounding you dies out.\n\r", ch);

    return FALSE;
  }

  if (cmd == MSG_ROUND) {
    send_to_char("The cloud of fire enveloping you burns you to the core...\n\r", ch);
    act("The cloud of fire enveloping $n burns $m to the core...", FALSE, ch, 0, 0, TO_ROOM);

    int dmg = 75;

    /* Don't kill the character, otherwise EXP is lost. */
    if (GET_HIT(ch) <= dmg) {
      dmg = GET_HIT(ch) - 1;
    }

    /* Don't consume position. */
    int set_pos = GET_POS(ch);

    damage(ch, ch, dmg, TYPE_UNDEFINED, DAM_FIRE);

    GET_POS(ch) = MIN(GET_POS(ch), set_pos);

    return FALSE;
  }

  return FALSE;
}

void spell_incendiary_cloud(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  act("You make a gesture and a huge ball of flame envelopes $N!", FALSE, ch, 0, victim, TO_CHAR);
  act("$n makes a gesture and a huge ball of flame envelopes you!", FALSE, ch, 0, victim, TO_VICT);
  act("$n makes a gesture and a huge ball of flame envelopes $N!", FALSE, ch, 0, victim, TO_NOTVICT);

  damage(ch, victim, 500, SPELL_INCENDIARY_CLOUD_NEW, DAM_FIRE);

  enchantment_apply(victim, TRUE, "Incendiary Cloud", SPELL_INCENDIARY_CLOUD_NEW, 20, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, incendiary_cloud_enchant);
}

void cast_tremor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_tremor(level, ch, 0, 0);
      break;

    default:
      log_f("Wrong 'type' called in cast_tremor!");
      break;
  }
}

void spell_tremor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  send_to_room("The ground begins to shake and heave.\n\r", CHAR_REAL_ROOM(ch));

  for (CHAR *temp_vict = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_vict; temp_vict; temp_vict = next_vict) {
    next_vict = temp_vict->next_in_room;

    if ((temp_vict == ch) || IS_IMMORTAL(temp_vict) || (IS_NPC(ch) && IS_NPC(temp_vict))) continue;

    if (IS_NPC(temp_vict) || ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_vict))) {
      damage(ch, temp_vict, 400, TYPE_UNDEFINED, DAM_MAGICAL);

      if (CHAR_REAL_ROOM(temp_vict) != NOWHERE) {
        GET_POS(temp_vict) = POSITION_RESTING;

        enchantment_apply(temp_vict, TRUE, "Staggering (Tremor)", SPELL_TREMOR, 10, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, 0);
      }
    }
  }
}

void cast_blur(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_blur(level, ch, 0, 0);
      break;

    default:
       log_f("Wrong 'type' called in cast_blur!");
      break;
  }
}

void spell_blur(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!affected_by_spell(ch, SPELL_BLUR)) {
    send_to_char("Your movements become a blur.\n\r", ch);
    act("$n's movements become a blur.", TRUE, ch, 0, 0, TO_ROOM);

    affect_apply(ch, SPELL_BLUR, (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) ? (GET_LEVEL(ch) / 10) : (GET_LEVEL(ch) / 5)), 0, 0, 0, 0);
  }
}

void cast_tranquility(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_tranquility(level, ch, 0, 0);
      break;

    default:
      log_f("Wrong 'type' called in cast_tranquility!");
      break;
  }
}

void spell_tranquility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  for (CHAR *temp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)), *next_ch; temp_ch; temp_ch = next_ch) {
    next_ch = temp_ch->next_in_room;

    if (!SAME_GROUP_EX(temp_ch, ch, TRUE)) continue;

    send_to_char("You suddenly feel awash in a sense of tranquility.\n\r", temp_ch);
    act("$n is suddenly awash in a sense of tranquility.", FALSE, temp_ch, 0, 0, TO_ROOM);

    affect_from_char(temp_ch, SPELL_TRANQUILITY);

    affect_apply(temp_ch, SPELL_TRANQUILITY, (ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_ch)) ? 3 : 6), 3, APPLY_HITROLL, 0, 0);
    affect_apply(temp_ch, SPELL_TRANQUILITY, (ROOM_CHAOTIC(CHAR_REAL_ROOM(temp_ch)) ? 3 : 6), 3, APPLY_DAMROLL, 0, 0);
  }
}

void cast_wither(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (victim)
      spell_wither(level, ch, victim, 0);
    break;

  default:
    log_f("Wrong 'type' called in cast_wither!");
    break;
  }
}

void spell_wither(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int wither_pulse_action(CHAR *victim);  

  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))   {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);

    return;
  }

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  damage(ch, victim, 350, SPELL_WITHER, DAM_MAGICAL);

  if (!victim || CHAR_REAL_ROOM(victim) == NOWHERE) return;

  affect_from_char(victim, SPELL_WITHER);

  affect_apply(victim, SPELL_WITHER, 4, 0, level, 0, 0);
}

void cast_shadow_wraith(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_shadow_wraith(level, ch, 0, 0);
      break;

    default:
      log_f("Wrong type called in shadow wraith!");
      break;
  }
}

void spell_shadow_wraith(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
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

  AFF af;

  af.type       = SPELL_SHADOW_WRAITH;
  af.duration   = 10;
  af.modifier   = 0;
  af.location   = 0;
  af.bitvector  = 0;
  af.bitvector2 = 0;

  affect_join(ch, &af, FALSE, FALSE);

  if (!affected_by_spell(ch, SPELL_SHADOW_WRAITH)) return;

  num_shadows = (MAX(1, (duration_of_spell(ch, SPELL_SHADOW_WRAITH) - 1)) / 10) + 1;

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

void cast_dusk_requiem(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj) {
  level = MIN(LEVEL_MORT, level); // Dusk Reqiuem has a hack for > LEVEL_MORT.

  switch (type)   {
  case SPELL_TYPE_SPELL:
    if (victim)
      spell_dusk_requiem(level, ch, victim, 0);
    break;

  default:
    log_f("Wrong 'type' called in cast_dusk_requiem!");
    break;
  }
}

void spell_dusk_requiem(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  if (!affected_by_spell(ch, SPELL_SHADOW_WRAITH)) {
    send_to_char("Your requiem fails to draw power from the fleeting shadows.\n\r", ch);
    act("$n's requiem fails to draw power from the fleeting shadows.", FALSE, ch, 0, 0, TO_ROOM);

    return;
  }

  int num_shadows = (MAX(1, (duration_of_spell(ch, SPELL_SHADOW_WRAITH) - 1)) / 10) + 1;

  int dam = MIN(350 * 4, 350 * num_shadows);

  /* If level is > LEVEL_MORT, Shadow Wraith must be expiring in signal_char() (or an immortal cast it). Inflict double damage. */
  if (level > LEVEL_MORT) {
    dam *= 2;
  }

  damage(ch, victim, dam, SPELL_DUSK_REQUIEM, DAM_MAGICAL);

  /* We only want to reduce duration if the spell was cast, not if Shadow Wraith is expiring. */
  if (level <= LEVEL_MORT) {
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

