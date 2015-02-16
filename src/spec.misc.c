 /* ************************************************************************
  *  file: spec_procs.c , Special module.                   Part of DIKUMUD *
  *  Usage: Procedures handling special procedures for object/room/mobile   *
  *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
  *    									    *
  *  Modified by Ranger May-96 Removing soom mob assignments and commenting *
  ************************************************************************* */

/*
$Author: ronin $
$Date: 2004/02/05 16:11:03 $
$Header: /home/ronin/cvs/ronin/spec.misc.c,v 2.0.0.1 2004/02/05 16:11:03 ronin Exp $
$Id: spec.misc.c,v 2.0.0.1 2004/02/05 16:11:03 ronin Exp $
$Name:  $
$Log: spec.misc.c,v $
Revision 2.0.0.1  2004/02/05 16:11:03  ronin
Reinitialization of cvs archives

May 12 -05 Removed wear spec from 29441

Revision 1.7  2003/11/08 ronin
Addition of 2003 chaos prize specs.

Revision 1.6  2002/11/15 23:33:45  ronin
Making sure that gun spec works, re-revision.

Revision 1.5  2002/11/05 23:33:45  ronin
Addition of chaos prize specs.

Revision 1.4  2002/10/22 23:20:24  ronin
Addition of chaos_gun_spec for Chaos '02.

Revision 1.3  2002/04/18 04:15:44  ronin
Removal of mob spec assigns for Loftwick mobs.

Revision 1.2  2002/03/31 07:42:15  ronin
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
 #include "mob.spells.h"
 /*   external vars  */

 extern CHAR *character_list;
 extern struct descriptor_data *descriptor_list;
 extern struct time_info_data time_info;
 extern struct spell_info_type spell_info[MAX_SPL_LIST];
 extern int CHAOSMODE;

struct char_data
*get_random_v( struct char_data *ch ) {
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


/* Chaos 2003 Prize */
int chaos2003_obj(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *holder;
  char buf[MIL];
  OBJ *object;
  int numa=0, numb=0, numc=0, numd=0, nume=0;

  if(!ch) return FALSE;
  if(!obj->carried_by) return FALSE;
  if(ch != obj->carried_by) return FALSE;

  holder=obj->carried_by;

  if(holder && ch==holder && cmd==CMD_WEAR && holder==obj->carried_by) {
    if(!(strcmp(arg, " all"))) {
      send_to_char("You must wear your Chaos object first.\n\r",holder);
      return TRUE;
    }
    one_argument(arg,buf);
    string_to_lower(buf);
    object=get_obj_in_list_vis(holder,buf,holder->carrying);
    if(!object) return FALSE;
    if(IS_NPC(holder)) return FALSE;

    if(object==obj) {

      switch(V_OBJ(object)) {
        case 29438:
        case 29439:
        case 29440:
          numa=704; /* Stanach */
          numb=1994; /* Ribald */
          numc=5505; /* Lews */
          numd=1232; /* Saemon */
          nume=2137; /* Kahlan */
          if((holder->ver3.id == numa) || (holder->ver3.id == numb) || (holder->ver3.id == numc) || (holder->ver3.id == numd) || (holder->ver3.id == nume)) {
            return FALSE;
          } else {
            if(V_OBJ(object)==29438)
              send_to_char("The Folkloric Kilt of the McRibald Clan tells you 'You cannot wear me.'\n\r",holder);
            if(V_OBJ(object)==29439)
              send_to_char("The last dying breath of Sorcerer Tepir tells you 'You cannot wear me.'\n\r",holder);
            if(V_OBJ(object)==29440)
              send_to_char("The last dying breath of Sorcerer Zazu tells you 'You cannot wear me.'\n\r",holder);
            return TRUE;
          }
          break;

        case 29442:
          numa=1;
          numb=2;
          numc=1;
          numd=2;
          nume=1;
          if((holder->ver3.id == numa) || (holder->ver3.id == numb) || (holder->ver3.id == numc) || (holder->ver3.id == numd) || (holder->ver3.id == nume)) {
            return FALSE;
          } else {
            send_to_char("The Achromatic Gem tells you 'You cannot wear me.'\n\r",holder);
            return TRUE;
          }
          break;

        default:
          break;
      }
    }
    return FALSE;
  }
  return FALSE;
}


int chaos2003_cloak (OBJ *shadow, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict, *holder;
  char buf[MIL];
  OBJ *object;
  int numa=0, numb=0, numc=0, numd=0, nume=0;

  if(ch && shadow->carried_by) {
    if(!(holder=shadow->carried_by)) return FALSE;

    if(holder && ch==holder && cmd==CMD_WEAR && holder==shadow->carried_by) {
      if(!(strcmp(arg, " all"))) {
        send_to_char("You must wear your Chaos object first.\n\r",holder);
        return TRUE;
      }
      one_argument(arg,buf);
      string_to_lower(buf);
      object=get_obj_in_list_vis(holder,buf,holder->carrying);
      if(!object) return FALSE;
      if(IS_NPC(holder)) return FALSE;

      if(object==shadow) {
        numa=3451; /* Tepir */
        numb=3452; /* Magma */
        numc=3453; /* Virte */
        numd=0; /*  */
        nume=0; /*  */
        if((holder->ver3.id == numa) || (holder->ver3.id == numb) || (holder->ver3.id == numc) || (holder->ver3.id == numd) || (holder->ver3.id == nume)) {
          return FALSE;
        } else {
          if(V_OBJ(object)==29444)
            send_to_char("The Shadow of the Harvest Moon tells you 'You cannot wear me.'\n\r",holder);
          return TRUE;
        }
      }
      return FALSE;
    }
    return FALSE;
  }

  if(!(owner=shadow->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n appears dim beneath a crimson veil of darkness!",FALSE,owner,shadow,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_MOBACT && IS_NIGHT && chance(6)) {
    if(chance(10)) {
      if(count_mortals_room(owner,TRUE) < 2) return FALSE;
      vict=get_random_v(owner);
      if(!vict) return FALSE;
      if(IS_AFFECTED(vict, AFF_IMINV)) return FALSE;
      act("A tendril of your Shadow reaches out and taints $N with your Evil.",FALSE,owner,shadow,vict,TO_CHAR);
      act("A tendril of $n's Shadow reaches out and taints $N with $n's Evil.",FALSE,owner,shadow,vict,TO_NOTVICT);
      act("A tendril of the $p has tainted you with $n's Evil.",FALSE,owner,shadow,vict,TO_VICT);
      cast_imp_invisibility(GET_LEVEL(owner), owner, "", SPELL_TYPE_SPELL, vict, 0);
      return FALSE;
    }
    return FALSE;
  }
  return FALSE;
}


/* Zazu's 2003 cape - from winged serpent tail in spec.hell.c */
int chaos2003_Cape (OBJ *cape, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict;
  struct descriptor_data *d;

  if(cape->equipped_by) {
    if(!(owner=cape->equipped_by)) return FALSE;

    d=owner->desc;
    if(!d) return FALSE;
    if(d->connected!=CON_PLYNG) return FALSE; /* If char died */

    if(cmd==MSG_MOBACT && chance(1)) {
      if(chance(20)) return FALSE;
      switch(number(1,2)) {
        case 1:
          if(count_mortals_room(owner,TRUE) < 2) break;
          vict=get_random_v(owner);
          if(!vict) break;
          act("Huge clouds of smoke drift into your lungs from $n's cape...\nYou feel groovy.",FALSE,owner,cape,vict,TO_VICT);
          act("Huge clouds of smoke blast explosively from $n's cape toward $N... groovy.",FALSE,owner,cape,vict,TO_NOTVICT);
          act("Your cape spews clouds of mellowing herbal smoke into $N's face.\nPuff, puff, give.",FALSE,owner,cape,vict,TO_CHAR);
          break;

        case 2:
          act("A wispy cloud of smoke billows from your cape, which you inhale satisfyingly.",FALSE,owner,cape,0,TO_CHAR);
          act("A wispy cloud of smoke billows from $n's cape, which $e inhales deeply.",FALSE,owner,cape,0,TO_ROOM);
          break;

        default:
          break;
      }
      return FALSE;
    }
    return FALSE;
  }

  return FALSE;
}


/* Ellisande Chaos 2002 Prize */
int chaos_spiked_gauntlets(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  struct char_data *victim;
  char name[256];

  if(!ch) return FALSE;
  if(ch != obj->equipped_by) return FALSE;
  if(IS_NPC(ch)) return FALSE;

  if(ch==obj->equipped_by && cmd==CMD_PUMMEL && chance(15)) {
    if(strcmp(GET_NAME(ch),"Soot") &&
       strcmp(GET_NAME(ch),"Shadowkiller") &&
       strcmp(GET_NAME(ch),"Aemon") &&
       GET_LEVEL(ch)<LEVEL_IMM) return FALSE;

    one_argument(arg, name);

    if(!(victim = get_char_room_vis(ch, name))) {
      if(ch->specials.fighting) {
        victim = ch->specials.fighting;
      } else {
        send_to_char("Pummel who?\n\r", ch);
        return TRUE;
      }
    }

    if(victim == ch) {
      send_to_char("Aren't we funny today...\n\r", ch);
      return TRUE;
    }

    if(!ch->equipment[WIELD]) {
      send_to_char("You need to wield a weapon for it to succeed.\n\r",ch);
      return TRUE;
    }

    if((IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_PUMMEL)) ||
      (IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch,victim,BT_INVUL))) {
      act("You pummel $N, but your pummel has no affect!", FALSE, ch, 0, victim, TO_CHAR);
      act("$N pummels you, but it is ineffective!", FALSE, victim, 0, ch, TO_CHAR);
      act("$n pummels $N, but the pummel has no affect!", FALSE, ch, 0, victim, TO_NOTVICT);
      damage(ch, victim, 0, SKILL_PUMMEL,DAM_NO_BLOCK);
    } else {
      act("Your armored fist slams into $N with savage force!", FALSE, ch, 0, victim, TO_CHAR);
      act("$n's armored fist slams into you with savage force!", FALSE, ch, 0, victim, TO_VICT);
      act("$n's armored fist slams into $N with savage force!", FALSE, ch, 0, victim, TO_NOTVICT);
      damage(ch, victim, 20, SKILL_PUMMEL,DAM_NO_BLOCK);
      GET_POS(victim) = POSITION_MORTALLYW;
      WAIT_STATE(victim, PULSE_VIOLENCE*2);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return TRUE;
  }
  return FALSE;
}


/* Elric Chaos 2002 Prize */
int chaos_stormbringer(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *owner;
  struct char_data *victim;
  int dam = 1, xp = 1, mana = 1, move = 1, min = 1, max = 1;

  if(!(owner = obj->equipped_by)) return FALSE;
  if(IS_NPC(owner)) return FALSE;
  if(!owner->specials.fighting) return FALSE;

  if(owner==obj->equipped_by && cmd==MSG_MOBACT && chance(8)) { /* mankey */
    if(strcmp(GET_NAME(owner),"Niobe") && GET_LEVEL(owner)<LEVEL_IMM) return FALSE;
    victim = owner->specials.fighting;
    if(!victim) return FALSE;
    if(IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, SAFE)&& (!CHAOSMODE)) return FALSE;
    if(IS_NPC(victim) && IS_SET(victim->specials.immune,IMMUNE_DRAIN)) {
      damage(owner, victim, 0, SPELL_ENERGY_DRAIN,DAM_MAGICAL);
      return FALSE;
    }
    if(GET_LEVEL(victim) < 5) {
      act("$N screams in agony as $p tears out a piece of their soul!",FALSE,owner,obj,victim,TO_CHAR);
      act("You scream in agony as $p tears out a piece of your soul!",FALSE,owner,obj,victim,TO_VICT);
      act("$N screams in agony as $p tears out a piece of their soul!",FALSE,owner,obj,victim,TO_NOTVICT);
      damage(owner, victim, 200, TYPE_UNDEFINED,DAM_MAGICAL);
      return FALSE;
    }

    min = GET_LEVEL(owner)-10;
    max = GET_LEVEL(owner)+10;
    xp = (number(min,max))*500;
    gain_exp(victim, -xp);
    gain_exp(owner, xp/4);
    dam = number(1,GET_LEVEL(owner));

    mana = number(min,max);
    move = number(min,max);
    GET_MANA(victim) = MAX(GET_MANA(victim)-mana,0);
    GET_MOVE(victim) = MAX(GET_MOVE(victim)-move,0);

    act("$N screams in agony as $p tears out a piece of their soul!",FALSE,owner,obj,victim,TO_CHAR);    
    act("You scream in agony as $p tears out a piece of your soul!",FALSE,owner,obj,victim,TO_VICT);    
    act("$N screams in agony as $p tears out a piece of their soul!",FALSE,owner,obj,victim,TO_NOTVICT);
    damage(owner, victim, dam, TYPE_UNDEFINED,DAM_NO_BLOCK);

    mana = mana/3;
    move = move/3;
    dam = dam/2;

    GET_MOVE(owner) = MIN(GET_MOVE(owner)+move,GET_MAX_MOVE(owner));
    GET_MANA(owner) = MIN(GET_MANA(owner)+mana,GET_MAX_MANA(owner));
    GET_HIT(owner) = MIN(GET_HIT(owner)+dam,GET_MAX_HIT(owner));

    return FALSE;
  }
  return FALSE;
}


/* Liner - Chaos '02 */
int chaos_gun_spec(OBJ *gun, CHAR *ch, int cmd, char *argument) {
  struct char_data *tmp_char;
  struct obj_data *tmp_object;
  int percent, bits, num;
  char buf[MIL];
  CHAR *owner, *holder;

  if(IS_NPC(ch)) return FALSE;

  if(gun->carried_by) 
    owner=gun->carried_by;
  if(gun->equipped_by)
    holder=gun->equipped_by;

#ifdef CHAOS_2002

  if((owner && cmd==MSG_CORPSE) || (holder && cmd==MSG_CORPSE)) {
    if(ch==owner &&  owner==gun->carried_by) {
      send_to_char("`kYour gun evaporates as you die!`q\n\r",owner);
      obj_from_char(gun);
      extract_obj(gun);
    }
    if(ch==holder && holder==gun->equipped_by) {
      send_to_char("`kYour gun evaporates as you die!`q\n\r",holder);
      unequip_char(holder,HOLD);
      extract_obj(gun);
    }
    return FALSE;
  }
#endif /* CHAOS_2002 */

  if(!ch) return FALSE;

#ifdef CHAOS_2002

  if(ch==owner && owner==gun->carried_by && (cmd==CMD_DROP || cmd==CMD_DONATE)) {
    one_argument (argument, buf);
    if(!isname(buf,OBJ_NAME(gun))) return FALSE;
    if(!IS_MORTAL(owner)) return FALSE;
    send_to_char("`kYou cannot drop the gun!`q\n\r",owner);
    return TRUE;
  }

  if(ch==owner && owner==gun->carried_by && cmd==CMD_RELOAD) {
    if(!IS_SET(gun->obj_flags.extra_flags, ITEM_DARK)) {
      SET_BIT(gun->obj_flags.extra_flags, ITEM_DARK);
      return FALSE;
    }
    else if(IS_SET(gun->obj_flags.extra_flags, ITEM_DARK)) {
      send_to_char("`kYou cannot reload!`q\n\r",owner);
      return TRUE;
    }
  }
#endif /* CHAOS_2002 */

  if(ch==holder && holder==gun->equipped_by && cmd==CMD_SHOOT) {
    if(holder->equipment[HOLD] == 0) {
      send_to_char("You are not holding anything.\n\r", holder);
      return TRUE;
    }

    gun = holder->equipment[HOLD];

    if(V_OBJ(gun)==1217) {

      argument = one_argument(argument,buf);

      bits = generic_find(buf, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                            FIND_OBJ_EQUIP, holder, &tmp_char, &tmp_object);
      if(bits) {
        if(bits == FIND_CHAR_ROOM) {
          if(IS_NPC(tmp_char) && GET_LEVEL(tmp_char)>50) {
            send_to_char("The gun is too weak to kill this being.\n\r",holder);
            return TRUE;
          }
          if(holder==tmp_char) {
            send_to_char("Shoot yourself?  That wouldn't be prudent at this juncture.\n\r",holder);
            return TRUE;
          }

          percent = IS_IMPLEMENTOR(holder) ? 0 : number(1, 100);
          if(percent > 72) {
            act("$n's gun misfires !\n\r", TRUE, holder, gun, tmp_char, TO_ROOM);
            act("`iYour gun misfires !`q\n\r$N grins evilly.\n\r", FALSE, holder, gun, tmp_char, TO_CHAR);
            WAIT_STATE(holder,PULSE_VIOLENCE*2);
            return TRUE;
          }

          num = number(0,9);

          if (IS_IMPLEMENTOR(holder))
          {
            argument = one_argument(argument,buf);
            if(!strncasecmp(buf,"rape",4)) num = 7;
          }
                    
          switch(num) {
            case 9:
              act("Your gun fires a volley of toilet paper rolls at $N, beating the life out of $M!",FALSE,holder,0,tmp_char,TO_CHAR);
              act("\n$n's gun fires a volley of toilet paper rolls at you, each beating you senselessly into the ground!",FALSE,holder,0,tmp_char,TO_VICT);
              act("$n's gun fires a volley of toilet paper rolls at $N, beating the life out of $M!",FALSE,holder,0,tmp_char,TO_NOTVICT);
              damage(holder,tmp_char,27,SKILL_PUMMEL,DAM_NO_BLOCK);
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            case 8:
              act("You fire your gun at $N! A horrible monster sphincter appears and \nchokes the life out of $M.",FALSE,holder,0,tmp_char,TO_CHAR);
              act("\n$n fires $s gun at you! A horrible monster sphincter appears and \nchokes the life out of you.",FALSE,holder,0,tmp_char,TO_VICT);
              act("$n fires $s gun at $N! A horrible monster sphincter appears and \nchokes the life out of $M.",FALSE,holder,0,tmp_char,TO_NOTVICT);
              damage(holder,tmp_char,24,SKILL_PUMMEL,DAM_NO_BLOCK);  
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            case 7:
              act("You fire a seed at the ground before $N! \nAn evil zombie tree grows suddenly and extends a branch toward $M, \nbrutally raping $S life away.",FALSE,holder,0,tmp_char,TO_CHAR);
              act("\n$n fires a seed at the ground before you! \nAn evil zombie tree grows suddenly and extends a branch which \nbrutally rapes your life away.",FALSE,holder,0,tmp_char,TO_VICT);
              act("$n fires a seed at the ground before $N! \nAn evil zombie tree grows suddenly and extends a branch toward $M, \nbrutally raping $S life away.",FALSE,holder,0,tmp_char,TO_NOTVICT);
              damage(holder,tmp_char,21,SKILL_PUMMEL,DAM_NO_BLOCK);
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            case 6:
              act("You fire a giant decaying pumpkin at $N, displacing $S head with its orange bulk!",FALSE,holder,0,tmp_char,TO_CHAR);
              act("\n$n fires a giant decaying pumpkin at you, displacing your head with its orange bulk!",FALSE,holder,0,tmp_char,TO_VICT);
              act("$n fires a giant decaying pumpkin at $N, displacing $S head with its orange bulk!",FALSE,holder,0,tmp_char,TO_NOTVICT);
              damage(holder,tmp_char,18,SKILL_PUMMEL,DAM_NO_BLOCK);
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            case 5:
              act("You fire at $N!  \nA gigantic shark with a laserbeam attached to its head appears. \nThe shark fries $N to a crispy brown.",FALSE,holder,0,tmp_char,TO_CHAR);
              act("\n$n fires at you! \nA gigantic shark with a laserbeam attached to its head appears. \nThe shark fries you to a crispy brown.",FALSE,holder,0,tmp_char,TO_VICT);
              act("$n fires at $N!  \nA gigantic shark with a laserbeam attached to its head appears. \nThe shark fries $N to a crispy brown.",FALSE,holder,0,tmp_char,TO_NOTVICT);
              damage(holder,tmp_char,15,SKILL_PUMMEL,DAM_NO_BLOCK);
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            case 4:
              act("Your gun fires a barrage of rabid bats at $N!",FALSE,holder,0,tmp_char,TO_CHAR);
              act("\n$n's gun fires a barrage of rabid bats at you!",FALSE,holder,0,tmp_char,TO_VICT);
              act("$n's gun fires a barrage of rabid bats at $N!",FALSE,holder,0,tmp_char,TO_NOTVICT);
              damage(holder,tmp_char,12,SKILL_PUMMEL,DAM_NO_BLOCK);
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            case 3:
              act("Your shot shatters $N's spine!  As $E falls to the ground, \na goblin explodes from the gaping wound and gnaws \n$N's head off as well.",FALSE,holder,0,tmp_char,TO_CHAR);
              act("\n$n's shot shatters your spine!  As you fall to the ground, \na goblin explodes from your back and gnaws your head off.",FALSE,holder,0,tmp_char,TO_VICT);
              act("$n's shot shatters $N's spine!  As $E falls to the ground, \na goblin explodes from the gaping wound and gnaws \n$N's head off as well.",FALSE,holder,0,tmp_char,TO_NOTVICT);
              damage(holder,tmp_char,9,SKILL_PUMMEL,DAM_NO_BLOCK);
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            case 2:
              act("You fire your gun! The bullets metamorphose into a pack of werewolves \nwhich commence tearing $N to bleeding shreds of flesh.",FALSE,holder,0,tmp_char,TO_CHAR);
              act("$n fires $s gun! The bullets metamorphose into a pack of werewolves \nwhich commence tearing $N to bleeding shreds of flesh.",FALSE,holder,0,tmp_char,TO_NOTVICT);
              act("\n$n fires $s gun at you! The bullets metamorphose into a pack of werewolves \nwhich commence tearing you to bleeding shreds of flesh.",FALSE,holder,0,tmp_char,TO_VICT);
              damage(holder,tmp_char,6,SKILL_PUMMEL,DAM_NO_BLOCK); 
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            case 1:
              act("Your gun fires a barrage of rotting eggs at $N, almost killing $M instantly!",FALSE,holder,0,tmp_char,TO_CHAR);
              act("$n fires a barrage of rotting eggs at $N, almost killing $M instantly!",FALSE,holder,0,tmp_char,TO_NOTVICT);
              act("\n$n fires a barrage of rotting eggs at you! *SPLATTER* You're almost dead.",FALSE,holder,0,tmp_char,TO_VICT);
              damage(holder,tmp_char,3,SKILL_PUMMEL,DAM_NO_BLOCK);
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            case 0:
              act("Your gun fires! A flaming bag of poo strikes $N in the face!",FALSE,holder,0,tmp_char,TO_CHAR);
              act("\n$n's gun fires a flaming bag of poo directly into your face!",FALSE,holder,0,tmp_char,TO_VICT);
              act("$n's gun fires a flaming bag of poo directly into $N's face!",FALSE,holder,0,tmp_char,TO_NOTVICT);
              damage(holder,tmp_char,30,SKILL_PUMMEL,DAM_NO_BLOCK);
              if (!IS_IMPLEMENTOR(tmp_char)) {
                WAIT_STATE(tmp_char,PULSE_VIOLENCE*2);
                GET_POS(tmp_char)=POSITION_STUNNED;
              }
              break;
            default:
              break;            
          }
          if (!IS_IMPLEMENTOR(holder)) WAIT_STATE(holder, PULSE_VIOLENCE*2);
          return TRUE;
        }
        else {
          act("$n fires a shot into the sky.\n\r", TRUE, holder, gun, 0, TO_ROOM);
          act("`kYour gun fires into thin air `q!\n\r", FALSE, holder, gun, 0, TO_CHAR);
          if (!IS_IMPLEMENTOR(holder)) WAIT_STATE(holder,PULSE_VIOLENCE);
          return TRUE;
        }
      }
      else {
        act("$n fires a shot into the sky.\n\r", TRUE, holder, gun, 0, TO_ROOM);
        act("`kYour gun fires into thin air `q!\n\r", FALSE, holder, gun, 0, TO_CHAR);
        if (!IS_IMPLEMENTOR(holder)) WAIT_STATE(holder,PULSE_VIOLENCE);
        return TRUE;
      }
    }
    else {
      send_to_char("You can't shoot that.\n\r", ch);
      return TRUE;
    }
  }
  return FALSE;
}


 int death_spray(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {

   if(cmd) return FALSE;

   if(GET_POS(mob)!=POSITION_FIGHTING) return FALSE;

   if(mob->specials.fighting &&
      (CHAR_REAL_ROOM(mob->specials.fighting) == CHAR_REAL_ROOM(mob)) &&
      (number(0, 5) == 0) && !IS_NPC(mob->specials.fighting))
     {
       cast_death_spray( GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL,
			mob->specials.fighting, 0);
       return FALSE;
     }
   return FALSE;
 }


 int backstab_thief(CHAR *mob,CHAR *ch,int cmd,char *arg)
 {
   CHAR *victim;

   if (!mob->equipment[WIELD])
     return FALSE;

   for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = victim->next_in_room )
     { if (number(0,1) == 0)
	 break;
     }
   if (!victim)
     return FALSE;

   if (cmd) return FALSE;

   if ((victim == mob) || (GET_POS(mob) == POSITION_FIGHTING) ||
       (GET_POS(victim) == 0) || (GET_POS(victim) > 6))
     return FALSE;


   hit(mob, victim, SKILL_BACKSTAB);

   return FALSE;
 }


 int red_dragon(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict;

   if (cmd) return FALSE;

   if (GET_POS(mob) != POSITION_FIGHTING) return FALSE;


   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
     if (vict->specials.fighting==mob && number(0,5)==0)
       break;

   if (!vict)
     return FALSE;


   act("$n utters the words 'dskahjshbv'.", 1, ch, 0, 0, TO_ROOM);
   spell_fire_breath(GET_LEVEL(mob), mob, vict, 0);
   return FALSE;

 }


 int fighter(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict;
   int num;
   OBJ *wield = 0;

   if (cmd) return FALSE;

   if (GET_POS(mob) != POSITION_FIGHTING) return FALSE;


   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
     if (vict->specials.fighting==mob && number(0,5)==0)
       break;

   if (!vict)
     return FALSE;

   num = number(0,3);

   if (num == 0)
     {	damage(mob, vict, 0, SKILL_KICK,DAM_NO_BLOCK);
	 WAIT_STATE(vict, PULSE_VIOLENCE*2);
	 WAIT_STATE(mob, PULSE_VIOLENCE*2);
	 return TRUE;
       }
   else if (num == 1)
     {
       if (vict->equipment[WIELD])
	 wield = vict->equipment[WIELD];
       else { return FALSE; }

       act("$N kicked off your weapon!", FALSE, vict, 0, mob, TO_CHAR);
       act("$n kicked off $N's weapon!", FALSE, mob, 0, vict, TO_ROOM);
       unequip_char(vict, WIELD);
       obj_to_room(wield, CHAR_REAL_ROOM(vict));
       WAIT_STATE(mob, PULSE_VIOLENCE*2);
       return FALSE;
     }
   else
     {	damage(mob, vict, 0, SKILL_BASH,DAM_NO_BLOCK);
	 GET_POS(vict) = POSITION_SITTING;
	 WAIT_STATE(vict, PULSE_VIOLENCE*2);
	 WAIT_STATE(mob, PULSE_VIOLENCE*2);
	 return FALSE;
       }

 }

 int bash_kick(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict;
   int num;

   if (cmd) return FALSE;

   if (GET_POS(mob) != POSITION_FIGHTING) return FALSE;


   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
     if (vict->specials.fighting==mob && number(0,5)==0)
       break;

   if (!vict)
     return FALSE;

   num = number(0,1);

   if (num == 0)
     {	damage(mob, vict, 0, SKILL_KICK,DAM_NO_BLOCK);
	 WAIT_STATE(vict, PULSE_VIOLENCE*2);
	 WAIT_STATE(mob, PULSE_VIOLENCE*2);
	 return FALSE;
       }
   else
     {	damage(mob, vict, 0, SKILL_BASH,DAM_NO_BLOCK);
	 GET_POS(vict) = POSITION_RESTING;
	 WAIT_STATE(vict, PULSE_VIOLENCE*2);
	 WAIT_STATE(mob, PULSE_VIOLENCE*2);
	 return FALSE;
       }

 }

 int drow_magic(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict;
   int i;
   if(!mob)
    return FALSE;
   if(cmd) return FALSE;

   if(GET_POS(mob)!=POSITION_FIGHTING) return FALSE;

   if(!mob->specials.fighting) return FALSE;


   /* Find a dude to to evil things upon ! */

   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
     if (vict->specials.fighting==mob && number(0,5)==0)
       break;

   if (!vict)
     return FALSE;


   i = number(1, 4);

   if( (GET_LEVEL(mob)>12) && (i==1))
     {
       act("$n utters the words 'ufo ufo ufo'.", 1, mob, 0, 0, TO_ROOM);
       cast_curse(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
       return FALSE;
     }

   if( (GET_LEVEL(mob)>7) && (i == 2) )
     {
       act("$n utters the words 'koholian dia'.", 1,mob, 0, 0, TO_ROOM);
       cast_blindness(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
       return FALSE;
     }

   if( (GET_LEVEL(mob)>12) && (i == 3) && IS_EVIL(mob))
     {
       act("$n utters the words 'ib er dranker'.", 1, mob, 0, 0, TO_ROOM);
       cast_energy_drain(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
       return FALSE;
     }

   if( (GET_LEVEL(mob)>15) && (i == 4))
     {
       act("$n utters the words 'llaberif'.", 1, mob, 0, 0, TO_ROOM);
       cast_fireball(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
       return FALSE;
     }

   return FALSE;

 }

 int magic_user(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict;

   if(cmd) return FALSE;
   if(!mob)
     return FALSE;


   if(GET_POS(mob)!=POSITION_FIGHTING) return FALSE;

   if(!mob->specials.fighting) return FALSE;


   /* Find a dude to to evil things upon ! */

   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
     if (vict->specials.fighting==mob && number(0,5)==0)
       break;

   if (!vict)
     return FALSE;


   if( (vict!=mob->specials.fighting) && (GET_LEVEL(mob)>13) && (number(0,7)==0) )
     {
       act("$n utters the words 'ZZzzzzzzzz'.", 1, mob, 0, 0, TO_ROOM);
       cast_sleep(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
       return FALSE;
     }

   if( (GET_LEVEL(mob)>12) && (number(0,6)==0) )
     {
       act("$n utters the words '#$$^%%*^^&*^&*'.", 1, mob, 0, 0, TO_ROOM);
       cast_curse(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
       return FALSE;
     }

   if( (GET_LEVEL(mob)>7) && (number(0,5)==0) )
     {
       act("$n utters the words 'koholian dia'.", 1, mob, 0, 0, TO_ROOM);
       cast_blindness(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
       return FALSE;
     }

   if( (GET_LEVEL(mob)>12) && (number(0,8)==0) && IS_EVIL(mob))
     {
       act("$n utters the words 'ib er dranker'.", 1, mob, 0, 0, TO_ROOM);
       cast_energy_drain(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
       return FALSE;
     }

   switch (GET_LEVEL(mob)) {
   case 1:
   case 2:
   case 3:
   case 4:
     act("$n utters the words 'hahili duvini'.", 1, mob, 0, 0, TO_ROOM);
     cast_magic_missile(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
     break;
   case 5:
   case 6:
   case 7:
   case 8:
     act("$n utters the words 'grynt oef'.", 1, mob, 0, 0, TO_ROOM);
     cast_burning_hands(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
     break;
   case 9:
   case 10:
     act("$n utters the words 'sjulk divi'.", 1, mob, 0, 0, TO_ROOM);
     cast_lightning_bolt(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
     break;
   case 11:
   case 12:
   case 13:
   case 14:
   case 15:
   case 16:
     act("$n utters the words 'nasson hof'.", 1, mob, 0, 0, TO_ROOM);
     cast_colour_spray(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
     break;
   case 17:
   case 18:
     act("$n utters the words 'hot hot'.", 1, mob, 0, 0, TO_ROOM);
     cast_fireball(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
     break;
   case 19:
   case 20:
     act("$n utters the words 'die die'.", 1, mob, 0, 0, TO_ROOM);
     cast_iceball(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
     break;
   case 21:
   case 22:
   case 23:
   case 24:
   case 25:
     act("$n utters the words 'milky cow'.", 1, mob, 0, 0, TO_ROOM);
     cast_lethal_fire(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
     break;
   default:
     act("$n utters the words 'mummy duck'.", 1, mob, 0, 0, TO_ROOM);
     cast_colour_spray(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, 0);
     break;


   }
   return FALSE;

 }

 int enfan(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict;

   if(cmd) return FALSE;

   if (GET_POS(mob) != POSITION_FIGHTING) return FALSE;

   if (!mob->specials.fighting) return FALSE;

   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
     if (vict->specials.fighting==mob && number(0,5)==0)
       break;

   if (!vict) return FALSE;

   /*	if (number(0,1)==1) */
   act("$n spits some acid on $N!", 1, mob, 0, vict,TO_NOTVICT);
   act("$n spits some acid on you!", 1, mob, 0, vict ,TO_VICT);
   spell_acid_breath(GET_LEVEL(mob), mob, vict, 0);
   return FALSE;
 }


 int tower_guard(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   char buf[256], buf2[256];

   if (cmd>6 || cmd<1)
     return FALSE;

   strcpy(buf,  "The guard blocks your way, and says '*&^%&%^%*'.\n\r");
   strcpy(buf2, "The guard pushes $n, and says '*&^%&%^%*'.");

   if ((CHAR_REAL_ROOM(mob) == real_room(1801)) && (cmd == CMD_SOUTH)) {
     act(buf2, FALSE, ch, 0, 0, TO_ROOM);
     send_to_char(buf, ch);
     return TRUE;
   } else if ((CHAR_REAL_ROOM(ch) == real_room(1805)) && (cmd == CMD_UP)) {
     act(buf2, FALSE, ch, 0, 0, TO_ROOM);
     send_to_char(buf, ch);
     return TRUE;
   } else if ((CHAR_REAL_ROOM(ch) == real_room(1806)) && (cmd == CMD_UP)) {
     act(buf2, FALSE, ch, 0, 0, TO_ROOM);
     send_to_char(buf, ch);
     return TRUE;
   } else if ((CHAR_REAL_ROOM(ch) == real_room(1807)) && (cmd == CMD_UP)) {
     act(buf2, FALSE, ch, 0, 0, TO_ROOM);
     send_to_char(buf, ch);
     return TRUE;
   } else if ((CHAR_REAL_ROOM(ch) == real_room(1808)) && (cmd == CMD_UP)) {
     act(buf2, FALSE, ch, 0, 0, TO_ROOM);
     send_to_char(buf, ch);
     return TRUE;
   } else if ((CHAR_REAL_ROOM(ch) == real_room(1809)) && (cmd == CMD_UP)) {
     act(buf2, FALSE, ch, 0, 0, TO_ROOM);
     send_to_char(buf, ch);
     return TRUE;
   } else if ((CHAR_REAL_ROOM(ch) == real_room(1810)) && (cmd == CMD_UP)) {
     act(buf2, FALSE, ch, 0, 0, TO_ROOM);
     send_to_char(buf, ch);
     return TRUE;

   }

   return FALSE;

 }





 int puff(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {

   if (cmd)
     return(0);

   switch (number(0, 60))
     {
     case 0:
       do_say(mob, "My god! It's full of stars!", 0);
       return(FALSE);
     case 1:
       do_say(mob, "How'd all those fish get up here?", 0);
       return(FALSE);
     case 2:
       do_say(mob, "I'm a very female dragon.", 0);
       return(FALSE);
     case 3:
       do_say(mob, "I've got a peaceful, easy feeling.", 0);
       return(FALSE);
     default:
       return(0);
     }
 }

 /* Idea of the LockSmith is functionally similar to the Pet Shop */
 /* The problem here is that each key must somehow be associated  */
 /* with a certain player. My idea is that the players name will  */
 /* appear as the another Extra description keyword, prefixed     */
 /* by the words 'item_for_' and followed by the player name.     */
 /* The (keys) must all be stored in a room which is (virtually)  */
 /* adjacent to the room of the lock smith.                       */

 int pray_for_items(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   char buf[256];
   int key_room, gold;
   bool found;
   OBJ *tmp_obj, *obj;
	 struct extra_descr_data *ext;

	 if (cmd != MSG_DIE) /* You must pray to get the stuff */
		 return FALSE;

	 key_room = 1+CHAR_REAL_ROOM(ch);

   strcpy(buf, "item_for_");
   strcat(buf, GET_NAME(ch));

   gold = 0;
   found = FALSE;

   for (tmp_obj = world[key_room].contents; tmp_obj; tmp_obj = tmp_obj->next_content)
     for(ext = tmp_obj->ex_description; ext; ext = ext->next)
       if (str_cmp(buf, ext->keyword) == 0) {
		   if (gold == 0) {
		      gold = 1;
			    act("$n kneels and at the altar and chants a prayer to Odin.",
					  FALSE, ch, 0, 0, TO_ROOM);
				 act("You notice a faint light in Odin's eye.",
					  FALSE, ch, 0, 0, TO_CHAR);
		   }
	 obj = read_object(tmp_obj->item_number, REAL);
	 obj_to_room(obj, CHAR_REAL_ROOM(ch));
		   act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_ROOM);
		   act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_CHAR);
	 gold += obj->obj_flags.cost;
	 found = TRUE;
       }


   if (found) {
     GET_GOLD(ch) -= gold;
     GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
     return TRUE;
	 }

   return FALSE;
 }


 /* ********************************************************************
 *  Special procedures for objects                                     *
 ******************************************************************** */



 #define CHAL_ACT \
 "You are torn out of reality!\n\r\
You roll and tumble through endless voids for what seems like eternity...\n\r\
\n\r\
After a time, a new reality comes into focus... you are elsewhere.\n\r"


 int chalice(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   /* 222 is the normal chalice, 223 is chalice-on-altar */

   OBJ *chalice;
   char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
   static int chl = -1, achl = -1;

   if (chl < 1)
     {
       chl = real_object(222);
       achl = real_object(223);
     }

   switch(cmd)
     {
     case CMD_GET:    /* get */
       if (!(chalice = get_obj_in_list_num(chl,
					   world[CHAR_REAL_ROOM(ch)].contents))
	   && CAN_SEE_OBJ(ch, chalice))
	 if (!(chalice = get_obj_in_list_num(achl,
					     world[CHAR_REAL_ROOM(ch)].contents)) && CAN_SEE_OBJ(ch, chalice))
	   return(0);

       /* we found a chalice.. now try to get us */
       do_get(ch, arg, cmd);
       /* if got the altar one, switch her */
       if (chalice == get_obj_in_list_num(achl, ch->carrying))
	 {
				 extract_obj(chalice);
				 chalice = read_object(chl, VIRTUAL);
				 obj_to_char(chalice, ch);
			 }
			 return(1);
		 break;
		 case CMD_PUT: /* put */
			 if (!(chalice = get_obj_in_list_num(chl, ch->carrying)))
				 return(0);

			 argument_interpreter(arg, buf1, buf2);
			 if (!str_cmp(buf1, "chalice") && !str_cmp(buf2, "altar"))
			 {
				 extract_obj(chalice);
				 chalice = read_object(achl, VIRTUAL);
				 obj_to_room(chalice, CHAR_REAL_ROOM(ch));
				 send_to_char("Ok.\n\r", ch);
			 }
			 return(1);
		 break;
		 case 192: /* pray */
			 if (!(chalice = get_obj_in_list_num(achl,
				 world[CHAR_REAL_ROOM(ch)].contents)))
				 return(0);

			 /*do_action(ch, arg, cmd);*/  /* pray */
			 send_to_char(CHAL_ACT, ch);
			 extract_obj(chalice);
			 act("$n is torn out of existence!", TRUE, ch, 0, 0, TO_ROOM);
			 char_from_room(ch);
			 char_to_room(ch, real_room(2500));   /* before the fiery gates */
			 do_look(ch, "", 15);
			 return(1);
		 break;
		 default:
			 return(0);
		 break;
	 }
 }

 int kings_hall(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   /*	if (cmd != 176)*/
   return(0);

   /*	do_action(ch, arg, 176); */

   send_to_char("You feel as if some mighty force has been offended.\n\r", ch);
   send_to_char(CHAL_ACT, ch);
   act("$n is struck by an intense beam of light and vanishes.",
       TRUE, ch, 0, 0, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, real_room(1420));  /* behind the altar */
   do_look(ch, "", 15);
   return(1);
 }

 int mobrescue(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *tch;

   if (cmd || !AWAKE(mob) || (GET_POS(mob) == POSITION_FIGHTING))
     return (FALSE);


   for (tch=world[CHAR_REAL_ROOM(mob)].people; tch; tch = tch->next_in_room)
     {
       if (tch->specials.fighting && !IS_NPC(tch))
	 {

	   act("$n screams 'I'll save you!'", FALSE, mob, 0, 0, TO_ROOM);
	   hit(mob, tch, TYPE_UNDEFINED);
	   return(FALSE);
	 }
     }

   return(FALSE);
 }

int death(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
  if (cmd)
    return(0);
  switch (number(0,40))
    {
    case 1:
      act ("$n throws back his head and cackles with insane glee!", FALSE, mob, 0, 0, TO_ROOM);
      return(FALSE);
    case 2:
      act ("$n looks at you.", FALSE, mob, 0, 0, TO_ROOM);
      act ("$n falls down laughing.", FALSE, mob, 0, 0, TO_ROOM);
      return(FALSE);
    case 3:
      do_say(mob, "Welcome to hell asshole. Don't try to quit, because that won't work.", 0);
      return(FALSE);
    case 4:
      do_say(mob, "The gods are not pleased.", 0);
      return(FALSE);
    case 5:
      do_say(mob, "You know, this is Xilus' hang out. Maybe he'll help torment you.", 0);
      return(FALSE);
    case 6:
      act ("$n growls at you.", FALSE, mob, 0, 0, TO_ROOM);
      return(FALSE);
    default:
      return(0);
    }
}

int hospital(int room,CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];

  if (cmd==CMD_LIST) { /* List */
    send_to_char("Available cures are:\n\r", ch);
    sprintf(buf,"Heal   50000\n\rSerious   5000\n\rPoison   4000\n\rArmor   1000\n\r");
    send_to_char(buf, ch);
    return(TRUE);
  }

  else if (cmd==CMD_BUY) { /* Buy */

    arg = one_argument(arg, buf);

    if (!*buf)
      {
	send_to_char("Buy what?\n\r",ch);
	return(TRUE);
      }
    if (!strcmp(buf,"heal"))
      {
	if (GET_GOLD(ch)<50000)
	  {
	    send_to_char("You dont have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    cast_heal(35,ch,"",SPELL_TYPE_SPELL,ch,0);
	    GET_GOLD(ch)-=50000;
	    send_to_char("That will cost you 50000.\n\r",ch);
	    return(TRUE);
	  }
      }
    if (!strcmp(buf,"poison"))
      {
	if (GET_GOLD(ch)<4000)
	  {
	    send_to_char("You dont have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    cast_remove_poison(35,ch,"",SPELL_TYPE_SPELL,ch,0);
	    GET_GOLD(ch)-=4000;
	    send_to_char("Thats 4000 coins.  Thanks for your support.\n\r",ch);
	    return(TRUE);
	  }
      }
    if (!strcmp(buf,"serious"))
      {
	if (GET_GOLD(ch)<5000)
	  {
	    send_to_char("You dont have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    cast_cure_serious(35,ch,"",SPELL_TYPE_SPELL,ch,0);
	    GET_GOLD(ch)-=5000;
	    send_to_char("You spend 5000 for a cure serious.\n\r",ch);
	    return(TRUE);
	  }
      }
    if (!strcmp(buf,"armor"))
      {
	if (GET_GOLD(ch)<1000)
	  {
	    send_to_char("You dont have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    cast_armor(35,ch,"",SPELL_TYPE_SPELL,ch,0);
	    GET_GOLD(ch)-=1000;
	    send_to_char("Thats 1000 coins.\n\r",ch);
	    return(TRUE);
	  }
      }
    else
      {
	send_to_char("You cant buy that.\n\r",ch);
	return(TRUE);
      }
  }
  /* All commands except list and buy */
  return(FALSE);
}

int tiamat(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;

  if(cmd) return FALSE;

  if(GET_POS(mob)!=POSITION_FIGHTING) return FALSE;

  if(!mob->specials.fighting) return FALSE;

  /* Find a dude to to evil things upon ! */

  for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
    if (vict->specials.fighting==mob && number(0,2)==0)
      break;

  if (!vict)
    return FALSE;

  switch (number(0,4))
    {
    case 0:
      spell_fire_breath(GET_LEVEL(mob), mob, vict, 0);
      break;
    case 1:
      spell_gas_breath(GET_LEVEL(mob), mob, vict, 0);
      break;
    case 2:
      spell_frost_breath(GET_LEVEL(mob), mob,vict, 0);
      break;
    case 3:
      spell_acid_breath(GET_LEVEL(mob), mob, vict, 0);
      break;
    case 4:
      spell_lightning_breath(GET_LEVEL(mob), mob, vict, 0);
      break;
    }
  return FALSE;

}

int onering_quest(OBJ *sh, CHAR *ch, int cmd, char *arg) {
  CHAR *tmp, *tch;
  char buf[1000];
  int maxhit=500;
  int maxmana=500;

  if (ch && cmd == MSG_DIE) {

      if (sh->equipped_by) {
        tch = sh->equipped_by;
        if(IS_MOB(tch)) return FALSE;
      }
      else
        return FALSE;

  if (IS_SET(world[CHAR_REAL_ROOM(tch)].room_flags, CHAOTIC)) return FALSE;

      act ("$n almost dies, but $p glows brightly and envelops $m.\n\r", TRUE, tch, sh, 0, TO_ROOM);
      send_to_char ("You have been given another life!\n\r",tch);

      maxhit = number(1,GET_MAX_HIT (tch));
      maxmana = number(1,GET_MAX_MANA (tch));
      GET_HIT (tch) = maxhit;
      GET_MANA (tch) = maxmana;
      GET_MOVE (tch) = GET_MAX_MOVE (tch);

      for (tmp = world[CHAR_REAL_ROOM(tch)].people;tmp;tmp=tmp->next_in_room)
        if (GET_OPPONENT(tmp) == tch)
          stop_fighting (tmp);

      sprintf (buf, "%s rescued %s from %s [%d].", OBJ_SHORT(sh),
               GET_NAME(tch), world[CHAR_REAL_ROOM(tch)].name,
               CHAR_VIRTUAL_ROOM(tch));
      wizlog(buf, LEVEL_WIZ, 6);
      log_f(buf);

      stop_fighting (tch);

      spell_word_of_recall (GET_LEVEL(tch), tch, tch, 0);
      WAIT_STATE(tch, PULSE_VIOLENCE*5);
      return TRUE;
    }

  return FALSE;
}



 /**********************************************************************\
 |* End Of the Special procedures for misc mobs and such               *|
 \**********************************************************************/

void assign_misc (void) {

  assign_mob(2009,bash_kick); /* Galaxy */
  assign_mob(2029,bash_kick); /* Galaxy */
  assign_mob(2102,bash_kick); /* Brigand Camp, by Odie */
  assign_mob(2303,bash_kick); /* Chess Kingdom */
  assign_mob(2308,bash_kick); /* Chess Kingdom */
  assign_mob(2312,bash_kick); /* Chess Kingdom */
  assign_mob(2317,bash_kick); /* Chess Kingdom */
  assign_mob(2803,bash_kick); /* Ivory Tower */
  assign_mob(4505,bash_kick); /* Goblins */
  assign_mob(5105,bash_kick); /* Drow City */
  assign_mob(5106,bash_kick); /* Drow City */
  assign_mob(5208,bash_kick); /* City of Thalos */
  assign_mob(6201,bash_kick); /* Helventia Mountains by Lem */
  assign_mob(8005,bash_kick); /* Sonicity */

  assign_mob(5103,drow_magic); /* Drow City */
  assign_mob(5104,drow_magic); /* Drow City */
  assign_mob(5107,drow_magic); /* Drow City */
  assign_mob(5108,drow_magic); /* Drow City */
  assign_mob(5109,drow_magic); /* Drow City */

  assign_mob(6516,magic_user); /*mazekeeper*/  /* Dwarven Kingdom */
  assign_mob(4809,magic_user); /* Halfling Village */
  assign_mob(4810,magic_user); /* Halfling Village */
  assign_mob(4811,magic_user); /* Halfling Village */

  assign_mob(6910,magic_user); /* Qufael Custom Rooms */
  assign_mob(8401,magic_user); /* Castle Cantile by Ankh */

  assign_mob(2002,mobrescue); /* Galaxy */
  assign_mob(2105,mobrescue); /* Brigand Camp, by Odie */
  assign_mob(2106,mobrescue); /* Brigand Camp, by Odie */
  assign_mob(2107,mobrescue); /* Brigand Camp, by Odie */
  assign_mob(6202,mobrescue); /* Helventia Mountains by Lem */
  assign_mob(6501,mobrescue); /* Dwarven Kingdom */
  assign_mob(6506,mobrescue); /* Dwarven Kingdom */
  assign_mob(6507,mobrescue); /* Dwarven Kingdom */
  assign_mob(6510,mobrescue); /* Dwarven Kingdom */

  assign_obj(1217,chaos_gun_spec); /* immortal chaos gun */
  assign_obj(29409,chaos_spiked_gauntlets); /* sk chaos prize */
  assign_obj(29838,chaos_stormbringer); /* elric chaos prize */

  assign_obj(29438,chaos2003_obj); /* ribald first chaos prize */
  assign_obj(29439,chaos2003_obj); /* ribald nth chaos prize */
  assign_obj(29440,chaos2003_obj); /* ribald nth chaos prize */
  assign_obj(29441,chaos2003_Cape); /* zazu third chaos prize */
  assign_obj(29444,chaos2003_cloak); /* tepir second chaos prize */
  assign_obj(29497,chaos2003_Cape); /* hemp vanity */
  /* assign_obj(1105,onering_quest);  quest fun test item */
}

