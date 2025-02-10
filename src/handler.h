/* ************************************************************************
*  file: handler.h , Handler module.                      Part of DIKUMUD *
*  Usage: Various routines for moving about objects/players               *
************************************************************************* */

/*
$Author: ronin $
$Date: 2005/01/21 14:55:27 $
$Header: /home/ronin/cvs/ronin/handler.h,v 2.1 2005/01/21 14:55:27 ronin Exp $
$Id: handler.h,v 2.1 2005/01/21 14:55:27 ronin Exp $
$Name:  $
$Log: handler.h,v $
Revision 2.1  2005/01/21 14:55:27  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.0.0.1  2004/02/05 16:13:09  ronin
Reinitialization of cvs archives


Revision 22-May-03 Ranger
Addition of get_char_vis_zone

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


/* handling the affected-structures */
void affect_total(CHAR *ch);
void remove_all_affects(CHAR *ch);
void affect_modify(CHAR *ch, int loc, int mod, long bitv, long bitv2, bool add);
void affect_to_char(CHAR *ch, AFF *af);
void affect_remove(CHAR *ch, AFF *af);
void affect_from_char(CHAR *ch, int skill);
bool affected_by_spell(CHAR *ch, int skill);
int duration_of_spell(CHAR *ch, int skill);
void affect_join(CHAR *ch, AFF *af, bool avg_dur, bool avg_mod);
AFF *get_affect_from_char(CHAR *ch, int type);
void affect_apply(CHAR *ch, int type, sh_int duration, sbyte modifier, byte location, long bitvector, long bitvector2);

/* utility */
OBJ *create_gold(uint64_t amount);
OBJ *create_money(int amount);
int isname(char *str, char *namelist);
char *fname(char *namelist);
int get_number(char **name);
char *rem_prefix(char *str);
/*strings*/

void init_string_block(struct string_block *sb);
void destroy_string_block(struct string_block *sb);
void page_string_block(struct string_block *sb,struct char_data *ch);
void append_to_string_block(struct string_block *sb, char *str);
/* ******** objects *********** */
void adjust_obj_list(struct obj_data *object,struct obj_data *list);

void obj_to_char(OBJ *object, CHAR *ch);
OBJ *obj_from_char(OBJ *object);

bool equip_char_ex(CHAR *ch, OBJ *obj, int pos, bool zap);
void equip_char(CHAR *ch, OBJ *obj, int pos);
bool rent_equip_char(CHAR *ch, OBJ *obj, int pos);
OBJ *unequip_char(CHAR *ch, int pos);

struct obj_data *get_obj_in_list(char *name, struct obj_data *list);
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);
struct obj_data *get_obj(char *name);
struct obj_data *get_obj_num(int nr);

void obj_to_room(struct obj_data *object, int room);
void obj_from_room(struct obj_data *object);
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void obj_from_obj(struct obj_data *obj);
void object_list_new_owner(struct obj_data *list, struct char_data *ch);

void extract_obj(struct obj_data *obj);

/* ******* characters ********* */

struct char_data *get_char_room(char *name, int room);
struct char_data *get_char_num(int nr);
struct char_data *get_char(char *name);

void char_from_room(CHAR *ch);
void char_to_room(CHAR *ch, int room);

/* find if character can see */
struct char_data *get_mortal_room_vis(struct char_data *ch, char *name);
struct char_data *get_char_vis_zone(struct char_data *ch, char *name);
struct char_data *get_mob_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_vis_in_rooms(struct char_data *ch, char *name);

void extract_char(struct char_data *ch);

/* Generic Find */

int generic_find(char *arg, int bitvector, CHAR *ch, CHAR **tar_ch, OBJ **tar_obj);

#define FIND_NOT_FOUND  0
#define FIND_CHAR_ROOM  1
#define FIND_CHAR_WORLD 2
#define FIND_OBJ_INV    4
#define FIND_OBJ_ROOM   8
#define FIND_OBJ_WORLD  16
#define FIND_OBJ_EQUIP  32
//#define FIND_CHAR_INVIS 64
//#define FIND_OBJ_INVIS  128

int dot_number(char *str, char **sub_ptr);

struct char_data *get_mob_by_vnum_in_room(int mob_vnum, int rm);
struct obj_data *get_obj_by_vnum_in_room(int obj_vnum, int rm);

OBJ *get_obj_in_list_ex(CHAR *ch, char *name, OBJ *list, bool must_see);
OBJ *get_obj_in_list_vis(CHAR *ch, char *name, OBJ *list);
OBJ *get_obj_equipped_by_name(CHAR* ch, char* obj_name);


CHAR *get_char_room_ex(CHAR *ch, char *name, bool must_see);
CHAR *get_char_room_vis(CHAR *ch, char *name);

CHAR *get_char_ex(CHAR *ch, char *name, bool must_see);
CHAR *get_char_vis(CHAR *ch, char *name);
