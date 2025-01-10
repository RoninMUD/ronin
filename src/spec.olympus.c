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






/*Mobs */
#define OLYMPUS_TYPHON 28830


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


int olympus_typhon(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict, *vict2, *vict3, *main_vict;

	int round_counter = 0; // Tracks the number of rounds in combat
    int current_phase = 1; // 0: Poison, 1: Ice, 2: Fire
    
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
						//One of his legs bites the tank.   //Remove 25% HP from the Buffer.
						//Heal for 10% of the Buffers Max HP.
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
						}
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
						damage(mob,vict,888,TYPE_UNDEFINED,DAM_COLD);
						
						
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
	//assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);
	

	/*Rooms */
	//assign_room(LIGHTWALL,             	cl_LightWallLink);

	/*Mobs */

	assign_mob(OLYMPUS_TYPHON, olympus_typhon);


}

/*


1. Typhon (Endgame Boss - Damage Check & Strategy)
Mechanics:
Elemental Chaos:
Typhon alternates between fire, poison, and ice attacks every 3 turns. Players must use the defend or counter commands with the correct elemental resistance to reduce damage.
Fire: Use cool command or fire-resistant gear.
Poison: Use antidotes or cleanse.
Ice: Use warm or spells to mitigate freezing effects.
Ground Shatter:
Every 5 turns, Typhon deals AoE damage that bypasses defenses unless players activate a brace command. Missing the brace results in a health loss equal to 20% of max HP.
Weak Spots:
Players must target head1, target head2, etc., to deal effective damage. Focusing on the same head for too long triggers a counterattack.
Win Condition: Destroy all heads, then survive 3 rounds of his rage phase to deliver the killing blow.

2. Zeus (Timed Burst Phases)
Mechanics:
Thunderbolt Barrage:
Every 3 turns, Zeus randomly targets one player with a high-damage lightning strike. Players can dodge (agility check) or reflect with a magical shield.
Storm Clouds:
Zeus periodically reduces visibility by summoning clouds. Players must locate Zeus (perception check) to ensure their attacks hit.
Wrath of Olympus:
A scripted, timed event where Zeus charges an attack that deals massive damage. Players must perform combined actions like shield ally or interrupt Zeus (based on team coordination and cooldowns) to survive.
Win Condition: Reduce Zeus' health below 20% to stop his wrath phase and trigger his surrender.

3. Hera (Curses and Minion Waves)
Mechanics:
Summoning Minions:
Hera starts by summoning waves of mythical beasts (e.g., lion, eagle) that must be killed before players can attack Hera. Each beast has a unique weakness (strike, shoot, or magic) that players must exploit.
Curses:
Hera curses players randomly with status effects:
Slowness: Reduces action speed unless cleanse curse.
Weakness: Reduces damage output; requires a stamina potion or priest intervention.
Confusion: Forces random actions unless cured by another player.
Divine Wrath:
Targets the most aggressive player, dealing high damage. Players can transfer threat to share damage or protect teammates.
Win Condition: Manage curses effectively, defeat her minions, and reduce her health to 0.

4. Poseidon (AoE and Summons)
Mechanics:
Tidal Fury:
Deals heavy AoE damage every 3 turns. Players must prepare (requires foresight or warning) to halve damage or ward with water-resistant items.
Whirlpool Trap:
Targets one player per turn with a trap that immobilizes them for 2 turns unless they succeed in a strength or magic check. Other players can rescue ally.
Leviathan Summon:
Summons a sea monster as a secondary boss. Players must decide whether to focus Leviathan or focus Poseidon. Ignoring Leviathan increases damage taken.
Win Condition: Outlast tidal fury and kill Poseidon before Leviathan becomes overwhelming.

5. Apollo (Skill Check and Debuffs)
Mechanics:
Solar Flare:
Blinds players for 2 turns, reducing accuracy unless they close eyes or equip protective items.
Golden Arrow Volley:
Fires a volley of arrows that hits all players. Players can block (shield), evade (dexterity check), or absorb with magical wards.
Harmonic Disruption:
Disrupts spellcasters by increasing cooldowns and interrupting actions. Non-casters must silence Apollo (requires timing) to stop this.
Win Condition: Use coordinated actions to overcome disruptions and land enough hits to defeat him.

6. Artemis (Precision Combat)
Mechanics:
Moonlit Arrows:
Artemis fires powerful arrows at players. Players can dodge (agility check) or parry with a weapon. Each miss reduces Artemis’ focus for a turn.
Beast Call:
Summons wolves or bears to attack the group. Players must prioritize killing beasts or risk being overwhelmed.
Silent Stalker:
Goes invisible for 2 turns and deals a critical hit to one random player. Players can use perception or area attack to counter her stealth.
Win Condition: Deplete her health while managing beast summons.

7. Hephaestus (Durability Test)
Mechanics:
Infernal Hammer:
Deals massive single-target damage every 3 turns. Players must block, brace, or disarm Hephaestus (strength check) to mitigate damage.
Flame Turrets:
Turrets activate every 4 turns, dealing AoE fire damage. Players can disable turret (intelligence check) or use water-based items to reduce damage.
Mechanical Constructs:
Summons automatons to protect himself. Automatons can be hacked (intelligence) or destroyed through brute force.
Win Condition: Disable turrets, destroy constructs, and deplete Hephaestus' health.
MUD-Specific Implementation Notes:
Combat Logs: Each ability should display vivid descriptions in combat logs (e.g., “Typhon's fire breath engulfs the room in a wave of searing heat!”).
Commands: Ensure abilities are accessible with clear commands (dodge, block, counter, etc.).
Cooldowns and Resources: Introduce cooldowns and resource management (e.g., stamina, mana) for players to make decisions matter.
Team Coordination: Fights should reward team synergy (e.g., sharing threat, assisting cursed allies).
Dynamic Messaging: Messages should adapt based on success or failure of player actions (e.g., "Your shield absorbs the impact!" vs. "The blast breaks through, dealing 50 damage.").
Would you like detailed pseudo-code or specific mechanics for any particular MUD framework?




*/