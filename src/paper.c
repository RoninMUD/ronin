/*
 * board.c - Patched board.c
 *
 * Added read p,s,n Sept 98 - Ranger
 *
 */

/*
$Author: ronin $
$Date: 2004/02/05 16:09:52 $
$Header: /home/ronin/cvs/ronin/paper.c,v 2.0.0.1 2004/02/05 16:09:52 ronin Exp $
$Id: paper.c,v 2.0.0.1 2004/02/05 16:09:52 ronin Exp $
$Name:  $
$Log: paper.c,v $
Revision 2.0.0.1  2004/02/05 16:09:52  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "act.h"
#include "modify.h"
#include "interpreter.h"
#include "utility.h"
#include "handler.h"
#include "paper.h"
#include "spells.h"
#include "cmd.h"
#include "spec_assign.h"

#define MAX_MSGS 15	               /* Max number of messages.          */
#define SAVE_PAPER "paper.messages"
#define BOARD_OBJECT  xxx           /* what are we?                     */
#define BOARD_ROOM    xxx           /* where are we?                    */
#define MAX_MESSAGE_LENGTH 2048     /* that should be enough            */

char *msgspaper[MAX_MSGS];
char *headpaper[MAX_MSGS];
char *bylinepaper[MAX_MSGS];
int paper_msg_num;

void paper_write_msg(CHAR *ch, char *arg);
void paper_reset_board();
void board_save_paper();
int paper_remove_msg(CHAR *ch, char *arg);
void board_load_paper();

/* ********************************************************* */


int newspaperboy(CHAR *boy,CHAR *ch,int cmd,char *arg)
{
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;
  char *temp_pointer;
  char *description_pointer;
  int i,x;
  static int counter=0;
  static int timer_counter=0;
#ifndef PAPER_COST
#define PAPER_COST 50
#endif

#ifndef PAPER_OBJ_NUMBER
#define PAPER_OBJ_NUMBER 8
#endif

#ifndef TIMER_HEARTBEATS
#define TIMER_HEARTBEATS 3
#endif

  board_load_paper();
  /*Put his combat stuff here*/

  if (!cmd)
    {
    if(boy->specials.fighting) {
      act("$n says 'GO AWAY, I'm workin' here.'\n\rHe recites a scroll of recall.",FALSE,boy,0,0,TO_ROOM);
      act("You say 'GO AWAY, I'm workin' here.'\n\rYou recite a scroll of recall.",FALSE,boy,0,0,TO_CHAR);
      spell_word_of_recall(30,boy,boy->specials.fighting, 0);
      return FALSE;
    }
      if(timer_counter++<TIMER_HEARTBEATS)
	return(FALSE);
      timer_counter = 0;
      if ((counter&0x1)==0)
	{
          sprintf(buf,"Read all about it!");
          do_say(boy,buf,0);
	}
      else
	{
          sprintf(buf,"%s!",headpaper[counter/2]);
          do_say(boy,buf,0);
	}
      if (counter++>=paper_msg_num*2)
	counter = 0;
      return(FALSE);
    }
  switch (cmd)
    {
    case MSG_TICK:
      if(boy->specials.timer>0) boy->specials.timer--;
      if(boy->specials.timer==5)
        do_yell(boy,"HEAR YE, HEAR YE: The game will reboot in about 5 minutes.",CMD_YELL);
      if(boy->specials.timer==2)
        do_yell(boy,"HEAR YE, HEAR YE: The game will reboot in about 2 minutes.",CMD_YELL);
      if(boy->specials.timer==1)
        do_yell(boy,"HEAR YE, HEAR YE: The game will reboot in about 1 minute.",CMD_YELL);
      break;
    case CMD_LIST:  /* List */
      sprintf(buf,"You want a paper? %d coins.",PAPER_COST);
      do_say(boy,buf,0);
      return(TRUE);
      break;
    case CMD_BUY:
      one_argument(arg,buf);
      if (!strcmp(buf,"paper"))
	{ /* Buy */
	  if (GET_GOLD(ch) < PAPER_COST )
	    {
              sprintf(buf,"Ok %s, I gotta work for a living too!",ch->player.name);
	      do_say(boy,buf,0);
	      act("$n kicks $N on the shin.",FALSE,boy,0,ch,TO_NOTVICT);
              act("$n kicks you on the shin.",FALSE,boy,0,ch,TO_VICT);
              return(TRUE);
	    }
	  else
	    {
	      /*create paper*/
	      temp_pointer = (char*)malloc(10000);
	      temp_pointer[0] = 0;
	      strcat(temp_pointer,"                             The Midgaard Herald\n\r--------------------------------------------------------------------------------\n\r");
        for (i=0;i< paper_msg_num;i++)  /*Copy each headline, and text onto a buffer pointed to by temp_pointer*/
          {
          for(x=40-strlen(headpaper[i])/2-8;x>0;x--)
            {
  		      strcat(temp_pointer," ");
    		    }
          strcat(temp_pointer,"*****");
          strcat(temp_pointer,headpaper[i]);
          strcat(temp_pointer,"*****\n\rby ");
          strcat(temp_pointer,bylinepaper[i]);
          strcat(temp_pointer,"\n\r\n\r");
          if(*msgspaper[i]) strcat(temp_pointer,msgspaper[i]);
          else break;
    		}
	      strcat(temp_pointer,                                   "--------------------------------------------------------------------------------\r\n");
              obj = read_object(PAPER_OBJ_NUMBER,VIRTUAL);
	      description_pointer=(char*)str_dup(temp_pointer);
	      free(temp_pointer);
              obj->action_description=description_pointer;
	      /*give paper*/
	      obj_to_char(obj,ch);
	      GET_GOLD(ch)-=PAPER_COST;
	      sprintf(buf,"Here's your paper.");
              do_say(boy,buf,0);
              act("$n gives $p to $N.",FALSE,boy,obj,ch,TO_NOTVICT);
              act("$n gives $p to you.",FALSE,boy,obj,ch,TO_VICT);
	      return(TRUE);
	    }
          break;
	}
    }
  return(FALSE);
}

int paperboard(struct obj_data *board,CHAR *ch, int cmd, char *arg)
{
  board_load_paper();
  if(!ch)
    return(FALSE);
  if (!ch->desc)
    return(FALSE); /* By MS or all NPC's will be trapped at the board */

  switch (cmd) {
    case CMD_LOOK:  /* look */
        board_save_paper();
	return(paper_show_board(ch, arg));
    case CMD_WRITE: /* write */
	paper_write_msg(ch, arg);
        return 1;
    case CMD_READ: /* read */
	return(paper_display_msg(ch, arg));
    case CMD_REMOVE: /* remove */
      return(paper_remove_msg(ch, arg));
    default:
      return 0;
  }
}


void paper_write_msg(CHAR *ch, char *arg) {
  char playername[35];

  if (paper_msg_num > MAX_MSGS - 1) {
    send_to_char("The board is full already.\n\r", ch);
    return;
  }
  /* skip blanks */
  for(; isspace(*arg); arg++);
  if (!*arg) {
    send_to_char("We must have a headline!\n\r", ch);
    return;
  }

  if(!IS_NPC(ch)) sprintf(playername,"%-16s",GET_NAME(ch));
  else sprintf(playername,"%-16s",MOB_SHORT(ch));
  bylinepaper[paper_msg_num] = (char *)malloc(strlen(playername) + 1);
  sprintf(bylinepaper[paper_msg_num],"%s",playername);
  headpaper[paper_msg_num] = (char *)malloc(strlen(arg) + 1 );
  if (!headpaper[paper_msg_num]) {
    log_f("Paper: Malloc for board header failed.");
    send_to_char("The board is malfunctioning - sorry.\n\r", ch);
    return;
  }
  sprintf(headpaper[paper_msg_num], "%s", arg);
  msgspaper[paper_msg_num] = NULL;

  send_to_char("Write your message. Terminate with a @.\n\r\n\r", ch);
  act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);

  ch->desc->str = &msgspaper[paper_msg_num];
  ch->desc->max_str = MAX_MESSAGE_LENGTH;

  paper_msg_num++;
}

int paper_remove_msg(CHAR *ch, char *arg) {
  int ind, msg;
  char buf[256], number[MAX_INPUT_LENGTH];

  one_argument(arg, number);
  if (!*number || !isdigit(*number))
    return(0);


  if (!(msg = atoi(number))) return(0);
   if (!paper_msg_num) {
     send_to_char("The board is empty!\n\r", ch);
     return(1);
   }
  if (msg < 1 || msg > paper_msg_num) {
    send_to_char("That message exists only in your imagination..\n\r",ch);
    return(1);
  }

  if (GET_LEVEL(ch) < LEVEL_ETE && strncmp(GET_NAME(ch), &headpaper[msg-1][1],strlen(GET_NAME(ch)))) {
    send_to_char("Due to misuse of the REMOVE command, only ETE+\n\ror message originator can remove messages.\n\r", ch);
    return(TRUE);
  }

  ind = msg;
  free(headpaper[--ind]);
  if (msgspaper[ind])
    free(msgspaper[ind]);
  for (; ind < paper_msg_num -1; ind++) {
   headpaper[ind] = headpaper[ind + 1];
   msgspaper[ind] = msgspaper[ind + 1];
   bylinepaper[ind]=bylinepaper[ind+1];
  }
  paper_msg_num--;
  send_to_char("Message removed.\n\r", ch);
  sprintf(buf, "$n just removed message %d.", ind + 1);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  board_save_paper();

  return(1);
}

void board_save_paper() {
  FILE *the_file;

  int ind, len;
  if (!paper_msg_num) {
   log_f("Paper: No messages to save.");
   return;
  }
  the_file = fopen(SAVE_PAPER, "w"); /* was "wb" TC */
  if (!the_file) {
   log_f("Paper: Unable to open/create savefile..");
   return;
  }
  fwrite(&paper_msg_num, sizeof(int), 1, the_file);
  for (ind = 0; ind < paper_msg_num; ind++) {
   len = strlen(headpaper[ind]) + 1;
   fwrite(&len, sizeof(int), 1, the_file);
   fwrite(headpaper[ind], sizeof(char), len, the_file);
   len = strlen(bylinepaper[ind]) + 1;
   fwrite(&len, sizeof(int), 1, the_file);
   fwrite(bylinepaper[ind], sizeof(char), len, the_file);
   if (msgspaper[ind] == NULL) { /* save an empty message 5/28/91 TC */
    len = 1;
    fwrite(&len, sizeof(int), 1, the_file);
    fwrite("", sizeof(char), len, the_file);
   }
   else {
    len = strlen(msgspaper[ind]) + 1;
    fwrite(&len, sizeof(int), 1, the_file);
    fwrite(msgspaper[ind], sizeof(char), len, the_file);
   }
  }
  fclose(the_file);
  return;
}

void board_load_paper() {
  FILE *the_file;
  int ind, len = 0;
  static int has_loaded = 0;

  if (has_loaded)
     {
     return;
     }

  has_loaded = 1;
  paper_reset_board();
  the_file = fopen(SAVE_PAPER, "r"); /* was "rb" TC */
  if (!the_file) {
   log_f("Paper: Can't open message file. Board will be empty.");
   return;
  }
  fread(&paper_msg_num, sizeof(int), 1, the_file);

  if (paper_msg_num < 1 || paper_msg_num > MAX_MSGS || feof(the_file)) {
   log_f("Paper: Board-message file corrupt or nonexistent.");
   fclose(the_file);
   return;
  }
  for (ind = 0; ind < paper_msg_num; ind++)
   {
   fread(&len, sizeof(int), 1, the_file);
   headpaper[ind] = (char *)malloc(len + 1);
   if (!headpaper[ind])
     {
     log_f("Paper: Malloc for board header failed.");
     paper_reset_board();
     fclose(the_file);
     return;
     }
   fread(headpaper[ind], sizeof(char), len, the_file);
   fread(&len, sizeof(int), 1, the_file);
   bylinepaper[ind] = (char *)malloc(len + 1);
   if (!bylinepaper[ind])
     {
     log_f("Paper: Malloc for board byline failed.");
     paper_reset_board();
     fclose(the_file);
     return;
     }
   fread(bylinepaper[ind], sizeof(char), len, the_file);
   fread(&len, sizeof(int), 1, the_file);
   msgspaper[ind] = (char *)malloc(len + 1);
   if (!msgspaper[ind]) {
    log_f("Paper: Malloc for board msg failed..");
    paper_reset_board();
    fclose(the_file);
    return;
   }
   fread(msgspaper[ind], sizeof(char), len, the_file);
  }
  fclose(the_file);
  return;
}

void paper_reset_board() {
  int ind;

  for (ind = 0; ind < MAX_MSGS; ind++) {
   if(headpaper[ind]) free(headpaper[ind]);
   if(msgspaper[ind]) free(msgspaper[ind]);
   if(bylinepaper[ind]) free(bylinepaper[ind]);
  }
  paper_msg_num = 0;
  return;
}

int paper_display_msg(CHAR *ch, char *arg) {
  char number[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
  int msg,tmp_msg = 0;
  bool next=FALSE;

  one_argument(arg, number);
  if (!*number) return FALSE;

  if(is_abbrev(number,"next")) {
    next=TRUE;
    tmp_msg=ch->specials.message;
    ch->specials.message++;
    msg=ch->specials.message;
  }

  if(is_abbrev(number,"same")) {
    next=TRUE;
    msg=ch->specials.message;
  }

  if(is_abbrev(number,"previous")) {
    next=TRUE;
    tmp_msg=ch->specials.message;
    ch->specials.message--;
    msg=ch->specials.message;
  }

  if(!next) {
    if(!isdigit(*number)) return(FALSE);
    if (!(msg = atoi(number))) return(FALSE);
    ch->specials.message=msg;
  }

  if (!paper_msg_num) {
    send_to_char("The board is empty!\n\r", ch);
    return(1);
   }
  if (msg < 1 || msg > paper_msg_num) {
    send_to_char("That message exists only in your imagination..\n\r",ch);
    if(next) ch->specials.message=tmp_msg;
    return(1);
  }

  sprintf(buffer, "Message %d %s: %s\n\r\n\r%s", msg, bylinepaper[msg-1],headpaper[msg - 1],
	  msgspaper[msg - 1]);
  if (msgspaper[msg-1] != NULL) /* 5/28/91 TC don't print empty messages */
   page_string(ch->desc, buffer, 1);
  return(1);
}

int paper_show_board(CHAR *ch, char *arg)
{
  int i;
  char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

  one_argument(arg, tmp);

  if (!*tmp || !isname(tmp, "board bulletin"))
   return(0);

  act("$n studies the board.", TRUE, ch, 0, 0, TO_ROOM);

  strcpy(buf,"This is a bulletin board. Usage: READ/REMOVE <messg #>, WRITE <header>.\n\rYou will need to look at the board to save your message.\n\r");
  if (!paper_msg_num)
   strcat(buf, "The board is empty.\n\r");
  else {
   sprintf(buf + strlen(buf), "There are %d messages on the board.\n\r",paper_msg_num);
   for (i = 0; i < paper_msg_num; i++)
    sprintf(buf + strlen(buf), "%-2d %s: %s\n\r", i + 1,bylinepaper[i],headpaper[i]);
  }
  page_string(ch->desc, buf, 1);
  return(TRUE);
}

void assign_newspaper(void) {
  assign_mob(3,newspaperboy);
  assign_obj(3095,paperboard);
}

