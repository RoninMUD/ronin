/*

    Specs for Norway, by Solmyr
    Written by Solmyr for RoninMUD
    
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "structs.h"
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

/* Room defines */

#define WESTERN_SHORE  19006
#define FARM_ROOM      19016
#define FARM_HOUSE     19032
#define BARN           19033

/* Object defines */

#define CLOSET         19000
#define CHAIR          19002
#define COPPER_KEY     19004
#define BEAR_TOOTH     19005
#define WOOL_STRING    19006
#define BEAR_AMULET    19007
#define SECOND_CLOSET  19010
#define INSTRUCTIONS   19011

/* Mobile defines */

#define FARMER         19000
#define LYNX           19004
#define BEAR           19007
#define LEMMING        19008
#define ADDER          19009
#define WOLVERINE      19010
#define MANEATER       19011


/* Room specs */

/* Some falling rocks almost hit people in room 19007 */
int norway_western_shore(int room, CHAR *ch, int cmd, char *arg)
{
	if(!world[room].people) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;
	if(chance(90)) return FALSE;
	send_to_room("Rocks and boulders fall down the mountainside, missing you only by a few feet.\n\r", room);
	return FALSE;
}

/* People can enter the farmhouse (19032) or the barn (19033) from the farmroom (19016) */
int norway_farm_room(int room, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	
	if(!ch) return FALSE;
	if((cmd != CMD_ENTER) || !AWAKE(ch)) return FALSE;
	
	one_argument(arg, buf);
	if(!*buf)
    return FALSE;

	if(!strncmp(buf, "house", 5) || !strncmp(buf, "farmhouse", 9))
	{
		if(GET_POS(ch) == POSITION_RIDING)
		{
			send_to_char("Dismount first.\n\r", ch);
			return TRUE;
		}
		act("$n enters the farmhouse through the door in the southern wall.\n\r",TRUE,ch,0,0,TO_ROOM);
		send_to_char("You enter the farmhouse through the door in the southern wall.\n\r\n\r",ch);
		char_from_room(ch);
		char_to_room(ch, real_room(FARM_HOUSE));
		act("$n arrives through the kitchen door.\n\r",TRUE,ch,0,0,TO_ROOM);
		do_look(ch, "", CMD_LOOK);
		return TRUE;
	}
	
	if(!strncmp(buf, "barn", 4))
	{
		if(GET_POS(ch) == POSITION_RIDING)
		{
			send_to_char("Dismount first.\n\r", ch);
			return TRUE;
		}
		act("$n enters the barn through the doorway in the western wall.",TRUE,ch,0,0,TO_ROOM);
		send_to_char("You enter the barn through the doorway in the western wall.\n\r\n\r",ch);
		char_from_room(ch);
		char_to_room(ch, real_room(BARN));
		act("$n enters the room through the doorway.",TRUE,ch,0,0,TO_ROOM);
		do_look(ch, "", CMD_LOOK);
		return TRUE;
	}
	return FALSE;
}

/* A cheesy spec showing a golden eagle getting some grub up on the mountain plateau */
int norway_golden_eagle(int room, CHAR *ch, int cmd, char *arg)
{
	if(!world[room].people) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;
	if(chance(95)) return FALSE;
	send_to_room("A golden eagle, seemingly appearing out of nowhere, snatches a roedeer calf\n\r", room);
	send_to_room("and flies off with it, only barely able to ascend under the heavy load.\n\r", room);
	return FALSE;
}

/* Object specs */

/* "open drawer" in the room of the closet (19000) will give ch
   a key for the other compartement in the same closet. Because
   a container only has one compartement, and so you can't get
   a shitload of keys, the spec also puts an almost duplicate of
   the closet (19010) into the room instead of the original one. */
int norway_closet(OBJ *closet, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];
	OBJ *obj2, *obj3, *obj4;
	
	if(!ch) return FALSE;
	if(cmd != CMD_OPEN) return FALSE;
	
	one_argument(arg, buf);
  if(!*buf) return FALSE;
  
  if(!strncmp(buf, "drawer", 6))
  {
  	act("$n opens the closet drawer and finds a key inside!",TRUE,ch,0,0,TO_ROOM);
  	send_to_char("You open the closet drawer and find a key inside!\n\r",ch);
  	obj2 = read_object(COPPER_KEY, VIRTUAL);
  	obj_to_char(obj2, ch);
  	extract_obj(closet);
  	obj2 = read_object(SECOND_CLOSET, VIRTUAL);
  	obj_to_room(obj2, real_room(FARM_HOUSE));
  	obj3 = read_object(WOOL_STRING, VIRTUAL);
  	obj_to_obj(obj3, obj2);
  	obj4 = read_object(INSTRUCTIONS, VIRTUAL);
  	obj_to_obj(obj4, obj2);
  	
  	return TRUE;
  }
  return FALSE;
}

int count_obj_in_room(int onum, int rnum)
{
	OBJ *obj;
	int i = 0;
	
	for(obj = world[real_room(rnum)].contents; obj; obj = obj->next_content)
	{
		if(onum == V_OBJ(obj)) i++;
	}
	return i;
}

/* If someone has triggered the last spec, the original closet comes back at zone repop */
int norway_closet_reset(int room, CHAR *ch, int cmd, char *arg)
{
	OBJ *obj;
	int closet = 0;
	int chairs;
	
	if(cmd != MSG_ZONE_RESET) return FALSE;
	chairs = count_obj_in_room(CHAIR, real_room(FARM_HOUSE));
	while(chairs < 2)
	{
		obj = read_object(CHAIR, VIRTUAL);
		obj_to_room(obj, real_room(FARM_HOUSE));
		chairs++;
	}
	if(!(closet = real_object(SECOND_CLOSET))) return FALSE;
	if(!(obj = get_obj_in_list_num(closet, world[room].contents))) return FALSE;
	extract_obj(obj);
	return FALSE;
}

/* By combining the bear tooth and wool string you create an amulet */
int norway_amulet(OBJ *tooth, CHAR *ch, int cmd, char *arg)
{
	OBJ *obj2;
	char arg2[MAX_INPUT_LENGTH];
	
	if(ch != tooth->carried_by) return FALSE;
	if(cmd == CMD_UNKNOWN)
	{
		one_argument(arg, arg2);
		
		if(!is_abbrev(arg, "thread"))
			return FALSE;
		
		if(!*arg2)
		{
			send_to_char("Thread what?\n\r", ch);
			return TRUE;
		}
		
		string_to_lower(arg2);
		if(strcmp(arg2, "string tooth"))
		{
			send_to_char("What is it you want to thread?\n\r", ch);
			return TRUE;
		}
		
		if(!get_obj_in_list_num(real_object(WOOL_STRING), ch->carrying))
		{
			send_to_char("You don't have the wool string.\n\r", ch);
			return TRUE;
		}
		
		if(!get_obj_in_list_num(real_object(BEAR_TOOTH), ch->carrying))
		{
			send_to_char("You don't have the bear tooth.\n\r", ch);
			return TRUE;
		}
		
		for(obj2 = ch->carrying; obj2; obj2 = obj2->next_content)
		{
			if((V_OBJ(obj2) == BEAR_TOOTH) || (V_OBJ(obj2) == WOOL_STRING))
			{
				extract_obj(obj2);
			}
		}
		
		act("$n threads a piece of string through a tooth and makes an amulet.",0,ch,0,0, TO_ROOM);
		act("You thread a piece of string through a tooth and make an amulet.",0,ch,0,0, TO_CHAR);
		obj2 = read_object(BEAR_AMULET, VIRTUAL);
		obj_to_char(obj2, ch);
	}
	return FALSE;
}


/* Mobile specs */

/* Specs for farmer (19000) */
int norway_farmer(CHAR *farmer, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict;
	
	if(cmd != MSG_MOBACT) return FALSE;
	
	if((chance(5)) && (GET_POS(farmer) == POSITION_STANDING))
	{
	  act("There is a resonating crack as $n tries to straighten out his back.",0,farmer,0,0,TO_ROOM);
	}
	
	if(chance(18) && farmer->specials.fighting)
	{
    vict = get_random_victim_fighting(farmer);
    if(!vict) return FALSE;
    act("You hit $N over the head with your shovel.",0,farmer,0,vict,TO_CHAR);
    act("$n clobs $N over the head with a rusty shovel!",0,farmer,0,vict,TO_NOTVICT);
    act("$n clobs you over the head with a rusty shovel!",0,farmer,0,vict,TO_VICT);
    damage(farmer, vict, number(50,100), TYPE_UNDEFINED, DAM_NO_BLOCK);
  }
	return FALSE;
}

/* Specs for Lynx (19004) */
int norway_lynx(CHAR *lynx, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict;
	
	if(cmd != MSG_MOBACT) return FALSE;
	
	if(chance(7) && !lynx->specials.fighting)
	{
		vict = get_random_victim(lynx);
		if(!vict) return FALSE;
		act("You leap at $N.",0,lynx,0,vict,TO_CHAR);
		act("Suddenly $n leaps onto $N, sinking its teeth into $S neck!",0,lynx,0,vict,TO_NOTVICT);
		act("You scream in surprise and pain as you feel sharp teeth puncturing your neck!",0,lynx,0,vict,TO_VICT);
		damage(lynx, vict, number(20,50), TYPE_UNDEFINED, DAM_NO_BLOCK);
		hit(lynx, vict, TYPE_UNDEFINED);
	}
	
	if(chance(20) && lynx->specials.fighting)
	{
		vict = get_random_victim_fighting(lynx);
		if(!vict) return FALSE;
		act("You grab $N's leg and rips his flesh with your bite.",0,lynx,0,vict,TO_CHAR);
		act("$n grabs $N's leg with its paw and rips through his flesh with its sharp teeth.",0,lynx,0,vict,TO_NOTVICT);
		act("$n grabs your leg with its paw and rips through your flesh with its sharp teeth.",0,lynx,0,vict,TO_VICT);
		damage(lynx, vict, number(40,60), TYPE_UNDEFINED, DAM_NO_BLOCK);
	}
	return FALSE;
}

/* Specs for bear (19007) */
int norway_bear(CHAR *bear, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict;
	
	if(cmd != MSG_MOBACT) return FALSE;
	
	if(chance(20) && bear->specials.fighting)
	{
		vict = get_random_victim_fighting(bear);
		if(!vict) return FALSE;
		act("You punch $N in the face, HAHA!",0,bear,0,vict,TO_CHAR);
		act("$n swings $s large paw into the face of $N.",0,bear,0,vict,TO_NOTVICT);
		act("$n's paw swings right into your face!",0,bear,0,vict,TO_VICT);
		damage(bear, vict, 60, TYPE_UNDEFINED, DAM_NO_BLOCK);
	}
	return FALSE;
}

/* Specs for lemmings (19008) */
int norway_lemming(CHAR *lemming, CHAR *ch, int cmd, char *arg)
{
	if(cmd != MSG_MOBACT) return FALSE;
	
	if(chance(30) && !lemming->specials.fighting)
	{
		act("You are so pissed off right now and intend to show that to everyone.",0,lemming,0,0,TO_CHAR);
		act("$n bounces around in rage, hissing and grinding its teeth at everyone.",0,lemming,0,0,TO_ROOM);
	}	
	return FALSE;
}

/* Specs for the adder */
int norway_viper(CHAR *adder, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict;
	
	if(cmd != MSG_MOBACT) return FALSE;
	
	if(chance(5) && !adder->specials.fighting)
	{
		vict = get_random_victim(adder);
		if(!vict) return FALSE;
		act("HEY THERE, I'M SLITHERING HERE! Damn you $N for stepping on me! Take this!",0,adder,0,vict,TO_CHAR);
		act("$N accidentally stepped on $n, resulting in $n biting $N in the calf.",0,adder,0,vict,TO_NOTVICT);
		act("You accidentally step on $n.\n\r$n bites you in the calf, filling you with poison.",0,adder,0,vict,TO_VICT);
		damage(adder, vict, 10, TYPE_UNDEFINED, DAM_NO_BLOCK);
		spell_poison(GET_LEVEL(adder), adder, vict, 0);
	}	
	return FALSE;
}

/* The wolverine locks onto the tanks leg and hurts him over a period of time */
int norway_wolverine(CHAR *wolverine, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict;
	
	if(cmd != MSG_MOBACT) return FALSE;
	if(!wolverine->specials.fighting) return FALSE;
	
	if(chance(35))
	{
		vict = wolverine->specials.fighting;
		if(!vict) return FALSE;
		act("You lock on to $N's leg",0,wolverine,0,vict, TO_CHAR);
		act("$n wraps his jaws around your leg and locks on tight!",0,wolverine,0,vict, TO_VICT);
		act("$n wraps his jaws around $N's leg and locks on tight!",0,wolverine,0,vict, TO_NOTVICT);
		damage(wolverine, vict, number(40,70), TYPE_UNDEFINED, DAM_NO_BLOCK);
	}
	return FALSE;	
}

/* The maneating bear either does damage to the tank or someone else fighting him */
int norway_maneater(CHAR *bear, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict;
	
	if(cmd != MSG_MOBACT) return FALSE;
	
	if(chance(25) && bear->specials.fighting)
	{
		switch(number(0,1))
		{
			case 0:
			{
				vict = bear->specials.fighting;
				if(!vict) return FALSE;
				act("You grab $N and sink your teeth into $S abdomen.",0,bear,0,vict, TO_CHAR);
				act("$n grabs you with his paws and sinks $s teeth into your abdomen.",0,bear,0,vict, TO_VICT);
				act("$n grabs $N with his paws and sinks $s teeth into $S abdomen.",0,bear,0,vict, TO_NOTVICT);
				damage(bear, vict, number(80, 200), TYPE_UNDEFINED, DAM_NO_BLOCK);
				WAIT_STATE(vict, number(4, 7) RL_SEC);
				break;
			}
			case 1:
			{
				vict = get_random_victim_fighting(bear);
				if(!vict) return FALSE;
				act("You throw your head around and bite $N.",0,bear,0,vict, TO_CHAR);
				act("$n throws $s head around and bites you!",0,bear,0,vict, TO_VICT);
				act("$n throws $s head around and bites $N.",0,bear,0,vict, TO_NOTVICT);
				damage(bear, vict, number(40, 125), TYPE_UNDEFINED, DAM_NO_BLOCK);
				WAIT_STATE(vict, number(2, 6) RL_SEC);
				break;
			}			
		}
	}
	return FALSE;
}

/* Assigns */

void assign_norway()
{
	int i;
	
	assign_room(WESTERN_SHORE , norway_western_shore);
	assign_room(FARM_ROOM     , norway_farm_room);
	for(i = 19018; i < 19026; i++) assign_room(i, norway_golden_eagle);
	assign_room(FARM_HOUSE    , norway_closet_reset);
	assign_obj(CLOSET         , norway_closet);
	assign_obj(BEAR_TOOTH     , norway_amulet);
	assign_mob(FARMER         , norway_farmer);
	assign_mob(LYNX           , norway_lynx);
	assign_mob(BEAR           , norway_bear);
	assign_mob(LEMMING        , norway_lemming);
	assign_mob(ADDER          , norway_viper);
	assign_mob(WOLVERINE      , norway_wolverine);
	assign_mob(MANEATER       , norway_maneater);
}
