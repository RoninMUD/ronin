/*
** Program goldstats.c
**
** Written for RoninMUD by Ranger
** Not to be distributed without author consent.
**
** This program takes the list of stats produced by
** statlist and outputs a text file for gold usage
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:09:24 $
$Header: /home/ronin/cvs/ronin/goldstats.c,v 2.0.0.1 2004/02/05 16:09:24 ronin Exp $
$Id: goldstats.c,v 2.0.0.1 2004/02/05 16:09:24 ronin Exp $
$Name:  $
$Log: goldstats.c,v $
Revision 2.0.0.1  2004/02/05 16:09:24  ronin
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
#include <math.h>

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
};

char *string_to_lower(char *string) {
  int i;
  for (i=0;i<strlen(string);i++) {
    string[i] = LOWER(string[i]);
  }
  return (string);
}

int main(int argc, char **argv) {
  FILE *fl,*f2;
  struct stat_file_u stat;
  int i;
  double tot_gold=0,avg_gold=0,max_gold=0,sum_gold_squared=0,std_dev_gold=0;
  int count=0,one_h=0,fifty=0,twenty=0,ten=0,five=0,two=0,one=0,zero=0;

  if (!(fl = fopen("statlist.dat", "rb"))) {
    printf ("::: Error opening statlist.dat.\n\r");
    return (-1);
  }
  if(!(f2 = fopen("gold.list", "wa"))) {
    printf ("::: Error opening gold list output file.\n\r");
    return (-1);
  }

  while(!feof(fl)) {
    if((fread(&stat,sizeof(struct stat_file_u),1,fl))!=1) {
      break;
    }

    if(stat.level > 19 && stat.level < 51) {
      fprintf(f2,"%s,%d\n\r",stat.name,stat.gold);
      count++;
      tot_gold+=(double)stat.gold;
      sum_gold_squared+=(double)stat.gold*(double)stat.gold;
      if(stat.gold > max_gold)
        max_gold=(double)stat.gold;
      if(stat.gold>100000000) one_h++;
      else if(stat.gold>50000000) fifty++;
      else if(stat.gold>20000000) twenty++;
      else if(stat.gold>10000000) ten++;
      else if(stat.gold>5000000) five++;
      else if(stat.gold>2000000) two++;
      else if(stat.gold>1000000) one++;
      else zero++;
    }
  }
  fclose(fl);
  fclose(f2);

  avg_gold=tot_gold/(double)count;
  std_dev_gold=sqrt((sum_gold_squared-tot_gold*tot_gold/(double)count)/(double)(count-1));

  if(!(fl = fopen("gold.stat", "wa"))) {
      printf ("::: Error opening stats output file.\n\r");
      return (-1);
    }
    fprintf(fl,"Gold stats for players from level 20 to 50\n\r");
    fprintf(fl,"Based on `n%d`q players.\n\r`q\n\r",count);
    fprintf(fl,"Total Gold:   `k%.0f`q\n\r",tot_gold);
    fprintf(fl,"Maximum Gold: `k%.0f`q\n\r",max_gold);
    fprintf(fl,"Average Gold: `k%.0f`q\n\r",avg_gold);
    fprintf(fl,"Std Dev Gold: `k%.0f`q\n\r",std_dev_gold);
    fprintf(fl,"\n\rGold Breakdown by range\n\r`q");
    fprintf(fl," > 100M: %d players\n\r",one_h);
    fprintf(fl," > 50M:  %d players\n\r",fifty);
    fprintf(fl," > 20M:  %d players\n\r",twenty);
    fprintf(fl," > 10M:  %d players\n\r",ten);
    fprintf(fl," >  5M:  %d players\n\r",five);
    fprintf(fl," >  2M:  %d players\n\r",two);
    fprintf(fl," >  1M:  %d players\n\r",one);
    fprintf(fl," <  1M:  %d players\n\r",zero);
    fclose(fl);

  exit (0);
}

