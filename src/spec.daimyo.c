/*
///   Special Procedure Module                   Orig. Date 12-20-1996
///                                              Last Modif 08-04-1998
///       By Thyas, Sumo and Ranger
///       written for RoninMUD
///
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
#include "fight.h"
#include "utility.h"
#include "act.h"
#include "spec_assign.h"

extern int CHAOSMODE;
extern struct time_info_data time_info;
extern struct char_data *character_list;
extern struct mob_proto *mob_proto_table;
extern OBJ  *object_list;
extern CHAR *character_list;

#define NEW_ROOM                  585
#define SCROLL_ROOM               584
#define HACHI_ROOM              20133
#define HACHI_START_ROOM        20132
#define BONE_WAND               20131
#define JADE_HELM               20138
#define NINJA_ROOM              20147

#define TONASHI_WRESTLER          540
#define SHOGUN_WARLORD          20145
#define NINJA_ASSASIN           20147
#define BINDING_SCROLL            595
#define BLOODY_KEY              20112
#define HACHIMAN_STATUE         20111
#define BLACK_PANTHER           20165
#define DM_PORTAL                 586
#define DM_PORTAL_INSIDE          586
#define ARAGO                     585
#define HACHIMAN                20133
#define SKELETAL_DRAGON         20134
#define DRAGON_ROOM             20134
#define CRONE                     571
#define BREASTPLATE             20136
#define BRACERS                 20135
#define GAUNTLETS               20133
#define BATTLESUIT              20132
#define DOOR_ROOM               20196
#define PRYBAR                    516
#define SIGNET_RING             20196
#define GONG                      599
#define THORN                     570
#define THORN_THOTH_VANITY        129
#define ROOM_DOWN                 554
#define ROOM_NORTH              20198
void stop_riding(struct char_data *ch,struct char_data *vict);

int dm_jade_helm(OBJ *obj, CHAR *ch,int cmd, char *arg) {
  OBJ *corpse;
  CHAR *vict;
  static int helm_damage=0;

  /* have it only spec every 5 hours during daylight OR every 2 during night */
  if(cmd==MSG_DEATHCRY) {
    if(!ch) return FALSE;
    if(IS_NPC(ch)) return FALSE;
    if(!obj->equipped_by) return(FALSE);
    if(ch!=obj->equipped_by) return FALSE;
    if(obj!=EQ(ch,WEAR_HEAD)) return(FALSE);

    if(IS_DAY && obj->obj_flags.value[3] && time_info.hours%5==0) {
      if(generic_find("corpse", FIND_OBJ_ROOM, ch, &vict, &corpse)) {
        if(corpse->obj_flags.timer != MAX_NPC_CORPSE_TIME) return FALSE;
        act("Crimson mist from your $p envelops the corpse, turning it into a withered husk!", FALSE,ch,obj,0,TO_CHAR);
        act("Strange crimson mist from $n's $p envelops the corpse, turning it into a withered husk!", FALSE,ch,obj,0,TO_ROOM);
        spell_spirit_levy(30,ch,0,corpse);
        obj->obj_flags.value[3]-=2;
        obj->obj_flags.value[3]=MAX(obj->obj_flags.value[3],2);
      }
     return(FALSE);
    }
    else if(IS_NIGHT && obj->obj_flags.value[3] && time_info.hours%2==0) {
      if(generic_find("corpse", FIND_OBJ_ROOM, ch, &vict, &corpse)) {
        act("Your $p envelops the corpse in a blazing crimson mist, turning it into a withered husk!", FALSE,ch,obj,0,TO_CHAR);
        act("$n's $p envelops the corpse in a blazing crimson mist, turning it into a withered husk!", FALSE,ch,obj,0,TO_ROOM);
        spell_spirit_levy(30,ch,0,corpse);
        obj->obj_flags.value[3]-=5;
        obj->obj_flags.value[3]=MAX(obj->obj_flags.value[3],2);
      }
      return(FALSE);
    }
    return FALSE;
  }

  if(cmd==MSG_TICK) {
    if(obj->obj_flags.value[3]<=0) {
      obj->obj_flags.value[3]=100;
      return FALSE;
    }
    if(!obj->equipped_by) return(FALSE);
    vict=obj->equipped_by;
    if(IS_NPC(vict)) return FALSE;
    if(obj!=EQ(vict,WEAR_HEAD)) return(FALSE);

    if(obj->obj_flags.value[3] && !vict->specials.fighting)
      obj->obj_flags.value[3]--;

    if(obj->obj_flags.value[3]<2) {
      obj->obj_flags.value[3]=100;
      if(!saves_spell(vict, SAVING_SPELL,GET_LEVEL(vict))) return FALSE;
      act("Your $p explodes with tremendous force!", FALSE,vict,obj,0,TO_CHAR);
      act("$n's $p explodes with thunderous force!", FALSE,vict,obj,0,TO_ROOM);
      helm_damage=1;
      damage(vict,vict,GET_MAX_HIT(vict) /2, TYPE_UNDEFINED,DAM_NO_BLOCK);
      helm_damage=0;
      if(CHAR_REAL_ROOM(vict)==NOWHERE) return FALSE;
    }
    return FALSE;
  }

  if(cmd==MSG_DEAD && helm_damage) {
    if(!obj->equipped_by) return(FALSE);
    vict=obj->equipped_by;
    if(obj!=EQ(vict,WEAR_HEAD)) return(FALSE);
    if(ch!=vict) return FALSE;
    act("The last thing you feel is your headless body twitching on the ground.", FALSE,vict,obj,0,TO_CHAR);
    act("$n's $p suddenly destroys itself in a thunderous explosion, taking $s head with it!", FALSE,vict,obj,0,TO_ROOM);
    extract_obj(unequip_char(vict,WEAR_HEAD));
    return FALSE;
  }
  return FALSE;
}

int binding_scroll (OBJ *obj, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_STRING_LENGTH];

  if (!ch) return FALSE;

  if (CHAR_REAL_ROOM(ch) !=(real_room(SCROLL_ROOM))) return FALSE;

  one_argument(arg,buf);

  if (cmd == CMD_USE) {
      if (!*buf) return FALSE;

      if (!isname(buf, OBJ_NAME(obj)))  return FALSE;

      if (!EQ(ch,HOLD) || EQ(ch,HOLD) != obj) {
        act("You are not holding the scroll.",TRUE,ch,0,0,TO_CHAR);
        return TRUE;
      }

      act ("$n lights a scroll in the brazier..\n\r.",FALSE,ch,0,0,TO_ROOM);
      act ("You light the scroll in the brazier...\n\r",FALSE,ch,0,0,TO_CHAR);

      send_to_room ("The fire burns an eerie green color as it hungrily devours the scroll!\n\r",real_room(SCROLL_ROOM));
      send_to_room ("Flames hotter than the sun attack the west wall, quickly melting it down.\n\r",real_room(SCROLL_ROOM));
      send_to_room ("The wall to the east quickly melts away openin a large passage!\n\r",real_room(NEW_ROOM));

      extract_obj(unequip_char(ch,HOLD));

      world[real_room(SCROLL_ROOM)].dir_option[WEST]->to_room_r = real_room(NEW_ROOM);
      world[real_room(NEW_ROOM)].dir_option[EAST]->to_room_r = real_room(SCROLL_ROOM);
      return TRUE;
  }
  return FALSE;
}

int tonashi_wrestler(CHAR *mob, CHAR *ch, int cmd, char *arg) {
   CHAR *vict;
   struct descriptor_data *d;
   int new_room = NOWHERE, a_victim=FALSE,attack=FALSE;

   if(!AWAKE(mob)) return FALSE;

   if(!mob->specials.fighting) return FALSE;

   if(cmd==MSG_MOBACT) {
     vict = get_random_victim_fighting(mob);
     if(vict && vict!=mob->specials.fighting && !number(0,4)) a_victim=TRUE;
   }

   if(cmd==CMD_PUNCH || cmd==CMD_CIRCLE || cmd==CMD_PUMMEL || cmd==CMD_KICK) {
     if(!ch) return FALSE;
     vict=ch;
     a_victim=TRUE;
     attack=TRUE;
   }

   if(!a_victim) return FALSE;

   d=vict->desc;
   if(!d) return FALSE;
   spell_fury(20,mob,mob,0);
   act("$n lets out a bone-chilling scream and charges headlong at $N.",
        1, mob, 0, vict, TO_NOTVICT);
   act("$n lets out a bone-chilling scream and charges headlong at you.",
        1, mob, 0 , vict, TO_VICT);
   act("You let out a bone-chilling scream and charge headlong at $N.",
        1, mob, 0, vict, TO_CHAR);
   damage(mob, vict, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);

   if(d->connected!=CON_PLYNG) return FALSE; /* If char died */

   if(vict->specials.riding) stop_riding(vict,vict->specials.riding);
   char_from_room(vict);

   switch(number(1,3)) {
    case 1: new_room = 536;
      break;
    case 2: new_room = 538;
      break;
    case 3: new_room = 540;
      break;
   }

   char_to_room(vict,real_room(new_room));
   do_look(vict,"",CMD_LOOK);
   act("$n comes crashing into the room.", FALSE, vict, 0, 0, TO_ROOM);
   if(attack) return TRUE;
   else return FALSE;
}

int warlord(CHAR *mob, CHAR *ch, int cmd, char *arg) {
   CHAR *vict, *ninja;
   int percent, num_ninjas;

   if(!AWAKE(mob)) return FALSE;

   switch(cmd) {
    case MSG_MOBACT:
      if(!mob->specials.fighting) return FALSE;
      if(!(vict=get_random_victim(mob))) return FALSE;
      if(GET_MAX_HIT(mob) > 0)
	    percent = (100*GET_HIT(mob))/GET_MAX_HIT(mob);
      else return FALSE;

      if (percent>20) return FALSE; /* Awful condition for ninja to rescue. */

      num_ninjas = 0;
      for (ninja = world[CHAR_REAL_ROOM(mob)].people; ninja; ninja = ninja->next_in_room) {
        if(IS_MOB(ninja)) {
          if(V_MOB(ninja)==NINJA_ASSASIN) num_ninjas = num_ninjas + 1;
        }
      }

      if(num_ninjas < 6) {
        // pull in an existing ninja from the adjoining room
        for (ninja = world[real_room(NINJA_ROOM)].people; ninja; ninja = ninja->next_in_room) {
          if (IS_MOB(ninja) && (V_MOB(ninja) == NINJA_ASSASIN)) break;
        }

        if (ninja) {
          char_from_room(ninja);
        }
        else {
          ninja = read_mobile(NINJA_ASSASIN,VIRTUAL);
        }

        char_to_room(ninja, CHAR_REAL_ROOM(vict));
        act("A black garbed ninja appears from the west, quickly draws his blade and attacks YOU!", FALSE, ninja, 0, vict, TO_VICT);
        act("You appear from the west and attack $N!", FALSE, ninja, 0, vict, TO_CHAR);
        act("A black garbed ninja appears from the west and attacks $N!", FALSE, ninja, 0, vict, TO_NOTVICT);
        set_fighting(ninja, vict);
      }
      break;
   }
   return FALSE;
}

int hachi_statue (OBJ *statue, CHAR *ch, int cmd, char *arg) {
  OBJ *key;
  char buf[MAX_INPUT_LENGTH];
  int keybloody;

  if(!ch) return FALSE;
  keybloody = real_object(BLOODY_KEY);

  if (cmd == CMD_UNLOCK) {
    one_argument(arg,buf);
    if (!isname(buf, OBJ_NAME(statue))) return FALSE;

    if (!(key = get_obj_in_list_num (keybloody, ch->carrying))) return FALSE;

    send_to_char ("*CLICK*\n\r*CRACK*\n\r",ch);
    REMOVE_BIT(statue->obj_flags.value[1], CONT_LOCKED);
    act("As $n unlocks $p, the key crumbles to dust.",FALSE,ch,statue,0,TO_ROOM);
    extract_obj (key);
    return TRUE;
  }
  return FALSE;
}

int black_panther(CHAR *mob,CHAR *ch, int cmd, char *arg) {
   if(!ch) return FALSE;

   if(cmd!=MSG_ENTER) return FALSE;
   if (mob->specials.fighting) return FALSE;
   if (!CAN_SEE(mob,ch)) return FALSE;

   act("The $n jumps at $N's throat!",0,mob,0,ch,TO_NOTVICT);
   act("You attempt to rip $N's throat out!",0,mob,0,ch,TO_CHAR);
   act("With a loud growl, the $n pounces upon you!",0,mob,0,ch,TO_VICT);
   hit(mob, ch, TYPE_HIT);
   hit(mob, ch, TYPE_HIT);
   return FALSE;
}

int dm_portal (OBJ *portal, CHAR *ch, int cmd, char *arg) {
  int inside_portal;
  char buf[MAX_INPUT_LENGTH];
  if(!ch) return(FALSE);

  inside_portal = real_room(DM_PORTAL_INSIDE);

  if (cmd != CMD_ENTER || !AWAKE (ch)) return FALSE;

  one_argument (arg,buf);
  if (!isname (buf, OBJ_NAME(portal)))
    return FALSE;

  if(GET_POS(ch)==POSITION_RIDING) {
     send_to_char("Dismount first.\n\r",ch);
     return TRUE;
  }
  act ("$n climbs into $p.", TRUE, ch, portal, 0, TO_ROOM);
  send_to_char ("You enter the shimmering onyx portal.\n\r",ch);
  act ("$n disappears into the portal's rippling surface.",TRUE,ch,0,0,TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, inside_portal);
  act ("$n appears out of nowhere.",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  return TRUE;
}

int dm_portal_inside(int room, CHAR *ch, int cmd, char *arg)
{

    if(cmd!=CMD_SONG) return FALSE;

    if(!ch) return FALSE;
    send_to_char("The room absorbs the magic of your song.\n\r",ch);

    return TRUE;
}

int bone_wand (OBJ *wand, CHAR *ch, int cmd, char *arg) {
  char victname[MAX_INPUT_LENGTH],objname[MAX_INPUT_LENGTH];
  CHAR *vict;

  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;

  if (cmd != CMD_USE || !AWAKE (ch) || wand->equipped_by != ch)
    return FALSE;

  arg = one_argument(arg, objname);
  arg = one_argument(arg, victname);

  if (!*objname) {
    send_to_char ("What are you trying to use?\n\r",ch);
    return TRUE;
  }

  if(!isname(objname, OBJ_NAME(wand))) {
    send_to_char("You do not hold that item in your hand.\n\r",ch);
    return TRUE;
  }

  if(V_ROOM(ch)!= HACHI_START_ROOM) {
    send_to_char("The wand seems powerless here.",ch);
    return TRUE;
  }

  if (!*victname) {
    send_to_char ("Who are you trying to use the wand on?\n\r",ch);
    return TRUE;
  }

  if (!(vict = get_char_room_vis (ch, victname))) {
    send_to_char ("You try and try but cant!\n\r",ch);
    return TRUE;
  }

  if(IS_NPC(vict)) {
    send_to_char ("It doesn't seem to work.\n\r",ch);
    return TRUE;
  }

  if(GET_POS(vict)==POSITION_FIGHTING) {
    act("$N seems a little busy.",0,ch,0,vict,TO_CHAR);
    return TRUE;
  }
  if(GET_POS(vict)==POSITION_RIDING) {
    act("$N's mount looks at you in annoyance.",0,ch,0,vict,TO_CHAR);
    return TRUE;
  }
  if(ch!=vict) {
    act("$N disappears in a flash of light as you point the wand at $M.",0,ch,0,vict,TO_CHAR);
    act("$n points $s Cracked Bone Wand at you.",0,ch,0,vict,TO_VICT);
    act("$n points a Cracked Bone Wand at $N, and $E disappears in a flash of light.",0,ch,0,vict,TO_NOTVICT);
  } else {
    act("You disappear in a flash of light as you point the wand at yourself.",0,ch,0,0,TO_CHAR);
    act("$n points a Cracked Bone Wand at $mself, and $e disappears in a flash of light.",0,ch,0,0,TO_ROOM);
    act("\n\rThe wand suddenly breaks in half.",0,ch,0,0,TO_CHAR);
    extract_obj(unequip_char(ch,HOLD));
   }
  char_from_room(vict);
  char_to_room(vict, real_room(HACHI_ROOM));
  act("$n suddenly appears in a flash of light.",0,ch,0,vict,TO_NOTVICT);
  act("Suddenly you find yourself somewhere else!",0,ch,0,vict,TO_VICT);
  do_look(vict,"",CMD_LOOK);
  signal_room(CHAR_REAL_ROOM(vict),vict,MSG_ENTER,"");
  return TRUE;
}

int dm_arago (CHAR *arago, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  int i;
  int num = 0;
  struct descriptor_data *d;

  if(CHAR_REAL_ROOM(arago)!=real_room(NEW_ROOM)) return FALSE;
  if (cmd==MSG_MOBACT) {
    if (!arago->specials.fighting) return FALSE;
    if(GET_HIT(arago->specials.fighting) <=1200) {
    num = number(0,4);
   }
    if(GET_HIT(arago->specials.fighting) >1200) {
    num = number(0,3);
   }
    if(GET_HIT(arago->specials.fighting) >=1350) {
    num = number(0,2);
   }
    vict=get_random_victim_fighting(arago);
    if(vict && vict!=arago->specials.fighting && !num ) {
      d=vict->desc;
      if(!d) return FALSE;
      act ("$n utters a curse and hurls $N at the portal!",FALSE,arago,0,vict,TO_NOTVICT);
      act ("$n hooks on to you with clawed hands and throws you headfirst at the portal!",FALSE,arago,0,vict,TO_VICT);
      act ("You throw foolish $N through the portal!",FALSE,arago,0,vict,TO_CHAR);
      damage(arago,vict,150,TYPE_UNDEFINED,DAM_NO_BLOCK);

      if(d->connected!=CON_PLYNG) return FALSE; /* If char died */

      if(vict->specials.riding) stop_riding(vict,vict->specials.riding);
      char_from_room (vict);
      char_to_room (vict, real_room (DM_PORTAL_INSIDE));
      do_look(vict,"",CMD_LOOK);
    }
    return FALSE;
  }
  if(cmd==MSG_DIE) {
      act("$n's loud scream of agony almost drowns out the sound\n\rof a crumbling wall.\n\r",FALSE,arago,0,0,TO_ROOM);
      world[real_room(DM_PORTAL_INSIDE)].dir_option[SOUTH]->to_room_r = real_room(NEW_ROOM);
      send_to_room ("The wall to the south crumbles!\n\r",real_room(DM_PORTAL_INSIDE));
      world[real_room(NEW_ROOM)].dir_option[NORTH]->to_room_r = real_room(DM_PORTAL_INSIDE);
      send_to_room ("The wall to the north crumbles as the wall to the east magically reappears!\n\r",real_room(NEW_ROOM));
      world[real_room(SCROLL_ROOM)].dir_option[WEST]->to_room_r = real_room(0);
      send_to_room ("The wall to the west magically magically reappears!\n\r",real_room(SCROLL_ROOM));
      world[real_room(NEW_ROOM)].dir_option[EAST]->to_room_r = real_room(0);

      /* Take his loaded stuff send it to the room */
      for (i=0; i < MAX_WEAR; i++) {
          if(arago->equipment[i]) {
            obj_to_room(unequip_char(arago,i),real_room(DM_PORTAL_INSIDE));
 	  }
      }
      return FALSE;
  }

  if(cmd==MSG_TICK) {
    if(world[real_room(NEW_ROOM)].dir_option[NORTH]->to_room_r == real_room(DM_PORTAL_INSIDE)) {
      world[real_room(NEW_ROOM)].dir_option[NORTH]->to_room_r = real_room(0);
      world[real_room(DM_PORTAL_INSIDE)].dir_option[SOUTH]->to_room_r = real_room(0);
    }
  }
  return FALSE;
}

int arago_door(int room, CHAR *ch, int cmd, char *arg) {

  if(cmd!=MSG_ZONE_RESET) return FALSE;

  world[real_room(SCROLL_ROOM)].dir_option[WEST]->to_room_r = real_room(0);
  send_to_room ("The wall to the west magically magically reappears!\n\r",real_room(SCROLL_ROOM));
  world[real_room(NEW_ROOM)].dir_option[EAST]->to_room_r = real_room(0);
  return FALSE;
}

int dm_hachiman(CHAR *mob,CHAR *ch, int cmd, char *arg) {
   CHAR *vict = NULL, *dragon = NULL, *next_vict = NULL;
   OBJ *obj = NULL;
   char buf[MAX_INPUT_LENGTH];

   switch (cmd) {
     case MSG_ENTER:
       if (!ch) return FALSE;
       if (mob->specials.fighting) return FALSE;
       if (!CAN_SEE(mob,ch)) return FALSE;

       act("$n senses $N's intent and quickly strikes first!",0,mob,0,ch,TO_NOTVICT);
       act("You sense $n's true intentions and attack!",0,mob,0,ch,TO_CHAR);
       act("$n peers deep into your soul and realizes your true intent!",0,mob,0,ch,TO_VICT);
       hit(mob, ch, TYPE_HIT);
       hit(mob, ch, TYPE_HIT);
       return TRUE;
       break;

     case CMD_OPEN:
       if (!ch) return FALSE;
       if (!*arg) return FALSE;
       one_argument (arg, buf);
       if (strcmp(buf,"trapdoor")) return FALSE;
       send_to_char ("I see no trapdoor here.\n\r",ch);
       return TRUE;
       break;

     case CMD_CLOSE:
       if (!ch) return FALSE;
       if (!*arg) return FALSE;
       one_argument (arg, buf);
       if (strcmp(buf,"trapdoor")) return FALSE;
       send_to_char ("I see no trapdoor here.\n\r",ch);
       return TRUE;
       break;

     case CMD_DOWN:
       if (!ch) return FALSE;
       send_to_char ("Alas, you cannot go that way...\n\r",ch);
       return TRUE;
       break;

     case MSG_DIE:
       if(CHAR_REAL_ROOM(mob)!=real_room(HACHI_ROOM)) return FALSE;
       ROOM_SPEC(real_room(HACHI_ROOM))=1;
       if(mob->equipment[WEAR_HEAD]) {
         obj=unequip_char(mob,WEAR_HEAD);
         act("A beautiful Wooden Helm flashes for a split second and vanishes.",0,mob,0,0,TO_ROOM);
         obj_to_room(obj,real_room(DRAGON_ROOM));
       }
       return FALSE;
       break;

     case CMD_ASSAULT:
       if (!ch) return FALSE;
       if(number(0,1)==0) {
         act("$n drives his gauntleted fist into $N's face angrily!",0,mob,0,ch,TO_NOTVICT);
         act("You catch foolish $N trying to punch you!",0,mob,0,ch,TO_CHAR);
         act("$n says, 'You fool, you can never catch me unaware!'",0,mob,0,ch,TO_VICT);
         act("You reel in pain as $n smashes you with his gauntleted hand!",0,mob,0,ch,TO_VICT);
         damage(mob,ch,250,TYPE_UNDEFINED,DAM_NO_BLOCK);
         return TRUE;
       }
       return FALSE;
       break;

     case CMD_FLEE:
       if (!ch) return FALSE;
       act("$n grabs $N by the neck as $E tries to flee and smashes $M hard against the wall!",0,mob,0,ch,TO_NOTVICT);
       act("You snag cowardly $N and violently smash $M against the wall!",0,mob,0,ch,TO_CHAR);
       act("$n says, 'COWARD!! Get back here and fight like a true warrior!",0,mob,0,ch,TO_VICT);
       act("$n roughly grabs you by the neck and the breath is knocked from you as he smashes you against the wall!",0,mob,0,ch,TO_VICT);
       return TRUE;
       break;

     case CMD_KILL:
       if (!ch) return FALSE;
       act("You use your superior powers to punish $N for $S attack.",0,mob,0,ch,TO_CHAR);
       act("$n angrily makes a crushing gesture with his right hand!",0,mob,0,ch,TO_VICT);
       act("You feel your heart being crushed from the inside!",0,mob,0,ch,TO_VICT);
       damage(mob,ch,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
       return TRUE;
       break;

     case CMD_PUNCH:
       if (!ch) return FALSE;
       act("Teeth fly from $N's mouth as $E is hit with a hard backhand from $n!",0,mob,0,ch,TO_NOTVICT);
       act("You knock most of $N's teeth out as you stop $S punch.",0,mob,0,ch,TO_CHAR);
       act("You find yourself toothless as $n smashes you in the face with a vicious backhand!",0,mob,0,ch,TO_VICT);
       damage(mob,ch,450,TYPE_UNDEFINED,DAM_NO_BLOCK);
       return TRUE;
       break;

     case CMD_CIRCLE:
       if (!ch) return FALSE;
       act("$n grabs $N's arm and drives $S weapon into $S own stomach!",0,mob,0,ch,TO_NOTVICT);
       act("You help foolish $N stab $Mself in the stomach.",0,mob,0,ch,TO_CHAR);
       act("$n aids you in disembowelling yourself- OUCH!!",0,mob,0,ch,TO_VICT);
       damage(mob,ch,350,TYPE_UNDEFINED,DAM_NO_BLOCK);
       return TRUE;
       break;

     case CMD_CAST:
       if(!ch) return FALSE;
       if (number(0,7)) return FALSE; /* If its not 0 it returns */
       act("$n reaches out toward $N, and a bolt of yellow energy is drawn away from $M.",0,mob,0,ch,TO_NOTVICT);
       act("You reach towards $N, and a stream of yellow energy flows to you.",0,mob,0,ch,TO_CHAR);
       act("$n utters the words, 'efaui waia'",0,mob,0,ch,TO_VICT);
       act("$n reaches out toward you and a stream of yellow energy leaps to his hand.",0,mob,0,ch,TO_VICT);
       GET_MANA(ch)=GET_MANA(ch)/2;
       return TRUE;
       break;

     case MSG_MOBACT:
       if(!mob->specials.fighting) return FALSE;

       /* Dragon exist? */
       if(!mob_proto_table[real_mobile(SKELETAL_DRAGON)].number) {
 	       dragon = read_mobile(SKELETAL_DRAGON,VIRTUAL);
	       char_to_room(dragon,real_room(DRAGON_ROOM));
       }

       if(!number(0,7)) {
         act("$n glows bright red in anger and begins chanting!",
              1, mob, 0, 0, TO_ROOM);
         act("You chant an arcane mantra to slay these foolish nondeities.",
              1, mob, 0, 0, TO_CHAR);

         for(vict = world[mob->in_room_r].people; vict;vict = next_vict) {
           next_vict = vict->next_in_room;
           if(vict !=mob && !IS_NPC(vict) && GET_LEVEL(vict)<LEVEL_IMM) {
             act("Large pieces of bone strike you as $n causes the room to shake violently!",
                  1, mob, 0, vict, TO_VICT);
             damage(mob, vict, 350, TYPE_UNDEFINED,DAM_NO_BLOCK);
           }
         }
         act("You grow tired of toying with these foolish mortals.",
              1, mob, 0, vict, TO_CHAR);
         return FALSE;
       }

       /* Transport to dragon */

       if(!number(0,10)) {
         if(!(vict=get_random_victim(mob))) return FALSE;
         if(vict==mob->specials.fighting) return FALSE;

         if(vict->specials.riding) stop_riding(vict,vict->specials.riding);
         act("$n makes a strange gesture.",0,mob,0,0,TO_ROOM);
         act("You makes a strange gesture.",0,mob,0,0,TO_CHAR);
         act("$n suddenly vanishes!",0,vict,0,0,TO_ROOM);
         act("You are suddenly transported",0,vict,0,0,TO_CHAR);

         char_from_room(vict);
         char_to_room(vict,real_room(DRAGON_ROOM));
         do_look(vict,"",CMD_LOOK);
         act("$n appears.", FALSE, vict, 0, 0, TO_ROOM);
       }
       break;
   }
   return FALSE;
}

int dm_hachi_room(int room,CHAR *ch, int cmd,char *arg) {

   if(cmd!=MSG_MOBACT) return FALSE;
   if(!ROOM_SPEC(room)) return FALSE;
   ROOM_SPEC(room)=0;
   send_to_room("\n\rA mound of bones near the center of the room shudders slightly, revealing something in the floor.\n\r",room);
   return FALSE;
}

int dm_crone(CHAR *mob,CHAR *ch, int cmd, char *arg) {
   CHAR *vict,*next_vict;

   if(cmd!=MSG_DIE) return FALSE;

   act("You let out a deafening wail with your dying breath.",0,mob,0,0,TO_CHAR);
   act("$n lets out a deafening wail with $s dying breath!",0,mob,0,0,TO_ROOM);

   for(vict = world[mob->in_room_r].people; vict;vict = next_vict) {
     next_vict = vict->next_in_room;
     if(vict !=mob && !IS_NPC(vict)&& GET_LEVEL(vict)<LEVEL_IMM) {
       act("That really HURT!",1,mob,0,vict,TO_VICT);
       damage(mob, vict, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
     }
   }
   return FALSE;
}

int dm_dragon(CHAR *mob,CHAR *ch, int cmd, char *arg) {
   OBJ *obj,*next_obj,*temp;

   if(cmd!=MSG_DIE) return FALSE;
   if(CHAR_REAL_ROOM(mob)!=real_room(DRAGON_ROOM)) return FALSE;
   ROOM_SPEC(real_room(DRAGON_ROOM))=1;
   for(obj=world[CHAR_REAL_ROOM(mob)].contents;obj;obj=next_obj) {
     next_obj=obj->next_content;
     if(V_OBJ(obj)==20137) {
       act("The Wooden Helm glows briefly.",0,mob,0,0,TO_ROOM);
       extract_obj(obj);
       temp=read_object(JADE_HELM,VIRTUAL);
       if(temp) obj_to_room(temp,CHAR_REAL_ROOM(mob));
     }
   }
   return FALSE;
}

int dm_dragon_room(int room,CHAR *ch, int cmd,char *arg) {
   CHAR *vict,*next_vict;
   OBJ *obj,*next_obj;

   if(cmd!=MSG_MOBACT) return FALSE;
   if(!ROOM_SPEC(room)) return FALSE;
   ROOM_SPEC(room)=0;
   send_to_room("\n\rThe fabric of reality holding the room together collapses.\n\r",room);

   for(vict = world[room].people; vict;vict = next_vict) {
     next_vict = vict->next_in_room;
     char_from_room(vict);
     char_to_room(vict,real_room(HACHI_START_ROOM));
     act("You fall through the air and hit the ground HARD!",1,vict,0,0,TO_CHAR);
     if(GET_LEVEL(vict)<LEVEL_IMM)
       damage(vict, vict,GET_MAX_HIT(vict)/4, TYPE_UNDEFINED,DAM_NO_BLOCK);
   }

   for(obj=world[room].contents;obj;obj=next_obj) {
     next_obj=obj->next_content;
     obj_from_room(obj);
     obj_to_room(obj,real_room(HACHI_START_ROOM));
   }

   return FALSE;
}

int dm_battlesuit(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  OBJ *tmp;

  if(!ch) return FALSE;
  if(cmd==MSG_OBJ_WORN) {
    if (ch->equipment[WEAR_BODY]) {
      send_to_char("You already wear something on your body.\n\r",ch);
      return TRUE;
    }

    if (ch->equipment[WEAR_ARMS]) {
      send_to_char("You already wear something on your arms.\n\r",ch);
      return TRUE;
    }

    if (ch->equipment[WEAR_HANDS]) {
      send_to_char("You already wear something on your hands.\n\r",ch);
      return TRUE;
    }

    if(GET_CLASS(ch)!=CLASS_NOMAD) {
      act("Green flame leaps out of $p burning $n!",1,ch,obj,0,TO_ROOM);
      act("Green flame leaps out of $p burning you!",1,ch,obj,0,TO_CHAR);
      damage(ch,ch,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return TRUE;
    }

    act("$n wears $p on $s body.",1,ch,obj,0,TO_ROOM);
    act("You wear $p on your body.",1,ch,obj,0,TO_CHAR);
    extract_obj(obj);
    tmp = read_object (BRACERS, VIRTUAL);
    equip_char(ch,tmp,WEAR_ARMS);
    tmp = read_object (GAUNTLETS, VIRTUAL);
    equip_char(ch,tmp,WEAR_HANDS);
    tmp = read_object (BREASTPLATE, VIRTUAL);
    equip_char(ch,tmp,WEAR_BODY);
    return TRUE;
  }
  return FALSE;
}

int dm_battlesuit2(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  OBJ *tmp;
  char buf[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH];
  int i;

  if(cmd==MSG_TICK && !obj->equipped_by) {
    extract_obj(obj);
  }

  if(cmd==CMD_WEAR && !obj->equipped_by) return TRUE;

  if(cmd==MSG_DIE || cmd==MSG_STONE) {
    if(V_OBJ(obj)!=GAUNTLETS) return FALSE;
    ch=obj->equipped_by;
    if(!ch) return FALSE;
    tmp=ch->equipment[WEAR_SHIELD];
    if(tmp && V_OBJ(tmp)==2720) return FALSE; /* Bone shield */
    tmp=ch->equipment[WEAR_HEAD];
    if(tmp && V_OBJ(tmp)==6973) return FALSE; /* Tarion Helmet */
    tmp=ch->equipment[WEAR_NECK_1];
    if(tmp && V_OBJ(tmp)==28600) return FALSE; /* Wooden Crucifix */
    tmp=ch->equipment[WEAR_NECK_2];
    if(tmp && V_OBJ(tmp)==28600) return FALSE; /* Wooden Crucifix */
    tmp=ch->equipment[WEAR_HANDS];
    if(tmp && obj==tmp) { /* Let Gauntlets be the master remove */
      extract_obj(unequip_char(ch,WEAR_HANDS));
      tmp=ch->equipment[WEAR_ARMS];
      if(tmp && V_OBJ(tmp)==BRACERS) extract_obj(unequip_char(ch,WEAR_ARMS));
      tmp=ch->equipment[WEAR_BODY];
      if(tmp && V_OBJ(tmp)==BREASTPLATE) extract_obj(unequip_char(ch,WEAR_BODY));
      tmp=read_object(BATTLESUIT,VIRTUAL);
      obj_to_char(tmp,ch);
    }
  }

  if(cmd==CMD_REMOVE) {
    if(!ch) return FALSE;
    one_argument(arg,buf);
    if (!*buf) return FALSE;

    if(!obj->equipped_by) return FALSE;
    if(ch!=obj->equipped_by) return FALSE;
    if (isname(buf, OBJ_NAME(obj))) {
      act("You stop using A Bright Jade Battlesuit.",0,ch,0,0,TO_CHAR);
      act("$n stops using A Bright Jade Battlesuit.",0,ch,0,0,TO_ROOM);
      tmp=ch->equipment[WEAR_HANDS];
      if(tmp && V_OBJ(tmp)==GAUNTLETS) extract_obj(unequip_char(ch,WEAR_HANDS));
      tmp=ch->equipment[WEAR_ARMS];
      if(tmp && V_OBJ(tmp)==BRACERS) extract_obj(unequip_char(ch,WEAR_ARMS));
      tmp=ch->equipment[WEAR_BODY];
      if(tmp && V_OBJ(tmp)==BREASTPLATE) extract_obj(unequip_char(ch,WEAR_BODY));
      tmp=read_object(BATTLESUIT,VIRTUAL);
      obj_to_char(tmp,ch);
      return TRUE;
    }

    if (!strcmp(buf,"all")) {
      tmp=ch->equipment[WEAR_HANDS];
      if(tmp && V_OBJ(tmp)==GAUNTLETS) {
        for (i=0; i <= WEAR_HANDS; i++) {
          tmp = ch->equipment[i];
          if (tmp) {
            sprintf(buf2,"remove %s",OBJ_NAME(tmp));
            command_interpreter(ch,buf2);
          }
        }
      }
    }
  }
  return FALSE;
}

int dm_two_doors(int room, CHAR *ch, int cmd, char *arg) {
  int wear_ring=0;
  char buf[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH];

  if (!ch) return FALSE;

  arg=one_argument(arg,buf);
  if (!*buf) return FALSE;

  if ((cmd==CMD_UNLOCK || cmd==CMD_OPEN || cmd==CMD_MOVE || cmd==CMD_CLOSE || cmd==CMD_LOCK)
      && (!strcmp(buf,"wall") || !strcmp(buf,"floorboard"))) {
    send_to_char("You cannot do that.\n\r",ch);
    return TRUE;
  }

  if (cmd!=CMD_USE) return FALSE;
  one_argument(arg,buf2);

  if (!strcmp(buf,"prybar")) {
    if(!ch->equipment[HOLD]) {
      send_to_char("You do not hold that item.\n\r",ch);
      return TRUE;
    }
    if(V_OBJ(ch->equipment[HOLD])!=PRYBAR) {
      send_to_char("You do not hold that item.\n\r",ch);
      return TRUE;
    }
    if(buf2[0] == '\0') {
      send_to_char("Use it where?\n\r",ch);
      return TRUE;
    }
    if(!strcmp(buf2,"floorboard")) {
      act("As $n uses the metal prybar to lift the floorboard,\n\rit breaks in $s hands.",0,ch,0,0,TO_ROOM);
      act("As you use the metal prybar to lift the floorboard,\n\rit breaks in your hand.",0,ch,0,0,TO_CHAR);
      extract_obj(unequip_char(ch,HOLD));
     	REMOVE_BIT(world[room].dir_option[DOWN]->exit_info, EX_LOCKED);
     	REMOVE_BIT(world[room].dir_option[DOWN]->exit_info, EX_CLOSED);
     	REMOVE_BIT(world[real_room(ROOM_DOWN)].dir_option[UP]->exit_info, EX_LOCKED);
     	REMOVE_BIT(world[real_room(ROOM_DOWN)].dir_option[UP]->exit_info, EX_CLOSED);
      send_to_room("The ceiling is opened from above.\n\r",real_room(ROOM_DOWN));
     	return TRUE;
    }
    else {
      send_to_char("Use it where?\n\r",ch);
      return TRUE;
    }
  }

  if (!strcmp(buf,"ring") || !strcmp(buf,"signet")) {
    if(ch->equipment[WEAR_FINGER_R] && V_OBJ(ch->equipment[WEAR_FINGER_R])==SIGNET_RING)
      wear_ring=1;
    if(ch->equipment[WEAR_FINGER_L] && V_OBJ(ch->equipment[WEAR_FINGER_L])==SIGNET_RING)
      wear_ring=2;

    if(!wear_ring) {
      send_to_char("You are not wearing the ring.\n\r",ch);
      return TRUE;
    }
    if(!*buf2) {
      send_to_char("Use it where?\n\r",ch);
      return TRUE;
    }
    if(!strcmp(buf2,"wall")) {
      act("As $n presses the ring against the north wall,\n\rboth the ring and wall disappear.",0,ch,0,0,TO_ROOM);
      act("As you press the ring against the north wall,\n\rboth it and the wall disappear.",0,ch,0,0,TO_CHAR);
      if(wear_ring==1) extract_obj(unequip_char(ch,WEAR_FINGER_R));
      if(wear_ring==2) extract_obj(unequip_char(ch,WEAR_FINGER_L));
     	REMOVE_BIT(world[room].dir_option[NORTH]->exit_info, EX_CLOSED);
     	REMOVE_BIT(world[real_room(ROOM_NORTH)].dir_option[SOUTH]->exit_info, EX_CLOSED);
      send_to_room("The south wall disappears.\n\r",real_room(ROOM_NORTH));
     	return TRUE;
    }
    else {
      send_to_char("Use it where?\n\r",ch);
      return TRUE;
    }
  }
  return FALSE;
}

int dm_exit_block(int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if (!ch) return FALSE;

  arg=one_argument(arg,buf);
  if (!*buf) return FALSE;

  if ((cmd==CMD_UNLOCK || cmd==CMD_OPEN || cmd==CMD_MOVE || cmd==CMD_CLOSE || cmd==CMD_LOCK)
      && (!strcmp(buf,"wall") || !strcmp(buf,"floorboard"))) {
    send_to_char("You cannot do that.\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int dm_gong(OBJ *obj, CHAR *tmp, int cmd, char *arg) {
  CHAR *ch;
  if (cmd != MSG_TICK) return (FALSE);
  if (!obj->equipped_by) return (FALSE);
  if(obj->obj_flags.value[3]>0) obj->obj_flags.value[3]--;
  ch=obj->equipped_by;
  if (!ch->specials.fighting) return (FALSE);
  if (obj != EQ(ch,HOLD)) return (FALSE);
  if (!number(0,29)) return (FALSE);

  if (IS_NEUTRAL(ch)) return (FALSE);
  if (IS_NEUTRAL(ch->specials.fighting)) return (FALSE);
  if (IS_EVIL(ch) && IS_EVIL(ch->specials.fighting)) return (FALSE);
  if (IS_GOOD(ch) && IS_GOOD(ch->specials.fighting)) return (FALSE);

  if(affected_by_spell(ch,SPELL_FURY)) return (FALSE);

  if(!obj->obj_flags.value[3]) {
    act("Your $p glows briefly red, filling you with righteous anger.",FALSE,ch,obj,0,TO_CHAR);
    act("$p held by $n glows briefly red.",FALSE,ch,obj,0,TO_ROOM);
    spell_haste(LEVEL_IMM-1,ch,ch,SPELL_TYPE_SPELL);
    obj->obj_flags.value[3]=30;
    return (FALSE);
  }
  return (FALSE);
}

int dm_thorn(OBJ *obj,CHAR *ch, int cmd, char *argument) {
  CHAR *vict;

  if(cmd != MSG_MOBACT) return FALSE;

  vict=obj->equipped_by;
  if(!vict) return FALSE;
  if(!vict->specials.fighting) return FALSE;
  if(chance(94)) return FALSE;
  if(CHAR_REAL_ROOM(vict)!=CHAR_REAL_ROOM(vict->specials.fighting)) return FALSE;
  act("$n's Thorn draws energy forth from $N.",FALSE,vict,obj,vict->specials.fighting,TO_ROOM);
  act("Your Thorn draws energy forth from $N.",FALSE,vict,obj,vict->specials.fighting,TO_CHAR);
  spell_mana_transfer(GET_LEVEL(vict->specials.fighting), vict->specials.fighting, vict, 0);
  return FALSE;
}

void assign_daimyo(void) {
  assign_mob(TONASHI_WRESTLER, tonashi_wrestler);
  assign_mob(SHOGUN_WARLORD,   warlord);
  assign_mob(CRONE,            dm_crone);
  assign_mob(SKELETAL_DRAGON,  dm_dragon);
  assign_obj(BINDING_SCROLL,   binding_scroll);
  assign_obj(DM_PORTAL,        dm_portal);
  assign_obj(HACHIMAN_STATUE,  hachi_statue);
  assign_mob(BLACK_PANTHER,    black_panther);
  assign_obj(BONE_WAND,        bone_wand);
  assign_mob(ARAGO,            dm_arago);
  assign_mob(HACHIMAN,         dm_hachiman);
  assign_obj(BATTLESUIT,       dm_battlesuit);
  assign_obj(GAUNTLETS,        dm_battlesuit2);
  assign_obj(BRACERS,          dm_battlesuit2);
  assign_obj(BREASTPLATE,      dm_battlesuit2);
  assign_obj(GONG,             dm_gong);
  assign_obj(JADE_HELM,        dm_jade_helm);
  assign_obj(THORN,            dm_thorn);
  assign_obj(THORN_THOTH_VANITY, dm_thorn);
  assign_room(DM_PORTAL_INSIDE,dm_portal_inside);
  assign_room(DOOR_ROOM,       dm_two_doors);
  assign_room(ROOM_NORTH,      dm_exit_block);
  assign_room(ROOM_DOWN,       dm_exit_block);
  assign_room(NEW_ROOM,        arago_door);
  assign_room(HACHI_ROOM,      dm_hachi_room);
  assign_room(DRAGON_ROOM,     dm_dragon_room);
}


