/*
   ///   Special Procedure Module                   Orig. Date 22-01-1994
   ///                                              Last Modif 25-15-2000
   ///
   ///   Spec.elf.c --- Specials for The Elven Village and The Forbidden Maze
   ///
   ///   Designed and coded by Kafka of Imperial (kafka@modeemi.cs.tut.fi)
   ///   Using this code is not allowed without permission from originator.
   ///
   ///   Mod by Lem May 25, 2000 -- added venom ring for BM
   ///
   */

/*
$Author: ronin $
$Date: 2005/01/21 14:55:28 $
$Header: /home/ronin/cvs/ronin/spec.elf.c,v 2.2 2005/01/21 14:55:28 ronin Exp $
$Id: spec.elf.c,v 2.2 2005/01/21 14:55:28 ronin Exp $
$Name:  $
$Log: spec.elf.c,v $
Revision 2.2  2005/01/21 14:55:28  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.1  2004/06/30 19:57:28  ronin
Update of saving throw routine.

Revision 2.0.0.1  2004/02/05 16:10:29  ronin
Reinitialization of cvs archives

Revision 1.3  2003/11/02  liner
Addition of log for objs lost to pudding.

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
#include "mob.spells.h"

extern struct time_info_data time_info;
extern int CHAOSMODE;

/* Zones */

#define ZONE_ELF_VILLAGE                13000
#define ZONE_FIRE                       13100
#define ZONE_WATER                      13200
#define ZONE_EARTH                      13300
#define ZONE_AIR                        13400
#define ZONE_DEMON_MAZE                 13500

/* Rooms */

#define GOLDEN_OUT                      ITEM(ZONE_DEMON_MAZE,0)
#define GOLDEN_IN                       ITEM(ZONE_DEMON_MAZE,1)

#define ENTRANCE_FIRE                   ITEM(ZONE_FIRE,0)
#define ENTRANCE_WATER                  ITEM(ZONE_WATER,0)
#define ENTRANCE_EARTH                  ITEM(ZONE_EARTH,0)
#define ENTRANCE_AIR                    ITEM(ZONE_AIR,0)
#define ELEMENTAL_CIRCLE                ITEM(ZONE_ELF_VILLAGE,99)

/* Objects */

#define STONE                     ITEM(ZONE_EARTH,1)
#define FLAME                     ITEM(ZONE_FIRE,1)
#define ICE                       ITEM(ZONE_WATER,1)
#define CLOUD                     ITEM(ZONE_AIR,1)

#define HELMET                    ITEM(ZONE_EARTH,0)
#define TORCH                     ITEM(ZONE_FIRE,0)
#define STRIP                     ITEM(ZONE_AIR,0)
#define BELT                      ITEM(ZONE_WATER,0)

#define TOTEM_OF_PLANES           ITEM(ZONE_ELF_VILLAGE,22)

#define SEED_OF_DOOM              ITEM(ZONE_DEMON_MAZE,2)
#define DEMON_BRACELET            ITEM(ZONE_DEMON_MAZE,4)
#define FLAMING_SWORD             ITEM(ZONE_DEMON_MAZE,8)
#define PLATES_OF_CREAZ           ITEM(ZONE_DEMON_MAZE,6)

#define VENOM_RING                ITEM(ZONE_ELF_VILLAGE,26)


/* Mobiles */

#define TREANT                    ITEM(ZONE_ELF_VILLAGE,17)
#define TREE                      ITEM(ZONE_ELF_VILLAGE,20)
#define BEASTMASTER               ITEM(ZONE_ELF_VILLAGE,19)
#define ELVEN_GUARD               ITEM(ZONE_ELF_VILLAGE,5)

#define LESSER_OGLOZT             ITEM(ZONE_DEMON_MAZE,0)

/* Others */

#define FIRE		          1
#define WATER		          2
#define EARTH		          3
#define AIR		            4
#define PRIME		          5

void stop_fighting (CHAR *ch);
void hit (CHAR *ch, CHAR *vict, int type);
char *one_argument(char *arg, char *buf);

bool
is_caster (CHAR *ch) {
  char a;
  a = GET_CLASS(ch);
  if (a == CLASS_CLERIC || a == CLASS_MAGIC_USER || a == CLASS_NINJA ||
      a == CLASS_PALADIN || a == CLASS_ANTI_PALADIN || a == CLASS_BARD ||
      a == CLASS_COMMANDO)
    return TRUE;
  return FALSE;
}

int venom_ring(OBJ *venom, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int i, j;
  struct affected_type_5 af;
  struct obj_data *obj, *obj2, *tmp, *tmp2, *tmp_object;
  struct char_data *tmp_char;
  int bits=0;

  if(cmd==MSG_TICK) {
    if(venom->obj_flags.value[3] > 0) {
      venom->obj_flags.value[3]--;
    }
    return FALSE;
  }

  if(cmd==MSG_OBJ_ENTERING_GAME) {
    if(is_number(arg)) {
      i = atoi(arg);
    } else {
      return FALSE;
    }
    j = venom->obj_flags.value[3]*60;

    if(i > j) {
      venom->obj_flags.value[3] = 0;
    }
    return FALSE;
  }

  if (!ch) return FALSE;
  if (cmd != CMD_USE) return FALSE;
  one_argument(arg,buf);
  if(!isname(buf,OBJ_NAME(venom))) return FALSE;

  if( (venom == EQ(ch,WEAR_FINGER_R)) || (venom == EQ(ch,WEAR_FINGER_L))) {
    if(venom->obj_flags.value[3]) {
      send_to_char("The ring seems powerless!\n\r",ch);
      return TRUE;
    }

    if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE)) {
      send_to_char("Your ring seems powerless here!\n\r",ch);
      return TRUE;
    }

    if(*arg) { /*parse the argument, grab the target*/
       bits = generic_find(arg, FIND_CHAR_ROOM, ch, &tmp_char, &tmp_object);
    }

    if(!(bits)) {
      if((tmp_char=ch->specials.fighting)) {
        bits = FIND_CHAR_ROOM;
      }
    }

    if(bits) {
      if(bits == FIND_CHAR_ROOM) {
	act("$n thrusts his $p at $N.", TRUE, ch, venom, tmp_char, TO_NOTVICT);
	act("You thrust your $p at $N.",FALSE,ch, venom, tmp_char, TO_CHAR);
	act("$n stabs you in the throat with $s $p.",FALSE,ch, venom, tmp_char, TO_VICT);
      }
    } else {
      send_to_char ("Use the ring on who?\n\r",ch);
      return TRUE;
    }

    if(!saves_spell(tmp_char,SAVING_PARA,GET_LEVEL(ch))) {
      if(IS_NPC(tmp_char) && IS_SET(tmp_char->specials.immune,IMMUNE_POISON)) return TRUE;
      af.type = SPELL_POISON;
      af.duration = 50;
      af.modifier = -3;
      af.location = APPLY_STR;
      af.bitvector = AFF_POISON;
      af.bitvector2 = 0;

      affect_join(tmp_char, &af, FALSE, FALSE);
      send_to_char("You feel very sick.\n\r", tmp_char);

    }
    venom->obj_flags.value[3] = 24;

    if(!(venom->equipped_by)) {
      return FALSE;
    }
    for(obj = venom->equipped_by->carrying; obj; obj = tmp) {
      tmp=obj->next_content;
      if(obj)
	if(COUNT_CONTENTS(obj)) {
	  for(obj2 = obj->contains;obj2;obj2 = tmp2) {
	    tmp2=obj2->next_content;
	    if(obj2) {
	      if(obj2->item_number_v == 13026) {
		obj2->obj_flags.value[3] = 24;
	      }
	    }
	  }
	}
      if(obj->item_number_v == 13026) {
	obj->obj_flags.value[3] = 24;
      }
    }

    return TRUE;
  }
  return FALSE;
}


int
Doom (OBJ *seed, CHAR *ch, int cmd, char *arg) {
  CHAR *tmp, *tmp_next;
  char buf[MAX_INPUT_LENGTH];
  int mana;
  int room = 0;

  if (cmd != CMD_USE) return FALSE;

  if (EQ(ch, HOLD) != seed) return FALSE;

  one_argument (arg, buf);
  if (!isname(buf, OBJ_NAME(seed))) return FALSE;

  if (!IS_MORTAL(ch) || !is_caster(ch)) {
    send_to_char ("Nothing happens.\n\r",ch);
    return TRUE;
  }

  act("You break $p and a swirling cloud of dust breaks outwards.",
      FALSE,ch,seed,0,TO_CHAR);
  act("$n breaks $p and a swirling cloud of dust breaks outwards.",
      FALSE,ch,seed,0,TO_ROOM);

  mana = GET_MANA(ch);
  GET_MANA(ch) = 0;
  WAIT_STATE(ch, 2*PULSE_VIOLENCE);

  act("You fall down to the floor, completely exhausted.",
      FALSE,ch,0,0,TO_CHAR);
  act("$n falls down to the floor, completely exhausted.",
      FALSE,ch,0,0,TO_ROOM);
  GET_POS(ch) = POSITION_STUNNED;

  room = CHAR_REAL_ROOM(ch);

  for(tmp = world[room].people; tmp; tmp = tmp_next) {
    tmp_next = tmp->next_in_room;
    if (tmp != ch) {
      send_to_char("You start to feel great pain...\n\r",tmp);
      if (IS_MORTAL(tmp) && !ROOM_CHAOTIC(room))
	drain_mana_hit_mv(ch,tmp,0,mana,0,FALSE,FALSE,FALSE);
      else if (IS_MOB(tmp))
	damage (ch, tmp, mana*10, TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
  }
  unequip_char (ch, HOLD);
  extract_obj(seed);
  return TRUE;
}

int
DemonBracelet (OBJ *obj,CHAR *ch, int cmd, char *arg) {
  int def, found = FALSE;

  static char *bracelet[] = {
    "The bracelet draws some of your energy into itself.\n",
    "The bracelet gives some energy to you from itself.\n"
  };

  static char *reward[] = {
    "$p around your wrist changes into black dust.",
    "$p around $n's wrist changes into black dust.",
    "$p around your wrist changes into bright dust.",
    "$p around $n's wrist changes into bright dust."
  };

  if (!IS_MORTAL(ch) || !is_caster(ch))
    return FALSE;

  if (cmd == CMD_CAST && !number(0,2)) {
    if (obj == EQ(ch,WEAR_WRIST_R)) {
      found = WEAR_WRIST_R;
    } else if (obj == EQ(ch,WEAR_WRIST_L)) {
      found = WEAR_WRIST_L;
    } else return FALSE;

    def = number(0,50) - 25;

    if (def<0) send_to_char (bracelet[0],ch);
    else if (def>0) send_to_char (bracelet[1],ch);

    GET_MANA(ch) = MAX(0,GET_MANA(ch) + def);
    OBJ_SPEC(obj) -= def;

    if (OBJ_SPEC(obj) < -250) {
      unequip_char (ch, found);
      act(reward[0],FALSE,ch,obj,0,TO_CHAR);
      act(reward[1],FALSE,ch,obj,0,TO_ROOM);
/*
      Originally, this spec removed 20 perma mana, but was removed due to a math
      error that actually added 80.  For now, the item just disappears.

      sprintf(buf,"WIZINFO: ELF - Screwed %s (%d)",GET_NAME(ch),ch->points.max_mana);
      log_string(buf);
      ch->points.max_mana = MAX(0,ch->points.max_mana-500);
*/
      extract_obj(obj);
    } else if (OBJ_SPEC(obj) > 250) {
      unequip_char (ch, found);
      act(reward[2],FALSE,ch,obj,0,TO_CHAR);
      act(reward[3],FALSE,ch,obj,0,TO_ROOM);
/*
      This part added 20 perma mana, but due to miscalculation, actually added 120.

      sprintf(buf,"WIZINFO: ELF - Screwed %s (%d)",GET_NAME(ch),ch->points.max_mana);
      log_string(buf);
      ch->points.max_mana = MAX(0,ch->points.max_mana-500);
*/
      extract_obj(obj);
    }

    affect_total (ch);
  }
  return FALSE;
}

int
DemonSword (OBJ *sw, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  int found = FALSE;

  static char *sword[] = {
    "Flames burst forth from $p in your hands, burning $N badly...",
    "Flames burst forth from $p in $n's hands, burning you badly...",
    "Flames burst forth from $p in $n's hands, burning $N badly...",
    "...and $p explodes in a ball of red, hot fire.",
  };

  if (!GET_OPPONENT(ch))
    return FALSE;

  if (number(0,19)) return FALSE;

  if (sw == EQ(ch,WIELD)) {
    found = WIELD;
  } else if (sw == EQ(ch,HOLD)) {
    found = HOLD;
  } else return FALSE;

  vict = GET_OPPONENT(ch);

  act(sword[0],FALSE,ch,sw,vict,TO_CHAR);
  act(sword[1],FALSE,ch,sw,vict,TO_VICT);
  act(sword[2],FALSE,ch,sw,vict,TO_NOTVICT);

  damage (ch,vict,number(100,200),TYPE_UNDEFINED,DAM_NO_BLOCK);

  if (!number(0,9)) {
    act(sword[3],FALSE,ch,sw,vict,TO_ROOM);
    act(sword[3],FALSE,ch,sw,vict,TO_CHAR);
    unequip_char (ch, found);
    extract_obj (sw);
    damage (ch,ch,number(100,200),TYPE_UNDEFINED,DAM_NO_BLOCK);
  }
  return FALSE;
}

int
Creaz (OBJ *plate, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if (cmd != CMD_PUNCH)
    return FALSE;

  if (!GET_OPPONENT(ch) || number(0,59))
    return FALSE;

  if (plate != EQ(ch, WEAR_ARMS))
    return FALSE;

  vict = GET_OPPONENT(ch);

  act("As you punch $N, $p explodes in a ball of thunder, at $S face.",
      FALSE,ch,plate,vict,TO_CHAR);
  act("As $n punches you, $p explodes in a ball of thunder, at your face.",
      FALSE,ch,plate,vict,TO_VICT);
  act("As $n punches $N, $p explodes in a ball of thunder, at $S face.",
      FALSE,ch,plate,vict,TO_NOTVICT);
  log_f("WIZINFO: %s's arm bands of creaz explode.",GET_NAME(ch));
  spell_thunderball (LEVEL_IMM-1,ch,vict,0);
  unequip_char (ch, WEAR_ARMS);
  extract_obj(plate);
  return TRUE;
}

int
LesserOg (CHAR *og, CHAR *ch, int cmd, char *arg) {
  int drain, tmp;
  static char *drain_msg[] = {
    "As you concentrate on your spell, you feel your head explode.\n\rThere is a blinding flash and you lose your conciousness.\n\r",
    "As you gather your energies, you feel slightly drained.\n\r"
  };

  if (cmd == CMD_CAST && IS_MORTAL(ch) && is_caster(ch)) {
    tmp = number(0,10);
    drain = 2*(5-tmp)*(5-tmp);
    if (drain == 50) {
      send_to_char(drain_msg[0],ch);
      drain = drain<<1;
      GET_POS(ch) = POSITION_STUNNED;
      WAIT_STATE(ch, PULSE_VIOLENCE);
    } else {
      send_to_char (drain_msg[1],ch);
    }
    drain_mana_hit_mv (og,ch,drain+10,0,0,FALSE,FALSE,FALSE);
  }
  return FALSE;
}

int
GreaterOg (CHAR *og, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if (cmd)
    return FALSE;

  if (!GET_OPPONENT(og))
    return FALSE;

  if (!AWAKE(og))
    return FALSE;

  vict = GET_OPPONENT(og);

  act("You punch $N right in $S face.",FALSE,og,0,vict,TO_CHAR);
  act("$n punches you right in your face.",FALSE,og,0,vict,TO_VICT);
  act("$n punches $N right in $S face.",FALSE,og,0,vict,TO_NOTVICT);
  damage (og,vict,150,SKILL_PUNCH,DAM_NO_BLOCK);
  return FALSE;
}

int
Demon (CHAR *demon, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  OBJ *ob;
  int dir;
  char buf[MAX_INPUT_LENGTH];
  bool move_ch = FALSE;

  if (cmd)
    return FALSE;

  if (!GET_OPPONENT(demon))
    return FALSE;

  vict = GET_OPPONENT(demon);

  if (EQ(vict,WIELD) && number(0,1)) {
    ob = unequip_char (vict, WIELD);
    act("With a swift snap of $s tail, $n sends\n\r$p flying from $N's hands.",
	FALSE,demon,ob,vict,TO_NOTVICT);
    act("With a swift snap of $s tail, $n sends\n\r$p flying from your hands.",
	FALSE,demon,ob,vict,TO_VICT);
    act("With a swift snap of your tail, you send\n$p flying from $N's hands.",
	FALSE,demon,ob,vict,TO_CHAR);

    dir = number (1,3);

    if (CAN_GO (demon, dir))
      move_ch = TRUE;

    if (move_ch) {
      act ("$p flies out of the Vault's door...",FALSE,demon,ob,0,TO_ROOM);
      obj_to_room(ob,EXIT(demon,dir)->to_room_r);
      /* Added disarm log, Solmyr - 2009 */
      sprintf(buf, "WIZINFO: %s disarms %s's %s and puts it in room %d", GET_NAME(demon), GET_NAME(vict), OBJ_SHORT(ob), world[EXIT(demon,dir)->to_room_r].number);
      log_string(buf);
      ob->log = TRUE;
    } else
    	{
      	obj_to_room (ob, CHAR_REAL_ROOM(demon));
      	/* Added disarm log, Solmyr - 2009 */
      	sprintf(buf, "WIZINFO: %s disarms %s's %s in room %d", GET_NAME(demon), GET_NAME(vict), OBJ_SHORT(ob), world[CHAR_REAL_ROOM(demon)].number);
      	log_string(buf);
      	ob->log = TRUE;
      }

    return FALSE;
  }

  vict = get_random_victim (demon);

  if (vict && number(0,2)) {
    dir = number (1,3);
    if (CAN_GO (demon, dir))
      move_ch = TRUE;

    if (move_ch) {
      act("With a swift snap of $s tail,\n\r$n sends $N flying...",
	  FALSE,demon,0,vict,TO_NOTVICT);
      act("With a swift snap of $s tail,\n\r$n sends you flying...EEAOOWW!!",
	  FALSE,demon,0,vict,TO_VICT);
      act("With a swift snap of your tail,\n\ryou send $N flying...",
	  FALSE,demon,0,vict,TO_CHAR);

      char_from_room(vict);
      char_to_room(vict,EXIT(demon,dir)->to_room_r);

      act("$n arrives flying high, and crashes on the floor.",
	  FALSE,vict,0,0,TO_ROOM);
      WAIT_STATE(vict,3*PULSE_VIOLENCE);
      return FALSE;
    } else {
      act("With a swift snap of $s tail,\n$n sends $N flying to the wall.",
	  FALSE,demon,0,vict,TO_NOTVICT);
      act("With a swift snap of $s tail,\n$n sends you flying to the wall.",
	  FALSE,demon,0,vict,TO_VICT);
      act("With a swift snap of your tail,\nyou send $N flying to the wall.",
	  FALSE,demon,0,vict,TO_CHAR);

      if (GET_OPPONENT (vict))
	stop_fighting (vict);

      GET_POS (vict) = POSITION_STUNNED;
      WAIT_STATE (vict, 3*PULSE_VIOLENCE);
      return FALSE;
    }
  }
  return FALSE;
}

#define TOTEM_TRANSFER \
"You blink your eyes, and feel the land disappear from beneath your feet.\n\r\
 Something lifts you through the air, covering vast distances in a single\n\r\
 moment and when you open your eyes, you are somewhere else.\n\r\
 You are confused.\n\r"

int
Totem_of_Planes (OBJ *totem, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  sh_int room_to;
  static int target[] = { 1,2,3,4,0 };
  static int toroom[] = { 13100, 13200, 13300, 13400, 13099 };

  void do_look (CHAR *ch, char *arg, int cmd);

  if (cmd != CMD_USE)
    return FALSE;

  if (!IS_MORTAL(ch))
    return FALSE;

  if (GET_OPPONENT(ch))
    return FALSE;

  if (totem != EQ(ch,HOLD))
    return FALSE;

  one_argument(arg,buf);
  if (isname (buf,OBJ_NAME(totem))) {
    room_to = real_room(toroom[OBJ_SPEC(totem)]);
    OBJ_SPEC(totem) = target[OBJ_SPEC(totem)];

    act("$n vanishes in a cloud of swirling dust.",FALSE,ch,0,0,TO_ROOM);
    send_to_char (TOTEM_TRANSFER,ch);
    char_from_room(ch);
    char_to_room(ch, room_to);
    act("$n appears in a cloud of swirling dust.",FALSE,ch,0,0,TO_ROOM);
    do_look(ch,"",CMD_LOOK);
    WAIT_STATE(ch,2*PULSE_VIOLENCE);
    return TRUE;
  }
  return FALSE;
}

char check_plane_eq (CHAR *ch, int plane) {
  static char *damage_to_room[] = {
    "The primal flames burn $n badly on $s every move...",
    "The great masses of water press $n badly on $s every move...",
    "The tons of earth crush $n badly on $s every move.",
    "The wild winds tear $n badly on $s every move."
  };
  static char *damage_to_self[] = {
    "The primal flames burn you badly on your every move...",
    "The great masses of water press you on your every move...",
    "The tons of earth crush you badly on your every move.",
    "The wild winds tear you badly on your every move."
  };

  static int needed[] = { BELT, STRIP, HELMET, TORCH };
  static int we_locat[] = { WEAR_WAISTE, HOLD, WEAR_HEAD, WEAR_LIGHT };

  if (is_wearing_obj(ch, needed[plane-1], we_locat[plane-1])) return TRUE;

  act(damage_to_room[plane-1], FALSE,ch,0,0,TO_ROOM);
  act(damage_to_self[plane-1], FALSE,ch,0,0,TO_CHAR);
  if(GET_HIT(ch) > 100)
    {
      damage(ch,ch,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
  return FALSE;
}


/* Added CMD_FLEE check to prevent infinite flee/damage/flee - Ranger June 96 */
int
PlaneFire (int room, CHAR *ch, int cmd, char *arg) {
  if(!ch)
    return(FALSE);
  if (IS_MORTAL(ch) && (cmd != CMD_FLEE) ) check_plane_eq(ch, FIRE);
  return FALSE;
}

int
PlaneWater (int room, CHAR *ch, int cmd, char *arg) {
  if(!ch)
    return(FALSE);
  if (IS_MORTAL(ch) && (cmd != CMD_FLEE) ) check_plane_eq(ch, WATER);
  return FALSE;
}

int
PlaneEarth (int room, CHAR *ch, int cmd, char *arg) {
  if(!ch)
    return(FALSE);
  if (IS_MORTAL(ch) && (cmd != CMD_FLEE) ) check_plane_eq(ch, EARTH);
  return FALSE;
}

int
PlaneAir (int room, CHAR *ch, int cmd, char *arg) {
  if(!ch)
    return(FALSE);
  if (IS_MORTAL(ch) && (cmd != CMD_FLEE) ) check_plane_eq(ch, AIR);
  return FALSE;
}

int Treant(CHAR *treant, CHAR *ch, int cmd, char *arg)
{
  CHAR *tree = NULL, *vict = NULL;
  int i = 0, tree_count = 0;

  if (cmd != MSG_MOBACT)
    return FALSE;

  if (!(vict = GET_OPPONENT(treant)))
    return FALSE;

  tree_count = mob_proto_table[real_mobile(TREE)].number;

  if (tree_count < 5)
  {
    for (i = tree_count; i < 5; i++)
    {
      if (!(tree = read_mobile(TREE, VIRTUAL)))
        return FALSE;

      act("$n steps forth from the forest, to aid $N.", FALSE, tree, 0, treant, TO_ROOM);
      char_to_room(tree, CHAR_REAL_ROOM(treant));

      hit(tree, vict, TYPE_HIT);
    }
  }

  return FALSE;
}

int Tree(CHAR *tree, CHAR *ch, int cmd, char *arg)
{
  if (cmd != MSG_MOBACT)
    return FALSE;

  if (!get_ch_room(TREANT, CHAR_REAL_ROOM(tree)))
  {
    act("$n stops moving, and changes back to a normal tree in the forest.", FALSE, tree, 0, 0, TO_ROOM);
    act("You stop moving, and change back to a normal tree in the forest.", FALSE, tree, 0, 0, TO_CHAR);

    if (GET_OPPONENT(tree))
      stop_fighting(tree);

    extract_char(tree);
  }

  return FALSE;
}

void
Splash (CHAR *ch, CHAR *vict) {
  char buf[MAX_INPUT_LENGTH];
  OBJ *o;
  int i;

  act("$n hits you with a splash of acid.",FALSE,ch,0,vict,TO_VICT);
  act("$n hits $N with a splash of acid.",FALSE,ch,0,vict,TO_NOTVICT);
  damage(ch,vict,number(100,200),TYPE_UNDEFINED,DAM_NO_BLOCK);

  i = get_random_obj_eq (vict);
  if (i == -1) return;

  o = EQ(vict,i);
  unequip_char (vict,i);

  act("$p melts when the acid hits it.",FALSE,vict,o,0,TO_ROOM);
  act("$p melts when the acid hits it.",FALSE,vict,o,0,TO_CHAR);
  extract_obj (o);

  if(IS_MORTAL(vict)) {  /* Linerfix 110203 */
    sprintf(buf,"ObjLog: [ %s had %s extracted at %d ]",GET_NAME(vict),OBJ_SHORT(o),world[CHAR_REAL_ROOM(vict)].number);
    log_string(buf);
  }
}

int
Pudding (CHAR *bl, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if (cmd)
    return FALSE;

  if (GET_OPPONENT(bl)) {
    if (!(vict = get_random_victim(bl)))
      return FALSE;
    if (!number(0,2)) Splash (bl, vict);
  }
  return FALSE;
}

int
ArchDruid (CHAR *dr, CHAR *ch, int cmd, char *arg) {
  CHAR *vict, *temp, *el;

  if (cmd) return FALSE;
  if (!GET_OPPONENT(dr)) return FALSE;

  switch (number(0,3)) {

  case 0:    /* Earthquake */
    act("$n utters the words, 'Coung Ungah', and the earth shudders.",
	FALSE,dr,0,0,TO_ROOM);
    act("You utter the words, 'Coung Ungah', and the earth shudders.",
	FALSE,dr,0,0,TO_CHAR);
    for (vict = world[CHAR_REAL_ROOM(dr)].people;vict;vict = temp) {
      temp = vict->next_in_room;
      if (vict != dr && IS_MORTAL(vict))
	damage (dr,vict,number(120,180), SPELL_EARTHQUAKE,DAM_NO_BLOCK);
    }
    return FALSE;
    break;

  case 1:    /* Flamestrike */
    act("$n utters the words, 'Arleath Sox', and flames burst forward.",
	FALSE,dr,0,0,TO_ROOM);
    act("You utter the words, 'Arleath Sox', and flames burst forward.",
	FALSE,dr,0,0,TO_CHAR);
    for (vict = world[CHAR_REAL_ROOM(dr)].people;vict;vict = temp) {
      temp = vict->next_in_room;
      if (vict != dr && IS_MORTAL(vict))
	damage (dr,vict,number(120,180), SPELL_FLAMESTRIKE,DAM_FIRE);
    }
    return FALSE;
    break;

  case 2:    /* Gust of wind */
    act("$n utters the words, 'Li'ir Afaym', and the winds tear at you.",
	FALSE,dr,0,0,TO_ROOM);
    act("You utter the words, 'Li'ir Afaym', and the winds tear at you.",
	FALSE,dr,0,0,TO_CHAR);
    for (vict = world[CHAR_REAL_ROOM(dr)].people;vict;vict = temp) {
      temp = vict->next_in_room;
      if (vict != dr && IS_MORTAL(vict))
	damage (dr,vict,number(120,180), TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
    break;

  case 3:    /* Elemental */
    if (!(vict = get_random_victim(dr))) return FALSE;
    el = read_mobile ((number(0,3)*100)+ZONE_FIRE, VIRTUAL);
    char_to_room (el, CHAR_REAL_ROOM(dr));
    act ("$n utters the words, 'Conagh Furths', and $N appears.",
	 FALSE,dr,0,el,TO_ROOM);
    act ("You utter the words, 'Conagh Furths', and $N appears.",
	 FALSE,dr,0,el,TO_CHAR);
    hit (el, vict, TYPE_HIT);
    return FALSE;
    break;
  }
  return FALSE;
}

int
Beastmaster (CHAR *bm, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  OBJ *o;
  char buf[MAX_INPUT_LENGTH];

  if (cmd) {
    switch (cmd) {
    case CMD_CIRCLE:
      act("$n quickly dodges your vain attempt and takes a wild swing at you.",
	  FALSE,bm,0,ch,TO_VICT);
      act("You quickly dodge $N's vain attempt and take a wild swing at $M.",
	  FALSE,bm,0,ch,TO_CHAR);
      act("$n quickly dodges $N's vain attempt and takes a wild swing at $M.",
	  FALSE,bm,0,ch,TO_NOTVICT);
      hit (bm,ch,TYPE_HIT);
      return TRUE ;

    case CMD_PUNCH:
    case CMD_PUMMEL:
    case CMD_KICK:
    case CMD_BASH:
    case CMD_HIT:
    case CMD_KILL:
      act("$n notices your attempt and takes a wild swing at you.",
	  FALSE,bm,0,ch,TO_VICT);
      act("You notice $N's attempt and take a wild swing at $M.",
	  FALSE,bm,0,ch,TO_CHAR);
      act("$n notice $N's attempt and takes a wild swing at $M.",
	  FALSE,bm,0,ch,TO_NOTVICT);
      hit (bm,ch,TYPE_HIT);
      return FALSE;

    case CMD_CAST:
    case CMD_SONG:
    case CMD_RECITE:
      act("$n notices your attempt to use magic and swings right at you.",
	  FALSE,bm,0,ch,TO_VICT);
      act("You notice $N's attempt to use magic and swing right at $M.",
	  FALSE,bm,0,ch,TO_CHAR);
      act("$n notice $N's attempt and swings right at $M.",
	  FALSE,bm,0,ch,TO_NOTVICT);
      damage (bm, ch, number (100,200), TYPE_HIT,DAM_PHYSICAL);
      return FALSE;

    default:
      return FALSE;
    }
  }

  if (!(vict = GET_OPPONENT(bm))) return FALSE;

  switch (number(0,2)) {

  case 0:
    act("$n takes a hefty swing with at your legs.",FALSE,bm,0,vict,TO_VICT);
    act("$n takes a hefty swing at $N's legs.",FALSE,bm,0,vict,TO_NOTVICT);
    act("You take a hefty swing at $N's legs.",FALSE,bm,0,vict,TO_CHAR);
    damage (bm, vict, number(50,90),TYPE_HIT,DAM_PHYSICAL);
    break;

  case 1:
    act("$n screams loudly at you. You are confused.",FALSE,bm,0,vict,TO_VICT);
    act("You scream loudly at $N. $E is confused.",FALSE,bm,0,vict,TO_CHAR);
    act("$n screams loudly at $N. $E is confused.",FALSE,bm,0,vict,TO_NOTVICT);
    stop_fighting(vict);
    WAIT_STATE(vict, PULSE_VIOLENCE);
    break;

  case 2:
    if (EQ(vict,WIELD)) {
      o = EQ(vict, WIELD);
      act("Your weapon goes flying, as $n hits it lightly.",
	  FALSE,bm,o,vict,TO_VICT);
      act("$N's weapon goes flying, as $n hits it lightly.",
	  FALSE,bm,o,vict,TO_NOTVICT);
      act("$N's weapon goes flying, as you hit it lightly.",
	  FALSE,bm,o,vict,TO_CHAR);
      unequip_char (vict, WIELD);
      obj_to_room (o, CHAR_REAL_ROOM(vict));
	    sprintf(buf, "WIZINFO: %s disarms %s's %s at %d", GET_NAME(bm), GET_NAME(vict), OBJ_SHORT(o), world[CHAR_REAL_ROOM(bm)].number);
	    log_string(buf);
	    o->log = TRUE;
    }
    break;
  }
  return FALSE;
}

int
ElvenGuard (CHAR *guard, CHAR *ch, int cmd, char *arg) {

  if (!guard)
    return FALSE;

  return FALSE;
  /*
  CHAR *v1, *tmp, *v2 = 0;
     if (cmd==MSG_MOBACT)
     for (v1 = world[CHAR_REAL_ROOM(guard)].people;v1;v1 = tmp, v2 = 0) {
     tmp = v1->next_in_room;

     if (GET_OPPONENT(v1))
     v2 = GET_OPPONENT(v1);

     if (v2 && IS_NPC(v2) && !IS_EVIL(v2)) {
     act("$n rushes in to protect $N.",FALSE,guard,0,v2,TO_ROOM);
     act("You rush in to protect $N.",FALSE,guard,0,v2,TO_CHAR);
     stop_fighting (v1);
     stop_fighting (v2);
     hit (guard, v1, TYPE_HIT);
     return FALSE;
     }
     }
     */
  return FALSE;
}

int
GreatElemental (CHAR *elem, CHAR *ch, int cmd, char *arg) {
  if(cmd>0 && !number(0,5))
    {
      if (IS_MORTAL(ch) && !is_wearing_obj(ch,TOTEM_OF_PLANES,HOLD)) {
	send_to_room("There is a great explosion of elemental energy all over.\n\r"
		     ,CHAR_REAL_ROOM(ch));
	damage (ch,ch,500,TYPE_UNDEFINED,DAM_NO_BLOCK);
	return TRUE;
      }
    }
  return FALSE;
}

int
direction_to_golden_door (int room) {
  switch (room) {
  case GOLDEN_OUT: return NORTH;
  case GOLDEN_IN: return SOUTH;
  }
  return (-1);
}

int
is_golden_closed (CHAR *ch) {
  int dir;

  dir = direction_to_golden_door(V_ROOM(ch));

  if (IS_SET(EXIT(ch,dir)->exit_info, EX_CLOSED) &&
      IS_SET(EXIT(ch,dir)->exit_info, EX_LOCKED))
    return TRUE;
  return FALSE;
}

int
GoldenDoors (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  static int door_state = 0;
  int dir;
  extern int rev_dir[];

  static char *door_desc[] = {
    "The golden doors are tightly closed.\n",
    "You see the first quarter of a mystical circle on the golden doors.\n",
    "You see the first half of a mystical circle on the golden doors.\n",
    "You see three quarters of a mystical circle on the golden doors.\n",
    "You see the complete mystical circle on the golden doors.\nThe doors are wide open\n"
  };

  static char *door_actions[] = {
    "Nothing happens.\n",
    "You see the first quarter of the circle form on the door.\n",
    "You see the second quarter of the circle form on the door.\n",
    "You see the third quarter of the circle form on the door.\n",
    "You see the final quarter of the circle form on the door.\n"
  };

  static int needed[4] = { STONE,FLAME,ICE,CLOUD };

  /* Added: So that the doors would reset too... */
  if (cmd == MSG_ZONE_RESET) {
    door_state = 0;
  }

  if(!ch)
    return(FALSE);

  switch (cmd) {
  case CMD_LOOK:
  case CMD_EXAMINE:
    dir = direction_to_golden_door (V_ROOM(ch));
    one_argument (arg, buf);
    if (isname(buf, EXIT(ch,dir)->keyword)) {
      send_to_char (door_desc[door_state], ch);
      return TRUE;
    }
    return FALSE;

  case CMD_USE:

    /* Added: The doors close if someone uses while the doors are open... */
    if (door_state == 4) {
      door_state = 0;
      SET_BIT(world[real_room(GOLDEN_OUT)].dir_option[NORTH]->exit_info,
	      EX_CLOSED);
      SET_BIT(world[real_room(GOLDEN_OUT)].dir_option[NORTH]->exit_info,
	      EX_LOCKED);
      SET_BIT(world[real_room(GOLDEN_IN)].dir_option[SOUTH]->exit_info,
	      EX_CLOSED);
      SET_BIT(world[real_room(GOLDEN_IN)].dir_option[SOUTH]->exit_info,
	      EX_LOCKED);
      send_to_room ("Strange force slams the golden doors shut.\n\r",
		    real_room(GOLDEN_OUT));
      send_to_room ("Strange force slams the golden doors shut.\n\r",
		    real_room(GOLDEN_IN));

      return TRUE;
    }

    one_argument (arg, buf);
    if (EQ(ch,HOLD) && isname(buf, OBJ_NAME(EQ(ch,HOLD)))) {
      if (!is_wearing_obj(ch, needed[door_state], HOLD)) {
	send_to_char(door_actions[0],ch);
	return TRUE;
      }
      send_to_char (door_actions[door_state+1], ch);
      door_state++;
      if (door_state == 4) {
	dir = direction_to_golden_door (V_ROOM(ch));
	one_argument (arg, buf);
	send_to_room ("The huge golden doors open slowly and silently.\n",
		      CHAR_REAL_ROOM(ch));
	REMOVE_BIT(EXIT(ch,dir)->exit_info,EX_CLOSED);
	REMOVE_BIT(EXIT(ch,dir)->exit_info,EX_LOCKED);
	REMOVE_BIT(world[EXIT(ch,dir)->to_room_r].dir_option[rev_dir[dir]]->exit_info,EX_CLOSED);
	REMOVE_BIT(world[EXIT(ch,dir)->to_room_r].dir_option[rev_dir[dir]]->exit_info,EX_LOCKED);
      }
      return TRUE;
    }
    return FALSE;

  case CMD_CLOSE:
    if (is_golden_closed(ch)) return FALSE;
    dir = direction_to_golden_door (V_ROOM(ch));
    one_argument (arg, buf);
    if (isname(buf, EXIT(ch, dir)->keyword)) {
      door_state = 0;
      SET_BIT(world[real_room(GOLDEN_OUT)].dir_option[NORTH]->exit_info,
	      EX_CLOSED);
      SET_BIT(world[real_room(GOLDEN_OUT)].dir_option[NORTH]->exit_info,
	      EX_LOCKED);
      SET_BIT(world[real_room(GOLDEN_IN)].dir_option[SOUTH]->exit_info,
	      EX_CLOSED);
      SET_BIT(world[real_room(GOLDEN_IN)].dir_option[SOUTH]->exit_info,
	      EX_LOCKED);
      return TRUE;
    }
    break;

  case CMD_SCORE:
    if (isname ("reset", arg)) {
      door_state = 0;
      SET_BIT(world[real_room(GOLDEN_OUT)].dir_option[NORTH]->exit_info,
	      EX_CLOSED);
      SET_BIT(world[real_room(GOLDEN_OUT)].dir_option[NORTH]->exit_info,
	      EX_LOCKED);
      SET_BIT(world[real_room(GOLDEN_IN)].dir_option[SOUTH]->exit_info,
	      EX_CLOSED);
      SET_BIT(world[real_room(GOLDEN_IN)].dir_option[SOUTH]->exit_info,
	      EX_LOCKED);
      return TRUE;
    }
    return FALSE;

  default:
    return FALSE;
    break;
  }

  return FALSE;
}

int
VaultDoorsOutside (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(cmd==CMD_CLOSE) {
    if(!ch) return FALSE;
    one_argument (arg, buf);
    if(!*buf) return FALSE;
    if(strcmp(buf,"door") && strcmp(buf,"stone")) return FALSE;
    send_to_char("You can't close it, its stuck!\n\r",ch);
    return TRUE;
  }

  if(!ch) return(FALSE);
  if (!IS_SET(EXIT(ch,NORTH)->exit_info, EX_CLOSED)) {
    GoldenDoors(0, ch, CMD_SCORE, "reset");
  }
  return FALSE;
}

int
repdemon_room (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(cmd==CMD_CLOSE) {
    if(!ch) return FALSE;
    one_argument (arg, buf);
    if(!*buf) return FALSE;
    if(strcmp(buf,"door") && strcmp(buf,"stone")) return FALSE;
    send_to_char("You can't close it, its stuck!\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

  /**********************************************************************\
  |* End Of the Special procedures for Elven Elemental                  *|
  \**********************************************************************/

#define ZONE_ELF_VILLAGE        13000
#define ZONE_FIRE               13100
#define ZONE_WATER              13200
#define ZONE_EARTH              13300
#define ZONE_AIR                13400
#define ZONE_DEMON_MAZE         13500

/* Rooms */

#define GOLDEN_DOOR_OUTSIDE     ITEM(ZONE_DEMON_MAZE,0)
#define GOLDEN_DOOR_INSIDE      ITEM(ZONE_DEMON_MAZE,1)

#define DEMON_VAULT_OUTSIDE     ITEM(ZONE_DEMON_MAZE,7)
#define DEMON_VAULT_INSIDE      ITEM(ZONE_DEMON_MAZE,16)

#define ENTRANCE_FIRE           ITEM(ZONE_FIRE,0)
#define ALTAR_FIRE              ITEM(ZONE_FIRE,3)

#define ENTRANCE_WATER          ITEM(ZONE_WATER,0)
#define ALTAR_WATER             ITEM(ZONE_WATER,3)

#define ENTRANCE_EARTH          ITEM(ZONE_EARTH,0)
#define ALTAR_EARTH             ITEM(ZONE_EARTH,3)

#define ENTRANCE_AIR            ITEM(ZONE_AIR,0)
#define ALTAR_AIR               ITEM(ZONE_AIR,3)

/* Mobiles */

#define ARCHDRUID               ITEM(ZONE_ELF_VILLAGE,1)
#define ELVEN_GUARD             ITEM(ZONE_ELF_VILLAGE,5)
#define PUDDING                 ITEM(ZONE_ELF_VILLAGE,13)
#define TREANT                  ITEM(ZONE_ELF_VILLAGE,17)
#define BEASTMASTER             ITEM(ZONE_ELF_VILLAGE,19)
#define TREE                    ITEM(ZONE_ELF_VILLAGE,20)

#define ELEMENTAL_FIRE          ITEM(ZONE_FIRE,1)
#define ELEMENTAL_WATER         ITEM(ZONE_WATER,1)
#define ELEMENTAL_EARTH         ITEM(ZONE_EARTH,1)
#define ELEMENTAL_AIR           ITEM(ZONE_AIR,1)

#define LESSER_OGLOZT           ITEM(ZONE_DEMON_MAZE,0)
#define GREATER_OGLOZT          ITEM(ZONE_DEMON_MAZE,1)
#define DEMON                   ITEM(ZONE_DEMON_MAZE,2)

/* Objects */

#define TOTEM_OF_PLANES         ITEM(ZONE_ELF_VILLAGE,22)

#define SEED_OF_DOOM            ITEM(ZONE_DEMON_MAZE,2)
#define DEMON_BRACELET          ITEM(ZONE_DEMON_MAZE,4)
#define FLAMING_SWORD           ITEM(ZONE_DEMON_MAZE,8)
#define PLATES_OF_CREAZ         ITEM(ZONE_DEMON_MAZE,6)

void assign_elf (void) {
  assign_mob(LESSER_OGLOZT, LesserOg);
  assign_mob(GREATER_OGLOZT, GreaterOg);
  assign_mob(DEMON, Demon);
  assign_mob(TREANT, Treant);
  assign_mob(TREE, Tree);
  assign_mob(PUDDING, Pudding);
  assign_mob(ARCHDRUID, ArchDruid);
  assign_mob(BEASTMASTER, Beastmaster);
  assign_mob(ELVEN_GUARD, ElvenGuard);
  assign_mob(ELEMENTAL_FIRE, GreatElemental);
  assign_mob(ELEMENTAL_WATER, GreatElemental);
  assign_mob(ELEMENTAL_EARTH, GreatElemental);
  assign_mob(ELEMENTAL_AIR, GreatElemental);

  assign_room(GOLDEN_DOOR_OUTSIDE,GoldenDoors);
  assign_room(GOLDEN_DOOR_INSIDE,GoldenDoors);
  assign_room(DEMON_VAULT_OUTSIDE,VaultDoorsOutside);
  assign_room(DEMON_VAULT_INSIDE,repdemon_room);
  assign_room(ENTRANCE_FIRE,PlaneFire);
  assign_room(ENTRANCE_WATER,PlaneWater);
  assign_room(ENTRANCE_EARTH,PlaneEarth);
  assign_room(ENTRANCE_AIR,PlaneAir);
  assign_room(ALTAR_FIRE,PlaneFire);
  assign_room(ALTAR_WATER,PlaneWater);
  assign_room(ALTAR_EARTH,PlaneEarth);
  assign_room(ALTAR_AIR,PlaneAir);

  assign_obj(TOTEM_OF_PLANES,Totem_of_Planes);
  assign_obj(SEED_OF_DOOM,Doom);
  assign_obj(DEMON_BRACELET,DemonBracelet);
  assign_obj(FLAMING_SWORD,DemonSword);
  assign_obj(PLATES_OF_CREAZ,Creaz);
  assign_obj(VENOM_RING,venom_ring);
}
