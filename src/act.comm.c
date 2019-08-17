/* ************************************************************************
*  file: act.comm.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Communication.                                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
Majority of act.comm.c code re-written by Night.
*/

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utility.h"
#include "act.h"
#include "constants.h"
#include "utils.h"
#include "db.h"
#include "cmd.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "spells.h"

extern char *skip_spaces(char * string);

const comm_info_t comm_info[] = {
  {
    .name = "tell",
    .text_to_ch = "$n tell $N",
    .text_to_vict = "$n tells $N",
    .text_to_other = "",
    .text_no_arg = "Who do you wish to tell, and what?",
    .text_not_found = "No one by that name here...",
    .pflag_on = 0,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT | PLR_NOMESSAGE,
    .pflag_no_hear = PLR_NOSHOUT | PLR_NOMESSAGE,
    .min_pos_hear = 0,
    .to = COMM_TO_CHAR,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_QUOTE,
    .direct = TRUE,
    .set_reply = TRUE,
    .color = 6,
  },
  {
    .name = "whisper",
    .text_to_ch = "$n whisper to $N",
    .text_to_vict = "$n whispers to $N",
    .text_to_other = "$n whispers something to $N.",
    .text_no_arg = "Who do you wish to whisper, and what?",
    .text_not_found = "No one by that name here...",
    .pflag_on = 0,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT,
    .pflag_no_hear = PLR_NOSHOUT | PLR_NOMESSAGE,
    .min_pos_hear = 0,
    .to = COMM_TO_CHAR_ROOM,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_QUOTE,
    .direct = TRUE,
    .set_reply = TRUE,
    .color = 0,
  },
  {
    .name = "ask",
    .text_to_ch = "$n ask $N",
    .text_to_vict = "$n asks $N",
    .text_to_other = "$n asks $N a question.",
    .text_no_arg = "Who do you wish to ask, and what?",
    .text_not_found = "No one by that name here...",
    .pflag_on = 0,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT,
    .pflag_no_hear = PLR_NOSHOUT | PLR_NOMESSAGE,
    .min_pos_hear = 0,
    .to = COMM_TO_CHAR_ROOM,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_QUOTE,
    .direct = TRUE,
    .set_reply = TRUE,
    .color = 0,
  },
  {
    .name = "reply",
    .text_to_ch = "$n reply to $N",
    .text_to_vict = "$n replies",
    .text_to_other = "",
    .text_no_arg = "Yes, but what do you wish to reply?",
    .text_not_found = "There is nobody to reply to...",
    .pflag_on = 0,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT,
    .pflag_no_hear = PLR_NOSHOUT,
    .min_pos_hear = 0,
    .to = COMM_TO_REPLY,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_QUOTE,
    .direct = TRUE,
    .set_reply = TRUE,
    .color = 6,
  },
  {
    .name = "gtell",
    .text_to_ch = "$n tell your group",
    .text_to_vict = "** $n tells $N",
    .text_to_other = "",
    .text_no_arg = "Yes, but what do you want to tell your group?",
    .text_not_found = "",
    .pflag_on = 0,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT,
    .pflag_no_hear = 0,
    .min_pos_hear = 0,
    .to = COMM_TO_GROUP,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_QUOTE,
    .direct = FALSE,
    .set_reply = FALSE,
    .color = 10,
  },
  {
    .name = "say",
    .text_to_ch = "$n say",
    .text_to_vict = "$n says",
    .text_to_other = "",
    .text_no_arg = "Yes, but what do you want to say?",
    .text_not_found = "",
    .pflag_on = 0,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT | PLR_QUIET,
    .pflag_no_hear = 0,
    .min_pos_hear = POSITION_RESTING,
    .to = COMM_TO_ROOM,
    .hide = FALSE,
    .style = COMM_STYLE_QUOTE,
    .direct = FALSE,
    .set_reply = FALSE,
    .color = 9,
  },
  {
    .name = "shout",
    .text_to_ch = "$n shout",
    .text_to_vict = "$n shouts",
    .text_to_other = "",
    .text_no_arg = "Shout?  Yes!  Fine!  Shout we must, but WHAT?",
    .text_not_found = "",
    .pflag_on = 0,
    .pflag_off = PLR_SHOUT_OFF,
    .pflag_no_do = PLR_NOSHOUT | PLR_KILL | PLR_THIEF | PLR_QUIET,
    .pflag_no_hear = PLR_NOSHOUT,
    .min_pos_hear = 0,
    .to = COMM_TO_ZONE,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_QUOTE,
    .direct = FALSE,
    .set_reply = FALSE,
    .color = 8,
  },
  {
    .name = "yell",
    .text_to_ch = "$n yell",
    .text_to_vict = "$n yells",
    .text_to_other = "",
    .text_no_arg = "Yell?  Yes!  Fine!  Yell we must, but WHAT?",
    .text_not_found = "",
    .pflag_on = 0,
    .pflag_off = PLR_YELL_OFF,
    .pflag_no_do = PLR_NOSHOUT | PLR_KILL | PLR_THIEF | PLR_QUIET,
    .pflag_no_hear = 0,
    .min_pos_hear = 0,
    .to = COMM_TO_WORLD,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_QUOTE,
    .direct = FALSE,
    .set_reply = FALSE,
    .color = 8,
  },
  {
    .name = "gossip",
    .text_to_ch = "$n (gossip)",
    .text_to_vict = "$n (gossip)",
    .text_to_other = "",
    .text_no_arg = "Yes, but what do you want to say on the gossip channel?",
    .text_not_found = "",
    .pflag_on = PLR_AUCTION,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT | PLR_QUIET,
    .pflag_no_hear = 0,
    .min_pos_hear = 0,
    .to = COMM_TO_WORLD,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_BRACKET,
    .direct = FALSE,
    .set_reply = FALSE,
    .color = 5,
  },
  {
    .name = "auction",
    .text_to_ch = "$n (auction)",
    .text_to_vict = "$n (auction)",
    .text_to_other = "",
    .text_no_arg = "Yes, but what do you want to say on the auction channel?",
    .text_not_found = "",
    .pflag_on = PLR_AUCTION,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT | PLR_QUIET,
    .pflag_no_hear = 0,
    .min_pos_hear = 0,
    .to = COMM_TO_WORLD,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_BRACKET,
    .direct = FALSE,
    .set_reply = FALSE,
    .color = 7,
  },
  {
    .name = "quest",
    .text_to_ch = "$n (quest)",
    .text_to_vict = "$n (quest)",
    .text_to_other = "",
    .text_no_arg = "Yes, but what do you want say on the quest channel?",
    .text_not_found = "",
    .pflag_on = PLR_QUESTC,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT | PLR_QUIET,
    .pflag_no_hear = 0,
    .min_pos_hear = 0,
    .to = COMM_TO_WORLD,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_BRACKET,
    .direct = FALSE,
    .set_reply = FALSE,
    .color = 14,
  },
  {
    .name = "chaos",
    .text_to_ch = "$n (chaos)",
    .text_to_vict = "$n (chaos)",
    .text_to_other = "",
    .text_no_arg = "Yes, but what do you want say on the chaos channel?",
    .text_not_found = "",
    .pflag_on = PLR_CHAOS,
    .pflag_off = 0,
    .pflag_no_do = PLR_NOSHOUT | PLR_QUIET,
    .pflag_no_hear = 0,
    .min_pos_hear = 0,
    .to = COMM_TO_WORLD,
    .hide = PERS_MORTAL,
    .style = COMM_STYLE_BRACKET,
    .direct = FALSE,
    .set_reply = FALSE,
    .color = 15,
  },
};


// TODO: Sanitize mud-wide color system.

const color_t color_info[] = {
  { .code = "\x1B[0m\x1B[30m", .name = "Black",        .key = "a" },
  { .code = "\x1B[0m\x1B[31m", .name = "Dark Red",     .key = "b" },
  { .code = "\x1B[0m\x1B[32m", .name = "Dark Green",   .key = "c" },
  { .code = "\x1B[0m\x1B[33m", .name = "Dark Yellow",  .key = "d" },
  { .code = "\x1B[0m\x1B[34m", .name = "Dark Blue",    .key = "e" },
  { .code = "\x1B[0m\x1B[35m", .name = "Dark Magenta", .key = "f" },
  { .code = "\x1B[0m\x1B[36m", .name = "Dark Cyan",    .key = "g" },
  { .code = "\x1B[0m\x1B[37m", .name = "Grey",         .key = "h" },
  { .code = "\x1B[1m\x1B[31m", .name = "Red",          .key = "i" },
  { .code = "\x1B[1m\x1B[32m", .name = "Green",        .key = "j" },
  { .code = "\x1B[1m\x1B[33m", .name = "Yellow",       .key = "k" },
  { .code = "\x1B[1m\x1B[34m", .name = "Blue",         .key = "l" },
  { .code = "\x1B[1m\x1B[35m", .name = "Magenta",      .key = "m" },
  { .code = "\x1B[1m\x1B[36m", .name = "Cyan",         .key = "n" },
  { .code = "\x1B[1m\x1B[37m", .name = "White",        .key = "o" },
  { .code = "\x1B[1m\x1B[30m", .name = "Dark Grey",    .key = "p" },
};


const color_field_t color_field_info[] = {
  { .field = "A", .name = "Text"               },
  { .field = "B", .name = "Prompt"             },
  { .field = "C", .name = "Room Name"          },
  { .field = "D", .name = "Exits"              },
  { .field = "E", .name = "Gossips"            },
  { .field = "F", .name = "Tells/Replies"      },
  { .field = "G", .name = "Auctions/Score"     },
  { .field = "H", .name = "Shouts/Yells"       },
  { .field = "I", .name = "Says"               },
  { .field = "J", .name = "Group Tells/Wiznet" },
  { .field = "K", .name = "Combat: Your Hits"  },
  { .field = "L", .name = "Combat: Mob Hits"   },
  { .field = "M", .name = "Background"         },
  { .field = "N", .name = "Quest"              },
  { .field = "O", .name = "Chaos"              },
};


struct drunk_t drunk[] = {
  { 3, 10, {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"} },
  { 8, 5,  {"b", "b", "b", "B", "B", "vb"} },
  { 3, 5,  {"c", "c", "C", "cj", "sj", "zj"} },
  { 5, 2,  {"d", "d", "D"} },
  { 3, 3,  {"e", "e", "eh", "E"} },
  { 4, 5,  {"f", "f", "ff", "fff", "fFf", "F"} },
  { 8, 2,  {"g", "g", "G"} },
  { 9, 6,  {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"} },
  { 7, 6,  {"i", "i", "Iii", "ii", "iI", "Ii", "I"} },
  { 9, 5,  {"j", "j", "jj", "Jj", "jJ", "J"} },
  { 7, 2,  {"k", "k", "K"} },
  { 3, 2,  {"l", "l", "L"} },
  { 5, 8,  {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"} },
  { 6, 6,  {"n", "n", "nn", "Nn", "nnn", "nNn", "N"} },
  { 3, 6,  {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"} },
  { 3, 2,  {"p", "p", "P"} },
  { 5, 5,  {"q", "q", "Q", "ku", "ququ", "kukeleku"} },
  { 4, 2,  {"r", "r", "R"} },
  { 2, 5,  {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"} },
  { 5, 2,  {"t", "t", "T"} },
  { 3, 6,  {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"} },
  { 4, 2,  {"v", "v", "V"} },
  { 4, 2,  {"w", "w", "W"} },
  { 5, 6,  {"x", "x", "X", "ks", "iks", "kz", "xz"} },
  { 3, 2,  {"y", "y", "Y"} },
  { 2, 9,  {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"} }
};


/* Make a string look drunk. Original code by Apex. Modifications by the Maniac
   from Mythran for envy(2), Kohl Desenee for ROM 2.4, and Ranger for RoninMUD.
   Re-written by Night. */
void drunkify_string(char *dst, const size_t dst_sz, const int drunk_level, const char *src, ...) {
  if (!src || !dst || !dst_sz) return;

  char str[MSL], buf[2 * MSL];
  va_list args;

  va_start(args, src);
  vsnprintf(str, sizeof(str), src, args);
  va_end(args);

  memset(buf, 0, sizeof(buf));

  int buf_pos = 0;

  for (int i = 0; (i < strlen(str)) && (buf_pos < (sizeof(buf) - 1)); i++) {
    char temp;

    temp = str[i];
    temp = UPPER(temp);

    if ((temp >= 'A') && (temp <= 'Z') && (drunk_level > drunk[temp - 'A'].min_drunk_level)) {
      char *replacement_str = drunk[temp - 'A'].replacement[number(0, drunk[temp - 'A'].num_replacements)];

      strncpy(&buf[buf_pos], replacement_str, sizeof(buf) - buf_pos);

      buf_pos += strlen(replacement_str);
    }
    else if (temp >= '0' && temp <= '9') {
      buf[buf_pos++] = '0' + number(0, 9);
    }
    else {
      buf[buf_pos++] = str[i];
    }
  }

  /* Null-terminate the drunkified string. */
  buf[sizeof(buf) - 1] = '\0';

  /* If the drunkified string is longer than the destination string, truncate it with ellipses. */
  if (strlen(buf) > dst_sz) {
    strcpy(&buf[dst_sz - 4], "...\0");
  }

  /* Write the drunkified string into the destination string. */
  snprintf(dst, dst_sz, "%s", buf);
}


/* Unified communication function. */
void communicate(CHAR *ch, char *arg, const int comm) {
  if (!ch || (comm < COMM_FIRST) || (comm > COMM_LAST)) return;

  arg = skip_spaces(arg);

  /* No arg? Let the character know. */
  if (!*arg) {
    printf_to_char(ch, "%s\n\r", comm_info[comm].text_no_arg);

    return;
  }

  /* NPCs can't use reply. */
  if (IS_NPC(ch) && (comm_info[comm].to == COMM_TO_REPLY)) {
    printf_to_char(ch, "NPCs can't use reply.\n\r");

    return;
  }

  /* Check for player conditions that might prevent the chosen method of communication. */
  if (!IS_NPC(ch)) {
    /* Is NoShout on?*/
    if (IS_SET(comm_info[comm].pflag_no_do, PLR_NOSHOUT) && IS_SET(GET_PFLAG(ch), PLR_NOSHOUT)) {
      printf_to_char(ch, "You have displeased the gods; you can't use %s.\n\r", comm_info[comm].name);

      return;
    }

    /* Is the player a killer or a thief? (Only applies when not Chaos Mode). */
    if (!CHAOSMODE &&
        ((IS_SET(comm_info[comm].pflag_no_do, PLR_KILL) && IS_SET(GET_PFLAG(ch), PLR_KILL)) ||
         (IS_SET(comm_info[comm].pflag_no_do, PLR_THIEF) && IS_SET(GET_PFLAG(ch), PLR_THIEF)))) {
      printf_to_char(ch, "You can't %s during Chaos!\n\r", comm_info[comm].name);

      return;
    }

    /* Is NoMessage on? */
    if (IS_SET(comm_info[comm].pflag_no_do, PLR_NOMESSAGE) && IS_SET(GET_PFLAG(ch), PLR_NOMESSAGE)) {
      printf_to_char(ch, "You can't use %s when you have NoMessage turned on.\n\r", comm_info[comm].name);

      return;
    }

    /* Is the player questing with their Quiet flag on? */
    if (IS_SET(comm_info[comm].pflag_no_do, PLR_QUIET) && (IS_SET(GET_PFLAG(ch), PLR_QUIET) && IS_SET(GET_PFLAG(ch), PLR_QUEST))) {
      printf_to_char(ch, "The Questmaster has taken away your ability to interrupt.\n\r");

      return;
    }

    /* Is the channel on? If not, turn it on. */
    if (comm_info[comm].pflag_on && !IS_SET(GET_PFLAG(ch), comm_info[comm].pflag_on)) {
      SET_BIT(GET_PFLAG(ch), comm_info[comm].pflag_on);

      printf_to_char(ch, "You turn ON the %s channel.\n\r", comm_info[comm].name);
    }
  }

  CHAR *listener = NULL;

  char name[MIL];

  /* If direct communication, get the listening character, and verify they can be reached, etc. */
  if (comm_info[comm].direct) {
    switch (comm_info[comm].to) {
      case COMM_TO_CHAR:
        arg = skip_spaces(one_argument(arg, name));

        listener = get_ch_by_name(name);
        break; // COMM_TO_CHAR

      case COMM_TO_CHAR_ROOM:
        arg = skip_spaces(one_argument(arg, name));

        listener = get_char_room_vis(ch, name);
        break; // COMM_TO_CHAR_ROOM

      case COMM_TO_REPLY:
        listener = get_ch_by_id(GET_REPLY_TO(ch));
        break; // COMM_TO_REPLY
    }

    /* Redirect listener if they're mobswitched. */
    if (listener && listener->switched) {
      listener = listener->switched;
    }

    /* The listening player was not found, or can't be seen. Let the acting character know. */
    if (!listener || ((comm_info[comm].to != COMM_TO_REPLY) && WIZ_INV(ch, listener))) {
      printf_to_char(ch, "%s\n\r", comm_info[comm].text_not_found);

      return;
    }

    /* Is the listening character connected? */
    if (!GET_DESCRIPTOR(listener) || (GET_DESCRIPTOR(listener)->connected != CON_PLYNG)) {
      act("$E isn't connected at the moment.", PERS_MORTAL, ch, 0, listener, TO_CHAR);

      return;
    }

    /* Is the listening character's NoShout on? */
    if (IS_MORTAL(ch) && (listener != ch) && IS_SET(comm_info[comm].pflag_no_hear, PLR_NOSHOUT) && IS_SET(GET_PFLAG(listener), PLR_NOSHOUT)) {
      act("The gods have taken away $S ability to communicate.", PERS_MORTAL, ch, 0, listener, TO_CHAR);

      return;
    }

    /* Is the listening character's NoMessage on? Is their position below the listening threshold? */
    if ((IS_MORTAL(ch) && (listener != ch) && IS_SET(comm_info[comm].pflag_no_hear, PLR_NOMESSAGE) && IS_SET(GET_PFLAG(listener), PLR_NOMESSAGE)) ||
        (GET_POS(listener) < comm_info[comm].min_pos_hear)) {
      act("$E can't hear you.", PERS_MORTAL, ch, 0, listener, TO_CHAR);

      return;
    }
  }

  char message[MSL];

  /* Drunkify the string as needed, otherwise, store the resulting arg text in the message variable. */
  if (!IS_NPC(ch) && (GET_COND(ch, DRUNK) > 10)) {
    drunkify_string(message, sizeof(message), GET_COND(ch, DRUNK), "%s", arg);
  }
  else {
    snprintf(message, sizeof(message), "%s", arg);
  }

  char style_open[3], style_close[3];

  /* Prepare the style for surrounding the message text. */
  switch (comm_info[comm].style) {
    case COMM_STYLE_QUOTE:
      snprintf(style_open, sizeof(style_open), "'");
      snprintf(style_close, sizeof(style_close), "'");
      break; // COMM_STYLE_QUOTE

    case COMM_STYLE_BRACKET:
      snprintf(style_open, sizeof(style_open), "[ ");
      snprintf(style_close, sizeof(style_close), " ]");
      break; // COMM_STYLE_BRACKET
  }

  char buf[MSL];

  /* Build the message string for the acting character. */
  snprintf(buf, sizeof(buf), "%s %s%s%s%s", comm_info[comm].text_to_ch,
           ((comm_info[comm].smell && affected_by_spell(ch, SMELL_FARTMOUTH)) ? "`q" : ""),
           style_open, message, style_close);

  /* Print the message string to the acting character. */
  if (comm_info[comm].color) COLOR(ch, comm_info[comm].color);
  act(buf, FALSE, ch, 0, listener, TO_CHAR);
  if (comm_info[comm].color) ENDCOLOR(ch);

  /* Build the message string for the listening character(s). */
  snprintf(buf, sizeof(buf), "%s %s%s%s%s", comm_info[comm].text_to_vict,
           ((comm_info[comm].smell && affected_by_spell(ch, SMELL_FARTMOUTH)) ? "`q" : ""),
           style_open, message, style_close);

  /* Print the message string to the listening character(s). */
  switch (comm_info[comm].to) {
    case COMM_TO_CHAR:
    case COMM_TO_CHAR_ROOM:
    case COMM_TO_REPLY:
      if (comm_info[comm].color) COLOR(listener, comm_info[comm].color);
      act(buf, comm_info[comm].hide, ch, 0, listener, TO_VICT);
      if (comm_info[comm].color) ENDCOLOR(listener);
      break; // COMM_TO_CHAR, COMM_TO_CHAR_ROOM

    case COMM_TO_GROUP:
      for (DESC* temp_desc = descriptor_list; temp_desc; temp_desc = temp_desc->next) {
        if ((temp_desc->connected != CON_PLYNG) || !temp_desc->character) continue;

        CHAR* temp_ch = temp_desc->character;

        if ((temp_ch != ch) && !IS_SET(GET_PFLAG(temp_ch), comm_info[comm].pflag_no_hear) && (GET_POS(temp_ch) >= comm_info[comm].min_pos_hear) &&
            SAME_GROUP(temp_ch, ch)) {
          if (comm_info[comm].color) COLOR(temp_ch, comm_info[comm].color);
          act(buf, comm_info[comm].hide, ch, 0, temp_ch, TO_VICT);
          if (comm_info[comm].color) ENDCOLOR(temp_ch);
        }
      }
      break; // COMM_TO_GROUP

    case COMM_TO_ROOM:
      for (CHAR* temp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); temp_ch; temp_ch = temp_ch->next_in_room) {
        if (!GET_DESCRIPTOR(temp_ch) || (GET_DESCRIPTOR(temp_ch)->connected != CON_PLYNG)) continue;

        if ((temp_ch != ch) && !IS_SET(GET_PFLAG(temp_ch), comm_info[comm].pflag_no_hear) && (GET_POS(temp_ch) >= comm_info[comm].min_pos_hear)) {
          if (comm_info[comm].color) COLOR(temp_ch, comm_info[comm].color);
          act(buf, comm_info[comm].hide, ch, 0, temp_ch, TO_VICT);
          if (comm_info[comm].color) ENDCOLOR(temp_ch);
        }
      }
      break; // COMM_TO_ROOM

    case COMM_TO_ZONE:
      for (DESC* temp_desc = descriptor_list; temp_desc; temp_desc = temp_desc->next) {
        if ((temp_desc->connected != CON_PLYNG) || !temp_desc->character) continue;

        CHAR* temp_ch = temp_desc->character;

        if ((temp_ch != ch) && !IS_SET(GET_PFLAG(temp_ch), comm_info[comm].pflag_no_hear) && (GET_POS(temp_ch) >= comm_info[comm].min_pos_hear) &&
            ((GET_ZONE(temp_ch) == GET_ZONE(ch)) || IS_IMMORTAL(temp_ch))) {
          if (comm_info[comm].color) COLOR(temp_ch, comm_info[comm].color);
          act(buf, comm_info[comm].hide, ch, 0, temp_ch, TO_VICT);
          if (comm_info[comm].color) ENDCOLOR(temp_ch);
        }
      }
      break; // COMM_TO_ZONE

    case COMM_TO_WORLD:
      for (DESC* temp_desc = descriptor_list; temp_desc; temp_desc = temp_desc->next) {
        if ((temp_desc->connected != CON_PLYNG) || !temp_desc->character) continue;

        CHAR* temp_ch = temp_desc->character;

        if ((temp_ch != ch) && !IS_SET(GET_PFLAG(temp_ch), comm_info[comm].pflag_no_hear) && (GET_POS(temp_ch) >= comm_info[comm].min_pos_hear)) {
          if (comm_info[comm].color) COLOR(temp_ch, comm_info[comm].color);
          act(buf, comm_info[comm].hide, ch, 0, temp_ch, TO_VICT);
          if (comm_info[comm].color) ENDCOLOR(temp_ch);
        }
      }
      break; // COMM_TO_WORLD
  }

  /* Perform some post-communication activites if there's a listening character. */
  if (listener) {
    /* Print text_to_other to the acting character's room. */
    if (strlen(comm_info[comm].text_to_other)) {
      snprintf(buf, sizeof(buf), "%s", comm_info[comm].text_to_other);

      act(buf, comm_info[comm].hide, ch, 0, listener, TO_NOTVICT);
    }

    /* Set the listener's reply_to value to the acting character's ID. */
    if (comm_info[comm].set_reply && (listener != ch) && !IS_NPC(listener) && !IS_NPC(ch) && GET_ID(ch)) {
      GET_REPLY_TO(listener) = GET_ID(ch);
    }
  }

  /* Signal communication events. */
  switch (comm) {
    case COMM_SAY:
      signal_room(CHAR_REAL_ROOM(ch), ch, MSG_SAID, arg);
      break;
  }
}


/* Function called by the 'tell' command. */
void do_tell(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_TELL);
}


/* Function called by the 'reply' command. */
void do_reply(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_REPLY);
}


/* Function called by the 'ask' command. */
void do_ask(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_ASK);
}


/* Function called by the 'whisper' command. */
void do_whisper(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_WHISPER);
}


/* Function called by the 'gtell' command. */
void do_gtell(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_GTELL);
}


/* Function called by the 'say' command. */
void do_say(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_SAY);
}


/* Function called by the 'shout' command. */
void do_shout(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_SHOUT);
}

/* Function called by the 'yell' command. */
void do_yell(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_YELL);
}


/* Function called by the 'gossip' command. */
void do_gossip(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_GOSSIP);
}


/* Function called by the 'auction' command. */
void do_auction(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_AUCTION);
}


/* Function called by the 'chaos' command. */
void do_chaos(CHAR *ch, char *arg, int cmd) {
  communicate(ch, arg, COMM_CHAOS);
}


/* For special procedure direct communications from mobs to players.
   Accepts printf-style args and bypasses all restrictions. */
void comm_special(CHAR *ch, CHAR *listener, const int comm, const char *message, ...) {
  if (!ch || !IS_NPC(ch) || !listener || IS_NPC(listener) || !message || !*message) return;

  /* Direct communication only; no reply. */
  if (!comm_info[comm].direct || (comm_info[comm].to == COMM_TO_REPLY)) return;

  /* Redirect listener if they're mobswitched. */
  if (listener->switched) {
    listener = listener->switched;
  }

  /* We still didn't find a valid listener, so return. */
  if (!listener || !GET_DESCRIPTOR(listener) || (GET_DESCRIPTOR(listener)->connected != CON_PLYNG)) return;

  char buf[MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  char style_open[3], style_close[3];

  /* Prepare the style for surrounding the message text. */
  switch (comm_info[comm].style) {
    case COMM_STYLE_QUOTE:
      snprintf(style_open, sizeof(style_open), "'");
      snprintf(style_close, sizeof(style_close), "'");
      break; // COMM_STYLE_QUOTE

    case COMM_STYLE_BRACKET:
      snprintf(style_open, sizeof(style_open), "[ ");
      snprintf(style_close, sizeof(style_close), " ]");
      break; // COMM_STYLE_BRACKET
  }

  /* Show the text to the acting character. */
  if (comm_info[comm].color) COLOR(ch, comm_info[comm].color);
  act_f(comm_info[comm].hide, ch, 0, listener, TO_CHAR, "%s %s%s%s", comm_info[comm].text_to_ch,
    style_open, buf, style_close);
  if (comm_info[comm].color) ENDCOLOR(ch);

  /* Show the text to the listening character. */
  if (comm_info[comm].color) COLOR(listener, comm_info[comm].color);
  act_f(comm_info[comm].hide, ch, 0, listener, TO_VICT, "%s %s%s%s", comm_info[comm].text_to_vict,
    style_open, buf, style_close);
  if (comm_info[comm].color) ENDCOLOR(listener);

  /* Show the text to other characters (if applicable). */
  if (listener && strlen(comm_info[comm].text_to_other)) {
    act_f(comm_info[comm].hide, ch, 0, listener, TO_NOTVICT, "%s", comm_info[comm].text_to_other);
  }
}


/* Function called by the 'channel' command. */
void do_channel(CHAR *ch, char *arg, int cmd) {
  if (!ch) return;

  if (IS_NPC(ch)) {
    printf_to_char(ch, "NPCs can't use the channel command.\n\r");

    return;
  }

  char buf[MIL], setting[MIL];

  chop_string(arg, buf, sizeof(buf), setting, sizeof(setting));

  /* Print channel on/off status if no argument is provided. */
  if (!*buf || is_abbrev(buf, "list")) {
    int longest_channel_name = 0;

    for (int i = 0; i < NUMELEMS(comm_info); i++) {
      if (comm_info[i].pflag_on || comm_info[i].pflag_off) {
        longest_channel_name = MAX(longest_channel_name, strlen(comm_info[i].name));
      }
    }

    printf_to_char(ch, "Channel Settings:\n\r-----------------\n\r");

    for (int i = 0; i < NUMELEMS(comm_info); i++) {
      if (comm_info[i].pflag_on) {
        snprintf(buf, sizeof(buf), "%s", comm_info[i].name);
        printf_to_char(ch, "%-*s is %s\n\r", longest_channel_name, CAP(buf), IS_SET(GET_PFLAG(ch), comm_info[i].pflag_on) ? "ON" : "OFF");
      }
      else if (comm_info[i].pflag_off) {
        snprintf(buf, sizeof(buf), "%s", comm_info[i].name);
        printf_to_char(ch, "%-*s is %s\n\r", longest_channel_name, CAP(buf), !IS_SET(GET_PFLAG(ch), comm_info[i].pflag_off) ? "ON" : "OFF");
      }
    }

    return;
  }

  int channel = -1;

  /* Get the channel number. */
  for (int i = 0; i < NUMELEMS(comm_info); i++) {
    if ((comm_info[i].pflag_on || comm_info[i].pflag_off) && (strncasecmp(buf, comm_info[i].name, strlen(buf)) == 0)) {
      channel = i;
      break;
    }
  }

  /* Invalid channel. */
  if (channel == -1) {
    int channel_count = 0;

    for (int i = 0; i < NUMELEMS(comm_info); i++) {
      if (comm_info[i].pflag_on || comm_info[i].pflag_off) {
        channel_count++;
      }
    }

    printf_to_char(ch, "Invalid channel.");

    if (channel_count) {
      printf_to_char(ch, "  Your options are: ");

      for (int i = 0; channel_count && (i < NUMELEMS(comm_info)); i++) {
        if (comm_info[i].pflag_on || comm_info[i].pflag_off) {
          printf_to_char(ch, "%s%s", comm_info[i].name, ((channel_count > 1) ? ", " : "."));

          channel_count--;
        }
      }

      printf_to_char(ch, "\n\r");
    }

    return;
  }

  if (!*setting || is_abbrev(setting, "list")) {
    memset(buf, 0, sizeof(buf));
    
    for (int i = 0; i < strlen(comm_info[channel].name); i++) {
      strlcat(buf, "-", sizeof(buf));
    }

    printf_to_char(ch, "Players in the %s channel:\n\r------------------------%s\n\r", comm_info[channel].name, buf);

    /* Loop through the descriptors in the game and show visibile players in the channel. */
    for (DESC *temp_desc = descriptor_list; temp_desc; temp_desc = temp_desc->next) {
      if ((temp_desc->connected != CON_PLYNG) || !temp_desc->character) continue;

      CHAR * temp_ch = temp_desc->character;

      if ((IS_SET(GET_PFLAG(temp_ch), comm_info[channel].pflag_on) || !IS_SET(GET_PFLAG(temp_ch), comm_info[channel].pflag_off)) &&
        (CHAR_REAL_ROOM(temp_ch) != NOWHERE) && !WIZ_INV(ch, temp_ch)) {
        printf_to_char(ch, "%s\n\r", GET_NAME(temp_ch));
      }
    }
  }
  else if (is_abbrev(setting, "on")) {
    if (comm_info[channel].pflag_on) {
      SET_BIT(GET_PFLAG(ch), comm_info[channel].pflag_on);

      printf_to_char(ch, "You turn %s the %s channel.\n\r", (IS_SET(GET_PFLAG(ch), comm_info[channel].pflag_on) ? "ON" : "OFF"), comm_info[channel].name);
    }
    else if (comm_info[channel].pflag_off) {
      REMOVE_BIT(GET_PFLAG(ch), comm_info[channel].pflag_off);

      printf_to_char(ch, "You turn %s the %s channel.\n\r", (IS_SET(GET_PFLAG(ch), comm_info[channel].pflag_off) ? "OFF" : "ON"), comm_info[channel].name);
    }
    else {
      printf_to_char(ch, "The %s channel cannot be turned on or off.\n\r", comm_info[channel].name);
    }
  }
  else if (is_abbrev(setting, "off")) {
    if (comm_info[channel].pflag_on) {
      REMOVE_BIT(GET_PFLAG(ch), comm_info[channel].pflag_on);

      printf_to_char(ch, "You turn %s the %s channel.\n\r", (IS_SET(GET_PFLAG(ch), comm_info[channel].pflag_on) ? "ON" : "OFF"), comm_info[channel].name);
    }
    else if (comm_info[channel].pflag_off) {
      SET_BIT(GET_PFLAG(ch), comm_info[channel].pflag_off);

      printf_to_char(ch, "You turn %s the %s channel.\n\r", (IS_SET(GET_PFLAG(ch), comm_info[channel].pflag_off) ? "OFF" : "ON"), comm_info[channel].name);
    }
    else {
      printf_to_char(ch, "The %s channel cannot be turned on or off.\n\r", comm_info[channel].name);
    }
  }
  else {
    printf_to_char(ch, "Usage: channel [auction|chaos|gossip|quest] <list|on|off>\n\r");
  }
}


/* Function called by the 'color' command. */
void do_setcolor(CHAR *ch, char *arg, int cmd) {
  if (IS_NPC(ch)) {
    send_to_char("NPCs can't set color.\n\r", ch);

    return;
  }

  char buf[MIL];

  arg = one_argument(arg, buf);

  if (!*buf) {
    GET_COMM_COLOR(ch, 0) = (GET_COMM_COLOR(ch, 0) == 0) ? 1 : 0;

    printf_to_char(ch, "Color is now %s.\n\r", (GET_COMM_COLOR(ch, 0) ? "`cON`q" : "OFF"));

    return;
  }

  if (is_abbrev(buf, "text")) {
    GET_COMM_COLOR(ch, 0) = (GET_COMM_COLOR(ch, 0) == 1) ? 2 : 1;

    printf_to_char(ch, "You will %s see added text color.\n\r", ((GET_COMM_COLOR(ch, 0) == 1) ? "now" : "no longer"));

    return;
  }

  if (is_abbrev(buf, "colors")) {
    send_to_char("0 : Disable\n\r", ch);

    for (int i = 0; i < NUMELEMS(color_info); i++) {
      char c = color_info[i].key[0];

      printf_to_char(ch, "%c : `%sSAMPLE`q %s\n\r",
        UPPER(c),
        color_info[i].key,
        color_info[i].name);
    }

    send_to_char("Q : Disable\n\r", ch);

    return;
  }

  if (is_abbrev(buf, "list")) {
    int longest_field_string = 0;

    for (int i = 0; i < NUMELEMS(color_field_info); i++) {
      longest_field_string = MAX(longest_field_string, strlen(color_field_info[i].name));
    }

    for (int i = 0; i < NUMELEMS(color_field_info); i++) {
      printf_to_char(ch, "%s : %-*s : `%sSAMPLE`q %s\n\r",
        color_field_info[i].field,
        longest_field_string,
        color_field_info[i].name,
        color_info[GET_COMM_COLOR(ch, i + 1) - 1].key,
        color_info[GET_COMM_COLOR(ch, i + 1) - 1].name);
    }

    return;
  }

  if (is_abbrev(buf, "set")) {
    arg = one_argument(arg, buf);

    if (!*buf) {
      printf_to_char(ch, "Set what?\n\r");

      return;
    }

    int color_field = -1;

    for (int i = 0; i < NUMELEMS(color_field_info); i++) {
      if (strcasecmp(buf, color_field_info[i].field) == 0) {
        color_field = i + 1;
        break;
      }
    }

    if (color_field < 0) {
      printf_to_char(ch, "Invalid field.  See 'color list' for possible fields.\n\r");

      return;
    }

    arg = one_argument(arg, buf);

    if ((strcasecmp(buf, "0") == 0) || (strcasecmp(buf, "q") == 0)) {
      printf_to_char(ch, "Color disabled for %s.\n\r", color_field_info[color_field].name);

      GET_COMM_COLOR(ch, color_field) = 0;

      return;
    }

    int color_idx = -1;

    for (int i = 0; i < NUMELEMS(color_info); i++) {
      if (strcasecmp(buf, color_info[i].key) == 0) {
        color_idx = i;
        break;
      }
    }

    if (color_idx <= 0) {
      printf_to_char(ch, "Invalid color.  See 'color colors' for possible colors.\n\r");

      return;
    }

    if ((color_field == 13) && (color_idx > 8)) {
      printf_to_char(ch, "Background color can only be set to colors A through H.\n\r");

      return;
    }

    GET_COMM_COLOR(ch, color_field) = color_idx + 1;

    printf_to_char(ch, "Color set to `%s%s`q for %s.\n\r", color_info[color_idx].key, color_info[color_idx].name, color_field_info[color_field - 1].name);

    return;
  }

  printf_to_char(ch, "Usage: color <colors|list|set|text> <field> <color>\n\r");
}


/* Function called by the 'title' command.*/
void do_title(CHAR * ch, char *arg, int cmd) {
  if (IS_NPC(ch)) {
    send_to_char("NPCs can't change their titles.\n\r", ch);

    return;
  }

  arg = skip_spaces(arg);

  /* Title command with no argument toggles the automatic titles given at level on and off. */
  if (!*arg) {
    TOGGLE_BIT(GET_PFLAG(ch), PLR_SKIPTITLE);

    printf_to_char(ch, "Your title will %s change when leveling.\n\r", (IS_SET(GET_PFLAG(ch), PLR_SKIPTITLE) ? "no longer" : "now"));

    return;
  }

  /* Set the player's title to default if so requested with the 'default' argument, otherwise set it to the string specified. */
  set_title(ch, ((strcasecmp(arg, "default") == 0) ? NULL : arg));

  printf_to_char(ch, "Done.\n\r");
}


/* Function called by the 'report' command. */
void do_report(CHAR * ch, char *argument, int cmd) {
  printf_to_char(ch, "You report %d(%d hp) %d(%d mana) %d(%d mv).\n\r",
    GET_HIT(ch), GET_MAX_HIT(ch),
    GET_MANA(ch), GET_MAX_MANA(ch),
    GET_MOVE(ch), GET_MAX_MOVE(ch));

  printf_to_room_except(CHAR_REAL_ROOM(ch), ch, "%s reports %d(%d hp) %d(%d mana) %d(%d mv).\n\r",
    GET_NAME(ch),
    GET_HIT(ch), GET_MAX_HIT(ch),
    GET_MANA(ch), GET_MAX_MANA(ch),
    GET_MOVE(ch), GET_MAX_MOVE(ch));
}


/* Function called by the 'write' command. */
void do_write(CHAR *ch, char *arg, int cmd) {
  /* Character does not have a descriptor so they can't write. */
  if (!GET_DESCRIPTOR(ch)) return;

  char arg1[MIL], arg2[MIL];

  two_arguments(arg, arg1, arg2);

  /* Write command was called with no argument. */
  if (!*arg1) {
    send_to_char("Write? On what? With what? What are you trying to do?\n\r", ch);

    return;
  }

  OBJ *note = NULL;

  /* The first argument was provided. This indicates the note to be written on. */
  if (*arg1) {
    /* Check the character's inventory for a visible object matching the first argument provided. */
    if (!(note = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      /* If nothing was found in the character's inventory, try the object held by the character. */
      if (!(note = EQ(ch, HOLD)) || !isname(arg1, OBJ_NAME(note))) {
        /* No object of the name provided was found. */
        printf_to_char(ch, "You have no %s.\n\r", arg1);

        return;
      }
    }
  }

  OBJ *pen = NULL;

  /* The second argument was provided. This indicates the pen to write with. */
  if (*arg2) {
    /* Check the character's inventory for a visible object matching the second argument provided. */
    if (!(pen = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
      /* If nothing was found in the character's inventory, try the object held by the character. */
      if (!(pen = EQ(ch, HOLD)) || !isname(arg2, OBJ_NAME(pen))) {
        /* No object of the name provided was found. */
        printf_to_char(ch, "You have no %s.\n\r", arg2);

        return;
      }
    }
  }

  /* An object was found to write on but no argument was provided to indicate a writing implement.
     The writing implement might be held by the character. */
  if (note && !*arg2 && !(pen = EQ(ch, HOLD))) {
    act("You can't write with $p alone.", TRUE, ch, note, 0, TO_CHAR);

    return;
  }

  /* An object was found to be written on and an object was found to write with. */
  if (note && pen) {
    /* No writing with invisible objects. */
    if (!CAN_SEE_OBJ(ch, note) || !CAN_SEE_OBJ(ch, pen)) {
      send_to_char("The stuff you're trying to write with is invisible! Yeech!\n\r", ch);

      return;
    }

    /* The object to write on isn't a note. */
    if (OBJ_TYPE(note) != ITEM_NOTE) {
      act("You can't write on $p.", TRUE, ch, note, 0, TO_CHAR);

      return;
    }

    /* The writing implement isn't a pen. */
    if (OBJ_TYPE(pen) != ITEM_PEN) {
      act("$p is no good for writing with.", TRUE, ch, pen, 0, TO_CHAR);

      return;
    }

    /* The note has already been written on. */
    if (OBJ_GET_ACTION(note)) {
      send_to_char("There's something written on it already.\n\r", ch);

      return;
    }

    /* Everything checked out. The character can write a note. */
    send_to_char("Ok, go ahead and write; end the note with an @ symbol on a new line.\n\r", ch);
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);

    SET_BIT(GET_PFLAG(ch), PLR_WRITING);

    DESC_MAX_STR(GET_DESCRIPTOR(ch)) = MSL / 2;
    DESC_STR(GET_DESCRIPTOR(ch)) = &OBJ_GET_ACTION(note);
  }
}
