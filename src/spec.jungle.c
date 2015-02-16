/*
///   Special Procedure Module                   Orig. Date 08-19-1996
///                                              Last Modif 10-06-1998
///
///   Spec.jungle.c --- Specials for Jungle of Despair, by Monkeyboy
///
///   Designed and coded by Lem of RoninMUD (im478@cleveland.freenet.edu)
///   Using this code is not allowed without permission from originator.
///
///   Added block for sentries, Ranger Oct 98
*/


/*
$Author: ronin $
$Date: 2004/02/05 16:10:51 $
$Header: /home/ronin/cvs/ronin/spec.jungle.c,v 2.0.0.1 2004/02/05 16:10:51 ronin Exp $
$Id: spec.jungle.c,v 2.0.0.1 2004/02/05 16:10:51 ronin Exp $
$Name:  $
$Log: spec.jungle.c,v $
Revision 2.0.0.1  2004/02/05 16:10:51  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "cmd.h"
#include "utility.h"
#include "act.h"
#include "spec_assign.h"

extern struct time_info_data time_info;
extern struct char_data *character_list;

extern int CHAOSMODE;
extern OBJ  *object_list;
extern CHAR *character_list;

#define ZONE_MONKEYBOY  27600
#define KETTLE             27627
#define MONKEY             27651
#define SENTRY             27681
#define SENTRYROOM         27681

int jungle_sentry(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd==CMD_WEST) {
    if(CHAR_VIRTUAL_ROOM(mob)!=SENTRYROOM) return FALSE;
    act("The sentry jumps up and blocks your entry into the hut.",0,ch,0,0,TO_CHAR);
    act("The sentry jumps up and blocks $n's entry into the hut.",0,ch,0,0,TO_ROOM);
    return TRUE;
  }
  return FALSE;
}

int drink_kettle(OBJ *kettle, CHAR *ch, int cmd, char *arg) {

  int condition;

  if(!ch) return FALSE;
  if (cmd != CMD_DRINK) return FALSE;
  if (GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;

/*
     We'll use the regular do_drink routine to test for enough
     liquid in the kettle, proper arguments, etc.
     The condition variable is used to see if the drink was
     successful.
*/

  condition = GET_COND(ch,THIRST);

  do_drink(ch,arg,cmd);

  if(GET_COND(ch,THIRST)>condition) {
    if (GET_COND(ch,THIRST) > -1) GET_COND(ch, THIRST) = 24;
    if (GET_COND(ch,FULL) > -1)   GET_COND(ch, FULL) = 24;
  }
  return(TRUE);
}

int jade_monkey(OBJ *monkey, CHAR *ch, int cmd, char *arg) {
  int hidden=FALSE;
  struct char_data *tar;

  /* New signal called MSG_ENTER sent to a room when player
     arrives in it. Send out from act.movement.c */
  if (!ch) return FALSE; /* Signal for MSG_ENTER passes the ch */
  if (cmd!=MSG_ENTER) return FALSE;
  if (!monkey) return FALSE;
  if (GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;

  for (tar=world[CHAR_REAL_ROOM(ch)].people ; tar ; tar=tar->next_in_room) {
    if(!IS_NPC(tar)&&GET_LEVEL(tar)>=LEVEL_IMM) continue;
    if(!CAN_SEE(ch,tar) || IS_AFFECTED(tar, AFF_HIDE)) hidden=TRUE;
  }

  if (hidden)
     act("Your jade monkey screams loudly!  There is a hidden life form in the room.\n\r"
         ,TRUE, ch, 0,0,TO_CHAR);

  return FALSE;
}

void assign_jungle(void) {
  assign_obj(KETTLE, drink_kettle);
  assign_obj(MONKEY, jade_monkey);
  assign_mob(SENTRY, jungle_sentry);
}
