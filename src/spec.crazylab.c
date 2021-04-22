/*spec.CrazyLab.c - Specs for Crazy Lab by Fisher

     Written by Fisher for RoninMUD
     Last Modification Date: 1/30/2021

     Basic Specs for the mobs and rooms in the zone.
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
#include "regex.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"
#include "cmd.h"
#include "enchant.h"
#include "spec_assign.h"
#include "act.h"
#include "mob.spells.h"
#include "aff_ench.h"

/*Rooms */
#define TEMPLE 3001
#define SWPEDESTAL 14436
#define SEPEDESTAL 14437
#define NWPEDESTAL 14433
#define NEPEDESTAL 14434
#define LIGHTWALL 14438
#define BEYONDLIGHTWALL 14439

/*Objects */
#define MARKER 14499
#define MINER_HAMMER 14497

/*Mobs */
//Lab
#define TEA_BAG 14405
#define PINK_SHEEP 14406
#define ANGRY_BELLS 14407
#define EXPLODES_ZOMBIE 14408
#define BIG_LAD 14409
#define BIG_LARGE_HUGE 14411
#define NEOLATHOTEP 14412
#define BROCOTEER_MAN 14413
#define ROSE_BUSH 14414
#define BUNCH_CARROTS 14415
#define CAPTAIN_CABBAGE 14417
#define WOODEN_RAFT 14418
#define CAT_RABBIT 14419
#define PAINT_SHAKER 14420
#define SHERBET_MONSTER 14421
#define GREEN_PEPPER 14422
#define COCKNEY_SPARROW 14423
#define BUFFALO 14424
#define BALL_PEAN_HAMMER 14425
#define HAROLD_SPIDER 14426
#define OGGLEBLOP 14427
#define SCIENTIST_BOB 14428

//Church

//Mountain
#define ORE_SLIME 14447
#define METAL_SLIME 14448
#define GOLD_SLIME 14449
#define GRASS_SLIME 14450
#define PLAIN_SLIME 14451
#define MUD_SLIME 14452
#define SILVER_SLIME 14453
#define DIAMOND_SLIME 14454
#define TED_MINER 14455
#define GOLD_POT 14465
#define XP_ESSENCE 14466
#define SLOT_MACHINE 14467
#define HOT_TUB 14468
#define MARSHMALLOW 14469

//Sherbert Kingdom
#define SHERBERT_QUEEN 14458

/*Miscellaneous strings */
#define ENCH_SHER1 "Sherbert Blessing of Strength"	//Damage
#define ENCH_SHER2 "Sherbert Blessing of Agility"	//Hitroll
#define ENCH_SHER3 "Sherbert Blessing of Constitution"	//HP Regen
#define ENCH_SHER4 "Sherbert Blessing of Wisdom"	//Mana Regen
/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */

/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

bool chapelRoomsOccupied()
{

 	//check all 4 rooms.  All 4 need a person in it to activate.
  /*
  SWPEDESTAL		 14436
  SEPEDESTAL		 14437
  NWPEDESTAL		 14433
  NEPEDESTAL		 14434
  */
  if (count_mortals_real_room(real_room(SWPEDESTAL)) < 1 ||
    count_mortals_real_room(real_room(SEPEDESTAL)) < 1 ||
    count_mortals_real_room(real_room(NWPEDESTAL)) < 1 ||
    count_mortals_real_room(real_room(NEPEDESTAL)) < 1
 )
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}
int cl_LightWallLink(int room, CHAR *ch, int cmd, char *arg)
{
  bool chapelRoomsOccupied = FALSE;

 	//When the zone is reset, reset the links.  Wall of light reforms.
  if (cmd == MSG_ZONE_RESET)
  {

    if (world[real_room(LIGHTWALL)].dir_option[EAST]->to_room_r != -1)
    {
      world[real_room(LIGHTWALL)].dir_option[EAST]->to_room_r = -1;
      world[real_room(BEYONDLIGHTWALL)].dir_option[WEST]->to_room_r = -1;
      send_to_room("The wall of light reforms and prevents all access.\n\r", real_room(LIGHTWALL));
      send_to_room("The wall of light reforms and prevents all access.\n\r", real_room(BEYONDLIGHTWALL));
    }
  }

  if (chapelRoomsOccupied)
  {
   	//do_room_link(ROOM_FROM, ROOM_TO)\; (again, i don't remember the function call for this)

    send_to_room("The wall of light dissipates and the path forward is clear.\n\r", real_room(LIGHTWALL));
    send_to_room("The wall of light dissipates and the path forward is clear.\n\r", real_room(BEYONDLIGHTWALL));
    world[real_room(LIGHTWALL)].dir_option[EAST]->to_room_r = real_room(BEYONDLIGHTWALL);
    world[real_room(BEYONDLIGHTWALL)].dir_option[WEST]->to_room_r = real_room(LIGHTWALL);
  }

  return FALSE;
}

/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */

/*Block moving forward until death
  40% chance of spec.
   Scald Random Player for 600  (45% chance)
   Scald Room for 900 (30% Chance) */

int cl_teabag(CHAR *bag, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
  bool bReturn = FALSE;

  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(bag, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(bag, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && bag->specials.fighting && chance(40))
  {
   	//45% chance to hit random player.
    if (chance(65))
    {
      vict = get_random_victim_fighting(bag);
      if (vict)
      {
        do_say(bag, "Have a cup of tea.  It will calm you down.", CMD_SAY);

        act("$n fills a small cup full of tea and throws it at $N.", 0, bag, 0, vict, TO_NOTVICT);
        act("$n fills a small cup full of tea and throws it at you.", 0, bag, 0, vict, TO_VICT);
        sprintf(buf, "%s is stuck by %s!", GET_NAME(vict), "a cup of tea.");
        act(buf, FALSE, bag, NULL, vict, TO_NOTVICT);
        sprintf(buf, "You are stuck by %s!", "a cup of tea.");
        act(buf, FALSE, bag, NULL, vict, TO_VICT);
        damage(bag, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
   	//30% chance to attack whole room.
    else if (chance(55))
    {
      act("$n splashes back and forth, sending scalding water throughout the room.", 0, bag, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(bag)].people; vict; vict = next_vict)
      {
       	//all chars for 450 through sanc
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        act("$n splashes scalding water onto you and you are burned!", 0, bag, 0, vict, TO_VICT);
        damage(bag, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
  }
  else
  {
    if (cmd == MSG_MOBACT && chance(15))
    {
      do_say(bag, "Let me finish my tea before you leave.", CMD_SAY);
    }

    if (ch && cmd == CMD_WEST)
    {
      do_say(bag, "Sorry, you cant go that way until I finish my tea.", CMD_SAY);
      bReturn = TRUE;
    }
    return bReturn;
  }

  return FALSE;
}

/*Pink Sheep Spec

45% chance for random affect
  Group attack

  Sheep wont allow you to go south unless you pet it or its dead.
*/
int cl_pinksheep(CHAR *pinksheep, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
  bool bReturn = FALSE;

  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(pinksheep, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(pinksheep, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && pinksheep->specials.fighting && chance(45))
  {
   	//65% chance to hit all player.
    if (chance(65))
    {
      do_say(pinksheep, "Bah Bah Pink Sheep, have you any wool?  I do!", CMD_SAY);
      act("$n rolls around the room as pink wool impacts your face.", 0, pinksheep, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(pinksheep)].people; vict; vict = next_vict)
      {
       	//all chars for 350 through sanc
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        act("$n splashes scalding water onto you and you are burned!", 0, pinksheep, 0, vict, TO_VICT);
        damage(pinksheep, vict, 700, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
  }
  else
  {
    if (cmd == MSG_MOBACT && chance(15))
    {
      do_say(pinksheep, "BAhhhhh BahAhhhhh bah.", CMD_SAY);
    }

   	//If you pet the sheep, it just despawns until the next tick.

    if (cmd == CMD_UNKNOWN)
    {
      arg = one_argument(arg, buf);

      if (!strcmp(buf, "pet"))
      {
        arg = one_argument(arg, buf);

        if (strcmp(buf, "sheep"))
        {

         	//Despawn the sheep
         	// Sheep wanders vanishes into the field of grass.
          act("$n hops away and disapears into the grass.", 0, pinksheep, 0, 0, TO_ROOM);
          char_from_room(pinksheep);

          switch (number(0, 22))
          {
            case 17:
             	//Load Token Object on ground.
             	//Load Object MARKER (defined above)

              break;
            default:
              break;
          }

          return TRUE;
        }
       	//Didnt try to pet the sheep.
        else
        {

          send_to_char("What are you trying to pet?\n\r", ch);
          bReturn = TRUE;
        }
      }
    }

    if (ch && cmd == CMD_SOUTH)
    {
      do_say(pinksheep, "Bahhhhhhh.", CMD_SAY);
      bReturn = TRUE;
    }
    return bReturn;
  }

  return FALSE;
}

/*
  Stuns the room for 2 rounds.
  */

int cl_angrybells(CHAR *angrybells, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;

  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(angrybells, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(angrybells, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && angrybells->specials.fighting && chance(30))
  {
   	//65% stun room for 2 rounds.
    if (chance(65))
    {
      do_say(angrybells, "Clang clang clang goes the trolley. Ring ring ring goes the BELLLS!", CMD_SAY);

      act("$n rings very loudly and you are defeaned by the noise.", 0, angrybells, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(angrybells)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        damage(angrybells, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
        WAIT_STATE(vict, PULSE_VIOLENCE *4);
      }
    }
  }
  return FALSE;
}

/*
  Combat Spec - Small explosions, big one at the end.
  Explodes at the end of the fight for 2000 damage.
  */

int cl_explodeszombie(CHAR *explodeszombie, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(explodeszombie, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && explodeszombie->specials.fighting && chance(10))
  {
   	//30% to explode against a random target.
    if (chance(30))
    {
      vict = get_random_victim_fighting(explodeszombie);
      if (vict)
      {
        do_say(explodeszombie, "Here, have an explosion. I love them.  I hope you do too.", CMD_SAY);

        act("$n glows a little and a small explsion forms around $N.", 0, explodeszombie, 0, vict, TO_NOTVICT);
        act("$n glows a little and an explosion forms around you.", 0, explodeszombie, 0, vict, TO_VICT);
        sprintf(buf, "%s is stuck by %s!", GET_NAME(vict), "a small explosion.");
        act(buf, FALSE, explodeszombie, NULL, vict, TO_NOTVICT);
        sprintf(buf, "You are stuck by %s!", "a small explosion.");
        act(buf, FALSE, explodeszombie, NULL, vict, TO_VICT);
        damage(explodeszombie, vict, 800, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
  }

 	//When the Mob Dies - Explodes.
 	//Normal Explosion does 2500 Damage.
 	//10% chance to destroy the corpse and damage goes up to 3300
  if (cmd == MSG_DIE)
  {
   	//Also Destroy Corpse.
    if (chance(10))
    {
      do_say(explodeszombie, "This one is gonna be big.", CMD_SAY);

      act("As the zombie dies, the corpse erupts and a wall of heat and pressure hits you hard.", 0, explodeszombie, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(explodeszombie)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        damage(explodeszombie, vict, 3300, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
    else
    {
      do_say(explodeszombie, "I hope you like explosions. I sure do!", CMD_SAY);

      act("As the zombie dies, it explodes and a wall of heat and pressure hits you hard.", 0, explodeszombie, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(explodeszombie)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        damage(explodeszombie, vict, 2500, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
  }

  return FALSE;
}

/*
Prevents Movement forward
Single Target high Damage smash with a hammer.
*/

int cl_biglad(CHAR *biglad, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  CHAR * vict;
  bool bReturn = FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(biglad, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && biglad->specials.fighting && chance(40))
  {
   	//45% chance to hit random player.
    if (chance(45))
    {
      vict = get_random_victim_fighting(biglad);
      if (vict)
      {
        switch (number(0, 3))
        {
          case 0:
            do_say(biglad, "Looks like we nailed that one on the head.", CMD_SAY);
            break;
          case 1:
            do_say(biglad, "Stop!  Hammer Time!", CMD_SAY);
            break;
          case 2:
            do_say(biglad, "Hammer.  This is not a drill.", CMD_SAY);
            break;
          case 3:
            do_say(biglad, "I was always wondering why hammers fall down.  Then it hit you.", CMD_SAY);
            break;
          default:
            break;
        }

        act("$n swings a giant hammer and smashes into $N.", 0, biglad, 0, vict, TO_NOTVICT);
        act("$n swings a giant hammer and smashes into you.", 0, biglad, 0, vict, TO_VICT);
        sprintf(buf, "%s is stuck on the head by %s!", GET_NAME(vict), "a very large hammer.");
        act(buf, FALSE, biglad, NULL, vict, TO_NOTVICT);
        sprintf(buf, "You are stuck on the head  by %s!", "a very large hammer.");
        act(buf, FALSE, biglad, NULL, vict, TO_VICT);
        damage(biglad, vict, 1750, TYPE_UNDEFINED, DAM_PHYSICAL);
        WAIT_STATE(vict, PULSE_VIOLENCE);
      }
    }
  }

  if (cmd == MSG_MOBACT && chance(15))
  {
    switch (number(0, 2))
    {
      case 0:
        do_say(biglad, "Time to swing my hammer!", CMD_SAY);
        break;
      case 1:
        do_say(biglad, "Watch out, dont want you getting smacked now.", CMD_SAY);
        break;
      case 2:
        do_say(biglad, "I love my hammer and it loves me!", CMD_SAY);
        break;
      default:
        break;
    }
  }
  if (ch && cmd == CMD_UP)
  {
    do_say(biglad, "My ladder!  Go Away!  Dont make me smash you.", CMD_SAY);
    bReturn = TRUE;
  }
  return bReturn;
  return FALSE;
}

int cl_biglargehuge(CHAR *biglargehuge, CHAR *ch, int cmd, char *arg)
{
  CHAR * vict;
  bool bReturn = FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(biglargehuge, TRUE) < 1) return FALSE;

 	// Look for specific commands and trigger on them.
  switch (cmd)
  {
    case MSG_MOBACT:
     	// Damage Random Victim
      if (biglargehuge->specials.fighting && chance(30))
      {
        vict = get_random_victim_fighting(biglargehuge);
        if (!vict) break;
        do_say(biglargehuge, "Go sit in the corner.", CMD_SAY);
        act("The zombie punches beat $N in the head.", 0, biglargehuge, 0, vict, TO_NOTVICT);
        act("The zombie punches beat you in the head.", 0, biglargehuge, 0, vict, TO_VICT);
        damage(biglargehuge, vict, 735, TYPE_UNDEFINED, DAM_PHYSICAL);
        WAIT_STATE(vict, PULSE_VIOLENCE *2);
      }
      break;
    case CMD_BASH:
    case CMD_PUMMEL:
    case CMD_SMITE:
      /*blocks pummel and smite.*/
      do_say(biglargehuge, "Thats just annoying. Sit Down!", CMD_SAY);
      GET_POS(ch) = POSITION_SITTING;
      bReturn = TRUE;
      break;

     	//Dont chat during the fight or get hit!
    case CMD_GOSSIP:
      do_say(biglargehuge, "Dont talk while we fight.", CMD_SAY);
      bReturn = TRUE;
      break;

    case CMD_LUNGE:
    case CMD_AMBUSH:
    case CMD_BACKSTAB:
      do_say(biglargehuge, "Your pathetic attacks wont work on me.", CMD_SAY);
      GET_POS(ch) = POSITION_SITTING;
      bReturn = TRUE;
      break;

    default:
      break;
  }

  return bReturn;
}
//Random Curses that lower damage or hitroll,remove postiive affects or group slash.
int cl_neolathotep(CHAR *neolathotep, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(neolathotep, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && neolathotep->specials.fighting && chance(60))
  {
   	//35% Lower Damroll - 3 tick duration  - Can stack and resets timer.
    if (chance(30))
    {
      vict = get_random_victim_fighting(neolathotep);
      if (vict)
      {
        do_say(neolathotep, "Owww. You hit too hard.  Lets fix that now.", CMD_SAY);
       	//Lower damroll by 3
      }
    }
   	//Lower Hitroll on random victim - 3 tick duration - Can stack and resets timer.
    else if (chance(30))
    {
      do_say(neolathotep, "Lets see if you can hit my perfect self now.", CMD_SAY);
      vict = get_random_victim_fighting(neolathotep);
      if (vict)
      {
       	//Lower Hitroll by 3
      }
    }
   	//Remove Fury/Haste/Rage/Mystic Swiftness/Rush
   	//Remove all bonus damage buffs -
    else if (chance(20))
    {
      do_say(neolathotep, "If you are soooo powerful, you won't need these now will you.", CMD_SAY);
     	//How to remove these spells?
     	//Target victims that are still buffed
      vict = get_random_victim_fighting(neolathotep);
      if (vict) {}
    }
   	//Claw them.
    else
    {
      do_say(neolathotep, "Claws are sharp. How do they feel?", CMD_SAY);
      act("$n slashes back and forth cutting everyone in the room", 0, neolathotep, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(neolathotep)].people; vict; vict = next_vict)
      {
       	//all chars for 300 through sanc
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        act("$n slashes with its claws and it cuts your skin.", 0, neolathotep, 0, vict, TO_VICT);
        damage(neolathotep, vict, 300, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
  }
  return FALSE;
}

int cl_rosebush(CHAR *rosebush, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(rosebush, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(rosebush, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && rosebush->specials.fighting && chance(38))
  {
   	//65% prick and poison party.
    if (chance(65))
    {
      do_say(rosebush, "Smell the flower, get the thorns!", CMD_SAY);

      act("$n launches poison covered thorns at the party.", 0, rosebush, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(rosebush)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        act("$n launches a poison covered thorn at you and it embeds in your skin,", 0, rosebush, 0, vict, TO_VICT);
        spell_poison(30, rosebush, vict, 0);
        damage(rosebush, vict, 100, TYPE_UNDEFINED, DAM_NO_BLOCK);
      }
    }
  }
  return FALSE;
}
int cl_bunchcarrots(CHAR *bunchcarrots, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  CHAR * vict;
  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(bunchcarrots, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(bunchcarrots, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && bunchcarrots->specials.fighting && chance(42))
  {
   	//65% stun room for 1 rounds.
    if (chance(65))
    {
      vict = get_random_victim_fighting(bunchcarrots);
      if (vict)
      {
        do_say(bunchcarrots, "Im Healthy. Stop throwing me out.", CMD_SAY);
        act("$n removes a carrot and approaches $N.", 0, bunchcarrots, 0, vict, TO_NOTVICT);
        act("$n removes a carrot and approaches you.", 0, bunchcarrots, 0, vict, TO_VICT);
        sprintf(buf, "%s has a carrot shoved down their throat.", GET_NAME(vict));
        act(buf, FALSE, bunchcarrots, NULL, vict, TO_NOTVICT);
        sprintf(buf, "You have a carrot shoved down your throat.!");
        act(buf, FALSE, bunchcarrots, NULL, vict, TO_VICT);

        switch (number(0, 2))
        {
          case 0:
            do_say(bunchcarrots, "Eat your vegetables or no dessert for you.", CMD_SAY);
            break;
          case 1:
            do_say(bunchcarrots, "Carrots improve your poor eyesight.", CMD_SAY);
            break;
          case 2:
            do_say(bunchcarrots, "Ranch makes me taste better.", CMD_SAY);
            break;
          default:
            break;
        }
        WAIT_STATE(vict, PULSE_VIOLENCE *1);
      }
    }
  }
  return FALSE;
}

// Raft really wont die.

//Have to Type Lean Left or Right to Dodge out of the way.  Surive 3 or get hit by 2 to end the fight.
int cl_woodenraft(CHAR *woodenraft, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;

  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(woodenraft, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(woodenraft, TRUE) < 1) return FALSE;

 	//Reset at the begining of each fight/load of the zone.
 	//Need to Dodge 5 Times to win the fight the special way.
  int TotalDodges = 0;
 	//If TotalMisses hits 5 first, kill the mob and drop nothing.
  int TotalMisses = 0;
  if (cmd == MSG_MOBACT && woodenraft->specials.fighting && chance(30))
  {

   	//You either dodge or you miss.

    if (chance(40))
    {
      do_say(woodenraft, "The wave is coming from the right! Lean Away Form It.", CMD_SAY);

     	//Lean Left is the proper command else, fail and damage them.
      if (!strcmp(buf, "lean"))
      {
        arg = one_argument(arg, buf);

        if (strcmp(buf, "left"))
        {

          TotalDodges++;
          return TRUE;
        }
        else
        {
         	//Increment by 1
          TotalMisses++;
          act("You are hit by a giant wave of water.", 0, woodenraft, 0, 0, TO_ROOM);
          for (vict = world[CHAR_REAL_ROOM(woodenraft)].people; vict; vict = next_vict)
          {
            next_vict = vict->next_in_room;
            if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
            damage(woodenraft, vict, 400, TYPE_UNDEFINED, DAM_PHYSICAL);
          }
        }
      }
      else if (chance(40))
      {
        do_say(woodenraft, "The wave is coming from the right! Lean Into it.", CMD_SAY);

       	//Lean Right is the proper command else, fail and damage them.
        if (!strcmp(buf, "lean"))
        {
          arg = one_argument(arg, buf);

          if (strcmp(buf, "right"))
          {

            TotalDodges++;
            return TRUE;
          }
          else
          {
           	//Increment by 1
            TotalMisses++;
            act("You are hit by a giant wave of water.", 0, woodenraft, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(woodenraft)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(woodenraft, vict, 400, TYPE_UNDEFINED, DAM_PHYSICAL);
            }
          }
        }
        else if (chance(40))
        {
          do_say(woodenraft, "The wave is coming from the left! Lean Away Form It.", CMD_SAY);

         	//Lean Right is the proper command else, fail and damage them.
          if (!strcmp(buf, "lean"))
          {
            arg = one_argument(arg, buf);

            if (strcmp(buf, "right"))
            {

              TotalDodges++;
              return TRUE;
            }
            else
            {
             	//Increment by 1
              TotalMisses++;
              act("You are hit by a giant wave of water.", 0, woodenraft, 0, 0, TO_ROOM);
              for (vict = world[CHAR_REAL_ROOM(woodenraft)].people; vict; vict = next_vict)
              {
                next_vict = vict->next_in_room;
                if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
                damage(woodenraft, vict, 400, TYPE_UNDEFINED, DAM_PHYSICAL);
              }
            }
          }
          else if (chance(40))
          {
            do_say(woodenraft, "The wave is coming from the left! Lean Into It.", CMD_SAY);

           	//Lean Left is the proper command else, fail and damage them.
            if (!strcmp(buf, "lean"))
            {
              arg = one_argument(arg, buf);

              if (strcmp(buf, "left"))
              {

                TotalDodges++;
                return TRUE;
              }
              else
              {
               	//Increment by 1
                TotalMisses++;
                act("You are hit by a giant wave of water.", 0, woodenraft, 0, 0, TO_ROOM);
                for (vict = world[CHAR_REAL_ROOM(woodenraft)].people; vict; vict = next_vict)
                {
                  next_vict = vict->next_in_room;
                  if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
                  damage(woodenraft, vict, 400, TYPE_UNDEFINED, DAM_PHYSICAL);
                }
              }
            }

            if (TotalDodges == 5)
            {
             	//If Total Dodges hits 5 first, then you win.
             	//Drop Between 2-3 million gold and 10% chance for a Marker (14499)

            }
            else if (TotalMisses == 5)
            {
             	//raft Crashes  and you get a small amount of gold.

            }
          }
        }
      }
    }
  }
  return FALSE;
}
// Switches target every 3 rounds.  Prevents rescue.  So Whole party must tank.
int cl_catrabbit(CHAR *catrabbit, CHAR *ch, int cmd, char *arg)
{
 	//CHAR * vict;
  int turns = 0;

  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(catrabbit, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(catrabbit, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && catrabbit->specials.fighting)
  {

    turns++;
   	//At 3 turns, chance target and reset counter.
    if (turns == 3)
    {
      do_say(catrabbit, "Lets play with someone else now.", CMD_SAY);
     	//vict = get_random_victim_fighting(catrabbit);
      turns = 0;
    }
  }
 	//Block all rescue attempts
  if (cmd == CMD_RESCUE)
  {
    do_say(catrabbit, "Everyone gets their chance to play with me.  Dont take that away from them.", CMD_SAY);
  }
  return FALSE;
}

//Shakes the party.
int cl_paintshaker(CHAR *paintshaker, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;

  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(paintshaker, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(paintshaker, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && paintshaker->specials.fighting && chance(30))
  {
   	//30% stun room for 2 rounds.
    if (chance(30))
    {

      do_say(paintshaker, "Shake shake shake.  Shake your body. Shake your body.", CMD_SAY);

      act("$n Grabs everyone and shakes them violently.", 0, paintshaker, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(paintshaker)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        damage(paintshaker, vict, 200, TYPE_UNDEFINED, DAM_PHYSICAL);
        WAIT_STATE(vict, PULSE_VIOLENCE *1);
      }
    }
  }
  return FALSE;
}

//Constant Quips for Each Attack.
//Stab random enemy with switchblade
//blinds everyone.
//Flurry of Feathers to attack everyone.
int cl_cockneysparrow(CHAR *cockneysparrow, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;

  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(cockneysparrow, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(cockneysparrow, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && cockneysparrow->specials.fighting && chance(70))
  {
    if (chance(10))
    {
      do_say(cockneysparrow, "'Why you star'in trou-bow", CMD_SAY);
    }
   	//30% stab random victim.
    if (chance(30))
    {

      vict = get_random_victim_fighting(cockneysparrow);
      if (vict)
      {
        do_say(cockneysparrow, "'ave a noice nife you blit'er", CMD_SAY);

        act("$n pulls a knife and stabs $N in the eye.", 0, cockneysparrow, 0, vict, TO_NOTVICT);
        act("$n pulls a knife and stabs you in the eye.", 0, cockneysparrow, 0, vict, TO_VICT);
        sprintf(buf, "%s has a knife rammed into their eye.", GET_NAME(vict));
        act(buf, FALSE, cockneysparrow, NULL, vict, TO_NOTVICT);
        sprintf(buf, "You has a knife rammed into your eye!");
        act(buf, FALSE, cockneysparrow, NULL, vict, TO_VICT);
      }
    }
    else if (chance(30))
    {
      do_say(cockneysparrow, "'ope you can see in the dark.", CMD_SAY);

     	//How to blind everyone?
    }
    else
    {
      do_say(cockneysparrow, "fink you can find your mo-thah.", CMD_SAY);
      for (vict = world[CHAR_REAL_ROOM(cockneysparrow)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        damage(cockneysparrow, vict, 300, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
  }
  return FALSE;
}
//6 random attacks
/*
1. Fury
2. Haste.
3. Mira Self
4. Group Attack - 800
5. Single Attack Damage and stun
6. Lethal Fire room.

*/
int cl_buffalo(CHAR *buffalo, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(buffalo, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(buffalo, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && buffalo->specials.fighting && chance(47))
  {
    switch (number(0, 5))
    {
     	//cast Fury on Self
      case 0:
        do_say(buffalo, "Buffalo", CMD_SAY);
        if (!affected_by_spell(buffalo, SPELL_FURY))
        {
          spell_fury(GET_LEVEL(buffalo), buffalo, buffalo, 0);
        }
        break;
      case 1:
       	//cast haste on self.
        do_say(buffalo, "Buffalo", CMD_SAY);
        if (!affected_by_spell(buffalo, SPELL_HASTE))
        {
          spell_haste(GET_LEVEL(buffalo), buffalo, buffalo, 0);
        }
        break;
      case 2:
       	//Heals 1200 HP
        do_say(buffalo, "Buffalo", CMD_SAY);
        GET_HIT(buffalo) = GET_HIT(buffalo) + 1200;
        break;
       	// Group Attack
      case 3:
        do_say(buffalo, "Buffalo", CMD_SAY);
        for (vict = world[CHAR_REAL_ROOM(buffalo)].people; vict; vict = next_vict)
        {
          next_vict = vict->next_in_room;
          if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
          damage(buffalo, vict, 800, TYPE_UNDEFINED, DAM_PHYSICAL);
        }
        break;
       	//Single Attack and Stun 1 Round.
      case 4:
        do_say(buffalo, "Buffalo", CMD_SAY);
        vict = get_random_victim_fighting(buffalo);
        if (vict)
        {

          act("$n stares wided eyed at $N.", 0, buffalo, 0, vict, TO_NOTVICT);
          act("$n stares wided eyed at you.", 0, buffalo, 0, vict, TO_VICT);
          sprintf(buf, "%s has a carrot shoved down their throat.", GET_NAME(vict));
          act(buf, FALSE, buffalo, NULL, vict, TO_NOTVICT);
          sprintf(buf, "You have a carrot shoved down your throat.!");
          act(buf, FALSE, buffalo, NULL, vict, TO_VICT);

          WAIT_STATE(vict, PULSE_VIOLENCE *1);
        }

        break;
       	//Burn the Room.
      case 5:
        do_say(buffalo, "Buffalo", CMD_SAY);
        for (CHAR *vict = world[CHAR_REAL_ROOM(buffalo)].people, *next_vict; vict; vict = next_vict)
        {
          next_vict = vict->next_in_room;

          /*Don't burn immortals or mobs that aren't attackers. */
          if (IS_IMMORTAL(vict) || (IS_NPC(vict) && (GET_OPPONENT(vict) != buffalo))) continue;

          act("$n is burned by the odd pink flames.!", TRUE, vict, 0, 0, TO_ROOM);
          printf_to_char(vict, "You are burned by the odd pink flames.\n\r");

          damage(buffalo, vict, (GET_LEVEL(buffalo) / 2) + number(180, 230), TYPE_UNDEFINED, DAM_MAGICAL);
        }

        return FALSE;
        break;
      default:
        break;
    }
  }
  return FALSE;
}
//Lots of group damage.
int cl_ballpeanhammer(CHAR *ballpeanhammer, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(ballpeanhammer, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(ballpeanhammer, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && ballpeanhammer->specials.fighting && chance(60))
  {

    do_say(ballpeanhammer, "Getting hit in the face with a hammer must really suck.", CMD_SAY);

    act("$n Swings around the room, smacking everything it touches.", 0, ballpeanhammer, 0, 0, TO_ROOM);
    for (vict = world[CHAR_REAL_ROOM(ballpeanhammer)].people; vict; vict = next_vict)
    {
      next_vict = vict->next_in_room;
      if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
      damage(ballpeanhammer, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
    }
  }

  return FALSE;
}
//If you dont attack for 30 seconds - Make Harold Fully Heal Party and Leave.
int cl_haroldspider(CHAR *haroldspider, CHAR *ch, int cmd, char *arg)
{

  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(haroldspider, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(haroldspider, TRUE) < 1) return FALSE;

 	//Trigger when party first enters the room and only then?

 	//Only on initial party entering the room.
  do_say(haroldspider, "I really dont want to fight.  Just give me a moment here.", CMD_SAY);

 	//Wait 30 seconds and then despawn

 	//Ties Victims up in Web, can be freed with fire spells?
 	//How can we make it so it webs characters and stuns them for 3 rounds or burns up with like lethal fire being cast?
  if (cmd == MSG_MOBACT && haroldspider->specials.fighting && chance(55))
  {

    do_say(haroldspider, "I got webs for days.  Lets keep you tied up.", CMD_SAY);
  }
  return FALSE;
}
int cl_oggleblop(CHAR *oggleblop, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  int throw_room = 14426;

  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(oggleblop, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(oggleblop, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && oggleblop->specials.fighting && chance(30))
  {

   	//Throw units from the room.
    if (chance(40))
    {
      if (chance(25))
      {
        do_say(oggleblop, "Leave the scientist alone. His creations are wonderful.", CMD_SAY);
      }

      vict = get_random_victim_fighting(oggleblop);
      if (vict)
      {
        act("$n grbas $N and throws them from the room.", 0, oggleblop, 0, vict, TO_NOTVICT);
        act("$n grabs you and throws you from the room.", 0, oggleblop, 0, vict, TO_VICT);
        char_from_room(vict);
        char_to_room(vict, throw_room);
      }
    }
   	//Group Attack
    else if (chance(40))
    {
      if (chance(25))
      {
        do_say(oggleblop, "Grrr.....you all need to go away.", CMD_SAY);
      }

      act("$n Swings angrily around the room. Large fists pummel everything in sight.", 0, oggleblop, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(oggleblop)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        damage(oggleblop, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
   	//Punch 2 Random Vicitms.
    else if (chance(40))
    {
      if (chance(25))
      {
        do_say(oggleblop, "Someone needs to get punched.", CMD_SAY);
      }

      vict = get_random_victim_fighting(oggleblop);
      if (vict)
      {
        act("$n punches $N with a huge closed fist.", 0, oggleblop, 0, vict, TO_NOTVICT);
        act("$n punches you with a huge closed fist..", 0, oggleblop, 0, vict, TO_VICT);
        damage(oggleblop, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
        WAIT_STATE(vict, PULSE_VIOLENCE *1);
      }

      vict = get_random_victim_fighting(oggleblop);
      if (vict)
      {
        act("$n punches $N with a huge closed fist.", 0, oggleblop, 0, vict, TO_NOTVICT);
        act("$n punches you with a huge closed fist..", 0, oggleblop, 0, vict, TO_VICT);
        damage(oggleblop, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
        WAIT_STATE(vict, PULSE_VIOLENCE *1);
      }
    }
   	//bite the tank
    else if (chance(50))
    {
      if (chance(35))
      {
        do_say(oggleblop, "Nom.  You are tasty.", CMD_SAY);
      }

     	//How do we get the tank?
      vict = get_random_victim_fighting(oggleblop);
      if (vict)
      {
        act("$n takes a huge bite out of $N and licks its lips.", 0, oggleblop, 0, vict, TO_NOTVICT);
        act("$n takes a huge bite out of you and licks its lips.", 0, oggleblop, 0, vict, TO_VICT);
        damage(oggleblop, vict, 700, TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
  }
  return FALSE;
}
/*
Throws Random Potions

Blue Potion - Pillar of Fire - 50% of current hp removed.
Red Potion - Confusion - Victim targets another party member for 1 hit and comes back to reality.
Green Potion - Remove All Buffs.
Pink Potion - Lightning Against the Room
Brown Potion - Stun Party for 2 Rounds.  Remove 20% of HP.
Purple Potion - Randomly shuffles HP values.

*/
int cl_scientistbob(CHAR *scientistbob, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
  /*Don't waste any more CPU time if no one is in the zone. */
  if (count_mortals_zone(scientistbob, TRUE) < 1) return FALSE;

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(scientistbob, TRUE) < 1) return FALSE;

  if (cmd == MSG_MOBACT && scientistbob->specials.fighting && chance(60))
  {
    do_say(scientistbob, "I wonder what potion to try out now.", CMD_SAY);
    switch (number(0, 5))
    {
     	//Blue Potion
      case 0:
        if (chance(65))
        {
          do_say(scientistbob, "Thank you for illuminating the room.", CMD_SAY);
        }
        vict = get_random_victim_fighting(scientistbob);
        act("$n grabs a blue potion from his potion rack and throws it at $N.", 0, scientistbob, 0, vict, TO_NOTVICT);
        act("$n grabs a blue potion from his potion rack and throws it at you.", 0, scientistbob, 0, vict, TO_VICT);
        sprintf(buf, "%s is engulfed by %s!", GET_NAME(vict), "a pillar of fire.");
        act(buf, FALSE, scientistbob, NULL, vict, TO_NOTVICT);
        sprintf(buf, "You are engulfed by a pillar of fire.");
        act(buf, FALSE, scientistbob, NULL, vict, TO_VICT);
        GET_HIT(vict) = GET_HIT(vict) / 2;
        break;
       	//Red Potion
      case 1:
        if (chance(65))
        {
          do_say(scientistbob, "Which one is clearer. Me or them?", CMD_SAY);
        }
        vict = get_random_victim_fighting(scientistbob);
        act("$n grabs a red potion from his potion rack and throws it at $N.", 0, scientistbob, 0, vict, TO_NOTVICT);
        act("$n grabs a red potion from his potion rack and throws it at you.", 0, scientistbob, 0, vict, TO_VICT);
        sprintf(buf, "%s is covered by %s!", GET_NAME(vict), "a fine red mist.");
        act(buf, FALSE, scientistbob, NULL, vict, TO_NOTVICT);
        sprintf(buf, "You are covered by a fine red mist.");
        act(buf, FALSE, scientistbob, NULL, vict, TO_VICT);

       	//Pick a random party member and have vict attack them.
       	//vict2 = get_random_victim_fighting(scientistbob);

        break;
       	//Green Potion
      case 2:
        if (chance(65))
        {
          do_say(scientistbob, "Ooooo, you are naked.  Lets all laugh at you.", CMD_SAY);
        }
        vict = get_random_victim_fighting(scientistbob);
        act("$n grabs a red potion from his potion rack and throws it at $N.", 0, scientistbob, 0, vict, TO_NOTVICT);
        act("$n grabs a red potion from his potion rack and throws it at you.", 0, scientistbob, 0, vict, TO_VICT);
        sprintf(buf, "%s is covered by %s!", GET_NAME(vict), "a fine red mist.");
        act(buf, FALSE, scientistbob, NULL, vict, TO_NOTVICT);
        sprintf(buf, "You are covered by a fine red mist.");
        act(buf, FALSE, scientistbob, NULL, vict, TO_VICT);

       	//Remove All Buffs from Vict

        break;
       	//Pink Potion
      case 3:
        if (chance(65))
        {
          do_say(scientistbob, "Everyone says pink is a friendly color. I think its deadly.", CMD_SAY);
        }
        act("$n grabs a red potion from his potion rack and throws it into the middle of the room.", 0, scientistbob, 0, 0, TO_ROOM);
        for (vict = world[CHAR_REAL_ROOM(scientistbob)].people; vict; vict = next_vict)
        {
          next_vict = vict->next_in_room;
          if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
          damage(scientistbob, vict, (GET_LEVEL(scientistbob) / 2) + number(780, 1230), TYPE_UNDEFINED, DAM_MAGICAL);
        }
        break;
       	//Brown Potion
      case 4:
        if (chance(65))
        {
          do_say(scientistbob, "Brown is such a bleh color.", CMD_SAY);
        }
        act("$n grabs a brown potion from his potion rack and throws it into the middle of the room.", 0, scientistbob, 0, 0, TO_ROOM);
        for (vict = world[CHAR_REAL_ROOM(scientistbob)].people; vict; vict = next_vict)
        {
          next_vict = vict->next_in_room;
          if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
          WAIT_STATE(vict, PULSE_VIOLENCE *2);
          GET_HIT(vict) = GET_HIT(vict) *0.8;	//Remove 20% of Current HP
        }

        break;
       	//Purple Potion
      case 5:
        if (chance(65))
        {
          do_say(scientistbob, "Tell Me if you feel any tingling.", CMD_SAY);
        }
        act("$n grabs a purple potion from his potion rack and throws it into the middle of the room.", 0, scientistbob, 0, 0, TO_ROOM);
        for (vict = world[CHAR_REAL_ROOM(scientistbob)].people; vict; vict = next_vict)
        {
          next_vict = vict->next_in_room;
          if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;

          switch (number(0, 3))
          {

            case 0:
              GET_HIT(vict) = GET_HIT(vict) + 600;
              break;
            case 1:
              GET_HIT(vict) = GET_HIT(vict) + 1200;
              break;
            case 2:
              GET_HIT(vict) = GET_HIT(vict) - 800;
              break;
            case 3:
              GET_HIT(vict) = GET_HIT(vict) - 500;
              break;
            default:
              break;
          }
        }
        break;
      default:
        break;
    }
  }
  return FALSE;
}

//Team of 3 Fight
//I want captain cabbage to fight the tank
//Brocoteer man to target the leader
//GreenPepper targets 1 random party member.

//A character can only rescue if not currently fighting.
//No real specs, just a chaotic fight.

int cl_brocoteerman(CHAR *angrybells, CHAR *ch, int cmd, char *arg)
{

 	//Brocoteer man to target the leader

  if (cmd == MSG_MOBACT && angrybells->specials.fighting && chance(30)) {}
  return FALSE;
}
int cl_captaincabbage(CHAR *angrybells, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
 	//I want captain cabbage to fight the tank
  if (cmd == MSG_MOBACT && angrybells->specials.fighting && chance(30))
  {
   	//65% stun room for 2 rounds.
    if (chance(65))
    {

      do_say(angrybells, "Clang clang clang goes the trolley. Ring ring ring goes the BELLLS!", CMD_SAY);

      act("$n rings very loudly and you are defeaned by the noise.", 0, angrybells, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(angrybells)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        damage(angrybells, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
        WAIT_STATE(vict, PULSE_VIOLENCE *4);
      }
    }
  }
  return FALSE;
}
//GreenPepper targets 1 random party member.
int cl_greenpepper(CHAR *angrybells, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  if (cmd == MSG_MOBACT && angrybells->specials.fighting && chance(30))
  {
   	//65% stun room for 2 rounds.
    if (chance(65))
    {

      do_say(angrybells, "Clang clang clang goes the trolley. Ring ring ring goes the BELLLS!", CMD_SAY);

      act("$n rings very loudly and you are defeaned by the noise.", 0, angrybells, 0, 0, TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(angrybells)].people; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;
        if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
        damage(angrybells, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
        WAIT_STATE(vict, PULSE_VIOLENCE *4);
      }
    }
  }
  return FALSE;
}

/***********************************SHERBERT OTHER FUNCTIONS   ***********************************/

int ench_sherbet_boon(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg)
{
  if (cmd == CMD_RENT || cmd == CMD_QUIT)
  {
    send_to_char("You cannot take the Sherbert Kingdom's power with you. \r\n", ench_ch);
    enchantment_remove(ench_ch, ench, 0);
    return FALSE;
  }

  if (cmd == MSG_DIE || cmd == MSG_AUTORENT || cmd == MSG_STONE || cmd == MSG_DEAD)
  {
    enchantment_remove(ench_ch, ench, 0);
    return FALSE;
  }

  return FALSE;
}

/*
//Teleport Everyone in the Shertbert Kingdom back to the temple.
void purge_sherbert_kingdom()
{
	int SHERBERT_KINGDOM_START = 14470;
	int SHERBERT_KINGDOM_END = 14480;

	//Once this is done, kick everyone out of the sherbert kingdom.
	//For all Chars in rooms 14470 - 14480 - Teleport to Temple.

	for (int i = SHERBERT_KINGDOM_START, i> SHERBERT_KINGDOM_END; i++)
	{
		move_chars_to_room_ex(i, TEMPLE, true);
	}
}
*/

// Increase HP regen by 30 and Mana Regen by 15
// Increase HR and DR by 5.
void cl_sherbertboon(CHAR *vict)
{
 	//  ench_apply(ch, overwrite, name, type, duration, interval, modifier, location, bitvector, bitvector2, func);

 	//Set variables for enchantment increases

  int BonusDamage = 5;
  int BonusHitroll = 5;
  int BonusHPRegen = 30;
  int BonusManaRegen = 15;

  ench_apply(vict, TRUE, ENCH_SHER1, 0, 30, 0, BonusDamage, APPLY_DAMROLL, 0, 0, ench_sherbet_boon);
  ench_apply(vict, TRUE, ENCH_SHER2, 0, 30, 0, BonusHitroll, APPLY_HITROLL, 0, 0, ench_sherbet_boon);
  ench_apply(vict, TRUE, ENCH_SHER3, 0, 30, 0, BonusHPRegen, APPLY_HP_REGEN, 0, 0, ench_sherbet_boon);
  ench_apply(vict, TRUE, ENCH_SHER4, 0, 30, 0, BonusManaRegen, APPLY_MANA_REGEN, 0, 0, ench_sherbet_boon);

  send_to_char("The power of pastel rushes over you.\r\n", vict);
}

//NPC that you cant attack.
//Will give a special blessing to all party members and then kick them out.
//After entering the room, count to 60 seconds. Then buff and teleport all players in the sherbert kingdom roosm to the temple.
int cl_queensherbert(CHAR *queensherbert, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
 	//Upon entering, start the timer.

 	//Prevent her from being attacked.

  if (cmd == MSG_VIOLENCE)
  {

    do_say(queensherbert, "I am busy right now. Leave me in peace.", CMD_SAY);

    return FALSE;
  }

  if (cmd == MSG_ENTER)
  {

   	//wait 60 Seconds.  then buff all people in the room.
   	//How do we wait ?

    for (vict = world[CHAR_REAL_ROOM(queensherbert)].people; vict; vict = next_vict)
    {
      next_vict = vict->next_in_room;

      if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
     	//If they have any of the enchantments, skip them.
      if (enchanted_by(vict, ENCH_SHER1) || enchanted_by(vict, ENCH_SHER2) || enchanted_by(vict, ENCH_SHER3) || enchanted_by(vict, ENCH_SHER4)) continue;

      cl_sherbertboon(vict);
    }

   	//purge_sherbert_kingdom();
  }

  return FALSE;
}

void assign_crazylab(void)
{

  /*Objects */
 	//assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);
  /*Rooms */
  assign_room(LIGHTWALL, cl_LightWallLink);
  /*Mobs */
 	//Lab
  assign_mob(TEA_BAG, cl_teabag);
  assign_mob(PINK_SHEEP, cl_pinksheep);
 	//assign_mob(ANGRY_BELLS, cl_angrybells);
 	//assign_mob(EXPLODES_ZOMBIE, cl_explodeszombie);
 	//assign_mob(BIG_LAD, cl_biglad);
 	//assign_mob(BIG_LARGE_HUGE, cl_biglargehuge);
 	//assign_mob(NEOLATHOTEP, cl_neolathotep);
 	//assign_mob(BROCOTEER_MAN, cl_brocoteerman);
 	//assign_mob(ROSE_BUSH, cl_rosebush);
 	//assign_mob(BUNCH_CARROTS, cl_bunchcarrots);
 	//assign_mob(CAPTAIN_CABBAGE, cl_captaincabbage);
 	//assign_mob(WOODEN_RAFT, cl_woodenraft);
 	//assign_mob(CAT_RABBIT, cl_catrabbit);
 	//assign_mob(PAINT_SHAKER, cl_paintshaker);
 	//assign_mob(GREEN_PEPPER, cl_greenpepper);
 	//assign_mob(COCKNEY_SPARROW, cl_cockneysparrow);
 	//assign_mob(BUFFALO, cl_buffalo);
 	//assign_mob(BALL_PEAN_HAMMER, cl_ballpeanhammer);
 	//assign_mob(HAROLD_SPIDER, cl_haroldspider);
 	//assign_mob(OGGLEBLOP, cl_oggleblop);
 	//assign_mob(SCIENTIST_BOB, cl_scientistbob);

 	//Sherbert Kingdom
 	//assign_mob(SHERBERT_QUEEN, cl_queensherbert);
}