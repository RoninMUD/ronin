/* *************************************************************************
 *Special Proceedures for Xalth's Revamp of the Newts? =b                 *
 *                        Written by DarkLeha
 *                        Modified by Fisher 04/11/2021
 *Small mod by Ranger                                                     *
 **************************************************************************/
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

/*Rooms */
#define TUNNEL_ROOM 2923
#define NEW_TUNNEL_ROOM 2934
#define LEADER_ROOM 2926

/*Mobs */
#define FIRE_NEWT_LEADER 2912

//Have the room close the new area off on reset.
int newt_tunnelroom(int room, CHAR *ch, int cmd, char *arg)
{
  if (cmd == MSG_ZONE_RESET)
  {

    if(world[real_room(TUNNEL_ROOM)].dir_option[NORTH]->to_room_r != -1){

      world[real_room(TUNNEL_ROOM)].dir_option[NORTH]->to_room_r = -1;
      world[real_room(NEW_TUNNEL_ROOM)].dir_option[SOUTH]->to_room_r = -1;
      send_to_room("Boulders collapse and seal the tunnel.\n\r", real_room(TUNNEL_ROOM));
      send_to_room("Boulders collapse and seal the tunnel.\n\r", real_room(NEW_TUNNEL_ROOM));
    }
  }
  return FALSE;
}

//Have the death of the newt leader, create the link to the new area.
int newt_leader(CHAR *leader, CHAR *ch, int cmd, char *arg)
{
  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(leader, TRUE) < 1) return FALSE;

  switch (cmd)
  {
    case MSG_DIE:  // On Death - Create link to new XP area.
      send_to_room("A rumbling can be heard from inside the tunnel.\n\r", real_room(LEADER_ROOM));
      send_to_room("A magical force removes boulders and a new tunnel appears.\n\r", real_room(TUNNEL_ROOM));
      send_to_room("A magical force removes boulders and a new tunnel appears.\n\r", real_room(NEW_TUNNEL_ROOM));
      world[real_room(TUNNEL_ROOM)].dir_option[NORTH]->to_room_r = real_room(NEW_TUNNEL_ROOM);
      world[real_room(NEW_TUNNEL_ROOM)].dir_option[SOUTH]->to_room_r = real_room(TUNNEL_ROOM);

      break;
  }

  return FALSE;

}

int is_shop(CHAR *mob);

int newt_spec(OBJ *obj, CHAR *ch, int cmd, char *argument)
{
  CHAR * vict;
  char arg[MAX_STRING_LENGTH];

  if (cmd != CMD_KILL) return FALSE;
  if (IS_NPC(ch)) return FALSE;

  if (obj->equipped_by != ch) return FALSE;
  if (ch->specials.fighting) return FALSE;
  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  one_argument(argument, arg);
  if (! *arg) return FALSE;
  if (!(vict = get_char_room_vis(ch, arg))) return FALSE;
  if (is_shop(vict)) return FALSE;
  if (ch == vict) return FALSE;
  if ((GET_LEVEL(ch) - GET_LEVEL(vict)) > 20) return FALSE;

  if (number(0, 20)) return FALSE;
  if (is_shop(vict)) return FALSE;

  act("$n is showered with a white holy light!", FALSE, ch, 0, 0, TO_ROOM);
  act("You are showered with a white holy light!\n\r", FALSE, ch, 0, 0, TO_CHAR);
  spell_cure_critic(GET_LEVEL(ch), ch, ch, 0);
  return FALSE;
}

void assign_newt()
{
  assign_obj(2911, newt_spec);
  assign_obj(2913, newt_spec);

#ifdef TEST_SITE
  /*Rooms */
  assign_room(TUNNEL_ROOM, newt_tunnelroom);
  /*Mobs */
  assign_mob(FIRE_NEWT_LEADER, newt_leader);
#endif

}
