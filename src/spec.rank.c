/* ************************************************************************
*  file: spec.rank.c ,                                                    *
*  Specs for ranking boards                                               *
*  Ranger of RoninMUD, do not distribute without permission.              *
*  April 99 - Modified March 23/01 - upped to 15 to keep track of         *
**************************************************************************/

/*
$Author: ronin $
$Date: 2005/04/27 17:13:30 $
$Header: /home/ronin/cvs/ronin/spec.rank.c,v 2.1 2005/04/27 17:13:30 ronin Exp $
$Id: spec.rank.c,v 2.1 2005/04/27 17:13:30 ronin Exp $
$Name:  $
$Log: spec.rank.c,v $
Revision 2.1  2005/04/27 17:13:30  ronin
Minor changes needed to compile on Slackware 10 for the new machine.

Revision 2.0.0.1  2004/02/05 16:11:14  ronin
Reinitialization of cvs archives

Revision 1.3  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "cmd.h"
#include "utility.h"
#include "enchant.h"
#include "spec.clan.h"
#include "spec_assign.h"

extern struct clan_data clan_list[MAX_CLANS];

struct rank_struct
{
  char name[20];
  int ranking;
  byte level;
  byte sc_level;
  sh_int hp;
  sh_int mana;
  long update_time;
  int clan;
};

struct rank_struct rank_board[11][15];
extern char *pc_class_types[];

void reset_rank_boards (void) {
  int i,j;
  char buf[MAX_INPUT_LENGTH];
  struct rank_struct rankfile;
  FILE *fl;

  for(i=0;i<11;i++) {
    sprintf(buf,"rank_file%d.dat",i);
    if(!(fl=fopen(buf,"wb"))) {
       log_f("Error opening rank file.");
       return;
    }
    for(j=0;j<15;j++) {
      strcpy(rankfile.name, "None");
      rankfile.ranking       = 0;
      rankfile.level         = 0;
      rankfile.sc_level      = 0;
      rankfile.hp            = 0;
      rankfile.mana          = 0;
      rankfile.update_time   = 0;
      rankfile.clan          = 0;
      fwrite(&rankfile,sizeof(rankfile),1,fl);
    }
    fclose(fl);
  }
}

void write_rank_board(int i) {
  int j;
  char buf[MAX_INPUT_LENGTH];
  FILE *fl;

  sprintf(buf,"rank_file%d.dat",i);
  if(!(fl=fopen(buf,"wb"))) {
     log_f("Error opening rank file.");
     return;
  }
  for(j=0;j<15;j++) {
    fwrite(&rank_board[i][j],sizeof(rank_board[i][j]),1,fl);
  }
  fclose(fl);
}

void resort_board(int i) {
  int j,issorted;
  char temptxt[FIELD_SIZE(rank_struct, name)];
  size_t len = FIELD_SIZE(rank_struct, name) - 1;

  memset(temptxt, 0, sizeof(temptxt));

  do {
    issorted = 1;
    for(j=0;j<14;j++)
      if(rank_board[i][j].ranking<rank_board[i][j+1].ranking) {
        strncpy(temptxt, rank_board[i][j].name, len);
        strncpy(rank_board[i][j].name, rank_board[i][j+1].name, len);
        strncpy(rank_board[i][j+1].name, temptxt, len);
        SWITCH(rank_board[i][j].ranking,rank_board[i][j+1].ranking);
        SWITCH(rank_board[i][j].level,rank_board[i][j+1].level);
        SWITCH(rank_board[i][j].sc_level,rank_board[i][j+1].sc_level);
        SWITCH(rank_board[i][j].hp,rank_board[i][j+1].hp);
        SWITCH(rank_board[i][j].mana,rank_board[i][j+1].mana);
        SWITCH(rank_board[i][j].update_time,rank_board[i][j+1].update_time);
        SWITCH(rank_board[i][j].clan,rank_board[i][j+1].clan);
        issorted = 0;
      }
  }
  while (!issorted);
}

void read_rank_boards (void) {
  int i,j,resort;
  char buf[MAX_INPUT_LENGTH];
  FILE *fl;

  for(i=0;i<11;i++) {
    sprintf(buf,"rank_file%d.dat",i);
    if(!(fl=fopen(buf,"rb"))) {
       log_f("Error opening rank file.");
       return;
    }
    resort=0;
    for(j=0;j<15;j++) {
      if((fread(&rank_board[i][j],sizeof(rank_board[i][j]),1,fl))!=1) {
        log_f("Error reading rankfile.");
        fclose(fl);
        return;
      }

      if(rank_board[i][j].update_time &&
        (rank_board[i][j].update_time+30*SECS_PER_REAL_DAY<time(0))) {
        strcpy(rank_board[i][j].name, "None");
        rank_board[i][j].ranking       = 0;
        rank_board[i][j].level         = 0;
        rank_board[i][j].sc_level      = 0;
        rank_board[i][j].hp            = 0;
        rank_board[i][j].mana          = 0;
        rank_board[i][j].update_time   = 0;
        rank_board[i][j].clan          = 0;
        resort=1;
      }
    }
    if(resort) resort_board(i);
    fclose(fl);
  }
}

int check_god_access(CHAR *ch, int active);
void do_rankfile(CHAR *ch, char *argument, int cmd) {
  int i,j,days;
  char buf[MAX_INPUT_LENGTH],clan[MAX_INPUT_LENGTH];
  char usage[]="\
Usage: rankfile <reset> (set all to zero)\n\r\
                <read>  (force a reread)\n\r\
                <write> (force a write)\n\r\
                <dump>  (dump file to screen)\n\r\n\r\
These commands should not be necessary during normal operation.\n\r.";

  if(!check_god_access(ch,1)) return;
  argument=one_argument(argument, buf);
  if(!*buf) {
    send_to_char(usage,ch);
    return;
  }
  if(!strcmp(buf,"reset")) {
    reset_rank_boards();
    send_to_char("All rank boards reset.\n\r'rankfile read' to bring in change.\n\r'rankfile write' to ignore reset.",ch);
    return;
  }
  if(!strcmp(buf,"read")) {
    read_rank_boards();
    send_to_char("All rank boards reread.\n\r",ch);
    return;
  }
  if(!strcmp(buf,"write")) {
    for(i=0;i<11;i++)
      write_rank_board(i);
    send_to_char("All rank boards written.\n\r",ch);
    return;
  }
  if(!strcmp(buf,"dump")) {
    for(i=0;i<11;i++) {
      printf_to_char(ch,"\n\r\
Herein lies a list of the best of the %s class seen among us\n\r\
in most recent times.  Study them and strive to be as good as they.\n\r\n\r",pc_class_types[i+1]);
      send_to_char("\
Name               |  Pts | Lvl | Sc |  Hps | Mana | Clan         | Last Seen\n\r\
==============================================================================\n\r",ch);

      for(j=0;j<10;j++) {
        if(rank_board[i][j].clan)
          sprintf(clan, "%s",clan_list[rank_board[i][j].clan].name);
        else
          sprintf(clan, "None");

        days=(time(0)-rank_board[i][j].update_time)/SECS_PER_REAL_DAY;
        if(rank_board[i][j].ranking)
          printf_to_char(ch,"%-19.19s| %4d |  %2d |  %1d | %4d | %4d | %-12.12s | %dd ago\n\r",
                  rank_board[i][j].name,
                  rank_board[i][j].ranking,
                  rank_board[i][j].level,
                  rank_board[i][j].sc_level,
                  rank_board[i][j].hp,
                  rank_board[i][j].mana+100,
                  clan,days);
        else
          send_to_char("None\n\r",ch);
      }
      send_to_char("\
==============================================================================\n\r",ch);
    }
    return;
  }
  send_to_char(usage,ch);
}

void update_rank_board(CHAR *ch) {
  int i,j,found=0,resort=0;

  i=GET_CLASS(ch)-1;

  for(j=0;j<15;j++) {
    if(!strcmp(rank_board[i][j].name,GET_NAME(ch))) {
      rank_board[i][j].ranking      =  ch->ver3.ranking;
      rank_board[i][j].level        =  GET_LEVEL(ch);
      rank_board[i][j].sc_level     =  ch->ver3.subclass_level;
      rank_board[i][j].clan         =  ch->ver3.clan_num;
      rank_board[i][j].hp           =  ch->specials.org_hit;
      rank_board[i][j].mana         =  ch->specials.org_mana;
      rank_board[i][j].update_time  =  time(0);
      found=1;
      resort=1;
    }
  }

  if(!found && ch->ver3.ranking>rank_board[i][14].ranking) {
    strcpy(rank_board[i][14].name,GET_NAME(ch));
    rank_board[i][14].ranking      =  ch->ver3.ranking;
    rank_board[i][14].level        =  GET_LEVEL(ch);
    rank_board[i][14].sc_level     =  ch->ver3.subclass_level;
    rank_board[i][14].clan         =  ch->ver3.clan_num;
    rank_board[i][14].hp           =  ch->specials.org_hit;
    rank_board[i][14].mana         =  ch->specials.org_mana;
    rank_board[i][14].update_time  =  time(0);
    resort=1;
  }

  if(resort) {
    resort_board(i);
    write_rank_board(i);
  }
}

/*
Routine to rank a character - Ranger March 99

Roughly from 1 to 10000
*/
int move_gain(struct char_data *ch);
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6);

void rank_char(CHAR *ch) {
  int i;
  OBJ *eq[MAX_WEAR];

  /* Unequip character before rank */
  for (i=0;i<MAX_WEAR;i++) {
    if (ch->equipment[i])
      eq[i]=unequip_char(ch, i);
    else
      eq[i] = 0;
  }

  ch->ver3.ranking=0;

/* General */
  ch->ver3.ranking+=100*ch->ver3.subclass_level;
  ch->ver3.ranking+=50*GET_LEVEL(ch);
  ch->ver3.ranking+=15*GET_OSTR(ch);
  ch->ver3.ranking+=2*GET_OADD(ch);

  ch->ver3.ranking+=20*graf(age(ch).year, 15,18,22,26,22,20,18);

  if (enchanted_by_type(ch, ENCHANT_SQUIRE)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_SWASHBUCKLER)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_KNIGHT)) ch->ver3.ranking+=150;
  if (enchanted_by_type(ch, ENCHANT_FIRSTSWORD)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_JUSTICIAR)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_LORDLADY)) ch->ver3.ranking+=150;
  if (enchanted_by_type(ch, ENCHANT_APPRENTICE)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_WARLOCK)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_SORCERER)) ch->ver3.ranking+=150;
  if (enchanted_by_type(ch, ENCHANT_PRIVATE)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_COMMODORE)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_COMMANDER)) ch->ver3.ranking+=150;
  if (enchanted_by_type(ch, ENCHANT_WANDERER)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_FORESTER)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_TAMER)) ch->ver3.ranking+=150;
  if (enchanted_by_type(ch, ENCHANT_TSUME)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_SHINOBI)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_SHOGUN)) ch->ver3.ranking+=150;
  if (enchanted_by_type(ch, ENCHANT_MINSTREL)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_POET)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_CONDUCTOR)) ch->ver3.ranking+=150;
  if (enchanted_by_type(ch, ENCHANT_MINION)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_DARKWARDER)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_DARKLORDLADY)) ch->ver3.ranking+=150;
  if (enchanted_by_type(ch, ENCHANT_ACOLYTE)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_BISHOP)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_PROPHET)) ch->ver3.ranking+=150;
  if (enchanted_by_type(ch, ENCHANT_HIGHWAYMAN)) ch->ver3.ranking+=50;
  if (enchanted_by_type(ch, ENCHANT_BRIGAND)) ch->ver3.ranking+=100;
  if (enchanted_by_type(ch, ENCHANT_ASSASSIN)) ch->ver3.ranking+=150;

/* Class Based */
  switch(GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
      ch->ver3.ranking+=8*ch->specials.org_hit/10;
      ch->ver3.ranking+=12*ch->specials.org_mana/10;
      ch->ver3.ranking+=5*GET_ODEX(ch);
      ch->ver3.ranking+=15*GET_OINT(ch);
      ch->ver3.ranking+=15*GET_OWIS(ch);
      ch->ver3.ranking+=5*GET_OCON(ch);
      break;
    case CLASS_NINJA:
    case CLASS_PALADIN:
    case CLASS_ANTI_PALADIN:
    case CLASS_AVATAR:
    case CLASS_BARD:
    case CLASS_COMMANDO:
      ch->ver3.ranking+=ch->specials.org_hit;
      ch->ver3.ranking+=ch->specials.org_mana;
      ch->ver3.ranking+=10*GET_ODEX(ch);
      ch->ver3.ranking+=10*GET_OINT(ch);
      ch->ver3.ranking+=10*GET_OWIS(ch);
      ch->ver3.ranking+=10*GET_OCON(ch);
      break;
    case CLASS_THIEF:
    case CLASS_WARRIOR:
    case CLASS_NOMAD:
      ch->ver3.ranking+=2*ch->specials.org_hit;
      ch->ver3.ranking+=15*GET_ODEX(ch);
      ch->ver3.ranking+=5*GET_OINT(ch);
      ch->ver3.ranking+=5*GET_OWIS(ch);
      ch->ver3.ranking+=15*GET_OCON(ch);
      break;
  }
  /* Board updates */
  update_rank_board(ch);

  /*Re-equip character */
  for (i=0;i<WIELD;i++)
    if (eq[i]) equip_char (ch, eq[i], i);
  if (eq[HOLD]) equip_char(ch, eq[HOLD], HOLD);
  if (eq[WIELD]) equip_char(ch, eq[WIELD], WIELD);

}

int rank_object(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  char tmp[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH],clan[MAX_INPUT_LENGTH];
  int i,j,days;

  if(!ch) return FALSE;
  if(cmd!=CMD_LOOK) return FALSE;
  one_argument(arg, tmp);
  if (!*tmp || !isname(tmp, obj->name)) return(FALSE);

  i=obj->obj_flags.value[1];
  sprintf(buf,"$n studies %s.",OBJ_SHORT(obj));
  act(buf,TRUE,ch,0,0,TO_ROOM);

      printf_to_char(ch,"\n\r\
Herein lies a list of the best of the %s class seen among us\n\r\
in most recent times.  Study them and strive to be as good as they.\n\r\n\r",pc_class_types[i+1]);
  send_to_char("\
Name               |  Pts | Lvl | Sc |  Hps | Mana | Clan         | Last Seen\n\r\
==============================================================================\n\r",ch);

  for(j=0;j<10;j++) {
    if(rank_board[i][j].clan)
      sprintf(clan, "%s",clan_list[rank_board[i][j].clan].name);
    else
      sprintf(clan, "None");

    days=(time(0)-rank_board[i][j].update_time)/SECS_PER_REAL_DAY;
    if(rank_board[i][j].ranking)
      printf_to_char(ch,"%-19.19s| %4d |  %2d |  %1d | %4d | %4d | %-12.12s | %dd ago\n\r",
                rank_board[i][j].name,
                rank_board[i][j].ranking,
                rank_board[i][j].level,
                rank_board[i][j].sc_level,
                rank_board[i][j].hp,
                rank_board[i][j].mana+100,
                clan,days);
    else
      send_to_char("None\n\r",ch);
  }
  send_to_char("\
==============================================================================\n\r",ch);
  if(GET_CLASS(ch)==i+1)
    printf_to_char(ch,"Your ranking .... %d.\n\r",ch->ver3.ranking);
  return TRUE;
}

void assign_rank_objects(void) {
  int i;
  for(i=3180;i<3191;i++)
    assign_obj(i,rank_object);
}
