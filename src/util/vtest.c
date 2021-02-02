
/*
$Author: ronin $
$Date: 2005/04/27 17:11:30 $
$Header: /home/ronin/cvs/ronin/vtest.c,v 2.3 2005/04/27 17:11:30 ronin Exp $
$Id: vtest.c,v 2.3 2005/04/27 17:11:30 ronin Exp $
$Name:  $
$Log: vtest.c,v $
Revision 2.3  2005/04/27 17:11:30  ronin
Addition of obj vnum to output.

Revision 2.2  2005/01/21 14:55:30  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.1  2004/03/04 17:23:58  ronin
Addition of object file version 2 which includes 8 ownerid fields
for addition of some objects only being able to be used by those
owners.

Revision 2.0.0.1  2004/02/05 16:12:00  ronin
Reinitialization of cvs archives

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

char *string_to_lower(char *string) {
  int i;
  for (i=0;i<strlen(string);i++) {
    string[i] = LOWER(string[i]);
  }
  return (string);
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

void read_obj_store_ver0 (FILE *fl,char *name) {
  struct obj_file_elem_ver0 object;

  memset(&object,0,sizeof (struct obj_file_elem_ver0));
  fread(&object,sizeof(struct obj_file_elem_ver0),1,fl);

  /* Print out the object stats and free the memory */

  if (object.item_number)
    printf ("(%s) #%-5d V(%d %d %d %d)\tE%-10d\tB%-10d\tAP(%-2d %d/%-2d %d)\r\n",
	    name,object.item_number,
	    object.value[0],object.value[1],object.value[2],object.value[3],
	    object.extra_flags,
	    object.bitvector,
	    object.affected[0].location,
	    object.affected[0].modifier,
	    object.affected[1].location,
	    object.affected[1].modifier);
}

void read_obj_store_ver1(FILE *fl,char *name) {
  struct obj_file_elem_ver1 object;

  memset(&object,0,sizeof (struct obj_file_elem_ver1));
  fread(&object,sizeof(struct obj_file_elem_ver1),1,fl);

  /* Print out the object stats and free the memory */

  if (object.item_number)
    printf ("(%s) #%-5d V(%d %d %d %d)\tE%-10d\tB%-10d\tAP(%-2d %d/%-2d %d)\r\n",
	    name,object.item_number,
	    object.value[0],object.value[1],object.value[2],object.value[3],
	    object.extra_flags,
	    object.bitvector,
	    object.affected[0].location,
	    object.affected[0].modifier,
	    object.affected[1].location,
	    object.affected[1].modifier);
}

void read_obj_store_ver2(FILE *fl,char *name) {
  struct obj_file_elem_ver2 object;

  memset(&object,0,sizeof (struct obj_file_elem_ver2));
  fread(&object,sizeof(struct obj_file_elem_ver2),1,fl);

  /* Print out the object stats and free the memory */

  if (object.item_number)
    printf ("(%s) #%-5d V(%d %d %d %d)\tE%-10d\tB%-10d\tAP(%-2d %d/%-2d %d)\r\n",
	    name,object.item_number,
	    object.value[0],object.value[1],object.value[2],object.value[3],
	    object.extra_flags,
	    object.bitvector,
	    object.affected[0].location,
	    object.affected[0].modifier,
	    object.affected[1].location,
	    object.affected[1].modifier);
}

void read_obj_store_ver3(FILE *fl,char *name) {
  struct obj_file_elem_ver3 object;

  memset(&object,0,sizeof (struct obj_file_elem_ver3));
  fread(&object,sizeof(struct obj_file_elem_ver3),1,fl);

  /* Print out the object stats and free the memory */

  if (object.item_number)
    printf ("(%s) #%-5d V(%d %d %d %d)\tE%-10d\tB%-10d%-10d\tAP(%-2d %d/%-2d %d)\r\n",
	    name,object.item_number,
	    object.value[0],object.value[1],object.value[2],object.value[3],
	    object.extra_flags,
	    object.bitvector,object.bitvector2,
	    object.affected[0].location,
	    object.affected[0].modifier,
	    object.affected[1].location,
	    object.affected[1].modifier);
}

int main(int argc, char **argv) {
  FILE *fl;
  char buf[MAX_STRING_LENGTH];

  if (!argv[1]) {
    printf ("Usage: %s <vaultname in lower case>\n\r",argv[0]);
    exit(0);
  }
  sprintf (buf, "%s", argv[1]);

  if (!(fl = fopen(buf, "rb"))) {
    printf ("::: %s didnt have a vault file.\n\r",argv[1]);
    return (-1);
  }

  printf ("Vault file dump for character %s.:\n\r",argv[1]);

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

