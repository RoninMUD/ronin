/* ************************************************************************
 *  file: Interpreter.c , Command interpreter module.      Part of DIKUMUD *
 *  Usage: Procedures interpreting user command                            *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

/*
$Author: ronin $
$Date: 2005/04/27 17:13:30 $
$Header: /home/ronin/cvs/ronin/interpreter.c,v 2.8 2005/04/27 17:13:30 ronin Exp $
$Id: interpreter.c,v 2.8 2005/04/27 17:13:30 ronin Exp $
$Name:  $
$Log: interpreter.c,v $
Revision 2.8  2005/04/27 17:13:30  ronin
Minor changes needed to compile on Slackware 10 for the new machine.

Revision 2.7  2005/01/21 14:55:27  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.6  2004/11/17 19:21:52  void
Added Nomad Skill Cover (1/2 Damage -10 Hitroll)

Revision 2.5  2004/11/16 05:26:41  ronin
DOS to UNIX conversion

Revision 2.4  2004/11/16 04:55:17  ronin
Commented out playeravg, lowered reindex to SUP

Revision 2.3  2004/06/02 13:39:35  ronin
Added zmult.

Revision 2.2  2004/03/13 05:27:14  pyro
updated for olc commands oname and owear

Revision 2.1  2004/02/11 13:40:34  ronin
idname command changed from LEVEL_WIZ to LEVEL_SUP.

Revision 2.0.0.1  2004/02/05 16:09:33  ronin
Reinitialization of cvs archives


Revision 05-Feb-04 Ranger
Addition of do_idname command

Revision 19-Dec-03 Ranger
Addition of email command

Revision 22-May-03 Ranger
Addition of hunt command (currently IMP only)

Revision 10-Mar-03 Ranger
Addition of social link to social command.

Revision 12-Feb-03 Ranger
Addition of do_board

Revision 1.3  2002/10/14 21:16:28  ronin
Addition of MSG_RECONNECT after player reconnect.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#include "structs.h"

#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "cmd.h"
#include "spells.h" /* Ranger - May 96 */
#include "limits.h"
#include "reception.h"
#include "utility.h"
#include "act.h"
#include "comm.h"
#include "fight.h"
#include "handler.h"
#include "enchant.h"

#define NOT !
#define AND &&
#define OR ||

#define STATE(d) ((d)->connected)
#define MAX_CMD_LIST 400

extern char *BKColor[];
extern char motd[MAX_STRING_LENGTH];
extern char godmotd[MAX_STRING_LENGTH];
extern char newbiemotd[MAX_STRING_LENGTH];
extern CHAR *character_list;
extern int top_of_p_table;
int max_connects=0;
struct command_info cmd_info[MAX_CMD_LIST];
extern char*   crypt __P((__const char *__key, __const char *__salt));
extern int CHAOSMODE;

char echo_on[]  = {IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0'};
char echo_off[] = {IAC, WILL, TELOPT_ECHO, '\0'};
char last_command[MSL];

extern int WIZLOCK;
extern int GAMELOCK;
extern int GAMECHECK;

/* external fcntls */

void init_char(CHAR *ch);
void do_check_pa(struct char_data *ch); /* Ranger May 96 */
void do_start(struct char_data *ch);
int insert_char_wizlist (struct char_data *ch);
void add_clanlist_name (struct char_data *ch, int clan);
int find_action(char *command,int length);
int special(CHAR *ch, int cmd, char *arg);
int obj_special(struct obj_data *obj, CHAR *ch, int cmd, char *arg);
int mob_special(CHAR *mob, CHAR *ch, int cmd, char *arg);
int room_special(int room, CHAR *ch, int cmd, char *arg);

char *str_dup(char *source);

char *unknownCMD[] = {
  "Arglebargle, glop-glyf!?!\n\r",
  "Blaah Blaah Blaah !\n\r",
  "Blow your keyboard !\n\r",
  "Are you serious ??\n\r",
  "THAT is your problem.\n\r",
  "*laugh*\n\r",
  "Wake up! That's impossible!\n\r",
  "Hey! That command is only for lvl 58 immortals!\n\r",
  "Next time you do that this game will crash... ;)\n\r",
  "You did it again!\n\r",
  "Small Dikubug tells you 'better luck next time :)'\n\r",
  "Small Dikubug tells you 'You did it! Gratz! :)'\n\r",
  "Small Dikubug tells you 'PAH! Blow your brains or try again!'\n\r",
  "Small Dikubug laughs at you.\n\r",
  "Small Dikubug points at you and laughs!\n\r",
  "Last warning: Dont make typos. They can kill you!\n\r",
  "Mudding without typos is not mudding.\n\r",
  "Oh...You mean...\n\r",
  "Suicide could be a better way to do that...\n\r",
  "Small Dikubug pats you on your head and says 'Blaah Blaah'\n\r",
  "Hmmm...pardon??\n\r",
  "AAAARRRRGGGGHHH!!! Typos rule this Mud...\n\r",
  "Some words are harder to type than others...try again!\n\r",
  "Small Dikubug comforts you.\n\r",
  "Arglebargle, glop-glyf!?!\n\r",
  "Blaah Blaah Blaah !\n\r",
  "Blow your keyboard !\n\r",
  "Are you serious ??\n\r",
  "THAT is your problem.\n\r",
  "*laugh*\n\r",
  "Wake up! That's impossible!\n\r",
  "Hey! Only level 58 immortals can make those typo's!\n\r",
  "Quit making typo's...you are confusing me!\n\r",
  "You screwed up again!\n\r",
  "Small Dikubug tells you 'Typos Rule!'\n\r",
  "Small Dikubug tells you 'You did it! Gratz! :)'\n\r",
  "Small Dikubug tells you 'Quit confusing me!'\n\r",
  "Small Dikubug laughs at your mistake.\n\r",
  "Small Dikubug points at you and laughs at your mistake!\n\r",
  "Last warning: Dont make typos. They can kill you!\n\r",
  "Mudding without typos is not mudding.\n\r",
  "Oh...You mean...\n\r",
  "Why don't you try that again?\n\r",
  "Small Dikubug tells you 'You are the typo mastre!'\n\r",
  "Hmmm...pardon??\n\r",
  "AAAARRRRGGGGHHH!!! Typos rule this Mud...\n\r",
  "Some words are harder to type than others...try again!\n\r",
  "Small Dikubug bonks you on your head because of your mistake.\n\r",
  "\n\r"
  };


char *fill[]=
{ "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

char *ill_name[]=
{ "up",
  "down",
  "north",
  "south",
  "east",
  "west",
  "\n"
};

int search_block(char *arg, char **list, bool exact)
{
  register int i,l;

  /* Make into lower case, and get length of string */
  for (l=0; *(arg+l); l++)
     *(arg+l)=LOWER(*(arg+l));
  if (exact) {
   for(i=0; **(list+i) != '\n'; i++)
    if (!strcmp(arg, *(list+i)))
      return(i);
  } else {
    if (!l)
      l=1; /* Avoid "" to match the first available string */
      for(i=0; **(list+i) != '\n'; i++)
       if (!strncmp(arg, *(list+i), l))
   return(i);
  }
  return(-1);
}

int determine_command(char *command,int length)
{
  int i=0;
  while(cmd_info[i].cmd_text)
  {
        if(!strncmp(cmd_info[i].cmd_text,command,length))
             break;
        i++;
        }
  if (!cmd_info[i].cmd_text||!length)
      i = -1;
  return i;
}

int old_search_block(char *argument,int begin,int length,char **list,int mode)
{
  int guess, found, search;

  /* If the word contain 0 letters, then a match is already found */
  found = (length < 1);
  guess = 0;

  /* Search for a match */
  if (mode)
    while ( NOT found AND *(list[guess]) != '\n' ) {
      found=(length==strlen(list[guess]));
      for (search=0;( search < length AND found );search++)
   found=(*(argument+begin+search)== *(list[guess]+search));
   guess++;
    }
  else {
   while ( NOT found AND *(list[guess]) != '\n' ) {
  found=1;
  for (search=0;( search < length AND found );search++)
     found=(*(argument+begin+search)== *(list[guess]+search));
     guess++;
   }
  }
  return ( found ? guess : -1 );
}

void command_interpreter(CHAR *ch, char *argument)
{
  int index;
  extern int no_specials;
  char *command,*args;
#ifndef TEST_SITE
  char buf[MAX_STRING_LENGTH];
#endif

  if(ch->desc) {
    if(!ch->desc->original)
      REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
  }
  else
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

  update_pos(ch);

  if(CHAR_REAL_ROOM(ch)==NOWHERE) return;

  /* Freeze extension to mortals - Ranger March 2000 */
  if(IS_MORTAL(ch) && IS_SET(ch->new.imm_flags, WIZ_FREEZE)) {
    send_to_char("You are completely frozen, unable to do anything.\n\r",ch);
    return;
  }

  if(affected_by_spell(ch, SKILL_MEDITATE) && GET_LEVEL(ch)<LEVEL_IMM) {   /* Chaos03 */
    if((CHAOSMODE && duration_of_spell(ch,SKILL_MEDITATE)>9) ||
     duration_of_spell(ch,SKILL_MEDITATE)>30) {
      send_to_char("You are in a deep healing trance, unable to do anything.\n\r",ch);
      return;
    }
  }

  if(affected_by_spell(ch, SKILL_PRAY) && GET_LEVEL(ch)<LEVEL_IMM) {
    send_to_char("You are deep in prayer, unable to do anything.\n\r",ch);
    return;
  }

  /* Find first non blank */
  for (command = argument ; *command == ' '  ; command++ );

  /* Find length of first word */
  for (args=command; *args > ' ' ; args++)
     *(args) = LOWER(*args);/* Make all letters lower case AND find length */

  if(signal_room(CHAR_REAL_ROOM(ch),ch, CMD_UNKNOWN, command)) return;
  index = determine_command(command,args-command);
  if (index <0 ) {
    index = find_action(command,args-command);
    if(index >-1) {
      switch(GET_POS(ch)) {
        case POSITION_DEAD:
          send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
          return;
          break;
        case POSITION_INCAP:
        case POSITION_MORTALLYW:
          send_to_char("You are in a pretty bad shape, unable to do anything!\n\r", ch);
          return;
          break;
        case POSITION_STUNNED:
          send_to_char("All you can do right now, is think about the stars!\n\r", ch);
          return;
          break;
        case POSITION_SLEEPING:
          send_to_char("In your dreams, or what?\n\r", ch);
          return;
          break;
      }

      do_action(ch,args,find_action(command,args-command));
      if( signal_room( CHAR_REAL_ROOM( ch ), ch, MSG_UNKNOWN, command ) )
        return;
      index = -1;     /*Set it like nothing happened */
      args = command;
    }
  }

  if (index >=0 && GET_LEVEL(ch)>=cmd_info[index].minimum_level ) {
    if (cmd_info[index].command_pointer != 0) {
      if ( GET_POS(ch) < cmd_info[index].minimum_position ) {
        switch(GET_POS(ch)) {
          case POSITION_DEAD:
            send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
            return;
            break;
          case POSITION_INCAP:
          case POSITION_MORTALLYW:
            send_to_char("You are in a pretty bad shape, unable to do anything!\n\r", ch);
            return;
            break;
          case POSITION_STUNNED:
            send_to_char("All you can do right now, is think about the stars!\n\r", ch);
            return;
            break;
          case POSITION_SLEEPING:
            send_to_char("In your dreams, or what?\n\r", ch);
            return;
            break;
          case POSITION_RESTING:
            send_to_char("Nah... You feel too relaxed to do that..\n\r", ch);
            return;
            break;
          case POSITION_SITTING:
            send_to_char("Maybe you should get on your feet first?\n\r",ch);
            return;
            break;
          case POSITION_FIGHTING:
            send_to_char("No way! You are fighting for your life!\n\r", ch);
            return;
            break;
        }
      }
      else {
        if(cmd_info[index].minimum_position>POSITION_FIGHTING &&
           ch->specials.fighting) {
          send_to_char("No way! You are fighting for your life!\n\r",ch);
          return;
        }

/* Log the command for output at crash - Ranger Jan 2001*/
        sprintf(last_command, "[%5d] %s in [%5d] %s: %s",
                IS_NPC(ch) ? V_MOB(ch): 0, GET_NAME(ch),
                CHAR_REAL_ROOM(ch)!=NOWHERE ? CHAR_VIRTUAL_ROOM(ch):0,
                CHAR_REAL_ROOM(ch)!=NOWHERE ? world[CHAR_REAL_ROOM(ch)].name : "(nowhere)",command);

        if (!no_specials && signal_room(CHAR_REAL_ROOM(ch),ch, cmd_info[index].num, args)) {
          sprintf(last_command, "(Finished) [%5d] %s in [%5d] %s: %s",
                  IS_NPC(ch) ? V_MOB(ch): 0, GET_NAME(ch),
                  CHAR_REAL_ROOM(ch)!=NOWHERE ? CHAR_VIRTUAL_ROOM(ch):0,
                  CHAR_REAL_ROOM(ch)!=NOWHERE ? world[CHAR_REAL_ROOM(ch)].name : "(nowhere)",command);
          return;
        }

        if(CHAR_REAL_ROOM(ch)==NOWHERE) {
          sprintf(last_command, "(Finished) [%5d] %s in [%5d] %s: %s",
                  IS_NPC(ch) ? V_MOB(ch): 0, GET_NAME(ch),
                  CHAR_REAL_ROOM(ch)!=NOWHERE ? CHAR_VIRTUAL_ROOM(ch):0,
                  CHAR_REAL_ROOM(ch)!=NOWHERE ? world[CHAR_REAL_ROOM(ch)].name : "(nowhere)",command);
          return;
        }

       ((*cmd_info[index].command_pointer) (ch, args, cmd_info[index].num));
#ifndef TEST_SITE
       if(cmd_info[index].num==CMD_NEWOLCM ||
          cmd_info[index].num==CMD_NEWOLCO ||
          cmd_info[index].num==CMD_NEWOLCZ ||
          cmd_info[index].num==CMD_NEWOLCR) {
          sprintf(buf,"WIZINFO: %s %s",GET_NAME(ch),command);
          wizlog(buf,GET_LEVEL(ch)+1,5);
          log_s(buf);
       }
#endif
       sprintf(last_command, "(Finished) [%5d] %s in [%5d] %s: %s",
               IS_NPC(ch) ? V_MOB(ch): 0, GET_NAME(ch),
               CHAR_REAL_ROOM(ch)!=NOWHERE ? CHAR_VIRTUAL_ROOM(ch):0,
               CHAR_REAL_ROOM(ch)!=NOWHERE ? world[CHAR_REAL_ROOM(ch)].name : "(nowhere)",command);
      }
    }
  }
  else {
    if (args-command) { /* i.e. the command has a length, not just a return */
       if(!signal_room(CHAR_REAL_ROOM(ch),ch, CMD_UNKNOWN, command)) {
         if(GET_LEVEL(ch)>2) {
           send_to_char(unknownCMD[number(0, 47)], ch);
         }
         else {
           send_to_char("\
Sorry, that command doesn't exist, try typing HELP or\n\r\
HELP INDEX to see a list of available commands.\n\r\
Also, HELP COMPARISON may help if you are use\n\r\
to a different type of mud.\n\r\n\r",ch);
         }
       }
    }
  }
  return;
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
  int look_at = 0, begin = 0;

  do
  {
    /* Find first non blank */
    for ( ;*(argument + begin ) == ' ' ; begin++);

    /* Find length of first word */
    for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to first_arg */
    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

    *(first_arg + look_at)='\0';
    begin += look_at;

  }
  while( fill_word(first_arg));

  do
  {
    /* Find first non blank */
    for ( ;*(argument + begin ) == ' ' ; begin++);

    /* Find length of first word */
    for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));

    *(second_arg + look_at)='\0';
    begin += look_at;

  }
  while( fill_word(second_arg));
}

int is_number(char *str)
{
  int look_at,start=0;

  if(*str=='\0') return(0);

  if(*str=='-') start=1;

  for (look_at=start;*(str+look_at) != '\0';look_at++)
     if ((*(str+look_at)<'0')||(*(str+look_at)>'9'))
       return(0);

  return(1);
}

int is_big_number(char *str)
{
  int index = 0;

  if (*str == '\0') return FALSE;

  for (index = (*str == '-') ? 1 : 0; *(str + index) != '\0'; index++)
     if ((*(str + index) < '0') || (*(str + index) > '9'))
       return FALSE;

  return TRUE;
}


/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg                  */
char *one_argument(char *argument, char *first_arg )
{
  int begin, look_at;

  begin = 0;

  do
  {
    /* Find first non blank */
    for ( ;isspace(*(argument + begin)); begin++);

    /* Find length of first word */
    for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
      /* Make all letters lower case, AND copy them to first_arg */
      *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
    *(first_arg + look_at)='\0';
    begin += look_at;
  }
  while (fill_word(first_arg));

  return(argument+begin);
}

int fill_word(char *argument)
{
  return ( search_block(argument,fill,TRUE) >= 0);
}

/* determine if a given string is an abbreviation of another */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1) return(0);

  for (; *arg1; arg1++, arg2++)
     if (LOWER(*arg1) != LOWER(*arg2))
       return(0);

  return(1);
}

/* return first 'word' plus trailing substring of input string */
void half_chop(char *string, char *arg1,int len1,char *arg2,int len2)
{
  for (; *string && isspace(*string); string++); /* skip spaces */
  for (; *string && len1>1 && !isspace(*arg1 = *string); string++, arg1++,len1--); /* assign all non space */
  *arg1 = '\0';
  for (; *string && isspace(*string); string++); /* skip spaces */
  for (; *string && len2>1 && (*arg2 = *string); string++, arg2++,len2--); /* assign all non space */
  *arg2 = '\0';
}

int obj_special(struct obj_data *obj,CHAR *ch,int cmd,char *arg)
{
  if( obj->func )
  {
    return obj->func(obj, ch, cmd, arg );
  }

  return ((*obj_proto_table[obj->item_number].func)(obj, ch, cmd, arg));
}

int mob_special(CHAR *mob,CHAR *ch,int cmd,char *arg)
{
  return ((*mob_proto_table[mob->nr].func)(mob, ch, cmd, arg));
}

int enchantment_special(struct enchantment_type_5 *enchantment, CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  if (!enchantment) return FALSE;

  if (((*enchantment->func)(enchantment, mob, ch, cmd, arg))) return TRUE;

  if (cmd == MSG_TICK)
  {
    if (enchantment->duration && enchantment->duration > 0)
    {
      enchantment->duration--;
    }
    else if (!enchantment_special(enchantment, mob, NULL, MSG_REMOVE_ENCH, NULL))
    {
      enchantment_remove(mob, enchantment, TRUE);
    }
  }

  return FALSE;
}

int room_special(int in_room,CHAR *ch,int cmd,char *arg)
{
  return ((*world[in_room].funct)(in_room, ch, cmd, arg));
}

int special(CHAR *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct enchantment_type_5 *tmp_ench;
  register CHAR *k,*temp;
  int j;
  if(CHAR_REAL_ROOM(ch) == NOWHERE)
     return(TRUE);

  /* special in room? */
  if (world[CHAR_REAL_ROOM(ch)].funct)
    if (room_special(CHAR_REAL_ROOM(ch), ch, cmd, arg))
      return(1);

  /* special in equipment list? */
  for (j = 0; j <= (MAX_WEAR - 1); j++)
    if (ch->equipment[j] && ch->equipment[j]->item_number>=0)
      if (obj_proto_table[ch->equipment[j]->item_number].func)
  if (obj_special(ch->equipment[j],ch,cmd,arg))
    return(1);

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (i->item_number>=0)
      if (obj_proto_table[i->item_number].func)
  if (obj_special(i, ch, cmd, arg))
    return(1);

  /* special in mobile or char present? */
  for (k = world[CHAR_REAL_ROOM(ch)].people; k; k = temp ) {
    temp = k->next_in_room; /* Added temp - Ranger June 96 */
    if(IS_NPC(k))
      {
      if (mob_proto_table[k->nr].func)
        {
  if (mob_special(k, ch, cmd, arg))
    return(1);
        }
      }
    for(tmp_ench = k->enchantments;tmp_ench;tmp_ench = tmp_ench->next)
      {
      if(tmp_ench->func)
        if(enchantment_special(tmp_ench,k,ch,cmd,arg))
             return(1);
      }
    }
  /* special in object present? */
  for (i = world[CHAR_REAL_ROOM(ch)].contents; i; i = i->next_content)
    if (i->item_number>=0)
      if (obj_proto_table[i->item_number].func)
  if (obj_special(i, ch, cmd, arg))
    return(1);

  return(0);
}


#ifdef TEST_SITE
 #define LEVEL_OLC1 LEVEL_DEI
 #define LEVEL_OLC2 LEVEL_DEI
 #define LEVEL_OLC3 LEVEL_DEI
#else
 #define LEVEL_OLC1 LEVEL_WIZ
 #define LEVEL_OLC2 LEVEL_ETE
 #define LEVEL_OLC3 LEVEL_IMP
#endif

void assign_command_pointers ( void )
{
  int position;

#define COMMANDO(string,number,min_pos,pointer,min_level) {        \
        while(cmd_info[position].num   ) position++;               \
        cmd_info[position].cmd_text         = str_dup( (string) ); \
  cmd_info[position].command_pointer  = (pointer);           \
  cmd_info[position].minimum_position = (min_pos);           \
  cmd_info[position].minimum_level    = (min_level);         \
        cmd_info[position].num              = (number);            }

  for (position = 0 ; position < MAX_CMD_LIST; position++)
    cmd_info[position].command_pointer = 0;

  position = 0;
  COMMANDO("north"    ,CMD_NORTH     ,POSITION_STANDING  ,do_move,0);
  COMMANDO("east"     ,CMD_EAST      ,POSITION_STANDING  ,do_move,0);
  COMMANDO("south"    ,CMD_SOUTH     ,POSITION_STANDING  ,do_move,0);
  COMMANDO("west"     ,CMD_WEST      ,POSITION_STANDING  ,do_move,0);
  COMMANDO("up"       ,CMD_UP        ,POSITION_STANDING  ,do_move,0);
  COMMANDO("down"     ,CMD_DOWN      ,POSITION_STANDING  ,do_move,0);
  COMMANDO("enter"    ,CMD_ENTER     ,POSITION_STANDING  ,do_enter,0);
  COMMANDO("exits"    ,CMD_EXITS     ,POSITION_RESTING   ,do_exits,0);
  COMMANDO("get"      ,CMD_GET       ,POSITION_RESTING   ,do_get,0);
  COMMANDO("drink"    ,CMD_DRINK     ,POSITION_RESTING   ,do_drink,0);
  COMMANDO("eat"      ,CMD_EAT       ,POSITION_RESTING   ,do_eat,0);
  COMMANDO("wear"     ,CMD_WEAR      ,POSITION_RESTING   ,do_wear,0);
  COMMANDO("wield"    ,CMD_WIELD     ,POSITION_RESTING   ,do_wield,0);
  COMMANDO("look"     ,CMD_LOOK      ,POSITION_RESTING   ,do_look,0);
  COMMANDO("score"    ,CMD_SCORE     ,POSITION_DEAD      ,do_score,0);
  COMMANDO("scoreboard",CMD_SCOREBOARD,POSITION_DEAD     ,do_scoreboard,0);/*Ranger Oct 99*/
  COMMANDO("say"      ,CMD_SAY       ,POSITION_RESTING   ,do_say,0);
  COMMANDO("shout"    ,CMD_SHOUT     ,POSITION_RESTING   ,do_shout,3);
  COMMANDO("tell"     ,CMD_TELL      ,POSITION_DEAD      ,do_tell,0);
  COMMANDO("vicious"  ,CMD_VICIOUS   ,POSITION_DEAD      ,do_vicious,0);
  COMMANDO("bleed"    ,CMD_BLEED     ,POSITION_DEAD      ,do_bleed,0);
  COMMANDO("inventory",CMD_INVENTORY ,POSITION_DEAD      ,do_inventory,0);
  COMMANDO("qui"      ,CMD_QUI       ,POSITION_DEAD      ,do_qui,0);
  COMMANDO("kill"     ,CMD_KILL      ,POSITION_FIGHTING  ,do_kill,0);
  COMMANDO("insult"   ,CMD_INSULT    ,POSITION_RESTING   ,do_insult,0);
  COMMANDO("help"     ,CMD_HELP      ,POSITION_DEAD      ,do_help,0);
  COMMANDO("who"      ,CMD_WHO       ,POSITION_DEAD      ,do_who,0);
  COMMANDO("emote"    ,CMD_EMOTE     ,POSITION_SLEEPING  ,do_emote,0);
  COMMANDO("echo"     ,CMD_ECHO      ,POSITION_DEAD      ,do_echo,LEVEL_IMM);
  COMMANDO("stand"    ,CMD_STAND     ,POSITION_RESTING   ,do_stand,0);
  COMMANDO("sit"      ,CMD_SIT       ,POSITION_RESTING   ,do_sit,0);
  COMMANDO("rest"     ,CMD_REST      ,POSITION_RESTING   ,do_rest,0);
  COMMANDO("sleep"    ,CMD_SLEEP     ,POSITION_SLEEPING  ,do_sleep,0);
  COMMANDO("wake"     ,CMD_WAKE      ,POSITION_SLEEPING  ,do_wake,0);
  COMMANDO("force"    ,CMD_FORCE     ,POSITION_DEAD      ,do_force,LEVEL_WIZ);
  COMMANDO("transfer" ,CMD_TRANSFER  ,POSITION_DEAD      ,do_trans,LEVEL_DEI);
  COMMANDO("motd"     ,CMD_NEWS      ,POSITION_SLEEPING  ,do_motd,0);
  COMMANDO("equipment",CMD_EQUIPMENT ,POSITION_SLEEPING  ,do_equipment,0);
  COMMANDO("buy"      ,CMD_BUY       ,POSITION_STANDING  ,do_not_here,0);
  COMMANDO("sell"     ,CMD_SELL      ,POSITION_STANDING  ,do_not_here,0);
  COMMANDO("value"    ,CMD_VALUE     ,POSITION_STANDING  ,do_not_here,0);
  COMMANDO("list"     ,CMD_LIST      ,POSITION_STANDING  ,do_not_here,0);
  COMMANDO("drop"     ,CMD_DROP      ,POSITION_RESTING   ,do_drop,0);
  COMMANDO("goto"     ,CMD_GOTO      ,POSITION_DEAD      ,do_goto,LEVEL_IMM);
  COMMANDO("weather"  ,CMD_WEATHER   ,POSITION_RESTING   ,do_weather,0);
  COMMANDO("read"     ,CMD_READ      ,POSITION_RESTING   ,do_read,0);
  COMMANDO("pour"     ,CMD_POUR      ,POSITION_STANDING  ,do_pour,0);
  COMMANDO("grab"     ,CMD_GRAB      ,POSITION_RESTING   ,do_grab,0);
  COMMANDO("remove"   ,CMD_REMOVE    ,POSITION_RESTING   ,do_remove,0);
  COMMANDO("put"      ,CMD_PUT       ,POSITION_RESTING   ,do_put,0);
  COMMANDO("save"     ,CMD_SAVE      ,POSITION_SLEEPING  ,do_save,0);
  COMMANDO("hit"      ,CMD_HIT       ,POSITION_FIGHTING  ,do_hit,1);
  COMMANDO("string"   ,CMD_STRING    ,POSITION_DEAD      ,do_string,LEVEL_TEM);
  COMMANDO("give"     ,CMD_GIVE      ,POSITION_RESTING   ,do_give,1);
  COMMANDO("quit"     ,CMD_QUIT      ,POSITION_DEAD      ,do_quit,1);
  COMMANDO("stat"     ,CMD_STAT      ,POSITION_DEAD      ,do_stat,LEVEL_IMM);
  COMMANDO("setskill" ,CMD_SETSKILL  ,POSITION_DEAD     ,do_setskill,LEVEL_IMP);
  COMMANDO("time"     ,CMD_TIME      ,POSITION_DEAD      ,do_time,1);
  COMMANDO("load"     ,CMD_LOAD      ,POSITION_DEAD      ,do_load,LEVEL_DEI);
  COMMANDO("purge"    ,CMD_PURGE     ,POSITION_DEAD      ,do_purge,LEVEL_DEI);
  COMMANDO("shutdown" ,CMD_SHUTDOWN  ,POSITION_DEAD      ,do_shutdown,LEVEL_SUP);
  COMMANDO("identify" ,CMD_IDENTIFY  ,POSITION_FIGHTING  ,auction_identify, 1); /* Identify in auction - Ranger May 96 */
  COMMANDO("idea"     ,CMD_USEIDEA   ,POSITION_DEAD      ,do_useidea,1);
  COMMANDO("typo"     ,CMD_TYPO      ,POSITION_DEAD      ,do_typo,1);
  COMMANDO("bug"      ,CMD_USEIDEA   ,POSITION_DEAD      ,do_useidea,1);
  COMMANDO("whisper"  ,CMD_WHISPER   ,POSITION_RESTING   ,do_whisper,1);
  COMMANDO("cast"     ,CMD_CAST      ,POSITION_SITTING   ,do_cast,1);
  COMMANDO("at"       ,CMD_AT        ,POSITION_DEAD      ,do_at,LEVEL_IMM);
  COMMANDO("ask"      ,CMD_ASK       ,POSITION_RESTING   ,do_ask,1);
  COMMANDO("order"    ,CMD_ORDER     ,POSITION_RESTING   ,do_order,1);
  COMMANDO("sip"      ,CMD_SIP       ,POSITION_RESTING   ,do_sip,1);
  COMMANDO("taste"    ,CMD_TASTE     ,POSITION_RESTING   ,do_taste,1);
  COMMANDO("snoop"    ,CMD_SNOOP     ,POSITION_DEAD      ,do_snoop,LEVEL_IMM);
  COMMANDO("follow"   ,CMD_FOLLOW    ,POSITION_RESTING   ,do_follow,1);
  COMMANDO("rent"     ,CMD_RENT      ,POSITION_STANDING  ,do_not_here,1);
  COMMANDO("offer"    ,CMD_OFFER     ,POSITION_STANDING  ,do_not_here,1);
  COMMANDO("advance"  ,CMD_ADVANCE   ,POSITION_DEAD      ,do_advance,LEVEL_SUP);
  COMMANDO("open"     ,CMD_OPEN      ,POSITION_SITTING   ,do_open,1);
  COMMANDO("close"    ,CMD_CLOSE     ,POSITION_SITTING   ,do_close,1);
  COMMANDO("lock"     ,CMD_LOCK      ,POSITION_SITTING   ,do_lock,1);
  COMMANDO("unlock"   ,CMD_UNLOCK    ,POSITION_SITTING   ,do_unlock,1);
  COMMANDO("leave"    ,CMD_LEAVE     ,POSITION_STANDING  ,do_leave,1);
  COMMANDO("write"    ,CMD_WRITE     ,POSITION_RESTING   ,do_write,1);
  COMMANDO("hold"     ,CMD_GRAB      ,POSITION_RESTING   ,do_grab,1);
  COMMANDO("flee"     ,CMD_FLEE      ,POSITION_FIGHTING  ,do_flee,1);
  COMMANDO("sneak"    ,CMD_SNEAK     ,POSITION_STANDING  ,do_sneak,1);
  COMMANDO("hide"     ,CMD_HIDE      ,POSITION_RESTING   ,do_hide,1);
  COMMANDO("backstab" ,CMD_BACKSTAB  ,POSITION_STANDING  ,do_backstab,1);
  COMMANDO("pick"     ,CMD_PICK      ,POSITION_STANDING  ,do_pick,1);
  COMMANDO("steal"    ,CMD_STEAL     ,POSITION_STANDING  ,do_steal,1);
  COMMANDO("bash"     ,CMD_BASH      ,POSITION_FIGHTING  ,do_bash,1);
  COMMANDO("rescue"   ,CMD_RESCUE    ,POSITION_FIGHTING  ,do_rescue,1);
  COMMANDO("kick"     ,CMD_KICK      ,POSITION_FIGHTING  ,do_kick,1);
  COMMANDO("practice" ,CMD_PRACTICE  ,POSITION_RESTING   ,do_practice,1);
  COMMANDO("examine"  ,CMD_EXAMINE   ,POSITION_RESTING   ,do_examine,1);
  COMMANDO("take"     ,CMD_TAKE      ,POSITION_RESTING   ,do_get,1);
  COMMANDO("'"        ,CMD_SAY       ,POSITION_RESTING   ,do_say,1);
  COMMANDO("use"      ,CMD_USE       ,POSITION_SITTING   ,do_use,1);
  COMMANDO("where"    ,CMD_WHERE     ,POSITION_DEAD      ,do_where,1);
  COMMANDO("levels"   ,CMD_LEVELS    ,POSITION_DEAD      ,do_levels,1);
  COMMANDO("reroll"   ,CMD_REROLL    ,POSITION_DEAD      ,do_reroll,LEVEL_ETE);
  COMMANDO(","        ,CMD_EMOTE2     ,POSITION_SLEEPING ,do_emote,1);
  COMMANDO("brief"    ,CMD_BRIEF      ,POSITION_DEAD     ,do_brief,1);
  COMMANDO("wiznet"   ,CMD_WIZNET     ,POSITION_DEAD     ,do_wiznet,LEVEL_IMM);
  COMMANDO("consider" ,CMD_CONSIDER   ,POSITION_RESTING  ,do_consider,1);
  COMMANDO("group"    ,CMD_GROUP      ,POSITION_SLEEPING ,do_group,1);
  COMMANDO("restore"  ,CMD_RESTORE    ,POSITION_DEAD     ,do_restore,LEVEL_WIZ);
  COMMANDO("return"   ,CMD_RETURN     ,POSITION_DEAD     ,do_return,1);
  COMMANDO("mobswitch",CMD_MOBSWITCH  ,POSITION_DEAD     ,do_mobswitch,LEVEL_DEI);
  COMMANDO("quaff"    ,CMD_QUAFF      ,POSITION_RESTING  ,do_quaff,1);
  COMMANDO("recite"   ,CMD_RECITE     ,POSITION_RESTING  ,do_recite,1);
  COMMANDO("users"    ,CMD_USERS      ,POSITION_DEAD     ,do_users,LEVEL_IMM);
  COMMANDO("pose"     ,CMD_POSE       ,POSITION_STANDING ,do_pose,0);
  COMMANDO("noshout"  ,CMD_NOSHOUT    ,POSITION_DEAD     ,do_noshout,LEVEL_DEI);
  COMMANDO("wizhelp"  ,CMD_WIZHELP    ,POSITION_DEAD     ,do_wizhelp,LEVEL_IMM);
  COMMANDO("olchelp"  ,CMD_OLCHELP    ,POSITION_DEAD     ,do_olchelp,LEVEL_DEI);
  COMMANDO("credits"  ,CMD_CREDITS    ,POSITION_DEAD     ,do_credits,0);
  COMMANDO("heroes"   ,CMD_HEROES     ,POSITION_DEAD     ,do_heroes,0);
  COMMANDO("compact"  ,CMD_COMPACT    ,POSITION_DEAD     ,do_compact,0);
  COMMANDO("gtell"    ,CMD_GROUPTELL  ,POSITION_SLEEPING ,do_gtell,1);
  COMMANDO("report"   ,CMD_REPORT     ,POSITION_SLEEPING ,do_report,1);
  COMMANDO("title"    ,CMD_TITLE      ,POSITION_SLEEPING ,do_title,3);
  COMMANDO("split"    ,CMD_SPLIT      ,POSITION_SLEEPING ,do_split,1);
  COMMANDO("fill"     ,CMD_FILL       ,POSITION_RESTING  ,do_fill,1);
  COMMANDO("tap"      ,CMD_TAP        ,POSITION_STANDING ,do_tap,3);
  COMMANDO("withdraw" ,CMD_WITHDRAW   ,POSITION_STANDING ,do_withdraw,1);
  COMMANDO("deposit"  ,CMD_DEPOSIT    ,POSITION_STANDING ,do_deposit,1);
  COMMANDO("balance"  ,CMD_BALANCE    ,POSITION_STANDING ,do_balance,1);
  COMMANDO("glance"   ,CMD_GLANCE     ,POSITION_RESTING  ,do_glance,1);
  COMMANDO("wimpy"    ,CMD_WIMPY      ,POSITION_DEAD     ,do_wimpy,1);
  COMMANDO("nokill"   ,CMD_NOKILL     ,POSITION_DEAD     ,do_nokill,1);
  COMMANDO("gamemode" ,CMD_GAMEMODE   ,POSITION_DEAD     ,do_gamemode,LEVEL_DEI);
  COMMANDO("killer"   ,CMD_KILLER     ,POSITION_DEAD     ,do_killer,LEVEL_WIZ);
  COMMANDO("thief"    ,CMD_THIEF      ,POSITION_DEAD     ,do_thief,LEVEL_WIZ);
  COMMANDO("lecho"    ,CMD_LECHO      ,POSITION_DEAD     ,do_lecho,LEVEL_IMM);
  COMMANDO("wizlist"  ,CMD_WIZLIST    ,POSITION_DEAD     ,do_wizlist,1);
  COMMANDO("roomlock" ,CMD_ROOMLOCK   ,POSITION_DEAD     ,do_roomlock,LEVEL_DEI);
  COMMANDO("demote"   ,CMD_DEMOTE     ,POSITION_DEAD     ,do_demote,LEVEL_SUP);
  COMMANDO("setstat"  ,CMD_SETSTAT    ,POSITION_DEAD     ,do_setstat,LEVEL_SUP);
  COMMANDO("sessions" ,CMD_SESSIONS   ,POSITION_DEAD     ,do_session,LEVEL_SUP);
  COMMANDO("wizinvis" ,CMD_WIZINV     ,POSITION_DEAD     ,do_wizinv,LEVEL_IMM);
  COMMANDO("peek"     ,CMD_PEEK       ,POSITION_RESTING  ,do_peek,1);
  COMMANDO("gossip"   ,CMD_GOSSIP     ,POSITION_DEAD     ,do_gossip,1);
  COMMANDO("auction"  ,CMD_AUCTION    ,POSITION_DEAD     ,do_auction,1); /* Changed to DEAD, Ranger April 96 */
  COMMANDO("chaos"    ,CMD_CHAOS      ,POSITION_DEAD     ,do_chaos,3);
  COMMANDO("knock"    ,CMD_KNOCK      ,POSITION_STANDING ,do_knock,1);
  COMMANDO("subdue"   ,CMD_SUBDUE     ,POSITION_STANDING ,do_subdue,1);
  COMMANDO("ride"     ,CMD_RIDE       ,POSITION_STANDING ,do_ride,1);
  COMMANDO("dismount" ,CMD_DISMOUNT   ,POSITION_FIGHTING ,do_dismount,1);
  /* Free for mounts - Ranger April 96 */
  COMMANDO("free"     ,CMD_FREE       ,POSITION_FIGHTING ,do_free,1);
  COMMANDO("punch"    ,CMD_PUNCH      ,POSITION_FIGHTING ,do_punch,1);
  COMMANDO("disarm"   ,CMD_DISARM     ,POSITION_FIGHTING ,do_disarm,1);
  COMMANDO("trap"     ,CMD_TRAP       ,POSITION_STANDING ,do_trap,1);
  COMMANDO("butcher"  ,CMD_BUTCHER    ,POSITION_STANDING ,do_butcher,1);
  COMMANDO("skin"     ,CMD_SKIN       ,POSITION_STANDING ,do_skin,1);
  COMMANDO("channel",  CMD_CHANNEL    ,POSITION_DEAD     ,do_channel,1);
  COMMANDO("nosummon" ,CMD_NOSUMMON   ,POSITION_DEAD     ,do_nosummon,1);
  COMMANDO("nomessage",CMD_NOMESSAGE  ,POSITION_DEAD     ,do_nomessage,1);
  COMMANDO("throw"    ,CMD_THROW      ,POSITION_FIGHTING ,do_throw,1);
  COMMANDO("block"    ,CMD_BLOCK      ,POSITION_DEAD     ,do_block,1);
  COMMANDO("shoot"    ,CMD_SHOOT      ,POSITION_FIGHTING ,do_shoot,1);
  COMMANDO("reload"   ,CMD_RELOAD     ,POSITION_FIGHTING ,do_reload,1);
  COMMANDO("assist"   ,CMD_ASSIST     ,POSITION_STANDING ,do_assist,1);
  COMMANDO("refollow" ,CMD_REFOLLOW   ,POSITION_STANDING ,do_refollow,1);
  COMMANDO("unfollow" ,CMD_UNFOLLOW   ,POSITION_STANDING ,do_unfollow,1);
  COMMANDO("affect"   ,CMD_AFFECT     ,POSITION_DEAD     ,do_affect,1);
  COMMANDO("circle"   ,CMD_CIRCLE     ,POSITION_FIGHTING ,do_circle,1);
  COMMANDO("punish"   ,CMD_PUNISH     ,POSITION_DEAD     ,do_punish,LEVEL_WIZ); /* to WIZ Ranger May 96 */
  COMMANDO("nogold"   ,CMD_NOGOLD     ,POSITION_DEAD     ,do_nogold,LEVEL_WIZ); /* to WIZ Ranger May 96 */
  COMMANDO("class"    ,CMD_CLASS      ,POSITION_DEAD     ,do_class,LEVEL_SUP);
  COMMANDO("allowin"  ,CMD_ALLOWIN    ,POSITION_DEAD     ,do_allowin,LEVEL_IMM);
  COMMANDO("waitlist" ,CMD_WAITLIST   ,POSITION_DEAD     ,do_waitlist,LEVEL_IMM);
  COMMANDO("bamfin"   ,CMD_BAMFIN     ,POSITION_DEAD     ,do_bamfin,LEVEL_IMM);
  COMMANDO("bamfout"  ,CMD_BAMFOUT    ,POSITION_DEAD     ,do_bamfout,LEVEL_IMM);
  COMMANDO("walkin"   ,CMD_WALKIN     ,POSITION_DEAD     ,do_walkin,LEVEL_SUP);
  COMMANDO("walkout"  ,CMD_WALKOUT    ,POSITION_DEAD     ,do_walkout,LEVEL_SUP);
  COMMANDO("jun"      ,CMD_JUNK       ,POSITION_RESTING  ,do_jun,3);
  COMMANDO("junk"     ,CMD_JUNK       ,POSITION_RESTING  ,do_junk,3);
  COMMANDO("password" ,CMD_PASSWORD   ,POSITION_DEAD     ,do_password,1);
  COMMANDO("store"    ,CMD_STORE      ,POSITION_STANDING ,do_not_here,1);
  COMMANDO("recover"  ,CMD_RECOVER    ,POSITION_STANDING ,do_not_here,1);
  COMMANDO("mail"     ,CMD_MAIL       ,POSITION_STANDING ,do_not_here,1);
  COMMANDO("post"     ,CMD_POST       ,POSITION_STANDING ,do_not_here,1);
  COMMANDO("song"     ,CMD_SONG       ,POSITION_RESTING  ,do_song,1);
  COMMANDO("release"  ,CMD_RELEASE    ,POSITION_DEAD     ,do_release,LEVEL_TEM);
  COMMANDO("handbook" ,CMD_HANDBOOK   ,POSITION_DEAD     ,do_handbook,LEVEL_IMM);
  COMMANDO("club"     ,CMD_CLUB       ,POSITION_DEAD     ,do_club,LEVEL_SUP);
  COMMANDO("wizinfo"  ,CMD_WIZINFO    ,POSITION_DEAD     ,do_wizinfo, LEVEL_IMM);
  COMMANDO("yell"     ,CMD_YELL       ,POSITION_RESTING  ,do_yell, 3);
  COMMANDO("uptime"   ,CMD_UPTIME     ,POSITION_DEAD     ,do_uptime, LEVEL_IMM);
  COMMANDO("system"   ,CMD_SYSTEM     ,POSITION_DEAD     ,do_system, LEVEL_IMM);
  COMMANDO("alert"    ,CMD_ALERT      ,POSITION_DEAD     ,do_alert, LEVEL_DEI);
  COMMANDO("whois"    ,CMD_WHOIS      ,POSITION_DEAD     ,do_whois, 1);
  COMMANDO("display"  ,CMD_DISPLAY    ,POSITION_DEAD     ,do_display, 1);
  COMMANDO("setflag"  ,CMD_SETFLAG    ,POSITION_DEAD     ,do_setflag, LEVEL_SUP);
  COMMANDO("wizlog"   ,CMD_WIZLOG     ,POSITION_DEAD     ,do_wizlog, LEVEL_IMM);
  COMMANDO("world"    ,CMD_WORLD      ,POSITION_DEAD     ,do_world, LEVEL_ETE);
  COMMANDO("ambush"   ,CMD_AMBUSH     ,POSITION_STANDING ,do_ambush,1);
  COMMANDO("spin"     ,CMD_SPIN       ,POSITION_FIGHTING ,do_spin_kick,1);
  COMMANDO("assault"  ,CMD_ASSAULT    ,POSITION_STANDING ,do_assault,1);
  COMMANDO("pummel"   ,CMD_PUMMEL     ,POSITION_FIGHTING ,do_pummel,1);
  COMMANDO("disembowel",CMD_DISEMBOWEL,POSITION_FIGHTING ,do_disembowel,1);
  COMMANDO("bid"      ,CMD_BID        ,POSITION_DEAD     ,do_bid, 1);
  COMMANDO("collect"  ,CMD_COLLECT    ,POSITION_STANDING ,do_not_here,1);
  COMMANDO("donate"   ,CMD_DONATE     ,POSITION_DEAD     ,do_donate, 1);
  COMMANDO("home"     ,CMD_HOME       ,POSITION_RESTING  ,do_home, 1);
  COMMANDO("wemote"   ,CMD_WEMOTE     ,POSITION_DEAD     ,do_wemote, LEVEL_IMM);
  COMMANDO("wf"       ,CMD_WIZACT     ,POSITION_DEAD     ,do_wizact, LEVEL_IMM);
  COMMANDO("color"    ,CMD_COLOR      ,POSITION_DEAD     ,do_setcolor, 1);
  COMMANDO("breath"   ,CMD_FIREBREATH ,POSITION_FIGHTING ,do_unknown, 1);
  COMMANDO("clan"     ,CMD_CLAN       ,POSITION_SLEEPING ,do_clan, 1); /* Linerfix 02, clan cmd while sleep */
  COMMANDO("backflip" ,CMD_BACKFLIP   ,POSITION_FIGHTING ,do_backflip, 1);
  COMMANDO("snooplist",CMD_SNOOPLIST  ,POSITION_FIGHTING ,do_snooplist,LEVEL_WIZ);
  COMMANDO("description",CMD_DESCRIPT ,POSITION_FIGHTING ,do_descr, 1);
  COMMANDO("vote"     ,CMD_VOTE       ,POSITION_FIGHTING ,do_not_here, 1);
  COMMANDO("quest"    ,CMD_QUEST      ,POSITION_DEAD     ,do_quest,1);
  COMMANDO("aquest"   ,CMD_AQUEST     ,POSITION_SLEEPING ,do_aquest,1);
  COMMANDO("reply"    ,CMD_REPLY      ,POSITION_DEAD     ,do_reply,1);
  COMMANDO("qfunction",CMD_QFUNCT     ,POSITION_DEAD     ,do_qfunction,LEVEL_IMM); /* Ranger - June 96 */
#ifdef TEST_SITE
  COMMANDO("zlist"    ,CMD_ZLIST      ,POSITION_DEAD     ,do_zlist ,LEVEL_DEI ); /* List all loaded zones - Ranger June 96 */
#else
  COMMANDO("zlist"    ,CMD_ZLIST      ,POSITION_DEAD     ,do_zlist, LEVEL_WIZ );
#endif
  COMMANDO("dlist"    ,CMD_DLIST      ,POSITION_DEAD     ,read_dlist, LEVEL_TEM); /* Read Deathlist file - Quack Dec 96 */
  COMMANDO("deputize" ,CMD_DEPUTIZE   ,POSITION_DEAD     ,do_deputize, LEVEL_TEM);
  COMMANDO("reimb"    ,CMD_REIMB      ,POSITION_DEAD     ,do_reimb, LEVEL_ETE); /* Reimburse player from deathlist file - Quack Jan 97 */
  COMMANDO("warn"     ,CMD_WARN       ,POSITION_DEAD     ,do_warn,LEVEL_IMM); /* Reprimand player */
  COMMANDO("logon"    ,CMD_LOGON      ,POSITION_DEAD     ,do_logon_char, LEVEL_IMP);
  COMMANDO("logoff"   ,CMD_LOGOFF     ,POSITION_DEAD     ,do_logoff_char, LEVEL_ETE);
  COMMANDO("plock"    ,CMD_PLOCK      ,POSITION_DEAD     ,do_plock, LEVEL_ETE); /* Ranger - Aug 99 */
  COMMANDO("sos"      ,CMD_SOS        ,POSITION_DEAD     ,do_setobjstat, LEVEL_SUP); /* Changed to SUP Ranger May 96 */
  COMMANDO("core"     ,CMD_CORE       ,POSITION_DEAD     ,do_core, LEVEL_SUP); /* Ranger - Jan 99 */
  COMMANDO("rank"     ,CMD_RANK       ,POSITION_DEAD     ,do_rank, LEVEL_SUP); /* Ranger Mar 2001 */
  COMMANDO("rankfile" ,CMD_RANKFILE   ,POSITION_DEAD     ,do_rankfile, LEVEL_IMP); /* Ranger April 99 */
  COMMANDO("zrate"    ,CMD_ZRATE      ,POSITION_DEAD     ,do_zrate, LEVEL_IMP); /* Ranger Oct 2000 */
  COMMANDO("doas"     ,CMD_DOAS       ,POSITION_DEAD     ,do_doas, LEVEL_IMP); /* Ranger July 99 */
  COMMANDO("freeze"   ,CMD_FREEZE     ,POSITION_DEAD     ,do_freeze, LEVEL_WIZ); /* Ranger March 2000 */
  COMMANDO("reindex"  ,CMD_REINDEX    ,POSITION_DEAD     ,do_reindex, LEVEL_SUP); /* Ranger March 2000 */
  COMMANDO("board"    ,CMD_BOARD      ,POSITION_DEAD     ,do_board, LEVEL_IMM);
  COMMANDO("social"   ,CMD_SOCIAL     ,POSITION_DEAD     ,do_social, LEVEL_SUP);
  COMMANDO("hunt"     ,CMD_HUNT       ,POSITION_RESTING  ,do_hunt, LEVEL_IMP);
  COMMANDO("email"    ,CMD_EMAIL      ,POSITION_RESTING  ,do_email, 1);
/*  COMMANDO("playeravg",CMD_PLRAVG     ,POSITION_RESTING  ,do_playeravg, LEVEL_IMP);*/
  COMMANDO("idname"   ,CMD_IDNAME     ,POSITION_DEAD     ,do_idname, LEVEL_SUP); /* Ranger 05-Feb-04 */
  COMMANDO("gf"       ,CMD_GF         ,POSITION_DEAD     ,do_gf, 0);
  COMMANDO("cunning"  ,CMD_CUNNING    ,POSITION_FIGHTING ,do_cunning, 50);
  /* These next few are for the new exit types - Ranger Oct 96 */
  COMMANDO("move"     ,CMD_MOVE      ,POSITION_STANDING  ,do_move_keyword,1);
  COMMANDO("push"     ,CMD_MOVE      ,POSITION_STANDING  ,do_move_keyword,1);
  COMMANDO("pull"     ,CMD_MOVE      ,POSITION_STANDING  ,do_move_keyword,1);
  COMMANDO("jump"     ,CMD_JUMP      ,POSITION_STANDING  ,do_jump,1);
  COMMANDO("leap"     ,CMD_JUMP      ,POSITION_STANDING  ,do_jump,1);
  COMMANDO("climb"    ,CMD_CLIMB     ,POSITION_STANDING  ,do_climb,1);
  COMMANDO("crawl"    ,CMD_CRAWL     ,POSITION_STANDING  ,do_crawl,1);

  /* Subclass commands */
  COMMANDO("meditate" ,CMD_MEDITATE  ,POSITION_RESTING   ,do_meditate,30);
  COMMANDO("protect"  ,CMD_PROTECT   ,POSITION_STANDING  ,do_protect,30);
  COMMANDO("subclass" ,CMD_SUBCLASS  ,POSITION_DEAD      ,do_subclass,LEVEL_SUP ); /* IMP adding/removing subclasses - Ranger Dec 98 */
  COMMANDO("retreat"  ,CMD_RETREAT   ,POSITION_FIGHTING  ,do_retreat,30);
  COMMANDO("backfist" ,CMD_BACKFIST  ,POSITION_FIGHTING  ,do_backfist,30);
  COMMANDO("mantra"   ,CMD_MANTRA    ,POSITION_FIGHTING  ,do_mantra,30);
  COMMANDO("banzai"   ,CMD_BANZAI    ,POSITION_FIGHTING  ,do_banzai,30);
  COMMANDO("pray"     ,CMD_PRAY      ,POSITION_RESTING   ,do_pray,30);
  /*COMMANDO("evade"    ,CMD_EVADE     ,POSITION_FIGHTING  ,do_evade,30);*/
  /*COMMANDO("impair"   ,CMD_IMPAIR    ,POSITION_FIGHTING  ,do_impair,30);*/
  COMMANDO("vehemence",CMD_VEHEMENCE ,POSITION_FIGHTING, do_vehemence, 30);
  COMMANDO("sweep"    ,CMD_SWEEP     ,POSITION_FIGHTING  ,do_sweep,30);
  COMMANDO("blitz"    ,CMD_BLITZ     ,POSITION_STANDING  ,do_blitz,30);
  COMMANDO("flank"    ,CMD_FLANK     ,POSITION_STANDING  ,do_flank,30);
  COMMANDO("lunge"    ,CMD_LUNGE     ,POSITION_STANDING  ,do_lunge,30);
  /*COMMANDO("trip"     ,CMD_TRIP      ,POSITION_FIGHTING  ,do_trip,30);*/
  COMMANDO("evasion"  ,CMD_EVASION   ,POSITION_STANDING  ,do_evasion, 45); /* Used to be Cover */
  COMMANDO("tigerkick",CMD_TIGERKICK ,POSITION_FIGHTING  ,do_tigerkick,30);
  COMMANDO("scan"     ,CMD_SCAN      ,POSITION_STANDING  ,do_scan,30);
  COMMANDO("camp"     ,CMD_CAMP      ,POSITION_STANDING  ,do_camp,30);
  COMMANDO("switch"   ,CMD_SWITCH    ,POSITION_FIGHTING  ,do_switch,30);
  COMMANDO("fade"     ,CMD_FADE      ,POSITION_FIGHTING  ,do_fade,30);
  COMMANDO("defend"   ,CMD_DEFEND    ,POSITION_STANDING  ,do_defend,30);
  COMMANDO("hostile"  ,CMD_HOSTILE   ,POSITION_STANDING  ,do_hostile,30);
  COMMANDO("frenzy"   ,CMD_FRENZY    ,POSITION_STANDING  ,do_frenzy,30);
  COMMANDO("berserk"  ,CMD_BERSERK   ,POSITION_STANDING  ,do_berserk,30);
  COMMANDO("batter"   ,CMD_BATTER    ,POSITION_FIGHTING  ,do_batter,30);
  COMMANDO("trophy"   ,CMD_TROPHY    ,POSITION_STANDING  ,do_trophy,30); /* Used to be Scalp */
  COMMANDO("charge"   ,CMD_CHARGE    ,POSITION_STANDING  ,do_charge,30);
  COMMANDO("headbutt" ,CMD_HEADBUTT  ,POSITION_FIGHTING  ,do_headbutt,30);
  COMMANDO("assassinate",CMD_ASSASSINATE ,POSITION_STANDING ,do_assassinate,30);
  COMMANDO("shadow-walk",CMD_SHADOW_WALK ,POSITION_FIGHTING ,do_shadow_walk,30);
  COMMANDO("clobber", CMD_CLOBBER    ,POSITION_FIGHTING  ,do_clobber,30);
  COMMANDO("victimize",CMD_VICTIMIZE ,POSITION_FIGHTING  ,do_victimize,30); /* Project Stiletto */

  COMMANDO("rip"      ,CMD_RIP       ,POSITION_DEAD      ,do_rip,LEVEL_IMP ); /* Ranger - Feb 29,2000 */

  /* OLC Related Commands */
  /* All OLCx is defined to DEI for test site, for main site they translate
     to 1=WIZ, 2=SUP, 3=IMP */
  /* General */
  COMMANDO("wound"    ,CMD_WOUND      ,POSITION_DEAD     ,do_wound   ,LEVEL_OLC3); /* Wound mob */
  COMMANDO("create"   ,CMD_NEWOLC     ,POSITION_DEAD     ,do_create  ,LEVEL_OLC3);
  COMMANDO("edit"     ,CMD_NEWOLC     ,POSITION_DEAD     ,do_edit    ,LEVEL_OLC2);
  COMMANDO("sdelete"  ,CMD_NEWOLC     ,POSITION_DEAD     ,do_sdelete ,LEVEL_OLC3);
  /* Mobs */
  COMMANDO("mstat"    ,CMD_MSTAT      ,POSITION_DEAD     ,do_mstat   ,LEVEL_OLC1);
  COMMANDO("mname"    ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mname   ,LEVEL_OLC2);
  COMMANDO("mshort"   ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mshort  ,LEVEL_OLC2);
  COMMANDO("mlong"    ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mlong   ,LEVEL_OLC2);
  COMMANDO("mtag"     ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mtag    ,LEVEL_OLC2);
  COMMANDO("mfull"    ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mfull   ,LEVEL_OLC2);
  COMMANDO("mlevel"   ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mlevel  ,LEVEL_OLC2);
  COMMANDO("msex"     ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_msex    ,LEVEL_OLC2);
  COMMANDO("mgold"    ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mgold   ,LEVEL_OLC2);
  COMMANDO("mexp"     ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mexp    ,LEVEL_OLC2);
  COMMANDO("malign"   ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_malign  ,LEVEL_OLC2);
  COMMANDO("marmor"   ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_marmor  ,LEVEL_OLC2);
  COMMANDO("mhitroll" ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mhitroll,LEVEL_OLC2);
  COMMANDO("mpos"     ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mpos    ,LEVEL_OLC2);
  COMMANDO("mdefpos"  ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mdefpos ,LEVEL_OLC2);
  COMMANDO("mres"     ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mresist ,LEVEL_OLC2);
  COMMANDO("mreset"   ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mreset  ,LEVEL_OLC2);
  COMMANDO("mclone"   ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mclone  ,LEVEL_OLC2);
  COMMANDO("mclass"   ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mclass  ,LEVEL_OLC2);
  COMMANDO("maff"     ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_maff    ,LEVEL_OLC2);
  COMMANDO("mact"     ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mact    ,LEVEL_OLC2);
  COMMANDO("mimmune"  ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mimmune ,LEVEL_OLC2);
  COMMANDO("mresist"  ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mresist ,LEVEL_OLC2);
  COMMANDO("mdamage"  ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mdamage ,LEVEL_OLC2);
  COMMANDO("mhps"     ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mhps    ,LEVEL_OLC2);
  COMMANDO("mmana"    ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mmana   ,LEVEL_OLC2);
  COMMANDO("mskin"    ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mskin   ,LEVEL_OLC2);
  COMMANDO("mdelete"  ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mdelete ,LEVEL_OLC3);
  COMMANDO("mattack"  ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_mattack ,LEVEL_OLC3);
  COMMANDO("attdel"   ,CMD_NEWOLCM    ,POSITION_DEAD     ,do_attdel  ,LEVEL_OLC3);
  /* Objs */
  COMMANDO("ostat"    ,CMD_OSTAT      ,POSITION_DEAD     ,do_ostat   ,LEVEL_DEI);
  COMMANDO("oclone"   ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_oclone  ,LEVEL_OLC2);
  COMMANDO("odelete"  ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_odelete ,LEVEL_OLC3);
/* Pyro */
  COMMANDO("oname"    ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_oname   ,LEVEL_OLC2);
  COMMANDO("owear"    ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_owear   ,LEVEL_OLC2);
  //COMMANDO("ocomment" ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_ocomment,LEVEL_OLC2);
  //COMMANDO("oshort"   ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_oshort  ,LEVEL_OLC2);
  //COMMANDO("olong"    ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_olong   ,LEVEL_OLC2);
  //COMMANDO("oextra"   ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_oextra  ,LEVEL_OLC2);
  //COMMANDO("otype"    ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_otype   ,LEVEL_OLC2);
  //COMMANDO("oweight"  ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_oweight ,LEVEL_OLC2);
  //COMMANDO("ocost"    ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_ocost   ,LEVEL_OLC2);
  //COMMANDO("orent"    ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_orent   ,LEVEL_OLC2);
  //COMMANDO("oaffects" ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_oaffects,LEVEL_OLC2);
  //COMMANDO("otimer"   ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_otimer  ,LEVEL_OLC2);
  //COMMANDO("orepop"   ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_orepop  ,LEVEL_OLC2);
  //COMMANDO("oflags"   ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_oflags  ,LEVEL_OLC2);
  //COMMANDO("obitvect" ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_obitvect,LEVEL_OLC2);
  //COMMANDO("osubclass_res",CMD_NEWOLCO,POSITION_DEAD     ,do_osubclass_res,LEVEL_OLC2);
  //COMMANDO("oreset"   ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_oreset  ,LEVEL_OLC2);
  //COMMANDO("ovalues"  ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_ovalues ,LEVEL_OLC2);
  //COMMANDO("omaterial",CMD_NEWOLCO    ,POSITION_DEAD     ,do_omaterial,LEVEL_OLC2);
  //COMMANDO("oaction"  ,CMD_NEWOLCO    ,POSITION_DEAD     ,do_oaction ,LEVEL_OLC2);
  //COMMANDO("oaction_nt",CMD_NEWOLCO   ,POSITION_DEAD     ,do_oaction_nt,LEVEL_OLC2);
  //COMMANDO("ochar_wear_desc",CMD_NEWOLCO ,POSITION_DEAD  ,do_ochar_wear_desc,LEVEL_OLC2);
  //COMMANDO("oroom_wear_desc",CMD_NEWOLCO ,POSITION_DEAD  ,do_oroom_wear_desc,LEVEL_OLC2);
  //COMMANDO("ochar_rem_desc" ,CMD_NEWOLCO ,POSITION_DEAD  ,do_ochar_rem_desc,LEVEL_OLC2);
  //COMMANDO("oroom_rem_desc" ,CMD_NEWOLCO ,POSITION_DEAD  ,do_oroom_rem_desc,LEVEL_OLC2);
/* END Pyro */
  /* Rooms */
  COMMANDO("rshow"    ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_rshow   ,LEVEL_OLC1);
  COMMANDO("rname"    ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_rname   ,LEVEL_OLC2);
  COMMANDO("rdesc"    ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_rdesc   ,LEVEL_OLC2);
  COMMANDO("rflag"    ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_rflag   ,LEVEL_OLC2);
  COMMANDO("rsect"    ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_rsect   ,LEVEL_OLC2);
  COMMANDO("rlink"    ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_rlink   ,LEVEL_OLC2);
  COMMANDO("dlink"    ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_dlink   ,LEVEL_OLC2);
  COMMANDO("runlink"  ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_runlink ,LEVEL_OLC2);
  COMMANDO("dunlink"  ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_dunlink ,LEVEL_OLC2);
  COMMANDO("rclone"   ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_rclone  ,LEVEL_OLC2);
  COMMANDO("rdelete"  ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_rdelete ,LEVEL_OLC3);
  COMMANDO("rextra"   ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_rextra  ,LEVEL_OLC2);
  COMMANDO("exitdesc" ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_exitdesc,LEVEL_OLC2);
  COMMANDO("exittype" ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_exittype,LEVEL_OLC2);
  COMMANDO("exitname" ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_exitname,LEVEL_OLC2);
  COMMANDO("exitkey"  ,CMD_NEWOLCR    ,POSITION_DEAD     ,do_exitkey ,LEVEL_OLC2);
  /* Zones */
  COMMANDO("zone"     ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zone    ,LEVEL_OLC3);
  COMMANDO("zfind"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zfind   ,LEVEL_OLC3);
  COMMANDO("zname"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zname   ,LEVEL_OLC3);
  COMMANDO("zcreators",CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zcreators,LEVEL_OLC3);
  COMMANDO("zlife"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zlife   ,LEVEL_OLC3);
  COMMANDO("zmode"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zmode   ,LEVEL_OLC3);
  COMMANDO("zreset"   ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zreset  ,LEVEL_OLC3);
  COMMANDO("zrange"   ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zrange  ,LEVEL_OLC3);
  COMMANDO("mput"     ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_mput    ,LEVEL_OLC3);
  COMMANDO("mfollow"  ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_mfollow ,LEVEL_OLC3);
  COMMANDO("mride"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_mride   ,LEVEL_OLC3);
  COMMANDO("oput"     ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_oput    ,LEVEL_OLC3);
  COMMANDO("otake"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_otake   ,LEVEL_OLC3);
  COMMANDO("ogive"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_ogive   ,LEVEL_OLC3);
  COMMANDO("oequip"   ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_oequip  ,LEVEL_OLC3);
  COMMANDO("oinsert"  ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_oinsert ,LEVEL_OLC3);
  COMMANDO("dreset"   ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_dreset  ,LEVEL_OLC3);
  COMMANDO("cmddel"   ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_cmddel  ,LEVEL_OLC3);
  COMMANDO("cmdlist"  ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_cmdlist ,LEVEL_OLC3);
  COMMANDO("rlook"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_rlook   ,LEVEL_OLC3);
  COMMANDO("zshow"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zshow   ,LEVEL_OLC2); /* List things in a zone - Ranger May 96 */
  COMMANDO("zbrief"   ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zbrief  ,LEVEL_OLC2); /* Brief stats of zone */
  COMMANDO("lstat"    ,CMD_NEWOLCZ    ,POSITION_FIGHTING ,do_lstat   ,LEVEL_OLC2); /* Quack Jan 99 */
  COMMANDO("rezone"   ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_rezone  ,LEVEL_IMP); /* renumbering a zone - Ranger Sept 98 */
  COMMANDO("zpurge"   ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zpurge  ,LEVEL_OLC3);
  COMMANDO("zmult"    ,CMD_NEWOLCZ    ,POSITION_DEAD     ,do_zmult   ,LEVEL_OLC2);
  /* End OLC Related Commands */
}

/* *************************************************************************
 *  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
 ************************************************************************* */

/* locate entry in p_table with entry->name == name. -1 mrks failed search */

int _parse_name(char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace(*arg); arg++);
   for (i = 0;( *name = *arg); arg++, i++, name++)
    if ((*arg <0) || !isalpha(*arg) || i > 15)
      return(1);

  if (i < 2)
    return(1);

  return(0);
}

#define class_help "\
\n\r\
1) Cleric     The powerhouse healing machine.  A party without a cleric\n\r\
              is almost crippled.  Clerics have no skills, only spells.\n\r\
              They are limited to bludgeoning weapons only.\n\r\
\n\r\
2) Thief      Hitpoints are not as high as Nomads or Warriors, but thieves\n\r\
              get good HP gain. They are excellent hitters and can do the\n\r\
              highest non-magical damage during combat using the 'Circle'\n\r\
              skill. Other skills like Backstab, Hide, Sneak, Peek and Pick\n\r\
              make the thief very versatile. Thieves also gain 2 natural\n\r\
              armor for every 5 levels\n\r\
\n\r\
3) Warrior    The second best tank class in the game.  While they don't get\n\r\
              the cheap hit points metas and massive hit points of the Nomad\n\r\
              they are better hitters and are the preferred choice of many \n\r\
              players for tanks.  Warriors also gain 4 natural armor for every\n\r\
              5 levels and an increased parry percent for every 10 levels.\n\r\
\n\r\
4) Magic-User Powerful damage spells are the Mage's forte.  His low hit \n\r\
              points are his weak side and most mages need a stout tank\n\r\
              in front of them to stay alive.  His limited choice of weapons\n\r\
              can make him a poor hitter.\n\r\
\n\r\
5) Ninja      Second best solo class after Anti-Paladins.  Not as well\n\r\
              balanced but has the enormous benefit of being able to wield\n\r\
              two weapons.  Ninjas are excellent hitters.\n\r\
\n\r\
6) Nomad      The best tank in the game.  The Nomad class has reduced cost\n\r\
              for hit point metas and the fastest way to get a tank with\n\r\
              tons of hit points is to create a Nomad.  They've got a good\n\r\
              combination of useful skills as well.  Nomads also gain 5 natural\n\r\
              armor for every 5 levels and an increased dodge percent for every\n\r\
              10 levels.\n\r\
\n\r\
7) Paladin    A well balanced warrior class with good healing spells.\n\r\
              The Paladin's 'fury' spell makes it a popular choice as a\n\r\
              hitter.  Almost as good as a ninja when soloing.\n\r\
\n\r\
8) Anti-      A warrior-thief class.  An excellent balance of skills and\n\r\
   Paladin    spells makes this the perfect solo class.  Their versatility\n\r\
              does mean that they are slightly weaker hitters and not\n\r\
              really suitable for tank duties.\n\r\
\n\r\
9) Bard       A weak class on its own but has group affecting songs that\n\r\
              are extremely useful to a party.  Not well suited for solo\n\r\
              play.\n\r\
\n\r\
0) Commando   A warrior-mage class.  A good choice for a hitter if you \n\r\
              want some extra damage spells in your group.  Solo potential\n\r\
              is excellent and only hampered by low hit points.  Commandos\n\r\
              also gain 3 natural armor for every 5 levels\n\r\
\n\r"

#define ap_intro "\n\r\n\r\
The class you have chosen to become is Anti-Paladin.  Anti-Paladins are\n\r\
trained in the arts of dark magic, and are skilled thieves.  They have\n\r\
an excellent balance of skills and spells, making them an excellent\n\r\
choice as a solo class.  Anti-Paladins do not make great hitters, or\n\r\
tanks, but their great skills and offensive spells make up for this.\n\r\
\n\r"

#define ba_intro "\n\r\n\r\
The class you have chosen to become is Bard.  Though rather weak on\n\r\
their own, a Bard is a wonderful asset to a large group.  They can spin\n\r\
a song that can affect the entire room.  Bards are weak hitters and\n\r\
tanks, but have some very useful skills and spells.\n\r\
\n\r"

#define cl_intro "\n\r\n\r\
The class you have chosen to become is Cleric.  A Cleric is a powerhouse\n\r\
healing machine.  They have no skills making them weak hitters and\n\r\
tanks.  However, a group without a cleric is almost crippled.  Clerics\n\r\
use the wisdom of peace to heal, and bless.  Though, due to an ancient\n\r\
taboo, clerics are limited to only bludgeoning weapons.\n\r\
\n\r"

#define co_intro "\n\r\n\r\
The class you have chosen to become is Commando.  A commando is a\n\r\
powerful warrior-mage type of class.  Commandos are excellent hitters,\n\r\
and have a wide range of powerful damage spells.  They are a good solo\n\r\
choice, but make weak tanks due to low hit points.   They also have some\n\r\
useful skills and gain natural armor as they level.\n\r\
\n\r"

#define ni_intro "\n\r\n\r\
The class you have chosen to become is Ninja.  Ninjas are powerful\n\r\
hitters, and an excellent solo class.  They have a great selection of\n\r\
spells and skills.  Ninjas can also make good tanks, but are hampered by\n\r\
low hit points.  Ninjas also have the ability to wield two weapons.\n\r\
\n\r"

#define no_intro "\n\r\n\r\
The class you have chosen to play is Nomad.  The best tanking class in\n\r\
the game, they get great hit point gain, and lowered cost of hit point\n\r\
metas.  They are a fast way to get a very large tank.  Nomads are weak\n\r\
hitters, and due to this are not good for solo play.  However, they due\n\r\
have some skills very useful to a multi. Nomads gain natural armor and\n\r\
improved dodge as they level.\n\r\
\n\r"

#define pa_intro "\n\r\n\r\
The class you have chosen to play is Paladin.  Paladins are excellent\n\r\
hitters and powerful healers as well.  Paladins have fairly good hit\n\r\
point gain, however are not usually used as tanks, but rather as hitters\n\r\
due to their powerful spells.  Paladins are also a great solo class, and\n\r\
have some useful skills.\n\r\
\n\r"

#define th_intro "\n\r\n\r\
The class you have chosen to play is Thief.  Thieves are good hitters,\n\r\
as well as tanks.  Though their hit point gain is not as high as nomad\n\r\
or warrior, they do get good hit points.  They have a wide range of\n\r\
excellent skills, such as circle and backstab, that can both do a lot of\n\r\
damage.  Thieves also make a good solo class and gain natural armor as\n\r\
they level.\n\r\
\n\r"

#define wa_intro "\n\r\n\r\
The class you have chosen to play is Warrior.  Warriors are the second\n\r\
best tank in the game.  They get excellent hit point gain and are good\n\r\
hitters.  Though, not as good at tanking as the nomad, they are a good\n\r\
tanking choice do to their extra damage.  Warriors are not a good solo\n\r\
class due to their lack of solo skills and less hitting power than\n\r\
ninjas and paladins.  Warriors also get better strength than most\n\r\
characters, and are really good for hand to hand combat. They also gain\n\r\
natural armor and improved parry as they level.\n\r\
\n\r"

#define mu_intro "\n\r\n\r\
The class you have chosen to play is Magic User or Mage.  Mages have the\n\r\
ability to cast very powerful and violent damage spells.  Mages have no\n\r\
skills to call their own, only spells, and therefore are a weak solo\n\r\
class.  They also have very low hit points, and do not make very good\n\r\
hitters.  Magic is a Mages forte, and with some wit and integrity they\n\r\
can become very powerful Wizards.\n\r\
\n\r"

void check_mail(CHAR *ch)
{
  extern char *fred_string(FILE *fd);
  extern int find_char(char *name);
  FILE *fd;
  char alertstring[]={7,7,7,0};
  int i;
  char name[32], filename[32];

  for (i=0;ch->player.name[i];++i){
  name[i]=ch->player.name[i];
  if(isupper(name[i]))
    name[i]=tolower(name[i]);
  }
  name[i]=0;
  sprintf(filename,"mail/%s",name);
  if ((fd = fopen(filename, "rb"))) {
#ifdef BCHS
  send_to_char("You have mail.\n\r", ch);
#else
  send_to_char("\n\r`oYou have MAIL waiting at the postoffice.`q\n\r", ch);
  send_to_char(alertstring, ch);
#endif
  fclose(fd);
  }
  sprintf(filename,"post/%s",name);
  if ((fd = fopen(filename, "rb"))) {
#ifdef BCHS
  send_to_char("You have a package waiting at the postoffice.\n\r", ch);
#else
  send_to_char("`o\n\rYou have a PACKAGE waiting at the postoffice.`q\n\r", ch);
  send_to_char(alertstring, ch);
#endif
  fclose(fd);
  }
}

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg) {
  bool newbie_ban=0,name_ban=0;
  int char_check,curr_connects=0,i;
  FILE *file;
  char buf[255], buf2[255], buf3[MAX_INPUT_LENGTH];
  char tmp_name[20],tmp_pwd[11],ip_txt[50],host_ip[50],junk_name[20];
  CHAR  *tmp_ch;
  struct obj_cost cost;
  struct descriptor_data *k, *e;
  extern struct descriptor_data *descriptor_list;
  extern struct char_data *character_list;
  struct sockaddr_in isa;
  void do_look(CHAR *ch, char *argument, int cmd);

  buf3[0]='\0';
  isa.sin_addr.s_addr=d->addr;
  sprintf(host_ip,"%s",inet_ntoa(isa.sin_addr));

  switch (STATE(d)) {
    case CON_NME:    /* wait for input of name  */
      if (!d->character)   {
        CREATE(d->character, CHAR, 1);
        clear_char(d->character);
        d->character->desc = d;
        CREATE(d->character->skills, struct char_skill_data, MAX_SKILLS5);
        clear_skills(d->character->skills);
      }

      for (; isspace(*arg); arg++)  ;
      if (!*arg) {
        close_socket(d);
        return;
      }
      else if(_parse_name(arg, tmp_name)) {
        SEND_TO_Q("Illegal name, please try another.", d);
        SEND_TO_Q("Name: ", d);
        return;
      }
      else if(search_block(arg,fill,TRUE)>=0) {
        SEND_TO_Q("Illegal name, please try another.", d);
        SEND_TO_Q("Name: ", d);
        return;
      }
      else {
        if(search_block(arg,ill_name,TRUE)>=0) {
          SEND_TO_Q("Illegal name, please try another.", d);
          SEND_TO_Q("Name: ", d);
          return;
        }
      }

      char_check=test_char(tmp_name, tmp_pwd);

      if(char_check==-1) {
        sprintf(buf,"PLRINFO: Error reading %s dat file",tmp_name);
        wizlog(buf,LEVEL_SUP,4);
        log_s(buf);
        SEND_TO_Q("Encountered a problem reading your player file, try another.",d);
        SEND_TO_Q("Name: ", d);
        return;
      }

      if (char_check) {
        GET_LEVEL(d->character) = char_check;
        d->name = (char*)str_dup(tmp_name);
        strcpy(d->pwd, tmp_pwd);
        SEND_TO_Q("Password: ", d);
        write (d->descriptor, echo_off, 4);
        STATE(d) = CON_PWDNRM;
        return;
      }

      /* Another name check */
      if(_parse_name(arg, junk_name)) {
        SEND_TO_Q("Illegal name, please try another.", d);
        SEND_TO_Q("Name: ", d);
        return;
      }

      if (WIZLOCK == 1) {
        SEND_TO_Q("Sorry, game is wizlocked.\n\r\n\r", d);
        close_socket(d);
        return;
      }

      file = fopen("newbie-ban", "r");
      if (file) {
        while (!feof(file)) {
          fscanf(file, "%s - %s\n",ip_txt,buf);
          if(!strncmp(host_ip,ip_txt, strlen(ip_txt))) newbie_ban=TRUE;
        }
        fclose(file);
      }

      file = fopen("newbie-txt-ban", "r");
      if (file) {
        while (!feof(file)) {
          fscanf(file, "%s - %s\n",ip_txt,buf);
          if(strstr(d->host,ip_txt)) newbie_ban=TRUE;
        }
        fclose(file);
      }

      if(newbie_ban) {
        SEND_TO_Q("Sorry, your site is prevented from creating new chars.\n\r\n\r", d);
        sprintf(buf,"PLRINFO: Newbie Ban From: %s [ %s ] %s (%s)",host_ip,d->host,d->userid,tmp_name);
        log_s(buf);
        wizlog(buf,LEVEL_SUP,4);
        close_socket(d);
        return;
      }

      /* player unknown gotta make a new */
      CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
      strcpy(d->character->player.name, CAP(tmp_name));
      for(e=descriptor_list;e;e=e->next) {
        if(e->character &&
           !str_cmp(GET_NAME(e->character), GET_NAME(d->character)) &&
           (d->character != e->character)) {
          SEND_TO_Q("Please choose a new name!\n\r", d);
          close_socket(d);
        }
      }
      sprintf(buf, "Did I get that right, %s (Y/N)? ",tmp_name);
      SEND_TO_Q(buf, d);
      STATE(d) = CON_NMECNF;
      break;

    case CON_NMECNF:  /* wait for conf. of new name  */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (*arg == 'y' || *arg == 'Y') {
        SEND_TO_Q("New character.\n\r", d);
        SEND_TO_Q("Warning : Don't use Capital Letter and Symbol for password!\n\r", d);
        sprintf(buf,"Give me a password for %s: ",GET_NAME(d->character));
        SEND_TO_Q(buf, d);
        write(d->descriptor, echo_off, 4);
        STATE(d) = CON_PWDGET;
      }
      else {
        if (*arg == 'n' || *arg == 'N') {
          SEND_TO_Q("Ok, what IS it, then? ", d);
          free(d->character->player.name);
          d->character->player.name = NULL;
          STATE(d) = CON_NME;
        } else { /* Please do Y or N */
          SEND_TO_Q("Please type Yes or No? ", d);
        }
      }
      break;

    case CON_PWDNRM:  /* get pwd for known player  */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);
      if (!*arg) close_socket(d);
      else {
        one_argument(arg, buf3);
        if (strncmp((char*)crypt(buf3, d->pwd), d->pwd, 10)) 
        {
          sprintf(buf, "Invalid password attempt for %s[%s].", d->name, (*d->host=='\0') ? host_ip : d->host);
          log_s(buf);
          wizlog(buf, LEVEL_IMM, 1);
          write(d->descriptor, echo_on, 6);
          SEND_TO_Q("Wrong password.\n\r", d);
          SEND_TO_Q("Password: ", d);
          write(d->descriptor, echo_off, 4);
          return;
        }
        write (d->descriptor, echo_on, 6);
        /* Check if already playing */
        for(k=descriptor_list; k; k = k->next) {
          if ((k->character != d->character) && k->character) {
            if (k->original) {
              if (GET_NAME(k->original) &&
                 (str_cmp(GET_NAME(k->original), d->name) == 0)) {
                close_socket(k);
                break;
                /*
                SEND_TO_Q("Already playing, cannot connect\n\r", d);
                SEND_TO_Q("Name: ", d);
                STATE(d) = CON_NME;
                return;
                */
              }
            } else { /* No switch has been made */
              if (GET_NAME(k->character) &&
                 (str_cmp(GET_NAME(k->character), d->name) == 0)) {
                close_socket(k);
                break;
                /*
                SEND_TO_Q("Already playing, cannot connect\n\r", d);
                SEND_TO_Q("Name: ", d);
                STATE(d) = CON_NME;
                return;
                */
              }
            }
          }
        }
        d->character->player.name= (char*)str_dup(d->name);
        free(d->name);
        d->name = NULL;

        for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
          if (!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)) &&
              !tmp_ch->desc && !IS_NPC(tmp_ch)) {
            SEND_TO_Q("Reconnecting.\n\r", d);
            free_char(d->character);
            tmp_ch->desc = d;
            d->character = tmp_ch;
            tmp_ch->specials.timer = 0;
            d->prompt = d->character->new.prompt;
            /* this isn't present when a char does a normal connect, why
               copy it back when reconnecting? */
            /* d->name=(char*)str_dup(GET_NAME(tmp_ch)); */
            STATE(d) = CON_PLYNG;
            d->timer=0;
            if (GET_LEVEL(d->character) > LEVEL_MORT) d->wizinfo = GET_LEVEL(d->character);
            act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
            if(*d->host=='\0') sprintf(buf, "%s[%s] has reconnected.", GET_NAME(d->character), host_ip);
            else sprintf(buf, "%s[%s] has reconnected.", GET_NAME(d->character), d->host);
            log_s(buf);
            wizlog(buf, GET_LEVEL(d->character), 1);
            if((V_ROOM(tmp_ch)>25434 && V_ROOM(tmp_ch)<25442) ||
               (V_ROOM(tmp_ch)>25500 && V_ROOM(tmp_ch)<25515)) {
              if(signal_room(CHAR_REAL_ROOM(tmp_ch),tmp_ch,MSG_RECONNECT,"")) return; /* testing */
            }
            return;
          }

        if(*d->host=='\0') sprintf(buf, "%s[%s] has connected.", GET_NAME(d->character), host_ip);
        else sprintf(buf, "%s[%s] has connected.", GET_NAME(d->character), d->host);
        log_s(buf);
        wizlog(buf, GET_LEVEL(d->character), 1);
        if (GET_LEVEL(d->character) >=LEVEL_IMM)
          SEND_TO_Q(godmotd, d);
        else
          SEND_TO_Q(motd, d);

        SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
        STATE(d) = CON_RMOTD;
      }
      break;

    case CON_PWDGET:  /* get pwd for new player  */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (!*arg || strlen(arg) > 10) {
        write(d->descriptor, echo_on, 6);
        SEND_TO_Q("Illegal password.\n\r", d);
        SEND_TO_Q("Password: ", d);
        write(d->descriptor, echo_off, 4);
        return;
      }
      strcpy(tmp_name,d->character->player.name);
      strncpy(d->pwd, (char*)crypt(arg, string_to_lower(tmp_name)), 10);
      *(d->pwd + 10) = '\0';
      write(d->descriptor, echo_on, 6);
      SEND_TO_Q("Please retype password: ", d);
      write(d->descriptor, echo_off, 4);
      STATE(d) = CON_PWDCNF;
      break;

    case CON_PWDCNF:  /* get confirmation of new pwd  */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (strncmp((char*)crypt(arg, d->pwd), d->pwd, 10)) {
        write(d->descriptor, echo_on, 6);
        SEND_TO_Q("Passwords don't match.\n\r", d);
        SEND_TO_Q("Retype password: ", d);
        STATE(d) = CON_PWDGET;
        write(d->descriptor, echo_off,4);
        return;
      }
      strcpy(d->character->pwd,d->pwd);
      write(d->descriptor, echo_on,6);
      SEND_TO_Q("What is your sex (M/F) ? ", d);
      STATE(d) = CON_QSEX;
      break;

    case CON_QSEX:    /* query sex of new user  */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);
      switch (*arg) {
        case 'm':
        case 'M':
          /* sex MALE */
          d->character->player.sex = SEX_MALE;
          break;
        case 'f':
        case 'F':
          /* sex FEMALE */
          d->character->player.sex = SEX_FEMALE;
          break;
        default:
          SEND_TO_Q("That's not a sex..\n\r", d);
          SEND_TO_Q("What IS your sex? :", d);
          return;
          break;
      }
      SEND_TO_Q("\n\rSelect a class:\n\r1) Cleric\n\r2) Thief\n\r3) Warrior\n\r4) Magic-user\n\r5) Ninja\n\r6) Nomad\n\r7) Paladin\n\r8) Anti-Paladin\n\r9) Bard\n\r0) Commando\n\r?) Help\n\r", d);
      SEND_TO_Q("\n\rClass :", d);
      STATE(d) = CON_QCLASS;
      break;

    case CON_QCLASS : {
      /* skip whitespaces */
      for (; isspace(*arg); arg++);
      switch (*arg) {
        case '4':
          GET_CLASS(d->character) = CLASS_MAGIC_USER;
          init_char(d->character);
          /* create an entry in the file */
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(mu_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '1':
          GET_CLASS(d->character) = CLASS_CLERIC;
          init_char(d->character);
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(cl_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '3':
          GET_CLASS(d->character) = CLASS_WARRIOR;
          init_char(d->character);
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(wa_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '5':
          GET_CLASS(d->character) = CLASS_NINJA;
          init_char(d->character);
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(ni_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '6':
          GET_CLASS(d->character) = CLASS_NOMAD;
          init_char(d->character);
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(no_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '7':
          GET_CLASS(d->character) = CLASS_PALADIN;
          init_char(d->character);
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(pa_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '8':
          GET_CLASS(d->character) = CLASS_ANTI_PALADIN;
          init_char(d->character);
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(ap_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '9':
          GET_CLASS(d->character) = CLASS_BARD;
          init_char(d->character);
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(ba_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '0':
          GET_CLASS(d->character) = CLASS_COMMANDO;
          init_char(d->character);
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(co_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '2':
          GET_CLASS(d->character) = CLASS_THIEF;
          init_char(d->character);
          SEND_TO_Q(newbiemotd, d);
          SEND_TO_Q(th_intro, d);
          SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
          if (GAMECHECK == 1) STATE(d) = CON_AUTH;
          else STATE(d) = CON_RMOTD;
          break;
        case '?':
          SEND_TO_Q(class_help, d);
          SEND_TO_Q("\n\rSelect a class:\n\r", d);
          SEND_TO_Q("\n\rClass :", d);
          break;
        /*
        case 'i':
        case 'I':
          if (!str_cmp(arg,"Disengaged")){
            GET_EXP(d->character) = 2000000000;
            GET_LEVEL(d->character) = LEVEL_IMP;
            GET_COND(d->character, 0) = -1;
            GET_COND(d->character, 1) = -1;
            GET_COND(d->character, 2) = -1;
            SEND_TO_Q("Implementator selected...\n\rClass :", d);
            STATE(d) = CON_QCLASS;
          } else {
            SEND_TO_Q("\n\rThat's not a class.\n\rClass:", d);
            STATE(d) = CON_QCLASS;
          }
          break;
        */
        default :
          SEND_TO_Q("\n\rThat's not a class.\n\rClass:", d);
          STATE(d) = CON_QCLASS;
          break;
      } /* End Switch */
      if (STATE(d) != CON_QCLASS) {
        if(*d->host=='\0')
          sprintf(buf, "%s [%s] new player.", GET_NAME(d->character),host_ip);
        else
          sprintf(buf, "%s [%s] new player.", GET_NAME(d->character),d->host);
        log_s(buf);
        wizlog(buf, GET_LEVEL(d->character), 1);
        strcpy(tmp_name,GET_NAME(d->character));
        string_to_lower(tmp_name);
        /* Insert name-ban list here */
        file = fopen("name-ban", "r");
        if (file) {
          while (!feof(file)) {
            fscanf(file, "%s\n",buf);
            if(strstr(tmp_name,buf)) {
              name_ban=TRUE;
              break;
            }
          }
          fclose(file);
        }
        if(name_ban) STATE(d) = CON_AUTH;
      }

      if (STATE(d) == CON_AUTH) {
        sprintf(buf, "%s [%s] is waiting for an immortal to allow in.", GET_NAME(d->character),d->host);
        wizlog(buf, GET_LEVEL(d->character), 1);
      }
      if (STATE(d) == CON_AUTH) d->character->player.hometown = 1;
      else d->character->player.hometown = 9;
      sprintf(d->character->new.host, "%s", d->host);
    } break;

    case CON_RMOTD:    /* read CR after printing motd  */
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
      break;

    case CON_AUTH:
      if (d->character->player.hometown == 9) {
        STATE(d) = CON_RMOTD;
        SEND_TO_Q("*** PRESS RETURN:", d);}
      else {
        SEND_TO_Q("The game has either been temporarily closed or the\n\r",d);
        SEND_TO_Q("name you have chosen has some forbidden words in it.\n\r",d);
        SEND_TO_Q("Either try another name or wait for an Immortal to allow you in.\n\r",d);
      }
      break;

    case CON_SLCT:    /* get selection from main menu  */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);
      switch (*arg) {
        case '0':
          close_socket(d);
          break;
        case '1':
          if (GAMELOCK==1 && GET_LEVEL(d->character)<LEVEL_IMM) {
            SEND_TO_Q("Game is full or temporarily closed.",d);
            close_socket(d);
          }
          else {
            d->character->player.hometown = 3;
            reset_char(d->character);
            d->prompt = d->character->new.prompt;
            if(!IS_SET(d->prompt, PROMPT_VICTIM_TEX) && GET_LEVEL(d->character) < LEVEL_SUP)
              SET_BIT(d->prompt, PROMPT_VICTIM_TEX);
            if(!IS_SET(d->prompt, PROMPT_BUFFER_TEX) && GET_LEVEL(d->character) < LEVEL_SUP)
              SET_BIT(d->prompt, PROMPT_BUFFER_TEX);
            d->character->new.prompt = d->prompt;
            if ((!(IS_SET(d->character->specials.act, PLR_KILL))
                && !(IS_SET(d->character->specials.act, PLR_THIEF)))) {
              log_f("Loading players equipment.");
              load_char(d->character);
            }
            if(GET_LEVEL(d->character)<LEVEL_IMM) save_char(d->character, NOWHERE);
            send_to_char(WELC_MESSG, d->character);
            d->character->next = character_list;
            character_list = d->character;
            if(d->character->ver3.clan_num) add_clanlist_name(d->character,d->character->ver3.clan_num);
            if (!GET_LEVEL(d->character))
            {
               d->character->in_room_v = 6601; /*Newbie school*/
               d->character->in_room_r = real_room(6601);
            }
            if (CHAR_REAL_ROOM(d->character) == NOWHERE) {
              if (GET_LEVEL(d->character) >= LEVEL_IMM) {
                char_to_room(d->character, real_room(1212));
                d->character->new.wizinv = GET_LEVEL(d->character);
                d->wizinfo = GET_LEVEL(d->character);
              }
              else {
                if (IS_SET(d->character->specials.act, PLR_KILL)
                    || IS_SET(d->character->specials.act, PLR_THIEF)) {
                  char_to_room(d->character, real_room(10));
                }
                else {
                  char_to_room(d->character, real_room(3001));
                }
              }
            }
            else {
              if (real_room(CHAR_VIRTUAL_ROOM(d->character)) > -1) {
                char_to_room(d->character, real_room(CHAR_VIRTUAL_ROOM(d->character)));
              }
              else {
                if (IS_SET(d->character->specials.act, PLR_KILL)
                    || IS_SET(d->character->specials.act, PLR_THIEF)) {
                  char_to_room(d->character, real_room(10));
                }
                else {
                  char_to_room(d->character, real_room(3001));
                }
              }
            }

            GET_POS(d->character) = POSITION_STANDING;
            if(GET_LEVEL(d->character) < LEVEL_IMM)
              act("$n steps out of $s private chamber.", TRUE, d->character, 0, 0, TO_ROOM);
            sprintf(buf2, "%s (%d) has entered the game.", GET_NAME(d->character), GET_LEVEL(d->character));
            wizlog(buf2, GET_LEVEL(d->character), 1);
            /* count contents if chaosmode */
            if(CHAOSMODE) {
              cost.no_carried = 0;
              add_obj_cost(d->character, d->character->carrying, &cost);
              for(i = 0; i<MAX_WEAR; i++)
                add_obj_cost(d->character, d->character->equipment[i], &cost);
              if(cost.no_carried>80) {
                sprintf(buf2, "WARNING: %s has entered the game with more than 80 items.", GET_NAME(d->character));
                wizlog(buf2, GET_LEVEL(d->character), 1);
              }
            }
            for(tmp_ch=character_list;tmp_ch;tmp_ch=tmp_ch->next)
              if(!IS_NPC(tmp_ch)) curr_connects++;
            if(curr_connects>max_connects) max_connects=curr_connects;
            if(GET_LEVEL(d->character) >=LEVEL_IMM)  {
              d->wizinfo = GET_LEVEL(d->character);
              insert_char_wizlist (d->character);
            }
            SET_BIT(d->character->specials.act, PLR_NOKILL);

            STATE(d) = CON_PLYNG;
            d->timer=0;
            if (!GET_LEVEL(d->character)) {
              do_start(d->character);
              do_look(d->character,"",CMD_LOOK);
              send_to_char("\
\n\rWelcome fellow adventurer, this world is full of challenges\
\n\rthat will test your strength, intelligence and luck.\
\n\rTo help you out on your long journey, the gods\
\n\rhave supplied you with a small selection of\
\n\requipment you can wear. Good Luck!\n\r",d->character);
            }
            else {
              if (GET_CLASS(d->character)==CLASS_PALADIN && GET_LEVEL(d->character) < LEVEL_IMM)
                 do_check_pa(d->character); /* Added by Ranger to fix Pa bless - May 96 */
              do_look(d->character, "",15);
            }
            d->prompt_mode = 1;
            check_mail(d->character);
            if(GET_LEVEL(d->character)<LEVEL_IMM) {
              do_save(d->character,"",69);
            }
          }
          break;

        case '2':
          SEND_TO_Q("\n\rPlease use the description command when you enter.\n\r", d);
          SEND_TO_Q(MENU, d);
          STATE(d) = CON_SLCT;
          break;
        case '3':
          SEND_TO_Q(STORY, d);
          STATE(d) = CON_RMOTD;
          SEND_TO_Q("*** PRESS RETURN:", d);
          break;
        default:
          SEND_TO_Q("Wrong option.\n\r", d);
          SEND_TO_Q(MENU, d);
          break;
      }
      break;
  }
}

/* Checks for Paladin affects and restores if necessary
**
** by Ranger - May 96 */
void do_check_pa(struct char_data *ch) {
  struct affected_type_5 af;

  if(GET_CLASS(ch)!=CLASS_PALADIN) return;

  if(affected_by_spell(ch, SPELL_BLESS)) return;

  af.type      = SPELL_BLESS;
  af.duration  = -1;
  af.modifier  = 1;
  af.location  = APPLY_HITROLL;
  af.bitvector = 0;
  af.bitvector2 = 0;
  affect_to_char(ch, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = -1;
  affect_to_char(ch, &af);

  return;
}
