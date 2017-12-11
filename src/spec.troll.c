/**********************************************************************/
/****                                                              ****/
/****          Specs for Lost Troll Village                        ****/
/****                                                              ****/
/**********************************************************************/

/****************************************************************************
 *               Special procedure module for  Troll_Village                 *
 *                                                                           *
 *                       by: Robert Upshall (Ankh)                           *
 *           special thanks: J Lasilla (Ervin) (for his Barovina code)       *
 *                           Vishnu(for his codeing advice)                  *
 *                           Poobah(for modifying the code)                  *
 *                     Kafka(for modifying the code)               *
 *                                                                           *
 ****************************************************************************/

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
#include "fight.h"
#include "spec_assign.h"

#define STATUS_NORMAL  0
#define STATUS_FLED    10
#define STATUS_READY   20

int troll_gypsy(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  void do_say (CHAR*, char*, int);
  CHAR *vict;
  char whatroom;
  static int status = 0;
  
  if (cmd != MSG_MOBACT && cmd != MSG_DIE && cmd != MSG_TICK) 
    return FALSE;

  if(cmd == MSG_TICK && (status%10) < 5)   /*increase the tick counter!*/
    status++;
  
  switch (status-(status%10)) {
  case STATUS_NORMAL:                      /* Gypsy teleports himself away */
    if(GET_HIT(mob) < 1000) {   
      status = STATUS_FLED;
      do_say(mob, "My cards told me not to trust you, I should have listened.",0);
      do_say(mob, "But you will never kill me!",0);
      act("$n grabs $s crystal ball and makes a strange hand gesture.",
       TRUE,mob,0,0,TO_ROOM);
      act("$n disappears in a thick blinding cloud of mist.",
       TRUE,mob,0,0,TO_ROOM);
      
      /* Blinds everybody in room */
      
      for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room) 
     if(vict != mob)
       spell_blindness(GET_LEVEL(mob), mob, vict, 0);
      
      char_from_room(mob);
      
      whatroom = number(0,3);
      if(whatroom == 0)
     char_to_room(mob,real_room(6105));     /* Woods by Ancient Tree */
      else if (whatroom == 1)
     char_to_room(mob,real_room(8641));     /* Woods by Ni */
      else if (whatroom == 2)
     char_to_room(mob,real_room(11603));    /* Woods by Oak Forest */
      else
     char_to_room(mob,real_room(1926));     /* Woods by Zyekian */
      
      act("$n arrives in a thick blinding cloud of mist.",
       TRUE,mob,0,0,TO_ROOM);
      
      /* Blinds everybody in room */
      
      for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room) 
     if(vict != mob)
       spell_blindness(GET_LEVEL(mob), mob, vict, 0);
    }
    break;
    
  case STATUS_FLED:                            /* Gypsy miras himself */
    if(GET_HIT(mob) < 1000) {
      act("$n grabs $s crystal ball tightly and makes a strange hand gesture.",
       TRUE, mob, 0, 0, TO_ROOM);
      act("$n looks much better now.", TRUE, mob, 0, 0, TO_ROOM);
      spell_miracle (GET_LEVEL(mob), mob, mob, 0);
      status = STATUS_READY;
    }
    break;

  case STATUS_READY:
    if(GET_HIT(mob) > 10000 && (status%10) == 5) { /* 5 ticks waiting */   
      act("$n says, 'I'm sick of this place.'",FALSE,mob,0,0,TO_ROOM);
      act("$n grabs $s crysal ball and makes a strange hand gesture.",
       TRUE, mob, 0, 0, TO_ROOM);
      act("$n disappears in a thick blinding cloud of mist.",
       TRUE, mob, 0, 0, TO_ROOM);
      
      /* Blinds everybody in room */
      for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room) 
     if(vict != mob)
       spell_blindness(GET_LEVEL(mob), mob, vict, 0);
      
      char_from_room(mob);
      char_to_room(mob,real_room(771));
      
      act("$n arrives in a thick blinding cloud of mist.",
       FALSE, mob, 0, 0, TO_ROOM);
      act("$n says, 'Ahh, it feels good to be home.'",FALSE,mob,0,0,TO_ROOM);
      
      /* Blinds everybody in room */
      for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room) 
     if(vict != mob)
       spell_blindness(GET_LEVEL(mob), mob, vict, 0);

      status  =  STATUS_NORMAL;
    }
    break; 
  }
  return FALSE;
}  

int troll_hermit(CHAR *herm, CHAR *ch, int cmd, char *arg) {
  if(cmd) 
    return FALSE; 
  
  /* Hermit Headbutts Tank */
  
  if (herm->specials.fighting && 
      (CHAR_REAL_ROOM(herm->specials.fighting) == CHAR_REAL_ROOM(herm))) {
    if(!number(0,4)) {
      act("$n throws back $s head wallops you with a massive headbutt.",
       FALSE, herm, 0, herm->specials.fighting, TO_VICT);
      act("$n throws back $s head and wallops $N with a massive headbutt.",
       FALSE, herm, 0, herm->specials.fighting, TO_NOTVICT);
      damage(herm,herm->specials.fighting,GET_HIT(herm->specials.fighting)/5,
          TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
    }
  }
  return FALSE;
}


int troll_lizardman(CHAR *liz, CHAR *ch, int cmd, char *arg) {
  CHAR *victim,*temp;
  
  if (cmd) 
    return FALSE;
  
  /* Lizardman Hisses */
  
  if(GET_POS(liz) == POSITION_STANDING) {
    if(!number(0,3))
      act("$n hisses loudly as $e moves in and out of the shadows.",
       FALSE,liz,0,0,TO_ROOM);
    return FALSE;
  }
  
  /* Lizardman Whips Tank With his Tail */
  
  victim = liz->specials.fighting;

  if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(liz))) {
    if(!number(0,7)) {
      act("$n whips around $s long tail and smacks you against the wall!",1,liz,0,victim,TO_VICT);
      act("$n whips around $s long tail and smacks $N against the wall!",1,liz,0,victim,TO_NOTVICT);
      damage(liz,victim,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
    }
    
    /* Lizardman Sends Chars Through Fountain (NOT Clerics or Tank) */
    
    for(victim = world[CHAR_REAL_ROOM(liz)].people; victim;     victim = temp) {
      temp = victim->next_in_room;  /* Added temp - Ranger June 96 */
      if ((!number(0,9)) && (victim != liz->specials.fighting) &&
          (GET_CLASS(victim) != CLASS_CLERIC) && (IS_MORTAL(victim))) {
        act("$n lets out a loud scream and pushes $N into the gusher.",
            1,liz,0,victim,TO_NOTVICT);
        act("$n lets out a loud screem and pushes you into the gusher.",
            1,liz,0,victim,TO_VICT);
        act("$N shoots out through the the hole in the ceiling!",
            1,liz,0,victim,TO_NOTVICT);
        act("You shoot out through the hole in the ceiling!",
            1,liz,0,victim,TO_VICT);
        char_from_room(victim);
        char_to_room(victim,real_room(729));
        act("$n shoots up through the fountain.",FALSE,victim,0,0,TO_NOTVICT);
        act("You shoot up through the fountain and find yourself in the village center.",1,victim,0,victim,TO_VICT); 
        act("You take in a mouthfull of water and start choking.",1,liz,0,victim,TO_VICT);
        damage(victim,victim,150,TYPE_UNDEFINED,DAM_NO_BLOCK);
      }
    }
  }
  return FALSE;
}

int troll_leach(CHAR *leach, CHAR *ch, int cmd, char *arg) {
  CHAR *victim,*temp;     
  
  if(cmd) 
    return FALSE; 
  
  /* Leach Sucks Your Blood! */
  
  for(victim = world[CHAR_REAL_ROOM(leach)].people; victim; 
      victim = temp ) {
    temp = victim->next_in_room;  /* Added temp - Ranger June 96 */
    if((!number(0,2)) && (IS_MORTAL(victim))) {
      act("$n sticks onto $N's neck and starts sucking $S blood!",
       1,leach,0,victim,TO_NOTVICT);
      act("$n sticks to your neck and starts sucking your blood!",
       1,leach,0,victim,TO_VICT);
      drain_mana_hit_mv(leach, victim, 0, number(50,100), 0,FALSE, TRUE, FALSE);
      break;
    }
  }
  return FALSE;
}


int troll_worker(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd) 
    return FALSE;
  
  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(!number(0,25)){
      act("$n says, 'Can't talk, have to work.'",FALSE,mob,0,0,TO_ROOM);
      act("$n sighs loudly.",FALSE,mob,0,0,TO_ROOM);
    }
  }
  return FALSE;
}


int troll_mother(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd) 
    return FALSE;
  
  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(!number(0,25)) {
      act("$n says, 'Have you seen my braty child?'",FALSE,mob,0,0,TO_ROOM);
      act("$n swears loudly.",FALSE,mob,0,0,TO_ROOM);
    }
  }
  return FALSE;
}


int troll_bum(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd) 
    return FALSE;
  
  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(!number(0,25)) {
      act("$n says, 'Can you spare some gold, times are tough.'",
       FALSE,mob,0,0,TO_ROOM);
      act("$n begs you for some gold.",FALSE,mob,0,0,TO_ROOM);
    }
  }
  return FALSE;
}


int troll_child(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd) 
    return FALSE;
  
  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(!number(0,25)) {
      act("$n says, 'My daddy works hard.'",FALSE,mob,0,0,TO_ROOM);
      act("$n smiles happily.",FALSE,mob,0,0,TO_ROOM);
    }
  }
  return FALSE;
}


int troll_brat(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd) 
    return FALSE;
  
  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(!number(0,25)) {
      act("$n says 'Hey, what are you looking at?'",FALSE,mob,0,0,TO_ROOM);
      act("$n kicks you in the shins.",FALSE,mob,0,0,TO_ROOM);
    }
  }
  return FALSE;
}

int troll_boulder_pile(int room, CHAR *ch, int cmd,char *arg) {
  if (!ch)
    return FALSE;
  
  if (cmd == CMD_UP) {
    if ((IS_AFFECTED(ch,AFF_FLY)))
      return FALSE;   
    
    act("You try to reach the exit above, but it is just too high and you fall.",1,ch,0,0,TO_CHAR);
    act("$n jumps up, trying to reach the exit above, but $e just can't reach.",1,ch,0,0,TO_ROOM);
    return TRUE;
  }
  return FALSE;
}

int troll_old_ladder_down(int room, CHAR *ch, int cmd,char *arg) {
  if (!ch)
      return FALSE;
  
  if (cmd == CMD_DOWN) {
    if ((IS_AFFECTED(ch,AFF_FLY)))
      return FALSE;   
    
    act("You are to heavy for the ladder, you slip and fall hard to the ground.",1,ch,0,0,TO_CHAR);
    act("$n is too heavy for the ladder, $n falls hard to the ground below.",
     1,ch,0,0,TO_ROOM);
    char_from_room (ch);
    char_to_room (ch, real_room(715)); 
    act("$n falls to the ground from high above, and lands with a heavy thump.",1,ch,0,0,TO_ROOM); 
    damage(ch, ch, (GET_HIT(ch)/2), TYPE_UNDEFINED,DAM_NO_BLOCK);  
    return TRUE;
  }
  return FALSE;
}

int troll_old_ladder_up(int room, CHAR *ch, int cmd,char *arg) {
  if (!ch)
    return FALSE;
   
  if (cmd == CMD_UP) {
    if ((IS_AFFECTED(ch,AFF_FLY)))
      return FALSE;   
    
    act("You start climbing the ladder and realize you are to heavy, and you fall.",1,ch,0,0,TO_CHAR);
    act("$n starts to climb the ladder, but the ladder breaks and $e falls.",1,ch,0,0,TO_ROOM);
    return TRUE;
  }
  return FALSE;
}


/* ********************** End of Lost Troll Village *********************** */

#define TROLL_ZONE                      700
#define TROLL_GYPSY                     ITEM(TROLL_ZONE,0)
#define TROLL_HERMIT                    ITEM(TROLL_ZONE,1)
#define TROLL_LIZARDMAN                 ITEM(TROLL_ZONE,3)
#define TROLL_LEACH                     ITEM(TROLL_ZONE,4)
#define TROLL_WORKER                    ITEM(TROLL_ZONE,5)
#define TROLL_MOTHER                    ITEM(TROLL_ZONE,6)
#define TROLL_BUM                       ITEM(TROLL_ZONE,7)
#define TROLL_CHILD                     ITEM(TROLL_ZONE,8)
#define TROLL_BRAT                      ITEM(TROLL_ZONE,9)
#define TROLL_BOULDER_PILE              ITEM(TROLL_ZONE,6)
#define TROLL_OLD_LADDER_DOWN           ITEM(TROLL_ZONE,14)     
#define TROLL_OLD_LADDER_UP             ITEM(TROLL_ZONE,15)

void assign_troll (void) {
  assign_mob(TROLL_GYPSY               ,   troll_gypsy);
  assign_mob(TROLL_HERMIT              ,   troll_hermit);
  assign_mob(TROLL_LIZARDMAN           ,  troll_lizardman);
  assign_mob(TROLL_LEACH               ,  troll_leach);
  assign_mob(TROLL_WORKER              ,  troll_worker);  
  assign_mob(TROLL_MOTHER              ,  troll_mother);
  assign_mob(TROLL_BUM                 ,  troll_bum);
  assign_mob(TROLL_CHILD               ,  troll_child);
  assign_mob(TROLL_BRAT                ,  troll_brat);
  
  assign_room(TROLL_BOULDER_PILE ,        troll_boulder_pile);
  assign_room(TROLL_OLD_LADDER_DOWN,      troll_old_ladder_down);
  assign_room(TROLL_OLD_LADDER_UP  ,      troll_old_ladder_up);
}
