/* ************************************************************************
*  file: mob.spells.h , Implementation of magic spells.   Part of DIKUMUD *
************************************************************************* */

/*
$Author: ronin $
$Date: 2004/02/05 16:13:16 $
$Header: /home/ronin/cvs/ronin/mob.spells.h,v 2.0.0.1 2004/02/05 16:13:16 ronin Exp $
$Id: mob.spells.h,v 2.0.0.1 2004/02/05 16:13:16 ronin Exp $
$Name:  $
$Log: mob.spells.h,v $
Revision 2.0.0.1  2004/02/05 16:13:16  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


void spell_firebreath(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_fire_breath(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_frost_breath(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_acid_breath(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_gas_breath(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_lightning_breath(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_vanish(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_enfeeblement(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_finger_of_death(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_thunderstorm(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_primal_rage(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_drain_mana(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

