/**********************************************************************/
/****                                                              ****/
/****             Specs Ancient Egyptian Sphinx                    ****/
/****                                                              ****/
/**********************************************************************/

/****************************************************************************
 *               Special procedure module for SPHINX                         *
 *                                                                           *
 *                       by: R Lewis (Thyas)                                 *
 *                Special thanks: J Lasilla (Ervin) for his Barovia code     *
 *                                                                           *
 *             contact Thyas at : rdlewis@europa.cs.mun.ca                   *
 ****************************************************************************/

/*
$Author: ronin $
$Date: 2004/02/05 16:11:15 $
$Header: /home/ronin/cvs/ronin/spec.sphinx.c,v 2.0.0.1 2004/02/05 16:11:15 ronin Exp $
$Id: spec.sphinx.c,v 2.0.0.1 2004/02/05 16:11:15 ronin Exp $
$Name:  $
$Log: spec.sphinx.c,v $
Revision 2.0.0.1  2004/02/05 16:11:15  ronin
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
#include "fight.h"
#include "spec_assign.h"

/* Sword of Healing */
extern int CHAOSMODE;
int is_shop(CHAR *mob);

int sword_of_healing(OBJ *obj,CHAR *ch, int cmd, char *argument)
{

  CHAR *vic;
  char arg[MAX_STRING_LENGTH];

  if(cmd != CMD_KILL) return FALSE;
  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(!ch->specials.fighting &&ch->equipment[WIELD]  == obj)
    {

    /* Check that the target is valid */

    one_argument(argument, arg);


    if (!(vic = get_char_room_vis(ch, arg))) {
      send_to_char("Your sword splits the air, as your victim isn't here !\n\r",ch);
      return TRUE;
    }

    if(!IS_NPC(vic) && !IS_NPC(ch)){
      send_to_char("Your sword will not unveil it's wraith on other players !\n\r",ch);
      return TRUE;
    }


    /* Cast the spell on him */

    if(number(0,20) != 0) return FALSE;
    if(is_shop(vic)) return FALSE;

    act("A briliant flash of light envelops $n !",FALSE,ch,0,0,TO_ROOM);
    send_to_char("A brilliant flash of light envelopes you !\n\r",ch);
    cast_heal(30, ch, "", SPELL_TYPE_SPELL, ch, 0);
  }
  return FALSE;
}

#define SPHINX_ZONE     800
#define SPHINX_SWORD    ITEM(SPHINX_ZONE,2)

void assign_sphinx (void) {
  assign_obj(SPHINX_SWORD,sword_of_healing);
}
