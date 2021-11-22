/*spec.Ubers.c - Specs for Ubers by Fisher

     Written by Fisher for RoninMUD
     Last Modification Date: 3/31/2021

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

/*Rooms */
#define MINE_OPENING 14700

/*Objects */

/*Mobs */
#define HEWER_HUNRAZAR 14705
#define HEWER_IZZILA 14706
#define HEWER_HANID 14707
#define HEWER_LUZZIR 14708
#define PUTTER_NIRUSAL 14709
#define PUTTER_ORRATHUS 14710
#define PUTTER_NANNICIR 14711
#define PUTTER_RUZLAZAN 14712
#define HURRIER_WALDUNID 14713
#define HURRIER_HAXILIG 14714
#define HURRIER_IMLUMITH 14715
#define HURRIER_OMMACATH 14716
#define LOADER_HAMMALAR 14701
#define LOADER_MAZILUTH 14702
#define LOADER_NUNLAZATH 14703
#define LOADER_HAXACAL 14704
#define OZLASAR 14700

#define LEVEL_GUARD_100 14717
#define LEVEL_GUARD_200 14718
#define LEVEL_GUARD_300 14719
#define LEVEL_GUARD_400 14720
#define LEVEL_GUARD_500 14721
//Forge
#define FORGE_TENDER_HUSLATHON    14724
#define FORGE_TENDER_WOKNASIG     14725
#define COAL_TENDER               14726
#define ORE_TENDER                14727
//Leaders
#define LEADER_HORDRAMON          14728
#define LEADER_SUDOSAL          14729
#define LEADER_ZOLRANATH          14730
#define LEADER_VANNUNUS          14731
#define LEADER_NIRDROXATH          14732




/*Miscellaneous strings */
//Generic States that are shifted to indicate different stages.
//Each Uber will use this differently.
#define STATE1 (1 << 0)	//1
#define STATE2 (1 << 1)	//2
#define STATE3 (1 << 2)	//4
#define STATE4 (1 << 3)	//8

/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */

/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */

//Mob Number 14600
//Load in Room 12094

/*
#define HEWER_HUNRAZAR 14705
#define HEWER_IZZILA   14706
#define HEWER_HANID    14707
#define HEWER_LUZZIR   14708

*/

int em_hewer(CHAR *hewer, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR * vict;

	//Add strings if you want them to just speak.  Adds life to zones.
	char *em_hewer_speak[3] = { "Dig...and dig and ...owww, don't hit me with that rock.", "Rocks are tasty, nom nom nom", "The earthsblood must be here somewhere." };

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(hewer, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			if (chance(30))
			{
				sprintf(buf, "%s", em_hewer_speak[number(0, NUMELEMS(em_hewer_speak) - 1)]);

				do_say(hewer, buf, CMD_SAY);
			}

			//if fighting - spec different attacks
			if (hewer->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.
				switch (number(0, 6))
				{
					case 0:
					case 1:
						vict = get_random_victim_fighting(hewer);
						if (vict)
						{
							act("$n hits $N in the head with a pickaxe.", 0, hewer, 0, vict, TO_NOTVICT);
							act("$n hits you in the head with a pickaxe..", 0, hewer, 0, vict, TO_VICT);
							sprintf(buf, "%s is hit on the head with a pickaxe.", GET_NAME(vict));
							act(buf, FALSE, hewer, NULL, vict, TO_NOTVICT);
							sprintf(buf, "You are hit on the head with a pickaxe.");
							act(buf, FALSE, hewer, NULL, vict, TO_VICT);
							GET_HIT(vict) = GET_HIT(vict) - GET_HIT(vict) / 6;
						}

						break;
					case 2:
						vict = get_random_victim_fighting(hewer);
						if (vict)
						{
							act("$n grabs a large rock and throws it at $N knocking them out.", 0, hewer, 0, vict, TO_NOTVICT);
							act("$n grabs a large rock and throws it at you, knocking you out.", 0, hewer, 0, vict, TO_VICT);
							damage(hewer, vict, 800, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *1);
						}

						break;
					case 3:

					case 4:

					case 5:
					case 6:
						break;
					default:
						break;
				}
			}

			break;
	}

	return FALSE;
}
int em_putter(CHAR *putter, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;

	//Add strings if you want them to just speak.  Adds life to zones.
	char *em_putter_speak[3] = { "These carts are really heavy...still better than hewing", "Get it in the cart and not on the floor.", "These rails really need to be addressed.  They are not aligned at all." };

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(putter, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			if (chance(30))
			{
				sprintf(buf, "%s", em_putter_speak[number(0, NUMELEMS(em_putter_speak) - 1)]);

				do_say(putter, buf, CMD_SAY);
			}

			//if fighting - spec different attacks
			if (putter->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.
				switch (number(0, 6))
				{
					case 0:
					case 1:
					case 2:
						vict = get_random_victim_fighting(putter);
						if (vict)
						{
							act("$n rams $N with a large metal cart.", 0, putter, 0, vict, TO_NOTVICT);
							act("$n rams you with a large metal cart.", 0, putter, 0, vict, TO_VICT);
							damage(putter, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *2);
						}

						break;
					case 3:
						act("$n flips the cart over and hits everyone with coal.", 0, putter, 0, 0, TO_ROOM);
						for (vict = world[CHAR_REAL_ROOM(putter)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(putter, vict, 400, TYPE_UNDEFINED, DAM_PHYSICAL);
						}
						break;
					case 4:

					case 5:
						act("$n rides the cart around the room hitting everyone.", 0, putter, 0, 0, TO_ROOM);
						for (vict = world[CHAR_REAL_ROOM(putter)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(putter, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *2);
						}
						break;
					case 6:
						break;
					default:
						break;
				}
			}

			break;
	}

	return FALSE;
}

void em_elemental_stun(CHAR *ch, CHAR *vict)
{
	act("$n smashes you with its fists.", FALSE, ch, 0, vict, TO_VICT);
	act("$n stuns $N with a smash to $S face.", FALSE, ch, 0, vict, TO_NOTVICT);
	act("You hit $N with your fists. $E collapses to the ground.", FALSE, ch, 0, vict, TO_CHAR);

	GET_POS(vict) = POSITION_STUNNED;

	if (GET_OPPONENT(vict))
	{
		stop_fighting(vict);
	}

	if (GET_OPPONENT(ch) && GET_OPPONENT(ch) == vict)
	{
		stop_fighting(ch);
		damage(ch, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
		WAIT_STATE(vict, 2 *PULSE_VIOLENCE);
	}
}

int em_hurrier(CHAR *hurrier, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR * vict;

	//Add strings if you want them to just speak.  Adds life to zones.
	char *em_hurrier_speak[3] = { "Breathing this stuff in can't be good for you.", "At least I get to see the surface.", "Pushing coal back to the surface is so boring. I want to go deeper." };

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(hurrier, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			if (chance(30))
			{
				sprintf(buf, "%s", em_hurrier_speak[number(0, NUMELEMS(em_hurrier_speak) - 1)]);

				do_say(hurrier, buf, CMD_SAY);
			}

			//if fighting - spec different attacks
			if (hurrier->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.
				switch (number(0, 6))
				{
					case 0:
					case 1:
						if (!(vict = get_random_victim(hurrier)))
						{
							return FALSE;
						}

						em_elemental_stun(hurrier, vict);
						break;

					case 2:

						break;
					case 3:
						act("$n flips the cart over and coat dust is scattered everywhere.", 0, hurrier, 0, 0, TO_ROOM);

						vict = get_random_victim_fighting(hurrier);

						if (!IS_AFFECTED(vict, AFF_BLIND))
						{
							act("$n scatters coal dust in $N's eyes.", FALSE, hurrier, 0, vict, TO_NOTVICT);
							act("$n scatters coal dust in your eyes.", FALSE, hurrier, 0, vict, TO_VICT);

							affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_HITROLL, -6, AFF_BLIND, 0);
							affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_ARMOR, 60, AFF_BLIND, 0);
						}

						break;
					case 4:
						break;
					case 5:

						break;
					case 6:
						break;
					default:
						break;
				}
			}

			break;
	}

	return FALSE;
}
int em_loader(CHAR *loader, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;

	//Add strings if you want them to just speak.  Adds life to zones.
	char *em_loader_speak[3] = { "No, dont mix the coal and the ore. They go to seperate places.", "At least I don't have to dig.", "This is the black gold we all love to find." };

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(loader, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			if (chance(30))
			{
				sprintf(buf, "%s", em_loader_speak[number(0, NUMELEMS(em_loader_speak) - 1)]);

				do_say(loader, buf, CMD_SAY);
			}

			//if fighting - spec different attacks
			if (loader->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.
				switch (number(0, 6))
				{
					case 0:
					case 1:
						if (!(vict = get_random_victim(loader)))
						{
							return FALSE;
						}

						em_elemental_stun(loader, vict);
						break;

					case 2:

						break;
					case 3:
						act("$n rolls multiple carts through the room and hits everyone.", 0, loader, 0, 0, TO_ROOM);
						for (vict = world[CHAR_REAL_ROOM(loader)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(loader, vict, 850, TYPE_UNDEFINED, DAM_PHYSICAL);
						}

						break;
					case 4:
						break;
					case 5:
						//Shove them into a cart and move them to the entrance to the level

						vict = get_random_victim_fighting(loader);

						/*Don't teleport non-mortals. */
						if (!IS_MORTAL(vict)) return FALSE;

						act("$n shoves $N's into a minecart and pushes them off into the tunnel.", FALSE, loader, 0, vict, TO_NOTVICT);
						act("$n shoves you into a minecart and pushes you off into the tunnel.", FALSE, loader, 0, vict, TO_VICT);

						char_from_room(vict);
						char_to_room(vict, real_room(MINE_OPENING));

						act("$n appears in a minecart and is dumped to the ground.", TRUE, vict, 0, 0, TO_ROOM);

						do_look(vict, "", CMD_LOOK);

						break;
					case 6:
						break;
					default:
						break;
				}
			}

			break;
	}

	return FALSE;
}

int em_ozlasar(CHAR *ozlasar, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;

	//Add strings if you want them to just speak.  Adds life to zones.
	char *em_ozlasar_speak[3] = { "Get back to work or else you get the whip again.", "If you don't meet your quotas you will be executed.", "First one to find what we are looking for.....gets nothing." };

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(ozlasar, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			if (chance(30))
			{
				sprintf(buf, "%s", em_ozlasar_speak[number(0, NUMELEMS(em_ozlasar_speak) - 1)]);

				do_say(ozlasar, buf, CMD_SAY);
			}

			//if you want to solo him, he is much harder.  Set the bank bit to change is 2nd round of attacks.
			//Also make sure you are constantly checking. We dont want people in the room and not fighting to cheese it.
			if (count_mortals_room(ozlasar, TRUE) > 5)
			{
			 	//STATE1 is > 5 people.  
				if (!IS_SET(GET_BANK(ozlasar), STATE1))
				{
					SET_BIT(GET_BANK(ozlasar), STATE1);
				}
			}
			else
			{
				if (IS_SET(GET_BANK(ozlasar), STATE1))
				{
					REMOVE_BIT(GET_BANK(ozlasar), STATE1);

					return FALSE;
				}
			}

			//if fighting - spec different attacks
			if (ozlasar->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.
				switch (number(0, 4))
				{
					case 0:
					case 1:
						if (!(vict = get_random_victim(ozlasar)))
						{
							return FALSE;
						}

						em_elemental_stun(ozlasar, vict);
						break;

					case 2:

						break;
					case 3:
						act("$n thrashes at everyone in the room.", 0, ozlasar, 0, 0, TO_ROOM);
						for (vict = world[CHAR_REAL_ROOM(ozlasar)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(ozlasar, vict, 1650, TYPE_UNDEFINED, DAM_PHYSICAL);
						}

						break;
					case 4:

						act("$n spins his massive arms around and slams everyone against the wall. ", 0, ozlasar, 0, 0, TO_ROOM);
						for (vict = world[CHAR_REAL_ROOM(ozlasar)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(ozlasar, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *1);
						}

						break;
					default:
						break;
				}

				//2 Different chances for attacks each round.
				//If you are soloing he actually will hit harder and spec differently. 
				//If State1 is set, hit harder and meaner.
				switch (number(0, 3))
				{
				 		//Heal for 10% instead of 5%
					case 0:
						if (!IS_SET(GET_BANK(ozlasar), STATE1))
						{
							do_say(ozlasar, "You dont have enough people to defeat me", CMD_SAY);
							GET_HIT(ozlasar) += GET_HIT(ozlasar) + 5000;
						}
						else
						{
							do_say(ozlasar, "You dont have enough people to defeat me", CMD_SAY);
							GET_HIT(ozlasar) += GET_HIT(ozlasar) + 2500;
						}
						break;
						//Face a torrent of rocks
					case 1:
						if (!IS_SET(GET_BANK(ozlasar), STATE1))
						{
							do_say(ozlasar, "Feel the fury of the taskmaster", CMD_SAY);
							for (vict = world[CHAR_REAL_ROOM(ozlasar)].people; vict; vict = next_vict)
							{
								next_vict = vict->next_in_room;
								if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
								damage(ozlasar, vict, 1700, TYPE_UNDEFINED, DAM_PHYSICAL);
							}
						}
						else
						{
							do_say(ozlasar, "Working together is what gets the job done quicker.", CMD_SAY);
							for (vict = world[CHAR_REAL_ROOM(ozlasar)].people; vict; vict = next_vict)
							{
								next_vict = vict->next_in_room;
								if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
								damage(ozlasar, vict, 1025, TYPE_UNDEFINED, DAM_PHYSICAL);
							}
						}
						break;
						//Single Target Smash. If bit is set, remove from combat.
					case 2:
						vict = get_random_victim(ozlasar);

						if (!IS_SET(GET_BANK(ozlasar), STATE1))
						{
							stop_fighting(vict);
							damage(ozlasar, vict, 1800, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, 2 *PULSE_VIOLENCE);
						}
						else
						{
							stop_fighting(vict);
							damage(ozlasar, vict, 1300, TYPE_UNDEFINED, DAM_PHYSICAL);
						}
						break;
					case 3:
						break;
					default:
						break;
				}
			}
			else
			{
			 	//Remove Existing States if not fighting.

				/*If not fighting, and wimpy, remove wimpy. */
				if (!GET_OPPONENT(ozlasar) && IS_SET(GET_BANK(ozlasar), STATE1))
				{
					REMOVE_BIT(GET_BANK(ozlasar), STATE1);

					return FALSE;
				}
			}

			break;
	}

	return FALSE;
}

int em_levelguard(CHAR *levelguard, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;

	//Add strings if you want them to just speak.  Adds life to zones.
	char *em_levelguard_speak[3] = { "The level below is even more dangerous. Caution is recommended.", "The mines are still unstable - be wary.", "We hope to find the Great Dismal Delve" };

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(levelguard, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			if (chance(30))
			{
				sprintf(buf, "%s", em_levelguard_speak[number(0, NUMELEMS(em_levelguard_speak) - 1)]);

				do_say(levelguard, buf, CMD_SAY);
			}

			//if fighting - spec different attacks
			if (levelguard->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.
				switch (number(0, 4))
				{
					case 0:
						vict = get_random_victim_fighting(levelguard);
						if (vict)
						{
							act("$n smashes $N in between its two large arms.", 0, levelguard, 0, vict, TO_NOTVICT);
							act("$n smashes you in between two large arms.", 0, levelguard, 0, vict, TO_VICT);
							damage(levelguard, vict, 1700, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *1);
						}
						break;
					case 1:
						if (!(vict = get_random_victim(levelguard)))
						{
							return FALSE;
						}

						em_elemental_stun(levelguard, vict);
						break;

					case 2:

						break;
					case 3:
						act("$n conjurs rock spheres and throws that at everyone.", 0, levelguard, 0, 0, TO_ROOM);
						for (vict = world[CHAR_REAL_ROOM(levelguard)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(levelguard, vict, 1550, TYPE_UNDEFINED, DAM_PHYSICAL);
						}
						break;
					case 4:
						break;
					case 5:
						break;
					case 6:
						break;
					default:
						break;
				}

				//Every Round room spec.
				act("$n smashes the ground causing rocks to fall from the ceiling.", 0, levelguard, 0, 0, TO_ROOM);
				for (vict = world[CHAR_REAL_ROOM(levelguard)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
					damage(levelguard, vict, 875, TYPE_UNDEFINED, DAM_PHYSICAL);
				}
			}

			break;
	}

	return FALSE;
}

int em_forgetender(CHAR *forgetender, CHAR *ch, int cmd, char *arg)
{
 	//These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
 	  
  //Add strings if you want them to just speak.  Adds life to zones.
  char *em_forgetender_speak[3] = { "Its hotter than the Plane of Elemental Fire in here.", "The fires are kept burning at all times.", "Burn......"};

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(forgetender, TRUE) < 1) return FALSE;

  switch (cmd)
  {
    case MSG_MOBACT:
	
	  if (chance(30))
      {
        sprintf(buf, "%s", em_forgetender_speak[number(0, NUMELEMS(em_forgetender_speak) - 1)]);

        do_say(forgetender, buf, CMD_SAY);
      }
	

     	//if fighting - spec different attacks
      if (forgetender->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.
        switch (number(0, 4))
        {
          case 0:
		    vict = get_random_victim_fighting(forgetender);
			  if (vict)
				{
				  act("$n smashes $N with a metal rod.", 0, forgetender, 0, vict, TO_NOTVICT);
				  act("$n smashes you with a metal rod.", 0, forgetender, 0, vict, TO_VICT);
				  damage(forgetender, vict, 1300, TYPE_UNDEFINED, DAM_PHYSICAL);
				 
				}
			break;
          case 1:
			if (!(vict = get_random_victim(forgetender)))
            {
              return FALSE;
            }

            em_elemental_stun(forgetender, vict);
            break;
		  
          case 2:
           vict = get_random_victim_fighting(forgetender);
			  if (vict)
				{
				  act("$n skewers $N with a metal rod and throws them against the wall.", 0, forgetender, 0, vict, TO_NOTVICT);
				  act("$n skewers you with a metal rod and throws you against the wall.", 0, forgetender, 0, vict, TO_VICT);
				  damage(forgetender, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
				  WAIT_STATE(vict, PULSE_VIOLENCE *3);
				}
            break;
          case 3:
            act("$n conjours rock spheres and throws that at everyone.", 0, forgetender, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(forgetender)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(forgetender, vict, 1550, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
		 
		 
			break;
          case 4:
			if (!IS_SET(GET_BANK(forgetender), STATE1))
				{
					do_say(forgetender, "Lets turn up the heat in here.", CMD_SAY);
					SET_BIT(GET_BANK(forgetender), STATE1);
				}
		  
           break;        
          default:
            break;
			
        }
		
		if (IS_SET(GET_BANK(forgetender), STATE1))
			{
		
		//Every Round room spec.
		    act("$n flings burning coal around the room.", 0, forgetender, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(forgetender)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(forgetender, vict, 400, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
			
			}
		
		
      }
      else
			{
			 	//Remove Existing States if not fighting.

				/*If not fighting, and wimpy, remove wimpy. */
				if (!GET_OPPONENT(forgetender) && IS_SET(GET_BANK(forgetender), STATE1))
				{
					REMOVE_BIT(GET_BANK(forgetender), STATE1);

					return FALSE;
				}
			}      

      break;  
  }

  return FALSE;
}


int em_coaltender(CHAR *coaltender, CHAR *ch, int cmd, char *arg)
{
 	//These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
 	  
  //Add strings if you want them to just speak.  Adds life to zones.
  char *em_coaltender_speak[3] = { "Coal, Coal Coal, Shovel All The Coal...", "Black Gold is so much better than the other things we find in here.", "Shovel Here...Shovel There...Shovel everywhere..."};

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(coaltender, TRUE) < 1) return FALSE;

  switch (cmd)
  {
    case MSG_MOBACT:
	
	  if (chance(30))
      {
        sprintf(buf, "%s", em_coaltender_speak[number(0, NUMELEMS(em_coaltender_speak) - 1)]);

        do_say(coaltender, buf, CMD_SAY);
      }
	

     	//if fighting - spec different attacks
      if (coaltender->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.
        switch (number(0, 4))
        {
          case 0:
		    vict = get_random_victim_fighting(coaltender);
			  if (vict)
				{
				  act("$n smashes $N with a fistful of coal.", 0, coaltender, 0, vict, TO_NOTVICT);
				  act("$n smashes you with a fistful of coal", 0, coaltender, 0, vict, TO_VICT);
				  damage(coaltender, vict, 1600, TYPE_UNDEFINED, DAM_PHYSICAL);
				 
				}
			break;
          case 1:
			if (!(vict = get_random_victim(coaltender)))
            {
              return FALSE;
            }

            em_elemental_stun(coaltender, vict);
            break;
		  
          case 2:
            break;
          case 3:
            act("$n conjours rock spheres and throws that at everyone.", 0, coaltender, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(coaltender)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(coaltender, vict, 1750, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
		 
		 
			break;
          case 4:
			if (!IS_SET(GET_BANK(coaltender), STATE1))
				{
					do_say(coaltender, "Lets turn up the heat in here.", CMD_SAY);
					SET_BIT(GET_BANK(coaltender), STATE1);
				}
		  
           break;        
          default:
            break;
			
        }
		
		if (IS_SET(GET_BANK(coaltender), STATE1))
			{
		
		//Every Round room spec.
		    act("$n flings burning coal around the room.", 0, coaltender, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(coaltender)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(coaltender, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
			
			}
		
		
      }
      else
			{
			 	//Remove Existing States if not fighting.

				/*If not fighting, and wimpy, remove wimpy. */
				if (!GET_OPPONENT(coaltender) && IS_SET(GET_BANK(coaltender), STATE1))
				{
					REMOVE_BIT(GET_BANK(coaltender), STATE1);

					return FALSE;
				}
			}      

      break;  
  }

  return FALSE;
}

int em_oretender(CHAR *oretender, CHAR *ch, int cmd, char *arg)
{
 	//These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
 	  
  //Add strings if you want them to just speak.  Adds life to zones.
  char *em_oretender_speak[3] = { "The power of the earth is stunning.", "Unrefined ore is so beautiful, dont you think?", "This ore will become a wonderful tool one day."};

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(oretender, TRUE) < 1) return FALSE;

  switch (cmd)
  {
    case MSG_MOBACT:
	
	  if (chance(30))
      {
        sprintf(buf, "%s", em_oretender_speak[number(0, NUMELEMS(em_oretender_speak) - 1)]);

        do_say(oretender, buf, CMD_SAY);
      }
	

     	//if fighting - spec different attacks
      if (oretender->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.
        switch (number(0, 4))
        {
          case 0:
		    act("$n swirls a shovel and scatters ore around the room.", 0, oretender, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(oretender)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(oretender, vict, 1300, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
			break;
          case 1:
		
            break;
		  
          case 2:
            break;
          case 3:
            act("$n throws water on the furnace, burning the room with hot steam.", 0, oretender, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(oretender)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(oretender, vict, 1680, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
		 
		 
			break;
          case 4:
			if (!IS_SET(GET_BANK(oretender), STATE1))
				{
					do_say(oretender, "Lets turn up the heat in here.", CMD_SAY);
					SET_BIT(GET_BANK(oretender), STATE1);
				}
		  
           break;        
          default:
            break;
			
        }
		
		if (IS_SET(GET_BANK(oretender), STATE1))
			{
		
		//Every Round room spec.
		    act("$n flings burning coal around the room.", 0, oretender, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(oretender)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(oretender, vict, 620, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
			
			}
		
		
      }
      else
			{
			 	//Remove Existing States if not fighting.

				/*If not fighting, and wimpy, remove wimpy. */
				if (!GET_OPPONENT(oretender) && IS_SET(GET_BANK(oretender), STATE1))
				{
					REMOVE_BIT(GET_BANK(oretender), STATE1);

					return FALSE;
				}
			}      

      break;  
  }

  return FALSE;
}

int em_leader(CHAR *leader, CHAR *ch, int cmd, char *arg)
{
 	//These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
 	  
  //Add strings if you want them to just speak.  Adds life to zones.
  char *em_leader_speak[3] = { "", "", ""};

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(leader, TRUE) < 1) return FALSE;

  switch (cmd)
  {
    case MSG_MOBACT:
	
	  if (chance(30))
      {
        sprintf(buf, "%s", em_leader_speak[number(0, NUMELEMS(em_leader_speak) - 1)]);

        do_say(leader, buf, CMD_SAY);
      }
	

     	//if fighting - spec different attacks
      if (leader->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.
        switch (number(0, 4))
        {
          case 0:
		    act("$n swirls a shovel and scatters ore around the room.", 0, leader, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(leader)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(leader, vict, 850, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
			break;
          case 1:
		
            break;
		  
          case 2:
            break;
          case 3:
            act("$n throws water on the furnace, burning the room with hot steam.", 0, leader, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(leader)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(leader, vict, 925, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
		 
		 
			break;
          case 4:
			if (!IS_SET(GET_BANK(leader), STATE1))
				{
					do_say(leader, "Lets turn up the heat in here.", CMD_SAY);
					act("$n flings burning coal around the room.", 0, leader, 0, 0, TO_ROOM);
					SET_BIT(GET_BANK(leader), STATE1);
				}
		  
           break;        
          default:
            break;
			
        }
		
		if (IS_SET(GET_BANK(leader), STATE1))
			{
		
		    //Every Round room spec.
		    act("$n flings burning coal around the room.", 0, leader, 0, 0, TO_ROOM);
            for (vict = world[CHAR_REAL_ROOM(leader)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
              damage(leader, vict, 700, TYPE_UNDEFINED, DAM_PHYSICAL);
              
            } 
				switch (number(0, 2))
				{
					case 0:
					vict = get_random_victim_fighting(leader);
				  if (vict)
					{
					  act("$n pierces $N with a sharp shard of rock.", 0, leader, 0, vict, TO_NOTVICT);
					  act("$n perces you with a sharp shard of rock", 0, leader, 0, vict, TO_VICT);
					  damage(leader, vict, 2200, TYPE_UNDEFINED, DAM_PHYSICAL);
					 
					}
					break;
					case 1:
					break;
					case 2:
					vict = get_random_victim_fighting(leader);
					  if (vict)
						{
						  act("$n crushes $N with a very large rock.", 0, leader, 0, vict, TO_NOTVICT);
						  act("$n crushes you with a very large rock", 0, leader, 0, vict, TO_VICT);
						  damage(leader, vict, 1650, TYPE_UNDEFINED, DAM_PHYSICAL);
						 
						}
					break;
					default:
					break;
				}
			
			
			
			}
		
		
      }
      else
			{
			 	//Remove Existing States if not fighting.

				/*If not fighting, and wimpy, remove wimpy. */
				if (!GET_OPPONENT(leader) && IS_SET(GET_BANK(leader), STATE1))
				{
					REMOVE_BIT(GET_BANK(leader), STATE1);

					return FALSE;
				}
			}      

      break;  
  }

  return FALSE;
}


/*



loader_leader
hewer_leader
putter_leader
hurrier_leader
rock_elemental_leader
forge_master
Royal_guard
team_leader
scout
ogremoch
sunnis
grumbar

*/

//Assign Spec for the zone. Sets all other specs.

void assign_elementalmines(void)
{
	/*Objects */
	//assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);

	/*Rooms */
	//assign_room(LIGHTWALL,             	cl_LightWallLink);

	/*Mobs */
	//Hewers
	assign_mob(HEWER_HUNRAZAR, em_hewer);
	assign_mob(HEWER_IZZILA, em_hewer);
	assign_mob(HEWER_HANID, em_hewer);
	assign_mob(HEWER_LUZZIR, em_hewer);

	//Putters
	assign_mob(PUTTER_NIRUSAL, em_putter);
	assign_mob(PUTTER_ORRATHUS, em_putter);
	assign_mob(PUTTER_NANNICIR, em_putter);
	assign_mob(PUTTER_RUZLAZAN, em_putter);

	//Hurriers
	assign_mob(HURRIER_WALDUNID, em_hurrier);
	assign_mob(HURRIER_HAXILIG, em_hurrier);
	assign_mob(HURRIER_IMLUMITH, em_hurrier);
	assign_mob(HURRIER_OMMACATH, em_hurrier);

	//Loaders
	assign_mob(LOADER_HAMMALAR, em_loader);
	assign_mob(LOADER_MAZILUTH, em_loader);
	assign_mob(LOADER_NUNLAZATH, em_loader);
	assign_mob(LOADER_HAXACAL, em_loader);

	assign_mob(OZLASAR, em_ozlasar);

    //Levelguards
	assign_mob(LEVEL_GUARD_100, em_levelguard);
	assign_mob(LEVEL_GUARD_200, em_levelguard);
	assign_mob(LEVEL_GUARD_300, em_levelguard);
	assign_mob(LEVEL_GUARD_400, em_levelguard);
	assign_mob(LEVEL_GUARD_500, em_levelguard);
	
	//Forge Area
	assign_mob(FORGE_TENDER_HUSLATHON, em_forgetender);
	assign_mob(FORGE_TENDER_WOKNASIG, em_forgetender);
	assign_mob(COAL_TENDER, em_coaltender);
	assign_mob(ORE_TENDER, em_oretender);

	//Leaders
	assign_mob(LEADER_HORDRAMON, em_leader);
	assign_mob(LEADER_SUDOSAL, em_leader);
	assign_mob(LEADER_ZOLRANATH, em_leader);
	assign_mob(LEADER_VANNUNUS, em_leader);
	assign_mob(LEADER_NIRDROXATH, em_leader);
	
	

}