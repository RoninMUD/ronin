
/*
$Author: ronin $
$Date: 2005/01/21 14:55:28 $
$Header: /home/ronin/cvs/ronin/nlp.c,v 2.1 2005/01/21 14:55:28 ronin Exp $
$Id: nlp.c,v 2.1 2005/01/21 14:55:28 ronin Exp $
$Name:  $
$Log: nlp.c,v $
Revision 2.1  2005/01/21 14:55:28  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.0.0.1  2004/02/05 16:09:51  ronin
Reinitialization of cvs archives

Revision 1.3  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.2  2002/03/31 07:42:15  ronin
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
#define IS_SET(flag,bit)  ((flag) & (bit))

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

void read_char_store_2 (struct char_file_u_2 *st) {

    printf("%s %d %s ",st->name,st->level,st->pwd);
  if(IS_SET(st->new.imm_flags, WIZ_ACTIVE))
    printf(" 1\n\r");
  else
    printf(" 0\n\r");
}

void read_char_store_4 (struct char_file_u_4 *st) {
  printf("%s %d %s ",st->name,st->level,st->pwd);
  if(IS_SET(st->new.imm_flags, WIZ_ACTIVE))
    printf(" 1\n\r");
  else
    printf(" 0\n\r");
}

void read_char_store_5 (struct char_file_u_5 *st) {
  printf("%s %d %s ",st->name,st->level,st->pwd);
  if(IS_SET(st->new.imm_flags, WIZ_ACTIVE))
    printf(" 1\n\r");
  else
    printf(" 0\n\r");
}

int main(int argc, char **argv) {
  FILE *fl;
  struct char_file_u_5 char_data_5;
  struct char_file_u_4 char_data_4;
  struct char_file_u_2 char_data_2;
  char buf[MAX_STRING_LENGTH];
  int version;

  if (!argv[1]) {
    printf ("Usage: %s <name in lower case.dat>\n\r",argv[0]);
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
      read_char_store_2(&char_data_2);
      break;
    case 3:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      read_char_store_4(&char_data_4);
      if(char_data_4.level>=31) char_data_4.level=51;
      break;
    case 4:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      read_char_store_4(&char_data_4);
      break;
    case 5:
      if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      read_char_store_5(&char_data_5);
      break;
    default:
      printf("Error getting pfile version(nlp)");
      return(-1);
  }

  fclose(fl);
  exit (0);
}

