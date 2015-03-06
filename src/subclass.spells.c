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

/*
$Author: ronin $
$Date: 2005/01/21 14:55:30 $
$Header: /home/ronin/cvs/ronin/subclass.spells.c,v 2.5 2005/01/21 14:55:30 ronin Exp $
$Id: subclass.spells.c,v 2.5 2005/01/21 14:55:30 ronin Exp $
$Name:  $
$Log: subclass.spells.c,v $
Revision 2.5  2005/01/21 14:55:30  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.4  2004/11/16 05:11:53  ronin
Chaos 2004 Update

Revision 2.3  2004/06/30 19:57:29  ronin
Update of saving throw routine.

Revision 2.2  2004/02/14 13:12:50  void
Fixed a couple errors I made

Revision 2.1  2004/02/14 00:17:53  void

Changed Rimefang to affect impy/hidden chars in chaos.

Revision 2.0.0.1  2004/02/05 16:11:56  ronin
Reinitialization of cvs archives

Revision 1.5 2003/11/03 ronin
Added check_equipment for all GET_ALIGN changes.

Revision - 14-Feb-03 Ranger
Fixed cloud of confusion to not attack IMM+

Revision - 03-Dec-02 Ranger
Rimefang - removed saving vs spell on damage, removed level check, increased duration to 3
Power of Faith - upped limit for healed to 700 and mana limit to 120

Revision - 20-Nov-02 Ranger
Removed damage messages from wrath_of_ancients, frost_bolt, divine_wind,
demonic_thunder, wrath_of_god, devasation.  Messages move to messages file in lib and
miss/death/god attack ones added.

Revision - removed chance(75) from disrupt sanct - increase mana cost to 350

Revision - additional heal message added to rejuvenation

Revision 1.4  2002/06/18 14:31:08  ronin
Change to allow divine wind castable against other players.

Revision 1.3  2002/05/09 20:18:48  ronin
Change of some text messages from TO_ROOM to TO_NOTVICT

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "structs.h"
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

extern struct time_info_data time_info;
extern struct room_data *world;
extern struct char_data *character_list;
extern int CHAOSMODE;

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
    af.duration   = 10;
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

void cast_cloud_confusion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_cloud_confusion(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in cloud of confusion!");
      break;
  }
}

void spell_cloud_confusion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;
  CHAR *leader = NULL;
  CHAR *tmp_victim = NULL;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!IS_AFFECTED(ch, AFF_GROUP) || !ch->master)
    leader = ch;
  else
    leader = ch->master;

  send_to_char("You whirl around, engulfing the area with a thick black cloud.\n\r", ch);
  act("$n whirls around, engulfing the area with a thick black cloud.", FALSE, ch, 0, 0, TO_ROOM);

  for (victim = world[CHAR_REAL_ROOM(ch)].people; victim; victim = tmp_victim)
  {
    tmp_victim = victim->next_in_room;

    if (!IS_NPC(victim) && GET_LEVEL(victim) >= LEVEL_IMM) continue;

    if (!affected_by_spell(victim, SPELL_CLOUD_CONFUSION) &&
        victim != leader &&
        victim->master != leader &&
        (!IS_AFFECTED(victim, AFF_SPHERE) || breakthrough(ch, victim, BT_SPHERE)))
    {
      send_to_char("You feel disoriented.\n\r", victim);

      af.type       = SPELL_CLOUD_CONFUSION;
      if (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
        af.duration = 1;
      else
        af.duration = number(2, 6); /* variable duration */
      af.modifier   = -(number(5, 8)); /* variable hitroll */
      if (IS_NIGHT)
        af.modifier--;
      af.location   = APPLY_HITROLL;
      af.bitvector  = 0;
      af.bitvector2 = 0;
      affect_to_char(victim, &af);
    }
  }
}

void cast_rage(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_rage(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in rage!");
      break;
  }
}

void spell_rage(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_RAGE))
  {
    send_to_char("Rage courses through your body!\n\r", ch);

    af.type       = SPELL_RAGE;
    af.duration   = 3;
    af.modifier   = 0;
    af.location   = 0;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_righteousness(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_righteousness(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in righteousness!");
      break;
  }
}

void spell_righteousness(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_RIGHTEOUSNESS))
  {
    send_to_char("You feel righteous!\n\r", ch);

    af.type       = SPELL_RIGHTEOUSNESS;
    af.duration   = 10;
    af.modifier   = 0;
    af.location   = 0;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
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

void cast_might(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_might(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in might!");
      break;
  }
}

void spell_might(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_MIGHT))
  {
    send_to_char("You feel more powerful.\n\r", ch);

    af.type       = SPELL_MIGHT;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 10;
    else
      af.duration = 20;
    af.modifier   = 2;
    af.location   = APPLY_DAMROLL;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
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

void cast_ironskin(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_ironskin(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in ironskin!");
      break;
  }
}

void spell_ironskin(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_IRONSKIN))
  {
    send_to_char("You feel your skin harden.\n\r", ch);
    act("$n's skin hardens and turns dark iron in color.", TRUE, ch, 0, 0, TO_ROOM);

    af.type       = SPELL_IRONSKIN;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 5;
    else
      af.duration = 10;
    af.modifier   = -25;
    af.location   = APPLY_AC;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_frostbolt(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_frostbolt(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in frostbolt!");
      break;
  }
}

void spell_frostbolt(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!IS_NPC(ch) && !IS_NPC(victim) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
    return;
  }

  GET_MOVE(victim) = MAX(GET_MOVE(victim) - 100, 0);
  damage(ch, victim, number(330, 370), SPELL_FROSTBOLT, DAM_COLD);
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
    send_to_char("There is already a wall of thorns here.\n\r", ch);
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
  switch (type)
  {
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
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && ch != victim)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      ((duration_of_spell(victim, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(victim, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", victim);
    return;
  }

  //GET_HIT(victim) = MIN(GET_HIT(victim) + 400, GET_MAX_HIT(victim));
  magic_heal(victim, SPELL_REJUVENATION, 400, FALSE);
  send_to_char("You feel much better!\n\r", victim);

  update_pos(victim);
}

void cast_divine_wind(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_divine_wind(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in divine wind!");
      break;
  }
}

int stack_position(CHAR *ch, int target_position);
void spell_divine_wind(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  int set_pos = 0;

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

  set_pos = stack_position(victim, POSITION_RESTING);

  damage(ch, victim, 300, SPELL_DIVINE_WIND, DAM_MAGICAL);

  if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim)) {
    GET_POS(victim) = set_pos;
  }
}

void cast_dark_ritual(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (tar_obj)
        spell_dark_ritual(level, ch, victim, tar_obj);
      break;
    default:
      log_f("Wrong type called in dark ritual!");
      break;
  }
}

void spell_dark_ritual(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  extern void check_equipment(CHAR *ch);
  int mob_level = 0;
  int heal = 0;
  AFF af;

  if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER || obj->obj_flags.value[3] != 1)
  {
    /* Object is not a corpse, or a container. */
    send_to_char("You must target a corpse to perform your dark ritual.\n\r", ch);

    return;
  }

  if (obj->obj_flags.cost == PC_CORPSE && obj->contains)
  {
    /* The corpse is that of a PC and contains items (prevent griefing). */
    send_to_char("The corpse has something in it.\n\r", ch);

    return;
  }

  mob_level = obj->obj_flags.value[2];
  heal = (mob_level * 9) / 2;

  if ((heal + GET_HIT(ch)) > hit_limit(ch))
    GET_HIT(ch) = hit_limit(ch);
  else
    GET_HIT(ch) += heal;

  GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - heal);

  if (!affected_by_spell(ch, SPELL_DARK_RITUAL) && (IS_NIGHT || IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, DARK)))
  {
    af.type       = SPELL_DARK_RITUAL;
    af.duration   = 2;
    af.modifier   = 2;
    af.location   = APPLY_DAMROLL;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
    af.location   = APPLY_HITROLL;
    affect_to_char(ch, &af);

    send_to_char("You are surrounded by a sinister presence.\n\r", ch);
  }

  check_equipment(ch);

  send_to_char("You perform a dark ritual and absorb life energy from the dead.\n\r", ch);
  act("$n performs a dark ritual and absorbs life energy from the dead.", TRUE, ch, obj, 0, TO_ROOM);

  extract_obj(obj);
  update_pos(ch);
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
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 3;
    else
      af.duration = 10;
    af.modifier   = 0;
    af.location   = 0;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_rimefang(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_rimefang(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in rimefang!");
      break;
  }
}

void spell_rimefang(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  int dam = 0;
  AFF af;
  AFF af2;
  CHAR *tmp_victim = NULL;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  dam = (5 * GET_LEVEL(ch));

  if (time_info.month <= 2 || time_info.month >= 15) dam += 75; /* Increased damage during winter. */

  send_to_char("An aura of frost starts to form around you.\n\r", ch);
  act("An aura of frost starts to form around $n.", FALSE, ch, 0, 0, TO_ROOM);

  af.type       = SPELL_PARALYSIS;
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
    af.duration = 1;
  else
    af.duration = (GET_LEVEL(ch) / 10);
  af.modifier   = 100;
  af.location   = APPLY_AC;
  af.bitvector  = AFF_PARALYSIS;
  af.bitvector2 = 0;

  af2 = af;
  af2.location = APPLY_HITROLL;
  af2.modifier = -5;

  for (victim = world[CHAR_REAL_ROOM(ch)].people; victim; victim = tmp_victim)
  {
    tmp_victim = victim->next_in_room;

    if (victim == ch || IS_IMMORTAL(victim)) continue;

    if (IS_NPC(victim) || ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
    {
      act("$n sends a wall of jagged ice cascading towards you!", FALSE, ch, 0, victim, TO_VICT);

      if (!IS_SET(victim->specials.immune, IMMUNE_PARALYSIS) &&
          !IS_AFFECTED(victim, AFF_PARALYSIS) &&
          ((GET_LEVEL(ch) + 10) >= GET_LEVEL(victim)) &&
          !saves_spell(victim, SAVING_PARA, level + 10)) /* Rimefang paralyze has an increased success rate. */
      {
        affect_to_char(victim, &af);
        affect_to_char(victim, &af2);

        send_to_char("Your limbs freeze in place.\n\r", victim);
        act("$n is paralyzed!", TRUE, victim, 0, 0, TO_ROOM);
      }

      if (saves_spell(victim, SAVING_SPELL, level)) dam /= 2;

      damage(ch, victim, dam, TYPE_UNDEFINED, DAM_MAGICAL);
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

void cast_desecrate(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (obj)
        spell_desecrate(level, ch, victim, obj);
      break;
    default:
      log_f("Wrong type called in desecrate!");
      break;
  }
}

void spell_desecrate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;
  int mob_level = 0;

  if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER || obj->obj_flags.value[3] != 1)
  {
    /* Object is not a corpse, or a container. */
    send_to_char("You must target a corpse in order to desecrate it.\n\r", ch);

    return;
  }

  if (obj->obj_flags.cost == PC_CORPSE && obj->contains)
  {
    /* The corpse is that of a PC and contains items (prevent griefing). */
    send_to_char("The corpse has something in it.\n\r", ch);

    return;
  }

  if (!affected_by_spell(ch, SPELL_DESECRATE))
  {
    mob_level = obj->obj_flags.value[2];

    af.type        = SPELL_DESECRATE;
    if(ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration  = 3;
    else
      af.duration  = 10;
    af.modifier    = -mob_level;
    af.location    = APPLY_AC;
    af.bitvector   = 0;
    af.bitvector2  = 0;
    affect_to_char(ch, &af);

    af.modifier    = mob_level;
    af.location    = APPLY_HP_REGEN;
    affect_to_char(ch, &af);
  }

  act("You perform vile acts upon $p, desecrating it.", FALSE, ch, obj, 0, TO_CHAR);
  act("$n performs vile acts upon $p, desecrating it.", FALSE, ch, obj, 0, TO_ROOM);

  check_equipment(ch);

  extract_obj(obj);
  update_pos(ch);
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
    af.duration   = 4;
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
  extern int wpn_extra(CHAR *ch, CHAR *victim, OBJ *wielded);
  extern struct str_app_type str_app[];
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

  if (victim->specials.fighting)
  {
    send_to_char("That victim is already in combat.\n\r", ch);
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
  if (wielded->obj_flags.type_flag == ITEM_2HWEAPON) dam = (dam*3)/2;
  dam += GET_DAMROLL(ch);
  dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
  dam += wpn_extra(ch,victim,wielded);

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
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 12;
    else
      af.duration = 30;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);

    tmp = MIN(GET_HIT(ch) - 10, GET_MAX_MANA(ch) - GET_MANA(ch));
    GET_HIT(ch) -= tmp;
    GET_MANA(ch) = MIN(GET_MANA(ch) + tmp, GET_MAX_MANA(ch));
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

void cast_wrath_of_god(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_wrath_of_god(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in wrath of god!");
      break;
  }
}

void spell_wrath_of_god(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  extern void check_equipment(CHAR *ch);

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

  if (IS_GOOD(victim)) GET_ALIGNMENT(ch) -= 200;
  else if (IS_NEUTRAL(victim)) GET_ALIGNMENT(ch) -= 100;

  check_equipment(ch);

  damage(ch, victim, 800, SPELL_WRATH_OF_GOD, DAM_MAGICAL);
}

void cast_focus(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_focus(level, ch, 0, 0);
      break;
    default :
      log_f("Wrong type called in focus!");
      break;
  }
}

void spell_focus(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_FOCUS))
  {
    send_to_char("You focus your thoughts.\n\r", ch);

    af.type       = SPELL_FOCUS;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 5;
    else
      af.duration = 10;
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_power_of_devotion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_power_of_devotion(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in power of devotion!");
      break;
  }
}

void spell_power_of_devotion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && victim != ch)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_POWER_OF_DEVOTION) &&
      !affected_by_spell(victim, SPELL_SANCTUARY) &&
      !IS_AFFECTED(victim, AFF_SANCTUARY))
  {
    send_to_char("You start glowing.\n\r", victim);
    act("$n is surrounded by a bright white aura.", TRUE, victim, 0, 0, TO_ROOM);

    af.type       = SPELL_SANCTUARY;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = (level / 8);
    else
      af.duration = (level / 4);
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = AFF_SANCTUARY;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);

    af.type      = SPELL_POWER_OF_DEVOTION;
    af.modifier  = -15;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.modifier  = -1;
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
}

void cast_power_of_faith(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_power_of_faith(level, ch, victim, NULL);
      break;
    default:
      log_f("Wrong type called in power of faith!");
      break;
  }
}

void spell_power_of_faith(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  int heal_max = 0;
  int mana_max = 0;
  int heal = 0;
  int mana = 0;

  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && victim != ch)
  {
    send_to_char("You cannot cast this spell on another player.\n\r", ch);
    return;
  }
  
  if (affected_by_spell(victim, SPELL_DEGENERATE) &&
      ((duration_of_spell(victim, SPELL_DEGENERATE) > 27) ||
       ((duration_of_spell(victim, SPELL_DEGENERATE) > 9) && ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))))
  {
    send_to_char("The magic of the spell fails to heal your degenerated body.\n\r", victim);
    return;
  }

  heal_max = MIN(GET_MAX_HIT(victim) - GET_HIT(victim), 1050);
  heal_max = MAX(heal_max, 0);
  mana_max = MIN(GET_MANA(ch), 70);
  mana_max = MAX(mana_max, 0);

  while ((heal < heal_max) && (mana < mana_max))
  {
    heal += 15;
    mana += 1;
  }

  heal = MIN(heal, 1050);
  heal = MIN(GET_MAX_HIT(victim), heal + 50);

  //GET_HIT(victim) += heal;
  magic_heal(victim, SPELL_POWER_OF_FAITH, heal, FALSE);
  GET_MANA(ch) -= mana;

  if (victim != ch)
  {
    act("The power of your faith heals $N's body.", FALSE, ch, 0, victim, TO_CHAR);
    act("The power of $n's faith heals your body.", FALSE, ch, 0, victim, TO_VICT);
  }
  else
  {
    send_to_char("The power of your faith heals your body.\n\r", ch);
  }

  update_pos(victim);
}

void cast_devastation( ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_devastation(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in devastation!");
      break;
  }
}

void spell_devastation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
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

  damage(ch, victim, number(1000, 1200), SPELL_DEVASTATION, DAM_MAGICAL);
}

void cast_incendiary_cloud(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_incendiary_cloud(level, ch, victim, 0);
      break;
    default:
      log_f("Wrong type called in incendiary cloud!");
      break;
  }
}

void spell_incendiary_cloud(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
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

  if (!affected_by_spell(victim, SPELL_INCENDIARY_CLOUD))
  {
    af.type       = SPELL_INCENDIARY_CLOUD;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
      af.duration = 2;
    else
      af.duration = 8;
    af.modifier   = 0;
    af.location   = 0;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
  }

  act("You make a gesture and a huge ball of flame envelopes $N.", FALSE, ch, 0, victim, TO_CHAR);
  act("$n makes a gesture and a huge ball of flame envelopes you.", FALSE, ch, 0, victim, TO_VICT);
  act("$n makes a gesture and a huge ball of flame envelopes $N.", FALSE, ch, 0, victim, TO_NOTVICT);
  damage(ch, victim, 600, TYPE_UNDEFINED, DAM_NO_BLOCK);
}

void cast_shadow_wraith(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
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
  AFF af;

  if (duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 60) /* 4 shadows */
  {
    send_to_char("You already have four shadows.\n\r", ch);
    return;
  }
  else if (duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 40) /* 3 shadows */
  {
    if (chance(90))
    {
      send_to_char("You failed to cast another shadow.\n\r", ch);
      return;
    }
  }
  else if (duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 20) /* 2 shadows */
  {
    if (chance(75))
    {
      send_to_char("You failed to cast another shadow.\n\r", ch);
      return;
    }
  }
  else if (affected_by_spell(ch, SPELL_SHADOW_WRAITH)) /* 1 shadow */
  {
    if (chance(60))
    {
      send_to_char("You failed to cast another shadow.\n\r", ch);
      return;
    }
  }
  else
  {
    if (chance(45))
    {
      send_to_char("You failed to cast a shadow.\n\r", ch);
      return;
    }
  }

  af.type       = SPELL_SHADOW_WRAITH;
  af.duration   = 20;
  af.modifier   = 0;
  af.location   = APPLY_NONE;
  af.bitvector  = 0;
  af.bitvector2 = 0;
  affect_join(ch, &af, FALSE, FALSE);

  /* Shadow position is based on the new duration. */
  if (duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 60) /* 4 shadows */
  {
    send_to_char("Your shadow stretches to the west.\n\r", ch);
    act("$n's shadow stretches to the west.", FALSE, ch, 0, 0, TO_ROOM);
  }
  else if (duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 40) /* 3 shadows */
  {
    send_to_char("Your shadow stretches to the east.\n\r", ch);
    act("$n's shadow stretches to the east.", FALSE, ch, 0, 0, TO_ROOM);
  }
  else if (duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 20) /* 2 shadows */
  {
    send_to_char("Your shadow stretches to the south.\n\r", ch);
    act("$n's shadow stretches to the south.", FALSE, ch, 0, 0, TO_ROOM);
  }
  else /* 1 shadow */
  {
    send_to_char("Your shadow stretches to the north.\n\r", ch);
    act("$n's shadow stretches to the north.", FALSE, ch, 0, 0, TO_ROOM);
  }
}

void cast_tremor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_tremor(level, ch, 0, 0);
      break;
    default:
      log_f("Wrong type called in tremor!");
      break;
  }
}

void spell_tremor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  CHAR *tmp_victim = NULL;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch)))
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  send_to_room("The ground begins to shake and heave.\n\r", CHAR_REAL_ROOM(ch));

  for (victim = world[CHAR_REAL_ROOM(ch)].people; victim; victim = tmp_victim)
  {
    tmp_victim = victim->next_in_room;

    if (victim == ch || IS_IMMORTAL(victim)) continue;

    if (IS_NPC(victim) || ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)))
    {
      damage(ch, victim, 400, TYPE_UNDEFINED, DAM_MAGICAL);

      if (CHAR_REAL_ROOM(victim) != NOWHERE)
      {
        GET_POS(victim) = POSITION_STUNNED;
      }
    }
  }
}

void cast_blur(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      spell_blur(level, ch, 0, 0);
      break;
    default:
       log_f("Wrong type called in blur!");
      break;
  }
}

void spell_blur(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(ch, SPELL_BLUR))
  {
    send_to_char("Your movements become a blur.\n\r", ch);
    act("$n's movements become a blur.", TRUE, ch, 0, 0, TO_ROOM);

    af.type       = SPELL_BLUR;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = (GET_LEVEL(ch) / 12);
    else
      af.duration = (GET_LEVEL(ch) / 6);
    af.modifier   = 0;
    af.location   = APPLY_NONE;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);
  }
}

void cast_tranquility(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj)
{
  switch (type)
  {
    case SPELL_TYPE_SPELL:
      if (victim)
        spell_tranquility(level, ch, victim, 0);
      break;
    default:
       log_f("Wrong type called in tranquility!");
      break;
  }
}

void spell_tranquility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj)
{
  AFF af;

  if (!affected_by_spell(victim, SPELL_TRANQUILITY))
  {
    send_to_char("You suddenly feel awash in a sense of tranquility.\n\r", victim);
    act("$n is suddenly awash in a sense of tranquility.", TRUE, victim, 0, 0, TO_ROOM);

    af.type       = SPELL_TRANQUILITY;
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
      af.duration = 2;
    else
      af.duration = 4;
    af.modifier   = 2;
    af.location   = APPLY_HITROLL;
    af.bitvector  = 0;
    af.bitvector2 = 0;
    affect_to_char(victim, &af);
    af.location   = APPLY_DAMROLL;
    affect_to_char(victim, &af);
  }
}
