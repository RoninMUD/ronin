/* constants.h
**
** Declares constants and constant structures used game-wide
*/

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include "structs.h"
#include "limits.h"

extern const int exp_table[58];

extern const char * const subclass_name[];
extern const int subclass_master[11];

extern const char * const pc_class_types[];
extern const char * const npc_class_types[];
extern const char * const immortal_types[];
extern const char * const connected_types[];
extern const char * const item_types[];
extern const char * const apply_types[];
extern const char * const equipment_types[];
extern const char * const sector_types[];
extern const char * const sector_types_i[];
extern const char * const position_types[];
extern const char * const att_types[];
extern const char * const att_targets[];
extern const char * const weapon_type[];

extern const char * const wear_bits[];
extern const char * const extra_bits[];
extern const char * const extra_bits2[];
extern const char * const affected_bits[];
extern const char * const affected_bits2[];
extern const char * const subclass_res_bits[];
extern const char * const room_bits[];
extern const char * const exit_bits[];
extern const char * const wiz_bits[];
extern const char * const action_bits[];
extern const char * const action_bits2[];
extern const char * const player_bits[];
extern const char * const immune_bits[];
extern const char * const immune_bits2[];
extern const char * const resist_bits[];

extern struct str_app_type str_app[];
extern struct dex_app_type dex_app[];
extern struct int_app_type int_app[];
extern struct wis_app_type wis_app[];
extern struct con_app_type con_app[];

extern struct dex_skill_type dex_app_skill[];

extern const char * const songs[];
extern const char * const spell_wear_off_msg[];

extern const char * const warrior_skills[];
extern const char * const thief_skills[];
extern const char * const ninja_skills[];
extern const char * const nomad_skills[];
extern const char * const paladin_skills[];
extern const char * const cleric_skills[];
extern const char * const anti_paladin_skills[];
extern const char * const avatar_skills[];
extern const char * const bard_skills[];
extern const char * const commando_skills[];

extern char *wpn_spc[];
extern int mob_id_hp_int[];
extern char *mob_id_hp_text[];
extern int mob_id_dmg_int[];
extern char *mob_id_dmg_text[];
extern int mob_id_level_int[];
extern char *mob_id_level_text[];
extern int mob_id_matt_int[];
extern char *mob_id_matt_text[];
extern char *mob_id_spec[];

extern struct message_list fight_messages[MAX_MESSAGES];

extern byte backstab_mult[58];
extern byte circle_mult[58];
extern byte ambush_mult[58];
extern byte assault_mult[58];

extern char *BKColor[];
extern char *Color[];

extern char *dirs[];
extern int rev_dir[];

extern char *where[];
extern const char * const drinks[];
extern char *drinknames[];
extern int drink_aff[][3];

extern char *color_liquid[];
extern char *fullness[];

extern const struct title_type titles[11][58];

extern int movement_loss[];

extern const char *weekdays[];
extern const char *month_name[];
extern char *Month[];

#endif /* _CONSTANTS_H_ */
