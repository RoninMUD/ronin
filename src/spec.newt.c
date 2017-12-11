/* *************************************************************************
 * Special Proceedures for Xalth's Revamp of the Newts? =b                 *
 *                         Written by DarkLeha                             *
 * Small mod by Ranger                                                     *
 ************************************************************************* */

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
#include "act.h"
#include "reception.h"
#include "spec_assign.h"

int is_shop(CHAR *mob);

int newt_spec(OBJ *obj,CHAR *ch, int cmd, char *argument) {
  CHAR *vict;
  char arg[MAX_STRING_LENGTH];

  if(cmd != CMD_KILL) return FALSE;
  if(IS_NPC(ch)) return FALSE;

  if(obj->equipped_by!=ch) return FALSE;
  if(ch->specials.fighting) return FALSE;
  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  one_argument(argument, arg);
  if(!*arg) return FALSE;
  if(!(vict = get_char_room_vis(ch, arg))) return FALSE;
  if(is_shop(vict)) return FALSE;
  if(ch==vict) return FALSE;
  if((GET_LEVEL(ch)-GET_LEVEL(vict)) > 20) return FALSE;

  if(number(0,20)) return FALSE;
  if(is_shop(vict)) return FALSE;

  act("$n is showered with a white holy light!",FALSE,ch,0,0,TO_ROOM);
  act("You are showered with a white holy light!\n\r",FALSE,ch,0,0,TO_CHAR);
  spell_cure_critic(GET_LEVEL(ch),ch,ch,0);
  return FALSE;
}

void assign_newt() {
  assign_obj(2911, newt_spec);
  assign_obj(2913, newt_spec);
}
