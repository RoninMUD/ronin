/*   spec.swordoftruth.c - Specs for Sword of Truth

     Written by Fisher for RoninMUD
     Last Modification Date: 3/17/2016

     Basic Specs for the mobs and rooms in the zone.
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
#include "act.h"
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"
extern struct time_info_data time_info;


/* Rooms */

#define GAR_ROOM 14251

/* Mobs */
#define GATEKEEPER 14205
#define CHASE 14211
#define SHORTTAILEDGAR 14208
#define LONGTAILEDGAR 14207


/* ROOM SPECS*/

#if 0

int sot_garbloodpool(OBJ *garbloodpool, CHAR *ch, int cmd, char *arg)
{
  switch(cmd)
  {
    case CMD_LOOK:
    /*Only want to trigger if they look at the blood */
    /* When the blood is looked at, i want a fight with the gars to start.
    however, 70% of the time i want the long tailed gar to show up, and 30% of the time i want the short tailed gar.

    Part Pseudo Code is below
    */

    /* if target of look is blood */
    if(lookTarget = blood)
    {
      if(number(0,10) < 3)
        loadMob 14208 and attack random group memeber
      else
        loadMob 14207 and attack random group memeber
    }


  }
}

#endif

/* MOB SPECS*/
/* Gatekeeper prevents you from going past, even if you have the key */
int sot_gatekeeper(CHAR *gatekeeper, CHAR *ch, int cmd, char *arg) {

  if((GET_POS(gatekeeper)==POSITION_STANDING) && (cmd==MSG_MOBACT)) {
    if(number(0,7)==0) {
      do_say(gatekeeper,"I shall not let anyone pass.", CMD_SAY);
      return(FALSE);
    }
  }
  /* If not a character, just return false */
  if(!ch) return FALSE;

  /* Prevents them from going a direction */
  if(cmd==CMD_NORTH) {
    do_say(gatekeeper,"I said, I shall not let anyone pass!", CMD_SAY);
    return TRUE;
  }

  return FALSE;
}

int sot_chase(CHAR *chase, CHAR *ch,int cmd, char *arg) {
  CHAR *vict, *next_vict;
  int bReturn = FALSE;

  switch (cmd) {
    /* */
    case CMD_PUMMEL:
      act("$n quickly dodges a pummel from $N.",0,chase,0,ch,TO_NOTVICT);
      act("$n quickly dodges a pummel.",0,chase,0,ch,TO_VICT);
      do_say(chase,"Got to be quicker than that.",CMD_SAY);

      if(number(0,19)< 6) /* 25% to knock down */ {
        do_say(chase,"Get on the ground scum.",CMD_SAY);
        act("$n kicks out $N's legs and throws $m to the ground.",0,chase,0,ch,TO_NOTVICT);
        act("$n kicks out your legs and throws you to the ground.",0,chase,0,ch,TO_VICT);
        damage(chase,ch,number(50,100),TYPE_UNDEFINED,  DAM_NO_BLOCK);
        GET_POS(ch)=POSITION_SITTING;
      }

      bReturn = TRUE;
      break;

    case MSG_MOBACT:
      if (chase->specials.fighting) {
        switch (number(0,10)) {

          case 0: /* Throw an axe at a random target*/
            vict = get_random_victim_fighting(chase);
            act("$n grabs an axe from his belt and hurls it at $N.",0,chase,0,vict,TO_NOTVICT);
            act("$n grabs an axe from his belt and hurls it at you.",0,chase,0,vict,TO_VICT);
            damage(chase,vict,number(100,200),TYPE_UNDEFINED,DAM_SKILL);
            break;

          case 1:
          case 2:/* Hit Everyone With A Mace */
            act("$n grabs a six bladed mace and spins around the room.",0,chase,0,0,TO_ROOM);
            for(vict = world[CHAR_REAL_ROOM(chase)].people; vict; vict = next_vict) {
              next_vict = vict->next_in_room;
              if(!vict || IS_NPC(vict) || !IS_MORTAL(vict)) continue;
              act("The fine edges of six blades carve deeply into your felsh.",0,chase,0,vict,TO_VICT);
              damage(chase,vict,300,TYPE_UNDEFINED,DAM_SKILL);
            }
            break;

          case 3:
          default:
            break;
        }
      }
  }
  return bReturn;
}

int sot_longtailedgar(CHAR *longtailedgar, CHAR *ch,int cmd, char *arg) {
  CHAR *vict; /* , *next_vict; */

  if(cmd==MSG_MOBACT && longtailedgar->specials.fighting && chance(14)) {
    vict = get_random_victim_fighting(longtailedgar);
    act("$n grabs some bloodflies and throws them at $N.",0,longtailedgar,0,vict,TO_NOTVICT);
    act("$n grabs some bloodflies and throws them at you.",0,longtailedgar,0,vict,TO_VICT);
    damage(longtailedgar,vict,number(100,200),TYPE_UNDEFINED,DAM_SKILL);
  }
  return FALSE;
}

int sot_shorttailedgar(CHAR *shorttailedgar, CHAR *ch,int cmd, char *arg) {
  CHAR *vict, *next_vict;

  if(cmd==MSG_MOBACT && shorttailedgar->specials.fighting && chance(14)) {
    act("$n grabs handfuls of blood flies and throws them at everyone.",0,shorttailedgar,0,0,TO_NOTVICT);

    for(vict = world[CHAR_REAL_ROOM(shorttailedgar)].people; vict; vict = next_vict) {
      next_vict = vict->next_in_room;
      if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
      act("$N is bitten by a bunch of small flies.",0,shorttailedgar,0,vict,TO_NOTVICT);
      act("You are bitten by a bunch of small flies",0,shorttailedgar,0,vict,TO_VICT);
      damage(shorttailedgar,vict,number(200,300),TYPE_UNDEFINED,DAM_SKILL);
    }
  }
  return FALSE;
}

void assign_swordoftruth(void) {
  assign_mob(GATEKEEPER , sot_gatekeeper);
  assign_mob(CHASE , sot_chase);
  assign_mob(LONGTAILEDGAR , sot_longtailedgar);
  assign_mob(SHORTTAILEDGAR  , sot_shorttailedgar);

  /* assign_room(GAR_ROOM, sot_garbloodpool); */
}
