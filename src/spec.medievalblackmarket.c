/*spec.MedievalBlackMarket.c - Specs for Medieval Black Market by Fisher

     Written by Fisher for RoninMUD
     Last Modification Date: 11/16/2021

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
#define JUNK_PILE 18010

/*Objects */

/*Mobs */
#define GUISE_MERCHANT 18000
#define SPECTOR_MERCHANT 18001
#define ECHO_MERCHANT 18002
#define MIME_MERCHANT 18003
#define FUSE_MERCHANT 18004
#define FEIGN_MERCHANT 18005

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

int blackmarket_guise(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	//Stores the random people to talk to.
	CHAR * vict;
	//Random Actions to be performed.
	//char *mob_social_actions[4] = { "nudge ", "cough ", "cod ", "highfive " };	
	char *cough = "cough";
	char *nudge = "nudge";

	/*
	Random Item List
	3013 - Bronze Tradebar
	3014 - Silver Tradebar
	3015 - Gold Tradebar

	30027 - Tricky Recipe for Spoon
	30028 - Complicated Recipe for Ubertank Plate
	30029 - Difficult Recipe for Crash Test Helmet
	30030 - Ambitous Recipe for Regenerator
	30031 - Fancy Recipe for Can
	30032 - Hard Recipe for Casters Cloak
	30033 - Tough Recipe for Boots
	30034 - Challenging Recipe for Chopsticks
	*/

	int random_item_rewards[11] = { 3013, 3014, 3015, 30027, 30028, 30029, 30030, 30031, 30032, 30033, 30034 };
	//char socialAct;
	int reward;

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	//Have the vendor talk.
	if (cmd == MSG_TICK)
	{
		if (chance(50))
		{
			//Get a random victim to help add more life to the characters.
			vict = get_random_victim(mob);
			if (vict)
			{
				//socialAct = mob_social_actions[number(0, NUMELEMS(mob_social_actions) - 1)];
				//Get Random Social Acction

				//Get Victim Name and concat it.
				//str_cat(socialAct,10,GET_DISP_NAME(vict));

				//do_social(mob, socialAct, CMD_SOCIAL);
			}

			switch (number(0, 5))
			{
				case (0):

					do_say(mob, "Hey You....you got the tokens....I'll give you something good in exchange.", CMD_SAY);
					break;
				case (1):
					do_say(mob, "One Token for a roll of the dice. Great Rewards to those that provide.", CMD_SAY);
					break;
				case (2):
					do_social(mob, nudge, CMD_SOCIAL);
					do_say(mob, "Ahem.  Give me a token please.", CMD_SAY);
					break;
				case (3):
					do_social(mob, cough, CMD_SOCIAL);
					do_say(mob, "Come and try your luck with the dice, only one Token.", CMD_SAY);
					break;
				case (4):
					//do_social(mob, cough, CMD_SOCIAL);
					do_say(mob, "TOKENS. I must have them all!", CMD_SAY);
					break;
				case (5):
					break;
				default:
					break;
			}
		}

		return FALSE;

	}

	//If threatened with violence, put them at 1 mana and hp and move to another roon. 
	if (cmd == MSG_VIOLENCE)
	{
		do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

		act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

		for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;

			/*Only teleport mortals. */
			if (!IS_MORTAL(vict)) continue;

			act("$n is shived in the gut, and then carried off by someone from the shadows.!", FALSE, vict, 0, 0, TO_ROOM);
			GET_HIT(vict) = 1;
			GET_MANA(vict) = 1;
			char_from_room(vict);
			char_to_room(vict, real_room(JUNK_PILE));

			act("$n is thrown into the refuse pile!", TRUE, vict, 0, 0, TO_ROOM);

			do_look(vict, "", CMD_LOOK);
		}

		return FALSE;
	}

	//Main spec for giving a SC Token
	/*

		If the item is not a token, simply give it back.
		If it is a token, consume it and have the mob mimic a throw.
			Rewards are random

	*/

	if (cmd == MSG_OBJ_GIVEN)
	{
		char buf[MIL];

		arg = one_argument(arg, buf);

		OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);
		//OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

		if (!obj) return TRUE;

		bool give_back = FALSE;

		if (OBJ_TYPE(obj) != ITEM_SC_TOKEN)
		{
			do_say(mob, "I dont want this junk..take it back.", CMD_SAY);

			give_back = TRUE;
		}

		if (give_back)
		{
			act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);

			obj_from_char(obj);
			obj_to_char(obj, ch);

			return TRUE;
		}

		//Remove the Token
		extract_obj(obj);

		//Roll Die.
		do_say(mob, "Lets see what you win this time.", CMD_SAY);
		act("$n pulls out a pair of bone die and throws them on the ground.", TRUE, mob, 0, 0, TO_ROOM);

		switch (number(0, 6))
		{
			//Award Random AQP (1-3 AQP)
			case 0:

				reward = number(1, 3);

				if (reward == 3)
				{
					do_say(mob, "Big Winner over here.", CMD_SAY);
				}
				else
				{
					do_say(mob, "Gotta love those points.", CMD_SAY);
				}

				ch->ver3.quest_points += reward;
				break;
				//Give Random Gold
			case 1:
				do_say(mob, "Shiny shiny gold.", CMD_SAY);
				reward = number(10000, 40000);
				GET_GOLD(ch) = GET_GOLD(ch) + reward;
				break;
				//Give AQP + 1
			case 2:
				//do_say(mob, "Let me cash that for you.", CMD_SAY);
				//ch->ver3.subclass_points += 1;
				
				do_say(mob, "Use this point wisely.", CMD_SAY);
				ch->ver3.quest_points += 1;
				break;

				break;
				//Give Junk Item
				//Pick from an array!
			case 3:
				do_say(mob, "Enjoy this item, courtesy of us.", CMD_SAY);

				reward = random_item_rewards[number(0, NUMELEMS(random_item_rewards) - 1)];

				//Read the Item
				OBJ *reward_object = read_object(reward, VIRTUAL);
				//Give Object to Char.
				obj_to_char(reward_object, ch);
				act("$N reaches into a box and gives you $p.", FALSE, ch, reward_object, mob, TO_CHAR);
				break;
				//Give 1 Coin
			case 4:
				do_say(mob, "One measly coin...come try again.", CMD_SAY);
				GET_GOLD(ch) = GET_GOLD(ch) + 1;
				break;
				//1 AQP
			case 5:
				do_say(mob, "Use this point wisely.", CMD_SAY);
				ch->ver3.quest_points += 1;
				break;
				//110k Gold Reward
			case 6:
				do_say(mob, "Gold rules this world.", CMD_SAY);
				GET_GOLD(ch) = GET_GOLD(ch) + 110000;
				break;
			default:
				break;
		}

		return TRUE;
	}

	return FALSE;
}

int blackmarket_spector(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	//Have the vendor talk.
	if (cmd == MSG_TICK)
	{
		if (chance(50))
		{
			switch (number(0, 5))
			{
				case (0):
				case (1):
				case (2):
				case (3):
				case (4):
				case (5):
					do_say(mob, "Watch yourself in the black market.", CMD_SAY);
					break;
				default:
					break;
			}
		}

		return FALSE;
	}

	//If threatened with violence, put them at 1 mana and hp and move to another roon. 
	if (cmd == MSG_VIOLENCE)
	{
		do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

		act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

		for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;

			/*Only teleport mortals. */
			if (!IS_MORTAL(vict)) continue;

			act("$n is shived in the gut, and then carried off by someone from the shadows.!", FALSE, vict, 0, 0, TO_ROOM);
			GET_HIT(vict) = 1;
			GET_MANA(vict) = 1;
			char_from_room(vict);
			char_to_room(vict, real_room(JUNK_PILE));

			act("$n is thrown into the refuse pile!", TRUE, vict, 0, 0, TO_ROOM);

			do_look(vict, "", CMD_LOOK);
		}

		return FALSE;
	}


	if (cmd == MSG_OBJ_GIVEN)
	{
		char buf[MIL];

		arg = one_argument(arg, buf);

		OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);
		//OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

		if (!obj) return TRUE;
		
		if (OBJ_TYPE(obj) == ITEM_SC_TOKEN)
		{
			do_say(mob, "Go see Guise.. he loves these things.", CMD_SAY);			
		}
		else
		{
			do_say(mob, "I dont want this junk..take it back.", CMD_SAY);
		}

		bool give_back = TRUE;		

		if (give_back)
		{
			
			act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);
			obj_from_char(obj);
			obj_to_char(obj, ch);

			return TRUE;
		}
	}
	
	return FALSE;
}

int blackmarket_echo(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	//Have the vendor talk.
	if (cmd == MSG_TICK)
	{
		if (chance(50))
		{
			switch (number(0, 5))
			{
				case (0):
				case (1):
				case (2):
				case (3):
				case (4):
				case (5):
					do_say(mob, "Watch yourself in the black market.", CMD_SAY);
					break;
				default:
					break;
			}
		}

		return FALSE;
	}

	//If threatened with violence, put them at 1 mana and hp and move to another roon. 
	if (cmd == MSG_VIOLENCE)
	{
		do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

		act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

		for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;

			/*Only teleport mortals. */
			if (!IS_MORTAL(vict)) continue;

			act("$n is shived in the gut, and then carried off by someone from the shadows.!", FALSE, vict, 0, 0, TO_ROOM);
			GET_HIT(vict) = 1;
			GET_MANA(vict) = 1;
			char_from_room(vict);
			char_to_room(vict, real_room(JUNK_PILE));

			act("$n is thrown into the refuse pile!", TRUE, vict, 0, 0, TO_ROOM);

			do_look(vict, "", CMD_LOOK);
		}

		return FALSE;
	}

   if (cmd == MSG_OBJ_GIVEN)
	{
		char buf[MIL];

		arg = one_argument(arg, buf);

		OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);
		//OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

		if (!obj) return TRUE;
		
		if (OBJ_TYPE(obj) == ITEM_SC_TOKEN)
		{
			do_say(mob, "We trade for these here, but I am not the one to speak to.", CMD_SAY);			
		}
		else
		{
			do_say(mob, "I dont want this junk..take it back.", CMD_SAY);
		}

		bool give_back = TRUE;		

		if (give_back)
		{
			
			act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);
			obj_from_char(obj);
			obj_to_char(obj, ch);

			return TRUE;
		}
	}

	return FALSE;
}

int blackmarket_mime(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	//Have the vendor talk.
	if (cmd == MSG_TICK)
	{
		if (chance(50))
		{
			switch (number(0, 5))
			{
				case (0):
				case (1):
				case (2):
				case (3):
				case (4):
				case (5):
					do_say(mob, "Watch yourself in the black market.", CMD_SAY);
					break;
				default:
					break;
			}
		}

		return FALSE;
	}

	//If threatened with violence, put them at 1 mana and hp and move to another roon. 
	if (cmd == MSG_VIOLENCE)
	{
		do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

		act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

		for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;

			/*Only teleport mortals. */
			if (!IS_MORTAL(vict)) continue;

			act("$n is shived in the gut, and then carried off by someone from the shadows.!", FALSE, vict, 0, 0, TO_ROOM);
			GET_HIT(vict) = 1;
			GET_MANA(vict) = 1;
			char_from_room(vict);
			char_to_room(vict, real_room(JUNK_PILE));

			act("$n is thrown into the refuse pile!", TRUE, vict, 0, 0, TO_ROOM);

			do_look(vict, "", CMD_LOOK);
		}

		return FALSE;
	}
	
	if (cmd == MSG_OBJ_GIVEN)
	{
		char buf[MIL];

		arg = one_argument(arg, buf);

		OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);
		//OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

		if (!obj) return TRUE;
		
		if (OBJ_TYPE(obj) == ITEM_SC_TOKEN)
		{
			do_say(mob, "Guise is the one you want, not me.", CMD_SAY);			
		}
		else
		{
			do_say(mob, "I dont want this junk..take it back.", CMD_SAY);
		}

		bool give_back = TRUE;		

		if (give_back)
		{
			
			act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);
			obj_from_char(obj);
			obj_to_char(obj, ch);

			return TRUE;
		}
	}

	return FALSE;
}

int blackmarket_fuse(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	//Have the vendor talk.
	if (cmd == MSG_TICK)
	{
		if (chance(50))
		{
			switch (number(0, 5))
			{
				case (0):
				case (1):
				case (2):
				case (3):
				case (4):
				case (5):
					do_say(mob, "Watch yourself in the black market.", CMD_SAY);
					break;
				default:
					break;
			}
		}

		return FALSE;
	}

	//If threatened with violence, put them at 1 mana and hp and move to another roon. 
	if (cmd == MSG_VIOLENCE)
	{
		do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

		act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

		for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;

			/*Only teleport mortals. */
			if (!IS_MORTAL(vict)) continue;

			act("$n is shived in the gut, and then carried off by someone from the shadows.!", FALSE, vict, 0, 0, TO_ROOM);
			GET_HIT(vict) = 1;
			GET_MANA(vict) = 1;
			char_from_room(vict);
			char_to_room(vict, real_room(JUNK_PILE));

			act("$n is thrown into the refuse pile!", TRUE, vict, 0, 0, TO_ROOM);

			do_look(vict, "", CMD_LOOK);
		}

		return FALSE;
	}
	if (cmd == MSG_OBJ_GIVEN)
	{
		char buf[MIL];

		arg = one_argument(arg, buf);

		OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);
		//OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

		if (!obj) return TRUE;
		
		if (OBJ_TYPE(obj) == ITEM_SC_TOKEN)
		{
			do_say(mob, "Go see Guise.. he loves these things.", CMD_SAY);			
		}
		else
		{
			do_say(mob, "I dont want this junk..take it back.", CMD_SAY);
		}

		bool give_back = TRUE;		

		if (give_back)
		{
			
			act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);
			obj_from_char(obj);
			obj_to_char(obj, ch);

			return TRUE;
		}
	}

	return FALSE;
}

int blackmarket_feign(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	//Have the vendor talk.
	if (cmd == MSG_TICK)
	{
		if (chance(50))
		{
			switch (number(0, 5))
			{
				case (0):
				case (1):
				case (2):
				case (3):
				case (4):
				case (5):
					do_say(mob, "Watch yourself in the black market.", CMD_SAY);
					break;
				default:
					break;
			}
		}

		return FALSE;
	}

	//If threatened with violence, put them at 1 mana and hp and move to another roon. 
	if (cmd == MSG_VIOLENCE)
	{
		do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

		act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

		for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;

			/*Only teleport mortals. */
			if (!IS_MORTAL(vict)) continue;

			act("$n is shived in the gut, and then carried off by someone from the shadows.!", FALSE, vict, 0, 0, TO_ROOM);
			GET_HIT(vict) = 1;
			GET_MANA(vict) = 1;
			char_from_room(vict);
			char_to_room(vict, real_room(JUNK_PILE));

			act("$n is thrown into the refuse pile!", TRUE, vict, 0, 0, TO_ROOM);

			do_look(vict, "", CMD_LOOK);
		}

		return FALSE;
	}

	if (cmd == MSG_OBJ_GIVEN)
	{
		char buf[MIL];

		arg = one_argument(arg, buf);

		OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);
		//OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

		if (!obj) return TRUE;
		
		if (OBJ_TYPE(obj) == ITEM_SC_TOKEN)
		{
			do_say(mob, "Go see Guise.. he loves these things.", CMD_SAY);			
		}
		else
		{
			do_say(mob, "I dont want this junk..take it back.", CMD_SAY);
		}

		bool give_back = TRUE;		

		if (give_back)
		{
			
			act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);
			obj_from_char(obj);
			obj_to_char(obj, ch);

			return TRUE;
		}
	}
	
	return FALSE;
}

//Assign Spec for the zone. Sets all other specs. 
//First Param - Object, Room or Mob Number.  Define it up above.
//Second Param - the name of the function that is for the mob Usually a shorthand for your zone. 
void assign_medievalblackmarket(void)
{
	/*Objects */
	//assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);

	/*Rooms */
	//assign_room(LIGHTWALL,             	cl_LightWallLink);

	/*Mobs */

	assign_mob(GUISE_MERCHANT, blackmarket_guise);
	assign_mob(SPECTOR_MERCHANT, blackmarket_spector);
	assign_mob(ECHO_MERCHANT, blackmarket_echo);
	assign_mob(MIME_MERCHANT, blackmarket_mime);
	assign_mob(FUSE_MERCHANT, blackmarket_fuse);
	assign_mob(FEIGN_MERCHANT, blackmarket_feign);

}