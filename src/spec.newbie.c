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
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"
extern struct time_info_data time_info;

#define MAX_LENGTH 1000

/*
Newbie School specs - by Sane

When a newbie leaves the zone, hand them a diploma
*/

#define NEWBIE_SCHOOL_EXIT 6619
#define DIPLOMA_OBJ 6624

int last_newbie_room(int room, CHAR *ch, int cmd, char *arg)
{
   static CHAR* triedCH = NULL;
   OBJ* obj;

   if (!ch) return FALSE;
   if (cmd == CMD_DOWN && GET_LEVEL(ch) == 1)
   {
      triedCH = ch;
   }
   if (cmd == MSG_LEAVE && triedCH == ch && GET_LEVEL(ch) == 1)
   {
      send_to_char("As you enter the Temple of Midgaard for the first time, you wince\n\r", ch);
      send_to_char("briefly at the sun. Before you can get your bearings, a small\n\r", ch);
      send_to_char("Mephit wearing robes bearing the Crest of Midgaard appears before you.\n\r", ch);
      send_to_char("It mutters a few words in a language you don't understand, then\n\r", ch);
      send_to_char("hands you a sealed Diploma. You may now begin your life here in\n\r", ch);
      send_to_char("the Realm of Ronin...hopefully you learned all that you can, and\n\r", ch);
      send_to_char("will apply your knowledge to further your skills.\n\r\n\r", ch);
      obj=read_object(DIPLOMA_OBJ,VIRTUAL);if(obj) obj_to_char(obj,ch);
   }
   return FALSE;
}

void assign_newbie(void)
{
    assign_room(NEWBIE_SCHOOL_EXIT, last_newbie_room);
}

