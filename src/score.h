/*
  score.h - Improved do_score() and related functions

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 10/21/2012
*/
/* Project Epee */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "comm.h"
#include "db.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "spec.clan.h"
#include "spells.h"
#include "utility.h"
#include "utils.h"
#include "enchant.h"
#include "remortv2.h"

extern char *skip_spaces(char *string);
extern int hit_gain(CHAR *ch);
extern int mana_gain(CHAR *ch);
extern int move_gain(CHAR *ch);
extern int check_subclass(CHAR *ch, int sub, int lvl);

extern const int exp_table[58];
extern char *pc_class_types[];
extern char *npc_class_types[];
extern char *subclass_name[];
extern char *wiz_bits[];
extern struct str_app_type str_app[];
extern struct dex_app_type dex_app[];
extern struct clan_data clan_list[MAX_CLANS];
extern struct zone_data *zone_table;

#define SCQ_NAME           0
#define SCQ_TITLE          1
#define SCQ_LEVEL          2
#define SCQ_CLASS_NAME     3
#define SCQ_SC_LEVEL       4
#define SCQ_SC_NAME        5
#define SCQ_RANK_LEVEL     6
#define SCQ_RANK_NAME      7
#define SCQ_MAX_HIT        8
#define SCQ_MAX_MANA       9
#define SCQ_MAX_MOVE       10
#define SCQ_NAT_HIT        11
#define SCQ_NAT_MANA       12
#define SCQ_NAT_MOVE       13
#define SCQ_HIT_GAIN       14
#define SCQ_MANA_GAIN      15
#define SCQ_MOVE_GAIN      16
#define SCQ_STR            17
#define SCQ_DEX            18
#define SCQ_CON            19
#define SCQ_INT            20
#define SCQ_WIS            21
#define SCQ_OSTR           22
#define SCQ_ODEX           23
#define SCQ_OCON           24
#define SCQ_OINT           25
#define SCQ_OWIS           26
#define SCQ_MOD_AC         27
#define SCQ_WIMPY_LIMIT    28
#define SCQ_BLEED_LIMIT    29
#define SCQ_SCP            30
#define SCQ_QP             31
#define SCQ_RANKING        32
#define SCQ_BEEN_KILLED    33
#define SCQ_AGE            34
#define SCQ_PRAC           35
#define SCQ_AVG_DAMAGE     36
#define SCQ_DAMROLL        37
#define SCQ_HITROLL        38
#define SCQ_ALIGNMENT      39
#define SCQ_EXP            40
#define SCQ_EXP_TO_LEVEL   41
#define SCQ_REMORT_EXP     42
#define SCQ_DEATH_EXP      43
#define SCQ_GOLD           44
#define SCQ_BANK           45
#define SCQ_QUEST_TIMER    46
#define SCQ_CLUB           47
#define SCQ_CLAN           48
#define SCQ_PLAY_TIME      49
#define SCQ_WEIGHT_CARRIED 50
#define SCQ_TOGGLES        51
#define SCQ_FLAGS          52
#define SCQ_DEATH_LIMIT    53
#define SCQ_POSITION       54
#define SCQ_WIZINV         55
#define SCQ_EDITING_ZONE   56
#define SCQ_REMORT_MULT    57

int calc_bhd(CHAR *ch);
int calc_avg_damage(CHAR *ch, int weapon_slot);
void score_query(CHAR *ch, int query, bool opt_text, bool new_line);
void do_score(CHAR *ch, char *argument, int cmd);
