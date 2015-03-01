/* ************************************************************************
*  File: fight.h , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2004/02/05 16:13:08 $
$Header: /home/ronin/cvs/ronin/fight.h,v 2.0.0.1 2004/02/05 16:13:08 ronin Exp $
$Id: fight.h,v 2.0.0.1 2004/02/05 16:13:08 ronin Exp $
$Name:  $
$Log: fight.h,v $
Revision 2.0.0.1  2004/02/05 16:13:08  ronin
Reinitialization of cvs archives

Revision 1.3  2002/06/18 14:32:20  ronin
Adding divide_experience before raw_kill to ensure proper quest
completion.  Addition of flag within divide_experience to force
amount to 0 if required.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

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
void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type,int shadow);
void damage(struct char_data *ch, struct char_data *victim,
            int dam, int attacktype, int damtype);
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

#endif /* __FIGHT_H__ */
