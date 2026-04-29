/*spec.veilofunspentfate.c - Specs for The Veil of Unspent Fate by Fisher

     Written by Fisher for RoninMUD
     Creation Date: 3/16/2026
     
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
#include "aff_ench.h"

/*Rooms */
#define DAILY_QP_ZONE_ROOM 23900
#define DAILY_SKILLS_ZONE_ROOM 23920
#define DIVIDING_VEIL_ROOM 23909


/*Objects */
#define SKILLGEM 23901


/*Mobs */
#define VEIL_BROKER 23912

#define ECHO_OF_THE_ALMOST 23900
#define WHISPER_OF_REGRET 23901
#define ECHO_OF_THE_ALMOST_VARIANT 23902
#define SHARDBOUND_FRAGMENT 23903
#define WHISPER_OF_REGRET_VARIANT 23904
#define FRACTURED_REVENANT 23905
#define SHARDBOUND_KEEPER 23906
#define CONVERGENCE_WRAITH 23907
#define VEILBOUND_ARBITER 23908
#define VEILTORN_DRIFTER 23909
#define FATEBREAKER_WARDEN 23910
#define CONVERGENCE_HORROR 23911


#define VEIL_WISP 23913
#define ECHO_FRAGMENT 23914
#define SHARDBOUND_HORROR 23915

#define MEMORY_LEECH 23916
#define REFRACTION_ENGINE 23917



#define DAILY_QP_FILE "util/daily_qp_users.txt"


/*Miscellaneous strings */
// Generic States that are shifted to indicate different stages.
// Each Uber will use this differently.
#define STATE1 (1 << 0) // 1
#define STATE2 (1 << 1) // 2
#define STATE3 (1 << 2) // 4
#define STATE4 (1 << 3) // 8

#define VEIL_ROOM_START 23900
#define VEIL_ROOM_END 23919
#define VEIL_QUEST_NAME "VEIL"

#define SKILL_ROOM_START 23920
#define SKILL_ROOM_END 23931
#define SKILLS_QUEST_NAME "SKILLS"



/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */




/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */


int dividing_veil_room(int room,CHAR *ch,int cmd,char *argument) {
  int morts;
  morts = count_mortals_zone(ch,TRUE);
  
  if(cmd==CMD_DOWN) {
    if(IS_IMMORTAL(ch)) return FALSE;
	
	if(morts >= 5){
		send_to_char("The veil allows you through.\n\r",ch);
		return FALSE;
	}else{
		send_to_char("The veil blocks your way.\n\r",ch);
		return TRUE;
	}
    
  }
  return FALSE;
}


/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */

int has_used_daily_qp(char *name, char *quest) {
    FILE *fp;
    char buf[MAX_INPUT_LENGTH];
    char file_name[MAX_INPUT_LENGTH];
    char file_quest[MAX_INPUT_LENGTH];

    if (!(fp = fopen(DAILY_QP_FILE, "r")))
        return FALSE;

    while (fgets(buf, sizeof(buf), fp)) {
        buf[strcspn(buf, "\r\n")] = '\0'; // strip newline

        // Parse using | delimiter
        if (sscanf(buf, "%[^|]|%s", file_name, file_quest) != 2)
            continue;

        if (!str_cmp(file_name, name) && !str_cmp(file_quest, quest)) {
            fclose(fp);
            return TRUE;
        }
    }

    fclose(fp);
    return FALSE;
}

void add_daily_qp_user(char *name, char *quest) {
    FILE *fp;

    if (!(fp = fopen(DAILY_QP_FILE, "a"))) {
        perror("add_daily_qp_user fopen");
        return;
    }

    fprintf(fp, "%s|%s\n", name, quest);
    fclose(fp);
}


void reset_daily_qp_file(void) {
    FILE *fp;

    if ((fp = fopen(DAILY_QP_FILE, "w"))) {
        fclose(fp); // truncate file
    }
}


int count_mortals_room_range(int start_room, int end_room) {
    int total_count = 0;

    for (int i = start_room; i <= end_room; i++) {
        int rnum = real_room(i);

        // SAFETY CHECK
        if (rnum < 0)
            continue;

        total_count += count_mortals_real_room(rnum);
    }

    return total_count;
}


int daily_qp_keeper(CHAR *mob, CHAR *ch, int cmd, char *arg){
    char keyword[MAX_INPUT_LENGTH];
	CHAR *vict, *next_vict;
	int mortal_count;
	
	switch (cmd)
    {
		
		case MSG_MOBACT:
		
			//We need to reset the file at midnight, but only once a day.  So we will set the bank bit at midnight to reset the file.
				if (!IS_SET(GET_BANK(mob), STATE1) && time_info.hours == 0)
				{
					do_say(mob, "File Reset", CMD_SAY);
					SET_BIT(GET_BANK(mob), STATE1);
					reset_daily_qp_file();
				}
		
			break;
		case CMD_KILL:
		case CMD_HIT:
		case CMD_KICK:
		case CMD_AMBUSH:
		case CMD_ASSAULT:
		case CMD_BACKSTAB:
			do_say(mob, "I have to protect the Veil...move away.", CMD_SAY);
			return TRUE;
		case MSG_VIOLENCE:

			for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = vict->next_in_room)
				if (vict->specials.fighting == mob)
					stop_fighting(vict->specials.fighting);
			stop_fighting(mob);
			GET_HIT(mob) = GET_MAX_HIT(mob);

			do_say(mob, "I have to protect the Veil...move away.", CMD_SAY);

			return FALSE;
		
		case CMD_SAY:	
		
			one_argument(arg, keyword);		
			
			if (!str_cmp(keyword, "veil")) {
				/* handle veil */
				
				mortal_count = count_mortals_room_range(VEIL_ROOM_START,VEIL_ROOM_END);
			
				if(mortal_count > 0){
					act("Please wait until others have left the veil.\n\r", FALSE, mob, 0, 0, TO_ROOM);
					return TRUE;
				}
				
				
				
				for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
				{
					next_vict = CHAR_NEXT_IN_ROOM(vict);
					
					if (!(vict) || IS_NPC(vict)){
							continue;
					}
					
					if(has_used_daily_qp(GET_NAME(vict),VEIL_QUEST_NAME)) {
						if(IS_IMMORTAL(vict)){
							act("Why you want to do this again is beyond my comprehension.....\n\r", FALSE, mob, 0, vict, TO_VICT);
						}else{
							//send_to_char("The guardian shakes its head. You have already claimed your reward today.\n\r", ch);
							act("You have already claimed your reward today.\n\r", FALSE, mob, 0, vict, TO_VICT);
						}	
						
					}else{

						if(IS_IMMORTAL(ch)){
							act("You shouldnt be exploring these areas as an immort.\n\r", FALSE, mob, 0, vict, TO_VICT);
						}else{
							//send_to_char("The guardian shakes its head. You have already claimed your reward today.\n\r", ch);
							//act("You have already claimed your reward today.\n\r", FALSE, mob, 0, ch, TO_CHAR);
							send_to_char("The guardian nods and gestures. You are enveloped in mist...\n\r", vict);
						}	

						char_from_room(vict);
						char_to_room(vict, real_room(DAILY_QP_ZONE_ROOM)); // define this

						do_look(vict, "", 0);

						add_daily_qp_user(GET_NAME(vict),VEIL_QUEST_NAME);
					}
					
				}		
				
				
				return TRUE;
			}
			
			if (!str_cmp(keyword, "skills")) {
				/* handle skills */
				mortal_count = count_mortals_room_range(SKILL_ROOM_START,SKILL_ROOM_END);
			
				if(mortal_count > 0){
					act("Please wait until others have left the skills area.\n\r", FALSE, mob, 0, 0, TO_ROOM);
					return TRUE;
				}
				
				
				
				for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
				{
					next_vict = CHAR_NEXT_IN_ROOM(vict);
					
					if (!(vict) || IS_NPC(vict)){
							continue;
					}
					
					if(has_used_daily_qp(GET_NAME(vict),SKILLS_QUEST_NAME)) {
						if(IS_IMMORTAL(vict)){
							act("Why you want to do this again is beyond my comprehension.....\n\r", FALSE, mob, 0, vict, TO_VICT);
						}else{
							//send_to_char("The guardian shakes its head. You have already claimed your reward today.\n\r", ch);
							act("You have already claimed your reward today.\n\r", FALSE, mob, 0, vict, TO_VICT);
						}	
						
					}else{

						if(IS_IMMORTAL(ch)){
							act("You shouldnt be exploring these areas as an immort.\n\r", FALSE, mob, 0, vict, TO_VICT);
						}else{
							//send_to_char("The guardian shakes its head. You have already claimed your reward today.\n\r", ch);
							//act("You have already claimed your reward today.\n\r", FALSE, mob, 0, ch, TO_CHAR);
							send_to_char("The guardian nods and gestures. You are enveloped in mist...\n\r", vict);
						}	

						char_from_room(vict);
						char_to_room(vict, real_room(DAILY_SKILLS_ZONE_ROOM)); // define this

						do_look(vict, "", 0);

						add_daily_qp_user(GET_NAME(vict),SKILLS_QUEST_NAME);
					}
					
				}		
				
				
				return TRUE;
				
			}
			
			
			
			return FALSE;
			break;
		case CMD_UNKNOWN:
		
			one_argument(arg, keyword);			
			
			if (IS_IMMORTAL(ch) && is_abbrev(keyword, "reset")){
				send_to_char("File reset. \n\r", ch);
				reset_daily_qp_file();
				return TRUE;
			}
			break;
		default:
			break;
		
	}
	
    return FALSE;
}

int veil_mob_echo(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	    // These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
	int reward = 1;
    // Define any other variables

    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1){
        return FALSE;
	}

    
	switch (cmd)
    {	
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the veil rewards you.\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			mob_aq_reward(reward, mob);
			break;
    }
    return FALSE;
}

/**
 * Calculates reward based on mortal density in a room.
 * Rewards +1 for every 5 mortals (integer division).
 *
 * @param num_mortals Total number of mortals in the room.
 * @return The calculated reward amount.
 */
int calculate_mortal_reward(int num_mortals) {
    if (num_mortals < 0) return 0; // Error handling
    
    // Using integer division: 0-4=0, 5-9=1, 10-14=2, etc.
    return num_mortals / 5;
}

int veil_mob_echo_upgraded(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	// These are default declarations to give variables to characters.
    char buf[MAX_STRING_LENGTH];
	int reward = 2;
	//int mortal_count;
    // Define any other variables

    /*Don't waste any more CPU time if no one is in the room. */
    if (count_mortals_room(mob, TRUE) < 1){
        return FALSE;
	}

	//mortal_count = count_mortals_room(mob, TRUE);
    //reward = calculate_mortal_reward(mortal_count);
	switch (cmd)
    {
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the veil rewards you.\n\r", GET_SHORT(mob));
			send_to_room(buf, CHAR_REAL_ROOM(mob));
			mob_aq_reward(reward, mob);
			break;
    }
    return FALSE;
}



int veil_memory_leech(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    
	OBJ *obj = NULL;
	CHAR *vict, *next_vict;

    switch (cmd)
    {
        case MSG_MOBACT:

            if (mob->specials.fighting)
            {
                vict = get_random_victim_fighting(mob);
                if (!vict) break;

                switch (number(0, 3))
                {
                    case 0: /* Memory Drain (STR) */
                        act("$n pulses and drains strength from $N!", 0, mob, 0, vict, TO_ROOM);
                        damage(mob, vict, number(120, 220), TYPE_UNDEFINED, DAM_MAGICAL);
                        affect_apply(vict, SPELL_CURSE, 0, APPLY_STR, -2, AFF_CURSE, 0);
                        GET_HIT(mob) += number(40, 80);
                        break;

                    case 1: /* Memory Drain (DEX) */
                        act("$n ripples and distorts $N's movements!", 0, mob, 0, vict, TO_ROOM);
                        damage(mob, vict, number(120, 220), TYPE_UNDEFINED, DAM_MAGICAL);
                        affect_apply(vict, SPELL_CURSE, 0, APPLY_DEX, -2, AFF_CURSE, 0);
                        GET_HIT(mob) += number(40, 80);
                        break;

                    case 2: /* Mind Slip (INT) */
                        act("$n seeps into $N's thoughts!", 0, mob, 0, vict, TO_ROOM);
                        damage(mob, vict, number(100, 200), TYPE_UNDEFINED, DAM_MAGICAL);
                        affect_apply(vict, SPELL_CURSE, 0, APPLY_INT, -2, AFF_CURSE, 0);
                        break;

                    default: /* Weak lash */
                        act("$n lashes out weakly at $N.", 0, mob, 0, vict, TO_ROOM);
                        damage(mob, vict, number(80, 160), TYPE_UNDEFINED, DAM_PHYSICAL);
                        break;
                }
            }
            break;

        case MSG_DIE:

            act("$n collapses into a dim glow that condenses into a small gem in your inventory.",
                0, mob, 0, 0, TO_ROOM);
				
			obj=read_object(SKILLGEM, VIRTUAL);

            for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
			{
				next_vict = CHAR_NEXT_IN_ROOM(vict);
				if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
				
				obj_to_char(obj,vict);
				
			}

            break;
    }

    return FALSE;
}

int veil_refraction_engine(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    CHAR *vict, *next_vict;   
	OBJ *obj = NULL;
	
    switch (cmd)
    {
        case MSG_MOBACT:

            if (mob->specials.fighting)
            {
               
                switch (number(0, 4))
                {
                    case 0: /* Refraction Pulse (AoE) */
                        act("$n releases a pulse of refracted energy!",
                            0, mob, 0, 0, TO_ROOM);

                        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                        {
                            next_vict = CHAR_NEXT_IN_ROOM(vict);
                            if (IS_NPC(vict) || !IS_MORTAL(vict)) continue;

                            damage(mob, vict, number(180, 320),TYPE_UNDEFINED, DAM_MAGICAL);
                        }
                        break;

                    case 1: /* Focused Beam */
                        vict = get_random_victim_fighting(mob);
                        if (vict)
                        {
                            act("$n focuses a beam of light into $N!",
                                0, mob, 0, vict, TO_ROOM);

                            damage(mob, vict, number(220, 380),  TYPE_UNDEFINED, DAM_MAGICAL);
                        }
                        break;

                    case 2: /* Crystallize Defense */
                        act("$n hardens into a dense crystalline form.",   0, mob, 0, 0, TO_ROOM);

                        affect_apply(mob, SPELL_ARMOR, 0,  APPLY_ARMOR, -40, 2, 0);
                        break;

                    case 3: /* Shard Burst */
                        vict = get_random_victim_fighting(mob);
                        if (vict)
                        {
                            act("Shards erupt from $n and tear into $N!",  0, mob, 0, vict, TO_ROOM);

                            damage(mob, vict, number(200, 340),  TYPE_UNDEFINED, DAM_PHYSICAL);
                        }
                        break;

                    default: /* Basic attack */
                        vict = get_random_victim_fighting(mob);
                        if (vict)
                        {
                            act("$n lashes out with jagged crystal at $N!", 0, mob, 0, vict, TO_ROOM);

                            damage(mob, vict, number(150, 260),  TYPE_UNDEFINED, DAM_PHYSICAL);
                        }
                        break;
                }
            }
            break;

        case MSG_DIE:

            act("The Refraction Engine collapses inward, condensing into radiant crystalline gems!", 0, mob, 0, 0, TO_ROOM);

           obj=read_object(SKILLGEM, VIRTUAL);

            for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
			{
				next_vict = CHAR_NEXT_IN_ROOM(vict);
				if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
				
				obj_to_char(obj,vict);
				
			}


            break;
    }

    return FALSE;
}

int veil_echo_block(CHAR *gatekeeper, CHAR *ch, int cmd, char *arg)
{
    bool bReturn = FALSE;

    if (cmd == MSG_MOBACT && chance(8))
    {
        do_say(gatekeeper, "The veil doesn't forget!.", CMD_SAY);
    }

    if (ch && (cmd == CMD_NORTH || cmd == CMD_EAST ||cmd == CMD_WEST ||cmd == CMD_SOUTH))
    {
        do_say(gatekeeper, "The veil won't let you leave.", CMD_SAY);
        bReturn = TRUE;
    }
    return bReturn;
}



// Assign Spec for the zone. Sets all other specs.
// First Param - Object, Room or Mob Number.  Define it up above.
// Second Param - the name of the function that is for the mob Usually a shorthand for your zone.
void assign_veilofunspentfate(void)
{
    /*Objects */
  

    /*Rooms */
	assign_room(DIVIDING_VEIL_ROOM,dividing_veil_room);

    /*Mobs */
    assign_mob(VEIL_BROKER, daily_qp_keeper);
   
   
   assign_mob(ECHO_OF_THE_ALMOST, veil_mob_echo);
   assign_mob(WHISPER_OF_REGRET, veil_mob_echo);
   assign_mob(ECHO_OF_THE_ALMOST_VARIANT, veil_mob_echo);
   assign_mob(SHARDBOUND_FRAGMENT, veil_mob_echo);
   assign_mob(WHISPER_OF_REGRET_VARIANT, veil_mob_echo);
   assign_mob(FRACTURED_REVENANT, veil_mob_echo);
   assign_mob(SHARDBOUND_KEEPER, veil_mob_echo);
   assign_mob(CONVERGENCE_WRAITH, veil_mob_echo_upgraded);
   assign_mob(VEILBOUND_ARBITER, veil_mob_echo_upgraded);
   assign_mob(VEILTORN_DRIFTER, veil_mob_echo_upgraded);
   assign_mob(FATEBREAKER_WARDEN, veil_mob_echo_upgraded);
   assign_mob(CONVERGENCE_HORROR, veil_mob_echo_upgraded);
   
   assign_mob(MEMORY_LEECH, veil_memory_leech);
   assign_mob(REFRACTION_ENGINE, veil_refraction_engine);
   
   assign_mob(VEIL_WISP, veil_echo_block);
   assign_mob(ECHO_FRAGMENT, veil_echo_block);
   assign_mob(SHARDBOUND_HORROR, veil_echo_block);
	
	
}