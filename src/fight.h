/* ************************************************************************
*  File: fight.h , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#ifndef __FIGHT_H__
#define __FIGHT_H__

#define TYPE_KILL -999

char *fread_string(FILE *f1);
void stop_follower(struct char_data *ch);
void do_flee(struct char_data *ch, char *argument, int cmd);
void hit(struct char_data *ch, struct char_data *victim, int type);
void die(struct char_data *ch);
void appear(struct char_data *ch);
void load_messages(void);
void update_pos( struct char_data *victim );
void set_fighting(struct char_data *ch, struct char_data *vict);
void stop_fighting(struct char_data *ch);
void make_corpse(struct char_data *ch);
void change_alignment(struct char_data *ch, struct char_data *victim);
void death_cry(struct char_data *ch);
void raw_kill(struct char_data *ch);
void divide_experience(struct char_data *ch, struct char_data *victim, int none);
void gain_exp(struct char_data *ch, int gain);
char *replace_string(char *str, char *weapon);
void dam_message(int dam, struct char_data *ch, struct char_data *victim, int w_type, int shadow);
void process_death(struct char_data *ch, struct char_data *victim);
int damage(struct char_data *ch, struct char_data *victim, int dmg, int attack_type, int damage_type);
void hit(struct char_data *ch, struct char_data *victim, int type);
void dhit(struct char_data *ch, struct char_data *victim, int type);
void thit(struct char_data *ch, struct char_data *victim, int type);
void qhit(struct char_data *ch, struct char_data *victim, int type);
void perform_violence(void);
int calc_hitroll(struct char_data *ch);
int calc_damroll(struct char_data *ch);
int calc_position_damage(int position, int dam);
int stack_position(struct char_data *ch, int target_position);
bool try_hit(struct char_data *ch, struct char_data *victim);
int compute_thaco(struct char_data *ch);
int compute_ac(struct char_data *ch);

extern struct char_data *combat_list;

#endif /* __FIGHT_H__ */
