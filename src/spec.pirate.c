/*
///   Special Procedure Module                   Orig. Date 03-02-1994
///
///   Spec.pirate.c --- Specials for The Pirate Ship by Drac
///
///   Designed and coded by Kafka of (kafka@modeemi.cs.tut.fi)
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
#include "fight.h"
#include "utility.h"
#include "cmd.h"
#include "spec_assign.h"

void stop_fighting (CHAR *ch);
void hit (CHAR *ch, CHAR *vict, int type);
char *one_argument(char *arg, char *buf);

/* Zone */

#define ZONE_SHIP           7500
#define ZONE_SEA            7200

/* Mobiles */

#define PIRATE_CAPTAIN      ITEM(ZONE_SHIP,0)
#define PIRATE_MATE         ITEM(ZONE_SHIP,1)
#define PIRATE_CREW         ITEM(ZONE_SHIP,2)
#define SCORPION            ITEM(ZONE_SHIP,5)
#define SKELETON            ITEM(ZONE_SHIP,6)

/* Objects */

#define CAPTAIN_PISTOL      ITEM(ZONE_SHIP,2)
#define PIRATE_PISTOL       ITEM(ZONE_SHIP,5)
#define TREASURE_MAP        ITEM(ZONE_SHIP,7)
#define TREASURE            ITEM(ZONE_SHIP,8)
#define SILK_GLOVES         ITEM(ZONE_SHIP,9)
#define DIAMOND_RING        ITEM(ZONE_SHIP,10)
#define DIAMOND_BRACELET    ITEM(ZONE_SHIP,11)
#define TRUST_FLAG          ITEM(ZONE_SHIP,17)
#define SHOVEL              ITEM(ZONE_SHIP,18)
#define KEYRING             ITEM(ZONE_SHIP,19)
#define CHESTV              ITEM(ZONE_SHIP,21)
#define SILK_PANTS          ITEM(ZONE_SHIP,22)
#define MARAUDERS_BOOTS		7524

/* Rooms */

#define STORAGE_ROOM        ITEM(ZONE_SHIP,4)
#define TREASURE_SPOT       ITEM(ZONE_SHIP,13)
#define TO_THE_SEA          ITEM(ZONE_SEA,42)

#define ATTACK_SHOOT        1
#define ATTACK_PUNCH        2
#define ATTACK_WINDOW       3
#define ATTACK_TABLE        4

void
PirateAttack(CHAR *ch, CHAR *vict, int type) {
  int sea;
  OBJ *obj;

  sea = real_room(TO_THE_SEA);

  switch (type) {

    case ATTACK_SHOOT:
      obj = EQ(ch, HOLD);
      act("$n shoots straight at you with $p.", FALSE, ch, obj, vict, TO_VICT);
      act("$n shoots straight at $N with $p.", FALSE, ch, obj, vict, TO_NOTVICT);
      act("You shoot straight at $N with $p.", FALSE, ch, obj, vict, TO_CHAR);
      damage(ch, vict, number(100, 200), TYPE_UNDEFINED, DAM_NO_BLOCK);
      break;

    case ATTACK_PUNCH:
      if (IS_AFFECTED(vict, AFF_INVUL) && !breakthrough(ch, vict, SKILL_PUNCH, BT_INVUL)) {
        act("$n punches you with a left hook to the stomach. Ouch!",
          FALSE, ch, 0, vict, TO_VICT);
        act("$n punches $N with a left hook to the stomach. Ouch!",
          FALSE, ch, 0, vict, TO_NOTVICT);
        act("You punch $N with a left hook to the stomach.",
          FALSE, ch, 0, vict, TO_CHAR);
        damage(ch, vict, 100, SKILL_PUNCH, DAM_PHYSICAL);
      }
      else {
        act("$n attempts to punch you, but just looks foolish.", FALSE, ch, 0, vict, TO_VICT);
        act("$n attempts to punch $N, but just looks foolish.", FALSE, ch, 0, vict, TO_NOTVICT);
        act("You attempt to punch $N, but just look foolish instead.", FALSE, ch, 0, vict, TO_CHAR);
      }
      break;

    case ATTACK_WINDOW:
      act("$n grabs $N and throws $M through the window, into the stormy sea.",
        FALSE, ch, 0, vict, TO_NOTVICT);
      act("$n grabs you and throws you through the window, into the stormy sea.",
        FALSE, ch, 0, vict, TO_VICT);
      act("You grab $N and throw $M through the window, into the stormy sea.",
        FALSE, ch, 0, vict, TO_CHAR);
      if (GET_OPPONENT(vict)) {
        if (GET_OPPONENT(GET_OPPONENT(vict)) == vict)
          stop_fighting(GET_OPPONENT(vict));
        stop_fighting(vict);
      }
      char_from_room(vict);
      char_to_room(vict, sea);
      act("$n arrives flying through the ship window.",
        FALSE, vict, 0, 0, TO_ROOM);
      break;

    case ATTACK_TABLE:
      act("$n places a powerful kick on the table, and it falls on top of $N.",
        FALSE, ch, 0, vict, TO_NOTVICT);
      act("$n places a powerful kick on the table, and it falls on top of you.",
        FALSE, ch, 0, vict, TO_VICT);
      act("You place a powerful kick on the table, and it falls on top of $N.",
        FALSE, ch, 0, vict, TO_CHAR);
      damage(ch, vict, number(50, 100), TYPE_UNDEFINED, DAM_NO_BLOCK);
      if (GET_OPPONENT(GET_OPPONENT(vict)) == vict)
        stop_fighting(GET_OPPONENT(vict));
      stop_fighting(vict);
      WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
      break;
  }
}

int
Captain (CHAR *cap, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*temp;
  static char status = 0;

  if (cmd && !GET_OPPONENT(cap)) {
    for (vict=world[CHAR_REAL_ROOM(cap)].people;vict;vict=temp ) {
      temp = vict->next_in_room;  /* Added temp - Ranger June 96 */
      if (is_carrying_obj(vict, TREASURE_MAP)) {
	status = 0;
	act("$n yells in rage at you, 'You bloody thief! You stole my map!'",
	    FALSE,cap,0,vict,TO_VICT);
	act("$n yells in rage at $N, 'You bloody thief! You stole my map!'",
	    FALSE,cap,0,vict,TO_NOTVICT);
	act("You yell in rage at $N, 'You bloody thief! You stole my map!'",
	    FALSE,cap,0,vict,TO_CHAR);
	hit (cap, vict, TYPE_HIT);
	hit (cap, vict, TYPE_HIT);
	return FALSE;
      }
    }
  }

  if (cmd)
    return FALSE;

  if (!(vict = get_random_victim (cap)))
    return FALSE;

  if (GET_OPPONENT(cap)) {
    switch (status) {
    case 0:
      status=1;
      PirateAttack (cap, vict, ATTACK_WINDOW);
      return FALSE;
    case 1:
      status=2;
      PirateAttack (cap, vict, ATTACK_TABLE);
      return FALSE;
    case 2:
    case 3:
    case 4:
    case 5:
      if (is_wearing_obj(cap,CAPTAIN_PISTOL,HOLD)) {
	if (number(0,1))
	  PirateAttack (cap, vict, ATTACK_SHOOT);
	vict = GET_OPPONENT(cap);
	PirateAttack (cap, vict, ATTACK_SHOOT);
	status++;
	return FALSE;
      }
    case 6:
      status = 2;
      return FALSE;
    }
  }
  status = 0;
  return FALSE;
}

int
FirstMate (CHAR *mate, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if (cmd)
    return FALSE;

  if (GET_OPPONENT(mate)) {
    if (!(vict = get_random_victim (mate)))
      return FALSE;

    switch (number(0,2)) {
    case 0:
    case 1:
      PirateAttack (mate, vict, ATTACK_PUNCH);
      return FALSE;
    case 2:
      if (is_wearing_obj(mate,PIRATE_PISTOL,HOLD)) {
	PirateAttack (mate, vict, ATTACK_SHOOT);
      }
    }
  }
  return FALSE;
}

int
Pirate (CHAR *pirate, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if (cmd)
    return FALSE;

  if (GET_OPPONENT(pirate)) {
    if (!(vict = get_random_victim (pirate)))
      return FALSE;

    switch (number(0,3)) {
    case 0:
    case 1:
    case 2:
      PirateAttack (pirate, vict, ATTACK_PUNCH);
      return FALSE;
    case 3:
      if (is_wearing_obj(pirate,PIRATE_PISTOL,HOLD)) {
	PirateAttack (pirate, vict, ATTACK_SHOOT);
      }
    }
  }
  return FALSE;
}

int
Scorpion (CHAR *sc, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  struct affected_type_5 af;

  if (cmd)
    return FALSE;

  if (GET_OPPONENT(sc)) {
    if (!(vict = get_random_victim (sc)))
      return FALSE;

    /* Scorpion stings poison */

    af.type = SPELL_POISON;
    af.modifier = -6;
    af.location = APPLY_STR;
    af.duration = number(2,4);
    af.bitvector = AFF_POISON;
    af.bitvector2 = 0;
    affect_to_char (vict, &af);

    act("$n strikes $N with $s sting, poisoning $M.",
	FALSE,sc,0,vict,TO_NOTVICT);
    act("$n strikes you with $s sting, poisoning you.",
	FALSE,sc,0,vict,TO_VICT);
    act("You strike $N with your sting, poisoning $M.",
	FALSE,sc,0,vict,TO_CHAR);
    send_to_char("You feel your blood burning.\n",vict);

    damage (sc, vict, number(120,180), TYPE_HIT,DAM_NO_BLOCK);
  }
  return FALSE;
}

int
StorageRoom (int room, CHAR *ch, int cmd, char *arg) {
  if(!ch)
    return(FALSE);
  if (!IS_MORTAL(ch))
    return FALSE;

  if (cmd == CMD_EAST) {
    if (is_wearing_obj(ch, TRUST_FLAG, HOLD))
      return FALSE;
    act("$n tries to go through the low doorway to the east, but stops suddenly.",FALSE,ch,0,0,TO_ROOM);
    send_to_char ("You try to go east through the low doorway, but something stops you.\n",ch);
    return TRUE;
  }
  return FALSE;
}

int
Shovel (OBJ *shovel, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  OBJ *che, *rewards;
  CHAR *guard;
  int scorpion, skeleton;
  int pants, chest, d_bracelet, d_ring, treasure, gloves;


  if (cmd != CMD_USE)
    return FALSE;

  if (!IS_MORTAL(ch))
    return FALSE;

  scorpion = real_mobile(SCORPION);
  skeleton = real_mobile(SKELETON);

  chest = real_object(CHESTV);
  d_bracelet = real_object(DIAMOND_BRACELET);
  d_ring = real_object(DIAMOND_RING);
  treasure = real_object(TREASURE);
  gloves = real_object(SILK_GLOVES);
  pants = real_object(SILK_PANTS);

  if (shovel == EQ(ch,HOLD)) {
    one_argument (arg, buf);

    if (!isname(buf, OBJ_NAME(shovel)))
      return FALSE;

    if (GET_OPPONENT(ch)) {
      send_to_char ("You're much too busy to dig something...\n",ch);
      return TRUE;
    }

    if (V_ROOM(ch) != TREASURE_SPOT) {
      act("$n starts digging with $p, but stops suddenly.",
	  FALSE,ch,shovel,0,TO_ROOM);
      send_to_char("You start to dig, but then you get a strange feeling.",ch);
      return TRUE;
    }

    switch (OBJ_SPEC(shovel)) {
    case 0:
      /* The Scorpion wakes up under the sands. */
      OBJ_SPEC(shovel) = 1;

      guard = read_mobile (scorpion, REAL);
      char_to_room (guard, CHAR_REAL_ROOM(ch));

      act("As you start to dig a hole, the sands shift and $N crawls out.",
	  FALSE,ch,0,guard, TO_CHAR);
      act("As $n starts to dig a hole, the sands shift and $N crawls out.",
	  FALSE,ch,0,guard, TO_ROOM);
      hit (guard, ch, TYPE_HIT);
      break;

    case 1:
      /* The Skeleton guardian rises up from its resting place. */
      OBJ_SPEC(shovel) = 2;

      guard = read_mobile(skeleton, REAL);
      char_to_room (guard, CHAR_REAL_ROOM(ch));

      act("$n rises up from the sands, slashing at you with $s claws.",
	  FALSE,guard,0,ch, TO_VICT);
      act("$n rises up from the sands, slashing at $N with $s claws.",
	  FALSE,guard,0,ch, TO_NOTVICT);
      act("You rise up from the sands, slashing at $N with $s claws.",
	  FALSE,guard,0,ch, TO_CHAR);
      hit (guard, ch, TYPE_HIT);
      break;

    case 2:
      /* Finally the chest is revealed. */
      che = read_object(chest, REAL);
      obj_to_room(che, CHAR_REAL_ROOM(ch));

      if (number(0,1)) { rewards = read_object(treasure, REAL);
			 obj_to_obj(rewards, che); }
      if (number(0,1)) { rewards = read_object(d_bracelet, REAL);
			 obj_to_obj(rewards, che); }
      if (number(0,1)) { rewards = read_object(d_ring, REAL);
			 obj_to_obj(rewards, che); }
      if (number(0,1)) { rewards = read_object(gloves, REAL);
			 obj_to_obj(rewards, che); }
      if (number(0,1)) { rewards = read_object(pants, REAL);
			 obj_to_obj(rewards, che); }

      act("Finally you hit your $o on the lid of $P, and the shaft breaks.",
	  FALSE,ch,shovel, che, TO_CHAR);
      act("Finally $n hits $s $o on the lid of $P, and the shaft breaks.",
	  FALSE,ch,shovel, che, TO_ROOM);
      unequip_char (ch, HOLD);
      extract_obj (shovel);
      break;
    }
    return TRUE;
  }
  return FALSE;
}

int
Treasure (int room, CHAR *ch, int cmd, char *arg) {
  OBJ *chest;
  if(!ch)
     return(FALSE);

  if (count_mortals_room(ch, TRUE))
    return FALSE;

  if ((chest = get_obj_room(CHESTV, world[room].number))) {
    if (!chest->contains)
      extract_obj (chest);
  }
  return FALSE;
}

int
Chest (OBJ *chest, CHAR *ch, int cmd, char *arg) {
  OBJ *key;
  char buf[MAX_INPUT_LENGTH];
  int keyring;

  keyring = real_object(KEYRING);

  if (cmd == CMD_UNLOCK) {
    one_argument(arg,buf);

    if (!isname(buf, OBJ_NAME(chest)))
      return FALSE;

    if (!(key = get_obj_in_list_num (keyring, ch->carrying)))
      return FALSE;

    send_to_char ("*CLICK*\n\r*CRACK*\n\r",ch);
    REMOVE_BIT(chest->obj_flags.value[1], CONT_CLOSED);
    act("$n opens $p, and the key breaks.",FALSE,ch,chest,0,TO_ROOM);
    extract_obj (key);
    return TRUE;
  }
  return FALSE;
}

int marauders_boots(OBJ *boots, CHAR *ch, int cmd, char *arg)
{
  bool bReturn = FALSE;

  switch(cmd)
  {
	case CMD_PRACTICE:
	  /* with all the SKILL modifiers, this will prevent skill-looping with the boots */
      if (ch != boots->equipped_by) return bReturn;
      if (!ch) return bReturn;
      if(AWAKE(ch) && *arg)
      {
	    send_to_char("Your feet pull you away from the practice grounds; there's marauding to be done!\n\r",ch);
	    bReturn = TRUE;
      }
	  break;
	case MSG_BEING_REMOVED:
      if (ch != boots->equipped_by) return bReturn;
      if (!ch) return bReturn;
      if(boots==EQ(ch,WEAR_FEET))
      {
        unequip_char(ch,WEAR_FEET);
        boots->affected[0].modifier=2; /* Location 0 should be set as DAMROLL */
        boots->affected[1].location=0; /* Location 1 varies by class */
        boots->affected[1].modifier=0;
        boots->affected[2].location=0; /* Location 2 varies by class */
        boots->affected[2].modifier=0;
        boots->obj_flags.value[0]=7;
	    equip_char(ch,boots,WEAR_FEET);
	    OBJ_SPEC(boots)=0;
      }
      break;
    case MSG_TICK:
      ch = boots->equipped_by;
      if (!ch) return bReturn;

      if(OBJ_SPEC(boots)!=GET_CLASS(ch))
      {
	    unequip_char(ch,WEAR_FEET);
	    switch(GET_CLASS(ch))
	    {
	      case CLASS_NINJA:
	        boots->affected[1].location=APPLY_HITROLL;
	        boots->affected[1].modifier=2;
	        boots->affected[2].location=APPLY_SKILL_ASSAULT;
	        boots->affected[2].modifier=7;
	        boots->obj_flags.value[0]=7;
            act("$n's steps seem to soften as $e slinks into the shadows.",0,ch,0,0,TO_ROOM);
            act("Your steps seem to soften as you slink into the shadows.",0,ch,0,0,TO_CHAR);
	        break;
	      case CLASS_THIEF:
	        boots->affected[1].location=APPLY_HP_REGEN;
	        boots->affected[1].modifier=3;
	        boots->affected[2].location=APPLY_SKILL_CIRCLE;
	        boots->affected[2].modifier=7;
	        boots->obj_flags.value[0]=13;
            act("$n whispers things to passing knaves - organized sculduggery is afoot.",0,ch,0,0,TO_ROOM);
            act("You whisper things to passing knaves - organized sculduggery is afoot.",0,ch,0,0,TO_CHAR);
	        break;
	      case CLASS_BARD:
	        boots->affected[1].location=APPLY_MANA_REGEN;
	        boots->affected[1].modifier=3;
	        boots->affected[2].location=APPLY_MANA;
	        boots->affected[2].modifier=19;
	        boots->obj_flags.value[0]=7;
            act("$n adds a gay spring to $s step and sets about some musical mischief.",0,ch,0,0,TO_ROOM);
            act("You add a gay spring to your step and set about some musical mischief.",0,ch,0,0,TO_CHAR);
	        break;
	      default:
            boots->affected[0].modifier=2; /* Location 0 should be set as DAMROLL */
            boots->affected[1].location=0; /* Location 1 varies by class */
            boots->affected[1].modifier=0;
            boots->affected[2].location=0; /* Location 2 varies by class */
            boots->affected[2].modifier=0;
            boots->obj_flags.value[0]=7;
            act("$n's boots seem too large for $s feet.",0,ch,0,0,TO_ROOM);
            act("Your boots seem too large for your feet.",0,ch,0,0,TO_CHAR);
	        break;
        }/* end switch(GET_CLASS(ch)) */
        equip_char(ch,boots,WEAR_FEET);
	    OBJ_SPEC(boots)=GET_CLASS(ch);
	  }/* end if(OBJ_SPEC(boots)!=GET_CLASS(ch)) */
	  break;
	default:
	  break;
  }/* end switch(cmd) */
  return bReturn;
}/* end marauders_boots() */

 /**********************************************************************\
 |* End Of the Special procedures for Pirate Ship                      *|
 \**********************************************************************/

void assign_pirate (void) {

//  assign_mob(PIRATE_CAPTAIN, Captain);
// ^^ chained in spec.elmuseo.c
  assign_mob(PIRATE_MATE,   FirstMate);
  assign_mob(PIRATE_CREW,   Pirate);
  assign_mob(SCORPION,   Scorpion);

  assign_obj(SHOVEL ,  Shovel);
  assign_obj(CHESTV, Chest);
  assign_obj(MARAUDERS_BOOTS, marauders_boots);

  assign_room(TREASURE, Treasure);
  assign_room(STORAGE_ROOM, StorageRoom);

}

