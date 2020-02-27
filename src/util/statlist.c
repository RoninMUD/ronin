/*
** Program statlist.c
**
** Written for RoninMUD by Ranger
** Not to be distributed without author consent.
**
** This program takes a character file and appends
** the stats to a binary file for further processing
** with the charstats program.
*/

/*
$Author: ronin $
$Date: 2005/01/21 14:55:29 $
$Header: /home/ronin/cvs/ronin/statlist.c,v 2.1 2005/01/21 14:55:29 ronin Exp $
$Id: statlist.c,v 2.1 2005/01/21 14:55:29 ronin Exp $
$Name:  $
$Log: statlist.c,v $
Revision 2.1  2005/01/21 14:55:29  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.0.0.1  2004/02/05 16:11:46  ronin
Reinitialization of cvs archives


Revision 1.4 2002/11/11 ranger
Addition of subclass and subclass level to statlist output.

Revision 1.3  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define BUFFERSIZE 65535

struct stat_file_u {
  char name[20];
  byte class;
  byte level;
  byte sex;
  time_t birth;
  int played;
  time_t last_logon;
  int been_killed;
  int alignment;
  int total_cost;
  sh_int max_mana;
  sh_int max_hit;
  int gold;
  int subclass;
  int subclass_level;
};

int char_version(FILE *fl) {
  struct char_file_version char_version;

  if((fread(&char_version,sizeof(struct char_file_version),1,fl))!=1) {
    printf("Error Checking Char Version");
    rewind(fl);
    return (0);
  }
  rewind(fl);
  if(char_version.version<3) return(2);
  else return(char_version.version);
}

char *string_to_lower(char *string) {
  int i;
  for (i=0;i<strlen(string);i++) {
    string[i] = LOWER(string[i]);
  }
  return (string);
}

/* copy data from the file structure to the stat struct */
void read_char_store_5 (struct char_file_u_5 *st, struct stat_file_u *stat) {
  strcpy(stat->name,st->name);
  stat->class=st->class;
  stat->level=st->level;
  stat->sex=st->sex;
  stat->birth=st->birth;
  stat->played=st->played;
  stat->last_logon=st->last_logon;
  stat->been_killed=st->new.been_killed;
  stat->alignment=st->alignment;
  stat->total_cost=st->total_cost;
  stat->max_mana=st->points.max_mana;
  stat->max_hit=st->points.max_hit;
  stat->gold=st->points.gold+st->points.bank;
  stat->subclass=st->ver3.subclass;
  stat->subclass_level=st->ver3.subclass_level;
}

void read_char_store_4 (struct char_file_u_4 *st, struct stat_file_u *stat) {
  strcpy(stat->name,st->name);
  stat->class=st->class;
  stat->level=st->level;
  stat->sex=st->sex;
  stat->birth=st->birth;
  stat->played=st->played;
  stat->last_logon=st->last_logon;
  stat->been_killed=st->new.been_killed;
  stat->alignment=st->alignment;
  stat->total_cost=st->total_cost;
  stat->max_mana=st->points.max_mana;
  stat->max_hit=st->points.max_hit;
  stat->gold=st->points.gold+st->points.bank;
  stat->subclass=st->ver3.subclass;
  stat->subclass_level=st->ver3.subclass_level;
}

void read_char_store_2 (struct char_file_u_2 *st, struct stat_file_u *stat) {
  strcpy(stat->name,st->name);
  stat->class=st->class;
  stat->level=st->level;
  stat->sex=st->sex;
  stat->birth=st->birth;
  stat->played=st->played;
  stat->last_logon=st->last_logon;
  stat->been_killed=st->new.been_killed;
  stat->alignment=st->alignment;
  stat->total_cost=st->total_cost;
  stat->max_mana=st->points.max_mana;
  stat->max_hit=st->points.max_hit;
  stat->gold=st->points.gold+st->points.bank;
  stat->subclass=0;
  stat->subclass_level=0;
}

int main(int argc, char **argv) {
  FILE *fl;
  struct char_file_u_5 char_data_5;
  struct char_file_u_4 char_data_4;
  struct char_file_u_2 char_data_2;
  struct stat_file_u stat_data;
  char buf[MAX_STRING_LENGTH];
  int version;

  if (!argv[1]) {
    printf ("Usage: %s <name in lower case>.dat\n\r",argv[0]);
    exit(0);
  }

  sprintf(buf,"%s",argv[1]);

  if (!(fl = fopen(buf, "rb"))) {
    printf ("::: %s didnt have .dat file.\n\r",argv[1]);
    return (-1);
  }

  version=char_version(fl);

  switch(version) {
    case 2:
      if((fread(&char_data_2,sizeof(struct char_file_u_2),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      read_char_store_2(&char_data_2,&stat_data);
      break;
    case 3:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      read_char_store_4(&char_data_4,&stat_data);
      if(char_data_4.level>=31) char_data_4.level=51;
      break;
    case 4:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      read_char_store_4(&char_data_4,&stat_data);
      break;
    case 5:
      if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      read_char_store_5(&char_data_5,&stat_data);
      break;
    default:
      printf("Error getting pfile version(statlist)");
      return(-1);
  }
  fclose(fl);

  if (!(fl = fopen("statlist.dat", "ab"))) {
    printf ("::: Error opening statlist.dat.\n\r");
    return (-1);
  }
  fwrite(&stat_data,sizeof(stat_data),1,fl);
  fclose(fl);
  exit (0);
}

