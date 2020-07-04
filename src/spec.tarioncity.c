/*
** Specs for City of Tarion, by Liner
** z 128, 1/10/01
**
** Last Modified Dec 14, 2001
**
** -revamped the code after finding bugs with the loading of mankeys, beasts, and paktats
** in conjunction with the whistle spec.. had tried to modify it with allowing more mobs
** in zone reset cmds, but didn't work, so now just making "decoy" mobs to be summoned.
** -tested and works good.  no more bugs.  break is good.
** -also fixed stupid bug that allows whistle to not blow up if soitz specs on a pc in
** same room as beast to kill him.
** Last Modified May 31, 2002
**
** Fixing soitz so it doesn't kill mobs anymore, it's too powerful with new
** people building mobs that might not know to look for it, and people are still
** finding loopholes in it.
*/

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
#include "act.h"
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"

#define SIGN       12817
#define SIGNB      12825
#define WHISTLE    12826
#define SUIT       12818
#define STONE      12824
#define CHEF       12808
#define BEAST      12810
#define BEAST_DC   12830
#define MANKEY     12811
#define MANKEY_DC  12829
#define PAKTAT_DC  12828
#define ALTAR      12825
#define CAGE_ROOM  12848
#define CLEANSING  12845
#define CAVERNA    12839
#define CAVERNB    12842

int tc_sign(OBJ *obj ,CHAR *ch, int cmd, char *argument) {
  int a=FALSE, b=FALSE, c=FALSE, d=FALSE, e=FALSE, f=FALSE, g=FALSE;
  int h=FALSE, j=FALSE, k=FALSE, l=FALSE, m=FALSE, n=FALSE, o=FALSE;
  int p=FALSE, q=FALSE, r=FALSE,i;
  OBJ *tmp;
  char buf[MIL];

  if(!obj) return FALSE;
  if (cmd != CMD_USE) return FALSE;
  if(!ch) return FALSE;

  if (!EQ(ch, HOLD) || EQ(ch,HOLD) != obj) return FALSE;
  one_argument (argument, buf);
  if (!isname(buf,OBJ_NAME(obj))) return FALSE;

  for(i=0; i<=WIELD; i++) {
    if (!(tmp=ch->equipment[i]))
      continue;

    if (V_OBJ(tmp) == 12800) a=TRUE;
    if (V_OBJ(tmp) == 12801) b=TRUE;
    if (V_OBJ(tmp) == 12802) c=TRUE;
    if (V_OBJ(tmp) == 12803) d=TRUE;
    if (V_OBJ(tmp) == 12804) e=TRUE;
    if (V_OBJ(tmp) == 12805) f=TRUE;
    if (V_OBJ(tmp) == 12806) g=TRUE;
    if (V_OBJ(tmp) == 12807) h=TRUE;
    if (V_OBJ(tmp) == 12808) j=TRUE;
    if (V_OBJ(tmp) == 12809) k=TRUE;
    if (V_OBJ(tmp) == 12810) l=TRUE;
    if (V_OBJ(tmp) == 12811) m=TRUE;
    if (V_OBJ(tmp) == 12812) n=TRUE;
    if (V_OBJ(tmp) == 12813) o=TRUE;
    if (V_OBJ(tmp) == 12814) p=TRUE;
    if (V_OBJ(tmp) == 12815) q=TRUE;
    if (V_OBJ(tmp) == 12816) r=TRUE;
  }

  if (a && b && c && d && e && f && g && h && j && k && l && m && n && o &&
p && q && r) {
    for(i=0; i<MAX_WEAR; i++)
      extract_obj(ch->equipment[i]);

    act("$n waves the sibr fitfully around the room until it explodes in a purple halo of light! ",1,ch,0,0,TO_ROOM);
    act("You wave the sibr about your head, watching as it explodes in a purple halo of light! ",1,ch,0,0,TO_CHAR);
    act("The sibr disappears as its work is completed - something new has been left in your arms.",1,ch,0,0,TO_CHAR);

    switch(number(0,1))
    {
      case 0:
        if((tmp=read_object(12818,VIRTUAL)))
          obj_to_char(tmp,ch);
        break;
      case 1:
        if((tmp=read_object(12824,VIRTUAL)))
          obj_to_char(tmp,ch);
        break;
    }
    return TRUE;

  } else {
    send_to_char("Nothing seems to happen.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int tc_suit_spec(OBJ *suit ,CHAR *ch, int cmd, char *argument) {
  bool steal = FALSE;
  int bleed;
  char buf[MAX_INPUT_LENGTH];
  CHAR *owner,*target,*mankey;

  if(cmd==MSG_OBJ_REMOVED) {
    if(!suit->equipped_by) return FALSE;
    if(!(owner=suit->equipped_by)) return FALSE;
    unequip_char(owner,WEAR_BODY);
    suit->affected[0].modifier=3; /* Location 0 should be set as damroll */
    equip_char(owner,suit,WEAR_BODY);
    return FALSE;
  }

  if(cmd!=MSG_TICK && cmd!=MSG_MOBACT) return FALSE;
  if(!(owner=suit->equipped_by)) return FALSE;
  if(IS_NPC(owner)) return FALSE;

  if(cmd==MSG_TICK) {
    /*if(GET_HIT(owner)<-2) {
      GET_HIT(owner)-=hit_gain(owner);  reverse reg regen (accounts for regen gear)
    }*/
    if(chance(20)) {
      unequip_char(owner,WEAR_BODY);  /* Linerfix 110303 */
      suit->affected[0].modifier=number(3,4); /* Location 0 should be set as damroll */
      equip_char(owner,suit,WEAR_BODY);  /* Linerfix 110303 */
      if(GET_CLASS(owner)==CLASS_WARRIOR || GET_CLASS(owner)==CLASS_NOMAD) {
        /* regen spec */
        if(GET_HIT(owner)>-1)
          GET_HIT(owner)=MIN(GET_HIT(owner)+101,GET_MAX_HIT(owner));
      }
    }
  }

#ifdef TEST_SITE
  if(cmd==MSG_MOBACT && chance(100)) {
#else
  if(cmd==MSG_MOBACT && chance(25)) {
#endif
    if(GET_MAX_HIT(owner)<500) return FALSE;
    if(!owner->specials.fighting) return FALSE;
    if(GET_LEVEL(owner)<30) return FALSE;      /* added check for owner to be over 30 */

    target=owner->specials.fighting;
    if(GET_LEVEL(target)<31) return FALSE;

    if(target->specials.fighting!=owner) return FALSE;
    if((owner->ver3.death_limit) &&
       (owner->new.been_killed > owner->ver3.death_limit)) return FALSE;

    bleed=(GET_HIT(owner)*100)/GET_MAX_HIT(owner);
    if(bleed>=15) return FALSE;

    act("A laktzatz soitz screams in ecstasy as the world explodes in violet around you!",1,owner,0,0,TO_ROOM);
    act("A laktzatz soitz screams in ecstasy as the world explodes in violet around you!",1,owner,0,0,TO_CHAR);

    /* mankey thieving spec */
    if ((mankey = get_char_room("mankey", real_room(12841))) && !EQ(mankey, WEAR_BODY)) {
      switch (number(0, 5)) {
      case 0:
        steal = TRUE;
        break;
      case 1:
        if (chance(90))
          steal = TRUE;
        break;
      case 2:
        if (chance(80))
          steal = TRUE;
        break;
      case 3:
        if (chance(80))
          steal = TRUE;
        break;
      case 4:
        if (chance(40))
          steal = TRUE;
        break;
      case 5:
        if (chance(30))
          steal = TRUE;
        break;
      default:
        break;
      }
      if (steal) {
        /* Log entry added for soitz-steal. Solmyr 2009 */
        sprintf(buf, "WIZINFO: %s steals %s's %s", GET_NAME(mankey), GET_NAME(owner), OBJ_SHORT(suit));
        log_s(buf);
        suit->log = TRUE;
        act("The Mankey appears and steals something from $n, cheeky bastard!", 1, owner, 0, 0, TO_ROOM);
        act("The Mankey appears and steals something from you, cheeky bastard!", 1, owner, 0, 0, TO_CHAR);
        equip_char(mankey, unequip_char(owner, WEAR_BODY), WEAR_BODY);
      }
    }

    /* remove bulk of soitz spec - pointless garbage that disincentivizes use: Hemp 2020
    for(vict=world[CHAR_REAL_ROOM(owner)].people; vict;vict=next_vict) {
      next_vict = vict->next_in_room;

      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;

      tmp=vict->equipment[WEAR_NECK_1];
      tmp2=vict->equipment[WEAR_NECK_2];

      if(tmp && V_OBJ(tmp)==WHISTLE) {
        sprintf(buf, "WIZINFO: %s whistle exploded from Soitz spec", GET_NAME(vict));
        log_s(buf);
        act("A mirtzaemn myistzpae shatters from the force of the violet surge around it.",1,vict,0,0,TO_ROOM);
        act("Your mirtzaemn myistzpae shatters from the force of the violet surge around it.\n\r",1,vict,0,0,TO_CHAR);
        extract_obj(unequip_char(vict,WEAR_NECK_1));
      }

      if(tmp2 && V_OBJ(tmp2)==WHISTLE) {
        sprintf(buf, "WIZINFO: %s whistle exploded from Soitz spec", GET_NAME(vict));
        log_s(buf);
        act("A mirtzaemn myistzpae shatters from the force of the violet surge around it.",1,vict,0,0,TO_ROOM);
        act("Your mirtzaemn myistzpae shatters from the force of the violet surge around it.\n\r",1,vict,0,0,TO_CHAR);
        extract_obj(unequip_char(vict,WEAR_NECK_2));
      }

      switch(end_switch) {
        case 0:

        if(IS_NPC(vict) && GET_LEVEL(vict)>30) {
          if (!IS_SET(vict->specials.immune,IMMUNE_DISEMBOWEL)) {
            SET_BIT(vict->specials.immune,IMMUNE_DISEMBOWEL);
          }
          if (!IS_SET(vict->specials.immune,IMMUNE_HIT)) {
            SET_BIT(vict->specials.immune,IMMUNE_HIT);
          }
          if (!IS_SET(vict->specials.act,ACT_AGGRESSIVE)) {
            SET_BIT(vict->specials.act,ACT_AGGRESSIVE);
          }
          spell_fury(50,vict,vict,0);
        }
        if(IS_NPC(vict)) continue;
        GET_HIT(vict)=-8;
        GET_MANA(vict)=1;
        GET_MOVE(vict)=1;
        stop_fighting(vict);
        var=number(5,10);
        WAIT_STATE(vict,PULSE_VIOLENCE*var);
        break;

        case 1:

        if(IS_NPC(vict) && GET_LEVEL(vict)>30) {
          if (!IS_SET(vict->specials.immune,IMMUNE_DISEMBOWEL)) {
            SET_BIT(vict->specials.immune,IMMUNE_DISEMBOWEL);
          }
          if (!IS_SET(vict->specials.immune,IMMUNE_HIT)) {
            SET_BIT(vict->specials.immune,IMMUNE_HIT);
          }
          if (!IS_SET(vict->specials.act,ACT_AGGRESSIVE)) {
            SET_BIT(vict->specials.act,ACT_AGGRESSIVE);
          }
          spell_fury(50,vict,vict,0);
        }
        if(IS_NPC(vict)) continue;
        GET_HIT(vict)=-2;
        GET_MANA(vict)=1;
        GET_MOVE(vict)=1;
        stop_fighting(vict);
        var=number(5,10);
        WAIT_STATE(vict,PULSE_VIOLENCE*var);
        break;

        case 2:

        if(IS_NPC(vict) && GET_LEVEL(vict)>30) {
          if (!IS_SET(vict->specials.immune,IMMUNE_DISEMBOWEL)) {
            SET_BIT(vict->specials.immune,IMMUNE_DISEMBOWEL);
          }
          if (!IS_SET(vict->specials.immune,IMMUNE_HIT)) {
            SET_BIT(vict->specials.immune,IMMUNE_HIT);
          }
          if (!IS_SET(vict->specials.act,ACT_AGGRESSIVE)) {
            SET_BIT(vict->specials.act,ACT_AGGRESSIVE);
          }
          spell_fury(50,vict,vict,0);
        }
        if(IS_NPC(vict)) continue;
        GET_HIT(vict)=-3;
        GET_MANA(vict)=1;
        GET_MOVE(vict)=1;
        stop_fighting(vict);
        var=number(10,20);
        WAIT_STATE(vict,PULSE_VIOLENCE*var);
        break;

        case 3:

        if(IS_NPC(vict) && GET_LEVEL(vict)>30) {
          if (!IS_SET(vict->specials.immune,IMMUNE_DISEMBOWEL)) {
            SET_BIT(vict->specials.immune,IMMUNE_DISEMBOWEL);
          }
          if (!IS_SET(vict->specials.act,ACT_AGGRESSIVE)) {
            SET_BIT(vict->specials.act,ACT_AGGRESSIVE);
          }
          do_yell(vict, "Magic people, voodoo people.",CMD_YELL);
        }
        GET_HIT(vict)=GET_MAX_HIT(vict);
        GET_MANA(vict)=GET_MAX_MANA(vict);
        GET_MOVE(vict)=GET_MAX_MOVE(vict);
        stop_fighting(vict);
        var=number(5,10);
        WAIT_STATE(vict,PULSE_VIOLENCE*var);
        break;

        case 4:

        if(IS_NPC(vict) && GET_LEVEL(vict)>30) {
          if (!IS_SET(vict->specials.act,ACT_AGGRESSIVE)) {
            SET_BIT(vict->specials.act,ACT_AGGRESSIVE);
          }
          spell_fury(50,vict,vict,0);
        }
        GET_HIT(vict)=GET_MAX_HIT(vict);
        GET_MANA(vict)=GET_MAX_MANA(vict);
        GET_MOVE(vict)=GET_MAX_MOVE(vict);
        stop_fighting(vict);
        var=number(1,5);
        WAIT_STATE(vict,PULSE_VIOLENCE*var);
        break;

        case 5:

        if(IS_NPC(vict) && GET_LEVEL(vict)>30) {
          if (!IS_SET(vict->specials.immune,IMMUNE_DISEMBOWEL)) {
            SET_BIT(vict->specials.immune,IMMUNE_DISEMBOWEL);
          }
          spell_fury(50,vict,vict,0);
        }
        GET_HIT(vict)=GET_MAX_HIT(vict);
        GET_MANA(vict)=GET_MAX_MANA(vict);
        GET_MOVE(vict)=GET_MAX_MOVE(vict);
        stop_fighting(vict);
        var=number(5,9);
        WAIT_STATE(vict,PULSE_VIOLENCE*var);
        break;
      } // switch
    } // for
    */
  } // if
  return FALSE;
}

int tc_stone_spec(OBJ *stone ,CHAR *ch, int cmd, char *argument) {
  int owner_age;
  CHAR *vict;

  if(cmd!=MSG_TICK && cmd!=MSG_OBJ_REMOVED) return FALSE;
  if(!(vict=stone->equipped_by)) return FALSE;
  if(!AWAKE(vict)) return FALSE;

  if(cmd==MSG_OBJ_REMOVED) {
    unequip_char(vict,WIELD);
    stone->obj_flags.value[1]=9; /*damnodice*/
    stone->obj_flags.value[2]=3; /*damsizedice*/
    stone->affected[0].modifier=1; /* Location 0 should be set as damroll */
    equip_char(vict,stone,WIELD);
    OBJ_SPEC(stone)=1;  /* Linerfix 110303 */
    return FALSE;
  }

  if(cmd==MSG_TICK && OBJ_SPEC(stone)!=GET_AGE(vict)) {  /* Linerfix 110303 */
  owner_age=GET_AGE(vict)/10;
    unequip_char(vict,WIELD);  /* Linerfix 110303 */

    switch(owner_age) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=3; /*damsizedice*/
      stone->affected[0].modifier=1; /* Location 0 should be set as damroll */
      break;
    case 6:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=3; /*damsizedice*/
      stone->affected[0].modifier=2; /* Location 0 should be set as damroll */
      break;
    case 7:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=3; /*damsizedice*/
      stone->affected[0].modifier=3; /* Location 0 should be set as damroll */
      break;
    case 8:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=4; /*damsizedice*/
      stone->affected[0].modifier=0; /* Location 0 should be set as damroll */
      break;
    case 9:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=4; /*damsizedice*/
      stone->affected[0].modifier=1; /* Location 0 should be set as damroll */
      break;
    case 10:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=4; /*damsizedice*/
      stone->affected[0].modifier=2; /* Location 0 should be set as damroll */
      break;
    case 11:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=4; /*damsizedice*/
      stone->affected[0].modifier=3; /* Location 0 should be set as damroll */
      break;
    case 12:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=5; /*damsizedice*/
      stone->affected[0].modifier=0; /* Location 0 should be set as damroll */
      break;
    case 13:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=5; /*damsizedice*/
      stone->affected[0].modifier=1; /* Location 0 should be set as damroll */
      break;
    case 14:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=5; /*damsizedice*/
      stone->affected[0].modifier=2; /* Location 0 should be set as damroll */
      break;
    case 15:
    default:
      stone->obj_flags.value[1]=9; /*damnodice*/
      stone->obj_flags.value[2]=5; /*damsizedice*/
      stone->affected[0].modifier=3; /* Location 0 should be set as damroll */
      break;
  }
    equip_char(vict,stone,WIELD);   /* Linerfix 110303 */
    OBJ_SPEC(stone)=GET_AGE(vict);
    return FALSE;
  }
  return FALSE;
}


int tc_whistle_spec(OBJ *whistle ,CHAR *ch, int cmd, char *argument) {
  CHAR *vict,*next_vict,*mob;
  char arg[MAX_STRING_LENGTH];
  char buf[MIL];
  int loss,stored,need,gain,room,m,totalm=0;

  if(cmd==MSG_TICK) {
    if(!(vict=whistle->equipped_by)) return FALSE;

    if(whistle==EQ(vict,WEAR_NECK_1)) {       /* on tick, mana randomly changes 13-53 */
      unequip_char(vict,WEAR_NECK_1);
      switch(number(0,1)) {
        case 0:
          whistle->spec_value-=number(1,9);  /* whistle timer "decays" by 1-9 per tick */
          whistle->spec_value=MAX(whistle->spec_value,0);
          equip_char(vict,whistle,WEAR_NECK_1);
          break;
        case 1:
          whistle->affected[1].modifier=number(13,53);
          equip_char(vict,whistle,WEAR_NECK_1);
          break;
       }/* end switch */

       /* NEW - adds regen to beast wearing whistle so death thieves can't circle it
          to death without some real time input */

       if(IS_NPC(vict) && GET_HIT(vict)>-1) {

         if(V_MOB(vict) != 12810) return FALSE;

         GET_HIT(vict)=MIN(GET_HIT(vict)+number(400,1400),GET_MAX_HIT(vict));

       }
       return FALSE;

    }/* end if neck1 */

    if(whistle==EQ(vict,WEAR_NECK_2)) {       /* on tick, mana randomly changes 13-53 */
      unequip_char(vict,WEAR_NECK_2);
      switch(number(0,1)) {
        case 0:
          whistle->spec_value-=number(1,9);  /* whistle timer "decays" by 1-9 per tick */
          whistle->spec_value=MAX(whistle->spec_value,0);
          equip_char(vict,whistle,WEAR_NECK_2);
          break;
        case 1:
          whistle->affected[1].modifier=number(13,53);
          equip_char(vict,whistle,WEAR_NECK_2);
          break;
       }/* end switch */
       return FALSE;
    }/* end if neck2 */
    return FALSE;
  }/* end if MSG_TICK */

  if(!ch) return FALSE;
  if(whistle->equipped_by!=ch) return FALSE;

  switch(cmd) {

    case MSG_OBJ_REMOVED:
      if(whistle==EQ(ch,WEAR_NECK_1)) {
        unequip_char(ch,WEAR_NECK_1);
        whistle->affected[1].modifier=33;
        whistle->spec_value-=5;  /* whistle timer "decays" by 5 */
        whistle->spec_value=MAX(whistle->spec_value,0);
        equip_char(ch,whistle,WEAR_NECK_1);
      }
      if(whistle==EQ(ch,WEAR_NECK_2)) {
        unequip_char(ch,WEAR_NECK_2);
      whistle->affected[1].modifier=33;
      whistle->spec_value-=5;  /* whistle timer "decays" by 5 */
      whistle->spec_value=MAX(whistle->spec_value,0);
        equip_char(ch,whistle,WEAR_NECK_2);
      }
      return FALSE;
      break;

    case CMD_UNKNOWN:

      if (!AWAKE (ch)) return FALSE;
      if (IS_SET(ch->specials.affected_by,AFF_CHARM)) {
        act("You are charmed, no blow for you !",FALSE,ch,0,0,TO_CHAR);
        return FALSE;
      }
      argument=one_argument(argument,arg);
      if(!*arg) return FALSE;
      if(whistle->equipped_by!=ch) return FALSE;
      if(!is_abbrev(arg,"gpum")) return FALSE; /* gpum = blow */
      one_argument(argument,arg);
      if(!*arg) {
        send_to_char("Gpum myatz?\n\r",ch); /* "blow what" */
        return TRUE;
      }

      if(!isname(arg,OBJ_NAME(whistle))) {
        send_to_char("Myatz amnae huo tzmnhirb tzu gpum?\n\r",ch);
        return TRUE;
      }

      room = CHAR_REAL_ROOM(ch);

      if(count_mortals_real_room(room)<2 && count_mobs_real_room(room)<1) {
        send_to_char("The myistzpae fails to make any discernable sound.\n\r",ch);
        return TRUE;
      } else {

        act("You purse your lips and blow a mirtzaemn myistzpae as loudly as you can.\n\r",0,ch,0,0,TO_CHAR);
        act("$n blows a mirtzaemn myistzpae as loudly as $e can.",0,ch,0,0,TO_ROOM);

        loss=number(1,9);       /* loss is the amount rolled for each vict's mana to go down */

        for(vict=world[CHAR_REAL_ROOM(ch)].people; vict;vict=next_vict) {
          next_vict = vict->next_in_room;
          if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
          if(vict == ch) continue;
          if(IS_NPC(vict)) {
            GET_MANA(vict)-=9*loss; /* mob victs lose 9xloss */
          GET_MANA(vict)=MAX(GET_MANA(vict),0);
          m=GET_MANA(vict);
          totalm=m+totalm;
          }  else {
            GET_MANA(vict)-=loss;   /* victs lose loss, pcs */
            GET_MANA(vict)=MAX(GET_MANA(vict),0);
            m=GET_MANA(vict);
            totalm=m+totalm;
          }
        }

        if(totalm<11) {
          send_to_char("The myistzpae's sound has become weakened.\n\r",ch);
          return TRUE;
        }
        whistle->spec_value+=loss;  /* amount of "loss" rolled is added to timer on whistle, stored */
        whistle->spec_value=MIN(whistle->spec_value,1600);  /* up to 1600 total */
        if(ch->specials.fighting) {
          WAIT_STATE(ch,PULSE_VIOLENCE*2);  /* can only do every 2 rnds or so */
        } else {
          WAIT_STATE(ch,PULSE_VIOLENCE*5);  /* or 5 rnds if not fighting */
        }
        switch(loss) {   /* randomly summons mobs on the whistles, very low rate, according to loss # */

          case 4:

          #ifdef TEST_SITE
            if(chance(100)) {
          #else
            if(chance(1)) {
          #endif
              mob = read_mobile(BEAST_DC,VIRTUAL);
              room = CHAR_REAL_ROOM(ch);
              if(room == 12847) break;
              char_to_room(mob,room);
              act("\n\rA violet bubble bursts forth into the room, carrying some hairy being within.\n\r",FALSE,mob,0,0,TO_ROOM);
              if(chance(10) && ch->specials.fighting) {     /* mob can load fighting them at 85% or assist them 15% if they are already fighting */
                set_fighting(mob,ch->specials.fighting);
              }  else if (IS_MORTAL(ch)){
                set_fighting(mob,ch);
              }
            }
            break;

          case 3:

          #ifdef TEST_SITE
            if(chance(100)) {
          #else
            if(chance(2)) {
          #endif
              mob = read_mobile(MANKEY_DC,VIRTUAL);
              room = CHAR_REAL_ROOM(ch);
              if(room == 12841) break;
              char_to_room(mob,room);
              act("\n\rThe whistle echoes strangely throughout the room.. PRATY !!\n\r",FALSE,mob,0,0,TO_ROOM);
              if(chance(20) && ch->specials.fighting) {     /* mob can load fighting them at 75% or assist them 25% if they are already fighting */
                set_fighting(mob,ch->specials.fighting);
              }  else if (IS_MORTAL(ch)){
                set_fighting(mob,ch);
              }
            }
            break;

          case 1:

          #ifdef TEST_SITE
            if(chance(100)) {
          #else
            if(chance(2)) {
          #endif
              mob = read_mobile(PAKTAT_DC,VIRTUAL);
              room = CHAR_REAL_ROOM(ch);
              char_to_room(mob,room);
              act("\n\rThe whistle's call brings forth a strange purple creature from beyond !!\n\r",FALSE,mob,0,0,TO_ROOM);
              if(chance(20) && ch->specials.fighting) {     /* mob can load fighting them at 75% or assist them 25% if they are already fighting */
                set_fighting(mob,ch->specials.fighting);
              }  else if (IS_MORTAL(ch)){
                set_fighting(mob,ch);
              }
            }
            break;

        }/* end switch(loss) */
        return TRUE;
      }/* end else check for mortals and mobs in room */
      return FALSE;
      break;

    case CMD_USE:

      if (!AWAKE (ch)) return FALSE;
      if(!whistle) return FALSE;
      if(whistle->equipped_by!=ch) return FALSE;

      one_argument (argument, buf);
      if (!isname(buf,OBJ_NAME(whistle))) return FALSE; /* has to be "use myistzpae" */

      if(whistle->spec_value<=0) {
        send_to_char("The myistzpae doesn't seem to hold any power.\n\r",ch);
        return TRUE;
      }
      if(GET_LEVEL(ch)<LEVEL_IMM && (time_info.month > 2 || time_info.month < 15)) { /* if mort uses, during winter months.. it works */
        send_to_char("The myistzpae doesn't seem to work properly without the winter wind.\n\r",ch);
        return TRUE;
      } else {

        act("You hold a mirtzaemn myistzpae high in the air, gathering power from within it.\n\r",0,ch,0,0,TO_CHAR);
        act("$n holds a mirtzaemn myistzpae high in the air, gathering power from within it.",0,ch,0,0,TO_ROOM);

        need=GET_MAX_MANA(ch)-GET_MANA(ch);   /* need = amount of mana char is below max */
        stored=whistle->spec_value;       /* stored = timer-store of mana on whistle */
        gain=stored-need;                     /* gain = amount that would be left on whistle */
        if(gain>=0) {          /* if gain is positive, whistle will have some left */
          GET_MANA(ch)+=need;  /* make the ch get all of it, to max mana */
          GET_MANA(ch)=MIN(GET_MANA(ch),GET_MAX_MANA(ch));
          whistle->spec_value-=need;   /* subtract need from whistle timer */
          whistle->spec_value=MAX(whistle->spec_value,0);   /* redundant, to 0 */
        }
        if(gain<0) {          /* if gain is negative, whistle would go to 0 */
          GET_MANA(ch)+=stored;  /* give the char all the whistle has */
          GET_MANA(ch)=MIN(GET_MANA(ch),GET_MAX_MANA(ch));  /* redundant, to max */
          whistle->spec_value=0;   /* set whistle timer to 0 */
        }
        WAIT_STATE(ch,PULSE_VIOLENCE*2);  /* can only do every 2 rnds or so */
        return TRUE;

      }/* end else after winter check */
      break;

    }/* end switch */
  return FALSE;

}/* end int */

/* end new spec */

int tc_paktat_spec(CHAR *paktat, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  int loss;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!paktat->specials.fighting) return FALSE;

if(IS_DAY && chance(9)) {
    if(!(vict=get_random_victim_fighting(paktat))) return FALSE;
    if(IS_NPC(vict)) return FALSE;
    act("$n hops over $N in a playful game of leapfrog!",0,paktat,0,vict,TO_NOTVICT);
    act("$n hops over your head in a playful game of leapfrog!",0,paktat,0,vict,TO_VICT);

    loss=number(5001,105001);
    switch(number(0,1)) {
      case 0:
        GET_EXP(vict)-=loss;
        GET_EXP(vict)=MAX(GET_EXP(vict),0);
        break;
      case 1:
        GET_GOLD(vict)-=loss;
        GET_GOLD(vict)=MAX(GET_GOLD(vict),0);
        break;
    }
    damage(vict,paktat,number(0,26),TYPE_UNDEFINED,DAM_PHYSICAL);
  }

  if(count_mortals_real_room(real_room(12848))==1) return FALSE;

  if(IS_NIGHT && chance(10)) {
    if(!(vict=get_random_victim_fighting(paktat))) return FALSE;
    if(IS_NPC(vict)) return FALSE;

    act("$n appears, snatches $N, and disappears into thin air!",0,paktat,0,vict,TO_NOTVICT);
    act("You are suddenly spirited away by $n and placed in a cage!",0,paktat,0,vict,TO_VICT);
    char_from_room(vict);
    char_to_room(vict, real_room(CAGE_ROOM));
    SET_BIT(vict->specials.affected_by2,AFF2_IMMINENT_DEATH);
    if(!vict->specials.death_timer)
      vict->specials.death_timer=number(3,40);
    else

      vict->specials.death_timer=MIN(vict->specials.death_timer,number(3,40));

    act("$n is dropped into the cage by a paktat.",0,vict,0,0,TO_ROOM);
    do_look(vict,"",CMD_LOOK);
    signal_room(CHAR_REAL_ROOM(vict),vict,MSG_ENTER,"");
  }
  return FALSE;
}

int tc_chef_spec(CHAR *chef, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(cmd==MSG_TICK && chance(33)) {
    switch(number(0,1))
    {
    case 0:
      sprintf(buf, "emote whistles as it chops vegetables into the steaming pot.");
      command_interpreter (chef, buf);
      break;
    case 1:
      sprintf(buf, "emote pours a bit of spicy broth on your feet, grinning.");
      command_interpreter (chef, buf);
      break;
    }
  }
  return FALSE;
}


int tc_beast_spec(CHAR *beast, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  int factor;
  int beastlev;
  char buf[MIL];

if(cmd!=MSG_MOBACT && cmd!=MSG_ZONE_RESET) return FALSE;
if(cmd==MSG_ZONE_RESET) {

    beastlev=number(5,45);
   sprintf(buf, "emote tarkaes amnuort tzyae mnuuw, mnaefaemntzirb tzu itzs kyiptisy mahs.");
   command_interpreter (beast, buf);
   GET_LEVEL(beast)=beastlev;
   return FALSE;

}

  if(!beast->specials.fighting) return FALSE;

  factor=10*GET_HIT(beast)/GET_MAX_HIT(beast);
  switch(factor)
  {
    case 9:
      beast->specials.damnodice=4;
      beast->specials.damsizedice=2;
      break;

    case 8:
      beast->specials.damnodice=8;
      beast->specials.damsizedice=2;
      break;

    case 7:
      beast->specials.damnodice=16;
      beast->specials.damsizedice=2;
      break;

    case 6:
      beast->specials.damnodice=32;
      beast->specials.damsizedice=2;
      break;

    case 5:
      beast->specials.damnodice=64;
      beast->specials.damsizedice=2;
      break;

    case 4:
      beast->specials.damnodice=127;
      beast->specials.damsizedice=2;
      break;

    case 3:
      beast->specials.damnodice=127;
      beast->specials.damsizedice=4;
      break;

    case 2:
      beast->specials.damnodice=127;
      beast->specials.damsizedice=8;
      break;

    case 1:
      beast->specials.damnodice=127;
      beast->specials.damsizedice=16;
      break;
  }

  if(chance(30) && EQ(beast,WEAR_NECK_1)) {
    sprintf(buf, "gpum myistzpae");
    command_interpreter (beast, buf);
    return FALSE;
  }

  if(chance(5)) {
    if(!(vict=get_random_victim_fighting(beast))) return FALSE;
    if(IS_NPC(vict)) return FALSE;
    do_say(beast, "Huo amnaer'tz lpahirb faemnh vaimn.",CMD_SAY);
    SET_BIT(vict->specials.affected_by2,AFF2_IMMINENT_DEATH);
    if(!vict->specials.death_timer)
      vict->specials.death_timer=number(3,45);
    else

      vict->specials.death_timer=MIN(vict->specials.death_timer,number(3,45));
  }

  return FALSE;
}

int tc_mankey_spec(CHAR *mankey, CHAR *ch, int cmd, char *arg) {
  if(cmd!=MSG_MOBACT) return FALSE;

  if(!mankey->specials.fighting) return FALSE;
  if(chance(50)) {
    act("The mankey jumps off the walls, screaming 'PRATY'!",FALSE,mankey,0,0,TO_ROOM);
    mankey->points.max_hit=GET_HIT(mankey)+1;
  }
  return FALSE;
}

int tc_altar_spec(int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_STRING_LENGTH];
  CHAR *vict;
  int num_in_room;

  if(cmd!=CMD_UNKNOWN) return FALSE;
  if(!ch) return FALSE;
  if(!AWAKE(ch)) return FALSE;

  one_argument(arg, buf);

  if(!*buf) return FALSE;
  if(strcmp(buf,"sakmnivikae")) return FALSE;

  num_in_room=count_mortals_real_room(real_room(12848));
  if(num_in_room<1) {
    send_to_char("Your sacrifice is not necessary.\n\r",ch);
    return TRUE;
  }
  if(num_in_room>1) {
    send_to_char("There are too many people there.\n\r",ch);
    return TRUE;
  }

  for(vict=world[real_room(12848)].people;vict;vict=vict->next_in_room)
    if(IS_MORTAL(vict)) break;

  if((GET_LEVEL(vict)-GET_LEVEL(ch))>5) {
    send_to_char("You must be within 5 levels of the victim.\n\r",ch);
    return TRUE;
  }
  if(GET_AGE(ch)>64 && ch->specials.org_hit>499)  {
    act("Your sacrifice is accepted.",FALSE,ch,0,0,TO_CHAR);
    act("$n bows $s head before the altar, awaiting sacrifice.",FALSE,ch,0,0,TO_ROOM);

    char_from_room(vict);
    char_to_room(vict,real_room(12825));
    do_look(vict,"",CMD_LOOK);
    REMOVE_BIT(vict->specials.affected_by2,AFF2_IMMINENT_DEATH);
    vict->specials.death_timer=0;
    SET_BIT(ch->specials.affected_by2,AFF2_IMMINENT_DEATH);
    ch->specials.death_timer=2;
    return TRUE;
  }
  else  {
    act("You are too inexperienced to be sacrificed.",FALSE,ch,0,0,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}
int tc_lair_spec(int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int dir=-1;

  if(!ch) return FALSE;
  if (!AWAKE (ch)) return FALSE;
  if(!IS_MORTAL(ch)) return FALSE;

  for (;isspace(*arg);arg++);

  switch (cmd) {
    case CMD_NORTH:
      act("Tzyatz is rotz a kowwart yaemnae.",FALSE,ch,0,0,TO_CHAR);
      return TRUE;
      break;
    case CMD_SOUTH:
      act("Tzyatz is rotz a kowwart yaemnae.",FALSE,ch,0,0,TO_CHAR);
      return TRUE;
      break;
    case CMD_EAST:
      act("Tzyatz is rotz a kowwart yaemnae.",FALSE,ch,0,0,TO_CHAR);
      return TRUE;
      break;
    case CMD_WEST:
      act("Tzyatz is rotz a kowwart yaemnae.",FALSE,ch,0,0,TO_CHAR);
      return TRUE;
      break;
    case CMD_UP:
      act("Tzyatz is rotz a kowwart yaemnae.",FALSE,ch,0,0,TO_CHAR);
      return TRUE;
      break;
    case CMD_DOWN:
      act("Tzyatz is rotz a kowwart yaemnae.",FALSE,ch,0,0,TO_CHAR);
      return TRUE;
      break;
    case CMD_CAST:
      act("Tzyatz is rotz a kowwart yaemnae.",FALSE,ch,0,0,TO_CHAR);
      return TRUE;
      break;
    case CMD_CRAWL:
      act("Tzyatz is rotz a kowwart yaemnae.",FALSE,ch,0,0,TO_CHAR);
      return TRUE;
      break;
    case CMD_UNKNOWN:
      if(!*arg) return FALSE;

      if(!strcmp(arg,"rumntzy")) {
        dir=NORTH;
      }

      if(!strcmp(arg,"aeastz")) {
        dir=EAST;
      }

      if(!strcmp(arg,"suotzy")) {
        dir=SOUTH;
      }

      if(!strcmp(arg,"maestz")) {
        dir=WEST;
      }

      if(!strcmp(arg,"ol")) {
        dir=UP;
      }

      if(!strcmp(arg,"tomr")) {
        dir=DOWN;
      }

      if(!strcmp(arg,"kmnamp yupae")) {
        do_crawl(ch,"yupae",CMD_CRAWL);
        return TRUE;
      }

      if(!strncmp(arg,"kastz",5)) {
        arg=one_argument(arg,buf);
        do_cast(ch,arg,CMD_CAST);
        return TRUE;
      }


      if(dir>-1) {
        switch(GET_POS(ch)) {
          case POSITION_DEAD:
            send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
            break;
          case POSITION_INCAP:
          case POSITION_MORTALLYW:
            send_to_char("You are in a pretty bad shape, unable to do anything!\n\r", ch);
            break;
          case POSITION_STUNNED:
            send_to_char("All you can do right now, is think about the stars!\n\r", ch);
            break;
          case POSITION_SLEEPING:
            send_to_char("In your dreams, or what?\n\r", ch);
            break;
          case POSITION_RESTING:
            send_to_char("Nah... You feel too relaxed to do that..\n\r", ch);
            break;
          case POSITION_SITTING:
            send_to_char("Maybe you should get on your feet first?\n\r",ch);
            break;
          case POSITION_FIGHTING:
            send_to_char("No way! You are fighting for your life!\n\r", ch);
            break;
          default:
            do_simple_move(ch,dir,0);
            return TRUE;
            break;
        }
      }
      break;
  }
  return FALSE;
}

void check_equipment(CHAR *ch);
int tc_cleansing_spec(int cleansing, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*next_vict;

  if(tc_lair_spec(cleansing,ch,cmd,arg)) return TRUE;

  if(cmd!=MSG_ENTER) return FALSE;
  if(!ch) return FALSE;
  if(!IS_MORTAL(ch)) return FALSE;

  for(vict=world[CHAR_REAL_ROOM(ch)].people;vict;vict=next_vict)
    {
    next_vict = vict->next_in_room;
    act("\n\rThe room spins about in a torrent of blues and greens, cleansing you of all evil.",FALSE,vict,0,0,TO_CHAR);
    GET_MANA(vict)=0;
    vict->specials.alignment=1000;
    check_equipment(vict);
    }
  return FALSE;
}


int tc_cavern_spec(int cavern, CHAR *ch, int cmd, char *arg) {
  if(tc_lair_spec(cavern,ch,cmd,arg)) return TRUE;
  if (ch && cmd==MSG_ENTER) {
    act("\n\rA shiver comes over you as this enchanted cavern unfolds before your eyes.",FALSE,ch,0,0,TO_CHAR);
    return FALSE;
  }
  return FALSE;
}

int tc_decay_items(OBJ *obj ,CHAR *ch, int cmd, char *argument) {
  CHAR *vict;
  char buf[MIL],buf2[MIL];

  if(cmd==MSG_TICK) {
    vict=obj->carried_by;
    if(!vict)
      vict=obj->equipped_by;
    if(!vict) return FALSE;
    if(!IS_NPC(vict)) return FALSE;
    if(inzone(V_MOB(vict))==128) return FALSE;
    sprintf(buf,"%s",OBJ_NAME(obj));
    one_argument(buf,buf2);
    if(obj->equipped_by)
      do_remove(vict,buf,CMD_REMOVE);
    do_drop(vict,buf,CMD_DROP);
  }
  return FALSE;
}

void assign_tarioncity(void) {
  int i;
  assign_obj(SIGN, tc_sign);
  assign_obj(SIGNB, tc_sign);/*  added 19 str sibr */
  assign_obj(SUIT, tc_suit_spec);
  assign_obj(STONE, tc_stone_spec);
  assign_obj(WHISTLE, tc_whistle_spec);
  assign_mob(CHEF, tc_chef_spec);
  assign_mob(BEAST, tc_beast_spec);
  assign_mob(BEAST_DC, tc_beast_spec);
  assign_mob(MANKEY, tc_mankey_spec);
  assign_mob(MANKEY_DC, tc_mankey_spec);
  for(i = 12800; i < 12808; i++) assign_mob(i, tc_paktat_spec);
  for(i = 12812; i < 12829; i++) assign_mob(i, tc_paktat_spec);
  for(i = 12800; i < 12816; i++) assign_obj(i, tc_decay_items);
  assign_room(ALTAR, tc_altar_spec);
  assign_room(CAVERNA, tc_cavern_spec);
  assign_room(CAVERNB, tc_cavern_spec);
  assign_room(12840, tc_lair_spec);
  assign_room(12841, tc_lair_spec);
  assign_room(12846, tc_lair_spec);
  assign_room(12847, tc_lair_spec);
  assign_room(CLEANSING, tc_cleansing_spec);
}


