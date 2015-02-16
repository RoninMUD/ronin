
/*
///   Special Procedure Module                   Orig. Date 03-02-1994
///
///   Spec.swamp.c --- Specials for The Swamp by Ere
///
///   Using this code is not allowed without permission from originator.

Addition of Ettin Headbutt spec - Jan 97 - Ranger
The sticky swamp spec hasn't been used in years and is
now commented out.
em says 'The large Ettin smacks its heads together with yours in the middle.'
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:11:18 $
$Header: /home/ronin/cvs/ronin/spec.swamp.c,v 2.0.0.1 2004/02/05 16:11:18 ronin Exp $
$Id: spec.swamp.c,v 2.0.0.1 2004/02/05 16:11:18 ronin Exp $
$Name:  $
$Log: spec.swamp.c,v $
Revision 2.0.0.1  2004/02/05 16:11:18  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"
#include "cmd.h"
#include "spec_assign.h"

#define ETTIN1       12017
#define ETTIN2       12208
#define LARGE_ETTIN1  12018
#define LARGE_ETTIN2  12208
#define BLACK_WORM    12015
#define PURPLE_WORM   12025




/*
void assign_swamp () {
 assign_mob(LARGE_ETTIN1 ,  swamp_large_ettin);
 assign_mob(LARGE_ETTIN2 ,  swamp_large_ettin);
 assign_mob(ETTIN1       ,  swamp_ettin);
 assign_mob(ETTIN2       ,  swamp_ettin);
 assign_mob(PURPLE_WORM  ,  swamp_purple_worm);
 assign_mob(BLACK_WORM   ,  swamp_black_worm);
}
*/

/* Old swamp specs

void assign_swamp()
{

  int stickyswamp (int room, CHAR *ch, int cmd, char *arg);

  int i=0;
  int swamp_rooms[] = { 12001,12002,12004,12005,12008,12010,
			  12011,12013,12014,12015,12016,12018,
			  12019,12020,12021,12022,12023,12024,
			  12026,12027,-1 };

  while(swamp_rooms[i]!=-1)
    {
      assign_room(swamp_rooms[i++],stickyswamp);
    }

}

int
stickyswamp(int room, CHAR *ch, int cmd, char *arg) {
  if (!IS_MORTAL(ch))
    return FALSE;

  if (cmd == CMD_EAST ||
      cmd == CMD_WEST ||
      cmd == CMD_NORTH||
      cmd == CMD_SOUTH  )
    {
      if (number(0,8)==0)
	{
	  act("$n gets $s feet stuck in the mud and goes nowhere fast.",
	      FALSE,ch,0,0,TO_NOTVICT);
	  act("Your feet get stuck in the mud and you go nowhere fast.",
	      FALSE,ch,0,0,TO_CHAR);
	  return(TRUE);
	}
    }
  return FALSE;
}
*/
