/**************************************************************************
 *  file: spec.stable.c , Special module.                 Part of DIKUMUD *
 *  Programed by: R. Rogers (Ranger)                                      *
 *                                                                        *
 *                                                                        *
 *  Special thanks to Quack for doing virtually all Stable room           *
 *  descriptions, and turning a one room - buy a horse idea into the      *
 *  multi-mount place it is today.                                        *
 *                                                                        *
 **************************************************************************/

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
#include "reception.h"

#define ZONE_STABLE  3900
#define STABLE_BOY         ITEM(ZONE_STABLE, 1)
#define STABLE_HUNCHBACK   ITEM(ZONE_STABLE,17)
#define STABLE_MASTER      ITEM(ZONE_STABLE,18)
#define STABLE_CDRAGON     ITEM(ZONE_STABLE,19)
#define STABLE_MULE        ITEM(ZONE_STABLE, 2)
#define STABLE_HORSE       ITEM(ZONE_STABLE, 3)
#define STABLE_WHORSE      ITEM(ZONE_STABLE, 4)
#define STABLE_CAMEL       ITEM(ZONE_STABLE, 5)
#define STABLE_ELEPHANT    ITEM(ZONE_STABLE, 6)
#define STABLE_BAT         ITEM(ZONE_STABLE, 7)
#define STABLE_GRIFFON     ITEM(ZONE_STABLE, 8)
#define STABLE_EAGLE       ITEM(ZONE_STABLE, 9)
#define STABLE_WYVERN      ITEM(ZONE_STABLE,10)
#define STABLE_DRAGON      ITEM(ZONE_STABLE,11)
#define STABLE_SNAIL       ITEM(ZONE_STABLE,12)
#define STABLE_SCORPION    ITEM(ZONE_STABLE,13)
#define STABLE_TARANTULA   ITEM(ZONE_STABLE,14)
#define STABLE_LIZARD      ITEM(ZONE_STABLE,15)
#define STABLE_GOLEM       ITEM(ZONE_STABLE,16)

#define STABLE_PLATFORM    ITEM(ZONE_STABLE, 7)
#define STABLE_FLYOPLAT    ITEM(ZONE_STABLE,33)
#define STABLE_FLYOM       ITEM(ZONE_STABLE, 8)
#define STABLE_HANDOR      ITEM(ZONE_STABLE,25)
#define STABLE_SONIC       ITEM(ZONE_STABLE,26)
#define STABLE_ZYCA        ITEM(ZONE_STABLE,27)
#define STABLE_FOREST      ITEM(ZONE_STABLE,28)
#define STABLE_ZEKIAN      ITEM(ZONE_STABLE,29)
#define STABLE_CORN        ITEM(ZONE_STABLE,30)

/* Data declarations */

struct social_type {
  char *cmd;
  int next_line;
};

int stable_boy(CHAR *stboy, CHAR *ch, int cmd, char *arg) {

  char buf[MAX_STRING_LENGTH], mount_name[MIL],tmp_pwd[11];
  struct obj_data *obj;
  struct follow_type *k;
  int stall_room, this_room, object;
  bool has_mount;
  CHAR *mount;

  if((GET_POS(stboy)==POSITION_STANDING) && (!cmd)) {
    if(number(0,9)==0) {
      act("$n says 'I have many types of mounts to serve you.'",FALSE,stboy,0,0,TO_ROOM);
      return(FALSE);
    }
  }

  if(!ch) return(FALSE);

  this_room = CHAR_REAL_ROOM(ch);
  stall_room = this_room+1;

  if( (cmd!=CMD_LIST) && (cmd!=CMD_BUY) ) return(FALSE);

  if (!AWAKE(stboy)) {
    act("$n is asleep... shhh.", FALSE, stboy, 0, ch, TO_VICT);
    return(TRUE);
  }
  if (!CAN_SEE(stboy, ch)) {
     act("$n says, 'Sorry, I don't deal with invisibles!'",
          FALSE, stboy, 0, 0, TO_ROOM);
     return(TRUE);
  }
  if ((this_room != real_room(3901)) && (this_room != real_room(3903)) &&
        (this_room != real_room(3905))) {
     act("$n tells you 'I'm sorry, I seem to have wandered from my stall.'",
          FALSE, stboy, 0, ch, TO_VICT);
     return(TRUE);
  }

  if (cmd==CMD_LIST) {
     act("$n tells you 'Available mounts are...'", FALSE, stboy, 0, ch, TO_VICT);
     for(mount = world[stall_room].people; mount; mount = mount->next_in_room) {
        sprintf(buf, "%8d - Lvl %2d - %s\n\r", 3*GET_EXP(mount), GET_LEVEL(mount), MOB_SHORT(mount));
        send_to_char(buf, ch);
     }
     return(TRUE);
  }
  else if (cmd==CMD_BUY) {

     arg = one_argument(arg, buf);
     arg = one_argument(arg, mount_name);

     if (!(mount = get_char_room(buf, stall_room))) {
        act("$n tells you 'I have no such creature!'", FALSE, stboy, 0, ch, TO_VICT);
        return(TRUE);
     }

     has_mount = FALSE;
     for (k=ch->followers; k; k= k->next) {
        if (IS_NPC(k->follower) && (k->follower->master==ch) &&
           IS_SET(k->follower->specials.act, ACT_MOUNT)) has_mount = TRUE;
     }

     if (has_mount) {
        act("$n tells you 'You already have one mount!'", FALSE, stboy, 0, ch, TO_VICT);
        return(TRUE);
     }

     if((GET_LEVEL(ch)+3)<GET_LEVEL(mount)) {
        act("$n tells you 'Your level isn't high enough for that creature.'",FALSE, stboy, 0, ch, TO_VICT);
        return(TRUE);
     }

     if (!*mount_name) {
        act("$n tells you 'Please supply a name for your mount'",FALSE, stboy, 0, ch, TO_VICT);
        return(TRUE);
     }

     if(test_char(mount_name, tmp_pwd)) {
       act("$n tells you 'Please use a different name for your mount'",FALSE, stboy, 0, ch, TO_VICT);
       return(TRUE);
     }

     if (GET_GOLD(ch) < (GET_EXP(mount)*3)) {
        act("$n tells you 'You need more gold!'", FALSE, stboy, 0, ch, TO_VICT);
        return(TRUE);
     }
     GET_GOLD(ch) -= GET_EXP(mount)*3;

     mount = read_mobile(mount->nr, REAL);
     GET_EXP(mount) = 0;
     SET_BIT(mount->specials.affected_by, AFF_CHARM);

     if (this_room==real_room(3903)) REMOVE_BIT(mount->specials.affected_by,AFF_FLY);

     /* Bridle for neck */
     object=real_object(3900);
     obj=read_object(object, REAL);
     equip_char(mount, obj, 3);

     /* Saddle for body */
     object=real_object(3904);
     obj=read_object(object, REAL);
     equip_char(mount, obj, 5);
     if (*mount_name) {
        sprintf(buf,"%s %s", MOB_NAME(mount),mount_name);
        free(mount->player.name);
        mount->player.name = str_dup(buf);

        sprintf(buf, "%sA tag on the collar says 'My Name is %s'.\n\r",
               MOB_DESCRIPTION(mount),CAP(mount_name));
        free(mount->player.description);
        mount->player.description = str_dup(buf);
     }

     act("$n tells you 'Please take care of your mount.'", FALSE, stboy, 0, ch, TO_VICT);
     act("$n bought $N as a mount.",TRUE,ch,0,mount,TO_ROOM);

     char_to_room(mount, this_room);
     add_follower(mount, ch);

     return(TRUE);
  }

  return(FALSE);
}

int platform(int room, CHAR *ch, int cmd, char *arg) {

  int this_room;
  bool flyer, flying;

  if (!ch) return(FALSE);
  if (cmd != CMD_UP) return(FALSE);
  if (GET_LEVEL(ch)>=LEVEL_IMM) return(FALSE);

  if (IS_NPC(ch)) {
     if(GET_POS(ch)==POSITION_FLYING) return(FALSE);
     if(GET_POS(ch)!=POSITION_RIDING) return(TRUE);
  }

  if (GET_POS(ch) != POSITION_RIDING) {
     send_to_char("You can't fly over Midgaard without riding a flyer!\n\r", ch);
     return(TRUE);
  }
  flyer = FALSE;
  flying = FALSE;

  this_room= CHAR_REAL_ROOM(ch);
  if(ch->specials.riding) {
     if (this_room == CHAR_REAL_ROOM(ch->specials.riding) &&
         IS_SET(ch->specials.riding->specials.act, ACT_FLY)) {
           flyer = TRUE;
           if (GET_POS(ch->specials.riding)==POSITION_FLYING) flying = TRUE;
     }
  }

  if(!flyer) {
    send_to_char("Your mount can not fly!\n\r",ch);
    return(TRUE);
  }

  if(!flying) {
    send_to_char("Your mount is not flying!\n\r",ch);
    return(TRUE);
  }

  return(FALSE);
}


int flying_air(int room, CHAR *ch, int cmd, char *arg) {

  int fall_room;
  char tmp[80];
  bool fall, flyer, flying, riding, fighting;
  CHAR *mount=0,*temp;

  if(cmd) return(FALSE);

  fall=FALSE;
  fall_room=0;
  if(room==real_room(3908)) fall_room=real_room(3014); /* To mksquare */
  if(room==real_room(3933)) fall_room=real_room(3907); /* To Platform */
  if(room==real_room(3925)) fall_room=real_room(6100); /* To forest Han-Dor */
  if(room==real_room(3926)) fall_room=real_room(8011); /* Sonicity */
  if(room==real_room(3927)) fall_room=real_room(10916); /* Zyca */
  if(room==real_room(3928)) fall_room=real_room(4801); /* Ent Half Village */
  if(room==real_room(3929)) fall_room=real_room(5609); /* Zekian */
  if(room==real_room(3930)) fall_room=real_room(1955); /* Cornelis */

  if(fall_room==0) {
    log_f("Invalid Fall Room called from stable spec");
    return FALSE;
  }

  for (ch=world[room].people;ch;ch=temp) {
     temp = ch->next_in_room;  /* Added temp - Ranger June 96 */
     if (!ch) continue;
     if ( (GET_LEVEL(ch)>=LEVEL_IMM) || IS_NPC(ch) ) continue;

     flyer = FALSE;
     flying = FALSE;
     riding = FALSE;
     fighting = FALSE;

     sprintf(tmp, "$n drops down to %s.", world[fall_room].name);

     if(ch->specials.riding) {
       if(CHAR_REAL_ROOM(ch)==CHAR_REAL_ROOM(ch->specials.riding)) {
         riding=TRUE;
         mount=ch->specials.riding;
         if(IS_SET(mount->specials.act,ACT_FLY)) flyer=TRUE;
         if(GET_POS(mount)==POSITION_FLYING) flying=TRUE;
         if(GET_POS(mount)==POSITION_FIGHTING) fighting=TRUE;
       }
     }

     if ((flyer) && (!riding) && (!fighting)) {
        send_to_char("You realize you're not riding your mount and begin to fall, but\n\r",ch);
        send_to_char("just before you hit the ground, your faithful mount swoops down\n\r",ch);
        send_to_char("and saves you.\n\r",ch);
        act(tmp, TRUE, ch, 0,0, TO_ROOM);
        char_from_room(ch);
        char_from_room(mount);
         /* No damage */
        GET_MOVE(ch)=1;
        char_to_room(ch, fall_room);
        char_to_room(mount, fall_room);
        act("$n falls from the sky.", TRUE, ch, 0,0, TO_ROOM);
        do_look(ch, "\0",15);
        fall=TRUE;
        continue;
     }

     if ((flyer) && (!riding) && (fighting)) {
        send_to_char("You realize your not riding your flyer and begin to fall. \n\r",ch);
        send_to_char("Unfortunately, your mount is too busy to save you.\n\r",ch);
        act(tmp, TRUE, ch, 0,0, TO_ROOM);
        char_from_room(ch);
        /* 1/2 damage */
        GET_HIT(ch)=GET_HIT(ch)/2;
        GET_MOVE(ch)=1;
        GET_MANA(ch)=GET_MANA(ch)/2;
        char_to_room(ch, fall_room);
        act("$n falls from the sky.", TRUE, ch, 0,0, TO_ROOM);
        do_look(ch, "\0",15);
        fall=TRUE;
        continue;
     }

     if ((flyer) && (riding) && (!flying) && (!fighting)) {
        send_to_char("You realize your flyer is not flying and you both drop out of the sky.\n\r",ch);
        act(tmp, TRUE, ch, 0,0, TO_ROOM);
        char_from_room(ch);
        char_from_room(mount);
        /* 1/4 damage */
        GET_HIT(ch)=GET_HIT(ch)*3/4;
        GET_MOVE(ch)=1;
        GET_MANA(ch)=GET_MANA(ch)*3/4;
        char_to_room(ch, fall_room);
        char_to_room(mount, fall_room);
        act("$n falls from the sky.", TRUE, ch, 0,0, TO_ROOM);
        do_look(ch, "\0",15);
        fall=TRUE;
        continue;
     }

     if (!flyer) {
        send_to_char("You realize you have no way to fly here and fall to the ground.\n\r",ch);
        act(tmp, TRUE, ch, 0,0, TO_ROOM);
        if (ch->specials.fighting) {
        	stop_fighting(ch->specials.fighting);
        	stop_fighting(ch);
        }
        char_from_room(ch);
        /* 3/4 damage */
        GET_HIT(ch)=GET_HIT(ch)/4;
        GET_MOVE(ch)=1;
        GET_MANA(ch)=GET_MANA(ch)/4;
        char_to_room(ch, fall_room);
        act("$n falls from the sky.", TRUE, ch, 0,0, TO_ROOM);
        do_look(ch, "\0",15);
        fall=TRUE;
        continue;
     }
     if (fall && ch->specials.fighting) {
        if (ch->specials.fighting->specials.fighting == ch) stop_fighting(ch->specials.fighting);
        stop_fighting(ch);
     }
  }

  return(fall);
}

int fly_walk(CHAR *flyer, CHAR *ch, int cmd, char *arg) {

  int m_check(CHAR *mount, CHAR *ch, int cmd, char *arg);
  CHAR *owner = 0;
  int this_room, char_room;
  if (cmd) return FALSE;

  m_check(flyer,ch,cmd,arg);

  owner=flyer->master;
  if(!owner) return(FALSE);

  this_room = CHAR_REAL_ROOM(flyer);
  char_room = CHAR_REAL_ROOM(owner);

  if (IS_OUTSIDE(flyer) && (weather_info.sky>SKY_RAINING) && (GET_POS(flyer)==POSITION_FLYING)) {
     if (this_room==char_room)
        act("$n refuses to fly in this weather.", FALSE, flyer, 0, owner, TO_VICT);
     GET_POS(flyer)=POSITION_STANDING;
     REMOVE_BIT(flyer->specials.affected_by, AFF_FLY);
     return(FALSE);
  }

  if (IS_OUTSIDE(flyer) && (weather_info.sky<=SKY_RAINING) && (GET_POS(flyer)==POSITION_STANDING)
      && !IS_SET(world[CHAR_REAL_ROOM(flyer)].room_flags, TUNNEL) ) {
     if (this_room==char_room)
        act("$n opens its wings and begins to fly.", FALSE, flyer, 0, owner, TO_VICT);
     GET_POS(flyer)=POSITION_FLYING;
     SET_BIT(flyer->specials.affected_by, AFF_FLY);
     return(FALSE);
  }

  if ((!IS_OUTSIDE(flyer)||IS_SET(world[CHAR_REAL_ROOM(flyer)].room_flags, TUNNEL))
      && (GET_POS(flyer)==POSITION_FLYING)) {
     if (this_room==char_room)
        act("$n can't fly in enclosed spaces.", FALSE, flyer, 0, owner, TO_VICT);
     GET_POS(flyer)=POSITION_STANDING;
     REMOVE_BIT(flyer->specials.affected_by, AFF_FLY);
     return(FALSE);
  }

  return(FALSE);
}

int m_check(CHAR *mount, CHAR *ch, int cmd, char *arg) {
  CHAR *owner = 0;
  struct obj_data *obj;
  int carrying_weight;
  int mob_num;
  int n=0;
  bool heavy;
  char buf[MAX_INPUT_LENGTH];

  void remove_item(struct char_data *ch, struct obj_data *obj, int pos);
  void stop_riding(struct char_data *ch,struct char_data *vict);

  if (cmd!=MSG_MOBACT) return(FALSE);

  if (CHAR_REAL_ROOM(mount)==real_room(3902)) return(FALSE);
  if (CHAR_REAL_ROOM(mount)==real_room(3904)) return(FALSE);
  if (CHAR_REAL_ROOM(mount)==real_room(3906)) return(FALSE);

  owner=mount->master;
  if (GET_POS(mount)==POSITION_FIGHTING) return(FALSE);
  if (!owner) return(FALSE);

  carrying_weight = IS_CARRYING_W(owner);
  mob_num = V_MOB(mount);

  for (n=0;n<MAX_WEAR;n++) {
     obj=mount->equipment[n];
     if ( (obj) && (V_OBJ(obj)!=3900) && (V_OBJ(obj)!=3904))
        remove_item(mount, obj, n);
  }

  /* Autoassist */
  if(owner->specials.fighting &&
     (CHAR_REAL_ROOM(mount)==CHAR_REAL_ROOM(owner)) &&
     !mount->specials.fighting) {
    sprintf(buf,"%s",GET_NAME(owner));
    do_assist(mount,buf,CMD_ASSIST);
  }


  if (!mount->specials.rider) return(FALSE);

  if (CHAR_REAL_ROOM(mount)!=CHAR_REAL_ROOM(mount->specials.rider)) {
    stop_riding(mount->specials.rider,mount);
    return FALSE;
  }

  heavy=FALSE;

  if ( carrying_weight>180 && (mob_num==STABLE_BAT || mob_num==STABLE_GRIFFON ))
     heavy=TRUE;

  if ( carrying_weight>210 && (mob_num==STABLE_EAGLE || mob_num==STABLE_SNAIL ))
     heavy=TRUE;

  if ( carrying_weight>230 && (mob_num==STABLE_WYVERN || mob_num==STABLE_MULE ||
     mob_num==STABLE_SCORPION )) heavy=TRUE;

  if ( carrying_weight>260 && (mob_num==STABLE_DRAGON || mob_num==STABLE_TARANTULA ))
     heavy=TRUE;

  if ( carrying_weight>290 && (mob_num==STABLE_HORSE || mob_num==STABLE_LIZARD ))
     heavy=TRUE;

  if ( carrying_weight>320 && (mob_num==STABLE_WHORSE)) heavy=TRUE;

  if ( carrying_weight>350 && (mob_num==STABLE_CAMEL)) heavy=TRUE;

  if (heavy) {
     act("$n refuses to carry such a heavy burden!", FALSE, mount, 0, owner, TO_VICT);
     stop_riding(mount->specials.rider,mount);
     return FALSE;
  }

  if (IS_GOOD(owner) && IS_EVIL(mount) ) {
     act("$n refuses to carry your holyness!", FALSE, mount, 0, owner, TO_VICT);
     stop_riding(mount->specials.rider,mount);
     return FALSE;
  }

  if (IS_GOOD(mount) && IS_EVIL(owner) ) {
     act("$n refuses to carry your evilness!", FALSE, mount, 0, owner, TO_VICT);
     stop_riding(mount->specials.rider,mount);
     return FALSE;
  }
  return(FALSE);
}

int cel_dragon(CHAR *dragon, CHAR *ch, int cmd, char *arg) {

  CHAR *vict,*temp;

  if (cmd || !AWAKE(dragon)) return(FALSE);

  if (GET_POS(dragon) != POSITION_FIGHTING) return(FALSE);

  if (number(0,2) != 1) return(FALSE);

  act("$n breathes a dangerous stream of fire.", FALSE, dragon, 0, 0, TO_ROOM);

  for (vict=world[CHAR_REAL_ROOM(dragon)].people;vict;vict=temp) {
     temp = vict->next_in_room;  /* Added temp - Ranger June 96 */
     if (vict) {
       if(number(0,5)<5) {
         if (GET_LEVEL(vict)<LEVEL_IMM) {
           send_to_char("You are burned by the dragon's fire!\n\r",vict);
           damage(dragon, vict, 150, TYPE_UNDEFINED,DAM_FIRE);
         }
       }
     }
  }
  return FALSE;
}

void assign_stables(void) {
  assign_mob(STABLE_BOY,        stable_boy);
  assign_mob(STABLE_MASTER,     stable_boy);
  assign_mob(STABLE_HUNCHBACK,  stable_boy);
  assign_mob(STABLE_BAT,        fly_walk);
  assign_mob(STABLE_GRIFFON,    fly_walk);
  assign_mob(STABLE_EAGLE,      fly_walk);
  assign_mob(STABLE_WYVERN,     fly_walk);
  assign_mob(STABLE_DRAGON,     fly_walk);
  assign_mob(STABLE_MULE,       m_check);
  assign_mob(STABLE_HORSE,      m_check);
  assign_mob(STABLE_WHORSE,     m_check);
  assign_mob(STABLE_CAMEL,      m_check);
  assign_mob(STABLE_SNAIL,      m_check);
  assign_mob(STABLE_SCORPION,   m_check);
  assign_mob(STABLE_TARANTULA,  m_check);
  assign_mob(STABLE_LIZARD,     m_check);
  assign_mob(STABLE_GOLEM,      m_check);
  assign_mob(STABLE_ELEPHANT,   m_check);
  assign_mob(STABLE_CDRAGON,    cel_dragon);

/*  assign_room(STABLE_PLATFORM,  platform);
  assign_room(STABLE_FLYOPLAT,  flying_air);
  assign_room(STABLE_FLYOM,     flying_air);
  assign_room(STABLE_HANDOR,    flying_air);
  assign_room(STABLE_SONIC,     flying_air);
  assign_room(STABLE_ZYCA,      flying_air);
  assign_room(STABLE_FOREST,    flying_air);
  assign_room(STABLE_ZEKIAN,    flying_air);
  assign_room(STABLE_CORN,      flying_air);*/
}

