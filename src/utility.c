/**
 * @file utility.c
*/

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
#include <sodium.h>


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

void update_pos(CHAR *ch);

/* RNG and Other Math Functions */

/**
 * @brief Generates a number in the range [from, to] inclusive, based on the
 *   chosen mode.
 *
 * Modes:
 * - RND_RND (0): A random number in the range [from, to].
 * - RND_MIN (1): The minimum possible number (from).
 * - RND_MAX (2): The maximum possible number (to).
 * - RND_AVG (3): The average (mean) of the range [from, to].
 *
 * @param[in] from The minimum possible number.
 * @param[in] to The maximum possible number.
 * @param[in] mode The mode used to determine the generated number.
 *
 * @return The generated number.
 */
int32_t number_ex(int32_t from, int32_t to, int mode) {
  int32_t result = 0, mod = 0, temp;

  /* Swap from and to if from is greater than to. */
  if (from > to) {
    temp = from;
    from = to;
    to = temp;
  }

  /* If from < 0, shift the range up, since randombytes_uniform() requires an upper bound >= 0. */
  if (from < 0) {
    mod = abs(from);

    from += mod;

    /* Overflow protection. */
    if (INT_MAX - mod < to) {
      to = INT_MAX;
    }
    else {
      to += mod;
    }
  }

  /* Generate a result based on the chosen mode. */
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
    case RND_RND:
    default:
      result = randombytes_uniform((to - from) + 1) + from;
      break;
  }

  /* Shift range down again, as needed. */
  if (mod > 0) {
    result -= mod;
  }

  return result;
}

/**
 * @brief Generates a number in the range [from, to] inclusive.
 *
 * @param[in] from The minimum possible number.
 * @param[in] to The maximum possible number.
 *
 * @return The generated number.
 */
int32_t number(int32_t from, int32_t to) {
  return number_ex(from, to, RND_RND);
}

/**
 * @brief Simulates rolling dice, generating a number in the range
 *   [num_dice, (num_dice * size_dice)], based on the chosen mode.
 *
 * Modes:
 * - RND_RND (0): A random number in the range
 *   [num_dice, (num_dice * size_dice)].
 * - RND_MIN (1): The minimum possible number (num_dice).
 * - RND_MAX (2): The maximum possible number (num_dice * size_dice).
 * - RND_AVG (3): The average (mean) of the range
 *   [num_dice, (num_dice * size_dice)].
 *
 * @param[in] num_dice The number of dice to roll.
 * @param[in] size_dice The size of the dice (e.g. the number of faces).
 * @param[in] mode The mode used to determine the generated number.
 *
 * @return The generated number.
 */
int32_t dice_ex(int32_t num_dice, int32_t size_dice, int mode) {
  /* Use int64_t to prevent integer overflow during arithmetic. */
  int64_t result = 0, roll;

  /* Return 0 if either num_dice or size_dice are less than 1. */
  if ((num_dice < 1) || (size_dice < 1)) return 0;

  /* Generate a result based on the chosen mode. */
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
    case RND_RND:
    default:
      for (roll = 1; roll <= num_dice; roll++) {
        result += number(1, size_dice);
      }
      break;
  }

  /* Ensure that result does not exceed INT_MAX. */
  if (result > INT_MAX) {
    result = INT_MAX;
  }

  /* Coerce result to int32_t. */
  return (int32_t)result;
}

/**
 * @brief Simulates rolling dice, generating a number in the range
 *   [num_dice, (num_dice * size_dice)].
 *
 * @param[in] num_dice The number of dice to roll.
 * @param[in] size_dice The size of the dice (e.g. the number of faces).
 *
 * @return The generated number.
 */
int32_t dice(int32_t num_dice, int32_t size_dice) {
  return dice_ex(num_dice, size_dice, RND_RND);
}

/**
 * @brief Determines success or failure based on the provided percent chance.
 *
 * @param[in] percent The percent chance of success.
 *
 * @return TRUE if success and FALSE if failure.
 */
bool chance(int32_t percent) {
  return (number(1, 100) <= percent);
}

/**
 * @brief Provided integers a and b, returns the integer with the lesser value.
 *
 * @param[in] a The first integer to compare.
 * @param[in] b The second integer to compare.
 *
 * @return The integer with the lesser value.
 */
int32_t MIN(int32_t a, int32_t b) {
  return a < b ? a : b;
}

/**
 * @brief Provided integers a and b, returns the integer with the greater value.
 *
 * @param[in] a The first integer to compare.
 * @param[in] b The second integer to compare.
 *
 * @return The integer with the greater value.
 */
int32_t MAX(int32_t a, int32_t b) {
  return a > b ? a : b;
}


/* String Manipulation Functions */

/**
 * @brief Merges a list of strings.
 *
 * @param[in,out] dest The destination string.
 * @param[in] dest_size The size allocated for the destination string.
 * @param[in] ... List of strings to merge. The final pointer MUST be NULL.
 *
 * @return The length of the merged strings.
 */
size_t str_mrg(char *dest, size_t dest_size, ...) {
  size_t len = 0, str_len;
  va_list str_list;
  char *str;

  va_start(str_list, dest_size);

  /* Copy each string from str_list to dest. */
  while (((str = va_arg(str_list, char *)) != NULL) && (len < dest_size - 1)) {
    str_len = strlen(str);

    /* Copy the current string to dest. */
    if (str != dest) {
      if (str_len > dest_size - 1) {
        str_len = dest_size - 1;
      }

      strncpy(dest, str, str_len);
    }

    dest += str_len;
    len += str_len;
  }

  va_end(str_list);

  /* Null-terminate dest. */
  *dest++ = '\0';

  return len;
}

/**
 * @brief Copies a source string to a destination string.
 *
 * @param[in,out] dest The destination string.
 * @param[in] dest_size The size allocated for the destination string.
 * @param[in] src The source string.
 *
 * @return The length of the copied string.
 */
size_t str_cpy(char *dest, size_t dest_size, const char *src) {
  return str_mrg(dest, dest_size, src, NULL);
}

/**
 * @brief Appends a copy of the source string to the destination string.
 *
 * @param[in,out] dest The destination string.
 * @param[in] dest_size The size allocated to the destination string.
 * @param[in] src The source string.
 *
 * @return The length of the concatenated strings.
 */
size_t str_cat(char *dest, size_t dest_size, const char *src) {
  return str_mrg(dest, dest_size, dest, src, NULL);
}

/**
 * @brief Gets a substring of a string that begins from a specified position
 *   and has a specified length.
 *
 * @param[in,out] dest The destination string.
 * @param[in] dest_size The size allocated for the destination string.
 * @param[in] src The source string.
 * @param[in] start_idx Index of the source string to start from.
 * @param[in] n The length of the substring.
 *
 * @return The substring.
 */
char *str_sub(char *dest, size_t dest_size, const char *src, size_t start_idx, size_t n) {
  size_t str_len = strlen(src);

  /* Ensure start_idx does not exceed str_len. */
  if (start_idx > str_len) {
    start_idx = str_len;
  }

  /* Ensure n does not exceed (str_len - start_idx). */
  if (n > (str_len - start_idx)) {
    n = str_len - start_idx;
  }

  /* Ensure n does not exceed dest_size - 1. */
  if (n > dest_size - 1) {
    n = dest_size - 1;
  }

  /* Increment str pointer by start_idx. */
  src += start_idx;

  /* Copy n characters from src to sub. */
  strncpy(dest, src, n);

  /* Null-terminate sub. */
  dest[n++] = '\0';

  return dest;
}

/**
 * @brief Gets the specified number of characters from the beginning of a string.
 *
 * @param[in,out] dest The destination string.
 * @param[in] dest_size The size allocated for the destination string.
 * @param[in] src The source string.
 * @param[in] n The number of characters to get.
 *
 * @return The head of the string.
 */
char *str_head(char *dest, size_t dest_size, const char *src, size_t n) {
  return str_sub(dest, dest_size, src, 0, n);
}

/**
 * @brief Gets the specified number of characters from the end of a string.
 *
 * @param[in,out] dest The destination string.
 * @param[in] dest_size The size allocated for the destination string.
 * @param[in] src The source string.
 * @param[in] n The number of characters to get.
 *
 * @return The tail of the string.
 */
char *str_tail(char *dest, size_t dest_size, const char *src, size_t n) {
  return str_sub(dest, dest_size, src, strlen(src) - n, n);
}

/**
 * @brief Parses a provided string and extracts the substring between the first
 *   and second delimiter.
 *
 * @param[in,out] dest The destination string.
 * @param[in] dest_size The size allocated for the destination string.
 * @param[in] src The source string.
 * @param[in] delim1 The first delimiter.
 * @param[in] delim2 The second delimiter.
 *
 * @return The length of the substring.
 */
size_t str_sub_delim(char *dest, size_t dest_size, const char *src, int delim1, int delim2) {
  size_t sub_len = 0;

  if (src) {
    char *d1 = strchr(src, delim1);
    char *d2 = strrchr(src, delim2);

    int d1_idx = d1 ? d1 - src : -1;
    int d2_idx = d2 ? d2 - src : -1;

    if (d1 && d2 && (d2_idx - d1_idx > 1)) {
      sub_len = strlen(str_sub(dest, dest_size, src, d1_idx + 1, d2_idx - d1_idx - 1));
    }
    else {
      dest[0] = '\0';
    }
  }

  return sub_len;
}

/**
 * @brief Deletes a list of substrings from a string.
 *
 * @param[in,out] src The source string.
 * @param[in] ... List of substrings to delete. The final pointer MUST be NULL.
 *
 * @return The modified string.
 */
char *str_del(char *src, ...) {
  va_list del_list;
  char *del, *idx, *end, *ptr;

  va_start(del_list, src);

  /* Delete each substring in del_list from src. */
  while ((del = va_arg(del_list, char *)) != NULL) {
    if ((idx = end = strstr(src, del)) != NULL) {
      while ((end = strstr(ptr = end + strlen(del), del)) != NULL) {
        while (ptr < end) {
          *idx++ = *ptr++;
        }
      }

      while ((*idx++ = *ptr++) != '\0');
    }
  }

  va_end(del_list);

  return src;
}

/**
 * @brief Converts a string to uppercase, storing the uppercase version in the
 *   specified destination string.
 *
 * @param[in,out] dest The destination string.
 * @param[in] dest_size The size allocated for the destination string.
 * @param[in] src The string to convert.
 *
 * @return The uppercase version of the string.
 */
char *str_upper(char *dest, size_t dest_size, char *src) {
  size_t str_len = strlen(src), i;

  /* Ensure len does not exceed (dest_sz - 1) to allow for null termination. */
  if (str_len >= dest_size) {
    str_len = dest_size - 1;
  }

  /* Copy len characters from src to dest. */
  strncpy(dest, src, str_len);

  /* Null-terminate dest. */
  dest[str_len + 1] = '\0';

  /* Convert each character in dest to uppercase. */
  for (i = 0; i < str_len; i++) {
    dest[i] = toupper(dest[i]);
  }

  return dest;
}

/**
 * @brief Converts a string to lowercase, storing the lowercase version in the
 *   specified destination string.
 *
 * @param[in,out] dest The destination string.
 * @param[in] dest_size The size allocated for the destination string.
 * @param[in] src The string to convert.
 *
 * @return The lowercase version of the string.
 */
char *str_lower(char *dest, size_t dest_size, char *src) {
  size_t str_len = strlen(src), i;

  /* Ensure len does not exceed (dest_sz - 1) to allow for null termination. */
  if (str_len >= dest_size) {
    str_len = dest_size - 1;
  }

  /* Copy len characters from src to dest. */
  strncpy(dest, src, str_len);

  /* Null-terminate dest. */
  dest[str_len + 1] = '\0';

  /* Convert each character in dest to uppercase. */
  for (i = 0; i < str_len; i++) {
    dest[i] = tolower(dest[i]);
  }

  return dest;
}

/**
 * @brief Converts a string to uppercase.
 *
 * @param[in,out] str The string to convert.
 *
 * @return The uppercase string.
 */
char *str_upr(char *str) {
  size_t i;

  /* Convert each character in str to uppercase. */
  for (i = 0; i < strlen(str); i++) {
    str[i] = toupper(str[i]);
  }

  return str;
}

/**
 * @brief Converts a string to lowercase.
 *
 * @param[in,out] str The string to convert.
 *
 * @return The lowercase string.
 */
char *str_lwr(char *str) {
  size_t i;

  /* Convert each character in str to lowercase. */
  for (i = 0; i < strlen(str); i++) {
    str[i] = tolower(str[i]);
  }

  return str;
}

/* TODO: Eventually transition code that uses these wrappers to use the direct functions instead. */

/* Wrapper for strdup. */
char *str_dup(char *src) {
  return strdup(src);
}

/* Wrapper for strcasecmp. */
int str_cmp(char *str1, char *str2) {
  return strcasecmp(str1, str2);
}

/* Wrapper for strncasecmp. */
int strn_cmp(char *str1, char *str2, size_t n) {
  return strncasecmp(str1, str2, n);
}

/* Wrapper for str_upr.  */
char *string_to_upper(char *str) {
  return str_upr(str);
}

/* Wrapper for str_lwr.  */
char *string_to_lower(char *str) {
  return str_lwr(str);
}


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
    log_f("WARNING: Unable to open death.log.");

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
  level = MIN(LEVEL_IMP, MAX(LEVEL_IMM, level));

  char buf[2 * MSL];

  snprintf(buf, sizeof(buf), "** %s **\n\r", str);

  for (DESC *desc = descriptor_list; desc; desc = desc->next) {
    if ((desc->connected == CON_PLYNG) && IS_IMMORTAL(desc->character) && (desc->wizinfo >= level)) {
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
  level = MIN(LEVEL_IMP, MAX(LEVEL_IMM, level));
  which = (((which < 1) || (which > 7)) ? 6 : which);

  char buf[2 * MSL];

  snprintf(buf, sizeof(buf), "[ %s ]\n\r", str);

  for (DESC *desc = descriptor_list; desc; desc = desc->next) {
    if ((desc->connected == CON_PLYNG) && IS_IMMORTAL(desc->character) && (desc->wizinfo >= level)) {
      if (((which == 1) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_ONE)) ||
        ((which == 2) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_TWO)) ||
        ((which == 3) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_THREE)) ||
        ((which == 4) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_FOUR)) ||
        ((which == 5) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_FIVE)) ||
        ((which == 6) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_LOG_SIX)) ||
        ((which == 7) && IS_SET(GET_IMM_FLAGS(desc->character), WIZ_QUEST_INFO))) {
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


void snprint_bits(char *dest, size_t dest_size, long bits, const char * const list[]) {
  if (!dest || !dest_size || !list) return;

  int list_count = 0;

  while (*(list[list_count++]) != '\n');

  dest[0] = '\0';

  if (bits && list_count) {
    for (int shift = 0; shift < list_count; shift++) {
      if (IS_SET(bits, 1 << shift)) {
        str_cat(dest, dest_size, list[shift]);

        if (bits >> (shift + 1)) {
          str_cat(dest, dest_size, " ");
        }
      }
    }
  }

  if (!(*dest)) {
    snprintf(dest, dest_size, "NOBITS");
  }
}

void snprint_type(char *dest, size_t dest_size, int type, const char * const list[]) {
  if (!dest || !dest_size || !list) return;

  int list_count = 0;

  while (*(list[list_count++]) != '\n');

  if ((list_count > 0) && (type >= 0) && (type < list_count)) {
    snprintf(dest, dest_size, "%s", list[type]);
  }
  else {
    snprintf(dest, dest_size, "UNDEFINED");
  }
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


char *PERS_ex(CHAR *ch, CHAR *vict, int mode) {
  static char buf[MIL];

  buf[0] = '\0';

  if (ch && vict) {
    if (IS_NPC(ch) && CAN_SEE(vict, ch)) {
      snprintf(buf, sizeof(buf), "%s", GET_DISP_NAME(ch));
    }
    else if ((IS_MORTAL(ch) && IS_SET(mode, COMM_ACT_HIDE_NON_MORT)) || CAN_SEE(vict, ch)) {
      if (!IS_SET(mode, COMM_ACT_HIDE_PRETITLE)) {
        signal_char(ch, vict, MSG_SHOW_PRETITLE, buf);
      }

      str_cat(buf, sizeof(buf), GET_DISP_NAME(ch));
    }
    else {
      snprintf(buf, sizeof(buf), "Somebody");
    }
  }

  CAP(buf);

  return buf;
}


char *PERS(CHAR *ch, CHAR *vict) {
  return PERS_ex(ch, vict, COMM_ACT_HIDE_NORMAL);
}


char *POSSESS_ex(CHAR *ch, CHAR *vict, int mode) {
  static char buf[MIL];

  buf[0] = '\0';

  if (IS_NPC(ch) && CAN_SEE(vict, ch)) {
    snprintf(buf, sizeof(buf), "%s's", MOB_SHORT(ch));
  }
  else if ((IS_MORTAL(ch) && IS_SET(mode, COMM_ACT_HIDE_NON_MORT)) || CAN_SEE(vict, ch)) {
    if (!IS_SET(mode, COMM_ACT_HIDE_PRETITLE)) {
      signal_char(ch, vict, MSG_SHOW_PRETITLE, buf);
    }

    str_cat(buf, sizeof(buf), GET_NAME(ch));
    str_cat(buf, sizeof(buf), "'s");
  }
  else {
    snprintf(buf, sizeof(buf), "Somebody's");
  }

  CAP(buf);

  return buf;
}

char *POSSESS(CHAR *ch, CHAR *vict) {
  return POSSESS_ex(ch, vict, COMM_ACT_HIDE_NON_MORT);
}


// TODO: Make a better color system someday.
char *CHCLR(CHAR *ch, int color) {
  static char color_code[32];

  color_code[0] = '\0';

  if (ch->colors[0] && ch->colors[color]) {
    str_cat(color_code, sizeof(color_code), Color[(((ch->colors[color]) * 2) - 2)]);
    str_cat(color_code, sizeof(color_code), BKColor[ch->colors[13]]);
  }

  return color_code;
}


// TODO: Make a better color system someday.
char *ENDCHCLR(CHAR *ch) {
  static char color_code[32];

  color_code[0] = '\0';

  if (ch->colors[0] && ch->colors[1]) {
    str_cat(color_code, sizeof(color_code), Color[(((ch->colors[1]) * 2) - 2)]);
    str_cat(color_code, sizeof(color_code), BKColor[ch->colors[13]]);
  }

  return color_code;
}


/* 50% chance when victim level is the same as the attacker.
   1% change +/- based on level diff.
   0% chance when victim level is 50 levels or higher than the attacker. */
bool breakthrough(CHAR *ch, CHAR *victim, int skill_spell, int breakthrough_type) {
  if (((breakthrough_type == BT_INVUL) && !IS_AFFECTED(victim, AFF_INVUL)) ||
    ((breakthrough_type == BT_SPHERE) && !IS_AFFECTED(victim, AFF_SPHERE))) {
    return TRUE;
  }

  /* Invulnerability never applies to Hostile victims. */
  if ((breakthrough_type == BT_INVUL) && IS_SET(GET_TOGGLES(victim), TOG_HOSTILE)) {
    return TRUE;
  }

  int check = 50 + ((GET_LEVEL(ch) - GET_LEVEL(victim)));

  switch (GET_CLASS(ch)) {
    case CLASS_CLERIC:
      if (breakthrough_type == BT_INVUL) check -= 5;
      else if (breakthrough_type == BT_SPHERE) check += 5;
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
      if (breakthrough_type == BT_INVUL) check += 10;
      else if (breakthrough_type == BT_SPHERE) check -= 10;
      break;
    case CLASS_THIEF:
      if (breakthrough_type == BT_INVUL) check += 10;
      else if (breakthrough_type == BT_SPHERE) check -= 10;
      break;
    case CLASS_NINJA:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check += 0;
      break;
    case CLASS_ANTI_PALADIN:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check += 5;
      break;
    case CLASS_PALADIN:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check += 0;
      break;
    case CLASS_BARD:
      if (breakthrough_type == BT_INVUL) check += 0;
      else if (breakthrough_type == BT_SPHERE) check += 5;
      break;
    case CLASS_COMMANDO:
      if (breakthrough_type == BT_INVUL) check += 5;
      else if (breakthrough_type == BT_SPHERE) check += 5;
      break;
  }

  check = number(1, 100) <= check;

  /* Cunning */
  if (!check && IS_SET(GET_TOGGLES(ch), TOG_CUNNING) && (GET_MANA(ch) >= 10) &&
    (IS_MORTAL(ch) && (GET_CLASS(ch) == CLASS_THIEF) && (GET_LEVEL(ch) >= 50)) &&
    ((skill_spell == SKILL_BACKSTAB) || (skill_spell == SKILL_CIRCLE))) {
    act("$n's weapon flashes with brilliant energy as $e bores through $N's protective shield.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n's weapon gleams with azure light as $e pierces through your protective shield.", FALSE, ch, 0, victim, TO_VICT);
    act("Your weapon is briefly sheathed in energy as you slice through $N's protective shield.", FALSE, ch, 0, victim, TO_CHAR);

    GET_MANA(ch) -= 10;

    return TRUE;
  }

  return check;
}


bool IS_LIGHT(int room) {
  if (ROOM_LIGHT(room) > 0) return TRUE;                   // If the room has light sources present, it is lit.
  if (ROOM_ZONE(room) == 30) return TRUE;                  // If the room is in zone 30 (Midgaard), it is lit.
  if (IS_SET(ROOM_FLAGS(room), LIT)) return TRUE;          // If the room has the LIT flag, it is lit.
  if (IS_SET(ROOM_FLAGS(room), DARK)) return FALSE;        // If the room has the DARK flag, it is unlit.
  if (IS_SET(ROOM_FLAGS(room), INDOORS)) return FALSE;     // If the room has the INDOORS flag, it is unlit.
  if (ROOM_SECTOR_TYPE(room) == SECT_INSIDE) return FALSE; // If the room has the sector type SECT_INSIDE, it is unlit.
  if (weather_info.sunlight != SUN_DARK) return TRUE;      // If the sun is not dark, the room is lit.

  return FALSE;
}


bool IS_DARK(int room) {
  return !IS_LIGHT(room);
}


int CAN_SEE(CHAR *ch, CHAR *vict) {
  if (!ch || !vict) return FALSE;

  if (WIZ_INV(ch, vict) ||
    IMP_INV(ch, vict) ||
    NRM_INV(ch, vict) ||
    (!IS_IMMORTAL(ch) && IS_AFFECTED(ch, AFF_BLIND)) ||
    (IS_MORTAL(ch) && !IS_AFFECTED(ch, AFF_INFRAVISION) && !IS_LIGHT(CHAR_REAL_ROOM(ch)))) {
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


int GETOBJ_WEIGHT(OBJ *obj) {
  if (!obj) return 0;

  int weight = OBJ_WEIGHT(obj);

  if ((OBJ_TYPE(obj) == ITEM_DRINKCON) && OBJ_VALUE(obj, 0)) {
    weight *= (0.5 + ((OBJ_VALUE(obj, 1) / 2.0) / OBJ_VALUE(obj, 0)));
  }

  if (obj->contains) {
    for (OBJ *tmp_obj = OBJ_CONTAINS(obj); tmp_obj; tmp_obj = OBJ_NEXT_CONTENT(tmp_obj))
      weight += GETOBJ_WEIGHT(tmp_obj);
  }

  return weight;
}


int IS_CARRYING_W(CHAR *ch) {
  if (!ch) return 0;

  int weight = 0;

  for (OBJ *tmp_obj = GET_CARRYING(ch); tmp_obj; tmp_obj = OBJ_NEXT_CONTENT(tmp_obj)) {
    weight += GETOBJ_WEIGHT(tmp_obj);
  }

  return weight;
}


int IS_CARRYING_N(CHAR *ch) {
  if (!ch) return 0;

  int num = 0;

  for (OBJ *tmp_obj = GET_CARRYING(ch); tmp_obj; tmp_obj = OBJ_NEXT_CONTENT(tmp_obj)) {
    num++;
  }

  return num;
}


int COUNT_CONTENTS(OBJ *obj) {
  if (!obj) return 0;

  int num = 0;

  for (OBJ *tmp_obj = OBJ_CONTAINS(obj); tmp_obj; tmp_obj = OBJ_NEXT_CONTENT(tmp_obj)) {
    num++;
  }

  return num;
}


int COUNT_RENTABLE_CONTENTS(OBJ *obj) {
  if (!obj) return 0;

  int num = 0;

  for (OBJ *tmp_obj = OBJ_CONTAINS(obj); tmp_obj; tmp_obj = OBJ_NEXT_CONTENT(tmp_obj)) {
    if (IS_RENTABLE(tmp_obj)) num++;
  }

  return num;
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

int V_OBJ(OBJ *obj) {
  if (((OBJ_TYPE(obj) == ITEM_CONTAINER) && OBJ_VALUE(obj, 3)) ||
      (OBJ_TYPE(obj) == ITEM_SKIN) ||
      (OBJ_TYPE(obj) == ITEM_TROPHY))   {
    return 0;
  }

  if (obj->item_number < 0 || obj->item_number > top_of_objt) {
    return 0;
  }

  return obj_proto_table[obj->item_number].virtual;
}

int V_MOB(CHAR *ch) {
  return mob_proto_table[ch->nr].virtual;
}

int V_ROOM(CHAR *ch) {
  return world[CHAR_REAL_ROOM(ch)].number;
}

struct obj_data
*get_obj_room (int virtual, int loc) {
  struct obj_data *tmp = NULL, *next = NULL;
  int room = NOWHERE;

  if (loc == NOWHERE) return NULL;
  if ((room = real_room(loc)) == NOWHERE) return NULL;
  for (tmp=world[room].contents;tmp;tmp=next) {
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
  for (ch=world[frm].people;ch;ch=tmp) {
    tmp = ch->next_in_room;
    char_from_room(ch);
    char_to_room(ch, to);
    if (IS_MORTAL(ch)) do_look(ch,"",0);
  }
}


void
move_chars_to_room_ex(int frm, int to, bool mortals_only) {
  struct char_data *ch, *tmp;
  void do_look(struct char_data *ch, char *arg, int cmd);
  for (ch=world[frm].people;ch;ch=tmp) {
    tmp = ch->next_in_room;
    if(mortals_only) {
      if (!IS_MORTAL(ch)) continue;
    }
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


void check_equipment(CHAR *ch) {
  for (int eq_pos = 0; eq_pos < WIELD; eq_pos++) {
    equip_char(ch, unequip_char(ch, eq_pos), eq_pos);
  }

  equip_char(ch, unequip_char(ch, HOLD), HOLD);
  equip_char(ch, unequip_char(ch, WIELD), WIELD);

  update_pos(ch);
}


void produce_core() {
   char *np = NULL;

   *np = 1;
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
CHAR *get_ch_by_name(char *name) {
  for (DESC *desc = descriptor_list; desc; desc = desc->next) {
    if (desc && !desc->connected && desc->character && (strcasecmp(name, GET_NAME(desc->character)) == 0)) {
      return (desc->character);
    }
  }

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
  if (IS_AFFECTED(ch, AFF_FLY) || (GET_CLASS(ch) == CLASS_NINJA)) {
    has_boat = TRUE;
  }

  /* Check if they are carrying a boat. */
  for (OBJ *temp_obj = ch->carrying; !has_boat && temp_obj; temp_obj = temp_obj->next_content) {
    if (OBJ_TYPE(temp_obj) == ITEM_BOAT) {
      has_boat = TRUE;
    }
  }

  /* Check if they are wearing a boat object (e.g. Boots of Water Walking). */
  for (int i = 0; !has_boat && (i < MAX_WEAR); i++) {
    if (EQ(ch, i) && OBJ_TYPE(EQ(ch, i)) == ITEM_BOAT) {
      has_boat = TRUE;
    }
  }

  return has_boat;
}

int CHAR_HAS_LEGS(CHAR *ch) {
  switch(GET_CLASS(ch)) {
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


bool SAME_GROUP_EX(CHAR *ch1, CHAR *ch2, bool ignore_aff_group) {
  if (!ch1 || !ch2) return FALSE;

  if (ch1 == ch2) return TRUE;

  if (!ignore_aff_group && (!IS_AFFECTED(ch1, AFF_GROUP) || !IS_AFFECTED(ch2, AFF_GROUP))) return FALSE;

  CHAR *group_leader = (GET_MASTER(ch1) ? GET_MASTER(ch1) : ch1);

  if (!ignore_aff_group && !IS_AFFECTED(group_leader, AFF_GROUP)) return FALSE;

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


bool SAME_GROUP(CHAR *ch1, CHAR *ch2) {
  return SAME_GROUP_EX(ch1, ch2, FALSE);
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
  return (*(int *)a > *(int *)b) - (*(int *)a < *(int *)b);
}


int qcmp_int_asc(const void *a, const void *b) {
  return qcmp_int(a, b);
}


int qcmp_int_desc(const void *a, const void *b) {
  return qcmp_int(b, a);
}


/* Perform a binary search on an ordered integer array. If the array is
   un-ordered, this will fail. If re-ordering the list is acceptable, use
   qsort() on the array first, using the qcmp_int* functions above. */
int binary_search_int_array(const int array[], const int l, const int r, const int value) {
  if (r >= l) {
    const int mid = l + (r - l) / 2;

    if (array[mid] == value)
      return mid;

    if (array[mid] > value)
      return binary_search_int_array(array, l, mid - 1, value);

    return binary_search_int_array(array, mid + 1, r, value);
  }

  return -1;
}


/* O(n) linear search for a value in an integer array. */
bool in_int_array(const int array[], const size_t num_elems, const int value) {
  for (size_t i = 0; i < num_elems; i++) {
    if (array[i] == value) {
      return TRUE;
    }
  }

  return FALSE;
}


/* Shuffle an array of integers. */
void shuffle_int_array(int array[], const size_t num_elems) {
  if (num_elems > 1) {
    for (size_t i = num_elems - 1; i > 0; i--) {
      size_t j = number(0, i);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}


/* Shuffle a 2D array of integers. */
void shuffle_2d_int_array(int array[][2], const size_t num_elems) {
  if (num_elems > 1) {
    for (size_t i = num_elems - 1; i > 0; i--) {
      size_t j = number(0, i);
      int t0 = array[j][0];
      int t1 = array[j][1];
      array[j][0] = array[i][0];
      array[j][1] = array[i][1];
      array[i][0] = t0;
      array[i][1] = t1;
    }
  }
}


/* Takes a character object and an array of eligible affects and returns
   an affect randomly from the array if the character is NOT affected by it.
   If there are no eligible effects found, this returns -1. */
int get_random_eligible_affect(CHAR *ch, const int eligible_affects_list[], const size_t list_size) {
  int affect = -1;

  if (ch && list_size) {
    int eligible_affects[MAX_SPL_LIST - 1] = { 0 }, eligible_affects_count = 0;

    for (size_t i = 0; i < list_size; i++) {
      if (!affected_by_spell(ch, eligible_affects_list[i])) {
        eligible_affects[eligible_affects_count++] = eligible_affects_list[i];
      }
    }

    if (eligible_affects_count > 0) {
      affect =  eligible_affects[number(0, eligible_affects_count - 1)];
    }
  }

  return affect;
}


/* Takes a character object and an array of eligible affects and returns
   an affect randomly from the array if the character IS affected by it.
   If there is no eligible affect found, this returns -1. */
int get_random_set_affect(CHAR *ch, const int eligible_affects_list[], const size_t list_size) {
  int affect = -1;

  if (ch && list_size) {
    int eligible_affects[MAX_SPL_LIST - 1] = { 0 }, eligible_affects_count = 0;

    for (AFF *aff = ch->affected; aff; aff = aff->next) {
      for (size_t i = 0; i < list_size; i++) {
        if (aff->type == eligible_affects_list[i]) {
          eligible_affects[eligible_affects_count++] = aff->type;
        }
      }
    }

    if (eligible_affects_count > 0) {
      affect = eligible_affects[number(0, eligible_affects_count - 1)];
    }
  }

  return affect;
}


/* Takes a bit mask and and returns one of the set bits randomly,
   or zero if no bits were set. */
int get_random_set_bit_from_mask(const int32_t mask) {
  const int32_t mask_size = (sizeof(int32_t) * CHAR_BIT);

  if (mask == 0) return 0;

  int32_t eligible_bits[mask_size];

  int32_t num_eligible_bits = 0;
  for (int32_t i = 0; i < mask_size; i++) {
    int32_t flag = (1 << i);
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


//Function to give all mortals in the room of a MOB, an AQP reward.
//Currently used for Ubers and End Game Bosses
void mob_aq_reward(int aqp_reward, CHAR *mob){
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;
	
	for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
	{
		next_vict = CHAR_NEXT_IN_ROOM(vict); 
		if (IS_NPC(vict) || !IS_MORTAL(vict)) continue;
		snprintf(buf, sizeof(buf),"You are awarded with %d quest point%s for the kill.\n\r", aqp_reward, aqp_reward > 1 ? "s" : "");
		send_to_char(buf, vict);
		vict->ver3.quest_points += aqp_reward;
	}
	
}


void strip_object_antis(OBJ *obj, int min_percent, int max_percent)
{
    int i, roll;

    static const int anti_flags[] = {
		ITEM_ANTI_GOOD,
		ITEM_ANTI_EVIL,
		ITEM_ANTI_NEUTRAL,
		ITEM_ANTI_WARRIOR,
		ITEM_ANTI_THIEF,
		ITEM_ANTI_CLERIC,
		ITEM_ANTI_MAGIC_USER,
		ITEM_ANTI_NINJA,
		ITEM_ANTI_NOMAD,
		ITEM_ANTI_PALADIN,
        ITEM_ANTI_ANTIPALADIN,
        ITEM_ANTI_BARD,
		ITEM_ANTI_COMMANDO
    };

    if (!obj)
        return;

    if (min_percent < 0) min_percent = 0;
    if (max_percent > 100) max_percent = 100;
    if (min_percent > max_percent)
        min_percent = max_percent;

    for (i = 0; i < NUMELEMS(anti_flags); i++)
    {
        roll = number(min_percent, max_percent);
        if (chance(roll))
            REMOVE_BIT(obj->obj_flags.extra_flags, anti_flags[i]);
    }
}