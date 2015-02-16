/* Specs for area Death's Playground by Xalth
**
** Written by Ranger - Feb 2001
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:25 $
$Header: /home/ronin/cvs/ronin/spec.deathplay.c,v 2.0.0.1 2004/02/05 16:10:25 ronin Exp $
$Id: spec.deathplay.c,v 2.0.0.1 2004/02/05 16:10:25 ronin Exp $
$Name:  $
$Log: spec.deathplay.c,v $
Revision 2.0.0.1  2004/02/05 16:10:25  ronin
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
#include "enchant.h"
#include "spec_assign.h"

int dp_block_e(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_es(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_esw(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_SOUTH || cmd==CMD_WEST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_n(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_ne(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_nes(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_SOUTH || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_new(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST || cmd==CMD_NORTH || cmd==CMD_EAST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_ns(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_SOUTH || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_nws(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST || cmd==CMD_SOUTH || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_s(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_se(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_w(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_we(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_WEST) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_wn(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_wne(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_WEST || cmd==CMD_NORTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_ws(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_WEST || cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dp_block_wse(int room,CHAR *ch,int cmd,char *argument) {
  if(cmd==CMD_EAST || cmd==CMD_WEST || cmd==CMD_SOUTH) {
    if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
    send_to_char("The tombstones block your way.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

void assign_deathplay(void) {
  assign_room(21111,dp_block_e);
  assign_room(21155,dp_block_e);
  assign_room(21161,dp_block_e);
  assign_room(21168,dp_block_e);
  assign_room(21117,dp_block_es);
  assign_room(21142,dp_block_es);
  assign_room(21149,dp_block_s);
  assign_room(21177,dp_block_es);
  assign_room(21118,dp_block_esw);
  assign_room(21121,dp_block_n);
  assign_room(21108,dp_block_n);
  assign_room(21115,dp_block_n);
  assign_room(21116,dp_block_n);
  assign_room(21134,dp_block_n);
  assign_room(21138,dp_block_n);
  assign_room(21135,dp_block_n);
  assign_room(21154,dp_block_n);
  assign_room(21174,dp_block_n);
  assign_room(21103,dp_block_ne);
  assign_room(21123,dp_block_ne);
  assign_room(21139,dp_block_n);
  assign_room(21159,dp_block_n);
  assign_room(21165,dp_block_ne);
  assign_room(21172,dp_block_ne);
  assign_room(21183,dp_block_ne);
  assign_room(21179,dp_block_nes);
  assign_room(21112,dp_block_new);
  assign_room(21143,dp_block_new);
  assign_room(21122,dp_block_ns);
  assign_room(21131,dp_block_ns);
  assign_room(21132,dp_block_ns);
  assign_room(21141,dp_block_ns);
  assign_room(21148,dp_block_ns);
  assign_room(21175,dp_block_ns);
  assign_room(21167,dp_block_nws);
  assign_room(21169,dp_block_nws);
  assign_room(21133,dp_block_s);
  assign_room(21137,dp_block_s);
  assign_room(21173,dp_block_ws);
  assign_room(21193,dp_block_s);
  assign_room(21196,dp_block_s);
  assign_room(21198,dp_block_s);
  assign_room(21145,dp_block_wse);
  assign_room(21151,dp_block_se);
  assign_room(21147,dp_block_w);
  assign_room(21178,dp_block_w);
  assign_room(21180,dp_block_w);
  assign_room(21104,dp_block_we);
  assign_room(21105,dp_block_we);
  assign_room(21106,dp_block_we);
  assign_room(21119,dp_block_w);
  assign_room(21124,dp_block_we);
  assign_room(21128,dp_block_we);
  assign_room(21129,dp_block_w);
  assign_room(21146,dp_block_we);
  assign_room(21152,dp_block_we);
  assign_room(21156,dp_block_we);
  assign_room(21162,dp_block_we);
  assign_room(21166,dp_block_we);
  assign_room(21187,dp_block_we);
  assign_room(21107,dp_block_wn);
  assign_room(21127,dp_block_wne);
  assign_room(21157,dp_block_wne);
  assign_room(21163,dp_block_wne);
  assign_room(21186,dp_block_wne);
  assign_room(21188,dp_block_wne);
  assign_room(21113,dp_block_ws);
  assign_room(21144,dp_block_wse);
  assign_room(21153,dp_block_ws);
  assign_room(21158,dp_block_ws);
  assign_room(21164,dp_block_ws);
  assign_room(21189,dp_block_ws);
  assign_room(21125,dp_block_wse);
  assign_room(21126,dp_block_wse);
  assign_room(21184,dp_block_wse);
  assign_room(21185,dp_block_wse);
}





