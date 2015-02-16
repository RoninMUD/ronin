/**********************************************************************
 *                     Specs for Welmar Sewers                        *
 **********************************************************************/

/**********************************************************************
 *              Special procedure module for Welmar Sewers            *
 *                                                                    *
 *      by: _Odie_ with thanks going out to Ranger (for the help)     *
 *                     Thyas and Ankh for the sample                  *
 **********************************************************************/

/*
$Author: ronin $
$Date: 2004/02/05 16:11:36 $
$Header: /home/ronin/cvs/ronin/spec.wsewer.c,v 2.0.0.1 2004/02/05 16:11:36 ronin Exp $
$Id: spec.wsewer.c,v 2.0.0.1 2004/02/05 16:11:36 ronin Exp $
$Name:  $
$Log: spec.wsewer.c,v $
Revision 2.0.0.1  2004/02/05 16:11:36  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:16  ronin
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
#include "act.h"
#include "reception.h"
#include "spec_assign.h"

#define WSEWER_CLOAKER         20840
#define WSEWER_JOCKO           20811
#define WSEWER_FUNGKING        20821
#define WSEWER_BOB             20853

/* ---- The cloaker lord (#20840) spec ---- */
int wsewers_cloaker(CHAR *cloak, CHAR *ch, int cmd, char *arg) {

  CHAR *victim;
  if(cmd) return FALSE;
  if(GET_POS(cloak) < POSITION_FIGHTING) return FALSE;
  if (number(0,4)) return FALSE;
  victim=get_random_victim(cloak); /* Gets a random mortal PC in the room */
  if(victim) {
    act("$n swirls his tail and smashes you into the wall!",FALSE,cloak,0,victim,TO_VICT);
    act("$n swirls his tail and smashes $N into the wall!",FALSE,cloak,0,victim,TO_NOTVICT);
    damage(cloak,victim,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
  }
  return FALSE;
}

/* ---- Jocko (#20811) spec ---- */
/* A random snore */
int wsewers_jocko(CHAR *mob, CHAR *ch, int cmd, char *arg) {

  if (cmd) return FALSE;

  if (GET_POS(mob) > POSITION_FIGHTING) {
     if (!number(0,4)){
        act("A tremendous snore comes from the direction of $n.",FALSE,mob,0,0,TO_ROOM);
     }
  }
  return FALSE;
}

/* ---- The Fungus King (#20821) specs ---- */
int wsewers_fungking(CHAR *mob, CHAR *ch, int cmd, char *arg) {

  if (cmd) return FALSE;

  if (GET_POS(mob)>POSITION_FIGHTING) {
     if(!number(0,4)){
        act("$n smacks his lip together tenderly.",FALSE,mob,0,0,TO_ROOM);
        act("$n says 'BEOOOORRRKKKK!'",FALSE,mob,0,0,TO_ROOM);
     }
  }
  return FALSE;
}

/* ---- Bob the insurance man (#20853) specs ---- */
int wsewers_bob(CHAR *bob, CHAR *ch, int cmd, char *arg) {

  int index;
  CHAR *vict;
  char *gum_flapping_bob[] = {
        "$n says 'My rates are the best in the sewer!'",
        "$n says 'DA BEST!'",
        "$n says 'Come on, be a pal'",
        "$n says 'You can't top these prices!'",
        "$n says 'For a cool million, you'd be protected for life!'"
  };

  if (cmd) return FALSE;
  if (!number(0,4)) {
    if (GET_POS(bob)>POSITION_FIGHTING) {
      index = number(0,4);
      act(gum_flapping_bob[index],FALSE,bob,0,0,TO_ROOM);
    }

    /* Bob talking in combat */
    if (GET_POS(bob) == POSITION_FIGHTING) {
       if(number(0,1))
         act("$n says 'What? You don't like my rate?!'",FALSE,bob,0,0,TO_ROOM);
       else
         act("$n says 'What's the matter? Is it my breath?'",FALSE,bob,0,0,TO_ROOM);
    }
    return FALSE;
  }

  /* Bob following / stopping following */
  if (GET_POS(bob)<=POSITION_FIGHTING) return FALSE;

  if (!number(0,2)) {
    if (!(bob->master)) {
       vict=get_random_victim(bob); /* Gets a random mortal PC in the room */
       if(vict) {
         act("$n says 'Hey! Mind if I tag along?",FALSE,bob,0,0,TO_ROOM);
         add_follower(bob,vict);
         return FALSE;
       }
    }
    else {

      send_to_char("Bob the insurance salesman says 'Fine! It's your loss pal!'",bob->master);
      stop_follower(bob);
      return FALSE;
    }
  }
  return FALSE;
}

void assign_wsewer (void) {
  assign_mob(WSEWER_CLOAKER,     wsewers_cloaker);
  assign_mob(WSEWER_JOCKO,       wsewers_jocko);
  assign_mob(WSEWER_FUNGKING,    wsewers_fungking);
  assign_mob(WSEWER_BOB,         wsewers_bob);
}


