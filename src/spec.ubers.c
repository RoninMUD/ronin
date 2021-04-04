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

/*Objects */

/*Mobs */
#define UBER_KING_SPIDER 14600
#define UBER_GREAT_MYSTIC 14601
#define UBER_ULT_MYSTIC 14602
#define UBER_HUGE_WHIRLWIND 14603
#define UBER_ZYCA_PRINCE 14604

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

int ub_uber_kingspider(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;
	//You must declare the reward so you get AQP.
	int reward = 3;

	//Define any other variables

	int drain = 0;

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(uber, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			//if fighting - spec different attacks
			if (uber->specials.fighting)
			{
				//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.
				switch (number(0, 6))
				{
					case 0:
					case 1:
						vict = get_random_victim_fighting(uber);
						if (vict)
						{
							act("$n grabs $N and shoves a leg through their chest.", 0, uber, 0, vict, TO_NOTVICT);
							act("$n grabs you and shoves a leg through your chest.", 0, uber, 0, vict, TO_VICT);
							sprintf(buf, "%s is impaled by a giant leg.", GET_NAME(vict));
							act(buf, FALSE, uber, NULL, vict, TO_NOTVICT);
							sprintf(buf, "You are impaled by a giant leg.");
							act(buf, FALSE, uber, NULL, vict, TO_VICT);
							GET_HIT(vict) = GET_HIT(vict) - GET_HIT(vict) / 3;
						}

						break;
					case 2:
						vict = get_random_victim_fighting(uber);
						if (vict)
						{
							act("$n spins and hits $N with his abdomen, knocking them out.", 0, uber, 0, vict, TO_NOTVICT);
							act("$n spins and hits you with his abdomen, knocking you out.", 0, uber, 0, vict, TO_VICT);
							damage(uber, vict, 1200, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *2);
						}

						break;
					case 3:

					case 4:

						act("$n hisses loudly and covers everyone in spidersilk.", 0, uber, 0, 0, TO_ROOM);
						for (vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(uber, vict, 600, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *3);
						}

					case 5:
					case 6:
						vict = get_random_victim_fighting(uber);
						//This spec will heal the uber based on the random number. It will also drain stats from the vict (target character).  The Actions send different messages to the room.
						act("$n pierces you with long dripping fangs and sucks out some blood.", FALSE, uber, 0, vict, TO_VICT);
						act("$n pierces $N with long dripping fangs and sucks out some blood.", FALSE, uber, 0, vict, TO_NOTVICT);
						act("You pierce $N with long dripping fangs and suck out some blood.", FALSE, uber, 0, vict, TO_CHAR);
						send_to_char("You feel your blood be sucked from you.\n", vict);
						drain = number(400, 600);
						//void drain_mana_hit_mv(struct char_data *ch, struct char_data *vict, int mana, int hit, int mv, bool add_m, bool add_hp, bool add_mv)
						drain_mana_hit_mv(uber, vict, drain, 0, 0, TRUE, FALSE, FALSE);
						GET_HIT(uber) = MIN(GET_MAX_HIT(uber), GET_HIT(uber) + (drain *5));
						break;
					default:
						break;
				}
			}
			else
			{
				if (chance(40))
				{
					do_say(uber, "Shakes its legs around the room and hisses.", CMD_SAY);
				}
			}

			break;
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
			send_to_room(buf, CHAR_REAL_ROOM(uber));
			for (vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
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

//Mob Number 14601
//Load in Room 12931

//ub_uber_greatmystic
int ub_uber_greatmystic(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *tch, *vict, *next_vict;

	CHAR * wind;
	int uber_wind_nr;
	//You must declare the reward so you get AQP.
	int reward = 4;
	uber_wind_nr = real_mobile(UBER_HUGE_WHIRLWIND);
	//Define any other variables

	char *uber_gm_speak[4] = { "Let me show you the power of enlightenment.", "Have you come for my teachings?", "Respect your Elders!", "Humility awaits you in this chamber." };

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(uber, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			//Have him to chat to you.
			if (chance(40))
			{
				sprintf(buf, "%s", uber_gm_speak[number(0, NUMELEMS(uber_gm_speak) - 1)]);

				do_say(uber, buf, CMD_SAY);
			}

			//if fighting - spec different attacks
			if (uber->specials.fighting)
			{
				//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.

				switch (number(0, 6))
				{
					case 0:
						vict = get_random_victim_fighting(uber);
						if (vict)
						{
							act("$n calmly touches $N with his palm.  You don't need this, do you?", 0, uber, 0, vict, TO_NOTVICT);
							act("$n calmly touches you saying you don't need this, do you? ", 0, uber, 0, vict, TO_VICT);
							GET_HIT(vict) = GET_HIT(vict) *0.8;	//Remove 20% of Current HP
						}

						break;
					case 1:

					case 2:
						vict = get_random_victim_fighting(uber);
						if (vict)
						{
							act("$n yells at $N saying do my paperwork as a stack of papers hits them in the face.", 0, uber, 0, vict, TO_NOTVICT);
							act("$n yells at you saying do my paperwork as a stack of papers hits you in the face.", 0, uber, 0, vict, TO_VICT);
							damage(uber, vict, 1500, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *1);
						}

						break;
					case 3:
						// Spawn Uber Whirlwinds
						if ((tch = GET_OPPONENT(uber)) && mob_proto_table[uber_wind_nr].number < 2)
						{
							wind = read_mobile(uber_wind_nr, REAL);
							char_to_room(wind, CHAR_REAL_ROOM(uber));

							act("$n cackles gleefully and flashes a quick smirk at you.", FALSE, uber, 0, tch, TO_VICT);
							act("$n cackle at $N and flashes them a quick smirk.", FALSE, uber, 0, tch, TO_NOTVICT);
							act("$n appears in a huge gust of wind and water.", FALSE, wind, 0, 0, TO_ROOM);
							hit(wind, tch, TYPE_UNDEFINED);

							return FALSE;

						}

					case 4:
						break;
					case 5:
						//Group Stun
						act("$n screams 'Mystical Powers' and rants incoherently ", 0, uber, 0, 0, TO_ROOM);
						for (vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(uber, vict, 1200, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *2);
						}

						break;
					case 6:
						act("$n punches himself in the leg.", FALSE, uber, 0, 0, TO_ROOM);
						do_say(uber, "Ahhh, feels so good.", CMD_SAY);
						GET_HIT(uber) = GET_HIT(uber) + 12000;
						break;
					default:
						break;
				}
			}

			//can add an else branch here if you want them to act but not in combat.

			break;

		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
			send_to_room(buf, CHAR_REAL_ROOM(uber));
			for (vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
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

//Uber Ultimate Mystic Extra Functions

void uber_um_stun(CHAR *ch, CHAR *vict)
{
	act("$n stuns you with a mighty blow on the head.", FALSE, ch, 0, vict, TO_VICT);
	act("$n stuns $N with a mighty blow on $S head.", FALSE, ch, 0, vict, TO_NOTVICT);
	act("You hit $N with a stunning blow. $E will be out for a while.", FALSE, ch, 0, vict, TO_CHAR);

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

void uber_um_vanish(CHAR *ch)
{
	CHAR * t;

	act("$n vanishes into the shadows.", FALSE, ch, 0, 0, TO_ROOM);
	stop_fighting(ch);

	for (t = world[CHAR_REAL_ROOM(ch)].people; t; t = t->next_in_room)
	{
		if (GET_OPPONENT(t) && GET_OPPONENT(t) == ch)
		{
			stop_fighting(t);
			send_to_char("You can't fight someone that vanished into thin air.\n", t);
		}
	}

	if (!(t = get_random_victim(ch)))
		return;

	act("$n materializes behind you. $e hits you in the head with a heavy punch.", FALSE, ch, 0, t, TO_VICT);
	act("$n materializes behind $N and hits them in the head with a heavy punch.", FALSE, ch, 0, t, TO_NOTVICT);
	act("You appear behind your next victim.", FALSE, ch, 0, t, TO_CHAR);
	hit(ch, t, TYPE_UNDEFINED);
}

//Mob Number 14602
//Load in Room 12904

//Uber Mystic Main Spec
//ub_uber_ultmystic
int ub_uber_ultmystic(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict, *vict2;

	//You must declare the reward so you get AQP.
	int reward = 5;
	int factor;

	//Define any other variables

	//STATE1 will be used as the healed state.  We only want them to heal once.

	/*Don't waste any more CPU time if no one is in the room. */
	if (count_mortals_room(uber, TRUE) < 1) return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			//if fighting - spec different attacks
			if (uber->specials.fighting)
			{
				//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.

				factor = 10* GET_HIT(uber) / GET_MAX_HIT(uber);
				switch (factor)
				{
					//Stun a random victim.
					case 9:

						do_say(uber, "You do show some promise.", CMD_SAY);

						if (!(vict = get_random_victim(uber)))
						{
							return FALSE;
						}

						uber_um_stun(uber, vict);
						break;
						//Vanish at 80% HP
					case 8:
						do_say(uber, "If your eyes cant keep up with me, how do you expect to win?", CMD_SAY);
						uber_um_vanish(uber);
						break;

					case 7:
						//Group Stun
						act("$n shouts Bow to the master of the mystical arts. ", 0, uber, 0, 0, TO_ROOM);
						for (vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(uber, vict, 1000, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *1);
						}

						break;
						//50/50 chance of single stun or double stun.
					case 6:
						do_say(uber, "How many shall I target today?", CMD_SAY);
						switch (number(0, 1))
						{
							case 0:
								if (!(vict = get_random_victim(uber)))
								{
									return FALSE;
								}

								uber_um_stun(uber, vict);
								break;

							case 1:

								vict = get_random_victim_fighting(uber);
								if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
								uber_um_stun(uber, vict);

								//Check for more than 1 person in the room.
								if (count_mortals_room(uber, TRUE) > 1)
								{
									//get 2nd victim that is still fighting
									vict2 = get_random_victim_fighting(uber);
									if (!(vict) || IS_NPC(vict2) || !(IS_MORTAL(vict2))) return FALSE;
									uber_um_stun(uber, vict2);
								}
						}

						break;

						//Heal for 10% of max hp. Only do it once, else this would be infinite loop.
					case 5:

						//STATE1 is Healed.  We will set the bit and then be done healing.
						if (!IS_SET(GET_BANK(uber), STATE1))
						{
							do_say(uber, "May enlightenment one day also find you.", CMD_SAY);
							SET_BIT(GET_BANK(uber), STATE1);
							GET_HIT(uber) += (GET_MAX_HIT(uber) / 10);
						}

						break;

					case 4:
						do_say(uber, "Bathe in mystical flames.", CMD_SAY);
						vict = get_random_victim_fighting(uber);
						act("$n makes a complex hand gesture and engulfs $N in flames.", 0, uber, 0, vict, TO_NOTVICT);
						act("$n makes a complex hand gesture and engulfs you in flames.", 0, uber, 0, vict, TO_VICT);
						sprintf(buf, "%s is engulfed by %s!", GET_NAME(vict), "mystical flames.");
						act(buf, FALSE, uber, NULL, vict, TO_NOTVICT);
						sprintf(buf, "You are engulfed by mystical flames.");
						act(buf, FALSE, uber, NULL, vict, TO_VICT);
						GET_HIT(vict) = GET_HIT(vict) / 3;
						break;

						//Vanish Again.
					case 3:
						do_say(uber, "Once again you were too slow to keep up.", CMD_SAY);
						uber_um_vanish(uber);
						break;

					case 2:
						do_say(uber, "Time to end this charade. Stand there and die.", CMD_SAY);
						for (vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
						{
							next_vict = vict->next_in_room;
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(uber, vict, 1600, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *3);
						}

						break;

					case 1:
						do_say(uber, "Wonderful. Absolutley wonderful. Continue your development.", CMD_SAY);

						switch (number(0, 1))
						{
							case 0:
								if (!(vict = get_random_victim(uber)))
								{
									return FALSE;
								}

								uber_um_stun(uber, vict);
								break;

							case 1:

								vict = get_random_victim_fighting(uber);
								if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
								uber_um_stun(uber, vict);

								//Check for more than 1 person in the room.
								if (count_mortals_room(uber, TRUE) > 1)
								{
									//get 2nd victim that is still fighting
									vict2 = get_random_victim_fighting(uber);
									if (!(vict) || IS_NPC(vict2) || !(IS_MORTAL(vict2))) return FALSE;
									uber_um_stun(uber, vict2);
								}
						}

						break;
				}
			}
			else
			{
				//Remove Existing States if not fighting.

				/*If not fighting, and wimpy, remove wimpy. */
				if (!GET_OPPONENT(uber) && IS_SET(GET_BANK(uber), STATE1))
				{
					REMOVE_BIT(GET_BANK(uber), STATE1);

					return FALSE;
				}
			}

			//can add an else branch here if you want them to act but not in combat.

			break;

		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
			send_to_room(buf, CHAR_REAL_ROOM(uber));
			for (vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
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


//Mob Number 14604
//Load in Room 10969

 int ub_uber_zycaprince(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;

	//You must declare the reward so you get AQP.
	int reward = 2;

	//Define any other variables

	char *uber_zp_speak[4] = { "Jeez Dad, leave me alone.", "Don't tell me what to do. You're not my real dad.", "The bachelor life is the life for me.", "Bah...i don't wanna do anything." };

	switch (cmd)
	{
		case MSG_MOBACT:

			//Have him to chat to you.
			if (chance(40))
			{
				sprintf(buf, "%s", uber_zp_speak[number(0, NUMELEMS(uber_zp_speak) - 1)]);

				do_say(uber, buf, CMD_SAY);
			}

			//if fighting - spec different attacks
			if (uber->specials.fighting)
			{
				//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.

				switch (number(0, 2))
				{
					case 0:
						vict = get_random_victim_fighting(uber);
						if (vict)
						{
							act("$n yells at $N saying get out of my rooooom.", 0, uber, 0, vict, TO_NOTVICT);
							act("$n yells at you saying get out of my rooooom.", 0, uber, 0, vict, TO_VICT);
							sprintf(buf, "%s is hit with a pillow.", GET_NAME(vict));
							act(buf, FALSE, uber, NULL, vict, TO_NOTVICT);
							sprintf(buf, "You are hit by a pillow.");
							act(buf, FALSE, uber, NULL, vict, TO_VICT);
							damage(uber, vict, 200, TYPE_UNDEFINED, DAM_PHYSICAL);
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

			//can add an else branch here if you want them to act but not in combat.

			break;

		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
			send_to_room(buf, CHAR_REAL_ROOM(uber));
			for (vict = world[CHAR_REAL_ROOM(uber)].people; vict; vict = next_vict)
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
void assign_ubers(void)
{
	/*Objects */
	//assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);

	/*Rooms */
	//assign_room(LIGHTWALL,             	cl_LightWallLink);

	/*Mobs */

	assign_mob(UBER_KING_SPIDER, ub_uber_kingspider);
	assign_mob(UBER_GREAT_MYSTIC, ub_uber_greatmystic);
	assign_mob(UBER_ULT_MYSTIC, ub_uber_ultmystic);
	assign_mob(UBER_ZYCA_PRINCE, ub_uber_zycaprince);

}
