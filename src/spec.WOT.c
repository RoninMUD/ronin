/* spec.WOT.c - Specs for Wheel of Time
**
** Written by Sephiroth and Shun for RoninMUD
** Last Modification Date: OCT/2017
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

/*
** Definitions
*/
#ifdef TEST_SITE
#define CHANCE_SAY 100
#define CHANCE_EMO 100
#else
#define CHANCE_SAY 15
#define CHANCE_EMO 10
#endif

/* Specs for merchants in WOT
**
** The merchant will have his gold amount increased
** by 10.000 every minute if under 2.000.000.
*/

#define MERCHANT      18700
#define MERCHANT_SIGH 2

int wot_merchant(CHAR *mob, CHAR *vict, int cmd, char *argument) {
  char buf[MIL] = { 0 };
  size_t i = 0;

  const char const *msg[] = {
    "I wish the Aiel would reopen the traderoutes to the Waste. Grandpa always said that the silk trade was the most profitable.",
    "Are you also heading to Tarvins Gap? I hear they are gathering an army there.",
    "I wish I wasn't heading to Tarvins Gap. Thats where the Trollocs will invade first. They better hold off the invasion until I've unloaded my goods.",
    "Trade is hard these days. All major cities are closed, and only a few are allowed to enter.",
    "I have to make due trading in the smaller towns, while Gerarld has gotten his permit. Unfare I tell you!",
    "I heard Tar Valon will be the first city to reopen their gates to normal trafic, but the city was still closed on my last visit."
  };

  if (!mob || !AWAKE(mob)) return FALSE;

  if(cmd == MSG_TICK) {
    if(GET_GOLD(mob) < 2000000) {
      GET_GOLD(mob) += 10000;
    }
  }

  if ((cmd == MSG_MOBACT) &&
      !mob->specials.fighting &&
      (vict = get_random_target(mob, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE)) &&
      chance(CHANCE_EMO)) {
    i = number(0, NUMELEMS(msg) - 1);

    if (i == MERCHANT_SIGH) {
      strncpy(buf, "sigh", sizeof(buf) - 1);
      command_interpreter(mob, buf);
    }

    strncpy(buf, msg[i], sizeof(buf) - 1);
    do_say(mob, buf, CMD_SAY);
  }

  return FALSE;
}


/* Specs for hunters in WOT
**
** The hunter will have his exp amount increased
** by 12.000 every minute if under 2.700.000.
*/

#define HUNTER     18702

int wot_hunter(CHAR *mob, CHAR *vict, int cmd, char *argument) {
  char buf[MIL] = { 0 };
  size_t i = 0;

  const char const *msg[] = {
    "To Tarvins Gap!",
    "Stop following me! I will not lead you to the Horn of Valere. It's mine!",
    "Hey! Stop following me!",
    "Did you hear the news? Some army has made landfall in Ebu Dar! I'm heading north, that's for sure!"
  };


  if (!mob || !AWAKE(mob)) return FALSE;

  if(cmd==MSG_TICK) {
    if(GET_EXP(mob) < 2700000) {
      GET_EXP(mob)+=12000;
    }
  }

  if ((cmd == MSG_MOBACT) &&
      !mob->specials.fighting &&
      (vict = get_random_target(mob, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE))) {

    if (chance(CHANCE_SAY)) {
      i = number(0, NUMELEMS(msg) - 1);
      strncpy(buf, msg[i], sizeof(buf) - 1);
      do_say(mob, buf, CMD_SAY);
    }
  }

  return FALSE;
}


/* Specs for tinker child in WOT
**
** The tinker child only has flavor text.
*/

#define TINKER      18704

int wot_tinker(CHAR *mob, CHAR *vict, int cmd, char *argument) {
  char buf[MIL] = { 0 };
  size_t i = 0;

  static const char const *msg[] = {
    "Do you follow the way of the leaf? I never hit anybody. Carl did once and father almost abandoned him in Camelyn!",
    "Hello!",
    "You're weird!"
  };

  if (!mob || !AWAKE(mob)) return FALSE;

  if ((cmd == MSG_MOBACT) &&
      !mob->specials.fighting &&
      (vict = get_random_target(mob, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE))) {

    if (chance(CHANCE_SAY)) {
      i = number(0, NUMELEMS(msg) - 1);
      strncpy(buf, msg[i], sizeof(buf) - 1);
      do_say(mob, buf, CMD_SAY);
    }

    if (chance(CHANCE_EMO)) {
      switch (number(0,4)) {
      case 0:
        snprintf(buf, sizeof(buf) - 1, "point %s", GET_NAME(vict));
        break;
      case 1:
        strncpy(buf, "dance", sizeof(buf) - 1);
        break;
      case 2:
        snprintf(buf, sizeof(buf) - 1, "smile %s", GET_NAME(vict));
        break;
      case 3:
        strncpy(buf, "tantrum", sizeof(buf) - 1);
        break;
      case 4:
      default:
        strncpy(buf, "giggle", sizeof(buf) - 1);
        break;
      };

      command_interpreter(mob, buf);
    }
  }

  return FALSE;
}


/* Specs for Seeker in WOT
**
** Seeker only has flavor text.
*/

#define SEEKER      18705

int wot_seeker(CHAR *mob, CHAR *vict, int cmd, char *argument) {
  char buf[MIL] = { 0 };
  size_t i = 0;

  const char const *msg[] = {
    "Greetings stranger! Do you know the song?",
    "Nobody remembers the song, but one day we will find it!",
    "Welcome to our camp strangers. Please sit down and rest your feet while we get to know each other."
  };

  if (!mob || !AWAKE(mob)) return FALSE;

  if ((cmd == MSG_MOBACT) &&
      !mob->specials.fighting &&
      (vict = get_random_target(mob, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE))) {

    if (chance(CHANCE_SAY)) {
      i = number(0, NUMELEMS(msg) - 1);
      strncpy(buf, msg[i], sizeof(buf) - 1);
      do_say(mob, buf, CMD_SAY);
    }

    if (chance(CHANCE_EMO)) {
      buf[0] = '\0';

      switch (number(0,3)) {
      case 0:
        snprintf(buf, sizeof(buf) - 1, "greet %s", GET_NAME(vict));
        break;
      case 1:
        if ((vict = get_ch(TINKER, ROOM, CHAR_REAL_ROOM(mob)))) {
          snprintf(buf, sizeof(buf) - 1, "ruffle %s", GET_NAME(vict));
        }
        break;
      case 2:
        snprintf(buf, sizeof(buf) - 1, "smile %s", GET_NAME(vict));
        break;
      case 3:
      default:
        /* handled by failsafe check later */
        break;
      };

      /* hum if no other command set */
      if (!buf[0]) {
        strncpy(buf, "hum", sizeof(buf) - 1);
      }

      command_interpreter(mob, buf);
    }
  }

  return FALSE;
}


/* Specs for cityguards in WOT
**
** The tinker child only has flavor text.
*/

#define GUARD_QUEEN      18706
#define GUARD_1          18707
#define GUARD_2          18709
#define GUARD_3          18710
#define GUARD_4          18711
#define GUARD_5          18712
#define GUARD_6          18713
#define GUARD_7          18714
#define GUARD_8          18715

int wot_guard(CHAR *mob, CHAR *vict, int cmd, char *argument) {
  char buf[MIL] = { 0 };
  size_t i = 0;

  static const char const *msg[] = {
    "Halt! This city is closed for foreigners for the time being.",
    "Hello travellers! The city is currently closed for foreigners.",
    "Hold on strangers! We want no foreigners in the city at the moment. The gates are closed!"
  };

  if (!mob || !AWAKE(mob)) return FALSE;

  if ((cmd == MSG_MOBACT) &&
      (vict = get_random_target(mob, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE))) {

    if (chance(CHANCE_SAY)) {
      i = number(0, NUMELEMS(msg) - 1);
      strncpy(buf, msg[i], sizeof(buf) - 1);
      do_say(mob, buf, CMD_SAY);
    }

    if (chance(CHANCE_EMO)) {
      snprintf(buf, sizeof(buf) - 1, "peer %s", GET_NAME(vict));
      command_interpreter(mob, buf);
    }
  }

  return FALSE;
}


/* Specs for whitecloak in WOT
**
** Whitecloak only has flavor text
*/

#define WHITECLOAK     18708

int wot_whitecloak(CHAR *mob, CHAR *vict, int cmd, char *argument) {
  char buf[MIL] = { 0 };
  size_t i = 0;

  const char const *msg_fight[] = {
    "Die Darkfriend!",
    "I sense the evil in you! Confess!",
    "I will get the truth from you in the end!"
  };

  const char const *msg_notfight[] = {
    "Do you walk in the light?",
    "Do you fear the light?"
  };


  if (!mob || !AWAKE(mob)) return FALSE;

  if ((cmd == MSG_MOBACT) &&
      (vict = get_random_target(mob, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE))) {

    if (mob->specials.fighting) {
      if (chance(2 * CHANCE_SAY)) {
        i = number(0, NUMELEMS(msg_fight) - 1);
        strncpy(buf, msg_fight[i], sizeof(buf) - 1);
        do_say(mob, buf, CMD_SAY);
      }
    }
    else {
      if (chance(CHANCE_SAY)) {
        i = number(0, NUMELEMS(msg_notfight) - 1);
        strncpy(buf, msg_notfight[i], sizeof(buf) - 1);
        do_say(mob, buf, CMD_SAY);
      }
    }
  }

  return FALSE;
}


/* Specs for Elyas in WOT
**
** Elyas will try to summon a wolf to attack random char in room every
** MOB_ACT with 50% chance of success
*/

#define ELYAS      18723
#define WOLF       18724
#define MAX_WOLVES 5

int wot_elyas(CHAR *elyas, CHAR *vict, int cmd, char *argument) {
  CHAR *wolf = NULL;
  CHAR *next_wolf = NULL;
  int num_wolves = 0;

  if (!elyas || !AWAKE(elyas)) return FALSE;

  if (cmd == MSG_MOBACT) {
    if (elyas->specials.fighting &&
        (vict = get_random_victim(elyas)) &&
        chance(50)) {
      /* 50% chance of summoning a wolf and have it ambush random char in room
      (This spec will work pretty much like for the Tribune of Pompeii
      only that wolf is going to ambush its target)
      Text for the spec :
      TO_VICT : Seemingly out of nowhere, a wolf emerges and charges you!
      TO_ROOM : Seemingly out of nowhere, a wolf emerges and charges VICT!
      TO_CHAR : As the fighters are occupied, you leave your cover and charges VICT. */

      /* count wolves */
      for (wolf = world[CHAR_REAL_ROOM(elyas)].people; wolf; wolf = next_wolf) {
        next_wolf = wolf->next_in_room;
        if (IS_MOB(wolf) && (V_MOB(wolf) == WOLF)) num_wolves++;
      }

      if (num_wolves < MAX_WOLVES) {
        act("Seemingly out of nowhere, a wolf emerges and charges $N!", TRUE, elyas, 0, vict, TO_NOTVICT);
        act("Seemingly out of nowhere, a wolf emerges and charges you!", TRUE, elyas, 0, vict, TO_VICT);
        wolf = read_mobile(WOLF, VIRTUAL);
        char_to_room(wolf, CHAR_REAL_ROOM(vict));
        set_fighting(wolf, vict);
      }
    }
  }

  return FALSE;
}

/* Assign specs to mobs/objects */
void assign_WOT() {
  assign_mob(MERCHANT,    wot_merchant);
  assign_mob(HUNTER,      wot_hunter);
  assign_mob(TINKER,      wot_tinker);
  assign_mob(SEEKER,      wot_seeker);
  assign_mob(GUARD_QUEEN, wot_guard);
  assign_mob(GUARD_1,     wot_guard);
  assign_mob(GUARD_2,     wot_guard);
  assign_mob(GUARD_3,     wot_guard);
  assign_mob(GUARD_4,     wot_guard);
  assign_mob(GUARD_5,     wot_guard);
  assign_mob(GUARD_6,     wot_guard);
  assign_mob(GUARD_7,     wot_guard);
  assign_mob(GUARD_8,     wot_guard);
  assign_mob(WHITECLOAK,  wot_whitecloak);
  assign_mob(ELYAS,       wot_elyas);
}
