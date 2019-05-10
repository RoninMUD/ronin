/* ************************************************************************
*  file: utility.c, Utility module.                       Part of DIKUMUD *
*  Usage: Utility procedures                                              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

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
#include <math.h>
#include <stdint.h>
#include <limits.h>
#include <openssl/rand.h>


#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "utility.h"
#include "limits.h"
#include "modify.h"
#include "comm.h"
#include "cmd.h"
#include "spells.h"

extern FILE *logfile;
void update_pos( struct char_data *ch );

/* Begin Fallback RNG Section */

/* Note: The fallback RNG exists for, as the name says, fallback
         situations where the new RNG could fail. This should
         never happen, but it's here just in case. */

#define FALLBACK_RAN_MAX  2147483647L
#define FALLBACK_RAN_MULT 16807

static uint32_t fallback_random_num = 1;

/* Part of the fallback RNG; do not use directly. */
void sfallback_random(uint32_t seed) {
  fallback_random_num = seed ? (seed & FALLBACK_RAN_MAX) : 1;
}

/* Part of the fallback RNG; do not use directly. */
uint32_t fallback_random_next(uint32_t seed) {
  uint32_t lo = FALLBACK_RAN_MULT * (long)(seed & 0xFFFF);
  uint32_t hi = FALLBACK_RAN_MULT * (long)((unsigned long)seed >> 16);

  lo += (hi & 0x7FFF) << 16;

  if (lo > FALLBACK_RAN_MAX) {
    lo &= FALLBACK_RAN_MAX;
    ++lo;
  }

  lo += hi >> 15;

  if (lo > FALLBACK_RAN_MAX) {
    lo &= FALLBACK_RAN_MAX;
    ++lo;
  }

  return lo;
}

/* Part of the fallback RNG; do not use directly. */
uint32_t fallback_random(void) {
  fallback_random_num = fallback_random_next(fallback_random_num);

  return fallback_random_num;
}

/* End Fallback RNG Section */

/* Begin New RNG Section */

/* Generates a random unsigned integer in interval [0, upper_bound] (exclusive).
   Note: Uses OpenSSL (lcrypto).
*/
uint32_t random_uint32_t(uint32_t upper_bound) {
  union {
    uint32_t i;
    unsigned char c[sizeof(uint32_t)];
  } u;

  do {
    if (RAND_bytes(u.c, sizeof(u.c)) == -1) {
      log_s("Failed to get random bytes (random_uint32_t); using fallback RNG.");

      return fallback_random() % upper_bound;
    }
  } while (u.i < (-upper_bound % upper_bound));

  return u.i % upper_bound;
}

/*
Generates a random integer in interval [from, to] (inclusive).

The 'mode' parameter allows for the normal value, minimum value, maximum value,
or the average value to be returned.

Valid modes: RND_NRM, RND_MIN, RND_MAX, RND_AVG
*/
int32_t number_ex(int32_t from, int32_t to, int32_t mode) {
  if (from > to) {
    int32_t temp = from;
    from = to;
    to = temp;
  }

  int32_t result = 0;

  switch (mode) {
    case RND_MIN:
      result = from;
      break;
    case RND_MAX:
      result = to;
      break;
    case RND_AVG:
      result = (from * to) / 2;
      break;
    default:
      result = random_uint32_t((to - from) + 1) + from;
      break;
  }

  return result;
}

/* Generates a random integer in interval [from, to] (inclusive). */
int32_t number(int32_t from, int32_t to) {
  return number_ex(from, to, RND_NRM);
}

/*
Simulates a dice roll.

The 'mode' parameter allows for the normal value, minimum value, maximum value,
or the average value to be returned.

Valid modes: RND_NRM, RND_MIN, RND_MAX, RND_AVG
*/
int32_t dice_ex(int32_t num_dice, int32_t size_dice, int32_t mode) {
  if ((num_dice < 1) || (size_dice < 1)) return 0;

  int64_t result = 0;

  switch (mode) {
    case RND_MIN:
      result = num_dice;
      break;
    case RND_MAX:
      result = num_dice * size_dice;
      break;
    case RND_AVG:
      result = (num_dice * (size_dice + 1)) / 2;
      break;
    default:
      for (int32_t r = 1; r <= num_dice; r++) {
        result += number(1, size_dice);
      }
      break;
  }

  if (result > INT_MAX) {
    result = INT_MAX;
  }

  return (int32_t)result;
}

/* Simulates a dice roll. */
int32_t dice(int32_t num_dice, int32_t size_dice) {
  return dice_ex(num_dice, size_dice, RND_NRM);
}

/* Returns true based on the odds out of 100 of success. */
bool chance(int32_t num) {
  if (number(1, 100) <= num) return TRUE;

  return FALSE;
}

/* Returns the number with the lowest value. */
int32_t MIN(int32_t a, int32_t b) {
  return a < b ? a : b;
}

/* Returns the number with the highest value. */
int32_t MAX(int32_t a, int32_t b) {
  return a > b ? a : b;
}

/* End New RNG Section */


// TODO: Change to not use static char.
char *PERS_ex(CHAR *ch, CHAR *vict, int mode) {
  assert(ch);
  assert(vict);

  static char buf[MIL];

  memset(buf, 0, sizeof(buf));

  if (IS_NPC(ch) && CAN_SEE(vict, ch)) {
    snprintf(buf, sizeof(buf), "%s", MOB_SHORT(ch));
  }
  else if ((IS_MORTAL(ch) && (mode == PERS_MORTAL)) || CAN_SEE(vict, ch)) {
    signal_char(ch, vict, MSG_SHOW_PRETITLE, buf);
    strlcat(buf, GET_NAME(ch), sizeof(buf));
  }
  else {
    snprintf(buf, sizeof(buf), "Somebody");
  }

  return buf;
}

char *PERS(CHAR *ch, CHAR *vict) {
  return PERS_ex(ch, vict, PERS_NORMAL);
}


// TODO: Change to not use static char.
char *POSSESS_ex(CHAR *ch, CHAR *vict, int mode) {
  assert(ch);
  assert(vict);

  static char buf[MIL];

  memset(buf, 0, sizeof(buf));

  if (IS_NPC(ch) && CAN_SEE(vict, ch)) {
    snprintf(buf, sizeof(buf), "%s's", MOB_SHORT(ch));
  }
  else if ((IS_MORTAL(ch) && (mode == PERS_MORTAL)) || CAN_SEE(vict, ch)) {
    signal_char(ch, vict, MSG_SHOW_PRETITLE, buf);
    strlcat(buf, GET_NAME(ch), sizeof(buf));
    strlcat(buf, "'s", sizeof(buf));
  }
  else {
    snprintf(buf, sizeof(buf), "Somebody's");
  }

  return buf;
}

char *POSSESS(CHAR *ch, CHAR *vict) {
  return POSSESS_ex(ch, vict, PERS_NORMAL);
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


/* 50% chance when victim level is the same as the attacker.
   100% chance when victim level is 10 levels or less than the attacker.
   0% chance when victim level is 10 levels or higher than the attacker. */
bool breakthrough(CHAR *ch, CHAR *victim, int skill_spell, int breakthrough_type) {
  if (((breakthrough_type == BT_INVUL) && !IS_AFFECTED(victim, AFF_INVUL)) ||
      ((breakthrough_type == BT_SPHERE) && !IS_AFFECTED(victim, AFF_SPHERE))) {
    return TRUE;
  }

  if (breakthrough_type == BT_INVUL) {
    /* Invulnerability never applies to Hostile victims. */
    if (IS_SET(GET_PFLAG2(victim), PLR2_HOSTILE)) return TRUE;

    /* Cunning */
    if ((IS_MORTAL(ch) && (GET_CLASS(ch) == CLASS_THIEF) && (GET_LEVEL(ch) >= 50)) &&
        ((skill_spell == SKILL_BACKSTAB) || (skill_spell == SKILL_CIRCLE)) &&
        IS_SET(GET_PFLAG2(ch), PLR2_CUNNING) &&
        (GET_MANA(ch) >= 10)) {
      act("$n's weapon flashes with brilliant energy as $e bores through $N's protective shield.", FALSE, ch, 0, victim, TO_NOTVICT);
      act("$n's weapon gleams with azure light as $e pierces through your protective shield.", FALSE, ch, 0, victim, TO_VICT);
      act("Your weapon is briefly sheathed in energy as you slice through $N's protective shield.", FALSE, ch, 0, victim, TO_CHAR);

      GET_MANA(ch) -= 10;

      return TRUE;
    }
  }

  int check = 50 + ((GET_LEVEL(ch) - GET_LEVEL(victim)) * 5);

  switch (GET_CLASS(ch)) {
    case CLASS_CLERIC:
      if (breakthrough_type == BT_INVUL) check -= 10;
      else if (breakthrough_type == BT_SPHERE) check -= 5;
      break;
    case CLASS_MAGIC_USER:
      if (breakthrough_type == BT_INVUL) check -= 10;
      else if (breakthrough_type == BT_SPHERE) check += 10;
      break;
    case CLASS_WARRIOR:
      if (breakthrough_type == BT_INVUL) check += 10;
      else if (breakthrough_type == BT_SPHERE) check -= 10;
      break;
    case CLASS_NOMAD:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check -= 10;
      break;
    case CLASS_THIEF:
      if (breakthrough_type == BT_INVUL) check += 10;
      else if (breakthrough_type == BT_SPHERE) check -= 10;
      break;
    case CLASS_NINJA:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check -= 5;
      break;
    case CLASS_ANTI_PALADIN:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check += 5;
      break;
    case CLASS_PALADIN:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check -= 5;
      break;
    case CLASS_BARD:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check += 5;
      break;
    case CLASS_COMMANDO:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check += 5;
      break;
  }
  
  return (number(1, 100) <= check);
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
  return !IS_LIGHT(room);
}

int IS_LIGHT(int room) {
  if (world[room].light ||
      world[room].zone == 30 ||
      (!IS_SET(world[room].room_flags, DARK) &&
       (IS_SET(world[room].room_flags, INDOORS) ||
        IS_SET(world[room].room_flags, LIT) ||
        world[room].sector_type == SECT_INSIDE ||
        weather_info.sunlight == SUN_RISE ||
        weather_info.sunlight == SUN_LIGHT))) {
    return TRUE;
  }

  return FALSE;
}

char *string_to_lower(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    string[i] = LOWER(string[i]);
  }

  return string;
}

char *string_to_upper(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    string[i] = UPPER(string[i]);
  }

  return string;
}

size_t strlmrg(char *dest, size_t size, ...) {
  char *s, *end = dest + (size - 1);
  size_t needed = 0;

  va_list ap;

  va_start(ap, size);

  while ((s = va_arg(ap, char *))) {
    if (s == dest) {
      size_t n = strnlen(s, (end + 1) - s);
      needed += n;
      dest += n;
    }
    else {
      needed += strlen(s);

      if (dest && (dest < end)) {
        while (*s && (dest < end)) {
          *dest++ = *s++;
        }

        *dest = 0;
      }
    }
  }

  va_end(ap);

  return needed;
}

size_t strlcpy(char *dest, const char *src, size_t size) {
  return strlmrg(dest, size, src, (void *)0);
}

size_t strlcat(char *dest, const char *src, size_t size) {
  return strlmrg(dest, size, dest, src, (void *)0);
}

int IS_CARRYING_W(struct char_data *ch)
{
  struct obj_data *tmp;
  int    tmp_weight=0;
  for(tmp=ch->carrying;tmp;tmp=tmp->next_content)
  {
    tmp_weight += GETOBJ_WEIGHT(tmp);
  }

  return tmp_weight;
}

int IS_CARRYING_N(struct char_data *ch)
{
  struct obj_data *tmp;
  int    i=0;

  for(tmp=ch->carrying;tmp;tmp=tmp->next_content)
  {
    i++;
  }

  return i;
}

char *str_cut(char *source,char *dest,int number) {
  int y;
  char buf[MAX_INPUT_LENGTH];

  if(strlen(source)>MAX_INPUT_LENGTH ||
    strlen(source)<number) return source;
  strncpy(buf,source,sizeof(buf));
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
  {
    i++;
  }
  return i;
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

/* Create a duplicate of a string */
char *str_dup(char *source)
{
  char *new = NULL;
  size_t len = strnlen(source, MAX_STRING_LENGTH);

  if (!source) source = "";

  CREATE(new, char, len + 1);

  return strncpy(new, source, len);
}

int str_cat(char *s, int len, int maxlen, const char *append)
{
  int i = 0;

  assert(len <= maxlen);

  for ( i = 0; len + i < maxlen - 1 && *(append + i) != '\0'; i++)
  {
    *(s + len + i) = *(append + i);
  }

  *(s + len + i) = '\0';

  if(*(append + i) != '\0')
  {
    log_f("BUG: too long append string in str_cat");
  }

  return len + i;
}


/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
  int chk = 0, i = 0;

  if (!arg1)
    return -1;
  if (!arg2)
    return 1;

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
  {
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
    {
      if (chk < 0)
        return -1;
      else
        return 1;
    }
  }

  return 0;
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
  int chk = 0, i = 0;

  if (!arg1)
    return -1;
  if (!arg2)
    return 1;

  for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n>0); i++, n--)
  {
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
    {
      if (chk < 0)
        return -1;
      else
        return 1;
    }
    }

  return 0;
}

/* changes the supplied string buffer to contain all uppercase chars */
char *str_upper(char *str)
{
  char *p;

  p = str;

  while (*p)
  {
    *p = toupper(*p);
    p++;
  }

  return str;
}


/* writes a string to the log */

void log_s(char *str) {
  if (logfile == NULL) {
    puts("SYSERR: Using log_s() before stream was initialized!");

    return;
  }

  time_t ct = time(0);
  char *tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  fprintf(logfile, "%s :: %s\n", tmstr, str);

  fflush(logfile);
}

void log_f(char *fmt, ...) {
  va_list args;
  char buf[2 * MSL];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  log_s(buf);
}

void deathlog(char *str) {
  FILE *fl;

  if (!(fl = fopen("death.log", "a"))) {
    log_f("WARNING: Unable to open death.log file.");

    return;
  }

  time_t ct = time(0);
  char *tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  fprintf(fl, "%s :: %s\n", tmstr, str);

  fclose(fl);
}

void deathlog_f(char *fmt, ...) {
  va_list args;
  char buf[2 * MSL];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  deathlog(buf);
}

void wizinfo(char *str, int level) {
  char buf[2 * MSL];

  snprintf(buf, sizeof(buf), "** %s **\n\r", str);

  for (DESC *desc = descriptor_list; desc; desc = desc->next) {
    if ((desc->connected == CON_PLYNG) && (desc->wizinfo != 0) && (desc->wizinfo >= level) && IS_IMMORTAL(desc->character)) {
      send_to_char(buf, desc->character);
    }
  }
}

void wizinfo_f(int level, char *fmt, ...) {
  va_list args;
  char buf[2 * MSL];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  wizinfo(buf, level);
}

void wizlog(char *str, int level, int which) {
  level = MIN(LEVEL_IMP, level);
  which = (((which < 1) || (which > 7)) ? 6 : which);

  char buf[2 * MSL];

  snprintf(buf, sizeof(buf), "[ %s ]\n\r", str);

  for (DESC *desc = descriptor_list; desc; desc = desc->next) {
    if ((desc->connected == CON_PLYNG) && (desc->wizinfo != 0) && (desc->wizinfo >= level) && IS_IMMORTAL(desc->character)) {
      if (((which == 1) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_ONE)) ||
          ((which == 2) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_TWO)) ||
          ((which == 3) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_THREE)) ||
          ((which == 4) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_FOUR)) ||
          ((which == 5) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_FIVE)) ||
          ((which == 6) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_SIX)) ||
          ((which == 7) && IS_SET(GET_IMM_FLAGS(desc->character), QUEST_INFO))) {
        send_to_char(buf, desc->character);
      }
    }
  }
}

void wizlog_f(int level, int which, char *fmt, ...) {
  va_list args;
  char buf[2 * MSL];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  wizlog(buf, level, which);
}


void sprintbit(long vektor, const char * const names[], char *result)
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

void sprinttype(int type, const char * const names[], char *result)
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

void drain_mana_hit_mv(struct char_data *ch, struct char_data *vict, int mana, int hit, int mv, bool add_m, bool add_hp, bool add_mv) {
  int MIN(int a, int b);
  int MAX(int a, int b);
  int damage(struct char_data *ch, struct char_data *victim, int dmg, int attack_type, int damage_type);

  int mana_gained, hp_gained, mv_gained;

  if (mana) {
    mana_gained = MIN(mana, GET_MANA(vict));

    GET_MANA(vict) = MAX(0, GET_MANA(vict) - mana);

    if (add_m) {
      GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch) + mana_gained);
    }
  }

  if (hit) {
    hp_gained = damage(ch, vict, hit, TYPE_UNDEFINED, DAM_MAGICAL);

    if (add_hp) {
      GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) + hp_gained);
    }
  }

  if (mv) {
    mv_gained = MIN(mv, GET_MOVE(vict));

    GET_MOVE(vict) = MAX(0, GET_MOVE(vict) - mv);

    if (add_mv) {
      GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch), GET_MOVE(ch) + mv_gained);
    }
  }
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

int V_OBJ(struct obj_data *obj)
{
  if (((GET_ITEM_TYPE(obj) == ITEM_CONTAINER) && OBJ_VALUE3(obj)) ||
      (GET_ITEM_TYPE(obj) == ITEM_SKIN) ||
      (GET_ITEM_TYPE(obj) == ITEM_TROPHY))
  {
    return 0;
  }

  return obj_proto_table[obj->item_number].virtual;
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


struct char_data * get_random_target(struct char_data *ch, bool see_invis, bool vict_canbe_pc, bool vict_canbe_npc, bool vict_canbe_mount, bool vict_canbe_ch, bool see_imm) {
  struct char_data *vict = NULL;
  int num=0,target_num=0;

  /* count # eligible targets in room */
  for(vict = world[CHAR_REAL_ROOM(ch)].people; vict; vict = vict->next_in_room) {
    if (ch == vict) {
      /* can see yourself */
      if (vict_canbe_ch) num++;
    }
    else if (see_invis || CAN_SEE(ch,vict)) {
      if (IS_MORTAL(vict) || (see_imm && IS_IMMORTAL(vict))) {
        if (vict_canbe_pc) num++;
      }
      else if (IS_NPC(vict)) {
        if (IS_MOUNT(vict)) {
          if (vict_canbe_mount) num++;
        }
        else if (vict_canbe_npc) num++;
      }
    }
  } /* end count # eligible targets in room */

  if(num <= 0) return NULL;

  target_num = number(1,num);
  num=0;

  /* cycle back through eligible targets until target_num is reached */
  for(vict = world[CHAR_REAL_ROOM(ch)].people; vict; vict = vict->next_in_room) {
    if (ch == vict) {
      /* can see yourself */
      if (vict_canbe_ch) num++;
    }
    else if (see_invis || CAN_SEE(ch,vict)) {
      if (IS_MORTAL(vict) || (see_imm && IS_IMMORTAL(vict))) {
        if (vict_canbe_pc) num++;
      }
      else if (IS_NPC(vict)) {
        if (IS_MOUNT(vict)) {
          if (vict_canbe_mount) num++;
        }
        else if (vict_canbe_npc) num++;
      }
    }

    if (target_num == num) return vict;
  }

  return NULL;
}

/* shortcut for get_random_target(ch,FALSE,TRUE,FALSE,TRUE,FALSE) */
struct char_data * get_random_victim(struct char_data *ch) {
  return get_random_target(ch, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE);
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

struct char_data * get_random_target_fighting(struct char_data *ch, bool see_invis, bool vict_canbe_pc, bool vict_canbe_npc, bool vict_canbe_mount, bool see_imm) {
  struct char_data *vict = NULL;
  int num=0,target_num=0;

  /* count # eligible targets in room */
  for(vict = world[CHAR_REAL_ROOM(ch)].people; vict; vict = vict->next_in_room) {
    if ((ch == vict) || (ch != vict->specials.fighting)) {
      continue;
    }
    else if (see_invis || CAN_SEE(ch,vict)) {
      if (IS_MORTAL(vict) || (see_imm && IS_IMMORTAL(vict))) {
        if (vict_canbe_pc) num++;
      }
      else if (IS_NPC(vict)) {
        if (IS_MOUNT(vict)) {
          if (vict_canbe_mount) num++;
        }
        else if (vict_canbe_npc) num++;
      }
    }
  } /* end count # eligible targets in room */

  if(num <= 0) return NULL;

  target_num = number(1,num);
  num=0;

  /* cycle back through eligible targets until target_num is reached */
  for(vict = world[CHAR_REAL_ROOM(ch)].people; vict; vict = vict->next_in_room) {
    if ((ch == vict) || (ch != vict->specials.fighting)) {
      continue;
    }
    else if (see_invis || CAN_SEE(ch,vict)) {
      if (IS_MORTAL(vict) || (see_imm && IS_IMMORTAL(vict))) {
        if (vict_canbe_pc) num++;
      }
      else if (IS_NPC(vict)) {
        if (IS_MOUNT(vict)) {
          if (vict_canbe_mount) num++;
        }
        else if (vict_canbe_npc) num++;
      }
    }

    if (target_num == num) return vict;
  }

  return NULL;
}

/* shortcut to  get_random_target_fighting(ch,FALSE,TRUE,FALSE,TRUE) */
struct char_data * get_random_victim_fighting( struct char_data *ch ) {
  return get_random_target_fighting(ch, FALSE, TRUE, FALSE, TRUE, FALSE);
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
   strcpy(buf, "(not learned)");
  else if (percent <= 10)
   strcpy(buf, "(awful)");
  else if (percent <= 20)
   strcpy(buf, "(bad)");
  else if (percent <= 40)
   strcpy(buf, "(poor)");
  else if (percent <= 55)
   strcpy(buf, "(average)");
  else if (percent <= 70)
   strcpy(buf, "(fair)");
  else if (percent <= 80)
   strcpy(buf, "(good)");
  else if (percent <= 85)
   strcpy(buf, "(very good)");
  else
   strcpy(buf, "(Superb)");

  return (buf);
}

void check_equipment(struct char_data *ch)
{
   int i = 0;

   for (i = 0; i < WIELD; i++)
   {
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
      fputs(program_queue[0].input, process_in);

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

bool HAS_BOAT(CHAR *ch) {
  bool has_boat = FALSE;

  /* If they're flying, or they're a ninja, they are considered to have a boat. */
  if (IS_AFFECTED(ch, AFF_FLY) || (GET_CLASS(ch) == CLASS_NINJA)) has_boat = TRUE;

  /* Check if they are carrying a boat. */
  for (OBJ *tmp_obj = ch->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
    if (OBJ_TYPE(tmp_obj) == ITEM_BOAT) {
      has_boat = TRUE;
      break;
    }
  }

  /* Check if they are wearing a boat object (e.g. Boots of Water Walking). */
  for (int i = 0; i < MAX_WEAR; i++) {
    if (EQ(ch, i) && OBJ_TYPE(EQ(ch, i)) == ITEM_BOAT) {
      has_boat = TRUE;
      break;
    }
  }

  return has_boat;
}

int CHAR_HAS_LEGS(CHAR *ch) {
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

int CORPSE_HAS_TROPHY(OBJ *obj) {
  switch(OBJ_MATERIAL(obj)) {
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

int CAN_SEE(CHAR *ch, CHAR *vict) {
  if (!ch || !vict) return FALSE;

  if (WIZ_INV(ch, vict) ||
      IMP_INV(ch, vict) ||
      NRM_INV(ch, vict) ||
      (!IS_IMMORTAL(ch) && IS_AFFECTED(ch, AFF_BLIND)) ||
      (IS_MORTAL(ch) && !IS_LIGHT(CHAR_REAL_ROOM(ch)) && !IS_AFFECTED(ch, AFF_INFRAVISION))) {
    return FALSE;
  }

  return TRUE;
}

int CAN_TAKE(CHAR *ch, OBJ *obj) {
  if (!ch || !obj) return FALSE;

  if (!IS_SET(OBJ_WEAR_FLAGS(obj), ITEM_TAKE) ||
      (IS_NPC(ch) && IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_NO_TAKE_MOB))) {
    return FALSE;
  }

  return TRUE;
}

CHAR *get_ch_by_id(int num) {
  for (struct descriptor_data *d = descriptor_list; d; d = d->next)
    if (d && !d->connected && d->character && (d->character->ver3.id == num))
      return d->character;

  return NULL;
}

int OSTRENGTH_APPLY_INDEX(struct char_data *ch) {
  if (!ch) return 0;

  if (GET_OSTR(ch) < 0 || GET_OSTR(ch) > 25) return 0;

  int index = GET_OSTR(ch);

  if (GET_OSTR(ch) == 18) {
    if (GET_OADD(ch) < 1)        index = 18;
    else if (GET_OADD(ch) < 50)  index = 26;
    else if (GET_OADD(ch) < 75)  index = 27;
    else if (GET_OADD(ch) < 90)  index = 28;
    else if (GET_OADD(ch) < 100) index = 29;
    else                         index = 30;
  }

  return index;
}

int STRENGTH_APPLY_INDEX(struct char_data *ch) {
  if (!ch) return 0;

  if (GET_STR(ch) < 0 || GET_STR(ch) > 25) return 0;

  int index = GET_STR(ch);

  if (GET_STR(ch) == 18) {
    if (GET_ADD(ch) < 1)        index = 18;
    else if (GET_ADD(ch) < 50)  index = 26;
    else if (GET_ADD(ch) < 75)  index = 27;
    else if (GET_ADD(ch) < 90)  index = 28;
    else if (GET_ADD(ch) < 100) index = 29;
    else                        index = 30;
  }

  return index;
}


bool SAME_GROUP(CHAR *ch1, CHAR *ch2) {
  if (!ch1 || !ch2) return FALSE;

  if (!IS_AFFECTED(ch1, AFF_GROUP) || !IS_AFFECTED(ch2, AFF_GROUP)) return FALSE;

  CHAR *group_leader = (GET_MASTER(ch1) ? GET_MASTER(ch1) : ch1);

  if (!IS_AFFECTED(group_leader, AFF_GROUP)) return FALSE;

  bool found_ch1 = FALSE, found_ch2 = FALSE;

  if (group_leader == ch1) found_ch1 = TRUE;
  if (group_leader == ch2) found_ch2 = TRUE;

  for (FOL *follower = group_leader->followers; follower && (!found_ch1 || !found_ch2); follower = follower->next) {
    CHAR *group_member = follower->follower;

    if (group_member == ch1) found_ch1 = TRUE;
    if (group_member == ch2) found_ch2 = TRUE;
  }

  return (found_ch1 && found_ch2);
}


void log_cmd(char *file, char *fmt, ...)
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


void WAIT_STATE(CHAR *ch, int cycle) {
  if (!ch || !GET_DESCRIPTOR(ch)) return;

  if (cycle >= GET_WAIT(ch)) {
    GET_WAIT(ch) = cycle;
  }
}

int get_weapon_type(OBJ *obj) {
  int w_type = TYPE_HIT;

  switch (OBJ_VALUE3(obj)) {
    case 0:
    case 1:
    case 2: w_type = TYPE_WHIP;     break;
    case 3: w_type = TYPE_SLASH;    break;
    case 4: w_type = TYPE_WHIP;     break;
    case 5: w_type = TYPE_STING;    break;
    case 6: w_type = TYPE_CRUSH;    break;
    case 7: w_type = TYPE_BLUDGEON; break;
    case 8: w_type = TYPE_CLAW;     break;
    case 9:
    case 10:
    case 11: w_type = TYPE_PIERCE;  break;
    case 12: w_type = TYPE_HACK;    break;
    case 13: w_type = TYPE_CHOP;    break;
    case 14: w_type = TYPE_SLICE;   break;
    default: w_type = TYPE_HIT;     break;
  }

  return w_type;
}

const char  *
get_weapon_type_desc(OBJ *obj) {
  char const * result = NULL;

  switch (get_weapon_type(obj)) {
    case TYPE_WHIP:     result = "Whip";     break;
    case TYPE_SLASH:    result = "Slash";    break;
    case TYPE_STING:    result = "Sting";    break;
    case TYPE_CRUSH:    result = "Crush";    break;
    case TYPE_BLUDGEON: result = "Bludgeon"; break;
    case TYPE_CLAW:     result = "Claw";     break;
    case TYPE_PIERCE:   result = "Pierce";   break;
    case TYPE_HACK:     result = "Hack";     break;
    case TYPE_CHOP:     result = "Chop";     break;
    case TYPE_SLICE:    result = "Slice";    break;
    default:            result = "Hit";      break;
  }

  return result;
}


int count_attackers(CHAR *ch) {
  int count = 0;

  for (CHAR *attacker = world[CHAR_REAL_ROOM(ch)].people; attacker; attacker = attacker->next_in_room) {
    if (GET_OPPONENT(attacker) == ch) {
      count++;
    }
  }

  return count;
}


int qcmp_int(const void *a, const void *b) {
  return *(int *)a - *(int *)b;
}

int qcmp_int_asc(const void *a, const void *b) {
  return qcmp_int(a, b);
}

int qcmp_int_desc(const void *a, const void *b) {
  return qcmp_int(b, a);
}


/* Takes a character object and an array of eligible effects and returns
   an effect randomly from the array if the character is not affected by it.
   If there are no eligible effects found, this returns TYPE_UNDEFINED.
   Note: You MUST terminate the array with -1 or bad things will happen. */
int get_random_eligible_effect(CHAR *ch, const int eligible_effects[]) {
  /* Count the number of elements in the list of eligible effects. */
  int list_size = 0;
  for (int i = 0; eligible_effects[i] != -1; list_size++, i++);

  if (!list_size) return TYPE_UNDEFINED;

  int eligible_ch_effects[MAX_SPL_LIST];
  memset(&eligible_ch_effects, 0, sizeof(eligible_ch_effects));

  int num_eligible_ch_effects = 0;
  for (int i = 0; i < list_size; i++) {
    if (!affected_by_spell(ch, eligible_effects[i])) {
      num_eligible_ch_effects++;
      eligible_ch_effects[num_eligible_ch_effects - 1] = eligible_effects[i];
    }
  }

  if (num_eligible_ch_effects > 0) {
    return eligible_ch_effects[number(1, num_eligible_ch_effects) - 1];
  }

  return TYPE_UNDEFINED;
}


/* Takes a character object and an array of eligible effects and returns
   an effect randomly from the array if the character is affected by it.
   If there are no eligible effects found, this returns TYPE_UNDEFINED.
   Note: You MUST terminate the array with -1 or bad things will happen. */
int get_random_set_effect(CHAR *ch, const int eligible_effects[]) {
  AFF *af = NULL;
  int i = 0;
  int list_size = 0;
  int eligible_ch_effects[MAX_SPL_LIST];
  int num_eligible_ch_effects = 0;

  memset(&eligible_ch_effects, 0, sizeof(eligible_ch_effects));

  /* Count the number of elements in the list of eligible effects. */
  for (list_size = 0, i = 0; eligible_effects[i] != -1; list_size++, i++);

  if (!list_size) return TYPE_UNDEFINED;

  for (af = ch->affected; af; af = af->next) {
    for (i = 0; i < list_size; i++) {
      if (af->type == eligible_effects[i]) {
        num_eligible_ch_effects++;
        eligible_ch_effects[num_eligible_ch_effects - 1] = af->type;
      }
    }
  }

  if (num_eligible_ch_effects > 0) {
    return eligible_ch_effects[number(1, num_eligible_ch_effects) - 1];
  }

  return TYPE_UNDEFINED;
}


/* Takes a bit mask and and returns one of the set bits randomly,
   or zero if no bits were set. */
int get_random_set_bit_from_mask_t(const int32_t mask) {
  const int32_t mask_size = (sizeof(int32_t) * CHAR_BIT);

  int32_t i = 0;
  int32_t flag = 0;
  int32_t eligible_bits[mask_size];
  int32_t num_eligible_bits = 0;

  if (mask == 0) return 0;

  for (i = 0; i < mask_size; i++) {
    flag = (1 << i);
    if (IS_SET(mask, flag)) {
      eligible_bits[num_eligible_bits] = flag;
      num_eligible_bits++;
    }
  }

  if (num_eligible_bits > 0) {
    return eligible_bits[number(0, num_eligible_bits - 1)];
  }

  return 0;
}


bool in_int_array(int value, int *array, size_t num_elems) {
  if ((num_elems > 1) && (num_elems < UINT_MAX)) {
    for (size_t i = 0; i < (num_elems - 1); i++) {
      if (array[i] == value) {
        return TRUE;
      }
    }
  }

  return FALSE;
}


/* Shuffle an array of integers. */
void shuffle_int_array(int *array, size_t num_elems) {
  if ((num_elems > 1) && (num_elems < UINT_MAX)) {
    for (size_t i = 0; i < (num_elems - 1); i++) {
      size_t j = i + number(0, (num_elems - i) - 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}


/* Shuffle a 2D array of integers. */
void shuffle_2d_int_array(int (*array)[2], size_t num_elems) {
  if ((num_elems > 1) && (num_elems < UINT_MAX)) {
    for (size_t i = 0; i < num_elems - 1; i++) {
      size_t j = i + number(0, (num_elems - i) - 1);
      int t0 = array[j][0];
      int t1 = array[j][1];
      array[j][0] = array[i][0];
      array[j][1] = array[i][1];
      array[i][0] = t0;
      array[i][1] = t1;
    }
  }
}


int MAX_PRAC(CHAR *ch) {
  if (!ch || IS_NPC(ch) || !(ch->skills)) return 0;

  switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
      return 95;
    case CLASS_AVATAR:
      return 100;
    default:
      return 85;
  }

  return 0;
}
