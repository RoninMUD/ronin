/*
 board.c

 Major revision.  Boards are now object TYPE_BOARD
 and are created with OLC.  Object Values are as
 follows
   0 - min board read level
   1 - min board write level
   2 - min board remove level

 Any special requirements such as blocking non-clan
 access or blocking inactive access are built into
 this procedure.

 Ranger - April 98
 Do not distribute without permission.
*/

/*
$Author: ronin $
$Date: 2004/08/13 12:25:04 $
$Header: /home/ronin/cvs/ronin/board.c,v 2.2 2004/08/13 12:25:04 ronin Exp $
$Id: board.c,v 2.2 2004/08/13 12:25:04 ronin Exp $
$Name:  $
$Log: board.c,v $
Revision 2.2  2004/08/13 12:25:04  ronin
Addition of ISA clan board.

Revision 2.1  2004/04/27 12:39:43  ronin
Addition of last message number to board last command.

Revision 2.0.0.1  2004/02/05 16:08:54  ronin
Reinitialization of cvs archives


Revision 12-Feb-03 Addition of board command - Ranger

Revision 1.3  2003/01/26 07:42:14  ronin
Revision - Adding Eternal Newbies Board
Revision - Adding Tabby's . Board

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/


#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "cmd.h"
#include "handler.h"
#include "modify.h"
#include "interpreter.h"
#include "db.h"
#include "spec.clan.h"

#define MAX_MSGS 99
#define MAX_MESSAGE_LENGTH 2048

#define ALTAR_BOARD       3099
#define IMM_BOARD         3098
#define GOD_BOARD         3097
#define ADVENTURER_BOARD  3096
#define MORT_QUEST_BOARD  3094
#define TOAD_BOARD        3087
#define IDEA_BOARD        3064
#define IMM_LOG_BOARD     1210
#define DEI_LOG_BOARD     1211
#define WIZ_LOG_BOARD     1212
#define QUEST_BOARD       1213
#define FEEDBACK_BOARD    1214
#define TODO_BOARD        1215
#define KABOODLE_BOARD    6273
#define DEPUTY_BOARD      3092
#define DIABOLIK_BOARD    27504
#define SOLO_BOARD        27511
#define VERTIGO_BOARD     27518
#define DARK_LEGION_BOARD 27519
#define MM_BOARD          27554
#define ETERNAL_BOARD     27555
#define TABBY_BOARD       27593
#define ISA_BOARD         27841

char *Months[12]= {"Jan","Feb","Mar","Apr","May","Jun",
                   "Jul","Aug","Sep","Oct","Nov","Dec"};

struct struct_board {
  int msg_num;
  int vnumber;
  int min_read;
  int min_write;
  int min_remove;
  char *msgs[MAX_MSGS];
  char *heading[MAX_MSGS];
  char *name;
  char *short_desc;
  struct struct_board *next;
};

struct struct_board *board_list=0;

int check_access(CHAR *ch,struct struct_board *board);
void write_msg(CHAR *ch, char *arg, struct struct_board *board);
int display_msg(CHAR *ch, char *arg, struct struct_board *board);
int remove_msg(CHAR *ch, char *arg, struct struct_board *board);
void save_board(struct struct_board *board);
struct struct_board *load_board(OBJ *obj);
void reset_board(struct struct_board *board);
int show_board(CHAR *ch, char *arg,struct struct_board *board);
char *save_file(struct struct_board *board);

int board(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  struct struct_board *board=0;
  int found=FALSE;

  if(!ch) return(FALSE);
  if(!ch->desc) return(FALSE);

  for(board=board_list;board;board=board->next) {
    if(board->vnumber==V_OBJ(obj)) {
      found=TRUE;
      break;
    }
  }

  if (!found) {
    if(!(board=load_board(obj))) {
      send_to_char("Board initialized.\n\r",ch);
      return(TRUE);
    }
  }

  switch (cmd) {
    case CMD_LOOK:
      if(GET_LEVEL(ch) >= board->min_read) {
        save_board(board);
        return(show_board(ch, arg, board));
      }
      else {
        send_to_char("You are not holy enough to see anything.\n\r", ch);
        return(TRUE);
      }
      break;
    case CMD_WRITE:
      if(GET_LEVEL(ch) >= board->min_write) {
        if (!strncmp(arg, " postcard pen", 13)) return FALSE;
        if (!strncmp(arg, " postcard quill", 15)) return FALSE;
        if (!strncmp(arg, " questcard pen", 14)) return FALSE;
        if (!strncmp(arg, " questcard quill", 16)) return FALSE;
        write_msg(ch, arg, board);
      }
      else {
        send_to_char("You are not holy enough to write here.\n\r", ch);
        return(TRUE);
      }
      return TRUE;
      break;
    case CMD_READ:
      if(GET_LEVEL(ch) >= board->min_read)
        return(display_msg(ch, arg, board));
      else {
        send_to_char("You are not holy enough to read here.\n\r", ch);
        return(TRUE);
      }
      break;
    case CMD_REMOVE:
      return(remove_msg(ch, arg, board));
      break;
    default:
      return FALSE;
      break;
  }
  return FALSE;
}

void write_msg(CHAR *ch, char *arg, struct struct_board *board) {
  long ct;
  struct tm *timeStruct;
  char tmstr[20];
  char playername[35];
  char tempstring[75];

  if (board->msg_num > MAX_MSGS - 1) {
    send_to_char("The board is full already.\n\r", ch);
    return;
  }

  /* skip blanks */
  for(; isspace(*arg); arg++) ;

  if (!*arg) {
    send_to_char("You must specify a message heading.\n\r", ch);
    return;
  }

  if(!check_access(ch,board)) return;

  if(!IS_NPC(ch)) sprintf(playername,"%-16s",GET_NAME(ch));
  else sprintf(playername,"%-16s",MOB_SHORT(ch));
  ct=time(0);
  timeStruct = localtime(&ct);
  sprintf(tmstr, "%s %2d ", Months[timeStruct->tm_mon], timeStruct->tm_mday);
  board->heading[board->msg_num] = (char *)malloc(strlen(tmstr) + strlen(arg) + 16 + 7);

  if (!board->heading[board->msg_num]) {
    log_f("Board: Malloc for board header failed.");
    send_to_char("The board is malfunctioning - sorry.\n\r", ch);
    return;
  }

  if (strlen(tmstr)+strlen(arg)+16+6+5 > 80) {
    send_to_char("Try a shorter heading.\n\r", ch);
    return;
  }

  if(board->vnumber==FEEDBACK_BOARD)
    sprintf(tempstring, "(                ) %s : %s",tmstr, arg);
  else
    sprintf(tempstring, "(%s) %s : %s", playername, tmstr, arg);
  strcat(tempstring,"\0");
  strcpy(board->heading[board->msg_num], tempstring);

  board->msgs[board->msg_num] = NULL;

  send_to_char("Write your message. Terminate with a @.\n\r\n\r", ch);
  act("$n starts to write on the board.", TRUE, ch, 0, 0, TO_ROOM);

  SET_BIT(ch->specials.pflag,PLR_WRITING);
  ch->desc->str = &board->msgs[board->msg_num];
  ch->desc->max_str = MAX_MESSAGE_LENGTH;

  board->msg_num++;
  save_board(board);
}

int remove_msg(CHAR *ch, char *arg, struct struct_board *board) {
  int ind, msg;
  char buf[256], number[MAX_INPUT_LENGTH];

  one_argument(arg, number);
  if (!*number || !isdigit(*number))
    return(FALSE);

  if (!(msg = atoi(number))) return(FALSE);

  if(!check_access(ch,board)) return TRUE;

  if (!board->msg_num) {
    send_to_char("The board is empty!\n\r", ch);
    return(TRUE);
  }
  if (msg < 1 || msg > board->msg_num) {
    send_to_char("That message exists only in your imagination....\n\r",ch);
    return(TRUE);
  }

  if(GET_LEVEL(ch) < board->min_remove &&
    strncmp(GET_NAME(ch), &board->heading[msg-1][1], strlen(GET_NAME(ch)))) {
    send_to_char("Due to misuse of the REMOVE command, only a high god\n\r", ch);
    send_to_char("or message originator can remove messages.\n\r", ch);
    return(TRUE);
  }

  ind = msg;
  free(board->heading[--ind]);
  if (board->msgs[ind])
    free(board->msgs[ind]);
  for (; ind < board->msg_num-1; ind++) {
    board->heading[ind] = board->heading[ind + 1];
    board->msgs[ind] = board->msgs[ind + 1];
  }
  board->msg_num--;
  send_to_char("Message removed.\n\r", ch);
  sprintf(buf, "$n just removed message %d.", msg);
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  save_board(board);

  return(TRUE);
}

void save_board(struct struct_board *board) {
  FILE *the_file;

  int ind, len;
  if (!board->msg_num) {
    log_f("Board: No messages to save.");
    return;
  }

  the_file = fopen(save_file(board), "w");

  if (!the_file) {
    log_f("Board: Unable to open/create savefile..");
    return;
  }

  fwrite(&board->msg_num, sizeof(int), 1, the_file);
  for (ind = 0; ind < board->msg_num; ind++) {
    len = strlen(board->heading[ind]) + 1;
    fwrite(&len, sizeof(int), 1, the_file);
    fwrite(board->heading[ind], sizeof(char), len, the_file);
    if (board->msgs[ind] == NULL) {
      len = 1;
      fwrite(&len, sizeof(int), 1, the_file);
      fwrite("", sizeof(char), len, the_file);
    }
    else {
      len = strlen(board->msgs[ind]) + 1;
      fwrite(&len, sizeof(int), 1, the_file);
      fwrite(board->msgs[ind], sizeof(char), len, the_file);
    }
  }
  fclose(the_file);

  return;
}

struct struct_board *load_board(OBJ *obj) {
  FILE *the_file;
  int ind, len = 0;
  struct struct_board *board,*cboard;

  for(cboard=board_list;cboard;cboard=cboard->next) {
    if(cboard->vnumber==V_OBJ(obj)) {
      return(NULL);
    }
  }

  CREATE(board, struct struct_board, 1);
  memset((char *)board, '\0', sizeof(struct struct_board));
  reset_board(board);
  board->min_read=obj->obj_flags.value[0];
  board->min_write=obj->obj_flags.value[1];
  board->min_remove=obj->obj_flags.value[2];
  board->vnumber=V_OBJ(obj);
  board->name=str_dup(OBJ_NAME(obj));
  board->short_desc=str_dup(OBJ_SHORT(obj));
  board->next = board_list;
  board_list = board;

  if (!(the_file = fopen(save_file(board), "r"))) {
    log_f("Board: Can't open message file. Board will be empty.");
    return(NULL);
  }

  fread(&board->msg_num, sizeof(int), 1, the_file);

  if (board->msg_num < 1 || board->msg_num > MAX_MSGS || feof(the_file)) {
    log_f("Board: Board-message file corrupt or nonexistent.");
    fclose(the_file);
    return(NULL);
  }

  for (ind = 0; ind < board->msg_num; ind++) {
    fread(&len, sizeof(int), 1, the_file);
    board->heading[ind] = (char *)malloc(len + 1);
    if (!board->heading[ind]) {
      log_f("Board: Malloc for board header failed.");
      reset_board(board);
      fclose(the_file);
      return(NULL);
    }
    fread(board->heading[ind], sizeof(char), len, the_file);
    fread(&len, sizeof(int), 1, the_file);
    board->msgs[ind] = (char *)malloc(len + 1);
    if (!board->msgs[ind]) {
      log_f("Board: Malloc for board msg failed..");
      reset_board(board);
      fclose(the_file);
      return(NULL);
    }
    fread(board->msgs[ind], sizeof(char), len, the_file);
  }
  fclose(the_file);
  return(board);
}

void reset_board(struct struct_board *board) {
  int ind;

  for (ind = 0; ind < MAX_MSGS; ind++) {
    if(board->heading[ind]) free(board->heading[ind]);
    if(board->msgs[ind]) free(board->msgs[ind]);
  }
  board->msg_num = 0;
  return;
}

int display_msg(CHAR *ch, char *arg, struct struct_board *board)
{
  char number[MIL], buffer[MSL];
  int msg = 0, tmp_msg = 0;
  bool next = FALSE;

  if (!check_access(ch, board)) return TRUE;

  if (!board->msg_num)
  {
    send_to_char("The board is empty!\n\r", ch);

    return TRUE;
  }

  one_argument(arg, number);

  if (!*number) return FALSE;

  if (is_abbrev(number, "next"))
  {
    next = TRUE;
    tmp_msg = ch->specials.message;
    ch->specials.message++;
    msg = ch->specials.message;
  }
  else if(is_abbrev(number, "same"))
  {
    next = TRUE;
    msg = ch->specials.message;
  }
  else if(is_abbrev(number, "previous"))
  {
    next = TRUE;
    tmp_msg = ch->specials.message;
    ch->specials.message--;
    msg = ch->specials.message;
  }
  else if (is_abbrev(number, "last"))
  {
    next = TRUE;
    msg = board->msg_num;
    ch->specials.message = msg;
  }
  else
  {
    if (!is_number(number)) return FALSE;

    if (!(msg = atoi(number))) return FALSE;

    ch->specials.message = msg;
  }

  if (msg < 1 || msg > board->msg_num)
  {
    send_to_char("That message exists only in your imagination...\n\r", ch);

    if (next) ch->specials.message = tmp_msg;

    return TRUE;
  }

  sprintf(buffer, "Message %d : %s\n\r\n\r%s", msg, board->heading[msg - 1], board->msgs[msg - 1]);

  if (board->msgs[msg - 1] != NULL)
  {
    page_string(ch->desc, buffer, 1);
  }

  return TRUE;
}

int show_board(CHAR *ch, char *arg, struct struct_board *board) {
  int i;
  char buf[MAX_STRING_LENGTH*2], tmp[MAX_INPUT_LENGTH];

  one_argument(arg, tmp);
  if (!*tmp || !isname(tmp, board->name)) return(FALSE);
  if(!check_access(ch,board)) return TRUE;

  sprintf(buf,"$n studies %s.",board->short_desc);
  act(buf,TRUE,ch,0,0,TO_ROOM);

  strcpy(buf, "This is a bulletin board. Usage: READ <#>/<next>/<same>/<previous> REMOVE <#>, WRITE <header>.\n\rYou will need to look at the board to save your message.\n\r");

  if(board->msg_num>MAX_MSGS-1)
    board->msg_num=MAX_MSGS-1;

  if (!board->msg_num)
    strcat(buf, "The board is empty.\n\r");
  else {
    sprintf(buf + strlen(buf), "There are %d messages on the board.\n\r",
     board->msg_num);
    for (i = 0; i < board->msg_num; i++)
      sprintf(buf + strlen(buf), "%-2d : %s\n\r", i + 1, board->heading[i]);
  }
  page_string(ch->desc, buf, 1);

  return(TRUE);
}

int check_access(CHAR *ch,struct struct_board *board) {
  int vnum;

  vnum=board->vnumber;

  if(vnum==DIABOLIK_BOARD || vnum==SOLO_BOARD || vnum==VERTIGO_BOARD ||
     vnum==DARK_LEGION_BOARD || vnum==MM_BOARD || vnum==ETERNAL_BOARD || vnum==TABBY_BOARD ||
     vnum==ISA_BOARD) {
    if(!check_clan_board(vnum,ch)) {
      send_to_char("You don't have access to this clan board.\n\r",ch);
      return FALSE;
    }
  }

  if(vnum==KABOODLE_BOARD) {
    if(!IS_SET(ch->new.imm_flags, WIZ_ACTIVE)) {
      send_to_char("You don't have access to this board.\n\r",ch);
      return FALSE;
    }
  }
  if(vnum==DEPUTY_BOARD) {
    if(!IS_SET(ch->specials.pflag,PLR_DEPUTY) && GET_LEVEL(ch)<LEVEL_WIZ) {
      send_to_char("You don't have access to this board.\n\r",ch);
      return FALSE;
    }
  }
  return TRUE;
}

char *save_file(struct struct_board *board) {
  char buf[MAX_INPUT_LENGTH];

  switch (board->vnumber) {
    case ALTAR_BOARD :
      return "board.messages";break;
    case IMM_BOARD:
      return "imm.messages";break;
    case GOD_BOARD:
      return "god.messages";break;
    case ADVENTURER_BOARD:
      return "adv.messages";break;
    case MORT_QUEST_BOARD:
      return "mquest.messages";break;
    case TOAD_BOARD:
      return "toad.messages";break;
    case IDEA_BOARD:
      return "idea.messages";break;
    case IMM_LOG_BOARD:
      return "imm-log.messages";break;
    case DEI_LOG_BOARD:
      return "dei-log.messages";break;
    case WIZ_LOG_BOARD:
      return "wiz-log.messages";break;
    case QUEST_BOARD:
      return "quest.messages";break;
    case DEPUTY_BOARD:
      return "deputy.messages";break;
    case FEEDBACK_BOARD:
      return "feedbak.messages";break;
    case TODO_BOARD:
      return "todo.messages";break;
    case KABOODLE_BOARD:
      return "bugs.old.Z";break;
    case DIABOLIK_BOARD:
      return "diabolik.messages";break;
    case SOLO_BOARD:
      return "solo.messages";break;
    case VERTIGO_BOARD:
      return "vertigo.messages";break;
    case DARK_LEGION_BOARD:
      return "darklegion.messages";break;
    case MM_BOARD:
      return "midnightm.messages";break;
    case ETERNAL_BOARD:
      return "eternal.messages";break;
    case TABBY_BOARD:
      return "tabby.messages";break;
    case ISA_BOARD:
      return "isa.messages";break;
    default:
      sprintf(buf,"%d.messages",board->vnumber);
      return str_dup(buf);
      break;
  }
  return NULL;
}

void write_board(int vnum,char *heading,char *message) {
  long ct;
  struct tm *timeStruct;
  char tmstr[20];
  char tempstring[75];
  struct struct_board *board=0;
  int found=0;\
  OBJ *obj;

  /* find board */
  for(board=board_list;board;board=board->next) {
    if(board->vnumber==vnum) {
      found=TRUE;
      break;
    }
  }

  if(!found) {
    if((obj=get_obj_world(vnum))) {
      if((board=load_board(obj)))
       found=TRUE;
    }
  }

  if(!found) {
    log_f("write_board: board %d not found",vnum);
    return;
  }

  if (board->msg_num > MAX_MSGS - 1) {
    log_f("write_board: board %d is full",vnum);
    return;
  }

  if (!*heading || !*message) {
    log_f("write board: no message or heading indicated - board %d",vnum);
    return;
  }

  if(strlen(message)>MAX_MESSAGE_LENGTH) {
    log_f("write board: message is too large - board %d",vnum);
    return;
  }

  ct=time(0);
  timeStruct = localtime(&ct);
  sprintf(tmstr, "%s %2d ", Months[timeStruct->tm_mon], timeStruct->tm_mday);
  board->heading[board->msg_num] = (char *)malloc(strlen(tmstr) + strlen(heading) + 16 + 11);

  if (strlen(tmstr)+strlen(heading)+16+11 > 80) {
    log_f("write_board: heading too long - board %d",vnum);
    return;
  }

  if (!board->heading[board->msg_num]) {
    log_f("write_board: Malloc for board %d header failed",vnum);
    return;
  }

  sprintf(tempstring, "(autopost        ) %s : %s", tmstr,heading);
  strcat(tempstring,"\0");
  strcpy(board->heading[board->msg_num], tempstring);

  board->msgs[board->msg_num] = NULL;
  board->msgs[board->msg_num] = (char *)malloc(strlen(message) + 1);
  if(!board->msgs[board->msg_num]) {
    log_f("write_board: malloc for board %d msg failed",vnum);
    return;
  }
  strcpy(board->msgs[board->msg_num],message);
  board->msg_num++;
  save_board(board);
}


/* This command should eventally be expanded to include more
   options for board message manipulation
*/

void do_board(struct char_data *ch, char *argument, int cmd) {
  char usage[]="\
This command is used for board message manipulation.\n\r\n\r\
  Usage: board last - list last message of all non-clan boards\n\r\
         board copy <board1> <mess#> <board2> - copy a message to board2\n\r";
  char arg[MAX_INPUT_LENGTH];
  int board1,board2,messnum,rnum;
  struct struct_board *board=0;
  struct struct_board *dboard=0;
  int found=FALSE,dfound=FALSE;

  if(!ch) return;
  if(!ch->desc) return;

  argument=one_argument(argument,arg);
  if(!*arg) { send_to_char(usage,ch); return; }

  if(is_abbrev(arg, "last")) {
    printf_to_char(ch,"Room  Board Name          M#  Last Message\n\r");
    for(board=board_list;board;board=board->next) {
      if(inzone(board->vnumber)==275) continue;
      if(inzone(board->vnumber)==278) continue;
      if(GET_LEVEL(ch)<LEVEL_SUP &&
        (board->vnumber==3097 || board->vnumber==3087 || board->vnumber==6273)) continue;
      if(GET_LEVEL(ch)<board->min_read) continue;
      rnum=board->vnumber;
      switch(board->vnumber) {
        case 3064: rnum=3087; break;
        case 3096: rnum=3084; break;
        case 3087: rnum=3092; break;
        case 3099: rnum=3054; break;
        case 3094: rnum=3038; break;
        case 1215: rnum=1249; break;
        case 1214: rnum=1203; break;
        case 1213: rnum=1204; break;
        case 1210: rnum=1256; break;
        case 1211: rnum=1257; break;
        case 1212: rnum=1258; break;
        case 3095: rnum=1225; break;
        case 3097: rnum=1220; break;
        case 6273: rnum=1265; break;
        case 3098: rnum=1212; break;
        case 3092: rnum=0;    break;
      }
      if(board->msg_num)
        printf_to_char(ch,"%-5d %-20s: %-2d %s\n\r",
                           rnum, obj_proto_table[real_object(board->vnumber)].short_description,
                           board->msg_num,board->heading[board->msg_num-1]);
      else
        printf_to_char(ch,"%-5d %-20s: Empty\n\r",
                           board->vnumber, obj_proto_table[real_object(board->vnumber)].short_description);
    }
    return;
  }

  if(is_abbrev(arg, "copy")) {

    /* check all arguments */
    argument=one_argument(argument,arg);
    if(!*arg) { send_to_char(usage,ch); return; }
    if(is_number(arg)) board1=atoi(arg);
    else { send_to_char(usage,ch); return; }

    argument=one_argument(argument,arg);
    if(!*arg) { send_to_char(usage,ch); return; }
    if(is_number(arg)) {
      messnum=atoi(arg);
      messnum--;
    }
    else { send_to_char(usage,ch); return; }

    argument=one_argument(argument,arg);
    if(!*arg) { send_to_char(usage,ch); return; }
    if(is_number(arg)) board2=atoi(arg);
    else { send_to_char(usage,ch); return; }

    for(board=board_list;board;board=board->next) {
      if(board->vnumber==board1) {
        found=TRUE;
        break;
      }
    }

    /* check for board */
    if(!found) {
      printf_to_char(ch,"Board %d not found.\n\r",board1);
      return;
    }
    /*check for access */
    if(GET_LEVEL(ch) < board->min_read) {
      printf_to_char(ch,"You are not a high enough level to read board %d.\n\r",board1);
    }
    if(!check_access(ch,board)) return;
    /* check for message */
    if(messnum > board->msg_num) {
      printf_to_char(ch,"Message number %d does not exist.\n\r",messnum+1);
      return;
    }

   /* find destination board */
    for(dboard=board_list;dboard;dboard=dboard->next) {
      if(dboard->vnumber==board2) {
        dfound=TRUE;
        break;
      }
    }
    if(!dfound) {
      printf_to_char(ch,"Board %d not found.\n\r",board2);
      return;
    }
    /*check for access */
    if(GET_LEVEL(ch) < dboard->min_write) {
      printf_to_char(ch,"You are not a high enough level to write to board %d.\n\r",board2);
    }
    if(!check_access(ch,dboard)) return;

    if (dboard->msg_num > MAX_MSGS - 1) {
      printf_to_char(ch,"Board %d is full",board2);
      return;
    }
    /* heading */
    dboard->heading[dboard->msg_num] = (char *)malloc(strlen(board->heading[messnum]));
    if(!dboard->heading[dboard->msg_num]) {
      printf_to_char(ch,"Problem writing to destination board %d.\n\r",board2);
      return;
    }
    strcpy(dboard->heading[dboard->msg_num], board->heading[messnum]);
    /* message body */
    dboard->msgs[dboard->msg_num] = NULL;
    dboard->msgs[dboard->msg_num] = (char *)malloc(strlen(board->msgs[messnum])+1);
    if(!dboard->msgs[dboard->msg_num]) {
      printf_to_char(ch,"Problem writing to destination board %d.\n\r",board2);
      return;
    }
    strcpy(dboard->msgs[dboard->msg_num],board->msgs[messnum]);
    dboard->msg_num++;
    save_board(dboard);
    printf_to_char(ch,"Message copied.\n\r");
    return;
  }

  send_to_char(usage,ch);
}
