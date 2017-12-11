/* spec.house.c
**
** Special procedures for the Haunted House
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
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "interpreter.h"
#include "utility.h"
#include "fight.h"
#include "reception.h"
#include "act.h"
#include "cmd.h"
#include "spec_assign.h"
#include "subclass.h"

void hit (CHAR *ch,CHAR *tch,int type);
bool is_caster (CHAR *ch);

#define ZONE_HOUSE      14000

#define SHADOW_CLOAK      ITEM(ZONE_HOUSE,2)
#define SHADOW_ORB      ITEM(ZONE_HOUSE,3)
#define SKULL           ITEM(ZONE_HOUSE,6)
#define GLOVES_ALDUIN      ITEM(ZONE_HOUSE,29)
#define BLACK_CLOTH      ITEM(ZONE_HOUSE,39)

#define INSIDE_MIRROR      ITEM(ZONE_HOUSE,28)
#define GUARDIAN_POS      ITEM(ZONE_HOUSE,61)

#define GUARDIAN      ITEM(ZONE_HOUSE,5)
#define MIRROR_IMAGE      ITEM(ZONE_HOUSE,12)
#define SHADOW           ITEM(ZONE_HOUSE,13)
#define PAINTING         14020
#define PAINTKEY         14030
#define ETHER_POWER      600
#define ETHER_POWER_NEW  900
#define HP_POWER_NEW     1000


void drain_mana_hp(struct char_data *ch, struct char_data *vict, int mana, int hit, bool add_m, bool
add_hp) {
  void update_pos(struct char_data *ch);
  int MIN(int a, int b);
  int MAX(int a, int b);
  int mana_gained, hp_gained;

  if(mana) {
    mana_gained = MIN(mana, GET_MANA(vict));
    GET_MANA(vict) = MAX(0, GET_MANA(vict) - mana_gained);
    if(add_m)
      GET_MANA(ch) = MIN(GET_MAX_MANA(ch)+(GET_MAX_MANA(ch)/3), GET_MANA(ch) + mana_gained);
  }      /* mana can't go over 133% max */

  if(hit) {
    hp_gained = MIN(hit, GET_HIT(vict));
    GET_HIT(vict) = MAX(0, GET_HIT(vict) - hp_gained);
    if(add_hp)
      GET_HIT(ch) = MIN(GET_MAX_HIT(ch)+(GET_MAX_HIT(ch)/3), GET_HIT(ch) + hp_gained);
  }      /* hit can't go over 133% max */
}


int painting(OBJ *paint, CHAR *ch, int cmd, char *arg) {
  OBJ *key;
  char buf[MAX_INPUT_LENGTH];
  int paint_key;

  if(!ch) return FALSE;
  paint_key = real_object(PAINTKEY);

  if (cmd == CMD_UNLOCK) {
    one_argument(arg,buf);
    if (!isname(buf, OBJ_NAME(paint))) return FALSE;

    if (!(key = get_obj_in_list_num (paint_key, ch->carrying))) return FALSE;

    send_to_char ("*CLICK*\n\r*CRACK*\n\r",ch);
    REMOVE_BIT(paint->obj_flags.value[1], CONT_LOCKED);
    act("As $n unlocks $p, the hexagon crumbles to dust.",FALSE,ch,paint,0,TO_ROOM);
    extract_obj (key);
    return TRUE;
  }
  return FALSE;
}

void
drain_life(CHAR *ch,CHAR *vict,int part) {
  int drain;

  act("Blue crackling energy flows from $N to $n.",FALSE,ch,0,vict,TO_NOTVICT);
  act("Blue,crackling energy flows fron you to $n.",FALSE,ch,0,vict,TO_VICT);
  act("Blue,crackling energy flows fron $N to you.",FALSE,ch,0,vict,TO_CHAR);
  send_to_char("You feel drained of your life.\n",vict);

  drain = (int) (part * GET_EXP(vict) / 100);
  GET_EXP(vict) = MAX(0,GET_EXP(vict) - drain);
  GET_EXP(ch) = GET_EXP(ch) + drain;
}

int
DreamShadow(CHAR *shadow, CHAR *ch,int cmd,char *arg) {
  CHAR *tch, *sh;
  int drain;
  struct affected_type_5 af;
  void stop_fighting (CHAR *ch);

  if(cmd) return FALSE;
  if(chance(90)) return FALSE;

  if (!(tch = get_random_victim(shadow))) return FALSE;
  switch (number(0,15)) {
    case 0:
    case 1:
    case 2:
      act("$n utters a strange mantra, and points at $N.",
       FALSE,shadow,0,tch,TO_NOTVICT);
      act("$n utters a strange mantra, and points at you.",
       FALSE,shadow,0,tch,TO_VICT);
      act("You utter a strange mantra, and point at $N.",
       FALSE,shadow,0,tch,TO_CHAR);
      send_to_char("You feel weaker.\n",tch);

      af.type = SPELL_STRENGTH;
      af.location = APPLY_STR;
      af.modifier = -6;
      af.duration = number(4,8);
      af.bitvector = 0;
      af.bitvector2 = 0;
      affect_to_char(tch,&af);
      break;

    case 3:
    case 4:
    case 5:
    case 6:
      act("$n utters a strange mantra, and points at $N.",
       FALSE,shadow,0,tch,TO_NOTVICT);
      act("$n utters a strange mantra, and points at you.",
       FALSE,shadow,0,tch,TO_VICT);
      act("You utter a strange mantra, and point at $N.",
       FALSE,shadow,0,tch,TO_CHAR);
      send_to_char ("You can't see! The shadows have blinded you.\n",tch);

      af.type = SPELL_BLINDNESS;
      af.location = APPLY_HITROLL;
      af.modifier = -8;
      af.bitvector = AFF_BLIND;
      af.bitvector2 = 0;
      af.duration = 2;
      affect_to_char (tch, &af);

      af.location = APPLY_ARMOR;
      af.modifier = +60;
      affect_to_char (tch, &af);
      break;

    case 7:
    case 8:
      if (!(tch = GET_OPPONENT(shadow))) return FALSE;
      act("$n utters a strange mantra, and points at $N.",
       FALSE,shadow,0,tch,TO_NOTVICT);
      act("$n utters a strange mantra, and points at you.",
       FALSE,shadow,0,tch,TO_VICT);
      act("You utter a strange mantra, and point at $N.",
       FALSE,shadow,0,tch,TO_CHAR);

      act("$n falls down on the floor, shaking uncontrollably all the time.",
       FALSE,tch,0,0,TO_ROOM);
      act("You fall down on the floor, shaking uncontrollably all the time.",
       FALSE,tch,0,0,TO_CHAR);

      stop_fighting(tch);
      WAIT_STATE(tch, 10+number(0,2)*PULSE_VIOLENCE);
      GET_POS(tch) = POSITION_STUNNED;
      break;

    case 9:
    case 10:
    case 11:
    case 12:
      act("$n reaches out with $s wispy tentacles and touches you.",
       FALSE,shadow,0,tch,TO_VICT);
      act("$n reaches out with $s wispy tentacles and touches $N.",
       FALSE,shadow,0,tch,TO_NOTVICT);
      act("You reach out with your wispy tentacles and touch $N.",
       FALSE,shadow,0,tch,TO_CHAR);
      send_to_char ("You feel drained of your energy.\n",tch);

      drain = number (50,75);
      drain_mana_hit_mv(shadow,tch,drain<<1,drain,0,TRUE,TRUE,FALSE);
      break;

    case 13:
    case 14:
    case 15:
      sh = read_mobile(SHADOW, VIRTUAL);
      char_to_room(sh, CHAR_REAL_ROOM(shadow));

      act("$n gestures, and $N is formed from the shadows of the room.",
       FALSE,shadow,0,sh,TO_ROOM);
      act("You gesture, and $N is formed from the shadows of the room.",
       FALSE,shadow,0,sh,TO_CHAR);

      hit (sh, tch, TYPE_HIT);
      break;
  }
  return FALSE;
}


/* adding int to count mortals with +mana in room for orb, not to split mana drained by mortals which
might have 0 mana to add to the pool - Liner 062902 */

int
count_orbfodder_room (struct char_data *ch, bool see_invis) {
  struct char_data *v;
  int num=0;

  for(v=world[CHAR_REAL_ROOM(ch)].people;v;v=v->next_in_room)
    if (IS_MORTAL(v) && GET_MANA(v)>0 && v != ch && (see_invis||CAN_SEE(ch,v)))
    num++;
  return num;
}

/* Made orb only work with CMD_USE and added a return
   TRUE Ranger - Oct 96 */
int
ShadowOrb(OBJ *orb, CHAR *ch,int cmd,char *arg) {
  CHAR *v;
  char buf[MAX_INPUT_LENGTH];
  int ppl = 0,drain = 0,i,j, ether = 0, num = 0;  /* added ether int, Linerfix 062902 */

  if(cmd==MSG_TICK) {
    if(orb->obj_flags.value[3]>0) orb->obj_flags.value[3]--;
    return FALSE;
  }

  if(cmd==MSG_OBJ_ENTERING_GAME) {
    if(is_number(arg)) i=atoi(arg);
    else return FALSE;
    j=orb->obj_flags.value[3]*60; /* Secs left to recharge */
    if(i>j) orb->obj_flags.value[3]=0;
    return FALSE;
  }

  if(!ch) return FALSE;
  if(cmd!=CMD_USE) return FALSE;

  one_argument(arg,buf);
  if(!isname(buf,OBJ_NAME(orb))) return FALSE;

  if(is_caster(ch) && orb == EQ(ch,HOLD)) {
    if(orb->obj_flags.value[3]) {
      send_to_char("The orb seems powerless!\n\r",ch);
      return TRUE;
    }

    if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_ORB)) {
      send_to_char("Your orb seems powerless here!\n\r",ch);
      return TRUE;
    }

    ppl = count_orbfodder_room(ch,TRUE); /* was count_mortals_room, this way doesn't divide by mortals who might have 0 mana */

    if(ppl > 0) {
      ether = MIN(600, GET_MAX_MANA(ch) - GET_MANA(ch)); /* difference to max or 600 total */
      ether = MAX(ether,0);                     /* makes sure that it's not going to be negative */
      drain = (int) (ether/ppl);  /* Linerfix 062902 - making the orb drain not > than taken to fill ch's mana */
    /*  drain = (int) (ETHER_POWER/ppl); */
    } else {
      send_to_char("There is no one here.\n\r",ch);
      return TRUE;
    }

    act("$p flashes once and you can feel the energy flowing in to you.",
     FALSE,ch,orb,0,TO_CHAR);
    act("$p flashes once in $n's hands.",FALSE,ch,orb,0,TO_ROOM);

    for(v = world[CHAR_REAL_ROOM(ch)].people;v;v = v->next_in_room)
      if(ch != v && IS_MORTAL(v) && !IS_NPC(v))
     drain_mana_hit_mv(ch,v,drain,0,0,TRUE,FALSE,FALSE);
    if(!ch->specials.fighting || ROOM_CHAOTIC(CHAR_REAL_ROOM(ch))) num = 2;  /* Default 2 round lag in chaos or out of fight */
      WAIT_STATE(ch, PULSE_VIOLENCE*num);
    if(IS_DAY) {
      num=48+num;
      orb->obj_flags.value[3]=num;
    } else {
      num=48-num;
      orb->obj_flags.value[3]=num;
    }
    return TRUE;
  }
  return FALSE;
}


int New_Orb_Spec(OBJ *orb, CHAR *ch,int cmd,char *arg) {
  CHAR *v;
  char buf[MAX_INPUT_LENGTH];
  int ppl=0,mobs=0,mdrain=0,hpdrain=0,i,j,num=0;

  if(cmd==MSG_TICK) {
    if(orb->obj_flags.value[3]>0) orb->obj_flags.value[3]--;
    return FALSE;
  }

  if(cmd==MSG_OBJ_ENTERING_GAME) {
    if(is_number(arg)) i=atoi(arg);
    else return FALSE;
    j=orb->obj_flags.value[3]*60; /* Secs left to recharge */
    if(i>j) orb->obj_flags.value[3]=0;
    return FALSE;
  }

  if(!ch) return FALSE;
  if(cmd!=CMD_USE) return FALSE;

  one_argument(arg,buf);
  if(!isname(buf,OBJ_NAME(orb))) return FALSE;

  if(is_caster(ch) && orb == EQ(ch,HOLD)) {
    if(orb->obj_flags.value[3]) {
      send_to_char("The darkened orb seems powerless!\n\r",ch);
      return TRUE;
    }

    if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_ORB)) {
      send_to_char("Your darkened orb seems powerless here!\n\r",ch);
      return TRUE;
    }

    ppl = count_orbfodder_room(ch,TRUE);
    mobs = count_mobs_real_room_except_followers(CHAR_REAL_ROOM(ch));

    if(ppl > 0) {
      mdrain = (int) (ETHER_POWER_NEW/ppl);
      if(mobs > 0)  hpdrain = (int) (HP_POWER_NEW/mobs);
    } else {
      send_to_char("There is no one here.\n\r",ch);
      return TRUE;
    }

    act("$p flashes once and you can feel the energy flowing in to you.",FALSE,ch,orb,0,TO_CHAR);
    act("$p flashes once in $n's hands.",FALSE,ch,orb,0,TO_ROOM);

    for(v = world[CHAR_REAL_ROOM(ch)].people;v;v = v->next_in_room) {
      if(ch != v && IS_MORTAL(v) && !IS_NPC(v))
        drain_mana_hp(ch,v,mdrain,0,TRUE,FALSE);
      if(ch != v && IS_NPC(v))
        drain_mana_hp(ch,v,0,hpdrain,FALSE,TRUE);
    }
    num=number(2,5);
    WAIT_STATE(ch, PULSE_VIOLENCE*num);
    if(IS_DAY) {
      num=38+num;
      orb->obj_flags.value[3]=num;
    } else {
      num=38-num;
      orb->obj_flags.value[3]=num;
    }
    return TRUE;
  }
  return FALSE;
}


int
ShadowCloak(OBJ *clo, CHAR *ch,int cmd,char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(!ch)
    return FALSE;
  if(cmd == CMD_USE && IS_MORTAL(ch) && is_caster(ch)) {

    if(clo != EQ(ch,WEAR_ABOUT))
      return FALSE;

    one_argument(arg,buf);
    if(isname(buf,OBJ_NAME(clo))) {
      act("$n waves $p in circles and disappears into its folds.",
       FALSE,ch,clo,0,TO_ROOM);
      act("You wave $p in circles and disappear into its folds.",
       FALSE,ch,clo,0,TO_CHAR);
      switch (number(1,2)) {
      case 1:
     spell_invisibility(LEVEL_IMM-1,ch,ch,0);
     return TRUE;
      case 2:
     spell_teleport(LEVEL_IMM-1,ch,ch,0);
     return TRUE;
      }
    }
  }
  return FALSE;
}

int
Wight (CHAR *w, CHAR *ch,int cmd,char *arg) {
  CHAR *vict;

  if (cmd) return FALSE;

  if (GET_OPPONENT(w)) {
    if (!(vict = get_random_victim(w))) return FALSE;
    act("$n touches you with its withered hand.",FALSE,w,0,vict,TO_VICT);
    act("$n touches $N with its withered hand.",FALSE,w,0,vict,TO_NOTVICT);
    act("You touch $N with your withered hand.",FALSE,w,0,vict,TO_CHAR);
    drain_life(w,vict,8);
  }
  return FALSE;
}

int
Wraith (CHAR *w, CHAR *ch,int cmd,char *arg) {
  CHAR *vict;

  if (cmd) return FALSE;

  if (GET_OPPONENT(w)) {
    if (!(vict = get_random_victim(w))) return FALSE;
    act("$n touches you with its withered hand.",FALSE,w,0,vict,TO_VICT);
    act("$n touches $N with its withered hand.",FALSE,w,0,vict,TO_NOTVICT);
    act("You touch $N with your withered hand.",FALSE,w,0,vict,TO_CHAR);
    drain_life(w,vict,14);
  }
  return FALSE;
}

int
Apparition (CHAR *app, CHAR *ch,int cmd,char *arg) {
  CHAR *tch,*next;
  void do_flee (CHAR *ch,char *arg,int cmd);

  if (cmd) return FALSE;

  if (GET_OPPONENT(app)) {
    act("$n opens her mouth in an unearthly wail...",FALSE,app,0,0,TO_ROOM);
    for (tch = world[CHAR_REAL_ROOM(app)].people; tch; tch = next) {
      next = tch->next_in_room;
      if (IS_MORTAL(tch) && !affected_by_spell(tch, SPELL_TRANQUILITY)) {
       send_to_char("...and you feel your nerves failing you.\n",tch);
       do_flee(tch,"",0);
      }
    }
  }
  return FALSE;
}

int
Skull (OBJ *skull,CHAR *ch,int cmd,char *arg) {
  if(!ch)
    return FALSE;
  if (cmd) {
    if (IS_MORTAL(ch) && skull == EQ(ch, HOLD)) {
      act("$n has sudden spasms and $e face turns blue.",FALSE,ch,0,0,TO_ROOM);
      send_to_char("You feel the evilness of the Skull.\n",ch);
      drain_mana_hit_mv(ch,ch,MAX(1,GET_MANA(ch)>>1),MAX(1,GET_HIT(ch)>>2),0,
               FALSE,FALSE,FALSE);
    }
  }
  return FALSE;
}

int
Guardian (CHAR *st, CHAR *ch,int cmd,char *arg) {

switch(cmd) {

case CMD_NORTH:
  if (!ch) return FALSE;
  if(IS_MORTAL(ch) && V_ROOM(ch) == GUARDIAN_POS && CHAR_REAL_ROOM(ch)!=NOWHERE) {
    act("$n steps forward and bashes you down with powerful blows.",
     FALSE,st,0,ch,TO_VICT);
    act("$n steps forward and bashes $N down with powerful blows.",
     FALSE,st,0,ch,TO_NOTVICT);
    act("You step forward and bash $N down with powerful blows.",
     FALSE,st,0,ch,TO_CHAR);
    hit(st,ch,TYPE_HIT);
    return TRUE;
  }
  break;

 case MSG_MOBACT:
  if(!st) return FALSE;
  if(CHAR_REAL_ROOM(st)==NOWHERE) return FALSE;
  if(GET_HIT(st) < 500 && !number(0,3)) {
   act("\n\rThe iron bands around $n start glowing in blue light,\n\rand $e seems to be whole again.\n\r",TRUE,st,0,0,TO_ROOM);
    spell_miracle(GET_LEVEL(st),st,st,0);
  }
  return FALSE;
  break;

  default:
  return FALSE;
  break;
}
return FALSE;
}

int Gloves (OBJ *g,CHAR *ch,int cmd,char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int i,j;

  if(cmd==MSG_TICK) {
    if(g->obj_flags.value[3]>0) g->obj_flags.value[3]--;
    return FALSE;
  }

  if(cmd==MSG_OBJ_ENTERING_GAME) {
    if(is_number(arg)) i=atoi(arg);
    else return FALSE;
    j=g->obj_flags.value[3]*60; /* Secs left to recharge */
    if(i>j) g->obj_flags.value[3]=0;
    return FALSE;
  }

  if (cmd != CMD_USE) return FALSE;

  if (g == EQ(ch,WEAR_HANDS)) {
    one_argument (arg,buf);

    if (isname (buf,OBJ_NAME(g))) {
      if (!g->obj_flags.value[3]) {
        act("An aura of protection flows forth from $p.",FALSE,ch,g,0,TO_ROOM);
        act("An aura of protection flows forth from $p.",FALSE,ch,g,0,TO_CHAR);
        spell_invulnerability(LEVEL_IMM-1,ch,ch,0);
        g->obj_flags.value[3]=30;
      } else {
        send_to_char ("Nothing happens.\n",ch);
      }
      return TRUE;
    }
  }
  return FALSE;
}

void death_list(CHAR *ch);
void
create_mirror_image (CHAR *vict) {
  char buf[MAX_STRING_LENGTH];
  CHAR *image;

  image = read_mobile(MIRROR_IMAGE,VIRTUAL);
  char_to_room(image,CHAR_REAL_ROOM(vict));

  GET_LEVEL(image)                = MAX(1,GET_LEVEL(vict));
  image->points.max_mana          = GET_MAX_MANA(vict);
  image->points.max_move          = GET_MAX_MOVE(vict);
  image->points.max_hit           = MAX(1,GET_MAX_HIT(vict));

  GET_HIT(image)                  = GET_MAX_HIT(image);
  GET_MANA(image)                 = GET_MAX_MANA(image);
  GET_MOVE(image)                 = GET_MAX_MOVE(image);
  GET_EXP(image)                  = 1;
  GET_SEX(image)                  = GET_SEX(vict);
  SET_BIT(image->specials.act, ACT_ISNPC);

  image->specials.alignment       = vict->specials.alignment;
  image->points.hitroll           = GET_LEVEL(image);
  if(image->player.name)
    free (image->player.name);
  sprintf (buf,"%s image mirror",CAP(GET_NAME(vict)));
  image->player.name = str_dup(buf);

  if(image->player.short_descr)
    free (image->player.short_descr);
  sprintf (buf,"The mirror image of %s",CAP(GET_NAME(vict)));
  image->player.short_descr = str_dup(buf);

  image->specials.affected_by = vict->specials.affected_by;

  if (GET_CLASS(vict) == CLASS_WARRIOR ||
      GET_CLASS(vict) == CLASS_THIEF ||
      GET_CLASS(vict) == CLASS_NOMAD ||
      GET_CLASS(vict) == CLASS_BARD ||
      GET_CLASS(vict) == CLASS_ANTI_PALADIN ||
      GET_CLASS(vict) == CLASS_PALADIN ||
      GET_CLASS(vict) == CLASS_NINJA ||
      GET_CLASS(vict) == CLASS_AVATAR) {
    SET_BIT(image->specials.affected_by,AFF_DODGE);
  }

  if (GET_CLASS(vict) == CLASS_WARRIOR ||
      GET_CLASS(vict) == CLASS_NINJA ||
      GET_CLASS(vict) == CLASS_THIEF ||
      GET_CLASS(vict) == CLASS_ANTI_PALADIN ||
      GET_CLASS(vict) == CLASS_AVATAR) {
    SET_BIT(image->specials.affected_by,AFF_DUAL);
  }

  REMOVE_BIT(image->specials.affected_by,AFF_BLIND);
  REMOVE_BIT(image->specials.affected_by,AFF_HOLD);
  REMOVE_BIT(image->specials.affected_by,AFF_CURSE);
  REMOVE_BIT(image->specials.affected_by,AFF_POISON);
  REMOVE_BIT(image->specials.affected_by,AFF_PARALYSIS);
  REMOVE_BIT(image->specials.affected_by,AFF_CHARM);
  REMOVE_BIT(image->specials.affected_by,AFF_GROUP);

  vict->new.been_killed += 1;
  if(!IS_NPC(vict) && vict->ver3.death_limit) vict->ver3.death_limit++;
  death_list(vict);
  move_eq_from_to (vict,image);
  move_inv_from_to (vict,image);

  act("...and $n appears from the mirror's inner surface.",
      FALSE,image,0,0,TO_ROOM);
  hit(image,vict,TYPE_HIT);
}

int
Mirror (OBJ *mirror, CHAR *ch,int cmd,char *arg) {
  char buf[MAX_INPUT_LENGTH];
  CHAR *v;

  if (!IS_MORTAL(ch))
    return FALSE;

  if (cmd == CMD_LOOK || cmd == CMD_EXAMINE) {
    one_argument(arg,buf);
    if (isname(buf, OBJ_NAME(mirror))) {
      act("$n has looked in to the mirror...\n...and the mirror's surface starts to swirl and darken...",FALSE,ch,0,0,TO_ROOM);
      act("You have looked in to the mirror...\n...and the mirror's surface starts to swirl and darken...",FALSE,ch,0,0,TO_CHAR);

      if (saves_spell(ch,SAVING_SPELL,GET_LEVEL(ch))) {
     send_to_char("...but nothing happens.\n",ch);
     return TRUE;
      }

      act("$n is drawn into the mirror.",FALSE,ch,0,0,TO_ROOM);
      act("You are drawn into the mirror.",FALSE,ch,0,0,TO_CHAR);

      char_from_room(ch);
      char_to_room(ch,real_room(INSIDE_MIRROR));

      act("A bright reflection of $n appears to the mirror walls.",
       FALSE,ch,0,0,TO_ROOM);
      act("You appear in a blinding flash to a room of mirrors.",
       FALSE,ch,0,0,TO_CHAR);

      for (v=world[CHAR_REAL_ROOM(ch)].people;v;v=v->next_in_room)
     if (IS_MORTAL(v) && v != ch)
       spell_blindness(LEVEL_IMM-1,v,v,0);

      do_look(ch,"",CMD_LOOK);
      create_mirror_image(ch);

      return TRUE;
    }
  }
  return FALSE;
}

int
MirrorImage (CHAR *image, CHAR *ch,int cmd,char *arg) {
  void die(CHAR *ch);

  if (cmd != CMD_NORTH) return FALSE;
  if (V_ROOM(ch) != INSIDE_MIRROR) return FALSE;

  if(!(image=get_ch_room(MIRROR_IMAGE,CHAR_REAL_ROOM(ch)))) return FALSE;

  if (is_wearing_obj(image,BLACK_CLOTH,HOLD)) {
    act("$n explodes in a blinding flash of light.",FALSE,image,0,0,TO_ROOM);
    die(image);
    return FALSE;
  }

  act("$n pushes you back from the doorway.",FALSE,image,0,ch,TO_VICT);
  act("$n pushes $N back from the doorway.",FALSE,image,0,ch,TO_NOTVICT);
  return FALSE;
}

int
MirrorRoom (int room, CHAR *ch,int cmd,char *arg) {
  if(!ch)
    return FALSE;
  if (!number(0,19)) do_look(ch,"mirror",CMD_LOOK);
  return FALSE;
}


int uber_dream_shadow(CHAR *uber, CHAR *ch, int cmd, char *arg) {
  CHAR *tch, *vict, *next_vict;
  int drain = 0;
  OBJ *light;
  char buf[MAX_STRING_LENGTH];
  struct affected_type_5 af;

  if (cmd==MSG_DIE) {
  // on boss death reward AQP
    sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
    send_to_room(buf, CHAR_REAL_ROOM(uber));
    for(vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
    {
      next_vict = vict->next_in_room;
      if ( IS_NPC(vict) || !IS_MORTAL(vict) ) continue;
      int reward = 6;
      sprintf(buf, "You are awarded with %d quest %s for the kill.\n\r", reward, reward > 1 ? "points" : "point");
      send_to_char(buf, vict);
      vict->ver3.quest_points += reward;
    }
    return FALSE;
  }

  if( cmd!=MSG_MOBACT || chance(35) || !(tch = get_random_victim(uber)) ) return FALSE;

  switch (number(0,12)) {
    case 0:
    case 1:
    case 2: // Steal light source
      act("$n swoops toward you and plunges your world into darkness.", FALSE,uber,0,tch,TO_VICT);
      act("$n swoops toward $N and plunges $M into darkness.", FALSE,uber,0,tch,TO_NOTVICT);
      act("You descend upon $N and plunge $M into darkness.", FALSE,uber,0,tch,TO_CHAR);
      if (EQ(tch, WEAR_LIGHT)) {
        light = tch->equipment[WEAR_LIGHT];
        sprintf(buf, "WIZINFO: %s stole %s's light: %s", GET_SHORT(uber), GET_NAME(tch), OBJ_SHORT(light));
        log_s(buf);
        light->log=1;
        obj_to_char(unequip_char(tch, WEAR_LIGHT), uber);
        save_char(tch, NOWHERE);
        act("Light is suddenly and simply absent from your world.  Darkness washes over you, darker than a Black Worm's tookus on a moonless prairie night.", FALSE,uber,0,tch,TO_VICT);
      }

      if (affected_by_spell( tch, SPELL_INFRAVISION )) {
        affect_from_char(tch, SPELL_INFRAVISION);
      }
      if (affected_by_spell( tch, SPELL_PERCEIVE )) {
        affect_from_char(tch, SPELL_PERCEIVE);
      }
      af.type = SPELL_BLINDNESS;
      af.location = APPLY_HITROLL;
      af.modifier = -8;
      af.bitvector = AFF_BLIND;
      af.bitvector2 = 0;
      af.duration = 2;
      affect_to_char (tch, &af);
      break;
    case 3:
    case 4:
    case 5: // Create an image
      act("A shadowy appendage coalesces, constricts around $N and forces $M to face the mirror.", FALSE,uber,0,tch,TO_NOTVICT);
      act("With a force of will you extend your presence into a lithe appendange which entwines $N and forces $M to face the mirror.", FALSE,uber,0,tch,TO_CHAR);

      if (!affected_by_spell( tch, SPELL_BLINDNESS )) {
        act("A shadowy appendage forms before your eyes, binds your limbs and body, and forces you to face the mirror.", FALSE,uber,0,tch,TO_VICT);
        create_mirror_image(tch);
      }
      else { // if you can't see, you get a free pass
        act("Stricken with blindness, you have no warning before a force suddenly binds your limbs and shoves you against a wall.", FALSE,uber,0,tch,TO_VICT);
      }
      break;
    case 6:
    case 7:
    case 8: // Cast a spell - poison smoke, cloud of confusion, or incendiary cloud
    send_to_room("An assault of sound, some nether language, echoes around the room.\n\r", CHAR_REAL_ROOM(uber));
      act("The sound itself seems to form into a cloud of shadow which envelops $N briefly, before dissipating.", FALSE,uber,0,tch,TO_NOTVICT);
      act("The sound itself seems to form into a cloud of shadow which envelops you briefly, before dissipating.", FALSE,uber,0,tch,TO_VICT);
      switch (number(1,3)) {
        case 1:
          if (!affected_by_spell( tch, SPELL_INCENDIARY_CLOUD )) {
            af.type = SPELL_INCENDIARY_CLOUD;
            af.duration = 8;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = 0;
            af.bitvector2 = 0;
            affect_to_char(tch, &af);
            break;
          } // carry on to SPELL_CLOUD_CONFUSION if affected by SPELL_INCENDIARY_CLOUD
        case 2:
          if(!affected_by_spell( tch, SPELL_CLOUD_CONFUSION )) {
            af.type = SPELL_CLOUD_CONFUSION;
            af.location = APPLY_HITROLL;
            af.duration = number(2,6);
            af.modifier = -(number(5, 8));
            if (IS_NIGHT) af.modifier--;
            af.bitvector  = 0;
            af.bitvector2 = 0;
            affect_to_char(tch, &af);
            break;
          }  // carry on to SPELL_POISON  if affected by SPELL_CLOUD_CONFUSION
        case 3:
          af.type = SPELL_POISON;
          af.duration = 111;
          af.modifier = -5;
          af.location = APPLY_STR;
          af.bitvector = AFF_POISON;
          af.bitvector2 = 0;
          affect_to_char(tch, &af);
          send_to_char("You feel sick throughout your body and soul.\n\r", tch);
          break;
        default:
          break;
      }
      break;
    case 9:
    case 10:
    case 11:
    case 12: // Drain and self-heal
      act("$n spears you with a wispy tentacle which wriggles momentarily before retreating back into the shadowy mass.", FALSE,uber,0,tch,TO_VICT);
      act("$n spears $N with a wispy tentacle which wriggles momentarily before retreating back into the shadowy mass.", FALSE,uber,0,tch,TO_NOTVICT);
      act("You spears $N with a wispy tentacle which wriggles momentarily before retreating back into your shadowy mass.", FALSE,uber,0,tch,TO_CHAR);
      send_to_char ("You feel your life force sapped away.\n",tch);
      drain = number (200,300);
      drain_mana_hit_mv(uber,tch,drain<<1,drain,0,TRUE,TRUE,FALSE);
      GET_HIT(uber) = MIN( GET_MAX_HIT(uber), GET_HIT(uber) + (drain * 10));
      break;
    default:
      break;
  }
  return FALSE;
}
 /**********************************************************************\
 |* End Of the Special procedures for Haunted House                    *|
 \**********************************************************************/
#define ZONE_HOUSE              14000

/* Rooms */

#define MIRROR_ROOM          ITEM(ZONE_HOUSE,45)

/* Mobiles */

#define GHOST               ITEM(ZONE_HOUSE,0)
#define MARIANNE          ITEM(ZONE_HOUSE,1)
#define SUSANNE               ITEM(ZONE_HOUSE,2)
#define ANNETTE               ITEM(ZONE_HOUSE,3)
#define DREAMSHADOW          ITEM(ZONE_HOUSE,4)
#define UBERDREAM         ITEM(ZONE_HOUSE,14)
#define GUARDIAN          ITEM(ZONE_HOUSE,5)
#define WRAITH               ITEM(ZONE_HOUSE,9)
#define WIGHT               ITEM(ZONE_HOUSE,10)
#define MIRROR_IMAGE          ITEM(ZONE_HOUSE,12)

/* Objects */

#define MIRROR               ITEM(ZONE_HOUSE,0)
#define CLOAK                   ITEM(ZONE_HOUSE,2)
#define ORB               ITEM(ZONE_HOUSE,3)
#define SKULL               ITEM(ZONE_HOUSE,6)
#define GLOVES                  ITEM(ZONE_HOUSE,29)


void assign_house (void) {

  assign_mob(MARIANNE ,  Apparition);
  assign_mob(SUSANNE  , Apparition);
  assign_mob(ANNETTE  , Apparition);
  assign_mob(DREAMSHADOW,   DreamShadow);
  assign_mob(UBERDREAM,   uber_dream_shadow);
  assign_mob(WRAITH  , Wraith);
  assign_mob(WIGHT , Wight);
  assign_mob(GUARDIAN,   Guardian);
  assign_mob(MIRROR_IMAGE, MirrorImage);

  assign_obj(MIRROR, Mirror);
  assign_obj(CLOAK, ShadowCloak);
  assign_obj(ORB ,ShadowOrb);
  assign_obj(1288,ShadowOrb);
  assign_obj(14053, New_Orb_Spec);
  assign_obj(SKULL, Skull);
  assign_obj(GLOVES, Gloves);
  assign_obj(PAINTING,painting);
  assign_room(MIRROR_ROOM, MirrorRoom);
}
