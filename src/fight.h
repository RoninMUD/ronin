/* ************************************************************************
*  File: fight.h , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#ifndef __FIGHT_H__
#define __FIGHT_H__

#define TYPE_KILL -999

#define HIT_FAILURE  0
#define HIT_SUCCESS  1
#define HIT_CRITICAL 2

char *fread_string(FILE *f1);
void stop_follower(struct char_data *ch);
void do_flee(CHAR *ch, char *argument, int cmd);
void die(struct char_data *ch);
void appear(struct char_data *ch);
void update_pos(CHAR *victim);
void set_fighting(CHAR *ch, struct char_data *vict);
void stop_fighting(CHAR *ch);
void make_corpse(struct char_data *ch);
void change_alignment(struct char_data *ch, struct char_data *victim);
void death_cry(struct char_data *ch);
void death_list(CHAR *ch);
void raw_kill_ex(CHAR *ch, bool statue);
void raw_kill(CHAR *ch);
void divide_experience(struct char_data *ch, struct char_data *victim, int none);
void gain_exp(CHAR *ch, int gain);
char *replace_string(char *str, char *weapon);
void dam_message(int dam, struct char_data *ch, struct char_data *victim, int w_type, int shadow);
void process_death(struct char_data *ch, struct char_data *victim);

bool is_immune(CHAR *ch, int attack_type, int damage_type);
bool is_resistant(CHAR *ch, int attack_type, int damage_type);
int resist_damage(CHAR *ch, int dmg, int attack_type, int damage_type);
int damage(CHAR *ch, CHAR *victim, int dmg, int attack_type, int damage_type);

void perform_violence(void);

int calc_hitroll(CHAR *ch);
int calc_damroll(CHAR *ch);
int calc_thaco(CHAR *ch);
int calc_ac(CHAR *ch);
int calc_position_damage(int position, int dam);
int calc_hit_damage(CHAR *ch, CHAR *victim, OBJ *weapon, int bonus, int mode);

int stack_position(CHAR *ch, int target_position);

int try_hit(CHAR *ch, CHAR *victim);
bool perform_hit(CHAR *attacker, CHAR *defender, int type, int hit_num);
void hit(CHAR *ch, CHAR *victim, int type);
void dhit(CHAR *ch, CHAR *victim, int type);
void thit(CHAR *ch, CHAR *victim, int type);
void qhit(CHAR *ch, CHAR *victim, int type);

bool mob_disarm(struct char_data *mob, struct char_data *victim, bool to_ground);

int get_attack_type(CHAR *ch, OBJ *weapon);

extern struct char_data *combat_list;

#endif /* __FIGHT_H__ */
