/* spec.vermillion.c - a part of RoninMUD
**
**        Author: Liner
**       Editors: Sumo, Ranger
**
** Creation Date:
**     Last Mod.: Apr. 24, 2001
**
**     Modified Twixt enter spec to work. was checking state==0 but it wasn't always 0.
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:11:30 $
$Header: /home/ronin/cvs/ronin/spec.vermillion.c,v 2.0.0.1 2004/02/05 16:11:30 ronin Exp $
$Id: spec.vermillion.c,v 2.0.0.1 2004/02/05 16:11:30 ronin Exp $
$Name:  $
$Log: spec.vermillion.c,v $
Revision 2.0.0.1  2004/02/05 16:11:30  ronin
Reinitialization of cvs archives

Revision 1.4  2003/02/11 05:55:03  ronin
Commented out the subclass master stuff in twixt.  Bard master now Pan.

Revision 1.3  2002/04/27 05:55:03  ronin
Fix to have twixt spec working and have twixt as subclass master.

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
#include "act.h"
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"

#define MUTANT      17008
#define TWIXT       17004
#define TWR_HALL    17012
#define LIT_CHAM    17016
#define TUNNEL_A    17021
#define TUNNEL_B    17022
#define TUNNEL_C    17023
#define BEAST       17003
#define LEDGE       17038
#define CORRIDOR    17055
#define MINION      17010
#define GUARD       17012
#define KING        17002
#define SECRET_DOOR 17067
#define TAPESTRY_OFF 17098
#define TAPESTRY_ON  17099
#define ROSE_KEY    17010
#define RED_KEY     17029
#define GRASS_KEY   17067
#define ORB_KEY     17094
#define BONE_KEY    17082
#define ROSE_ROOM   17010
#define ROSE_EX     17012
#define RED_ROOM    17029
#define RED_EX      17031
#define GRASS_ROOM  17067
#define GRASS_EX    17080
#define ORB_ROOM    17094
#define ORB_EX      17095
#define BONE_ROOM   17082
#define BONE_EX     17083
#define SLIME       17032

int key_rose_spec (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int rosekey;
  OBJ *key;

  if(!ch) return FALSE;
  rosekey = real_object(ROSE_KEY);

  if (cmd == CMD_UNLOCK) {
    one_argument(arg,buf);
    if (!*buf) return FALSE;
    if(strcmp(buf,"gate"))  return FALSE;
    if (!(key = get_obj_in_list_num (rosekey, ch->carrying))) return FALSE;
    send_to_char ("*CLICK*\n\rThe rose crumbles in your hands.\n\r",ch);
    REMOVE_BIT(world[room].dir_option[NORTH]->exit_info,EX_LOCK_15);
    REMOVE_BIT(world[room].dir_option[NORTH]->exit_info,EX_LOCKED);
    REMOVE_BIT(world[real_room(ROSE_ROOM)].dir_option[SOUTH]->exit_info,EX_LOCK_15);
    REMOVE_BIT(world[real_room(ROSE_ROOM)].dir_option[SOUTH]->exit_info,EX_LOCKED);
    act("As $n unlocks the gate the rose crumbles to dust.",FALSE,ch,0,0,TO_ROOM);
    extract_obj (key);
    return TRUE;
  }
  return FALSE;
}


int key_red_spec (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int redkey;
  OBJ *key;

  if(!ch) return FALSE;
  redkey = real_object(RED_KEY);

  if (cmd == CMD_UNLOCK) {
    one_argument(arg,buf);
    if (!*buf) return FALSE;
    if(strcmp(buf,"wall"))  return FALSE;
    if (!(key = get_obj_in_list_num (redkey, ch->carrying))) return FALSE;

    send_to_char ("*CLICK*\n\rThe key crumbles in your hands.\n\r",ch);
    REMOVE_BIT(world[room].dir_option[EAST]->exit_info,EX_LOCKED);
    REMOVE_BIT(world[real_room(RED_EX)].dir_option[WEST]->exit_info,EX_LOCKED);
    act("As $n unlocks the wall, the key crumbles to dust.",FALSE,ch,0,0,TO_ROOM);
    extract_obj (key);
    return TRUE;
  }
  return FALSE;
}

int key_orb_spec (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int orbkey;
  OBJ *key;

  if(!ch) return FALSE;
  orbkey = real_object(ORB_KEY);

  if (cmd == CMD_UNLOCK) {
    one_argument(arg,buf);
    if (!*buf) return FALSE;
    if(strcmp(buf,"door"))  return FALSE;
    if (!(key = get_obj_in_list_num (orbkey, ch->carrying))) return FALSE;

    send_to_char ("*CLICK*\n\rThe orb crumbles in your hands.\n\r",ch);
    REMOVE_BIT(world[room].dir_option[UP]->exit_info,EX_LOCKED);
    REMOVE_BIT(world[real_room(ORB_EX)].dir_option[DOWN]->exit_info,EX_LOCKED);
    act("As $n unlocks the door, the orb crumbles to dust.",FALSE,ch,0,0,TO_ROOM);
    extract_obj (key);
    return TRUE;
  }
  return FALSE;
}


int key_bone_spec (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int bonekey;
  OBJ *key;

  if(!ch) return FALSE;
  bonekey = real_object(BONE_KEY);

  if (cmd == CMD_UNLOCK) {
    one_argument(arg,buf);
    if (!*buf) return FALSE;
    if(strcmp(buf,"stone"))  return FALSE;
    if (!(key = get_obj_in_list_num (bonekey, ch->carrying))) return FALSE;

    send_to_char ("*CLICK*\n\rThe jawbone crumbles in your hands.\n\r",ch);
    REMOVE_BIT(world[room].dir_option[UP]->exit_info,EX_LOCKED);
    REMOVE_BIT(world[real_room(BONE_EX)].dir_option[DOWN]->exit_info,EX_LOCKED);
    act("As $n unlocks the stone, the jawbone crumbles to dust.",FALSE,ch,0,0,TO_ROOM);
    extract_obj (key);
    return TRUE;
  }
  return FALSE;
}


int mutant_spec(CHAR *mutant, CHAR *ch, int cmd, char *arg) {
  if(ch && cmd==MSG_ENTER && (!number(0,2))) {
    act("\n\r$n screams loudly at you!",FALSE,mutant,0,ch,TO_VICT);
  }
  return FALSE;
}

/* int subclass_master(CHAR *mob, CHAR *ch, int cmd, char *arg); */

int twixt_spec(CHAR *twixt, CHAR *ch, int cmd, char *arg) {
  static int state = 0;

/*  if(subclass_master(twixt,ch,cmd,arg)) return TRUE; */

  if(ch && cmd==MSG_ENTER) { /* Linerfix 02 - wasn't working for some reason, modified */
    if(IS_NPC(ch)) return FALSE;
    if(!state) {
     act("\n\r$n bows gracefully toward you and begins to sing.",FALSE,twixt,0,ch,TO_VICT);
     state++;
     return FALSE;
   }
  }

  if(cmd==MSG_MOBACT && state) {
    switch (state)
    {
    case 1:
      act("$n sings 'There once was a man",FALSE,twixt,0,0,TO_ROOM);
      act("             who ruled over the land",FALSE,twixt,0,0,TO_ROOM);
      act("             and the Universe entire",FALSE,twixt,0,0,TO_ROOM);
      act("             from his place on a spire.'",FALSE,twixt,0,0,TO_ROOM);
      state++;
      break;

    case 2:
      act("$n sings 'He was gentle and kind,",FALSE,twixt,0,0,TO_ROOM);
      act("             pure in body and mind.",FALSE,twixt,0,0,TO_ROOM);
      act("             And as many people said,",FALSE,twixt,0,0,TO_ROOM);
      act("             he came cloaked in red.'",FALSE,twixt,0,0,TO_ROOM);
      state++;
      break;

    case 3:
      act("$n sings 'One day an Evil came,",FALSE,twixt,0,0,TO_ROOM);
      act("             and he put up a good fight,",FALSE,twixt,0,0,TO_ROOM);
      act("             but his power was vanquished,",FALSE,twixt,0,0,TO_ROOM);
      act("             Good had lost all its might.'",FALSE,twixt,0,0,TO_ROOM);
      state++;
      break;

    case 4:
      act("$n sings 'He gave in to this Evil,",FALSE,twixt,0,0,TO_ROOM);
      act("             and one might surmise",FALSE,twixt,0,0,TO_ROOM);
      act("             that the Universe cried out",FALSE,twixt,0,0,TO_ROOM);
      act("             and bent to certain demise.'",FALSE,twixt,0,0,TO_ROOM);
      state++;
      break;

    case 5:
      act("$n sings 'A great, green magician",FALSE,twixt,0,0,TO_ROOM);
      act("             came to this great Tower,",FALSE,twixt,0,0,TO_ROOM);
      act("             and sealed from this portal",FALSE,twixt,0,0,TO_ROOM);
      act("             this corruption and power.'",FALSE,twixt,0,0,TO_ROOM);
      state++;
      break;

    case 6:
      act("$n sings 'Until a brave warrior",FALSE,twixt,0,0,TO_ROOM);
      act("             comes to end this suffering,",FALSE,twixt,0,0,TO_ROOM);
      act("             I must abide by my vow,",FALSE,twixt,0,0,TO_ROOM);
      act("             guard forever and sing.'",FALSE,twixt,0,0,TO_ROOM);
      state++;
      break;

    case 7:
      state = 0;
      break;

    }
  }
  return FALSE;
}

/* This one should work, Note its TO_CHAR in the act */
int twr_hall_spec(int twr_hall, CHAR *ch, int cmd, char *arg) {
  if(ch && cmd==MSG_ENTER) {
    /* could switch(number(0,3)) for diff messages here */
    act("\n\rA grotesque gurgling sound drifts from the west..",FALSE,ch,0,0,TO_CHAR);
    return FALSE;
  }
  return FALSE;
}

int lit_cham_spec(int lit_cham, CHAR *ch, int cmd, char *arg) {
  if (ch && cmd==MSG_ENTER) {
    act("\n\rA forlorn howl from above pierces the air.",FALSE,ch,0,0,TO_CHAR);
    return FALSE;
  }
  return FALSE;
}

int tunnel_a_spec(int tunnel_a, CHAR *ch, int cmd, char *arg) {
  OBJ *slime;
  if(ch && cmd==MSG_ENTER && !IS_NPC(ch)) {
    switch (number(1,2)) {
      case 1:
        act("\n\rA whiff of hot, stinky air rises from someone's behind.",FALSE,ch,0,0,TO_CHAR);
        break;
      case 2:
        act("\n\rSomething lands on your shoulder with a slimy plop!",FALSE,ch,0,0,TO_CHAR);
        slime=read_object(SLIME,VIRTUAL);
        obj_to_char(slime,ch);
        save_char(ch,NOWHERE);
        break;
    }
    return FALSE;
  }
  return FALSE;
}

int tunnel_c_spec(int tunnel_c, CHAR *ch, int cmd, char *arg) {
  if(ch && cmd==MSG_ENTER) {
    act("\n\rShrill laughter erupts from a dark corner to the south.",FALSE,ch,0,0,TO_CHAR);
    return FALSE;
  }
  return FALSE;
}

int beast_spec(CHAR *beast, CHAR *ch, int cmd, char *arg) {
  if(ch && cmd==MSG_ENTER) {
    act("\n\rA growing rumble issues forth from deep within the sand, and a sinister",FALSE,beast,0,ch,TO_VICT);
    act("growl echos off the chamber walls.",FALSE,beast,0,ch,TO_VICT);
    return FALSE;
  }
  return FALSE;
}

int ledge_spec(int ledge, CHAR *ch, int cmd, char *arg) {
  if(ch && cmd==MSG_ENTER) {
    act("\n\rA gust of wind billows up, almost plucking you from the thin ledge!",FALSE,ch,0,0,TO_CHAR);
    return FALSE;
  }
  return FALSE;
}

int corridor_spec(int corridor, CHAR *ch, int cmd, char *arg) {
  if(ch && cmd==MSG_ENTER) {
    act("\n\rA sweet melody drifts from the east.",FALSE,ch,0,0,TO_CHAR);
    return FALSE;
  }
  return FALSE;
}

int minion_spec(CHAR *minion, CHAR *ch, int cmd, char *arg) {
  if(ch && cmd==MSG_ENTER && (!number(0,2))) {
    act("\n\r$n screams 'You will never reach my master!'",FALSE,minion,0,ch,TO_VICT);
    act("$n suddenly lunges forward to grab you!",FALSE,minion,0,ch,TO_VICT);
    return FALSE;
  }
  if(cmd==MSG_TICK  && (!number(0,2))) {
    act("$n says 'Evil will prevail.'",FALSE,minion,0,0,TO_ROOM);
    return FALSE;
  }
  return FALSE;
}

int guard_spec(CHAR *guard, CHAR *ch, int cmd, char *arg) {
  if(ch && cmd==MSG_ENTER) {
    act("\n\r$n stands up and says 'Who goes there?'",FALSE,guard,0,ch,TO_VICT);
    return FALSE;
  }
  return FALSE;
}

int king_spec(CHAR *king, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(ch && cmd==MSG_ENTER) {
    act("\n\r$n rises from his Throne and fixes you with a deadly gaze.",FALSE,king,0,ch,TO_VICT);
    act("$n says 'So you have come to seal your fate.  So be it.'",FALSE,king,0,ch,TO_VICT);
    return FALSE;
  }
  if(cmd==MSG_MOBACT  && (!number(0,3))) {
    act("$n snarls and flails about in a blind rage!",FALSE,king,0,0,TO_ROOM);
    act("$n says 'You will never defeat me!'",FALSE,king,0,0,TO_ROOM);
    return FALSE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;

  if(!king->specials.fighting) return FALSE;

  if(!(vict=get_random_victim_fighting(king))) return FALSE;

  if(IS_NPC(vict)) return FALSE;

  if(vict==king->specials.fighting) return FALSE;

  if(number(0,5)==0) {
      act("$n gestures toward $N, sending $M soaring on a pillar of fire!",0,king,0,vict,TO_NOTVICT);
      act("$n gestures toward you, and you black out for a moment.",0,king,0,vict,TO_VICT);
      act("You gesture toward $N, sending $M soaring on a pillar of fire!",0,king,0,vict,TO_CHAR);
      damage(king,vict,100,TYPE_UNDEFINED,DAM_FIRE);
      stop_fighting(vict);
      return FALSE;
  }
  return FALSE;
}

int secret_door_spec (int room, CHAR *ch, int cmd, char *arg) {
  int grasskey;
  OBJ *key, *tmp;
  OBJ *block_on=NULL, *block_off=NULL;
  char buf[MAX_INPUT_LENGTH];
  int tapestry_on, tapestry_off;

  tapestry_on = real_object (TAPESTRY_ON);
  tapestry_off = real_object (TAPESTRY_OFF);

  one_argument (arg, buf);

  switch (cmd) {
    case CMD_MOVE:
      if(!*buf) return FALSE;
      if(!is_abbrev(buf,"tapestry")) return FALSE;

      block_on = get_obj_in_list_num(tapestry_on,world[room].contents);
      block_off = get_obj_in_list_num(tapestry_off,world[room].contents);

      if (!block_on && !block_off) {
        send_to_char ("There seems to be nothing to do.\n\r",ch);
        return TRUE;
      }

      if(block_off) {
        send_to_char("It's already moved.\n\r",ch);
        return TRUE;
      }
      if (block_on) {
        act("$n moves $p away from the wall, revealing a secret door.", FALSE, ch, block_on, ch, TO_ROOM);
        act("You move $p away from the wall, revealing a secret door.", FALSE, ch, block_on, ch, TO_CHAR);
        if (!block_off) {
          block_off = read_object (tapestry_off, REAL);
          obj_to_room (block_off, room);
        }
        extract_obj (block_on);
        return TRUE;
      }
      break;

    case CMD_OPEN:
    case CMD_CLOSE:
    case CMD_PICK:
    case CMD_KNOCK:
      if(!*buf) return FALSE;
      if (strcmp(buf,"door")) return FALSE;

      block_on = get_obj_in_list_num(tapestry_on,world[room].contents);

      if (block_on) {
        send_to_char ("You don't see that here.\n\r",ch);
        return TRUE;
      }
      break;
    case CMD_NORTH:
      block_on = get_obj_in_list_num(tapestry_on,world[room].contents);

      if (block_on) {
        send_to_char ("Alas, you cannot go that way...\n\r",ch);
        return TRUE;
      }
      break;
    case CMD_UNLOCK:
      if (!*buf) return FALSE;
      if(strcmp(buf,"door"))  return FALSE;
      block_on = get_obj_in_list_num(tapestry_on,world[room].contents);
      if (block_on) {
        send_to_char ("What are you trying to unlock?\n\r",ch);
        return TRUE;
      }
      grasskey = real_object(GRASS_KEY);
      if (!(key = get_obj_in_list_num (grasskey, ch->carrying))) return FALSE;

      send_to_char ("*CLICK*\n\rThe grass crumbles in your hands.\n\r",ch);
      REMOVE_BIT(world[room].dir_option[NORTH]->exit_info,EX_LOCKED);
      REMOVE_BIT(world[real_room(GRASS_EX)].dir_option[SOUTH]->exit_info,EX_LOCKED);
      act("As $n unlocks the door, the grass crumbles to dust.",FALSE,ch,0,0,TO_ROOM);
      extract_obj (key);
      return TRUE;
      break;
    case MSG_ZONE_RESET:
      block_on = get_obj_in_list_num(tapestry_on,world[room].contents);
      block_off = get_obj_in_list_num(tapestry_off,world[room].contents);
      if(!block_on) {
        tmp = read_object(tapestry_on, REAL);
        obj_to_room(tmp, room);
      }
      if(block_off) {
        extract_obj(block_off);
      }
      break;
  }
  return FALSE;
}

int slime_spec(OBJ *slime, CHAR *ch, int cmd, char *arg) {
  struct obj_data *obj,*obj2,*tmp,*tmp2;
  int to_slime=FALSE;
  char buf[MAX_INPUT_LENGTH];

  if(cmd==CMD_EAT) {
    if(!ch) return FALSE;
    one_argument(arg,buf);
    if(!*buf) return FALSE;
    if (!isname(buf, OBJ_NAME(slime))) return FALSE;
    act("You try to eat the slime, but it oozes through your fingers.",0,ch,0,0,TO_CHAR);
    act("$n tries to eat some slime, but it oozes through $s fingers.",0,ch,0,0,TO_ROOM);
    return TRUE;
  }

  if(cmd==MSG_TICK) {
    if(!slime->carried_by) return FALSE;
    for(obj = slime->carried_by->carrying; obj; obj = tmp) {
      tmp=obj->next_content;
      if (obj) {
        if(COUNT_CONTENTS(obj)) {
          for (obj2 = obj->contains;obj2;obj2 = tmp2) {
            tmp2=obj2->next_content;
            if(obj2) {
              if(GET_ITEM_TYPE(obj2)==ITEM_FOOD) {
                extract_obj(obj2);
                to_slime=TRUE;
              }
            }
          }
        }
        if(GET_ITEM_TYPE(obj)==ITEM_FOOD) {
          extract_obj(obj);
          to_slime=TRUE;
        }
      }
    }
    if(to_slime) send_to_char("The Slime absorbs all your food!!\n\r",slime->carried_by);
    return FALSE;
  }
  return FALSE;
}

void assign_vermillion(void) {
  assign_mob(MUTANT,       mutant_spec);
  assign_mob(KING,         king_spec);
  assign_mob(GUARD,        guard_spec);
  assign_mob(TWIXT,        twixt_spec);
  assign_mob(MINION,       minion_spec);
  assign_mob(BEAST,        beast_spec);
  assign_obj(SLIME,        slime_spec);
  assign_room(ROSE_ROOM,     key_rose_spec);
  assign_room(RED_ROOM,      key_red_spec);
  assign_room(ORB_ROOM,      key_orb_spec);
  assign_room(BONE_ROOM,     key_bone_spec);

  assign_room(TWR_HALL,    twr_hall_spec);
  assign_room(LIT_CHAM,    lit_cham_spec);
  assign_room(CORRIDOR,    corridor_spec);
  assign_room(LEDGE,       ledge_spec);
  assign_room(TUNNEL_A,    tunnel_a_spec);
  assign_room(TUNNEL_B,    tunnel_a_spec);
  assign_room(TUNNEL_C,    tunnel_c_spec);
  assign_room(SECRET_DOOR, secret_door_spec);
}



