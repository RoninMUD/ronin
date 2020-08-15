/*
** subclass.h
**   Include for subclasses.
**
** Written for RoninMUD by Ranger
**
** Do not distribute without permission.
*/

#ifndef _SUBCLASS_H_
#define _SUBCLASS_H_

/* utility*/
int check_sc_access(CHAR *ch, int s);
int check_subclass(CHAR *ch, int sub, int lvl);
int set_subclass(CHAR *ch, int sub, int lvl);
void remove_subclass(CHAR *ch);

/* spells */
void spell_aid(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void spell_rally(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void spell_warchant(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void spell_luck(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_rejuvenation(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_rejuvenation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_cloud_confusion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_cloud_confusion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_rage(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_rage(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_righteousness(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_righteousness(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_debilitate(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_debilitate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_might(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_might(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_blade_barrier(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_blade_barrier(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_distortion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_distortion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_iron_skin(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_iron_skin(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_frostbolt(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_frostbolt(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_orb_protection(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_orb_protection(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_wrath_ancients(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_wrath_ancients(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_meteor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_meteor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_wall_thorns(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_wall_thorns(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_rejuvenation(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_rejuvenation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_divine_wind(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_divine_wind(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_blackmantle(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_blackmantle(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_rimefang(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_rimefang(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_passdoor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_passdoor(ubyte level, CHAR *ch, int direction);
void cast_desecrate(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_desecrate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_engage(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_engage(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_ethereal_nature(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_ethereal_nature(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_magic_armament(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_magic_armament(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_fortification(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_fortification(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_disrupt_sanct(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_disrupt_sanct(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_power_of_faith(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_power_of_faith(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_wrath_of_god(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_wrath_of_god(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_power_of_devotion(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_power_of_devotion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_incendiary_cloud(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_incendiary_cloud(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_devastation(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_devastation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_shadow_wraith(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_shadow_wraith(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_tremor(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_tremor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_blur(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_blur(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_tranquility(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_tranquility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_wither(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_wither(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_dusk_requiem(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_dusk_requiem(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_divine_hammer(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_divine_hammer(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);

extern int token_mob_time;

#endif /* _SUBCLASS_H_ */
