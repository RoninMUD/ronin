/* ************************************************************************
*  file: utils.h, Utility module.                         Part of DIKUMUD *
*  Usage: Utility macros                                                  *
************************************************************************* */

/*
$Author: ronin $
$Date: 2004/02/05 16:13:43 $
$Header: /home/ronin/cvs/ronin/utils.h,v 2.0.0.1 2004/02/05 16:13:43 ronin Exp $
$Id: utils.h,v 2.0.0.1 2004/02/05 16:13:43 ronin Exp $
$Name:  $
$Log: utils.h,v $
Revision 2.0.0.1  2004/02/05 16:13:43  ronin
Reinitialization of cvs archives


Revision - Addition of GET_EMAIL(ch) - 19-Dec-03 Ranger

Revision - Addition of RM_BLOOD macro 26-Nov-02 Ranger

Revision - removed commented text from IMP_INV

Revision 1.3  2002/04/18 04:07:31  ronin
Changing log output from perror to log_f for internal syslog manipulation.

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/

#ifndef __UTILS_H__
#define __UTILS_H__


#include "utility.h"

extern char *index(const char *s, int c);

#define CHAR_REAL_ROOM(char_s)      (char_s ? (char_s)->in_room_r : NOWHERE)
#define CHAR_VIRTUAL_ROOM(char_s)   (char_s ? (char_s)->in_room_v : 0)

#define MOB_NAME(o)                 ((o)->player.name ? (o)->player.name : mob_proto_table[(o)->nr].name)
#define MOB_DESCRIPTION(o)          (o->player.description ? o->player.description : mob_proto_table[o->nr].description)
#define MOB_SHORT(o)                (o->player.short_descr ? o->player.short_descr : mob_proto_table[o->nr].short_descr)
#define MOB_LONG(o)                 (o->player.long_descr ? o->player.long_descr : mob_proto_table[o->nr].long_descr)
#define MOB_NUM_ATTACKS(mob)        (mob->specials.no_att)
#define MOB_ATTACK_TIMER(mob)       (mob->specials.att_timer)
#define MOB_ATTACK_CHANCE(mob, num) (mob->specials.att_percent[num])
#define MOB_ATTACK_TYPE(mob, num)   (mob->specials.att_type[num])
#define MOB_ATTACK_TARGET(mob, num) (mob->specials.att_target[num])
#define MOB_ATTACK_SPELL(mob, num)  (mob->specials.att_spell[num])

#define OBJ_VNUM(obj)           (obj->item_number_v)
#define OBJ_RNUM(obj)           (obj->item_number)
#define OBJ_NUM_IN_GAME(obj)    (((OBJ_RNUM(obj) > -1) && (OBJ_RNUM(obj) < top_of_objt)) ? obj_proto_table[OBJ_RNUM(obj)].number : 0)
#define OBJ_FUNC(obj)           (obj->func)
#define OBJ_NAME(obj)           (obj->name ? obj->name : obj_proto_table[obj->item_number].name)
#define OBJ_DESCRIPTION(obj)    (obj->description ? obj->description : obj_proto_table[obj->item_number].description)
#define OBJ_SHORT(obj)          (obj->short_description ? obj->short_description : obj_proto_table[obj->item_number].short_description)
#define OBJ_ACTION(obj)         (obj->action_description ? obj->action_description : obj_proto_table[obj->item_number].action_description)
#define OBJ_ACTION_NT(obj)      (obj->action_description_nt ? obj->action_description_nt : obj_proto_table[obj->item_number].action_description_nt)
#define OBJ_CWEAR_DESC(obj)     (obj->char_wear_desc ? obj->char_wear_desc : obj_proto_table[obj->item_number].char_wear_desc)
#define OBJ_RWEAR_DESC(obj)     (obj->room_wear_desc ? obj->room_wear_desc : obj_proto_table[obj->item_number].room_wear_desc)
#define OBJ_CREM_DESC(obj)      (obj->char_rem_desc ? obj->char_rem_desc : obj_proto_table[obj->item_number].char_rem_desc)
#define OBJ_RREM_DESC(obj)      (obj->room_rem_desc ? obj->room_rem_desc : obj_proto_table[obj->item_number].room_rem_desc)
#define OBJ_WEAR_FLAGS(obj)     (obj->obj_flags.wear_flags)
#define OBJ_SUBCLASS_RES(obj)   (obj->obj_flags.subclass_res)
#define OBJ_BITS(obj)           (obj->obj_flags.bitvector)
#define OBJ_BITS2(obj)          (obj->obj_flags.bitvector2)
#define OBJ_EXTRA_FLAGS(obj)    (obj->obj_flags.extra_flags)
#define OBJ_EXTRA_FLAGS2(obj)   (obj->obj_flags.extra_flags2)
#define OBJ_REPOP(obj)          (obj->obj_flags.repop_percent)
#define OBJ_WEIGHT(obj)         (obj->obj_flags.weight)
#define OBJ_MATERIAL(obj)       (obj->obj_flags.material)
#define OBJ_COST(obj)           (obj->obj_flags.cost)
#define OBJ_RENT_COST(obj)      (obj->obj_flags.cost_per_day)
#define OBJ_TYPE(obj)           (obj->obj_flags.type_flag)
#define OBJ_AFF(obj, num)       (obj->affected[num])
#define OBJ_AFF0(obj)           (obj->affected[0])
#define OBJ_AFF1(obj)           (obj->affected[1])
#define OBJ_AFF2(obj)           (obj->affected[2])
#define OBJ_AFF_LOC(obj, num)   (((num > -1) && (num < MAX_OBJ_AFFECT)) ? obj->affected[num].location : 0)
#define OBJ_AFF_MOD(obj, num)   (((num > -1) && (num < MAX_OBJ_AFFECT)) ? obj->affected[num].modifier : 0)
#define OBJ_VALUE(obj, num)     (obj->obj_flags.value[num])
#define OBJ_VALUE0(obj)         (obj->obj_flags.value[0])
#define OBJ_VALUE1(obj)         (obj->obj_flags.value[1])
#define OBJ_VALUE2(obj)         (obj->obj_flags.value[2])
#define OBJ_VALUE3(obj)         (obj->obj_flags.value[3])
#define OBJ_POPPED(obj)         (obj->obj_flags.popped)
#define OBJ_SPEC(obj)           (obj->spec_value)
#define OBJ_TIMER(obj)          (obj->obj_flags.timer)
#define OBJ_OWNER_ID(obj, num)  (((num > -1) && (num < MAX_OBJ_OWNER_ID)) ? obj->ownerid[num] : 0)
#define OBJ_IN_ROOM(obj)        (obj->in_room)
#define OBJ_IN_OBJ(obj)         (obj->in_obj)
#define OBJ_CARRIED_BY(obj)     (obj->carried_by)
#define OBJ_EQUIPPED_BY(obj)    (obj->equipped_by)
#define OBJ_LOG(obj)            (obj->log)
#define OBJ_REAL_ROOM(obj)      (obj ? obj->in_room : NOWHERE)
#define OBJ_VIRTUAL_ROOM(obj)   (obj ? obj->in_room_v : 0)

#define GET_OPPONENT(ch)        ((ch) ? (ch)->specials.fighting : NULL)
#define GET_ID(ch)              ((ch) ? (ch)->ver3.id : -1)
#define GET_ZONE(ch)            (world[CHAR_REAL_ROOM(ch)].zone)

#define ROOM_SPEC(rm)           (world[(rm)].spec_tmp)
#define RM_BLOOD(rm)            (world[rm].blood)
#define ITEM(zone,x)            ((zone)+(x))

#define IS_NIGHT                (time_info.hours>18||time_info.hours<6)
#define IS_DAY                  !IS_NIGHT
#define ROOM                    1
#define ZONE                    2
#define WORLD                   3

#define TRUE  1
#define FALSE 0

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define NUMELEMS(x)             (sizeof(x) / sizeof(x[0]))

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)
#define LOW(st)  (*(st) = LOWER(*(st)), st)

#define CREATE(result, type, number)                          \
do {                                                          \
  if (!((result) = (type *)calloc((number), sizeof(type)))) { \
    log_f("malloc failure");                                  \
    abort();                                                  \
  }                                                           \
} while (0)

#define RECREATE(result, type, number)                                    \
do {                                                                      \
  if (!((result) = (type *)realloc((result), sizeof(type) * (number)))) { \
    log_f("realloc failure");                                             \
    abort();                                                              \
  }                                                                       \
} while (0)

#define DESTROY(value) \
do {                   \
  if (value != NULL) { \
    free(value);       \
  }                    \
} while (0)            \

#define IS_SET(flag,bit)  ((flag) & (bit))

#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }

#define IS_AFFECTED(ch,skill) (ch ? IS_SET((ch)->specials.affected_by, (skill) ): FALSE )

#define SET_BIT(var,bit)     ((var) = (var) | (bit))
#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

#define WIZ_INV(ch, vict) \
( (((vict)->new.wizinv > GET_LEVEL(ch))  && !IS_NPC(vict)) || \
  (((vict)->new.wizinv > LEVEL_IMM) && IS_NPC(ch)) )

#define IMP_INV(ch, vict) (IS_AFFECTED(vict, AFF_IMINV) && GET_LEVEL(ch) < LEVEL_IMM && !CHAOSMODE)

#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
        "Superb" )))))))

#define HSHR(ch) ((ch)->player.sex ?          \
  (((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) ((ch)->player.sex ?          \
  (((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) ((ch)->player.sex ?           \
  (((ch)->player.sex == 1) ? "him" : "her") : "it")

#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define IS_NPC(ch)         (ch ? IS_SET(ch->specials.act, ACT_ISNPC) : FALSE)
#define IS_MOB(ch)         (ch ? (IS_SET(ch->specials.act, ACT_ISNPC) && (ch->nr > -1)) : FALSE)
#define IS_MOUNT(ch)       (ch ? IS_SET(ch->specials.act, ACT_MOUNT) : FALSE)

#define IS_MORTAL(ch)      (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) < LEVEL_IMM)) : FALSE)

#define IS_IMMORTAL(ch)    (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_IMM)) : FALSE)
#define IS_DEITY(ch)       (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_DEI)) : FALSE)
#define IS_TEMPORAL(ch)    (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_TEM)) : FALSE)
#define IS_WIZARD(ch)      (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_WIZ)) : FALSE)
#define IS_ETERNAL(ch)     (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_ETE)) : FALSE)
#define IS_SUPREME(ch)     (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_SUP)) : FALSE)
#define IS_IMPLEMENTOR(ch) (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_IMP)) : FALSE)

#define GET_POS(ch)        ((ch)->specials.position)
#define GET_COND(ch, i)    ((ch)->specials.conditions[(i)])

#define GET_NAME(ch)       (!IS_MOB(ch) ? (ch)->player.name : MOB_NAME(ch))
#define GET_LONG(ch)       (!IS_MOB(ch) ? "none" : MOB_LONG(ch))
#define GET_SHORT(ch)      (!IS_MOB(ch) ? "none" : MOB_SHORT(ch))
#define GET_DISP_NAME(ch)  (!IS_NPC(ch) ? GET_NAME(ch) : GET_SHORT(ch))
#define GET_TITLE(ch)      ((ch)->player.title)
#define GET_EMAIL(ch)      ((ch)->ver3.email_addr)
#define GET_LEVEL(ch)      ((ch)->player.level)
#define GET_CLASS(ch)      ((ch)->player.class)
#define GET_HOME(ch)       ((ch)->player.hometown)
#define GET_AGE(ch)        (age(ch).year)

#define GET_STR(ch)     ((ch)->tmpabilities.str)
#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)
#define GET_DEX(ch)     ((ch)->tmpabilities.dex)
#define GET_INT(ch)     ((ch)->tmpabilities.intel)
#define GET_WIS(ch)     ((ch)->tmpabilities.wis)
#define GET_CON(ch)     ((ch)->tmpabilities.con)
#define GET_WIMPY(ch)   ((ch)->new.wimpy)
#define GET_OSTR(ch)    ((ch)->abilities.str)
#define GET_OADD(ch)    ((ch)->abilities.str_add)
#define GET_ODEX(ch)    ((ch)->abilities.dex)
#define GET_OINT(ch)    ((ch)->abilities.intel)
#define GET_OWIS(ch)    ((ch)->abilities.wis)
#define GET_OCON(ch)    ((ch)->abilities.con)

#define GET_AC(ch)      ((ch)->points.armor)

#define GET_HIT(ch)     ((ch)->points.hit)
#define GET_MAX_HIT(ch) (hit_limit(ch))

#define GET_MOVE(ch)    ((ch)->points.move)
#define GET_MAX_MOVE(ch) (move_limit(ch))

#define GET_MANA(ch)    ((ch)->points.mana)
#define GET_MAX_MANA(ch) (mana_limit(ch))

#define GET_GOLD(ch)    ((ch)->points.gold)
#define GET_BANK(ch)    ((ch)->points.bank)

#define GET_EXP(ch)     ((ch)->points.exp)

#define GET_HEIGHT(ch)  ((ch)->player.height)
#define GET_WEIGHT(ch)  ((ch)->player.weight)

#define GET_SEX(ch)     ((ch)->player.sex)

#define GET_HITROLL(ch) ((ch)->points.hitroll)
#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)

/* Object And Carry related macros */

#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, wear_flag) (IS_SET((obj)->obj_flags.wear_flags, wear_flag))

#define IS_OBJ_STAT(obj, stat) (IS_SET((obj)->obj_flags.extra_flags, stat))

#define CAN_SEE_OBJ(ch, obj) ( \
  IS_IMMORTAL(ch) || \
  IS_AFFECTED((ch), AFF_INFRAVISION) || \
  ((!IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_INVISIBLE) || IS_AFFECTED((ch), AFF_DETECT_INVISIBLE)) && !IS_AFFECTED((ch), AFF_BLIND) && IS_LIGHT(CHAR_REAL_ROOM(ch))) \
)

#define CAN_CARRY_W(ch) (\
  IS_IMMORTAL(ch) ? 10000 : \
  (ch->ver3.prestige >= 20) ? (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w * 1.1) : str_app[STRENGTH_APPLY_INDEX(ch)].carry_w) // Prestige Perk 20

#define CAN_CARRY_N(ch) (\
  IS_IMMORTAL(ch) ? 200 : \
  ((((5 + GET_DEX(ch)) / 2) + (GET_LEVEL(ch) / 2)) < 20) ? 20 : \
  (ch->ver3.prestige >= 14) ? ((((5 + GET_DEX(ch)) / 2) + (GET_LEVEL(ch) / 2)) * 1.1) : (((5 + GET_DEX(ch)) / 2) + (GET_LEVEL(ch) / 2))) // Prestige Perk 14

#define CAN_CARRY_OBJ(ch, obj) ( \
  IS_IMMORTAL(ch) || \
  ((((IS_CARRYING_W(ch) + GETOBJ_WEIGHT(obj)) <= (3 * CAN_CARRY_W(ch))) && ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))) \
)

#define CAN_GET_OBJ(ch, obj) \
(CAN_TAKE((ch), (obj)) && CAN_CARRY_OBJ((ch), (obj)) && CAN_SEE_OBJ((ch), (obj)))

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
  OBJ_SHORT((obj)) : "something")

#define OBJS2(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
  rem_prefix(OBJ_SHORT((obj))) : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
  fname(OBJ_NAME((obj))) : "something")

#define IS_INDOORS(ch) (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, INDOORS) || world[CHAR_REAL_ROOM(ch)].sector_type == SECT_INSIDE)
#define IS_OUTSIDE(ch) (!IS_INDOORS(ch))

#define EXIT(ch, door)  (world[CHAR_REAL_ROOM(ch)].dir_option[door])

#define CAN_GO(ch, door)                          \
(EXIT(ch, door) &&                                \
 EXIT(ch, door)->to_room_r != NOWHERE &&          \
 EXIT(ch, door)->to_room_v != 0 &&                \
 !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) && \
 !IS_SET(EXIT(ch, door)->exit_info, EX_CLIMB) &&  \
 !IS_SET(EXIT(ch, door)->exit_info, EX_CRAWL) &&  \
 !IS_SET(EXIT(ch, door)->exit_info, EX_ENTER))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define COLORNAME(ch,i) (Color[(ch->colors[i]+1)])
#define COLOR(ch,i) if(ch->colors[0]&&ch->colors[i]) {send_to_char(Color[(((ch->colors[i])*2)-2)],ch);send_to_char(BKColor[ch->colors[13]],ch);}
#define ENDCOLOR(ch) if(ch->colors[0]&&ch->colors[1]) {COLOR(ch,1);send_to_char(BKColor[ch->colors[13]],ch);}

#define IS_RENTABLE(obj) ( (obj) ? (((obj)->item_number > -1) &&\
                             (!IS_OBJ_STAT((obj), ITEM_ANTI_RENT)     ) &&\
                             ((obj)->obj_flags.type_flag != ITEM_FOOD ) &&\
                             ((obj)->obj_flags.type_flag != ITEM_SC_TOKEN ) &&\
                             ((obj)->obj_flags.type_flag != ITEM_TRASH) &&\
                             ((obj)->obj_flags.type_flag != ITEM_KEY  )   ) : 0 )

#define GET_OBJ_WEIGHT(obj)     ((obj)->obj_flags.weight)
#define strMove(d,s) memmove(d,s,strlen(s)+1)

#define ROOM_CHAOTIC(rm) (CHAOSMODE || ((rm != NOWHERE) ? IS_SET(world[rm].room_flags, CHAOTIC) : FALSE))
#define ROOM_SAFE(rm) (!CHAOSMODE && ((rm != NOWHERE) ? IS_SET(world[rm].room_flags, SAFE) : FALSE))
#define ROOM_ARENA(rm) ((rm != NOWHERE) ? IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA) : FALSE)

#define GET_CLASS_NAME(ch) (!IS_NPC(ch) ? pc_class_types[(int)GET_CLASS(ch)] : npc_class_types[(int)GET_CLASS(ch)])
#define GET_NAT_HIT(ch) (ch->specials.org_hit)
#define GET_NAT_MANA(ch) (100 + ch->specials.org_mana)
#define GET_NAT_MOVE(ch) (100 + ch->specials.org_move)
#define GET_MOD_AC(ch) (compute_ac(ch))
#define GET_EXP_TO_LEVEL(ch) (GET_LEVEL(ch) < LEVEL_IMP ? exp_table[GET_LEVEL(ch) + 1] - GET_EXP(ch) : 0)
#define ZONE_NAME_CH(ch) (zone_table[real_zone(ch->specials.zone)].name)
#define ZONE_NUM_CH(ch) (ch->specials.zone)
#define GET_PLAY_TIME(ch) (real_time_passed((time(0) - ch->player.time.logon) + ch->player.time.played, 0))
#define GET_CLAN(ch) (clan_list[(int)GET_CLAN_NUM(ch)])
#define GET_CLAN_NUM(ch) (ch->ver3.clan_num)
#define GET_WIZINV(ch) (ch->new.wizinv)
#define GET_DEATH_LIMIT(ch) (ch->ver3.death_limit)
#define GET_SC(ch) (ch->ver3.subclass)
#define GET_SC_NAME(ch) (subclass_name[GET_SC(ch) - 1])
#define GET_SC_LEVEL(ch) (ch->ver3.subclass_level)
#define GET_RANKING(ch) (ch->ver3.ranking)
#define GET_SCP(ch) (ch->ver3.subclass_points)
#define GET_QP(ch) (ch->ver3.quest_points)
#define GET_PRAC(ch) (ch->specials.spells_to_learn)
#define GET_BLEED(ch) (ch->ver3.bleed_limit)
#define GET_BEEN_KILLED(ch) (ch->new.been_killed)
#define GET_THACO(ch) (thaco[GET_CLASS(ch) - 1][GET_LEVEL(ch)])
#define CHAR_ROOM_FLAGS(ch) (world[CHAR_REAL_ROOM(ch)].room_flags)
#define IS_CORPSE(obj) (obj ? (GET_ITEM_TYPE(obj) == ITEM_CONTAINER && obj->obj_flags.value[3]) : FALSE)
#define S_ANA(string) (index("aeiouyAEIOUY", *string) ? "An" : "A")
#define S_SANA(string) (index("aeiouyAEIOUY", *string) ? "an" : "a")
#define OBJ_TIMER(obj) (obj->obj_flags.timer)
#define GET_REMORT_EXP(ch) (ch->ver3.remort_exp)
#define IS_AFFECTED2(ch, aff) (ch ? IS_SET(ch->specials.affected_by2, aff) : FALSE)
#define GET_DEATH_EXP(ch) (ch->ver3.death_exp)
#define GET_SCORE_STYLE(ch) (ch->ver3.sc_style)
#define GET_CLAN_NAME(ch) ((GET_CLAN_NUM(ch) && GET_CLAN(ch).name) ? GET_CLAN(ch).name : "None")
#define GET_QUEST_STATUS(ch) (ch->quest_status)
#define GET_QUEST_TIMER(ch) (ch->ver3.time_to_quest)
#define GET_IMM_FLAGS(ch) (ch->new.imm_flags)
#define IS_WEAPON(obj) (obj ? (obj->obj_flags.type_flag == ITEM_WEAPON || obj->obj_flags.type_flag == ITEM_2HWEAPON) : FALSE)
#define GET_MOUNT(ch) (ch->specials.riding)
#define GET_PFLAG(ch) (ch->specials.pflag)
#define GET_ACT(ch) (ch->specials.act)
#define GET_ACT2(ch) (ch->specials.act2)
#define GET_AFF(ch) (ch->specials.affected_by)
#define GET_AFF2(ch) (ch->specials.affected_by2)
#define OBJ_TYPE(obj) (obj->obj_flags.type_flag)
#define GET_WEAPON(ch) (ch->equipment[WIELD])
#define GET_DEX_APP(ch) (dex_app[GET_DEX(ch)].prac_bonus)
#define GET_WIS_APP(ch) (wis_app[GET_WIS(ch)].bonus)
#define GET_LEARNED(ch, skill) (ch->skills[skill].learned)
#define IS_IMMUNE(ch, immunity) (ch ? IS_SET(ch->specials.immune, immunity) : FALSE)
#define IS_IMMUNE2(ch, immunity) (ch ? IS_SET(ch->specials.immune2, immunity) : FALSE)
#define IS_RESISTANT(ch, resistance) (ch ? IS_SET(ch->specials.resist, resistance) : FALSE)
#define GET_MASTER(ch) (ch->master)
#define GET_RIDER(ch) (ch->specials.rider)
#define SAME_ROOM(ch1, ch2) ((CHAR_REAL_ROOM(ch1) == CHAR_REAL_ROOM(ch2)) && (CHAR_REAL_ROOM(ch1) != NOWHERE))
#define IS_ALIVE(ch) (ch && (CHAR_REAL_ROOM(ch) != NOWHERE) && (GET_POS(ch) > POSITION_DEAD) ? TRUE : FALSE)
#define IS_DEAD(ch) (!IS_ALIVE(ch))
#define GET_PROTECTOR(ch) (ch->specials.protect_by)
#define GET_PROTECTEE(ch) (ch->specials.protecting)
#define GET_IMMUNE(ch) (ch->specials.immune)
#define GET_IMMUNE2(ch) (ch->specials.immune2)
#define GET_RESIST(ch) (ch->specials.resist)
#define GET_DESCRIPTOR(ch) (ch->desc)
#define GET_WAS_IN_ROOM(ch) (ch->specials.was_in_room)
#define GET_SKILLS(ch) (ch->skills)
#define GET_ROOM_PEOPLE(room) (world[room].people)
#define GET_ROOM_FLAGS(room) (world[room].room_flags)
#define GET_OBJ_BITS(obj) (obj->obj_flags.bitvector)
#define GET_OBJ_BITS2(obj) (obj->obj_flags.bitvector2)
#define GET_LAST_DIR(mob) (mob->specials.last_direction)
#define GET_PRESTIGE(ch) (ch->ver3.prestige)
#define GET_PRESTIGE_PERK(ch) ((GET_PRESTIGE(ch) >= 5) ? (GET_PRESTIGE(ch) - 5 / 10) : 0)
#define GET_WHO_FILTER(ch) (ch->ver3.who_filter)

#define FIELD_SIZE(t,f) sizeof(((struct t*)0)->f)

#endif /* __UTILS_H__ */
