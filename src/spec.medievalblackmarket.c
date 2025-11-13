/*spec.MedievalBlackMarket.c - Specs for Medieval Black Market by Fisher

     Written by Fisher for RoninMUD
     Creation Date: 11/16/2021
     Modified Date: 12/27/2024 - Updated Guise Spec for Better Items
     Basic Specs for the mobs and rooms in the zone.
*/
/*System Includes */
#include <string.h>
#include <stdlib.h>
#include <math.h>

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
#define RUNEGATE 18029

/*Objects */
#define QUESTGIVER_TOKEN_HIGH_CARD 18000
#define QUESTGIVER_TOKEN_QUEST_ONE 18001
#define QUESTGIVER_TOKEN_QUEST_TWO 18002
#define QUESTGIVER_TOKEN_QUEST_THREE 18003
#define QUESTGIVER_TOKEN_QUEST_FOUR 18004
#define QUESTGIVER_TOKEN_QUEST_FIVE 18005
#define QUESTGIVER_TOKEN_QUEST_SIX 18006
#define QUESTGIVER_TOKEN_QUEST_SEVEN 18007
#define QUESTGIVER_TOKEN_QUEST_EIGHT 18008
#define QUESTGIVER_TOKEN_QUEST_NINE 18009
#define QUESTGIVER_TOKEN_QUEST_TEN 18010
#define LOOT_GOBLIN_GOLD 18011

/*Mobs */
#define GUISE_MERCHANT 18000
#define SPECTOR_MERCHANT 18001
#define ECHO_MERCHANT 18002
#define MIME_MERCHANT 18003
#define FUSE_MERCHANT 18004
#define FEIGN_MERCHANT 18005
#define QUESTGIVER 18006

#define LOOT_GOBLIN_OFFICER_ONE 18018
#define LOOT_GOBLIN_OFFICER_TWO 18019
#define LOOT_GOBLIN_OFFICER_THREE 18020
#define LOOT_GOBLIN_OFFICER_FOUR 18021
#define LOOT_GOBLIN_BOSS 18022
#define LOOT_GOBLIN 18023
#define GRINTAK_HUNTER 18024

/* Dummies for Quests */
#define QUESTGIVER_DUMMY_HIGH_CARD 18007
#define QUESTGIVER_DUMMY_QUEST_ONE 18008
#define QUESTGIVER_DUMMY_QUEST_TWO 18009
#define QUESTGIVER_DUMMY_QUEST_THREE 18010
#define QUESTGIVER_DUMMY_QUEST_FOUR 18011
#define QUESTGIVER_DUMMY_QUEST_FIVE 18012
#define QUESTGIVER_DUMMY_QUEST_SIX 18013
#define QUESTGIVER_DUMMY_QUEST_SEVEN 18014
#define QUESTGIVER_DUMMY_QUEST_EIGHT 18015
#define QUESTGIVER_DUMMY_QUEST_NINE 18016
#define QUESTGIVER_DUMMY_QUEST_TEN 18017

/* Misc */
#define QUESTGIVER_TOKEN_MIN_VNUM 18000
#define QUESTGIVER_TOKEN_MAX_VNUM 18010

#define QUESTGIVER_DUMMY_MIN_VNUM 18007
#define QUESTGIVER_DUMMY_MAX_VNUM 18017


/* Zones */


#define TURKEY_TAKEOVER_ZONE 181
#define TURKEY_TAKEOVER_ROOM 18143


#define CHRISTMAS_VILLAGE_ZONE 263
#define CHRISTMAS_VILLAGE_ROOM 26301

/*Miscellaneous strings */
// Generic States that are shifted to indicate different stages.
// Each Uber will use this differently.
#define STATE1 (1 << 0) // 1
#define STATE2 (1 << 1) // 2
#define STATE3 (1 << 2) // 4
#define STATE4 (1 << 3) // 8

/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */


int getCurrentMonth()
{
	//If its Christmas Time, dont remove the link
	
	
	time_t t;
    struct tm *current_time;

    // Get the current system time
    t = time(NULL);
    current_time = localtime(&t);

    int month = current_time->tm_mon + 1;  // tm_mon is 0–11, so add 1
    //int day = current_time->tm_mday;       // tm_mday is 1–31

	return month;
}




/* Teleport players into one of the elemental canyon zones randomly. */
int blackmarket_runegate(int room, CHAR *ch, int cmd, char *arg) {
	
  int currentMonth = getCurrentMonth();
	
  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch)) return FALSE;

    act("The forest opens into a wide clearing where a stone ring stands at its center, the runegate pulses faintly in the soft light.", FALSE, ch, 0, 0, TO_CHAR);

    return FALSE;
  }

  if (cmd == CMD_USE) {
    char buf[MIL];

    one_argument(arg, buf);

    if (!*buf) return FALSE;

    if (!isname(buf, "runegate rune")) return FALSE;

    act("The runes along the runegate ignite in a burst of blue light, tracing the full circle before the air within folds inward and pulls you through.", FALSE, ch, 0, 0, TO_CHAR);
    act("The runegate blaze to life as $n touches the Runegate. The air inside the circle collapses with a flash, and $n vanishes into the light.", TRUE, ch, 0, 0, TO_ROOM);
	
	int teleport_room = NOWHERE;

      switch (currentMonth) {
        case 1:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
        case 2:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
		case 3:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
		case 4:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
		case 5:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
		case 6:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
		case 7:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
		case 8:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
		case 9:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
		case 10:
			//teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;          
			break;
		case 11:
			//Validate the zone is loaded.
			if(real_zone(TURKEY_TAKEOVER_ZONE) != -1){
				teleport_room = TURKEY_TAKEOVER_ROOM;				
			}
			break;
		case 12:
			//Validate the zone is loaded.
			if(real_zone(CHRISTMAS_VILLAGE_ZONE) != -1){
				teleport_room = CHRISTMAS_VILLAGE_ROOM;				
			}          
			break;
      }
		
	  //If there are no linked zones, dont do anything.
      if (teleport_room == NOWHERE){
		  act("The runegate dimly glow and fade back to darkness.", TRUE, ch, 0, 0, TO_ROOM);
		  
		  return FALSE;		  
	  }

      for (CHAR *tel_ch = world[CHAR_REAL_ROOM(ch)].people, *tel_next = NULL; tel_ch; tel_ch = tel_next) {
        tel_next = tel_ch->next_in_room;

        if (IS_NPC(tel_ch)) continue;

        char_from_room(tel_ch);
        char_to_room(tel_ch, real_room(teleport_room));

        do_look(tel_ch, "\0", CMD_LOOK);
      }   
   
    return TRUE;
  }

  return FALSE;
}


/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */

//Blackmarket NPC for Subclass Tokens
int blackmarket_guise(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    // Stores the random people to talk to.
    CHAR *vict;
    // Random Actions to be performed.
    // char *mob_social_actions[4] = { "nudge ", "cough ", "cod ", "highfive " };
    char *cough = "cough";
    char *nudge = "nudge";
    OBJ *reward_object;

    /*
    Random Item List


    30017 - Easy Recipe for Rusty Spoon
    30018 - Plain Recipe for Ubertank Plate
    30019 - Uninvolved Recipe for Crash-Test Helmet
    30020 - Straightforward Recipe for Regenerator
    30021 - Piece of Cake Recipe for Can
    30022 - Basic Repice for Casters Cloak
    30023 - Simple Recipe for Boots of Swiftness
    30027 - Tricky Recipe for Spoon
    30028 - Complicated Recipe for Ubertank Plate
    30029 - Difficult Recipe for Crash Test Helmet
    30030 - Ambitous Recipe for Regenerator
    30031 - Fancy Recipe for Can
    30032 - Hard Recipe for Casters Cloak
    30033 - Tough Recipe for Boots
    30034 - Challenging Recipe for Chopsticks
    30035 - Modest Recipe for Chopsticks
    */

    int random_item_rewards[16] = {30017, 30018, 30019, 30020, 30021, 30022, 30023, 30027, 30028, 30029, 30030, 30031, 30032, 30033, 30034, 30035};
    // char socialAct;
    int reward;

    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    // Have the vendor talk.
    if (cmd == MSG_TICK)
    {
        if (chance(50))
        {
            // Get a random victim to help add more life to the characters.
            vict = get_random_victim(mob);
            if (vict)
            {
                // socialAct = mob_social_actions[number(0, NUMELEMS(mob_social_actions) - 1)];
                // Get Random Social Acction

                // Get Victim Name and concat it.
                // str_cat(socialAct,10,GET_DISP_NAME(vict));

                // do_social(mob, socialAct, CMD_SOCIAL);
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
                // do_social(mob, cough, CMD_SOCIAL);
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

    // If threatened with violence, put them at 1 mana and hp and move to another roon.
    if (cmd == MSG_VIOLENCE)
    {
        do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

        act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

        for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
        {
            next_vict = vict->next_in_room;

            /*Only teleport mortals. */
            if (!IS_MORTAL(vict))
                continue;

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

    // Main spec for giving a SC Token
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
        // OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

        if (!obj)
            return TRUE;

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

        // Remove the Token
        extract_obj(obj);

        // Roll Die.
        do_say(mob, "Lets see what you win this time.", CMD_SAY);
        act("$n pulls out a pair of bone die and throws them on the ground.", TRUE, mob, 0, 0, TO_ROOM);

        switch (number(0, 6))
        {
        // Award Random AQP (1-3 AQP)
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
        // Random Anti Rent Recipe
        case 1:
            do_say(mob, "Enjoy this item, courtesy of us.", CMD_SAY);

            reward = random_item_rewards[number(0, NUMELEMS(random_item_rewards) - 1)];

            // Read the Item
            reward_object = read_object(reward, VIRTUAL);
            // Give Object to Char.
            obj_to_char(reward_object, ch);
            act("$N reaches into a box and gives you $p.", FALSE, ch, reward_object, mob, TO_CHAR);
            break;

        // Random Anti Rent Recipe
        case 2:
            do_say(mob, "Enjoy this item, courtesy of us.", CMD_SAY);

            reward = random_item_rewards[number(0, NUMELEMS(random_item_rewards) - 1)];

            // Read the Item
            reward_object = read_object(reward, VIRTUAL);
            // Give Object to Char.
            obj_to_char(reward_object, ch);
            act("$N reaches into a box and gives you $p.", FALSE, ch, reward_object, mob, TO_CHAR);
            break;
        // Random Anti Rent Recipe
        case 3:
            do_say(mob, "Enjoy this item, courtesy of us.", CMD_SAY);

            reward = random_item_rewards[number(0, NUMELEMS(random_item_rewards) - 1)];

            // Read the Item
            reward_object = read_object(reward, VIRTUAL);
            // Give Object to Char.
            obj_to_char(reward_object, ch);
            act("$N reaches into a box and gives you $p.", FALSE, ch, reward_object, mob, TO_CHAR);
            break;
        // Random Anti Rent Recipe
        case 4:
            do_say(mob, "Enjoy this item, courtesy of us.", CMD_SAY);

            reward = random_item_rewards[number(0, NUMELEMS(random_item_rewards) - 1)];

            // Read the Item
            reward_object = read_object(reward, VIRTUAL);
            // Give Object to Char.
            obj_to_char(reward_object, ch);
            act("$N reaches into a box and gives you $p.", FALSE, ch, reward_object, mob, TO_CHAR);
            break;
        // Random Anti Rent Recipe
        case 5:
            do_say(mob, "Enjoy this item, courtesy of us.", CMD_SAY);

            reward = random_item_rewards[number(0, NUMELEMS(random_item_rewards) - 1)];

            // Read the Item
            reward_object = read_object(reward, VIRTUAL);
            // Give Object to Char.
            obj_to_char(reward_object, ch);
            act("$N reaches into a box and gives you $p.", FALSE, ch, reward_object, mob, TO_CHAR);

            break;
            // 1.1 mil gold Gold Reward
        case 6:
            do_say(mob, "Gold rules this world.", CMD_SAY);
            GET_GOLD(ch) = GET_GOLD(ch) + 1100000;
            break;
        default:
            break;
        }

        return TRUE;
    }

    return FALSE;
}

//Event Token Mob
int blackmarket_spector(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    // Have the vendor talk.
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

    // If threatened with violence, put them at 1 mana and hp and move to another roon.
    if (cmd == MSG_VIOLENCE)
    {
        do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

        act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

        for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
        {
            next_vict = vict->next_in_room;

            /*Only teleport mortals. */
            if (!IS_MORTAL(vict))
                continue;

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
        // OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

        if (!obj)
            return TRUE;

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
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    // Have the vendor talk.
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

    // If threatened with violence, put them at 1 mana and hp and move to another roon.
    if (cmd == MSG_VIOLENCE)
    {
        do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

        act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

        for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
        {
            next_vict = vict->next_in_room;

            /*Only teleport mortals. */
            if (!IS_MORTAL(vict))
                continue;

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
        // OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

        if (!obj)
            return TRUE;

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
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    // Have the vendor talk.
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

    // If threatened with violence, put them at 1 mana and hp and move to another roon.
    if (cmd == MSG_VIOLENCE)
    {
        do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

        act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

        for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
        {
            next_vict = vict->next_in_room;

            /*Only teleport mortals. */
            if (!IS_MORTAL(vict))
                continue;

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
        // OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

        if (!obj)
            return TRUE;

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
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    // Have the vendor talk.
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

    // If threatened with violence, put them at 1 mana and hp and move to another roon.
    if (cmd == MSG_VIOLENCE)
    {
        do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

        act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

        for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
        {
            next_vict = vict->next_in_room;

            /*Only teleport mortals. */
            if (!IS_MORTAL(vict))
                continue;

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
        // OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

        if (!obj)
            return TRUE;

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
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    // Have the vendor talk.
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

    // If threatened with violence, put them at 1 mana and hp and move to another roon.
    if (cmd == MSG_VIOLENCE)
    {
        do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

        act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

        for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
        {
            next_vict = vict->next_in_room;

            /*Only teleport mortals. */
            if (!IS_MORTAL(vict))
                continue;

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
        // OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

        if (!obj)
            return TRUE;

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

int blackmarket_questgiver(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    CHAR *questgiver_dummy;
    int questgiver_dummy_nr = 0;
    int dummy_int;

    char buf[MIL];
    bool give_back = FALSE;
    /*Don't waste any more CPU time if no one is in the room. */
    // if (count_mortals_room(mob, TRUE) < 1) return FALSE;

    // Have the vendor talk.
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

    // If threatened with violence, put them at 1 mana and hp and move to another roon.
    if (cmd == MSG_VIOLENCE)
    {
        do_say(mob, "Violence is not necessary...though you wouldnt win anyways.", CMD_SAY);

        act("$n draws a dagger and advances on the room.", TRUE, mob, 0, 0, TO_ROOM);

        for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
        {
            next_vict = vict->next_in_room;

            /*Only teleport mortals. */
            if (!IS_MORTAL(vict))
                continue;

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
        // OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

        if (!obj)
            return TRUE;

        if (OBJ_TYPE(obj) == ITEM_SC_TOKEN)
        {
            do_say(mob, "Go see Guise.. he loves these things.", CMD_SAY);
            give_back = TRUE;
        }

        // Add a new check here for the Game Tokens.
        // TODO:  Set Bank Value corresponding to the Game.   STATE1 - STATE11
        // IF State is set, set give_back
        // If Not, load corresponding dummy to "play" the game.   They will receive the cards/quest objects.
        // A Dummy is used to split the specs and make it so you can have multiple quests going if you want.

        else if ((V_OBJ(obj) >= QUESTGIVER_TOKEN_MIN_VNUM && V_OBJ(obj) <= QUESTGIVER_TOKEN_MAX_VNUM))
        {

            if (V_OBJ(obj) == QUESTGIVER_TOKEN_HIGH_CARD)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_HIGH_CARD;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_ONE)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_ONE;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_TWO)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_TWO;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_THREE)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_THREE;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_FOUR)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_FOUR;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_FIVE)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_FIVE;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_SIX)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_SIX;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_SEVEN)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_SEVEN;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_EIGHT)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_EIGHT;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_NINE)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_NINE;
            }
            else if (V_OBJ(obj) == QUESTGIVER_TOKEN_QUEST_TEN)
            {
                questgiver_dummy_nr = QUESTGIVER_DUMMY_QUEST_TEN;
            }
            else
            {

                do_say(mob, "I don't recognize this token", CMD_SAY);
                give_back = TRUE;
            }

            // If we have have a good token, find the mob and load it into the room

            if (!IS_SET(GET_BANK(mob), STATE1))
            {
                if (questgiver_dummy_nr != 0)
                {
                    dummy_int = real_mobile(questgiver_dummy_nr);
                    questgiver_dummy = read_mobile(dummy_int, REAL);
                    char_to_room(questgiver_dummy, CHAR_REAL_ROOM(mob));
                    act("$n makes an odd gesture and summons a robot.", 0, mob, 0, 0, TO_ROOM);
                    act("$n appears out of nowhere.", 0, questgiver_dummy, 0, 0, TO_ROOM);
                    // The Bank will Track the Game.   1 Game at a time.
                    SET_BIT(GET_BANK(mob), STATE1);
                }
            }
            else
            {
                do_say(mob, "I apologize, but a game is already underway.", CMD_SAY);
                give_back = TRUE;
            }

            // We Summoned the Dummy.   Lets Play the Game.
        }
        else
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
    }

    if (cmd == CMD_LIST)
    {

        if (!ch || IS_NPC(ch))
            return FALSE;

        /* if (IS_IMMORTAL(ch)) {
          send_to_char("Immortals will need to consult with an IMP in order to remort.\n\r", ch);

          return TRUE;
        }*/

        send_to_char("You can buy:\n\r", ch);
        int count = 0;
        // Define Tokens for Games.   It will only be 1 Token to start.
        for (int vnum = QUESTGIVER_TOKEN_MIN_VNUM; vnum <= QUESTGIVER_TOKEN_MAX_VNUM; vnum++)
        {
            OBJ *token = read_object(vnum, VIRTUAL);

            if (!token)
            {
                snprintf(buf, sizeof(buf), "WIZINFO: Error loading object (#%d) in blackmarket_questgiver(), cmd == CMD_LIST.", vnum);

                wizlog(buf, LEVEL_SUP, 5);
                log_f("%s", buf);

                return FALSE;
            }

            count++;

            printf_to_char(ch, "%s\n\r", OBJ_SHORT(token));

            extract_obj(token);
        }

        if (!count)
        {
            send_to_char("Nothing.\n\r", ch);
        }

        return TRUE;
    }

    // Once a token has been purchased - they can say play to start the game.
    if (cmd == CMD_UNKNOWN)
    {
        if (!ch || IS_NPC(ch))
            return FALSE;
    }

    /* Buying tokens is handled as a spec due to shopkeeper limitations and a few other requirements. */
    if (cmd == CMD_BUY)
    {
        if (!ch || IS_NPC(ch))
            return FALSE;

        one_argument(arg, buf);

        int vnum = -1;

        if (!str_cmp(buf, "high"))
            vnum = QUESTGIVER_TOKEN_HIGH_CARD;
        else if (!str_cmp(buf, "one"))
            vnum = QUESTGIVER_TOKEN_QUEST_ONE;
        else if (!str_cmp(buf, "two"))
            vnum = QUESTGIVER_TOKEN_QUEST_TWO;
        else if (!str_cmp(buf, "three"))
            vnum = QUESTGIVER_TOKEN_QUEST_THREE;
        else if (!str_cmp(buf, "four"))
            vnum = QUESTGIVER_TOKEN_QUEST_FOUR;
        else if (!str_cmp(buf, "five"))
            vnum = QUESTGIVER_TOKEN_QUEST_FIVE;
        else if (!str_cmp(buf, "six"))
            vnum = QUESTGIVER_TOKEN_QUEST_SIX;
        else if (!str_cmp(buf, "seven"))
            vnum = QUESTGIVER_TOKEN_QUEST_SEVEN;
        else if (!str_cmp(buf, "eight"))
            vnum = QUESTGIVER_TOKEN_QUEST_EIGHT;
        else if (!str_cmp(buf, "nine"))
            vnum = QUESTGIVER_TOKEN_QUEST_NINE;
        else if (!str_cmp(buf, "ten"))
            vnum = QUESTGIVER_TOKEN_QUEST_TEN;
        else
        {

            send_to_char("Questgiver states, 'I am not familiar with that game. '\n\r", ch);

            return TRUE;
        }

        /* Ensure that a character can't buy more than one of each type of token. */
        bool deny = FALSE;

        for (OBJ *tmp_obj = GET_CARRYING(ch); tmp_obj && !deny; tmp_obj = OBJ_NEXT_CONTENT(tmp_obj))
        {
            if (V_OBJ(tmp_obj) < QUESTGIVER_TOKEN_MIN_VNUM || V_OBJ(tmp_obj) > QUESTGIVER_TOKEN_MAX_VNUM)
                continue;

            if (((vnum >= QUESTGIVER_TOKEN_HIGH_CARD && vnum <= QUESTGIVER_TOKEN_QUEST_TEN) && (V_OBJ(tmp_obj) >= QUESTGIVER_TOKEN_HIGH_CARD && V_OBJ(tmp_obj) <= QUESTGIVER_TOKEN_QUEST_TEN)))
            {
                deny = TRUE;
            }
        }

        if (deny)
        {
            send_to_char("Questgiver states, 'You are only allowed to buy a Single Game Token.\n\r", ch);

            return TRUE;
        }

        /* Load up the token and give it the character. */
        OBJ *token = read_object(vnum, VIRTUAL);

        if (!token)
        {
            snprintf(buf, sizeof(buf), "WIZINFO: Error loading object (#%d) in rv2_mob_spec_immortalis(), cmd == CMD_BUY.", vnum);

            wizlog(buf, LEVEL_SUP, 5);
            log_f("%s", buf);

            return TRUE;
        }

        /* Set the token's Value 0 to the unique ID of the character a safety precaution, and set its timer to 10. */
        OBJ_VALUE0(token) = GET_ID(ch);
        OBJ_TIMER(token) = 10;

        obj_to_char(token, ch);
        send_to_char("Here if your token. Please enjoy your game.\n\r", ch);

        // The user has purchased a game token.  GIve them the next instructions.
        // TODO - Why cant i read the info?
        /* Find out what tokens the character has now. */
        /*
        token_info = token->ex_description.description;

        if(token_info)
        {
            printf_to_char(ch, "Here is the game. \n\r %s.\n\r",token_info);
            send_to_char("Questgiver states, 'Once you are ready, give me the token and issue the command `jstart game`q.\n\r", ch);
        }else
        {
            send_to_char("Let me get back to you.\n\r", ch);
        }



          */

        return TRUE;
    }

    return FALSE;
}

// The Goblins will drop random tradebars for extra money.
// You can pass additional chance_adjust to increase the odds.
/*

        40% [3013 ] a bronze tradebar -- 50k
        30% [3014 ] a silver tradebar -- 100k
        20% [3015 ] a gold tradebar -- 200k
        5%  [3016 ] a platinum tradebar -- 500k
        3%  [3017 ] a mithril tradebar -- 1 Mil
        2%  [3018 ] a large diamond -- 5 Mil

//Boss Goblin will drop  [3146 ] a palladium tradebar  if 10+ people fight it., else it will just drop 3 diamonds when killed.
*/
int determine_goblin_loot(int chance_adjust)
{

    int initial_number = number(0, 100);
    int adjusted_number = initial_number + chance_adjust;

    int reward;
    // 40% chance for Bronze Tradebar
    if (adjusted_number <= 40)
    {
        reward = 3013; // 40% [3013 ] a bronze tradebar -- 50k
    }

    else if (adjusted_number <= 70)
    {
        reward = 3014; // 30% [3014 ] a silver tradebar -- 100k
    }
    else if (adjusted_number <= 90)
    {
        reward = 3015; // 20% [3015 ] a gold tradebar -- 200k
    }
    else if (adjusted_number <= 95)
    {
        reward = 3016; // 5%  [3016 ] a platinum tradebar 500k
    }
    else if (adjusted_number <= 98)
    {
        reward = 3017; // 3%  [3017 ] a mithril tradebar -- 1 Mil
    }
    else
    {
        reward = 3018; // 2%  [3018 ] a large diamond -- 5 Mil
    }

    return reward;
}

void loot_goblin_coin_throw(CHAR *ch, CHAR *vict, int max_gold)
{
    int gold_number, gold_damage;
    OBJ *tmp;

    // Calculate the Max Number of Gold

    // Hit the Victim with that gold (Damage is Gold/100)
    if (max_gold <= 10000)
    {
        gold_number = number(10000, 20000);
    }
    else
    {
        gold_number = number(10000, max_gold);
    }

    gold_damage = (int)round(gold_number / 100);

    act("$n grabs a fistful of coins and throws them at you.", FALSE, ch, 0, vict, TO_VICT);
    act("$n grabs a fistful of coins and throws them at $N.", FALSE, ch, 0, vict, TO_NOTVICT);

    if (GET_OPPONENT(ch) && GET_OPPONENT(ch) == vict)
    {
        damage(ch, vict, gold_damage, TYPE_UNDEFINED, DAM_PHYSICAL);
        WAIT_STATE(vict, 1 * PULSE_VIOLENCE);
    }

    // Spawn that same amount of money on the ground.

    tmp = read_object(LOOT_GOBLIN_GOLD, VIRTUAL);
    tmp->obj_flags.value[0] = gold_number;
    obj_to_room(tmp, CHAR_REAL_ROOM(vict));
}

// See if all the loot goblins are dead.  If they are - on death, spawn the 4 lieutenants.
// Return TRUE if all dead.
bool check_for_loot_goblins()
{

    bool allDead = FALSE;
    int loot_goblin_number = 18023;
    int loot_goblin_nr;

    loot_goblin_nr = real_mobile(loot_goblin_number);

    if (mob_proto_table[loot_goblin_nr].number < 1)
    {
        allDead = TRUE;
    }

    return allDead;
}

// 2nd Check to see if the 4 Lieutenants are dead.
int check_for_loot_goblin_officers()
{

    int loot_goblins_left = 0;

    int loot_goblin_officers[4] = {18018, 18019, 18020, 18021};
    int loot_goblin_number, loot_goblin_nr;

    for (int i = 0; i < 4; i++)
    {
        // Grab the room and mob based on index value.
        loot_goblin_number = loot_goblin_officers[i];

        // Check if the room exists
        if (!loot_goblin_number)
            continue;

        loot_goblin_nr = real_mobile(loot_goblin_number);

        if (mob_proto_table[loot_goblin_nr].number > 0)
        {
            loot_goblins_left++;
        }
    }

    return loot_goblins_left;
}

// Use the same code of Teleport to spawn 30 goblins into the world.
void spawn_loot_goblns(int max_goblins)
{

    int to_room, loot_goblin_nr;
    int loot_goblin_number = 18023;
    CHAR *loot_goblin;

    loot_goblin_nr = real_mobile(loot_goblin_number);

    for (int i = 0; i < (max_goblins); i++)
    {

        do
        {
            to_room = number(0, top_of_world);
        } while (IS_SET(world[to_room].room_flags, PRIVATE) ||
                 IS_SET(world[to_room].room_flags, NO_MAGIC) ||
                 (IS_SET(world[to_room].room_flags, TUNNEL) && !CHAOSMODE) ||
                 IS_SET(world[to_room].room_flags, CHAOTIC) ||
                 IS_SET(world[to_room].room_flags, NO_BEAM) ||
                 IS_SET(world[to_room].room_flags, SAFE) ||
                 IS_SET(world[to_room].room_flags, DEATH) ||
                 IS_SET(world[to_room].room_flags, HAZARD) ||
                 IS_SET(world[to_room].room_flags, NO_MOB) ||
                 IS_SET(world[to_room].room_flags, LOCK) ||
                 real_room(to_room) == NOWHERE);

        if (!to_room)
            continue;
        if (!loot_goblin_nr)
            continue;
        if (real_room(to_room) == NOWHERE)
            continue;

        loot_goblin = read_mobile(loot_goblin_nr, REAL);
        char_to_room(loot_goblin, real_room(to_room));
    }
}

/*Spawn the 4 Officers of the loot goblins.

// They spawn in caves.
//LEts give 8 places to spawn them
//Pick 4

// 6298 - Cave Bear Cave
// 1938 - Orak Cave
// 19420 - DyingWorld ECanyon
// 10438 - Quicklings - Treasure Room.
// 21130 - Deaths Playground Cave
// 970 - Storage Caves - Zan Kingdom
// 4561 - Marsh Cave - Goblins
// 7505 - Treasure Cabin - Pirates

//The 4 Officers
[18018] Gemfang Goblin (goblin gemfang) - Kender
[18019] Gildpurse Goblin (goblin gildpurse) - Kender
[18020] Packmaster Goblin (goblin packmaster) - Kender
[18021] Mirage Goblin (goblin mirage) - Kender

//Test Load Z Commands

load z ThePirateShip
load z QuicklandrevampbyCBLeha
load z DeathsPlayground
load z TheZanKingdombyMitch
load z MedievalBlackMarket
load z ElementalCanyonDyingWorld
load z HelventiaMountainsbyLem
load z TheLands
load z GoblinKingdomandMarshCaveb


*/
void spawn_loot_goblin_officers()
{

    int spawn_rooms[8] = {6298, 1938, 19420, 10438, 21130, 970, 4561, 7505};
    int loot_goblin_officers[4] = {18018, 18019, 18020, 18021};

    int loot_goblin_number;
    int loot_goblin_nr;
    int room_number;

    CHAR *loot_goblin;

    // Shuffle the spawn_rooms array

    // Shuffle the indices using Fisher-Yates shuffle
    for (int i = NUMELEMS(spawn_rooms) - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = spawn_rooms[i];
        spawn_rooms[i] = spawn_rooms[j];
        spawn_rooms[j] = temp;
    }

    for (int i = 0; i < 4; i++)
    {
        // Grab the room and mob based on index value.
        loot_goblin_number = loot_goblin_officers[i];
        room_number = spawn_rooms[i];

        // Check if the room exists
        if (!real_room(room_number))
            continue;
        if (!loot_goblin_number)
            continue;
        if (real_room(room_number) == NOWHERE)
            continue;

        loot_goblin_nr = real_mobile(loot_goblin_number);

        if (mob_proto_table[loot_goblin_nr].number < 1)
        {

            loot_goblin = read_mobile(loot_goblin_nr, REAL);
            char_to_room(loot_goblin, real_room(room_number));
        }
    }
}

void spawn_loot_goblin_leader()
{

    int spawn_rooms[8] = {6298, 1938, 19420, 10438, 21130, 970, 4561, 7505};
    int leader_spawn_room;
    int loot_goblin_number = 18022;
    int loot_goblin_nr;
    CHAR *loot_goblin;

    leader_spawn_room = spawn_rooms[number(0, NUMELEMS(spawn_rooms) - 1)];

    if (!real_room(leader_spawn_room))
        return;
    if (real_room(leader_spawn_room) == NOWHERE)
        return;
    loot_goblin_nr = real_mobile(loot_goblin_number);

    if (mob_proto_table[loot_goblin_nr].number < 1)
    {

        loot_goblin = read_mobile(loot_goblin_nr, REAL);
        char_to_room(loot_goblin, real_room(leader_spawn_room));
    }
}

int blackmarket_loot_goblin(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    int goblin_loot_object;
    OBJ *goblin_loot, *tmp;
    ;
    CHAR *vict, *next_vict;

    int loot_goblin_number = 18023;
    int loot_goblin_boss_number = 18022;
    int loot_goblin_nr;

    int mortal_count = 0;
    int boss_treasure_death_item, loot_mult;

    switch (cmd)
    {
    case MSG_MOBACT:
        // If they are fighting, give a chance to drop a tradebar for extra money.

        if (mob->specials.fighting)
        {
            vict = get_random_victim_fighting(mob);

            // Add checks for the different goblins. ADjust their ranges
            if (V_MOB(mob) == loot_goblin_number)
            {
                loot_goblin_coin_throw(mob, vict, 10000);
            }
            else if (V_MOB(mob) == loot_goblin_boss_number)
            {
                loot_goblin_coin_throw(mob, vict, 100000);
            }
            else //THis accounts for the 4 different officers
            {
                loot_goblin_coin_throw(mob, vict, 60000);
            }

            if (chance(80))
            {

                goblin_loot_object = determine_goblin_loot(0);
                goblin_loot = read_object(goblin_loot_object, VIRTUAL);
                obj_to_room(goblin_loot, CHAR_REAL_ROOM(mob));
                act("A glittery object drops from the Goblin's bag.", TRUE, mob, 0, 0, TO_ROOM);
                act("$n cackles in glee and disapears.", TRUE, mob, 0, 0, TO_ROOM);
                spell_teleport(50, mob, mob, 0);
            }
        }
        else
        {
            if (chance(10))
            {

                act("$n cackles in glee and disapears.", TRUE, mob, 0, 0, TO_ROOM);
                spell_teleport(50, mob, mob, 0);
            }
        }

        break;

    case MSG_ENTER:
        // If NPCs or they are already fighting, dont do anything else.
        if (IS_NPC(ch) || IS_IMMORTAL(ch) || mob->specials.fighting)
            return FALSE;

        // 10% chance to teleport away as a MORT moves into the room

        if (chance(10))
        {

            act("$n cackles in glee and disapears.", TRUE, mob, 0, 0, TO_ROOM);
            spell_teleport(50, mob, mob, 0);
        }

        break;

    case MSG_DIE:

        // Checks for Loot Goblins
        loot_goblin_nr = real_mobile(loot_goblin_number);

        if (mob_proto_table[loot_goblin_nr].number > 1 && V_MOB(mob) == loot_goblin_number)
        {
            act("You will never be able to catch us all.'", TRUE, mob, 0, 0, TO_ROOM);
        }
        else if (mob_proto_table[loot_goblin_nr].number == 1 && V_MOB(mob) == loot_goblin_number)
        {
            act("I may be the last, but I shall be avenged.", TRUE, mob, 0, 0, TO_ROOM);
        }

        // Checks for Loot Goblin Boss
        loot_goblin_nr = real_mobile(loot_goblin_boss_number);
        if (mob_proto_table[loot_goblin_nr].number == 1 && V_MOB(mob) == loot_goblin_boss_number)
        {
            act("NOOOO, not my Treasure!!!", TRUE, mob, 0, 0, TO_ROOM);

            // If there are 5 people in a group - Drop 1-2 Diamonds
            // If there are between 5-10, drop 5-8 Diamonds
            // If more than 10 - Drop Random 1 Palladium Bar
            // If more than 15 - Drop Random 2-3 Palladium

            for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
            {
                next_vict = vict->next_in_room;
                if (IS_MORTAL(vict))
                {
                    mortal_count++;
                }
            }

            if (mortal_count > 15)
            {
                boss_treasure_death_item = 3146;
                loot_mult = number(2, 3);
            }
            else if (mortal_count > 10)
            {
                boss_treasure_death_item = 3146;
                loot_mult = 1;
            }
            else if (mortal_count > 5)
            {
                boss_treasure_death_item = 3018;
                loot_mult = number(5, 8);
            }
            else
            {
                boss_treasure_death_item = 3018;
                loot_mult = 1;
            }

            for (int i = 0; i < loot_mult; i++)
            {
                tmp = read_object(boss_treasure_death_item, VIRTUAL);
                obj_to_room(tmp, CHAR_REAL_ROOM(mob));
            }
        }

        break;
    }
    return FALSE;
}

int blackmarket_grintak_hunter(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    // char buf2[MIL];
    char buf[MAX_STRING_LENGTH];
    CHAR *vict;

    bool allDead = FALSE;
    int loot_goblins_left;
    int loot_goblin_number = 18023;
    int loot_goblin_nr;
    int max_loot_goblins = 30;

    switch (cmd)
    {
    case MSG_TICK:
        // Track this until this part of the quest is complete. THen ensure it doesnt fire again.
        if (!IS_SET(GET_BANK(mob), STATE1) && IS_SET(GET_BANK(mob), STATE4))
        {
            allDead = check_for_loot_goblins();

            // If all are dead, call the function to spawn the next 4.
            if (allDead)
            {
                sprintf(buf, "The Goblin Officers have appeared, you must kill them.");
                do_quest(mob, buf, CMD_QUEST);
                spawn_loot_goblin_officers();
                SET_BIT(GET_BANK(mob), STATE1);
            }
            // If all of them arent dead, lets provide updates.
            else
            {
                loot_goblin_nr = real_mobile(loot_goblin_number);
                loot_goblins_left = mob_proto_table[loot_goblin_nr].number;

                // Only speak 60% of the time.
                if (chance(60))
                {

                    if (loot_goblins_left == 1)
                    {
                        sprintf(buf, "Only one of these traitor's remain. Hunt them down!");
                    }
                    else if (loot_goblins_left == 15)
                    {
                        sprintf(buf, "Half of the traitors have been killed. Continue hunting them down.");
                    }
                    else if (loot_goblins_left == 30)
                    {
                        sprintf(buf, "Those traitor goblins are out there with the loot. Kill them!");
                    }
                    else if (loot_goblins_left > 0 && loot_goblins_left < 8)
                    {
                        sprintf(buf, "Only a quarter of their ranks remain. Continue the hunt!");
                    }
                    else if (loot_goblins_left > 22 && loot_goblins_left < 30)
                    {
                        sprintf(buf, "A few of the Goblins have been slain.  Keep up the good work");
                    }
                    else if (loot_goblins_left > 8 && loot_goblins_left < 15)
                    {
                        sprintf(buf, "Less than half of them remain, kill them all!");
                    }
                    else
                    {
                        sprintf(buf, "Find the loot goblins and kill them!");
                    }

                    do_quest(mob, buf, CMD_QUEST);
                }
            }
        }
        // Branch for Loot Goblin Officers.   STATE1 has to be set, and STATE2 is NOT.
        // There will be 4 Officers in the world.
        if (!IS_SET(GET_BANK(mob), STATE2) && IS_SET(GET_BANK(mob), STATE1) && IS_SET(GET_BANK(mob), STATE4))
        {
            loot_goblins_left = check_for_loot_goblin_officers();

            // If all are dead, call the function to spawn the next 4.
            if (loot_goblins_left == 0)
            {
                sprintf(buf, "The Goblin Leader have appeared, you need to destroy him.");
                do_quest(mob, buf, CMD_QUEST);
                spawn_loot_goblin_leader();
                SET_BIT(GET_BANK(mob), STATE2);
            }
            // If all of them arent dead, lets provide updates.
            else
            {

                // Only speak 60% of the time.
                if (chance(60))
                {

                    if (loot_goblins_left == 1)
                    {
                        sprintf(buf, "Find the final one and destroy them.");
                    }
                    else if (loot_goblins_left == 2)
                    {
                        sprintf(buf, "Two Down, Two to go!");
                    }
                    else if (loot_goblins_left == 3)
                    {
                        sprintf(buf, "One of those officers has met their fate.");
                    }
                    else if (loot_goblins_left == 4)
                    {
                        sprintf(buf, "Find the officers and kill them!");
                    }

                    do_quest(mob, buf, CMD_QUEST);
                }
            }
        }
        // Only the Boss Goblin Remains.

        if (IS_SET(GET_BANK(mob), STATE2) && IS_SET(GET_BANK(mob), STATE1) && IS_SET(GET_BANK(mob), STATE4))
        {

            loot_goblin_number = 18022;
            loot_goblin_nr = real_mobile(loot_goblin_number);
            loot_goblins_left = mob_proto_table[loot_goblin_nr].number;

            // If all are dead, call the function to spawn the next 4.
            if (loot_goblins_left == 0 && !IS_SET(GET_BANK(mob), STATE3))
            {
                sprintf(buf, "The Goblin Leader has been killed. Thank you for ridding the world of this scourge.");
                do_quest(mob, buf, CMD_QUEST);
                SET_BIT(GET_BANK(mob), STATE3);
            }
            // If all of them arent dead, lets provide updates.
            else
            {

                // Only speak 60% of the time.
                if (chance(60) && !IS_SET(GET_BANK(mob), STATE3))
                {

                    if (loot_goblins_left == 1)
                    {
                        sprintf(buf, "Find the final one and destroy them. They don't deserve treasure. They deserve DEATH!");
                    }
                    else
                    {
                        sprintf(buf, "Those Pesky IMMORTS have spawned additional Bosses to kill!");
                    }

                    do_quest(mob, buf, CMD_QUEST);
                }
            }
        }

        break;

    case CMD_KILL:
    case CMD_HIT:
    case CMD_KICK:
    case CMD_AMBUSH:
    case CMD_ASSAULT:
    case CMD_BACKSTAB:
        do_say(mob, "Stop That. Go kill the Loot Goblins.", CMD_SAY);
        return TRUE;

    case MSG_VIOLENCE:

        for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room)
            if (vict->specials.fighting == mob)
                stop_fighting(vict->specials.fighting);
        stop_fighting(mob);
        GET_HIT(mob) = GET_MAX_HIT(mob);

        do_say(mob, "Stop That. Go kill the Loot Goblins.", CMD_SAY);

        return FALSE;

    case MSG_OBJ_GIVEN:

        arg = one_argument(arg, buf);

        OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);
        // OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

        if (!obj)
            return TRUE;

        do_say(mob, "I dont want this junk..take it back.", CMD_SAY);

        bool give_back = TRUE;

        if (give_back)
        {
            act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);
            obj_from_char(obj);
            obj_to_char(obj, ch);

            return TRUE;
        }

        break;
    case CMD_UNKNOWN:
        // Set a bit to ensure you can only start the quest once.

        arg = one_argument(arg, buf);
        if (*buf && is_abbrev(buf, "start"))
        {
            if (!IS_IMMORTAL(ch))
            {
                do_say(mob, "Go find an IMMORT!", CMD_SAY);
                return TRUE;
            }

            if (!IS_SET(GET_BANK(mob), STATE4))
            {
                spawn_loot_goblns(max_loot_goblins);
                send_to_char("Starting Loot Goblin Quest.\n\r", ch);
                SET_BIT(GET_BANK(mob), STATE4);
                sprintf(buf, "Loot Goblins have stolen my treasure. Go get it back!");
                do_quest(mob, buf, CMD_QUEST);
                return TRUE;
            }
            else
            {
                do_say(mob, "You can't start this again", CMD_SAY);
                return TRUE;
            }
        }
        else
        {
            return FALSE;
        }

        break;

    default:
        break;
    }
    return FALSE;
}

// Assign Spec for the zone. Sets all other specs.
// First Param - Object, Room or Mob Number.  Define it up above.
// Second Param - the name of the function that is for the mob Usually a shorthand for your zone.
void assign_medievalblackmarket(void)
{
    /*Objects */
    // assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);

    /*Rooms */
    assign_room(RUNEGATE, blackmarket_runegate);

    /*Mobs */

    assign_mob(GUISE_MERCHANT, blackmarket_guise);
    assign_mob(SPECTOR_MERCHANT, blackmarket_spector);
    assign_mob(ECHO_MERCHANT, blackmarket_echo);
    assign_mob(MIME_MERCHANT, blackmarket_mime);
    assign_mob(FUSE_MERCHANT, blackmarket_fuse);
    assign_mob(FEIGN_MERCHANT, blackmarket_feign);

    assign_mob(QUESTGIVER, blackmarket_questgiver);

    assign_mob(LOOT_GOBLIN_OFFICER_ONE, blackmarket_loot_goblin);
    assign_mob(LOOT_GOBLIN_OFFICER_TWO, blackmarket_loot_goblin);
    assign_mob(LOOT_GOBLIN_OFFICER_THREE, blackmarket_loot_goblin);
    assign_mob(LOOT_GOBLIN_OFFICER_FOUR, blackmarket_loot_goblin);
    assign_mob(LOOT_GOBLIN_BOSS, blackmarket_loot_goblin);
    assign_mob(LOOT_GOBLIN, blackmarket_loot_goblin);
    assign_mob(GRINTAK_HUNTER, blackmarket_grintak_hunter);
}