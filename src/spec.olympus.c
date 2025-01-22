/*spec.Morgoth.c - Specs for Morgoth Revamp by Fisher and Boreal

     Written by Fisher for RoninMUD
     Creation Date: 1/2/2025
	 Basic Specs for the mobs and rooms in the zone.
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

/*Rooms */

#define WATERFALL_ROOM 28866
#define OFF_MOUNTAIN 28801
#define HEPHAESTUS_FORGE_ROOM 28880

/*Objects */
#define LARGE_ROCK 28840
#define HEPHAESTUS_FORGE 28841


/*Mobs */
#define OLYMPUS_ZEUS 28804
#define OLYMPUS_HERA 28805
#define OLYMPUS_POSEIDON 28806
#define OLYMPUS_ARES 28807
#define OLYMPUS_APOLLO 28808
#define OLYMPUS_ARTEMIS 28809
#define OLYMPUS_HEPHAESTUS 28811


#define HERA_ARGUS 28831
#define HERA_PEACOCK 28832
#define HERA_LION 28833
#define HERA_EAGLE 28834
#define HERA_SERPENT 28835
#define HERA_BULL 28836
#define HERA_PHOENIX 28837

#define POSEIDON_STINGRAY 28815
#define POSEIDON_OCTOPUS 28816


#define OLYMPUS_TYPHON 28830


/*Miscellaneous strings */
//Generic States that are shifted to indicate different stages.
//Each Uber will use this differently.  
#define STATE1 (1 << 0)	//1
#define STATE2 (1 << 1)	//2
#define STATE3 (1 << 2)	//4
#define STATE4 (1 << 3)	//8
#define STATE5 (1 << 4)	//32
#define STATE6 (1 << 5)	//16

/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */


/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

int olympus_waterfallroom(int room, CHAR *ch, int cmd, char *arg)
{
	OBJ *obj;
  	//When the zone is reset, remove the rock.
  if (cmd == MSG_ZONE_RESET)
  {	
	for(obj = world[room].contents; obj; obj = obj->next_content)
	{
		if(V_OBJ(obj)== LARGE_ROCK){
		obj_from_room(obj);
		}
		
	}
	return FALSE;
	
  }
  return FALSE;
}


/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */


int olympus_typhon(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict, *vict2, *vict3, *main_vict;

	int round_counter = 0; // Tracks the number of rounds in combat
    int current_phase = 1; // 0: Poison, 1: Ice, 2: Fire
    
	int reward = 10;
	int HP_to_Heal=0;
	int stun_delay;    
			
	char *typhon_speak[4] = { "", "","","" };

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		
		case MSG_TICK:		
			if (!mob->specials.fighting){
			round_counter = 0; // Reset counter when not fighting
			current_phase = 0; // Reset to default phase
			return FALSE;
			}
		
		break;
		
		case MSG_MOBACT:

			//Have him to chat to you.
			if (chance(40))
			{
				sprintf(buf, "%s", typhon_speak[number(0, NUMELEMS(typhon_speak) - 1)]);

				//do_say(mob, buf, CMD_SAY);
			}



			//if fighting - spec different attacks
			if (mob->specials.fighting)
			{
							 	
				 round_counter++;
				 if (round_counter % 10 == 0) {
						
						
					//I considered adding a chance here as well, but i think having it RNG a number 33% always makes it more fun.
					current_phase = number(0, 2);
					switch (current_phase) {
						case 0:							
							act("$n's body oozes poison, filling the room with venomous fumes!\r\n",0,mob,0,0,TO_ROOM);
							break;
						case 1:
							act("$n emits a freezing chill from his body and ice covers the floor.\r\n",0,mob,0,0,TO_ROOM);
							break;
						case 2:
							act("$n's body ignites with flames.\r\n",0,mob,0,0,TO_ROOM); 
							break;
					}
						
				}
				
				
				
				switch (current_phase) {
					case 0: // Poison Phase
						//Debug Statement to Watch the Phase Changes
						act("OMG POISON.\r\n",0,mob,0,0,TO_ROOM);
					
						switch (number(0, 2)){
						case 0:
						//Target the Tank
						vict=mob->specials.fighting;
						act("A giant snake violently bites $N who screams in agony.",1, mob, 0,vict,  TO_ROOM);
						act("A giant snake violently bites you and you scream in agony.",1, mob, 0,vict,  TO_VICT);						
						damage(mob, vict, 800, TYPE_UNDEFINED,DAM_PHYSICAL);
						
						
						
						break;
						case 1:
						//One of his legs bites the tank.   //Remove 25% HP from the Buffer.
						//Heal for 10% of the Buffers Max HP.
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("$n uncurls his leg and a giant snake lunges at $N.", 0, mob, 0, vict, TO_NOTVICT);
							act("$n uncurls his leg and a giant snake lunges at you.", 0, mob, 0, vict, TO_VICT);
							sprintf(buf, "%s is bitten by a giant snake.", GET_NAME(vict));
							act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
							sprintf(buf, "You are bitten by a giant snake.");
							act(buf, FALSE, mob, NULL, vict, TO_VICT);
							GET_HIT(vict) = GET_HIT(vict) - GET_HIT(vict) / 4;
							
							//Heal 10% of victims max hp.
							HP_to_Heal= GET_MAX_HIT(vict)/10;						
							GET_HIT(mob) = GET_HIT(mob) + HP_to_Heal;
						
						}
						
						break;
						case 2:
						//Poison everyone in the room,
						act("A giant snake on $n's Leg opens his mouth and breaths out smoke.",0,mob,0,0,TO_ROOM);						
						
						for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
							{
							  next_vict = vict->next_in_room;

							  if (IS_MORTAL(vict))
							  {
								cast_poison(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);  
								cast_blindness(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, vict, NULL);
							  }
							}
						
						
						
						
						break;
						default:
						break;
						}
						break;
					case 1: // Ice Phase
						act("OMG ICE.\r\n",0,mob,0,0,TO_ROOM);
									
						
						
						switch (number(0, 2)){
						
						
						case 0: //-- Freeze Random Characters - 2 Round Stun
						stun_delay = number(2, 4);
						vict = get_random_victim_fighting(mob);
						act("$n summons a pillar of ice under $N.", 0, mob, 0, vict, TO_NOTVICT);
						act("$n summons a pillar of ice under you.", 0, mob, 0, vict, TO_VICT);
						sprintf(buf, "%s is briefly encased in a pillar of ice.", GET_NAME(vict));
						act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
						sprintf(buf, "You are briefly encased in a pillar of ice.");
						act(buf, FALSE, mob, NULL, vict, TO_VICT);
												
						if (IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
						damage(mob,vict,450,TYPE_UNDEFINED,DAM_COLD);
						WAIT_STATE(vict,PULSE_VIOLENCE*stun_delay);
						
						break;
						case 1: //Ice Blast - 3 random characters
											
						vict = get_random_victim_fighting(mob);
						
						act("$n throws a large shard of ice at $N.", 0, mob, 0, vict, TO_NOTVICT);
						act("$n throws a large shard of ice at you.", 0, mob, 0, vict, TO_VICT);
						
						
						if (IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
						damage(mob,vict,500,TYPE_UNDEFINED,DAM_COLD);
						WAIT_STATE(vict,PULSE_VIOLENCE*1);

						//Check for more than 1 person in the room.
						if (count_mortals_room(mob, TRUE) > 1)
						{
							//get 2nd victim that is still fighting
							vict2 = get_random_victim_fighting(mob);
							act("$n throws a large shard of ice at $N.", 0, mob, 0, vict2, TO_NOTVICT);
							act("$n throws a large shard of ice at you.", 0, mob, 0, vict2, TO_VICT);
							if (!(vict) || IS_NPC(vict2) || !(IS_MORTAL(vict2))) return FALSE;
							damage(mob,vict,560,TYPE_UNDEFINED,DAM_COLD);
							WAIT_STATE(vict,PULSE_VIOLENCE*1);
						
							//Check for more than 2 people in the room.
							
							
							if (count_mortals_room(mob, TRUE) > 2)
							{
								//get 3rd victim that is still fighting
								vict3 = get_random_victim_fighting(mob);
								act("$n throws a large shard of ice at $N.", 0, mob, 0, vict3, TO_NOTVICT);
								act("$n throws a large shard of ice at you.", 0, mob, 0, vict3, TO_VICT);
								if (!(vict) ||!(vict2) || IS_NPC(vict3) || !(IS_MORTAL(vict3))) return FALSE;
								damage(mob,vict,620,TYPE_UNDEFINED,DAM_COLD);
								WAIT_STATE(vict,PULSE_VIOLENCE*1);
							}
						}
						
						
						break;
						case 2: //-- Blizzard - Ice Damage to everyone
						
						
						act("$n shoots large shards of ice at everyone in the room.",0,mob,0,0,TO_ROOM);
						for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(mob,vict,600,TYPE_UNDEFINED,DAM_COLD);
							WAIT_STATE(vict,PULSE_VIOLENCE*1);
						}
						break;
						
						break;
						default:
						break;
						}
						break;

					case 2: // Fire Phase
						act("OMG FIRE.\r\n",0,mob,0,0,TO_ROOM);
						switch (number(0, 2)){
						case 0: // Hit everyone in the room with fire.
						act("$n shoots balls of fire at everyone in the room.",0,mob,0,0,TO_ROOM);
						for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(mob,vict,400,TYPE_UNDEFINED,DAM_FIRE);
							
						}
				
						
						break;
						case 1: //COnflag like spec - Hit a main vict and then hit everyone else with smaller damage
						
						main_vict = get_random_victim_fighting(mob);
						
						act("$N is engulfed by a large jet of fire.", 0, mob, 0, main_vict, TO_NOTVICT);
						act("You are engulfed by a large jet of fire.", 0, mob, 0, main_vict, TO_VICT);
						if (!(main_vict) || IS_NPC(main_vict) || !(IS_MORTAL(main_vict))) return FALSE;
						damage(mob,vict,888,TYPE_UNDEFINED,DAM_FIRE);
						
						
						act("The residual heat from $n's jet of fire singes your skin",0,mob,0,0,TO_ROOM);
						for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if(!(vict) || !(main_vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(mob,vict,366,TYPE_UNDEFINED,DAM_FIRE);
							
						}
												
						break;
						case 2: // Hit random victim - set position sitting and stun for 2 rounds.
						
						stun_delay = number(1, 3);
						vict = get_random_victim_fighting(mob);
						act("An intense fire knocks $N to the ground.", 0, mob, 0, vict, TO_NOTVICT);
						act("An instense fire knocks you to the ground.", 0, mob, 0, vict, TO_VICT);
						
												
						if (IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
						damage(mob,vict,766,TYPE_UNDEFINED,DAM_FIRE);
						WAIT_STATE(vict,PULSE_VIOLENCE*stun_delay);
						GET_POS(vict) = POSITION_SITTING;
							
						break;
						default:
						break;
						}
						break;						
				}
				
				
			}

			//can add an else branch here if you want them to act but not in combat.

			break;

		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain. Olympus Seeks Revenge!\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if (IS_NPC(vict) || !IS_MORTAL(vict)) continue;
				sprintf(buf, "You are awarded with %d quest %s for the kill.\n\r", reward, reward> 1 ? "points" : "point");
				send_to_char(buf, vict);
				vict->ver3.quest_points += reward;
			}

			break;
	}

	return FALSE;
}


int olympus_zeus(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
  
  int stun_delay;

  //Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1) return FALSE;

  char *michael_speak[4] = { "This domain is mine alone!", "Don't you dare and try to tell me what to do.","Just obey my commands!","Leave this domain at once." };

  switch (cmd)
  {
    case MSG_MOBACT:

     	//Have him to chat to you.
      if (chance(35))
      {
        sprintf(buf, "%s", michael_speak[number(0, NUMELEMS(michael_speak) - 1)]);

        do_say(mob, buf, CMD_SAY);
      }

     	//if fighting - spec different attacks
      if (mob->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.

        switch (number(0, 5))
        {
          case 0:
            stun_delay = number(1, 3);
			act("$n shouts an oppressive order, causing your head to ache and your mind to wander.",0,mob,0,0,TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
				damage(mob,vict,600,TYPE_UNDEFINED,DAM_PHYSICAL);
				WAIT_STATE(vict,PULSE_VIOLENCE*stun_delay);
            }
            break;
          case 1:
            break;
          case 2:
            vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n wields his sword and advances towards the enemy.",0,mob,0,0,TO_ROOM);
			  act("A fine steel blade stabs into your chest.",0,mob,0,vict,TO_VICT);
			  act("$N screams as the blade pierces $S chest.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,1450,TYPE_UNDEFINED,DAM_PHYSICAL);
			}
			break;
		  case 3:
			break;
		  case 4:
			break;
		  case 5:
			vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n wields his sword and jabs towards the enemy.",0,mob,0,0,TO_ROOM);
			  act("A fine steel blade jabs into your chest.",0,mob,0,vict,TO_VICT);
			  act("$N screams as the blade jabs $S chest.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,850,TYPE_UNDEFINED,DAM_PHYSICAL);
			}
			break;		  
          default:
            break;
        }
      }

     	//can add an else branch here if you want them to act but not in combat.

      break;
   
  }

  return FALSE;
}


//Hera Wraithful Rebule
void hera_wrathful_rebuke(CHAR *ch, CHAR *vict)
{
	act("$n stuns you with a holy blow.", FALSE, ch, 0, vict, TO_VICT);
	act("$n stuns $N with a holy blow.", FALSE, ch, 0, vict, TO_NOTVICT);
	

	GET_POS(vict) = POSITION_STUNNED;

	if (GET_OPPONENT(vict))
	{
		stop_fighting(vict);
	}

	if (GET_OPPONENT(ch) && GET_OPPONENT(ch) == vict)
	{
		stop_fighting(ch);
		damage(ch, vict, 300, TYPE_UNDEFINED, DAM_PHYSICAL);
		WAIT_STATE(vict, 1 *PULSE_VIOLENCE);
	}
}

int olympus_hera(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict, *tch;
  
  //int stun_delay;
  CHAR *hera_summon;
  int hera_summon_nr, hera_summon_next_nr;
  int reward = 10;
  
  int hera_mobs[7] = {HERA_ARGUS,HERA_BULL,HERA_EAGLE,HERA_LION,HERA_PEACOCK,HERA_PHOENIX,HERA_SERPENT};

  //Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1) return FALSE;

  char *hera_speak[6] = { "A true queen does not tolerate insolence", "I have outlasted storms and betrayal; you are nothing.","My power rivals the heavens, yet mortals still test me.!","The strength of a queen lies in her patience... and her vengeance.","Loyalty is a rare gem, tarnished by betrayal and deceit.","Olympus is my throne, and I shall not be usurped." };
  
  char *hera_combat_speak[5] = { "The Queen of Olympus commands your end!","Your defiance is an insult to the divine!","Feel the weight of my righteous fury!","My vengeance is unyielding!","Face the wrath of a goddess betrayed!"};
  
  switch (cmd)
  {
    case MSG_MOBACT:

     	//Have him to chat to you if not fighting.
      if (chance(35) && (!mob->specials.fighting))
      {
        sprintf(buf, "%s", hera_speak[number(0, NUMELEMS(hera_speak) - 1)]);

        do_say(mob, buf, CMD_SAY);
      }

     	//if fighting - spec different attacks
      if (mob->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.
		
		
		if (chance(35))
      {
        sprintf(buf, "%s", hera_combat_speak[number(0, NUMELEMS(hera_combat_speak) - 1)]);

        do_say(mob, buf, CMD_SAY);
      }
		
		
		
				if (!IS_SET(GET_BANK(mob), STATE1))
				{
					do_say(mob, "Holy Beasts, Come to me!", CMD_SAY);
					
					SET_BIT(GET_BANK(mob), STATE1);
					//Select a random mob to summon.
					//Make sure only 1 is spawned in.
					hera_summon_nr = real_mobile(hera_mobs[number(0, NUMELEMS(hera_mobs) - 1)]);
					
					if ((tch = GET_OPPONENT(mob)) && mob_proto_table[hera_summon_nr].number < 1){
						
						hera_summon = read_mobile(hera_summon_nr, REAL);
						char_to_room(hera_summon, CHAR_REAL_ROOM(mob));
						
						if((tch = GET_OPPONENT(mob))){
						
							hit(hera_summon, tch, TYPE_UNDEFINED);
						}					
						
					}
					//Summon a 2nd Mob.  Ensure its not the same as the first.
					
					hera_summon_next_nr = real_mobile(hera_mobs[number(0, NUMELEMS(hera_mobs) - 1)]);
					//Keep looping if the number is the same.
					do {
							hera_summon_next_nr = real_mobile(hera_mobs[number(0, NUMELEMS(hera_mobs) - 1)]);
						} while (hera_summon_next_nr == hera_summon_nr); // Ensure a different index
					
					if ((tch = GET_OPPONENT(mob)) && mob_proto_table[hera_summon_next_nr].number < 1){
						
						hera_summon = read_mobile(hera_summon_next_nr, REAL);
						char_to_room(hera_summon, CHAR_REAL_ROOM(mob));
						
						if((tch = GET_OPPONENT(mob))){
						
							hit(hera_summon, tch, TYPE_UNDEFINED);
						}					
						
					}				
				}
					
		

        switch (number(0, 5))
        {
          case 0: // Break
            break;
          case 1://Cure Hera or Hera's Summons
		  
				//STATE2 is tracking the healing.  We will do 1 big heal at any point.  				
				if (!IS_SET(GET_BANK(mob), STATE2))
				{
					do_say(mob, "Holy Judgment will sustain me.", CMD_SAY);
					SET_BIT(GET_BANK(mob), STATE2);
					GET_HIT(mob) += (GET_MAX_HIT(mob) / 10);
				}
				else {
				
					do_say(mob, "Holy Judgment will continue to find you", CMD_SAY);
					SET_BIT(GET_BANK(mob), STATE2);
					GET_HIT(mob) += (GET_MAX_HIT(mob) / 25);
					
				}

				break;
            break;
          case 2: //Divine Retribution Single Char
            vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n throws holy energy at you.",0,mob,0,vict,TO_VICT);
			  act("$N gasps as they are hit by holy energy.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,1200,TYPE_UNDEFINED,DAM_MAGICAL);
			}
			break;
		  case 3: //Stun a Chracter out of the fight. Except the Tank.
		  
			vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n throws holy energy at you.",0,mob,0,vict,TO_VICT);
			  act("$N gasps as they are hit by holy energy.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,1200,TYPE_UNDEFINED,DAM_MAGICAL);
			}
			break;
			
		  
			
		  case 4: //Divine Wrath Full Room
			
			act("$n douses the room with massive rays of holy energy.",0,mob,0,0,TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
				damage(mob,vict,1100,TYPE_UNDEFINED,DAM_MAGICAL);
				
			}
		  
			break;
		  case 5:
			
			break;		  
          default:
            break;
        }
      }
	  case CMD_CIRCLE:
       if (!ch) return FALSE;       
       if (chance(25)){
		   act("You foolish actions will not be tolerated. ", 0, mob, 0, 0, TO_ROOM);
			hera_wrathful_rebuke(mob,ch);
			return TRUE;
	   }
       break;
	   case CMD_PUMMEL:
	   if (!ch) return FALSE;       
       if (chance(25)){
		   act("You foolish actions will not be tolerated. ", 0, mob, 0, 0, TO_ROOM);
			hera_wrathful_rebuke(mob,ch);
			return TRUE;
	   }
       break;
	   case CMD_CAST:
	   if (!ch) return FALSE;       
       if (chance(25)){
		   act("You foolish actions will not be tolerated. ", 0, mob, 0, 0, TO_ROOM);
			hera_wrathful_rebuke(mob,ch);
			return TRUE;
	   }
       break;
	   case CMD_SONG:
	   if (!ch) return FALSE;       
       if (chance(25)){
		   act("You foolish actions will not be tolerated. ", 0, mob, 0, 0, TO_ROOM);
			hera_wrathful_rebuke(mob,ch);
			return TRUE;
	   }
       break;
	   case CMD_DISEMBOWEL:
	   if (!ch) return FALSE;       
       if (chance(25)){
		   act("You foolish actions will not be tolerated. ", 0, mob, 0, 0, TO_ROOM);
			hera_wrathful_rebuke(mob,ch);
			return TRUE;
	   }
       break;
	   
	   
	   case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain. Olympus Seeks Revenge!\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if (IS_NPC(vict) || !IS_MORTAL(vict)) continue;
				sprintf(buf, "You are awarded with %d quest %s for the kill.\n\r", reward, reward> 1 ? "points" : "point");
				send_to_char(buf, vict);
				vict->ver3.quest_points += reward;
			}

			break;

     	//can add an else branch here if you want them to act but not in combat.

      
   
  }

  return FALSE;
  
}



int olympus_poseidon(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict, *tch, *tmp;
  
  int waterfall_room = 28866 ;
  int beginning_room = 28801;
  int large_rock = 28840;
  
  int stingray = 28815;
  int octopus = 28816;
  
  
  
  CHAR *poseidon_summon;
  int poseidon_summon_nr, poseidon_summon_next_nr;
  int reward = 10;
  
  
  OBJ *obj;
  int found = FALSE;
  int summon_dead = FALSE;
  

  //Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1) return FALSE;

  char *poseidon_speak[6] = { "Mortals are as fleeting as ripples in a pond.","The ocean's depths hold secrets you'll never know.","The ocean whispers secrets only I can understand.","Even the mightiest ships are but toys for my waves","Beware the depths, for they hold terrors beyond imagining.","The tide rises, and with it, my unrelenting fury." };
  
  char *poseidon_combat_speak[5] = { "The seas rise to swallow you whole!","No one escapes the tide of Poseidon's fury!","I am the unyielding tide, the ocean's master!","Drown beneath the waves of your folly!","You challenge the god of the sea? Foolish!"};
  
  
  switch (cmd)
  {
    case MSG_MOBACT:

     	//Have him to chat to you.
      if (chance(35) && (!mob->specials.fighting))
      {
        sprintf(buf, "%s", poseidon_speak[number(0, NUMELEMS(poseidon_speak) - 1)]);

        do_say(mob, buf, CMD_SAY);
      }

     	//if fighting - spec different attacks
      if (mob->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.

		  if (chance(35))
		  {
			sprintf(buf, "%s", poseidon_combat_speak[number(0, NUMELEMS(poseidon_combat_speak) - 1)]);

			do_say(mob, buf, CMD_SAY);
		  }
		  

		  
		  if (!IS_SET(GET_BANK(mob), STATE1))
				{
					do_say(mob, "Creatures!  Rise from the Deep!", CMD_SAY);
					
					SET_BIT(GET_BANK(mob), STATE1);
					//Select a random mob to summon.
					//Make sure only 1 is spawned in.
					poseidon_summon_nr = real_mobile(POSEIDON_STINGRAY);
					
					if ((tch = GET_OPPONENT(mob)) && mob_proto_table[poseidon_summon_nr].number < 1){
						
						poseidon_summon = read_mobile(poseidon_summon_nr, REAL);
						char_to_room(poseidon_summon, CHAR_REAL_ROOM(mob));
						
						if((tch = GET_OPPONENT(mob))){
						
							hit(poseidon_summon, tch, TYPE_UNDEFINED);
						}					
						
					}
					//Summon a 2nd Mob.  Ensure its not the same as the first.
					
					poseidon_summon_next_nr = real_mobile(POSEIDON_OCTOPUS);
					//Keep looping if the number is the same.
					
					if ((tch = GET_OPPONENT(mob)) && mob_proto_table[poseidon_summon_next_nr].number < 1){
						
						poseidon_summon = read_mobile(poseidon_summon_next_nr, REAL);
						char_to_room(poseidon_summon, CHAR_REAL_ROOM(mob));
						
						if((tch = GET_OPPONENT(mob))){
						
							hit(poseidon_summon, tch, TYPE_UNDEFINED);
						}					
						
					}				
				}
		  
		  


        switch (number(0, 5))
        {
          case 0:
            //Check for Summoned Mobs.
			for(tmp=world[CHAR_REAL_ROOM(mob)].people; tmp; tmp=tmp->next_in_room) {
				if(IS_MOB(tmp)) {
				  if(V_MOB(tmp)==octopus){
					  act("$n screams 'Get them my tentacled Friend'",1, mob, 0,0, TO_ROOM);
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
						  act("$n crushes you with his tentacle",0,tmp,0,vict,TO_VICT);
						  act("$n crushes $N with his tentacle.",0,tmp,0,vict,TO_NOTVICT);
						  
						  WAIT_STATE(vict, 3 *PULSE_VIOLENCE);
						}
						summon_dead = FALSE;
						break;
				  }else{
					  summon_dead = TRUE;				  
				  }
				  
				  
				}
			  }
			  
			  if(summon_dead){
				  act("$n screams 'How dare you kill my friends.",1, mob, 0,0, TO_ROOM);
					vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n stabs you in the chest with his trident.",0,mob,0,vict,TO_VICT);
					  act("$n stabs $N in the chest with his trident.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,1200,TYPE_UNDEFINED,DAM_MAGICAL);
					}
			  }
						
				
			
			break;
            
          case 1:
		   //Check for Summoned Mobs
			
			for(tmp=world[CHAR_REAL_ROOM(mob)].people; tmp; tmp=tmp->next_in_room) {
				if(IS_MOB(tmp)) {
				  if(V_MOB(tmp)==stingray){
					  act("$n screams 'Get them with your barb!'",1, mob, 0,0, TO_ROOM);
						vict = get_random_victim_fighting(mob);
						if (vict)
						{
						  act("$n stabs you in the chest with his stinger.",0,tmp,0,vict,TO_VICT);
						  act("$n stabs $N in the chest with his stinger.",0,tmp,0,vict,TO_NOTVICT);				  
						  damage(tmp,vict,300,TYPE_UNDEFINED,DAM_MAGICAL);
						  WAIT_STATE(vict, 2 *PULSE_VIOLENCE);
						}
						summon_dead = FALSE;
						break;
				  }
				  else {
					  summon_dead = TRUE;
					  
					  
					  
				  }
				}
			  }
			  
			  if(summon_dead){
				  act("$n screams 'How dare you kill my friends.",1, mob, 0,0, TO_ROOM);
					vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n stabs you in the chest with his trident.",0,mob,0,vict,TO_VICT);
					  act("$n stabs $N in the chest with his trident.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,1000,TYPE_UNDEFINED,DAM_MAGICAL);
					}
					
			  }	  
            
			break;
          case 2: //Send  A Victim from the Room to Room 2 South.
		  
			vict = get_random_victim_fighting(mob);			
			act("$n waves his trident and a giant wave of water appears.",0,mob,0,0,TO_ROOM);
			act("You wave your trident and a giant wave of water appears.",0,mob,0,0,TO_CHAR);
			act("$n is pushed out of the room by a wave of water.!",0,vict,0,0,TO_ROOM);
			act("You are hit by a wall of water.",0,vict,0,0,TO_CHAR);
			
			
			//Check For a Rock at the top of the waterfall
			//IF it exists stop the character.  If not wash them back to the start.			
			
			if(world[real_room(waterfall_room)].contents) {				
				 for(obj = world[real_room(waterfall_room)].contents; obj; obj = obj->next_content)
				{
					if(V_OBJ(obj) == large_rock){
						found = TRUE;
					}
					
				}
			}
			
			//Send a random Victim out of the room			
				
			if(found == TRUE){
				 char_from_room(vict);
				 char_to_room(vict,real_room(waterfall_room));
				 do_look(vict,"",CMD_LOOK);
				 act("$n appears soaking wet.", FALSE, vict, 0, 0, TO_ROOM);
				 return TRUE;
			  }
			  else{
				 char_from_room(vict);
				 char_to_room(vict,real_room(beginning_room));
				 do_look(vict,"",CMD_LOOK);
				 act("$n appears soaking wet.", FALSE, vict, 0, 0, TO_ROOM);
				 return TRUE;
			  }			 
								
			
			  
			  
            
			break;
		  case 3: //Trident Water Attack
			  act("$n floods the room with water. .",0,mob,0,0,TO_ROOM);
				for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
					damage(mob,vict,900,TYPE_UNDEFINED,DAM_MAGICAL);
					WAIT_STATE(vict, 2 *PULSE_VIOLENCE);
					act("A wall of water hits you hard.",0,mob,0,vict,TO_VICT);
					
				}
			break;
		  case 4: //Trident Electrical Attack
		  act("$n fires bolts of electricity from his trident.",0,mob,0,0,TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
				damage(mob,vict,1200,TYPE_UNDEFINED,DAM_MAGICAL);
				act("A bolt of electricity travels through your body.",0,mob,0,vict,TO_VICT);
				
			}
			break;
		  case 5:
			
			break;		  
          default:
            break;
        }
      }

     	//can add an else branch here if you want them to act but not in combat.
	
      break;
	 case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain. Olympus Seeks Revenge!\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if (IS_NPC(vict) || !IS_MORTAL(vict)) continue;
				sprintf(buf, "You are awarded with %d quest %s for the kill.\n\r", reward, reward> 1 ? "points" : "point");
				send_to_char(buf, vict);
				vict->ver3.quest_points += reward;
			}

			break;
  }

  return FALSE;
}




int ares_bleed_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {
	
	int hp_loss;
	
	
	if(cmd==MSG_REMOVE_ENCH) {
		send_to_char("Your wounds finally close and stop bleeding.\n\r",ench_ch);
		return FALSE;
	}
	if(cmd==CMD_QUIT) {
		if(ch!=ench_ch) return FALSE;
		send_to_char("You have lost too much blood to quit.\n\r",ench_ch);
		return TRUE;
	 }
	 
	 //On Tick, lose 10% of your HP, but cant drop below 100 HP.
	 
	 if(ench_ch && cmd==MSG_TICK){
		
		hp_loss = GET_MAX_HIT(ench_ch) * .1;
		
		if ((GET_HIT(ench_ch) - hp_loss) <= 100){
			GET_HIT(ench_ch) = 100;
		}else
		{
			GET_HIT(ench_ch) -= hp_loss;
		}
		 
		 
	 }
	  
	
	return FALSE;
	
}


void ares_bleed_ench(CHAR *vict){
	char buf4[MIL];
	ENCH *tmp_enchantment;
	
	 CREATE(tmp_enchantment, ENCH, 1);
	  tmp_enchantment->name     = str_dup("Ares-Slash");
	  tmp_enchantment->duration = 1;
	  tmp_enchantment->func     = ares_bleed_func;
	  enchantment_to_char(vict, tmp_enchantment, FALSE);
	  act("$n has been cut deeply by Ares's Slash.\n\r",FALSE,vict,0,0,TO_ROOM);
	  send_to_char("You feel a long slash deep into your flesh.\n\r",vict);
	  sprintf(buf4,"Olympus Log Ench: [ %s just contracted Ares Bleed at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
	  log_s(buf4);
	  
	
}



int olympus_ares(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

  
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
  
  int stun_delay;
  int factor;
  int reward = 10;
  
  
  //Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1) return FALSE;

  char *ares_speak[6] = { "Peace is but a fleeting illusion.","The drums of war echo in my soul.","Conflict breeds strength and honor.","In the calm before the storm, I sharpen my blade.","Even in times of peace, the warrior's spirit must remain vigilant.","You never should have stepped foot on this mountain." };
  
  char *ares_combat_speak[5] = { "Feel the wrath of the god of war!","Your blood shall stain the battlefield!","Embrace the chaos of combat!","I thrive in the fury of battle!","War is my domain, and you are but a pawn!"};
  
  
  switch (cmd)
  {
    case MSG_MOBACT:

     	//Have him to chat to you.
      if (chance(35) && (!mob->specials.fighting))
      {
        sprintf(buf, "%s", ares_speak[number(0, NUMELEMS(ares_speak) - 1)]);

        do_say(mob, buf, CMD_SAY);
      }

     	//if fighting - spec different attacks
      if (mob->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.

		  if (chance(35))
		  {
			sprintf(buf, "%s", ares_combat_speak[number(0, NUMELEMS(ares_combat_speak) - 1)]);

			do_say(mob, buf, CMD_SAY);
		  }
		/*3 Phases
		
		Sword Phase
		--All around attacks 
		Slashes
		--Can we do bleeds?   Tkae away 10% hp for 2 rounds.  
		
		
		Shield Phase
		-- Block Attack COmmands And Retaliate
		-- Damroll down - but more aggressive with stuns
		
		Spear Phase
		-- Single Target Stabs - High Damage
		--Damroll goes up, lower the specs		
		
		*/
		
		  factor=10*GET_HIT(mob)/GET_MAX_HIT(mob);
		  switch(factor)
		  {
			case 9:
			  
			  switch (number(0, 2)){
				  
				  case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")){
						ares_bleed_ench(vict);
					}					
				  
					break;
				  case 1:					
				  case 2:
				  vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n wields his sword and slashes towards $N.",0,mob,0,vict,TO_ROOM);
					  act("A blade slashes into your flesh.",0,mob,0,vict,TO_VICT);
					  act("$N screams as the blade slashes $S flesh.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,650,TYPE_UNDEFINED,DAM_PHYSICAL);
					}
					break;
				  default:
					break;				  
			  }
			  
			  
			  
			 break;
			 //Up the damage on the single target spec and keep the bleeds up.
			case 8:
			  switch (number(0, 2)){
				  
				  case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")){
						ares_bleed_ench(vict);
					}					
				  
					break;
				  case 1:					
				  case 2:
				  vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n wields his sword and slashes towards $N.",0,mob,0,vict,TO_ROOM);
					  act("A blade slashes into your flesh.",0,mob,0,vict,TO_VICT);
					  act("$N screams as the blade slashes $S flesh.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,750,TYPE_UNDEFINED,DAM_PHYSICAL);
					}
					break;
				  default:
					break;				  
			  }
			  
			  break;
			//Add a group attack with a potential for a bleed
			//30% chance to inflict bleed on everyone for each victim.
			case 7:
			  switch (number(0, 2)){
				  
				  case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")){
						ares_bleed_ench(vict);
					}					
				  
					break;
				  case 1:	
					act("$n dances around the room with his blade.",0,mob,0,0,TO_ROOM);
						for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							
							if(chance(30)){
								ares_bleed_ench(vict);
							}else {
								act("A blade barely misses your body..",0,mob,0,vict,TO_VICT);
							}
							
						}
						
						break;
				  case 2:
				  vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n wields his sword and slashes towards $N.",0,mob,0,vict,TO_ROOM);
					  act("A blade slashes into your flesh.",0,mob,0,vict,TO_VICT);
					  act("$N screams as the blade slashes $S flesh.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,850,TYPE_UNDEFINED,DAM_PHYSICAL);
					}
					break;
				  default:
					break;				  
			  }
			  break;
			//Switch to Shield Mode.
			//Sets use BIts to Track the one time shift.
			case 6:
			  if (!IS_SET(GET_BANK(mob), STATE1))
				{
					do_say(mob, "Lets see how you handle my defenses!", CMD_SAY);
					act("$n sheathes his blade and wields a large shield.",0,mob,0,0,TO_ROOM);	
					SET_BIT(GET_BANK(mob), STATE1);
					
				}
				
				switch (number(0, 2)){
				  //Shield base for damage only
				  case 0:
					vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n slames $N with his shield.",0,mob,0,vict,TO_ROOM);
					  act("A shield slams into you.",0,mob,0,vict,TO_VICT);
					  act("$N groans as a shield smashes into them.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,700,TYPE_UNDEFINED,DAM_PHYSICAL);
					}
				  
					break;
				  case 1:	
					act("$n smashes his shield into the group.",0,mob,0,0,TO_ROOM);
						for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							
							damage(ch, vict, 700, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, 1 *PULSE_VIOLENCE);
							
						}
						
						break;
				  case 2:
					  vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("$n slames $N on the foot with his shield.",0,mob,0,vict,TO_ROOM);
							act("A shield slams into your foot.",0,mob,0,vict,TO_VICT);
							act("$N yelps as their foot is crushed.",0,mob,0,vict,TO_NOTVICT);
							if (GET_OPPONENT(vict))
							{
								stop_fighting(vict);
								damage(mob,vict,400,TYPE_UNDEFINED,DAM_PHYSICAL);
							}
							
						}
					break;
				  default:
					break;				  
			  }
				
				
			  break;

			case 5:
			  switch (number(0, 2)){
				  //Shield base for damage only
				  case 0:
					vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n slames $N with his shield.",0,mob,0,vict,TO_ROOM);
					  act("A shield slams into you.",0,mob,0,vict,TO_VICT);
					  act("$N groans as a shield smashes into them.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,1000,TYPE_UNDEFINED,DAM_PHYSICAL);
					}
				  
					break;
				  case 1:	
					act("$n smashes his shield into the group.",0,mob,0,0,TO_ROOM);
						for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							
							damage(ch, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, 1 *PULSE_VIOLENCE);
							
						}
						
						break;
				  case 2:
					  vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("$n slames $N on the foot with his shield.",0,mob,0,vict,TO_ROOM);
							act("A shield slams into your foot.",0,mob,0,vict,TO_VICT);
							act("$N yelps as their foot is crushed.",0,mob,0,vict,TO_NOTVICT);
							if (GET_OPPONENT(vict))
							{
								stop_fighting(vict);
								damage(mob,vict,600,TYPE_UNDEFINED,DAM_PHYSICAL);
							}
							
						}
					break;
				  default:
					break;				  
			  }
			  break;

			case 4:
			  switch (number(0, 2)){
				  //Shield base for damage only
				  case 0:
					vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n slames $N with his shield.",0,mob,0,vict,TO_ROOM);
					  act("A shield slams into you.",0,mob,0,vict,TO_VICT);
					  act("$N groans as a shield smashes into them.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,700,TYPE_UNDEFINED,DAM_PHYSICAL);
					}
				  
					break;
				  case 1:	
					act("$n smashes his shield into the group.",0,mob,0,0,TO_ROOM);
						for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
						{
							
							next_vict = vict->next_in_room;
							if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							stun_delay = number(1, 3);
							damage(ch, vict, 700, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, stun_delay*PULSE_VIOLENCE);
							
						}
						
						break;
				  case 2:
					  vict = get_random_victim_fighting(mob);
						if (vict)
						{
							act("$n slames $N on the foot with his shield.",0,mob,0,vict,TO_ROOM);
							act("A shield slams into your foot.",0,mob,0,vict,TO_VICT);
							act("$N yelps as their foot is crushed.",0,mob,0,vict,TO_NOTVICT);
							if (GET_OPPONENT(vict))
							{
								stop_fighting(vict);
								damage(mob,vict,400,TYPE_UNDEFINED,DAM_PHYSICAL);
							}
							
						}
					break;
				  default:
					break;				  
			  }
			  break;
			//Switch to the Spear
			case 3:
				//Remove STATE1 BIT - Allow Attacks to start gain.				
				if (IS_SET(GET_BANK(mob), STATE1)) { 
					REMOVE_BIT(GET_BANK(mob), STATE1);
					do_say(mob, "Let the real dual begin!", CMD_SAY);
					act("$n drops his blade and wields a large spear.",0,mob,0,0,TO_ROOM);	
					
					mob->specials.damnodice=127;
					mob->specials.damsizedice=4;
				}
				
				switch (number(0, 2)){
				  
				  case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")){
						ares_bleed_ench(vict);
					}					
				  
					break;
				  case 1:	
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")){
						ares_bleed_ench(vict);
					}					
				  
					break;
				  case 2:
				  vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n wields his spear and Stabs towards $N.",0,mob,0,vict,TO_ROOM);
					  act("A Spear pierces into your flesh.",0,mob,0,vict,TO_VICT);
					  act("$N screams as the spear pierces $S flesh.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,1150,TYPE_UNDEFINED,DAM_PHYSICAL);
					}
					break;
				  default:
					break;				  
			  }
				
			
			  
			  break;

			case 2:
			  mob->specials.damnodice=127;
			  mob->specials.damsizedice=6;
			  
			  switch (number(0, 2)){
				  
				  case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")){
						ares_bleed_ench(vict);
					}					
				  
					break;
				  case 1:	
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")){
						ares_bleed_ench(vict);
					}					
				  
					break;
				  case 2:
				  vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n wields his spear and Stabs towards $N.",0,mob,0,vict,TO_ROOM);
					  act("A Spear pierces into your flesh.",0,mob,0,vict,TO_VICT);
					  act("$N screams as the spear pierces $S flesh.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,1150,TYPE_UNDEFINED,DAM_PHYSICAL);
					}
					break;
				  default:
					break;				  
			  }
			  
			  break;

			case 1:
			  
			  switch (number(0, 2)){
				  
				  case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")){
						ares_bleed_ench(vict);
					}					
				  
					break;
				  case 1:	
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")){
						ares_bleed_ench(vict);
					}					
				  
					break;
				  case 2:
				  vict = get_random_victim_fighting(mob);
					if (vict)
					{
					  act("$n wields his spear and Stabs towards $N.",0,mob,0,vict,TO_ROOM);
					  act("A Spear pierces into your flesh.",0,mob,0,vict,TO_VICT);
					  act("$N screams as the spear pierces $S flesh.",0,mob,0,vict,TO_NOTVICT);
					  damage(mob,vict,1150,TYPE_UNDEFINED,DAM_PHYSICAL);
					}
					break;
				  default:
					break;				  
			  }
			  
			  mob->specials.damnodice=127;
			  mob->specials.damsizedice=8;
			  break;
		  }
	  }
		
	case CMD_CIRCLE:
       if (!ch) return FALSE;   
		//We only want to trigger this if he is in Phase 1 with the shield.
       if (!IS_SET(GET_BANK(mob), STATE1)) { return FALSE;}
	   act("You need to fight with real combat skills. ", 0, mob, 0, 0, TO_ROOM);			
		return TRUE;
	   
       break;
	   case CMD_PUMMEL:
		if (!ch) return FALSE;   
		//We only want to trigger this if he is in Phase 1 with the shield.
		if (!IS_SET(GET_BANK(mob), STATE1)){ return FALSE;}
		   act("You need to fight with real combat skills. ", 0, mob, 0, 0, TO_ROOM);			
			return TRUE;

		break;


        case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain. Olympus Seeks Revenge!\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if (IS_NPC(vict) || !IS_MORTAL(vict)) continue;
				sprintf(buf, "You are awarded with %d quest %s for the kill.\n\r", reward, reward> 1 ? "points" : "point");
				send_to_char(buf, vict);
				vict->ver3.quest_points += reward;
			}

			break;

     	//can add an else branch here if you want them to act but not in combat.

      
   
  }

  return FALSE;
}


int olympus_apollo(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
  
  int stun_delay;

  //Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1) return FALSE;

  char *apollo_speak[6] = { "Art and war share a common rhythm.","The sun does not shine on fools.","","","","" };
  
  char *apollo_combat_speak[5] = { "The sun's light shall burn you away","Face the precision of a god’s bow","Each arrow finds its mark; each strike seals your fate!","The sun rises only for the worthy!","Let my music and arrows be your last memory!"};
  
  
  switch (cmd)
  {
    case MSG_MOBACT:

     	//Have him to chat to you.
      if (chance(35) && (!mob->specials.fighting))
      {
        sprintf(buf, "%s", apollo_speak[number(0, NUMELEMS(apollo_speak) - 1)]);

        do_say(mob, buf, CMD_SAY);
      }

     	//if fighting - spec different attacks
      if (mob->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.

		  if (chance(35))
		  {
			sprintf(buf, "%s", apollo_combat_speak[number(0, NUMELEMS(apollo_combat_speak) - 1)]);

			do_say(mob, buf, CMD_SAY);
		  }


        switch (number(0, 5))
        {
          case 0:
            stun_delay = number(1, 3);
			act("$n shouts an oppressive order, causing your head to ache and your mind to wander.",0,mob,0,0,TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
				damage(mob,vict,600,TYPE_UNDEFINED,DAM_PHYSICAL);
				WAIT_STATE(vict,PULSE_VIOLENCE*stun_delay);
            }
            break;
          case 1:
            break;
          case 2:
            vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n wields his sword and advances towards the enemy.",0,mob,0,0,TO_ROOM);
			  act("A fine steel blade stabs into your chest.",0,mob,0,vict,TO_VICT);
			  act("$N screams as the blade pierces $S chest.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,1450,TYPE_UNDEFINED,DAM_PHYSICAL);
			}
			break;
		  case 3:
			break;
		  case 4:
			break;
		  case 5:
			vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n wields his sword and jabs towards the enemy.",0,mob,0,0,TO_ROOM);
			  act("A fine steel blade jabs into your chest.",0,mob,0,vict,TO_VICT);
			  act("$N screams as the blade jabs $S chest.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,850,TYPE_UNDEFINED,DAM_PHYSICAL);
			}
			break;		  
          default:
            break;
        }
      }

     	//can add an else branch here if you want them to act but not in combat.

      break;
   
  }

  return FALSE;
}

int olympus_artemis(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  CHAR *vict, *next_vict;
  
  int stun_delay;

  //Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1) return FALSE;

  char *artemis_speak[6] = { "The wilds are my domain, and you are trespassing.","Hunted or hunter, the choice was yours.","","","","" };
  
  char *artemis_combat_speak[5] = { "The huntress never misses her prey!","You will fall like a wounded stag!","The wilds themselves fight by my side!","Run if you can; the hunt always ends the same.","Your arrogance blinds you to the predator before you."};
  
  
  switch (cmd)
  {
    case MSG_MOBACT:

     	//Have him to chat to you.
      if (chance(35) && (!mob->specials.fighting))
      {
        sprintf(buf, "%s", artemis_speak[number(0, NUMELEMS(artemis_speak) - 1)]);

        do_say(mob, buf, CMD_SAY);
      }

     	//if fighting - spec different attacks
      if (mob->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.

		  if (chance(35))
		  {
			sprintf(buf, "%s", artemis_combat_speak[number(0, NUMELEMS(artemis_combat_speak) - 1)]);

			do_say(mob, buf, CMD_SAY);
		  }


        switch (number(0, 5))
        {
          case 0:
            stun_delay = number(1, 3);
			act("$n shouts an oppressive order, causing your head to ache and your mind to wander.",0,mob,0,0,TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
				damage(mob,vict,600,TYPE_UNDEFINED,DAM_PHYSICAL);
				WAIT_STATE(vict,PULSE_VIOLENCE*stun_delay);
            }
            break;
          case 1:
            break;
          case 2:
            vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n wields his sword and advances towards the enemy.",0,mob,0,0,TO_ROOM);
			  act("A fine steel blade stabs into your chest.",0,mob,0,vict,TO_VICT);
			  act("$N screams as the blade pierces $S chest.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,1450,TYPE_UNDEFINED,DAM_PHYSICAL);
			}
			break;
		  case 3:
			break;
		  case 4:
			break;
		  case 5:
			vict = get_random_victim_fighting(mob);
			if (vict)
			{
			  act("$n wields his sword and jabs towards the enemy.",0,mob,0,0,TO_ROOM);
			  act("A fine steel blade jabs into your chest.",0,mob,0,vict,TO_VICT);
			  act("$N screams as the blade jabs $S chest.",0,mob,0,vict,TO_NOTVICT);
			  damage(mob,vict,850,TYPE_UNDEFINED,DAM_PHYSICAL);
			}
			break;		  
          default:
            break;
        }
      }

     	//can add an else branch here if you want them to act but not in combat.

      break;
   
  }

  return FALSE;
}

/*
Check the state of the forge.
Increment by 1 for each state that exists.


*/

int get_forge_state(CHAR *mob){
	
	int state = 0;
	
	if(IS_SET(GET_BANK(mob), STATE1)){
		
		state++;
	}
	if(IS_SET(GET_BANK(mob), STATE2)){
		
		state++;
	}
	if(IS_SET(GET_BANK(mob), STATE3)){
		
		state++;
	}
	if(IS_SET(GET_BANK(mob), STATE4)){
		
		state++;
	}
	if(IS_SET(GET_BANK(mob), STATE5)){
		
		state++;
	}

	return state;
}
// Increase the state of Forge by 1
//If 1 bit is set, exit the function.
void increment_forge_state(CHAR *mob){
	
	bool cont = TRUE;
	
	if(!IS_SET(GET_BANK(mob), STATE1) && cont){
		
		SET_BIT(GET_BANK(mob), STATE1);
		cont = FALSE;
	}
	if(!IS_SET(GET_BANK(mob), STATE2) && cont){
		
		SET_BIT(GET_BANK(mob), STATE2);
		cont = FALSE;
	}
	if(!IS_SET(GET_BANK(mob), STATE3) && cont){
		
		SET_BIT(GET_BANK(mob), STATE3);
		cont = FALSE;
	}
	if(!IS_SET(GET_BANK(mob), STATE4) && cont){
		
		SET_BIT(GET_BANK(mob), STATE4);
		cont = FALSE;
	}
	if(!IS_SET(GET_BANK(mob), STATE5) && cont){
		
		SET_BIT(GET_BANK(mob), STATE5);
		cont = FALSE;
	}
	
	
	
}

//Decrement the state of the forge by 1
//Once you have removed all 5, remove STATE6, so it can start over.
void decrement_forge_state(CHAR *mob){
	
	bool cont = TRUE;
	
	
	
	if(IS_SET(GET_BANK(mob), STATE1) && cont){
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);	
		REMOVE_BIT(GET_BANK(mob), STATE1);
		cont = FALSE;
	}
	if(IS_SET(GET_BANK(mob), STATE2) && cont){
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);	
		REMOVE_BIT(GET_BANK(mob), STATE2);
		cont = FALSE;
	}
	if(IS_SET(GET_BANK(mob), STATE3) && cont){
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);	
		REMOVE_BIT(GET_BANK(mob), STATE3);
		cont = FALSE;
	}
	if(IS_SET(GET_BANK(mob), STATE4) && cont){
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);	
		REMOVE_BIT(GET_BANK(mob), STATE4);
		cont = FALSE;
	}
	if(IS_SET(GET_BANK(mob), STATE5) && cont){
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);	
		REMOVE_BIT(GET_BANK(mob), STATE5);
		cont = FALSE;
	}
	
	//If you have unset all 5 flags and cont is true, then lets reset STATE 6
	//This will allow the forge to start charging again.
	
	if(cont){
		REMOVE_BIT(GET_BANK(mob), STATE6);
		act("The Forge has become cold and lifeless.",0,mob,0,0,TO_ROOM);	
	}
	
	
	
}

int olympus_hephaestus(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];
  //CHAR *vict, *next_vict;
  
  //int stun_delay;
  int forge_state=0;

  //Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(mob, TRUE) < 1) return FALSE;

  char *hephaestus_speak[6] = { "Even gods need the tools I create.","I craft destiny with my hammer and anvil.","","","","" };
  
  char *hephaestus_combat_speak[5] = { "Behold the strength of a god who builds and destroys!","Your armor will crumble under my strike!","My hammer forges not only weapons but destruction!","The flames of creation become your undoing!","Feel the weight of my forge's fury!"};
  
  //char *missles[5] = {"Metal Tongs","A dense hammer","A sharp chisel","A flat file","A giant chunk of coal"};
  
  switch (cmd)
  {
    case MSG_MOBACT:

     	//Have him to chat to you.
      if (chance(35) && (!mob->specials.fighting))
      {
        sprintf(buf, "%s", hephaestus_speak[number(0, NUMELEMS(hephaestus_speak) - 1)]);

        do_say(mob, buf, CMD_SAY);
      }

     	//if fighting - spec different attacks
      if (mob->specials.fighting)
      {
       	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
       	//Each Case statement that has an action needs to break out at the end.

		  if (chance(35))
		  {
			sprintf(buf, "%s", hephaestus_combat_speak[number(0, NUMELEMS(hephaestus_combat_speak) - 1)]);

			do_say(mob, buf, CMD_SAY);
		  }

		
		
		
        switch (number(0, 7))
        {
          case 0: //Infuse the Forge.
          case 1:
          case 2:
		  case 3:
		  case 4:
		  case 5:		
		  case 6:		
		  case 7: // Increment the Forge	
					if(!IS_SET(GET_BANK(mob), STATE6)){
					  act("$n infuses the forge with magic.",0,mob,0,0,TO_ROOM);					
					  increment_forge_state(mob);
					}
			break;
          default:
            break;
        }
		//Let the party know the state of the forge.
		//State 6 means the spec fired, dont warn if this is set.
		
		forge_state = get_forge_state(mob);
		
		if(forge_state == 0 && !IS_SET(GET_BANK(mob), STATE6)){
			send_to_room("The Forge stands cold and lifeless, its hearth devoid of any warmth.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
		}
		else if(forge_state == 1 && !IS_SET(GET_BANK(mob), STATE6)){
			send_to_room("The Forge shows a slight flicker of life.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
		}
		else if(forge_state == 2 && !IS_SET(GET_BANK(mob), STATE6)){			
			send_to_room("The Forge starts to radiate heat as the flames grow steadily.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
		}
		else if(forge_state == 3 && !IS_SET(GET_BANK(mob), STATE6)){			
			send_to_room("The Forge roars to life, its core glowing brightly as intense heat fills the area.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
		}
		else if(forge_state == 4 && !IS_SET(GET_BANK(mob), STATE6)){			
			send_to_room("The Forge maintains a consistent, powerful blaze, ready for any blacksmithing endeavor.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
		}
		else if(forge_state == 5 && !IS_SET(GET_BANK(mob), STATE6)){
			act("The forge explodes into the room.",0,mob,0,0,TO_ROOM);	
			
			//Set State 6 to start the cooldown phase.
			SET_BIT(GET_BANK(mob), STATE6);
			
			
			//TODO: Add Full Room Damage Spec Here.
		
		}else if(IS_SET(GET_BANK(mob), STATE6)){
			decrement_forge_state(mob);
		}
		
		
		
		
		
		
		
		
		
		
      }

     	//can add an else branch here if you want them to act but not in combat.

      break;
   
  }

  return FALSE;
}




//Assign Spec for the zone. Sets all other specs. 
//First Param - Object, Room or Mob Number.  Define it up above.
//Second Param - the name of the function that is for the mob Usually a shorthand for your zone. 
void assign_olympus(void)
{
	/*Objects */

	/*Rooms */
	assign_room(WATERFALL_ROOM, olympus_waterfallroom);

	/*Mobs */

	assign_mob(OLYMPUS_TYPHON, olympus_typhon);
	assign_mob(OLYMPUS_HERA, olympus_hera);
	assign_mob(OLYMPUS_POSEIDON, olympus_poseidon);
	assign_mob(OLYMPUS_ARES, olympus_ares);
	assign_mob(OLYMPUS_HEPHAESTUS, olympus_hephaestus);


}

/*



Hemp (gossip) [ Cronus loads: bit of cloud (key) ]


Hemp (gossip) [ Icarus loads: waxy feather (key) ]


Hemp (gossip) [ Pluto loads: rusty nail (key) ]



*/
