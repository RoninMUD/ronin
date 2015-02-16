/*
  meta.h - Specs for Meta, re-written by Night

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 2/5/2012
*/

/* Defines */
#define META_MAX_HIT_MANA_MOVE 32767
#define META_MAX_PRAC          127
#define META_MAX_ABILITY       18
#define META_MAX_STR_ADD       100

#define META_HIT               1
#define META_MANA              2
#define META_MOVE              3
#define META_PRAC              4
#define META_STR               5
#define META_INT               6
#define META_WIS               7
#define META_DEX               8
#define META_CON               9
#define META_STR_ADD           10
#define META_AGE_UP            11
#define META_AGE_DOWN          12
#define META_MOVE_UP_1         13
#define META_MOVE_DOWN_1       14

#define META_MOVE_COST         1000000
#define META_PRAC_COST         300000
#define META_MOVE_UP1_COST     50000
#define META_MOVE_DOWN1_COST   50000

/* Prototypes */
int get_max_stat(CHAR *ch, int stat);
int get_ability(CHAR *ch, int ability);
void adjust_hit_mana_move(CHAR *ch, int stat, int amt);
void adjust_ability(CHAR *ch, int ability, int amt);
int hit_mana_cost(CHAR *ch, int stat);
int meta_cost(CHAR *ch, int choice);
int str_add_cost(CHAR *ch);
int increase_age_cost(CHAR *ch);
int decrease_age_cost(CHAR *ch);
int get_meta_adjust(int choice, int bribe);
