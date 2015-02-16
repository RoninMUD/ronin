/*
 Special Procedure Module                   Orig. Date 03-Mar-2000

 spec.underworld.c

 Written by Ranger of RoninMUD
 Brutally Hacked by Sumo of RoninMUD
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:11:28 $
$Header: /home/ronin/cvs/ronin/spec.underworld.c,v 2.0.0.1 2004/02/05 16:11:28 ronin Exp $
$Id: spec.underworld.c,v 2.0.0.1 2004/02/05 16:11:28 ronin Exp $
$Name:  $
$Log: spec.underworld.c,v $
Revision 2.0.0.1  2004/02/05 16:11:28  ronin
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
#include "interpreter.h"
#include "act.h"

/*
modification of death_cry function-
sends death cry to others in the room if they arent dead themselves :*/

/*void crashing_noise(CHAR *ch)
{
  int door, was_in;

  act("Your blood freezes as you hear $n's death cry.",
      FALSE, ch,0,0,TO_NOTVICT);
  was_in = CHAR_REAL_ROOM(ch);

  if (was_in == -1) return;

  for (door = 0; door <= 5; door++) {
   if (CAN_GO(ch, door)) {
     ch->in_room_r = world[was_in].dir_option[door]->to_room_r;
     act("You cringe in horror at the sound of tons of rock crashing onto someone.",
         FALSE,ch,0,0,TO_ROOM);
     ch->in_room_r = was_in;
   }
  }
}
*/

/*
collapsing roof
Room 11166
*/
int collapsing_roof(int room, CHAR *ch, int cmd, char *arg) {

  if (!ch) return FALSE;

  if(cmd==MSG_ENTER) {
      if (IS_NPC(ch)) return FALSE;
      if(chance(50)) return FALSE;
      send_to_char("The ceiling beams collapse onto you with a large *CRACK*\n\r", ch);
      damage(ch,ch,number(350,900),TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
  }
  return FALSE;
}

/*
cracking timbers - room 11165
*/
int cracking_timbers(int room, CHAR *ch, int cmd, char *arg) {
  if(cmd!=MSG_MOBACT) return FALSE;
  if(!world[room].people) return FALSE;
  if(chance(75)) return FALSE;
  send_to_room ("The sound of timbers cracking is heard nearby.\n\r",room);
  return FALSE;
}


void assign_underworld(void) {


  assign_room(11166,  collapsing_roof);
  assign_room(11165,  cracking_timbers);


}
