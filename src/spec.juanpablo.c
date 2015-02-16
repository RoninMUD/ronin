/*****************************A part of RoninMUD**************************** 
*                                                                          *
*                          Special Procedure Module                        *
*                                    for                                   *
*                             JuanPablo, by Louis                          *
*                        Written by: Ranger and Sumo                       *
*        Creation Date: Mar 8/98                                           *
*        Last Edited:                                                      *
***************************************************************************/

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


#define GLOWING_KEY  10700
#define GLOWING_EX   10725
#define ALTAR_EX     
#define TAPESTRY_ONE 10767
#define TAPESTRY_TWO 10763
#define BOARD_ON     10710
#define BOARD_OFF    10713
#define ALTAR_ON     10708
#define ALTAR_OFF    10712

int glowing_key_spec (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int glowing_key;
  OBJ *key;

  if(!ch) return FALSE;  
  glowing_key = real_object(GLOWING_KEY);

  if (cmd == CMD_UNLOCK) {
    one_argument(arg,buf);
    if (!*buf) return FALSE;
    if(strcmp(buf,"trapdoor"))  return FALSE;    
    if (!(key = get_obj_in_list_num (glowing_key, ch->carrying))) return FALSE;
    
    send_to_char ("*CLICK*\n\rThe key crumbles in your hands.\n\r",ch);
    REMOVE_BIT(world[room].dir_option[DOWN]->exit_info,EX_LOCKED);
    REMOVE_BIT(world[real_room(GLOWING_EX)].dir_option[UP]->exit_info,EX_LOCKED);
    act("As $n unlocks the trapdoor, the key crumbles to dust.",FALSE,ch,0,0,TO_ROOM);
    extract_obj (key);
    return TRUE;
  }
  return FALSE;
}


int board_spec (int room, CHAR *ch, int cmd, char *arg) {
  OBJ *block_on=NULL, *block_off=NULL;
  char buf[MAX_INPUT_LENGTH];
  int board_on, board_off;
  
  board_on = real_object (BOARD_ON);
  board_off = real_object (BOARD_OFF);
  
  one_argument (arg, buf);
  
  switch (cmd) {
    case CMD_MOVE:
      if(!*buf) return FALSE;
      if(!is_abbrev(buf,"board")) return FALSE;
      
      block_on = get_obj_in_list_num(board_on,world[room].contents);
      block_off = get_obj_in_list_num(board_off,world[room].contents);

      if (!block_on && !block_off) {
        send_to_char ("There seems to be nothing to do.\n\r",ch);
        return TRUE;
      }

      if(block_off) {
        send_to_char("It's already moved.\n\r",ch);
        return TRUE;
      }
      if (block_on) {
        act("$n lifts $p away from the floor, revealing a hole.", FALSE, ch, block_on, ch, TO_ROOM);
        act("You lifts $p away from the floor, revealing a hole.", FALSE, ch, block_on, ch, TO_CHAR);
        if (!block_off) {
          block_off = read_object (board_off, REAL);
          obj_to_room (block_off, room);
        }
        extract_obj (block_on);
        return TRUE;
      }
      break;
      
    case CMD_CRAWL:
      if(!*buf) return FALSE;
      if (strcmp(buf,"hole")) return FALSE;
      
      block_on = get_obj_in_list_num(board_on,world[room].contents);
      
      if (block_on) {
        send_to_char ("You don't see that here.\n\r",ch);
        return TRUE;
      }
      break;
    case CMD_DOWN:
      block_on = get_obj_in_list_num(board_on,world[room].contents);
      
      if (block_on) {
        send_to_char ("Alas, you cannot go that way...\n\r",ch);
        return TRUE;
      }
      break;
  
    case MSG_ZONE_RESET:
      block_on = get_obj_in_list_num(board_on,world[room].contents);
      block_off = get_obj_in_list_num(board_off,world[room].contents);
      if(!block_on) {
        tmp = read_object(board_on, REAL);
        obj_to_room(tmp, room);
      }
      if(block_off) {
        extract_obj(block_off);
      }
      break;
  }
  return FALSE;
}



int altar_spec_one (int room, CHAR *ch, int cmd, char *arg) {
  int glowing_key;
  OBJ *key, *tmp;
  OBJ *block_on=NULL, *block_off=NULL;
  char buf[MAX_INPUT_LENGTH];
  int altar_on, altar_off;
  
  altar_on = real_object (ALTAR_ON);
  altar_off = real_object (ALTAR_OFF);
  
  one_argument (arg, buf);
  
  switch (cmd) {
    case CMD_MOVE:
      if(!*buf) return FALSE;
      if(!is_abbrev(buf,"altar")) return FALSE;
      
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      block_off = get_obj_in_list_num(altar_off,world[room].contents);

      if (!block_on && !block_off) {
        send_to_char ("There seems to be nothing to do.\n\r",ch);
        return TRUE;
      }

      if(block_off) {
        send_to_char("It's already moved.\n\r",ch);
        return TRUE;
      }
      if (block_on) {
        act("$n moves $p against the wall, revealing a secret door.", FALSE, ch, block_on, ch, TO_ROOM);
        act("You move $p against the wall, revealing a secret door.", FALSE, ch, block_on, ch, TO_CHAR);
        if (!block_off) {
          block_off = read_object (altar_off, REAL);
          obj_to_room (block_off, room);
        }
        extract_obj (block_on);
        return TRUE;
      }
      break;
      
    case CMD_OPEN:
    case CMD_CLOSE:
    case CMD_PICK:
    case CMD_KNOCK:
      if(!*buf) return FALSE;
      if (strcmp(buf,"trapdoor")) return FALSE;
      
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      
      if (block_on) {
        send_to_char ("You don't see that here.\n\r",ch);
        return TRUE;
      }
      break;
    case CMD_NORTH:
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      
      if (block_on) {
        send_to_char ("Alas, you cannot go that way...\n\r",ch);
        return TRUE;
      }
      break;
    case CMD_UNLOCK:
      if (!*buf) return FALSE;
      if(strcmp(buf,"trapdoor"))  return FALSE;    
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      if (block_on) {
        send_to_char ("What are you trying to unlock?\n\r",ch);
        return TRUE;
      }
      glowing_key = real_object(GLOWING_KEY);
      if (!(key = get_obj_in_list_num (glowing_key, ch->carrying))) return FALSE;

      send_to_char ("*CLICK*\n\rThe grass crumbles in your hands.\n\r",ch);
      REMOVE_BIT(world[room].dir_option[DOWN]->exit_info,EX_LOCKED);
      REMOVE_BIT(world[real_room(ALTAR_EX)].dir_option[UP]->exit_info,EX_LOCKED);
      act("As $n unlocks the door, the grass crumbles to dust.",FALSE,ch,0,0,TO_ROOM);
      extract_obj (key);
      return TRUE;
      break;
    case MSG_ZONE_RESET:
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      block_off = get_obj_in_list_num(altar_off,world[room].contents);
      if(!block_on) {
        tmp = read_object(altar_on, REAL);
        obj_to_room(tmp, room);
      }
      if(block_off) {
        extract_obj(block_off);
      }
      break;
  }
  return FALSE;
}


int altar_spec_two (int room, CHAR *ch, int cmd, char *arg) {
  int grasskey;
  OBJ *key, *tmp;
  OBJ *block_on=NULL, *block_off=NULL;
  char buf[MAX_INPUT_LENGTH];
  int altar_on, altar_off;
  
  altar_on = real_object (ALTAR_TWO_ON);
  altar_off = real_object (ALTAR_TWO_OFF);
  
  one_argument (arg, buf);
  
  switch (cmd) {
    case CMD_MOVE:
      if(!*buf) return FALSE;
      if(!is_abbrev(buf,"altar")) return FALSE;
      
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      block_off = get_obj_in_list_num(altar_off,world[room].contents);

      if (!block_on && !block_off) {
        send_to_char ("There seems to be nothing to do.\n\r",ch);
        return TRUE;
      }

      if(block_off) {
        send_to_char("It's already moved.\n\r",ch);
        return TRUE;
      }
      if (block_on) {
        act("$n moves $p away from the wall, revealing a secret door.", FALSE, ch, block_on, ch, TO_ROOM);
        act("You move $p away from the wall, revealing a secret door.", FALSE, ch, block_on, ch, TO_CHAR);
        if (!block_off) {
          block_off = read_object (altar_off, REAL);
          obj_to_room (block_off, room);
        }
        extract_obj (block_on);
        return TRUE;
      }
      break;
      
    case CMD_OPEN:
    case CMD_CLOSE:
    case CMD_PICK:
    case CMD_KNOCK:
      if(!*buf) return FALSE;
      if (strcmp(buf,"door")) return FALSE;
      
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      
      if (block_on) {
        send_to_char ("You don't see that here.\n\r",ch);
        return TRUE;
      }
      break;
    case CMD_NORTH:
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      
      if (block_on) {
        send_to_char ("Alas, you cannot go that way...\n\r",ch);
        return TRUE;
      }
      break;
    case CMD_UNLOCK:
      if (!*buf) return FALSE;
      if(strcmp(buf,"door"))  return FALSE;    
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      if (block_on) {
        send_to_char ("I see no door here.\n\r",ch);
        return TRUE;
      }
      grasskey = real_object(GRASS_KEY);
      if (!(key = get_obj_in_list_num (grasskey, ch->carrying))) return FALSE;

      send_to_char ("*CLICK*\n\rThe grass crumbles in your hands.\n\r",ch);
      REMOVE_BIT(world[room].dir_option[NORTH]->exit_info,EX_LOCKED);
      REMOVE_BIT(world[real_room(GRASS_EX)].dir_option[SOUTH]->exit_info,EX_LOCKED);
      act("As $n unlocks the door, the grass crumbles to dust.",FALSE,ch,0,0,TO_ROOM);
      extract_obj (key);
      return TRUE;
      break;
    case MSG_ZONE_RESET:
      block_on = get_obj_in_list_num(altar_on,world[room].contents);
      block_off = get_obj_in_list_num(altar_off,world[room].contents);
      if(!block_on) {
        tmp = read_object(altar_on, REAL);
        obj_to_room(tmp, room);
      }
      if(block_off) {
        extract_obj(block_off);
      }
      break;
  }
  return FALSE;
}

int tapestry_spec(int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  OBJ *obj=0;

  if(cmd==MSG_ZONE_RESET) {
    if(world[real_room(TAPESTRY_TWO)].dir_option[SOUTH]->to_room_r == real_room(DIABOLIK_ACCESS)) {
      world[real_room(TAPESTRY_ONE)].dir_option[NORTH]->to_room_r = real_room(0);
      world[real_room(TAPESTRY_TWO)].dir_option[SOUTH]->to_room_r = real_room(0);
      block_on = get_obj_in_list_num(block_on,world[room].contents);
      block_off = get_obj_in_list_num(block_off,world[room].contents);
      if(!block_on) {
        tmp = read_object(altar_on, REAL);
        obj_to_room(tmp, room);
      }
      if(block_off) {
        extract_obj(block_off);
      }
    }
  }

  if (!ch) return FALSE;

  if (cmd==CMD_MOVE) {
    arg=one_argument(arg,buf);
    if (!*buf) return FALSE;
    if(!strcmp(buf,"tapestry")) {
  
      if(world[real_room(TAPESTRY_TWO)].dir_option[SOUTH]->to_room_r == real_room(TAPESTRY_ONE)) {
        act("You pull hard on the ring, but the drawbridge is already open!",TRUE,ch,0,0,TO_CHAR);
      }
      else {
        world[real_room(TAPESTRY_TWO)].dir_option[SOUTH]->to_room_r = real_room(TAPESTRY_ONE);
        send_to_room ("A cold draft wafts across your body as the tapestry is moved to the floor.\n\r",real_room(TAPESTRY_TWO));
        world[real_room(TAPESTRY_ONE)].dir_option[NORTH]->to_room_r = real_room(TAPESTRY_TWO);
        send_to_room ("A cold draft wafts across your body as the tapestry is moved to the floor.\n\r",real_room(TAPESTRY_ONE));
      }
      return TRUE;
    }
  }
  return FALSE;
}

void assign_louis(void) {
    assign_room(10708, board_spec);
    assign_room(10718, altar_spec_one);
    assign_room(10715, altar_spec_two);
    assign_room(TAPESTRY_TWO, tapestry_spec);
    assign_room(TAPESTRY_ONE, tapestry_spec);
    assign_obj(10700, glowing_key_spec);
 
