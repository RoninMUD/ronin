/* ************************************************************************
*  file: utility.c, Utility module.                       Part of DIKUMUD *
*  Usage: Utility procedures                                              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2005/01/21 14:55:30 $
$Header: /home/ronin/cvs/ronin/utility.c,v 2.4 2005/01/21 14:55:30 ronin Exp $
$Id: utility.c,v 2.4 2005/01/21 14:55:30 ronin Exp $
$Name:  $
$Log: utility.c,v $
Revision 2.4  2005/01/21 14:55:30  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.3  2004/09/30 15:58:10  ronin
Fixed V_OBJ to return 0 for ITEM_SKIN

Revision 2.2  2004/05/12 13:22:40  ronin
Added update_pos in check_equipment.

Revision 2.1  2004/03/13 05:27:13  pyro
updated for olc commands oname and owear

Revision 2.0.0.1  2004/02/05 16:11:58  ronin
Reinitialization of cvs archives


Revision - fix to allow players with perceive to see invis too

Revision - fix to see yourself in CAN_SEE proc

Revision 1.5  2002/06/30 11:49:42  ronin
Addition of check for maximum mana to prevent orb giving owners
over and above their maximum mana.

Revision 1.4  2002/04/17 15:47:55  ronin
Changed to do internal manipulation of syslog for hotboot fix.

Revision 1.3  2002/03/31 16:35:06  ronin
Added braces to remove ambiguous else warning.

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "utility.h"
#include "limits.h"
#include "modify.h"
#include "comm.h"
#include "cmd.h"
#include "spells.h"

extern struct time_data time_info;
extern struct weather_data weather_info;
extern int CHAOSMODE;
extern char *BKColor[];
extern char *Color[];

extern struct room_data *world;
extern struct obj_proto *obj_proto_table;
extern struct char_data *character_list;
extern struct mob_proto *mob_proto_table;

extern FILE *logfile;
void update_pos( struct char_data *ch );

/* Yet another random number generator - Ranger March 10, 98 */

/* Back to the old Ranger March 12, 98
#define      ran_m  (unsigned long)2147483647
#define      ran_q  (unsigned long)127773
#define      ran_a (unsigned int)16807
#define      ran_r (unsigned int)2836

static unsigned long ran_seed;

void slongrand(unsigned long initial_seed)
{
  ran_seed = initial_seed;
}

unsigned long longrand(void)
{
   register int lo, hi, test;

    hi   = ran_seed/ran_q;
    lo   = ran_seed%ran_q;

    test = ran_a*lo - ran_r*hi;

    if (test > 0)
          ran_seed = test;
    else
      ran_seed = test+ ran_m;

    return ran_seed;
}

*/

#define MULT 16807
#define RANMAX 2147483647L

static long randomnum = 1;

void slongrand(unsigned long seed)
{
      randomnum = seed ? (seed & RANMAX) : 1;
}

long nextlongrand(long seed)
{
      unsigned long lo, hi;

      lo = MULT * (long)(seed & 0xFFFF);
      hi = MULT * (long)((unsigned long)seed >> 16);
      lo += (hi & 0x7FFF) << 16;
      if (lo > RANMAX)
      {
            lo &= RANMAX;
            ++lo;
      }
      lo += hi >> 15;
      if (lo > RANMAX)
      {
            lo &= RANMAX;
            ++lo;
      }
      return (long)lo;
}

long longrand(void) {
      randomnum = nextlongrand(randomnum);
      return randomnum;
}



int MIN(int a, int b)
{
      return a < b ? a:b;
}


int MAX(int a, int b)
{
      return a > b ? a:b;
}


char *PERS(CHAR *ch, CHAR*vict)
{
  static char name[100];
  memset(name,0,sizeof(name));
  //name[0]= 0;
  if(CAN_SEE(vict, ch))
    {
    if(IS_MOB(ch))
      strcat(name,MOB_SHORT(ch));
    else
      {
      signal_char(ch, vict, MSG_SHOW_PRETITLE,name);
      strncat(name,ch->player.name,sizeof(name)-1);
      }
    }
  else
    {
    strcat(name, "somebody");
    }
  return name;
}

char *POSSESS(CHAR *ch, CHAR*vict)
{
  static char name[100];
  name[0]= 0;
  if(CAN_SEE(vict, ch))
    {
    if(IS_MOB(ch)) {
      strcat(name,MOB_SHORT(ch));
      strcat(name,"'s");
    }
    else
      {
      signal_char(ch, vict, MSG_SHOW_PRETITLE,name);
      strcat(name,ch->player.name);
      strcat(name,"'s");
      }
    }
  else
    {
    strcat(name, "somebody's");
    }
  return name;
}

char *CHCLR(CHAR *ch, int color) {
  static char colorcode1[100];
  colorcode1[0]=0;
  if(ch->colors[0]&&ch->colors[color]) {
    strcat(colorcode1,Color[(((ch->colors[color])*2)-2)]);
    strcat(colorcode1,BKColor[ch->colors[13]]);
  }
  return colorcode1;
}

char *ENDCHCLR(CHAR *ch) {
  static char colorcode2[100];
  colorcode2[0]=0;
  if(ch->colors[0]&&ch->colors[1]) {
    strcat(colorcode2,Color[(((ch->colors[1])*2)-2)]);
    strcat(colorcode2,BKColor[ch->colors[13]]);
  }
  return colorcode2;
}

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
  int randnum,temp;
  char buf[MSL];

  if(from>to) {
    temp = from;
    from = to;
    to = temp;
  }

/*  return((random() % (to - from + 1)) + from);*/

  randnum=(int)((longrand() % (to - from + 1)) + from);
  if(randnum<from || randnum>to) {
    sprintf(buf,"WIZINFO: Random Number beyond range. From: %d To: %d  Num: %d",from,to,randnum);
    log_f(buf);
  }
  return randnum;
}

bool chance(int num) {
  if(number(1,100)<=num) return TRUE;
  return FALSE;
}

/* 50% true at same level, 0% true vict 10 above, 100% true vict 10 below */
/* Bonuses by class added Nov 23/99*/
bool breakthrough(CHAR *ch, CHAR *vict,int btype) {
  int num;
  num=50+(GET_LEVEL(ch)-GET_LEVEL(vict))*5;
  switch(GET_CLASS(ch)) {
    case CLASS_CLERIC:
      if(btype==BT_INVUL) num-=10;
      if(btype==BT_SPHERE) num-=5;
      break;
    case CLASS_MAGIC_USER:
      if(btype==BT_INVUL) num-=10;
      if(btype==BT_SPHERE) num+=10;
      break;
    case CLASS_WARRIOR:
      if(btype==BT_INVUL) num+=10;
      if(btype==BT_SPHERE) num-=10;
      break;
    case CLASS_NOMAD:
      if(btype==BT_INVUL) num+=5;
      if(btype==BT_SPHERE) num-=10;
      break;
    case CLASS_THIEF:
      if(btype==BT_INVUL) num+=10;
      if(btype==BT_SPHERE) num-=10;
      break;
    case CLASS_NINJA:
      if(btype==BT_INVUL) num+=5;
      if(btype==BT_SPHERE) num-=5;
      break;
    case CLASS_ANTI_PALADIN:
      if(btype==BT_INVUL) num+=5;
      if(btype==BT_SPHERE) num+=5;
      break;
    case CLASS_PALADIN:
      if(btype==BT_INVUL) num+=5;
      if(btype==BT_SPHERE) num-=5;
      break;
    case CLASS_BARD:
      if(btype==BT_INVUL) num+=5;
      break;
    case CLASS_COMMANDO:
      if(btype==BT_INVUL) num+=5;
      if(btype==BT_SPHERE) num+=5;
      break;
    default:
      break;
  }

  if(number(1,100)<=num) return TRUE;
  return FALSE;
}


int GETOBJ_WEIGHT(struct obj_data *obj)
{
  int  tmp_weight = 0;
  struct obj_data *tmp;

  if (obj->obj_flags.type_flag == ITEM_DRINKCON)
    tmp_weight = obj->obj_flags.weight*(.5 + (float)
                              ((float)obj->obj_flags.value[1]/2.0/(float)obj->obj_flags.value[0]));
  else
    tmp_weight = obj->obj_flags.weight;

  if (obj->contains)
    {
      for(tmp=obj->contains;tmp;tmp=tmp->next_content)
      tmp_weight += GETOBJ_WEIGHT(tmp);
    }
  return(tmp_weight);
}
int IS_DARK(int room) {
  if(world[room].zone==30) return FALSE;
  if(world[room].light) return FALSE;
  if(IS_SET(world[room].room_flags, DARK)) return TRUE;
  if(IS_SET(world[room].room_flags, INDOORS)) return FALSE;
  if(world[room].sector_type==SECT_INSIDE) return FALSE;
  if(IS_SET(world[room].room_flags, LIT)) return FALSE;
  if(weather_info.sunlight==SUN_SET || weather_info.sunlight==SUN_DARK) return TRUE;
  return FALSE;
}

int IS_LIGHT(int room) {
  if(world[room].zone==30) return TRUE;
  if(world[room].light) return TRUE;
  if(IS_SET(world[room].room_flags, DARK)) return FALSE;
  if(IS_SET(world[room].room_flags, INDOORS)) return TRUE;
  if(world[room].sector_type==SECT_INSIDE) return TRUE;
  if(IS_SET(world[room].room_flags, LIT)) return TRUE;
  if(weather_info.sunlight==SUN_SET || weather_info.sunlight==SUN_DARK) return FALSE;
  return TRUE;
}

char *string_to_lower(char *string)
{
  int i;
  for (i=0;i<strlen(string);i++)
    {
      string[i] = LOWER(string[i]);
    }
  return (string);
}

char *string_to_upper(char *string)
{
  int i;
  for (i=0;i<strlen(string);i++)
    {
      string[i] = UPPER(string[i]);
    }
  return (string);
}

int IS_CARRYING_W(struct char_data *ch)
{
  struct obj_data *tmp;
  int    tmp_weight=0;
  for(tmp=ch->carrying;tmp;tmp=tmp->next_content)
    tmp_weight += GETOBJ_WEIGHT(tmp);
  return(tmp_weight);
}

int IS_CARRYING_N(struct char_data *ch)
{
  struct obj_data *tmp;
  int    i=0;
  for(tmp=ch->carrying;tmp;tmp=tmp->next_content)
    i++;
  return(i);
}

char *str_cut(char *source,char *dest,int number) {
  int y;
  char buf[MAX_INPUT_LENGTH];

  if(strlen(source)>MAX_INPUT_LENGTH ||
    strlen(source)<number) return source;
  sprintf(buf,source);
  for(y=0;source[y];y++){
    buf[y]=source[y+number];
  }
  dest[0]='\0';
  strncat(dest,buf,strlen(source)-number);
  return dest;
}

/*
**  sstrdel() - Delete multiple substrings
**
**  public domain by Shamim Islam
**
**  Usage: sstrdel(char * s,char * del0,del1...deln)
**
**  Remarks: sstrdel takes a string s, and removes all occurrences of
**           the substrings del0, del1, ... deln
**
**  Return:  sstrdel returns a pointer to the string s, unless s is NULL.
**           sstrdel will return a NULL for this exception.
**
**  Comment: Use sstrdel to remove a list of substrings from a string.
**
**           sstrdel matches the largest substring for deletion, if more than
**           one substring matches a particular portion of the string s.
**
**  NOTE:    The last element in the variable substring list MUST be NULL
**           or your program will have a high likelihood of hanging.
*/

char *sstrdel(char *s, ...)
{
      /* Find out how many arguments are present */

      int c = 0;
      va_list ap, hold;

      if (s == NULL)
            return NULL;
      va_start(ap, s);
      memcpy(&hold, &ap, sizeof(va_list));
      while (va_arg(ap, char*) != NULL)
            c++;
      va_end(ap);
      if (c)
      {
            /* Assign pointers  */

            char *r = s,*n = s;
            char *p;
            int len, i;

            /* Copy next character to result */
            /* And then check for matches if */
            /* not at end of string          */

            while ((*r = *n) != 0)
            {
                  int l = 0;

                  /* Substitute for va_start(ap,s) */

                  memcpy(&ap, &hold, sizeof(va_list));
                  for (i = 0; i < c; i++)
                  {
                        /* Initialise the pointer and the length    */

                        len = strlen(p = va_arg(ap, char*));

                        /* Compare ONLY if we haven't found one yet */
                        /* or if this one is bigger than the one we */
                        /* found AND this arg has a length > 0      */

                        if(len > 0 && (l == 0 || len> l) &&
                              strncmp(n, p, len) == 0)
                        {
                              l = len;
                        }
                  }
                  va_end(ap);
                  if (l)
                        n += l;
                  else  n++, r++;
            }
      }
      return s;
}

int COUNT_CONTENTS(struct obj_data *obj)
{
  struct obj_data *tmp;
  int    i=0;
  for(tmp=obj->contains;tmp;tmp=tmp->next_content)
    i++;
  return(i);
}

int COUNT_RENTABLE_CONTENTS(struct obj_data *obj)
{
  struct obj_data *tmp;
  int    i=0;
  for(tmp=obj->contains;tmp;tmp=tmp->next_content)
    if(IS_RENTABLE(tmp))
      i++;
  return(i);
}

/* simulates dice roll */
int dice(int number, int size)
{
  int r;
  int sum = 0;

  if ((number < 1) || (size < 1))
    return(0);

  for (r = 1; r <= number; r++) sum += ((random() % size)+1);
  return(sum);
}

/* Create a duplicate of a string */
char *str_dup(char *source)
{
  char *new;

  CREATE(new, char, strlen(source)+1);
  return(strcpy(new, source));
}

int str_cat(char *s, int len, int maxlen, const char *append)
{
  int i;

  assert(len <= maxlen);

  for ( i = 0; len + i < maxlen - 1 && *(append + i) != '\0'; i++)
    *(s + len + i) = *(append + i);

  *(s + len + i) = '\0';

  if(*(append + i) != '\0')
    log_f("BUG: too long append string in str_cat");

  return(len + i);
}


/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
  int chk, i;

  if (!arg1)
    return (-1);
  if (!arg2)
    return (1);

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
      if (chk < 0)
      return (-1);
      else
      return (1);
    }
  return(0);
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
  int chk, i;

  if (!arg1)
    return (-1);
  if (!arg2)
    return (1);
  for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n>0); i++, n--)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
      if (chk < 0)
      return (-1);
      else
      return (1);
    }

  return(0);
}

/* changes the supplied string buffer to contain all uppercase chars */
char *str_upper(char *str)
{
  char *p;

  p = str;

  while (*p) {
    *p = toupper(*p);
    p++;
  }

  return str;
}


/* writes a string to the log */

void log_string(char *str)
{
  long ct;
  char *tmstr;

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (logfile == NULL) {
    puts("SYSERR: Using log_string() before stream was initialized!");
    return;
  }
  fprintf(logfile, "%s :: %s\n", tmstr, str);
  fflush(logfile);
}

void log_f (char * fmt, ...)
{
  char buf [2*MSL];
  va_list args;
  va_start (args, fmt);
  vsnprintf (buf, 2*MSL, fmt, args);
  va_end (args);

  log_string (buf);
}

void deathlog(char *str)
{
  FILE *fl;
  long ct;
  char *tmstr;

  if(!(fl = fopen("death.log", "a"))) {
    log_f("Unable to open death.log file");
    return;
  }

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(fl, "%s :: %s\n", tmstr, str);
  fclose(fl);
}

void wizinfo(char *str, int level)
{
  char buf[MSL];
  struct descriptor_data *t;

  extern struct descriptor_data *descriptor_list;

  strcpy(buf,"** ");
  strcat(buf, str);
  strcat(buf," **\n\r");
  for (t = descriptor_list; t; t = t->next) {
    if (!t->connected &&
      GET_LEVEL(t->character) > LEVEL_MORT &&
      level <= t->wizinfo &&
      t->wizinfo != 0){
      send_to_char(buf, t->character);
    } /* if */
  } /* for */
}

void wizlog(char *str, int level, int which)
{
  char buf[MSL];
  struct descriptor_data *t;

  extern struct descriptor_data *descriptor_list;

  level = MIN(LEVEL_IMP, level);

  strcpy(buf, "[ ");
  strcat(buf, str);
  strcat(buf, " ]\n\r");
  for (t = descriptor_list; t; t = t->next) {
    if (!t->connected && GET_LEVEL(t->character) >= LEVEL_IMM &&
      level <= t->wizinfo && t->wizinfo != 0) {
      switch(which) {
      case 1: if (IS_SET(t->character->new.imm_flags, WIZ_LOG_ONE))
      send_to_char(buf, t->character);
      break;
      case 2: if (IS_SET(t->character->new.imm_flags, WIZ_LOG_TWO))
      send_to_char(buf, t->character);
      break;
      case 3: if (IS_SET(t->character->new.imm_flags, WIZ_LOG_THREE))
      send_to_char(buf, t->character);
      break;
      case 4: if (IS_SET(t->character->new.imm_flags, WIZ_LOG_FOUR))
      send_to_char(buf, t->character);
      break;
      case 5: if (IS_SET(t->character->new.imm_flags, WIZ_LOG_FIVE))
      send_to_char(buf, t->character);
      break;
      case 6: if (IS_SET(t->character->new.imm_flags, WIZ_LOG_SIX))
      send_to_char (buf, t->character);
      break;
      case 7: if (IS_SET(t->character->new.imm_flags, QUEST_INFO))
      send_to_char (buf, t->character);
      break;
      default: break;
      }
    } /* if */
  } /* for */
}


void sprintbit(long vektor, char *names[], char *result)
{
  long nr;

  *result = '\0';

  for(nr=0; vektor; vektor>>=1)
    {
      if (IS_SET(1, vektor)) {
        if (*names[nr] != '\n') {
          strcat(result,names[nr]);
          strcat(result," ");
        } else {
          strcat(result,"UNDEFINED");
          strcat(result," ");
        }
      }
      if (*names[nr] != '\n')
      nr++;
    }

  if (!*result)
    strcat(result, "NOBITS");
}

void sprinttype(int type, char *names[], char *result)
{
  int nr;

  for(nr=0;(*names[nr]!='\n');nr++);
  if(type < nr)
    strcpy(result,names[type]);
  else
    strcpy(result,"UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs/SECS_PER_REAL_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR*now.hours;

  now.day = (secs/SECS_PER_REAL_DAY);          /* 0..34 days  */
  secs -= SECS_PER_REAL_DAY*now.day;

  now.month = -1;
  now.year  = -1;

  return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
/* If time is negative, return 0 - Ranger August 99 */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  if(t2>t1)
    secs = (long) (t2 - t1);
  else
    secs = 0;

  now.hours = (secs/SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR*now.hours;

  now.day = (secs/SECS_PER_MUD_DAY) % 28;     /* 0..34 days  */
  secs -= SECS_PER_MUD_DAY*now.day;

  now.month = (secs/SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
  secs -= SECS_PER_MUD_MONTH*now.month;

  now.year = (secs/SECS_PER_MUD_YEAR);        /* 0..XX? years */

  return now;
}



struct time_info_data age(struct char_data *ch)
{
  struct time_info_data player_age;

  player_age = mud_time_passed(time(0),ch->player.time.birth);

  player_age.year += 17;   /* All players start at 17 */

  return player_age;
}

struct obj_data
*EQ( struct char_data *ch, int loc ) {
  if( ch->equipment[loc] ) return( ch->equipment[loc] );
  else return NULL;
}

char
is_carrying_obj (struct char_data *ch, int virtual) {
  struct obj_data *o;
  for (o=ch->carrying;o;o=o->next_content)
    if (V_OBJ(o) == virtual) return TRUE;
  return FALSE;
}

int count_carrying_obj(struct char_data *ch, int virtual) {
  struct obj_data *o;
  int i=0;
  for (o=ch->carrying;o;o=o->next_content)
    if (V_OBJ(o) == virtual) i++;
  return(i);
}

char
is_wearing_obj (struct char_data *ch, int virtual, int loc) {
  if (EQ(ch,loc) && V_OBJ(EQ(ch, loc)) == virtual) return TRUE;
  return FALSE;
}

void
drain_mana_hit_mv( struct char_data *ch , struct char_data *vict ,
              int mana , int hit , int mv ,
              bool add_m , bool add_hp , bool add_mv ) {
  int MIN( int a , int b );
  int MAX( int a , int b );
  int mana_gained, hp_gained, mv_gained;

  if( mana ) {
    mana_gained = MIN( mana, GET_MANA(vict) );
    GET_MANA( vict ) = MAX( 0 , GET_MANA( vict ) - mana_gained );
    if( add_m )
      GET_MANA( ch ) = MIN( GET_MAX_MANA( ch ) , GET_MANA( ch ) + mana_gained );
  }      /* Linerfix - 062902, mana can't go over max */

  if( hit ) {
    hp_gained = MIN( hit, GET_HIT(vict) );
    GET_HIT( vict ) = MAX( 0 , GET_HIT( vict ) - hp_gained );
    if( add_hp )
      GET_HIT( ch ) = MIN( GET_MAX_HIT( ch ) , GET_HIT( ch ) + hp_gained );
  }      /* Linerfix - 062902, hit can't go over max */

  if( mv ) {
    mv_gained = MIN( mv, GET_MOVE(vict) );
    GET_MOVE( vict ) = MAX( 0 , GET_MOVE( vict ) - mv );
    if( add_mv )
      GET_MOVE( ch ) = MIN( GET_MAX_MOVE( ch ) , GET_MOVE( ch ) + mv_gained );
  }      /* Linerfix - 062902, move can't go over max */
}

struct char_data
*get_ch (int virtual, int scope, int location) {
  struct char_data *tmp, *tmp_next;

  switch (scope) {
  case WORLD:
    for (tmp=character_list;tmp;tmp=tmp_next) {
      if (mob_proto_table[tmp->nr].virtual == virtual)
      return tmp;
      tmp_next = tmp->next;
    }
    break;

  case ZONE:
    for (tmp=character_list;tmp;tmp=tmp_next) {
      if (mob_proto_table[tmp->nr].virtual == virtual &&
        GET_ZONE(tmp) == location)
      return tmp;
      tmp_next = tmp->next;
    }
    break;

  case ROOM:
    for (tmp=world[location].people;tmp;tmp=tmp_next) {
      if (mob_proto_table[tmp->nr].virtual == virtual)
      return tmp;
      tmp_next = tmp->next_in_room;
    }
    break;
  }
  return NULL;
}

struct char_data
*get_ch_world (int virtual) {
  return( get_ch( virtual, WORLD, 0 ) );
}

struct char_data
*get_ch_zone (int virtual, int zone) {
  return( get_ch( virtual, ZONE, zone ) );
}

struct char_data
*get_ch_room (int virtual, int realroom) {
  return( get_ch( virtual, ROOM, realroom ) );
}

int
V_OBJ(struct obj_data *o) {
  if((GET_ITEM_TYPE(o)==ITEM_CONTAINER) && (o->obj_flags.value[3])) /*corpse*/
    return(0);
  if(GET_ITEM_TYPE(o)==ITEM_SKIN)
    return(0);
  if(GET_ITEM_TYPE(o)==ITEM_SCALP)
    return(0);
    return( obj_proto_table[o->item_number].virtual );
}

int
V_MOB(struct char_data *m) {
  return( mob_proto_table[m->nr].virtual );
}

int
V_ROOM(struct char_data *m) {
  return( world[CHAR_REAL_ROOM(m)].number );
}

struct obj_data
*get_obj_room (int virtual, int loc) {
  struct obj_data *tmp, *next;

  if(loc==NOWHERE) return NULL;
  for (tmp=world[real_room(loc)].contents;tmp;tmp=next) {
    if( V_OBJ(tmp) == virtual ) return (tmp);
    next = tmp->next_content;
  }
  return NULL;
}

struct obj_data *get_obj_world(int virtual) {
  struct obj_data *obj=0;
  if(real_object(virtual)>0)
    obj=get_obj_num(real_object(virtual));
  return(obj);
}

int count_mob_followers(struct char_data *ch) {
  int num=0;
  struct follow_type *fol;

  for (fol=ch->followers; fol; fol = fol->next)
    if(IS_NPC(fol->follower)) num++;
  return num;
}

int
count_mortals (struct char_data *ch, byte scope, bool see_invis) {
  struct char_data *v;
  int num=0;

  switch (scope) {
  case ROOM:
    for(v=world[CHAR_REAL_ROOM(ch)].people;v;v=v->next_in_room)
      if (IS_MORTAL(v) && v != ch && (see_invis||CAN_SEE(ch,v)))
      num++;
    return num;
  case ZONE:
    for(v=character_list;v;v=v->next)
      if (IS_MORTAL(v) && v != ch && (see_invis || CAN_SEE(ch, v)) &&
        (GET_ZONE(ch) == GET_ZONE(v)))
      num++;
    return num;
  case WORLD:
    for(v=character_list;v;v=v->next)
      if (IS_MORTAL(v) && v != ch && (see_invis || CAN_SEE(ch, v)))
      num++;
    return num;
  }
  return num;
}

int count_mortals_real_room (int room) {
  struct char_data *v;
  int num=0;
  for(v=world[room].people;v;v=v->next_in_room)
    if (IS_MORTAL(v)) num++;
  return num;
}

int count_mobs_real_room (int room) {
  struct char_data *v;
  int num=0;
  for(v=world[room].people;v;v=v->next_in_room)
    if (IS_NPC(v)) num++;
  return num;
}

int count_mobs_real_room_except_followers(int room) {
  struct char_data *v;
  int num=0;
  for(v=world[room].people;v;v=v->next_in_room)
    if (IS_NPC(v) && !v->master) num++;
  return num;
}

int
count_mortals_room (struct char_data *ch, bool see_invis) {
  return (count_mortals(ch,ROOM,see_invis));
}

int
count_mortals_zone (struct char_data *ch, bool see_invis) {
  return (count_mortals(ch,ZONE,see_invis));
}

int
count_mortals_world (struct char_data *ch, bool see_invis) {
  return (count_mortals (ch, WORLD, see_invis));
}

struct obj_data
*get_random_obj_in_list (struct obj_data *list) {
  struct obj_data *obj, *next;
  int i=0,j=0;
  int number(int mix, int max);

  for (obj = list ; obj ; obj = next ) {
    next = obj->next_content;
    i++;
  }

  if (i==0) return NULL;
  j = number(1,i);
  i=0;

  for (obj = list ; obj ; obj = next ) {
    next = obj->next_content;
    i++;
    if (i==j) return (obj);
  }
  return NULL;
}

struct obj_data
*get_random_obj_room (struct char_data *ch) {
  return (get_random_obj_in_list (world[CHAR_REAL_ROOM(ch)].contents));
}

struct obj_data
*get_random_obj_inv (struct char_data *ch) {
  return (get_random_obj_in_list (ch->carrying));
}

int
get_random_obj_eq (struct char_data *ch) {
  int locat, where[MAX_WEAR];
  int tmp=0;
  int number(int min, int max);

  for (locat=0;locat<MAX_WEAR;locat++) {
    if (EQ(ch,locat)) where[tmp++] = locat;
  }

  if (tmp == 0) return (-1);
  else return (where[number(0,tmp-1)]);
  return FALSE;
}

/* mimics behavior of get_random_target(ch,FALSE,TRUE,FALSE,FALSE,FALSE) */
struct char_data
*get_random_victim( struct char_data *ch ) {
  struct char_data *vict;
  int num=0,tmp=0;
  int number(int min, int max);

  num = count_mortals_room( ch , FALSE );
  if( num == 0 ) return NULL;
  tmp = number (1,num);
  num=0;

  for( vict = world[CHAR_REAL_ROOM(ch)].people ; vict ; vict = vict->next_in_room ) {
    if(IS_MORTAL(vict) && vict != ch && CAN_SEE(ch,vict)) num++;
    if (tmp == num && IS_MORTAL(vict)) return vict;
  }
  return NULL;
}

struct char_data
*get_random_target( struct char_data *ch, bool see_invis, bool vict_canbe_pc, bool vict_canbe_npc, bool vict_canbe_mount, bool vict_canbe_ch )
{
  struct char_data *vict, *temp;
  int num=0,target_num=0;

  /* count # eligible targets in room */
  for(temp = world[CHAR_REAL_ROOM(ch)].people; temp; temp = temp->next_in_room)
  {
    if(vict_canbe_pc && IS_MORTAL(temp) && (see_invis || CAN_SEE(ch,temp)))
    {
      if(vict_canbe_ch || (!vict_canbe_ch && temp!=ch)) num++;
    }
    else if((vict_canbe_mount || vict_canbe_npc) && IS_NPC(temp) && (see_invis || CAN_SEE(ch,temp)))
    {
  	  if(vict_canbe_mount || (!vict_canbe_mount && !IS_MOUNT(temp))) num++;
	}
  }/* end count # eligible targets in room */

  if(num <= 0) return NULL;
  target_num = number(1,num);
  num=0;

  /* cycle back through eligible targets until target_num is reached */
  for( vict = world[CHAR_REAL_ROOM(ch)].people ; vict ; vict = vict->next_in_room )
  {
    if(vict_canbe_pc && IS_MORTAL(temp) && (see_invis || CAN_SEE(ch,temp)))
    {
      if(vict_canbe_ch || (!vict_canbe_ch && temp!=ch)) num++;
    }
    else if((vict_canbe_mount || vict_canbe_npc) && IS_NPC(temp) && (see_invis || CAN_SEE(ch,temp)))
    {
  	  if(vict_canbe_mount || (!vict_canbe_mount && !IS_MOUNT(temp))) num++;
	}
    if (target_num == num) return vict;
  }
  return NULL;
}

int
count_mortals_room_fighting (struct char_data *ch, bool see_invis) {
  struct char_data *v;
  int num=0;

  for(v=world[CHAR_REAL_ROOM(ch)].people;v;v=v->next_in_room)
     if (IS_MORTAL(v) && v != ch && (see_invis||CAN_SEE(ch,v)) &&
     (ch==v->specials.fighting) )
       num++;
  return num;
}

/* mimics behavior of get_random_target_fighting(ch,FALSE,TRUE,FALSE,FALSE) */
struct char_data
*get_random_victim_fighting( struct char_data *ch ) {
  struct char_data *vict;
  int num=0,tmp=0;
  int number(int min, int max);

  num = count_mortals_room_fighting( ch , FALSE );
  if( num == 0 ) return NULL;
  tmp = number (1,num);
  num=0;

  for( vict = world[CHAR_REAL_ROOM(ch)].people ; vict ; vict = vict->next_in_room ) {
    if(IS_MORTAL(vict) && vict != ch && CAN_SEE(ch,vict) &&
       (ch==vict->specials.fighting)) num++;
    if (tmp == num && IS_MORTAL(vict)) return vict;
  }
  return NULL;
}

struct char_data
*get_random_target_fighting( struct char_data *ch, bool see_invis, bool vict_canbe_pc, bool vict_canbe_npc, bool vict_canbe_mount  ) {
  struct char_data *vict, *temp;
  int num=0,target_num=0;

  /* count # eligible targets in room */
  for(temp = world[CHAR_REAL_ROOM(ch)].people; temp; temp = temp->next_in_room)
  {
    if(vict_canbe_pc && IS_MORTAL(temp) && temp->specials.fighting && (temp!=ch) && (see_invis || CAN_SEE(ch,temp)))
      num++;
    else if((vict_canbe_mount || vict_canbe_npc) && IS_NPC(temp) && temp->specials.fighting && (see_invis || CAN_SEE(ch,temp)))
    {
  	  if(vict_canbe_mount || (!vict_canbe_mount && !IS_MOUNT(temp))) num++;
	}
  }/* end count # eligible targets in room */

  if(num <= 0) return NULL;
  target_num = number(1,num);
  num=0;

  /* cycle back through eligible targets until target_num is reached */
  for( vict = world[CHAR_REAL_ROOM(ch)].people ; vict ; vict = vict->next_in_room )
  {
    if(vict_canbe_pc && IS_MORTAL(temp) && temp->specials.fighting && (temp!=ch) && (see_invis || CAN_SEE(ch,temp)))
	  num++;
    else if((vict_canbe_mount || vict_canbe_npc) && IS_NPC(temp) && temp->specials.fighting && (see_invis || CAN_SEE(ch,temp)))
    {
  	  if(vict_canbe_mount || (!vict_canbe_mount && !IS_MOUNT(temp))) num++;
	}
    if (target_num == num) return vict;
  }
  return NULL;
}

void
move_eq_from_to (struct char_data *fch, struct char_data *tch) {
  int i;
  if ((!fch) || (!tch)) return;

  for (i=0;i<MAX_WEAR;i++)
    if (EQ(fch,i)) equip_char(tch, unequip_char(fch,i),i);
}

void
move_inv_from_to (struct char_data *fch, struct char_data *tch) {
  struct obj_data *tmp, *next;
  if ((!fch) || (!tch)) return;
  for (tmp=fch->carrying;tmp;tmp=next) {
    next = tmp->next_content;
    obj_from_char (tmp);
    obj_to_char (tmp, tch);
  }
}

void
set_item_value (struct obj_data *o, int val, int new) {
  o->obj_flags.value[val] = new;
  return;
}

void
move_objs_to_room (int frm, int to) {
  struct obj_data *obj, *tmp;
  for (obj=world[frm].contents;obj;obj=tmp) {
    tmp = obj->next_content;
    obj_from_room(tmp);
    obj_to_room(obj,to);
  }
}

void
move_chars_to_room (int frm, int to) {
  struct char_data *ch, *tmp;
  void do_look(struct char_data *ch, char *arg, int cmd);
  for (ch=world[frm].people;ch;ch=tmp){
    tmp = ch->next_in_room;
    char_from_room(ch);
    char_to_room(ch, to);
    if (IS_MORTAL(ch)) do_look(ch,"",0);
  }
}

char *how_good(int percent)
{
  static char buf[256];

  if (percent == 0)
   strcpy(buf, " (not learned)");
  else if (percent <= 10)
   strcpy(buf, " (awful)");
  else if (percent <= 20)
   strcpy(buf, " (bad)");
  else if (percent <= 40)
   strcpy(buf, " (poor)");
  else if (percent <= 55)
   strcpy(buf, " (average)");
  else if (percent <= 70)
   strcpy(buf, " (fair)");
  else if (percent <= 80)
   strcpy(buf, " (good)");
  else if (percent <= 85)
   strcpy(buf, " (very good)");
  else
   strcpy(buf, " (Superb)");

  return (buf);
}

void check_equipment(struct char_data *ch)
{
   int i;
   for(i=0;i<WIELD;i++) {
     equip_char(ch,unequip_char(ch,i),i);
   }
   equip_char(ch,unequip_char(ch,HOLD),HOLD);
   equip_char(ch,unequip_char(ch,WIELD),WIELD);
   update_pos(ch);
}

void produce_core()
{
   char *NullPointer=NULL;
   *NullPointer = 1;
}

/* assumes j is a container */
void empty_container(OBJ *j) {
  struct obj_data *next_thing, *jj;
  for (jj = j->contains; jj; jj = next_thing) {
    next_thing = jj->next_content; /* Next in inventory */
    obj_from_obj(jj);
    if (j->in_obj)
      obj_to_obj(jj,j->in_obj);
    else if (j->carried_by) {
      if(jj->obj_flags.type_flag == ITEM_MONEY)
        GET_GOLD(j->carried_by) += jj->obj_flags.value[0];
      else obj_to_char(jj,j->carried_by);
    }
    else if (j->in_room != NOWHERE) obj_to_room(jj,j->in_room);
 }
}

void empty_all_containers(CHAR *ch) {
  struct obj_data *obj=NULL,*obj2=NULL,*next_obj;
  int empty=FALSE;
  for(obj = ch->carrying; obj; obj = obj->next_content) {
    if (obj && (obj->obj_flags.type_flag==ITEM_CONTAINER)) {
      for (obj2=obj->contains;obj2;obj2=next_obj) {
        next_obj=obj2->next_content;
        if(obj2) {
          empty=TRUE;
          obj_from_obj(obj2);
          if(obj2->obj_flags.type_flag == ITEM_MONEY)
            GET_GOLD(ch) += obj2->obj_flags.value[0];
          else obj_to_char(obj2, ch);
        }
      }
      if(empty) empty_all_containers(ch);
      return;
    }
  }
}

/* Checks objs for different stats than those in the obj files.
   Ranger - Oct 23/97
*/
int diff_obj_stats(struct obj_data *obj) {
  int nr,i;

  nr=obj->item_number;

  if (nr == -1) return TRUE;

  if(obj->obj_flags.bitvector != obj_proto_table[nr].obj_flags.bitvector)
    return TRUE;
  if(obj->obj_flags.wear_flags  != obj_proto_table[nr].obj_flags.wear_flags)
    return TRUE;

  for( i = 0 ; i < 4; i++) {
    if(obj->obj_flags.value[i] != obj_proto_table[nr].obj_flags.value[i])
      return TRUE;
  }

  for( i = 0 ; (i < MAX_OBJ_AFFECT); i++) {
    if(obj->affected[i].location != obj_proto_table[nr].affected[i].location)
      return TRUE;
    if(obj->affected[i].modifier != obj_proto_table[nr].affected[i].modifier)
      return TRUE;
  }

  if(obj->obj_flags.extra_flags+ITEM_INVISIBLE == obj_proto_table[nr].obj_flags.extra_flags)
    return FALSE;

  if(obj->obj_flags.extra_flags != obj_proto_table[nr].obj_flags.extra_flags)
    return TRUE;

  return FALSE;
}


/* Running external programs - Ranger Feb 99
   based on prgrun.c by Petr Vilim (Petr.Vilim@st.mff.cuni.cz)
   Asynchronyously run unix programs from MUD
   structs in utility.h
*/


struct program_info program_queue[PQUEUE_LENGTH];
int pqueue_counter=0;       /* Number of requests in queue                   */
int pid=-1;                 /* PID of runnig command, -1 if none             */
int to[2], from[2];         /* file descriptors of pipes between processes   */
time_t start_time;          /* Time when process started, for timeout        */
int had_output;             /* Flag if running command already had an output */
FILE *process_in;           /* Standart input for runned program             */

/*
 * get_ch_by_name : given a name, searches every descriptor for a
 *              character with that name and returns a pointer to it.
 */
struct char_data *get_ch_by_name(char *chname)
{
  struct descriptor_data *d;
  extern struct descriptor_data *descriptor_list;

  for (d = descriptor_list; d; d = d->next)
    if (d && !d->connected && d->character &&
        !str_cmp(chname, GET_NAME(d->character)))
      return (d->character);

  return NULL;
}

/* Fork a new process to handle external program execution. */
void program_fork()
{
  int i = 0, flags = 0;

  /* Set up the input/output pipes. */
  pipe(to);
  pipe(from);

  had_output = 0; /* Clear the output status. */
  start_time = time(0); /* Set the start time to 'now'. */

  /* Obtain the PID of the child process. */
  pid = fork();

  /* Check if fork() had a problem. */
  if (pid < 0)
  {
    pid = -1;

    return;
  }

  /* Child Process */
  if (pid == 0)
  {
    /* Set from[1] as standard output from the child process. */
    dup2(from[1], 1);
    close(from[0]);
    close(from[1]);

    /* Set to[0] as standard input from the child process. */
    dup2(to[0], 0);
    close(to[0]);
    close(to[1]);

    /* Close all other unused file descriptors. 1000 should be enough in theory. */
    for (i = 2; i < 1000; i++)
      close(i);

    /* Execute the program. */
    execvp(program_queue[0].args[0], program_queue[0].args);

    /* Terminate the child process. */
    exit(0);
  }
  else
  {
    /* Close unused ends of the pipes. */
    close(from[1]);
    close(to[0]);

    /* Obtain the file descriptor to write program input into. */
    process_in = fdopen(to[1], "w");

    /* Set process_in to be unbuffered. */
    setbuf(process_in, NULL);

    /* Send input to the program (if any). */
    if (program_queue[0].input)
      fprintf(process_in, program_queue[0].input);

    /* Get the file flags of from[0]. */
    flags = fcntl(from[0], F_GETFL);

    /* Set from[0] to non-blocking mode. */
    fcntl(from[0], F_SETFL, flags | O_NONBLOCK);
  }
}

void program_done() {
  int i;

  if (pid!=-1) {
    close(from[0]);         /* Close process standart output */
    fclose(process_in);         /* Close process standart input  */
    waitpid(pid, NULL, 0); /* Wait for process termination  */
    for (i=0; i<MAX_ARGS+1; i++)
      if (program_queue[0].args[i]) free(program_queue[0].args[i]);
      else break;
    free(program_queue[0].name);
    if (program_queue[0].input) free(program_queue[0].input);
    for (i=0; i<pqueue_counter; i++)
      program_queue[i]=program_queue[i+1]; /* shift queue */
    pqueue_counter--;
    if (pqueue_counter)
      program_fork();       /* Start next process */
    else
      pid=-1;
  }
}

void process_program_output() {
  int len;
  struct char_data *ch;
  char *c,*d,buf[MSL],buf1[MSL];

  if (pid==-1)
    return;
  len=read(from[0], buf, MSL-1);
  if ((len==-1) && (errno==EAGAIN)) { /* No datas are available now */
    if (time(0)<start_time+program_queue[0].timeout)
      return;
    else
      sprintf(buf, "`iKilling %s because of timeout.`q\r\n",
            program_queue[0].name);
  } else if (len < 0) {       /* Error with read or timeout */
    sprintf(buf, "`iError with reading from %s, killed.`q",
          program_queue[0].name);
  } else if (len == 0) {  /* EOF readed  */
    if (had_output)
      buf[0]=0;
    else
      sprintf(buf, "`iNo output from %s.`q\r\n", program_queue[0].name);
  };
  ch=get_ch_by_name(program_queue[0].chname);
  if (len<=0) {
    kill(pid, 9);         /* kill process with signal 9 if is still running */
    program_done();
    if ((ch) && (buf[0]))
      send_to_char(buf, ch);
    return;
  }
  if (!ch) return;       /* Player quited the game? */
  had_output=1;
  buf[len]='\0';
  buf[MSL-1]='\0';
  for (c=buf, d=buf1; *c; *d++=*c++)
    if (*c=='\n') *d++='\r';
  *d=0;
  send_to_char("`iOutput from ", ch);
  send_to_char(program_queue[0].name, ch);
  send_to_char(":`q\r\n", ch);
  send_to_char(buf1, ch);
}

/* Following function add program into queue */
void add_program(struct program_info prg, struct char_data *ch)
{
  if (pqueue_counter==PQUEUE_LENGTH) {
    send_to_char("`iSorry, there are too many requests now, try later.`q\r\n", ch);
    return;
  }
  prg.chname=strdup(GET_NAME(ch));
  program_queue[pqueue_counter]=prg;
  pqueue_counter++;
  if (pqueue_counter==1)       /* No process is running now so start new process */
    program_fork();
}

/* Just a list of mob classes with no legs */
int CHAR_HAS_LEGS(struct char_data *ch) {
  switch(GET_CLASS(ch)) {
    case CLASS_LICH:
    case CLASS_LESSER_ELEMENTAL:
    case CLASS_GREATER_ELEMENTAL:
    case CLASS_LESSER_PLANAR:
    case CLASS_GREATER_PLANAR:
    case CLASS_FUNGUS:
    case CLASS_FISH:
    case CLASS_PLANT:
    case CLASS_BLOB:
    case CLASS_GHOST:
    case CLASS_INVERTIBRATE:
      return FALSE;
      break;
  }
  return TRUE;
}

int CORPSE_HAS_SCALP(struct obj_data *obj) {
  switch(obj->obj_flags.material) {
    case CLASS_LICH:
    case CLASS_LESSER_ELEMENTAL:
    case CLASS_GREATER_ELEMENTAL:
    case CLASS_LESSER_PLANAR:
    case CLASS_GREATER_PLANAR:
    case CLASS_FUNGUS:
    case CLASS_FISH:
    case CLASS_PLANT:
    case CLASS_BLOB:
    case CLASS_GHOST:
    case CLASS_INVERTIBRATE:
      return FALSE;
      break;
  }
  return TRUE;
}

int CAN_SEE(struct char_data *ch,struct char_data *vict) {
  if(WIZ_INV(ch,vict)) return FALSE;
  if(IMP_INV(ch,vict) && ch!=vict && !(GET_LEVEL(vict)<=GET_LEVEL(ch) && affected_by_spell(ch, SPELL_PERCEIVE))) return FALSE;
  if(IS_AFFECTED(ch,AFF_BLIND)) return FALSE;
  if(IS_AFFECTED(vict,AFF_INVISIBLE) && !IS_AFFECTED(ch, AFF_DETECT_INVISIBLE) && !affected_by_spell(ch, SPELL_PERCEIVE) && IS_MORTAL(ch)) return FALSE;
  if(!IS_LIGHT(CHAR_REAL_ROOM(ch)) && IS_MORTAL(ch) && !IS_AFFECTED(ch,AFF_INFRAVISION)) return FALSE;
  return TRUE;
}

int CAN_TAKE(struct char_data *ch,struct obj_data *obj) {
  if(!IS_SET(obj->obj_flags.wear_flags, ITEM_TAKE)) return FALSE;
  if(IS_NPC(ch) && IS_SET(obj->obj_flags.extra_flags2, ITEM_NO_TAKE_MOB)) return FALSE;
  return TRUE;
}

struct char_data *get_ch_by_id(int num)
{
  struct descriptor_data *d;
  extern struct descriptor_data *descriptor_list;

  for (d = descriptor_list; d; d = d->next)
    if (d && !d->connected && d->character &&
        d->character->ver3.id==num )
      return (d->character);

  return NULL;
}

int OSTRENGTH_APPLY_INDEX(struct char_data *ch) {
  int index;

  if(GET_OSTR(ch)!=18) index=GET_OSTR(ch);

  if(GET_OSTR(ch)==18) {
    if(GET_OADD(ch)==100) index=30;
    if(GET_OADD(ch)<=99)  index=29;
    if(GET_OADD(ch)<=90)  index=28;
    if(GET_OADD(ch)<=75)  index=27;
    if(GET_OADD(ch)<=50)  index=26;
    if(GET_OADD(ch)==0)   index=18;
  }
  return index;
}

int STRENGTH_APPLY_INDEX(struct char_data *ch) {
  int index;

  if(GET_STR(ch)!=18) index=GET_STR(ch);

  if(GET_STR(ch)==18) {
    if(GET_ADD(ch)==100) index=30;
    if(GET_ADD(ch)<=99)  index=29;
    if(GET_ADD(ch)<=90)  index=28;
    if(GET_ADD(ch)<=75)  index=27;
    if(GET_ADD(ch)<=50)  index=26;
    if(GET_ADD(ch)==0)   index=18;
  }
  return index;
}

int IS_GROUPED(struct char_data *ch,struct char_data *vict) {
  struct char_data *k;
  struct follow_type *f;
  int found_ch=0,found_vict=0;

  if(!IS_AFFECTED(vict, AFF_GROUP)) return FALSE;
  if(!IS_AFFECTED(ch, AFF_GROUP)) return FALSE;

  if(vict->master)
    k=vict->master;
  else
    k=vict;

  if(!IS_AFFECTED(k, AFF_GROUP)) return FALSE;
  if(k==ch) found_ch=TRUE;
  if(k==vict) found_vict=TRUE;

  for(f=k->followers; f; f=f->next) {
     if(f->follower==ch) found_ch=TRUE;
     if(f->follower==vict) found_vict=TRUE;
  }
  if(found_ch && found_vict) return TRUE;
  return FALSE;
}

void log_cmd(char *file,char *fmt, ...)
{
  char buf [2*MSL],filename[20];
  FILE *fl;
  va_list args;

#ifndef LOG_CMD
  return;
#endif
  sprintf(filename,"%s",file);
  if(!(fl=fopen(filename,"a+"))) return;

  va_start (args, fmt);
  vsnprintf (buf, 2*MSL, fmt, args);
  va_end (args);

  fprintf(fl,"%s :: %s\n",filename, buf);
  fclose(fl);
}


void WAIT_STATE(CHAR *ch,int cycle) {
  if(!ch) return;
  if(!ch->desc) return;
  if(cycle < ch->desc->wait) return;
  ch->desc->wait=cycle;
  return;
}

