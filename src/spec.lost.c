/*
** spec.lost.c  - Specials for "Lost World by Quack"
**
** Written for RoninMUD by Ranger
**
** Date:            June 16, 1996
** Last Modified:   July 28, 1996 by Ranger
**
** Using this code is not allowed without permission from originator.
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:55 $
$Header: /home/ronin/cvs/ronin/spec.lost.c,v 2.0.0.1 2004/02/05 16:10:55 ronin Exp $
$Id: spec.lost.c,v 2.0.0.1 2004/02/05 16:10:55 ronin Exp $
$Name:  $
$Log: spec.lost.c,v $
Revision 2.0.0.1  2004/02/05 16:10:55  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"

#include "utility.h"
#include "cmd.h"
#include "spec_assign.h"

#define LW_BODYGUARD     27115
#define LW_TREX          27109
#define LW_WITCH         27105
#define LW_JUJU          27108
#define LW_TRIBUNE       27101

int lw_trex(CHAR *mob,CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(!AWAKE(mob)) return FALSE;

  switch (cmd) {
    case MSG_MOBACT:
      if (!mob->specials.fighting) return FALSE;
      if (!(vict = mob->specials.fighting)) return FALSE;

      if(!number(0,4)) {
        act("$n grabs $N in $s jaws and shakes $M back and forth a few times!",
            1, mob, 0, vict, TO_NOTVICT);
        act("$n grabs you in $s jaws and shakes you back and forth a few times!",
            1, mob, 0, vict ,TO_VICT);
        act("You grab $N in your jaws and shake $M back and forth a few times!",
            1, mob, 0, vict, TO_CHAR);
        damage(mob, vict, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);
      }
      break;
    }
  return FALSE;
}

int lw_tribune(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict, *guard;
  int percent,num_guards;

  if(!AWAKE(mob)) return FALSE;

  switch (cmd) {
    case MSG_MOBACT:
      if (!mob->specials.fighting) return FALSE;
      if (!(vict = get_random_victim(mob))) return FALSE;
      if (GET_MAX_HIT(mob) > 0)
        percent = (100*GET_HIT(mob))/GET_MAX_HIT(mob);
      else return FALSE;

      if(percent>59) return FALSE; /* 49 and below is hurt down */

      /* Count number of guards in room with Tribune */
      num_guards=0;
      for(guard = world[CHAR_REAL_ROOM(mob)].people; guard; guard = guard->next_in_room) {
        if(IS_MOB(guard)) {
          if(V_MOB(guard)==LW_BODYGUARD) num_guards = num_guards+1;
        }
      }

      if(num_guards<4) {
        act("$n calls out 'GUARDS!!!! GUARDS!!!'",
            1, mob, 0, vict, TO_ROOM);
        act("You call out for a guard to assist you.",
            1, mob, 0, vict, TO_CHAR);
        guard = read_mobile(LW_BODYGUARD, VIRTUAL);
        char_to_room(guard, CHAR_REAL_ROOM(mob));
        act("$N comes running in and assists $n!",
            1, mob, 0, guard, TO_ROOM);
        set_fighting(guard,vict);
      }
      break;
    }
  return FALSE;
}

int lw_witchdoctor(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *juju;
  OBJ *i,*temp,*next_obj,*tmp;
  int num_jujus;

  if(!AWAKE(mob)) return FALSE;

  switch (cmd) {
    case MSG_TICK:
     switch (number(0,2)) {
      case 0:
      act("$n raises $s hands and calls out 'Zakhamedi, mileki suu!'"
              ,FALSE, mob, 0, 0, TO_ROOM);
      act("You raise your hands and call out to your master, Zakhamedi!"
              ,FALSE, mob, 0, 0, TO_CHAR);
	return FALSE;
	break;

      case 1:
      act("$n grins evilly and cackles softly to $mself."
              ,FALSE, mob, 0, 0, TO_ROOM);
      act("You grin evilly at the thought of another feast coming your way."
              ,FALSE, mob, 0, 0, TO_CHAR);
         return FALSE;
	break;

      case 2:
      act("$n draws a few symbols into the dirt on the ground."
              ,FALSE, mob, 0, 0, TO_ROOM);
      act("You draw the symbols of warding on the ground to protect your home."
              ,FALSE, mob, 0, 0, TO_CHAR);
         return FALSE;
	break;
      }

    case MSG_MOBACT:

      /* Count number of jujus in room */
      num_jujus=0;
      for(juju = world[CHAR_REAL_ROOM(mob)].people; juju; juju = juju->next_in_room) {
        if(IS_MOB(juju)) {
          if(V_MOB(juju)==LW_JUJU) num_jujus = num_jujus+1;
        }
      }

      if(num_jujus>6) return FALSE;

      for (i=world[CHAR_REAL_ROOM(mob)].contents;i;i=tmp) {
        tmp = i->next_content;

        /* if it is a corpse */
        if (OBJ_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3]) {
          if(number(0,4)) continue; /* 20% chance of making a zombie */

          act("$n gestures slightly and says 'Arise, my minion!'"
              ,FALSE, mob, 0, 0, TO_ROOM);
          act("You gesture slightly and say 'Arise, my minion!'"
              ,FALSE, mob, 0, 0, TO_CHAR);

          /* Empty corpse of stuff */
          for (temp=i->contains;temp;temp=next_obj) {
            next_obj = temp->next_content;
	    obj_from_obj(temp);
	    obj_to_room(temp, CHAR_REAL_ROOM(mob));
          }
          extract_obj(i); /* remove corpse */

          /* Add zombie to room */
          juju = read_mobile(LW_JUJU, VIRTUAL);
          char_to_room(juju, CHAR_REAL_ROOM(mob));
          act("$n has created a JuJu-Zombie!!",FALSE, mob, 0, 0, TO_ROOM);
        }
      }
      break;

    case CMD_TAKE:
    case CMD_GET:

      if(!ch) return FALSE;
      if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
      if (!strncmp(" all ",arg, strlen(" all "))) return FALSE;
      if (!strncmp(" all", arg, strlen(" all"))  ||
	  !strncmp(" 1.corpse", arg, strlen(" 1.corpse")) ||
	  !strncmp(" all.corpse", arg, strlen(" all.corpse")) ||
          !strncmp(" corpse", arg, strlen(" corpse"))) {
        act("$n says 'Get Away!!  That's mine!'",FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Get Away!!  That's mine!'",FALSE, mob, 0, 0, TO_CHAR);
        return TRUE;
      }
      break;
    }
  return FALSE;
}

void assign_lostworld (void) {
  assign_mob(LW_TREX       , lw_trex);
  assign_mob(LW_TRIBUNE    , lw_tribune);
  assign_mob(LW_WITCH      , lw_witchdoctor);
}


