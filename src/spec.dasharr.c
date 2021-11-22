/*
///   Special Procedure Module                   Orig. Date 11-09-96
///
///   spec.prison.c --- Specials for the Prison
///
///   Written by Odie of RoninMUD - whelan@engr.mun.ca
///
///   Last Update: May 1, 1998 - Ranger (Spirit Specs)
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
#include "cmd.h"
#include "spec_assign.h"
#include "interpreter.h"
#include "act.h"

/***************
Mob specs
****************/

/* mobs 4429 and 4448 */
int prison_guards(CHAR *mob, CHAR *ch, int cmd, char *arg) {

  if(cmd!=MSG_TICK) return FALSE;
  if(mob->specials.fighting) return FALSE;
  if(number(0,5)) return FALSE;
  switch(number(1,20)) {
    case 1:
      spell_protection_from_good(GET_LEVEL(mob), mob, mob, 0);
      break;
    case 5:
      spell_endure(GET_LEVEL(mob), mob, mob, 0);
      break;
    case 9:
      spell_protection_from_evil(GET_LEVEL(mob), mob, mob, 0);
      break;
    case 13:
      spell_armor(GET_LEVEL(mob), mob, mob, 0);
      break;
    case 20:
      spell_invisibility(GET_LEVEL(mob), mob, mob, 0);
      break;
  }
  return FALSE;
}

/*
Mob#4472 - Voldra
  - 150 hp spec ... everyone fighting
  - vamp touch touch the tank
*/
int prison_voldra(CHAR *voldra, CHAR *ch, int cmd, char *arg) {

  CHAR *vict=0, *next_vict;

  if(cmd) return FALSE;
  if(!voldra->specials.fighting) return FALSE;
  if(!number(0,4)) {

        act("$n makes a strange magical gesture ...",
                1, voldra, 0, 0, TO_ROOM);
        act("You make a strange magical gesture ...",
                1, voldra, 0, 0, TO_CHAR);

        for(vict = world[voldra->in_room_r].people; vict;vict = next_vict) {
          next_vict = vict->next_in_room;
          if(!IS_NPC(vict) && IS_MORTAL(vict)) {
                act("Red and blue flame streaks from his crystal ball!",
                1, voldra, 0, vict, TO_VICT);
                damage(voldra, vict, 150, TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
        }
        return FALSE;
  }

  if (!number(0,4)) {
        vict=voldra->specials.fighting;
        spell_vampiric_touch(GET_LEVEL(voldra), voldra, vict, 0);
        return FALSE;
  }
  return FALSE;
}

/*
Mob#4469 Animated skeleton
  - Hit tank only .. 300hp spec
  - random ... 300hp spec
*/
int prison_skeleton(CHAR *skel, CHAR *ch, int cmd, char *arg) {

  CHAR *vict;

  if(cmd) return FALSE;
  if(!skel->specials.fighting) return FALSE;

  if(!number(0,4)) {
        vict=skel->specials.fighting;
        act("Pieces of bone fly from $n and strikes $N squarely in the chest.",
                1, skel, 0, vict, TO_NOTVICT);
        act("Pieces of bone fly from you and strikes $N squarely in the chest.",
                1, skel, 0, vict, TO_CHAR);
        act("Pieces of bone fly from $n and strikes you squarely in the chest.",
                1, skel, 0, vict, TO_VICT);
        damage(skel, vict, 300, TYPE_UNDEFINED,DAM_NO_BLOCK);
        return FALSE;
  }

  if (!number(0,4)) {

        vict=get_random_victim(skel);
        if(!vict) return FALSE;
        act("Glowing eyes gaze into the soul of $N, making $M scream in horror.",
                1, skel, 0, vict, TO_NOTVICT);
        act("Your glowing eyes gaze into the soul of $N, making $M scream in horror.",
                1, skel, 0, vict, TO_CHAR);
        act("Glowing eyes gaze into your soul, making you scream in horror.",
                1, skel, 0, vict, TO_VICT);
        damage(skel, vict, 300, TYPE_UNDEFINED,DAM_NO_BLOCK);
        return FALSE;
  }
  return FALSE;
}

/*
Mob#4486 - Lady Justice
  - forces everyone to unwield
  - hit everyone for 200 hp
*/

int prison_justice(CHAR *justice, CHAR *ch, int cmd, char *arg) {

  CHAR *vict=0, *next_vict;

  if(cmd) return FALSE;
  if(!justice->specials.fighting) return FALSE;


  if(!number(0,4)) {
        act("$n sighs and says, 'In our wicked tormented souls lies great goodness.\n",
                1, justice, 0, 0, TO_ROOM);
        act("You sigh and say, 'In our wicked tormented souls lies great goodness.\n",
                1, justice, 0, 0, TO_CHAR);
        act("The chamber glows bright blue suddenly, blinding you for a split second.",
                1, justice, 0, 0, TO_ROOM);

        for(vict = world[justice->in_room_r].people; vict;vict = next_vict) {
          next_vict = vict->next_in_room;
          if(vict != justice && !IS_NPC(vict) && IS_MORTAL(vict))
          damage(justice, vict, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
        }

        return FALSE;
  }

  if(!number(0,3)) {
        act("$n waves $m arms in a circular pattern.",
                  1, justice, 0, 0, TO_ROOM);
        act("You wave your arms in a circular pattern.",
                  1, justice, 0, 0, TO_CHAR);
        for(vict = world[justice->in_room_r].people; vict;vict = next_vict) {
          next_vict = vict->next_in_room;
          if(vict != justice && EQ(vict, WIELD) && IS_MORTAL(vict)) {
                act("You feel at ease as you sheath you weapon.",
                  1, justice, 0, vict, TO_VICT);
               obj_to_char(unequip_char(vict, WIELD), vict);

          }
        }

        return FALSE;
  }


  if(!number(0,5)) {
    if( (vict = get_random_victim(justice)) ) {
     act("An Etheral Axe shimmers magically in $n's hand.\n\r",1,justice,0,vict,TO_ROOM);
     act("An Etheral Axe shimmers magically in you hand.",1,justice,0,vict,TO_CHAR);
     act("$n says, 'Only in eternal death can one find absolute peace.",1,justice,0,vict,TO_ROOM);
     act("You say, 'only in eternal death can one find absolute peace.",1,justice,0,vict,TO_CHAR);
     act("$n viciously cleaves $N in half with a mighty stroke of her Axe!\n\rBlood flies everywhere!",1,justice,0,vict,TO_NOTVICT);
     act("With a mighty stroke, You swing your Axe and cleave $N in half!\n\rBlood flies everywhere!",1,justice,0,vict,TO_CHAR);
     act("$n's Axe strikes into the side of your neck!\n\rChunks of flesh fall onto ground, blood squirting out like a garden hose.",1,justice,0,vict,TO_VICT);
     act("\n\r$n's Axe shimmers dimly and disappears from her hands.",1,justice,0,vict,TO_ROOM);
     act("Your Axe shimmers and disappears from your hands.",1,justice,0,vict,TO_CHAR);
     damage(justice,vict,750,TYPE_UNDEFINED,DAM_NO_BLOCK);
     return FALSE;
    }
  }
  return FALSE;
}

/*
Mob#4479 - Warden
*/
int prison_warden(CHAR *warden, CHAR *ch, int cmd, char *arg) {

  bool is_stake_used=FALSE; /* Set to FALSE at the start */
  OBJ *object;
  CHAR *vic;
  char buf[MAX_INPUT_LENGTH];

  if (warden->points.armor > -200) is_stake_used = TRUE;

  if((GET_POS(warden)==POSITION_STANDING) && (cmd==MSG_ENTER)) {
    if(!ch) return FALSE;
    act("\n\r$n rises from behind his desk and says, 'I've been expecting you'",1,warden,0,ch,TO_VICT);
    return FALSE;
  }

/* stake through the heart */
/* Need to put this whole thing within the USE cmd - Ranger */
  if(cmd==CMD_USE) {
    if(!ch) return FALSE; /* Two more little checks - Ranger */
    if(IS_NPC(ch)) return FALSE;
    if (!ch->equipment[WIELD]) return FALSE;
    if (ch->equipment[WIELD]->item_number_v != 4463) return FALSE;

    object = ch->equipment[WIELD];
    one_argument(arg, buf);
    if(!*buf) return FALSE;  /* Just a little extra check - Ranger */

    if (is_stake_used) {
      if(!isname(buf, OBJ_NAME(object))) return FALSE;
      act("$n screams 'You fool! Do you think that will work on me a second time!?'",
           1, warden, 0, ch, TO_ROOM);
      act("You scream 'You fool! Do you think that will work on me a second time!?'",
           1, warden, 0, ch, TO_CHAR);
      act("You smash $N into the wall!",1, warden, 0, ch, TO_CHAR);
      act("$n smashes $N into the wall!",1, warden, 0, ch, TO_NOTVICT);
      act("$n smashes you into the wall!",1, warden, 0, ch, TO_VICT);
      damage(warden,ch,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return TRUE; /* TRUE cause the cmd was USE and it passed all checks */
    }
    else {
      warden->points.armor = -50;
/*      if (IS_SET(warden->specials.affected_by,AFF_DODGE))
                REMOVE_BIT(warden->specials.affected_by,AFF_DODGE);*/
      GET_HIT (warden) -=3000;
      object = unequip_char (ch, WIELD);
      extract_obj(object);
      act("$n screams in agony as you plunge the wooden stake into his heart.",
                  1, warden, 0, ch, TO_VICT);
      act("You scream in agony as $N plunges the wooden stake into your heart.",
                  1, warden, 0, ch, TO_CHAR);
      act("The Wooden Stake Vanishes from your hands magically.",
                  1, warden, 0, ch, TO_VICT);
      act("$n screams in agony as $N plunges the wooden stake into $s heart.",
                  1, warden, 0, ch, TO_NOTVICT);
      damage(warden,ch,0,TYPE_UNDEFINED,DAM_NO_BLOCK); /* To start a fight if one isn't started */
      return TRUE; /* Again a return TRUE */
    }
  }

  if(cmd==MSG_DIE) {
    if (generic_find("coffin", FIND_OBJ_ROOM, warden, &vic, &object)) {
      if(object->item_number_v!=4476) return FALSE;
      if(IS_SET(object->obj_flags.value[1],CONT_PICKPROOF)) {
        REMOVE_BIT(object->obj_flags.value[1],CONT_PICKPROOF);
        REMOVE_BIT(object->obj_flags.value[1],CONT_CLOSED);
        REMOVE_BIT(object->obj_flags.value[1],CONT_LOCKED);
        act("\nThe coffin suddenly opens!",1,warden,0,0,TO_ROOM);
        return FALSE;
      }
    }
  }

  if (cmd) return FALSE;

  if(!is_stake_used && !number(0,1) && (GET_HIT(warden)<GET_MAX_HIT(warden))) {
    act("You make the sign of the inverted cross. Your life is restored!",1,warden,0,0,TO_CHAR);
    act("$n makes the sign of the inverted cross!  $s life has been restored!",1,warden,0,0,TO_ROOM);
    GET_HIT(warden)=GET_MAX_HIT(warden);
  }

  if (!warden->specials.fighting) return FALSE; /* Moved this under the USE part - Ranger */

  vic = get_random_victim(warden); /* Moved above the if */
  if(!vic) return FALSE;

  if (!number(0,3)) {
    act("$n bites $N on the neck!",1, warden, 0, vic, TO_NOTVICT);
    act("$n bites you on the neck!",1, warden, 0, vic, TO_VICT);
    damage(warden,vic,250,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if (!number(0,3)) {
    act("You transfix $N with your horrendous gaze.",1,warden,0,vic,TO_CHAR);
    act("$n transfixes $N with his horrendous gaze.",1,warden,0,vic,TO_NOTVICT);
    act("You become transfixed by $n's gaze.",1,warden,0,vic,TO_VICT);
    WAIT_STATE(vic, 3*PULSE_VIOLENCE);
    GET_POS(vic)= POSITION_STUNNED;
    return FALSE;
  }

  if (!number(0,4)) {
    act("$n raises his hands in silent gesture to HELL.",1,warden,0,vic,TO_ROOM);
    act("You raise your hands in silent gesture to HELL.",1,warden,0,vic,TO_CHAR);
    act("$n shoves his hand into $N's chest!!",1,warden,0,vic,TO_NOTVICT);
    act("You shove your hand into $N's chest!!",1,warden,0,vic,TO_CHAR);
    act("A surging hand digs deep into your HEART!!",1,warden,0,vic,TO_VICT);
    act("$n throws his head back and cackles with insane glee!",1,warden,0,vic,TO_ROOM);
    act("You throw you head back and cackle with insane glee!",1,warden,0,vic,TO_CHAR);
    damage(warden,vic,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  return FALSE;
}


/****************************
Object Specs
*****************************/

/*
Obj#4472 - Crystal Ball
  - Spell effect blindness when looked at
*/
int prison_crystal_ball (OBJ *cb, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_STRING_LENGTH];
  struct affected_type_5 af;

  if(!ch) return FALSE;    /* Two checks */
  if(IS_NPC(ch)) return FALSE;

  one_argument(arg, buf);
  if(buf[0] == '\0') return FALSE; /* check for look on cb added */

  switch (cmd) {
    case CMD_LOOK:
    case CMD_EXAMINE:
      if(!isname(buf, OBJ_NAME(cb))) return FALSE;
      if(affected_by_spell(ch, SPELL_BLINDNESS)) return FALSE;
      act("You are stunned by a blinding light!", 1, ch, 0, 0, TO_CHAR);
      act("$n seems to be blinded!", TRUE, ch, 0, 0, TO_ROOM);

      af.type      = SPELL_BLINDNESS;
      af.location  = APPLY_HITROLL;
      af.modifier  = -4;  /* Make hitroll worse */
      af.duration  = 2;
      af.bitvector = AFF_BLIND;
      af.bitvector2 = 0;
      affect_to_char(ch, &af);
      af.location = APPLY_ARMOR;
      af.modifier = +40; /* Make AC Worse! */
      affect_to_char(ch, &af);
      return TRUE;
      break;
    default:
      return FALSE;
  }
}

/*
Obj#4478 - Noose
  - 40 hps every random hours when worn
  Damage was lowered cause item will spec an average of
every 12 ticks. - Ranger
*/


int prison_noose(OBJ *noose, CHAR *ch, int cmd, char *arg) {

  CHAR *vict;
  char buf[MAX_STRING_LENGTH];

  if(!noose->equipped_by) return FALSE;
  vict=noose->equipped_by;

  one_argument(arg, buf);
  if(buf[0] == '\0') return FALSE;

  if (cmd==CMD_REMOVE && !number(0,3) )
  {
    if(!ch) return FALSE;
    if(ch!=vict) return FALSE;
    one_argument(arg, buf);
    if(!*buf) return FALSE;
    if(!isname(buf,OBJ_NAME(noose))) return FALSE;
    act("Your cheeks turn blue as the noose grips firmly around your neck!",1, ch, 0, 0, TO_CHAR);
    act("$n writhes in agony as $s noose strangles $m!",1, ch, 0, 0, TO_ROOM);
    damage(ch, ch, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
   }

  if (cmd != MSG_TICK) return FALSE;

  if (OBJ_SPEC(noose)==time_info.hours) {
    act("Your cheeks turn blue as the noose grips firmly around your neck!",1, vict, 0, 0, TO_CHAR);
    act("$n writhes in agony as $s noose strangles $m!",1, vict, 0, 0, TO_ROOM);
    OBJ_SPEC(noose)=number(0,23);
    damage(vict, vict, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);
  }
  return FALSE;
}

/*
Obj#4444 - Toilet
*/

int prison_toilet (OBJ *toilet, CHAR *ch, int cmd, char *arg) {
  CHAR *vict=0,*next_vict;
  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;

  if(world[CHAR_REAL_ROOM(ch)].number!= 4444) return FALSE;

  if ((cmd == CMD_OPEN) && !(strcmp(arg, " seat")) ) {
        act("As you lift the seat, the whole floor slides open!",
                FALSE,ch,0,0,TO_CHAR);
        act("As $n lifts the seat, the whole floor slides open and you fall!",
                FALSE,ch,0,0,TO_ROOM);

        for(vict = world[CHAR_REAL_ROOM(ch)].people; vict;vict = next_vict) {
          next_vict = vict->next_in_room;
          char_from_room(vict);
          char_to_room(vict, real_room(4485));
          do_look(vict,"",0);
          act("$n appears out of nowhere.",0,vict,0,0,TO_ROOM);
        }
        return TRUE;
  }
  return FALSE;
}

/*
Object#4476 - The Warden's Coffin
  - IMPORTANT that it be set as pickproof originally
  - Changed to CLOSEABLE when warden dies.
*/
int prison_coffin (OBJ *coffin, CHAR *ch, int cmd, char *arg) {

  if(!ch) return FALSE;
  if ((cmd == CMD_OPEN) && IS_SET(coffin->obj_flags.value[1],CONT_PICKPROOF) ) {
    act("A magical seal prevents opening of the coffin while the warden lives.",
                FALSE,ch,0,0,TO_CHAR);
    return TRUE;
  }
  return FALSE;

}
/*********************
Room Specs
**********************/

/*
Room#4475-4478
take away movement
*/
int prison_halls(int room, CHAR *ch, int cmd, char *arg) {

  if (!ch) return FALSE;

  switch (cmd) {
    case MSG_LEAVE:
      if (GET_POS(ch) == POSITION_FLYING || GET_POS(ch)==POSITION_RIDING) return FALSE;
      if (IS_NPC(ch)) return FALSE;
      send_to_char("You feel burdened as your feet shuffle on the ground.\n\r", ch);
      ch->points.move = MAX(0,ch->points.move-29);
      return FALSE;
      break;
    default:
      return FALSE;
  }
}

int prison_key(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  bool carry=FALSE,wear=FALSE;
  char arg[MAX_INPUT_LENGTH];
  int dam;

  if(cmd!=CMD_LOCK) return FALSE;
  if(!ch) return FALSE;

  if (obj->equipped_by==ch) {
    wear=TRUE;
  }
  else if (obj->carried_by==ch) {
    carry=TRUE;
  }
  else return FALSE;

  one_argument(argument,arg);
  if(!*arg) return FALSE;
  if(strcmp(arg,"gate")) return FALSE;
  if(CHAR_VIRTUAL_ROOM(ch)!=4410 && CHAR_VIRTUAL_ROOM(ch)!=4411)  return FALSE;
  if(IS_NPC(ch) && V_MOB(ch)==4410) return FALSE;

  act("The $o shatters violently, spraying you with metal shards!",FALSE,ch,obj,0,TO_CHAR);
  act("$n's $o shatters violently, spraying metallic shards everywhere!",FALSE,ch,obj,0,TO_ROOM);
  if(wear) extract_obj(unequip_char (ch, HOLD));
  if(carry) extract_obj(obj);
  dam=MIN(50,GET_HIT(ch)-1);
  damage(ch,ch,dam,TYPE_UNDEFINED,DAM_NO_BLOCK);
  return TRUE;
}

void assign_dasharr (void) {
  assign_mob(4429  ,  prison_guards);
  assign_mob(4448  ,  prison_guards);
  assign_mob(4472  ,  prison_voldra);
  assign_mob(4469  ,  prison_skeleton);
  assign_mob(4486  ,  prison_justice);
  assign_mob(4479  ,  prison_warden);
  assign_obj(4472  ,  prison_crystal_ball);
  assign_obj(4478  ,  prison_noose);
  assign_obj(4444  ,  prison_toilet);
  assign_obj(4476  ,  prison_coffin);
  assign_obj(4410  ,  prison_key);
  assign_room(4475 ,  prison_halls);
  assign_room(4476 ,  prison_halls);
  assign_room(4477 ,  prison_halls);
  assign_room(4478 ,  prison_halls);
}


