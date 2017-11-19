/**************************************************************************
*  File: fight.c , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "act.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "cmd.h"
#include "fight.h"
#include "utility.h"
#include "subclass.h"
#include "memory.h"
#include "limits.h"
#include "enchant.h"

/* Structures */

CHAR *combat_list = 0;      /* head of l-list of fighting chars */
CHAR *combat_next_dude = 0; /* Next dude global trick           */

extern char *Color[];
extern char *spells[];
extern char *BKColor[];

/* External structures */
struct scoreboard_data
{
  char killer[80];
  char killed[80];
  char location[80];
  char time_txt[80];
};

extern struct scoreboard_data scores[101];
extern int number_of_kills;

extern struct time_info_data time_info;
extern struct obj_proto *obj_proto_table;

extern struct message_list fight_messages[MAX_MESSAGES];

extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct obj_data  *object_list;

extern int thaco[11][58];
extern byte backstab_mult[58];
extern byte circle_mult[58];
extern byte ambush_mult[58];
extern byte assault_mult[58];

extern struct str_app_type str_app[];
extern struct dex_app_type dex_app[];
extern struct con_app_type con_app[];

extern int CHAOSMODE;
extern int CHAOSDEATH;
extern int DOUBLEXP;

/* External procedures */

char *fread_string(FILE *f1);
void page_string(struct descriptor_data *d, char *str, int keep_internal);

void stop_follower(CHAR *ch);
void do_flee(CHAR *ch, char *argument, int cmd);
void stop_riding(struct char_data *ch,struct char_data *vict);
void die(CHAR *ch);
void brag(struct char_data *ch, struct char_data *victim);
int hit_limit(CHAR * ch);

int trophy_bonus(CHAR *ch); /* Used to be scalp_bonus */
int close_combat_bonus(CHAR *ch,int type);
void hit(CHAR *ch, CHAR *victim, int type);
void dhit(CHAR *ch, CHAR *victim, int type);
void thit(CHAR *ch, CHAR *victim, int type);
void qhit(CHAR *ch, CHAR *victim, int type);

int calc_hit_damage(CHAR *ch, CHAR *victim, OBJ *weapon);

int impair_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg);

/* Weapon attack texts. */
struct attack_hit_type attack_hit_text[] =
{
  {"hit", "hits"},       /* TYPE_HIT      */
  {"pound", "pounds"},   /* TYPE_BLUDGEON */
  {"pierce", "pierces"}, /* TYPE_PIERCE   */
  {"slash", "slashes"},  /* TYPE_SLASH    */
  {"whip", "whips"},     /* TYPE_WHIP     */
  {"claw", "claws"},     /* TYPE_CLAW     */
  {"bite", "bites"},     /* TYPE_BITE     */
  {"sting", "stings"},   /* TYPE_STING    */
  {"crush", "crushes"},  /* TYPE_CRUSH    */
  {"hack", "hacks"},     /* TYPE_HACK     */
  {"chop", "chops"},     /* TYPE_CHOP     */
  {"slice", "slices"}    /* TYPE_SLICE    */
};

/* The Fight related routines */


void appear(CHAR *ch)
{
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);

  if (affected_by_spell(ch, SPELL_INVISIBLE))
  {
    affect_from_char(ch, SPELL_INVISIBLE);
  }

  REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}

void load_messages(void)
{
  FILE *f1;
  int i, type;
  struct message_type *messages;
  char chk[100];

  if (!(f1 = fopen(MESS_FILE, "r"))) {
    log_f("read messages");
    produce_core();
  }

  for (i = 0; i < MAX_MESSAGES; i++) {
     fight_messages[i].a_type = 0;
     fight_messages[i].number_of_attacks = 0;
     fight_messages[i].msg = 0;
  }

  fscanf(f1, " %s \n", chk);

  while (*chk == 'M') {
    fscanf(f1," %d\n", &type);
    
    for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) && (fight_messages[i].a_type); i++);

    if (i >= MAX_MESSAGES) {
      log_f("Too many combat messages.");
      produce_core();
    }

    CREATE(messages, struct message_type, 1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type = type;
    messages->next = fight_messages[i].msg;
    fight_messages[i].msg  =messages;

    messages->die_msg.attacker_msg  = fread_string(f1);
    messages->die_msg.victim_msg    = fread_string(f1);
    messages->die_msg.room_msg      = fread_string(f1);
    messages->miss_msg.attacker_msg = fread_string(f1);
    messages->miss_msg.victim_msg   = fread_string(f1);
    messages->miss_msg.room_msg     = fread_string(f1);
    messages->hit_msg.attacker_msg  = fread_string(f1);
    messages->hit_msg.victim_msg    = fread_string(f1);
    messages->hit_msg.room_msg      = fread_string(f1);
    messages->god_msg.attacker_msg  = fread_string(f1);
    messages->god_msg.victim_msg    = fread_string(f1);
    messages->god_msg.room_msg      = fread_string(f1);

    fscanf(f1, " %s \n", chk);
  }

  fclose(f1);
}


void update_pos(CHAR *victim) {
  if (GET_MOUNT(victim) &&
      CHAR_REAL_ROOM(GET_MOUNT(victim)) != CHAR_REAL_ROOM(victim)) {
    stop_riding(victim, GET_MOUNT(victim));
  }

  if (GET_RIDER(victim) &&
      CHAR_REAL_ROOM(GET_RIDER(victim)) != CHAR_REAL_ROOM(victim)) {
    stop_riding(GET_RIDER(victim), victim);
  }

  if (GET_POS(victim) == POSITION_FIGHTING &&
      !GET_OPPONENT(victim)) {
    GET_POS(victim) = POSITION_STANDING;
  }

  if (GET_POS(victim) == POSITION_STANDING &&
      IS_AFFECTED(victim, AFF_FLY)) {
    GET_POS(victim) = POSITION_FLYING;
  }

  if (GET_POS(victim) == POSITION_FLYING &&
      !IS_AFFECTED(victim, AFF_FLY)) {
    GET_POS(victim) = POSITION_STANDING;
  }

  if (GET_HIT(victim) > 0 && GET_POS(victim) > POSITION_STUNNED) return;
  else if (GET_HIT(victim) > 0) GET_POS(victim) = POSITION_STANDING;
  else if (GET_HIT(victim) <= -11) GET_POS(victim) = POSITION_DEAD;
  else if (GET_HIT(victim) <= -6) GET_POS(victim) = POSITION_MORTALLYW;
  else if (GET_HIT(victim) <= -3) GET_POS(victim) = POSITION_INCAP;
  else GET_POS(victim) = POSITION_STUNNED;
}


/* Start one char fighting another (yes, it is horrible, I know... ) */
void set_fighting(CHAR *ch, CHAR *vict) {
  char buf[MSL];

  if (!ch) return;

  if (GET_OPPONENT(ch) == vict) return;

  if (ch->specials.fighting) {
    ch->specials.fighting->specials.num_fighting--;
    ch->specials.num_fighting--;
  }

  /* Included to prevent double counting when the mob is the one starting the fight. */
  if (IS_NPC(ch) && ch->specials.num_fighting) {
    ch->specials.num_fighting--;
  }

  ch->next_fighting = combat_list;
  combat_list = ch;

  if (IS_AFFECTED(ch, AFF_SLEEP)) {
    affect_from_char(ch, SPELL_SLEEP);
  }

  ch->specials.fighting = vict;
  ch->specials.num_fighting++;
  ch->specials.max_num_fighting = MAX(ch->specials.max_num_fighting, ch->specials.num_fighting);

  if (GET_OPPONENT(vict) != ch) {
    vict->specials.num_fighting++;
    vict->specials.max_num_fighting = MAX(vict->specials.max_num_fighting, vict->specials.num_fighting);
  }

  GET_POS(ch) = POSITION_FIGHTING;

  if (CHAOSMODE && IS_MORTAL(ch) && IS_MORTAL(vict)) {
    sprintf(buf, "CHAOS: %s started a fight with %s!", GET_NAME(ch), GET_NAME(vict));
    wizlog(buf, LEVEL_IMM, 3);
  }
}


/* remove a char from the list of fighting chars */
void stop_fighting(CHAR *ch) {
  CHAR *tmp = NULL;
  bool bFound = FALSE;

  if (!ch) return;
  if (!ch->specials.fighting) return;

  /* check that ch is not fighting self */
  if (ch == ch->next_fighting) {
    log_f("Char next_fighting refers to self, aborting (fight.c, stop_fighting)");
    abort();
  }

  /* update current list pointer */
  if (ch == combat_next_dude) {
    combat_next_dude = ch->next_fighting;
  }

  /* remove from the head of the list, if eq to ch */
  if (ch == combat_list) {
    combat_list = ch->next_fighting;
    bFound = TRUE;
  }

  /* remove all instances of ch from combat list */
  for (tmp = combat_list; tmp; tmp = tmp->next_fighting) {
    if (tmp->next_fighting == ch) {
      tmp->next_fighting = ch->next_fighting;
      bFound = TRUE;
    }
  }

  if (!bFound) {
    log_f("Char fighting not found, aborting (fight.c, stop_fighting)");
    abort();
  }

  ch->specials.fighting->specials.num_fighting--;
  ch->specials.fighting->specials.num_fighting = MAX(0,ch->specials.fighting->specials.num_fighting);

  ch->specials.num_fighting--;
  ch->specials.num_fighting = MAX(0, ch->specials.num_fighting);

  ch->next_fighting = 0;
  ch->specials.fighting = 0;

  if (ch->specials.riding && (CHAR_REAL_ROOM(ch) == CHAR_REAL_ROOM(ch->specials.riding))) {
    GET_POS(ch) = POSITION_RIDING;
  }
  else {
    GET_POS(ch) = POSITION_STANDING;
  }

  /* make sure player doesn't automatically fall in a must-fly area */
  if (IS_NPC(ch) && IS_SET(ch->specials.act, ACT_FLY)) {
    GET_POS(ch) = POSITION_FLYING;
  }

  update_pos(ch);
}

void death_list(CHAR *ch);
void make_corpse(CHAR *ch)
{
  struct obj_data *corpse, *o;
  struct obj_data *money;
  char buf[MAX_STRING_LENGTH];
  int i;

  char *str_dup(char *source);
  struct obj_data *create_money( int amount );

  ch->new.been_killed += 1;
  death_list(ch);
  CREATE(corpse, struct obj_data, 1);
  clear_object(corpse);

  corpse->item_number = NOWHERE;
  corpse->in_room = NOWHERE;
  if(!IS_NPC(ch)) {
    sprintf(buf,"corpse %s",GET_NAME(ch));
    string_to_lower(buf);
    corpse->name = str_dup(buf);
  }
  else
    corpse->name = str_dup("corpse");
  sprintf(buf,"corpse of %s is lying here.",(IS_NPC(ch) ? MOB_SHORT(ch) : GET_NAME(ch)));
  corpse->description = str_dup(buf);

  sprintf(buf, "Corpse of %s",
    (IS_NPC(ch) ? MOB_SHORT(ch) : GET_NAME(ch)));
  corpse->short_description = str_dup(buf);

  corpse->contains = ch->carrying;
  if (GET_GOLD(ch)>0) {
    money = create_money(GET_GOLD(ch));
    GET_GOLD(ch)=0;
    obj_to_obj(money,corpse);
  }

  corpse->obj_flags.type_flag = ITEM_CONTAINER;
  corpse->obj_flags.wear_flags = ITEM_TAKE;
  corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
  corpse->obj_flags.value[2] = GET_LEVEL(ch);
  corpse->obj_flags.value[3] = 1; /* corpse identifyer */
  corpse->obj_flags.weight = GET_WEIGHT(ch);
  corpse->obj_flags.cost_per_day = 1;
  if (IS_NPC(ch)) {
    corpse->obj_flags.cost = NPC_CORPSE;
    corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
    corpse->obj_flags.material = mob_proto_table[ch->nr].class;
    corpse->obj_flags.cost_per_day=mob_proto_table[ch->nr].skin_value;
    corpse->obj_flags.skin_vnum[0]=mob_proto_table[ch->nr].skin_vnum[0];
    corpse->obj_flags.skin_vnum[1]=mob_proto_table[ch->nr].skin_vnum[1];
    corpse->obj_flags.skin_vnum[2]=mob_proto_table[ch->nr].skin_vnum[2];
    corpse->obj_flags.skin_vnum[3]=mob_proto_table[ch->nr].skin_vnum[3];
    corpse->obj_flags.skin_vnum[4]=mob_proto_table[ch->nr].skin_vnum[4];
    corpse->obj_flags.skin_vnum[5]=mob_proto_table[ch->nr].skin_vnum[5];
    /* some skins should be worthless based on mob class - those checks
    added to do_skin */
  }
  else {
    corpse->obj_flags.cost = PC_CORPSE;
    corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;
  }

  for (i=0; i<MAX_WEAR; i++)
   if (ch->equipment[i])
     obj_to_obj(unequip_char(ch, i), corpse);

  ch->carrying = 0;

  corpse->next = object_list;
  object_list = corpse;

  for(o = corpse->contains; o; o->in_obj = corpse, o = o->next_content);
   object_list_new_owner(corpse, 0);

  if(GET_LEVEL(ch)<10 && corpse->obj_flags.cost==PC_CORPSE &&
     !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,DEATH) &&
     !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,HAZARD) ) {
    send_to_char("\n\rYour corpse is in the Midgaard Morgue 2 west from the Temple.\n\r\
`iIMPT: When you reach level 10, your corpse will be where-ever you died.`q\n\r",ch);
    obj_to_room(corpse,real_room(3088));
  }
  else {
    obj_to_room(corpse, CHAR_REAL_ROOM(ch));
  }
  ch->points.hit  = MIN(ch->points.max_hit,1);
  ch->points.mana = MIN(ch->points.max_mana,1);
  remove_all_affects(ch);
}

/* New check for Chaotic items for deaths in Chaotic Rooms
   Ranger - Jan 97 */

void check_chaotic(OBJ *obj ,OBJ *corpse) {
  int rnum,pos;
  struct obj_data *tmp,*next_obj;
  if (!obj) return;

  rnum = obj->item_number;
  if( (rnum>0) && IS_OBJ_STAT(obj, ITEM_CHAOTIC) ) {
    if(obj->in_obj) {
      obj_from_obj(obj);
      obj_to_obj(obj,corpse);
    }
    if(obj->carried_by) {
      obj_from_char(obj);
      obj_to_obj(obj,corpse);
    }
    if(obj->equipped_by) {
     for(pos=0; pos<MAX_WEAR; pos++) {
      if(obj==EQ(obj->equipped_by,pos)) {
        obj_to_obj(unequip_char(obj->equipped_by, pos), corpse);
        break;
      }
     }
    }
  }
  for (tmp = obj->contains;tmp;tmp=next_obj) {
    next_obj=tmp->next_content;
    check_chaotic(tmp,corpse);
  }
}

void make_chaos_corpse(CHAR *ch)
{
  struct obj_data *corpse, *obj,*next_obj;
  char buf[MAX_STRING_LENGTH];
  int i;

  char *str_dup(char *source);

  CREATE(corpse, struct obj_data, 1);
  clear_object(corpse);

  corpse->item_number = NOWHERE;
  corpse->in_room = NOWHERE;
  corpse->name = str_dup("corpse");

  sprintf(buf, "Corpse of %s is lying here.",
    (IS_NPC(ch) ? MOB_SHORT(ch) : GET_NAME(ch)));
  corpse->description = str_dup(buf);

  sprintf(buf, "Corpse of %s",
    (IS_NPC(ch) ? MOB_SHORT(ch) : GET_NAME(ch)));
  corpse->short_description = str_dup(buf);

  corpse->obj_flags.type_flag = ITEM_CONTAINER;
  corpse->obj_flags.wear_flags = ITEM_TAKE;
  corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
  corpse->obj_flags.value[2] = GET_LEVEL(ch);
  corpse->obj_flags.value[3] = 1; /* corpse identifier */
  corpse->obj_flags.weight = GET_WEIGHT(ch);
  corpse->obj_flags.cost_per_day = 100000;
  corpse->obj_flags.cost = CHAOS_CORPSE;
  corpse->obj_flags.timer = MAX_CHAOS_CORPSE_TIME;

  /* Check for Chaotic items in inventory and equipped */

  obj =NULL;
  for(i=0; i<MAX_WEAR; i++) {
    obj=ch->equipment[i];
    if (obj) check_chaotic(obj,corpse);
  }

  obj =NULL;
  for(obj = ch->carrying; obj; obj = next_obj) {
    next_obj=obj->next_content;
    if (obj) check_chaotic(obj,corpse);
  }

  corpse->next = object_list;
  object_list = corpse;

  obj_to_room(corpse, CHAR_REAL_ROOM(ch));
  remove_all_affects(ch);
}

/* When ch kills victim */

void change_alignment(CHAR *ch, CHAR *victim)
{
  int align;

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC)) return;
  align = (-1*GET_ALIGNMENT(victim)/10);
  GET_ALIGNMENT(ch) += align/2;
  GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch), -1000);
  GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch), 1000);

}

void death_cry(CHAR *ch)
{
  int door = 0;

  /* Ranger - June 96 */
  if (CHAR_REAL_ROOM(ch) == NOWHERE) return;

  act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);

  for (door = 0; door <= 5; door++)
  {
    if (CAN_GO(ch, door))
    {
      send_to_room("Your blood freezes as you hear someone's death cry.\n\r",
        world[CHAR_REAL_ROOM(ch)].dir_option[door]->to_room_r);
    }
  }
}

/*
** deathlist procs  - For listing a players stuff at death
**
** Written for RoninMUD by Ranger
**
** Date: June 16, 1996
**
** Using this code is not allowed without permission from originator.
**
** Last Modification: May 13, 1997
**   Added obj stats for reimb command
**   Changed whole thing into a binary file write - Ranger
*/

void death_flag_write(sbyte option, FILE *fl)
{
  struct death_file_check dcheck;

  memset(&dcheck, 0, sizeof(dcheck));
  dcheck.flag = option;
  fwrite(&dcheck, sizeof(dcheck), 1, fl);
}

void obj_to_dlist(struct obj_data *obj, FILE *fl)
{
  int j;
  struct obj_data *tmp;
  struct obj_file_elem_ver3 object;
  memset(&object,0,sizeof(object));

  if (!obj) return;
  if(!IS_OBJ_STAT(obj,ITEM_CLONE) && obj->item_number>0) {
    object.position   = -1;
    object.item_number= obj_proto_table[obj->item_number].virtual;
    object.value[0]   = obj->obj_flags.value[0];
    object.value[1]   = obj->obj_flags.value[1];
    object.value[2]   = obj->obj_flags.value[2];
    object.value[3]   = obj->obj_flags.value[3];
    if (obj->obj_flags.type_flag == ITEM_CONTAINER) object.value[3]=0;
    object.extra_flags= obj->obj_flags.extra_flags;
    object.weight     = obj->obj_flags.weight;
    object.timer      = obj->obj_flags.timer;
    object.bitvector  = obj->obj_flags.bitvector;
/* new obj saves */
    object.version=32003;
    object.type_flag=obj->obj_flags.type_flag;
    object.wear_flags=obj->obj_flags.wear_flags;
    object.extra_flags2=obj->obj_flags.extra_flags2;
    object.subclass_res=obj->obj_flags.subclass_res;
    object.material=obj->obj_flags.material;
    object.spec_value =OBJ_SPEC(obj);
    for(j=0;j<MAX_OBJ_AFFECT;j++)
      object.affected[j]=obj->affected[j];
/* end new obj saves */

/* new obj saves for obj ver3 */
      object.bitvector2  =obj->obj_flags.bitvector2;
/* end new ver3 obj saves */

/* New owner id */
    object.ownerid[0] =obj->ownerid[0];
    object.ownerid[1] =obj->ownerid[1];
    object.ownerid[2] =obj->ownerid[2];
    object.ownerid[3] =obj->ownerid[3];
    object.ownerid[4] =obj->ownerid[4];
    object.ownerid[5] =obj->ownerid[5];
    object.ownerid[6] =obj->ownerid[6];
    object.ownerid[7] =obj->ownerid[7];

    death_flag_write(5,fl); /* was 2 for object version 0 */
    fwrite(&object, sizeof(object),1,fl);
  }

  if(obj->contains) {
    for (tmp = obj->contains;tmp;tmp = tmp->next_content)
       obj_to_dlist(tmp, fl);
  }
}

void char_to_dlist(CHAR *ch, struct death_file_u *st)
{
  strcpy(st->name, GET_NAME(ch));
  st->number      = ch->new.been_killed;
  st->time_death  = time(0);
  st->location    = ch->in_room_v;
  st->level       = GET_LEVEL(ch);
  st->gold        = GET_GOLD(ch);
  st->exp         = GET_EXP(ch) * 2;
  st->hp          = ch->specials.org_hit;
  st->mana        = ch->specials.org_mana + 100;
  st->move        = ch->specials.org_move + 100;
  st->str         = ch->abilities.str;
  st->add         = ch->abilities.str_add;
  st->intel       = ch->abilities.intel;
  st->wis         = ch->abilities.wis;
  st->dex         = ch->abilities.dex;
  st->con         = ch->abilities.con;
}

void death_list(CHAR *ch)
{
  struct death_file_u dfile;
  char name[20];
  char buf[MSL];
  FILE *fl = NULL;
  int i = 0;
  OBJ *obj = NULL;

  if (!IS_MORTAL(ch)) return;

  memset(&dfile, 0, sizeof(dfile));

  sprintf(name, "%s", GET_NAME(ch));
  sprintf(buf, "rent/death/%s.lst", string_to_lower(name));

  if (!(fl = fopen(buf, "ab"))) return;

  death_flag_write(1, fl);
  char_to_dlist(ch, &dfile);
  fwrite(&dfile, sizeof(dfile), 1, fl);

  for (i = 0; i < MAX_WEAR; i++)
  {
    obj = ch->equipment[i];

    if (obj)
    {
      obj_to_dlist(obj, fl);
    }
  }

  for (obj = ch->carrying; obj; obj = obj->next_content)
  {
    if (obj)
    {
      obj_to_dlist(obj, fl);
    }
  }

  death_flag_write(9, fl);
  fclose(fl);
}

void raw_kill(CHAR *ch)
{
  if (ch->specials.fighting)
  {
    stop_fighting(ch);
  }

  death_cry(ch);
  make_corpse(ch);

  if (ch->quest_status == QUEST_RUNNING || ch->quest_status == QUEST_COMPLETED)
  {
    ch->ver3.time_to_quest = 30;
  }

  ch->questgiver = 0;

  if (ch->questobj)
  {
    ch->questobj->owned_by = 0;
  }
  ch->questobj = 0;

  if (ch->questmob)
  {
    ch->questmob->questowner = 0;
  }
  ch->questmob = 0;

  ch->quest_status = QUEST_NONE;
  ch->quest_level = 0;

  save_char(ch, NOWHERE);
  extract_char(ch);
}

CHAR *get_mount(CHAR *ch)
{
  CHAR *mount = NULL;
  struct follow_type *follower_list;

  for (follower_list = ch->followers; follower_list; follower_list = follower_list->next)
  {
    if (IS_NPC(follower_list->follower) &&
      follower_list->follower->master == ch &&
      CHAR_REAL_ROOM(follower_list->follower) == CHAR_REAL_ROOM(ch) &&
      IS_SET(follower_list->follower->specials.act, ACT_MOUNT))
    {
      mount = follower_list->follower;
      break;
    }
  }

  return mount;
}

/* Rework of xp level "Ranges" - Ranger March 99 and again Aug 99 */
/*divide_experience() works for ANY kill type dividing of experience,
  whether 1 person kills  the victim, an NPC in a group kills the victim
  an NPC kills a PC, PC in a group kill a PC.  ANYTHING!
*/
extern int rv2_gain_remort_exp(CHAR *ch, int exp);
extern int gain_death_exp(CHAR *ch, int exp);
void divide_experience(CHAR *ch, CHAR *victim, int none)
{
  CHAR *leader_of_killer = NULL;
  CHAR *mount = NULL;
  CHAR *tmp_char = NULL;
  FOL *followers = NULL;
  int total_exp = 0;
  int allow_diff = 0;
  int level_diff = 0;
  int cummulative_levels = 0;
  int experience = 0;
  int remort_exp = 0;
  int death_exp = 0;
  int total_pcs = 0;

  total_exp = GET_EXP(victim) / 3;

  /* Sometime in the future this will be set in mob stats - Aug 99 */
  if (victim->specials.max_num_fighting == 0)
  {
    allow_diff = 0;
  }
  else
  {
    allow_diff = MAX(((15 / victim->specials.max_num_fighting) - 1), 4);
  }

  /* If the killer (ch) is not following anyone, or the char/mob
     he is following is not grouped, The leader of the group is
     the killer, otherwise, it is the person he is following */
  if ((!IS_AFFECTED(ch, AFF_GROUP) || ch->master == NULL))
  {
    leader_of_killer = ch;
  }
  else
  {
    if (!IS_AFFECTED(ch->master, AFF_GROUP))
    {
      leader_of_killer = ch;
    }
    else
    {
      leader_of_killer = ch->master;
    }
  }

  if (IS_NPC(ch) &&
      IS_SET(ch->specials.act, ACT_MOUNT) &&
      ch->master != NULL)
  {
    leader_of_killer = ch->master;

    if (IS_AFFECTED(ch->master, AFF_GROUP) && (ch->master->master != NULL))
    {
      leader_of_killer = ch->master->master;
    }
  }

  if ((!IS_NPC(leader_of_killer) || !IS_NPC(ch)) &&
      !IS_NPC(victim))
  {
    total_exp = 1;
  }

  if (CHAR_REAL_ROOM(leader_of_killer) == CHAR_REAL_ROOM(ch))
  {
    cummulative_levels = GET_LEVEL(leader_of_killer);

    if (!IS_NPC(leader_of_killer)) total_pcs++;

    if ((mount = get_mount(leader_of_killer)))
    {
      cummulative_levels += GET_LEVEL(mount);

      if (GET_LEVEL(leader_of_killer) < 16)
      {
        cummulative_levels -= GET_LEVEL(mount);
      }
      else if (GET_LEVEL(leader_of_killer) < 26)
      {
        cummulative_levels -= (GET_LEVEL(mount) / 2);
      }
    }
  }
  else
  {
    cummulative_levels = 0;
  }

  /* Go through the list of followers, counting up their levels */
  for (followers = leader_of_killer->followers; followers; followers = followers->next)
  {
    if (IS_AFFECTED(followers->follower, AFF_GROUP) &&
        CHAR_REAL_ROOM(followers->follower) == CHAR_REAL_ROOM(ch))
    {
      cummulative_levels += GET_LEVEL(followers->follower);

      if (!IS_NPC(followers->follower)) total_pcs++;

      if ((mount = get_mount(followers->follower)))
      {
        cummulative_levels += GET_LEVEL(mount);

        if (GET_LEVEL(followers->follower) < 16)
        {
          cummulative_levels -= GET_LEVEL(mount);
        }
        else if (GET_LEVEL(followers->follower) < 26)
        {
          cummulative_levels -= (GET_LEVEL(mount) / 2);
        }
      }
    }
  }

  if (cummulative_levels <= 0)
  {
    cummulative_levels = MAX(GET_LEVEL(ch), 1);
  }

  /* Add bonus to xp for groups containing 4 or more players */

  if (4 <= total_pcs)
  {
    /* Group XP Bonus:
    **
    **   4 or more = 10%
    **   7 or more = 20%
    **   10 or more = 30%
    **   every 3 adds an extra 10% up to a max of 100%
    */

    total_exp = ((total_exp * (10 + MIN(10,(((total_pcs-4)/3)+1)))) / 10);
  }

  /* Distribute the exp, based on their level */
  /* First to the leader */
  if (CHAR_REAL_ROOM(leader_of_killer) == CHAR_REAL_ROOM(ch))
  {
    experience = ((total_exp / cummulative_levels) * GET_LEVEL(leader_of_killer));
    level_diff = MAX((GET_LEVEL(leader_of_killer) - GET_LEVEL(victim)), 0);

    if (level_diff > allow_diff)
    {
      experience = MAX(((experience * (6 - level_diff + allow_diff)) / 6), (experience / 10));
    }

    if (leader_of_killer != victim)
    {
      if (none)
      {
        experience = 0;
      }

      printf_to_char(leader_of_killer, "You gained %d experience points for the kill.\n\r", experience);

      gain_exp(leader_of_killer, experience);
      change_alignment(leader_of_killer, victim);

      if (DOUBLEXP)
      {
        gain_exp(leader_of_killer, experience);
        printf_to_char(leader_of_killer, "You gained %d extra special bonus xp!\n\r", experience);
      }

      if (GET_REMORT_EXP(leader_of_killer))
      {
        remort_exp = rv2_gain_remort_exp(leader_of_killer, experience);

        printf_to_char(leader_of_killer, "You gained %d remort experience for the kill.\n\r", remort_exp);
      }

      if (GET_DEATH_EXP(leader_of_killer))
      {
        death_exp = gain_death_exp(leader_of_killer, experience);

        printf_to_char(leader_of_killer, "You gained %d death experience for the kill.\n\r", death_exp);
      }

      if (leader_of_killer->questmob == victim)
      {
        send_to_char("Quest completed! Return to the quest giver for credit.\n\r", leader_of_killer);

        leader_of_killer->quest_status = QUEST_COMPLETED;
        leader_of_killer->questmob = NULL;
        victim->questowner = NULL;
      }
    }

    save_char(leader_of_killer, NOWHERE);

    if ((mount = get_mount(leader_of_killer)))
    {
      if (GET_LEVEL(leader_of_killer) < 16)
      {
        experience = 0;
      }
      else
      if (GET_LEVEL(leader_of_killer) < 26)
      {
        experience = ((((total_exp / 2) / cummulative_levels) / 3) * GET_LEVEL(mount));
      }
      else
      {
        experience = (((total_exp / cummulative_levels) / 3) * GET_LEVEL(mount));
      }

      if (none)
      {
        experience = 0;
      }

      printf_to_char(mount, "You gained %d experience for the kill.\n\r", experience);

      gain_exp(mount, experience);
    }
  }

  /*Then to his followers, in the room */
  for (followers = leader_of_killer->followers; followers; followers = followers->next)
  {
    if (!(tmp_char = followers->follower)) continue;

    if (IS_AFFECTED(tmp_char, AFF_GROUP) &&
        CHAR_REAL_ROOM(tmp_char) == CHAR_REAL_ROOM(ch))
    {
      experience = ((total_exp / cummulative_levels) * GET_LEVEL(tmp_char));
      level_diff = MAX((GET_LEVEL(tmp_char) - GET_LEVEL(victim)), 0);

      if (level_diff > allow_diff)
      {
        experience = MAX(((experience * (6 - level_diff + allow_diff)) / 6), (experience / 10));
      }

      if (tmp_char != victim)
      {
        if (none)
        {
          experience = 0;
        }

        printf_to_char(tmp_char, "You gained %d experience for the kill.\n\r", experience);

        gain_exp(tmp_char, experience);
        change_alignment(tmp_char, victim);

        if (DOUBLEXP)
        {
          gain_exp(tmp_char, experience);
          printf_to_char(tmp_char, "You gained %d extra special bonus xp!\n\r", experience);
        }

        if (GET_REMORT_EXP(tmp_char))
        {
          remort_exp = rv2_gain_remort_exp(tmp_char, experience);

          printf_to_char(tmp_char, "You gained %d remort experience for the kill.\n\r", remort_exp);
        }

        if (GET_DEATH_EXP(tmp_char))
        {
          death_exp = gain_death_exp(tmp_char, experience);

          printf_to_char(tmp_char, "You gained %d death experience for the kill.\n\r", death_exp);
        }

        if (tmp_char->questmob == victim)
        {
          send_to_char("Quest completed! Return to the quest giver for credit.\n\r", tmp_char);

          tmp_char->questmob = NULL;
          tmp_char->quest_status = QUEST_COMPLETED;
          victim->questowner = NULL;
        }
      }

      save_char(tmp_char, NOWHERE);

      if ((mount = get_mount(tmp_char)))
      {
        if (GET_LEVEL(tmp_char) < 16)
        {
          experience = 0;
        }
        else if (GET_LEVEL(tmp_char) < 26)
        {
          experience = ((((total_exp / 2) / cummulative_levels) / 3) * GET_LEVEL(mount));
        }
        else
        {
          experience = (((total_exp / cummulative_levels) / 3) * GET_LEVEL(mount));
        }

        if (none)
        {
          experience = 0;
        }

        printf_to_char(mount, "You gained %d experience for the kill.\n\r", experience);

        gain_exp(mount, experience);
      }
    }
  }

  /* Check for death of a guildmaster. */
  if (IS_NPC(victim))
  {
    switch (V_MOB(victim))
    {
      case 3018: /* Nomad        */
      case 3019: /* Ninja        */
      case 3020: /* Mage         */
      case 3021: /* Cleric       */
      case 3022: /* Thief        */
      case 3023: /* Warrior      */
      case 3029: /* Commando     */
      case 3031: /* Bard         */
      case 3033: /* Paladin      */
      case 3035: /* Anti-Paladin */
        for (tmp_char = character_list; tmp_char; tmp_char = tmp_char->next)
        {
          if (!IS_NPC(tmp_char) &&
            tmp_char->questgiver == victim)
          {
            if (tmp_char->questmob)
            {
              tmp_char->questmob->questowner = NULL;
            }

            tmp_char->questmob = NULL;
            tmp_char->questgiver = NULL;
            tmp_char->quest_status = QUEST_FAILED;
            tmp_char->quest_level = 0;
            tmp_char->ver3.time_to_quest = 2;

            printf_to_char(tmp_char, "Someone has killed your guildmaster; you have failed your quest! You can start another in %d tick(s).\n\r",
              tmp_char->ver3.time_to_quest);
          }
        }
      break;
    }
  }

  /* wasn't killed by owner in the group */
  if (victim->questowner)
  {
    victim->questowner->questmob = NULL;
    victim->questowner->questgiver = NULL;
    victim->questowner->quest_status = QUEST_FAILED;
    victim->questowner->quest_level = 0;
    victim->questowner->ver3.time_to_quest = 2;

    printf_to_char(victim->questowner, "Someone has killed your victim; you have failed your quest! You can start another in %d tick(s).\n\r",
      victim->questowner->ver3.time_to_quest);

    victim->questowner = NULL;
  }
}

char *replace_string(char *str, char *weapon)
{
  static char buf[256];
  char *cp;

  cp = buf;

  while (*str)
  {
    if (*str == '#')
    {
      switch(*(++str))
      {
        case 'W':
          while (*weapon) *(cp++) = *(weapon++);
          break;

        default:
          *(cp++) = '#';
          break;
      }
    }
    else
    {
      *(cp++) = *str;
    }

    *cp = 0;

    str++;
  }

  return buf;
}


void dam_message(int dam, CHAR *ch, CHAR *victim, int attack_type, int shadow)
{
  char *buf;
  CHAR *tmp_char = NULL;
  int index = 0;

  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_weapons[] = {
    { /* 0 */
      "$n misses $N with $s #W.",
      "You miss $N with your #W.",
      "$n misses you with $s #W."
    },

    { /* > 0 */
      "$n tickles $N with $s #W.",
      "You tickle $N as you #W $M.",
      "$n tickles you as $e #W you."
    },

    { /* > 2 */
      "$n barely #W $N.",
      "You barely #W $N.",
      "$n barely #W you."
    },

    { /* > 4 */
      "$n #W $N.",
      "You #W $N.",
      "$n #W you."
    },

    { /* > 6 */
      "$n #W $N hard.",
      "You #W $N hard.",
      "$n #W you hard."
    },

    { /* > 10 */
      "$n #W $N very hard.",
      "You #W $N very hard.",
      "$n #W you very hard."
    },

    { /* > 15 */
      "$n #W $N extremely hard.",
      "You #W $N extremely hard.",
      "$n #W you extremely hard."
    },

    { /* > 20 */
      "$n massacres $N to small fragments with $s #W.",
      "You massacre $N to small fragments with your #W.",
      "$n massacres you to small fragments with $s #W."
    },

    { /* > 30 */
      "$n obliterates $N with $s #W.",
      "You obliterate $N with your #W.",
      "$n obliterates you with $s #W."
    },

    { /* > 50 */
      "$n utterly annihilates $N with $s #W.",
      "You utterly annihilate $N with your #W.",
      "$n utterly annihilates you with $s #W."
    },

    { /* > 70 */
      "$n removes chunks of flesh from $N with $s #W.",
      "You remove chunks of flesh from $N with your #W.",
      "$n sends chunks of your flesh flying with $s #W."
    },

    { /* > 100 */
      "$n makes $N see stars with a terrific wallop from $s #W.",
      "You make $N see stars with a terrific wallop from your #W.",
      "$n makes you see stars with a terrific wallop from $s #W."
    },

    { /* > 170 */
      "$n's #W makes $N think twice about $S continued existence.",
      "Your #W makes $N think twice about $S continued existence.",
      "$n's #W makes you think twice your continued existence."
    },

    { /* > 250 */
      "$N's bones crumble under $n's terrific #W.",
      "$N's bones crumble under your terrific #W.",
      "Your bones crumble under $n's terrific #W."
    },

    { /* > 350 */
      "$n's tremendous #W sends gouts of blood and gore showering from $N.",
      "Your tremendous #W sends gouts of blood and gore showering from $N.",
      "$n's tremendous #W sends gouts of blood and gore showering from you."
    },

    { /* > 450 */
      "With soul shattering force, $n's #W completely devastates $N.",
      "With soul shattering force, your #W completely devastates $N.",
      "With soul shattering force, $n's #W completely devastates you."
    },

    { /* > 550 */
      "$n's cataclysmic #W pulverizes $N's flesh into a fine paste.",
      "Your cataclysmic #W pulverizes $N's flesh into a fine paste.",
      "$n's cataclysmic #W pulverizes your flesh into a fine paste."
    }
  };

  static struct dam_shadow_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_shadow[] = {
    { /* 0 */
      "$n's shadow misses $N with its hit.",
      "Your shadow misses $N with its hit.",
      "$n's shadow misses you with its hit."
    },

    { /* > 0 */
      "$n's shadow tickles $N with its hit.",
      "Your shadow tickles $N as it hits $M.",
      "$n's shadow tickles you as it hits you."
    },

    { /* > 2 */
      "$n's shadow barely hits $N.",
      "Your shadow barely hits $N.",
      "$n's shadow barely hits you."
    },

    { /* > 4 */
      "$n's shadow hits $N.",
      "Your shadow hits $N.",
      "$n's shadow hits you."
    },

    { /* > 6 */
      "$n's shadow hits $N hard.",
      "Your shadow hits $N hard.",
      "$n's shadow hits you hard."
    },

    { /* > 10 */
      "$n's shadow hits $N very hard.",
      "Your shadow hits $N very hard.",
      "$n's shadow hits you very hard."
    },

    { /* > 15 */
      "$n's shadow hits $N extremely hard.",
      "Your shadow hits $N extremely hard.",
      "$n's shadow hits you extremely hard."
    },

    { /* > 20 */
      "$n's shadow massacres $N to small fragments with its hit.",
      "Your shadow massacres $N to small fragments with its hit.",
      "$n's shadow massacres you to small fragments with its hit."
    },

    { /* > 30 */
      "$n's shadow obliterates $N with its hit.",
      "Your shadow obliterates $N with its hit.",
      "$n's shadow obliterates you with its hit."
    },

    { /* > 50 */
      "$n's shadow utterly annihilates $N with its hit.",
      "Your shadow utterly annihilates $N with its hit.",
      "$n's shadow utterly annihilates you with its hit."
    },

    { /* > 70 */
      "$n's shadow removes chunks of flesh from $N with its hit.",
      "Your shadow removes chunks of flesh from $N with its hit.",
      "$n's shadow sends chunks of your flesh flying with its hit."
    },

    { /* > 100 */
      "$n's shadow makes $N see stars with a terrific wallop from its hit.",
      "Your shadow makes $N see stars with a terrific wallop from its hit.",
      "$n's shadow makes you see stars with a terrific wallop from its hit."
    },

    { /* > 170 */
      "$n's shadow's hit makes $N think twice about $S continued existence.",
      "Your shadow's hit makes $N think twice about $S continued existence.",
      "$n's shadow's hit makes you think twice your continued existence."
    },

    { /* > 250 */
      "$N's bones crumble under $n's shadow's terrific hit.",
      "$N's bones crumble under your shadow's terrific hit.",
      "Your bones crumble under $n's shadow's terrific hit."
    },

    { /* > 350 */
      "$n's shadow's tremendous hit sends gouts of blood and gore showering from $N.",
      "Your shadow's tremendous hit sends gouts of blood and gore showering from $N.",
      "$n's shadow's tremendous hit sends gouts of blood and gore showering from you."
    },

    { /* > 450 */
      "With soul shattering force, $n's shadow's hit completely devastates $N.",
      "With soul shattering force, your shadow's hit completely devastates $N.",
      "With soul shattering force, $n's shadow's hit completely devastates you."
    },

    { /* > 550 */
      "$n's shadow's cataclysmic hit pulverizes $N's flesh into a fine paste.",
      "Your shadow's cataclysmic hit pulverizes $N's flesh into a fine paste.",
      "$n's shadow's cataclysmic hit pulverizes your flesh into a fine paste."
    }

  };

  if      (dam < 1)    index = 0;
  else if (dam <= 2)   index = 1;
  else if (dam <= 4)   index = 2;
  else if (dam <= 6)   index = 3;
  else if (dam <= 10)  index = 4;
  else if (dam <= 15)  index = 5;
  else if (dam <= 20)  index = 6;
  else if (dam <= 30)  index = 7;
  else if (dam <= 50)  index = 8;
  else if (dam <= 70)  index = 9;
  else if (dam <= 100) index = 10;
  else if (dam <= 170) index = 11;
  else if (dam <= 250) index = 12;
  else if (dam <= 350) index = 13;
  else if (dam <= 450) index = 14;
  else if (dam <= 550) index = 15;
  else                 index = 16;

  /* Change to base of table with text. */
  attack_type -= TYPE_HIT;

  if (shadow)
  {
    if (index != 0 && index <= 6)
    {
      buf = replace_string(dam_shadow[index].to_room, attack_hit_text[attack_type].plural);
    }
    else
    {
      buf = replace_string(dam_shadow[index].to_room, attack_hit_text[attack_type].singular);
    }
  }
  else
  {
    if (index != 0 && index <= 6)
    {
      buf = replace_string(dam_weapons[index].to_room, attack_hit_text[attack_type].plural);
    }
    else
    {
      buf = replace_string(dam_weapons[index].to_room, attack_hit_text[attack_type].singular);
    }
  }

  /* act_by_type - for snoop brief only type so far is 1 - combat */
  for (tmp_char = world[CHAR_REAL_ROOM(ch)].people; tmp_char; tmp_char = tmp_char->next_in_room)
  {
    if (tmp_char == ch || tmp_char == victim) continue;

    if (!IS_SET(tmp_char->specials.pflag, PLR_FIGHTBRF))
    {
      act_by_type(buf, FALSE, ch, tmp_char, victim, TO_OTHER, 1);
    }
    else
    {
      if (index)
      {
        act_by_type("$n hits $N.", 0, ch, tmp_char, victim, TO_OTHER, 1);
      }
      else
      {
        act_by_type("$n misses $N.", 0, ch, tmp_char, victim, TO_OTHER, 1);
      }
    }
  }

  if (shadow)
  {
    buf = replace_string(dam_shadow[index].to_char, attack_hit_text[attack_type].singular);
  }
  else
  {
    buf = replace_string(dam_weapons[index].to_char, attack_hit_text[attack_type].singular);
  }
  act_by_type(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR, 1);

  if (shadow)
  {
    if (index != 0 && index <= 6)
    {
      buf = replace_string(dam_shadow[index].to_victim, attack_hit_text[attack_type].plural);
    }
    else
    {
      buf = replace_string(dam_shadow[index].to_victim, attack_hit_text[attack_type].singular);
    }
  }
  else
  {
    if (index != 0 && index <= 6)
    {
      buf = replace_string(dam_weapons[index].to_victim, attack_hit_text[attack_type].plural);
    }
    else
    {
      buf = replace_string(dam_weapons[index].to_victim, attack_hit_text[attack_type].singular);
    }
  }
  act_by_type(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT, 1);
}


void process_death(CHAR *ch, CHAR *victim) {
  CHAR *temp = NULL;
  DESC *d = NULL;
  char buf[MSL];
  char *tmstr = NULL;
  long ct = 0;

  if (IS_NPC(victim)) {
    /* Died due to blood loss; find the next attacker in the combat_list and give them EXP. */
    if (ch == victim) {
      for (temp = combat_list; temp; temp = temp->next_fighting) {
        if (GET_OPPONENT(temp) == victim &&
            CHAR_REAL_ROOM(temp) == CHAR_REAL_ROOM(victim)) {
          divide_experience(temp, victim, 0);

          break;
        }
      }
    }
    else {
      divide_experience(ch, victim, 0);
    }
  }

  if (GET_MOUNT(victim)) {
    stop_follower(GET_MOUNT(victim));
  }

  if (GET_RIDER(victim)) {
    stop_riding(GET_RIDER(victim), victim);
    stop_follower(victim);
  }

  if (!IS_NPC(victim) &&
      !IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, CHAOTIC)) {
    if (ch != victim) {
      sprintf(buf, "%s killed by %s at %s [%d]", GET_NAME(victim), (IS_NPC(ch) ? MOB_SHORT(ch) : GET_NAME(ch)),
              world[CHAR_REAL_ROOM(victim)].name, world[CHAR_REAL_ROOM(victim)].number);
    }
    else {
      sprintf(buf, "%s dies from bloodlack at %s [%d]", GET_NAME(victim),
              world[CHAR_REAL_ROOM(victim)].name, world[CHAR_REAL_ROOM(victim)].number);
    }

    log_s(buf);
    deathlog(buf);

    if ((CHAOSMODE || (IS_NPC(ch) && GET_LEVEL(victim) > 14)) && ch != victim) {
      brag(ch, victim);    /* Mob brag - Ranger Aug 00 - Added player brag in Chaos Oct 00*/
    }

    if (ch != victim) {
      ch->new.killed = ch->new.killed + 1;
    }

    if (CHAOSMODE && GET_LEVEL(victim) < LEVEL_IMM) {
      for (d = descriptor_list; d; d = d->next) {
        if (!d->connected) {
          act(buf, 0, d->character, 0, 0, TO_CHAR);
        }
      }

      number_of_kills++;

      if (number_of_kills < 100) {
        sprintf(scores[number_of_kills].killer, "%s", (IS_NPC(ch) ? MOB_SHORT(ch) : GET_NAME(ch)));
        sprintf(scores[number_of_kills].killed, "%s", GET_NAME(victim));
        sprintf(scores[number_of_kills].location, "%s", world[CHAR_REAL_ROOM(victim)].name);
        ct = time(0);
        tmstr = asctime(localtime(&ct));
        *(tmstr + strlen(tmstr) - 1) = '\0';
        sprintf(scores[number_of_kills].time_txt, "%s", tmstr);

        if (number_of_kills == CHAOSDEATH) {
          sprintf(buf,
                  "**** Kill number %d has been reached, we have a winner!!! ****\n\r\n\r",
                  CHAOSDEATH);
          send_to_all(buf);
          send_to_all(buf); /* yes, twice */
        }
      }
      else {
        number_of_kills = 99;
      }
    }
    else {
      wizlog(buf, LEVEL_IMM, 3);
    }
  }

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, CHAOTIC)
      && !IS_NPC(victim)) {
    if (ch != victim) {
      sprintf(buf, "\n\rThe Dungeonmaster (chaos) [ ** %s slays %s at ", IS_NPC(ch) ? MOB_SHORT(ch) : GET_NAME(ch), GET_NAME(victim));
    }
    else {
      sprintf(buf, "\n\rThe Dungeonmaster (chaos) [ ** %s dies from bloodlack at ", GET_NAME(ch));
    }

    strcat(buf, world[CHAR_REAL_ROOM(ch)].name);
    strcat(buf, " ** ]\n\r\n\r");

    if (ch != victim) {
      ch->new.killed = ch->new.killed + 1;
    }

    for (d = descriptor_list; d; d = d->next) {
      if (d->character && (d->character != ch) && !d->connected
          && ((!IS_SET(d->character->specials.pflag, PLR_NOSHOUT)
          && IS_SET(d->character->specials.pflag, PLR_CHAOS))
          || d->original)) {
        COLOR(d->character, 15);
        send_to_char(buf, d->character);
        ENDCOLOR(d->character);
      }
    }

    number_of_kills++;

    if (number_of_kills < 100) {
      sprintf(scores[number_of_kills].killer, "%s", (IS_NPC(ch) ? MOB_SHORT(ch) : GET_NAME(ch)));
      sprintf(scores[number_of_kills].killed, "%s", GET_NAME(victim));
      sprintf(scores[number_of_kills].location, "%s", world[CHAR_REAL_ROOM(victim)].name);
      ct = time(0);
      tmstr = asctime(localtime(&ct));
      *(tmstr + strlen(tmstr) - 1) = '\0';
      sprintf(scores[number_of_kills].time_txt, "%s", tmstr);
    }
    else {
      number_of_kills = 99;
    }
  }

  if (IS_NPC(ch) &&
      !IS_NPC(victim) &&
      IS_SET(GET_ACT(ch), ACT_MEMORY)) {
    forget(victim, ch);
  }

  die(victim);
  signal_char(ch, ch, MSG_DEATHCRY, "");
}


/* Returns the amount of damage done after any/all mitigation. */
int damage(CHAR *ch, CHAR *victim, int dmg, int attack_type, int damage_type) {
  char buf[MSL];
  struct message_type *messages = NULL;
  int original_damage = 0;
  int shadow_damage = 0;
  int reflect = 0;
  int max_reflect = 0;
  int reduct = 0;
  int mana_shield = 0;
  int dmg_text = 0;
  int i = 0;
  int j = 0;
  int nr = 0;

  original_damage = dmg;

  if (!ch || !victim) {
    dmg = 0;

    return dmg;
  }

  /* Can't damage the dead. */
  if (!IS_ALIVE(victim)) {
    dmg = 0;

    return dmg;
  }

  /* Avoid doing damage to players not in the same room. */
  if (!SAME_ROOM(victim, ch)) {
    dmg = 0;

    return dmg;
  }

  /* No damage to victims in safe rooms. */
  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    dmg = 0;

    return dmg;
  }

  /* No damage to immortals. */
  if (IS_IMMORTAL(victim)) {
    dmg = 0;
  }

  /* Adjust for shadow damage. */
  if (attack_type == TYPE_SHADOW) {
    attack_type = TYPE_HIT;
    shadow_damage = TRUE;
  }

  if (!IS_NPC(victim) &&
      GET_PROTECTOR(victim) &&
      !IS_NPC(GET_PROTECTOR(victim)) &&
      IS_ALIVE(GET_PROTECTOR(victim)) &&
      SAME_ROOM(victim, GET_PROTECTOR(victim)) &&
      GET_PROTECTEE(GET_PROTECTOR(victim)) == victim &&
      (!IS_AFFECTED(GET_PROTECTOR(victim), AFF_FURY) || GET_CLASS(GET_PROTECTOR(victim)) == CLASS_PALADIN) &&
      number(1, 101) <= GET_LEARNED(GET_PROTECTOR(victim), SKILL_PROTECT) &&
      chance(90)) {
    act("$N takes the damage meant for you!", FALSE, victim, 0, GET_PROTECTOR(victim), TO_CHAR);
    act("You take the damage meant for $n!", FALSE, victim, 0, GET_PROTECTOR(victim), TO_VICT);

    victim = GET_PROTECTOR(victim);
  }

  /* If the victim is praying, interrupt them and apply a 2 round input lag. */
  if (affected_by_spell(victim, SKILL_PRAY)) {
    affect_from_char(victim, SKILL_PRAY);
    send_to_char("Your prayers are interrupted and you are slightly confused.\n\r", ch);
    WAIT_STATE(victim, PULSE_VIOLENCE * 2);
  }

  /* No damage dealt by a player if they are sleeping, resting or sitting; except when ch == victim. */
  if (victim != ch &&
      !IS_NPC(ch) &&
      (GET_POS(ch) == POSITION_SLEEPING || GET_POS(ch) == POSITION_RESTING || GET_POS(ch) == POSITION_SITTING)) {
    dmg = 0;
  }

  /* Process Immunities and Resistances */
  if (IS_NPC(victim)) {
    /* specials.immune */
    if (GET_IMMUNE(victim) &&
        ((attack_type == TYPE_HIT && IS_IMMUNE(victim, IMMUNE_HIT)) ||
        (attack_type == TYPE_BLUDGEON && IS_IMMUNE(victim, IMMUNE_BLUDGEON)) ||
        (attack_type == TYPE_PIERCE && IS_IMMUNE(victim, IMMUNE_PIERCE)) ||
        (attack_type == TYPE_SLASH && IS_IMMUNE(victim, IMMUNE_SLASH)) ||
        (attack_type == TYPE_WHIP && IS_IMMUNE(victim, IMMUNE_WHIP)) ||
        (attack_type == TYPE_CLAW && IS_IMMUNE(victim, IMMUNE_CLAW)) ||
        (attack_type == TYPE_BITE && IS_IMMUNE(victim, IMMUNE_BITE)) ||
        (attack_type == TYPE_STING && IS_IMMUNE(victim, IMMUNE_STING)) ||
        (attack_type == TYPE_CRUSH && IS_IMMUNE(victim, IMMUNE_CRUSH)) ||
        (attack_type == TYPE_HACK && IS_IMMUNE(victim, IMMUNE_HACK)) ||
        (attack_type == TYPE_CHOP && IS_IMMUNE(victim, IMMUNE_CHOP)) ||
        (attack_type == TYPE_SLICE && IS_IMMUNE(victim, IMMUNE_SLICE)) ||
        (damage_type == DAM_FIRE && IS_IMMUNE(victim, IMMUNE_FIRE)) ||
        (damage_type == DAM_ELECTRIC && IS_IMMUNE(victim, IMMUNE_ELECTRIC)) ||
        (damage_type == DAM_POISON && IS_IMMUNE(victim, IMMUNE_POISON)))) {
      dmg = 0;
    }

    /* specials.immune2 */
    if (GET_IMMUNE2(victim) &&
        ((damage_type == DAM_SOUND && IS_IMMUNE2(victim, IMMUNE_COLD)) ||
        (damage_type == DAM_ACID && IS_IMMUNE2(victim, IMMUNE_SOUND)) ||
        (damage_type == DAM_CHEMICAL && IS_IMMUNE2(victim, IMMUNE_CHEMICAL)) ||
        (damage_type == DAM_ACID && IS_IMMUNE(victim, IMMUNE_ACID)))) {
      dmg = 0;
    }

    /* specials.resist */
    if (GET_RESIST(victim) &&
        ((damage_type == DAM_FIRE && IS_RESISTANT(victim, RESIST_FIRE)) ||
        (damage_type == DAM_ELECTRIC && IS_RESISTANT(victim, RESIST_ELECTRIC)) ||
        (damage_type == DAM_COLD && IS_RESISTANT(victim, RESIST_COLD)) ||
        (damage_type == DAM_SOUND && IS_RESISTANT(victim, RESIST_SOUND)) ||
        (damage_type == DAM_CHEMICAL && IS_RESISTANT(victim, RESIST_CHEMICAL)) ||
        (damage_type == DAM_ACID && IS_RESISTANT(victim, RESIST_ACID)) ||
        (damage_type == DAM_MAGICAL && IS_RESISTANT(victim,  RESIST_MAGICAL)) ||
        (damage_type == DAM_PHYSICAL && IS_RESISTANT(victim, RESIST_PHYSICAL)))) {
      dmg = lround(dmg * (number(25, 75) / 100.0));
    }
  }

  /* Force a player mount to flee if it does damage to a player that isn't a thief or killer; except when damage is of the type DAM_NO_BLOCK_NO_FLEE.*/
  if (IS_NPC(ch) &&
      GET_RIDER(ch) &&
      !IS_NPC(GET_RIDER(ch)) &&
      !IS_NPC(victim) &&
      (!IS_SET(GET_PFLAG(victim), PLR_KILL) || !IS_SET(GET_PFLAG(victim), PLR_THIEF)) &&
      damage_type != DAM_NO_BLOCK_NO_FLEE) {
    do_flee(ch, "", CMD_FLEE);

    dmg = 0;

    return dmg;
  }

  /* Handle PvP */
  if (victim != ch &&
      !IS_NPC(ch) &&
      !IS_NPC(victim) &&
      !ROOM_ARENA(CHAR_REAL_ROOM(victim)) &&
      !ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) &&
      (!IS_SET(GET_PFLAG(victim), PLR_KILL) || !IS_SET(GET_PFLAG(victim), PLR_THIEF))) {
    /* Prevent players from PKing if PLR_NOKILL is turned on. */
    if (IS_SET(GET_PFLAG(ch), PLR_NOKILL)) {
      send_to_char("You can't attack other players.\n\r", ch);

      dmg = 0;

      return dmg;
    }
    /* Flag the attacker as a killer if PKing. */
    else {
      send_to_char("You are a killer!\n\r", ch);

      SET_BIT(GET_PFLAG(ch), PLR_KILL);

      sprintf(buf, "PLRINFO: %s just attacked %s; Killer flag set. (Room %d)",
              GET_NAME(ch), GET_NAME(victim), world[CHAR_REAL_ROOM(ch)].number);
      wizlog(buf, LEVEL_SUP, 4);
      log_s(buf);
    }
  }

  /* Prevent players from attacking mounts if PLR_NOKILL is turned on. */
  if (!IS_NPC(ch) &&
      IS_SET(GET_PFLAG(ch), PLR_NOKILL) &&
      IS_NPC(victim) &&
      IS_MOUNT(victim) &&
      !ROOM_ARENA(CHAR_REAL_ROOM(victim)) &&
      !ROOM_CHAOTIC(CHAR_REAL_ROOM(victim))) {
    send_to_char("You can't attack mounts.\n\r", ch);

    dmg = 0;

    return dmg;
  }

  /* Handle charmed NPCs that might be wielding an ANTI-MORTAL weapon. */
  if (IS_NPC(ch) &&
      IS_AFFECTED(ch, AFF_CHARM) &&
      GET_WEAPON(ch) &&
      IS_SET(OBJ_EXTRA_FLAGS(GET_WEAPON(ch)), ITEM_ANTI_MORTAL)) {
    send_to_char("Perhaps you shouldn't be using an ANTI-MORTAL weapon.\n\r", ch);

    dmg = 0;

    return dmg;
  }

  /* Prevent charmed NPCs from attacking players except during Chaos. */
  if (IS_NPC(ch) &&
      IS_AFFECTED(ch, AFF_CHARM) &&
      !IS_NPC(victim) &&
      GET_OPPONENT(victim) != ch &&
      !CHAOSMODE) {
    send_to_char("You can't harm a player!\n\r", ch);

    dmg = 0;

    return dmg;
  }

  /* Start or stop people fighting. */
  if (victim != ch) {
    /* Process the victim. */
    if (GET_POS(victim) > POSITION_STUNNED) {
      /* Start the victim fighting the attacker. */
      if (!GET_OPPONENT(victim)) {
        set_fighting(victim, ch);
      }

      /* Make mobs with memory remember their attacker. */
      if (IS_NPC(victim) &&
          IS_SET(GET_ACT(victim), ACT_MEMORY) &&
          !IS_NPC(ch) &&
          CAN_SEE(victim, ch)) {
        remember(ch, victim);
      }

      GET_POS(victim) = POSITION_FIGHTING;
    }

    /* Process the attacker. */
    if (GET_POS(ch) > POSITION_STUNNED) {
      /* Start the attacker fighting the victim. */
      if (!GET_OPPONENT(ch)) {
        set_fighting(ch, victim);

        GET_POS(ch) = POSITION_FIGHTING;
      }

      /* NPCs have a chance to switch to a charmed NPC's master. */
      if (IS_NPC(ch) &&
          IS_NPC(victim) &&
          IS_AFFECTED(victim, AFF_CHARM) &&
          GET_MASTER(victim) &&
          CHAR_REAL_ROOM(GET_MASTER(victim)) == CHAR_REAL_ROOM(ch) &&
          !number(0, 10)) {
        if (GET_OPPONENT(ch)) {
          stop_fighting(ch);
        }

        hit(ch, GET_MASTER(victim), TYPE_UNDEFINED);

        dmg = 0;

        return dmg;
      }

      /* NPCs have a chance to switch to a mount's rider. */
      if (IS_NPC(ch) &&
          IS_NPC(victim) &&
          GET_RIDER(victim) &&
          CHAR_REAL_ROOM(GET_RIDER(victim)) == CHAR_REAL_ROOM(ch) &&
          !number(0, 10)) {
        if (GET_OPPONENT(ch)) {
          stop_fighting(ch);
        }

        hit(ch, GET_RIDER(victim), TYPE_UNDEFINED);

        dmg = 0;

        return dmg;
      }
    }
  }

  /* Stop riding a mount if the master attacks it and stop the mount from following the master. */
  if (GET_MASTER(victim) == ch) {
    if (GET_RIDER(victim)) {
      stop_riding(ch, victim);
    }

    stop_follower(victim);
  }

  /* Make the attacker appear if they are invisible. */
  if (IS_AFFECTED(ch, AFF_INVISIBLE)) {
    appear(ch);
  }

  /* Handle TYPE_KILL */
  if (attack_type == TYPE_KILL) {
    GET_HIT(victim) = 1;
    GET_MANA(victim) = -100;
  }

  /* Process weapon damage improving skills/spells. */
  if (dmg > 0 &&
      attack_type >= TYPE_HIT &&
      attack_type <= TYPE_SLICE) {
    /* Fury */
    if (IS_AFFECTED(ch, AFF_FURY)) {
      /* Paladin Level 50 */
      if (GET_CLASS(ch) == CLASS_PALADIN &&
          GET_LEVEL(ch) == 50 &&
          GET_ALIGNMENT(ch) > 500 &&
          !CHAOSMODE) {
        if (GET_ALIGNMENT(ch) > 900) {
          dmg = lround(dmg * 2.5);
        }
        else if (GET_ALIGNMENT(ch) > 800) {
          dmg = lround(dmg * 2.4);
        }
        else if (GET_ALIGNMENT(ch) > 700) {
          dmg = lround(dmg * 2.3);
        }
        else if (GET_ALIGNMENT(ch) > 600) {
          dmg = lround(dmg * 2.2);
        }
        else {
          dmg = lround(dmg * 2.1);
        }
      }
      /* Normal Fury */
      else {
        dmg *= 2;
      }
    }

    /* Infidel SC3: Rage */
    if (!IS_AFFECTED(ch, AFF_FURY) &&
        affected_by_spell(ch, SPELL_RAGE)) {
      dmg = lround(dmg * 1.5);
    }

    /* Frenzy */
    if (!IS_AFFECTED(ch, AFF_FURY) &&
        affected_by_spell(ch, SKILL_FRENZY) &&
        !shadow_damage) {
      dmg = lround(dmg * 1.5);
    }
  }

  /* Crusader SC1: Righteousness */
  if (dmg > 0 &&
      damage_type == DAM_PHYSICAL &&
      affected_by_spell(ch, SPELL_RIGHTEOUSNESS) &&
      IS_EVIL(victim)) {
    dmg += 5;
  }

  /* Invulnerability */
  if (IS_AFFECTED(victim, AFF_INVUL)) {
    /* Reduce DAM_PHYSICAL damage less than 20 to 0. */
    if (damage_type == DAM_PHYSICAL &&
        (!IS_AFFECTED(victim, AFF_FURY) || IS_NPC(victim)) &&
        dmg < 20) {
      dmg = 0;
    }

    /* Reduce DAM_SKILL damage to 0 if breakthrough() fails. */
    if (damage_type == DAM_SKILL &&
        !IS_AFFECTED(victim, AFF_FURY) &&
        !breakthrough(ch, victim, BT_INVUL)) {
      dmg = 0;
    }
  }

  /* Enchanter SC4: Ethereal Nature */
  if (damage_type == DAM_PHYSICAL || damage_type == DAM_SKILL) {
    if (affected_by_spell(victim, SPELL_ETHEREAL_NATURE) &&
        (duration_of_spell(victim, SPELL_ETHEREAL_NATURE) == 30 ||
         (CHAOSMODE && duration_of_spell(victim, SPELL_ETHEREAL_NATURE) == 12))) {
      dmg = 0;
    }

    if (affected_by_spell(ch, SPELL_ETHEREAL_NATURE) &&
        (duration_of_spell(ch, SPELL_ETHEREAL_NATURE) == 30 ||
         (CHAOSMODE && duration_of_spell(ch, SPELL_ETHEREAL_NATURE) == 12))) {
      dmg = 0;
    }
  }

  /* Defiler SC1: Blackmantle */
  if (reflect <= 0 &&
      attack_type >= TYPE_HIT &&
      attack_type <= TYPE_SLICE &&
      affected_by_spell(victim, SPELL_BLACKMANTLE)) {
    /* Blackmantle inflicts some damage even if the attacker misses. */
    if (dmg <= 0) {
      reflect = MIN(original_damage, GET_LEVEL(victim) / 5);

      act("$n is scorched by your mantle of darkness as $e gets too close.", FALSE, ch, 0, victim, TO_VICT);
      act("$n is scorched by $N's mantle of darkness as $e gets too close.", FALSE, ch, 0, victim, TO_NOTVICT);
      act("You are scorched by $N's mantle of darkness as you get too close!", FALSE, ch, 0, victim, TO_CHAR);
    }
    else {
      max_reflect = GET_LEVEL(victim) * 2;

      if (IS_AFFECTED(victim, AFF_FURY)) {
        max_reflect *= 2;
      }
      else if (affected_by_spell(victim, SPELL_RAGE)) {
        max_reflect = lround(max_reflect * 1.5);
      }

      if (max_reflect > 0 &&
          affected_by_spell(victim, SPELL_RIGHTEOUSNESS) &&
          IS_EVIL(ch)) {
        max_reflect += 5;
      }

      reflect = MIN(max_reflect, dmg / 10);
      reflect = MAX(reflect, GET_LEVEL(victim) / 5);
      dmg = MAX(0, dmg - reflect);

      act("Your mantle of darkness reflects some of $n's damage back to $m.", FALSE, ch, 0, victim, TO_VICT);
      act("$N's mantle of darkness reflects some of $n's damage back to $m.", FALSE, ch, 0, victim, TO_NOTVICT);
      act("$N's mantle of darkness reflects some of your damage back to you!", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  /* Enchanter SC1: Blade Barrier */
  if (dmg > 0 &&
      reflect <= 0 &&
      attack_type >= TYPE_HIT &&
      attack_type <= TYPE_SLICE &&
      affected_by_spell(victim, SPELL_BLADE_BARRIER)) {
    max_reflect = calc_hit_damage(victim, ch, GET_WEAPON(victim));

    if (IS_AFFECTED(victim, AFF_FURY)) {
      max_reflect *= 2;
    }

    if (max_reflect > 0 &&
        damage_type == DAM_PHYSICAL &&
        affected_by_spell(victim, SPELL_RIGHTEOUSNESS) &&
        IS_EVIL(ch)) {
      max_reflect += 5;
    }

    reflect = MIN(max_reflect, dmg / 4);
    dmg = MAX(0, dmg - reflect);

    act("Some of $n's damage is reflected to $m.", 0, ch, 0, 0, TO_ROOM);
    send_to_char("Some of your damage is reflected to you!\n\r", ch);
  }

  /* Protection from Evil */
  if (dmg > 0 &&
      IS_AFFECTED(victim, AFF_PROTECT_EVIL) &&
      IS_EVIL(ch) &&
      (!IS_EVIL(victim) || IS_NPC(victim))) {
    reduct = MIN(10, (GET_LEVEL(victim) + 10) - GET_LEVEL(ch));
    dmg = MAX(0, dmg - reduct);
  }

  /* Protection from Good */
  if (dmg > 0 &&
      IS_AFFECTED(victim, AFF_PROTECT_GOOD) &&
      IS_GOOD(ch) &&
      (!IS_GOOD(victim) || IS_NPC(victim))) {
    reduct = MIN(10, (GET_LEVEL(victim) + 10) - GET_LEVEL(ch));
    dmg = MAX(0, dmg - reduct);
  }

  /* Sphere */
  if (IS_AFFECTED(victim, AFF_SPHERE) &&
      damage_type >= DAM_MAGICAL) {
    if (!breakthrough(ch, victim, BT_SPHERE)) {
      /* Archmage SC5: Distortion allows 50% of DAM_MAGICAL damage through Sphere. */
      if (affected_by_spell(victim, SPELL_DISTORTION)) {
        dmg = lround(dmg * 0.50);
      }
      else {
        dmg = 0;
      }
    }
  }

  /* Constitution Damage Reduction */
  dmg = lround(dmg * (1.0 - (con_app[GET_CON(victim)].reduct / 100.0)));

  /* Sanctuary and Templar SC5: Fortification */
  if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
    if (affected_by_spell(victim, SPELL_DISRUPT_SANCT)) {
      /* 15% reduction with only Fortification. */
      if (affected_by_spell(victim, SPELL_FORTIFICATION)) {
        dmg = lround(dmg * 0.85);
      }
    }
    else {
      /* 65% reduction with both Sanctuary and Fortification. */
      if (affected_by_spell(victim, SPELL_FORTIFICATION)) {
        dmg = lround(dmg * 0.35);
      }
      /* 50% reduction with only Sanctuary. */
      else {
        dmg = lround(dmg * 0.50);
      }
    }
  }

  /* Bandit SC5: Evasion */
  if (affected_by_spell(victim, SKILL_EVASION)) {
    if (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim))) {
      dmg = lround(dmg * 0.80); /* 20% reduction when Chaotic. */
    }
    else {
      dmg = lround(dmg * 0.75); /* 25% reduction normally. */
    }
  }

  /* Ronin SC1: Combat Zen grants half damage from poison. */
  if (damage_type == DAM_POISON &&
      check_subclass(victim, SC_RONIN, 1)) {
    dmg = lround(dmg * 0.50);
  }

  /* Limit total damage. */
  dmg = MIN(30000, dmg);
  dmg = MAX(0, dmg);

  /* Limit PvP damage in chaotic rooms and during Chaos. */
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) &&
      IS_MORTAL(ch) &&
      IS_MORTAL(victim)) {
    dmg = MIN(600, dmg);
  }

  /* Record damage for use later when printing damage text. */
  dmg_text = dmg;

  /* Archmage SC2/Templar SC4: Orb of Protection */
  if (dmg > 0 &&
      affected_by_spell(victim, SPELL_ORB_PROTECTION)) {
    mana_shield = ((200 + dmg) - GET_HIT(victim)) / 2;

    if (mana_shield > 0) {
      mana_shield = MIN(mana_shield, GET_MANA(victim));
      GET_MANA(victim) -= mana_shield;
      dmg = MAX(0, dmg - (mana_shield * 2));
    }
  }

  /* Infidel SC5: Shadow Wraith */
  if (shadow_damage) {
    shadow_damage = number(2, 6);
    dmg /= shadow_damage;
    dmg_text /= shadow_damage;
  }

  /* It's so anti-climactic. */
  GET_HIT(victim) -= dmg;
  update_pos(victim);

  /* Grant hit EXP. */
  if (ch != victim) {
    gain_exp(ch, (GET_LEVEL(victim) * dmg) / 4);

    if (GET_REMORT_EXP(ch)) {
      rv2_gain_remort_exp(ch, (GET_LEVEL(victim) * dmg) / 4);
    }

    if (GET_DEATH_EXP(ch)) {
      gain_death_exp(ch, (GET_LEVEL(victim) * dmg) / 4);
    }
  }

  /* Cleric Level 50: Divine Intervention */
  if (affected_by_spell(victim, SPELL_DIVINE_INTERVENTION) &&
      (GET_POS(victim) <= POSITION_INCAP)) {
    /* Divine Intervention doesn't work on actively Degenerated people. */
    if (affected_by_spell(victim, SPELL_DEGENERATE) &&
        (duration_of_spell(victim, SPELL_DEGENERATE) > (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) ? 9 : 27))) {
      send_to_char("Your call to the gods to intervene and heal your degenerated body falls on deaf ears.\n\r", victim);
    }
    else {
      GET_HIT(victim) = GET_MAX_HIT(victim);

      act("$n's life has been restored by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
      send_to_char("Your life has been restored by divine forces.\n\r", victim);
    }

    affect_from_char(victim, SPELL_DIVINE_INTERVENTION);
    update_pos(victim);
  }

  /* Handle death. */
  if (GET_POS(victim) == POSITION_DEAD) {
    /* Signal death in non-chaotic rooms (or always during Chaos). */
    if (!ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) || CHAOSMODE) {
      signal_char(victim, ch, MSG_DIE, "");
    }

    update_pos(victim);
  }

  /* Print messages for physical attacks. */
  if (attack_type >= TYPE_HIT &&
      attack_type <= TYPE_SLICE &&
      GET_POS(victim) != POSITION_DEAD) {
    if (dmg_text) {
      if (!IS_NPC(ch)) {
        COLOR(ch, 11);
      }

      if (!IS_NPC(victim)) {
        COLOR(victim, 12);
      }
    }

    if (!GET_WEAPON(ch)) {
      dam_message(dmg_text, ch, victim, TYPE_HIT, shadow_damage);
    }
    else {
      dam_message(dmg_text, ch, victim, attack_type, shadow_damage);
    }

    if (!IS_NPC(ch)) {
      ENDCOLOR(ch);
    }

    if (!IS_NPC(victim)) {
      ENDCOLOR(victim);
    }
  }
  /* Print messages for skills and spells. */
  else {
    if (attack_type >= TYPE_HIT &&
        attack_type <= TYPE_SLICE &&
        !GET_WEAPON(ch)) {
      attack_type = TYPE_HIT;
    }

    for (i = 0; i < MAX_MESSAGES; i++) {
      if (fight_messages[i].a_type == attack_type) {
        nr = dice(1, fight_messages[i].number_of_attacks);

        for (j = 1, messages = fight_messages[i].msg; (j < nr) && messages; j++) {
          messages = messages->next;
        }

        if (IS_IMMORTAL(victim)) {
          act(messages->god_msg.attacker_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_CHAR);
          act(messages->god_msg.victim_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_VICT);
          act(messages->god_msg.room_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_NOTVICT);

          break;
        }

        /* Damage > 0 (Hit) */
        if (dmg > 0 ||
            (original_damage > 0 &&
             (attack_type == SKILL_PUNCH ||
              attack_type == SKILL_BASH ||
              attack_type == SKILL_KICK))) {
          /* Death Messages */
          if (GET_POS(victim) == POSITION_DEAD) {
            act(messages->die_msg.attacker_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_CHAR);
            act(messages->die_msg.victim_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_VICT);
            act(messages->die_msg.room_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_NOTVICT);
          }
          /* Damage Messages */
          else {
            act(messages->hit_msg.attacker_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_CHAR);
            act(messages->hit_msg.victim_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_VICT);
            act(messages->hit_msg.room_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_NOTVICT);
          }
        }
        /* Damage <= 0 (Miss) */
        else {
          act(messages->miss_msg.attacker_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_CHAR);
          act(messages->miss_msg.victim_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_VICT);
          act(messages->miss_msg.room_msg, FALSE, ch, GET_WEAPON(ch), victim, TO_NOTVICT);
        }

        break;
      }
    }
  }

  /* Print position messages. */
  switch (GET_POS(victim)) {
    case POSITION_MORTALLYW:
      act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are mortally wounded, and will die soon, if not aided.\n\r", victim);
      break;

    case POSITION_INCAP:
      act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are incapacitated an will slowly die, if not aided.\n\r", victim);
      break;

    case POSITION_STUNNED:
      act("$n is stunned, but will probably regain consciousness.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You're stunned, but will probably regain consciousness.\n\r", victim);
      break;

    case POSITION_DEAD:
      /* Disembowel */
      if (attack_type == SKILL_DISEMBOWEL) {
        act("Guts spatter everywhere. Yuck!", FALSE, victim, 0, 0, TO_ROOM);
      }

      /* Handle MSG_DEAD in case it was intercepted. */
      if (signal_char(victim, ch, MSG_DEAD, "")) {
        return dmg;
      }

      act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are dead! Sorry...\n\r", victim);
      break;

    /* >= POSITION SLEEPING */
    default:
      /* Handle player wimpy. */
      if (!IS_NPC(victim) &&
          GET_HIT(victim) < GET_WIMPY(victim) &&
          !ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) &&
          damage_type != DAM_NO_BLOCK_NO_FLEE) {
        do_flee(victim, "", CMD_FLEE);

        if (CHAR_REAL_ROOM(ch) != CHAR_REAL_ROOM(victim)) {
          return dmg;
        }
      }

      /* Send a message if the victim received damage equal to or greater than 20% of their maximum HP. */
      if (dmg_text > (hit_limit(victim) / 5)) {
        send_to_char("That really did HURT!\n\r", victim);
      }

      /* Handle bleed limit. NPCs flee when under 20% of their maximum HP. */
      if ((IS_NPC(victim) && (GET_HIT(victim) < (hit_limit(victim) / 5))) ||
          (!IS_NPC(victim) && (!GET_BLEED(victim) && (GET_HIT(victim) < (hit_limit(victim) / 5)))) ||
          (!IS_NPC(victim) && (GET_BLEED(victim) && (GET_HIT(victim) < GET_BLEED(victim))))) {
        send_to_char("You wish that your wounds would stop BLEEDING that much!\n\r", victim);

        /* Handle NPC wimpy. */
        if (IS_NPC(victim) &&
            IS_SET(GET_ACT(victim), ACT_WIMPY) &&
            GET_POS(victim) > POSITION_STUNNED &&
            damage_type != DAM_NO_BLOCK_NO_FLEE) {
          do_flee(victim, "", CMD_FLEE);

          if (CHAR_REAL_ROOM(ch) != CHAR_REAL_ROOM(victim)) {
            return dmg;
          }
        }
      }
      break;
  }

  /* Handle link-dead players. */
  if (!IS_NPC(victim) &&
      !GET_DESCRIPTOR(victim) &&
      damage_type != DAM_NO_BLOCK_NO_FLEE) {
    do_flee(victim, "", 0);

    if (!GET_OPPONENT(victim)) {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);

      GET_WAS_IN_ROOM(victim) = CHAR_REAL_ROOM(victim);
      char_from_room(victim);
      char_to_room(victim, 1);

      GET_POS(victim) = POSITION_STUNNED;
    }

    if (CHAR_REAL_ROOM(ch) != CHAR_REAL_ROOM(victim)) {
      return dmg;
    }
  }

  /* Handle vicious. */
  if (GET_HIT(victim) <= 0 &&
      GET_OPPONENT(ch) == victim) {
    if (IS_SET(GET_PFLAG(ch), PLR_VICIOUS)) {
      if (GET_POS(victim) < POSITION_MORTALLYW) {
        stop_fighting(ch);
      }

      if (GET_POS(victim) < POSITION_STUNNED &&
          IS_GOOD(ch)) {
        GET_ALIGNMENT(ch) -= 5;
      }
    }
    else {
      if (GET_POS(victim) < POSITION_STUNNED) {
        stop_fighting(ch);
      }
    }
  }

  /* Handle sleeping victims. */
  if (!AWAKE(victim) &&
      GET_OPPONENT(victim)) {
    stop_fighting(victim);
  }

  /* Process reflected damage. */
  if (reflect > 0 &&
      GET_POS(victim) > POSITION_DEAD) {
    damage(victim, ch, reflect, TYPE_UNDEFINED, damage_type);
  }

  /* Process death. */
  if (GET_POS(victim) == POSITION_DEAD) {
    process_death(ch, victim);
  }

  return dmg;
}

/* Assumes a weapon is wielded. */
int wpn_extra(CHAR *ch, CHAR *victim, OBJ *weapon)
{
  int dam = 0;
  int attack_type = 0;
  int victim_class = 0;

  if (!ch) return 0;
  if (!victim) return 0;
  if (!weapon) return 0;

  if (weapon->obj_flags.value[0] <= 20) return 0;

  attack_type = weapon->obj_flags.value[0];

  if (victim->specials.fighting) victim_class = (int)GET_CLASS(victim);

  if (attack_type == victim_class)
  {
    dam += number(1, 5);
  }
  else
  {
    switch(attack_type)
    {
      case 21: /* Slay Evil Beings */
        if (IS_EVIL(victim))
        {
          dam += number(1, 5);
        }
        else
        if (IS_GOOD(victim))
        {
          dam -= number(1, 5);
        }
        break;

      case 22: /* Slay Neutral Beings */
        if (IS_NEUTRAL(victim))
        {
          dam += number(1, 5);
        }
        break;

      case 23: /* Slay Good Beings */
        if (IS_GOOD(victim))
        {
          dam += number(1, 5);
        }
        else
        if (IS_EVIL(victim))
        {
          dam -= number(1, 5);
        }
        break;

      case 30: /* ChAoTiC Weapon */
        dam += (10 - number(0, 20));
        break;

      case CLASS_GREATER_UNDEAD:
        if (victim_class == CLASS_LESSER_UNDEAD)
        {
          dam += number(1, 5);
        }
        break;

      case CLASS_GREATER_VAMPIRE:
        if (victim_class == CLASS_LESSER_VAMPIRE)
        {
          dam += number(1, 5);
        }
        break;

      case CLASS_GREATER_DRAGON:
        if (victim_class == CLASS_LESSER_DRAGON)
        {
          dam += number(1, 5);
        }
        break;

      case CLASS_GREATER_GIANT:
        if (victim_class == CLASS_LESSER_GIANT)
        {
          dam += number(1, 5);
        }
        break;

      case CLASS_GREATER_LYCANTHROPE:
        if (victim_class == CLASS_LESSER_LYCANTHROPE)
        {
          dam += number(1, 5);
        }
        break;

      case CLASS_GREATER_DEMON:
        if (victim_class == CLASS_LESSER_DEMON)
        {
          dam += number(1, 5);
        }
        break;

      case CLASS_GREATER_ELEMENTAL:
        if (victim_class == CLASS_LESSER_ELEMENTAL)
        {
          dam += number(1, 5);
        }
        break;

      case CLASS_GREATER_PLANAR:
        if (victim_class == CLASS_LESSER_PLANAR)
        {
          dam += number(1, 5);
        }
        break;

      default:
        if (victim_class <= 50 &&
            GET_CLASS(victim) == (attack_type - 30))
        {
          dam += number(1, 5);
        }
        else if (attack_type > 300 &&
                 attack_type < 312 &&
                 GET_CLASS(ch) == (attack_type - 300))
        {
          dam += number(1, 5);
        }
        break;
    }
  }

  return dam;
}

int trophy_bonus(CHAR *ch)
{
  OBJ *tmp_obj = NULL;
  int bonus = 0;

  for (tmp_obj = ch->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
  {
    if (tmp_obj->obj_flags.type_flag == ITEM_TROPHY)
    {
      bonus++;
    }
  }

  bonus = MIN(bonus, 2);

  return bonus;
}

int close_combat_bonus(CHAR *ch, int type)
{
  int bonus = 0;

  if (!ch->skills || !check_subclass(ch, SC_BANDIT, 4)) return 0;

  switch (type)
  {
    case 0:
      if (IS_INDOORS(ch) ||
          world[CHAR_REAL_ROOM(ch)].sector_type == SECT_INSIDE ||
          world[CHAR_REAL_ROOM(ch)].sector_type == SECT_CITY ||
          world[CHAR_REAL_ROOM(ch)].sector_type == SECT_FOREST)
      {
        bonus = 5;
      }
      else
      {
        bonus = 1;
      }
      break;
    case 1:
      bonus = -20;
      break;
    default:
      bonus = 0;
      break;
  }

  return bonus;
}

int calc_hitroll(CHAR *ch)
{
  int value = 0;
  int str_bonus = 0;
  OBJ* wielded = NULL;
  AFF *aff = NULL;

  str_bonus = str_app[MAX(0,MIN(STRENGTH_APPLY_INDEX(ch), OSTRENGTH_APPLY_INDEX(ch)))].tohit;

  if ((wielded = EQ(ch, WIELD)) && OBJ_TYPE(wielded) == ITEM_2HWEAPON)
  {
    str_bonus = ((str_bonus * 3) / 2);
  }

  value += GET_HITROLL(ch);
  value += str_bonus;
  value += close_combat_bonus(ch, 0);
  value += trophy_bonus(ch);

  if (affected_by_spell(ch, SKILL_FRENZY))
  {
    value -= 10;
  }

  /* Combat Zen */
  if (!IS_NPC(ch) && check_subclass(ch, SC_RONIN, 1)) {
    for (aff = ch->affected; aff; aff = aff->next)
    {
      if (aff->type == SPELL_BLINDNESS && aff->location == APPLY_HITROLL)
      {
        value += abs(aff->modifier);
      }
    }
  }

  return value;
}

int calc_damroll(CHAR *ch)
{
  int value = 0;
  int str_bonus = 0;
  OBJ* wielded = NULL;

  str_bonus = str_app[MAX(0,MIN(STRENGTH_APPLY_INDEX(ch), OSTRENGTH_APPLY_INDEX(ch)))].todam;

  if ((wielded = EQ(ch, WIELD)) && OBJ_TYPE(wielded) == ITEM_2HWEAPON)
  {
    str_bonus = (str_bonus * 3) / 2;
  }

  value += GET_DAMROLL(ch);
  value += str_bonus;
  value += close_combat_bonus(ch, 0);
  value += trophy_bonus(ch);

  return value;
}

int compute_thaco(CHAR *ch)
{
  int ch_thaco = 0;

  if (!IS_NPC(ch))
  {
    ch_thaco  = GET_THACO(ch);
  }
  else
  {
    ch_thaco = 20; // THAC0 for NPCs is set by adjusting their Hitroll.
  }

  ch_thaco -= calc_hitroll(ch);
  ch_thaco -= close_combat_bonus(ch, 0);

  return ch_thaco;
}

int compute_ac(CHAR *ch)
{
  int ch_ac = 0;

  ch_ac = (GET_AC(ch));

  if (AWAKE(ch)) {
    ch_ac += (dex_app[GET_DEX(ch)].defensive); // adds negative value
  }

  ch_ac += close_combat_bonus(ch, 1); // adds negative value

  if (affected_by_spell(ch, SPELL_BLUR)) {
    ch_ac -= (GET_LEVEL(ch) / 2);
  }

  if (affected_by_spell(ch, SKILL_VEHEMENCE)) {
    ch_ac += 30; /* 30 AC Penalty */
  }

  if (!IS_NPC(ch)) {
    if (affected_by_spell(ch, SKILL_DEFEND) &&
        !affected_by_spell(ch, SKILL_BERSERK)) {
      ch_ac = MAX(-300, ch_ac);
    }
    else {
      ch_ac = MAX(-250, ch_ac);
    }
  }

  return ch_ac;
}

int get_attack_type(CHAR *ch, OBJ *weapon)
{
  int attack_type = 0;

  if (!ch) return TYPE_HIT;

  if (!weapon)
  {
    if (IS_NPC(ch) && ch->specials.attack_type >= TYPE_HIT)
    {
      attack_type = ch->specials.attack_type;
    }
    else
    {
      attack_type = TYPE_HIT;
    }
  }
  else
  {
    switch (OBJ_VALUE3(weapon))
    {
      case 0:
      case 1:
      case 2:
        attack_type = TYPE_WHIP;
      break;

      case 3:
        attack_type = TYPE_SLASH;
      break;

      case 4:
        attack_type = TYPE_WHIP;
      break;

      case 5:
        attack_type = TYPE_STING;
      break;

      case 6:
        attack_type = TYPE_CRUSH;
      break;

      case 7:
        attack_type = TYPE_BLUDGEON;
      break;

      case 8:
        attack_type = TYPE_CLAW;
      break;

      case 9:
      case 10:
      case 11:
        attack_type = TYPE_PIERCE;
      break;

      case 12:
        attack_type = TYPE_HACK;
      break;

      case 13:
        attack_type = TYPE_CHOP;
      break;

      case 14:
        attack_type = TYPE_SLICE;
      break;

      default:
        attack_type = TYPE_HIT;
      break;
    }
  }

  return attack_type;
}

int stack_position(CHAR *ch, int target_position)
{
  int position = 0;

  if (!ch) return POSITION_DEAD; // Assume dead if there is no character.

  if (target_position <= POSITION_INCAP ||
      target_position > POSITION_SITTING ||
      GET_POS(ch) <= POSITION_INCAP ||
      GET_POS(ch) > POSITION_SITTING)
  {
    position = MIN(target_position, GET_POS(ch)); // Don't stack position if better than sitting or incapacitated (or worse).
  }
  else
  {
    if (GET_POS(ch) == POSITION_STUNNED)
    {
      position = POSITION_INCAP;
    }
    else if (GET_POS(ch) == POSITION_RESTING || GET_POS(ch) == POSITION_SITTING)
    {
      if (target_position == POSITION_RESTING || target_position == POSITION_SITTING)
      {
        position = POSITION_STUNNED;
      }
      else
      {
        position = POSITION_INCAP;
      }
    }
    else
    {
      position = MIN(target_position, GET_POS(ch));
    }
  }

  return position;
}

int calc_position_damage(int position, int damage)
{
  int dam = 0;

  if (position < POSITION_FIGHTING)
  {
    switch (position)
    {
      /* x 1.33 */
      case POSITION_SITTING:
        dam = (damage * 4) / 3;
      break;

      /* x 1.50 */
      case POSITION_RESTING:
        dam = (damage * 3) / 2;
      break;

      case POSITION_SLEEPING:
      /* x 1.66 */
        dam = (damage * 5) / 3;
      break;

      /* x 2.00 */
      case POSITION_STUNNED:
        dam = (damage * 2) / 1;
      break;

      /* x 2.33 */
      case POSITION_INCAP:
        dam = (damage * 7) / 3;
      break;

      /* x 2.50 */
      case POSITION_MORTALLYW:
        dam = (damage * 5) / 2;
      break;
    }
  }
  else
  {
    dam = damage;
  }

  return dam;
}

int calc_hit_damage(CHAR *ch, CHAR *victim, OBJ *weapon)
{
  int dam = 0;

  if (!weapon)
  {
    if (IS_NPC(ch))
    {
      dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
    }
    else
    {
      if (GET_CLASS(ch) == CLASS_NINJA)
      {
        /* Combat Zen */
        if (check_subclass(ch, SC_RONIN, 1))
          dam += dice(4, 9);
        else if (GET_LEVEL(ch) > 27)
          dam += dice(5, 4);
        else if (GET_LEVEL(ch) > 24)
          dam += dice(6, 3);
        else if (GET_LEVEL(ch) > 19)
          dam += dice(3, 6);
        else if (GET_LEVEL(ch) > 15)
          dam += dice(4, 4);
        else if (GET_LEVEL(ch) > 10)
          dam += dice(3, 4);
        else if (GET_LEVEL(ch) > 5)
          dam += dice(2, 6);
        else if (GET_LEVEL(ch) > 2)
          dam += dice(2, 5);
        else
          dam += dice(1, 8);
      }
      else
      {
        dam += number(1, 2);
      }
    }
  }
  else
  {
    dam += dice(OBJ_VALUE1(weapon), OBJ_VALUE2(weapon));
    dam += wpn_extra(ch, victim, weapon);
  }

  dam += calc_damroll(ch);
  dam += close_combat_bonus(ch, 0);

  dam = calc_position_damage(GET_POS(victim), dam);

  /* Didn't miss, so not less than 1 damage. */
  dam = MAX(1, dam);

  if (affected_by_spell(ch, SKILL_DEFEND) &&
      !affected_by_spell(ch, SKILL_BERSERK))
  {
    dam = 1;
  }

  if (affected_by_spell(ch, SPELL_CLARITY))
  {
    dam = 0;
  }

  return dam;
}

bool try_hit(CHAR *ch, CHAR *victim)
{
  int check = 0;

  if (!ch || !victim) return FALSE;

  if (!AWAKE(victim) ||
      IS_AFFECTED(victim, AFF_FURY) ||
      affected_by_spell(victim, SKILL_HOSTILE) ||
      (affected_by_spell(victim, SPELL_RAGE) && chance(50)))
  {
    return TRUE;
  }

  check = number(1, 20);

  if (check == 1) return FALSE; // 1 always results in a miss.
  else if (check == 20) return TRUE; // 20 always results in a hit.

  if (compute_thaco(ch) - check > (compute_ac(victim) / 10)) return FALSE;

  return TRUE;
}

bool perform_hit(CHAR *ch, CHAR *victim, int type, int hit_num)
{
  OBJ *weapon = NULL;
  OBJ *disarm = NULL;
  ENCH ench;

  int check = 0;
  int attack_type = 0;
  int dam = 0;
  int multi = 0;
  int reflect = 0;
  bool riposte = FALSE;
  bool special_message = FALSE;

  if (IS_NPC(victim) &&
      GET_POS(victim) >= POSITION_STUNNED &&
      type != SKILL_BACKSTAB &&
      type != SKILL_ASSASSINATE &&
      !affected_by_spell(victim, SKILL_HOSTILE) &&
      !affected_by_spell(victim, SKILL_FRENZY))
  {
    if (IS_AFFECTED(victim, AFF_DODGE) &&
        chance(20))
    {
      switch (number(0, 3))
      {
        case 1:
          act("$n rolls under $N's attack!", FALSE, victim, 0, ch, TO_NOTVICT);
          act("$n rolls under your attack!", FALSE, victim, 0, ch, TO_VICT);
          act("You roll under $N's attack!", FALSE, victim, 0, ch, TO_CHAR);
        break;

        case 2:
          act("$n ducks under $N's mighty blow!", FALSE, victim, 0, ch, TO_NOTVICT);
          act("$n ducks under your mighty blow!", FALSE, victim, 0, ch, TO_VICT);
          act("You duck under $N's mighty blow!", FALSE, victim, 0, ch, TO_CHAR);
        break;

        case 3:
          act("$n sidesteps $N's attack!", FALSE, victim, 0, ch, TO_NOTVICT);
          act("$n sidesteps your attack!", FALSE, victim, 0, ch, TO_VICT);
          act("You sidestep $N's attack!", FALSE, victim, 0, ch, TO_CHAR);
        break;

        default:
          act("$n dodges $N's attack!", FALSE, victim, 0, ch, TO_NOTVICT);
          act("$n dodges your attack!", FALSE, victim, 0, ch, TO_VICT);
          act("You dodge $N's attack!", FALSE, victim, 0, ch, TO_CHAR);
        break;
      }

      if (!GET_OPPONENT(victim))
      {
        set_fighting(victim, ch);
        GET_POS(victim) = POSITION_FIGHTING;
      }

      /* Combat Zen */
      if (!IS_NPC(ch) && check_subclass(ch, SC_RONIN, 1)) return TRUE;

      return FALSE;
    }
  }
  else if (hit_num == 1 &&
           !IS_NPC(victim) &&
           GET_POS(victim) > POSITION_STUNNED &&
           type != SKILL_BACKSTAB &&
           type != SKILL_ASSASSINATE &&
           !IS_AFFECTED(victim, AFF_FURY) &&
           !affected_by_spell(victim, SKILL_HOSTILE) &&
           !affected_by_spell(victim, SKILL_FRENZY) &&
           (!affected_by_spell(victim, SPELL_RAGE) || chance(50)))
  {
    if (GET_CLASS(victim) == CLASS_THIEF ||
        GET_CLASS(victim) == CLASS_NINJA ||
        GET_CLASS(victim) == CLASS_NOMAD ||
        GET_CLASS(victim) == CLASS_BARD ||
        IS_AFFECTED(victim, AFF_DODGE))
    {
      if (IS_AFFECTED(victim, AFF_DODGE) &&
          victim->skills[SKILL_DODGE].learned < 85)
      {
        check = 85;
      }
      else
      {
        check = victim->skills[SKILL_DODGE].learned;
      }

      if (GET_CLASS(victim) == CLASS_THIEF)
      {
        check += GET_LEVEL(victim) / 20;
      }
      else if (GET_CLASS(victim) == CLASS_NOMAD)
      {
        check += GET_LEVEL(victim) / 10;
      }

      if (affected_by_spell(victim, SKILL_DEFEND) &&
          !affected_by_spell(victim, SKILL_BERSERK))
      {
        check += 50;
      }

      if (affected_by_spell(victim, SKILL_FADE))
      {
        check += GET_LEVEL(victim) * 1.5;
      }

      if (affected_by_spell(victim, SPELL_BLUR))
      {
        check += GET_LEVEL(victim);
      }

      if (affected_by_spell(ch, SKILL_VEHEMENCE))
      {
        check -= 70; /* 10% Penalty */
      }

      if (number(1, 700) - (GET_DEX_APP(victim) * 5) <= check)
      {
        switch (number(0, 3))
        {
          case 1:
            act("$n rolls under $N's attack!", FALSE, victim, 0, ch, TO_NOTVICT);
            act("$n rolls under your attack!", FALSE, victim, 0, ch, TO_VICT);
            act("You roll under $N's attack!", FALSE, victim, 0, ch, TO_CHAR);
          break;

          case 2:
            act("$n ducks under $N's mighty blow!", FALSE, victim, 0, ch, TO_NOTVICT);
            act("$n ducks under your mighty blow!", FALSE, victim, 0, ch, TO_VICT);
            act("You duck under $N's mighty blow!", FALSE, victim, 0, ch, TO_CHAR);
          break;

          case 3:
            act("$n sidesteps $N's attack!", FALSE, victim, 0, ch, TO_NOTVICT);
            act("$n sidesteps your attack!", FALSE, victim, 0, ch, TO_VICT);
            act("You sidestep $N's attack!", FALSE, victim, 0, ch, TO_CHAR);
          break;

          default:
            act("$n dodges $N's attack!", FALSE, victim, 0, ch, TO_NOTVICT);
            act("$n dodges your attack!", FALSE, victim, 0, ch, TO_VICT);
            act("You dodge $N's attack!", FALSE, victim, 0, ch, TO_CHAR);
          break;
        }

        if (!GET_OPPONENT(victim))
        {
          set_fighting(victim, ch);
          GET_POS(victim) = POSITION_FIGHTING;
        }

        /* Combat Zen */
        if (!IS_NPC(ch) && check_subclass(ch, SC_RONIN, 1)) return TRUE;

        return FALSE;
      }
    }
    else if (GET_CLASS(victim) == CLASS_WARRIOR ||
             GET_CLASS(victim) == CLASS_PALADIN ||
             GET_CLASS(victim) == CLASS_AVATAR)
    {
      check = victim->skills[SKILL_PARRY].learned;

      if (GET_CLASS(victim) == CLASS_WARRIOR)
      {
        check += GET_LEVEL(victim) / 10;
      }

      if (affected_by_spell(victim, SKILL_DEFEND))
      {
        check += 50;
      }

      if (number(1, 700) - (dex_app[GET_DEX(victim)].prac_bonus * 5) <= check)
      {
        act("$n parries $N's attack!", FALSE, victim, 0, ch, TO_NOTVICT);
        act("$n parries your attack!", FALSE, victim, 0, ch, TO_VICT);
        act("You parry $N's attack!", FALSE, victim, 0, ch, TO_CHAR);

        if (!GET_OPPONENT(victim))
        {
          set_fighting(victim, ch);
          GET_POS(victim) = POSITION_FIGHTING;
        }

        /* Combat Zen */
        if (!IS_NPC(ch) && check_subclass(ch, SC_RONIN, 1)) return TRUE;

        return FALSE;
      }
    }
    else if (check_sc_access(victim, SKILL_FEINT))
    {
      check = victim->skills[SKILL_FEINT].learned;

      if (number(1, 850) - (dex_app[GET_DEX(victim)].prac_bonus * 5) <= check)
      {
        if (affected_by_spell(victim, SPELL_BLACKMANTLE))
        {
          reflect = GET_LEVEL(victim) / 5;

          if (IS_AFFECTED(victim, AFF_FURY))
          {
            reflect *= 2;
          }
          else if (affected_by_spell(victim, SPELL_RAGE))
          {
            reflect = (reflect * 3) / 2;
          }

          act("$n is scorched by your mantle of darkness as $e gets too close.", FALSE, ch, 0, victim, TO_VICT);
          act("$n is scorched by $N's mantle of darkness as $e gets too close.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("You are scorched by $N's mantle of darkness as you get too close!", FALSE, ch, 0, victim, TO_CHAR);

          damage(victim, ch, reflect, TYPE_UNDEFINED, DAM_MAGICAL);
        }

        act("$n feints, preventing $N's attack. $n hits back!", FALSE, victim, 0, ch, TO_NOTVICT);
        act("$n feints, preventing your attack. $n hits back!", FALSE, victim, 0, ch, TO_VICT);
        act("You feint, preventing $N's attack. You hit back!", FALSE, victim, 0, ch, TO_CHAR);

        hit(victim, ch, SKILL_FEINT);

        /* Combat Zen */
        if (!IS_NPC(ch) && check_subclass(ch, SC_RONIN, 1)) return TRUE;

        return FALSE;
      }
    }
    else if (check_sc_access(victim, SKILL_RIPOSTE))
    {
      check = victim->skills[SKILL_RIPOSTE].learned;

      if (number(1, 850) - (dex_app[GET_DEX(victim)].prac_bonus * 5) <= check)
      {
        act("$n deflects $N's attack. $n hits back!", FALSE, victim, 0, ch, TO_NOTVICT);
        act("$n deflects your attack. $n hits back!", FALSE, victim, 0, ch, TO_VICT);
        act("You deflect $N's attack. You hit back!", FALSE, victim, 0, ch, TO_CHAR);

        hit(victim, ch, TYPE_UNDEFINED);

        if (CHAR_REAL_ROOM(victim) == NOWHERE ||
            CHAR_REAL_ROOM(ch) == NOWHERE)
        {
          return FALSE;
        }
        else
        {
          riposte = TRUE;
        }
      }
    }
  }

  if (EQ(ch, WIELD) && IS_WEAPON(EQ(ch, WIELD)))
  {
    weapon = EQ(ch, WIELD);
  }

  if (hit_num == 2 && !IS_NPC(ch) && GET_CLASS(ch) == CLASS_NINJA)
  {
    if (EQ(ch, HOLD) && IS_WEAPON(EQ(ch, HOLD)))
    {
      weapon = EQ(ch, HOLD);
    }
    else
    {
      weapon = NULL;
    }
  }

  if (type == SPELL_SHADOW_WRAITH)
  {
    attack_type = TYPE_SHADOW;
  }
  else
  {
    attack_type = get_attack_type(ch, weapon);
  }

  if (!try_hit(ch, victim))
  {
    switch (type)
    {
      case SKILL_ASSASSINATE:
      case SKILL_BACKSTAB:
        damage(ch, victim, 0, SKILL_BACKSTAB, DAM_NO_BLOCK);
      break;

      case SKILL_CIRCLE:
        act("$n missteps as $e lunges toward $N, missing $S back completely.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("You see $n misstep out of the corner of your eye, stumbling past your flank.", FALSE, ch, 0, victim, TO_VICT);
        act("You misstep while lunging toward $N, missing $S back completely.", FALSE, ch, 0, victim, TO_CHAR);

        damage(ch, victim, 0, SKILL_CIRCLE, DAM_NO_BLOCK);
      break;

      case SKILL_AMBUSH:
        act("$n tries to ambush $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n tries to ambush you, but misses.", FALSE, ch, 0, victim, TO_VICT);
        act("You try to ambush $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);

        damage(ch, victim, 0, SKILL_AMBUSH, DAM_NO_BLOCK);
      break;

      case SKILL_FLANK:
        act("$n tries to flank $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n tries to flank you, but misses.", FALSE, ch, 0, victim, TO_VICT);
        act("You try to flank $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);

        damage(ch, victim, 0, SKILL_FLANK, DAM_NO_BLOCK);
      break;

      case SKILL_CHARGE:
        act("$n tries to charge $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n tries to charge you, but misses.", FALSE, ch, 0, victim, TO_VICT);
        act("You try to charge $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);

        damage(ch, victim, 0, SKILL_CHARGE, DAM_NO_BLOCK);
      break;

      case SKILL_ASSAULT:
        act("$n tries to assault $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n tries to assault you, but misses.", FALSE, ch, 0, victim, TO_VICT);
        act("You try to assault $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);

        damage(ch, victim, 0, SKILL_ASSAULT, DAM_NO_BLOCK);
      break;

      case SKILL_BLITZ:
        act("$n tries to blitz $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n tries to blitz you, but misses.", FALSE, ch, 0, victim, TO_VICT);
        act("You try to blitz $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);

        damage(ch, victim, 0, SKILL_BLITZ, DAM_NO_BLOCK);
      break;

      case SKILL_LUNGE:
        act("$n tries to lunge at $N, but misses.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n tries to lunge at you, but misses.", FALSE, ch, 0, victim, TO_VICT);
        act("You try to lunge at $N, but miss.", FALSE, ch, 0, victim, TO_CHAR);

        damage(ch, victim, 0, SKILL_LUNGE, DAM_NO_BLOCK);
      break;

      default:
        damage(ch, victim, 0, attack_type, DAM_NO_BLOCK);
      break;
    }

    /* Combat Zen */
    if (!IS_NPC(ch) && check_subclass(ch, SC_RONIN, 1)) return TRUE;

    return FALSE;
  }
  else
  {
    dam = calc_hit_damage(ch, victim, weapon);

    switch (type)
    {
      case SKILL_BACKSTAB:
      case SKILL_ASSASSINATE:
        if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
        {
          damage(ch, victim, 0, SKILL_BACKSTAB, DAM_NO_BLOCK);
        }
        else
        {
          /* Impair */
          if (CHAR_REAL_ROOM(victim) != NOWHERE &&
              !IS_NPC(ch) &&
              check_sc_access(ch, SKILL_IMPAIR) &&
              chance(40 + GET_DEX_APP(ch)))
          {
            ench.name = strdup("Impaired");
            ench.type = SKILL_IMPAIR;
            if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)))
              ench.duration = 0;
            else
              ench.duration = 1;
            ench.location = APPLY_NONE;
            ench.modifier = 0;
            ench.bitvector = AFF_PARALYSIS;
            ench.bitvector2 = AFF_NONE;
            ench.func = impair_enchantment;

            enchantment_to_char(victim, &ench, FALSE);

            special_message = TRUE;
          }

          multi = backstab_mult[GET_LEVEL(ch)];
          dam *= MAX(1, multi);
          damage(ch, victim, dam, SKILL_BACKSTAB, DAM_PHYSICAL);

          if (special_message) {
            act("You nearly sever $N's spine with your backstab, temporarily paralyzing $M.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n nearly severs your spine with $s backstab, temporarily paralyzing you.", FALSE, ch, 0, victim, TO_VICT);
            act("$n nearly severs $N's spine with $s backstab, temporarily paralyzing $M.", FALSE, ch, 0, victim, TO_NOTVICT);

            special_message = FALSE;
          }
        }
        break;

      case SKILL_CIRCLE:
        if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
        {
          act("$n's weapon makes contact with $N, but slides harmlessly off $S back.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$n's weapon slides harmlessly off of your back.", FALSE, ch, 0, victim, TO_VICT);
          act("Your weapon makes contact with $N, but slides harmlessly off $S back.", FALSE, ch, 0, victim, TO_CHAR);

          damage(ch, victim, 0, SKILL_CIRCLE, DAM_NO_BLOCK);
        }
        else
        {
          /* Impair */
          if (CHAR_REAL_ROOM(victim) != NOWHERE &&
              CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(ch) &&
              !IS_NPC(ch) &&
              check_sc_access(ch, SKILL_IMPAIR) &&
              chance(20 + (GET_DEX_APP(ch) / 2)))
          {
            ench.name = strdup("Dazed (Armor Penalty)");
            ench.type = SKILL_IMPAIR;
            ench.duration = 0;
            ench.location = APPLY_ARMOR;
            ench.modifier = 30;
            ench.bitvector = AFF_NONE;
            ench.bitvector2 = AFF_NONE;
            ench.func = impair_enchantment;
            enchantment_to_char(victim, &ench, FALSE);

            ench.name = strdup("Dazed (Hitroll Penalty)");
            ench.location = APPLY_HITROLL;
            ench.modifier = -2;
            enchantment_to_char(victim, &ench, FALSE);

            special_message = TRUE;
          }

          act("$n plunges $p deep into $N's back.", FALSE, ch, weapon, victim, TO_NOTVICT);
          act("$n plunges $p deep into your back.", FALSE, ch, weapon, victim, TO_VICT);
          act("You plunge $p deep into $N's back.", FALSE, ch, weapon, victim, TO_CHAR);

          dam *= circle_mult[GET_LEVEL(ch)];
          damage(ch, victim, dam, SKILL_CIRCLE, DAM_PHYSICAL);

          if (special_message) {
            act("You strike a nerve in $N's back with your attack, severely weakening $M.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n strikes a nerve in your back with $s attack, severely weakening you.", FALSE, ch, 0, victim, TO_VICT);
            act("$n strikes a nerve in $N's back with $s attack, severely weakening $M.", FALSE, ch, 0, victim, TO_NOTVICT);

            special_message = FALSE;
          }

          /* Twist */
          if (CHAR_REAL_ROOM(victim) != NOWHERE &&
              CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(ch) &&
              GET_CLASS(ch) == CLASS_THIEF &&
              GET_LEVEL(ch) >= 45 &&
              GET_LEARNED(ch, SKILL_TWIST) > number(1, 131) - GET_DEX_APP(ch) - (affected_by_spell(ch, SKILL_VEHEMENCE) ? (5 + (GET_DEX_APP(ch) / 2)) : 0)) {
            act("You twist your weapon in the flesh of $N.", FALSE, ch, 0, victim, TO_CHAR);
            act("You writhe in pain as $n twists his weapon in your back.", FALSE, ch, 0, victim, TO_VICT);
            act("$n gruesomely twists $s weapon in the flesh of $N.", TRUE, ch, 0, victim, TO_NOTVICT);

            damage(ch, victim, 250, SKILL_TWIST, DAM_PHYSICAL);
          }
        }
        break;

      case SKILL_AMBUSH:
        if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
        {
          act("$n tries to ambush $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$n tries to ambush you, but fails.", FALSE, ch, 0, victim, TO_VICT);
          act("You try to ambush $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);

          damage(ch, victim, 0, SKILL_AMBUSH, DAM_NO_BLOCK);
        }
        else
        {
          act("$N fell into a cunning ambush by $n.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("You fell into an ambush by $n.", FALSE, ch, 0, victim, TO_VICT);
          act("You managed to take $N completely by surprise.", FALSE, ch, 0, victim, TO_CHAR);

          multi = ambush_mult[GET_LEVEL(ch)];

          if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_FOREST ||
              world[CHAR_REAL_ROOM(ch)].sector_type == SECT_MOUNTAIN)
          {
            multi += 1;
          }
          else if (world[CHAR_REAL_ROOM(ch)].sector_type == SECT_INSIDE ||
                   world[CHAR_REAL_ROOM(ch)].sector_type == SECT_CITY)
          {
            multi -= 1;
          }

          dam *= MAX(1, multi);
          damage(ch, victim, dam, SKILL_AMBUSH, DAM_PHYSICAL);
        }
        break;

      case SKILL_FLANK:
        if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
        {
          act("$n tries to flank $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$n tries to flank you, but fails.", FALSE, ch, 0, victim, TO_VICT);
          act("You try to flank $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);

          damage(ch, victim, 0, SKILL_FLANK, DAM_NO_BLOCK);
        }
        else
        {
          act("$n quickly moves to $N's side and hits $M with a devastating blow.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$n quickly moves to your side and hits you with a devastating blow.", FALSE, ch, 0, victim, TO_VICT);
          act("You quickly move to $N's side and hits $M with a devastating blow.", FALSE, ch, 0, victim, TO_CHAR);

          dam *= GET_LEVEL(ch) / 10;
          damage(ch, victim, dam, SKILL_FLANK, DAM_PHYSICAL);
        }
        break;

      case SKILL_CHARGE:
        if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
        {
          act("$n tries to charge $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$n tries to charge you, but fails.", FALSE, ch, 0, victim, TO_VICT);
          act("You try to charge $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);

          damage(ch, victim, 0, SKILL_CHARGE, DAM_NO_BLOCK);
        }
        else
        {
          act("With a bloodthirsty scream, $n charges $N.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("With a bloodthirsty scream, $n charges you.", FALSE, ch, 0, victim, TO_VICT);
          act("With a bloodthirsty scream, you charge $N.", FALSE, ch, 0, victim, TO_CHAR);

          dam *= 2;
          damage(ch, victim, dam, SKILL_CHARGE, DAM_PHYSICAL);
        }
        break;

      case SKILL_ASSAULT:
        if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
        {
          act("$n tries to assault $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$n tries to assault you, but fails.", FALSE, ch, 0, victim, TO_VICT);
          act("You try to assault $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);

          damage(ch, victim, 0, SKILL_ASSAULT, DAM_NO_BLOCK);
        }
        else
        {
          act("$n attacked $N suddenly without $M noticing.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("You are attacked suddenly by $n.", FALSE, ch, 0, victim, TO_VICT);
          act("You attacked $N suddenly without $M noticing.", FALSE, ch, 0, victim, TO_CHAR);

          multi = assault_mult[GET_LEVEL(ch)];

          /* Dual Assault */
          if (check_subclass(ch, SC_RONIN, 3) && hit_num == 4)
          {
            multi -= 3;
          }

          dam *= MAX(1, multi);

          if ((GET_LEVEL(ch) >= 35 && GET_LEVEL(ch) < 40) || (GET_LEVEL(ch) >= 45 && GET_LEVEL(ch) < 50))
          {
            dam += dam / 2;
          }

          damage(ch, victim, dam, SKILL_ASSAULT, DAM_PHYSICAL);
        }
        break;

      case SKILL_BLITZ:
        if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
        {
          act("You try to blitz $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);
          act("$N tries to blitz you, but fails.", FALSE, victim, 0, ch, TO_CHAR);
          act("$n tries to blitz $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);

          damage(ch, victim, 0, SKILL_BLITZ, DAM_NO_BLOCK);
        }
        else
        {
          act("$n charges into the fray, wildly swinging $s weapon at $N.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$n charges into the fray, wildly swinging $s weapon at you.", FALSE, ch, 0, victim, TO_VICT);
          act("You charge into the fray, wildly swinging your weapon at $N.", FALSE, ch, 0, victim, TO_CHAR);

          if (GET_WEAPON(victim) && chance(20))
          {
            disarm = unequip_char(victim, WIELD);

            act("$n's $p is knocked from $s grasp.", FALSE, victim, disarm, 0, TO_ROOM);
            act("Your $p is knocked from your grasp.", FALSE, victim, disarm, 0, TO_CHAR);

            if (IS_SET(CHAR_ROOM_FLAGS(ch), CHAOTIC))
            {
              obj_to_char(disarm, victim);
            }
            else
            {
              log_f("WIZINFO: %s disarms %s's %s (Room %d)",
                    GET_NAME(ch), GET_NAME(victim), OBJ_NAME(disarm), V_ROOM(victim));
              obj_to_room(disarm, CHAR_REAL_ROOM(victim));
              OBJ_LOG(disarm) = TRUE;
            }

            save_char(victim, NOWHERE);
          }

          dam = (dam * 3) / 2;
          damage(ch, victim, dam, SKILL_BLITZ, DAM_PHYSICAL);
        }
        break;

      case SKILL_LUNGE:
        if (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL))
        {
          act("$n tries to lunge at $N, but fails.", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$n tries to lunge at you, but fails.", FALSE, ch, 0, victim, TO_VICT);
          act("You try to lunge at $N, but fail.", FALSE, ch, 0, victim, TO_CHAR);

          damage(ch, victim, 0, SKILL_LUNGE, DAM_NO_BLOCK);
        }
        else
        {
          act("$n lunges forward with $s weapon, impacting $N's hide!", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$n lunges forward with $s weapon, impacting your hide!", FALSE, ch, 0, victim, TO_VICT);
          act("You lunge forward with your weapon, impacting $N's hide!", FALSE, ch, 0, victim, TO_CHAR);

          dam *= (assault_mult[GET_LEVEL(ch)] * 5) / 4;
          damage(ch, victim, dam, SKILL_LUNGE, DAM_PHYSICAL);
        }
        break;

      case SKILL_FEINT:
        /* Because defiler no longer has shadows, Feint results in a doubled return attack */
        dam *= 2;
        damage(ch, victim, dam, attack_type, DAM_PHYSICAL);
        break;

      default:
        if (riposte)
        {
          dam = (dam * 8) / 10;
        }

        damage(ch, victim, dam, attack_type, DAM_PHYSICAL);
        break;
    }

    return TRUE;
  }

  return FALSE;
}

void hit(CHAR *ch, CHAR *victim, int type)
{
  char buf[MSL];

  int check = 0;

  if (!ch || !victim) return;
  if (CHAR_REAL_ROOM(ch) != CHAR_REAL_ROOM(victim)) return;

  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, SAFE) &&
      !CHAOSMODE)
  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  if (!IS_NPC(ch) &&
      IS_SET(GET_PFLAG(ch), PLR_NOKILL) &&
      IS_NPC(victim) &&
      IS_SET(GET_ACT(victim), ACT_MOUNT) &&
      !IS_SET(CHAR_ROOM_FLAGS(ch), ARENA) &&
      !IS_SET(CHAR_ROOM_FLAGS(ch), CHAOTIC) &&
      !CHAOSMODE)
  {
    send_to_char("You can't attack mounts.\n\r", ch);
    return;
  }

  if (!IS_NPC(ch) &&
      !IS_NPC(victim) &&
      ch != victim &&
      IS_SET(GET_PFLAG(ch), PLR_NOKILL) &&
      (!IS_SET(GET_PFLAG(victim), PLR_KILL) || !IS_SET(GET_PFLAG(victim), PLR_THIEF)) &&
      !IS_SET(CHAR_ROOM_FLAGS(ch), ARENA) &&
      !IS_SET(CHAR_ROOM_FLAGS(ch), CHAOTIC) &&
      !CHAOSMODE)
  {
    send_to_char("You can't attack other players.\n\r", ch);
    return;
  }

  if (!IS_NPC(ch) &&
      !IS_NPC(victim) &&
      ch != victim &&
      !IS_SET(GET_PFLAG(ch), PLR_KILL) &&
      (!IS_SET(GET_PFLAG(victim), PLR_KILL) || !IS_SET(GET_PFLAG(victim), PLR_THIEF)) &&
      !IS_SET(CHAR_ROOM_FLAGS(ch), ARENA) &&
      !IS_SET(CHAR_ROOM_FLAGS(ch), CHAOTIC) &&
      !CHAOSMODE)
  {
    send_to_char("You are a killer!\n\r", ch);

    SET_BIT(GET_PFLAG(ch), PLR_KILL);

    sprintf(buf,"PLRINFO: %s just attacked %s; Killer flag set. (Room %d)",
            GET_NAME(ch), GET_NAME(victim), V_ROOM(ch));
    wizlog(buf, LEVEL_SUP, 4);
    log_f("%s", buf);
  }

  if (GET_OPPONENT(ch) &&
      GET_POS(ch) > POSITION_FIGHTING)
  {
    GET_POS(ch) = POSITION_FIGHTING;
  }

  if (!perform_hit(ch, victim, type, 1)) return;

  if (affected_by_spell(ch, SPELL_SHADOW_WRAITH))
  {
    dhit(ch, victim, SPELL_SHADOW_WRAITH);

    if (CHAR_REAL_ROOM(victim) == NOWHERE || CHAR_REAL_ROOM(ch) == NOWHERE) return;

    if (duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 60)
    {
      dhit(ch, victim, SPELL_SHADOW_WRAITH);

      if (CHAR_REAL_ROOM(victim) == NOWHERE || CHAR_REAL_ROOM(ch) == NOWHERE) return;
    }

    if (duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 40)
    {
      dhit(ch, victim, SPELL_SHADOW_WRAITH);

      if (CHAR_REAL_ROOM(victim) == NOWHERE || CHAR_REAL_ROOM(ch) == NOWHERE) return;
    }

    if (duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 20)
    {
      dhit(ch, victim, SPELL_SHADOW_WRAITH);

      if (CHAR_REAL_ROOM(victim) == NOWHERE || CHAR_REAL_ROOM(ch) == NOWHERE) return;
    }

    return;
  }
  else if (affected_by_spell(ch, SKILL_BERSERK))
  {
    if (chance(40))
    {
      dhit(ch, victim, TYPE_UNDEFINED);
    }

    return;
  }
  else if (affected_by_spell(ch, SKILL_FRENZY))
  {
    if (chance(10))
    {
      dhit(ch, victim, TYPE_UNDEFINED);
    }

    return;
  }
  else if (!IS_NPC(ch) &&
           GET_CLASS(ch) == CLASS_NINJA)
  {
    dhit(ch, victim, TYPE_UNDEFINED);

    return;
  }
  else if (!IS_NPC(ch) &&
           (IS_SET(GET_AFF(ch), AFF_DUAL) ||
            (GET_CLASS(ch) == CLASS_WARRIOR ||
             GET_CLASS(ch) == CLASS_AVATAR ||
             GET_CLASS(ch) == CLASS_COMMANDO)))
  {
    if (!ch->skills) return;

    if (affected_by_spell(ch, SKILL_HOSTILE) ||
        affected_by_spell(ch, SPELL_RUSH))
    {
      check = number(1, 200);
    }
    else
    {
      check = number(1, 370);
    }

    check -= GET_DEX_APP(ch) * 5;

    if (check <= ch->skills[SKILL_DUAL].learned ||
        (IS_AFFECTED(ch, AFF_DUAL) &&
         ch->skills[SKILL_DUAL].learned < 85 &&
         check <= 85))
    {
      dhit(ch, victim, TYPE_UNDEFINED);
    }

    return;
  }
  else if (IS_NPC(ch) &&
           IS_AFFECTED(ch, AFF_DUAL))
  {
    if (!number(0, 2))
    {
      dhit(ch, victim, TYPE_UNDEFINED);
    }

    return;
  }
}

void dhit(CHAR *ch, CHAR *victim, int type)
{
  int check = 0;

  if (!ch || !victim) return;
  if (CHAR_REAL_ROOM(ch) != CHAR_REAL_ROOM(victim)) return;

  if (!perform_hit(ch, victim, type, 2)) return;

  if (!IS_NPC(ch) &&
      (IS_AFFECTED2(ch, AFF_TRIPLE) ||
       ((GET_CLASS(ch) == CLASS_WARRIOR ||
         GET_CLASS(ch) == CLASS_AVATAR ||
         GET_CLASS(ch) == CLASS_COMMANDO) &&
         GET_LEVEL(ch) >= 20)))
  {
    if (!ch->skills) return;

    if (affected_by_spell(ch, SKILL_HOSTILE) ||
        affected_by_spell(ch, SPELL_RUSH))
    {
      check = number(1, 190);
    }
    else if (check_subclass(ch, SC_WARLORD, 1))
    {
      check = number(1, 235);
    }
    else
    {
      check = number(1, 335);
    }

    check -= GET_DEX_APP(ch) * 5;

    if (check <= ch->skills[SKILL_TRIPLE].learned ||
        (IS_AFFECTED2(ch, AFF_TRIPLE) &&
         ch->skills[SKILL_TRIPLE].learned < 85 &&
         check <= 85))
    {
      thit(ch, victim, TYPE_UNDEFINED);
    }

    return;
  }
  else if (!IS_NPC(ch) &&
           GET_CLASS(ch) == CLASS_NINJA &&
           GET_LEVEL(ch) >= 50 &&
           affected_by_spell(ch, SPELL_MYSTIC_SWIFTNESS) &&
           !CHAOSMODE)
  {
    if (number(1, 100) - GET_DEX_APP(ch) <= 50)
    {
      thit(ch, victim, TYPE_UNDEFINED);
    }

    return;
  }
  else if (IS_NPC(ch) &&
           IS_AFFECTED2(ch, AFF_TRIPLE))
  {
    if (!number(0, 2))
    {
      thit(ch, victim, TYPE_UNDEFINED);
    }

    return;
  }
}

void thit(CHAR *ch, CHAR *victim, int type)
{
  int check = 0;

  if (!ch || !victim) return;
  if (CHAR_REAL_ROOM(ch) != CHAR_REAL_ROOM(victim)) return;

  if (!perform_hit(ch, victim, type, 3)) return;

  if (!IS_NPC(ch) &&
      (IS_AFFECTED2(ch, AFF_QUAD) ||
       (GET_CLASS(ch) == CLASS_WARRIOR &&
        GET_LEVEL(ch) >= 50 &&
        !CHAOSMODE)))
  {
    if (!ch->skills) return;

    if (affected_by_spell(ch, SKILL_HOSTILE))
    {
      check = number(1, 180);
    }
    else
    {
      check = number(1, 300);
    }

    check -= GET_DEX_APP(ch) * 5;

    if (check <= ch->skills[SKILL_QUAD].learned ||
        (IS_AFFECTED2(ch, AFF_QUAD) &&
         ch->skills[SKILL_QUAD].learned < 85 &&
         check <= 85))
    {
      qhit(ch, victim, TYPE_UNDEFINED);
    }

    return;
  }
  else if (IS_NPC(ch) &&
           IS_AFFECTED2(ch, AFF_QUAD))
  {
    if (!number(0, 2))
    {
      qhit(ch, victim, TYPE_UNDEFINED);
    }

    return;
  }
}

void qhit(CHAR *ch, CHAR *victim, int type)
{
  if (!ch || !victim) return;
  if (CHAR_REAL_ROOM(ch) != CHAR_REAL_ROOM(victim)) return;

  perform_hit(ch, victim, type, 4);
}

void blood_lust_action(CHAR *ch, CHAR *vict)
{
  int dmg = 0;

  if (!ch || !vict) return;

  switch(number(1, 20))
  {
    case 1:
    case 2:
      act("$n bites viciously at $N with $s fangs!", TRUE, ch, 0, vict, TO_NOTVICT);
      act("$n viciously bites at you with $s fangs!", FALSE, ch, 0, vict, TO_VICT);
      act("You bite at $N viciously with your fangs!", FALSE, ch, 0, vict, TO_CHAR);

      dmg = 60;

      if (affected_by_spell(ch, SPELL_BLACKMANTLE)) dmg *= 1.1;

      damage(ch, vict, dmg, TYPE_UNDEFINED, DAM_PHYSICAL);
      break;

    case 3:
      act("$n sinks $s fangs into $N's neck, draining $S life!", TRUE, ch, 0, vict, TO_NOTVICT);
      act("$n sinks $s fangs into your neck, draining your life!", FALSE, ch, 0, vict, TO_VICT);
      act("You sink your fangs into $N's neck, draining $S life!", FALSE, ch, 0, vict, TO_CHAR);

      dmg = 40;

      if (affected_by_spell(ch, SPELL_BLACKMANTLE)) dmg *= 1.1;

      damage(ch, vict, dmg, TYPE_UNDEFINED, DAM_MAGICAL);
      magic_heal(ch, SPELL_BLOOD_LUST, dmg, TRUE);
      GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - dmg);
      break;

    case 4:
      act("$n bites savagely at $N, draining $S magical essence!", TRUE, ch, 0, vict, TO_NOTVICT);
      act("$n bites at you savagely, draining your magical essence!", FALSE, ch, 0, vict, TO_VICT);
      act("You bite savagely at $N, draining $S magical essence!", FALSE, ch, 0, vict, TO_CHAR);

      dmg = 20;

      if (affected_by_spell(ch, SPELL_BLACKMANTLE)) dmg *= 1.1;

      drain_mana_hit_mv(ch, vict, dmg, 0, 0, TRUE, FALSE, FALSE);
      break;
  }
}


int victimize_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg)
{
  return FALSE;
}


void victimize_action(CHAR *ch, CHAR *vict)
{
  ENCH ench;
  ENCH *tmp_ench = NULL, *next_ench = NULL;
  char buf[MSL];
  int ac_debuff = FALSE, hr_debuff = FALSE, dr_debuff = FALSE;
  int check = 0, debuff = 0;

  if (!ch || !vict) return;

  if (IS_GOOD(ch)) return;

  check = number(1, 170) - GET_DEX_APP(ch);

  if (check > ch->skills[SKILL_VICTIMIZE].learned) return;

  sprintf(buf, "Victimized by %s", GET_NAME(ch));

  for (tmp_ench = vict->enchantments; tmp_ench; tmp_ench = next_ench)
  {
    next_ench = tmp_ench->next;

    if (!str_cmp(tmp_ench->name, buf)) return;

    if (tmp_ench->type == SKILL_VICTIMIZE)
    {
      switch (tmp_ench->location)
      {
        case APPLY_AC:
          ac_debuff = TRUE;
          break;
        case APPLY_HITROLL:
          hr_debuff = TRUE;
          break;
        case APPLY_DAMROLL:
          dr_debuff = TRUE;
          break;
      }
    }
  }

  if (ac_debuff && hr_debuff && dr_debuff) return;

  if (!ac_debuff && !hr_debuff && !dr_debuff) debuff = number(1, 3);
  else if (!ac_debuff && hr_debuff && dr_debuff) debuff = 1;
  else if (!ac_debuff && !hr_debuff && dr_debuff) debuff = number(1, 2);
  else if (ac_debuff && !hr_debuff && dr_debuff) debuff = 2;
  else if (ac_debuff && !hr_debuff && !dr_debuff) debuff = number(2, 3);
  else if (ac_debuff && hr_debuff && !dr_debuff) debuff = 3;
  else if (!ac_debuff && hr_debuff && !dr_debuff)
  {
    if (number(0, 1)) debuff = 1;
    else debuff = 3;
  }
  else return;

  if (debuff == 3 && !GET_DAMROLL(vict) && !hr_debuff) debuff = 2;
  else if (debuff == 3 && !GET_DAMROLL(vict) && hr_debuff) return;

  ench.name = strdup(buf);
  ench.type = SKILL_VICTIMIZE;
  ench.duration = 0;
  ench.location = APPLY_NONE;
  ench.modifier = 0;
  ench.bitvector = 0;
  ench.bitvector2 = 0;
  ench.func = victimize_enchantment;

  switch (debuff)
  {
    case 1:
      ench.location = APPLY_AC;
      ench.modifier = GET_LEVEL(ch) / 4;
      break;
    case 2:
      ench.location = APPLY_HITROLL;
      ench.modifier = -1 * number(3, 5);
      break;
    case 3:
      ench.location = APPLY_DAMROLL;
      ench.modifier = -1 * ((GET_DAMROLL(vict) * number(5, 10)) / 100);
      break;
  }

  if (ench.location)
  {
    act("You victimize $N, inflicting physical and mental torment.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n victimizes you, inflicting physical and mental torment.", FALSE, ch, 0, vict, TO_VICT);
    act("$n victimizes $N, inflicting physical and mental torment.", FALSE, ch, 0, vict, TO_NOTVICT);

    enchantment_to_char(vict, &ench, FALSE);
  }
}


void shadowstep_action(CHAR *ch, CHAR *vict)
{
  int check = 0, dmg = 0;
  double multi = 2.0;

  if (!ch || !vict) return;

  if (!EQ(ch, WIELD)) return;

  check = number(1, 450) - (GET_DEX_APP(ch) * 5);

  if (check > GET_LEARNED(ch, SKILL_SHADOWSTEP)) return;

  if (!IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), DARK) &&
      ((IS_DAY && OUTSIDE(ch)) || IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), LIT)))
  {
    check += 50;
    multi -= 0.5;
  }

  if (IS_NIGHT && IS_EVIL(ch))
  {
    check -= 25;
    multi += 0.5;
  }

  if (IS_SET(GET_ROOM_FLAGS(CHAR_REAL_ROOM(ch)), DARK))
  {
    check -= 25;
    multi += 0.5;
  }

  if (!CAN_SEE(vict, ch) ||
      affected_by_spell(ch, SPELL_IMP_INVISIBLE) ||
      affected_by_spell(ch, SPELL_BLACKMANTLE))
  {
    check -= 25;
    multi += 0.5;
  }

  if (multi < 1.5) multi = 1.5;
  else if (multi > 3.0) multi = 3.0;

  dmg = (int)((double)calc_hit_damage(ch, vict, EQ(ch, WIELD)) * multi);

  act("You step into the shadows and attack $N by surprise!", FALSE, ch, 0, vict, TO_CHAR);
  act("$n steps into the shadows and attacks you by surprise!", FALSE, ch, 0, vict, TO_VICT);
  act("$n steps into the shadows and attacks $N by surprise!", FALSE, ch, 0, vict, TO_NOTVICT);

  damage(ch, vict, dmg, get_attack_type(ch, EQ(ch, WIELD)), DAM_PHYSICAL);
}


int dirty_tricks_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg)
{
  int set_pos = 0;

  if (cmd != MSG_MOBACT) return FALSE;

  act("Blood oozes from your gaping wound.", FALSE, enchanted_ch, 0, 0, TO_CHAR);
  act("Blood oozes from $n's gaping wound.", TRUE, enchanted_ch, 0, 0, TO_ROOM);

  set_pos = GET_POS(enchanted_ch);

  damage(enchanted_ch, enchanted_ch, dice(3, 12), SKILL_DIRTY_TRICKS, DAM_PHYSICAL);

  GET_POS(enchanted_ch) = set_pos;

  return FALSE;
}


void dirty_tricks_action(CHAR *ch, CHAR *victim) {
  AFF af;
  ENCH ench;
  bool can_stab = TRUE;
  bool can_blind = TRUE;
  int trick = 0;
  int set_pos = 0;

  if (!ch || !victim) return;

  if (!GET_WEAPON(ch) || affected_by_spell(victim, SKILL_DIRTY_TRICKS)) {
    can_stab = FALSE;
  }

  if ((IS_NPC(victim) && IS_IMMUNE(victim, IMMUNE_BLINDNESS)) || IS_AFFECTED(victim, AFF_BLIND)) {
    can_blind = FALSE;
  }

  /* The logic here is a bit odd looking, but should be correct. */
  if (!can_stab && can_blind) {
    if (number(1, 100) <= 40) {
      trick = 21;
    }
    else {
      trick = 51;
    }
  }
  else if (can_stab && !can_blind) {
    if (number(1, 100) <= 35) {
      trick = 1;
    }
    else {
      trick = 51;
    }
  }
  else if (!can_stab && !can_blind) {
    trick = 51;
  }
  else {
    trick = number(1, 100);
  }

  /* 20% Chance Stab+Bleed (Requires Weapon) */
  if (trick <= 20) {
    act("You stab your weapon deeply into $N, opening a gruesome gaping wound.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n stabs $s weapon deeply into you, opening a gruesome gaping wound.", FALSE, ch, 0, victim, TO_VICT);
    act("$n stabs $s weapon deeply into $N, opening a gruesome gaping wound.", FALSE, ch, 0, victim, TO_NOTVICT);

    ench.name = strdup("Gaping Wound");
    ench.type = SKILL_DIRTY_TRICKS;
    ench.duration = 0;
    ench.location = 0;
    ench.modifier = 0;
    ench.bitvector = 0;
    ench.bitvector2 = 0;
    ench.func = dirty_tricks_enchantment;

    enchantment_to_char(victim, &ench, FALSE);
  }
  /* 30% Chance Blind */
  else if (trick <= 50) {
    act("You throw some blinding dust into $N's eyes.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n throws some blinding dust into your eyes.", FALSE, ch, 0, victim, TO_VICT);
    act("$n throws blinding dust into $N's eyes.", FALSE, ch, 0, victim, TO_NOTVICT);

    act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You have been blinded!\n\r", victim);

    af.type = SPELL_BLINDNESS;
    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.duration = 0;
    af.bitvector = AFF_BLIND;
    af.bitvector2 = 0;

    affect_to_char(victim, &af);

    af.location = APPLY_AC;
    af.modifier = +40;

    affect_to_char(victim, &af);
  }
  /* 50% Chance Stun */
  else {
    if (AWAKE(victim) && IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, BT_INVUL)) {
      act("You kick $N savagely in the groin but $E seems unfazed.", FALSE, ch, 0, victim, TO_CHAR);
      act("$n kicks you savagely in the groin but you feel unfazed.", FALSE, ch, 0, victim, TO_VICT);
      act("$n kicks $N savagely in the groin but $E seems unfazed.", FALSE, ch, 0, victim, TO_NOTVICT);

      damage(ch, victim, 0, SKILL_DIRTY_TRICKS, DAM_NO_BLOCK);
    }
    else {
      act("You kick $N savagely in the groin, causing $M to double over in pain!", FALSE, ch, 0, victim, TO_CHAR);
      act("$n kicks you savagely in the groin, causing you to double over in pain!", FALSE, ch, 0, victim, TO_VICT);
      act("$n kicks $N savagely in the groin, causing $M to double over in pain!", FALSE, ch, 0, victim, TO_NOTVICT);

      set_pos = stack_position(victim, POSITION_SITTING);

      damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_DIRTY_TRICKS, DAM_PHYSICAL);

      if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim)) {
        GET_POS(victim) = set_pos;

        /* Can't use skill_wait() since this applies to victim. */
        WAIT_STATE(victim, PULSE_VIOLENCE);
      }
    }
  }
}


/* control the fights going on */
void perform_violence(void) {
  CHAR *ch = NULL;
  CHAR *vict = NULL;

  for (ch = combat_list; ch; ch = combat_next_dude) {
    combat_next_dude = ch->next_fighting;
    assert(vict = GET_OPPONENT(ch));

    if (AWAKE(ch) && SAME_ROOM(ch, vict)) {
      /* Linerfix - Makes MSG_VIOLENCE only signal to the room of Satan, Cryohydra, Shadowraith or Ancient Red Dragon. */
      if (V_ROOM(ch) == 25541 ||
          V_ROOM(ch) == 23063 ||
          V_ROOM(ch) == 27748 ||
          V_ROOM(ch) == 17532) {
        if (signal_char(ch, vict, MSG_VIOLENCE, "")) return;
      }

      /* Shadowstep is before hit() in order to take advantage of pummel, etc. */
      if (affected_by_spell(ch, SKILL_SHADOWSTEP) && SAME_ROOM(ch, vict)) {
        shadowstep_action(ch, vict);
      }

      if (SAME_ROOM(ch, vict)) {
        hit(ch, vict, TYPE_UNDEFINED);
      }

      /* These skills are applied after hit() in order to avoid consuming pummel, etc. */
      if (affected_by_spell(ch, SPELL_BLOOD_LUST) && SAME_ROOM(ch, vict)) {
        blood_lust_action(ch, vict);
      }

      if (affected_by_spell(ch, SKILL_VICTIMIZE) && SAME_ROOM(ch, vict)) {
        victimize_action(ch, vict);
      }

        /* 30% average per MSG_MOBACT (1.8 average attempts per 60 seconds, or 18 combat rounds). */
      if (affected_by_spell(ch, SKILL_DIRTY_TRICKS) && chance(10) && SAME_ROOM(ch, vict)) {
        dirty_tricks_action(ch, vict);
      }
    }
    else { /* Not in same room. */
      stop_fighting(ch);
    }
  }
}

void mob_attack(CHAR *MOB);
void perform_mob_attack(void) {
  CHAR *ch = NULL;
  CHAR *vict = NULL;

  for (ch = combat_list; ch; ch = combat_next_dude) {
    combat_next_dude = ch->next_fighting;
    assert(vict = GET_OPPONENT(ch));

    if (IS_NPC(ch) && AWAKE(ch) && CHAR_REAL_ROOM(ch) == CHAR_REAL_ROOM(vict)) {
      mob_attack(ch);
    }
  }
}

/* New mob attack routine by Ranger of RoninMUD.
   Do not distribute without permission of the originator
   or of the IMPs of RoninMUD.

   Last Modification:  Jan 4, 1997
*/

void mob_attack(CHAR *mob) {

  int i,done=FALSE;
  char buf[MAX_STRING_LENGTH];
  CHAR *vict,*temp;
  struct obj_data *wield = 0;

  /* Some basic checks - most totally unnecessary, so sue me */
  if(!mob) return;
  if(!IS_NPC(mob)) return;
  if(mob->specials.no_att<1) return;

  /* We use mob att_timer the same way WAIT_STATE is used for PCs */
  if(mob->specials.att_timer>0) {
    mob->specials.att_timer--;
    return;
  }
  if(GET_POS(mob)<POSITION_FIGHTING) return;

  if(!mob->specials.fighting) return;
  if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(mob->specials.fighting)) return;

  /* Loop over the defined mob attacks */
  for (i=0;i<mob->specials.no_att;i++) {
    vict=0;
    /* Check for attack roll success */
    if(number(0,100) > mob->specials.att_percent[i]) continue;

    /* Roll was successful, try the defined attack */
    switch (mob->specials.att_type[i]) {

    case ATT_KICK:
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          /* This results in a continue, mobs don't kick themselves */
          continue;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              damage(mob, vict,0, SKILL_KICK,DAM_NO_BLOCK);
            }
            else {
              damage(mob, vict, GET_LEVEL(mob), SKILL_KICK,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_LEADER:
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              damage(mob, vict,0, SKILL_KICK,DAM_NO_BLOCK);
            }
            else {
              damage(mob, vict, GET_LEVEL(mob), SKILL_KICK,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_RAN_ROOM:
          vict=get_random_victim(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              damage(mob, vict,0, SKILL_KICK,DAM_NO_BLOCK);
            }
            else {
              damage(mob, vict, GET_LEVEL(mob), SKILL_KICK,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_RAN_GROUP:
          vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              damage(mob, vict,0, SKILL_KICK,DAM_NO_BLOCK);
            }
            else {
              damage(mob, vict, GET_LEVEL(mob), SKILL_KICK,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_ROOM:
          act("Your spin-kick has generated a big whirl.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n's spin-kick has generated a big whirl.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && GET_LEVEL(vict)<LEVEL_IMM && !IS_NPC(vict)) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You have been kicked by $n.", FALSE, mob, 0, vict, TO_VICT);
              damage(mob,vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_GROUP:
          act("Your spin-kick has generated a big whirl.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n's spin-kick has generated a big whirl.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && mob==vict->specials.fighting && GET_LEVEL(vict)<LEVEL_IMM) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You have been kicked by $n.", FALSE, mob, 0, vict, TO_VICT);
              damage(mob,vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      mob->specials.att_timer=3;
      done=TRUE;
      break;
    case ATT_PUMMEL:
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          /* This results in a continue, mobs don't pummel themselves */
          continue;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              act("You try to pummel $N, but miss and nearly hurt yourself.", FALSE, mob, 0, vict, TO_CHAR);
              act("$N tried to pummel you, but missed.", FALSE, vict, 0, mob, TO_CHAR);
              act("$n tried to pummel $N, but missed.", FALSE, mob, 0, vict, TO_NOTVICT);
              damage(mob, vict, 0, SKILL_PUMMEL,DAM_NO_BLOCK);
            }
            else {
              act("You pummel $N, and $N is stunned now!",FALSE,mob,0,vict,TO_CHAR);
              act("$N pummels you, and you are stunned now!",FALSE, vict, 0, mob, TO_CHAR);
              act("$n pummels $N and $N is stunned now!",FALSE, mob, 0, vict, TO_NOTVICT);
              damage(mob, vict, 10, SKILL_PUMMEL,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_STUNNED;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_LEADER:
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              act("You try to pummel $N, but miss and nearly hurt yourself.", FALSE, mob, 0, vict, TO_CHAR);
              act("$N tried to pummel you, but missed.", FALSE, vict, 0, mob, TO_CHAR);
              act("$n tried to pummel $N, but missed.", FALSE, mob, 0, vict, TO_NOTVICT);
              damage(mob, vict, 0, SKILL_PUMMEL,DAM_NO_BLOCK);
            }
            else {
              act("You pummel $N, and $N is stunned now!",FALSE,mob,0,vict,TO_CHAR);
              act("$N pummels you, and you are stunned now!",FALSE, vict, 0, mob, TO_CHAR);
              act("$n pummels $N and $N is stunned now!",FALSE, mob, 0, vict, TO_NOTVICT);
              damage(mob, vict, 10, SKILL_PUMMEL,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_STUNNED;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_RAN_ROOM:
          vict=get_random_victim(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              act("You try to pummel $N, but miss and nearly hurt yourself.", FALSE, mob, 0, vict, TO_CHAR);
              act("$N tried to pummel you, but missed.", FALSE, vict, 0, mob, TO_CHAR);
              act("$n tried to pummel $N, but missed.", FALSE, mob, 0, vict, TO_NOTVICT);
              damage(mob, vict, 0, SKILL_PUMMEL,DAM_NO_BLOCK);
            }
            else {
              act("You pummel $N, and $N is stunned now!",FALSE,mob,0,vict,TO_CHAR);
              act("$N pummels you, and you are stunned now!",FALSE, vict, 0, mob, TO_CHAR);
              act("$n pummels $N and $N is stunned now!",FALSE, mob, 0, vict, TO_NOTVICT);
              damage(mob, vict, 10, SKILL_PUMMEL,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_STUNNED;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_RAN_GROUP:
          vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              act("You try to pummel $N, but miss and nearly hurt yourself.", FALSE, mob, 0, vict, TO_CHAR);
              act("$N tried to pummel you, but missed.", FALSE, vict, 0, mob, TO_CHAR);
              act("$n tried to pummel $N, but missed.", FALSE, mob, 0, vict, TO_NOTVICT);
              damage(mob, vict, 0, SKILL_PUMMEL,DAM_NO_BLOCK);
            }
            else {
              act("You pummel $N, and $N is stunned now!",FALSE,mob,0,vict,TO_CHAR);
              act("$N pummels you, and you are stunned now!",FALSE, vict, 0, mob, TO_CHAR);
              act("$n pummels $N and $N is stunned now!",FALSE, mob, 0, vict, TO_NOTVICT);
              damage(mob, vict, 10, SKILL_PUMMEL,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_STUNNED;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_ROOM:
          act("You spin wildly about the room, pummeling like crazy.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n spins wildly about the room, pummeling like crazy.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && GET_LEVEL(vict)<LEVEL_IMM && !IS_NPC(vict)) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
                act("$N tried to pummel you, but missed.", FALSE, vict, 0, mob, TO_CHAR);
                damage(mob, vict, 0, SKILL_PUMMEL,DAM_NO_BLOCK);
              }
              else {
                act("You are pummeled by $N.",FALSE, vict, 0, mob, TO_CHAR);
                damage(mob, vict, 10, SKILL_PUMMEL,DAM_NO_BLOCK);
                GET_POS(vict) = POSITION_STUNNED;
                WAIT_STATE(vict, PULSE_VIOLENCE*2);
              }
            }
          }
          break;
        case TAR_GROUP:
          act("You spin wildly about the room, pummeling like crazy.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n spins wildly about the room, pummeling like crazy.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && mob==vict->specials.fighting && GET_LEVEL(vict)<LEVEL_IMM) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
                act("$N tried to pummel you, but missed.", FALSE, vict, 0, mob, TO_CHAR);
                damage(mob, vict, 0, SKILL_PUMMEL,DAM_NO_BLOCK);
              }
              else {
                act("You are pummeled by $N.",FALSE, vict, 0, mob, TO_CHAR);
                damage(mob, vict, 10, SKILL_PUMMEL,DAM_NO_BLOCK);
                GET_POS(vict) = POSITION_STUNNED;
                WAIT_STATE(vict, PULSE_VIOLENCE*2);
              }
            }
          }
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      mob->specials.att_timer=2;
      done=TRUE;
      break;
    case ATT_DISARM:
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          /* This results in a continue, mobs don't disarm themselves */
          continue;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          if(vict) {
            if (vict->equipment[WIELD]) wield = vict->equipment[WIELD];
            else continue;
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
            if(V_OBJ(wield)==11523) continue;
            act("You kick off $N's weapon.",FALSE,mob,0,vict,TO_CHAR);
            act("$N kicks off your weapon.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n kicks off $N's weapon.",FALSE, mob, 0, vict, TO_NOTVICT);
            sprintf(buf,"WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(mob),
                    GET_NAME(vict),OBJ_NAME(wield),world[CHAR_REAL_ROOM(vict)].number);
            log_s(buf);
            unequip_char(vict, WIELD);
            obj_to_room(wield, CHAR_REAL_ROOM(vict));
            wield->log=1;
            save_char(vict,NOWHERE);
          }
          break;
        case TAR_LEADER:
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          if(vict) {
            if (vict->equipment[WIELD]) wield = vict->equipment[WIELD];
            else continue;
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
            if(V_OBJ(wield)==11523) continue;
            act("You kick off $N's weapon.",FALSE,mob,0,vict,TO_CHAR);
            act("$N kicks off your weapon.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n kicks off $N's weapon.",FALSE, mob, 0, vict, TO_NOTVICT);
            sprintf(buf,"WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(mob),
                    GET_NAME(vict),OBJ_NAME(wield),world[CHAR_REAL_ROOM(vict)].number);
            log_s(buf);
            unequip_char(vict, WIELD);
            obj_to_room(wield, CHAR_REAL_ROOM(vict));
            wield->log=1;
            save_char(vict,NOWHERE);
          }
          break;
        case TAR_RAN_ROOM:
          vict=get_random_victim(mob);
          if(vict) {
            if (vict->equipment[WIELD]) wield = vict->equipment[WIELD];
            else continue;
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
            if(V_OBJ(wield)==11523) continue;
            act("You kick off $N's weapon.",FALSE,mob,0,vict,TO_CHAR);
            act("$N kicks off your weapon.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n kicks off $N's weapon.",FALSE, mob, 0, vict, TO_NOTVICT);
            sprintf(buf,"WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(mob),
                    GET_NAME(vict),OBJ_NAME(wield),world[CHAR_REAL_ROOM(vict)].number);
            log_s(buf);
            unequip_char(vict, WIELD);
            obj_to_room(wield, CHAR_REAL_ROOM(vict));
            wield->log=1;
            save_char(vict,NOWHERE);
          }
          break;
        case TAR_RAN_GROUP:
          vict=get_random_victim_fighting(mob);
          if(vict) {
            if (vict->equipment[WIELD]) wield = vict->equipment[WIELD];
            else continue;
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
            if(V_OBJ(wield)==11523) continue;
            act("You kick off $N's weapon.",FALSE,mob,0,vict,TO_CHAR);
            act("$N kicks off your weapon.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n kicks off $N's weapon.",FALSE, mob, 0, vict, TO_NOTVICT);
            sprintf(buf,"WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(mob),
                    GET_NAME(vict),OBJ_NAME(wield),world[CHAR_REAL_ROOM(vict)].number);
            log_s(buf);
            unequip_char(vict, WIELD);
            obj_to_room(wield, CHAR_REAL_ROOM(vict));
            wield->log=1;
            save_char(vict,NOWHERE);
          }
          break;
        case TAR_ROOM:
          act("You spins wildly about the room.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n spins wildly about the room.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && !IS_NPC(vict) && GET_LEVEL(vict)<LEVEL_IMM
               && vict!=mob && vict->equipment[WIELD]) {
              wield = vict->equipment[WIELD];
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              if(V_OBJ(wield)==11523) continue;
              act("$N kicks off your weapon.",FALSE, vict, 0, mob, TO_CHAR);
              sprintf(buf,"WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(mob),
                    GET_NAME(vict),OBJ_NAME(wield),world[CHAR_REAL_ROOM(vict)].number);
              log_s(buf);
              unequip_char(vict, WIELD);
              obj_to_room(wield, CHAR_REAL_ROOM(vict));
              wield->log=1;
              save_char(vict,NOWHERE);
            }
          }
          break;
        case TAR_GROUP:
          act("You spins wildly about the room.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n spins wildly about the room.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && !IS_NPC(vict) && GET_LEVEL(vict)<LEVEL_IMM && vict!=mob
               && mob==vict->specials.fighting && vict->equipment[WIELD]) {
              wield = vict->equipment[WIELD];
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              if(V_OBJ(wield)==11523) continue;
              act("$N kicks off your weapon.",FALSE, vict, 0, mob, TO_CHAR);
              sprintf(buf,"WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(mob),
                    GET_NAME(vict),OBJ_NAME(wield),world[CHAR_REAL_ROOM(vict)].number);
              log_s(buf);
              unequip_char(vict, WIELD);
              obj_to_room(wield, CHAR_REAL_ROOM(vict));
              wield->log=1;
              save_char(vict,NOWHERE);
            }
          }
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      mob->specials.att_timer=2;
      done=TRUE;
      break;
/* Removed attack bash messages - they are in the message file
   Ranger March 98 */
    case ATT_BASH:
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          /* This results in a continue, mobs don't bash themselves */
          continue;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          if(vict) {
            damage(mob, vict, number(1,GET_LEVEL(mob)), SKILL_BASH,DAM_SKILL);
          }
          break;
        case TAR_LEADER:
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          if(vict) {
            damage(mob, vict, number(1,GET_LEVEL(mob)), SKILL_BASH,DAM_SKILL);
          }
          break;
        case TAR_RAN_ROOM:
          vict=get_random_victim(mob);
          if(vict) {
            damage(mob, vict, number(1,GET_LEVEL(mob)), SKILL_BASH,DAM_SKILL);
          }
          break;
        case TAR_RAN_GROUP:
          vict=get_random_victim_fighting(mob);
          if(vict) {
            damage(mob, vict, number(1,GET_LEVEL(mob)), SKILL_BASH,DAM_SKILL);
          }
          break;
        case TAR_ROOM:
          act("You spin wildly about the room, bashing like crazy.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n spins wildly about the room, bashing like crazy.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && GET_LEVEL(vict)<LEVEL_IMM && !IS_NPC(vict)) {
              damage(mob, vict, number(1,GET_LEVEL(mob)), SKILL_BASH,DAM_SKILL);
            }
          }
          break;
        case TAR_GROUP:
          act("You spin wildly about the room, bashing like crazy.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n spins wildly about the room, bashing like crazy.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && mob==vict->specials.fighting && GET_LEVEL(vict)<LEVEL_IMM) {
              damage(mob, vict, number(1,GET_LEVEL(mob)), SKILL_BASH,DAM_SKILL);
            }
          }
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      mob->specials.att_timer=2;
      done=TRUE;
      break;
/* Removed attack punch messages - they are in the message file
   Ranger March 98 */
    case ATT_PUNCH:
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          /* This results in a continue, mobs don't punch themselves */
          continue;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              damage(mob, vict, 0, SKILL_PUNCH,DAM_NO_BLOCK);
            }
            else {
              damage(mob, vict, GET_LEVEL(mob)*2, SKILL_PUNCH,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_SITTING;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_LEADER:
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              damage(mob, vict, 0, SKILL_PUNCH,DAM_NO_BLOCK);
            }
            else {
              damage(mob, vict, GET_LEVEL(mob)*2, SKILL_PUNCH,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_SITTING;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_RAN_ROOM:
          vict=get_random_victim(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              damage(mob, vict, 0, SKILL_PUNCH,DAM_NO_BLOCK);
            }
            else {
              damage(mob, vict, GET_LEVEL(mob)*2, SKILL_PUNCH,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_SITTING;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_RAN_GROUP:
          vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) {
              damage(mob, vict, 0, SKILL_PUNCH,DAM_NO_BLOCK);
            }
            else {
              damage(mob, vict, GET_LEVEL(mob)*2, SKILL_PUNCH,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_SITTING;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_ROOM:
          act("You spin around with your arms outstretched.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n's spins around with $s arms outstretched.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && GET_LEVEL(vict)<LEVEL_IMM && !IS_NPC(vict)) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You are hit by a punch from $N.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_SITTING;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_GROUP:
          act("You spin around with your arms outstretched.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n's spins around with $s arms outstretched.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && mob==vict->specials.fighting && GET_LEVEL(vict)<LEVEL_IMM) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You are hit by a punch from $N.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_SITTING;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      mob->specials.att_timer=2;
      done=TRUE;
      break;
    case ATT_TAILSLAM:
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          /* This results in a continue, mobs don't tailslam themselves */
          continue;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You slam $N with your tail.",FALSE,mob,0,vict,TO_CHAR);
            act("$N slams you with $S tail.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n slams $N with $s tail.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
            GET_POS(vict) = POSITION_STUNNED;
            WAIT_STATE(vict, PULSE_VIOLENCE*2);
          }
          break;
        case TAR_LEADER:
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You slam $N with your tail.",FALSE,mob,0,vict,TO_CHAR);
            act("$N slams you with $S tail.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n slams $N with $s tail.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
            GET_POS(vict) = POSITION_STUNNED;
            WAIT_STATE(vict, PULSE_VIOLENCE*2);
          }
          break;
        case TAR_RAN_ROOM:
          vict=get_random_victim(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You slam $N with your tail.",FALSE,mob,0,vict,TO_CHAR);
            act("$N slams you with $S tail.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n slams $N with $s tail.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
            GET_POS(vict) = POSITION_STUNNED;
            WAIT_STATE(vict, PULSE_VIOLENCE*2);
          }
          break;
        case TAR_RAN_GROUP:
          vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You slam $N with your tail.",FALSE,mob,0,vict,TO_CHAR);
            act("$N slams you with $S tail.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n slams $N with $s tail.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
            GET_POS(vict) = POSITION_STUNNED;
            WAIT_STATE(vict, PULSE_VIOLENCE*2);
          }
          break;
        case TAR_ROOM:
          act("You spin your tail around.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n's spins his tail around.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && GET_LEVEL(vict)<LEVEL_IMM && !IS_NPC(vict)) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You are hit by $N's tail.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_STUNNED;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_GROUP:
          act("You spin your tail around.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n's spins his tail around.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && mob==vict->specials.fighting && GET_LEVEL(vict)<LEVEL_IMM) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You are hit by $N's tail.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
              GET_POS(vict) = POSITION_STUNNED;
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      mob->specials.att_timer=3;
      done=TRUE;
      break;
    case ATT_TRAMPLE:
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          /* This results in a continue, mobs don't trample themselves */
          continue;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You jump into the air and land on $N.",FALSE,mob,0,vict,TO_CHAR);
            act("$N jumps into the air and lands on you.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n jumps into the air and lands on $N.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE*2);
          }
          break;
        case TAR_LEADER:
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You jump into the air and land on $N.",FALSE,mob,0,vict,TO_CHAR);
            act("$N jumps into the air and lands on you.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n jumps into the air and lands on $N.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE*2);
          }
          break;
        case TAR_RAN_ROOM:
          vict=get_random_victim(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You jump into the air and land on $N.",FALSE,mob,0,vict,TO_CHAR);
            act("$N jumps into the air and lands on you.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n jumps into the air and lands on $N.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE*2);
          }
          break;
        case TAR_RAN_GROUP:
          vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You jump into the air and land on $N.",FALSE,mob,0,vict,TO_CHAR);
            act("$N jumps into the air and lands on you.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n jumps into the air and lands on $N.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE*2);
          }
          break;
        case TAR_ROOM:
          act("You jump high in the air and land on everyone.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n's jumps high in the air and lands on everyone.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && GET_LEVEL(vict)<LEVEL_IMM && !IS_NPC(vict)) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You have been trampled by $N.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        case TAR_GROUP:
          act("You jump high in the air and land on everyone.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n's jumps high in the air and lands on everyone.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && mob==vict->specials.fighting && GET_LEVEL(vict)<LEVEL_IMM) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You have been trampled by $N.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob)*2, TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*2);
            }
          }
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      mob->specials.att_timer=2;
      done=TRUE;
      break;
    case ATT_BITE:
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          /* This results in a continue, mobs don't bite themselves */
          continue;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You take a bite out of $N.",FALSE,mob,0,vict,TO_CHAR);
            act("$N takes a bite out of you.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n takes a bite out of $N.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob, vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE);
          }
          break;
        case TAR_LEADER:
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You take a bite out of $N.",FALSE,mob,0,vict,TO_CHAR);
            act("$N takes a bite out of you.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n takes a bite out of $N.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob, vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE);
          }
          break;
        case TAR_RAN_ROOM:
          vict=get_random_victim(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You take a bite out of $N.",FALSE,mob,0,vict,TO_CHAR);
            act("$N takes a bite out of you.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n takes a bite out of $N.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob, vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE);
          }
          break;
        case TAR_RAN_GROUP:
          vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You take a bite out of $N.",FALSE,mob,0,vict,TO_CHAR);
            act("$N takes a bite out of you.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n takes a bite out of $N.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob, vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE);
          }
          break;
        case TAR_ROOM:
          act("You snap your jaws wildly about, biting all around you.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n snaps $s jaws wildly about, biting all around $m.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && GET_LEVEL(vict)<LEVEL_IMM && !IS_NPC(vict)) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You are bitten by $N.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE);
            }
          }
          break;
        case TAR_GROUP:
          act("You snap your jaws wildly about, biting all around you.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n snaps $s jaws wildly about, biting all around $m.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && mob==vict->specials.fighting && GET_LEVEL(vict)<LEVEL_IMM) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You are bitten by $N.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE);
            }
          }
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      mob->specials.att_timer=2;
      done=TRUE;
      break;
    case ATT_CLAW:
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          /* This results in a continue, mobs don't claw themselves */
          continue;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You rake $N with your claws.",FALSE,mob,0,vict,TO_CHAR);
            act("$N rakes you with $S claws.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n rakes $N with $s claws.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob, vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE*3);
          }
          break;
        case TAR_LEADER:
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You rake $N with your claws.",FALSE,mob,0,vict,TO_CHAR);
            act("$N rakes you with $S claws.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n rakes $N with $s claws.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob, vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE*3);
          }
          break;
        case TAR_RAN_ROOM:
          vict=get_random_victim(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You rake $N with your claws.",FALSE,mob,0,vict,TO_CHAR);
            act("$N rakes you with $S claws.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n rakes $N with $s claws.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob, vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE*3);
          }
          break;
        case TAR_RAN_GROUP:
          vict=get_random_victim_fighting(mob);
          if(vict) {
            if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) break;
            act("You rake $N with your claws.",FALSE,mob,0,vict,TO_CHAR);
            act("$N rakes you with $S claws.",FALSE, vict, 0, mob, TO_CHAR);
            act("$n rakes $N with $s claws.",FALSE, mob, 0, vict, TO_NOTVICT);
            damage(mob, vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(vict, PULSE_VIOLENCE*3);
          }
          break;
        case TAR_ROOM:
          act("You lash wildly about, raking everyone with your claws.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n lashs wildly about, raking everyone with $s claws.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && GET_LEVEL(vict)<LEVEL_IMM && !IS_NPC(vict)) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You have been clawed by $N.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*3);
            }
          }
          break;
        case TAR_GROUP:
          act("You lash wildly about, raking everyone with your claws.", FALSE, mob, 0, 0, TO_CHAR);
          act("$n lashs wildly about, raking everyone with $s claws.", FALSE, mob, 0, 0, TO_ROOM);
          for (vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=temp) {
            temp = vict->next_in_room;
            if(vict && vict!=mob && mob==vict->specials.fighting && GET_LEVEL(vict)<LEVEL_IMM) {
              if(IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(mob,vict,BT_INVUL)) continue;
              act("You have been clawed by $N.",FALSE, vict, 0, mob, TO_CHAR);
              damage(mob,vict, GET_LEVEL(mob), TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*3);
            }
          }
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      mob->specials.att_timer=3;
      done=TRUE;
      break;
    case ATT_SPELL_CAST:
    case ATT_SPELL_SKILL:
      if(mob->specials.att_spell[i]<=0) continue;
      switch (mob->specials.att_target[i]) {
        case TAR_SELF:
          vict=mob;
          break;
        case TAR_BUFFER:
          vict=mob->specials.fighting;
          break;
        case TAR_LEADER:
          vict=mob->specials.fighting;
          if(vict->master) vict=vict->master;
          if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict))
            vict=get_random_victim_fighting(mob);
          break;
        case TAR_RAN_ROOM:
        case TAR_ROOM:
          vict=get_random_victim(mob);
          break;
        case TAR_RAN_GROUP:
        case TAR_GROUP:
          vict=get_random_victim_fighting(mob);
          break;
        default:
          sprintf(buf,"WIZINFO: Invalid attack target (%d) called in attack (%d), mob %d.",
          mob->specials.att_target[i],mob->specials.att_type[i],V_MOB(mob));
          wizlog(buf, LEVEL_SUP, 6);
          continue;
      }
      if(!vict) continue;

      sprintf(buf,"'%s' %s",spells[mob->specials.att_spell[i]-1],GET_NAME(vict));
      if(mob->specials.att_type[i]==ATT_SPELL_SKILL) do_mob_cast(mob,buf,0);
      if(mob->specials.att_type[i]==ATT_SPELL_CAST) do_mob_cast(mob,buf,1);

      done=TRUE;
      break;
    default:
      sprintf(buf,"WIZINFO: Invalid attack type (%d) called in mob (%d) attack (i=%d).",
      mob->specials.att_type[i],V_MOB(mob),i);
      wizlog(buf, LEVEL_SUP, 6);
    }
    if(done) return;
  }
  return; /* Not needed, but looks nice at the end <grin> */
}

#define TOKEN_MOB 11

void brag(struct char_data *ch, struct char_data *vict) {
  struct descriptor_data *i;
  char brag[MSL];
  CHAR* rashgugh;

  switch (number(0, 45)) {
  case 0:
   sprintf(brag, "$n brags, '%s was just too easy a kill!'", GET_NAME(vict));break;
  case 1:
   sprintf(brag, "$n brags, '%s was a tasty dinner, now who's for dessert?'", GET_NAME(vict));break;
  case 2:
   sprintf(brag, "$n brags, 'Bahaha! %s should stick to Odif's !'",GET_NAME(vict));break;
  case 3:
   sprintf(brag, "$n brags, '%s is now in need of some exp...'", GET_NAME(vict));break;
  case 4:
   sprintf(brag, "$n brags, '%s needs a hospital now.'",GET_NAME(vict));break;
  case 5:
   sprintf(brag, "$n brags, '%s is such a wimp, no challenge at all.'", GET_NAME(vict));break;
  case 6:
   sprintf(brag, "$n brags, '%s is a punk, hits like a dragonfly. Bah.'", GET_NAME(vict));break;
  case 7:
   sprintf(brag, "$n brags, '%s, your life force has just run out...'", GET_NAME(vict));break;
  case 8:
   sprintf(brag, "$n brags, 'Bah, %s should stick to the training ground!'",GET_NAME(vict));break;
  case 9:
   sprintf(brag, "$n brags, '%s, give me your family's number and I might return your corpse.'", GET_NAME(vict));break;
  case 10:
   sprintf(brag, "$n brags, 'Hey %s!  Come back, you dropped your corpse!'", GET_NAME(vict));break;
  case 11:
   sprintf(brag, "$n brags, 'I think %s wears pink chainmail.  Fights like a girl!'", GET_NAME(vict));break;
  case 12:
   sprintf(brag, "$n brags, 'Hahaha! %s hits like a girl! Sissy!'", GET_NAME(vict));break;
  case 13:
   sprintf(brag, "$n brags, 'I charp in your direction, %s!'", GET_NAME(vict));break;
  case 14:
   sprintf(brag, "$n brags, 'I guess you thought you could whoop me, eh %s?'", GET_NAME(vict));break;
  case 15:
   sprintf(brag, "$n brags, 'If that's all you can do, you had better try harder %s, cause it ain't enough to bring ME down!'", GET_NAME(vict));break;
  case 16:
   sprintf(brag, "$n brags, 'Where did you get that weapon %s? The newts? :P'", GET_NAME(vict));break;
  case 17:
   sprintf(brag, "$n brags, 'If you wanna play with the big boys, you had better get BIG %s.'", GET_NAME(vict));break;
  case 18:
   sprintf(brag, "$n brags, 'Dunt, Dunt, Dunt, Another one bites the dust! Or Should I say, %s did. *chortle*'", GET_NAME(vict));break;
  case 19:
   sprintf(brag, "$n brags, 'Game Over %s.. Game Over.'", GET_NAME(vict));break;
  case 20:
   sprintf(brag, "$n brags, 'Haha, %s is no match for me!'", GET_NAME(vict));break;
  case 21:
   sprintf(brag, "$n brags, '%s fights like a yeasty bit of stomach bile!'", GET_NAME(vict));break;
  case 22:
   sprintf(brag, "$n brags, 'Mmmm! Look what goodies %s left for me in their corpse!'", GET_NAME(vict));break;
  case 23:
   sprintf(brag, "$n brags, '%s couldn't hit the broad side of a barn! Let alone the actual cow.'", GET_NAME(vict));break;
  case 24:
   sprintf(brag, "$n brags, 'Who taught you how to fight %s, your Grandma?'", GET_NAME(vict));break;
  case 25:
   sprintf(brag, "$n brags, 'Muahaha %s, try as you might, you'll never kill me!'", GET_NAME(vict));break;
  case 26:
   sprintf(brag, "$n brags, 'One thing's for sure, %s should stop trying while ahead!'", GET_NAME(vict));break;
  case 27:
   sprintf(brag, "$n brags, 'Oow %s, hope you didn't lose any stats! Hahaha!'", GET_NAME(vict));break;
  case 28:
   sprintf(brag, "$n brags, 'So you thought you could kill me, eh %s?'", GET_NAME(vict));break;
  case 29:
   sprintf(brag, "$n brags, 'To junk %s's corpse, or to not junk %s's corpse, that is the question...'", GET_NAME(vict), GET_NAME(vict));break;
  case 30:
   sprintf(brag, "$n brags, 'Hey %s, come back and fight like a man!'", GET_NAME(vict));break;
  case 31:
   sprintf(brag, "$n brags, 'Hey Everyone, %s is naked at the temple! Don't stare too long. :P'", GET_NAME(vict));break;
  case 32:
   sprintf(brag, "$n brags, '%s is inferior to my manliness!!'", GET_NAME(vict));break;
  case 33:
   sprintf(brag, "$n brags, 'Ooh %s, that just had to hurt.'", GET_NAME(vict));break;
  case 34:
   sprintf(brag, "$n brags, 'Someone come get %s's corpse outta here, its starting to smell like rotten fish.'", GET_NAME(vict));break;
  case 35:
   sprintf(brag, "$n brags, 'That was a really good attempt at killing me %s. Better luck next time.'", GET_NAME(vict));break;
  case 36:
   sprintf(brag, "$n brags, 'Hey %s, you coming back for more anytime soon?'", GET_NAME(vict));break;
  case 37:
   sprintf(brag, "$n brags, '%s is no match for my superior skills! *flex*'", GET_NAME(vict));break;
  case 38:
   sprintf(brag, "$n brags, 'Hey %s, nice killing you, see ya again sometime!'", GET_NAME(vict));break;
  case 39:
   sprintf(brag, "$n brags, 'Ouch, how much exp will that cost you %s?'", GET_NAME(vict));break;
  case 40:
   sprintf(brag, "$n brags, 'I have SLAIN %s! Fear my wrath!'", GET_NAME(vict));break;
  case 41:
   sprintf(brag, "$n brags, '%s Has failed, yet again, to slay me!'", GET_NAME(vict));break;
  case 42:
   sprintf(brag, "$n brags, '%s, you are the weakest link, good-bye!'", GET_NAME(vict));break;
  case 43:
   sprintf(brag, "$n brags, 'Thanks for the meta, %s!  I got a 7!'", GET_NAME(vict));break;
  case 44:
   sprintf(brag, "$n brags, 'LEVEL !!'");break;
  case 45:
   sprintf(brag, "$n brags, 'Hey %s, where'd you get your equipment - donation? MUhaha!'", GET_NAME(vict));break;
  }

  for (i = descriptor_list; i; i = i->next) {
    if(i->character && (i->character != ch) && !i->connected
       && (IS_SET(i->character->specials.pflag, PLR_GOSSIP)
       || i->original ) ) {
      COLOR(i->character,5);
      act(brag,0,ch,0,i->character,TO_VICT);
      ENDCOLOR(i->character);
    }
  }
  /*If rashgugh is in the game, brag too*/
  if (chance(25))
  {
     rashgugh = get_ch_world(TOKEN_MOB);
     if (rashgugh)
     {
       sprintf(brag, "I didn't attend the funeral, but I sent a nice letter saying I approved of it.");
       do_yell(rashgugh, brag, CMD_YELL);
     }
  }
}
