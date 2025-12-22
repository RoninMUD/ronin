/* Specs for area ChristmasVillage by Arodtanjoe

** Written by Arodtanjoe - Feb 2025
*/


/*System Includes */
#include <string.h>
#include <stdlib.h>

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
#include <stdio.h>
#include <time.h>

/*Rooms */

#define SORTING_HALL 26371


//Event Summon Rooms
#define WISHBOUND_SPIRIT_ROOM 26367
#define PARCEL_GOLEM_ROOM 26378
#define GUIDING_LIGHT_ROOM 26369
#define MERRYMAKER_JESTER_ROOM 26379
#define YULETIDE_COURIER_ROOM 26374
#define KRINGLES_PORTER_ROOM 26373
#define WRAPPING_WRAITH_ROOM 26368

/*Objects */


#define CHRISTMAS_AMULET 26341 //Amulet of Frozen Grace	26341
#define CHRISTMAS_MECHANICAL_FAMILIAR 26343  //Mechanical Gift Familiar	26343
#define CHRISTMAS_CHARM 26344 //Charm of the Snowbound Guardian	26344
#define CHRISTMAS_CHIME 26346 //Chime of Cheer	26346
#define CHRISTMAS_CROWN 26347 //Crown of Eternal Winter	26347

//Event Items

#define PRESENT_BOTTOM 26355
#define PRESENT_TOP 26361

#define WRAPPED_PRESENT 26355
#define SEALED_GIFTBOX 26356
#define TOPPED_PARCEL 26357
#define BUNDLE_CHEER 26358
#define SANTA_LETTER 26359
#define SANTA_GIFT 26360
#define FESTIVE_PARCEL 26361





/*Mobs */


#define WISHBOUND_SPIRIT 26317
#define PARCEL_GOLEM 26318
#define GUIDING_LIGHT 26319
#define MERRYMAKER_JESTER 26320
#define YULETIDE_COURIER 26321
#define KRINGLES_PORTER 26322
#define WRAPPING_WRAITH 26323
#define GIFTWARDEN 26324
#define MERRYMAKER_JESTER_CLONE 26325


/*Miscellaneous strings */
// Generic States that are shifted to indicate different stages.

#define CHRISTMAS_FAMILIAR_BLESSING_NAME "Christmas Joy"
#define CHRISTMAS_CHIME_BLESSING_NAME "Christmas Cheer"


#define STATE1 (1 << 0) // 1
#define STATE2 (1 << 1) // 2
#define STATE3 (1 << 2) // 4
#define STATE4 (1 << 3) // 8

/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */


// Function to check if the current date is between Dec 1 and Dec 30
bool check_is_christmas() {
    
	bool isChristmas = FALSE;
	
	time_t t;
    struct tm *current_time;

    // Get the current system time
    t = time(NULL);
    current_time = localtime(&t);

    int month = current_time->tm_mon + 1;  // tm_mon is 0–11, so add 1
    int day = current_time->tm_mday;       // tm_mday is 1–31

    // Check if it's between Dec 1 and Dec 30
    if (month == 12 && day >= 1 && day <= 31) {
        // Action to take during December 1–30
        isChristmas = TRUE;        
    } else {
		isChristmas = FALSE;
    }
	
	return isChristmas;
}


//Elixir of the Winter Heart	26342
//Miracle/Sanc/Endure

//Amulet of Frozen Grace	26341
int christmas_amulet(OBJ *obj, CHAR *ch, int cmd, char *arg) {

  if (ch) return FALSE; // MOBACT has a null signaler, so sanity check
  if (!obj || !obj->equipped_by) return FALSE;
  ch = obj->equipped_by;
  if (cmd != MSG_MOBACT) return FALSE;  

  // Roughly once every 45 minutes
  if (number(0,125) != 1) return FALSE;
  act("The amulet pulses and glows while covering $n in a warm light", 1, ch, 0, 0, TO_CHAR);  
  spell_satiate(50, ch, ch, 0);  

  return FALSE;
}


int christmas_mechanical_familiar_blessing_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg)
{

    int hp_gain;
    int mana_gain;
	
	bool isChristmas = check_is_christmas();

    if (cmd == MSG_REMOVE_ENCH)
    {
        send_to_char("Snowflakes envelop you and Christmas joy leaves your body.\n\r", ench_ch);
        return FALSE;
    }
    if (cmd == CMD_QUIT)
    {
        if (ch != ench_ch)
            return FALSE;
        send_to_char("The Joy of Christmas leaves your body.\n\r", ench_ch);
        return FALSE;
    }

    if (ench_ch && cmd == MSG_TICK)
    {
		
		if(isChristmas){
		
			hp_gain = number(100, 200);
			mana_gain = number(40, 80);
		}else {
			hp_gain = number(50, 100);
			mana_gain = number(20, 40);
		}

        GET_HIT(ench_ch) = GET_HIT(ench_ch) + hp_gain;
        GET_MANA(ench_ch) = GET_MANA(ench_ch) + mana_gain;
    }

    return FALSE;
}


//Mechanical Gift Familiar	26343

//Single use hold item.  Provides the blessing of christmas. 
int christmas_mechanical_familiar(OBJ *obj, CHAR *ch, int cmd, char *arg)
{

	CHAR *vict, *next_vict;
	char buf[MIL];
	CHAR *owner;
	
	if (cmd == CMD_USE)
    {
        /* Don't spec if no ch. */
        if (!ch)
            return FALSE;
        /* Don't spec if ch is not awake. */
        if (!AWAKE(ch))
            return FALSE;
        /* Don't spec if obj is not equipped by the actor. */
        if (!(owner = obj->equipped_by))
            return FALSE;
        /* Don't spec if actor is not the owner. */
        if (ch != owner)
            return FALSE;

        one_argument(arg, buf);

        /* Return if no target. */
        if (!*buf)
            return FALSE;
        
		//As long as it is equipped, grant the buff and disintigrate the familiar.
		if (*buf && is_abbrev(buf, "familiar")){
			
			act("The familiar jumps from your hand onto the ground.", FALSE, owner, 0, 0, TO_CHAR);
			act("The familiar jumps from $n's hand onto the ground.", FALSE, owner, 0, 0, TO_ROOM);
			act("The familiar jumps glows and spins scattering snow everywhere.", FALSE, owner, 0, 0, TO_ROOM);
		
			for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); vict; vict = next_vict)
			{
				next_vict = CHAR_NEXT_IN_ROOM(vict);

				if (IS_NPC(vict))
				{
					continue;
				}

				if (IS_MORTAL(vict))
				{
					enchantment_apply(ch, FALSE, CHRISTMAS_FAMILIAR_BLESSING_NAME, 0, 60, ENCH_INTERVAL_TICK, 5, APPLY_HITROLL, 0, 0, christmas_mechanical_familiar_blessing_func);
				}
			}
			act("The familiar breaks down and falls apart.", FALSE, owner, 0, 0, TO_CHAR);
			act("The familiar breaks down and falls apart.", FALSE, owner, 0, 0, TO_ROOM);
			extract_obj(obj);
			
			
			
			return TRUE;
		}
	}
	return FALSE;
		
}


//Charm of the Snowbound Guardian	26344
int christmas_charm (OBJ *obj,CHAR *ch,int cmd,char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int i,j;

  if(cmd==MSG_TICK) {
    if(obj->obj_flags.value[3]>0) obj->obj_flags.value[3]--;
    return FALSE;
  }

  if(cmd==MSG_OBJ_ENTERING_GAME) {
    if(is_number(arg)) i=atoi(arg);
    else return FALSE;
    j=obj->obj_flags.value[3]*60; /* Secs left to recharge */
    if(i>j) obj->obj_flags.value[3]=0;
    return FALSE;
  }

  if (cmd != CMD_USE) return FALSE;

  if (obj == EQ(ch,WEAR_WRIST_L) || obj == EQ(ch,WEAR_WRIST_R)) {
    one_argument (arg,buf);

    if (isname (buf,OBJ_NAME(obj))) {
      if (!obj->obj_flags.value[3]) {
        act("The Veil of Christmas descends on $n.",FALSE,ch,obj,0,TO_ROOM);        
		act("The Veil of Christmas descends on you.",FALSE,ch,obj,0,TO_CHAR);
        spell_sphere(50,ch,ch,0);
        obj->obj_flags.value[3]=30;
      } else {
        send_to_char ("The charm does nothing.\n",ch);
      }
      return TRUE;
    }
  }
  return FALSE;
}

//Chime of Cheer	26346
int christmas_chime_blessing_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg)
{

    int hp_gain;
    int mana_gain;
	bool isChristmas = check_is_christmas();

    if (cmd == MSG_REMOVE_ENCH)
    {
        send_to_char("Snowflakes envelop you and Christmas Cheer leaves your body.\n\r", ench_ch);
        return FALSE;
    }
    if (cmd == CMD_QUIT)
    {
        if (ch != ench_ch)
            return FALSE;
        send_to_char("The Cheer of Christmas leaves your body.\n\r", ench_ch);
        return FALSE;
    }

    if (ench_ch && cmd == MSG_TICK)
    {
        if(isChristmas){		
			hp_gain = number(100, 200);
			mana_gain = number(40, 80);
		}else {
			hp_gain = number(50, 100);
			mana_gain = number(20, 40);
		}

        GET_HIT(ench_ch) = GET_HIT(ench_ch) + hp_gain;
        GET_MANA(ench_ch) = GET_MANA(ench_ch) + mana_gain;
    }

    return FALSE;
}


int christmas_chime(OBJ *obj, CHAR *ch, int cmd, char *arg)
{

	CHAR *vict, *next_vict;
	char buf[MIL];
	CHAR *owner;
	
	if (cmd == CMD_USE)
    {
        /* Don't spec if no ch. */
        if (!ch)
            return FALSE;
        /* Don't spec if ch is not awake. */
        if (!AWAKE(ch))
            return FALSE;
        /* Don't spec if obj is not equipped by the actor. */
        if (!(owner = obj->equipped_by))
            return FALSE;
        /* Don't spec if actor is not the owner. */
        if (ch != owner)
            return FALSE;

        one_argument(arg, buf);

        /* Return if no target. */
        if (!*buf)
            return FALSE;
        
		//As long as it is equipped, grant the buff and disintigrate the familiar.
		if (*buf && is_abbrev(buf, "chime")){
			
			act("The chime glows as it is shaken.", FALSE, owner, 0, 0, TO_ROOM);
			act("The chime glows as it is shaken.", FALSE, owner, 0, 0, TO_CHAR);
		
			for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); vict; vict = next_vict)
			{
				next_vict = CHAR_NEXT_IN_ROOM(vict);

				if (IS_NPC(vict))
				{
					continue;
				}

				if (IS_MORTAL(vict))
				{
					enchantment_apply(ch, FALSE, CHRISTMAS_CHIME_BLESSING_NAME, 0, 60, ENCH_INTERVAL_TICK, 5, APPLY_DAMROLL, 0, 0, christmas_chime_blessing_func);
				}
			}
			
			if(chance(30)){
				act("The chime breaks on in your hand and falls apart.", FALSE, owner, 0, 0, TO_CHAR);
				act("The chime breaks in $n's hand and falls apart.", FALSE, owner, 0, 0, TO_ROOM);
				extract_obj(obj);
			}
			
			
			
			return TRUE;
		}
	}
	return FALSE;
		
}



int christmas_crown_frozen_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg)
{

    if (cmd == MSG_REMOVE_ENCH)
    {
        act("$n thaws out as the pillar melts around them.", TRUE, ench_ch, NULL, NULL, TO_ROOM);        
        return FALSE;
    }

    if (cmd == MSG_SHOW_AFFECT_TEXT)
    {
        act("......$n stands frozen in ice.", FALSE, ench_ch, 0, ch, TO_VICT);
        return FALSE;
    }

    /* The only thing you can do is look. Else, it will stop you from doing anything else. */

    if (cmd != CMD_LOOK)
    {
        if (ch != ench_ch)
            return FALSE;        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    return FALSE;
}

//Crown of Eternal Winter	26347


int christmas_crown(OBJ *obj, CHAR *ch, int cmd, char *arg)
{

	CHAR *vict, *next_vict;	
	CHAR *owner;
	int mana_gain;
	
	bool isChristmas = check_is_christmas();

	if (cmd == MSG_MOBACT) {
		owner = OBJ_EQUIPPED_BY(obj);

		if (!owner || !IS_MORTAL(owner)) return FALSE;

		if (chance(2)) {
			vict = GET_OPPONENT(owner);

			if (!vict || IS_IMMORTAL(vict) || (IS_MORTAL(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(vict)))) return FALSE;

		
		  
		 switch (number(0, 5))
			{
			case 0: //Heal all mortals in the room
				act("The crown glows with red and green colors.", FALSE, owner, 0, vict, TO_ROOM);
				act("The crown glows with red and green colors.", FALSE, owner, 0, vict, TO_CHAR);
			
				for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(owner)); vict; vict = next_vict)
				{
					next_vict = CHAR_NEXT_IN_ROOM(vict);

					if (IS_NPC(vict))
					{
						continue;
					}

					if (IS_MORTAL(vict))
					{
						spell_heal(50, ch, vict, NULL);
					}
				}
				break;
			case 1:   //Cast a ice spell against the current target.
				act("The crown generates a large ball of ice and throws it at $N.", FALSE, owner, 0, vict, TO_ROOM);
				act("The crown generates a large ball of ice and throws it at $N.", FALSE, owner, 0, vict, TO_CHAR);
				damage_spell(owner, vict, SPELL_ICEBALL, 50, number(300,500), DAM_COLD, SAVING_SPELL);
				break;
			case 2: // Stun the mob for 1 round.
				act("The crown generates a freezing wind that chills $N.", FALSE, owner, 0, vict, TO_ROOM);
				act("The crown generates a freezing wind that chills $N.", FALSE, owner, 0, vict, TO_CHAR);
				act("$N stops moving as they freeze.", FALSE, owner, 0, vict, TO_ROOM);
				act("$N stops moving as they freeze.", FALSE, owner, 0, vict, TO_CHAR);
				WAIT_STATE(vict,PULSE_VIOLENCE*1);
				break;
			case 3: //Freeze the Target for 2 rounds
				act("The crown generates a freezes $N solid.", FALSE, owner, 0, vict, TO_ROOM);
				act("The crown generates a freezes $N solid.", FALSE, owner, 0, vict, TO_CHAR);
				enchantment_apply(vict, TRUE, "CROWN FROZEN", TYPE_UNDEFINED, 2, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, christmas_crown_frozen_func);
				break;
			case 4: //Restore a little Mana
			
				if(isChristmas){							
					mana_gain = number(40, 80);
				}else {					
					mana_gain = number(20, 40);
				}
				act("A small amount of crystal clear snow covers $n.", FALSE, owner, 0, 0, TO_ROOM);
				act("A small amount of crystal clear snow covers you.", FALSE, owner, 0, 0, TO_CHAR);
				GET_MANA(owner) = GET_MANA(owner) + mana_gain;
				break;
			case 5:// Enforce a single hit (no dual/triple/quad).
				 act("The crown compels $n to strike their foe.", FALSE, owner, 0, vict, TO_ROOM);
				 act("The crown compels you to strike their foe.", FALSE, owner, 0, vict, TO_CHAR);
				 perform_hit(owner, vict, TYPE_UNDEFINED, 1); 
				break;

			}				
		  
	  }
	  return FALSE;
	}
	
	return FALSE;
		
}





/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */


int christmas_wishbound_spirit(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	    // These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict;

    int stun_delay;
	int reward = 3;
    // Define any other variables

    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    char *wishbound_spirit_speak[4] = {"A wish once made must be kept, and I will answer the call.", "Let hope take shape and strike!", "So many small dreams, so many bright threads.", "I carry their laughter on my ribbons."};

    switch (cmd)
    {
    case MSG_MOBACT:

        // Have him to chat to you.
        if (chance(35))
        {
            sprintf(buf, "%s", wishbound_spirit_speak[number(0, NUMELEMS(wishbound_spirit_speak) - 1)]);

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
                    act("$n grabs a present and throws it at the enemy.", 0, mob, 0, 0, TO_ROOM);
                    act("A large present hits you in the arm.", 0, mob, 0, vict, TO_VICT);
                    act("$N is in shock as a present hits their arm.", 0, mob, 0, vict, TO_NOTVICT);
                    damage(mob, vict, 1150, TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;
            case 1:
            case 2:
                act("$n shouts 'Unfurl, ribbons of promise!'", 0, mob, 0, 0, TO_ROOM);
				
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
                        continue;
                    act("A large ribbon wraps around you and constricts you tightly.", 0, mob, 0, vict, TO_VICT);
					damage(mob, vict, 800, TYPE_UNDEFINED, DAM_PHYSICAL);
                    stun_delay = number(1, 3);
                    WAIT_STATE(vict, PULSE_VIOLENCE * stun_delay);
                }
                break;
            case 3:
            case 4:
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n grabs a wrapped present and throws it at the enemy.", 0, mob, 0, 0, TO_ROOM);
                    act("A large wrapped present hits you in the head.", 0, mob, 0, vict, TO_VICT);
                    act("$N is int shock as a present hits their head.", 0, mob, 0, vict, TO_NOTVICT);                    
                    damage(mob, vict, 1200, TYPE_UNDEFINED, DAM_PHYSICAL);
					
					
                }
                break;
            case 5:
				act("$n shouts 'Feel the burst of Joy mortals.'", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
                        continue;
                    GET_HIT(vict) = GET_HIT(vict) - (GET_HIT(vict) / 10);
                }
				
			
                break;
            default:
                break;
            }
        }

        // can add an else branch here if you want them to act but not in combat.

        break;
		
		
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the joy of christmas has returned.\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			mob_aq_reward(reward, mob);
			break;
    }

    return FALSE;
}

int christmas_parcel_golem(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	    // These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict;

    int stun_delay;
	int reward = 3;
    // Define any other variables

    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    char *golem_speak[4] = {"Unauthorized opening detected. Prepare to be contained!", "Sealed and delivered — to you!", "Tag: FRAGILE. Threat level: HIGH.","Alert: Intruder detected"};

    switch (cmd)
    {
		
    case MSG_MOBACT:

        // Have him to chat to you.
        if (chance(35))
        {
            sprintf(buf, "%s", golem_speak[number(0, NUMELEMS(golem_speak) - 1)]);

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
            case 1:
				act("$n beeps  Reinforced packing: engage!", 0, mob, 0, 0, TO_ROOM);
				act("Layers of Cardboard appear around the Golem and fuse to the frame.", 0, mob, 0, 0, TO_ROOM);				
				GET_HIT(mob) = GET_HIT(mob) + 1100;				
				break;
            case 2:
                act("$n grabs parcels from around the room and throws them into the air. ", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
                        continue;
                    
					if(chance(44)){
						
						act("A large parcel falls onto your head.", 0, mob, 0, vict, TO_VICT);
						act("A large parcel falls onto $N", 0, mob, 0, vict, TO_NOTVICT);						
						damage(mob, vict, 1150, TYPE_UNDEFINED, DAM_PHYSICAL);
						stun_delay = number(1, 3);
						WAIT_STATE(vict, PULSE_VIOLENCE * stun_delay);
					}
                }
                break;
            case 3:
            case 4:
                vict = get_random_victim_fighting(mob);
                if (vict) 	
                {
                    act("$n winds up his arm.", 0, mob, 0, 0, TO_ROOM);
                    act("You are struck by $N in the chest by a large fist.", 0, mob, 0, vict, TO_VICT);
                    act("$N groans as they are hit by the golem's fist.", 0, mob, 0, vict, TO_NOTVICT);
                    damage(mob, vict, 1500, TYPE_UNDEFINED, DAM_PHYSICAL);
					stop_fighting(vict);
                }
                break;
            case 5: // A parcel stuns and removes from the fight
				vict = get_random_victim_fighting(mob);
                if (vict)
                {                    
                    act("An incredibly large parcel hits $s and explodes into fragments.", 0, mob, 0, vict, TO_VICT);
                    act("$N gasps as they are hit by $n.", 0, mob, 0, vict, TO_NOTVICT);
                    stop_fighting(vict);
					damage(mob, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
					WAIT_STATE(vict, 2 *PULSE_VIOLENCE);
                }
                break;
            default:
                break;
            }
        }

        // can add an else branch here if you want them to act but not in combat.

        break;
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the joy of christmas has returned.\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			mob_aq_reward(reward, mob);
			break;
    }

    return FALSE;


}

int christmas_guiding_light(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	    // These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict;

    // Define any other variables
	int reward = 3;
    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    char *light_speak[4] = {"Lost lights disturb the night — I will set them right.", "By the North’s beacon, be humbled!", "Follow the star, and fall before it!", "A steady glow steadies the heart."};

    switch (cmd)
    {
    case MSG_MOBACT:

        // Have him to chat to you.
        if (chance(35))
        {
            sprintf(buf, "%s", light_speak[number(0, NUMELEMS(light_speak) - 1)]);

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
                    act("$n glows brightly and blinds the room.", 0, mob, 0, 0, TO_ROOM);
                    act("You are blinded by a majestic light.", 0, mob, 0, vict, TO_VICT);
                    act("$N is blinded by a bright light.", 0, mob, 0, vict, TO_NOTVICT);
                    affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_HITROLL, -4, AFF_BLIND, 0);
					affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_ARMOR, 40, AFF_BLIND, 0);
                }
                break;
            case 1:
            case 2:
                act("$n splits into hundreds of pieces and explodes in front of everyone.", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
                        continue;
                   if(chance(65)){
						act("You cannot see after the explosion of light.", 0, mob, 0, vict, TO_VICT);
						affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_HITROLL, number(-1,-15), AFF_BLIND, 0);
						affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_STR, number(-1,-7), AFF_BLIND, 0);
						affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_DAMROLL, number(-20,-40), AFF_BLIND, 0);
						affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_ARMOR, 80, AFF_BLIND, 0);
				   }
                }
                break;
            case 3:
            case 4:
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n throws a ball of light into the air..", 0, mob, 0, 0, TO_ROOM);
                    act("A large ball of light strikes you in the chest..", 0, mob, 0, vict, TO_VICT);
                    act("A large ball of light strikes $N in the chest.", 0, mob, 0, vict, TO_NOTVICT);
                    damage(mob, vict, 1600, TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;
            case 5:
                break;
            default:
                break;
            }
			
        }
		break;
        // can add an else branch here if you want them to act but not in combat.
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the joy of christmas has returned.\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			mob_aq_reward(reward, mob);
        break;
    }

    return FALSE;
}

int christmas_merrymaker_jester(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	    // These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
    CHAR *tch, *vict, *next_vict, *current_tank;

    int stun_delay,card_number;
	int reward = 5;
	CHAR * jester_clone;
	int jester_clone_nr;
	jester_clone_nr = real_mobile(MERRYMAKER_JESTER_CLONE);

    // Define any other variables

    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    char *jester_speak[4] = {"Ho ho HOO! Let the games begin!", "Who wants a prize? Who wants a prize?", "A little joke to warm the halls.", "Do you prefer a bell or a joke?"};

    switch (cmd)
    {
    case MSG_MOBACT:

        // Have him to chat to you.
        if (chance(35))
        {
            sprintf(buf, "%s", jester_speak[number(0, NUMELEMS(jester_speak) - 1)]);

            do_say(mob, buf, CMD_SAY);
        }

        // if fighting - spec different attacks
        if (mob->specials.fighting)
        {
            // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
            // Each Case statement that has an action needs to break out at the end.

            switch (number(0, 3))
            {
            case 0:
                
				card_number = number(1,4);
				//Different cards are used for different effects.
				switch(card_number){
					
					case 1:  // Star Card: Jester is Buffed
							act("$n pulls the Star card from his deck.", 0, mob, 0, 0, TO_ROOM);
							act("$n jumps into the air and flys around.", 0, mob, 0, 0, TO_ROOM);
							affect_apply(mob, SPELL_FLY, 0, APPLY_HITROLL, 8, AFF_FLY, 0);
							affect_apply(mob, SPELL_FLY, 0, APPLY_DAMROLL, 8, AFF_FLY, 0);
							affect_apply(mob, SPELL_FLY, 0, APPLY_ARMOR, 80, AFF_FLY, 0);
						break;
					case 2:  // Skull Card: AoE to the room.
						act("$n pulls the Skull card from his deck.", 0, mob, 0, 0, TO_ROOM);
						act("Rays of darkness bombard the room.", 0, mob, 0, 0, TO_ROOM);
							for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
							{
								next_vict = CHAR_NEXT_IN_ROOM(vict);
								if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
									continue;
								damage(mob, vict, number(600,700), TYPE_UNDEFINED, DAM_PHYSICAL);
								stun_delay = number(1, 2);
								WAIT_STATE(vict, PULSE_VIOLENCE * stun_delay);
							}
							break;
					case 3: // Gift Card - Single Target Damage with a Gift				
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("$n pulls the Skull card from his deck.", 0, mob, 0, 0, TO_ROOM);
							act("A gift flings from $n's hand at you", 0, mob, 0, vict, TO_VICT);
							act("$n throws a gift at $N", 0, mob, 0, vict, TO_NOTVICT);
							damage(mob, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
						}
						break;
					case 4: // Twist Card - Swaps Tank.
					
						current_tank = GET_OPPONENT(mob);
						act("$n pulls the twist card from his deck.", 0, mob, 0, 0, TO_ROOM);
						if (GET_OPPONENT(mob) && GET_OPPONENT(mob) == current_tank)
						{
							act("$n suddenly slumps to the ground.", 0, mob, 0, current_tank, TO_VICT);
							stop_fighting(current_tank);
							damage(mob, current_tank, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(current_tank, 1*PULSE_VIOLENCE);
						}
												
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("You are now being focused by the Jester!", 0, mob, 0, vict, TO_VICT);
							act("$N is shocked as the jester attacks them.", 0, mob, 0, vict, TO_NOTVICT);
							damage(mob, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
						}				
						break;			
				}
				
                break;

            case 1:
				act("$n throws razor sharp cards at the room.", 0, mob, 0, 0, TO_ROOM);
				for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
				{
					next_vict = CHAR_NEXT_IN_ROOM(vict);
					if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
						continue;
					
					
					if(chance(40)){
						damage(mob, vict, number(400,700), TYPE_UNDEFINED, DAM_PHYSICAL);					
					}
				}
				break;
            case 2:
               // Spawn Jester Clones
				if ((tch = GET_OPPONENT(mob)) && mob_proto_table[jester_clone_nr].number < 2)
				{
					jester_clone = read_mobile(jester_clone_nr, REAL);
					char_to_room(jester_clone, CHAR_REAL_ROOM(mob));
					vict = get_random_victim_fighting(mob);

					act("$n laughs and disapears in a cloud of smoke.", FALSE, mob, 0, vict, TO_VICT);
					act("$n laughs at $N and disapears into smoke.", FALSE, mob, 0, vict, TO_NOTVICT);
					act("$n appears with a cackle and attacks $N.", FALSE, jester_clone, 0, vict, TO_ROOM);
					hit(jester_clone, vict, TYPE_UNDEFINED);

					return FALSE;
				}else {
					vict = get_random_victim_fighting(mob);
					if (vict)
					{
						act("I'll Torment you instead", 0, mob, 0, vict, TO_VICT);
						act("$n lunges at $N.", 0, mob, 0, vict, TO_NOTVICT);
						damage(mob, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
					}
					
				}
               
                break;
            case 3:
				break;
            default:
                break;
            }
        }

        // can add an else branch here if you want them to act but not in combat.

        break;
		
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the joy of christmas has returned.\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			mob_aq_reward(reward, mob);
			break;
		
    }

    return FALSE;
}

int christmas_yuletide_courier(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	    // These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict;

    int stun_delay,factor;
	int reward = 4;
    // Define any other variables

    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    char *courier_speak[4] = {"This violation delays the post — unacceptable.", "Parcel priority: neutralize!", "Speed of the sleigh! Make way!", "Stamps in order. Routes planned. Time matters."};

    switch (cmd)
    {
    case MSG_MOBACT:

        // Have him to chat to you.
        if (chance(35))
        {
            sprintf(buf, "%s", courier_speak[number(0, NUMELEMS(courier_speak) - 1)]);

            do_say(mob, buf, CMD_SAY);
        }

        // if fighting - spec different attacks
        if (mob->specials.fighting)
        {
            // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
            // Each Case statement that has an action needs to break out at the end.

            factor = 10* GET_HIT(mob) / GET_MAX_HIT(mob);
			switch (factor)
            {
            case 9:
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n casually hurls a parcel your way.", 0, mob, 0, 0, TO_ROOM);
                    act("A parcel is thrown at your face.", 0, mob, 0, vict, TO_VICT);
                    act("$N grunts as they are hit in the face with a parcel.", 0, mob, 0, vict, TO_NOTVICT);
                    damage(mob, vict, 950, TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;
            case 8:
				
				vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n slaps a PRIORITY label on a parcel and pitches it violently!", 0, mob, 0, 0, TO_ROOM);
                    act("A priority parcel is thrown at your face.", 0, mob, 0, vict, TO_VICT);
                    act("$N grunts as they are hit in the face with a priority parcel.", 0, mob, 0, vict, TO_NOTVICT);
                    damage(mob, vict, 1150, TYPE_UNDEFINED, DAM_PHYSICAL);
					if(chance(30)){
						act("You are knocked to the ground by the parcel", 0, mob, 0, vict, TO_VICT);
						act("$N is knocked to the ground by the parcel.", 0, mob, 0, vict, TO_NOTVICT);
						stun_delay = number(1, 3);
						WAIT_STATE(vict, PULSE_VIOLENCE * stun_delay);
					}
					
                }
                break;
            case 7:
                vict = get_random_victim_fighting(mob);
				act("Get out of my way, I have work to get done.", 0, mob, 0, 0, TO_ROOM);
                
                if (vict && chance(63))
                {
					act("You are shoved into a parcel and thrown out of the room..", 0, mob, 0, vict, TO_VICT);
					act("$N is shoved into a parcel and $n throws them out of the room.", 0, mob, 0, vict, TO_NOTVICT);
					damage(mob, vict, 300, TYPE_UNDEFINED, DAM_PHYSICAL);
					
                    char_from_room(vict);
                    char_to_room(vict, real_room(SORTING_HALL));
                    do_look(vict, "", CMD_LOOK);
                    act("$n appears from within a parcel.", FALSE, vict, 0, 0, TO_ROOM);
                }
                break;				
				
            case 6:
				act("$n runs around the room with a roll of tape.", 0, mob, 0, 0, TO_ROOM);
				for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
				{
					next_vict = CHAR_NEXT_IN_ROOM(vict);
					if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
						continue;
					
					switch (number(0, 3))
					{
					case 0:
						act("You are enveloped in tape and wrapping paper.", 0, mob, 0, vict, TO_VICT);
						act("$N is covered in tape and paper and falls to the floor.", 0, mob, 0, vict, TO_NOTVICT);
						stop_fighting(vict);
						WAIT_STATE(vict, 1*PULSE_VIOLENCE);						
						break;
					case 1:
					case 2:
						act("You are constricted by the tape.", 0, mob, 0, vict, TO_VICT);
						act("$N screams as they are constricted by tape.", 0, mob, 0, vict, TO_NOTVICT);
						damage(mob, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
						break;
					case 3:
						act("You have a bow taped to your face and are pushed out the door.", 0, mob, 0, vict, TO_VICT);
						act("$N is shoved into a parcel and $n throws them out of the room.", 0, mob, 0, vict, TO_NOTVICT);
						damage(mob, vict, 300, TYPE_UNDEFINED, DAM_PHYSICAL);						
						char_from_room(vict);
						char_to_room(vict, real_room(SORTING_HALL));
						do_look(vict, "", CMD_LOOK);
						act("$n appears confused with a bow on their face.", FALSE, vict, 0, 0, TO_ROOM);
						break;
					}
				}
				break;
			
            case 5:
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n throws glitter and packing materials into the air.", 0, mob, 0, 0, TO_ROOM);
                    act("A handful of glitter hits you in the face.", 0, mob, 0, vict, TO_VICT);
                    act("$N is hit in the face with a bunch of glitter.", 0, mob, 0, vict, TO_NOTVICT);
                    damage(mob, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
					affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_HITROLL, -30, AFF_BLIND, 0);
					affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_STR, -10, AFF_BLIND, 0);
					affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_DAMROLL, -30, AFF_BLIND, 0);
					affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_ARMOR, 120, AFF_BLIND, 0);
                }
                break;
            case 4:			
				vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n throws glitter and packing materials into the air.", 0, mob, 0, 0, TO_ROOM);
                    
					if (GET_OPPONENT(mob) && GET_OPPONENT(mob) == vict){
						act("A bunch of glowing glitter is scatted on you", 0, mob, 0, vict, TO_VICT);
						act("$N is covered in glowing glitter.", 0, mob, 0, vict, TO_NOTVICT);						
						affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_ARMOR, 200, AFF_BLIND, 0);	
						damage(mob, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
						
					}else {
						act("A packing label is tapped over your eyes", 0, mob, 0, vict, TO_VICT);
						act("$N has their eyes covered in a label.", 0, mob, 0, vict, TO_NOTVICT);
						affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_HITROLL, -30, AFF_BLIND, 0);
						damage(mob, vict, 1150, TYPE_UNDEFINED, DAM_PHYSICAL);
					}
					
                }
                break;
				
			case 3:
				act("$n strikes out with his legs.", 0, mob, 0, 0, TO_ROOM);
				for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
				{
					next_vict = CHAR_NEXT_IN_ROOM(vict);
					if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
						continue;
					
					if(chance(60)){
						act("You are struck by a powerful leg", 0, mob, 0, vict, TO_VICT);
						damage(mob, vict, number(400,700), TYPE_UNDEFINED, DAM_PHYSICAL);					
					}
				}
				break;
			case 2:
				act("$n Screams 'Desperate Times mean Desperate Measures'.", 0, mob, 0, 0, TO_ROOM);
				for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
				{
					next_vict = CHAR_NEXT_IN_ROOM(vict);
					if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
						continue;
					
					if(chance(70)){
						act("You are struck by a left hook", 0, mob, 0, vict, TO_VICT);
						damage(mob, vict, number(500,1100), TYPE_UNDEFINED, DAM_PHYSICAL);					
					}else{
						act("You are struck by a uppercut", 0, mob, 0, vict, TO_VICT);
						stop_fighting(vict);
						WAIT_STATE(vict, 2*PULSE_VIOLENCE);
					}
				}
                break;
			case 1:
			
				if(chance(10)){
					 do_say(mob, "I love ovetime", CMD_SAY);
					
				}			
			
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
				{
					next_vict = CHAR_NEXT_IN_ROOM(vict);
					if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
						continue;
					
					switch (number(0, 3))
					{
					case 0:
						act("You are enveloped in dense packing tape and wrapping paper.", 0, mob, 0, vict, TO_VICT);
						act("$N is covered in tape and paper and falls to the floor.", 0, mob, 0, vict, TO_NOTVICT);
						stop_fighting(vict);
						WAIT_STATE(vict, 2*PULSE_VIOLENCE);						
						break;
					case 1:
					case 2:
						act("You are constricted by the tape.", 0, mob, 0, vict, TO_VICT);
						act("$N screams as they are constricted by tape.", 0, mob, 0, vict, TO_NOTVICT);
						damage(mob, vict, 800, TYPE_UNDEFINED, DAM_PHYSICAL);
						break;
					case 3:
						act("You have a bow taped to your face and are pushed out the door.", 0, mob, 0, vict, TO_VICT);
						act("$N is shoved into a parcel and $n throws them out of the room.", 0, mob, 0, vict, TO_NOTVICT);
						damage(mob, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);						
						char_from_room(vict);
						char_to_room(vict, real_room(SORTING_HALL));
						do_look(vict, "", CMD_LOOK);
						act("$n appears confused with a bow on their face.", FALSE, vict, 0, 0, TO_ROOM);
						break;
					}
				}
				break;
				
            default:
                break;
            }
        }

        // can add an else branch here if you want them to act but not in combat.
        break;
		
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the joy of christmas has returned.\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			mob_aq_reward(reward, mob);
			break;
    }
    return FALSE;
}

int christmas_kringles_porter(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	    // These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict;

    int stun_delay;
	int reward = 4;
    // Define any other variables
	
	bool isChristmas = check_is_christmas();
	//If Not Month of November, dont spam anything.
	if (isChristmas == FALSE) return FALSE;

    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    char *porter_speak[4] = {"Lift, load, and learn your lesson!", "Feel the weight of winter!", "Carry the load. Keep the cheer.", "Strong shoulders, warmer hearts."};

    switch (cmd)
    {
    case MSG_MOBACT:

        // Have him to chat to you.
        if (chance(35))
        {
            sprintf(buf, "%s", porter_speak[number(0, NUMELEMS(porter_speak) - 1)]);

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
            case 1:
				vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n hefts his sack onto his shoulder.", 0, mob, 0, 0, TO_ROOM);
                    act("A large sack slams into your chest.", 0, mob, 0, vict, TO_VICT);
                    act("$N groans as a sack hits $S chest.", 0, mob, 0, vict, TO_NOTVICT);
                    damage(mob, vict, 850, TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;
			
            case 2:
                act("$n slams his hefty sack against the ground.", 0, mob, 0, 0, TO_ROOM);
				act("$A large shockwave spreads out from the ground.", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
					if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
                        continue;
					
					if(vict->specials.riding) {
						if(IS_AFFECTED(vict->specials.riding,AFF_FLY)){
							continue;
						}
					  }
					  
                    damage(mob, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
                    stun_delay = 4;
                    WAIT_STATE(vict, PULSE_VIOLENCE * stun_delay);
                }
                break;
            case 3:
				vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n opens his burly arms and sprints into the fray.", 0, mob, 0, 0, TO_ROOM);
                    act("You are grabbed and crushed between the Porter's arms.", 0, mob, 0, vict, TO_VICT);
                    act("$N is crushed between two massive arms and crumples to the ground.", 0, mob, 0, vict, TO_NOTVICT);
                    damage(mob, vict, 800, TYPE_UNDEFINED, DAM_PHYSICAL);
					stun_delay = 4;
                    WAIT_STATE(vict, PULSE_VIOLENCE * stun_delay);
                }
            case 4:
                act("$n swings his hefty sack against the ground.", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
				
                    if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
                        continue;
                    act("You are struck by a large hefty sack.", 0, mob, 0, vict, TO_VICT);
					damage(mob, vict, number(700,1250), TYPE_UNDEFINED, DAM_PHYSICAL);
                    
                }
                break;
            case 5:
				act("$n psyches himself up with a little christmas tune.", 0, mob, 0, 0, TO_ROOM);
				affect_apply(mob, SPELL_FLY, 0, APPLY_HITROLL, 8, AFF_FLY, 0);
				affect_apply(mob, SPELL_FLY, 0, APPLY_DAMROLL, 8, AFF_FLY, 0);
				affect_apply(mob, SPELL_FLY, 0, APPLY_ARMOR, 80, AFF_FLY, 0);
                break;
            default:
                break;
            }
        }

        // can add an else branch here if you want them to act but not in combat.

        break;
		 case CMD_CAST:
			   if(!ch) return FALSE;
			   if (!(IS_MORTAL(ch))) return FALSE;
			   if (GET_MANA(ch) <= 0) return FALSE;
			   if (chance(30)){ // 30% chance to intercept the cast.				   
				   act("$n screams at $N. That isnt about Christmas.",0,mob,0,ch,TO_NOTVICT);
				   act("$n screams at you, causing you to fumble your cast.",0,mob,0,ch,TO_VICT);				
				   return TRUE;
			   }
			   break;
		 case CMD_SONG:
			   if(!ch) return FALSE;
			   if (!(IS_MORTAL(ch))) return FALSE;
			   if (GET_MANA(ch) <= 0) return FALSE;
			   if (chance(30)){ // 30% chance to intercept the cast.				   
				   act("$n screams at $N. That isnt about Christmas.",0,mob,0,ch,TO_NOTVICT);
				   act("$n screams at you, causing you to fumble your song.",0,mob,0,ch,TO_VICT);				
				   return TRUE;
			   }
			   break;
		case CMD_KICK:
		case CMD_AMBUSH:
		case CMD_ASSAULT:
		case CMD_BACKSTAB:
		case CMD_CIRCLE:
		case CMD_LUNGE:
		case CMD_PUMMEL:
			if(!ch) return FALSE;
			if (!(IS_MORTAL(ch))) return FALSE;
			act("$n grabs $N and throws them to the ground.",0,mob,0,ch,TO_NOTVICT);
			act("$n laughs at your pitiful attack and throws you to the ground!",0,mob,0,ch,TO_VICT);
			damage(mob,ch,350,TYPE_UNDEFINED,DAM_NO_BLOCK);
			return TRUE;
			break;
			
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the joy of christmas has returned.\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			mob_aq_reward(reward, mob);
			break;
    }
    return FALSE;
}

int wrapping_wraith_bleed_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg)
{

    int hp_loss;

    if (cmd == MSG_REMOVE_ENCH)
    {
        send_to_char("Your wounds close and stop bleeding.\n\r", ench_ch);
        return FALSE;
    }
    if (cmd == CMD_QUIT)
    {
        if (ch != ench_ch)
            return FALSE;
        send_to_char("You have lost a lot of blood and can't quit.\n\r", ench_ch);
        return TRUE;
    }

    // On Tick, lose 10% of your HP, but cant drop below 100 HP.

    if (ench_ch && cmd == MSG_TICK)
    {
        hp_loss = GET_MAX_HIT(ench_ch) * .025;

        if ((GET_HIT(ench_ch) - hp_loss) <= 100)
        {
            GET_HIT(ench_ch) = 100;
        }
        else
        {
            GET_HIT(ench_ch) -= hp_loss;
        }
    }

    return FALSE;
}

void wrapping_wraith_bleed_ench(CHAR *vict)
{
    ENCH *tmp_enchantment;

    CREATE(tmp_enchantment, ENCH, 1);
    tmp_enchantment->name = str_dup("A Nasty Papercut");
    tmp_enchantment->duration = 10;
    tmp_enchantment->func = wrapping_wraith_bleed_func;
    enchantment_to_char(vict, tmp_enchantment, FALSE);
    send_to_char("You feel a ribbon cut your skin.\n\r", vict);
}


int christmas_wrapping_wraith(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	    // These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict;

	int reward = 4;
    // Define any other variables
	
	//bool isChristmas = check_is_christmas();
	//If Not Month of November, dont spam anything.
	//if (isChristmas == FALSE) return FALSE;

    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    char *wraith_speak[4] = {"Untie yourself from fate… if you can!", "Bind! Fold! Unravel their courage!", "Ribbons of regret, tighten!", "A neat bow hides many sins."};

    switch (cmd)
    {
    case MSG_MOBACT:

        // Have him to chat to you.
        if (chance(35))
        {
            sprintf(buf, "%s", wraith_speak[number(0, NUMELEMS(wraith_speak) - 1)]);

            do_say(mob, buf, CMD_SAY);
        }

        // if fighting - spec different attacks
        if (mob->specials.fighting)
        {
            // Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
            // Each Case statement that has an action needs to break out at the end.

            switch (number(0, 1))
            {
            case 0: // Physical Form
                   switch (number(0, 2))
					{
					case 0:
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("$n conjurs a ribbon into the room.", 0, mob, 0, 0, TO_ROOM);
							act("A single ribbon strikes you hard.", 0, mob, 0, vict, TO_VICT);
							act("$N is suddenly struck by a single ribbon.", 0, mob, 0, vict, TO_NOTVICT);
							damage(mob, vict, 1500, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, 1*PULSE_VIOLENCE);
						}
						break;
					case 1:
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("$n Manifests a single ribbon into the room.", 0, mob, 0, 0, TO_ROOM);
							act("You are entangled by a single ethereal ribbon.", 0, mob, 0, vict, TO_VICT);
							act("$N is entagled by a ribbon.", 0, mob, 0, vict, TO_NOTVICT);
							damage(mob, vict, 1250, TYPE_UNDEFINED, DAM_PHYSICAL);
							stop_fighting(vict);
						}
						break;
					case 2:
						act("$n manifests ribbons into the room.", 0, mob, 0, 0, TO_ROOM);
						for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
						{
							next_vict = CHAR_NEXT_IN_ROOM(vict);
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict)))
								continue;
							
							if (!enchanted_by(vict, "A Nasty Papercut") && chance(40))
							{
								wrapping_wraith_bleed_ench(vict);
							}else{
								act("Papercuts hurt!", 0, mob, 0, vict, TO_VICT);
								damage(mob, vict, 700, TYPE_UNDEFINED, DAM_PHYSICAL);
							}
						}
						break;					
					default:
						break;
					}
				break;
            case 1: // Ethereal Form
				   switch (number(0, 2))
					{
					case 0:
						vict = get_random_victim_fighting(mob);
						if (vict)
						{							
							act("A beam of electrictity shoots into you.", 0, mob, 0, vict, TO_VICT);
							act("$N is hit by a beam of electrictity.", 0, mob, 0, vict, TO_NOTVICT);
							damage(mob, vict, 1150, TYPE_UNDEFINED, DAM_ELECTRIC);
						}
						break;
					case 1:
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("A beam of fire shoots into you.", 0, mob, 0, vict, TO_VICT);
							act("$N is hit by a beam of fire.", 0, mob, 0, vict, TO_NOTVICT);
							damage(mob, vict, 1250, TYPE_UNDEFINED, DAM_FIRE);
						}
						break;
					case 2:
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("A beam of frost shoots into you.", 0, mob, 0, vict, TO_VICT);
							act("$N is hit by a beam of frost.", 0, mob, 0, vict, TO_NOTVICT);
							damage(mob, vict, 1350, TYPE_UNDEFINED, DAM_COLD);
						}
						break;
					default:
						break;
					}
				break;
            default:
                break;
            }
        }

        // can add an else branch here if you want them to act but not in combat.

        break;
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the joy of christmas has returned.\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			mob_aq_reward(reward, mob);
			break;
    }

    return FALSE;
}

//Holiday Mob Summoning NPC
int christmas_giftwarden(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	    // These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
    CHAR * holiday_summon;
	int summon_room;
	int holiday_summon_nr;
	CHAR *vict;
	
	bool give_back = FALSE;
	bool can_summon = FALSE;
	
    // Define any other variables
	
    /*Don't waste any more CPU time if no one is in the room. */
    //if (count_mortals_room(mob, TRUE) < 1)
     //   return FALSE;

    char *giftwarden_speak[4] = {"Hello All - Have a season full of joy and jolly fun.", "Christmas is the best time of year", "I love presents...if you have any.", "The remnants of christmas past slumber here."};

    switch (cmd)
    {
    case MSG_MOBACT:

        // Have him to chat to you.
        if (chance(35))
        {
            sprintf(buf, "%s", giftwarden_speak[number(0, NUMELEMS(giftwarden_speak) - 1)]);

            do_say(mob, buf, CMD_SAY);
        }

        
        // can add an else branch here if you want them to act but not in combat.

        break;
	case MSG_ENTER:
        // If NPCs or they are already fighting, dont do anything else.        
        
        act("$n Smiles and extends his hand.", TRUE, mob, 0, 0, TO_ROOM);
		act("Welcome travellers.  Please provide the proof of Christmas Joy.", TRUE, mob, 0, 0, TO_ROOM);
        break;
	
	case CMD_KILL:
    case CMD_HIT:
    case CMD_KICK:
    case CMD_AMBUSH:
    case CMD_ASSAULT:
    case CMD_BACKSTAB:
        do_say(mob, "Stop That. That doesn't bring christmas cheer.", CMD_SAY);
        return TRUE;

    case MSG_VIOLENCE:

        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = vict->next_in_room)
            if (vict->specials.fighting == mob)
                stop_fighting(vict->specials.fighting);
        stop_fighting(mob);
        GET_HIT(mob) = GET_MAX_HIT(mob);

        do_say(mob, "Please leave me in peace.  Bring me presents instead.", CMD_SAY);

        return FALSE;
	
	case MSG_OBJ_GIVEN:
	
	    arg = one_argument(arg, buf);
		
        OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);
        
		if (!obj)
            return TRUE;
		//First Check if the Item give is a Christmas Event Item
		//If it is, do the following
		// Each Item spawns its own mob if one doesnt already exist.  if exist, give item back.
	
        if (V_OBJ(obj) >= PRESENT_BOTTOM && V_OBJ(obj) <= PRESENT_TOP)
        {
			
			if(V_OBJ(obj) == WRAPPED_PRESENT){
				holiday_summon_nr = real_mobile(WISHBOUND_SPIRIT);
				summon_room = WISHBOUND_SPIRIT_ROOM;
				can_summon = TRUE;				
			}else if(V_OBJ(obj) == SEALED_GIFTBOX){
				holiday_summon_nr = real_mobile(PARCEL_GOLEM);
				summon_room = PARCEL_GOLEM_ROOM;
				can_summon = TRUE;
			}else if(V_OBJ(obj) == TOPPED_PARCEL){
				holiday_summon_nr = real_mobile(GUIDING_LIGHT);
				summon_room = GUIDING_LIGHT_ROOM;
				can_summon = TRUE;
			}else if(V_OBJ(obj) == BUNDLE_CHEER){
				holiday_summon_nr = real_mobile(MERRYMAKER_JESTER);
				summon_room = MERRYMAKER_JESTER_ROOM;
				can_summon = TRUE;
			}else if(V_OBJ(obj) == SANTA_LETTER){
				holiday_summon_nr = real_mobile(YULETIDE_COURIER);
				summon_room = YULETIDE_COURIER_ROOM;
				can_summon = TRUE;
			}else if(V_OBJ(obj) == SANTA_GIFT){
				holiday_summon_nr = real_mobile(KRINGLES_PORTER);
				summon_room = KRINGLES_PORTER_ROOM;
				can_summon = TRUE;
			}else if(V_OBJ(obj) == FESTIVE_PARCEL){
				holiday_summon_nr = real_mobile(WRAPPING_WRAITH);
				summon_room = WRAPPING_WRAITH_ROOM;
				can_summon = TRUE;
			}else {
				give_back = TRUE;			
			}
			//If we dont need to give an item back, we need to now validate if the mob exists in the wild. 
			if(can_summon){
				//Check the Proto Table for the Mob.  If less than 1 exist, summon the mob.
				if(mob_proto_table[holiday_summon_nr].number < 1){
					do_say(mob, "A remnant from the past has spawned. Be careful.", CMD_SAY);
					//Remove the Object
					give_back = FALSE;
					extract_obj(obj);
					//Summon the Mob.
					holiday_summon = read_mobile(holiday_summon_nr, REAL);
					char_to_room(holiday_summon, real_room(summon_room));
				
				}else {
					do_say(mob, "A remnant already exists. Vanquish them first.", CMD_SAY);
					give_back = TRUE;
				}
			}
			
        }
        else
        {
            do_say(mob, "This is not related to christmas.  Please take it back.", CMD_SAY);
			give_back = TRUE;
        }

        if (give_back)
        {

            act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);
            obj_from_char(obj);
            obj_to_char(obj, ch);

            return TRUE;
        }
		break;
	}

    return FALSE;
}

void assign_christmasvillage(void) {

/*Objects */

assign_obj(CHRISTMAS_AMULET, christmas_amulet);
assign_obj(CHRISTMAS_MECHANICAL_FAMILIAR, christmas_mechanical_familiar);
assign_obj(CHRISTMAS_CHARM, christmas_charm);
assign_obj(CHRISTMAS_CHIME, christmas_chime);
assign_obj(CHRISTMAS_CROWN, christmas_crown);


/*Rooms*/ 

/*Mobiles*/
assign_mob(WISHBOUND_SPIRIT, christmas_wishbound_spirit);
assign_mob(PARCEL_GOLEM, christmas_parcel_golem);
assign_mob(GUIDING_LIGHT, christmas_guiding_light);
assign_mob(MERRYMAKER_JESTER, christmas_merrymaker_jester);
assign_mob(YULETIDE_COURIER, christmas_yuletide_courier);
assign_mob(KRINGLES_PORTER, christmas_kringles_porter);
assign_mob(WRAPPING_WRAITH, christmas_wrapping_wraith);
assign_mob(GIFTWARDEN, christmas_giftwarden);
 
 
}





