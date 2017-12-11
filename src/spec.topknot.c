/*
///   Special Procedure Module                   Orig. Date    20-05-1994
///                                              Last Modified 13-06-1994
///
///   spec.topknot.c --- Specials for The HMS Topknot
///
///   Designed by Drac, Leo, Bam@Imperial and Coded by Kafka@Imperial
///   Using this code is not allowed without permission from originator.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"
#include "utility.h"
#include "cmd.h"
#include "spec_assign.h"

void stop_fighting (CHAR *);
void hit (CHAR *, CHAR *, int);
void command_interpreter (CHAR *ch, char *arg);
char *one_argument(char *, char *);

void do_say  (CHAR*, char*, int);

#define ZONE_TOPKNOT           5800

/* Objects */

#define TK_PISTOL              ITEM(ZONE_TOPKNOT,0)
#define TK_HOOPAK              ITEM(ZONE_TOPKNOT,3)
#define TK_CLOAK               ITEM(ZONE_TOPKNOT,5)
#define TK_SYMBOL              ITEM(ZONE_TOPKNOT,6)
#define TK_SILVER_PIN          ITEM(ZONE_TOPKNOT,8)
#define TK_COFFIN              ITEM(ZONE_TOPKNOT,10)
#define TK_CANNON              ITEM(ZONE_TOPKNOT,11)

/* Mobiles */

#define TK_BARON               ITEM(ZONE_TOPKNOT,0)
#define TK_SLAPHOFF            ITEM(ZONE_TOPKNOT,1)
#define TK_SCOTTY              ITEM(ZONE_TOPKNOT,2)
#define TK_KENDER_START        ITEM(ZONE_TOPKNOT,3) /* First standard kender */
#define TK_KENDER_END          ITEM(ZONE_TOPKNOT,11)/* Last standard kender */

/* Locations */

#define TK_TELEPORT_ROOM       ITEM(ZONE_TOPKNOT,8)
#define TK_BURNED_FOREST       ITEM(ZONE_TOPKNOT,0)
#define TK_COFFIN_INSIDE       ITEM(ZONE_TOPKNOT,58)
#define TK_STORAGE             ITEM(ZONE_TOPKNOT,7)
#define TK_CRASHED             ITEM(ZONE_TOPKNOT,5)

/* Miscellaneous */

#define TK_START             5800
#define TK_END               5858

#define TK_TO_COFFIN            0
#define TK_TO_WORLD             1

#define TK_BARON_TARGET      6003

#define TK_CANNON_MSG_YES \
"A bright beam of light comes out of the upper end of the tube. It hits\n\r\
the ground in the forest around the crater! BOOM!\n\r"

#define TK_CANNON_MSG_NO  "You can't figure out how to use this thing.\n\r"

#define TK_BOOM           "BOOM!\n\r"
#define TK_CANNON_HIT     "A bright beam of light hits you!\n\r"
#define TK_CANNON_MISS    "A bright beam of light hits the ground!\n\r"

/* Prototypes */

int tk_kender   (CHAR*, CHAR*, int, char*);
int tk_slaphoff (CHAR*, CHAR*, int, char*);
int tk_baron    (CHAR*, CHAR*, int, char*);

int tk_pin      (OBJ*, CHAR*, int, char*);
int tk_symbol   (OBJ*, CHAR*, int, char*);
int tk_cloak    (OBJ*, CHAR*, int, char*);
int tk_hoopak   (OBJ*, CHAR*, int, char*);
int tk_cannon   (OBJ*, CHAR*, int, char*);
int tk_pistol   (OBJ*, CHAR*, int, char*);
int tk_coffin   (OBJ*, CHAR*, int, char*);

int tk_crashed  (int, CHAR*, int, char*);

/* Assignment */

void assign_topknot (void)
{
  int i;

  for (i = TK_KENDER_START;i<=TK_KENDER_END;i++)
    assign_mob(i,tk_kender);

  assign_mob(TK_BARON,tk_baron);
  assign_mob(TK_SLAPHOFF,tk_slaphoff);

  assign_obj(TK_SILVER_PIN,tk_pin);
  assign_obj(TK_SYMBOL,tk_symbol);
  assign_obj(TK_HOOPAK,tk_hoopak);
  assign_obj(TK_PISTOL,tk_pistol);
  assign_obj(TK_CANNON,tk_cannon);
  assign_obj(TK_COFFIN,tk_coffin);
  assign_obj(TK_CLOAK,tk_cloak);
  assign_room(TK_CRASHED,tk_crashed);
}

/* Utility Functions */

int
tk_is_friend (CHAR *ch)
{
  int virtual;

  if (!IS_NPC (ch))
    return 0;
  else
    {
      virtual = V_MOB (ch);
      if (TK_BARON <= virtual && virtual <= TK_KENDER_END)
	return 1;
    }
  return 0;
}

int
tk_has_operating_pin (CHAR *ch)
{
  OBJ *pin = NULL;

  if (is_wearing_obj (ch, TK_SILVER_PIN, WEAR_NECK_1))
    {
      pin = EQ (ch, WEAR_NECK_1);
      if (IS_SET(pin->obj_flags.extra_flags, ITEM_HUM))
	return TRUE;
    }
  else
    if (is_wearing_obj (ch, TK_SILVER_PIN, WEAR_NECK_2))
      {
	pin = EQ (ch, WEAR_NECK_2);
	if (IS_SET(pin->obj_flags.extra_flags, ITEM_HUM))
	  return TRUE;
      }
    else
      if (is_wearing_obj (ch, TK_SILVER_PIN, WEAR_ARMS))
	{
	  pin = EQ (ch, WEAR_ARMS);
	  if (IS_SET(pin->obj_flags.extra_flags, ITEM_HUM))
	    return TRUE;
	}

  return FALSE;
}

void
tk_scotty_beam (CHAR *sc, CHAR *beam, CHAR *to)
{
  char buf[MAX_INPUT_LENGTH];

  act ("$n pushes some buttons and pulls a few levers,\n\rand you hear a humming sound.", FALSE,sc,0,0,TO_ROOM);
  act ("You push some buttons and pull a few levers,\n\rand you hear a humming sound.", FALSE,sc,0,0,TO_CHAR);
  act ("$n disappears in a beam of bright light.",TRUE,beam,0,0,TO_ROOM);
  send_to_char ("A bright beam of light takes you away...\n\r",beam);
  char_from_room (beam);
  char_to_room (beam, CHAR_REAL_ROOM(to));
  act ("$n materializes in a bright beam of light.",TRUE,beam,0,0,TO_ROOM);
  sprintf (buf, "look");
  command_interpreter (beam, buf);
}

void
tk_beam_request (CHAR *ch)
{
  CHAR *scotty = 0, *tmp, *t = 0;
  int teleport_nr;

  teleport_nr = real_room (TK_TELEPORT_ROOM);

  if (GET_OPPONENT (ch))
    return;

  for (tmp = world[teleport_nr].people;tmp && !scotty;tmp=tmp->next_in_room)
    if (IS_NPC (tmp) && V_MOB(tmp) == TK_SCOTTY)
      scotty = tmp;

  if (!scotty || !AWAKE(scotty) || GET_OPPONENT(scotty))
    return;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_BEAM)) {
    send_to_char("You cannot teleport from here.\n\r",ch);
    return;
  }                              /* Liner 02 - only adding NO_BEAM rooms to not be beamable from.. */
  
  if (GET_ZONE(ch) == world[teleport_nr].zone)
    {
      for (tmp = character_list;tmp;tmp = tmp->next)
	if ((IS_NPC (tmp) || GET_LEVEL (tmp) < LEVEL_IMM) &&
	    GET_ZONE(tmp) != world[real_room(0)].zone &&    /* Void & Limbo */
	    GET_ZONE(tmp) != world[real_room(3014)].zone && /* Upper City */
	    GET_ZONE(tmp) != world[real_room(1212)].zone && /* Immort */
	    GET_ZONE(tmp) != world[real_room(3137)].zone && /* Lower City */
	    GET_ZONE(tmp) != world[real_room(3501)].zone && /* Training Gr. */
	    CAN_SEE (scotty, tmp) &&
	    !IS_SET (world[CHAR_REAL_ROOM(tmp)].room_flags, SAFE) &&
	    !IS_SET (world[CHAR_REAL_ROOM(tmp)].room_flags, CLUB) &&
	    !IS_SET (world[CHAR_REAL_ROOM(tmp)].room_flags, NO_BEAM) &&
	    !IS_SET (world[CHAR_REAL_ROOM(ch)].room_flags, NO_BEAM) &&
	    !IS_SET (world[CHAR_REAL_ROOM(tmp)].room_flags, PRIVATE))
	  {
	    if (!t)
	      t = tmp;

	    if (!number (0,9))
	      t = tmp;
	  }

      if (t)
	{
	  if (!number (0,4))
	    tk_scotty_beam (scotty, ch, t);
	  else
	    spell_teleport (30, ch, ch, 0);
	}
      else return;
    }
  else
    tk_scotty_beam (scotty, ch, scotty);
}

void
tk_pin_message (CHAR *ch, char *msg)
{
  char buf2[MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];
  CHAR *tch;
  int tk_teleport_room;

  tk_teleport_room = real_room (TK_TELEPORT_ROOM);

  while (*msg == ' ')  /* skip any spaces in the beginning */
    msg++;

  for (tch=character_list;tch;tch=tch->next)
    {
      if (tk_has_operating_pin (tch) && tch != ch && AWAKE (tch) &&
	  CHAR_REAL_ROOM(tch) != tk_teleport_room )
	{
	  sprintf (buf2,
		   "You hear a voice coming from the silver pin,\n\r'%s'\n\r",
		   msg);
	  send_to_char (buf2, tch);
	}
    }

  sprintf (buf3,
	   "You hear a voice coming from the big box in the wall,\n\r'%s'\n\r",
	   msg);
  send_to_room (buf3, tk_teleport_room);

  if (strstr (msg, "beam") || strstr (msg, "Beam")) {
    if(CHAOSMODE) send_to_char("Chaotic energies disrupt the beam.\n\r",ch);
    else tk_beam_request (ch);
  }
}

CHAR*
tk_find_kender (CHAR *ch)
{
  CHAR *tk_kender = 0;
  int v_num = 0;

  for (tk_kender = character_list;tk_kender;tk_kender=tk_kender->next)
    if (IS_NPC (tk_kender))
      {
	v_num = V_MOB (tk_kender);
	if (v_num >= TK_KENDER_START && v_num <= TK_KENDER_END &&
	    CHAR_REAL_ROOM(tk_kender) != CHAR_REAL_ROOM(ch))
	  return tk_kender;
      }
  return 0;
}


CHAR*
tk_get_baron_victim (CHAR *ch)
{
  CHAR *tmp, *vict = 0;

  for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp = tmp->next_in_room)
    if (IS_MORTAL (tmp) && tmp->specials.fighting == ch &&
	CAN_SEE(ch, tmp))
      {
	if (!vict)
	  vict = tmp;

	if (!number (0,4))
	  vict = tmp;
      }
  return vict;
}

CHAR*
tk_get_steal_victim (CHAR *ch)
{
  CHAR *tmp, *vict = 0;

  for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp = tmp->next_in_room)
    if (!tk_is_friend (tmp) && CAN_SEE(ch, tmp) &&
	(IS_NPC (tmp) ||
	 (GET_LEVEL (tmp) < LEVEL_IMM && GET_LEVEL (tmp) > 20)))
      {
	if (V_MOB(ch) == TK_SLAPHOFF && !IS_NPC(tmp) &&
	    GET_LEVEL (tmp) < 25)
	  continue;

	if (tmp->desc && !tmp->desc->descriptor)
	  continue;

	if (tmp->desc && (tmp->desc->connected != CON_PLYNG)) continue;

	if (!vict)
	  vict = tmp;

	if (!number (0,9))
	  vict = tmp;
      }

  return vict;
}

int
tk_assist_friend (CHAR *ch)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR *tmp;

  if (GET_OPPONENT (ch))
    return 0;

  for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp = tmp->next_in_room)
    if (tk_is_friend (tmp) && GET_OPPONENT (tmp) && CAN_SEE (ch, tmp) &&
	!tk_is_friend (GET_OPPONENT (tmp)))
      {
	sprintf (buf, "say Hey, don't have all the fun alone!");
	command_interpreter (ch, buf);
	sprintf (buf, "assist %s", GET_NAME(tmp));
	command_interpreter (ch, buf);
	return 1;
      }
  return 0;
}

int
tk_assist_master (CHAR *ch)
{
  CHAR *tar;
  char buf[MAX_INPUT_LENGTH];

  if (GET_OPPONENT (ch))
    return 0;

  if (!ch->master || !ch->master->specials.fighting)
    return 0;

  tar = ch->master->specials.fighting;

  if (!CAN_SEE(ch, tar))
    return 0;

  if (!number(0,6) && !tk_is_friend (tar))
    {
      sprintf (buf, "say That's not fair !");
      command_interpreter (ch, buf);
      sprintf (buf, "hit %s", GET_NAME (tar));
      command_interpreter (ch, buf);
      return 1;
    }
  return 0;
}

int
tk_assist_random (CHAR *ch)
{
  CHAR *tmp;
  char buf[MAX_INPUT_LENGTH];

  if (GET_OPPONENT (ch))
    return 0;

  for (tmp = world[CHAR_REAL_ROOM(ch)].people;tmp;tmp = tmp->next_in_room)
    if (GET_OPPONENT (tmp) && !tk_is_friend (GET_OPPONENT (tmp)) &&
	!number (0,15) && CAN_SEE(ch, GET_OPPONENT(tmp)))
      {
	sprintf (buf, "say Cool fight you have there.");
	command_interpreter(ch, buf);
	sprintf (buf, "assist %s", GET_NAME(tmp));
	command_interpreter (ch, buf);
	return 1;
      }
  return 0;
}

void
tk_baron_drain (CHAR *ch, CHAR *v)
{
  int chance = number (1,100);

  if (chance < 60)
    {
      if(v->points.max_hit>0) {
        v->points.max_hit = MAX (v->points.max_hit - number (1,4), 0);
        GET_HIT (v) = MIN (GET_MAX_HIT (v), GET_HIT (v));
      }
    }
  else if (chance < 65)
    {
      if(v->points.max_mana>0) {
        v->points.max_mana = MAX (v->points.max_mana - number (1,4), 0);
        GET_MANA (v) = MIN (GET_MAX_MANA (v), GET_MANA (v));
      }
    }
  else if (chance < 70)
    {
      if(v->points.max_move>0) {
        v->points.max_move = MAX (v->points.max_move - number (1,10), 0);
        GET_MOVE (v) = MIN (GET_MAX_MOVE (v), GET_MOVE (v));
      }
    }
  else if (chance < 79)
    {
      if (v->abilities.str == 18 && v->abilities.str_add > 0)
	{
	  v->abilities.str_add = MAX (0,v->abilities.str_add - number (1,2));
	  v->tmpabilities.str_add = v->abilities.str_add;
	}
      else
	{
	  v->abilities.str = MAX (3,v->abilities.str - 1);
	  v->tmpabilities.str = v->abilities.str;
	}
    }
  else if (chance < 80)
    {
      v->abilities.intel = MAX (3,v->abilities.intel - 1);
      v->tmpabilities.intel = v->abilities.intel;
    }
  else if (chance < 85)
    {
      v->abilities.wis = MAX (3,v->abilities.wis - 1);
      v->tmpabilities.wis = v->abilities.wis;
    }
  else if (chance < 90)
    {
      v->abilities.dex = MAX (3,v->abilities.dex - 1);
      v->tmpabilities.dex = v->abilities.dex;
    }
  else if (chance < 95)
    {
      v->abilities.con = MAX (3,v->abilities.con - 1);
      v->tmpabilities.con = v->abilities.con;
    }
  else if (chance < 99)
    v->points.exp = MAX (0, v->points.exp * number (1,50) / 100);
  else
    {
      GET_LEVEL (v) = GET_LEVEL (v) - 1;
      GET_EXP (v) = 0;
      v->points.max_hit -= ((v->points.max_hit)-10)/(GET_LEVEL (v));
      v->points.max_mana -= ((v->points.max_mana)/GET_LEVEL (v));
    }
  affect_total (v);
}

OBJ*
tk_get_loot_inv (CHAR *k, CHAR *ch)
{
  OBJ *tar = 0, *tmp;

  for (tmp = ch->carrying;tmp;tmp = tmp->next_content)
    {
      if (GET_OBJ_WEIGHT(tmp) > 20 || !CAN_SEE_OBJ(k, tmp))
	continue;

      if (!tar)
	tar = tmp;

      if (!number (0,5))
	tar = tmp;
    }
  return tmp;
}

int
tk_get_loot_eq (CHAR *k, CHAR *ch)
{
  OBJ *t;
  int tar = -1, tmp;

  for (tmp = 0;tmp < MAX_WEAR;tmp++)
    {
      if (tmp == WEAR_ARMS || tmp == WEAR_LEGS || tmp == WEAR_HEAD ||
	  tmp == WIELD || tmp == WEAR_BODY || tmp == WEAR_FEET ||
	  tmp == WEAR_ABOUT || tmp == WEAR_SHIELD ||
	  (GET_CLASS(ch) == CLASS_NINJA && tmp == HOLD))
	continue;

      if ((t = EQ (ch, tmp)))
	{
	  if (GET_OBJ_WEIGHT(t) > 20 || !CAN_SEE_OBJ (k,t))
	    continue;

	  if (tar == -1)
	    tar = tmp;

	  if (!number (0,5))
	    tar = tmp;
	}
    }
  return tar;
}

int
tk_kender_steal (CHAR *ch)
{
  CHAR *vict;
  OBJ *loot = 0;
  int l = -1;
  char buf[MAX_STRING_LENGTH];

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE))
    return 0;

  if ((vict = tk_get_steal_victim (ch)))
    {
      if (number (0,19))
	{
	  loot = tk_get_loot_inv (ch, vict);
	  if (loot)
	    {
	      obj_from_char (loot);
	      obj_to_char (loot, ch);
	      save_char(vict, NOWHERE);
	      sprintf(buf, "%s just stole %s from %s",
		      MOB_SHORT(ch), OBJ_SHORT(loot),
		      (IS_NPC(vict) ? MOB_SHORT(vict) :
		       GET_NAME(vict)));
	      /*wizlog(buf, LEVEL_WIZ, 6);*/
	      log_s(buf);
	      loot->log=1;
	      return 1;
	    }
	  return 0;
	}
      else
	if (number (0,12))
	  {
	    l = tk_get_loot_eq (ch, vict);

	    if (l != -1)
	      {
		loot = unequip_char (vict, l);
		obj_to_char (loot, ch);
    save_char(vict, NOWHERE);
		sprintf(buf, "%s just stole %s from %s",
			MOB_SHORT(ch), OBJ_SHORT(loot),
			(IS_NPC(vict) ? MOB_SHORT(vict) :
			 GET_NAME(vict)));
		/*wizlog(buf, LEVEL_WIZ, 6);*/
		log_s(buf);
    loot->log=1;
		return 1;
	      }

	    return 0;
	  }
	else
	  {
	    act ("You discover $n's hands in your pockets!",
		 FALSE, ch, 0, vict, TO_VICT);
	    act ("Hey, $n stares at you strangely.",
		 TRUE, vict, 0, ch, TO_VICT);
	    act ("$n stares at $N strangely.", TRUE, vict, 0, ch, TO_NOTVICT);
	  }
    }
  return 0;
}

int
tk_kender_follow (CHAR *ch)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR *c_tmp;

  if (ch->master)
    {
      sprintf (buf, "say I have to be going now... Bye!");
      command_interpreter (ch, buf);
      sprintf (buf, "follow %s", GET_NAME(ch));
      command_interpreter (ch, buf);
      return 1;
    }
  else if ((c_tmp = tk_get_steal_victim (ch)))
    {
      sprintf (buf, "say Hi, I am coming with you.");
      command_interpreter (ch, buf);
      sprintf (buf, "follow %s", GET_NAME(c_tmp));
      command_interpreter (ch, buf);
      return 1;
    }
  return 0;
}

void
tk_kender_action (CHAR *ch)
{
  static char *tk_kender_msg[] =
    {
      "Beam me out, Scotty!",
      "Beam me up, Scotty!"
    };

  static char *tk_kender_speech[] =
    {
      "Got a snack?",
      "I'm hungry. Are you hungry?",
      "Have you seen Mikey?",
      "Where is the nearest bakery?",
      "Where are all the sprites, I want a sprite hat.",
      "Have you seen a good, long pole?",
      "Cool!",
      "What's that?",
      "Which way leads to Aldor? It's dull here...",
      "Have you seen the milky way? It's awesome...",
      "Come on, be a sport!",
      "Pleeeaase...",
      "Can you tell me where I could find a puppy for myself.",
      "I did not, I found it!",
      "I heard there are really weird objects lying around here.",
      "Just wait until I show this to Scotty."
    };

  OBJ *o_tmp;
  CHAR *c_tmp;
  int storage,teleport;
  int v_rm;
  char buf2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if (number (0,1))
    {
      sprintf (buf, "say %s", tk_kender_speech[number(0,15)]);
      command_interpreter (ch, buf);
    }

  storage = real_room (TK_STORAGE);
  teleport = real_room (TK_TELEPORT_ROOM);

  v_rm = V_ROOM (ch);

  if (TK_START <= v_rm && v_rm <= TK_END && !number (0,1))
    {
      sprintf (buf2, "say %s", tk_kender_msg[0]);
      command_interpreter (ch, buf2);
      return;
    }

  if (number (0,1) && tk_kender_steal (ch))
    return;

  if ((v_rm < TK_START || TK_END < v_rm) && !number (0,9))
    {
      sprintf (buf2, "say %s", tk_kender_msg[1]);
      command_interpreter (ch, buf2);

      if (CHAR_REAL_ROOM(ch) == teleport && ch->carrying)
	{
	  o_tmp = ch->carrying;
	  obj_from_char (o_tmp);
	  obj_to_room (o_tmp, storage);
	}

      return;
    }

  if (!number (0,3) && tk_kender_follow (ch))
    return;

  if (tk_assist_friend (ch))
    return;

  if (V_MOB(ch) != TK_SLAPHOFF)
    {
      if (tk_assist_master (ch))
	return;

      if (tk_assist_random (ch))
	return;
    }

  if (ch->carrying && number (1,100) < ch->specials.carry_items<<1)
    {
      o_tmp = ch->carrying;
      sprintf (buf2, "drop %s", fname(OBJ_NAME(o_tmp)));
      command_interpreter (ch, buf2);
      sprintf (buf, "%s just dropped %s in #%d",
	       MOB_SHORT(ch), OBJ_SHORT(o_tmp), CHAR_VIRTUAL_ROOM(ch));
      /*wizlog(buf, LEVEL_WIZ, 6);*/
      log_s(buf);
      return;
    }

  if (ch->carrying && (c_tmp = tk_get_steal_victim (ch)) &&
      number (1,70) < (55 - GET_LEVEL(c_tmp)))
    {
      o_tmp = ch->carrying;
      sprintf (buf2, "give %s %s", fname(OBJ_NAME(o_tmp)), GET_NAME(c_tmp));
      command_interpreter (ch, buf2);
      sprintf (buf, "%s just gave %s to %s in #%d",
	       MOB_SHORT(ch), OBJ_SHORT(o_tmp), PERS(c_tmp, ch),
	       CHAR_VIRTUAL_ROOM(ch));
      /*wizlog(buf, LEVEL_WIZ, 6);*/
      log_s(buf);
      return;
    }

}

void
tk_baron_offensive (CHAR *ch, CHAR *vict)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR *tk_kender = 0;

  static char *tk_baron_speech[] =
    {
      "Hah! I'll get you now!",
      "You can never kill me!",
      "I am eternal, there is no hope for you.",
      "I'll suck you dry...",
      "Zum Teufel!",
      "Beware...Behind you!",
      "Karsch will avenge me!",
      "Stop your pitiful whining, mortal!",
      "You ignorant fools, you will surely die by my hand."
    };

  if (!number (0,5))
    {
      act("$n points at $N and whispers a word.",FALSE,ch,0,vict,TO_NOTVICT);
      act("$n points at you and whispers a word.",FALSE,ch,0,vict,TO_VICT);
      act("You point at $N and whisper a command to Heinrich.",
	  FALSE, ch, 0, vict, TO_CHAR);
      spell_teleport (GET_LEVEL (ch), vict, vict, 0);
      return;
    }

  if (!number (0,3) && (tk_kender = tk_find_kender(ch)))
    {
      act ("$n whispers something.",FALSE,ch,0,0,TO_ROOM);
      act ("You whisper a quiet command to Heinrich.", FALSE,ch,0,0,TO_CHAR);
      act ("You hear a command from $N, and are summoned to aid your master.",
	   FALSE, tk_kender, 0, ch, TO_CHAR);
      act ("$n disappears suddenly, in a bright beam of light.",
	   FALSE, tk_kender, 0, 0, TO_ROOM);
      char_from_room (tk_kender);
      char_to_room (tk_kender, CHAR_REAL_ROOM(ch));
      act ("$n appears suddenly, in a bright beam of light.",
	   FALSE, tk_kender, 0, 0, TO_ROOM);

      sprintf (buf, "look");
      command_interpreter (tk_kender, buf);

      sprintf (buf, "say I hear and I obey!");
      command_interpreter (tk_kender, buf);

      sprintf (buf, "hit %s", GET_NAME(vict));
      command_interpreter (tk_kender, buf);
      return;
    }

  if (!number (0,1))
    {
      act("$n sinks $s teeth into $N's bare neck.",FALSE,ch,0,vict,TO_NOTVICT);
      act("$n sinks $s teeth into your bare neck. You scream in pain!",
	   FALSE, ch, 0, vict, TO_VICT);
      act("You sink your teeth into $N's bare neck with great desire.",
	   FALSE, ch, 0, vict, TO_CHAR);
      tk_baron_drain (ch, vict);
      damage (ch, vict, number (1,5)*100, TYPE_UNDEFINED,DAM_NO_BLOCK);
      return;
    }

  sprintf (buf, "say %s", tk_baron_speech[number(0,8)]);
  command_interpreter (ch, buf);
  return;
}

void
tk_baron_escape (CHAR *ch, int type)
{
  int inside_coffin;
  CHAR *c_tmp;
  char buf[MAX_INPUT_LENGTH];

  inside_coffin = real_room (TK_COFFIN_INSIDE);

  for (c_tmp = world[CHAR_REAL_ROOM(ch)].people;c_tmp;c_tmp = c_tmp->next_in_room)
    if (GET_OPPONENT (c_tmp) == ch)
      stop_fighting (c_tmp);

  if (ch->specials.fighting)
    stop_fighting (ch);

  switch (type)
    {

    case TK_TO_COFFIN:
      act ("$n utters a strange mantra and disappears..",FALSE,ch,0,0,TO_ROOM);
      send_to_char ("You escape back to the shadows...\n\r",ch);
      SET_BIT (ch->specials.act, ACT_AGGRESSIVE);
      REMOVE_BIT (ch->specials.affected_by, AFF_HIDE);
      char_from_room (ch);
      char_to_room (ch, inside_coffin);
      sprintf (buf, "look");
      command_interpreter (ch, buf);
      break;

    case TK_TO_WORLD:
      act ("$n disappears after uttering an arcane mantra...",
	   FALSE,ch,0,0,TO_ROOM);
      send_to_char ("You leave for the hunt...\n\r",ch);
      REMOVE_BIT (ch->specials.act, ACT_AGGRESSIVE);
      SET_BIT (ch->specials.affected_by, AFF_HIDE);
      char_from_room (ch);
      char_to_room (ch, real_room (TK_BARON_TARGET));
      sprintf (buf, "look");
      command_interpreter (ch,buf);
      break;
    }
}

/* Special Procedures */

int
tk_pin (OBJ *pin, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];

  switch (cmd)
    {
    case CMD_SAY:
      if (tk_has_operating_pin (ch))
	{
	  do_say (ch, arg, CMD_SAY);
	  tk_pin_message (ch, arg);
	  return TRUE;
	}
      else
	return FALSE;

    case CMD_USE:
      one_argument (arg,buf);

      if (!isname(buf, OBJ_NAME(pin)) || pin->equipped_by != ch)
	return FALSE;

      act ("$n touches $p lightly.",FALSE,ch,pin,0,TO_ROOM);

      if (IS_SET(pin->obj_flags.extra_flags, ITEM_HUM))
	{
	  send_to_char ("The pin stops humming as you touch it.\n\r",ch);
	  REMOVE_BIT (pin->obj_flags.extra_flags, ITEM_HUM);
	}
      else
	{
	  send_to_char ("The pin starts humming as you touch it.\n\r",ch);
	  SET_BIT (pin->obj_flags.extra_flags, ITEM_HUM);
	}
      return TRUE;

    default:
      return FALSE;
    }
}

int
tk_symbol (OBJ *sym, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  void remove_item (CHAR*, OBJ*, int);

  if (cmd == MSG_BEING_REMOVED) {
    if(!ch) return FALSE;
	  send_to_char ("Eeek! You can't let it go!\n\r",ch);
	  return TRUE;
	}

  if(cmd == MSG_MOBACT) {
    if(!(vict=sym->equipped_by)) return FALSE;
    if(!IS_MORTAL(vict)) return FALSE;
    if(GET_ALIGNMENT (vict)>-1000) {
      GET_ALIGNMENT (vict) = MAX(-1000, GET_ALIGNMENT (vict) - 100);
      check_equipment(vict);
    }
  }
  return FALSE;
}

int
tk_cloak (OBJ *cloak, CHAR *ch, int cmd, char *arg)
{
  if(!ch)
    return FALSE;
  if (cmd == MSG_MOBACT && is_wearing_obj (ch, TK_CLOAK, WEAR_ABOUT))
    {
      gain_condition (ch, THIRST, -5);
      gain_condition (ch, FULL, -5);
    }

  return FALSE;
}

int
tk_hoopak (OBJ *hp, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR *vict;
  int coins = number (4,14);

  if(!ch)
    return(FALSE);
  if (cmd != CMD_SHOOT || !AWAKE (ch) || hp->equipped_by != ch)
    return FALSE;

  if (!*arg && !ch->specials.fighting)
    {
      send_to_char ("Who are you going to shoot at?\n\r",ch);
      return TRUE;
    }
  else
    {
      if (*arg)
	{
	  one_argument (arg, buf);

	  if (!(vict = get_char_room_vis (ch, buf)))
	    {
	      send_to_char ("Even a hoopak doesn't carry that far!\n\r",ch);
	      return TRUE;
	    }
	}
      else
	vict = ch->specials.fighting;
    }

  if (GET_GOLD (ch) == 0)
    {
      send_to_char("But you have no coins to throw!\n\r",ch);
      return TRUE;
    }

  coins = MIN (GET_GOLD (ch), coins);
  GET_GOLD (ch) = MAX (0, GET_GOLD (ch) - coins);

  act ("$n loads $p with a few coins and shoots them at $N.",
       FALSE,ch,hp,vict,TO_ROOM);
  act ("You load $p with a few coins and shoot the at $N.",
       FALSE,ch,hp,vict,TO_CHAR);
  damage (ch, vict, coins<<1, SKILL_THROW,DAM_NO_BLOCK);
  WAIT_STATE (ch, PULSE_VIOLENCE*2);
  return TRUE;
}

int
tk_cannon (OBJ *cannon, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];
  int dmg = 0;
  int forest_nr;
  CHAR *vict;
  if(!ch)
    return(FALSE);

  forest_nr = real_room (TK_BURNED_FOREST);

  if (cmd != CMD_USE || !AWAKE (ch))
    return FALSE;

  one_argument (arg, buf);
  if (!isname(buf,OBJ_NAME(cannon)))
    return FALSE;

  if (GET_INT (ch) < number (1,50))
    {
      send_to_char (TK_CANNON_MSG_NO, ch);
      act ("$n pushes some buttons on $p's side, but nothing happens.",
	   TRUE,ch,cannon,0,TO_ROOM);
    }
  else
    {
      send_to_char (TK_CANNON_MSG_YES, ch);
      act ("$n pushes some buttons on $p's side, and something happens!!",
	   TRUE,ch,cannon,0,TO_ROOM);

      for (vict = character_list;vict;vict = vict->next)
	if (GET_ZONE (vict) == GET_ZONE (ch))
	  send_to_char (TK_BOOM, vict);

      if ((vict = world[forest_nr].people))
	{
	  send_to_char (TK_CANNON_HIT, vict);
	  send_to_room_except (TK_CANNON_MISS, forest_nr, vict);
	  dmg = number (1,3000);
	  damage (vict, vict, dmg, TYPE_UNDEFINED,DAM_NO_BLOCK);
	  sprintf (buf, "Kender:: [%s fired the cannon at %s for %d points.]\n\r", GET_NAME(ch), GET_NAME(vict), dmg);
	  log_s(buf);
	}
      send_to_char ("You are stunned by the sound.\n\r",ch);
      WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
    }
  return TRUE;
}


int
tk_pistol (OBJ *pistol, CHAR *ch, int cmd, char *arg)
{
  if(!ch)
    return(FALSE);
  if (cmd != CMD_SHOOT || !AWAKE (ch) || pistol->equipped_by != ch)
    return FALSE;

  if (GET_INT (ch) < number (0,30))
    {
      send_to_char ("You can't figure out how to use that.\n\r",ch);
      return TRUE;
    }
  else
    return FALSE;
}


int
tk_coffin (OBJ *coffin, CHAR *ch, int cmd, char *arg)
{
  int inside_coffin;
  OBJ *tmp, *next;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  if(!ch)
    return(FALSE);

  inside_coffin = real_room(TK_COFFIN_INSIDE);

  if (IS_SET (coffin->obj_flags.value[1], CONT_CLOSED))
    SET_BIT (world[inside_coffin].room_flags, DARK);
  else
    REMOVE_BIT (world[inside_coffin].room_flags, DARK);

  for (tmp = coffin->contains;tmp;tmp = next)
    {
      next = tmp->next_content;
      obj_from_obj (tmp);
      obj_to_room (tmp, inside_coffin);
    }

  if (cmd != CMD_ENTER || !AWAKE (ch))
    return FALSE;

  one_argument (arg,buf);
  if (!isname (buf, OBJ_NAME(coffin)))
    return FALSE;

  if (!IS_SET (coffin->obj_flags.value[1], CONT_CLOSED))
    {
      if(GET_POS(ch)==POSITION_RIDING) {
        send_to_char("Dismount first.\n\r",ch);
        return TRUE;
      }
      act ("$n climbs into $p.", TRUE, ch, coffin, 0, TO_ROOM);
      send_to_char ("You climb into the coffin.\n\r",ch);
      char_from_room (ch);
      char_to_room (ch, inside_coffin);
      act ("$n enters $p through the open lid.", TRUE,ch,coffin,0,TO_ROOM);
      sprintf (buf2, "look");
      command_interpreter (ch, buf2);
    }
  else
    send_to_char ("The coffin seems to be closed.\n\r",ch);

  return TRUE;
}

int
tk_baron (CHAR *baron, CHAR *ch, int cmd, char *arg)
{
  int coffin_nr;
  CHAR *vict;

  coffin_nr = real_room (TK_COFFIN_INSIDE);

  if (cmd)
    return FALSE;

  if (!AWAKE (baron))
    GET_POS (baron) = POSITION_RESTING;

  if(GET_HIT(baron)!=GET_MAX_HIT(baron) && count_mortals_room(baron,TRUE)<1 && !number(0,7))
     spell_layhands(GET_LEVEL(baron), baron, baron, 0);

  if (GET_OPPONENT (baron))
    {
      if (GET_HIT(baron) < GET_MAX_HIT(baron)/3 && CHAR_REAL_ROOM(baron) != coffin_nr)
	{
	  tk_baron_escape (baron, TK_TO_COFFIN);
	  return FALSE;
	}

      if ((vict = tk_get_baron_victim (baron)))
	tk_baron_offensive (baron, vict);
      return FALSE;
    }

  if (time_info.hours > 22 || time_info.hours < 5) /* Night... */
    {
      if (CHAR_REAL_ROOM(baron) == coffin_nr && !number (0,3))
	{
	  tk_baron_escape (baron, TK_TO_WORLD);
	  return FALSE;
	}
    }
  else /* during the daytime... */
    if (CHAR_REAL_ROOM(baron)!= coffin_nr)
      tk_baron_escape (baron, TK_TO_COFFIN);
  return FALSE;
}

int
tk_slaphoff (CHAR *slap, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = 0, *vict2 = 0;

  if (cmd)
    return FALSE;

  if (!AWAKE (slap))
    return FALSE;

  if (GET_OPPONENT (slap))
    {
      if (!(vict = tk_get_baron_victim (slap)))
	return FALSE;

      switch (number (1,5))
	{
	case 1:
	case 2:
	  act ("$n quickly sidesteps behind you and hits you in the backside.",
	       FALSE,slap,0,vict,TO_VICT);
	  act ("$n quickly sidesteps behind $N and hits $M in the backside.",
	       FALSE,slap,0,vict,TO_NOTVICT);
	  act ("You quickly sidestep behind $N and hit $M in the backside.",
	       FALSE,slap,0,vict,TO_CHAR);
	  damage (slap, vict, number(1,100)+200, TYPE_UNDEFINED,DAM_NO_BLOCK);
	  break;

	case 3:
	case 4:
	case 5:
	  vict2 = tk_get_baron_victim (slap);
	  act ("$n throws a handful of coins at you with an\n\rexcellent aim.",
	       FALSE,slap,0,vict,TO_VICT);
	  if (vict2)
	    act ("$n throws a handful of coins at you with an\n\rexcellent aim.",
		 FALSE,slap,0,vict2,TO_VICT);
	  act ("You throw a handful of coins at some of them with an\n\rexcellent aim.",
	       FALSE,slap,0,0,TO_CHAR);
	  damage (slap, vict, number (1,100), TYPE_UNDEFINED,DAM_NO_BLOCK);
	  if (vict2)
	    damage (slap, vict2, number (1,100), TYPE_UNDEFINED,DAM_NO_BLOCK);
	  break;
	}
      return FALSE;
    }

  if (!number(0,1))
    tk_kender_action (slap);
  return FALSE;
}

int
tk_kender (CHAR *kender, CHAR *ch, int cmd, char *arg)
{
  if (cmd)
    return FALSE;

  if (!number(0,1))
    tk_kender_action (kender);
  return FALSE;
}

int
tk_crashed (int room, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];

  if (cmd != CMD_USE)
    return FALSE;

  if (!*arg)
    return FALSE;

  one_argument (arg, buf);

  if (isname(buf, "button red"))
    {
      act ("$n pushes the red button on the wall.",TRUE, ch,0,0,TO_ROOM);
      act ("You push the red button on the wall.",TRUE, ch,0,0,TO_CHAR);
      send_to_room ("You hear a high, whining sound.",room);
      return TRUE;
    }
  else if (isname (buf, "button green"))
    {
      act ("$n pushes the red button on the wall.",TRUE, ch,0,0,TO_ROOM);
      act ("You push the red button on the wall.",TRUE, ch,0,0,TO_CHAR);
      send_to_room ("A low, rumbling sound comes from within the machinery.",
		    room);
      return TRUE;
    }
  else if (isname (buf, "lever"))
    {
      act ("$n pushes the red button on the wall.",TRUE, ch,0,0,TO_ROOM);
      act ("You push the red button on the wall.",TRUE, ch,0,0,TO_CHAR);
      send_to_room ("A bit more black fluid squirts on the floor.",room);
      return TRUE;
    }
  return FALSE;
}
