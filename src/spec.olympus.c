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
#define ZEUS_ROOM 28852
#define LIBRARY_ROOM 28877
#define TUNNEL_ROOM 28876
#define IMMORTAL_VILLAGE_ROOM 28726

#define SEAL_ROOM_ONE 28801
#define SEAL_ROOM_TWO 28802
#define SEAL_ROOM_THREE 28803

#define TYPHON_ROOM 28770



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

#define OLYMPUS_TYPHON_SEALED 28701
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

#define TYPHON_ICE_ENCH_NAME "Encased in Ice"
#define TYPHON_FIRE_ENCH_NAME "On Fire!"


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
			if(V_OBJ(obj)== LARGE_ROCK) {
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

bool isSealRoomsOccupied()
{
	//check all 3 rooms.  All 3 need a person in it to activate.

	if (count_mortals_real_room(real_room(SEAL_ROOM_ONE)) < 1 ||
	        count_mortals_real_room(real_room(SEAL_ROOM_TWO)) < 1 ||
	        count_mortals_real_room(real_room(SEAL_ROOM_THREE)) < 1
	   )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

int olympus_typhonroom(int room, CHAR *ch, int cmd, char *arg)
{
	bool sealRoomsOccupied = FALSE;
	char buf[MAX_STRING_LENGTH];
	CHAR* zeus;
	CHAR* typhon_sealed;
	CHAR* typhon;
	CHAR *vict;
	int typhon_nr;


	//When the zone is reset, remove the Olympus Version of Typhon
	if (cmd == MSG_ZONE_RESET)
	{

		for (vict = world[real_room(TYPHON_ROOM)].people; vict; vict = vict->next_in_room)
		{
			if(!IS_NPC(vict) || (V_MOB(vict)!=OLYMPUS_TYPHON)) continue;
			//If Typhon is found and the seals are occupied then we want to remove the sealed version and summon the real.
			if(V_MOB(vict)==OLYMPUS_TYPHON) {

				typhon= get_ch_world(OLYMPUS_TYPHON);

				act("The seals around Typhon reform",0,0,0,0,TO_ROOM);

				char_from_room(typhon);
			}
		}

	}

	sealRoomsOccupied = isSealRoomsOccupied();
	//If the room is occupied - Lets spawn in the proper typhon
	//Lets also let everyone one.
	if (sealRoomsOccupied)
	{

		for (vict = world[real_room(TYPHON_ROOM)].people; vict; vict = vict->next_in_room)
		{
			if(!IS_NPC(vict) || (V_MOB(vict)!=OLYMPUS_TYPHON_SEALED)) continue;
			//If Typhon is found and the seals are occupied then we want to remove the sealed version and summon the real.
			if(V_MOB(vict)==OLYMPUS_TYPHON_SEALED) {

				typhon_sealed = get_ch_world(OLYMPUS_TYPHON_SEALED);
				typhon_nr = real_mobile(OLYMPUS_TYPHON);
				typhon = read_mobile(typhon_nr, REAL);

				char_from_room(typhon_sealed);
				char_to_room(typhon,real_room(TYPHON_ROOM));

				act("The seals around typhon break and shatter",0,typhon,0,0,TO_ROOM);
				act("Foolish Mortals - Prepare to feel my wrath.",0,typhon,0,0,TO_ROOM);

				zeus = get_ch_world(OLYMPUS_ZEUS);
				// He should exist now
				if (zeus)
				{
					sprintf(buf, "The seals have been broken! Olympus goes to war!");
					do_quest(zeus, buf, CMD_QUEST);
				}

			}
		}

	}

	return FALSE;
}


int typhon_poison_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {

	int hp_loss;


	if(cmd==MSG_REMOVE_ENCH) {
		send_to_char("Your blood stops burning.\n\r",ench_ch);
		return FALSE;
	}
	if(cmd==CMD_QUIT) {
		if(ch!=ench_ch) return FALSE;
		send_to_char("You have lost too much blood to quit.\n\r",ench_ch);
		return TRUE;
	}

	//On Tick, lose 10% of your HP, but cant drop below 100 HP.

	if(ench_ch && cmd==MSG_TICK) {

		hp_loss = 200;

		if ((GET_HIT(ench_ch) - hp_loss) <= 100) {
			GET_HIT(ench_ch) = 100;
		} else
		{
			GET_HIT(ench_ch) -= hp_loss;
		}


	}


	return FALSE;

}


void typhon_poison_ench(CHAR *vict) {
	char buf4[MIL];
	ENCH *tmp_enchantment;

	CREATE(tmp_enchantment, ENCH, 1);
	tmp_enchantment->name     = str_dup("Burning Blood");
	tmp_enchantment->duration = 2;
	tmp_enchantment->func     = typhon_poison_func;
	enchantment_to_char(vict, tmp_enchantment, FALSE);
	act("$n has been biten by a snake.\n\r",FALSE,vict,0,0,TO_ROOM);
	send_to_char("You feel the snake's fangs plunge into your arm.\n\r",vict);
	sprintf(buf4,"Olympus Log Ench: [ %s just contracted Burning Blood at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
	log_s(buf4);


}


int typhon_ice_pillar_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n thaws out as the pillar melts around them.",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your body finally thaws as your limbs are free.\n\r",ench_ch);
    return FALSE;
  }

  if(cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your frozen body cant move.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n stands encased in ice.",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }

           /* The only thing you can do is look. Else, it will stop you from doing anything else. */

  if(cmd!=CMD_LOOK) {
    if(ch!=ench_ch) return FALSE;    
    send_to_char("You can't do that right now, you're frozen in a pillar of ice!\n\r",ench_ch);
    return TRUE;
  } else {    
    return FALSE;
  }
  return FALSE;
}

int typhon_fire_pillar_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n is relieved as the fire burns out..",TRUE,ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("The flames on your body fizzle out.\n\r",ench_ch);
    return FALSE;
  }

  if(cmd==CMD_QUIT) {
    if(ch!=ench_ch) return FALSE;
    send_to_char("Your frozen body cant move.\n\r",ench_ch);
    return TRUE;
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n is engulfed in fire.",FALSE, ench_ch,0,ch,TO_VICT);
    return FALSE;
  }


  if(cmd==CMD_CAST || cmd ==CMD_SONG){
	if(ch!=ench_ch) return FALSE;    
  if(chance(50)){
	
	send_to_char("The flames make it hard to cocentrate.\n\r",ench_ch);
    return TRUE;
	
  }else {
	  return FALSE;
  }
	  
  }

           /* The only thing you can do is look. Else, it will stop you from doing anything else. */

  if(cmd!=CMD_LOOK) {
    if(ch!=ench_ch) return FALSE;    
    send_to_char("You can't do that right now, you're frozen in a pillar of ice!\n\r",ench_ch);
    return TRUE;
  } else {    
    return FALSE;
  }
  return FALSE;
}




int olympus_typhon(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict, *vict1, *vict2, *vict3, *main_vict;


	static int current_phase; // 0: Poison, 1: Ice, 2: Fire
	static int next_phase = 0;
	struct affected_type_5 af;


	int reward = 10;
	int HP_to_Heal=0;
	int mortal_count;


	char *typhon_speak[6] = { "My wrath is eternal; none shall escape!","Tremble, mortals, before my unyielding might!","The earth trembles at my presence; the skies weep with fear.","No prison can hold me forever, and no hero shall survive my wrath.","Soon, all of Olympus will fall, and chaos will reign supreme!","Do you feel it? The end approaches, carried on the winds of despair." };

	char *typhon_combat_poison_speak[3] = { "Feel the sting of my venom coursing through your veins, weakling!","My toxic breath will be your undoing; succumb to the poison!","You cannot escape the serpents' bite; their venom will claim you!"};
	char *typhon_combat_fire_speak[3] = { "Burn! Feel the searing heat of my flames consume you!","My fire will reduce you to ashes; nothing will remain of your pitiful existence!","The inferno beckons; embrace the flames and meet your end!"};
	char *typhon_combat_ice_speak[3] = { "Shiver before my icy wrath; your blood will freeze in your veins!","The cold will claim you; feel your strength ebb away with each frostbitten breath!","My blizzard will bury you; there is no escape from the chill of death!"};


	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	switch (cmd)
	{

	case MSG_TICK:
		next_phase = number(0, 2);
		break;

	case CMD_NORTH:
		act("$n says 'You're not getting past me.'",0,mob,0,0,TO_ROOM);
		return TRUE;

		break;

	case MSG_MOBACT:

		//Have him to chat to you.
		if (chance(40))
		{
			sprintf(buf, "%s", typhon_speak[number(0, NUMELEMS(typhon_speak) - 1)]);

			do_say(mob, buf, CMD_SAY);
		}



		//if fighting - spec different attacks
		if (mob->specials.fighting)
		{

			if(!current_phase) {
				current_phase = number(0,2);
			}

			//Every Tick, check to see if the phase changes - If the next phase isnt the same, show the transformation message.
			if(next_phase != current_phase) {
				current_phase = next_phase;
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
				if (chance(35))
				{
					sprintf(buf, "%s", typhon_combat_poison_speak[number(0, NUMELEMS(typhon_combat_poison_speak) - 1)]);

					do_say(mob, buf, CMD_SAY);
				}


				switch (number(0, 10)) {
				case 0:					
				case 1:
				for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
					{
						next_vict = vict->next_in_room;

						if (IS_NPC(vict)) return FALSE;
						if (IS_MORTAL(vict))
						{

							if(chance(55)) {

								typhon_poison_ench(vict);

							}
						}
					}
					break;
				case 2:
				case 3:
				case 4:

					//Poison everyone in the room,
					act("A giant snake on $n's Leg opens his mouth and breaths out smoke.",0,mob,0,0,TO_ROOM);

					for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
					{
						next_vict = vict->next_in_room;

						if (IS_NPC(vict)) return FALSE;
						if (IS_MORTAL(vict))
						{

							if(chance(40)) {

								if(!affected_by_spell( vict, SPELL_BLINDNESS )) {
									af.type = SPELL_BLINDNESS;
									af.duration = 1;
									af.location = APPLY_STR;
									af.modifier = -5;
									af.bitvector  = AFF_BLIND;
									af.bitvector2 = 0;
									affect_to_char(vict, &af);
								}

							}
						}
					}

					break;
				case 6:
					//Target the Tank
					vict=mob->specials.fighting;
					act("A giant snake violently bites $N who screams in agony.",1, mob, 0,vict,  TO_ROOM);
					act("A giant snake violently bites you and you scream in agony.",1, mob, 0,vict,  TO_VICT);
					damage(mob, vict, 1500, TYPE_UNDEFINED,DAM_PHYSICAL);
					break;
				case 7:
				case 8:
				case 9:
				case 10:
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

				default:
					break;
				}
				break;
			case 1: // Ice Phase

				if (chance(35))
				{
					sprintf(buf, "%s", typhon_combat_ice_speak[number(0, NUMELEMS(typhon_combat_ice_speak) - 1)]);

					do_say(mob, buf, CMD_SAY);
				}
				
						
				switch (number(0, 10)) {


				case 0: //-- Freeze Random Characters - 2 Round Stun				
				case 1: //Ice Blast - 3 random characters
					vict = get_random_victim_fighting(mob);
					if (IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
					act("You are trapped in a pillar of ice.", 0, mob, 0, vict, TO_VICT);
					act("$n traps $N in a pillar of ice.", 0, mob, 0, vict, TO_NOTVICT);
					enchantment_apply(vict, TRUE, TYPHON_ICE_ENCH_NAME, TYPE_UNDEFINED, 5, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, typhon_ice_pillar_func);
					break;
				case 2:
				case 3:
				case 4:
				case 5:

					mortal_count = count_mortals_room(mob, TRUE);
					if (mortal_count == 1){
						
						vict1=get_random_victim(mob);
						if (IS_NPC(vict1) || !(IS_MORTAL(vict1))) return FALSE;				
						act("You are foolish to take me on alone!.\r\n",0,mob,0,0,TO_ROOM);
						act("$n throws a large shard of ice at you.", 0, mob, 0, vict1, TO_VICT);
						act("$n throws a large shard of ice at $N.", 0, mob, 0, vict1, TO_NOTVICT);
						damage(mob,vict1,number(500,620),TYPE_UNDEFINED,DAM_COLD);
						WAIT_STATE(vict1,PULSE_VIOLENCE*1);
						
					}
					else if (mortal_count == 2){
						vict1=get_random_victim(mob);
						if (IS_NPC(vict1) || !(IS_MORTAL(vict1))) return FALSE;				
						act("One of you will face my wrath!\r\n",0,mob,0,0,TO_ROOM);
						act("$n throws a large shard of ice at you.", 0, mob, 0, vict1, TO_VICT);
						act("$n throws a large shard of ice at $N.", 0, mob, 0, vict1, TO_NOTVICT);
						damage(mob,vict1,number(500,620),TYPE_UNDEFINED,DAM_COLD);
						WAIT_STATE(vict1,PULSE_VIOLENCE*1);
						
						
					}
					else {
					
						vict1=get_random_victim(mob);
						vict2=get_random_victim(mob);
						vict3=get_random_victim(mob);

						int failcount = 0;
						while(failcount < 10 && ((vict2==vict1) || (vict3==vict1) || (vict3==vict2)))
						{
						  
						  if(vict2==vict1){
							vict2=get_random_victim(mob);
						  }
						  else if(vict3==vict1){
							vict3=get_random_victim(mob);  
						  }
						  else if(vict3==vict2){
							  vict3=get_random_victim(mob);  
						  }  
						  
						  failcount++;
						}
						
						if(failcount <10){
							//Loop through the room and find the 3 Victims
							for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
							{
								next_vict = vict->next_in_room;
								if(IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
								
								if(vict==vict1 || vict==vict2 ||vict==vict3){								
									act("$n throws a large shard of ice at $N.", 0, mob, 0, vict, TO_NOTVICT);
									act("$n throws a large shard of ice at you.", 0, mob, 0, vict, TO_VICT);
									damage(mob,vict1,number(500,620),TYPE_UNDEFINED,DAM_COLD);
									WAIT_STATE(vict1,PULSE_VIOLENCE*1);
								}
							}
							
							
						}
					}
						
					break;
				case 6: //-- Blizzard - Ice Damage to everyone
				case 7:
				case 8:
				case 9:
				case 10:

					act("$n shoots large icicles at everyone in the room.",0,mob,0,0,TO_ROOM);
					for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
					{
						next_vict = vict->next_in_room;
						if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
						damage(mob,vict,number(400,900),TYPE_UNDEFINED,DAM_COLD);
						act("$n throws a large icicle at you.", 0, mob, 0, vict, TO_VICT);
						if(chance(20)){
							WAIT_STATE(vict,PULSE_VIOLENCE*1);
						}
					}
					break;

					
				default:
					break;
				}
				break;

			case 2: // Fire Phase

				if (chance(35))
				{
					sprintf(buf, "%s", typhon_combat_fire_speak[number(0, NUMELEMS(typhon_combat_fire_speak) - 1)]);

					do_say(mob, buf, CMD_SAY);
				}

				
				switch (number(0, 2)) {
				case 0: 
				case 1: 
				
				vict = get_random_victim_fighting(mob);
				if (IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
				
				
				act("$n summons an volcanic eruption into the room.",0,mob,0,0,TO_ROOM);
				act("$n is pushed out of the room by a wave of water.!",0,vict,0,0,TO_ROOM);
				act("You are hit by a wall of water.",0,vict,0,0,TO_CHAR);
				
				if(vict){
					char_from_room(vict);
					char_to_room(vict,real_room(IMMORTAL_VILLAGE_ROOM));
					do_look(vict,"",CMD_LOOK);
					act("$n appears covered in glowing rocks and steam.", FALSE, vict, 0, 0, TO_ROOM);
					return TRUE;
				}				
				
				break;
				case 2:
				case 3:
				case 4:
				case 5:
					vict = get_random_victim_fighting(mob);
					if (IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
					act("You are trapped in a pillar of ice.", 0, mob, 0, vict, TO_VICT);
					act("$n traps $N in a pillar of ice.", 0, mob, 0, vict, TO_NOTVICT);
					enchantment_apply(vict, TRUE, TYPHON_FIRE_ENCH_NAME, TYPE_UNDEFINED, 5, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, typhon_fire_pillar_func);
					break;
					

					break;
				case 6: 
				case 7:
				case 8:
				case 9:
				case 10:
				

					main_vict = get_random_victim_fighting(mob);

					act("$N is engulfed by a large jet of fire.", 0, mob, 0, main_vict, TO_NOTVICT);
					act("You are engulfed by a large jet of fire.", 0, mob, 0, main_vict, TO_VICT);
					if (!(main_vict) || IS_NPC(main_vict) || !(IS_MORTAL(main_vict))) return FALSE;
					damage(mob,main_vict,888,TYPE_UNDEFINED,DAM_FIRE);


					act("The residual heat from $n's jet of fire singes your skin",0,mob,0,0,TO_ROOM);
					for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
					{
						next_vict = vict->next_in_room;
						if(!(vict) || !(main_vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
						act("You are hit by a wave of heat.", 0, mob, 0, main_vict, TO_VICT);
						damage(mob,vict,450,TYPE_UNDEFINED,DAM_FIRE);

					}

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
//Resets Zeus for Wrath of Olympus - Let it randomly build up again
void reset_zeus_wrath(CHAR *mob) {
	act("The Clouds clear up.",0,mob,0,0,TO_ROOM);

	if(IS_SET(GET_BANK(mob), STATE1)) {
		REMOVE_BIT(GET_BANK(mob), STATE1);
	}
	if(IS_SET(GET_BANK(mob), STATE2)) {
		REMOVE_BIT(GET_BANK(mob), STATE2);
	}
	if(IS_SET(GET_BANK(mob), STATE3)) {
		REMOVE_BIT(GET_BANK(mob), STATE3);
	}
	if(IS_SET(GET_BANK(mob), STATE4)) {
		REMOVE_BIT(GET_BANK(mob), STATE4);
	}
}


/*

Special Abilities:
Chain Lightning: "Feel the fury of my bolts as they leap from foe to foe!"
Judgment Blast: "Behold the power of true justice!"
*/
int olympus_zeus(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict, *vict2, *vict3;

	int stun_delay, damage_chance,dodge_chance;
	int reward = 20;

	//Define any other variables

	/*Don't waste any more CPU time if no one is in the room. */
	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	char *zeus_speak[6] = { "Bow, and you may yet live.","You are ants beneath my gaze!","The heavens bow to my will, as do all who dare oppose me.","A storm brews in my soul, waiting to be unleashed.","Mortals forget their place far too easily.","Olympus stands eternal, as does my dominion over all." };

	char *zeus_combat_speak[5] = { "Face the thunder of Olympus!","I am the storm, and you are but dust!","Kneel before the King of Gods!","My lightning shall purify your arrogance!","No mortal or god can escape my wrath!"};


	switch (cmd)
	{
	case MSG_MOBACT:

		//Have him to chat to you.
		if (chance(35) && (!mob->specials.fighting))
		{
			sprintf(buf, "%s", zeus_speak[number(0, NUMELEMS(zeus_speak) - 1)]);

			do_say(mob, buf, CMD_SAY);
		}

		//if fighting - spec different attacks
		if (mob->specials.fighting)
		{
			//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
			//Each Case statement that has an action needs to break out at the end.


			if (chance(35))
			{
				sprintf(buf, "%s", zeus_combat_speak[number(0, NUMELEMS(zeus_combat_speak) - 1)]);

				do_say(mob, buf, CMD_SAY);
			}
			//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
			//Each Case statement that has an action needs to break out at the end.

			switch (number(0, 5))
			{
			case 0: //Thunderbolt - Single Target
				vict = get_random_victim_fighting(mob);
				if (vict)
				{
					act("$n strikes you with a large thunderbolt.",0,mob,0,vict,TO_VICT);
					act("$N gasps as they are struck by a large thunderbolt.",0,mob,0,vict,TO_NOTVICT);
					damage(mob,vict,1200,TYPE_UNDEFINED,DAM_ELECTRIC);
				}
				//Casting a Thunder spells stokes the clouds, closer to wrath
				if(!IS_SET(GET_BANK(mob), STATE1)) {
					send_to_room("The sky sparks with electricity.\n\r", real_room(ZEUS_ROOM));
					SET_BIT(GET_BANK(mob), STATE1);
				}


				break;

			case 1: //Storm Clouds
				act("$n summons storm clouds over the battlefield.",0,mob,0,0,TO_ROOM);
				for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
					damage(mob,vict,1100,TYPE_UNDEFINED,DAM_MAGICAL);

				}

				if(!IS_SET(GET_BANK(mob), STATE2)) {
					send_to_room("The Clouds grow darker in the sky.\n\r", real_room(ZEUS_ROOM));
					SET_BIT(GET_BANK(mob), STATE2);
				}

				break;
			case 2:
				if(!IS_SET(GET_BANK(mob), STATE3)) {
					act("$n channels energy into the sky.",0,mob,0,0,TO_ROOM);
					SET_BIT(GET_BANK(mob), STATE3);
				}
				break;
			case 3:
				break;
			case 4:
				break;
			case 5://Chain Lightning

				vict = get_random_victim_fighting(mob);

				act("$n launches chain lightning at $N.", 0, mob, 0, vict, TO_NOTVICT);
				act("$n launches chain lightning at you.", 0, mob, 0, vict, TO_VICT);


				if (IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
				damage(mob,vict,900,TYPE_UNDEFINED,DAM_ELECTRIC);


				//Check for more than 1 person in the room.
				if (count_mortals_room(mob, TRUE) > 1)
				{
					//get 2nd victim that is still fighting
					vict2 = get_random_victim_fighting(mob);
					act("Chain lightning bounces at $N.", 0, mob, 0, vict2, TO_NOTVICT);
					act("Chain lightning bounces at you.", 0, mob, 0, vict2, TO_VICT);
					if (!(vict) || IS_NPC(vict2) || !(IS_MORTAL(vict2))) return FALSE;
					damage(mob,vict,600,TYPE_UNDEFINED,DAM_ELECTRIC);


					//3rd hop is low damage but a stun

					if (count_mortals_room(mob, TRUE) > 2)
					{
						//get 3rd victim that is still fighting
						vict3 = get_random_victim_fighting(mob);
						act("Chain lightning fizzles as it hits $N.", 0, mob, 0, vict3, TO_NOTVICT);
						act("Chain lightning fizzles as it hits you.", 0, mob, 0, vict3, TO_VICT);
						if (!(vict) ||!(vict2) || IS_NPC(vict3) || !(IS_MORTAL(vict3))) return FALSE;
						damage(mob,vict,300,TYPE_UNDEFINED,DAM_ELECTRIC);
						WAIT_STATE(vict,PULSE_VIOLENCE*1);
					}
				}
				if(!IS_SET(GET_BANK(mob), STATE4)) {
					send_to_room("Static Electricity permeates the room.\n\r", real_room(ZEUS_ROOM));
					SET_BIT(GET_BANK(mob), STATE4);
				}

				break;
			default:
				break;
			}

			//Wrath of Olympus
			//If all 4 states are set, hit the room and have a chance for them to dodge the hit, else damage and stun.



			if(IS_SET(GET_BANK(mob), STATE1) && IS_SET(GET_BANK(mob), STATE2) && IS_SET(GET_BANK(mob), STATE3) && IS_SET(GET_BANK(mob), STATE4)) {

				act("$n channels the Wrath of Olympus!.",0,mob,0,0,TO_ROOM);
				for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;

					/*
					Damage
						30% Full Damage
						30% 60% Damage
						20% 20% Damage
						20% Dodge

						If you Dodge
						60% Random Stun 1-3 Rounds
						40% Knocked out of combat and stunned for 1-3 Rounds

					*/
					damage_chance = number(0,100);
					dodge_chance = number (0,100);
					stun_delay = number(1,3);

					if(damage_chance <= 30) {
						act("You are hit by the full force of Olympus.", 0, mob, 0, vict, TO_VICT);
						damage(mob,vict,2500,TYPE_UNDEFINED,DAM_MAGICAL);
					}
					else if(damage_chance >=31 && damage_chance <=60) {
						act("You are struck by a mighty electrical force.", 0, mob, 0, vict, TO_VICT);
						damage(mob,vict,1500,TYPE_UNDEFINED,DAM_MAGICAL);
					}
					else if(damage_chance >=61 && damage_chance <=80) {
						act("Lightning streaks past your body, grazing your skin.", 0, mob, 0, vict, TO_VICT);
						damage(mob,vict,500,TYPE_UNDEFINED,DAM_MAGICAL);
					}
					else { // You Dodge the Lightning Bolt
						act("Lightning dances all around you.", 0, mob, 0, vict, TO_VICT);
						if(dodge_chance <=60) {
							act("Your limbs fizzle with electricity.", 0, mob, 0, vict, TO_VICT);
							WAIT_STATE(vict, stun_delay *PULSE_VIOLENCE);
						}
						else {
							act("You collapse to the ground, fizzling with electricity.", 0, mob, 0, vict, TO_VICT);
							WAIT_STATE(vict, stun_delay *PULSE_VIOLENCE);
							stop_fighting(vict);
						}

					}

				}
				reset_zeus_wrath(mob);

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

				if ((tch = GET_OPPONENT(mob)) && mob_proto_table[hera_summon_nr].number < 1) {

					hera_summon = read_mobile(hera_summon_nr, REAL);
					char_to_room(hera_summon, CHAR_REAL_ROOM(mob));

					if((tch = GET_OPPONENT(mob))) {

						hit(hera_summon, tch, TYPE_UNDEFINED);
					}

				}
				//Summon a 2nd Mob.  Ensure its not the same as the first.

				hera_summon_next_nr = real_mobile(hera_mobs[number(0, NUMELEMS(hera_mobs) - 1)]);
				//Keep looping if the number is the same.
				do {
					hera_summon_next_nr = real_mobile(hera_mobs[number(0, NUMELEMS(hera_mobs) - 1)]);
				} while (hera_summon_next_nr == hera_summon_nr); // Ensure a different index

				if ((tch = GET_OPPONENT(mob)) && mob_proto_table[hera_summon_next_nr].number < 1) {

					hera_summon = read_mobile(hera_summon_next_nr, REAL);
					char_to_room(hera_summon, CHAR_REAL_ROOM(mob));

					if((tch = GET_OPPONENT(mob))) {

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
		if (chance(25)) {
			act("You foolish actions will not be tolerated. ", 0, mob, 0, 0, TO_ROOM);
			hera_wrathful_rebuke(mob,ch);
			return TRUE;
		}
		break;
	case CMD_PUMMEL:
		if (!ch) return FALSE;
		if (chance(25)) {
			act("You foolish actions will not be tolerated. ", 0, mob, 0, 0, TO_ROOM);
			hera_wrathful_rebuke(mob,ch);
			return TRUE;
		}
		break;
	case CMD_CAST:
		if (!ch) return FALSE;
		if (chance(25)) {
			act("You foolish actions will not be tolerated. ", 0, mob, 0, 0, TO_ROOM);
			hera_wrathful_rebuke(mob,ch);
			return TRUE;
		}
		break;
	case CMD_SONG:
		if (!ch) return FALSE;
		if (chance(25)) {
			act("You foolish actions will not be tolerated. ", 0, mob, 0, 0, TO_ROOM);
			hera_wrathful_rebuke(mob,ch);
			return TRUE;
		}
		break;
	case CMD_DISEMBOWEL:
		if (!ch) return FALSE;
		if (chance(25)) {
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

				if ((tch = GET_OPPONENT(mob)) && mob_proto_table[poseidon_summon_nr].number < 1) {

					poseidon_summon = read_mobile(poseidon_summon_nr, REAL);
					char_to_room(poseidon_summon, CHAR_REAL_ROOM(mob));

					if((tch = GET_OPPONENT(mob))) {

						hit(poseidon_summon, tch, TYPE_UNDEFINED);
					}

				}
				//Summon a 2nd Mob.  Ensure its not the same as the first.

				poseidon_summon_next_nr = real_mobile(POSEIDON_OCTOPUS);
				//Keep looping if the number is the same.

				if ((tch = GET_OPPONENT(mob)) && mob_proto_table[poseidon_summon_next_nr].number < 1) {

					poseidon_summon = read_mobile(poseidon_summon_next_nr, REAL);
					char_to_room(poseidon_summon, CHAR_REAL_ROOM(mob));

					if((tch = GET_OPPONENT(mob))) {

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
						if(V_MOB(tmp)==octopus) {
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
						} else {
							summon_dead = TRUE;
						}


					}
				}

				if(summon_dead) {
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
						if(V_MOB(tmp)==stingray) {
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

				if(summon_dead) {
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
						if(V_OBJ(obj) == large_rock) {
							found = TRUE;
						}

					}
				}

				//Send a random Victim out of the room

				if(found == TRUE) {
					char_from_room(vict);
					char_to_room(vict,real_room(waterfall_room));
					do_look(vict,"",CMD_LOOK);
					act("$n appears soaking wet.", FALSE, vict, 0, 0, TO_ROOM);
					return TRUE;
				}
				else {
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

	if(ench_ch && cmd==MSG_TICK) {

		hp_loss = GET_MAX_HIT(ench_ch) * .1;

		if ((GET_HIT(ench_ch) - hp_loss) <= 100) {
			GET_HIT(ench_ch) = 100;
		} else
		{
			GET_HIT(ench_ch) -= hp_loss;
		}


	}


	return FALSE;

}


void ares_bleed_ench(CHAR *vict) {
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

				switch (number(0, 2)) {

				case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")) {
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
				switch (number(0, 2)) {

				case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")) {
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
				switch (number(0, 2)) {

				case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")) {
						ares_bleed_ench(vict);
					}

					break;
				case 1:
					act("$n dances around the room with his blade.",0,mob,0,0,TO_ROOM);
					for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
					{
						next_vict = vict->next_in_room;
						if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;

						if(chance(30)) {
							ares_bleed_ench(vict);
						} else {
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

				switch (number(0, 2)) {
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
				switch (number(0, 2)) {
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
				switch (number(0, 2)) {
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

				switch (number(0, 2)) {

				case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")) {
						ares_bleed_ench(vict);
					}

					break;
				case 1:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")) {
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

				switch (number(0, 2)) {

				case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")) {
						ares_bleed_ench(vict);
					}

					break;
				case 1:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")) {
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

				switch (number(0, 2)) {

				case 0:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")) {
						ares_bleed_ench(vict);
					}

					break;
				case 1:
					vict = get_random_victim_fighting(mob);
					if(!enchanted_by(vict,"Ares-Slash")) {
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
		if (!IS_SET(GET_BANK(mob), STATE1)) {
			return FALSE;
		}
		act("You need to fight with real combat skills. ", 0, mob, 0, 0, TO_ROOM);
		return TRUE;

		break;
	case CMD_PUMMEL:
		if (!ch) return FALSE;
		//We only want to trigger this if he is in Phase 1 with the shield.
		if (!IS_SET(GET_BANK(mob), STATE1)) {
			return FALSE;
		}
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

//Apollo will single target with a bow.
//Disrupt Spell Casting and Songs
//If still alive, also spec the mobs fighting Apollo
//If artemis dies first - Boost Damage dice size
int olympus_apollo(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;

	int artemis_nr = real_mobile(OLYMPUS_ARTEMIS);

	int stun_delay;
	int reward = 15;

	//Define any other variables

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	char *apollo_speak[6] = { "Art and war share a common rhythm.","The sun does not shine on fools.",  "Every sunrise brings new glory to my domain.","The music of the spheres guides my hand and my bow.","The sun's light reveals all truths, no matter how painful.","Art and war are both tools of creation... and destruction." };

	char *apollo_combat_speak[5] = { "The sun's light shall burn you away","Face the precision of a godb s bow","Each arrow finds its mark; each strike seals your fate!","The sun rises only for the worthy!","Let my music and arrows be your last memory!"};

	char *apollo_to_artemis_speak[5] = {  "Artemis, let's light up this battlefield together!","May my music inspire your arrows to fly true.","Together, sister, we are an unstoppable force.","Feel the rhythm of the fight, Artemis; it's like a symphony.","Our enemies will fall under the brilliance of our combined might."};

	char *apollo_artemis_death_speak[3] = { "My sister, my twin, slain by your hands; feel the scorching fury of the sun's vengeance!","Artemis's arrows no longer fly, but my flames will consume you whole!","You have extinguished the moon's glow; now burn under the relentless blaze of my rage!"};


	switch (cmd)
	{
	case MSG_MOBACT:

		if(!mob->specials.fighting) {

			if (chance(35))
			{
				sprintf(buf, "%s", apollo_speak[number(0, NUMELEMS(apollo_speak) - 1)]);

				do_say(mob, buf, CMD_SAY);
			}

			//Only engage if Artemis is attacked.
			for (vict = world[real_room(LIBRARY_ROOM)].people; vict; vict = vict->next_in_room)
			{
				if(!IS_NPC(vict) || (V_MOB(vict)!=OLYMPUS_ARTEMIS)) continue;
				if((V_MOB(vict)==OLYMPUS_ARTEMIS)  && vict->specials.fighting)
				{
					act("$n wields his bow and attacks.",0,mob,0,0,TO_ROOM);
					do_say(mob,"How dare you attack my sister!",CMD_SAY);
					hit(mob,vict->specials.fighting,TYPE_HIT);
				}
			}
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

			//Check for Artemis - Use State Change
			//If he exists in the proto table -
			if(mob_proto_table[artemis_nr].number > 0) {

				if (chance(35))
				{
					sprintf(buf, "%s", apollo_to_artemis_speak[number(0, NUMELEMS(apollo_to_artemis_speak) - 1)]);

					do_say(mob, buf, CMD_SAY);
				}


			} else {
				//Set Bit - Change Damage Dice
				if(!IS_SET(GET_BANK(mob), STATE1)) {
					act("You will pay for killing my brother.",0,mob,0,0,TO_ROOM);
					SET_BIT(GET_BANK(mob), STATE1);

					mob->specials.damsizedice=35;

				}


				if (chance(35))
				{
					sprintf(buf, "%s", apollo_artemis_death_speak[number(0, NUMELEMS(apollo_artemis_death_speak) - 1)]);

					do_say(mob, buf, CMD_SAY);
				}

			}


			switch (number(0, 5))
			{
			case 0:
				stun_delay = number(1, 3);
				act("$n draws multiple golden arrows and fires them into the room.",0,mob,0,0,TO_ROOM);
				for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
					damage(mob,vict,600,TYPE_UNDEFINED,DAM_PHYSICAL);

				}
				break;
			case 1:
				break;
			case 2:
				vict = get_random_victim_fighting(mob);
				stun_delay = number(1, 3);
				if (vict)
				{
					act("$n draws a golden arrow from his quiver.",0,mob,0,0,TO_ROOM);
					act("A golden arrow pierces your chest.",0,mob,0,vict,TO_VICT);
					act("$N screams as a golden arrow pierces $S chest.",0,mob,0,vict,TO_NOTVICT);
					damage(mob,vict,1050,TYPE_UNDEFINED,DAM_PHYSICAL);
					WAIT_STATE(vict,PULSE_VIOLENCE*stun_delay);
				}
				break;
			case 3:
				vict = get_random_victim_fighting(mob);
				stun_delay = number(3, 7);
				if (vict)
				{
					act("$n draws a silver arrow from his quiver.",0,mob,0,0,TO_ROOM);
					act("A silver arrows hits your leg.",0,mob,0,vict,TO_VICT);
					act("$N screams as a silver arrow hits $S leg.",0,mob,0,vict,TO_NOTVICT);
					damage(mob,vict,300,TYPE_UNDEFINED,DAM_PHYSICAL);
					WAIT_STATE(vict,PULSE_VIOLENCE*stun_delay);
					stop_fighting(vict);
				}
				break;
			case 4:
				break;
			case 5:
				vict = get_random_victim_fighting(mob);
				if (vict)
				{
					act("$n swings with his bow..",0,mob,0,0,TO_ROOM);
					act("A large longbow crashes into your body..",0,mob,0,vict,TO_VICT);
					act("$N groans as $n hits $S with his bow.",0,mob,0,vict,TO_NOTVICT);
					damage(mob,vict,700,TYPE_UNDEFINED,DAM_PHYSICAL);
				}
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


int artemis_bleed_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg) {

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

	if(ench_ch && cmd==MSG_TICK) {

		hp_loss = GET_MAX_HIT(ench_ch) * .05;

		if ((GET_HIT(ench_ch) - hp_loss) <= 100) {
			GET_HIT(ench_ch) = 100;
		} else
		{
			GET_HIT(ench_ch) -= hp_loss;
		}


	}


	return FALSE;

}


void artemis_bleed_ench(CHAR *vict) {
	char buf4[MIL];
	ENCH *tmp_enchantment;

	CREATE(tmp_enchantment, ENCH, 1);
	tmp_enchantment->name     = str_dup("Artemis-Boar-Bleed");
	tmp_enchantment->duration = 3;
	tmp_enchantment->func     = artemis_bleed_func;
	enchantment_to_char(vict, tmp_enchantment, FALSE);
	act("$n has been gored by a celestial boar.\n\r",FALSE,vict,0,0,TO_ROOM);
	send_to_char("You feel a tusk pierce deep into your flesh.\n\r",vict);
	sprintf(buf4,"Olympus Log Ench: [ %s just contracted Artemis Boar Bleed at %d ]",GET_NAME(vict),world[CHAR_REAL_ROOM(vict)].number);
	log_s(buf4);


}



void artemis_animal_summon(CHAR *mob) {
	CHAR *vict, *next_vict;
	int dog_summons;

	//Lets random choose animals and then do different things


	switch (number(1, 5)) {
	case 1: //Deer Antler Charge
		vict = get_random_victim_fighting(mob);
		if (vict)
		{
			act("$n wields his spear and Stabs towards $N.",0,mob,0,vict,TO_ROOM);
			act("A Spear pierces into your flesh.",0,mob,0,vict,TO_VICT);
			act("$N screams as the spear pierces $S flesh.",0,mob,0,vict,TO_NOTVICT);
			damage(mob,vict,1150,TYPE_UNDEFINED,DAM_PHYSICAL);
		}

		break;
	case 2: //Bear Ferocious Swipe (Damage and Bleed)
		act("$n summons a giant celestial bear.",0,mob,0,0,TO_ROOM);

		for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;
			if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
			act("A giant bear paw swipes into your face.",0,mob,0,vict,TO_VICT);

			damage(mob,vict,1500,TYPE_UNDEFINED,DAM_PHYSICAL);
		}


		break;
	case 3: // Random Number of Dogs attack a single person.
		vict = get_random_victim_fighting(mob);
		if (vict)
		{
			act("$n summons a pack of hounds that run through the room.",0,mob,0,vict,TO_ROOM);
			dog_summons = number(1,4);

			switch(dog_summons) {
			case 1:
				act("One dog nips you on the foot.",0,mob,0,vict,TO_VICT);
				act("$N has one dog nip their foot.",0,mob,0,vict,TO_NOTVICT);
				break;
			case 2:
				act("Two dogs latch onto your limbs.",0,mob,0,vict,TO_VICT);
				act("$N has two dogs latch onto their limbs.",0,mob,0,vict,TO_NOTVICT);
				break;
			case 3:
				act("Three dogs bury their fangs into your body.",0,mob,0,vict,TO_VICT);
				act("$N has three dogs bury their fangs into their body.",0,mob,0,vict,TO_NOTVICT);
				break;
			case 4:
				act("Four dogs rip chunks of your flesh off..",0,mob,0,vict,TO_VICT);
				act("$N has four dogs rip chunks of their flesh off.",0,mob,0,vict,TO_NOTVICT);
				break;
			default:
				break;


			}

			damage(mob,vict,450*dog_summons,TYPE_UNDEFINED,DAM_PHYSICAL);

		}



		break;
	case 4:// Boars - Number of Random Bleeds

		vict = get_random_victim_fighting(mob);
		if(!enchanted_by(vict,"Artemis-Boar-Bleed")) {
			artemis_bleed_ench(vict);
		}





		break;
	case 5: // Ariel Dive - Push out of the room.
		vict = get_random_victim_fighting(mob);
		act("$n is engulfed in feathers and carriered from the room.!",0,vict,0,0,TO_ROOM);
		act("You are engulfed by featers and carried away.",0,vict,0,0,TO_CHAR);
		char_from_room(vict);
		char_to_room(vict,real_room(TUNNEL_ROOM));
		do_look(vict,"",CMD_LOOK);
		act("$n appears covered in feathers.", FALSE, vict, 0, 0, TO_ROOM);
		break;
	default:
		break;

	}



}



//Artemis will summon random animals that do different damages and affects.
//Will Vanish from Battle randomly.
//If still alive, also spec the mobs fighting Apollo
//If Apollo dies first - Boost Damage Dice Number


int olympus_artemis(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;


	int apollo_nr = real_mobile(OLYMPUS_APOLLO);
	int stun_delay;


	//Define any other variables

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	char *artemis_speak[6] = { "The wilds are my domain, and you are trespassing.","Hunted or hunter, the choice was yours.","The forest speaks, and I am its voice.","The thrill of the hunt is a gift, not a curse.","Under the moon's gaze, all prey is revealed.","To tread upon the wilds is to challenge my dominion." };

	char *artemis_combat_speak[5] = { "The huntress never misses her prey!","You will fall like a wounded stag!","The wilds themselves fight by my side!","Run if you can; the hunt always ends the same.","Your arrogance blinds you to the predator before you."};


	char *artemis_to_apollo_speak[5] = {  "Stay focused, Apollo; the hunt requires precision.","Remember, brother, the wilds demand respect.","Let the night guide your aim, Apollo.","Even the sun must set; embrace the shadows in battle.","Trust in your instincts, as I trust in mine."};

	char *artemis_apollo_death_speak[3] = { "You have taken my brother, the light of my life; now face the eternal darkness of my wrath!","Apollo's melody is silenced, and for that, you will hear only the dirge of your demise!","The sun has set forever, and the huntress shall avenge its fall!"};





	switch (cmd)
	{
	case MSG_MOBACT:

		//Have him to chat to you.
		if(!mob->specials.fighting) {

			if (chance(35))
			{
				sprintf(buf, "%s", artemis_speak[number(0, NUMELEMS(artemis_speak) - 1)]);

				do_say(mob, buf, CMD_SAY);
			}

			//Only engage if Artemis is attacked.
			for (vict = world[real_room(LIBRARY_ROOM)].people; vict; vict = vict->next_in_room)
			{
				if(!IS_NPC(vict) || (V_MOB(vict)!=OLYMPUS_APOLLO)) continue;
				if((V_MOB(vict)==OLYMPUS_APOLLO)  && vict->specials.fighting) {
					act("$n moves to attack the group.",0,mob,0,0,TO_ROOM);
					do_say(mob,"How dare you attack my brother!",CMD_SAY);
					hit(mob,vict->specials.fighting,TYPE_HIT);
				}
			}
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

			//Check for Artemis - Use State Change
			//If he exists in the proto table -
			if(mob_proto_table[apollo_nr].number > 0) {

				if (chance(35))
				{
					sprintf(buf, "%s", artemis_to_apollo_speak[number(0, NUMELEMS(artemis_to_apollo_speak) - 1)]);

					do_say(mob, buf, CMD_SAY);
				}


			} else {
				//Set Bit - Change Damage Dice
				if(!IS_SET(GET_BANK(mob), STATE1)) {
					act("You will pay for killing my sister.",0,mob,0,0,TO_ROOM);
					SET_BIT(GET_BANK(mob), STATE1);


					mob->specials.damnodice=35;

				}


				if (chance(35))
				{
					sprintf(buf, "%s", artemis_apollo_death_speak[number(0, NUMELEMS(artemis_apollo_death_speak) - 1)]);

					do_say(mob, buf, CMD_SAY);
				}

			}


			switch (number(0, 5))
			{
			case 0:

				act("$n throws a handful of hunting knives into the room.",0,mob,0,0,TO_ROOM);
				for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
					damage(mob,vict,750,TYPE_UNDEFINED,DAM_PHYSICAL);

				}
				break;
			case 1:
				break;
			case 2:
				vict = get_random_victim_fighting(mob);
				stun_delay = number(1, 2);
				if (vict)
				{
					act("$n draws a golden arrow from his quiver.",0,mob,0,0,TO_ROOM);
					act("A golden arrow pierces your chest.",0,mob,0,vict,TO_VICT);
					act("$N screams as a golden arrow pierces $S chest.",0,mob,0,vict,TO_NOTVICT);
					damage(mob,vict,850,TYPE_UNDEFINED,DAM_PHYSICAL);
					WAIT_STATE(vict,PULSE_VIOLENCE*stun_delay);
				}
				break;
			case 3: //Vanish and reappear

				act("$n disapears into thin air.", FALSE, mob, 0, 0, TO_ROOM);
				stop_fighting(mob);

				for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if (GET_OPPONENT(vict) && GET_OPPONENT(vict) == mob)
					{
						stop_fighting(vict);
						send_to_char("You can't fight someone that you can't see.\n", vict);
					}
				}

				if (!(vict = get_random_victim(mob))) return FALSE;

				act("$n fades back into existence holding a golden arrow. $e stabs you with a sharp golden arrow.", FALSE, mob, 0, vict, TO_VICT);
				act("$n fades into existence and stabs $N with a golden arrow.", FALSE, mob, 0, vict, TO_NOTVICT);
				act("You appear behind your next victim.", FALSE, mob, 0, vict, TO_CHAR);
				hit(mob, vict, TYPE_UNDEFINED);


				break;
			case 4:
			case 5:
				artemis_animal_summon(mob);

				if(IS_SET(GET_BANK(mob), STATE1)) {
					act("$n screams 'The animals will make you pay'", FALSE, mob, 0, 0, TO_ROOM);
					artemis_animal_summon(mob);

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

int get_forge_state(CHAR *mob) {

	int state = 0;

	if(IS_SET(GET_BANK(mob), STATE1)) {

		state++;
	}
	if(IS_SET(GET_BANK(mob), STATE2)) {

		state++;
	}
	if(IS_SET(GET_BANK(mob), STATE3)) {

		state++;
	}
	if(IS_SET(GET_BANK(mob), STATE4)) {

		state++;
	}
	if(IS_SET(GET_BANK(mob), STATE5)) {

		state++;
	}

	return state;
}
// Increase the state of Forge by 1
//If 1 bit is set, exit the function.
void increment_forge_state(CHAR *mob) {

	bool cont = TRUE;

	if(!IS_SET(GET_BANK(mob), STATE1) && cont) {

		SET_BIT(GET_BANK(mob), STATE1);
		cont = FALSE;
	}
	if(!IS_SET(GET_BANK(mob), STATE2) && cont) {

		SET_BIT(GET_BANK(mob), STATE2);
		cont = FALSE;
	}
	if(!IS_SET(GET_BANK(mob), STATE3) && cont) {

		SET_BIT(GET_BANK(mob), STATE3);
		cont = FALSE;
	}
	if(!IS_SET(GET_BANK(mob), STATE4) && cont) {

		SET_BIT(GET_BANK(mob), STATE4);
		cont = FALSE;
	}
	if(!IS_SET(GET_BANK(mob), STATE5) && cont) {

		SET_BIT(GET_BANK(mob), STATE5);
		cont = FALSE;
	}



}

//Decrement the state of the forge by 1
//Once you have removed all 5, remove STATE6, so it can start over.
void decrement_forge_state(CHAR *mob) {

	bool cont = TRUE;



	if(IS_SET(GET_BANK(mob), STATE1) && cont) {
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);
		REMOVE_BIT(GET_BANK(mob), STATE1);
		cont = FALSE;
	}
	if(IS_SET(GET_BANK(mob), STATE2) && cont) {
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);
		REMOVE_BIT(GET_BANK(mob), STATE2);
		cont = FALSE;
	}
	if(IS_SET(GET_BANK(mob), STATE3) && cont) {
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);
		REMOVE_BIT(GET_BANK(mob), STATE3);
		cont = FALSE;
	}
	if(IS_SET(GET_BANK(mob), STATE4) && cont) {
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);
		REMOVE_BIT(GET_BANK(mob), STATE4);
		cont = FALSE;
	}
	if(IS_SET(GET_BANK(mob), STATE5) && cont) {
		act("The Forge starts to cool down.",0,mob,0,0,TO_ROOM);
		REMOVE_BIT(GET_BANK(mob), STATE5);
		cont = FALSE;
	}

	//If you have unset all 5 flags and cont is true, then lets reset STATE 6
	//This will allow the forge to start charging again.

	if(cont) {
		REMOVE_BIT(GET_BANK(mob), STATE6);
		act("The Forge has become cold and lifeless.",0,mob,0,0,TO_ROOM);
	}



}

int olympus_hephaestus(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;

	//int stun_delay;
	int forge_state=0;
	int missle;
	int reward = 15;
	//Define any other variables

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(mob, TRUE) < 1) return FALSE;

	char *hephaestus_speak[6] = { "Even gods need the tools I create.","I craft destiny with my hammer and anvil.","","","","" };

	char *hephaestus_combat_speak[5] = { "Behold the strength of a god who builds and destroys!","Your armor will crumble under my strike!","My hammer forges not only weapons but destruction!","The flames of creation become your undoing!","Feel the weight of my forge's fury!"};

	char *missles[5] = {"Metal Tongs","A dense hammer","A sharp chisel","A flat file","A giant chunk of coal"};

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
			case 0:
				vict = get_random_victim_fighting(mob);
				if (vict)
				{
					act("$n grabs something from the workbench and throws it at $N.",0,mob,0,vict,TO_NOTVICT);
					act("$n grabs something from the workbench and throws it at you.",0,mob,0,vict,TO_VICT);
					missle = number(0, NUMELEMS(missles)-1);
					sprintf(buf, "%s is stuck by %s!", GET_NAME(vict), missles[missle]);
					act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
					sprintf(buf, "You are stuck by %s!", missles[missle]);
					act(buf, FALSE, mob, NULL, vict, TO_VICT);
					damage(mob,vict,1089,TYPE_UNDEFINED,DAM_PHYSICAL);
				}
				break;
			case 1:
				break;
			case 2:
				vict = get_random_victim_fighting(mob);
				if (vict)
				{
					act("$n grabs something from the workbench and throws it at $N.",0,mob,0,vict,TO_NOTVICT);
					act("$n grabs something from the workbench and throws it at you.",0,mob,0,vict,TO_VICT);
					missle = number(0, NUMELEMS(missles)-1);
					sprintf(buf, "%s is stuck by %s!", GET_NAME(vict), missles[missle]);
					act(buf, FALSE, mob, NULL, vict, TO_NOTVICT);
					sprintf(buf, "You are stuck by %s!", missles[missle]);
					act(buf, FALSE, mob, NULL, vict, TO_VICT);
					damage(mob,vict,960,TYPE_UNDEFINED,DAM_PHYSICAL);
				}
				break;
			case 3:
				vict = get_random_victim_fighting(mob);
				if (vict)
				{
					act("$n smashes you with a large wrench.",0,mob,0,0,TO_ROOM);
					act("A large wrench smashes into you.",0,mob,0,vict,TO_VICT);
					act("$N screams as a large wrench hits them.",0,mob,0,vict,TO_NOTVICT);
					damage(mob,vict,866,TYPE_UNDEFINED,DAM_PHYSICAL);
					WAIT_STATE(vict,PULSE_VIOLENCE*1);
				}
				break;
			case 4:
				break;
			case 5:
				vict = get_random_victim_fighting(mob);
				if (vict)
				{
					act("$n smashes you with a large hammer.",0,mob,0,0,TO_ROOM);
					act("A large hammer smashes into you.",0,mob,0,vict,TO_VICT);
					act("$N screams as a large hammer hits them.",0,mob,0,vict,TO_NOTVICT);
					damage(mob,vict,1050,TYPE_UNDEFINED,DAM_PHYSICAL);
				}
				break;
			case 6:
			case 7: // Increment the Forge
				if(!IS_SET(GET_BANK(mob), STATE6)) {
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

			if(forge_state == 0 && !IS_SET(GET_BANK(mob), STATE6)) {
				send_to_room("The Forge stands cold and lifeless, its hearth devoid of any warmth.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
			}
			else if(forge_state == 1 && !IS_SET(GET_BANK(mob), STATE6)) {
				send_to_room("The Forge shows a slight flicker of life.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
			}
			else if(forge_state == 2 && !IS_SET(GET_BANK(mob), STATE6)) {
				send_to_room("The Forge starts to radiate heat as the flames grow steadily.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
			}
			else if(forge_state == 3 && !IS_SET(GET_BANK(mob), STATE6)) {
				send_to_room("The Forge roars to life, its core glowing brightly as intense heat fills the area.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
			}
			else if(forge_state == 4 && !IS_SET(GET_BANK(mob), STATE6)) {
				send_to_room("The Forge maintains a consistent, powerful blaze, ready for any blacksmithing endeavor.\n\r", real_room(HEPHAESTUS_FORGE_ROOM));
			}
			else if(forge_state == 5 && !IS_SET(GET_BANK(mob), STATE6)) {
				act("The forge explodes into the room.",0,mob,0,0,TO_ROOM);

				//Set State 6 to start the cooldown phase.
				SET_BIT(GET_BANK(mob), STATE6);


				for(vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if(!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
					damage(mob,vict,2300,TYPE_UNDEFINED,DAM_FIRE);
				}

			} else if(IS_SET(GET_BANK(mob), STATE6)) {
				decrement_forge_state(mob);
			}


		}

		break;
	//can add an else branch here if you want them to act but not in combat.
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




//Assign Spec for the zone. Sets all other specs.
//First Param - Object, Room or Mob Number.  Define it up above.
//Second Param - the name of the function that is for the mob Usually a shorthand for your zone.
void assign_olympus(void)
{
	/*Objects */

	/*Rooms */
	assign_room(WATERFALL_ROOM, olympus_waterfallroom);
	assign_room(TYPHON_ROOM, olympus_typhonroom);

	/*Mobs */

	assign_mob(OLYMPUS_TYPHON, olympus_typhon);
	assign_mob(OLYMPUS_HERA, olympus_hera);
	assign_mob(OLYMPUS_POSEIDON, olympus_poseidon);
	assign_mob(OLYMPUS_ARES, olympus_ares);
	assign_mob(OLYMPUS_HEPHAESTUS, olympus_hephaestus);
	assign_mob(OLYMPUS_ZEUS, olympus_zeus);
	assign_mob(OLYMPUS_APOLLO, olympus_apollo);
	assign_mob(OLYMPUS_ARTEMIS, olympus_artemis);

}

/*



Hemp (gossip) [ Cronus loads: bit of cloud (key) ]


Hemp (gossip) [ Icarus loads: waxy feather (key) ]


Hemp (gossip) [ Pluto loads: rusty nail (key) ]



*/

