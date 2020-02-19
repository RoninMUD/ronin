/*
** Program charstats.c
**
** Written for RoninMUD by Ranger
** Not to be distributed without author consent.
**
** This program takes the list of stats produced by
** statlist and outputs a text file for each class
** with descriptive statistics
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:08:55 $
$Header: /home/ronin/cvs/ronin/charstats.c,v 2.0.0.1 2004/02/05 16:08:55 ronin Exp $
$Id: charstats.c,v 2.0.0.1 2004/02/05 16:08:55 ronin Exp $
$Name:  $
$Log: charstats.c,v $
Revision 2.0.0.1  2004/02/05 16:08:55  ronin
Reinitialization of cvs archives


Revision 1.4 2002/11/11 ranger
Addition of subclass and subclass level to charstats output.

Revision 1.3  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

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

char *string_to_lower(char *string) {
  int i;
  for (i=0;i<strlen(string);i++) {
    string[i] = LOWER(string[i]);
  }
  return (string);
}

int main(int argc, char **argv) {
  FILE *fl;
  struct stat_file_u stat;
  int i,cl_count[11],sc_count[22],sclvl_count[22][5];
  char name_hp[11][20],name_mana[11][20],name_gold[11][20],name_death[11][20];
  int avg_hp[11],max_hp[11];
  int avg_mana[11],max_mana[11];
  unsigned long avg_gold[11],max_gold[11],avg_rent[11];
  int avg_death[11],max_death[11];
  int avg_align[11];
  long avg_age[11];
  char *filename[]={
    "mu.stat","cl.stat","th.stat","wa.stat","ni.stat",
    "no.stat","pa.stat","ap.stat","av.stat","ba.stat","co.stat"
  };
  char *class[]={
    "Magic User","Cleric","Thief","Warrior","Ninja","Nomad",
    "Paladin","Anti-Paladin","Avatar","Bard","Commando"
  };

  char *subclass_name[] = {
	"ENCHANTER","ARCHMAGE","DRUID","TEMPLAR","ROGUE","BANDIT",
	"WARLORD","GLADIATOR","RONIN","MYSTIC","RANGER","TRAPPER",
	"CAVALIER","CRUSADER","DEFILER","INFIDEL",
	"","",/* Avatar*/
	"BLADESINGER","CHANTER","LEGIONNAIRE","MERCENARY"
  };

  if (!(fl = fopen("statlist.dat", "rb"))) {
    printf ("::: Error opening statlist.dat.\n\r");
    return (-1);
  }

  /* Make sure all counters are set to zero */
  for(i=0;i<11;i++) {
    cl_count[i]=0;
    avg_hp[i]=0;max_hp[i]=0;
    avg_mana[i]=0;max_mana[i]=0;
    avg_gold[i]=0;max_gold[i]=0;
    avg_death[i]=0;max_death[i]=0;
    avg_align[i]=0;avg_rent[i]=0;
    avg_age[i]=0;
    sc_count[i]=0;
    sc_count[i+11]=0;
    sclvl_count[i][0]=0;
    sclvl_count[i+11][0]=0;
    sclvl_count[i][1]=0;
    sclvl_count[i+11][1]=0;
    sclvl_count[i][2]=0;
    sclvl_count[i+11][2]=0;
    sclvl_count[i][3]=0;
    sclvl_count[i+11][3]=0;
    sclvl_count[i][4]=0;
    sclvl_count[i+11][4]=0;
  }

  while(!feof(fl)) {
    if((fread(&stat,sizeof(struct stat_file_u),1,fl))!=1) {
      break;
    }

    if(stat.level > 27 && stat.level < 51) {
      cl_count[stat.class-1]++;
      avg_hp[stat.class-1]+=stat.max_hit;
      if(stat.max_hit > max_hp[stat.class-1]) {
        max_hp[stat.class-1]=stat.max_hit;
        strcpy(name_hp[stat.class-1],stat.name);
      }
      stat.max_mana+=100;
      avg_mana[stat.class-1]+=stat.max_mana;
      if(stat.max_mana > max_mana[stat.class-1]) {
        max_mana[stat.class-1]=stat.max_mana;
        strcpy(name_mana[stat.class-1],stat.name);
      }
      avg_gold[stat.class-1]+=stat.gold;
      if(stat.gold > max_gold[stat.class-1]) {
        max_gold[stat.class-1]=stat.gold;
        strcpy(name_gold[stat.class-1],stat.name);
      }
      avg_death[stat.class-1]+=stat.been_killed;
      if(stat.been_killed > max_death[stat.class-1]) {
        max_death[stat.class-1]=stat.been_killed;
        strcpy(name_death[stat.class-1],stat.name);
      }
      avg_align[stat.class-1]+=stat.alignment;
      avg_rent[stat.class-1]+=stat.total_cost;
      avg_age[stat.class-1]+=stat.played;

      if(stat.subclass) {
        sc_count[stat.subclass-1]++;
        sclvl_count[stat.subclass-1][stat.subclass_level-1]++;
      }
    }
  }
  fclose(fl);

  for(i=0;i<11;i++) {
    if(!cl_count[i]) continue;
    avg_hp[i]/=cl_count[i];
    avg_mana[i]/=cl_count[i];
    avg_gold[i]/=cl_count[i];
    avg_death[i]/=cl_count[i];
    avg_align[i]/=cl_count[i];
    avg_rent[i]/=cl_count[i];
    avg_age[i]/=cl_count[i];
    if (!(fl = fopen(filename[i], "wa"))) {
      printf ("::: Error opening stats output file.\n\r");
      return (-1);
    }
    fprintf(fl,"Statistics for the Class of `i%s`q\n\r`q\n\r",class[i]);
    fprintf(fl,"Based on `n%d`q players between the level of `n28`q and `n50`q.\n\r`q\n\r",cl_count[i]);
    fprintf(fl,"Avg `jHps           `k%5d`q  Max `jHps`q           `k%5d `i(%s)`q\n\r",avg_hp[i],max_hp[i],name_hp[i]);
    fprintf(fl,"Avg `jMana          `k%5d`q  Max `jMana`q          `k%5d `i(%s)`q\n\r",avg_mana[i],max_mana[i],name_mana[i]);
    fprintf(fl,"Avg `jGold   `k%12ld`q  Max `jGold`q   `k%12ld `i(%s)`q\n\r",avg_gold[i],max_gold[i],name_gold[i]);
    fprintf(fl,"Avg `jDeaths          `k%3d`q  Max `jDeaths`q          `k%3d `i(%s)`q\n\r",avg_death[i],max_death[i],name_death[i]);
    fprintf(fl,"Avg `jAlign         `k%5d`q\n\r",avg_align[i]);
    fprintf(fl,"Avg `jRent   `k%12ld`q\n\r\n\r",avg_rent[i]);
    fprintf(fl,"Subclass Stats\n\r");
/* Compilation of the count for each sc level is not working for some reason
that I can't figure out - just outputting the total count for now - Ranger 12-Nov-02 */
/*    fprintf(fl,"Subclass: `j%s`q Total: `k%d`q Lvl1: `k%d`q Lvl2: `k%d`q Lvl3: `k%d`q Lvl4: `k%d`q Lvl5: `k%d`q\n\r",
               subclass_name[2*i],sc_count[2*i],sclvl_count[2*i,0],sclvl_count[2*i,1],sclvl_count[2*i,2],sclvl_count[2*i,3],sclvl_count[2*i,4]);
    fprintf(fl,"Subclass: `j%s`q Total: `k%d`q Lvl1: `k%d`q Lvl2: `k%d`q Lvl3: `k%d`q Lvl4: `k%d`q Lvl5: `k%d`q\n\r",
               subclass_name[2*i+1],sc_count[2*i+1],sclvl_count[2*i+1,0],sclvl_count[2*i+1,1],sclvl_count[2*i+1,2],sclvl_count[2*i+1,3],sclvl_count[2*i+1,4]);
*/
    fprintf(fl,"Subclass: `j%s`q Total: `k%d`q\n\r",
               subclass_name[2*i],sc_count[2*i]);
    fprintf(fl,"Subclass: `j%s`q Total: `k%d`q\n\r",
               subclass_name[2*i+1],sc_count[2*i+1]);
    fclose(fl);
  }
  exit (0);
}

