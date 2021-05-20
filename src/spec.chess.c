/*spec.chess.c - Specs for Chess

     Written by Sane for RoninMUD
     Last Modification Date: 4/21/2021
   Modified by Fisher.

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

#define WHITE_PAWN          2301
#define WHITE_QUEEN_ROOK    2302
#define WHITE_QUEEN_KNIGHT  2303
#define WHITE_QUEEN_BISHOP  2304
#define WHITE_QUEEN         2305
#define WHITE_KING          2306
#define WHITE_KING_BISHOP   2307
#define WHITE_KING_KNIGHT   2308
#define WHITE_KING_ROOK     2309


#define BLACK_PAWN          2310
#define BLACK_QUEEN_ROOK    2311
#define BLACK_QUEEN_KNIGHT  2312
#define BLACK_QUEEN_BISHOP  2313
#define BLACK_QUEEN         2314
#define BLACK_KING          2315
#define BLACK_KING_BISHOP   2316
#define BLACK_KING_KNIGHT   2317
#define BLACK_KING_ROOK     2318

#define UBER_WHITE_QUEEN    2319
#define UBER_WHITE_BISHOP   2320
#define UBER_WHITE_KNIGHT   2321
#define UBER_WHITE_ROOK     2322

#define UBER_BLACK_QUEEN    2323
#define UBER_BLACK_BISHOP   2324
#define UBER_BLACK_KNIGHT   2325
#define UBER_BLACK_ROOK     2326

#define CHESS_WHITE         1
#define CHESS_BLACK         2

#define CHESS_ZONE          23
#define CHESS_ZONE_BOTTOM   2301
#define CHESS_ZONE_TOP      2364

//Items
#define BLACK_ROOK_ITEM     2306
#define WHITE_ROOK_ITEM     2301
#define WHITE_BISHOP_ITEM   2304
#define BLACK_BISHOP_ITEM   2309

//Recipes

#define ROOK_RECIPE         2312
#define BISHOP_RECIPE       2314
#define KNIGHT_RECIPE       2316
#define QUEEN_RECIPE        2318


//Functions

#define IS_WHITE_CHESS(mob) (V_MOB(mob) == WHITE_PAWN \
|| V_MOB(mob) == WHITE_QUEEN_ROOK \
|| V_MOB(mob) == WHITE_KING_ROOK \
|| V_MOB(mob) == WHITE_QUEEN_KNIGHT \
|| V_MOB(mob) == WHITE_KING_KNIGHT \
|| V_MOB(mob) == WHITE_QUEEN_BISHOP \
|| V_MOB(mob) == WHITE_KING_BISHOP \
|| V_MOB(mob) == WHITE_QUEEN \
|| V_MOB(mob) == WHITE_KING \
|| V_MOB(mob) == UBER_WHITE_QUEEN \
|| V_MOB(mob) == UBER_WHITE_BISHOP \
|| V_MOB(mob) == UBER_WHITE_KNIGHT \
|| V_MOB(mob) == UBER_WHITE_ROOK \
)

#define IS_BLACK_CHESS(mob) (V_MOB(mob) == BLACK_PAWN \
|| V_MOB(mob) == BLACK_QUEEN_ROOK \
|| V_MOB(mob) == BLACK_KING_ROOK \
|| V_MOB(mob) == BLACK_QUEEN_KNIGHT \
|| V_MOB(mob) == BLACK_KING_KNIGHT \
|| V_MOB(mob) == BLACK_QUEEN_BISHOP \
|| V_MOB(mob) == BLACK_KING_BISHOP \
|| V_MOB(mob) == BLACK_QUEEN \
|| V_MOB(mob) == BLACK_KING \
|| V_MOB(mob) == UBER_BLACK_QUEEN \
|| V_MOB(mob) == UBER_BLACK_BISHOP \
|| V_MOB(mob) == UBER_BLACK_KNIGHT \
|| V_MOB(mob) == UBER_BLACK_ROOK \
)

#define IS_WHITE_PAWN(mob) (V_MOB(mob) == WHITE_PAWN)
#define IS_BLACK_PAWN(mob) (V_MOB(mob) == BLACK_PAWN)
#define IS_WHITE_KING(mob) (V_MOB(mob) == WHITE_KING)
#define IS_BLACK_KING(mob) (V_MOB(mob) == BLACK_KING)

#define IS_SUPER_WHITE_ROOK(mob) (V_MOB(mob) == UBER_WHITE_ROOK)
#define IS_SUPER_BLACK_ROOK(mob) (V_MOB(mob) == UBER_BLACK_ROOK)
#define IS_SUPER_WHITE_KNIGHT(mob) (V_MOB(mob) == UBER_WHITE_KNIGHT)
#define IS_SUPER_BLACK_KNIGHT(mob) (V_MOB(mob) == UBER_BLACK_KNIGHT)

#define IS_SUPER_WHITE_BISHOP(mob) (V_MOB(mob) == UBER_WHITE_BISHOP)
#define IS_SUPER_BLACK_BISHOP(mob) (V_MOB(mob) == UBER_BLACK_BISHOP)
#define IS_SUPER_WHITE_QUEEN(mob) (V_MOB(mob) == UBER_WHITE_QUEEN)
#define IS_SUPER_BLACK_QUEEN(mob) (V_MOB(mob) == UBER_BLACK_QUEEN)



#define IS_WHITE_LAST_RANK(room) (room >= 2301 && room <= 2308)
#define IS_BLACK_LAST_RANK(room) (room >= 2357 && room <= 2364)

#define IS_CHESS_MOB(ch) (IS_WHITE_CHESS(ch) || IS_BLACK_CHESS(ch))
#define IS_CHESS_OBJECT(obj) (V_OBJ(obj) >= 2300 && V_OBJ(obj) <= 2399)

#define IS_SUPER_ROOK(ch)(IS_SUPER_WHITE_ROOK(ch) || IS_SUPER_BLACK_ROOK(ch))
#define IS_SUPER_KNIGHT(ch)(IS_SUPER_WHITE_KNIGHT(ch) || IS_SUPER_BLACK_KNIGHT(ch))
#define IS_SUPER_BISHOP(ch)(IS_SUPER_WHITE_BISHOP(ch) || IS_SUPER_BLACK_BISHOP(ch))
#define IS_SUPER_QUEEN(ch)(IS_SUPER_WHITE_QUEEN(ch) || IS_SUPER_BLACK_QUEEN(ch))





int get_uber_chess(int color)
{
    switch (number(1, 10))
    {
    case 1:
    case 2:
    case 3:
        return color == CHESS_WHITE ? UBER_WHITE_ROOK : UBER_BLACK_ROOK;
        break;
    case 4:
    case 5:
    case 6:
        return color == CHESS_WHITE ? UBER_WHITE_BISHOP : UBER_BLACK_BISHOP;
        break;
    case 7:
    case 8:
    case 9:
        return color == CHESS_WHITE ? UBER_WHITE_KNIGHT : UBER_BLACK_KNIGHT;
        break;
    case 10:
        return color == CHESS_WHITE ? UBER_WHITE_QUEEN : UBER_BLACK_QUEEN;
        break;
    }
    return 0;
}


void reset_chess_board(CHAR* mob)
{
    int i,j;
    CHAR* mob_to_spawn;
    CHAR *vict, *next_v;
    //struct obj_data *obj, *next_o;

    //Purge All Remaining Pieces
    for(i=CHESS_ZONE_BOTTOM;i<=CHESS_ZONE_TOP;i++) {
		
		

        for (vict = world[real_room(i)].people; vict; vict = next_v) {
            next_v = vict->next_in_room;

            //You have to ignore the king that died or else it will fail.
            //Check against the mob passed in.

            if(V_MOB(mob) == V_MOB(vict)) continue;

            //If not a chess mob - continue
            if (!IS_CHESS_MOB(vict)) continue;

            //Double Check NPC Flag
            if (IS_NPC(vict)){
				strip_char(vict);
                extract_char(vict);
            }
        }

      
        
    }
	
	/*
	  //Purge all the corpses and other objects

        //Can we protect against losing player items?
        
        for (obj = world[real_room(i)].contents; obj; obj = next_o) {
			//Get Real Object Number
			//If Not a Chess Item - Skip Item
			next_o = obj->next_content;
					
			if(!IS_CHESS_OBJECT(obj)) continue;	
			
			//Remove Object.            
            extract_obj(obj);
        }
	*/
	
    //Reset All Pieces

    //Load White Pieces

    //Load the Pawns
    for(i=2349;i<=2356;i++) {
        mob_to_spawn = read_mobile(WHITE_PAWN, VIRTUAL);
        char_to_room(mob_to_spawn, real_room(i));
    }

    //Load the Other Pieces
    //Start with Queens Rook.  Increment Up.
    j=2302;
    for(i=2357;i<=2364;i++) {
        mob_to_spawn = read_mobile(j, VIRTUAL);
        char_to_room(mob_to_spawn, real_room(i));
        j++;
    }

     //Load Black Pieces

     //Load the Pawns
     //Start with Queens Rook.  Increment Up.

     for(i=2309;i<=2316;i++) {
         mob_to_spawn = read_mobile(BLACK_PAWN, VIRTUAL);
         char_to_room(mob_to_spawn, real_room(i));
     }

     //Start with Queens Rook.  Increment Up.
     j=2311;
     for(i=2301;i<=2308;i++) {
         mob_to_spawn = read_mobile(j, VIRTUAL);
         char_to_room(mob_to_spawn, real_room(i));
         j++;
     }
}


/*


//Items
#define BLACK_ROOK_ITEM     2306
#define WHITE_ROOK_ITEM     2301
#define WHITE_BISHOP_ITEM   2304
#define BLACK_BISHOP_ITEM   2309

//Recipes

#define ROOK_RECIPE         2312
#define BISHOP_RECIPE       2314
#define KNIGHT_RECIPE       2316
#define QUEEN_RECIPE        2318

Load the items based on their repop rates


#define IS_SUPER_ROOK(ch)(IS_SUPER_WHITE_ROOK(ch) || IS_SUPER_BLACK_ROOK(ch))
#define IS_SUPER_KNIGHT(ch)(IS_SUPER_WHITE_KNIGHT(ch) || IS_SUPER_BLACK_KNIGHT(ch))
#define IS_SUPER_BISHOP(ch)(IS_SUPER_WHITE_BISHOP(ch) || IS_SUPER_BLACK_BISHOP(ch))
#define IS_SUPER_QUEEN(ch)(IS_SUPER_WHITE_QUEEN(ch) || IS_SUPER_BLACK_QUEEN(ch))

*/

void load_mob_items(CHAR* mob){
    //Declare Variables
    OBJ *obj = NULL;
    int object_to_load = 0;
    int recipe = 0;
    int rnum = 0;

    //Set the Item Values based on the Mob.

    //Set Normal Item

    if(IS_SUPER_WHITE_ROOK(mob)){
        object_to_load = WHITE_ROOK_ITEM;
    }
    else if (IS_SUPER_BLACK_ROOK(mob)){
        object_to_load = BLACK_ROOK_ITEM;
    }
    else if (IS_SUPER_WHITE_KNIGHT(mob)){
        object_to_load = WHITE_ROOK_ITEM;
    }
    else if (IS_SUPER_BLACK_KNIGHT(mob)){
        object_to_load = BLACK_BISHOP_ITEM;
    }
    else if (IS_SUPER_WHITE_BISHOP(mob)){
        object_to_load = WHITE_BISHOP_ITEM;
    }
    else if (IS_SUPER_BLACK_BISHOP(mob)){
        object_to_load = BLACK_BISHOP_ITEM;
    }
    else if (IS_SUPER_WHITE_QUEEN(mob)){
        object_to_load = WHITE_ROOK_ITEM;
    }
    else if (IS_SUPER_BLACK_QUEEN(mob)){
        object_to_load = WHITE_ROOK_ITEM;
    }

    //Set Recipe
    if(IS_SUPER_ROOK(mob)){
        recipe = ROOK_RECIPE;
    }
    else if (IS_SUPER_KNIGHT(mob)){
        recipe = KNIGHT_RECIPE;
    }
    else if(IS_SUPER_BISHOP(mob)){
        recipe = BISHOP_RECIPE;
    }
    else if (IS_SUPER_QUEEN(mob)){
        recipe = QUEEN_RECIPE;
    }


    //Load Normal Item into inventory if its under its repop percent.
    obj=read_object(object_to_load, VIRTUAL);
    rnum=real_object(object_to_load);
    if(number(1,100)<=(obj_proto_table[rnum].obj_flags.repop_percent))
    {
        obj_to_char(obj,mob);
    }


    //Load Recipe onto equip slot.

    obj=EQ(mob, HOLD);
    rnum=real_object(recipe);
    if(number(1,100)<=(obj_proto_table[rnum].obj_flags.repop_percent))
    {
        if (obj && ( V_OBJ(obj) != recipe )) obj_to_char( unequip_char(mob, HOLD), mob );
        else if (!obj) {
            obj=read_object(recipe, VIRTUAL);
            equip_char(mob, obj, HOLD);
        }
    }
}




int chess_mob(CHAR* mob, CHAR* ch, int cmd, char* arg)
{
    CHAR* uber_chess;
    CHAR* victim;
    CHAR* chess_king;
    char buf[MAX_STRING_LENGTH];
    int zone;


    //Set the Zone so it exists for the purge
    //zone = world[CHAR_REAL_ROOM(mob)].zone;

    switch (cmd)
    {
    case MSG_MOBACT:
        uber_chess = NULL;

        // Are we a pawn on the last rank?
        if (IS_WHITE_PAWN(mob) && IS_WHITE_LAST_RANK(world[CHAR_REAL_ROOM(mob)].number))
        {
            uber_chess = read_mobile(get_uber_chess(CHESS_WHITE), VIRTUAL);
        }
        else if (IS_BLACK_PAWN(mob) && IS_BLACK_LAST_RANK(world[CHAR_REAL_ROOM(mob)].number))
        {
            uber_chess = read_mobile(get_uber_chess(CHESS_BLACK), VIRTUAL);
        }
        if (uber_chess)
        {
            // Message to room
            sprintf(buf, "The %s transforms into %s before your eyes!", GET_SHORT(mob), GET_SHORT(uber_chess));
            send_to_room(buf, CHAR_REAL_ROOM(mob));
            // Let the zone know
            chess_king = get_ch_world(IS_WHITE_CHESS(mob) ? WHITE_KING : BLACK_KING);
            if (chess_king)
            {
                sprintf(buf, "CHECK AND MATE! A pawn has advanced across the board!");
                do_quest(chess_king, buf, CMD_QUEST);
            }

            char_to_room(uber_chess, CHAR_REAL_ROOM(mob));
            load_mob_items(uber_chess);
            extract_char(mob);
            return FALSE;
        }

        // Not on the last rank.
        // Is there an opposite color chess mob in the same room?

        //Make sure the mobs aren't fighting.
        if(!mob->specials.fighting){
            for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = victim->next_in_room)
            {
                if (!victim) return FALSE;
                if (!IS_NPC(victim)) return FALSE;
                if ((IS_WHITE_CHESS(mob) && IS_BLACK_CHESS(victim))
                    || (IS_BLACK_CHESS(mob) && IS_WHITE_CHESS(victim)))
                {
                    do_say(mob, "GOD SAVE THE KING!", CMD_SAY);
                    hit(mob, victim, TYPE_UNDEFINED);
                    return FALSE;
                }
            }
        }
        break;
    case MSG_DIE:
        if (IS_WHITE_KING(mob) || IS_BLACK_KING(mob))
        {
            // The king is dead, LONG LIVE THE KING!

            zone = CHESS_ZONE;

            if (zone >= 0)
            {
                clean_zone(zone);
                reset_chess_board(mob);
                chess_king = get_ch_world(WHITE_KING);
                // He should exist now
                if (chess_king)
                {
                    sprintf(buf, "How about a game of chess?");
                    do_quest(chess_king, buf, CMD_QUEST);
                }
            }
            else
            {
                log_f("Unable to reset chess zone on king death.\n");
            }
        }
        break;
    }

    return FALSE;
}


int bishop_holy_ring(OBJ *obj, CHAR *ch, int cmd, char *argument)
{
    if(cmd != MSG_TICK)
        return FALSE;
    if(obj->equipped_by)
    {
        GET_HIT(obj->equipped_by) = MIN( GET_MAX_HIT(obj->equipped_by),
             GET_HIT(obj->equipped_by)+10);
        GET_MANA(obj->equipped_by) = MIN( GET_MAX_MANA(obj->equipped_by),
             GET_MANA(obj->equipped_by)+10);

    }
     return FALSE;
}




void assign_chess(void)
{

    //Assing Objects
    assign_obj(WHITE_BISHOP_ITEM, bishop_holy_ring);


    //Assign Spec to All White Chess Pieces
    assign_mob(WHITE_PAWN, chess_mob);
    assign_mob(WHITE_QUEEN_ROOK, chess_mob);
    assign_mob(WHITE_QUEEN_KNIGHT, chess_mob);
    assign_mob(WHITE_QUEEN_BISHOP, chess_mob);
    assign_mob(WHITE_QUEEN, chess_mob);
    assign_mob(WHITE_KING, chess_mob);
    assign_mob(WHITE_KING_ROOK, chess_mob);
    assign_mob(WHITE_KING_KNIGHT, chess_mob);
    assign_mob(WHITE_KING_BISHOP, chess_mob);
    assign_mob(UBER_WHITE_QUEEN, chess_mob);
    assign_mob(UBER_WHITE_BISHOP, chess_mob);
    assign_mob(UBER_WHITE_KNIGHT, chess_mob);
    assign_mob(UBER_WHITE_ROOK, chess_mob);


    //Assign Spec to All Black Chess Pieces

    assign_mob(BLACK_PAWN, chess_mob);
    assign_mob(BLACK_QUEEN_ROOK, chess_mob);
    assign_mob(BLACK_QUEEN_KNIGHT, chess_mob);
    assign_mob(BLACK_QUEEN_BISHOP, chess_mob);
    assign_mob(BLACK_QUEEN, chess_mob);
    assign_mob(BLACK_KING, chess_mob);
    assign_mob(BLACK_KING_ROOK, chess_mob);
    assign_mob(BLACK_KING_KNIGHT, chess_mob);
    assign_mob(BLACK_KING_BISHOP, chess_mob);
    assign_mob(UBER_BLACK_QUEEN, chess_mob);
    assign_mob(UBER_BLACK_BISHOP, chess_mob);
    assign_mob(UBER_BLACK_KNIGHT, chess_mob);
    assign_mob(UBER_BLACK_ROOK, chess_mob);
}



