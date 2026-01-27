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
#include "aff_ench.h"

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

#define ITEM_SEPULCHER_ROOM_ONE 21621
#define ITEM_SEPULCHER_ROOM_TWO 21624
#define ITEM_SEPULCHER_ROOM_THREE 21626
#define ITEM_SEPULCHER_ROOM_FOUR 21628

#define RITUAL_SEAL_ROOM 21649
#define PHAX_NISRUTH_THRONE_ROOM 21699

/*Objects */

#define SACRIFICIAL_JAR_ONE 21601
#define SACRIFICIAL_JAR_TWO 21610
#define SEPULCHER_ASCENSION 21608
#define ITEM_SEPULCHER 21612
#define SIGIL_NERATH_PRIEST 21609

#define OATH_RELIQUARY 21620
#define GRAVEPIERCER_RELIC 21611
#define DIRGECALLER_LUTE 21619

#define ITEM_SIGNET 21614
#define ITEM_SHROUD 21615
#define ITEM_RELIQUARY 21616
#define ITEM_DAGGER 21618

/*Mobs */

#define NERATH_PRIEST 21615
#define NERATH_BONECLUTCH 21613

/*Miscellaneous strings */
// Generic States that are shifted to indicate different stages.

#define STATE1 (1 << 0) // 1
#define STATE2 (1 << 1) // 2
#define STATE3 (1 << 2) // 4
#define STATE4 (1 << 3) // 8

#define NARETH_BLESSING_NAME "Blessing of Nareth"
#define UNDEAD_CITY_ZONE_CHECK 21600

int dpc_button_room_one(int room, CHAR *ch, int cmd, char *argument)
{

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

    if (ch && cmd == CMD_MOVE)
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

int dpc_smash_jar_room_one(int room, CHAR *ch, int cmd, char *argument)
{

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
	
	if(adjusted_number > 100){
		adjusted_number = 100;
	}

    int reward;
    // 60% chance for Bronze Tradebar
    if (adjusted_number <= 70)
    {
        reward = 3013; // 70% [3013 ] a bronze tradebar -- 50k
    }
    else if (adjusted_number <= 85)
    {
        reward = 3014; // 15% [3014 ] a silver tradebar -- 100k
    }
    else if (adjusted_number <= 90)
    {
        reward = 3015; // 7% [3015 ] a gold tradebar -- 200k
    }
    else if (adjusted_number <= 96)
    {
        reward = 3016; // 2%  [3016 ] a platinum tradebar 500k
    }
    else if (adjusted_number <= 98)
    {
        reward = 3017; // 2%  [3017 ] a mithril tradebar -- 1 Mil
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
    OBJ *tmp;
    int jar_loot_object;

    if (ch && cmd == CMD_UNKNOWN)
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

                // Give a chance to open the path to the next part of the zone.
                if (chance(50))
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
    OBJ *tmp;
    int jar_loot_object;

    if (ch && cmd == CMD_UNKNOWN)
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

int get_skill_random_number(int chosen_skill)
{
    int random_skill_number = 0;

    if (chosen_skill == APPLY_DAMROLL)
    { // #define APPLY_DAMROLL          19
        random_skill_number = number(0, 2);
    }
    else if (chosen_skill == APPLY_HITROLL)
    { // #define APPLY_HITROLL          18
        random_skill_number = number(0, 2);
    }
    else if (chosen_skill == APPLY_MANA_REGEN)
    { // #define APPLY_MANA_REGEN       53
        random_skill_number = number(0, 3);
    }
    else if (chosen_skill == APPLY_HP_REGEN)
    { // #define APPLY_HP_REGEN         52
        random_skill_number = number(0, 10);
    }
    else if (chosen_skill == APPLY_HIT)
    { // #define APPLY_HIT              13
        random_skill_number = number(0, 80);
    }
    else if (chosen_skill == APPLY_MANA)
    { // #define APPLY_MANA             12
        random_skill_number = number(0, 80);
    }
    else if (chosen_skill == APPLY_MOVE)
    { // #define APPLY_MOVE             14
        random_skill_number = number(0, 80);
    }
    else if (chosen_skill == APPLY_SKILL_PARRY)
    { // #define APPLY_SKILL_PARRY      36
        random_skill_number = number(0, 8);
    }
    else if (chosen_skill == APPLY_SKILL_DODGE)
    { // #define APPLY_SKILL_DODGE      39
        random_skill_number = number(0, 8);
    }
    return random_skill_number;
}

void load_sepulchers_with_items()
{

    OBJ *tmp_item, *sepulcher;
    int sepulcher_room_number, item_number, random_skill_number, selected_skill;
    int sepulcher_rooms[4] = {ITEM_SEPULCHER_ROOM_ONE, ITEM_SEPULCHER_ROOM_TWO, ITEM_SEPULCHER_ROOM_THREE, ITEM_SEPULCHER_ROOM_FOUR};
    int sepulcher_items[4] = {ITEM_SIGNET, ITEM_SHROUD, ITEM_RELIQUARY, ITEM_DAGGER};

    int item_affects[9] = {
        APPLY_DAMROLL,
        APPLY_HP_REGEN,
        APPLY_HITROLL,
        APPLY_MANA_REGEN,
        APPLY_HIT,
        APPLY_MANA,
        APPLY_MOVE,
        APPLY_SKILL_PARRY,
        APPLY_SKILL_DODGE};

    for (int i = 0; i < 4; i++)
    {

        // Grab the room and mob based on index value.
        sepulcher_room_number = sepulcher_rooms[i];
        sepulcher = get_obj_by_vnum_in_room(ITEM_SEPULCHER, real_room(sepulcher_room_number));

        if (!sepulcher)
            break;
        // If they are empty - have a chance to load items.
        if (!OBJ_CONTAINS(sepulcher))
        {

            for (int i = 0; i < 4; i++)
            {
				//10% chance to load any item into the sepulcher
                if (chance(10))
                {
                    item_number = sepulcher_items[i];
                    tmp_item = read_object(item_number, VIRTUAL);

                    // 50% chance to set 1, 30% chance to set 2, and then 10% chance to set 3

                    if (chance(50))
                    {

                        selected_skill = item_affects[number(0, NUMELEMS(item_affects) - 1)];
                        random_skill_number = get_skill_random_number(selected_skill);

                        tmp_item->affected[0].modifier = random_skill_number;
                        tmp_item->affected[0].location = selected_skill;

                        if (chance(30))
                        {
                            selected_skill = item_affects[number(0, NUMELEMS(item_affects) - 1)];
                            random_skill_number = get_skill_random_number(selected_skill);

                            tmp_item->affected[1].modifier = random_skill_number;
                            tmp_item->affected[1].location = selected_skill;

                            if (chance(10))
                            {
                                selected_skill = item_affects[number(0, NUMELEMS(item_affects) - 1)];
                                random_skill_number = get_skill_random_number(selected_skill);

                                tmp_item->affected[2].modifier = random_skill_number;
                                tmp_item->affected[2].location = selected_skill;
                            }
                        }
                    }
                    // Once the Affects are set - Load the object.

                    obj_to_obj(tmp_item, sepulcher);
                }
            }
        }
    }
}

// Use a room to assign the function so on MSG_ZONE_RESET, they may get loaded.
int dpc_load_sepulchers_room(int room, CHAR *ch, int cmd, char *argument)
{

    if (cmd == MSG_ZONE_RESET)
    {
        load_sepulchers_with_items();
    }

    return FALSE;
}

bool check_sepulchers()
{

    bool allHaveSymbols = FALSE;
    int sepulcher_room_number;
    int sepulcher_rooms[5] = {SEPULCHER_ROOM_ONE, SEPULCHER_ROOM_TWO, SEPULCHER_ROOM_THREE, SEPULCHER_ROOM_FOUR, SEPULCHER_ROOM_FIVE};
    OBJ *sepulcher;
    int total_count = 0;

    for (int i = 0; i < 5; i++)
    {
        // Grab the room and mob based on index value.
        sepulcher_room_number = sepulcher_rooms[i];
        sepulcher = get_obj_by_vnum_in_room(SEPULCHER_ASCENSION, real_room(sepulcher_room_number));

        if (!sepulcher)
            return FALSE;

        if (OBJ_CONTAINS(sepulcher))
        {

            OBJ *temp_obj = OBJ_CONTAINS(sepulcher);

            if (V_OBJ(temp_obj) == SIGIL_NERATH_PRIEST)
            {
                total_count += i;
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

    if (total_count == 10)
    {
        allHaveSymbols = TRUE;
    }

    return allHaveSymbols;
}

void extract_sigil_sepulchers()
{

    int sepulcher_room_number;
    int sepulcher_rooms[5] = {SEPULCHER_ROOM_ONE, SEPULCHER_ROOM_TWO, SEPULCHER_ROOM_THREE, SEPULCHER_ROOM_FOUR, SEPULCHER_ROOM_FIVE};
    OBJ *sepulcher;

    for (int i = 0; i < 5; i++)
    {
        // Grab the room and mob based on index value.
        sepulcher_room_number = sepulcher_rooms[i];
        sepulcher = get_obj_by_vnum_in_room(SEPULCHER_ASCENSION, real_room(sepulcher_room_number));

        if (OBJ_CONTAINS(sepulcher))
        {

            for (OBJ *temp_obj = OBJ_CONTAINS(sepulcher), *temp_obj_next; temp_obj; temp_obj = temp_obj_next)
            {
                temp_obj_next = OBJ_NEXT_CONTENT(temp_obj);
                extract_obj(temp_obj);
            }
        }
    }
}

int sepulcher_ascension(OBJ *obj, CHAR *ch, int cmd, char *arg)
{

    if (cmd == CMD_PUT)
    {

        if (OBJ_CONTAINS(obj))
        {
            send_to_char("Only a single object can fit.\r\n", ch);
            return TRUE;
        }
    }
    return FALSE;
}

// Use a room to assign the function so on MSG_ZONE_RESET, they may get loaded.
int dpc_check_sepulcher_loads(int room, CHAR *ch, int cmd, char *argument)
{

    bool summonUndead = FALSE;
    CHAR *nerath_priest, *found_char;
    int nerath_priest_nr;

    if (cmd == MSG_TICK)
    {
        // Check all 5 Sepulchers to see if they all have the sigil.
        summonUndead = check_sepulchers();

        if (summonUndead)
        {

            found_char = get_ch_world(NERATH_PRIEST);

            if (!found_char)
            {
                nerath_priest_nr = real_mobile(NERATH_PRIEST);
                nerath_priest = read_mobile(nerath_priest_nr, REAL);
                char_to_room(nerath_priest, real_room(SEPULCHER_SEAL_ROOM));
                send_to_room("A mysterious figure appears in the middle of the room.\r\n", CHAR_REAL_ROOM(ch));

                for (int i = 21645; i <= 21653; i++)
                {

                    if (i == SEPULCHER_SEAL_ROOM)
                    {
                        send_to_room("A mysterious figure appears in front of you.\r\n", real_room(i));
                    }
                    else
                    {
                        send_to_room("A mysterious figure appears in the middle of the room.\r\n", real_room(i));
                    }
                }
            }

            extract_sigil_sepulchers();
        }
    }

    if (cmd == MSG_ZONE_RESET)
    {
        found_char = get_ch_world(NERATH_PRIEST);

        if (found_char)
        {
            act("$n decays before you", 0, found_char, 0, 0, TO_ROOM);
            extract_char(found_char);
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

    if (ench_ch && cmd == MSG_TICK)
    {
        hp_gain = number(100, 200);
        mana_gain = number(40, 80);

        GET_HIT(ench_ch) = GET_HIT(ench_ch) + hp_gain;
        GET_MANA(ench_ch) = GET_MANA(ench_ch) + mana_gain;
    }

    return FALSE;
}

int nerath_high_priest(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    CHAR *vict, *next_vict;

    char buf[MAX_INPUT_LENGTH];
    bool keyword_said = FALSE;

    if (cmd == MSG_MOBACT && mob->specials.fighting)
    {
        act("$n says 'There is no need to fight. Just resite the keyword.", 0, mob, 0, 0, TO_ROOM);
        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = CHAR_NEXT_IN_ROOM(vict))
        {
            if (vict->specials.fighting == mob)
                stop_fighting(vict->specials.fighting);
        }
        stop_fighting(mob);
        GET_HIT(mob) = GET_MAX_HIT(mob);
        return FALSE;
    }

    if (cmd == MSG_TICK && !mob->specials.fighting)
    {
        if (chance(30))
        {
            send_to_room("Bound in dust, yet voices remain\r\n", CHAR_REAL_ROOM(mob));
            send_to_room("In silent halls where dead ones reign.\r\n", CHAR_REAL_ROOM(mob));
            send_to_room("Three words lie waiting, lost in tomes,\r\n", CHAR_REAL_ROOM(mob));
            send_to_room("Keys to fate in hallowed stones.\r\n", CHAR_REAL_ROOM(mob));
            send_to_room("One grants passage, one grants might,\r\n", CHAR_REAL_ROOM(mob));
            send_to_room("One will cast you into night.\r\n", CHAR_REAL_ROOM(mob));
            send_to_room("Seek the echoes, read them true,\r\n", CHAR_REAL_ROOM(mob));
            send_to_room("The library holds what’s due to you.\r\n", CHAR_REAL_ROOM(mob));
        }
    }

    if (cmd == CMD_SAY)
    {

        arg = one_argument(arg, buf);
        if (*buf && is_abbrev(buf, "oathbound"))
        {
            act("$n says 'Your devotion is acknowledged. The path ahead is veiled in darkness, yet your steps shall not falter. Walk forward, and may the echoes of the fallen guide your way.", 0, mob, 0, 0, TO_ROOM);
            keyword_said = TRUE;

            for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
            {
                next_vict = CHAR_NEXT_IN_ROOM(vict);

                if (IS_NPC(vict))
                {
                    continue;
                }

                send_to_char("You are sent deeper into the crypt!\n\r", vict);
                char_from_room(vict);
                char_to_room(vict, real_room(CRYPT_TRANSPORT_ROOM)); /*move all chars to this room*/
                do_look(vict, "", CMD_LOOK);
            }
        }
        else if (*buf && is_abbrev(buf, "eternal"))
        {
            act("$n says 'You seek the gift of endurance, yet eternity is a burden few can bear. So be it—let the veil of undeath not claim you so easily. May your flesh resist decay, and your soul linger beyond its time.", 0, mob, 0, 0, TO_ROOM);

            for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
            {
                next_vict = CHAR_NEXT_IN_ROOM(vict);

                if (IS_NPC(vict))
                {
                    continue;
                }

                if (IS_MORTAL(vict))
                {
                    enchantment_apply(ch, FALSE, NARETH_BLESSING_NAME, 0, 35, ENCH_INTERVAL_TICK, 3, APPLY_DAMROLL, 0, 0, nareth_blessing_func);
                }
            }

            keyword_said = TRUE;
        }
        else if (*buf && is_abbrev(buf, "forsaken"))
        {
            act("$n says 'Ah… you have spoken the word of the lost. This place is not for you. The forsaken do not remain—they are cast into the void, beyond time and memory. So shall you be.", 0, mob, 0, 0, TO_ROOM);

            act("$n says 'The Path will soon be open.'", 0, mob, 0, 0, TO_ROOM);

            keyword_said = TRUE;
        }
        // Hidden keyword obtained from Nerath using Oath Reliquary //10% to get this.
        else if (*buf && is_abbrev(buf, "aurum"))
        {
            act("$n says 'Ah, you dare speak of the King's lost treasure? Beware, for the golden realm is both a blessing and a curse. Only the worthy may claim its riches.", 0, mob, 0, 0, TO_ROOM);

            act("$n says 'The Path will soon be open.'", 0, mob, 0, 0, TO_ROOM);

            keyword_said = TRUE;
        }

        // Hidden keyword obtained from Nerath using Oath Reliquary //10% to get this.
        else if (*buf && is_abbrev(buf, "elysium"))
        {
            act("$n says 'Elysium... a place of endless trials, where only those who seek experience and power will find what they desire. But beware, it is not a place for the faint of heart.", 0, mob, 0, 0, TO_ROOM);

            act("$n says 'The Path will soon be open.'", 0, mob, 0, 0, TO_ROOM);

            keyword_said = TRUE;
        }
		else{
			return FALSE;
		}

        // Once a single keyword is said, extract the NPC.
        if (keyword_said)
        {
            act("$n says 'Farewell and good luck!.", 0, mob, 0, 0, TO_ROOM);

            act("$n decays before you", 0, mob, 0, 0, TO_ROOM);

            extract_char(mob);
        }

        return TRUE;
    }

    return FALSE;
}

int gravepiercer_relic(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
    bool bReturn = FALSE;
    struct extra_descr_data *extradesc;
    int zone;
    CHAR *owner;

    switch (cmd)
    {
    case MSG_OBJ_REMOVED:
        if (ch != obj->equipped_by)
            return bReturn;
        if (!ch)
            return bReturn;
        if (obj == EQ(ch, WEAR_WIELD))
        {
            // Reset back to base stats.
            obj->short_description = str_dup("Gravepiercer, the Veil-Torn Relic");
            obj->obj_flags.value[3] = 11;
            // unequip_char(ch,WEAR_WIELD);
            obj->affected[0].modifier = 2; /* Location 0 should be set as DAMROLL */
            obj->affected[1].modifier = 2; /* Location 0 should be set as HITROLL */
            obj->affected[2].location = 0; /* Location 2 varies by class */
            obj->affected[2].modifier = 0; /* Null out value 2 */
            obj->obj_flags.value[1] = 1;   /*damnodice*/
            obj->obj_flags.value[2] = 6;   /*damsizedice*/
            // OBJ_SPEC(obj)=0;
            obj->ex_description = 0;
			obj->spec_value = 10; // Reset the spec value to ensure the specs fire again.
            send_to_char("The Relic shifts back to its original form.\n\r", ch);
        }
        break;
    case CMD_PRACTICE:
        /* with all the SKILL modifiers, this will prevent skill-looping with the relic */
        if (ch != obj->equipped_by)
            return bReturn;
        if (!ch)
            return bReturn;
        if (AWAKE(ch) && *arg)
        {
            send_to_char("The undead don't take kindly to this kinda of abuse.\n\r", ch);
            bReturn = TRUE;
        }
        break;
	case MSG_OBJ_ENTERING_GAME:
		 //If the relic is equipped, continue to transform it.  Else just break;
		 if (ch != obj->equipped_by){
			 break;
		 }
		 obj->spec_value = 10; // Reset the spec value to ensure the specs fire again.
	
    case MSG_OBJ_WORN:

        if (!ch)
            return FALSE;
        /* Don't spec if ch is not awake. */
        if (!AWAKE(ch))
            return FALSE;

        switch (GET_CLASS(ch))
        {
        case CLASS_CLERIC:
        case CLASS_MAGIC_USER:
            obj->short_description = str_dup("Gravepiercer, the Deathbinder");
            obj->obj_flags.value[3] = 7;
            CREATE(extradesc, struct extra_descr_data, 1);
            extradesc->keyword = str_dup("deathbinder");
            extradesc->description = str_dup("A staff of blackened bone, wrapped in withered cloth inscribed with forgotten incantations. It hums with necrotic energy, bending the will of lingering spirits and channeling the whispers of the Veil.\r\n");
            obj->ex_description = extradesc;

            send_to_char("The relic transforms into a staff of bone!.\n\r", ch);

            break;
        case CLASS_PALADIN:
        case CLASS_THIEF:
        case CLASS_WARRIOR:
            obj->short_description = str_dup("Gravepiercer, the Soulfang");
            obj->obj_flags.value[3] = 13;
            CREATE(extradesc, struct extra_descr_data, 1);
            extradesc->keyword = str_dup("soulfang");
            extradesc->description = str_dup("A wicked blade of spectral metal, its edge flickering between the material and ethereal planes. Each strike rends not just flesh, but the lingering essence of the undead, severing their ties to the mortal realm.\r\n");
            obj->ex_description = extradesc;

            send_to_char("The relic transforms into a metal blade!.\n\r", ch);
            break;
        case CLASS_NOMAD:
        case CLASS_ANTI_PALADIN:
        case CLASS_COMMANDO:
            obj->short_description = str_dup("Gravepiercer, the Doomcleaver");
            obj->obj_flags.value[3] = 12;
            CREATE(extradesc, struct extra_descr_data, 1);
            extradesc->keyword = str_dup("doomcleaver");
            extradesc->description = str_dup("A massive, cleaver-like axe forged from the bones of fallen titans, its surface cracked with glowing, spectral veins. With each swing, it carves through flesh and spirit alike, sending echoes of agony rippling through the Veil.\r\n");
            obj->ex_description = extradesc;
            send_to_char("The relic transforms into a massive axe!.\n\r", ch);
            break;
        case CLASS_BARD:
		case CLASS_NINJA:
            obj->short_description = str_dup("Gravepiercer, the Wraithclasp");
            obj->obj_flags.value[3] = 6;
            CREATE(extradesc, struct extra_descr_data, 1);
            extradesc->keyword = str_dup("wraithclasp");
            extradesc->description = str_dup("A gauntlet of tarnished silver and fused bone, its fingers trailing ghostly afterimages as if reaching beyond the veil. When clenched, it pulls at the souls of the dead, binding them to the wielder’s command.\r\n");
            obj->ex_description = extradesc;
            send_to_char("The relic transforms into a gauntlet of silver and bone.\n\r", ch);
            break;

        } /* end switch(GET_CLASS(ch)) */
        // OBJ_SPEC(obj)=GET_CLASS(ch);
        break;

    case MSG_TICK:
        // IF in Undead City Phas Nisruth - Damage is Upped.

        if (!(owner = obj->equipped_by))
            return FALSE;

        zone = world[CHAR_REAL_ROOM(owner)].zone;
        // char buf[MAX_STRING_LENGTH];
        // sprintf(buf, "The zone is %d!", zone);
        // send_to_room(buf, CHAR_REAL_ROOM(owner));

        if (zone == world[real_room(UNDEAD_CITY_ZONE_CHECK)].zone && obj->spec_value != 30)
        {
            send_to_char("The relic glows with power and energy!.\n\r", owner);
			obj->spec_value = 30;
            obj->obj_flags.value[1] = 8;   /*damnodice*/
            obj->obj_flags.value[2] = 8;   /*damsizedice*/
            obj->affected[0].modifier = 8; /* Location 0 should be set as DAMROLL */
            obj->affected[1].modifier = 8; /* Location 0 should be set as HITROLL */
        }
        else if(zone != world[real_room(UNDEAD_CITY_ZONE_CHECK)].zone && obj->spec_value != 60)
        {
            send_to_char("The relic gathers power from the surrounding zone.\n\r", owner);
			obj->spec_value = 60;
            obj->obj_flags.value[1] = 3;   /*damnodice*/
            obj->obj_flags.value[2] = 3;   /*damsizedice*/
            obj->affected[0].modifier = 3; /* Location 0 should be set as DAMROLL */
            obj->affected[1].modifier = 3; /* Location 0 should be set as HITROLL */
        }

        break;
    default:
        break;
    } /* end switch(cmd) */
    return bReturn;
}

int dpc_dirgecaller_lute(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
    CHAR *tmp_victim = NULL;
    CHAR *temp = NULL;
    CHAR *owner, *actor;
    char buf[MAX_INPUT_LENGTH];
    CHAR *found_char;
    OBJ *tmp_relic;
	OBJ *obj2;
	bool found_relic = FALSE;
    
	
	// Cast Rage on Everyone in the room.
	switch (cmd)
    {
		case MSG_MOBACT:
		{
			if (!obj || !obj->equipped_by)
				return FALSE;
			ch = obj->equipped_by;
			/* Don't spec if ch is not awake. */
			if (!AWAKE(ch))
				return FALSE;
			/* Don't spec if obj is not equipped by the actor. */
			if (!(owner = obj->equipped_by))
				return FALSE;
			/* Don't spec if actor is not the owner. */
			if (ch != owner)
				return FALSE;
			if (!GET_OPPONENT(owner))
				return FALSE;
			if (GET_CLASS(owner) != CLASS_BARD)
			{
				return FALSE;
			}

			if (number(0, 550) != 1) return FALSE;

			act("Your lute's spectral strings shriek with a discordant wail.", 1, ch, 0, owner, TO_CHAR);
			act("$n plays the lute and it's spectral strings shriek with a discordant wail.", 1, ch, 0, owner, TO_NOTVICT);
			for (tmp_victim = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); tmp_victim; tmp_victim = temp)
			{
				temp = CHAR_NEXT_IN_ROOM(tmp_victim);
				
				//Null Check
				if(!(temp)){ 
					continue;
				}
				
				if (ch != tmp_victim)
				{

					spell_rage(50, ch, tmp_victim, 0);
				}
			}
			spell_rage(50, ch, ch, 0);
		}
		break;

    case CMD_UNKNOWN:
    	
		
		actor = ch;
		owner = obj->equipped_by;

		if (!owner)
			return FALSE;

		if (actor != owner)
			return FALSE;

		if (!AWAKE(actor))
			return FALSE;

		if (GET_CLASS(actor) != CLASS_BARD)
			return FALSE;

		
        arg = one_argument(arg, buf);
        if (AWAKE(ch) && *buf && !strncmp(buf, "play", MAX_INPUT_LENGTH))
        {
            if (ch->specials.fighting)
            {
                send_to_char("Its impossible to concentrate on playing right now.\n\r", ch);
                return TRUE;
            }
            one_argument(arg, buf);
            if (!*buf)
                send_to_char("What do you want to play?\n\r", ch);
            else if (strncmp(buf, "lute", MAX_INPUT_LENGTH))
                send_to_char("You can't play that.\n\r", ch);
            else
            {

                // If in the throne room, place the relic on the boss
                if (V_ROOM(ch) == PHAX_NISRUTH_THRONE_ROOM)
                {

                    // See if the Boss is alive, if he is, load the GRAVEPIERCER Relic onto him.
                    for (tmp_victim = ROOM_PEOPLE(real_room(PHAX_NISRUTH_THRONE_ROOM)); tmp_victim; tmp_victim = CHAR_NEXT_IN_ROOM(tmp_victim))
                    {
                        if (!IS_NPC(tmp_victim) || (V_MOB(tmp_victim) != NERATH_BONECLUTCH))
                            continue;
                        // If Nerath is found and the lute is played in front of him, the relic is loaded.  
                        if (V_MOB(tmp_victim) == NERATH_BONECLUTCH)
                        {

                            found_char = get_ch_world(NERATH_BONECLUTCH);
							//See if he already has the relic. Only load 1
							
							for(obj2 = found_char->carrying; obj2; obj2 = obj->next_content)
							  {
								if(obj2 && V_OBJ(obj2)==GRAVEPIERCER_RELIC){
										found_relic = TRUE;
								}

							  }
							
                            tmp_relic = read_object(GRAVEPIERCER_RELIC, VIRTUAL);

                            if (tmp_relic && !found_relic)
                            {
                                obj_to_char(tmp_relic, found_char);
                                send_to_room("Nerath glows as a relic appears around his neck.\n\r", real_room(PHAX_NISRUTH_THRONE_ROOM));
                            }else{
								send_to_room("A chilling tune fills the air.....\n\r", CHAR_REAL_ROOM(ch));
							}
                        }
                    }
                }
                // IF in the seal room - Have the mob respond talking about how boneclutch wants to hear music.
                else if (V_ROOM(ch) == RITUAL_SEAL_ROOM)
                {

                    for (tmp_victim = ROOM_PEOPLE(real_room(RITUAL_SEAL_ROOM)); tmp_victim; tmp_victim = CHAR_NEXT_IN_ROOM(tmp_victim))
                    {
                        if (!IS_NPC(tmp_victim) || (V_MOB(tmp_victim) != NERATH_PRIEST))
                            continue;
                        // If Typhon is found and the seals are occupied then we want to remove the sealed version and summon the real.
                        if (V_MOB(tmp_victim) == NERATH_PRIEST)
                        {
                            found_char = get_ch_world(NERATH_PRIEST);
                            send_to_room("As the final notes hang in the air, the priest's hollow eyes glimmer with recognition.\n\r", real_room(RITUAL_SEAL_ROOM));
                            act("$n says 'The king stirs... He remembers this melody. You must go to him.'\r\n", 0, found_char, 0, 0, TO_ROOM);
                        }
                    }
                }
                else
                {
                    send_to_room("A mournful tune drifts through the air like a funeral hymn, its eerie resonance pulling at the Veil, causing distant whispers to rise and the restless dead to stir..\n\r", CHAR_REAL_ROOM(ch));
                }
            }
            return TRUE;
        }
        
    break;
	
	default:
        break;
    } /* end switch(cmd) */
    
    return FALSE;
}

int dpc_oath_reliquary(OBJ *obj, CHAR *ch, int cmd, char *arg)
{

    CHAR *tmp_victim = NULL;
    CHAR *owner;

    CHAR *found_char;

    char buf[MAX_INPUT_LENGTH];
    bool bReturn = FALSE;
	int zone;
	
	
	switch (cmd)
    {
    case MSG_OBJ_REMOVED:
         if (ch != obj->equipped_by)
            return bReturn;
        if (!ch)
            return bReturn;
        if (obj == EQ(ch, WEAR_HOLD))
        {
            // Reset back to base stats.           
            // unequip_char(ch,WEAR_WIELD);
            obj->affected[0].modifier = 1; /* Location 0 should be set as DAMROLL */
            obj->affected[1].modifier = 1; /* Location 1 should be set as HITROLL */ 
			obj->spec_value = 10; // Reset the spec value to ensure the specs fire again.			
            send_to_char("The Reliquary loses its brilliance.\n\r", ch);
        }
        break;
   case CMD_USE:
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

        if (*buf && !strncmp(buf, "reliquary", MAX_INPUT_LENGTH))
        {
            bReturn = TRUE;
            // If in the throne room, place the relic on the boss
            if (V_ROOM(ch) == PHAX_NISRUTH_THRONE_ROOM)
            {

                // See if the Boss is alive, if he is, load the GRAVEPIERCER Relic onto him.
                for (tmp_victim = ROOM_PEOPLE(real_room(PHAX_NISRUTH_THRONE_ROOM)); tmp_victim; tmp_victim = CHAR_NEXT_IN_ROOM(tmp_victim))
                {
                    if (!IS_NPC(tmp_victim) || (V_MOB(tmp_victim) != NERATH_BONECLUTCH))
                        continue;
                    // If Nerath is found, have him speak
                    if (V_MOB(tmp_victim) == NERATH_BONECLUTCH)
                    {
                        // The Boss is alive - Lets have him respond with words related to the keywords.   3 Options, 80% of the time its gibberish
                        found_char = get_ch_world(NERATH_BONECLUTCH);

                        if (found_char)
                        {
                            send_to_room("The reliquary glows in the presence of the ancient undead..\n\r", real_room(PHAX_NISRUTH_THRONE_ROOM));
                            act("$n says 'Give me the Relic. Ill trade you information'\r\n", 0, found_char, 0, 0, TO_ROOM);
                        }
                    }
                }
            }
            // IF in the seal room - Have the mob respond talking about how boneclutch wants to hear music.
            else if (V_ROOM(ch) == RITUAL_SEAL_ROOM)
            {

                for (tmp_victim = ROOM_PEOPLE(real_room(RITUAL_SEAL_ROOM)); tmp_victim; tmp_victim = CHAR_NEXT_IN_ROOM(tmp_victim))
                {
                    if (!IS_NPC(tmp_victim) || (V_MOB(tmp_victim) != NERATH_PRIEST))
                        continue;
                    // If Typhon is found and the seals are occupied then we want to remove the sealed version and summon the real.
                    if (V_MOB(tmp_victim) == NERATH_PRIEST)
                    {
                        found_char = get_ch_world(NERATH_PRIEST);
                        send_to_room("The reliquary glows in the presence of the ancient undead..\n\r", real_room(RITUAL_SEAL_ROOM));
                        act("$n says 'I never thought I would see this relic again.  Thank you!'\r\n", 0, found_char, 0, 0, TO_ROOM);
                    }
                }
            }
            else
            {
                send_to_room("The reliquary faintly glows and then goes dark.\n\r", CHAR_REAL_ROOM(ch));
            }
        }
    
   
		break;
	

    case MSG_TICK:
    // IF in Undead City Phas Nisruth - Damage is Upped.

        if (!(owner = obj->equipped_by))
            return FALSE;

        zone = world[CHAR_REAL_ROOM(owner)].zone;
        // char buf[MAX_STRING_LENGTH];
        // sprintf(buf, "The zone is %d!", zone);
        // send_to_room(buf, CHAR_REAL_ROOM(owner));

        if (zone == world[real_room(UNDEAD_CITY_ZONE_CHECK)].zone && obj->spec_value != 30)
        {
            send_to_char("The reliquary gleams brightly!.\n\r", owner);
			obj->spec_value = 30;
            obj->affected[0].modifier = 8; /* Location 0 should be set as DAMROLL */
            obj->affected[1].modifier = 2; /* Location 1 should be set as HITROLL */
        }
        else if(zone != world[real_room(UNDEAD_CITY_ZONE_CHECK)].zone && obj->spec_value != 60)
        {
            send_to_char("The reliquary slightly glows.\n\r", owner);
			obj->spec_value = 60;            
            obj->affected[0].modifier = 2; /* Location 0 should be set as DAMROLL */
            obj->affected[1].modifier = 2; /* Location 1 should be set as HITROLL */
        }
	

        break;
	case MSG_OBJ_ENTERING_GAME:
		 obj->spec_value = 10; // Reset the spec value to ensure the specs fire again.
    default:
        break;
    } /* end switch(cmd) */
    return bReturn;
}

int nerath_boneclutch(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

    int response_choice;

    if (cmd == MSG_OBJ_GIVEN)
    {
        char buf[MIL];

        arg = one_argument(arg, buf);

        OBJ *obj = get_obj_in_list_vis(mob, buf, mob->carrying);

        if (!obj)
            return TRUE;

        bool give_back = FALSE;

        // Check for the Reliquary - If not, return the item.
        if (V_OBJ(obj) != OATH_RELIQUARY)
        {
            do_say(mob, "This serves no use to me, take it and begone.", CMD_SAY);

            give_back = TRUE;
        }

        if (give_back)
        {
            act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);

            obj_from_char(obj);
            obj_to_char(obj, ch);

            return TRUE;
        }

        // Remove the reliquary
        extract_obj(obj);

        response_choice = number(1, 10);

        if (response_choice <= 8)
        {
            act("$N whispers at you, words offer the means to meaning.", FALSE, ch, 0, mob, TO_CHAR);
        }
        else if (response_choice == 9)
        { // Hidden Keyword Elysium
            act("$N whispers at you, The king’s secrets are buried deep, but you have unearthed the truth. Speak ‘Aurum’ to walk among his gilded sins.", FALSE, ch, 0, mob, TO_CHAR);
        }
        else if (response_choice == 10)
        { // Hidden Keyword Aurum
            act("$N whispers at you, For those who seek the trials of the lost, whisper ‘Elysium’ and walk the path of forsaken souls.", FALSE, ch, 0, mob, TO_CHAR);
        }
    }
    return FALSE;
}

void assign_deathplaygroundcrypt(void)
{

    /*Objects */
    assign_obj(SACRIFICIAL_JAR_ONE, sacrifical_jar_large);
    assign_obj(SEPULCHER_ASCENSION, sepulcher_ascension);
    assign_obj(SACRIFICIAL_JAR_TWO, sacrifical_jar_medium);
    assign_obj(GRAVEPIERCER_RELIC, gravepiercer_relic);
    assign_obj(DIRGECALLER_LUTE, dpc_dirgecaller_lute);
    assign_obj(OATH_RELIQUARY, dpc_oath_reliquary);

    /*Rooms*/
    assign_room(BUTTOM_ROOM_ONE_START, dpc_button_room_one);
    assign_room(SMASH_JAR_ROOM_ONE_START, dpc_smash_jar_room_one);

    assign_room(ITEM_SEPULCHER_ROOM_ONE, dpc_load_sepulchers_room);

    assign_room(RITUAL_SEAL_ROOM, dpc_check_sepulcher_loads);

    /*Mobs*/
    assign_mob(NERATH_PRIEST, nerath_high_priest);
    assign_mob(NERATH_BONECLUTCH, nerath_boneclutch);
}
