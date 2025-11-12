/*spec.TurkeyTakeover.c - Specs for TurkeyTakeover by Fisher

     Written by Fisher for RoninMUD
     Last Modification Date: 11/23/2021

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
#include "reception.h"
#include "mob.spells.h"

/*Rooms */
#define TURKEY_PORTAL 18103
#define TURKEY_GATE 18118
#define TURKEY_GATE_TWO 18124
#define TURKEY_GATE_THREE 18113
#define TOWN_ENTRANCE 18136
#define ROAD_SPOT_ONE 18129
#define ROAD_SPOT_TWO 18133
#define ROAD_SPOT_THREE 18125
#define ISSAC_ALTAR 18100
#define FAITH_ALTAR 18106
#define PILGRIM_LEADER_ROOM 18137

/*Objects */
#define BASKET 18100
#define TURKEY 18101
#define CRATE 18102
#define CREST 18103
#define BANNER 18104
#define BEAK 18105
#define FEATHERS 18106
#define CHEST 18107
#define CROWN 18108
#define SHAWL 18109
#define VAMBRACES 18110

/*Mobs */
#define TURKEY_TERROR 18100
#define ISSAC_PRIEST 18101
#define FAITH_PRIESTESS 18102
#define PILGRIM_LEADER 18103

//Pilgrim Army
#define PILGRIM_SOLDIER 18104
#define PILGRIM_MAGE 18105
#define PILGRIM_BARD 18106

//Turkey Army
#define TURKEY_TOT_BERSERKER 18107
#define TURKEY_TOT_WIZARD 18108
#define TURKEY_TOT_SOLDIER 18109
#define TURKEY_TOT_COMMANDER 18110
#define TURKEY_TOT_GENERAL 18111

//Summons
#define PILGRIM_BERSERKER 18112
#define PILGRIM_VANGUARD 18113
#define PILGRIM_ARCHMAGE 18114
#define TURKEY_DEATHKNIGHT 18115
#define TURKEY_LICH 18116
#define TURKEY_SHADOW 18117
#define TURKEY_ZOMBIE 18118

/*Miscellaneous strings */
#define STATE1 (1 << 0)    //1
#define STATE2 (1 << 1)    //2
#define STATE3 (1 << 2)    //4
#define STATE4 (1 << 3)    //8
#define ZONE_NUMBER 181
#define ZONE_BOTTOM 18100
#define ZONE_TOP 18143

//Functions
#define IS_PILGRIM_ARMY(mob)(V_MOB(mob) == PILGRIM_SOLDIER\
||    V_MOB(mob) == PILGRIM_MAGE\
||  V_MOB(mob) == PILGRIM_BARD\
||  V_MOB(mob) == PILGRIM_LEADER\
)
#define IS_TURKEY_ARMY(mob)(V_MOB(mob) == TURKEY_TOT_BERSERKER\
||    V_MOB(mob) == TURKEY_TOT_WIZARD\
||    V_MOB(mob) == TURKEY_TOT_SOLDIER\
||    V_MOB(mob) == TURKEY_TOT_COMMANDER\
||    V_MOB(mob) == TURKEY_TOT_GENERAL\
)
#define IS_TURKEY_MOB(mob)(\
    V_MOB(mob) == TURKEY_TERROR\
||    V_MOB(mob) == ISSAC_PRIEST\
||    V_MOB(mob) == FAITH_PRIESTESS\
||    V_MOB(mob) == PILGRIM_LEADER\
||  V_MOB(mob) == PILGRIM_SOLDIER\
||  V_MOB(mob) == PILGRIM_MAGE\
||  V_MOB(mob) == PILGRIM_BARD\
||    V_MOB(mob) == TURKEY_TOT_BERSERKER\
||    V_MOB(mob) == TURKEY_TOT_WIZARD\
||    V_MOB(mob) == TURKEY_TOT_SOLDIER\
||    V_MOB(mob) == TURKEY_TOT_COMMANDER\
||    V_MOB(mob) == TURKEY_TOT_GENERAL\
||    V_MOB(mob) == PILGRIM_BERSERKER\
||    V_MOB(mob) == PILGRIM_VANGUARD\
||    V_MOB(mob) == PILGRIM_ARCHMAGE\
||    V_MOB(mob) == TURKEY_DEATHKNIGHT\
||    V_MOB(mob) == TURKEY_LICH\
||    V_MOB(mob) == TURKEY_SHADOW\
 ||    V_MOB(mob) == TURKEY_ZOMBIE\
)

/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */

/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */


bool isThanksgivingTimeCheck()
{
	//If its Christmas Time, dont remove the link
	
	bool thanksgivingFlag = FALSE;
	
	time_t t;
    struct tm *current_time;

    // Get the current system time
    t = time(NULL);
    current_time = localtime(&t);

    int month = current_time->tm_mon + 1;  // tm_mon is 0–11, so add 1
    int day = current_time->tm_mday;       // tm_mday is 1–31
	
	if (month == 11 && day >= 1 && day <= 30) {
		thanksgivingFlag = TRUE;
		
	}
	
	
	
	return thanksgivingFlag;
}





int turkeytakeover_issac(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    //Stores the random victim
    CHAR * summon = NULL;
    int summon_nr;

	bool isThanksgiving = isThanksgivingTimeCheck();
	//If Not Month of November, dont spam anything.
	if (isThanksgiving == FALSE) return FALSE;
	
    /*Don't waste any more CPU time if no one is in the room. */
    //if (count_mortals_room(mob, TRUE) < 1) return FALSE;
	
    //Have the vendor talk.
    if (cmd == MSG_TICK)
    {
        if (chance(50))
        {
            switch (number(0, 5))
            {
                case (0):
                    do_shout(mob, "Hurry Champions. Bring me the offerings the pilgrims have dropped.", CMD_SHOUT);
                    break;
                case (1):
                    break;
                case (2):
                    do_shout(mob, "Dont let the Plgrim's Sacrifices be in vain. Bring me their items.", CMD_SHOUT);
                    break;
                case (3):
                    break;
                case (4):
                    do_shout(mob, "Help me call upon our past warriors to fight the terror.", CMD_SHOUT);
                    break;
                case (5):
                    break;
                default:
                    break;
            }
        }

        return FALSE;

    }

    /*

        Issac Summons Pilgrim Mobs to Fight the Turkey
        Summons 1 of 3 mobs to fight the turkey given the item.

    */

    if (cmd == MSG_OBJ_GIVEN)
    {
        char buf[MIL];

        arg = one_argument(arg, buf);

        OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);

        if (!obj) return TRUE;

        bool give_back = FALSE;

        //Summon a Berserker to fight
        if (V_OBJ(obj) == BASKET)
        {
            do_say(mob, "Let me summon a berserker for you.", CMD_SAY);
            summon_nr = real_mobile(PILGRIM_BERSERKER);
            summon = read_mobile(summon_nr, REAL);
            char_to_room(summon, real_room(TURKEY_PORTAL));
            act("$n appears from a beam of light.", FALSE, summon, 0, 0, TO_ROOM);
        }

        //Summon a Vanguard to fight
        else if (V_OBJ(obj) == TURKEY)
        {
            do_say(mob, "Let me summon a vanguard for you.", CMD_SAY);
            summon_nr = real_mobile(PILGRIM_VANGUARD);
            summon = read_mobile(summon_nr, REAL);
            char_to_room(summon, real_room(TURKEY_PORTAL));
            act("$n appears from a beam of light.", FALSE, summon, 0, 0, TO_ROOM);
        }

        //Summon an Archmage to Fight
        else if (V_OBJ(obj) == CRATE)
        {
            do_say(mob, "Let me summon an Archmage for you.", CMD_SAY);
            summon_nr = real_mobile(PILGRIM_ARCHMAGE);
            summon = read_mobile(summon_nr, REAL);
            char_to_room(summon, real_room(TURKEY_PORTAL));
            act("$n appears from a beam of light .", FALSE, summon, 0, 0, TO_ROOM);
        }
        else
        {
            do_say(mob, "This cannot help you here.", CMD_SAY);

            give_back = TRUE;

        }

        if (give_back)
        {
            obj_from_char(obj);
            obj_to_char(obj, ch);
            act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);

            return TRUE;
        }

        //Remove the Token
        extract_obj(obj);

        do_say(mob, "A Champion has been summoned to help save the realm.", CMD_SAY);
        do_say(mob, "Bring me more goods so I can summon additional help.", CMD_SAY);
        return TRUE;
    }

    return FALSE;
}

int turkeytakeover_faith(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    //Stores the random victim
    CHAR * summon;
    int summon_nr;
    
	bool isThanksgiving = isThanksgivingTimeCheck();
	//If Not Month of November, dont spam anything.
	if (isThanksgiving == FALSE) return FALSE;
	
	/*Don't waste any more CPU time if no one is in the room. */
    //if (count_mortals_room(mob, TRUE) < 1) return FALSE;
	
    //Have the vendor talk.
    if (cmd == MSG_TICK)
    {
        if (chance(50))
        {
            switch (number(0, 5))
            {
                case (0):
                    do_shout(mob, "Hurry Champions. Bring me proof that you killed the vile turkeys.", CMD_SHOUT);
                    break;
                case (1):
                    break;
                case (2):
                    do_shout(mob, "The Terror must be contained", CMD_SHOUT);
                    break;
                case (3):
                    break;
                case (4):
                    do_shout(mob, "Help me call upon those that want revenge.", CMD_SHOUT);
                    break;
                case (5):
                    break;
                default:
                    break;
            }
        }

        return FALSE;

    }

    /*

        Faith Summons Turkey Mobs to Fight the Turkey
        Summons 1 of 3 mobs to fight the turkey given the item.

    */

    if (cmd == MSG_OBJ_GIVEN)
    {
        char buf[MIL];

        arg = one_argument(arg, buf);

        OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);
        //OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

        if (!obj) return TRUE;

        bool give_back = FALSE;

        //Summon a Deathknight to fight
        if (V_OBJ(obj) == CREST)
        {
            do_say(mob, "Let me summon a deathknight for you.", CMD_SAY);
            summon_nr = real_mobile(TURKEY_DEATHKNIGHT);
            summon = read_mobile(summon_nr, REAL);
            char_to_room(summon, real_room(TURKEY_PORTAL));
            act("$n appears from a beam of shadow.", FALSE, summon, 0, 0, TO_ROOM);
        }

        //Summon a Lich to fight
        else if (V_OBJ(obj) == BANNER)
        {
            do_say(mob, "Let me summon a lich for you.", CMD_SAY);
            summon_nr = real_mobile(TURKEY_LICH);
            summon = read_mobile(summon_nr, REAL);
            char_to_room(summon, real_room(TURKEY_PORTAL));
            act("$n appears from a beam of shadow.", FALSE, summon, 0, 0, TO_ROOM);
        }

        //Summon an Shadow to Fight
        else if (V_OBJ(obj) == BEAK)
        {
            do_say(mob, "Let me summon a shadow for you.", CMD_SAY);
            summon_nr = real_mobile(TURKEY_SHADOW);
            summon = read_mobile(summon_nr, REAL);
            char_to_room(summon, real_room(TURKEY_PORTAL));
            act("$n appears from a beam of shadow .", FALSE, summon, 0, 0, TO_ROOM);
        }

        //Summon a zombie to fight.
        else if (V_OBJ(obj) == FEATHERS)
        {
            do_say(mob, "Let me summon a zombie for you.", CMD_SAY);
            summon_nr = real_mobile(TURKEY_ZOMBIE);
            summon = read_mobile(summon_nr, REAL);
            char_to_room(summon, real_room(TURKEY_PORTAL));
            act("$n appears from a beam of shadow .", FALSE, summon, 0, 0, TO_ROOM);
        }
        else
        {
            do_say(mob, "This cannot help you here.", CMD_SAY);

            give_back = TRUE;

        }

        if (give_back)
        {
            act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);

            obj_from_char(obj);
            obj_to_char(obj, ch);

            return TRUE;
        }

        //Remove the Token
        extract_obj(obj);

        do_say(mob, "A Champion has been summoned to help save the realm.", CMD_SAY);
        do_say(mob, "Bring me more goods so I can summon additional help.", CMD_SAY);

        return TRUE;
    }

    return FALSE;
}

/*
    Basic Spec for all summons to attack the terror.

*/
int turkeytakeover_summons(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    CHAR *victim, *next_victim;
	
	bool isThanksgiving = isThanksgivingTimeCheck();
	//If Not Month of November, dont spam anything.
	if (isThanksgiving == FALSE) return FALSE;
	
    switch (cmd)
    {
        case MSG_MOBACT:

            //Have the summons attack the turkey
            if (!mob->specials.fighting)
            {
                for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_victim)
                {
                    next_victim = victim->next_in_room;

                    if (V_MOB(victim) == TURKEY_TERROR)
                    {
                        do_say(mob, "Defeat the terror!", CMD_SAY);
                        hit(mob, victim, TYPE_UNDEFINED);
                        return FALSE;
                    }
                }
            }

            break;
    }

    return FALSE;
}

/*
    Basic Spec for all army mobs to attack each other.

*/
int turkeytakeover_army(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    CHAR *victim, *next_victim;
    OBJ * obj2;
	
	bool isThanksgiving = isThanksgivingTimeCheck();
	//If Not Month of November, dont spam anything.
	if (isThanksgiving == FALSE) return FALSE;
	
    switch (cmd)
    {
        case MSG_MOBACT:

            //Have the summons attack the turkey
            if (!mob->specials.fighting)
            {
                for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_victim)
                {
                    next_victim = victim->next_in_room;

                    //If the pilgrim attacks.
                    if ((IS_PILGRIM_ARMY(mob) && IS_TURKEY_ARMY(victim)))
                    {
                        do_say(mob, "Die Turkey!", CMD_SAY);
                        hit(mob, victim, TYPE_UNDEFINED);
                        return FALSE;
                    }

                    //if the turkey attacks.
                    if ((IS_TURKEY_ARMY(mob) && IS_PILGRIM_ARMY(victim)))
                    {
                        do_say(mob, "Gobble Gobble Gobble", CMD_SAY);
                        hit(mob, victim, TYPE_UNDEFINED);
                        return FALSE;
                    }
                }
            }

            break;

        case MSG_DIE:
            //When a pilgrim dies - they need a chance to drop a summoning item.
            if (IS_PILGRIM_ARMY(mob))
            {
                
                if (chance(30))
                {
                    do_say(mob, "Take this as an offering. It will help you.", CMD_SAY);
                    obj2 = read_object(BASKET, VIRTUAL);
                    obj_to_room(obj2, CHAR_REAL_ROOM(mob));
                }

                if (chance(20))
                {
                    do_say(mob, "Take this as an offering. It will help you.", CMD_SAY);
                    obj2 = read_object(TURKEY, VIRTUAL);
                    obj_to_room(obj2, CHAR_REAL_ROOM(mob));
                }

                if (chance(10))
                {
                    do_say(mob, "Take this as an offering. It will help you.", CMD_SAY);
                    obj2 = read_object(CRATE, VIRTUAL);
                    obj_to_room(obj2, CHAR_REAL_ROOM(mob));

                }
            }

            break;
    }

    return FALSE;
}

void purge_turkey_takeover(CHAR *mob, bool loadRewards)
{
    int i;
    CHAR *vict, *next_v;

    OBJ *che, *rewards;
    int chest, crown, shawl, vambraces;
	
	//Purge All Remaining Turkeys

    for (i = ZONE_BOTTOM; i <= ZONE_TOP; i++)
    {
        for (vict = world[real_room(i)].people; vict; vict = next_v)
        {
            next_v = vict->next_in_room;

            //You have to ignore the king that died or else it will fail.
            //Check against the mob passed in.

            if (V_MOB(mob) == V_MOB(vict)) continue;

            //If not a turkey mob - continue
            if (!IS_TURKEY_MOB(vict)) continue;

            //Double Check NPC Flag
            if (IS_NPC(vict))
            {
                strip_char(vict);
                extract_char(vict);
            }
        }
    }

    if (loadRewards)
    {
        //Load Prizes
        /*
        Load Objects based on the repop rate.
        CROWN 18108
        SHAWL 18109
        VAMBRACES 18110
        */

        chest = real_object(CHEST);
        crown = real_object(CROWN);
        shawl = real_object(SHAWL);
        vambraces = real_object(VAMBRACES);

        che = read_object(chest, REAL);
        obj_to_room(che, real_room(TOWN_ENTRANCE));
		
		
		
        //Load Items into Chest

        if (number(1, 100) <= (obj_proto_table[crown].obj_flags.repop_percent))
        {
            rewards = read_object(crown, REAL);
            obj_to_obj(rewards, che);
        }

        if (number(1, 100) <= (obj_proto_table[shawl].obj_flags.repop_percent))
        {
            rewards = read_object(shawl, REAL);
            obj_to_obj(rewards, che);
        }

        if (number(1, 100) <= (obj_proto_table[vambraces].obj_flags.repop_percent))
        {
            rewards = read_object(vambraces, REAL);
            obj_to_obj(rewards, che);
        }
    }
}

/*
    The terror turkey will attack all the NPCs that are summoned to fight him.   

    Peck one, and hit all others.  

*/
int turkeytakeover_gobbles(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    CHAR * summon;
    int summon_nr;

    CHAR *victim, *next_vict;
	
	bool isThanksgiving = isThanksgivingTimeCheck();
	//If Not Month of November, dont spam anything.
	if (isThanksgiving == FALSE) return FALSE;

    switch (cmd)
    {
        case MSG_MOBACT:
            //Send All Players back to town so they arent fighting.
			
            for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
            {
                next_vict = vict->next_in_room;

                //Only teleport mortals.
                if (!IS_MORTAL(vict)) continue;

                act("$n is engulfed in a beam of holy light and vanishes.", FALSE, vict, 0, 0, TO_ROOM);

                char_from_room(vict);
                char_to_room(vict, real_room(TOWN_ENTRANCE));

                act("$n emerges from a beam of holy light.", TRUE, vict, 0, 0, TO_ROOM);

                do_look(vict, "", CMD_LOOK);
            }

            //If he is fighting - spec a few different attacks to murder the room.
            if (mob->specials.fighting)
            {
                switch (number(0, 6))
                {
                    case 0:
                        act("$n flaps his wings as rays of dark light hit everyone.", 0, mob, 0, 0, TO_ROOM);
                        for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_vict)
                        {
                            next_vict = victim->next_in_room;
                            if (IS_NPC(victim))
                            {
                                damage(mob, victim, 9000, TYPE_UNDEFINED, DAM_PHYSICAL);
                            }
                        }

                        break;
                    case 1:
                        break;
                    case 2:
                    case 3:
                        act("$n pecks everyone in the room.", 0, mob, 0, 0, TO_ROOM);
                        for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_vict)
                        {
                            next_vict = victim->next_in_room;
                            if (IS_NPC(victim))
                            {
                                damage(mob, victim, 6000, TYPE_UNDEFINED, DAM_PHYSICAL);
                            }
                        }

                        break;
                    case 4:
                        break;
                    case 5:
                        act("$n scarifices his own health to harm its enemies.", 0, mob, 0, 0, TO_ROOM);
                        for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_vict)
                        {
                            next_vict = victim->next_in_room;
                            if (IS_NPC(victim))
                            {
                                damage(mob, victim, 20000, TYPE_UNDEFINED, DAM_PHYSICAL);
                                GET_HIT(mob) = GET_HIT(mob) - 7500;
                            }
                        }

                        break;
                    case 6:
                        break;
                    default:
                        break;
                }
            }

            if (chance(55))
            {
                summon_nr = real_mobile(TURKEY_TOT_BERSERKER);
                if (mob_proto_table[summon_nr].number < 10)
                {
                    summon = read_mobile(summon_nr, REAL);
                    char_to_room(summon, real_room(TURKEY_GATE));
                    act("$n Appears from a Dark Stygian Pit..", FALSE, summon, 0, 0, TO_ROOM);

                }

                summon_nr = real_mobile(TURKEY_TOT_WIZARD);
                if (mob_proto_table[summon_nr].number < 15)
                {
                    summon = read_mobile(summon_nr, REAL);
                    char_to_room(summon, real_room(TURKEY_GATE_TWO));
                    act("$n Appears from a Dark Stygian Pit..", FALSE, summon, 0, 0, TO_ROOM);

                }

                summon_nr = real_mobile(TURKEY_TOT_SOLDIER);
                if (mob_proto_table[summon_nr].number < 15)
                {
                    summon = read_mobile(summon_nr, REAL);
                    char_to_room(summon, real_room(TURKEY_GATE_THREE));
                    act("$n Appears from a Dark Stygian Pit..", FALSE, summon, 0, 0, TO_ROOM);

                }
            }

            //Also Summon in Commanders and Generals for added Harm.
            if (chance(8))
            {
                summon_nr = real_mobile(TURKEY_TOT_COMMANDER);
                if (mob_proto_table[summon_nr].number < 10)
                {
                    summon = read_mobile(summon_nr, REAL);
                    char_to_room(summon, real_room(TOWN_ENTRANCE));
                    act("$n Appears from a Dark Stygian Pit..", FALSE, summon, 0, 0, TO_ROOM);

                }
            }

            if (chance(3))
            {
                summon_nr = real_mobile(TURKEY_TOT_GENERAL);
                if (mob_proto_table[summon_nr].number < 8)
                {
                    summon = read_mobile(summon_nr, REAL);
                    char_to_room(summon, real_room(TOWN_ENTRANCE));
                    act("$n Appears from a Dark Stygian Pit..", FALSE, summon, 0, 0, TO_ROOM);

                }
            }

            break;

        case MSG_DIE:
			do_shout(mob, "Curse you pilgrims.  Ill come back.......", CMD_SHOUT);
            purge_turkey_takeover(mob, TRUE);

            break;
    }

    return FALSE;
}

int turkeytakeover_pilgrimleader(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    CHAR * summon;
	CHAR *vict;
    int summon_nr;

    CHAR *victim, *next_victim;
	
	int percent_left;
	
	bool isThanksgiving = isThanksgivingTimeCheck();
	//If Not Month of November, dont spam anything.
	if (isThanksgiving == FALSE) return FALSE;
	
	
    switch (cmd)
    {
        case MSG_MOBACT:
			
			//Talk to the Entire Mud.   
            if (chance(3))
            {
                switch (number(0, 5))
                {
                    case (0):
                        do_quest(mob, "Champions - The Calamity is upon us. Please join our fight.", CMD_QUEST);
                        break;
                    case (1):
                        do_quest(mob, "Come and give thanks Champions.", CMD_QUEST);
                        break;
                    case (2):
                        break;
                    case (3):
                        break;
                    case (4):
                        do_quest(mob, "The Turkeys have invaded.  Help us drive them back.", CMD_QUEST);
                        break;
                    case (5):
                        break;
                    default:
                        break;
                }
            }
			
			//Talk to the zone
            if (chance(40))
            {
                switch (number(0, 5))
                {
                    case (0):
                        do_shout(mob, "Defeat the Turkeys and Skin them. Bring them to the priestess.", CMD_SHOUT);
                        break;
                    case (1):
                        do_shout(mob, "Our noble pilgrims bring goods to the temple. Should they fall, the priest needs their items.", CMD_SHOUT);
                        break;
                    case (2):
                        break;
                    case (3):
                        break;
                    case (4):
                        do_shout(mob, "Use the items found on the battlefield to defeat the vile Turkey Terror.", CMD_SHOUT);
                        break;
                    case (5):
                        break;
                    default:
                        break;
                }
            }
			
			
            //If he is fighting - spec a few different attacks to murder the room.
            if (mob->specials.fighting)
            {
                switch (number(0, 6))
                {
                    case 0:
                        act("$n grabs his spear and spins in a circle.", 0, mob, 0, 0, TO_ROOM);

                        for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_victim)
                        {
                            next_victim = victim->next_in_room;
                            if (IS_TURKEY_ARMY(victim))
                            {
                                damage(mob, victim, 16000, TYPE_UNDEFINED, DAM_PHYSICAL);
                                WAIT_STATE(victim, PULSE_VIOLENCE *2);
                            }
                        }

                        break;
                    case 1:
                    case 2:
                        act("$n prays to the gods for healing.", 0, mob, 0, 0, TO_ROOM);
                        GET_HIT(mob) = GET_HIT(mob) + 12000;
                        break;
                    case 3:
                        act("$n deflects the turkies attacks back at them.", 0, mob, 0, 0, TO_ROOM);
                        for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_victim)
                        {
                            next_victim = victim->next_in_room;
                            if (IS_TURKEY_ARMY(victim))
                            {
                                damage(mob, victim, 9000, TYPE_UNDEFINED, DAM_PHYSICAL);
                            }
                        }

                        break;
                    case 4:
                    case 5:
                        act("$n calls upon the gods to banish the turkies.", 0, mob, 0, 0, TO_ROOM);
                        for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = next_victim)
                        {
                            next_victim = victim->next_in_room;
                            if (IS_TURKEY_ARMY(victim))
                            {
                                damage(mob, victim, 30000, TYPE_UNDEFINED, DAM_PHYSICAL);
                                GET_HIT(mob) = GET_HIT(mob) - 1000;
                            }
                        }

                        break;
                    case 6:
                        break;
                    default:
                        break;
                }
            }

            //Check for Gobbles HP and Report the progress.
			
			
			for (vict = world[real_room(TURKEY_PORTAL)].people; vict; vict = vict->next_in_room)
			{
				if (!IS_NPC(vict) || (V_MOB(vict) != TURKEY_TERROR))
					continue;
				// If Gobbles is found, lets check his HP and report at certain percentages
				if (V_MOB(vict) == TURKEY_TERROR)
				{
					percent_left = ((GET_HIT(vict)*100)/GET_MAX_HIT(vict));
					if(chance(25)){
						if(percent_left > 90){
							do_shout(mob, "The foul bird is still going strong. Continue to summon the warriors!", CMD_SHOUT);
						}else if (percent_left > 70){
							do_shout(mob, "The vile bird is feeling the pain now. Keep up the fight", CMD_SHOUT);
						}else if (percent_left > 40){
							do_shout(mob, "Gobbles is feeling the pain now.", CMD_SHOUT);
						}else if (percent_left > 20){
							do_shout(mob, "The Turkey Terror is going to be dinner", CMD_SHOUT);							
						}else if (percent_left > 5){
							do_shout(mob, "Once he falls, visit the town for a gift!", CMD_SHOUT);
						}else if (percent_left > 1){
							do_shout(mob, "One final push, the world is almost safe!", CMD_SHOUT);
						}
					}
				}
			}
			
			/*Check for losses in the Pilgrim Army.  Spawm More

            */
            if (chance(55))
            {
                summon_nr = real_mobile(PILGRIM_SOLDIER);
                if (mob_proto_table[summon_nr].number < 11)
                {
                    summon = read_mobile(summon_nr, REAL);
                    char_to_room(summon, real_room(ROAD_SPOT_ONE));
                    act("$n runs onto the battlefield.", FALSE, summon, 0, 0, TO_ROOM);

                }

                summon_nr = real_mobile(PILGRIM_MAGE);
                if (mob_proto_table[summon_nr].number < 7)
                {
                    summon = read_mobile(summon_nr, REAL);
                    char_to_room(summon, real_room(ROAD_SPOT_TWO));
                    act("$n flys in on the breeze.", FALSE, summon, 0, 0, TO_ROOM);

                }

                summon_nr = real_mobile(PILGRIM_BARD);
                if (mob_proto_table[summon_nr].number < 5)
                {
                    summon = read_mobile(summon_nr, REAL);
                    char_to_room(summon, real_room(ROAD_SPOT_THREE));
                    act("$n Prances into the room.", FALSE, summon, 0, 0, TO_ROOM);

                }
            }

            break;
    }

    return FALSE;
}

//Assign Spec for the zone. Sets all other specs. 
//First Param - Object, Room or Mob Number.  Define it up above.
//Second Param - the name of the function that is for the mob Usually a shorthand for your zone. 
void assign_turkeytakeover(void)
{
    /*Objects */

    /*Rooms */

    /*Mobs */

    assign_mob(ISSAC_PRIEST, turkeytakeover_issac);
    assign_mob(FAITH_PRIESTESS, turkeytakeover_faith);
    assign_mob(TURKEY_TERROR, turkeytakeover_gobbles);
    assign_mob(PILGRIM_LEADER, turkeytakeover_pilgrimleader);

    //Pilgrim Army
    assign_mob(PILGRIM_SOLDIER, turkeytakeover_army);
    assign_mob(PILGRIM_MAGE, turkeytakeover_army);
    assign_mob(PILGRIM_BARD, turkeytakeover_army);

    //Turkey Army
    assign_mob(TURKEY_TOT_BERSERKER, turkeytakeover_army);
    assign_mob(TURKEY_TOT_WIZARD, turkeytakeover_army);
    assign_mob(TURKEY_TOT_SOLDIER, turkeytakeover_army);
    assign_mob(TURKEY_TOT_COMMANDER, turkeytakeover_army);
    assign_mob(TURKEY_TOT_GENERAL, turkeytakeover_army);

    //Summons
    assign_mob(PILGRIM_BERSERKER, turkeytakeover_summons);
    assign_mob(PILGRIM_VANGUARD, turkeytakeover_summons);
    assign_mob(PILGRIM_ARCHMAGE, turkeytakeover_summons);
    assign_mob(TURKEY_DEATHKNIGHT, turkeytakeover_summons);
    assign_mob(TURKEY_LICH, turkeytakeover_summons);
    assign_mob(TURKEY_SHADOW, turkeytakeover_summons);
    assign_mob(TURKEY_ZOMBIE, turkeytakeover_summons);

}
