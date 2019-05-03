/* ************************************************************************
*  file: spells.h , Implementation of magic spells.       Part of DIKUMUD *
*  Usage : Spells                                                         *
************************************************************************* */

#ifndef __SPELLS_H__
#define __SPELLS_H__

/* Damage types for skills/spells/specs. */
#define DAM_NO_BLOCK                  0
#define DAM_POISON                    1
#define DAM_NO_BLOCK_NO_FLEE          2

/* 10 through 49 are blocked by Invulnerability */
#define DAM_PHYSICAL                 10
#define DAM_PHYSICAL_CRITICAL        11

/* 50+ are blocked by Sphere */
#define DAM_MAGICAL                  50
#define DAM_FIRE                     51
#define DAM_COLD                     52
#define DAM_ELECTRIC                 53
#define DAM_SOUND                    54
#define DAM_CHEMICAL                 55
#define DAM_ACID                     56

/* Reserved. Do not change. */
#define TYPE_UNDEFINED               -1 
#define SPELL_RESERVED_DBC            0
#define MAX_SPL_LIST                255

/* Spells and Skills */
#define SPELL_ARMOR                   1
#define SPELL_TELEPORT                2
#define SPELL_BLESS                   3
#define SPELL_BLINDNESS               4
#define SPELL_BURNING_HANDS           5
#define SPELL_CALL_LIGHTNING          6
#define SPELL_CHARM_PERSON            7
#define SPELL_CHILL_TOUCH             8
#define SPELL_CLONE                   9
#define SPELL_COLOUR_SPRAY           10
#define SPELL_CONTROL_WEATHER        11
#define SPELL_CREATE_FOOD            12
#define SPELL_CREATE_WATER           13
#define SPELL_CURE_BLIND             14
#define SPELL_CURE_CRITIC            15
#define SPELL_CURE_LIGHT             16
#define SPELL_CURSE                  17
#define SPELL_DETECT_ALIGNMENT       18
#define SPELL_DETECT_INVISIBLE       19
#define SPELL_DETECT_MAGIC           20
#define SPELL_DETECT_POISON          21
#define SPELL_DISPEL_EVIL            22
#define SPELL_EARTHQUAKE             23
#define SPELL_ENCHANT_WEAPON         24
#define SPELL_ENERGY_DRAIN           25 // Consider removal
#define SPELL_FIREBALL               26
#define SPELL_HARM                   27
#define SPELL_HEAL                   28
#define SPELL_INVISIBLE              29
#define SPELL_LIGHTNING_BOLT         30
#define SPELL_LOCATE_OBJECT          31
#define SPELL_MAGIC_MISSILE          32
#define SPELL_POISON                 33
#define SPELL_PROTECT_FROM_EVIL      34
#define SPELL_REMOVE_CURSE           35
#define SPELL_SANCTUARY              36
#define SPELL_SHOCKING_GRASP         37
#define SPELL_SLEEP                  38
#define SPELL_STRENGTH               39
#define SPELL_SUMMON                 40
#define SPELL_VENTRILOQUATE          41 // Consider removal
#define SPELL_WORD_OF_RECALL         42
#define SPELL_REMOVE_POISON          43
#define SPELL_SENSE_LIFE             44
#define SKILL_SNEAK                  45
#define SKILL_HIDE                   46
#define SKILL_STEAL                  47
#define SKILL_BACKSTAB               48
#define SKILL_PICK_LOCK              49
#define SKILL_KICK                   50
#define SKILL_BASH                   51
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
#define SPELL_FORGET                101 // Consider removal
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
#define SPELL_REVEAL                116 // Merge with Reappear
#define SPELL_RELOCATION            117
#define SPELL_LOCATE_CHARACTER      118
#define SPELL_SUPER_HARM            119
#define SKILL_PUMMEL                120
#define SKILL_COIN_TOSS             121
#define SPELL_GREAT_MANA            122
#define SMELL_FARTMOUTH             123
#define SPELL_PERCEIVE              124
#define SKILL_PRAY                  125
#define SKILL_ASSASSINATE           126
#define SPELL_HASTE                 127
#define SPELL_PW_KILL               128
#define SPELL_DISPEL_MAGIC          129
#define SPELL_CONFLAGRATION         130
#define SPELL_SKILL_131             131
#define SPELL_CONVERGENCE           132 // Consider removal
#define SPELL_ENCHANT_ARMOUR        133
#define SPELL_DISINTEGRATE          134
#define SKILL_HIDDEN_BLADE          135
#define SPELL_VAMPIRIC              136
#define SPELL_SEARING_ORB           137
#define SPELL_CLAIRVOYANCE          138
#define SPELL_FIREBREATH            139
#define SPELL_LAY_HANDS             140
#define SPELL_DISPEL_SANCT          141
#define SPELL_DISENCHANT            142
#define SPELL_PETRIFY               143
#define SKILL_BACKFLIP              144
#define SPELL_PROTECT_FROM_GOOD     145
#define SPELL_REMOVE_IMP_INVIS      146
#define SPELL_SKILL_147             147
#define SKILL_QUAD                  148
#define SPELL_QUICK                 149

/* Note: 150-162 should not be used for direct damage spells/skills to
         avoid conflicting with weapon attack types used in damage(). */
#define SPELL_DIVINE_INTERVENTION   150
#define SPELL_RUSH                  151
#define SPELL_BLOOD_LUST            152
#define SKILL_SCAN                  153
#define SPELL_MYSTIC_SWIFTNESS      154
#define SPELL_SKILL_155             155
#define SPELL_SKILL_156             156
#define SPELL_SKILL_157             157
#define SPELL_SKILL_158             158
#define SPELL_SKILL_159             159
#define SPELL_SKILL_160             160
#define SPELL_SKILL_161             161
#define SPELL_SKILL_162             162

#define SKILL_TWIST                 163
#define SKILL_CUNNING               164
#define SPELL_WIND_SLASH            165

/* Subclass Spells and Skills start here. */
#define SPELL_SKILL_166             166
#define SPELL_SKILL_167             167
#define SPELL_SKILL_168             168
#define SPELL_SKILL_169             169
#define SPELL_SKILL_170             170
#define SPELL_SKILL_171             171
#define SPELL_DEBILITATE            172
#define SPELL_MANA_HEAL             173
#define SKILL_CLOBBER               174
#define SPELL_BLUR                  175
#define SPELL_TRANQUILITY           177
#define SKILL_VEHEMENCE             178
#define SPELL_TREMOR                179
#define SPELL_SHADOW_WRAITH         180
#define SPELL_DEVASTATION           181
#define SPELL_INCENDIARY_CLOUD      182 /* Old Version */
#define SKILL_SNIPE                 183
#define SKILL_RIPOSTE               184
#define SKILL_TROPHY                185
#define SKILL_FRENZY                186
#define SPELL_POWER_OF_FAITH        187
#define SPELL_INCENDIARY_CLOUD_NEW  188 /* New Version */
#define SPELL_POWER_OF_DEVOTION     189
#define SPELL_WRATH_OF_GOD          190
#define SPELL_DISRUPT_SANCT         191
#define SPELL_FORTIFICATION         192
#define SPELL_DEGENERATE            193
#define SPELL_MAGIC_ARMAMENT        194
#define SPELL_ETHEREAL_NATURE       195
#define SPELL_ENGAGE                196
#define SKILL_MANTRA                197
#define SKILL_BANZAI                198
#define SKILL_HEADBUTT              199
#define SKILL_MAIM                  200
#define SPELL_BLESSING_OF_SACRIFICE 201
#define SPELL_AID                   202
#define SPELL_DEMONIC_THUNDER       203
#define SKILL_SHADOWSTEP            204
#define SKILL_BATTER                205
#define SPELL_DESECRATE             206
#define SKILL_DEFEND                207
#define SKILL_HOSTILE               208
#define SPELL_RIMEFANG              209
#define SPELL_WITHER                210
#define SPELL_BLACKMANTLE           211
#define SPELL_DIVINE_WIND           212
#define SKILL_ZEAL                  213
#define SKILL_IMPAIR                214
#define SKILL_FLANK                 215
#define SPELL_REJUVENATION          216
#define SPELL_WALL_THORNS           217
#define SPELL_METEOR                218
#define SKILL_BERSERK               219
#define SKILL_AWARENESS             220
#define SKILL_FEINT                 221
#define SKILL_SMITE                 222
#define SKILL_CAMP                  223
#define SPELL_LUCK                  224
#define SPELL_WARCHANT              225
#define SPELL_RALLY                 226
#define SKILL_EVASION               227
#define SKILL_TIGERKICK             228
#define SKILL_TRIP                  229
#define SKILL_DIRTY_TRICKS          230
#define SKILL_SWITCH                231
#define SKILL_TRUSTY_STEED          232
#define SKILL_BACKFIST              233
#define SKILL_RETREAT               234
#define SKILL_CLOSE_COMBAT          235
#define SPELL_CLOUD_CONFUSION       236
#define SKILL_LUNGE                 237
#define SPELL_RAGE                  238
#define SPELL_RIGHTEOUSNESS         239
#define SKILL_PROTECT               240
#define SPELL_WRATH_ANCIENTS        241
#define SKILL_VICTIMIZE             242
#define SKILL_MEDITATE              243
#define SPELL_SANCTIFY              244
#define SKILL_BLITZ                 245
#define SPELL_ORB_PROTECTION        246
#define SPELL_DUSK_REQUIEM          247
#define SPELL_FROSTBOLT             248
#define SPELL_IRON_SKIN             249
#define SPELL_DISTORTION            250
#define SPELL_PASSDOOR              251
#define SPELL_BLADE_BARRIER         252
#define SPELL_MIGHT                 253
#define SPELL_CLARITY               254

/* Attack Types. These values are shared with Spells/Skills, as noted above. */
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

/* More of anything but spells and weapon types can be inserted here. */
#define SAVING_PARA        0
#define SAVING_ROD         1
#define SAVING_PETRI       2
#define SAVING_BREATH      3
#define SAVING_SPELL       4

#define TAR_IGNORE         1
#define TAR_CHAR_ROOM      2
#define TAR_CHAR_WORLD     4
#define TAR_FIGHT_SELF     8
#define TAR_FIGHT_VICT    16
#define TAR_SELF_ONLY     32 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO     64 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV      128
#define TAR_OBJ_ROOM     256
#define TAR_OBJ_WORLD    512
#define TAR_OBJ_EQUIP   1024

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


/* Attack types with grammar */
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
void cast_lay_hands( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
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
void cast_quick( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_divine_intervention( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_rush( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blood_lust( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mystic_swiftness( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mana_heal( ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_wind_slash(ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_wither(ubyte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

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

void spell_lay_hands(ubyte level, struct char_data *ch,
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

void magic_heal(struct char_data *victim, int spell, int heal, bool overheal);

/* externals */

extern struct spell_info_type spell_info[MAX_SPL_LIST+1];

#endif /* __SPELLS_H__ */