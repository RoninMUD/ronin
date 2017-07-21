/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "utility.h"
#include "cmd.h"
#include "fight.h"
#include "spec.clan.h"
#include "interpreter.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct room_data *world;              /* dyn alloc'ed array of rooms     */
int top_of_world = -1;                 /* ref to the top element of world */
int top_of_flying = 0;
int *flying_rooms;                     /* list of flying rooms */
int top_of_mobt = -1;                  /* top of mobile index table       */
int top_of_objt = -1;                  /* top of object index table       */
int top_of_zone_table = -1;
int top_of_helpt;                     /* top of help index table         */
int top_of_wizhelpt;
int top_of_olchelpt;
int rebooting_check;
struct obj_data  *object_list = 0;    /* the global linked list of obj's */
struct char_data *character_list = 0; /* global l-list of chars          */

struct zone_data *zone_table;         /* table of reset data             */
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
int top_of_p_table = 0;               /* ref to top of table             */
int top_of_p_file = 0;
extern int CHAOSMODE;
extern int BAMDAY;
extern int BOOTFULL;
extern int TOKENCOUNT;

char credits[MSL];      /* the Credits List                */
char heroes[MSL];       /* the Heroes List                */
char news[MSL];         /* the news                        */
char motd[MSL];         /* the messages of today           */
char newbiemotd[MSL];
char godmotd[MSL];      /* the messages for the immortal   */
char help[MSL];         /* the main help page              */
char wizlist[MSL];      /* the wizlist                     */
char wizlist_inactive[MSL];
char helpcontents[3*MSL];
char olchelpcontents[3*MSL];
char wizhelpcontents[3*MSL];

FILE *help_fl,                        /* file for help texts (HELP <kwd>)*/
     *wizhelp_fl,
     *olchelp_fl;

struct help_index_element *help_index = 0;
struct help_index_element *wizhelp_index = 0;
struct help_index_element *olchelp_index = 0;

struct obj_proto *obj_proto_table = 0; /* object proto table             */
struct mob_proto *mob_proto_table = 0; /* mobile proto table             */

struct time_info_data time_info;     /* the infomation about the time   */
struct weather_data weather_info;     /* the infomation about the weather */

/* local procedures */
void boot_zones(void);
void setup_dir(FILE *fl, int room, int dir);
int create_wizlist (FILE *wizlist);
int create_inactive_wizlist (FILE *wizlist);
void boot_world(void);
void build_player_index(void);
int is_empty(int zone_nr);
int file_to_string(char *name, char *buf);
void reset_time(void);
void clear_char(struct char_data *ch);
void read_mobs( FILE *mob_f );
void read_objs( FILE *obj_f );
void assign_mobiles(void); /* Next 3 added by Ranger - July 96 */
void assign_objects(void);
void assign_rooms(void);
void initial_boot_area(char *name);
void examine_last_command(void);

/* external refs */
extern struct descriptor_data *descriptor_list;
extern int noroomdesc;
extern int noextradesc;
void read_vote(void);
void read_voters (void);
void load_messages(void);
void weather_and_time ( int mode );
void assign_command_pointers ( void );
void assign_enchantments   ( void );
void assign_spell_pointers ( void );
int dice(int number, int size);
int number(int from, int to);
void boot_social_messages(void);
void boot_pose_messages(void);
struct help_index_element *build_help_index(FILE *fl, int *num);
char *str_dup(char *source);
void intialize_auction_board(void);
void intialize_lottery(void);
void help_contents(struct help_index_element *help_index,char *contents,int top);
void read_corpselist(void);
void read_rank_boards(void);
void read_zone_rating(void);
void initialize_token_mob(void);
void read_idname(void);
char *skip_spaces(char *string);
void boot_the_shops();
/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

/* body of the booting system */

void boot_db(void)
{
     int i,token_number,loop=0,n_mid=0,s_mid=0;
     FILE *wiz_fl,*wiz_in_fl;
     extern int no_specials;

     log_f("Boot db -- BEGIN.");

     log_f("Resetting the game time:");
     reset_time();

     log_f("Reading newsfile, credits, heroes, help-page and motd.");
     file_to_string(NEWS_FILE, news);
     file_to_string(CREDITS_FILE, credits);
     file_to_string(HEROES_FILE, heroes);
     file_to_string(MOTD_FILE, motd);
     file_to_string(NEWBIEMOTD_FILE, newbiemotd);
     file_to_string(GODMOTD_FILE, godmotd);
     file_to_string(HELP_PAGE_FILE, help);

     if (!(wiz_fl = fopen (WIZLIST_FILE, "w"))) {
       log_f("   Could not open wizlist.");
     } else {
       create_wizlist (wiz_fl);
       file_to_string (WIZLIST_FILE, wizlist);
     }

     if (!(wiz_in_fl = fopen (WIZLIST_INACTIVE_FILE, "w"))) {
       log_f("   Could not open inactive wizlist.");
     } else {
       create_inactive_wizlist (wiz_in_fl);
       file_to_string(WIZLIST_INACTIVE_FILE, wizlist_inactive);
     }

     log_f("Opening help files.");
     if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
      log_f("   Could not open help file.");
     else {
          help_index = build_help_index(help_fl, &top_of_helpt);
          help_contents(help_index,helpcontents,top_of_helpt);
          }

     if (!(wizhelp_fl = fopen(WIZHELP_FILE, "r")))
         log_f("   Could not open wizhelp file.");
     else {
          wizhelp_index = build_help_index(wizhelp_fl, &top_of_wizhelpt);
          help_contents(wizhelp_index,wizhelpcontents,top_of_wizhelpt);
          }

     if (!(olchelp_fl = fopen(OLCHELP_FILE, "r")))
         log_f("   Could not open olchelp file.");
     else {
          olchelp_index = build_help_index(olchelp_fl, &top_of_olchelpt);
          help_contents(olchelp_index,olchelpcontents,top_of_olchelpt);
          }

     log_f("Loading World.");
     boot_world();
     log_f("Renumbering World.");
     renum_world();

     log_f("Loading fight messages.");
     load_messages();

     log_f("Loading social messages.");
     boot_social_messages();

     log_f("Loading clan list.");
     load_clan_file();

     log_f("Loading pose messages.");
     boot_pose_messages();

     log_f("Loading main shop file.");
     boot_the_shops();

     log_f("Assigning function pointers:");
     if (!no_specials)
     {
          log_f("   Mobiles.");
          assign_mobiles();
          log_f("   Objects.");
          assign_objects();
          log_f("   Room.");
          assign_rooms();
     }

     log_f("   Commands.");
     assign_command_pointers();
     log_f("   Spells.");
     assign_spell_pointers();

     log_f("   Enchantments.");
     assign_enchantments();

     log_f("Intializing auction system");
     intialize_auction_board();

     log_f("Intializing lottery system");
     intialize_lottery();

     log_f("Initializing Voting Machine");
     read_vote();
     read_voters();

     rebooting_check=1; /* For increased (use to reduce) load rate at reboot or crash - Ranger Oct 7/99 */

     for (i = 0; i <= top_of_zone_table; i++) {
       if(zone_table[i].virtual==30) {
         n_mid=i;
         continue;
       }

       if(zone_table[i].virtual==31) {
         s_mid=i;
         continue;
       }

       log_f("Performing boot-time reset of %s (rooms %d-%d).\n",
             zone_table[i].name,
             (i ? (zone_table[i - 1].top + 1) : 0),
             zone_table[i].top);

       // send MSG_ZONE_RESET to zone rooms, and ignore return value to handle spec reset
       signal_zone(NULL, zone_table[i].virtual, MSG_ZONE_RESET, "");
       reset_zone(i,BOOTFULL);
     }

     if(n_mid) {
       log_f("Performing boot-time reset of %s (rooms %d-%d).\n",
             zone_table[n_mid].name,
             (n_mid ? (zone_table[n_mid - 1].top + 1) : 0),
             zone_table[n_mid].top);

       // send MSG_ZONE_RESET to zone rooms, and ignore return value to handle spec reset
       signal_zone(NULL, zone_table[n_mid].virtual, MSG_ZONE_RESET, "");
       reset_zone(n_mid,BOOTFULL);
     }

     if(s_mid) {
       log_f("Performing boot-time reset of %s (rooms %d-%d).\n",
             zone_table[s_mid].name,
             (s_mid ? (zone_table[s_mid - 1].top + 1) : 0),
             zone_table[s_mid].top);

       // send MSG_ZONE_RESET to zone rooms, and ignore return value to handle spec reset
       signal_zone(NULL, zone_table[s_mid].virtual, MSG_ZONE_RESET, "");
       reset_zone(s_mid,BOOTFULL);
     }

     rebooting_check=0;
     log_f("Reading Corpselist file");
     read_corpselist();
     log_f("Reading rank files");
     read_rank_boards();
     log_f("Reading zone ratings");
     read_zone_rating();
     log_f("Reading player ids");
     read_idname();
     log_f("Distributing Subclass Tokens");
     token_number=TOKENCOUNT;

  if(CHAOSMODE)
    token_number=0;

  while(token_number>0) {
    if(distribute_token()) token_number--;
    loop++;
    if(loop>1000) {
      log_f("Breaking loop distribute tokens");
      break;
    }
  }
  log_f("Initializing token mob");
  initialize_token_mob();
  log_f("Checking for last_command file");
  examine_last_command();
  log_f("Boot db -- DONE.");
}

int is_shop(CHAR *mob);

int distribute_token(void)
{
    int i = 0, zone = 0, rzone = 0;
    int bReturn = FALSE;
    CHAR *mob = NULL;
    OBJ *token = NULL;

    i = number(1,top_of_mobt);

    zone = inzone(mob_proto_table[i].virtual);
    rzone = real_zone(zone);

    if(rzone != -1 &&
        zone != 0 && /*LIMBO*/
        zone != 10 && /*Quest Gear II*/
        zone != 12 && /*Immortal Rooms*/
        zone != 30 && /*Northern Midgaard Update*/
        zone != 31 && /*Southern Midgaard Update*/
        zone != 35 && /*Training, by Nosferatu*/
        zone != 36 && /*Cafe, by Jarldian*/
        zone != 39 && /*Stables, by Ranger and Quack*/
        zone != 58 && /*HMS Topknot*/
        zone != 66 && /*NewbieMist*/
        zone != 69 && /*Quest Gear*/
        zone != 123 && /*Boards*/
        zone != 253 && /*Hell1*/
        zone != 254 && /*Hell2*/
        zone != 255 && /*Hell3*/
        zone != 260 && /*Questy Vader III*/
        zone != 261 && /*Questy Nosferatu*/
        zone != 262 && /*Questy by Feroz*/
        zone != 275 && /*Clan Halls*/
        zone != 278 && /*ISA Hall*/
        zone != 294 && /*Custom Gear III*/
        zone != 295 && /*Lottery Items*/
        zone != 298 && /*Custom Gear II*/
        zone != 299 && /*Custom Gear*/
        zone != 300)   /*Labyrinth of Skelos by Quack*/
    {
  if(mob_proto_table[i].number > 0 && /*Mob exists in game*/
   mob_proto_table[i].level >= 15 && /*Mob is at least level 15*/
   !IS_SET(mob_proto_table[i].act2, ACT_NO_TOKEN)) /*Mob does not have NO_TOKEN flag*/
  {
   for(mob = character_list; mob; mob = mob->next) /*Iterate through all mobs/chars in game until we find the right one*/
   {
    if(mob->nr != i) /*Mob isn't the one we're after*/
     continue;
    if(GET_MAX_HIT(mob) >= 15000) /*Mob is too big*/
     continue;
    if(GET_MAX_HIT(mob) < 500 && GET_LEVEL(mob) < 42) /*Mob is too small*/
     continue;
    if(is_shop(mob)) /*Mob is a shopkeeper*/
     break;
    token = read_object(5,VIRTUAL);
    if (token)
    {
     token->obj_flags.value[0] = number(1,2);
     log_f("SUBLOG: Tokened %s v(%d) r(%d)",GET_SHORT(mob),V_MOB(mob),mob->nr);
     obj_to_char(token, mob);
     bReturn = TRUE;
     break;
    }
   }
  }
    }
    return bReturn;
}

int adjust_ticket_strings(OBJ *obj); /*Added Oct 98 Ranger */
struct obj_data *corpsefile_to_obj(FILE *fl) {
  struct obj_file_elem_ver3 object;
  int i;
  struct obj_data *obj=NULL;

  memset(&object,0,sizeof (struct obj_file_elem_ver3));
  fread(&object,sizeof(struct obj_file_elem_ver3),1,fl);
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

/* new obj saves for obj ver3 */
    obj->obj_flags.bitvector2   = object.bitvector2;
    obj->obj_flags.popped       = object.popped;
/* end new ver3 obj saves */

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
        obj_to_obj(corpsefile_to_obj(fl),obj);
    }
  }
  else  {
    if(object.item_number != 0) {
      log_f("BUG: No such item #%d",object.item_number);
    }
  }
  return obj;
}

char *corpsename_read(FILE *fl) {
  struct corpsefile_name cname;
  static char name[30];

  memset(&cname,0,sizeof(cname));
  fread(&cname,sizeof(cname),1,fl);
  sprintf(name,"%s",cname.name);
  return name;
}

void read_corpselist(void) {
  FILE *fl;
  OBJ *obj;
  char name[30],buf[MIL],buf2[MIL];

  fl=fopen("corpselist","rb");
  if(!fl) return;
  while (!feof(fl)) {
    strcpy(name,corpsename_read(fl));
    obj=corpsefile_to_obj(fl);
    if(obj) {
      obj->name=str_dup(name);
      strcpy(name,one_argument(name,buf));
      sprintf(buf2,"%s of%s is lying here.",buf,CAP(name));
      obj->description = str_dup(buf2);
      sprintf(buf2, "%s of%s",CAP(buf),CAP(name));
      obj->short_description = str_dup(buf2);
      obj_to_room(obj,real_room(1201));
    }
  }
  fclose(fl);
  if(remove("corpselist")) log_f("Error removing corpselist");
}

void help_contents(struct help_index_element *help_index,char *buf, int top) {
  char tmp[100],place[2];
  int line_len=0,ind;

  *buf = '\0';
  if(top==0) return;

  sprintf(place,"~");
  for (ind = 0; ind <= top; ind++) {
    sprintf(tmp,"%s",help_index[ind].keyword);
    string_to_lower(tmp);
    if(place[0]!=UPPER(tmp[0])) {
      if(strlen(buf) + 105 > 3*MSL) {
        log_f("Error: Not enough space for help contents");
        return;
      }
      place[0]=UPPER(tmp[0]);
      strcat(buf,"\n\r   ----------------------------------------------------------------------\n\r   ");
      strcat(buf,CAP(place));
      strcat(buf,"\n\r   ");
      line_len=0;
    }
    else strcat(buf,", ");

    if(strlen(buf) + strlen(tmp)+5 > 3*MSL) {
      log_f("Error: Not enough space for help contents");
      return;
    }
    line_len +=(strlen(tmp)+2);
    if(line_len>55) {
      line_len=0;
      strcat(buf,"\n\r   ");
      strcat(buf,CAP(tmp));
    }
    else strcat(buf,CAP(tmp));
  }

  if(strlen(buf) + 105 > 3*MSL) {
    log_f("Error: Not enough space for help contents");
    return;
  }
  strcat(buf,"\n\r   ----------------------------------------------------------------------\n\r");
  *(buf + strlen(buf) + 1) = '\0';
}


/* reset the time in the game from file */
void reset_time(void)
{
     long beginning_of_time = 650336715;
     struct time_info_data mud_time_passed(time_t t2, time_t t1);

     time_info = mud_time_passed(time(0), beginning_of_time);

     switch(time_info.hours){
          case 0 :
          case 1 :
          case 2 :
          case 3 :
          case 4 :
          {
               weather_info.sunlight = SUN_DARK;
               break;
          }
          case 5 :
          {
               weather_info.sunlight = SUN_RISE;
               break;
          }
          case 6 :
          case 7 :
          case 8 :
          case 9 :
          case 10 :
          case 11 :
          case 12 :
          case 13 :
          case 14 :
          case 15 :
          case 16 :
          case 17 :
          case 18 :
          case 19 :
          case 20 :
          {
               weather_info.sunlight = SUN_LIGHT;
               break;
          }
          case 21 :
          {
               weather_info.sunlight = SUN_SET;
               break;
          }
          case 22 :
          case 23 :
          default :
          {
               weather_info.sunlight = SUN_DARK;
               break;
          }
     }

     log_f("   Current Gametime: %dH %dD %dM %dY.",
             time_info.hours, time_info.day,
             time_info.month, time_info.year);

     weather_info.pressure = 960;
     if ((time_info.month>=7)&&(time_info.month<=12))
          weather_info.pressure += dice(1,50);
     else
          weather_info.pressure += dice(1,80);

     weather_info.change = 0;

     if (weather_info.pressure<=980)
          weather_info.sky = SKY_LIGHTNING;
     else if (weather_info.pressure<=1000)
          weather_info.sky = SKY_RAINING;
     else if (weather_info.pressure<=1020)
          weather_info.sky = SKY_CLOUDY;
     else weather_info.sky = SKY_CLOUDLESS;
}

/* update the time file */
void update_time(void)
{
     FILE *f1;
     extern struct time_info_data time_info;
     long current_time;

     return;


     if (!(f1 = fopen(TIME_FILE, "w")))
     {
          log_f("update time");
          produce_core();
     }

     current_time = time(0);
     log_f("Time update.");

     fprintf(f1, "#\n");

     fprintf(f1, "%ld\n", current_time);
     fprintf(f1, "%d\n", time_info.hours);
     fprintf(f1, "%d\n", time_info.day);
     fprintf(f1, "%d\n", time_info.month);
     fprintf(f1, "%d\n", time_info.year);

     fclose(f1);
}

void add_flying_room(int room) {
  int i,found=FALSE;

  for (i = 0;i<top_of_flying; i++) {
    if(*(flying_rooms+i)==room) found=TRUE;
  }
  if(found) return;
  top_of_flying++;
  flying_rooms=(int *)realloc(flying_rooms,(top_of_flying)*sizeof(int));
  *(flying_rooms+top_of_flying-1)=room;
}

void remove_flying_room(int room) {
  int i,found=FALSE,temp;
  for (i = 0;i<top_of_flying; i++) {
    if(!found && *(flying_rooms+i)==room) found=TRUE;
    if(found) {
      temp=*(flying_rooms+i+1);
      *(flying_rooms+i)=temp;
    }
  }
  if(found) top_of_flying--;
}

void read_rooms(FILE *fl)
{
     int room_nr = 0, zone = 0, virtual_nr, flag,tmp;
     long tmp2;
     char *temp, chk[50], *temp2;
     struct extra_descr_data *new_descr;
     struct extra_descr_data *tmp_desc;
     struct extra_descr_data *next_desc;

     do
     {
       fscanf(fl, " #%d\n", &virtual_nr);

       temp = fread_string(fl);
       if ((flag = (*temp != '$')))     /* a new record to be read */
         {
           room_nr = allocate_room(virtual_nr);
           world[room_nr].number = virtual_nr;
              if(world[room_nr].name)
                 free(world[room_nr].name);
           world[room_nr].name = temp;
              if(world[room_nr].description)
                 free(world[room_nr].description);
           if(noroomdesc) {
          temp2 = fread_string(fl);
          free(temp2);
          world[room_nr].description = str_dup("\r");
           } else {
          world[room_nr].description = fread_string(fl);
           }
              zone = 0;
           if (top_of_zone_table >= 0)
          {
            fscanf(fl, " %*d ");

            /* OBS: Assumes ordering of input rooms */

            if (world[room_nr].number <= (zone ? zone_table[zone-1].top : -1))
              {
                log_f("Room nr %d is below zone %d.\n",
                     room_nr, zone);
                produce_core();
              }
            while (world[room_nr].number > zone_table[zone].top)
              if (++zone > top_of_zone_table)
                {
               log_f("Room %d is outside of any zone.\n",
                    virtual_nr);
               produce_core();
                }
            world[room_nr].zone = zone_table[zone].virtual;
          }
           fscanf(fl, " %ld ", &tmp2);
           world[room_nr].room_flags = tmp2;
           if(IS_SET(world[room_nr].room_flags,FLYING)) add_flying_room(room_nr);
           fscanf(fl, " %d ", &tmp);
           world[room_nr].sector_type = tmp;

           world[room_nr].funct = NULL;
/*           world[room_nr].contents = 0;
           world[room_nr].people = 0;
           world[room_nr].light = 0;*/ /* Zero light sources */

           for (tmp = 0; tmp <= 5; tmp++)
                {
                if(world[room_nr].dir_option[tmp])
                  {
                  free(world[room_nr].dir_option[tmp]);
            world[room_nr].dir_option[tmp] = 0;
                  }
                }
           if(world[room_nr].ex_description)
                {
                for(tmp_desc = world[room_nr].ex_description;tmp_desc;tmp_desc = next_desc)
                   {
                   if(tmp_desc->keyword)
                      free(tmp_desc->keyword);
                   if(tmp_desc->description)
                      free(tmp_desc->description);
                   next_desc = tmp_desc->next;
                   }
                }
           world[room_nr].ex_description = 0;

           for (;;)
          {
            fscanf(fl, " %s \n", chk);

            if (*chk == 'D')  /* direction field */
              setup_dir(fl, room_nr, atoi(chk + 1));
            else if (*chk == 'E')  /* extra description field */
              {
                if(!noextradesc) {
               CREATE(new_descr, struct extra_descr_data, 1);
               new_descr->keyword = fread_string(fl);
               new_descr->description = fread_string(fl);
               new_descr->next = world[room_nr].ex_description;
               world[room_nr].ex_description = new_descr;
                } else {
               temp2 = fread_string(fl);
               free(temp2);
               temp2 = fread_string(fl);
               free(temp2);
                }
              }
            else if (*chk == 'S')     /* end of current room */
              break;
          }

            }
     }
     while (flag);

     free(temp);     /* cleanup the area containing the terminal $  */


}

/* load the rooms */
void boot_world(void)
{
     FILE *fl;
     char pZoneName[100];

     world = 0;
     character_list = 0;
     object_list = 0;
        top_of_world = -1;

     if (!(fl = fopen(WORLD_LIST_FILE, "r")))
     {
          log_f("fopen");
          log_f("boot_world: could not open list file.");
          produce_core();
     }
        else
          {
          while(!feof(fl))
            {
            fgets(pZoneName, 100, fl);
            if(strlen(pZoneName))
              {
              initial_boot_area(pZoneName);
              }
            }
       fclose(fl);
          }
}
int allocate_mob(int virtual_number)
{
  CHAR *mob;
  struct mob_proto *new_mob;
  int new_top;
  int i,new_number=-1;
  int bot, top, mid;

  bot = 0;
  top = top_of_mobt;

     /* perform binary search on mob-table */
  if(top_of_mobt != -1)
  {
  for (;;)
     {
          mid = (bot + top) / 2;

          if ((mob_proto_table + mid)->virtual == virtual_number)
                        {
                        new_number = mid;
                        break;
                        }
          if (bot >= top)
                        {
               new_number = -1;
                        break;
                        }
          if ((mob_proto_table + mid)->virtual > virtual_number)
               top = mid - 1;
          else
               bot = mid + 1;
     }
   }
  if(new_number != -1)
      return (new_number);
  new_top = top_of_mobt+1;
  if (new_top) {
    if (!(new_top%3500)) {
      if (!(new_mob = (struct mob_proto *)
         realloc(mob_proto_table, (new_top + 3500) * sizeof(struct mob_proto)))) {
     log_f("alloc_mob");
     produce_core();
      }
    } else
      new_mob = mob_proto_table;
  } else
    CREATE(new_mob, struct mob_proto, 3500);

  mob_proto_table = new_mob;

  top_of_mobt++;

  /* Find the index as to where it goes */
  if(top_of_mobt ==0)
     new_number = 0;
  else
    {
    if(virtual_number > mob_proto_table[top_of_mobt-1].virtual)
      {
      new_number = top_of_mobt;
      }
    else
      {/*Cheezey search for one the place to put the new one */
      for(i=0;i<top_of_mobt;i++)
        {
        if(virtual_number < mob_proto_table[i].virtual)
          {
          memmove(&mob_proto_table[i+1], &mob_proto_table[i], sizeof(struct mob_proto) *(top_of_mobt  -i));
          memset(&mob_proto_table[i], 0, sizeof(struct mob_proto));
          new_number = i;
          break;
          }
        }
      }
   for(mob=character_list;mob;mob= mob->next)
     {
     /*  if an item is above this one, increase its real number by one*/
     if(IS_NPC(mob) && mob->nr >= new_number && mob->nr_v != virtual_number)
       {
       mob->nr++;
       }
     }
    }
   return new_number;
}
int allocate_obj(int virtual_number)
{
  OBJ *obj;
  struct obj_proto *new_obj;
  int new_top;
  int i,new_number=-1;
  int bot, top, mid;

  bot = 0;
  top = top_of_objt;

     /* perform binary search on obj-table */
  if(top_of_objt != -1)
  {
  for (;;)
     {
          mid = (bot + top) / 2;

          if ((obj_proto_table + mid)->virtual == virtual_number)
                        {
                        new_number = mid;
                        break;
                        }
          if (bot >= top)
                        {
               new_number = -1;
                        break;
                        }
          if ((obj_proto_table + mid)->virtual > virtual_number)
               top = mid - 1;
          else
               bot = mid + 1;
     }
     }
  if(new_number != -1)
     return(new_number);
  new_top = top_of_objt+1;
  if (new_top) {
    if (!(new_top%3500)) {
      if (!(new_obj = (struct obj_proto *)
         realloc(obj_proto_table, (new_top + 3500) * sizeof(struct obj_proto)))) {
     log_f("alloc_room");
     produce_core();
      }
    } else
      new_obj = obj_proto_table;
  } else
    CREATE(new_obj, struct obj_proto, 3500);

  obj_proto_table = new_obj;

  top_of_objt++;

  /* Find the index as to where it goes */
  if(top_of_objt ==0)
     new_number = 0;
  else
    {
    if(virtual_number > obj_proto_table[top_of_objt-1].virtual)
      {
      new_number = top_of_objt;
      }
    else
      {/*Cheezey search for one the place to put the new one */
      for(i=0;i<top_of_objt;i++)
        {
        if(virtual_number < obj_proto_table[i].virtual)
          {
          memmove(&obj_proto_table[i+1], &obj_proto_table[i], sizeof(struct obj_proto) *(top_of_objt  -i));
          memset(&obj_proto_table[i], 0, sizeof(struct obj_proto));
          new_number = i;
          break;
          }
        }
      }
   for(obj=object_list;obj;obj= obj->next)
     {
     /*  if an item is above this one, increase its real number by one*/
     if(obj->item_number >= new_number && obj->item_number_v != virtual_number)
       {
       obj->item_number++;
       }
     }

    }
   return new_number;
}
int allocate_room(int virtual_number)
{
  struct room_data *new_world;
  int new_top;
  int i,new_number=-1;

   CHAR *chs = NULL, *next_ch=NULL;
   OBJ  *obj = NULL, *next_obj=NULL;
   int bot, top, mid;

   bot = 0;
   top = top_of_world;

     /* perform binary search on world-table */
    if(top_of_world != -1)
    {
    for (;;)
     {
          mid = (bot + top) / 2;

          if ((world + mid)->number == virtual_number)
                        {
               new_number = mid;
                        break;
                        }
          if (bot >= top)
          {
               new_number = -1;
                        break;
          }
          if ((world + mid)->number > virtual_number)
               top = mid - 1;
          else
               bot = mid + 1;
     }
    }
  if(new_number != -1)
    return new_number;
  new_top = top_of_world+1;
  if (new_top) {
    if (!(new_top%3500)) {
      if (!(new_world = (struct room_data *)
         realloc(world, (new_top + 3500) * sizeof(struct room_data)))) {
     log_f("alloc_room");
     produce_core();
      }
    } else
      new_world = world;
  } else
    CREATE(new_world, struct room_data, 3500);

  world = new_world;

  top_of_world++;

  /* Find the index as to where it goes */
  if(top_of_world ==0)
     new_number = 0;
  else
    {
    if(virtual_number > world[top_of_world-1].number)
      {
      new_number = top_of_world;
      }
    else
      {/*Cheezey search for one the place to put the new one */
      for(i=0;i<top_of_world;i++)
        {
        if(virtual_number < world[i].number)
          {
          memmove(&world[i+1], &world[i], sizeof(struct room_data) *(top_of_world  -i));
          memset(&world[i],0, sizeof(struct room_data));
          world[i].number = virtual_number;
          new_number = i;
          break;
          }
        }
      }

   for(chs=character_list;chs;chs= next_ch)
     {
     next_ch = chs->next;
     i = real_room(chs->in_room_v);
     if(i == NOWHERE)
       {
       log_f("Deleting char (%s) because was not attached to a room", GET_NAME(chs));
       char_to_room(chs, 0);
       extract_char(chs);
       }
     else
       {
       if(i!=chs->in_room_r)
          chs->in_room_r = i;
       }
     }
   /* Renumber the objs to match, also */
   for(obj=object_list;obj;obj= next_obj)
     {
     next_obj = obj->next;
     if(obj->in_room_v!=NOWHERE)
       {
       i = real_room(obj->in_room_v);
       if(i == NOWHERE)
         {
         log_f("Deleting obj (%s) because was not attached to a room", OBJ_NAME(obj));
         obj_to_room(obj, 0);
         extract_obj(obj);
         }
       else
         {
         if(i!=obj->in_room)
            obj->in_room = i;
         }
       }
     }
    }
   return new_number;
}
int allocate_zone(int virtual_number)
{
  struct zone_data *new_zone;
  int new_top;
  int i,new_number=-1;
    for(i=0;i<=top_of_zone_table;i++)
      {
      if(zone_table[i].virtual == virtual_number)
          new_number = i;
      }
  if(new_number != -1)
     return new_number;
  new_top = top_of_zone_table+1;
  if (new_top) {
    if (!(new_top%350)) {
      if (!(new_zone = (struct zone_data *)
         realloc(zone_table, (new_top + 350) * sizeof(struct zone_data)))) {
     log_f("alloc_zone");
     produce_core();
      }
    } else
      new_zone = zone_table;
  } else
    CREATE(new_zone, struct zone_data, 350);

  zone_table = new_zone;

  top_of_zone_table++;

  /* Find the index as to where it goes */
  if(top_of_zone_table ==0)
     new_number = 0;
  else
    {
    if(virtual_number > zone_table[top_of_zone_table-1].virtual)
      {
      new_number = top_of_zone_table;
      }
    else
      {/*Cheezey search for one the place to put the new one */
      for(i=0;i<top_of_zone_table;i++)
        {
        if(virtual_number < zone_table[i].virtual)
          {
          memmove(&zone_table[i+1], &zone_table[i], sizeof(struct zone_data) *(top_of_zone_table  -i));
          memset(&zone_table[i], 0, sizeof(struct zone_data));
          new_number = i;
          break;
          }
        }
      }
    }
   return new_number;
}
/* read direction data */
void setup_dir(FILE *fl, int room, int dir)
{
     int tmp;

     CREATE(world[room].dir_option[dir],
          struct room_direction_data, 1);

     world[room].dir_option[dir]->general_description =
          fread_string(fl);
     world[room].dir_option[dir]->keyword = fread_string(fl);

     fscanf(fl, " %d ", &tmp);

        /* Addition of lock levels - Ranger July 96 */
        /* Addition of new movements - Ranger Oct 96 */

        switch (tmp) {
          case 1: {
         world[room].dir_option[dir]->exit_info = EX_ISDOOR;
            break;
          }
          case 2: {
            world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
            break;
          }
          case 3: {
            world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_LOCK_10;
            break;
          }
          case 4: {
            world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_LOCK_15;
            break;
          }
          case 5: {
            world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_LOCK_20;
            break;
          }
          case 6: {
            world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_LOCK_25;
            break;
          }
          case 7: {
            world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_LOCK_30;
            break;
          }
          case 8: {
            world[room].dir_option[dir]->exit_info = EX_MOVE;
            break;
          }
          case 9: {
            world[room].dir_option[dir]->exit_info = EX_CRAWL;
            break;
          }
          case 10: {
            world[room].dir_option[dir]->exit_info = EX_JUMP;
            break;
          }
          case 11: {
            world[room].dir_option[dir]->exit_info = EX_CLIMB;
            break;
          }
          case 12: {
            world[room].dir_option[dir]->exit_info = EX_ENTER;
            break;
          }
          default:
           world[room].dir_option[dir]->exit_info = 0;
        }

     fscanf(fl, " %d ", &tmp);
     world[room].dir_option[dir]->key = tmp;

     fscanf(fl, " %d ", &tmp);
     world[room].dir_option[dir]->to_room_v = tmp;
}

void renum_world(void) {
  register int room, door;
  CHAR *chs,*next_ch;
  OBJ *obj,*next_obj;
  int i;

  for (room = 0; room <= top_of_world; room++)
    for (door = 0; door <= 5; door++)
      if (world[room].dir_option[door])
          world[room].dir_option[door]->to_room_r =
          real_room(world[room].dir_option[door]->to_room_v);

  /* Renumber the chs to match new rooms */
  for(chs=character_list;chs;chs= next_ch) {
    next_ch = chs->next;
    i = real_room(chs->in_room_v);
    if(i == NOWHERE) {
      log_f("Deleting char (%s) because was not attached to a room", GET_NAME(chs));
      char_to_room(chs, 0);
      extract_char(chs);
    }
    else {
     if(i!=chs->in_room_r)
       chs->in_room_r = i;
    }
  }
  /* Renumber the objs to match, also */
  for(obj=object_list;obj;obj= next_obj) {
    next_obj = obj->next;
    if(obj->in_room_v!=NOWHERE) {
      i=real_room(obj->in_room_v);
      if(i == NOWHERE) {
        log_f("Deleting obj (%s) because was not attached to a room", OBJ_NAME(obj));
        obj_to_room(obj, 0);
        extract_obj(obj);
      }
      else {
       if(i!=obj->in_room)
         obj->in_room = i;
      }
    }
  }
}

int read_zone(FILE *fl)
{
     int zon = 0, cmd_no = 0, expand, tmp,virtual_nr;
     char *check, buf[81],date1[10],date2[10],names[100];

            for (;;)
            {
          fscanf(fl, " #%d\n",&virtual_nr);
          check = fread_string(fl);

          if (*check == '$')
               break;          /* end of file */

          /* alloc a new zone */

          zon = allocate_zone(virtual_nr);
          if(zone_table[zon].name)
            free(zone_table[zon].name);
          zone_table[zon].name = check;
          zone_table[zon].virtual = virtual_nr;
          fscanf(fl, " %d ", &zone_table[zon].top);
          fscanf(fl, " %d ", &zone_table[zon].lifespan);
          fscanf(fl, " %d ", &zone_table[zon].reset_mode);
          if(virtual_nr==30)
            zone_table[zon].bottom = 2995;
          else
            zone_table[zon].bottom = virtual_nr*100;

          zone_table[zon].climate      = 0;
          zone_table[zon].create_date  = str_dup("Unknown");
          zone_table[zon].mod_date     = str_dup("Unknown");
          zone_table[zon].creators     = str_dup("Unknown");
          zone_table[zon].mult_hp      = 100;
          zone_table[zon].mult_mana    = 100;
          zone_table[zon].mult_hitroll = 100;
          zone_table[zon].mult_damage  = 100;
          zone_table[zon].mult_armor   = 100;
          zone_table[zon].mult_xp      = 100;
          zone_table[zon].mult_gold    = 100;
          zone_table[zon].mult_level   = 100;

          /* read the command table */

          if(zone_table[zon].cmd)
            free(zone_table[zon].cmd);
          cmd_no = 0;

          for (expand = 1;;)
          {
               if (expand) {
                    if (!cmd_no)
                         CREATE(zone_table[zon].cmd, struct reset_com, 1);
                    else
                         if (!(zone_table[zon].cmd =
                           (struct reset_com *) realloc(zone_table[zon].cmd,
                           (cmd_no + 1) * sizeof(struct reset_com))))
                         {
                              log_f("reset command load");
                              produce_core();
                         }
               }
               expand = 1;

               fscanf(fl, " "); /* skip blanks */
               fscanf(fl, "%c",
                    &zone_table[zon].cmd[cmd_no].command);

               if (zone_table[zon].cmd[cmd_no].command == 'S')
                    break;

               if(zone_table[zon].cmd[cmd_no].command == 'X') {
                 fscanf(fl," %d %s %s %s\n",&zone_table[zon].climate,date1,date2,names);
	           zone_table[zon].create_date = str_dup(date1);
                 zone_table[zon].mod_date    = str_dup(date2);
                 zone_table[zon].creators    = str_dup(names);
                 continue;
               }

               if(zone_table[zon].cmd[cmd_no].command == 'Y') {
                 fscanf(fl," %d %d %d %d %d %d %d %d\n",&zone_table[zon].mult_hp,&zone_table[zon].mult_mana,
                        &zone_table[zon].mult_hitroll,&zone_table[zon].mult_damage,&zone_table[zon].mult_armor,
                        &zone_table[zon].mult_xp,&zone_table[zon].mult_gold,&zone_table[zon].mult_level);
                 continue;
               }

               if (zone_table[zon].cmd[cmd_no].command == '*')
               {
                    expand = 0;
                    fgets(buf, 80, fl); /* skip command */
                    continue;
               }

               fscanf(fl, " %d %d %d",
                    &tmp,
                    &zone_table[zon].cmd[cmd_no].arg1,
                    &zone_table[zon].cmd[cmd_no].arg2);

               zone_table[zon].cmd[cmd_no].if_flag = tmp;

               if (zone_table[zon].cmd[cmd_no].command == 'M' ||
                    zone_table[zon].cmd[cmd_no].command == 'O' ||
                    zone_table[zon].cmd[cmd_no].command == 'T' ||
                    zone_table[zon].cmd[cmd_no].command == 'R' ||
                    zone_table[zon].cmd[cmd_no].command == 'F' ||
                    zone_table[zon].cmd[cmd_no].command == 'E' ||
                    zone_table[zon].cmd[cmd_no].command == 'P' ||
                    zone_table[zon].cmd[cmd_no].command == 'D')
                    fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

               fgets(buf, 80, fl);     /* read comment */

               cmd_no++;
          }
          zon++;
     }
     free(check);
     return virtual_nr;
}
/* load the zone table and command tables */
/*************************************************************************
*  procedures for resetting, both play-time and boot-time            *
*********************************************************************** */


/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
  char buf[MSL];
  int i,zone,rzone;
  float mult_hp=100,mult_mana=100,mult_hitroll=100,mult_damage=100,mult_armor=100;
  float mult_xp=100,mult_gold=100,mult_level=100;
  struct char_data *mob;

  i = nr;
  if (type == VIRTUAL)
    if ((nr = real_mobile(nr)) < 0) {
      sprintf(buf, "Mobile (V) %d does not exist in database.", i);
      return(0);
    }

  zone = inzone(mob_proto_table[nr].virtual);
  if(zone != -1) {
    rzone         = real_zone(zone);
    mult_hp       = zone_table[rzone].mult_hp;
    mult_mana     = zone_table[rzone].mult_mana;
    mult_hitroll  = zone_table[rzone].mult_hitroll;
    mult_damage   = zone_table[rzone].mult_damage;
    mult_armor    = zone_table[rzone].mult_armor;
    mult_xp       = zone_table[rzone].mult_xp;
    mult_gold     = zone_table[rzone].mult_gold;
    mult_level    = zone_table[rzone].mult_level;
  }

  CREATE(mob, struct char_data, 1);
  clear_char(mob);

  /***** String data *** */

  mob->player.name        = 0;
  mob->player.short_descr = 0;
  mob->player.long_descr  = 0;
  mob->player.description = 0;
  mob->player.title       = 0;
  mob->player.tagline     = 0;

  /* *** Numeric data *** */

  mob->specials.act         = mob_proto_table[nr].act;
  mob->specials.act2         = mob_proto_table[nr].act2;
  mob->specials.affected_by = mob_proto_table[nr].affected_by;
  mob->specials.affected_by2 = mob_proto_table[nr].affected_by2;
  mob->specials.alignment   = mob_proto_table[nr].alignment;
  GET_LEVEL(mob)            = mob_proto_table[nr].level*(mult_level/100);
  mob->specials.num_fighting=0;
  mob->specials.max_num_fighting=0;
  mob->abilities.str   = 18;
  mob->abilities.intel = 11;
  mob->abilities.wis   = 11;
  mob->abilities.dex   = 11;
  mob->abilities.con   = 11;
  mob->specials.rider  = 0;
  mob->specials.riding = 0;
  mob->specials.protecting = 0;
  mob->specials.protect_by=0;
  mob->specials.vaultaccess=0;
  mob->specials.wiznetlvl=0;
  mob->questgiver = 0;
  mob->questmob = 0;
  mob->questobj = 0;
  mob->questowner = 0;
  mob->quest_status = 0;
  mob->quest_level = 0;
  strcpy(mob->specials.vaultname,"not-set");
  mob->specials.reply_to=0;
  mob->bot.meta_update=0;
  mob->bot.meta_number=0;
  mob->bot.meta_amount=0;
  mob->bot.misses=0;


  mob->specials.death_timer=0; /* Imminent_death - Ranger Dec 2000 */
  /* New stuff - Ranger Sept 96 */
  mob->specials.att_timer = 0;
  mob->specials.no_att = mob_proto_table[nr].no_att;
  if(mob->specials.no_att>0) {
   for (i=0;i<mob->specials.no_att;i++) {
     if(i==MAX_ATTS) break;
     mob->specials.att_type[i] = mob_proto_table[nr].att_type[i];
     mob->specials.att_target[i] = mob_proto_table[nr].att_target[i];
     mob->specials.att_percent[i] = mob_proto_table[nr].att_percent[i];
     if( (mob->specials.att_type[i]==ATT_SPELL_CAST) ||
         (mob->specials.att_type[i]==ATT_SPELL_SKILL) )
       mob->specials.att_spell[i] = mob_proto_table[nr].att_spell[i];
   }
  }

  mob->specials.immune = mob_proto_table[nr].immune;
  mob->specials.immune2 = mob_proto_table[nr].immune2;
  mob->specials.resist =  mob_proto_table[nr].resist;

  if(mob_proto_table[nr].mana_nodice>0) {
    mob->points.max_mana = dice(mob_proto_table[nr].mana_nodice, mob_proto_table[nr].mana_sizedice) + mob_proto_table[nr].mana_add;
  } else {
    mob->points.max_mana = 10 * GET_LEVEL(mob);
  }
  mob->points.max_mana = mob->points.max_mana*(mult_mana/100);
  mob->points.mana     = mob->points.max_mana;

  mob->player.class = mob_proto_table[nr].class;

  /* End of new stuff */

  mob->points.hitroll = mob_proto_table[nr].hitroll*(mult_hitroll/100);;
  mob->points.armor = mob_proto_table[nr].armor*(mult_armor/100);;

  mob->points.max_hit = dice(mob_proto_table[nr].hp_nodice, mob_proto_table[nr].hp_sizedice) + mob_proto_table[nr].hp_add;
  mob->points.max_hit = mob->points.max_hit*(mult_hp/100);
  mob->points.hit     = mob->points.max_hit;

  mob->specials.damnodice   = mob_proto_table[nr].damnodice*(mult_damage/100);
  mob->specials.damsizedice = mob_proto_table[nr].damsizedice*(mult_damage/100);
  mob->points.damroll       = mob_proto_table[nr].damroll*(mult_damage/100);

  mob->points.move = 50;
  mob->points.max_move = 50;

  /* Random gold over 100k - Ranger July 99 */
  if(mob_proto_table[nr].gold>100000)
    mob->points.gold = 100000+(mob_proto_table[nr].gold-100000)/100*number(75,105);
  else
    mob->points.gold = mob_proto_table[nr].gold;
  mob->points.gold = mob->points.gold*(mult_gold/100);

  GET_EXP(mob) = mob_proto_table[nr].exp*(mult_xp/100);

  mob->specials.position    = mob_proto_table[nr].position;
  mob->specials.default_pos = mob_proto_table[nr].default_pos;
  if(GET_POS(mob)==POSITION_INCAP) GET_HIT(mob)=-3;

  mob->player.sex = mob_proto_table[nr].sex;

/*  mob->player.class = 0;*/
  mob->player.time.birth  = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon  = time(0);
  mob->player.weight      = 200;
  mob->player.height      = 198;

  for (i = 0; i < 3; i++)
    GET_COND(mob, i) = -1;

  for (i = 0; i < 5; i++)
    mob->specials.apply_saving_throw[i] = GET_LEVEL(mob); /*MAX(30-GET_LEVEL(mob), 2);*/

  mob->tmpabilities = mob->abilities;

  for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
    mob->equipment[i] = 0;

  mob->nr = nr;
  mob->nr_v = mob_proto_table[nr].virtual;

  mob->desc = 0;

  /* insert in list */

  mob->next = character_list;
  character_list = mob;

  mob_proto_table[nr].number++;

  return(mob);
}

/* read an object from OBJ_FILE */

extern struct struct_board *load_board(struct obj_data *obj);
struct obj_data *read_object(int nr, int type)
{
  struct obj_data *obj;
  int i,mod, j;
  char buf[100];

  memset(buf, 0, sizeof(buf));

  i = nr;
  if (type == VIRTUAL)
    if ((nr = real_object(nr)) < 0) {
      log_f("BUG: Object (V) %d does not exist in database.", i);
      return(0);
    }

  CREATE(obj, struct obj_data, 1);

  clear_object(obj);

  /* *** string data *** */
  strncpy(buf, obj_proto_table[nr].name, sizeof(buf)-1);
  obj->name = str_dup(buf);
  /* *** numeric data *** */

  obj->obj_flags.type_flag   = obj_proto_table[nr].obj_flags.type_flag;
  obj->obj_flags.extra_flags = obj_proto_table[nr].obj_flags.extra_flags;
  obj->obj_flags.wear_flags  = obj_proto_table[nr].obj_flags.wear_flags;
  obj->obj_flags.value[0] = obj_proto_table[nr].obj_flags.value[0];
  obj->obj_flags.value[1] = obj_proto_table[nr].obj_flags.value[1];
  obj->obj_flags.value[2] = obj_proto_table[nr].obj_flags.value[2];
  obj->obj_flags.value[3] = obj_proto_table[nr].obj_flags.value[3];
  obj->obj_flags.weight   = obj_proto_table[nr].obj_flags.weight;
  obj->obj_flags.cost     = obj_proto_table[nr].obj_flags.cost;
  obj->obj_flags.cost_per_day = obj_proto_table[nr].obj_flags.cost_per_day;
  obj->obj_flags.timer        = obj_proto_table[nr].obj_flags.timer;
  obj->obj_flags.bitvector    = obj_proto_table[nr].obj_flags.bitvector;
  obj->obj_flags.bitvector2    = obj_proto_table[nr].obj_flags.bitvector2;
  /* Extended May 2000 - Ranger */
  obj->obj_flags.extra_flags2 = obj_proto_table[nr].obj_flags.extra_flags2;
  obj->obj_flags.subclass_res = obj_proto_table[nr].obj_flags.subclass_res;
  obj->obj_flags.material     = obj_proto_table[nr].obj_flags.material;

  /* skin_loads for corpses - Ranger Feb 2001 */
  for(i=0;i<6;i++)
    obj->obj_flags.skin_vnum[i]=0;


  /* affects */
  for( i = 0 ; (i < MAX_OBJ_AFFECT); i++) {
    obj->affected[i].location = obj_proto_table[nr].affected[i].location;
    obj->affected[i].modifier = obj_proto_table[nr].affected[i].modifier;
  }


  /* If RANDOM flag, assign random stats */
  for (j = 0; j < MAX_OBJ_AFFECT; ++j) {
    mod = 0;

    if((IS_SET(obj->obj_flags.extra_flags2, ITEM_RANDOM) || /* All random */
        ((j == 0 && IS_SET(obj->obj_flags.extra_flags2, ITEM_RANDOM_0)) || /* First position random */
         (j == 1 && IS_SET(obj->obj_flags.extra_flags2, ITEM_RANDOM_1)) || /* Second position random */
         (j == 2 && IS_SET(obj->obj_flags.extra_flags2, ITEM_RANDOM_2))))) { /* Third position random */

      i = number(1, 100);

      if (obj->affected[j].location == APPLY_MOVE) {
        // non-weighted distribution
        mod = i;
      } else if ((obj->affected[j].location == APPLY_HIT) || (obj->affected[j].location == APPLY_MANA)) {
        // 50% non-weighted distribution
        if (i > 50) mod = 0;
        else mod = i;
      } else if (obj->affected[j].location == APPLY_ARMOR) {
        // weighted 0-10 distribution
        if (i < 5)       mod = -10; // 4%
        else if (i < 10) mod = -9;  // 5%
        else if (i < 16) mod = -8;  // 6%
        else if (i < 23) mod = -7;  // 7%
        else if (i < 31) mod = -6;  // 8%
        else if (i < 41) mod = -5;  // 10%
        else if (i < 51) mod = -4;  // 10%
        else if (i < 61) mod = -3;  // 10%
        else if (i < 71) mod = -2;  // 10%
        else if (i < 81) mod = -1;  // 10%
        else             mod = 0;   // 20%
      } else if (obj->affected[j].location == APPLY_HP_REGEN) {
        // weighted distribution 0-30 (by 5s)
        if (i < 6)       mod = 30;  // 5%
        else if (i < 11) mod = 25;  // 5%
        else if (i < 16) mod = 20;  // 5%
        else if (i < 26) mod = 15;  // 10%
        else if (i < 36) mod = 10;  // 10%
        else if (i < 46) mod = 5;   // 10%
        else             mod = 0;   // 55%
      } else if (obj->affected[j].location == APPLY_MANA_REGEN) {
        // weighted distribution 0-6
        if (i < 6)       mod = 6;  // 5%
        else if (i < 11) mod = 5;  // 5%
        else if (i < 16) mod = 4;  // 5%
        else if (i < 26) mod = 3;  // 10%
        else if (i < 36) mod = 2;  // 10%
        else if (i < 46) mod = 1;  // 10%
        else             mod = 0;  // 55%
      } else if (obj->affected[j].location == APPLY_HITROLL || obj->affected[j].location == APPLY_DAMROLL) {
        // weighted distribution 0-2
        if (i < 6)       mod = 2;  // 5%
        else if (i < 21) mod = 1;  // 15%
        else             mod = 0;  // 80%
      }
      obj->affected[j].modifier += mod;
    }
  }

  obj->log = 0;
  obj->in_room = NOWHERE;
  obj->in_room_v = NOWHERE;
  obj->next_content = 0;
  obj->carried_by = 0;
  obj->equipped_by = 0;
  obj->in_obj = 0;
  obj->contains = 0;
  obj->item_number   = nr;
  obj->item_number_v = obj_proto_table[nr].virtual;

  obj->next = object_list;
  object_list = obj;

  obj_proto_table[nr].number++;

  /* New ownerid field */
  obj->ownerid[0] = 0;
  obj->ownerid[1] = 0;
  obj->ownerid[2] = 0;
  obj->ownerid[3] = 0;
  obj->ownerid[4] = 0;
  obj->ownerid[5] = 0;
  obj->ownerid[6] = 0;
  obj->ownerid[7] = 0;

  obj->obj_flags.popped = time(NULL) / (60*60*24); /* current time in days since Jan 1, 1970 */

  /* if board, initialize it */
  if(obj->obj_flags.type_flag==ITEM_BOARD) load_board(obj);

  return (obj);
}

void read_mobs( FILE *mob_f )
{
  int i = 0, j = 0;
  char letter = 0x00, chk[50];
  char *temp = NULL;
  long tmp = 0;
  int tmp2 = 0, tmp3 = 0, tmp4 = 0, typ = 0;
  int not_end = TRUE, virtual_nr,ok=TRUE;
  struct tagline_data *tmp_tag = NULL, *next_tag = NULL, *new_tag = NULL;

  fscanf(mob_f, "#");
  do {
    fscanf(mob_f, "%d \r", &virtual_nr);
    temp = fread_string(mob_f);
    if((not_end = (temp[0] != '$')))
      {
      /* *** string data *** */
      i = allocate_mob(virtual_nr);

      mob_proto_table[i].virtual     = virtual_nr;
      if(mob_proto_table[i].name)
        free (mob_proto_table[i].name);
      if(mob_proto_table[i].short_descr)
        free (mob_proto_table[i].short_descr);
      if(mob_proto_table[i].long_descr)
        free (mob_proto_table[i].long_descr);
      if(mob_proto_table[i].description)
        free (mob_proto_table[i].description);
      mob_proto_table[i].name        = temp;
      mob_proto_table[i].short_descr = fread_string(mob_f);
      mob_proto_table[i].long_descr  = fread_string(mob_f);
      mob_proto_table[i].description = fread_string(mob_f);
      mob_proto_table[i].func       = NULL;

      /* *** Numeric data *** */

      fscanf(mob_f, "%ld ", &tmp);
      mob_proto_table[i].act = tmp;
      SET_BIT(mob_proto_table[i].act, ACT_ISNPC);

      fscanf(mob_f, " %ld ", &tmp);
      mob_proto_table[i].affected_by = tmp;

      fscanf(mob_f, " %ld ", &tmp);
      mob_proto_table[i].alignment = tmp;

      fscanf(mob_f, " %c \n", &letter);

      if (letter == 'S') {
        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].level = MAX(1,tmp2);

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].hitroll = MIN(mob_proto_table[i].level,(20 - tmp2));
        if(mob_proto_table[i].level>19) mob_proto_table[i].hitroll+=2;
        if(mob_proto_table[i].level>23) mob_proto_table[i].hitroll+=2;
        if(mob_proto_table[i].level>26) mob_proto_table[i].hitroll+=2;
        if(mob_proto_table[i].level>29) mob_proto_table[i].hitroll+=2;
        if(mob_proto_table[i].level>32) mob_proto_table[i].hitroll+=3;
        if(mob_proto_table[i].level>35) mob_proto_table[i].hitroll+=3;
        if(mob_proto_table[i].level>38) mob_proto_table[i].hitroll+=4;
        mob_proto_table[i].hitroll = MIN(mob_proto_table[i].level,mob_proto_table[i].hitroll);

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].armor  = 10*tmp2;

        fscanf(mob_f, " %dd%d+%d ", &tmp2, &tmp3, &tmp4);
        mob_proto_table[i].hp_nodice   = tmp2;
        mob_proto_table[i].hp_sizedice = tmp3;
        mob_proto_table[i].hp_add      = tmp4;

        fscanf(mob_f, " %dd%d+%d \n", &tmp2, &tmp3, &tmp4);
        mob_proto_table[i].damnodice   = tmp2;
        mob_proto_table[i].damsizedice = tmp3;
        mob_proto_table[i].damroll     = tmp4;

        fscanf(mob_f, " %ld ", &tmp);
        mob_proto_table[i].gold = tmp;

        fscanf(mob_f, " %ld \n", &tmp);
        mob_proto_table[i].exp = tmp;

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].position = tmp2;

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].default_pos = tmp2;

        fscanf(mob_f, " %d \n", &tmp2);
        mob_proto_table[i].sex = tmp2;

/* New stuff for "X"tended mobs - Ranger Sept 96 */

        mob_proto_table[i].class = 0;
        mob_proto_table[i].immune = 0;
        mob_proto_table[i].mana_nodice   = 0;
        mob_proto_table[i].mana_sizedice = 0;
        mob_proto_table[i].mana_add      = 0;
        mob_proto_table[i].no_att = 0;
/* New stuff for version Y mobs - Ranger March 99 */
        mob_proto_table[i].hit_type = 0;
        mob_proto_table[i].act2=0;
        mob_proto_table[i].affected_by2=0;
        mob_proto_table[i].immune2=0;

      } else if (letter == 'X') {
        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].level = MAX(1,tmp2);

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].hitroll = MIN(mob_proto_table[i].level,(20 - tmp2));
        if(mob_proto_table[i].level>19) mob_proto_table[i].hitroll+=2;
        if(mob_proto_table[i].level>23) mob_proto_table[i].hitroll+=2;
        if(mob_proto_table[i].level>26) mob_proto_table[i].hitroll+=2;
        if(mob_proto_table[i].level>29) mob_proto_table[i].hitroll+=2;
        if(mob_proto_table[i].level>32) mob_proto_table[i].hitroll+=3;
        if(mob_proto_table[i].level>35) mob_proto_table[i].hitroll+=3;
        if(mob_proto_table[i].level>38) mob_proto_table[i].hitroll+=4;
        mob_proto_table[i].hitroll = MIN(mob_proto_table[i].level,mob_proto_table[i].hitroll);

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].armor  = 10*tmp2;

        fscanf(mob_f, " %dd%d+%d ", &tmp2, &tmp3, &tmp4);
        mob_proto_table[i].hp_nodice   = tmp2;
        mob_proto_table[i].hp_sizedice = tmp3;
        mob_proto_table[i].hp_add      = tmp4;

        fscanf(mob_f, " %dd%d+%d \n", &tmp2, &tmp3, &tmp4);
        mob_proto_table[i].damnodice   = tmp2;
        mob_proto_table[i].damsizedice = tmp3;
        mob_proto_table[i].damroll     = tmp4;

        fscanf(mob_f, " %ld ", &tmp);
        mob_proto_table[i].gold = tmp;

        fscanf(mob_f, " %ld \n", &tmp);
        mob_proto_table[i].exp = tmp;

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].position = tmp2;

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].default_pos = tmp2;

        fscanf(mob_f, " %d \n", &tmp2);
        mob_proto_table[i].sex = tmp2;

        /* New stuff for "X"tended mobs - Ranger Sept 96 */

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].class = tmp2;

        fscanf(mob_f, " %ld ", &tmp);
        mob_proto_table[i].immune = tmp;

        fscanf(mob_f, " %dd%d+%d ", &tmp2, &tmp3, &tmp4);
        mob_proto_table[i].mana_nodice   = tmp2;
        mob_proto_table[i].mana_sizedice = tmp3;
        mob_proto_table[i].mana_add      = tmp4;

        fscanf(mob_f, " %d \n", &tmp2);
        mob_proto_table[i].no_att = tmp2;

        if(mob_proto_table[i].no_att>0) {
         for (j=0;j<mob_proto_table[i].no_att;j++) {
          if(j==MAX_ATTS) break;
          fscanf(mob_f, "%d %d %d %d\n", &typ,&tmp2,&tmp3,&tmp4);
          mob_proto_table[i].att_type[j]= typ;
          mob_proto_table[i].att_target[j]= tmp2;
          mob_proto_table[i].att_percent[j]= tmp3;
          mob_proto_table[i].att_spell[j]= tmp4;
         }
        }

/* New stuff for version Y mobs - Ranger March 99 */
        mob_proto_table[i].hit_type = 0;
        mob_proto_table[i].act2=0;
        mob_proto_table[i].affected_by2=0;
        mob_proto_table[i].immune2=0;

      } else if (letter == 'Y') {
        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].level = MAX(1,tmp2);

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].hitroll = MIN(mob_proto_table[i].level,(20 - tmp2));

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].armor  = 10*tmp2;

        fscanf(mob_f, " %dd%d+%d ", &tmp2, &tmp3, &tmp4);
        mob_proto_table[i].hp_nodice   = tmp2;
        mob_proto_table[i].hp_sizedice = tmp3;
        mob_proto_table[i].hp_add      = tmp4;

        fscanf(mob_f, " %dd%d+%d \n", &tmp2, &tmp3, &tmp4);
        mob_proto_table[i].damnodice   = tmp2;
        mob_proto_table[i].damsizedice = tmp3;
        mob_proto_table[i].damroll     = tmp4;

        fscanf(mob_f, " %ld ", &tmp);
        mob_proto_table[i].gold = tmp;

        fscanf(mob_f, " %ld \n", &tmp);
        mob_proto_table[i].exp = tmp;

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].position = tmp2;

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].default_pos = tmp2;

        fscanf(mob_f, " %d \n", &tmp2);
        mob_proto_table[i].sex = tmp2;

        /* New stuff for "X"tended mobs - Ranger Sept 96 */

        fscanf(mob_f, " %d ", &tmp2);
        mob_proto_table[i].class = tmp2;

        fscanf(mob_f, " %ld ", &tmp);
        mob_proto_table[i].immune = tmp;

        fscanf(mob_f, " %dd%d+%d ", &tmp2, &tmp3, &tmp4);
        mob_proto_table[i].mana_nodice   = tmp2;
        mob_proto_table[i].mana_sizedice = tmp3;
        mob_proto_table[i].mana_add      = tmp4;

        fscanf(mob_f, " %d \n", &tmp2);
        mob_proto_table[i].no_att = tmp2;

        if(mob_proto_table[i].no_att>0) {
         for (j=0;j<mob_proto_table[i].no_att;j++) {
          if(j==MAX_ATTS) break;
          fscanf(mob_f, "%d %d %d %d\n", &typ,&tmp2,&tmp3,&tmp4);
          mob_proto_table[i].att_type[j]= typ;
          mob_proto_table[i].att_target[j]= tmp2;
          mob_proto_table[i].att_percent[j]= tmp3;
          mob_proto_table[i].att_spell[j]= tmp4;
         }
        }

/* New stuff for version Y mobs - Ranger March 99 */
        fscanf(mob_f, "%d ", &typ);
        mob_proto_table[i].hit_type = typ;

        fscanf(mob_f, " %ld ", &tmp);
        mob_proto_table[i].act2 = tmp;

        fscanf(mob_f, " %ld ", &tmp);
        mob_proto_table[i].affected_by2 = tmp;

        fscanf(mob_f, " %ld\n", &tmp);
        mob_proto_table[i].immune2 = tmp;


      }
/*        fscanf(mob_f, " #");*/

      /* Taglines - Ranger May 2000 */
      /* *** taglines *** */
      if(mob_proto_table[i].tagline) {
        for(tmp_tag = mob_proto_table[i].tagline;tmp_tag;tmp_tag = next_tag) {
          if(tmp_tag->desc)
            free(tmp_tag->desc);
          next_tag = tmp_tag->next;
        }
      }
      mob_proto_table[i].tagline = 0;

      /* zero mob skin - Ranger Feb 2001 */
      mob_proto_table[i].skin_value = 0;
      for (j=0;j<6;j++)
        mob_proto_table[i].skin_vnum[j] = 0;

      ok = TRUE;
      while (ok) {
        fscanf(mob_f, " %1s", chk);
        switch(*chk) {
          case 'T': {
            fscanf(mob_f, " ");
            CREATE(new_tag, struct tagline_data, 1);
            new_tag->desc = fread_string(mob_f);
            new_tag->next = mob_proto_table[i].tagline;
            mob_proto_table[i].tagline = new_tag;
          } break;
          case 'K': {
            /* 2009.10.04 Shun: re-written to handle increasing numbers
               of objects loaded in skin */

            fscanf(mob_f, "%d", &typ);
            mob_proto_table[i].skin_value = typ;
            for (j=0; j<6; j++) {
              fscanf(mob_f, "%d%c", &typ, &letter);
              mob_proto_table[i].skin_vnum[j] = typ;
              if (letter == '\n') break;
            }
          } break;
          case 'R': {
            fscanf(mob_f, "%ld\n", &tmp);
            mob_proto_table[i].resist = tmp;
          } break;
          default:
            ok = FALSE;
            break;
        }
      }
    } /* if not end */
  } while(not_end);
  free(temp);
}

void read_objs( FILE *obj_f )
{
  char chk[50], buf[MSL], *temp;
  int i, tmp, loc, ok,not_end = TRUE, virtual_nr;
  struct extra_descr_data *new_descr;
  struct extra_descr_data *tmp_desc;
  struct extra_descr_data *next_desc;

  fscanf(obj_f,"#");
  do {
    fscanf(obj_f, "%d \r", &virtual_nr);
    temp = fread_string(obj_f);
    if((not_end = (temp[0] != '$')))
      {
      /* *** string data *** */
      i = allocate_obj(virtual_nr);
      obj_proto_table[i].virtual            = virtual_nr;
      if(obj_proto_table[i].name)
         free(obj_proto_table[i].name);
      if(obj_proto_table[i].short_description)
         free(obj_proto_table[i].short_description);
      if(obj_proto_table[i].description)
         free(obj_proto_table[i].description);
      if(obj_proto_table[i].action_description)
         free(obj_proto_table[i].action_description);
      obj_proto_table[i].name               = temp;
      obj_proto_table[i].short_description  = fread_string(obj_f);
      obj_proto_table[i].description        = fread_string(obj_f);
      obj_proto_table[i].action_description = fread_string(obj_f);
      if(obj_proto_table[i].action_description) {
        sprintf(buf,"%s",obj_proto_table[i].action_description);
        if(!strcmp(buf,"(null)"))
          obj_proto_table[i].action_description=NULL;
      }
      /* *** numeric data *** */

      fscanf(obj_f, " %d ", &tmp);
      obj_proto_table[i].obj_flags.type_flag = tmp;
      fscanf(obj_f, " %d ", &tmp);
      obj_proto_table[i].obj_flags.extra_flags = tmp;
      fscanf(obj_f, " %d ", &tmp);
      obj_proto_table[i].obj_flags.wear_flags = tmp;
      fscanf(obj_f, " %d ", &tmp);
      obj_proto_table[i].obj_flags.value[0] = tmp;
      fscanf(obj_f, " %d ", &tmp);
      obj_proto_table[i].obj_flags.value[1] = tmp;
      fscanf(obj_f, " %d ", &tmp);
      obj_proto_table[i].obj_flags.value[2] = tmp;
      fscanf(obj_f, " %d ", &tmp);
      obj_proto_table[i].obj_flags.value[3] = tmp;
      fscanf(obj_f, " %d ", &tmp);
      obj_proto_table[i].obj_flags.weight = tmp;
      fscanf(obj_f, " %d \n", &tmp);
      obj_proto_table[i].obj_flags.cost = tmp;
      fscanf(obj_f, " %d \n", &tmp);
      obj_proto_table[i].obj_flags.cost_per_day = tmp;
      fscanf(obj_f, " %d \n", &tmp);
      obj_proto_table[i].obj_flags.repop_percent = tmp;

      obj_proto_table[i].obj_flags.timer = 0;
      obj_proto_table[i].obj_flags.bitvector = 0;
      /* *** extra descriptions *** */
      if(obj_proto_table[i].ex_description)
        {
        for(tmp_desc = obj_proto_table[i].ex_description;tmp_desc;tmp_desc = next_desc)
           {
           if(tmp_desc->keyword)
              free(tmp_desc->keyword);
           if(tmp_desc->description)
              free(tmp_desc->description);
           next_desc = tmp_desc->next;
           }
        }
      obj_proto_table[i].ex_description = 0;
      loc = 0;
      ok = TRUE;

      while (ok) {
        fscanf(obj_f, " %1s", chk);
        switch(*chk) {
          case 'E': {
             if(!noextradesc) {
              fscanf(obj_f, " ");
             CREATE(new_descr, struct extra_descr_data, 1);

            new_descr->keyword = fread_string(obj_f);
           new_descr->description = fread_string(obj_f);

           new_descr->next = obj_proto_table[i].ex_description;
           obj_proto_table[i].ex_description = new_descr;
           }
            else
              {
           temp = fread_string(obj_f);
           free(temp);
           temp = fread_string(obj_f);
           free(temp);
           }
            } break;
          case 'A': {
            if(loc >= MAX_OBJ_AFFECT) {
              log_f("BUG: too many applys. #%d %s",
                      obj_proto_table[i].virtual, obj_proto_table[i].short_description);
              fscanf(obj_f, " %d ", &tmp);
              fscanf(obj_f, " %d \n", &tmp);
            } else {
              fscanf(obj_f, " %d ", &tmp);
              obj_proto_table[i].affected[loc].location = tmp;
              fscanf(obj_f, " %d \n", &tmp);
              obj_proto_table[i].affected[loc++].modifier = tmp;
            }
          } break;
        case 'B': {
            fscanf(obj_f, " %d ", &tmp);
            obj_proto_table[i].obj_flags.bitvector = tmp;
          } break;
        case 'C': {
            fscanf(obj_f, " %d ", &tmp);
            obj_proto_table[i].obj_flags.bitvector2 = tmp;
          } break;
        case 'T': {
          fscanf(obj_f, " %d ", &tmp);
          obj_proto_table[i].obj_flags.timer = tmp;
          } break;
        case 'X': { /* Extended obj properties - Ranger May 2000 */
          /* No target action descript */
          fscanf(obj_f, " ");
          if(obj_proto_table[i].action_description_nt)
            free(obj_proto_table[i].action_description_nt);
          obj_proto_table[i].action_description_nt = fread_string(obj_f);
          if(obj_proto_table[i].action_description_nt) {
            obj_proto_table[i].action_description_nt = skip_spaces(obj_proto_table[i].action_description_nt);
            sprintf(buf,"%s",obj_proto_table[i].action_description_nt);
            if(!strcmp(buf,"(null)"))
              obj_proto_table[i].action_description_nt=NULL;
          }

          fscanf(obj_f, " %d ", &tmp);
          obj_proto_table[i].obj_flags.extra_flags2 = tmp;
          fscanf(obj_f, " %d ", &tmp);
          obj_proto_table[i].obj_flags.subclass_res = tmp;
          fscanf(obj_f, " %d \n", &tmp);
          obj_proto_table[i].obj_flags.material = tmp;
          } break;
        case 'M': { /* Equipment messages - Ranger May 2000 */
          fscanf(obj_f, " ");
          if(obj_proto_table[i].char_wear_desc)
            free(obj_proto_table[i].char_wear_desc);
          obj_proto_table[i].char_wear_desc = fread_string(obj_f);
          if(obj_proto_table[i].char_wear_desc) {
            obj_proto_table[i].char_wear_desc = skip_spaces(obj_proto_table[i].char_wear_desc);
            sprintf(buf,"%s",obj_proto_table[i].char_wear_desc);
            if(!strcmp(buf,"(null)"))
              obj_proto_table[i].char_wear_desc=NULL;
          }
          if(obj_proto_table[i].room_wear_desc)
            free(obj_proto_table[i].room_wear_desc);
          obj_proto_table[i].room_wear_desc = fread_string(obj_f);
          if(obj_proto_table[i].room_wear_desc) {
            sprintf(buf,"%s",obj_proto_table[i].room_wear_desc);
            if(!strcmp(buf,"(null)"))
              obj_proto_table[i].room_wear_desc=NULL;
          }
          if(obj_proto_table[i].char_rem_desc)
            free(obj_proto_table[i].char_rem_desc);
          obj_proto_table[i].char_rem_desc = fread_string(obj_f);
          if(obj_proto_table[i].char_rem_desc) {
            sprintf(buf,"%s",obj_proto_table[i].char_rem_desc);
            if(!strcmp(buf,"(null)"))
              obj_proto_table[i].char_rem_desc=NULL;
          }
          if(obj_proto_table[i].room_rem_desc)
            free(obj_proto_table[i].room_rem_desc);
          obj_proto_table[i].room_rem_desc = fread_string(obj_f);
          if(obj_proto_table[i].room_rem_desc) {
            sprintf(buf,"%s",obj_proto_table[i].room_rem_desc);
            if(!strcmp(buf,"(null)"))
              obj_proto_table[i].room_rem_desc=NULL;
          }

          } break;
        default: {
          ok = FALSE;
        } break;
      }
    }

    for (;(loc < MAX_OBJ_AFFECT); loc++) {
      obj_proto_table[i].affected[loc].location = APPLY_NONE;
      obj_proto_table[i].affected[loc].modifier = 0;
      }
    }
  } while(not_end);
  free(temp);
}

#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
/* Only allow 5 zones to reset at a tick, delay the rest for the next one
   Ranger Oct 98 */
void zone_update(void) {
  int i,reset=0;

  for (i = 0; i <= top_of_zone_table; i++) {
    if(zone_table[i].age < zone_table[i].lifespan && zone_table[i].reset_mode)
      (zone_table[i].age)++;
    else
      if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
        reset++;
        if(reset>5) continue;
        if(!signal_zone(NULL,zone_table[i].virtual,MSG_ZONE_RESET,""))
          reset_zone(i,FALSE);
      }
  }
}

#define ZCMD zone_table[zone].cmd[cmd_no]
void add_follower(struct char_data *ch, struct char_data *leader);
/* Added above reference - Ranger June 96 */
void reset_zone(int zone, int full)
{
  int cmd_no,pop_percent,last_cmd = 1;
  char buf[256];
  struct char_data *mob=NULL,*tmpmob=NULL; /* tmpmob - Ranger June 96 */
  struct obj_data *obj, *obj_to;
  int room, object, object_to, mobile=0,pop_bonus=4,num;

/* The following switch was added by Ranger to fix the zone reset when
     empty problem. */

  switch(zone_table[zone].reset_mode) {

    case 0: /* Zone never resets. */
      break; /* This case should never be called except at boot */

    case 1: /* Zone resets when empty */
      if(!full) {
        if(is_empty(zone) || CHAOSMODE) break;
        zone_table[zone].age = 0;
        return;
      }
      break;
    case 2: /* Zone resets regardless */
      break;

    case 3: /* Zone block */
      if(!full) {
#ifndef TEST_SITE
        sprintf(buf,"WIZINFO: Zone %d : Reset Blocked.",zone_table[zone].virtual);
        wizlog(buf, LEVEL_WIZ , 6);
#endif
        zone_table[zone].age = 0;
        return;
      }
      break;
    case 4:
      if(!full) {
#ifndef TEST_SITE
        sprintf(buf,"WIZINFO: Zone %d : Locked.",zone_table[zone].virtual);
        wizlog(buf, LEVEL_WIZ , 6);
#endif
        zone_table[zone].age = 0;
        return;
      }
      break;
    case 5: /* Doors only */
      break;

    default:
      sprintf(buf,"WIZINFO: Zone %d : Invalid Case.",zone_table[zone].virtual);
      wizlog(buf, LEVEL_WIZ , 6);
      break;
  }

/* End of switch add by Ranger */

    for (cmd_no = 0;;cmd_no++)
    {
    if (ZCMD.command == 'S')
      break;

    if (last_cmd || !ZCMD.if_flag)
      switch(ZCMD.command)
     {
     case 'M': /* read a mobile */
          if(zone_table[zone].reset_mode==5) break;
          mobile  = real_mobile(ZCMD.arg1);
          room    = real_room(ZCMD.arg3);
          if(mobile != -1 && room != -1 )
            {
           if (mob_proto_table[mobile].number < ZCMD.arg2 || full)
             {
            mob = read_mobile(mobile, REAL);
              tmpmob=mob;
              char_to_room(mob, room);
           last_cmd = 1;
           }
         else
           last_cmd = 0;
            }
          else
            last_cmd = 0;
          break;
/* Follow reset command - Ranger June 96 */

     case 'F': /* follow a mobile */
          if(zone_table[zone].reset_mode==5) break;
          mobile  = real_mobile(ZCMD.arg1);
          room    = real_room(ZCMD.arg3);
          if(mobile != -1 && room != -1 )
            {
           if ((mob_proto_table[mobile].number < ZCMD.arg2 || full) && tmpmob)
             {
            mob = read_mobile(mobile, REAL);
              char_to_room(mob, room);
              add_follower(mob, tmpmob);
           last_cmd = 1;
           }
         else
           last_cmd = 0;
            }
          else
            last_cmd = 0;
          break;

     case 'R': /* add mount for M */
          if(zone_table[zone].reset_mode==5) break;
          mobile  = real_mobile(ZCMD.arg1);
          room    = real_room(ZCMD.arg3);
          if(mobile != -1 && room != -1 )
            {
           if ((mob_proto_table[mobile].number < ZCMD.arg2 || full) && tmpmob) {
            mob = read_mobile(mobile, REAL);
              char_to_room(mob, room);
              if (CHAR_REAL_ROOM(mob) == CHAR_REAL_ROOM(tmpmob) ) {
                 if(GET_POS(tmpmob)!=POSITION_RIDING) {
                   tmpmob->specials.riding=mob;
                   GET_POS(tmpmob)=POSITION_RIDING;
                   if(mob->master) stop_follower(mob);
                   add_follower(mob,tmpmob);
                   mob->specials.rider=tmpmob;
                 }
              }
           last_cmd = 1;
         }
         else
           last_cmd = 0;
            }
          else
            last_cmd = 0;
          break;

     case 'O': /* read an object */
       if(zone_table[zone].reset_mode==5) break;
       object = real_object(ZCMD.arg1);
       room   = real_room(ZCMD.arg3);
       if(object != -1 && room != -1) {
         if(ZCMD.arg3 >= 0) {

           if((obj=get_obj_in_list_num(object,world[room].contents))) {
             if(obj->obj_flags.type_flag==ITEM_CONTAINER &&
                !IS_SET(obj->obj_flags.value[1], CONT_CLOSED) &&
                !obj->contains)
               extract_obj(obj);
           }

           if(!get_obj_in_list_num(object,world[room].contents)) {
             pop_percent=obj_proto_table[object].obj_flags.repop_percent;
             /*if(rebooting_check && pop_percent<21) pop_percent=pop_percent/2;*/
             if(pop_percent && rebooting_check) pop_percent+=pop_bonus;
             if(BAMDAY) pop_percent=pop_percent+20;
             if(number(1,100)<=pop_percent||
                obj_proto_table[object].obj_flags.type_flag==ITEM_KEY ||
                obj_proto_table[object].obj_flags.type_flag==ITEM_CONTAINER ||
                full) {
               obj = read_object(object, REAL);
               obj_to_room(obj, room);
               last_cmd = 1;
             }
             else
              last_cmd = 0;
           }
           else
             last_cmd = 0;
         }
         else {
           pop_percent=obj_proto_table[object].obj_flags.repop_percent;
           /*if(rebooting_check && pop_percent<21) pop_percent=pop_percent/2;*/
           if(pop_percent && rebooting_check) pop_percent+=pop_bonus;
           if(BAMDAY) pop_percent=pop_percent+10;
           if(number(1,100)<=pop_percent||
              obj_proto_table[object].obj_flags.type_flag==ITEM_KEY||
              obj_proto_table[object].obj_flags.type_flag==ITEM_CONTAINER ||
              full) {
             obj = read_object(object, REAL);
             obj->in_room = NOWHERE;
             last_cmd = 1;
           }
           else
             last_cmd = 0;
         }
       }
       else
         last_cmd = 0;
       break;

     case 'P': /* object to object */
            if(zone_table[zone].reset_mode==5) break;
            object = real_object(ZCMD.arg1);
            object_to = real_object(ZCMD.arg3);
            if(object != -1 && object_to != -1)
              {
               pop_percent=obj_proto_table[object].obj_flags.repop_percent;
               /*if(rebooting_check && pop_percent<21) pop_percent=pop_percent/2;*/
               if(pop_percent && rebooting_check) pop_percent+=pop_bonus;
               if(BAMDAY) pop_percent=pop_percent+10;
               if(number(1,100)<=pop_percent||
             obj_proto_table[object].obj_flags.type_flag==ITEM_KEY||
             obj_proto_table[object].obj_flags.type_flag==ITEM_CONTAINER||
                  full)
             {
                obj = read_object(object, REAL);
             obj_to = get_obj_num(object_to);
             obj_to_obj(obj, obj_to);
             last_cmd = 1;
             }
              }
            else
              last_cmd = 0;
       break;

     case 'T': /* take an object */
       if(zone_table[zone].reset_mode==5) break;
       object = real_object(ZCMD.arg1);
       num = 0;
       room   = real_room(ZCMD.arg3);
       if(object != -1 && room != -1) {
         if(ZCMD.arg3 >= 0) {
           while((!ZCMD.arg2 || ZCMD.arg2>num)  && last_cmd) {
             if((obj=get_obj_in_list_num(object,world[room].contents))) {
                 extract_obj(obj);
                 last_cmd = 1;
                 num++;
             }
             else {
               last_cmd = 0;
             }
           }
           last_cmd = 1;
         }
       }
       else
         last_cmd = 0;
       break;

     case 'G': /* obj_to_char */
     	if (mob)
     		{
            if(zone_table[zone].reset_mode==5) break;
            object = real_object(ZCMD.arg1);
            if(object != -1)
              {
               if(rebooting_check) mob_proto_table[mobile].loads++;
               pop_percent=obj_proto_table[object].obj_flags.repop_percent;
               /*if(rebooting_check && pop_percent<21) pop_percent=pop_percent/2;*/
               if(pop_percent && rebooting_check) pop_percent+=pop_bonus;
               if(BAMDAY) pop_percent=pop_percent+10;
               if(number(1,100)<=pop_percent||
             obj_proto_table[object].obj_flags.type_flag==ITEM_KEY|| full)
             {
                obj = read_object(object, REAL);
             obj_to_char(obj, mob);
             last_cmd = 1;
             }
              }
            }
       break;

     case 'E': /* object to equipment list */
     	if (mob)
     		{
            if(zone_table[zone].reset_mode==5) break;
            if(mob->equipment[ZCMD.arg3]) break;
            object = real_object(ZCMD.arg1);
            if(object != -1)
              {
               if(rebooting_check) mob_proto_table[mobile].loads++;
               pop_percent=obj_proto_table[object].obj_flags.repop_percent;
               /*if(rebooting_check && pop_percent<21) pop_percent=pop_percent/2;*/
               if(pop_percent && rebooting_check) pop_percent+=pop_bonus;
               if(BAMDAY) pop_percent=pop_percent+10;
               if(number(1,100)<=pop_percent||
             obj_proto_table[object].obj_flags.type_flag==ITEM_KEY || full)
             {
                obj = read_object(object, REAL);
             equip_char(mob, obj, ZCMD.arg3);
             last_cmd = 1;
             }
              }
            }
       break;

     case 'D': /* set state of door */
          room  = real_room(ZCMD.arg1);
          if(room != -1)
            {
         switch (ZCMD.arg3)
           {
           case 0:
             if(world[room].dir_option[ZCMD.arg2])
                  {
                 REMOVE_BIT(world[room].dir_option[ZCMD.arg2]->exit_info,
                 EX_LOCKED);
               REMOVE_BIT(world[room].dir_option[ZCMD.arg2]->exit_info,
                 EX_CLOSED);
                  }
             break;
           case 1:
                if(world[room].dir_option[ZCMD.arg2])
                  {
               SET_BIT(world[room].dir_option[ZCMD.arg2]->exit_info,
                  EX_CLOSED);
               REMOVE_BIT(world[room].dir_option[ZCMD.arg2]->exit_info,
                EX_LOCKED);
                  }
             break;
           case 2:
                if(world[room].dir_option[ZCMD.arg2])
                  {
               SET_BIT(world[room].dir_option[ZCMD.arg2]->exit_info,
                 EX_LOCKED);
               SET_BIT(world[room].dir_option[ZCMD.arg2]->exit_info,
                 EX_CLOSED);
                  }
             break;
           }
            }
       break;

     default:
       log_f("Undefd cmd in reset table; zone %d cmd %d.\n\r",
          zone, cmd_no);
       produce_core();
       break;
        }
     else
       last_cmd = 0;
     }
   zone_table[zone].age = 0;
}

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
/* old procedure, commenting out to add a working version, by Ranger. */

/*
int is_empty(int zone_nr)
{
     struct descriptor_data *i;

     for (i = descriptor_list; i; i = i->next)
          if (!i->connected)
               if (world[CHAR_REAL_ROOM(i->character)].zone == zone_nr)
                    return(0);

     return(1);
}
*/

/* Begin new is_empty procedure by Ranger */
/* This one accounts for link dead players in a zone */

int is_empty(int zone_nr) {

  struct char_data *i;

  for (i=character_list; i; i=i->next) {

    if (!IS_NPC(i) && (world[CHAR_REAL_ROOM(i)].zone == zone_table[zone_nr].virtual)){
      return(0);
    }
  }
  return(1);
}

/************************************************************************
*  procs of a (more or less) general utility nature               *
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
     char buf[MSL], tmp[500];
     char *rslt;
     register char *point;
     int flag;

     bzero(buf, MSL);

     do
     {
          if (!fgets(tmp, 500, fl))
          {
               log_f("fread_str");
               break;
/*               produce_core();
*/
          }

          if (strlen(tmp) + strlen(buf) > MSL)
          {
               log_f("fread_string: string too large (db.c)");
               break;
/*               produce_core();
*/
          }
          else
               strcat(buf, tmp);

          for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
               point--);
          if ((flag = (*point == '~')))
               if (*(buf + strlen(buf) - 3) == '\n')
               {
                    *(buf + strlen(buf) - 2) = '\r';
                    *(buf + strlen(buf) - 1) = '\0';
               }
               else
                    *(buf + strlen(buf) -2) = '\0';
          else
          {
               *(buf + strlen(buf) + 1) = '\0';
               *(buf + strlen(buf)) = '\r';
          }
     }
     while (!flag);

     /* do the allocate boogie  */

     if (strlen(buf) > 0)
     {
          CREATE(rslt, char, strlen(buf) + 1);
          strcpy(rslt, buf);
     }
     else
          rslt = 0;
     return(rslt);
}

/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
     struct affected_type_5 *af;

  if (ch->player.name)        free(ch->player.name);
     if (ch->player.title)       free(ch->player.title);
     if (ch->player.short_descr) free(ch->player.short_descr);
     if (ch->player.long_descr)  free(ch->player.long_descr);
     if(ch->player.description)  free(ch->player.description);

     for (af = ch->affected; af; af = af->next)
          affect_remove(ch, af);

     free(ch);
}

/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
  struct extra_descr_data *i_descr = NULL;
  struct extra_descr_data *n_descr = NULL;

  if(obj->name)
    free(obj->name);
  if(obj->description)
    free(obj->description);
  if(obj->short_description)
    free(obj->short_description);
  if(obj->action_description)
    free(obj->action_description);
  if(obj->action_description_nt)
    free(obj->action_description_nt);
  if(obj->char_wear_desc)
    free(obj->char_wear_desc);
  if(obj->room_wear_desc)
    free(obj->room_wear_desc);
  if(obj->char_rem_desc)
    free(obj->char_rem_desc);
  if(obj->room_rem_desc)
    free(obj->room_rem_desc);


  /* if object instance has overridden extra decs, delete them */
  if (obj->ex_description) {
    for (i_descr = obj->ex_description; i_descr; i_descr = n_descr) {
      n_descr = i_descr->next;
      if (i_descr->keyword) free(i_descr->keyword);
      if (i_descr->description) free(i_descr->description);
      free(i_descr);
    }
    obj->ex_description = NULL;
  }

  free(obj);
}

/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
     FILE *fl;
     char tmp[100];

     *buf = '\0';

     if (!(fl = fopen(name, "r")))
     {
          log_f("file-to-string");
          *buf = '\0';
          return(-1);
     }

     do
     {
          fgets(tmp, 99, fl);

          if (!feof(fl))
          {
               if (strlen(buf) + strlen(tmp) + 2 > MSL)
               {
                    log_f("fl->strng: string too big (db.c, file_to_string)");
                    *buf = '\0';
                    return(-1);
               }

               strcat(buf, tmp);
               *(buf + strlen(buf) + 1) = '\0';
               *(buf + strlen(buf)) = '\r';
          }
     }
     while (!feof(fl));

     fclose(fl);

     return(0);
}

/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
     int i;

     for (i = 0; i < MAX_WEAR; i++) /* Initialisering */
          ch->equipment[i] = 0;

     ch->followers = 0;
     ch->master = 0;
     ch->switched = 0;
/*     ch->in_room = NOWHERE; Used for start in room */
     ch->carrying = 0;
     ch->next = 0;
     ch->next_fighting = 0;
     ch->next_in_room = 0;
     ch->specials.fighting = 0;
     ch->specials.position = POSITION_STANDING;
     ch->specials.default_pos = POSITION_STANDING;
     ch->specials.carry_items = 0;
     ch->questgiver = 0;
     ch->questmob = 0;
     ch->questobj = 0;
     ch->questowner = 0;
     ch->quest_status = 0;
     ch->quest_level = 0;

     if (GET_HIT(ch) <= 0)
          GET_HIT(ch) = 1;
     if (GET_MOVE(ch) <= 0)
          GET_MOVE(ch) = 1;
     if (GET_MANA(ch) <= 0)
          GET_MANA(ch) = 1;
}

/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
  memset((char *)ch, '\0', sizeof(struct char_data));

  ch->in_room_r = NOWHERE;
  ch->specials.was_in_room = NOWHERE;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  ch->skills = 0;
  GET_AC(ch) = 100; /* Basic Armor */
  ch->new.prompt = PROMPT_HP | PROMPT_HP_TEX | PROMPT_MANA | PROMPT_MANA_TEX | PROMPT_MOVE | PROMPT_MOVE_TEX | PROMPT_BUFFER | PROMPT_BUFFER_TEX | PROMPT_VICTIM | PROMPT_VICTIM_TEX;
}

void clear_skills(struct char_skill_data *skills)
{
  memset((char *)skills, '\0', sizeof(struct char_skill_data)*MAX_SKILLS5);
}

void clear_object(struct obj_data *obj)
{
     memset((char *)obj, '\0', sizeof(struct obj_data));

     obj->item_number = -1;
     obj->in_room       = NOWHERE;
}

int generate_id(void);
void check_idname(CHAR *ch);
/* initialize a new character only if class is set */
void init_char(struct char_data *ch)
{
     int i;
     struct tm *timeStruct;
     long ct;

     assert(ch->skills);

     /* *** if this is our first player --- he be God *** */

     if (top_of_p_table < 0)
     {
          GET_EXP(ch) = 2000000000;
          GET_LEVEL(ch) = LEVEL_IMP;
     }

     set_title(ch,NULL);

     ch->player.short_descr = 0;
     ch->player.long_descr = 0;
     ch->player.description = 0;

/*     ch->player.hometown = number(1,4); */

     ch->player.time.birth = time(0);
     ch->player.time.played = 1;
     ch->player.time.logon = time(0);
     ct=ch->player.time.birth;
     timeStruct=localtime(&ct);
     ch->ver3.created=(timeStruct->tm_mon+1)*1000000+timeStruct->tm_mday*10000+
                       1900+timeStruct->tm_year;

     for (i = 0; i < MAX_TONGUE; i++)
      ch->player.talks[i] = 0;

     GET_STR(ch) = 9;
     GET_INT(ch) = 9;
     GET_WIS(ch) = 9;
     GET_DEX(ch) = 9;
     GET_CON(ch) = 9;

     /* make favors for sex */
     if (ch->player.sex == SEX_MALE) {
          ch->player.weight = number(120,180);
          ch->player.height = number(160,200);
     } else {
          ch->player.weight = number(100,160);
          ch->player.height = number(150,180);
     }

     ch->points.mana = GET_MAX_MANA(ch);
     ch->points.hit = GET_MAX_HIT(ch);
     ch->points.move = GET_MAX_MOVE(ch);
     ch->points.armor = 100;

     for (i = 0; i <= MAX_SKILLS5 - 1; i++)
     {
          if (GET_LEVEL(ch) < LEVEL_SUP) {
               ch->skills[i].learned = 0;
          }     else {
               ch->skills[i].learned = 100;
          }
     }

     ch->specials.affected_by = 0;
     ch->specials.fighting = 0;
     ch->questgiver = 0;
     ch->questmob = 0;
     ch->questobj = 0;
     ch->questowner = 0;
     ch->quest_status = 0;
     ch->quest_level = 0;
     ch->specials.vaultaccess  = 0;
     ch->specials.wiznetlvl  = 0;
     strcpy(ch->specials.vaultname,"not-set");
     ch->specials.message  = 0;
     ch->specials.zone = -1;
     ch->specials.rider  = 0;
     ch->specials.riding = 0;
     ch->specials.protecting = 0;
     ch->specials.protect_by=0;
     ch->specials.num_fighting=0;
     ch->specials.max_num_fighting=0;
     ch->ver3.clan_num=0;
     ch->ver3.death_limit=0;
     ch->ver3.bleed_limit=0;
     ch->ver3.subclass=0;
     ch->ver3.subclass_points=0;
     ch->ver3.subclass_level=0;
     ch->ver3.time_to_quest=0;
     ch->ver3.quest_points=0;
     ch->ver3.sc_style=0;
     ch->ver3.death_timer=0;
     ch->specials.death_timer=0;
     ch->ver3.id=generate_id();
     check_idname(ch);
     ch->specials.reply_to=0;

     ch->specials.spells_to_learn = 0;
     SET_BIT(ch->specials.pflag,PLR_GOSSIP);
     SET_BIT(ch->specials.pflag,PLR_AUCTION);
     SET_BIT(ch->specials.pflag,PLR_NOKILL);
     SET_BIT(ch->specials.pflag,PLR_QUESTC);
     SET_BIT(ch->specials.pflag,PLR_CHAOS);
     for (i = 0; i < 5; i++)
          ch->specials.apply_saving_throw[i] = 0;

     for (i = 0; i < 3; i++)
          GET_COND(ch, i) = (GET_LEVEL(ch) >= LEVEL_IMM ? -1 : 24);
     for(i=0;i<MAX_COLORS;i++)
          ch->colors[i]=0;
     ch->colors[0]=0;
     ch->colors[1]=8;
     ch->colors[2]=3;
     ch->colors[3]=5;
     ch->colors[4]=7;
     ch->colors[5]=6;
     ch->colors[6]=4;
     ch->colors[7]=15;
     ch->colors[13]=1;
     ch->colors[14]=6;
     ch->colors[15]=6;
}

/* returns the real number of the room with given virtual number */
int real_room(int virtual)
{
     int bot, top, mid;

     if(virtual==NOWHERE) return(-1);

     bot = 0;
     top = top_of_world;

     /* perform binary search on world-table */
     for (;;)
     {
          mid = (bot + top) / 2;

          if ((world + mid)->number == virtual)
               return(mid);
          if (bot >= top)
          {
          /*log_f("Room %d does not exist in database\n", virtual);*/
               return(-1);
          }
          if ((world + mid)->number > virtual)
               top = mid - 1;
          else
               bot = mid + 1;
     }
}

int real_zone(int virtual)
{
  int i;
  int real_zone = -1;
    for(i=0;i<=top_of_zone_table;i++)
      {
      if(zone_table[i].virtual == virtual)
          real_zone = i;
      }
  return real_zone;
}




/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
     int bot, top, mid;

     bot = 0;
     top = top_of_mobt;

     /* perform binary search on mob-table */
     for (;;)
     {
          mid = (bot + top) / 2;

          if ((mob_proto_table + mid)->virtual == virtual)
               return(mid);
          if (bot >= top)
               return(-1);
          if ((mob_proto_table + mid)->virtual > virtual)
               top = mid - 1;
          else
               bot = mid + 1;
     }
}






/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
     int bot, top, mid;

     bot = 0;
     top = top_of_objt;

     /* perform binary search on obj-table */
     for (;;)
     {
          mid = (bot + top) / 2;

          if ((obj_proto_table + mid)->virtual == virtual)
               return(mid);
          if (bot >= top)
               return(-1);
          if ((obj_proto_table + mid)->virtual > virtual)
               top = mid - 1;
          else
               bot = mid + 1;
     }
}


int inzone(int number)
{
int i;
    for(i=0;i<=top_of_zone_table;i++)
      {
      if( number >= zone_table[i].bottom && number <= zone_table[i].top)
           return zone_table[i].virtual;
      }
   return -1;
}

/* MAX_ID in db.h */
void write_board(int vnum,char *heading,char *message);
int generate_id(void) {
  FILE *fl;
  int id;
  char buf[MSL];

  fl=fopen("highest_id", "r");
  if(!fl) id=0;
  else {
    fscanf(fl,"%d\n",&id);
    fclose(fl);
  }
  id++;
  fl=fopen("highest_id","w");
  fprintf(fl,"%d\n",id);
  fclose(fl);

  if(id>MAX_ID-100) { /* send a warning to the board in 1220 */
    sprintf(buf, "WARNING: Player id is %d.  Max currently set to %d.",id,MAX_ID);
    write_board(3097,buf,"Check generate_id in db.c");
  }
  return id;
}

struct idname_struct idname[MAX_ID];

void write_idname (void) {
  int i;
  FILE *fl;

  if(!(fl=fopen("player_idname","wb"))) {
    log_f("Error opening player_idname.");
    return;
  }
  for(i=0;i<MAX_ID;i++) {
    fwrite(&idname[i],sizeof(idname[i]),1,fl);
  }
  fclose(fl);
}

void read_idname (void) {
  int i=0;
  FILE *fl;

  if(!(fl=fopen("player_idname","rb"))) {
    log_f("Error opening player_idname.");
    return;
  }

  for(i=0;i<MAX_ID;i++) {
    if((fread(&idname[i],sizeof(idname[i]),1,fl))!=1) {
      log_f("Error reading player_idname.");
      fclose(fl);
      return;
    }
  }
  fclose(fl);
}

void check_idname(CHAR *ch) {
  int id;
  char tmp_name[50];

  sprintf(tmp_name,"%s",GET_NAME(ch));
  string_to_lower(tmp_name);
  id=ch->ver3.id;

  if(strcmp(idname[id].name,tmp_name)) { /* not a match */
    sprintf(idname[id].name,"%s",tmp_name);
    write_idname();
  }
}
