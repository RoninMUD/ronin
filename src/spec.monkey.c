/**********************************************************************/
/****                                                              ****/
/****          Specs for Stone Monkey Island                       ****/
/****                                                              ****/
/**********************************************************************/

/****************************************************************************
 *               Special procedure module for Stone Monkey Island            *
 *                                                                           *
 *                       by: Robert Upshall (Ankh)                           *
 *           special thanks: Roy Lewis (Thyas)                               *
 *                                                                           *
 ****************************************************************************/

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

int monkey_parrot(CHAR *mob, CHAR *ch, int cmd, char *arg) {

  if (cmd) return FALSE;

  if (!AWAKE(mob)) return FALSE;

  if (GET_POS(mob)!=POSITION_FIGHTING) {
	 if(number(0,2)==0){
		act("$n says 'I know the way.'",FALSE,mob,0,0,TO_ROOM);
		act("$n squawks loudly.",FALSE,mob,0,0,TO_ROOM);
	 }
  }
  return FALSE;
}

int monkey_turtle(CHAR *turt, CHAR *ch, int cmd, char *arg) {

  CHAR *victim;
  if(cmd) return FALSE;

  /* Turtle Swats Tank With Flipper*/
  victim=turt->specials.fighting;

  if (victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(turt)) && !IS_NPC(victim)) {
	if(number(0,4) == 0) {
	act("$n lifts her flipper, and swats you hard.",FALSE,turt,0,victim,TO_VICT);
	act("$n lifts her flipper, and swats $N hard.",FALSE,turt,0,victim,TO_NOTVICT);
	damage(turt,victim,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
	return FALSE;
	}
  }
  return FALSE;
}

int monkey_monkey(CHAR *monk, CHAR *ch, int cmd, char *arg)
{
  CHAR *victim,*temp;

  if(cmd) return FALSE;

  /* Monkey Rakes Your Face */
  for(victim = world[CHAR_REAL_ROOM(monk)].people; victim; victim = temp) {
     temp = victim->next_in_room; /* Added temp - Ranger June 96 */
     if ((number(0,1) == 0) && (GET_LEVEL(victim) < LEVEL_IMM) && !IS_NPC(victim) ) {
	act("$n jumps on $N's face and starts raking their eyes.",FALSE,monk,0,victim,TO_NOTVICT);
	act("$n jumps on your face and starts to claw your eyes out!",FALSE,monk,0,victim,TO_VICT);
	damage(monk,victim,70,TYPE_UNDEFINED,DAM_NO_BLOCK);
     }
  }
  return FALSE;
}

int monkey_monk(CHAR *mob, CHAR *ch, int cmd, char *arg) {

  CHAR *victim;
  if (cmd) return FALSE;
  /* Monk Warns You */
  if (!AWAKE(mob)) return FALSE;

  if (GET_POS(mob)!=POSITION_FIGHTING) {
     if (number(0,3)==0) {
        act("$n says 'My master will make quick work of you.'",FALSE,mob,0,0,TO_ROOM);
	act("$n cackles loudly.",FALSE,mob,0,0,TO_ROOM);
     }
  }

  /* Monk Kicks You */

  victim=mob->specials.fighting;
  if (victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(mob)) && !IS_NPC(victim) ) {
     if (number(0,3) == 0) {
	act("$n kicks you in the stomach.",FALSE,mob,0,victim,TO_VICT);
	act("$n kicks $N in the stomach.",FALSE,mob,0,victim,TO_NOTVICT);
	damage(mob,victim,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
	return FALSE;
     }
  }
  return FALSE;
}

int monkey_priest(CHAR *pri, CHAR *ch, int cmd, char *arg) {
  CHAR *victim,*temp;

  if (cmd) return FALSE;
  if (!AWAKE(pri)) return FALSE;
  /* Priest Taunts You */
  if (GET_POS(pri)!=POSITION_FIGHTING) {
     if(number(0,3)==0)
       act("$n says 'I may be old, but I can mess you up.'",FALSE,pri,0,0,TO_ROOM);
     return FALSE;
  }

  /* Priest Punchs Tank */

  victim = pri->specials.fighting;
  if (victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(pri)) && !IS_NPC(victim)) {
     if (number(0,4)==0) {
	act("$n opens his fist, and punches you in the nose.",FALSE,pri,0,victim,TO_VICT);
	act("$n opens his fist, and punches $N in the nose.",FALSE,pri,0,victim,TO_NOTVICT);
	damage(pri,victim,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
     return FALSE;
     }

     /* Priest Throws Dust in the Your Eyes */
     for(victim = world[CHAR_REAL_ROOM(pri)].people; victim; victim = temp) {
        temp = victim->next_in_room;  /* Added temp - Ranger June 96 */
	if ( (number(0,4) == 0) && !IS_NPC(victim) ) {
	   act("$n reaches into the pouch on his belt, and throws a strange dust in $N's eyes.",FALSE,pri,0,victim,TO_NOTVICT);
	   act("$n reaches into the pouch on his belt, and throws a strange dust in your eyes.",FALSE,pri,0,victim,TO_VICT);
	   damage(pri,victim,50,TYPE_UNDEFINED,DAM_NO_BLOCK);
	}
     }
     return FALSE;
  }
  return FALSE;
}

/* ************************* End of Stone Monkey Island *********************** */

#define MONKEY_ZONE                      1600
#define MONKEY_PARROT                   ITEM(MONKEY_ZONE,1)
#define MONKEY_TURTLE                   ITEM(MONKEY_ZONE,2)
#define MONKEY_MONKEY                   ITEM(MONKEY_ZONE,3)
#define MONKEY_MONK                     ITEM(MONKEY_ZONE,6)
#define MONKEY_PRIEST                   ITEM(MONKEY_ZONE,7)

void assign_monkey (void) {
  assign_mob(MONKEY_PARROT,     monkey_parrot);
  assign_mob(MONKEY_TURTLE,     monkey_turtle);
  assign_mob(MONKEY_MONKEY,     monkey_monkey);
  assign_mob(MONKEY_MONK,       monkey_monk);
  assign_mob(MONKEY_PRIEST,     monkey_priest);
}

