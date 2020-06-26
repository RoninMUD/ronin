/**************************************************************************
*  File: reception.c, Special module for Inn's.           Part of DIKUMUD *
*  Usage: Procedures handling saving/loading of player objects            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "constants.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "limits.h"
#include "fight.h"
#include "utils.h"
#include "spells.h"
#include "cmd.h"
#include "utility.h"
#include "reception.h"
#include "enchant.h"
#include "subclass.h"

#define BUFFERSIZE 65535
#ifdef TEST_SITE
#define MAX_ITEMS_RENT 666
#else
#define MAX_ITEMS_RENT 200
#endif

int cc;
int total_connects;

/* Extern functions */
void store_to_char_5(struct char_file_u_5 *st, CHAR *ch); /* Latest version is 5 */
void store_to_char_4(struct char_file_u_4 *st, CHAR *ch);
void store_to_char_2(struct char_file_u_2 *st, CHAR *ch);
void char_to_store(CHAR *ch, struct char_file_u_5 *st);
void do_tell(CHAR *ch, char *argument, int cmd);
int str_cmp(char *arg1, char *arg2);
void clear_char(CHAR *ch);
void strip_char(CHAR *ch);
void death_list(CHAR *ch);
int check_god_access(CHAR *ch, int active);


/* ************************************************************************
* Routines used for the "Offer"                                           *
************************************************************************* */
int total_cost_of_obj (struct obj_data *obj) {
  int cost=0;
  struct obj_data *tmp;
  if(obj) {
    for(tmp = obj->contains;tmp;tmp=tmp->next_content)
      cost += total_cost_of_obj(tmp);
    if (IS_RENTABLE(obj)) cost += MAX(0,obj->obj_flags.cost_per_day)/2;
  }
  return cost;
}

void corpse_check(CHAR *ch) {
  struct obj_data *obj=NULL,*next_o,*obj2=NULL,*next_o2;
  for(obj = ch->carrying; obj; obj = next_o) {
    next_o=obj->next_content;
    if (obj && (obj->obj_flags.type_flag==ITEM_CONTAINER) && obj->obj_flags.value[3]) { /* Its a corpse */
      for (obj2=obj->contains;obj2;obj2=next_o2) {
        next_o2=obj2->next_content;
        if(obj2) {
          obj_from_obj(obj2);
          if(obj2->obj_flags.type_flag == ITEM_MONEY) {
            GET_GOLD(ch) += obj2->obj_flags.value[0];
            extract_obj(obj2);
          }
          else obj_to_char(obj2, ch);
        }
      }
      obj_from_char(obj);
      extract_obj(obj);
      corpse_check(ch);
      return;
    }
  }
}

void money_check(CHAR *ch) {
  struct obj_data *obj=NULL,*next_o;
  for(obj = ch->carrying; obj; obj = next_o) {
    next_o=obj->next_content;
    if(obj && (obj->obj_flags.type_flag == ITEM_MONEY)) { /* Its money */
      GET_GOLD(ch) += obj->obj_flags.value[0];
      extract_obj(obj);
    }
  }
}

#define IS_HEMP_FLAG(obj) (obj->item_number_v >= 26220 && obj->item_number_v <= 26280)
void add_obj_cost(CHAR *ch, struct obj_data *obj,struct obj_cost *cost) {
  char buf[MAX_STRING_LENGTH];
  /* Add cost for an item and it's contents, and next->contents */

  if (obj) {
    if(obj->obj_flags.type_flag==ITEM_SC_TOKEN || IS_HEMP_FLAG(obj)) {
      sprintf(buf, "You can't rent %s !\n\r",fname(obj->name));
      send_to_char(buf, ch);
      add_obj_cost(ch, obj->contains, cost);
      add_obj_cost(ch, obj->next_content, cost);
      cost->ok =FALSE;
      return;
    }
    if (IS_RENTABLE(obj)) {
      cost->total_cost += MAX(0, obj->obj_flags.cost_per_day)/2;
      cost->no_carried++;
      sprintf(buf, "%s rent is %d\n\r",OBJ_SHORT(obj), obj->obj_flags.cost_per_day);
      send_to_char(buf, ch);
      add_obj_cost(ch, obj->contains, cost);
      add_obj_cost(ch, obj->next_content, cost);
    }
    else {
      if (obj->item_number > -1) {
        sprintf(buf, "You can't rent %s (will be removed from you when rent)\n\r",fname(OBJ_NAME(obj)));
        send_to_char(buf, ch);
        add_obj_cost(ch, obj->contains, cost);
        add_obj_cost(ch, obj->next_content, cost);
      }
      else {
        sprintf(buf, "You can't rent %s !\n\r",fname(obj->name));
        send_to_char(buf, ch);
        add_obj_cost(ch, obj->contains, cost);
        add_obj_cost(ch, obj->next_content, cost);
        cost->ok =FALSE;
      }
    }
  }
}

bool recep_offer(CHAR *ch,CHAR *receptionist,struct obj_cost *cost) {
  int i, can_day;
  char buf[MAX_STRING_LENGTH];

  cost->total_cost = 1;   /* Minimum cost */
  cost->no_carried = 0;
  cost->ok = TRUE;        /* Use if any "-1" objects */

  add_obj_cost(ch, ch->carrying, cost);

  for(i = 0; i<MAX_WEAR; i++)
    add_obj_cost(ch, ch->equipment[i], cost);

  if(!cost->ok) {
    act("$n tells you 'You have something unrentable!'",FALSE,receptionist,0,ch,TO_VICT);
    return(FALSE);
  }

  if (cost->no_carried == 0) {
    act("$n tells you 'But you are not carrying anything?'",FALSE,receptionist,0,ch,TO_VICT);
    return(FALSE);
  }

  sprintf(buf,"$n tells you 'That's a total of %d items.", cost->no_carried);
  act(buf,FALSE,receptionist,0,ch,TO_VICT);

  if (cost->no_carried > MAX_ITEMS_RENT) {
    sprintf(buf,"$n tells you 'I can't store any more than %d items.", MAX_ITEMS_RENT);
    act(buf,FALSE,receptionist,0,ch,TO_VICT);
    return(FALSE);
  }
  if(ch->desc)
    if(ch->desc->free_rent)
      cost->total_cost = 0;

  // Prestige Perk 5
  if (GET_PRESTIGE_PERK(ch) >= 5) {
    cost->total_cost *= 0.9;
  }

  sprintf(buf, "$n tells you 'It will cost you %d coins per day'", cost->total_cost);
  act(buf,FALSE,receptionist,0,ch,TO_VICT);

  if(GET_LEVEL(ch) >= LEVEL_IMM) cost->total_cost = 0;

  if(cost->total_cost > GET_GOLD(ch)) {
    act("$n tells you 'Which I can see you can't afford'",
         FALSE,receptionist,0,ch,TO_VICT);
    return(FALSE);
  }

  if (GET_LEVEL(ch) < LEVEL_IMM) {
    if(cost->total_cost)
      can_day = (GET_GOLD(ch) / cost->total_cost);
    else
      can_day = 180;

    sprintf(buf, "$n tells you 'You can rent %d times'", can_day);
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
  }
  return(TRUE);
}

void rank_char(CHAR *ch);
/* ************************************************************************
* Routines used to load a character from disk                             *
************************************************************************* */

/* Version will start at version 2 as the first byte in a pfile
   is the sex variable which can take values of 0, 1 or 2.  Later
   pfiles will have the first byte as the version number, starting
   at version 3. - Ranger May 97

   Version 4 is exactly the same as version 3, except the version
   change is used to reset xp and god levels for the change to 50 levels.
   Ranger March 99
*/

int char_version(FILE *fl) {
  struct char_file_version char_version;

  if((fread(&char_version,sizeof(struct char_file_version),1,fl))!=1) {
    log_s("Error Checking Char Version");
    rewind(fl);
    return (0);
  }
  rewind(fl);
  if(char_version.version<3) return(2);
  else return(char_version.version);
}

int test_char(char *name, char *pwd) {
  FILE *fl;
  struct char_file_u_2 char_data_2;
  struct char_file_u_4 char_data_4;
  struct char_file_u_5 char_data_5;
  char buf[MAX_STRING_LENGTH];
  char tmp_name[50];
  int version;

  snprintf(tmp_name,sizeof(tmp_name),"%s",name);
  string_to_lower(tmp_name);
  sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);
  /* r+b is for Binary Reading/Writing */
  if (!(fl = fopen(buf, "rb"))) {
    sprintf(buf,"%c : %s didnt have .dat file--new character : %c",
            UPPER(name[0]),name,UPPER(name[0]));
    log_s(buf);
    return(0);
  }

  version=char_version(fl);

  switch(version) {
    case 2:
      if((fread(&char_data_2,sizeof(struct char_file_u_2),1,fl))!=1)
      {log_s("Error Reading");fclose(fl);return (-1);}
      sprintf(pwd,"%s",char_data_2.pwd);
      fclose(fl);
      return(char_data_2.level);
      break;
    case 3:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
      {log_s("Error Reading");fclose(fl);return (-1);}
      sprintf(pwd,"%s",char_data_4.pwd);
      fclose(fl);
      return(char_data_4.level);
      break;
    case 4:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
      {log_s("Error Reading");fclose(fl);return (-1);}
      sprintf(pwd,"%s",char_data_4.pwd);
      fclose(fl);
      return(char_data_4.level);
      break;
    case 5:
      if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1)
      {log_s("Error Reading");fclose(fl);return (-1);}
      sprintf(pwd,"%s",char_data_5.pwd);
      fclose(fl);
      return(char_data_5.level);
      break;
    default:
      log_s("Error getting pfile version (test_char)");
      fclose(fl);
  }
  return(-1);
}

int generate_id(void);
void check_idname(CHAR *ch);

void load_char(CHAR *ch) {
  FILE *fl;
  double timegold;
  struct tm *timeStruct;
  long last_up, ct;
  int rent,version,tot_cost;
  struct char_file_u_2 char_data_2;
  struct char_file_u_4 char_data_4;
  struct char_file_u_5 char_data_5;
  char buf[MAX_STRING_LENGTH];
  char tmp_name[50];
  char alertstring[]={7,7,7,0};
  void obj_to_char(struct obj_data *object, CHAR *ch);
  bool rent_equip_char(CHAR *ch, struct obj_data *obj, int possition);

  sprintf(tmp_name,"%s",GET_NAME(ch));
  string_to_lower(tmp_name);
  sprintf(buf,"cp rent/%c/%s.dat rent/%c/%s.bak",
          UPPER(tmp_name[0]),tmp_name,UPPER(tmp_name[0]),tmp_name);
  system(buf);
  /* r+b is for Binary Reading/Writing */
  sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);

  if (!(fl = fopen(buf, "rb"))) {
    sprintf(buf,"%c : %s didnt have .dat file : %c",
            UPPER(tmp_name[0]),GET_NAME(ch),UPPER(tmp_name[0]));
    log_s(buf);
    return;
  }

  version=char_version(fl);

  switch(version) {
    case 2:
      if((fread(&char_data_2,sizeof(struct char_file_u_2),1,fl))!=1)
      {log_s("Error Reading rent file(load_char)");fclose(fl);return;}
      store_to_char_2(&char_data_2,ch);
      last_up=char_data_2.last_update;
      tot_cost=char_data_2.total_cost;
      break;
    case 3:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
      {log_s("Error Reading rent file(load_char)");fclose(fl);return;}
      store_to_char_4(&char_data_4,ch);
      last_up=char_data_4.last_update;
      tot_cost=char_data_4.total_cost;
      /* Changes for 50 level update */
      GET_EXP(ch)=0;
      if(GET_LEVEL(ch)==35 && IS_SET(ch->new.imm_flags, WIZ_ACTIVE)) GET_LEVEL(ch)=LEVEL_IMP;
      else if(GET_LEVEL(ch)>=31) GET_LEVEL(ch)=LEVEL_IMM;
      break;
    case 4:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
      {log_s("Error Reading rent file(load_char)");fclose(fl);return;}
      store_to_char_4(&char_data_4,ch);
      last_up=char_data_4.last_update;
      tot_cost=char_data_4.total_cost;
      break;
    case 5:
      if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1)
      {log_s("Error Reading rent file(load_char)");fclose(fl);return;}
      store_to_char_5(&char_data_5,ch);
      last_up=char_data_5.last_update;
      tot_cost=char_data_5.total_cost;
      break;
    default:
      log_s("Error getting pfile version(load_char)");
      return;
  }

#ifdef CHAOS2017

  if (CHAOSMODE && GET_LEVEL(ch)<LEVEL_IMM)
  {
    int max_hit = 1000, max_mana = 900, max_move = 900;
    AFF *af = NULL;
    ENCH *ench = NULL, *ench_next = NULL, **ench_prev = NULL;
    int i = 0;

    GET_LEVEL(ch) = 50;
    GET_CLASS(ch) = CLASS_AVATAR;
    GET_REMORT_EXP(ch) = 0LL;
    GET_DEATH_EXP(ch) = 0;
    GET_EXP(ch) = 0;
    GET_SCP(ch) = 0;
    GET_QP(ch) = 0;
    GET_PRAC(ch) = 127;
    GET_SC(ch) = 0;
    GET_SC_LEVEL(ch) = 0;

    /* Affects and enchantments have already been added by store_to_char */

    /* No equipment has been processed yet, so ignore it. */

    /* Unmodify all current affects */

    for(af = ch->affected; af; af=af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector, af->bitvector2, FALSE);

    for(ench = ch->enchantments, ench_prev = &ch->enchantments; ench; ench=ench_next)
    {
      ench_next = ench->next;
      affect_modify(ch, ench->location, ench->modifier, ench->bitvector,ench->bitvector2, FALSE);

      if (((ENCHANT_SQUIRE <= ench->type) && (ENCHANT_CONDUCTOR >= ench->type)) ||
          (ENCHANT_REMORTV2 == ench->type) ||
          (ENCHANT_IMM_GRACE == ench->type))
      {
        *ench_prev = ench_next;
        DESTROY(ench->name);
        DESTROY(ench);
      }
      else
      {
        ench_prev = &ench->next;
      }
    }

    /* Set all char attributes to 20 */

    ch->abilities.str = 20;
    ch->abilities.str_add = 100;
    ch->abilities.intel = 20;
    ch->abilities.wis = 20;
    ch->abilities.dex = 20;
    ch->abilities.con = 20;

    ch->tmpabilities = ch->abilities;

    /* Normalize all char points - 2017 not used */

    switch (GET_CLASS(ch))
    {
      case CLASS_WARRIOR:
        max_hit = 1750;
        max_mana = 0;
        break;

      case CLASS_NOMAD:
        max_hit = 2000;
        max_mana = 0;
        break;

      case CLASS_THIEF:
        max_hit = 1500;
        max_mana = 0;
        break;

      case CLASS_CLERIC:
      case CLASS_MAGIC_USER:
      case CLASS_BARD:
        max_hit = 750;
        max_mana = 1150;

      default:
       break;
    }

    ch->points.mana      = max_mana;
    ch->points.max_mana  = max_mana;
    ch->points.hit       = max_hit;
    ch->points.max_hit   = max_hit;
    ch->points.move      = max_move;
    ch->points.max_move  = max_move;

    ch->specials.prev_max_mana = max_mana;
    ch->specials.prev_max_hit  = max_hit;
    ch->specials.prev_max_move = max_move;
    ch->specials.org_mana      = max_mana;
    ch->specials.org_hit       = max_hit;
    ch->specials.org_move      = max_move;

    /* Re-apply affects and enchantments */

    for(af = ch->affected; af; af=af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector,af->bitvector2, TRUE);

    for(ench = ch->enchantments; ench; ench=ench->next)
      affect_modify(ch, ench->location, ench->modifier, ench->bitvector,ench->bitvector2, TRUE);

    /* Affects to not increase supernatural stats, pin them */

    GET_CON(ch) = MAX(0,MIN(GET_CON(ch),GET_OCON(ch)));
    GET_INT(ch) = MAX(0,MIN(GET_INT(ch),GET_OINT(ch)));
    GET_DEX(ch) = MAX(0,MIN(GET_DEX(ch),GET_ODEX(ch)));
    GET_WIS(ch) = MAX(0,MIN(GET_WIS(ch),GET_OWIS(ch)));
    GET_STR(ch) = MAX(0,MIN(GET_STR(ch),GET_OSTR(ch)));

    if(GET_STR(ch)<18)
    {
      i=GET_ADD(ch)/10;
      GET_STR(ch)+=i;
      GET_ADD(ch)-=i*10;
      if(GET_STR(ch)>18)
      {
        i=GET_ADD(ch)+((GET_STR(ch)-18)*10);
        GET_ADD(ch)=MIN(i, 100);
        GET_STR(ch)=18;
      }
    }
    /* set skills to 100 */
    for (int done = FALSE,number = 0,i = 0; !done; i++) {
      if (*avatar_skills[i] == '\n') done = TRUE;
      else
      {
        number = old_search_block(avatar_skills[i], 0, strlen(avatar_skills[i]), spells, TRUE);

        if (number == 0) continue;
        else if (!check_sc_access(ch, number)) continue;
        else
        {
          ch->skills[number].learned = 100;
        }
      }
    }
    /* set spells to 100 */
    for (i = 0; *spells[i] != '\n'; i++)
    {
      if (!spell_info[i + 1].spell_pointer) continue;
      else if ((ch->skills[i + 1].learned >= 85)) continue;
      else
      {
        ch->skills[i+1].learned = 100;
      }
    }
  }
#endif



  if(GET_LEVEL(ch)<LEVEL_IMM) rank_char(ch);

  if(GET_LEVEL(ch)<LEVEL_SUP && IS_SET(ch->new.imm_flags, WIZ_QUEST_INFO))
    REMOVE_BIT(ch->new.imm_flags, WIZ_QUEST_INFO);

/* CHAOS2017: no equipment */
#ifndef CHAOS2017
  
  while (!feof(fl)) {
    switch(obj_version(fl)) {
      case 3:
        obj_to_char(store_to_obj_ver3(fl,ch),ch);
        break;
      case 2:
        obj_to_char(store_to_obj_ver2(fl,ch),ch);
        break;
      case 1:
        obj_to_char(store_to_obj_ver1(fl,ch),ch);
        break;
      case 0:
        obj_to_char(store_to_obj_ver0(fl,ch),ch);
        break;
    }
  }
#endif
  fclose(fl);



  if(last_up<uptime) total_connects++;

  /* New creation date */
  if(ch->ver3.created==0) {
    ct=ch->player.time.birth;
    timeStruct=localtime(&ct);
    ch->ver3.created=(timeStruct->tm_mon+1)*1000000+timeStruct->tm_mday*10000+
                     1900+timeStruct->tm_year;
  }

  /* New player id system */
  if(ch->ver3.id==0 || ch->ver3.id < 0) {
    ch->ver3.id=generate_id();
  }
  check_idname(ch);

  if (last_up+12*SECS_PER_REAL_HOUR < time(0)) {
    GET_MANA(ch) = MAX(GET_MANA(ch),GET_MAX_MANA(ch));
    GET_HIT(ch) = MAX(GET_HIT(ch),GET_MAX_HIT(ch));
    GET_MOVE(ch) = MAX(GET_MOVE(ch),GET_MAX_MOVE(ch));
  }

  if(CHAR_REAL_ROOM(ch)!=NOWHERE) {
    if(last_up + 1*SECS_PER_REAL_DAY < time(0)) {
      timegold = (double)tot_cost;
      send_to_char("You just got lowered rent... be happy ;)\n\r", ch);
    }
    else {
      timegold = (((double)tot_cost*
         ((double)time(0) - (double)last_up))/(double)(SECS_PER_REAL_DAY));
      if(timegold>(double)tot_cost) timegold=(double)tot_cost;
    }

    if((timegold<0) || IS_IMMORTAL(ch)) timegold=0; /* set rent to zero if neg away time for char */

    // Prestige Perk 2
    if ((GET_PRESTIGE_PERK(ch) >= 2) && chance(5)) {
      timegold = 0;
      send_to_char("You just got free rent, thanks to your prestige!\n\r", ch);
    }

    rent=(long int)timegold;
    sprintf(buf,"Your rent was %d coins.\n\r", rent);
    send_to_char(buf, ch);

    if ((GET_GOLD(ch)-rent) < 0 && GET_LEVEL(ch)<LEVEL_IMM) {
      sprintf(buf,"`oYou couldn't afford it! You only have %d coins.`q\n\r",GET_GOLD(ch));
      send_to_char(buf, ch);
      send_to_char(alertstring, ch);
      send_to_char("Checking your bank balance.  You will be charged 1.5x the outstanding balance.\n\r",ch);
      rent=rent-GET_GOLD(ch);
      GET_GOLD(ch)=0;
      if((GET_BANK(ch)-3*rent/2)<0) {
        send_to_char("You STILL couldn't afford it!\n\r", ch);
        send_to_char("You should be ashamed about that!\n\r", ch);
        GET_BANK(ch)=0;
        sprintf(buf,"%s didn't have enough for his/her rent! (%d coins)",GET_NAME(ch),rent);
        log_s(buf);
        wizlog(buf, GET_LEVEL(ch), 3);
        deathlog(buf);
       ch->new.been_killed += 1;
       death_list(ch);
        strip_char(ch);
      }  else GET_BANK(ch)=GET_BANK(ch)-3*rent/2;
    } else GET_GOLD(ch)=GET_GOLD(ch)-rent;
  }

  /* New signal for recharging items in rent. Ranger Oct 98*/
  sprintf(buf,"%ld",time(0)-last_up);
  if(signal_char(ch,ch,MSG_OBJ_ENTERING_GAME,buf))
    log_s("Error: Return TRUE from MSG_OBJ_ENTERING_GAME");

  if (ch->ver3.time_to_quest > 0) {
    ch->ver3.time_to_quest = MAX(ch->ver3.time_to_quest - 40, 5);
  }
  else {
    ch->ver3.time_to_quest = 0;
  }

  /* Default all imms gold to 10000 coins - Ranger Sept 97 */
  if(IS_IMMORTAL(ch)) GET_GOLD(ch)=10000;
  /* Save char, to avoid strange data if crashing, IMM check
     to disable players typing whois knowing you just came on */
  if(GET_LEVEL(ch)<LEVEL_IMM) save_char(ch, NOWHERE);
}

void strip_char(CHAR *ch) {
  int i;
  struct obj_data *obj,*next_item;
  for (obj=ch->carrying; obj; obj=next_item) {
    next_item = obj->next_content;
    extract_obj(obj);
  }
  for(i=0; i < MAX_WEAR; i++) {
    if(ch->equipment[i]) extract_obj(unequip_char(ch,i));
  }
}

/* ************************************************************************
* Routines used to save a characters equipment from disk                  *
************************************************************************* */
/*  Takes the item, and adds it to the player/rent file structure */
void obj_to_store(struct obj_data *obj, FILE *fl, CHAR * ch, char pos, bool includeNoRent) {
  int j;
  struct obj_data *tmp;
  struct obj_file_elem_ver3 object;

  memset(&object,0,sizeof(object));

  if (obj) {
    if(!IS_RENTABLE(obj) && !includeNoRent) {/*damn mushroom */
      return;
    }
    else {
      object.position   =pos;
      object.item_number=obj_proto_table[obj->item_number].virtual;
      object.value[0]   =obj->obj_flags.value[0];
      object.value[1]   =obj->obj_flags.value[1];
      object.value[2]   =obj->obj_flags.value[2];
      object.value[3]   =obj->obj_flags.value[3];
      if (obj->obj_flags.type_flag == ITEM_CONTAINER)
         object.value[3]=(includeNoRent ? COUNT_CONTENTS(obj) : COUNT_RENTABLE_CONTENTS(obj));
      object.extra_flags=obj->obj_flags.extra_flags;
      object.weight     =obj->obj_flags.weight;
      object.timer      =obj->obj_flags.timer;
      object.bitvector  =obj->obj_flags.bitvector;

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
      object.popped      =obj->obj_flags.popped;
/* end new ver3 obj saves */

/* New owner id */
      object.ownerid[0] =obj->ownerid[0];
      object.ownerid[1] =obj->ownerid[1];
      object.ownerid[2] =obj->ownerid[2];
      object.ownerid[3] =obj->ownerid[3];
      object.ownerid[4] =obj->ownerid[4];
      object.ownerid[5] =obj->ownerid[5];
      object.ownerid[6] =obj->ownerid[6];
      if (obj->obj_flags.type_flag != ITEM_AQ_ORDER) {
        object.ownerid[7] =obj->ownerid[7];
      } else { // hack our own container-like repacking using available field
        object.ownerid[7] = (includeNoRent ? COUNT_CONTENTS(obj) : COUNT_RENTABLE_CONTENTS(obj));
      }

      fwrite(&object, sizeof(object),1,fl);
    }
    if(COUNT_CONTENTS(obj)) {
      for (tmp = obj->contains;tmp;tmp = tmp->next_content)
         obj_to_store(tmp, fl, ch,-1,includeNoRent);
    }
  }
}

int obj_version(FILE *fl) {
  struct obj_file_version obj_version;
  if((fread(&obj_version,sizeof(struct obj_file_version),1,fl))!=1) {
    return(-1);
  }
  else {
    fseek(fl,-sizeof(struct obj_file_version),SEEK_CUR);
    if(obj_version.version==32001) return(1);
    if(obj_version.version==32002) return(2);
    if(obj_version.version==32003) return(3);
  }
  return(0);
}

int adjust_ticket_strings(OBJ *obj); /*Added Oct 98 Ranger */

struct obj_data *store_to_obj_ver3(FILE *fl, CHAR *ch) {
  struct obj_file_elem_ver3 object;

  memset(&object, 0, sizeof(struct obj_file_elem_ver3));

  fread(&object, sizeof(struct obj_file_elem_ver3), 1, fl);

  if (real_object(object.item_number) < 0) {
    log_f("BUG: store_to_obj_ver3() :: No such item #%d", object.item_number);

    return NULL;
  }

  OBJ *obj = read_object(object.item_number, VIRTUAL);

  OBJ_TYPE(obj) = object.type_flag;

  for (int i = 0; i < MAX_OBJ_VALUE; i++) {
    if ((i == 3) && (OBJ_TYPE(obj) == ITEM_CONTAINER)) continue;

    OBJ_VALUE(obj, i) = object.value[i];
  }

  for (int i = 0; i < MAX_OBJ_OWNER_ID; i++) {
    OBJ_OWNER_ID(obj, i) = object.ownerid[i];
  }

  OBJ_WEAR_FLAGS(obj) = object.wear_flags;
  OBJ_EXTRA_FLAGS(obj) = object.extra_flags;
  OBJ_EXTRA_FLAGS2(obj) = object.extra_flags2;
  OBJ_SC_RES(obj) = object.subclass_res;
  OBJ_WEIGHT(obj) = object.weight;

  if ((!IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_ALL_DECAY) && !IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_EQ_DECAY)) || (object.timer != 0)) {
    OBJ_TIMER(obj) = object.timer;
  }

  OBJ_MATERIAL(obj) = object.material;
  OBJ_BITS(obj) = object.bitvector;
  OBJ_BITS2(obj) = object.bitvector2;
  OBJ_SPEC(obj) = object.spec_value;

  for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
    OBJ_AFF(obj, i) = object.affected[i];
  }

  OBJ_POPPED(obj) = object.popped;

  /* Pack Containers */
  if (OBJ_TYPE(obj) == ITEM_CONTAINER) {
    for (int i = 0; i < object.value[3]; i++) {
      obj_to_obj(store_to_obj_ver3(fl, ch), obj);
    }
  }

  /* Pack AQ Orders */
  if (OBJ_TYPE(obj) == ITEM_AQ_ORDER) {
    for (int i = 0; i < object.ownerid[7]; i++) {
      obj_to_obj(store_to_obj_ver3(fl, ch), obj);
    }
  }

  /* Adjust Ticket Strings */
  if ((OBJ_TYPE(obj) == ITEM_TICKET) && !adjust_ticket_strings(obj)) return NULL;

  /* Attempt to equip the object in the correct EQ slot if it was worn when the character rented. */
  if (rent_equip_char(ch, obj, object.position)) return NULL;

  return obj;
}

struct obj_data *store_to_obj_ver2(FILE *fl,CHAR *ch) {
  struct obj_file_elem_ver2 object;
  struct obj_data *obj=NULL;
  int i;

  memset(&object,0,sizeof(struct obj_file_elem_ver2));
  fread(&object,sizeof(struct obj_file_elem_ver2),1,fl);
  if(real_object(object.item_number)>-1) {
    obj=read_object(object.item_number,VIRTUAL);
    if((IS_SET(obj->obj_flags.extra_flags2,ITEM_ALL_DECAY) ||
        IS_SET(obj->obj_flags.extra_flags2,ITEM_EQ_DECAY)) &&
        object.timer==0) {
      /* Timer is as assigned in read_object */
      /* Allows the addition of timers to popped objects for decay items*/
    }
    else {
      obj->obj_flags.timer    =object.timer;
    }
    obj->obj_flags.value[0]   =object.value[0];
    obj->obj_flags.value[1]   =object.value[1];
    obj->obj_flags.value[2]   =object.value[2];
    if(obj->obj_flags.type_flag != ITEM_CONTAINER)
      obj->obj_flags.value[3]   =object.value[3];
    obj->obj_flags.extra_flags=object.extra_flags;
    obj->obj_flags.weight     =object.weight;
    obj->obj_flags.bitvector  =object.bitvector;

/* new obj reads */
    obj->obj_flags.type_flag    =object.type_flag;
    obj->obj_flags.wear_flags   =object.wear_flags;
    obj->obj_flags.extra_flags2 =object.extra_flags2;
    obj->obj_flags.subclass_res =object.subclass_res;
    obj->obj_flags.material     =object.material;
    OBJ_SPEC(obj)               =object.spec_value;

    for(i=0;i<MAX_OBJ_AFFECT;i++)
      obj->affected[i]  =object.affected[i];
/* end new obj reads */

/* New ver3 reads */
    obj->obj_flags.bitvector2  =0;

/* New ownerid field */
    obj->ownerid[0]             =object.ownerid[0];
    obj->ownerid[1]             =object.ownerid[1];
    obj->ownerid[2]             =object.ownerid[2];
    obj->ownerid[3]             =object.ownerid[3];
    obj->ownerid[4]             =object.ownerid[4];
    obj->ownerid[5]             =object.ownerid[5];
    obj->ownerid[6]             =object.ownerid[6];
    obj->ownerid[7]             =object.ownerid[7];

    if (obj->obj_flags.type_flag == ITEM_TICKET) {
      if(!adjust_ticket_strings(obj)) return 0;
    }

    if(obj->obj_flags.type_flag == ITEM_CONTAINER) {
      for(i=0;i<object.value[3];i++)
        obj_to_obj(store_to_obj_ver2(fl,ch),obj);
    }
    if(rent_equip_char( ch, obj, object.position))
          obj = NULL;  /*  make it so it will return null, so load_char
                        *  wont add it to the inventory.
                        */
  }
  else  {
    if(object.item_number != 0)
      log_f("BUG: No such item #%d",object.item_number);
  }
  return obj;
}

struct obj_data *store_to_obj_ver1(FILE *fl,CHAR *ch) {
  struct obj_file_elem_ver1 object;
  struct obj_data *obj=NULL;
  int i;

  memset(&object,0,sizeof(struct obj_file_elem_ver1));
  fread(&object,sizeof(struct obj_file_elem_ver1),1,fl);
  if(real_object(object.item_number)>-1) {
    obj=read_object(object.item_number,VIRTUAL);
    if((IS_SET(obj->obj_flags.extra_flags2,ITEM_ALL_DECAY) ||
        IS_SET(obj->obj_flags.extra_flags2,ITEM_EQ_DECAY)) &&
        object.timer==0) {
      /* Timer is as assigned in read_object */
      /* Allows the addition of timers to popped objects for decay items*/
    }
    else {
      obj->obj_flags.timer    =object.timer;
    }
    obj->obj_flags.value[0]   =object.value[0];
    obj->obj_flags.value[1]   =object.value[1];
    obj->obj_flags.value[2]   =object.value[2];
    if(obj->obj_flags.type_flag != ITEM_CONTAINER)
      obj->obj_flags.value[3]   =object.value[3];
    obj->obj_flags.extra_flags=object.extra_flags;
    obj->obj_flags.weight     =object.weight;
    obj->obj_flags.bitvector  =object.bitvector;

/* new obj reads */
    obj->obj_flags.type_flag    =object.type_flag;
    obj->obj_flags.wear_flags   =object.wear_flags;
    obj->obj_flags.extra_flags2 =object.extra_flags2;
    obj->obj_flags.subclass_res =object.subclass_res;
    obj->obj_flags.material     =object.material;
    OBJ_SPEC(obj)               =object.spec_value;

    for(i=0;i<MAX_OBJ_AFFECT;i++)
      obj->affected[i]  =object.affected[i];
/* end new obj reads */

/* New ver3 reads */
    obj->obj_flags.bitvector2  =0;

    if (obj->obj_flags.type_flag == ITEM_TICKET) {
      if(!adjust_ticket_strings(obj)) return 0;
    }

    if(obj->obj_flags.type_flag == ITEM_CONTAINER) {
      for(i=0;i<object.value[3];i++)
        obj_to_obj(store_to_obj_ver1(fl,ch),obj);
    }
    if(rent_equip_char( ch, obj, object.position))
          obj = NULL;  /*  make it so it will return null, so load_char
                        *  wont add it to the inventory.
                        */
  }
  else  {
    if(object.item_number != 0)
      log_f("BUG: No such item #%d",object.item_number);
  }
  return obj;
}

struct obj_data *store_to_obj_ver0(FILE *fl,CHAR *ch) {
  struct obj_file_elem_ver0 object;
  struct obj_data *obj=NULL;
  int i;

  memset(&object,0,sizeof (struct obj_file_elem_ver0));
  fread(&object,sizeof(struct obj_file_elem_ver0),1,fl);
  if(real_object(object.item_number)>-1) {
    obj=read_object(object.item_number,VIRTUAL);
    if((IS_SET(obj->obj_flags.extra_flags2,ITEM_ALL_DECAY) ||
        IS_SET(obj->obj_flags.extra_flags2,ITEM_EQ_DECAY)) &&
        object.timer==0) {
      /* Timer is as assigned in read_object */
      /* Allows the addition of timers to popped objects for decay items*/
    }
    else {
      obj->obj_flags.timer    =object.timer;
    }
    obj->obj_flags.value[0]   =object.value[0];
    obj->obj_flags.value[1]   =object.value[1];
    obj->obj_flags.value[2]   =object.value[2];
    if(obj->obj_flags.type_flag != ITEM_CONTAINER)
      obj->obj_flags.value[3]   =object.value[3];
    obj->obj_flags.extra_flags=object.extra_flags;
    obj->obj_flags.weight     =object.weight;
    obj->obj_flags.bitvector  =object.bitvector;
    for(i=0;i<OFILE_MAX_OBJ_AFFECT;i++) {
      obj->affected[i].location = (int)object.affected[i].location;
      obj->affected[i].modifier = (int)object.affected[i].modifier;
    }

    if (obj->obj_flags.type_flag == ITEM_TICKET) {
      if(!adjust_ticket_strings(obj)) return 0;
    }

    if(obj->obj_flags.type_flag == ITEM_CONTAINER) {
      for(i=0;i<object.value[3];i++)
        obj_to_obj(store_to_obj_ver0(fl,ch),obj);
    }
    if(rent_equip_char( ch, obj, object.position))
          obj = NULL;  /*  make it so it will return null, so load_char
                        *  wont add it to the inventory.
                        */
  }
  else  {
    if(object.item_number != 0)
      log_f("BUG: No such item #%d",object.item_number);
  }
  return obj;
}

int cost_need(struct obj_cost *cost) {
  return (cost->total_cost);
}

/* write the vital data of a player to the player file */
void save_char(CHAR *ch, sh_int load_room) {
  char buf[100];
  struct char_file_u_5  char_info;
  struct obj_data *tmp;
  FILE *fl;
  int i;
  char tmp_name[50];

  if(IS_NPC(ch)) /* NPC's don't save. */
      return;
  if(GET_LEVEL(ch)<LEVEL_IMM) rank_char(ch);

  memset(&char_info,0,sizeof(char_info));

  char_to_store(ch,&char_info);

  sprintf(tmp_name,"%s",GET_NAME(ch));
  string_to_lower(tmp_name);

  sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);
  if(!(fl=fopen(buf,"wb+"))) {
    log_s("Problem opening file (save_char)");
    log_f("Error--can't open file %s for writing", buf);
    return;
  }
  for (i=0;i<MAX_WEAR;i++)
    char_info.total_cost += total_cost_of_obj(ch->equipment[i]);
  for (tmp = ch->carrying;tmp;tmp=tmp->next_content)
    char_info.total_cost += total_cost_of_obj(tmp);
  char_info.version=5;
  char_info.load_room = load_room;
  char_info.last_update = time(0);
  fwrite(&char_info,sizeof(char_info),1,fl);

  for(i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i]) {
      obj_to_store(ch->equipment[i], fl, ch,i,FALSE);
    }
  }
  for (tmp = ch->carrying;tmp;tmp=tmp->next_content)
    obj_to_store(tmp, fl, ch,-1,FALSE);

  fclose(fl);
  check_equipment(ch);
}

/* ************************************************************************
* Routine Receptionist                                                    *
************************************************************************* */

int receptionist(CHAR *recep,CHAR *ch, int cmd, char *arg) {
  char buf[240], buf2[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  struct obj_data *obj;
  struct obj_cost cost;
  struct char_data *victim=NULL;
  struct follow_type *k,*next_fol;
	OBJ *tmp_obj;

  void do_action(CHAR *ch, char *argument, int cmd);
  int number(int from, int to);

  if(!recep) {
    log_s("No receptionist.\n\r");
    produce_core();
  }

  if((cmd != CMD_RENT) && (cmd != CMD_OFFER)) return(FALSE);
  if(IS_NPC(ch)) return(FALSE);

  if(!ch->desc) return(FALSE); /* You've forgot FALSE - NPC couldn't leave */

  if(!AWAKE(recep)) {
    act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
    return(TRUE);
  }

  if(!CAN_SEE(recep, ch) && GET_LEVEL(ch)<LEVEL_IMM) {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
    return(TRUE);
  }

  if (IS_SET(ch->specials.pflag, PLR_KILL) || IS_SET(ch->specials.pflag, PLR_THIEF)) {
    act("$n screams 'I don't deal with a killer or thief!'",FALSE,recep,0,0,TO_ROOM);
    return(TRUE);
  }

  if (GET_POS(ch) == POSITION_RIDING) {
    act ("$n says, 'Please dismount first.'", FALSE, recep, 0, ch, TO_VICT);
    return(TRUE);
  }

  money_check(ch);

  if (cmd == CMD_RENT) { /* Rent  */
    if (recep_offer(ch, recep, &cost)) {
      for (k=ch->followers; k; k= next_fol) {
        next_fol = k->next;
        if(CHAR_REAL_ROOM(ch)==CHAR_REAL_ROOM(k->follower) &&
          IS_NPC(k->follower) && k->follower->master==ch) {
          victim = k->follower;
          stop_follower(victim);
          char_from_room(victim);
          extract_char(victim);
        }
      }
      GET_POS(ch)=POSITION_STANDING;
      if(GET_BANK(ch)>(3*cost_need(&cost)/2))
        act("$n tells you, 'I see you have enough gold in your bank account to prevent\n\rlosing your equipment in rent - very wise.",0,recep,0,ch,TO_VICT);
      else
        act("$n tells you, 'If you can afford it, putting 1.5x your rent in the bank\n\rwill prevent a possible loss of your equipment.",0,recep,0,ch,TO_VICT);

      if(ch->quest_status==QUEST_FAILED) {
        printf_to_char(ch,"You have failed your quest, you can start another in %d ticks.\n\r",ch->ver3.time_to_quest);
      }
      if(ch->quest_status==QUEST_RUNNING || ch->quest_status==QUEST_COMPLETED) {
        int ttq = MAX(ch->ver3.time_to_quest - 40, 5);
        snprintf(buf, sizeof(buf), "Your quest has been automatically ended.  You can start another in %d ticks.\n\r", ttq);
        send_to_char(buf, ch);
        ch->ver3.time_to_quest=30;
      }

      ch->questgiver=0;
      if(ch->questobj)
  		{
				if(V_OBJ(ch->questobj) == 35)
				{
					for(tmp_obj = object_list; tmp_obj; tmp_obj = tmp_obj->next)
					{
						if(V_OBJ(tmp_obj) != 35) continue; //not a questcard? skip
						if(OBJ_SPEC(tmp_obj) != ch->ver3.id) continue; //not the char's questcard? skip
						extract_obj(tmp_obj);
					}
				}
				else
					ch->questobj->owned_by=0;
			}
      ch->questobj=0;
      if(ch->questmob) ch->questmob->questowner=0;
      ch->questmob=0;
      ch->quest_status=QUEST_NONE;
      ch->quest_level=0;

      act("$n stores your stuff in the safe, and helps you into your chamber.",
           FALSE, recep, 0, ch, TO_VICT);
      act("$n helps $N into $S private chamber.",FALSE, recep,0,ch,TO_NOTVICT);
      sprintf(buf2, "%s(%d) has rent with %d coins and needs %d per day.",
              GET_NAME(ch), GET_LEVEL(ch), GET_GOLD(ch), cost_need(&cost));
      wizlog(buf2, GET_LEVEL(ch), 3);
      log_s(buf2);



      if ( (GET_LEVEL(ch) < LEVEL_IMM) && IS_SET(ch->specials.pflag, PLR_QUEST)) REMOVE_BIT(ch->specials.pflag, PLR_QUEST);
      /* Ranger - June 96 */
      if ( (GET_LEVEL(ch) < LEVEL_IMM) && IS_SET(ch->specials.pflag, PLR_QUIET)) REMOVE_BIT(ch->specials.pflag, PLR_QUIET);
      save_char(ch, world[CHAR_REAL_ROOM(ch)].number);
      strip_char(ch);
      extract_char(ch);
    }

  } else {         /* Offer */
    arg = one_argument(arg, name);
    if(!*name)
      recep_offer(ch, recep, &cost);
    else {
      if(!(obj = get_obj_in_list_vis(ch, name, ch->carrying)))
        act("$N tells you 'but you aren't carrying that object!'",0,ch,0,recep,TO_CHAR);
      else {
        if(!IS_RENTABLE(obj))
          act("$N tells you 'You cannot rent that object!'",0,ch,0,recep,TO_CHAR);
        else
          sprintf(buf,"$N tells you 'That object's rent is %d coins/day.'",obj->obj_flags.cost_per_day);
        act(buf,0,ch,0,recep,TO_CHAR);
      }
    }
    act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
  }
  return(TRUE);
}

/* ************************************************************************
* Routines to autorent                                                    *
************************************************************************* */

void auto_rent(CHAR *ch) {
  int i;
  char buf[MAX_STRING_LENGTH];
  struct obj_cost cost;
	OBJ *tmp_obj;

  if (IS_NPC(ch)) return;

  if(world[CHAR_REAL_ROOM(ch)].number != NOWHERE) char_from_room(ch);
  if (ch->specials.was_in_room != NOWHERE) ch->specials.was_in_room = NOWHERE;
  if(ch->player.level>LEVEL_MORT) char_to_room(ch,real_room(1212));
  else char_to_room(ch, real_room(3008));

  corpse_check(ch);

  cost.total_cost = 1; /* Minimum cost */
  cost.no_carried = 0;
  cost.ok = TRUE;        /* Use if any "-1" objects */

  add_obj_cost(ch, ch->carrying, &cost);

  for(i = 0; i<MAX_WEAR; i++)
    add_obj_cost(ch, ch->equipment[i], &cost);

  if (cost.no_carried == 0) {
    sprintf(buf, "Autorent: %s didn't have eq to rent", GET_NAME(ch));
    log_s(buf);
  }

  if(ch->quest_status==QUEST_RUNNING || ch->quest_status==QUEST_COMPLETED)
    ch->ver3.time_to_quest = MAX(ch->ver3.time_to_quest - 40, 5);

  ch->questgiver=0;
  if(ch->questobj)
  {
		if(V_OBJ(ch->questobj) == 35)
		{
			for(tmp_obj = object_list; tmp_obj; tmp_obj = tmp_obj->next)
			{
				if(V_OBJ(tmp_obj) != 35) continue; //not a questcard? skip
				if(OBJ_SPEC(tmp_obj) != ch->ver3.id) continue; //not the char's questcard? skip
				extract_obj(tmp_obj);
			}
		}
		else
			ch->questobj->owned_by=0;
	}
  ch->questobj=0;
  if(ch->questmob) ch->questmob->questowner=0;
  ch->questmob=0;
  ch->quest_status=QUEST_NONE;
  ch->quest_level=0;
  signal_char(ch, ch, MSG_AUTORENT, "");

  save_char(ch, world[CHAR_REAL_ROOM(ch)].number);
  strip_char(ch);
  extract_char(ch);
  sprintf(buf,"Autorent: %s with %d coins!",ch->player.name,ch->points.gold);
  log_s(buf);
  if(ch->desc) {
    close_socket(ch->desc);
    ch->desc = 0;
  }
  else free(ch);
  return;
}

void autorent_all(void) {
  CHAR *i, *next_dude;

  for (i = character_list; i; i = next_dude) {
    next_dude = i->next;
    if (!IS_NPC(i)){
      auto_rent(i);
    } /* if */
  }  /* for */
}

void autosave_all(void) {
  CHAR *i, *next_dude;

  for (i = character_list; i; i = next_dude) {
    next_dude = i->next;
    if (!IS_NPC(i)){
      log_f("Saving %s with %d coins.",GET_NAME(i),GET_GOLD(i));
      save_char(i,NOWHERE);
    } /* if */
  }  /* for */
}

/* copy data from the file structure to a char struct
   setting newer version 3&4 stuff to defaults in store_to_char_2
   Ranger - June 97
*/

void store_to_char_5(struct char_file_u_5 *st, CHAR *ch) {
  int i;
  struct affected_type_5 af;
  ENCH ench;
  size_t len = 0;

  /* Ver 3 stuff - set specifics to default in store_to_char_2*/
  ch->ver3=st->ver3;
  ch->specials.death_timer=st->ver3.death_timer;
  /* End ver 3 stuff */

  GET_SEX(ch) = st->sex;
  GET_CLASS(ch) = st->class;
  GET_LEVEL(ch) = st->level;

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;

  if (*st->title) {
    len = strnlen(st->title, FIELD_SIZE(char_file_u_5, title));
    CREATE(ch->player.title, char, len + 1);
    strncpy(ch->player.title, st->title, len);
  }
  else GET_TITLE(ch) = 0;

  if (*st->description) {
    len = strnlen(st->description, FIELD_SIZE(char_file_u_5, description));
    CREATE(ch->player.description, char, len + 1);
    strncpy(ch->player.description, st->description, len);
  } else ch->player.description = 0;

  if (*st->poofin) {
    len = strnlen(st->poofin, FIELD_SIZE(char_file_u_5, poofin));
    CREATE(ch->player.poofin, char, len + 1);
    strncpy(ch->player.poofin, st->poofin, len);
  } else ch->player.poofin = 0;

  if (*st->poofout) {
    len = strnlen(st->poofout, FIELD_SIZE(char_file_u_5, poofout));
    CREATE(ch->player.poofout, char, len + 1);
    strncpy(ch->player.poofout, st->poofout, len);
  } else ch->player.poofout = 0;

  ch->player.hometown = st->hometown;
  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;

  ch->player.time.logon = time(0);

  for (i = 0; i <= MAX_TONGUE - 1; i++)
    ch->player.talks[i] = st->talks[i];

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->abilities = st->abilities;
  ch->tmpabilities = st->abilities;
  /*points*/
  /*ch->points = st->points;*/
  ch->points.mana       =     st->points.mana;
  ch->points.max_mana   =     st->points.max_mana;
  ch->points.hit        =     st->points.hit;
  ch->points.max_hit    =     st->points.max_hit;
  ch->points.move       =     st->points.move;
  ch->points.max_move   =     st->points.max_move;
  ch->points.armor      =     st->points.armor;
  ch->points.gold       =     st->points.gold;
  ch->points.exp        =     st->points.exp;
  ch->points.bank       =     st->points.bank;
  ch->points.hitroll    =     st->points.hitroll;
  ch->points.damroll    =     st->points.damroll;

  ch->specials.prev_max_mana=ch->points.max_mana;
  ch->specials.prev_max_hit=ch->points.max_hit;
  ch->specials.prev_max_move=ch->points.max_move;
  ch->specials.org_hit=ch->points.max_hit;
  ch->specials.org_mana=ch->points.max_mana;
  ch->specials.org_move=ch->points.max_move;

  ch->new    = st->new;
  if(ch->desc) ch->desc->prompt = st->new.prompt;
  for (i = 0; i <= MAX_SKILLS5 - 1; i++)
    ch->skills[i] = st->skills[i];


  ch->specials.spells_to_learn = st->spells_to_learn;
  ch->specials.alignment    = st->alignment;

  ch->specials.pflag        = st->pflag;
  ch->specials.carry_items  = 0;
  ch->specials.vaultaccess  = 0;
  ch->specials.wiznetlvl  = 0;
  ch->specials.message  = 0;
  ch->specials.num_fighting=0;
  ch->specials.max_num_fighting=0;
  ch->specials.zone = -1;
  ch->specials.rider  = 0;
  ch->specials.riding = 0;
  ch->specials.protecting = 0;
  ch->specials.protect_by=0;
  ch->specials.reply_to=0;
  strcpy(ch->specials.vaultname,"not-set");
  ch->points.armor          = 100;
  ch->points.hitroll        = 0;
  ch->points.damroll        = 0;
  ch->bot.meta_update = 0;
  ch->bot.meta_number=0;
  ch->bot.meta_amount=0;
  ch->bot.misses=0;
  ch->questgiver = 0;
  ch->questmob = 0;
  ch->questobj = 0;
  ch->questowner = 0;
  ch->quest_status = 0;
  ch->quest_level = 0;

  len = strnlen(st->name, FIELD_SIZE(char_file_u_5, name));
  CREATE(ch->player.name, char, len + 1);
  strncpy(ch->player.name, st->name, len);

  /* Not used as far as I can see (Michael) */
  for(i = 0; i <= 4; i++)
    ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];

  for (i = 0; i <= 2; i++) {
    GET_COND(ch, i) = MAX(0, st->conditions[i]);
  }

  if (IS_IMMORTAL(ch) ||
      GET_CLASS(ch) == CLASS_AVATAR ||
      GET_PRESTIGE_PERK(ch) >= 26) { // Prestige Perk 26
    GET_COND(ch, FULL) = -1;
    GET_COND(ch, THIRST) = -1;
  }

  if (IS_IMMORTAL(ch)) {
    GET_COND(ch, DRUNK) = -1;
  }

  /* Add all spell effects */
  for(i=0; i < MAX_AFFECT; i++)
    {
    if (st->affected[i].type)
      {
      af.type     = st->affected[i].type;
      af.duration = st->affected[i].duration;
      af.modifier = st->affected[i].modifier;
      af.location = st->affected[i].location;
      af.bitvector= st->affected[i].bitvector;
      af.bitvector2= st->affected[i].bitvector2;
      affect_to_char(ch, &af);
      }
    }
  for(i=0; i < MAX_AFFECT; i++) {
    if (st->enchantments[i].type)
      {
      ench.name     = NULL;
      ench.type     = st->enchantments[i].type;
      ench.duration = st->enchantments[i].duration;
      ench.modifier = st->enchantments[i].modifier;
      ench.location = st->enchantments[i].location;
      ench.bitvector= st->enchantments[i].bitvector;
      ench.bitvector2= st->enchantments[i].bitvector2;
      ench.func     = NULL;
      enchantment_to_char(ch, &ench, TRUE);
      }
  }
  ch->in_room_v = st->load_room;
  ch->in_room_r = real_room(ch->in_room_v);
  affect_total(ch);
  for(i=0;i<MAX_COLORS;i++)
    ch->colors[i]=st->colors[i];
  sprintf(ch->pwd,"%s",st->pwd);
} /* store_to_char_5 */

void store_to_char_4(struct char_file_u_4 *st, CHAR *ch) {
  int i;
  struct affected_type_5 af;
  ENCH ench;
  size_t len = 0;

  /* Ver 3 stuff - set specifics to default in store_to_char_2*/
  ch->ver3=st->ver3;
  ch->specials.death_timer=st->ver3.death_timer;
  /* End ver 3 stuff */

  GET_SEX(ch) = st->sex;
  GET_CLASS(ch) = st->class;
  GET_LEVEL(ch) = st->level;

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;

  if (*st->title) {
    len = strnlen(st->title, FIELD_SIZE(char_file_u_4, title));
    CREATE(ch->player.title, char, len + 1);
    strncpy(ch->player.title, st->title, len);
  }
  else GET_TITLE(ch) = 0;

  if (*st->description) {
    len = strnlen(st->description, FIELD_SIZE(char_file_u_4, description));
    CREATE(ch->player.description, char, len + 1);
    strncpy(ch->player.description, st->description, len);
  } else ch->player.description = 0;

  if (*st->poofin) {
    len = strnlen(st->poofin, FIELD_SIZE(char_file_u_4, poofin));
    CREATE(ch->player.poofin, char, len + 1);
    strncpy(ch->player.poofin, st->poofin, len);
  } else ch->player.poofin = 0;

  if (*st->poofout) {
    len = strnlen(st->poofout, FIELD_SIZE(char_file_u_4, poofout));
    CREATE(ch->player.poofout, char, len + 1);
    strncpy(ch->player.poofout, st->poofout, len);
  } else ch->player.poofout = 0;

  ch->player.hometown = st->hometown;
  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;

  ch->player.time.logon = time(0);

  for (i = 0; i <= MAX_TONGUE - 1; i++)
    ch->player.talks[i] = st->talks[i];

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->abilities = st->abilities;
  ch->tmpabilities = st->abilities;
  /*points*/
  /*ch->points = st->points;*/
  ch->points.mana       =     st->points.mana;
  ch->points.max_mana   =     st->points.max_mana;
  ch->points.hit        =     st->points.hit;
  ch->points.max_hit    =     st->points.max_hit;
  ch->points.move       =     st->points.move;
  ch->points.max_move   =     st->points.max_move;
  ch->points.armor      =     st->points.armor;
  ch->points.gold       =     st->points.gold;
  ch->points.exp        =     st->points.exp;
  ch->points.bank       =     st->points.bank;
  ch->points.hitroll    =     st->points.hitroll;
  ch->points.damroll    =     st->points.damroll;

  ch->specials.prev_max_mana=ch->points.max_mana;
  ch->specials.prev_max_hit=ch->points.max_hit;
  ch->specials.prev_max_move=ch->points.max_move;
  ch->specials.org_hit=ch->points.max_hit;
  ch->specials.org_mana=ch->points.max_mana;
  ch->specials.org_move=ch->points.max_move;

  ch->new    = st->new;
  if(ch->desc) ch->desc->prompt = st->new.prompt;
  for (i = 0; i <= MAX_SKILLS4 - 1; i++)
    ch->skills[i] = st->skills[i];
  for (i = MAX_SKILLS4; i <= MAX_SKILLS5 - 1; i++)
    ch->skills[i].learned = 0;

  ch->specials.spells_to_learn = st->spells_to_learn;
  ch->specials.alignment    = st->alignment;

  ch->specials.pflag        = st->pflag;
  ch->specials.carry_items  = 0;
  ch->specials.vaultaccess  = 0;
  ch->specials.wiznetlvl  = 0;
  ch->specials.message  = 0;
  ch->specials.num_fighting=0;
  ch->specials.max_num_fighting=0;
  ch->specials.zone = -1;
  ch->specials.rider  = 0;
  ch->specials.riding = 0;
  ch->specials.protecting = 0;
  ch->specials.protect_by=0;
  ch->specials.reply_to=0;
  strcpy(ch->specials.vaultname,"not-set");
  ch->points.armor          = 100;
  ch->points.hitroll        = 0;
  ch->points.damroll        = 0;
  ch->bot.meta_update = 0;
  ch->bot.meta_number=0;
  ch->bot.meta_amount=0;
  ch->bot.misses=0;
  ch->questgiver = 0;
  ch->questmob = 0;
  ch->questobj = 0;
  ch->questowner = 0;
  ch->quest_status = 0;
  ch->quest_level = 0;

  len = strnlen(st->name, FIELD_SIZE(char_file_u_4, name));
  CREATE(ch->player.name, char, len + 1);
  strncpy(ch->player.name, st->name, len);

  /* Not used as far as I can see (Michael) */
  for(i = 0; i <= 4; i++)
    ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];

  for(i = 0; i <= 2; i++)
    {
    GET_COND(ch, i) = st->conditions[i];
    if(GET_LEVEL(ch)<LEVEL_IMM && GET_CLASS(ch)!=CLASS_AVATAR)
      GET_COND(ch,i)=MAX(0,GET_COND(ch,i));
    }

  /* Add all spell effects */
  for(i=0; i < MAX_AFFECT; i++)
    {
    if (st->affected[i].type)
      {
      af.type     = st->affected[i].type;
      af.duration = st->affected[i].duration;
      af.modifier = st->affected[i].modifier;
      af.location = st->affected[i].location;
      af.bitvector= st->affected[i].bitvector;
      af.bitvector2= 0;
      affect_to_char(ch, &af);
      }
    }
  for(i=0; i < MAX_AFFECT; i++) {
    if (st->enchantments[i].type)
      {
      ench.name     = NULL;
      ench.type     = st->enchantments[i].type;
      ench.duration = st->enchantments[i].duration;
      ench.modifier = st->enchantments[i].modifier;
      ench.location = st->enchantments[i].location;
      ench.bitvector= st->enchantments[i].bitvector;
      ench.bitvector2= 0;
      ench.func     = NULL;
      enchantment_to_char(ch, &ench,TRUE);
      }
  }
  ch->in_room_v = st->load_room;
  ch->in_room_r = real_room(ch->in_room_v);
  affect_total(ch);
  for(i=0;i<MAX_COLORS;i++)
    ch->colors[i]=st->colors[i];
  sprintf(ch->pwd,"%s",st->pwd);
} /* store_to_char_4 */


/* copy data from the file structure to a char struct
   old version 2
   setting newer version 3&4 specific stuff to defaults
   Ranger - June 97
*/
void store_to_char_2(struct char_file_u_2 *st, CHAR *ch) {
  int i;
  struct affected_type_5 af;
  ENCH ench;

  /* Ver 3 stuff */
  ch->ver3.clan_num=0;
  ch->ver3.death_limit=0;
  ch->ver3.subclass=0;
  ch->ver3.subclass_level=0;
  ch->ver3.subclass_points=0;
  ch->ver3.bleed_limit=0;
  ch->ver3.time_to_quest=0;
  ch->ver3.quest_points=0;
  ch->ver3.id=0;
  ch->ver3.sc_style=0;
  ch->ver3.death_timer=0;
  ch->specials.death_timer=0;
  /* End ver 3 stuff */

  GET_SEX(ch) = st->sex;
  GET_CLASS(ch) = st->class;
  GET_LEVEL(ch) = st->level;

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;

  if (*st->title) {
    CREATE(ch->player.title, char, strnlen(st->title, 80) + 1);
    strncpy(ch->player.title, st->title, 80);
  }
  else GET_TITLE(ch) = 0;

  if (*st->description) {
    CREATE(ch->player.description, char,
     strlen(st->description) + 1);
    strcpy(ch->player.description, st->description);
  }
  else ch->player.description = 0;

  if (*st->poofin) {
    CREATE(ch->player.poofin, char, strlen(st->poofin) + 1);
    strcpy(ch->player.poofin, st->poofin);
  } else
    ch->player.poofin = 0;

  if (*st->poofout) {
    CREATE(ch->player.poofout, char, strlen(st->poofout) + 1);
    strcpy(ch->player.poofout, st->poofout);
  } else
    ch->player.poofout = 0;

  ch->player.hometown = st->hometown;
  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;

  ch->player.time.logon = time(0);

  for (i = 0; i <= MAX_TONGUE - 1; i++)
    ch->player.talks[i] = st->talks[i];

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->abilities = st->abilities;
  ch->tmpabilities = st->abilities;
  /*points*/
  /*ch->points = st->points;*/
  ch->points.mana       =     st->points.mana;
  ch->points.max_mana   =     st->points.max_mana;
  ch->points.hit        =     st->points.hit;
  ch->points.max_hit    =     st->points.max_hit;
  ch->points.move       =     st->points.move;
  ch->points.max_move   =     st->points.max_move;
  ch->points.armor      =     st->points.armor;
  ch->points.gold       =     st->points.gold;
  ch->points.exp        =     st->points.exp;
  ch->points.bank       =     st->points.bank;
  ch->points.hitroll    =     st->points.hitroll;
  ch->points.damroll    =     st->points.damroll;

  ch->specials.prev_max_mana=ch->points.max_mana;
  ch->specials.prev_max_hit=ch->points.max_hit;
  ch->specials.prev_max_move=ch->points.max_move;
  ch->specials.org_hit=ch->points.max_hit;
  ch->specials.org_mana=ch->points.max_mana;
  ch->specials.org_move=ch->points.max_move;

  ch->new    = st->new;
  if(ch->desc) ch->desc->prompt = st->new.prompt;
  for (i = 0; i <= MAX_SKILLS4 - 1; i++)
    ch->skills[i] = st->skills[i];
  for (i = MAX_SKILLS4; i <= MAX_SKILLS5 - 1; i++)
    ch->skills[i].learned = 0;

  ch->specials.spells_to_learn = st->spells_to_learn;
  ch->specials.alignment    = st->alignment;

  ch->specials.pflag        = st->pflag;
  ch->specials.carry_items  = 0;
  ch->specials.vaultaccess  = 0;
  ch->specials.wiznetlvl  = 0;
  ch->specials.message  = 0;
  ch->specials.zone = -1;
  ch->specials.rider  = 0;
  ch->specials.riding = 0;
  ch->specials.protecting = 0;
  ch->specials.protect_by=0;
  ch->specials.num_fighting=0;
  ch->specials.max_num_fighting=0;
  ch->specials.reply_to=0;
  strcpy(ch->specials.vaultname,"not-set");
  ch->points.armor          = 100;
  ch->points.hitroll        = 0;
  ch->points.damroll        = 0;
  ch->bot.meta_update = 0;
  ch->bot.meta_number=0;
  ch->bot.meta_amount=0;
  ch->bot.misses=0;
  ch->questgiver = 0;
  ch->questmob = 0;
  ch->questobj = 0;
  ch->questowner = 0;
  ch->quest_status = 0;
  ch->quest_level = 0;

  CREATE(ch->player.name, char, strlen(st->name) +1);
  strcpy(ch->player.name, st->name);

  /* Not used as far as I can see (Michael) */
  for(i = 0; i <= 4; i++)
    ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];

  for(i = 0; i <= 2; i++)
    {
    GET_COND(ch, i) = st->conditions[i];
    if(GET_LEVEL(ch)<LEVEL_IMM && GET_CLASS(ch)!=CLASS_AVATAR)
      GET_COND(ch,i)=MAX(0,GET_COND(ch,i));
    }

  /* Add all spell effects */
  for(i=0; i < MAX_AFFECT; i++)
    {
    if (st->affected[i].type)
      {
      af.type     = st->affected[i].type;
      af.duration = st->affected[i].duration;
      af.modifier = st->affected[i].modifier;
      af.location = st->affected[i].location;
      af.bitvector= st->affected[i].bitvector;
      af.bitvector2= 0;
      affect_to_char(ch, &af);
      }
    }
  for(i=0; i < MAX_AFFECT; i++) {
    if (st->enchantments[i].type)
      {
      ench.name     = NULL;
      ench.type     = st->enchantments[i].type;
      ench.duration = st->enchantments[i].duration;
      ench.modifier = st->enchantments[i].modifier;
      ench.location = st->enchantments[i].location;
      ench.bitvector= st->enchantments[i].bitvector;
      ench.bitvector2= 0;
      ench.func     = NULL;
      enchantment_to_char(ch, &ench,TRUE);
      }
  }
  ch->in_room_v = st->load_room;
  ch->in_room_r = real_room(ch->in_room_v);
  affect_total(ch);
  for(i=0;i<MAX_COLORS;i++)
    ch->colors[i]=st->colors[i];
  sprintf(ch->pwd,"%s",st->pwd);
} /* store_to_char_2*/

/* copy vital data from a players char-structure to the file structure */
void char_to_store(CHAR *ch, struct char_file_u_5 *st)
{
  int i,increase;
  char buf[MAX_STRING_LENGTH];
  struct affected_type_5 *af;
  ENCH *af2;
  struct obj_data *char_eq[MAX_WEAR];

  for(i=0; i<MAX_WEAR; i++) {
    if(ch->equipment[i]) char_eq[i] = unequip_char(ch,i);
    else char_eq[i] = NULL;
  }

  /* Unaffect everything a character can be affected by */

  for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
    if (af) {
      st->affected[i] = *af;
      st->affected[i].next = 0;
      /* subtract effect of the spell or the effect will be doubled */
      affect_modify( ch, st->affected[i].location,
                         st->affected[i].modifier,
                         st->affected[i].bitvector,
                         st->affected[i].bitvector2,FALSE);
      af = af->next;
    } else {
      st->affected[i].type = 0;  /* Zero signifies not used */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].bitvector2 = 0;
      st->affected[i].next = 0;
    }
  }

  for(af2 = ch->enchantments, i = 0; i<MAX_AFFECT; i++) {
    if (af2) {
      st->enchantments[i].type = af2->type;  /* Zero signifies not used */
      st->enchantments[i].duration = af2->duration;
      st->enchantments[i].modifier = af2->modifier;
      st->enchantments[i].location = af2->location;
      st->enchantments[i].bitvector = af2->bitvector;
      st->enchantments[i].bitvector2 = af2->bitvector2;
      st->enchantments[i].next = 0;
      /* subtract effect of the spell or the effect will be doubled */
      affect_modify( ch, st->enchantments[i].location,
                         st->enchantments[i].modifier,
                         st->enchantments[i].bitvector,
                         st->enchantments[i].bitvector2,FALSE);
      af2 = af2->next;
    } else {
      st->enchantments[i].type = 0;  /* Zero signifies not used */
      st->enchantments[i].duration = 0;
      st->enchantments[i].modifier = 0;
      st->enchantments[i].location = 0;
      st->enchantments[i].bitvector = 0;
      st->enchantments[i].bitvector2 = 0;
      st->enchantments[i].next = 0;
    }
  }

  if ((i >= MAX_AFFECT) && af && af->next)
    log_s("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

  /* New check for unusual increase in stats - Ranger May 98 */
  increase=ch->points.max_mana-ch->specials.prev_max_mana;
  if(increase>19 && GET_LEVEL(ch)>1) {
    sprintf(buf,"PLRINFO: WARNING %s's mana just increased by %d. (Room %d)",
      GET_NAME(ch),increase,world[CHAR_REAL_ROOM(ch)].number);
    wizlog(buf,LEVEL_SUP,4);
    log_s(buf);
  }
  increase=ch->points.max_hit-ch->specials.prev_max_hit;
  if(increase>39 && GET_LEVEL(ch)>1) {
    sprintf(buf,"PLRINFO: WARNING %s's hps just increased by %d. (Room %d)",
      GET_NAME(ch),increase,world[CHAR_REAL_ROOM(ch)].number);
    wizlog(buf,LEVEL_SUP,4);
    log_s(buf);
  }
  increase=ch->points.max_move-ch->specials.prev_max_move;
  if(increase>30 && GET_LEVEL(ch)>1) {
    sprintf(buf,"PLRINFO: WARNING %s's move just increased by %d. (Room %d)",
      GET_NAME(ch),increase,world[CHAR_REAL_ROOM(ch)].number);
    wizlog(buf,LEVEL_SUP,4);
    log_s(buf);
  }

  ch->specials.prev_max_mana=ch->points.max_mana;
  ch->specials.prev_max_hit=ch->points.max_hit;
  ch->specials.prev_max_move=ch->points.max_move;
  ch->specials.org_hit=ch->points.max_hit;
  ch->specials.org_mana=ch->points.max_mana;
  ch->specials.org_move=ch->points.max_move;

  ch->tmpabilities = ch->abilities;

  st->birth      = ch->player.time.birth;
  st->played     = ch->player.time.played;
  st->played    += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);

  st->load_room=ch->in_room_v;
  st->hometown = ch->player.hometown;
  st->weight   = GET_WEIGHT(ch);
  st->height   = GET_HEIGHT(ch);
  st->sex      = GET_SEX(ch);
  st->class    = GET_CLASS(ch);
  st->level    = GET_LEVEL(ch);
  st->abilities = ch->abilities;
  /*points*/
  /*st->points    = ch->points;*/
  st->points.mana       =     ch->points.mana;
  st->points.max_mana   =     ch->points.max_mana;
  st->points.hit        =     ch->points.hit;
  st->points.max_hit    =     ch->points.max_hit;
  st->points.move       =     ch->points.move;
  st->points.max_move   =     ch->points.max_move;
  st->points.armor      =     0;
  st->points.gold       =     ch->points.gold;
  st->points.exp        =     ch->points.exp;
  st->points.bank       =     ch->points.bank;
  st->points.hitroll    =     0;
  st->points.damroll    =     0;

  st->new       = ch->new;
  st->ver3      = ch->ver3;
  st->ver3.death_timer = ch->specials.death_timer;
  if (ch->desc) st->new.prompt = ch->desc->prompt;
  st->alignment       = ch->specials.alignment;
  st->spells_to_learn = ch->specials.spells_to_learn;
  st->pflag           = ch->specials.pflag;

  if (GET_TITLE(ch))
    strncpy(st->title, GET_TITLE(ch), 80);
  else
    *st->title = '\0';

  if (ch->player.description)
    strncpy(st->description, ch->player.description, 240);
  else
    *st->description = '\0';

  *st->poofin = '\0';
  if (ch->player.poofin)
    str_cat(st->poofin, sizeof(st->poofin), ch->player.poofin);

  *st->poofout = '\0';
  if (ch->player.poofout)
    str_cat(st->poofout, sizeof(st->poofout), ch->player.poofout);


  for (i = 0; i <= MAX_TONGUE - 1; i++)
    st->talks[i] = ch->player.talks[i];

  for (i = 0; i <= MAX_SKILLS5 - 1; i++)
    st->skills[i] = ch->skills[i];

  strcpy(st->name, GET_NAME(ch) );

  for(i = 0; i <= 4; i++)
    st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

  for(i = 0; i <= 2; i++)
    st->conditions[i] = GET_COND(ch, i);

  ch->points.armor=100;

  for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
    if (af) {
      /* Add effect of the spell or it will be lost */
      /* When saving without quitting               */
      affect_modify( ch, st->affected[i].location,
                         st->affected[i].modifier,
                         st->affected[i].bitvector,
                         st->affected[i].bitvector2, TRUE);
      af = af->next;
    }
  }

  for(af2 = ch->enchantments, i = 0; i<MAX_AFFECT; i++) {
    if (af2) {
      /* Add effect of the spell or it will be lost */
      /* When saving without quitting               */
      affect_modify( ch, st->enchantments[i].location,
                         st->enchantments[i].modifier,
                         st->enchantments[i].bitvector,
                         st->enchantments[i].bitvector2, TRUE);
      af2 = af2->next;
    }
  }

/* Weapon falls from grasp because of weakness when the strength
required is from a held item.  Therefore, held item equipped before
wield - Ranger Aug 96*/



  for(i=0; i<WIELD; i++) {
    if (char_eq[i])
      equip_char(ch, char_eq[i], i);
  }
  if (char_eq[HOLD]) equip_char(ch, char_eq[HOLD], HOLD);
  if (char_eq[WIELD]) equip_char(ch, char_eq[WIELD], WIELD);

  affect_total(ch);
  for(i=0;i<MAX_COLORS;i++)
    st->colors[i]=ch->colors[i];
  sprintf(st->pwd,"%s",ch->pwd);
} /* Char to store */

void do_crent(CHAR *ch, char *argument, int cmd)
{
   return;
}

/* Procedures logon and logoff
** written for RoninMUD by Ranger
** do not distrubute without permission of originator
** Last Modification - May 14/97
*/

void do_logoff_char(CHAR *ch, char *argument, int cmd) {
  char tmp_name[50],ch_name[50],buf[MAX_INPUT_LENGTH];
  CHAR *i, *next_dude;
  int found=0;
  if(!check_god_access(ch,TRUE)) return;

  if(IS_MOB(ch)) return;

  one_argument(argument, tmp_name);
  if (!*tmp_name) {
    send_to_char("Usage: logoff <charname>\n\r",ch);
    return;
  }
  string_to_lower(tmp_name);

  for (i = character_list; i; i = next_dude) {
    next_dude = i->next;
    if(!IS_NPC(i)) {
      sprintf(ch_name,"%s",GET_NAME(i));
      string_to_lower(ch_name);
      if(!strcmp(tmp_name,ch_name)) {
        found=TRUE;
        auto_rent(i);
        break;
      }
    }
  }
  if(found) {
    send_to_char("Character logged off.\n\r",ch);
    sprintf(buf,"WIZINFO: %s logged off %s.",GET_NAME(ch),CAP(tmp_name));
    wizlog(buf,GET_LEVEL(ch)+1,5);
    log_s(buf);
  } else send_to_char("Character not found.\n\r",ch);
}

void do_logon_char(CHAR *ch, char *argument, int cmd) {
  FILE *fl;
  int version;
  struct char_file_u_5 char_data_5;
  struct char_file_u_4 char_data_4;
  struct char_file_u_2 char_data_2;
  char buf[MAX_STRING_LENGTH],tmp_name[50],ch_name[50];
  CHAR *i, *next_dude,*vict;
  struct descriptor_data *d;
  long last_up = 0;

  if(IS_MOB(ch)) return;

  if(GET_LEVEL(ch)<LEVEL_IMP) {
    send_to_char("This command is IMP only.\n\r",ch);
    return;
  }
  if(!check_god_access(ch,TRUE)) return;

  one_argument(argument, tmp_name);
  if (!*tmp_name) {
    send_to_char("Usage: logon <charname>\n\r",ch);
    return;
  }

  string_to_lower(tmp_name);
  for (i = character_list; i; i = next_dude) {
    next_dude = i->next;
    if(!IS_NPC(i)) {
      sprintf(ch_name,"%s",GET_NAME(i));
      string_to_lower(ch_name);
      if(!strcmp(tmp_name,ch_name)) {
        send_to_char("Player already connected.\n\r",ch);
        return;
      }
    }
  }

  /* Double check for character, perhaps at menu */
  for(d = descriptor_list; d; d = d->next) {
    if(d->character) {
      sprintf(ch_name,"%s",GET_NAME(d->character));
      string_to_lower(ch_name);
      if(!strcmp(tmp_name,ch_name)) {
        send_to_char("Player already connected.\n\r",ch);
        return;
      }
    }
  }

  sprintf(buf,"cp rent/%c/%s.dat rent/%c/%s.bak",UPPER(tmp_name[0]),tmp_name,UPPER(tmp_name[0]),tmp_name);
  system(buf);
  sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);
  if (!(fl = fopen(buf, "rb"))) {
    sprintf(buf,"%s didnt have .dat file.\n\r",CAP(tmp_name));
    send_to_char(buf,ch);
    return;
  }

  version=char_version(fl);

  switch(version) {
    case 2:
      if((fread(&char_data_2,sizeof(struct char_file_u_2),1,fl))!=1)
      {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
      last_up=char_data_2.last_update;
      break;
    case 3:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
      {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
      last_up=char_data_4.last_update;
      break;
    case 4:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
      {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
      last_up=char_data_4.last_update;
      break;
    case 5:
      if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1)
      {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
      last_up=char_data_5.last_update;
      break;
    default:
      log_s("Error getting pfile version (logon_char)");
      return;
  }

  CREATE(vict, CHAR, 1);
  clear_char(vict);
  vict->desc = 0;
  CREATE(vict->skills, struct char_skill_data, MAX_SKILLS5);
  clear_skills(vict->skills);
  reset_char(vict);

  switch (version) {
    case 2:
      store_to_char_2(&char_data_2,vict);
      break;
    case 3:
      store_to_char_4(&char_data_4,vict);
      /* Changes for 50 level update */
      GET_EXP(vict)=0;
      if(GET_LEVEL(vict)==35 && IS_SET(vict->new.imm_flags, WIZ_ACTIVE)) GET_LEVEL(vict)=LEVEL_IMP;
      else if(GET_LEVEL(vict)>=31) GET_LEVEL(vict)=LEVEL_IMM;
      break;
    case 4:
      store_to_char_4(&char_data_4,vict);
      break;
    case 5:
      store_to_char_5(&char_data_5,vict);
      break;
    default:
      log_s("Version number corrupted? (logon_char)");
      return;
  }

  while (!feof(fl)) {
    switch(obj_version(fl)) {
      case 3:
        obj_to_char(store_to_obj_ver3(fl,vict),vict);
        break;
      case 2:
        obj_to_char(store_to_obj_ver2(fl,vict),vict);
        break;
      case 1:
        obj_to_char(store_to_obj_ver1(fl,vict),vict);
        break;
      case 0:
        obj_to_char(store_to_obj_ver0(fl,vict),vict);
        break;
    }
  }

  fclose(fl);

  sprintf(buf,"%ld",time(0)-last_up);
  if(signal_char(vict,vict,MSG_OBJ_ENTERING_GAME,buf))
    log_s("Error: Return TRUE from MSG_OBJ_ENTERING_GAME");

  save_char(vict, NOWHERE);
  vict->next = character_list;
  character_list = vict;
  char_to_room(vict, CHAR_REAL_ROOM(ch));
  if(GET_LEVEL(vict)<LEVEL_IMM) rank_char(vict);

  send_to_char("Character loaded.\n\r",ch);
  sprintf(buf,"WIZINFO: %s logged on %s.",GET_NAME(ch),GET_NAME(vict));
  wizlog(buf,GET_LEVEL(ch)+1,5);
  log_s(buf);
}

