/* spec.WOT.c - Specs for Wheel of Time
**
** Written by Sephiroth for RoninMUD
** Last Modification Date: JAN/2015
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "interpreter.h"
#include "utility.h"
#include "fight.h"
#include "reception.h"
#include "act.h"
#include "cmd.h"
#include "spec_assign.h"

/* Specs for hunters in WOT
**
** The hunter will have his exp amount increased
** by 12.000 every minute if under 2.700.000.
*/

#define HUNTER 18702

int wot_hunter(CHAR *hunter, CHAR *vict, int cmd, char *argument) {
  if (!hunter) return FALSE;

  if(cmd==MSG_TICK) {
    if(GET_EXP(hunter) < 2700000) {
      GET_EXP(hunter)+=12000;
    }
  }

  return FALSE;
}

/* Specs for merchants in WOT
**
** The merchant will have his gold amount increased
** by 10.000 every minute if under 2.000.000.
*/

#define MERCHANT 18700

int wot_merchant(CHAR *merchant, CHAR *vict, int cmd, char *argument) {
  if (!merchant) return FALSE;

  if(cmd==MSG_TICK) {
    if(GET_GOLD(merchant) < 2000000) {
      GET_GOLD(merchant) += 10000;
    }
  }

  return FALSE;
}

/* Spec for all armies in WOT
** 400 base damage single target, 10% chance
** 300 base damage room spec, 14.2% chance
** Both damages can be mitigated(to a degree) by having low armor
*/

#define ARMY_START 18722
#define ARMY_END   18733

int wot_army(CHAR *army, CHAR *vict, int cmd, char *argument) {
  int dam = 0;
  CHAR* next_vict = NULL;

  if (!army || !army->specials.fighting || (cmd != MSG_MOBACT)) return FALSE;

  if (chance(10)) {  // 10% chance

    vict = get_random_victim(army);

    if (vict) {
      act("A cavalry charge flanks you and hits you hard, making you fall to the ground.",0,army,0,vict,TO_VICT);
      act("You charge $N and make $M fall to the ground.",0,army,0,vict,TO_CHAR);
      act("A cavalry charge flanks $N and makes $M fall to the ground.",0,army,0,vict,TO_NOTVICT);

      dam = MAX(400 + GET_AC(vict), 200);  // Having less (better) armor makes the attack do less damage. Having positive armor value increases it
      damage(army,vict,dam,TYPE_UNDEFINED,DAM_PHYSICAL);

      GET_POS(vict) = POSITION_SITTING;
    }
  }

  if (chance(15)) {  // 15% chance

    act("A large volley of arrows rain down all around you.",0,army,0,0,TO_ROOM);

    for(vict = world[CHAR_REAL_ROOM(army)].people; vict; vict = next_vict) {

      next_vict = vict->next_in_room;

      if((vict != army) && IS_MORTAL(vict)) {
        dam = MAX(300 + GET_AC(vict), 100);  // Having less (better) armor makes the attack do less damage. Having positive armor value increases it
        damage(army,vict,dam,TYPE_UNDEFINED,DAM_PHYSICAL);
      }
    }
  }

  return FALSE;
}

/* Specs for Elyas in WOT
**
** Elyas will try to summon a wolf to attack random char in room every
** MOB_ACT with 50% chance of success
*/


#define ELYAS      18723
#define WOLF       18724
#define MAX_WOLVES 5

int wot_elyas(CHAR *elyas, CHAR *vict, int cmd, char *argument) {
  CHAR *wolf = NULL;
  CHAR *next_wolf = NULL;
  int num_wolves = 0;

  if (!elyas || !AWAKE(elyas)) return FALSE;

  if (cmd == MSG_MOBACT) {
    if (elyas->specials.fighting &&
        (vict = get_random_victim(elyas)) &&
        chance(50)) {
      /* 50% chance of summoning a wolf and have it ambush random char in room
      (This spec will work pretty much like for the Tribune of Pompeii
      only that wolf is going to ambush its target)
      Text for the spec :
      TO_VICT : Seemingly out of nowhere, a wolf emerges and charges you!
      TO_ROOM : Seemingly out of nowhere, a wolf emerges and charges VICT!
      TO_CHAR : As the fighters are occupied, you leave your cover and charges VICT. */

      /* count wolves */
      for (wolf = world[CHAR_REAL_ROOM(elyas)].people; wolf; wolf = next_wolf) {
        next_wolf = wolf->next_in_room;
        if (IS_MOB(wolf) && (V_MOB(wolf) == WOLF)) num_wolves++;
      }

      if (num_wolves < MAX_WOLVES) {
        act("Seemingly out of nowhere, a wolf emerges and charges $N!", TRUE, elyas, 0, vict, TO_NOTVICT);
        act("Seemingly out of nowhere, a wolf emerges and charges you!", TRUE, elyas, 0, vict, TO_VICT);
        wolf = read_mobile(WOLF, VIRTUAL);
        char_to_room(wolf, CHAR_REAL_ROOM(vict));
        set_fighting(wolf, vict);
      }
    }
  }

  return FALSE;
}


/* Assign specs to mobs/objects */
void assign_WOT() {
  //int i = 0;
  assign_mob(ELYAS, wot_elyas);
  assign_mob(HUNTER, wot_hunter);
  assign_mob(MERCHANT, wot_merchant);
  //for (i = ARMY_START; i <= ARMY_END; i++) assign_mob(i, wot_army);
}
