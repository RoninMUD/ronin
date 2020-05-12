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
#include <stdint.h>

#include "structs.h"
#include "constants.h"
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
#include "quest.h"
#include "enchant.h"

/* Structures */

CHAR *combat_list = 0;      /* head of l-list of fighting chars */
CHAR *combat_next_dude = 0; /* Next dude global trick           */

/* External procedures */

char *fread_string(FILE *f1);
void page_string(struct descriptor_data *d, char *str, int keep_internal);

void stop_follower(CHAR *ch);
void do_flee(CHAR *ch, char *argument, int cmd);
void stop_riding(struct char_data *ch,struct char_data *vict);
void die(CHAR *ch);
void brag(struct char_data *ch, struct char_data *victim);
int hit_limit(CHAR * ch);

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


void appear(CHAR *ch) {
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);

  if (affected_by_spell(ch, SPELL_INVISIBLE)) {
    affect_from_char(ch, SPELL_INVISIBLE);
  }

  REMOVE_BIT(GET_AFF(ch), AFF_INVISIBLE);
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
  if (GET_MOUNT(victim) && !SAME_ROOM(GET_MOUNT(victim), victim)) {
    stop_riding(victim, GET_MOUNT(victim));
  }

  if (GET_RIDER(victim) && !SAME_ROOM(GET_RIDER(victim), victim)) {
    stop_riding(GET_RIDER(victim), victim);
  }

  if ((GET_POS(victim) == POSITION_FIGHTING) && !GET_OPPONENT(victim)) {
    GET_POS(victim) = POSITION_STANDING;
  }

  if (((GET_POS(victim) == POSITION_FLYING) && !IS_AFFECTED(victim, AFF_FLY)) ||
      (IS_NPC(victim) && !IS_SET(GET_ACT(victim), ACT_FLY))) {
    GET_POS(victim) = POSITION_STANDING;
  }

  if (((GET_POS(victim) == POSITION_STANDING) && IS_AFFECTED(victim, AFF_FLY)) ||
      (IS_NPC(victim) && IS_SET(GET_ACT(victim), ACT_FLY))) {
    GET_POS(victim) = POSITION_FLYING;
  }

  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) return;
  else if (GET_HIT(victim) > 0) GET_POS(victim) = POSITION_STANDING;
  else if (GET_HIT(victim) < -10) GET_POS(victim) = POSITION_DEAD;
  else if (GET_HIT(victim) < -5) GET_POS(victim) = POSITION_MORTALLYW;
  else if (GET_HIT(victim) < -2) GET_POS(victim) = POSITION_INCAP;
  else GET_POS(victim) = POSITION_STUNNED;
}


/* Start one char fighting another (yes, it is horrible, I know...) */
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

  /* Sidearm
     Note: This is a bit of a hack, but it adds some "realism" to the initial
     variance of when Sidearm will first trigger after engaging in combat. */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_MERCENARY, 5) && GET_WEAPON2(ch) && !enchanted_by(ch, "Readying Sidearm...")) {
    enchantment_apply(ch, FALSE, "Readying Sidearm...", 0, number(2, 4), ENCH_INTERVAL_ROUND, 0, 0, 0, 0, 0);
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

  if (GET_MOUNT(ch) && SAME_ROOM(ch, GET_MOUNT(ch))) {
    GET_POS(ch) = POSITION_RIDING;
  }
  else {
    GET_POS(ch) = POSITION_STANDING;
  }

  /* make sure player doesn't automatically fall in a must-fly area */
  if (IS_NPC(ch) && IS_SET(GET_ACT(ch), ACT_FLY)) {
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
    sprintf(buf,"corpse pcorpse %s",GET_NAME(ch));
    string_to_lower(buf);
    corpse->name = str_dup(buf);
  }
  else
    corpse->name = str_dup("corpse mcorpse");
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
void check_chaotic(OBJ *obj, OBJ *corpse) {
  int rnum, pos;
  struct obj_data *tmp, *next_obj;

  if (!obj || !corpse) return;

  rnum = obj->item_number;

  if ((rnum > 0) && IS_OBJ_STAT(obj, ITEM_CHAOTIC)) {
    if (obj->in_obj) {
      obj_from_obj(obj);
      obj_to_obj(obj, corpse);
    }

    if (obj->carried_by) {
      obj_from_char(obj);
      obj_to_obj(obj, corpse);
    }

    if (obj->equipped_by) {
      for (pos = 0; pos < MAX_WEAR; pos++) {
        if (obj == EQ(obj->equipped_by, pos)) {
          obj_to_obj(unequip_char(obj->equipped_by, pos), corpse);
          break;
        }
      }
    }
  }

  for (tmp = obj->contains; tmp; tmp = next_obj) {
    next_obj = tmp->next_content;
    check_chaotic(tmp, corpse);
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
void change_alignment(CHAR *ch, CHAR *victim) {
  if (!ch || !victim) return;

  if ((CHAR_REAL_ROOM(ch) < 0) || (CHAR_REAL_ROOM(ch) > top_of_world)) return;

  if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CHAOTIC)) return;

  int align = -(GET_ALIGNMENT(victim) / 10);

  GET_ALIGNMENT(ch) += align / 2;
  GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch), -1000);
  GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch), 1000);

}


void death_cry(CHAR *ch) {
  if (!ch) return;

  if ((CHAR_REAL_ROOM(ch) < 0) || (CHAR_REAL_ROOM(ch) > top_of_world)) return;

  act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);

  for (int dir = NORTH; dir <= DOWN; dir++) {
    if (CAN_GO(ch, dir)) {
      send_to_room("Your blood freezes as you hear someone's death cry.\n\r",
        world[CHAR_REAL_ROOM(ch)].dir_option[dir]->to_room_r);
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


void raw_kill(CHAR *ch) {
  if (ch->specials.fighting) {
    stop_fighting(ch);
  }

  death_cry(ch);

  make_corpse(ch);

  if (ch->questobj) {
    ch->questobj->owned_by = 0;
  }

  if (ch->questmob) {
    ch->questmob->questowner = 0;
  }

  if ((ch->quest_status == QUEST_RUNNING) || (ch->quest_status == QUEST_COMPLETED)) {
    ch->ver3.time_to_quest = MAX(ch->ver3.time_to_quest - 40, 5);
  }

  ch->questgiver = 0;
  ch->questobj = 0;
  ch->questmob = 0;
  ch->quest_level = 0;
  ch->quest_status = QUEST_NONE;

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

  /* Add bonus to xp for groups */
  if (total_pcs >= 6) {
    /* Group XP Bonus:
    **
    **   6 or more = 10%
    **   11 or more = 20%
    **   16 or more = 30%
    **   Every 5 players adds an extra 10%, up to a max of 100%
    */

    total_exp = ((total_exp * (10 + MIN(10, (((total_pcs - 6) / 5) + 1)))) / 10);
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


char *replace_string(char *str, char *weapon) {
  static char buf[MIL];
  char *cp;

  cp = buf;

  while (*str) {
    if (*str == '#') {
      switch (*(++str)) {
        case 'W':
          while (*weapon) *(cp++) = *(weapon++);
          break;

        default:
          *(cp++) = '#';
          break;
      }
    }
    else {
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
      "$n's shadow misses $N with its #W.",
      "Your shadow misses $N with its #W.",
      "$n's shadow misses you with its #W."
    },

    { /* > 0 */
      "$n's shadow tickles $N with its #W.",
      "Your shadow tickles $N as it #W $M.",
      "$n's shadow tickles you as it #W you."
    },

    { /* > 2 */
      "$n's shadow barely #W $N.",
      "Your shadow barely #W $N.",
      "$n's shadow barely #W you."
    },

    { /* > 4 */
      "$n's shadow #W $N.",
      "Your shadow #W $N.",
      "$n's shadow #W you."
    },

    { /* > 6 */
      "$n's shadow #W $N hard.",
      "Your shadow #W $N hard.",
      "$n's shadow #W you hard."
    },

    { /* > 10 */
      "$n's shadow #W $N very hard.",
      "Your shadow #W $N very hard.",
      "$n's shadow #W you very hard."
    },

    { /* > 15 */
      "$n's shadow #W $N extremely hard.",
      "Your shadow #W $N extremely hard.",
      "$n's shadow #W you extremely hard."
    },

    { /* > 20 */
      "$n's shadow massacres $N to small fragments with its #W.",
      "Your shadow massacres $N to small fragments with its #W.",
      "$n's shadow massacres you to small fragments with its #W."
    },

    { /* > 30 */
      "$n's shadow obliterates $N with its #W.",
      "Your shadow obliterates $N with its #W.",
      "$n's shadow obliterates you with its #W."
    },

    { /* > 50 */
      "$n's shadow utterly annihilates $N with its #W.",
      "Your shadow utterly annihilates $N with its #W.",
      "$n's shadow utterly annihilates you with its #W."
    },

    { /* > 70 */
      "$n's shadow removes chunks of flesh from $N with its #W.",
      "Your shadow removes chunks of flesh from $N with its #W.",
      "$n's shadow sends chunks of your flesh flying with its #W."
    },

    { /* > 100 */
      "$n's shadow makes $N see stars with a terrific wallop from its #W.",
      "Your shadow makes $N see stars with a terrific wallop from its #W.",
      "$n's shadow makes you see stars with a terrific wallop from its #W."
    },

    { /* > 170 */
      "$n's shadow's #W makes $N think twice about $S continued existence.",
      "Your shadow's #W makes $N think twice about $S continued existence.",
      "$n's shadow's #W makes you think twice your continued existence."
    },

    { /* > 250 */
      "$N's bones crumble under $n's shadow's terrific #W.",
      "$N's bones crumble under your shadow's terrific #W.",
      "Your bones crumble under $n's shadow's terrific #W."
    },

    { /* > 350 */
      "$n's shadow's tremendous #W sends gouts of blood and gore showering from $N.",
      "Your shadow's tremendous #W sends gouts of blood and gore showering from $N.",
      "$n's shadow's tremendous #W sends gouts of blood and gore showering from you."
    },

    { /* > 450 */
      "With soul shattering force, $n's shadow's #W completely devastates $N.",
      "With soul shattering force, your shadow's #W completely devastates $N.",
      "With soul shattering force, $n's shadow's #W completely devastates you."
    },

    { /* > 550 */
      "$n's shadow's cataclysmic #W pulverizes $N's flesh into a fine paste.",
      "Your shadow's cataclysmic #W pulverizes $N's flesh into a fine paste.",
      "$n's shadow's cataclysmic #W pulverizes your flesh into a fine paste."
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
    if (index != 0 && index <= 6)
    {
      buf = replace_string(dam_shadow[index].to_char, attack_hit_text[attack_type].plural);
    }
    else
    {
      buf = replace_string(dam_shadow[index].to_char, attack_hit_text[attack_type].singular);
    }
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

bool is_immune(CHAR *ch, int attack_type, int damage_type) {
  if (!ch) return FALSE;

  if (GET_IMMUNE(ch) &&
      (((attack_type == TYPE_HIT) && IS_IMMUNE(ch, IMMUNE_HIT)) ||
       ((attack_type == TYPE_BLUDGEON) && IS_IMMUNE(ch, IMMUNE_BLUDGEON)) ||
       ((attack_type == TYPE_PIERCE) && IS_IMMUNE(ch, IMMUNE_PIERCE)) ||
       ((attack_type == TYPE_SLASH) && IS_IMMUNE(ch, IMMUNE_SLASH)) ||
       ((attack_type == TYPE_WHIP) && IS_IMMUNE(ch, IMMUNE_WHIP)) ||
       ((attack_type == TYPE_CLAW) && IS_IMMUNE(ch, IMMUNE_CLAW)) ||
       ((attack_type == TYPE_BITE) && IS_IMMUNE(ch, IMMUNE_BITE)) ||
       ((attack_type == TYPE_STING) && IS_IMMUNE(ch, IMMUNE_STING)) ||
       ((attack_type == TYPE_CRUSH) && IS_IMMUNE(ch, IMMUNE_CRUSH)) ||
       ((attack_type == TYPE_HACK) && IS_IMMUNE(ch, IMMUNE_HACK)) ||
       ((attack_type == TYPE_CHOP) && IS_IMMUNE(ch, IMMUNE_CHOP)) ||
       ((attack_type == TYPE_SLICE) && IS_IMMUNE(ch, IMMUNE_SLICE)) ||
       ((damage_type == DAM_FIRE) && IS_IMMUNE(ch, IMMUNE_FIRE)) ||
       ((damage_type == DAM_ELECTRIC) && IS_IMMUNE(ch, IMMUNE_ELECTRIC)) ||
       ((damage_type == DAM_POISON) && IS_IMMUNE(ch, IMMUNE_POISON)) ||
       ((damage_type == DAM_PHYSICAL) && IS_IMMUNE(ch, IMMUNE_PHYSICAL)) ||
       ((damage_type == DAM_MAGICAL) && IS_IMMUNE(ch, IMMUNE_MAGICAL)))) {
    return TRUE;
  }

  if (GET_IMMUNE2(ch) &&
      (((damage_type == DAM_COLD) && IS_IMMUNE2(ch, IMMUNE2_COLD)) ||
       ((damage_type == DAM_SOUND) && IS_IMMUNE2(ch, IMMUNE2_SOUND)) ||
       ((damage_type == DAM_CHEMICAL) && IS_IMMUNE2(ch, IMMUNE2_CHEMICAL)) ||
       ((damage_type == DAM_ACID) && IS_IMMUNE2(ch, IMMUNE2_ACID)))) {
    return TRUE;
  }

  return FALSE;
}

bool is_resistant(CHAR *ch, int attack_type, int damage_type) {
  if (!ch) return FALSE;

  if (GET_RESIST(ch) &&
      (((damage_type == DAM_FIRE) && IS_RESISTANT(ch, RESIST_FIRE)) ||
       ((damage_type == DAM_ELECTRIC) && IS_RESISTANT(ch, RESIST_ELECTRIC)) ||
       ((damage_type == DAM_COLD) && IS_RESISTANT(ch, RESIST_COLD)) ||
       ((damage_type == DAM_SOUND) && IS_RESISTANT(ch, RESIST_SOUND)) ||
       ((damage_type == DAM_CHEMICAL) && IS_RESISTANT(ch, RESIST_CHEMICAL)) ||
       ((damage_type == DAM_ACID) && IS_RESISTANT(ch, RESIST_ACID)) ||
       ((damage_type == DAM_MAGICAL) && IS_RESISTANT(ch, RESIST_MAGICAL)) ||
       ((damage_type == DAM_PHYSICAL) && IS_RESISTANT(ch, RESIST_PHYSICAL)))) {
    return TRUE;
  }

  return FALSE;
}

int resist_damage(CHAR *ch, int dmg, int attack_type, int damage_type) {
  if (!ch || !dmg) return 0;

  if (is_immune(ch, attack_type, damage_type)) {
    return 0;
  }

  if (is_resistant(ch, attack_type, damage_type)) {
    return lround((dmg * (number(25, 75)) / 100.0));
  }

  return dmg;
}


/* Returns the amount of damage done after any/all mitigation. */
int get_attack_type(CHAR *ch, OBJ *weapon);
int damage(CHAR *ch, CHAR *victim, int dmg, int attack_type, int damage_type) {
  char buf[MSL];

  int original_damage = dmg;

  if (!ch || !victim || !IS_ALIVE(victim) || !SAME_ROOM(victim, ch) || (dmg < 0)) return 0;

  /* No damage to victims in safe rooms. */
  if (ROOM_SAFE(CHAR_REAL_ROOM(victim))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return 0;
  }

  /* No damage to immortals. */
  if (IS_IMMORTAL(victim)) {
    dmg = 0;
  }

  /* Adjust for shadow damage. */
  int shadow_damage = FALSE;
  if (attack_type == TYPE_SHADOW) {
    attack_type = get_attack_type(ch, GET_WEAPON(ch));

    shadow_damage = TRUE;
  }

  /* Protect */
  if (!IS_NPC(victim) &&
      GET_PROTECTOR(victim) &&
      !IS_NPC(GET_PROTECTOR(victim)) &&
      IS_ALIVE(GET_PROTECTOR(victim)) &&
      SAME_ROOM(victim, GET_PROTECTOR(victim)) &&
      (GET_PROTECTEE(GET_PROTECTOR(victim)) == victim) &&
      !IS_AFFECTED(GET_PROTECTOR(victim), AFF_FURY) &&
      (number(1, SKILL_MAX_PRAC) <= GET_LEARNED(GET_PROTECTOR(victim), SKILL_PROTECT)) &&
      chance(90)) {
    act("$N takes the damage meant for you!", FALSE, victim, 0, GET_PROTECTOR(victim), TO_CHAR);
    act("You take the damage meant for $n!", FALSE, victim, 0, GET_PROTECTOR(victim), TO_VICT);
    act("$N takes the damage meant for $n!", FALSE, victim, 0, GET_PROTECTOR(victim), TO_NOTVICT);

    victim = GET_PROTECTOR(victim);
  }

  /* If the victim is praying, interrupt them and apply a 2 round input lag. */
  if (affected_by_spell(victim, SKILL_PRAY)) {
    affect_from_char(victim, SKILL_PRAY);

    send_to_char("Your prayers are interrupted and you are slightly confused.\n\r", ch);

    WAIT_STATE(victim, PULSE_VIOLENCE * 2);
  }

  /* No damage dealt by a player to another character if they are sleeping, resting or sitting. */
  if (!IS_NPC(ch) &&
      (victim != ch) &&
      ((GET_POS(ch) == POSITION_SLEEPING) || (GET_POS(ch) == POSITION_RESTING) || (GET_POS(ch) == POSITION_SITTING))) {
    dmg = 0;
  }

  /* Adjust for immunities and resistances. */
  if (IS_NPC(victim)) {
    dmg = resist_damage(victim, dmg, attack_type, damage_type);
  }

  /* Force a player mount to flee if it inflicts damage to a player that isn't marked PLR_KILL or PLR_THIEF, except when damage is of the type DAM_NO_BLOCK_NO_FLEE.*/
  if (IS_NPC(ch) &&
      IS_MOUNT(ch) &&
      GET_RIDER(ch) &&
      !IS_NPC(GET_RIDER(ch)) &&
      !IS_NPC(victim) &&
      (!IS_SET(GET_PFLAG(victim), PLR_KILL) || !IS_SET(GET_PFLAG(victim), PLR_THIEF)) &&
      (damage_type != DAM_NO_BLOCK_NO_FLEE)) {
    do_flee(ch, "", CMD_FLEE);

    return 0;
  }

  /* Handle PvP */
  if ((victim != ch) &&
      !IS_NPC(ch) &&
      !IS_NPC(victim) &&
      !ROOM_ARENA(CHAR_REAL_ROOM(victim)) &&
      !ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) &&
      (!IS_SET(GET_PFLAG(victim), PLR_KILL) || !IS_SET(GET_PFLAG(victim), PLR_THIEF))) {
    /* Prevent players from PKing if PLR_NOKILL is turned on. */
    if (IS_SET(GET_PFLAG(ch), PLR_NOKILL)) {
      send_to_char("You can't attack other players.\n\r", ch);

      return 0;
    }
    /* Flag the attacker as a killer if PKing. */
    else {
      send_to_char("You are a killer!\n\r", ch);

      SET_BIT(GET_PFLAG(ch), PLR_KILL);

      snprintf(buf, sizeof(buf), "PLRINFO: %s just attacked %s. Killer flag set. (Room %d)",
               GET_NAME(ch), GET_NAME(victim), world[CHAR_REAL_ROOM(ch)].number);
      wizlog(buf, LEVEL_SUP, 4);
      log_s(buf);
    }
  }

  /* Prevent players from attacking player mounts if PLR_NOKILL is turned on. */
  if (!IS_NPC(ch) &&
      IS_SET(GET_PFLAG(ch), PLR_NOKILL) &&
      IS_NPC(victim) &&
      IS_MOUNT(victim) &&
      !IS_NPC(GET_RIDER(victim)) &&
      !ROOM_ARENA(CHAR_REAL_ROOM(victim)) &&
      !ROOM_CHAOTIC(CHAR_REAL_ROOM(victim))) {
    send_to_char("You can't attack player mounts.\n\r", ch);

    return 0;
  }

  /* Handle charmed NPCs that might be wielding an ANTI-MORTAL weapon. */
  if (IS_NPC(ch) &&
      IS_AFFECTED(ch, AFF_CHARM) &&
      GET_WEAPON(ch) &&
      IS_SET(OBJ_EXTRA_FLAGS(GET_WEAPON(ch)), ITEM_ANTI_MORTAL)) {
    send_to_char("Perhaps you shouldn't be using an ANTI-MORTAL weapon.\n\r", ch);

    return 0;
  }

  /* Prevent charmed NPCs from attacking players, except during Chaos. */
  if (IS_NPC(ch) &&
      IS_AFFECTED(ch, AFF_CHARM) &&
      !IS_NPC(victim) &&
      (GET_OPPONENT(victim) != ch) &&
      !CHAOSMODE) {
    send_to_char("You can't harm a player!\n\r", ch);

    return 0;
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
      if (!IS_NPC(ch) &&
          IS_NPC(victim) &&
          IS_SET(GET_ACT(victim), ACT_MEMORY) &&
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
      }

      /* NPCs have a chance to switch to a charmed NPC's master. */
      if (IS_NPC(ch) &&
          IS_NPC(victim) &&
          IS_AFFECTED(victim, AFF_CHARM) &&
          GET_MASTER(victim) &&
          (CHAR_REAL_ROOM(GET_MASTER(victim)) == CHAR_REAL_ROOM(ch)) &&
          chance(10)) {
        if (GET_OPPONENT(ch)) {
          stop_fighting(ch);
        }

        hit(ch, GET_MASTER(victim), TYPE_UNDEFINED);

        return 0;
      }

      /* NPCs have a chance to switch to a mount's rider. */
      if (IS_NPC(ch) &&
          IS_NPC(victim) &&
          GET_RIDER(victim) &&
          (CHAR_REAL_ROOM(GET_RIDER(victim)) == CHAR_REAL_ROOM(ch)) &&
          chance(10)) {
        if (GET_OPPONENT(ch)) {
          stop_fighting(ch);
        }

        hit(ch, GET_RIDER(victim), TYPE_UNDEFINED);

        return 0;
      }
    }
  }

  /* Stop victim following its master if the master attacks it.
     Unmount the attacker they attack their mount.*/
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

  /* Process damage improving skills/spells. */
  if (dmg && IS_WEAPON_ATTACK(attack_type)) {
    /* Fury */
    if (IS_AFFECTED(ch, AFF_FURY)) {
      /* Paladin Level 50 */
      if (IS_MORTAL(ch) &&
          (GET_CLASS(ch) == CLASS_PALADIN) &&
          (GET_LEVEL(ch) >= 50) &&
          (GET_ALIGNMENT(ch) > 500) &&
          !CHAOSMODE) {
        dmg = lround(dmg * ((double)(25 - ((1000 - GET_ALIGNMENT(ch)) / 100)) / 10.0));
      }
      /* Normal Fury */
      else {
        dmg *= 2;
      }
    }

    /* Rage */
    if (!IS_AFFECTED(ch, AFF_FURY) &&
        IS_AFFECTED2(ch, AFF2_RAGE)) {
      dmg = lround(dmg * 1.5);
    }

    /* Frenzy */
    if (!IS_AFFECTED(ch, AFF_FURY) &&
        !IS_AFFECTED2(ch, AFF2_RAGE) &&
        affected_by_spell(ch, SKILL_FRENZY)) {
      dmg = lround(dmg * 1.5);
    }
  }

  /* Physical Critical Hit */
  if (damage_type == DAM_PHYSICAL_CRITICAL) {
    dmg *= 2;
  }

  /* Maim */
  int maim_damage = 0;
  if (dmg && IS_SKILL_ATTACK(attack_type, damage_type)) {
    ENCH *maim_ench = get_enchantment_by_name(victim, "Savaged (Maim)");

    if (maim_ench) {
      maim_damage = resist_damage(victim, maim_ench->modifier, TYPE_UNDEFINED, DAM_PHYSICAL);

      if (maim_damage) {
        dmg += maim_damage;
      }
    }
  }

  /* Invulnerability */
  if (IS_AFFECTED(victim, AFF_INVUL)) {
    /* Reduce DAM_PHYSICAL damage less than 20 to 0. */
    if ((dmg < 20) &&
        IS_PHYSICAL_DAMAGE(damage_type) &&
        (!IS_AFFECTED(victim, AFF_FURY) || IS_NPC(victim))) {
      dmg = 0;
    }
  }

  /* Enchanter SC4: Ethereal Nature */
  if (IS_PHYSICAL_DAMAGE(damage_type) &&
      (affected_by_spell(victim, SPELL_ETHEREAL_NATURE) && (duration_of_spell(victim, SPELL_ETHEREAL_NATURE) == (CHAOSMODE ? 12 : 30)))) {
    dmg = 0;
  }

  int reflect = 0, max_reflect = 0;

  /* Juggernaut */
  if ((reflect <= 0) &&
      IS_WEAPON_ATTACK(attack_type) &&
      (IS_MORTAL(victim) && check_subclass(victim, SC_WARLORD, 4)) &&
      EQ(victim, WEAR_BODY) &&
      chance(10 + (GET_DEX_APP(victim) / 2))) {
    reflect = MIN(40, MAX(0, (-calc_ac(victim) + 100) / 10));

    act("$n is cut by the jagged spikes protruding from your armor.", FALSE, ch, 0, victim, TO_VICT);
    act("$n is cut by the jagged spikes protruding from $N's armor.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You are cut by the jagged spikes protruding from $N's armor!", FALSE, ch, 0, victim, TO_CHAR);
  }

  /* Defiler SC1: Blackmantle */
  if ((reflect <= 0) &&
      IS_WEAPON_ATTACK(attack_type) &&
      affected_by_spell(victim, SPELL_BLACKMANTLE)) {
    /* Blackmantle inflicts some damage even if the attacker misses. */
    if (dmg <= 0) {
      reflect = MIN(original_damage, GET_LEVEL(victim) / 5);

      act("$n is scorched by your mantle of darkness as $e gets too close.", FALSE, ch, 0, victim, TO_VICT);
      act("$n is scorched by $N's mantle of darkness as $e gets too close.", FALSE, ch, 0, victim, TO_NOTVICT);
      act("You are scorched by $N's mantle of darkness as you get too close!", FALSE, ch, 0, victim, TO_CHAR);
    }
    else {
      max_reflect = calc_hit_damage(victim, ch, EQ(victim, WIELD), 0, RND_MAX);

      if (IS_AFFECTED(victim, AFF_FURY)) {
        max_reflect *= 2;
      }
      else if (IS_AFFECTED2(ch, AFF2_RAGE)) {
        max_reflect = lround(max_reflect * 1.5);
      }

      /* NPCs can reflect more damage, to add an element of challenge. */
      if (IS_NPC(victim)) {
        max_reflect *= 4;
      }

      reflect = MIN(max_reflect, lround(dmg * 0.1));
      reflect = MAX(reflect, GET_LEVEL(victim) / 5);

      dmg = MAX(0, dmg - reflect);

      act("Your mantle of darkness reflects some of $n's damage back to $m.", FALSE, ch, 0, victim, TO_VICT);
      act("$N's mantle of darkness reflects some of $n's damage back to $m.", FALSE, ch, 0, victim, TO_NOTVICT);
      act("$N's mantle of darkness reflects some of your damage back to you!", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  /* Enchanter SC1: Blade Barrier */
  if ((reflect <= 0) &&
      (dmg > 0) &&
      IS_WEAPON_ATTACK(attack_type) &&
      affected_by_spell(victim, SPELL_BLADE_BARRIER)) {
    max_reflect = calc_hit_damage(victim, ch, EQ(victim, WIELD), 0, RND_MAX);

    if (IS_AFFECTED(victim, AFF_FURY)) {
      max_reflect *= 2;
    }
    else if (IS_AFFECTED2(ch, AFF2_RAGE)) {
      max_reflect = lround(max_reflect * 1.5);
    }

    /* NPCs can reflect more damage, to add an element of challenge. */
    if (IS_NPC(victim)) {
      max_reflect *= 4;
    }

    reflect = MIN(max_reflect, lround(dmg * 0.25));

    dmg = MAX(0, dmg - reflect);

    act("Some of $n's damage is reflected to $m.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("Some of your damage is reflected to you!\n\r", ch);
  }

  /* Vehemence penalty. */
  if (IS_MORTAL(victim) &&
      IS_SET(GET_TOGGLES(victim), TOG_VEHEMENCE)) {
    dmg = lround(dmg * 1.1);
  }

  /* Protection from Evil */
  if ((dmg > 0) &&
      IS_AFFECTED(victim, AFF_PROTECT_EVIL) &&
      IS_EVIL(ch) &&
      (!IS_EVIL(victim) || IS_NPC(victim))) {
    dmg = MAX(0, dmg - MIN(10, (GET_LEVEL(victim) + 10) - GET_LEVEL(ch)));
  }

  /* Protection from Good */
  if ((dmg > 0) &&
      IS_AFFECTED(victim, AFF_PROTECT_GOOD) &&
      IS_GOOD(ch) &&
      (!IS_GOOD(victim) || IS_NPC(victim))) {
    dmg = MAX(0, dmg - MIN(10, (GET_LEVEL(victim) + 10) - GET_LEVEL(ch)));
  }

  /* Sphere */
  if ((dmg > 0) &&
      IS_MAGICAL_DAMAGE(damage_type) &&
      IS_AFFECTED(victim, AFF_SPHERE) &&
      !breakthrough(ch, victim, attack_type, BT_SPHERE)) {
    /* Archmage SC5: Distortion allows 50% of DAM_MAGICAL damage through Sphere. */
    if (affected_by_spell(victim, SPELL_DISTORTION)) {
      dmg = lround(dmg * 0.5);
    }
    else {
      dmg = 0;
    }
  }

  /* Constitution Damage Reduction */
  if (IS_MORTAL(victim)) {
    dmg = lround(dmg * (1.0 - (GET_CON_DAM_REDUCT(victim) / 100.0)));
  }

  double damage_reduction = 0.0;

  /* Sanctuary */
  if (IS_AFFECTED(victim, AFF_SANCTUARY) &&
      !affected_by_spell(victim, SPELL_DISRUPT_SANCT)) {
    damage_reduction += 0.5;
  }

  /* Fortification */
  if (affected_by_spell(victim, SPELL_FORTIFICATION)) {
    damage_reduction += 0.15;
  }

  /* Damage Reduction Effects */
  dmg = lround(dmg * (1.0 - damage_reduction));

  /* Evasion */
  if (IS_PHYSICAL_DAMAGE(damage_type) &&
      IS_SET(GET_TOGGLES(victim), TOG_EVASION)) {
    dmg = lround(dmg * 0.75);
  }

  /* Bullwark */
  if (IS_MORTAL(victim) &&
      check_subclass(victim, SC_WARLORD, 5) &&
      IS_PHYSICAL_DAMAGE(damage_type) &&
      (calc_ac(victim) < -250)) {
    dmg = lround(dmg * (1.0 - (((abs(calc_ac(victim)) - 250.0) / 3.0) / 100.0)));
  }

  /* Combat Zen */
  if (IS_MORTAL(victim) &&
      check_subclass(victim, SC_RONIN, 3) &&
      (damage_type == DAM_POISON)) {
    dmg = lround(dmg * 0.5);
  }

  /* Limit total damage. */
  dmg = MAX(0, MIN(30000, dmg));

  /* Limit PvP damage in chaotic rooms and during Chaos. */
  if (ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) &&
      IS_MORTAL(ch) &&
      IS_MORTAL(victim)) {
    dmg = MIN(600, dmg);
  }

  /* Record damage for use later when printing damage text. */
  int dmg_text = dmg;

  /* Archmage SC2/Templar SC4: Orb of Protection */
  if ((dmg > 0) &&
      affected_by_spell(victim, SPELL_ORB_PROTECTION)) {
    int mana_shield = ((200 + dmg) - GET_HIT(victim)) / 2;

    if (mana_shield > 0) {
      mana_shield = MIN(mana_shield, GET_MANA(victim));
      GET_MANA(victim) -= mana_shield;
      dmg = MAX(0, dmg - (mana_shield * 2));
    }
  }

  /* No damage to immortals. */
  if (IS_IMMORTAL(victim)) {
    dmg = 0;
  }

  /* Shadow Wraith */
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
    GET_HIT(victim) = GET_MAX_HIT(victim);

    act("Your life has been restored by divine forces.", FALSE, victim, 0, 0, TO_CHAR);
    act("$n's life has been restored by divine forces.", FALSE, victim, 0, 0, TO_ROOM);

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

    dam_message(dmg_text, ch, victim, attack_type, shadow_damage);

    if (!IS_NPC(ch)) {
      ENDCOLOR(ch);
    }

    if (!IS_NPC(victim)) {
      ENDCOLOR(victim);
    }
  }
  /* Print messages for skills and spells. */
  else {
    for (int i = 0; i < MAX_MESSAGES; i++) {
      if (fight_messages[i].a_type == attack_type) {
        int nr = dice(1, fight_messages[i].number_of_attacks);
        struct message_type *messages = fight_messages[i].msg;

        for (int j = 1; (j < nr) && messages; j++) {
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

  /* Maim */
  if (maim_damage) {
    signal_char(victim, ch, MSG_DAMAGED, "SKILL_MAIM");
  }

  update_pos(victim);

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
      /* Handle MSG_DEAD in case it was intercepted. */
      if (signal_char(victim, ch, MSG_DEAD, "")) {
        return dmg;
      }

      /* Disembowel */
      if (attack_type == SKILL_DISEMBOWEL) {
        act("Guts splatter everywhere. Yuck!", FALSE, victim, 0, 0, TO_ROOM);
      }

      act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are dead! Sorry...\n\r", victim);
      break;

    /* >= POSITION SLEEPING */
    default:
      /* Handle player wimpy. */
      if (!IS_NPC(victim) &&
          !ROOM_CHAOTIC(CHAR_REAL_ROOM(victim)) &&
          (damage_type != DAM_NO_BLOCK_NO_FLEE) &&
          (GET_HIT(victim) < GET_WIMPY(victim)) &&
          (!affected_by_spell(victim, SPELL_ORB_PROTECTION) || (affected_by_spell(victim, SPELL_ORB_PROTECTION) && (GET_MANA(victim) < 1))) &&
          !affected_by_spell(victim, SPELL_DIVINE_INTERVENTION)) {
        do_flee(victim, "", CMD_FLEE);

        if (!SAME_ROOM(ch, victim)) return dmg;
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

          if (!SAME_ROOM(ch, victim)) return dmg;
        }
      }
      break;
  }

  /* Handle link-dead players. */
  if (!IS_NPC(victim) &&
      !GET_DESCRIPTOR(victim) &&
      (damage_type != DAM_NO_BLOCK_NO_FLEE)) {
    do_flee(victim, "", 0);

    if (!GET_OPPONENT(victim)) {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);

      GET_WAS_IN_ROOM(victim) = CHAR_REAL_ROOM(victim);
     
      char_from_room(victim);
      char_to_room(victim, 1);

      GET_POS(victim) = POSITION_STUNNED;
    }

    if (!SAME_ROOM(ch, victim)) return dmg;
  }

  /* Process reflected damage. */
  if (reflect && IS_ALIVE(victim)) {
    damage(victim, ch, reflect, TYPE_UNDEFINED, damage_type);
  }

  /* Stop fighting, as appropriate. */
  if (!GET_HIT(victim) && (GET_OPPONENT(ch) == victim) && (GET_POS(victim) < POSITION_STUNNED)) {
    if (IS_SET(GET_PFLAG(ch), PLR_VICIOUS)) {
      if (!IS_ALIVE(victim)) {
        stop_fighting(ch);
      }
      else if (IS_GOOD(ch)) {
        GET_ALIGNMENT(ch) -= 5;
      }
    }
    else {
      stop_fighting(ch);
    }
  }

  /* Stop the victim fighting if it's in a non-awake state. */
  if (!AWAKE(victim) && GET_OPPONENT(victim)) {
    stop_fighting(victim);
  }

  /* Process death. */
  if (GET_POS(victim) == POSITION_DEAD) {
    process_death(ch, victim);
  }

  return dmg;
}


int get_attack_type(CHAR *ch, OBJ *weapon) {
  if (!ch) return TYPE_HIT;

  if (!weapon && IS_NPC(ch) && (MOB_ATTACK_TYPE(ch) >= TYPE_HIT) && (MOB_ATTACK_TYPE(ch) <= TYPE_SLICE)) {
    return MOB_ATTACK_TYPE(ch);
  }

  if (weapon) {
    switch (OBJ_VALUE(weapon, 3)) {
      case 0:
      case 1:
      case 2:
        return TYPE_WHIP;
      case 3:
        return TYPE_SLASH;
      case 4:
        return TYPE_WHIP;
      case 5:
        return TYPE_STING;
      case 6:
        return TYPE_CRUSH;
      case 7:
        return TYPE_BLUDGEON;
      case 8:
        return TYPE_CLAW;
      case 9:
      case 10:
      case 11:
        return TYPE_PIERCE;
      case 12:
        return TYPE_HACK;
      case 13:
        return TYPE_CHOP;
      case 14:
        return TYPE_SLICE;
    }
  }

  return TYPE_HIT;
}

/*
Returns extra damage inflicted by a weapon, optionally based on an attacker's
attributes, and/or the attacker's weapon vs. a victim.

The 'ch' and 'victim' parameters can be NULL, but are required if the weapon
can only provide extra damage if one or both characters are involved in the
calculation.

The 'mode' parameter allows for the normal value, minimum value, maximum value,
or the average value to be returned.

Valid modes: RND_RND, RND_MIN, RND_MAX, RND_AVG
*/
int wpn_extra(OBJ *weapon, CHAR *ch, CHAR *victim, int mode) {
  if (!weapon) return 0;

  int attack_type = OBJ_VALUE0(weapon);

  if (attack_type < 21) return 0;

  int dam = 0;

  if (attack_type == WPN_CHAOTIC) {
    dam = (11 - dice_ex(1, 21, mode));
  }
  else if ((attack_type >= WPN_CLASS_FIRST) && (attack_type <= WPN_CLASS_LAST) && ch && ((attack_type - 300) == GET_CLASS(ch))) {
    dam = dice_ex(1, 5, mode);
  }
  else if (victim) {
    int victim_class = (int)GET_CLASS(victim);

    switch (attack_type) {
      case WPN_SLAY_EVIL_BEINGS:
        if (IS_EVIL(victim))
          dam = dice_ex(1, 5, mode);
        else if (IS_GOOD(victim))
          dam = -dice_ex(1, 5, mode);
        break;
      case WPN_SLAY_NEUTRAL_BEINGS:
        if (IS_NEUTRAL(victim))
          dam = dice_ex(1, 5, mode);
        break;
      case WPN_SLAY_GOOD_BEINGS:
        if (IS_GOOD(victim))
          dam = dice_ex(1, 5, mode);
        else if (IS_EVIL(victim))
          dam = -dice_ex(1, 5, mode);
        break;
      case CLASS_GREATER_UNDEAD:
        if (victim_class == CLASS_LESSER_UNDEAD)
          dam = dice_ex(1, 5, mode);
        break;
      case CLASS_GREATER_VAMPIRE:
        if (victim_class == CLASS_LESSER_VAMPIRE)
          dam = dice_ex(1, 5, mode);
        break;
      case CLASS_GREATER_DRAGON:
        if (victim_class == CLASS_LESSER_DRAGON)
          dam = dice_ex(1, 5, mode);
        break;
      case CLASS_GREATER_GIANT:
        if (victim_class == CLASS_LESSER_GIANT)
          dam = dice_ex(1, 5, mode);
        break;
      case CLASS_GREATER_LYCANTHROPE:
        if (victim_class == CLASS_LESSER_LYCANTHROPE)
          dam = dice_ex(1, 5, mode);
        break;
      case CLASS_GREATER_DEMON:
        if (victim_class == CLASS_LESSER_DEMON)
          dam = dice_ex(1, 5, mode);
        break;
      case CLASS_GREATER_ELEMENTAL:
        if (victim_class == CLASS_LESSER_ELEMENTAL)
          dam = dice_ex(1, 5, mode);
        break;
      case CLASS_GREATER_PLANAR:
        if (victim_class == CLASS_LESSER_PLANAR)
          dam = dice_ex(1, 5, mode);
        break;
      default:
        if (((victim_class >= CLASS_LICH) && (victim_class <= CLASS_STATUE) && (attack_type == victim_class)) ||
            ((victim_class <= WPN_SLAY_FIRST) && (victim_class <= WPN_SLAY_LAST) && (attack_type == (victim_class - 30)))) {
          dam = dice_ex(1, 5, mode);
        }
        break;
    }
  }

  return dam;
}

int calc_hitroll(CHAR *ch) {
  if (!ch) return 0;

  int hitroll = GET_HITROLL(ch);

  int str_bonus = str_app[MAX(0, MIN(STRENGTH_APPLY_INDEX(ch), OSTRENGTH_APPLY_INDEX(ch)))].tohit;

  /* Two-handed weapons grant a 150% strength modifier, unless a 2nd weapon is wielded (e.g. Sidearm). */
  if (IS_2H_WEAPON(GET_WEAPON(ch)) && !GET_WEAPON2(ch)) {
    str_bonus *= 1.5;
  }

  hitroll += str_bonus;

  if (!IS_NPC(ch)) {
    /* Close Combat: Hitroll Bonus */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_BANDIT, 4)) {
      hitroll += 2;
    }

    /* Evasion: Hitroll Penalty */
    if (IS_SET(GET_TOGGLES(ch), TOG_EVASION)) {
      hitroll -= 5;
    }

    /* Frenzy: Hitroll Penalty */
    if (affected_by_spell(ch, SKILL_FRENZY)) {
      hitroll -= 10;
    }

    /* Combat Zen: Blindness Hitroll Penalty Nullification */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_RONIN, 3)) {
      for (AFF *aff = ch->affected; aff; aff = aff->next) {
        if ((aff->type == SPELL_BLINDNESS) && (aff->location == APPLY_HITROLL) && (aff->modifier < 0)) {
          hitroll += abs(aff->modifier);
          break;
        }
      }
    }
  }

  return hitroll;
}

int calc_damroll(CHAR *ch) {
  if (!ch) return 0;

  int damroll = GET_DAMROLL(ch);

  int str_bonus = str_app[MAX(0, MIN(STRENGTH_APPLY_INDEX(ch), OSTRENGTH_APPLY_INDEX(ch)))].todam;

  /* Two-handed weapons grant a 150% strength modifier, unless a 2nd weapon is wielded (e.g. Sidearm). */
  if (IS_2H_WEAPON(GET_WEAPON(ch)) && !GET_WEAPON2(ch)) {
    str_bonus *= 1.5;
  }

  damroll += str_bonus;

  if (!IS_NPC(ch)) {
    /* Close Combat: Damroll Bonus */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_BANDIT, 4)) {
      damroll += 2;
    }
  }

  return damroll;
}

int calc_thaco(CHAR *ch) {
  if (!ch || !GET_LEVEL(ch) || IS_NPC(ch)) {
    return 20;
  }

  int thaco = 20;

  /* This new thaco calculation method levels the playing field for melee characters
     and maintains almost identical parity with the old table-based system.  Casters
     are naturally worse at hitting things in melee combat, as they were in the
     original system. */
  for (int i = 4; (i <= GET_LEVEL(ch)) && (i <= 18); i += 3) {
    thaco -= (((GET_CLASS(ch) == CLASS_MAGIC_USER) || (GET_CLASS(ch) == CLASS_CLERIC)) ? 1 : 2);
  }

  for (int i = 20; (i <= GET_LEVEL(ch)) && (i <= 36); i += 3) {
    thaco -= 1;
  }

  for (int i = 40; (i <= GET_LEVEL(ch)) && (i <= LEVEL_MORT); i += 5) {
    thaco -= 1;
  }

  return MAX(1, thaco);
}

int calc_ac(CHAR *ch) {
  if (!ch) return 100;

  int ac = GET_AC(ch);

  if (!IS_NPC(ch)) {
    /* Normal PC AC limit is -250. */
    int min_pc_ac = -250;

    /* Dexterity bonus only applies if awake. */
    if (AWAKE(ch)) {
      ac += dex_app[GET_DEX(ch)].defensive;
    }

    /* Vehemence: AC Penalty */
    if (IS_SET(GET_TOGGLES(ch), TOG_VEHEMENCE)) {
      ac += 30;
    }

    /* Close Combat: Max AC Adjustment */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_BANDIT, 4)) {
      ENCH *cc_ench = NULL;

      if (((cc_ench = get_enchantment_by_name(ch, "-10 AC (Close Combat)")) || (cc_ench = get_enchantment_by_name(ch, "-20 AC (Close Combat)"))) && (cc_ench->modifier < 0)) {
        min_pc_ac += cc_ench->modifier;
      }
    }

    /* Warlord: Max AC Adjustment */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 1)) {
      min_pc_ac -= GET_SC_LEVEL(ch) * 10;
    }

    /* Defend: AC Bonus and Max AC Adjustment */
    if (affected_by_spell(ch, SKILL_DEFEND) && !affected_by_spell(ch, SKILL_BERSERK)) {
      ac -= 100;
      min_pc_ac = -300;
    }

    /* Blur: AC Bonus */
    if (affected_by_spell(ch, SPELL_BLUR)) {
      ac -= (GET_LEVEL(ch) / 2);
    }

    /* Limit AC */
    ac = MAX(ac, min_pc_ac);
  }

  return ac;
}

int calc_position_damage(int position, int damage) {
  double multi = 1.0;

  switch (position) {
    case POSITION_SITTING:
      multi = 1.3333;
      break;
    case POSITION_RESTING:
      multi = 1.5;
      break;
    case POSITION_SLEEPING:
      multi = 1.6666;
      break;
    case POSITION_STUNNED:
      multi = 2;
      break;
    case POSITION_INCAP:
      multi = 2.3333;
      break;
    case POSITION_MORTALLYW:
      multi = 2.5;
      break;
  }

  return lround(damage * multi);
}

/*
Calculate the hit damage of a character.

The 'victim' parameter can be NULL, but obviously no extra damage that requires
a victim will be calculated.

The 'weapon' parameter can be NULL if the attacker is not wielding a weapon, in
which case bare-hand damage will be calculated.

The 'bonus' parameter is applied before resulting damage is calculated.

The 'mode' parameter allows for the normal value, minimum value, maximum value,
or the average value to be returned.

Valid modes: RND_RND, RND_MIN, RND_MAX, RND_AVG
*/
int calc_hit_damage(CHAR *ch, CHAR *victim, OBJ *weapon, int bonus, int mode) {
  int dam = 0;
  int num_dice = 0;
  int size_dice = 0;
  int extra = 0;

  if (!ch) return 0;

  if (!weapon) {
    /* NPC 'barehand' damage. */
    if (IS_NPC(ch)) {
      num_dice = ch->specials.damnodice;
      size_dice = ch->specials.damsizedice;
    }
    else {
      /* PC barehand damage. */
      num_dice = 1;
      size_dice = 2;

      if (GET_CLASS(ch) == CLASS_NINJA) {
        num_dice = (GET_LEVEL(ch) / 10) + 1;
        size_dice = 5;

        if (IS_MORTAL(ch) && check_subclass(ch, SC_RONIN, 3)) {
          num_dice += 1;
          size_dice += 1;
        }
      }
    }
  }
  else {
    /* Weapon damage. */
    num_dice = OBJ_VALUE1(weapon);
    size_dice = OBJ_VALUE2(weapon);

    extra = wpn_extra(weapon, ch, victim, mode);
  }

  /* Calculate weapon/barehand damage. */
  dam = dice_ex(num_dice, size_dice, mode) + extra + bonus;

  /* Add damroll. */
  dam += calc_damroll(ch);

  /* Rightousness */
  if (!IS_NPC(ch)) {
    if (victim && IS_GOOD(ch) && affected_by_spell(ch, SPELL_RIGHTEOUSNESS)) {
      if (IS_EVIL(victim)) {
        dam += dice_ex(1, 6, mode);
      }
      else if (IS_NEUTRAL(victim)) {
        dam += dice_ex(1, 4, mode);
      }
      else {
        dam += dice_ex(1, 2, mode);
      }
    }
  }

  /* Minimum damage is 1, unless modified below. */
  dam = MAX(1, (victim) ? calc_position_damage(GET_POS(victim), dam) : dam);

  if (!IS_NPC(ch)) {
    /* Defend Damage Penalty */
    if (affected_by_spell(ch, SKILL_DEFEND) && !affected_by_spell(ch, SKILL_BERSERK)) {
      dam = 1;
    }

    /* Clarity */
    if (affected_by_spell(ch, SPELL_CLARITY)) {
      dam = 0;
    }
  }

  return dam;
}

int stack_position(CHAR *ch, int target_position) {
  if (!ch || !target_position) return POSITION_DEAD;

  if (GET_POS(ch) <= POSITION_INCAP) {
    return GET_POS(ch);
  }

  if (GET_POS(ch) >= POSITION_FIGHTING) {
    return target_position;
  }

  if (GET_POS(ch) >= POSITION_RESTING) {
    if ((target_position <= POSITION_SITTING) && (target_position >= POSITION_RESTING)) {
      return POSITION_STUNNED;
    }
    else {
      return POSITION_INCAP;
    }
  }

  if (GET_POS(ch) >= POSITION_STUNNED) {
    return POSITION_INCAP;
  }

  return GET_POS(ch);
}

/* Note: For private use by try_hit(). */
int hit_success(int attack_roll, int attacker_thaco, int attacker_hitroll, int defender_ac) {
  /* Automatic success on an attack roll of 20, and automatic failure on an attack roll of 1. */
  if ((attack_roll == 20) || ((attack_roll != 1) && (attacker_thaco - attacker_hitroll - attack_roll) <= (defender_ac / 10))) {
    return HIT_SUCCESS;
  }

  return HIT_FAILURE;
}

int try_hit(CHAR *attacker, CHAR *defender) {
  if (!attacker || !defender) return FALSE;

  /* The following conditions always result in a hit. */
  if (!AWAKE(defender) ||
      IS_AFFECTED(defender, AFF_FURY) ||
      IS_SET(GET_TOGGLES(defender), TOG_HOSTILE) ||
      affected_by_spell(defender, SKILL_FRENZY)) {
    return HIT_SUCCESS;
  }

  /* Rage imposes a 50% chance of an automatic hit; Desecrate reduces this to 20%. */
  if (IS_AFFECTED2(defender, AFF2_RAGE) && chance((affected_by_spell(defender, SPELL_DESECRATE) ? 20 : 50))) {
    return HIT_SUCCESS;
  }

  int attack_roll = number(1, 20);
  int success = hit_success(attack_roll, calc_thaco(attacker), calc_hitroll(attacker), calc_ac(defender));

  /* Sento Kata */
  if (IS_MORTAL(attacker) && check_subclass(attacker, SC_RONIN, 5)) {
    /* Re-roll the attack if it was a 1. The re-roll can't critically hit. */
    if ((success == HIT_FAILURE) && (attack_roll == 1)) {
      attack_roll = number(1, 20);
      success = hit_success(attack_roll, calc_thaco(attacker), calc_hitroll(attacker), calc_ac(defender));
    }
    /* Critical hit if the initial attack was a success and was a 17+. */
    else if ((success == HIT_SUCCESS) && (attack_roll >= 17)) {
      success = HIT_CRITICAL;
    }
  }

  return success;
}


void print_avoidance_messages(CHAR *attacker, CHAR *defender, int skill) {
  if (!attacker || !defender || !skill) return;

  switch (skill) {
    case SKILL_DODGE:
      switch (number(1, 4)) {
        case 1:
          act("$n dodges $N's attack!", FALSE, defender, 0, attacker, TO_NOTVICT);
          act("$n dodges your attack!", FALSE, defender, 0, attacker, TO_VICT);
          act("You dodge $N's attack!", FALSE, defender, 0, attacker, TO_CHAR);
          break;
        case 2:
          act("$n rolls under $N's attack!", FALSE, defender, 0, attacker, TO_NOTVICT);
          act("$n rolls under your attack!", FALSE, defender, 0, attacker, TO_VICT);
          act("You roll under $N's attack!", FALSE, defender, 0, attacker, TO_CHAR);
          break;
        case 3:
          act("$n ducks under $N's mighty blow!", FALSE, defender, 0, attacker, TO_NOTVICT);
          act("$n ducks under your mighty blow!", FALSE, defender, 0, attacker, TO_VICT);
          act("You duck under $N's mighty blow!", FALSE, defender, 0, attacker, TO_CHAR);
          break;
        case 4:
          act("$n sidesteps $N's attack!", FALSE, defender, 0, attacker, TO_NOTVICT);
          act("$n sidesteps your attack!", FALSE, defender, 0, attacker, TO_VICT);
          act("You sidestep $N's attack!", FALSE, defender, 0, attacker, TO_CHAR);
          break;
      }
      break;
    case SKILL_PARRY:
      act("$n parries $N's attack!", FALSE, defender, 0, attacker, TO_NOTVICT);
      act("$n parries your attack!", FALSE, defender, 0, attacker, TO_VICT);
      act("You parry $N's attack!", FALSE, defender, 0, attacker, TO_CHAR);
      break;
    case SKILL_FEINT:
      act("$n feints, preventing $N's attack.  $n hits back!", FALSE, defender, 0, attacker, TO_NOTVICT);
      act("$n feints, preventing your attack.  $n hits back!", FALSE, defender, 0, attacker, TO_VICT);
      act("You feint, preventing $N's attack.  You hit back!", FALSE, defender, 0, attacker, TO_CHAR);
      break;
    case SKILL_RIPOSTE:
      act("$n deflects $N's attack.  $n hits back!", FALSE, defender, 0, attacker, TO_NOTVICT);
      act("$n deflects your attack.  $n hits back!", FALSE, defender, 0, attacker, TO_VICT);
      act("You deflect $N's attack.  You hit back!", FALSE, defender, 0, attacker, TO_CHAR);
      break;
  }
}


/* Returns the skill number of the skill that successfully avoided the attack or FALSE otherwise. */
int try_avoidance(CHAR *attacker, CHAR *defender) {
  if (!attacker || !defender) return FALSE;

  /* NPC Section */
  if (IS_NPC(defender)) {
    /* The following conditions always result in a failure. */
    if ((GET_POS(defender) <= POSITION_INCAP) ||
        IS_SET(GET_TOGGLES(defender), TOG_HOSTILE) ||
        affected_by_spell(defender, SKILL_FRENZY)) {
      return FALSE;
    }

    /* NPC dodge rate is always 20%. */
    if (IS_AFFECTED(defender, AFF_DODGE) && chance(20)) {
      print_avoidance_messages(attacker, defender, SKILL_DODGE);

      return SKILL_DODGE;
    }

    return FALSE;
  }

  /* PC Section */
  if (!IS_NPC(defender)) {
    /* The following conditions always result in a failure. */
    if ((GET_POS(defender) <= POSITION_STUNNED) ||
        IS_AFFECTED(defender, AFF_FURY) ||
        IS_SET(GET_TOGGLES(defender), TOG_HOSTILE) ||
        affected_by_spell(defender, SKILL_FRENZY)) {
      return FALSE;
    }

    /* Rage imposes a 50% chance of automatic failure; Desecrate reduces this to 20%. */
    if (IS_AFFECTED2(defender, AFF2_RAGE) && chance(affected_by_spell(defender, SPELL_DESECRATE) ? 20 : 50)) {
      return FALSE;
    }

    int skill = 0;

    switch (GET_CLASS(defender)) {
      case CLASS_THIEF:
      case CLASS_NINJA:
      case CLASS_NOMAD:
      case CLASS_BARD:
        skill = SKILL_DODGE;
        break;

      case CLASS_WARRIOR:
      case CLASS_PALADIN:
      case CLASS_AVATAR:
        skill = SKILL_PARRY;
        break;
    }

    switch (GET_SC(defender)) {
      case SC_DEFILER:
        if (IS_MORTAL(defender) && check_subclass(defender, SC_DEFILER, 3)) {
          skill = SKILL_FEINT;
        }
        break;

      case SC_MERCENARY:
        if (IS_MORTAL(defender) && check_subclass(defender, SC_MERCENARY, 3)) {
          skill = SKILL_RIPOSTE;
        }
        break;
    }

    /* AFF_DODGE overrides any selected skill. */
    if (IS_AFFECTED(defender, AFF_DODGE)) {
      skill = SKILL_DODGE;
    }

    /* If no skill or affect applies, return FALSE. */
    if (!skill) return FALSE;

    int defender_ac = calc_ac(defender);

    int check = 0;

    switch (skill) {
      case SKILL_PARRY:
        check = GET_LEARNED(defender, SKILL_PARRY);

        /* Dexterity */
        check += GET_DEX_APP(defender) * 5;

        /* Class */
        if (GET_CLASS(defender) == CLASS_WARRIOR) {
          check += GET_LEVEL(defender) / 10;
        }

        /* Bullwark */
        if (IS_MORTAL(defender) && check_subclass(defender, SC_WARLORD, 5) && (defender_ac < -250)) {
          check += (int)(700.0 * ((((double)defender_ac - 250.0) / 6.0) / 100.0));
        }

        if (number(1, 700) <= check) {
          print_avoidance_messages(attacker, defender, SKILL_PARRY);

          return SKILL_PARRY;
        }
        break;

      case SKILL_DODGE:
        check = GET_LEARNED(defender, SKILL_DODGE);

        /* If affected by AFF_DODGE and SKILL_MAX_PRAC is higher than the character's dodge skill, use SKILL_MAX_PRAC instead. */
        if (IS_AFFECTED(defender, AFF_DODGE) && (GET_LEARNED(defender, SKILL_DODGE) < SKILL_MAX_PRAC)) {
          check = SKILL_MAX_PRAC;
        }

        /* Dexterity */
        check += GET_DEX_APP(defender) * 5;

        /* Class */
        if ((GET_CLASS(defender) == CLASS_THIEF) || (GET_CLASS(defender) == CLASS_NOMAD)) {
          check += GET_LEVEL(defender) / 10;
        }

        /* Defend */
        if (affected_by_spell(defender, SKILL_DEFEND) && !affected_by_spell(defender, SKILL_BERSERK)) {
          check += 50;
        }

        /* Fade */
        if (IS_MORTAL(defender) && check_subclass(defender, SC_BANDIT, 2)) {
          check += GET_LEVEL(defender) * 1.5;
        }

        /* Blur */
        if (affected_by_spell(defender, SPELL_BLUR)) {
          check += GET_LEVEL(defender);
        }

        if (number(1, 700) <= check) {
          print_avoidance_messages(attacker, defender, SKILL_DODGE);

          /* Close Combat */
          if (IS_MORTAL(defender) && check_subclass(defender, SC_BANDIT, 4)) {
            ENCH *cc_ench = NULL;

            if (!(cc_ench = get_enchantment_by_name(defender, "-10 AC (Close Combat)")) && !(cc_ench = get_enchantment_by_name(defender, "-20 AC (Close Combat)"))) {
              enchantment_apply(defender, FALSE, "-10 AC (Close Combat)", SKILL_CLOSE_COMBAT, 3, ENCH_INTERVAL_ROUND, -10, APPLY_AC, 0, 0, 0);
            }
            else if ((cc_ench = get_enchantment_by_name(defender, "-10 AC (Close Combat)"))) {
              enchantment_remove(defender, cc_ench, FALSE);

              enchantment_apply(defender, FALSE, "-20 AC (Close Combat)", SKILL_CLOSE_COMBAT, 3, ENCH_INTERVAL_ROUND, -20, APPLY_AC, 0, 0, 0);
            }
          }

          return SKILL_DODGE;
        }
        break;

      case SKILL_FEINT:
        check = GET_LEARNED(defender, SKILL_FEINT);

        /* Dexterity */
        check += GET_DEX_APP(defender) * 5;

        if (number(1, 850) <= check) {
          /* Blackmantle */
          if (affected_by_spell(defender, SPELL_BLACKMANTLE)) {
            double reflect_multi = 1.0;

            if (IS_AFFECTED(defender, AFF_FURY)) {
              reflect_multi *= 2.0;
            }
            else if (IS_AFFECTED2(defender, AFF2_RAGE)) {
              reflect_multi *= 1.5;
            }

            /* NPCs can reflect more damage, to add an element of challenge. */
            if (IS_NPC(defender)) {
              reflect_multi *= 4;
            }

            act("$n is scorched by your mantle of darkness as $e gets too close.", FALSE, attacker, 0, defender, TO_VICT);
            act("$n is scorched by $N's mantle of darkness as $e gets too close.", FALSE, attacker, 0, defender, TO_NOTVICT);
            act("You are scorched by $N's mantle of darkness as you get too close!", FALSE, attacker, 0, defender, TO_CHAR);

            damage(defender, attacker, ((GET_LEVEL(defender) / 5) * reflect_multi), TYPE_UNDEFINED, DAM_MAGICAL);
          }

          print_avoidance_messages(attacker, defender, SKILL_FEINT);

          /* Attack back. */
          hit(defender, attacker, SKILL_FEINT);

          return SKILL_FEINT;
        }
        break;

      case SKILL_RIPOSTE:
        check = GET_LEARNED(defender, SKILL_RIPOSTE);

        /* Dexterity modfier */
        check += GET_DEX_APP(defender) * 5;

        if (number(1, 850) <= check) {
          return SKILL_RIPOSTE;
        }
        break;
    }

    return FALSE;
  }

  return FALSE;
}


/* Returns TRUE if the hit succeeded, or FALSE if it was avoided or otherwise missed. */
bool perform_hit(CHAR *attacker, CHAR *defender, int type, int hit_num) {
  if (!attacker || !defender || !SAME_ROOM(attacker, defender)) return FALSE;

  /* Sanity check. */
  hit_num = MAX(1, hit_num);

  int avoidance_skill = FALSE;

  /* Check avoidance skills and affects. */
  if (IS_NPC(defender) || (!IS_NPC(defender) && (hit_num == 1))) {
    if ((type != SKILL_ASSASSINATE) && (type != SKILL_BACKSTAB)) {
      /* Check avoidance skills and record which skill (if any) avoided the attack. */
      avoidance_skill = try_avoidance(attacker, defender);

      /* Check if attacker and defender are still in the same room after avoidance. If not, someone died/fled/etc. */
      if (!SAME_ROOM(attacker, defender)) return FALSE;

      bool hit_avoided = FALSE;

      switch (avoidance_skill) {
        case SKILL_PARRY:
        case SKILL_DODGE:
        case SKILL_FEINT:
          hit_avoided = TRUE; // Attack was avoided entirely.
          break;
        case SKILL_RIPOSTE:
          hit_avoided = FALSE; // Attack continues as normal, but some damage is deflected.
          break;
      }

      /* Some classes have a chance to hit again, if the attack was avoided. */
      if (hit_avoided) {
        if (IS_MORTAL(attacker) && check_subclass(attacker, SC_MERCENARY, 3)) {
          if ((number(1, 1000) - (GET_DEX_APP(attacker) * 5)) <= GET_LEARNED(attacker, SKILL_RIPOSTE)) {
            act("As $N avoids your attack, you riposte and strike back!", FALSE, attacker, 0, defender, TO_CHAR);
            act("As you avoid $n's attack, $e ripostes and strikes back!", FALSE, attacker, 0, defender, TO_VICT);
            act("As $N avoids $n's attack, $e ripostes and strikes back!", FALSE, attacker, 0, defender, TO_NOTVICT);

            hit(attacker, defender, TYPE_UNDEFINED); // A normal hit (can dual/triple).
          }
        }

        if (IS_MORTAL(attacker) && check_subclass(attacker, SC_DEFILER, 3)) {
          if ((number(1, 1000) - (GET_DEX_APP(attacker) * 5)) <= GET_LEARNED(attacker, SKILL_FEINT)) {
            act("You feint after $N avoids your attack and you attack once again!", FALSE, attacker, 0, defender, TO_CHAR);
            act("$n feints after you avoid $s attack and $e attacks once again!", FALSE, attacker, 0, defender, TO_VICT);
            act("$n feints after $N avoids $s attack and $e attacks once again!", FALSE, attacker, 0, defender, TO_NOTVICT);

            hit(attacker, defender, SKILL_FEINT); // A feint hit (doubled).
          }
        }
      }

      /* If the hit was avoided, return the appropriate result. */
      if (hit_avoided) {
        /* Combat Zen */
        if (IS_MORTAL(attacker) && check_subclass(attacker, SC_RONIN, 3)) {
          return TRUE; // Override avoidance result to continue with further attacks.
        }

        return FALSE;
      }
    }
  }

  /* Get the weapon involved for use later on. */
  OBJ *weapon = (type != TYPE_WEAPON2) ? GET_WEAPON(attacker) : GET_WEAPON2(attacker);

  /* Get the attack type for use later on. */
  int attack_type = get_attack_type(attacker, weapon);

  /* Sento Kata */
  if (IS_MORTAL(attacker) && check_subclass(attacker, SC_RONIN, 5)) {
    if (is_immune(defender, attack_type, 0) || is_resistant(defender, attack_type, 0)) {
      switch (attack_type) {
        case TYPE_HIT:
          if (!weapon) {
            if (!is_immune(defender, TYPE_CLAW, 0) && !is_resistant(defender, TYPE_CLAW, 0)) {
              attack_type = TYPE_CLAW;
            }
            else if (!is_immune(defender, TYPE_WHIP, 0) && !is_resistant(defender, TYPE_WHIP, 0)) {
              attack_type = TYPE_WHIP;
            }
          }
          break;

        case TYPE_CHOP:
        case TYPE_HACK:
          if (!is_immune(defender, TYPE_SLASH, 0) && !is_resistant(defender, TYPE_SLASH, 0)) {
            attack_type = TYPE_SLASH;
          }
          else if (!is_immune(defender, TYPE_CRUSH, 0) && !is_resistant(defender, TYPE_CRUSH, 0)) {
            attack_type = TYPE_CRUSH;
          }
          break;

        case TYPE_PIERCE:
          if (!is_immune(defender, TYPE_SLICE, 0) && !is_resistant(defender, TYPE_SLICE, 0)) {
            attack_type = TYPE_SLICE;
          }
          else if (!is_immune(defender, TYPE_HACK, 0) && !is_resistant(defender, TYPE_HACK, 0)) {
            attack_type = TYPE_HACK;
          }
          break;

        case TYPE_SLASH:
        case TYPE_SLICE:
          if (!is_immune(defender, TYPE_PIERCE, 0) && !is_resistant(defender, TYPE_PIERCE, 0)) {
            attack_type = TYPE_PIERCE;
          }
          else if (!is_immune(defender, TYPE_BLUDGEON, 0) && !is_resistant(defender, TYPE_BLUDGEON, 0)) {
            attack_type = TYPE_BLUDGEON;
          }
          break;
      }
    }
  }

  if (type == SPELL_SHADOW_WRAITH) {
    attack_type = TYPE_SHADOW;
  }

  int hit_success = try_hit(attacker, defender);

  if (hit_success == HIT_FAILURE) {
    switch (type) {
      case SKILL_ASSASSINATE:
      case SKILL_BACKSTAB:
        damage(attacker, defender, 0, SKILL_BACKSTAB, DAM_NO_BLOCK);
      break;

      case SKILL_CIRCLE:
        act("$n missteps as $e lunges toward $N, missing $S back completely.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("You see $n misstep out of the corner of your eye, stumbling past your flank.", FALSE, attacker, 0, defender, TO_VICT);
        act("You misstep while lunging toward $N, missing $S back completely.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_CIRCLE, DAM_NO_BLOCK);
      break;

      case SKILL_AMBUSH:
        act("$n tries to ambush $N, but misses.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to ambush you, but misses.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to ambush $N, but miss.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_AMBUSH, DAM_NO_BLOCK);
      break;

      case SKILL_FLANK:
        act("$n tries to flank $N, but misses.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to flank you, but misses.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to flank $N, but miss.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_FLANK, DAM_NO_BLOCK);
      break;

      case SKILL_ASSAULT:
        act("$n tries to assault $N, but misses.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to assault you, but misses.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to assault $N, but miss.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_ASSAULT, DAM_NO_BLOCK);
      break;

      case SKILL_BLITZ:
        act("$n tries to blitz $N, but misses.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to blitz you, but misses.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to blitz $N, but miss.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_BLITZ, DAM_NO_BLOCK);
      break;

      case SKILL_LUNGE:
        act("$n tries to lunge at $N, but misses.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to lunge at you, but misses.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to lunge at $N, but miss.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_LUNGE, DAM_NO_BLOCK);
      break;

      default:
        damage(attacker, defender, 0, attack_type, DAM_NO_BLOCK);
      break;
    }

    /* Combat Zen */
    if (IS_MORTAL(attacker) && check_subclass(attacker, SC_RONIN, 3)) {
      return TRUE; // Override miss result to continue with further attacks.
    }

    return FALSE;
  }

  int dam = calc_hit_damage(attacker, defender, weapon, 0, RND_RND);

  switch (type) {
    case SKILL_BACKSTAB:
    case SKILL_ASSASSINATE:
    {
      if (!breakthrough(attacker, defender, type, BT_INVUL)) {
        damage(attacker, defender, 0, SKILL_BACKSTAB, DAM_NO_BLOCK);
      }
      else {
        bool special_message = FALSE;

        /* Impair */
        if (SAME_ROOM(attacker, defender) &&
            (IS_MORTAL(attacker) && check_subclass(attacker, SC_BANDIT, 1)) &&
            !IS_IMMUNE(defender, IMMUNE_PARALYSIS) &&
            chance(50 + GET_DEX_APP(attacker))) {
          enchantment_apply(defender, FALSE, "Paralyzed (Impair)", SKILL_IMPAIR, (ROOM_CHAOTIC(CHAR_REAL_ROOM(attacker)) ? 0 : 2), 0, 0, 0, AFF_PARALYSIS, 0, 0);

          special_message = TRUE;
        }

        damage(attacker, defender, (dam * backstab_mult[GET_LEVEL(attacker)]), SKILL_BACKSTAB, DAM_PHYSICAL);

        if ((CHAR_REAL_ROOM(defender) != NOWHERE) && special_message) {
          act("$n nearly severs $N's spine with $s backstab, temporarily paralyzing $M.", FALSE, attacker, 0, defender, TO_NOTVICT);
          act("$n nearly severs your spine with $s backstab, temporarily paralyzing you.", FALSE, attacker, 0, defender, TO_VICT);
          act("You nearly sever $N's spine with your backstab, temporarily paralyzing $M.", FALSE, attacker, 0, defender, TO_CHAR);
        }
      }
      break;
    }

    case SKILL_CIRCLE:
    {
      if (!breakthrough(attacker, defender, type, BT_INVUL)) {
        act("$n's weapon makes contact with $N, but slides harmlessly off $S back.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n's weapon slides harmlessly off of your back.", FALSE, attacker, 0, defender, TO_VICT);
        act("Your weapon makes contact with $N, but slides harmlessly off $S back.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_CIRCLE, DAM_NO_BLOCK);
      }
      else {
        bool special_message = FALSE;

        /* Impair */
        if (SAME_ROOM(attacker, defender) &&
            (IS_MORTAL(attacker) && check_subclass(attacker, SC_BANDIT, 1)) &&
            chance(30 + GET_DEX_APP(attacker))) {
          enchantment_apply(defender, FALSE, "+30 AC (Impair)", SKILL_IMPAIR, 1, 0, 30, APPLY_AC, 0, 0, 0);
          enchantment_apply(defender, FALSE, "-2 Hitroll (Impair)", SKILL_IMPAIR, 1, 0, -2, APPLY_HITROLL, 0, 0, 0);

          special_message = TRUE;
        }

        act("$n plunges $p deep into $N's back.", FALSE, attacker, weapon, defender, TO_NOTVICT);
        act("$n plunges $p deep into your back.", FALSE, attacker, weapon, defender, TO_VICT);
        act("You plunge $p deep into $N's back.", FALSE, attacker, weapon, defender, TO_CHAR);

        damage(attacker, defender, (dam * circle_mult[GET_LEVEL(attacker)]), SKILL_CIRCLE, DAM_PHYSICAL);

        if ((CHAR_REAL_ROOM(defender) != NOWHERE) && special_message) {
          act("You strike a nerve in $N's back with your attack, severely weakening $M.", FALSE, attacker, 0, defender, TO_CHAR);
          act("$n strikes a nerve in your back with $s attack, severely weakening you.", FALSE, attacker, 0, defender, TO_VICT);
          act("$n strikes a nerve in $N's back with $s attack, severely weakening $M.", FALSE, attacker, 0, defender, TO_NOTVICT);
        }

        /* Twist */
        if (SAME_ROOM(attacker, defender) &&
            (GET_CLASS(attacker) == CLASS_THIEF) &&
            (GET_LEVEL(attacker) >= 45) &&
            ((number(1, 131) - GET_DEX_APP(attacker) - (IS_SET(GET_TOGGLES(attacker), TOG_VEHEMENCE) ? (5 + (GET_DEX_APP(attacker) / 2)) : 0)) <= GET_LEARNED(attacker, SKILL_TWIST))) {
          act("You twist your weapon in the flesh of $N.", FALSE, attacker, 0, defender, TO_CHAR);
          act("You writhe in pain as $n twists his weapon in your back.", FALSE, attacker, 0, defender, TO_VICT);
          act("$n gruesomely twists $s weapon in the flesh of $N.", TRUE, attacker, 0, defender, TO_NOTVICT);

          damage(attacker, defender, 250, SKILL_TWIST, DAM_PHYSICAL);
        }

        /* Bathed in Blood */
        if (!IS_NPC(attacker) && check_subclass(attacker, SC_DEFILER, 5) && chance(20)) {
          send_to_room("Blood sprays across the room, staining the surroundings dark crimson.\n\r", CHAR_REAL_ROOM(attacker));

          ROOM_BLOOD(CHAR_REAL_ROOM(attacker)) = MIN(ROOM_BLOOD(CHAR_REAL_ROOM(attacker)) + 1, 10);
        }
      }
      break;
    }

    case SKILL_AMBUSH:
      if (IS_AFFECTED(defender, AFF_INVUL) && !breakthrough(attacker, defender, type, BT_INVUL)) {
        act("$n tries to ambush $N, but fails.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to ambush you, but fails.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to ambush $N, but fail.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_AMBUSH, DAM_NO_BLOCK);
      }
      else {
        act("$N fell into a cunning ambush by $n.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("You fell into an ambush by $n.", FALSE, attacker, 0, defender, TO_VICT);
        act("You managed to take $N completely by surprise.", FALSE, attacker, 0, defender, TO_CHAR);

        int multi = ambush_mult[GET_LEVEL(attacker)];

        if ((world[CHAR_REAL_ROOM(attacker)].sector_type == SECT_FOREST) ||
            (world[CHAR_REAL_ROOM(attacker)].sector_type == SECT_MOUNTAIN)) {
          multi += 1;
        }

        if ((world[CHAR_REAL_ROOM(attacker)].sector_type == SECT_INSIDE) ||
            (world[CHAR_REAL_ROOM(attacker)].sector_type == SECT_CITY)) {
          multi -= 1;
        }

        damage(attacker, defender, (dam * MAX(multi, 1)), SKILL_AMBUSH, DAM_PHYSICAL);
      }
      break;

    case SKILL_FLANK:
      if (IS_AFFECTED(defender, AFF_INVUL) && !breakthrough(attacker, defender, type, BT_INVUL)) {
        act("$n tries to flank $N, but fails.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to flank you, but fails.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to flank $N, but fail.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_FLANK, DAM_NO_BLOCK);
      }
      else {
        act("$n quickly moves to $N's side and hits $M with a devastating blow.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n quickly moves to your side and hits you with a devastating blow.", FALSE, attacker, 0, defender, TO_VICT);
        act("You quickly move to $N's side and hit $M with a devastating blow.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, (dam * (GET_LEVEL(attacker) / 10)), SKILL_FLANK, DAM_PHYSICAL);
      }
      break;

    case SKILL_ASSAULT:
      if (IS_AFFECTED(defender, AFF_INVUL) && !breakthrough(attacker, defender, type, BT_INVUL)) {
        act("$n tries to assault $N, but fails.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to assault you, but fails.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to assault $N, but fail.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_ASSAULT, DAM_NO_BLOCK);
      }
      else {
        act("$n attacked $N suddenly without $M noticing.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("You are attacked suddenly by $n.", FALSE, attacker, 0, defender, TO_VICT);
        act("You attacked $N suddenly without $M noticing.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, (dam * assault_mult[GET_LEVEL(attacker)]), SKILL_ASSAULT, DAM_PHYSICAL);
      }
      break;

    case SKILL_BLITZ:
      if (IS_AFFECTED(defender, AFF_INVUL) && !breakthrough(attacker, defender, type, BT_INVUL)) {
        act("$n tries to blitz $N, but fails.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to blitz you, but fails.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to blitz $N, but fail.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_BLITZ, DAM_NO_BLOCK);
      }
      else {
        act("$n charges into the fray, wildly swinging $s weapon at $N.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n charges into the fray, wildly swinging $s weapon at you.", FALSE, attacker, 0, defender, TO_VICT);
        act("You charge into the fray, wildly swinging your weapon at $N.", FALSE, attacker, 0, defender, TO_CHAR);

        if (GET_WEAPON(defender) && chance(20)) {
          OBJ *disarm = unequip_char(defender, WIELD);

          act("$n's $p is knocked from $s grasp.", FALSE, defender, disarm, 0, TO_ROOM);
          act("Your $p is knocked from your grasp.", FALSE, defender, disarm, 0, TO_CHAR);

          if (IS_SET(CHAR_ROOM_FLAGS(attacker), CHAOTIC)) {
            obj_to_char(disarm, defender);
          }
          else {
            OBJ_LOG(disarm) = TRUE;
            obj_to_room(disarm, CHAR_REAL_ROOM(defender));

            log_f("WIZINFO: %s disarms %s's %s (Room %d)", GET_NAME(attacker), GET_NAME(defender), OBJ_NAME(disarm), V_ROOM(defender));
          }

          save_char(defender, NOWHERE);
        }

        damage(attacker, defender, lround(dam * 1.5), SKILL_BLITZ, DAM_PHYSICAL);
      }
      break;

    case SKILL_LUNGE:
      if (IS_AFFECTED(defender, AFF_INVUL) && !breakthrough(attacker, defender, type, BT_INVUL)) {
        act("$n tries to lunge at $N, but fails.", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n tries to lunge at you, but fails.", FALSE, attacker, 0, defender, TO_VICT);
        act("You try to lunge at $N, but fail.", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, 0, SKILL_LUNGE, DAM_NO_BLOCK);
      }
      else {
        act("$n lunges forward with $s weapon, impacting $N's hide!", FALSE, attacker, 0, defender, TO_NOTVICT);
        act("$n lunges forward with $s weapon, impacting your hide!", FALSE, attacker, 0, defender, TO_VICT);
        act("You lunge forward with your weapon, impacting $N's hide!", FALSE, attacker, 0, defender, TO_CHAR);

        damage(attacker, defender, lround(assault_mult[GET_LEVEL(attacker)] * 1.25), SKILL_LUNGE, DAM_PHYSICAL);
      }
      break;

    case SKILL_FEINT:
      /* Feint attack back. */
      damage(attacker, defender, (dam * 2), attack_type, DAM_PHYSICAL);
      break;

    /* Standard Melee Attacks */
    default:
      /* Riposte damage reduction. */
      if (avoidance_skill == SKILL_RIPOSTE) {
        dam = lround(dam * 0.6);
      }

      dam = damage(attacker, defender, dam, attack_type, (hit_success == HIT_CRITICAL) ? DAM_PHYSICAL_CRITICAL : DAM_PHYSICAL);

      /* Riposte attack back. */
      if (SAME_ROOM(attacker, defender) && (avoidance_skill == SKILL_RIPOSTE)) {
        print_avoidance_messages(attacker, defender, SKILL_RIPOSTE);

        hit(defender, attacker, TYPE_UNDEFINED);
      }

      /* Rightousness */
      if (dam && (hit_num == 1) && IS_GOOD(attacker) && affected_by_spell(attacker, SPELL_RIGHTEOUSNESS)) {
        GET_MANA(attacker) = MIN(GET_MAX_MANA(attacker), GET_MANA(attacker) + dice(1, 3));
      }
      break;
  }

  return TRUE;
}


void hit(CHAR *ch, CHAR *victim, int type) {
  char buf[MSL];

  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  if (ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  /* PC Checks */
  if (!IS_NPC(ch) && (ch != victim)) {
    if (IS_MOUNT(victim) &&
        IS_SET(GET_PFLAG(ch), PLR_NOKILL) &&
        !ROOM_ARENA(CHAR_REAL_ROOM(ch)) &&
        !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
      send_to_char("You can't attack mounts.\n\r", ch);

      return;
    }

    /* PvP Checks */
    if (!IS_NPC(victim)) {
      if ((!IS_SET(GET_PFLAG(victim), PLR_KILL) || !IS_SET(GET_PFLAG(victim), PLR_THIEF)) &&
          IS_SET(GET_PFLAG(ch), PLR_NOKILL) &&
          !ROOM_ARENA(CHAR_REAL_ROOM(ch)) &&
          !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
        send_to_char("You can't attack other players.\n\r", ch);

        return;
      }

      if ((!IS_SET(GET_PFLAG(victim), PLR_KILL) || !IS_SET(GET_PFLAG(victim), PLR_THIEF)) &&
          !IS_SET(GET_PFLAG(ch), PLR_KILL) &&
          !ROOM_ARENA(CHAR_REAL_ROOM(ch)) &&
          !ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) {
        send_to_char("You are a killer!\n\r", ch);

        SET_BIT(GET_PFLAG(ch), PLR_KILL);

        snprintf(buf, sizeof(buf), "PLRINFO: %s just attacked %s in room %d. Killer flag set.",
                 GET_NAME(ch), GET_NAME(victim), V_ROOM(ch));

        wizlog(buf, LEVEL_SUP, 4);

        log_f("%s", buf);
      }
    }
  }

  /* Ensure ch engages victim if not already fighting someone. */
  if (!GET_OPPONENT(ch)) {
    set_fighting(ch, victim);
  }

  /* Ensure victim engages ch if not already fighting someone. */
  if (!GET_OPPONENT(victim)) {
    set_fighting(victim, ch);
  }

  /* Perform the attack, returning if it is avoided or misses. */
  if (!perform_hit(ch, victim, type, 1)) return;

  /* Shadow Wraith */
  if (affected_by_spell(ch, SPELL_SHADOW_WRAITH)) {
    int num_shadows = (MAX(1, (duration_of_spell(ch, SPELL_SHADOW_WRAITH) - 1)) / 10) + 1;

    for (int i = 0; i < num_shadows; i++) {
      perform_hit(ch, victim, SPELL_SHADOW_WRAITH, 1);
    }
  }

  /* The following attack types never result in a multi hit. */
  switch (type) {
    case SKILL_ASSASSINATE:
    case SKILL_BACKSTAB:
    case SKILL_AMBUSH:
    case SKILL_CIRCLE:
      return;
  }

  /* Haste */
  /* This adds the chance for an additional "hit" and performs it before any
     additional hit skill checks, which may fail at any point in the chain. */
  if (affected_by_spell(ch, SPELL_HASTE) && chance(30 + GET_DEX_APP(ch))) {
    /* Force main weapon for Haste attacks. */
    if (!perform_hit(ch, victim, TYPE_UNDEFINED, 1)) return;
  }

  /* PC Ninja 2nd Hit */
  if (!IS_NPC(ch) && (GET_CLASS(ch) == CLASS_NINJA)) {
    /* Force the appropriate weapon for Ninja 2nd attack. */
    dhit(ch, victim, (IS_2H_WEAPON(EQ(ch, WIELD)) ? TYPE_UNDEFINED : TYPE_WEAPON2));

    return;
  }

  /* NPC Dual */
  if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_DUAL) && !number(0, 2)) {
    /* Force main weapon for NPC attacks. */
    dhit(ch, victim, TYPE_UNDEFINED);

    return;
  }

  /* Coerce attack type as needed. */
  if ((type != TYPE_UNDEFINED) && (type != TYPE_WEAPON2)) {
    type = TYPE_UNDEFINED;
  }

  /* PC Dual */
  if (!IS_NPC(ch) && (IS_AFFECTED(ch, AFF_DUAL) || (GET_CLASS(ch) == CLASS_WARRIOR || GET_CLASS(ch) == CLASS_AVATAR || GET_CLASS(ch) == CLASS_COMMANDO))) {
    int skill = IS_AFFECTED(ch, AFF_DUAL) ? MAX(SKILL_MAX_PRAC, GET_LEARNED(ch, SKILL_DUAL)) : GET_LEARNED(ch, SKILL_DUAL);
    int bonus = GET_DEX_APP(ch) * 5;
    int check = 298;

    /* Juggernaut */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 4)) {
      check = 259;
    }
    /* Hostile/Rush */
    else if (IS_SET(GET_TOGGLES(ch), TOG_HOSTILE) || affected_by_spell(ch, SPELL_RUSH)) {
      check = 213;
    }

    if (number(1, check) < (((skill + 150) / 2) + bonus)) {
      dhit(ch, victim, type);

      return;
    }
  }

  /* Berserk */
  if (affected_by_spell(ch, SKILL_BERSERK)) {
    int bonus = (!IS_NPC(ch) ? GET_DEX_APP(ch) : 0);
    int percent = 40;

    if (chance(percent + bonus)) {
      dhit(ch, victim, type);

      return;
    }
  }

  /* Frenzy */
  if (affected_by_spell(ch, SKILL_FRENZY)) {
    int bonus = (!IS_NPC(ch) ? GET_DEX_APP(ch) : 0);
    int percent = 10;

    if (chance(percent + bonus)) {
      dhit(ch, victim, type);

      return;
    }
  }
}

void dhit(CHAR *ch, CHAR *victim, int type) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  /* Perform the attack, returning if it is avoided or misses. */
  if (!perform_hit(ch, victim, type, 2)) return;

  /* Mystic Swiftness */
  if (affected_by_spell(ch, SPELL_MYSTIC_SWIFTNESS) && chance(50 + (GET_DEX_APP(ch) * 2.5))) {
    /* Force main weapon for Mystic Swiftness bonus attack. */
    thit(ch, victim, TYPE_UNDEFINED);

    return;
  }

  /* NPC Triple */
  if (IS_NPC(ch) && IS_AFFECTED2(ch, AFF2_TRIPLE) && !number(0, 2)) {
    /* Force main weapon for NPC attacks. */
    thit(ch, victim, TYPE_UNDEFINED);

    return;
  }

  /* Coerce attack type as needed. */
  if ((type != TYPE_UNDEFINED) && (type != TYPE_WEAPON2)) {
    type = TYPE_UNDEFINED;
  }

  /* PC Triple */
  if (!IS_NPC(ch) && (IS_AFFECTED2(ch, AFF2_TRIPLE) || ((GET_CLASS(ch) == CLASS_WARRIOR || GET_CLASS(ch) == CLASS_AVATAR || GET_CLASS(ch) == CLASS_COMMANDO) && GET_LEVEL(ch) >= 20))) {
    int skill = IS_AFFECTED2(ch, AFF2_TRIPLE) ? MAX(SKILL_MAX_PRAC, GET_LEARNED(ch, SKILL_TRIPLE)) : GET_LEARNED(ch, SKILL_TRIPLE);
    int bonus = GET_DEX_APP(ch) * 5;
    int check = 283;

    /* Juggernaut */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 4)) {
      check = 246;
    }
    /* Hostile/Rush */
    else if (IS_SET(GET_TOGGLES(ch), TOG_HOSTILE) || affected_by_spell(ch, SPELL_RUSH)) {
      check = 202;
    }

    if (number(1, check) < (((skill + 150) / 2) + bonus)) {
      thit(ch, victim, type);

      return;
    }
  }
}

void thit(CHAR *ch, CHAR *victim, int type) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  /* Perform the attack, returning if it is avoided or misses. */
  if (!perform_hit(ch, victim, type, 3)) return;

  /* NPC Quad */
  if (IS_NPC(ch) && IS_AFFECTED2(ch, AFF2_QUAD) && !number(0, 2)) {
    /* Force main weapon for NPC attacks. */
    qhit(ch, victim, TYPE_UNDEFINED);

    return;
  }

  /* Coerce attack type as needed. */
  if ((type != TYPE_UNDEFINED) && (type != TYPE_WEAPON2)) {
    type = TYPE_UNDEFINED;
  }

  /* PC Quad */
  if (!IS_NPC(ch) && (IS_AFFECTED2(ch, AFF2_QUAD) || ((GET_CLASS(ch) == CLASS_WARRIOR || GET_CLASS(ch) == CLASS_AVATAR) && GET_LEVEL(ch) >= 50))) {
    int skill = IS_AFFECTED2(ch, AFF2_QUAD) ? MAX(SKILL_MAX_PRAC, GET_LEARNED(ch, SKILL_QUAD)) : GET_LEARNED(ch, SKILL_QUAD);
    int bonus = GET_DEX_APP(ch) * 5;
    int check = 269;

    /* Juggernaut */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 4)) {
      check = 234;
    }
    /* Hostile */
    else if (IS_SET(GET_TOGGLES(ch), TOG_HOSTILE)) {
      check = 192;
    }

    if (number(1, check) < (((skill + 150) / 2) + bonus)) {
      qhit(ch, victim, type);

      return;
    }
  }
}

void qhit(CHAR *ch, CHAR *victim, int type) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  /* Coerce attack type as needed. */
  if ((type != TYPE_UNDEFINED) && (type != TYPE_WEAPON2)) {
    type = TYPE_UNDEFINED;
  }

  perform_hit(ch, victim, type, 4);
}


void blood_lust_action(CHAR *ch, CHAR *vict) {
  if (!ch || !vict) return;

  int check = 20;

  /* Bathed in Blood */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_DEFILER, 5)) {
    check += 5 + (CHAR_REAL_ROOM(ch) != NOWHERE) ? ROOM_BLOOD(CHAR_REAL_ROOM(ch)) : 0;
  }

  if (!chance(check)) return;

  int dmg = 0;

  switch(number(1, 4)) {
    case 1:
    case 2:
      act("$n bites viciously at $N with $s fangs!", TRUE, ch, 0, vict, TO_NOTVICT);
      act("$n viciously bites at you with $s fangs!", FALSE, ch, 0, vict, TO_VICT);
      act("You bite at $N viciously with your fangs!", FALSE, ch, 0, vict, TO_CHAR);

      dmg = 60;

      if (affected_by_spell(ch, SPELL_DESECRATE)) {
        dmg *= 1.1;
      }

      damage(ch, vict, dmg, TYPE_UNDEFINED, DAM_PHYSICAL);
      break;

    case 3:
      act("$n sinks $s fangs into $N's neck, draining $S life!", TRUE, ch, 0, vict, TO_NOTVICT);
      act("$n sinks $s fangs into your neck, draining your life!", FALSE, ch, 0, vict, TO_VICT);
      act("You sink your fangs into $N's neck, draining $S life!", FALSE, ch, 0, vict, TO_CHAR);

      dmg = 40;

      if (affected_by_spell(ch, SPELL_DESECRATE)) {
        dmg *= 1.1;
      }

      damage(ch, vict, dmg, TYPE_UNDEFINED, DAM_MAGICAL);

      magic_heal(ch, SPELL_BLOOD_LUST, dmg, TRUE);

      GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - dmg);
      break;

    case 4:
      act("$n bites savagely at $N, draining $S magical essence!", TRUE, ch, 0, vict, TO_NOTVICT);
      act("$n bites at you savagely, draining your magical essence!", FALSE, ch, 0, vict, TO_VICT);
      act("You bite savagely at $N, draining $S magical essence!", FALSE, ch, 0, vict, TO_CHAR);

      dmg = 20;

      if (affected_by_spell(ch, SPELL_DESECRATE)) {
        dmg *= 1.1;
      }

      drain_mana_hit_mv(ch, vict, dmg, 0, 0, TRUE, FALSE, FALSE);
      break;
  }

  if (IS_MORTAL(ch) && chance(20)) {
    send_to_room("Blood sprays across the room, staining the surroundings dark crimson.\n\r", CHAR_REAL_ROOM(ch));

    ROOM_BLOOD(CHAR_REAL_ROOM(ch)) = MIN(ROOM_BLOOD(CHAR_REAL_ROOM(ch)) + 1, 10);
  }
}


void victimize_action(CHAR *ch, CHAR *vict) {
  if (!ch || !vict) return;

  struct victimize_data_t {
    char *name;
    int location;
    int modifier;
  };

  struct victimize_data_t victimize_data[] = {
    {
      .name = "Victimized (AC Penalty)",
      .modifier = (GET_LEVEL(ch) / 4),
      .location = APPLY_AC,
    },
    {
      .name = "Victimized (Hitroll Penalty)",
      .modifier = -(number(3, 5)),
      .location = APPLY_HITROLL,
    },
    {
      .name = "Victimized (Damroll Penalty)",
      .modifier = -((GET_DAMROLL(vict) * number(5, 10)) / 100),
      .location = APPLY_DAMROLL,
    },
  };

  /* Check access to Victimize and that the character is evil. */
  if (!check_sc_access(ch, SKILL_VICTIMIZE) || !IS_EVIL(ch)) return;

  /* Calculate the skill check. */
  int check = number(1, 171) - GET_DEX_APP(ch);

  /* Check skill success/failure. */
  if (check > GET_LEARNED(ch, SKILL_VICTIMIZE)) return;

  /* Create a list to store possible affect locations for the Victimize affect. */
  int loc_list[NUMELEMS(victimize_data)];

  /* Fill the list by checking if the victim is already enchanted by .name from victimize_data above.
     If they aren't enchanted, put the .location number in the list index.
     If they are enchanted, put -1 in the list index. */
  for (int i = 0; (i < NUMELEMS(loc_list)) && (i < NUMELEMS(victimize_data)); i++) {
    loc_list[i] = (!enchanted_by(vict, victimize_data[i].name) ? victimize_data[i].location : -1);
  }

  /* Quick sort the location list descending, so that any -1 values are at the end of the list. */
  qsort(loc_list, NUMELEMS(loc_list), sizeof(int), qcmp_int_desc);

  /* Count the number of locations from the location list that are not -1 (already applied). */
  int num_locs = 0;
  for (int i = 0; (i < NUMELEMS(loc_list)) && (loc_list[i] != -1); i++) {
    num_locs++;
  }

  /* Check if there's nothing to choose. */
  if (!num_locs) return;

  /* Shuffle the num_loc elements of the location list. */
  shuffle_int_array(loc_list, num_locs);

  /* Create a victimize_data_t structure, then populate it with the data from the
     chosen location (first index of loc_list) by searching for it in victimize_data. */
  struct victimize_data_t vic_action = { 0 };
  for (int i = 0; i < NUMELEMS(victimize_data); i++) {
    if (victimize_data[i].location == loc_list[0]) {
      vic_action = victimize_data[i];

      break;
    }
  }

  /* Sanity check. */
  if (!vic_action.location) return;

  act("You victimize $N, inflicting physical and mental torment.", FALSE, ch, 0, vict, TO_CHAR);
  act("$n victimizes you, inflicting physical and mental torment.", FALSE, ch, 0, vict, TO_VICT);
  act("$n victimizes $N, inflicting physical and mental torment.", FALSE, ch, 0, vict, TO_NOTVICT);

  enchantment_apply(vict, FALSE, vic_action.name, SKILL_VICTIMIZE, 20, ENCH_INTERVAL_ROUND, vic_action.modifier, vic_action.location, 0, 0, 0);
}


void shadowstep_action(CHAR *ch, CHAR *vict) {
  if (!ch || !vict) return;

  if (!check_sc_access(ch, SKILL_SHADOWSTEP) || !EQ(ch, WIELD)) return;

  int check = number(1, 450) - (GET_DEX_APP(ch) * 5);

  double multi = 1.5;

  if (!IS_SET(CHAR_ROOM_FLAGS(ch), DARK) &&
      ((IS_DAY && IS_OUTSIDE(ch)) || IS_SET(CHAR_ROOM_FLAGS(ch), LIT))) {
    check += 50;
    multi -= 0.5;
  }

  if (IS_NIGHT) {
    check -= 15;
    multi += 0.3;

    if (IS_EVIL(ch)) {
      check -= 10;
      multi += 0.2;
    }
  }

  if (IS_SET(CHAR_ROOM_FLAGS(ch), DARK)) {
    check -= 15;
    multi += 0.3;

    if (IS_EVIL(ch)) {
      check -= 10;
      multi += 0.2;
    }
  }

  if (!CAN_SEE(vict, ch) ||
      affected_by_spell(ch, SPELL_IMP_INVISIBLE) ||
      affected_by_spell(ch, SPELL_BLACKMANTLE)) {
    check -= 25;
    multi += 0.5;
  }

  if (multi < 1.0) multi = 1.0;

  if (check_subclass(ch, SC_DEFILER, 5)) {
    multi += 0.5;
  }

  if (multi > 3.0) multi = 3.0;

  if (check > GET_LEARNED(ch, SKILL_SHADOWSTEP)) return;

  act("You step into the shadows and attack $N by surprise!", FALSE, ch, 0, vict, TO_CHAR);
  act("$n steps into the shadows and attacks you by surprise!", FALSE, ch, 0, vict, TO_VICT);
  act("$n steps into the shadows and attacks $N by surprise!", FALSE, ch, 0, vict, TO_NOTVICT);

  damage(ch, vict, (lround(calc_hit_damage(ch, vict, EQ(ch, WIELD), 0, RND_RND) * multi)), get_attack_type(ch, EQ(ch, WIELD)), DAM_PHYSICAL);
}


int dirty_tricks_enchantment(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    act("Blood oozes from your gaping wound.", FALSE, ch, 0, 0, TO_CHAR);
    act("Blood oozes from $n's gaping wound.", TRUE, ch, 0, 0, TO_ROOM);

    int set_pos = GET_POS(ch);

    int dmg = dice(3, 12);

    if (dmg >= GET_HIT(ch)) {
      dmg = GET_HIT(ch) - 1;
    }

    damage(ch, ch, dmg, SKILL_DIRTY_TRICKS, DAM_PHYSICAL);

    GET_POS(ch) = set_pos;

    return FALSE;
  }

  return FALSE;
}

void dirty_tricks_action(CHAR *ch, CHAR *vict) {
  const int DT_STAB = 1, DT_BLIND = 2, DT_KICK = 3;

  /* Check access to Dirty Tricks. */
  if (!check_sc_access(ch, SKILL_DIRTY_TRICKS)) return;

  /* Calculate the skill check. */
  int check = number(1, 850) - (GET_DEX_APP(ch) * 5);

  /* Check skill success/failure. */
  if (check > GET_LEARNED(ch, SKILL_DIRTY_TRICKS)) return;

  int trick_idx = 0;

  /* Create a list to store possible tricks. */
  int trick_list[3];

  if (EQ(ch, WIELD) && !enchanted_by(vict, "Gaping Wound (Dirty Tricks)")) {
    trick_list[trick_idx] = DT_STAB;
  }
  else {
    trick_list[trick_idx] = -1;
  }

  trick_idx++;

  if (!IS_IMMUNE(vict, IMMUNE_BLINDNESS) && !IS_AFFECTED(vict, AFF_BLIND)) {
    trick_list[trick_idx] = DT_BLIND;
  }
  else {
    trick_list[trick_idx] = -1;
  }

  trick_idx++;

  /* Kick is always possible. */
  trick_list[trick_idx] = DT_KICK;

  /* Quick sort the trick list descending, so that any FALSE values are at the end of the list. */
  qsort(trick_list, NUMELEMS(trick_list), sizeof(int), qcmp_int_desc);

  /* Count the number of tricks from the trick list that are not FALSE (unavailable). */
  int num_tricks = 0;
  for (int i = 0; (i < NUMELEMS(trick_list)) && (trick_list[i] != -1); i++) {
    num_tricks++;
  }

  /* Check if there's nothing to choose. */
  if (!num_tricks) return;

  /* Shuffle the num_trick elements of the trick list. */
  shuffle_int_array(trick_list, num_tricks);

  /* Choose the trick to use (first index of trick_list). */
  int trick = trick_list[0];

  if (trick == DT_STAB) {
    act("You stab your weapon deeply into $N, opening a gruesome gaping wound.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n stabs $s weapon deeply into you, opening a gruesome gaping wound.", FALSE, ch, 0, vict, TO_VICT);
    act("$n stabs $s weapon deeply into $N, opening a gruesome gaping wound.", FALSE, ch, 0, vict, TO_NOTVICT);

    enchantment_apply(vict, FALSE, "Gaping Wounds (Dirty Tricks)", SKILL_DIRTY_TRICKS, 20, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, dirty_tricks_enchantment);
  }
  else if (trick == DT_BLIND) {
    act("You throw some blinding dust into $N's eyes.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n throws some blinding dust into your eyes.", FALSE, ch, 0, vict, TO_VICT);
    act("$n throws blinding dust into $N's eyes.", FALSE, ch, 0, vict, TO_NOTVICT);

    act("$n seems to be blinded!", TRUE, vict, 0, 0, TO_ROOM);
    send_to_char("You have been blinded!\n\r", vict);

    affect_apply(vict, SPELL_BLINDNESS, 0, 40, APPLY_AC, AFF_BLIND, 0);
    affect_apply(vict, SPELL_BLINDNESS, 0, -4, APPLY_HITROLL, AFF_BLIND, 0);
  }
  else if (trick == DT_KICK) {
    if (AWAKE(vict) && IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(ch, vict, SKILL_DIRTY_TRICKS, BT_INVUL)) {
      act("You kick $N savagely, but $E seems unfazed.", FALSE, ch, 0, vict, TO_CHAR);
      act("$n kicks you savagely,  but you feel unfazed.", FALSE, ch, 0, vict, TO_VICT);
      act("$n kicks $N savagely,  but $E seems unfazed.", FALSE, ch, 0, vict, TO_NOTVICT);

      damage(ch, vict, 0, SKILL_DIRTY_TRICKS, DAM_NO_BLOCK);
    }
    else {
      act("You kick $N savagely, causing $M to double over in pain!", FALSE, ch, 0, vict, TO_CHAR);
      act("$n kicks you savagely, causing you to double over in pain!", FALSE, ch, 0, vict, TO_VICT);
      act("$n kicks $N savagely, causing $M to double over in pain!", FALSE, ch, 0, vict, TO_NOTVICT);

      int set_pos = stack_position(vict, POSITION_SITTING);

      damage(ch, vict, calc_position_damage(GET_POS(vict), 10), SKILL_DIRTY_TRICKS, DAM_PHYSICAL);

      if (CHAR_REAL_ROOM(vict) != NOWHERE && !IS_IMPLEMENTOR(vict)) {
        GET_POS(vict) = set_pos;

        WAIT_STATE(vict, PULSE_VIOLENCE);
      }
    }
  }
}


void snipe_action(CHAR *ch, CHAR *victim) {
  if (!ch || !victim) return;

  char buf[MIL];

  snprintf(buf, sizeof(buf), "Sniped by %s", GET_DISP_NAME(ch));

  if (enchanted_by(victim, buf)) return;

  int dmg = GET_LEVEL(ch) * number(15, 25);
  double percent = (double)GET_HIT(victim) / (double)GET_MAX_HIT(victim);

  double multi = 1.0;
  
  if (IS_AFFECTED(victim, AFF_SANCTUARY) && !IS_AFFECTED2(victim, AFF2_FORTIFICATION)) multi = 0.5;
  else if (!IS_AFFECTED(victim, AFF_SANCTUARY) && IS_AFFECTED2(victim, AFF2_FORTIFICATION)) multi = 0.85;
  else if (IS_AFFECTED(victim, AFF_SANCTUARY) && IS_AFFECTED2(victim, AFF2_FORTIFICATION)) multi = 0.35;

  int check = 0;
  
  if (GET_HIT(victim) > lround(dmg * multi)) {
    check = (int)(100 * (0.5 - percent));
  }
  else {
    check = MIN(100, (int)(10 * (1.0 / percent)));
  }

  if (!check || !chance(check)) return;

  act("You take advantage of $N's weakness and snipe $M with a deadly attack!", FALSE, ch, 0, victim, TO_CHAR);
  act("$n takes advantage of your weakness and snipes you with a deadly attack!", FALSE, ch, 0, victim, TO_VICT);
  act("$n takes advantage of $N's weakness and snipes $m with a deadly attack!", FALSE, ch, 0, victim, TO_NOTVICT);

  damage(ch, victim, dmg, SKILL_SNIPE, DAM_PHYSICAL);

  enchantment_apply(victim, FALSE, buf, SKILL_SNIPE, 10, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, 0);
}


void perform_violence(void) {
  for (CHAR *ch = combat_list; ch && GET_OPPONENT(ch); ch = combat_next_dude) {
    combat_next_dude = ch->next_fighting;

    CHAR *vict = GET_OPPONENT(ch);

    if (!AWAKE(ch) || !SAME_ROOM(ch, vict)) {
      stop_fighting(ch);

      continue;
    }

    if (signal_char(ch, vict, MSG_VIOLENCE, "")) return;

    /* Shadowstep is before hit() in order to take advantage of pummel, etc. */
    if (IS_MORTAL(ch)) {
      if (SAME_ROOM(ch, vict) && IS_SET(GET_TOGGLES(ch), TOG_SHADOWSTEP) && check_sc_access(ch, SKILL_SHADOWSTEP)) {
        shadowstep_action(ch, vict);
      }
    }

    /* Perform a melee attack. */
    if (SAME_ROOM(ch, vict)) {
      hit(ch, vict, TYPE_UNDEFINED);
    }

    /* These skills are applied after melee attacks in order to avoid consuming position state. */

    /* Sidearm */
    if (SAME_ROOM(ch, vict) && IS_MORTAL(ch) && check_subclass(ch, SC_MERCENARY, 5) && GET_WEAPON2(ch) && !enchanted_by(ch, "Readying Sidearm...")) {
      act("You draw your sidearm and attack $N!", FALSE, ch, 0, vict, TO_CHAR);
      act("$n draws $s sidearm and attacks $N!", FALSE, ch, 0, vict, TO_ROOM);

      hit(ch, vict, TYPE_WEAPON2);

      enchantment_apply(ch, FALSE, "Readying Sidearm...", 0, number(4, 6), ENCH_INTERVAL_ROUND, 0, 0, 0, 0, 0);
    }

    /* Blood Lust */
    if (SAME_ROOM(ch, vict) && IS_MORTAL(ch) && affected_by_spell(ch, SPELL_BLOOD_LUST)) {
      blood_lust_action(ch, vict);
    }

    /* Victimize */
    if (SAME_ROOM(ch, vict) && IS_MORTAL(ch) && IS_SET(GET_TOGGLES(ch), TOG_VICTIMIZE) && check_sc_access(ch, SKILL_VICTIMIZE)) {
      victimize_action(ch, vict);
    }

    /* Snipe */
    if (SAME_ROOM(ch, vict) && IS_MORTAL(ch) && IS_SET(GET_TOGGLES(ch), TOG_SNIPE) && check_sc_access(ch, SKILL_SNIPE)) {
      snipe_action(ch, vict);
    }

    /* Dirty Tricks */
    if (SAME_ROOM(ch, vict) && IS_MORTAL(ch) && IS_SET(GET_TOGGLES(ch), TOG_DIRTY_TRICKS) && check_sc_access(ch, SKILL_DIRTY_TRICKS)) {
      dirty_tricks_action(ch, vict);
    }
  }
}


#define TOKEN_MOB 11

void brag(CHAR *ch, CHAR *vict) {
  const char *brags[] = {
    "%s was just too easy a kill!",
    "%s was a tasty dinner!  Now who's for dessert?",
    "Bahaha! %s should stick to fighting Odifs!",
    "%s is now in need of some exp...",
    "%s needs a hospital now.",
    "%s is such a wimp; no challenge at all.",
    "%s is a punk and hits like a dragonfly.  Bah.",
    "%s, your life force has just run out...",
    "Bah!  %s should stick to the training ground!",
    "%s, give me your family's address and I might return your corpse.",
    "Hey %s! Come back!  You dropped your corpse!",
    "%s wears pink chainmail and fights like a pansy!",
    "Hahaha! %s hits like a fido!  Sissy!",
    "I charp in your direction, %s!",
    "I guess you thought you could whoop me, eh %s?",
    "If that's all you can do, you had better try harder %s, cause it ain't enough to bring ME down!",
    "Where did you get that weapon %s?  From the newts?  :P",
    "If you wanna play with the big boys, you had better get BIG, %s.",
    "Dunt, dunt, dunt...  Another one bites the dust!  Or, should I say, %s did.  *chortle*",
    "Game Over %s...  Game Over.",
    "Haha, %s is no match for me!",
    "%s fights like a yeasty bit of stomach bile!",
    "Mmm!  Look what goodies %s left for me in their corpse!",
    "%s couldn't hit the broad side of a barn, let alone the actual cow!",
    "Who taught you how to fight %s?  Your Grandma?",
    "Muhaha!  Try as you might %s, you'll never kill me!",
    "One thing's for certain: %s should stop trying while ahead!",
    "Ouch, %s...  Hope you didn't lose any stats!  Hahaha!",
    "So you thought you could kill me, eh %s?",
    "To junk %s's corpse, or to not junk %s's corpse, that is the question...",
    "Hey %s, come back and fight like a man!",
    "Hey everyone, %s is naked at the temple!  Don't stare too long.  :P",
    "%s is inferior to my greatness!",
    "Someone come get %s's corpse outta here; its starting to smell like rotten fish.",
    "That was a really good attempt at killing me, %s.  Better luck next time.",
    "Hey %s, you coming back for more anytime soon?",
    "%s is no match for my superior skills!  *flex*",
    "Hey %s, nice killing you! See ya again sometime!",
    "Ouch, how much exp will that cost you, %s?",
    "I have SLAIN %s!  Fear my wrath!",
    "%s has failed, yet again, to slay me!",
    "%s, you are the weakest link!  Goodbye!",
    "Thanks for the meta, %s!  I got a 7!",
    "LEVEL!!!",
    "Hey %s, where'd you get your equipment?  The donation?  Muhaha!",
  };

  char buf[MIL], brag[MSL];

  /* Pick a random brag and print it to buf. Need to include GET_NAME(vict) for as many %s' in the string. */
  snprintf(buf, sizeof(buf), brags[number(0, NUMELEMS(brags) - 1)], GET_NAME(vict), GET_NAME(vict));

  /* Construct the brag. */
  snprintf(brag, sizeof(brag), "$n brags '%s'", buf);

  /* Do this the "hard way", since we want a "custom" gossip string. */
  for (DESC *temp_desc = descriptor_list; temp_desc; temp_desc = temp_desc->next) {
    CHAR *temp_char = temp_desc->character;

    if ((temp_desc->connected == CON_PLYNG) && temp_char && (temp_char != ch) && IS_SET(GET_PFLAG(temp_char), PLR_GOSSIP)) {
      COLOR(temp_char, 5);
      act(brag, FALSE, ch, 0, temp_char, TO_VICT);
      ENDCOLOR(temp_char);
    }
  }

  /* If Rashgugh is in the game, have him brag too. */
  CHAR *rashgugh = get_ch_world(TOKEN_MOB);

  if (rashgugh && chance(25)) {
    snprintf(brag, sizeof(brag), "I didn't attend the funeral, but I sent a nice letter saying I approved of it.");

    do_yell(rashgugh, brag, CMD_YELL);
  }
}
