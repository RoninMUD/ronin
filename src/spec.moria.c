/* Specs for Moria by Blink
**
** for use in RoninMUD
**
** Addtions/modifications by Ranger - May 99
*/

/*
$Author: ronin $
$Date: 2004/11/16 04:59:08 $
$Header: /home/ronin/cvs/ronin/spec.moria.c,v 2.1 2004/11/16 04:59:08 ronin Exp $
$Id: spec.moria.c,v 2.1 2004/11/16 04:59:08 ronin Exp $
$Name:  $
$Log: spec.moria.c,v $
Revision 2.1  2004/11/16 04:59:08  ronin
Typo fix.

Revision 2.0.0.1  2004/02/05 16:11:06  ronin
Reinitialization of cvs archives

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
#include "enchant.h"
#include "act.h"
#include "spec_assign.h"
#include "mob.spells.h"

#define PORTAL_TARGET 21200

int moria_portal(OBJ *portal, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(!ch) return(FALSE);

  if (cmd != CMD_ENTER || !AWAKE (ch)) return FALSE;

  one_argument(arg,buf);
  if (!isname (buf, OBJ_NAME(portal))) return FALSE;

  if(GET_POS(ch)==POSITION_RIDING) {
     send_to_char("Dismount first.\n\r",ch);
     return TRUE;
  }

  act ("$n climbs into $p.", TRUE, ch, portal, 0, TO_ROOM);
  send_to_char ("You enter the shimmering portal.\n\r",ch);
  act ("$n disappears into the shimmering portal.",TRUE,ch,0,0,TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, real_room(PORTAL_TARGET));
  act ("$n drops to the ground from a swirling dark portal.",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  return TRUE;
}

int moria_thug(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;
  if(!mob->equipment[WIELD]) return FALSE;

  if(chance(75)) {
    vict=world[CHAR_REAL_ROOM(mob)].people;
    if(!vict) return FALSE;
    if(IS_NPC(vict)) return FALSE;

    act("$n plunges his Golden Dagger deep into $N's back.",0,mob,0,vict,TO_NOTVICT);
    act("You drive your weapon into $N's back.",0,mob,0,vict,TO_CHAR);
    act("You feel terrible pain as $n plunges something in your back.",0,mob,0,vict,TO_VICT);

    hit(mob,vict,SKILL_BACKSTAB);
    return FALSE;
  }

  return FALSE;
}

int moria_guardian(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd==CMD_WEST) {
    act("$n says 'I cannot let you pass!'",0,mob,0,ch,TO_ROOM);
    act("You step in front of $N, blocking $S way.",0,mob,0,ch,TO_CHAR);
    return TRUE;
 }
  return FALSE;
}


int moria_king(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(20)) {
    act("The Morian King surges with fury as he flails his arms!",0,mob,0,0,TO_ROOM);
    act("You go nuts and flail you arms at everyone",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;

      act("The Morian King surges with fury as he flails his arms!",0,mob,0,vict,TO_VICT);
      damage(mob,vict,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  if(chance(10)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("The King slaps $N in the face!",0,mob,0,vict,TO_NOTVICT);
    act("You slap $N in the face!",0,mob,0,vict,TO_CHAR);
    act("$N gets slapped in the face by the King.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  return FALSE;
}

int adrel_sage(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;
  static int trans=0;

  if (cmd == MSG_DIE) {
    trans=0;
    return FALSE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(5)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Adrel emits a tremendous loop of power at $N!",0,mob,0,vict,TO_NOTVICT);
    act("You emit a tremendous loop of power at $N!",0,mob,0,vict,TO_CHAR);
    act("Adrel emits a tremendous loop of power right at you!",0,mob,0,vict,TO_VICT);
    damage(mob,vict,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(10))   {
    act("$n creates a blazing fire, scorching the room.",0,mob,0,0,TO_ROOM);
    act("You burn the room with your power!",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      act("Adrel's magical fire sears your skin.",0,mob,0,vict,TO_VICT);
      damage(mob,vict,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  if(!trans && GET_HIT(mob)<GET_MAX_HIT(mob)/2) {
    trans=1;
    act("Adrel raises her hands above her head and hurls you into a dark portal.",TRUE,mob,0,0,TO_ROOM);
    for(vict = world[CHAR_REAL_ROOM(mob)].people;vict;vict = vict_n) {
      vict_n = vict->next_in_room;
      if(mob==vict) continue;
      char_from_room (vict);
      char_to_room(vict, real_room(21225));
      do_look(vict,"",CMD_LOOK);
    }
    return FALSE;
  }

  return FALSE;
}

int ulric_advisor(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(5)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Ulric conjures a small ball of ice and throws it at $N.",0,mob,0,vict,TO_NOTVICT);
    act("You conjure up a small ball of ice and throw it at $N.",0,mob,0,vict,TO_CHAR);
    act("Ulric throws a small ball of ice right in your face.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(5)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Ulric summons a massive ball of ice at throws it at $N!!",0,mob,0,vict,TO_NOTVICT);
    act("You emit a tremendous loop of power at $N!",0,mob,0,vict,TO_CHAR);
    act("Ulric sends a tremendous ball of ice at you!",0,mob,0,vict,TO_VICT);
    damage(mob,vict,450,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }
  return FALSE;
}

int griselda_queen(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(10)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Griselda moans and lays her icy hand on $N, draining them of life.",0,mob,0,vict,TO_NOTVICT);
    act("You drain $N's life with your icy hand.",0,mob,0,vict,TO_CHAR);
    act("You feel the life being drained from you as Griselda touches you.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }
  return FALSE;
}

int dark_priest(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(15)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("The Dark Priest showers $N in deadly dark magic!",0,mob,0,vict,TO_NOTVICT);
    act("You shower $N in your dark magics.",0,mob,0,vict,TO_CHAR);
    act("You scream in agony as the Dark Priest showers you with deadly magics.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }
  return FALSE;
}

int mykras_god(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;
  OBJ *object;
  char buf[MAX_INPUT_LENGTH];

  if(cmd==CMD_USE) {
    if(!ch) return FALSE;
    if(IS_NPC(ch)) return FALSE;
    if (!ch->equipment[WIELD]) return FALSE;
    if (ch->equipment[WIELD]->item_number_v != 21214) return FALSE;

    object = ch->equipment[WIELD];
    one_argument(arg, buf);
    if(!*buf) return FALSE;
    if(!isname(buf, OBJ_NAME(object))) return FALSE;
    act("$n' godly power dissapates as you drive the ritual dagger in his flesh.  ",
                  1, mob, 0, ch, TO_VICT);
    act("You lose your godly power as $N stabs you with the ritual dagger.",
                  1, mob, 0, ch, TO_CHAR);
    act("The dagger disappears into Mykras' body.",
                  1, mob, 0, ch, TO_VICT);
    act("$n' power disspates as $N drives the ritual dagger into his flesh.",
                  1, mob, 0, ch, TO_NOTVICT);
    object = unequip_char (ch, WIELD);
    extract_obj(object);
    REMOVE_BIT(mob->specials.immune, IMMUNE_HIT);
    REMOVE_BIT(mob->specials.immune, IMMUNE_BLUDGEON);
    REMOVE_BIT(mob->specials.immune, IMMUNE_PIERCE);
    REMOVE_BIT(mob->specials.immune, IMMUNE_SLASH);
    REMOVE_BIT(mob->specials.immune, IMMUNE_WHIP);
    REMOVE_BIT(mob->specials.immune, IMMUNE_CLAW);
    REMOVE_BIT(mob->specials.immune, IMMUNE_BITE);
    REMOVE_BIT(mob->specials.immune, IMMUNE_STING);
    REMOVE_BIT(mob->specials.immune, IMMUNE_CRUSH);
    REMOVE_BIT(mob->specials.immune, IMMUNE_HACK);
    REMOVE_BIT(mob->specials.immune, IMMUNE_CHOP);
    REMOVE_BIT(mob->specials.immune, IMMUNE_SLICE);
    REMOVE_BIT(mob->specials.immune, IMMUNE_BACKSTAB);
    return TRUE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(10)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Mykras claws at $N with his sharp talons.",0,mob,0,vict,TO_NOTVICT);
    act("You claw at $N with your talons.",0,mob,0,vict,TO_CHAR);
    act("You feel Mykras' deep talons penetrate your skin.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(10)) {
    act("Mykras creates a deadly smoke that fills the room.",0,mob,0,0,TO_ROOM);
    act("You create deadly smoke around the room!",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      act("You feel intense pain as you inhale some deadly smoke.",0,mob,0,vict,TO_VICT);
      damage(mob,vict,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  if(chance(5)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Mykras summons a cloud of smoke that engulfs $N.",0,mob,0,vict,TO_NOTVICT);
    act("You summon a cloud of smoke, engulfing $N.",0,mob,0,vict,TO_CHAR);
    act("Mykras engulfs you in a dense cloud of smoke.",0,mob,0,vict,TO_VICT);
    char_from_room(vict);
    char_to_room(vict,real_room(3014));
    do_look(vict,"",CMD_LOOK);
    return FALSE;
  }
  return FALSE;
}

int stone_dragon(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(5)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("The Huge Stone Dragon breathes fire at $N.",0,mob,0,vict,TO_NOTVICT);
    act("You breath fire at $N!",0,mob,0,vict,TO_CHAR);
    act("The Huge Stone Dragons breathes fire at you, burning you badly.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(4)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("The Huge Stone Dragon jumps into the air and tramples on $N.",0,mob,0,vict,TO_NOTVICT);
    act("You jump on $N, tramlping him",0,mob,0,vict,TO_CHAR);
    act("The Huge Stone Dragons jumps into the air and tramples you.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,550,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(5)) {
    act("The Huge Stone Dragon pounds the earth, showering the room with boulders.",0,mob,0,0,TO_ROOM);
    act("You pound the earth causing huge boulders to fall.",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      act("You are crushed by huge boulders falling from the ceiling.",0,mob,0,vict,TO_VICT);
      damage(mob,vict,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }
  return FALSE;
}


int uber_stone_dragon(CHAR *uber, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;
  char buf[MAX_INPUT_LENGTH];
  int i;
  OBJ *shield = NULL, *wield = NULL;

  if( cmd != MSG_MOBACT || !uber->specials.fighting) return FALSE;
  
  // when it first sets its SHIELD bit it will stone 3 random people
  // when it gets below 80% hp it loses SHIELD and stones 3 random people again
  if( IS_SET( uber->specials.act, ACT_AGGNO ) && ( ( 4*GET_MAX_HIT(uber)/5 ) > GET_HIT(uber) ) )
  {   // if is <= 80% hp  and still has SHIELD
    REMOVE_BIT( uber->specials.act, ACT_AGGNO );
    sprintf( buf, "%s roars angrily, and a thundering crack of shattered stone echoes off the walls.\n\r", GET_SHORT(uber) );
    send_to_room(buf, CHAR_REAL_ROOM(uber) );
    for( i = 0; i < 3; i++ ) {
      vict = get_random_victim(uber);
      if( vict ) {
         spell_petrify( GET_LEVEL(uber), uber, vict, 0);
      }
    }    
  }
  else if( ( 7*GET_MAX_HIT(uber)/10 ) <= GET_HIT(uber) ) 
  { // if is > 70% hp
    if( !IS_SET( uber->specials.act, ACT_SHIELD ) ) {
      sprintf( buf, "%s roars loudly, causing the stone of the cavern and the earth to shiver and quake at its power.\n\r", GET_SHORT(uber) );
      send_to_room(buf, CHAR_REAL_ROOM(uber) );
      for( i = 0; i < 3; i++ ) {
        vict = get_random_victim(uber);
        if( vict ) {
           spell_petrify( GET_LEVEL(uber), uber, vict, 0);
        }
      }
      SET_BIT( uber->specials.act, ACT_SHIELD );
    }
  } 
  
  switch ( number(1,20) ) {
    case 1:
    case 2:
    case 3: // damage and drop weapon/shield
      vict = get_random_victim( uber );
      if( vict ) {
        act("$n seizes $N in a stone claw and brutally smashes $M into the cavern wall.",0,uber,0,vict,TO_NOTVICT);
        act("You seize $N in a stone claw and brutally smash $M into the cavern wall.",0,uber,0,vict,TO_CHAR);
        act("$n seizes you in a stone claw and brutally smashes you into the cavern wall.",0,uber,0,vict,TO_VICT);
        if( EQ(vict, WEAR_SHIELD)) {
          shield = vict->equipment[WEAR_SHIELD];
          sprintf(buf, "WIZINFO: %s dropped %s in %d", GET_NAME(vict), OBJ_SHORT(shield), V_ROOM(vict) );
          log_s(buf);
          shield->log=1;
          obj_to_room( unequip_char(vict, WEAR_SHIELD), CHAR_REAL_ROOM(vict) );
          save_char(vict, NOWHERE);
        }
        if( EQ(vict, WIELD)) {
          wield = vict->equipment[WIELD];
          sprintf(buf, "WIZINFO: %s dropped %s in %d", GET_NAME(vict), OBJ_SHORT(wield), V_ROOM(vict) );
          log_s(buf);
          wield->log=1;
          obj_to_room( unequip_char(vict, WIELD), CHAR_REAL_ROOM(vict) );
          save_char(vict, NOWHERE);
        }
        
        // create string messages to victim based on what was dropped
        if( wield && shield ) {
          sprintf( buf, "The force of the blow causes you to drop your %s and %s in stunned shock.\n\r", OBJ_SHORT(wield), OBJ_SHORT(shield) );
        } else if ( wield ) {
          sprintf( buf, "The force of the blow causes you to drop your %s in stunned shock.\n\r", OBJ_SHORT(wield) );
        } else if ( shield ) {
          sprintf( buf, "The force of the blow causes you to drop your %s in stunned shock.\n\r", OBJ_SHORT(shield) );
        }
        if ( wield || shield ) {
          send_to_char( buf, vict );
        }
        damage(uber, vict, number(700,1000), TYPE_UNDEFINED, DAM_NO_BLOCK);
        stop_fighting(vict);
      }
      break;
    case 4:
    case 5:
    case 6:
    case 7: // damage and stun
      vict = get_random_victim( uber );
      act("$n buffets $N mercilessly with a strong gust as it flaps its stony wings, before swiping at $M with a rocky claw.",0,uber,0,vict,TO_NOTVICT);
      act("You buffet $N mercilessly with a strong gust as you flap your stony wings, before swiping at $M with a rocky claw.",0,uber,0,vict,TO_CHAR);
      act("$n buffets you mercilessly with a strong gust as it flaps its stony wings, before swiping at you with a rocky claw.",0,uber,0,vict,TO_VICT);
      damage(uber, vict, number(1000,1200), TYPE_UNDEFINED, DAM_NO_BLOCK);
      GET_POS(vict) = POSITION_STUNNED;
      WAIT_STATE( vict, PULSE_VIOLENCE * 5 );
      break;
    case 8:
    case 9:
    case 10: // room damage and sitting
      act("$n sweeps its giant tail in a broad arc through the cavern.",0,uber,0,0,TO_ROOM);
      act("You whip your tail around the cavern in a broad arc.",0,uber,0,0,TO_CHAR);
      for( vict=world[CHAR_REAL_ROOM(uber)].people; vict; vict = vict_n ) {
        vict_n = vict->next_in_room;
        if( vict == uber ) continue;
        if( ( !IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM ) || IS_NPC(vict) ) continue;
        act("You are clipped by $n's enormous tail, which hits with the force of a cannonball.",0,uber,0,vict,TO_VICT);
        damage(uber, vict, number(500,750), TYPE_UNDEFINED, DAM_NO_BLOCK);
        GET_POS(vict) = POSITION_SITTING;
      }  
      break;
    default:
      break;
  }
  return FALSE;
}

#define MAUL_CHARGE   100

int boulder_maul(OBJ *maul, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL;
  char buf[MAX_INPUT_LENGTH];
  int strmod = 0, spec_chance = 0;
  ch = maul->equipped_by;
  
  if(cmd==MSG_MOBACT  && ch && !IS_NPC(ch) && ch->specials.fighting) {
	  // damage spec, charge builds up in about 5.5 ticks of fighting
    vict = ch->specials.fighting;
    OBJ_SPEC(maul)++;

    if(OBJ_SPEC(maul) > MAUL_CHARGE) {
      strcpy( buf, world[CHAR_REAL_ROOM(ch)].name );
      spec_chance = 1 + ( ( OBJ_SPEC(maul) - MAUL_CHARGE ) / 5 );
      
      // increased likelihood in Mountain sector, and "cave"-like room
      if( world[CHAR_REAL_ROOM(ch)].sector_type == SECT_MOUNTAIN ) {
        spec_chance+=5;
      }
      if( strstr( string_to_lower( buf ), "cave" ) ) {
        spec_chance+=5;
      }
      
      if( chance( spec_chance ) ) {
        act("$n lets out a thundering, inhuman roar, swinging $s maul with draconic-like strength.\n\r$n strikes $N with godly force, causing a small shockwave and spraying gore in every direction.",0,ch,0,vict,TO_NOTVICT);
        act("You let out a thundering, inhuman roar, swinging your maul with draconic-like strength.\n\rYou strike $N with godly force, causing a small shockwave and spraying gore in every direction.",0,ch,0,vict,TO_CHAR);
        act("$n lets out a thundering, inhuman roar, swinging $s maul with draconic-like strength.\n\r$n strikes you with godly force, causing a small shockwave and spraying gore in every direction.",0,ch,0,vict,TO_VICT);
        strmod = GET_STR(ch) - 18;
        damage(ch, vict, MIN (1250, 500 + ( strmod * number( 50, OBJ_SPEC(maul) ) ) ), TYPE_UNDEFINED, DAM_NO_BLOCK);
        OBJ_SPEC(maul) = 0;
      }
    }
  }
  return FALSE;
}

/*when item #21213 is dropped at room# 21263 it will teleport everyone in
there to room #21264*/
int find_obj(int v_num, int virt_room); /* From spec.wyvern.c */
int moria_teleport_to_mykras(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  char buf[MAX_INPUT_LENGTH];
  CHAR *tel_ch,*tel_char_next;
  if(!ch) return FALSE;
  if(cmd!=CMD_DROP) return FALSE;
  one_argument (argument, buf);
  if(!isname(buf,OBJ_NAME(obj)))
    return FALSE;

  do_drop(ch, argument, cmd);
  if(!find_obj(21213,21263)) return TRUE;

  act("You breifly phase out of reality, and return in another world.",TRUE,ch,obj,0,TO_ROOM);
  act("You breifly feel disoriented, then realise you are in another world.",TRUE,ch,obj,0,TO_CHAR);

  for(tel_ch = world[real_room(21263)].people;tel_ch;tel_ch = tel_char_next) {
    tel_char_next = tel_ch->next_in_room;
    char_from_room (tel_ch);
    char_to_room(tel_ch, real_room(21264));
    do_look(tel_ch,"",CMD_LOOK);
  }

  extract_obj(obj);
  return TRUE;
}


int jeweler(CHAR *mob, CHAR *ch, int cmd, char *arg);
int postoffice(CHAR *mob, CHAR *ch, int cmd, char *arg);

void assign_moria(void) {
  assign_obj(4108,moria_portal);
  assign_mob(4110,moria_guardian);
  assign_mob(21203,moria_king);
  assign_mob(21204,adrel_sage);
  assign_mob(21223,moria_thug);
  assign_mob(21205,ulric_advisor);
  assign_mob(21206,griselda_queen);
  assign_mob(21210,dark_priest);
  assign_mob(21211,mykras_god);
  assign_mob(21219,stone_dragon);
  assign_mob(21217,jeweler);
  assign_mob(21214,postoffice);
  assign_mob(21220,uber_stone_dragon);
  assign_obj(21213,moria_teleport_to_mykras);
  assign_obj(21228,boulder_maul);
}

