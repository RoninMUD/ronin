/*
  remortv2.h - Specs for Remort v2, by Night

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 2/5/2012
*/

#ifndef __REMORTV2_H__
#define __REMORTV2_H__

/* Defines */
/* Mobiles */
#define RV2_MOB_IMMORTALIS         3098

/* Objects */
#define RV2_OBJ_TOKEN_NAME         86
#define RV2_OBJ_TOKEN_MALE         87
#define RV2_OBJ_TOKEN_FEMALE       88
#define RV2_OBJ_TOKEN_AVATAR       89
#define RV2_OBJ_TOKEN_MAGE         90
#define RV2_OBJ_TOKEN_CLERIC       91
#define RV2_OBJ_TOKEN_THIEF        92
#define RV2_OBJ_TOKEN_WARRIOR      93
#define RV2_OBJ_TOKEN_NINJA        94
#define RV2_OBJ_TOKEN_NOMAD        95
#define RV2_OBJ_TOKEN_PALADIN      96
#define RV2_OBJ_TOKEN_ANTI_PALADIN 97
#define RV2_OBJ_TOKEN_BARD         98
#define RV2_OBJ_TOKEN_COMMANDO     99

/* Rooms */
#define RV2_WLD_SANCTUM_ETERNITY   3098

/* Misc */
#define RV2_TOKEN_MIN_VNUM         86
#define RV2_TOKEN_MAX_VNUM         99

#define RV2_MIN_EXP_COST           700000000
#define RV2_MAX_EXP_COST           8000000000
#define RV2_MAX_QP_COST            200
#define RV2_MAX_SCP_COST           70

#define RV2_SIM_AVG_STAT           13

#define RV2_EXP_MULTIPLIER         4
#define RV2_MAX_EXP_GAIN           100000000

/* Structs */
struct rv2_token_info
{
  /* OBJ *name_token; */
  OBJ *gender_token;
  OBJ *class_token;
  int gender;
  int class_num;
  char gender_name[MIL];
  char class_name[MIL];
};

struct rv2_remort_info
{
  long long int exp;
  int qp;
  int scp;
  int qp_fee;
  int scp_fee;
};

/* Prototypes */
void rv2_sim_advance_level(CHAR *ch);
int rv2_adjust_remort_exp(CHAR *ch, int amt);
int rv2_gain_remort_exp(CHAR *ch, int exp);
long long int rv2_meta_sim(CHAR *ch);
int rv2_get_ability_by_apply(CHAR *ch, int ability_apply);
void rv2_extract_token(OBJ *obj);
struct rv2_token_info rv2_get_token_info(CHAR *ch);
int rv2_token_check(CHAR *ch, struct rv2_token_info token_info);
struct rv2_remort_info rv2_appraise(CHAR *ch);
void rv2_appraise_message(CHAR *ch, struct rv2_remort_info remort_info);
void rv2_remove_enchant(CHAR *ch);
void rv2_add_enchant(CHAR *ch);
int rv2_calc_remort_mult(CHAR *ch);

#endif /* __REMORTV2_H__ */
