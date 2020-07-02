/*
///   Special Procedure Module                   Orig. Date 15-02-1994
///
///   spec.chaos.c --- Specials for The Chaos Kingdom
///
///   Using this code is not allowed without permission from originator.
*/

/*
$Author: ronin $
$Date: 2004/11/16 04:59:08 $
$Header: /home/ronin/cvs/ronin/spec.chaos.c,v 2.1 2004/11/16 04:59:08 ronin Exp $
$Id: spec.chaos.c,v 2.1 2004/11/16 04:59:08 ronin Exp $
$Name:  $
$Log: spec.chaos.c,v $
Revision 2.1  2004/11/16 04:59:08  ronin
Typo fix.

Revision 2.0.0.1  2004/02/05 16:10:18  ronin
Reinitialization of cvs archives

Revision 1.3  2002/06/05 02:54:59  ronin
Fix to stop dagger specing on backstab if owner is fighting.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"

#include "utility.h"
#include "cmd.h"
#include "spec_assign.h"

void stop_fighting (CHAR *);
void hit (CHAR *, CHAR *, int);
char *one_argument(char *, char *);

#define CHAOS_ZONE      10800

#define TZ_DAGGER       ITEM(CHAOS_ZONE,19)

void assign_chaos (void) {
  int TZ_Dagger (OBJ *obj, CHAR *ch, int cmd, char *arg);

  assign_obj(TZ_DAGGER, TZ_Dagger);
}

int
TZ_Dagger (OBJ *dagger, CHAR *ch, int cmd, char *arg) {
  int found;
  CHAR *vict,*temp;
  OBJ *obj=0;
  int start_room;
  int DONE = FALSE;

  if (!IS_MORTAL(ch))
    return FALSE;

  if (dagger == EQ(ch, WIELD)) found = WIELD;
  else if (dagger == EQ(ch, HOLD)) found = HOLD;
  else return FALSE;

  switch(cmd)
    {
	case CMD_PRACTICE:
	  if(AWAKE(ch) && *arg)
	  {
	    send_to_char("Your mind is too chaotic to concentrate!\n\r",ch);
	    return TRUE;
      }
	  break;
    case CMD_BACKSTAB:
    case CMD_CIRCLE:

      while((*arg == ' ')) arg++;
      if ((vict = get_char_room_vis(ch,arg)))
        if(GET_LEVEL(vict) >= GET_LEVEL(ch))
          {
    	    start_room = CHAR_REAL_ROOM(ch);
            switch (cmd)
	      {
	      case CMD_BACKSTAB:
	        if ((GET_CLASS(ch) != CLASS_THIEF) &&
	    	    (GET_CLASS(ch) != CLASS_ANTI_PALADIN))
	          {
	    	    send_to_char("You don't know this skill.\n\r", ch);
	   	    return TRUE;
	         }
            if (ch->specials.fighting) break; /* Linerfix 06/03/02, no backstab kill while fight, dammit */
	    if (number(0,30)==0)  /* Stick in mob */
	      {
		act("$N screams as $n sticks $p is $N's back, burying it too deeply \n\rto be removed."
		    ,FALSE, ch, dagger, vict,TO_NOTVICT);
		act("$N screams as you stick $p in $S back, but you are unable to \n\rremove it."
		    ,FALSE, ch, dagger, vict,TO_CHAR);
		act("You scream as $n's dagger sticks in your back."
		    ,FALSE, ch, dagger, vict,TO_VICT);
		dagger->log=1;
		obj_to_char(unequip_char(ch,found),vict);
		damage(ch,vict,300 + number(0,300), TYPE_UNDEFINED,DAM_NO_BLOCK);
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
		DONE = TRUE;
	      }
	    else
	      if (number(0,100)==0)  /* Tear in the warp */
		{
		  act("$N explodes in a ball of chaotic energy as $n sticks $p into $N's back."
		      ,FALSE, ch, dagger, vict,TO_NOTVICT);
		  act("Your $p burns white hot as $N screams and explodes in a deafening blast of chaos."
		      ,FALSE, ch, dagger, vict,TO_CHAR);
		  act("Your body is ripped apart in a deafening explosion of chaotic energy."
		      ,FALSE, ch, dagger, vict,TO_VICT);
		  for (temp = world[CHAR_REAL_ROOM(vict)].people;temp;temp = temp->next_in_room)
		    damage(vict,temp,fabs(GET_HIT(temp)/2), TYPE_UNDEFINED,DAM_NO_BLOCK);
		  damage(ch,vict,32000, TYPE_KILL,DAM_NO_BLOCK);
		  for (obj = world[start_room].contents; obj; obj = obj->next_content)
		    extract_obj(obj);
		  DONE = TRUE;
		}
	    break;
	  case CMD_CIRCLE:
	    if (GET_CLASS(ch) != CLASS_THIEF)
	      {
                send_to_char("You don't know this skill.\n\r", ch);
                return TRUE;
	      }
	    found = get_random_obj_eq(vict);
	    if (found>= 0 && number(0,100)==0)
	      {
                obj = unequip_char(vict,found);
		act("$n's weapon twists in $s hand, and destroys $N's $p with its chaotic yearnings."
		    ,FALSE, ch, obj, vict,TO_NOTVICT);
		act("Your weapon twists in your hand, seeking out $p to satisfy its chaotic desires."
		    ,FALSE, ch, obj, vict,TO_CHAR);
		act("Your $p is eaten by the chaotic energy of $n's weapon."
		    ,FALSE, ch, obj, vict,TO_VICT);
		extract_obj(obj);
                  }
	        break;
	      }
          }
       break;
    case MSG_OBJ_REMOVED:
	act("$n's weapon whines piteously in $s hand, not wanting to end its destruction."
	    ,FALSE, ch, obj, NULL,TO_NOTVICT);
	act("Your weapon whines piteously, upset that you'd stop using it to spread chaos."
	    ,FALSE, ch, obj, NULL,TO_CHAR);
        break;
    }
  return DONE;
}




