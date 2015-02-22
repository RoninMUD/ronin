/*
    
    spec.remorhaz.c -
    specs for:  Glacial Tunnels of Remorhaz
    by:                 Commandoboy

    Written by Solmyr for RoninMUD
    last modification date: 18.04.2008

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
extern struct time_info_data time_info;

#define ZONE_REMORHAZ 230
#define BEAR_SOUTH    23078
#define BEAR_CAVE     23079
#define BEAR_NORTH    23080
#define ICE_HELM      23001
#define ICE_BRACERS   23002
#define ICE_GAUNTLETS 23003
#define ICE_GIRDLE    23004
#define ICE_BRACELET  23005
#define REMORHAZ      23001
#define CRYOHYDRA     23007
#define POLAR_BEAR    23009


int get_obj_zone(OBJ *obj)
{
    while (obj->in_obj)
        obj = obj->in_obj;

    if(obj->carried_by) return GET_ZONE(obj->carried_by);
    if(obj->equipped_by) return GET_ZONE(obj->equipped_by);
    if(obj->in_room != NOWHERE) return world[obj->in_room].zone;
    return -1;
}

int ice_melt(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
    char buf[250];
    int iEq;
    int bFound;

    if(cmd!=MSG_TICK) return FALSE;
    if(get_obj_zone(obj) == ZONE_REMORHAZ) return FALSE;

    if((V_OBJ(obj) == ICE_HELM)
        || (V_OBJ(obj) == ICE_BRACERS)
        || (V_OBJ(obj) == ICE_GAUNTLETS)
        || (V_OBJ(obj) == ICE_GIRDLE)
        || (V_OBJ(obj) == ICE_BRACELET)
    )
    {
        if (obj->carried_by || obj->equipped_by)
        {
            sprintf(buf, "Your %s melts away in the heat, leaving behind only a pool of cold water.\n\r", OBJ_SHORT(obj));
            send_to_char(buf,obj->carried_by ? obj->carried_by : obj->equipped_by);
            if (obj->equipped_by)
            {
            	bFound = FALSE;
            	for (iEq = 0; iEq < MAX_WEAR; ++iEq)
            	{
            		if (obj == EQ(obj->equipped_by, iEq))
            		{
            			bFound = TRUE;
            			break;
            		}
            	}
            	if (bFound)
            	{
                    extract_obj(unequip_char(obj->equipped_by, iEq));
                    return FALSE;
                }
                else
                {
                    wizlog("Can't decay equiped ice thing.", LEVEL_DEI, 6);
                    return FALSE;
                }
            }
            extract_obj(obj_from_char(obj));
        }
        else if (obj->in_room != NOWHERE)
        {
            if(world[obj->in_room].people)
            {
                act("An icy shape melts into a pool of water.",1,world[obj->in_room].people,obj,0,TO_ROOM);             
            }
            extract_obj(obj);
        }
        else if(obj->in_obj)
        {
            extract_obj(obj);
        }
        else
        {
            wizlog("Someone found a way to not decay ice things.", LEVEL_DEI, 6);
        }
    }
    return FALSE;
}

int cryohydra(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    int hp_percent, hydra_heads, i;
    
    /* mob->specials.timer is 7 - the number of heads - 0 means 7 heads */
    if(cmd==MSG_ENTER)
    {
        if(IS_NPC(ch) || mob->specials.fighting) return FALSE;
        act("With fourteen eyes, the denizen of this lair easily notices you before you notice it.",0,mob,0,ch,TO_VICT);
        act("You attack $n as $E enters the room.",0,mob,0,ch,TO_CHAR);
        act("$N attacks $n as $E enters the room.",0,mob,0,ch,TO_NOTVICT);
        set_fighting(mob, ch);
    }
    
    if(cmd!=MSG_VIOLENCE) return FALSE;
    if(!mob->specials.fighting) return FALSE;
    
    hp_percent=(GET_HIT(mob)*100)/GET_MAX_HIT(mob);
    
    if(hp_percent >= 87) hydra_heads = 7;
    else if(hp_percent >= 72) hydra_heads = 6;
    else if(hp_percent >= 58) hydra_heads = 5;
    else if(hp_percent >= 44) hydra_heads = 4;
    else if(hp_percent >= 30) hydra_heads = 3;
    else if(hp_percent >= 16) hydra_heads = 2;
    else hydra_heads = 1;

if (hydra_heads < (7 - mob->specials.timer) && mob->specials.timer < 7)
    {
        mob->specials.timer = 7 - hydra_heads;
        act("One of $n's mean heads slumps, falling lifeless to the ground.",0,mob,0,ch,TO_ROOM);
    }
    else if (hydra_heads > (7 - mob->specials.timer) && mob->specials.timer > 0)
    {
        mob->specials.timer = 7 - hydra_heads;
        act("One of $n's mean heads lifts from the ground, returning to life.",0,mob,0,ch,TO_ROOM);
    }
    
    for (i = 0; i < hydra_heads-1; ++i)
    {
        hit(mob, ch, CMD_HIT);
    }
    return FALSE;
}

int polar_bear(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    int new_room = NOWHERE;
    CHAR *vict;
    
    if(!mob->specials.fighting) return FALSE;
    if(cmd!=MSG_MOBACT) return FALSE;
    
    if(chance(10))
    {
        vict = get_random_victim_fighting(mob);
        if (!vict) return FALSE;
        
        switch(V_ROOM(mob))
        {
            case BEAR_CAVE:
                new_room = BEAR_NORTH;
                act("After careful selection, you drag $N away to the north to feast.",1,mob,0,vict,TO_CHAR);
                act("After selecting her prey, $n drags you away to the north to feast.",1,mob,0,vict,TO_VICT);
                act("After selecting her prey, $n drags $N away to the north to feast.",1,mob,0,vict,TO_NOTVICT);
                break;
            
            case BEAR_NORTH:
                new_room = BEAR_CAVE;
                act("After careful selection, you drag $N away to the south to feast.",1,mob,0,vict,TO_CHAR);
                act("After selecting her prey, $n drags you away to the south to feast.",1,mob,0,vict,TO_VICT);
                act("After selecting her prey, $n drags $N away to the south to feast.",1,mob,0,vict,TO_NOTVICT);
                break;
        }
            
        char_from_room(mob);
        char_to_room(mob, real_room(new_room));
        char_from_room(vict);
        char_to_room(vict, real_room(new_room));
        do_look(vict, "", CMD_LOOK);
        act("$n comes in, dragging $N with $M",0,mob,0,vict,TO_ROOM);
        hit(mob, vict, CMD_HIT);
    }
    return FALSE;
}

void assign_remorhaz(void)
{
    assign_mob(CRYOHYDRA     , cryohydra);
    assign_mob(POLAR_BEAR    , polar_bear);
    assign_obj(ICE_HELM      , ice_melt);
    assign_obj(ICE_BRACERS   , ice_melt);
    assign_obj(ICE_GAUNTLETS , ice_melt);
    assign_obj(ICE_GIRDLE    , ice_melt);
    assign_obj(ICE_BRACELET  , ice_melt);
}
