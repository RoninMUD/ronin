
/*
///   Special Procedure Module                   Orig. Date 13-01-1996
///
///   spec.helventia.c --- Specials for Helventia
///
///   Written by Shun of RoninMUD - qxotl@u.washington.edu
///
///   Using this code is not allowed without permission from originator.
*/

/*
$Author: ronin $
$Date: 2005/01/25 21:50:26 $
$Header: /home/ronin/cvs/ronin/spec.helventia.c,v 2.1 2005/01/25 21:50:26 ronin Exp $
$Id: spec.helventia.c,v 2.1 2005/01/25 21:50:26 ronin Exp $
$Name:  $
$Log: spec.helventia.c,v $
Revision 2.1  2005/01/25 21:50:26  ronin
Fix for cave bear loop with imm in room.

Revision 2.0.0.1  2004/02/05 16:10:46  ronin
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

#include "utility.h"
#include "cmd.h"
#include "spec_assign.h"


int helventia_ant_lion(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
   CHAR *vict, *tank, *next_vict = NULL;

   switch (cmd)
     {
     case MSG_MOBACT:

         if (!mob->specials.fighting)
             return FALSE;

         switch (number(0,2))
         {
            case 0:
            case 1:
              if(number(0,3))
              {
                if(!(vict = get_random_victim(mob)))
                     return FALSE;
                if(vict == mob)
                     return FALSE;

                act("$n grabs $N in its mandibles and throws $M to the ground!",
                     1, mob, 0, vict, TO_NOTVICT);
                act("$n grabs you in its mandibles and throws you to the ground!",
                     1, mob, 0, vict, TO_VICT);
                act("You grab $N in your mandibles and throw $M to the ground!",
                     1, mob, 0, vict, TO_CHAR);
                damage(mob, vict, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
                WAIT_STATE(vict,PULSE_VIOLENCE*3);
              }
              break;

            case 2:
              if(!number(0,1))
              {
                if(!(tank = mob->specials.fighting))
                     return FALSE;
                if(tank == mob)
                     return FALSE;

                act("$n grabs $N in its mandibles and uses $M as a giant swinging cudgel!",
                     1, mob, 0, tank, TO_NOTVICT);
                act("$n skewers you with its mandibles and swings you around the room!",
                     1, mob, 0, tank, TO_VICT);
                act("You grab $N in your mandibles and swing $M around the room!",
                     1, mob, 0, tank, TO_CHAR);
                damage(mob, tank, 500, TYPE_UNDEFINED,DAM_NO_BLOCK);
                WAIT_STATE(tank, PULSE_VIOLENCE*3);

                for(vict = world[mob->in_room_r].people; vict;vict = next_vict)
                {
                     if(vict == mob)
                          return FALSE;
                     if(vict != tank)
                     {
                        act("$n knocks you back as it bludgeons you with its prey!",
                            1, mob, 0, vict, TO_VICT);
                        act("You knock $N around the room as you bludgeon $M with your prey!",
                            1, mob, 0, vict, TO_CHAR);
                        damage(mob, vict, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
                        WAIT_STATE(vict, PULSE_VIOLENCE*2);
                     }
                     next_vict = vict->next_in_room;
                }
              }
              break;
         }
     }
     return FALSE;
}

int helventia_bear(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
   CHAR *vict, *next_vict;

   switch (cmd)
     {
     case MSG_MOBACT:
         if (!(mob->specials.fighting))
             return FALSE;
         if (!(vict = mob->specials.fighting))
             return FALSE;

         if(!number(0,1))
           {
           switch(number(0,10))
             {
             case 0:
             case 1:
             case 2:
             case 3:
             case 4:
             case 5:

                   act("$n grabs $N in a crushing hug!", 1, mob, 0, vict,TO_NOTVICT);
                   act("$n grabs you in a crushing hug!", 1, mob, 0, vict ,TO_VICT);
                   act("You grab $N in a crushing hug!", 1, mob, 0, vict, TO_CHAR);
                   damage(mob, vict, 350, TYPE_UNDEFINED,DAM_NO_BLOCK);
                   WAIT_STATE(vict,PULSE_VIOLENCE*3);
                   break;

             case 6:
             case 7:
             case 8:
             case 9:
             case 10:
                   act("$n lets loose with a giant, swinging slap!", 1, mob, NULL, NULL, TO_ROOM);
                   act("You let loose with a giant, swinging slap!", 1, mob, NULL, NULL, TO_CHAR);
                   for(vict = world[mob->in_room_r].people; vict;vict = next_vict)
                   {
                      next_vict = vict->next_in_room;
                      if(vict == mob)
                        return FALSE;
                      if(!IS_NPC(vict) && GET_LEVEL(vict)>LEVEL_MORT) continue;
                      damage(mob, vict, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
                      WAIT_STATE(vict, PULSE_VIOLENCE*2);
                     
                   }
                   break;
             }
           }
         break;

     }

   return FALSE;

 }

  /**********************************************************************\
 |* End Of the Special procedures for misc mobs and such               *|
 \**********************************************************************/

void assign_helventia (void) {

  assign_mob(6298 ,  helventia_bear);
  assign_mob(6273 ,  helventia_ant_lion);
}

