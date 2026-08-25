/*
** Special Procedure Module
**
** spec.rpupgrade.c --- RP/AQP gear-upgrade broker
**
** Lets a player spend AQP (converted 15:1 from RP) at an NPC to raise a
** stat, a skill, or add a spell-effect flag on a carried item, in place
** of a god manually running 'sos' and 'qf award -<n>'.  Supports
** pooling AQP from multiple online characters that share the requester's
** connection (their own multis), each specifying exactly how much they
** are contributing.
**
** This is a system feature, not tied to any single zone's lore, so it
** lives in its own file rather than inside a zone file like
** spec.midgaard.c (which is scoped to Midgaard's guild masters, vault,
** jeweler, etc.) -- same convention as spec.quests.c / spec.rank.c.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "cmd.h"
#include "act.h"
#include "spec_assign.h"
#include "quest.h"

/* No header currently declares this (quest.c calls it the same way). */
void write_board(int vnum, char *heading, char *message);

#define RP_TO_AQP_RATIO 15
#define RP_MAX_PAYERS   8
#define RPBROKER_VNUM   3097
#define RP_NEW_OWNER_FEE_RP 10

/*
** ========================================================================
** PART 1: tiered STAT purchases (Damroll, Hitroll, Mana_regen, HP_regen,
** Armor, and any SKILL_XXX with an APPLY_SKILL_XXX behind it).
** ========================================================================
*/

typedef struct {
  const char *keyword;      /* what the player types */
  const char *label;        /* canonical display name */
  int         apply_type;   /* APPLY_ constant */
  int         block_size;   /* modifier gained per tier */
  int         direction;    /* +1 for ascending stats, -1 for inverted
                                ones like Armor, where more-negative is
                                better (this engine floors PC AC at
                                -250 -- see calc_ac() in fight.c) */
  const int  *cost_table;   /* RP cost, indexed [tier-1] */
  int         max_tier;
} rp_stat_def;

static const int damroll_cost_table[] =
  { 10, 20, 30, 45, 60, 75, 100, 125, 150, 175 };

static const int hitroll_cost_table[] =
  { 5, 10, 15, 25, 35, 45, 60, 75, 90, 105 };

/* blocks of 3: ->3, ->6, ->9 ... ->30 */
static const int mana_regen_cost_table[] =
  { 10, 20, 30, 45, 60, 75, 100, 125, 150, 175 };

/* blocks of 15: ->15, ->30, ->45 ... ->150 */
static const int hp_regen_cost_table[] =
  { 5, 10, 15, 25, 35, 45, 60, 75, 90, 105 };

/* blocks of 2: ->2, ->4, ->6 ... ->20, shared by every SKILL_XXX */
static const int skill_cost_table[] =
  { 10, 20, 30, 45, 60, 75, 100, 125, 150, 175 };

/* blocks of 5, but APPLY_ARMOR is inverted (more-negative = better),
   so this stat uses direction = -1. Players think of it as "5 points
   of Armor, 10 points, ... 50 points" but the modifier actually
   written to the item is -5, -10, ... -50. */
static const int armor_cost_table[] =
  { 5, 10, 15, 25, 35, 45, 60, 75, 90, 105 };

static const rp_stat_def rp_stats[] = {
  { "damroll",    "Damroll",        APPLY_DAMROLL,    1,  1, damroll_cost_table,    10 },
  { "hitroll",    "Hitroll",        APPLY_HITROLL,    1,  1, hitroll_cost_table,    10 },
  { "mana_regen", "Mana Regen",     APPLY_MANA_REGEN, 3,  1, mana_regen_cost_table, 10 },
  { "hp_regen",   "HP Regen",       APPLY_HP_REGEN,   15, 1, hp_regen_cost_table,   10 },
  { "armor",      "Armor",          APPLY_ARMOR,      5, -1, armor_cost_table,      10 },
  { NULL, NULL, 0, 0, 0, NULL, 0 } /* sentinel */
};

/*
** Resolves a typed stat/skill name to everything needed to price and
** apply it.  Tries the fixed stats first, then falls back to a
** generic SKILL_XXX lookup against apply_types[] -- the same array
** 'oset' itself uses -- so any skill with an APPLY_SKILL_XXX behind it
** works automatically, with no per-skill table to maintain here.
**
** Returns TRUE and fills the out-params on success.
*/
static bool resolve_stat(const char *typed_name, int *out_apply_type,
    int *out_block_size, int *out_direction, const int **out_cost_table, int *out_max_tier, const char **out_label)
{
  int i, tmp;
  char upper[MIL];

  for (i = 0; rp_stats[i].keyword; i++) {
    if (is_abbrev((char *)typed_name, (char *)rp_stats[i].keyword)) {
      *out_apply_type = rp_stats[i].apply_type;
      *out_block_size = rp_stats[i].block_size;
      *out_direction  = rp_stats[i].direction;
      *out_cost_table = rp_stats[i].cost_table;
      *out_max_tier   = rp_stats[i].max_tier;
      *out_label = rp_stats[i].label;
      return TRUE;
    }
  }

  /* Generic SKILL_XXX path. Accepts either the full "SKILL_PUMMEL"
     form or a bare "pummel" -- prepend the prefix ourselves if the
     player didn't type it, then look up against apply_types[] the
     same way either way. */
  strncpy(upper, typed_name, sizeof(upper) - 1);
  upper[sizeof(upper) - 1] = '\0';
  string_to_upper(upper);

  if (strncmp(upper, "SKILL_", 6) != 0) {
    char prefixed[MIL];
    int written = snprintf(prefixed, sizeof(prefixed), "SKILL_%s", upper);
    if (written < 0 || (size_t)written >= sizeof(prefixed))
      prefixed[sizeof(prefixed) - 1] = '\0';
    strncpy(upper, prefixed, sizeof(upper) - 1);
    upper[sizeof(upper) - 1] = '\0';
  }

  tmp = old_search_block(upper, 0, strlen(upper), apply_types, FALSE);
  if (tmp <= 0)
    return FALSE;

  *out_apply_type = tmp - 1; /* apply_types[N] holds the name for APPLY_ value N */
  *out_block_size = 2;
  *out_direction  = 1;
  *out_cost_table = skill_cost_table;
  *out_max_tier   = 10;
  *out_label      = apply_types[tmp - 1]; /* full canonical name, not the player's abbreviated input */

  return TRUE;
}

/*
** ========================================================================
** PART 2: flat-cost spell-AFFECT purchases (Sanctuary, Fly, etc.)
**
** These set a bit in obj_flags.bitvector / bitvector2 -- entirely
** separate from the affected[] stat array above, so they do NOT
** consume one of the item's MAX_OBJ_AFFECT (3) stat slots.  Resolved
** against affected_bits[] / affected_bits2[], the same arrays 'oset
** ... affected ...' already uses, so the canonical names/spellings
** always match what staff can already set by hand.
** ========================================================================
*/

typedef struct {
  const char *keyword;   /* friendly name the player types */
  const char *canonical; /* exact string as it appears in affected_bits[2] */
  int         rp_cost;
} rp_affect_def;

static const rp_affect_def rp_affects[] = {
  /* Top tier -- 250 RP */
  { "sanctuary",       "SANCTUARY",        250 },
  { "invulnerability", "INVUL",            250 },
  { "sphere",          "SPHERE",           250 },
  { "perceive",        "PERCEIVE",         250 },
  /* Mid tier -- 100 RP */
  { "protect-good",    "PROTECT-GOOD",     100 },
  { "protect-evil",    "PROTECT-EVIL",     100 },
  /* Bot tier -- 25 RP */
  { "fly",             "FLY",              25 },
  { "infravision",     "INFRAVISION",      25 },
  { "sense-life",      "SENSE-LIFE",       25 },
  { "detect-invis",    "DETECT-INVISIBLE", 25 },
  { "detect-magic",    "DETECT-MAGIC",     25 },
  { "detect-align",    "DETECT-ALIGNMENT", 25 },
  { "detect-poison",   "DETECT_POISON",    25 },
  { NULL, NULL, 0 } /* sentinel */
};

static const rp_affect_def *find_affect_def(const char *typed_name)
{
  int i;

  for (i = 0; rp_affects[i].keyword; i++) {
    if (is_abbrev((char *)typed_name, (char *)rp_affects[i].keyword))
      return &rp_affects[i];
  }

  return NULL;
}

/*
** Resolves an affect_def's canonical name to (which bitvector, bit
** value), exactly the way 'oset ... affected ...' does: try
** affected_bits[] (bitvector) first, then affected_bits2[]
** (bitvector2).
*/
static bool resolve_affect_bit(const rp_affect_def *def, int *out_which, int *out_bitv)
{
  char upper[MIL];
  int tmp;

  strncpy(upper, def->canonical, sizeof(upper) - 1);
  upper[sizeof(upper) - 1] = '\0';
  string_to_upper(upper);

  tmp = old_search_block(upper, 0, strlen(upper), affected_bits, FALSE);
  if (tmp > 0) {
    *out_which = 0;
    *out_bitv = 1 << (tmp - 1);
    return TRUE;
  }

  tmp = old_search_block(upper, 0, strlen(upper), affected_bits2, FALSE);
  if (tmp > 0) {
    *out_which = 1;
    *out_bitv = 1 << (tmp - 1);
    return TRUE;
  }

  return FALSE;
}

/*
** ========================================================================
** Shared payer parsing/validation
**
** Both purchase types (stat and affect) need the same "who's paying
** how much" logic -- resolve each name against the room, check it's
** online, check it's on the requester's own connection, check it has
** enough AQP, and require the declared amounts to sum to exactly the
** AQP cost.  Re-run fresh on both 'offer' (dry run) and 'buy' (commit)
** rather than trusting an earlier quote.
** ========================================================================
*/

struct rp_payer {
  CHAR *ch;
  int   amount;
};

static bool collect_payers(CHAR *ch, char *payer_arg, int aqp_cost,
    struct rp_payer *payers, int *out_count, char *err_buf)
{
  char name_tok[MIL], amt_tok[MIL];
  char *cursor = payer_arg;
  int   count = 0;
  int   total = 0;

  while (*cursor) {
    cursor = one_argument(cursor, name_tok);
    if (!*name_tok) break;

    cursor = one_argument(cursor, amt_tok);

    if (!*amt_tok || !is_number(amt_tok)) {
      sprintf(err_buf, "'%s' needs a number right after it.", name_tok);
      return FALSE;
    }

    if (count >= RP_MAX_PAYERS) {
      sprintf(err_buf, "Too many payers -- max %d.", RP_MAX_PAYERS);
      return FALSE;
    }

    payers[count].ch = get_char_room_vis(ch, name_tok);
    payers[count].amount = atoi(amt_tok);

    if (!payers[count].ch) {
      sprintf(err_buf, "%s isn't here.", name_tok);
      return FALSE;
    }

    if (IS_NPC(payers[count].ch)) {
      sprintf(err_buf, "%s can't pay for this.", name_tok);
      return FALSE;
    }

    if (payers[count].amount <= 0) {
      sprintf(err_buf, "Amounts must be positive.");
      return FALSE;
    }

    if (!ch->desc || !payers[count].ch->desc ||
        strcmp(ch->desc->host, payers[count].ch->desc->host)) {
      sprintf(err_buf, "%s isn't one of your characters.", GET_NAME(payers[count].ch));
      return FALSE;
    }

    if (GET_QP(payers[count].ch) < payers[count].amount) {
      sprintf(err_buf, "%s doesn't have %d AQP.", GET_NAME(payers[count].ch), payers[count].amount);
      return FALSE;
    }

    total += payers[count].amount;
    count++;
  }

  if (count == 0) {
    sprintf(err_buf, "You need to say who's paying, e.g. 'Bob 300'.");
    return FALSE;
  }

  if (total != aqp_cost) {
    sprintf(err_buf, "That adds up to %d AQP, but this costs %d AQP.", total, aqp_cost);
    return FALSE;
  }

  *out_count = count;
  return TRUE;
}

static void charge_payers(struct rp_payer *payers, int count)
{
  int i;
  for (i = 0; i < count; i++)
    GET_QP(payers[i].ch) -= payers[i].amount;
}

/*
** Persists the transaction so a crash between now and whenever these
** characters would otherwise be saved doesn't roll back the AQP
** deduction or the item changes -- there's no periodic autosave tick
** in this codebase to fall back on, so this matches the save-after-
** transaction convention used everywhere else (shops, quest procs).
*/
static void save_purchase_participants(CHAR *ch, struct rp_payer *payers, int payer_count)
{
  int i;
  bool ch_already_saved = FALSE;

  for (i = 0; i < payer_count; i++) {
    save_char(payers[i].ch, NOWHERE);
    if (payers[i].ch == ch)
      ch_already_saved = TRUE;
  }

  if (!ch_already_saved)
    save_char(ch, NOWHERE);
}

/*
** Adds ch as an owner of item without disturbing any existing owners
** (up to MAX_OBJ_OWNER_ID, currently 8) -- a no-op if ch is already
** one. This used to unconditionally wipe every slot and set only
** ch, which was harmless when only the original owner ever bought
** upgrades, but would silently erase a co-owner the moment someone
** else was added -- see check_owner_gate() below, which is what
** actually gates and charges for adding a second owner.
*/
static void add_owner(OBJ *item, CHAR *ch)
{
  int i;
  int id = GET_ID(ch);

  for (i = 0; i < MAX_OBJ_OWNER_ID; i++)
    if (item->ownerid[i] == id)
      return; /* already an owner */

  for (i = 0; i < MAX_OBJ_OWNER_ID; i++) {
    if (item->ownerid[i] == 0) {
      item->ownerid[i] = id;
      return;
    }
  }
  /* all 8 slots full -- extremely unlikely, but do nothing rather than
     silently overwrite an existing owner */
}

/*
** Finds a currently-online player character by numeric ID. Returns
** NULL if that id isn't connected right now. There's no persisted
** host history to check an offline owner against, so verifying
** ownership continuity requires them to actually be online.
*/
static CHAR *find_online_char_by_id(int id)
{
  CHAR *tmp;

  if (id <= 0) return NULL;

  for (tmp = character_list; tmp; tmp = tmp->next) {
    if (!IS_NPC(tmp) && tmp->desc && GET_ID(tmp) == id)
      return tmp;
  }

  return NULL;
}

/*
** If item already has a different owner, adding ch as a new owner
** costs a flat 10 RP surcharge and requires ch to be verified as one
** of the existing owner's own characters (same connection) -- same
** check collect_payers() uses for payers. The existing owner must be
** online right now to be checked against.
**
** Returns TRUE and fills *extra_aqp_cost (0 if no gate applies) on
** success. Returns FALSE and fills err_buf on failure.
*/
static bool check_owner_gate(CHAR *ch, OBJ *item, int *extra_aqp_cost, char *err_buf)
{
  int i;
  bool has_any_owners = FALSE;
  bool all_owners_same_multi = TRUE;

  *extra_aqp_cost = 0;

  for (i = 0; i < MAX_OBJ_OWNER_ID; i++) {
    if (item->ownerid[i] <= 0) {
      continue;
    }

    /* requester already owner, exit */
    if (item->ownerid[i] == GET_ID(ch)) {
      *extra_aqp_cost = 0; // ensure no fee
      return TRUE;
    }

    has_any_owners = TRUE;

    /* check if this specific co-owner */
    CHAR *existing_owner = find_online_char_by_id(item->ownerid[i]);
    
    if (!existing_owner || !ch->desc || !existing_owner->desc || 
        strcmp(ch->desc->host, existing_owner->desc->host) != 0) {
      all_owners_same_multi = FALSE;
      /* NOTE: don't break here, still need to check if later owner slot is requester */
    }
  }

  if(!has_any_owners) {
    return TRUE; // unowned item, no fee
  }

  if(all_owners_same_multi) {
    *extra_aqp_cost = RP_NEW_OWNER_FEE_RP * RP_TO_AQP_RATIO;
    return TRUE; /* gate passed, fee applied. */
  }

  /* owners exist, but none are online on your host to approve you */
  sprintf(err_buf, "$n tells you 'Not all of the current owners are online on your connection to validate this purchase.'");
  return FALSE;
}

static void apply_item_tags(CHAR *ch, OBJ *item)
{
  SET_BIT(item->obj_flags.wear_flags, ITEM_QUESTWEAR);
  if (!IS_SET(item->obj_flags.extra_flags, ITEM_LIMITED))
    SET_BIT(item->obj_flags.extra_flags, ITEM_LIMITED);

  add_owner(item, ch);
}

static void log_purchase(CHAR *ch, OBJ *item, const char *what, const char *from, const char *to,
    struct rp_payer *payers, int payer_count, int aqp_cost, bool new_owner)
{
  char logbuf[MSL], paybuf[MSL] = "", tmp[MIL];
  int i, len;

  for (i = 0; i < payer_count; i++) {
    snprintf(tmp, sizeof(tmp), "%s:%d ", GET_NAME(payers[i].ch), payers[i].amount);
    strncat(paybuf, tmp, sizeof(paybuf) - strlen(paybuf) - 1);
  }

  /* GET_NAME/short_description are always short in practice (well under
     100 chars); this is just silencing GCC's worst-case truncation
     analysis, not fixing a real bug -- snprintf already can't overflow. */
  len = snprintf(logbuf, sizeof(logbuf), "RPUPGRADE: %s bought %s %s->%s on %s %s [%s] (%d AQP total)",
           GET_NAME(ch), what, from, to, OBJ_SHORT(item), new_owner ? "and added owner" : "", paybuf, aqp_cost);
  if (len < 0 || (size_t)len >= sizeof(logbuf))
    logbuf[sizeof(logbuf) - 1] = '\0';

  wizlog(logbuf, GET_LEVEL(ch) + 1, 4);
  log_s(logbuf);
  write_board(3097, "RP Upgrade", logbuf);
}

/*
** ========================================================================
** PART 3: named-item purchases (buy a whole pre-built item outright)
**
** Separate from the stat/affect paths -- there's no existing item being
** modified here, just a catalog entry loaded fresh from its prototype.
** Uses a curated keyword rather than the item's own in-game keyword
** list (obj->name), since that list is written for get/wear/wield
** sentences, not for a short, unambiguous shop word -- and it decouples
** what players type from whatever the item happens to be named.
** ========================================================================
*/

typedef struct {
  const char *keyword;
  int         vnum;
  int         rp_cost;
} rp_item_def;

/* TODO: fill in with the actual curated catalog. */
static const rp_item_def rp_items[] = {
  /* { "cloak",  30047, 300 }, */
  { NULL, 0, 0 } /* sentinel */
};

static const rp_item_def *find_item_def(const char *typed_name)
{
  int i;

  for (i = 0; rp_items[i].keyword; i++) {
    if (is_abbrev((char *)typed_name, (char *)rp_items[i].keyword))
      return &rp_items[i];
  }

  return NULL;
}

static bool do_named_item_purchase(CHAR *ch, const char *item_keyword, char *payer_arg,
    bool commit, char *result_buf)
{
  const rp_item_def *def;
  int aqp_cost, rnum;
  struct rp_payer payers[RP_MAX_PAYERS];
  int payer_count = 0;
  char err_buf[MSL];
  OBJ *item;

  def = find_item_def(item_keyword);
  if (!def) {
    sprintf(result_buf, "I don't sell that.");
    return FALSE;
  }

  rnum = real_object(def->vnum);
  if (rnum < 0) {
    sprintf(result_buf, "That item isn't available right now -- tell a god.");
    return FALSE;
  }

  aqp_cost = def->rp_cost * RP_TO_AQP_RATIO;

  if (!commit) {
    sprintf(result_buf, "That's %d RP (%d AQP) for %s. Say buy with the same payers to confirm.",
            def->rp_cost, aqp_cost, def->keyword);
    return TRUE;
  }

  if (!collect_payers(ch, payer_arg, aqp_cost, payers, &payer_count, err_buf)) {
    strcpy(result_buf, err_buf);
    return FALSE;
  }

  item = read_object(def->vnum, VIRTUAL);
  if (!item) {
    sprintf(result_buf, "Something went wrong loading that -- tell a god, nothing was charged.");
    return FALSE;
  }

  charge_payers(payers, payer_count);
  apply_item_tags(ch, item);
  obj_to_char(item, ch);
  log_purchase(ch, item, def->keyword, "n/a", "purchased", payers, payer_count, aqp_cost, FALSE);
  save_purchase_participants(ch, payers, payer_count);

  sprintf(result_buf, "Here you go.");
  return TRUE;
}

static void list_item_catalog(CHAR *broker, CHAR *ch)
{
  int i;
  char buf[MSL];

  if (!rp_items[0].keyword) {
    act("$n tells you 'Nothing in stock right now.'", FALSE, broker, 0, ch, TO_VICT);
    return;
  }

  send_to_char("Available items:\n\r", ch);
  for (i = 0; rp_items[i].keyword; i++) {
    sprintf(buf, "  %-20s %5d RP\n\r", rp_items[i].keyword, rp_items[i].rp_cost);
    send_to_char(buf, ch);
  }
}

/*
** ========================================================================
** Stat purchase (offer/buy path A)
** ========================================================================
*/

static bool do_stat_purchase(CHAR *ch, OBJ *item, const char *stat_name, char *payer_arg,
    bool commit, char *result_buf)
{
  const char *display_name = stat_name;
  int apply_type, block_size, direction, max_tier;
  const int *cost_table;
  int i, existing_slot = -1, free_slot = -1, current_modifier = 0;
  int current_tier, target_tier, target_modifier, rp_cost, aqp_cost, tier;
  struct rp_payer payers[RP_MAX_PAYERS];
  int payer_count = 0;
  char err_buf[MSL];
  char peek_tok[MIL];
  char *after_peek;
  int requested_value;

  if (IS_SET(OBJ_EXTRA_FLAGS(item), ITEM_DISPELLED)) {
    sprintf(result_buf, "$p has been dispelled and cannot be upgraded.");
    return FALSE;
  }

  if (!resolve_stat(stat_name, &apply_type, &block_size, &direction, &cost_table, &max_tier, &display_name)) {
    sprintf(result_buf, "I don't upgrade that.");
    return FALSE;
  }

  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (item->affected[i].location == apply_type) {
      existing_slot = i;
      current_modifier = item->affected[i].modifier;
    }
    else if (item->affected[i].location == APPLY_NONE && free_slot == -1) {
      free_slot = i;
    }
  }

  if (existing_slot == -1 && free_slot == -1) {
    sprintf(result_buf, "$p doesn't have room for another stat -- all %d affect slots are used.", MAX_OBJ_AFFECT);
    return FALSE;
  }

  /* Normalize tier math so values below zero are still priced on the same
     curve as their mirrored positive side.  A Damroll of -5 should cost the
     same to move to -4 as a Damroll of 4 does to move to 5, because both are
     the 5th priced step away from zero. */
  if (current_modifier < 0 && direction > 0)
    current_tier = (abs(current_modifier) + block_size - 1) / block_size;
  else
    current_tier = (current_modifier * direction) / block_size;

  /* Optional absolute target: "buy upgrade sword damroll 3 Bob 300" means
     "get this item's damroll to 3 total", not "raise it by 3 steps". If
     the next token isn't a positive number, there's no target given and
     this falls back to the single-tier behavior (next tier up), Negative
     values are treated as being below base, so the default next step moves
     toward zero instead of being rejected. */
  after_peek = one_argument(payer_arg, peek_tok);
  if (*peek_tok && is_number(peek_tok)) {
    requested_value = atoi(peek_tok);
    target_modifier = requested_value;
    target_tier = (abs(target_modifier) + block_size - 1) / block_size; /* round up to the tier that reaches it */
    payer_arg = after_peek;
  }
  else if (current_modifier < 0 && direction > 0) {
    target_modifier = current_modifier + block_size;
    target_tier = (abs(target_modifier) + block_size - 1) / block_size;
  }
  else {
    target_tier = current_tier + 1;
    target_modifier = direction * target_tier * block_size;
  }

  /* A crossing purchase (negative -> positive in one shot) hits the
     abs()-based tier math below in a way that silently mis-prices it.
     Simplest fix: refuse it outright and have the player buy up to 0
     first, then buy again from 0 upward. */
  if (current_modifier < 0 && direction > 0 && target_modifier > 0) {
    sprintf(result_buf, "$p's %s is negative (currently %d) -- buy it up to 0 first, then make a separate purchase to go above 0.",
            display_name, current_modifier);
    return FALSE;
  }

  if ((current_modifier < 0 && direction > 0 && target_modifier <= current_modifier) ||
      (!(current_modifier < 0 && direction > 0) && target_tier <= current_tier)) {
    sprintf(result_buf, "$p is already at or past that.");
    return FALSE;
  }

  if ((abs(target_modifier) + block_size - 1) / block_size > max_tier) {
    sprintf(result_buf, "That's past the maximum for that stat -- the highest tier reaches %d.", max_tier * block_size);
    return FALSE;
  }

  if (current_modifier >= 0 || direction <= 0)
    target_modifier = direction * target_tier * block_size;

  /* Sum every tier crossed, not just the last one, so a multi-tier
     purchase (e.g. 0 -> 3 damroll in one go) charges the full cost of
     every tier in between, not just the price of the final tier.  For a
     negative value on a positive-direction stat, the upgrade is toward zero,
     so the cost curve is mirrored across zero instead of being treated as an
     impossible negative tier. */
  aqp_cost = 0;
  if (current_modifier < 0 && direction > 0) {
    for (tier = current_tier; tier > target_tier; tier--)
      aqp_cost += cost_table[tier - 1] * RP_TO_AQP_RATIO;
  }
  else {
    for (tier = current_tier + 1; tier <= target_tier; tier++)
      aqp_cost += cost_table[tier - 1] * RP_TO_AQP_RATIO;
  }
  rp_cost = aqp_cost / RP_TO_AQP_RATIO;

  bool new_owner = FALSE;
  {
    int owner_fee = 0;

    if (!check_owner_gate(ch, item, &owner_fee, result_buf))
      return FALSE;

    aqp_cost += owner_fee;
    rp_cost += owner_fee / RP_TO_AQP_RATIO;
    if(owner_fee)
      new_owner = TRUE;
  }

  if (!commit) {
    if (new_owner) {
      sprintf(result_buf, "That's %d RP (%d AQP), taking %s from %d to %d %s and adding %s as an owner. Say buy with the same payers to confirm.",
            rp_cost, aqp_cost, OBJ_SHORT(item), current_modifier, target_modifier, display_name, GET_NAME(ch));
    }
    else {
      sprintf(result_buf, "That's %d RP (%d AQP), taking %s from %d to %d %s. Say buy with the same payers to confirm.",
            rp_cost, aqp_cost, OBJ_SHORT(item), current_modifier, target_modifier, display_name);
    }
    return TRUE;
  }

  /* collect_payers() below already requires the declared amounts to sum
     to exactly aqp_cost, and returns FALSE (no charge, no mutation) if
     they don't -- so a short payment already hard-blocks the whole
     transaction rather than partially applying it, for single-tier and
     multi-tier purchases alike. */
  if (!collect_payers(ch, payer_arg, aqp_cost, payers, &payer_count, err_buf)) {
    strcpy(result_buf, err_buf);
    return FALSE;
  }

  charge_payers(payers, payer_count);

  if (existing_slot != -1) {
    item->affected[existing_slot].modifier = target_modifier;
  }
  else {
    item->affected[free_slot].location = apply_type;
    item->affected[free_slot].modifier = target_modifier;
  }

  apply_item_tags(ch, item);

  {
    char from[16], to[16];
    sprintf(from, "%d", current_modifier);
    sprintf(to, "%d", target_modifier);
    log_purchase(ch, item, display_name, from, to, payers, payer_count, aqp_cost, new_owner);
  }

  save_purchase_participants(ch, payers, payer_count);

  sprintf(result_buf, "Done. $p is now %d %s%s.", target_modifier, display_name, new_owner ? " and you are an owner" : "");
  return TRUE;
}

/*
** ========================================================================
** Affect-flag purchase (offer/buy path B)
** ========================================================================
*/

static bool do_affect_purchase(CHAR *ch, OBJ *item, const char *affect_name, char *payer_arg,
    bool commit, char *result_buf)
{
  const rp_affect_def *def;
  int which, bitv;
  int aqp_cost;
  struct rp_payer payers[RP_MAX_PAYERS];
  int payer_count = 0;
  char err_buf[MSL];

  def = find_affect_def(affect_name);
  if (!def) {
    sprintf(result_buf, "I don't offer that effect.");
    return FALSE;
  }

  if (!resolve_affect_bit(def, &which, &bitv)) {
    /* Shouldn't happen unless affected_bits[]/affected_bits2[] and
       rp_affects[] have drifted out of sync -- surface it loudly
       rather than silently failing. */
    sprintf(result_buf, "That effect isn't wired up correctly -- tell a god.");
    return FALSE;
  }

  if ((which == 0 && IS_SET(item->obj_flags.bitvector, bitv)) ||
      (which == 1 && IS_SET(item->obj_flags.bitvector2, bitv))) {
    sprintf(result_buf, "$p already has that effect.");
    return FALSE;
  }

  aqp_cost = def->rp_cost * RP_TO_AQP_RATIO;

  bool new_owner = FALSE;
  {
    int owner_fee = 0;

    if (!check_owner_gate(ch, item, &owner_fee, result_buf))
      return FALSE;

    aqp_cost += owner_fee;
    if(owner_fee)
      new_owner = TRUE;
  }

  if (!commit) {
    if (new_owner) {
      sprintf(result_buf, "That's %d RP (%d AQP) to add %s and add %s as an owner. Say buy with the same payers to confirm.",
            aqp_cost / RP_TO_AQP_RATIO, aqp_cost, def->canonical, GET_NAME(ch));
    }
    else {
      sprintf(result_buf, "That's %d RP (%d AQP) to add %s. Say buy with the same payers to confirm.",
            aqp_cost / RP_TO_AQP_RATIO, aqp_cost, def->canonical);
    }
    return TRUE;
  }  

  if (!collect_payers(ch, payer_arg, aqp_cost, payers, &payer_count, err_buf)) {
    strcpy(result_buf, err_buf);
    return FALSE;
  }

  charge_payers(payers, payer_count);

  if (which == 0)
    SET_BIT(item->obj_flags.bitvector, bitv);
  else
    SET_BIT(item->obj_flags.bitvector2, bitv);

  apply_item_tags(ch, item);
  log_purchase(ch, item, def->canonical, "off", "on", payers, payer_count, aqp_cost, new_owner);
  save_purchase_participants(ch, payers, payer_count);

  sprintf(result_buf, "Done. $p now has %s%s.", def->canonical, new_owner ? " and you are an owner" : "");
  return TRUE;
}

/*
** ========================================================================
** Spec proc entry points
** ========================================================================
*/

static int rp_broker_upgrade(CHAR *broker, CHAR *ch, char *arg, bool commit)
{
  char item_name[MIL], what_name[MIL];
  char result_buf[MSL];
  OBJ *item;

  arg = one_argument(arg, item_name);
  arg = one_argument(arg, what_name);

  if (!*item_name || !*what_name) {
    act("$n tells you 'Try: offer upgrade <item> <stat/effect> [target] <payer> <amount> ...'", FALSE, broker, 0, ch, TO_VICT);
    return TRUE;
  }

  item = get_obj_in_list_vis(ch, item_name, ch->carrying);
  if (!item) {
    act("$n tells you 'You don't seem to have that.'", FALSE, broker, 0, ch, TO_VICT);
    return TRUE;
  }

  /* Decide which purchase path applies by resolving the name directly,
     rather than string-matching an error message. */
  {
    int tmp_apply, tmp_block, tmp_direction, tmp_max;
    const int *tmp_table;
    const char *display_name = NULL;

    if (resolve_stat(what_name, &tmp_apply, &tmp_block, &tmp_direction, &tmp_table, &tmp_max, &display_name)) {
      do_stat_purchase(ch, item, what_name, arg, commit, result_buf);
    }
    else if (find_affect_def(what_name)) {
      do_affect_purchase(ch, item, what_name, arg, commit, result_buf);
    }
    else {
      strcpy(result_buf, "I don't offer that stat or effect.");
    }
  }

  act(result_buf, FALSE, broker, item, ch, TO_VICT);
  return TRUE;
}

static int rp_broker_item(CHAR *broker, CHAR *ch, char *arg, bool commit)
{
  char item_keyword[MIL];
  char result_buf[MSL];

  arg = one_argument(arg, item_keyword);

  if (!*item_keyword) {
    act("$n tells you 'Try: offer item <keyword> <payer> <amount> ...'", FALSE, broker, 0, ch, TO_VICT);
    return TRUE;
  }

  do_named_item_purchase(ch, item_keyword, arg, commit, result_buf);
  act(result_buf, FALSE, broker, 0, ch, TO_VICT);
  return TRUE;
}

static int rp_broker_dispatch(CHAR *broker, CHAR *ch, char *arg, bool commit)
{
  char mode[MIL];

  arg = one_argument(arg, mode);

  if (is_abbrev(mode, "upgrade"))
    return rp_broker_upgrade(broker, ch, arg, commit);

  if (is_abbrev(mode, "item"))
    return rp_broker_item(broker, ch, arg, commit);

  act("$n tells you 'Try: offer upgrade ... or offer item ..., or list to see what I stock.'", FALSE, broker, 0, ch, TO_VICT);
  return TRUE;
}

int rp_broker(CHAR *broker, CHAR *ch, int cmd, char *arg)
{
  if (!ch || IS_NPC(ch)) return FALSE;

  switch (cmd) {
    case CMD_OFFER:
      return rp_broker_dispatch(broker, ch, arg, FALSE);

    case CMD_BUY:
      return rp_broker_dispatch(broker, ch, arg, TRUE);

    case CMD_LIST:
      list_item_catalog(broker, ch);
      return TRUE;

    default:
      return FALSE;
  }
}

/*
** ========================================================================
** Registration -- same pattern every other spec file uses.
** RPBROKER_VNUM is defined up top with the other constants.
** ========================================================================
*/

void assign_rpupgrade(void)
{
  assign_mob(RPBROKER_VNUM, rp_broker);
}
