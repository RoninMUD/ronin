/* Specs for area Death's Playground Ancient Graves by Arodtanjoe

** Written by Arodtanjoe - Feb 2025
*/



/*System Includes */
#include <string.h>
#include <stdlib.h>

/*Ronin Includes */
#include "structs.h"
#include "utils.h"
#include "act.h"
#include "db.h"
#include "char_spec.h"
#include "cmd.h"
#include "comm.h"
#include "constants.h"
#include "enchant.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "mob.spells.h"
#include "spec_assign.h"
#include "spells.h"
#include "subclass.h"
#include "utility.h"


/*Rooms */

#define SENTINEL_ROOM 21495

#define GRAVESTONE_ONE_ROOM_START 21461
#define GRAVESTONE_ONE_ROOM_END 21600


/*Objects */

#define GRAVESTONE_ONE 21407
#define GRAVESTONE_TWO 21408

#define NAMELESS_VEIL 21400
#define CHATTERING_RELIC 21401
#define SILENCE_SHROUD 21402
#define RUSTED_OATHPLATE 21403
#define SORROW_LOCKET 21404
#define FUNERAL_WRAPPING 21405



/*Mobs */



/*Miscellaneous strings */
// Generic States that are shifted to indicate different stages.

#define CRYPT_ZONE 216

#define STATE1 (1 << 0) // 1
#define STATE2 (1 << 1) // 2
#define STATE3 (1 << 2) // 4
#define STATE4 (1 << 3) // 8





int dpag_block_e(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_es(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_esw(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_SOUTH || cmd==CMD_WEST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_n(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_ne(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_nes(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_SOUTH || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_new(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST || cmd==CMD_NORTH || cmd==CMD_EAST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_ns(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_SOUTH || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_nws(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST || cmd==CMD_SOUTH || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_s(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_se(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_w(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_we(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_WEST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_wn(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_wne(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_WEST || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_ws(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST || cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dpag_block_wse(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_WEST || cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The greater tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}



int obsidian_sentinel_block(int room,CHAR *ch,int cmd,char *argument) {
  CHAR *tmp;
  int sentinel_num = 21400;
  
  if(cmd==CMD_NORTH || cmd==CMD_EAST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
	
	 for (tmp = world[real_room(SENTINEL_ROOM)].people; tmp; tmp = tmp->next_in_room)
		{
			if (IS_MOB(tmp))
			{
				if (V_MOB(tmp) == sentinel_num)
				{
					//Check if the Obsidian Sentinel is still alive. If it is, block movement.	
					send_to_char("The graveyard seal prevents you from moving forward.\n\r",ch);										
					return TRUE;
				}			
			}
		}	
  }
  return FALSE;
}


/* Special Gravestone 1 - Grants access to Deaths Playground Crypts. */
// Push the protrusion on the tombstone to unlock the stairway leading down.
int dpag_gravestone_one(OBJ *obj, CHAR *ch, int cmd, char *arg)
{

    char buf[MAX_INPUT_LENGTH];
    bool bReturn = FALSE;
    

    if (ch && cmd == CMD_MOVE )
    {
        one_argument(arg, buf);
        if (*buf && !strncmp(buf, "protrusion", MAX_INPUT_LENGTH))
        {	
			
			send_to_room("The gravestone glows and shakes.\n\r", real_room(GRAVESTONE_ONE_ROOM_START));
			
			//Ensure the other zone is loaded before this is attempted.
			if(real_zone(CRYPT_ZONE) != -1){		
			
				if (world[real_room(GRAVESTONE_ONE_ROOM_START)].dir_option[DOWN]->to_room_r == -1)
				{
				  world[real_room(GRAVESTONE_ONE_ROOM_START)].dir_option[DOWN]->to_room_r = real_room(GRAVESTONE_ONE_ROOM_END);
				  world[real_room(GRAVESTONE_ONE_ROOM_END)].dir_option[UP]->to_room_r = real_room(GRAVESTONE_ONE_ROOM_START);
				  send_to_room("The gravestone snaps open, revealing a path into the earth.\n\r", real_room(GRAVESTONE_ONE_ROOM_START));
				  send_to_room("The ceiling opens revealing a path back outside.\n\r", real_room(GRAVESTONE_ONE_ROOM_END));				  
				}			
			}
			
			bReturn = TRUE;
			
        }
    }
    return bReturn;
}


int dpag_gravestone_one_link(int room,CHAR *ch,int cmd,char *argument) {


	if(cmd==CMD_WEST || cmd==CMD_NORTH || cmd==CMD_EAST) {
		if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
		send_to_char("The greater tombstones block your way.\n\r",ch);
		return TRUE;
	  }


   
  if (cmd == MSG_ZONE_RESET)
  {

    if (world[real_room(GRAVESTONE_ONE_ROOM_START)].dir_option[DOWN]->to_room_r != -1)
    {
      world[real_room(GRAVESTONE_ONE_ROOM_START)].dir_option[DOWN]->to_room_r = -1;
      world[real_room(GRAVESTONE_ONE_ROOM_END)].dir_option[UP]->to_room_r = -1;
      send_to_room("The gravestone slams shut.\n\r", real_room(GRAVESTONE_ONE_ROOM_START));
      send_to_room("The gravestone slams shut.\n\r", real_room(GRAVESTONE_ONE_ROOM_END));
    }
  }	
	
  return FALSE;
}


/* Special Gravestone 1 - Grants access to Deaths Playground Caves. */
// Read or Examine the inscriptions to unlock the path to the caves.
int dpag_gravestone_two(OBJ *obj, CHAR *ch, int cmd, char *arg)
{

    char buf[MAX_INPUT_LENGTH];
    bool bReturn = FALSE;
    

    if (ch && (cmd == CMD_READ || cmd == CMD_EXAMINE ))
    {
        one_argument(arg, buf);
        if (*buf && !strncmp(buf, "inscriptions", MAX_INPUT_LENGTH))
        {
            
            act("The gravestone glows and shakes.", 0, ch, 0, 0, TO_ROOM);
            act("The gravestone glows and shakes.", 0, ch, 0, 0, TO_CHAR);
            
            bReturn = TRUE;
        }
    }
    return bReturn;
}

void assign_deathplaygroundancientgrave(void) {

/*Objects */
assign_obj(GRAVESTONE_ONE, dpag_gravestone_one);
assign_obj(GRAVESTONE_TWO, dpag_gravestone_two);

/*Rooms*/ 
assign_room(21400,dpag_block_nws);
assign_room(21401,dpag_block_n);
assign_room(21402,dpag_block_ns);
assign_room(21403,dpag_block_ns);
assign_room(21404,dpag_block_ns);
assign_room(21405,dpag_block_ne);
assign_room(21406,dpag_block_wn);
assign_room(21407,dpag_block_ns);
assign_room(21408,dpag_block_ns);
assign_room(21409,dpag_block_ne);
assign_room(21410,dpag_block_wn);
assign_room(21411,dpag_block_es);
assign_room(21412,dpag_block_wn);
assign_room(21413,dpag_block_ns);
assign_room(21414,dpag_block_ns);
assign_room(21415,dpag_block_es);
assign_room(21416,dpag_block_ws);
assign_room(21417,dpag_block_n);
assign_room(21418,dpag_block_nes);
assign_room(21419,dpag_block_we);
assign_room(21420,dpag_block_ws);
assign_room(21421,dpag_block_ne);
assign_room(21422,dpag_block_ws);
assign_room(21423,dpag_block_ne);
assign_room(21424,dpag_block_nws);
assign_room(21425,dpag_block_ns);
assign_room(21426,dpag_block_ne);
assign_room(21427,dpag_block_wse);
assign_room(21428,dpag_block_wn);
assign_room(21429,dpag_block_es);
assign_room(21430,dpag_block_wne);
assign_room(21431,dpag_block_ws);
assign_room(21432,dpag_block_nes);
assign_room(21433,dpag_block_ws);
assign_room(21434,dpag_block_ns);
assign_room(21435,dpag_block_ne);
assign_room(21436,dpag_block_we);
assign_room(21437,dpag_block_wn);
assign_room(21438,dpag_block_e);
assign_room(21439,dpag_block_new);
assign_room(21440,dpag_block_ws);
assign_room(21441,dpag_block_ne);
assign_room(21442,dpag_block_wn);
assign_room(21443,dpag_block_ns);
assign_room(21444,dpag_block_ns);
assign_room(21445,dpag_block_s);
assign_room(21446,dpag_block_s);
assign_room(21447,dpag_block_e);
assign_room(21448,dpag_block_ws);
assign_room(21449,dpag_block_e);
assign_room(21450,dpag_block_wn);
assign_room(21451,dpag_block_s);
assign_room(21452,dpag_block_se);
assign_room(21453,dpag_block_wn);
assign_room(21454,dpag_block_ns);
assign_room(21455,dpag_block_ns);
assign_room(21456,dpag_block_ne);
assign_room(21457,dpag_block_we);
assign_room(21458,dpag_block_wn);
assign_room(21459,dpag_block_es);
assign_room(21460,dpag_block_we);
assign_room(21461,dpag_gravestone_one_link);
assign_room(21462,dpag_block_wn);
assign_room(21463,dpag_block_se);
assign_room(21464,dpag_block_nws);
assign_room(21465,dpag_block_n);
assign_room(21466,dpag_block_se);
assign_room(21467,dpag_block_we);
assign_room(21468,dpag_block_we);
assign_room(21469,dpag_block_wne);
assign_room(21470,dpag_block_w);
assign_room(21471,dpag_block_se);
assign_room(21472,dpag_block_ws);
assign_room(21473,dpag_block_ns);
assign_room(21474,dpag_block_ne);
assign_room(21475,dpag_block_we);
assign_room(21476,dpag_block_wn);
assign_room(21477,dpag_block_se);
assign_room(21478,dpag_block_we);
assign_room(21479,dpag_block_we);
assign_room(21480,dpag_block_w);
assign_room(21481,dpag_block_ne);
assign_room(21482,dpag_block_wn);
assign_room(21483,dpag_block_ne);
assign_room(21484,dpag_block_we);
assign_room(21485,dpag_block_wse);
assign_room(21486,dpag_block_we);
assign_room(21487,dpag_block_new);
assign_room(21488,dpag_block_w);
assign_room(21489,dpag_block_se);
assign_room(21490,dpag_block_wse);
assign_room(21491,dpag_block_ws);
assign_room(21492,dpag_block_se);
assign_room(21493,dpag_block_ws);
assign_room(21494,dpag_block_se);
assign_room(21495,dpag_block_nws);
assign_room(21496,obsidian_sentinel_block);
assign_room(21497,dpag_block_se);
assign_room(21498,dpag_block_ws);
assign_room(21499,dpag_block_nes);

 
}





