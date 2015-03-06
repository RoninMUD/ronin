/* ************************************************************************
*  file: spells.h , Implementation of magic spells.       Part of DIKUMUD *
*  Usage : Spells                                                         *
************************************************************************* */

/*
$Author: void $
$Date: 2004/11/19 17:20:33 $
$Header: /home/ronin/cvs/ronin/spells.h,v 2.10 2004/11/19 17:20:33 void Exp $
$Id: spells.h,v 2.10 2004/11/19 17:20:33 void Exp $
$Name:  $
$Log: spells.h,v $
Revision 2.10  2004/11/19 17:20:33  void
Changed name of lvl 50 ninja spell to mystic swiftness

Revision 2.9  2004/11/19 14:47:21  void
New Level 50 Ninja Spell (adds chance of 3rd hit)

Revision 2.8  2004/11/17 19:21:53  void
Added Nomad Skill Cover (1/2 Damage -10 Hitroll)

Revision 2.7  2004/10/21 17:08:22  void
Added level 50 AP spell Blood Lust

Revision 2.6  2004/09/29 22:56:13  void
Spell Rush for Commando

Revision 2.5  2004/09/28 14:28:01  void
Added Level 50 Cleric Spell Divine Intervention

Revision 2.4  2004/09/22 21:58:26  void
Added level 50 Mage spell Quick (Allows for 2 casts in a single combat
round)

Revision 2.3  2004/08/17 15:37:49  void
Added Warrior Skill Quad

Revision 2.2  2004/06/30 19:57:29  ronin
Update of saving throw routine.

Revision 2.1  2004/02/19 19:20:22  void
Added Spell Shroud of Existence (Good Align Commando Spell)

Revision 2.0.0.1  2004/02/05 16:13:38  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/


#define MAX_BUF_LENGTH              240

/* New damage types for skills/spells/specs - Ranger Oct 99 */
#define DAM_NO_BLOCK                  0 /* Not blocked */
#define DAM_POISON                    1
#define DAM_NO_BLOCK_NO_FLEE          2

#define DAM_PHYSICAL                 10 /* 10-49 Blocked by Invul (10 at 20 dam or less)*/
#define DAM_SKILL                    11

#define DAM_MAGICAL                  50 /* 50+ Blocked by Sphere */
#define DAM_FIRE                     51
#define DAM_COLD                     52
#define DAM_ELECTRIC                 53
#define DAM_SOUND                    54
#define DAM_CHEMICAL                 55
#define DAM_ACID                     56

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM_PERSON            7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOUR_SPRAY           10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_ALIGNMENT       18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVISIBLE       19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROTECT_FROM_EVIL      34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SNEAK                  45 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                   46 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                  47 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BACKSTAB               48 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK              49 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                   50 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                   51 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                 52
#define SKILL_BLOCK                  53
#define SKILL_KNOCK                  54
#define SKILL_PUNCH                  55
#define SKILL_PARRY                  56
#define SKILL_DUAL                   57
#define SKILL_THROW                  58
#define SKILL_DODGE                  59
#define SKILL_PEEK                   60
#define SKILL_BUTCHER                61
#define SKILL_TRAP                   62
#define SKILL_DISARM                 63
#define SKILL_SUBDUE                 64
#define SKILL_CIRCLE                 65
#define SKILL_TRIPLE                 66
#define SKILL_AMBUSH                 67
#define SKILL_SPIN_KICK              68
#define SKILL_ASSAULT                69
#define SKILL_DISEMBOWEL             70
#define SPELL_IDENTIFY               71
#define SPELL_CURE_SERIOUS           72
#define SPELL_INFRAVISION            73
#define SPELL_REGENERATION           74
#define SPELL_VITALITY               75
#define SPELL_CURE_LIGHT_SPRAY       76
#define SPELL_CURE_SERIOUS_SPRAY     77
#define SPELL_CURE_CRITIC_SPRAY      78
#define SPELL_HEAL_SPRAY             79
#define SPELL_DEATH_SPRAY            80
#define SPELL_HOLY_WORD              81
#define SPELL_ICEBALL                82
#define SPELL_TOTAL_RECALL           83
#define SPELL_RECHARGE               84
#define SPELL_MIRACLE                85
#define SPELL_FLY                    86
#define SPELL_MANA_TRANSFER          87
#define SPELL_HOLY_BLESS             88
#define SPELL_EVIL_BLESS             89
#define SPELL_SATIATE                90
#define SPELL_ANIMATE_DEAD           91
#define SPELL_GREAT_MIRACLE          92
#define SPELL_FLAMESTRIKE            93
#define SPELL_SPIRIT_LEVY            94
#define SPELL_LETHAL_FIRE            95
#define SPELL_HOLD                   96
#define SPELL_SPHERE                 97
#define SPELL_IMP_INVISIBLE          98
#define SPELL_INVUL                  99
#define SPELL_FEAR                  100
#define SPELL_FORGET                101
#define SPELL_FURY                  102
#define SPELL_ENDURE                103
#define SPELL_BLINDNESS_DUST        104
#define SPELL_POISON_SMOKE          105
#define SPELL_HELL_FIRE             106
#define SPELL_HYPNOTIZE             107
#define SPELL_RECOVER_MANA          108
#define SPELL_THUNDERBALL           109
#define SPELL_ELECTRIC_SHOCK        110
#define SPELL_PARALYSIS             111
#define SPELL_REMOVE_PARALYSIS      112
#define SPELL_DISPEL_GOOD           113
#define SPELL_EVIL_WORD             114
#define SPELL_REAPPEAR              115
#define SPELL_REVEAL                116
#define SPELL_RELOCATION            117
#define SPELL_LOCATE_CHARACTER      118
#define SPELL_SUPER_HARM            119
#define SKILL_PUMMEL                120
#define SPELL_LEGEND_LORE           121
#define SPELL_GREAT_MANA            122
#define SMELL_FARTMOUTH             123
#define SPELL_PERCEIVE              124
/*
125 free
*/
/* 126 increase mana */
#define SPELL_HASTE                 127 /*  Quack - Feb 96 */
#define SPELL_PW_KILL               128
#define SPELL_DISPEL_MAGIC          129
#define SPELL_CONFLAGRATION         130
#define SPELL_MASS_INVISIBILITY     131
#define SPELL_CONVERGENCE           132
#define SPELL_ENCHANT_ARMOUR        133
#define SPELL_DISINTEGRATE          134
#define SPELL_CONFUSION             135
#define SPELL_VAMPIRIC              136
#define SPELL_SEARING_ORB           137
#define SPELL_CLAIRVOYANCE          138
#define SPELL_FIREBREATH            139
#define SPELL_LAYHANDS              140
#define SPELL_DISPEL_SANCT          141 /* Ranger - Sept 96 */
#define SPELL_DISENCHANT            142 /* Quack - Oct  96 */
#define SPELL_PETRIFY               143 /* Quack - Oct 96 */
#define SKILL_BACKFLIP              144
#define SPELL_PROTECT_FROM_GOOD     145 /* Sumo - Dec 97 */
#define SPELL_REMOVE_IMP_INVIS      146 /* Ranger - Dec 98 */
#define SPELL_EXISTENCE             147 /* Void - Feb 04 */
#define SKILL_QUAD                  148 /* Void - Aug 04 */
#define SPELL_QUICK                 149 /* Void - Sept 04 */
#define SPELL_DIVINE_INTERVENTION   150 /* Void - Sept 04 */
#define SPELL_RUSH                  151 /* Void - Sept 04 */
#define SPELL_BLOOD_LUST            152 /* Void - Oct 04 */
#define SKILL_SCAN                  153 /* Used to be Cover */
#define SPELL_MYSTIC_SWIFTNESS      154 /* Void - Nov 04 */
#define SKILL_TWIST                 163 /* Void - Oct 04 */
#define SKILL_CUNNING               164
#define SPELL_WIND_SLASH            165 /* Project Tsurugi */
/*
#define SPELL_QUESTER
#define SPELL_SPIRITWRACK
#define SPELL_SPECTRAL_BLADE
#define SPELL_DOPPELGANGER
#define SPELL_SHAPESHIFT
#define SPELL_SILENCE
#define SPELL_TRANQUILITY
#define SPELL_LEECH
#define SPELL_ARMAGEDDON
*/

/* NOTE: Spells 150-162 should not be direct damage spells to avoid */
/*       conflict with hit types in the damage() function           */

#define TYPE_HIT                    150
#define TYPE_BLUDGEON               151
#define TYPE_PIERCE                 152
#define TYPE_SLASH                  153
#define TYPE_WHIP                   154
#define TYPE_CLAW                   155
#define TYPE_BITE                   156
#define TYPE_STING                  157
#define TYPE_CRUSH                  158
#define TYPE_HACK                   159
#define TYPE_CHOP                   160
#define TYPE_SLICE                  161
#define TYPE_SHADOW                 162

#define TYPE_SUFFERING              200

/* Sub-class Spell and Skill types - Start at 254 and go down. */
/* Spell and Skill procedures are in subclass.spells.c,
   subclass.skills.c and declarations are in subclass.h.
   Spell names, SPELLO,  spell wear off    are still added in
     spell_parser.c        constants.c
*/

#define SPELL_CLARITY               254
#define SPELL_MIGHT                 253
#define SPELL_BLADE_BARRIER         252
#define SPELL_PASSDOOR              251
#define SPELL_DISTORTION            250
#define SPELL_IRONSKIN              249
#define SPELL_FROSTBOLT             248
#define SKILL_ASSASSINATE           247
#define SPELL_ORB_PROTECTION        246
#define SKILL_BLITZ                 245
#define SPELL_SANCTIFY              244
#define SKILL_MEDITATE              243
#define SKILL_VICTIMIZE             242
#define SPELL_WRATH_ANCIENTS        241
#define SKILL_PROTECT               240
#define SPELL_RIGHTEOUSNESS         239
#define SPELL_RAGE                  238
#define SKILL_LUNGE                 237
#define SPELL_CLOUD_CONFUSION       236
#define SKILL_FADE                  235
#define SKILL_RETREAT               234
#define SKILL_BACKFIST              233
#define SKILL_PRAY                  232
#define SKILL_SWITCH                231
#define SKILL_DIRTY_TRICKS          230
#define SKILL_TRIP                  229
#define SKILL_TIGERKICK             228
#define SKILL_EVASION               227 /* Used to be Scan */
#define SPELL_RALLY                 226
#define SPELL_WARCHANT              225
#define SPELL_LUCK                  224
#define SKILL_CAMP                  223
/* 222 Used to be Evade */
#define SKILL_FEINT                 221
#define SKILL_AWARENESS             220
#define SKILL_BERSERK               219
#define SPELL_METEOR                218
#define SPELL_WALL_THORNS           217
#define SPELL_REJUVENATION          216
#define SKILL_FLANK                 215
#define SKILL_IMPAIR                214
#define SKILL_SWEEP                 213
#define SPELL_DIVINE_WIND           212
#define SPELL_BLACKMANTLE           211
#define SPELL_DARK_RITUAL           210
#define SPELL_RIMEFANG              209
#define SKILL_HOSTILE               208
#define SKILL_DEFEND                207
#define SPELL_DESECRATE             206
#define SKILL_BATTER                205
#define SKILL_SHADOW_WALK           204
#define SPELL_DEMONIC_THUNDER       203
#define SPELL_AID                   202
#define SKILL_CHARGE                201
/* skip 200 */
#define SKILL_HEADBUTT              199
#define SKILL_BANZAI                198
#define SKILL_MANTRA                197
#define SPELL_ENGAGE                196
#define SPELL_ETHEREAL_NATURE       195
#define SPELL_MAGIC_ARMAMENT        194
#define SPELL_DEGENERATE            193
#define SPELL_FORTIFICATION         192
#define SPELL_DISRUPT_SANCT         191
#define SPELL_WRATH_OF_GOD          190
#define SPELL_POWER_OF_DEVOTION     189
#define SPELL_FOCUS                 188
#define SPELL_POWER_OF_FAITH        187
#define SKILL_FRENZY                186
#define SKILL_TROPHY                185 /* Used to be Scalp */
#define SKILL_RIPOSTE               184
#define SKILL_UNUSED_183            183 /* Used to be close combat */
#define SPELL_INCENDIARY_CLOUD      182
#define SPELL_DEVASTATION           181
#define SPELL_SHADOW_WRAITH         180
#define SPELL_TREMOR                179
#define SKILL_VEHEMENCE             178 /* Used to be Execute */
#define SPELL_TRANQUILITY           177
#define SPELL_BLUR                  175
#define SKILL_CLOBBER               174
#define SPELL_MANA_HEAL             173
#define SPELL_DEBILITATE            172
/* stop before 165 */
/* More anything but spells and weapontypes can be inserted here! */

#define MAX_TYPES 255

#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define MAX_SPL_LIST  255

#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO    64 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024

struct spell_info_type
{
  void (*spell_pointer) (ubyte level, struct char_data *ch, char *arg, int type,
                         struct char_data *tar_ch, struct obj_data *tar_obj);
  ubyte minimum_position;  /* Position for caster              */
  sh_int min_usesmana;     /* Amount of mana used by a spell  */
  ubyte beats;             /* Heartbeats until ready for next */

  ubyte min_level_cleric;  /* Level required for cleric       switch these 2*/
  ubyte min_level_magic;   /* Level required for magic user   */
  ubyte min_level_ninja;   /* Level required for ninja        */
  ubyte min_level_paladin;
  ubyte min_level_anti_paladin;
  ubyte min_level_avatar;
  ubyte min_level_bard;
  ubyte min_level_commando;
  sh_int targets;         /* See below for use with TAR_XXX  */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
  char *singular;
  char *plural;
};

/* Extern procedures */
/* Format: cast_<spell name>(level,player,argument,type,victim,object)

           The values passed on to the actual spell <spell name> routine
           are level, player, victim, object.  The type is used for
           defining what casted the spell as above.  The only two spells
           currently using the argument are 'control weather' and
           'ventriloquate'.
*/
void do_mob_cast(struct char_data *ch, char *arg,int mode);

bool saves_spell(struct char_data *ch, sh_int spell, int level);
void cast_armor( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_teleport( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_bless( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_burning_hands( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_call_lightning( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_person( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_chill_touch( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shocking_grasp( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_clone( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_colour_spray( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_control_weather( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_food( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_water( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_blind( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_critic( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_curse( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_alignment( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_invisibility( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_magic( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_poison( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_evil( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_earthquake( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_weapon( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_energy_drain( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fireball( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_harm( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_invisibility( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_lightning_bolt( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_locate_object( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_magic_missile( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_poison( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_protection_from_evil( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_curse( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sleep( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_strength( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_summon( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_ventriloquate( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_word_of_recall( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_poison( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sense_life( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_identify( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_legend_lore( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_serious( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_infravision( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_regeneration( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_vitality( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light_spray( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_serious_spray( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_critic_spray( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal_spray( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_death_spray( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_holy_word( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_iceball( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_total_recall( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_recharge( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_miracle( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fly( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mana_transfer( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_holy_bless( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_evil_bless( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_satiate( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_animate_dead( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_great_miracle( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flamestrike( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_spirit_levy( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_lethal_fire( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_hold( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sphere( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_imp_invisibility( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_improved_invis( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_invulnerability( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fear( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_forget( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fury( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_endure( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness_dust( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_poison_smoke( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_hell_fire( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_hypnotize( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_recover_mana( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_great_mana( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_thunderball( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_electric_shock( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_paralyze( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_paralysis( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_good( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_evil_word( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_reappear( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_reveal( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_relocation( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_locate_character( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness_dust( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_super_harm( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_firebreath( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_layhands( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_magic( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_power_word_kill( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_conflagration( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mass_invisibility( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_convergence( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_armour( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_disintegrate( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_confusion( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_vampiric_touch( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_searing_orb( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_clairvoyance( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_regeneration( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_sanct( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_disenchant( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_petrify( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_protection_from_good( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_haste( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shroud_existence( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_quick( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_divine_intervention( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_rush( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blood_lust( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mystic_swiftness( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mana_heal( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_wind_slash(ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void spell_reappear(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_clairvoyance(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_searing_orb(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_vampiric_touch(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_conflagration(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_confusion(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_disintegrate(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_enchant_armour(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_convergence(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_dispel_magic(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_power_word_kill(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_mass_invisibility(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_reveal(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_paralyze(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_remove_paralysis(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_magic_missile(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_hell_fire(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_death_spray(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_death(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_chill_touch(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_burning_hands(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_shocking_grasp(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_lightning_bolt(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_flamestrike(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_colour_spray(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_energy_drain(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_fireball(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_iceball(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_lethal_fire(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_thunderball(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_electric_shock(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_earthquake(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_dispel_evil(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_dispel_good(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_evil_word(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_holy_word(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_call_lightning(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_harm(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_super_harm(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_armor(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_teleport(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_bless(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_blindness(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_blindness_dust(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_clone(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_control_weather(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_create_food(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_create_water(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_cure_blind(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_cure_critic(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_mana_transfer(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_holy_bless(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_evil_bless(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_cure_light_spray(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_cure_serious_spray(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_cure_critic_spray(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_heal_spray(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_great_miracle(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_cure_serious(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_cure_light(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_curse(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_detect_alignment(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_detect_invisibility(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_detect_magic(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_detect_poison(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_enchant_weapon(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_endure(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_fear(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_forget(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_fly(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_fury(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_heal(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_hold(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_invisibility(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_improved_invisibility(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_remove_improved_invis(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_locate_object(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_locate_character(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)   ;

void spell_poison(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_poison_smoke(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_protection_from_evil(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_recharge(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_remove_curse(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_remove_poison(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_sanctuary(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_satiate(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_sleep(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_hypnotize(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_sphere(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_invulnerability(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_strength(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_ventriloquate(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_word_of_recall(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_total_recall(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_summon(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_relocation(ubyte level, struct char_data *ch,
      struct char_data *victim, struct obj_data *obj);

void spell_charm_person(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_sense_life(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_vitality(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_miracle(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_recover_mana(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_great_mana(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_spirit_levy(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_identify(ubyte level, struct char_data *ch,
        struct char_data *victim, struct obj_data *obj);

void spell_legend_lore(ubyte level, struct char_data *ch,
        struct char_data *victim, struct obj_data *obj);

void spell_infravision(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_regeneration(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_animate_dead(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_layhands(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_regeneration(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_dispel_sanct(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_disenchant(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_petrify(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_protection_from_good(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_haste(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_firebreath(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_shroud_existence(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
  
void spell_quick(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj); 
  
void spell_divine_intervention(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_rush(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_blood_lust(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_mystic_swiftness(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_mana_heal(ubyte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
/*
void cast_quester( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_spiritwrack( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_spectral_blade( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_doppelganger( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_armageddon( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_tranquility( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_leech( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shapeshift( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_silence( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
*/
/*
void spell_quester(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_spiritwrack(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_spectral_blade(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_doppelganger(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_armageddon(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_leech(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_tranquility(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_shapeshift(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

void spell_silence(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
*/

/*
void spell_phase_door(ubyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

*/

void cast_perceive(ubyte level, struct char_data *ch, char *arg, int type,struct char_data *victim, struct obj_data *tar_obj );
void spell_perceive(ubyte level, struct char_data *ch, struct char_data *victim, struct obj_data *obj);
void spell_wind_slash(ubyte level, struct char_data *ch, struct char_data *victim, struct obj_data *obj); /* Project Tsurugi */

void magic_heal(struct char_data *ch, int spell, int heal, bool overheal);
