/* Procedure pcompare for RoninMUD
**
** Looks at the pfiles in two diff directories (all
** pfiles should be in the sub dirs, not in A, B, etc)
** and compares the natural hp and mana between pfiles.
** any jumps over 80 mana/hp and noted and logged.
**
** Author: Ranger
** Creation Date: May 18, 1998
** Last Modified: May 18, 1999
**
*/

/*
$Author: ronin $
$Date: 2005/01/21 14:55:28 $
$Header: /home/ronin/cvs/ronin/pcompare.c,v 2.1 2005/01/21 14:55:28 ronin Exp $
$Id: pcompare.c,v 2.1 2005/01/21 14:55:28 ronin Exp $
$Name:  $
$Log: pcompare.c,v $
Revision 2.1  2005/01/21 14:55:28  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.0.0.1  2004/02/05 16:09:53  ronin
Reinitialization of cvs archives

Revision 1.4  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.3  2002/03/31 07:52:59  ronin
Addition of case 4 in player file version check.

$State: Exp $
*/


#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define BUFFERSIZE 65535

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

int stat5(struct char_file_u_5 *st,int which) {

	switch (which) {
	case 1:
	  return(st->points.max_mana);
	  break;
	case 2:
		return(st->points.max_hit);
	  break;
	}
	return 0;

}

int stat4(struct char_file_u_4 *st,int which) {

	switch (which) {
	case 1:
	  return(st->points.max_mana);
	  break;
	case 2:
		return(st->points.max_hit);
	  break;
	}
	return 0;

}

int stat2(struct char_file_u_2 *st,int which) {

	switch (which) {
	case 1:
	  return(st->points.max_mana);
	  break;
	case 2:
		return(st->points.max_hit);
	  break;
	}
	return 0;

}

int main(int argc, char **argv) {
  FILE *fl;
  char name[100],name2[100];
  char name3[100],filename[100];
  FILE *fd,*fd2,*fd3;
  struct char_file_u_5 char_data_5;
  struct char_file_u_4 char_data_4;
  struct char_file_u_2 char_data_2;
  char buf[MAX_STRING_LENGTH],dir1[MAX_STRING_LENGTH],dir2[MAX_STRING_LENGTH];
  int version,mana1,mana2,hit1,hit2;

  printf("Program Pfile Compare\n\r\n\r");
  printf("Usage: pcompare <dir1> <dir2>.\n\r\n\r");
  printf("Will compare all pfiles in dir1 with those in dir2\n\r");
  printf("and log any unusual jumps in stats.\n\r\n\r");
  printf("Assumes all pfiles are in one diretory.\n\r\n\r");

  if (!argv[1]) {
    printf ("Usage: %s <dir1> <dir2>\n\r",argv[0]);
    exit(0);
  }

  if (!argv[2]) {
    printf ("Usage: %s <dir1> <dir2>\n\r",argv[0]);
    exit(0);
  }

  sprintf(dir1,"%s",argv[1]);
  sprintf(dir2,"%s",argv[2]);

  remove("pcompare.chk");
  sprintf(buf,"ls -1 %s > pcompare.chk",dir1);
  system(buf);
  if(!(fd=fopen("pcompare.chk","r"))) {
    printf("Error: No pcompare.chk file created\n\r");
    exit(0);
  }

  sprintf(buf,"%s-%s.cmp",dir1,dir2);
  if(!(fd2=fopen(buf,"w"))) {
    printf("Error: No summary created\n\r");
    exit(0);
  }
  fprintf(fd2,"SUMMARY FOR: %s - %s\n\r\n\r",dir1,dir2);

  while(!feof(fd)) {
    mana1=0;mana2=0;hit1=0;hit2=0;
    *name = '\0';
    fscanf(fd, "%[a-z].dat\n",name);

    sprintf(buf,"%s/%s.dat",dir1,name);
    if (!(fl = fopen(buf, "rb"))) {
      printf ("::: %s didnt have .dat file.\n\r",name);
      continue;
    }

    version=char_version(fl);

    switch(version) {
      case 2:
        if((fread(&char_data_2,sizeof(struct char_file_u_2),1,fl))!=1) {
          printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
          continue;
        }
	      mana1=stat2(&char_data_2,1);
	      hit1=stat2(&char_data_2,2);
        break;
      case 3:
      case 4:
        if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1) {
          printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
          continue;
        }
	      mana1=stat4(&char_data_4,1);
        hit1=stat4(&char_data_4,2);
        break;
      case 5:
        if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1) {
          printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
          continue;
        }
	      mana1=stat5(&char_data_5,1);
        hit1=stat5(&char_data_5,2);
        break;
      default:
        printf("Error getting pfile version(pcompare)");
        continue;
    }
    fclose(fl);

    sprintf(buf,"%s/%s.dat",dir2,name);
    if (!(fl = fopen(buf, "rb"))) {
      printf ("::: %s didnt have .dat file.\n\r",name);
      continue;
    }

    version=char_version(fl);

    switch(version) {
      case 2:
        if((fread(&char_data_2,sizeof(struct char_file_u_2),1,fl))!=1) {
          printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
          continue;
        }
	      mana2=stat2(&char_data_2,1);
	      hit2=stat2(&char_data_2,2);
        break;
      case 3:
      case 4:
        if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1) {
          printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
          continue;
        }
	      mana2=stat4(&char_data_4,1);
        hit2=stat4(&char_data_4,2);
        break;
      case 5:
        if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1) {
          printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
          continue;
        }
	      mana2=stat5(&char_data_5,1);
        hit2=stat5(&char_data_5,2);
        break;
      default:
        printf("Error getting pfile version(pcompare)");
        continue;
    }
    fclose(fl);

    if((mana2>mana1+70)) {
      hit1=mana2-mana1;
      fprintf(fd2,"ALERT: %20s M2: %5d M1: %5d DIFF: %5d\n\r",name,mana2,mana1,hit1);
    }
  }
  fclose(fd);
  fclose(fd2);
  exit (0);
}
