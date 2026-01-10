/*spec.darkspire.c - Specs for Dark Spire by Fisher

     Written by Fisher for RoninMUD
     Last Modification Date: 2/11/2024

     Basic Specs for the mobs and rooms in the zone.
*/
/*System Includes */
#include <string.h>

/*Ronin Includes */
#include "structs.h"
#include "utils.h"
#include "act.h"
#include "db.h"
#include "char_spec.h"
#include "cmd.h"
#include "comm.h"
#include "constants.h"
#include "enchant.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "mob.spells.h"
#include "spec_assign.h"
#include "spells.h"
#include "subclass.h"
#include "utility.h"

/*Rooms */

/*Objects */

/*Mobs */
#define GOLD_MOB 30110
#define TAMUR_WERE_BESAT 30111
#define THEURGI_LEADER 30114
#define DEAD_EYE_VAMPIRE 30118
#define HALF_CHANGED_WEREWOLF 30121
#define TROMOD_GARGOYLE 30122
#define XALGATH_DEMON 30123
#define STEFAN_ALCHEMIST 30125
#define VOID_ELEMENTAL 30126

/*Miscellaneous strings */
// Generic States that are shifted to indicate different stages.
#define STATE1 (1 << 0) // 1
#define STATE2 (1 << 1) // 2
#define STATE3 (1 << 2) // 4
#define STATE4 (1 << 3) // 8

/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */

/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */

int darkspire_goldmob(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

  char *cough = "cough";
  int reward;
  /*Don't waste any more CPU time if no one is in the room. */
  // We want to make sure the gold increments even if no one is there.
  // if (count_mortals_room(mob, TRUE) < 1) return FALSE;

  // Have the mob interact
  if (cmd == MSG_TICK)
  {
    if (chance(70))
    {

      // IF we hit the loop, we want to randomly bring the gold count up or down.

      switch (number(0, 5))
      {
      case (0):
        do_social(mob, cough, CMD_SOCIAL);
        reward = number(10000, 40000);
        act("$n coughts softly and gold coins scatter to the floor before disolving.", 0, mob, 0, 0, TO_ROOM);
        GET_GOLD(mob) -= reward;
        break;
      case (1):
        reward = number(10000, 40000);
        if (GET_GOLD(mob) < 3000000)
        {
          GET_GOLD(mob) += reward;
        }
        break;
      case (2):
        do_social(mob, cough, CMD_SOCIAL);
        act("$n coughts hard and gold coins scatter to the floor before disolving.", 0, mob, 0, 0, TO_ROOM);
        reward = number(60000, 100000);
        GET_GOLD(mob) -= reward;
        break;
      case (3):
        reward = number(10000, 40000);
        if (GET_GOLD(mob) < 1000000)
        {
          GET_GOLD(mob) += reward;
        }
        break;
      case (4):
        reward = number(20000, 40000);
        if (GET_GOLD(mob) < 1500000)
        {
          GET_GOLD(mob) += reward;
        }
        break;
      case (5):
        reward = number(20000, 60000);
        if (GET_GOLD(mob) < 2000000)
        {
          GET_GOLD(mob) += reward;
        }
        break;
      default:
        break;
      }
    }

    return FALSE;
  }

  return FALSE;
}

int darkspire_tamur(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

  // These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict;

  // Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1)
    return FALSE;

  char *tamur_speak[4] = {"Pathetic creatures. Invading my space.", "Why are you here. Go away.", "This is agony enough. You are just making it worse.", "Begone and leave me in peace."};

  switch (cmd)
  {
  case MSG_MOBACT:

    // Have him to chat to you.
    if (chance(55))
    {
      sprintf(buf, "%s", tamur_speak[number(0, NUMELEMS(tamur_speak) - 1)]);

      do_say(mob, buf, CMD_SAY);
    }

    // if fighting - spec different attacks
    if (mob->specials.fighting)
    {
      // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
      // Each Case statement that has an action needs to break out at the end.

      switch (number(0, 2))
      {
      case 0:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n growls at $N in a low pitched rumble.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n growls at you in a low pitched rumble.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is hit with a clawed hand.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are hit by a clawed hand.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 1:
        break;
      case 2:
        break;
      default:
        break;
      }
    }

    // can add an else branch here if you want them to act but not in combat.

    break;
  }

  return FALSE;
}

int darkspire_theurgi(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

  // These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;

  // Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1)
    return FALSE;

  char *theurgi_speak[4] = {"Welcome to the tower. I hope you enjoy your visit.", "Why have you come to the spire?", "Sit back and relax after the long climb to the top.", "The spire doesnt want you here. Please leave."};

  switch (cmd)
  {
  case MSG_MOBACT:

    // Have him to chat to you.
    if (chance(55))
    {
      sprintf(buf, "%s", theurgi_speak[number(0, NUMELEMS(theurgi_speak) - 1)]);

      do_say(mob, buf, CMD_SAY);
    }

    // if fighting - spec different attacks
    if (mob->specials.fighting)
    {
      // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
      // Each Case statement that has an action needs to break out at the end.

      switch (number(0, 2))
      {
      case 0:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n points at $N with a single finger.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n points at you with a single finger.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is hit with a burst of dark energy.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are hit with a burst of dark energy.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 1:
        break;
      case 2:
        act("$n shouts This is the real power of the Spire.", 0, mob, 0, 0, TO_ROOM);
        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
        {
          next_vict = CHAR_NEXT_IN_ROOM(vict);
          if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
            continue;
          damage(mob, vict, 700, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      default:
        break;
      }
    }

    // can add an else branch here if you want them to act but not in combat.

    break;
  }

  return FALSE;
}

int darkspire_deadeye_vampire(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  // These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;

  // Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1)
    return FALSE;

  char *vampire_speak[4] = {"Let me taste your delicious blood.", "Why are you mortals here.", "Stop invading my space and leave.", "Be afraid of what lies further down."};

  switch (cmd)
  {
  case MSG_MOBACT:

    // Have him to chat to you.
    if (chance(55))
    {
      sprintf(buf, "%s", vampire_speak[number(0, NUMELEMS(vampire_speak) - 1)]);

      do_say(mob, buf, CMD_SAY);
    }

    // if fighting - spec different attacks
    if (mob->specials.fighting)
    {
      // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
      // Each Case statement that has an action needs to break out at the end.

      switch (number(0, 2))
      {
      case 0:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n bites $N on the neck.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n bites you on the neck.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is bitten on the neck.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are bitten on the neck.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 800, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 1:
        break;
      case 2:
        act("$n shrieks loudly as bats fill the room, hitting everyone in their path.", 0, mob, 0, 0, TO_ROOM);
        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
        {
          next_vict = CHAR_NEXT_IN_ROOM(vict);
          if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
            continue;
          damage(mob, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      default:
        break;
      }
    }
    // can add an else ranch here if you want them to act but not in combat.

    break;
  }

  return FALSE;
}

int darkspire_werewolf(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  // These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict;

  // Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1)
    return FALSE;

  char *werewolf_speak[4] = {"The werewolves will rise again.", "Feel the wrath of a true creature of the night.", "Claws and Fangs are better then any weapon you possess.", "I can't wait to sink my fangs into you."};

  switch (cmd)
  {
  case MSG_MOBACT:

    // Have him to chat to you.
    if (chance(55))
    {
      sprintf(buf, "%s", werewolf_speak[number(0, NUMELEMS(werewolf_speak) - 1)]);

      do_say(mob, buf, CMD_SAY);
    }

    // if fighting - spec different attacks
    if (mob->specials.fighting)
    {
      // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
      // Each Case statement that has an action needs to break out at the end.

      switch (number(0, 5))
      {
      case 0:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n slashes at $N with a clawed hand.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n slashes at you with a clawed hand.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is hit with a clawed hand.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are hit by a clawed hand.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 1:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n snaps at $N with a vicious snap of its jaws.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n snaps at you with a vicious snap of its jaws.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is biten with a snap of its jaws.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are bite hard with a snap of its jaws.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 400, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 2:
      case 3:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n lunges at $N with fangs bared and claws outstretched.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n lunges at you with fangs bared and claws outstretched.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is hit with a flurry of claws and teeth.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are hit with a flurry of claws and teeth.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 800, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 4:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n snaps at $N with a vicious snap of its jaws.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n snaps at you with a vicious snap of its jaws.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is biten with a snap of its jaws.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are bite hard with a snap of its jaws.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 400, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 5:
        break;
      default:
        break;
      }
    }

    // can add an else branch here if you want them to act but not in combat.

    break;
  }

  return FALSE;
}

int darkspire_tromod(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  // These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;

  // Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1)
    return FALSE;

  char *tromod_speak[4] = {"", "", "", ""};

  switch (cmd)
  {
  case MSG_MOBACT:

    // Have him to chat to you.
    if (chance(55))
    {
      sprintf(buf, "%s", tromod_speak[number(0, NUMELEMS(tromod_speak) - 1)]);

      do_say(mob, buf, CMD_SAY);
    }

    // if fighting - spec different attacks
    if (mob->specials.fighting)
    {
      // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
      // Each Case statement that has an action needs to break out at the end.

      switch (number(0, 2))
      {
      case 0:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n picks up a stone and throws it at $N.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n picks up a stone and throws it at you.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is hit with a crude stone.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are hit by a crude stone.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 1:
        act("$n pounds the walls causing parts of the ceiling to hit everyone in the room.", 0, mob, 0, 0, TO_ROOM);
        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
        {
          next_vict = CHAR_NEXT_IN_ROOM(vict);
          if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
            continue;
          damage(mob, vict, 850, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 2:
        break;
      default:
        break;
      }
    }

    // can add an else branch here if you want them to act but not in combat.

    break;
  }

  return FALSE;
}

int darkspire_xalgath(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  // These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;

  // Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1)
    return FALSE;

  char *xalgath_speak[5] = {"Pathetic mortals, disapear from my sight.", "Soon my master will come and envelop the world in darkness.", "Fear the coming of the Demon Hordes.", "I can't wait to bathe in your blood.", "How many claws do you think you can handle?"};

  switch (cmd)
  {
  case MSG_MOBACT:

    // Have him to chat to you.
    if (chance(55))
    {
      sprintf(buf, "%s", xalgath_speak[number(0, NUMELEMS(xalgath_speak) - 1)]);

      do_say(mob, buf, CMD_SAY);
    }

    // if fighting - spec different attacks
    if (mob->specials.fighting)
    {
      // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
      // Each Case statement that has an action needs to break out at the end.

      switch (number(0, 5))
      {
      case 0:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n slashes $N with a single black claw.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n slashes you with a single black claw.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is slashed with a single black claw.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are slashed by a single black claw.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
        }

        break;
      case 1:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n slashes $N with two black claws.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n slashes you with two black claws.", 0, mob, 0, vict, TO_VICT);
          sprintf(buf, "%s is slashed with two black claws.", GET_NAME(vict));
          act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You are slashed by two black claws.");
          act(buf, FALSE, mob, NULL, vict, TO_VICT);
          damage(mob, vict, 950, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 2:
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n drives a claw into $N and drains their life essence!", 0, mob, 0, vict, TO_NOTVICT);
          act("$n drives a claw into you, draining your life essence!", 0, mob, 0, vict, TO_VICT);

          damage(mob, vict, number(500, 700), TYPE_UNDEFINED, DAM_MAGICAL);
          GET_HIT(mob) = MIN(GET_MAX_HIT(mob), GET_HIT(mob) + number(300, 500));
        }
        break;
      case 3:
        act("$n slashes his claws at everyone in the room.", 0, mob, 0, 0, TO_ROOM);
        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
        {
          next_vict = CHAR_NEXT_IN_ROOM(vict);
          if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
            continue;
          damage(mob, vict, 850, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
      case 4: /* Hellfire Eruption - Magical AoE */
        act("$n rears back and exhales a wave of hellfire!", 0, mob, 0, 0, TO_ROOM);

        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
        {
          next_vict = CHAR_NEXT_IN_ROOM(vict);
          if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
            continue;

          act("Hellfire scorches your flesh and soul!", 0, mob, 0, vict, TO_VICT);
          damage(mob, vict, number(600, 900), TYPE_UNDEFINED, DAM_MAGICAL);
        }
        break;

      case 5: /* Abyssal Grasp - Single Target Control */
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n extends a claw as shadowy chains burst from the void and bind $N!", 0, mob, 0, vict, TO_NOTVICT);
          act("Shadowy chains erupt from the void and bind you in place!", 0, mob, 0, vict, TO_VICT);

          damage(mob, vict, number(450, 650), TYPE_UNDEFINED, DAM_MAGICAL);
          WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        }
        break;
      default:
        break;
      }
    }

    // can add an else branch here if you want them to act but not in combat.

    break;
  }

  return FALSE;
}

int darkspire_stefan(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  // These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;

  // Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1)
    return FALSE;

  char *stefan_speak[4] = {"I wonder which potion you would enjoy the most?", "Go away and leave me with my experiments.", "I alone can control the power of this tower.", "Face the power of the Spires!"};

  switch (cmd)
  {
  case MSG_MOBACT:

    // Have him to chat to you.
    if (chance(55))
    {
      sprintf(buf, "%s", stefan_speak[number(0, NUMELEMS(stefan_speak) - 1)]);

      do_say(mob, buf, CMD_SAY);
    }

    // if fighting - spec different attacks
    if (mob->specials.fighting)
    {
      // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
      // Each Case statement that has an action needs to break out at the end.

      switch (number(0, 7))
      {
        // Cast Fury on Itself
      case 0:
        do_say(mob, "This yellow one looks good", CMD_SAY);
        if (!affected_by_spell(mob, SPELL_FURY))
        {
          spell_fury(GET_LEVEL(mob), mob, mob, 0);
        }
        break;
        // Cast Haste on Itself
      case 1:
        do_say(mob, "This orange one makes me jittery.", CMD_SAY);
        if (!affected_by_spell(mob, SPELL_HASTE))
        {
          spell_haste(GET_LEVEL(mob), mob, mob, 0);
        }
        break;

      case 2:
        do_say(mob, "I have heard the other test subjects say this green one burns.", CMD_SAY);
        act("$n grabs a green potion from his belt and throws it into the middle of the room.", 0, mob, 0, 0, TO_ROOM);
        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
        {
          next_vict = CHAR_NEXT_IN_ROOM(vict);
          if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
            continue;
          damage(mob, vict, (GET_LEVEL(mob) / 2) + number(350, 550), TYPE_UNDEFINED, DAM_MAGICAL);
        }

        break;
      case 3:
        do_say(mob, "Brown should keep you from moving..", CMD_SAY);
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n throws a brown potion at $N.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n throws a brown potion at you.", 0, mob, 0, vict, TO_VICT);
          damage(mob, vict, 300, TYPE_UNDEFINED, DAM_PHYSICAL);
          WAIT_STATE(vict, PULSE_VIOLENCE * 1);
        }
        break;
      case 4: /* Red Potion - Volatile Explosion */
        do_say(mob, "Ahhh yes… the volatile red concoction.", CMD_SAY);
        act("$n hurls a red potion that detonates in a fiery blast!", 0, mob, 0, 0, TO_ROOM);

        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
        {
          next_vict = CHAR_NEXT_IN_ROOM(vict);
          if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
            continue;

          damage(mob, vict, number(500, 750), TYPE_UNDEFINED, DAM_FIRE);
        }
        break;

      case 5: /* Blue Potion - Mana Freeze */
        do_say(mob, "This blue one chills the mind.", CMD_SAY);
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n smashes a blue potion across $N's chest!", 0, mob, 0, vict, TO_NOTVICT);
          act("$n smashes a blue potion across your chest, freezing your thoughts!", 0, mob, 0, vict, TO_VICT);

          damage(mob, vict, number(350, 500), TYPE_UNDEFINED, DAM_COLD);
          GET_MANA(vict) = MAX(0, GET_MANA(vict) - number(150, 300));
        }
        break;

      case 6: /* Black Potion - Forbidden Elixir */
        do_say(mob, "I really shouldn’t drink this one…", CMD_SAY);
        act("$n uncorks a black potion and drinks deeply.", 0, mob, 0, 0, TO_ROOM);

        /* Heal */
        GET_HIT(mob) = MIN(GET_MAX_HIT(mob), GET_HIT(mob) + number(600, 900));

        /* Temporary rage-like effect */
        if (!affected_by_spell(mob, SPELL_FURY))
        {
          spell_fury(GET_LEVEL(mob), mob, mob, 0);
        }
        break;

      case 7: /* Violet Potion - Unstable Debuff */
        do_say(mob, "Results may vary…", CMD_SAY);
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n splashes a violet potion over $N!", 0, mob, 0, vict, TO_NOTVICT);
          act("$n splashes a violet potion over you!", 0, mob, 0, vict, TO_VICT);

          switch (number(0, 2))
          {
          case 0:
            WAIT_STATE(vict, PULSE_VIOLENCE * 2);
            break;
          case 1:
            GET_MOVE(vict) = MAX(0, GET_MOVE(vict) - number(150, 300));
            break;
          case 2:
            GET_MANA(vict) = MAX(0, GET_MANA(vict) - number(150, 300));
            break;
          }

          damage(mob, vict, number(250, 400), TYPE_UNDEFINED, DAM_MAGICAL);
        }
        break;

      default:
        break;
      }
    }

    // can add an else branch here if you want them to act but not in combat.

    break;
  }

  return FALSE;
}

int darkspire_void_elemental(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  // These are default declarations to give variables to characters.
  CHAR *vict;

  // Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1)
    return FALSE;

  switch (cmd)
  {
  case MSG_MOBACT:

    // if fighting - spec different attacks
    if (mob->specials.fighting)
    {
      // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
      // Each Case statement that has an action needs to break out at the end.

      switch (number(0, 3))
      {
      case 0: /* Clawed hand - physical baseline */
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n growls at $N in a low pitched rumble.", 0, mob, 0, vict, TO_NOTVICT);
          act("$n growls at you in a low pitched rumble.", 0, mob, 0, vict, TO_VICT);

          act("$n rips into $N with a clawed hand formed of void.", FALSE, mob, NULL, vict, TO_NOTVICT);
          act("$n rips into you with a clawed hand formed of void.", FALSE, mob, NULL, vict, TO_VICT);

          damage(mob, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;

      case 1: /* Void Lash */
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n's body distorts as tendrils of darkness snap outward.", FALSE, mob, NULL, vict, TO_NOTVICT);
          act("Tendrils of darkness snap from $n and coil around you!", FALSE, mob, NULL, vict, TO_VICT);

          damage(mob, vict, number(450, 650), TYPE_UNDEFINED, DAM_MAGICAL);
        }
        break;

      case 2: /* Entropic Drain */
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n presses a hollow palm against $N, draining all warmth.", FALSE, mob, NULL, vict, TO_NOTVICT);
          act("$n presses a hollow palm against you, draining all warmth!", FALSE, mob, NULL, vict, TO_VICT);

          damage(mob, vict, number(350, 500), TYPE_UNDEFINED, DAM_MAGICAL);

          /* Resource drain */
          GET_MANA(vict) = MAX(0, GET_MANA(vict) - number(50, 120));
          GET_MOVE(vict) = MAX(0, GET_MOVE(vict) - number(50, 120));

          /* Small self-heal */
          GET_HIT(mob) = MIN(GET_MAX_HIT(mob), GET_HIT(mob) + number(200, 300));
        }
        break;

      case 3: /* Collapse into Nothing */
        vict = get_random_victim_fighting(mob);
        if (vict)
        {
          act("$n collapses inward, then explodes in a silent wave of nothingness!", FALSE, mob, NULL, vict, TO_NOTVICT);
          act("$n collapses inward, then explodes in a silent wave of nothingness!", FALSE, mob, NULL, vict, TO_VICT);

          damage(mob, vict, number(700, 900), TYPE_UNDEFINED, DAM_MAGICAL);

          /* Short stun / lag */
          WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        }
        break;

      default:
        break;
      }
    }

    // can add an else branch here if you want them to act but not in combat.

    break;
  }

  return FALSE;
}

// Assign Spec for the zone. Sets all other specs.
// First Param - Object, Room or Mob Number.  Define it up above.
// Second Param - the name of the function that is for the mob Usually a shorthand for your zone.
void assign_darkspire(void)
{
  /*Objects */
  // assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);

  /*Rooms */
  // assign_room(LIGHTWALL,             	cl_LightWallLink);

  /*Mobs */

  assign_mob(GOLD_MOB, darkspire_goldmob);
  assign_mob(TAMUR_WERE_BESAT, darkspire_tamur);
  assign_mob(THEURGI_LEADER, darkspire_theurgi);
  assign_mob(DEAD_EYE_VAMPIRE, darkspire_deadeye_vampire);
  assign_mob(HALF_CHANGED_WEREWOLF, darkspire_werewolf);
  assign_mob(TROMOD_GARGOYLE, darkspire_tromod);
  assign_mob(XALGATH_DEMON, darkspire_xalgath);
  assign_mob(STEFAN_ALCHEMIST, darkspire_stefan);
  assign_mob(VOID_ELEMENTAL, darkspire_void_elemental);
}