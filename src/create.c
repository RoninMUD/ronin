/* create.c
**
** Online creation routines
**
** Written for RoninMUD
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "cmd.h"
#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "shop.h"
#include "db.h"
#include "handler.h"
#include "comm.h"
#include "interpreter.h"
#include "act.h"

#define CREATE_ROOM 1
#define CREATE_MOB  2
#define CREATE_OBJ  3
#define CREATE_ZONE 4
#define CREATE_SHOP 5

#define EDIT_ROOM 1
#define EDIT_MOB  2
#define EDIT_ZONE 3
#define EDIT_OBJ  4
#define EDIT_SHOP 5

#define LEN_EXTRADESC_MAX 2048

void fix_mob_resets(int vmob,int j,int max);
void create_obj(CHAR *ch, char *arg1);
void create_mob(CHAR *ch, char *arg1);
void create_room(CHAR *ch, char *arg1);
void create_zone(CHAR *ch, char *arg1);
void create_shop(CHAR *ch, char *arg1);

void write_obj(FILE *obj_f, int nr);
void write_room(FILE *fl, int nr);
void write_mob(FILE *fl, int nr);
void write_zone(FILE *fl, int nr);
void write_shop(FILE *fl, int nr);
void fwrite_string(FILE *fl, char *str);
void save_zone(int nr);
void add_flying_room(int room);
void remove_flying_room(int room);

void fwrite_string(FILE *wfile,char *wstr)
{
  char *tempstr;
  int  i,j;
  if(wstr!=NULL)
  {
    CREATE(tempstr,char ,strlen(wstr)+20);
    j=0;
    if(strlen(wstr))
      {
      for(i=0;i<=strlen(wstr);i++)
        {
        if(wstr[i]!='\r')
          {
         tempstr[j]=wstr[i];
         j++;
          }
        }
      }
    else
      {
      sprintf(tempstr, "invalid");
      }

    fprintf(wfile,"%s~\n",tempstr);
    free(tempstr);
  }
  else
    fprintf(wfile,"~\n");
}

void print_bits_to_char(const char * const *bits, CHAR *ch) {
  if (!bits || !ch) return;

  const int wrap = 70;
  char buf[wrap + 3]; // Enough for wrap, plus "\n", "\r", and "\0".

  buf[0] = 0;

  for (int i = 0; *bits[i] != '\n'; i++) {
    if ((strlen(buf) + strlen(bits[i]) + 1) > wrap) {
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      buf[0] = 0;
    }

    strcat(buf, bits[i]);

    if ((*bits[i + 1] != '\n') && (strlen(buf) + 1 < wrap)) {
      strcat(buf, " ");
    }
  }

  if (strlen(buf) > 0) {
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
  }
}

/* The following are new olc commands to improve area creation functionality.
**
** Added by Ranger
*/

int check_olc_access(CHAR *ch) {
  if (!IS_SET(GET_IMM_FLAGS(ch), WIZ_CREATE) ||
      !IS_SET(GET_IMM_FLAGS(ch), WIZ_ACTIVE) ||
      !IS_SET(GET_IMM_FLAGS(ch), WIZ_TRUST) ||
      IS_SET(GET_IMM_FLAGS(ch), WIZ_FREEZE)) {
    send_to_char("`iYou are not allowed to use the online creation system.`q\n\r", ch);
    return FALSE;
  }
  return TRUE;
}

int check_zone_access(CHAR *ch, int zone) {
  if (GET_LEVEL(ch) > LEVEL_WIZ) return TRUE;
  if (strstr(zone_table[real_zone(zone)].creators, "ALL")) return TRUE;
  if (isname(GET_NAME(ch), zone_table[real_zone(zone)].name)) return TRUE;
  if (strstr(zone_table[real_zone(zone)].creators, GET_NAME(ch))) return TRUE;
  send_to_char("`iYou are not allowed to work on a zone without your name on it.`q\n\r", ch);
  return FALSE;
}

int check_zone_access_no_message(CHAR *ch, int zone) {
  if (GET_LEVEL(ch) > LEVEL_WIZ) return TRUE;
  if (strstr(zone_table[real_zone(zone)].creators, "ALL")) return TRUE;
  if (isname(GET_NAME(ch), zone_table[real_zone(zone)].name)) return TRUE;
  if (strstr(zone_table[real_zone(zone)].creators, GET_NAME(ch))) return TRUE;
  return FALSE;
}

int check_zone(CHAR *ch, int zone) {
  if (real_zone(zone) == -1) {
    send_to_char("`iThat zone doesn't exist.`q\n\r", ch);
    return FALSE;
  }
  return TRUE;
}

int check_room(CHAR *ch, int real_room) {
  if (real_room == -1 || real_room > top_of_world) {
    send_to_char("`iThat room doesn't exist.`q\n\r", ch);
    return FALSE;
  }
  return TRUE;
}

int check_mob(CHAR *ch, int real_mob) {
  if (real_mob == -1) {
    send_to_char("`iThat mobile doesn't exist.`q\n\r", ch);
    return FALSE;
  }
  return TRUE;
}

int check_obj(CHAR *ch, int real_obj) {
  if (real_obj == -1) {
    send_to_char("`iThat object doesn't exist.`q\n\r", ch);
    return FALSE;
  }
  return TRUE;
}

int check_zone_no_message(int zone) {
  if (real_zone(zone) == -1) {
    return FALSE;
  }
  return TRUE;
}

int check_room_no_message(int real_room) {
  if (real_room == -1 || real_room > top_of_world) {
    return FALSE;
  }
  return TRUE;
}

int check_mob_no_message(int real_mob) {
  if (real_mob == -1) {
    return FALSE;
  }
  return TRUE;
}

int check_obj_no_message(int real_obj) {
  if (real_obj == -1) {
    return FALSE;
  }
  return TRUE;
}

void do_create(CHAR* ch, char *arg, int cmd) {
  char usage[] = {
    "Usage: create <room|object|mobile|zone|shop> <number>\n\r"
  };

  if (!check_olc_access(ch)) return;

  char arg1[256], arg2[256];

  sscanf(arg, "%255s %255s", arg1, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0') {
    send_to_char(usage, ch);
    return;
  }

  if (is_abbrev(arg1, "room")) {
    create_room(ch, arg2);
  }
  else if (is_abbrev(arg1, "mobile")) {
    create_mob(ch, arg2);
  }
  else if (is_abbrev(arg1, "object")) {
    create_obj(ch, arg2);
  }
  else if (is_abbrev(arg1, "zone")) {
    create_zone(ch, arg2);
  }
  else if (is_abbrev(arg1, "shop")) {
    create_shop(ch, arg2);
  }
  else {
    send_to_char(usage, ch);
    return;
  }
}

void create_zone(CHAR *ch, char *arg1) {
  if (GET_LEVEL(ch) < LEVEL_SUP) {
    send_to_char("You may not create a zone.\r\n", ch);
    return;
  }

  int zone_num = -1;

  sscanf(arg1, "%d", &zone_num);

  if (zone_num < 0) {
    send_to_char("You must specify a zone number, and the number must be greater than 0.\n\r", ch);
    return;
  }

  if (zone_num == 320) {
    send_to_char("Zone # 320 is a reserved zone # used for object versions.\n\r", ch);
    return;
  }

  if (real_zone(zone_num) != -1) {
    send_to_char("That zone already exists.\n\r", ch);
    return;
  }

  int zone_index = allocate_zone(zone_num);

  zone_table[zone_index] = (const struct zone_data) { 0 };

  zone_table[zone_index].virtual = zone_num;

  zone_table[zone_index].name = str_dup("Temporary Zone");

  zone_table[zone_index].creators = str_dup("Unknown");

  zone_table[zone_index].top = ((zone_num * 100) + 99);
  zone_table[zone_index].bottom = (zone_num * 100);

  char timestamp[MSL];
  const long ct = time(NULL);
  struct tm *timeStruct = localtime(&ct);
  snprintf(timestamp, sizeof(timestamp), "%2d%s%4d", timeStruct->tm_mday, Month[timeStruct->tm_mon], (timeStruct->tm_year + 1900));
  zone_table[zone_index].create_date = str_dup(timestamp);
  zone_table[zone_index].mod_date = str_dup(timestamp);

  zone_table[zone_index].lifespan = 10;

  zone_table[zone_index].mult_hp = 100;
  zone_table[zone_index].mult_mana = 100;
  zone_table[zone_index].mult_hitroll = 100;
  zone_table[zone_index].mult_damage = 100;
  zone_table[zone_index].mult_armor = 100;
  zone_table[zone_index].mult_xp = 100;
  zone_table[zone_index].mult_gold = 100;
  zone_table[zone_index].mult_level = 100;

  CREATE(zone_table[zone_index].cmd, struct reset_com, 1);
  zone_table[zone_index].cmd[0].command = 'S';

  char buf[MSL];
  snprintf(buf, sizeof(buf), "%d", zone_table[zone_index].bottom);
  create_room(ch, buf);
  snprintf(buf, sizeof(buf), "%d", zone_table[zone_index].top);
  create_room(ch, buf);

  send_to_char("Zone created.\n\r", ch);
}

void create_room(CHAR *ch, char *arg1) {
  int room_num = -1;

  sscanf(arg1, "%d", &room_num);

  if (room_num < 0) {
    send_to_char("You must specify a room number, and the number must be greater than 0.\n\r", ch);
    return;
  }

  int zone_num = (room_num / 100);

  if (!check_zone(ch, zone_num)) return;
  if (!check_zone_access(ch, zone_num)) return;

  if (real_room(room_num) != -1) {
    send_to_char("That room already exists.\n\r", ch);
    return;
  }

  int room_index = allocate_room(room_num);

  world[room_index] = (const struct room_data) { 0 };

  world[room_index].number = room_num;
  world[room_index].zone = zone_num;
  world[room_index].name = str_dup("Temporary Room");
  world[room_index].description = str_dup("Temporary description.\n");

  renum_world();

  send_to_char("Room created.\n\r", ch);
}

void create_obj(CHAR *ch, char *arg1) {
  int obj_num = -1;

  sscanf(arg1, "%d", &obj_num);

  if (obj_num < 0) {
    send_to_char("You must specify an object number, and the number must be greater than 0.\n\r", ch);
    return;
  }

  int zone_num = inzone(obj_num);

  if (!check_zone(ch, zone_num)) return;
  if (!check_zone_access(ch, zone_num)) return;

  if (real_object(obj_num) != -1) {
    send_to_char("That object already exists.\n\r", ch);
    return;
  }

  int obj_index = allocate_obj(obj_num);

  obj_proto_table[obj_index] = (const struct obj_proto) { 0 };

  obj_proto_table[obj_index].virtual = obj_num;

  obj_proto_table[obj_index].name = str_dup("nothing");
  obj_proto_table[obj_index].short_description = str_dup("a bit of nothing");
  obj_proto_table[obj_index].description = str_dup("There is a bit of nothing here.");

  SET_BIT(obj_proto_table[obj_index].obj_flags.wear_flags, ITEM_TAKE);

  send_to_char("Object created.\n\r", ch);
}

void create_mob(CHAR *ch, char *arg1) {
  int mob_num = -1;

  sscanf(arg1, "%d", &mob_num);

  if (mob_num < 0) {
    send_to_char("You must specify a mobile number, and the number must be greater than 0.\n\r", ch);
    return;
  }

  int zone_num = inzone(mob_num);

  if (!check_zone(ch, zone_num)) return;
  if (!check_zone_access(ch, zone_num)) return;

  if (real_mobile(mob_num) != -1) {
    send_to_char("That mobile already exists.\n\r", ch);
    return;
  }

  int mob_index = allocate_mob(mob_num);

  mob_proto_table[mob_index] = (const struct mob_proto) { 0 };

  mob_proto_table[mob_index].virtual = mob_num;

  mob_proto_table[mob_index].name = str_dup("blank primal clay");
  mob_proto_table[mob_index].short_descr = str_dup("a blank of primal clay");
  mob_proto_table[mob_index].long_descr = str_dup("A blank of primal clay stands here, waiting to be shaped.\n\r");
  mob_proto_table[mob_index].description = str_dup("A blank of primal clay, waiting to be shaped.\n\r");

  SET_BIT(mob_proto_table[mob_index].act, ACT_ISNPC);

  mob_proto_table[mob_index].level = 1;

  mob_proto_table[mob_index].hp_nodice = 1;
  mob_proto_table[mob_index].hp_sizedice = 1;

  mob_proto_table[mob_index].mana_nodice = 1;
  mob_proto_table[mob_index].mana_sizedice = 1;

  mob_proto_table[mob_index].damnodice = 1;
  mob_proto_table[mob_index].damsizedice = 1;

  mob_proto_table[mob_index].armor = 100;

  mob_proto_table[mob_index].position = POSITION_STANDING;
  mob_proto_table[mob_index].default_pos = POSITION_STANDING;

  send_to_char("Mobile created.\n\r", ch);
}

void create_shop(CHAR *ch, char *arg1) {
  int shop_num = -1;

  sscanf(arg1, "%d", &shop_num);

  if (shop_num < 0) {
    send_to_char("You must specify a shop number, and the number must be greater than 0.\n\r", ch);
    return;
  }

  int zone_num = (shop_num / 100);

  if (!check_zone(ch, zone_num)) return;
  if (!check_zone_access(ch, zone_num)) return;

  for (int i = 0; i < number_of_shops; i++) {
    if (shop_index[i].keeper == shop_num) {
      send_to_char("That shop already exists.\n\r", ch);
      return;
    }
  }

  if (!number_of_shops) {
    CREATE(shop_index, struct shop_data, 1);
  }
  else {
    if (!(shop_index = (struct shop_data*)realloc(shop_index, ((number_of_shops + 1) * sizeof(struct shop_data))))) {
      send_to_char("Unable to create shop structure (realloc).\n\r", ch);
      return;
    }
  }

  shop_index[number_of_shops].keeper = shop_num;

  for (int i = 0; i < MAX_PROD; i++) {
    shop_index[number_of_shops].producing[i] = -1;
  }

  shop_index[number_of_shops].profit_buy = 1;
  shop_index[number_of_shops].profit_sell = 1;

  for (int i = 0; i < MAX_TRADE; i++) {
    shop_index[number_of_shops].type[i] = 0;
  }

  shop_index[number_of_shops].no_such_item1 = str_dup("%s What keeper says if it doesn't have the item.");
  shop_index[number_of_shops].no_such_item2 = str_dup("%s What keeper says if seller doesn't have the item.");
  shop_index[number_of_shops].do_not_buy = str_dup("%s What keeper says if it doesn't buy that kind of item.");
  shop_index[number_of_shops].missing_cash1 = str_dup("%s What keeper says if it doesn't have cash to buy an item.");
  shop_index[number_of_shops].missing_cash2 = str_dup("%s What keeper says if buyer doesn't have the cash to buy an item.");
  shop_index[number_of_shops].message_buy = str_dup("%s What keeper says after buyer gets item (can include %d for amount).");
  shop_index[number_of_shops].message_sell = str_dup("%s What keeper says after it buys a sellers item (can include %d for amount).");

  shop_index[number_of_shops].temper1 = 0;
  shop_index[number_of_shops].temper2 = 0;

  shop_index[number_of_shops].with_who = 0;

  shop_index[number_of_shops].in_room = 0;

  shop_index[number_of_shops].open1 = 0;
  shop_index[number_of_shops].open2 = 0;

  shop_index[number_of_shops].close1 = 28;
  shop_index[number_of_shops].close2 = 0;

  number_of_shops++;

  send_to_char("Shop created.\n\r\
Note: A mobile with the same virtual number as the shop must exist and\n\r\
this mobile must load in the room indicated in the shop structure.\n\r", ch);
}

void dsearch(char *string, char *tmp);
void write_obj(FILE *obj_f, int nr)
{
  int i;
  struct extra_descr_data *wext;
  char temptext[MSL],buf[MSL];

  if(obj_proto_table[nr].action_description) {
    sprintf(temptext,"%s",obj_proto_table[nr].action_description);
    dsearch(temptext,buf);
    obj_proto_table[nr].action_description=str_dup(buf);
  }

  fprintf(obj_f,"#%d\n",obj_proto_table[nr].virtual);
  fwrite_string(obj_f,obj_proto_table[nr].name);
  fwrite_string(obj_f,obj_proto_table[nr].short_description);
  fwrite_string(obj_f,obj_proto_table[nr].description);
  fwrite_string(obj_f,obj_proto_table[nr].action_description);

  fprintf(obj_f,"%u %u %u\n",obj_proto_table[nr].obj_flags.type_flag,obj_proto_table[nr].obj_flags.extra_flags,obj_proto_table[nr].obj_flags.wear_flags);
  fprintf(obj_f,"%d %d %d %d\n",obj_proto_table[nr].obj_flags.value[0],obj_proto_table[nr].obj_flags.value[1],obj_proto_table[nr].obj_flags.value[2],obj_proto_table[nr].obj_flags.value[3]);
  fprintf(obj_f,"%u %u %u\n",obj_proto_table[nr].obj_flags.weight,obj_proto_table[nr].obj_flags.cost,obj_proto_table[nr].obj_flags.cost_per_day);
  fprintf(obj_f,"%d\n", obj_proto_table[nr].obj_flags.repop_percent);

  wext=obj_proto_table[nr].ex_description;
  while(wext!=NULL) {
    fprintf(obj_f,"E\n");
    fwrite_string(obj_f,wext->keyword);
    fwrite_string(obj_f,wext->description);
    wext=wext->next;
  }

  fprintf(obj_f,"X\n");
  if(obj_proto_table[nr].action_description_nt) {
    sprintf(temptext,"%s",obj_proto_table[nr].action_description_nt);
    dsearch(temptext,buf);
    obj_proto_table[nr].action_description_nt=str_dup(buf);
  }
  fwrite_string(obj_f,obj_proto_table[nr].action_description_nt);

  fprintf(obj_f,"%u %u %u\n",obj_proto_table[nr].obj_flags.extra_flags2,obj_proto_table[nr].obj_flags.subclass_res,obj_proto_table[nr].obj_flags.material);

  fprintf(obj_f,"M\n");
  if(obj_proto_table[nr].char_wear_desc) {
    sprintf(temptext,"%s",obj_proto_table[nr].char_wear_desc);
    dsearch(temptext,buf);
    obj_proto_table[nr].char_wear_desc=str_dup(buf);
  }
  if(obj_proto_table[nr].room_wear_desc) {
    sprintf(temptext,"%s",obj_proto_table[nr].room_wear_desc);
    dsearch(temptext,buf);
    obj_proto_table[nr].room_wear_desc=str_dup(buf);
  }
  if(obj_proto_table[nr].char_rem_desc) {
    sprintf(temptext,"%s",obj_proto_table[nr].char_rem_desc);
    dsearch(temptext,buf);
    obj_proto_table[nr].char_rem_desc=str_dup(buf);
  }
  if(obj_proto_table[nr].room_rem_desc) {
    sprintf(temptext,"%s",obj_proto_table[nr].room_rem_desc);
    dsearch(temptext,buf);
    obj_proto_table[nr].room_rem_desc=str_dup(buf);
  }
  fwrite_string(obj_f,obj_proto_table[nr].char_wear_desc);
  fwrite_string(obj_f,obj_proto_table[nr].room_wear_desc);
  fwrite_string(obj_f,obj_proto_table[nr].char_rem_desc);
  fwrite_string(obj_f,obj_proto_table[nr].room_rem_desc);

  for( i = 0 ; (i < MAX_OBJ_AFFECT); i++) {
    if(obj_proto_table[nr].affected[i].location!=0) {
      fprintf(obj_f,"A\n");
      fprintf(obj_f,"%d %d\n",obj_proto_table[nr].affected[i].location,obj_proto_table[nr].affected[i].modifier);
    }
  }
  if(obj_proto_table[nr].obj_flags.timer>0) {
    fprintf(obj_f,"T\n");
    fprintf(obj_f,"%d\n",obj_proto_table[nr].obj_flags.timer);
  }
  if(obj_proto_table[nr].obj_flags.bitvector!=0) {
    fprintf(obj_f,"B\n");
    fprintf(obj_f,"%lu\n",obj_proto_table[nr].obj_flags.bitvector);
  }
  if(obj_proto_table[nr].obj_flags.bitvector2!=0) {
    fprintf(obj_f,"C\n");
    fprintf(obj_f,"%lu\n",obj_proto_table[nr].obj_flags.bitvector2);
  }
}


void write_room(FILE *room_f, int nr)
{
  struct extra_descr_data *wext;
  int idir;
  int tmpInfo=0;

  fprintf(room_f,"#%d\n",world[nr].number);
  fwrite_string(room_f,world[nr].name);
  fwrite_string(room_f,world[nr].description);
  fprintf(room_f,"%d %ld %d\n", world[nr].zone, world[nr].room_flags, world[nr].sector_type);
  for (idir = 0; idir < 6;idir++) {
     if (world[nr].dir_option[idir])
        {

        /* Addition of lock levels - Ranger July 96 */
        tmpInfo = 0;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_LOCK_30))   tmpInfo = 7;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_LOCK_25))   tmpInfo = 6;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_LOCK_20))   tmpInfo = 5;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_LOCK_15))   tmpInfo = 4;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_LOCK_10))   tmpInfo = 3;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_PICKPROOF)) tmpInfo = 2;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_MOVE))      tmpInfo = 8;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_CRAWL))     tmpInfo = 9;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_JUMP))      tmpInfo = 10;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_CLIMB))     tmpInfo = 11;
        if(IS_SET(world[nr].dir_option[idir]->exit_info, EX_ENTER))     tmpInfo = 12;
        if((tmpInfo==0) && world[nr].dir_option[idir]->exit_info)       tmpInfo = 1;

        fprintf(room_f,"D%d\n", idir);
        fwrite_string(room_f,world[nr].dir_option[idir]->general_description);
        fwrite_string(room_f,world[nr].dir_option[idir]->keyword);
        fprintf(room_f,"%u %d %d\n",tmpInfo,
                world[nr].dir_option[idir]->key, world[nr].dir_option[idir]->to_room_v);
         }
       }
       wext=world[nr].ex_description;
       while(wext!=NULL)
       {
         fprintf(room_f,"E\n");
         fwrite_string(room_f,wext->keyword);
         fwrite_string(room_f,wext->description);
         wext=wext->next;
       }
       fprintf(room_f,"S\n");
}

void write_mob(FILE *mob_f,int nr)
{
  int i;
  struct tagline_data *tag;

  fprintf(mob_f,"#%d\n",mob_proto_table[nr].virtual);
  fwrite_string(mob_f,mob_proto_table[nr].name);
  fwrite_string(mob_f,mob_proto_table[nr].short_descr);
  fwrite_string(mob_f,mob_proto_table[nr].long_descr);
  fwrite_string(mob_f,mob_proto_table[nr].description);

  fprintf(mob_f,"%lu %lu %d Y\n",mob_proto_table[nr].act,
                                 mob_proto_table[nr].affected_by,
                                 mob_proto_table[nr].alignment);
  fprintf(mob_f,"%d %d %d %dd%d+%d %dd%d+%d\n",mob_proto_table[nr].level,
                                               20-mob_proto_table[nr].hitroll,
                                               mob_proto_table[nr].armor/10,
                                               mob_proto_table[nr].hp_nodice,
                                               mob_proto_table[nr].hp_sizedice,
                                               mob_proto_table[nr].hp_add,
                                               mob_proto_table[nr].damnodice,
                                               mob_proto_table[nr].damsizedice,
                                               mob_proto_table[nr].damroll);

  fprintf(mob_f,"%u %d\n",mob_proto_table[nr].gold,
                          mob_proto_table[nr].exp);
  fprintf(mob_f,"%d %d %d\n",(int)mob_proto_table[nr].position,
                             (int)mob_proto_table[nr].default_pos,
                             (int)mob_proto_table[nr].sex);
  fprintf(mob_f,"%d %lu %dd%d+%d %d\n",(int)mob_proto_table[nr].class,
                                       mob_proto_table[nr].immune,
                                       mob_proto_table[nr].mana_nodice,
                                       mob_proto_table[nr].mana_sizedice,
                                       mob_proto_table[nr].mana_add,
                                       mob_proto_table[nr].no_att);
  if(mob_proto_table[nr].no_att>0) {
    for (i=0;i<mob_proto_table[nr].no_att;i++) {
      if(i==MAX_ATTS) break;
      if( (mob_proto_table[nr].att_type[i]==ATT_SPELL_CAST) ||
          (mob_proto_table[nr].att_type[i]==ATT_SPELL_SKILL) )
        fprintf(mob_f,"%d %d %d %d\n",(int)mob_proto_table[nr].att_type[i],
                                      mob_proto_table[nr].att_target[i],
                                      mob_proto_table[nr].att_percent[i],
                                      mob_proto_table[nr].att_spell[i]);
      else
        fprintf(mob_f,"%d %d %d 0\n",(int)mob_proto_table[nr].att_type[i],
                                     mob_proto_table[nr].att_target[i],
                                     mob_proto_table[nr].att_percent[i]);
    }
  }
  fprintf(mob_f,"%d %lu %lu %lu\n",mob_proto_table[nr].hit_type,
                                   mob_proto_table[nr].act2,
                                   mob_proto_table[nr].affected_by2,
                                   mob_proto_table[nr].immune2);

  if(mob_proto_table[nr].resist) {
    fprintf(mob_f,"R\n");
    fprintf(mob_f,"%lu\n",mob_proto_table[nr].resist);
  }

  tag=mob_proto_table[nr].tagline;
  while(tag!=NULL) {
    fprintf(mob_f,"T\n");
    fwrite_string(mob_f,tag->desc);
    tag=tag->next;
  }

  fprintf(mob_f,"K\n");
  fprintf(mob_f,"%d %d %d %d %d %d %d\n",mob_proto_table[nr].skin_value,
                        mob_proto_table[nr].skin_vnum[0], mob_proto_table[nr].skin_vnum[1],
                        mob_proto_table[nr].skin_vnum[2], mob_proto_table[nr].skin_vnum[3],
                        mob_proto_table[nr].skin_vnum[4], mob_proto_table[nr].skin_vnum[5]);

}

void write_shop(FILE *shop_f, int nr) {
  int i;

  fprintf(shop_f,"#%d~\n",shop_index[nr].keeper);
  for(i=0;i<MAX_PROD;i++) {
    fprintf(shop_f,"%d\n",shop_index[nr].producing[i]);
  }
  fprintf(shop_f,"%f\n",shop_index[nr].profit_buy);
  fprintf(shop_f,"%f\n",shop_index[nr].profit_sell);
  for(i=0;i<MAX_TRADE;i++) {
    fprintf(shop_f,"%d\n",shop_index[nr].type[i]);
  }
  fwrite_string(shop_f,shop_index[nr].no_such_item1);
  fwrite_string(shop_f,shop_index[nr].no_such_item2);
  fwrite_string(shop_f,shop_index[nr].do_not_buy);
  fwrite_string(shop_f,shop_index[nr].missing_cash1);
  fwrite_string(shop_f,shop_index[nr].missing_cash2);
  fwrite_string(shop_f,shop_index[nr].message_buy);
  fwrite_string(shop_f,shop_index[nr].message_sell);

  fprintf(shop_f,"%d\n",shop_index[nr].temper1);
  fprintf(shop_f,"%d\n",shop_index[nr].temper2);
  fprintf(shop_f,"%d\n",shop_index[nr].keeper);
  fprintf(shop_f,"%d\n",shop_index[nr].with_who);
  fprintf(shop_f,"%d\n",shop_index[nr].in_room);
  fprintf(shop_f,"%d\n",shop_index[nr].open1);
  fprintf(shop_f,"%d\n",shop_index[nr].close1);
  fprintf(shop_f,"%d\n",shop_index[nr].open2);
  fprintf(shop_f,"%d\n",shop_index[nr].close2);
}

void do_create_save(CHAR *ch, char* arg) {
  int nr=-1;
  FILE *fl;
  char args[MAX_STRING_LENGTH];
  char *tmp;
  sprintf(args, "%s", arg);
  tmp = strtok(args, " ");
  if(tmp) {
    if(isdigit(tmp[0])) {
      nr = atoi(tmp);
      nr = real_zone(nr);
    } else {
      if(!strcmp(tmp, "all")) {
	if(GET_LEVEL(ch) > LEVEL_SUP) {
	  fl = fopen(WORLD_LIST_FILE, "w");
	  if(fl) {
             for(nr=0;nr<=top_of_zone_table;nr++) {
               fprintf(fl, "%s\n", zone_table[nr].name);
               save_zone(nr);
	     }
             fclose(fl);
             nr = -1;
	  }
	}
      } else {
	send_to_char("Save <# of zone>, not 'save zone <#>', please.\r\n",ch);
	nr=-1;
      }
    }
  }

  if(nr!=-1) {
    if(GET_LEVEL(ch)> LEVEL_WIZ || isname(GET_NAME(ch), zone_table[nr].name) || strstr(zone_table[nr].creators,GET_NAME(ch))) {
      save_zone(nr);
      sprintf(args, "Zone '%s' (#%d) saved, barring disk troubles\r\n",
	      zone_table[nr].name, zone_table[nr].virtual);

      send_to_char(args, ch);
    } else {
      send_to_char("You are not allowed to save that zone.\r\n", ch);
    }
  }
}



void save_zone(int nr) {
  FILE *fp;
  char filename[35]="", *nameptr, tmpname[100];
  int i;
  bool full=FALSE,shop=FALSE;

  if(zone_table[nr].virtual==299 ||
     zone_table[nr].virtual==298 ||
     zone_table[nr].virtual==295 ||
     zone_table[nr].virtual==294 || /* Vanity */
     zone_table[nr].virtual==10) full = TRUE; 
  nameptr = filename;
  for(i=0;i<MIN(32, strlen(zone_table[nr].name));i++) {
    if(isalnum(zone_table[nr].name[i]))
      *nameptr++ = zone_table[nr].name[i];
  }
  *nameptr = 0;

 /* Backup vanity zone */
  if(full) {
    sprintf(tmpname,"cp -p world/%s.zon world/%s.zon.bak",filename,filename);
    system(tmpname);
    sprintf(tmpname,"cp -p world/%s.wld world/%s.wld.bak",filename,filename);
    system(tmpname);
    sprintf(tmpname,"cp -p world/%s.obj world/%s.obj.bak",filename,filename);
    system(tmpname);
    sprintf(tmpname,"cp -p world/%s.mob world/%s.mob.bak",filename,filename);
    system(tmpname);
  }

  sprintf(tmpname, "world/%s.zon", filename);

  if((fp=fopen(tmpname,"w"))) {
    write_zone(fp,nr);
    fclose(fp);
  }

  sprintf(tmpname, "world/%s.wld", filename);

  if((fp = fopen(tmpname, "w"))) {
    for(i=0;i<top_of_world+1;i++) {
      if((world[i].number >= zone_table[nr].bottom)&&
         (world[i].number <= zone_table[nr].top   )  )
        write_room(fp, i);
    }
    fprintf(fp, "#0\n$~\n");
    fclose(fp);
  }

  sprintf(tmpname, "world/%s.mob", filename);

  if((fp=fopen(tmpname, "w"))) {
    for(i=0;i<top_of_mobt+1;i++) {
      if((mob_proto_table[i].virtual >= zone_table[nr].bottom)&&
         (mob_proto_table[i].virtual <= zone_table[nr].top   )  )
        write_mob(fp, i);
      }
    fprintf(fp, "#0\n$~\n");
    fclose(fp);
  }

  sprintf(tmpname, "world/%s.obj", filename);

  if((fp=fopen(tmpname, "w"))) {
    for(i=0;i<top_of_objt+1;i++) {
      if((obj_proto_table[i].virtual >= zone_table[nr].bottom)&&
         (obj_proto_table[i].virtual <= zone_table[nr].top   )  )
        write_obj(fp,i);
    }
    fprintf(fp, "#0\n$~\n");
    fclose(fp);
  }

  sprintf(tmpname, "world/%s.shp", filename);

  if((fp = fopen(tmpname, "w"))) {
    for( i = 0; i <number_of_shops; i++) {
      if((shop_index[i].keeper >= zone_table[nr].bottom) &&
         (shop_index[i].keeper <= zone_table[nr].top) ) {
        shop=TRUE;
        write_shop(fp, i);
      }
    }
    if(shop)
      fprintf(fp, "$~\n");
    fclose(fp);
    if(!shop)
      remove(tmpname);
  }
}

void write_zone(FILE *zone_f,int nr)
{
  int year;
  char buf[MIL];
  long ct;
  struct tm *timeStruct;
  int tmp, tmp2;

  fprintf(zone_f,"#%d\n",zone_table[nr].virtual);
  fwrite_string(zone_f,zone_table[nr].name);

  ct=time(0);
  timeStruct = localtime(&ct);
  year=1900+timeStruct->tm_year;
  sprintf(buf,"%2d%s%4d",timeStruct->tm_mday,Month[timeStruct->tm_mon],year);
  zone_table[nr].mod_date = str_dup(buf);
  fprintf(zone_f,"%d %d %d\n",zone_table[nr].top,zone_table[nr].lifespan,zone_table[nr].reset_mode);
  fprintf(zone_f,"X %d %s %s %s\n",zone_table[nr].climate,
          zone_table[nr].create_date,zone_table[nr].mod_date,
          zone_table[nr].creators);
  fprintf(zone_f,"Y %d %d %d %d %d %d %d %d\n",zone_table[nr].mult_hp,zone_table[nr].mult_mana,
          zone_table[nr].mult_hitroll,zone_table[nr].mult_damage,zone_table[nr].mult_armor,
          zone_table[nr].mult_xp,zone_table[nr].mult_gold,zone_table[nr].mult_level);

      if(zone_table[nr].cmd!=NULL)
        {
      /* Do mobs */
        for(tmp=0;(zone_table[nr].cmd[tmp].command!= 'S');tmp++)
          {
          if(zone_table[nr].cmd[tmp].command == 'M')
            {
            fprintf(zone_f,"M 0 %d %d %d\n",zone_table[nr].cmd[tmp].arg1,zone_table[nr].cmd[tmp].arg2,zone_table[nr].cmd[tmp].arg3);
            for(tmp2=tmp+1;((zone_table[nr].cmd[tmp2].command=='E')||(zone_table[nr].cmd[tmp2].command=='G'));tmp2++)
              {
              zone_table[nr].cmd[tmp2].arg2=0; /* Not used for E or G - Ranger */
              if(zone_table[nr].cmd[tmp2].command=='G') zone_table[nr].cmd[tmp2].arg3=0;/* Not used for G - Ranger */
              fprintf(zone_f,"%c 1 %d %d %d\n",zone_table[nr].cmd[tmp2].command,
                                               zone_table[nr].cmd[tmp2].arg1,
                                               zone_table[nr].cmd[tmp2].arg2,
                                               zone_table[nr].cmd[tmp2].arg3);
              }
            }
/* Added saving of F and R reset commands - Ranger June 96 */
          if(zone_table[nr].cmd[tmp].command == 'F')
            {
            fprintf(zone_f,"F 1 %d %d %d\n",zone_table[nr].cmd[tmp].arg1,zone_table[nr].cmd[tmp].arg2,zone_table[nr].cmd[tmp].arg3);
            for(tmp2=tmp+1;((zone_table[nr].cmd[tmp2].command=='E')||(zone_table[nr].cmd[tmp2].command=='G'));tmp2++)
              {
              zone_table[nr].cmd[tmp2].arg2=0; /* Not used for E or G - Ranger */
              if(zone_table[nr].cmd[tmp2].command=='G') zone_table[nr].cmd[tmp2].arg3=0;/* Not used for G - Ranger */
              fprintf(zone_f,"%c 1 %d %d %d\n",zone_table[nr].cmd[tmp2].command,
                                               zone_table[nr].cmd[tmp2].arg1,
                                               zone_table[nr].cmd[tmp2].arg2,
                                               zone_table[nr].cmd[tmp2].arg3);
              }
            }
          if(zone_table[nr].cmd[tmp].command == 'R')
            {
            fprintf(zone_f,"R 1 %d %d %d\n",zone_table[nr].cmd[tmp].arg1,zone_table[nr].cmd[tmp].arg2,zone_table[nr].cmd[tmp].arg3);
            for(tmp2=tmp+1;((zone_table[nr].cmd[tmp2].command=='E')||(zone_table[nr].cmd[tmp2].command=='G'));tmp2++)
              {
              zone_table[nr].cmd[tmp2].arg2=0; /* Not used for E or G - Ranger */
              if(zone_table[nr].cmd[tmp2].command=='G') zone_table[nr].cmd[tmp2].arg3=0;/* Not used for G - Ranger */
              fprintf(zone_f,"%c 1 %d %d %d\n",zone_table[nr].cmd[tmp2].command,
                                               zone_table[nr].cmd[tmp2].arg1,
                                               zone_table[nr].cmd[tmp2].arg2,
                                               zone_table[nr].cmd[tmp2].arg3);
              }
            }
          } /* main for */

        /* objects */
        for(tmp=0;(zone_table[nr].cmd[tmp].command!= 'S');tmp++)
          {
          if(zone_table[nr].cmd[tmp].command == 'O')
            {
            zone_table[nr].cmd[tmp].arg2=0; /* Not used for O - Ranger */
            fprintf(zone_f,"O 0 %d %d %d\n",zone_table[nr].cmd[tmp].arg1,zone_table[nr].cmd[tmp].arg2,zone_table[nr].cmd[tmp].arg3);
            for(tmp2=tmp+1;(zone_table[nr].cmd[tmp2].command=='P');tmp2++)
              {
              zone_table[nr].cmd[tmp2].arg2=0; /* Not used for P - Ranger */
              fprintf(zone_f,"%c 1 %d %d %d\n",zone_table[nr].cmd[tmp2].command,
                                              zone_table[nr].cmd[tmp2].arg1,
                                              zone_table[nr].cmd[tmp2].arg2,
                                              zone_table[nr].cmd[tmp2].arg3);
              }
            }
          }

        /* object take */
        for(tmp=0;(zone_table[nr].cmd[tmp].command!= 'S');tmp++)
          {
          if(zone_table[nr].cmd[tmp].command == 'T')
            {
            fprintf(zone_f,"T 0 %d %d %d\n",zone_table[nr].cmd[tmp].arg1,zone_table[nr].cmd[tmp].arg2,zone_table[nr].cmd[tmp].arg3);
            }
          }

        for(tmp=0;(zone_table[nr].cmd[tmp].command!= 'S');tmp++)
          {
          if(zone_table[nr].cmd[tmp].command == 'D')
            {
            fprintf(zone_f,"D 0 %d %d %d\n",zone_table[nr].cmd[tmp].arg1,zone_table[nr].cmd[tmp].arg2,zone_table[nr].cmd[tmp].arg3);
            }
          }
        }
      fprintf(zone_f,"S\n$~\n");
}
#define EDIT_ROOM_NAME 1 /*rname*/
#define EDIT_ROOM_SECTOR 2 /*rsect*/
#define EDIT_ROOM_FLAGS 3 /*rflag*/
#define EDIT_ROOM_DIRECTION 4 /*rlink, runlink, dlink, dunlink, exitname, exitdesc, exittype, exitkey */
#define EDIT_ROOM_EXTRA 5 /*rextra*/
#define EDIT_ROOM_DESCRIPTION 6 /*rdesc*/
#define EDIT_ROOM_RESET 7 /* Ranger May 96 */ /*rreset*/

void edit_room(CHAR *ch, char *fargs)
{
  char args[MAX_STRING_LENGTH];
  int edit,tmp,tmp1, tmp2;
  char *arg2;
  int v_room=-1, room, dir, option,next_room,found;
  int zone;
  struct extra_descr_data *tmp_descr, **prev_descr_ptr, *tmp_descr1;
  sprintf(args, "%s", fargs);
  if(args[0] != '\0')
    {
    arg2 = strtok(args, " ");
    if(arg2)
       v_room = atoi(arg2);
    }
  zone = inzone(v_room);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  room = real_room(v_room);
  if(room >= 0 && room <= top_of_world)
    {
    arg2 = strtok(NULL, " ");
    edit = -1;
    if(arg2)
    {
    if(is_abbrev(arg2, "name"))
      edit = EDIT_ROOM_NAME;
    if(is_abbrev(arg2, "sector"))
      edit = EDIT_ROOM_SECTOR;
    if(is_abbrev(arg2, "flags"))
      edit = EDIT_ROOM_FLAGS;
    if(is_abbrev(arg2, "direction"))
      edit = EDIT_ROOM_DIRECTION;
    if(is_abbrev(arg2, "extradescription"))
      edit = EDIT_ROOM_EXTRA;
    if(is_abbrev(arg2, "description"))
      edit = EDIT_ROOM_DESCRIPTION;
    if(is_abbrev(arg2, "reset"))
      edit = EDIT_ROOM_RESET;
    }
   switch(edit)
      {
      case EDIT_ROOM_EXTRA:
        arg2 = strtok(NULL, " ");
        tmp = -1;
        if(arg2)
        {
        if(is_abbrev(arg2, "add"))
          tmp = 1;
        if(is_abbrev(arg2, "remove"))
          tmp = 2;
        }
        if(tmp!=-1)
          {
          switch(tmp)
            {
            case 1:
             arg2 = strtok(NULL, "\0");
             if(arg2)
             {
             send_to_char("Enter the extra description, terminate with @ on its own line.\n\r",ch);
             CREATE(tmp_descr, struct extra_descr_data, 1);
             tmp_descr->keyword = str_dup(arg2);
             tmp_descr->next =world[room].ex_description;
             ch->desc->str = &tmp_descr->description;
             ch->desc->max_str = LEN_EXTRADESC_MAX;
             world[room].ex_description = tmp_descr;
             tmp_descr = NULL;
             }
             break;
           case 2:
             arg2 = strtok(NULL, "\0");
             if(arg2)
             {
             tmp_descr= world[room].ex_description;
             prev_descr_ptr = &world[room].ex_description;
             found=0;
             while(tmp_descr)
               {
               if(!strcmp(tmp_descr->keyword, arg2))
                 {/*delete that one*/
                 *prev_descr_ptr = tmp_descr->next;
                 if(tmp_descr->description)
                   free(tmp_descr->description);
                 if(tmp_descr->keyword)
                   free(tmp_descr->keyword);
                 tmp_descr1 = tmp_descr->next;
                 free(tmp_descr);
                 tmp_descr = tmp_descr1;
                 send_to_char("Extra description deleted.\n\r",ch);
                 found=1;
                 }
               else
                 {
                 prev_descr_ptr = &tmp_descr->next;
                 tmp_descr = tmp_descr->next;
                 }
               }
               if(!found) send_to_char("Extra description not found.\n\r",ch);
             }
             break;
             }
          }
        else
          send_to_char("edit room <#> extra <add|remove> <keywords>\n\rWhen removing, all keywords must be used.\n\r",ch);
        break;
      case EDIT_ROOM_SECTOR:
         arg2 = strtok(NULL, " ");
         tmp2 = -1;
         if(arg2)
           {
           tmp2 = old_search_block(arg2, 0, strlen(arg2), sector_types_i, FALSE);
           if(tmp2!=-1)
             {
             world[room].sector_type= tmp2-1;
             }
           }
         if(tmp2 == -1)
           send_to_char("edit room <#> sector <type>, where <type> is on of the following:\n\r\
INSIDE, CITY, FOREST, FIELD, HILLS, MOUNTAIN, SWIMMABLE WATER, NOSWIM WATER,\n\rARCTIC, DESERT.\n\r", ch);
         break;
     case EDIT_ROOM_FLAGS:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
         {
         if(is_abbrev(arg2, "remove"))
            tmp = 1;
         if(is_abbrev(arg2, "add"))
            tmp = 2;
         }
         if(tmp==-1)
           {
           send_to_char("edit room <#> flags <add|remove> all, or a list of the following:\r\n\
DARK, DEATH, NO_MOB, INDOORS, LAWFUL, CHAOTIC, SAFE, NO_MAGIC, TUNNEL, \n\r\
PRIVATE, LOCK, TRAP, ARENA, CLUB, QUIET, NO_BEAM, HAZARD, MOVETRAP, \n\r\
FLYING, NO_PEEK, NO_SONG, NO_REGEN, NO_QUAFF, REV_REGEN, DOUBLE_MANA,\n\r\
HALF_CONC, LIT, NO_ORB, QRTR_CONC, MANADRAIN, NO_SUM.\n\r", ch);
           }
         else
           {
           arg2 = strtok(NULL, " ");
           if(arg2)
             {
             if(!strcmp(arg2, "all"))
               {
               if(tmp ==1)
                 world[room].room_flags = 0;
               else
                 world[room].room_flags = ROOM_FINAL -1;
               }
             else
               {
               while (arg2)
                 {
                 tmp1 = old_search_block(arg2, 0, strlen(arg2), room_bits, FALSE);
                 if(tmp1 != -1)
                  {
                   if(tmp==1)
                     world[room].room_flags &= ~(1<<(tmp1-1));
                   else
                     world[room].room_flags |=  1<<(tmp1-1);
                   if(!strcmp(arg2,"FLYING")) {
                      if(tmp==1) remove_flying_room(room);
                      else add_flying_room(room);
                   }
                  }
                 arg2 = strtok(NULL, " ");
                 }
               }
             send_to_char("Done.\n\r", ch);
             }
           }

         break;
      case EDIT_ROOM_NAME:
         if(world[room].name)
           world[room].name[0] = 0;
         ch->desc->str = &world[room].name;
         ch->desc->max_str = 80;
         break;
      case EDIT_ROOM_DESCRIPTION:
         if(world[room].description)
           world[room].description[0] = 0;
         ch->desc->str = &world[room].description;
         ch->desc->max_str = 4096;
         break;
      case EDIT_ROOM_DIRECTION:
        arg2 = strtok(NULL, " ");
        dir = -1;
        if(arg2)
        {
        if(is_abbrev(arg2, "north"))
           dir = 0;
        if(is_abbrev(arg2, "east"))
           dir = 1;
        if(is_abbrev(arg2, "south"))
           dir = 2;
        if(is_abbrev(arg2, "west"))
           dir = 3;
        if(is_abbrev(arg2, "up"))
           dir = 4;
        if(is_abbrev(arg2, "down"))
           dir = 5;
        }
        if(dir != -1)
          {
          arg2 = strtok(NULL, " ");
          option = -1;
          if(arg2)
          {
          if(is_abbrev(arg2, "add"))
             option = 1;
          if(is_abbrev(arg2, "remove"))
             option = 2;
          if(is_abbrev(arg2, "change"))
             option = 3;
          }
          switch(option)
            {
            case 1:
              if(!world[room].dir_option[dir]) /*add*/
                {
                CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
                world[room].dir_option[dir]->general_description = NULL;
                world[room].dir_option[dir]->keyword = NULL;
                world[room].dir_option[dir]->exit_info = 0;
                arg2 = strtok(NULL, " ");
                if(arg2) {
                  next_room=real_room(atoi(arg2));
                  if(next_room < 0 || next_room > top_of_world) {
                    send_to_char("The exit room doesn't exist.\n\r",ch);
                    break;
                  }
                  world[room].dir_option[dir]->to_room_v = atoi(arg2);
                } else world[room].dir_option[dir]->to_room_v = 0;

                world[room].dir_option[dir]->key = -1;
                world[room].dir_option[dir]->to_room_r = real_room(world[room].dir_option[dir]->to_room_v);
                send_to_char("Done.\n\r",ch);
                }
              else
                send_to_char("A exit in that direction already exists.\n\r",ch);
              break;
            case 2: /*remove*/
              if(world[room].dir_option[dir])
                {
                if(world[room].dir_option[dir]->general_description)
                  free(world[room].dir_option[dir]->general_description);
                if(world[room].dir_option[dir]->keyword)
                  free(world[room].dir_option[dir]->keyword);
                free(world[room].dir_option[dir]);
                world[room].dir_option[dir]= NULL;
                send_to_char("Done.\n\r",ch);
                }
              else
                send_to_char("A exit in that direction does not exist.\n\r",ch);
              break;
            case 3:
              if(world[room].dir_option[dir])
              {
              arg2 = strtok(NULL, " ");
              tmp2 = -1;
              if(arg2)
              {
              if(is_abbrev(arg2, "room"))
                tmp2 = 1;
              if(is_abbrev(arg2, "keyword"))
                tmp2 = 2;
              if(is_abbrev(arg2, "description"))
                tmp2 = 3;
              if(is_abbrev(arg2, "key"))
                tmp2 = 4;
              if(is_abbrev(arg2, "type"))
                tmp2 = 5;
              }
              switch(tmp2)
                {
                case 1:  /*to room*/
                  arg2 = strtok(NULL, " ");
                  if(arg2)
                    world[room].dir_option[dir]->to_room_v = atoi(arg2);
                  else
                    world[room].dir_option[dir]->to_room_v = -1;
                  world[room].dir_option[dir]->to_room_r = real_room(world[room].dir_option[dir]->to_room_v);
                  send_to_char("Done.\n\r",ch);
                  break;
                case 2:  /*keyword*/
                  arg2 = strtok(NULL, "\0");
                  if(arg2)
                    {
                    if(world[room].dir_option[dir]->keyword)
                      free(world[room].dir_option[dir]->keyword);
                    world[room].dir_option[dir]->keyword = str_dup(arg2);
                    send_to_char("Done.\n\r",ch);
                    }
                  else
                    send_to_char("You must input a keyword!\r\n",ch);
                  break;
                case 3: /*description*/
                  if(world[room].dir_option[dir]->general_description)
                     world[room].dir_option[dir]->general_description[0] = 0;
                  ch->desc->str = &world[room].dir_option[dir]->general_description;
                  ch->desc->max_str = 4096;
                  break;
                case 4: /*key */
                  arg2 = strtok(NULL, " ");
                  if(arg2)
                  {
                  if(isdigit(arg2[0]))
                    {
                    world[room].dir_option[dir]->key = atoi(arg2);
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "none"))
                    {
                    world[room].dir_option[dir]->key = -1;
                    send_to_char("Done.\n\r",ch);
                    }
                  }
                  else
                    send_to_char("edit room <#> direction change key <virtual key# | none>\n\r",ch);
                  break;
                case 5: /*type of door*/
                  arg2 = strtok(NULL, " ");
                  if(arg2)
                  {
                  if(is_abbrev(arg2, "normal"))
                    {
                    if(world[room].dir_option[dir]->keyword)
                      {
                      free(world[room].dir_option[dir]->keyword);
                      world[room].dir_option[dir]->keyword=NULL;
                      }
                    world[room].dir_option[dir]->exit_info = 0;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "door"))
                    {
                    if(!world[room].dir_option[dir]->keyword)
                      world[room].dir_option[dir]->keyword=str_dup("door");
                    world[room].dir_option[dir]->exit_info = EX_ISDOOR;
                    send_to_char("Done.\n\r",ch);
                    }
                  /* Addition of move/jump/crawl/climb - Ranger Oct 96 */
                  else if(is_abbrev(arg2, "move"))
                    {
                    if(!world[room].dir_option[dir]->keyword)
                      world[room].dir_option[dir]->keyword=str_dup("nothing");
                    world[room].dir_option[dir]->exit_info = EX_MOVE;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "jump"))
                    {
                    if(!world[room].dir_option[dir]->keyword)
                      world[room].dir_option[dir]->keyword=str_dup("nothing");
                    world[room].dir_option[dir]->exit_info = EX_JUMP;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "crawl"))
                    {
                    if(!world[room].dir_option[dir]->keyword)
                      world[room].dir_option[dir]->keyword=str_dup("nothing");
                    world[room].dir_option[dir]->exit_info = EX_CRAWL;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "climb"))
                    {
                    if(!world[room].dir_option[dir]->keyword)
                      world[room].dir_option[dir]->keyword=str_dup("nothing");
                    world[room].dir_option[dir]->exit_info = EX_CLIMB;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "enter"))
                    {
                    if(!world[room].dir_option[dir]->keyword)
                      world[room].dir_option[dir]->keyword=str_dup("nothing");
                    world[room].dir_option[dir]->exit_info = EX_ENTER;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "nopick"))
                    {
                    world[room].dir_option[dir]->exit_info = EX_PICKPROOF | EX_ISDOOR;
                    send_to_char("Done.\n\r",ch);
                    }
                 /* Addition of lock levels - Ranger July 96 */
                  else if(is_abbrev(arg2, "lock_10"))
                    {
                    world[room].dir_option[dir]->exit_info = EX_LOCK_10 | EX_ISDOOR;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "lock_15"))
                    {
                    world[room].dir_option[dir]->exit_info = EX_LOCK_15 | EX_ISDOOR;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "lock_20"))
                    {
                    world[room].dir_option[dir]->exit_info = EX_LOCK_20 | EX_ISDOOR;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "lock_25"))
                    {
                    world[room].dir_option[dir]->exit_info = EX_LOCK_25 | EX_ISDOOR;
                    send_to_char("Done.\n\r",ch);
                    }
                  else if(is_abbrev(arg2, "lock_30"))
                    {
                    world[room].dir_option[dir]->exit_info = EX_LOCK_30 | EX_ISDOOR;
                    send_to_char("Done.\n\r",ch);
                    }
                  else
                    send_to_char("edit room # direction <n|e|w|s|u|d> change type <normal|climb|crawl|enter|jump|move|door|nopick|lock_(10/15/20/25/30)>\n\r",ch);
                  }
                  else
                    send_to_char("edit room # direction <n|e|w|s|u|d> change type <normal|climb|crawl|enter|jump|move|door|nopick|lock_(10/15/20/25/30)>\n\r",ch);
                  break;
                default:
                  send_to_char("edit room # direction <n|e|w|s|u|d> change <type|key|description|room|keyword> [options]\n\r",ch);
                  break;
                }
              }
              else
                send_to_char("An exit in that direction does not exist.\n\r",ch);
              break;
            }
          }
        else
          send_to_char("edit room # direction <n|e|w|s|u|d> <add|change|remove> [options]\n\r",ch);
        break;
        case EDIT_ROOM_RESET:
          world[room].name        = str_dup("A temporary name");
          world[room].description = str_dup("A temporary description\n");
          world[room].room_flags  = 0;
          world[room].sector_type = 0;
          world[room].funct       = NULL;
          world[room].contents    = NULL;
          world[room].light       = 0;
          world[room].zone        = zone;
          for(dir=0;dir<6;dir++) world[room].dir_option[dir] = NULL;
          world[room].ex_description = NULL;
          break;
        default:
send_to_char("Room editables are:\n\r\
flags, extradescription, sector, name, description, direction, reset\
\r\n", ch);
        break;
      }
    }
  else
    send_to_char("Room does not exist.\n\r",ch);
}

#define EDIT_MOB_NAME           1  /*mname*/
#define EDIT_MOB_SHORT          2  /*mshort*/
#define EDIT_MOB_LONG           3  /*mlong*/
#define EDIT_MOB_FULL           4  /*mfull*/
#define EDIT_MOB_LEVEL          5  /*mlevel*/
#define EDIT_MOB_SEX            6  /*msex*/
#define EDIT_MOB_HITPOINT       7  /*mhps*/
#define EDIT_MOB_DAMAGE         8  /*mdamage*/
#define EDIT_MOB_GOLD           9  /*mgold*/
#define EDIT_MOB_EXP            10 /*mexp*/
#define EDIT_MOB_ACT            11 /*mact*/
#define EDIT_MOB_AFFECTED       12 /*maff*/
#define EDIT_MOB_ALIGNMENT      13 /*malign*/
#define EDIT_MOB_POSITION       14 /*mpos*/
#define EDIT_MOB_DEFPOS         15 /*mdefpos*/
#define EDIT_MOB_ARMOR          16 /*marmor*/
#define EDIT_MOB_HITROLL        17 /*mhitroll*/
#define EDIT_MOB_RESET          18 /* Ranger May 96 */ /*mreset*/
#define EDIT_MOB_CLASS          19 /*mclass*/
#define EDIT_MOB_ATTACK         20 /*mattack, attdel*/
#define EDIT_MOB_MANA           21 /*mmana*/
#define EDIT_MOB_IMMUNE         22 /*mimmune*/
#define EDIT_MOB_TAGLINE        23 /*mtag*/ /* Ranger May 2000 */
#define EDIT_MOB_RESIST         24 /*mresist*/ /* Ranger Aug 2000 */
#define EDIT_MOB_SKIN           25 /*mskin */ /* Ranger Feb 2001 */

void edit_mob(CHAR *ch, char *fargs)
{
  char args[MAX_INPUT_LENGTH];
  int v_mob=-1, mob, edit=-1, tmp, tmp1, tmp2, tmp3, tmp4, zone,i=0,num,act,aff,immune;
  char *arg2;
  struct tagline_data *tmp_tag, **prev_tag_ptr, *tmp_tag1;

  sprintf(args, "%s", fargs);
  arg2 = strtok(args, " ");
  if(arg2)
    v_mob = atoi(arg2);
  zone = inzone(v_mob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  mob = real_mobile(v_mob);
  if(mob != -1)
    {
/*    if(mob_proto_table[mob].number == 0)*/
    if(1)
    {
    arg2 = strtok(NULL, " ");
    edit = -1;
    if(arg2)
    {
    if(is_abbrev(arg2, "name"))
      edit = EDIT_MOB_NAME;
    if(is_abbrev(arg2, "short"))
      edit = EDIT_MOB_SHORT;
    if(is_abbrev(arg2, "long"))
      edit = EDIT_MOB_LONG;
    if(is_abbrev(arg2, "full"))
      edit = EDIT_MOB_FULL;
    if(is_abbrev(arg2, "level"))
      edit = EDIT_MOB_LEVEL;
    if(is_abbrev(arg2, "sex"))
      edit = EDIT_MOB_SEX;
    if(is_abbrev(arg2, "hp"))
      edit = EDIT_MOB_HITPOINT;
    if(is_abbrev(arg2, "damage"))
      edit = EDIT_MOB_DAMAGE;
    if(is_abbrev(arg2, "gold"))
      edit = EDIT_MOB_GOLD;
    if(is_abbrev(arg2, "exp"))
      edit = EDIT_MOB_EXP;
    if(is_abbrev(arg2, "act"))
      edit = EDIT_MOB_ACT;
    if(is_abbrev(arg2, "affects"))
      edit = EDIT_MOB_AFFECTED;
    if(is_abbrev(arg2, "align"))
      edit = EDIT_MOB_ALIGNMENT;
    if(is_abbrev(arg2, "position"))
      edit = EDIT_MOB_POSITION;
    if(is_abbrev(arg2, "defaultpos"))
      edit = EDIT_MOB_DEFPOS;
    if(is_abbrev(arg2, "armor"))
      edit = EDIT_MOB_ARMOR;
    if(is_abbrev(arg2, "hitroll"))
      edit = EDIT_MOB_HITROLL;
    if(is_abbrev(arg2, "reset"))
      edit = EDIT_MOB_RESET;
    if(is_abbrev(arg2, "mana"))
      edit = EDIT_MOB_MANA;
    if(is_abbrev(arg2, "attack"))
      edit = EDIT_MOB_ATTACK;
    if(is_abbrev(arg2, "class"))
      edit = EDIT_MOB_CLASS;
    if(is_abbrev(arg2, "immune"))
      edit = EDIT_MOB_IMMUNE;
    if(is_abbrev(arg2, "tagline"))
      edit = EDIT_MOB_TAGLINE;
    if(is_abbrev(arg2, "resist"))
      edit = EDIT_MOB_RESIST;
    if(is_abbrev(arg2, "skin"))
      edit = EDIT_MOB_SKIN;
    }
    switch(edit)
      {
      case EDIT_MOB_SEX:
         arg2 = strtok(NULL, " ");
         edit = -1;
         if(arg2)
         {
         if(is_abbrev(arg2, "male"))
            edit = 1;
         else if(is_abbrev(arg2, "female"))
            edit = 2;
         else if(is_abbrev(arg2, "neutral"))
            edit = 0;
         }
         if(edit != -1)
            {
            mob_proto_table[mob].sex = edit;
            send_to_char("Done.\n\r", ch);
            }
         else
            send_to_char("Invalid Sex, \r\nedit mob <#> sex <female|male|neutral>.\n\r", ch);
         break;
      case EDIT_MOB_AFFECTED:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
         {
         if(is_abbrev(arg2, "remove"))
            tmp = 1;
         if(is_abbrev(arg2, "add"))
            tmp = 2;
         }
         if(tmp==-1)
           {
           send_to_char("edit mob <#> affect <add|remove> all, or a list of the following:\r\n\
BLIND, INVISIBLE, DETECT-ALIGNMENT, DETECT-INVISIBLE, DETECT-MAGIC,\r\n\
SENSE-LIFE, HOLD, SANCTUARY, CURSE, SPHERE, POISON, PROTECT-EVIL,\r\n\
PARALYSIS, INFRAVISION, STATUE, SLEEP, DODGE, SNEAK, HIDE, FLY, IMINV,\r\n\
INVUL, DUAL, FURY, PROTECT-GOOD, TRIPLE, QUAD.\r\n", ch);
           }
         else
           {
           arg2= strtok(NULL, " ");
           if(arg2)
             {
             if(!strcmp(arg2, "all"))
               {
               if(tmp ==1) {
                 mob_proto_table[mob].affected_by = 0;
                 mob_proto_table[mob].affected_by2 = 0;
               }
               else {
                 mob_proto_table[mob].affected_by = AFF_FINAL-1;
                 mob_proto_table[mob].affected_by2 = AFF2_FINAL-1;
                 mob_proto_table[mob].affected_by2 -= AFF2_SEVERED;
                 mob_proto_table[mob].affected_by2 -= AFF2_IMMINENT_DEATH;
               }
               }
             else
               {
               while (arg2)
                 {
                 aff=1;
                 tmp1 = old_search_block(arg2, 0, strlen(arg2), affected_bits, FALSE);
                 if(tmp1==-1) {
                   aff=2;
                   tmp1 = old_search_block(arg2, 0, strlen(arg2), affected_bits2, FALSE);
                 }
                 if(tmp1 != -1)
                  {
                   if(tmp==1) {
                     if(aff==1)
                       mob_proto_table[mob].affected_by &= ~(1<<(tmp1-1));
                     else
                       mob_proto_table[mob].affected_by2 &= ~(1<<(tmp1-1));
                   }
                   else {
                     if(aff==1)
                       mob_proto_table[mob].affected_by |=  1<<(tmp1-1);
                     else
                       mob_proto_table[mob].affected_by2 |=  1<<(tmp1-1);
                   }
                  }
                 arg2 = strtok(NULL, " ");
                 }
               }
               send_to_char("Done.\n\r", ch);
            }
           else
             send_to_char("edit mob <#> affect <add|remove> all, or a list of the following:\r\n\
BLIND, INVISIBLE, DETECT-ALIGNMENT, DETECT-INVISIBLE, DETECT-MAGIC,\r\n\
SENSE-LIFE, HOLD, SANCTUARY, CURSE, SPHERE, POISON, PROTECT-EVIL,\r\n\
PARALYSIS, INFRAVISION, STATUE, SLEEP, DODGE, SNEAK, HIDE, FLY, IMINV,\r\n\
INVUL, DUAL, FURY, PROTECT-GOOD, TRIPLE, QUAD.\r\n", ch);
          }
          break;
      case EDIT_MOB_TAGLINE:
        arg2 = strtok(NULL, " ");
        tmp = -1;
        if(arg2)
        {
        if(is_abbrev(arg2, "add"))
          tmp = 1;
        if(is_abbrev(arg2, "remove"))
          tmp = 2;
        }
        if(tmp!=-1)
          {
          switch(tmp)
            {
            case 1:
             arg2 = strtok(NULL, "\0");
             if(arg2) {
               CREATE(tmp_tag, struct tagline_data, 1);
               tmp_tag->desc = str_dup(arg2);
               tmp_tag->next = mob_proto_table[mob].tagline;
               mob_proto_table[mob].tagline = tmp_tag;
               tmp_tag = NULL;
               send_to_char("Done.\n\r",ch);
             }
             break;
           case 2:
             arg2 = strtok(NULL, "\0");
             if(arg2) {
             tmp_tag= mob_proto_table[mob].tagline;
             prev_tag_ptr = &mob_proto_table[mob].tagline;
             while(tmp_tag) {
               if(!strcmp(tmp_tag->desc, arg2))
                 {/*delete that one*/
                 *prev_tag_ptr = tmp_tag->next;
                 if(tmp_tag->desc)
                   free(tmp_tag->desc);
                 tmp_tag1 = tmp_tag->next;
                 free(tmp_tag);
                 tmp_tag = tmp_tag1;
                 send_to_char("Done.\n\r",ch);
                 }
               else
                 {
                 prev_tag_ptr = &tmp_tag->next;
                 tmp_tag = tmp_tag->next;
                 }
               }
             }
             break;
             }
          }
        else
          send_to_char("edit mob <#> tagline <add|remove> <tagline>\n\r",ch);
        break;
        case EDIT_MOB_ACT:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           if(is_abbrev(arg2, "remove"))
             tmp = 1;
           if(is_abbrev(arg2, "add"))
             tmp = 2;
           }
         if(tmp==-1)
           {
           send_to_char("edit mob <#> action <add|remove> all, or a list of the following:\r\n\
SENTINEL, SCAVENGER, ISNPC, NICE-THIEF, AGGRESSIVE, STAY-ZONE, WIMPY,\n\r\
SUBDUE, RIDE, MOUNT, FLY, AGGWA, AGGTH, AGGCL, AGGMU, MEMORY, AGGNI,\n\r\
AGGNO, AGGPA, AGGAP, AGGBA, AGGCO, AGGEVIL, AGGGOOD, AGGNEUT,\n\r\
AGGLEADER, AGGRANDOM, ARM, SHIELD, OPEN-DOOR, NO_TOKEN, IGNORE-SPHERE.\r\n", ch);
 /* MOUNT & FLY added for Stables Ranger April 96 */
 /* Four aggressive flags added April 96 - Ranger */
 /* New mob magic shield flag - Ranger May 96 */
 /* More aggressive acts - Ranger May 96 */
           }
         else
           {
           arg2 = strtok(NULL, " ");
           if(arg2)
             {
             if(!strcmp(arg2, "all"))
               {
               if(tmp ==1)
                 {
                 mob_proto_table[mob].act  = 0;
                 mob_proto_table[mob].act2 = 0;
                 }
               else
                 {
                 mob_proto_table[mob].act  = ACT_FINAL-1;
                 mob_proto_table[mob].act2 = ACT2_FINAL-1;
                 }
               }
             else
               {
               while (arg2)
                  {
                  act=1;
                  tmp1 = old_search_block(arg2, 0, strlen(arg2), action_bits, FALSE);
                  if(tmp1 == -1) {
                    act=2;
                    tmp1 = old_search_block(arg2, 0, strlen(arg2), action_bits2, FALSE);
                  }
                  if(tmp1 != -1)
                     {
                     if(tmp==1)
                       {
                       if(act==1)
                         mob_proto_table[mob].act &= ~(1<<(tmp1-1));
                       else
                         mob_proto_table[mob].act2 &= ~(1<<(tmp1-1));
                       }
                     else
                       {
                       if(act==1)
                         mob_proto_table[mob].act |=  1<<(tmp1-1);
                       else
                         mob_proto_table[mob].act2 |=  1<<(tmp1-1);
                       }
                     }
                   arg2 = strtok(NULL, " ");
                   }
                }
              send_to_char("Done.\n\r", ch);
              }
            }
          break;
      case EDIT_MOB_DAMAGE:
         arg2 = strtok(NULL, "\0");
         if(arg2)
         {
         sscanf(arg2, "%d d%d +%d", &tmp1, &tmp2, &tmp3);
         if(tmp1 >0 && tmp2 > 0 && tmp3 >= 0)
           {
           if(tmp3>32767)
             {
             send_to_char("`iA mob's damroll must be less than 32768, reseting it to 32767.`q\n\r",ch);
             tmp3=32767;
             }
           mob_proto_table[mob].damnodice = tmp1;
           mob_proto_table[mob].damsizedice = tmp2;
           mob_proto_table[mob].damroll = tmp3;
           send_to_char("Done.\n\r", ch);
           }
         }
         else
           send_to_char("edit mob <#> damage <#>d<#>+<#>.\n\r", ch);
         break;
      case EDIT_MOB_HITPOINT:
         arg2 = strtok(NULL, "\0");
         if(arg2)
         {
         sscanf(arg2, "%d d%d +%d", &tmp1, &tmp2, &tmp3);
         if(tmp1 >0 && tmp2 > 0 && tmp3 >= 0)
           {
           mob_proto_table[mob].hp_nodice = tmp1;
           mob_proto_table[mob].hp_sizedice = tmp2;
           mob_proto_table[mob].hp_add = tmp3;
           send_to_char("Done.\n\r", ch);
           }
         }
         else
           send_to_char("edit mob <#> hp <#>d<#>+<#>.\n\r", ch);
         break;
      case EDIT_MOB_MANA:
         arg2 = strtok(NULL, "\0");
         if(arg2)
         {
         sscanf(arg2, "%d d%d +%d", &tmp1, &tmp2, &tmp3);
         if(tmp1 >0 && tmp2 > 0 && tmp3 >= 0)
           {
           mob_proto_table[mob].mana_nodice = tmp1;
           mob_proto_table[mob].mana_sizedice = tmp2;
           mob_proto_table[mob].mana_add = tmp3;
           send_to_char("Done.\n\r", ch);
           }
         }
         else
           send_to_char("edit mob <#> mana <#>d<#>+<#>.\n\r", ch);
         break;
      case EDIT_MOB_NAME:
         mob_proto_table[mob].name[0] = 0;
         ch->desc->str = &mob_proto_table[mob].name;
         ch->desc->max_str = 50;
         break;
      case EDIT_MOB_SHORT:
         if(!mob_proto_table[mob].short_descr)
           CREATE(mob_proto_table[mob].short_descr, char,1);
         mob_proto_table[mob].short_descr[0] = 0;
         ch->desc->str = &mob_proto_table[mob].short_descr;
         ch->desc->max_str = 80;
         break;
      case EDIT_MOB_LONG:
         if(!mob_proto_table[mob].long_descr)
           CREATE(mob_proto_table[mob].long_descr, char,1);
         mob_proto_table[mob].long_descr[0] = 0;
         ch->desc->str = &mob_proto_table[mob].long_descr;
         ch->desc->max_str = MAX_STRING_LENGTH;
         break;
      case EDIT_MOB_FULL:
         if(!mob_proto_table[mob].description)
           CREATE(mob_proto_table[mob].description, char,1);
         mob_proto_table[mob].description[0] = 0;
         ch->desc->str = &mob_proto_table[mob].description;
         ch->desc->max_str = MAX_STRING_LENGTH;
         break;
      case EDIT_MOB_HITROLL:
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           if(atoi(arg2)>127)
             {
             send_to_char("`iA mob's hitroll must be less than 128, reseting it to 127.`q\n\r",ch);
             mob_proto_table[mob].hitroll=127;
             }
           else
             {
             mob_proto_table[mob].hitroll = atoi(arg2);
             send_to_char("Done.\n\r", ch);
             }
           }
         else
           send_to_char("edit mobile <#> hitroll <hitroll amount>.\n\r", ch);
         break;
      case EDIT_MOB_ARMOR:
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           mob_proto_table[mob].armor = atoi(arg2);
           send_to_char("Done.\n\r", ch);
           }
         else
           send_to_char("edit mobile <#> armor <armor amount>.\n\r", ch);
         break;
      case EDIT_MOB_ALIGNMENT:

         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           mob_proto_table[mob].alignment = atoi(arg2);
           send_to_char("Done.\n\r", ch);
           }
         else
           send_to_char("edit mobile <#> alignment <alignment amount>.\n\r", ch);
         break;
      case EDIT_MOB_GOLD:
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           mob_proto_table[mob].gold = atoi(arg2);
           send_to_char("Done.\n\r", ch);
           }
         else
           send_to_char("edit mobile <#> gold <amount>.\n\r", ch);
         break;
      case EDIT_MOB_EXP:
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           mob_proto_table[mob].exp = atoi(arg2);
           send_to_char("Done.\n\r", ch);
           }
         else
           send_to_char("edit mobile <#> experience <amount>.\n\r", ch);
         break;
      case EDIT_MOB_POSITION:
         arg2 = strtok(NULL, " ");
         tmp = -1;
         if(arg2)
         {
         if(isdigit(arg2[0]))
           {
           tmp = atoi(arg2);
           if(tmp >=0 && tmp <12)
             mob_proto_table[mob].position = tmp;
           else
             tmp = -1;
           }
         }
         if(tmp == -1)
            {
            send_to_char("Incorrect position, possible are:\n\r\
0)  Dead              4)  Sleeping    8)  Standing\n\r\
1)  Mortally Wounded  5)  Resting     9)  Flying\n\r\
2)  Incapacitated     6)  Sitting    10)  Riding\n\r\
3)  Stunned           7)  Fighting   11)  Swimming\n\r",ch);
            }
         else
           send_to_char("Done.\n\r", ch);
         break;
      case EDIT_MOB_DEFPOS:
         arg2 = strtok(NULL, " ");
         tmp = -1;
         if(arg2)
         {
         if(isdigit(arg2[0]))
           {
           tmp = atoi(arg2);
           if(tmp >=0 && tmp <12)
             mob_proto_table[mob].default_pos = tmp;
           else
             tmp = -1;
           }
         }
         if(tmp == -1)
            {
            send_to_char("Incorrect position, possible are:\n\r\
0)  Dead              4)  Sleeping    8)  Standing\n\r\
1)  Mortally Wounded  5)  Resting     9)  Flying\n\r\
2)  Incapacitated     6)  Sitting    10)  Riding\n\r\
3)  Stunned           7)  Fighting   11)  Swimming\n\r",ch);
            }
         else
           send_to_char("Done.\n\r", ch);
         break;

      case EDIT_MOB_ATTACK:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(!arg2) {
           send_to_char("edit mob <#> attack add <#> <type> <target> <percent> <spell>/0.\n\r", ch);
           send_to_char("or edit mob <#> attack rem <#>.\n\r\n\r", ch);
           send_to_char("  Types         Targets     Spells: See 'olchelp SPELL TYPES'\n\r",ch);
           send_to_char(" 1) SPELL CAST  1) BUFFER\n\r",ch);
           send_to_char(" 2) KICK        2) RAN_GROUP\n\r",ch);
           send_to_char(" 3) PUMMEL      3) RAN_ROOM\n\r",ch);
           send_to_char(" 4) PUNCH       4) GROUP\n\r",ch);
           send_to_char(" 5) BITE        5) ROOM\n\r",ch);
           send_to_char(" 6) CLAW        6) SELF\n\r",ch);
           send_to_char(" 7) BASH        7) LEADER\n\r",ch);
           send_to_char(" 8) TAILSLAM\n\r",ch);
           send_to_char(" 9) DISARM\n\r",ch);
           send_to_char("10) TRAMPLE\n\r",ch);
           send_to_char("11) SPELL SKILL\n\r",ch);
           break;
         }

         if(is_abbrev(arg2, "add")) tmp = 1;
         if(is_abbrev(arg2, "remove")) tmp = 2;

         arg2 = strtok(NULL, "\0");
         if((!arg2) || (tmp==-1)) {
           send_to_char("edit mob <#> attack add <#> <type> <target> <percent> <spell>/0.\n\r", ch);
           send_to_char("or edit mob <#> attack rem <#>.\n\r\n\r", ch);
           send_to_char("  Types         Targets     Spells: See 'olchelp SPELL TYPES'\n\r",ch);
           send_to_char(" 1) SPELL CAST  1) BUFFER\n\r",ch);
           send_to_char(" 2) KICK        2) RAN_GROUP\n\r",ch);
           send_to_char(" 3) PUMMEL      3) RAN_ROOM\n\r",ch);
           send_to_char(" 4) PUNCH       4) GROUP\n\r",ch);
           send_to_char(" 5) BITE        5) ROOM\n\r",ch);
           send_to_char(" 6) CLAW        6) SELF\n\r",ch);
           send_to_char(" 7) BASH        7) LEADER\n\r",ch);
           send_to_char(" 8) TAILSLAM\n\r",ch);
           send_to_char(" 9) DISARM\n\r",ch);
           send_to_char("10) TRAMPLE\n\r",ch);
           send_to_char("11) SPELL SKILL\n\r",ch);
           break;
         }

         if(tmp==1) {
           sscanf(arg2, "%d %d %d %d %d", &num, &tmp1, &tmp2, &tmp3, &tmp4);
           if(num  > 0 && num  < MAX_ATTS &&
              tmp1 > 0 && tmp1 < ATT_MAX  &&
              tmp2 > 0 && tmp2 < TAR_MAX  &&
              tmp3 > 0 && tmp3 < 101      && tmp4 >=0 ) {
             if( (tmp1==1 || tmp1==11) && tmp4==0) {
               send_to_char("The spell number can't be zero.\n\r",ch);
               return;
             }
             mob_proto_table[mob].no_att=mob_proto_table[mob].no_att+1;
             if(num>mob_proto_table[mob].no_att) num=mob_proto_table[mob].no_att;
             for(i=mob_proto_table[mob].no_att-1;i>=num;i--) {
               mob_proto_table[mob].att_type[i] = mob_proto_table[mob].att_type[i-1];
               mob_proto_table[mob].att_target[i] = mob_proto_table[mob].att_target[i-1];
               mob_proto_table[mob].att_percent[i] = mob_proto_table[mob].att_percent[i-1];
               mob_proto_table[mob].att_spell[i] = mob_proto_table[mob].att_spell[i-1];
             }
             mob_proto_table[mob].att_type[num-1] = tmp1;
             mob_proto_table[mob].att_target[num-1] = tmp2;
             mob_proto_table[mob].att_percent[num-1] = tmp3;
             mob_proto_table[mob].att_spell[num-1] = tmp4;
             send_to_char("Done.\n\r", ch);

           } else {
           send_to_char("edit mob <#> attack add <#> <type> <target> <percent> <spell>/0.\n\r\n\r", ch);
           send_to_char("  Types         Targets     Spells: See 'olchelp SPELL TYPES'\n\r",ch);
           send_to_char(" 1) SPELL CAST  1) BUFFER\n\r",ch);
           send_to_char(" 2) KICK        2) RAN_GROUP\n\r",ch);
           send_to_char(" 3) PUMMEL      3) RAN_ROOM\n\r",ch);
           send_to_char(" 4) PUNCH       4) GROUP\n\r",ch);
           send_to_char(" 5) BITE        5) ROOM\n\r",ch);
           send_to_char(" 6) CLAW        6) SELF\n\r",ch);
           send_to_char(" 7) BASH        7) LEADER\n\r",ch);
           send_to_char(" 8) TAILSLAM\n\r",ch);
           send_to_char(" 9) DISARM\n\r",ch);
           send_to_char("10) TRAMPLE\n\r",ch);
           send_to_char("11) SPELL SKILL\n\r",ch);
           }
           break;
         }

         if(tmp==2) {
           sscanf(arg2, "%d", &num);
           if(num  > 0 && num  < MAX_ATTS) {
             if(num>mob_proto_table[mob].no_att) num=mob_proto_table[mob].no_att;
             for(i=num-1;i<mob_proto_table[mob].no_att;i++) {
               mob_proto_table[mob].att_type[i] = mob_proto_table[mob].att_type[i+1];
               mob_proto_table[mob].att_target[i] = mob_proto_table[mob].att_target[i+1];
               mob_proto_table[mob].att_percent[i] = mob_proto_table[mob].att_percent[i+1];
               mob_proto_table[mob].att_spell[i] = mob_proto_table[mob].att_spell[i+1];
             }
             mob_proto_table[mob].no_att=mob_proto_table[mob].no_att-1;
             send_to_char("Done.\n\r", ch);
           } else {
             send_to_char("edit mob <#> attack rem <#>.\n\r", ch);
           }
           break;
         }
         break;
      case EDIT_MOB_LEVEL:
         arg2 = strtok(NULL, " ");
         if(arg2)
         {
         tmp = atoi(arg2);
         if(tmp >=0 && tmp <= LEVEL_MAX )
           {
           mob_proto_table[mob].level = tmp;
           send_to_char("Done.\n\r", ch);
           }
         else
           send_to_char("must be between 0 and LVL_MAX\r\n", ch);
         }
         else
           send_to_char("edit mobile <#> level <0-LVLMAX>\r\n", ch);
         break;
      case EDIT_MOB_CLASS:
         arg2 = strtok(NULL, " ");
         if(arg2)
         {
         tmp = atoi(arg2);
         if(tmp >=0 && tmp <= CLASS_MOB_LAST )
           {
           mob_proto_table[mob].class = tmp;
           send_to_char("Done.\n\r", ch);
           }
         else
           send_to_char("must be between 0 and CLASS_MAX\r\n", ch);
         }
         else
           send_to_char("edit mobile <#> class <0-CLASS_MAX>\r\n", ch);
         break;
      case EDIT_MOB_IMMUNE:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           if(is_abbrev(arg2, "remove"))
             tmp = 1;
           if(is_abbrev(arg2, "add"))
             tmp = 2;
           }
         if(tmp==-1)
           {
           send_to_char("edit mob <#> immune <add|remove> all, or a list of the following:\r\n\
FIRE, ELECTRIC, POISON, PUMMEL, KICK, PUNCH, SLEEP,\r\n\
CHARM, BLINDNESS, PARALYSIS, DRAIN, DISEMBOWEL, DISINTEGRATE, CLAIR,\r\n\
SUMMON, HIT, BLUDGEON, PIERCE, SLASH, WHIP, CLAW, BITE, STING, CRUSH,\r\n\
HACK, CHOP, SLICE, BACKSTAB, AMBUSH, ASSAULT, LOCATE, COLD, SOUND,\n\r\
CHEMICAL, ACID, FEAR, CIRCLE, STEAL.\r\n", ch); /* Removed Throatstrike and Execute */
           }
         else
           {
           arg2 = strtok(NULL, " ");
           if(arg2) {
             if(!strcmp(arg2, "all")) {
               if(tmp ==1) {
                 mob_proto_table[mob].immune=0;
                 mob_proto_table[mob].immune2=0;
               }
               else {
                 tmp1=old_search_block("\n", 0, strlen("\n"), immune_bits, FALSE);
                 mob_proto_table[mob].immune = (1<<(tmp1-1)) -1;
                 tmp1=old_search_block("\n", 0, strlen("\n"), immune_bits2, FALSE);
                 mob_proto_table[mob].immune2 = (1<<(tmp1-1)) -1;
               }
             }
             else {
               while(arg2) {
                 immune=1;
                 tmp1=old_search_block(string_to_upper(arg2), 0, strlen(arg2), immune_bits, FALSE);
                 if(tmp1==-1) {
                   immune=2;
                   tmp1=old_search_block(string_to_upper(arg2), 0, strlen(arg2), immune_bits2, FALSE);
                 }
                 if(tmp1!=-1) {
                   if(immune==1) {
                     if(tmp==1) mob_proto_table[mob].immune &= ~(1<<(tmp1-1));
                     else mob_proto_table[mob].immune |=  1<<(tmp1-1);
                   }
                   else {
                     if(tmp==1) mob_proto_table[mob].immune2 &= ~(1<<(tmp1-1));
                     else mob_proto_table[mob].immune2 |=  1<<(tmp1-1);
                   }
                 }
                 arg2 = strtok(NULL, " ");
               }
             }
             send_to_char("Done.\n\r", ch);
           }
          }
          break;
      case EDIT_MOB_RESIST:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           if(is_abbrev(arg2, "remove"))
             tmp = 1;
           if(is_abbrev(arg2, "add"))
             tmp = 2;
           }
         if(tmp==-1)
           {
           send_to_char("edit mob <#> resist <add|remove> all, or a list of the following:\r\n\
POISON, PHYSICAL, MAGICAL, FIRE, COLD, ELECTRIC, SOUND, CHEMICAL, ACID.\r\n", ch);
           }
         else
           {
           arg2 = strtok(NULL, " ");
           if(arg2) {
             if(!strcmp(arg2, "all")) {
               if(tmp ==1) {
                 mob_proto_table[mob].resist=0;
               }
               else {
                 tmp1=old_search_block("\n", 0, strlen("\n"), resist_bits, FALSE);
                 mob_proto_table[mob].resist = (1<<(tmp1-1)) -1;
               }
             }
             else {
               while(arg2) {
                 tmp1=old_search_block(string_to_upper(arg2), 0, strlen(arg2), resist_bits, FALSE);
                 if(tmp1!=-1) {
                   if(tmp==1) mob_proto_table[mob].resist &= ~(1<<(tmp1-1));
                   else mob_proto_table[mob].resist |=  1<<(tmp1-1);
                 }
                 arg2 = strtok(NULL, " ");
               }
             }
             send_to_char("Done.\n\r", ch);
           }
          }
          break;
      case EDIT_MOB_RESET:
        mob_proto_table[mob].name        = str_dup("primal clay blank");
        mob_proto_table[mob].short_descr = str_dup("blank of primal clay");
        mob_proto_table[mob].long_descr  = str_dup("A blank of primal clay stands here, waiting to be shaped\n\r");
        mob_proto_table[mob].description = str_dup("Featureless and non-descript, the blank of primal clay waits for someone to breath life into it.\n\r");

        mob_proto_table[mob].func = NULL;
        /* *** Numeric data *** */

        mob_proto_table[mob].act    = 0;
        mob_proto_table[mob].act2   = 0;
        mob_proto_table[mob].number = 0;
        SET_BIT(mob_proto_table[mob].act, ACT_ISNPC);
        mob_proto_table[mob].affected_by = 0;
        mob_proto_table[mob].affected_by2 = 0;
        mob_proto_table[mob].alignment = 0;
        mob_proto_table[mob].level = 1;
        mob_proto_table[mob].hitroll = MIN(mob_proto_table[mob].level,(20 - 1));
        mob_proto_table[mob].armor  = 100;
        mob_proto_table[mob].hp_nodice   = 1;
        mob_proto_table[mob].hp_sizedice = 1;
        mob_proto_table[mob].hp_add      = 1;
        mob_proto_table[mob].damnodice   = 1;
        mob_proto_table[mob].damsizedice = 1;
        mob_proto_table[mob].damroll     = 1;
        mob_proto_table[mob].gold = 1;
        mob_proto_table[mob].exp = 1;
        mob_proto_table[mob].position = POSITION_STANDING;
        mob_proto_table[mob].default_pos = POSITION_STANDING;
        mob_proto_table[mob].sex = SEX_MALE;
        mob_proto_table[mob].class = 0;
        mob_proto_table[mob].immune = 0;
        mob_proto_table[mob].mana_nodice   = 1;
        mob_proto_table[mob].mana_sizedice = 1;
        mob_proto_table[mob].mana_add      = 1;
        mob_proto_table[mob].no_att = 0;
        mob_proto_table[mob].hit_type = 0;
        mob_proto_table[mob].act2=0;
        mob_proto_table[mob].affected_by2=0;
        mob_proto_table[mob].immune2=0;
        mob_proto_table[mob].resist=0;
        mob_proto_table[mob].skin_value=0;
        mob_proto_table[mob].skin_vnum[0]=0;
        mob_proto_table[mob].skin_vnum[1]=0;
        mob_proto_table[mob].skin_vnum[2]=0;
        mob_proto_table[mob].skin_vnum[3]=0;
        mob_proto_table[mob].skin_vnum[4]=0;
        mob_proto_table[mob].skin_vnum[5]=0;
        break;
      case EDIT_MOB_SKIN:
         arg2 = strtok(NULL, " ");
         edit = -1;
         if(arg2) {
         if(is_abbrev(arg2, "value"))
            edit = 0;
         else if(is_abbrev(arg2, "load1"))
            edit = 1;
         else if(is_abbrev(arg2, "load2"))
            edit = 2;
         else if(is_abbrev(arg2, "load3"))
            edit = 3;
         else if(is_abbrev(arg2, "load4"))
            edit = 4;
         else if(is_abbrev(arg2, "load5"))
            edit = 5;
         else if(is_abbrev(arg2, "load6"))
            edit = 6;
         }
         if(edit==-1) {
           send_to_char("edit mob <#> skin value/load1/load2/load3/load4/load5/load6 <number>.\n\r", ch);
           return;
         }
         arg2 = strtok(NULL, " ");
         if(!arg2) {
           send_to_char("edit mob <#> skin value/load1/load2/load3/load4/load5/load6 <number>.\n\r", ch);
           return;
         }

         tmp=atoi(arg2);
         if(edit==0)
           mob_proto_table[mob].skin_value=tmp;
         else
           mob_proto_table[mob].skin_vnum[edit-1]=tmp;
         send_to_char("Done.\n\r", ch);
         break;

      default:
        send_to_char("Mobile editables include:\r\n\
act, affects, align, armor, damage, defaultpos, full, hitroll, position, short,\r\n\
sex, name, hp, exp, level, long, gold, class, attack, mana, immune, reset,\n\r\
tagline, resist, skin.\r\n", ch);
        break;
      }
    }
    else
      send_to_char("You cannot edit a mobile proto that has an instance loaded in the world.\n\r",ch);
    }
}

#define EDIT_OBJ_NAME            1
#define EDIT_OBJ_SHORT           2
#define EDIT_OBJ_LONG            3
#define EDIT_OBJ_ACTION_DESC     4
#define EDIT_OBJ_EXTRA_DESC      5
#define EDIT_OBJ_AFFECTS         6
#define EDIT_OBJ_TYPE            7
#define EDIT_OBJ_WEAR            8
#define EDIT_OBJ_FLAGS           9
#define EDIT_OBJ_COST            11
#define EDIT_OBJ_RENT_COST       12
#define EDIT_OBJ_TIMER           13
#define EDIT_OBJ_BITVECT         14
#define EDIT_OBJ_REPOP           15
#define EDIT_OBJ_VALUES          16
#define EDIT_OBJ_WEIGHT          17
#define EDIT_OBJ_RESET           18 /* Ranger - May 96 */
#define EDIT_OBJ_ACTION_DESC_NT  19
#define EDIT_OBJ_SUBCLASS_RES    20
#define EDIT_OBJ_MATERIAL        21
#define EDIT_OBJ_CHAR_WEAR_DESC  22
#define EDIT_OBJ_ROOM_WEAR_DESC  23
#define EDIT_OBJ_CHAR_REM_DESC   24
#define EDIT_OBJ_ROOM_REM_DESC   25


void edit_obj(CHAR *ch, char *fargs)
{
  char args[MAX_STRING_LENGTH];
  int edit,tmp,tmp1, tmp2,tmp3,loc,extra,found, iReadCount;
  char *arg2;
  int v_object=-1, object, zone;
  struct extra_descr_data *tmp_descr = NULL, **prev_descr_ptr = NULL, *tmp_descr1 = NULL;
  sprintf(args, "%s", fargs);
  arg2 = strtok(args, " ");
  if(arg2)
     v_object = atoi(arg2);
  zone = inzone(v_object);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  object = real_object(v_object);
  if(object >= 0 && object <= top_of_objt)
    {
/*    if(obj_proto_table[object].number == 0)*/
    if(1)
    {
    arg2 = strtok(NULL, " ");
    edit = -1;
    if(arg2)
    {
    if(is_abbrev(arg2, "name"))
      edit = EDIT_OBJ_NAME;
    if(is_abbrev(arg2, "timer"))
      edit = EDIT_OBJ_TIMER;
    if(is_abbrev(arg2, "repop"))
      edit = EDIT_OBJ_REPOP;
    if(is_abbrev(arg2, "bitvect"))
      edit = EDIT_OBJ_BITVECT;
    if(is_abbrev(arg2, "rentcost"))
      edit = EDIT_OBJ_RENT_COST;
    if(is_abbrev(arg2, "short"))
      edit = EDIT_OBJ_SHORT;
    if(is_abbrev(arg2, "long"))
      edit = EDIT_OBJ_LONG;
    if(is_abbrev(arg2, "action_nt"))
      edit = EDIT_OBJ_ACTION_DESC_NT;
    if(is_abbrev(arg2, "action"))
      edit = EDIT_OBJ_ACTION_DESC;
    if(is_abbrev(arg2, "extradescription"))
      edit = EDIT_OBJ_EXTRA_DESC;
    if(is_abbrev(arg2, "affects"))
      edit = EDIT_OBJ_AFFECTS;
    if(is_abbrev(arg2, "type"))
      edit = EDIT_OBJ_TYPE;
    if(is_abbrev(arg2, "wear"))
      edit = EDIT_OBJ_WEAR;
    if(is_abbrev(arg2, "flags"))
      edit = EDIT_OBJ_FLAGS;
    if(is_abbrev(arg2, "weight"))
      edit = EDIT_OBJ_WEIGHT;
    if(is_abbrev(arg2, "values"))
      edit = EDIT_OBJ_VALUES;
    if(is_abbrev(arg2, "cost"))
      edit = EDIT_OBJ_COST;
    if(is_abbrev(arg2, "reset"))
      edit = EDIT_OBJ_RESET;
    if(is_abbrev(arg2, "subclass_res"))
      edit = EDIT_OBJ_SUBCLASS_RES;
    if(is_abbrev(arg2, "material"))
      edit = EDIT_OBJ_MATERIAL;
    if(is_abbrev(arg2, "char_wear_desc"))
      edit = EDIT_OBJ_CHAR_WEAR_DESC;
    if(is_abbrev(arg2, "room_wear_desc"))
      edit = EDIT_OBJ_ROOM_WEAR_DESC;
    if(is_abbrev(arg2, "char_rem_desc"))
      edit = EDIT_OBJ_CHAR_REM_DESC;
    if(is_abbrev(arg2, "room_rem_desc"))
      edit = EDIT_OBJ_ROOM_REM_DESC;
    }
    switch(edit)
      {
      case EDIT_OBJ_EXTRA_DESC:
        arg2 = strtok(NULL, " ");
        tmp = -1;
        if(arg2)
        {
        if(is_abbrev(arg2, "add"))
          tmp = 1;
        if(is_abbrev(arg2, "remove"))
          tmp = 2;
        }
        if(tmp!=-1)
          {
          switch(tmp)
            {
            case 1:
             arg2 = strtok(NULL, "\0");
             if(arg2)
             {
             send_to_char("Enter the extra description, terminate with @ on its own line.\n\r",ch);
             CREATE(tmp_descr, struct extra_descr_data, 1);
             tmp_descr->keyword = str_dup(arg2);
             tmp_descr->next = obj_proto_table[object].ex_description;
             ch->desc->str = &tmp_descr->description;
             ch->desc->max_str = LEN_EXTRADESC_MAX;
             obj_proto_table[object].ex_description = tmp_descr;
             tmp_descr = NULL;
             }
             break;
           case 2:
             arg2 = strtok(NULL, "\0");
             if(arg2) {
               tmp_descr= obj_proto_table[object].ex_description;
               prev_descr_ptr = &obj_proto_table[object].ex_description;
               found=0;
               while(tmp_descr) {
                 if(!strcmp(tmp_descr->keyword, arg2)) {
                   /*delete current tmp_descr*/
                   *prev_descr_ptr = tmp_descr->next;
                   if(tmp_descr->description)
                     free(tmp_descr->description);
                   if(tmp_descr->keyword)
                     free(tmp_descr->keyword);
                   tmp_descr1 = tmp_descr->next;
                   free(tmp_descr);
                   tmp_descr = tmp_descr1;
                   send_to_char("Extra description deleted.\n\r",ch);
                   found=1;
                 }
                 else {
                   prev_descr_ptr = &tmp_descr->next;
                   tmp_descr = tmp_descr->next;
                 }
               }
               if(!found) send_to_char("Extra description not found.\n\r",ch);
             }
             break;
             }
          }
        else
          send_to_char("edit obj <#> extra <add|remove> <keywords>\n\rWhen removing, all keywords must be used.\n\r",ch);
        break;
      case EDIT_OBJ_VALUES:
        arg2 = strtok(NULL, "\0");
        if(arg2 ? isdigit(arg2[0]) : FALSE)
          {
          iReadCount = sscanf(arg2, "%d %d %d %d", &tmp, &tmp1, &tmp2, &tmp3);
          if (iReadCount < 4) tmp3 = -1;
          if (iReadCount < 3) tmp2 = -1;
          if (iReadCount < 2) tmp1 = -1;
          if (iReadCount < 1) tmp = -1;
          obj_proto_table[object].obj_flags.value[0] = tmp;
          obj_proto_table[object].obj_flags.value[1] = tmp1;
          obj_proto_table[object].obj_flags.value[2] = tmp2;
          obj_proto_table[object].obj_flags.value[3] = tmp3;
          }
        else
          {
          switch(obj_proto_table[object].obj_flags.type_flag)
            {
            case ITEM_LIGHT :
              send_to_char("For light sources:  <Color> <Type> <Hours> <not used>\n\r", ch);
              break;
            case ITEM_RECIPE :
              send_to_char("For Recipes: <Creates> <Requires> <Requires> <Requires> (-1 for none)\n\r", ch);
            break;
            case ITEM_AQ_ORDER :
              send_to_char("For AQ Orders: <Requires> <Requires> <Requires> <Requires> (-1 for none)\n\r", ch);
            break;
            case ITEM_SCROLL :
            case ITEM_POTION :
              send_to_char("For Scrolls and Potions:  <Level> <Spell1|0> <Spell2|0> <Spell3|0>\n\r", ch);
              break;
            case ITEM_WAND :
            case ITEM_STAFF :
              send_to_char("For Staves and Wands:  <Level> <Max Charges> <Charges Left> <Spell>\n\r", ch);
              break;
         case ITEM_2HWEAPON:
            case ITEM_WEAPON :
              send_to_char("For weapons:  <olchelp weapon attacks> <dice damage> <dice size> <type>\n\r", ch);
              break;
            case ITEM_FIREWEAPON :
              send_to_char("For guns:  <license number> <bullets left> <dam dice number> <dam dice size>\n\r", ch);
              break;
            case ITEM_MISSILE :
              send_to_char("For thrown weapons:  <dam dice number> <dam dice size> <unused> <unused>\n\r", ch);
              break;
            case ITEM_ARMOR :
              send_to_char("For armor:  <AC apply(positive better)> <unused> <unused> <unused>\n\r", ch);
              break;
            case ITEM_TRAP :
              send_to_char("For traps:  <spell> <hp's> <unused> <unused>\n\r", ch);
              break;
            case ITEM_CONTAINER :
              send_to_char("For containers:  <max contains> <how locked> <key #> <corpse>\n\r", ch);
              break;
            case ITEM_NOTE :
              send_to_char("For notes:  <tongue> <unused> <unused> <unused>\n\r", ch);
              break;
            case ITEM_DRINKCON :
              send_to_char("For drink containers:  <max contains> <current contains> <liquid> <poison>\n\r", ch);
              break;
            case ITEM_BULLET :
              send_to_char("For bullets:  <unused> <unused> <gun #> <unused>\n\r", ch);
              break;
            case ITEM_KEY :
              send_to_char("For keys:  <keytype> <unused> <unused> <unused>\n\r", ch);
              break;
            case ITEM_FOOD :
              send_to_char("For food:  <feeds> <unused> <unused> <poisoned>\n\r", ch);
              break;
            case ITEM_MONEY :
              send_to_char("For money:  <coins> <unused> <unused> <unused>\n\r", ch);
              break;
            case ITEM_LOCKPICK :
              send_to_char("For lockpicks:  <# picks> <Max # picks> <unused> <unused>\n\r", ch);
              break;
            case ITEM_BOARD :
              send_to_char("For boards:  <Min Read Lvl> <Min Write Lvl> <Min Remove Lvl> <unused>\n\r", ch);
              break;
            case ITEM_SC_TOKEN:
              send_to_char("For subclass tokens: <Subclass Points Given> <unused> <unused> <unused>\n\r",ch);
            case ITEM_SKIN:
              send_to_char("For skins: <All unused> Use COST for value of skin\n\r",ch);
            } /* switch */
          }
         break;
      case EDIT_OBJ_TYPE:
         arg2 = strtok(NULL, " ");
         tmp2 = -1;
         if(arg2)
           {
           tmp2 = old_search_block(arg2, 0, strlen(arg2), item_types, FALSE);
           if(tmp2!=-1)
             {
             obj_proto_table[object].obj_flags.type_flag = tmp2-1;
             send_to_char("Done.\n\r",ch);
             return;
             }
           }
         if(tmp2 == -1)
           send_to_char("edit object <#> type <type> where <type> is on of the following:\n\r\
LIGHT, SCROLL, WAND, STAFF, WEAPON, FIRE WEAPON, MISSILE, TREASURE, ARMOR, \n\r\
POTION, WORN, OTHER, TRASH, TRAP, CONTAINER, NOTE, LIQUID, KEY, FOOD, MONEY,\n\r\
PEN, BOAT, BULLET, MUSICAL, LOCKPICK, 2H-WEAPON, BOARD, SC_TOKEN, SKIN, RECIPE,\n\r\
AQ_ORDER.\n\r", ch);
         break;
      case EDIT_OBJ_AFFECTS:
         tmp1 = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
         {
         if(is_abbrev(arg2, "add"))
           tmp1 = 1;
         if(is_abbrev(arg2, "remove"))
           tmp1 = 2;
         }
         if(tmp1 != -1)
           {
           switch(tmp1)
              {
              case 1: /*  add */
                if( obj_proto_table[object].affected[0].location == 0 ||
                    obj_proto_table[object].affected[1].location == 0 ||
                    obj_proto_table[object].affected[2].location == 0 )
                  {
                  arg2 = strtok(NULL, " ");
                  if(arg2)
                    tmp2 = old_search_block(arg2, 0, strlen(arg2), apply_types, FALSE);
                  if(tmp2 != -1)
                    {
                    arg2 = strtok(NULL, "\0");
                    tmp3=  -9999;
                    if(arg2)
                      if(is_number(arg2))
                        tmp3=atoi(arg2);
                    if(tmp3 != -9999)
                      {
                      if(obj_proto_table[object].affected[0].location == 0)
                          {
                          obj_proto_table[object].affected[0].location = tmp2-1;
                          obj_proto_table[object].affected[0].modifier = tmp3;
                          }
                      else if(obj_proto_table[object].affected[1].location == 0)
                          {
                          obj_proto_table[object].affected[1].location = tmp2-1;
                          obj_proto_table[object].affected[1].modifier = tmp3;
                          }
                      else if(obj_proto_table[object].affected[2].location == 0)
                          {
                          obj_proto_table[object].affected[2].location = tmp2-1;
                          obj_proto_table[object].affected[2].modifier = tmp3;
                          }
                      send_to_char("Done.\n\r",ch);
                      return;
                      }
                    else
                      send_to_char("edit object <#> affects add <location> <amount>, where <location>:\n\r\
NONE, STR, DEX, INT, WIS, CON, AGE, MANA, HIT, MOVE, ARMOR, HITROLL, DAMROLL,\n\r\
SAVING_PARA, SAVING_ROD, SAVING_PETRI, SAVING_BREATH, SAVING_SPELL,\n\r\
SKILL_SNEAK, SKILL_HIDE, SKILL_STEAL, SKILL_BACKSTAB, SKILL_PICKLOCK,\n\r\
SKILL_KICK, SKILL_BASH, SKILL_RESCUE, SKILL_BLOCK, SKILL_KNOCK, \n\r\
SKILL_PUNCH, SKILL_PARRY, SKILL_DUAL, SKILL_THROW, SKILL_DODGE,\n\r\
SKILL_PEEK, SKILL_BUTCHER, SKILL_TRAP, SKILL_DISARM, SKILL_SUBDUE,\n\r\
SKILL_CIRCLE, SKILL_TRIPLE, SKILL_PUMMEL, SKILL_AMBUSH, SKILL_ASSAULT\n\r\
SKILL_DISEMBOWEL, SKILL_BACKFLIP, MANA_REGEN, HP_REGEN\n\r.", ch);
                    }
                  else
                      send_to_char("edit object <#> affects add <location> <amount>, where <location>:\n\r\
NONE, STR, DEX, INT, WIS, CON, AGE, MANA, HIT, MOVE, ARMOR, HITROLL, DAMROLL,\n\r\
SAVING_PARA, SAVING_ROD, SAVING_PETRI, SAVING_BREATH, SAVING_SPELL,\n\r\
SKILL_SNEAK, SKILL_HIDE, SKILL_STEAL, SKILL_BACKSTAB, SKILL_PICKLOCK,\n\r\
SKILL_KICK, SKILL_BASH, SKILL_RESCUE, SKILL_BLOCK, SKILL_KNOCK, \n\r\
SKILL_PUNCH, SKILL_PARRY, SKILL_DUAL, SKILL_THROW, SKILL_DODGE,\n\r\
SKILL_PEEK, SKILL_BUTCHER, SKILL_TRAP, SKILL_DISARM, SKILL_SUBDUE,\n\r\
SKILL_CIRCLE, SKILL_TRIPLE, SKILL_PUMMEL, SKILL_AMBUSH, SKILL_ASSAULT\n\r\
SKILL_DISEMBOWEL, SKILL_BACKFLIP, MANA_REGEN, HP_REGEN\n\r.", ch);
                  }
                else
                  printf_to_char(ch,"Max of %d AFFECTS on each object.\n\r",MAX_OBJ_AFFECT);
                break;
              case 2: /*  remove */
                arg2 =  strtok(NULL, " ");
                while (arg2)
                  {
                  tmp2 = old_search_block(arg2, 0, strlen(arg2), apply_types, FALSE);
                  if(tmp2-1 == obj_proto_table[object].affected[0].location)
                    {
                    obj_proto_table[object].affected[0].location = 0;
                    obj_proto_table[object].affected[0].modifier = 0;
                    }
                  else if(tmp2-1 == obj_proto_table[object].affected[1].location)
                    {
                    obj_proto_table[object].affected[1].location = 0;
                    obj_proto_table[object].affected[1].modifier = 0;
                    }
                  else if(tmp2-1 == obj_proto_table[object].affected[2].location)
                    {
                    obj_proto_table[object].affected[2].location = 0;
                    obj_proto_table[object].affected[2].modifier = 0;
                    }
                  else
                    send_to_char("Affect did not exist on item\n\r", ch);
                  arg2 = strtok(NULL, " ");
                  }
                break;
              }
            }
            else
              send_to_char("edit object <#> affects <add|remove> <location> <amount>, where <location>:\n\r\
NONE, STR, DEX, INT, WIS, CON, AGE, MANA, HIT, MOVE, ARMOR, HITROLL, DAMROLL,\n\r\
SAVING_PARA, SAVING_ROD, SAVING_PETRI, SAVING_BREATH, SAVING_SPELL,\n\r\
SKILL_SNEAK, SKILL_HIDE, SKILL_STEAL, SKILL_BACKSTAB, SKILL_PICKLOCK,\n\r\
SKILL_KICK, SKILL_BASH, SKILL_RESCUE, SKILL_BLOCK, SKILL_KNOCK, \n\r\
SKILL_PUNCH, SKILL_PARRY, SKILL_DUAL, SKILL_THROW, SKILL_DODGE,\n\r\
SKILL_PEEK, SKILL_BUTCHER, SKILL_TRAP, SKILL_DISARM, SKILL_SUBDUE,\n\r\
SKILL_CIRCLE, SKILL_TRIPLE, SKILL_PUMMEL, SKILL_AMBUSH, SKILL_ASSAULT\n\r\
SKILL_DISEMBOWEL, SKILL_BACKFLIP, MANA_REGEN, HP_REGEN\n\r.", ch);
         break;
      case EDIT_OBJ_WEAR:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
         {
         if(is_abbrev(arg2, "remove"))
            tmp = 1;
         if(is_abbrev(arg2, "add"))
            tmp = 2;
         }
         if(tmp==-1)
           {
           send_to_char("edit object <#> wear <add|remove> all, or a list of the following:\r\n\
TAKE, FINGER, NECK, BODY, HEAD, LEGS, FEET, HANDS, ARMS, SHIELD, ABOUT,\n\r\
WAIST, WRIST, WIELD, HOLD, THROW, LIGHT-SOURCE, NO_REMOVE, NO_SCAVENGE.\
.\n\r", ch);
           }
         else
           {
           arg2 = strtok(NULL, " ");
           if(arg2)
             {
             if(!strcmp(arg2, "all"))
               {
               if(tmp ==1)
                 obj_proto_table[object].obj_flags.wear_flags = 0;
               else
                 obj_proto_table[object].obj_flags.wear_flags = (ITEM_NO_SCAVENGE<<1) -1;
               send_to_char("Done.\n\r",ch);
               }
             else
              {
              found=1;
              while (arg2)
                {
                tmp1 = old_search_block(arg2, 0, strlen(arg2), wear_bits, FALSE);
                if(tmp1 != -1)
                  {
                  if(tmp==1)
                    obj_proto_table[object].obj_flags.wear_flags &= ~(1<<(tmp1-1));
                  else
                    obj_proto_table[object].obj_flags.wear_flags |=  1<<(tmp1-1);
                  }
                else
                  {
                    found=0;
                  }
                arg2 = strtok(NULL, " ");
                }
                if(found) send_to_char("Done.\n\r", ch);
                else send_to_char("Some wear flags didn't exist.\n\r",ch);
               }
             }
           }
         break;
      case EDIT_OBJ_MATERIAL:
         send_to_char("Not implemented yet.\n\r",ch);
         return;
         break;
      case EDIT_OBJ_FLAGS:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           if(is_abbrev(arg2, "remove"))
             tmp = 1;
           if(is_abbrev(arg2, "add"))
             tmp = 2;
           }
         if(tmp==-1)
           {
           send_to_char("edit object <#> flags <add|remove> all, or a list of the following:\r\n\
GLOW, HUM, DARK, CLONE, EVIL, INVISIBLE, MAGIC, NODROP, BLESS, ANTI-GOOD,\n\r\
ANTI-EVIL, ANTI-NEUTRAL, ANTI-WARRIOR, ANTI-CLERIC, ANTI-THIEF,\n\r\
ANTI-MAGIC_USER, ANTI-MORTAL, ANTI-RENT, ANTI-NINJA, ANTI-NOMAD,\n\r\
ANTI-PALADIN, ANTI-ANTI-PALADIN, ANTI-AVATAR, ANTI-BARD,\n\r\
ANTI-COMMANDO, LIMITED, ANTI-AUCTION, CHAOTIC, RANDOM, ALL_DECAY,\n\r\
EQ_DECAY, NO_GIVE, NO_GIVE_MOB, NO_PUT, NO_TAKE_MOB, NO_LOCATE,\n\r\
RANDOM_0, RANDOM_1, RANDOM_2.\n\r", ch);
           }
         else
           {
           arg2 = strtok(NULL, " ");
           if(arg2)
             {
             if(!strcmp(arg2, "all"))
               {
               if(tmp ==1) {
                 obj_proto_table[object].obj_flags.extra_flags = 0;
                 obj_proto_table[object].obj_flags.extra_flags2 = 0;
               }
               else {
                 obj_proto_table[object].obj_flags.extra_flags = (ITEM_CHAOTIC<<1) -1;
                 obj_proto_table[object].obj_flags.extra_flags2 = (ITEM_RANDOM_2<<1) -1;
               }
               send_to_char("Done.\n\r",ch);
             }
             else
               {
               found=1;
               while (arg2)
                 {
                 extra=1;
                 tmp1 = old_search_block(arg2, 0, strlen(arg2), extra_bits, FALSE);
                 if(tmp1 == -1) {
                   extra=2;
                   tmp1 = old_search_block(arg2, 0, strlen(arg2), extra_bits2, FALSE);
                 }

                 if(tmp1 != -1)
                   {
                   if(tmp==1)
                     {
                     if(extra==1)
                       obj_proto_table[object].obj_flags.extra_flags &= ~(1<<(tmp1-1));
                     else
                       obj_proto_table[object].obj_flags.extra_flags2 &= ~(1<<(tmp1-1));
                     }
                   else
                     {
                     if(extra==1)
                       obj_proto_table[object].obj_flags.extra_flags |=  1<<(tmp1-1);
                     else
                       obj_proto_table[object].obj_flags.extra_flags2 |=  1<<(tmp1-1);
                     }
                   }
                 else
                   {
                   found=0;
                   }
                 arg2 = strtok(NULL, " ");
                 }
               if(found) send_to_char("Done.\n\r", ch);
               else send_to_char("Some extra flags not found.\n\r",ch);
               }
             }
           }
          break;

      case EDIT_OBJ_SUBCLASS_RES:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           if(is_abbrev(arg2, "remove"))
             tmp = 1;
           if(is_abbrev(arg2, "add"))
             tmp = 2;
           }
         if(tmp==-1)
           {
           send_to_char("edit object <#> subclass_res <add|remove> all, or a list of the following:\r\n\
ANTI_ENCHANTER, ANTI_ARCHMAGE, ANTI_DRUID, ANTI_TEMPLAR, ANTI_ROGUE,\n\r\
ANTI_BANDIT, ANTI_WARLORD, ANTI_GLADIATOR, ANTI_RONIN, ANTI_MYSTIC,\n\r\
ANTI_RANGER, ANTI_TRAPPER, ANTI_CAVALIER, ANTI_CRUSADER, ANTI_DEFILER,\n\r\
ANTI_INFIDEL, ANTI_BLADESINGER, ANTI_CHANTER, ANTI_LEGIONNAIRE,\n\r\
ANTI_MERCENARY.\n\r", ch);
           }
         else
           {
           arg2 = strtok(NULL, " ");
           if(arg2)
             {
             if(!strcmp(arg2, "all"))
               {
               if(tmp ==1) {
                 obj_proto_table[object].obj_flags.subclass_res = 0;
               }
               else {
                 obj_proto_table[object].obj_flags.subclass_res = (ITEM_ANTI_MERCENARY<<1) -1;
               }
               send_to_char("Done.\n\r",ch);
             }
             else
               {
               found=1;
               while (arg2)
                 {
                 tmp1 = old_search_block(arg2, 0, strlen(arg2), subclass_res_bits, FALSE);
                 if(tmp1 != -1)
                   {
                   if(tmp==1) {
                       obj_proto_table[object].obj_flags.subclass_res &= ~(1<<(tmp1-1));
                   }
                   else {
                       obj_proto_table[object].obj_flags.subclass_res |=  1<<(tmp1-1);
                   }
                  }
                  else {
                    found=0;
                  }
                 arg2 = strtok(NULL, " ");
                 }
                 if(found) send_to_char("Done.\n\r", ch);
                 else send_to_char("Some subclass restrictions not found.\n\r",ch);
               }
             }
           }
          break;

      case EDIT_OBJ_BITVECT:
         tmp = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           if(is_abbrev(arg2, "remove"))
             tmp = 1;
           if(is_abbrev(arg2, "add"))
             tmp = 2;
           }
         if(tmp==-1)
           {
           send_to_char("edit object <#> bitvect <add|remove> all, or a list of the following:\r\n\
BLIND, INVISIBLE, DETECT-ALIGNMENT, DETECT-INVISIBLE, DETECT-MAGIC,\r\n\
SENSE-LIFE, HOLD, SANCTUARY, CURSE, SPHERE, POISON, PROTECT-EVIL,\r\n\
PARALYSIS, INFRAVISION, STATUE, SLEEP, DODGE, SNEAK, HIDE, FLY, IMINV,\r\n\
INVUL, DUAL, FURY, PROTECT-GOOD, TRIPLE, QUAD.\r\n", ch);
           }
         else
           {
           arg2 = strtok(NULL, " ");
           if(arg2)
             {
             if(!strcmp(arg2, "all"))
               {
               if(tmp ==1) {
                 obj_proto_table[object].obj_flags.bitvector = 0;
                 obj_proto_table[object].obj_flags.bitvector2 = 0;
                }
               else {
                 obj_proto_table[object].obj_flags.bitvector = AFF_FINAL-1;
                 obj_proto_table[object].obj_flags.bitvector2 = AFF2_FINAL-1;
               }
               send_to_char("Done.\n\r",ch);
               }
             else
               {
               found=1;
               while (arg2) {
                 tmp1 = old_search_block(arg2, 0, strlen(arg2), affected_bits, FALSE);
                 if(tmp1 != -1) {
                   if(tmp==1)
                     obj_proto_table[object].obj_flags.bitvector &= ~(1<<(tmp1-1));
                   else
                     obj_proto_table[object].obj_flags.bitvector |=  1<<(tmp1-1);
                 }
                 else {
                   tmp1 = old_search_block(arg2, 0, strlen(arg2), affected_bits2, FALSE);
                   if(tmp1 != -1) {
                     if(tmp==1)
                       obj_proto_table[object].obj_flags.bitvector2 &= ~(1<<(tmp1-1));
                     else
                       obj_proto_table[object].obj_flags.bitvector2 |=  1<<(tmp1-1);
                   }
                   else {
                     found=0;
                   }
                 }
               arg2 = strtok(NULL, " ");
               }
               if(found) send_to_char("Done.\n\r", ch);
               else send_to_char("Some bitvectors not found.\n\r",ch);
               }
             }
           }
          break;
      case EDIT_OBJ_NAME:
         send_to_char("Enter object name.  End with an @@ on the same line.\n\r",ch);
         if(obj_proto_table[object].name)
           obj_proto_table[object].name[0] = 0;
         ch->desc->str = &obj_proto_table[object].name;
         ch->desc->max_str = 50;
         break;
      case EDIT_OBJ_ACTION_DESC:
         send_to_char("\n\r\
Action Description: Use $n for name/you, $s for his/her/your,\n\r\
$e for he/she/you, $V for victim/you, $m for him/her/you,\n\r\
$r for <name>'s/your.\n\r\
Use @@ on the same line.  Use '@@@' to remove it.\n\r",ch);
         if(obj_proto_table[object].action_description)
           obj_proto_table[object].action_description[0] = 0;
         ch->desc->str = &obj_proto_table[object].action_description;
         ch->desc->max_str = (obj_proto_table[object].obj_flags.type_flag != ITEM_NOTE) ? 120 : 4096;
         break;
      case EDIT_OBJ_ACTION_DESC_NT:
         send_to_char("\n\r\
Action Description No Target: Must also define Action Description.\n\r\
Use $n for name/you, $s for his/her/your,\n\r\
$e for he/she/you, $m for him/her/you, $r for <name>'s/your.\n\r\
Use @@ on same line. Use '@@@' to remove it.\n\r",ch);
         if(obj_proto_table[object].action_description_nt)
           obj_proto_table[object].action_description_nt[0] = 0;
         ch->desc->str = &obj_proto_table[object].action_description_nt;
         ch->desc->max_str = 120;
         break;
      case EDIT_OBJ_CHAR_WEAR_DESC:
         send_to_char("\n\r\
Character Wear Description: (Do room as well) $n for name, $s for his/her\n\r\
$e for he/she, $m for him/her\n\r\
Use @@ on same line.  Use '@@@' to remove it.\n\r",ch);
         if(obj_proto_table[object].char_wear_desc)
           obj_proto_table[object].char_wear_desc[0] = 0;
         ch->desc->str = &obj_proto_table[object].char_wear_desc;
         ch->desc->max_str = 120;
         break;
      case EDIT_OBJ_ROOM_WEAR_DESC:
         send_to_char("\n\r\
Room Wear Description: (Do char as well) $n for name, $s for his/her\n\r\
$e for he/she, $m for him/her\n\r\
Use @@ on same line.  Use '@@@' to remove it.\n\r",ch);
         if(obj_proto_table[object].room_wear_desc)
           obj_proto_table[object].room_wear_desc[0] = 0;
         ch->desc->str = &obj_proto_table[object].room_wear_desc;
         ch->desc->max_str = 120;
         break;
      case EDIT_OBJ_CHAR_REM_DESC:
         send_to_char("\n\r\
Character Remove Description: (Do room as well) $n for name, $s for his/her\n\r\
$e for he/she, $m for him/her\n\r\
Use @@ on same line.  Use '@@@' to remove it.\n\r",ch);
         if(obj_proto_table[object].char_rem_desc)
           obj_proto_table[object].char_rem_desc[0] = 0;
         ch->desc->str = &obj_proto_table[object].char_rem_desc;
         ch->desc->max_str = 120;
         break;
      case EDIT_OBJ_ROOM_REM_DESC:
         send_to_char("\n\r\
Room Remove Description: (Do char as well) $n for name, $s for his/her\n\r\
$e for he/she, $m for him/her\n\r\
Use @@ on same line.  Use '@@@' to remove it.\n\r",ch);
         if(obj_proto_table[object].room_rem_desc)
           obj_proto_table[object].room_rem_desc[0] = 0;
         ch->desc->str = &obj_proto_table[object].room_rem_desc;
         ch->desc->max_str = 120;
         break;
      case EDIT_OBJ_LONG:
         send_to_char("Enter object long.  End with an @@ on the same line.\n\r",ch);
         if(obj_proto_table[object].description)
           obj_proto_table[object].description[0] = 0;
         ch->desc->str = &obj_proto_table[object].description;
         ch->desc->max_str = 256;
         break;
      case EDIT_OBJ_SHORT:
         send_to_char("Enter object short.  End with an @@ on the same line.\n\r",ch);
         if(obj_proto_table[object].short_description)
           obj_proto_table[object].short_description[0] = 0;
         ch->desc->str = &obj_proto_table[object].short_description;
         ch->desc->max_str = 50;
         break;
      case EDIT_OBJ_TIMER:
        arg2 = strtok(NULL, " ");
        if(arg2)
        {
        tmp = atoi(arg2);

        if(tmp>=0)
          {
          obj_proto_table[object].obj_flags.timer=tmp;
          send_to_char("Done.\n\r", ch);
          }
        else
          send_to_char("Argument must be positive\n\r", ch);
        }
        else
          send_to_char("edit object <#> timer <value>\n\r", ch);
        break;

      case EDIT_OBJ_COST:
        arg2 = strtok(NULL, " ");
        if(arg2)
        {
        tmp = atoi(arg2);
        if(tmp>=0)
          {
          obj_proto_table[object].obj_flags.cost=tmp;
          send_to_char("Done.\n\r", ch);
          }
        else
          send_to_char("Argument must be positive\n\r", ch);
        }
        else
          send_to_char("edit object <#> cost <value>\n\r", ch);
        break;
      case EDIT_OBJ_WEIGHT:
        arg2 = strtok(NULL, " ");
        if(arg2)
        {
        tmp = atoi(arg2);
        if(tmp>=0)
          {
          obj_proto_table[object].obj_flags.weight=tmp;
          send_to_char("Done.\n\r", ch);
          }
        else
          send_to_char("Argument must be positive\n\r", ch);
        }
        else
          send_to_char("edit object <#> weight <value>\n\r", ch);
        break;
      case EDIT_OBJ_RENT_COST:
        arg2 = strtok(NULL, " ");
        if(arg2)
        {
        tmp = atoi(arg2);
        if(tmp>=0)
          {
          obj_proto_table[object].obj_flags.cost_per_day=tmp;
          send_to_char("Done.\n\r", ch);
          }
        else
          send_to_char("Argument must be positive\n\r", ch);
        }
        else
          send_to_char("edit object <#> rent <value>\n\r", ch);
        break;
       case EDIT_OBJ_REPOP:
        arg2 = strtok(NULL, " ");
        if(arg2)
        {
        tmp = atoi(arg2);
        if(tmp>=0)
          {
          obj_proto_table[object].obj_flags.repop_percent=tmp;
          send_to_char("Done.\n\r", ch);
          }
        else
          send_to_char("Argument must be positive\n\r", ch);
        }
        else
          send_to_char("edit object <#> repop <value>\n\r", ch);
        break;
      case EDIT_OBJ_RESET:
        obj_proto_table[object].name               = str_dup("nothing");
        obj_proto_table[object].short_description  = str_dup("a bit of nothing");
        obj_proto_table[object].description        = str_dup("There is a bit of nothing sitting here.");
        obj_proto_table[object].action_description = NULL;
        obj_proto_table[object].action_description_nt = NULL;
        obj_proto_table[object].char_wear_desc     = NULL;
        obj_proto_table[object].room_wear_desc     = NULL;
        obj_proto_table[object].char_rem_desc      = NULL;
        obj_proto_table[object].room_rem_desc      = NULL;

        /* *** numeric data *** */

        obj_proto_table[object].obj_flags.type_flag = 0;
        obj_proto_table[object].obj_flags.extra_flags = 0;
        obj_proto_table[object].obj_flags.extra_flags2 = 0;
        obj_proto_table[object].obj_flags.subclass_res = 0;
        obj_proto_table[object].obj_flags.material = 0;
        obj_proto_table[object].obj_flags.wear_flags = ITEM_TAKE;
        obj_proto_table[object].obj_flags.value[0] = 0;
        obj_proto_table[object].obj_flags.value[1] = 0;
        obj_proto_table[object].obj_flags.value[2] = 0;
        obj_proto_table[object].obj_flags.value[3] = 0;
        obj_proto_table[object].obj_flags.weight = 10;
        obj_proto_table[object].obj_flags.cost = 10;
        obj_proto_table[object].obj_flags.cost_per_day = 10;
        obj_proto_table[object].obj_flags.repop_percent = 10;
        obj_proto_table[object].obj_flags.timer = 0;
        obj_proto_table[object].obj_flags.bitvector = 0;
        obj_proto_table[object].obj_flags.bitvector2 = 0;
        obj_proto_table[object].ex_description = 0;
        for (loc = 0;(loc < MAX_OBJ_AFFECT); loc++) {
          obj_proto_table[object].affected[loc].location = APPLY_NONE;
          obj_proto_table[object].affected[loc].modifier = 0;
        }

        send_to_char("Done.\n\r",ch);
        break;
      default:
        send_to_char("Object editables include:\r\n\
wear, flags, weight, cost, rentcost, extradescription, action,\r\n\
action_nt,affects, type, name, timer, repop, bitvect, short, long,\n\r\
reset, values, subclass_res, material, char_wear_desc, room_wear_desc,\n\r\
char_rem_desc, room_rem_desc.\r\n", ch);
        break;
      }
    }
    else
      send_to_char("You cannot edit an object proto that has an instance loaded.\n\r",ch);
    }
}
#define EDIT_ZONE_COMMAND 1
#define EDIT_ZONE_COMMAND_DELETE 10 /*cmddel*/
#define EDIT_ZONE_COMMAND_ADD    11 /*mput, mfollow, mride*/
#define EDIT_ZONE_COMMAND_CHANGE 12 /*no update*/
#define EDIT_ZONE_RANGE   4 /*zrange*/
#define EDIT_ZONE_MODE    5 /*zmode*/
#define EDIT_ZONE_NAME    6 /*zname*/
#define EDIT_ZONE_LIFE    7 /*zlife*/
#define EDIT_ZONE_RESET   13 /*zreset*/
#define EDIT_ZONE_RESET_FULL    14
#define EDIT_ZONE_RESET_NORMAL  15
#define EDIT_ZONE_RESET_GENERATE 16 /*no update*/
#define EDIT_ZONE_CREATORS 17

void edit_zone(CHAR *ch, char *fargs)
{
  char args[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  struct reset_com tmp_reset;
  char *arg2;
  int edit = 0, delete = 1;
  int top, bottom, i,i2;
  int v_zone=-1,zone, iArg, iMaxCommands=0;
  void generate_zone_commands(int r_zone);
  void clean_zone(int r_zone);
  int add_command(int r_zone, struct reset_com *cmd, int where);

  if(!check_olc_access(ch)) return;

  sprintf(args, "%s", fargs);
  arg2 = strtok(args, " ");
  if(arg2)
    v_zone = atoi(arg2);
  zone = real_zone(v_zone);
  if(!check_zone(ch,v_zone)) return;
  if(!check_zone_access(ch,v_zone)) return;

  if(zone >= 0 && zone <= top_of_zone_table)
    {
    arg2 = strtok(NULL, " ");
    edit = -1;
    if(arg2)
    {
    if(is_abbrev(arg2, "command"))
      edit = EDIT_ZONE_COMMAND;
    if(is_abbrev(arg2, "range"))
      edit = EDIT_ZONE_RANGE;
    if(is_abbrev(arg2, "mode"))
      edit = EDIT_ZONE_MODE;
    if(is_abbrev(arg2, "name"))
      edit = EDIT_ZONE_NAME;
    if(is_abbrev(arg2, "creators"))
      edit = EDIT_ZONE_CREATORS;
    if(is_abbrev(arg2, "life"))
      edit = EDIT_ZONE_LIFE;
    if(is_abbrev(arg2, "reset"))
      edit = EDIT_ZONE_RESET;
    }
    while (zone_table[zone].cmd[iMaxCommands].command != 'S')
       iMaxCommands++;

    switch(edit)
      {
      case EDIT_ZONE_RESET:
         edit = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
           {
           if(is_abbrev(arg2, "normal"))
             edit = EDIT_ZONE_RESET_NORMAL;
           if(is_abbrev(arg2, "full"))
             edit = EDIT_ZONE_RESET_FULL;
           if(is_abbrev(arg2, "generate"))
             edit = EDIT_ZONE_RESET_GENERATE;
           }
         switch(edit)
           {
           case EDIT_ZONE_RESET_NORMAL:
             reset_zone(zone, FALSE);
             send_to_char("Done.\n\r", ch);
             break;
           case EDIT_ZONE_RESET_FULL:
             clean_zone(zone);
             reset_zone(zone, TRUE);
             send_to_char("Done.\n\r", ch);
             break;
           case EDIT_ZONE_RESET_GENERATE:
             generate_zone_commands(zone);
             send_to_char("Done.\n\r", ch);
             break;
           }
         break;
      case EDIT_ZONE_COMMAND:
         edit = -1;
         arg2 = strtok(NULL, " ");
         if(arg2)
         {
         if(is_abbrev(arg2, "delete"))
           edit = EDIT_ZONE_COMMAND_DELETE;
         if(is_abbrev(arg2, "add"))
           edit = EDIT_ZONE_COMMAND_ADD;
         if(is_abbrev(arg2, "change"))
           edit = EDIT_ZONE_COMMAND_CHANGE;
         }
         switch (edit)
           {
           case EDIT_ZONE_COMMAND_DELETE:
             delete = 1;
             arg2 = strtok(NULL, " ");
             iArg = -1;
             if(arg2)
               iArg = atoi(arg2);
             if (iArg >= 0 && iArg < iMaxCommands)
                {/*determine its deleteability */
                if(delete)
                  {
                  memmove(&zone_table[zone].cmd[iArg], &zone_table[zone].cmd[iArg+1],
                            sizeof(struct reset_com) * (iMaxCommands - iArg));
                  iMaxCommands --;
                  zone_table[zone].cmd = (struct reset_com*)realloc(zone_table[zone].cmd, sizeof(struct reset_com) * (iMaxCommands+1));
                  send_to_char("Done.\n\r", ch);
                  }
                }
             else
                {
                send_to_char("That reset command does not exist.\n\r", ch);
                }
             break;
           case EDIT_ZONE_COMMAND_ADD:
             arg2 = strtok(NULL, " ");
             iArg = -1;
             if(arg2)
               iArg = atoi(arg2);/*where to add it*/
             /*parse the added command, make sure its ok */
             memset(&tmp_reset, 0, sizeof(struct reset_com));
             arg2 = strtok(NULL, " ");
             if(arg2)
                tmp_reset.command = arg2[0];
             arg2 = strtok(NULL, " ");
             if(arg2)
               tmp_reset.if_flag = atoi(arg2);
             arg2 = strtok(NULL, " ");
             if(arg2)
               tmp_reset.arg1    = atoi(arg2);
             arg2 = strtok(NULL, " ");
             if(arg2)
               tmp_reset.arg2    = atoi(arg2);
             arg2              = strtok(NULL, " ");
             if(arg2) {
	       if(isdigit(arg2[0])) {
		 tmp_reset.arg3 = atoi(arg2);
	       } else {
		 tmp_reset.arg3 = -1;
	       }
	     }

             /* atm, the max # for objects isn't used - set to zero as default
                - Ranger April 98 */
             if((tmp_reset.command == 'G')||(tmp_reset.command == 'E') ||
                (tmp_reset.command == 'P')||(tmp_reset.command == 'O')) tmp_reset.arg2=0;
             if(tmp_reset.command == 'G') tmp_reset.arg3=0;

             /* Door check added by Ranger - May 96 */
             if((tmp_reset.command == 'D') && tmp_reset.if_flag &&
                tmp_reset.arg1 && tmp_reset.arg2 && tmp_reset.arg3) {
               i=real_room(tmp_reset.arg1);
               if (i==-1) {
                 sprintf(buf,"Room %d doesn't exist.\n\r",tmp_reset.arg1);
                 send_to_char(buf,ch);
                 return;
               }
               if(inzone(tmp_reset.arg1) != v_zone) {
                 sprintf(buf,"Room %d is not in zone %d.\n\r",tmp_reset.arg1,v_zone);
                 send_to_char(buf,ch);
                 return;
               }
               if(!world[i].dir_option[tmp_reset.arg2]) {
                 sprintf(buf,"Direction %s is not defined for room %d.\n\r",
                         dirs[tmp_reset.arg2],tmp_reset.arg1);
                 send_to_char(buf,ch);
                 return;
               }

               i2=world[i].dir_option[tmp_reset.arg2]->to_room_v;
               if (i2==0) {
                 sprintf(buf,"The room %s from %d is not defined.\n\r",
                         dirs[tmp_reset.arg2],tmp_reset.arg1);
                 send_to_char(buf,ch);
                 return;
               }
               /* I guess D command is okay then */
             }

             if(tmp_reset.command == 'M' || tmp_reset.command == 'O' || tmp_reset.command == 'T' ||
                tmp_reset.command == 'G' || tmp_reset.command == 'P' ||
                tmp_reset.command == 'E' || tmp_reset.command == 'D' ||
                tmp_reset.command == 'F' || tmp_reset.command == 'R' )
               {
               if((tmp_reset.command =='E' &&( tmp_reset.arg3 >= 0 && tmp_reset.arg3 <= 17)) || tmp_reset.command != 'E')
                 {
                 if(!add_command(zone, &tmp_reset, iArg)) {
                   send_to_char("That index is out of range.\n\r", ch);
                 }
                 else {
                   if(tmp_reset.command == 'M' || tmp_reset.command == 'F' || tmp_reset.command == 'R' )
                     fix_mob_resets(tmp_reset.arg1,zone,tmp_reset.arg2);
                   send_to_char("Done.\n\r", ch);
                 }
               }
               else
                 send_to_char("Unknown equip position, must be: \r\n\
edit zone <#> command add <#> E 1         <obj #> <max allowed> <pos(0-17)>\r\n\
where\r\n\
0) Special(torches)   5) Body     10) arms          15)  left wrist \r\n\
1) Right finger       6) Head     11) shield        16)  wield\r\n\
2) Left finger        7) Legs     12) about body    17)  held\r\n\
3) Neck position A    8) Feet     13) about waist \r\n\
4) Neck position B    9) Hands    14) right wrist\r\n\
", ch);
             }
             else
               send_to_char("Unknown reset command, must be:\r\n\
edit zone <#> command add <#> M <if_flag> <mob #> <max allowed> <room #>\r\n\
edit zone <#> command add <#> F <if_flag> <mob #> <max allowed> <room #>\r\n\
edit zone <#> command add <#> R <if_flag> <mob #> <max allowed> <room #>\r\n\
edit zone <#> command add <#> E 1         <obj #> 0             <pos(0-17)>\r\n\
edit zone <#> command add <#> G 1         <obj #> 0\r\n\
edit zone <#> command add <#> O <if_flag> <obj #> 0             <room #>\r\n\
edit zone <#> command add <#> T <if_flag> <obj #> <max>         <room #>\r\n\
edit zone <#> command add <#> P 1         <obj #> 0             <into obj#>\r\n\
edit zone <#> command add <#> D <if_flag> <room#> <direction  > <state(0-2)>\r\n", ch);
           break;
           case EDIT_ZONE_COMMAND_CHANGE:
             arg2 = strtok(NULL, " ");
             iArg = -1;
             if(arg2)
               iArg = atoi(arg2);/*where to add it*/
             /*parse the added command, make sure its ok */
             memset(&tmp_reset, 0, sizeof(struct reset_com));
             arg2 = strtok(NULL, " ");
             if(arg2)
                tmp_reset.command = arg2[0];
             arg2 = strtok(NULL, " ");
             if(arg2)
               tmp_reset.if_flag = atoi(arg2);
             arg2 = strtok(NULL, " ");
             if(arg2)
               tmp_reset.arg1    = atoi(arg2);
             arg2 = strtok(NULL, " ");
             if(arg2)
               tmp_reset.arg2    = atoi(arg2);
             arg2              = strtok(NULL, " ");
             if(arg2) {
	       if(isdigit(arg2[0])) {
		 tmp_reset.arg3 = atoi(arg2);
	       } else {
		 tmp_reset.arg3 = -1;
	       }
	     }

             /* atm, the max # for objects isn't used - set to zero as default
                - Ranger April 98 */
             if((tmp_reset.command == 'G')||(tmp_reset.command == 'E') ||
                (tmp_reset.command == 'P')||(tmp_reset.command == 'O')) tmp_reset.arg2=0;

             /* Door check added by Ranger - May 96 */
             if((tmp_reset.command == 'D') && tmp_reset.if_flag &&
                tmp_reset.arg1 && tmp_reset.arg2 && tmp_reset.arg3) {
               i=real_room(tmp_reset.arg1);
               if (i==-1) {
                 sprintf(buf,"Room %d doesn't exist.\n\r",tmp_reset.arg1);
                 send_to_char(buf,ch);
                 return;
               }
               if(inzone(tmp_reset.arg1) != v_zone) {
                 sprintf(buf,"Room %d is not in zone %d.\n\r",tmp_reset.arg1,v_zone);
                 send_to_char(buf,ch);
                 return;
               }
               if(!world[i].dir_option[tmp_reset.arg2]) {
                 sprintf(buf,"Direction %s is not defined for room %d.\n\r",
                         dirs[tmp_reset.arg2],tmp_reset.arg1);
                 send_to_char(buf,ch);
                 return;
               }

               i2=world[i].dir_option[tmp_reset.arg2]->to_room_v;
               if (i2==0) {
                 sprintf(buf,"The room %s from %d is not defined.\n\r",
                         dirs[tmp_reset.arg2],tmp_reset.arg1);
                 send_to_char(buf,ch);
                 return;
               }
               /* I guess D command is okay then */
             }

             if(tmp_reset.command == 'M' || tmp_reset.command == 'O' || tmp_reset.command == 'T' ||
                tmp_reset.command == 'G' || tmp_reset.command == 'P' ||
                tmp_reset.command == 'E' || tmp_reset.command == 'D' ||
                tmp_reset.command == 'F' || tmp_reset.command == 'R' )
               {
               if((tmp_reset.command =='E' &&( tmp_reset.arg3 >= 0 && tmp_reset.arg3 <= 17)) || tmp_reset.command != 'E')
                 {
                 if(iArg >=0 && iArg <= iMaxCommands)
                   {
                   zone_table[zone].cmd[iArg] = tmp_reset;
                   if(tmp_reset.command == 'M' || tmp_reset.command == 'F' || tmp_reset.command == 'R' )
                     fix_mob_resets(tmp_reset.arg1,zone,tmp_reset.arg2);
                   send_to_char("Done.\n\r", ch);
                   }
                 else
                   send_to_char("That index is out of range.\n\r", ch);
                 }
               else
                   send_to_char("Unknown equip position, must be:\r\n\
edit zone <#> command change <#> E 1         <obj #> <max allowed> <pos(0-17)>\r\n\
where\r\n\
0) Special(torches)   5) Body     10) arms          15)  left wrist \r\n\
1) Right finger       6) Head     11) shield        16)  wield\r\n\
2) Left finger        7) Legs     12) about body    17)  held\r\n\
3) Neck position A    8) Feet     13) about waist \r\n\
4) Neck position B    9) Hands    14) right wrist\r\n\
", ch);

               }
             else
               send_to_char("Unknown reset command, must be:\r\n\
edit zone <#> command change <#> M <if_flag> <mob #> <max allowed> <room #>\r\n\
edit zone <#> command change <#> F <if_flag> <mob #> <max allowed> <room #>\r\n\
edit zone <#> command change <#> R <if_flag> <mob #> <max allowed> <room #>\r\n\
edit zone <#> command change <#> E 1         <obj #> 0             <pos(0-17)>\r\n\
edit zone <#> command change <#> G 1         <obj #> 0\r\n\
edit zone <#> command change <#> O <if_flag> <obj #> 0             <room #>\r\n\
edit zone <#> command change <#> T <if_flag> <obj #> <max>         <room #>\r\n\
edit zone <#> command change <#> P 1         <obj #> 0             <into obj#>\r\n\
edit zone <#> command change <#> D <if_flag> <room#> <direction  > <state(0-2)>\r\n", ch);
             break;
           default:
             send_to_char("edit zone <#> command <add|change|delete> [arguments]\r\n", ch);
             break;
           }
         break;
      case EDIT_ZONE_LIFE:
         arg2 = strtok(NULL, " ");
         if(arg2)
           zone_table[zone].lifespan = atoi(arg2);
           send_to_char("Done.\n\r", ch);
         break;
      case EDIT_ZONE_MODE:
         arg2 = strtok(NULL, " ");
         if(arg2)
           if(is_abbrev(arg2, "always")) {
             zone_table[zone].reset_mode = 2;
             send_to_char("Done.\n\r", ch);
           } else if(is_abbrev(arg2, "empty")) {
             zone_table[zone].reset_mode = 1;
             send_to_char("Done.\n\r", ch);
           } else if(is_abbrev(arg2, "never")) {
             zone_table[zone].reset_mode = 0;
             send_to_char("Done.\n\r", ch);
           } else if(is_abbrev(arg2, "block")) {
             zone_table[zone].reset_mode = 3;
             send_to_char("Done.\n\r", ch);
           } else if(is_abbrev(arg2, "lock")) {
             zone_table[zone].reset_mode = 4;
             send_to_char("Done.\n\r", ch);
           } else if(is_abbrev(arg2, "doors")) {
             zone_table[zone].reset_mode = 5;
             send_to_char("Done.\n\r", ch);
           } else
             send_to_char("edit zone <#> mode <always|empty|never|block|lock|doors>\r\n", ch);
         else
             send_to_char("edit zone <#> mode <always|empty|never|block|lock|doors>\r\n", ch);
         break;
      case EDIT_ZONE_NAME:
         if(GET_LEVEL(ch)<LEVEL_IMP) {
           send_to_char("Please request a name change from an IMP.\n\r",ch);
           return;
         }
         arg2 = strtok(NULL, "\0");
         if(arg2) {
           if(zone_table[zone].name)
              free(zone_table[zone].name);
           zone_table[zone].name = str_dup(arg2);
           send_to_char("Done.\n\r", ch);
         }
         else {
           send_to_char("What's the new name?\n\r",ch);
         }
         break;
      case EDIT_ZONE_CREATORS:
         arg2 = strtok(NULL, " ");
         if(arg2) {
           if(zone_table[zone].creators)
              free(zone_table[zone].creators);
           zone_table[zone].creators = str_dup(arg2);
           send_to_char("Done.\n\r", ch);
         }
         else {
           send_to_char("Who are the new creators? (No spaces between names)\n\r",ch);
         }
         break;
      case EDIT_ZONE_RANGE:
         arg2=strtok(NULL, " -");
         if(arg2) {
           if(is_number(arg2)) bottom=atoi(arg2);
           else bottom=0;
         }
         else bottom=0;
         arg2=strtok(NULL, " ");
         if(arg2) {
           if(is_number(arg2)) top=atoi(arg2);
           else top=0;
         }
         else top=0;

         if(bottom==0) bottom=v_zone*100;
         if(top==0) top=v_zone*100+99;

         if((real_room(top) != -1) && (real_room(bottom) != -1))
             {
             zone_table[zone].bottom = bottom;
             zone_table[zone].top    = top;
             top = real_room(top);
             for(i=real_room(bottom);i<=top;i++)
               world[i].zone = v_zone;
             send_to_char("Done.\n\r", ch);
             }
         else send_to_char("edit zone <#> range <bottom>-<top>.\n\r",ch);
         break;
       default:
         send_to_char("edit zone <#> <command|life|mode|name|creators|range|reset> [arguments]\n\r",ch);
         break;
       }
    }
}

#define EDIT_SHOP_PRODUCING    1
#define EDIT_SHOP_PROFIT_BUY   2
#define EDIT_SHOP_PROFIT_SELL  3
#define EDIT_SHOP_TRADE_TYPE   4
#define EDIT_SHOP_MESSAGE      5
#define EDIT_SHOP_TEMPER1      6
#define EDIT_SHOP_TEMPER2      7
#define EDIT_SHOP_WITH_WHO     8
#define EDIT_SHOP_IN_ROOM      9
#define EDIT_SHOP_OPEN1       10
#define EDIT_SHOP_CLOSE1      11
#define EDIT_SHOP_OPEN2       12
#define EDIT_SHOP_CLOSE2      13
#define EDIT_SHOP_RESET       14

void edit_shop(CHAR *ch, char *fargs)
{
  char args[MAX_STRING_LENGTH];
  int i,edit,vshop = -1,rshop=-1,option,zone,tmp;
  char *arg2;

  sprintf(args, "%s", fargs);
  if(*args)
    {
    arg2 = strtok(args, " ");
    if(arg2)
       vshop = atoi(arg2);
    }
  zone = inzone(vshop);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  for(i=0 ; i<number_of_shops ; i++) {
    if(shop_index[i].keeper==vshop) {
      rshop=i;
      break;
    }
  }

  if(rshop<0 || rshop > number_of_shops) {
    printf_to_char(ch,"Shop %d does not exist.\n\r",vshop);
    return;
  }

  arg2 = strtok(NULL, " ");
  edit = -1;
  if(arg2) {
    if(is_abbrev(arg2, "producing"))
      edit = EDIT_SHOP_PRODUCING;
    if(is_abbrev(arg2, "profit_buy"))
      edit = EDIT_SHOP_PROFIT_BUY;
    if(is_abbrev(arg2, "profit_sell"))
      edit = EDIT_SHOP_PROFIT_SELL;
    if(is_abbrev(arg2, "trade_type"))
      edit = EDIT_SHOP_TRADE_TYPE;
    if(is_abbrev(arg2, "message"))
      edit = EDIT_SHOP_MESSAGE;
    if(is_abbrev(arg2, "temper1"))
      edit = EDIT_SHOP_TEMPER1;
    if(is_abbrev(arg2, "temper2"))
      edit = EDIT_SHOP_TEMPER2;
    if(is_abbrev(arg2, "with_who"))
      edit = EDIT_SHOP_WITH_WHO;
    if(is_abbrev(arg2, "room"))
      edit = EDIT_SHOP_IN_ROOM;
    if(is_abbrev(arg2, "open1"))
      edit = EDIT_SHOP_OPEN1;
    if(is_abbrev(arg2, "close1"))
      edit = EDIT_SHOP_CLOSE1;
    if(is_abbrev(arg2, "open2"))
      edit = EDIT_SHOP_OPEN2;
    if(is_abbrev(arg2, "close2"))
      edit = EDIT_SHOP_CLOSE2;
    if(is_abbrev(arg2, "reset"))
      edit = EDIT_SHOP_RESET;
  }

  switch(edit)  {
    case EDIT_SHOP_MESSAGE:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        if(is_abbrev(arg2, "noitem1"))
          option = 1;
        if(is_abbrev(arg2, "noitem2"))
          option = 2;
        if(is_abbrev(arg2, "nobuy"))
          option = 3;
        if(is_abbrev(arg2, "nocash1"))
          option = 4;
        if(is_abbrev(arg2, "nocash2"))
          option = 5;
        if(is_abbrev(arg2, "buy"))
          option = 6;
        if(is_abbrev(arg2, "sell"))
          option = 7;
        switch(option) {
          case 1:
            if(shop_index[rshop].no_such_item1) {
              send_to_char("What keeper say if it doesn't have the item, end with @@ on the same line.\n\r",ch);
              shop_index[rshop].no_such_item1[0] = 0;
              ch->desc->str = &shop_index[rshop].no_such_item1;
              ch->desc->max_str = 80;
              return;
            }
            break;
          case 2:
            if(shop_index[rshop].no_such_item2) {
              send_to_char("What keeper says if seller doesn't have the item, end with @@ on the same line.\n\r",ch);
              shop_index[rshop].no_such_item2[0] = 0;
              ch->desc->str = &shop_index[rshop].no_such_item2;
              ch->desc->max_str = 80;
              return;
            }
            break;
          case 3:
            if(shop_index[rshop].do_not_buy) {
              send_to_char("What keeper says if it doesn't buy that kind of item, end with @@ on the same line.\n\r",ch);
              shop_index[rshop].do_not_buy[0] = 0;
              ch->desc->str = &shop_index[rshop].do_not_buy;
              ch->desc->max_str = 80;
              return;
            }
            break;
          case 4:
            if(shop_index[rshop].missing_cash1) {
              send_to_char("What keeper says if it doesn't have cash to buy item, end with @@ on the same line.\n\r",ch);
              shop_index[rshop].missing_cash1[0] = 0;
              ch->desc->str = &shop_index[rshop].missing_cash1;
              ch->desc->max_str = 80;
              return;
            }
            break;
          case 5:
            if(shop_index[rshop].missing_cash2) {
              send_to_char("What keeper says if buyer doesn't have the cash, end with @@ on the same line.\n\r",ch);
              shop_index[rshop].missing_cash2[0] = 0;
              ch->desc->str = &shop_index[rshop].missing_cash2;
              ch->desc->max_str = 80;
              return;
            }
            break;
          case 6:
            if(shop_index[rshop].message_buy ) {
              send_to_char("What keeper says after buyer gets item, end with @@ on the same line.\n\r",ch);
              shop_index[rshop].message_buy[0] = 0;
              ch->desc->str = &shop_index[rshop].message_buy;
              ch->desc->max_str = 80;
              return;
            }
            break;
          case 7:
            if(shop_index[rshop].message_sell) {
              send_to_char("What keeper says after it buys a sellers item, end with @@ on the same line.\n\r",ch);
              shop_index[rshop].message_sell[0] = 0;
              ch->desc->str = &shop_index[rshop].message_sell;
              ch->desc->max_str = 80;
              return;
            }
            break;
         }/*switch*/
      }
      send_to_char("edit shop <#> message <noitem1/noitem2/nobuy/nocash1/nocash2/buy/sell>\n\r",ch);
      return;
      break;
    case EDIT_SHOP_PROFIT_BUY:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        option=atoi(arg2);
        if(option>0) {
           shop_index[rshop].profit_buy = (float)option / (float)100;
           send_to_char("Done.\n\r", ch);
           return;
        }
      }
      send_to_char("edit shop <#> profit_buy <#>*100 - Result should be > 1.0, Example 120 = 1.2\n\r",ch);
      return;
      break;
    case EDIT_SHOP_PROFIT_SELL:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        option=atoi(arg2);
        if(option>0) {
           shop_index[rshop].profit_sell = (float)option / (float)100;
           send_to_char("Done.\n\r", ch);
           return;
        }
      }
      send_to_char("edit shop <#> profit_sell <#>*100 - Result should be <= 1.0, Example 50 = 0.5\n\r",ch);
      return;
      break;
    case EDIT_SHOP_PRODUCING:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        option=atoi(arg2);
        if(option>=1 && option<=5) {
          arg2=strtok(NULL, " ");
          if(arg2) {
            tmp=atoi(arg2);
            shop_index[rshop].producing[option-1]= tmp;
            send_to_char("Done.\n\r",ch);
            return;
          }
        }
      }
      send_to_char("edit shop <#> producing 1/2/3/4/5 <obj_num>.\n\r\
This is the number of the item the shop keeper will have in unlimited supply.\n\r", ch);
      break;
    case EDIT_SHOP_TRADE_TYPE:
      arg2 = strtok(NULL, " ");
      tmp = -1;
      if(arg2) {
        option=atoi(arg2);
        if(option>=1 && option<=5) {
          arg2=strtok(NULL, " ");
          if(arg2) {
            tmp=old_search_block(arg2, 0, strlen(arg2), item_types, FALSE);
            if(tmp!=-1) {
              shop_index[rshop].type[option-1] = tmp-1;
              send_to_char("Done.\n\r",ch);
              return;
            }
            else {
              send_to_char("Item type set to 0.\n\r",ch);
              shop_index[rshop].type[option-1] = 0;
              return;
            }
          }
        }
      }
      send_to_char("edit shop <#> trade_type 1/2/3/4/5 <type> where <type> is one of the following:\n\r\
LIGHT, SCROLL, WAND, STAFF, WEAPON, FIRE WEAPON, MISSILE, TREASURE, ARMOR, \n\r\
POTION, WORN, OTHER, TRASH, TRAP, CONTAINER, NOTE, LIQUID, KEY, FOOD, MONEY,\n\r\
PEN, BOAT, BULLET, MUSICAL, LOCKPICK, 2H-WEAPON, BOARD, SC_TOKEN, SKIN, RECIPE,\n\r\
AQ_ORDER, NONE.\n\r\
Note: This is the type of item the shop keeper will buy.\n\r", ch);
      break;
    case EDIT_SHOP_TEMPER1:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        option=atoi(arg2);
        if(option==0 || option==1) {
           shop_index[rshop].temper1 = option;
           send_to_char("Done.\n\r", ch);
           return;
        }
      }
      send_to_char("edit shop <#> temper1 <0/1> - 0=default action 1=smokes a joint)\n\r",ch);
      return;
      break;
    case EDIT_SHOP_TEMPER2:
      printf_to_char(ch,"This option is currently not supported in shops.\n\r");
      return;
      break;
    case EDIT_SHOP_WITH_WHO:
      printf_to_char(ch,"This option is currently not supported in shops.\n\r");
      return;
      break;
    case EDIT_SHOP_IN_ROOM:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        option=atoi(arg2);
        if(zone==inzone(option)) {
           shop_index[rshop].in_room = option;
           send_to_char("Done.\n\r", ch);
           return;
        }
        else {
          printf_to_char(ch,"The shop room %d should be in zone %d.\n\r",option,zone);
          return;
        }
      }
      send_to_char("edit shop <#> room <#> - Room where keeper has the shop (In same zone).\n\r",ch);
      return;
      break;
    case EDIT_SHOP_OPEN1:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        option=atoi(arg2);
        if(option>=0 && option <=28) {
           shop_index[rshop].open1 = option;
           send_to_char("Done.\n\r", ch);
           return;
        }
      }
      send_to_char("edit shop <#> open1 <#> - 1st hour to open (0-28).\n\r",ch);
      return;
      break;
    case EDIT_SHOP_CLOSE1:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        option=atoi(arg2);
        if(option>=0 && option <=28) {
           shop_index[rshop].close1 = option;
           send_to_char("Done.\n\r", ch);
           return;
        }
      }
      send_to_char("edit shop <#> close1 <#> - 1st hour to close (0-28).\n\r",ch);
      return;
      break;
    case EDIT_SHOP_OPEN2:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        option=atoi(arg2);
        if(option>=0 && option <=28) {
           shop_index[rshop].open2 = option;
           send_to_char("Done.\n\r", ch);
           return;
        }
      }
      send_to_char("edit shop <#> open2 <#> - 2nd hour to open (0-28).\n\r",ch);
      return;
      break;
    case EDIT_SHOP_CLOSE2:
      arg2 = strtok(NULL, " ");
      if(arg2) {
        option=atoi(arg2);
        if(option>=0 && option <=28) {
           shop_index[rshop].close2 = option;
           send_to_char("Done.\n\r", ch);
           return;
        }
      }
      send_to_char("edit shop <#> close2 <#> - 2nd hour to close (0-28).\n\r",ch);
      return;
      break;
    case EDIT_SHOP_RESET:
      for(i=0;i<MAX_PROD;i++) {
        shop_index[rshop].producing[i]= -1;
      }
      shop_index[rshop].profit_buy=1;
      shop_index[rshop].profit_sell=1;
      for(i=0;i<MAX_TRADE;i++) {
        shop_index[rshop].type[i] = 0;
      }
      shop_index[rshop].no_such_item1 = str_dup("%s What keeper says if it doesn't have the item.");
      shop_index[rshop].no_such_item2 = str_dup("%s What keeper says if seller doesn't have the item.");
      shop_index[rshop].do_not_buy    = str_dup("%s What keeper says if it doesn't buy that kind of item.");
      shop_index[rshop].missing_cash1 = str_dup("%s What keeper says if it doesn't have cash to buy an item.");
      shop_index[rshop].missing_cash2 = str_dup("%s What keeper says if buyer doesn't have the cash.");
      shop_index[rshop].message_buy   = str_dup("%s What keeper says after buyer gets item (Can include %d for amount).");
      shop_index[rshop].message_sell  = str_dup("%s What keeper says after it buys a sellers item (Can include %d for amount).");
      shop_index[rshop].temper1=0;
      shop_index[rshop].temper2=0;
      shop_index[rshop].with_who=0;
      shop_index[rshop].in_room=0;
      shop_index[rshop].open1=0;
      shop_index[rshop].close1=28;
      shop_index[rshop].open2=0;
      shop_index[rshop].close2=0;
      printf_to_char(ch,"Shop %d reset.\n\r",vshop);
      return;
      break;
    default:
      send_to_char("Shop editables are:\n\rproducing, profit_buy, profit_sell, trade_type, message, temper1\n\rtemper2, with_who, room, open1, close1, open2, close2, reset\r\n", ch);
      return;
      break;
  }
}

void do_edit(CHAR* ch, char *arg, int cmd) {
  char args[MAX_STRING_LENGTH]="";
#ifndef TEST_SITE
  char buf[MAX_STRING_LENGTH];
#endif
  char *arg1, *arg2;
  int edit=0;

  if(!check_olc_access(ch)) return;

  sprintf(args, "%s", arg);
  arg1 = strtok(args, " ");
  arg2 = strtok(NULL, "\0");

  if(arg1)
  {
#ifndef TEST_SITE
     sprintf(buf,"WIZINFO: %s edit %s",GET_NAME(ch),arg);
     wizlog(buf,LEVEL_IMP,5);
     log_s(buf);
#endif
     if(is_abbrev(arg1, "room"))
       edit = EDIT_ROOM;
     if(is_abbrev(arg1, "mobile"))
       edit = EDIT_MOB;
     if(is_abbrev(arg1, "object"))
       edit = EDIT_OBJ;
     if(is_abbrev(arg1, "zone"))
       edit = EDIT_ZONE;
     if(is_abbrev(arg1, "shop"))
       edit = EDIT_SHOP;
  }

  if (!arg2)
  {
     /* if arg list is null, then there was an error in strtok */
     /* display the usage */
     edit = 0;
  }
  
  switch (edit)
  {
  case EDIT_ROOM:
     edit_room(ch, arg2);
     break;
  case EDIT_MOB:
     edit_mob(ch, arg2);
     break;
  case EDIT_OBJ:
     edit_obj(ch, arg2);
     break;
  case EDIT_ZONE:
     edit_zone(ch, arg2);
     break;
  case EDIT_SHOP:
     edit_shop(ch, arg2);
     break;
  default:
     send_to_char("edit room <#> [more]\r\nedit mobile <#> [more]\r\nedit object <#> [more]\r\nedit zone <#> <delete|change|add> [more]\r\nedit shop <#> [more]\n\r", ch);
     break;
  }

}
int add_command(int r_zone, struct reset_com *cmd, int where)
{
   int Ok;
   int iMaxCommands=0;
   while (zone_table[r_zone].cmd[iMaxCommands].command != 'S')
       iMaxCommands++;
   if(where == -1)
     where = iMaxCommands;
   if(where > iMaxCommands)
     where = iMaxCommands;
   if(where >=0 && where <= iMaxCommands)
     {
     iMaxCommands++;
     zone_table[r_zone].cmd = (struct reset_com*)realloc(zone_table[r_zone].cmd, sizeof(struct reset_com) * (iMaxCommands+1));
     memmove(&zone_table[r_zone].cmd[where+1], &zone_table[r_zone].cmd[where],
     sizeof(struct reset_com) * (iMaxCommands - where));
     zone_table[r_zone].cmd[where] = *cmd;
     Ok = TRUE;
     }
  else
     Ok = FALSE;
  return Ok;
}
void clean_zone(int r_zone)
{
   int r_bottom, r_top;
   int i;
   void clean_room(int r_room);
   if(r_zone > 0 && r_zone <= top_of_zone_table)
     {
     r_bottom = real_room (zone_table[r_zone].bottom);
     r_top    = real_room (zone_table[r_zone].top);
     if(r_bottom != -1 && r_top !=-1)
       {
       for(i=r_bottom; i <= r_top;i++)
         clean_room(i);
       }
     }
}
void clean_room(int r_room)
{
   OBJ *obj, *next_o;
   CHAR *vict, *next_v;
   if(r_room!=-1 && r_room <= top_of_world)
     {
    for (vict = world[r_room].people; vict; vict = next_v)
        {
        next_v = vict->next_in_room;
        if (IS_NPC(vict))
          extract_char(vict);
        }

      for (obj = world[r_room].contents; obj; obj = next_o)
        {
        next_o = obj->next_content;
        extract_obj(obj);
        }
     }
}
void generate_zone_commands(int r_zone)
{
   int r_bottom, r_top;
   int mode;
   int i,x;
   CHAR *tmpMob;
   OBJ *tmpObj;

   void make_door_resets(int r_zone, int v_where, int direction, int mode);
   void make_mob_resets(int r_zone, CHAR *mob);
   void make_obj_resets(int r_zone, OBJ *obj, char mode, int v_where);

   if(r_zone > 0 && r_zone <= top_of_zone_table)
     {
     r_bottom = real_room (zone_table[r_zone].bottom);
     r_top    = real_room (zone_table[r_zone].top);
     if(r_bottom != -1 && r_top !=-1)
       {
       if(zone_table[r_zone].cmd)
         {
         zone_table[r_zone].cmd = realloc(zone_table[r_zone].cmd, sizeof(struct reset_com));
         zone_table[r_zone].cmd[0].command = 'S';
         }
       else
         {
         produce_core();
         }

       for(i=r_bottom; i <= r_top;i++)
         {
         for(tmpMob = world[i].people;tmpMob;tmpMob = tmpMob->next_in_room)
           make_mob_resets(r_zone, tmpMob);
         for(tmpObj = world[i].contents;tmpObj;tmpObj = tmpObj->next_content)
           make_obj_resets(r_zone, tmpObj, 'O', world[i].number);
         for(x=0;x<6;x++)
           {
           if(world[i].dir_option[x])
             {
             if(IS_SET(world[i].dir_option[x]->exit_info, EX_ISDOOR))
               {
               mode = 0;
               if(IS_SET(world[i].dir_option[x]->exit_info, EX_CLOSED))
                 {
                 if(IS_SET(world[i].dir_option[x]->exit_info, EX_LOCKED))
                   mode = 2;
                 else
                   mode = 1;
                 }
               make_door_resets(r_zone,world[i].number, x, mode);
               }
             }
           }
         }
       }
     }
}
void make_door_resets(int r_zone, int v_where, int direction, int mode)
{
   struct reset_com tmp;
   tmp.command = 'D';
   tmp.if_flag = 0;
   tmp.arg1 = v_where;
   tmp.arg2 = direction;
   tmp.arg3 = mode;
   add_command(r_zone, &tmp, -1);
}
void make_mob_resets(int r_zone, CHAR *mob)
{
   struct reset_com tmp;
   OBJ *tmpObj;
   int i;
   void make_obj_resets(int r_zone, OBJ *obj, char mode, int v_where);
   if(mob)
     {
     if(IS_NPC(mob) )
       {
       tmp.command = 'M';
       tmp.if_flag = 0;
       tmp.arg1 = V_MOB(mob);
       tmp.arg2 = 99;
       tmp.arg3 = CHAR_VIRTUAL_ROOM(mob);
       add_command(r_zone, &tmp, -1);
       for(i=0;i<=HOLD;i++)
         {
         if(mob->equipment[i])
           make_obj_resets(r_zone, mob->equipment[i], 'E', i);
         }
       for(tmpObj = mob->carrying;tmpObj;tmpObj = tmpObj->next_content)
         {
         make_obj_resets(r_zone, tmpObj, 'G', 0);
         }
       }
     }
}
void make_obj_resets(int r_zone, OBJ *obj, char mode, int v_where)
{
   struct reset_com tmp;
   if(obj)
     {
     if(V_OBJ(obj)!= 0)
       {
       tmp.command = mode;
       if(mode == 'O')
         tmp.if_flag = 0;
       else
         tmp.if_flag = 1;
       tmp.arg1 = V_OBJ(obj);
       tmp.arg2 = 99;
       tmp.arg3 = v_where;
       add_command(r_zone, &tmp, -1);
       if(obj->contains)
         make_obj_resets(r_zone, obj->contains, 'P', 0);
       }
     }
}

/* Append text to a room descript - Ranger Jan 99 */
void append_text_room(int num, char *text, int type) {
  int room;
  char newtext[MAX_STRING_LENGTH];

  if(type) room=real_room(num);
  else room=num;
  if(room==NOWHERE) return;
  if(!*text) return;

  if((strlen(world[room].description)+strlen(text))>MSL) return;
  sprintf(newtext,"%s%s\n\r",world[room].description,text);
  world[room].description = str_dup(newtext);
}

/* Procedure rclone written for Ronin by Ranger
**
** Allows a room to be cloned for online creation purposes.
**
** May 9, 1996
**
** Further modifed May 20, 1997 - Ranger
**    - fixed up the type casts
**    - fixed up extra descrip cloning - lowered total clone to WIZ+
*/
void do_rclone(CHAR *ch, char *argument, int cmd) {
  int number1,number2;
  int index1,index2;
  int i,zone1,zone2;
  char usage[]="Usage: rclone <room #1> <room #2>  (Room 1 exists)\n\rIf the command crashes, try a different # for room 2.\n\r";
  char txt[100],num1[100],num2[100];
  char temptext[MAX_STRING_LENGTH];
  struct extra_descr_data *desc,*tmp_desc;
  FILE *fl;

  if(!check_olc_access(ch)) return;

  argument_interpreter(argument,num1,num2);
  if(!*num2) {
    send_to_char(usage,ch);
    return;
  }

  for (i = 0; i < strlen(num1); i++) {
    if (!isdigit(*num1)) {
      send_to_char("Room value is not a valid number.\n\r", ch);
      send_to_char(usage,ch);
      return;
    }
  }
  for (i = 0; i < strlen(num2); i++) {
    if (!isdigit(*num2)) {
      send_to_char("Room value is not a valid number.\n\r", ch);
      send_to_char(usage,ch);
      return;
    }
  }

  number1 = atoi(num1);
  number2 = atoi(num2);

  zone1 = inzone(number1);
  zone2 = inzone(number2);
  if(!check_zone(ch,zone1)) return;
  if(!check_zone(ch,zone2)) return;
  if(!check_zone_access(ch,zone2)) return;

  index1 = real_room(number1);
  if(index1 == -1) {
    sprintf(txt,"Room %d doesn't exist.\n\r",number1);
    send_to_char(txt,ch);
    return;
  }
  index2 = real_room(number2);
  if(index2 == -1) {
    index2 = allocate_room(number2);
    index1 = real_room(number1);
    world[index2].number      = number2;
    if(GET_LEVEL(ch)>LEVEL_DEI) {
      sprintf(temptext,"%s",world[index1].name);
      world[index2].name           = str_dup(temptext);
      sprintf(temptext,"%s",world[index1].description);
      world[index2].description    = str_dup(temptext);
      world[index2].ex_description = NULL;
      if(world[index1].ex_description) {
        for (desc = world[index1].ex_description; desc; desc = desc->next) {
          CREATE(tmp_desc, struct extra_descr_data, 1);
          sprintf(temptext,"%s",desc->keyword);
          tmp_desc->keyword     = str_dup(temptext);
          sprintf(temptext,"%s",desc->description);
          tmp_desc->description = str_dup(temptext);
          tmp_desc->next = world[index2].ex_description;
          world[index2].ex_description = tmp_desc;
        }
      }
    }
    else {
      world[index2].name           = str_dup("A temporary name");
      world[index2].description    = str_dup("A temporary description\n");
      world[index2].ex_description = NULL;
    }
    world[index2].room_flags  = (int)world[index1].room_flags;
    world[index2].sector_type = (int)world[index1].sector_type;
    world[index2].funct       = NULL;
    world[index2].contents    = NULL;
    world[index2].people      = NULL;
    world[index2].light       = (int)world[index1].light;
    world[index2].zone        = zone2;

    for(i=0;i<6;i++) world[index2].dir_option[i] = NULL;

    /* Renumber the world to match the new real indeces */
    renum_world();
    fl = fopen("test.wld","w");
    write_room(fl, index2);
    fclose(fl);
  }
  else {
    sprintf(txt,"Room %d already exists.\n\r",number2);
    send_to_char(txt, ch);
    return;
  }
  send_to_char("Done\n\r",ch);
}

/* Procedure mclone written for Ronin by Ranger
**
** Allows a mob to be cloned for online creation purposes.
**
** May 9, 1996
**   - last mod May 20, 97
**       - fixed up the type casts - Ranger
*/
void do_mclone(CHAR *ch, char *argument, int cmd) {
  int number1,number2;
  struct tagline_data *tag,*tmp_tag;
  int i1,i2;
  int i,zone1,zone2;
  char usage[]="Usage: mclone <mob #1> <mob #2>  (Mobile 1 exists)\n\rIf command crashes, try a different # for mob 2\n\r";
  char txt[100],num1[100],num2[100];
  char temptext[MAX_STRING_LENGTH];
  FILE *fl;

  if(!check_olc_access(ch)) return;

  argument_interpreter(argument,num1,num2);

  if(!*num2) {
    send_to_char(usage,ch);
    return;
  }

  for (i = 0; i < strlen(num1); i++) {
    if (!isdigit(*num1)) {
      send_to_char("Mobile value is not a valid number.\n\r", ch);
      send_to_char(usage,ch);
      return;
    }
  }
  for (i = 0; i < strlen(num2); i++) {
    if (!isdigit(*num2)) {
      send_to_char("Mobile value is not a valid number.\n\r", ch);
      send_to_char(usage,ch);
      return;
    }
  }

  number1 = atoi(num1);
  number2 = atoi(num2);

  zone1 = inzone(number1);
  zone2 = inzone(number2);
  if(!check_zone(ch,zone1)) return;
  if(!check_zone(ch,zone2)) return;
  if(!check_zone_access(ch,zone2)) return;

  i1 = real_mobile(number1);
  if(i1 == -1) {
    sprintf(txt,"Mobile %d doesn't exist.\n\r",number1);
    send_to_char(txt,ch);
    return;
  }
  i2 = real_mobile(number2);
  if(i2 == -1) {
    log_cmd("mclone","mclone %d %d by %s",number1,number2,GET_NAME(ch));
    log_cmd("mclone","  allocating mob %d",number2);
    i2 = allocate_mob(number2);

    /* the new mob virtual number must be set, or the mob table search will fail (binary search is performed on 'virtual' member) */
    log_cmd("mclone","  assigning number");
    mob_proto_table[i2].virtual     = number2;

    /* re-read source mob real number (since table will change from allocate_mob) */
    i1 = real_mobile(number1);

    if (i1 == -1) {
      log_s("Error in mclone, mob table corrupt.");
      produce_core();
      return;
    }

    sprintf(temptext,"%s",mob_proto_table[i1].name);
    log_cmd("mclone","  assigning name");
    mob_proto_table[i2].name        = str_dup(temptext);
    sprintf(temptext,"%s",mob_proto_table[i1].short_descr);
    log_cmd("mclone","  assigning short description");
    mob_proto_table[i2].short_descr = str_dup(temptext);
    sprintf(temptext,"%s",mob_proto_table[i1].long_descr);
    log_cmd("mclone","  assigning long description");
    mob_proto_table[i2].long_descr  = str_dup(temptext);
    log_cmd("mclone","  long descript assigned");
    sprintf(temptext,"%s",mob_proto_table[i1].description);
    log_cmd("mclone","  assigning description");
    mob_proto_table[i2].description = str_dup(temptext);

    mob_proto_table[i2].func = 0;
    mob_proto_table[i2].number = 0;

    log_cmd("mclone","  assigning acts, affects, immunes, resists");
    mob_proto_table[i2].act         = (unsigned long)mob_proto_table[i1].act;
    mob_proto_table[i2].act2        = (unsigned long)mob_proto_table[i1].act2;
    mob_proto_table[i2].affected_by = (unsigned long)mob_proto_table[i1].affected_by;
    mob_proto_table[i2].affected_by2= (unsigned long)mob_proto_table[i1].affected_by2;
    mob_proto_table[i2].immune      = (unsigned long)mob_proto_table[i1].immune;
    mob_proto_table[i2].immune2     = (unsigned long)mob_proto_table[i1].immune2;
    mob_proto_table[i2].resist      = (unsigned long)mob_proto_table[i1].resist;
    log_cmd("mclone","  assigning all ints up to mana_add");
    mob_proto_table[i2].alignment   = (int)mob_proto_table[i1].alignment;
    mob_proto_table[i2].level       = (int)mob_proto_table[i1].level;
    mob_proto_table[i2].hitroll     = (int)mob_proto_table[i1].hitroll;
    mob_proto_table[i2].armor       = (int)mob_proto_table[i1].armor;
    mob_proto_table[i2].hp_nodice   = (int)mob_proto_table[i1].hp_nodice;
    mob_proto_table[i2].hp_sizedice = (int)mob_proto_table[i1].hp_sizedice;
    mob_proto_table[i2].hp_add      = (int)mob_proto_table[i1].hp_add;
    mob_proto_table[i2].damnodice   = (int)mob_proto_table[i1].damnodice;
    mob_proto_table[i2].damsizedice = (int)mob_proto_table[i1].damsizedice;
    mob_proto_table[i2].damroll     = (int)mob_proto_table[i1].damroll;
    mob_proto_table[i2].gold        = (int)mob_proto_table[i1].gold;
    mob_proto_table[i2].exp         = (int)mob_proto_table[i1].exp;
    mob_proto_table[i2].position    = (int)mob_proto_table[i1].position;
    mob_proto_table[i2].default_pos = (int)mob_proto_table[i1].default_pos;
    mob_proto_table[i2].sex         = (int)mob_proto_table[i1].sex;
    mob_proto_table[i2].class       = (int)mob_proto_table[i1].class;
    mob_proto_table[i2].mana_nodice = (int)mob_proto_table[i1].mana_nodice;
    mob_proto_table[i2].mana_sizedice = (int)mob_proto_table[i1].mana_sizedice;
    mob_proto_table[i2].mana_add      = (int)mob_proto_table[i1].mana_add;

    mob_proto_table[i2].skin_value     = (int)mob_proto_table[i1].skin_value;
    for (i=0;i<6;i++)
      mob_proto_table[i2].skin_vnum[i] = (int)mob_proto_table[i1].skin_vnum[i];

    log_cmd("mclone","  assigning %d attacks",(int)mob_proto_table[i1].no_att);
    mob_proto_table[i2].no_att        = (int)mob_proto_table[i1].no_att;
    if(mob_proto_table[i2].no_att>0) {
      for (i=0;i<mob_proto_table[i2].no_att;i++) {
        if(i==MAX_ATTS) break;
        mob_proto_table[i2].att_type[i]    = (int)mob_proto_table[i1].att_type[i];
        mob_proto_table[i2].att_target[i]  = (int)mob_proto_table[i1].att_target[i];
        mob_proto_table[i2].att_percent[i] = (int)mob_proto_table[i1].att_percent[i];
        mob_proto_table[i2].att_spell[i]   = (int)mob_proto_table[i1].att_spell[i];
      }
    }
    mob_proto_table[i2].tagline = 0;
    if(mob_proto_table[i1].tagline) {
      for (tag = mob_proto_table[i1].tagline; tag; tag = tag->next) {
        log_cmd("mclone","  assigning tagline %s",tag->desc);
        CREATE(tmp_tag, struct tagline_data, 1);
        sprintf(temptext,"%s",tag->desc);
        tmp_tag->desc = str_dup(temptext);
        tmp_tag->next = mob_proto_table[i2].tagline;
        mob_proto_table[i2].tagline = tmp_tag;
      }
    }
    log_cmd("mclone","  writing test.mob file");
    fl = fopen("test.mob","w");
    write_mob(fl, i2);
    fclose(fl);
    log_cmd("mclone","Done");
    send_to_char("Done\n\r",ch);
  }
  else {
    sprintf(txt,"Mobile %d already exists.\n\r",number2);
    send_to_char(txt, ch);
  }
}

/* Procedure oclone written for Ronin by Ranger
**
** Allows a obj to be cloned for online creation purposes.
**
** May 20, 1996
*/
void do_oclone(CHAR *ch, char *argument, int cmd) {
  int number1,number2;
  int i1,i2;
  int i,zone1,zone2;
  char usage[]="Usage: oclone <obj #1> <obj #2>  (Object 1 exists)\n\rIf the command crashes, try a different # for obj 2.\n\r";
  char txt[100],num1[100],num2[100];
  char temptext[MAX_STRING_LENGTH];
  struct extra_descr_data *desc,*tmp_desc;
  FILE *fl;

  if(!check_olc_access(ch)) return;

  argument_interpreter(argument,num1,num2);

  if(!*num2) {
    send_to_char(usage,ch);
    return;
  }

  for (i = 0; i < strlen(num1); i++) {
    if (!isdigit(*num1)) {
      send_to_char("Object value is not a valid number.\n\r", ch);
      send_to_char(usage,ch);
      return;
    }
  }
  for (i = 0; i < strlen(num2); i++) {
    if (!isdigit(*num2)) {
      send_to_char("Object value is not a valid number.\n\r", ch);
      send_to_char(usage,ch);
      return;
    }
  }

  number1 = atoi(num1);
  number2 = atoi(num2);

  zone1 = inzone(number1);
  zone2 = inzone(number2);
  if(!check_zone(ch,zone1)) return;
  if(!check_zone(ch,zone2)) return;
  if(!check_zone_access(ch,zone2)) return;

  i1 = real_object(number1);
  if(i1 == -1) {
    sprintf(txt,"Object %d doesn't exist.\n\r",number1);
    send_to_char(txt,ch);
    return;
  }
  i2 = real_object(number2);
  if(i2 == -1) {
    log_cmd("oclone","oclone %d %d by %s",number1,number2,GET_NAME(ch));
    log_cmd("oclone","  allocating obj %d",number2);
    i2 = allocate_obj(number2);
    obj_proto_table[i2].virtual=number2;
    i1 = real_object(number1);
    if (i1 == -1) {
      log_s("Error in oclone, obj table corrupt.");
      produce_core();
      return;
    }

    sprintf(temptext,"%s",obj_proto_table[i1].name);
    log_cmd("oclone","  assigning obj name");
    obj_proto_table[i2].name               = str_dup(temptext);
    sprintf(temptext,"%s",obj_proto_table[i1].short_description);
    log_cmd("oclone","  assigning short description");
    obj_proto_table[i2].short_description  = str_dup(temptext);
    sprintf(temptext,"%s",obj_proto_table[i1].description);
    log_cmd("oclone","  assigning obj description");
    obj_proto_table[i2].description        = str_dup(temptext);
    log_cmd("oclone","  assigning obj action description");
    if(obj_proto_table[i1].action_description) {
      sprintf(temptext,"%s",obj_proto_table[i1].action_description);
      obj_proto_table[i2].action_description = str_dup(temptext);
    }
    else obj_proto_table[i2].action_description = 0;

    log_cmd("oclone","  assigning obj action description nt");
    if(obj_proto_table[i1].action_description_nt) {
      sprintf(temptext,"%s",obj_proto_table[i1].action_description_nt);
      obj_proto_table[i2].action_description_nt = str_dup(temptext);
    }
    else obj_proto_table[i2].action_description_nt = 0;

    log_cmd("oclone","  assigning obj char wear description");
    if(obj_proto_table[i1].char_wear_desc) {
      sprintf(temptext,"%s",obj_proto_table[i1].char_wear_desc);
      obj_proto_table[i2].char_wear_desc = str_dup(temptext);
    }
    else obj_proto_table[i2].char_wear_desc = 0;

    log_cmd("oclone","  assigning obj room wear description");
    if(obj_proto_table[i1].room_wear_desc) {
      sprintf(temptext,"%s",obj_proto_table[i1].room_wear_desc);
      obj_proto_table[i2].room_wear_desc = str_dup(temptext);
    }
    else obj_proto_table[i2].room_wear_desc = 0;

    log_cmd("oclone","  assigning obj char remove description");
    if(obj_proto_table[i1].char_rem_desc) {
      sprintf(temptext,"%s",obj_proto_table[i1].char_rem_desc);
      obj_proto_table[i2].char_rem_desc = str_dup(temptext);
    }
    else obj_proto_table[i2].char_rem_desc = 0;

    log_cmd("oclone","  assigning obj room remove description");
    if(obj_proto_table[i1].room_rem_desc) {
      sprintf(temptext,"%s",obj_proto_table[i1].room_rem_desc);
      obj_proto_table[i2].room_rem_desc = str_dup(temptext);
    }
    else obj_proto_table[i2].room_rem_desc = 0;

    log_cmd("oclone","  assigning ints up to bitvector");
    obj_proto_table[i2].obj_flags.type_flag = (int)obj_proto_table[i1].obj_flags.type_flag;
    obj_proto_table[i2].obj_flags.extra_flags = (int)obj_proto_table[i1].obj_flags.extra_flags;
    obj_proto_table[i2].obj_flags.extra_flags2 = (int)obj_proto_table[i1].obj_flags.extra_flags2;
    obj_proto_table[i2].obj_flags.subclass_res = (int)obj_proto_table[i1].obj_flags.subclass_res;
    obj_proto_table[i2].obj_flags.material = (int)obj_proto_table[i1].obj_flags.material;
    obj_proto_table[i2].obj_flags.wear_flags = (int)obj_proto_table[i1].obj_flags.wear_flags;
    obj_proto_table[i2].obj_flags.value[0] = (int)obj_proto_table[i1].obj_flags.value[0];
    obj_proto_table[i2].obj_flags.value[1] = (int)obj_proto_table[i1].obj_flags.value[1];
    obj_proto_table[i2].obj_flags.value[2] = (int)obj_proto_table[i1].obj_flags.value[2];
    obj_proto_table[i2].obj_flags.value[3] = (int)obj_proto_table[i1].obj_flags.value[3];
    obj_proto_table[i2].obj_flags.weight = (int)obj_proto_table[i1].obj_flags.weight;
    obj_proto_table[i2].obj_flags.cost = (int)obj_proto_table[i1].obj_flags.cost;
    obj_proto_table[i2].obj_flags.cost_per_day = (int)obj_proto_table[i1].obj_flags.cost_per_day;
    obj_proto_table[i2].obj_flags.repop_percent = (int)obj_proto_table[i1].obj_flags.repop_percent;
    obj_proto_table[i2].obj_flags.timer = (int)obj_proto_table[i1].obj_flags.timer;
    obj_proto_table[i2].obj_flags.bitvector = (long)obj_proto_table[i1].obj_flags.bitvector;
    obj_proto_table[i2].obj_flags.bitvector2 = (long)obj_proto_table[i1].obj_flags.bitvector2;
    obj_proto_table[i2].ex_description = 0;
    if(obj_proto_table[i1].ex_description) {
      for (desc = obj_proto_table[i1].ex_description; desc; desc = desc->next) {
        log_cmd("oclone","  assigning obj extra description %s",desc->keyword);
        CREATE(tmp_desc, struct extra_descr_data, 1);
        sprintf(temptext,"%s",desc->keyword);
        tmp_desc->keyword     = str_dup(temptext);
        sprintf(temptext,"%s",desc->description);
        tmp_desc->description = str_dup(temptext);
        tmp_desc->next = obj_proto_table[i2].ex_description;
        obj_proto_table[i2].ex_description = tmp_desc;
      }
    }
    log_cmd("oclone","  assigning obj affects");
    for (i = 0;(i < MAX_OBJ_AFFECT); i++) {
      obj_proto_table[i2].affected[i].location = (int)obj_proto_table[i1].affected[i].location;
      obj_proto_table[i2].affected[i].modifier = (int)obj_proto_table[i1].affected[i].modifier;
    }

    log_cmd("oclone","  writing test.obj file");
    fl = fopen("test.obj","w");
    write_obj(fl, i2);
    fclose(fl);
    log_cmd("oclone","Done");
    send_to_char("Done\n\r",ch);
  }
  else {
    sprintf(txt,"Object %d already exists.\n\r",number2);
    send_to_char(txt, ch);
  }
}

/* rezone written for RoninMUD by Ranger
**
** Allows a zone number to be changed.
**
** Do not distribute without permission from the originator
**
**/

#define ZCMD zone_table[j].cmd[i]
void do_rezone(CHAR *ch, char *argument, int cmd) {
  int vzone1,vzone2,i,j,change;
  char usage[]="Usage: rezone <znum 1> <znum 2>  (Zone 1 exists)\n\r";
  char buf[100],num1[100],num2[100];
  CHAR *k,*k2;
  OBJ *obj,*obj2;

  if(!check_olc_access(ch)) return;

  argument_interpreter(argument,num1,num2);
  if(!*num2) {
    send_to_char(usage,ch);
    return;
  }

  if(!is_number(num1)|| !is_number(num2)) {
    send_to_char("Zone values are not valid numbers.\n\r", ch);
    send_to_char(usage,ch);
    return;
  }

  vzone1=atoi(num1);
  vzone2=atoi(num2);

  if(real_zone(vzone1)==-1) {
    sprintf(buf,"Zone %d doesn't exist.\n\r",vzone1);
    send_to_char(buf,ch);
    return;
  }
  if(!check_zone_access(ch,vzone1)) return;

  if(real_zone(vzone2)!=-1) {
    sprintf(buf,"Zone %d already exists.\n\r",vzone2);
    send_to_char(buf,ch);
    return;
  }

  if(vzone2==320) {
    send_to_char("Zone # 320 is a reserved zone # used for object versions.\n\r",ch);
    return;
  }

  /* Checks are done, now extract stuff from the zone */
  for(k=character_list; k; k=k2) {
    k2=k->next;
    if(IS_NPC(k)) {
       if( vzone1==inzone(V_MOB(k)) || vzone1==inzone(CHAR_VIRTUAL_ROOM(k)) )
         extract_char(k);
    }
    else {
      if(vzone1==inzone(CHAR_VIRTUAL_ROOM(k))) {
        char_from_room(k);
        char_to_room(k,real_room(1212));
      }
    }
  }

  for(obj=object_list;obj;obj=obj2) {
    obj2=obj->next;
    if(vzone1==inzone(V_OBJ(obj)) || vzone1==inzone(obj->in_room_v))
      extract_obj(obj);
  }

  /* renumbering  */
  change=vzone2*100-vzone1*100;

  /*mobs*/
  for(i=0;i<=top_of_mobt;i++)
    if(vzone1==inzone(mob_proto_table[i].virtual))
      mob_proto_table[i].virtual+=change;

  /*objs*/
  for(i=0;i<=top_of_objt;i++)
    if(vzone1==inzone(obj_proto_table[i].virtual))
      obj_proto_table[i].virtual+=change;

  /*rooms*/
  for(i=0;i<=top_of_world;i++) {
    if(vzone1==inzone(world[i].number))
      world[i].number+=change;
    for(j=0;j<6;j++) {
      if(world[i].dir_option[j])
        if(vzone1==inzone(world[i].dir_option[j]->to_room_v))
          world[i].dir_option[j]->to_room_v+=change;
    }
  }

  /*zone resets*/
  j=real_zone(vzone1);
  for (i=0;;i++) {
    if(ZCMD.command == 'S') break;
    switch(ZCMD.command) {
      case 'M': /* read a mobile */
      case 'F': /* follow a mobile */
      case 'R': /* add mount for M */
      case 'O': /* read an object */
      case 'P': /* object to object */
        if(vzone1==inzone(ZCMD.arg1)) ZCMD.arg1+=change;
        if(vzone1==inzone(ZCMD.arg3)) ZCMD.arg3+=change;
        break;
      case 'G': /* obj_to_char */
      case 'E': /* object to equipment list */
      case 'D': /* set state of door */
        if(vzone1==inzone(ZCMD.arg1)) ZCMD.arg1+=change;
        break;
      default:
        sprintf(buf,"Undefd cmd in reset table; zone %d cmd %d.\n\r",j,i);
        log_s(buf);
        produce_core();
        break;
    }
  }

  /*zone*/
  i=real_zone(vzone1);
  zone_table[i].virtual=vzone2;
  zone_table[i].bottom=vzone2*100;
  zone_table[i].top=vzone2*100+99;
  send_to_char("Done. You must now reboot and reload to work on the new zone.\n\r",ch);
}

void do_rflag(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command toggles the room flags listed on/off.\n\r\
\n\r\
  Usage: `krflag`q <#> <list of flags>/all/none\n\r\
     Ex: `krflag`q 1212 `iPRIVATE DEATH`q (caps not important)\n\r\
      or `krflag `iPRIVATE DEATH`q (flags the room you're in)\n\r\
\n\r\
Flags are:\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    print_bits_to_char(room_bits, ch);
    return;
  }

  int room_vnum = -1;

  if (is_number(arg)) {
    room_vnum = atoi(arg);

    argument = one_argument(argument, arg);

    if (!*arg) {
      send_to_char(usage, ch);
      print_bits_to_char(room_bits, ch);
      return;
    }
  }
  else {
    room_vnum = CHAR_VIRTUAL_ROOM(ch);
  }

  int room_zone = inzone(room_vnum);

  if (!check_zone(ch, room_zone)) return;
  if (!check_zone_access(ch, room_zone)) return;

  int room_rnum = real_room(room_vnum);

  if (!check_room(ch, room_rnum)) return;

  if (!strcmp(arg, "all")) {
    int bitpos = old_search_block("\n", 0, strlen("\n"), room_bits, FALSE);
    world[room_rnum].room_flags = ((1 << (bitpos - 1)) - 1);
    add_flying_room(room_rnum);
    send_to_char("`iAll room flags added.`q\n\r", ch);
    return;
  }

  if (!strcmp(arg, "none")) {
    world[room_rnum].room_flags = 0;
    remove_flying_room(room_rnum);
    send_to_char("`iAll room flags removed.`q\n\r", ch);
    return;
  }

  while (*arg) {
    int bit_pos = old_search_block(string_to_upper(arg), 0, strlen(arg), room_bits, FALSE);

    if (bit_pos != -1) {
      unsigned long bit = (1 << (bit_pos - 1));

      if (IS_SET(world[room_rnum].room_flags, bit)) {
        REMOVE_BIT(world[room_rnum].room_flags, bit);

        if (bit == FLYING) remove_flying_room(room_rnum);
      }
      else {
        SET_BIT(world[room_rnum].room_flags, bit);

        if (bit == FLYING) add_flying_room(room_rnum);
      }
    }
    else {
      char buf[MSL];
      snprintf(buf, sizeof(buf), "`kRoom flag %s not found.`q\n\r", arg);
      send_to_char(buf, ch);
    }

    argument = one_argument(argument, arg);
  }

  send_to_char("\n\r`iRoom flag(s) added/removed.`q\n\r", ch);
}

void do_rdelete(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command is used to delete a room from a zone.\n\r\
\n\r\
  Usage: `krdelete`q <room number>\n\r\
     Ex: `krdelete`q 1212\n\r\
\n\r\
Anyone standing in the room will be moved to room 1212.\n\r\
Mobs and objects in the room will be purged from the game,\n\r\
but will not be deleted from the zone.\n\r\
\n\r\
Any existing exits to the room being deleted will be removed.\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  if (!is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int room_vnum = atoi(arg);

  if (room_vnum == 0 || room_vnum == 1 || room_vnum == 1212) {
    send_to_char("`iYou cannot delete that room.`q\n\r", ch);
    return;
  }

  int room_zone = inzone(room_vnum);

  if (!check_zone(ch, room_zone)) return;
  if (!check_zone_access(ch, room_zone)) return;

  int room_rnum = real_room(room_vnum);

  if (!check_room(ch, room_rnum)) return;

  /* Move players to 1212 and purge mobiles from the room. */
  for (CHAR *vict = world[room_rnum].people, *next_vict = NULL; vict; vict = next_vict) {
    next_vict = vict->next_in_room;

    if (!IS_NPC(vict)) {
      char_from_room(vict);
      char_to_room(vict, real_room(1212));
      do_look(vict, "\0", CMD_LOOK);
    }
    else {
      extract_char(vict);
    }
  }

  /* Purge objects from the room. */
  for (OBJ *obj = world[room_rnum].contents, *next_obj = NULL; obj; obj = next_obj) {
    next_obj = obj->next_content;

    extract_obj(obj);
  }

  /* Remove any existing exits to the room. */
  for (int i = 0; i <= top_of_world; i++) {
    for (int door = NORTH; door <= DOWN; door++) {
      if (world[i].dir_option[door]) {
        if (world[i].dir_option[door]->to_room_v == room_vnum) {
          if (world[i].dir_option[door]->general_description) {
            free(world[i].dir_option[door]->general_description);
          }
          if (world[i].dir_option[door]->keyword) {
            free(world[i].dir_option[door]->keyword);
          }
          free(world[i].dir_option[door]);
          world[i].dir_option[door] = NULL;
        }
      }
    }
  }

  /* If not the top room, move all room that are above down by one. */
  if (room_rnum != top_of_world) {
    memmove(&world[room_rnum], &world[room_rnum + 1], (sizeof(struct room_data) * (top_of_world - room_rnum)));
  }

  /* Clear and free the top room element. */
  memset(&world[top_of_world], 0, sizeof(struct room_data));
  free(&world[top_of_world]);

  /* Decrement the count of rooms by one. */
  top_of_world--;

  /* Fix up the world after room deletion. */
  renum_world();

  send_to_char("`iRoom deleted.`q\n\r", ch);
}

void do_mdelete(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command is used to delete a mob from a zone.\n\r\
\n\r\
  Usage: `kmdelete`q <mob number>\n\r\
     Ex: `kmdelete`q 3005\n\r\
\n\r\
Any equipment the mob has will be purged from the game,\n\r\
but will not be deleted from the zone.\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  if (!is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int mob_vnum = atoi(arg);

  int mob_zone = inzone(mob_vnum);

  if (!check_zone(ch, mob_zone)) return;
  if (!check_zone_access(ch, mob_zone)) return;

  int mob_rnum = real_mobile(mob_vnum);

  if (!check_mob(ch, mob_rnum)) return;

  /* Purge any objects carried or equipped, then purge this mob from the character list. */
  for (CHAR *mob = character_list, *next_mob = NULL; mob; mob = next_mob) {
    next_mob = mob->next;

    if (IS_NPC(mob) && V_MOB(mob) == mob_zone) {
      for (OBJ *obj = mob->carrying, *next_obj = NULL; obj; obj = next_obj) {
        next_obj = obj->next_content;

        extract_obj(obj);
      }

      for (int i = 0; i < MAX_WEAR; i++) {
        if (mob->equipment[i]) {
          extract_obj(unequip_char(mob, i));
        }
      }

      extract_char(mob);
    }
  }

  /* If not the top mob, move all mobs that are above down by one. */
  if (mob_rnum != top_of_mobt) {
    memmove(&mob_proto_table[mob_rnum], &mob_proto_table[mob_rnum + 1], (sizeof(struct char_data) * (top_of_mobt - mob_rnum)));
  }

  /* Clear and free the top mob element. */
  memset(&mob_proto_table[top_of_mobt], 0, sizeof(struct char_data));
  free(&mob_proto_table[top_of_mobt]);

  /* Decrement the count of mobs by one. */
  top_of_mobt--;

  /* Reassign real numbers for existing mobs. */
  for (CHAR *mob = character_list, *next_mob = NULL; mob; mob = next_mob) {
    next_mob = mob->next;

    if (IS_NPC(mob)) {
      mob->nr = real_mobile(mob->nr_v);
    }
  }

  send_to_char("`iMob deleted.`q\n\r", ch);
}

void do_odelete(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command is used to delete an object from a zone.\n\r\
\n\r\
  Usage: `kodelete`q <object number>\n\r\
     Ex: `kodelete`q 2296\n\r\
\n\r\
Any equipment in the object will be purged from the game,\n\r\
but will not be deleted from the zone.\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  if (!is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int obj_vnum = atoi(arg);

  int obj_zone = inzone(obj_vnum);

  if (!check_zone(ch, obj_zone)) return;
  if (!check_zone_access(ch, obj_zone)) return;

  int obj_rnum = real_object(obj_vnum);

  if (!check_obj(ch, obj_rnum)) return;

  /* Purge this object from the object list. */
  for (OBJ *obj = object_list, *next_obj = NULL; obj; obj = next_obj) {
    next_obj = obj->next;

    if (V_OBJ(obj) == obj_vnum) {
      extract_obj(obj);
    }
  }

  /* If not the top object, move all objects that are above down by one. */
  if (obj_rnum != top_of_objt) {
    memmove(&obj_proto_table[obj_rnum], &obj_proto_table[obj_rnum + 1], (sizeof(struct obj_data) * (top_of_objt - obj_rnum)));
  }

  /* Clear and free the top object element. */
  memset(&obj_proto_table[top_of_objt], 0, sizeof(struct obj_data));
  free(&obj_proto_table[top_of_objt]);

  /* Decrement the count of objects by one. */
  top_of_objt--;

  /* Reassign real numbers for existing objects. */
  for (OBJ *obj = object_list, *next_obj = NULL; obj; obj = next_obj) {
    next_obj = obj->next;

    obj->item_number = real_object(obj->item_number_v);
  }

  send_to_char("`iObject deleted.`q\n\r", ch);
}

void do_sdelete(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command is used to delete a shop from a zone.\n\r\
\n\r\
  Usage: `ksdelete`q <shop number>\n\r\
     Ex: `ksdelete`q 2999\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  if (!is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int shop_vnum = atoi(arg);

  int shop_zone = inzone(shop_vnum);

  if (!check_zone(ch, shop_zone)) return;
  if (!check_zone_access(ch, shop_zone)) return;

  int shop_num = -1;

  for (int i = 0; i < number_of_shops; i++) {
    if (shop_index[i].keeper == shop_vnum) {
      shop_num = i;
      break;
    }
  }

  if (shop_num < 0 || shop_num > number_of_shops) {
    printf_to_char(ch, "Shop %d does not exist.\n\r", shop_vnum);
    return;
  }

  /* If not the top shop, move all shops that are above down by one. */
  if (shop_num != number_of_shops) {
    memmove(&shop_index[shop_num], &shop_index[shop_num + 1], (sizeof(struct shop_data) * (number_of_shops - shop_num)));
  }

  /* Clear and free the top shop element. */
  memset(&shop_index[number_of_shops], 0, sizeof(struct shop_data));
  free(&shop_index[number_of_shops]);

  /* Decrement the count of shops by one. */
  number_of_shops--;

  send_to_char("`iShop deleted.`q\n\r", ch);
}

void do_rlink(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command will link one room to another with a one way exit.\n\r\
\n\r\
  Usage: `krlink`q <room1> <n/s/e/w/u/d> <room2> (room1 is optional)\n\r\
     Ex: `krlink`q 1212 d 1255\n\r\
      or `krlink`q d 1255 (standing in room 1212)\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  int vroom1 = -1;

  if (is_number(arg)) {
    vroom1 = atoi(arg);

    argument = one_argument(argument, arg);

    if (!*arg) {
      send_to_char(usage, ch);
      return;
    }
  }
  else {
    vroom1 = CHAR_VIRTUAL_ROOM(ch);
  }

  int dir = -1;

  if (is_abbrev(arg, "north")) { dir = 0; }
  else if (is_abbrev(arg, "east")) { dir = 1; }
  else if (is_abbrev(arg, "south")) { dir = 2; }
  else if (is_abbrev(arg, "west")) { dir = 3; }
  else if (is_abbrev(arg, "up")) { dir = 4; }
  else if (is_abbrev(arg, "down")) { dir = 5; }
  else {
    send_to_char(usage, ch);
    return;
  }

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  if (!is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vroom2 = atoi(arg);

  int zone1 = inzone(vroom1);

  if (!check_zone_no_message(zone1)) {
    send_to_char("`iThe zone for room 1 doesn't exist.`q\n\r", ch);
    return;
  }
 
  if (!check_zone_access_no_message(ch, zone1)) {
    send_to_char("`iYou don't have permission to edit zone 1.`q\n\r", ch);
    return;
  }

  int zone2 = inzone(vroom2);

  if (!check_zone_no_message(zone2)) {
    send_to_char("`iThe zone for room 2 doesn't exist.`q\n\r", ch);
    return;
  }
  if (!check_zone_access_no_message(ch, zone2)) {
    send_to_char("`iYou don't have permission to edit zone 2.`q\n\r", ch);
    return;
  }

  int room1 = real_room(vroom1);

  if (!check_room_no_message(room1)) {
    send_to_char("`iRoom 1 doesn't exist.`q\n\r", ch);
    return;
  }
  if (world[room1].dir_option[dir]) {
    send_to_char("`iAn exit already exists in that direction.`q\n\r", ch);
    return;
  }

  int room2 = real_room(vroom2);

  if (!check_room_no_message(room2)) {
    send_to_char("`iRoom 2 doesn't exist.`q\n\r", ch);
    return;
  }

  CREATE(world[room1].dir_option[dir], struct room_direction_data, 1);
  world[room1].dir_option[dir]->general_description = NULL;
  world[room1].dir_option[dir]->keyword = NULL;
  world[room1].dir_option[dir]->to_room_v = vroom2;
  world[room1].dir_option[dir]->to_room_r = room2;
  world[room1].dir_option[dir]->exit_info = 0;
  world[room1].dir_option[dir]->key = -1;

  send_to_char("\n\r`iRoom link added.`q\n\r",ch);
}

void do_runlink(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command will unlink one room from another, it won't unlink the other side.\n\r\
\n\r\
  Usage: `krunlink`q <room> <n/s/e/w/u/d> (room is optional)\n\r\
     Ex: `krunlink`q 1212 d\n\r\
      or `krunlink`q d (standing in room 1212)\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  int vroom1 = -1;

  if (is_number(arg)) {
    vroom1 = atoi(arg);

    argument = one_argument(argument, arg);

    if (!*arg) {
      send_to_char(usage, ch);
      return;
    }
  }
  else {
    vroom1 = CHAR_VIRTUAL_ROOM(ch);
  }

  int dir = -1;

  if (is_abbrev(arg, "north")) { dir = 0; }
  else if (is_abbrev(arg, "east")) { dir = 1; }
  else if (is_abbrev(arg, "south")) { dir = 2; }
  else if (is_abbrev(arg, "west")) { dir = 3; }
  else if (is_abbrev(arg, "up")) { dir = 4; }
  else if (is_abbrev(arg, "down")) { dir = 5; }
  else {
    send_to_char(usage, ch);
    return;
  }

  int zone1 = inzone(vroom1);

  if (!check_zone(ch, zone1)) return;
  if (!check_zone_access(ch, zone1)) return;

  int room1 = real_room(vroom1);

  if (!check_room(ch, room1)) return;

  if (!world[room1].dir_option[dir]) {
    send_to_char("`iNo exit exists in that direction.`q\n\r", ch);
    return;
  }

  if (world[room1].dir_option[dir]->general_description) {
    free(world[room1].dir_option[dir]->general_description);
  }

  if (world[room1].dir_option[dir]->keyword) {
    free(world[room1].dir_option[dir]->keyword);
  }

  free(world[room1].dir_option[dir]);

  world[room1].dir_option[dir] = NULL;

  send_to_char("\n\r`iRoom unlinked.`q\n\r",ch);
}

void do_dlink(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command will link one room to another with a two-way exit.\n\r\
\n\r\
  Usage: `kdlink`q <room1> <n/s/e/w/u/d> <room2> (room1 is optional)\n\r\
     Ex: `kdlink`q 1212 d 1255\n\r\
      or `kdlink`q d 1255 (standing in room 1212)\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  int vroom1 = -1;

  if (is_number(arg)) {
    vroom1 = atoi(arg);

    argument = one_argument(argument, arg);

    if (!*arg) {
      send_to_char(usage, ch);
      return;
    }
  }
  else {
    vroom1 = CHAR_VIRTUAL_ROOM(ch);
  }

  int dir = -1, odir = -1;

  if (is_abbrev(arg, "north")) { dir = 0; odir = 2; }
  else if (is_abbrev(arg, "east")) { dir = 1; odir = 3; }
  else if (is_abbrev(arg, "south")) { dir = 2; odir = 0; }
  else if (is_abbrev(arg, "west")) { dir = 3; odir = 1; }
  else if (is_abbrev(arg, "up")) { dir = 4; odir = 5; }
  else if (is_abbrev(arg, "down")) { dir = 5; odir = 4; }
  else {
    send_to_char(usage, ch);
    return;
  }

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  if (!is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vroom2 = atoi(arg);

  int zone1 = inzone(vroom1);

  if (!check_zone_no_message(zone1)) {
    send_to_char("`iThe zone for room 1 doesn't exist.`q\n\r", ch);
    return;
  }
 
  if (!check_zone_access_no_message(ch, zone1)) {
    send_to_char("`iYou don't have permission to edit zone 1.`q\n\r", ch);
    return;
  }

  int zone2 = inzone(vroom2);

  if (!check_zone_no_message(zone2)) {
    send_to_char("`iThe zone for room 2 doesn't exist.`q\n\r", ch);
    return;
  }
  if (!check_zone_access_no_message(ch, zone2)) {
    send_to_char("`iYou don't have permission to edit zone 2.`q\n\r", ch);
    return;
  }

  int room1 = real_room(vroom1);

  if (!check_room_no_message(room1)) {
    send_to_char("`iRoom 1 doesn't exist.`q\n\r", ch);
    return;
  }
  if (world[room1].dir_option[dir]) {
    send_to_char("`iAn exit in room 1 already exists in that direction.`q\n\r", ch);
    return;
  }

  int room2 = real_room(vroom2);

  if (!check_room_no_message(room2)) {
    send_to_char("`iRoom 2 doesn't exist.`q\n\r", ch);
    return;
  }
  if (world[room2].dir_option[odir]) {
    send_to_char("`iAn exit in room 2 already exists in the opposite direction.`q\n\r", ch);
    return;
  }

  CREATE(world[room1].dir_option[dir], struct room_direction_data, 1);
  world[room1].dir_option[dir]->general_description = NULL;
  world[room1].dir_option[dir]->keyword = NULL;
  world[room1].dir_option[dir]->to_room_v = vroom2;
  world[room1].dir_option[dir]->to_room_r = room2;
  world[room1].dir_option[dir]->exit_info = 0;
  world[room1].dir_option[dir]->key = -1;

  CREATE(world[room2].dir_option[odir], struct room_direction_data, 1);
  world[room2].dir_option[odir]->general_description = NULL;
  world[room2].dir_option[odir]->keyword = NULL;
  world[room2].dir_option[odir]->to_room_v = vroom1;
  world[room2].dir_option[odir]->to_room_r = room1;
  world[room2].dir_option[odir]->exit_info = 0;
  world[room2].dir_option[odir]->key = -1;

  send_to_char("\n\r`iDouble room link added.`q\n\r", ch);
}

void do_dunlink(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command will unlink one room from another, and the other side if\n\r\
it is a two-way exit.\n\r\
\n\r\
  Usage: `kdunlink`q <room> <n/s/e/w/u/d> (room is optional)\n\r\
     Ex: `kdunlink`q 1212 d\n\r\
      or `kdunlink`q d (standing in room 1212)\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  int vroom1 = -1;

  if (is_number(arg)) {
    vroom1 = atoi(arg);

    argument = one_argument(argument, arg);

    if (!*arg) {
      send_to_char(usage, ch);
      return;
    }
  }
  else {
    vroom1 = CHAR_VIRTUAL_ROOM(ch);
  }

  int dir = -1, odir = -1;

  if (is_abbrev(arg, "north")) { dir = 0; odir = 2; }
  else if (is_abbrev(arg, "east")) { dir = 1; odir = 3; }
  else if (is_abbrev(arg, "south")) { dir = 2; odir = 0; }
  else if (is_abbrev(arg, "west")) { dir = 3; odir = 1; }
  else if (is_abbrev(arg, "up")) { dir = 4; odir = 5; }
  else if (is_abbrev(arg, "down")) { dir = 5; odir = 4; }
  else {
    send_to_char(usage, ch);
    return;
  }

  int zone1 = inzone(vroom1);

  if (!check_zone_no_message(zone1)) {
    send_to_char("`iThe zone for room 1 doesn't exist.`q\n\r", ch);
    return;
  }
  if (!check_zone_access_no_message(ch, zone1)) {
    send_to_char("`iYou don't have permission to edit zone 1.`q\n\r", ch);
    return;
  }

  int room1 = real_room(vroom1);

  if (!check_room_no_message(room1)) {
    send_to_char("`iRoom 1 doesn't exist.`q\n\r", ch);
    return;
  }
  if (!world[room1].dir_option[dir]) {
    send_to_char("`iNo exit exists in that direction.`q\n\r", ch);
    return;
  }

  int vroom2 = world[room1].dir_option[dir]->to_room_v;

  if (world[room1].dir_option[dir]->general_description) {
    free(world[room1].dir_option[dir]->general_description);
  }

  if (world[room1].dir_option[dir]->keyword) {
    free(world[room1].dir_option[dir]->keyword);
  }

  free(world[room1].dir_option[dir]);

  world[room1].dir_option[dir] = NULL;

  int zone2 = inzone(vroom2);

  if (!check_zone_no_message(zone2)) {
    send_to_char("`iThe zone for room 2 doesn't exist, removing the first side only.`q\n\r", ch);
    return;
  }
  if (!check_zone_access_no_message(ch, zone2)) {
    send_to_char("`iYou don't have permission to edit zone 2, removing the first side only.`q\n\r", ch);
    return;
  }

  int room2 = real_room(vroom2);

  if (!check_room_no_message(room2) || !world[room2].dir_option[odir]) {
    send_to_char("`iThe other side of the exit doesn't exist, removing the first side only.`q\n\r", ch);
    return;
  }
  if (world[room2].dir_option[odir]->to_room_v != vroom1) {
    send_to_char("`iThe other side of the exit isn't linked back to the first room, removing the first side only.`q\n\r", ch);
    return;
  }

  if (world[room2].dir_option[odir]->general_description) {
    free(world[room2].dir_option[odir]->general_description);
  }

  if (world[room2].dir_option[odir]->keyword) {
    free(world[room2].dir_option[odir]->keyword);
  }

  free(world[room2].dir_option[odir]);

  world[room2].dir_option[odir] = NULL;

  send_to_char("\n\r`iDouble room link removed.`q\n\r", ch);
}

void do_rdesc(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a room description.\n\r\n\r\
  Usage: `krdesc`q <room#>\n\r\
     Ex: `krdesc`q 1212\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vroom,room,zone;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vroom=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  room=real_room(vroom);
  if(!check_room(ch,room)) return;

  zone=inzone(vroom);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  send_to_char("`iEnter the room description, terminate with @ on its own line.`q\n\r",ch);

  if(world[room].description)
    world[room].description[0] = 0;
  ch->desc->str = &world[room].description;
  ch->desc->max_str = 4096;
}

void do_rname(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a room name.\n\r\n\r\
  Usage: `krname`q <room#>\n\r\
     Ex: `krname`q 1212\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vroom,room,zone;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vroom=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  room=real_room(vroom);
  if(!check_room(ch,room)) return;

  zone=inzone(vroom);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  send_to_char("`iEnter the room name, terminate with @@ on the same line.`q\n\r",ch);

  if(world[room].name)
    world[room].name[0] = 0;
  ch->desc->str = &world[room].name;
  ch->desc->max_str = 80;
}

void do_rsect(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command sets the sector of a room.\n\r\n\r\
  Usage: `krsect`q <#> <sector>\n\r\
     Ex: `krsect`q 1212 `iINSIDE`q\n\r\
      or `krsect `iINSIDE`q (changes the sector of the room you're in)\n\r\n\r\
Flags are: `iINSIDE, CITY, FOREST, FIELD, HILLS, MOUNTAIN, SWIMMABLE WATER,\n\r\
            NOSWIM WATER, ARCTIC, DESERT.`q\n\r";
  char arg[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH];
  int vroom,room,zone,sect;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) {
    vroom=atoi(arg);
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
  }
  else {
    vroom=CHAR_VIRTUAL_ROOM(ch);
  }

  room=real_room(vroom);
  if(!check_room(ch,room)) return;

  zone=inzone(vroom);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  sect=old_search_block(string_to_upper(arg),0,strlen(arg), sector_types_i, FALSE);
  if(sect==-1) {
    send_to_char(usage,ch);
    return;
  }

  world[room].sector_type= sect-1;
  sprintf(buf,"`kRoom sector changed to %s.`q\n\r",sector_types_i[sect-1]);
  send_to_char(buf,ch);

}

void do_exitdesc(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command will add an exit description to the indicated direction.\n\r\n\r\
  Usage: `kexitdesc`q <room> <n/s/e/w/u/d> (room is optional)\n\r\
     Ex: `kexitdesc`q 1212 e\n\r\
      or `kexitdesc`q e (standing in room 1212)\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vroom,room,zone,dir;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) {
    vroom=atoi(arg);
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
  }
  else {
    vroom=CHAR_VIRTUAL_ROOM(ch);
  }

  dir=-1;
  if(is_abbrev(arg, "north")) dir = 0;
  if(is_abbrev(arg, "east"))  dir = 1;
  if(is_abbrev(arg, "south")) dir = 2;
  if(is_abbrev(arg, "west"))  dir = 3;
  if(is_abbrev(arg, "up"))    dir = 4;
  if(is_abbrev(arg, "down"))  dir = 5;
  if(dir==-1) {
    send_to_char(usage,ch);
    return;
  }

  room=real_room(vroom);
  if(room==NOWHERE || room>top_of_world) {
    send_to_char("`iThat room doesn't exist.`q\n\r",ch);
    return;
  }

  zone=inzone(vroom);
  if(zone==NOWHERE) {
    send_to_char("`iThe zone for that room doesn't exist.`q\n\r",ch);
    return;
  }

  if(!check_zone_access(ch,zone)) return;

  if(!world[room].dir_option[dir]) {
    send_to_char("`iAn exit doesn't exist in that direction.`q\n\r",ch);
    return;
  }

  send_to_char("\n\r`iEnter the exit description, terminate with @ on its own line.`q\n\r",ch);
  if(world[room].dir_option[dir]->general_description)
    world[room].dir_option[dir]->general_description[0] = 0;
  ch->desc->str = &world[room].dir_option[dir]->general_description;
  ch->desc->max_str = 4096;
}

void do_exitname(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command will add exit keywords/names to the indicated direction.\n\r\n\r\
  Usage: `kexitname`q <room> <n/s/e/w/u/d> <keywords/names> (room is optional)\n\r\
     Ex: `kexitname`q 1212 n archway passage\n\r\
      or `kexitname`q n archway passage (standing in room 1212)\n\r\
      or `kexitname`q 1212 n north (to put a default name)\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vroom,room,zone,dir;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) {
    vroom=atoi(arg);
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
  }
  else {
    vroom=CHAR_VIRTUAL_ROOM(ch);
  }

  dir=-1;
  if(is_abbrev(arg, "north")) dir = 0;
  if(is_abbrev(arg, "east"))  dir = 1;
  if(is_abbrev(arg, "south")) dir = 2;
  if(is_abbrev(arg, "west"))  dir = 3;
  if(is_abbrev(arg, "up"))    dir = 4;
  if(is_abbrev(arg, "down"))  dir = 5;
  if(dir==-1) {
    send_to_char(usage,ch);
    return;
  }

  for (;isspace(*argument);argument++);
  if(!*argument) {
    send_to_char(usage,ch);
    return;
  }

  room=real_room(vroom);
  if(room==NOWHERE || room>top_of_world) {
    send_to_char("`iThat room doesn't exist.`q\n\r",ch);
    return;
  }

  zone=inzone(vroom);
  if(zone==NOWHERE) {
    send_to_char("`iThe zone for that room doesn't exist.`q\n\r",ch);
    return;
  }

  if(!check_zone_access(ch,zone)) return;

  if(!world[room].dir_option[dir]) {
    send_to_char("`iAn exit doesn't exist in that direction.`q\n\r",ch);
    return;
  }

  if(world[room].dir_option[dir]->keyword)
    free(world[room].dir_option[dir]->keyword);
  world[room].dir_option[dir]->keyword = str_dup(argument);
  send_to_char("`iExit keyword added.`q\n\r",ch);
}

void do_exitkey(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command will assign a key number to the indicated direction.\n\r\n\r\
  Usage: `kexitkey`q <room> <n/s/e/w/u/d> <key number/none> (room is optional)\n\r\
     Ex: `kexitkey`q 1212 e 1234\n\r\
      or `kexitkey`q e none (standing in room 1212)\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vroom,room,zone,dir,key;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) {
    vroom=atoi(arg);
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
  }
  else {
    vroom=CHAR_VIRTUAL_ROOM(ch);
  }

  dir=-1;
  if(is_abbrev(arg, "north")) dir = 0;
  if(is_abbrev(arg, "east"))  dir = 1;
  if(is_abbrev(arg, "south")) dir = 2;
  if(is_abbrev(arg, "west"))  dir = 3;
  if(is_abbrev(arg, "up"))    dir = 4;
  if(is_abbrev(arg, "down"))  dir = 5;
  if(dir==-1) {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_abbrev(arg, "none")) {
    key=-1;
  }
  else {
    if(!is_number(arg)) {
      send_to_char(usage,ch);
      return;
    } else {
      key=atoi(arg);
    }
  }

  room=real_room(vroom);
  if(room==NOWHERE || room>top_of_world) {
    send_to_char("`iThat room doesn't exist.`q\n\r",ch);
    return;
  }

  zone=inzone(vroom);
  if(zone==NOWHERE) {
    send_to_char("`iThe zone for that room doesn't exist.`q\n\r",ch);
    return;
  }

  if(!check_zone_access(ch,zone)) return;

  if(!world[room].dir_option[dir]) {
    send_to_char("`iAn exit doesn't exist in that direction.`q\n\r",ch);
    return;
  }

  world[room].dir_option[dir]->key = key;
  if(key==-1)
    send_to_char("`iExit key reset to none.`q\n\r",ch);
  else
    send_to_char("`iExit key number added.`q\n\r",ch);
}

void do_exittype(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command will add an exit type to the indicated direction.\n\r\n\r\
  Usage: `kexittype`q <room> <n/s/e/w/u/d> <type> (room is optional)\n\r\
     Ex: `kexittype`q 1212 e normal\n\r\
      or `kexittype`q n door (standing in room 1212)\n\r\
  Types are: `inormal, climb, crawl, enter, jump, move, door, nopick, lock_10, lock_15,\n\r\
             lock_20, lock_25, lock_30\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vroom,room,zone,dir;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) {
    vroom=atoi(arg);
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
  }
  else {
    vroom=CHAR_VIRTUAL_ROOM(ch);
  }

  dir=-1;
  if(is_abbrev(arg, "north")) dir = 0;
  if(is_abbrev(arg, "east"))  dir = 1;
  if(is_abbrev(arg, "south")) dir = 2;
  if(is_abbrev(arg, "west"))  dir = 3;
  if(is_abbrev(arg, "up"))    dir = 4;
  if(is_abbrev(arg, "down"))  dir = 5;
  if(dir==-1) {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  room=real_room(vroom);
  if(room==NOWHERE || room>top_of_world) {
    send_to_char("`iThat room doesn't exist.`q\n\r",ch);
    return;
  }

  zone=inzone(vroom);
  if(zone==NOWHERE) {
    send_to_char("`iThe zone for that room doesn't exist.`q\n\r",ch);
    return;
  }

  if(!check_zone_access(ch,zone)) return;

  if(!world[room].dir_option[dir]) {
    send_to_char("`iAn exit doesn't exist in that direction.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "normal")) {
    if(world[room].dir_option[dir]->keyword) {
      free(world[room].dir_option[dir]->keyword);
      world[room].dir_option[dir]->keyword=NULL;
    }
    world[room].dir_option[dir]->exit_info = 0;
    send_to_char("`iExit set to normal.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "door")) {
    if(!world[room].dir_option[dir]->keyword)
      world[room].dir_option[dir]->keyword=str_dup("door");
    world[room].dir_option[dir]->exit_info = EX_ISDOOR;
    send_to_char("`iExit set to a door.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "move")) {
    if(!world[room].dir_option[dir]->keyword)
      world[room].dir_option[dir]->keyword=str_dup("nothing");
    world[room].dir_option[dir]->exit_info = EX_MOVE;
    send_to_char("`iExit set to move.`q.\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "jump")) {
    if(!world[room].dir_option[dir]->keyword)
      world[room].dir_option[dir]->keyword=str_dup("nothing");
    world[room].dir_option[dir]->exit_info = EX_JUMP;
    send_to_char("`iExit set to jump.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "crawl")) {
    if(!world[room].dir_option[dir]->keyword)
      world[room].dir_option[dir]->keyword=str_dup("nothing");
    world[room].dir_option[dir]->exit_info = EX_CRAWL;
    send_to_char("`iExit set to crawl.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "climb")) {
    if(!world[room].dir_option[dir]->keyword)
      world[room].dir_option[dir]->keyword=str_dup("nothing");
    world[room].dir_option[dir]->exit_info = EX_CLIMB;
    send_to_char("`iExit set to climb.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "enter")) {
    if(!world[room].dir_option[dir]->keyword)
      world[room].dir_option[dir]->keyword=str_dup("nothing");
    world[room].dir_option[dir]->exit_info = EX_ENTER;
    send_to_char("`iExit set to enter.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "nopick")) {
    world[room].dir_option[dir]->exit_info = EX_PICKPROOF | EX_ISDOOR;
    send_to_char("`iExit set to nopick.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "lock_10")) {
    world[room].dir_option[dir]->exit_info = EX_LOCK_10 | EX_ISDOOR;
    send_to_char("`iExit set to lock level 10.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "lock_15")) {
    world[room].dir_option[dir]->exit_info = EX_LOCK_15 | EX_ISDOOR;
    send_to_char("`iExit set to lock level 15.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "lock_20")) {
    world[room].dir_option[dir]->exit_info = EX_LOCK_20 | EX_ISDOOR;
    send_to_char("`iExit set to lock level 20.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "lock_25")) {
    world[room].dir_option[dir]->exit_info = EX_LOCK_25 | EX_ISDOOR;
    send_to_char("`iExit set to lock level 25.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "lock_30")) {
    world[room].dir_option[dir]->exit_info = EX_LOCK_30 | EX_ISDOOR;
    send_to_char("`iExit set to lock level 30.`q\n\r",ch);
    return;
  }

  send_to_char(usage,ch);
}

void do_rextra(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command adds/removes an extra description from a room.\n\r\n\r\
  Usage: `krextra`q <room> <add/remove> <keywords> (room is optional)\n\r\
     Ex: `krextra`q 1212 add floor carpet\n\r\
      or `krextra`q rem floor carpet(standing in room 1212)\n\r\
      When removing, the full list of keywords must be used.\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct extra_descr_data *tmp_descr, **prev_descr_ptr, *tmp_descr1;
  int vroom,room,zone,add,found;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) {
    vroom=atoi(arg);
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
  }
  else {
    vroom=CHAR_VIRTUAL_ROOM(ch);
  }

  add=-1;
  if(is_abbrev(arg, "add")) add = 1;
  if(is_abbrev(arg, "remove"))  add = 0;
  if(add==-1) {
    send_to_char(usage,ch);
    return;
  }

  if(!*argument) {
    send_to_char(usage,ch);
    return;
  }

  room=real_room(vroom);
  if(room==NOWHERE || room>top_of_world) {
    send_to_char("`iThat room doesn't exist.`q\n\r",ch);
    return;
  }

  zone=inzone(vroom);
  if(zone==NOWHERE) {
    send_to_char("`iThe zone for that room doesn't exist.`q\n\r",ch);
    return;
  }

  if(!check_zone_access(ch,zone)) return;

  for (;isspace(*argument);argument++);
  if(add) {
    send_to_char("\n\r`iEnter the extra description, terminate with @ on its own line.`q\n\r",ch);
    CREATE(tmp_descr, struct extra_descr_data, 1);
    tmp_descr->keyword = str_dup(argument);
    tmp_descr->next =world[room].ex_description;
    ch->desc->str = &tmp_descr->description;
    ch->desc->max_str = LEN_EXTRADESC_MAX;
    world[room].ex_description = tmp_descr;
    tmp_descr = NULL;
    return;
  }
  else {
    tmp_descr= world[room].ex_description;
    prev_descr_ptr = &world[room].ex_description;
    found=0;
    while(tmp_descr) {
      if(!strcmp(tmp_descr->keyword, argument)) {/*delete that one*/
        *prev_descr_ptr = tmp_descr->next;
        if(tmp_descr->description)
          free(tmp_descr->description);
        if(tmp_descr->keyword)
          free(tmp_descr->keyword);
        tmp_descr1 = tmp_descr->next;
        free(tmp_descr);
        tmp_descr = tmp_descr1;
        send_to_char("Extra description deleted.\n\r",ch);
        found=1;
      }
      else {
        prev_descr_ptr = &tmp_descr->next;
        tmp_descr = tmp_descr->next;
      }
    }
    if(!found) send_to_char("Extra description not found.\n\r",ch);
  }
}

void do_lstat(struct char_data *ch, char *argument, int cmd) {
  struct char_data *i;
  int zone,rzone,count=0;
  char buf[MAX_STRING_LENGTH];
  char usage[] = "Usage: `klstat`q <zone #>\n\r";

  if(!check_olc_access(ch)) return;

  one_argument(argument,buf);

  if (!*buf) {
    zone=world[CHAR_REAL_ROOM(ch)].zone;
  }
  else {
    if(is_number(buf)) {
      zone=atoi(buf);
    }
    else {
      send_to_char("`iZone value is not a valid number.`q\n\r", ch);
      send_to_char(usage,ch);
      return;
    }
  }

  rzone=real_zone(zone);
  if(rzone==-1) {
    send_to_char("`iThat zone doesn't exist.`q\n\r",ch);
    return;
  }
  if(!check_zone_access(ch,zone)) return;

  sprintf(buf, "Mobs in zone number %d:\n\r---------------------\n\r",zone);
  send_to_char(buf, ch);
  for (i = character_list; i ; i = i->next) {
    if (IS_NPC(i) && (world[CHAR_REAL_ROOM(i)].zone==zone)) {
      sprintf(buf, "%-24s [%-5d] in %-40s [%-5d]\n\r", MOB_SHORT(i),mob_proto_table[i->nr].virtual,
              world[CHAR_REAL_ROOM(i)].name,world[CHAR_REAL_ROOM(i)].number);
      send_to_char(buf, ch);
      count++;
    }
  }

  sprintf(buf, "-------------------------------------------------------\n\rTotal: %d mobs loaded.\n\r",count);
  send_to_char(buf, ch);
  return;
}

/* Mob Procedures */
void do_mname(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a mob name.\n\r\n\r\
  Usage: `kmname`q <mob#>\n\r\
     Ex: `kmname`q 3005\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  send_to_char("`iMob name is what a player type to interact with the mob.\n\r\
Enter the mob name, terminate with @@ on the same line.`q\n\r",ch);
  mob_proto_table[rmob].name[0] = 0;
  ch->desc->str = &mob_proto_table[rmob].name;
  ch->desc->max_str = 50;
}

void do_mshort(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a mob short description.\n\r\n\r\
  Usage: `kmshort`q <mob#>\n\r\
     Ex: `kmshort`q 3005\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  send_to_char("`iMob short desc. is what a player sees when the mob does something.\n\r\
Enter the mob short desc., terminate with @@ on the same line.`q\n\r",ch);
  if(!mob_proto_table[rmob].short_descr)
    CREATE(mob_proto_table[rmob].short_descr, char,1);
  mob_proto_table[rmob].short_descr[0] = 0;
  ch->desc->str = &mob_proto_table[rmob].short_descr;
  ch->desc->max_str = 80;
}

void do_mlong(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a mob long description.\n\r\n\r\
  Usage: `kmlong`q <mob#>\n\r\
     Ex: `kmlong`q 3005\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  send_to_char("`iMob long desc. is what a player sees when looking in a room.\n\r\
Enter the mob long desc., terminate with @ a line by itself.`q\n\r",ch);
  if(!mob_proto_table[rmob].long_descr)
    CREATE(mob_proto_table[rmob].long_descr, char,1);
  mob_proto_table[rmob].long_descr[0] = 0;
  ch->desc->str = &mob_proto_table[rmob].long_descr;
  ch->desc->max_str = MAX_STRING_LENGTH;
}

void do_mfull(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a mob full description.\n\r\n\r\
  Usage: `kmfull`q <mob#>\n\r\
     Ex: `kmfull`q 3005\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  send_to_char("`iMob full desc. is what a player sees when looking at the mob.\n\r\
Enter the mob full desc., terminate with @ a line by itself.`q\n\r",ch);
  if(!mob_proto_table[rmob].description)
    CREATE(mob_proto_table[rmob].description, char,1);
  mob_proto_table[rmob].description[0] = 0;
  ch->desc->str = &mob_proto_table[rmob].description;
  ch->desc->max_str = MAX_STRING_LENGTH;
}

void do_mtag(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to add a tagline to a mob.\n\r\n\r\
  Usage: `kmtag`q <mob#> add/remove <tagline>\n\r\
     Ex: `kmtag`q 3005 add ..... the receptionist glows with a bright light.\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct tagline_data *tmp_tag, **prev_tag_ptr, *tmp_tag1;
  int vmob,rmob,zone,add=0;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_abbrev(arg, "add"))
    add = 1;
  if(is_abbrev(arg, "remove"))
    add = 2;
  if(!add || !*argument) {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  switch(add) {
    case 1:
      CREATE(tmp_tag, struct tagline_data, 1);
      tmp_tag->desc = str_dup(argument);
      tmp_tag->next = mob_proto_table[rmob].tagline;
      mob_proto_table[rmob].tagline = tmp_tag;
      tmp_tag = NULL;
      send_to_char("Done.\n\r",ch);
      return;
      break;
    case 2:
      tmp_tag=mob_proto_table[rmob].tagline;
      prev_tag_ptr=&mob_proto_table[rmob].tagline;
      while(tmp_tag) {
        if(!strcmp(tmp_tag->desc, argument)) {/*delete that one*/
          *prev_tag_ptr = tmp_tag->next;
          if(tmp_tag->desc)
            free(tmp_tag->desc);
          tmp_tag1 = tmp_tag->next;
          free(tmp_tag);
          tmp_tag = tmp_tag1;
          send_to_char("Done.\n\r",ch);
          return;
        }
        else {
          prev_tag_ptr = &tmp_tag->next;
          tmp_tag = tmp_tag->next;
        }
      }
      send_to_char("Tagline not found.\n\r",ch);
      return;
      break;
  }
}

void do_msex(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a mob's sex.\n\r\n\r\
  Usage: `kmsex`q <mob#> <male|female|neutral>\n\r\
     Ex: `kmsex`q 3005 female\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_abbrev(arg, "male"))
    mob_proto_table[rmob].sex = 1;
  else if(is_abbrev(arg, "female"))
    mob_proto_table[rmob].sex = 2;
  else if(is_abbrev(arg, "neutral"))
    mob_proto_table[rmob].sex = 0;
  else {
    send_to_char(usage,ch);
    return;
  }
  send_to_char("`iDone`q\n\r",ch);
}

void do_mlevel(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a mob's level.\n\r\n\r\
  Usage: `kmlevel`q <mob#> <level>\n\r\
     Ex: `kmlevel`q 3005 30\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,level;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) level=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  if(level<1 || level>LEVEL_MAX) {
    sprintf(arg,"`iA mob's level must be between 1 and %d.`q\n\r",LEVEL_MAX);
    send_to_char(arg,ch);
    return;
  }
  mob_proto_table[rmob].level=level;
  send_to_char("`iDone`q\n\r",ch);
}

void do_mhitroll(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a mob's hitroll.\n\r\
The hitroll controls how often the mob hits others and\n\r\
a high hitroll of around 30, almost guarantees a hit\n\r\
every time.\n\r\n\r\
  Usage: `kmhitroll`q <mob#> <hitroll>\n\r\
     Ex: `kmhitroll`q 3005 10\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,hitroll;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) hitroll=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  if(hitroll<1 || hitroll >127) {
    send_to_char("`iA mob's hitroll should be positive and less than 127.`q\n\r",ch);
    return;
  }
  mob_proto_table[rmob].hitroll=hitroll;
  send_to_char("`iDone`q\n\r",ch);
}

void do_marmor(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a mob's armor. (Range -250 to 100)\n\r\
The armor controls how often the mob gets hit by others and\n\r\
the lower the number, the harder the mob is to hit.\n\r\n\r\
  Usage: `kmarmor`q <mob#> <armor>\n\r\
     Ex: `kmarmor`q 3005 -50\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,armor;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) armor=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(armor<-32767 || armor>100) {
    send_to_char("`iA mob's armor should be between -32767 and 100.`q\n\r",ch);
    return;
  }
  mob_proto_table[rmob].armor=armor;
  send_to_char("`iDone`q\n\r",ch);
}

void do_malign(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change a mob's alignment.\n\r\
-1000 to -350 is evil; -350 to 350 is neutral; 350 to 1000 is saintly.\n\r\n\r\
  Usage: `kmalign`q <mob#> <alignment>\n\r\
     Ex: `kmalign`q 3005 130\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,align;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) align=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(align<-1000 || align>1000) {
    send_to_char("`iA mob's alignment should be between -1000 and 1000.`q\n\r",ch);
    return;
  }
  mob_proto_table[rmob].alignment=align;
  send_to_char("`iDone`q\n\r",ch);
}

void do_mgold(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change the number of gold coins a mob has.\n\r\
Larger amounts of gold should be reserved for difficult mobs.\n\r\n\r\
  Usage: `kmgold`q <mob#> <gold>\n\r\
     Ex: `kmgold`q 3005 20000\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,gold;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) gold=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(gold<0) {
    send_to_char("`iA mob's gold should be 0 or positive.`q\n\r",ch);
    return;
  }
  mob_proto_table[rmob].gold=gold;
  send_to_char("`iDone`q\n\r",ch);
}

void do_mexp(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change the amount of experience a mob gives.\n\r\
Larger amounts of experience should be reserved for difficult mobs.\n\r\
Also, the experience given to players is 1/3 the mobs experience.\n\r\n\r\
  Usage: `kmexp`q <mob#> <experience>\n\r\
     Ex: `kmexp`q 3005 50000\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,exp;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) exp=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(exp<0) {
    send_to_char("`iA mob's experience should be 0 or positive.`q\n\r",ch);
    return;
  }
  mob_proto_table[rmob].exp=exp;
  send_to_char("`iDone`q\n\r",ch);
}

void do_mpos(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change the mobs position.\n\r\
Positions are as follows:\n\r\
0)  Dead              4)  Sleeping    8)  Standing\n\r\
1)  Mortally Wounded  5)  Resting     9)  Flying\n\r\
2)  Incapacitated     6)  Sitting    10)  Riding\n\r\
3)  Stunned           7)  Fighting   11)  Swimming\n\r\n\r\
  Usage: `kmpos`q <mob#> <pos num>\n\r\
     Ex: `kmpos`q 3005 8\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,pos;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) pos=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(pos<0 || pos>11) {
    send_to_char(usage,ch);
    return;
  }

  mob_proto_table[rmob].position = pos;
  sprintf(arg,"`iPosition changed to %s.`q\n\r",position_types[pos]);
  send_to_char(arg,ch);
}

void do_mdefpos(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change the mobs default position.\n\r\
Default positions are as follows:\n\r\
0)  Dead              4)  Sleeping    8)  Standing\n\r\
1)  Mortally Wounded  5)  Resting     9)  Flying\n\r\
2)  Incapacitated     6)  Sitting    10)  Riding\n\r\
3)  Stunned           7)  Fighting   11)  Swimming\n\r\n\r\
  Usage: `kmdefpos`q <mob#> <defpos num>\n\r\
     Ex: `kmdefpos`q 3005 8\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,pos;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) pos=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(pos<0 || pos>11) {
    send_to_char(usage,ch);
    return;
  }

  mob_proto_table[rmob].default_pos = pos;
  sprintf(arg,"`iDefault position changed to %s.`q\n\r",position_types[pos]);
  send_to_char(arg,ch);
}

void do_mreset(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to reset the mob and start editing it\n\r\
from scratch.\n\r\n\r\
  Usage: `kmreset`q <mob#>\n\r\
     Ex: `kmreset`q 3005\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  mob_proto_table[rmob].name        = str_dup("primal clay blank");
  mob_proto_table[rmob].short_descr = str_dup("blank of primal clay");
  mob_proto_table[rmob].long_descr  = str_dup("A blank of primal clay stands here, waiting to be shaped\n\r");
  mob_proto_table[rmob].description = str_dup("Featureless and non-descript, the blank of primal clay waits for someone to breath life into it.\n\r");

  mob_proto_table[rmob].func = NULL;

  mob_proto_table[rmob].act    = 0;
  mob_proto_table[rmob].number = 0;
  SET_BIT(mob_proto_table[rmob].act, ACT_ISNPC);
  mob_proto_table[rmob].affected_by = 0;
  mob_proto_table[rmob].alignment = 0;
  mob_proto_table[rmob].level = 1;
  mob_proto_table[rmob].hitroll = MIN(mob_proto_table[rmob].level,(20 - 1));
  mob_proto_table[rmob].armor  = 100;
  mob_proto_table[rmob].hp_nodice   = 1;
  mob_proto_table[rmob].hp_sizedice = 1;
  mob_proto_table[rmob].hp_add      = 1;
  mob_proto_table[rmob].damnodice   = 1;
  mob_proto_table[rmob].damsizedice = 1;
  mob_proto_table[rmob].damroll     = 1;
  mob_proto_table[rmob].gold = 1;
  mob_proto_table[rmob].exp = 1;
  mob_proto_table[rmob].position = POSITION_STANDING;
  mob_proto_table[rmob].default_pos = POSITION_STANDING;
  mob_proto_table[rmob].sex = SEX_MALE;
  mob_proto_table[rmob].class = 0;
  mob_proto_table[rmob].immune = 0;
  mob_proto_table[rmob].mana_nodice   = 1;
  mob_proto_table[rmob].mana_sizedice = 1;
  mob_proto_table[rmob].mana_add      = 1;
  mob_proto_table[rmob].no_att = 0;
  mob_proto_table[rmob].hit_type = 0;
  mob_proto_table[rmob].act2=0;
  mob_proto_table[rmob].affected_by2=0;
  mob_proto_table[rmob].immune2=0;
  mob_proto_table[rmob].resist=0;
  mob_proto_table[rmob].skin_value=0;
  mob_proto_table[rmob].skin_vnum[0]=0;
  mob_proto_table[rmob].skin_vnum[1]=0;
  mob_proto_table[rmob].skin_vnum[2]=0;
  mob_proto_table[rmob].skin_vnum[3]=0;
  mob_proto_table[rmob].skin_vnum[4]=0;
  mob_proto_table[rmob].skin_vnum[5]=0;

  send_to_char("`iDone.`q\n\r",ch);
}

void do_mclass(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to change the mobs class.\n\r\
For a list of classes use OLCHELP MOB CLASS.\n\r\n\r\
  Usage: `kmclass`q <mob#> <class num>\n\r\
     Ex: `kmclass`q 3005 2\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,class;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) class=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(class<0 || class>CLASS_MOB_LAST) {
    send_to_char(usage,ch);
    return;
  }

  if(class>11 && class <51) {
    send_to_char(usage,ch);
    return;
  }

  mob_proto_table[rmob].class = class;
  sprintf(arg,"`iClass changed to %s.`q\n\r",npc_class_types[class]);
  send_to_char(arg,ch);
}

void do_maff(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command toggles the mob affects listed on/off.\n\r\n\r\
  Usage: `kmaff`q <mob #> <list of affects>/all/none\n\r\
     Ex: `kmaff`q 3005 `iSANTUARY DUAL`q (caps not important)\n\r\n\r\
Affs are:  `iBLIND, INVISIBLE, DETECT-ALIGNMENT, DETECT-INVISIBLE,\n\r\
           DETECT-MAGIC, SENSE-LIFE, HOLD, SANCTUARY, CURSE, SPHERE,\n\r\
           POISON, PROTECT-EVIL, PARALYSIS, INFRAVISION, STATUE, SLEEP,\n\r\
           DODGE, SNEAK, HIDE, FLY, IMINV, INVUL, DUAL, FURY, PROTECT-GOOD,\n\r\
           TRIPLE, QUAD.`q\n\r\n\r";
  char arg[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,bitpos,aff;
  long bit;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);

  if(!strcmp(arg,"all")) {
    bitpos=old_search_block("\n", 0, strlen("\n"), affected_bits, FALSE);
    mob_proto_table[rmob].affected_by = (1<<(bitpos-1)) -1;
    bitpos=old_search_block("\n", 0, strlen("\n"), affected_bits2, FALSE);
    mob_proto_table[rmob].affected_by2 = (1<<(bitpos-1)) -1;
    mob_proto_table[rmob].affected_by2 -= AFF2_SEVERED;
    mob_proto_table[rmob].affected_by2 -= AFF2_IMMINENT_DEATH;
    send_to_char("`iAll mob affects added.`q\n\r",ch);
    return;
  }

  if(!strcmp(arg,"none")) {
    mob_proto_table[rmob].affected_by = 0;
    mob_proto_table[rmob].affected_by2 = 0;
    send_to_char("`iAll mob affects removed.`q\n\r",ch);
    return;
  }

  while(*arg) {
    aff=1;
    bitpos=old_search_block(string_to_upper(arg), 0, strlen(arg), affected_bits, FALSE);
    if(bitpos==-1) {
      aff=2;
      bitpos=old_search_block(string_to_upper(arg), 0, strlen(arg), affected_bits2, FALSE);
    }

    if(bitpos!=-1) {
      bit=(1<<(bitpos-1));
      if(aff==1) {
        if(IS_SET(mob_proto_table[rmob].affected_by, bit)) {
          REMOVE_BIT(mob_proto_table[rmob].affected_by, bit);
        }
        else {
          SET_BIT(mob_proto_table[rmob].affected_by, bit);
        }
      }
      else {
        if(IS_SET(mob_proto_table[rmob].affected_by2, bit)) {
          REMOVE_BIT(mob_proto_table[rmob].affected_by2, bit);
        }
        else {
          SET_BIT(mob_proto_table[rmob].affected_by2, bit);
        }
      }
    }
    else {
      sprintf(buf,"`kMob affect %s not found.`q\n\r",arg);
      send_to_char(buf,ch);
    }
    argument=one_argument(argument,arg);
  }
  send_to_char("\n\r`iMob affect(s) added/removed.`q\n\r",ch);
}

void do_mact(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command toggles the mob acts listed on/off.\n\r\n\r\
  Usage: `kmact`q <mob #> <list of acts>/all/none\n\r\
     Ex: `kmact`q 3005 `iAGGRESSIVE ARM`q (caps not important)\n\r\n\r\
Acts are: `iSENTINEL, SCAVENGER, ISNPC, NICE-THIEF, AGGRESSIVE, STAY-ZONE,\n\r\
          WIMPY, SUBDUE, RIDE, MOUNT, FLY, AGGWA, AGGTH, AGGCL, AGGMU,\n\r\
          MEMORY, AGGNI, AGGNO, AGGPA, AGGAP, AGGBA, AGGCO, AGGEVIL, AGGGOOD,\n\r\
          AGGNEUT, AGGLEADER, AGGRANDOM, ARM, SHIELD, OPEN-DOOR, NO-TOKEN,\n\r\
          IGNORE-SPHERE.`q\n\r\n\r";
  char arg[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,bitpos,act;
  long bit;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);

  if(!strcmp(arg,"all")) {
    bitpos=old_search_block("\n", 0, strlen("\n"), action_bits, FALSE);
    mob_proto_table[rmob].act = (1<<(bitpos-1)) -1;
    bitpos=old_search_block("\n", 0, strlen("\n"), action_bits2, FALSE);
    mob_proto_table[rmob].act2 = (1<<(bitpos-1)) -1;
    send_to_char("`iAll mob acts added.`q\n\r",ch);
    return;
  }

  if(!strcmp(arg,"none")) {
    mob_proto_table[rmob].act = ACT_ISNPC;
    mob_proto_table[rmob].act2 = 0;
    send_to_char("`iAll mob acts removed.`q\n\r",ch);
    return;
  }

  while(*arg) {
    act=1;
    bitpos=old_search_block(string_to_upper(arg), 0, strlen(arg), action_bits, FALSE);
    if(bitpos==-1) {
      act=2;
      bitpos=old_search_block(string_to_upper(arg), 0, strlen(arg), action_bits2, FALSE);
    }

    if(bitpos!=-1) {
      bit=(1<<(bitpos-1));
      if(act==1) {
        if(IS_SET(mob_proto_table[rmob].act, bit)) {
          REMOVE_BIT(mob_proto_table[rmob].act, bit);
        }
        else {
          SET_BIT(mob_proto_table[rmob].act, bit);
        }
      }
      else {
        if(IS_SET(mob_proto_table[rmob].act2, bit)) {
          REMOVE_BIT(mob_proto_table[rmob].act2, bit);
        }
        else {
          SET_BIT(mob_proto_table[rmob].act2, bit);
        }
      }
    }
    else {
      sprintf(buf,"`kMob act %s not found.`q\n\r",arg);
      send_to_char(buf,ch);
    }
    argument=one_argument(argument,arg);
  }
  send_to_char("\n\r`iMob act(s) added/removed.`q\n\r",ch);
}

void do_mimmune(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command toggles the mob immunities listed on/off.\n\r\n\r\
  Usage: `kmimmune`q <mob #> <list of immunitiess>/all/none\n\r\
     Ex: `kmimmune`q 3005 `iPUMMEL CLAIR`q (caps not important)\n\r\n\r\
Immunes are: `iFIRE, ELECTRIC, POISON, PUMMEL, KICK, PUNCH, SLEEP,\n\r\
             CHARM, BLINDNESS, PARALYSIS, DRAIN, DISEMBOWEL, DISINTEGRATE,\n\r\
             CLAIR, SUMMON, HIT, BLUDGEON, PIERCE, SLASH, WHIP, CLAW,\n\r\
             BITE, STING, CRUSH, HACK, CHOP, SLICE, BACKSTAB, AMBUSH,\n\r\
             ASSAULT, LOCATE, COLD, SOUND, CHEMICAL, ACID, FEAR,\n\r\
             CIRCLE, STEAL.`q\n\r\n\r"; /* Removed Throatstrike and Execute */
  char arg[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,bitpos,immune;
  long bit;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);

  if(!strcmp(arg,"all")) {
    bitpos=old_search_block("\n", 0, strlen("\n"), immune_bits, FALSE);
    mob_proto_table[rmob].immune = (1<<(bitpos-1)) -1;
    bitpos=old_search_block("\n", 0, strlen("\n"), immune_bits2, FALSE);
    mob_proto_table[rmob].immune2 = (1<<(bitpos-1)) -1;
    send_to_char("`iAll mob immunities added.`q\n\r",ch);
    return;
  }

  if(!strcmp(arg,"none")) {
    mob_proto_table[rmob].immune = 0;
    mob_proto_table[rmob].immune2 = 0;
    send_to_char("`iAll mob immunities removed.`q\n\r",ch);
    return;
  }

  while(*arg) {
    immune=1;
    bitpos=old_search_block(string_to_upper(arg), 0, strlen(arg), immune_bits, FALSE);
    if(bitpos==-1) {
      immune=2;
      bitpos=old_search_block(string_to_upper(arg), 0, strlen(arg), immune_bits2, FALSE);
    }

    if(bitpos!=-1) {
      bit=(1<<(bitpos-1));
      if(immune==1) {
        if(IS_SET(mob_proto_table[rmob].immune, bit)) {
          REMOVE_BIT(mob_proto_table[rmob].immune, bit);
        }
        else {
          SET_BIT(mob_proto_table[rmob].immune, bit);
        }
      }
      else {
        if(IS_SET(mob_proto_table[rmob].immune2, bit)) {
          REMOVE_BIT(mob_proto_table[rmob].immune2, bit);
        }
        else {
          SET_BIT(mob_proto_table[rmob].immune2, bit);
        }
      }
    }
    else {
      sprintf(buf,"`kMob immunity %s not found.`q\n\r",arg);
      send_to_char(buf,ch);
    }
    argument=one_argument(argument,arg);
  }
  send_to_char("\n\r`iMob immunities added/removed.`q\n\r",ch);
}

void do_mresist(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command toggles the mob resistances listed on/off.\n\r\n\r\
  Usage: `kmresist`q <mob #> <list of immunitiess>/all/none\n\r\
     Ex: `kmresist`q 3005 `iCOLD SOUND`q (caps not important)\n\r\n\r\
Resistances are: `iPOISON, PHYSICAL, MAGICAL, FIRE, COLD, ELECTRIC,\n\r\
                 SOUND, CHEMICAL, ACID.`q\n\r\n\r";
  char arg[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,bitpos;
  long bit;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);

  if(!strcmp(arg,"all")) {
    bitpos=old_search_block("\n", 0, strlen("\n"), resist_bits, FALSE);
    mob_proto_table[rmob].resist = (1<<(bitpos-1)) -1;
    send_to_char("`iAll mob resistances added.`q\n\r",ch);
    return;
  }

  if(!strcmp(arg,"none")) {
    mob_proto_table[rmob].resist = 0;
    send_to_char("`iAll mob resistances removed.`q\n\r",ch);
    return;
  }

  while(*arg) {
    bitpos=old_search_block(string_to_upper(arg), 0, strlen(arg), resist_bits, FALSE);
    if(bitpos!=-1) {
      bit=(1<<(bitpos-1));
      if(IS_SET(mob_proto_table[rmob].resist, bit)) {
        REMOVE_BIT(mob_proto_table[rmob].resist, bit);
      }
      else {
        SET_BIT(mob_proto_table[rmob].resist, bit);
      }
    }
    else {
      sprintf(buf,"`kMob resist %s not found.`q\n\r",arg);
      send_to_char(buf,ch);
    }
    argument=one_argument(argument,arg);
  }
  send_to_char("\n\r`iMob resistances added/removed.`q\n\r",ch);
}

void do_mdamage(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to set the mobs damage.\n\r\n\r\
  Usage: `kmdamage`q <mob#> <#>d<#>+<#>\n\r\
     Ex: `kmdamage`q 3005 10d20+100\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,d1=0,d2=0,d3=0,min,max,avg;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(!isdigit(arg[0])) {
    send_to_char(usage,ch);
    return;
  }
  sscanf(arg, "%d d%d +%d", &d1, &d2, &d3);

  if(d1 >0 && d2 > 0 && d3 >= 0) {
    if(d3>32767) {
      send_to_char("`iA mob's damroll must be less than 32768, reseting it to 32767.`q\n\r",ch);
      d3=32767;
    }
    mob_proto_table[rmob].damnodice = d1;
    mob_proto_table[rmob].damsizedice = d2;
    mob_proto_table[rmob].damroll = d3;
  }
  else {
    send_to_char(usage,ch);
    return;
  }

  min=d1+d3;
  max=d1*d2+d3;
  avg=d1*(d2+1)/2+d3;
  sprintf(arg,"`iMob Damage: Min: %d Max: %d Avg: %d.`q\n\r",min,max,avg);
  send_to_char(arg,ch);
}

void do_mhps(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to set the mobs hps.\n\r\n\r\
  Usage: `kmhps`q <mob#> <#>d<#>+<#>\n\r\
     Ex: `kmhps`q 3005 10d50+2000\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,d1=0,d2=0,d3=0,min,max,avg;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(!isdigit(arg[0])) {
    send_to_char(usage,ch);
    return;
  }
  sscanf(arg, "%d d%d +%d", &d1, &d2, &d3);

  if(d1 >0 && d2 > 0 && d3 >= 0) {
    mob_proto_table[rmob].hp_nodice = d1;
    mob_proto_table[rmob].hp_sizedice = d2;
    mob_proto_table[rmob].hp_add = d3;
  }
  else {
    send_to_char(usage,ch);
    return;
  }

  min=d1+d3;
  max=d1*d2+d3;
  avg=d1*(d2+1)/2+d3;
  sprintf(arg,"`iMob Hps: Min: %d Max: %d Avg: %d.`q\n\r",min,max,avg);
  send_to_char(arg,ch);
}

void do_mmana(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to set the mobs mana.\n\r\n\r\
  Usage: `kmmana`q <mob#> <#>d<#>+<#>\n\r\
     Ex: `kmmana`q 3005 1d20+100\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,d1=0,d2=0,d3=0,min,max,avg;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(!isdigit(arg[0])) {
    send_to_char(usage,ch);
    return;
  }
  sscanf(arg, "%d d%d +%d", &d1, &d2, &d3);

  if(d1 >0 && d2 > 0 && d3 >= 0) {
    mob_proto_table[rmob].mana_nodice = d1;
    mob_proto_table[rmob].mana_sizedice = d2;
    mob_proto_table[rmob].mana_add = d3;
  }
  else {
    send_to_char(usage,ch);
    return;
  }

  min=d1+d3;
  max=d1*d2+d3;
  avg=d1*(d2+1)/2+d3;
  sprintf(arg,"`iMob Mana: Min: %d Max: %d Avg: %d.`q\n\r",min,max,avg);
  send_to_char(arg,ch);
}

void do_mskin(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to assign a value to a mob's skin and also\n\r\
have objects be found within the corpse after its skinned.\n\r\n\r\
  Usage: `kmskin`q <mob#> value/load1/load2/load3/load4/load5/load6 #\n\r\
     Ex: `kmskin`q 3005 value 20000 (skin is worth 20000)\n\r\
     Ex: `kmskin`q 3005 load3 3052  (might get a recall scroll after skinning\n\r";

  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,tmp,edit;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_abbrev(arg, "value"))
    edit = 0;
  else if(is_abbrev(arg, "load1"))
    edit = 1;
  else if(is_abbrev(arg, "load2"))
    edit = 2;
  else if(is_abbrev(arg, "load3"))
    edit = 3;
  else if(is_abbrev(arg, "load4"))
    edit = 4;
  else if(is_abbrev(arg, "load5"))
    edit = 5;
  else if(is_abbrev(arg, "load6"))
    edit = 6;
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) tmp=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  if(edit==0)
     mob_proto_table[rmob].skin_value=tmp;
   else
     mob_proto_table[rmob].skin_vnum[edit-1]=tmp;
  send_to_char("`iDone`q\n\r",ch);
}


void do_zone(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command sets the zone for all subsequent zone commands.\n\r\n\r\
  Usage: `kzone`q <zone#> (zone must be loaded)\n\r\
     Ex: `kzone`q 12\n\r";
  char arg[MAX_INPUT_LENGTH];
  int zone,vzone;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vzone=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  ch->specials.zone=vzone;
  sprintf(arg,"`iEditting zone: %d - %s`q\n\r",vzone,zone_table[zone].name);
  send_to_char(arg,ch);
}

int zone_assigned(CHAR *ch) {
  if(ch->specials.zone!=-1) return TRUE;
  send_to_char("`iYou do not have a zone assigned for editting - use the zone command.`q\n\r",ch);
  return FALSE;
}

/* ZCMD defined just before rezone */
void fix_mob_resets(int vmob,int j,int max) {
  int i;

  for (i = 0;;i++) {
    if(ZCMD.command == 'S') break;
    switch(ZCMD.command) {
     case 'M':
     case 'F':
     case 'R':
       if(vmob==ZCMD.arg1) ZCMD.arg2=max;
       break;
    }
  }
}

void do_cmddel(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command deletes the indicated command number.\n\r\n\r\
  Usage: `kcmddel`q <cmd#>\n\r\
     Ex: `kcmddel`q 7\n\r";
  char arg[MAX_INPUT_LENGTH];
  int iMaxCommands=0,where,vzone,zone;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) where=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  while (zone_table[zone].cmd[iMaxCommands].command != 'S')
       iMaxCommands++;

  if(where<0 || where >=iMaxCommands) {
    send_to_char("`iThat reset command does not exist.`q\n\r", ch);
    return;
  }

  memmove(&zone_table[zone].cmd[where], &zone_table[zone].cmd[where+1],
          sizeof(struct reset_com) * (iMaxCommands - where));
  iMaxCommands --;
  zone_table[zone].cmd = (struct reset_com*)realloc(zone_table[zone].cmd, sizeof(struct reset_com) * (iMaxCommands+1));

  send_to_char("`iReset command deleted.`q\n\r", ch);
}

void do_cmdlist(CHAR *ch, char *argument, int cmd) {
  char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
  struct string_block sb;
  struct zone_data *zone;
  int tab = 0,i,vzone,zoneNum,cmd_no,mobile, room, object,object_to;

  if(!check_olc_access(ch)) return;
  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zoneNum=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  init_string_block(&sb);
  zone=&zone_table[zoneNum];

  sprintf(buf,"Zone: %d  Name: %s.\n\rCreators: %s\n\r",
          vzone,zone->name,zone->creators);
  append_to_string_block(&sb, buf);

  sprintf(buf,"Creation Date: %s   Modify Date: %s\n\r",
          zone->create_date,zone->mod_date);
  append_to_string_block(&sb,buf);

  sprintf(buf,"  Climate: %d Lifespan: %d Age: %d  Bottom: %d Top: %d\n\r",
          zone->climate,zone->lifespan,zone->age,zone->bottom,zone->top);
  append_to_string_block(&sb,buf);

  sprintf(buf," Multipliers:  Hp Mana Hitroll Damage Armor  Xp Gold Level\n\r");
  append_to_string_block(&sb,buf);
  sprintf(buf,"              %3d  %3d   %3d    %3d    %3d  %3d  %3d  %3d\n\r",
          zone->mult_hp,zone->mult_mana,zone->mult_hitroll,zone->mult_damage,
          zone->mult_armor,zone->mult_xp,zone->mult_gold,zone->mult_level);
  append_to_string_block(&sb,buf);

  switch(zone->reset_mode) {
    case 0:
     append_to_string_block(&sb,"  Zone never resets.\n\r");
     break;
    case 1:
     append_to_string_block(&sb,"  Zone resets when empty.\n\r");
     break;
    case 2:
     append_to_string_block(&sb,"  Zone resets regardless.\n\r");
     break;
    case 3:
     append_to_string_block(&sb,"  Zone reset is blocked.\n\r");
     break;
    case 4:
     append_to_string_block(&sb,"  Zone is locked.\n\r");
     break;
    case 5:
     append_to_string_block(&sb,"  Only doors reset.\n\r");
     break;
    default:
     append_to_string_block(&sb,"  Invalid reset mode!\n\r");
     break;
  }

  for(cmd_no = 0;;cmd_no++) {
    if(zone->cmd[cmd_no].command == 'S') break;

    switch(zone->cmd[cmd_no].command) {
      case 'M': /* read a mobile */
        tab = 2;
        mobile  = real_mobile(zone->cmd[cmd_no].arg1);
        room    = real_room(zone->cmd[cmd_no].arg3);
        sprintf(buf,"%3d) ",cmd_no);
        for(i=0;i<tab;i++)
          strcat(buf," ");
        append_to_string_block(&sb,buf);
        if(mobile != -1 && room != -1 ) {
          sprintf(buf,"Load mob %s (#%d) in %s (#%d).\n\r",mob_proto_table[mobile].name,
                       zone->cmd[cmd_no].arg1, world[room].name, zone->cmd[cmd_no].arg3);
          tab += 2;
          append_to_string_block(&sb,buf);
        }
        else {
          append_to_string_block(&sb,"Error in reset command.\n\r");
        }
        break;

      case 'F': /* add a follower */
        mobile  = real_mobile(zone->cmd[cmd_no].arg1);
        room    = real_room(zone->cmd[cmd_no].arg3);
        sprintf(buf,"%3d) ",cmd_no);
        for(i=0;i<tab;i++)
          strcat(buf," ");
        append_to_string_block(&sb,buf);
        if(mobile != -1 && room != -1 ) {
          sprintf(buf,"Load follower %s (#%d) in %s (#%d).\n\r",mob_proto_table[mobile].name,
                       zone->cmd[cmd_no].arg1, world[room].name, zone->cmd[cmd_no].arg3);
          append_to_string_block(&sb,buf);
        }
        else {
          append_to_string_block(&sb,"Error in reset command.\n\r");
        }
        break;

      case 'R': /* add a mount */
        mobile  = real_mobile(zone->cmd[cmd_no].arg1);
        room    = real_room(zone->cmd[cmd_no].arg3);
        sprintf(buf,"%3d) ",cmd_no);
        for(i=0;i<tab;i++)
          strcat(buf," ");
        append_to_string_block(&sb,buf);
        if(mobile != -1 && room != -1 ) {
          sprintf(buf,"Ride %s (#%d) in %s (#%d).\n\r",mob_proto_table[mobile].name,
                       zone->cmd[cmd_no].arg1, world[room].name, zone->cmd[cmd_no].arg3);
          append_to_string_block(&sb,buf);
        }
        else {
          append_to_string_block(&sb,"Error in reset command.\n\r");
        }
        break;

      case 'T': /* take an object */
        tab = 2;
        object = real_object(zone->cmd[cmd_no].arg1);
        room   = real_room(zone->cmd[cmd_no].arg3);
        sprintf(buf,"%3d) ",cmd_no);
        for(i=0;i<tab;i++)
          strcat(buf," ");
        append_to_string_block(&sb,buf);
        if(object != -1 && room != -1) {
          if(zone->cmd[cmd_no].arg2==0) {
            sprintf(buf,"Take object %s (#%d) in %s (#%d) (All).\n\r",obj_proto_table[object].name,
                       zone->cmd[cmd_no].arg1, world[room].name, zone->cmd[cmd_no].arg3);
          }
          else {
            sprintf(buf,"Take object %s (#%d) in %s (#%d) (Max %d).\n\r",obj_proto_table[object].name,
                       zone->cmd[cmd_no].arg1, world[room].name, zone->cmd[cmd_no].arg3, zone->cmd[cmd_no].arg2);
          }
          tab+=2;
          append_to_string_block(&sb,buf);
        }
        else {
          append_to_string_block(&sb,"Error in reset command.\n\r");
        }
        break;

      case 'O': /* read an object */
        tab = 2;
        object = real_object(zone->cmd[cmd_no].arg1);
        room   = real_room(zone->cmd[cmd_no].arg3);
        sprintf(buf,"%3d) ",cmd_no);
        for(i=0;i<tab;i++)
          strcat(buf," ");
        append_to_string_block(&sb,buf);
        if(object != -1 && room != -1) {
          sprintf(buf,"Load object %s (#%d) in %s (#%d).\n\r",obj_proto_table[object].name,
                       zone->cmd[cmd_no].arg1, world[room].name, zone->cmd[cmd_no].arg3);
          tab+=2;
          append_to_string_block(&sb,buf);
        }
        else {
          append_to_string_block(&sb,"Error in reset command.\n\r");
        }
        break;

      case 'P': /* object to object */
        object = real_object(zone->cmd[cmd_no].arg1);
        object_to = real_object(zone->cmd[cmd_no].arg3);
        sprintf(buf,"%3d) ",cmd_no);
        for(i=0;i<tab;i++)
          strcat(buf," ");
        append_to_string_block(&sb,buf);
        if(object != -1 && object_to != -1) {
          sprintf(buf,"Put object %s (#%d) in it.\n\r",obj_proto_table[object].name,
                        zone->cmd[cmd_no].arg1);
          append_to_string_block(&sb,buf);
        }
        else {
          append_to_string_block(&sb,"Error in reset command.\n\r");
        }
        break;

      case 'G': /* obj_to_char */
        object = real_object(zone->cmd[cmd_no].arg1);
        sprintf(buf,"%3d) ",cmd_no);
        for(i=0;i<tab;i++)
          strcat(buf," ");
        append_to_string_block(&sb,buf);
        if(object != -1) {
          sprintf(buf,"Give %s (#%d).\n\r",obj_proto_table[object].name,zone->cmd[cmd_no].arg1);
          append_to_string_block(&sb,buf);
        }
        else {
          append_to_string_block(&sb,"Error in reset command.\n\r");
        }
        break;

      case 'E': /* object to equipment list */
        object = real_object(zone->cmd[cmd_no].arg1);
        sprintf(buf,"%3d) ",cmd_no);
        for(i=0;i<tab;i++)
          strcat(buf," ");
        append_to_string_block(&sb,buf);
        if(object != -1) {
          sprintf(buf,"Equip %s (#%d) in the %s (#%d) position.\n\r",obj_proto_table[object].name,
                       zone->cmd[cmd_no].arg1,equipment_types[zone->cmd[cmd_no].arg3],zone->cmd[cmd_no].arg3);
          append_to_string_block(&sb,buf);
        }
        else {
          append_to_string_block(&sb,"Error in reset command.\n\r");
        }
        break;

      case 'D': /* set state of door */
        room  = real_room(zone->cmd[cmd_no].arg1);
        tab = 2;
        sprintf(buf,"%3d) ",cmd_no);
        for(i=0;i<tab;i++)
          strcat(buf," ");
        append_to_string_block(&sb,buf);
        if(room == -1) {
          append_to_string_block(&sb,"Error in reset command.\n\r");
        }
        else {
          switch (zone->cmd[cmd_no].arg3) {
            case 0:
              sprintf(buf2, "unlocked and open");
              break;
            case 1:
              sprintf(buf2, "unlocked, but closed");
              break;
            case 2:
              sprintf(buf2, "locked and closed");
              break;
          }

          /* Check on door resets added by Ranger - May 96 */
          if(!world[room].dir_option[zone->cmd[cmd_no].arg2]) {
            sprintf(buf,"Set the door %s from room #%d to (direction doesn't exist).\n\r",
                         dirs[zone->cmd[cmd_no].arg2], zone->cmd[cmd_no].arg1);
            append_to_string_block(&sb,buf);
          }
          else if(world[room].dir_option[zone->cmd[cmd_no].arg2]->to_room_v==0) {
            sprintf(buf,"Set the door %s from room #%d to (no room in that direction).\n\r",
                         dirs[zone->cmd[cmd_no].arg2], zone->cmd[cmd_no].arg1);
            append_to_string_block(&sb,buf);
          }
          /* End of door reset check */
          else {
            sprintf(buf,"Set the door %s from room #%d to #%d to %s.\n\r",
                    dirs[zone->cmd[cmd_no].arg2], zone->cmd[cmd_no].arg1,
                    world[room].dir_option[zone->cmd[cmd_no].arg2]->to_room_v,buf2);
            append_to_string_block(&sb,buf);
          }
        }
        break;

      default:
        sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r",(int)zone, (int)cmd_no);
        append_to_string_block(&sb,buf);
        break;
    }
  }
  page_string_block(&sb, ch);
  destroy_string_block(&sb);
}

void clean_zone(int r_zone);
void do_zreset(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command resets the current zone.\n\r\n\r\
  Usage: `kzreset`q <normal/full/spec>\n\r\
     Ex: `kzreset`q normal\n\r\n\r\
The normal reset applies all reset commands as the game would.\n\r\
The full reset applies all resets, ignoring repop rates.\n\r\
The spec reset will signal MSG_ZONE_RESET first, allowing specs to test.\n\r\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vzone,zone,reset=0;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_abbrev(arg, "normal")) reset=1;
  if(is_abbrev(arg, "full")) reset=2;
  if(is_abbrev(arg, "spec")) reset = 3;
  if(!reset) {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  if(reset==1 || reset == 3) {
    if (reset == 3)
      if (signal_zone(NULL, zone_table[zone].virtual, MSG_ZONE_RESET, ""))
        return;
    reset_zone(zone, FALSE);
    sprintf(arg,"`iCompleted normal reset of zone %d.`q\n\r",vzone);
    send_to_char(arg,ch);
  }
  else {
    clean_zone(zone);
    reset_zone(zone, TRUE);
    sprintf(arg,"`iCompleted full reset of zone %d.`q\n\r",vzone);
    send_to_char(arg,ch);
  }
}

void do_zlife(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command sets the length of time it takes a zone to reset.\n\r\n\r\
  Usage: `kzlife`q <#minutes>\n\r\
     Ex: `kzlife`q 30\n\r\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vzone,zone,life=0;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) life=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  zone_table[zone].lifespan=life;
  sprintf(arg,"`iLife of zone %d set to %d.`q\n\r",vzone,life);
  send_to_char(arg,ch);
}

void do_zmode(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command sets the reset mode of the current zone.\n\r\n\r\
  Usage: `kzmode`q <always|empty|never|block|lock|doors>\n\r\
     Ex: `kzmode`q empty\n\r\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vzone,zone,mode=-1;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_abbrev(arg, "always")) mode = 2;
  if(is_abbrev(arg, "empty")) mode = 1;
  if(is_abbrev(arg, "never")) mode = 0;
  if(is_abbrev(arg, "block")) mode = 3;
  if(is_abbrev(arg, "lock")) mode = 4;
  if(is_abbrev(arg, "doors")) mode = 5;

  if(mode==-1) {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  zone_table[zone].reset_mode=mode;
  send_to_char("`iZone mode changed.`q\n\r",ch);
}

void do_zname(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command sets the name of the current zone.\n\r\n\r\
  Usage: `kzname`q <zone name>\n\r\
     Ex: `kzname`q Northern Midgaard Main City\n\r\n\r";
  int vzone,zone;
  if(!check_olc_access(ch)) return;

  if(GET_LEVEL(ch)<LEVEL_IMP) {
    send_to_char("Please request a name change from an IMP.\n\r",ch);
    return;
  }

  for(;isspace(*argument);argument++);
  if(!*argument) {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  if(zone_table[zone].name)
    free(zone_table[zone].name);
  zone_table[zone].name = str_dup(argument);
  send_to_char("`iZone name changed.`q\n\r",ch);
}

void do_zrange(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command sets the range of the current zone.\n\r\n\r\
  Usage: `kzrange`q <default>/<bottom> <top>\n\r\
     Ex: `kzrange`q 1200 1298\n\r\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vzone,zone,bottom=-1,top=-1,i;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_abbrev(arg, "default")) {
    top = 0;
    bottom = 0;
  }

  if(bottom==-1) {
    if(is_number(arg)) bottom=atoi(arg);
    else {
      send_to_char(usage,ch);
      return;
    }
  }

  if(top==-1) {
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
    if(is_number(arg)) top=atoi(arg);
    else {
      send_to_char(usage,ch);
      return;
    }
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  if(bottom==0) bottom=vzone*100;
  if(top==0) top=bottom+99;
  zone_table[zone].bottom = bottom;
  zone_table[zone].top    = top;
  for(i=real_room(bottom);i<=real_room(top);i++)
    world[i].zone = vzone;
  sprintf(arg,"`iRange of zone %d set as: %d to %d.`q\n\r",vzone,bottom,top);
  send_to_char(arg,ch);
}

void do_mput(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command places a mob in a room.\n\r\n\r\
  Usage: `kmput`q <cmd#> <mob#> <room#> <max#>\n\r\
     Ex: `kmput`q 5 3005 1212 3\n\r\
     The latest <max#> will be applied to any other mob resets of\n\r\
     the same mob vnumber.\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct reset_com tmp_reset;
  int vmob,vroom,where,vzone,zone,real,max;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) where=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_mobile(vmob);
  if(!check_mob(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vroom=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_room(vroom);
  if(!check_room(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) max=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  memset(&tmp_reset, 0, sizeof(struct reset_com));
  tmp_reset.command = 'M';
  tmp_reset.if_flag = 0;
  tmp_reset.arg1    = vmob;
  tmp_reset.arg2    = max;
  tmp_reset.arg3    = vroom;
  if(!add_command(zone, &tmp_reset, where)) {
    send_to_char("`iThere was an error adding that zone command.`q\n\r",ch);
    return;
  }
  fix_mob_resets(vmob,zone,max);
  sprintf(arg,"`iCommand added for mob %d to room %d.`q\n\r",vmob,vroom);
  send_to_char(arg,ch);
}

void do_mfollow(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command adds a mob to follow the preceding mob.\n\r\
The room should be the same as the preceding M command.\n\r\n\r\
  Usage: `kmfollow`q <cmd#> <mob#> <room#> <max#>\n\r\
     Ex: `kmfollow`q 6 3001 1212 3\n\r\
     The latest <max#> will be applied to any other mob resets of\n\r\
     the same mob vnumber.\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct reset_com tmp_reset;
  int vmob,vroom,where,vzone,real,zone,max;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) where=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_mobile(vmob);
  if(!check_mob(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vroom=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_room(vroom);
  if(!check_room(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) max=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  memset(&tmp_reset, 0, sizeof(struct reset_com));
  tmp_reset.command = 'F';
  tmp_reset.if_flag = 1;
  tmp_reset.arg1    = vmob;
  tmp_reset.arg2    = max;
  tmp_reset.arg3    = vroom;
  if(!add_command(zone, &tmp_reset, where)) {
    send_to_char("`iThere was an error adding that zone command.`q\n\r",ch);
    return;
  }
  fix_mob_resets(vmob,zone,max);
  sprintf(arg,"`iCommand added for mob %d to follow preceding mob in room %d.`q\n\r",vmob,vroom);
  send_to_char(arg,ch);
}

void do_mride(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command adds a mob for the preceding mob to ride.\n\r\
The room should be the same as the preceding M command.\n\r\n\r\
  Usage: `kmride`q <cmd#> <mob#> <room#> <max#>\n\r\
     Ex: `kmride`q 7 3001 1212 3\n\r\
     The latest <max#> will be applied to any other mob resets of\n\r\
     the same mob vnumber.\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct reset_com tmp_reset;
  int vmob,vroom,where,vzone,real,zone,max;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) where=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_mobile(vmob);
  if(!check_mob(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vroom=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_room(vroom);
  if(!check_room(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) max=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  memset(&tmp_reset, 0, sizeof(struct reset_com));
  tmp_reset.command = 'R';
  tmp_reset.if_flag = 1;
  tmp_reset.arg1    = vmob;
  tmp_reset.arg2    = max;
  tmp_reset.arg3    = vroom;
  if(!add_command(zone, &tmp_reset, where)) {
    send_to_char("`iThere was an error adding that zone command.`q\n\r",ch);
    return;
  }
  fix_mob_resets(vmob,zone,max);
  sprintf(arg,"`iCommand added for preceding mob to ride mob %d in room %d.`q\n\r",vmob,vroom);
  send_to_char(arg,ch);
}

void do_oput(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command places an object in a room.\n\r\n\r\
  Usage: `koput`q <cmd#> <obj#> <room#>\n\r\
     Ex: `koput`q 5 2096 1212\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct reset_com tmp_reset;
  int vobj,vroom,where,vzone,zone,real;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) where=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vobj=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_object(vobj);
  if(!check_obj(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vroom=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_room(vroom);
  if(!check_room(ch,real)) return;


  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  memset(&tmp_reset, 0, sizeof(struct reset_com));
  tmp_reset.command = 'O';
  tmp_reset.if_flag = 0;
  tmp_reset.arg1    = vobj;
  tmp_reset.arg2    = 0;
  tmp_reset.arg3    = vroom;
  if(!add_command(zone, &tmp_reset, where)) {
    send_to_char("`iThere was an error adding that zone command.`q\n\r",ch);
    return;
  }
  sprintf(arg,"`iCommand added for obj %d to room %d.`q\n\r",vobj,vroom);
  send_to_char(arg,ch);
}

void do_otake(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command takes an object from a room.\n\r\n\r\
  Usage: `kotake`q <cmd#> <obj#> <max#> <room#> (Max=0 for All)\n\r\
     Ex: `kotake`q 5 2096 0 1212\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct reset_com tmp_reset;
  int vobj,vroom,where,vzone,zone,real,max;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) where=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vobj=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_object(vobj);
  if(!check_obj(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) max=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  max=MAX(0,max);

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vroom=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_room(vroom);
  if(!check_room(ch,real)) return;


  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  memset(&tmp_reset, 0, sizeof(struct reset_com));
  tmp_reset.command = 'T';
  tmp_reset.if_flag = 0;
  tmp_reset.arg1    = vobj;
  tmp_reset.arg2    = max;
  tmp_reset.arg3    = vroom;
  if(!add_command(zone, &tmp_reset, where)) {
    send_to_char("`iThere was an error adding that zone command.`q\n\r",ch);
    return;
  }
  sprintf(arg,"`iCommand added for taking obj %d from room %d.`q\n\r",vobj,vroom);
  send_to_char(arg,ch);
}

void do_ogive(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command gives an object to the preceding mob.\n\r\n\r\
  Usage: `kogive`q <cmd#> <obj#>\n\r\
     Ex: `kogive`q 5 2096\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct reset_com tmp_reset;
  int vobj,where,vzone,zone,real;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) where=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vobj=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_object(vobj);
  if(!check_obj(ch,real)) return;

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  memset(&tmp_reset, 0, sizeof(struct reset_com));
  tmp_reset.command = 'G';
  tmp_reset.if_flag = 1;
  tmp_reset.arg1    = vobj;
  tmp_reset.arg2    = 0;
  tmp_reset.arg3    = 0;
  if(!add_command(zone, &tmp_reset, where)) {
    send_to_char("`iThere was an error adding that zone command.`q\n\r",ch);
    return;
  }
  sprintf(arg,"`iCommand added for obj %d given to preceding mob.`q\n\r",vobj);
  send_to_char(arg,ch);
}

void do_oinsert(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command inserts an object to the preceding object.\n\r\n\r\
  Usage: `koinsert`q <cmd#> <obj#> <in obj#>\n\r\
     Ex: `koinsert`q 5 2096 3015\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct reset_com tmp_reset;
  int vobj,vobj2,where,vzone,zone,real;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) where=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vobj=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_object(vobj);
  if(!check_obj(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vobj2=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_object(vobj2);
  if(!check_obj(ch,real)) return;

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  memset(&tmp_reset, 0, sizeof(struct reset_com));
  tmp_reset.command = 'P';
  tmp_reset.if_flag = 1;
  tmp_reset.arg1    = vobj;
  tmp_reset.arg2    = 0;
  tmp_reset.arg3    = vobj2;
  if(!add_command(zone, &tmp_reset, where)) {
    send_to_char("`iThere was an error adding that zone command.`q\n\r",ch);
    return;
  }
  sprintf(arg,"`iCommand added for obj %d inserted in preceding object %d.`q\n\r",vobj,vobj2);
  send_to_char(arg,ch);
}

void do_oequip(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command equips an object to the preceding mob at the given position.\n\r\n\r\
  Usage: `koequip`q <cmd#> <obj#> <pos#>\n\r\
     Ex: `koequip`q 5 2096 12\n\r\n\r\
Postions are:\n\r\
  0) Special(torches)   5) Body     10) arms          15)  left wrist\r\n\
  1) Right finger       6) Head     11) shield        16)  wield\r\n\
  2) Left finger        7) Legs     12) about body    17)  held\r\n\
  3) Neck position A    8) Feet     13) about waist\r\n\
  4) Neck position B    9) Hands    14) right wrist\r\n\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct reset_com tmp_reset;
  int vobj,where,vzone,zone,real,pos;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) where=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vobj=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  real=real_object(vobj);
  if(!check_obj(ch,real)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) pos=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(pos<0 || pos>17) {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  memset(&tmp_reset, 0, sizeof(struct reset_com));
  tmp_reset.command = 'E';
  tmp_reset.if_flag = 1;
  tmp_reset.arg1    = vobj;
  tmp_reset.arg2    = 0;
  tmp_reset.arg3    = pos;
  if(!add_command(zone, &tmp_reset, where)) {
    send_to_char("`iThere was an error adding that zone command.`q\n\r",ch);
    return;
  }
  sprintf(arg,"`iCommand added for obj %d equipped to preceding mob.`q\n\r",vobj);
  send_to_char(arg,ch);
}

void do_dreset(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command sets the door reset of a room.\n\r\n\r\
  Usage: `kdreset`q <room#> <n/s/e/w/u/d> <open/closed/locked>\n\r\
     Ex: `kdreset`q 1212 n open\n\r";
  char arg[MAX_INPUT_LENGTH];
  struct reset_com tmp_reset;
  int vroom,vzone,zone,room,dir=-1,state=-1;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vroom=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  room=real_room(vroom);
  if(!check_room(ch,room)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_abbrev(arg, "north")) dir = 0;
  if(is_abbrev(arg, "east"))  dir = 1;
  if(is_abbrev(arg, "south")) dir = 2;
  if(is_abbrev(arg, "west"))  dir = 3;
  if(is_abbrev(arg, "up"))    dir = 4;
  if(is_abbrev(arg, "down"))  dir = 5;
  if(dir==-1) {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_abbrev(arg, "open"))   state = 0;
  if(is_abbrev(arg, "closed")) state = 1;
  if(is_abbrev(arg, "locked")) state = 2;
  if(state==-1) {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  if(!world[room].dir_option[dir]) {
    sprintf(arg,"`iDirection %s is not defined for room %d.`q\n\r",dirs[dir],vroom);
    send_to_char(arg,ch);
    return;
  }
  if(world[room].dir_option[dir]->to_room_v==0) {
    sprintf(arg,"`iThe room %s from %d is not defined.`q\n\r",dirs[dir],vroom);
    send_to_char(arg,ch);
    return;
  }

  memset(&tmp_reset, 0, sizeof(struct reset_com));
  tmp_reset.command = 'D';
  tmp_reset.if_flag = 1;
  tmp_reset.arg1    = vroom;
  tmp_reset.arg2    = dir;
  tmp_reset.arg3    = state;

  if(!add_command(zone, &tmp_reset, -1)) {
    send_to_char("`iThere was an error adding that zone command.`q\n\r",ch);
    return;
  }
  sprintf(arg,"`iCommand added for door in room %d.`q\n\r",vroom);
  send_to_char(arg,ch);
}

void do_attdel(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to delete a mob attack.\n\r\n\r\
  Usage: `kattdel`q <mob#> <attack#>\n\r\
     Ex: `kattdel`q 3005 1\n\r\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,num,i;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) num=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(num<1 || num>MAX_ATTS-1) {
    send_to_char(usage,ch);
    return;
  }

  if(mob_proto_table[rmob].no_att<1) {
    send_to_char("`iNo mob attacks are defined on this mob.`q\n\r",ch);
    return;
  }
  if(num>mob_proto_table[rmob].no_att) num=mob_proto_table[rmob].no_att;
  for(i=num-1;i<mob_proto_table[rmob].no_att;i++) {
    mob_proto_table[rmob].att_type[i]    = mob_proto_table[rmob].att_type[i+1];
    mob_proto_table[rmob].att_target[i]  = mob_proto_table[rmob].att_target[i+1];
    mob_proto_table[rmob].att_percent[i] = mob_proto_table[rmob].att_percent[i+1];
    mob_proto_table[rmob].att_spell[i]   = mob_proto_table[rmob].att_spell[i+1];
  }
  mob_proto_table[rmob].no_att--;
  sprintf(arg,"`iAttack %d deleted.`q\n\r",num);
  send_to_char(arg,ch);
}

void do_mattack(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to add attacks to a mob.\n\r\n\r\
  Usage: `kmattack`q <mob#> <type> <target> <percent> <spell#(cast/skill)>.\n\r\
     Ex: `kmattack`q 3005 kick group 15\n\r\
     or  `kmattack`q 3005 cast leader 20 34\n\r\n\r\
The following types and targets are available.  You can use the name or the #.\n\r\
Spell number is only required with CAST or SKILL.\n\r\
    Types             Targets    Spells: See 'olchelp SPELL TYPES'\n\r\
 1) CAST           1) BUFFER\n\r\
 2) KICK           2) RAN_GROUP\n\r\
 3) PUMMEL         3) RAN_ROOM\n\r\
 4) PUNCH          4) GROUP\n\r\
 5) BITE           5) ROOM\n\r\
 6) CLAW           6) SELF\n\r\
 7) BASH           7) LEADER\n\r\
 8) TAILSLAM\n\r\
 9) DISARM\n\r\
10) TRAMPLE\n\r\
11) SKILL\n\r\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vmob,rmob,zone,i,target=-1,percent,type=-1,spell=-1;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) vmob=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  rmob=real_mobile(vmob);
  if(!check_mob(ch,rmob)) return;

  zone=inzone(vmob);
  if(!check_zone(ch,zone)) return;
  if(!check_zone_access(ch,zone)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) type=atoi(arg);
  else {
    string_to_lower(arg);
    if(is_abbrev(arg,"cast"))     type=1;
    if(is_abbrev(arg,"kick"))     {type=2;spell=0;}
    if(is_abbrev(arg,"pummel"))   {type=3;spell=0;}
    if(is_abbrev(arg,"punch"))    {type=4;spell=0;}
    if(is_abbrev(arg,"bite"))     {type=5;spell=0;}
    if(is_abbrev(arg,"claw"))     {type=6;spell=0;}
    if(is_abbrev(arg,"bash"))     {type=7;spell=0;}
    if(is_abbrev(arg,"tailslam")) {type=8;spell=0;}
    if(is_abbrev(arg,"disarm"))   {type=9;spell=0;}
    if(is_abbrev(arg,"trample") ) {type=10;spell=0;}
    if(is_abbrev(arg,"skill"))    type=11;
  }
  if(type==-1 || type<1 || type>11) {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) target=atoi(arg);
  else {
    string_to_lower(arg);
    if(is_abbrev(arg,"buffer"))    target=1;
    if(is_abbrev(arg,"ran_group")) target=2;
    if(is_abbrev(arg,"ran_room"))  target=3;
    if(is_abbrev(arg,"group"))     target=4;
    if(is_abbrev(arg,"room"))      target=5;
    if(is_abbrev(arg,"self"))      target=6;
    if(is_abbrev(arg,"leader"))    target=7;

  }
  if(target==-1 || target<1 || target>7) {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_number(arg)) percent=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }
  if(percent<1 || percent >100) {
    send_to_char("`iPercent should be between 0 and 101.`q\n\r",ch);
    return;
  }

  if(spell==-1) {
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
    if(is_number(arg)) spell=atoi(arg);
    else {
      send_to_char(usage,ch);
      return;
    }
  }

  if(mob_proto_table[rmob].no_att+1>MAX_ATTS) {
    send_to_char("`iMaximum number of attacks reached.`q\n\r",ch);
    return;
  }
  i=mob_proto_table[rmob].no_att;
  mob_proto_table[rmob].no_att++;
  mob_proto_table[rmob].att_type[i] = type;
  mob_proto_table[rmob].att_target[i] = target;
  mob_proto_table[rmob].att_percent[i] = percent;
  mob_proto_table[rmob].att_spell[i] = spell;
  send_to_char("`iMob attack added.`q\n\r",ch);
}

void do_rlook(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command displays zone reset commands by room.\n\r\
\n\r\
  Usage: `krlook`q <room#>\n\r\
     Ex: `krlook`q 1212\n\r\
\n\r\
The zone reset command must be in the same zone as the room to be\n\r\
displayed.\n\r";

  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

  if (!check_olc_access(ch)) return;

  argument = one_argument(argument, arg);

  int vroom = 0;

  if (!*arg) {
    vroom = CHAR_VIRTUAL_ROOM(ch);
  }
  else if (is_number(arg)) {
    vroom = atoi(arg);
  }
  else {
    send_to_char(usage, ch);
    return;
  }

  int room = real_room(vroom);

  if (!check_room(ch, room)) return;

  int vzone = inzone(vroom);

  if (!check_zone(ch, vzone)) return;
  if (!check_zone_access(ch, vzone)) return;

  int zone_num = real_zone(vzone);
  struct zone_data *zone = &zone_table[zone_num];

  snprintf(buf, sizeof(buf), "Zone resets for room %d in zone %d.\n\r", vroom, vzone);

  struct string_block sb;
  init_string_block(&sb);

  append_to_string_block(&sb, buf);

  for (int cmd_no = 0, tab = 0, last_mob = 0, last_obj = 0, mob = 0, obj = 0, obj_to = 0; zone->cmd[cmd_no].command != 'S'; cmd_no++) {
    switch (zone->cmd[cmd_no].command) {
      case 'M': /* add a mobile */
        last_mob = 0;
        last_obj = 0;

        if (zone->cmd[cmd_no].arg3 != vroom) break;

        last_mob = 1;

        snprintf(buf, sizeof(buf), "%3d) ", cmd_no);

        tab = 2;
        for (int i = 0; i < tab; i++) {
          strcat(buf, " ");
        }

        append_to_string_block(&sb, buf);

        mob = real_mobile(zone->cmd[cmd_no].arg1);
        room = real_room(zone->cmd[cmd_no].arg3);

        if (mob != -1 && room != -1) {
          snprintf(buf, sizeof(buf), "Load mob %s (#%d) in %s (#%d).\n\r",
            mob_proto_table[mob].name,
            zone->cmd[cmd_no].arg1,
            world[room].name,
            zone->cmd[cmd_no].arg3);

          append_to_string_block(&sb, buf);

          tab += 2;
        }
        else {
          append_to_string_block(&sb, "Error in reset command.\n\r");
        }
        break;

      case 'F': /* add a follower */
        if (!last_mob) break;

        snprintf(buf, sizeof(buf), "%3d) ", cmd_no);

        for (int i = 0; i < tab; i++) {
          strcat(buf, " ");
        }

        append_to_string_block(&sb, buf);

        mob = real_mobile(zone->cmd[cmd_no].arg1);
        room = real_room(zone->cmd[cmd_no].arg3);

        if (mob != -1 && room != -1) {
          snprintf(buf, sizeof(buf), "Load follower %s (#%d) in %s (#%d).\n\r",
            mob_proto_table[mob].name,
            zone->cmd[cmd_no].arg1,
            world[room].name,
            zone->cmd[cmd_no].arg3);

          append_to_string_block(&sb, buf);
        }
        else {
          append_to_string_block(&sb, "Error in reset command.\n\r");
        }
        break;

      case 'R': /* add a mount */
        if (!last_mob) break;

        snprintf(buf, sizeof(buf), "%3d) ", cmd_no);

        for (int i = 0; i < tab; i++) {
          strcat(buf, " ");
        }

        append_to_string_block(&sb, buf);

        mob = real_mobile(zone->cmd[cmd_no].arg1);
        room = real_room(zone->cmd[cmd_no].arg3);

        if (mob != -1 && room != -1) {
          snprintf(buf, sizeof(buf), "Ride %s (#%d) in %s (#%d).\n\r",
            mob_proto_table[mob].name,
            zone->cmd[cmd_no].arg1,
            world[room].name,
            zone->cmd[cmd_no].arg3);

          append_to_string_block(&sb, buf);
        }
        else {
          append_to_string_block(&sb, "Error in reset command.\n\r");
        }
        break;

      case 'O': /* add an object */
        last_mob = 0;
        last_obj = 0;

        if (zone->cmd[cmd_no].arg3 != vroom) break;

        last_obj = 1;

        snprintf(buf, sizeof(buf), "%3d) ", cmd_no);

        tab = 2;
        for (int i = 0; i < tab; i++) {
          strcat(buf, " ");
        }

        append_to_string_block(&sb, buf);

        obj = real_object(zone->cmd[cmd_no].arg1);
        room = real_room(zone->cmd[cmd_no].arg3);

        if (obj != -1 && room != -1) {
          snprintf(buf, sizeof(buf), "Load object %s (#%d) in %s (#%d).\n\r",
            obj_proto_table[obj].name,
            zone->cmd[cmd_no].arg1,
            world[room].name,
            zone->cmd[cmd_no].arg3);

          append_to_string_block(&sb, buf);

          tab += 2;
        }
        else {
          append_to_string_block(&sb, "Error in reset command.\n\r");
        }
        break;

      case 'P': /* put object in object */
        if (!last_obj) break;

        snprintf(buf, sizeof(buf), "%3d) ", cmd_no);

        for (int i = 0; i < tab; i++) {
          strcat(buf, " ");
        }

        append_to_string_block(&sb, buf);

        obj = real_object(zone->cmd[cmd_no].arg1);
        obj_to = real_object(zone->cmd[cmd_no].arg3);

        if (obj != -1 && obj_to != -1) {
          snprintf(buf, sizeof(buf), "Put object %s (#%d) in it.\n\r",
            obj_proto_table[obj].name,
            zone->cmd[cmd_no].arg1);

          append_to_string_block(&sb, buf);
        }
        else {
          append_to_string_block(&sb, "Error in reset command.\n\r");
        }
        break;

      case 'G': /* give object */
        if (!last_mob) break;

        snprintf(buf, sizeof(buf), "%3d) ", cmd_no);

        for (int i = 0; i < tab; i++) {
          strcat(buf, " ");
        }

        append_to_string_block(&sb, buf);

        obj = real_object(zone->cmd[cmd_no].arg1);

        if (obj != -1) {
          snprintf(buf, sizeof(buf), "Give %s (#%d).\n\r",
            obj_proto_table[obj].name,
            zone->cmd[cmd_no].arg1);

          append_to_string_block(&sb, buf);
        }
        else {
          append_to_string_block(&sb, "Error in reset command.\n\r");
        }
        break;

      case 'E': /* equip object */
        if (!last_mob) break;

        snprintf(buf, sizeof(buf), "%3d) ", cmd_no);

        for (int i = 0; i < tab; i++) {
          strcat(buf, " ");
        }

        append_to_string_block(&sb, buf);

        obj = real_object(zone->cmd[cmd_no].arg1);

        if (obj != -1) {
          snprintf(buf, sizeof(buf), "Equip %s (#%d) in the %s (#%d) position.\n\r",
            obj_proto_table[obj].name,
            zone->cmd[cmd_no].arg1,
            equipment_types[zone->cmd[cmd_no].arg3],
            zone->cmd[cmd_no].arg3);

          append_to_string_block(&sb, buf);
        }
        else {
          append_to_string_block(&sb, "Error in reset command.\n\r");
        }
        break;

      case 'D': /* set state of door */
        last_mob = 0;
        last_obj = 0;

        if (zone->cmd[cmd_no].arg1 != vroom) break;

        snprintf(buf, sizeof(buf), "%3d) ", cmd_no);

        tab = 2;
        for (int i = 0; i < tab; i++) {
          strcat(buf, " ");
        }

        append_to_string_block(&sb, buf);

        room = real_room(zone->cmd[cmd_no].arg1);

        if (room != -1) {
          switch (zone->cmd[cmd_no].arg3) {
            case 0:
              snprintf(buf2, sizeof(buf2), "unlocked and open");
              break;
            case 1:
              snprintf(buf2, sizeof(buf2), "unlocked, but closed");
              break;
            case 2:
              snprintf(buf2, sizeof(buf2), "locked and closed");
              break;
            default:
              snprintf(buf2, sizeof(buf2), "unknown door state");
              break;
          }

          if (!world[room].dir_option[zone->cmd[cmd_no].arg2]) {
            snprintf(buf, sizeof(buf), "Set the door %s from room #%d to (direction doesn't exist).\n\r",
              dirs[zone->cmd[cmd_no].arg2],
              zone->cmd[cmd_no].arg1);

            append_to_string_block(&sb, buf);
          }
          else if (world[room].dir_option[zone->cmd[cmd_no].arg2]->to_room_v == 0) {
            snprintf(buf, sizeof(buf), "Set the door %s from room #%d to (no room in that direction).\n\r",
              dirs[zone->cmd[cmd_no].arg2],
              zone->cmd[cmd_no].arg1);

            append_to_string_block(&sb, buf);
          }
          else {
            snprintf(buf, sizeof(buf), "Set the door %s from room #%d to #%d to %s.\n\r",
              dirs[zone->cmd[cmd_no].arg2],
              zone->cmd[cmd_no].arg1,
              world[room].dir_option[zone->cmd[cmd_no].arg2]->to_room_v,
              buf2);

            append_to_string_block(&sb, buf);
          }
        }
        else {
          append_to_string_block(&sb, "Error in reset command.\n\r");
        }
        break;

      default:
        snprintf(buf, sizeof(buf), "Undefined cmd in reset table; zone %d, cmd %d.\n\r", vzone, cmd_no);
        append_to_string_block(&sb, buf);
        break;
    }
  }

  page_string_block(&sb, ch);

  destroy_string_block(&sb);
}

void do_zpurge(CHAR *ch, char *argument, int cmd) {
  char arg[MIL],usage[]="\
This zone command purges all mobs/objs from the specified zone.\n\r\n\r\
  Usage: `kzpurge`q <zone number>\n\r\
     Ex: `kzpurge`q 30\n\r\n\r";
  int i,vzone;
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) vzone=atoi(arg);
  else {
    send_to_char(usage,ch);
    return;
  }

  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  for(i=0;i<=top_of_world;i++) {
    if(vzone==inzone(world[i].number)) {
      for (vict = world[i].people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        if (IS_NPC(vict))
          extract_char(vict);
      }

      for (obj = world[i].contents; obj; obj = next_o) {
        next_o = obj->next_content;
        extract_obj(obj);
      }
    }
  }

  printf_to_char(ch,"`iZone %d purged.`q\n\r",vzone);
}

void do_zcreators(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command sets the names for the creators of the current zone.\n\r\
The creators have the ability to modify the zone items in olc.\n\r\n\r\
  Usage: `kzcreators`q <names> (No spaces between Names)\n\r\
     Ex: `kzcreators`q RangerSumoLiner\n\r\n\r";
  int vzone,zone;
  if(!check_olc_access(ch)) return;

  for(;isspace(*argument);argument++);
  if(!*argument) {
    send_to_char(usage,ch);
    return;
  }

  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  if(zone_table[zone].creators)
    free(zone_table[zone].creators);
  zone_table[zone].creators = str_dup(argument);
  send_to_char("`iZone creators changed.`q\n\r",ch);
}

void do_zfind(CHAR *ch, char *argument, int cmd) {
  char buf[MIL];
  char usage[]="\
This zone command will search the world directory for zones with the\n\r\
given number or the given text. Only 30 maximum file names will be shown.\n\r\n\r\
  Usage: `kzfind`q <#zone/text>\n\r\
     Ex: `kzfind`q #30\n\r\
         `kzfind`q Northern\n\r";
  struct program_info dtail;

  if(!check_olc_access(ch)) return;

  for(;isspace(*argument);argument++);
  if(!*argument) {
    send_to_char(usage,ch);
    return;
  }
  sprintf(buf,"grep -l '%s' world/*.zon > grepedit.txt", argument); /* probably a better way to do this */
  system(buf);

  dtail.args[0]=strdup("tail");
  dtail.args[1]=strdup("-30");
  dtail.args[2]=strdup("grepedit.txt");
  dtail.args[3]=NULL;
  dtail.input=NULL;
  dtail.timeout=5;
  dtail.name=strdup("zfind");
  add_program(dtail, ch);
}

#define MAX_OBJ_NAME_LEN             50
#define MAX_OBJ_SHORT_DESC_LEN       50
#define MAX_OBJ_LONG_DESC_LEN        256
#define MAX_OBJ_ACTION_DESC_LEN      120
#define MAX_OBJ_ACTION_DESC_NOTE_LEN 4096
#define MAX_OBJ_ACTION_NT_DESC_LEN   120
#define MAX_OBJ_CHAR_WEAR_DESC_LEN   120
#define MAX_OBJ_ROOM_WEAR_DESC_LEN   120
#define MAX_OBJ_CHAR_REM_DESC_LEN    120
#define MAX_OBJ_ROOM_REM_DESC_LEN    120

void do_oreset(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command resets an object to the default values.\n\r\
\n\r\
  Usage: `koreset`q <#>\n\r\
     Ex: `koreset`q 3005\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  obj_proto_table[robj] = (const struct obj_proto) { 0 };

  obj_proto_table[robj].virtual = vobj;

  obj_proto_table[robj].name = str_dup("nothing");
  obj_proto_table[robj].short_description = str_dup("a bit of nothing");
  obj_proto_table[robj].description = str_dup("There is a bit of nothing here.");

  SET_BIT(obj_proto_table[robj].obj_flags.wear_flags, ITEM_TAKE);

  send_to_char("Object reset.\n\r", ch);
}

void do_oname(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command changes the name of the specified object.\n\r\
\n\r\
  Usage: `koname`q <#>\n\r\
     Ex: `koname`q 32535\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  send_to_char("Enter object name.  End with an @@ on the same line.\n\r", ch);

  if (obj_proto_table[robj].name) {
    obj_proto_table[robj].name[0] = 0;
  }

  ch->desc->str = &(obj_proto_table[robj].name);
  ch->desc->max_str = MAX_OBJ_NAME_LEN;
}

void do_oshort(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command changes the short description of the specified object.\n\r\
\n\r\
  Usage: `koshort`q <#>\n\r\
     Ex: `koshort`q 3005\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  send_to_char("Enter object short description.  End with an @@ on the same line.\n\r", ch);

  if (obj_proto_table[robj].short_description) {
    obj_proto_table[robj].short_description[0] = 0;
  }

  ch->desc->str = &(obj_proto_table[robj].short_description);
  ch->desc->max_str = MAX_OBJ_SHORT_DESC_LEN;
}

void do_olong(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command changes the long description of the specified object.\n\r\
\n\r\
  Usage: `kolong`q <#>\n\r\
     Ex: `kolong`q 3005\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  send_to_char("Enter object long description.  End with an @@ on the same line.\n\r", ch);

  if (obj_proto_table[robj].description) {
    obj_proto_table[robj].description[0] = 0;
  }

  ch->desc->str = &(obj_proto_table[robj].description);
  ch->desc->max_str = MAX_OBJ_LONG_DESC_LEN;
}

void do_oaction(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command changes the action description of the specified object.\n\r\
\n\r\
  Usage: `koaction`q <#>\n\r\
     Ex: `koaction`q 3005\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  send_to_char("\
Use $n for name/you, $s for his/her/your, $e for he/she/you,\n\r\
$V for victim/you, $m for him/her/you, $r for <name>'s/your.\n\r\
End with an @@ on the same line.  Use @@@ to remove the description.\n\r", ch);

  if (obj_proto_table[robj].action_description) {
    obj_proto_table[robj].action_description[0] = 0;
  }

  ch->desc->str = &(obj_proto_table[robj].action_description);
  ch->desc->max_str = (obj_proto_table[robj].obj_flags.type_flag != ITEM_NOTE) ? MAX_OBJ_ACTION_DESC_LEN : MAX_OBJ_ACTION_DESC_NOTE_LEN;
}

void do_oaction_nt(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command changes the action_nt description of the specified object.\n\r\
\n\r\
  Usage: `koaction_nt`q <#>\n\r\
     Ex: `koaction_nt`q 3005\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  send_to_char("\
Note: You must also define an Action Description.\n\r\
Use $n for name/you, $s for his/her/your, $e for he/she/you,\n\r\
$m for him/her/you, $r for <name>'s/your.\n\r\
End with an @@ on the same line.  Use @@@ to remove the description.\n\r", ch);

  if (obj_proto_table[robj].action_description_nt) {
    obj_proto_table[robj].action_description_nt[0] = 0;
  }

  ch->desc->str = &(obj_proto_table[robj].action_description_nt);
  ch->desc->max_str = MAX_OBJ_ACTION_NT_DESC_LEN;
}

void do_ochar_wear_desc(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command changes the char_wear_desc description of the specified object.\n\r\
\n\r\
  Usage: `kochar_wear_desc`q <#>\n\r\
     Ex: `kochar_wear_desc`q 3005\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  send_to_char("\
Note: Define room_wear_desc as well.\n\r\
Use $n for name/you, $s for his/her/your, $e for he/she/you,\n\r\
$m for him/her/you, $r for <name>'s/your.\n\r\
End with an @@ on the same line.  Use @@@ to remove the description.\n\r", ch);

  if (obj_proto_table[robj].char_wear_desc) {
    obj_proto_table[robj].char_wear_desc[0] = 0;
  }

  ch->desc->str = &(obj_proto_table[robj].char_wear_desc);
  ch->desc->max_str = MAX_OBJ_CHAR_WEAR_DESC_LEN;
}

void do_oroom_wear_desc(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command changes the room_wear_desc description of the specified object.\n\r\
\n\r\
  Usage: `koroom_wear_desc`q <#>\n\r\
     Ex: `koroom_wear_desc`q 3005\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  send_to_char("\
Note: Define char_wear_desc as well.\n\r\
Use $n for name/you, $s for his/her/your, $e for he/she/you,\n\r\
$m for him/her/you, $r for <name>'s/your.\n\r\
End with an @@ on the same line.  Use @@@ to remove the description.\n\r", ch);

  if (obj_proto_table[robj].room_wear_desc) {
    obj_proto_table[robj].room_wear_desc[0] = 0;
  }

  ch->desc->str = &(obj_proto_table[robj].room_wear_desc);
  ch->desc->max_str = MAX_OBJ_ROOM_WEAR_DESC_LEN;
}

void do_ochar_rem_desc(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command changes the char_rem_desc description of the specified object.\n\r\
\n\r\
  Usage: `kochar_rem_desc`q <#>\n\r\
     Ex: `kochar_rem_desc`q 3005\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  send_to_char("\
Note: Define room_rem_desc as well.\n\r\
Use $n for name/you, $s for his/her/your, $e for he/she/you,\n\r\
$m for him/her/you, $r for <name>'s/your.\n\r\
End with an @@ on the same line.  Use @@@ to remove the description.\n\r", ch);

  if (obj_proto_table[robj].char_rem_desc) {
    obj_proto_table[robj].char_rem_desc[0] = 0;
  }

  ch->desc->str = &(obj_proto_table[robj].char_rem_desc);
  ch->desc->max_str = MAX_OBJ_CHAR_REM_DESC_LEN;
}

void do_oroom_rem_desc(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command changes the room_rem_desc description of the specified object.\n\r\
\n\r\
  Usage: `koroom_rem_desc`q <#>\n\r\
     Ex: `koroom_rem_desc`q 3005\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  send_to_char("\
Note: Define char_rem_desc as well.\n\r\
Use $n for name/you, $s for his/her/your, $e for he/she/you,\n\r\
$m for him/her/you, $r for <name>'s/your.\n\r\
End with an @@ on the same line.  Use @@@ to remove the description.\n\r", ch);

  if (obj_proto_table[robj].room_rem_desc) {
    obj_proto_table[robj].room_rem_desc[0] = 0;
  }

  ch->desc->str = &(obj_proto_table[robj].room_rem_desc);
  ch->desc->max_str = MAX_OBJ_ROOM_REM_DESC_LEN;
}

void do_oextra(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command adds/removes an extra description from an object.\n\r\
\n\r\
  Usage: `koextra`q <#> <add/remove> <keywords> (room is optional)\n\r\
     Ex: `koextra`q 3005 add gemstone gem\n\r\
\n\r\
  When removing, the full list of keywords must be used.\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  bool add = TRUE;

  if (is_abbrev(arg, "add")) add = TRUE;
  else if (is_abbrev(arg, "remove"))  add = FALSE;
  else {
    send_to_char(usage, ch);
    return;
  }

  if (!*argument) {
    send_to_char(usage, ch);
    return;
  }

  for (; isspace(*argument); argument++);

  if (add) {
    send_to_char("\n\r`iEnter the extra description, terminate with @ on its own line.`q\n\r", ch);

    struct extra_descr_data *tmp_descr = NULL;
    CREATE(tmp_descr, struct extra_descr_data, 1);
    tmp_descr->keyword = str_dup(argument);
    tmp_descr->next = obj_proto_table[robj].ex_description;
    ch->desc->str = &tmp_descr->description;
    ch->desc->max_str = LEN_EXTRADESC_MAX;
    obj_proto_table[robj].ex_description = tmp_descr;
    tmp_descr = NULL;
  }
  else {
    struct extra_descr_data *tmp_descr = NULL;
    tmp_descr = obj_proto_table[robj].ex_description;
    struct extra_descr_data **prev_descr_ptr;
    prev_descr_ptr = &obj_proto_table[robj].ex_description;
    bool found = FALSE;
    while (tmp_descr) {
      if (!strcmp(tmp_descr->keyword, argument)) {
        *prev_descr_ptr = tmp_descr->next;
        if (tmp_descr->description) {
          free(tmp_descr->description);
        }
        if (tmp_descr->keyword) {
          free(tmp_descr->keyword);
        }
        struct extra_descr_data *tmp_descr_next = NULL;
        tmp_descr_next = tmp_descr->next;
        free(tmp_descr);
        tmp_descr = tmp_descr_next;
        found = TRUE;

        send_to_char("Extra description deleted.\n\r", ch);
      }
      else {
        prev_descr_ptr = &tmp_descr->next;
        tmp_descr = tmp_descr->next;
      }
    }
    if (!found) {
      send_to_char("Extra description not found.\n\r", ch);
    }
  }
}

void do_otype(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command sets the object's type.\n\r\
\n\r\
  Usage: `kotype`q <#> <type>\n\r\
     Ex: `kotype`q 3005 `iSCROLL`q (caps not important)\n\r\
\n\r\
Types are:\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    print_bits_to_char(item_types, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    print_bits_to_char(item_types, ch);
    return;
  }

  int type = old_search_block(arg, 0, strlen(arg), item_types, FALSE);

  if (type == -1) {
    char buf[MSL];
    snprintf(buf, sizeof(buf), "`kObject type %s not found.`q\n\r", arg);
    send_to_char(buf, ch);
    return;
  }

  obj_proto_table[robj].obj_flags.type_flag = (type - 1);

  send_to_char("\n\r`iObject type set.`q\n\r", ch);
}

void do_owear(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command toggles the object wear flags for an object on/off.\n\r\
\n\r\
  Usage: `kowear`q <#> <list of flags>/all/none\n\r\
     Ex: `kowear`q 3005 `iTAKE BODY`q (caps not important)\n\r\
\n\r\
Wear flags are:\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    print_bits_to_char(wear_bits, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!strcmp(arg, "all")) {
    int bit_num = 0;
    for (bit_num = 0; *wear_bits[bit_num] != '\n'; bit_num++);
    if (bit_num) obj_proto_table[robj].obj_flags.wear_flags = ((1 << (bit_num)) - 1);

    send_to_char("`iAll object wear flags added.`q\n\r", ch);
    return;
  }

  if (!strcmp(arg, "none")) {
    obj_proto_table[robj].obj_flags.wear_flags = 0;

    send_to_char("`iAll object wear flags removed.`q\n\r", ch);
    return;
  }

  while (*arg) {
    int bit_pos = old_search_block(string_to_upper(arg), 0, strlen(arg), wear_bits, FALSE);

    if (bit_pos != -1) {
      unsigned long bit = (1 << (bit_pos - 1));

      if (IS_SET(obj_proto_table[robj].obj_flags.wear_flags, bit)) {
        REMOVE_BIT(obj_proto_table[robj].obj_flags.wear_flags, bit);
      }
      else {
        SET_BIT(obj_proto_table[robj].obj_flags.wear_flags, bit);
      }
    }
    else {
      char buf[MSL];

      snprintf(buf, sizeof(buf), "`kObject wear flag %s not found.`q\n\r", arg);

      send_to_char(buf, ch);
    }

    argument = one_argument(argument, arg);
  }

  send_to_char("\n\r`iObject wear flag(s) added/removed.`q\n\r", ch);
}

void do_oflags(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command toggles the object flags for an object on/off.\n\r\
\n\r\
  Usage: `koflags`q <#> <list of flags>/all/none\n\r\
     Ex: `koflags`q 3005 `iGLOW HUM`q (caps not important)\n\r\
\n\r\
Flags are:\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    print_bits_to_char(extra_bits, ch);
    print_bits_to_char(extra_bits2, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!strcmp(arg, "all")) {
    int bit_num = 0;
    for (bit_num = 0; *extra_bits[bit_num] != '\n'; bit_num++);
    if (bit_num) obj_proto_table[robj].obj_flags.extra_flags = ((1 << (bit_num)) - 1);

    int bit_num2 = 0;
    for (bit_num2 = 0; *extra_bits2[bit_num2] != '\n'; bit_num2++);
    if (bit_num2) obj_proto_table[robj].obj_flags.extra_flags2 = ((1 << (bit_num2)) - 1);

    send_to_char("`iAll object flags added.`q\n\r", ch);
    return;
  }

  if (!strcmp(arg, "none")) {
    obj_proto_table[robj].obj_flags.extra_flags = 0;
    obj_proto_table[robj].obj_flags.extra_flags2 = 0;

    send_to_char("`iAll object flags removed.`q\n\r", ch);
    return;
  }

  while (*arg) {
    int bit_pos = old_search_block(string_to_upper(arg), 0, strlen(arg), extra_bits, FALSE);

    if (bit_pos != -1) {
      unsigned long bit = (1 << (bit_pos - 1));

      if (IS_SET(obj_proto_table[robj].obj_flags.extra_flags, bit)) {
        REMOVE_BIT(obj_proto_table[robj].obj_flags.extra_flags, bit);
      }
      else {
        SET_BIT(obj_proto_table[robj].obj_flags.extra_flags, bit);
      }
    }
    else {
      int bit_pos2 = old_search_block(string_to_upper(arg), 0, strlen(arg), extra_bits2, FALSE);

      if (bit_pos2 != -1) {
        unsigned long bit2 = (1 << (bit_pos2 - 1));

        if (IS_SET(obj_proto_table[robj].obj_flags.extra_flags2, bit2)) {
          REMOVE_BIT(obj_proto_table[robj].obj_flags.extra_flags2, bit2);
        }
        else {
          SET_BIT(obj_proto_table[robj].obj_flags.extra_flags2, bit2);
        }
      }
      else {
        char buf[MSL];

        snprintf(buf, sizeof(buf), "`kObject flag %s not found.`q\n\r", arg);

        send_to_char(buf, ch);
      }
    }

    argument = one_argument(argument, arg);
  }

  send_to_char("\n\r`iObject flag(s) added/removed.`q\n\r", ch);
}

void do_obitvect(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command toggles the object bits for an object on/off.\n\r\
\n\r\
  Usage: `kobitvect`q <#> <list of flags>/all/none\n\r\
     Ex: `kobitvect`q 3005 `iINFRAVISION FLY`q (caps not important)\n\r\
\n\r\
Bits are:\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    print_bits_to_char(affected_bits, ch);
    print_bits_to_char(affected_bits2, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!strcmp(arg, "all")) {
    int bit_num = 0;
    for (bit_num = 0; *affected_bits[bit_num] != '\n'; bit_num++);
    if (bit_num) obj_proto_table[robj].obj_flags.bitvector = ((1 << (bit_num)) - 1);

    int bit_num2 = 0;
    for (bit_num2 = 0; *affected_bits2[bit_num2] != '\n'; bit_num2++);
    if (bit_num2) obj_proto_table[robj].obj_flags.bitvector2 = ((1 << (bit_num2)) - 1);

    send_to_char("`iAll object bits added.`q\n\r", ch);
    return;
  }

  if (!strcmp(arg, "none")) {
    obj_proto_table[robj].obj_flags.bitvector = 0;
    obj_proto_table[robj].obj_flags.bitvector2 = 0;

    send_to_char("`iAll object bits removed.`q\n\r", ch);
    return;
  }

  while (*arg) {
    int bit_pos = old_search_block(string_to_upper(arg), 0, strlen(arg), affected_bits, FALSE);

    if (bit_pos != -1) {
      unsigned long bit = (1 << (bit_pos - 1));

      if (IS_SET(obj_proto_table[robj].obj_flags.bitvector, bit)) {
        REMOVE_BIT(obj_proto_table[robj].obj_flags.bitvector, bit);
      }
      else {
        SET_BIT(obj_proto_table[robj].obj_flags.bitvector, bit);
      }
    }
    else {
      int bit_pos2 = old_search_block(string_to_upper(arg), 0, strlen(arg), affected_bits2, FALSE);

      if (bit_pos2 != -1) {
        unsigned long bit2 = (1 << (bit_pos2 - 1));

        if (IS_SET(obj_proto_table[robj].obj_flags.bitvector2, bit2)) {
          REMOVE_BIT(obj_proto_table[robj].obj_flags.bitvector2, bit2);
        }
        else {
          SET_BIT(obj_proto_table[robj].obj_flags.bitvector2, bit2);
        }
      }
      else {
        char buf[MSL];

        snprintf(buf, sizeof(buf), "`kObject bit %s not found.`q\n\r", arg);

        send_to_char(buf, ch);
      }
    }

    argument = one_argument(argument, arg);
  }

  send_to_char("\n\r`iObject bit(s) added/removed.`q\n\r", ch);
}

void do_oweight(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command allows you to change an object's weight.\n\r\
\n\r\
  Usage: `koweight`q <#> <weight>\n\r\
     Ex: `koweight`q 3005 10\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int weight = atoi(arg);

  obj_proto_table[robj].obj_flags.weight = weight;

  send_to_char("`iObject weight set.`q\n\r", ch);
}

void do_ocost(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command allows you to change an object's cost.\n\r\
\n\r\
  Usage: `kocost`q <#> <cost>\n\r\
     Ex: `kocost`q 3005 1000\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int cost = atoi(arg);

  obj_proto_table[robj].obj_flags.cost = cost;

  send_to_char("`iObject cost set.`q\n\r", ch);
}

void do_orent(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command allows you to change an object's rent cost.\n\r\
\n\r\
  Usage: `korent`q <#> <rent>\n\r\
     Ex: `korent`q 3005 100\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int rent = atoi(arg);

  obj_proto_table[robj].obj_flags.cost_per_day = rent;

  send_to_char("`iObject rent cost set.`q\n\r", ch);
}

void do_otimer(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command allows you to change an object's timer.\n\r\
\n\r\
  Usage: `kotimer`q <#> <timer>\n\r\
     Ex: `kotimer`q 3005 30\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int timer = atoi(arg);

  obj_proto_table[robj].obj_flags.timer = timer;

  send_to_char("`iObject timer set.`q\n\r", ch);
}

void do_orepop(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command allows you to change an object's repop percent.\n\r\
\n\r\
  Usage: `korepop`q <#> <repop>\n\r\
     Ex: `korepop`q 3005 60\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int repop = atoi(arg);

  obj_proto_table[robj].obj_flags.repop_percent = repop;

  send_to_char("`iObject repop percent set.`q\n\r", ch);
}

void do_osubclass_res(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command toggles the object subclass restrictions for an object on/off.\n\r\
\n\r\
  Usage: `kosubclass_res`q <#> <list of flags>/all/none\n\r\
     Ex: `kosubclass_res`q 3005 `iANTI_ENCHANTER ANTI_DEFILER`q (caps not important)\n\r\
\n\r\
Subclass restrictions are:\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    print_bits_to_char(subclass_res_bits, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!strcmp(arg, "all")) {
    int bit_num = 0;
    for (bit_num = 0; *subclass_res_bits[bit_num] != '\n'; bit_num++);
    if (bit_num) obj_proto_table[robj].obj_flags.subclass_res = ((1 << (bit_num)) - 1);

    send_to_char("`iAll object subclass restriction flags added.`q\n\r", ch);
    return;
  }

  if (!strcmp(arg, "none")) {
    obj_proto_table[robj].obj_flags.subclass_res = 0;

    send_to_char("`iAll object subclass restriction flags removed.`q\n\r", ch);
    return;
  }

  while (*arg) {
    int bit_pos = old_search_block(string_to_upper(arg), 0, strlen(arg), subclass_res_bits, FALSE);

    if (bit_pos != -1) {
      unsigned long bit = (1 << (bit_pos - 1));

      if (IS_SET(obj_proto_table[robj].obj_flags.subclass_res, bit)) {
        REMOVE_BIT(obj_proto_table[robj].obj_flags.subclass_res, bit);
      }
      else {
        SET_BIT(obj_proto_table[robj].obj_flags.subclass_res, bit);
      }
    }
    else {
      char buf[MSL];

      snprintf(buf, sizeof(buf), "`kObject subclass restriction %s not found.`q\n\r", arg);

      send_to_char(buf, ch);
    }

    argument = one_argument(argument, arg);
  }

  send_to_char("\n\r`iObject subclass restriction(s) added/removed.`q\n\r", ch);
}

void do_omaterial(CHAR *ch, char *argument, int cmd) {
  send_to_char("`iObject material not implemented.`q\n\r", ch);
}

void do_oaffects(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command adds/removes affects from an object.\n\r\
\n\r\
  Usage: `koaffects`q <#> <add/remove> <affect> <value>\n\r\
     Ex: `koaffects`q 3005 add mana 10\n\r\
\n\r\
Affects are:\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    print_bits_to_char(apply_types, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    print_bits_to_char(apply_types, ch);
    return;
  }

  bool add = TRUE;

  if (is_abbrev(arg, "add")) add = TRUE;
  else if (is_abbrev(arg, "remove"))  add = FALSE;
  else {
    send_to_char(usage, ch);
    print_bits_to_char(apply_types, ch);
    return;
  }

  char location[MIL];

  argument = one_argument(argument, location);

  if (!*location) {
    send_to_char(usage, ch);
    print_bits_to_char(apply_types, ch);
    return;
  }

  if (add) {
    argument = one_argument(argument, arg);

    if (!*arg || !is_number(arg)) {
      send_to_char(usage, ch);
      print_bits_to_char(apply_types, ch);
      return;
    }

    int value = atoi(arg);

    int affect_num = old_search_block(string_to_upper(location), 0, strlen(location), apply_types, FALSE);

    if (affect_num != -1) {
      if (obj_proto_table[robj].affected[0].location == 0) {
        obj_proto_table[robj].affected[0].location = (affect_num - 1);
        obj_proto_table[robj].affected[0].modifier = value;
      }
      else if (obj_proto_table[robj].affected[1].location == 0) {
        obj_proto_table[robj].affected[1].location = (affect_num - 1);
        obj_proto_table[robj].affected[1].modifier = value;
      }
      else if (obj_proto_table[robj].affected[2].location == 0) {
        obj_proto_table[robj].affected[2].location = (affect_num - 1);
        obj_proto_table[robj].affected[2].modifier = value;
      }
      else {
        char buf[MSL];
        snprintf(buf, sizeof(buf), "`kMax of %d AFFECTS on each object.`q\n\r", MAX_OBJ_AFFECT);
        send_to_char(buf, ch);
      }
    }
    else {
      char buf[MSL];
      snprintf(buf, sizeof(buf), "`kAffect %s does not exist.`q\n\r", location);
      send_to_char(buf, ch);
      send_to_char("\n\rAffects are:\n\r\n\r", ch);
      print_bits_to_char(apply_types, ch);
    }
  }
  else {
    while (*location) {
      int affect_num = old_search_block(string_to_upper(location), 0, strlen(location), apply_types, FALSE);

      if ((affect_num - 1) == obj_proto_table[robj].affected[0].location) {
        obj_proto_table[robj].affected[0].location = 0;
        obj_proto_table[robj].affected[0].modifier = 0;
      }
      else if ((affect_num - 1) == obj_proto_table[robj].affected[1].location) {
        obj_proto_table[robj].affected[1].location = 0;
        obj_proto_table[robj].affected[1].modifier = 0;
      }
      else if ((affect_num - 1) == obj_proto_table[robj].affected[2].location) {
        obj_proto_table[robj].affected[2].location = 0;
        obj_proto_table[robj].affected[2].modifier = 0;
      }
      else {
        char buf[MSL];
        snprintf(buf, sizeof(buf), "`kAffect %s did not exist on object.`q\n\r", location);
        send_to_char(buf, ch);
      }

      argument = one_argument(argument, location);
    }
  }

  send_to_char("\n\r`iObject affect(s) added/removed.`q\n\r", ch);
}

void do_ovalues(CHAR *ch, char *argument, int cmd) {
  char usage[] = "\
This command modifies the values of an object.\n\r\
\n\r\
  Usage: `kovalues`q <#> <value 0> <value 1> <value 2> <value 3>\n\r\
     Ex: `kovalues`q 3005 10 0 0 0\n\r\
\n\r\
   Note: You must specify all values, even if they are unused.\n\r\
\n\r";

  if (!check_olc_access(ch)) return;

  char arg[MIL];

  argument = one_argument(argument, arg);

  if (!*arg || !is_number(arg)) {
    send_to_char(usage, ch);
    return;
  }

  int vobj = atoi(arg);

  int robj = real_object(vobj);

  if (!check_obj(ch, robj)) return;

  int zone = inzone(vobj);

  if (!check_zone(ch, zone)) return;
  if (!check_zone_access(ch, zone)) return;

  int val[4] = { 0 };

  for (int i = 0; i < 4; i++) {
    argument = one_argument(argument, arg);

    if (!*arg || !is_number(arg)) {
      goto print_item_value_info;
    }

    val[i] = atoi(arg);
  }

  obj_proto_table[robj].obj_flags.value[0] = val[0];
  obj_proto_table[robj].obj_flags.value[1] = val[1];
  obj_proto_table[robj].obj_flags.value[2] = val[2];
  obj_proto_table[robj].obj_flags.value[3] = val[3];

  send_to_char("`iObject value(s) set.`q\n\r", ch);
  return;

print_item_value_info:
  switch (obj_proto_table[robj].obj_flags.type_flag) {
    case ITEM_LIGHT:
      send_to_char("For light sources: <color> <type> <hours> <not used>\n\r", ch);
      break;
    case ITEM_RECIPE:
      send_to_char("For Recipes: <Creates> <Requires> <Requires> <Requires> (-1 for none)\n\r", ch);
      break;
    case ITEM_AQ_ORDER:
      send_to_char("For AQ Orders: <Requires> <Requires> <Requires> <Requires> (-1 for none)\n\r", ch);
      break;
    case ITEM_SCROLL:
    case ITEM_POTION:
      send_to_char("For Scrolls and Potions: <Level> <Spell1|0> <Spell2|0> <Spell3|0>\n\r", ch);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      send_to_char("For Staves and Wands: <Level> <Max Charges> <Charges Left> <Spell>\n\r", ch);
      break;
    case ITEM_2HWEAPON:
    case ITEM_WEAPON:
      send_to_char("For weapons: <olchelp weapon attacks> <dice damage> <dice size> <type>\n\r", ch);
      break;
    case ITEM_FIREWEAPON:
      send_to_char("For guns: <license number> <bullets left> <dam dice number> <dam dice size>\n\r", ch);
      break;
    case ITEM_MISSILE:
      send_to_char("For thrown weapons: <dam dice number> <dam dice size> <unused> <unused>\n\r", ch);
      break;
    case ITEM_ARMOR:
      send_to_char("For armor: <AC apply (positive is better)> <unused> <unused> <unused>\n\r", ch);
      break;
    case ITEM_TRAP:
      send_to_char("For traps: <spell> <damage> <unused> <unused>\n\r", ch);
      break;
    case ITEM_CONTAINER:
      send_to_char("For containers: <max contains> <how locked> <key #> <corpse>\n\r", ch);
      break;
    case ITEM_NOTE:
      send_to_char("For notes: <tongue> <unused> <unused> <unused>\n\r", ch);
      break;
    case ITEM_DRINKCON:
      send_to_char("For drink containers: <max contains> <current contains> <liquid> <poisoned (0 = False, 1 = True)>\n\r", ch);
      break;
    case ITEM_BULLET:
      send_to_char("For bullets: <unused> <unused> <gun #> <unused>\n\r", ch);
      break;
    case ITEM_KEY:
      send_to_char("For keys: <keytype> <unused> <unused> <unused>\n\r", ch);
      break;
    case ITEM_FOOD:
      send_to_char("For food: <feeds> <unused> <unused> <poisoned (0 = False, 1 = True)>\n\r", ch);
      break;
    case ITEM_MONEY:
      send_to_char("For money: <coins> <unused> <unused> <unused>\n\r", ch);
      break;
    case ITEM_LOCKPICK:
      send_to_char("For lockpicks: <# picks> <max # picks> <unused> <unused>\n\r", ch);
      break;
    case ITEM_BOARD:
      send_to_char("For boards: <min read level> <min write level> <min remove level> <unused>\n\r", ch);
      break;
    case ITEM_SC_TOKEN:
      send_to_char("For subclass tokens: <Subclass Points Given> <unused> <unused> <unused>\n\r", ch);
      break;
    case ITEM_SKIN:
      send_to_char("For skins: <all unused> Use COST for value of skin\n\r", ch);
      break;
  }
}

void do_zmult(CHAR *ch, char *argument, int cmd) {
  char usage[]="\
This zone command sets multipliers for mob stats.  It can be used to quickly\n\r\
update the mob stats of a zone instead of individually changing each mob.\n\r\
The original mob stats do not change.\n\r\n\r\
  Usage: `kzmult`q hp/mana/hitroll/damage/armor/xp/gold/level <#>\n\r\
     Ex: `kzmult`q hp 150\n\r\n\r\
The multiplier is a percentage. A number of 50 would multiply the stat by 0.50\n\r\
a number of 150 would multiply the stat by 1.5.  To use original mob stats set the\n\r\
multiplier to 100.\n\r";
  char arg[MAX_INPUT_LENGTH];
  int vzone,zone,stat=0,mult;
  if(!check_olc_access(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_abbrev(arg, "hp")) stat = 1;
  if(is_abbrev(arg, "mana")) stat = 2;
  if(is_abbrev(arg, "hitroll")) stat = 3;
  if(is_abbrev(arg, "damage")) stat = 4;
  if(is_abbrev(arg, "armor")) stat = 5;
  if(is_abbrev(arg, "xp")) stat = 6;
  if(is_abbrev(arg, "gold")) stat = 7;
  if(is_abbrev(arg, "level")) stat = 8;

  if(stat==0) {
    send_to_char(usage,ch);
    return;
  }

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_number(arg)) {
    mult=atoi(arg);
  }
  else {
    send_to_char(usage,ch);
    return;
  }

  if(mult<0) {
    send_to_char("The multiplier must be positive.\n\r",ch);
    return;
  }


  if(!zone_assigned(ch)) return;
  vzone=ch->specials.zone;
  zone=real_zone(vzone);
  if(!check_zone(ch,vzone)) return;
  if(!check_zone_access(ch,vzone)) return;

  switch (stat) {
    case 1: /* hp */
      zone_table[zone].mult_hp=mult;
      printf_to_char(ch,"Zone Hp multiplier set to %d.\n\r",mult);
      return;
    case 2: /* mana */
      zone_table[zone].mult_mana=mult;
      printf_to_char(ch,"Zone Mana multiplier set to %d.\n\r",mult);
      return;
    case 3: /* hitroll */
      zone_table[zone].mult_hitroll=mult;
      printf_to_char(ch,"Zone Hitroll multiplier set to %d.\n\r",mult);
      return;
    case 4: /* damage */
      zone_table[zone].mult_damage=mult;
      printf_to_char(ch,"Zone Damage multiplier set to %d.\n\r",mult);
      return;
    case 5: /* armor */
      zone_table[zone].mult_armor=mult;
      printf_to_char(ch,"Zone Armor multiplier set to %d.\n\r",mult);
      return;
    case 6: /* xp */
      zone_table[zone].mult_xp=mult;
      printf_to_char(ch,"Zone Xp multiplier set to %d.\n\r",mult);
      return;
    case 7: /* gold */
      zone_table[zone].mult_gold=mult;
      printf_to_char(ch,"Zone Gold multiplier set to %d.\n\r",mult);
      return;
    case 8: /* level */
      zone_table[zone].mult_level=mult;
      printf_to_char(ch,"Zone Level multiplier set to %d.\n\r",mult);
      return;
  }
}
