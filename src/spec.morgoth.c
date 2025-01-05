/*spec.Morgoth.c - Specs for Morgoth Revamp by Fisher and Boreal

     Written by Fisher for RoninMUD
     Creation Date: 1/2/2025
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
#define MORGOTH_ALTAR 9652





/*Mobs */
#define MORGOTH_NECROMANCER 9675
#define UNDEAD_CLERIC 9670
#define UNDEAD_MAGE 9671
#define UNDEAD_BARD 9672
#define UNDEAD_WARRIOR 9673
#define UNDEAD_THIEF 9674
#define UNDEAD_PALADIN 9676
#define UNDEAD_COMMANDO 9677
#define UNDEAD_ANTI_PALADIN 9678
#define UNDEAD_NINJA 9679

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
/*

	The Altar will combine the 9 class items and the Book of the Dead
	and it will produce a random Class ring - With a chance to remove the 
	decay part from the item.

	All 10 Items are
	
	Necromanacer - [9675 ] The book of the dead 
	Paladin - [9685 ] a tarnished silver cross 
	Anti-Paladin - [9687 ] a sharp, poisoned dagger 
	Mage - [9681 ] dirty mage robes
	Thief - [9684 ] a shabby purse filled with coins 
	Ninja - [9688 ] a dull, rusty katana 
	Cleric - [9680 ] torn robes of the cleric
	Commando - [9686 ] a worn pair of battle gloves 
	Bard - [9682 ] smashed and broken guitar
	Warrior - [9683 ] the gilded hilt of a broken sword 
	

	
	Class RIng Entries are
	
	Paladin
	[10500] Signet of the Tarion Cavaliers 
	[12909] a band of holy spirits 
	
	Anti-Paladin
	[16828] Sigil of the Speaking-Demon 
	[6807 ] The Ring of the Conqueror (ring conqueror)
	
	Mage
	[26413] Lost Signet of Mahamoti Djinn 
	[17310] The Dark Seal of Illusions 
	
	Thief
	[27124] A ring of tattered flesh 
	[13026] Ring of Venom 
	
	
	Ninja
	[20146] shimmering band of Mokume Gane 
	[20185] Yawata's Ring of Power 
	
	Cleric
	[12908] The Prismatic Star of Gemini
	[26412] A Band of Demonic Aura 
	
	Commando
	[11327] a band of twisted veins 
	[712  ] The Severed Talon of a Lizard 
	
	Bard
	[12934] a glittering moonstone band 
	[26478] The Signet Ring of the Sultan 
	
	Warrior
	[26581] Ringlet of Coiled Bronze 
	[585  ] Small Portal of Evil 
	
	Nomad
	[4489 ] Ornament of Righteousness 
	[17372] The Seal of the Drake 
	


*/
int morgoth_altar(OBJ * altar, CHAR * ch, int cmd, char * arg) {
  OBJ *obj, *next_obj,*new_ring;
  char buf[MIL];
  
  int ring_num;
  
  int necromancer = FALSE, paladin = FALSE, antipaladin = FALSE, mage = FALSE, thief = FALSE, ninja = FALSE, cleric = FALSE, commando = FALSE, bard = FALSE, warrior = FALSE;

  int class_rings[20] = {10500,12909,16828,6807,26413,17310,27124,13026,20146,20185,12908,26412,11327,712,12934,26478,26581,585,4489,17372};


  if (cmd == CMD_UNKNOWN) {

    arg = one_argument(arg, buf);
    if ( *buf && is_abbrev(buf, "rise")) {
      //The spec is assigned directly to the object

      //Once you have the altar - Loop through all items.
      //Once all 10 items are placed inside, transform them into a random ring.

      for (obj = altar -> contains; obj; obj = next_obj) {
        next_obj = obj -> next_content;
        if (V_OBJ(obj) == 9675) {
          /* Necromanacer - [9675 ] The book of the dead  */
          necromancer = TRUE;
          
        }
        if (V_OBJ(obj) == 9685) {
          /* Paladin - [9685 ] a tarnished silver cross  */
          paladin = TRUE;
          
        }
        if (V_OBJ(obj) == 9687) {
          /* Anti-Paladin - [9687 ] a sharp, poisoned dagger   */
          antipaladin = TRUE;
          
        }
        if (V_OBJ(obj) == 9681) {
          /* Mage - [9681 ] dirty mage robes  */
          mage = TRUE;
          
        }
        if (V_OBJ(obj) == 9684) {
          /* Thief - [9684 ] a shabby purse filled with coins   */
          thief = TRUE;
          
        }
        if (V_OBJ(obj) == 9688) {
          /* Ninja - [9688 ] a dull, rusty katana   */
          ninja = TRUE;
          
        }
        if (V_OBJ(obj) == 9680) {
          /* Cleric - [9680 ] torn robes of the cleric  */
          cleric = TRUE;
          
        }
        if (V_OBJ(obj) == 9686) {
          /* Commando - [9686 ] a worn pair of battle gloves   */
          commando = TRUE;
          
        }
        if (V_OBJ(obj) == 9682) {
          /* Bard - [9682 ] smashed and broken guitar  */
          bard = TRUE;
          
        }
        if (V_OBJ(obj) == 9683) {
          /* Warrior - [9683 ] the gilded hilt of a broken sword   */
          warrior = TRUE;
          
        }

      }
	  
	  // Now that we have all the items remove, we need to add in a random class ring.
		
		// There is a 10% chance that the Decay will be removed.
		
		ring_num = class_rings[number(0, NUMELEMS(class_rings) - 1)];
				
		new_ring = read_object(ring_num,VIRTUAL);
		
		if(!new_ring){
			send_to_char("Nothing seems to happen.\n\r", ch);
			return TRUE;
			
		}
	  

      //All items are in the Altar.   REmove them all and load the ring. 
      if (necromancer && paladin && antipaladin && mage && thief && ninja && cleric && commando && bard && warrior) {
		
        act("The altar glows red and blood drains on the floor.", 1, ch, obj, 0, TO_CHAR);
        act("The altar glows red and blood drains on the floor.", 1, ch, obj, 0, TO_ROOM);
        

        for (obj = altar -> contains; obj; obj = next_obj) {
          next_obj = obj -> next_content;

          if (V_OBJ(obj) == 9675){ /* Necromanacer - [9675 ] The book of the dead  */
            extract_obj(obj);
		  }
          if (V_OBJ(obj) == 9685){ /* Paladin - [9685 ] a tarnished silver cross  */
            obj_from_obj(obj);
			extract_obj(obj);
		  }
          if (V_OBJ(obj) == 9687){ /* Anti-Paladin - [9687 ] a sharp, poisoned dagger   */
            obj_from_obj(obj);
			extract_obj(obj);
		  }
          if (V_OBJ(obj) == 9681){ /* Mage - [9681 ] dirty mage robes  */
            obj_from_obj(obj);
			extract_obj(obj);
		  }
          if (V_OBJ(obj) == 9684){ /* Thief - [9684 ] a shabby purse filled with coins   */
            obj_from_obj(obj);
			extract_obj(obj);
		  }
          if (V_OBJ(obj) == 9688){ /* Ninja - [9688 ] a dull, rusty katana   */
            obj_from_obj(obj);
			extract_obj(obj);
		  }
          if (V_OBJ(obj) == 9680){ /* Cleric - [9680 ] torn robes of the cleric  */
            obj_from_obj(obj);
			extract_obj(obj);
		  }
          if (V_OBJ(obj) == 9686){ /* Commando - [9686 ] a worn pair of battle gloves   */
            obj_from_obj(obj);
			extract_obj(obj);
		  }
          if (V_OBJ(obj) == 9682){ /* Bard - [9682 ] smashed and broken guitar  */
            obj_from_obj(obj);
			extract_obj(obj);
		  }
          if (V_OBJ(obj) == 9683){ /* Warrior - [9683 ] the gilded hilt of a broken sword   */
            obj_from_obj(obj);
			extract_obj(obj);
		  }
			
					
        }
		
		
		if(chance(10)) REMOVE_BIT(new_ring->obj_flags.extra_flags2,ITEM_EQ_DECAY);
		
		obj_to_obj(new_ring, altar);
		return TRUE;

      } 
	  else {
        send_to_char("Nothing seems to happen.\n\r", ch);
        return TRUE;
      }

    }
  }
  return FALSE;
}


/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */

/*
THe necromancer will summon random mobs up to 4 when engaged to engage the party.

[9670 ] An undead cleric (undead cleric) - Cleric
[9671 ] An undead mage (undead mage) - Magic User
[9672 ] An undead bard (bard dancing) - Bard
[9673 ] An undead warrior (undead warrior) - Warrior
[9674 ] An undead thief (undead thief) - Thief
[9675 ] The necromancer (necromancer) - Normal
[9676 ] An undead paladin (undead paladin) - Paladin
[9677 ] An undead commando (undead commando) - Commando
[9678 ] An undead anti-paladin (anti paladin anti-paladin undead) - Anti-Paladin
[9679 ] An undead ninja (undead ninja) - Ninja


*/


int morgoth_necromancer(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *tch, *vict;//, *next_vict;

	CHAR *undead;
	int undead_nr;	
	
	//Define any other variables

	//Define all the undead mobs he can summon.
	int undead_mobs[9] = {UNDEAD_CLERIC,UNDEAD_MAGE,UNDEAD_BARD,UNDEAD_WARRIOR,UNDEAD_THIEF,UNDEAD_PALADIN,UNDEAD_COMMANDO,UNDEAD_ANTI_PALADIN,UNDEAD_NINJA};
	
	char *necromanacer_speak[4] = { "Test1", "Test2", "Test3", "Test4" };

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			//Have him to chat to you.
			if (chance(40))
			{
				sprintf(buf, "%s", necromanacer_speak[number(0, NUMELEMS(necromanacer_speak) - 1)]);

				do_say(mob, buf, CMD_SAY);
			}

			//if fighting - spec different attacks
			if (mob->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.
				
				// Summon Undead Helpers at the start.
				do_say(mob, "The undead will kill you all.", CMD_SAY);
				if (!IS_SET(GET_BANK(mob), STATE1))
				{
					SET_BIT(GET_BANK(mob), STATE1);
					//Select a random mob to summon.
					
					undead_nr = real_mobile(undead_mobs[number(0, NUMELEMS(undead_mobs) - 1)]);
					undead = read_mobile(undead_nr, REAL);
					char_to_room(undead, CHAR_REAL_ROOM(mob));
					
					if((tch = GET_OPPONENT(mob))){
					
						hit(undead, tch, TYPE_UNDEFINED);
					}
				}
				if (!IS_SET(GET_BANK(mob), STATE2))
				{
					SET_BIT(GET_BANK(mob), STATE2);
					//Select a random mob to summon.
					
					undead_nr = real_mobile(undead_mobs[number(0, NUMELEMS(undead_mobs) - 1)]);
					undead = read_mobile(undead_nr, REAL);
					char_to_room(undead, CHAR_REAL_ROOM(mob));
					
					if((tch = GET_OPPONENT(mob))){
					
						hit(undead, tch, TYPE_UNDEFINED);
					}
				}
				if (!IS_SET(GET_BANK(mob), STATE3))
				{
					SET_BIT(GET_BANK(mob), STATE3);
					//Select a random mob to summon.
					
					undead_nr = real_mobile(undead_mobs[number(0, NUMELEMS(undead_mobs) - 1)]);
					undead = read_mobile(undead_nr, REAL);
					char_to_room(undead, CHAR_REAL_ROOM(mob));
					
					if((tch = GET_OPPONENT(mob))){
					
						hit(undead, tch, TYPE_UNDEFINED);
					}
				}
				
				
				switch (number(0, 3))
				{
					case 0:
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("$n calmly touches $N with his gnarly.  You don't need this, do you?", 0, mob, 0, vict, TO_NOTVICT);
							act("$n calmly touches you saying you don't need this, do you? ", 0, mob, 0, vict, TO_VICT);
							GET_HIT(vict) = GET_HIT(vict) *0.8;	//Remove 20% of Current HP
						}

						break;
					case 1:
						break;
					case 2:
						break;
					case 3:
						break;					
					default:
						break;
				}
			}

			//can add an else branch here if you want them to act but not in combat.

			break;

		case MSG_DIE:	// on boss death reward AQP

			break;
	}

	return FALSE;
}


//Assign Spec for the zone. Sets all other specs. 
//First Param - Object, Room or Mob Number.  Define it up above.
//Second Param - the name of the function that is for the mob Usually a shorthand for your zone. 
void assign_morgoth(void)
{
	/*Objects */
	//assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);
	assign_obj( MORGOTH_ALTAR, morgoth_altar );

	/*Rooms */
	//assign_room(LIGHTWALL,             	cl_LightWallLink);

	/*Mobs */

	assign_mob(MORGOTH_NECROMANCER, morgoth_necromancer);


}