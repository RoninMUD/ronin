/*
///  Special Procedure Module        Orig. Date 7-06-97
///                                  Last Modif 7-14-98
///  spec.drow.c --- Specials for Eryndlyn I,II and III by Jarldian
///
///  Written by Jarldian of RoninMUD - burn@cnct.com
///  Using This code is not allowed without permission from originator.
///
///  Mucho changes and additions by Ranger - July 98
///  -Dec 2001 - added two vanity scythes-assigns to spec. -by Liner
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

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
#include "spec_assign.h"

void move_eq_from_to (struct char_data *fch, struct char_data *tch);

/*************************
MOB SPECS
*************************/

int drow_mushroom(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd) return FALSE;
  if(!mob->specials.fighting) return FALSE;
  if(!(vict=get_random_victim(mob))) return FALSE;

  if(!number(0,4)) {
    act("$n shifts its cap slightly and blasts a cloud of spores at $N.", 1, mob, 0, vict, TO_NOTVICT);
    act("$n shifts its cap slightly and blasts a cloud of spores at you.",1, mob, 0, vict, TO_VICT);
    act("You shift your cap slightly and blast a cloud of spores at $N.", 1, mob, 0, vict, TO_CHAR);
    spell_poison(30,mob,vict,0);
    damage(mob, vict, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }
  return FALSE;
}

int drow_koala(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *newmob,*tmp;

  if(cmd) return FALSE;
  if(!mob->specials.fighting) return FALSE;
  act("$n squeaks, 'You messed with the wrong bear...pal!", 1, mob, 0, 0, TO_NOTVICT);
  act("You squeak 'You messed with the wrong bear...pal!", 1, mob, 0, 0, TO_CHAR);
  act("The koala bear grows and transforms into a huge monster.", 1, mob, 0, 0, TO_NOTVICT);
  act("You grow and transform into a huge monster.", 1, mob, 0, 0, TO_CHAR);

  newmob=read_mobile(5597,VIRTUAL);
  if(newmob) char_to_room(newmob,CHAR_REAL_ROOM(mob));

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
int is_empty(int i);
int drow_queen(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict1=0,*vict2=0,*tmp;
  OBJ *obj;
  int i,i1,i2,num;
  char buf[MAX_INPUT_LENGTH],buf1[MAX_INPUT_LENGTH];

  if(cmd==MSG_MOBACT && V_MOB(mob)==5580) {
    if(mob_proto_table[real_mobile(5578)].number) {
      for(i=0; i<MAX_WEAR; i++) {
        obj=mob->equipment[i];
        if(obj) {
          unequip_char(mob,i);
          extract_obj(obj);
        }
      }
      extract_char(mob);
      return FALSE;
    }
  }

  if(cmd) return FALSE;
  if(!mob->specials.fighting) return FALSE;

  if(V_MOB(mob)==5580 && GET_HIT(mob)<2000) {
    act("$n snarls, 'filthy mortals, do you know who I am??? You cannot kill one who is IMMORTAL!!'", 1, mob, 0, 0, TO_NOTVICT);
    act("You snarls, 'filthy mortals, do you know who I am??? You cannot kill one who is IMMORTAL!!'", 1, mob, 0, 0, TO_CHAR);
    act("$n screams in agony as she takes the form of a drow maiden.", 1, mob, 0, 0, TO_NOTVICT);
    act("You screams in agony as she takes the form of a drow maiden.", 1, mob, 0, 0, TO_CHAR);

    vict1=read_mobile(5578,VIRTUAL);
    if(vict1) char_to_room(vict1,CHAR_REAL_ROOM(mob));
    move_eq_from_to (mob,vict1);

    /*tank*/
    tmp=mob->specials.fighting;
    if(tmp->specials.fighting==mob) {
      stop_fighting(tmp);
      set_fighting(vict1,tmp);
    }

    for(tmp=world[CHAR_REAL_ROOM(mob)].people;tmp;tmp=tmp->next_in_room) {
      if(tmp->specials.fighting==mob) {
        stop_fighting(tmp);
        set_fighting(tmp,vict1);
      }
    }
    stop_fighting(mob);
    extract_char(mob);
    return FALSE;
  }

  if(V_MOB(mob)==5578 && GET_HIT(mob)<GET_MAX_HIT(mob)/10 && world[CHAR_REAL_ROOM(mob)].number==5580) {
    if(!mob->specials.fighting) return FALSE;
    obj=read_object(5999,VIRTUAL);
    obj_to_room(obj,CHAR_REAL_ROOM(mob));
    act("$n makes a strange twist and a portal appears.",0,mob,0,0,TO_ROOM);
    act("You makes a strange twist and a portal appears.",0,mob,0,0,TO_CHAR);
    act("$n says 'I will NEVER die!' and jumps in the portal.",0,mob,0,0,TO_ROOM);
    act("You say 'I will NEVER die!' and jump in the portal.",0,mob,0,0,TO_CHAR);
    if(real_room(5998)!=NOWHERE) {
      char_from_room(mob);
      char_to_room(mob,real_room(5998));
      GET_HIT(mob)=GET_MAX_HIT(mob);
    }
    else {
      send_to_room("GIMP! Load the other zone *FART*\n\r",CHAR_REAL_ROOM(mob));
    }
  }
 /* stun spec */
  num=count_mortals_real_room(CHAR_REAL_ROOM(mob))-1;
  if(num>1 && V_MOB(mob)==5580 && !number(0,3)) {
    i1=number(1,num);
    i2=number(1,num);
    if(i2==i1) {
      if(i2<num) i2++;
      else i2--;
    }
    vict1=0;
    vict2=0;
    i=0;
    for(tmp=world[CHAR_REAL_ROOM(mob)].people;tmp;tmp=tmp->next_in_room) {
      if(IS_MORTAL(tmp)&& tmp!=mob->specials.fighting) {
        i++;
        if(!vict1 && i==i1) vict1=tmp;
        if(!vict2 && i==i2) vict2=tmp;
      }
    }
    if(vict1 && vict2) {
      sprintf(buf,"The Spider Queen's flailing limbs reach and grab for %s and %s.\n\r",GET_NAME(vict1),GET_NAME(vict2));
      sprintf(buf1,"The Spider Queen laughs heartily as she hurls them into a nearby wall like a pair of wooden twigs.\n\r");
      for(tmp=world[CHAR_REAL_ROOM(mob)].people;tmp;tmp=tmp->next_in_room) {
        if(tmp!=mob && tmp!=vict1 && tmp!=vict2)
          send_to_char(buf,tmp);
          send_to_char(buf1,tmp);
      }
      sprintf(buf,"The Spider Queen laughs heartily as she hurls you into a nearby wall..\n\r");
      sprintf(buf1,"The Spider Queen's flailing limbs reach and grab for you and %s.\n\r",GET_NAME(vict2));
      send_to_char(buf1,vict1);
      send_to_char(buf,vict1);
      sprintf(buf1,"The Spider Queen's flailing limbs reach and grab for you and %s.\n\r",GET_NAME(vict1));
      send_to_char(buf1,vict2);
      send_to_char(buf,vict2);
      sprintf(buf,"You grab %s and %s and throw them against the wall.\n\r",GET_NAME(vict1),GET_NAME(vict2));
      send_to_char(buf,mob);
      stop_fighting(vict1);
      stop_fighting(vict2);
      GET_POS(vict1)=POSITION_SITTING;
      GET_POS(vict2)=POSITION_SITTING;
      WAIT_STATE(vict1, PULSE_VIOLENCE *2);
      WAIT_STATE(vict2, PULSE_VIOLENCE *2);
      return FALSE;
    }
  } /* end of stun */

  /* Always keep 4 yochols around */
  num=0;
  for(tmp=world[CHAR_REAL_ROOM(mob)].people; tmp; tmp=tmp->next_in_room) {
    if(IS_MOB(tmp)) {
      if(V_MOB(tmp)==5510) num = num + 1;
    }
  }
  if(num<4) {
    act("$n screams 'handmaidens, come forth and do my bidding!'",1, mob, 0,0, TO_ROOM);
    act("You scream 'handmaidens, come forth and do my bidding!'",1, mob, 0,0, TO_CHAR);
    tmp=read_mobile(5510, VIRTUAL);
    if(tmp) char_to_room(tmp, CHAR_REAL_ROOM(mob));
    send_to_room("A portal opens and a yochlol steps out.\n\r",CHAR_REAL_ROOM(mob));
    if((vict1=get_random_victim(mob))) set_fighting(tmp,vict1);
    return FALSE;
  }
  /* end of yochols */

  /* stun and stop fighting */
  if(!number(0,5)) {
    if((vict1=get_random_victim(mob))) {
      if(vict1!=mob->specials.fighting) {
        act("$n snarls, 'congratulations you've been recruited.'",1,mob,0,0,TO_ROOM);
        act("$n right eye flickers with a strange light.",1,mob,0,0,TO_ROOM);
        act("Your right eye flickers with a strange light.",1,mob,0,0,TO_CHAR);
        act("You glare right into the face of $N.",1,mob,0,vict1,TO_CHAR);
        act("$n glares right into the face of $N.",1,mob,0,vict1,TO_NOTVICT);
        act("$n glares right into your face.",1,mob,0,vict1,TO_VICT);
        stop_fighting(vict1);
        GET_POS(vict1)=POSITION_SITTING;
        WAIT_STATE(vict1,PULSE_VIOLENCE*6);
        return FALSE;
      }
    }
  }

  /* Massive room damage - ouch */
  if(!number(0,5)) {
    send_to_room("The daylight from the room turns into night as fire erupts from the bowels of hell!!\n\r",CHAR_REAL_ROOM(mob));
    for(vict1=world[CHAR_REAL_ROOM(mob)].people;vict1;vict1=tmp) {
      tmp=vict1->next_in_room;
      if(vict1!=mob && GET_LEVEL(vict1)<LEVEL_IMM) {
        send_to_char("Hellfire scorces your skin and nearly blinds you!\n\r",vict1);
        damage(mob,vict1,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
      }
    }
    return FALSE;
  }

  return FALSE;
}

/*
Mob#5153 - Gnome
  - talk spec
*/

int drow_gnome(CHAR *mob, CHAR *ch, int cmd, char *arg) {

  if(cmd) return FALSE;
  if(world[CHAR_REAL_ROOM(mob)].people==mob) return FALSE;

  if (GET_POS(mob)>POSITION_FIGHTING) {
   if(!number(0,20)) {
    act("$n happily mines away at the wall.",FALSE,mob,0,0,TO_ROOM);
    act("$n smiles happily.",FALSE,mob,0,0,TO_ROOM);
   }
  }
  return FALSE;
}

int drow_apprentice(CHAR *mob, CHAR *ch, int cmd, char *arg) {

  if(cmd!=MSG_TICK) return FALSE;
  if(world[CHAR_REAL_ROOM(mob)].people==mob) return FALSE;

  if (GET_POS(mob)>POSITION_FIGHTING && !number(0,3)) {
    act("$n closes his eyes and conjures a ball of purple flame.",FALSE,mob,0,0,TO_ROOM);
    act("$n hurls the ball into the pillar and it is absorbed instantly.",FALSE,mob,0,0,TO_ROOM);
  }
  return FALSE;
}

int drow_leader(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  struct char_data *vict;
  struct follow_type *f;
  char buf[MAX_STRING_LENGTH];
  extern const char *pc_class_types[];

  if(cmd) return FALSE;
  if(!mob->specials.fighting) return FALSE;
  if(!(vict=get_random_victim(mob)) || number(0,3) ) return FALSE;
  if(IS_MOB(vict)) return FALSE;
  sprintf(buf,"$n yells 'Get the %s'!!",pc_class_types[GET_CLASS(vict)]);
  act(buf,0,mob,0,0,TO_ROOM);
  act("$n and his followers start hitting YOU!",0,mob,0,vict,TO_VICT);
  act("You and your followers start hitting $N!",0,mob,0,vict,TO_CHAR);
  act("$n and his followers start hitting $N!",0,mob,0,vict,TO_ROOM);

  for(f=mob->followers; f; f=f->next) {
    if(IS_MOB(f->follower)) {
      if (f->follower->specials.fighting) stop_fighting(f->follower);
      hit(f->follower, vict, TYPE_UNDEFINED);
      if(CHAR_REAL_ROOM(vict)==NOWHERE) return FALSE;
    }
  }
  stop_fighting(mob);
  hit(mob,vict,TYPE_UNDEFINED);
  return FALSE;
}

/*
Mob#5167 - Duegar Dwarf
  - talk spec
*/

int drow_duegar(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd) return FALSE;

  if(world[CHAR_REAL_ROOM(mob)].people==mob) return FALSE;
  if (GET_POS(mob)>POSITION_FIGHTING) {
   if(!number(0,20)) {
    act("$n groans loudly at the sight of his broken cart.",FALSE,mob,0,0,TO_ROOM);
    act("$n growls, 'damned cart! If aye don't fix this, damned Matron mother is going to have me head!",FALSE,mob,0,0,TO_ROOM);
   }
  }
  return FALSE;
}

/*
Mob#5169 - Huge Lizard
  - talk spec
*/

int drow_lizard(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd)
   return FALSE;

  if(world[CHAR_REAL_ROOM(mob)].people==mob) return FALSE;
  if (GET_POS(mob)>POSITION_FIGHTING) {
   if(!number(0,20)) {
    act("$n snorts loudly.",FALSE,mob,0,0,TO_ROOM);
    }
  }
  return FALSE;
}

/*
Mob#5169 - Glabrezu
  - 200 hp random spec (arm flail at vic)
  - 370 hp breath of fire
*/

int drow_glabrezu(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd) return FALSE;
  if(!mob->specials.fighting) return FALSE;
  if(!(vict=get_random_victim(mob))) return FALSE;

  if(!number(0,7)) {
      act("$n grabs $N with his 10 arms and flails $M into a bloody pulp!\n\rBlood flies everywhere!", 1, mob, 0, vict, TO_NOTVICT);
      act("$n grabs you with his 10 arms and flails you into bloody pulp!",1, mob, 0, vict, TO_VICT);
      act("You grab $N with your 10 arms and flails $M into a bloody pulp!", 1, mob, 0, vict, TO_CHAR);
      damage(mob, vict, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
  }
  if(!number(0,7)) {
      act("$n eye's suddenly flares a bright red.\n", 1, mob, 0, vict, TO_NOTVICT);
      act("$n eye's suddenly flares a bright red.\n",1, mob, 0, vict, TO_VICT);
      act("You scream as flames erupt abruptly from thin air.\n\n",1, mob, 0, 0, TO_CHAR);
      damage(mob, vict, 350, TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
  }
  return FALSE;
}

/*
Mobs#5593,5594,5595,5598 - skeletons
  -block
  -msg when pc enters room
*/

int drow_skeleton(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  if(cmd==MSG_ENTER){
    act("From the dirt, piles of bones rise up, joints connecting to produce an animated form.",1,mob,0,ch,TO_VICT);
    return FALSE;
  }
  if((cmd==CMD_EAST)||(cmd==CMD_SOUTH)||(cmd==CMD_WEST)||(cmd==CMD_DOWN)||(cmd==CMD_NORTH)){
    send_to_char("A rotted arm swings into your path and freezes you into place.\n\r", ch);
    act("A skeletal warrior lifts its arm and swiftly blocks $n's attempt to advance.", FALSE, ch, 0,0,TO_ROOM);
    return TRUE;
  }
  if(!cmd && V_MOB(mob)==5593) {
    if(!mob->specials.fighting) return FALSE;
    if(!(vict=get_random_victim(mob)) || number(0,4)) return FALSE;
    act("The skeleton claws $N with a withered hand, drawing some of $S strength.",0,mob,0,vict,TO_ROOM);
    act("The skeleton claws you with a withered hand, drawing some of your strength.",0,mob,0,vict,TO_VICT);
    act("You claw $N with a withered hand, drawing some of $S strength.",0,mob,0,vict,TO_CHAR);
    spell_chill_touch(30,mob,vict,0);
    return FALSE;
  }
  return FALSE;
}

/*
Mob#5168 - statue of daggoor
  -block
*/

int drow_statue(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if((cmd==CMD_WEST)||(cmd==CMD_EAST)){
    send_to_char("You try to advance, but can't!\n\rYou hear a whisper of whisling followed by a handful of webbing dragging you by the feet.\n\r", ch);
    act("A strand of web shoots out from the statue's outstreched hands and tangles up $n!\n\r", FALSE, ch, 0,0,TO_ROOM);
    return TRUE;
  }
  return FALSE;
}

/*
Mob#5140 & 5191 5189 - First & second & fourth spider sentry
  -block
*/

int sentry_one(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  if(cmd==CMD_NORTH){
    send_to_char("The Spider crawls into your path.\n\r", ch);
    act("The Spider crawls into $n's path.", FALSE, ch, 0,0,TO_ROOM);
    return TRUE;
  }
  if(!cmd && mob->specials.fighting) {
    if(!(vict=get_random_victim(mob)) || number(0,5)) return FALSE;
    act("With a chilling scream, $n rips a huge gash in $N's gut!",0,mob,0,vict,TO_NOTVICT);
    act("With a chilling scream, you rip a huge gash in $N's gut!",0,mob,0,vict,TO_CHAR);
    act("With a chilling scream, $n rips a huge gash in your gut!",0,mob,0,vict,TO_VICT);
    damage(mob,vict,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }
  return FALSE;
}

/*
Mob#5184 - Third spider sentry
  -block
*/

int sentry_three(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  if(cmd==CMD_SOUTH){
    send_to_char("The Spider crawls into your path.\n\r", ch);
    act("The Spider crawls into $n's path.", FALSE, ch, 0,0,TO_ROOM);
    return TRUE;
  }
  if(!cmd && mob->specials.fighting) {
    if(!(vict=get_random_victim(mob)) || number(0,5)) return FALSE;
    act("With a chilling scream, $n rips a huge gash in $N's gut!",0,mob,0,vict,TO_NOTVICT);
    act("With a chilling scream, you rip a huge gash in $N's gut!",0,mob,0,vict,TO_CHAR);
    act("With a chilling scream, $n rips a huge gash in your gut!",0,mob,0,vict,TO_VICT);
    damage(mob,vict,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }
  return FALSE;
}

/**************************
OBJECT SPECS
**************************/

int narbondel(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if(cmd!=MSG_TICK) return FALSE;
  if(!world[obj->in_room].people) return FALSE;
  send_to_room("Azure flames flick off this towering artifact, beginning the cycle anew.\n\r", obj->in_room);
  return FALSE;
}

int altar_dagoor(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if(cmd!=MSG_TICK) return FALSE;
  if(!world[obj->in_room].people) return FALSE;
  send_to_room("Thousands of tormented souls swirl around and about the room, wailing in agony.\n\r", obj->in_room);
  return FALSE;
}

int find_obj(int v_num, int virt_room);

int segments(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  int tel=FALSE;
  CHAR *tel_ch, *tel_char_next;
  OBJ *tmp, *tmp_next;
  char buf[80];

  if(!ch)return FALSE;
  if(cmd!=CMD_DROP) return FALSE;
  if(world[CHAR_REAL_ROOM(ch)].number!=5567) return FALSE;

  one_argument(arg,buf);
  if(!isname(buf,OBJ_NAME(obj))) return FALSE;
  do_drop(ch, arg, cmd);

  if(find_obj(5122,5567)&& find_obj(5123,5567)&& find_obj(5125,5567)&& find_obj(5126,5567)&& find_obj(5551,5567))
    tel=TRUE;

  if(tel==TRUE){
    act("$p glows briefly and suddendly disappears.\n\rYou are suddently coaxed in bright white light.",TRUE,ch,obj,0,TO_ROOM);
    act("$p glows briefly and suddendly disappears.\n\rYou are suddently coaxed in bright white light.",TRUE,ch,obj,0,TO_CHAR);
    for(tmp=world[real_room(5567)].contents;tmp;tmp=tmp_next){
      tmp_next=tmp->next_content;
      if(V_OBJ(tmp)==5122 || V_OBJ(tmp)==5123 || V_OBJ(tmp)==5125 || V_OBJ(tmp)==5126 || V_OBJ(tmp)==5551)
        extract_obj(tmp); /*destroy all segments lying around*/
    }

    for(tel_ch=world[real_room(5567)].people;tel_ch;tel_ch=tel_char_next){
      tel_char_next=tel_ch->next_in_room;
      send_to_char("You are transported!\n\r",tel_ch);
      char_from_room(tel_ch);
      char_to_room(tel_ch, real_room(5587));/*move all chars to this room*/
      do_look(tel_ch,"",CMD_LOOK);
    }
  }
  return TRUE;
}

int drow_relic(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  OBJ *newobj;

  char buf[MAX_INPUT_LENGTH];
  one_argument(arg,buf);

  if(cmd==MSG_TICK) {
    if(obj->carried_by && chance(33)) {
      send_to_char("A powerful voice in your mind urges you to wield the relic.\n\r",obj->carried_by);
    }
  }

  if(cmd==CMD_WIELD) {
    if(!ch) return FALSE;
    if(obj->carried_by!=ch) return FALSE;

    /* Make sure char is wielding relic or terrakar, etc
     *  -- Lem, Aug 2001
     */
    if(!(isname(buf, OBJ_NAME(obj)))) {
      return FALSE;
    }
    if(ch->equipment[WIELD]) {
      send_to_char("You are already wielding something.\n\r",ch);
      return TRUE;
    }
    if(GET_CLASS(ch)==CLASS_CLERIC ||
       GET_CLASS(ch)==CLASS_MAGIC_USER ||
       GET_CLASS(ch)==CLASS_THIEF ||
       GET_CLASS(ch)==CLASS_NINJA ||
       GET_CLASS(ch)==CLASS_NOMAD) return FALSE;
    newobj=0;
    if(IS_EVIL(ch)) newobj=read_object(5588, VIRTUAL);
    if(IS_GOOD(ch)) newobj=read_object(5589, VIRTUAL);
    if(IS_NEUTRAL(ch)) newobj=read_object(5590, VIRTUAL);
    if(!newobj) return FALSE;
    act("The drow relic glows brightly and magically shapes into $p.",0,ch,newobj,0,TO_ROOM);
    act("The drow relic glows brightly and magically shapes into $p.",0,ch,newobj,0,TO_CHAR);
    equip_char(ch,newobj,WIELD);
    extract_obj(obj);
    return TRUE;
  }

  return FALSE;
}
int drow_relic2(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd==MSG_MOBACT) {
    if(!obj->equipped_by) return FALSE;
    if(!obj->equipped_by->specials.fighting) return FALSE;
    if(OBJ_SPEC(obj)!=30 || number(0,9)) return FALSE;
    act("$n's weapon shivers in $s hand suddenly, and strikes $N.",1,obj->equipped_by,0,obj->equipped_by->specials.fighting,TO_ROOM);
    act("Terrakar shivers in your hands, and strikes $N.",1,obj->equipped_by,0,obj->equipped_by->specials.fighting,TO_CHAR);
    OBJ_SPEC(obj)=0;
    damage(obj->equipped_by,obj->equipped_by->specials.fighting,50,TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  if(cmd==MSG_TICK) {
    if(obj->equipped_by) {
      if(GET_CLASS(obj->equipped_by)==CLASS_CLERIC ||
         GET_CLASS(obj->equipped_by)==CLASS_MAGIC_USER ||
         GET_CLASS(obj->equipped_by)==CLASS_THIEF ||
         GET_CLASS(obj->equipped_by)==CLASS_NINJA ||
         GET_CLASS(obj->equipped_by)==CLASS_NOMAD) {
           if(obj==EQ(obj->equipped_by,WIELD)) {
             vict=obj->equipped_by;
             obj_to_char(unequip_char(vict,WIELD),vict);
             send_to_char("Your weapon shivers and leaps out of your hand.\n\r",vict);
           }
      }

      if(OBJ_SPEC(obj)<30) OBJ_SPEC(obj)+=1;
    }
    return FALSE;
  }
  return FALSE;
}

/**************************
ROOM SPECS
**************************/

/*
Room#5163 - 5165
Subsequent messages each tick
*/

int red_lake(int room, CHAR *ch, int cmd, char *arg) {
  if(cmd!=MSG_TICK) return FALSE;
  if(!world[room].people) return FALSE;
  if(number(0,1))
    send_to_room("Steam rises up from below the lake, making the room quite humid.\n\r",room);
  else
    send_to_room("The waters continue to bubble steadily, filling your eyes with condensation.\n\r",room);
  return FALSE;
}

int devil_pool(int room, CHAR *ch, int cmd, char *arg) {
  if(cmd!=MSG_ENTER) return FALSE;
  send_to_char("You hear movement in the water.\n\r", ch);
  return FALSE;
}

int draining_fissure(int room, CHAR *ch, int cmd, char *arg) {
  if(cmd!=MSG_TICK) return FALSE;
  if(!world[room].people) return FALSE;
  send_to_room("Trickling water drains noisily into the fissure.\n\r",room);
  return FALSE;
}

int drow_city(int room, CHAR *ch, int cmd, char *arg) {

  if(cmd!=MSG_TICK) return FALSE;
  if(!world[room].people) return FALSE;
  switch(number(0,3)) {
    case 0:
      send_to_room("You hear footsteps in the distance.\n\r", room);
      break;
    case 1:
      send_to_room("Suddendly, a blood gurguled scream cries out, then, it ends abruptly all together.\n\r", room);
      break;
    case 2:
      send_to_room("You can sense the movement within the shadows.\n\r", room);
      break;
    case 3:
      send_to_room("An orc runs up and points to you, 'Zarish, Jakjis psow! Ha! Ha!' and runs off.\n\r", room);
      break;
  }
  return FALSE;
}

int drow_chasm(int room, CHAR *ch, int cmd, char *arg) {

  if(cmd!=MSG_TICK) return FALSE;
  if(!world[room].people) return FALSE;
  send_to_room("The bridge rattles to the fierce winds swirling continously.\n\r",room);
  return FALSE;
}

int pier_magic(int room, CHAR *ch, int cmd, char *arg) {
  CHAR *tmp;

  if(cmd!=MSG_TICK) return FALSE;
  if(!world[room].people) return FALSE;
  send_to_room("Magic swirls around the globe and fills the room with purple light.\n\rYou bask in its warm illumination.\n\r", room);

  for(tmp=world[room].people;tmp;tmp=tmp->next_in_room) {
    GET_MANA(tmp)+=40;
    GET_MANA(tmp)=MIN(GET_MANA(tmp),GET_MAX_MANA(tmp));
  }
  return FALSE;
}

int drow_portal(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(cmd==MSG_ZONE_RESET && !mob_proto_table[real_mobile(5578)].number) {
    if(obj->in_room!=NOWHERE) {
      send_to_room("With a loud POP! the portal disappears.\n\r",obj->in_room);
      extract_obj(obj);
    }
    return FALSE;
  }

  if(!ch) return(FALSE);
  if (cmd != CMD_ENTER || !AWAKE (ch)) return FALSE;

  one_argument(arg,buf);
  if (!isname(buf, OBJ_NAME(obj))) return FALSE;

  if(GET_POS(ch)==POSITION_RIDING) {
    send_to_char("Dismount first.\n\r",ch);
    return TRUE;
  }

  if(GET_POS(ch)<POSITION_STANDING) {
    send_to_char("You must be standing.\n\r",ch);
    return TRUE;
  }

  if(real_room(5999)==NOWHERE) {
    send_to_char("Nothing happens.\n\r",ch);
    return TRUE;
  }
  send_to_char("You enter the dark blue portal.\n\r",ch);
  act ("$n disappears into the portal's surface.",TRUE,ch,0,0,TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, real_room(5999));
  act("$n appears from nowhere.",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  return TRUE;
}


void assign_eryndlyn(void) {
  int i;

  assign_mob(5580  , drow_queen);
  assign_mob(5578  , drow_queen);
  assign_mob(5153  , drow_gnome);
  assign_mob(5167  , drow_duegar);
  assign_mob(5169  , drow_lizard);
  assign_mob(5169  , drow_glabrezu);
  assign_mob(5593  , drow_skeleton);
  assign_mob(5594  , drow_skeleton);
  assign_mob(5595  , drow_skeleton);
  assign_mob(5598  , drow_skeleton);
  assign_mob(5105  , drow_apprentice);
  assign_mob(5197  , drow_mushroom);
  assign_mob(5579  , drow_koala);
  assign_mob(5168  , drow_statue);
  assign_mob(5132  , drow_leader);
  assign_mob(5901  , drow_leader);
  assign_mob(5140  , sentry_one);
  assign_mob(5191  , sentry_one);/*2*/
  assign_mob(5184  , sentry_three);
  assign_mob(5189  , sentry_one);/*4*/

  assign_obj(5170  , narbondel);
  assign_obj(5587  , drow_relic);
  assign_obj(5588  , drow_relic2);
  assign_obj(5589  , drow_relic2);
  assign_obj(5590  , drow_relic2);
  assign_obj(29888 , drow_relic2);   /* Dec '01 new assign vanity */
  assign_obj(29890 , drow_relic2);   /* Dec '01 new assign vanity */
  assign_obj(5124  , altar_dagoor);
  assign_obj(5567  , altar_dagoor);
  assign_obj(5122  , segments);
  assign_obj(5123  , segments);
  assign_obj(5125  , segments);
  assign_obj(5126  , segments);
  assign_obj(5551  , segments);
  assign_obj(5999  , drow_portal);

  assign_room(5163 , red_lake);
  assign_room(5164 , red_lake);
  assign_room(5165 , red_lake);
  assign_room(5590 , devil_pool);
  assign_room(5166,  draining_fissure);
  for(i=5101;i<5103;i++)
    assign_room(i    , drow_city);
  for(i=5114;i<5117;i++)
    assign_room(i    , drow_city);
  for(i=5120;i<5121;i++)
    assign_room(i    , drow_city);
  for(i=5169;i<5170;i++)
    assign_room(i    , drow_city);
  for(i=5171;i<5176;i++)
    assign_room(i    , drow_city);

  assign_room(5125 , drow_city);
  assign_room(5178 , drow_city);
  assign_room(5137 , drow_chasm);
  assign_room(5138 , pier_magic);
}
