/* ************************************************************************
*  file: utility.h, Utility module.                       Part of DIKUMUD *
*  Usage: Utility procedures                                              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2005/01/21 14:55:30 $
$Header: /home/ronin/cvs/ronin/utility.h,v 2.2 2005/01/21 14:55:30 ronin Exp $
$Id: utility.h,v 2.2 2005/01/21 14:55:30 ronin Exp $
$Name:  $
$Log: utility.h,v $
Revision 2.2  2005/01/21 14:55:30  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.1  2004/03/13 05:27:13  pyro
updated for olc commands oname and owear

Revision 2.0.0.1  2004/02/05 16:13:42  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/


#ifndef __UTILITY_H
#define __UTILITY_H

typedef struct char_data CHAR;
typedef struct obj_data OBJ;
typedef struct room_data Room;
typedef struct enchantment_type_5 ENCH;
typedef struct affected_type_5 AFF;
typedef struct social_messg SOC;
typedef struct descriptor_data DESC;
typedef struct follow_type FOL;

/* Common external variables */
extern struct room_data *world;
extern struct mob_proto *mob_proto_table;
extern struct obj_proto *obj_proto_table;
extern int rev_dir[];
extern char *affected_bits[];
extern char *affected_bits2[];
extern char *action_bits[];
extern char *action_bits2[];
extern char *extra_bits[];
extern char *wear_bits[];
extern char *apply_types[];
extern char *item_types[];
extern char *sector_types[];
extern char *sector_types_i[];
extern char *room_bits[];
int MIN(int a, int b);
int MAX(int a, int b);
int number(int from, int to) ;
bool chance(int num);
bool breakthrough(CHAR *ch, CHAR *vict,int btype);
int dice(int number, int size) ;
char *str_dup(char *source);

void WAIT_STATE(CHAR *ch,int cycle);

int str_cat(char *s, int len, int maxlen, const char *append);
int str_cmp(char *arg1, char *arg2);
int strn_cmp(char *arg1, char *arg2, int n);
char *str_upper(char *str);
char *string_to_lower(char *string);
char *string_to_upper(char *string);
char *PERS(CHAR *ch, CHAR*vict);
char *POSSESS(CHAR *ch, CHAR*vict);
char *CHCLR(CHAR *ch, int color);
char *ENDCHCLR(CHAR *ch);
void log_s(char *str);
void log_f(char * fmt, ...) __attribute__ ((format(printf,1,2)));
void log_cmd(char *file,char *fmt, ...) __attribute__ ((format(printf,2,3)));
void deathlog(char *str);
void wizinfo(char *str, int level);
void wizlog(char *str, int level, int which);
void sprintbit(long vektor, char *names[], char *result);
void sprinttype(int type, char *names[], char *result);
int IS_CARRYING_W(struct char_data *ch);
int IS_CARRYING_N(struct char_data *ch);
int COUNT_CONTENTS(struct obj_data *obj);
int COUNT_RENTABLE_CONTENTS(struct obj_data *obj);
struct time_info_data real_time_passed(time_t t2, time_t t1);
struct time_info_data mud_time_passed(time_t t2, time_t t1);
struct time_info_data age(struct char_data *ch);

struct obj_data *EQ( struct char_data *ch, int loc );
char is_carrying_obj(struct char_data *ch, int virtual);
int count_carrying_obj(struct char_data *ch, int virtual);
char is_wearing_obj(struct char_data *ch, int virtual, int loc);

void drain_mana_hit_mv(struct char_data *ch,struct char_data *vict,int m,
  int h,int mv,bool add_m,bool add_hp,bool add_mv);

struct char_data *get_ch (int virtual, int scope, int location) ;
struct char_data *get_ch_world (int virtual) ;
struct char_data *get_ch_zone (int virtual, int zone) ;
struct char_data *get_ch_room (int virtual, int realroom) ;

int V_OBJ(struct obj_data *o) ;
int V_MOB(struct char_data *m) ;
int V_ROOM(struct char_data *m) ;
int IS_DARK(int room);
int IS_LIGHT(int room);
int CAN_SEE(struct char_data *ch,struct char_data *vict);
int CAN_TAKE(struct char_data *ch,struct obj_data *obj);
struct obj_data *get_obj_room (int virtual, int loc) ;
struct obj_data *get_obj_world(int virtual) ;

int count_mob_followers(struct char_data *ch) ;
int count_mortals (struct char_data *ch, byte scope, bool see_invis) ;
int count_mortals_room (struct char_data *ch, bool see_invis) ;
int count_mortals_room_fighting (struct char_data *ch, bool see_invis) ;
int count_mortals_zone (struct char_data *ch, bool see_invis) ;
int count_mortals_world (struct char_data *ch, bool see_invis) ;
int count_mortals_real_room (int room) ;
int count_mobs_real_room (int room) ;
int count_mobs_real_room_except_followers(int room);

struct obj_data *get_random_obj_in_list (struct obj_data *list) ;
struct obj_data *get_random_obj_room (struct char_data *ch) ;
struct obj_data *get_random_obj_inv (struct char_data *ch) ;
int get_random_obj_eq (struct char_data *ch) ;

struct char_data *get_random_victim( struct char_data *ch ) ;
struct char_data *get_random_victim_fighting( struct char_data *ch ) ;

struct char_data *get_random_target( struct char_data *ch, bool see_invis, bool vict_canbe_pc, bool vict_canbe_npc, bool vict_canbe_mount, bool vict_canbe_ch, bool see_imm ) ;
struct char_data *get_random_target_fighting( struct char_data *ch, bool see_invis, bool vict_canbe_pc, bool vict_canbe_npc, bool vict_canbe_mount, bool see_imm ) ;

void move_eq_from_to (struct char_data *fch, struct char_data *tch) ;
void move_inv_from_to (struct char_data *fch, struct char_data *tch) ;

void set_item_value (struct obj_data *o, int val, int new) ;

void move_objs_to_room (int frm, int to) ;
void move_chars_to_room (int frm, int to) ;

int ISCARRYING_W(struct char_data *ch);
int GETOBJ_WEIGHT(struct obj_data *obj);
int CHAR_HAS_LEGS(struct char_data *ch);
int CORPSE_HAS_TROPHY(struct obj_data *obj); /* Used to be CORPSE_HAS_SCALP */
int OSTRENGTH_APPLY_INDEX(struct char_data *ch);
int STRENGTH_APPLY_INDEX(struct char_data *ch);
int IS_GROUPED(struct char_data *ch,struct char_data *vict);

char *how_good(int percent);
void check_equipment(struct char_data *ch);
void produce_core();
void empty_container(struct obj_data *j) ;
void empty_all_containers(struct char_data *ch);
int diff_obj_stats(struct obj_data *obj);

#define PQUEUE_LENGTH    20  /* Max. number of requests in queue */
#define MAX_ARGS         5   /* Max. number of arguments of program */

struct program_info {
  char *args[MAX_ARGS+1];/* args[0] is name of program, rest are arguments,
                            after last argument must be NULL    */
  char *input;           /* Input sended to process             */
  char *chname;          /* Name of character                   */
  char *name;            /* Name used when printing output      */
  int timeout;           /* Timeout in seconds                  */
};

struct char_data *get_ch_by_name(char *chname);
void add_program(struct program_info prg, struct char_data *ch);

struct char_data *get_ch_by_id(int num);

int get_weapon_type(OBJ *obj);
const char const * get_weapon_type_desc(OBJ *obj);

#endif
