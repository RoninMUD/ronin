/* ************************************************************************
*  file: mobact.c , Mobile action module.                 Part of DIKUMUD *
*  Usage: Procedures generating 'intelligent' behavior in the mobiles.    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2004/02/05 16:09:47 $
$Header: /home/ronin/cvs/ronin/mobact.c,v 2.0.0.1 2004/02/05 16:09:47 ronin Exp $
$Id: mobact.c,v 2.0.0.1 2004/02/05 16:09:47 ronin Exp $
$Name:  $
$Log: mobact.c,v $
Revision 2.0.0.1  2004/02/05 16:09:47  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>

#include "structs.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "utility.h"
#include "handler.h"
#include "cmd.h"
#include "act.h"
#include "subclass.h"

extern CHAR *character_list;
extern struct str_app_type str_app[];
extern int CHAOSMODE;

void hit(CHAR *ch, CHAR *victim, int type);
void do_kill(struct char_data *ch, char *argument, int cmd);

int test_object (OBJ *o) {
  int i;

  for (i = 0;i<MAX_OBJ_AFFECT;i++) {
    if ((o->affected[i].location == APPLY_AC &&
       o->affected[i].modifier > 0) ||
      ((o->affected[i].location == APPLY_DAMROLL ||
        o->affected[i].location == APPLY_HITROLL) &&
       o->affected[i].modifier < 0))
      return FALSE;
  }

  return TRUE;
}

void mobile_activity(CHAR *ch)
{
  CHAR *tmp_ch, *vict=NULL;
  char buf[MAX_INPUT_LENGTH];
  struct obj_data *obj, *best_obj;
  int door, found, max;
  MEMtMemoryRec *memory;
  char *dirs[] =
    {
      "north",
      "east",
      "south",
      "west",
      "up",
      "down",
      "\n"
    };

  extern int number(int min, int max);
  void wear(struct char_data *ch, struct obj_data *obj_object, int keyword);
  void do_move(CHAR *ch, char *argument, int cmd);
  void do_get(CHAR *ch, char *argument, int cmd);
  void do_wear(CHAR *ch, char *argument, int cmd);

  /* Modified for fewer indents and new aggressive order
     Ranger - May 96 */

  if (!ch) return;
  if (!IS_MOB(ch)) return;
  if (ch->specials.fighting) return;
  if (!(AWAKE(ch) || GET_POS(ch)==POSITION_STUNNED)) return;


  if (IS_SET(ch->specials.act, ACT_SCAVENGER)) {
    if (world[CHAR_REAL_ROOM(ch)].contents && !number(0,3)) {
      for (max = 1, best_obj = 0, obj = world[CHAR_REAL_ROOM(ch)].contents;
           obj; obj = obj->next_content) {
      if (CAN_GET_OBJ(ch, obj) && !CAN_WEAR(obj,ITEM_NO_SCAVENGE)) {
        if (obj->obj_flags.cost > max) {
          best_obj = obj;
          max = obj->obj_flags.cost;
        }
      }
      } /* for */

      if (best_obj) {
        /* Log getting eq from the morgue  */
        if(CHAR_VIRTUAL_ROOM(ch)==3088) {
          if(ch->master)
            sprintf(buf,"WIZ: %s (%s) gets %s in the morgue.",GET_NAME(ch),GET_NAME(ch->master),OBJ_SHORT(best_obj));
          else
            sprintf(buf,"WIZ: %s (no master) gets %s in the morgue.",GET_NAME(ch),OBJ_SHORT(best_obj));
          log_s(buf);
        }
        obj_from_room(best_obj);
        obj_to_char(best_obj, ch);
        act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);

        if (test_object (best_obj)) {
          if (best_obj->obj_flags.type_flag==ITEM_LIGHT) wear(ch, best_obj, 0);
        else if (best_obj->obj_flags.type_flag==ITEM_WEAPON) wear(ch, best_obj, 12);
        else if (best_obj->obj_flags.type_flag==ITEM_FIREWEAPON ||
               best_obj->obj_flags.type_flag==ITEM_KEY ||
               best_obj->obj_flags.type_flag==ITEM_MUSICAL) wear(ch, best_obj, 13);
        else do_wear(ch, "all", 0);
      }
      }
    }
  } /* Scavenger */

  /* Added || Riding || Flying so a riding mob could move - Ranger June 96 */
  if((!IS_SET(ch->specials.act, ACT_SENTINEL) && GET_MOVE(ch)>0) && ((GET_POS(ch)==POSITION_STANDING) ||
     (GET_POS(ch)==POSITION_RIDING) || (GET_POS(ch)==POSITION_FLYING)) &&
     ((door = number(0, 45)) <= 5) && CAN_GO(ch,door) &&
     !IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, NO_MOB) &&
     !IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, SAFE) &&
     !IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, DEATH) &&
     !IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, HAZARD) &&
     !ch->specials.rider && !IS_AFFECTED(ch, AFF_HOLD)) {
    if (ch->specials.last_direction == door) {
      ch->specials.last_direction = -1;
    } else {
      if (!IS_SET(ch->specials.act, ACT_STAY_ZONE)) {
        ch->specials.last_direction = door;
        do_move(ch, "", ++door);
      } else {
        if (world[EXIT(ch, door)->to_room_r].zone == world[CHAR_REAL_ROOM(ch)].zone) {
          ch->specials.last_direction = door;
        do_move(ch, "", ++door);
      }
      }
    }
  } /* if can go */

  /* Open doors - Ranger Feb 99 */
  if(IS_SET(ch->specials.act, ACT_OPEN_DOOR)) {
    for (door = 0; door <= 5; door++) {
      if(!chance(5)) continue;
      if (!EXIT(ch, door)) continue;
      if(!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) continue;
      if(!EXIT(ch, door)->keyword) continue;
      sprintf(buf," %s %s",EXIT(ch,door)->keyword,dirs[door]);
      if(IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
        do_open(ch,buf,CMD_OPEN);
        break;
      }
      else {
        do_close(ch,buf,CMD_CLOSE);
        break;
      }
    }
  }

/* Added new aggressive flags and added the if(!found) check
   also reordered aggressiveness.  Also added the addition
   of EVIL/NEUT/GOOD aggressive within all others.  Basically,
   this whole section has been modified, hopefully for the
   better. Mob attack is placed at the end as well.
   Ranger - May 96

  AGGs LEADER and RANDOM added Sept 96 - Ranger

  Check for any mortals in the room - Jan 97 - Ranger
*/

  /* Make sure mob didn't die from moving. Ranger - April 98 */
  if(CHAR_REAL_ROOM(ch)==NOWHERE) return;

  if(!count_mortals_real_room(CHAR_REAL_ROOM(ch))) return;

  found=FALSE;
  if (IS_SET(ch->specials.act, ACT_SUBDUE) || IS_AFFECTED(ch,AFF_HOLD)) return;

  if ((!found) && IS_SET(ch->specials.act,ACT_AGGLEADER)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
    }
  } /* if(!found) */

  if ((!found) && IS_SET(ch->specials.act,ACT_AGGRANDOM)) {
    if((tmp_ch = get_random_victim(ch))) {
      vict = tmp_ch;
      found=TRUE;
    }
  }

  /* 1 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGCL)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_CLERIC) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;


      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* 2 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGMU)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_MAGIC_USER) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;


      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* 3 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGPA)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_PALADIN) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;


      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* 4 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGNI)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_NINJA) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;


      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* 5 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGAP)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_ANTI_PALADIN) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* 6 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGCO)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_COMMANDO) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;


      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* 7 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGBA)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_BARD) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* 8 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGTH)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_THIEF) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* 9 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGNO)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_NOMAD) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* 10 */
  if ((!found) && IS_SET(ch->specials.act,ACT_AGGWA)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_CLASS(tmp_ch) != CLASS_WARRIOR) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  if ((!found) && IS_SET(ch->specials.act,ACT_AGGEVIL)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (!IS_EVIL(tmp_ch)) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      break;
    }
  } /* if(!found) */

  if ((!found) && IS_SET(ch->specials.act,ACT_AGGGOOD)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (!IS_GOOD(tmp_ch)) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      break;
    }
  } /* if(!found) */

  if ((!found) && IS_SET(ch->specials.act,ACT_AGGNEUT)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (!IS_NEUTRAL(tmp_ch)) continue;
      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      break;
    }
  } /* if(!found) */

  if ((!found) && IS_SET(ch->specials.act,ACT_AGGRESSIVE)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {

      if (GET_LEVEL(tmp_ch) >= LEVEL_IMM) continue;
      if (IS_NPC(tmp_ch) || !CAN_SEE(ch,tmp_ch)) continue;
      if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) continue;
      if (!IS_SET(ch->specials.act2, ACT_IGNORE_SPHERE) &&
          ((IS_AFFECTED(tmp_ch, AFF_SPHERE) || IS_AFFECTED(tmp_ch, AFF_INVUL)) &&
          (GET_LEVEL(ch) <= GET_LEVEL(tmp_ch)-5)) ) continue;

      /* All checks are done so tmp_ch is a valid target */
      found=TRUE;
      vict = tmp_ch;
      if (IS_SET(ch->specials.act, ACT_AGGEVIL)) {
        if(IS_EVIL(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGGOOD)) {
        if(IS_GOOD(vict)) break;
        else continue;
      }
      else if (IS_SET(ch->specials.act, ACT_AGGNEUT)) {
        if(IS_NEUTRAL(vict)) break;
        else continue;
      }
      else break;
    }
  } /* if(!found) */

  /* End of Aggressives, if found attack the player */
  if(found) {
    do_stand(ch,"",CMD_STAND);
    if(vict->skills && (check_subclass(vict,SC_RANGER,1) || check_subclass(vict,SC_WARLORD,2))
       && !affected_by_spell(vict, SKILL_BERSERK) && !IS_AFFECTED(vict, AFF_FURY) &&
       !vict->specials.fighting && !affected_by_spell(vict, SKILL_FRENZY) &&
       number(1,121)<vict->skills[SKILL_AWARENESS].learned) {
      act("You notice $N trying to attack you and hit first!",0,vict,0,ch,TO_CHAR);
      act("$n notices your attack and hits first!",0,vict,0,ch,TO_VICT);
      act("$n notices $N's attack and hits first!",0,vict,0,ch,TO_NOTVICT);
      hit(vict,ch,0);
    }
    else {
      do_kill(ch, GET_NAME(vict), 0);
    }
    return;
  }

  if ((!found) && IS_SET(ch->specials.act, ACT_MEMORY)) {
    for (tmp_ch = world[CHAR_REAL_ROOM(ch)].people; tmp_ch;tmp_ch = tmp_ch->next_in_room) {
      for (memory = ch->specials.memory; memory != NULL; memory = memory->next) {
        if(memory->id==tmp_ch->ver3.id && CAN_SEE(ch, tmp_ch) && !IS_NPC(tmp_ch)) {
          found = TRUE;
          vict = tmp_ch;
          break;
        }
      }
      if(found) break;
    }
    if ((found) && (ch != vict)) {
      act("$n screams 'I remember the like of you!'", TRUE, ch, 0, 0, TO_ROOM);
    if(vict->skills && (check_subclass(vict,SC_RANGER,1) || check_subclass(vict,SC_WARLORD,2))
       && !affected_by_spell(vict, SKILL_BERSERK) && !IS_AFFECTED(vict, AFF_FURY) &&
       !vict->specials.fighting && !affected_by_spell(vict, SKILL_FRENZY) &&
       number(1,121)<vict->skills[SKILL_AWARENESS].learned) {
        act("You notice $N trying to attack you and hit first!",0,vict,0,ch,TO_CHAR);
        act("$n notices your attack and hits first!",0,vict,0,ch,TO_VICT);
        act("$n notices $N's attack and hits first!",0,vict,0,ch,TO_NOTVICT);
        hit(vict,ch,0);
      }
      else {
        hit(ch, vict, 0);
      }
    }
  }
}

