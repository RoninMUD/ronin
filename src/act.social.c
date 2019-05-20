/* ************************************************************************
*  file: act.social.c , Implementation of commands.       Part of DIKUMUD *
*  Usage : Social commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "cmd.h"
#include "utility.h"
#include "modify.h"
#include "utils.h"

/* Global Data */
int social_list_top;

/* extern functions */
void parse_string(char *input, char *output, struct char_data *ch1,struct char_data *ch2, struct char_data *to);
int action(int cmd);
char *fread_action(FILE *fl);
extern char *skip_spaces(char *string);

#define MAX_SOCIALS 500

struct social_messg
{
  int act_nr;
  int hide;
  int min_victim_position; /* Position of victim */

  /* No argument was supplied */
  char *cmd_text;
  char *char_no_arg;
  char *others_no_arg;

  /* An argument was there, and a victim was found */
  char *char_found;    /* if NULL, read no further, ignore args */
  char *others_found;
  char *vict_found;

  /* An argument was there, but no victim was found */
  char *not_found;

  /* The victim turned out to be the character */
  char *char_auto;
  char *others_auto;
} soc_mess_list[MAX_SOCIALS];



struct pose_type
{
  int level;
  char *poser_msg[11];
  char *room_msg[11];
} pose_messages[MAX_MESSAGES];



char *fread_action(FILE *fl)
{
  char buf[MAX_STRING_LENGTH], *rslt;

  for (;;)
  {
    if(!fgets(buf, MAX_STRING_LENGTH, fl)) {
      log_s("Fread_action - unexpected EOF.");
      produce_core();
    }

    if (*buf == '#')
      return(0);
    {
      *(buf + strlen(buf) - 1) = '\0';
      CREATE(rslt, char, strlen(buf) + 1);
      strcpy(rslt, buf);
      return(rslt);
    }
  }
}

void boot_social_messages(void) {
  FILE *fl;
  int hide, min_pos;
  char buf1[MAX_INPUT_LENGTH];

  if (!(fl = fopen(SOCMESS_FILE, "r"))) {
    log_f("boot_social_messages");
    produce_core();
  }
 social_list_top=0;

  while(!feof(fl)) {
    if(EOF == fscanf(fl, " %s ", buf1)) break;
    if(!strcmp(buf1,"-1")) break;
    soc_mess_list[social_list_top].cmd_text = str_dup(buf1);
    if(EOF == fscanf(fl, " %d ", &hide)) break;
    if(EOF == fscanf(fl, " %d \n", &min_pos)) break;

    /* read the stuff */
    soc_mess_list[social_list_top].hide = hide;
    soc_mess_list[social_list_top].min_victim_position = min_pos;

    soc_mess_list[social_list_top].char_no_arg   = fread_action(fl);
    soc_mess_list[social_list_top].others_no_arg = fread_action(fl);
    soc_mess_list[social_list_top].char_found    = fread_action(fl);

    /* if no char_found, the rest is to be ignored */
    if(soc_mess_list[social_list_top].char_found) {
       soc_mess_list[social_list_top].others_found  = fread_action(fl);
       soc_mess_list[social_list_top].vict_found    = fread_action(fl);
       soc_mess_list[social_list_top].not_found     = fread_action(fl);
       soc_mess_list[social_list_top].char_auto     = fread_action(fl);
       soc_mess_list[social_list_top].others_auto   = fread_action(fl);
    }
    social_list_top++;
    if(social_list_top>=MAX_SOCIALS) {
      log_f("Maximum number of socials reached.");
      break;
    }
  }
  fclose(fl);
}

int find_action_ex(char *arg, size_t len, bool ignore_case) {
  if (!arg || !*arg || !len) return -1;

  for (int i = 0; soc_mess_list[i].cmd_text; i++) {
    if ((ignore_case && !strncasecmp(soc_mess_list[i].cmd_text, arg, len)) ||
        (!ignore_case && !strncmp(soc_mess_list[i].cmd_text, arg, len))) {
      return i;
    }
  }

  return -1;
}

int find_action(char *arg, size_t len) {
  return find_action_ex(arg, len, FALSE);
}

void list_socials(struct char_data *ch) {
  struct string_block sb;
  char buf[MAX_INPUT_LENGTH];
  int i=0,count=0;

  init_string_block(&sb);
  append_to_string_block(&sb,"\n\r`iHere are a list of current socials on RoninMUD\n\r------------------------------------------------------------------------------`q\n\r\n\r");

  for(i=0;i<social_list_top;i++) {
    count++;
    if (count>5) { /* 6 socials to a line @ 13 spacing = 78 chars*/
      count=0;
      append_to_string_block(&sb,"\n\r");
    }
    sprintf(buf,"%-11s  ",soc_mess_list[i].cmd_text);
    append_to_string_block(&sb, buf);
  }
  append_to_string_block(&sb,"\n\r`i------------------------------------------------------------------------------`q\n\r\n\r");
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}


#define SOCIAL_MODE_NONE 0
#define SOCIAL_MODE_CHAR 1

void do_gf(CHAR *ch, char *arg, int cmd) {
  if (!ch || !arg) return;

  if (!IS_NPC(ch) && IS_SET(GET_PFLAG(ch), PLR_NOSHOUT)) {
    send_to_char("You have displeased the gods, you cannot use the gossip channel.\n\r", ch);

    return;
  }

  if (!IS_NPC(ch) && IS_SET(GET_PFLAG(ch), PLR_QUEST) && IS_SET(GET_PFLAG(ch), PLR_QUIET)) {
    send_to_char("The Questmaster has taken away your ability to interrupt.\n\r", ch);

    return;
  }

  if (!IS_NPC(ch) && !IS_SET(GET_PFLAG(ch), PLR_GOSSIP)) {
    SET_BIT(GET_PFLAG(ch), PLR_GOSSIP);

    send_to_char("You turn ON the gossip channel.\n\r", ch);
  }

  char action[MIL], name[MIL];

  two_arguments(arg, action, name);

  int action_index = find_action_ex(action, strlen(action), TRUE);

  if (action_index < 0) {
    send_to_char("Want to try that again?\n\r", ch);

    return;
  }

  SOC *social = &soc_mess_list[action_index];

  CHAR *victim = NULL;

  if (*name) {
    victim = get_char_ex(ch, name, FALSE);

    if (!victim || WIZ_INV(ch, victim)) {
      send_to_char("They aren't here.\n\r", ch);

      return;
    }
  }

  char buf[MIL], message[MSL];

  int type = 0;

  if (!*name && !social->others_no_arg) {
    snprintf(buf, sizeof(buf), "%s", social->char_no_arg);

    type = TO_CHAR;
  }
  else {
    for (DESC *temp_desc = descriptor_list; temp_desc; temp_desc = temp_desc->next) {
      CHAR *temp_ch = temp_desc->character;

      if (temp_desc->connected || !temp_ch || !IS_SET(GET_PFLAG(temp_ch), PLR_GOSSIP)) continue;

      if ((victim == ch) && social->others_auto) {
        snprintf(buf, sizeof(buf), "%s", social->others_auto);
      }
      else if (!*name && social->others_no_arg) {
        snprintf(buf, sizeof(buf), "%s", social->others_no_arg);
      }
      else if ((victim == temp_ch) && social->vict_found) {
        snprintf(buf, sizeof(buf), "%s", social->vict_found);

        type = TO_VICT;
      }
      else if (social->others_found) {
        snprintf(buf, sizeof(buf), "%s", social->others_found);

        type = TO_OTHER;
      }

      snprintf(message, sizeof(message), "[gossip] %s", buf);

      COLOR(temp_ch, 5);
      if (type == TO_VICT) {
        act(message, PERS_MORTAL, ch, 0, victim, TO_VICT);
      }
      else if (type == TO_OTHER) {
        act(message, PERS_MORTAL, ch, temp_ch, victim, TO_OTHER);
      }
      else {
        act(message, PERS_MORTAL, ch, 0, temp_ch, TO_VICT);
      }
      ENDCOLOR(temp_ch);

      type = 0;
    }

    if ((victim == ch) && social->char_auto) {
      snprintf(buf, sizeof(buf), "%s", social->char_auto);
    }
    else if (*name && social->char_found) {
      snprintf(buf, sizeof(buf), "%s", social->char_found);
    }
    else if (social->char_no_arg) {
      snprintf(buf, sizeof(buf), "%s", social->char_no_arg);
    }
  }

  snprintf(message, sizeof(message), "[gossip] %s", buf);

  COLOR(ch, 5);
  if (type == TO_CHAR) {
    act(message, FALSE, ch, 0, 0, TO_CHAR);
  }
  else {
    act(message, FALSE, ch, 0, victim, TO_CHAR);
  }
  ENDCOLOR(ch);
}

void do_wizact(CHAR *ch, char *arg, int cmd)
{
  char buf[MIL];
  char buf2[MIL];
  char name[MIL];
  char *command = NULL;
  char *args = NULL;
  int level = 0;
  int index = 0;
  int style = 0;
  SOC *action = NULL;
  CHAR *vict = NULL;
  CHAR *tmp_vict = NULL;
  DESC *tmp_desc = NULL;

  memset(buf, 0, sizeof(buf));
  memset(buf2, 0, sizeof(buf));
  memset(name, 0, sizeof(name));

  level = ch->specials.wiznetlvl;

  arg = skip_spaces(arg);

  one_argument(arg, buf);

  if (is_number(buf))
  {
    level = atoi(buf);

    arg = one_argument(arg, buf);
  }

  /* Find first non blank */
  for (command = arg; *command == ' '; command++) /*continue*/;

  /* Find length of first word */
  for (args = command; *args > ' ' ; args++)
  {
     *(args) = LOWER(*args);/* Make all letters lower case AND find length */
  }

  index = find_action(command, args - command);

  if (index < 0)
  {
    send_to_char("Want to try that again?\n\r", ch);

    return;
  }

  action = &soc_mess_list[index];

  buf[0] = '\0';

  half_chop(arg, buf2, sizeof(buf2), buf, sizeof(buf));

  name[0] = '\0';

  if (action->char_found)
  {
    one_argument(buf, name);
  }
  else
  {
    name[0] = '\0';
  }

  if (name[0] == ' ')
  {
    name[0] = '\0';
  }

  if (name[0] != '\0')
  {
    vict = get_char_vis(ch, name);

    if (!vict)
    {
      send_to_char("They aren't here.\n\r", ch);
      return;
    }
  }

  if (!name[0] &&
      !action->others_no_arg)
  {
    strncpy(buf2, action->char_no_arg, sizeof(buf2));
    style = 1;
  }
  else
  {
    for (tmp_desc = descriptor_list; tmp_desc; tmp_desc = tmp_desc->next)
    {
      tmp_vict = tmp_desc->character;

      if (!tmp_vict ||
        tmp_vict->desc->connected ||
        GET_LEVEL(tmp_vict) < LEVEL_IMM ||
        GET_LEVEL(tmp_vict) < level)
      {
        continue;
      }

      if (action->char_found &&
        vict == ch)
      {
        sprintf(buf2, "%s", action->others_auto);
      }
      else
      if (!name[0])
      {
        sprintf(buf2, "%s", action->others_no_arg);
      }
      else if (action->char_found &&
               vict == tmp_vict)
      {
        sprintf(buf2, "%s", action->vict_found);
        style = 2;
      }
      else
      {
        sprintf(buf2, "%s", action->others_found);
        style = 3;
      }

      if (level > LEVEL_IMM &&
        level <= LEVEL_IMP)
      {
        sprintf(buf, "[wiz] (%d) %s", level, buf2);
      }
      else
      {
        sprintf(buf, "[wiz] %s", buf2);
      }

      COLOR(tmp_vict, 10);
      if (style == 2)
      {
        act(buf, FALSE, ch, NULL, vict, TO_VICT);
      }
      else if (style == 3)
      {
        act(buf, FALSE, ch, tmp_vict, vict, TO_OTHER);
      }
      else
      {
        act(buf, FALSE, ch, NULL, tmp_vict, TO_VICT);
      }
      ENDCOLOR(tmp_vict);

      style = 0;
    }

    if (vict == ch)
    {
      sprintf(buf2, "%s", action->char_auto);
    }
    else if (name[0] &&
             action->char_found)
    {
      sprintf(buf2, "%s", action->char_found);
    }
    else
    {
      sprintf(buf2, "%s", action->char_no_arg);
    }
  }

  if (level > LEVEL_IMM &&
      level <= LEVEL_IMP)
  {
    sprintf(buf, "[wiz] (%d) %s", level, buf2);
  }
  else
  {
    sprintf(buf, "[wiz] %s", buf2);
  }

  COLOR(ch, 10);
  if (style == 1)
  {
    act(buf, FALSE, ch, NULL, NULL, TO_CHAR);
  }
  else
  {
    act(buf, FALSE, ch, NULL, vict, TO_CHAR);
  }
  ENDCOLOR(ch);
}

void do_action(struct char_data *ch, char *argument, int cmd)
{
  int act_nr;
  char buf[MAX_INPUT_LENGTH];
  struct social_messg *action;
  struct char_data *vict;

  if ((act_nr = cmd) < 0)
  {
    return;
  }

  if (IS_SET(ch->specials.pflag, PLR_NOSHOUT) && !IS_NPC(ch))
  {
    send_to_char("You have displeased the gods, you cannot do actions.\n\r", ch);
    return;
  }

  if (IS_SET(ch->specials.pflag,PLR_QUEST) && 
      IS_SET(ch->specials.pflag,PLR_QUIET) && 
	  !IS_NPC(ch)) 
  {
    send_to_char("The Questmaster has taken away your ability to interrupt.\n\r",ch);
    return; /* For quests - Ranger June 96 */
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, QUIET) && 
      (GET_LEVEL(ch) < LEVEL_IMM)) 
  {
    send_to_char("A magical force prevents your actions.\n\r", ch);
    return; /* Ranger - July 96 */
  }

  action = &soc_mess_list[act_nr];

  if (action->char_found)
    one_argument(argument, buf);
  else
    *buf = '\0';

  if (!*buf) {
    send_to_char(action->char_no_arg, ch);
    send_to_char("\n\r", ch);
    act(action->others_no_arg, action->hide, ch, 0, 0, TO_ROOM);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char(action->not_found, ch);
    send_to_char("\n\r", ch);
    return;
  }

  if (vict == ch) {
    send_to_char(action->char_auto, ch);
    send_to_char("\n\r", ch);
    act(action->others_auto, action->hide, ch, 0, 0, TO_ROOM);
    return;
  }

  if((IS_AFFECTED(vict, AFF_HIDE)) &&
     !IS_AFFECTED2(ch, AFF2_PERCEIVE) && (GET_LEVEL(ch) < LEVEL_IMM)) {
    send_to_char(action->not_found, ch);
    send_to_char("\n\r", ch);
    return;
  }

  if (GET_POS(vict) < action->min_victim_position) {
    act("$N is not in a proper position for that.",FALSE,ch,0,vict,TO_CHAR);
  } else {
    act(action->char_found, 0, ch, 0, vict, TO_CHAR);
    act(action->others_found, action->hide, ch, 0, vict, TO_NOTVICT);
    act(action->vict_found, action->hide, ch, 0, vict, TO_VICT);
  }
}



void do_insult(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_STRING_LENGTH];
  struct char_data *victim;
  struct affected_type_5 af;

  if(IS_SET(ch->specials.pflag, PLR_NOSHOUT) && !IS_NPC(ch))
    {
    send_to_char("You have displeased the gods, you cannot insult.\n\r", ch);
    return;
  }

  one_argument(argument, arg);

  if(!*arg) {
    send_to_char("Sure you don't want to insult everybody.\n\r", ch);
    return;
  }
  if(!(victim = get_char_room_vis(ch, arg))) {
    send_to_char("Can't hear you!\n\r", ch);
    return;
  }

  if(victim==ch) {
    send_to_char("You feel insulted.\n\r", ch);
    return;
  }

  switch(number(0,5)) {
    case 0 :
      if (GET_SEX(ch) == SEX_MALE) {
        if (GET_SEX(victim) == SEX_MALE) {
          act("$n accuses you of fighting like a woman!", FALSE,ch, 0, victim, TO_VICT);
          act("You accuse $N of fighting like a woman!", FALSE,ch, 0, victim, TO_CHAR);
        }
        else {
          act("$n says that women can't fight.",FALSE, ch, 0, victim, TO_VICT);
          act("You say that women can't fight.",FALSE, ch, 0, victim, TO_CHAR);
        }
      } else { /* Ch == Woman */
        if (GET_SEX(victim) == SEX_MALE) {
          act("$n accuses you of having the smallest.... (brain?)",FALSE, ch, 0, victim, TO_VICT );
          act("You accuse $N of having the smallest.... (brain?)",FALSE, ch, 0, victim, TO_CHAR);
        }
        else {
          act("$n tells you that you'd lose a beautycontest against a troll.",FALSE, ch, 0, victim, TO_VICT );
          act("You tell $N that she'd lose a beautycontest against a troll.",FALSE, ch, 0, victim, TO_CHAR );
        }
      }
      break;
    case 1 :
      act("$n calls your mother a bitch!",FALSE, ch, 0, victim, TO_VICT );
      act("You call $N's mother a bitch!",FALSE, ch, 0, victim, TO_CHAR );
      break;
    case 2 :
      act("$n farts in your open mouth!",FALSE, ch, 0, victim, TO_VICT );
      act("You fart in $N's open mouth!",FALSE, ch, 0, victim, TO_CHAR );
      if (GET_LEVEL(ch) >= LEVEL_IMM) {
        if (!affected_by_spell(victim, SMELL_FARTMOUTH)) {
          af.type = SMELL_FARTMOUTH;
          af.duration = 5;
          af.modifier = 0;
          af.location = 0;
          af.bitvector = 0;
          af.bitvector2 = 0;
          if (GET_LEVEL(ch) < GET_LEVEL(victim))
            affect_to_char(ch, &af);
          else
            affect_to_char(victim, &af);
        }
      }
      break;
    case 3 :
      act("$n tells you 'May the fleas of a thousand camels infest your genitals!'",FALSE, ch, 0, victim, TO_VICT );
      act("You tell $N 'May the fleas of a thousand camels infest your genitals!'",FALSE, ch, 0, victim, TO_CHAR );
      break;
    case 4 :
      act("$n slaps your face and calls you $s bitch!",FALSE, ch, 0, victim, TO_VICT );
      act("You slap $N's face and call $M your bitch!",FALSE, ch, 0, victim, TO_CHAR );
      break;
    case 5 :
      act("$n tells you to get lost!",FALSE,ch,0,victim,TO_VICT);
      act("You tell $N to get lost!",FALSE,ch,0,victim,TO_CHAR);
      break;
    default:
      /* it'll never get here */
      break;
  } /* end switch */

  act("$n insults $N.", TRUE, ch, 0, victim, TO_NOTVICT);
}

void boot_pose_messages(void)
{
  FILE *fl;
  unsigned char  counter;
  unsigned char  class;

  if (!(fl = fopen(POSEMESS_FILE, "r")))
  {
    log_f("boot_pose_messages");
    produce_core();
  }

  for (counter = 0;;counter++)
  {
    if (EOF == fscanf(fl, " %d ", &pose_messages[counter].level)) break;
    if (pose_messages[counter].level < 0)
      break;
    for (class = 0;class < 11;class++)
    {
      pose_messages[counter].poser_msg[class] = fread_action(fl);
      pose_messages[counter].room_msg[class] = fread_action(fl);
    }
  }

  fclose(fl);
}

void do_pose(struct char_data *ch, char *argument, int cmd)
{
  int to_pose;
  int counter;

  if(IS_SET(ch->specials.pflag, PLR_NOSHOUT) && !IS_NPC(ch))
    {
    send_to_char("You have displeased the gods, you cannot do poses.\n\r", ch);
    return;
  }

  if ((GET_LEVEL(ch) < pose_messages[0].level) || IS_NPC(ch))
  {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  for (counter = 0; (pose_messages[counter].level < GET_LEVEL(ch)) &&
                    (pose_messages[counter].level > 0); counter++);
  counter--;

  to_pose = number(0, counter);

  act(pose_messages[to_pose].poser_msg[GET_CLASS(ch)-1], 0, ch, 0, 0, TO_CHAR);
  act(pose_messages[to_pose].room_msg[GET_CLASS(ch)-1], 0, ch, 0, 0, TO_ROOM);
}


int check_god_access(CHAR *ch, int active);

void sort_socials(void) {
  int issorted,i,hide,min_pos;
  char buf1[MIL],buf2[MIL];

  do {
    issorted = 1;
    for(i=0;i<social_list_top-1;i++) {
      if(str_cmp(soc_mess_list[i].cmd_text,soc_mess_list[i+1].cmd_text)>0) {
        sprintf(buf1,"%s",soc_mess_list[i].cmd_text);
        sprintf(buf2,"%s",soc_mess_list[i+1].cmd_text);
        soc_mess_list[i].cmd_text=str_dup(buf2);
        soc_mess_list[i+1].cmd_text=str_dup(buf1);

        hide=soc_mess_list[i].hide;
        soc_mess_list[i].hide=soc_mess_list[i+1].hide;
        soc_mess_list[i+1].hide=hide;

        min_pos=soc_mess_list[i].min_victim_position;
        soc_mess_list[i].min_victim_position=soc_mess_list[i+1].min_victim_position;
        soc_mess_list[i+1].min_victim_position=min_pos;

        sprintf(buf1,"%s",soc_mess_list[i].char_no_arg);
        sprintf(buf2,"%s",soc_mess_list[i+1].char_no_arg);
        soc_mess_list[i].char_no_arg=str_dup(buf2);
        soc_mess_list[i+1].char_no_arg=str_dup(buf1);

        sprintf(buf1,"%s",soc_mess_list[i].others_no_arg);
        sprintf(buf2,"%s",soc_mess_list[i+1].others_no_arg);
        soc_mess_list[i].others_no_arg=str_dup(buf2);
        soc_mess_list[i+1].others_no_arg=str_dup(buf1);

        sprintf(buf1,"%s",soc_mess_list[i].char_found);
        sprintf(buf2,"%s",soc_mess_list[i+1].char_found);
        soc_mess_list[i].char_found=str_dup(buf2);
        soc_mess_list[i+1].char_found=str_dup(buf1);

        sprintf(buf1,"%s",soc_mess_list[i].others_found);
        sprintf(buf2,"%s",soc_mess_list[i+1].others_found);
        soc_mess_list[i].others_found=str_dup(buf2);
        soc_mess_list[i+1].others_found=str_dup(buf1);

        sprintf(buf1,"%s",soc_mess_list[i].vict_found);
        sprintf(buf2,"%s",soc_mess_list[i+1].vict_found);
        soc_mess_list[i].vict_found=str_dup(buf2);
        soc_mess_list[i+1].vict_found=str_dup(buf1);

        sprintf(buf1,"%s",soc_mess_list[i].not_found);
        sprintf(buf2,"%s",soc_mess_list[i+1].not_found);
        soc_mess_list[i].not_found=str_dup(buf2);
        soc_mess_list[i+1].not_found=str_dup(buf1);

        sprintf(buf1,"%s",soc_mess_list[i].char_auto);
        sprintf(buf2,"%s",soc_mess_list[i+1].char_auto);
        soc_mess_list[i].char_auto=str_dup(buf2);
        soc_mess_list[i+1].char_auto=str_dup(buf1);

        sprintf(buf1,"%s",soc_mess_list[i].others_auto);
        sprintf(buf2,"%s",soc_mess_list[i+1].others_auto);
        soc_mess_list[i].others_auto=str_dup(buf2);
        soc_mess_list[i+1].others_auto=str_dup(buf1);
        issorted = 0;
      }
    }
  }
  while (!issorted);
}

void do_social(struct char_data *ch, char *argument, int cmd) {
  int i,act_nr,tmp=-1;
  char arg[MAX_INPUT_LENGTH];
  char usage[]="\
This command is used to manipulate the social file.\n\r\
  Usage: social list\n\r\
                show <name>\n\r\
                sort\n\r\
                create <name>\n\r\
                delete <name>\n\r\
                edit <name> <field> (for fields use edit <name>)\n\r\
                load\n\r\
                save\n\r";
  char edit_usage[]="\
social edit <hide/min/m1/m2/m3/m4/m5/m6/m7/m8> where:\n\r\
             hide <0/1>  - is the text shown if the character can't be seen 0=yes 1=no\n\r\
             min <pos #> - is the minimum position of the victim (OLCHELP POSITIONS)\n\r\
             m1 - Text to character if no argument\n\r\
             m2 - Text to others if no argument\n\r\
             m3 - Text to character if victim found\n\r\
             m4 - Text to others if victim found\n\r\
             m5 - Text to victim if victim found\n\r\
             m6 - Text to character if victim not found\n\r\
             m7 - Text to character if it is victim\n\r\
             m8 - Text to others if character is victim\n\r";
  struct social_messg *action;
  FILE *fl;

  if(!check_god_access(ch,TRUE)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_abbrev(arg, "list")) {
    list_socials(ch);
    return;
  }

  if(is_abbrev(arg, "show")) {
    one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
    if((act_nr=find_action(arg,strlen(arg)))<0) {
      printf_to_char(ch,"Could not find social: %s.\n\r",arg);
      return;
    }
    action=&soc_mess_list[act_nr];
    printf_to_char(ch,"Social text for: %s  Hide: %d  Min Position: %d\n\r",action->cmd_text,action->hide,action->min_victim_position);
    printf_to_char(ch,"Text to character if no argument:      %s\n\r",action->char_no_arg);
    printf_to_char(ch,"Text to others if no argument:         %s\n\r",action->others_no_arg);
    if(action->char_found) {
      printf_to_char(ch,"Text to character if victim found:     %s\n\r",action->char_found);
      printf_to_char(ch,"Text to others if victim found:        %s\n\r",action->others_found);
      printf_to_char(ch,"Text to victim if victim found:        %s\n\r",action->vict_found);
      printf_to_char(ch,"Text to character if victim not found: %s\n\r",action->not_found);
      printf_to_char(ch,"Text to character if it is victim:     %s\n\r",action->char_auto);
      printf_to_char(ch,"Text to others if character is victim: %s\n\r",action->others_auto);
    }
    else {
      printf_to_char(ch,"Text to character if victim found:     None.\n\r");
      printf_to_char(ch,"Text to others if victim found:        None.\n\r");
      printf_to_char(ch,"Text to victim if victim found:        None.\n\r");
      printf_to_char(ch,"Text to character if victim not found: None.\n\r");
      printf_to_char(ch,"Text to character if it is victim:     None.\n\r");
      printf_to_char(ch,"Text to others if character is victim: None.\n\r");
    }
    return;
  }

  if(is_abbrev(arg, "create")) {
    /* check for max */
    if(social_list_top>=MAX_SOCIALS) {
      printf_to_char(ch,"There are already a maximum of %d socials.\n\r",MAX_SOCIALS);
      return;
    }
    /* get social name */
    one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
    /* check for existing */
    if((act_nr=find_action(arg,strlen(arg)))>=0) {
      printf_to_char(ch,"Social %s already exists.\n\r",arg);
      return;
    }
    /* create and set default text */
    soc_mess_list[social_list_top].cmd_text      =str_dup(arg);
    soc_mess_list[social_list_top].hide = 0;
    soc_mess_list[social_list_top].min_victim_position = 0;
    soc_mess_list[social_list_top].char_no_arg   = str_dup("Text to character if no argument.");
    soc_mess_list[social_list_top].others_no_arg = str_dup("Text to others if no argument.");
    soc_mess_list[social_list_top].char_found    = str_dup("Text to character if victim found.");
    soc_mess_list[social_list_top].others_found  = str_dup("Text to others if victim found.");
    soc_mess_list[social_list_top].vict_found    = str_dup("Text to victim if victim found.");
    soc_mess_list[social_list_top].not_found     = str_dup("Text to character if victim not found.");
    soc_mess_list[social_list_top].char_auto     = str_dup("Text to character if it is victim.");
    soc_mess_list[social_list_top].others_auto   = str_dup("Text to others if character is victim.");
    social_list_top++;
    printf_to_char(ch,"Social %s ready for editting.\n\r",arg);
    return;
  }

  if(is_abbrev(arg, "sort")) {
    sort_socials();
    send_to_char("Socials sorted alphabetically.\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "edit")) { /* fields: hide min m1 m2 m3 m4 m5 m6 m7 m8 */
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
    if((act_nr=find_action(arg,strlen(arg)))<0) {
      printf_to_char(ch,"Could not find social: %s.\n\r",arg);
      return;
    }
    action=&soc_mess_list[act_nr];
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(edit_usage,ch);
      return;
    }
    if(is_abbrev(arg, "hide")) {
      one_argument(argument,arg);
      if(!*arg) {
        send_to_char(edit_usage,ch);
        return;
      }
      tmp=atoi(arg);
      if(tmp != 0 && tmp != 1) {
        send_to_char(edit_usage,ch);
        return;
      }
      action->hide=tmp;
      send_to_char("Done.\n\r",ch);
      return;
    }
    if(is_abbrev(arg, "min")) {
      one_argument(argument,arg);
      if(!*arg) {
        send_to_char(edit_usage,ch);
        return;
      }
      tmp=atoi(arg);
      if(tmp < 0 || tmp > 11) {
        send_to_char(edit_usage,ch);
        return;
      }
      action->min_victim_position=tmp;
      send_to_char("Done.\n\r",ch);
      return;
    }
    if(is_abbrev(arg, "m1")) {
      send_to_char("Enter text with variables (OLCHELP SOCIALS). End with @@ on same line.\n\r",ch);
      soc_mess_list[act_nr].char_no_arg[0] = 0;
      ch->desc->str = &soc_mess_list[act_nr].char_no_arg;
      ch->desc->max_str = 80;
      return;
    }
    if(is_abbrev(arg, "m2")) {
      send_to_char("Enter text with variables (OLCHELP SOCIALS). End with @@ on same line.\n\r",ch);
      soc_mess_list[act_nr].others_no_arg[0] = 0;
      ch->desc->str = &soc_mess_list[act_nr].others_no_arg;
      ch->desc->max_str = 80;
      return;
    }
    if(is_abbrev(arg, "m3")) {
      send_to_char("Enter text with variables (OLCHELP SOCIALS). End with @@ on same line.\n\r",ch);
      soc_mess_list[act_nr].char_found[0] = 0;
      ch->desc->str = &soc_mess_list[act_nr].char_found;
      ch->desc->max_str = 80;
      return;
    }
    if(is_abbrev(arg, "m4")) {
      send_to_char("Enter text with variables (OLCHELP SOCIALS). End with @@ on same line.\n\r",ch);
      soc_mess_list[act_nr].others_found[0] = 0;
      ch->desc->str = &soc_mess_list[act_nr].others_found;
      ch->desc->max_str = 80;
      return;
    }
    if(is_abbrev(arg, "m5")) {
      send_to_char("Enter text with variables (OLCHELP SOCIALS). End with @@ on same line.\n\r",ch);
      soc_mess_list[act_nr].vict_found[0] = 0;
      ch->desc->str = &soc_mess_list[act_nr].vict_found;
      ch->desc->max_str = 80;
      return;
    }
    if(is_abbrev(arg, "m6")) {
      send_to_char("Enter text with variables (OLCHELP SOCIALS). End with @@ on same line.\n\r",ch);
      soc_mess_list[act_nr].not_found[0] = 0;
      ch->desc->str = &soc_mess_list[act_nr].not_found;
      ch->desc->max_str = 80;
      return;
    }
    if(is_abbrev(arg, "m7")) {
      send_to_char("Enter text with variables (OLCHELP SOCIALS). End with @@ on same line.\n\r",ch);
      soc_mess_list[act_nr].char_auto[0] = 0;
      ch->desc->str = &soc_mess_list[act_nr].char_auto;
      ch->desc->max_str = 80;
      return;
    }
    if(is_abbrev(arg, "m8")) {
      send_to_char("Enter text with variables (OLCHELP SOCIALS). End with @@ on same line.\n\r",ch);
      soc_mess_list[act_nr].others_auto[0] = 0;
      ch->desc->str = &soc_mess_list[act_nr].others_auto;
      ch->desc->max_str = 80;
      return;
    }
    send_to_char(edit_usage,ch);
    return;
  }

  if(is_abbrev(arg, "load")) {
    boot_social_messages();
    send_to_char("Socials file reloaded.\n\r",ch);
    return;
  }

  if(is_abbrev(arg, "delete")) {
    one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage,ch);
      return;
    }
    if((act_nr=find_action(arg,strlen(arg)))<0) {
      printf_to_char(ch,"Could not find social: %s.\n\r",arg);
      return;
    }
    soc_mess_list[act_nr].cmd_text=str_dup("zzzzzzzz");
    sort_socials();
    social_list_top--;
    printf_to_char(ch,"Social %s deleted\n\r",arg);
    return;
  }

  if(is_abbrev(arg, "save")) {
    if(!(fl = fopen(SOCMESS_FILE, "w"))) {
      send_to_char("Could not open social message file for saving.\n\r",ch);
      return;
    }
    for(i=0;i<social_list_top;i++) {
      fprintf(fl,"%s %d %d\n",soc_mess_list[i].cmd_text,soc_mess_list[i].hide,soc_mess_list[i].min_victim_position);
      fprintf(fl,"%s\n",soc_mess_list[i].char_no_arg);
      fprintf(fl,"%s\n",soc_mess_list[i].others_no_arg);
      if(soc_mess_list[i].char_found) {
        fprintf(fl,"%s\n",soc_mess_list[i].char_found);
        fprintf(fl,"%s\n",soc_mess_list[i].others_found);
        fprintf(fl,"%s\n",soc_mess_list[i].vict_found);
        fprintf(fl,"%s\n",soc_mess_list[i].not_found);
        fprintf(fl,"%s\n",soc_mess_list[i].char_auto);
        fprintf(fl,"%s\n",soc_mess_list[i].others_auto);
      }
      fprintf(fl,"\n");
    }
    fprintf(fl,"-1\n");
    fclose(fl);
    send_to_char("Social file saved.\n\r",ch);
    return;
  }

  send_to_char(usage,ch);
}
