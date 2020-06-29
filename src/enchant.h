#ifndef __ENCHANT_H__
#define __ENCHANT_H__

#define ENCHANT_COLD          1
#define ENCHANT_FIREBREATH    2
#define ENCHANT_REGENERATION  3
#define ENCHANT_SQUIRE        4
#define ENCHANT_FIRSTSWORD    5
/* Was: ENCHANT_ALCOLYTE      6 */
#define ENCHANT_APPRENTICE    7
#define ENCHANT_TSUME         8
#define ENCHANT_WANDERER      9
#define ENCHANT_MINSTREL     10
#define ENCHANT_MINION       11
#define ENCHANT_PRIVATE      12
#define ENCHANT_HIGHWAYMAN   13
#define ENCHANT_ACOLYTE      14
#define ENCHANT_SWASHBUCKLER 15
#define ENCHANT_KNIGHT       16
#define ENCHANT_BISHOP       17
#define ENCHANT_PROPHET      18
#define ENCHANT_WARLOCK      19
#define ENCHANT_SORCERER     20
#define ENCHANT_BRIGAND      21
#define ENCHANT_ASSASSIN     22
#define ENCHANT_SHINOBI      23
#define ENCHANT_SHOGUN       24
#define ENCHANT_JUSTICIAR    25
#define ENCHANT_LORDLADY     26
#define ENCHANT_DARKWARDER   27
#define ENCHANT_DARKLORDLADY 28
#define ENCHANT_COMMODORE    29
#define ENCHANT_COMMANDER    30
#define ENCHANT_FORESTER     31
#define ENCHANT_TAMER        32
#define ENCHANT_POET         33
#define ENCHANT_CONDUCTOR    34

#define MAX_RANK              3

#define ENCHANT_WRATH        35
#define ENCHANT_ENVY         36
#define ENCHANT_LUST         37
#define ENCHANT_AVARICE      38
#define ENCHANT_PRIDE        39
#define ENCHANT_GLUTTONY     40
#define ENCHANT_SLOTH        41
#define ENCHANT_REDDEATH     42
#define ENCHANT_GREASY       43
#define ENCHANT_LIZARD       44

#define ENCHANT_TOXICFUMES   45

#define ENCHANT_FRIGHTFUL    46

#define ENCHANT_LICH         47

#define ENCHANT_REMORTV2     48
#define ENCHANT_IMM_GRACE    49

#define ENCH_MANTRA_HEAL     "ENCH_MANTRA_HEAL"
#define ENCH_APPLY_DMG_PCT   "ENCH_APPLY_DMG_PCT"

#define ENCH_INTERVAL_TICK   0 // Decrements every 60 seconds (MSG_TICK)
#define ENCH_INTERVAL_MOBACT 1 // Decrements every 10 seconds (MSG_MOBACT)
#define ENCH_INTERVAL_ROUND  2 // Decrements every 3 seconds  (MSG_ROUND)
#define ENCH_INTERVAL_USER   3 // User-defined interval; must be handled by spec.

#define TOTAL_ENCHANTMENTS   50

#define ENCHANTO(_name, _type, _duration, _interval, _modifier, _location, _bitvector, _bitvector2, _func) { \
  if (_type < TOTAL_ENCHANTMENTS) { \
    enchantments[_type].name       = strdup(_name); \
    enchantments[_type].type       = _type;         \
    enchantments[_type].duration   = _duration;     \
    enchantments[_type].interval   = _interval;     \
    enchantments[_type].modifier   = _modifier;     \
    enchantments[_type].location   = _location;     \
    enchantments[_type].bitvector  = _bitvector;    \
    enchantments[_type].bitvector2 = _bitvector2;   \
    enchantments[_type].func       = _func;         \
  } \
  else { \
    log_f("WARNING: Enchantment %s out of range (%d).", _name, _type); \
  } \
}

void assign_enchantments(void);

ENCH *get_enchantment_by_name(CHAR *ch, const char *name);
ENCH *get_enchantment_by_type(CHAR *ch, int type);
int enchanted_by(CHAR *victim, const char *name);
int enchanted_by_type(CHAR *victim, int type);
void enchantment_to_char(CHAR *victim, ENCH *enchantment, int must_find);
void enchantment_remove(CHAR *victim, ENCH *enchantment, int to_log);
void enchantment_apply(CHAR *ch, bool overwrite, const char *name, int type, sh_int duration, byte interval, int modifier, byte location, long bitvector, long bitvector2, int(*func)(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg));

int get_rank(CHAR *ch);
char *get_rank_name(CHAR *ch);

#endif /* __ENCHANT_H__ */
