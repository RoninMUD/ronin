/* ************************************************************************
*  file: mob.magic.c , Implementation of mob spells.      Part of DIKUMUD *
************************************************************************* */

/*
$Author: ronin $
$Date: 2005/01/21 14:55:28 $
$Header: /home/ronin/cvs/ronin/mob.magic.c,v 2.2 2005/01/21 14:55:28 ronin Exp $
$Id: mob.magic.c,v 2.2 2005/01/21 14:55:28 ronin Exp $
$Name:  $
$Log: mob.magic.c,v $
Revision 2.2  2005/01/21 14:55:28  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.1  2004/06/30 19:57:28  ronin
Update of saving throw routine.

Revision 2.0.0.1  2004/02/05 16:09:46  ronin
Reinitialization of cvs archives

Revision 1.4  2003/11/02  liner
Added "ObjLog:" to all the logs in which mobs extract
objects with breath.

Revision 1.3  2002/06/18 14:32:20  ronin
Adding divide_experience before raw_kill to ensure proper quest
completion.  Addition of flag within divide_experience to force
amount to 0 if required.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "db.h"
#include "utility.h"
#include "fight.h"
#include "act.h"
#include "memory.h"
#include "enchant.h"
#include "mob.spells.h"

/* Extern structures */

extern Room *world;
extern OBJ  *object_list;
extern CHAR *character_list;
extern int CHAOSMODE;

/* Extern procedures */

void make_statue (struct char_data *ch);
void weight_change_object(OBJ *obj, int weight);
char *str_dup(char *source);
char *one_argument (char*, char*);
int dice(int number, int size);
void stop_follower(CHAR *ch);
void do_look(CHAR *ch, char *argument, int cmd);

void spell_fire_breath(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
  int hpch;
  OBJ *burn;
  char buf[MAX_INPUT_LENGTH];

  if (!IS_NPC(ch) && !IS_NPC(victim)) {
  	    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
  	        return;
  }

  act("$n breathes fire on $N.",
        FALSE, ch, 0, victim, TO_ROOM);
          act("You breathe fire on $N.", FALSE, ch, 0, victim, TO_CHAR);
            send_to_char("A wall of fire hits you, melting away your weapons and armour.\n\r",
                           victim);


  hpch = GET_HIT(ch);

  if(hpch<10) hpch=10;

  dam = number(0,hpch>>2);

  if ( saves_spell(victim, SAVING_BREATH,level) )
    dam >>= 1;

  damage(ch, victim, dam, TYPE_UNDEFINED,DAM_FIRE);

  /* And now for the damage on inventory */

  if(number(40,60)>GET_LEVEL(ch)) {
    if (!saves_spell(victim, SAVING_BREATH,level) ) {
      for(burn=victim->carrying ;
	  burn && (burn->obj_flags.type_flag!=ITEM_SCROLL) &&
	  (burn->obj_flags.type_flag!=ITEM_WAND) &&
	  (burn->obj_flags.type_flag!=ITEM_STAFF) &&
	  (burn->obj_flags.type_flag!=ITEM_NOTE) &&
	  (burn->obj_flags.type_flag!=ITEM_RECIPE) ;
	  burn=burn->next_content) {
 
        /* Shun: it is not sufficient to put the continue
           check in the conditional of the for statement
           above, it can result in the loop stopping on
           an item that is none of the checked types and
           destroying it */

        if (number(0,2)==0) continue;
      }

      if(burn) {
	act("$o goes up in flames!",0,victim,burn,0,TO_CHAR);
        sprintf(buf,"ObjLog: %s just destroyed %s worn by %s",MOB_SHORT(ch),
          OBJ_SHORT(burn), GET_NAME(victim));
	extract_obj(burn);
        log_f(buf);
      }
    }
  }
}

void spell_frost_breath(ubyte level, CHAR *ch,CHAR *victim, OBJ *obj) {
  int dam;
  int hpch;
  OBJ *frozen;
  char buf[MAX_INPUT_LENGTH];

  hpch = GET_HIT(ch);
  if(hpch<10) hpch=10;

  dam = number(0,hpch>>2);

  if ( saves_spell(victim, SAVING_BREATH,level) )
    dam >>= 1;

  if (!IS_NPC(ch) && !IS_NPC(victim)) {
  	    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
  	        return;
  }

  act("$n breathes out a cone of frost at $N.",
        FALSE, ch, 0, victim, TO_ROOM);
          act("You breathe out a cone of frost at $N.", FALSE, ch, 0, victim, TO_CHAR);
            send_to_char("A cone of frost hits you, turning your weapons and armour brittle.\n\r",
                           victim);

  damage(ch, victim, dam, TYPE_UNDEFINED,DAM_COLD);

  /* And now for the damage on inventory */

  if(number(40,60)>GET_LEVEL(ch)) {
    if (!saves_spell(victim, SAVING_BREATH,level) ) {
      for(frozen=victim->carrying ;
	  frozen && (frozen->obj_flags.type_flag!=ITEM_DRINKCON) &&
	  (frozen->obj_flags.type_flag!=ITEM_FOOD) &&
	  (frozen->obj_flags.type_flag!=ITEM_POTION) ;
	  frozen=frozen->next_content) {

        /* Shun: it is not sufficient to put the continue
           check in the conditional of the for statement
           above, it can result in the loop stopping on
           an item that is none of the checked types and
           destroying it */

        if (number(0,2)==0) continue;
      }

      if(frozen) {
	act("$o becomes brittle and shatters.",0,victim,frozen,0,TO_CHAR);
        sprintf(buf,"ObjLog: %s just destroyed %s worn by %s",MOB_SHORT(ch),
          OBJ_SHORT(frozen), GET_NAME(victim));
	extract_obj(frozen);
        log_f(buf);
      }
    }
  }
}

void spell_acid_breath(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
  int damaged;
  char buf[MAX_INPUT_LENGTH];
  OBJ *eq = 0;
  int apply_ac(CHAR *ch, int eq_pos);

  if (!IS_NPC(ch) && !IS_NPC(victim)) {
  	    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
  	        return;
  }

  act("$n spits acid on $N.",
        FALSE, ch, 0, victim, TO_ROOM);
          act("You spit out acid at $N.", FALSE, ch, 0, victim, TO_CHAR);
            send_to_char("A shower of acid hits you, burning away at your clothes and equipment!\n\r",
                           victim);


  dam = number(20,60);
  if ( saves_spell(victim, SAVING_BREATH,level) )
    dam = dam/2;

  damage(ch, victim, dam, TYPE_UNDEFINED,DAM_CHEMICAL);

  /* And now for the damage on equipment */

  if (!saves_spell(victim, SAVING_BREATH,level) && number(0,2)==0) {
    damaged = number(0, 17);
    if (victim->equipment[damaged])
      eq = victim->equipment[damaged];
    else return;
    act("$o is destroyed by the acid.",0,victim,eq,0,TO_CHAR);
    sprintf(buf,"ObjLog: %s just destroyed %s worn by %s",MOB_SHORT(ch),
     OBJ_SHORT(eq), GET_NAME(victim));
    unequip_char(victim, damaged);
    extract_obj(eq);
    log_f(buf);
    return;
  }
}

void spell_gas_breath(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
  int hpch;

  act("$n breathes out a green cloud of poisonous fumes on $N.",
        FALSE, ch, 0, victim, TO_ROOM);
          act("You breathe poisonous fumes on $N.", FALSE, ch, 0, victim, TO_CHAR);
            send_to_char("Gas envelopes you and you suddenly find it hard to breathe.\n\r",
                           victim);

  hpch = GET_HIT(ch);
  if(hpch<10) hpch=10;

  dam = number(0,hpch>>2);

  if ( saves_spell(victim, SAVING_BREATH,level) )
    dam >>= 1;

  damage(ch, victim, dam, TYPE_UNDEFINED,DAM_CHEMICAL);
}


void spell_lightning_breath(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  int dam;
  int hpch;

  if (!IS_NPC(ch) && !IS_NPC(victim)) {
  	    send_to_char("You can't cast such a powerful spell on a player.\n\r", ch);
  	        return;
  }

  act("The air crackles as $n breathes lightning on $N.",
        FALSE, ch, 0, victim, TO_ROOM);
          act("You breathe lightning bolts on $N.", FALSE, ch, 0, victim, TO_CHAR);
            send_to_char("Several lightning bolts course through your body.\n\r",
                           victim);

  hpch = GET_HIT(ch);
  if(hpch<10) hpch=10;

  dam = number(0,hpch>>2);

  if ( saves_spell(victim, SAVING_BREATH,level) )
    dam >>= 1;

  damage(ch, victim, dam, TYPE_UNDEFINED,DAM_ELECTRIC);
}

void spell_vanish(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;
  extern int top_of_world;
  int to_room;
  int location;
  bool found = FALSE;

  assert(victim);
    to_room = 3088;
  char_from_room(victim);

  act("$n points at $N and $E vanishes into thin air.", FALSE,
ch,0,victim,TO_ROOM);
  act("You point at $N and $E vanishes into thin air.", FALSE,
ch,0,victim,TO_CHAR);
   act("$n points at you and everything fades to black....",FALSE,
ch,0,victim,TO_VICT);

    af.type      = SPELL_SLEEP;
    af.duration  = 3;
    af.bitvector = AFF_SLEEP;
    af.bitvector2 = 0;
    af.location  = APPLY_NONE;
    affect_to_char(victim, &af);
    af.type      = SPELL_IMP_INVISIBLE;
    af.duration  = 3;
    af.bitvector = AFF_IMINV;
    affect_to_char(victim, &af);

    for (location = 0; location <= top_of_world; location++)
       if (world[location].number == to_room) {
     	      found = TRUE;
              break;
   }

  if ((location == top_of_world) || !found) {
       send_to_char("You are completely lost.\n\r", victim);
       return;
  }
  char_to_room(victim, location);
      act("$n slowly fades into existence.", FALSE, victim,0,0,TO_ROOM);
       GET_POS(victim) = POSITION_SLEEPING;
}

void spell_enfeeblement(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  struct affected_type_5 af;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }
    act("$n gestures towards $N and a green cloud surrounds $M.",
        FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n gestures towards you and a green cloud of gas surrounds you!",
        FALSE, ch, 0, victim, TO_VICT);
    act("You gesture towards $N and a green cloud of gas surrounds $M!",
        FALSE, ch, 0, victim, TO_CHAR);

    if (affected_by_spell(victim, SPELL_POISON)) {
     	return;
    }

     if(!saves_spell(victim, SAVING_PARA,level)) {
      af.type      = SPELL_POISON;
      af.duration  = 12;
      af.modifier  = -12;  /* Make str worse */
      af.location  = APPLY_STR;
      af.bitvector = AFF_POISON;
      af.bitvector2 = 0;
      affect_join(victim, &af, FALSE, FALSE);
     send_to_char("You feel very sick.\n\r", victim);
  }
}

void spell_finger_of_death(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {

   if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) {
     send_to_char("Behave yourself here please!\n\r", ch);
     return;
   }
   if(!saves_spell(victim, SAVING_PARA,level)) {

     if(victim->specials.fighting && victim->specials.fighting == ch) stop_fighting(victim);
     if (ch->specials.fighting) stop_fighting(ch);

     act("$n points a finger at $N and $E falls down dead!", FALSE, ch, 0, victim, TO_NOTVICT);
     act("$n points a finger at you, you die in horrible agony!.", FALSE, ch, 0, victim, TO_VICT);
     act("You point a finger at $N and $E falls down dead!", FALSE, ch, 0, victim, TO_CHAR);
     divide_experience(ch,victim,1);
     raw_kill(victim);
     return;
   }
   act("$n points a finger at $N and $E writhes in agony.",FALSE, ch, 0, victim, TO_NOTVICT);
   act("$n points a finger at you and you writhe in agony!", FALSE, ch, 0, victim, TO_VICT);
   act("You point a finger at $N and $E writhes in agony!", FALSE, ch, 0, victim, TO_CHAR);

   damage(ch, victim, number(40, 80), TYPE_UNDEFINED,DAM_NO_BLOCK);
}

void spell_primal_rage(ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) {
  CHAR *tmp_victim,*temp;
  int dam;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  act("$n becomes a blur of teeth and claws.", TRUE, ch, 0, 0, TO_ROOM);
  act("You release the primal beast within you upon your foes.",FALSE,ch,0,0,TO_CHAR);

     damage(ch, victim, number(100,200), TYPE_BITE,DAM_NO_BLOCK );
     damage(ch, victim, number(100,200), TYPE_CLAW,DAM_NO_BLOCK );
     WAIT_STATE(victim, PULSE_VIOLENCE *2);

  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people;tmp_victim;tmp_victim = temp) {
    temp = tmp_victim->next_in_room;

    if (ch != tmp_victim) {
     if (tmp_victim != victim) {
         dam = number(100, 200);
          if (saves_spell(victim, SAVING_SPELL,level)) dam >>= 1;
           damage(ch, tmp_victim, dam, TYPE_BITE,DAM_NO_BLOCK );
         dam = number(100, 200);
          if (saves_spell(victim, SAVING_SPELL,level)) dam >>= 1;
           damage(ch, tmp_victim, dam, TYPE_CLAW,DAM_NO_BLOCK );
     }
    }
  }
}

void spell_drain_mana (ubyte lvl, CHAR *ch, CHAR *vict, OBJ *obj) {
  int drain,dam;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) {
    send_to_char("Behave yourself in here please!\n",ch);
    return;
  }

  dam = dice (GET_LEVEL(ch), 15);
  drain = MAX(0,MIN(dam, GET_MANA(vict)));

  if (IS_AFFECTED(vict, AFF_SANCTUARY)) {
    drain = drain>>1;
  }

  act("$n touches $N and a stream of yellow energy leaps to $s hand.",
      FALSE,ch,0,vict,TO_NOTVICT);
  act("$n touches you and you feel your magic draining away into $s hands.",
      FALSE,ch,0,vict,TO_VICT);
  act("You touch $N and a stream of yellow energy leaps to your hand.",
      FALSE,ch,0,vict,TO_CHAR);

  GET_MANA(vict) -= drain;
  send_to_char("You feel the drained magic flowing into you.\n\r", ch);
  GET_MANA(ch) += drain>>1;
}

