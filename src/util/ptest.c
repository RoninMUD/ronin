
/*
$Author: ronin $
$Date: 2005/04/27 17:11:51 $
$Header: /home/ronin/cvs/ronin/ptest.c,v 2.3 2005/04/27 17:11:51 ronin Exp $
$Id: ptest.c,v 2.3 2005/04/27 17:11:51 ronin Exp $
$Name:  $
$Log: ptest.c,v $
Revision 2.3  2005/04/27 17:11:51  ronin
Addition of obj vnum to output.

Revision 2.2  2005/01/21 14:55:28  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.1  2004/03/04 17:23:57  ronin
Addition of object file version 2 which includes 8 ownerid fields
for addition of some objects only being able to be used by those
owners.

Revision 2.0.0.1  2004/02/05 16:09:55  ronin
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

void read_obj_store_ver0(FILE *fl,char *name) {
  struct obj_file_elem_ver0 object;

  memset(&object,0,sizeof(struct obj_file_elem_ver0));
  fread(&object,sizeof(struct obj_file_elem_ver0),1,fl);

  /* Print out the object stats and free the memory */

  if (object.item_number)
    printf ("(%s) #%-5d V(%d %d %d %d)\tE%-10d\tB%-10ld\tAP(%-2d %d/%-2d %d)\r\n",
	    name,object.item_number,
	    object.value[0],object.value[1],object.value[2],object.value[3],
	    object.extra_flags,
	    (long)object.bitvector,
	    object.affected[0].location,
	    object.affected[0].modifier,
	    object.affected[1].location,
	    object.affected[1].modifier);
}

void read_obj_store_ver1(FILE *fl,char *name) {
  struct obj_file_elem_ver1 object;

  memset(&object,0,sizeof(struct obj_file_elem_ver1));
  fread(&object,sizeof(struct obj_file_elem_ver1),1,fl);

  /* Print out the object stats and free the memory */

  if (object.item_number)
    printf ("(%s) #%-5d V(%d %d %d %d)\tE%-10d\tB%-10ld\tAP(%-2d %d/%-2d %d)\r\n",
	    name,object.item_number,
	    object.value[0],object.value[1],object.value[2],object.value[3],
	    object.extra_flags,
	    (long)object.bitvector,
	    object.affected[0].location,
	    object.affected[0].modifier,
	    object.affected[1].location,
	    object.affected[1].modifier);
}

void read_obj_store_ver2(FILE *fl,char *name) {
  struct obj_file_elem_ver2 object;

  memset(&object,0,sizeof(struct obj_file_elem_ver2));
  fread(&object,sizeof(struct obj_file_elem_ver2),1,fl);

  /* Print out the object stats and free the memory */

  if (object.item_number)
    printf ("(%s) #%-5d V(%d %d %d %d)\tE%-10d\tB%-10ld\tAP(%-2d %d/%-2d %d)\r\n",
	    name,object.item_number,
	    object.value[0],object.value[1],object.value[2],object.value[3],
	    object.extra_flags,
	    (long)object.bitvector,
	    object.affected[0].location,
	    object.affected[0].modifier,
	    object.affected[1].location,
	    object.affected[1].modifier);
}

void read_obj_store_ver3(FILE *fl,char *name) {
  struct obj_file_elem_ver3 object;

  memset(&object,0,sizeof(struct obj_file_elem_ver3));
  fread(&object,sizeof(struct obj_file_elem_ver3),1,fl);

  /* Print out the object stats and free the memory */

  if (object.item_number)
    printf ("(%s) #%-5d V(%d %d %d %d)\tE%-10d\tB%-10ld\tAP(%-2d %d/%-2d %d)\r\n",
	    name,object.item_number,
	    object.value[0],object.value[1],object.value[2],object.value[3],
	    object.extra_flags,
	    (long)object.bitvector,(long)object.bitvector2,
	    object.affected[0].location,
	    object.affected[0].modifier,
	    object.affected[1].location,
	    object.affected[1].modifier);
}

/* copy data from the file structure to a char struct */
void read_char_store_2 (struct char_file_u_2 *st) {
  int i;

	printf("Mother-Host : [%s]\n\r", st->new.host);
  printf ("SEX : %d CLS : %d LVL : %d\tplayed:%ld\tAL:%-4d\tPLR:%ld\n\r",
	  st->sex, st->class, st->level, (long)st->played,
	  st->alignment, (long)st->pflag);

  printf ("Str %2d(%2d) Int %2d Wis %2d Dex %2d Con %2d\n\r",
	  st->abilities.str, st->abilities.str_add,
	  st->abilities.intel, st->abilities.wis,
	  st->abilities.dex, st->abilities.con);

  printf ("MANA : %d HIT : %d MOVE : %d GOLD : (%d / %d) EXP : %d\n\r",
	  st->points.max_mana, st->points.max_hit,
	  st->points.max_move, st->points.gold,
	  st->points.bank, st->points.exp);

/*  if (st->level < LEVEL_IMM)
    for (i = 0; i <= MAX_SKILLS - 1; i++) {
      if(st->skills[i].learned > 85)
	printf ("skill : %d at %d\n\r",i, st->skills[i].learned);
    }*/
}
/* copy data from the file structure to a char struct */
void read_char_store_4 (struct char_file_u_4 *st) {
  int i;

	printf("Mother-Host : [%s]\n\r", st->new.host);
  printf ("(%s) SEX : %d CLS : %d LVL : %d\tplayed:%ld\tAL:%-4d\tPLR:%ld\n\r",
	  st->name,st->sex, st->class, st->level, (long)st->played,
	  st->alignment, (long)st->pflag);

  printf ("(%s) Str %2d(%2d) Int %2d Wis %2d Dex %2d Con %2d\n\r",
	  st->name,st->abilities.str, st->abilities.str_add,
	  st->abilities.intel, st->abilities.wis,
	  st->abilities.dex, st->abilities.con);

  printf ("(%s) MANA : %d HIT : %d MOVE : %d GOLD : (%d / %d) EXP : %d SCPTS : %d\n\r",
	  st->name,st->points.max_mana, st->points.max_hit,
	  st->points.max_move, st->points.gold,
	  st->points.bank, st->points.exp,st->ver3.subclass_points);

  if (st->level < LEVEL_IMM)
    for (i = 0; i <= MAX_SKILLS4 - 1; i++) {
      if(st->skills[i].learned > 85)
	printf ("skill : %d at %d\n\r",i, st->skills[i].learned);
    }
}

void read_char_store_5 (struct char_file_u_5 *st) {
  int i;

	printf("Mother-Host : [%s]\n\r", st->new.host);
  printf ("(%s) SEX : %d CLS : %d LVL : %d\tplayed:%ld\tAL:%-4d\tPLR:%ld\n\r",
	  st->name,st->sex, st->class, st->level, (long)st->played,
	  st->alignment, (long)st->pflag);

  printf ("(%s) Str %2d(%2d) Int %2d Wis %2d Dex %2d Con %2d\n\r",
	  st->name,st->abilities.str, st->abilities.str_add,
	  st->abilities.intel, st->abilities.wis,
	  st->abilities.dex, st->abilities.con);

  printf ("(%s) MANA : %d HIT : %d MOVE : %d GOLD : (%d / %d) EXP : %d SCPTS : %d\n\r",
	  st->name,st->points.max_mana, st->points.max_hit,
	  st->points.max_move, st->points.gold,
	  st->points.bank, st->points.exp,st->ver3.subclass_points);

  if (st->level < LEVEL_IMM)
    for (i = 0; i <= MAX_SKILLS5 - 1; i++) {
      if(st->skills[i].learned > 85)
	printf ("skill : %d at %d\n\r",i, st->skills[i].learned);
    }
}

int main(int argc, char **argv) {
  FILE *fl;
  struct char_file_u_5 char_data_5;
  struct char_file_u_4 char_data_4;
  struct char_file_u_2 char_data_2;
  char buf[MAX_STRING_LENGTH];
  int version;

  if (!argv[1]) {
    printf ("Usage: %s <name in lower case>\n\r",argv[0]);
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
      printf ("Rent file dump on character %s.\n\r",argv[1]);
      read_char_store_2(&char_data_2);
      break;
    case 3:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      printf ("Rent file dump on character %s.\n\r",argv[1]);
      read_char_store_4(&char_data_4);
      if(char_data_4.level>=31) char_data_4.level=51;
      break;
    case 4:
      if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      printf ("Rent file dump on character %s.\n\r",argv[1]);
      read_char_store_4(&char_data_4);
      break;
    case 5:
      if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1) {
        printf ("::: Error Reading rent file on %s.\n\r", argv[1]);
        return (-1);
      }
      printf ("Rent file dump on character %s.\n\r",argv[1]);
      read_char_store_5(&char_data_5);
      break;
    default:
      printf("Error getting pfile version(ptest)");
      return(-1);
  }

  while (!feof(fl)) {
    switch(obj_version(fl)) {
      case 3:
        read_obj_store_ver3(fl,argv[1]);
        break;
      case 2:
        read_obj_store_ver2(fl,argv[1]);
        break;
      case 1:
        read_obj_store_ver1(fl,argv[1]);
        break;
      case 0:
        read_obj_store_ver0(fl,argv[1]);
        break;
    }
  }

  fclose(fl);
  exit (0);
}

