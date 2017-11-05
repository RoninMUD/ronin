
/* Specs for Keening by Blink
**
** for use in RoninMUD
**
** mods by Ranger
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:52 $
$Header: /home/ronin/cvs/ronin/spec.keening.c,v 2.0.0.1 2004/02/05 16:10:52 ronin Exp $
$Id: spec.keening.c,v 2.0.0.1 2004/02/05 16:10:52 ronin Exp $
$Name:  $
$Log: spec.keening.c,v $
Revision 2.0.0.1  2004/02/05 16:10:52  ronin
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

extern struct obj_data  *object_list;

#define MOB_ARAK_PRIEST  14509

#define OBJ_ORB_WAILING  14505
#define OBJ_ORB_REVERSAL 14506
#define OBJ_ORB_REMOVAL  14508
#define OBJ_KEIRA_CORPSE 14514

#define WLD_ORB_ROOM     14518

int kg_keira_banshee(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;
  OBJ *obj,*corpse,*obj_next,*money;
  int i;
  struct obj_data *create_money( int amount );

  if(cmd==MSG_CORPSE) {
    if(!ch) return FALSE;
    if(ch!=mob) return FALSE;
    if (ch->specials.fighting) stop_fighting(ch);
    death_cry(ch);

    /* make corpse */
    corpse=read_object(OBJ_KEIRA_CORPSE, VIRTUAL);
    if (GET_GOLD(ch)>0) {
      money = create_money(GET_GOLD(ch));
      GET_GOLD(ch)=0;
      obj_to_obj(money,corpse);
    }
    corpse->obj_flags.type_flag = ITEM_CONTAINER;
    corpse->obj_flags.wear_flags = 0;
    corpse->obj_flags.value[0] = 0;
    corpse->obj_flags.value[1] = CONT_NOREMOVE;
    corpse->obj_flags.value[2] = GET_LEVEL(ch);
    corpse->obj_flags.value[3] = 0;
    corpse->obj_flags.weight = GET_WEIGHT(ch);
    for (i=0; i<MAX_WEAR; i++)
     if (ch->equipment[i])
       obj_to_obj(unequip_char(ch, i), corpse);

    for(obj = ch->carrying; obj; obj= obj_next) {
      obj_next=obj->next_content;
      obj_from_char(obj);
      obj_to_obj(obj, corpse);
    }
    obj_to_room(corpse,CHAR_REAL_ROOM(ch));
    send_to_room("The corpse shimmers with an eerie glow.\n\r",CHAR_REAL_ROOM(ch));
    extract_char(ch);
    return TRUE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(5)) {
    act("The Banshee unleashes a sorrowful wail that makes your body convulse in pain.",0,mob,0,0,TO_ROOM);
    act("You wail very loudly!",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      damage(mob,vict,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

 if(chance(5)) {
    act("Keira's eyes glow sadly as she begins a mournful wail.",0,mob,0,0,TO_ROOM);
    act("You wail very loudly!",0,mob,0,0,TO_CHAR);
    send_to_world("A mournful wail echos through the world.\n\r");
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      damage(mob,vict,900,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }
  return FALSE;
}

int kg_ardaan_inquisitor(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(5)) {
    act("Ardaan summons fire from Hell to burn you!",0,mob,0,0,TO_ROOM);
    act("You summon fire from Hell!",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      damage(mob,vict,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  if(chance(10)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Ardaan conjures up an ethereal spear and throws it at $N!",0,mob,0,vict,TO_NOTVICT);
    act("You headbutt $N!",0,mob,0,vict,TO_CHAR);
    act("Ardaan conjures up an ethereal spear and throws it at you!",0,mob,0,vict,TO_VICT);
    damage(mob,vict,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(5)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("Ardaan summons a huge ball of energy and hurls it at $N!",0,mob,0,vict,TO_NOTVICT);
    act("You throw a huge ball of energy at $N!",0,mob,0,vict,TO_CHAR);
    act("You are engulfed in dark energy from Ardaan's evil magic.",0,mob,0,vict,TO_VICT);
    damage(mob,vict,700,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(chance(30)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    if(GET_CLASS(vict)!=CLASS_WARRIOR) return FALSE;
    act("Ardaan grabs $N and viciously pounds $s head.",0,mob,0,vict,TO_NOTVICT);
    act("You pound $N's head madly!",0,mob,0,vict,TO_CHAR);
    act("Ardaan grabs you and viciously pounds your head!",0,mob,0,vict,TO_VICT);
    damage(mob,vict,1300,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  return FALSE;
}

int kg_eduard_magistrate(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*vict_n,*tmp,*newmob;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(5)) {
    act("Eduard runs about the room, viciously trying to claw everyone!",0,mob,0,0,TO_ROOM);
    act("You claw everyone!",0,mob,0,0,TO_CHAR);
    for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=vict_n) {
      vict_n=vict->next_in_room;
      if(vict==mob) continue;
      if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;
      damage(mob,vict,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  if(GET_HIT(mob)>GET_MAX_HIT(mob)/2) return FALSE;

  act("$n growls, 'You have angered me!!!", 1, mob, 0, 0, TO_NOTVICT);
  act("You growl, You have angered me!!!", 1, mob, 0, 0, TO_CHAR);
  act("Eduard the Magistrate grows and transforms into a deadly Were-Rat!", 1,mob, 0, 0, TO_NOTVICT);
  act("You grow and transform into a deadly Were-Rat!", 1, mob, 0, 0,TO_CHAR);

  newmob=read_mobile(14508,VIRTUAL);
  if(newmob) char_to_room(newmob,CHAR_REAL_ROOM(mob));

  /* Move over any items */
  move_eq_from_to(mob,newmob);
  move_inv_from_to(mob,newmob);

  /*tank*/
  tmp=mob->specials.fighting;
  if(tmp->specials.fighting==mob) {
    stop_fighting(tmp);
    set_fighting(newmob,tmp);
  }

  for(tmp=world[CHAR_REAL_ROOM(mob)].people;tmp;tmp=tmp->next_in_room) {
    if(tmp->specials.fighting==mob) {
      stop_fighting(tmp);
      set_fighting(tmp,newmob);
    }
  }
  stop_fighting(mob);
  extract_char(mob);
  return FALSE;
}

int kg_eduard_wererat (CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd!=MSG_MOBACT) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(chance(10)) {
    vict=get_random_victim(mob);
    if(!vict) return FALSE;
    act("The Wererat jumps on $N, gnawing at $s flesh!",0,mob,0,vict,TO_NOTVICT);
    act("You gnaw on $N!",0,mob,0,vict,TO_CHAR);
    act("The Wererat jumps on you and starts to gnaw at your flesh!",0,mob,0,vict,TO_VICT);
    damage(mob,vict,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }
  return FALSE;
}

int kg_yorla_truth(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd!=MSG_MOBACT) return FALSE;

  if(chance(10)) {
    if((victim=get_random_victim(mob))) {
      switch(number(1,4)) {
        case 1:
          sprintf(buf,"say Dispel the Evil that lurks through Keening!");
          break;
        case 2:
          sprintf(buf,"glance %s",GET_NAME(victim));
          break;
        case 3:
          sprintf(buf,"say Two Orbs to form what you need to destroy the odd glow..");
          break;
        case 4:
          sprintf(buf,"say Seek the Priest of Arak!");
          break;
      }
      command_interpreter(mob,buf);
    }
  }
  return FALSE;
}

int kg_dirty_beggar(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd!=MSG_MOBACT) return FALSE;
  if(chance(10)) {
    if((victim=get_random_victim(mob))) {
      switch(number(1,2)) {
        case 1:
          sprintf(buf,"say Alms, Alms for the poor!");
          break;
        case 2:
          sprintf(buf,"say Please, Have mercy!");
          break;
      }
      command_interpreter(mob,buf);
    }
  }
  return FALSE;
}

int kg_arak_priest(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd!=MSG_MOBACT) return FALSE;

  if(chance(10)) {
    if((victim=get_random_victim(mob))) {
      switch(number(1,4)) {
        case 1:
          sprintf(buf,"emot lowers his head in silence.");
          break;
        case 2:
          sprintf(buf,"emote raises his hands to the sky in prayer.");
          break;
        case 3:
          sprintf(buf,"say You require a magical artifact which I can create...");
          break;
        case 4:
          sprintf(buf,"say Bring me two special orbs and place them at my feet, and I shall give you a gift.");
          break;
      }
      command_interpreter(mob,buf);
    }
  }
  return FALSE;
}

int kg_girl_phantom(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd!=MSG_MOBACT) return FALSE;
  if(chance(10)) {
    if((victim=get_random_victim(mob))) {
      switch(number(1,4)) {
        case 1:
          sprintf(buf,"say Keira's evil wail ehoes through my head.");
          break;
        case 2:
          sprintf(buf,"emote cries softly.");
          break;
        case 3:
          sprintf(buf,"say Keira killed me, now I lurk in this meadow for eternity.");
          break;
        case 4:
          sprintf(buf,"say Please, destroy me.. Please..");
          break;
      }
      command_interpreter(mob,buf);
    }
  }
  return FALSE;
}

int kg_make_orb(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  char buf[MIL];
  OBJ *orb_wailing = NULL;
  OBJ *orb_reversal = NULL;
  OBJ *orb_removal = NULL;
  CHAR *arak_priest = NULL;

  if (cmd != CMD_DROP) return FALSE;
  if (!ch) return FALSE;
  if (V_ROOM(ch) != WLD_ORB_ROOM) return FALSE;

  if (ch != obj->carried_by) return FALSE;

  one_argument(arg, buf);

  if (!isname(buf, OBJ_NAME(obj))) return FALSE;

  if (!(arak_priest = get_ch_room(MOB_ARAK_PRIEST, real_room(WLD_ORB_ROOM)))) return FALSE;

  if (V_OBJ(obj) == OBJ_ORB_WAILING)
  {
    orb_wailing = obj;
    orb_reversal = get_obj_room(OBJ_ORB_REVERSAL, WLD_ORB_ROOM);
  }
  else if (V_OBJ(obj) == OBJ_ORB_REVERSAL)
  {
    orb_reversal = obj;
    orb_wailing = get_obj_room(OBJ_ORB_WAILING, WLD_ORB_ROOM);
  }

  if (!orb_wailing || !orb_reversal) return FALSE;

  orb_removal = read_object(OBJ_ORB_REMOVAL, VIRTUAL);

  if (!orb_removal) return FALSE;

  act("$n drops $p.", TRUE, ch, obj, NULL, TO_ROOM);
  act("You drop $p.", TRUE, ch, obj, NULL, TO_CHAR);

  obj_from_char(obj);
  obj_to_room(obj, CHAR_REAL_ROOM(ch));

  act("$N magically binds the orbs in a bright blast of light.", TRUE, ch, NULL, arak_priest, TO_ROOM);
  act("$N magically binds the orbs in a bright blast of light.", TRUE, ch, NULL, arak_priest, TO_CHAR);

  obj_to_room(orb_removal, real_room(WLD_ORB_ROOM));

  obj_from_room(orb_wailing);
  obj_from_room(orb_reversal);

  extract_obj(orb_wailing);
  extract_obj(orb_reversal);

  return TRUE;
}

int kg_keira_drow(CHAR *mob,CHAR *ch, int cmd, char *arg) {
  OBJ *obj;
  CHAR *vict;
  int i;

  if(cmd==MSG_MOBACT) {
    if(mob_proto_table[real_mobile(14501)].number) {
      for(i=0; i<MAX_WEAR; i++)
        if((obj=mob->equipment[i]))
          extract_obj(unequip_char(mob,i));
      extract_char(mob);
      return FALSE;
    }
  }

  if(cmd==MSG_DEAD) {
     act("\n\r$n's body falls to the ground in a lifeless heap.", 0, mob, 0,0 ,TO_ROOM);
     if(CHAR_VIRTUAL_ROOM(mob)!=14590) return FALSE;
     send_to_room("\n\rA dark ray of pure evil bathes the corpse of Keira the Drow Elf.\n\r\
Her corpse slowly rises into the air, fading from reality.  A mist\n\r\
of dark ethereal magic fills the room, and slowly begins to take on\n\r\
a human-like form.  A loud noise erupts from the centre of the mist\n\r\
and from nowhere a terrible banshee takes shape, Keira has returned!\n\r\n\r", mob->in_room_r);
     vict = read_mobile(14501, VIRTUAL);
     char_to_room(vict, mob->in_room_r);
     if(chance(10)) {
       obj=read_object(14507, VIRTUAL);
       if(chance(20)) SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_THIEF);
       if(chance(20)) SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_MAGIC_USER);
       if(chance(20)) SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_NINJA);
       if(chance(20)) SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_NOMAD);
       if(chance(20)) SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_ANTIPALADIN);
       if(chance(20)) SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_BARD);
       if(chance(20)) SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_COMMANDO);
       equip_char(vict, obj, WEAR_FEET);
     }
     if(ch && ch!=mob) set_fighting(vict, ch);
     do_yell(vict, "Please release me from this HELL!!",CMD_YELL);
     send_to_world("The Land of Keening emits a tremendous wave of evil!\n\r");
     extract_char(mob);
     return TRUE;
   }
   return FALSE;
}

int kg_new_orb(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  char buf[MIL];
  OBJ *keira_corpse = NULL;

  if (cmd != CMD_DROP) return FALSE;
  if (!ch) return FALSE;

  if (ch != obj->carried_by) return FALSE;

  one_argument(arg, buf);

  if (!isname(buf, OBJ_NAME(obj))) return FALSE;

  if (!(keira_corpse = get_obj_room(OBJ_KEIRA_CORPSE, CHAR_VIRTUAL_ROOM(ch)))) return FALSE;
  if (keira_corpse->obj_flags.value[1] == 0) return FALSE;

  act("$n drops $p.", TRUE, ch, obj, NULL, TO_ROOM);
  act("You drop $p.", TRUE, ch, obj, NULL, TO_CHAR);

  obj_from_char(obj);
  obj_to_room(obj, CHAR_REAL_ROOM(ch));

  keira_corpse->obj_flags.value[1] = 0;

  send_to_room("The corpse appears to lose its shimmering glow as the orb vanishes.\n\r", CHAR_REAL_ROOM(ch));

  extract_obj(obj);

  return TRUE;
}

int kg_keira_corpse(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  char buf[MIL];
  int bits = 0;
  CHAR *tmp_char = NULL;
  OBJ *tmp_obj = NULL;

  if (cmd != CMD_BUTCHER && cmd != CMD_SKIN) return FALSE;
  if (!ch) return FALSE;

  one_argument(arg, buf);

  if (!isname(buf, OBJ_NAME(obj))) return FALSE;

  if (obj->obj_flags.value[1] == 0) return FALSE;

  if (!(bits = generic_find(buf, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &tmp_obj))) return FALSE;
  if (obj != tmp_obj) return FALSE;

  switch (cmd)
  {
    case CMD_BUTCHER:
      act("The corpse shimmers brightly and repels your attempt at butchering it.", FALSE, ch, NULL, NULL, TO_CHAR);
      act("The corpse shimmers brightly and repels $n's attempt at butchering it.", TRUE, ch, NULL, NULL, TO_ROOM);
    break;

    case CMD_SKIN:
      act("The corpse shimmers brightly and repels your attempt at skinning it.", FALSE, ch, NULL, NULL, TO_CHAR);
      act("The corpse shimmers brightly and repels $n's attempt at skinning it.", TRUE, ch, NULL, NULL, TO_ROOM);
    break;
  }

  return TRUE;
}

void assign_keening(void) {
  assign_mob(14500,kg_keira_drow);
  assign_mob(14501,kg_keira_banshee);
  assign_mob(14502,kg_yorla_truth);
  assign_mob(14503,kg_ardaan_inquisitor);
  assign_mob(14507,kg_eduard_magistrate);
  assign_mob(14508,kg_eduard_wererat);
  assign_mob(MOB_ARAK_PRIEST, kg_arak_priest);
  assign_mob(14510,kg_girl_phantom);
  assign_mob(14516,kg_dirty_beggar);

  assign_obj(OBJ_ORB_WAILING, kg_make_orb);
  assign_obj(OBJ_ORB_REVERSAL, kg_make_orb);
  assign_obj(OBJ_ORB_REMOVAL, kg_new_orb);
  assign_obj(OBJ_KEIRA_CORPSE, kg_keira_corpse);
}


