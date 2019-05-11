/* ************************************************************************
*  file: act.comm.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Communication.                                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
Majority of act.comm.c code re-written by Night.
*/

/* Includes */

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


/* Externs */

extern char *skip_spaces(char * string);


/* Variables */

char *color_field_full[] =
{
  "(A) Text               ",
  "(B) Prompt             ",
  "(C) Room Name          ",
  "(D) Exits              ",
  "(E) Gossips            ",
  "(F) Tells              ",
  "(G) Auctions/Score     ",
  "(H) Shouts/Yells       ",
  "(I) Says               ",
  "(J) Group Tells        ",
  "(K) Combat->Your Hits  ",
  "(L) Combat->Mob Hits   ",
  "(M) Background         ",
  "(N) Quest              ",
  "(O) Chaos              ",
  "(P)                    "
};


char *color_field_simple[] =
{
  "A",
  "B",
  "C",
  "D",
  "E",
  "F",
  "G",
  "H",
  "I",
  "J",
  "K",
  "L",
  "M",
  "N",
  "O",
  "P",
  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "10",
  "11",
  "12",
  "13",
  "14",
  "15",
  "16"
};


char *color_key[] =
{
  "a",
  "b",
  "c",
  "d",
  "e",
  "f",
  "g",
  "h",
  "i",
  "j",
  "k",
  "l",
  "m",
  "n",
  "o",
  "p",
  "q"
};


/* Structs */

struct drunk_t {
  int min_drunk_level;
  int num_replacements;
  char *replacement[11];
};


struct drunk_t drunk[] =
{
  {3, 10, {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
  {8, 5,  {"b", "b", "b", "B", "B", "vb"}},
  {3, 5,  {"c", "c", "C", "cj", "sj", "zj"}},
  {5, 2,  {"d", "d", "D"}},
  {3, 3,  {"e", "e", "eh", "E"}},
  {4, 5,  {"f", "f", "ff", "fff", "fFf", "F"}},
  {8, 2,  {"g", "g", "G"}},
  {9, 6,  {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
  {7, 6,  {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
  {9, 5,  {"j", "j", "jj", "Jj", "jJ", "J"}},
  {7, 2,  {"k", "k", "K"}},
  {3, 2,  {"l", "l", "L"}},
  {5, 8,  {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
  {6, 6,  {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
  {3, 6,  {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
  {3, 2,  {"p", "p", "P"}},
  {5, 5,  {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
  {4, 2,  {"r", "r", "R"}},
  {2, 5,  {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
  {5, 2,  {"t", "t", "T"}},
  {3, 6,  {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
  {4, 2,  {"v", "v", "V"}},
  {4, 2,  {"w", "w", "W"}},
  {5, 6,  {"x", "x", "X", "ks", "iks", "kz", "xz"}},
  {3, 2,  {"y", "y", "Y"}},
  {2, 9,  {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
};



/* Prototypes */

char *make_drunk(char *string, CHAR *ch);
void do_setcolor(CHAR *ch, char *arg, int cmd);
void do_title(CHAR *ch, char *arg, int cmd);
void do_report(CHAR *ch, char *argument, int cmd);
void do_say(CHAR *ch, char *arg, int cmd);
void do_yell(CHAR *ch, char *arg, int cmd);
void do_shout(CHAR *ch, char *arg, int cmd);
void do_gossip(CHAR *ch, char *arg, int cmd);
void do_auction(CHAR *ch, char *arg, int cmd);
void do_chaos(CHAR *ch, char *arg, int cmd);
void do_channel(CHAR *ch, char *arg, int cmd);
void do_tell(CHAR *ch, char *arg, int cmd);
void do_reply(CHAR *ch, char *arg, int cmd);
void do_gtell(CHAR *ch, char *arg, int cmd);
void do_ask(CHAR *ch, char *arg, int cmd);
void do_whisper(CHAR *ch, char *arg, int cmd);
void do_write(CHAR *ch, char *arg, int cmd);


/* Functions */

/* Make a string look drunk. Original code by Apex. Modifications by the Maniac
   from Mythran for envy(2), Kohl Desenee for ROM 2.4 and Ranger for RoninMUD.
   Re-written by Night, 12/10/2011 */
char *make_drunk(char *string, CHAR *ch) {
  char buf[MSL];
  char temp_string[MIL];
  int drunk_level = 0;
  int i = 0;
  char temp = 0;
  int replacement_number = 0;
  int character_position = 0;

  /* Copy the input string into a temporary array for processing. */
  strcpy(temp_string, string);

  /* Check how drunk the character is. */
  drunk_level = GET_COND(ch, DRUNK);

  /* Character is drunk so we'll mangle their text. */
  if (drunk_level > 0) {
    /* Loop through each character in the string. */
    for (i = 0; i < strlen(temp_string); i++) {
      temp = UPPER(temp_string[i]);

      /* Check if the character is A through Z. */
      if (temp >= 'A' &&
        temp <= 'Z') {
        /* If the character is drunk enough, mangle the character. */
        if (drunk_level > drunk[temp - 'A'].min_drunk_level) {
          replacement_number = number(0, drunk[temp - 'A'].num_replacements);
          strcpy(&buf[character_position], drunk[temp - 'A'].replacement[replacement_number]);
          character_position += strlen(drunk[temp - 'A'].replacement[replacement_number]);
        }
      }
      else /* Scramble any numbers in the text. */
        if (temp >= '0' &&
          temp <= '9') {
          temp = '0' + number(0, 9);

          buf[character_position] = temp;
          character_position++;
        }
        else /* Non-alphanumeric character, leave it alone. */
        {
          buf[character_position] = temp_string[i];
          character_position++;
        }
    }

    /* If the mangled text is too big, truncate it and replace the last three characters with ... */
    if (character_position > MIL) {
      strcpy(&buf[MIL - 3], "...");
      character_position = MIL;
    }

    /* Ensure that the string is null-terminated. */
    buf[character_position] = '\0';

    /* Copy the mangled text into the original string variable. */
    strcpy(string, buf);
  }

  return string;
}


/* Function called by the 'color' command.
   Re-written by Night, 12/10/2011 */
void do_setcolor(CHAR *ch, char *arg, int cmd) {
  char buf[MSL];
  char first_arg[10];
  char second_arg[10];
  char field_name[10];
  char field_color[10];
  int i = 0;
  int temp = 0;
  int set_field = 0;
  int set_color = 0;

  /* NPCs don't get to use the color command. */
  if (IS_NPC(ch)) {
    send_to_char("NPCs can't turn on color.\n\r", ch);

    return;
  }

  half_chop(arg, first_arg, sizeof(first_arg), second_arg, sizeof(second_arg));

  /* Color command was called without any arguments. Toggle color on or off. */
  if (!*first_arg) {
    if (ch->colors[0]) {
      send_to_char("Color is now OFF.\n\r", ch);
      ch->colors[0] = 0;
    }
    else {
      send_to_char("Color is now ON.\n\r", ch);
      ch->colors[0] = 1;
    }
  }
  else /* Color command was called with an argument of 'text'. Toggle added text color on or off.*/
    if (!strcmp(first_arg, "text")) {
      /* Color is off. */
      if (ch->colors[0] == 0) {
        send_to_char("You must turn color on first.\n\r", ch);
      }
      else
        if (ch->colors[0] == 1) {
          send_to_char("You will no longer see added text color.\n\r", ch);
          ch->colors[0] = 2;
        }
        else
          if (ch->colors[0] == 2) {
            send_to_char("You will now see added text color.\n\r", ch);
            ch->colors[0] = 1;
          }
    }
    else /* Color command was called with an argument of 'list'. Show the list of the character's */
      if (!strcmp(first_arg, "list")) {
        /* Loop through the color fields. */
        for (i = 0; i < (MAX_COLORS - 1); i++) {
          /* Wiznet uses the same color as Group Tells. */
          if (i == 9 &&
            (!IS_NPC(ch) && !IS_MORTAL(ch))) {
            send_to_char("(J) Group Tells/Wiznet ", ch);
          }
          else /* Print the name of the color field. */
          {
            send_to_char(color_field_full[i], ch);
          }

          /* Check if the current color field is set. */
          temp = ch->colors[i + 1];

          /* If the color field isn't set, print that out. */
          if (!temp) {
            send_to_char(":        Not set.\n\r", ch);
          }
          else /* Print the current color and show a sample of it. */
          {
            sprintf(buf, ": `%sSAMPLE`q %s\n\r", color_key[temp - 1], Color[(temp * 2) - 1]);
            send_to_char(buf, ch);
          }
        }
      }
      else /* Color command was called with an argument of 'colors'. Show the list of colors. */
        if (!strcmp(first_arg, "colors")) {
          send_to_char(" 0 : Disable\n\r", ch);

          /* Loop through the available colors, print out the code and show a sample of it. */
          for (i = 1; i < 17; i++) {
            sprintf(buf, " %s : `%sSAMPLE`q ", color_field_simple[i - 1], color_key[i - 1]);
            send_to_char(buf, ch);
            send_to_char(Color[(i * 2) - 1], ch);
            send_to_char("\n\r", ch);
          }

          send_to_char(" Q : Disable\n\r", ch);
        }
        else /* Color command was called with an argument of 'set'. */
          if (!strcmp(first_arg, "set")) {
            /* No argument was provided to the set option. */
            if (!*second_arg) {
              send_to_char("Set what?\n\r", ch);

              return;
            }

            half_chop(second_arg, field_name, sizeof(field_name), field_color, sizeof(field_color));

            /* No color was provided. */
            if (!*field_color) {
              send_to_char("What color?\n\r", ch);

              return;
            }

            /* Loop through the possible color fields and check if the field specified exists. */
            for (i = 0; i < (MAX_COLORS - 1); i++) {
              if (!str_cmp(field_name, color_field_simple[i])) {
                set_field = i + 1;
              }
            }

            /* The field specified doesn't exist. */
            if (!set_field) {
              send_to_char("Invalid field.\n\r", ch);

              return;
            }

            /* Loop through the possible colors and check if the color specified exists. */
            for (i = 0; i < 16; i++) {
              if (!str_cmp(field_color, color_field_simple[i])) {
                set_color = (i + 1);
              }
            }

            /* Fixed color disable, Ranger - June 96 */
            if (!strcmp(field_color, "0") || !strcmp(field_color, "q")) {
              send_to_char("Color disabled.\n\r", ch);
              set_color = 0;
            }
            else /* The color specified doesn't exist. */
              if (!set_color) {
                send_to_char("Invalid color.\n\r", ch);

                return;
              }
              else /* The specified field was the background and an invalid color was chosen. */
                if (set_field == 13 &&
                  set_color > 8) {
                  send_to_char("Background color can only be (A-H).\n\r", ch);

                  return;
                }

            ch->colors[set_field] = set_color;

            /* Fixed color disable, Ranger - June 96*/
            if (set_color) {
              send_to_char("Color set.\n\r", ch);
            }
          }
          else /* Color command was called with an unknown option. Print command usage info. */
          {
            send_to_char("Usage: color <list|colors|set|text> <field> <color>\n\r", ch);
          }

  ENDCOLOR(ch);
}


/* Function called by the 'title' command.
   Re-written by Night, 12/11/2011 */
void do_title(CHAR *ch, char *arg, int cmd) {
  if (IS_NPC(ch)) {
    send_to_char("NPCs can't change their titles!\n\r", ch);

    return;
  }

  arg = skip_spaces(arg);

  /* Title command with no argument toggles the automatic titles given at level on and off. */
  if (!*arg) {
    if (IS_SET(ch->specials.pflag, PLR_SKIPTITLE)) {
      send_to_char("Your title will now change when leveling.\n\r", ch);
      REMOVE_BIT(ch->specials.pflag, PLR_SKIPTITLE);
    }
    else {
      send_to_char("Your title will no longer change when leveling.\n\r", ch);
      SET_BIT(ch->specials.pflag, PLR_SKIPTITLE);
    }

    return;
  }

  /* Set the player's title to default if so requested with the 'def' or 'default' arguments,
     otherwise set it to the argument specified. */
  if (!strcmp(arg, "def") ||
    !strcmp(arg, "default")) {
    set_title(ch, NULL);
  }
  else {
    set_title(ch, arg);
  }

  send_to_char("Done.\n\r", ch);
}


/* Function called by the 'report' command. */
void do_report(CHAR *ch, char *argument, int cmd) {
  printf_to_room_except(CHAR_REAL_ROOM(ch), ch, "%s reports %d(%d hp) %d(%d mana) %d(%d mv).\n\r",
    GET_NAME(ch),
    GET_HIT(ch), GET_MAX_HIT(ch),
    GET_MANA(ch), GET_MAX_MANA(ch),
    GET_MOVE(ch), GET_MAX_MOVE(ch));

  printf_to_char(ch, "You report %d(%d hp) %d(%d mana) %d(%d mv).\n\r",
    GET_HIT(ch), GET_MAX_HIT(ch),
    GET_MANA(ch), GET_MAX_MANA(ch),
    GET_MOVE(ch), GET_MAX_MOVE(ch));
}


/* Function called by the 'say' command.
   Re-written by Night, 12/10/2011 */
void do_say(CHAR *ch, char *arg, int cmd) {
  char buf[MSL];
  CHAR *listener = NULL;

  /* NoShouted players can't talk. */
  if (!IS_NPC(ch) &&
    IS_SET(ch->specials.pflag, PLR_NOSHOUT)) {
    send_to_char("You have displeased the gods, you cannot talk.\n\r", ch);

    return;
  }

  /* Ranger - June 96 */
  if (!IS_NPC(ch) &&
    IS_SET(ch->specials.pflag, PLR_QUEST) &&
    IS_SET(ch->specials.pflag, PLR_QUIET)) {
    send_to_char("The Questmaster has taken away your ability to interrupt.\n\r", ch);

    return;
  }

  /* Ranger - July 96 */
  if (IS_MORTAL(ch) &&
    IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, QUIET)) {
    send_to_char("A magical force absorbs your words.\n\r", ch);

    return;
  }

  arg = skip_spaces(arg);

  if (!*arg) {
    send_to_char("Yes, but what do you want to say?\n\r", ch);

    return;
  }

  if (GET_COND(ch, DRUNK) > 10) {
    arg = make_drunk(arg, ch);
  }

  /* Build the buffer for the acting character. */
  /* Fartmouth makes the text brown. */
  if (affected_by_spell(ch, SMELL_FARTMOUTH)) {
    sprintf(buf, "You say `d'%s'", arg);
  }
  else {
    sprintf(buf, "You say '%s'", arg);
  }

  /* Show the text to the acting character. */
  COLOR(ch, 9);
  act(buf, 0, ch, 0, 0, TO_CHAR);
  ENDCOLOR(ch);

  /* Build the buffer for the listeners. */
  /* Fartmouth makes the text brown. */
  if (affected_by_spell(ch, SMELL_FARTMOUTH)) {
    sprintf(buf, "$n says `d'%s'", arg);
  }
  else {
    sprintf(buf, "$n says '%s'", arg);
  }

  /* Loop through the characters in the room and show them the text if they're connected
     and in an awake state. */
  for (listener = world[CHAR_REAL_ROOM(ch)].people; listener; listener = listener->next_in_room) {
    if (!listener->desc) continue;

    if (listener != ch &&
      AWAKE(listener)) {
      /* Show the text to the listener. */
      COLOR(listener, 9);
      act(buf, FALSE, ch, 0, listener, TO_VICT);
      ENDCOLOR(listener);
    }
  }

  /* Signal the game that an event happened in the room. */
  signal_room(CHAR_REAL_ROOM(ch), ch, MSG_SAID, arg);
}


/* Function called by the 'shout' command.
   Re-written by Night, 1/6/2012 */
void do_shout(CHAR *ch, char *arg, int cmd) {
  char buf[MSL];
  struct descriptor_data *listener_desc = NULL;
  CHAR *listener = NULL;

  /* Various conditions that might prevent a character from yelling. */
  if (!IS_NPC(ch) &&
    (IS_SET(ch->specials.pflag, PLR_NOSHOUT) ||
      IS_SET(ch->specials.pflag, PLR_NOMESSAGE) ||
      (IS_SET(ch->specials.pflag, PLR_KILL) && !CHAOSMODE) ||
      (IS_SET(ch->specials.pflag, PLR_THIEF) && !CHAOSMODE))) {
    send_to_char("You can't shout!\n\r", ch);

    return;
  }

  if (!IS_NPC(ch) &&
    IS_SET(ch->specials.pflag, PLR_QUEST) &&
    IS_SET(ch->specials.pflag, PLR_QUIET)) {
    send_to_char("The Questmaster has taken away your ability to interrupt.\n\r", ch);

    return;
  }

  arg = skip_spaces(arg);

  if (!*arg) {
    send_to_char("Shout? Yes! Fine! Shout we must, but WHAT?\n\r", ch);

    return;
  }

  if (GET_COND(ch, DRUNK) > 10) {
    arg = make_drunk(arg, ch);
  }

  /* Build the buffer for the acting character. */
  sprintf(buf, "You shout '%s'", arg);

  /* Show the text to the acting character. */
  COLOR(ch, 8);
  act(buf, 0, ch, 0, 0, TO_CHAR);
  ENDCOLOR(ch);

  /* Build the buffer for the listeners. */
  sprintf(buf, "$n shouts '%s'", arg);

  for (listener_desc = descriptor_list; listener_desc; listener_desc = listener_desc->next) {
    listener = listener_desc->character;

    if (!listener) continue;

    if (!listener->desc->connected &&
      listener != ch &&
      !IS_SET(listener->specials.pflag, PLR_NOSHOUT) &&
      (world[CHAR_REAL_ROOM(listener)].zone == world[CHAR_REAL_ROOM(ch)].zone || GET_LEVEL(listener) > LEVEL_MORT)) {
      /* Show the text to the listener. */
      COLOR(listener, 8);
      act(buf, 0, ch, 0, listener, TO_VICT);
      ENDCOLOR(listener);
    }
  }
}


/* Function called by the 'yell' command.
   Re-written by Night, 1/6/2012 */
void do_yell(CHAR *ch, char *arg, int cmd) {
  char buf[MSL];
  struct descriptor_data *listener_desc = NULL;
  CHAR *listener = NULL;

  arg = skip_spaces(arg);

  if (!*arg) {
    /* If it's a player, toggle the yell channel on and off if no argument provided. */
    if (!IS_NPC(ch)) {
      if (IS_SET(ch->specials.pflag, PLR_NOYELL)) {
        send_to_char("You turn ON the yell channel.\n\r", ch);
        REMOVE_BIT(ch->specials.pflag, PLR_NOYELL);
      }
      else {
        send_to_char("You turn OFF the yell channel.\n\r", ch);
        SET_BIT(ch->specials.pflag, PLR_NOYELL);
      }
    }
    else /* It's an NPC. */
    {
      send_to_char("What do you want to yell?\n\r", ch);
    }

    return;
  }

  /* Various conditions that might prevent a character from yelling. */
  if (!IS_NPC(ch) &&
    (IS_SET(ch->specials.pflag, PLR_NOSHOUT) ||
      IS_SET(ch->specials.pflag, PLR_NOMESSAGE) ||
      (IS_SET(ch->specials.pflag, PLR_KILL) && !CHAOSMODE) ||
      (IS_SET(ch->specials.pflag, PLR_THIEF) && !CHAOSMODE))) {
    send_to_char("You can't yell!\n\r", ch);

    return;
  }

  if (!IS_NPC(ch) &&
    IS_SET(ch->specials.pflag, PLR_QUEST) &&
    IS_SET(ch->specials.pflag, PLR_QUIET)) {
    send_to_char("The Questmaster has taken away your ability to interrupt.\n\r", ch);

    return;
  }

  /* Turn the yell channel on if it's off. */
  if (!IS_NPC(ch) &&
    IS_SET(ch->specials.pflag, PLR_NOYELL)) {
    send_to_char("You turn ON the yell channel.\n\r", ch);
    REMOVE_BIT(ch->specials.pflag, PLR_NOYELL);
  }

  if (GET_COND(ch, DRUNK) > 10) {
    arg = make_drunk(arg, ch);
  }

  /* Build the buffer for the acting character. */
  sprintf(buf, "You yell '%s'", arg);

  /* Show the text to the acting character. */
  COLOR(ch, 8);
  act(buf, 0, ch, 0, 0, TO_CHAR);
  ENDCOLOR(ch);

  /* Build the buffer for the listeners. */
  sprintf(buf, "$n yells '%s'", arg);

  /* Loop through the descriptors in the game and show them the text if they're connected
     and can hear yells. */
  for (listener_desc = descriptor_list; listener_desc; listener_desc = listener_desc->next) {
    listener = listener_desc->character;

    if (!listener) continue;

    if (!listener->desc->connected &&
      listener != ch &&
      !IS_SET(listener->specials.pflag, PLR_NOYELL) &&
      !IS_SET(listener->specials.pflag, PLR_NOSHOUT) &&
      (!IS_SET(listener->specials.pflag, PLR_NOSHOUT) || listener_desc->original)) {
      /* Show the text to the listener. */
      COLOR(listener, 8);
      act(buf, 0, ch, 0, listener, TO_VICT);
      ENDCOLOR(listener);
    }
  }
}


bool comm_pers(CHAR *ch, CHAR *vict) {
  if (!ch || !vict) return FALSE;

  if (WIZ_INV(ch, vict)) return FALSE;

  if (IS_MORTAL(vict)) return TRUE;

  return CAN_SEE(ch, vict);
}


void channel_comm(CHAR *ch, char *arg, int comm) {
  const channel_t channel_info[] = {
    { "gossip",  PLR_GOSSIP,  5 },
    { "auction", PLR_AUCTION, 7 },
    { "quest",   PLR_QUESTC,  14 },
    { "chaos",   PLR_CHAOS,   15 },
  };

  if (!ch || (comm < 0) || (comm >= NUMELEMS(channel_info))) return;

  arg = skip_spaces(arg);

  if (!*arg) {
    /* If it's a player, toggle the channel on or off if no argument is provided. */
    if (!IS_NPC(ch)) {
      if IS_SET(GET_PFLAG(ch), channel_info[comm].channel_flag) {
        REMOVE_BIT(GET_PFLAG(ch), channel_info[comm].channel_flag);
        printf_to_char(ch, "You turn OFF the %s channel.\n\r", channel_info[comm].channel_name);
      }
      else {
        SET_BIT(GET_PFLAG(ch), channel_info[comm].channel_flag);
        printf_to_char(ch, "You turn ON the %s channel.\n\r", channel_info[comm].channel_name);
      }
    }
    else {
      /* It's an NPC. */
      printf_to_char(ch, "What do you want to %s?\n\r", channel_info[comm].channel_name);
    }

    return;
  }

  /* NoShouted characters can't use the gossip channel. */
  if (!IS_NPC(ch) &&
      IS_SET(GET_PFLAG(ch), PLR_NOSHOUT)) {
    printf_to_char(ch, "You have displeased the gods; you cannot use the %s channel.\n\r", channel_info[comm].channel_name);

    return;
  }

  /* If Quest and Quiet flags, character can't interrupt the ongoing quest. */
  if (!IS_NPC(ch) &&
      IS_SET(GET_PFLAG(ch), PLR_QUEST) &&
      IS_SET(GET_PFLAG(ch), PLR_QUIET)) {
    printf_to_char(ch, "The Questmaster has taken away your ability to interrupt.\n\r");

    return;
  }

  /* Turn the channel on if it's off. */
  if (!IS_NPC(ch) &&
      !IS_SET(GET_PFLAG(ch), channel_info[comm].channel_flag)) {
    SET_BIT(GET_PFLAG(ch), channel_info[comm].channel_flag);

    printf_to_char(ch, "You turn ON the %s channel.\n\r", channel_info[comm].channel_name);
  }

  /* If the player's drunk, drunkify the text. */
  if (GET_COND(ch, DRUNK) > 10) {
    arg = make_drunk(arg, ch);
  }

  char buf[MSL];

  snprintf(buf, sizeof(buf), "$n (%s) [ %s ]", channel_info[comm].channel_name, arg);

  /* Show the text to the acting character. */
  COLOR(ch, channel_info[comm].channel_color);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  ENDCOLOR(ch);

  /* Loop through the descriptors in the game and show them the text if they're connected
     and can hear the channel. */
  for (DESC *listener_desc = descriptor_list; listener_desc; listener_desc = listener_desc->next) {
    CHAR *listener = listener_desc->character;

    if (!listener || !listener->desc || (listener->desc->connected != CON_PLYNG)) continue;
    if ((listener == ch) || !IS_SET(GET_PFLAG(listener), channel_info[comm].channel_flag)) continue;

    /* Show the text to the listener. */
    COLOR(listener, channel_info[comm].channel_color);
    act(buf, PERS_MORTAL, ch, 0, listener, TO_VICT);
    ENDCOLOR(listener);
  }
}


/* Function called by the 'gossip' command. */
void do_gossip(CHAR *ch, char *arg, int cmd) {
  channel_comm(ch, arg, CHANNEL_COMM_GOSSIP);
}


/* Function called by the 'auction' command. */
void do_auction(CHAR *ch, char *arg, int cmd) {
  channel_comm(ch, arg, CHANNEL_COMM_AUCTION);
}


/* Function called by the 'chaos' command. */
void do_chaos(CHAR *ch, char *arg, int cmd) {
  channel_comm(ch, arg, CHANNEL_COMM_CHAOS);
}


/* Function called by the 'channel' command. */
void do_channel(CHAR *ch, char *arg, int cmd) {
  char buf[MSL];
  DESC *desc = NULL;
  CHAR *player = NULL;

  one_argument(arg, buf);

  /* Print channel on/off status if no argument is provided. */
  if (!*buf) {
    if IS_SET(ch->specials.pflag, PLR_GOSSIP) {
      send_to_char("Gossip is ON\n\r", ch);
    }
    else {
      send_to_char("Gossip is OFF\n\r", ch);
    }

    if IS_SET(ch->specials.pflag, PLR_AUCTION) {
      send_to_char("Auction is ON\n\r", ch);
    }
    else {
      send_to_char("Auction is OFF\n\r", ch);
    }

    if IS_SET(ch->specials.pflag, PLR_CHAOS) {
      send_to_char("Chaos is ON\n\r", ch);
    }
    else {
      send_to_char("Chaos is OFF\n\r", ch);
    }

    if IS_SET(ch->specials.pflag, PLR_QUESTC) {
      send_to_char("Quest is ON\n\r", ch);
    }
    else {
      send_to_char("Quest is OFF\n\r", ch);
    }
  }
  else
    if (is_abbrev(buf, "gossip")) {
      send_to_char("The players in Gossip Channel are:\n\r", ch);

      /* Loop through the descriptors in the game and show visibile players in the channel. */
      for (desc = descriptor_list; desc; desc = desc->next) {
        player = desc->character;

        if (!player) continue;

        if ((IS_MORTAL(ch) || (IS_IMMORTAL(ch) && !WIZ_INV(player, ch)) || CAN_SEE(player, ch)) &&
          CHAR_REAL_ROOM(player) != NOWHERE &&
          IS_SET(player->specials.pflag, PLR_GOSSIP)) {
          sprintf(buf, "%s\n\r", GET_NAME(player));
          send_to_char(buf, ch);
        }
      }
    }
    else
      if (is_abbrev(buf, "auction")) {
        send_to_char("The players in Auction Channel are:\n\r", ch);

        /* Loop through the descriptors in the game and show visibile players in the channel. */
        for (desc = descriptor_list; desc; desc = desc->next) {
          player = desc->character;

          if (!player) continue;

          if ((IS_MORTAL(ch) || (IS_IMMORTAL(ch) && !WIZ_INV(player, ch)) || CAN_SEE(player, ch)) &&
            CHAR_REAL_ROOM(player) != NOWHERE &&
            IS_SET(player->specials.pflag, PLR_AUCTION)) {
            sprintf(buf, "%s\n\r", GET_NAME(player));
            send_to_char(buf, ch);
          }
        }
      }
      else
        if (is_abbrev(buf, "chaos")) {
          send_to_char("The players in Chaos Channel are:\n\r", ch);

          /* Loop through the descriptors in the game and show visibile players in the channel. */
          for (desc = descriptor_list; desc; desc = desc->next) {
            player = desc->character;

            if (!player) continue;

            if ((IS_MORTAL(ch) || (IS_IMMORTAL(ch) && !WIZ_INV(player, ch)) || CAN_SEE(player, ch)) &&
              CHAR_REAL_ROOM(player) != NOWHERE &&
              IS_SET(player->specials.pflag, PLR_CHAOS)) {
              sprintf(buf, "%s\n\r", GET_NAME(player));
              send_to_char(buf, ch);
            }
          }
        }
        else
          if (is_abbrev(buf, "quest")) {
            send_to_char("The players in Quest Channel are:\n\r", ch);

            /* Loop through the descriptors in the game and show visibile players in the channel. */
            for (desc = descriptor_list; desc; desc = desc->next) {
              player = desc->character;

              if (!player) continue;

              if ((IS_MORTAL(ch) || (IS_IMMORTAL(ch) && !WIZ_INV(player, ch)) || CAN_SEE(player, ch)) &&
                CHAR_REAL_ROOM(player) != NOWHERE &&
                IS_SET(player->specials.pflag, PLR_QUESTC)) {
                sprintf(buf, "%s\n\r", GET_NAME(player));
                send_to_char(buf, ch);
              }
            }
          }
          else /* Non-existent channel. */
          {
            send_to_char("Which channel are you looking for?\n\r", ch);
          }
}


/* Function called by the 'tell' command. */
void do_tell(CHAR *ch, char *arg, int cmd) {
  /* NoShouted players can't tell. */
  if (!IS_NPC(ch) && IS_SET(GET_PFLAG(ch), PLR_NOSHOUT)) {
    printf_to_char(ch, "You have displeased the gods; you cannot speak.\n\r");

    return;
  }

  /* Players with NoMessage on can't tell. */
  if (!IS_NPC(ch) && IS_SET(GET_PFLAG(ch), PLR_NOMESSAGE)) {
    printf_to_char(ch, "You can't tell when you have NoMessage turned on.\n\r");

    return;
  }

  char name[MIL], message[MIL];

  half_chop(arg, name, sizeof(name), message, sizeof(message));

  /* Tell was used without any arguments. */
  if (!*name || !*message) {
    printf_to_char(ch, "Who do you wish to tell, and what?\n\r");

    return;
  }

  /* Get the listening character. */
  CHAR *listener = get_ch_by_name(name);

  /* Redirect listener if they're mobswitched. */
  if (listener->switched) {
    listener = listener->switched;
  }

  /* We didn't find anyone, or the listener is not visible to the acting character. */
  if (!listener || WIZ_INV(ch, listener)) {
    printf_to_char(ch, "No one by that name here...\n\r");

    return;
  }

  /* Check if the listener is disconnected. */
  if (!GET_DESCRIPTOR(listener) || GET_DESCRIPTOR(listener)->connected) {
    act_f(PERS_MORTAL, ch, 0, listener, TO_CHAR, "$E isn't connected at the moment.");

    return;
  }

  /* Players can't reply through NoMessage. */
  if (IS_MORTAL(ch) && IS_SET(GET_PFLAG(listener), PLR_NOMESSAGE)) {
    act_f(PERS_MORTAL, ch, 0, listener, TO_CHAR, "$E can't hear you.");

    return;
  }

  /* Players can't reply through NoShout. */
  if (IS_MORTAL(ch) && IS_SET(GET_PFLAG(listener), PLR_NOSHOUT)) {
    act_f(PERS_MORTAL, ch, 0, listener, TO_CHAR, "The gods have taken away $N's ability to communicate.");

    return;
  }

  /* Show the text to the acting character. */
  COLOR(ch, 6);
  act_f(PERS_MORTAL, ch, 0, listener, TO_CHAR, "You tell $N, '%s'", message);
  ENDCOLOR(ch);

  /* Show the text to the listening character. */
  COLOR(listener, 6);
  act_f(PERS_MORTAL, ch, 0, listener, TO_VICT, "$n tells you, '%s'", message);
  ENDCOLOR(listener);

  /* Update the listening character's reply_to variable. */
  if (!IS_NPC(ch)) {
    GET_REPLY_TO(listener) = GET_ID(ch);
  }

  signal_char(listener, ch, MSG_TOLD, arg);
}


/* Function called by the 'reply' command. */
void do_reply(CHAR *ch, char *arg, int cmd) {
  if (IS_NPC(ch)) {
    send_to_char("NPCs can't reply.\n\r", ch);

    return;
  }

  /* NoShouted players can't reply. */
  if (!IS_NPC(ch) && IS_SET(GET_PFLAG(ch), PLR_NOSHOUT)) {
    send_to_char("You have displeased the gods; you cannot speak.\n\r", ch);

    return;
  }

  /* Players with NoMessage on can't reply. */
  if (!IS_NPC(ch) && IS_SET(GET_PFLAG(ch), PLR_NOMESSAGE)) {
    send_to_char("You can't reply when you have NoMessage turned on.\n\r", ch);

    return;
  }

  /* The acting character doesn't have anyone to reply to. */
  if (!GET_REPLY_TO(ch)) {
    send_to_char("There is no one to reply to.\n\r", ch);

    return;
  }

  arg = skip_spaces(arg);

  /* Reply was used without any arguments. */
  if (!*arg) {
    send_to_char("Yes, but what do you want to reply?\n\r", ch);

    return;
  }

  char message[MIL];

  /* Store the message. */
  snprintf(message, sizeof(message), "%s", arg);

  /* Attempt to locate the listener. */
  CHAR *listener = get_ch_by_id(GET_REPLY_TO(ch));

  /* No listener was found. */
  if (!listener) {
    send_to_char("There is nobody to reply to...\n\r", ch);

    return;
  }

  /* Redirect listener if they're mobswitched. */
  if (listener->switched) {
    listener = listener->switched;
  }

  /* Check if the listener is disconnected. */
  if (!GET_DESCRIPTOR(listener) || GET_DESCRIPTOR(listener)->connected) {
    act("$E isn't connected at the moment.", PERS_MORTAL, ch, 0, listener, TO_CHAR);

    return;
  }

  /* NPCs/Players can't reply through NoMessage. */
  if (IS_MORTAL(ch) && IS_SET(GET_PFLAG(listener), PLR_NOMESSAGE)) {
    act("$E can't hear you.", PERS_MORTAL, ch, 0, listener, TO_CHAR);

    return;
  }

  /* Players can't reply through NoShout. */
  if (IS_MORTAL(ch) && IS_SET(GET_PFLAG(listener), PLR_NOSHOUT)) {
    act("The gods have taken away $N's ability to communicate.", PERS_MORTAL, ch, 0, listener, TO_CHAR);

    return;
  }

  char buf[MSL];

  /* Build the buffer for the acting player. */
  snprintf(buf, sizeof(buf), "You reply to $N, '%s'", message);

  /* Show the text to the acting character. */
  COLOR(ch, 6);
  act(buf, PERS_MORTAL, ch, 0, listener, TO_CHAR);
  ENDCOLOR(ch);

  /* Build the buffer for the listening character. */
  snprintf(buf, sizeof(buf), "$n replies, '%s'", message);

  /* Show the text to the listening character. */
  COLOR(listener, 6);
  act(buf, PERS_MORTAL, ch, 0, listener, TO_VICT);
  ENDCOLOR(listener);

  /* Update the listening character's reply_to variable. */
  if (!IS_NPC(ch)) {
    GET_REPLY_TO(listener) = GET_ID(ch);
  }
}


/* Function called by the 'gtell' command.
   Re-written by Night, 12/10/2011 */
void do_gtell(CHAR *ch, char *arg, int cmd) {
  /* NoShouted players can't gtell. */
  if (!IS_NPC(ch) && IS_SET(GET_PFLAG(ch), PLR_NOSHOUT)) {
    send_to_char("You have displeased the gods; you cannot speak.\n\r", ch);

    return;
  }

  arg = skip_spaces(arg);

  if (!*arg) {
    send_to_char("Yes, but what do you want to tell your group?\n\r", ch);

    return;
  }

  char buf[MSL];

  /* Build the buffer for the acting character. */
  snprintf(buf, sizeof(buf), "You tell your group, '%s'", arg);

  /* Show the text to the acting character. */
  COLOR(ch, 10);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  ENDCOLOR(ch);

  /* Get the group leader, or the acting character if no leader. */
  CHAR *group_leader = (GET_MASTER(ch) ? GET_MASTER(ch) : ch);

  /* Show the text to the group leader if it's not the acting character. */
  if ((group_leader != ch) && IS_AFFECTED(group_leader, AFF_GROUP)) {
    /* Build the buffer for the goup leader. */
    snprintf(buf, sizeof(buf), "** %s tells you, '%s'", (!IS_NPC(ch) ? (WIZ_INV(group_leader, ch) ? "Somebody" : GET_NAME(ch)) : GET_SHORT(ch)), arg);

    COLOR(group_leader, 10);
    act(buf, FALSE, ch, 0, group_leader, TO_VICT);
    ENDCOLOR(group_leader);
  }

  /* Loop through all of the group members of group_leader's group and show them the text. */
  for (FOL *follower = group_leader->followers; follower; follower = follower->next) {
    CHAR *group_member = follower->follower;

    if (!group_member || (group_member == ch) || !IS_AFFECTED(group_member, AFF_GROUP)) continue;

    /* Build the buffer for the group member. */
    snprintf(buf, sizeof(buf), "** %s tells you, '%s'", (!IS_NPC(ch) ? (WIZ_INV(group_member, ch) ? "Somebody" : GET_NAME(ch)) : GET_SHORT(ch)), arg);

    COLOR(group_member, 10);
    act(buf, FALSE, ch, 0, group_member, TO_VICT);
    ENDCOLOR(group_member);
  }
}


/* Function called by the 'ask' command.
   Re-written by Night, 12/10/2011 */
void do_ask(CHAR *ch, char *arg, int cmd) {
  char buf[MSL];
  char name[20];
  char message[MSL];
  CHAR *temp_ch = NULL;
  CHAR *listener = NULL;

  /* NoShouted players can't use ask. */
  if (!IS_NPC(ch) &&
      IS_SET(GET_PFLAG(ch), PLR_NOSHOUT)) {
    send_to_char("You have displeased the gods; you cannot speak.\n\r", ch);

    return;
  }

  /* Players with NoMessage on can't use ask. */
  if (!IS_NPC(ch) &&
      IS_SET(GET_PFLAG(ch), PLR_NOMESSAGE)) {
    send_to_char("You can't ask anything when you have NoMessage turned on.\n\r", ch);

    return;
  }

  half_chop(arg, name, sizeof(name), message, sizeof(message));

  /* Ask was used without any arguments. */
  if (!*name || !*message) {
    send_to_char("Who do you wish to ask, and what?\n\r", ch);

    return;
  }

  temp_ch = get_char_room_vis(ch, name);

  /* We didn't find anyone, or the listener is not visible to the acting character. */
  if (!temp_ch) {
    send_to_char("No one by that name here...\n\r", ch);

    return;
  }

  /* Allow asking a switched person to go to the mob. */
  if (temp_ch->switched) {
    listener = temp_ch->switched;
  }
  else {
    listener = temp_ch;
  }

  /* Various conditions that might prevent a character from using ask.
     Here we're building the buffer for the acting character. */
  if ((IS_NPC(ch) || IS_MORTAL(ch)) &&
      IS_SET(GET_PFLAG(listener), PLR_NOMESSAGE)) {
    /* NPCs/Players can't ask through NoMessage. */
    act("$E can't hear you.", FALSE, ch, 0, listener, TO_CHAR);
    return;
  }

  if ((IS_NPC(ch) || IS_MORTAL(ch)) &&
      IS_SET(GET_PFLAG(listener), PLR_NOSHOUT)) {
    /* NPCs/Players can't ask through NoShout. */
    act("The gods have taken away $N's ability to communicate.", FALSE, ch, 0, listener, TO_CHAR);
    return;
  }

  if ((!IS_NPC(ch) && !IS_MORTAL(ch)) &&
      IS_SET(GET_PFLAG(listener), PLR_NOMESSAGE) &&
      !IS_SET(GET_PFLAG(listener), PLR_NOSHOUT)) {
    /* Immortals can ask through NoMessage. */
    sprintf(buf, "You ask $N (deaf), '%s'", message);
  }
  else if ((!IS_NPC(ch) && !IS_MORTAL(ch)) &&
            !IS_SET(GET_PFLAG(listener), PLR_NOMESSAGE) &&
            IS_SET(GET_PFLAG(listener), PLR_NOSHOUT)) {
    /* Immortals can ask through NoShout. */
    sprintf(buf, "You ask $N (noshout), '%s'", message);
  }
  else if ((!IS_NPC(ch) && !IS_MORTAL(ch)) &&
           IS_SET(GET_PFLAG(listener), PLR_NOMESSAGE) &&
            IS_SET(GET_PFLAG(listener), PLR_NOSHOUT)) {
    /* Immortals can ask through NoMessage & NoShout. */
    sprintf(buf, "You ask $N (deaf & noshout), '%s'", message);
  }
  else {
    /* Everything worked out. */
    sprintf(buf, "You ask $N, '%s'", message);
  }

  /* Show the text to the acting character. */
  act(buf, 0, ch, 0, listener, TO_CHAR);

  /* Build the buffer for the listening player. */
  sprintf(buf, "$n asks you '%s'", message);

  /* Show the text to the listening player. */
  act(buf, 0, ch, 0, listener, TO_VICT);

  /* Show some text to the room. */
  if (listener == ch) {
    act("$n quietly asks $mself a question.", FALSE, ch, 0, 0, TO_ROOM);
  }
  else {
    act("$n asks $N a question.", FALSE, ch, 0, listener, TO_NOTVICT);
  }

  /* Update the listening player's reply_to variable. */
  if (!IS_NPC(ch)) {
    listener->specials.reply_to = GET_ID(ch);
  }
}


/* Function called by the 'whisper' command.
   Re-written by Night, 12/10/2011 */
void do_whisper(CHAR *ch, char *arg, int cmd) {
  char buf[MSL];
  char name[20];
  char message[MSL];
  CHAR *temp_ch = NULL;
  CHAR *listener = NULL;

  /* NoShouted players can't use whisper. */
  if (!IS_NPC(ch) &&
    IS_SET(ch->specials.pflag, PLR_NOSHOUT)) {
    send_to_char("You have displeased the gods, you cannot speak.\n\r", ch);

    return;
  }

  /* Players with NoMessage on can't use whisper. */
  if (!IS_NPC(ch) &&
    IS_SET(ch->specials.pflag, PLR_NOMESSAGE)) {
    send_to_char("You can't whisper when you have NoMessage turned on.\n\r", ch);

    return;
  }

  half_chop(arg, name, sizeof(name), message, sizeof(message));

  /* Whisper was used without any arguments. */
  if (!*name ||
    !*message) {
    send_to_char("Who do you wish to whisper, and what?\n\r", ch);

    return;
  }

  temp_ch = get_char_room_vis(ch, name);

  /* We didn't find anyone, or the listener is not visible to the acting character. */
  if (!temp_ch) {
    send_to_char("No one by that name here...\n\r", ch);

    return;
  }

  /* Allow a whisper to a switched person to go to the mob. */
  if (temp_ch->switched) {
    listener = temp_ch->switched;
  }
  else {
    listener = temp_ch;
  }

  /* Various conditions that might prevent a character from using whisper.
     Here we're building the buffer for the acting character. */
  if ((IS_NPC(ch) || IS_MORTAL(ch)) &&
      IS_SET(listener->specials.pflag, PLR_NOMESSAGE)) {
    /* NPCs/Players can't whisper through NoMessage. */
    act("$E can't hear you.", FALSE, ch, 0, listener, TO_CHAR);
    return;
  }

  if ((IS_NPC(ch) || IS_MORTAL(ch)) &&
      IS_SET(listener->specials.pflag, PLR_NOSHOUT)) {
    /* NPCs/Players can't whisper through NoShout. */
    act("The gods have taken away $N's ability to communicate.", FALSE, ch, 0, listener, TO_CHAR);
    return;
  }

  if ((!IS_NPC(ch) && !IS_MORTAL(ch)) &&
      IS_SET(listener->specials.pflag, PLR_NOMESSAGE) &&
      !IS_SET(listener->specials.pflag, PLR_NOSHOUT)) {
    /* Immortals can whisper through NoMessage. */
    sprintf(buf, "You whisper to $N (deaf), '%s'", message);
  }
  else if ((!IS_NPC(ch) && !IS_MORTAL(ch)) &&
           !IS_SET(listener->specials.pflag, PLR_NOMESSAGE) &&
           IS_SET(listener->specials.pflag, PLR_NOSHOUT)) {
    /* Immortals can whisper through NoShout. */
    sprintf(buf, "You whisper to $N (noshout), '%s'", message);
  }
  else if ((!IS_NPC(ch) && !IS_MORTAL(ch)) &&
           IS_SET(listener->specials.pflag, PLR_NOMESSAGE) &&
           IS_SET(listener->specials.pflag, PLR_NOSHOUT)) {
    /* Immortals can whisper through NoMessage & NoShout. */
    sprintf(buf, "You whisper to $N (deaf & noshout), '%s'", message);
  }
  else {
    /* Everything worked out. */
    sprintf(buf, "You whisper to $N, '%s'", message);
  }

  /* Show the text to the acting character. */
  act(buf, 0, ch, 0, listener, TO_CHAR);

  /* Build the buffer for the listening player. */
  sprintf(buf, "$n whispers to you '%s'", message);

  /* Show the text to the listening player. */
  act(buf, 0, ch, 0, listener, TO_VICT);

  /* Show some text to the room. */
  if (listener == ch) {
    act("$n quietly asks $mself a question.", FALSE, ch, 0, 0, TO_ROOM);
  }
  else {
    act("$n whispers something to $N.", FALSE, ch, 0, listener, TO_NOTVICT);
  }

  /* Update the listening player's reply_to variable. */
  if (!IS_NPC(ch)) {
    listener->specials.reply_to = ch->ver3.id;
  }
}


/* Arbitrary */
#define MAX_NOTE_LENGTH 2000

/* Function called by the 'write' command.
   Re-written by Night, 12/11/2011 */
void do_write(CHAR *ch, char *arg, int cmd) {
  char buf[MSL];
  char first_arg[MIL];
  char second_arg[MIL];
  OBJ *note = NULL;
  OBJ *pen = NULL;

  /* Character does not have a descriptor so they can't write. */
  if (!ch->desc) {
    return;
  }

  argument_interpreter(arg, first_arg, second_arg);

  /* Write command was called with no argument. */
  if (!*first_arg) {
    send_to_char("Write? On what? With what? What are you trying to do?\n\r", ch);

    return;
  }

  /* The first argument was provided. This indicates the note to be written on. */
  if (*first_arg) {
    /* Check the character's inventory for a visible object matching the first argument provided. */
    if (!(note = get_obj_in_list_vis(ch, first_arg, ch->carrying))) {
      /* If nothing was found in the character's inventory, try the object held by the character. */
      if (!(note = EQ(ch, HOLD)) || !isname(first_arg, OBJ_NAME(note))) {
        /* No object of the name provided was found. */
        sprintf(buf, "You have no %s.\n\r", first_arg);
        send_to_char(buf, ch);

        return;
      }
    }
  }

  /* The second argument was provided. This indicates the pen to write with. */
  if (*second_arg) {
    /* Check the character's inventory for a visible object matching the second argument provided. */
    if (!(pen = get_obj_in_list_vis(ch, second_arg, ch->carrying))) {
      /* If nothing was found in the character's inventory, try the object held by the character. */
      if (!(pen = EQ(ch, HOLD)) || !isname(second_arg, OBJ_NAME(pen))) {
        /* No object of the name provided was found. */
        sprintf(buf, "You have no %s.\n\r", second_arg);
        send_to_char(buf, ch);

        return;
      }
    }
  }

  /* An object was found to write on but no argument was provided to indicate a writing implement.
     The writing implement might be held by the character. */
  if (note && !*second_arg && !(pen = EQ(ch, HOLD))) {
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
    if (note->action_description) {
      send_to_char("There's something written on it already.\n\r", ch);

      return;
    }

    /* Everything checked out. The character can write a note. */
    send_to_char("Ok, go ahead and write; end the note with an @ symbol on a new line.\n\r", ch);
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);

    SET_BIT(GET_PFLAG(ch), PLR_WRITING);
    ch->desc->str = &note->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}

/* For special usage. Accepts printf-style args and bypasses tell restrictions. */
void tell_special(CHAR *ch, CHAR *listener, char *message, ...) {
  if (!ch || !listener || !message || !*message) return;

  /* Redirect listener if thye're mobswitched. */
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

  /* Show the text to the acting character. */
  COLOR(ch, 6);
  act_f(PERS_MORTAL, ch, 0, listener, TO_CHAR, "You tell $N, '%s'", buf);
  ENDCOLOR(ch);

  /* Show the text to the listening character. */
  COLOR(listener, 6);
  act_f(PERS_MORTAL, ch, 0, listener, TO_VICT, "$n tells you, '%s'", buf);
  ENDCOLOR(listener);

  /* Update the listening character's reply_to variable. */
  if (!IS_NPC(ch)) {
    GET_REPLY_TO(listener) = GET_ID(ch);
  }
}
