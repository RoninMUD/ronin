/*spec.caeltirshores.c - Specs for Cael Tir Shores by Fisher

     Written by Fisher for RoninMUD
     Creation Date: 2/5/2026
     Basic Specs for the mobs and rooms in the zone.
*/
/*System Includes */
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>

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

/*Objects */
#define OBJ_SALT_WARPED_TIMBER 24000
#define OBJ_SHOREBOUND_STONE 24001
#define OBJ_WOVEN_FIBER_BUNDLE 24002

/*Mobs */
#define EDRIK_HARBORMASTER 24000

/* Zones */

/*Miscellaneous strings */
// Generic States that are shifted to indicate different stages.
// Each Uber will use this differently.
#define STATE1 (1 << 0) // 1
#define STATE2 (1 << 1) // 2
#define STATE3 (1 << 2) // 4
#define STATE4 (1 << 3) // 8

#define EDRIK_DATA_FILE "util/edrik_progress.dat"
#define SAVE_INTERVAL_TICKS 30 /* save every ~30 ticks */
#define MAX_MATERIALS 8

/* -------------------------------------------------------------
 * Material Definitions
 * ------------------------------------------------------------- */

typedef struct
{
    int vnum;
    const char *name;
    const char *short_desc;
} EDRIK_MATERIAL;

enum
{
    MAT_TIMBER = 0,
    MAT_STONE,
    MAT_FIBER,
    MAT_CORAL,
    MAT_TAR,
    MAT_IRON,
    MAT_GLASS,
    MAT_SAILCLOTH
};

static EDRIK_MATERIAL edrik_materials[MAX_MATERIALS] = {
    {24000, "Salt-Warped Timber", "weather-beaten timber warped by salt and sun"},
    {24001, "Shorebound Stone", "heavy stones dredged from the shoreline"},
    {24002, "Woven Fiber Bundles", "bundles of tightly woven island fiber"},
    {24003, "Sunken Coral Bricks", "bricks carved from ancient coral"},
    {24004, "Pitch-Tar Casks", "sticky casks of boiled pitch-tar"},
    {24005, "Iron Drift Ingots", "salt-rusted ingots of reclaimed iron"},
    {24006, "Stormglass Shards", "shards of glass formed by lightning-struck sand"},
    {24007, "Sailcloth Rolls", "thick rolls of reinforced sailcloth"}};

/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */

/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */

/* -------------------------------------------------------------
 * Persistent Progress
 * ------------------------------------------------------------- */

typedef struct
{
    long count[MAX_MATERIALS];
    int highest_tier_reached;

    long total_turnins; /* total items handed in */
    time_t last_save;   /* last successful save */

    int dirty;
    int save_timer;
    int loaded;
} EDRIK_PROGRESS;

static EDRIK_PROGRESS edrik_progress;

/* -------------------------------------------------------------
 * Threshold Rules
 *
 * Tier 1:
 *   5000 each of Timber, Stone, Fiber
 *
 * Tier 2:
 *   7500 each of first 3 + 5000 Coral
 *
 * Tier 3:
 *   10000 each of first 4 + 5000 Tar
 *
 * Tier N:
 *   base_required = 5000 + (tier * 2500)
 *   new material required at 5000
 * ------------------------------------------------------------- */

static bool edrik_check_threshold(int tier)
{
    int i;
    long base_required = 5000 + (tier * 2500);
    int materials_required = 3 + tier;

    if (materials_required > MAX_MATERIALS)
        materials_required = MAX_MATERIALS;

    for (i = 0; i < materials_required; i++)
    {
        long needed = (i < 3 + tier - 1) ? base_required : 5000;
        if (edrik_progress.count[i] < needed)
            return FALSE;
    }

    return TRUE;
}

static long edrik_material_required_for_tier(int tier, int index)
{
    long base_required = 5000 + (tier * 2500);
    int materials_required = 3 + tier;

    if (index >= materials_required)
        return 0;

    /* Newest material only needs 5000 */
    if (index == materials_required - 1)
        return 5000;

    return base_required;
}



/* -------------------------------------------------------------
 * Persistence
 * ------------------------------------------------------------- */

static void edrik_load_progress(void)
{
    FILE *fp;
    char line[MAX_STRING_LENGTH];
    int idx, version;
    long count;

    memset(&edrik_progress, 0, sizeof(edrik_progress));

    fp = fopen(EDRIK_DATA_FILE, "r");
    if (!fp)
        return;

    while (fgets(line, sizeof(line), fp))
    {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        if (sscanf(line, "VERSION %d", &version) == 1)
            continue;

        if (sscanf(line, "HIGHEST_TIER %d",
                   &edrik_progress.highest_tier_reached) == 1)
            continue;

        if (sscanf(line, "TOTAL_TURNINS %ld",
                   &edrik_progress.total_turnins) == 1)
            continue;

        if (sscanf(line, "LAST_SAVE %ld",
                   &edrik_progress.last_save) == 1)
            continue;

        if (!strncmp(line, "MAT ", 4))
        {
            char *p = line + 4;
            idx = atoi(p);

            char *last_space = strrchr(line, ' ');
            if (last_space)
            {
                count = atol(last_space + 1);
                if (idx >= 0 && idx < MAX_MATERIALS)
                    edrik_progress.count[idx] = count;
            }
            continue;
        }
    }

    fclose(fp);
    edrik_progress.loaded = 1;
}


static void edrik_save_progress(void)
{
    FILE *fp;
    int i;

    if (!edrik_progress.dirty)
    {
        return;
    }

    char *g_datadir = NULL;
    g_datadir = DFLT_DIR;
    chdir(g_datadir);

    fp = fopen(EDRIK_DATA_FILE, "w");
    if (!fp)
    {
        return;
    }

    edrik_progress.last_save = time(NULL);

    fprintf(fp,
            "# ==================================================\n"
            "# Edrik Expedition Progress File\n"
            "# Zone: Cael Tir Shores\n"
            "# ==================================================\n\n"
            "VERSION 2\n\n"
            "HIGHEST_TIER %d\n"
            "TOTAL_TURNINS %ld\n"
            "LAST_SAVE %ld\n\n"
            "# Material Counts (index | name | count)\n",
            edrik_progress.highest_tier_reached,
            edrik_progress.total_turnins,
            (long)edrik_progress.last_save);

    for (i = 0; i < MAX_MATERIALS; i++)
    {
        fprintf(fp,
                "MAT %d %-22s %ld\n",
                i,
                edrik_materials[i].name,
                edrik_progress.count[i]);
    }

    fflush(fp);
    fclose(fp);

    edrik_progress.dirty = 0;
}

static long edrik_required_for(int tier, int mat_idx)
{
    long base_required = 5000 + (tier * 2500);

    /* New material for this tier */
    if (mat_idx == (3 + tier - 1))
        return 5000;

    /* Existing materials */
    if (mat_idx < (3 + tier - 1))
        return base_required;

    return 0;
}

static void edrik_force_rebuild(CHAR *mob, int tier)
{
    int i;

    if (tier < 0)
        tier = 0;
    if (tier > MAX_MATERIALS - 3)
        tier = MAX_MATERIALS - 3;

    edrik_progress.highest_tier_reached = tier;

    for (i = 0; i < MAX_MATERIALS; i++)
        edrik_progress.count[i] = 0;

    for (i = 0; i < 3 + tier; i++)
        edrik_progress.count[i] = edrik_required_for(tier, i);

    edrik_progress.dirty = 1;
    edrik_save_progress();

    do_say(mob, "The harbor plans are unfurled and rebuilt by decree.", CMD_SAY);
}


static bool edrik_is_abbrev(const char *a, const char *b)
{
    return is_abbrev((char *)a, (char *)b);
}

static bool edrik_is_number(const char *a)
{
    return is_number((char *)a);
}



static int edrik_material_lookup(const char *arg)
{
    int i;

    if (edrik_is_number(arg)){
		return atoi(arg);
	}


    for (i = 0; i < MAX_MATERIALS; i++)
        if (edrik_is_abbrev(arg, edrik_materials[i].name)){
            return i;
		}

    return -1;
}



/* -------------------------------------------------------------
 * Utility
 * ------------------------------------------------------------- */

static int edrik_material_index(OBJ *obj)
{
    int i;
    for (i = 0; i < MAX_MATERIALS; i++)
        if (V_OBJ(obj) == edrik_materials[i].vnum)
            return i;

    return -1;
}

static void edrik_announce_progress(CHAR *mob, int tier)
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,
            "The docks groan as progress is made. Tier %d of the harbor rebuild is complete!",
            tier + 1);

    do_say(mob, buf, CMD_SAY);

    /* Hook for zone updates, loading rooms, mobs, etc */
    /* trigger_edrik_zone_update(tier); */
}

/* -------------------------------------------------------------
 * Immortal Status Output
 * ------------------------------------------------------------- */

static void edrik_show_status(CHAR *ch)
{
    char buf[MAX_STRING_LENGTH];
    int i;

    sprintf(buf,
            "Edrik Rebuild Progress (Tier %d)\n"
            "Total Turn-ins: %ld\n"
            "Last Save: %s\n\n",
            edrik_progress.highest_tier_reached + 1,
            edrik_progress.total_turnins,
            edrik_progress.last_save ? ctime(&edrik_progress.last_save) : "Never\n");

    send_to_char(buf, ch);

    for (i = 0; i < MAX_MATERIALS; i++)
    {
        sprintf(buf, " %-22s : %ld\n",
                edrik_materials[i].name,
                edrik_progress.count[i]);
        send_to_char(buf, ch);
    }
	
	send_to_char("\n\r", ch);
}

static void edrik_show_mortal_status(CHAR *ch)
{
    char buf[MAX_STRING_LENGTH];
    int i;
    int tier = edrik_progress.highest_tier_reached;
    int materials_required = 3 + tier;

    if (materials_required > MAX_MATERIALS){
        materials_required = MAX_MATERIALS;
	}

	snprintf(buf, sizeof(buf),
        "[ Expedition Progress ]\n\r"
        "Current Tier : %d\n\r\n\r",
        (tier + 1)
    );

    send_to_char( buf,  ch );

    for (i = 0; i < materials_required; i++)
    {
        long required = edrik_material_required_for_tier(tier, i);
        long current  = edrik_progress.count[i];

        snprintf(buf, sizeof(buf),
            "  %-22s : %ld / %ld\n\r",
            edrik_materials[i].name,
            current,
            required
        );

        send_to_char(buf, ch);
    }

    send_to_char("\n\r", ch);

    if (edrik_check_threshold(tier))
        send_to_char("Threshold MET — Tier can advance.\n\r", ch);
    else
        send_to_char("Threshold NOT met.\n\r", ch);
}



/* -------------------------------------------------------------
 * Harbormaster Mob Special
 * ------------------------------------------------------------- */

int edrik_harbormaster(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    CHAR *vict;
	
	char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int idx;
    long amount;

    if (!edrik_progress.loaded)
        edrik_load_progress();

    switch (cmd)
    {
	case MSG_ZONE_RESET:
		edrik_progress.loaded =0;  //Force a reload of the file.
	
	break;
    case MSG_TICK:
        edrik_progress.save_timer++;

        if (edrik_progress.save_timer >= SAVE_INTERVAL_TICKS)
        {
            edrik_save_progress();
            edrik_progress.save_timer = 0;
        }
        break;

    case MSG_VIOLENCE:
        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = CHAR_NEXT_IN_ROOM(vict))
            if (vict->specials.fighting == mob)
                stop_fighting(vict->specials.fighting);

        stop_fighting(mob);
        GET_HIT(mob) = GET_MAX_HIT(mob);
        do_say(mob, "Oi! This harbor needs lumber, not blood.", CMD_SAY);
        break;

    case MSG_OBJ_GIVEN:
    {
        OBJ *obj;
        int idx;

        arg = one_argument(arg, buf);
        obj = get_obj_in_list_vis(mob, buf, mob->carrying);

        if (!obj)
            return TRUE;

        idx = edrik_material_index(obj);
        if (idx < 0)
        {
            do_say(mob, "This won't help rebuild the harbor.", CMD_SAY);
            obj_from_char(obj);
            obj_to_char(obj, ch);
            break;
        }

        edrik_progress.count[idx]++;
        edrik_progress.total_turnins++;
        edrik_progress.dirty = 1;

        act("$n nods as $p is added to the growing stockpile.", FALSE, mob, obj, NULL, TO_ROOM);

        extract_obj(obj);

        while (edrik_check_threshold(edrik_progress.highest_tier_reached))
        {
            edrik_announce_progress(mob, edrik_progress.highest_tier_reached);
            edrik_progress.highest_tier_reached++;
            edrik_progress.dirty = 1;
        }
    }
    break;
    case CMD_UNKNOWN:
        

		arg = one_argument(arg, arg1);
		arg = one_argument(arg, arg2);

        if (IS_IMMORTAL(ch) && is_abbrev(buf, "status"))
        {
            edrik_show_status(ch);
			edrik_show_mortal_status(ch);
            return TRUE;
        }
		else if (IS_MORTAL(ch) && is_abbrev(buf, "status"))
        {
			edrik_show_mortal_status(ch);
            return TRUE;
        }
        else if (IS_IMMORTAL(ch) && is_abbrev(buf, "save"))
        {
            edrik_save_progress();
            send_to_char("Edrik progress saved.\n", ch);
            return TRUE;
        }
		
		/* REBUILD <tier> */
    else if (IS_IMMORTAL(ch) && is_abbrev(arg1, "rebuild"))
    {
        if (!*arg2)
        {
            send_to_char("Usage: rebuild <tier>\n\r", ch);
            return TRUE;
        }

        edrik_force_rebuild(mob, atoi(arg2));
        send_to_char("Harbor rebuild forced.\n\r", ch);
        return TRUE;
    }

    /* ADDMAT <mat> <amount> */
    else if (IS_IMMORTAL(ch) && is_abbrev(arg1, "addmat"))
    {
        idx = edrik_material_lookup(arg2);
        if (idx < 0)
        {
            send_to_char("Invalid material.\n\r", ch);
            return TRUE;
        }

        amount = atol(arg);
        if (amount <= 0)
        {
            send_to_char("Amount must be positive.\n\r", ch);
            return TRUE;
        }

        edrik_progress.count[idx] += amount;
        edrik_progress.total_turnins += amount;
        edrik_progress.dirty = 1;

        while (edrik_check_threshold(edrik_progress.highest_tier_reached))
        {
            edrik_announce_progress(mob, edrik_progress.highest_tier_reached);
            edrik_progress.highest_tier_reached++;
        }

        send_to_char("Material added.\n\r", ch);
        return TRUE;
    }

    /* SETMAT <index> <amount> */
    else if (IS_IMMORTAL(ch) && is_abbrev(arg1, "setmat"))
    {
        idx = atoi(arg2);
        amount = atol(arg);

        if (idx < 0 || idx >= MAX_MATERIALS)
        {
            send_to_char("Invalid material index.\n\r", ch);
            return TRUE;
        }

        edrik_progress.count[idx] = amount;
        edrik_progress.dirty = 1;

        send_to_char("Material set.\n\r", ch);
        return TRUE;
    }

    /* RESET */
    if (IS_IMMORTAL(ch) && is_abbrev(arg1, "reset"))
    {
        memset(&edrik_progress, 0, sizeof(edrik_progress));
        edrik_progress.loaded = 1;
        edrik_progress.dirty = 1;
        edrik_save_progress();

        do_say(mob, "The harbor plans are struck from the ledger.", CMD_SAY);
        send_to_char("Edrik progress reset.\n\r", ch);
        return TRUE;
    }
		
		

        break;
    }

    return FALSE;
}

// Assign Spec for the zone. Sets all other specs.
// First Param - Object, Room or Mob Number.  Define it up above.
// Second Param - the name of the function that is for the mob Usually a shorthand for your zone.
void assign_caeltirshores(void)
{
    /*Objects */
    // assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);

    /*Rooms */
    // assign_room(RUNEGATE, blackmarket_runegate);

    /*Mobs */
    assign_mob(EDRIK_HARBORMASTER, edrik_harbormaster);
}