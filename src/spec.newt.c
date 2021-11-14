/* ************************************************************************
 * Special Procedures for Xalth's Revamp of the Newts                     *
 *                        Written by DarkLeha                             *
 *                        Modified by Ranger                              *
 *                        Modified by Fisher                              *
 ************************************************************************ */

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
#include "shop.h"

/* Rooms */
#define TUNNEL_ROOM      2923
#define LEADER_ROOM      2926
#define NEW_TUNNEL_ROOM  2934

/* Mobs */
#define NEWT_LEADER      2912

/* Objects */
#define NEWT_AXE         2911
#define NEWT_MACE        2913

// Have the room close the new area off on reset.
int newt_tunnel_room(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ZONE_RESET) {
    if (real_room(TUNNEL_ROOM) != NOWHERE && world[real_room(TUNNEL_ROOM)].dir_option[NORTH]) {
      world[real_room(TUNNEL_ROOM)].dir_option[NORTH]->to_room_r = NOWHERE;

      send_to_room("Boulders collapse and seal the tunnel to the north.\n\r", real_room(TUNNEL_ROOM));
    }

    if (real_room(NEW_TUNNEL_ROOM) != NOWHERE && world[real_room(NEW_TUNNEL_ROOM)].dir_option[SOUTH]) {
      world[real_room(NEW_TUNNEL_ROOM)].dir_option[SOUTH]->to_room_r = NOWHERE;

      send_to_room("Boulders collapse and seal the tunnel to the south.\n\r", real_room(NEW_TUNNEL_ROOM));
    }

    return FALSE;
  }

  return FALSE;
}

// Have the death of the newt leader, create a link to the new area.
int newt_leader(CHAR *leader, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_DIE) {
    send_to_room("A rumbling can be heard from inside the tunnel.\n\r", real_room(LEADER_ROOM));

    if (real_room(TUNNEL_ROOM) != NOWHERE && world[real_room(TUNNEL_ROOM)].dir_option[NORTH]) {
      world[real_room(TUNNEL_ROOM)].dir_option[NORTH]->to_room_r = real_room(NEW_TUNNEL_ROOM);

      send_to_room("A magical force removes some boulders and a new tunnel appears to the north.\n\r", real_room(TUNNEL_ROOM));
    }

    if (real_room(NEW_TUNNEL_ROOM) != NOWHERE && world[real_room(NEW_TUNNEL_ROOM)].dir_option[SOUTH]) {
      world[real_room(NEW_TUNNEL_ROOM)].dir_option[SOUTH]->to_room_r = real_room(TUNNEL_ROOM);

      send_to_room("A magical force removes some boulders and a new tunnel appears to the south.\n\r", real_room(NEW_TUNNEL_ROOM));
    }
  }

  return FALSE;

}

// 5% chance to cast Cure Critic on wielder when using the kill command against a valid target.
int newt_weapon_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_KILL) {
    if (!ch || IS_NPC(ch)) return FALSE;

    if (OBJ_EQUIPPED_BY(obj) != ch || ROOM_SAFE(CHAR_REAL_ROOM(ch)) || GET_OPPONENT(ch)) return FALSE;

    char buf[MIL];

    one_argument(arg, buf);

    if (!*arg) return FALSE;

    CHAR *victim = get_char_room_vis(ch, buf);

    if (!victim || victim == ch || is_shopkeeper(victim) || (GET_LEVEL(ch) - GET_LEVEL(victim)) > 20) return FALSE;

    if (chance(5)) {
      act("$n is showered with a white holy light!", FALSE, ch, 0, 0, TO_ROOM);
      act("You are showered with a white holy light!", FALSE, ch, 0, 0, TO_CHAR);

      spell_cure_critic(GET_LEVEL(ch), ch, ch, 0);
    }

    return FALSE;
  }

  return FALSE;
}

void assign_newt() {
  assign_obj(NEWT_AXE, newt_weapon_spec);
  assign_obj(NEWT_MACE, newt_weapon_spec);

#ifdef TEST_SITE
  /*Rooms */
  assign_room(TUNNEL_ROOM, newt_tunnel_room);

  /*Mobs */
  assign_mob(NEWT_LEADER, newt_leader);
#endif
}
