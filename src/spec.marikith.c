/*
///   Special Procedure Module                   Orig. Date    30-05-1994
///                                              Last Modified
///
///   spec.marikith.c --- Specials for The Lair of the Hive Queen
///
///   Designed and coded by Kafka of (kafka@modeemi.cs.tut.fi)
///   Using this code is not allowed without permission from originator.
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:57 $
$Header: /home/ronin/cvs/ronin/spec.marikith.c,v 2.0.0.1 2004/02/05 16:10:57 ronin Exp $
$Id: spec.marikith.c,v 2.0.0.1 2004/02/05 16:10:57 ronin Exp $
$Name:  $
$Log: spec.marikith.c,v $
Revision 2.0.0.1  2004/02/05 16:10:57  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "cmd.h"
#include "fight.h"
#include "spec_assign.h"
#include "interpreter.h"
extern struct time_info_data time_info;
extern CHAR *character_list;
extern char *spells[];
extern OBJ  *object_list;

extern int CHAOSMODE;

void command_interpreter (CHAR *, char*);
char *one_argument(char *, char *);
void move_inv_from_to(struct char_data *fch, struct char_data *tch);

#define ZONE_MAR			2700

/* Mobiles */

#define MAR_MADMAN			ITEM (ZONE_MAR,0)
#define MAR_HIVE_QUEEN			ITEM (ZONE_MAR,1)
#define MAR_ELDER_MARIKITH              ITEM (ZONE_MAR,2)
#define MAR_MARIKITH                    ITEM (ZONE_MAR,3)
#define MAR_LARVAE                      ITEM (ZONE_MAR,4)
#define MAR_GHOST                       ITEM (ZONE_MAR,5)

#define MAR_ELDER_MARIKITH_LIMIT        3
#define MAR_MARIKITH_LIMIT              4
#define MAR_LARVAE_LIMIT                5

/* Objects */

#define MAR_MADMAN_KEY                  ITEM (ZONE_MAR,18)
#define MAR_MADMAN_KEY_LIMIT		1

#define MAR_QUEEN_EGG                   ITEM (ZONE_MAR,13)
#define MAR_EGG_LIMIT			3

#define MAR_QUEEN_STING                 ITEM (ZONE_MAR,16)
#define MAR_QUEEN_SHIELD                ITEM (ZONE_MAR,20)

#define MAR_BLOCK_ONHATCH               ITEM (ZONE_MAR,6)
#define MAR_BLOCK_OFFHATCH              ITEM (ZONE_MAR,12)
#define MAR_IRON_BAR                    ITEM (ZONE_MAR,4)
#define MAR_IRON_BAR_NAME               "bar long iron"

#define MAR_FEATHER                     ITEM (ZONE_MAR,19)
#define MAR_FEATHER_LIMIT               3
#define MAR_BARREL                      ITEM (ZONE_MAR,3)

/* Locations */

#define MAR_CELLAR                      ITEM (ZONE_MAR,10)
#define MAR_HATCH                       ITEM (ZONE_MAR,6)
#define MAR_HATCH_NAME                  "hatch metal"
#define MAR_DARK_PASSAGE                ITEM (ZONE_MAR,15)
#define MAR_PURGING_ROOM                ITEM (ZONE_MAR,33)

/* Miscellaneous */

#define MAR_QUEEN_ATTACK_CLAW		0
#define MAR_QUEEN_ATTACK_CLAW_2		1
#define MAR_QUEEN_ATTACK_CLAW_3		2
#define MAR_QUEEN_ATTACK_CLAW_4		3
#define MAR_QUEEN_ATTACK_BITE		4
#define MAR_QUEEN_ATTACK_STING		5
#define MAR_QUEEN_ATTACK_FEAR		6

#define MAR_TORCH_HOLDER_NAME           "holder torch"
#define MAR_LEVER_NAME                  "lever iron"

#define MAR_DOOR_CLOSED                 0
#define MAR_DOOR_OPEN                   1

/* Prototypes */

int mar_hive_queen (CHAR *qun, CHAR *ch, int cmd, char *arg);
int mar_elder      (CHAR *mar, CHAR *ch, int cmd, char *arg);
int mar_marikith   (CHAR *mar, CHAR *ch, int cmd, char *arg);
int mar_larvae     (CHAR *lrv, CHAR *ch, int cmd, char *arg);
int mar_madman     (CHAR *man, CHAR *ch, int cmd, char *arg);
int mar_ghost      (CHAR *gst, CHAR *ch, int cmd, char *arg);

int mar_egg        (OBJ *egg, CHAR *ch, int cmd, char *arg);
int mar_feather    (OBJ *egg, CHAR *ch, int cmd, char *arg);
int mar_shield     (OBJ *egg, CHAR *ch, int cmd, char *arg);
int mar_sting      (OBJ *egg, CHAR *ch, int cmd, char *arg);
int mar_barrel     (OBJ *obj, CHAR *ch, int cmd, char *arg);

int mar_hatch      (int room, CHAR *ch, int cmd, char *arg);
int mar_torch      (int room, CHAR *ch, int cmd, char *arg);
int mar_lever      (int room, CHAR *ch, int cmd, char *arg);
int mar_slime      (int room, CHAR *ch, int cmd, char *arg);

/* Assignment */

void assign_marikith (void)
{
  assign_mob(MAR_MADMAN      ,    mar_madman);
  assign_mob(MAR_HIVE_QUEEN   ,   mar_hive_queen);
  assign_mob(MAR_ELDER_MARIKITH,  mar_elder);
  assign_mob(MAR_MARIKITH      ,  mar_marikith);
  assign_mob(MAR_LARVAE        ,  mar_larvae);
//  assign_mob(MAR_GHOST         ,  mar_ghost);
// ^^ chained in spec.elmuseo.c
  assign_obj(MAR_QUEEN_EGG     ,  mar_egg);
  assign_obj(MAR_FEATHER       ,  mar_feather);
  assign_obj(MAR_QUEEN_SHIELD  ,  mar_shield);
  assign_obj(MAR_QUEEN_STING   ,  mar_sting);
  assign_obj(MAR_BARREL        ,  mar_barrel);

  assign_room(MAR_CELLAR       ,  mar_torch);
  assign_room(MAR_DARK_PASSAGE ,  mar_lever);
  assign_room(MAR_HATCH        ,  mar_hatch);
  assign_room(ITEM(ZONE_MAR,16),  mar_slime);
  assign_room(ITEM(ZONE_MAR,18),  mar_slime);
  assign_room(ITEM(ZONE_MAR,19),  mar_slime);
  assign_room(ITEM(ZONE_MAR,22),  mar_slime);
}

/* Functions */

int mar_barrel(OBJ *obj, CHAR *ch, int cmd, char *arg) {

  if(cmd==MSG_ZONE_RESET)
    if(obj->in_room!=NOWHERE) extract_obj(obj);
  return FALSE;
}

void
mar_to_purging_room (CHAR *ch)
{
  int purging_room = 0;

  purging_room = real_room (MAR_PURGING_ROOM);

  if (purging_room != NOWHERE)
   {
    char_from_room (ch);
    char_to_room (ch, purging_room);
   }
}

void
mar_purge(void)
{
  CHAR *tmp, *next;
  int purging_room;
  purging_room = real_room (MAR_PURGING_ROOM);

  if(purging_room!=-1)
    {
      for (tmp = world[purging_room].people;tmp;tmp=next)
        {
          next = tmp->next_in_room;
          if (IS_NPC(tmp))
	    extract_char (tmp);
        }
    }
}

int
mar_friend (CHAR *ch)
{
  int virtual;

  if (!IS_NPC (ch))
    return FALSE;
  else
    {
      virtual = V_MOB (ch);
      if (MAR_HIVE_QUEEN <= virtual && virtual <= MAR_GHOST)
        return TRUE;
    }
  return FALSE;
}

void
mar_lay_egg (CHAR *ch)
{
  OBJ *egg;
  int egg_nr;

  egg_nr = real_object (MAR_QUEEN_EGG);

  egg = read_object (egg_nr, REAL);
  obj_to_room (egg, CHAR_REAL_ROOM(ch));
  act ("$n lays $p.", TRUE, ch, egg, 0,TO_ROOM);
}

int
mar_egg_count (void)
{
  int egg_nr;

  egg_nr = real_object (MAR_QUEEN_EGG);

  return (obj_proto_table[egg_nr].number);
}

void
mar_queen_attack (CHAR *ch, CHAR *vic, int type)
{
  char buf[MAX_INPUT_LENGTH];

  switch (type)
    {
    case MAR_QUEEN_ATTACK_CLAW:
    case MAR_QUEEN_ATTACK_CLAW_2:
    case MAR_QUEEN_ATTACK_CLAW_3:
    case MAR_QUEEN_ATTACK_CLAW_4:
      act("$n slashes $N with $s long,sharp claws.",FALSE,ch,0,vic,TO_NOTVICT);
      act("$n slashes you with $s long, sharp claws.",FALSE,ch,0,vic,TO_VICT);
      act("You slash $N with your long, sharp claws.",FALSE,ch,0,vic,TO_CHAR);
      spell_poison (GET_LEVEL (ch), ch, vic, 0);
      damage (ch, vic, number (100,200), TYPE_UNDEFINED,DAM_NO_BLOCK);
      return;

    case MAR_QUEEN_ATTACK_BITE:
      act ("Suddenly, $n reaches forward and bites $N with $s huge incicors",
	   FALSE, ch, 0, vic, TO_NOTVICT);
      act ("Suddenly, $n reaches forward and bites you with $s huge incisors.",
	   FALSE, ch, 0, vic, TO_VICT);
      act ("Suddenly, you reach forward and bite $N with your huge incisors.",
	   FALSE, ch, 0, vic, TO_CHAR);
      spell_paralyze (GET_LEVEL (ch), ch, vic, 0);
      damage (ch, vic, number (200,300), TYPE_UNDEFINED,DAM_NO_BLOCK);
      return;

    case MAR_QUEEN_ATTACK_STING:
      if (!number (0,2))
	return;

      act ("$n turns slightly and stings you with $s revealed tail-sting.",
	   FALSE, ch, 0, vic, TO_VICT);
      act ("$n turns slightly and stings $N with $s revealed tail-sting.",
	   FALSE, ch, 0, vic, TO_NOTVICT);
      act ("You turn slightly and sting $N with your revealed tail-sting.",
	   FALSE, ch, 0, vic, TO_CHAR);
      damage (ch, vic, number(350,500),
	      (number(0,16) ? TYPE_UNDEFINED : TYPE_KILL),DAM_NO_BLOCK);
      return;

    case MAR_QUEEN_ATTACK_FEAR:
      if (!number (0,2))
	return;

      act ("$n stares at you, and the horrors within $s mind frighten you!",
	   FALSE, ch, 0, vic, TO_VICT);
      act ("$n stares at $N, and $E tries to flee in panic!",
	   FALSE, ch, 0, vic, TO_NOTVICT);
      act ("You stare at $N, and show $M the horrors within your mind.",
	   FALSE, ch, 0, vic, TO_CHAR);
      sprintf (buf, "flee");
      command_interpreter (vic, buf);
      return;

    default:
      return;
    }
}

CHAR*
mar_get_queen_victim (CHAR *ch)
{
  CHAR *tar = 0, *tmp;

  for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp = tmp->next_in_room)
    if ((IS_NPC (tmp) || GET_LEVEL (tmp) < LEVEL_IMM) &&
	!mar_friend(tmp))
      {
	if (!tar)
	  tar = tmp;

	if (!number (0,5))
	  tar = tmp;
      }
  return tar;
}

void
mar_vanish (CHAR *ch)
{
  CHAR *tmp;

  act ("$n vanishes into the darkness right in front of your eyes.",
       TRUE,ch,0,0,TO_ROOM);
  act ("You vanish into the darkness right in front of their eyes.",
       TRUE,ch,0,0,TO_CHAR);

  for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp=tmp->next_in_room)
    if (GET_OPPONENT(tmp) == ch)
      stop_fighting (tmp);

  SET_BIT (ch->specials.affected_by, AFF_IMINV);
}

void
mar_appear (CHAR *ch)
{
  REMOVE_BIT (ch->specials.affected_by, AFF_IMINV);
  act ("$n appears from the darkness right in front of your eyes.",
       TRUE,ch,0,0,TO_ROOM);
  act ("You appear from the darkness right in front of their eyes.",
       TRUE,ch,0,0,TO_CHAR);
}

int
mar_player_in_room (CHAR *ch)
{
  CHAR *tmp;
  for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp = tmp->next_in_room)
    if (!IS_NPC (tmp) && GET_LEVEL (tmp) < LEVEL_IMM)
      return TRUE;
  return FALSE;
}

void
mar_vanish_if_alone (CHAR *ch)
{
  if (!mar_player_in_room (ch) && !GET_OPPONENT (ch))
    mar_vanish (ch);
}

void
mar_change_to (CHAR *ch, int type)
{
  int elder_nr,marikith_nr;
  CHAR *mob = 0;

  elder_nr = real_mobile (MAR_ELDER_MARIKITH);
  marikith_nr = real_mobile (MAR_MARIKITH);

  switch (type)
    {

    case MAR_ELDER_MARIKITH:
      if (mob_proto_table[elder_nr].number < MAR_ELDER_MARIKITH_LIMIT)
	{
	  mob = read_mobile (elder_nr, REAL);
	  char_to_room (mob, CHAR_REAL_ROOM(ch));
    move_inv_from_to(ch,mob);
    mar_to_purging_room (ch);
	}
      break;

    case MAR_MARIKITH:
      if (mob_proto_table[marikith_nr].number < MAR_MARIKITH_LIMIT)
	{
	  mob = read_mobile (marikith_nr, REAL);
	  char_to_room (mob, CHAR_REAL_ROOM(ch));
    move_inv_from_to(ch,mob);
	  mar_to_purging_room (ch);
	}
      break;
    }
}

void
mar_block_check (void)
{
  OBJ *tmp, *tmp2;
  int off, on, room;

  off = real_object (MAR_BLOCK_OFFHATCH);
  on =  real_object (MAR_BLOCK_ONHATCH);
  room = real_room (MAR_HATCH);


  tmp = get_obj_in_list_num (on, world[room].contents);

  if (tmp && (tmp2 = get_obj_in_list_num (off, world[room].contents)))
    extract_obj (tmp2);
}

/* Special procedures */

int
mar_hive_queen (CHAR *queen, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = 0,*next_vict;
  int i,dam;
  int qend, spl;
  char *tmp_string;

  char *skip_spaces (char*);

  mar_purge ();

  /* New check for allowing only 10 tballs to the hive over
     30 ticks. - Every 3 ticks another tball is allowed.
     After that some serious damage is given to the tball caster
     and minor damage spread around depending on group size.

     In addition, a conflag spell gives 1/2 damage.  I'm tricking
     the spell by adding 750 hps to the hive at a conflag cast.
     Not neat, but it is simple - Ranger Nov 96 */

  if(cmd==MSG_TICK) {
    if(queen->specials.timer>0) queen->specials.timer-=1;
    return FALSE;
  }

  if (cmd==CMD_CAST) {
     if(!ch) return FALSE;
     tmp_string = str_dup (arg);
     tmp_string = skip_spaces(tmp_string);

     if (!(*tmp_string)) return FALSE;
     if (*tmp_string != '\'') return FALSE;

     for (qend=1; *(tmp_string+qend) && (*(tmp_string+qend) != '\'') ; qend++)
	*(tmp_string+qend) = LOWER(*(tmp_string+qend));

     if (*(tmp_string+qend) != '\'') return FALSE;

     spl = old_search_block(tmp_string, 1, qend-1,spells, 0);

     if(spl==SPELL_THUNDERBALL || spl==SPELL_WRATH_ANCIENTS) {
       if(queen->specials.timer>27) {
         dam = spl == SPELL_THUNDERBALL ? number(800, 1100) : number(1800, 2000);
         i=count_mortals_room(queen,FALSE);
         if(i==0) return FALSE;

         if(spl==SPELL_THUNDERBALL) {
           act("With a blood curdling scream $n deflects the thunderball back at $N!", 0, queen, 0, ch, TO_NOTVICT);
           act("With a blood curdling scream you deflect the thunderball back at $N!", 0, queen, 0, ch, TO_CHAR);
           act("With a blood curdling scream $n deflects the thunderball back at you!", 0, queen, 0, ch, TO_VICT);
         } else {
           act("With a blood curdling scream $n deflects the wrath back at $N!", 0, queen, 0, ch, TO_NOTVICT);
           act("With a blood curdling scream you deflect the wrath back at $N!", 0, queen, 0, ch, TO_CHAR);
           act("With a blood curdling scream $n deflects the wrath back at you!", 0, queen, 0, ch, TO_VICT);
         }
         damage(queen, ch, dam*2/3, SPELL_THUNDERBALL,DAM_SOUND);

         for(vict = world[queen->in_room_r].people; vict;vict = next_vict) {
           next_vict = vict->next_in_room;
           if(!IS_NPC(vict) && IS_MORTAL(vict) && ch!=vict) {
             send_to_char("You are caught in the backlash!\n\r",vict);
             damage(queen,vict,dam/3/(i-1),SPELL_THUNDERBALL,DAM_SOUND);
           }
         }
         return TRUE;
       }
       queen->specials.timer+=3;
       return FALSE;
     }
     if(spl==SPELL_CONFLAGRATION) {
        if (IS_AFFECTED(queen, AFF_SANCTUARY)) GET_HIT(queen)+=375;
        else GET_HIT(queen)+=750;
        GET_HIT(queen)=MIN(GET_HIT(queen),GET_MAX_HIT(queen));
     }
     return FALSE;
  }


  if (cmd) return FALSE;

  if (GET_OPPONENT (queen))
    {
      if (IS_AFFECTED(queen,AFF_IMINV))
	mar_appear (queen);

      for (i = MAR_QUEEN_ATTACK_CLAW;i <= MAR_QUEEN_ATTACK_FEAR;i++)
	if (!number(0,3) || !(vict = mar_get_queen_victim (queen)))
	  continue;
	else
	  mar_queen_attack (queen, vict, i);

      return FALSE;
    }
  else if (number (0,1) && (vict = mar_get_queen_victim (queen)))
    {
      if (IS_AFFECTED(queen,AFF_IMINV))
	mar_appear (queen);

      mar_queen_attack (queen, vict, MAR_QUEEN_ATTACK_STING);
      hit (queen, vict, TYPE_HIT);
      return FALSE;
    }

  if (mar_egg_count () < MAR_EGG_LIMIT && !number (0,9))
    {
      mar_lay_egg (queen);
      return FALSE;
    }

  if (!IS_AFFECTED(queen, AFF_IMINV))
    mar_vanish_if_alone (queen);

  return FALSE;
}

int
mar_elder (CHAR *mar, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = 0;
  char buf[MAX_INPUT_LENGTH];

  if (CHAR_REAL_ROOM(mar) == 0)
    {
      extract_char (mar);
      return (cmd ? FALSE : TRUE);
    }

  if (cmd == MSG_DIE)
    {
      act ("$n screams and falls down to the ground.", FALSE,mar,0,0,TO_ROOM);
      send_to_char ("You scream and fall down to the ground in the moment of death.\n\r",mar);

      for (vict = character_list;vict;vict = vict->next)
	{
	  if (GET_ZONE(vict) == GET_ZONE(mar) &&
	      CHAR_REAL_ROOM(mar) != CHAR_REAL_ROOM(vict))
	    send_to_char ("You hear a blood curdling scream!\n\r",vict);

	  if (CHAR_REAL_ROOM(vict)== CHAR_REAL_ROOM(mar) &&
	      (IS_NPC (vict) || GET_LEVEL (vict) < LEVEL_IMM) &&
	      !mar_friend(vict))
	    spell_paralyze (GET_LEVEL (ch), ch, vict, 0);
	}
      return TRUE;
    }

  if (cmd)
    return FALSE;

  if (GET_OPPONENT (mar))
    {
      if (IS_AFFECTED(mar, AFF_IMINV))
	mar_appear (mar);
      mar_queen_attack (mar, GET_OPPONENT (mar), number (0,4));
      return FALSE;
    }
  else if (number (0,2) && IS_AFFECTED (mar, AFF_IMINV))
    {
      if (!(vict = mar_get_queen_victim (mar)))
	return FALSE;

      mar_appear (mar);
      sprintf (buf, "kill %s", GET_NAME(vict));
      command_interpreter (mar, buf);
      return FALSE;
    }

  if (!IS_AFFECTED(mar, AFF_IMINV))
    mar_vanish_if_alone (mar);
  return FALSE;
}

int
mar_marikith (CHAR *mar, CHAR *ch, int cmd, char *arg)
{
  if (CHAR_REAL_ROOM(mar) == 0)
    {
      extract_char (mar);
      return FALSE;
    }

  if (cmd)
    return FALSE;

  if (GET_OPPONENT (mar))
    {
      mar_queen_attack (mar, GET_OPPONENT (mar), number (0,4));
      return FALSE;
    }

  if (!number (0,19))
    {
      mar_change_to (mar, MAR_ELDER_MARIKITH);
      return FALSE;
    }

  return FALSE;
}

int
mar_larvae (CHAR *larvae, CHAR *ch, int cmd, char *arg)
{
  if (CHAR_REAL_ROOM(larvae) == 0)
    {
      extract_char (larvae);
      return FALSE;
    }

  if (cmd)
    return FALSE;

  if (!GET_OPPONENT (larvae) && !number (0,19))
    {
      mar_change_to (larvae, MAR_MARIKITH);
      return FALSE;
    }

  return FALSE;
}

int
mar_madman (CHAR *man, CHAR *ch, int cmd, char *arg)
{
  static int state = 0;
  int key_nr;
  OBJ *key;
  char buf[MAX_INPUT_LENGTH];

  key_nr = real_object (MAR_MADMAN_KEY);

  mar_block_check();

  if (cmd)
    return FALSE;

  if(GET_OPPONENT (man) || GET_POS(man) < POSITION_RESTING)
    {
      state = 0;
      return FALSE;
    }

  switch (state++)
    {

    case 0:
      sprintf (buf,"say From the blackness of the night...");
      command_interpreter (man,buf);
      break;

    case 1:
      sprintf (buf, "say those creatures of the darkness, they came...");
      command_interpreter (man,buf);
      break;

    case 2:
      sprintf (buf, "say Gone forever is the peace...");
      command_interpreter (man,buf);
      break;

    case 3:
      sprintf (buf, "say Weakened is my mortal frame and mind...");
      command_interpreter (man,buf);
      break;

    case 4:
      sprintf (buf, "say Never again shall I claim a victory!");
      command_interpreter (man,buf);
      break;

    case 5:
      sprintf (buf, "say Destroy the evil, and save the world of man...");
      command_interpreter (man,buf);
      break;

    case 6:
      sprintf (buf, "say Beware the darkness!");
      command_interpreter (man,buf);
      break;

    case 7:
      sprintf (buf, "say Venture down into the earth with great care, and...");
      command_interpreter (man,buf);
      break;

    case 8:
      sprintf (buf, "say Be careful, down there even the darkness has eyes!");
      command_interpreter (man,buf);
      break;

    case 9:
      sprintf (buf, "say The key...the key!");
      command_interpreter (man,buf);
      break;

    case 10:
      sprintf (buf, "say Use this on the lock, but then you are on your own!");
      command_interpreter (man,buf);
      break;

    case 11:
      if (obj_proto_table[key_nr].number < MAR_MADMAN_KEY_LIMIT)
	{
	  key = read_object (key_nr, REAL);
	  obj_to_room (key, CHAR_REAL_ROOM(man));
	  act ("$n drops $p.",FALSE,man,key,0,TO_ROOM);
	}
      break;

    case 12:
      sprintf (buf, "sleep");
      command_interpreter (man,buf);
      break;
    }
  return FALSE;
}

int
mar_hatch (int room, CHAR *ch, int cmd, char *arg) {
  OBJ *block_on, *block_off;
  char buf[MAX_INPUT_LENGTH];
  int block1, block2;

  if(!ch) return(FALSE);

  block1 = real_object (MAR_BLOCK_ONHATCH);
  block2 = real_object (MAR_BLOCK_OFFHATCH);

  if (cmd != CMD_USE && cmd != CMD_OPEN && cmd != CMD_CLOSE)
    return FALSE;

  if (!AWAKE (ch))
    return FALSE;

  one_argument (arg, buf);

  switch (cmd)
    {
    case CMD_USE:
      if (!isname (buf, MAR_IRON_BAR_NAME))
	return FALSE;
      if (!is_wearing_obj (ch, MAR_IRON_BAR, WIELD))
	return FALSE;

      block_on = get_obj_in_list_num(block1,world[CHAR_REAL_ROOM(ch)].contents);
      block_off = get_obj_in_list_num(block2,world[CHAR_REAL_ROOM(ch)].contents);

      if (!block_on && !block_off)
	{
	  send_to_char ("There seems to be nothing to do.\n\r",ch);
	  return TRUE;
	}
      else if (block_on)
	{
	  act ("$n moves $p away from the top of the hatch with $P.",
	       FALSE, ch, block_on, ch->equipment[WIELD], TO_ROOM);
	  act ("You move $p away from the top of the hatch with $P.",
	       FALSE, ch, block_on, ch->equipment[WIELD], TO_CHAR);

	  if (!block_off)
	    {
	      block_off = read_object (block2, REAL);
	      obj_to_room (block_off, CHAR_REAL_ROOM(ch));
	    }
	  extract_obj (block_on);
	}
      else
	{
	  act ("$n moves $p back to the top of the hatch with $P.",
	       FALSE, ch, block_off, ch->equipment[WIELD], TO_ROOM);
	  act ("You move $p back to the top of the hatch with $P.",
	       FALSE, ch, block_off, ch->equipment[WIELD], TO_CHAR);
	  block_on = read_object (block1, REAL);
	  obj_to_room (block_on, CHAR_REAL_ROOM(ch));
	  extract_obj (block_off);
	}

      return TRUE;

    case CMD_OPEN:
    case CMD_CLOSE:
      if (!isname (buf, MAR_HATCH_NAME))
	return FALSE;

      block_on = get_obj_in_list_num(block1,world[CHAR_REAL_ROOM(ch)].contents);

      if (block_on)
	{
	  act ("$n attempts to move the hatch which just doesn't budge.",
	       FALSE,ch,0,0,TO_ROOM);
	  send_to_char ("You try to move the hatch, but the stone blocks the way.",ch);
	  return TRUE;
	}
      break;
    }

  return FALSE;
}

int
mar_torch (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int target;
  if(!ch) return(FALSE);

  target = real_room (MAR_DARK_PASSAGE);

  if (cmd != CMD_USE)
    return FALSE;

  if (!AWAKE (ch))
    return FALSE;

  one_argument (arg, buf);
  if (!isname (buf, MAR_TORCH_HOLDER_NAME))
    return FALSE;

  act ("$n reaches up to the torch holder and pulls it hard...",
       TRUE,ch,0,0,TO_ROOM);
  act ("You reach up to the torch holder and pull it hard...",
       TRUE,ch,0,0,TO_CHAR);

  switch (ROOM_SPEC (room))
    {
    case MAR_DOOR_CLOSED:
      send_to_room ("The wall at the end of the passage slides slowly open,\n\rrevealing an opening to a dark room.\n\r",target);
      send_to_room ("A part of the east wall slides slowly aside,\n\rrevealing a dark passage.\n\r",room);
      ROOM_SPEC (room) = MAR_DOOR_OPEN;
      world[room].dir_option[EAST]->to_room_r = target;
      ROOM_SPEC (target) = MAR_DOOR_OPEN;
      world[target].dir_option[WEST]->to_room_r = room;
      return TRUE;

    case MAR_DOOR_OPEN:
            send_to_room ("The opening on the western end of the passage closes up,\n\ras the walls slides back to its place.\n\r", target);
      send_to_room ("The opening on the east wall to the dark passage closes up,\n\ras the wall slides back to its place.\n\r", room);
      ROOM_SPEC (room) = MAR_DOOR_CLOSED;
      world[room].dir_option[EAST]->to_room_r = -1;
      ROOM_SPEC (target) = MAR_DOOR_CLOSED;
      world[target].dir_option[WEST]->to_room_r = -1;
      return TRUE;
    }

  send_to_char ("Nothing happens.\n\r",ch);
  ROOM_SPEC (room) = MAR_DOOR_CLOSED;
  world[room].dir_option[EAST]->to_room_r = -1;
  ROOM_SPEC (target) = MAR_DOOR_CLOSED;
  world[target].dir_option[WEST]->to_room_r = -1;
  return TRUE;
}

int
mar_lever (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int target;
  if(!ch) return(FALSE);

  target = real_room (MAR_CELLAR);

  if (cmd != CMD_USE)
    return FALSE;

  if (!AWAKE (ch))
    return FALSE;

  one_argument (arg, buf);
  if (!isname (buf, MAR_LEVER_NAME))
    return FALSE;

  act ("$n reaches up to the iron lever and pulls it hard...",
       TRUE,ch,0,0,TO_ROOM);
  act ("You reach up to the iron lever and pull it hard...",
       TRUE,ch,0,0,TO_CHAR);

  switch (ROOM_SPEC (room))
    {
    case MAR_DOOR_CLOSED:
      send_to_room ("The wall at the end of the passage slides slowly open,\n\rrevealing an opening to a dark room.\n\r",room);
      send_to_room ("A part of the east wall slides slowly aside,\n\rrevealing a dark passage.\n\r",target);
      ROOM_SPEC (room) = MAR_DOOR_OPEN;
      world[room].dir_option[WEST]->to_room_r = target;
      ROOM_SPEC (target) = MAR_DOOR_OPEN;
      world[target].dir_option[EAST]->to_room_r = room;
      return TRUE;

    case MAR_DOOR_OPEN:
      send_to_room ("The opening on the western end of the passage closes up,\n\ras the walls slides back to its place.\n\r", room);
      send_to_room ("The opening on the east wall to the dark passage closes up\n\r, as the wall slides back to its place.\n\r", target);
      ROOM_SPEC (room) = MAR_DOOR_CLOSED;
      world[room].dir_option[WEST]->to_room_r = -1;
      ROOM_SPEC (target) = MAR_DOOR_CLOSED;
      world[target].dir_option[EAST]->to_room_r = -1;
      return TRUE;
    }

  send_to_char ("Nothing happens.\n\r",ch);
  ROOM_SPEC (room) = MAR_DOOR_CLOSED;
  world[room].dir_option[WEST]->to_room_r = -1;
  ROOM_SPEC (target) = MAR_DOOR_CLOSED;
  world[target].dir_option[EAST]->to_room_r = -1;
  return TRUE;
}

int
mar_slime (int room, CHAR *ch, int cmd, char *arg)
{
  if(!ch)
    return(FALSE);
  if (CMD_NORTH <= cmd && cmd <= CMD_DOWN && !number (0,3))
    {
      act ("$n falls down on the slippery floor.",TRUE,ch,0,0,TO_ROOM);
      send_to_char ("You fall down because of the slippery ooze on the floor.",
		    ch);
      GET_POS (ch) = POSITION_SITTING;
      WAIT_STATE (ch, PULSE_VIOLENCE);
      return TRUE;
    }
  return FALSE;
}

int
mar_egg (OBJ *egg, CHAR *ch, int cmd, char *arg)
{
  CHAR *lrv;
  int larvae_nr;

  if(!ch) return(FALSE);

  if(GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;

  larvae_nr = real_mobile (MAR_LARVAE);

  if (number (0,10) || mob_proto_table[larvae_nr].number > MAR_LARVAE_LIMIT)
    return FALSE;

  lrv = read_mobile (larvae_nr, REAL);

  char_to_room (lrv, CHAR_REAL_ROOM(ch));
  act ("$p bursts open, and $N crawls out.",FALSE,ch, egg, lrv, TO_ROOM);
  act ("$p bursts open, and $N crawls out.",FALSE,ch, egg, lrv, TO_CHAR);
  act ("$p bursts open, and you are born!",FALSE,lrv, egg, 0, TO_CHAR);
  extract_obj (egg);

  return FALSE;
}

int
mar_feather (OBJ *f, CHAR *ch, int cmd, char *arg)
{
  int count = 0;
  OBJ *oi = NULL;

  if (MSG_MOBACT != cmd) return FALSE;
  if (!(ch = f->equipped_by)) return FALSE;

  // count equipped feathers
  for (oi = object_list; oi; oi = oi->next) {
    if ((MAR_FEATHER == oi->item_number_v) && oi->equipped_by) {
      count++;
    }
  }

  if (MAR_FEATHER_LIMIT < count) {
    act("$p bursts into bright flames and\n\rdisappears from your hands.",
      FALSE, ch, f, 0, TO_CHAR);
    act ("$p bursts into bright flames and\n\rdisappears from $n's hands.",
      FALSE, ch, f, 0, TO_ROOM);
    unequip_char (ch, HOLD);
    obj_to_char (f, ch);
    extract_obj (f);
  }

  return FALSE;
}

int
mar_ghost (CHAR *gh, CHAR *ch, int cmd, char *arg)
{
  static int state = 0;
  char buf[MAX_INPUT_LENGTH];

  if (cmd)
    return FALSE;

  if (GET_OPPONENT (gh))
    {
      act ("$n disappears to the faint winds...", TRUE, gh, 0, 0, TO_ROOM);
      act ("You disappear to the faint winds...", TRUE, gh, 0, 0, TO_CHAR);
      if (EQ(gh, HOLD))
	extract_obj (unequip_char (gh, HOLD));
      extract_char (gh);
      return FALSE;
    }

  switch (state++)
    {
    case 0:
      REMOVE_BIT (gh->specials.affected_by, AFF_IMINV);
      break;

    case 1:
      sprintf(buf, "emote smiles sadly and caresses the roses gently.");
      command_interpreter (gh, buf);
      break;

    case 2:
      sprintf(buf, "sigh");
      command_interpreter (gh, buf);
      break;

    case 3:
      sprintf(buf, "say Pity the lost one. He suffered much fighting off the darkness.");
      command_interpreter (gh, buf);
      break;

    case 4:
      sprintf(buf, "say He did not stand a chance... Please, you be more careful!");
      command_interpreter (gh, buf);
      break;

    case 5:
      sprintf(buf, "say The foulness is still here, forever I fear...");
      command_interpreter (gh, buf);
      break;

    case 6:
      sprintf(buf, "emote tries to touch your face, but her hand passes through it.");
      command_interpreter (gh, buf);
      break;

    case 7:
      SET_BIT (gh->specials.affected_by, AFF_IMINV);
      break;

    case 9:
      state = -7;
      break;

    default:
      break;
    }

  return FALSE;
}

int
mar_shield (OBJ *sh, CHAR *ch, int cmd, char *arg)
{
  CHAR *tmp, *tch;
  char buf[1000];

  if(CHAOSMODE) return FALSE;

  if (cmd == MSG_DIE)
    {
      /* Find the person who is getting killed ... */

      if (sh->equipped_by) {
	      tch = sh->equipped_by;
        if(IS_MOB(tch)) return FALSE;
      }
      else
	      return FALSE;

  if (IS_SET(world[CHAR_REAL_ROOM(tch)].room_flags, CHAOTIC)) return FALSE;

      act ("$n is just about to die, but $p grows and envelopes $m.\n\r$p disappears.", TRUE, tch, sh, 0, TO_ROOM);
      send_to_char ("As you draw your last breath, your shield envelopes you\n\rand there is only blackness...\n\r",tch);

      GET_HIT (tch) = 1;
      GET_MANA (tch) = 0;
      GET_MOVE (tch) = 0;

      for (tmp = world[CHAR_REAL_ROOM(tch)].people;tmp;tmp=tmp->next_in_room)
	if (GET_OPPONENT(tmp) == tch)
	  stop_fighting (tmp);

      sprintf (buf, "%s rescued %s from %s [%d].", OBJ_SHORT(sh),
	       GET_NAME(tch), world[CHAR_REAL_ROOM(tch)].name,
	       CHAR_VIRTUAL_ROOM(tch));
      wizlog(buf, LEVEL_WIZ, 6);
      log_s(buf);

      stop_fighting (tch);
      unequip_char (tch, WEAR_SHIELD);
      obj_to_char (sh, tch);
      extract_obj (sh);

      spell_word_of_recall (GET_LEVEL(tch), tch, tch, 0);
      return TRUE;
    }

  return FALSE;
}

int
mar_sting (OBJ *st, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR *vict;

  if (st->equipped_by != ch)
    return FALSE;

  if (cmd != CMD_BACKSTAB)
    return FALSE;

  //Skeena 6/17/2011: insta kill any mob with zero effort! yeehaw!
  if(ch->specials.fighting) return FALSE;

  if ((GET_CLASS (ch) == CLASS_THIEF || GET_CLASS(ch) == CLASS_ANTI_PALADIN) &&
      !number (0,69))
    {
      one_argument (arg, buf);

      if (!(vict = get_char_room_vis (ch, buf)))
	return FALSE;

      if(IS_NPC(vict) && IS_SET(vict->specials.immune,IMMUNE_BACKSTAB))
        return FALSE;

      //Skeena 6/17/2011 No more sting specs in chaos :(
      if(!IS_NPC(vict)) return FALSE;
      act ("$n sticks $p into $N's back, and waves of greenish energy wash over $M.", TRUE, ch, st, vict, TO_NOTVICT);
      act ("$n sticks $p into your back, and waves of lethal poison wash over you.", FALSE, ch, st, vict, TO_VICT);
      act ("You stick $p into $N's back, and waves of greenish energy wash over $M.", TRUE, ch, st, vict, TO_CHAR);

      if (chance(50))
	{
	  act ("$p cracks and falls apart.", FALSE,ch, st, 0, TO_ROOM);
	  act ("$p cracks and falls apart.", FALSE,ch, st, 0, TO_CHAR);
	  unequip_char (ch, WIELD);
	  extract_obj (st);
	}

      send_to_char ("You can feel the poison burning its way into your heart.\n\rYou are dying...\n\r", vict);
      damage (ch, vict, 1000, TYPE_KILL,DAM_NO_BLOCK);
      return TRUE;
    }
  return FALSE;
}
