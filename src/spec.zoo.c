/************************************************
 * File: specs.zoo.c
 * Date: 7/20/99
 * Author: Kyle Boyd (supreme_overlord@world-domination.net)
 * Description: This is a file for all of the special code for
 * the mobs the the revamped midgaard zoo for RoninMUD, with the
 * 2600-2699 vnum range.
 *
 * Modifications by Ranger 7/22/99
 ************************************************/

/*
$Author: ronin $
$Date: 2004/02/05 16:11:39 $
$Header: /home/ronin/cvs/ronin/spec.zoo.c,v 2.0.0.1 2004/02/05 16:11:39 ronin Exp $
$Id: spec.zoo.c,v 2.0.0.1 2004/02/05 16:11:39 ronin Exp $
$Name:  $
$Log: spec.zoo.c,v $
Revision 2.0.0.1  2004/02/05 16:11:39  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
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

/* vnum defines */
#define SAND_BEAST          2644
#define TINY_MOSQUITO       2643
#define GIANT_GREEN_PARROT  2611
#define PARROT_EGG          2604
#define SCORPION            2614
#define CHIMP               2616
#define ALLIGATOR           2645
#define SWAMP_ROOM_1        2646
#define SWAMP_ROOM_2        2647
#define FIRE_ANT            2635

int zoo_swamp_alligator_spec(CHAR *alligator, CHAR *ch, int cmd, char *arg)
{

  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;

  if(cmd==MSG_ENTER && chance(20)) {
    act("You slip and fall in the muck!",FALSE,ch,0,0,TO_CHAR);
    act("$n slips and falls in the muck!",FALSE,ch,0,0,TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
    if(!IS_SET(alligator->specials.affected_by, AFF_HIDE)) return FALSE;
    REMOVE_BIT(alligator->specials.affected_by, AFF_HIDE);
    do_kill(alligator,GET_NAME(ch),CMD_KILL);
  }
  return FALSE;
}

int zoo_sand_beast_spec(CHAR *sand_beast, CHAR *ch, int cmd, char *arg)
{
  CHAR *victim, *tmp_ch,*target=0;
  if (cmd!=MSG_MOBACT) return FALSE;

  if(GET_OPPONENT(sand_beast)) return FALSE;
  if(!IS_SET(sand_beast->specials.affected_by, AFF_HIDE)) return FALSE;

  for(victim=world[CHAR_REAL_ROOM(sand_beast)].people;victim;victim=tmp_ch) {
    tmp_ch=victim->next_in_room;
    if(GET_LEVEL(victim) >= LEVEL_IMM) continue;
    if(IS_NPC(victim)) continue;
    GET_POS(victim) = POSITION_SITTING;

    act("The sand beneath your feet moves, and suddenly you're knocked from\n\ryour feet.  The Sand Beast erupts from the ground in front of you."
        ,FALSE,victim,0,0,TO_CHAR);
    if(!CAN_SEE(sand_beast,victim)) continue;
    if((IS_AFFECTED(victim, AFF_SPHERE) || IS_AFFECTED(victim, AFF_INVUL)) &&
       GET_LEVEL(sand_beast) <= GET_LEVEL(victim)) continue;
    target=victim;
  }
  if(target) {
    REMOVE_BIT(sand_beast->specials.affected_by, AFF_HIDE);
    do_kill(sand_beast, GET_NAME(target),CMD_KILL);
  }
  return FALSE;
}

int zoo_tiny_mosquito_spec(CHAR *mosquito, CHAR *ch, int cmd, char *arg)
{
  CHAR *victim=0;

  if (cmd!=MSG_MOBACT) return FALSE;
  if(GET_OPPONENT(mosquito)) return FALSE; /* not if fighting */

  if (chance(30)) {
    victim = get_random_victim(mosquito);
    if(victim) {
      act("$n tries to suck your blood!",FALSE,mosquito,0,victim,TO_VICT);
      act("$n tries to suck $N's blood!",FALSE,mosquito,0,victim,TO_NOTVICT);
      do_kill( mosquito, GET_NAME(victim),CMD_KILL );
    }
  }
  return FALSE;
}

bool give_object_to(struct char_data *ch, struct obj_data *obj,
                    struct char_data *vict, bool showit);
int zoo_chimp_spec(CHAR *chimp, CHAR *ch, int cmd, char *argument)
{
  char *arg,buf[MAX_INPUT_LENGTH];
  OBJ *obj=0;

  if(cmd!=CMD_GIVE) return FALSE;
  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  arg=one_argument(argument,buf);
  if(!*buf) return FALSE;
  if(!(obj=get_obj_in_list_vis(ch,buf,ch->carrying))) return FALSE;
  one_argument(arg,buf);
  if(!*buf) return FALSE;
  if(strcmp(buf,"chimp") && strcmp(buf,"chimpanzee")) return FALSE;
  if(!give_object_to(ch, obj, chimp, TRUE)) return TRUE;
  if(obj->obj_flags.type_flag==ITEM_FOOD) {
    act("$n jumps around excitedly and gives you a big hug!",0,chimp,0,ch,TO_VICT);
    act("$n jumps around excitedly and gives $N a big hug!",0,chimp,0,ch,TO_NOTVICT);
    do_eat(chimp,OBJ_NAME(obj),CMD_EAT);
    return TRUE;
  }
  else {
    act("$n hates your gift. Watch OUT!",0,chimp,0,ch,TO_VICT);
    act("$n hates $N's gift. Watch OUT!",0,chimp,0,ch,TO_NOTVICT);
    do_kill(chimp,GET_NAME(ch),CMD_KILL);
    return TRUE;
  }
  return FALSE;
}

int zoo_fire_ant_spec(CHAR *ant, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;

  if(GET_OPPONENT(ant)) return FALSE;

  if(cmd!=MSG_MOBACT) return FALSE;

  for(vict = world[CHAR_REAL_ROOM(ant)].people; vict;vict = next_vict) {
    next_vict = vict->next_in_room;
    if(!IS_NPC(vict)) continue;
    if(V_MOB(ant)==V_MOB(vict)) {
      if(GET_OPPONENT(vict)) {
        act("$n assists it's comrade!",FALSE,ant,0,0,TO_ROOM);
        do_kill(ant, GET_NAME(GET_OPPONENT(vict)),CMD_KILL); /* attack whoever the ant is fighting */
        return FALSE;
      }
    }
  }
  return FALSE;
}

int zoo_scorpion_spec(CHAR *scorpion, CHAR *ch, int cmd, char *arg)
{
  CHAR *victim;

  if ( cmd != MSG_MOBACT ) return FALSE;

  if (chance(70)) return FALSE;
  if(!(victim = GET_OPPONENT(scorpion))) return FALSE;

  act("$n stabs you with its stinger!",FALSE,scorpion,0,victim,TO_VICT); /* ouch */
  act("$n stabs $N with its stinger!",FALSE,scorpion,0,victim,TO_NOTVICT);
  damage(scorpion, victim, number(4,6), TYPE_UNDEFINED,DAM_PHYSICAL);
  return FALSE;
}

int zoo_parrot_egg_spec(OBJ *egg,CHAR *ch, int cmd, char *arg)
{
  CHAR *bird;
  char buf[MAX_INPUT_LENGTH];

  if(egg->in_room==NOWHERE) return FALSE;
  if(cmd!=CMD_GET || !ch) return FALSE;

  one_argument(arg,buf);

  if(strcmp(buf,"egg")) return FALSE;

  if(!CAN_GET_OBJ(ch,egg)) return FALSE;

  bird=get_ch_room(GIANT_GREEN_PARROT, CHAR_REAL_ROOM(ch));

  if(bird) {
    act("Defending the egg, $n squawks loudly and attacks you!",FALSE,bird,0,ch,TO_VICT);
    act("Defending the egg, $n squawks loudly and attacks $N!",FALSE,bird,0,ch,TO_NOTVICT);
    do_kill(bird, GET_NAME(ch),CMD_KILL);
  }

  return FALSE;
}

/* spec assignments */
void assign_zoo(void)
{
  assign_mob(SAND_BEAST , zoo_sand_beast_spec);
  assign_mob(TINY_MOSQUITO , zoo_tiny_mosquito_spec);
  assign_mob(SCORPION , zoo_scorpion_spec);
  assign_mob(CHIMP, zoo_chimp_spec);
  assign_mob(FIRE_ANT , zoo_fire_ant_spec);
  assign_mob(ALLIGATOR, zoo_swamp_alligator_spec);
  assign_obj(PARROT_EGG, zoo_parrot_egg_spec);
}
