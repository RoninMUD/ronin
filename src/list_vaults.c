/*
**  File: list_vaults.c, Special module.                   Part of DIKUMUD
**  Will list the vaults a player has access to without causing machine lag.
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:09:36 $
$Header: /home/ronin/cvs/ronin/list_vaults.c,v 2.0.0.1 2004/02/05 16:09:36 ronin Exp $
$Id: list_vaults.c,v 2.0.0.1 2004/02/05 16:09:36 ronin Exp $
$Name:  $
$Log: list_vaults.c,v $
Revision 2.0.0.1  2004/02/05 16:09:36  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )
#define CAP(st)  (*(st) = UPPER(*(st)), st)

int main(int argc, char **argv) {
  char name[255],buf[4096];
  FILE *fd;
  int loop;

  if (!argv[1]) {
    printf("Unable to list access to vaults.\n\r");
    exit(0);
  }
  sprintf (name,"%s",argv[1]);

  remove("access.chk");
  sprintf(buf,"grep -lx %s vault/*.name > vault/access.chk",name);
  system(buf);

  printf("You have access to the following vaults...\n\r\n\r");

  /* Check for their own vault */
  sprintf(buf,"vault/%s.vault",name);
  if((fd=fopen(buf,"r"))) {
    printf("%s\n\r",CAP(name));
    fclose(fd);
  }

  if(!(fd=fopen("vault/access.chk","r"))) {
    printf("Unable to list access to further vaults.\n\r");
    exit(0);
  }

  loop=0;
  *name = '\0';
  while(!feof(fd) && loop<30) {
    fscanf(fd, "vault/%[a-z].name\n",name);
    if(name) {
      printf("%s\n\r",CAP(name));
    }
    loop++;
  }
  fclose(fd);
  exit(0);
}

