/*spec.ossuaryoftheFallenTitan.c - Specs for Ossuary of the Fallen Titan by Geldrin
     Plagiarize from Fisher for RoninMUD
     Creation Date: 4/24/2026
     
*/
/*System Includes */
#include <string.h>
#include <stdlib.h>
#include <math.h>

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
#include "aff_ench.h"

/*Rooms */

/*Objects */
#define BROODSKULL_VNUM  ITEM(19600, 8)

/*Mobs */
#define CRYSTAL_ENCRUSTED_SCAVENGER 19600
#define TITAN_BONE_CONSTRUCT 19601
#define CRYSTALLINE_GOLEM 19602
#define CRYSTAL_BACKED_SCARAB 19603
#define CARAPACE_RECLAIMER 19604
#define APEX_DEVOURER 19605

/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */


/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */
int ossuary_mob_echo(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    int reward = 0; // Initialize reward
    
    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1){
        return FALSE;
    }
    
    switch (cmd)
    {	
        case MSG_DIE:	// on boss death reward AQP
            sprintf(buf, "%s has been slain.\n\r", GET_SHORT(mob));
            send_to_room(buf, CHAR_REAL_ROOM(mob));
            
            // Set reward based on mob vnum
            switch (MOB_VNUM(mob)) {
                case CRYSTAL_ENCRUSTED_SCAVENGER: reward = 3; break;
                case TITAN_BONE_CONSTRUCT:      reward = 3; break;
                case CRYSTALLINE_GOLEM:         reward = 3; break;
                case CRYSTAL_BACKED_SCARAB:     reward = 3; break;
                case CARAPACE_RECLAIMER:        reward = 5; break;
                case APEX_DEVOURER:             reward = 10; break;
                default:                        reward = 0; break; // Fallback for unknown mobs
            }
            
            mob_aq_reward(reward, mob);
            break;
    }
    return FALSE;
}

/*
 *************************************************************************
 Like a wand of wonder, the Broodskull will use one of its four powers randomly
 *************************************************************************
 */
#ifndef MN_Broodskull_MAX_POWER
#define MN_Broodskull_MAX_POWER   30
#endif

int
BroodskullSPEC (OBJ *Broodskull, CHAR *ch, int cmd, char *arg) {
  int set;
  int power[] = {7, 7, 5, 20};

  if (!Broodskull) return FALSE;

  if (cmd == MSG_TICK) {
    CHAR *vict;

    if (!Broodskull->equipped_by) {
      if (OBJ_SPEC(Broodskull) > 0) {
        OBJ_SPEC(Broodskull) -= 3;
      }
      if (OBJ_SPEC(Broodskull) < 0) {
        OBJ_SPEC(Broodskull) = 0;
      }
      return FALSE;
    }

    vict = Broodskull->equipped_by;

    if (OBJ_SPEC(Broodskull) < MN_Broodskull_MAX_POWER) {
      /* Charging */
      OBJ_SPEC(Broodskull) += 10;
      if (OBJ_SPEC(Broodskull) > MN_Broodskull_MAX_POWER) {
        OBJ_SPEC(Broodskull) = MN_Broodskull_MAX_POWER;
      }
    } else {
      /* Fully charged — fire */
      if (vict) {
        set = number(0, 3);

        act("$p erupts with holy energy!",
            FALSE, vict, Broodskull, 0, TO_CHAR);
        act("$n's $p erupts with holy energy!",
            FALSE, vict, Broodskull, 0, TO_ROOM);

        OBJ_SPEC(Broodskull) -= power[set];

        switch (set) {
          case 0:
            spell_holy_bless(30, vict, vict, 0);
            spell_bless(30, vict, vict, 0);
            break;
          case 1:
            spell_holy_bless(30, vict, vict, 0);
            spell_armor(30, vict, vict, 0);
            break;
          case 2:
            spell_holy_bless(30, vict, vict, 0);
            spell_vitality(30, vict, vict, 0);
            break;
          case 3:
            spell_holy_bless(30, vict, vict, 0);
            spell_holy_bless(30, vict, vict, 0);
            break;
        }
      }
    }

    return FALSE;
  }

  return FALSE;
}

// Assign Spec for the zone. Sets all other specs.
void assign_ossuaryoftheFallenTitan(void)
{
    /*Objects */
    assign_obj(BROODSKULL_VNUM, BroodskullSPEC);

    /*Rooms */
	

    /*Mobs */
   assign_mob(CRYSTAL_ENCRUSTED_SCAVENGER, ossuary_mob_echo);
   assign_mob(TITAN_BONE_CONSTRUCT, ossuary_mob_echo);
   assign_mob(CRYSTALLINE_GOLEM, ossuary_mob_echo);
   assign_mob(CRYSTAL_BACKED_SCARAB, ossuary_mob_echo); // Updated to match fixed define
   assign_mob(CARAPACE_RECLAIMER, ossuary_mob_echo);
   assign_mob(APEX_DEVOURER, ossuary_mob_echo);	
}