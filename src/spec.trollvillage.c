/**********************************************************************/
/****                                                              ****/
/****          Specs for Troll Village                             ****/
/****                                                              ****/
/**********************************************************************/

/****************************************************************************
*               Special procedure module for  Troll_Village
*
*
*
*                       by: Robert Upshall (Ankh)
*
*           special thanks: J Lasilla (Ervin) (for his Barovina code)
*
*                           Vishnu(for his codeing advice)
*
*                           Poobah(for modifying the code)
*
*                     	     Kafka(for modifying the code)
*
*                           Rick(for the revamp specs)
*
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
#include "act.h"

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

  if(cmd == MSG_TICK && (status%10) < 5)   /* increase the tick counter! */
    status++;

  switch (status-(status%10)) {
  case STATUS_NORMAL:                      /* Sakdul teleports himself away */
    if(GET_HIT(mob) < 1000) {
      status = STATUS_FLED;
      do_say(mob, "My cards told me not to trust you, I should have listened.",0);
      do_say(mob, "But you will never kill me!",0);
      act("$n grabs $s crystal ball and makes a strange hand gesture.",TRUE,mob,0,0,TO_ROOM);
      act("$n disappears in a thick blinding cloud of mist.",TRUE,mob,0,0,TO_ROOM);

      /* Sakdul blinds everybody in room */
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

      act("$n arrives in a thick blinding cloud of mist.",TRUE,mob,0,0,TO_ROOM);

      /* Sakdul blinds everybody in room */
      for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room)
     if(vict != mob)
       spell_blindness(GET_LEVEL(mob), mob, vict, 0);
    }
    break;

  case STATUS_FLED:                            /* Sakdul miras himself */
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
      act("$n grabs $s crysal ball and makes a strange hand gesture.",TRUE, mob, 0, 0, TO_ROOM);
      act("$n disappears in a thick blinding cloud of mist.",TRUE, mob, 0, 0, TO_ROOM);

      /* Sakdul blinds everybody in room */
      for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict->next_in_room)
     if(vict != mob)
       spell_blindness(GET_LEVEL(mob), mob, vict, 0);

      char_from_room(mob);
      char_to_room(mob,real_room(771));

      act("$n arrives in a thick blinding cloud of mist.",FALSE, mob, 0, 0, TO_ROOM);
      act("$n says, 'Ahh, it feels good to be home.'",FALSE,mob,0,0,TO_ROOM);

      /* Sakdul blinds everybody in room */
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

  /* Racti headbutts tank */
  if (herm->specials.fighting &&
      (CHAR_REAL_ROOM(herm->specials.fighting) == CHAR_REAL_ROOM(herm))) {
    if(!number(0,4)) {
      act("$n throws back $s head wallops you with a massive headbutt.",FALSE, herm, 0, herm->specials.fighting, TO_VICT);
      act("$n throws back $s head and wallops $N with a massive headbutt.",FALSE, herm, 0, herm->specials.fighting, TO_NOTVICT);
      damage(herm,herm->specials.fighting,GET_HIT(herm->specials.fighting)/5,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
    }
  }
  return FALSE;
}


int troll_lizardassassin(CHAR *liz, CHAR *ch, int cmd, char *arg) {
  CHAR *victim,*temp;

  if (cmd != MSG_MOBACT)
    return FALSE;

  /* Neyuv hisses */
  if(GET_POS(liz) == POSITION_STANDING) {
    if(!number(0,3))
      act("$n hisses loudly as $e moves in and out of the shadows.",FALSE,liz,0,0,TO_ROOM);
    return FALSE;
  }

  /* Neyuv whips tank with his tail */
  victim = liz->specials.fighting;

  if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(liz))) {
    if(!number(0,7)) {
      act("$n whips around $s long tail and smacks you against the wall!",1,liz,0,victim,TO_VICT);
      act("$n whips around $s long tail and smacks $N against the wall!",1,liz,0,victim,TO_NOTVICT);
      damage(liz,victim,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
    }

    /* Neyuv sends chars through fountain when above 10% life (NOT tank) */
    for(victim = world[CHAR_REAL_ROOM(liz)].people; victim;     victim = temp) {
      temp = victim->next_in_room;  /* Added temp - Ranger June 96 */
      if ((!number(0,9)) && (victim != liz->specials.fighting) && (IS_MORTAL(victim)) &&
		  (((GET_HIT(liz) * 100) / GET_MAX_HIT(liz)) > 10)) {
        act("$n lets out a loud scream and pushes $N into the gusher.",1,liz,0,victim,TO_NOTVICT);
        act("$n lets out a loud scream and pushes you into the gusher.",1,liz,0,victim,TO_VICT);
        act("$N shoots out through the the hole in the ceiling!",1,liz,0,victim,TO_NOTVICT);
        act("You shoot out through the hole in the ceiling!",1,liz,0,victim,TO_VICT);
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

/* Leach sucks your blood! */
for(victim = world[CHAR_REAL_ROOM(leach)].people; victim;
    victim = temp ) {
  temp = victim->next_in_room;  /* Added temp - Ranger June 96 */
  if((!number(0,2)) && (IS_MORTAL(victim))) {
    act("$n sticks onto $N's neck and starts sucking $S blood!",1,leach,0,victim,TO_NOTVICT);
    act("$n sticks to your neck and starts sucking your blood!",1,leach,0,victim,TO_VICT);
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

int troll_foreman(CHAR *juktoa, CHAR *ch, int cmd, char *arg) {
  CHAR *victim,*temp;
  int BRAIN_OK,brains;
  if (cmd)
    return FALSE;

  /* Juktoa talks */
  if (GET_POS(juktoa)!=POSITION_FIGHTING) {
    if(!number(0,25)) {
      act("$n says, 'These drafts are all wrong, I hate stupidity!'",FALSE,juktoa,0,0,TO_ROOM);
      act("$n swears loudly.",FALSE,juktoa,0,0,TO_ROOM);
    }
  }

  /* Juktoa fightings specs */
  for(victim = world[CHAR_REAL_ROOM(juktoa)].people; victim; victim = temp) {

    temp = victim->next_in_room;

    if (victim == juktoa)
       continue;
    if(IS_NPC(victim))
       continue;
    if(GET_LEVEL(victim) > 50)
       continue;

    /* set brain check vars */
    BRAIN_OK = 36;
    brains = (GET_INT(victim) + GET_WIS(victim));

     /* Juktoa randomly bonks stupid people */
     if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(juktoa)) && (brains < BRAIN_OK)) {
       if(!number(0,7)) {
         act("$n says 'Bah $N, you are stupid too!'",FALSE,juktoa,0,victim,TO_ROOM);
         act("$n bonks you on the head!",1,juktoa,0,victim,TO_VICT);
         act("$n bonks $N on the head!",1,juktoa,0,victim,TO_NOTVICT);
         damage(juktoa,victim,(BRAIN_OK - brains)*200,TYPE_UNDEFINED,DAM_NO_BLOCK);
         return FALSE;
    }
  }

     /* Juktoa randomly slaps people */
     if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(juktoa))) {
       if(!number(0,9)) {
         act("$n says 'Damn you $N, I have work to do!'",FALSE,juktoa,0,victim,TO_ROOM);
         act("$n slaps you hard!",1,juktoa,0,victim,TO_VICT);
         act("$n slaps $N hard!",1,juktoa,0,victim,TO_NOTVICT);
         damage(juktoa,victim,((GET_LEVEL(victim)/2)*number(10,30)),TYPE_UNDEFINED,DAM_NO_BLOCK);
  return FALSE;
}
     }

     /* Juktoa randomly hits you with a blueprint */
     if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(juktoa))) {
       if(!number(0,15)) {
         act("$n says 'So, you want to see my work!'",FALSE,juktoa,0,victim,TO_ROOM);
         act("$n swats you with a rolled up blueprint!",1,juktoa,0,victim,TO_VICT);
         act("$n swats $N with a rolled up blueprint!",1,juktoa,0,victim,TO_NOTVICT);
         damage(juktoa,victim,(GET_LEVEL(victim)*number(5,10)),TYPE_UNDEFINED,DAM_NO_BLOCK);
	 WAIT_STATE(victim, PULSE_VIOLENCE*2);
	 return FALSE;
	}
     }
  }

  return FALSE;
}


/* Shield Spec */
int heavy_stone_shield (OBJ *shield, CHAR *ch, int cmd, char *arg) {

  if(shield->equipped_by)
  {
    switch(cmd)
    {
      case MSG_MOBACT:

	/* char stumbles and bashes vic */
        if(!number(0,9))
        {
          if(shield->equipped_by->specials.fighting)
	  {
            act("$n's heavy shield causes $m to stumble into $N!", 0, shield->equipped_by, 0,  shield->equipped_by->specials.fighting, TO_NOTVICT);
            act("$n stumbles into you and you fall!", 0, shield->equipped_by, 0,  shield->equipped_by->specials.fighting, TO_VICT);
            act("The weight from your shield causes you to stumble into $N!", 0, shield->equipped_by, 0,  shield->equipped_by->specials.fighting, TO_CHAR);
	    act("$N stumbles and falls!", 0, shield->equipped_by, 0,  shield->equipped_by->specials.fighting, TO_ROOM);
            GET_POS(shield->equipped_by->specials.fighting) = POSITION_SITTING;
            WAIT_STATE(shield->equipped_by->specials.fighting, PULSE_VIOLENCE*2);

	    /* char useing shield falls too */
            if(!number(0,2))
            {
	        act("$n stumbles and falls!", 0, shield->equipped_by, 0,  shield->equipped_by->specials.fighting, TO_ROOM);
		act("You lose your balance and fall!", 0, shield->equipped_by, 0,  shield->equipped_by->specials.fighting, TO_CHAR);
                GET_POS(shield->equipped_by) = POSITION_SITTING;
                WAIT_STATE(shield->equipped_by, PULSE_VIOLENCE*2);
	    }
          }
        }

        break;
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

int troll_mother(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd)
    return FALSE;

  if (GET_POS(mob)!=POSITION_FIGHTING) {
    if(!number(0,25)) {
      act("$n says, 'Have you seen my bratty child?'",FALSE,mob,0,0,TO_ROOM);
      act("$n swears loudly.",FALSE,mob,0,0,TO_ROOM);
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

    act("You are too heavy for the ladder, you slip and fall hard to the ground.",1,ch,0,0,TO_CHAR);
    act("$n is too heavy for the ladder, $e falls hard to the ground below.",     1,ch,0,0,TO_ROOM);
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

    act("You start climbing the ladder and realize you are too heavy, and you fall.",1,ch,0,0,TO_CHAR);
    act("$n starts to climb the ladder, but the ladder breaks and $e falls.",1,ch,0,0,TO_ROOM);
    return TRUE;
  }
  return FALSE;
}


int troll_jump_moat(int room, CHAR *ch, int cmd,char *arg) {
  char buf[MIL];
  struct affected_type_5 af;
  int dam;

  if (!ch)
     return FALSE;

  if (cmd == CMD_JUMP) {
    one_argument(arg,buf);
    if(!*buf) return FALSE;
    if (strcmp(buf,"moat")) return FALSE;

    act("You jump into the cold water and feel hypothermia set in.",1,ch,0,0,TO_CHAR);
    act("$n jumps into the moat.",1,ch,0,0,TO_ROOM);
    char_from_room (ch);
    char_to_room (ch, real_room(7772));
    dam = GET_LEVEL(ch)/2+number(10,40);
    af.type      = SPELL_CHILL_TOUCH;
    af.duration  = 6;
    af.modifier  = -1;
    af.location  = APPLY_STR;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_join(ch, &af, TRUE, FALSE);
    do_look(ch,"",CMD_LOOK);
        act("$n dives into the cold water from the ledge above.", 2, ch, 0,0, TO_ROOM);
    damage(ch, ch, dam, SPELL_CHILL_TOUCH,DAM_COLD);
    return TRUE;
  }

  return FALSE;

}

int troll_pipe_crack_south(int room, CHAR *ch, int cmd,char *arg) {
  struct affected_type_5 af;
  char buf[MIL];

  if (!ch)
      return FALSE;

  if (cmd == CMD_CRAWL) {

    one_argument(arg,buf);
    if(!*buf) return FALSE;
    if(strcmp(buf,"crack") && strcmp(buf,"pipe"))
       return FALSE;

    act("You crawl through the crack and cut yourself on a rusty burr.",1,ch,0,0,TO_CHAR);
    act("$n crawls through the crack.",1,ch,0,0,TO_ROOM);
    char_from_room (ch);
    char_to_room (ch, real_room(784));
    af.type = SPELL_POISON;
    af.duration = GET_LEVEL(ch)*2;
    af.modifier = -3;
    af.location = APPLY_STR;
    af.bitvector = AFF_POISON;
    af.bitvector2 = 0;
    affect_join(ch, &af, FALSE, FALSE);
    send_to_char("You feel very sick.\n\r",ch);
    do_look(ch,"",CMD_LOOK);
      act("$n wiggles through the crack.", 2, ch, 0,0, TO_ROOM);
    return TRUE;
  }

  return FALSE;

}

int troll_pipe_crack_north(int room, CHAR *ch, int cmd,char *arg) {
  struct affected_type_5 af;
  char buf[MIL];

  if (!ch)
      return FALSE;

  if (cmd == CMD_CRAWL) {

    one_argument(arg,buf);
    if(!*buf) return FALSE;
    if(strcmp(buf,"crack") && strcmp(buf,"pipe"))
       return FALSE;

    act("You crawl through the crack and cut yourself on a rusty burr.",1,ch,0,0,TO_CHAR);
    act("$n crawls through the crack.",1,ch,0,0,TO_ROOM);
    char_from_room (ch);
    char_to_room (ch, real_room(753));
    af.type = SPELL_POISON;
    af.duration = GET_LEVEL(ch)*2;
    af.modifier = -3;
    af.location = APPLY_STR;
    af.bitvector = AFF_POISON;
    af.bitvector2 = 0;
    affect_join(ch, &af, FALSE, FALSE);
    send_to_char("You feel very sick.\n\r",ch);
    do_look(ch,"",CMD_LOOK);
       act("$n wiggles through the crack.", 2, ch, 0,0, TO_ROOM);
    return TRUE;
  }

  return FALSE;

}

/* ************************ End of Troll Village ************************* */

#define TROLL_ZONE                      700
#define TROLL_SAKDUL                    ITEM(TROLL_ZONE,0)
#define TROLL_RACTI                     ITEM(TROLL_ZONE,1)
#define TROLL_NEYUV                     ITEM(TROLL_ZONE,3)
#define TROLL_LEACH                     ITEM(TROLL_ZONE,4)
#define TROLL_WORKER                    ITEM(TROLL_ZONE,5)
#define TROLL_JUKTOA                    ITEM(TROLL_ZONE,6)
#define TROLL_BUM                       ITEM(TROLL_ZONE,7)
#define TROLL_CHILD                     ITEM(TROLL_ZONE,8)
#define TROLL_BRAT                      ITEM(TROLL_ZONE,9)
#define TROLL_NOAKI                     ITEM(TROLL_ZONE,13)
#define TROLL_SUNCA                     ITEM(TROLL_ZONE,14)
#define TROLL_PHIE                      ITEM(TROLL_ZONE,15)
#define TROLL_COTKA                     ITEM(TROLL_ZONE,16)
#define TROLL_BOULDER_PILE              ITEM(TROLL_ZONE,6)
#define TROLL_OLD_LADDER_DOWN           ITEM(TROLL_ZONE,14)
#define TROLL_OLD_LADDER_UP             ITEM(TROLL_ZONE,15)
#define TROLL_JUMP_MOAT                 ITEM(TROLL_ZONE,83)
#define TROLL_PIPE_CRACK_SOUTH          ITEM(TROLL_ZONE,53)
#define TROLL_PIPE_CRACK_NORTH          ITEM(TROLL_ZONE,84)
#define TROLL_HEAVY_STONE_SHIELD        ITEM(TROLL_ZONE,14)

void assign_trollvillage (void) {
  assign_mob(TROLL_SAKDUL              ,  troll_gypsy);
  assign_mob(TROLL_RACTI               ,  troll_hermit);
  assign_mob(TROLL_NEYUV               ,  troll_lizardassassin);
  assign_mob(TROLL_LEACH               ,  troll_leach);
  assign_mob(TROLL_WORKER              ,  troll_worker);
  assign_mob(TROLL_JUKTOA              ,  troll_foreman);
  assign_mob(TROLL_BUM                 ,  troll_bum);
  assign_mob(TROLL_CHILD               ,  troll_child);
  assign_mob(TROLL_BRAT                ,  troll_brat);
  assign_mob(TROLL_NOAKI               ,  troll_mother);
  assign_mob(TROLL_SUNCA               ,  troll_mother);
  assign_mob(TROLL_PHIE                ,  troll_mother);
  assign_mob(TROLL_COTKA               ,  troll_mother);

  assign_room(TROLL_BOULDER_PILE ,        troll_boulder_pile);
  assign_room(TROLL_OLD_LADDER_DOWN,      troll_old_ladder_down);
  assign_room(TROLL_OLD_LADDER_UP  ,      troll_old_ladder_up);
  assign_room(TROLL_JUMP_MOAT ,           troll_jump_moat);
  assign_room(TROLL_PIPE_CRACK_SOUTH,     troll_pipe_crack_south);
  assign_room(TROLL_PIPE_CRACK_NORTH,     troll_pipe_crack_north);

  assign_obj(TROLL_HEAVY_STONE_SHIELD,    heavy_stone_shield);
}

