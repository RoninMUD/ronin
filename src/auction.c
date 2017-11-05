
/* Spec proc for the Auctioneer. (c) Ossi Tulonen 1994.
   otulonen@niksula.hut.fi
   Not for public domain, do not distribute.

   Modifications May 96 by Ranger
     - Added price reduction for items over MAX_TIME and 50,000 coins
     - Added a check_auction (done at reboot) for SOLD items, they
       get put back on if not bought
     - Added a reset time when a char buys an item. This gives previous
       owner MAX_TIME to collect
     - Added an identify command costing 3500 coins.
   Typoed 15/09 by Quack

May 98 - Lowered 50K to 30K
*/

/*
$Author: ronin $
$Date: 2005/01/21 14:55:26 $
$Header: /home/ronin/cvs/ronin/auction.c,v 2.2 2005/01/21 14:55:26 ronin Exp $
$Id: auction.c,v 2.2 2005/01/21 14:55:26 ronin Exp $
$Name:  $
$Log: auction.c,v $
Revision 2.2  2005/01/21 14:55:26  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.1  2004/03/04 17:23:57  ronin
Addition of object file version 2 which includes 8 ownerid fields
for addition of some objects only being able to be used by those
owners.

Revision 2.0.0.1  2004/02/05 16:08:53  ronin
Reinitialization of cvs archives

Revision 1.5  2002/04/18 04:07:30  ronin
Changing log output from perror to log_f for internal syslog manipulation.

Revision - changed collection time for auction from 4 days to 20 days

Revision 1.4  2002/03/31 16:46:42  ronin
Added #include <string.h> to remove implicit declaration warning.

Revision 1.3  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <strings.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"
#include "utils.h"
#include "reception.h"
#include "utility.h"
#include "utils.h"
#include "act.h"
#include "modify.h"
#include "cmd.h"

#define AUCTION_REDUCE 30000
#define MAX_OBJS_AUCTION 300
#define MIN_OBJ_VALUE 0
#define MAX_TIME 4*SECS_PER_REAL_DAY
#define AUCTION_FREE    0
#define AUCTION_BIDS    1
#define AUCTION_BIDS2   2
#define AUCTION_BIDS3   3
#define AUCTION_1_TIME  4
#define AUCTION_1_TIME2 5
#define AUCTION_1_TIME3 6
#define AUCTION_2_TIME  7
#define AUCTION_2_TIME2 8
#define AUCTION_2_TIME3 9
#define AUCTION_SOLD    10
#define AUCTION_COLLECT 11
#define CMD_collect 299
#define CMD_auction 240
#define CMD_buy     56
#define CMD_look    15
#define AUCTION_FILE "auction.data"

extern CHAR *character_list;
extern struct descriptor_data *descriptor_list;
extern int CHAOSMODE;
void auction_recover(CHAR *auctioneer, CHAR *ch, char *arg); /* Add Jan 2000 - Ranger */
char *str_dup(char *source);

struct auction_elem
{
  char *owner;
  char *last_bidder;
  struct obj_data *obj;
  int value;
  int status;
  time_t time;
};

struct auction_board
{
  struct auction_elem auc[MAX_OBJS_AUCTION];
  int obj_num;
} A_Board;

struct auction_file_elem
{
  char owner[20];
  char last_bidder[20];
  struct obj_file_elem_ver3 obj_elem;
  int value;
  int status;
  time_t time;
};

void intialize_auction_board(void)
{
  char buf[256];
  int i, j, k;
  struct auction_file_elem elem;
  struct obj_data *obj;
  FILE *fl;

  for(i = 0; i < MAX_OBJS_AUCTION; i++) {
    A_Board.auc[i].owner  = 0;
    A_Board.auc[i].last_bidder = 0;
    A_Board.auc[i].obj   = 0;
    A_Board.auc[i].value  = 0;
    A_Board.auc[i].status = AUCTION_FREE;
    A_Board.auc[i].time       = 0;
  }
  A_Board.obj_num = 0;
  k = 0;

  if(!(fl = fopen(AUCTION_FILE, "rb+"))) {
    log_f("  No auction file.");
    return;
  }

  while (!feof(fl)) {
    fread(&elem, sizeof(elem), 1, fl);
    if (ferror(fl)) {
      log_f("reading auction file -intialize auction board");
      fclose(fl);
      return;
    }
    if (!feof(fl)) {
      if(elem.obj_elem.item_number == -1)
	obj = 0;
      else
	if((obj = read_object(elem.obj_elem.item_number, VIRTUAL))) {

	  obj->obj_flags.timer = elem.obj_elem.timer;

	  obj->obj_flags.value[0] = elem.obj_elem.value[0];
	  obj->obj_flags.value[1] = elem.obj_elem.value[1];
	  obj->obj_flags.value[2] = elem.obj_elem.value[2];
	  obj->obj_flags.value[3] = elem.obj_elem.value[3];

	  obj->obj_flags.extra_flags = elem.obj_elem.extra_flags;
	  obj->obj_flags.weight      = elem.obj_elem.weight;
	  obj->obj_flags.bitvector   = elem.obj_elem.bitvector;

	  for(j = 0; j < OFILE_MAX_OBJ_AFFECT; j++) {
      obj->affected[j].location = (int)elem.obj_elem.affected[j].location;
      obj->affected[j].modifier = (int)elem.obj_elem.affected[j].modifier;
    }
          /* new obj saves for obj ver3 */
          obj->obj_flags.bitvector2   = elem.obj_elem.bitvector2;
          obj->obj_flags.popped       = elem.obj_elem.popped;
          /* end new ver3 obj saves */

	  A_Board.obj_num++;
	} else {
	  log_f("Unknown object in auction.");
	  continue;
	}

      A_Board.auc[k].obj    = obj;
      A_Board.auc[k].value  = elem.value;
      if(*elem.owner)
	A_Board.auc[k].owner  = str_dup(elem.owner);
      if(*elem.last_bidder)
	A_Board.auc[k].last_bidder = str_dup(elem.last_bidder);
      A_Board.auc[k].status = elem.status;
      A_Board.auc[k].time   = elem.time;
      k++;
    }
  }
  fclose(fl);
  sprintf(buf, "  Auction board size: %d and %d objects", k, A_Board.obj_num);
  log_s(buf);
}

void save_auction(void)
{
  int i, j;
  struct auction_file_elem elem;
  FILE *fl;

  if(!(fl = fopen(AUCTION_FILE, "wb+"))) {
    log_f(AUCTION_FILE);
    return;
  }

  for(i = 0; i < MAX_OBJS_AUCTION; i++) {
    if(A_Board.auc[i].status == AUCTION_FREE )
	continue;
    if(!A_Board.auc[i].obj&&(A_Board.auc[i].status!=AUCTION_COLLECT))
      continue;

    if(A_Board.auc[i].owner)
      strcpy(elem.owner, A_Board.auc[i].owner);
    else
      *elem.owner = '\0';
    if(A_Board.auc[i].last_bidder)
      strcpy(elem.last_bidder, A_Board.auc[i].last_bidder);
    else
      *elem.last_bidder = '\0';
    if(A_Board.auc[i].obj) {
      elem.obj_elem.item_number = obj_proto_table[A_Board.auc[i].obj->item_number].virtual;
      elem.obj_elem.timer       = A_Board.auc[i].obj->obj_flags.timer;
      elem.obj_elem.value[0]    = A_Board.auc[i].obj->obj_flags.value[0];
      elem.obj_elem.value[1]    = A_Board.auc[i].obj->obj_flags.value[1];
      elem.obj_elem.value[2]    = A_Board.auc[i].obj->obj_flags.value[2];
      elem.obj_elem.value[3]    = A_Board.auc[i].obj->obj_flags.value[3];
      elem.obj_elem.extra_flags = A_Board.auc[i].obj->obj_flags.extra_flags;
      elem.obj_elem.weight      = A_Board.auc[i].obj->obj_flags.weight;
      elem.obj_elem.bitvector   = A_Board.auc[i].obj->obj_flags.bitvector;
/* new obj saves */
      elem.obj_elem.version=32003;
      elem.obj_elem.type_flag   = A_Board.auc[i].obj->obj_flags.type_flag;
      elem.obj_elem.wear_flags  = A_Board.auc[i].obj->obj_flags.wear_flags;
      elem.obj_elem.extra_flags2= A_Board.auc[i].obj->obj_flags.extra_flags2;
      elem.obj_elem.subclass_res= A_Board.auc[i].obj->obj_flags.subclass_res;
      elem.obj_elem.material    = A_Board.auc[i].obj->obj_flags.material;
      elem.obj_elem.spec_value  = A_Board.auc[i].obj->spec_value;
      for(j = 0; j < MAX_OBJ_AFFECT; j++)
	     elem.obj_elem.affected[j] = A_Board.auc[i].obj->affected[j];
/* end new obj saves */

/* new obj saves for obj ver3 */
      elem.obj_elem.bitvector2  = A_Board.auc[i].obj->obj_flags.bitvector2;
      elem.obj_elem.popped      = A_Board.auc[i].obj->obj_flags.popped;
/* end new ver3 obj saves */

/* New ownerid field */
      elem.obj_elem.ownerid[0]  = A_Board.auc[i].obj->ownerid[0];
      elem.obj_elem.ownerid[1]  = A_Board.auc[i].obj->ownerid[1];
      elem.obj_elem.ownerid[2]  = A_Board.auc[i].obj->ownerid[2];
      elem.obj_elem.ownerid[3]  = A_Board.auc[i].obj->ownerid[3];
      elem.obj_elem.ownerid[4]  = A_Board.auc[i].obj->ownerid[4];
      elem.obj_elem.ownerid[5]  = A_Board.auc[i].obj->ownerid[5];
      elem.obj_elem.ownerid[6]  = A_Board.auc[i].obj->ownerid[6];
      elem.obj_elem.ownerid[7]  = A_Board.auc[i].obj->ownerid[7];

    } else {
      elem.obj_elem.item_number = -1;
    }

    elem.value  = A_Board.auc[i].value;
    elem.status = A_Board.auc[i].status;
    elem.time   = A_Board.auc[i].time;
    if (fwrite(&elem, sizeof(elem), 1, fl) < 1) {
	log_f("ERROR! writing auction data");

      break;
    }

  }
  fclose(fl);
}

void update_auction(CHAR *auctioneer)
{
  char buf[MAX_STRING_LENGTH];
  int i, needsave;
  time_t timenow;

  timenow = time(0);
  needsave = FALSE;
  for(i = 0; i < MAX_OBJS_AUCTION; i++) {
    switch(A_Board.auc[i].status) {
    case AUCTION_BIDS: {
      if(A_Board.auc[i].last_bidder) {
        A_Board.auc[i].status = AUCTION_BIDS2;
      }
    } break;
    case AUCTION_BIDS2: {
        A_Board.auc[i].status = AUCTION_BIDS3;
    } break;
    case AUCTION_BIDS3: {
      A_Board.auc[i].status = AUCTION_1_TIME;
      sprintf(buf, "%s Your lot #%d (%s) %d coins first time.",
              A_Board.auc[i].owner, i,
              OBJ_SHORT(A_Board.auc[i].obj),
              A_Board.auc[i].value);
      do_tell(auctioneer, buf, 19);
      sprintf(buf, "%s lot #%d (%s) %d coins to you first time.",
              A_Board.auc[i].last_bidder, i,
              OBJ_SHORT(A_Board.auc[i].obj),
              A_Board.auc[i].value);
      do_tell(auctioneer, buf, 19);
    } break;
    case AUCTION_1_TIME: {
      A_Board.auc[i].status = AUCTION_1_TIME2;
    } break;
    case AUCTION_1_TIME2: {
      A_Board.auc[i].status = AUCTION_1_TIME3;
    } break;
    case AUCTION_1_TIME3: {
      A_Board.auc[i].status = AUCTION_2_TIME;
      sprintf(buf, "%s Your lot #%d (%s) %d coins second time.",
              A_Board.auc[i].owner, i,
              OBJ_SHORT(A_Board.auc[i].obj),
              A_Board.auc[i].value);
      do_tell(auctioneer, buf, 19);
      sprintf(buf, "%s lot #%d (%s) %d coins to you second time.",
              A_Board.auc[i].last_bidder, i,
              OBJ_SHORT(A_Board.auc[i].obj),
              A_Board.auc[i].value);
      do_tell(auctioneer, buf, 19);
    } break;
    case AUCTION_2_TIME: {
      A_Board.auc[i].status = AUCTION_2_TIME2;
    } break;
    case AUCTION_2_TIME2: {
      A_Board.auc[i].status = AUCTION_2_TIME3;
    } break;
    case AUCTION_2_TIME3: {
      A_Board.auc[i].status = AUCTION_SOLD;
      sprintf(buf, "%s Your lot #%d (%s) %d coins sold to %s.",
              A_Board.auc[i].owner, i,
              OBJ_SHORT(A_Board.auc[i].obj),
              A_Board.auc[i].value, A_Board.auc[i].last_bidder);
      do_tell(auctioneer, buf, 19);
      sprintf(buf, "%s lot #%d (%s) sold to you for %d coins.",
              A_Board.auc[i].last_bidder, i,
              OBJ_SHORT(A_Board.auc[i].obj),
              A_Board.auc[i].value);
      do_tell(auctioneer, buf, 19);
    } break;
    } /* switch */

/* The next if statement is modified to lower object costs if they
   have been there more than MAX_TIME/2. SOLD case moved to check_auction
   which is called at every reboot - Ranger May 96 - All other cases
   go the full MAX_TIME in the net if.
   changed to max_time/4 - ranger march 2001 */

    if(A_Board.auc[i].time && timenow >= A_Board.auc[i].time + MAX_TIME/4 ) {
      switch(A_Board.auc[i].status) {
        case AUCTION_FREE: {
          log_f("BUG in update auction. FREE item showing a time.");
          A_Board.auc[i].time = 0;
          continue;
        } break;

        case AUCTION_SOLD: {
          continue;
        } break;

        case AUCTION_COLLECT: {
        } break;

        default: {
          if (A_Board.auc[i].value > AUCTION_REDUCE) {
            sprintf(buf, "Auction Check: %s (MIN BID: %d coins) (%s) - Item reduced",
                OBJ_SHORT(A_Board.auc[i].obj), A_Board.auc[i].value, A_Board.auc[i].owner);
            log_s(buf);
            A_Board.auc[i].value=A_Board.auc[i].value*7/10;
    /* added by Sane 4-19  */
            if (A_Board.auc[i].value<1)(A_Board.auc[i].value=10000);
    /*                     */
            A_Board.auc[i].time=time(0);
            needsave=TRUE;
            continue;
          } else {
            sprintf(buf, "Auction Check: %s (MIN BID: %d coins) (%s) - Item removed",
                OBJ_SHORT(A_Board.auc[i].obj), A_Board.auc[i].value, A_Board.auc[i].owner);
            log_s(buf);

            if(A_Board.auc[i].owner) free(A_Board.auc[i].owner);
            A_Board.auc[i].owner = 0;
            if(A_Board.auc[i].last_bidder) free(A_Board.auc[i].last_bidder);
            A_Board.auc[i].last_bidder = 0;

            if(A_Board.auc[i].obj) {
              extract_obj(A_Board.auc[i].obj);
              A_Board.obj_num--;
            }
            A_Board.auc[i].obj = 0;
            A_Board.auc[i].value      = 0;
            A_Board.auc[i].status     = AUCTION_FREE;
            A_Board.auc[i].time       = 0;
            needsave = TRUE;
            continue;
          }
        } break;
      } /* switch */
    } /* if */

    /* 4 days */
    if(A_Board.auc[i].time && (timenow >= (A_Board.auc[i].time + (MAX_TIME))) ) {
      switch(A_Board.auc[i].status) {
        case AUCTION_FREE: {
          log_f("BUG in update auction. FREE item showing a time.");
          A_Board.auc[i].time = 0;
          continue;
        } break;

        case AUCTION_SOLD: {
          continue;
        } break;

        default: {
            continue;
        } break;
      } /* switch */
    } /* if */

    /* 20 days */
    if(A_Board.auc[i].time && (timenow >= (A_Board.auc[i].time + (MAX_TIME*5))) ) {
      switch(A_Board.auc[i].status) {
        case AUCTION_COLLECT: {
          sprintf(buf, "Auction Check: (COLLECT: %d coins) (%s) - MAX_TIME exceeded - coins removed",
                A_Board.auc[i].value, A_Board.auc[i].owner);
          log_s(buf);
          if(A_Board.auc[i].owner) free(A_Board.auc[i].owner);
          A_Board.auc[i].owner = 0;
          if(A_Board.auc[i].last_bidder) free(A_Board.auc[i].last_bidder);
          A_Board.auc[i].last_bidder = 0;

          if(A_Board.auc[i].obj) {
            extract_obj(A_Board.auc[i].obj);
            A_Board.obj_num--;
          }
          A_Board.auc[i].obj = 0;
          A_Board.auc[i].value      = 0;
          A_Board.auc[i].status     = AUCTION_FREE;
          A_Board.auc[i].time       = 0;
          needsave = TRUE;
          continue;
        } break;

        default: {
            continue;
        } break;
      } /* switch */
    } /* if */

  } /* for */

  if(needsave) save_auction();
}

int auction_look(CHAR *auctioneer, CHAR *ch, char *arg)
{
  char buf[MAX_OBJS_AUCTION * 110 + 1124], tmp[MAX_STRING_LENGTH];
  int i;

  char *auction_status[] = {
    " FREE  ",
    " Bids  ",
    " Bids  ",
    " Bids  ",
    "1.Time ",
    "1.Time ",
    "1.Time ",
    "2.Time ",
    "2.Time ",
    "2.Time ",
    " SOLD  ",
    "COLLECT"
    };

  one_argument( arg, buf);

  if (!*buf || !isname(buf, "board auction"))
    return(FALSE);

  act("$n studies the auction board which is held by $N.", TRUE, ch, 0, auctioneer, TO_ROOM);

  strcpy(buf, "This is an auction board. More help about: HELP auction.\n\r");
  if(GET_LEVEL(ch)>=LEVEL_SUP)
    strcat(buf, "SUP+ only.  To remove an item type 'recover <#>'.\n\r");
  if(!A_Board.obj_num)
    strcat(buf, "There are no objects being auctioned at this moment.\n\r");
  else {
    sprintf(tmp, "There are %d object(s) on auction.\n\r", A_Board.obj_num);
    strcat(buf, tmp);
    strcat(buf, "lot   status   last bid:  object:                          last bidder:\n\r");
    strcat(buf, "--------------------------------------------------------------------------------\n\r");
    for( i = 0; i < MAX_OBJS_AUCTION; i++) {
      if(A_Board.auc[i].status != AUCTION_FREE &&
         (A_Board.auc[i].status != AUCTION_COLLECT ||
          str_cmp(A_Board.auc[i].owner, GET_NAME(ch)) == 0 ||
          GET_LEVEL(ch) >= LEVEL_SUP)) { /*IMP_LEVEL)) {*/
        sprintf(tmp, "[%3d] [%s] [%9d] [%28.28s] [%19s]",
                i,
	        auction_status[A_Board.auc[i].status],
		A_Board.auc[i].value,
                (A_Board.auc[i].obj ? OBJ_SHORT(A_Board.auc[i].obj) : \
		 (GET_LEVEL(ch) >= LEVEL_SUP ? A_Board.auc[i].owner : \
		  "no object anymore")),
		(A_Board.auc[i].last_bidder ? A_Board.auc[i].last_bidder : "no one"));
	   strcat(buf, tmp);
	    if(GET_LEVEL(ch)>=LEVEL_SUP) sprintf(tmp," [%19s]\n\r",A_Board.auc[i].owner);
	    else strcpy(tmp,"\n\r");
	    strcat(buf,tmp);
      }
    } /* for */
  }
  strcat(buf,"\n\rTo identify an item, just type identify <lot number>. Cost: 3500 coins.\n\r");
  page_string(ch->desc, buf, 1);
  return(TRUE);
}

void auction_buy(CHAR *auctioneer, CHAR *ch, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int number, found;

  arg = one_argument(arg, buf);
  if(!*buf) {
    act("$n asks you 'What object do you want to buy from auction ?'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if(!strcmp("all", buf)) {
    found = FALSE;
    for(number = 0; number < MAX_OBJS_AUCTION; number++) {
      if(A_Board.auc[number].status == AUCTION_SOLD &&
         !strcmp(GET_NAME(ch), A_Board.auc[number].last_bidder)) {

        if(GET_GOLD(ch) < A_Board.auc[number].value) {
          act("$n tells you 'Sorry but you don't have enough gold on you for $p.'", FALSE, auctioneer, A_Board.auc[number].obj, ch, TO_VICT);
        } else {
          act("$n gives you $p.", FALSE, auctioneer, A_Board.auc[number].obj, ch, TO_VICT);
          act("$n gives $p to $N.", FALSE, auctioneer, A_Board.auc[number].obj, ch, TO_NOTVICT);
          sprintf(buf, "%s You may now collect your gold for lot #%d (%s).",
	          A_Board.auc[number].owner, number,
	          OBJ_SHORT(A_Board.auc[number].obj));
          do_tell(auctioneer, buf, 19);

          obj_to_char(A_Board.auc[number].obj, ch);
          A_Board.auc[number].obj = 0;
          GET_GOLD(ch) -= A_Board.auc[number].value;
          free(A_Board.auc[number].last_bidder);
          A_Board.auc[number].last_bidder = 0;
          A_Board.auc[number].status = AUCTION_COLLECT;
          A_Board.obj_num--;
          A_Board.auc[number].time = time(0);
        } /* if else */
        found = TRUE;
      } /* if */
    } /* for */
    if(!found)
      act("$n tells you 'But you haven't bid on anything yet.'", FALSE, auctioneer, 0, ch, TO_VICT);
    save_auction();
    save_char(ch, NOWHERE);
    return;
  } else if((number = atoi(buf)) < 0) {
    act("$n tells you 'Please make that a positive number.'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if(number >= MAX_OBJS_AUCTION || A_Board.auc[number].status == AUCTION_FREE) {
    act("$n tells you 'Sorry but there isn't any such lot number.'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if(A_Board.auc[number].status != AUCTION_SOLD) {
    act("$n tells you 'I'm sorry, but that hasn't been sold yet.'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if(strcmp(GET_NAME(ch), A_Board.auc[number].last_bidder)) {
    act("$n tells you 'I'm sorry, but that hasn't been sold to you.'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if(GET_GOLD(ch) < A_Board.auc[number].value) {
    act("$n tells you 'I'm sorry, but you don't have enough gold on you.'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  act("$n gives you $p.", FALSE, auctioneer, A_Board.auc[number].obj, ch, TO_VICT);
  act("$n gives $p to $N.", FALSE, auctioneer, A_Board.auc[number].obj, ch, TO_NOTVICT);

  sprintf(buf, "%s You may now come collect your gold for lot #%d (%s).",
	  A_Board.auc[number].owner, number,
	  OBJ_SHORT(A_Board.auc[number].obj));
  do_tell(auctioneer, buf, 19);

  obj_to_char(A_Board.auc[number].obj, ch);
  A_Board.auc[number].obj = 0;
  GET_GOLD(ch) -= A_Board.auc[number].value;
  free(A_Board.auc[number].last_bidder);
  A_Board.auc[number].last_bidder = 0;
  A_Board.auc[number].status = AUCTION_COLLECT;
  A_Board.obj_num--;
  A_Board.auc[number].time = time(0);

  save_auction();
  save_char(ch, NOWHERE);
}

void auction_auction(CHAR *auctioneer, CHAR *ch, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int number, start;
  struct obj_data *tmp_obj;
  struct descriptor_data *i;

  for(number = 0; number < MAX_OBJS_AUCTION; number++)
    if(A_Board.auc[number].status == AUCTION_FREE)
      break;

  if(number == MAX_OBJS_AUCTION) {
    act("$n tells you 'I'm sorry, but the auction board is full.", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }

  arg = one_argument(arg, buf);
  if(!*buf) {
    act("$n asks from you 'What do you want to auction?'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if(!( tmp_obj = get_obj_in_list_vis( ch, buf, ch->carrying))) {
    act("$n tells you 'But you don't have that item!'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if((tmp_obj->obj_flags.type_flag == ITEM_FOOD) ||
     (tmp_obj->obj_flags.type_flag == ITEM_TRASH) ||
     (tmp_obj->item_number < 0) ||
     (tmp_obj->obj_flags.cost < MIN_OBJ_VALUE) ||
     (IS_SET(tmp_obj->obj_flags.extra_flags, ITEM_CLONE)) ||
     (IS_SET(tmp_obj->obj_flags.extra_flags, ITEM_ANTI_RENT)) ||
     (IS_SET(tmp_obj->obj_flags.extra_flags, ITEM_ANTI_AUCTION))) {
    act("$n tells you 'You cannot put $p in auction.'", FALSE, auctioneer, tmp_obj, ch, TO_VICT);
    return;
  }

  arg = one_argument(arg, buf);
  if(!*buf) {
    act("$n asks from you 'What would be the starting bid?'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if((start = atoi(buf)) < 1000) {
    act("$n tells you 'Minimum bids must be over 1000 coins.'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }

  if((start = atoi(buf)) > 50000000) {
    act("$n tells you 'Maximum bids must be under 50000000 coins.'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }

  obj_from_char(tmp_obj);
  A_Board.auc[number].obj = tmp_obj;
  A_Board.auc[number].value = start;
  A_Board.auc[number].owner = str_dup(GET_NAME(ch));
  A_Board.auc[number].status = AUCTION_BIDS;
  A_Board.auc[number].time   = time(0);
  A_Board.obj_num++;

  act("$n gives $p to $N.",1,ch,tmp_obj,auctioneer,TO_ROOM);

  sprintf(buf, "$n tells you 'New lot (#%d): %s.'\n\r" \
	       "$n tells you 'Bids start at %d, HELP BID for more details.'",
	  number, OBJ_SHORT(tmp_obj), start);

  for (i = descriptor_list; i; i = i->next)
    if (i->character != ch &&
	!i->connected &&
	!IS_SET(i->character->specials.pflag, PLR_NOSHOUT) &&
	IS_SET(i->character->specials.pflag, PLR_AUCTION))
      act(buf, FALSE, auctioneer, 0, i->character, TO_VICT);

  send_to_char("Ok.\n\r", ch);
  save_auction();
  save_char(ch, NOWHERE);
}

void auction_collect(CHAR *auctioneer, CHAR *ch)
{
  char buf[MAX_STRING_LENGTH];
  int found, i;

  found = FALSE;
  for(i = 0; i < MAX_OBJS_AUCTION; i++) {
    if(A_Board.auc[i].status == AUCTION_COLLECT &&
       !str_cmp(A_Board.auc[i].owner, GET_NAME(ch))) {
      GET_GOLD(ch) += A_Board.auc[i].value - 100;
      sprintf(buf, "$n gives you %d coins for your lot #%d.",
              A_Board.auc[i].value - 100, i);
      act(buf, FALSE, auctioneer, 0, ch, TO_VICT);
      free(A_Board.auc[i].owner);
      A_Board.auc[i].owner = 0;
      A_Board.auc[i].value = 0;
      A_Board.auc[i].status = AUCTION_FREE;
      A_Board.auc[i].time = 0;
      found = TRUE;
    }
  }
  if(!found) {
    act("$n tells you 'But there isn't anything for you to collect.'", FALSE, auctioneer, 0, ch, TO_VICT);
  } else {
    save_auction();
    save_char(ch, NOWHERE);
  }
}
int check_god_access(CHAR *ch, int active);
int spec_auctioneer(CHAR *auctioneer, CHAR *ch, int cmd, char *arg)
{
  int irc;

  if(cmd==MSG_MOBACT) {
    if (!IS_NPC(auctioneer))
      log_f("Error in spec_auctioneer");
    else
      update_auction(auctioneer);
    return(FALSE);
    }
  if(!ch)
      return(FALSE);
  if (IS_NPC(ch))
      return(FALSE);

  if (!auctioneer) {
      log_f("No auctioneer.\n\r");
      produce_core();
    }

  if (!AWAKE(auctioneer)) {
      return(FALSE);
    }

  switch(cmd) {
    case CMD_LOOK: {
      irc = auction_look( auctioneer, ch, arg);
      return(irc);
    } break;
    case CMD_RECOVER:
      if(!check_god_access(ch,TRUE) || GET_LEVEL(ch)<LEVEL_SUP) {
        act("$n says, 'You don not have this ability!'", FALSE, auctioneer, 0, 0, TO_ROOM);
        return(TRUE);
      }
      auction_recover(auctioneer,ch, arg);
      return(TRUE);
      break;
    case CMD_BUY: {
      if (!CAN_SEE(auctioneer, ch)) {
        act("$n says, 'I refuse to deal with someone I can't see!'", FALSE, auctioneer, 0, 0, TO_ROOM);
        return(TRUE);
      }
      auction_buy(auctioneer, ch, arg);
      return(TRUE);
    } break;
    case CMD_AUCTION: {
      if (!CAN_SEE(auctioneer, ch)) {
        act("$n says, 'I refuse to deal with someone I can't see!'", FALSE, auctioneer, 0, 0, TO_ROOM);
        return(TRUE);
      }
      auction_auction(auctioneer, ch, arg);
      return(TRUE);
    } break;
    case CMD_COLLECT: {
      if (!CAN_SEE(auctioneer, ch)) {
        act("$n says, 'I refuse to deal with someone I can't see!'", FALSE, auctioneer, 0, 0, TO_ROOM);
        return(TRUE);
      }
      auction_collect(auctioneer, ch);
      return(TRUE);
    } break;
    default: {
      return(FALSE);
    } break;
    } /* switch */

  return(FALSE);
}

void do_bid(CHAR *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH], tmp2[MAX_INPUT_LENGTH];
  int number, amount;
  CHAR *mob;

  arg = one_argument(arg, tmp);
  one_argument(arg, tmp2);
  if(!*tmp2) {
    send_to_char("Usage: bid <lot number> <gold amount>\n\r", ch);
    return;
  }
  mob = 0;
  if(mob_proto_table[real_mobile(3010)].number) {
    for(mob = character_list; mob; mob = mob->next)
      if(IS_MOB(mob))
        if(mob_proto_table[mob->nr].func == spec_auctioneer)
          break;
  }

  if(!mob) {
    send_to_char("Sorry, no auctioneer...\n\r", ch);
    return;
  }

  if (!CAN_SEE(mob, ch)) {
    send_to_char("Sorry, but the auctioneer cannot see you.\n\r", ch);
    return;
  }

  if (!AWAKE(mob)) {
    send_to_char("Sorry, but the auctioneer is sleeping.\n\r", ch);
    return;
  }

  if((number = atoi(tmp)) < 0 ||
     number >= MAX_OBJS_AUCTION ||
     A_Board.auc[number].status == AUCTION_FREE) {
    act("$n tells you 'Sorry, but there isn't such lot number.'", FALSE, mob, 0, ch, TO_VICT);
    return;
  }
  if(str_cmp(A_Board.auc[number].owner, GET_NAME(ch)) == 0) {
    act("$n tells you 'You cannot bid on your own items.'", FALSE, mob, 0, ch, TO_VICT);
    return;
  }
  if(A_Board.auc[number].last_bidder)
   if(str_cmp(A_Board.auc[number].last_bidder, GET_NAME(ch)) == 0) {
    act("$n tells you 'You already have the last bid on that lot.'", FALSE, mob, 0, ch, TO_VICT);
    return;
  }
  if(A_Board.auc[number].status == AUCTION_SOLD ||
     A_Board.auc[number].status == AUCTION_COLLECT) {
    act("$n tells you 'That lot has already been sold.'", FALSE, mob, 0, ch, TO_VICT);
    return;
  }
  if((amount = atoi(tmp2)) > GET_GOLD(ch)) {
    act("$n tells you 'You don't have that much gold on you.'", FALSE, mob, 0, ch, TO_VICT);
    return;
  }

  if(!A_Board.auc[number].last_bidder && A_Board.auc[number].value > amount) {
    sprintf(buf, "$n tells you 'Bidding starts from %d coins on that lot.'",
            A_Board.auc[number].value);
    act(buf, FALSE, mob, 0, ch, TO_VICT);
    return;
  }
  if (A_Board.auc[number].last_bidder && A_Board.auc[number].value >= amount) {
    sprintf(buf, "$n tells you 'That lot already has a bid of %d coins.'",
            A_Board.auc[number].value);
    act(buf, FALSE, mob, 0, ch, TO_VICT);
    return;
  }

  if (A_Board.auc[number].last_bidder && A_Board.auc[number].value+1000 > amount) {
    act("$n tells you 'You must raise the bid by at least 1000 coins.'", FALSE, mob, 0, ch, TO_VICT);
    return;
  }

  if(A_Board.auc[number].last_bidder) {
    sprintf(buf, "%s %s just bid for your lot #%d (%s) %d coins.",
            A_Board.auc[number].last_bidder, GET_NAME(ch), number,
            OBJ_SHORT(A_Board.auc[number].obj), amount);
    do_tell(mob, buf, 19);
    free(A_Board.auc[number].last_bidder);
  }

  sprintf(buf, "%s %s just bid for your lot #%d (%s) %d coins.",
          A_Board.auc[number].owner, GET_NAME(ch), number,
          OBJ_SHORT(A_Board.auc[number].obj), amount);
  do_tell(mob, buf, 19);

  act("$n tells you 'You just made a bid for $p.'", FALSE, mob, A_Board.auc[number].obj, ch, TO_VICT);
  sprintf(buf, "$n says '$N just made a new bid %d coins for lot #%d ($p).'", amount, number);
  act( buf, FALSE, mob, A_Board.auc[number].obj, ch, TO_ROOM);

  A_Board.auc[number].last_bidder = str_dup(GET_NAME(ch));
  A_Board.auc[number].value = amount;
  A_Board.auc[number].status = AUCTION_BIDS;
}

/* Check auction at normal reboot and identify added by Ranger May 96
**
** Written for RoninDiku
**
** Please do not distribute without permission of your friendly
** IMP or the author
*/

void check_auction(void) {
  char buf[MAX_STRING_LENGTH];
  int i, needsave;
  needsave = FALSE;

  for(i = 0; i < MAX_OBJS_AUCTION; i++) {
    switch(A_Board.auc[i].status) {
      case AUCTION_SOLD: {
        sprintf(buf, "Auction Check: %s (SOLD: %s %d coins) (%s) - Put back on block",
                OBJ_SHORT(A_Board.auc[i].obj), A_Board.auc[i].last_bidder,
                A_Board.auc[i].value, A_Board.auc[i].owner);
        log_s(buf);
        free(A_Board.auc[i].last_bidder);
        A_Board.auc[i].last_bidder = 0;
        A_Board.auc[i].status = AUCTION_BIDS;
        A_Board.auc[i].time = time(0);
        needsave = TRUE;
        continue;
      } break;

      default: {
        continue;
      } break;
    } /* switch */
  } /* for */
  if(needsave) save_auction();
}

void spell_identify(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj);
void auction_identify(CHAR *ch, char *arg, int cmd) {

  char tmp[MAX_INPUT_LENGTH];
  int number;
  CHAR *mob;

  arg = one_argument(arg, tmp);
  if(!*tmp) {
    send_to_char("Usage: identify <lot number>\n\r", ch);
    return;
  }
  mob = 0;
  if(mob_proto_table[real_mobile(3010)].number) {
    for(mob = character_list; mob; mob = mob->next)
      if(IS_MOB(mob))
        if(mob_proto_table[mob->nr].func == spec_auctioneer)
          break;
  }

  if(!mob) {
    send_to_char("Sorry no auctioneer...\n\r", ch);
    return;
  }

  if (!CAN_SEE(mob, ch)) {
    send_to_char("Sorry but the auctioneer cannot see you.\n\r", ch);
    return;
  }

  if (!AWAKE(mob)) {
    send_to_char("Sorry but auctioneer is sleeping.\n\r", ch);
    return;
  }

  if(!is_number(tmp) ) {
    send_to_char("Please indicate a proper lot number.\n\r",ch);
    return;
  }

  if((number = atoi(tmp)) < 0 ||
     number >= MAX_OBJS_AUCTION ||
     A_Board.auc[number].status == AUCTION_FREE) {
    act("$n tells you 'Sorry but there isn't any such lot number.'", FALSE, mob, 0, ch, TO_VICT);
    return;
  }

  if(A_Board.auc[number].status == AUCTION_SOLD ||
     A_Board.auc[number].status == AUCTION_COLLECT) {
    act("$n tells you 'That lot has already been sold.'", FALSE, mob, 0, ch, TO_VICT);
    return;
  }

  if(3500 > GET_GOLD(ch)) {
    act("$n tells you 'Sorry but you don't have enough gold for an id.'", FALSE, mob, 0, ch, TO_VICT);
    return;
  }

  act("$n magically removes 3500 coins from your purse.", FALSE, mob, 0, ch, TO_VICT);
  GET_GOLD(ch)=GET_GOLD(ch)-3500;

  spell_identify(GET_LEVEL(ch), ch, 0, A_Board.auc[number].obj);
}

void auction_recover(CHAR *auctioneer, CHAR *ch, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int number;

  arg = one_argument(arg, buf);
  if(!*buf) {
    act("$n asks you 'What object do you want to recover from auction ?'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if((number = atoi(buf)) < 0) {
    act("$n tells you 'Please make that a positive number.'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  if(number >= MAX_OBJS_AUCTION || A_Board.auc[number].status == AUCTION_FREE) {
    act("$n tells you 'Sorry but there isn't any such lot number.'", FALSE, auctioneer, 0, ch, TO_VICT);
    return;
  }
  act("$n gives you $p.", FALSE, auctioneer, A_Board.auc[number].obj, ch, TO_VICT);
  act("$n gives $p to $N.", FALSE, auctioneer, A_Board.auc[number].obj, ch, TO_NOTVICT);
  sprintf(buf,"WIZINFO: %s recovers %s (%d) from auction.",GET_NAME(ch),OBJ_SHORT(A_Board.auc[number].obj),number);
  log_s(buf);
  wizlog(buf,GET_LEVEL(ch),5);

  obj_to_char(A_Board.auc[number].obj, ch);
  A_Board.auc[number].obj = 0;
  free(A_Board.auc[number].last_bidder);
  A_Board.auc[number].last_bidder = 0;
  A_Board.auc[number].status = AUCTION_FREE;
  A_Board.obj_num--;
  A_Board.auc[number].time = time(0);
  save_auction();
  save_char(ch, NOWHERE);
}
