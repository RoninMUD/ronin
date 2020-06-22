
/***************************************************************************\
*                  Special procedure module for HELL                        *
*                    by Sumo, Balinor, and Liner                            *
*         Special thanks to Slash, Vishnu for coding and concept            *
*                -Ankh, Feroz, and Lem for area-reviews                     *
*                -and Ranger for debugging help/ideas                       *
* 10/13/02  making more crappy fixes                                        *
* -NOTE! BEFORE MAKING ANY AND ALL CHANGES TO THIS FILE, PLEASE             *
*  EITHER NOTIFY LINER AT PHANT9@HOTMAIL.COM, OR *MAKE SURE* YOUR CHANGES   *
*  CAN BOTH COMPILE AND STILL WORK UNDER PLAYTEST.  ALL SPECS HAVE BEEN     *
*  PLAYTESTED AND ARE IN WORKING CONDITION AS-IS.  Thanks.  -Liner          *
*          "If you don't test it, I will kill you."                         *
*                                                                           *
\***************************************************************************/

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
#include "enchant.h"
#include "shop.h"
#include "act.h"
#include "spec_assign.h"
#include "mob.spells.h"

#define GRIM_REAPER          25300
#define FAYN                 25321
#define MINOS                25301
#define WATER_WEIRD          25302
#define WATER_WEIRD_B        25339
#define PAOLO                25330
#define FRANCESCA            25331
#define WIND_DEVIL           25306
#define CHARYBDIS            25307
#define GLUTTON              25308
#define GLUTTON_B            25335
#define CIACCO               25329
#define LAKE_GUARD           25314
#define LIZARD               25322
#define DISEASED_CREATURE    25325
#define PAGAN                25303
#define DGUARD_ONE           25318
#define DGUARD_TWO           25319
#define DGUARD_THREE         25320
#define DGUARD_FOUR          25340
#define DGUARD_FIVE          25341
#define DGUARD_SIX           25342
#define DGUARD_SEVEN         25343
#define DGUARD_EIGHT         25344
#define DGUARD_NINE          25345
#define GERYON               25316
#define LEMURE               25305
#define MURDERER             25315
#define FLAME                25323
#define ROBE                 25324
#define SORCERESS            25327
#define SORCERESS_B          25328
#define INCUBUS              25338
#define SERPENT              25337
#define LUCIFER              25326

#define PORTAL_ENTER         3914
#define MARKET               3014
#define REAPER_ROOM          25301
#define VESTIBULE            25302
#define FALL_ROOM            25323
#define HELL_LEDGE           25324
#define GERYON_BELT          25305
#define BELT_ROOM            25515
#define ISLAND               25428
#define SORCERESS_B_ROOM     25488
#define SPIKE_ROOM           25532
#define ADDICT_ROOM          25528
#define BELT_TARGET          25516
#define TECHNO_PIT           25534
#define TECHNO_TARGET        25524
#define ROOM_TWO             25442
#define ROOM_THREE           25443
#define ROOM_FOUR            25444
#define TYPO_PIT             25530
#define CANYON_END           25441
#define LUCIFER_ROOM         25541
#define WOODS_A              25447

#define MINOS_KEY            25322
#define UNGUIS               25308
#define UNGUIS_B             25408
#define NIGHT_SWORD          25312
#define NIGHT_SWORD_B        25412
#define LEGGINGS             25313
#define LEGGINGS_B           25413
#define LUCIFER_SWORD        25324
#define DOOM_GAUNTLETS       25311
#define DOOM_GAUNTLETS_B     25411
#define HORN_GERYON          25306
#define HORN_GERYON_B        25406
#define CLOAK_FLAME          25301
#define CLOAK_FLAME_B        25401
#define FILTHY_BRACELET      25310
#define FILTHY_BRACELET_B    25410
#define RING_AGONY           25316
#define RING_AGONY_B         25416
#define HOOVES_SATAN         25315
#define HOOVES_SATAN_B       25415
#define IRON_BOOTS           25302
#define IRON_BOOTS_B         25402
#define ACHROMATIC_GEM       25323
#define ACHROMATIC_GEM_B     25423
#define HORNS_LUCIFER        25326
#define HORNS_LUCIFER_B      25426
#define RING_ACCURSED        25309
#define RING_ACCURSED_B      25409
#define RING_DESPAIR         25314
#define RING_DESPAIR_B       25414
#define CROSS_INVERTED       25328
#define CROSS_INVERTED_B     25428
#define WINGS_LUCIFER        25327
#define WINGS_LUCIFER_B      25427
#define BARRIER_DAEMONIC     25329
#define BARRIER_DAEMONIC_B   25429
#define SHADOW_MOON          25330
#define SHADOW_MOON_B        25430
#define TAIL_SERPENT         25325
#define TAIL_SERPENT_B       25425
#define CIRCLET              25331
#define CIRCLET_B            25431
#define TOKEN_A              25333
#define TOKEN_B              25334
#define TOKEN_C              25335
#define TOKEN_D              25336

struct char_data
*get_random_vict( struct char_data *ch ) {
  struct char_data *vict;
  int num=0,tmp=0;
  int number(int min, int max);
  struct char_data *v;

  for(v=world[CHAR_REAL_ROOM(ch)].people;v;v=v->next_in_room) {
    if (v!=ch)
    num++;
  }
  if(num==0) return NULL;
  tmp=number(1,num);
  num=0;

  for(vict=world[CHAR_REAL_ROOM(ch)].people;vict;vict=vict->next_in_room) {
    if(vict!=ch) num++;
    if(tmp==num) return vict;
  }
  return NULL;
}


void stop_riding(CHAR *ch, CHAR *vict);
void look_in_room (CHAR *ch, int vnum);


void block(CHAR *ch, CHAR *mob) {
  act("$n curses and pushes you back.",1,mob,0,ch,TO_VICT);
  act("$n curses and pushes $N back.",1,mob,0,ch,TO_NOTVICT);
}


void remeq(CHAR *victim, int pos) {
  char buf[MAX_INPUT_LENGTH];
  OBJ *o = NULL;
  struct descriptor_data *d;

  if(!victim) return;
  d=victim->desc;
  if(d && d->connected==CON_PLYNG) {

    if(IS_NPC(victim)) return;
    o = EQ(victim, pos);
    if(!o) return;
    unequip_char (victim, pos);
    obj_to_room (o, CHAR_REAL_ROOM(victim));
    sprintf(buf,"Hell Log: [ %s had %s removed at %d ]",GET_NAME(victim),OBJ_SHORT(o),world[CHAR_REAL_ROOM(victim)].number);
    log_s(buf);
    o->log=1;
    save_char(victim,NOWHERE);
  }
}


void Slice(CHAR *s, CHAR *victim) {
  char *body = NULL;
  char buf[MAX_INPUT_LENGTH];
  struct descriptor_data *d;
  int i;

  i=number(0,11);
  switch(i) {
    case
     0:  body = "right finger";               /*lose your ring?*/
	           remeq(victim, WEAR_FINGER_R);
	           damage(s,victim,100,TYPE_SLASH,DAM_NO_BLOCK);
	           break;
    case 1:  body = "left finger";                /*lose your ring?*/
	           remeq(victim, WEAR_FINGER_L);
	           damage(s,victim,100,TYPE_SLASH,DAM_NO_BLOCK);
	           break;
    case 2:  body = "neck";                       /*where's your neck?*/
	           remeq(victim, WEAR_NECK_1);
	           remeq(victim, WEAR_NECK_2);
	           remeq(victim, WEAR_HEAD);
	           damage(s,victim,2000,TYPE_SLASH,DAM_NO_BLOCK);
	           break;
    case 3:  body = "head";
	           remeq(victim, WEAR_HEAD);
	           damage(s,victim,7000,TYPE_SLASH,DAM_NO_BLOCK);
	           break;
    case 4:  body = "legs";
	           remeq(victim, WEAR_LEGS);
	           remeq(victim, WEAR_FEET);
	           damage(s,victim,600,TYPE_SLASH,DAM_NO_BLOCK);
                   break;
    case 5:  body = "feet";
                   remeq(victim, WEAR_FEET);
	           damage(s,victim,250,TYPE_SLASH,DAM_NO_BLOCK);
	           break;
    case 6:  body = "right arm";
                   remeq(victim, WEAR_ARMS);
	           remeq(victim, WEAR_HANDS);
                   remeq(victim, WEAR_FINGER_R);
                   remeq(victim, WEAR_WRIST_R);
                   remeq(victim, WIELD);
	           damage(s,victim,500,TYPE_SLASH,DAM_NO_BLOCK);
	           break;
    case 7:  body = "left arm";
	           remeq(victim, WEAR_ARMS);
                   remeq(victim, WEAR_HANDS);
	           remeq(victim, WEAR_FINGER_L);
	           remeq(victim, WEAR_WRIST_L);
                   remeq(victim, HOLD);
                   remeq(victim, WEAR_SHIELD);
	           damage(s,victim,500,TYPE_SLASH,DAM_NO_BLOCK);
                   break;
    case 8:  body = "right hand";
                   remeq(victim, WEAR_HANDS);
	           remeq(victim, WIELD);
	           remeq(victim, WEAR_FINGER_R);
                   damage(s,victim,250,TYPE_SLASH,DAM_NO_BLOCK);
	           break;
    case 9:  body = "left hand";
	           remeq(victim, WEAR_HANDS);
      	           remeq(victim, HOLD);
                   remeq(victim, WEAR_SHIELD);
                   remeq(victim, WEAR_FINGER_L);
	           damage(s,victim,250,TYPE_SLASH,DAM_NO_BLOCK);
                   break;
    case 10: body = "right wrist";
                   remeq(victim, WEAR_WRIST_R);
	           remeq(victim, WEAR_HANDS);
	           remeq(victim, WEAR_FINGER_R);
                   remeq(victim, WIELD);
	           damage(s,victim,200,TYPE_SLASH,DAM_NO_BLOCK);
                   break;
    case 11: body = "left wrist";
	           remeq(victim, WEAR_HANDS);
	           remeq(victim, HOLD);
	           remeq(victim, WEAR_SHIELD);
	           remeq(victim, WEAR_FINGER_L);
	           remeq(victim, WEAR_WRIST_L);
	           damage(s,victim,200,TYPE_SLASH,DAM_NO_BLOCK);
	           break;
    default:
                   break;
  }
  if(!victim) return;
  if(!s) return;
  if(!body) return;
  d=victim->desc;
  if(d && d->connected==CON_PLYNG) {
    sprintf(buf,"\n\rThe Daemon-Lord Fayn hacks off your %s with his massive fingernail!",body);
    send_to_char(buf,victim);
    act("\n\rYou hack gleefully at $N's body!",FALSE,s,0,victim,TO_CHAR);
    act("The Daemon-Lord Fayn hacks part of $N's body away, blood spurting on his gleeful face!!",FALSE,s,0,victim,TO_NOTVICT);
    act("\n\rYou are forced to drop some equipment.",FALSE,s,0,victim,TO_VICT);
  }
}

   /* Hell Object Specs */

int is_shop(CHAR *mob);


int Unguis (OBJ *dagger, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*temp, *next_vict;
  int DONE = FALSE;

  if (!IS_MORTAL(ch)) return FALSE;

  if (dagger != EQ(ch, WIELD)) return FALSE;

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(cmd==CMD_BACKSTAB) {
    while((*arg == ' ')) arg++;
    if ((vict = get_char_room_vis(ch,arg)))
      if(GET_LEVEL(vict) >= GET_LEVEL(ch)) {
        if(cmd==CMD_BACKSTAB) {
          if ((GET_CLASS(ch) != CLASS_THIEF) && (GET_CLASS(ch) != CLASS_ANTI_PALADIN)) {
             send_to_char("You don't know this skill.\n\r", ch);
             return TRUE;
          }/* end if getclass */

          if(ch->specials.fighting) return FALSE; /* added 06/03/02 */
          if (GET_OPPONENT(vict)) return FALSE;

          //Skeena 6/12/11: No free damage/death spec in Museum
          if(GET_ZONE(ch) == 165) {}
          else if(number(0,45)==0) {
            act("$N staggers forward as $n slashes $s $p across $N's throat!",0,ch,dagger,vict,TO_NOTVICT);
            act("$N staggers forward as you slash your $p across $S throat!",0,ch,dagger,vict,TO_CHAR);
            act("You let out a blood curdling scream as $n slashes $s $p across your throat!",0,ch,dagger,vict,TO_VICT);
            damage(ch, vict, 8*(GET_LEVEL(ch))+number(0,500), TYPE_UNDEFINED,DAM_NO_BLOCK);
            WAIT_STATE(ch, PULSE_VIOLENCE*3);
            DONE = TRUE;
          }/* end if chance 0,50 */

          else if(number(0,190)==0) {
            act("$n's $p tears violently at $N's throat, decapitating $M and sending the head flying through the air!",0,ch,dagger,vict,TO_NOTVICT);
            act("Your $p tears violently at $N's throat, decapitating $M and sending $S head flying through the air!",0,ch,dagger,vict,TO_CHAR);
            act("$n's $p tears at your throat, loosening your head from your shoulders. *rrrrrrip*",0,ch,dagger,vict,TO_VICT);
            act("\n\r$p spins in a fury of bloodlust, slashing everyone in the room !!\n\r",0,ch,dagger,vict,TO_ROOM);
            act("\n\r$p spins in a fury of bloodlust, slashing everyone in the room !!\n\r",0,ch,dagger,vict,TO_CHAR);
            for (temp=world[CHAR_REAL_ROOM(vict)].people;temp;temp=next_vict) {
              next_vict = temp->next_in_room;
              damage(vict, temp, abs(GET_HIT(temp)/2), TYPE_UNDEFINED,DAM_NO_BLOCK);
            }
            damage(ch, vict, 66666, TYPE_KILL,DAM_NO_BLOCK);
            WAIT_STATE(ch, PULSE_VIOLENCE*4);
            DONE = TRUE;
          }/* end if else 0,200 */
        }/* end if backstab */
      }/* end if getlevel */
    }  /* end if cmd backstab */
return DONE;
}/* end int */


int Night_Sword (OBJ *sword, CHAR *ch, int cmd, char *arg) {
  int chance;
  char buf[MAX_INPUT_LENGTH], name[256], name2[256], buf2[MIL];
  CHAR *victim = NULL, *targ = NULL, *owner = NULL, *holder = NULL;

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    if(!sword->equipped_by) return FALSE;
    owner=sword->equipped_by;
    act("......$n is surrounded by a shroud of starlight!",FALSE,owner,sword,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_MOBACT && sword->equipped_by) {
    victim = sword->equipped_by;
/* new charmie check */
    if(IS_NPC(victim) && V_OBJ(sword)==NIGHT_SWORD_B && IS_AFFECTED(victim, AFF_CHARM)) {
      if(number(0,10)!=0) return FALSE;
      act("You have been blasted by the awesome power of the Sword of Night!!",FALSE,victim,0,0,TO_CHAR);
      act("$n has been blasted by the Sword of Night!!",FALSE,victim,0,0,TO_NOTVICT);
      extract_obj(unequip_char(victim,WIELD));
      damage(victim,victim,66666,TYPE_SLASH,DAM_NO_BLOCK);
      return FALSE;
    }

    if(GET_ALIGNMENT(victim) > -300) {
      act("You have been blasted by the awesome power of the Sword of Night!!",FALSE,victim,0,0,TO_CHAR);
      act("$n has been blasted by the Sword of Night!!",FALSE,victim,0,0,TO_NOTVICT);
      if(IS_MORTAL(victim)) damage(victim,victim,number(110,190),TYPE_SLASH,DAM_NO_BLOCK);
      unequip_char(victim,WIELD);
      obj_to_room(sword,CHAR_REAL_ROOM(victim));
      sprintf(buf,"[ %s had %s unwielded at %d ]",GET_NAME(victim),OBJ_SHORT(sword),world[CHAR_REAL_ROOM(victim)].number);
      log_s(buf);
      return FALSE;
    }
  }

  if(sword->equipped_by) owner = sword->equipped_by;
  if(sword->carried_by) holder = sword->carried_by;

  /* AT NIGHTTIME SWORD BECOMES BETTER */
  if(sword->equipped_by && owner==sword->equipped_by && cmd==MSG_TICK && IS_NIGHT) {
    if(sword->affected[0].modifier>14) return FALSE;  /* if already changed to better, false */
    unequip_char(owner,WIELD);

    if(sword->affected[0].modifier==13) {      /* night a */
      sword->affected[0].modifier=19;
    }
    if(sword->affected[0].modifier==14) {      /* night b */
      sword->affected[0].modifier=24;
    }
    equip_char(owner,sword,WIELD);
    save_char(owner,NOWHERE);
    return FALSE;
  }
  else if(sword->equipped_by && owner==sword->equipped_by && cmd==MSG_TICK && !IS_NIGHT) {
    if(sword->affected[0].modifier<19) return FALSE;  /* if already changed to normal, false */
    unequip_char(owner,WIELD);

    if(sword->affected[0].modifier==19) {      /* night a */
      sword->affected[0].modifier=13;
    }
    if(sword->affected[0].modifier==24) {      /* night b */
      sword->affected[0].modifier=14;
    }
    equip_char(owner,sword,WIELD);
    save_char(owner,NOWHERE);
    return FALSE;
  }

  switch(cmd) {
    case CMD_WIELD:
      if(ch!=holder) break;
      if(GET_CLASS(ch)==CLASS_MAGIC_USER || GET_CLASS(ch)==CLASS_CLERIC ||
         GET_CLASS(ch)==CLASS_PALADIN || GET_CLASS(ch)==CLASS_NINJA) break;
      one_argument(arg,buf);
      if(!isname(buf,OBJ_NAME(sword))) break;
      if(!sword->carried_by || holder!=sword->carried_by) break;
        /* ALREADY WIELDING SOMETHING */
      if(EQ(holder,WIELD)) {
        act("You are already wielding something!",FALSE,holder,0,0,TO_CHAR);;
        return TRUE;
      }
      if((GET_LEVEL(holder)<30 || GET_ALIGNMENT(holder)>-350) && IS_MORTAL(holder)) {
        act("You have been blasted by the awesome power of the Sword of Night!!",FALSE,holder,0,0,TO_CHAR);
        act("$n has been blasted by the Sword of Night!",FALSE,holder,0,0,TO_NOTVICT);
        sprintf(buf2,"Hell Log: %s blasted by %s (Room %d)",GET_NAME(holder),OBJ_SHORT(sword),CHAR_VIRTUAL_ROOM(holder));
        log_s(buf2);
        if(IS_MORTAL(holder)) damage(holder,holder,number(110,190),TYPE_SLASH,DAM_NO_BLOCK);
        /* check if holder died */
        if (OBJ_CARRIED_BY(sword)) {
          obj_from_char(sword);
          save_char(holder, NOWHERE);
          obj_to_room(sword,CHAR_REAL_ROOM(holder));
          sword->log=1;
        }
        holder=0;
        return TRUE;
      }
      else if(enchanted_by(holder,"Greasy Palms")) {
        send_to_char("Your hands are much too slippery to hold anything right now.\n\r",holder);
        return TRUE;
      }
      else {
        act("As you wield the Sword of Night, you can feel its awesome power.",FALSE,holder,0,0,TO_CHAR);
        act("As $n wields the Sword of Night, the surroundings darken slightly.",FALSE,holder,0,0,TO_NOTVICT);
      }
      break;

    case CMD_KILL:
      if(ch!=owner) break;
      if(IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, SAFE) && (!CHAOSMODE)) return TRUE;
      if(owner->specials.fighting || !sword->equipped_by || owner!=sword->equipped_by) break;
      one_argument(arg,buf);
      if(!*buf) {
        /* NO TARGET? HIT SELF! */
        act("You swing the Sword of Night violently towards yourself!",FALSE,owner,0,0,TO_CHAR);
        act("$n slashes $mself with the Sword of Night!",FALSE,owner,0,0,TO_NOTVICT);
        if(IS_MORTAL(owner))
          damage(owner,owner,66,TYPE_SLASH,DAM_NO_BLOCK);
        return TRUE;
      }
      if (!(victim = get_char_room_vis(owner, buf))) {
        send_to_char("You swing the Sword of Night around through the air!\n\r",ch);
        return TRUE;
      }
      if(owner == victim) {
        /* TARGET IS SELF!? */
        act("You swing the Sword of Night violently towards yourself!",FALSE,owner,0,0,TO_CHAR);
        act("$n slashes $mself with the Sword of Night!",FALSE,owner,0,0,TO_NOTVICT);
        if(IS_MORTAL(owner))
          damage(owner,owner,66,TYPE_SLASH,DAM_NO_BLOCK);
        return TRUE;
      }
      if(!IS_NPC(victim) && !IS_NPC(owner)) {
        send_to_char("Your sword refuses to attack other players!\n\r",owner);
        return TRUE;
      }
      if(is_shop(victim)) break;

      if(IS_SET(world[CHAR_REAL_ROOM(victim)].room_flags,SAFE) || GET_POS(owner)!=POSITION_STANDING || victim==owner->specials.fighting || !IS_NPC(victim)) break;
      if(GET_ALIGNMENT(owner) <= -950 && GET_ALIGNMENT(victim) >= 750) chance = 66;
      if(GET_ALIGNMENT(owner) <= -950 && GET_ALIGNMENT(victim) < 750) chance = 96;
      if(GET_ALIGNMENT(owner) > -950 && GET_ALIGNMENT(victim) >= -600) chance = 111;
      if(GET_ALIGNMENT(owner) > -950 && GET_ALIGNMENT(victim) < -600) chance = 222;

      //Skeena 6/12/11: SoN kill spec doesn't work in Museo
      if(number(0,chance) == 0 && GET_ZONE(owner) != 165) {
        /* DO INSTAKILL? */
        act("The Sword of Night drains all the life from $N's body!",FALSE,owner,0,victim,TO_NOTVICT);
        act("You feel your life force drained as the Sword of Night slices through you!",FALSE,owner,0,victim,TO_VICT);
        act("Your Sword of Night drains all the life from $N's body!",FALSE,owner,0,victim,TO_CHAR);
        damage(owner,victim,66666, TYPE_KILL,DAM_NO_BLOCK);
        WAIT_STATE(owner, PULSE_VIOLENCE*2);
        return TRUE;
      }
      else {
        /* HIT MOB */
        act("The Sword of Night chills your soul as it hungers for blood!",FALSE,victim,0,0,TO_ROOM);
        act("You notice the surroundings darken slightly as the Sword of Night rapidly approaches your body!",FALSE,victim,0,0,TO_CHAR);
        WAIT_STATE(owner, PULSE_VIOLENCE*2);
      }
      break;

    case CMD_CONSIDER:
      if(ch!=owner) break;
      /* LIKE FUNG_TAI? ALWAYS MAKE MOBS EASY? */
      if(!sword->equipped_by || owner!=sword->equipped_by) break;
      one_argument(arg, name2);

      if (!(targ = get_char_room_vis(owner, name2))) {
        send_to_char("Consider killing who?\n\r", owner);
        return TRUE;
      }

      if (targ == owner) {
        send_to_char("Easy! Very easy indeed!\n\r", owner);
        return TRUE;
      }

      send_to_char("Now where did that chicken go?\n",owner);
      return TRUE;

    case CMD_FLEE:
      if(ch!=owner) break;
      /* Sword of Night YEARNS FOR BLOOD AND DOESN'T TOLERATE WIMPS */
      if(!sword->equipped_by || owner!=sword->equipped_by) break;
      if(GET_POS(owner) != POSITION_FIGHTING) break;

      act("You don't want to flee, you want to Fight!",FALSE,owner,0,0,TO_CHAR);
      act("$n tries to flee but something stops him!",FALSE,owner,0,0,TO_ROOM);
      return TRUE;

    case CMD_REMOVE:
      if(ch!=owner) break;
      /* SWORD DOESN'T LIKE BEING REMOVED... SUFFER PUNISHMENT */
      if(!sword->equipped_by || owner!=sword->equipped_by) break;
      one_argument(arg,buf);
      if(!isname(buf,OBJ_NAME(sword))) break;
      act("The Sword of Night lashes out at you as you remove it!",FALSE,owner,0,0,TO_CHAR);
      act("$n is slashed by the Sword of Night as $e removes it.",FALSE,owner,0,0,TO_NOTVICT);

      unequip_char(owner,WIELD);
      if(sword->affected[0].modifier==19) {      /* night a */
        sword->affected[0].modifier=13;
      }
      if(sword->affected[0].modifier==24) {      /* night b */
        sword->affected[0].modifier=14;
      }
      equip_char(owner,sword,WIELD);

      if(IS_MORTAL(owner)) damage(owner,owner,number(50,100),TYPE_SLASH,DAM_NO_BLOCK);
      break;

    case CMD_GET:
      if(ch!=holder && ch!=owner) break;
     /* if(sword->in_room != CHAR_REAL_ROOM(ch)) break; */
      one_argument(arg,buf);
      if(!isname(buf,OBJ_NAME(sword))) break;
      if(ch==holder) {
        act("You cannot get another Sword!",FALSE,holder,0,0,TO_CHAR);
        return TRUE;
      } else if(ch==owner) {
        act("You cannot get another Sword!",FALSE,owner,0,0,TO_CHAR);
        return TRUE;
      }
      if((!IS_EVIL(ch) || GET_LEVEL(ch) < 30) && IS_MORTAL(ch)) {

        /* GOOD, NEUTRAL OR WRONG LEVEL --- BAD! */
        act("You have been blasted by the awesome power of the Sword of Night!",FALSE,ch,0,0,TO_CHAR);
        act("$n has been blasted by the Sword of Night!",FALSE,ch,0,0,TO_NOTVICT);
        damage(ch,ch,number(110,190),TYPE_SLASH,DAM_NO_BLOCK);
        return TRUE;
      }
      act("You can feel the raw power contained in this evil blade as you pick it up.",FALSE,ch,0,0,TO_CHAR);
      act("The sky darkens considerably.",FALSE,ch,0,0,TO_NOTVICT);
      break;

    case CMD_SIT:
    case CMD_REST:
    case CMD_SLEEP:
      if(ch!=owner) break;
      /* IF IT IS NIGHTTIME, Sword of Night WANTS TO FIGHT NOT SLEEP! */
      if(!IS_NIGHT) break;
      if(!sword->equipped_by || owner!=sword->equipped_by) break;
      act("For some reason you don't feel like resting right now.\n\r You have a strong desire to kill something.",FALSE,owner,0,0,TO_CHAR);
      act("Something prevents $n from resting.",FALSE,owner,0,0,TO_NOTVICT);
      return TRUE;

    case CMD_GIVE:
      if(ch!=holder) break;
      if(!sword->carried_by || holder!=sword->carried_by) break;
      if(!sword || (V_OBJ(sword)!=25312 && V_OBJ(sword)!=25412)) break;
      arg=one_argument(arg,name);
      if(!*name) break;
      if(!isname(name,OBJ_NAME(sword))) break;
      one_argument(arg,name);
      if(!*name) break;
      if(!(victim = get_char_vis(holder,name))) {
        sprintf(buf, "They are not here to receive the Sword of Night.\n\r");
        send_to_char(buf,holder);
        return TRUE;
      }
      victim=get_char_vis(holder,name);
      if(victim==holder) {
        act("What the hell do you think you're trying to do?!",FALSE,holder,0,victim,TO_CHAR);
        return TRUE;
      }
      if(GET_ALIGNMENT(victim) < -300 && GET_LEVEL(victim) >= 30) {
        act("As you give the Sword of Night to $N, it slashes you viciously!",FALSE,holder,0,victim,TO_CHAR);
        act("As $n gives you the Sword of Night, it slashes $m violently!",FALSE,holder,0,victim,TO_VICT);
        act("As $n gives the Sword of Night to $N, it slashes $m violently!",FALSE,holder,0,victim,TO_NOTVICT);
        if(IS_MORTAL(holder)) damage(holder,holder,number(50,100),TYPE_SLASH,DAM_NO_BLOCK);
      }
      else {
        act("Something prevents you from giving the Sword of Night to $N.",FALSE,holder,0,victim,TO_CHAR);
        return TRUE;
      }
      break;

    case CMD_DROP:
    case CMD_DONATE:
      if(ch!=holder) break;
      if(!sword->carried_by || holder!=sword->carried_by) break;
      one_argument(arg,buf);
      if(!isname(buf,OBJ_NAME(sword)) || (V_OBJ(sword)!=NIGHT_SWORD && V_OBJ(sword)!=NIGHT_SWORD_B)) break;
      act("As you release the Sword of Night it slashes you viciously!",FALSE,holder,0,0,TO_CHAR);
      act("$n is slashed violently by the Sword of Night as $e releases it.",FALSE,holder,0,0,TO_NOTVICT);
      if(IS_MORTAL(holder)) damage(holder,holder,number(50,100),TYPE_SLASH,DAM_NO_BLOCK);
      break;

    default:
      break;
  }
  return FALSE;
}


int Doom_Gauntlets(OBJ *obj,CHAR *ch, int cmd, char *argument) {
  CHAR *wearer, *vict;
  int counter = number(60,70);

  if(!(wearer=obj->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(wearer)) {
    act("......$n's hands gleam with the wrath of Lucifer!",FALSE,wearer,obj,ch,TO_VICT);
    return FALSE;
  }

  if(!wearer) return FALSE;
  if(GET_LEVEL(wearer)<25) return FALSE;
  if(!wearer->specials.fighting) return FALSE;

  if(IS_SET(world[CHAR_REAL_ROOM(wearer)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(cmd==MSG_TICK && obj->equipped_by) {
    if(OBJ_SPEC(obj) >0)  OBJ_SPEC(obj)--;
    return FALSE;
  }

  if(cmd==MSG_MOBACT && OBJ_SPEC(obj) != 0 && chance(9)) {
    vict=GET_OPPONENT(wearer);
    if(!vict) return FALSE;
    if(CHAR_REAL_ROOM(wearer)!=CHAR_REAL_ROOM(vict)) return FALSE;

    act("$n sends a powerful punch at your stomach!", FALSE, wearer, 0, vict, TO_VICT);
    act("You send a powerful punch towards $N!", FALSE, wearer, 0, vict, TO_CHAR);
    act("$n punches $N in the gut!", FALSE, wearer, 0 ,vict, TO_NOTVICT);
    damage(wearer, vict, number(50,100), TYPE_UNDEFINED,DAM_NO_BLOCK);
    WAIT_STATE(wearer,PULSE_VIOLENCE*2);
    return FALSE;
  }

  if(cmd==MSG_MOBACT && OBJ_SPEC(obj) == 0) {
    if(chance(1)) {
      vict=GET_OPPONENT(wearer);
      if(!vict) return FALSE;
      if(CHAR_REAL_ROOM(wearer) != CHAR_REAL_ROOM(vict)) return FALSE;

      act("You viciously drive your fist into $N's chest, tearing out $S still beating heart!",FALSE,wearer,0,vict,TO_CHAR);
      act("A gout of blood sprays from your chest as $n drives $s fist through your ribcage and tears out your heart!",FALSE,wearer,0,vict,TO_VICT);
      act("$n drives $s fist right through $N's ribcage, and tears out $S heart!!",FALSE,wearer,0,vict,TO_NOTVICT);
      damage(wearer,vict,66666,TYPE_UNDEFINED,DAM_NO_BLOCK);
      OBJ_SPEC(obj)=counter;
      return FALSE;
    } else {
      OBJ_SPEC(obj)=counter;
      return FALSE;
    }
  }
  return FALSE;
}


int Flame_Cloak(OBJ *obj,CHAR *ch, int cmd, char *argument) {
  CHAR *wearer,*vict;
  int chance;
  int counter= number(30,35);

  if(!(wearer=obj->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(wearer)) {
    act("......$n is ablaze with an unholy light!",FALSE,wearer,obj,ch,TO_VICT);
    return FALSE;
  }

  if(!wearer) return FALSE;
  if(IS_NPC(wearer)) return FALSE;
  if(GET_LEVEL(wearer) < 25) return FALSE;

  if(IS_SET(world[CHAR_REAL_ROOM(wearer)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(cmd==MSG_TICK && obj->equipped_by) {
    if(OBJ_SPEC(obj) >0) OBJ_SPEC(obj)--;
    return FALSE;
  }

  vict=wearer->specials.fighting;
  if(!vict) return FALSE;
  if(CHAR_REAL_ROOM(wearer) != CHAR_REAL_ROOM(vict)) return FALSE;

  if(cmd==MSG_MOBACT && OBJ_SPEC(obj) == 0) {
    chance = number(0,3);
    if(chance==0) {
      act("$n's $p ignites into a mantle of unholy black flames and engulfs $N!",FALSE,wearer,obj,vict,TO_NOTVICT);
      act("A shroud of unholy black flames erupts from $n's cloak, engulfing you!",FALSE,wearer,obj,vict,TO_VICT);
      act("Your $p bursts into a mantle of unholy black flames and engulfs $N!",FALSE,wearer,obj,vict,TO_CHAR);
      spell_fire_breath(GET_LEVEL(wearer), wearer, vict, 0);
      OBJ_SPEC(obj) = counter;
      return FALSE;
    }
    else {
      act("A thick cloud of smoke billows out from the folds of $n's $p!",FALSE,wearer,obj,0,TO_NOTVICT);
      act("Your vision is obscured by a thick cloud of smoke billowing from your $p!",FALSE,wearer,obj,0,TO_CHAR);
      spell_blindness(GET_LEVEL(wearer),wearer,wearer,0);
      OBJ_SPEC(obj) = counter;
      return FALSE;
    }
  }
  return FALSE;
}


const char * const geryon_song[] =
{
  "ode to the big black demon i call my lord",	/* 1 */
  "\n",
};


int Geryon_Horn(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *vict, *next_vict, *holder, *tar_ch;
  struct affected_type_5 af;
  int song_affect = number(1,9);
  int qend, so;
  char *skip_spaces(char *string);
  int counter = number(50,60);

  if(!(holder=obj->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(holder)) {
    act("......$n commands a daemonic power within $s voice!",FALSE,holder,obj,ch,TO_VICT);
    return FALSE;
  }

  if (cmd != MSG_MOBACT && cmd != CMD_SONG && cmd != MSG_TICK) return FALSE;
  if(!holder) return FALSE;

/*  if(IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE; */

  if(cmd==MSG_TICK && obj->equipped_by) {
    if(OBJ_SPEC(obj) >0) OBJ_SPEC(obj)--;
    if(OBJ_SPEC(obj)==0) OBJ_SPEC(obj)=0;
    return FALSE;
  }

  if(ch==holder && cmd==CMD_SONG) {
    if(ch != holder) return FALSE;
    if(IS_NPC(ch)) return FALSE;

    if((GET_LEVEL(holder) < 42) && (GET_CLASS(holder) != CLASS_BARD)) {
      send_to_char("Please leaves this song to the high-level bards.\n\r", holder);
      return TRUE;
    }

    arg = skip_spaces(arg);
    if (!(*arg)) {
      send_to_char("What Song?\n\r", holder);
      return TRUE;
    }

    if (*arg != '\'') {
      send_to_char("Songs must always enclosed by the symbols :'\n\r", holder);
      return TRUE;
    }

    for (qend=1; *(arg+qend) && (*(arg+qend) != '\'') ; qend++)
      *(arg+qend) = LOWER(*(arg+qend));

    if (*(arg+qend) != '\'') {
      send_to_char("Songs must always be enclosed by the symbols :'\n\r", holder);
      return TRUE;
    }

    so = old_search_block(arg, 1, qend-1, geryon_song, 0);

    if(so!=1) return FALSE;

    if(IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, SAFE) && (!CHAOSMODE))
    {
      send_to_char("Even you are not evil enough to summon your master to this sacred place!", holder);
      return FALSE;
    }

    if (GET_LEVEL(holder) < 42) {
      send_to_char("You cannot quite remember the words to that song!\n\r", holder);
      return TRUE;
    }

    if(song_affect < 9) {
      if(GET_MANA(holder) < 100) {
        send_to_char("You can't summon enough energy to sing the song!\n\r",holder);
        return TRUE;
      }
      else
        GET_MANA(holder) -= 100;
    }
    else {
      if(GET_MAX_MANA(holder) < 500) {
        send_to_char("Your feeble mana is too low for such a powerful song!\n\r",holder);
        return TRUE;
      }
      if(GET_MANA(holder) < (3*(GET_MAX_MANA(holder))/4)) {
        send_to_char("You can't summon enough energy to sing the song!\n\r",holder);
        return TRUE;
      }
      else
        GET_MANA(ch) = 50;
    }

    if (IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, NO_MAGIC)) {
      send_to_char("Your song has been absorbed by the surroundings.\n\r", holder);
      return TRUE;
    }

    if (IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, NO_SONG)) {
      send_to_char("Your song has been absorbed by the surroundings.\n\r", holder);
      return TRUE;
    }

    WAIT_STATE(holder, PULSE_VIOLENCE * 2);

    send_to_char("You sing the song.\n\r", holder);

    act("$n sings 'Ode to the Big Black Demon I Call My Lord....'\n\r",0,holder,0,0,TO_ROOM);
    act("You sing 'Ode to the Big Black Demon I Call My Lord....'",0,holder,0,0,TO_CHAR);
    switch(song_affect) {

      case 3:
               for (tar_ch = world[CHAR_REAL_ROOM(holder)].people;tar_ch;tar_ch=next_vict) {
                 next_vict=tar_ch->next_in_room;
                 if(IS_MORTAL(tar_ch)) {
                   act("Lord Geryon appears and pummels you into submission, vanishing in flame after.",1,tar_ch,0,0,TO_CHAR);
                   damage(tar_ch,tar_ch,198,TYPE_UNDEFINED,DAM_NO_BLOCK);  /* don't want killer flags */
                   WAIT_STATE(tar_ch,PULSE_VIOLENCE*2);
                 }
               }
               break;
      case 4:
               for (tar_ch = world[CHAR_REAL_ROOM(holder)].people;tar_ch;tar_ch=tar_ch->next_in_room) {
                 if(!affected_by_spell(tar_ch, SPELL_POISON) && (IS_MORTAL(tar_ch) || (IS_NPC(tar_ch) && GET_LEVEL(tar_ch)<46))) {
                   spell_poison(50,holder,tar_ch,0);
                 }
               }
               break;
      case 5:
               for (tar_ch = world[CHAR_REAL_ROOM(holder)].people;tar_ch;tar_ch=tar_ch->next_in_room) {
                 if(!affected_by_spell(tar_ch, SPELL_BLINDNESS) && (IS_MORTAL(tar_ch) || (IS_NPC(tar_ch) && GET_LEVEL(tar_ch)<46))) {
                   spell_blindness(50,holder,tar_ch,0);
                 }
               }
               break;
      case 6:
               for (tar_ch = world[CHAR_REAL_ROOM(holder)].people;tar_ch;tar_ch=tar_ch->next_in_room) {
                 if(!affected_by_spell(tar_ch, SPELL_ENDURE)) {
                   spell_endure(50,holder,tar_ch,0);
                 }
               }
               break;
      case 7:
               for (tar_ch = world[CHAR_REAL_ROOM(holder)].people;tar_ch;tar_ch=tar_ch->next_in_room) {
                 if(!affected_by_spell(tar_ch, SPELL_SENSE_LIFE)) {
                   spell_sense_life(50,tar_ch,tar_ch,0);
                 }
               }
               break;
      case 8:
               for (tar_ch = world[CHAR_REAL_ROOM(holder)].people;tar_ch;tar_ch=tar_ch->next_in_room) {
                 if(!IS_NPC(tar_ch)) {
                   spell_evil_bless(50,tar_ch,tar_ch,0);
                   spell_evil_bless(50,tar_ch,tar_ch,0);
                   spell_evil_bless(50,tar_ch,tar_ch,0);
                   spell_evil_bless(50,tar_ch,tar_ch,0);
                   spell_evil_bless(50,tar_ch,tar_ch,0);
                 }
               }
               break;
      case 9:
               for (tar_ch = world[CHAR_REAL_ROOM(holder)].people;tar_ch;tar_ch=tar_ch->next_in_room) {
                 if (!IS_AFFECTED2(ch, AFF2_RAGE)) {
                   send_to_char("Rage courses through your body!\n\r", tar_ch);
                   act("$n is enraged with the fury of Lord Geryon!",1,tar_ch,0,0,TO_ROOM);
                   af.type      = SPELL_RAGE;
                   af.duration  = 5;
                   af.modifier  = 0;
                   af.location  = 0;
                   af.bitvector = 0;
                   af.bitvector2 = AFF2_RAGE;
                   affect_to_char(tar_ch, &af);
                 }
               }
               break;

      default:
               for (tar_ch = world[CHAR_REAL_ROOM(holder)].people;tar_ch;tar_ch=tar_ch->next_in_room) {
                 spell_vitality(50,holder,tar_ch,0);
               }
               break;
    }
    return TRUE;
  }

  if(cmd==MSG_MOBACT && holder->specials.fighting && chance(6)) {
    if(OBJ_SPEC(obj) == 0) {
      for(vict = world[CHAR_REAL_ROOM(holder)].people;vict; vict = next_vict) {
        next_vict = vict->next_in_room;
        if(vict!=holder) {
          act("$n's $p begins thundering out an ancient battle song, blasting your ears with a cacophony of destruction!",FALSE,holder,obj,vict,TO_VICT);
          if(IS_MORTAL(vict))
            damage(vict, vict, number(111,222), TYPE_UNDEFINED,DAM_NO_BLOCK);
          else if(IS_MOB(vict))
            damage(holder, vict, number(333,666), TYPE_UNDEFINED,DAM_NO_BLOCK);
          GET_POS(vict) = POSITION_STUNNED;
        }
      }
      act("Your $p begins thundering out an ancient battle song, causing you to grab your head in pain!",FALSE,holder,obj,0,TO_CHAR);
      OBJ_SPEC(obj) = counter;
      return FALSE;
    }
    return FALSE;
  }
  return FALSE;
}


int greasy_palms(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg) {

  if(cmd==MSG_REMOVE_ENCH) {
    act("The grease on $n's hands finally clears completely.", TRUE, ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your hands don't feel quite so slick anymore.\n\r",ench_ch);
    return FALSE;
  }

  if(CHAR_REAL_ROOM(ench_ch)==real_room(PORTAL_ENTER) && cmd==CMD_NORTH) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("A magical force blocks your sinful being from entering this place.\n\r",ench_ch);
    return TRUE;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(ench_ch)].room_flags, CHAOTIC) && cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(ch==ench_ch && cmd==CMD_WIELD) {
    send_to_char("Your hands are much too slippery to hold anything right now.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==MSG_TICK && ench->duration==1) {
    act("Most of the grease has been rubbed from $n's hands.",TRUE, ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your hands are starting to become less slippery.\n\r",ench_ch);
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n's hands are covered with globules of decaying flesh!",FALSE,ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

  return FALSE;
}


int Filthy_Bracelet(OBJ *obj,CHAR *ch, int cmd, char *argument) {
  OBJ *weap;
  CHAR *wearer;
  char buf[MIL];
  ENCH *tmp_enchantment;
  struct descriptor_data *d;
  int counter = number(20,60);


  if(!(wearer=obj->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(wearer)) {
    act("......$n exudes a faint odor of decay!",FALSE,wearer,obj,ch,TO_VICT);
    return FALSE;
  }

  if(!wearer) return FALSE;
  if(IS_NPC(wearer)) return FALSE;
  if(GET_LEVEL(wearer) < 25) return FALSE;

  if(cmd==MSG_TICK && obj->equipped_by) {
    if(OBJ_SPEC(obj) >0) OBJ_SPEC(obj)--;
    return FALSE;
  }

  if(cmd==MSG_MOBACT && wearer->specials.fighting && OBJ_SPEC(obj)==0) {
    d=wearer->desc;
    if(!d) return FALSE;
    if(d->connected!=CON_PLYNG) return FALSE; /* If char died */

    weap = EQ(wearer, WIELD);
    if(!weap) return FALSE;
    unequip_char (wearer, WIELD);
    obj_to_char (weap, wearer);
    act("$n's hand becomes coated with a thick coat of grease from $s bracelet and has to let go of $s $p.",FALSE,wearer,weap,0,TO_NOTVICT);
    act("Your hand becomes slick with a coating of grease from your bracelet and your $p slips from your grasp!",FALSE,wearer,weap,0,TO_CHAR);
    CREATE(tmp_enchantment, ENCH, 1);
    tmp_enchantment->name     = str_dup("Greasy Palms");
    tmp_enchantment->duration = 6;
    tmp_enchantment->func     = greasy_palms;
    enchantment_to_char(wearer, tmp_enchantment, FALSE);
    OBJ_SPEC(obj) = counter;
    sprintf(buf,"Hell Log Ench: [ %s just contracted Greasy Palms at %d ]",GET_NAME(wearer),world[CHAR_REAL_ROOM(wearer)].number);
    log_s(buf);
    return FALSE;
  }
  return FALSE;
}


int Ring_Agony (OBJ *ring, CHAR *ch, int cmd, char *arg) {
  CHAR *vict, *owner;
  char buf[MAX_INPUT_LENGTH];
  int counter=80;

  if(!(owner=ring->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(owner)) {
    act("......a series of intense convulsions rack $n's body!",FALSE,owner,ring,ch,TO_VICT);
    return FALSE;
  }

  if(ring->equipped_by && !IS_NPC(ring->equipped_by))  owner = ring->equipped_by;

  if(ch==owner && cmd==CMD_REMOVE) {
    if(ch!=owner) return FALSE;
    if(!ring->equipped_by || owner!=ring->equipped_by) return FALSE;
    one_argument(arg,buf);
    if(!isname(buf,OBJ_NAME(ring))) return FALSE;
    act("Your finger detaches with a spray of blood as you remove the Ring of Agony!",FALSE,owner,0,0,TO_CHAR);
    act("$n loses $s finger as $e removes the Ring of Agony.",FALSE,owner,0,0,TO_NOTVICT);
    if(IS_MORTAL(owner)) damage(owner,owner,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
    if(ring == EQ(owner,WEAR_FINGER_L)) remeq(owner,WEAR_FINGER_L);
    else if(ring == EQ(owner,WEAR_FINGER_R)) remeq(owner,WEAR_FINGER_R);
    owner=0;
    return TRUE;
  }

  if(cmd==MSG_TICK) {
    if(ring->equipped_by) {
      if(OBJ_SPEC(ring) >0) OBJ_SPEC(ring)--;
      return FALSE;
    }
    return FALSE;
  }

  if(cmd==MSG_MOBACT && ring->equipped_by) {
    vict=ring->equipped_by;

    if(OBJ_SPEC(ring) == 60 || OBJ_SPEC(ring) == 50) {
      act("Your body is wracked with unreal agony!",0,vict,0,0,TO_CHAR);
      act("$n screams in extreme agony!",0,vict,0,0,TO_NOTVICT);
      return FALSE;
    }
    if(OBJ_SPEC(ring) == 0 && vict->specials.fighting && !IS_NPC(vict)) {
      act("Your $p falls to the ground, taking your severed digit with it!",0,vict,ring,0,TO_CHAR);
      act("$n's $p falls to the ground taking $s finger with it!",0,vict,ring,0,TO_NOTVICT);
      damage(vict,vict,332,TYPE_UNDEFINED,DAM_NO_BLOCK);
      if(ring == EQ(vict,WEAR_FINGER_L)) remeq(vict,WEAR_FINGER_L);
      else if(ring == EQ(vict,WEAR_FINGER_R)) remeq(vict,WEAR_FINGER_R);
      OBJ_SPEC(ring)=counter;
      owner=0;
      return FALSE;
    }
    if(OBJ_SPEC(ring) <= 20 && vict->specials.fighting && OBJ_SPEC(ring) > 0 && chance(3)) {
      act("You are stricken with ever-increasing agony!",0,vict,0,0,TO_CHAR);
      act("$n arches $s back in extreme agony!",0,vict,0,0,TO_NOTVICT);
      damage(vict,vict,40,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
    }
    return FALSE;
  }
  return FALSE;
}


int Iron_Boots (OBJ *boot, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(!ch) return FALSE;
  if(!boot->equipped_by) return FALSE;
  if(ch!=boot->equipped_by) return FALSE;
  if(IS_NPC(ch)) return FALSE;

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if((cmd==CMD_EAST || cmd==CMD_NORTH ||
      cmd==CMD_WEST || cmd==CMD_SOUTH ||
      cmd==CMD_UP || cmd==CMD_DOWN) && chance(1)) {
    act("You are terribly encumbered by your $p.",0,ch,boot,0,TO_CHAR);
    act("$n is held back by $s heavy iron boots.",0,ch,0,0,TO_NOTVICT);
    GET_MOVE(ch)=1;
    return TRUE;
  }
  //Skeena 6/12/11: boots melt if used against Djinns in museum
  if(cmd==CMD_KICK && (world[CHAR_REAL_ROOM(ch)].number == 16558 || world[CHAR_REAL_ROOM(ch)].number == 16564)) {
    return FALSE;
  }

  if (cmd == CMD_KICK)
  {
    if(!ch->specials.fighting) return FALSE; /* added 06/03/02 */
    vict = ch->specials.fighting;
    if(!vict) return FALSE;

    if(number(1,4) == 1) {
      act("You send $N a fierce kick in the gut!",0,ch,0,vict,TO_CHAR);
      act("$n's $p collides horribly with your stomach!",0,ch,boot,vict,TO_VICT);
      act("$n delivers a furious kick to $N's gut!",0,ch,0,vict,TO_NOTVICT);
      damage(ch,vict,GET_LEVEL(ch)*2.5,TYPE_UNDEFINED,DAM_NO_BLOCK);
      WAIT_STATE(ch,PULSE_VIOLENCE*2);
    }
    else
    { // Skeena 6/12/11: lag them if kick fails
      act("Your kick misses its target!",0,ch,0,vict,TO_CHAR);
      act("$n's $p go flying past you as $e misses $s kick!",0,ch,boot,vict,TO_VICT);
      act("$n fans the air near $N's body!",0,ch,0,vict,TO_NOTVICT);
      WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }
    return TRUE;
  }
  return FALSE;
}


void append_text_room(int room, char *text, int type);

int Hooves_Satan (OBJ *hooves, CHAR *ch, int cmd, char *arg) {
  CHAR *owner;

  if(!(owner=hooves->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(owner)) {
    act("......$n's cloven hooves pound the earth into submission!",FALSE,owner,hooves,ch,TO_VICT);
    return FALSE;
  }

  if(!ch) return FALSE;
  if(ch != hooves->equipped_by) return FALSE;
  if(IS_NPC(ch)) return FALSE;

  if(ch==hooves->equipped_by && cmd==MSG_ENTER && chance(1)) {

    if((strlen(world[CHAR_REAL_ROOM(ch)].description)+strlen(" There is a nasty hoofprint here."))>MSL) return FALSE;
    if(strstr(world[CHAR_REAL_ROOM(ch)].description,"There is a nasty hoofprint here.")) return FALSE;
    act("\n\rYour $p collide viciously with the ground leaving a mark!",0,ch,hooves,0,TO_CHAR);
    act("\n\r$n's $p crash into the ground leaving a deep print!",0,ch,hooves,0,TO_NOTVICT);
    append_text_room(V_ROOM(ch),"There is a nasty hoofprint here.",VIRTUAL);
    return FALSE;
  }
  return FALSE;
}


int Achromatic_Gem (OBJ *gem, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *holder, *vict;
  struct char_data *victim;
  char name[240];
  char *skip_spaces (char*);
  char buf[MIL], buf3[MIL], targ[MIL];
  OBJ *obj;
  int spl;

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    if(!(owner=gem->equipped_by)) return FALSE;
    if (!IS_NPC(owner)) return FALSE;
    act("......$n holds an unfathomable Evil in $s hands!",FALSE,owner,gem,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_OBJ_ENTERING_GAME) {
    if(!gem->equipped_by) return FALSE;
    if(gem->equipped_by) {
      owner=gem->equipped_by;
      send_to_char("\n\rThe gem pulsates as it floats into your hands.\n\r",owner);
      return FALSE;
    }
    return FALSE;
  }

  if(ch && cmd==CMD_RENT) {
    if(gem->equipped_by && gem->equipped_by==ch) return FALSE;
    if(gem->carried_by && gem->carried_by==ch) {
      send_to_char("\n\rThe gem pulsates angrily in your hands.\n\rYou cannot rent with the gem unequipped.\n\r",ch);
      return TRUE;
    }
    return FALSE;
  }

  owner=gem->carried_by;

  if(ch==owner && cmd==CMD_STORE && owner==gem->carried_by) {
    if(!gem->carried_by || ch!=gem->carried_by) return FALSE;
    one_argument(arg,buf3);
    string_to_lower(buf3);
    obj=get_obj_in_list_vis(owner,buf3,owner->carrying);
    if(!obj) return FALSE;
    if(obj==gem) {
      act("The Achromatic Gem tells you 'You cannot store me.'",0,owner,0,0,TO_CHAR);
      return TRUE;
    }
    return FALSE;
  }

  if(ch==owner && cmd==CMD_GRAB && owner==gem->carried_by) {
    one_argument(arg,buf3);
    string_to_lower(buf3);
    obj=get_obj_in_list_vis(owner,buf3,owner->carrying);
    if(!obj) return FALSE;
    if(owner->equipment[17]) return FALSE; /* if holding something already, don't try */
    if(GET_ALIGNMENT(owner)>-351) {
        send_to_char("The gem screams in fury as you attempt to hold it.\n\r",owner);
        damage(owner,owner,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
        return TRUE;     /* if person is good or neut, they can't hold */
    }

    if(gem->obj_flags.value[1] == 0) {
      send_to_char("The gem pulsates as it floats into your hands.\n\r",owner);
      gem->obj_flags.value[1]=owner->ver3.id; /* gem id becomes owner id */
      return FALSE;
    }
    else {
      if(owner->ver3.id == gem->obj_flags.value[1]) {
        return FALSE;    /* if owner id is = obj id, can wear */
      }
      else {
        send_to_char("The gem screams in fury as you attempt to hold it.\n\r",owner);
        damage(owner,owner,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
        return TRUE;     /* if owner id isn't obj id, can't wear */
      }
    }

    return FALSE;
  }

  if(!gem->equipped_by) return FALSE;
  holder=gem->equipped_by;
  if(IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

                               /* actual gem spec, using OBJ_SPEC and class as determinant */
  if(cmd==MSG_TICK && gem->equipped_by && holder->specials.fighting && OBJ_SPEC(gem)>6) {
    if(gem->equipped_by!=holder) return FALSE;
    if(chance(33)) return FALSE;
    if(GET_CLASS(holder)==CLASS_PALADIN) return FALSE;
    if(OBJ_SPEC(gem)<7 && GET_CLASS(holder)==CLASS_NOMAD) return FALSE;
    if(OBJ_SPEC(gem)<8 && (GET_CLASS(holder)==CLASS_BARD || GET_CLASS(holder)==CLASS_NINJA)) return FALSE;
    if(OBJ_SPEC(gem)<10 && GET_CLASS(holder)==CLASS_CLERIC) return FALSE;
    if(OBJ_SPEC(gem)<12 && (GET_CLASS(holder)==CLASS_COMMANDO || GET_CLASS(holder)==CLASS_ANTI_PALADIN)) return FALSE;
    if(OBJ_SPEC(gem)<13 && GET_CLASS(holder)==CLASS_MAGIC_USER) return FALSE;
    if(OBJ_SPEC(gem)<15 && GET_CLASS(holder)==CLASS_WARRIOR) return FALSE;
    if(OBJ_SPEC(gem)<18 && GET_CLASS(holder)==CLASS_THIEF) return FALSE;

    vict=holder->specials.fighting;

    act("$p implodes in your hands, damaging $N with its evil power!",FALSE,holder,gem,vict,TO_CHAR);
    act("$p implodes in $n's hands, damaging you with its evil power!",FALSE,holder,gem,vict,TO_VICT);
    act("$p implodes with a sonic blast, damaging $N with its evil power!",FALSE,holder,gem,vict,TO_NOTVICT);
    damage(holder,vict,number(600,732),TYPE_UNDEFINED,DAM_NO_BLOCK);

    if(GET_CLASS(holder)==CLASS_NOMAD) OBJ_SPEC(gem)-=7;
    if(GET_CLASS(holder)==CLASS_BARD || GET_CLASS(holder)==CLASS_NINJA) OBJ_SPEC(gem)-=8;
    if(GET_CLASS(holder)==CLASS_CLERIC) OBJ_SPEC(gem)-=10;
    if(GET_CLASS(holder)==CLASS_COMMANDO || GET_CLASS(holder)==CLASS_ANTI_PALADIN) OBJ_SPEC(gem)-=12;
    if(GET_CLASS(holder)==CLASS_MAGIC_USER) OBJ_SPEC(gem)-=13;
    if(GET_CLASS(holder)==CLASS_WARRIOR) OBJ_SPEC(gem)-=15;
    if(GET_CLASS(holder)==CLASS_THIEF) OBJ_SPEC(gem)-=18;

    return FALSE;
  }

  if(ch==holder && cmd==MSG_SPELL_CAST) {

    if(GET_CLASS(holder)==CLASS_THIEF ||
       GET_CLASS(holder)==CLASS_WARRIOR ||
       GET_CLASS(holder)==CLASS_NOMAD ||
       GET_CLASS(holder)==CLASS_PALADIN) return FALSE;
    if(!*arg) return FALSE;

    argument_interpreter(arg, buf, targ);

    if(!is_number(buf)) return FALSE;
    spl=atoi(buf);
    if(*targ) {   /* if there is a target arg */
      one_argument(targ,buf);
      if(!(victim = get_char_room_vis(holder,buf))) return FALSE;
      if(!IS_NPC(victim)) return FALSE;
    }
      switch(spl) {

      case SPELL_THUNDERBALL:
        if(GET_CLASS(holder)!=CLASS_MAGIC_USER) return FALSE;
        if(!holder->specials.fighting) return FALSE;
        if(IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, SAFE)&& (!CHAOSMODE)) return FALSE;
        victim=holder->specials.fighting;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case SPELL_VAMPIRIC:
        if(GET_CLASS(holder)!=CLASS_MAGIC_USER &&
           GET_CLASS(holder)!=CLASS_ANTI_PALADIN &&
           GET_CLASS(holder)!=CLASS_COMMANDO) return FALSE;
        if(!holder->specials.fighting) return FALSE;
        if(IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, SAFE)&& (!CHAOSMODE)) return FALSE;
        victim=holder->specials.fighting;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case SPELL_HELL_FIRE:
        if(GET_CLASS(holder)!=CLASS_NINJA &&
           GET_CLASS(holder)!=CLASS_ANTI_PALADIN) return FALSE;
        if(!holder->specials.fighting) return FALSE;
        if(IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, SAFE)&& (!CHAOSMODE)) return FALSE;
        victim=holder->specials.fighting;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case SPELL_EVIL_WORD:
      case SPELL_DISPEL_GOOD:
        if(GET_CLASS(holder)!=CLASS_CLERIC &&
           GET_CLASS(holder)!=CLASS_ANTI_PALADIN) return FALSE;
        if(!holder->specials.fighting) return FALSE;
        if(IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, SAFE)&& (!CHAOSMODE)) return FALSE;
        victim=holder->specials.fighting;
        if(chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case SPELL_LETHAL_FIRE:
        if(GET_CLASS(holder)!=CLASS_MAGIC_USER &&
           GET_CLASS(holder)!=CLASS_COMMANDO) return FALSE;
        if(!holder->specials.fighting) return FALSE;
        if(IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, SAFE)&& (!CHAOSMODE)) return FALSE;
        victim=holder->specials.fighting;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case SPELL_FEAR:
        if(GET_CLASS(holder)!=CLASS_BARD) return FALSE;
        if(!holder->specials.fighting) return FALSE;
        if(IS_SET(world[CHAR_REAL_ROOM(holder)].room_flags, SAFE)&& (!CHAOSMODE)) return FALSE;
        victim=holder->specials.fighting;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=2,20);
        }
        break;

      case SPELL_HEAL:
        if(GET_CLASS(holder)!=CLASS_CLERIC &&
           GET_CLASS(holder)!=CLASS_NINJA) return FALSE;
        OBJ_SPEC(gem)=MAX(OBJ_SPEC(gem)-=1,0);
        if (chance(33)) {
          send_to_char("You lost your concentration!\n\r", holder);
          WAIT_STATE(holder,PULSE_VIOLENCE*2);
          return TRUE;
        }
        break;

      case SPELL_SANCTUARY:
        if(GET_CLASS(holder)!=CLASS_CLERIC) return FALSE;
        if(holder->specials.fighting) return FALSE;
        OBJ_SPEC(gem)=MAX(OBJ_SPEC(gem)-=1,0);
        if (chance(33)) {
          send_to_char("You lost your concentration!\n\r", holder);
          WAIT_STATE(holder,PULSE_VIOLENCE*2);
          return TRUE;
        }
        break;

      case SPELL_MIRACLE:
        if(GET_CLASS(holder)!=CLASS_CLERIC) return FALSE;
        OBJ_SPEC(gem)=MAX(OBJ_SPEC(gem)-=1,0);
        if (chance(33)) {
          send_to_char("You lost your concentration!\n\r", holder);
          WAIT_STATE(holder,PULSE_VIOLENCE*2);
          return TRUE;
        }
        break;

      default:
        break;

      }/* end switch */

    return FALSE;
  }/* end if cast */

  if(ch==holder && cmd) {

    switch(cmd) {

      case CMD_CIRCLE:
        if(!holder->equipment[WIELD]) return FALSE;
        if(GET_CLASS(holder)!=CLASS_THIEF) return FALSE;
        if(!holder->specials.fighting) return FALSE;
        victim=holder->specials.fighting;
        if(!IS_NPC(victim)) return FALSE;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case CMD_BACKSTAB:
        if(!holder->equipment[WIELD]) return FALSE;
        one_argument(arg, name);
        if(GET_CLASS(holder)!=CLASS_THIEF &&
           GET_CLASS(holder)!=CLASS_ANTI_PALADIN) return FALSE;
        if(holder->specials.fighting) return FALSE;
        if (!(victim = get_char_room_vis(holder, name))) return FALSE;
        if(!IS_NPC(victim)) return FALSE;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case CMD_PUNCH:
        if(GET_CLASS(holder)!=CLASS_WARRIOR) return FALSE;
        if(!holder->specials.fighting) return FALSE;
        victim=holder->specials.fighting;
        if(!IS_NPC(victim)) return FALSE;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case CMD_DISEMBOWEL:
        if(!holder->equipment[WIELD]) return FALSE;
        if(GET_CLASS(holder)!=CLASS_NOMAD) return FALSE;
        if(!holder->specials.fighting) return FALSE;
        victim=holder->specials.fighting;
        if(!IS_NPC(victim)) return FALSE;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case CMD_ASSAULT:
        if(!holder->equipment[WIELD]) return FALSE;
        one_argument(arg,name);
        if(GET_CLASS(holder)!=CLASS_NINJA &&
           GET_CLASS(holder)!=CLASS_COMMANDO) return FALSE;
        if(holder->specials.fighting) return FALSE;
        if (!(victim = get_char_room_vis(holder, name))) return FALSE;
        if(!IS_NPC(victim)) return FALSE;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      case CMD_AMBUSH:
        if(!holder->equipment[WIELD]) return FALSE;
        one_argument(arg,name);
        if(GET_CLASS(holder)!=CLASS_NOMAD) return FALSE;
        if(holder->specials.fighting) return FALSE;
        if (!(victim = get_char_room_vis(holder, name))) return FALSE;
        if(!IS_NPC(victim)) return FALSE;
        if (chance(33)) {
          act("The achromatic gem pulsates as it gains power.",FALSE,holder,0,0,TO_CHAR);
          OBJ_SPEC(gem)=MIN(OBJ_SPEC(gem)+=1,20);
        }
        break;

      default:
        break;

    } /* end switch */

    return FALSE;
  } /* end if cmd */

  return FALSE;
}


int Lucifer_Sword (OBJ *sword, CHAR *ch, int cmd, char *arg) {
  CHAR *owner;

  if(!(owner=sword->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n is surrounded by a shroud of starlight!",FALSE,owner,sword,ch,TO_VICT);
    return FALSE;
  }

  return FALSE;
}


int Lucifer_Horns (OBJ *horns, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict, *holder;
  int room;

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    if(!(owner=horns->equipped_by)) return FALSE;
    if(time_info.hours<6) {
      act("......$n spurts fire and brimstone from the Horns atop $s head!",FALSE,owner,horns,ch,TO_VICT);
    } else {
      act("......$n commands a mighty presence with the Horns atop $s head!",FALSE,owner,horns,ch,TO_VICT);
    }
    return FALSE;
  }

  holder=horns->carried_by;

  if(ch==holder && cmd==CMD_WEAR && holder==horns->carried_by) {
    if(time_info.hours<6) OBJ_SPEC(horns)=1;
    else OBJ_SPEC(horns)=0;
    return FALSE;
  }

  if(!(owner=horns->equipped_by)) return FALSE;

  if(cmd==MSG_TICK && horns->equipped_by) {
    if(time_info.hours==0 && OBJ_SPEC(horns)==0) {
      room=CHAR_REAL_ROOM(owner);
      send_to_room("\n\rA Blazing Set of Horns burn bright orange with intense heat as the Witching Hour commences.\n\r",room);
      if(!IS_SET(horns->obj_flags.bitvector,AFF_INFRAVISION))
        SET_BIT(horns->obj_flags.bitvector,AFF_INFRAVISION);
      OBJ_SPEC(horns)=1;
    }
    if(time_info.hours==6 && OBJ_SPEC(horns)==1) {
      room=CHAR_REAL_ROOM(owner);
      send_to_room("\n\rA Blazing Set of Horns extinguish with a great puff of smoke as daylight touches them.\n\r",room);
      if(IS_SET(horns->obj_flags.bitvector,AFF_INFRAVISION))
        REMOVE_BIT(horns->obj_flags.bitvector,AFF_INFRAVISION);
      OBJ_SPEC(horns)=0;
    }
    return FALSE;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(cmd==MSG_MOBACT && horns->equipped_by && owner->specials.fighting && time_info.hours<6 && chance(15)) {
    vict=get_random_vict(owner);
    if(!vict) return FALSE;
    act("$n gores you with $s giant flaming horns!",FALSE,owner,horns,vict,TO_VICT);
    act("$n gores $N with $s giant flaming horns!",FALSE,owner,horns,vict,TO_NOTVICT);
    act("You gore $N with your giant flaming horns!",FALSE,owner,horns,vict,TO_CHAR);
    if(IS_MORTAL(vict))
      damage(vict,vict,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
    else if(IS_MOB(vict))
      damage(owner,vict,166,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }
  return FALSE;
}


int Ring_Accursed (OBJ *ring, CHAR *ch, int cmd, char *arg) {
  CHAR *owner;

  if(!(owner=ring->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(owner)) {
    act("......$n's fist is clenched in a crushing grip!",FALSE,owner,ring,ch,TO_VICT);
    return FALSE;
  }
  return FALSE;
}


int Ring_Despair (OBJ *ring, CHAR *ch, int cmd, char *arg) {
  CHAR *owner;
  char buf[MIL];

  if(!(owner=ring->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(owner)) {
    act("......a tear passes silently down $n's cheek.",FALSE,owner,ring,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_TICK && weather_info.sky>=SKY_RAINING && !IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, INDOORS)
  	&& (world[CHAR_REAL_ROOM(owner)].sector_type != SECT_INSIDE) && chance(66) ) {
    if(GET_HIT(owner)==GET_MAX_HIT(owner)) return FALSE;
    act("You soak up the rain and gain power from the Despair within!",FALSE,owner,ring,0,TO_CHAR);
    act("$n soaks up the rain and gains power from the Despair within!",FALSE,owner,ring,0,TO_ROOM);
    GET_HIT(owner)=MIN(GET_HIT(owner)+=66,GET_MAX_HIT(owner));
    return FALSE;
  }

  if(ch==owner && cmd==CMD_REMOVE && IS_DAY) {
    if(!ring->equipped_by || owner!=ring->equipped_by) return FALSE;
    one_argument(arg,buf);
    if(!isname(buf,OBJ_NAME(ring))) return FALSE;
    act("You are too apathetic in your state of depression to remove the ring..",FALSE,owner,0,0,TO_CHAR);
    act("$n tries pathetically to remove $s ring, to no avail.",FALSE,owner,0,0,TO_NOTVICT);
    return TRUE;
  }

  if(ch==owner && cmd==CMD_SLEEP && IS_DAY) {
    send_to_char("\n\rYour feelings of despair overcome you so that you cannot sleep..\n\r",owner);
    return TRUE;
  }
  return FALSE;
}

void check_equipment(CHAR *ch);
int Inverted_Cross (OBJ *cross, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict, *next_vict;

  if(!(owner=cross->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(owner)) {
    act("......$n wears the sigil of the Unspeakable One!",FALSE,owner,cross,ch,TO_VICT);
    return FALSE;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(cmd==MSG_MOBACT) {
    if(owner->specials.fighting  && chance(3)) {
      if(chance(50)) return FALSE;
      for(vict=world[CHAR_REAL_ROOM(owner)].people;vict;vict=next_vict) {
        next_vict=vict->next_in_room;
        if(GET_ALIGNMENT(vict)>0 && chance(33)) {
          act("$n's $p channels the power of Lucifer into you!",FALSE,owner,cross,vict,TO_VICT);
          act("$n's $p channels the power of Lucifer into $N!",FALSE,owner,cross,vict,TO_NOTVICT);
          act("Your $p channels the power of Lucifer into $N!",FALSE,owner,cross,vict,TO_CHAR);
          if(!strcmp(GET_NAME(vict),"Infinity")) {
            divide_experience(owner,vict,1);
            raw_kill(vict);
            WAIT_STATE(vict,PULSE_VIOLENCE*10);
          }
          if(IS_MORTAL(vict)) {
            damage(vict,vict,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
            GET_ALIGNMENT(vict)=MAX(GET_ALIGNMENT(vict)-=100,-1000);
            check_equipment(vict);   /* Linerfix 110203 */
          } else if(IS_MOB(vict)) {
            damage(owner,vict,166,TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
        }
      }
      return FALSE;

    } else {
      if(count_mortals_room(owner,TRUE) < 2) return FALSE;
      vict=get_random_vict(owner);
      if(vict && chance(1)) {
        act("$n's $p channels the power of Lucifer into you!",FALSE,owner,cross,vict,TO_VICT);
        act("$n's $p channels the power of Lucifer into $N!",FALSE,owner,cross,vict,TO_NOTVICT);
        act("Your $p channels the power of Lucifer into $N!",FALSE,owner,cross,vict,TO_CHAR);
        if(!strcmp(GET_NAME(vict),"Infinity")) {
          divide_experience(owner,vict,1);
          raw_kill(vict);
          WAIT_STATE(vict,PULSE_VIOLENCE*10);
        }
        if(IS_MORTAL(vict)) GET_ALIGNMENT(vict)=MAX(GET_ALIGNMENT(vict)-=100,-1000);
        check_equipment(vict);  /*  Linerfix 110203 */
      }
    }
    return FALSE;
  }
  return FALSE;
}


int Lucifer_Wings (OBJ *wings, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict;

  if(!(owner=wings->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(owner)) {
    act("......$n spreads $s fated dark wings around you!",FALSE,owner,wings,ch,TO_VICT);
    return FALSE;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(cmd==MSG_MOBACT) {
    if(owner->specials.fighting && chance(6)) {
      vict=get_random_vict(owner);
      if(!vict) return FALSE;
      act("$n's wings unfurl and beat heavily, creating a whirlwind that assails $N!",FALSE,owner,0,vict,TO_NOTVICT);
      act("$n's wings unfurl and beat heavily, creating a whirlwind that pummels you senseless!",FALSE,owner,0,vict,TO_VICT);
      act("Your wings unfurl and beat heavily, creating a whirlwind around $N!",FALSE,owner,0,vict,TO_CHAR);
      if(IS_MORTAL(vict))
        damage(vict,vict,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      else if(IS_MOB(vict))
        damage(owner,vict,266,TYPE_UNDEFINED,DAM_NO_BLOCK);
      if(chance(66)) GET_POS(vict)=POSITION_STUNNED;
      return FALSE;
    }
    return FALSE;
  }
  return FALSE;
}


int Daemonic_Barrier (OBJ *barrier, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict;

  if(!(owner=barrier->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT && IS_NPC(owner)) {
    act("......$n is encased in a din of daemonic anguish!",FALSE,owner,barrier,ch,TO_VICT);
    return FALSE;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(cmd==MSG_MOBACT && owner->specials.fighting && chance(6)) {
    vict=get_random_vict(owner);
    if(!vict) return FALSE;
    if(IS_NPC(vict) && ((GET_LEVEL(owner) < GET_LEVEL(vict)) || IS_SET(vict->specials.act,ACT_SENTINEL))) return FALSE;
    act("$p screams with horrid laughter, causing $N to flee in fear!",FALSE,owner,barrier,vict,TO_NOTVICT);
    act("$p screams with horrid laughter, causing you to flee in fear!",FALSE,owner,barrier,vict,TO_VICT);
    act("Your Daemonic Barrier screams with horrid laughter, causing $N to flee in fear!",FALSE,owner,barrier,vict,TO_CHAR);
    do_flee(vict," ",CMD_FLEE);
    return FALSE;
  }
  return FALSE;
}


int Shadow_Moon (OBJ *shadow, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict;

  if(!(owner=shadow->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n appears dim beneath a crimson veil of darkness!",FALSE,owner,shadow,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_MOBACT && IS_NIGHT && chance(6)) {
    if(chance(10)) {
      if(count_mortals_room(owner,TRUE) < 2) return FALSE;
      vict=get_random_vict(owner);
      if(!vict) return FALSE;
      if(IS_AFFECTED(vict, AFF_IMINV)) return FALSE;
      act("A tendril of your Shadow reaches out and taints $N with its Evil.",FALSE,owner,shadow,vict,TO_CHAR);
      act("A tendril of $n's Shadow reaches out and taints $N with its Evil.",FALSE,owner,shadow,vict,TO_NOTVICT);
      act("A tendril of the $p has tainted you with pure Evil.",FALSE,owner,shadow,vict,TO_VICT);
      cast_imp_invisibility(GET_LEVEL(owner), owner, "", SPELL_TYPE_SPELL, vict, 0);
      return FALSE;
    }
    return FALSE;
  }
  return FALSE;
}


int Serpent_Tail (OBJ *tail, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict;
  struct descriptor_data *d;

  if(!(owner=tail->equipped_by)) return FALSE;

  d=owner->desc;
  if(!d) return FALSE;
  if(d->connected!=CON_PLYNG) return FALSE; /* If char died */

  if(cmd==MSG_MOBACT && chance(1)) {
    if(chance(50)) return FALSE;
    switch(number(1,3)) {
      case 1:
      if(count_mortals_room(owner,TRUE) < 2) break;
      vict=get_random_vict(owner);
      if(!vict) break;
      act("$p suddenly comes to life and rattles intimidatingly in your face!",FALSE,owner,tail,vict,TO_VICT);
      act("$p suddenly comes to life and rattles intimidatingly in $N's face!",FALSE,owner,tail,vict,TO_NOTVICT);
      act("$p suddenly comes to life and rattles intimidatingly in $N's face!",FALSE,owner,tail,vict,TO_CHAR);
      break;

      case 2:
      act("$p gets angry and ventures to find a way up your nose!",FALSE,owner,tail,0,TO_CHAR);
      act("$n's $p gets angry and decides to tunnel up $s left nostril!",FALSE,owner,tail,0,TO_ROOM);
      break;

      case 3:
      act("$p gets angry and ventures to find a way up your nose!",FALSE,owner,tail,0,TO_CHAR);
      act("$n's $p gets angry and decides to tunnel up $s right nostril!",FALSE,owner,tail,0,TO_ROOM);
      break;

      default:
      break;

    }
    return FALSE;
  }
  return FALSE;
}


int Circlet (OBJ *circlet, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict;
  char buf[MIL];

  if(ch==circlet->carried_by && cmd==CMD_WEAR && chance(50)) {
    one_argument(arg, buf);
    if(isname(buf,OBJ_NAME(circlet)) || !strcmp(buf, " all")) {
      if(ch->equipment[14] && ch->equipment[15]) return FALSE;  /* both wrists full */
      act("Your wrist begins to weep from your fresh wounds.",FALSE,ch,0,0,TO_CHAR);
      act("$n's wrist begins to weep from $s fresh wounds.",FALSE,ch,0,0,TO_ROOM);
      damage(ch,ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  if(!(owner=circlet->equipped_by)) return FALSE;
  if(IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(cmd==MSG_MOBACT && chance(9) && owner->specials.fighting) {
    switch(number(1,4)) {
      case 1:
      vict=get_random_vict(owner);
      if(!vict) break;
      act("$p gashes $N badly as you throw a roundhouse punch at $M!",FALSE,owner,circlet,vict,TO_CHAR);
      act("$p gashes $N badly as $n throws a roundhouse punch at $M!",FALSE,owner,circlet,vict,TO_NOTVICT);
      act("$p gashes you badly as $n throws a roundhouse punch at you!",FALSE,owner,circlet,vict,TO_VICT);
      if(IS_MORTAL(vict))
        damage(vict,vict,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      else if(IS_MOB(vict))
        damage(owner,vict,99,TYPE_UNDEFINED,DAM_NO_BLOCK);
      break;

      case 2:
      act("$p constricts around your wrist, letting fresh blood flow!",FALSE,owner,circlet,0,TO_CHAR);
      act("$n's Crescent Circlet constricts around $s wrist, letting fresh blood flow!",FALSE,owner,circlet,0,TO_ROOM);
      damage(owner,owner,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      break;

      case 3:
      break;

      case 4:
      break;

      default:
      break;

    }
    return FALSE;
  }
  return FALSE;
}


int Charon_Token(OBJ *obj ,CHAR *ch, int cmd, char *argument) {
  int room=3014;
  char buf[MIL];

  if(!ch) return FALSE;
  if(!obj) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(CHAOSMODE) return FALSE;

  if(cmd == CMD_USE) {

    if(!EQ(ch,HOLD) || EQ(ch,HOLD) != obj) return FALSE;
    one_argument(argument, buf);
    if(!isname(buf,OBJ_NAME(obj))) return FALSE;

    if(V_OBJ(obj) == TOKEN_A) {
      room=25318;
      extract_obj(unequip_char(ch,HOLD));
    }

    if(V_OBJ(obj) == TOKEN_B) {
      room=25445;
      extract_obj(unequip_char(ch,HOLD));
    }

    if(V_OBJ(obj) == TOKEN_C) {
      room=25434;
      extract_obj(unequip_char(ch,HOLD));
    }

    if(V_OBJ(obj) == TOKEN_D) {
      room=25516;
      extract_obj(unequip_char(ch,HOLD));
    }

    act("\n\rCharon appears and whisks you away to the depths of hell on his boat.\n\r",1,ch,0,0,TO_CHAR);
    act("\n\rCharon appears and whisks $n away to the depths of hell on his boat.\n\r",1,ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, real_room(room));
    do_look(ch,"",CMD_LOOK);
    act("\n\rCharon takes your Token and disappears in a fine cloud of smoke.\n\r",1,ch,0,0,TO_CHAR);
    act("\n\rCharon appears with $n, and disappears just as quickly with Token in hand.\n\r",1,ch,0,0,TO_ROOM);
    return TRUE;

  }
  return FALSE;
}


    /* Hell Mob Specs */

void check_equipment(CHAR *ch);
int fayn(CHAR *mob,CHAR *ch, int cmd, char *arg) {
  CHAR *victim;
  char buf[MIL];
  struct descriptor_data *d;

  if(cmd==CMD_UP  && IS_MORTAL(ch)) {
    block(ch,mob);
    return TRUE;
  }

  if (IS_MORTAL(ch) && cmd == CMD_WEAR && !(strcmp(arg, " all"))) {
    act("\n\rA strange force prohibits you from doing that here.",FALSE,ch,0,0,TO_CHAR);
    return TRUE;
  }

  if(ch && cmd==CMD_STEAL) {  /* nosteal fix */
    if(GET_LEVEL(ch)>49) return FALSE;
    if(GET_CLASS(ch)!=CLASS_THIEF) return FALSE;
    sprintf(buf, "emote flays the infidel thief with his giant fingernail!");
    command_interpreter (mob, buf);
    GET_HIT(ch)=-6;
    return TRUE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;

  if(mob->specials.fighting) {

    if((3*GET_MAX_HIT(mob)/4) >= GET_HIT(mob)) {   /* if fayn is below 75% hp, SHIELD */
      if(!IS_SET(mob->specials.act, ACT_SHIELD)) SET_BIT(mob->specials.act,ACT_SHIELD);
    } else { /* if fayn is above 75% hp, UNSHIELD */
      if(IS_SET(mob->specials.act, ACT_SHIELD)) REMOVE_BIT(mob->specials.act,ACT_SHIELD);
    }

    victim=get_random_victim_fighting(mob);
    if(!victim) return FALSE;
    if(IS_NPC(victim)) return FALSE;
    if(CHAR_REAL_ROOM(victim) != CHAR_REAL_ROOM(mob)) return FALSE;
    if(GET_CLASS(victim)==CLASS_PALADIN && affected_by_spell(victim,SPELL_FURY)) {
      do_say(mob, "Pathetic paladins with all of their rage!", 0);
      act("$n taps you lightly on the head with $s enormous fingernail.\nYou calm down.",1,mob,0,victim,TO_VICT);
      act("$n taps $N lightly on the head with $s enormous fingernail.",1,mob,0,victim,TO_NOTVICT);
      act("You tap $N lightly on the head with your fingernail and calm $M down.",1,mob,0,victim,TO_CHAR);
      d=victim->desc;
      if(d && d->connected==CON_PLYNG) {
        affect_from_char(victim, SPELL_FURY);
      }
    }
    else if(IS_GOOD(victim)) {
      do_say(mob, "Don't you realize that this is far too deep for your kind to be?", 0);
      act("$n brings an evil looking cloud down over you!",1,mob,0,victim,TO_VICT);
      act("An evil looking cloud descends over $N then dissipates!",1,mob,0,victim,TO_NOTVICT);
      act("Those stinky holy characters really get you down.",1,mob,0,victim,TO_CHAR);
      d=victim->desc;
      if(d && d->connected==CON_PLYNG) {
        GET_ALIGNMENT(victim) = -351;
        check_equipment(victim); /*  Linerfix 110203 */
      }
    }
    if(chance(33)) Slice(mob,victim);
  }
  return FALSE;
}


int grim_reaper(CHAR *reaper, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(world[CHAR_REAL_ROOM(reaper)].number!=25301) return FALSE;
  if(!reaper->specials.fighting) return FALSE;
  vict=reaper->specials.fighting;
  if(cmd!=MSG_MOBACT) return FALSE;
  if(GET_POS(vict)==POSITION_DEAD) return FALSE;
  if(IS_NPC(vict)) return FALSE;

  switch (number(1,2)) {
    case 1:
      if(vict && GET_HIT(vict)>2) {
        act("$n's scythe swings easily through your neck!",0,reaper,0,vict,TO_VICT);
        act("That Really did HURT!",0,reaper,0,vict,TO_VICT);
        act("$N suddenly becomes a quivering corpse as $n severs $N's head with $s scythe.",0,reaper,0,vict,TO_NOTVICT);
        act("$N is dead! R.I.P.",0,reaper,0,vict,TO_NOTVICT);
        act("The Reaper appears and escorts $N to the afterlife!",0,reaper,0,vict,TO_NOTVICT);
        act("Your blood freezes as you hear $N's death cry.",0,reaper,0,vict,TO_NOTVICT);
        act("$n demands $s toll for your passage to the planes of hell.",0,reaper,0,vict,TO_VICT);
        act("Your soul burns as $e tears some lifeforce from you.",0,reaper,0,vict,TO_VICT);
        act("You send another soul to the planes of hell.",0,reaper,0,vict,TO_CHAR);
        if(vict->specials.riding)
          stop_riding(vict,vict->specials.riding);
        char_from_room(vict);
        char_to_room(vict, real_room(VESTIBULE));
        act("$n hits the floor with a THUD!",1,vict,0,0,TO_ROOM);
        GET_HIT(vict)=1;
        GET_MOVE(vict)=1;
        GET_MANA(vict)=1;
        do_look(vict, "",CMD_LOOK);
      }
      break;

   case 2:
      if(vict && GET_HIT(vict)>2) {
        act("$n swings $s black scythe through your body,  You feel your guts spill!",1,reaper,0,vict,TO_VICT);
        act("That Really did HURT!",1,reaper,0,vict,TO_VICT);
        act("The Reaper considers your soul and intones 'May you burn in hell!'",1,reaper,0,vict,TO_VICT);
        act("$n swings $s scythe clean through $N!  You retch at the spray of gore.",1,reaper,0,vict,TO_NOTVICT);
        act("$N is dead! R.I.P.",1,reaper,0,vict,TO_NOTVICT);
        act("The Reaper appears and escorts $N to the afterlife!",1,reaper,0,vict,TO_NOTVICT);
        act("Your blood freezes as you hear $N's death cry.",1,reaper,0,vict,TO_NOTVICT);
        act("Your soul burns as $e tears some lifeforce from you.",1,reaper,0,vict,TO_VICT);
        act("You send another soul to the planes of hell.",1,reaper,0,vict,TO_CHAR);
        if(vict->specials.riding)
          stop_riding(vict,vict->specials.riding);
        char_from_room(vict);
        char_to_room(vict, real_room(VESTIBULE));
        act("$n hits the floor with a THUD!",1,vict,0,0,TO_ROOM);
        GET_HIT(vict)=1;
        GET_MANA(vict)=1;
        GET_MOVE(vict)=1;
        do_look(vict, "",CMD_LOOK);
      }
      break;

    default:
      break;
  }
  return FALSE;
}


int minos(CHAR *minos, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  OBJ *tmp_obj;

  if(cmd==CMD_DOWN && IS_MORTAL(ch)) {
    if(!ch) return FALSE;
    if(!(vict=ch)) return FALSE;
    act("$n prevents $N from leaving by smashing $M in the chest with $s tail!",0,minos,0,vict,TO_NOTVICT);
    act("Just before you can cross the threshold, $n whips his tail at you and catches you square in the chest!",0,minos,0,vict,TO_VICT);
    act("Your tail smashes into $N's chest with a dull THUD!",0,minos,0,vict,TO_CHAR);
    if(saves_spell(vict,SAVING_SPELL,GET_LEVEL(minos)))
      damage(minos,vict,150,TYPE_UNDEFINED,DAM_NO_BLOCK);
    else
      damage(minos,vict,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
    if(number(1,25) > GET_CON(vict))
      spell_poison(GET_LEVEL(minos), vict, vict, 0);
    return TRUE;
  }

  if(cmd==MSG_MOBACT && minos->specials.fighting) {

    if((3*GET_MAX_HIT(minos)/4) >= GET_HIT(minos)) {   /* if minos is below 75% hp */
      if(!IS_SET(minos->specials.act, ACT_SHIELD)) SET_BIT(minos->specials.act,ACT_SHIELD);
    } else { /* if minos is above 75% hp */
      if(IS_SET(minos->specials.act, ACT_SHIELD)) REMOVE_BIT(minos->specials.act,ACT_SHIELD);
    }

    vict=get_random_victim_fighting(minos);
    if(!vict) return FALSE;

    switch(number(1,5)) {

      case 1:
      case 2:
          do_say(minos, "O tu che vieni al doloroso ospizio, guarda com' entri e di cui tu ti fide;", 0);
          do_say(minos, "Non t'inganni l'ampiezza de l'intrare!", 0);
          break;

      case 3:
      case 4:
          act("$n swings $s huge spiked tail, and catches $N in the face!",0,minos,0,vict,TO_NOTVICT);
          act("$n's huge spiked tail imbeds itself squarely in your face!",0,minos,0,vict,TO_VICT);
          act("Your tail smashes into $N with a satisfying WHUMP!",0,minos,0,vict,TO_CHAR);
          if(saves_spell(vict,SAVING_SPELL,GET_LEVEL(minos)))
            damage(minos,vict,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
          else
            damage(minos,vict,700,TYPE_UNDEFINED,DAM_NO_BLOCK);
          if(number(1,27) > GET_CON(vict)) spell_poison(GET_LEVEL(minos), vict, vict, 0);
          if(number(1,27) > GET_CON(vict)) spell_blindness(GET_LEVEL(minos), vict, vict, 0);
          break;

      case 5:
          act("$n smashes a section of the wall, and it falls on $N!",0,minos,0,vict,TO_NOTVICT);
          act("The breath is forced from your lungs as a pile of rock lands on you!",0,minos,0,vict,TO_VICT);
          act("You smash a section of the wall with your huge tail!",0,minos,0,vict,TO_CHAR);
          if(saves_spell(vict,SAVING_SPELL,GET_LEVEL(minos)))
            damage(minos,vict,700,TYPE_UNDEFINED,DAM_NO_BLOCK);
          else
            damage(minos,vict,1000,TYPE_UNDEFINED,DAM_NO_BLOCK);
          if(number(1,25) > GET_DEX(vict)) {
            stop_fighting(vict);
            WAIT_STATE(vict,PULSE_VIOLENCE*4);
          }
          break;

      default:
          break;
    }
    return FALSE;
  }

  if (minos && cmd == MSG_DIE) {
    tmp_obj = read_object(25322,VIRTUAL); /* load up a key */
    obj_to_char(tmp_obj, minos); /* stick it on Minos */
  }
  return FALSE;
}


int Charybdis(CHAR *dis, CHAR *ch, int cmd, char *arg) {
  CHAR *victim, *next_vict;
  int num, f, specnum, i;
  struct descriptor_data *d;

  if(dis && cmd==MSG_DIE) {
    for (victim = world[CHAR_REAL_ROOM(dis)].people; victim; victim = next_vict) {
      next_vict = victim->next_in_room;
      if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(dis)))
        damage(victim,victim,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
      else
        damage(victim,victim,800,TYPE_UNDEFINED,DAM_NO_BLOCK);
      act("$n's head explodes into fiery fragments!",1,dis,0,victim,TO_VICT);
      d=victim->desc;
      if(d && d->connected==CON_PLYNG) {
        WAIT_STATE(victim, PULSE_VIOLENCE*3);
      }
    }
    return FALSE;
  }

  if(cmd==CMD_CRAWL && IS_MORTAL(ch)) {
    if(!ch) return FALSE;
    if(!(victim=ch)) return FALSE;
    act("$n gestures, and a wall of flame envelops you!",0,dis,0,victim,TO_VICT);
    act("$n gestures and a wall of flame envelops $N!",0,dis,0,victim,TO_NOTVICT);
    act("You turn $N into a walking torch as $E tries to leave.",0,dis,0,victim,TO_CHAR);
    if(chance(50) && GET_HIT(victim)<275 && GET_HIT(victim)>1) {
      GET_HIT(victim)=1;
      WAIT_STATE(victim, PULSE_VIOLENCE);
      return TRUE;
    }
    if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(dis)))
      damage(dis,victim,number(235,350),TYPE_UNDEFINED,DAM_NO_BLOCK);
    else
      damage(dis,victim,number(370,600),TYPE_UNDEFINED,DAM_NO_BLOCK);
    WAIT_STATE(victim, PULSE_VIOLENCE);
    return TRUE;
  }

  if(count_mortals_room(dis, TRUE) < 1) return FALSE;

  if(GET_POS(dis)==POSITION_STANDING && chance(50)) {
    if(cmd) return FALSE;
    switch(number(0,2)) {
      case 0: act("$n lights his cigar with the touch of a fingertip.\n\r",FALSE,dis,0,0,TO_ROOM);
              break;
      case 1: do_say(dis, "What brings you fools to your death?", 0);
              break;
      case 2: do_say(dis, "Prepare to burn in the eternal fires of Hell!", 0);
              break;
      default:
              break;
    }
    return FALSE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;

  if(dis->specials.fighting) {
    if((3*GET_MAX_HIT(dis)/4) <= GET_HIT(dis)) {   /* if is above 75% hp */
      if(!IS_SET(dis->specials.act, ACT_SHIELD)) SET_BIT(dis->specials.act,ACT_SHIELD);
    } else { /* if is below 75% hp */
      if(IS_SET(dis->specials.act, ACT_SHIELD)) REMOVE_BIT(dis->specials.act,ACT_SHIELD);
    }

    f=number(1,100);
    if(f<60) {
      if(f<25)
        specnum=3;
      else
        specnum=2;
    }
    else {
      specnum=1;
    }

    for(i = 0; i < specnum; i++) {
      num=number(0,6);
      switch(num) {
        case 0:
          victim=get_random_victim_fighting(dis);
          if(victim && victim!=dis && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(dis)) {
            act("$n points a finger at $N and issues forth a stream of flame!",1,dis,0,victim,TO_NOTVICT);
            act("$n points a finger at you!",1,dis,0,victim,TO_VICT);
            act("You point a finger at $N!",1,dis,0,victim,TO_CHAR);
            if(chance(50) && GET_HIT(victim)<250 && GET_HIT(victim)>1) {
               GET_HIT(victim)=1;
               WAIT_STATE(victim, PULSE_VIOLENCE);
               break;
            }
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(dis)))
              damage(dis,victim,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(dis,victim,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        case 1:
          victim=get_random_victim_fighting(dis);
          if(victim && victim!=dis && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(dis)) {
            act("$n raises his hands and a wall of fire encompasses $N!",1,dis,0,victim,TO_NOTVICT);
            act("$n raises his hands and you are surrounded by fire!",1,dis,0,victim,TO_VICT);
            act("You raise your hands and $N is surrounded by fire!",1,dis,0,victim,TO_CHAR);
            if(chance(40) && GET_HIT(victim)<300 && GET_HIT(victim)>1) {
              GET_HIT(victim)=1;
              WAIT_STATE(victim, PULSE_VIOLENCE);
              stop_fighting(victim);
              break;
            }
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(dis)))
              damage(dis,victim,333,TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(dis,victim,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
            d=victim->desc;
            if(d && d->connected==CON_PLYNG)
              stop_fighting(victim);
          }
          break;
        case 2:
          victim=get_random_victim_fighting(dis);
          if(victim && victim!=dis && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(dis)) {
            act("$n swears loudly and clutches $N with burning hands!",1,dis,0,victim,TO_NOTVICT);
            act("$n clutches at you with his burning hands!",1,dis,0,victim,TO_VICT);
            act("You swear loudly at $N and burn $M with your hands!",1,dis,0,victim,TO_CHAR);
            if(chance(30) && GET_HIT(victim)<300 && GET_HIT(victim)>1) {
              GET_HIT(victim)=1;
              WAIT_STATE(victim, PULSE_VIOLENCE);
              break;
            }
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(dis)))
              damage(dis,victim,520,TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(dis,victim,649,TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        case 3:
          victim=get_random_victim_fighting(dis);
          if(victim && victim!=dis && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(dis) && GET_HIT(victim)>150) {
            if(victim!=dis->specials.fighting && IS_MORTAL(victim)) {
              act("$n utters the words 'gaeburae'. \nA blast of flame pushes $N out of the tower!",1,dis,0,victim,TO_NOTVICT);
              act("$n utters the words 'gaeburae'. \nA blast of heat forces you to retreat from the tower!",1,dis,0,victim,TO_VICT);
              act("You utter the words 'begone'.",1,dis,0,victim,TO_CHAR);
              if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(dis)))
                damage(dis,victim,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
              else
                damage(dis,victim,660,TYPE_UNDEFINED,DAM_NO_BLOCK);
              d=victim->desc;
              if(d && d->connected==CON_PLYNG) {
                if(victim->specials.riding) stop_riding(victim,victim->specials.riding);
                char_from_room(victim);
                char_to_room(victim, real_room(25332));
                stop_fighting(victim);
              }
            }
          }
          break;
        case 4:
          act("With a flourish of $s cape, $n melts the floor beneath your feet!",1,dis,0,0,TO_ROOM);
          act("With a flourish of your cape, you melt the floor beneath!",1,dis,0,0,TO_CHAR);
          for (victim = world[CHAR_REAL_ROOM(dis)].people; victim; victim = next_vict) {
            next_vict = victim->next_in_room;
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(dis)))
              damage(dis,victim,275,TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(dis,victim,350,TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        case 5:
          victim=get_random_victim_fighting(dis);
          if(victim && victim!=dis && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(dis)) {
            act("$n pokes $N with $s stogie!",1,dis,0,victim,TO_NOTVICT);
            act("$n jams a lit cigar in your face!",1,dis,0,victim,TO_VICT);
            act("You decide to share your cigar with $N up close.",1,dis,0,victim,TO_CHAR);
            if(chance(60) && GET_HIT(victim)<110 && GET_HIT(victim)>1) {
              GET_HIT(victim)=1;
              WAIT_STATE(victim, PULSE_VIOLENCE);
              break;
            }
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(dis)))
              damage(dis,victim,220,TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(dis,victim,440,TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        case 6:
          victim=get_random_victim_fighting(dis);
          if(victim && victim!=dis && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(dis)) {
            act("$n cackles insanely and throws a ball of fire at $N!",1,dis,0,victim,TO_NOTVICT);
            act("A ball of fire slams into you!",1,dis,0,victim,TO_VICT);
            act("You cackle as you toss a fireball at $N!",1,dis,0,victim,TO_CHAR);
            if(chance(25) && GET_HIT(victim)<350 && GET_HIT(victim)>1) {
              GET_HIT(victim)=1;
              WAIT_STATE(victim, PULSE_VIOLENCE);
              break;
            }
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(dis)))
              damage(dis,victim,450,TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(dis,victim,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        default:
          break;
      }
    }
  }
  return FALSE;
}


int Virtuous_Pagan(CHAR *s, CHAR *ch, int cmd, char *arg) {
  if(count_mortals_room(s, TRUE) < 1) return FALSE;
  if(cmd) return FALSE;
  if(GET_POS(s)==POSITION_STANDING || GET_POS(s)==POSITION_FIGHTING)
    switch(number(0,11)) {
      case 0: do_say(s, "In what direction does the way out lie?", 0);
              break;
      case 1: do_say(s, "Why do you invade a place not meant for you?", 0);
              break;
      case 2: do_say(s, "We do not want you here!", 0);
              break;
      case 3: do_say(s, "You must leave!", 0);
              break;
      case 4: do_say(s, "Why have you come to this place?", 0);
              break;
      case 5: do_say(s, "We had our chance.  We didn't take it.", 0);
              break;
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
              break;

      default:
              break;
    }
   return FALSE;
}


int Bridge_Guard(CHAR *s, CHAR *ch, int cmd, char *arg) {
  CHAR *victim;
  struct descriptor_data *d;
  int room;

  if (cmd==CMD_EAST && IS_MORTAL(ch)) {
    block(ch,s);
    return TRUE;
  }

  if (cmd==CMD_DOWN && IS_MORTAL(ch)) {
    block(ch,s);
    return TRUE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;

  if(s->specials.fighting) {
    if((5*GET_MAX_HIT(s)/6) >= GET_HIT(s)) {   /* if is below 80% hp */
      if(!IS_SET(s->specials.act, ACT_SHIELD)) SET_BIT(s->specials.act,ACT_SHIELD);
    } else { /* if is above 80% hp */
      if(IS_SET(s->specials.act, ACT_SHIELD)) REMOVE_BIT(s->specials.act,ACT_SHIELD);
    }

    victim=get_random_victim_fighting(s);
    if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {

      if(chance(70) && /* victim!=s->specials.fighting && */ IS_MORTAL(victim)) {
        act("$n snarls 'Into the pit, pig!'",1,s,0,victim,TO_ROOM);
        act("$n throws $N into the pit.",1,s,0,victim,TO_NOTVICT);
        act("$n throws you into the pit.",1,s,0,victim,TO_VICT);
        act("You throw $N into the pit below!",1,s,0,victim,TO_CHAR);
        damage(victim,victim,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
        d=victim->desc;
        if(d && d->connected==CON_PLYNG) {
          stop_fighting(victim);
          if(victim->specials.riding) stop_riding(victim,victim->specials.riding);
          room=CHAR_REAL_ROOM(s)+10;
          if(!room) return FALSE;
          char_from_room(victim);
          char_to_room(victim, room);
          act("$n drops to the floor of the Pit before you!",FALSE,victim,0,0,TO_ROOM);
          do_look(victim,"",CMD_LOOK);
          GET_POS(victim)=POSITION_SITTING;
        }
        if(CHAR_REAL_ROOM(victim)==real_room(ADDICT_ROOM)) {
          act("\n\rYou scream incoherently as a demon appears, slices your ring fingers off, and flies away laughing!\n\r",FALSE,victim,0,0,TO_CHAR);
          act("\n\r$n screams $s head off as a demon appears and slices $s fingers off!",FALSE,victim,0,0,TO_ROOM);
          damage(victim,victim,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
          d=victim->desc;
          if(d && d->connected==CON_PLYNG) {
            if(EQ(victim,WEAR_FINGER_R)) remeq(victim, WEAR_FINGER_R);
            if(EQ(victim,WEAR_FINGER_L)) remeq(victim, WEAR_FINGER_L);
          }
          return FALSE;
        }
        if(CHAR_REAL_ROOM(victim)==real_room(SPIKE_ROOM)) {
          act("\n\rYou are impaled by a large spike that narrowly misses any vital organs.\n\r",FALSE,victim,0,0,TO_CHAR);
          act("\n\r$n is impaled by a large spike and twitches just above the floor.",FALSE,victim,0,0,TO_ROOM);
          GET_HIT(victim)=(GET_HIT(victim)/2);
          WAIT_STATE(victim, PULSE_VIOLENCE*15);
          return FALSE;
        }
      }
    }
  }
  return FALSE;
}


int Geryon(CHAR *s, CHAR *ch, int cmd, char *arg) {
  CHAR *victim, *next_vict;
  OBJ *wield;
  char buf[MIL];
  int num, i, f, specnum;
  struct descriptor_data *d;

  if(ch && cmd==CMD_CLIMB && IS_MORTAL(ch)) {
    block(ch,s);
    return TRUE;
  }

  if(count_mortals_room(s, TRUE) < 1) return FALSE;

  if(GET_POS(s)==POSITION_STANDING && chance(50)) {
    if(cmd) return FALSE;
    switch(number(0,3)) {
      case 0:
        do_say(s, "Hello smeghead, you must be from Newfoundland.", 0);
        break;
      case 1:
        do_say(s, "I pity the fool who wishes to go further.", 0);
        break;
      case 2:
        act("$n paces back and forth impatiently.",FALSE,s,0,0,TO_ROOM);
        break;
      case 3:
        act("$n hoists his belt and momentarily rises from the ground.",FALSE,s,0,0,TO_ROOM);
        break;
      default:
        break;
    }
    return FALSE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;

  if(s->specials.fighting) {
    if((GET_MAX_HIT(s)/2) >= GET_HIT(s)) {   /* if is below 50% hp */
      if(!IS_SET(s->specials.act, ACT_SHIELD)) SET_BIT(s->specials.act,ACT_SHIELD);
    } else { /* if is above 50% hp */
      if(IS_SET(s->specials.act, ACT_SHIELD)) REMOVE_BIT(s->specials.act,ACT_SHIELD);
    }

    f=number(1,100);
    if(f<50) {
      if(f<15)
        specnum=3;
      else
        specnum=2;
    }
    else {
      specnum=1;
    }

    for(i = 0; i < specnum; i++) {
      num=number(0,6);
      switch(num) {
        case 0:
          victim=get_random_victim_fighting(s);
          if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {
            act("$n whips $N with his belt!",1,s,0,victim,TO_NOTVICT);
            act("$n lashes out at you with his wide leather belt!",1,s,0,victim,TO_VICT);
            act("You whip $N with your belt!",1,s,0,victim,TO_CHAR);
            if(chance(30) && GET_HIT(victim)<250 && GET_HIT(victim)>1) {
              GET_HIT(victim)=1;
              WAIT_STATE(victim, PULSE_VIOLENCE);
              break;
            }
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(s)))
              damage(s,victim,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(s,victim,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        case 1:
          victim=get_random_victim_fighting(s);
          if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {
            if(victim!=s->specials.fighting &&  IS_MORTAL(victim)) {
              act("$n flaps his wings and forces $N out of the fight!",1,s,0,victim,TO_NOTVICT);
              act("You are blown back by the force of $n's wings!",1,s,0,victim,TO_VICT);
              act("You flap your wings and force $N out of the fight!",1,s,0,victim,TO_CHAR);
              if(chance(30) && GET_HIT(victim)<175 && GET_HIT(victim)>1) {
                GET_HIT(victim)=1;
                WAIT_STATE(victim, PULSE_VIOLENCE);
                stop_fighting(victim);
                break;
              }
              if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(s)))
                damage(s,victim,250,TYPE_UNDEFINED,DAM_NO_BLOCK);
              else
                damage(s,victim,350,TYPE_UNDEFINED,DAM_NO_BLOCK);
              d=victim->desc;
              if(d && d->connected==CON_PLYNG) {
                stop_fighting(victim);
                WAIT_STATE(victim, PULSE_VIOLENCE);
              }
            }
          }
          break;
        case 2:
          victim=get_random_victim_fighting(s);
          if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {
            if((GET_HIT(victim)>350) || ((GET_HIT(victim)<=350) && chance(50))) {
              if(EQ(victim, WIELD)) {
                wield = victim->equipment[WIELD];
                if(V_OBJ(wield)!=11523) {
                  act("$n bites $N's arm forcing $M to drop $S $p!",1,s,wield,victim,TO_NOTVICT);
                  act("$n bites your arm and you drop your $p.",1,s,wield,victim,TO_VICT);
                  act("You bite $N's arm and force $M to drop $S $p!",1,s,wield,victim,TO_CHAR);
                  if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(s)))
                    damage(s,victim,350,TYPE_UNDEFINED,DAM_NO_BLOCK);
                  else
                    damage(s,victim,550,TYPE_UNDEFINED,DAM_NO_BLOCK);
                }

               d=victim->desc;
                if(d && d->connected==CON_PLYNG) {
                  sprintf(buf,"Hell Log: %s disarms %s's %s (Room %d).",GET_NAME(s),GET_NAME(victim),OBJ_SHORT(wield),world[CHAR_REAL_ROOM(victim)].number);
                  log_s(buf);
                  unequip_char(victim, WIELD);
                  obj_to_room(wield, CHAR_REAL_ROOM(victim));
                  wield->log=1;
                  save_char(victim, NOWHERE);
                  WAIT_STATE(victim, PULSE_VIOLENCE);
                }
              }
            }
          }
          break;
        case 3:
          victim=get_random_victim_fighting(s);
          if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {
            if((GET_HIT(victim)>350) || ((GET_HIT(victim)<=350) && chance(50))) {
              if(/* victim!=s->specials.fighting && */ IS_MORTAL(victim)) {
                act("With a mighty heave, $n grabs $N and pushes $M out of the castle!",1,s,0,victim,TO_NOTVICT);
                act("You are pushed out of the castle by $n!",1,s,0,victim,TO_VICT);
                act("You heave $N out of the castle!",1,s,0,victim,TO_CHAR);
                if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(s)))
                  damage(s,victim,number(100,500),TYPE_UNDEFINED,DAM_NO_BLOCK);
                else
                  damage(s,victim,number(200,700),TYPE_UNDEFINED,DAM_NO_BLOCK);
                d=victim->desc;
                if(d && d->connected==CON_PLYNG) {
                  if(victim->specials.riding) stop_riding(victim,victim->specials.riding);
                  char_from_room(victim);
                  char_to_room(victim, real_room(25431));
                  stop_fighting(victim);
                  WAIT_STATE(victim, PULSE_VIOLENCE*3);
                }
              }
            }
          }
          break;
        case 4:
          act("$n begins frothing at the mouth and bites madly at your party!",1,s,0,0,TO_ROOM);
          act("You bite the hell out of everyone around you !!",1,s,0,0,TO_CHAR);
          for(victim = world[CHAR_REAL_ROOM(s)].people; victim; victim = next_vict) {
            next_vict=victim->next_in_room;
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(s)))
              damage(s,victim,number(200,400),TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(s,victim,number(300,600),TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        case 5:
          victim=get_random_victim_fighting(s);
          if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {
            act("$n kicks $N in the groin!",1,s,0,victim,TO_NOTVICT);
            act("$n kicks you in the midsection!",1,s,0,victim,TO_VICT);
            act("You kick $N in the groin!",1,s,0,victim,TO_CHAR);
            if(chance(20) && GET_HIT(victim)<225 && GET_HIT(victim)>1) {
              GET_HIT(victim)=1;
              WAIT_STATE(victim, PULSE_VIOLENCE);
              break;
            }
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(s)))
              damage(s,victim,number(100,275),TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(s,victim,number(200,350),TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        case 6:
          victim=get_random_victim_fighting(s);
          if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {
            act("$n reaches for $N with his clawed hand and rips at $N's throat!",1,s,0,victim,TO_NOTVICT);
            act("$n reaches out for you and tears at your throat!",1,s,0,victim,TO_VICT);
            act("You reach out and claw at $N's throat!",1,s,0,victim,TO_CHAR);
            if(GET_HIT(victim)>625) {
              GET_HIT(victim)=1;
              WAIT_STATE(victim, PULSE_VIOLENCE*2);
              break;
            }
            if(saves_spell(victim, SAVING_SPELL,GET_LEVEL(s)))
              damage(s,victim,666,TYPE_UNDEFINED,DAM_NO_BLOCK);
            else
              damage(s,victim,1222,TYPE_UNDEFINED,DAM_NO_BLOCK);
            d=victim->desc;
            if(d && d->connected==CON_PLYNG)
              WAIT_STATE(victim, PULSE_VIOLENCE*2);
          }
          break;
        default:
          break;
      }
    }
  }
  return FALSE;
}


int Lemure(CHAR *s, CHAR *ch, int cmd, char *arg) {
  if((cmd==CMD_FLEE || cmd==CMD_EAST || cmd==CMD_NORTH || cmd==CMD_WEST || cmd==CMD_SOUTH) && IS_MORTAL(ch)) {
    block(ch,s);
    return TRUE;
  }
 return FALSE;
}


int Ciacco(CHAR *s, CHAR *ch, int cmd, char *arg) {
  char buf[MIL];

  if(ch && cmd==MSG_ENTER && IS_MORTAL(ch) && !s->specials.fighting) {
    do_say(s, "Come to me, valentine.  You will make a tender morsel..", 0);
    sprintf(buf, "emote dribbles saliva from his many chins.");
    command_interpreter (s, buf);
    set_fighting(s,ch);
    return FALSE;
  }

  if(cmd==CMD_CRAWL && IS_MORTAL(ch)) {
    block(ch,s);
    return TRUE;
  }
  return FALSE;
}


int water_weird(CHAR *weird,CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  char buf[MIL];

  if(cmd==CMD_EAST && IS_MORTAL(ch)) {
    block(ch,weird);
    return TRUE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;

  if(weird->specials.fighting && chance(50)) {
    vict = GET_OPPONENT(weird);
    if(!vict) return FALSE;
    act("$n coils its body around $N, and drags $M under the surface of the water!",1,weird,0,vict,TO_NOTVICT);
    act("$n constricts its body around you and drags you under the water!",1,weird,0,vict,TO_VICT);
    act("You easily force $N under the surface of the water.",1,weird,0,vict,TO_CHAR);
    damage(weird,vict,number(50,100),TYPE_UNDEFINED,DAM_NO_BLOCK);
    cast_blindness(GET_LEVEL(weird), weird,"", SPELL_TYPE_SPELL, vict, 0);
    WAIT_STATE(vict,PULSE_VIOLENCE*2);
    sprintf(buf, "emote pops to the surface, gasping frantically for air.");
    command_interpreter(vict,buf);
  }

  return FALSE;
}


int Wind_Devil(CHAR *s, CHAR *ch, int cmd, char *arg) {
  CHAR *victim;
  int num, i, f, specnum;
  struct descriptor_data *d;

  if (cmd==CMD_CLIMB && IS_MORTAL(ch)) {
    block(ch,s);
    return TRUE;
  }

  if(count_mortals_room(s, TRUE) < 1) return FALSE;

  if(GET_POS(s)==POSITION_STANDING && chance(50)) {
    if(cmd) return FALSE;
    switch(number(0,3)) {
      case 0:
        do_say(s, "Thieves!  Pah!  You're pathetic.", 0);
        if((victim=get_random_victim(s))) {
          if(IS_NPC(victim)) break;
          if(GET_CLASS(victim)!=CLASS_THIEF) break;
          set_fighting(s,victim);
        }
        break;
      case 1:
        do_say(s, "You'll never pass here!", 0);
        break;
      case 2:
        do_say(s, "Leave before I kill you.", 0);
        break;
      case 3:
        do_say(s, "You fools have no idea what you're getting into!", 0);
        break;
      default:
        break;
    }
    return FALSE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;

  if(s->specials.fighting) {
    if((3*GET_MAX_HIT(s)/4) >= GET_HIT(s)) {   /* if is below 75% hp */
      if(!IS_SET(s->specials.act, ACT_SHIELD)) SET_BIT(s->specials.act,ACT_SHIELD);
    } else { /* if is above 75% hp */
      if(IS_SET(s->specials.act, ACT_SHIELD)) REMOVE_BIT(s->specials.act,ACT_SHIELD);
    }

    f=number(1,100);
    if(f<50) {
      if(f<15)
        specnum=3;
      else
        specnum=2;
    }
    else {
      specnum=1;
    }

    for(i = 0; i < specnum; i++) {
      num=number(0,3);
      switch(num) {
        case 0:
          victim=get_random_victim_fighting(s);
          if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {
            act("$n flings $N against the rock face!",1,s,0,victim,TO_NOTVICT);
            act("$n slams you into the cliff.",1,s,0,victim,TO_VICT);
            act("You fling $N against the rock face!",1,s,0,victim,TO_CHAR);
            damage(s,victim,350,TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        case 1:
          victim=get_random_victim_fighting(s);
          if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {
            act ("$n takes a deep breath and blasts $N with a huge jet of air!",FALSE,s,0,victim,TO_NOTVICT);
            act ("$N goes flying through the air!!",FALSE,s,0,victim,TO_NOTVICT);
            act ("$n blasts you with a powerful breath and you go flying through the air!",FALSE,s,0,victim,TO_VICT);
            act ("You hurl $N's pathetic shell off the cliff.",FALSE,s,0,victim,TO_CHAR);
            damage(s,victim,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
            d=victim->desc;
            if(d && d->connected==CON_PLYNG) {
              if(victim->specials.riding) stop_riding(victim,victim->specials.riding);
              char_from_room (victim);
              char_to_room (victim, real_room (FALL_ROOM));
              do_look(victim,"",CMD_LOOK);
            }
          }
          break;
        case 2:
          victim=get_random_victim_fighting(s);
          if(victim && victim!=s && CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) {
            act("$n inhales and blows a gale at $N!",1,s,0,victim,TO_NOTVICT);
            act("$n spews gale force winds at you!",1,s,0,victim,TO_VICT);
            act("You inhale and blow a gale at $N!",1,s,0,victim,TO_CHAR);
            damage(s,victim,250,TYPE_UNDEFINED,DAM_NO_BLOCK);
          }
          break;
        case 3:
          break;
        default:
          break;
      }
    }
  }
  return FALSE;
}


int red_death(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {
  OBJ *tmp;

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n's color begins to return to $m as the disease finally subsides.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("You feel the weakening affects of the Red Death leaving your body.\n\r",ench_ch);
    return FALSE;
  }

  if(CHAR_REAL_ROOM(ench_ch)==real_room(PORTAL_ENTER) && cmd==CMD_NORTH) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("A magical force blocks your sinful being from entering this place.\n\r",ench_ch);
    return TRUE;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(ench_ch)].room_flags, CHAOTIC) && cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(ench_ch && cmd==MSG_MOBACT && chance(15)) {

    switch (number(1,5)) {
      case 1:
        act("$n collapses to the ground, $s body wracked with agony.",1,ench_ch,0,NULL,TO_ROOM);
	act("You collapse to the ground, unable to move and in extreme agony.",1,ench_ch,0,NULL,TO_CHAR);
        damage(ench_ch ,ench_ch ,number(35,95) , TYPE_UNDEFINED,DAM_NO_BLOCK);
        GET_POS(ench_ch)=POSITION_INCAP;
        WAIT_STATE(ench_ch,PULSE_VIOLENCE*4);
	break;
      case 2:
        break;
      case 3:
        act("You begin sweating profusely, strange visions appear in your mind.",1,ench_ch,0,NULL,TO_CHAR);
	act("$n suddenly pierces the silence with a frightening shriek!",1,ench_ch,0,NULL,TO_ROOM);
        GET_POS(ench_ch)=POSITION_STUNNED;
        WAIT_STATE(ench_ch,PULSE_VIOLENCE);
	break;
      case 4:
        break;
      case 5:
        if(!EQ(ench_ch,WIELD)) return FALSE;
        tmp=EQ(ench_ch,WIELD);
        act("You start shivering uncontrollably and drop your $p!",1,ench_ch,tmp,0,TO_CHAR);
	act("$n's body begins quivering uncontrollably, causing $m to drop $s $p!",1,ench_ch,tmp,0,TO_ROOM);
        WAIT_STATE(ench_ch,PULSE_VIOLENCE*2);
        remeq(ench_ch,WIELD);
        break;
      default:
        break;
    }
    return FALSE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n's skin glows with a red hue!",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_TICK && ench->duration==1) {
    act("$n lets out a loud scream, the disease beginning to release its grip.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("The disease finally starts to release its grip on you.\n\r",ench_ch);
  }
  return FALSE;
}


int diseased_creature(CHAR *mob ,CHAR *ch, int cmd, char *argument) {
  CHAR *vict;
  char buf[MIL];
  ENCH *tmp_enchantment;
  struct descriptor_data *d;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(mob->specials.fighting) {
    vict=get_random_victim_fighting(mob);
    if(vict==mob->specials.fighting && chance(33)) vict=get_random_victim_fighting(mob);
    if(vict && chance(33)) {
    d=vict->desc;
    if(!d) return FALSE;
    if(d->connected!=CON_PLYNG) return FALSE; /* If char died */
    act("You claw at $N's arm, infecting $M with your disease!",1,mob,0,vict,TO_CHAR);
    act("$n claws at $N's arm, infecting $M with its disease!",1,mob,0,vict,TO_NOTVICT);
    send_to_char("A Diseased Creature tears open your arm, infecting you with its disease!\n\r",vict);
    CREATE(tmp_enchantment, ENCH, 1);
    tmp_enchantment->name     = str_dup("Red Death");
    tmp_enchantment->duration = 30;
    tmp_enchantment->func     = red_death;
    enchantment_to_char(vict, tmp_enchantment, FALSE);
    sprintf(buf,"Hell Log Ench: [ %s just contracted Red Death at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
    log_s(buf);
    }
  }
  return FALSE;
}


int lizard_bite(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {
  CHAR *act_ch, *act_ch_next;

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n's body begins to shift back to normal, and $s scales begin falling off.",TRUE,ench_ch,0,0,TO_ROOM);
    send_to_char("Your body wrenches back to its original form, causing you considerable pain.\n\r",ench_ch);
    damage(ench_ch,ench_ch,number(20,35),TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(CHAR_REAL_ROOM(ench_ch)==real_room(PORTAL_ENTER) && cmd==CMD_NORTH) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("A magical force blocks your sinful being from entering this place.\n\r",ench_ch);
    return TRUE;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(ench_ch)].room_flags, CHAOTIC) && cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(ench_ch && cmd==MSG_MOBACT && chance(33)) {
    switch (number(1,5)) {
      case 1:
        act("$n flits $s tongue around in the air and catches a fly.",1,ench_ch,0,NULL,TO_ROOM);
	act("You spot a fly buzzing around in front of you, and whip your tongue at it to catch it.",1,ench_ch,0,NULL,TO_CHAR);
        gain_condition(ench_ch, THIRST, -2);
        gain_condition(ench_ch, FULL, -2);
        break;
      case 2:
        break;
      case 3:
        act("You suddenly feel very cold, lying on a nice warm rock would feel great right now.",1,ench_ch,0,NULL,TO_CHAR);
	act("$n curls up into a ball and lies down on a nice warm rock.",1,ench_ch,0,NULL,TO_ROOM);
        GET_POS(ench_ch)=POSITION_RESTING;
        WAIT_STATE(ench_ch,PULSE_VIOLENCE*2);
        break;
      case 4:
        break;
      case 5:
        for(act_ch=world[CHAR_REAL_ROOM(ench_ch)].people;act_ch;act_ch=act_ch_next) {
          act_ch_next = act_ch->next_in_room;
          if(act_ch!=ench_ch && CAN_SEE(ench_ch,act_ch) && chance(50)) {
            act("You hiss angrily at $N as $E moves towards you.",1,ench_ch,0,act_ch,TO_CHAR);
            act("$n lets out an angry hiss as $N moves near $m.",1,ench_ch,0,act_ch,TO_ROOM);
            act("As you move near $n, $e emits an angry hiss that seems directed at you!",1,ench_ch,0,act_ch,TO_VICT);
            return FALSE;
          }
          return FALSE;
        }
        break;
      default:
        break;
    }
    return FALSE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n is covered in slimy green lizard scales!",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_TICK && ench->duration==1) {
    act("$n lets out a loud scream, the disease beginning to release its grip.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("The disease finally starts to release its grip on you.\n\r",ench_ch);
  }
  return FALSE;
}


int lizard(CHAR *mob ,CHAR *ch, int cmd, char *argument) {
  CHAR *vict;
  char buf[MIL];
  ENCH *tmp_enchantment;
  struct descriptor_data *d;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(mob->specials.fighting) {
    vict=get_random_victim_fighting(mob);
    if(vict==mob->specials.fighting && chance(33)) vict=get_random_victim_fighting(mob);
    if(vict && chance(25)) {
      act("You bite $N on the leg!",FALSE,mob,0,vict,TO_CHAR);
      act("$n bites $N on the leg!",FALSE,mob,0,vict,TO_NOTVICT);
      act("$n bites you on the leg, taking a good chunk of flesh with it!",1,mob,0,ch,TO_VICT);
      damage(mob,vict,40,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=vict->desc;
      if(!d) return FALSE;
      if(d->connected!=CON_PLYNG) return FALSE; /* If char died */
      CREATE(tmp_enchantment, ENCH, 1);
      tmp_enchantment->name     = str_dup("Lizard Lycanthropy");
      tmp_enchantment->duration = 30;
      tmp_enchantment->func     = lizard_bite;
      enchantment_to_char(vict, tmp_enchantment, FALSE);
      act("$n begins to shrink and shift into a small lizard!",FALSE,vict,0,0,TO_ROOM);
      send_to_char("You feel rather strange, and suddenly realize you've shrunk and are covered in green scales!\n\r",vict);
      sprintf(buf,"Hell Log Ench: [ %s just contracted Lizard Lycanthropy at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
      log_s(buf);
      return FALSE;
    }
  }
  return FALSE;
}


int lake_guard(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  int roomnum;
  struct descriptor_data *d;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(mob->specials.fighting) {
    vict=get_random_victim_fighting(mob);
    if(vict==mob->specials.fighting && chance(33)) vict=get_random_victim_fighting(mob);
    if(vict && chance(66)) {
      roomnum = (CHAR_VIRTUAL_ROOM(vict));
      do_say(mob,"Get back where you belong!",0);
      act ("$n pushes $N hard, causing $M to lose $S footing and fall back into the lake!",FALSE,mob,0,vict,TO_NOTVICT);
      act ("$n pushes you hard, causing you to lose your footing and fall back into the lake!",FALSE,mob,0,vict,TO_VICT);
      act ("You push $N back into the boiling lake, haha what fun!",FALSE,mob,0,vict,TO_CHAR);
      damage(mob,vict,50,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=vict->desc;
      if(!d) return FALSE;
      if(d->connected!=CON_PLYNG) return FALSE; /* If char died */
      if(vict->specials.riding) stop_riding(vict,vict->specials.riding);

      switch(roomnum) {

      case 25418:
        char_from_room (vict);
        char_to_room (vict, real_room (25420));
        do_look(vict,"",CMD_LOOK);
        break;
      case 25419:
        if(number(1,2)==1) {
          char_from_room (vict);
          char_to_room (vict, real_room (25423));
          do_look(vict,"",CMD_LOOK);
        }
        else {
          char_from_room (vict);
          char_to_room (vict, real_room (25420));
          do_look(vict,"",CMD_LOOK);
        }
        break;
      case 25421:
        if(number(1,2)==1) {
          char_from_room (vict);
          char_to_room (vict, real_room (25420));
          do_look(vict,"",CMD_LOOK);
        }
        else {
          char_from_room (vict);
          char_to_room (vict, real_room (25425));
          do_look(vict,"",CMD_LOOK);
        }
        break;
      case 25422:
        char_from_room (vict);
        char_to_room (vict, real_room (25423));
        do_look(vict,"",CMD_LOOK);
        break;
      case 25426:
        char_from_room (vict);
        char_to_room (vict, real_room (25425));
        do_look(vict,"",CMD_LOOK);
        break;
      case 25430:
        if(number(1,2)==1) {
          char_from_room (vict);
          char_to_room (vict, real_room (25427));
          do_look(vict,"",CMD_LOOK);
        }
        else {
          char_from_room (vict);
          char_to_room (vict, real_room (25431));
          do_look(vict,"",CMD_LOOK);
        }
        break;
      case 25432:
        if(number(1,2)==1) {
          char_from_room (vict);
          char_to_room (vict, real_room (25429));
          do_look(vict,"",CMD_LOOK);
        }
        else {
          char_from_room (vict);
          char_to_room (vict, real_room (25431));
          do_look(vict,"",CMD_LOOK);
        }
        break;
      default:
        break;
      }
    }
  }
  return FALSE;
}


int murderer(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  struct descriptor_data *d;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(mob->specials.fighting) {
    vict=get_random_victim_fighting(mob);
    if(CHAR_REAL_ROOM(vict) !=(real_room(ISLAND))) return FALSE;
    if(vict && !IS_NPC(vict) && chance(33)) {
      act("$n screams 'Get off our island you bastard!'",FALSE,mob,0,vict,TO_ROOM);
      act ("$n pushes $N hard, causing $M to lose $S footing and fall back into the lake!",FALSE,mob,0,vict,TO_NOTVICT);
      act ("$n pushes you hard, causing you to lose your footing and fall back into the lake!",FALSE,mob,0,vict,TO_VICT);
      act ("You push $N back into the boiling lake, haha what fun!",FALSE,mob,0,vict,TO_CHAR);
      damage(mob,vict,75,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=vict->desc;
      if(!d) return FALSE;
      if(d->connected!=CON_PLYNG) return FALSE; /* If char died */
      if(vict->specials.riding) stop_riding(vict,vict->specials.riding);
      if(CHAR_REAL_ROOM(vict)==NOWHERE) return FALSE;

      switch(number(1,4)) {
        case 1:
          char_from_room (vict);
          char_to_room (vict, real_room (25427));
          do_look(vict,"",CMD_LOOK);
          break;
        case 2:
          char_from_room (vict);
          char_to_room (vict, real_room (25424));
          do_look(vict,"",CMD_LOOK);
          break;
        case 3:
          char_from_room (vict);
          char_to_room (vict, real_room (25429));
          do_look(vict,"",CMD_LOOK);
          break;
        case 4:
          char_from_room (vict);
          char_to_room (vict, real_room (25431));
          do_look(vict,"",CMD_LOOK);
          break;
        default:
          break;
      }
    }
  }
  return FALSE;
}


int glutton(CHAR *mob, CHAR *vict, int cmd, char *arg) {

  if(cmd==MSG_MOBACT && mob->specials.fighting) {
    vict=get_random_victim_fighting(mob);
    if(vict && chance(35)) {
      act("You lurch forward and crush $N under you.",1,mob,0,vict,TO_CHAR);
      act("$n lurches towards you with unusual agility and crushes you under $s massive bulk!",1,mob,0,vict,TO_VICT);
      act("$n lurches towards $N with unusual agility and crushes $M under $s massive bulk!",1,mob,0,vict,TO_NOTVICT);
      damage(mob, vict, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
  }
  return FALSE;
}


int Flame(CHAR *flame, CHAR *ch, int cmd, char *arg) {
  struct obj_data *paper, *anot;
  struct char_data *tmp, *next;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(flame->specials.fighting && chance(25)) {
    act("$n suddenly erupts into a brilliant ball of fire that envelops you!",FALSE,flame,0,0,TO_ROOM);

    for (paper = world[CHAR_REAL_ROOM(flame)].contents;paper;paper = anot) {
      anot = paper->next_content;
      if ((OBJ_TYPE(paper) == ITEM_SCROLL || OBJ_TYPE(paper) == ITEM_RECIPE) && !number(0,5)) {
        act("$p burns in bright and hot flames...",FALSE,flame,paper,0,TO_ROOM);
        act("$p burns in bright and hot flames...",FALSE,flame,paper,0,TO_CHAR);
        extract_obj(paper);
      }
      if (OBJ_TYPE(paper) == ITEM_POTION && !number(0,5)) {
        act("$p boils up in steam...",FALSE,flame,paper,0,TO_ROOM);
        act("$p boils up in steam...",FALSE,flame,paper,0,TO_CHAR);
        extract_obj(paper);
      }
    }

    for (tmp = world[CHAR_REAL_ROOM(flame)].people;tmp;tmp = next) {
      next = tmp->next_in_room;
      for (paper = tmp->carrying;paper;paper = anot) {
        anot = paper->next_content;
        if ((OBJ_TYPE(paper) == ITEM_SCROLL || OBJ_TYPE(paper) == ITEM_RECIPE) && !number(0,5)) {
	        act("$p burns in bright and hot flames...",FALSE,flame,paper,0,TO_ROOM);
	        act("$p burns in bright and hot flames...",FALSE,flame,paper,0,TO_CHAR);
	        extract_obj(paper);
        }
        if (OBJ_TYPE(paper) == ITEM_POTION && !number(0,5)) {
	        act("$p boils up in steam...",FALSE,flame,paper,0,TO_ROOM);
	        act("$p boils up in steam...",FALSE,flame,paper,0,TO_CHAR);
	        extract_obj(paper);
        }
        if (tmp != flame)
	        damage (flame, tmp, number(60,100), SPELL_SEARING_ORB,DAM_NO_BLOCK);
      }
    }
    extract_char(flame);
  }
  return FALSE;
}


int Sorceress_spec (CHAR *sor, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  int num;

  if(ch && cmd==CMD_NORTH && IS_MORTAL(ch)) {
    vict=ch;
    act("$n waves a strange form in the air above your head, disrupting your reality.",0,sor,0,vict,TO_VICT);
    act("$n waves a strange form in the air over $N's head, causing $M to disappear!",0,sor,0,vict,TO_NOTVICT);
    act("You wave about in the air, sending $N to the other side of the Woods.",0,sor,0,vict,TO_CHAR);
    if(vict->specials.riding) stop_riding(vict,vict->specials.riding);
    char_from_room (vict);
    char_to_room (vict, real_room(SORCERESS_B_ROOM));
    act("\n\rYou feel slightly nauseous..",1,vict,0,0,TO_CHAR);
    act("\n\r$n appears from a greenish cloud of mist.",1,vict,0,0,TO_ROOM);
    spell_poison(50,vict,vict,0);
    return TRUE;
  }

  if(cmd==MSG_ZONE_RESET && !sor->specials.fighting) {
    if(GET_HIT(sor)>466455)
      GET_HIT(sor)=466455;
    sor->points.max_hit=466455;  /* set to avg if sor isn't fighting on zone reset */
  }

  if(cmd!=MSG_MOBACT) return FALSE;
  if(sor->specials.fighting) {
    if(chance(25)) {
      if(GET_HIT(sor) < 66000) return FALSE; /* allow her to be wimpied (no change max) under 66000 hp */
      act("You draw power from the souls of the damned in the trees around you.",FALSE,sor,0,0,TO_CHAR);
      act("$n draws power from the souls of the damned in the trees around $m.",FALSE,sor,0,0,TO_ROOM);
      num=number(1,5)*GET_HIT(sor)/10;  /* num is 10,20,30,40,or 50% of sor's current hp */
      switch(number(1,2)) {
        case 1:
          sor->points.max_hit=GET_HIT(sor)+num; /* sets her max to hp+10% to 50%, makes wounded to fine */
          break;
        case 2:
          sor->points.max_hit=GET_HIT(sor)+1;  /* sets her max to her hp+1, makes full pretty much */
          break;
        default:
          break;
      }
    }
    if(GET_HIT(sor) >= 100000) {   /* if is above 25% hp */
      if(!IS_SET(sor->specials.act, ACT_SHIELD)) SET_BIT(sor->specials.act,ACT_SHIELD);
    } else { /* if is below 25% hp */
      if(IS_SET(sor->specials.act, ACT_SHIELD)) REMOVE_BIT(sor->specials.act,ACT_SHIELD);
    }
    return FALSE;
  }
  return FALSE;
}


int Sorceress_B_spec (CHAR *sor, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(ch && cmd==CMD_NORTH && IS_MORTAL(ch)) {
    vict=ch;
    act("$n waves a strange form in the air above your head, disrupting your reality.",0,sor,0,vict,TO_VICT);
    act("$n waves a strange form in the air over $N's head, causing $M to disappear!",0,sor,0,vict,TO_NOTVICT);
    act("You wave about in the air, sending $N to the other side of the Woods.",0,sor,0,vict,TO_CHAR);
    if(vict->specials.riding) stop_riding(vict,vict->specials.riding);
    act("\n\rYou feel slightly nauseous..",1,vict,0,0,TO_CHAR);
    act("\n\r$n is enveloped within a greenish cloud of mist.",1,vict,0,0,TO_ROOM);
    spell_poison(50,sor,vict,0);
    return TRUE;
  }
  return FALSE;
}


int Serpent_spec (CHAR *serpent, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  struct descriptor_data *d;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(serpent->specials.fighting && chance(18)) {
    vict=get_random_victim_fighting(serpent);
    if(!vict) return FALSE;
    act("$n impales you with $s venomous fangs!",0,serpent,0,vict,TO_VICT);
    act("$n impales $N with $s venomous fangs!",0,serpent,0,vict,TO_NOTVICT);
    act("You take a bite of $N's flesh, infusing $M with venom!",0,serpent,0,vict,TO_CHAR);
    damage(serpent,vict,666,TYPE_UNDEFINED,DAM_NO_BLOCK);
    d=vict->desc;
    if(d && d->connected==CON_PLYNG) {
      spell_poison(50,serpent,vict,0);
    }
    return FALSE;
  }
  return FALSE;
}


int Incubus_spec (CHAR *incubus, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(count_mortals_room(incubus, TRUE) < 1) return FALSE;
  if(cmd) return FALSE;
  if(chance(10)) {
    vict=get_random_vict(incubus);
    if(!vict) return FALSE;
    act("$n transfixes you with a salacious glare, undressing you with $s eyes..",0,incubus,0,vict,TO_VICT);
    act("$n transfixes $N with a salacious glare, undressing $M with $s eyes..",0,incubus,0,vict,TO_NOTVICT);
    act("You set your sights on $N; $E looks particularly sexy today..",0,incubus,0,vict,TO_CHAR);
    if(chance(35) && !IS_NPC(vict))
      hit(incubus,vict,TYPE_HIT);
    return FALSE;
  }
  return FALSE;
}


int Robe_spec (CHAR *robe, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  char buf[MIL];

  if(cmd!=MSG_MOBACT) return FALSE;
  if(robe->specials.fighting && chance(50)) {
    vict=get_random_vict(robe);
    if(!vict || IS_NPC(vict)) return FALSE;
    sprintf (buf, "say You will never leave this place, %s.  You belong here with us.", GET_NAME(vict));
    command_interpreter (robe, buf);
    if(chance(50)) {
      stop_fighting(robe);
      stop_fighting(vict);
      sprintf (buf, "follow %s", GET_NAME(vict));
      command_interpreter (robe, buf);
    }
    return FALSE;
  }
  return FALSE;
}


int Paolo_spec (CHAR *paolo, CHAR *ch, int cmd, char *arg) {
  CHAR *fran;
  int found;

  static int state = 0;

  if(ch && cmd==MSG_ENTER) {
    if(!state) {
        act("\n\r$n whirls about the cliff edge on a constant breeze.",FALSE,paolo,0,ch,TO_ROOM);
        state++;
        return FALSE;
    }
  }

  if(cmd==MSG_MOBACT && state) {
    found=0;
    for (fran=world[CHAR_REAL_ROOM(paolo)].people; fran; fran=fran->next_in_room) {
      if (IS_NPC(fran) && V_MOB(fran)==25331) {
         found=TRUE;
         break;
      }
    }

    switch (state) {
    case 1:
      if(!found) break;
      act("$n sighs in deep sadness, a tear cascading down her pale cheek.",FALSE,fran,0,ch,TO_ROOM);
      state++;
      break;

    case 2:
      if(!found) break;
      act("$n whispers softly in Francesca's ear, trying in vain to comfort her.",FALSE,paolo,0,ch,TO_ROOM);
      state++;
      break;

    case 3:
      if(!found) break;
      act("$n holds her breath and stares through tears toward the vast grey skies.",FALSE,fran,0,ch,TO_ROOM);
      state++;
      break;

    case 4:
      if(!found) break;
      act("$n and Francesca cling tightly to each other, continuing on in their eternal dance.",FALSE,paolo,0,ch,TO_ROOM);
      state++;
      break;

    case 5:
      state = 0;
      break;

    default:
      break;
    }
  }
  return FALSE;
}


int sin_wrath(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {
  CHAR *act_ch, *act_ch_next;

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n is relieved of the burden of $s sin, becoming normal again.", TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your sin is finally cleansed from your body.\n\r",ench_ch);
    return FALSE;
  }

  if(CHAR_REAL_ROOM(ench_ch)==real_room(PORTAL_ENTER) && cmd==CMD_NORTH) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("A magical force blocks your sinful being from entering this place.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(ench_ch && cmd==MSG_MOBACT && chance(4)) {
    if(IS_SET(world[CHAR_REAL_ROOM(ench_ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;
    for(act_ch=world[CHAR_REAL_ROOM(ench_ch)].people;act_ch;act_ch=act_ch_next) {
      act_ch_next=act_ch->next_in_room;
      if(act_ch!=ench_ch && CAN_SEE(ench_ch,act_ch) && chance(33)) {
        act("Your wrath overcomes you as you attack $N viciously!",1,ench_ch,0,act_ch,TO_CHAR);
        act("$n's wrath overcomes $m as $e attacks $N viciously!",1,ench_ch,0,act_ch,TO_NOTVICT);
        act("$n's wrath overcomes $m as $e attacks you viciously!",1,ench_ch,0,act_ch,TO_VICT);
        damage(ench_ch,act_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
        return FALSE;
      }
      break;
    }
    return FALSE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n has been touched by the Hand of Lucifer!",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_TICK && ench->duration==1) {
    act("$n prays in vain that $s affliction might be lifted..",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your affliction finally begins to abate as you pray to the gods.\n\r",ench_ch);
  }
  return FALSE;
}


int sin_avarice(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {
  CHAR *act_ch, *act_ch_next;

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n is relieved of the burden of $s sin, becoming normal again.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your sin is finally cleansed from your body.\n\r",ench_ch);
    return FALSE;
  }

  if(CHAR_REAL_ROOM(ench_ch)==real_room(PORTAL_ENTER) && cmd==CMD_NORTH) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("A magical force blocks your sinful being from entering this place.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(ench_ch && cmd==MSG_TICK && GET_GOLD(ench_ch)>66666) {
    for(act_ch=world[CHAR_REAL_ROOM(ench_ch)].people;act_ch;act_ch=act_ch_next) {
      act_ch_next=act_ch->next_in_room;
      if(act_ch!=ench_ch) {
        damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
        if(chance(66) && CAN_SEE(ench_ch,act_ch) && GET_GOLD(ench_ch)>66665) {
          act("Your sin causes you to .. be generous !?!",1,ench_ch,0,act_ch,TO_CHAR);
          act("$n flaunts $s wealth by giving 66666 coins to $N!",1,ench_ch,0,act_ch,TO_NOTVICT);
          act("$n flaunts $s wealth by giving you 66666 coins!",1,ench_ch,0,act_ch,TO_VICT);
          GET_GOLD(ench_ch)-=66666;
          GET_GOLD(act_ch)+=66666;
          return FALSE;
        }
      }
    }
    return FALSE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n has been touched by the Hand of Lucifer!",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_TICK && ench->duration==1) {
    act("$n prays in vain that $s affliction might be lifted..",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your affliction finally begins to abate as you pray to the gods.\n\r",ench_ch);
  }
  return FALSE;
}


int ench_pregnant(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {
  if(cmd==MSG_REMOVE_ENCH) {
    return FALSE;
  }
  return FALSE;
}


int sin_lust(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {
  int dur;
  ENCH *tmp_enchantment2;
  ENCH *tmp_pregnant;
  CHAR *demon;
  char buf[MIL], buf2[MIL];
  struct descriptor_data *d;

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n is relieved of the burden of $s sin, becoming normal again.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your sin is finally cleansed from your body.\n\r",ench_ch);
    return FALSE;
  }

  if(CHAR_REAL_ROOM(ench_ch)==real_room(PORTAL_ENTER) && cmd==CMD_NORTH) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("A magical force blocks your sinful being from entering this place.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(ench_ch && cmd==MSG_TICK && IS_MORTAL(ench_ch)) {
    dur=ench->duration;
    switch(dur) {

      case 60:
      act("A large hairy demon appears from nowhere and begins assailing your body!",1,ench_ch,0,0,TO_CHAR);
      act("A large hairy demon appears from nowhere and does unspeakable things to $n!",1,ench_ch,0,0,TO_ROOM);
      damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        demon = read_mobile(INCUBUS, VIRTUAL);
        char_to_room(demon,CHAR_REAL_ROOM(ench_ch));

/* give lust to new demons as well, but just once */
        CREATE(tmp_enchantment2, ENCH, 1);
        tmp_enchantment2->name     = str_dup("Lust");
        tmp_enchantment2->duration = 3;
        tmp_enchantment2->func     = sin_lust;
        enchantment_to_char(demon, tmp_enchantment2, FALSE);
        act("$n has been touched by the Hand of Lucifer!",FALSE,demon,0,0,TO_ROOM);
        send_to_char("You feel empowered by the seed of Satan!\n\r",demon);
        sprintf (buf, "follow %s", GET_NAME(ench_ch));
        command_interpreter (demon, buf);
      }
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        CREATE(tmp_pregnant, ENCH, 1);
        tmp_pregnant->name     = str_dup("Pregnant!");
        tmp_pregnant->duration = 10;
        tmp_pregnant->func     = ench_pregnant;
        enchantment_to_char(ench_ch, tmp_pregnant, FALSE);
        send_to_char("Congratulations! You're expecting!\n\r",ench_ch);
        if(IS_MORTAL(ench_ch)) {
          sprintf(buf2,"Hell Log Ench: [ %s just contracted 1st Pregnancy at %d ]",GET_NAME(ench_ch),world[CHAR_REAL_ROOM(ench_ch)].number);
          log_s(buf2);
        }
      }
      break;

      case 51:
      act("You scream as your stomach begins to convulse and break open!",1,ench_ch,0,0,TO_CHAR);
      act("$n screams as $e begins to give birth to something horrible!",1,ench_ch,0,0,TO_ROOM);
      break;

      case 50:
      act("Your stomach explodes with the force of the demon escaping it!",1,ench_ch,0,0,TO_CHAR);
      act("$n's stomach explodes as $e gives birth to a demon!",1,ench_ch,0,0,TO_ROOM);
      damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        demon = read_mobile(INCUBUS, VIRTUAL);
        char_to_room(demon,CHAR_REAL_ROOM(ench_ch));

/* give lust to new demons as well, but just once */
        CREATE(tmp_enchantment2, ENCH, 1);
        tmp_enchantment2->name     = str_dup("Lust");
        tmp_enchantment2->duration = 3;
        tmp_enchantment2->func     = sin_lust;
        enchantment_to_char(demon, tmp_enchantment2, FALSE);
        act("$n has been touched by the Hand of Lucifer!",FALSE,demon,0,0,TO_ROOM);
        send_to_char("You feel empowered by the seed of Satan!\n\r",demon);
        sprintf (buf, "follow %s", GET_NAME(ench_ch));
        command_interpreter (demon, buf);
      }
      break;

      case 45:
      act("A large hairy demon appears from nowhere and begins assailing your body!",1,ench_ch,0,0,TO_CHAR);
      act("A large hairy demon appears from nowhere and does unspeakable things to $n!",1,ench_ch,0,0,TO_ROOM);
      damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        demon = read_mobile(INCUBUS, VIRTUAL);
        char_to_room(demon,CHAR_REAL_ROOM(ench_ch));

/* give lust to new demons as well, but just once */
        CREATE(tmp_enchantment2, ENCH, 1);
        tmp_enchantment2->name     = str_dup("Lust");
        tmp_enchantment2->duration = 3;
        tmp_enchantment2->func     = sin_lust;
        enchantment_to_char(demon, tmp_enchantment2, FALSE);
        act("$n has been touched by the Hand of Lucifer!",FALSE,demon,0,0,TO_ROOM);
        send_to_char("You feel empowered by the seed of Satan!\n\r",demon);
        sprintf (buf, "follow %s", GET_NAME(ench_ch));
        command_interpreter (demon, buf);
      }

      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        CREATE(tmp_pregnant, ENCH, 1);
        tmp_pregnant->name     = str_dup("Pregnant!");
        tmp_pregnant->duration = 10;
        tmp_pregnant->func     = ench_pregnant;
        enchantment_to_char(ench_ch, tmp_pregnant, FALSE);
        send_to_char("Congratulations! You're expecting!\n\r",ench_ch);
        if(IS_MORTAL(ench_ch)) {
          sprintf(buf2,"Hell Log Ench: [ %s just contracted 2nd Pregnancy at %d ]",GET_NAME(ench_ch),world[CHAR_REAL_ROOM(ench_ch)].number);
          log_s(buf2);
        }
      }
      break;

      case 36:
      act("You scream as your stomach begins to convulse and break open!",1,ench_ch,0,0,TO_CHAR);
      act("$n screams as $e begins to give birth to something horrible!",1,ench_ch,0,0,TO_ROOM);
      break;

      case 35:
      act("Your stomach explodes with the force of the demon escaping it!",1,ench_ch,0,0,TO_CHAR);
      act("$n's stomach explodes as $e gives birth to a demon!",1,ench_ch,0,0,TO_ROOM);
      damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        demon = read_mobile(INCUBUS, VIRTUAL);
        char_to_room(demon,CHAR_REAL_ROOM(ench_ch));

/* give lust to new demons as well, but just once */
        CREATE(tmp_enchantment2, ENCH, 1);
        tmp_enchantment2->name     = str_dup("Lust");
        tmp_enchantment2->duration = 3;
        tmp_enchantment2->func     = sin_lust;
        enchantment_to_char(demon, tmp_enchantment2, FALSE);
        act("$n has been touched by the Hand of Lucifer!",FALSE,demon,0,0,TO_ROOM);
        send_to_char("You feel empowered by the seed of Satan!\n\r",demon);
        sprintf (buf, "follow %s", GET_NAME(ench_ch));
        command_interpreter (demon, buf);
      }
      break;

      case 30:
      act("A large hairy demon appears from nowhere and begins assailing your body!",1,ench_ch,0,0,TO_CHAR);
      act("A large hairy demon appears from nowhere and does unspeakable things to $n!",1,ench_ch,0,0,TO_ROOM);
      damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        demon = read_mobile(INCUBUS, VIRTUAL);
        char_to_room(demon,CHAR_REAL_ROOM(ench_ch));

/* give lust to new demons as well, but just once */
        CREATE(tmp_enchantment2, ENCH, 1);
        tmp_enchantment2->name     = str_dup("Lust");
        tmp_enchantment2->duration = 3;
        tmp_enchantment2->func     = sin_lust;
        enchantment_to_char(demon, tmp_enchantment2, FALSE);
        act("$n has been touched by the Hand of Lucifer!",FALSE,demon,0,0,TO_ROOM);
        send_to_char("You feel empowered by the seed of Satan!\n\r",demon);
        sprintf (buf, "follow %s", GET_NAME(ench_ch));
        command_interpreter (demon, buf);
      }

      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        CREATE(tmp_pregnant, ENCH, 1);
        tmp_pregnant->name     = str_dup("Pregnant!");
        tmp_pregnant->duration = 10;
        tmp_pregnant->func     = ench_pregnant;
        enchantment_to_char(ench_ch, tmp_pregnant, FALSE);
        send_to_char("Congratulations! You're expecting!\n\r",ench_ch);
        if(IS_MORTAL(ench_ch)) {
          sprintf(buf2,"Hell Log Ench: [ %s just contracted 3rd Pregnancy at %d ]",GET_NAME(ench_ch),world[CHAR_REAL_ROOM(ench_ch)].number);
          log_s(buf2);
        }
      }
      break;

      case 21:
      act("You scream as your stomach begins to convulse and break open!",1,ench_ch,0,0,TO_CHAR);
      act("$n screams as $e begins to give birth to something horrible!",1,ench_ch,0,0,TO_ROOM);
      break;

      case 20:
      act("Your stomach explodes with the force of the demon escaping it!",1,ench_ch,0,0,TO_CHAR);
      act("$n's stomach explodes as $e gives birth to a demon!",1,ench_ch,0,0,TO_ROOM);
      damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        demon = read_mobile(INCUBUS, VIRTUAL);
        char_to_room(demon,CHAR_REAL_ROOM(ench_ch));

/* give lust to new demons as well, but just once */
        CREATE(tmp_enchantment2, ENCH, 1);
        tmp_enchantment2->name     = str_dup("Lust");
        tmp_enchantment2->duration = 3;
        tmp_enchantment2->func     = sin_lust;
        enchantment_to_char(demon, tmp_enchantment2, FALSE);
        act("$n has been touched by the Hand of Lucifer!",FALSE,demon,0,0,TO_ROOM);
        send_to_char("You feel empowered by the seed of Satan!\n\r",demon);
        sprintf (buf, "follow %s", GET_NAME(ench_ch));
        command_interpreter (demon, buf);
      }
      break;

      case 15:
      act("A large hairy demon appears from nowhere and begins assailing your body!",1,ench_ch,0,0,TO_CHAR);
      act("A large hairy demon appears from nowhere and does unspeakable things to $n!",1,ench_ch,0,0,TO_ROOM);
      damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        demon = read_mobile(INCUBUS, VIRTUAL);
        char_to_room(demon,CHAR_REAL_ROOM(ench_ch));

/* give lust to new demons as well, but just once */
        CREATE(tmp_enchantment2, ENCH, 1);
        tmp_enchantment2->name     = str_dup("Lust");
        tmp_enchantment2->duration = 3;
        tmp_enchantment2->func     = sin_lust;
        enchantment_to_char(demon, tmp_enchantment2, FALSE);
        act("$n has been touched by the Hand of Lucifer!",FALSE,demon,0,0,TO_ROOM);
        send_to_char("You feel empowered by the seed of Satan!\n\r",demon);
        sprintf (buf, "follow %s", GET_NAME(ench_ch));
        command_interpreter (demon, buf);
      }

      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        CREATE(tmp_pregnant, ENCH, 1);
        tmp_pregnant->name     = str_dup("Pregnant!");
        tmp_pregnant->duration = 10;
        tmp_pregnant->func     = ench_pregnant;
        enchantment_to_char(ench_ch, tmp_pregnant, FALSE);
        send_to_char("Congratulations! You're expecting!\n\r",ench_ch);
        if(IS_MORTAL(ench_ch)) {
          sprintf(buf2,"Hell Log Ench: [ %s just contracted 4th Pregnancy at %d ]",GET_NAME(ench_ch),world[CHAR_REAL_ROOM(ench_ch)].number);
          log_s(buf2);
        }
      }
      break;

      case 6:
      act("You scream as your stomach begins to convulse and break open!",1,ench_ch,0,0,TO_CHAR);
      act("$n screams as $e begins to give birth to something horrible!",1,ench_ch,0,0,TO_ROOM);
      break;

      case 5:
      act("Your stomach explodes with the force of the demon escaping it!",1,ench_ch,0,0,TO_CHAR);
      act("$n's stomach explodes as $e gives birth to a demon!",1,ench_ch,0,0,TO_ROOM);
      damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        demon = read_mobile(INCUBUS, VIRTUAL);
        char_to_room(demon,CHAR_REAL_ROOM(ench_ch));

/* give lust to new demons as well, but just once */
        CREATE(tmp_enchantment2, ENCH, 1);
        tmp_enchantment2->name     = str_dup("Lust");
        tmp_enchantment2->duration = 3;
        tmp_enchantment2->func     = sin_lust;
        enchantment_to_char(demon, tmp_enchantment2, FALSE);
        act("$n has been touched by the Hand of Lucifer!",FALSE,demon,0,0,TO_ROOM);
        send_to_char("You feel empowered by the seed of Satan!\n\r",demon);
        sprintf (buf, "follow %s", GET_NAME(ench_ch));
        command_interpreter (demon, buf);
      }
      break;

      case 1:
      act("$n prays in vain that $s affliction might be lifted..",TRUE,ench_ch,NULL,NULL,TO_ROOM);
      send_to_char("Your affliction finally begins to abate as you pray to the gods.\n\r",ench_ch);
      break;

      default:
      break;
    }
    return FALSE;

  } else if(ench_ch && cmd==MSG_TICK && IS_NPC(ench_ch)) {
    dur=ench->duration;
    switch(dur) {

      case 2:
      act("You scream as your stomach begins to convulse and break open!",1,ench_ch,0,0,TO_CHAR);
      act("$n screams as $e begins to give birth to something horrible!",1,ench_ch,0,0,TO_ROOM);
      break;

      case 1:
      act("Your stomach explodes with the force of the demon escaping it!",1,ench_ch,0,0,TO_CHAR);
      act("$n's stomach explodes as $e gives birth to a demon!",1,ench_ch,0,0,TO_ROOM);
      demon = read_mobile(INCUBUS, VIRTUAL);
      char_to_room(demon,CHAR_REAL_ROOM(ench_ch));
      break;

      default:
      break;
    }
    return FALSE;
  }

  if(ench_ch && cmd==MSG_MOBACT && IS_MORTAL(ench_ch)) {
    if(enchanted_by(ench_ch,"Pregnant!") && chance(6)) {
      act("A shooting pain cramps in your stomach!",1,ench_ch,0,0,TO_CHAR);
      act("$n doubles over with the pain of impending labor!",1,ench_ch,0,0,TO_ROOM);
      damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
    }
    if(enchanted_by(ench_ch,"Pregnant!") && ench_ch->specials.fighting) {
      d=ench_ch->desc;
      if(d && d->connected==CON_PLYNG) {
        act("Perhaps you shouldn't be fighting in your condition..",1,ench_ch,0,0,TO_CHAR);
        act("$n decides that $e should probably rest.",1,ench_ch,0,0,TO_ROOM);
        stop_fighting(ench_ch->specials.fighting);
        stop_fighting(ench_ch);
        GET_POS(ench_ch)=POSITION_RESTING;
        update_pos(ench_ch);
      }
      return FALSE;
    }
    return FALSE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    if(enchanted_by(ench_ch,"Pregnant!")) {
      act("......$n is bulging with the offspring of Satan!",FALSE, ench_ch,0,ch,TO_VICT);
      return FALSE;
    }
    else {
      act("......$n has been touched by the Hand of Lucifer!",FALSE, ench_ch,0,ch,TO_VICT);
      return FALSE;
    }
  }

  return FALSE;
}


int sin_gluttony(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n is relieved of the burden of $s sin, becoming normal again.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your sin is finally cleansed from your body.\n\r",ench_ch);
    return FALSE;
  }

  if(CHAR_REAL_ROOM(ench_ch)==real_room(PORTAL_ENTER) && cmd==CMD_NORTH) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("A magical force blocks your sinful being from entering this place.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(ch==ench_ch && (cmd==CMD_EAT || cmd==CMD_QUAFF || cmd == CMD_DRINK)) {
    act("Your stomach convulses as you attempt to fill it!",1,ench_ch,0,0,TO_CHAR);
    act("$n is repulsed by $s own voraciousness!",1,ench_ch,0,0,TO_ROOM);
    damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n has been touched by the Hand of Lucifer!",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_TICK && ench->duration==1) {
    act("$n prays in vain that $s affliction might be lifted..",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your affliction finally begins to abate as you pray to the gods.\n\r",ench_ch);
  }
  return FALSE;
}


int sin_envy(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {
  CHAR *act_ch;
  char buf[MAX_STRING_LENGTH];
  OBJ *t, *junk = 0, *temp;
  int tar = -1, tmp = 0;
  int a=FALSE, b=FALSE, c=FALSE, d=FALSE, e=FALSE, f=FALSE, g=FALSE;
  int h=FALSE, j=FALSE, k=FALSE, l=FALSE, m=FALSE, n=FALSE, o=FALSE;
  int p=FALSE, q=FALSE, r=FALSE, s= FALSE, check = 0, i;

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n is relieved of the burden of $s sin, becoming normal again.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your sin is finally cleansed from your body.\n\r",ench_ch);
    return FALSE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n has been touched by the Hand of Lucifer!",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_TICK && ench->duration==1) {
    act("$n prays in vain that $s affliction might be lifted..",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your affliction finally begins to abate as you pray to the gods.\n\r",ench_ch);
  }

  if(CHAR_REAL_ROOM(ench_ch)==real_room(PORTAL_ENTER) && cmd==CMD_NORTH) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("A magical force blocks your sinful being from entering this place.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(ench_ch && cmd==MSG_MOBACT && chance(6)) {

    for(i=0; i<=WIELD; i++) {
      if (!(temp=ench_ch->equipment[i]))
        continue;

      if (ench_ch->equipment[0]) a=TRUE;
      if (ench_ch->equipment[1]) b=TRUE;
      if (ench_ch->equipment[2]) c=TRUE;
      if (ench_ch->equipment[3]) d=TRUE;
      if (ench_ch->equipment[4]) e=TRUE;
      if (ench_ch->equipment[5]) f=TRUE;
      if (ench_ch->equipment[6]) g=TRUE;
      if (ench_ch->equipment[7]) h=TRUE;
      if (ench_ch->equipment[8]) j=TRUE;
      if (ench_ch->equipment[9]) k=TRUE;
      if (ench_ch->equipment[10]) l=TRUE;
      if (ench_ch->equipment[11]) m=TRUE;
      if (ench_ch->equipment[12]) n=TRUE;
      if (ench_ch->equipment[13]) o=TRUE;
      if (ench_ch->equipment[14]) p=TRUE;
      if (ench_ch->equipment[15]) q=TRUE;
      if (ench_ch->equipment[16]) r=TRUE;
      if (ench_ch->equipment[17]) s=TRUE;
    }

    if(a || b || c || d || e || f || g || h || j || k || l || m || n || o || p || q || r || s)
      check = 1;

    if(check==0) return FALSE;

    switch(number(1,2)) {
      case 1:
      act_ch=get_random_vict(ench_ch);
      if(!act_ch) break;
      if(act_ch!=ench_ch) {
        act("You flaunt your gear, causing everyone to envy you deeply!",1,ench_ch,0,act_ch,TO_CHAR);
        act("$n flaunts $s gear, causing you to boil over with envy!",1,ench_ch,0,act_ch,TO_NOTVICT);
        act("$n flaunts $s gear, giving you a worthless piece!",1,ench_ch,0,act_ch,TO_VICT);

                                    /* checks ench_ch's gear to remove a piece and give to act_ch */
        for (tmp = 0;tmp < MAX_WEAR;tmp++) {
          if ((t = EQ (ench_ch, tmp)))  {
            if (tar == -1) tar = tmp;
            if (chance(20)) tar = tmp;
          }
        }
        junk = unequip_char(ench_ch, tar);
        obj_to_char (junk, act_ch);
        save_char(ench_ch, NOWHERE);
        save_char(act_ch, NOWHERE);
        sprintf(buf,"Hell Log - Envy : %s just gave %s to %s.",GET_NAME(ench_ch),OBJ_SHORT(junk),(IS_NPC(act_ch) ? MOB_SHORT(act_ch) : GET_NAME(act_ch)));
        log_s(buf);
        junk->log=1;
      }
      break;

      case 2:                    /* checks ench_ch's gear to remove a piece and drop it */
      for (tmp = 0;tmp < MAX_WEAR;tmp++) {
        if ((t = EQ (ench_ch, tmp)))  {
          if (tar == -1) tar = tmp;
          if (chance(20)) tar = tmp;
        }
      }
      act("You dispose of some worthless gear.",1,ench_ch,0,0,TO_CHAR);
      act("$n disposes of some worthless gear, making you feel envious!",1,ench_ch,0,0,TO_ROOM);
      junk = unequip_char (ench_ch,tar);
      obj_to_room (junk, CHAR_REAL_ROOM(ench_ch));
      save_char(ench_ch, NOWHERE);
      sprintf(buf,"Hell Log - Envy : %s just dropped %s.",GET_NAME(ench_ch),OBJ_SHORT(junk));
      log_s(buf);
      junk->log=1;
      break;

      default:
      break;

    }
    return FALSE;
  }
  return FALSE;
}


int sin_pride(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n is relieved of the burden of $s sin, becoming normal again.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your sin is finally cleansed from your body.\n\r",ench_ch);
    return FALSE;
  }

  if(cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n stands statuesque here in the center of Market Square.",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_TICK && ench->duration==1) {
    act("$n prays in vain that $s affliction might be lifted..",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your affliction finally begins to abate as you pray to the gods.\n\r",ench_ch);
  }
           /* until there is a better way to have cmd_affect go through, this will suffice.. */

  if(cmd==CMD_NORTH || cmd==CMD_EAST || cmd==CMD_SOUTH || cmd==CMD_WEST || cmd==CMD_UP ||
     cmd==CMD_LOOK || cmd==CMD_SAY || cmd==CMD_SHOUT || cmd==CMD_ENTER || cmd==CMD_LEAVE ||
     cmd==CMD_SLEEP || cmd==CMD_WAKE || cmd==CMD_REST || cmd==CMD_SIT || cmd==CMD_STAND ||
     cmd==CMD_SUBDUE || cmd==CMD_RIDE || cmd==CMD_GET || cmd==CMD_DROP || cmd==CMD_PUT ||
     cmd==CMD_GIVE || cmd==CMD_WEAR || cmd==CMD_GRAB || cmd==CMD_EXAMINE || cmd==CMD_EAT ||
     cmd==CMD_TASTE || cmd==CMD_POUR || cmd==CMD_WRITE || cmd==CMD_DRINK || cmd==CMD_SIP ||
     cmd==CMD_JUNK || cmd==CMD_SPLIT || cmd==CMD_KILL || cmd==CMD_WIELD || cmd==CMD_FLEE ||
     cmd==CMD_ASSIST || cmd==CMD_THROW || cmd==CMD_RELOAD || cmd==CMD_SHOOT || cmd==CMD_CAST ||
     cmd==CMD_TELL || cmd==CMD_ASK || cmd==CMD_WHISPER || cmd==CMD_GROUPTELL || cmd==CMD_GOSSIP ||
     cmd==CMD_AUCTION || cmd==CMD_CHAOS || cmd==CMD_EMOTE || cmd==CMD_REPLY || cmd==CMD_DOWN ||
     cmd==CMD_BID || cmd==CMD_HIT || cmd==CMD_ORDER || cmd==CMD_FOLLOW || cmd==CMD_REFOLLOW ||
     cmd==CMD_UNFOLLOW || cmd==CMD_TAKE || cmd==CMD_USE || cmd==CMD_EMOTE2 || cmd==CMD_GROUP ||
     cmd==CMD_QUAFF || cmd==CMD_RECITE || cmd==CMD_REPORT || cmd==CMD_FILL || cmd==CMD_TAP ||
     cmd==CMD_SONG || cmd==CMD_COLLECT || cmd==CMD_DONATE || cmd==CMD_HOME || cmd==CMD_QUEST ||
     cmd==CMD_AQUEST || cmd==CMD_SKIN) {
    if(ch!=ench_ch) return FALSE;
    GET_POS(ench_ch)=POSITION_SITTING;
    send_to_char("You are a statue! Your pride is your weakness and all those around you shall behold your insolence!\n\r",ench_ch);
    return TRUE;
  } else {
    GET_POS(ench_ch)=POSITION_SITTING;
    return FALSE;
  }
  return FALSE;
}


int sin_sloth(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n is relieved of the burden of $s sin, becoming normal again.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your sin is finally cleansed from your body.\n\r",ench_ch);
    return FALSE;
  }

  if(CHAR_REAL_ROOM(ench_ch)==real_room(PORTAL_ENTER) && cmd==CMD_NORTH) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("A magical force blocks your sinful being from entering this place.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your sin will not allow you to quit.\n\r",ench_ch);
    return TRUE;
  }

  if(ch==ench_ch && (cmd==CMD_SLEEP || cmd==CMD_REST || cmd==CMD_SIT)) {
    if(ch!=ench_ch) return FALSE;
    act("You cannot rest for some reason as your behavior has become erratic!",1,ench_ch,0,0,TO_CHAR);
    act("$n swears loudly as $e tries in vain to rest!",1,ench_ch,0,0,TO_ROOM);
    damage(ench_ch,ench_ch,66,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
  }

  if(ch==ench_ch && GET_POS(ench_ch)==POSITION_SLEEPING && cmd==CMD_WAKE) {
    if(chance(50)) {
      act("You cannot seem to awaken.", FALSE,ench_ch,0,0,TO_CHAR);
      return TRUE;
    }
    return FALSE;
  }

  if(ench_ch && cmd==MSG_MOBACT && chance(3)) {
    if(GET_POS(ench_ch)==POSITION_SLEEPING) return FALSE;
    if(ench_ch->specials.fighting) stop_fighting(ench_ch);
    act("You go to sleep.", FALSE,ench_ch,0,0,TO_CHAR);
    act("$n lies down to sleep.", TRUE, ench_ch, 0, 0, TO_ROOM);
    GET_POS(ench_ch)=POSITION_SLEEPING;
    return FALSE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n has been touched by the Hand of Lucifer!",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_TICK && ench->duration==1) {
    act("$n prays in vain that $s affliction might be lifted..",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your affliction finally begins to abate as you pray to the gods.\n\r",ench_ch);
  }
  return FALSE;
}


void check_equipment(CHAR *ch);
int Lucifer_spec (CHAR *lucifer, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*belial,*reaper;
  ENCH *tmp_enchantment;
  char buf[MIL], buf3[MIL], buf4[MIL];
  OBJ *tmp, *tmp2;
  int stingnum, num, dmg, i, qend, spl, attack=100, pop_percent, object, sword=0, reset=1;
  struct descriptor_data *d;
  struct char_data *victim, *next_vict;
  struct obj_data *obj;
  struct affected_type_5 af;
  char *tmp_string;
  char *skip_spaces (char*);

  if(lucifer && cmd==MSG_ZONE_RESET) {

    if((belial=get_char_room("belial",real_room(25416)))) {
      if(count_mortals_zone(belial,TRUE)) return FALSE;
    }
    if((reaper=get_char_room("reaper",real_room(25301)))) {
      if(count_mortals_zone(reaper,TRUE)) return FALSE;
    }
    if(count_mortals_zone(lucifer,TRUE)) return FALSE;

    if(IS_AFFECTED(lucifer, AFF_DETECT_INVISIBLE) &&
       IS_AFFECTED(lucifer, AFF_DETECT_ALIGNMENT) &&
       IS_AFFECTED(lucifer, AFF_DETECT_MAGIC) &&
       IS_AFFECTED(lucifer, AFF_SNEAK)) {
      reset=4;
    }
    if(IS_AFFECTED(lucifer, AFF_DETECT_INVISIBLE) &&
       IS_AFFECTED(lucifer, AFF_DETECT_ALIGNMENT) &&
       IS_AFFECTED(lucifer, AFF_DETECT_MAGIC) &&
       !IS_AFFECTED(lucifer, AFF_SNEAK)) {
      SET_BIT(lucifer->specials.affected_by, AFF_SNEAK);
      reset=3;
    }
    if(IS_AFFECTED(lucifer, AFF_DETECT_INVISIBLE) &&
       IS_AFFECTED(lucifer, AFF_DETECT_ALIGNMENT) &&
       !IS_AFFECTED(lucifer, AFF_DETECT_MAGIC) &&
       !IS_AFFECTED(lucifer, AFF_SNEAK)) {
      SET_BIT(lucifer->specials.affected_by, AFF_DETECT_MAGIC);
      reset=2;
    }
    if(IS_AFFECTED(lucifer, AFF_DETECT_INVISIBLE) &&
       !IS_AFFECTED(lucifer, AFF_DETECT_ALIGNMENT) &&
       !IS_AFFECTED(lucifer, AFF_DETECT_MAGIC) &&
       !IS_AFFECTED(lucifer, AFF_SNEAK)) {
      SET_BIT(lucifer->specials.affected_by, AFF_DETECT_ALIGNMENT);
      reset=1;
    }
    if(!(lucifer->equipment[1])) {
      object = real_object(RING_DESPAIR);
      pop_percent=obj_proto_table[object].obj_flags.repop_percent;
      if(pop_percent) pop_percent=pop_percent*reset;
      if(number(1,100)<=pop_percent) {
        obj = read_object(RING_DESPAIR, VIRTUAL);
        equip_char(lucifer, obj, 1);
      } else {
        if(number(1,100)<=(5*reset)) {
          obj = read_object(RING_DESPAIR_B, VIRTUAL);
          equip_char(lucifer, obj, 1);
        }
      }
    }
    if(!(lucifer->equipment[2])) {
      object = real_object(RING_AGONY);
      pop_percent=obj_proto_table[object].obj_flags.repop_percent;
      if(pop_percent) pop_percent=pop_percent*reset;
      if(number(1,100)<=pop_percent) {
        obj = read_object(RING_AGONY, VIRTUAL);
        equip_char(lucifer, obj, 2);
      } else {
        if(number(1,100)<=(5*reset)) {
          obj = read_object(RING_AGONY_B, VIRTUAL);
          equip_char(lucifer, obj, 2);
        }
      }
    }
    if(!(lucifer->equipment[6])) {
      object = real_object(HORNS_LUCIFER);
      pop_percent=obj_proto_table[object].obj_flags.repop_percent;
      if(pop_percent) pop_percent=pop_percent*reset;
      if(number(1,100)<=pop_percent) {
        obj = read_object(HORNS_LUCIFER, VIRTUAL);
        equip_char(lucifer, obj, 6);
      } else {
        if(number(1,100)<=(5*reset)) {
          obj = read_object(HORNS_LUCIFER_B, VIRTUAL);
          equip_char(lucifer, obj, 6);
        }
      }
    }
    if(!(lucifer->equipment[7])) {
      object = real_object(LEGGINGS);
      pop_percent=obj_proto_table[object].obj_flags.repop_percent;
      if(pop_percent) pop_percent=pop_percent*reset;
      if(number(1,100)<=pop_percent) {
        obj = read_object(LEGGINGS, VIRTUAL);
        equip_char(lucifer, obj, 7);
      } else {
        if(number(1,100)<=(5*reset)) {
          obj = read_object(LEGGINGS_B, VIRTUAL);
          equip_char(lucifer, obj, 7);
        }
      }
    }
    if(!(lucifer->equipment[8])) {
      if(number(1,100)<=(10*reset)) {
        obj = read_object(HOOVES_SATAN_B, VIRTUAL);
        equip_char(lucifer, obj, 8);
      }
    }
    if(!(lucifer->equipment[9])) {
      object = real_object(DOOM_GAUNTLETS);
      pop_percent=obj_proto_table[object].obj_flags.repop_percent;
      if(pop_percent) pop_percent=pop_percent*reset;
      if(number(1,100)<=pop_percent) {
        obj = read_object(DOOM_GAUNTLETS, VIRTUAL);
        equip_char(lucifer, obj, 9);
      } else {
        if(number(1,100)<=(5*reset)) {
          obj = read_object(DOOM_GAUNTLETS_B, VIRTUAL);
          equip_char(lucifer, obj, 9);
        }
      }
    }
    if(!(lucifer->equipment[10])) {
      object = real_object(WINGS_LUCIFER);
      pop_percent=obj_proto_table[object].obj_flags.repop_percent;
      if(pop_percent) pop_percent=pop_percent*reset;
      if(number(1,100)<=pop_percent) {
        obj = read_object(WINGS_LUCIFER, VIRTUAL);
        equip_char(lucifer, obj, 10);
      } else {
        if(number(1,100)<=(5*reset)) {
          obj = read_object(WINGS_LUCIFER_B, VIRTUAL);
          equip_char(lucifer, obj, 10);
        }
      }
    }
    if(!(lucifer->equipment[17])) {
      object = real_object(CROSS_INVERTED);
      pop_percent=obj_proto_table[object].obj_flags.repop_percent;
      if(pop_percent) pop_percent=pop_percent*reset;
      if(number(1,100)<=pop_percent) {
        obj = read_object(CROSS_INVERTED, VIRTUAL);
        equip_char(lucifer, obj, 17);
      } else {
        if(number(1,100)<=(5*reset)) {
          obj = read_object(CROSS_INVERTED_B, VIRTUAL);
          equip_char(lucifer, obj, 17);
        }
      }
    }
    for(tmp2=lucifer->carrying; tmp2; tmp2=tmp2->next_content) {
      if ((V_OBJ(tmp2) == 25312) || (V_OBJ(tmp2) == 25412)) sword=1;
    }
    if(sword==0 && number(1,100)<=(5*reset)) {
      obj = read_object(NIGHT_SWORD, VIRTUAL);
      obj_to_char(obj, lucifer);
    } else if(sword==0 && number(1,100)<=(10*reset)) {
      obj = read_object(NIGHT_SWORD_B, VIRTUAL);
      obj_to_char(obj, lucifer);
    }

    for (d = descriptor_list; d; d = d->next) {
      if (d->character != ch && !d->connected) {
        victim = d->character;
        if(GET_LEVEL(victim)>50 && GET_LEVEL(victim)<54 && !IS_NPC(victim)) {
          if(CHAR_REAL_ROOM(victim)!=real_room(1212)) break;
          if(!strcmp(GET_NAME(victim),"Thor"))
            attack=5;
          else
            attack=50;
          if(chance(attack)) return FALSE;
          if(IS_SET(victim->specials.pflag,PLR_WRITING)) return FALSE;
          act("Lucifer appears and guts you with his pitchfork !!\n\r",FALSE,victim,0,0,TO_CHAR);
          act("Lucifer appears and guts $n with his pitchfork !!",FALSE,victim,0,0,TO_ROOM);

          /* by popular demand, removing kill part of spec - Linerfix 051907

          divide_experience(lucifer,victim,1);
          raw_kill(victim);

          */
          WAIT_STATE(victim, PULSE_VIOLENCE*5);
        }
        return FALSE;
      }
    }
    return FALSE;
  }

  if(lucifer && cmd==MSG_DIE) {

    for (victim = world[CHAR_REAL_ROOM(lucifer)].people; victim; victim = next_vict) {
      next_vict=victim->next_in_room;
      if(IS_MORTAL(victim)) {
        act("\n\rA gigantic earthquake issues from beneath Satan, and cracks gape open around",FALSE,victim,0,0,TO_CHAR);
        act("you as molten rock flows from the depths below.  Satan's harrowed scream drowns",FALSE,victim,0,0,TO_CHAR);
        act("all thunder of the falling rock.  Suddenly, everything freezes in time, and",FALSE,victim,0,0,TO_CHAR);
        act("the world holds still in awe as Lucifer falls dead to the floor before you.\n\r",FALSE,victim,0,0,TO_CHAR);
        GET_ALIGNMENT(victim)=1000;
        check_equipment(victim); /*  Linerfix 110203 */
        act("The Miracle of Lucifer's defeat fills you with great joy!\n\r",FALSE,victim,0,0,TO_CHAR);
        GET_HIT(victim)=GET_MAX_HIT(victim);

        if(affected_by_spell(victim,SPELL_SANCTUARY))
          affect_from_char(victim, SPELL_SANCTUARY);

        if(!affected_by_spell(victim,SPELL_SANCTUARY)) {
          af.type      = SPELL_SANCTUARY;
          af.duration  = 66;
          af.modifier  = 0;
          af.location  = APPLY_NONE;
          af.bitvector = AFF_SANCTUARY;
          af.bitvector2 = 0;
          affect_to_char(victim, &af);
        }

        if(affected_by_spell(victim,SPELL_POWER_OF_DEVOTION))
          affect_from_char(victim, SPELL_POWER_OF_DEVOTION);

        if(!affected_by_spell(victim,SPELL_POWER_OF_DEVOTION)) {
          af.type      = SPELL_POWER_OF_DEVOTION;
          af.duration  = 66;
          af.modifier  = -15;
          af.location  = APPLY_AC;
          af.bitvector = 0;
          af.bitvector2 = 0;
          affect_to_char(victim, &af);

          af.modifier  = -1;
          af.location  = APPLY_SAVING_PARA;
          affect_to_char(victim, &af);
          af.location  = APPLY_SAVING_ROD;
          affect_to_char(victim, &af);
          af.location  = APPLY_SAVING_PETRI;
          affect_to_char(victim, &af);
          af.location  = APPLY_SAVING_BREATH;
          affect_to_char(victim, &af);
          af.location  = APPLY_SAVING_SPELL;
          affect_to_char(victim, &af);
        }

        act("$n is surrounded by a miraculous light.",TRUE,victim,0,0,TO_ROOM);
        act("You start glowing with a miraculous light.",TRUE,victim,0,0,TO_CHAR);

      }
    }

    for (d = descriptor_list; d; d = d->next) {
      if (d->character != ch && !d->connected) {
        victim = d->character;

        if(IS_MORTAL(victim) && CHAR_REAL_ROOM(victim)!=real_room(25541)) {
          act("\n\rA gigantic earthquake rumbles across the world, and cracks gape open around",FALSE,victim,0,0,TO_CHAR);
          act("you as molten rock flows from the depths below.  A harrowed scream drowns out",FALSE,victim,0,0,TO_CHAR);
          act("all semblance of normalcy.  Just as suddenly, everything freezes in time, and",FALSE,victim,0,0,TO_CHAR);
          act("the world holds still in awe of some great act.\n\r",FALSE,victim,0,0,TO_CHAR);
          if(victim->specials.fighting) {
            stop_fighting(victim->specials.fighting);
            stop_fighting(victim);
            WAIT_STATE(victim, PULSE_VIOLENCE);
          }
          if(GET_LEVEL(victim)<25 || GET_HIT(victim) < 0) return FALSE;
          if(GET_ALIGNMENT(victim) > 0) {
            act("A Miracle has occurred!\n\r",FALSE,victim,0,0,TO_CHAR);
            GET_HIT(victim)=GET_MAX_HIT(victim); /* mira them */
          } else {
              act("Something horrible has happened to the Great Evil!",FALSE,victim,0,0,TO_CHAR);
              act("That Really did HURT!",FALSE, victim,0,0,TO_CHAR);
              GET_HIT(victim)=MAX(GET_HIT(victim)-(GET_MAX_HIT(victim)/2),-6);
              update_pos(victim);
            }
        }
      }
    }
    return TRUE;
  }

/* on tick at night when lucifer isn't fighting and isn't at full hp, he gets 10% max regen */
  if(cmd==MSG_TICK && IS_NIGHT && GET_HIT(lucifer)>-1) {
    if(lucifer->specials.fighting) return FALSE;
    if(GET_HIT(lucifer)==GET_MAX_HIT(lucifer)) return FALSE;
    GET_HIT(lucifer)=MIN(GET_HIT(lucifer)+6666,GET_MAX_HIT(lucifer));
    sprintf(buf, "emote draws power from the threads of nightfall around him.");
    command_interpreter (lucifer, buf);
    return FALSE;
  }

  if(ch && cmd==CMD_STEAL) {  /* nosteal fix */
    if(GET_LEVEL(ch)>49) return FALSE;
    if(GET_CLASS(ch)!=CLASS_THIEF) return FALSE;
    sprintf(buf, "emote seethes with rage!");
    command_interpreter (lucifer, buf);
    GET_HIT(ch)=-6;
    return TRUE;
  }

  /* evil caster fun */
  if(cmd==CMD_CAST) {
    if(!ch) return FALSE;
    if(GET_CLASS(ch)==CLASS_THIEF ||
       GET_CLASS(ch)==CLASS_WARRIOR ||
       GET_CLASS(ch)==CLASS_NOMAD ||
       GET_CLASS(ch)==CLASS_PALADIN ||
       GET_CLASS(ch)==CLASS_ANTI_PALADIN ||
       GET_CLASS(ch)==CLASS_MAGIC_USER ||
       GET_CLASS(ch)==CLASS_BARD ||
       GET_CLASS(ch)==CLASS_COMMANDO) return FALSE;
    if(GET_ALIGNMENT(ch)>-350) return FALSE; /* this is for evil ninja/cleric casters only */

    tmp_string = str_dup (arg);
    tmp_string = skip_spaces(tmp_string);

    if (!(*tmp_string)) return FALSE;
    if (*tmp_string != '\'') return FALSE;

    for (qend=1; *(tmp_string+qend) && (*(tmp_string+qend) != '\'') ; qend++)
      *(tmp_string+qend) = LOWER(*(tmp_string+qend));

    if (*(tmp_string+qend) != '\'') return FALSE;

    spl = old_search_block(tmp_string, 1, qend-1,spells, 0);

    switch(spl) {

      case SPELL_HEAL:
        do_say(lucifer, "Thank you, my brethren! Your evil ways will prevail.", 0);
        act("A warm feeling fills your body as $N heals you.\n",FALSE,lucifer,0,ch,TO_CHAR);
        act("You heal $n.\n",FALSE,lucifer,0,ch,TO_VICT);
        act("$N heals $n.\n",FALSE,lucifer,0,ch,TO_NOTVICT);
        GET_HIT(lucifer)=MIN(GET_HIT(lucifer)+=100,GET_MAX_HIT(lucifer));
        break;

      case SPELL_MIRACLE:
        if(GET_CLASS(ch)!=CLASS_CLERIC) return FALSE;
        do_say(lucifer, "Ahhh, the Miracles of the truly despicable are a gift to the cause of Evil.", 0);
        act("Your life has been restored by $N.\n",FALSE,lucifer,0,ch,TO_CHAR);
        act("You restore $n.\n",FALSE,lucifer,0,ch,TO_VICT);
        act("$N restores $n.\n",FALSE,lucifer,0,ch,TO_NOTVICT);
        GET_HIT(lucifer)=GET_MAX_HIT(lucifer);
        break;

      case SPELL_GREAT_MIRACLE:
        if(GET_CLASS(ch)!=CLASS_CLERIC) return FALSE;
        do_say(lucifer, "Ahhh, the Miracles of the truly despicable are a gift to the cause of Evil.", 0);
        act("Your life has been restored by $N.\n",FALSE,lucifer,0,ch,TO_CHAR);
        act("You restore $n.\n",FALSE,lucifer,0,ch,TO_VICT);
        act("$N restores $n.\n",FALSE,lucifer,0,ch,TO_NOTVICT);
        GET_HIT(lucifer)=GET_MAX_HIT(lucifer);
        break;

      default:
        break;
    }
    return FALSE;
  }

               /* Lucifer's enchantment specs */

  if(lucifer && cmd==MSG_MOBACT && lucifer->specials.fighting) {

    if(chance(15) && GET_HIT(lucifer)<(GET_MAX_HIT(lucifer)/4)) {
      if(!affected_by_spell(lucifer, SPELL_RAGE)) {
        num=number(1,3);
        send_to_char("Rage courses through your body!\n\r", lucifer);
        act("\n$n utters the words, 'gruoz'\n$n becomes enraged in a torrent of hail and brimstone!!\n\r",1,lucifer,0,0,TO_ROOM);
        af.type      = SPELL_RAGE;
        af.duration  = num;
        af.modifier  = 0;
        af.location  = 0;
        af.bitvector = 0;
        af.bitvector2 = AFF2_RAGE;
        affect_to_char(lucifer, &af);
      }
    }

    if(!(lucifer->equipment[WIELD]) && chance(25)) {
      act("You become enraged with fury, choosing to display the true power of Hell.",FALSE,lucifer,0,0,TO_CHAR);
      act("\n\rLucifer gains strength as his fury overflows !\nThe true power of Hell is revealed !\n\r",FALSE,lucifer,0,0,TO_ROOM);
      if((tmp=read_object(LUCIFER_SWORD,VIRTUAL))) {
        lucifer->equipment[WIELD] = tmp;
        act("$n wields $p.", TRUE, lucifer, tmp, 0, TO_ROOM);
        act("You wield $p.", TRUE, lucifer, tmp, 0, TO_CHAR);
      }
    }

    if((9*GET_MAX_HIT(lucifer)/10) >= GET_HIT(lucifer)) {   /* if lucifer is below 90% hp, SHIELD */
      if(!IS_SET(lucifer->specials.act, ACT_SHIELD)) SET_BIT(lucifer->specials.act,ACT_SHIELD);
    } else { /* if lucifer is above 90% hp, UNSHIELD */
      if(IS_SET(lucifer->specials.act, ACT_SHIELD)) REMOVE_BIT(lucifer->specials.act,ACT_SHIELD);
    }

    if(chance(29)) {
      switch(number(1,5)) {
        case 1:
          vict=get_random_victim_fighting(lucifer);
          if(!vict) break;
          act("You force $N to drop $S worldly things!",0,lucifer,0,vict,TO_CHAR);
          act("$n bellows an order and you drop your possessions instantly!",0,lucifer,0,vict,TO_VICT);
          act("$n bellows an order at $N!",0,lucifer,0,vict,TO_NOTVICT);
          sprintf(buf3, "remove all");
          command_interpreter(vict,buf3);
          sprintf(buf3, "drop all");
          command_interpreter(vict,buf3);
          break;
        case 2:
          vict=get_random_victim_fighting(lucifer);
          if(!vict) break;
          if(IS_NPC(vict)) break;
          sprintf(buf3, "say I will eat your soul, %s.",GET_NAME(vict));
          command_interpreter(lucifer,buf3);
          break;
        case 3:
          do_say(lucifer, "You shall revel in your sins here with me in eternal fire!",0);
          act("\n\r$n raises his muscular arms above his head !!\n\r",0,lucifer,0,0,TO_ROOM);
          act("You prepare to smite the evildoers!",0,lucifer,0,0,TO_CHAR);
          for(vict = world[CHAR_REAL_ROOM(lucifer)].people; vict;vict = next_vict) {
            next_vict = vict->next_in_room;
            if(IS_MORTAL(vict) && GET_ALIGNMENT(vict)<0) {
              act("$n streams blinding white heat from $s fingertips into your body!\n\r",0,lucifer,0,vict,TO_VICT);
              act("$n streams blinding white heat from $s fingertips at $N!\n",0,lucifer,0,vict,TO_NOTVICT);
              GET_HIT(vict)=-6;
            }
          }
          break;
        case 4:
          vict=get_random_victim_fighting(lucifer);
          if(!vict) break;
          if(vict!=lucifer->specials.fighting && IS_MORTAL(vict)) {
            sprintf(buf3, "say %s is dead to me.",GET_NAME(vict));
            command_interpreter(lucifer,buf3);
            act("\nYour life has been nullified.\n",0,lucifer,0,vict,TO_VICT);
            GET_HIT(vict)=-666666;
            damage(lucifer,vict,1,TYPE_UNDEFINED,DAM_NO_BLOCK);
            update_pos(vict);
          }
          break;
        case 5:
          vict=get_random_victim_fighting(lucifer);
          if(!vict) break;
          if(IS_NPC(vict)) break;
          sprintf(buf3, "pelt %s",GET_NAME(vict));
          command_interpreter(lucifer,buf3);
          damage(lucifer,vict,666,TYPE_UNDEFINED,DAM_NO_BLOCK);
          break;
        default:
          break;
      }
    }

    if(chance(18)) {
      vict=get_random_victim_fighting(lucifer);
      if(vict==lucifer->specials.fighting && chance(33)) vict=get_random_victim_fighting(lucifer);
      if(!vict) return FALSE;
      if(enchanted_by(vict,"Deadly Sin - Wrath") ||
         enchanted_by(vict,"Deadly Sin - Lust") ||
         enchanted_by(vict,"Deadly Sin - Avarice") ||
         enchanted_by(vict,"Deadly Sin - Gluttony") ||
         enchanted_by(vict,"Deadly Sin - Envy") ||
         enchanted_by(vict,"Deadly Sin - Pride") ||
         enchanted_by(vict,"Deadly Sin - Sloth")) return FALSE;
      act("\n\r$n grasps $N in his huge gnarled hands and infuses $m with sin!\n\r",FALSE,lucifer,0,vict,TO_NOTVICT);
      act("\n\r$n grasps you in his huge gnarled hands and infuses you with sin!\n\r",0,lucifer,0,vict,TO_VICT);
      act("You grasp $N in your hands and infuse them with the power of the Damned!",0,lucifer,0,vict,TO_CHAR);
      damage(lucifer,vict,132,TYPE_UNDEFINED,DAM_NO_BLOCK);
      d=vict->desc;
      if(!d) return FALSE;
      if(d->connected!=CON_PLYNG) return FALSE; /* If char died */

      switch(number(1,7)) {
        case 1:

          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup("Deadly Sin - Wrath");
          tmp_enchantment->duration = 66;
          tmp_enchantment->func     = sin_wrath;
          enchantment_to_char(vict, tmp_enchantment, FALSE);
          act("$n has been touched by the Hand of Lucifer!\n\r",FALSE,vict,0,0,TO_ROOM);
          send_to_char("You feel rather strange and very nauseous.\n\r",vict);
          sprintf(buf4,"Hell Log Ench: [ %s just contracted WRATH at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
          log_s(buf4);
          break;

        case 2:

          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup("Deadly Sin - Lust");
          tmp_enchantment->duration = 66;
          tmp_enchantment->func     = sin_lust;
          enchantment_to_char(vict, tmp_enchantment, FALSE);
          act("$n has been touched by the Hand of Lucifer!\n\r",FALSE,vict,0,0,TO_ROOM);
          send_to_char("You feel rather strange and very nauseous.\n\r",vict);
          sprintf(buf4,"Hell Log Ench: [ %s just contracted LUST at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
          log_s(buf4);
          break;

        case 3:

          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup("Deadly Sin - Avarice");
          tmp_enchantment->duration = 66;
          tmp_enchantment->func     = sin_avarice;
          enchantment_to_char(vict, tmp_enchantment, FALSE);
          act("$n has been touched by the Hand of Lucifer!\n\r",FALSE,vict,0,0,TO_ROOM);
          send_to_char("You feel rather strange and very nauseous.\n\r",vict);
          sprintf(buf4,"Hell Log Ench: [ %s just contracted AVARICE at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
          log_s(buf4);
          break;

        case 4:

          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup("Deadly Sin - Gluttony");
          tmp_enchantment->duration = 66;
          tmp_enchantment->func     = sin_gluttony;
          enchantment_to_char(vict, tmp_enchantment, FALSE);
          act("$n has been touched by the Hand of Lucifer!\n\r",FALSE,vict,0,0,TO_ROOM);
          send_to_char("You feel rather strange and very nauseous.\n\r",vict);
          sprintf(buf4,"Hell Log Ench: [ %s just contracted GLUTTONY at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
          log_s(buf4);
          break;

        case 5:

          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup("Deadly Sin - Envy");
          tmp_enchantment->duration = 66;
          tmp_enchantment->func     = sin_envy;
          enchantment_to_char(vict, tmp_enchantment, FALSE);
          act("$n has been touched by the Hand of Lucifer!\n\r",FALSE,vict,0,0,TO_ROOM);
          send_to_char("You feel rather strange and very nauseous.\n\r",vict);
          sprintf(buf4,"Hell Log Ench: [ %s just contracted ENVY at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
          log_s(buf4);
          break;

        case 6:

          if(vict==lucifer->specials.fighting) break;
          d=vict->desc;
          if(!d) break;
          if(d->connected!=CON_PLYNG) break; /* If char died */
          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup("Deadly Sin - Pride");
          tmp_enchantment->duration = 66;
          tmp_enchantment->func     = sin_pride;
          enchantment_to_char(vict, tmp_enchantment, FALSE);
          act("$n has been touched by the Hand of Lucifer!\n\r",FALSE,vict,0,0,TO_ROOM);
          send_to_char("You feel rather strange and very nauseous.\n\r",vict);
          sprintf(buf4,"Hell Log Ench: [ %s just contracted PRIDE at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
          log_s(buf4);

          if(vict->specials.riding) stop_riding(vict,vict->specials.riding);
          char_from_room (vict);
          char_to_room (vict, real_room(MARKET));
          do_look(vict,"",CMD_LOOK);
          GET_POS(vict)=POSITION_SITTING;
          act("\n\rYou are frozen in place suddenly!\n\r",1,vict,0,0,TO_CHAR);
          act("\n\r$n hits the ground and freezes in a gaudy pose!\n\r",1,vict,0,0,TO_ROOM);
          break;

        case 7:

          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup("Deadly Sin - Sloth");
          tmp_enchantment->duration = 66;
          tmp_enchantment->func     = sin_sloth;
          enchantment_to_char(vict, tmp_enchantment, FALSE);
          act("$n has been touched by the Hand of Lucifer!\n\r",FALSE,vict,0,0,TO_ROOM);
          send_to_char("You feel rather strange and very nauseous.\n\r",vict);
          sprintf(buf4,"Hell Log Ench: [ %s just contracted SLOTH at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
          log_s(buf4);
          break;

        default:
          break;
      }
      return FALSE;
    }

  }
  if(cmd!=MSG_VIOLENCE) return FALSE;

                /* Lucifer's fung-type spec */
  if(lucifer && cmd==MSG_VIOLENCE && lucifer->specials.fighting && lucifer->equipment[WIELD]) {

    vict=lucifer->specials.fighting;
    stingnum=number(8,28);
    for(i = 0; i < stingnum; i++) {
      update_pos(vict);
      if(GET_POS(vict)!=POSITION_FIGHTING) return FALSE;
      dmg=number(69,82);

      switch(number(0,7)) {
        case 0:
          act("$n's Sword of Night slashes you viciously.",FALSE,lucifer,0,vict,TO_VICT);
          act("$n's Sword of Night slashes $N viciously.",FALSE,lucifer,0,vict,TO_NOTVICT);
          act("Your Sword of Night slashes $N viciously.",FALSE,lucifer,0,vict,TO_CHAR);
          damage(lucifer,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
          break;

        case 1:
          act("$n's Sword of Night grazes the back of your head!",FALSE,lucifer,0,vict,TO_VICT);
          act("$n's Sword of Night grazes the back of $N's head!",FALSE,lucifer,0,vict,TO_NOTVICT);
          act("Your Sword of Night grazes the back of $N's head!",FALSE,lucifer,0,vict,TO_CHAR);
          damage(lucifer,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
          break;

        case 2:
          act("$n's Sword of Night hacks into the small of your back with brutal force.",FALSE,lucifer,0,vict,TO_VICT);
          act("$n's Sword of Night hacks into the small of $N's back with brutal force.",FALSE,lucifer,0,vict,TO_NOTVICT);
          act("Your Sword of Night hacks into the small of $N's back with brutal force.",FALSE,lucifer,0,vict,TO_CHAR);
          damage(lucifer,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
          break;

        case 3:
          act("$n's Sword of Night carves deftly along the bridge of your nose.",FALSE,lucifer,0,vict,TO_VICT);
          act("$n's Sword of Night carves deftly along the bridge of $N's nose.",FALSE,lucifer,0,vict,TO_NOTVICT);
          act("Your Sword of Night carves deftly along the bridge of $N's nose.",FALSE,lucifer,0,vict,TO_CHAR);
          damage(lucifer,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
          break;

        case 4:
          act("$n's Sword of Night slashes at your groin!",FALSE,lucifer,0,vict,TO_VICT);
          act("$n's Sword of Night slashes at $N's groin!",FALSE,lucifer,0,vict,TO_NOTVICT);
          act("Your Sword of Night slashes at $N's groin!",FALSE,lucifer,0,vict,TO_CHAR);
          damage(lucifer,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
          break;

        case 5:
          act("$n's Sword of Night cuts deep into your ribcage, spurting blood.",FALSE,lucifer,0,vict,TO_VICT);
          act("$n's Sword of Night cuts deep into $N's ribcage, spurting blood.",FALSE,lucifer,0,vict,TO_NOTVICT);
          act("Your Sword of Night cuts deep into $N's ribcage, spurting blood.",FALSE,lucifer,0,vict,TO_CHAR);
          damage(lucifer,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
          break;

        case 6:
          act("$n's Sword of Night screams across the knuckles of your right hand.",FALSE,lucifer,0,vict,TO_VICT);
          act("$n's Sword of Night screams across the knuckles of $N's right hand.",FALSE,lucifer,0,vict,TO_NOTVICT);
          act("Your Sword of Night screams across the knuckles of $N's right hand.",FALSE,lucifer,0,vict,TO_CHAR);
          damage(lucifer,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
          break;

        case 7:
          act("$n's Sword of Night gets caught in your thigh as $e throws it wildly.",FALSE,lucifer,0,vict,TO_VICT);
          act("$n's Sword of Night gets caught in the thigh of $N as $e throws it wildly.",FALSE,lucifer,0,vict,TO_NOTVICT);
          act("Your Sword of Night gets caught in the thigh of $N as you throw it wildly.",FALSE,lucifer,0,vict,TO_CHAR);
          damage(lucifer,vict,dmg,TYPE_UNDEFINED,DAM_NO_BLOCK);
          break;

        default:
          break;

      }/* end switch */
    } /* end for */
    return FALSE;
  }
  return FALSE;
}

/* Hell Room Specs */


int hell_ledge(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if (!ch) return FALSE;

  if((cmd==CMD_CLIMB) && chance(50)) {
    act("The ledge crumbles away as you try scaling it!",1,ch,0,0,TO_CHAR);
    act("$n slips, and falls to the ground below!",1,ch,0,0,TO_ROOM);
    if(ch->specials.riding) stop_riding(ch,ch->specials.riding);
    char_from_room (ch);
    char_to_room (ch, real_room(FALL_ROOM));
    do_look(ch,"",CMD_LOOK);
    act("You smash into the ground in a sickly heap!",1,ch,0,0,TO_CHAR);
    act("$n hits the ground with a sickly thud!",1,ch,0,0,TO_ROOM);
    damage(ch,ch,number(200,300),TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
  }
  return FALSE;
}


int blood_lake(int room, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*next_vict;
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         obj->log=1;
         log_s(buf);

         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if(cmd==MSG_MOBACT && count_mortals_real_room(room)>0) {
    for(vict = world[room].people; vict;vict = next_vict) {
      next_vict = vict->next_in_room;
      if(IS_MORTAL(vict)) {
        act("Your body cries out in agony as the boiling blood sears your flesh.",FALSE,vict,0,0,TO_CHAR);
        damage(vict, vict, 50,TYPE_UNDEFINED,DAM_NO_BLOCK);
      }
    }
    return FALSE;
  }

  if(ch && cmd>0 && chance(30) && IS_MORTAL(ch)) {
    act("Your body cries out in agony as the boiling blood sears your flesh.",FALSE,ch,0,0,TO_CHAR);
    act("$n cries out in agony as $s flesh is seared by the boiling blood.",FALSE,ch,0,0,TO_ROOM);
    if(saves_spell(ch,SAVING_SPELL,GET_LEVEL(ch)))
      damage(ch,ch,25,TYPE_UNDEFINED,DAM_NO_BLOCK);
    else
      damage(ch,ch,50,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
  }
  return FALSE;
}


int boulders(int room, CHAR *ch, int cmd, char *arg) {
  struct descriptor_data *d;
  CHAR *vict, *next_vict;
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if(cmd==MSG_RECONNECT) {
    for(vict = world[room].people; vict;vict = next_vict) {
      next_vict = vict->next_in_room;
      if(IS_MORTAL(vict)) {
        d=vict->desc;
        if(d && d->connected==CON_PLYNG) {
          char_from_room(vict);
          char_to_room(vict, real_room(25434));
          WAIT_STATE(vict,PULSE_VIOLENCE*3);
        }
      }
    }
    return FALSE;
  }

  if (!ch) return FALSE;
  if (!IS_MORTAL(ch) || IS_NPC(ch)) return FALSE;
  if (cmd==CMD_EAST && number(0,2) && GET_HIT(ch) > 100 && (time_info.hours == 0 || time_info.hours == 6 || time_info.hours == 12 || time_info.hours == 18)) {
    act("You are run over by a group of workers pushing a huge boulder!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n is flattened by a group of workers pushing a huge boulder!", FALSE, ch, 0, 0, TO_ROOM);
    damage(ch, ch, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
    WAIT_STATE(ch,PULSE_VIOLENCE*3);
    GET_MOVE(ch) = 0;
    if(chance(33)) {
      GET_MOVE(ch)=4;
      do_move(ch,"",CMD_WEST);
    }
    else {
      act("A loud rumbling noise shakes the ground as it reverberates off the canyon walls.",FALSE,ch,0,0,TO_ROOM);
    }
    return TRUE;
  }
  return FALSE;
}


int boulders2(int room, CHAR *ch, int cmd, char *arg) {
  struct descriptor_data *d;
  CHAR *vict, *next_vict;
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if(cmd==MSG_RECONNECT) {
    for(vict = world[room].people; vict;vict = next_vict) {
      next_vict = vict->next_in_room;
      if(IS_MORTAL(vict)) {
        d=vict->desc;
        if(d && d->connected==CON_PLYNG) {
          char_from_room(vict);
          char_to_room(vict, real_room(25434));
          WAIT_STATE(vict,PULSE_VIOLENCE*3);
        }
      }
    }
    return FALSE;
  }

  if (!ch) return FALSE;
  if (!IS_MORTAL(ch) || IS_NPC(ch)) return FALSE;
  if (cmd==CMD_DOWN && !number(0,2) && GET_HIT(ch) > 200) {
    act("You are run over by a group of workers pushing a huge boulder!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n is flattened by a group of workers pushing a huge boulder!", FALSE, ch, 0, 0, TO_ROOM);
    damage(ch, ch, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
    WAIT_STATE(ch,PULSE_VIOLENCE*5);
    GET_MOVE(ch) = 0;
    if(chance(33)) {
      GET_MOVE(ch)=4;
      do_move(ch,"",CMD_WEST);
    }
    return TRUE;
  }
  return FALSE;
}


int ouchie_flakes(int room, CHAR *ch, int cmd, char *arg) {
  struct descriptor_data *d;
  CHAR *vict, *next_vict;
  char *tmp_string;
  char *skip_spaces (char*);
  char buf[MIL];
  OBJ *obj, *next_obj;
  int v_room, i, qend, spl, vroom;
  v_room = world[room].number;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if(cmd==MSG_MOBACT && count_mortals_real_room(room)>0) {
    for(vict = world[room].people; vict;vict = next_vict) {
      next_vict = vict->next_in_room;
      if(IS_MORTAL(vict)) {
        act("\n\rBurning flakes of sulphur fall from the sky and burn your flesh!\n\r",FALSE,vict,0,0,TO_CHAR);
        damage(vict, vict, number(50,100),TYPE_UNDEFINED,DAM_NO_BLOCK);
      }
    }
    return FALSE;
  }

  if(cmd==MSG_RECONNECT) {
    for(vict = world[room].people; vict;vict = next_vict) {
      next_vict = vict->next_in_room;
      if(IS_MORTAL(vict)) {
        d=vict->desc;
        if(d && d->connected==CON_PLYNG) {
          char_from_room(vict);
          char_to_room(vict, real_room(25500));
          WAIT_STATE(vict,PULSE_VIOLENCE*3);
        }
      }
    }
    return FALSE;
  }

  if(ch && cmd==MSG_ENTER) {
    switch(v_room) {
      case 25502:
      case 25504:
      case 25505:
      case 25507:
      case 25508:
      case 25510:
      case 25511:
      case 25513:
      case 25514:
        if(!IS_NPC(ch)) {
          i=number(10,18);
          for(vict = world[CHAR_REAL_ROOM(ch)].people; vict;vict = next_vict) {
            next_vict=vict->next_in_room;
            if(chance(70) && IS_MORTAL(vict)) {
              act("\n\rA cloud of burning sulphur descends and obscures your path !!\n\r",FALSE,vict,0,0,TO_CHAR);
              WAIT_STATE(vict,PULSE_VIOLENCE*i);
            }
          }
        }
        break;

      default:
        break;
    }
    return FALSE;
  }

  if(ch && cmd==CMD_CAST) {
    if(!ch) return FALSE;
    if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA)) return FALSE;
    if(GET_CLASS(ch)==CLASS_THIEF ||
       GET_CLASS(ch)==CLASS_WARRIOR ||
       GET_CLASS(ch)==CLASS_NOMAD ||
       GET_CLASS(ch)==CLASS_PALADIN ||
       GET_CLASS(ch)==CLASS_ANTI_PALADIN ||
       GET_CLASS(ch)==CLASS_MAGIC_USER ||
       GET_CLASS(ch)==CLASS_BARD ||
       GET_CLASS(ch)==CLASS_COMMANDO ||
       GET_CLASS(ch)==CLASS_NINJA) return FALSE;

    tmp_string = str_dup (arg);
    tmp_string = skip_spaces(tmp_string);

    if (!(*tmp_string)) return FALSE;
    if (*tmp_string != '\'') return FALSE;

    for (qend=1; *(tmp_string+qend) && (*(tmp_string+qend) != '\'') ; qend++)
      *(tmp_string+qend) = LOWER(*(tmp_string+qend));

    if (*(tmp_string+qend) != '\'') return FALSE;

    spl = old_search_block(tmp_string, 1, qend-1,spells, 0);

    switch(spl) {

      case SPELL_SUMMON:
        if(!IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA)) {
          if(!IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, DARK)) {
            SET_BIT(world[CHAR_REAL_ROOM(ch)].room_flags, DARK);
            act("The desert plain darkens beneath a cloud of evil..",FALSE,ch,0,0,TO_CHAR);
            act("The desert plain darkens beneath a cloud of evil..",FALSE,ch,0,0,TO_ROOM);
            break;
          }
          SET_BIT(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA);
          act("A sulphur stench pervades the air and prevents any further summons!",FALSE,ch,0,0,TO_CHAR);
          act("$n's spell calls forth a stench of sulphur, preventing further summons!",FALSE,ch,0,0,TO_ROOM);
        }
        break;
      default:
        break;
    }
    return FALSE;
  }


  return FALSE;
}


int typos(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if (!ch) return FALSE;
  if (!IS_MORTAL(ch) || IS_NPC(ch)) return FALSE;
  if(cmd<1) return FALSE;
  if(!number(0,2)) {
    send_to_char(unknownCMD[number(0,15)], ch);
     return TRUE;
  }
  return FALSE;
}


int pit_spec(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if (!ch) return FALSE;
  if (cmd == CMD_UP && IS_MORTAL(ch)) {
    if (chance(30)) {
      act("The heat of the walls surrounding you burn brightly and fling you back into the Pit!",FALSE,ch,0,0,TO_CHAR);
      act("The heat of the walls surrounding the Pit causes $n to fall back to the floor!",FALSE,ch,0,0,TO_ROOM);
      damage(ch,ch,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return TRUE;
    }
    return FALSE;
  }
  return FALSE;
}


int spike_spec(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if (!ch) return FALSE;
  if (cmd == CMD_UP && IS_MORTAL(ch)) {
    if (chance(30)) {
      act("You slip and graze yourself on a sharpened steel spike!",FALSE,ch,0,0,TO_CHAR);
      act("$n slips and grazes $mself on a sharpened steel spike!",FALSE,ch,0,0,TO_ROOM);
      damage(ch,ch,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return TRUE;
    }
    return FALSE;
  }
  return FALSE;
}


int techno_spec(int room, CHAR *ch, int cmd, char *arg) {
  char buf[MIL], buf2[MIL];
  OBJ *obj, *next_obj;
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf2,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf2);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf2,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf2);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if (!ch) return FALSE;
  if (cmd == CMD_UP && IS_MORTAL(ch)) {
    act("You cannot seem to climb past the hoards of Techno-Peasants under your burden!",FALSE,ch,0,0,TO_CHAR);
    act("$n tries fruitlessly to pass by the hoards of Techno-Peasants.",FALSE,ch,0,0,TO_ROOM);
    return TRUE;
  }

  if (cmd == CMD_REMOVE && !(strcmp(arg, " all"))) {
    act("\n\rAs you begin to remove your equipment, the hoards of Techno-Peasants are distracted and allow you to pass out of the Pit.",FALSE,ch,0,0,TO_CHAR);
    act("\n\r$n dashes past the hoards of Techno-Peasants while distracting them with $s gear.",FALSE,ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch,real_room(TECHNO_TARGET));
    do_look(ch,"",CMD_LOOK);
    act("\n\r",0,ch,0,0,TO_CHAR);
    sprintf(buf,"remove all");
    command_interpreter(ch,buf);
    act("\n\r$n leaps from the Pit below!",0,ch,0,0,TO_ROOM);
    return TRUE;
  }
  return FALSE;
}


int floating_rooms(int room, CHAR *ch, int cmd, char *arg) {
  CHAR *vict, *next_vict;
  char buf[MIL],buf2[MIL],buf3[MIL];
  OBJ *o, *tmp, *obj, *next_obj;
  static int second_room=0, third_room=0, fourth_room=0;
  int v_room, i, vroom;
  v_room = world[room].number;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34 || V_OBJ(obj)==25336) break;  /* break on statue of peace, wall of thorns, and Token */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf3,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf3);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf3,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf3);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if (IS_NPC(ch)) return FALSE;

  switch(v_room) {
  case BELT_ROOM:
   if(cmd==CMD_DOWN) {
     o=ch->equipment[13];
     if(o && V_OBJ(o)==25305) {
       act("Your $p lifts you gently off the ground and you float over the cliff.\n\r",0,ch,o,0,TO_CHAR);
       act("$n is lifted off $s feet by $s $p and lowered over the cliff.",0,ch,o,0,TO_ROOM);
       act("$n's wake pulls you down into the void below.",0,ch,o,0,TO_ROOM);
       for(vict= world[room].people;vict;vict = next_vict) {
         next_vict = vict->next_in_room;
         if(!IS_NPC(vict)) {
           char_from_room(vict);
           char_to_room(vict,real_room(ROOM_TWO));
           do_look(vict,"",CMD_LOOK);
           act("$n floats down gently from above.",0,vict,0,0,TO_ROOM);
         }
       }
       return TRUE;
     }
     else {
       act("You step too close to the edge and begin falling uncontrollably!\n\r",0,ch,0,0,TO_CHAR);
       act("$n steps too close to the edge and falls down the hole!",0,ch,0,0,TO_ROOM);
       char_from_room(ch);
       char_to_room(ch,real_room(BELT_TARGET));

       look_in_room(ch,world[real_room(ROOM_TWO)].number);
       look_in_room(ch,world[real_room(ROOM_THREE)].number);
       look_in_room(ch,world[real_room(ROOM_FOUR)].number);
       look_in_room(ch,world[real_room(BELT_TARGET)].number);

       act("\n\rYou slam into the ground so hard and so fast you hardly have time to feel every bone in your body shattering.",0,ch,0,0,TO_CHAR);
       act("$n turns into a red splat as $e smashes headfirst into the hard ground!",0,ch,0,0,TO_ROOM);
       damage(ch,ch,20000,TYPE_UNDEFINED,DAM_NO_BLOCK);
       WAIT_STATE(ch, PULSE_VIOLENCE*10);
       return TRUE;
     }
   }
   break;

   case ROOM_TWO:

    if (cmd == CMD_REMOVE && ch->equipment[13]) {
      o=ch->equipment[13];
      if(o && V_OBJ(o)==25305) {
        one_argument (arg, buf);
        if (isname (buf, OBJ_NAME(o))) {
          act("You remove the belt and go plummeting to the ground below!!\n\r",0,ch,0,0,TO_CHAR);
          act("$n removes the belt, causing everyone to plummet to the ground below!!\n\r",0,ch,0,0,TO_ROOM);
          obj_to_char(unequip_char(ch,WEAR_WAIST),ch);

          for(vict= world[room].people;vict;vict = next_vict) {
            next_vict = vict->next_in_room;
            if(!IS_NPC(vict)) {
              char_from_room(vict);
              char_to_room(vict,real_room(BELT_TARGET));

              look_in_room(vict,world[real_room(ROOM_THREE)].number);
              look_in_room(vict,world[real_room(ROOM_FOUR)].number);
              look_in_room(vict,world[real_room(BELT_TARGET)].number);

              act("\n\rYou slam into the ground so hard and so fast you hardly have time to feel every bone in your body shattering.",0,vict,0,0,TO_CHAR);
              act("$n turns into a red splat as $e smashes headfirst into the hard ground!",0,vict,0,0,TO_ROOM);
              damage(vict,vict,20000,TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*7);
            }
          }
          return TRUE;
        }
        else if (!strcmp (buf, "all")) {
          for (i=0; i < MAX_WEAR; i++) {
            tmp = ch->equipment[i];
            if (tmp) {
              sprintf(buf2,"remove %s",OBJ_NAME(tmp));
              command_interpreter(ch,buf2);
            }
          }
          return TRUE;
        }
      }
    }

    if(cmd==MSG_MOBACT) {
      second_room++;
      if(second_room==5) {
        second_room=0;
        send_to_room("\n\rA warm gust of wind gently floats you closer to the ground.\n\r\n\r",room);
        for(vict= world[room].people;vict;vict = next_vict) {
          next_vict = vict->next_in_room;
          if(!IS_NPC(vict)) {
            char_from_room(vict);
            char_to_room(vict,real_room(ROOM_THREE));
            do_look(vict,"",CMD_LOOK);
            act("$n floats gently from above.",0,vict,0,0,TO_ROOM);
          }
        }
      }
      return FALSE;
    }
   break;

   case ROOM_THREE:

    if (cmd == CMD_REMOVE && ch->equipment[13]) {
      o=ch->equipment[13];
      if(o && V_OBJ(o)==25305) {
        one_argument (arg, buf);
        if (isname (buf, OBJ_NAME(o))) {
          send_to_char ("You remove the belt and go plummeting to the ground below!!\n\r",ch);
          obj_to_char(unequip_char(ch,WEAR_WAIST),ch);

          for(vict= world[room].people;vict;vict = next_vict) {
            next_vict = vict->next_in_room;
            if(!IS_NPC(vict)) {
              char_from_room(vict);
              char_to_room(vict,real_room(BELT_TARGET));

              look_in_room(vict,world[real_room(ROOM_FOUR)].number);
              look_in_room(vict,world[real_room(BELT_TARGET)].number);

              act("\n\rYou slam into the ground so hard and so fast you hardly have time to feel every bone in your body shattering.",0,vict,0,0,TO_CHAR);
              act("$n turns into a red splat as $e smashes headfirst into the hard ground!",0,vict,0,0,TO_ROOM);
              damage(vict,vict,20000,TYPE_UNDEFINED,DAM_NO_BLOCK);
              WAIT_STATE(vict, PULSE_VIOLENCE*4);
            }
          }
          return TRUE;
        }
        else if (!strcmp (buf, "all")) {
          for (i=0; i < MAX_WEAR; i++) {
            tmp = ch->equipment[i];
            if (tmp) {
              sprintf(buf2,"remove %s",OBJ_NAME(tmp));
              command_interpreter(ch,buf2);
            }
          }
          return TRUE;
        }
      }
    }

    if(cmd==MSG_MOBACT) {
      third_room++;
      if(third_room==10) {
        third_room=0;
        send_to_room("\n\rA warm gust of wind gently floats you closer to the ground.\n\r\n\r",room);
        for(vict= world[room].people;vict;vict = next_vict) {
          next_vict = vict->next_in_room;
          if(!IS_NPC(vict)) {
            char_from_room(vict);
            char_to_room(vict,real_room(ROOM_FOUR));
            do_look(vict,"",CMD_LOOK);
            act("$n floats gently from above.",0,vict,0,0,TO_ROOM);
          }
        }
      }
      return FALSE;
    }
   break;

   case ROOM_FOUR:

    if (cmd == CMD_REMOVE && ch->equipment[13]) {
      o=ch->equipment[13];
      if(o && V_OBJ(o)==25305) {
        one_argument (arg, buf);
        if (isname (buf, OBJ_NAME(o))) {
          send_to_char ("You remove the belt and go plummeting to the ground below!!\n\r",ch);
          obj_to_char(unequip_char(ch,WEAR_WAIST),ch);
          char_from_room(ch);
          char_to_room(ch,real_room(BELT_TARGET));

          for(vict= world[room].people;vict;vict = next_vict) {
            next_vict = vict->next_in_room;
            if(!IS_NPC(vict)) {
              char_from_room(vict);
              char_to_room(vict,real_room(BELT_TARGET));

              look_in_room(vict,world[real_room(BELT_TARGET)].number);

              act("\n\rYou slam into the ground, barely surviving the impact.",0,vict,0,0,TO_CHAR);
              act("$n smacks into the ground with great force!",0,vict,0,0,TO_ROOM);
              GET_HIT(vict)=1;
              WAIT_STATE(vict, PULSE_VIOLENCE*3);
            }
          }
          return TRUE;
        }
        else if (!strcmp (buf, "all")) {
          for (i=0; i < MAX_WEAR; i++) {
            tmp = ch->equipment[i];
            if (tmp) {
              sprintf(buf2,"remove %s",OBJ_NAME(tmp));
              command_interpreter(ch,buf2);
            }
          }
          return TRUE;
        }
      }
    }

    if(cmd==MSG_MOBACT) {
      fourth_room++;
      if(fourth_room==15) {
        fourth_room=0;
        send_to_room("\n\rA warm gust of wind gently floats you to the base of the cliff.\n\r\n\r",room);
        for(vict= world[room].people;vict;vict = next_vict) {
          next_vict = vict->next_in_room;
          if(!IS_NPC(vict)) {
            char_from_room(vict);
            char_to_room(vict,real_room(BELT_TARGET));
            do_look(vict,"",CMD_LOOK);
            act("$n floats gently from above.",0,vict,0,0,TO_ROOM);
          }
        }
      }
      return FALSE;
    }
   break;

   default:
   break;
  }
 return FALSE;
}


int hell_rooms(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *obj = NULL, *next_obj = NULL;
  char buf[MAX_INPUT_LENGTH], buf2[MIL];
  int v_room, vroom;
  v_room = world[room].number;

  if(cmd==MSG_TICK) {
    switch(v_room) {
      case 25540:
        if(IS_NIGHT) {
          if(IS_SET(world[room].room_flags,ARENA))
            REMOVE_BIT(world[room].room_flags,ARENA);
        } else {
          if(!IS_SET(world[room].room_flags,ARENA))
            SET_BIT(world[room].room_flags,ARENA);
        }
        break;
      default:
        break;
    }

    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf2,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf2);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf2,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf2);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }
                      /* staff use is 2/3 unsuccessful */

  if(!ch) return FALSE;

  if (cmd == CMD_USE && !IS_NPC(ch)) {
    if (EQ(ch,HOLD))  obj = EQ(ch,HOLD);
    if (!EQ(ch,HOLD) || EQ(ch,HOLD) != obj) return FALSE;
    one_argument (arg, buf);
    if (!isname(buf, OBJ_NAME(obj))) return FALSE;
    if (!(OBJ_TYPE(obj) == ITEM_STAFF) || (OBJ_TYPE(obj) == ITEM_WAND)) return FALSE;
    if (chance(66)) {
      act("The flames of Hell burn brightly, eclipsing $p's power!",FALSE,ch,obj,0,TO_CHAR);
      act("The power of Hell disintegrates $p in $n's hands!",FALSE,ch,obj,0,TO_ROOM);
      extract_obj(unequip_char(ch,HOLD));
      return TRUE;
    }
    return FALSE;
  }
  return FALSE;
}


int woods_a_spec(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if(!ch) return FALSE;

  if(IS_NPC(ch)) return FALSE;

  if(cmd==CMD_NORTH && !ch->master) {
    switch(number(0,3))
    {
    case 0:
      world[room].dir_option[NORTH]->to_room_r = real_room(25448);
      break;
    case 1:
      world[room].dir_option[NORTH]->to_room_r = real_room(25350);
      break;
    case 2:
      world[room].dir_option[NORTH]->to_room_r = real_room(25362);
      break;
    case 3:
      world[room].dir_option[NORTH]->to_room_r = real_room(25374);
      break;
    default:
      break;
    }
  }

  if(cmd==CMD_EAST && !ch->master) {
    switch(number(0,2))
    {
    case 0:
      world[room].dir_option[EAST]->to_room_r = real_room(25451);
      break;
    case 1:
      world[room].dir_option[EAST]->to_room_r = real_room(25356);
      break;
    case 2:
      world[room].dir_option[EAST]->to_room_r = real_room(25368);
      break;
    default:
      break;
    }
  }

  return FALSE;
}


int woods_b_spec(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if(!ch) return FALSE;

  if(IS_NPC(ch)) return FALSE;

  if(world[CHAR_REAL_ROOM(ch)].number==25379) {
    if(cmd==CMD_DOWN && !ch->master) {
      switch(number(0,1))
      {
      case 0:
        world[room].dir_option[DOWN]->to_room_r = real_room(25489);
        break;
      case 1:
        world[room].dir_option[DOWN]->to_room_r = real_room(25490);
        break;
      default:
        break;
      }
    }
    return FALSE;
  }

  if(world[CHAR_REAL_ROOM(ch)].number==25367) {
    if(cmd==CMD_DOWN && !ch->master) {
      switch(number(0,2))
      {
      case 0:
        world[room].dir_option[DOWN]->to_room_r = real_room(25491);
        break;
      case 1:
        world[room].dir_option[DOWN]->to_room_r = real_room(25492);
        break;
      case 2:
        world[room].dir_option[DOWN]->to_room_r = real_room(25493);
        break;
      default:
        break;
      }
    }
    return FALSE;
  }

  if(world[CHAR_REAL_ROOM(ch)].number==25355) {
    if(cmd==CMD_DOWN && !ch->master) {
      switch(number(0,1))
      {
      case 0:
        world[room].dir_option[DOWN]->to_room_r = real_room(25494);
        break;
      case 1:
        world[room].dir_option[DOWN]->to_room_r = real_room(25495);
        break;
      default:
        break;
      }
    }
    return FALSE;
  }

  if(world[CHAR_REAL_ROOM(ch)].number==25456) {
    if(cmd==CMD_DOWN && !ch->master) {
      switch(number(0,2))
      {
      case 0:
        world[room].dir_option[DOWN]->to_room_r = real_room(25460);
        break;
      case 1:
        world[room].dir_option[DOWN]->to_room_r = real_room(25467);
        break;
      case 2:
        world[room].dir_option[DOWN]->to_room_r = real_room(25468);
        break;
      default:
        break;
      }
    }
    return FALSE;
  }

  if(world[CHAR_REAL_ROOM(ch)].number==25459) {
    if(cmd==CMD_DOWN && !ch->master) {
      switch(number(0,2))
      {
      case 0:
        world[room].dir_option[DOWN]->to_room_r = real_room(25461);
        break;
      case 1:
        world[room].dir_option[DOWN]->to_room_r = real_room(25469);
        break;
      case 2:
        world[room].dir_option[DOWN]->to_room_r = real_room(25470);
        break;
      default:
        break;
      }
    }
    return FALSE;
  }

  if(world[CHAR_REAL_ROOM(ch)].number==25361) {
    if(cmd==CMD_DOWN && !ch->master) {
      switch(number(0,1))
      {
      case 0:
        world[room].dir_option[DOWN]->to_room_r = real_room(25496);
        break;
      case 1:
        world[room].dir_option[DOWN]->to_room_r = real_room(25497);
        break;
      default:
        break;
      }
    }
    return FALSE;
  }

  if(world[CHAR_REAL_ROOM(ch)].number==25373) {
    if(cmd==CMD_DOWN && !ch->master) {
      switch(number(0,1))
      {
      case 0:
        world[room].dir_option[DOWN]->to_room_r = real_room(25498);
        break;
      case 1:
        world[room].dir_option[DOWN]->to_room_r = real_room(25499);
        break;
      default:
        break;
      }
    }
    return FALSE;
  }

  return FALSE;
}


int woods_c_spec(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if(!ch) return FALSE;

  if(IS_NPC(ch)) return FALSE;

  if(cmd==CMD_UP && !ch->master) {
    switch(number(0,6))
    {
    case 0:
      world[room].dir_option[UP]->to_room_r = real_room(25379);
      break;
    case 1:
      world[room].dir_option[UP]->to_room_r = real_room(25367);
      break;
    case 2:
      world[room].dir_option[UP]->to_room_r = real_room(25355);
      break;
    case 3:
      world[room].dir_option[UP]->to_room_r = real_room(25456);
      break;
    case 4:
      world[room].dir_option[UP]->to_room_r = real_room(25459);
      break;
    case 5:
      world[room].dir_option[UP]->to_room_r = real_room(25361);
      break;
    case 6:
      world[room].dir_option[UP]->to_room_r = real_room(25373);
      break;
    default:
      break;
    }
  }

  return FALSE;
}


int woods_d_spec(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *obj, *next_obj;
  char buf[MIL];
  int vroom;

  if(cmd==MSG_TICK) {
    if(world[room].contents) {
      for(obj=world[room].contents;obj;obj=next_obj) {
         next_obj=obj->next_content;
         if(V_OBJ(obj)==9 || V_OBJ(obj)==34) break;  /* break on statue of peace and wall of thorns */
         send_to_room("\n\rThe Grim Reaper descends, stealing something from the surroundings.\n\r",room);
         vroom=obj->in_room_v;
         obj_from_room(obj);
         sprintf(buf,"Hell Log: %s removed from room %d.",OBJ_SHORT(obj),vroom);
         log_s(buf);
         obj_to_room(obj,real_room(REAPER_ROOM));
         sprintf(buf,"Hell Log: %s placed in Reaper's Hold.",OBJ_SHORT(obj));
         log_s(buf);
         obj->log=1;
         break;
      }
      return FALSE;
    }
    return FALSE;
  }

  if(!ch) return FALSE;

  if(IS_NPC(ch)) return FALSE;

  if(cmd==CMD_EAST && !ch->master) {
    switch(number(0,4))
    {
    case 0:
      world[room].dir_option[EAST]->to_room_r = real_room(25483);
      break;
    case 1:
      world[room].dir_option[EAST]->to_room_r = real_room(25484);
      break;
    case 2:
      world[room].dir_option[EAST]->to_room_r = real_room(25485);
      break;
    case 3:
      world[room].dir_option[EAST]->to_room_r = real_room(25486);
      break;
    case 4:
      world[room].dir_option[EAST]->to_room_r = real_room(25487);
      break;
    default:
      break;
    }
  }

  return FALSE;
}



void assign_hell(void) {
int i;
for(i = 25500; i < 25515; i++) assign_room(i, ouchie_flakes);
for(i = 25435; i < 25441; i++) assign_room(i, boulders);
for(i = 25423; i < 25426; i++) assign_room(i, blood_lake);
for(i = 25516; i < 25527; i++) assign_room(i, hell_rooms);
for(i = 25535; i < 25542; i++) assign_room(i, hell_rooms);
for(i = 25489; i <= 25499; i++) assign_room(i, woods_c_spec);
for(i = 25302; i < 25324; i++) assign_room(i, hell_rooms);
for(i = 25325; i < 25350; i++) assign_room(i, hell_rooms);
for(i = 25462; i < 25467; i++) assign_room(i, hell_rooms);
for(i = 25483; i < 25489; i++) assign_room(i, hell_rooms);
for(i = 25432; i < 25435; i++) assign_room(i, hell_rooms);
for(i = 25350; i < 25355; i++) assign_room(i, hell_rooms);
for(i = 25356; i < 25361; i++) assign_room(i, hell_rooms);
for(i = 25362; i < 25367; i++) assign_room(i, hell_rooms);
for(i = 25368; i < 25373; i++) assign_room(i, hell_rooms);
for(i = 25374; i < 25379; i++) assign_room(i, hell_rooms);
for(i = 25380; i < 25382; i++) assign_room(i, hell_rooms);
for(i = 25417; i < 25420; i++) assign_room(i, hell_rooms);
for(i = 25421; i < 25423; i++) assign_room(i, hell_rooms);
for(i = 25445; i < 25447; i++) assign_room(i, hell_rooms);
for(i = 25448; i < 25456; i++) assign_room(i, hell_rooms);
for(i = 25457; i < 25459; i++) assign_room(i, hell_rooms);
for(i = 25471; i < 25473; i++) assign_room(i, hell_rooms);
for(i = 25474; i < 25476; i++) assign_room(i, hell_rooms);
for(i = 25477; i < 25479; i++) assign_room(i, hell_rooms);
for(i = 25480; i < 25482; i++) assign_room(i, hell_rooms);
for(i = 25527; i < 25530; i++) assign_room(i, pit_spec);
for(i = 25442; i < 25445; i++) assign_room(i, floating_rooms);
assign_room(25426, hell_rooms);
assign_room(25428, hell_rooms);
assign_room(25430, hell_rooms);
assign_room(25420, blood_lake);
assign_room(25427, blood_lake);
assign_room(25429, blood_lake);
assign_room(HELL_LEDGE, hell_ledge);
assign_room(CANYON_END, boulders2);
assign_room(TYPO_PIT, typos);
assign_room(BELT_ROOM, floating_rooms);
assign_room(WOODS_A, woods_a_spec);
assign_room(25379, woods_b_spec);
assign_room(25367, woods_b_spec);
assign_room(25355, woods_b_spec);
assign_room(25456, woods_b_spec);
assign_room(25459, woods_b_spec);
assign_room(25361, woods_b_spec);
assign_room(25373, woods_b_spec);
assign_room(25460, woods_c_spec);
assign_room(25467, woods_c_spec);
assign_room(25468, woods_c_spec);
assign_room(25461, woods_c_spec);
assign_room(25469, woods_c_spec);
assign_room(25470, woods_c_spec);
assign_room(25473, woods_d_spec);
assign_room(25476, woods_d_spec);
assign_room(25479, woods_d_spec);
assign_room(25482, woods_d_spec);
assign_room(25531, pit_spec);
assign_room(25533, pit_spec);
assign_room(SPIKE_ROOM, spike_spec);
assign_room(TECHNO_PIT, techno_spec);

assign_mob(DGUARD_ONE, Bridge_Guard);
assign_mob(DGUARD_TWO, Bridge_Guard);
assign_mob(DGUARD_THREE, Bridge_Guard);
assign_mob(DGUARD_FOUR, Bridge_Guard);
assign_mob(DGUARD_FIVE, Bridge_Guard);
assign_mob(DGUARD_SIX, Bridge_Guard);
assign_mob(DGUARD_SEVEN, Bridge_Guard);
assign_mob(DGUARD_EIGHT, Bridge_Guard);
assign_mob(DGUARD_NINE, Bridge_Guard);
assign_mob(MINOS, minos);
assign_mob(GRIM_REAPER, grim_reaper);
assign_mob(FAYN, fayn);
assign_mob(MURDERER, murderer);
assign_mob(WIND_DEVIL, Wind_Devil);
assign_mob(WATER_WEIRD, water_weird);
assign_mob(WATER_WEIRD_B, water_weird);
assign_mob(PAOLO, Paolo_spec);
assign_mob(LAKE_GUARD, lake_guard);
assign_mob(CHARYBDIS, Charybdis);
assign_mob(LIZARD, lizard);
assign_mob(DISEASED_CREATURE, diseased_creature);
assign_mob(GLUTTON, glutton);
assign_mob(GLUTTON_B, glutton);
assign_mob(CIACCO, Ciacco);
assign_mob(PAGAN, Virtuous_Pagan);
assign_mob(GERYON, Geryon);
assign_mob(LEMURE, Lemure);
assign_mob(FLAME,Flame);
assign_mob(SORCERESS, Sorceress_spec);
assign_mob(SORCERESS_B, Sorceress_B_spec);
assign_mob(SERPENT, Serpent_spec);
assign_mob(INCUBUS, Incubus_spec);
assign_mob(ROBE, Robe_spec);
assign_mob(LUCIFER, Lucifer_spec);

assign_obj(NIGHT_SWORD, Night_Sword);
assign_obj(NIGHT_SWORD_B, Night_Sword);
assign_obj(LUCIFER_SWORD, Lucifer_Sword);
assign_obj(FILTHY_BRACELET, Filthy_Bracelet);
assign_obj(FILTHY_BRACELET_B, Filthy_Bracelet);
assign_obj(DOOM_GAUNTLETS, Doom_Gauntlets);
assign_obj(DOOM_GAUNTLETS_B, Doom_Gauntlets);
assign_obj(CLOAK_FLAME, Flame_Cloak);
assign_obj(CLOAK_FLAME_B, Flame_Cloak);
assign_obj(HORN_GERYON, Geryon_Horn);
assign_obj(HORN_GERYON_B, Geryon_Horn);
assign_obj(UNGUIS, Unguis);
assign_obj(UNGUIS_B, Unguis);
assign_obj(RING_AGONY, Ring_Agony);
assign_obj(RING_AGONY_B, Ring_Agony);
assign_obj(HOOVES_SATAN, Hooves_Satan);
assign_obj(HOOVES_SATAN_B, Hooves_Satan);
assign_obj(IRON_BOOTS, Iron_Boots);
assign_obj(IRON_BOOTS_B, Iron_Boots);
assign_obj(ACHROMATIC_GEM, Achromatic_Gem);
assign_obj(ACHROMATIC_GEM_B, Achromatic_Gem);
assign_obj(HORNS_LUCIFER, Lucifer_Horns);
assign_obj(HORNS_LUCIFER_B, Lucifer_Horns);
assign_obj(RING_ACCURSED, Ring_Accursed);
assign_obj(RING_ACCURSED_B, Ring_Accursed);
assign_obj(RING_DESPAIR, Ring_Despair);
assign_obj(RING_DESPAIR_B, Ring_Despair);
assign_obj(CROSS_INVERTED, Inverted_Cross);
assign_obj(CROSS_INVERTED_B, Inverted_Cross);
assign_obj(WINGS_LUCIFER, Lucifer_Wings);
assign_obj(WINGS_LUCIFER_B, Lucifer_Wings);
assign_obj(BARRIER_DAEMONIC, Daemonic_Barrier);
assign_obj(BARRIER_DAEMONIC_B, Daemonic_Barrier);
assign_obj(SHADOW_MOON, Shadow_Moon);
assign_obj(SHADOW_MOON_B, Shadow_Moon);
assign_obj(TAIL_SERPENT, Serpent_Tail);
assign_obj(TAIL_SERPENT_B, Serpent_Tail);
assign_obj(CIRCLET, Circlet);
assign_obj(CIRCLET_B, Circlet);
assign_obj(TOKEN_A, Charon_Token);
assign_obj(TOKEN_B, Charon_Token);
assign_obj(TOKEN_C, Charon_Token);
assign_obj(TOKEN_D, Charon_Token);
}


