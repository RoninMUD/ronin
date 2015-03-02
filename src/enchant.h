
/*
$Author: ronin $
$Date: 2005/01/21 14:55:27 $
$Header: /home/ronin/cvs/ronin/enchant.h,v 2.1 2005/01/21 14:55:27 ronin Exp $
$Id: enchant.h,v 2.1 2005/01/21 14:55:27 ronin Exp $
$Name:  $
$Log: enchant.h,v $
Revision 2.1  2005/01/21 14:55:27  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.0.0.1  2004/02/05 16:13:07  ronin
Reinitialization of cvs archives

Revision 1.3  2002/05/09 20:15:32  ronin
Added hell enchantments for saving through rent.
Increased maximum number of enchantments to 50.

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/

#ifndef __ENCHANT_H__
#define __ENCHANT_H__

#define TOTAL_ENCHANTMENTS 50

#define ENCHANTO(nme,tpe,dration,mdifier,lcation,btvector,btvector2,fnc) { \
                 if( (tpe) < TOTAL_ENCHANTMENTS )                 \
                  {                                               \
                   enchantments[tpe].duration  = (dration);       \
                   enchantments[tpe].modifier  = (mdifier);       \
                   enchantments[tpe].location  = (lcation);       \
                   enchantments[tpe].bitvector = (btvector);      \
                   enchantments[tpe].bitvector2 = (btvector2);    \
                   enchantments[tpe].func      = (fnc);           \
                   enchantments[tpe].name      = strdup( (nme) ); \
                  } else {                                        \
                   log_f("Enchantment out of range %s", (nme));   \
                  }                                              }


#define ENCHANT_COLD          1
#define ENCHANT_FIREBREATH    2
#define ENCHANT_REGENERATION  3
#define ENCHANT_SQUIRE        4
#define ENCHANT_FIRSTSWORD    5
/* Was: ENCHANT_ALCOLYTE      6 */
#define ENCHANT_APPRENTICE    7
#define ENCHANT_TSUME         8
#define ENCHANT_WANDERER      9
#define ENCHANT_TRAVELER     10
#define ENCHANT_MINION       11
#define ENCHANT_PRIVATE      12
#define ENCHANT_HIGHWAYMAN   13
#define ENCHANT_ACOLYTE      14
/* New 2nd and 3rd ranks - Ranger July 1998 */
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
/* End 2nd and 3rd Ranks */

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
/* End Hell Enchantments */

#define ENCHANT_TOXICFUMES   45
/* End Digsite Enchantments */

#define ENCHANT_FRIGHTFUL    46
/* End Red Dragons Enchantments */

#define ENCHANT_LICH         47
/* End Abyss Enchantments */

#define ENCHANT_REMORTV2     48
#define ENCHANT_IMM_GRACE    49

/* Added by Quack Early 1997 for testing
#define ENCHANT_SILENCE
#define ENCHANT_SHAPESHIFT
#define ENCHANT_QUESTER
int silence_enchantment(struct enchantment_type_5 *ench, CHAR *enchanted_ch, CHAR *char_in_room,int cmd, char*arg);
int shapeshift_enchantment(struct enchantment_type_5 *ench, CHAR *enchanted_ch, CHAR *char_in_room,int cmd, char*arg);
int quester_enchantment(struct enchantment_type_5 *ench, CHAR *enchanted_ch, CHAR *char_in_room,int cmd, char*arg);
*/

int cold_enchantment(struct enchantment_type_5 *ench, CHAR *enchanted_ch, CHAR *char_in_room,int cmd, char*arg);
int firebreath_enchantment(struct enchantment_type_5 *ench, CHAR *enchanted_ch, CHAR *char_in_room,int cmd, char*arg);
int regeneration_enchantment(struct enchantment_type_5 *ench, CHAR *enchanted_ch, CHAR *char_in_room,int cmd, char*arg);
int lord_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg);

void assign_enchantments(void);
struct enchantment_type_5 *get_enchantment(struct enchantment_type_5 *enchantment,int must_find);
void enchantment_to_char(CHAR *victim,struct enchantment_type_5 *enchantment,int must_find);
void enchantment_remove(CHAR *victim,struct enchantment_type_5 *enchantment, int tolog);
int enchanted_by(CHAR *victim,char *enchantment_name);
int enchanted_by_type(CHAR *victim, int type);
int get_rank(CHAR *ch);
char *get_rank_name(CHAR *ch);

#endif /* __ENCHANT_H__ */
