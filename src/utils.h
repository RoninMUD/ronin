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

Revision - Addition of ROOM_BLOOD macro 26-Nov-02 Ranger

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
#include "spells.h"

extern char *index(const char *s, int c);

#define TRUE                    1
#define FALSE                   0

#define LOWER(c)                (((c) >= 'A' && (c) <= 'Z') ? ((c) + ('a' - 'A')) : (c))
#define UPPER(c)                (((c) >= 'a' && (c) <= 'z') ? ((c) + ('A' - 'a')) : (c))
#define CAP(st)                 (*(st) = UPPER(*(st)), st)
#define LOW(st)                 (*(st) = LOWER(*(st)), st)

#define ISNEWL(ch)              ((ch) == '\n' || (ch) == '\r')

#define NUMELEMS(x)             (sizeof(x) / sizeof(x[0]))

#define FIELD_SIZE(t, f)        (sizeof(((struct t*)0)->f))

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
} while (0)

#define IS_NIGHT                 (weather_info.sunlight != SUN_LIGHT)
#define IS_DAY                   !IS_NIGHT

#define ROOM                     1
#define ZONE                     2
#define WORLD                    3

#define CHAR_REAL_ROOM(ch)       ((ch) ? (ch)->in_room_r : NOWHERE)
#define CHAR_VIRTUAL_ROOM(ch)    ((ch) ? (ch)->in_room_v : 0)

#define OBJ_REAL_ROOM(obj)       ((obj) ? (obj)->in_room : NOWHERE)
#define OBJ_VIRTUAL_ROOM(obj)    ((obj) ? (obj)->in_room_v : 0)

#define DESC_DESCRIPTOR(desc)    (desc->descriptor)
#define DESC_TIMER(desc)         (desc->timer)
#define DESC_HOST(desc)          (desc->host)
#define DESC_USER_ID(desc)       (desc->userid)
#define DESC_PORT(desc)          (desc->port)
#define DESC_ADDR(desc)          (desc->addr)
#define DESC_WIZINFO(desc)       (desc->wizinfo)
#define DESC_CONNECTED(desc)     (desc->connected)
#define DESC_WAIT(desc)          (desc->wait)
#define DESC_SHOWSTR_HEAD(desc)  (desc->showstr_head)
#define DESC_SHOWSTR_POINT(desc) (desc->showstr_point)
#define DESC_STR(desc)           (desc->str)
#define DESC_MAX_STR(desc)       (desc->max_str)
#define DESC_PROMPT(desc)        (desc->prompt)
#define DESC_PROMPT_MODE(desc)   (desc->prompt_mode)
#define DESC_BUF(desc)           (desc->buf)
#define DESC_LAST_INPUT(desc)    (desc->last_input)
#define DESC_OUTPUT_Q(desc)      (desc->output)
#define DESC_INPUT_Q(desc)       (desc->input)
#define DESC_CHAR(desc)          (desc->character)
#define DESC_ORIGINAL(desc)      (desc->original)
#define DESC_SNOOP(desc)         (desc->snoop)
#define DESC_SNOOPING(desc)      (DESC_SNOOP(desc).snooping)
#define DESC_SNOOP_BY(desc)      (DESC_SNOOP(desc).snoop_by)

#define MOB_RNUM(mob)            (mob->nr)
#define MOB_VNUM(mob)            (mob->nr_v)
#define MOB_NAME(mob)            (mob->player.name ? mob->player.name : mob_proto_table[mob->nr].name)
#define MOB_DESCRIPTION(mob)     (mob->player.description ? mob->player.description : mob_proto_table[mob->nr].description)
#define MOB_SHORT(mob)           (mob->player.short_descr ? mob->player.short_descr : mob_proto_table[mob->nr].short_descr)
#define MOB_LONG(mob)            (mob->player.long_descr ? mob->player.long_descr : mob_proto_table[mob->nr].long_descr)
#define MOB_ATTACK_TYPE(mob)     (mob->specials.attack_type)
#define MOB_ATT_NUM(mob)         (mob->specials.no_att)
#define MOB_ATT_TIMER(mob)       (mob->specials.att_timer)
#define MOB_ATT_CHANCE(mob, num) (mob->specials.att_percent[num])
#define MOB_ATT_TYPE(mob, num)   (mob->specials.att_type[num])
#define MOB_ATT_TARGET(mob, num) (mob->specials.att_target[num])
#define MOB_ATT_SPELL(mob, num)  (mob->specials.att_spell[num])

#define OBJ_RNUM(obj)            (obj->item_number)
#define OBJ_VNUM(obj)            (obj->item_number_v)
#define OBJ_NAME(obj)            (obj->name ? obj->name : obj_proto_table[obj->item_number].name)
#define OBJ_GET_NAME(obj)        (obj->name)
#define OBJ_SHORT(obj)           (obj->short_description ? obj->short_description : obj_proto_table[obj->item_number].short_description)
#define OBJ_GET_SHORT(obj)       (obj->short_description)
#define OBJ_DESCRIPTION(obj)     (obj->description ? obj->description : obj_proto_table[obj->item_number].description)
#define OBJ_GET_DESCRIPTION(obj) (obj->description)
#define OBJ_ACTION(obj)          (obj->action_description ? obj->action_description : obj_proto_table[obj->item_number].action_description)
#define OBJ_GET_ACTION(obj)      (obj->action_description)
#define OBJ_ACTION_NT(obj)       (obj->action_description_nt ? obj->action_description_nt : obj_proto_table[obj->item_number].action_description_nt)
#define OBJ_GET_ACTION_NT(obj)   (obj->action_description_nt)
#define OBJ_CWEAR_DESC(obj)      (obj->char_wear_desc ? obj->char_wear_desc : obj_proto_table[obj->item_number].char_wear_desc)
#define OBJ_GET_CWEAR_DESC(obj)  (obj->char_wear_desc)
#define OBJ_RWEAR_DESC(obj)      (obj->room_wear_desc ? obj->room_wear_desc : obj_proto_table[obj->item_number].room_wear_desc)
#define OBJ_GET_RWEAR_DESC(obj)  (obj->room_wear_desc)
#define OBJ_CREM_DESC(obj)       (obj->char_rem_desc ? obj->char_rem_desc : obj_proto_table[obj->item_number].char_rem_desc)
#define OBJ_GET_CREM_DESC(obj)   (obj->char_rem_desc)
#define OBJ_RREM_DESC(obj)       (obj->room_rem_desc ? obj->room_rem_desc : obj_proto_table[obj->item_number].room_rem_desc)
#define OBJ_GET_RREM_DESC(obj)   (obj->room_rem_desc)
#define OBJ_GET_EX_DESC(obj)     (obj->ex_description)
#define OBJ_TYPE(obj)            (obj->obj_flags.type_flag)
#define OBJ_WEAR_FLAGS(obj)      (obj->obj_flags.wear_flags)
#define OBJ_BITS(obj)            (obj->obj_flags.bitvector)
#define OBJ_BITS2(obj)           (obj->obj_flags.bitvector2)
#define OBJ_EXTRA_FLAGS(obj)     (obj->obj_flags.extra_flags)
#define OBJ_EXTRA_FLAGS2(obj)    (obj->obj_flags.extra_flags2)
#define OBJ_SC_RES(obj)          (obj->obj_flags.subclass_res)
#define OBJ_MATERIAL(obj)        (obj->obj_flags.material)
#define OBJ_WEIGHT(obj)          (obj->obj_flags.weight)
#define OBJ_COST(obj)            (obj->obj_flags.cost)
#define OBJ_RENT_COST(obj)       (obj->obj_flags.cost_per_day)
#define OBJ_REPOP(obj)           (obj_proto_table[obj->item_number].obj_flags.repop_percent)
#define OBJ_AFF(obj, num)        (obj->affected[num])
#define OBJ_AFF0(obj)            (obj->affected[0])
#define OBJ_AFF1(obj)            (obj->affected[1])
#define OBJ_AFF2(obj)            (obj->affected[2])
#define OBJ_AFF_LOC(obj, num)    (obj->affected[num].location)
#define OBJ_AFF_MOD(obj, num)    (obj->affected[num].modifier)
#define OBJ_VALUE(obj, num)      (obj->obj_flags.value[num])
#define OBJ_VALUE0(obj)          (obj->obj_flags.value[0])
#define OBJ_VALUE1(obj)          (obj->obj_flags.value[1])
#define OBJ_VALUE2(obj)          (obj->obj_flags.value[2])
#define OBJ_VALUE3(obj)          (obj->obj_flags.value[3])
#define OBJ_POPPED(obj)          (obj->obj_flags.popped)
#define OBJ_SPEC(obj)            (obj->spec_value)
#define OBJ_TIMER(obj)           (obj->obj_flags.timer)
#define OBJ_OWNER_ID(obj, num)   (obj->ownerid[num])
#define OBJ_IN_ROOM(obj)         (obj->in_room)
#define OBJ_IN_ROOM_V(obj)       (obj->in_room_v)
#define OBJ_IN_OBJ(obj)          (obj->in_obj)
#define OBJ_CARRIED_BY(obj)      (obj->carried_by)
#define OBJ_EQUIPPED_BY(obj)     (obj->equipped_by)
#define OBJ_OWNED_BY(obj)        (obj->owned_by)
#define OBJ_FUNC(obj)            (obj->func)
#define OBJ_LOG(obj)             (obj->log)
#define OBJ_CONTAINS(obj)        (obj->contains)
#define OBJ_NEXT_CONTENT(obj)    (obj->next_content)

#define OBJ_PROTO_GET_EX_DESC(obj) (obj_proto_table[OBJ_RNUM(obj)].ex_description)
#define OBJ_PROTO_TIMER(obj)       (obj_proto_table[OBJ_RNUM(obj)].obj_flags.timer)

#define OBJ_NUM_IN_GAME(obj)     (((OBJ_RNUM(obj) > -1) && (OBJ_RNUM(obj) < top_of_objt)) ? obj_proto_table[OBJ_RNUM(obj)].number : 0)

#define ROOM_VNUM(rm)            (world[rm].number)
#define ROOM_ZONE(rm)            (world[rm].zone)
#define ROOM_SPEC(rm)            (world[rm].spec_tmp)
#define ROOM_SECTOR_TYPE(rm)     (world[rm].sector_type)
#define ROOM_NAME(rm)            (world[rm].name)
#define ROOM_DESC(rm)            (world[rm].description)
#define ROOM_GET_EXTRA_DESC(rm)  (world[rm].ex_description)
#define ROOM_EX_DESC(rm)         (world[rm].ex_description->description)
#define ROOM_EX_KEYWORD(rm)      (world[rm].ex_description->keyword)
#define ROOM_FLAGS(rm)           (world[rm].room_flags)
#define ROOM_PEOPLE(rm)          (world[rm].people)
#define ROOM_LIGHT(rm)           (world[rm].light)
#define ROOM_BLOOD(rm)           (world[rm].blood)
#define ROOM_CONTENTS(rm)        (world[rm].contents)
#define ROOM_ARENA(rm)           ((rm != NOWHERE) ? IS_SET(ROOM_FLAGS(rm), ARENA) : FALSE)
#define ROOM_CHAOTIC(rm)         (CHAOSMODE || ((rm != NOWHERE) ? IS_SET(ROOM_FLAGS(rm), CHAOTIC) : FALSE))
#define ROOM_SAFE(rm)            (!CHAOSMODE && ((rm != NOWHERE) ? IS_SET(ROOM_FLAGS(rm), SAFE) : FALSE))

#define CHAR_ROOM_FLAGS(ch)      (ROOM_FLAGS(CHAR_REAL_ROOM(ch)))

#define ITEM(zone, x)            ((zone) + (x))

#define IS_SET(flag, bit)        ((flag) & (bit))
#define SET_BIT(flag, bit)       ((flag) = (flag) | (bit))
#define REMOVE_BIT(flag, bit)    ((flag) = (flag) & ~(bit))
#define TOGGLE_BIT(flag, bit)    (IS_SET(flag, bit) ? REMOVE_BIT(flag, bit) : SET_BIT(flag, bit))

#define IS_AFFECTED(ch, skill)   ((ch) ? IS_SET((ch)->specials.affected_by, (skill)) : FALSE)

#define SWITCH(a, b)             { (a) ^= (b); (b) ^= (a); (a) ^= (b); }

#define GET_REQ(i) \
(i < 2 ?  "Awful"     : \
(i < 4 ?  "Bad"       : \
(i < 7 ?  "Poor"      : \
(i < 10 ? "Average"   : \
(i < 14 ? "Fair"      : \
(i < 20 ? "Good"      : \
(i < 24 ? "Very Good" : \
          "Superb")))))))

#define HSHR(ch)  ((ch)->player.sex ? (((ch)->player.sex == SEX_MALE) ? "his" : "her") : "its")
#define HSSH(ch)  ((ch)->player.sex ? (((ch)->player.sex == SEX_MALE) ? "he"  : "she") : "it")
#define HMHR(ch)  ((ch)->player.sex ? (((ch)->player.sex == SEX_MALE) ? "him" : "her") : "it")

#define ANA(obj)       (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj)      (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define S_ANA(string)  (index("aeiouyAEIOUY", *string) ? "An" : "A")
#define S_SANA(string) (index("aeiouyAEIOUY", *string) ? "an" : "a")

#define GET_ZONE(ch)       (world[CHAR_REAL_ROOM(ch)].zone)

#define GET_OPPONENT(ch)   ((ch) ? (ch)->specials.fighting : NULL)

#define GET_COND(ch, i)    ((ch)->specials.conditions[(i)])

#define GET_POS(ch)        ((ch)->specials.position)

#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)

#define GET_ID(ch)              (ch ? ch->ver3.id : -1)

#define GET_IN_ROOM_R(ch)       (ch->in_room_r)
#define GET_IN_ROOM_V(ch)       (ch->in_room_v)

#define GET_NAME(ch)            (!IS_MOB(ch) ? (ch)->player.name : MOB_NAME(ch))
#define GET_LONG(ch)            (!IS_MOB(ch) ? "None" : MOB_LONG(ch))
#define GET_SHORT(ch)           (!IS_MOB(ch) ? "None" : MOB_SHORT(ch))
#define GET_DISP_NAME(ch)       (!IS_NPC(ch) ? GET_NAME(ch) : GET_SHORT(ch))
#define GET_TITLE(ch)           (ch->player.title)

#define GET_EMAIL(ch)           (ch->ver3.email_addr)

#define GET_CLAN_NUM(ch)        (ch->ver3.clan_num)
#define GET_CLAN(ch)            (clan_list[(int)GET_CLAN_NUM(ch)])
#define GET_CLAN_NAME(ch)       ((GET_CLAN_NUM(ch) && GET_CLAN(ch).name) ? GET_CLAN(ch).name : "None")

#define GET_CLASS(ch)           (ch->player.class)
#define GET_LEVEL(ch)           (ch->player.level)
#define GET_CLASS_NAME(ch)      (!IS_NPC(ch) ? pc_class_types[(int)GET_CLASS(ch)] : npc_class_types[(int)GET_CLASS(ch)])

#define GET_SC(ch)              (ch->ver3.subclass)
#define GET_SC_NAME(ch)         (GET_SC(ch) ? subclass_name[GET_SC(ch) - 1] : "None")
#define GET_SC_LEVEL(ch)        (ch->ver3.subclass_level)

#define GET_RANKING(ch)         (ch->ver3.ranking)

#define GET_SCP(ch)             (ch->ver3.subclass_points)
#define GET_QP(ch)              (ch->ver3.quest_points)

#define GET_EXP(ch)             (ch->points.exp)
#define GET_REMORT_EXP(ch)      (ch->ver3.remort_exp)
#define GET_DEATH_EXP(ch)       (ch->ver3.death_exp)
#define GET_EXP_TO_LEVEL(ch)    ((GET_LEVEL(ch) < LEVEL_MORT) ? exp_table[GET_LEVEL(ch) + 1] - GET_EXP(ch) : 0)

#define GET_SEX(ch)             (ch->player.sex)
#define GET_AGE(ch)             (age(ch).year)
#define GET_HEIGHT(ch)          (ch->player.height)
#define GET_WEIGHT(ch)          (ch->player.weight)

#define GET_STR(ch)             (ch->tmpabilities.str)
#define GET_ADD(ch)             (ch->tmpabilities.str_add)
#define GET_DEX(ch)             (ch->tmpabilities.dex)
#define GET_INT(ch)             (ch->tmpabilities.intel)
#define GET_WIS(ch)             (ch->tmpabilities.wis)
#define GET_CON(ch)             (ch->tmpabilities.con)

#define GET_OSTR(ch)            (ch->abilities.str)
#define GET_OADD(ch)            (ch->abilities.str_add)
#define GET_ODEX(ch)            (ch->abilities.dex)
#define GET_OINT(ch)            (ch->abilities.intel)
#define GET_OWIS(ch)            (ch->abilities.wis)
#define GET_OCON(ch)            (ch->abilities.con)

#define GET_HIT(ch)             (ch->points.hit)
#define GET_NAT_HIT(ch)         (ch->specials.org_hit)
#define GET_MAX_HIT_POINTS(ch)  (ch->points.max_hit)
#define GET_MAX_HIT(ch)         (hit_limit(ch))

#define GET_MANA(ch)            (ch->points.mana)
#define GET_NAT_MANA(ch)        (100 + ch->specials.org_mana)
#define GET_MAX_MANA_POINTS(ch) (ch->points.max_mana)
#define GET_MAX_MANA(ch)        (mana_limit(ch))

#define GET_MOVE(ch)            (ch->points.move)
#define GET_NAT_MOVE(ch)        (100 + ch->specials.org_move)
#define GET_MAX_MOVE_POINTS(ch) (ch->points.max_move)
#define GET_MAX_MOVE(ch)        (move_limit(ch))

#define GET_MANA_REGEN_TMP(ch)  (ch->points.mana_regen_tmp)

#define GET_THACO(ch)           (calc_thaco(ch))

#define GET_HITROLL(ch)         (ch->points.hitroll)
#define GET_DAMROLL(ch)         (ch->points.damroll)

#define GET_AC(ch)              (ch->points.armor)
#define GET_MOD_AC(ch)          (calc_ac(ch))

#define GET_GOLD(ch)            (ch->points.gold)
#define GET_BANK(ch)            (ch->points.bank)

#define GET_HOME(ch)            (ch->player.hometown)

#define GET_BLEED(ch)           (ch->ver3.bleed_limit)
#define GET_WIMPY(ch)           (ch->new.wimpy)

#define GET_DEATH_LIMIT(ch)     (ch->ver3.death_limit)

#define GET_CARRYING(ch)        (ch->carrying)

#define CHAR_NEXT_IN_ROOM(ch)   (ch->next_in_room)

/* Object And Carry related macros */

#define CAN_WEAR(obj, wear_flag) (IS_SET((obj)->obj_flags.wear_flags, wear_flag))

#define IS_OBJ_STAT(obj, stat) (IS_SET((obj)->obj_flags.extra_flags, stat))

#define CAN_GET_OBJ(ch, obj) \
(CAN_TAKE((ch), (obj)) && CAN_CARRY_OBJ((ch), (obj)) && CAN_SEE_OBJ((ch), (obj)))

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? OBJ_SHORT((obj)) : "something")
#define OBJS2(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? rem_prefix(OBJ_SHORT((obj))) : "something")
#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? fname(OBJ_NAME((obj))) : "something")

#define IS_INDOORS(ch) (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, INDOORS) || world[CHAR_REAL_ROOM(ch)].sector_type == SECT_INSIDE)
#define IS_OUTSIDE(ch) (!IS_INDOORS(ch))

#define EXIT(ch, door)  (world[CHAR_REAL_ROOM(ch)].dir_option[door])

#define CAN_GO(ch, door)                          \
(EXIT(ch, door) &&                                \
 EXIT(ch, door)->to_room_r != NOWHERE &&          \
 EXIT(ch, door)->to_room_v != 0 &&                \
 !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) && \
 !IS_SET(EXIT(ch, door)->exit_info, EX_CLIMB) &&  \
 !IS_SET(EXIT(ch, door)->exit_info, EX_JUMP) &&  \
 !IS_SET(EXIT(ch, door)->exit_info, EX_CRAWL) &&  \
 !IS_SET(EXIT(ch, door)->exit_info, EX_ENTER))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define COLORNAME(ch, i) (Color[(ch->colors[i] + 1)])
#define COLOR(ch, i)     if (ch->colors[0] && ch->colors[i]) { send_to_char(Color[(((ch->colors[i]) * 2) - 2)], ch); send_to_char(BKColor[ch->colors[13]], ch); }
#define ENDCOLOR(ch)     if (ch->colors[0] && ch->colors[1]) { send_to_char(Color[(((ch->colors[1]) * 2) - 2)], ch); send_to_char(BKColor[ch->colors[13]], ch); }

#define IS_RENTABLE(obj) ( (obj) ? (((obj)->item_number > -1) &&\
                             (!IS_OBJ_STAT((obj), ITEM_ANTI_RENT)     ) &&\
                             ((obj)->obj_flags.type_flag != ITEM_FOOD ) &&\
                             ((obj)->obj_flags.type_flag != ITEM_SC_TOKEN ) &&\
                             ((obj)->obj_flags.type_flag != ITEM_TRASH) &&\
                             ((obj)->obj_flags.type_flag != ITEM_KEY  )   ) : 0 )

#define strMove(d,s) memmove(d,s,strlen(s)+1)

#define ZONE_NAME_CH(ch) (zone_table[real_zone(ch->specials.zone)].name)
#define ZONE_NUM_CH(ch) (ch->specials.zone)
#define GET_PLAY_TIME(ch) (real_time_passed((time(0) - ch->player.time.logon) + ch->player.time.played, 0))
#define GET_WIZINV(ch) (ch->new.wizinv)
#define GET_PRAC(ch) (ch->specials.spells_to_learn)
#define GET_BEEN_KILLED(ch) (ch->new.been_killed)
#define OBJ_TIMER(obj) (obj->obj_flags.timer)
#define IS_AFFECTED2(ch, aff) (ch ? IS_SET(ch->specials.affected_by2, aff) : FALSE)
#define GET_SCORE_STYLE(ch) (ch->ver3.sc_style)
#define GET_QUEST_STATUS(ch) (ch->quest_status)
#define GET_QUEST_TIMER(ch) (ch->ver3.time_to_quest)
#define GET_IMM_FLAGS(ch) (ch->new.imm_flags)
#define GET_MOUNT(ch) (ch->specials.riding)
#define GET_PFLAG(ch) (ch->specials.pflag)
#define GET_ACT(ch) (ch->specials.act)
#define GET_ACT2(ch) (ch->specials.act2)
#define GET_AFF(ch) (ch->specials.affected_by)
#define GET_AFF2(ch) (ch->specials.affected_by2)
#define OBJ_TYPE(obj) (obj->obj_flags.type_flag)
#define IS_IMMUNE(ch, immunity) (ch ? IS_SET(ch->specials.immune, immunity) : FALSE)
#define IS_IMMUNE2(ch, immunity2) (ch ? IS_SET(ch->specials.immune2, immunity2) : FALSE)
#define IS_RESISTANT(ch, resistance) (ch ? IS_SET(ch->specials.resist, resistance) : FALSE)
#define GET_MASTER(ch) (ch->master)
#define GET_RIDER(ch) (ch->specials.rider)
#define IS_ALIVE(ch) (ch && (CHAR_REAL_ROOM(ch) != NOWHERE) && (GET_POS(ch) > POSITION_DEAD) ? TRUE : FALSE)
#define IS_DEAD(ch) (!IS_ALIVE(ch))
#define SAME_ROOM(ch1, ch2) ((CHAR_REAL_ROOM(ch1) != NOWHERE) && (CHAR_REAL_ROOM(ch1) == CHAR_REAL_ROOM(ch2)))
#define SAME_ZONE(ch1, ch2) (GET_ZONE(ch1) == GET_ZONE(ch2))
#define GET_PROTECTOR(ch) (ch->specials.protect_by)
#define GET_PROTECTEE(ch) (ch->specials.protecting)
#define GET_IMMUNE(ch) (ch->specials.immune)
#define GET_IMMUNE2(ch) (ch->specials.immune2)
#define GET_RESIST(ch) (ch->specials.resist)
#define GET_DESCRIPTOR(ch) (ch->desc)
#define GET_WAS_IN_ROOM(ch) (ch->specials.was_in_room)
#define GET_SKILLS(ch) (ch->skills)
#define GET_OBJ_BITS(obj) (obj->obj_flags.bitvector)
#define GET_OBJ_BITS2(obj) (obj->obj_flags.bitvector2)
#define GET_LAST_DIR(mob) (mob->specials.last_direction)
#define GET_PRESTIGE(ch) (ch->ver3.prestige)
#define GET_PRESTIGE_PERK(ch) ((GET_PRESTIGE(ch) >= 5) ? ((int)(GET_PRESTIGE(ch) + 5) / 10) : 0)
#define GET_WHO_FILTER(ch) (ch->ver3.who_filter)
#define GET_WAIT(ch) (GET_DESCRIPTOR(ch)->wait)
#define GET_DEFAULT_POSITION(ch) (ch->specials.default_pos)
#define GET_DEATH_TIMER(ch) (ch->specials.death_timer)
#define GET_REPLY_TO(ch) (ch->specials.reply_to)
#define GET_TOGGLES(ch) (ch->ver3.toggles)
#define GET_COMM_COLOR(ch, num) (ch->colors[num])
#define GET_COLORS(ch) (ch->colors)
#define GET_FOLLOWERS(ch) (ch->followers)
#define GET_SWITCHED(ch) (ch->switched)
#define GET_TIMER(ch) (ch->specials.timer)

#define GET_QUEST_GIVER(ch) (ch->questgiver)
#define GET_QUEST_OBJ(ch)   (ch->questobj)
#define GET_QUEST_OWNER(ch) (ch->questowner)
#define GET_QUEST_MOB(ch)   (ch->questmob)
#define GET_QUEST_LEVEL(ch) (ch->quest_level)

#define GET_STR_TO_HIT(ch)     (str_app[STRENGTH_APPLY_INDEX(ch)].tohit)
#define GET_STR_TO_DAM(ch)     (str_app[STRENGTH_APPLY_INDEX(ch)].todam)
#define GET_DEX_APP(ch)        (dex_app[GET_DEX(ch)].prac_bonus)
#define GET_DEX_AC(ch)         (dex_app[GET_DEX(ch)].defensive)
#define GET_CON_REGEN(ch)      (con_app[GET_CON(ch)].regen)
#define GET_CON_DAM_REDUCT(ch) (con_app[GET_CON(ch)].reduct)
#define GET_INT_APP(ch)        (int_app[GET_WIS(ch)].learn)
#define GET_INT_CONC(ch)       (int_app[GET_WIS(ch)].conc)
#define GET_WIS_APP(ch)        (wis_app[GET_WIS(ch)].bonus)
#define GET_WIS_CONC(ch)       (wis_app[GET_WIS(ch)].conc)

#define GET_LEARNED(ch, skill) (ch->skills[skill].learned)

#define MOB_NUM_IN_GAME(ch) (ch->nr)
#define MOB_ACT(ch)         (ch->specials.act)

#define IS_NPC(ch)          (ch ? IS_SET(MOB_ACT(ch), ACT_ISNPC) : FALSE)
#define IS_MOB(ch)          (ch ? IS_SET(MOB_ACT(ch), ACT_ISNPC) && (MOB_NUM_IN_GAME(ch) > -1) : FALSE)
#define IS_MOUNT(ch)        (ch ? IS_SET(MOB_ACT(ch), ACT_MOUNT) : FALSE)

#define EQ(ch, pos)        (ch->equipment[pos])
#define IS_WEAPON(obj)     (obj ? ((OBJ_TYPE(obj) == ITEM_WEAPON) || (OBJ_TYPE(obj) == ITEM_2H_WEAPON)) : FALSE)
#define IS_1H_WEAPON(obj)  (obj ? (OBJ_TYPE(obj) == ITEM_WEAPON) : FALSE)
#define IS_2H_WEAPON(obj)  (obj ? (OBJ_TYPE(obj) == ITEM_2H_WEAPON) : FALSE)
#define GET_WEAPON(ch)     ((EQ(ch, WIELD) && IS_WEAPON(EQ(ch, WIELD))) ? EQ(ch, WIELD) : NULL)
#define GET_WEAPON2(ch)    ((EQ(ch, HOLD) && IS_WEAPON(EQ(ch, HOLD))) ? EQ(ch, HOLD) : NULL)

#define IS_PHYSICAL_DAMAGE(damage_type)           ((damage_type >= DAM_PHYSICAL) && (damage_type < DAM_MAGICAL))
#define IS_MAGICAL_DAMAGE(damage_type)            (damage_type >= DAM_MAGICAL)
#define IS_WEAPON_ATTACK(attack_type)             ((attack_type >= TYPE_HIT) && (attack_type <= TYPE_SLICE))
#define IS_SKILL_ATTACK(attack_type, damage_type) (!IS_WEAPON_ATTACK(attack_type) && IS_PHYSICAL_DAMAGE(damage_type))
#define IS_SPELL_ATTACK(attack_type, damage_type) (!IS_WEAPON_ATTACK(attack_type) && IS_MAGICAL_DAMAGE(damage_type))

#define IS_CORPSE(obj)     (obj ? ((OBJ_TYPE(obj) == ITEM_CONTAINER) && (OBJ_VALUE(obj, 3) == 1)) : FALSE)
#define IS_PC_CORPSE(obj)  (obj ? (IS_CORPSE(obj) && ((OBJ_COST(obj) == PC_CORPSE) || (OBJ_COST(obj) == CHAOS_CORPSE))) : FALSE)
#define IS_NPC_CORPSE(obj) (obj ? (IS_CORPSE(obj) && (OBJ_COST(obj) == NPC_CORPSE)) : FALSE)
#define IS_STATUE(obj)     (obj ? (IS_CORPSE(obj) && ((OBJ_COST(obj) == PC_STATUE) || (OBJ_COST(obj) == NPC_STATUE))) : FALSE)
#define IS_PC_STATUE(obj)  (obj ? (IS_CORPSE(obj) && (OBJ_COST(obj) == PC_STATUE)) : FALSE)
#define IS_NPC_STATUE(obj) (obj ? (IS_CORPSE(obj) && (OBJ_COST(obj) == NPC_STATUE)) : FALSE)

#define IS_MORTAL(ch)      (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) < LEVEL_IMM)) : FALSE)
#define IS_IMMORTAL(ch)    (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_IMM)) : FALSE)
#define IS_DEITY(ch)       (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_DEI)) : FALSE)
#define IS_TEMPORAL(ch)    (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_TEM)) : FALSE)
#define IS_WIZARD(ch)      (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_WIZ)) : FALSE)
#define IS_ETERNAL(ch)     (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_ETE)) : FALSE)
#define IS_SUPREME(ch)     (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_SUP)) : FALSE)
#define IS_IMPLEMENTOR(ch) (ch ? (!IS_NPC(ch) && (GET_LEVEL(ch) >= LEVEL_IMP)) : FALSE)

#define WIZ_INV(ch, vict)        (GET_WIZINV(vict) && ((!IS_NPC(vict) && (GET_WIZINV(vict) > GET_LEVEL(ch))) || (IS_NPC(ch) && (GET_WIZINV(vict) > LEVEL_IMM))))
#define IMP_INV(ch, vict)        (IS_AFFECTED(vict, AFF_IMINV) && (vict != ch) && (GET_LEVEL(ch) < LEVEL_IMM) && !((GET_LEVEL(vict) <= GET_LEVEL(ch)) && IS_AFFECTED2(ch, AFF2_PERCEIVE)) && !CHAOSMODE)
#define NRM_INV(ch, vict)        (IS_AFFECTED(vict, AFF_INVISIBLE) && (vict != ch) && (GET_LEVEL(ch) < LEVEL_IMM) && !IS_AFFECTED(ch, AFF_DETECT_INVISIBLE) && !((GET_LEVEL(vict) <= GET_LEVEL(ch)) && IS_AFFECTED2(ch, AFF2_PERCEIVE)))
#define IS_HIDING_FROM(ch, vict) (IS_AFFECTED(vict, AFF_HIDE) && (vict != ch) && (GET_LEVEL(ch) < LEVEL_IMM) && !IS_AFFECTED2(ch, AFF2_PERCEIVE))

#define CAN_SEE_OBJ(ch, obj) (\
  IS_IMMORTAL(ch) || \
  IS_AFFECTED((ch), AFF_INFRAVISION) || \
  ((!IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_INVISIBLE) || IS_AFFECTED((ch), AFF_DETECT_INVISIBLE)) && !IS_AFFECTED((ch), AFF_BLIND) && IS_LIGHT(CHAR_REAL_ROOM(ch))) \
)

#define CAN_CARRY_W(ch) (\
  IS_IMMORTAL(ch) ? 10000 : \
  (GET_PRESTIGE_PERK(ch) >= 20) ? (int)(str_app[STRENGTH_APPLY_INDEX(ch)].carry_w * 1.1) : str_app[STRENGTH_APPLY_INDEX(ch)].carry_w) // Prestige Perk 20

#define CAN_CARRY_N(ch) (\
  IS_IMMORTAL(ch) ? 200 : \
  ((((5 + GET_DEX(ch)) / 2) + (GET_LEVEL(ch) / 2)) < 20) ? 20 : \
  (GET_PRESTIGE_PERK(ch) >= 14) ? (int)((((5 + GET_DEX(ch)) / 2) + (GET_LEVEL(ch) / 2)) * 1.1) : (((5 + GET_DEX(ch)) / 2) + (GET_LEVEL(ch) / 2))) // Prestige Perk 14

#define CAN_CARRY_OBJ(ch, obj) ( \
  IS_IMMORTAL(ch) || \
  ((((IS_CARRYING_W(ch) + GETOBJ_WEIGHT(obj)) <= (3 * CAN_CARRY_W(ch))) && ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))) \
)

#endif /* __UTILS_H__ */
