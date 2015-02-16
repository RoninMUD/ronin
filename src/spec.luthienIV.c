/*
	Spec procedures for LuthienIV
	Written by Solmyr for RoninMUD
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

#define ANGEL    14305
#define GATEROOM 14344

int angel_spec(CHAR *angel, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];
  
  if((GET_POS(angel) == POSITION_STANDING) && (cmd == MSG_ENTER))
  {
    if(!ch) return FALSE;
	act("Who is this, that without death goes through the kingdom of the dead?",1,angel,0,ch,TO_VICT);
	return FALSE;
  }
  
  if((GET_POS(angel) == POSITION_STANDING) && (cmd == CMD_ENTER) && (world[CHAR_REAL_ROOM(angel)].number == GATEROOM))
  {
    if(!ch) return FALSE;
	one_argument(arg, buf);
	if(!*buf) return FALSE;
	if(strncmp(buf, "gate", 4)) return FALSE;
	do_say(angel, "In the name of the Dark Prince, no living soul shall pass this gate!", CMD_SAY);
	return TRUE;
  }
  return FALSE;
}

void assign_luthienIV(void)
{
  assign_mob(ANGEL, angel_spec);
}
