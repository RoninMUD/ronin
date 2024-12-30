/*   spec.swordoftruth.c - Specs for Sword of Truth by Fisher

     Written by Kith for RoninMUD
     Last Modification Date: 9/23/2017

     Basic Specs for the mobs and rooms in the zone.
*/



#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "structs.h"
#include "constants.h"
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


/* Rooms */
#define GAR_ROOM            14251

/* Objects */
#define CHASE_MACE          14211

/* Mobs */
#define MICHAEL             14202
#define GATEKEEPER          14205
#define GAR_LONG_TAIL       14207
#define GAR_SHORT_TAIL      14208
#define EMMA                14210
#define CHASE               14211

/* ======================================================================== */
/* ===============================OBJECT SPECS============================= */
/* ======================================================================== */

/* ======================================================================== */
/* ================================ROOM SPECS============================== */
/* ======================================================================== */

/* 70% chance to load GAR_LONG_TAIL  */
/* 30% chance to load GAR_SHORT_TAIL */
int sot_spawn_gar(int room, CHAR *ch, int cmd, char *arg)
{
  CHAR *gar;
  int num_gars;
  static bool spawned_gar = FALSE;

  if(cmd == MSG_ZONE_RESET && spawned_gar)
  {
    spawned_gar = FALSE;
  }

  if(cmd == MSG_ENTER && !spawned_gar) {
    num_gars = 0;
    for (gar = world[real_room(GAR_ROOM)].people; gar; gar = gar->next_in_room) {
      if(IS_MOB(gar)) {
        if(V_MOB(gar)==GAR_SHORT_TAIL || V_MOB(gar)==GAR_LONG_TAIL) num_gars = num_gars + 1;
      }
    }
    if(num_gars == 0) {
      if(chance(70)) {
        gar = read_mobile(GAR_LONG_TAIL,VIRTUAL);
      } else {
        gar = read_mobile(GAR_SHORT_TAIL,VIRTUAL);
      }
      char_to_room(gar, real_room(GAR_ROOM));
      spawned_gar = TRUE;
    }
  }
  return FALSE;
}

/* ======================================================================== */
/* ===============================MOBILE SPECS============================= */
/* ======================================================================== */

int sot_gatekeeper(CHAR *gatekeeper, CHAR *ch, int cmd, char *arg) {
  bool bReturn = FALSE;

  if(cmd == MSG_MOBACT && chance(15))
  {
    do_say(gatekeeper,"I shall not let anyone pass.", CMD_SAY);
  }

    
  if(ch && cmd == CMD_NORTH)
  {
    do_say(gatekeeper,"I said, I shall not let anyone pass!", CMD_SAY);
    bReturn = TRUE;
  }
  return bReturn;
}

int sot_chase(CHAR *chase, CHAR *ch, int cmd, char *arg) {
  CHAR *vict, *next_vict;

  if(cmd == MSG_MOBACT && chase->specials.fighting && chance(60))
  {
    if(chance(50)) {
      if(chase->equipment[WIELD] && V_OBJ(chase->equipment[WIELD]) == CHASE_MACE)
      {
        //smashes random char with mace 250 through sanc
        vict = get_random_victim_fighting(chase);
        if (vict)
        {
          act("$n grabs a six bladed mace and spins around the room.",0,chase,0,0,TO_ROOM);
          act("The fine edges of six blades carve deeply into your flesh.",0,chase,0,vict,TO_VICT);
          act("$N screams as the blades of $r mace carve into $S flesh.",0,chase,0,vict,TO_NOTVICT);
          damage(chase,vict,750,TYPE_UNDEFINED,DAM_PHYSICAL);
        }
      }
    } else {
      act("$n grabs a bunch of daggers and spins about the room.",0,chase,0,0,TO_ROOM);
      for(vict = world[CHAR_REAL_ROOM(chase)].people; vict; vict = next_vict)
      {
        //all chars for 300 through sanc
        next_vict = vict->next_in_room;
        if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        act("$n throws daggers as he spins and you are hit hard!",0,chase,0,vict,TO_VICT);
        damage(chase,vict,900,TYPE_UNDEFINED,DAM_PHYSICAL);
      }
    }
  }
  return FALSE;
}

int sot_emma(CHAR *emma, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  char buf[MAX_INPUT_LENGTH];
  char *missles[7] = {"a rolling pin","a frying pan","a loaf of bread","a kitchen knife","a dinner fork","a clay jar","a flower vase"};
  int missle;

  if(cmd == MSG_MOBACT && emma->specials.fighting && chance(60))
  {
    vict = get_random_victim_fighting(emma);
    if (vict)
    {
      act("$n grabs something from the counter and throws it at $N.",0,emma,0,vict,TO_NOTVICT);
      act("$n grabs something from the counter and throws it at you.",0,emma,0,vict,TO_VICT);
      missle = number(0, NUMELEMS(missles)-1);
      sprintf(buf, "%s is stuck by %s!", GET_NAME(vict), missles[missle]);
      act(buf, FALSE, emma, NULL, vict, TO_NOTVICT);
      sprintf(buf, "You are stuck by %s!", missles[missle]);
      act(buf, FALSE, emma, NULL, vict, TO_VICT);
      damage(emma,vict,600,TYPE_UNDEFINED,DAM_PHYSICAL);
    }
  }
  return FALSE;
}


int sot_michael(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
  
  int stun_delay;

  //Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1) return FALSE;

  char *michael_speak[4] = { "This domain is mine alone!", "Don't you dare and try to tell me what to do.","Just obey my commands!","Leave this domain at once." };

  switch (cmd)
  {
    case MSG_MOBACT:

     	//Have him to chat to you.
      if (chance(35))
      {
        sprintf(buf, "%s", michael_speak[number(0, NUMELEMS(michael_speak) - 1)]);

        do_say(mob, buf, CMD_SAY);
      }

     	//if fighting - spec different attacks
      if (mob->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.

        switch (number(0, 5))
        {
          case 0:
            stun_delay = number(1, 3);
			act("$n shouts an oppressive order, causing your head to ache and your mind to wander.",0,mob,0,0,TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
				damage(mob,vict,600,TYPE_UNDEFINED,DAM_PHYSICAL);
				WAIT_STATE(vict,PULSE_VIOLENCE*stun_delay);
            }
            break;
          case 1:
            break;
          case 2:
            vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n wields his sword and advances towards the enemy.",0,mob,0,0,TO_ROOM);
			  act("A fine steel blade stabs into your chest.",0,mob,0,vict,TO_VICT);
			  act("$N screams as the blade pierces $S chest.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,1450,TYPE_UNDEFINED,DAM_PHYSICAL);
			}
			break;
		  case 3:
			break;
		  case 4:
			break;
		  case 5:
			vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n wields his sword and jabs towards the enemy.",0,mob,0,0,TO_ROOM);
			  act("A fine steel blade jabs into your chest.",0,mob,0,vict,TO_VICT);
			  act("$N screams as the blade jabs $S chest.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,850,TYPE_UNDEFINED,DAM_PHYSICAL);
			}
			break;		  
          default:
            break;
        }
      }

     	//can add an else branch here if you want them to act but not in combat.

      break;
   
  }

  return FALSE;
}

int sot_gar(CHAR *gar, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd == MSG_MOBACT && gar->specials.fighting && chance(50))
  {
    vict = get_random_victim_fighting(gar);
    if (vict)
    {
      act("$n grabs some bloodflies and throws them at $N.",0,gar,0,vict,TO_NOTVICT);
      act("$n grabs some bloodflies and throws them at you.",0,gar,0,vict,TO_VICT);
      act("$N is bitten by a bunch of small flies.",0,gar,0,vict,TO_NOTVICT);
      act("You are bitten by a bunch of small flies.",0,gar,0,vict,TO_VICT);
      if(V_MOB(gar)==GAR_LONG_TAIL) damage(gar,vict,300,TYPE_UNDEFINED,DAM_PHYSICAL);
      else if(V_MOB(gar)==GAR_SHORT_TAIL) damage(gar,vict,300,TYPE_UNDEFINED,DAM_PHYSICAL);
      //heal gar for 5%
      act("$n was healed!",0,gar,0,0,TO_ROOM);
      GET_HIT(gar)=MIN(GET_HIT(gar) + GET_MAX_HIT(gar)/20, GET_MAX_HIT(gar));
    }
  }

  if(cmd == MSG_ENTER && !gar->specials.fighting)
  {
    vict = get_random_victim(gar);
    if (vict)
    {
      act("A large blur rushes from the grass!",0,gar,0,vict,TO_ROOM);
      do_kill(gar, GET_NAME(vict),CMD_KILL);
    }
  }
  return FALSE;
}

void assign_swordoftruth(void) {

  /* Objects */
  /* Rooms */
  assign_room(GAR_ROOM,             sot_spawn_gar);
  /* Mobs */
  assign_mob(GATEKEEPER,            sot_gatekeeper);
  assign_mob(CHASE,                 sot_chase);
  assign_mob(EMMA,                  sot_emma);
  assign_mob(MICHAEL,               sot_michael);
  assign_mob(GAR_LONG_TAIL,         sot_gar);
  assign_mob(GAR_SHORT_TAIL,        sot_gar);
}
