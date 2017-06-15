/* ************************************************************************
*  file: modify.c                                         Part of DIKUMUD *
*  Usage: Run-time modification (by users) of game variables              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************ */

/*
$Author: ronin $
$Date: 2005/01/21 14:55:28 $
$Header: /home/ronin/cvs/ronin/modify.c,v 2.1 2005/01/21 14:55:28 ronin Exp $
$Id: modify.c,v 2.1 2005/01/21 14:55:28 ronin Exp $
$Name:  $
$Log: modify.c,v $
Revision 2.1  2005/01/21 14:55:28  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.0.0.1  2004/02/05 16:09:49  ronin
Reinitialization of cvs archives


Revision 12-Mar-03 Ranger
Addition of code in string_add to remove double $$.

Revision 1.5  2002/04/18 18:45:54  ronin
Addition of reboot_type for normal or hotboot.

Revision 1.4  2002/04/18 04:07:31  ronin
Changing log output from perror to log_f for internal syslog manipulation.

Revision 1.3  2002/03/31 16:35:06  ronin
Added braces to remove ambiguous else warning.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/



#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "utility.h"
#include "cmd.h"
#include "act.h"


#define TP_MOB    0
#define TP_OBJ     1
#define TP_ERROR  2

extern struct obj_proto *obj_proto_table;
extern struct char_data *character_list;

void show_string(struct descriptor_data *d, char *input);
extern char *spells[];

char *string_fields[] =
{
  "name",
  "short",
  "long",
  "description",
  "title",
  "delete-description",
  "\n"
};

/* maximum length for text field x+1 */
int length[] =
{
  20,
  60,
  256,
  256,
  80
};




char *skill_fields[] =
{
  "learned",
  "affected",
  "duration",
  "recognize",
  "\n"
};




int max_value[] =
{
  255,
  255,
  10000,
  1
};

/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data *d, char *str) {
  char *scan,*fix;
  int terminator = 0;
  int bMakeNull = 0;

  /*check the first byte to see if its a 'command from writing a letter' */
  if(d->connected == CON_PLYNG) {
    if(str[0] == '~') {
      command_interpreter(d->character, &str[1]);
      return;
    }
    if(str[0] == '@') {
      str[0] = 0;
      terminator = 1;
      bMakeNull = ((str[1] == '@') && (str[2] == '@')); 
    }
  }

  /* determine if this is the terminal string, and truncate if so
     also filter out any ~ */
  for (scan = str; *scan; scan++) {
    if((*scan == '~')) {
      *scan = ' ';
    }

    /* rem double $$ from string */
    if((*scan == '$') && (scan[1] == '$')) {
      for(fix=scan; *fix; fix++)
        fix[0]=fix[1];
    }

    if((*scan == '@') && (scan[1] == '@')) {
      terminator = 1;
      *scan = '\0';
      break;
    }
  }

  if (!(*d->str)) {
    if (strlen(str) > d->max_str) {
      send_to_char("String too long - Truncated.\n\r", d->character);
      *(str + d->max_str) = '\0';
      terminator = 1;
    }
    CREATE(*d->str, char, strlen(str) + 3);
    strcpy(*d->str, str);
  }
  else {
    if (strlen(str) + strlen(*d->str) > d->max_str) {
      send_to_char("String too long. Last line skipped.\n\r",d->character);
      terminator = 1;
    }
    else {
      if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str)+strlen(str)+3))) {
        log_f("string_add");
        produce_core();
      }
      strcat(*d->str, str);
    }
  }

  if (terminator) {
    REMOVE_BIT(d->character->specials.pflag,PLR_WRITING);

    if (bMakeNull)
    {
       /*
        * BUG: Need to check if d->str isn't filled in with
        * previous lines.  If so, make null should just act
        * as a normal terminator.
        */
       free(*d->str);
       *d->str = 0;
    }

    d->str = 0;
    if (d->connected == CON_EXDSCR) {
      SEND_TO_Q(MENU, d);
      d->connected = CON_SLCT;
    }
  }
  else
    strcat(*d->str, "\n\r");
  /* maybe add a save board function here? */
}

#undef MAX_STR

/* interpret an argument for do_string */
void quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
  char buf[MAX_STRING_LENGTH];

  /* determine type */
  arg = one_argument(arg, buf);
  if (is_abbrev(buf, "char"))
     *type = TP_MOB;
  else if (is_abbrev(buf, "obj"))
     *type = TP_OBJ;
  else
  {
    *type = TP_ERROR;
    return;
  }

  /* find name */
  arg = one_argument(arg, name);

  /* field name and number */
  arg = one_argument(arg, buf);
  if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))
     return;

  /* string */
  for (; isspace(*arg); arg++);
  for (; (*string = *arg); arg++, string++);

  return;
}

/* modification of malloc'ed strings in chars/objects */
void do_string(struct char_data *ch, char *arg, int cmd)
{
  char name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
  char buffer[MAX_INPUT_LENGTH];
  int field, type;
  struct char_data *mob;
  struct obj_data *obj;

  if (IS_NPC(ch))
    return;

  quad_arg(arg, &type, name, &field, string);

  if (type == TP_ERROR) {
    send_to_char("Syntax:\n\rstring ('obj'|'char') <name> <field> [<string>].",
     ch);
    return;
  }

  if (!field) {
    send_to_char("No field by that name. Try 'wizhelp string'.\n\r",
     ch);
    return;
  }


  if (type == TP_MOB) {
    /* locate the beast */
    if (!(mob = get_char_vis(ch, name))) {
      send_to_char("I don't know anyone by that name...\n\r",
       ch);
      return;
    }

    if ((GET_LEVEL(ch)!=LEVEL_IMP) &&
        !IS_NPC(mob) && 
        (GET_LEVEL(mob)==LEVEL_IMP))
    {
      send_to_char("You cannot use this command on an Implementor.\n\r",ch);
      return;
    }

    if(ch!=mob) {
      sprintf(buffer,"WIZINFO: (%s) string %s",GET_NAME(ch),arg);
      wizlog(buffer, GET_LEVEL(ch)+1, 5);
      log_s(buffer);
    }

    if(field==1 && (!*string) ) {
      send_to_char("You must enter a name at the command line.\n\r",ch);
      return;
    }

    switch(field) {
    case 1:
      if (!IS_NPC(mob) && GET_LEVEL(ch) < LEVEL_IMP) {
  send_to_char("You can't change that field for players.", ch);
  return;
      }
      ch->desc->str = &mob->player.name;
      if (!IS_NPC(mob))
        send_to_char("WARNING: You have changed the name of a player.\n\r",ch);
      break;
    case 2:
      if (!IS_NPC(mob)) {
  send_to_char("That field is for monsters only.\n\r", ch);
  return;
      }
      ch->desc->str = &mob->player.short_descr;
      break;
    case 3:
      if (!IS_NPC(mob)) {
  send_to_char("That field is for monsters only.\n\r", ch);
  return;
      }
      ch->desc->str = &mob->player.long_descr;
      break;
    case 4:ch->desc->str = &mob->player.description; break;
    case 5:
      if (IS_NPC(mob)) {
  send_to_char("Monsters have no titles.\n\r",
         ch);
  return;
      }
      if(!ch->desc) return;

      ch->desc->str = &mob->player.title;
      break;
    case 6:
      if (IS_NPC(mob)) {
  send_to_char("Monsters can't do that.\n\r",
         ch);
  return;
      }
      ch->desc->str = &mob->player.poofin;
      break;
    default:
      send_to_char(
       "That field is undefined for monsters.\n\r", ch);
      return;
      break;
    }
  } else {   /* type == TP_OBJ */
    /* locate the object */
    if (!(obj = get_obj_vis(ch, name))) {
      send_to_char("Can't find such a thing here..\n\r", ch);
      return;
    }

    sprintf(buffer,"WIZINFO: (%s) string %s",GET_NAME(ch),arg);
    wizlog(buffer, LEVEL_IMP, 5);
    log_s(buffer);

    if(field==1 && (!*string) ) {
      send_to_char("You must enter a name at the command line.\n\r",ch);
      return;
    }

    switch(field) {
    case 1:
      ch->desc->str = &obj->name;
      break;
    case 2:
      ch->desc->str = &obj->short_description;
      break;
    case 3:
      ch->desc->str = &obj->description;
      break;
    case 4:
    case 6:
      send_to_char("This has been removed.\n\r", ch);
      return;
      break;
    default:
      send_to_char( "That field is undefined for objects.\n\r", ch);
      return;
      break;
    }
  }

  if (*ch->desc->str) {
    free(*ch->desc->str);
    *ch->desc->str = 0;
  }

  if (*string) {  /* there was a string in the argument array */
    if (strlen(string) > length[field - 1]) {
      send_to_char("String too long - truncated.\n\r", ch);
      *(string + length[field - 1]) = '\0';
    }
    CREATE(*ch->desc->str, char, strlen(string) + 1);
    strcpy(*ch->desc->str, string);
    ch->desc->str = 0;
    send_to_char("Ok.\n\r", ch);
  } else {         /* there was no string. enter string mode */
    send_to_char("Enter string. terminate with '@'.\n\r", ch);
    *ch->desc->str = 0;
    ch->desc->max_str = length[field - 1];
  }
}

/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */

void do_setskill (struct char_data *ch, char *arg, int cmd) {
  struct char_data *victim;
  int i, skill_number, value_number;
  char buf[MAX_INPUT_LENGTH];
  char name[100], skill[100], value[100];
  char buf3[MAX_INPUT_LENGTH];
  struct string_block sb;

  if (!*arg) {
    send_to_char("I need arguments!\n\r", ch);
    return;
  }

  half_chop(arg, name,100,buf,MIL);
  argument_interpreter(buf, skill, value);

  if (IS_NPC(ch)) return;

  if (!IS_SET(ch->new.imm_flags, WIZ_TRUST)) {
    send_to_char("You need a Trust flag to do that!\n\r", ch);
    return;
  }

  if (IS_SET(ch->new.imm_flags, WIZ_FREEZE)) {
    send_to_char("You are frozen! You can't to do that!\n\r", ch);
    return;
  }

  if (!IS_SET(ch->new.imm_flags, WIZ_ACTIVE)) {
    send_to_char("You need an active flag for this command.\n\r", ch);
    return;
  }

  if (*name) {
    if (!(victim = get_char_vis(ch, name))) {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }
  } else {
    send_to_char("Setskill who?\n\r", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }

  if (!*skill) {
    if(!victim->skills) {
      send_to_char("Your victim doesn't know any skills !\n\r", ch);
      return;
    }

    init_string_block (&sb);

    for(i=1; i < MAX_SKILLS5 && *(spells[i]) !='\n';i++) {
      if (victim->skills[i].learned) {
        sprintf(buf3,"%3d   %-35s   %3d percent\n\r", i, spells[i-1], victim->skills[i].learned);
        append_to_string_block(&sb, buf3);
      }
    }
    page_string_block (&sb, ch);
    destroy_string_block (&sb);
    return;
  } else {
    if (GET_LEVEL(ch) < LEVEL_SUP) {
      send_to_char("You need more power!\n\r", ch);
      return;
    }

    if (!isdigit(*skill) || !isdigit(*value)) {
      send_to_char("Second and third arguments must be integers.\n\r",ch);
      return;
    }

    skill_number = atoi(skill);
    value_number = atoi(value);

    if ((skill_number < 0) || (skill_number > MAX_SKILLS5) || (value_number < 0) || (value_number > 127)) {
      send_to_char("Bad values. \n\r", ch);
      return;
    }

    victim->skills[skill_number].learned = value_number;

    sprintf(buf, "WIZINFO: %s setskill %s %d %d", GET_NAME(ch),
      GET_NAME(victim), skill_number, value_number);
    log_s(buf);
    wizlog(buf, GET_LEVEL(ch), 5);
  }
}

/* db stuff *********************************************** */


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char *one_word(char *argument, char *first_arg )
{
  int begin, look_at;

  begin = 0;

  do
  {
    for ( ;isspace(*(argument + begin)); begin++);

    if (*(argument+begin) == '\"') {  /* is it a quote */

      begin++;

      for (look_at=0; (*(argument+begin+look_at) >= ' ') &&
          (*(argument+begin+look_at) != '\"') ; look_at++)
        *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

      if (*(argument+begin+look_at) == '\"')
        begin++;

    } else {

      for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
        *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

    }

    *(first_arg + look_at) = '\0';
    begin += look_at;
  }
  while (fill_word(first_arg));

  return(argument+begin);
}


struct help_index_element *build_help_index(FILE *fl, int *num)
{
  int nr = -1, issorted, i;
  struct help_index_element *list = 0, mem;
  char buf[81], tmp[81], *scan;
  long pos;

  for (;;)
  {
    pos = ftell(fl);
    fgets(buf, 81, fl);
    *(buf + strlen(buf) - 1) = '\0';
    scan = buf;
    for (;;)
    {
      /* extract the keywords */
      scan = one_word(scan, tmp);

      if (!*tmp)
        break;

      if (!list)
      {
        CREATE(list, struct help_index_element, 1);
        nr = 0;
      }
      else
        RECREATE(list, struct help_index_element, ++nr + 1);

      list[nr].pos = pos;
      CREATE(list[nr].keyword, char, strlen(tmp) + 1);
      strcpy(list[nr].keyword, tmp);
    }
    /* skip the text */
    do
      fgets(buf, 81, fl);
    while (*buf != '#');
    if (*(buf + 1) == '~')
      break;
  }
  /* we might as well sort the stuff */
  do
  {
    issorted = 1;
    for (i = 0; i < nr; i++)
      if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0)
      {
        mem = list[i];
        list[i] = list[i + 1];
        list[i + 1] = mem;
        issorted = 0;
      }
  }
  while (!issorted);

  *num = nr;
  return(list);
}



void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
  if (!d)
    return;

  if (keep_internal)
  {
    CREATE(d->showstr_head, char, strlen(str) + 1);
    strcpy(d->showstr_head, str);
    d->showstr_point = d->showstr_head;
  }
  else
    d->showstr_point = str;

  show_string(d, "");
}



void show_string(struct descriptor_data *d, char *input)
{
  char buffer[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH];
  register char *scan, *chk;
  int lines = 0, toggle = 1;

  one_argument(input, buf);

  if (*buf)
  {
    if (d->showstr_head)
    {
      free(d->showstr_head);
      d->showstr_head = 0;
    }
    d->showstr_point = 0;
    return;
  }

  /* show a chunk */
  for (scan = buffer;; scan++, d->showstr_point++)
    if((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) &&
      ((toggle = -toggle) < 0))
      lines++;
    else if (!*scan || (lines >= 22))
    {
      *scan = '\0';
      if(d->character) send_to_char(buffer,d->character);
                        else SEND_TO_Q(buffer, d);

      /* see if this is the end (or near the end) of the string */
      for (chk = d->showstr_point; isspace(*chk); chk++);
      if (!*chk)
      {
        if (d->showstr_head)
        {
          free(d->showstr_head);
          d->showstr_head = 0;
        }
        d->showstr_point = 0;
      }
      return;
    }
}

void night_watchman(void)
{
  long tc;
  struct tm *t_info;

  extern int cleanshutdown;

  void send_to_all(char *messg);

  tc = time(0);
  t_info = localtime(&tc);

  if ((t_info->tm_hour >= 6) && (t_info->tm_hour < 8) &&
      (t_info->tm_wday > 0) && (t_info->tm_wday < 6)) {
    if ((t_info->tm_min > 55) || (t_info->tm_hour == 7 && t_info->tm_min < 7)){
      log_f("Leaving the scene for the serious folks.");
      send_to_all("Closing down. Thank you for flying DikuMUD.\n\r");
      cleanshutdown = 1;
    } else if (t_info->tm_min > 45)
      send_to_all("ATTENTION: DikuMUD will shut down in 10 minutes.\n\r");
    else if (t_info->tm_min > 35)
      send_to_all("Warning: The game will close in 20 minutes.\n\r");
  }
}


void check_auction(void); /* Ranger - May 96 */
int copyover_write(int same_room);
void check_reboot(void) {
  long tc;
  struct tm *t_info;
  struct char_data *i,*boy=0;
  int min=0,next_hour;
  extern int cleanshutdown, cleanreboot,chreboot,disablereboot,REBOOT_AT,uptime,reboot_type;
  void send_to_all(char *messg);

  if(disablereboot) return; /* Added by Ranger Sept 97 */

  tc = time(0);
  min=(int)((tc-uptime)/60);
  if(min<140) return;
  t_info = localtime(&tc);

  next_hour=t_info->tm_hour+1;
  if(next_hour==24) next_hour=0;

  if( (next_hour == REBOOT_AT && t_info->tm_min > 34) ||
      ((t_info->tm_hour==REBOOT_AT)&&(t_info->tm_min<10))) {

    if(t_info->tm_min > 54 || (t_info->tm_min<10 && min>20) ) {
      check_auction(); /* Ranger - May 96 */
      if(reboot_type) {
        if(!copyover_write(0))
          log_f("Hotboot failed! Defaulting to full reboot.");
      }

      send_to_all("Automatic reboot. Come back in a little while.\n\r");
      cleanshutdown = cleanreboot = 1;
    }
    else if (t_info->tm_min > 44) {
      for (i=character_list; i; i=i->next) {
        if (IS_NPC(i) && V_MOB(i)==3) {
          boy=i;
          boy->specials.timer=10;
        }
      }
      if(boy) do_yell(boy,"HEAR YE, HEAR YE: The game will reboot in about 10 minutes.",CMD_YELL);
      else send_to_all("ATTENTION: The game will reboot in about 10 minutes.\n\r");
    }
    else if (t_info->tm_min > 34) {
      for (i=character_list; i; i=i->next) {
        if (IS_NPC(i) && V_MOB(i)==3) boy=i;
      }
      if(boy) do_yell(boy,"HEAR YE, HEAR YE: The game will reboot in about 20 minutes.",CMD_YELL);
      else send_to_all("Warning: The game will reboot in about 20 minutes.\n\r");
    }
  }
  chreboot=1;
}

/*#define GR*/
#define NEW

#ifdef GR

int workhours()
{
  long tc;
  struct tm *t_info;

  tc = time(0);
  t_info = localtime(&tc);

  return((t_info->tm_wday > 0) && (t_info->tm_wday < 6) && (t_info->tm_hour >= 9)
    && (t_info->tm_hour < 17));
}






/*
* This procedure is *heavily* system dependent. If your system is not set up
* properly for this particular way of reading the system load (It's weird all
* right - but I couldn't think of anything better), change it, or don't use -l.
* It shouldn't be necessary to use -l anyhow. It's oppressive and unchristian
* to harness man's desire to play. Who needs a friggin' degree, anyhow?
*/

int load(void)
{
  struct syslinfo {
    char  sl_date[12];  /* "Tue Sep 16\0" */
    char  sl_time[8];  /* "11:10\0" */
    char  sl_load1[6];  /* "12.0\0" */
    char  sl_load2[10];  /* "+2.3 14u\0" */
  } info;
  FILE *fl;
  int i, sum;
  static int previous[5];
  static int p_point = -1;
  extern int slow_death;

  if (!(fl = fopen("/tmp/.sysline", "r")))
  {
    log_f("sysline. (dying)");
    slow_death = 1;
    return(-1);
  }
  if (!fread(&info, sizeof(info), 1, fl))
  {
    log_f("fread sysline (dying)");
    slow_death = 1;
    return(-1);
  }
  fclose(fl);

  if (p_point < 0)
  {
    previous[0] = atoi(info.sl_load1);
    for (i = 1; i< 5; i++)
      previous[i] = previous[0];
    p_point = 1;
    return(previous[0]);
  }
  else
  {
    /* put new figure in table */
    previous[p_point] = atoi(info.sl_load1);
    if (++p_point > 4)
      p_point = 0;

    for (i = 0, sum = 0; i < 5; i++)
      sum += previous[i];
    return((int) sum / 5);
  }
}

char *nogames(void)
{
  static char text[200];
  FILE *fl;

  if ((fl = fopen("lib/nogames", "r")))
  {
    log_f("/usr/games/nogames exists");
#ifdef LINUX
    fgets(text,81, fl);
#else
    fgets(text,fl);
#endif
    return(text);
    fclose(fl);
  }
  else
    return(0);
}


#ifdef OLD_COMA

void comatose(void)
{
  extern struct descriptor_data *descriptor_list;
  extern int tics;

  void close_socket(struct descriptor_data *d);

  log_f("Entering comatose state");

  while (descriptor_list)
    close_socket(descriptor_list);

  do
  {
    sleep(300);
    tics = 1;
    if (workhours())
    {
      log_f("Working hours collision during coma. Exit.");
      produce_core();
    }
  }
  while (load() >= 6);

  log_f("Leaving coma");
}

#endif



/* emulate the game regulator */
void gr(int s)
{
  char *txt = 0, buf[1024];
  int ld = 0;
  static char *warnings[3] =
  {
    "If things don't look better within 3 minutes, the game will pause.\n\r",
    "The game will close temporarily 2 minutes from now.\n\r",
    "WARNING: The game will close in 1 minute.\n\r"
   };
  static int wnr = 0;

  extern int slow_death, cleanshutdown;

  void send_to_all(char *messg);

  void comatose(int s);

  if (((ld = load()) >= 6) || (txt = nogames()) || slow_death)
  {
    if (ld >= 6)
    {
      sprintf(buf, "The system load is greater than 6.0 (%d)\n\r", ld);
      send_to_all(buf);
    }
    else if (slow_death)
      send_to_all("The game is dying.\n\r");
    else
    {
      strcpy(buf,
        "Game playing is no longer permitted on this machine:\n\r");
      strcat(buf, txt);
      strcat(buf, "\n\r");
      send_to_all(buf);
    }

    if (wnr < 3)
      send_to_all(warnings[wnr++]);
    else
      if (ld >= 6)
      {
        comatose(s);
        wnr = 0;
      }
      else
        cleanshutdown = 1;
  }
  else if (workhours())
    cleanshutdown = 1;
  else if (wnr)
  {
    send_to_all("Things look brighter now - you can continue playing.\n\r");
    wnr = 0;
  }
}



#endif
