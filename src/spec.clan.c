/* spec.clan.c
**
** Procedures used in clan file reading, clan house maintenance,
** and any specs required for a given clan house.
**
** This also includes the code for the sheriff badge.
**
** Programmed by Ranger of RoninMUD
**   Creation date: July 3, 1997
**   Last Mod date: January 18, 2003
**
** Added Jesuit Monger clan and changed MAX_CLANS in spec.clan.h to 30. -Liner 01/18/03
** Added add/rem of clan members even if not logged on - Ranger Sept 26, 2000
**
** Not to be distributed without author's consent.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "utility.h"
#include "cmd.h"
#include "comm.h"
#include "spec.clan.h"
#include "spells.h"
#include "interpreter.h"
#include "fight.h"
#include "spec_assign.h"
#include "reception.h"
#include "act.h"

int do_vault(CHAR *mob,CHAR *ch,int cmd,char *arg);
int receptionist(CHAR *mob, CHAR *ch, int cmd, char *arg);
void do_look(struct char_data *ch, char *argument, int cmd);
int meta(CHAR *mob,CHAR *ch, int cmd, char *arg);
void store_to_char_5(struct char_file_u_5 *st, CHAR *ch); /* Latest version is 5 */
void store_to_char_4(struct char_file_u_4 *st, CHAR *ch); 
void store_to_char_2(struct char_file_u_2 *st, CHAR *ch);
void auto_rent(CHAR *ch);
int char_version(FILE *fl);
void rank_char(CHAR *ch);
struct clan_data clan_list[MAX_CLANS];
#define SHERIFF_BADGE 6900

/* Clan list
 1 - Diabolik
 2 - Da Doobies - share with Diabolik
 3 - SOLO
 4 - Blood Moon
 5 - Dark Legion
 6 - HIA
 7 - DraGonHeaRt
 8 - cDc
 9 - High Elves
10 - {Ankh}
11 - Midnight Marauders
12 - KoE
13 - Hellforge
14 - The Firm
15 - MAMFA
16 - ISA
17 - Astral Warriors
18 - Unicorns
19 - Eternal Newbies
20 - CLAN OF BEAR
21 - Jesuit Monger
22 - . - Tabitha's clan
23 - Bud
*/

/* Boards */
#define DIABOLIK_BOARD    27504
#define SOLO_BOARD        27511
#define VERTIGO_BOARD     27518
#define DARK_LEGION_BOARD 27519
#define MM_BOARD          27554
#define ETERNAL_BOARD     27555
#define HE_BOARD          27575
#define TABBY_BOARD       27593
#define ISA_BOARD         27841

/* Access Rooms */
#define DIABOLIK_ACCESS   2902
#define VERTIGO_ACCESS    27516
#define DARK_LEGION_ACCESS 27522
#define SOLO_ACCESS       3065 /* room for pentagram to operate in */
#define MM_ACCESS         28589
#define ETERNAL_ACCESS    27538
#define TABBY_ACCESS      27592
#define HELLIONS_ACCESS   27551
#define HE_ACCESS         11601
#define BUD_ACCESS        3077
#define UNICORNS_ACCESS   3019

/* Entry rooms */
#define BUD_ENTRANCE      27516
#define UNICORNS_ENTRANCE 27553

int clan_top=0;
char *fread_action(FILE *fl);
void load_clan_file(void) {
  FILE *fl;
  int clan_num;
  char temp[64];

  if (!(fl = fopen("clans", "r"))) {
          log_s("Clan file not found (load_clan_file)");
          return;
  }

  while(!feof(fl)) {
    fscanf(fl, "#%d\n", &clan_num);
    sprintf(temp,"Clan Number %d",clan_num);
    if(clan_num==-1) break;
    log_s(temp);
    clan_top=clan_num;
    clan_list[clan_num].name=fread_action(fl);
    fscanf(fl, "%ld\n", &clan_list[clan_num].monthly_rent);
    fscanf(fl, "%d\n", &clan_list[clan_num].paid_to);
          fscanf(fl, "%s\n",temp);
    clan_list[clan_num].head1=str_dup(temp);
          fscanf(fl, "%s\n",temp);
    clan_list[clan_num].head2=str_dup(temp);
    fscanf(fl, "%s\n",temp);
    clan_list[clan_num].head3=str_dup(temp);
        fscanf(fl, "%s\n",temp);
    clan_list[clan_num].head4=str_dup(temp);
          fscanf(fl, "%s\n",temp);
    clan_list[clan_num].head5=str_dup(temp);
  }
  fclose(fl);
}

void save_clan_file(void) {
  FILE *fl;
  int clan_num;

  if(!clan_top) {
    log_s("No clans loaded (save_clan_file)");
    return;
  }
  if (!(fl = fopen("clans", "w"))) {
          log_s("Unable to open for write to clan file");
          return;
  }

  for(clan_num=1;clan_num<=clan_top;clan_num++) {
    fprintf(fl, "#%d\n", clan_num);
    fprintf(fl, "%s\n", clan_list[clan_num].name);
    fprintf(fl, "%ld\n", clan_list[clan_num].monthly_rent);
    fprintf(fl, "%d\n", clan_list[clan_num].paid_to);
    fprintf(fl, "%s\n", clan_list[clan_num].head1);
    fprintf(fl, "%s\n", clan_list[clan_num].head2);
    fprintf(fl, "%s\n", clan_list[clan_num].head3);
    fprintf(fl, "%s\n", clan_list[clan_num].head4);
    fprintf(fl, "%s\n", clan_list[clan_num].head5);
  }
  fprintf(fl, "#-1\n");
  fclose(fl);
}

void make_clan(CHAR *ch, char *arg) {
  if(!*arg) {
    send_to_char("Please supply a clan name.\n\r",ch);
    return;
  }
  if(clan_top==MAX_CLANS) {
    send_to_char("Maximum number of clans reached.\n\r",ch);
    return;
  }
  clan_top++;
  clan_list[clan_top].name=str_dup(arg);
  clan_list[clan_top].monthly_rent=0;
  clan_list[clan_top].paid_to=0;
  clan_list[clan_top].head1=str_dup("undefined");
  clan_list[clan_top].head2=str_dup("undefined");
  clan_list[clan_top].head3=str_dup("undefined");
  clan_list[clan_top].head4=str_dup("undefined");
  clan_list[clan_top].head5=str_dup("undefined");
  save_clan_file();
  send_to_char("Clan initialized, file ready for edit and then reload.\n\r",ch);
}

void list_clan_file(struct char_data *ch,int clan_num) {
  char buf[MAX_STRING_LENGTH],clanlist[80];
  char tmp_name[64];
  FILE *fl;

  if(!clan_top) {
    log_s("No clans loaded (list_clan_file)");
    send_to_char("The clan file is not currently loaded.\n\r",ch);
    return;
  }

  if(!clan_num) {
    for(clan_num=1;clan_num<=clan_top;clan_num++) {
      sprintf(buf, "\n\rClan Number %d.\n\r", clan_num);
      send_to_char(buf,ch);
      sprintf(buf, "     Name: %s\n\r", clan_list[clan_num].name);
      send_to_char(buf,ch);
      sprintf(buf, "     Rent: %ld\n\r", clan_list[clan_num].monthly_rent);
      send_to_char(buf,ch);
      sprintf(buf, "  Paid To: %d\n\r", clan_list[clan_num].paid_to);
      send_to_char(buf,ch);
      sprintf(buf, "    Heads: %s %s %s %s %s\n\r", clan_list[clan_num].head1,
                 clan_list[clan_num].head2, clan_list[clan_num].head3,
                 clan_list[clan_num].head4, clan_list[clan_num].head5);
      send_to_char(buf,ch);
    }
    return;
  }

  sprintf(clanlist,"clanlist.%d",clan_num);
  if(!(fl = fopen (clanlist, "r"))) {
    send_to_char("Clan list not found.\n\r",ch);
    return;
  }

  tmp_name[0]=0;
  while(!feof(fl)) {
    fscanf (fl,"%s\n",tmp_name);
    sprintf(buf,"%s\n\r",tmp_name);
    send_to_char(buf,ch);
  }

  fclose(fl);
}

int check_clanlist_name (char *name, const char *filename) {
  char tmp_name[64];
  FILE *fl;

  if(!(fl=fopen(filename,"r"))) return -1;
  while(!feof(fl)) {
    fscanf(fl,"%s\n",tmp_name);
    if(!strcmp(name,tmp_name)) {
      fclose(fl);
      return 1;
    }
  }
  fclose(fl);
  return 0;
}

void rem_clanlist_name (CHAR *ch) {
  FILE *fl, *real_fl;
  char buf[100],clanlist[80];
  char tmpname[80];
  int ind;

  for (ind=1; ind<=clan_top;ind++) {
    sprintf(tmpname," ");
    sprintf(clanlist,"clanlist.%d",ind);
    if(!(fl=fopen(clanlist,"r"))) return;
    real_fl=fopen("names.tmp", "w");
    while(!feof(fl)) {
      fscanf(fl,"%s\n",tmpname);
      if(strcmp(GET_NAME(ch),tmpname))
        fprintf(real_fl,"%s\n",tmpname);
    }
    fclose(fl);
    fclose(real_fl);
    sprintf(buf,"mv names.tmp %s",clanlist);
    system(buf);
  }
}

void add_clanlist_name (CHAR *ch, int clan) {
  FILE *fl;
  char clanlist[80];

  sprintf(clanlist,"clanlist.%d",clan);
  if(check_clanlist_name(GET_NAME(ch), clanlist)) return;
  rem_clanlist_name(ch);
  fl=fopen(clanlist, "a+");
  fprintf (fl, "%s\n", GET_NAME(ch));
  fclose (fl);
}

int check_clan_board(int board,struct char_data *ch) {

  if(GET_LEVEL(ch)>LEVEL_SUP && IS_SET(ch->new.imm_flags, WIZ_ACTIVE)) return TRUE;
  switch(ch->ver3.clan_num) {
    case 0:
      break;
    case 1:
      if(board==DIABOLIK_BOARD) return TRUE;
      break;
    case 2:
      if(board==DIABOLIK_BOARD) return TRUE;
      break;
    case 3:
      if(board==SOLO_BOARD) return TRUE;
      break;
    case 4:
      if(board==VERTIGO_BOARD) return TRUE;
      break;
    case 5:
      if(board==DARK_LEGION_BOARD) return TRUE;
      break;
    case 9:
    	if(board==HE_BOARD) return TRUE;
    	break;
    case 11:
    case 13:
      if(board==MM_BOARD) return TRUE;
      break;
    case 19:
      if(board==ETERNAL_BOARD) return TRUE;
      break;
    case 22:
      if(board==TABBY_BOARD) return TRUE;
      break;
    case 16:
      if(board==ISA_BOARD) return TRUE;
    default:
      break;
  }
  return FALSE;
}

int check_clan_access(int room, struct char_data *ch) {

  if(GET_LEVEL(ch)>LEVEL_SUP) return TRUE;
  switch(ch->ver3.clan_num) {
    case 0:
      break;
    case 1:
      if(room==DIABOLIK_ACCESS) return TRUE;
      break;
    case 2:
      if(room==DIABOLIK_ACCESS) return TRUE;
      break;
    case 4:
      if(room==VERTIGO_ACCESS) return TRUE;
      break;
    case 5:
      if(room==DARK_LEGION_ACCESS) return TRUE;
      break;
    case 9:
    	if(room==HE_ACCESS) return TRUE;
    	break;
    case 11:
    case 13:
      if(room==MM_ACCESS) return TRUE;
      break;
    case 18: // Unicorns
      if (room == UNICORNS_ACCESS) return TRUE;
      break;
    case 19:
      if(room==ETERNAL_ACCESS) return TRUE;
      break;
    case 22:
      if(room==TABBY_ACCESS) return TRUE;
      break;
    case 23:
      if (room == BUD_ACCESS) return TRUE;
      break;
#ifdef TEST_SITE
    case 25:
      if (room == HELLIONS_ACCESS) return TRUE;
      break;
#endif
    default:
      break;
  }
  return FALSE;
}

void do_clan(struct char_data *ch, char *argument, int cmd) {
  FILE *fl;
  struct char_file_u_5 char_data_5;
  struct char_file_u_4 char_data_4;
  struct char_file_u_2 char_data_2;
  char arg[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH],name[MAX_INPUT_LENGTH],tmp_name[50];
  int clan,month,year,logon=0,version,vict_is_mob=0,vict_is_pc=0;
  CHAR *vict=0;

  if(IS_NPC(ch)) return;
  if(!ch->ver3.clan_num && GET_LEVEL(ch)<LEVEL_SUP) {
    send_to_char("Sorry, you don't belong to a clan.\n\r",ch);
    return;
  }

  argument=one_argument(argument, arg);
  if(!*arg) {
    if(GET_LEVEL(ch)<LEVEL_SUP) {
      send_to_char("\n\r\
Usage: clan add <name> (needs leader access)\n\r\
            remove <name> (needs leader access)\n\r\
            status     (displays clan status)\n\r\
            list       (lists all current members)\n\r",ch);
    } else {
      send_to_char("\n\r\
Usage: clan add <name> <num>\n\r\
       remove <name> <num>\n\r\
       status <num> (displays clan status)\n\r\
       list <num> (no number given - lists all clans)\n\r\
       reload\n\r\
       make <name>\n\r",ch);
    }
    return;
  }

  if(is_abbrev(arg,"list")) {
    if(GET_LEVEL(ch)>=LEVEL_SUP) {
      one_argument(argument,arg);
      if(!*arg) {
        clan=0;
      }
      else {
        if(!isdigit(*arg)) {
          send_to_char("That isn't a valid number.\n\r",ch);
          return;
        }
        clan=atoi(arg);
      }
    }
    else {
      clan=ch->ver3.clan_num;
    }

    if(clan<0 || clan > clan_top) {
      send_to_char("Invalid Clan Number.\n\r",ch);
      return;
    }
    list_clan_file(ch,clan);
    return;
  }

  if(is_abbrev(arg,"make") && GET_LEVEL(ch)>=LEVEL_SUP) {
    make_clan(ch,argument);
    return;
  }

  if(is_abbrev(arg,"reload") && GET_LEVEL(ch)>=LEVEL_SUP) {
    load_clan_file();
    send_to_char("Clan file reloaded.\n\r",ch);
    return;
  }

  if(is_abbrev(arg,"status")) {
    if(GET_LEVEL(ch)>=LEVEL_SUP) {
      one_argument(argument,arg);
      if(!*arg) {
        send_to_char("Please supply a clan number.\n\r",ch);
        return;
      }
      if(!isdigit(*arg)) {
        send_to_char("That isn't a valid number.\n\r",ch);
        return;
      }
      clan=atoi(arg);
      if(clan<1 || clan > clan_top) {
        send_to_char("Invalid Clan Number.\n\r",ch);
        return;
      }
    }
    else clan=ch->ver3.clan_num;

    sprintf(buf,"\n\rClan Name: %s\n\r",clan_list[clan].name);
    send_to_char(buf,ch);
    if(clan_list[clan].monthly_rent) {
      month=clan_list[clan].paid_to/100;
      year=clan_list[clan].paid_to-month*100;
      sprintf(buf, "     Rent: %ld\n\r", clan_list[clan].monthly_rent);
      send_to_char(buf,ch);
      sprintf(buf, "  Paid To: %d-%d\n\r", month,year);
      send_to_char(buf,ch);
    } else {
      send_to_char("     Rent: None\n\r",ch);
    }
    sprintf(buf, "    Heads: %s %s %s %s %s\n\r", clan_list[clan].head1,
               clan_list[clan].head2, clan_list[clan].head3,
               clan_list[clan].head4, clan_list[clan].head5);
    send_to_char(buf,ch);
    return;
  }

  if(is_abbrev(arg,"pay")) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) {
      send_to_char("\n\rYou must pay with your mortal.\n\r",ch);
      return;
    }
    clan=ch->ver3.clan_num;
    if(clan_list[clan].monthly_rent==0) {
      send_to_char("\n\rYou have no rent, no need to pay.\n\r",ch);
      return;
    }
    if(GET_GOLD(ch)<clan_list[clan].monthly_rent) {
      send_to_char("\n\rSorry, you don't have enough gold.\n\r",ch);
      return;
    }
    GET_GOLD(ch)-=clan_list[clan].monthly_rent;
    month=clan_list[clan].paid_to/100;
    year=clan_list[clan].paid_to-month*100;
    month++;
    if(month>12) {
      year++;
      month=1;
    }
    clan_list[clan].paid_to=month*100+year;
    save_clan_file();
    sprintf(buf,"\n\rYour rent is paid to %d-%d.\n\r",month,year);
    send_to_char(buf,ch);
    return;
  }

  if(is_abbrev(arg,"add")) {
    argument=one_argument(argument,tmp_name);
    if(!*tmp_name) {
      send_to_char("\n\rAdd who to the clan?.\n\r",ch);
      return;
    }

    if(GET_LEVEL(ch)>=LEVEL_SUP) {
      one_argument(argument,arg);
      if(!*arg) {
        send_to_char("Please supply a clan number.\n\r",ch);
        return;
      }
      if(!isdigit(*arg)) {
        send_to_char("That isn't a valid number.\n\r",ch);
        return;
      }
      clan=atoi(arg);
      if(clan<1 || clan > clan_top) {
        send_to_char("Invalid Clan Number.\n\r",ch);
        return;
      }
    } else {
      clan=ch->ver3.clan_num;
    }

    vict=get_char(tmp_name);

    strcpy(name,GET_NAME(ch));
    string_to_lower(name);
    if(!strcmp(name,"undefined")) {
      send_to_char("\n\rSorry, you cannot give clan access to other players.\n\r",ch);
      return;
    }

    if(GET_LEVEL(ch)<LEVEL_SUP && strcmp(name,clan_list[clan].head1) &&
       strcmp(name,clan_list[clan].head2) && strcmp(name,clan_list[clan].head3) &&
       strcmp(name,clan_list[clan].head4) && strcmp(name,clan_list[clan].head5)) {
      send_to_char("\n\rSorry, you cannot give clan access to other players.\n\r",ch);
      return;
    }

    if(vict) {
      if(IS_NPC(vict)) {
        vict_is_mob=1;
        vict=0;
      }
    }

    if(!vict) {
      sprintf(buf,"cp rent/%c/%s.dat rent/%c/%s.bak",UPPER(tmp_name[0]),tmp_name,UPPER(tmp_name[0]),tmp_name);
      system(buf);
      sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);
      if (!(fl = fopen(buf, "rb"))) {
        sprintf(buf,"%s doesn't exist.\n\r",CAP(tmp_name));
        send_to_char(buf,ch);
        return;
      }

      version=char_version(fl);

      switch(version) {
        case 2:
          if((fread(&char_data_2,sizeof(struct char_file_u_2),1,fl))!=1)
          {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
          break;
        case 3:
          if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
          {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
          break;
        case 4:
          if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
          {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
          break;
        case 5:
          if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1)
          {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
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
      save_char(vict, NOWHERE);
      vict->next = character_list;
      character_list = vict;
      char_to_room(vict, CHAR_REAL_ROOM(ch));
      if(GET_LEVEL(vict)<LEVEL_IMM) rank_char(vict);
      log_f("WIZINFO: %s logged on to change clan status",GET_NAME(vict));
      logon=1;
      vict_is_pc=1;
    }

    if(vict_is_mob && !vict_is_pc) {
      send_to_char("\n\rYou want a mob to join your clan?\n\r",ch);
      return;
    }

    if(vict->ver3.clan_num!=clan) {
      vict->ver3.clan_num=clan;
      sprintf(buf,"\n\rYou are now a part of the %s clan.\n\r",clan_list[clan].name);
      send_to_char(buf,vict);
      sprintf(buf,"\n\r%s is now a part of the %s clan.\n\r",GET_NAME(vict),clan_list[clan].name);
      send_to_char(buf,ch);
      add_clanlist_name(vict,clan);
    }
    else {
      sprintf(buf,"\n\r%s is already a member the %s clan.\n\r",GET_NAME(vict),clan_list[clan].name);
      send_to_char(buf,ch);
    }
    if(logon) { /* logoff vict */
      auto_rent(vict);
    }
    return;
  }

  if(is_abbrev(arg,"remove")) {
    argument=one_argument(argument,tmp_name);
    if(!*tmp_name) {
      send_to_char("\n\rRemove who from the clan?.\n\r",ch);
      return;
    }

    if(GET_LEVEL(ch)>=LEVEL_SUP) {
      one_argument(argument,arg);
      if(!*arg) {
        send_to_char("Please supply a clan number.\n\r",ch);
        return;
      }
      if(!isdigit(*arg)) {
        send_to_char("That isn't a valid number.\n\r",ch);
        return;
      }
      clan=atoi(arg);
      if(clan<1 || clan > clan_top) {
        send_to_char("Invalid Clan Number.\n\r",ch);
        return;
      }
    }
    else clan=ch->ver3.clan_num;

    vict=get_char(tmp_name);

    strcpy(name,GET_NAME(ch));
    string_to_lower(name);
    if(!strcmp(name,"undefined")) {
      send_to_char("\n\rSorry, you cannot remove clan access from other players.\n\r",ch);
      return;
    }

    if(GET_LEVEL(ch)<LEVEL_SUP && strcmp(name,clan_list[clan].head1) &&
       strcmp(name,clan_list[clan].head2) && strcmp(name,clan_list[clan].head3) &&
       strcmp(name,clan_list[clan].head4) && strcmp(name,clan_list[clan].head5)) {
      send_to_char("\n\rSorry, you cannot remove clan access from other players.\n\r",ch);
      return;
    }

    if(vict) {
      if(IS_NPC(vict)) {
        vict_is_mob=1;
        vict=0;
      }
    }

    if(!vict) {
      sprintf(buf,"cp rent/%c/%s.dat rent/%c/%s.bak",UPPER(tmp_name[0]),tmp_name,UPPER(tmp_name[0]),tmp_name);
      system(buf);
      sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);
      if (!(fl = fopen(buf, "rb"))) {
        sprintf(buf,"%s doesn't exist.\n\r",CAP(tmp_name));
        send_to_char(buf,ch);
        return;
      }

      version=char_version(fl);

      switch(version) {
        case 2:
          if((fread(&char_data_2,sizeof(struct char_file_u_2),1,fl))!=1)
          {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
          break;
        case 3:
          if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
          {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
          break;
        case 4:
          if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
          {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
          break;
        case 5:
          if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1)
          {log_s("Error Reading rent file(logon_char)");fclose(fl);return;}
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
      save_char(vict, NOWHERE);
      vict->next = character_list;
      character_list = vict;
      char_to_room(vict, CHAR_REAL_ROOM(ch));
      if(GET_LEVEL(vict)<LEVEL_IMM) rank_char(vict);
      log_f("WIZINFO: %s logged on to change clan status",GET_NAME(vict));
      vict_is_pc=1;
      logon=1;
    }

    if(vict_is_mob && !vict_is_pc) {
      send_to_char("\n\rYou want a mob removed from your clan?\n\r",ch);
      return;
    }

    if(vict->ver3.clan_num!=clan) {
      sprintf(buf,"\n\r%s is not a part of the %s clan.\n\r",GET_NAME(vict),clan_list[clan].name);
      send_to_char(buf,ch);
    }
    else {
      vict->ver3.clan_num=0;
      sprintf(buf,"\n\rYou are no longer a member of the %s clan.\n\r",clan_list[clan].name);
      send_to_char(buf,vict);
      sprintf(buf,"\n\r%s is no longer a member of the %s clan.\n\r",GET_NAME(vict),clan_list[clan].name);
      send_to_char(buf,ch);
      rem_clanlist_name(vict);
    }
    if(logon) { /* logoff vict */
      auto_rent(vict);
    }
    return;
  }
  send_to_char("Invalid Option, type 'clan' for a list.\n\r",ch);
}


int sheriff_badge(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  char arg[MAX_STRING_LENGTH];
  CHAR *vict;

  if(!ch) return FALSE;
  if(cmd!=CMD_UNKNOWN) return FALSE;
  argument=one_argument(argument,arg);
  if(!*arg) return FALSE;
  if(!obj->equipped_by) return FALSE;
  if(obj->equipped_by!=ch) return FALSE;
  if(!is_abbrev(arg,"deputize")) return FALSE;
  one_argument(argument,arg);
  if(!*arg) {
    send_to_char("Deputize who?\n\r",ch);
    return TRUE;
  }
  if(!(vict=get_char_vis(ch,arg))) {
    send_to_char("That person isn't here.\n\r",ch);
    return TRUE;
  }
  if(CHAR_REAL_ROOM(ch)!=CHAR_REAL_ROOM(vict)) {
    send_to_char("That person isn't here.\n\r",ch);
    return TRUE;
  }
  if(IS_SET(vict->specials.pflag, PLR_DEPUTY)) {
    send_to_char("Deputy status removed.\n\r",ch);
    send_to_char("Your deputy status has been removed.\n\r",vict);
    REMOVE_BIT(vict->specials.pflag, PLR_DEPUTY);
    return TRUE;
  }
  else {
    send_to_char("Deputy status added.\n\r",ch);
    send_to_char("You are now a Midgaard Deputy!\n\r",vict);
    SET_BIT(vict->specials.pflag, PLR_DEPUTY);
    return TRUE;
  }
  return FALSE;
}


/* All scrolls for clan halls should be type scroll,
   holdable and the first value should be the location
   to recall the player to.  All other values are
   meaningless.

   For each new recall scroll,
     assign_obj(vnumber,clan_scroll_recall);
   needs to be added to the assign_clan() procedure.
*/

int clan_recall_scroll(OBJ *scroll,CHAR *ch,int cmd,char *argument) {
  char buf[100];
  CHAR *victim=0,*vict;
  OBJ *tmp_obj;
  int location,org_room,bits=0;
  bool mount;

  if(cmd!=CMD_RECITE) return FALSE;
  if(!ch) return FALSE;
  if(ch!=scroll->carried_by && ch!=scroll->equipped_by) return FALSE;

  argument=one_argument(argument,buf);
  if(!isname(buf,OBJ_NAME(scroll))) return FALSE;

  if(*argument) {
    bits = generic_find(argument, FIND_CHAR_ROOM, ch, &victim, &tmp_obj);
    if(bits == 0) {
      send_to_char("That character isn't here.\n\r", ch);
      return TRUE;
    }
  }

  act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  act("You recite $p which dissolves.",FALSE,ch,scroll,0,TO_CHAR);

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_MAGIC)) {
    send_to_char("Your magic has been absorbed by the surroundings.\n\r", ch);
    extract_obj(scroll);
    return TRUE;
  }

  if(!victim) victim=ch;
  location=real_room(scroll->obj_flags.value[0]);

  if(location == NOWHERE) {
    send_to_char("Your scroll doesn't have a valid destination.\n\r", ch);
    return TRUE;
  }
  if(IS_NPC(victim) && !(IS_AFFECTED(victim, AFF_CHARM))) {
    send_to_char("You can't recall creatures.\n\r",ch);
    return TRUE;
  }
  if(victim->specials.rider) {
    send_to_char("That creature has a rider.\n\r",ch);
    return TRUE;
  }
  if (IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags, CHAOTIC)) {
    send_to_char("Chaotic energies disrupt your magic!\n\r",ch);
    return TRUE;
  }

  mount=FALSE;
  org_room = CHAR_REAL_ROOM(victim);
  if(victim->specials.riding) {
    if (org_room == CHAR_REAL_ROOM(victim->specials.riding)) {
      vict = victim->specials.riding;
      mount=TRUE;
    }
  }

  if (mount) act("$n disappears with $s mount.", TRUE, victim, 0, 0, TO_ROOM);
  else act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  if(victim->specials.fighting)
    stop_fighting(victim);
  char_to_room(victim, location);
  victim->specials.was_in_room=NOWHERE;
  if (mount) {
     char_from_room(vict);
     if(vict->specials.fighting)
       stop_fighting(vict);
     char_to_room(vict,location);
     vict->specials.was_in_room=NOWHERE;
  }
  if(mount) act("$n appears in the middle of the room with $s mount.", TRUE, victim, 0, 0, TO_ROOM);
  else act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  do_look(victim, "",CMD_LOOK);

  extract_obj(scroll);
  return TRUE;
}

/* Rest of file are procs for specific clans */

/* Diabolik */

#define DIABOLIK_FOUNTAIN     27501
#define DIABOLIK_GARGOYLE     27501
#define DIABOLIK_HALL         27501
#define DIABOLIK_BARKEEP      27503
#define DIABOLIK_LIBRARY      27504

int diabolik_ring(int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  CHAR *mob=0;
  OBJ *obj=0;


  if(cmd==MSG_TICK) {
    if(real_room(DIABOLIK_HALL)==NOWHERE) return FALSE;
    if(real_room(DIABOLIK_ACCESS)==NOWHERE) return FALSE;
    if(world[real_room(DIABOLIK_HALL)].dir_option[EAST]->to_room_r == real_room(DIABOLIK_ACCESS)) {
      world[real_room(DIABOLIK_ACCESS)].dir_option[WEST]->to_room_r = real_room(0);
      world[real_room(DIABOLIK_HALL)].dir_option[EAST]->to_room_r = real_room(0);
    }
    return FALSE;
  }

  if (cmd==CMD_USE) {
    if (!ch) return FALSE;
    if(real_room(DIABOLIK_HALL)==NOWHERE) return FALSE;
    if(real_room(DIABOLIK_ACCESS)==NOWHERE) return FALSE;
     arg=one_argument(arg,buf);
    if (!*buf) return FALSE;
    if(!strcmp(buf,"ring")) {
      if(!check_clan_access(DIABOLIK_ACCESS,ch)) {
        if(!mob_proto_table[real_mobile(DIABOLIK_GARGOYLE)].number) {
          mob=read_mobile(DIABOLIK_GARGOYLE, VIRTUAL);
                      if((obj = get_obj_in_list_vis(ch, "fountain", world[real_room(DIABOLIK_HALL)].contents)))
                        extract_obj(obj);
          char_to_room(mob, real_room(DIABOLIK_HALL));
          act("The fountain begins to crack and move as it senses an intruder!",1,mob,0,0,TO_ROOM);
          /* close up internal exits
          South - Da Doobies' House of Hemp [27500] (CLUB)
          West  - Diabolik Room of Ancient Knowledge [27504]
          Up    - The Rusty Sword [27503] (SAFE) (CLUB)
          */
          world[real_room(DIABOLIK_HALL)].dir_option[SOUTH]->to_room_r = real_room(0);
          world[real_room(DIABOLIK_HALL)].dir_option[WEST]->to_room_r = real_room(0);
          world[real_room(DIABOLIK_HALL)].dir_option[UP]->to_room_r = real_room(0);
        }
      }
      if(world[real_room(DIABOLIK_HALL)].dir_option[EAST]->to_room_r == real_room(DIABOLIK_ACCESS)) {
        act("You pull hard on the ring, but the drawbridge is already open!",TRUE,ch,0,0,TO_CHAR);
      }
      else {
        world[real_room(DIABOLIK_HALL)].dir_option[EAST]->to_room_r = real_room(DIABOLIK_ACCESS);
        send_to_room ("You hear a gentle thud as the drawbridge gently falls into its open position.\n\r",real_room(DIABOLIK_HALL));
        world[real_room(DIABOLIK_ACCESS)].dir_option[WEST]->to_room_r = real_room(DIABOLIK_HALL);
        send_to_room ("The drawbridge thuds gently on the ground as it reaches its fully opened position.\n\r",real_room(DIABOLIK_ACCESS));
      }
      return TRUE;
    }
  }
  return FALSE;
}

int diabolik_library(int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  OBJ *tmp;
  int temp = 0;
  if(!ch) return(FALSE);
  if (cmd != CMD_LOOK || !AWAKE (ch)) return FALSE;
  one_argument (arg,buf);
  if (strcmp(buf,"shelf")) return FALSE;

  if(number(0,20)!=0) {
    send_to_char ("Your search turns up nothing of use.\n\r",ch);
    return TRUE;
  }
  else {
    act ("$n looks diligently through the shelves and finds a scroll!", TRUE, ch, 0, 0, TO_ROOM);
    send_to_char ("You search and search and eventually find a scroll of value!\n\r",ch);
    switch (number(1,6)) {
      case 1: temp = 3050; /* ident */
          break;
      case 2: temp = 13016; /* earthquake */
          break;
      case 3: temp = 9423; /* cure critic */
          break;
      case 4: temp = 5216; /* detect align/invis/magic */
          break;
      case 5: temp = 3424; /* colorspray/create food/magic miss*/
          break;
      case 6: temp = 3052; /* recall */
          break;
    }
    tmp = read_object(temp,VIRTUAL);
    obj_to_char(tmp,ch);
    return TRUE;
  }
  return FALSE;
}

void spell_word_of_recall(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
int diabolik_gargoyle (CHAR *garg, CHAR *ch, int cmd, char *arg) {
  OBJ *obj;

  if(cmd==MSG_TICK && !garg->specials.fighting) {
    extract_char(garg);
    obj = read_object(DIABOLIK_FOUNTAIN, VIRTUAL);
    obj_to_room(obj, real_room(DIABOLIK_HALL));

    /* close external exit */
    if(real_room(DIABOLIK_HALL)==NOWHERE) return FALSE;
    if(real_room(DIABOLIK_ACCESS)==NOWHERE) return FALSE;
    if(world[real_room(DIABOLIK_HALL)].dir_option[EAST]->to_room_r == real_room(DIABOLIK_ACCESS)) {
      world[real_room(DIABOLIK_ACCESS)].dir_option[WEST]->to_room_r = real_room(0);
      world[real_room(DIABOLIK_HALL)].dir_option[EAST]->to_room_r = real_room(0);
    }
    /* open up internal exits
    South - Da Doobies' House of Hemp [27500] (CLUB)
    West  - Diabolik Room of Ancient Knowledge [27504]
    Up    - The Rusty Sword [27503] (SAFE) (CLUB)
    */
    world[real_room(DIABOLIK_HALL)].dir_option[SOUTH]->to_room_r = real_room(27500);
    world[real_room(DIABOLIK_HALL)].dir_option[WEST]->to_room_r = real_room(27504);
    world[real_room(DIABOLIK_HALL)].dir_option[UP]->to_room_r = real_room(27503);
    return FALSE;
  }

  if(!ch) return FALSE;
  if(cmd==MSG_ENTER && !check_clan_access(DIABOLIK_ACCESS,ch)) {
    act("$n bursts forth from its stony covering and attacks $N!",1,garg,0,ch,TO_NOTVICT);
    act("You burst forth from your stony covering and attack $N!",1,garg,0,ch,TO_CHAR);
    act("$n bursts forth from its stony covering and attacks you!",1,garg,0,ch,TO_VICT);
    hit(garg, ch, TYPE_CLAW);
  }
  return FALSE;
}

int gruumsh_barkeep(CHAR *mob, CHAR *ch, int cmd, char *arg){
  char buf[MAX_STRING_LENGTH];
  OBJ *tmp;
  if(GET_POS(mob)<=POSITION_FIGHTING) return(FALSE);
  if(CHAR_REAL_ROOM(mob)!=real_room(27503)) return(FALSE);

   if (cmd==CMD_LIST) { /* List */

    char* message1 ="            *** The Rusty Sword ***\n\r";
    char* message2 ="              Specials of the Day: \n\r";
    char* message3 ="                                   \n\r";
    char* message4 ="       Diabolik house reserve ale - 10 coins\n\r";
    char* message5 ="          Doobie Space Cake - 10 coins\n\r\n\r";
    if (!ch) return FALSE;
    send_to_char(message1,ch);
    send_to_char(message2,ch);
    send_to_char(message3,ch);
    send_to_char(message4,ch);
    send_to_char(message5,ch);
    return(TRUE);
  }

  if (cmd==CMD_BUY) { /* Buy */

    arg = one_argument(arg, buf);

    if (!*buf)
      {
        send_to_char("Buy what?\n\r",ch);
        return(TRUE);
      }
    if (!strcmp(buf,"ale"))
      {
        if (GET_GOLD(ch)<10)
          {
            send_to_char("You don't have enough money.\n\r",ch);
            return(TRUE);
          }
        else
          {
            tmp=read_object(27503,VIRTUAL);
            obj_to_char(tmp,ch);
            GET_GOLD(ch)-=10;
            send_to_char("That will cost you 10 coins.\n\r",ch);
            act("$n floats the $o to $N's hand using his eye of levitation.",1,mob,tmp,ch,TO_NOTVICT);
            act("$n floats the $o to your hand using his eye of levitation.",1,mob,tmp,ch,TO_VICT);

             return(TRUE);
          }
      }
    else
    if (!strcmp(buf,"cake"))
      {
        if (GET_GOLD(ch)<10)
          {
            send_to_char("You don't have enough money.\n\r",ch);
            return(TRUE);
          }
        else
          {
            tmp=read_object(27505,VIRTUAL);
            obj_to_char(tmp,ch);
            GET_GOLD(ch)-=10;
            send_to_char("That's 10 coins.  Enjoy.\n\r",ch);
            act("$n floats the $o to $N's hand using his eye of levitation.",1,mob,tmp,ch,TO_NOTVICT);
            act("$n floats the $o to your hand using his eye of levitation.",1,mob,tmp,ch,TO_VICT);
             return(TRUE);
      }
   }
}
   return(FALSE);
}

/* End of Diabolik */

/* Clan SOLO */

/* Portal obj & guard */

int portal_guard(CHAR *mob, CHAR *ch, int cmd, char *arg){
  char buf[MAX_STRING_LENGTH];
  OBJ *tmp;
  if(!ch) return FALSE;
  if(GET_POS(mob)<=POSITION_FIGHTING) return(FALSE);
  if(CHAR_REAL_ROOM(mob)!=real_room(27510)) return(FALSE);

  if (cmd==CMD_LIST) {
    if (!ch) return FALSE;
    send_to_char("You can buy a pentagram, absolutely free!\n\r",ch);
    return(TRUE);
  }

  if (cmd==CMD_BUY) {
    arg = one_argument(arg, buf);
    if (!*buf) {
      send_to_char("Buy what?\n\r",ch);
      return(TRUE);
    }
    if(!strcmp(buf,"pentagram")) {
            tmp=read_object(27510,VIRTUAL);
      obj_to_char(tmp,ch);
            send_to_char("That will cost you 10 coins.\n\r",ch);
      act("A pentagram suddenly appears in $N's hands.",1,mob,0,ch,TO_NOTVICT);
      act("A pentagram suddenly appears in your hands.",1,mob,0,ch,TO_VICT);
      return(TRUE);
          }
          send_to_char("Nothing at all by that name.\n\r",ch);
  }
  return(FALSE);
}

int clan_portal(OBJ *portal, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(cmd==MSG_TICK) {
    if(portal->in_room!=NOWHERE) {
      send_to_room("The firey portal folds in upon itself with a loud *pop*.\n\r",portal->in_room);
      extract_obj(portal);
    }
    return FALSE;
  }

  if(!ch) return(FALSE);
  if (cmd != CMD_ENTER && !check_clan_access(SOLO_ACCESS,ch)) return FALSE;
  if (!AWAKE (ch)) return FALSE;

  one_argument(arg,buf);
  if (!isname(buf, OBJ_NAME(portal)))
    return FALSE;

  if(GET_POS(ch)==POSITION_RIDING) {
     send_to_char("Dismount first.\n\r",ch);
     return TRUE;
  }

  if(GET_POS(ch)<POSITION_STANDING) {
    send_to_char("You must be standing.\n\r",ch);
    return TRUE;
  }

  send_to_char("You enter the flaming surface of the portal.\n\r",ch);
  act ("$n's body is consumed by the flaming surface of the portal..",TRUE,ch,0,0,TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, real_room(27510));
  act ("$n appears out of nowhere.",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  return TRUE;
}

int clan_pentagram(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  char arg[MAX_STRING_LENGTH];
  OBJ *portal;

  if(!ch) return FALSE;
  if(cmd!=CMD_UNKNOWN) return FALSE;
  argument=one_argument(argument,arg);
  if(!*arg) return FALSE;
  if(!obj->carried_by) return FALSE;
  if(obj->carried_by!=ch) return FALSE;
  if(!is_abbrev(arg,"pentagram")) return FALSE;
  if(V_ROOM(ch)!= SOLO_ACCESS) {
    send_to_char("The pentagram quickly glows red hot, but soon fades.\n\r",ch);
    return TRUE;
  }

  if (ch->ver3.clan_num == 3) {
  send_to_char("As you direct your will towards the pentagram, a firey portal begins to take form before you.\n\r",ch);
  act("As $n starts to concentrate, you notice a firey red portal shimmer into existance before $m.",0,ch,0,0,TO_ROOM);
  portal = read_object(27512, VIRTUAL);
  obj_to_room(portal, CHAR_REAL_ROOM(ch));
  return TRUE;
  } else {
    send_to_char("The pentagram quickly glows red hot, but soon fades.\n\r",ch);
    return TRUE;
  }
}


int lilith_shopkeep(CHAR *mob, CHAR *ch, int cmd, char *arg){
  char buf[MAX_STRING_LENGTH];
  OBJ *tmp;
  if(GET_POS(mob)<=POSITION_FIGHTING) return(FALSE);
  if(CHAR_REAL_ROOM(mob)!=real_room(27515)) return(FALSE);

   if (cmd==CMD_LIST) { /* List */
    if (!ch) return FALSE;
    send_to_char("\
         Specials of the Day:\n\r\
  Tasty paladin steaks - 10 coins\n\r\
  Rusty fork           - 10 coins\n\r\
  Scroll of the damned - 10 coins\n\r\n\r",ch);
    return(TRUE);
  }

  if (cmd==CMD_BUY) { /* Buy */

    arg = one_argument(arg, buf);

    if (!*buf)
      {
        send_to_char("Buy what?\n\r",ch);
        return(TRUE);
      }
    if (!strcmp(buf,"fork"))
      {
        if (GET_GOLD(ch)<10)
          {
            send_to_char("You don't have enough money.\n\r",ch);
            return(TRUE);
          }
        else
          {
            tmp=read_object(27515,VIRTUAL);
            obj_to_char(tmp,ch);
            GET_GOLD(ch)-=10;
            send_to_char("That will cost you 10 coins.\n\r",ch);
            act("$n viciously jabs a pathetic, snivelling paladin in the ribs with a rusted fork!",1,mob,tmp,ch,TO_NOTVICT);
            act("$n hands the $o to $N with a wicked grin.",1,mob,tmp,ch,TO_NOTVICT);
            act("$n viciously jabs a pathetic, snivelling paladin in the ribs with a rusted fork!",1,mob,tmp,ch,TO_VICT);
            act("$n hands you the $o, a wickedly evil grin spreading across $s face.",1,mob,tmp,ch,TO_VICT);

             return(TRUE);
          }
      }
    else
    if (!strcmp(buf,"steak"))
      {
        if (GET_GOLD(ch)<10)
          {
            send_to_char("You don't have enough money.\n\r",ch);
            return(TRUE);
          }
        else
          {
            tmp=read_object(1,VIRTUAL);
            obj_to_char(tmp,ch);
            GET_GOLD(ch)-=10;
            send_to_char("That's 10 coins.  Enjoy.\n\r",ch);
            act("$n licks the $o devilishly and exclaims 'yummy!' before handing it to $N.",1,mob,tmp,ch,TO_NOTVICT);
            act("$n licks the $o devilishly and exclaims 'yummy!' before handing it to you.",1,mob,tmp,ch,TO_VICT);
             return(TRUE);
      }
   }
    else
    if (!strcmp(buf,"scroll"))
      {
        if (GET_GOLD(ch)<10)
          {
            send_to_char("You don't have enough money.\n\r",ch);
            return(TRUE);
          }
        else
          {
            tmp=read_object(27530,VIRTUAL);
            obj_to_char(tmp,ch);
            GET_GOLD(ch)-=10;
            send_to_char("That's 10 coins.  Enjoy.\n\r",ch);
            act("$n sets the $o alight and thrusts it into $N's hand.",1,mob,tmp,ch,TO_NOTVICT);
            act("$n sets the $o alight and thrusts it into your hand.",1,mob,tmp,ch,TO_VICT);
             return(TRUE);
      }
   }
}
   return(FALSE);
}

/* blood moon */

int bloodmoon_statue(int room,CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(cmd==MSG_ENTER && !check_clan_access(VERTIGO_ACCESS,ch)) {
    act("\n\rSensing an intruder the eyes of the dragon perched upon the fountain glow\n\r\
bright red.  A loud cracking accompanies the shattering of the marble\n\r\
sheath that encloses the gigantic beast.  Breathing fire and clawing\n\r\
wildly the fountain dragon scoops $n up and carries $m back to Midgaard.",0,ch,0,0,TO_ROOM);
    act("\n\rSensing an intruder the eyes of the dragon perched upon the fountain glow\n\r\
bright red.  A loud cracking accompanies the shattering of the marble\n\r\
sheath that encloses the gigantic beast.  Breathing fire and clawing\n\r\
wildly the fountain dragon scoops you up and carries you back to Midgaard.",0,ch,0,0,TO_CHAR);
    spell_word_of_recall(30, ch, ch, 0);

  }
  return FALSE;
}

int bloodmoon_main(int room,CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(cmd==CMD_ENTER) {
    one_argument(arg,buf);
    if(!*buf) return FALSE;
    if(strcmp(buf,"portal")) return FALSE;

    if(ch->specials.riding) {
      send_to_char("Dismount first.\n\r",ch);
      return TRUE;
    }

    act ("$n enters the swirling obsidian portal and melts into the watery surface.",TRUE,ch,0,0,TO_ROOM);
    send_to_char ("You enter the swirling obsidian portal and melt into the watery surface.\n\r\n\r",ch);
    char_from_room (ch);
    char_to_room (ch, real_room(3014));
    act ("$n emerges through a rift in the sky with scorching flames at $s heels.",TRUE,ch,0,0,TO_ROOM);
    send_to_char("You emerge through a rift in the sky with scorching flames at your heels.\n\r\n\r",ch);
    do_look(ch,"",CMD_LOOK);
    signal_room(CHAR_REAL_ROOM(ch),ch,MSG_ENTER,"");
    return TRUE;
  }

  if(cmd==CMD_CRAWL && CHAR_VIRTUAL_ROOM(ch)==27517) {
    act("$n crawls into the bathtub with a splash.", TRUE, ch, 0, 0, TO_ROOM);
    act ("You crawl into the bathtub with a splash.\n\r", TRUE, ch, 0, 0, TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, real_room(27520));
    do_look(ch,"",0);
    act("$n crawls into the bathtub with a splash.", TRUE, ch, 0, 0, TO_ROOM);
    return TRUE;
  }
  return FALSE;
}

int bloodmoon_orb(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  char buf[MAX_INPUT_LENGTH];

  if(IS_NPC(ch)) return FALSE;
  if(cmd==CMD_USE) {
    one_argument(argument,buf);
    if(!*buf) return FALSE;
    if(strcmp(buf,"orb")) return FALSE;
    if(!obj->equipped_by) return FALSE;
    if(obj->equipped_by!=ch) return FALSE;

    if(ch->specials.riding) {
      send_to_char("Dismount first.\n\r",ch);
      return TRUE;
    }

    if(CHAR_VIRTUAL_ROOM(ch)!=3014) {
      send_to_char("Your orb seems powerless here.\n\r",ch);
      return TRUE;
    }

    act("$n's gelatinous orb expands to envelop $m in a shimmering bubble\n\rthat shoots off into the sky.",TRUE,ch,0,0,TO_ROOM);
    act("Your gelatinous orb expands to envelop you in a shimmering bubble\n\rthat shoots off into the sky.\n\r",TRUE,ch,0,0,TO_CHAR);
    char_from_room (ch);
    char_to_room (ch, real_room(27516));

    act("$n floats down through the hole in the dome amidst a glistening bubble\n\r\
that reverts to a small orb in the palm of $s hand as $e touches the ground.",TRUE,ch,0,0,TO_ROOM);
    act("You float down through the hole in the dome amidst a glistening bubble\n\r\
that reverts to a small orb in the palm of your hand as you touch the ground.\n\r",TRUE,ch,0,0,TO_CHAR);
    do_look(ch,"",CMD_LOOK);
    signal_room(CHAR_REAL_ROOM(ch),ch,MSG_ENTER,"");
    return TRUE;
  }
  return FALSE;
}

int bloodmoon_tub(int room,CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(cmd==CMD_CRAWL && CHAR_VIRTUAL_ROOM(ch)==27520) {
    act("$n crawls out of the bathtub, slipping on the marble and onto the floor.", TRUE, ch, 0, 0, TO_ROOM);
    act ("You crawl out of the bathtub, slipping on the marble and onto the floor.\n\r", TRUE, ch, 0, 0, TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, real_room(27517));
    do_look(ch,"",0);
    act("$n crawls out of the bathtub, slipping on the marble and onto the floor.", TRUE, ch, 0, 0, TO_ROOM);
    return TRUE;
  }
  return FALSE;
}

/* Dark Legion */
int dl_holo(CHAR *holo, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd==MSG_ENTER && !check_clan_access(DARK_LEGION_ACCESS,ch)) {
    act("$N vanishes into a swirling dark cloud as Holo points at $M.",1,holo,0,ch,TO_NOTVICT);
    act("You vanish in a swirling dark cloud as Holo points his talon at you.",1,holo,0,ch,TO_VICT);
    act("You point at $N, who vanishes into a swirling dark cloud.",1,holo,0,ch,TO_CHAR);
    spell_word_of_recall(30,holo, ch, 0);
  }
  return FALSE;
}

int dl_enter(int room,CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(cmd!=CMD_ENTER) return FALSE;
  one_argument(arg,buf);
  if(!*buf) return FALSE;
  if(strcmp(buf,"mist")) return FALSE;

  if(ch->specials.riding) {
    send_to_char("Dismount first.\n\r",ch);
    return TRUE;
  }

  act ("$n disappears into the dark mist.",TRUE,ch,0,0,TO_ROOM);
  send_to_char ("The dark mist magically teleports you to the Ancient Castle Dark Legion.\n\r",ch);

  char_from_room (ch);
  char_to_room (ch, real_room(DARK_LEGION_ACCESS));
  act ("$n appears in a swirling dark mist",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  signal_room(CHAR_REAL_ROOM(ch),ch,MSG_ENTER,"");
  return TRUE;
}

int dl_leave(int room,CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(cmd!=CMD_ENTER) return FALSE;
  one_argument(arg,buf);
  if(!*buf) return FALSE;
  if(strcmp(buf,"cloud")) return FALSE;

  if(ch->specials.riding) {
    send_to_char("Dismount first.\n\r",ch);
    return TRUE;
  }

  act ("$n is swept up into the swirling cloud.",TRUE,ch,0,0,TO_ROOM);
  send_to_char ("You are swept up into the swirling cloud.\n\r",ch);
  char_from_room (ch);
  char_to_room (ch, real_room(3014));
  act ("$n appears in a swirling dark mist",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  signal_room(CHAR_REAL_ROOM(ch),ch,MSG_ENTER,"");
  return TRUE;
}

/* MM */
int mm_block(int room, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd==CMD_SOUTH && !check_clan_access(MM_ACCESS,ch)) {
    act("A magical force pushes you back.",1,ch,0,0,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

int mm_portal(int room,CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(cmd!=CMD_ENTER) return FALSE;
  one_argument(arg,buf);
  if(!*buf) return FALSE;
  if(strcmp(buf,"portal")) return FALSE;

  if(ch->specials.riding) {
    send_to_char("Dismount first.\n\r",ch);
    return TRUE;
  }

  act ("$n disappears into the large dark portal.",TRUE,ch,0,0,TO_ROOM);
  send_to_char ("You disappear into the large dark portal.\n\r",ch);

  char_from_room (ch);
  char_to_room (ch, real_room(3014));
  act ("$n appears through a portal in the center of the square.",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  signal_room(CHAR_REAL_ROOM(ch),ch,MSG_ENTER,"");
  return TRUE;
}


/* Eternal Newbies */
int eternal_entrance(int room,CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(cmd==MSG_ENTER && !check_clan_access(ETERNAL_ACCESS,ch)) {
    act("\n\rGrunthor the Vigilant roars loudly at you!\n\rHe grabs you by the foot and flings you back out of the door !!\n\r",0,ch,0,0,TO_CHAR);
    act("\n\rGrunthor the Vigilant roars loudly at $n!\n\rHe grabs $m by the foot and flings $m back out of the door !!\n\r",0,ch,0,0,TO_ROOM);
    spell_word_of_recall(30, ch, ch, 0);
  }
  return FALSE;
}


/* Tabitha's . */
int tabby_entrance(int room,CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(cmd==MSG_ENTER && !check_clan_access(TABBY_ACCESS,ch)) {
    act("\n\rThe Saber-Tooth Tiger roars ferociously, sending you screaming back to the Pet Shop!\n\r",0,ch,0,0,TO_CHAR);
    act("\n\rThe Saber-Tooth Tiger roars ferociously, sending $n screaming back to the Pet Shop!\n\r",0,ch,0,0,TO_ROOM);
    char_from_room (ch);
    char_to_room (ch, real_room(3031));
    act ("$n falls to the floor with an audible grunt.",TRUE,ch,0,0,TO_ROOM);
    do_look(ch,"",CMD_LOOK);
    signal_room(CHAR_REAL_ROOM(ch),ch,MSG_ENTER,"");
  }
  return FALSE;
}

/* High Elves */
int he_block(int room, CHAR *ch, int cmd, char *arg)
{
	if(!ch) return FALSE;
	if(cmd==CMD_UP && !check_clan_access(HE_ACCESS,ch))
	{
		act("The breath of The One prevents $n from entering Brethil Tham!", 1, ch, 0, 0, TO_ROOM);
		act("The breath of The One prevents you from entering Brethil Tham!", 1, ch, 0, 0, TO_CHAR);
		return TRUE;
	}
	return FALSE;
}

/*
** Unicorns
*/
static int unicorns_entrance(int room, CHAR *ch, int cmd, char *arg) {
  struct room_direction_data dir = {0};
  struct room_direction_data* o_dir = NULL;
  static bool open = FALSE;

  if (!ch || IS_NPC(ch) || !AWAKE(ch) || (cmd != CMD_DOWN) || open) return FALSE;
  if (!check_clan_access(UNICORNS_ACCESS, ch)) return FALSE;

  o_dir = world[room].dir_option[DOWN];
  dir.to_room_r = real_room(UNICORNS_ENTRANCE);

  world[room].dir_option[DOWN] = &dir;

  open = TRUE;

  do_move(ch, "", CMD_DOWN);

  world[room].dir_option[DOWN] = o_dir;

  open = FALSE;

  return TRUE;
}


/*
** Bud
*/
static int bud_entrance(int room, CHAR *ch, int cmd, char *arg) {
  struct room_direction_data dir = {0};
  struct room_direction_data* o_dir = NULL;
  static bool open = FALSE;

  if (!ch || IS_NPC(ch) || !AWAKE(ch) || (cmd != CMD_UP) || open) return FALSE;
  if (!check_clan_access(BUD_ACCESS, ch)) return FALSE;

  o_dir = world[room].dir_option[UP];
  dir.to_room_r = real_room(BUD_ENTRANCE);

  world[room].dir_option[UP] = &dir;

  open = TRUE;

  do_move(ch, "", CMD_UP);

  world[room].dir_option[UP] = o_dir;

  open = FALSE;

  return TRUE;
}


/* ISA */
/*
  spec.isa.c - Specs for ISAHall, by NightBoba

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 01/30/2003
*/

/* Defines */

#define ISA_CLAN_NUM        16


/* Rooms*/
#define TEMPLE              3001
#define MARKET_SQUARE       3014

#define PATH_OF_DIVINITY    27850
#define GINZUISHOU_ROAD     27853
#define ASSASSINS_PARADISE  27844
#define ICE_PALACE          27857
#define TUNNEL_OF_ROOTS     27872

#define FERRY               27899

#define STAR_WARS           27876


/* Mobiles */
#define PALADIN             27810
#define PRIEST              27811
#define DOJO_MASTER         27814
#define MAGE                27813

#define BAKER               27802
#define SCROLLMASTER        27800
#define FERRYMASTER         27821
#define CLAN_GUARD          27820
#define MAINLAND            27828
#define ISLAND              27827


/* Objects */
#define MIRROR              27840
#define ICEFRUIT            27830
#define SNOWBERRY           27831
#define GHOST_APPLE         27832

#define AZURE_POPPY         27833
#define DRAGONFLOWER        27834
#define RAZOR_THISTLE       27835

#define BAD_ICEFRUIT        27810
#define BAD_SNOWBERRY       27810
#define BAD_GHOST_APPLE     27810


/* Mobile Specs */

/* Paladin Spec */
int paladin_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ENTER) {
    if (!ch) return FALSE;
    if (IS_NPC(ch)) return FALSE;
    if (!chance(20)) return FALSE;

    if (IS_GOOD(ch)) {
      act("$n looks at $N and nods $s head approvingly.", FALSE, mob, 0, ch, TO_NOTVICT);
      act("$n looks at you and nods $s head approvingly.", FALSE, mob, 0, ch, TO_VICT);
      act("You look at $N and nod your head approvingly.", FALSE, mob, 0, ch, TO_CHAR);

      if (GET_AGE(ch) < GET_AGE(mob)) {
        do_say(mob, "Good luck on yer travels, young 'un.", CMD_SAY);
      }
      else {
        do_say(mob, "Good luck on yer travels.", CMD_SAY);
      }
    }
    else if (IS_EVIL(ch)) {
      act("$n looks at $N and shakes $s head disapprovingly.", FALSE, mob, 0, ch, TO_NOTVICT);
      act("$n looks at you and shakes $s head disapprovingly.", FALSE, mob, 0, ch, TO_VICT);
      act("You look at $N and shake your head disapprovingly.", FALSE, mob, 0, ch, TO_CHAR);
    }
    else {
      act("$n looks at $N and scratches $s head and shrugs.", FALSE, mob, 0, ch, TO_NOTVICT);
      act("$n looks at you and scratches $s head and shrugs.", FALSE, mob, 0, ch, TO_VICT);
      act("You look at $N and scratch your head and shrug.", FALSE, mob, 0, ch, TO_CHAR);
    }

    return FALSE;
  }

  return FALSE;
}

/* Priest Spec */
int priest_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ENTER) {
    if (!ch) return FALSE;
    if (IS_NPC(ch)) return FALSE;
    if (!chance(20)) return FALSE;

    do_say(mob, "Welcome to this sanctuary.  May you find the solace that you desire.", CMD_SAY);

    return FALSE;
  }

  return FALSE;
}

/* Dojo Master Spec */
int dojo_master_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    CHAR *opponent = GET_OPPONENT(mob);

    if (!opponent) return FALSE;

    if (GET_HIT(mob) < (GET_MAX_HIT(mob) / 10)) {
      stop_fighting(mob);
      stop_fighting(opponent);

      do_say(mob, "That is enough for this session.  Challenge me again any time.", CMD_SAY);

      GET_HIT(mob) = GET_MAX_HIT(mob);
    }
    else {
      switch (number(0, 9)) {
        case 0:
          do_say(mob, "You must try harder!  Focus...  You must become one with your weapon.", CMD_SAY);
          break;
        case 1:
          do_say(mob, "That's it!  You're learning!", CMD_SAY);
          break;
      }
    }

    return FALSE;
  }

  return FALSE;
}

/* Baker Spec */
int baker_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ENTER) {
    if (!ch) return FALSE;
    if (IS_NPC(ch)) return FALSE;

    char buf[MSL];

    snprintf(buf, sizeof(buf), "Good %s friend, how may I help you?", (IS_DAY ? "day" : "evening"));

    do_say(mob, buf, CMD_SAY);

    return FALSE;
  }

  return FALSE;
}

/* Scrollmaster Spec */
int scrollmaster_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ENTER) {
    if (!ch) return FALSE;
    if (IS_NPC(ch)) return FALSE;
    if (!chance(20)) return FALSE;

    act("An abrupt sneeze comes from behind a pile of scrolls off in one corner.", FALSE, mob, 0, 0, TO_ROOM);
    act("You sneeze violently!", FALSE, mob, 0, 0, TO_CHAR);

    return FALSE;
  }

  return FALSE;
}

/* Mage Spec */
int mage_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ENTER) {
    if (!ch) return FALSE;
    if (IS_NPC(ch)) return FALSE;
    if (!chance(10)) return FALSE;

    act("$n eyes $N suspiciously.", FALSE, mob, 0, ch, TO_NOTVICT);
    act("$n eyes you suspiciously.", FALSE, mob, 0, ch, TO_VICT);
    act("You eye $N suspiciously.", FALSE, mob, 0, ch, TO_CHAR);

    return FALSE;
  }

  return FALSE;
}

/* Clan Guard Spec */
int clan_guard_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_WEST) {
    if (!ch) return FALSE;
    if (IS_NPC(ch)) return TRUE;

    if (ch->ver3.clan_num != ISA_CLAN_NUM) {
      act("$n blocks $N's way and pushes $M back!", TRUE, mob, 0, ch, TO_NOTVICT);
      act("$n blocks your way and pushes you back!", FALSE, mob, 0, ch, TO_VICT);
      act("You bock $N's way and push $M back!", FALSE, mob, 0, ch, TO_CHAR);

      return TRUE;
    }
  }

  return FALSE;
}

/* Ferrymaster Spec */
int ferrymaster_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  static int timer = 0;
  static int dock = 0;

  if (cmd == MSG_TICK) {
    if (CHAR_REAL_ROOM(mob) != FERRY) return FALSE;

    if (timer > 0) {
      timer--;

      if (chance(5)) {
        act("$n scampers over to one of $s cannons and sets off the charge!", TRUE, mob, 0, 0, TO_ROOM);
        act("You scamper over to one of your cannons and set off the charge!", FALSE, mob, 0, 0, TO_CHAR);
        send_to_room("BOOM!!!\r\n", CHAR_REAL_ROOM(mob));
        do_say(mob, "DEATH FROM ABOVE!", CMD_SAY);

        if (number(0, 1)) {
          send_to_room("A volley of pink flowers and confetti is shot out to the mainland!", CHAR_REAL_ROOM(mob));
          send_to_room("You are pelted by pink flowers and confetti as they are shot out at you from\r\n"
            "the the ship's cannons!\r\n", real_room(MAINLAND));

          return FALSE;
        }
        else {
          send_to_room("A volley of pink flowers and confetti is shot out to the island!", CHAR_REAL_ROOM(mob));
          send_to_room("You are pelted by pink flowers and confetti as they are shot out at you from\r\n"
            "the the ship's cannons!\r\n", real_room(ISLAND));

          return FALSE;
        }

        return FALSE;
      }

      return FALSE;
    }
    else {
      if (world[CHAR_REAL_ROOM(mob)].dir_option[EAST]->to_room_r == real_room(MAINLAND)) {
        send_to_room("The ferry pulls away from the shore, heading for the island.", real_room(MAINLAND));
        send_to_room("The ferry pulls away from the shore, heading for the island.", CHAR_REAL_ROOM(mob));
        do_say(mob, "Arr me mateys! We be under way to the island!", CMD_SAY);
        act("$n giggles.", TRUE, mob, 0, 0, TO_ROOM);
        act("You giggle.", FALSE, mob, 0, 0, TO_CHAR);

        world[CHAR_REAL_ROOM(mob)].dir_option[EAST]->to_room_r = 0;

        timer = number(0, 1);
        dock = ISLAND;

        return FALSE;
      }
      else if (world[CHAR_REAL_ROOM(mob)].dir_option[WEST]->to_room_r == real_room(ISLAND)) {
        send_to_room("The ferry pulls away from the shore, heading for the mainland.", real_room(MAINLAND));
        send_to_room("The ferry pulls away from the shore, heading for the mainland.", CHAR_REAL_ROOM(mob));
        do_say(mob, "Avast ye land lubbers! We be ashore swiftly!", CMD_SAY);
        act("$n giggles.", TRUE, mob, 0, 0, TO_ROOM);
        act("You giggle.", FALSE, mob, 0, 0, TO_CHAR);

        world[CHAR_REAL_ROOM(mob)].dir_option[WEST]->to_room_r = 0;

        timer = number(0, 1);
        dock = MAINLAND;

        return FALSE;
      }
      else {
        if (dock == MAINLAND) {
          send_to_room("The ferry pulls up to the shore.", real_room(MAINLAND));
          send_to_room("The ferry pulls up to the shore.", CHAR_REAL_ROOM(mob));
          do_say(mob, "Land ho! Fortress is just a hop, skip and a jump to the east!", CMD_SAY);
          act("$n giggles.", TRUE, mob, 0, 0, TO_ROOM);
          act("You giggle.", FALSE, mob, 0, 0, TO_CHAR);

          world[CHAR_REAL_ROOM(mob)].dir_option[EAST]->to_room_r = real_room(MAINLAND);

          return FALSE;
        }
        else {
          send_to_room("The ferry pulls up to the shore.", real_room(ISLAND));
          send_to_room("The ferry pulls up to the shore.", CHAR_REAL_ROOM(mob));
          do_say(mob, "We're gonna crash!!! Hehehe... just kidding!  ISA Castle is just to the east!", CMD_SAY);
          act("$n giggles.", TRUE, mob, 0, 0, TO_ROOM);
          act("You giggle.", FALSE, mob, 0, 0, TO_CHAR);

          world[CHAR_REAL_ROOM(mob)].dir_option[WEST]->to_room_r = real_room(ISLAND);

          return FALSE;
        }

        return FALSE;
      }

      return FALSE;
    }

    return FALSE;
  }

  if (cmd == MSG_MOBACT) {
    if (CHAR_REAL_ROOM(mob) != FERRY) return FALSE;

    switch (number(0, 19)) {
      case 0:
        do_say(mob, "Ho hum... This pirate game got boring, but now its my job!", CMD_SAY);
        act("$n sighs.", TRUE, mob, 0, 0, TO_ROOM);
        act("You sigh.", FALSE, mob, 0, 0, TO_CHAR);
        break;
      case 1:
        do_say(mob, "I'm a very womanly pirate!", CMD_SAY);
        act("$n twitches $s mustache.", TRUE, mob, 0, 0, TO_ROOM);
        act("You twitch your mustache.", FALSE, mob, 0, 0, TO_CHAR);
        break;
      case 2:
        do_say(mob, "Now where did I put my grog?", CMD_SAY);
        act("$n burps.", TRUE, mob, 0, 0, TO_ROOM);
        act("You burps.", FALSE, mob, 0, 0, TO_CHAR);
        break;
      case 3:
        do_say(mob, "I finally found a good, long pole...", CMD_SAY);
        act("$n eyes the ship's mast.", TRUE, mob, 0, 0, TO_ROOM);
        act("You eye the ship's mast.", FALSE, mob, 0, 0, TO_CHAR);
        break;
      case 4:
        do_say(mob, "I'm sooo hungry!  Would you mind if I gnawed on you?", CMD_SAY);
        act("$n licks $s lips hungrily.", TRUE, mob, 0, 0, TO_ROOM);
        act("You lick your lips hungrily.", FALSE, mob, 0, 0, TO_CHAR);
        break;
    }

    return FALSE;
  }

  return FALSE;
}


/* Object Specs */

/* Mirror Spec */
int mirror_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  char buf[MIL];

  if (cmd == CMD_ENTER) {
    if (!ch) return FALSE;
    if (IS_NPC(ch)) return FALSE;
    if (!AWAKE(ch)) return FALSE;

    one_argument(arg, buf);

    if (!*buf) return FALSE;

    if (!isname(buf, OBJ_NAME(obj))) return FALSE;

    if (ch->specials.riding) {
      send_to_char("Dismount first.\r\n", ch);

      return TRUE;
    }

    act("$n enters the mirror and disappears!", TRUE, ch, 0, 0, TO_ROOM);
    act("You enter the mirror and disappear!", FALSE, ch, 0, 0, TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, real_room(STAR_WARS));
    act("$n appears in the room suddenly!", TRUE, ch, 0, 0, TO_ROOM);
    do_look(ch, "\0", CMD_LOOK);

    return TRUE;
  }

  return FALSE;
}

/* Fruit Spec */
int fruit_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_TICK) {
    if (((OBJ_SPEC(obj) > 36) || (OBJ_SPEC(obj) < 36)) && (OBJ_SPEC(obj) != 0)) {
      OBJ_SPEC(obj)--;
    }
    else if (OBJ_SPEC(obj) == 36) {
      act("$p rots and turns bad!", FALSE, 0, obj, 0, TO_ROOM);

      switch (V_OBJ(obj)) {
        case(ICEFRUIT):
          obj = read_object(BAD_ICEFRUIT, VIRTUAL);
          break;
        case(SNOWBERRY):
          obj = read_object(BAD_SNOWBERRY, VIRTUAL);
          break;
        case(GHOST_APPLE):
          obj = read_object(BAD_GHOST_APPLE, VIRTUAL);
          break;
      }
    }
    else {
      act("$p decays into a brown mush and then disintegrates.", FALSE, 0, obj, 0, TO_ROOM);

      extract_obj(obj);
    }

    return FALSE;
  }

  return FALSE;
}

/* Flower Spec */
int flower_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_TICK) {
    if (obj->carried_by) {
      act("$p wilts and dies, having been picked.", FALSE, obj->carried_by, obj, 0, TO_CHAR);

      extract_obj(obj);

      return FALSE;
    }

    if (OBJ_SPEC(obj) > 0) {
      OBJ_SPEC(obj)--;

      return FALSE;
    }

    switch (V_OBJ(obj)) {
      case(AZURE_POPPY):
        act("$p melts away as its lifecycle runs out.", FALSE, 0, obj, 0, TO_ROOM);
        extract_obj(obj);
        break;
      case(DRAGONFLOWER):
        act("$p slumps to the ground as its lifeless stem can no longer hold it aloft.", FALSE, 0, obj, 0, TO_ROOM);
        extract_obj(obj);
        break;
      case(RAZOR_THISTLE):
        act("$p loses all of its spines and turns brown as it dies away.", FALSE, 0, obj, 0, TO_ROOM);
        extract_obj(obj);
        break;
    }
  }

  return FALSE;
}


/* Room Specs */

/* Path of Divinity Spec */
int path_of_divinity_spec(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_TICK) {
    if (!chance(10)) return FALSE;

    send_to_room("\
A loud crack sounds in the distance as a limb that can no longer bear the\r\n\
weight of snow and ice succumbs to gravity and crashes to the ground.\r\n", room);
  }

  return FALSE;
}

/* Ginzuishou Road Spec */
int ginzuishou_road_spec(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ENTER) {
    if (!ch) return FALSE;
    if (IS_NPC(ch)) return FALSE;
    if (!chance(25)) return FALSE;

    send_to_char("You feel hidden eyes upon you, watching your every move.\r\n", ch);
  }

  return FALSE;
}

/* Ice Palace Spec */
int ice_palace_spec(int room, CHAR *ch, int cmd, char *arg) {
  char buf[MSL];

  if (cmd == MSG_TICK) {
    if (!chance(25)) return FALSE;
    if (count_mortals_real_room(room) < 1) return FALSE;

    snprintf(buf, sizeof(buf), "An audience member shakes %s head in disgust and gets up to leave the arena.\n\r", (chance(50) ? "his" : "her"));

    send_to_room(buf, room);

    return FALSE;
  }

  return FALSE;
}

/* Tunnel of Roots Spec */
int tunnel_of_roots_spec(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_TICK) {
    if (!chance(50)) return FALSE;

    send_to_room("A tiny worm tries to burrow a nest into your skin... Ick!\r\n", room);

    return FALSE;
  }

  return FALSE;
}

/* Assassin's Paradise Spec */
int assassins_paradise_spec(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ENTER) {
    if (!ch) return FALSE;
    if (IS_NPC(ch)) return FALSE;
    if (!chance(5)) return FALSE;

    if (EQ(ch, WEAR_ABOUT)) return FALSE;
    if (!isname(OBJ_NAME(EQ(ch, WEAR_ABOUT)), "cloak")) return FALSE;

    send_to_char("\
A small gnome fades in from the shadows.\r\n\
Qualimere says, 'May I take your cloak master?'\r\n\
Without waiting for a reply, he snatches your cloak and scampers away into the shadows!\r\n\
A moment later you hear a loud crash and a series of muffled curses.\r\n\
Your cloak floats out of the darkness and lands at your feet.\r\n", ch);

    OBJ *tmp_obj = unequip_char(ch, WEAR_ABOUT);
    obj_from_char(tmp_obj);
    obj_to_room(tmp_obj, room);

    return FALSE;
  }

  return FALSE;
}

/* Fruit Spec */
int fruit_room_spec(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ZONE_RESET) {
    int fruit_type = 0;
    int fruit_num = 0;

    if ((time_info.month < 3) || (time_info.month > 10)) {
      fruit_type = ICEFRUIT;
      fruit_num = number(0, 3);
    }
    else if ((time_info.month > 2) && (time_info.month < 7)) {
      fruit_type = SNOWBERRY;
      fruit_num = number(2, 8);
    }
    else {
      fruit_type = GHOST_APPLE;
      fruit_num = number(1, 5);
    }

    for (int i = 0; i < fruit_num; i++) {
      OBJ *fruit = read_object(fruit_type, VIRTUAL);

      OBJ_SPEC(fruit) = 72;

      obj_to_room(fruit, room);

      act("$p reaches ripeness and falls to the ground.", FALSE, 0, fruit, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}

/* Flower Spec */
int flower_room_spec(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ZONE_RESET) {
    int flower_type = 0;
    int flower_num = 0;

    if ((time_info.month < 3) || (time_info.month > 10)) {
      flower_type = AZURE_POPPY;
      flower_num = number(0, 2);
    }
    else if ((time_info.month > 2) && (time_info.month < 7)) {
      flower_type = DRAGONFLOWER;
      flower_num = number(3, 8);
    }
    else {
      flower_type = RAZOR_THISTLE;
      flower_num = number(1, 3);
    }

    for (int i = 0; i < flower_num; i++) {
      OBJ *flower = read_object(flower_type, VIRTUAL);

      OBJ_SPEC(flower) = 288;

      obj_to_room(flower, room);

      act("$p sprouts from the ground in a brilliant display of life.", FALSE, 0, flower, 0, TO_ROOM);
    }

    return FALSE;
  }

  if (cmd == MSG_TICK) {
    static int flower_blooming = FALSE;

    if (IS_DAY && !flower_blooming) {
      for (OBJ *tmp_obj = world[room].contents, *next_content = NULL; tmp_obj; tmp_obj = next_content) {
        next_content = tmp_obj->next_content;

        if ((V_OBJ(tmp_obj) == AZURE_POPPY) || (V_OBJ(tmp_obj) == DRAGONFLOWER)) {
          act("$p reveals its petals and soaks in the light.", FALSE, 0, tmp_obj, 0, TO_ROOM);
        }
      }

      flower_blooming = TRUE;
    }
    else if (IS_NIGHT && flower_blooming) {
      for (OBJ *tmp_obj = world[room].contents, *next_content = NULL; tmp_obj; tmp_obj = next_content) {
        next_content = tmp_obj->next_content;

        if ((V_OBJ(tmp_obj) == AZURE_POPPY) || (V_OBJ(tmp_obj) == DRAGONFLOWER)) {
          act("$p wilts and closes its petals as the light fades.", FALSE, 0, tmp_obj, 0, TO_ROOM);
        }
      }

      flower_blooming = FALSE;
    }
    else {
      flower_blooming = FALSE;
    }

    return FALSE;
  }

  return FALSE;
}

#define ISA_TALISMAN 27800
#define TEMPLE       3001
#define MARKET       3014
#define ISA_HALL     27801

int talisman_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *owner = NULL;

  if (cmd != CMD_USE) return FALSE;

  if (!(owner = obj->carried_by)) return FALSE;

  if (owner->ver3.clan_num != ISA_CLAN_NUM) {
    act("$p pulses with energy but fizzles and nothing happens!", TRUE, owner, obj, 0, TO_ROOM);
    act("$p pulses with energy but fizzles and nothing happens!", FALSE, owner, obj, 0, TO_CHAR);
    send_to_char("(You are not authorized to use this clan item.)\n\r", owner);

    return TRUE;
  }

  if (owner->specials.fighting) {
    act("$p pulses with energy but fizzles and nothing happens!", TRUE, owner, obj, 0, TO_ROOM);
    act("$p pulses with energy but fizzles and nothing happens!", FALSE, owner, obj, 0, TO_CHAR);
    send_to_char("(Clan recall items may only be used outside of combat.)\n\r", owner);

    return TRUE;
  }

  if ((V_ROOM(owner) != TEMPLE) && (V_ROOM(owner) != MARKET)) {
    act("$p pulses with energy but fizzles and nothing happens!", TRUE, owner, obj, 0, TO_ROOM);
    act("$p pulses with energy but fizzles and nothing happens!", FALSE, owner, obj, 0, TO_CHAR);
    send_to_char("(Clan recall items may only be used at Temple of Midgaard and Market Square.)\n\r", owner);

    return TRUE;
  }

  act("$n opens a small portal with $s $p, steps through, and dissapears!", TRUE, owner, obj, 0, TO_ROOM);
  act("You open a small portal with your $p, step through, and dissapear!", FALSE, owner, obj, 0, TO_CHAR);

  char_from_room(owner);
  char_to_room(owner, real_room(ISA_HALL));
  do_look(owner, "\0", CMD_LOOK);

  act("A small portal appears in the middle of the room, and out of it steps $n!", FALSE, owner, obj, 0, TO_ROOM);

  return TRUE;
}


#ifdef TEST_SITE
int hellions_entrance(int room,CHAR *ch, int cmd, char *arg)
{
   if (cmd == CMD_ENTER && ch && !IS_NPC(ch))
   {
      if (check_clan_access(HELLIONS_ACCESS,ch))
         return FALSE;
      else
      {
         act("$N takes a step east, but is stopped in $p tracks by a mysterious force.", TRUE, ch, NULL, 0, TO_ROOM);
         send_to_char("As you step east, a strange magical force holds you back!", ch);
         return TRUE;
      }
   }
   return FALSE;
}
#endif

void assign_clan(void) {
  assign_obj(SHERIFF_BADGE,sheriff_badge);
  assign_mob(DIABOLIK_GARGOYLE, diabolik_gargoyle);
  assign_mob(DIABOLIK_BARKEEP, gruumsh_barkeep);
  assign_mob(27502,do_vault); /* Diabolik vault */
  assign_room(DIABOLIK_LIBRARY, diabolik_library);
  assign_room(DIABOLIK_HALL, diabolik_ring);
  assign_room(DIABOLIK_ACCESS, diabolik_ring);
  assign_mob(27505, receptionist); /* Diaboilik receptionist */

  assign_mob(27513,do_vault); /* solo vault */
  assign_mob(27512,receptionist); /* solo vault */
  assign_mob(27510,portal_guard);
  assign_mob(27514,lilith_shopkeep);
  assign_obj(27512,clan_portal);
  assign_obj(27510,clan_pentagram); /* solo entrance */
  assign_obj(27530,clan_recall_scroll);

  /* Bud */
  assign_room(BUD_ACCESS, bud_entrance);
  assign_mob(27517, receptionist);
  assign_mob(27518, do_vault);
  //assign_obj(27526, bloodmoon_orb);
  //assign_room(27520, bloodmoon_tub);
  //assign_room(27517, bloodmoon_main);
  //assign_room(27516, bloodmoon_statue);

  assign_room(MM_ACCESS, mm_block);
  assign_room(27536, mm_portal);

  assign_room(ETERNAL_ACCESS, eternal_entrance); /* eternal newbies */
  assign_mob(27525, do_vault);

  assign_room(TABBY_ACCESS, tabby_entrance); /* tabitha's . clan */
  assign_mob(27528, do_vault);
  assign_mob(27529, receptionist);
  assign_obj(27589, clan_recall_scroll);

  assign_mob(27520, dl_holo); /* dark legion */
  assign_room(6016, dl_enter);
  assign_room(27526, dl_leave);
  assign_mob(27521,receptionist);
  assign_mob(27523,do_vault);
  assign_mob(27540,receptionist);
  assign_mob(27531,meta);
  /* ISA Mobiles */
  assign_mob(27822, receptionist);
  assign_mob(27824, do_vault);
  assign_mob(PALADIN, paladin_spec);
  assign_mob(PRIEST, priest_spec);
  assign_mob(DOJO_MASTER, dojo_master_spec);
  assign_mob(MAGE, mage_spec);
  assign_mob(BAKER, baker_spec);
  assign_mob(SCROLLMASTER, scrollmaster_spec);
  assign_mob(FERRYMASTER, ferrymaster_spec);
  assign_mob(CLAN_GUARD, clan_guard_spec);
  /* ISA Objects */
  assign_obj(MIRROR, mirror_spec);
  assign_obj(ICEFRUIT, fruit_spec);
  assign_obj(SNOWBERRY, fruit_spec);
  assign_obj(GHOST_APPLE, fruit_spec);
  assign_obj(AZURE_POPPY, flower_spec);
  assign_obj(DRAGONFLOWER, flower_spec);
  assign_obj(RAZOR_THISTLE, flower_spec);
  assign_obj(ISA_TALISMAN, talisman_spec);
  /* ISA Rooms */
  assign_room(PATH_OF_DIVINITY, path_of_divinity_spec);
  assign_room(GINZUISHOU_ROAD, ginzuishou_road_spec);
  assign_room(ICE_PALACE, ice_palace_spec);
  assign_room(ASSASSINS_PARADISE, assassins_paradise_spec);
  assign_room(TUNNEL_OF_ROOTS, tunnel_of_roots_spec);

  /* High Elves */
  assign_room(HE_ACCESS, he_block);
  assign_mob(27576, do_vault);
  assign_mob(27577, meta);

  /* Unicorns */
  assign_room(UNICORNS_ACCESS, unicorns_entrance);
  assign_mob(27554, do_vault);
  assign_mob(27553, receptionist);
  assign_mob(27556, meta);
}
