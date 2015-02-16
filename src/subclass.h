/*
** subclass.h
**   Include for subclasses.
**
** Written for RoninMUD by Ranger
**
** Do not distribute without permission.
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:13:41 $
$Header: /home/ronin/cvs/ronin/subclass.h,v 2.0.0.1 2004/02/05 16:13:41 ronin Exp $
$Id: subclass.h,v 2.0.0.1 2004/02/05 16:13:41 ronin Exp $
$Name:  $
$Log: subclass.h,v $
Revision 2.0.0.1  2004/02/05 16:13:41  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/


/* utility*/
int check_sc_access(CHAR *ch,int s);
int check_sc_song_access(CHAR *ch,int s);
int check_subclass(CHAR *ch,int sub,int lvl);
int set_subclass(CHAR *ch,int sub,int lvl);
void remove_subclass(CHAR *ch);

/* spells */
void spell_aid(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void spell_rally(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void spell_warchant(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_luck(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_luck(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_rejuvenation(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_rejuvenation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_cloud_confusion(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_cloud_confusion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_rage(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_rage(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_righteousness(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_righteousness(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_debilitate(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_debilitate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_might(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_might(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_clarity(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_clarity(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_blade_barrier(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_blade_barrier(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_distortion(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_distortion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_ironskin(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_ironskin(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_frostbolt(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_frostbolt(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_orb_protection(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_orb_protection(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_sanctify(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_sanctify(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_wrath_ancients(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_wrath_ancients(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_meteor(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_meteor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_wall_thorns(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_wall_thorns(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_rejuvenation(ubyte level, CHAR *ch, char *arg, int type, CHAR *victim, OBJ *tar_obj);
void spell_rejuvenation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_divine_wind(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_divine_wind(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_dark_ritual(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_dark_ritual(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_blackmantle(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_blackmantle(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_rimefang(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_rimefang(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_passdoor(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_passdoor(ubyte level, CHAR *ch, int direction);
void cast_desecrate(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_desecrate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_demonic_thunder(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_demonic_thunder(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_engage(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_engage(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_ethereal_nature(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_ethereal_nature(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_magic_armament(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_magic_armament(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_degenerate(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_degenerate(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_fortification(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_fortification(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_disrupt_sanct(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_disrupt_sanct(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_power_of_faith(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_power_of_faith(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_wrath_of_god(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_wrath_of_god(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_power_of_devotion(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_power_of_devotion(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_focus(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_focus(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_incendiary_cloud(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_incendiary_cloud(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_devastation(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_devastation(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_shadow_wraith(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_shadow_wraith(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_tremor(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj );
void spell_tremor(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_blur(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj);
void spell_blur(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void cast_tranquility(ubyte level, CHAR *ch, char *arg, int type,CHAR *victim, OBJ *tar_obj);
void spell_tranquility(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
