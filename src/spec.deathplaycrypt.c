/* Specs for area Death's Playground Crypt by Arodtanjoe

** Written by Arodtanjoe - March 2025
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


#define BUTTOM_ROOM_ONE_START 21613
#define BUTTOM_ROOM_ONE_END 21614

#define SMASH_JAR_ROOM_ONE_START 21614
#define SMASH_JAR_ROOM_ONE_END 21630

#define SEPULCHER_ROOM_ONE 21645
#define SEPULCHER_ROOM_TWO 21646
#define SEPULCHER_ROOM_THREE 21650
#define SEPULCHER_ROOM_FOUR 21651
#define SEPULCHER_ROOM_FIVE 21652
#define SEPULCHER_SEAL_ROOM 21649

#define CRYPT_TRANSPORT_ROOM 21654

/*Objects */

#define SACRIFICIAL_JAR_ONE 21601
#define SACRIFICIAL_JAR_TWO 21610

#define SEPULCHER_ASCENSION 21608

#define SIGIL_NERATH_PRIEST 21609


/*Mobs */

#define NERATH_PRIEST 21615

/*Miscellaneous strings */
// Generic States that are shifted to indicate different stages.

#define STATE1 (1 << 0) // 1
#define STATE2 (1 << 1) // 2
#define STATE3 (1 << 2) // 4
#define STATE4 (1 << 3) // 8

#define NARETH_BLESSING_NAME "Blessing of Nareth"

int dpc_button_room_one(int room,CHAR *ch,int cmd,char *argument) {
	
  char buf[MAX_INPUT_LENGTH];
  
  if (cmd == MSG_ZONE_RESET)
  {

    if (world[real_room(BUTTOM_ROOM_ONE_START)].dir_option[DOWN]->to_room_r != -1)
    {
      world[real_room(BUTTOM_ROOM_ONE_START)].dir_option[DOWN]->to_room_r = -1;
      world[real_room(BUTTOM_ROOM_ONE_END)].dir_option[UP]->to_room_r = -1;
      send_to_room("The granite wall seals shut.\n\r", real_room(BUTTOM_ROOM_ONE_START));
      send_to_room("The granite wall seals shut.\n\r", real_room(BUTTOM_ROOM_ONE_END));
    }
  }
	
	if (ch && cmd == CMD_MOVE )
    {
        one_argument(argument, buf);
        if (*buf && !strncmp(buf, "button", MAX_INPUT_LENGTH))
        {
			
			send_to_room("The granite wall opens revealing a path down.\n\r", real_room(BUTTOM_ROOM_ONE_START));
			send_to_room("The granite wall opens above you, opened by something or someone else. \n\r", real_room(BUTTOM_ROOM_ONE_END));
			world[real_room(BUTTOM_ROOM_ONE_START)].dir_option[DOWN]->to_room_r = real_room(BUTTOM_ROOM_ONE_END);
			world[real_room(BUTTOM_ROOM_ONE_END)].dir_option[UP]->to_room_r = real_room(BUTTOM_ROOM_ONE_START);
			return TRUE;
			
			
        }
    } 

  return FALSE;
}

int dpc_smash_jar_room_one(int room,CHAR *ch,int cmd,char *argument) {

   
  if (cmd == MSG_ZONE_RESET)
  {

    if (world[real_room(SMASH_JAR_ROOM_ONE_START)].dir_option[EAST]->to_room_r != -1)
    {
      world[real_room(SMASH_JAR_ROOM_ONE_START)].dir_option[EAST]->to_room_r = -1;
      world[real_room(SMASH_JAR_ROOM_ONE_END)].dir_option[WEST]->to_room_r = -1;
      send_to_room("The granite wall slides shut.\n\r", real_room(SMASH_JAR_ROOM_ONE_START));
      send_to_room("The granite wall slides shut.\n\r", real_room(SMASH_JAR_ROOM_ONE_END));
    }
  }	
	
  return FALSE;
}



int determine_jar_loot(int chance_adjust)
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


int sacrifical_jar_large(OBJ *obj, CHAR *ch, int cmd, char *arg)
{

    char buf[MAX_INPUT_LENGTH]; 
	OBJ  *tmp;	
    int jar_loot_object;

    if (ch && cmd == CMD_UNKNOWN )
    {
		
        arg = one_argument(arg, buf);
        if (*buf && is_abbrev(buf, "smash"))
        {
			arg = one_argument(arg, buf);
			if (*buf && is_abbrev(buf, "jar"))
			{
				act("The pot smashes to the ground and something sparkly hits the ground.", 0, ch, obj, 0, TO_ROOM);
				act("The pot smashes to the ground and something sparkly hits the ground.", 0, ch, obj, 0, TO_CHAR);
				jar_loot_object = determine_jar_loot(0);
				tmp = read_object(jar_loot_object, VIRTUAL);    
				obj_to_room(tmp, CHAR_REAL_ROOM(ch));				
				extract_obj(obj);
				
				//Give a chance to open the path to the next part of the zone.
				if(chance(40))
				{					
					if (world[real_room(SMASH_JAR_ROOM_ONE_START)].dir_option[EAST]->to_room_r == -1)
					{
					  world[real_room(SMASH_JAR_ROOM_ONE_START)].dir_option[EAST]->to_room_r = real_room(SMASH_JAR_ROOM_ONE_END);
					  world[real_room(SMASH_JAR_ROOM_ONE_END)].dir_option[WEST]->to_room_r = real_room(SMASH_JAR_ROOM_ONE_START);
					  send_to_room("The granite wall seals shut.\n\r", real_room(SMASH_JAR_ROOM_ONE_START));
					  send_to_room("The granite wall seals shut.\n\r", real_room(SMASH_JAR_ROOM_ONE_END));
					  send_to_room("A Rumbling can be heard somewhere in the crypt", CHAR_REAL_ROOM(ch));
					}
					
				}
				
				
				return TRUE;
			}          
			
        }
    }
    return FALSE;
}


int sacrifical_jar_medium(OBJ *obj, CHAR *ch, int cmd, char *arg)
{

    char buf[MAX_INPUT_LENGTH]; 
	OBJ  *tmp;	
    int jar_loot_object;

    if (ch && cmd == CMD_UNKNOWN )
    {
		
        arg = one_argument(arg, buf);
        if (*buf && is_abbrev(buf, "smash"))
        {
			arg = one_argument(arg, buf);
			if (*buf && is_abbrev(buf, "jar"))
			{
				act("The pot smashes to the ground and something sparkly hits the ground.", 0, ch, obj, 0, TO_ROOM);
				act("The pot smashes to the ground and something sparkly hits the ground.", 0, ch, obj, 0, TO_CHAR);
				jar_loot_object = determine_jar_loot(10);
				tmp = read_object(jar_loot_object, VIRTUAL);    
				obj_to_room(tmp, CHAR_REAL_ROOM(ch));				
				extract_obj(obj);				
				
				return TRUE;
			}          
			
        }
    }
    return FALSE;
}




bool check_sepulchers()
{

    bool allHaveSymbols = FALSE;
    int sepulcher_room_number;    
	int sepulcher_rooms[5] = {SEPULCHER_ROOM_ONE,SEPULCHER_ROOM_TWO,SEPULCHER_ROOM_THREE,SEPULCHER_ROOM_FOUR,SEPULCHER_ROOM_FIVE};
	OBJ *sepulcher;
	
	for (int i = 0; i < 4; i++)
    {
        // Grab the room and mob based on index value.
        sepulcher_room_number = sepulcher_rooms[i];		
		 
		sepulcher = get_obj_by_vnum_in_room(SEPULCHER_ASCENSION, real_room(sepulcher_room_number));
		
		if (OBJ_CONTAINS(sepulcher))
		{
			OBJ *temp_obj = OBJ_CONTAINS(sepulcher);
			
			if(V_OBJ(temp_obj) == SIGIL_NERATH_PRIEST){
				allHaveSymbols = TRUE;
			}
			else
			{
				allHaveSymbols = FALSE;
			}
		}
		else
		{
			allHaveSymbols = FALSE;
		}
        
    }
    return allHaveSymbols;
}

void extract_sigil_sepulchers()
{
    
    int sepulcher_room_number;    
	int sepulcher_rooms[5] = {SEPULCHER_ROOM_ONE,SEPULCHER_ROOM_TWO,SEPULCHER_ROOM_THREE,SEPULCHER_ROOM_FOUR,SEPULCHER_ROOM_FIVE};
	OBJ *sepulcher;
	
	for (int i = 0; i < 4; i++)
    {
        // Grab the room and mob based on index value.
        sepulcher_room_number = sepulcher_rooms[i];		
		 
		sepulcher = get_obj_by_vnum_in_room(SEPULCHER_ASCENSION, real_room(sepulcher_room_number));
		
		if (OBJ_CONTAINS(sepulcher))
		{
			OBJ *temp_obj = OBJ_CONTAINS(sepulcher);
			
			if(temp_obj){
				extract_obj(temp_obj);
			}			
		}	        
    }
    
}


int sepulcher_ascension(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
	
	bool summonUndead = FALSE;
	CHAR *nerath_priest;
    int nerath_priest_nr;
	
	
	if (cmd == CMD_PUT)
    {

        if (OBJ_CONTAINS(obj))
        {
            send_to_char("Only a single object can fit.\r\n", ch);
            return TRUE;
        }
		
		//Check all 5 Sepulchers to see if they all have the sigil.
		summonUndead = check_sepulchers();
		
		if(summonUndead){			
			nerath_priest_nr = real_mobile(NERATH_PRIEST);
			nerath_priest = read_mobile(nerath_priest_nr, REAL);
			
			char_to_room(nerath_priest, real_room(SEPULCHER_SEAL_ROOM));			
			send_to_room("A mysterious figure appears in the middle of the room.\r\n",CHAR_REAL_ROOM(ch));

			extract_sigil_sepulchers();
			
		}
		
		
    }
	return FALSE;
	
}

int nareth_blessing_func(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg)
{

    int hp_gain;
	int mana_gain;

    if (cmd == MSG_REMOVE_ENCH)
    {
        send_to_char("You feel the boosted endurance leave your body.\n\r", ench_ch);
        return FALSE;
    }
    if (cmd == CMD_QUIT)
    {
        if (ch != ench_ch)
            return FALSE;
        send_to_char("Your life is worth too much to quit.\n\r", ench_ch);
        return TRUE;
    }

    // On Tick, lose 10% of your HP, but cant drop below 100 HP.

    if (ench_ch && cmd == MSG_TICK)
    {
        hp_gain = number(100,200);
		mana_gain = number(40,80);
		
		GET_HIT(ench_ch) = GET_HIT(ench_ch) +  hp_gain;
		GET_MANA(ench_ch) =  GET_MANA(ench_ch) + mana_gain;		
    }

    return FALSE;
}

int nerath_high_priest(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict,*next_vict;
	
	char buf[MAX_INPUT_LENGTH];
	bool keyword_said = FALSE;
	
	 if(cmd==MSG_MOBACT && mob->specials.fighting) {
		act("$n says 'There is no need to fight. Just resite the keyword.",0,mob,0,0,TO_ROOM);
		for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room){
		  if(vict->specials.fighting==mob) stop_fighting(vict->specials.fighting);
		  }
		stop_fighting(mob);
		GET_HIT(mob)=GET_MAX_HIT(mob);
		return FALSE;
	  }
	 
	 if(cmd==MSG_TICK && !mob->specials.fighting) {
		 if(chance(30)){
			send_to_room("Bound in dust, yet voices remain\r\n",CHAR_REAL_ROOM(mob));
			send_to_room("In silent halls where dead ones reign.\r\n",CHAR_REAL_ROOM(mob));
			send_to_room("Three words lie waiting, lost in tomes,\r\n",CHAR_REAL_ROOM(mob));
			send_to_room("Keys to fate in hallowed stones.\r\n",CHAR_REAL_ROOM(mob));
			send_to_room("One grants passage, one grants might,\r\n",CHAR_REAL_ROOM(mob));
			send_to_room("One will cast you into night.\r\n",CHAR_REAL_ROOM(mob));
			send_to_room("Seek the echoes, read them true,\r\n",CHAR_REAL_ROOM(mob));
			send_to_room("The library holds what’s due to you.\r\n",CHAR_REAL_ROOM(mob)); 
		 }
		 
		 
		 
	 }
	 
	 

	  
	  if(cmd==CMD_SAY){
			
			
			
			arg = one_argument(arg, buf);
			if (*buf && is_abbrev(buf, "oathbound"))
			{
				act("$n says 'Your devotion is acknowledged. The path ahead is veiled in darkness, yet your steps shall not falter. Walk forward, and may the echoes of the fallen guide your way.",0,mob,0,0,TO_ROOM);
				keyword_said = TRUE;
				
				for(vict=world[CHAR_REAL_ROOM(mob)].people;vict;vict=next_vict){
				  next_vict=vict->next_in_room;
				  
				  if (IS_NPC(vict)){ continue; }
				  
				  send_to_char("You are sent deeper into the crypt!\n\r",vict);
				  char_from_room(vict);
				  char_to_room(vict, real_room(CRYPT_TRANSPORT_ROOM));/*move all chars to this room*/
				  do_look(vict,"",CMD_LOOK);
				}
				
				
				
				
				
			}
			if (*buf && is_abbrev(buf, "eternal"))
			{
				act("$n says 'You seek the gift of endurance, yet eternity is a burden few can bear. So be it—let the veil of undeath not claim you so easily. May your flesh resist decay, and your soul linger beyond its time.",0,mob,0,0,TO_ROOM);
				
				 for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
                    {
                        next_vict = vict->next_in_room;

                        if (IS_NPC(vict)){ continue; }
                            
                        if (IS_MORTAL(vict))
                        {
                            enchantment_apply(ch, FALSE, NARETH_BLESSING_NAME, 0, 35, ENCH_INTERVAL_TICK, 3, APPLY_DAMROLL, 0, 0, nareth_blessing_func);
                        }
                    }
				
				
				keyword_said = TRUE;
				
				
			}
			if (*buf && is_abbrev(buf, "forsaken"))
			{
				act("$n says 'Ah… you have spoken the word of the lost. This place is not for you. The forsaken do not remain—they are cast into the void, beyond time and memory. So shall you be.",0,mob,0,0,TO_ROOM);
				
				act("$n says 'The Path will soon be open.'",0,mob,0,0,TO_ROOM);
				
				keyword_said = TRUE;
			}
		  
		  
		   //Once a single keyword is said, extract the NPC.
		   if(keyword_said){
			act("$n says 'Farewell and good luck!.",0,mob,0,0,TO_ROOM);
			
			act("$n decays before you",0,mob,0,0,TO_ROOM);
			
			char_from_room(mob);
		   }
		   
		   
		   return TRUE;
		  
	  }
	  
	  
	return FALSE;
}





void assign_deathplaygroundcrypt(void) {

/*Objects */
assign_obj(SACRIFICIAL_JAR_ONE, sacrifical_jar_large);
assign_obj(SEPULCHER_ASCENSION, sepulcher_ascension);
assign_obj(SACRIFICIAL_JAR_TWO, sacrifical_jar_medium);


/*Rooms*/ 
assign_room(BUTTOM_ROOM_ONE_START,dpc_button_room_one);
assign_room(SMASH_JAR_ROOM_ONE_START,dpc_smash_jar_room_one);



/*Mobs*/
assign_mob(NERATH_PRIEST, nerath_high_priest);

 
}





