/* Procedure vcheck for RoninMUD
**
** Looks at the vaults in the current directory (should
** be ran in lib/vault) and makes sure the owner exists
** and the people that have access exist.
**
** This should be ran after any pfile purge
**
** Author: Ranger
** Creation Date: Dec 11, 1997
** Last Modified: Dec 11, 1997
**
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:11:59 $
$Header: /home/ronin/cvs/ronin/vcheck.c,v 2.0.0.1 2004/02/05 16:11:59 ronin Exp $
$Id: vcheck.c,v 2.0.0.1 2004/02/05 16:11:59 ronin Exp $
$Name:  $
$Log: vcheck.c,v $
Revision 2.0.0.1  2004/02/05 16:11:59  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )
#define BUFFERSIZE 65535

int main(int argc, char **argv) {
  char name[100],name2[100];
  char name3[100],filename[100];
  char buf[255];
  int changed,recheck,loop=0;
  FILE *fd,*fd2,*fd3;

  printf("Program Vault Check\n\r\n\r");
  printf("Only to be ran in the lib/vault directory.\n\r\n\r");
  printf("Makes sure existing vaults have owners and players.\n\r");
  printf("with access exist.\n\r\n\r");
  printf("Should be ran after any player file purge.\n\r");

  remove("vcheck.chk");
  system("ls -1 *.name > vcheck.chk");
  if(!(fd=fopen("vcheck.chk","r"))) {
    printf("Error: No vcheck.chk file created\n\r");
    exit(0);
  }

  while(!feof(fd) && loop<1000) {
    *name = '\0';
    fscanf(fd, "%[a-z].name\n",name);
    if(name) {
      /* Check for vault owner existing */
      sprintf(filename,"../rent/%c/%s.dat",UPPER(name[0]),name);
      if(!(fd2=fopen(filename,"rb"))) {
        sprintf(buf,"mv %s.name ../VPURGED/%s.name",name,name);
        system(buf);
        sprintf(buf,"mv %s.vault ../VPURGED/%s.vault",name,name);
        system(buf);
      }
      else {
        fclose(fd2);
        /* Check for players with access actually existing */
        recheck=1;
        while(recheck) {
          sprintf(filename,"%s.name",name);
          if(!(fd2=fopen(filename,"r"))) {
            sprintf(buf,"Error: %s doesn't exist.\n\r",filename);
            printf("%s",buf);
            exit(0);
          }
          changed=0;
          while(!changed && !feof(fd2)) {
            *name2 = '\0';
            fscanf(fd2, "%s\n",name2);
            if(name2) {
              sprintf(filename,"../rent/%c/%s.dat",UPPER(name2[0]),name2);
              if(!(fd3=fopen(filename,"rb"))) {
                changed=1;
                rewind(fd2);
                fd3=fopen("vcheck.names","w");
                while (!feof(fd2)) {
                  *name3 = '\0';
                  if(fscanf(fd2, "%s\n",name3)) {
                    if(strcmp(name2, name3))
	                    fprintf(fd3,"%s\n",name3);
                  }
                }
                fclose(fd3);
                sprintf (buf, "mv vcheck.names %s.name", name);
                system (buf);
              } else {
                fclose(fd3);
              }
            }
          }
          fclose(fd2);
          if(changed) recheck=1;
          else recheck=0;
        }
      }
    }
    loop++;
  }
  if(loop>999) printf("Went over 1000 checks - ensure this is correct.\n\r");
  fclose(fd);
  exit (0);
}

