/* ************************************************************************
*  file: constants.c                                      Part of DIKUMUD *
*  Usage: For constants used by the game.                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2004/12/29 17:30:19 $
$Header: /home/ronin/cvs/ronin/constants.c,v 2.17 2004/12/29 17:30:19 ronin Exp $
$Id: constants.c,v 2.17 2004/12/29 17:30:19 ronin Exp $
$Name:  $
$Log: constants.c,v $
Revision 2.17  2004/12/29 17:30:19  ronin
Removed AFF_TWIST and AFF_COVER

Revision 2.16  2004/11/19 17:20:33  void
Changed name of lvl 50 ninja spell to mystic swiftness

Revision 2.15  2004/11/19 14:47:20  void
New Level 50 Ninja Spell (adds chance of 3rd hit)

Revision 2.14  2004/11/17 19:21:52  void
Added Nomad Skill Cover (1/2 Damage -10 Hitroll)

Revision 2.13  2004/11/16 16:58:30  void
Fixed Quad to only affect level 50 warriors

Revision 2.12  2004/11/16 14:59:09  void
Changed the damage multipliers on Ambush, Assault, Circle, & Backstab based
on level.

Revision 2.11  2004/11/15 22:29:10  void
Made changes to Blood Lust, Twist, Quick, & Quad

Revision 2.10  2004/10/21 17:08:22  void
Added level 50 AP spell Blood Lust

Revision 2.9  2004/10/20 12:36:36  ronin
Fix of two minor typos.

Revision 2.8  2004/09/29 22:56:12  void
Spell Rush for Commando

Revision 2.7  2004/09/29 16:59:01  void
Added spells Quick and Divine Intervention

Revision 2.6  2004/08/19 12:57:18  ronin
Removed SKILL_QUAD from apply types list.

Revision 2.5  2004/08/17 15:47:23  void
*** empty log message ***

Revision 2.3  2004/08/16 18:10:05  void
Added Warrior Skill Quad

Revision 2.2  2004/03/08 21:00:24  ronin
Addition of QUESTWEAR to wear flags.

Revision 2.1  2004/02/19 19:20:22  void
Added Spell Shroud of Existence (Good Align Commando Spell)

Revision 2.0.0.1  2004/02/05 16:09:00  ronin
Reinitialization of cvs archives


Revision 19-Dec-03 Ranger
Added EMAIL to player_bits

Revision 22-Feb-03 Liner
Change of ambush mult to 7 over level 44 nomad, first reviews.

Revision 23-Jan-03 Liner
Changed back to Vamp Touch for 3rd wpn_spc

Revision 11-Dec-02 Ranger
Addition of IMMUNE_STEAL

Revision 1.3  2002/04/16 18:00:34  ronin
Addition of IMMUNE_EXECUTE

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/


#include "structs.h"
#include "limits.h"


const char *spell_wear_off_msg[] = {
  "RESERVED DB.C",
  "You feel less protected.",
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",
  "!Call Lightning",
  "You feel more self-confident.",
  "",
  "!Clone!",
  "!Color Spray!",
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",
  "!Cure Light!",
  "You feel better.",
  "You sense the red in your vision disappear.",
  "The detect invisible wears off.",
  "The detect magic wears off.",
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",
  "!Fireball!",
  "!Harm!",
  "!Heal",
  "You feel yourself exposed.",
  "!Lightning Bolt!",
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected from evil.",
  "!Remove Curse!",
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your surroundings.",
  "!Sneak!",  /* NO MESSAGE FOR SNEAK*/
  "!Hide!",
  "!Steal!",
  "!Backstab!",
  "!Pick Lock!",
  "!Kick!",
  "!Bash!",
  "!Rescue!",
  "!Block!",
  "!Knock!",
  "!Punch!",
  "!Parry!",
  "!Dual!",
  "!Throw!",
  "!Dodge!",
  "!Peek!",
  "!Butcher!",
  "!Trap!",
  "!Disarm!",
  "!Subdue!",
  "!Circle!",
  "!Triple!",
  "!Ambush!",
  "!Spin Kick!",
  "!Assault!",
  "!Disembowel!",
  "!Identify!",
  "!Cure Serious!",
  "Your eyes stop glowing red.",
  "Your regenerative power fades.", /* Regeneration (Not used by Spell: Regeneration) */
  "!Vitality!",
  "!Cure Light Spray!",
  "!Cure Serious Spray!",
  "!Cure Critic Spray!",
  "!Heal Spray!",
  "!Death Spray!",
  "!Holy Word!",
  "!IceBall!",
  "!Total Recall!",
  "!Recharge!",
  "!Miracle!",
  "You landed down on the ground successfully.",
  "!Mana Transfer!",
  "!Holy Bless!",
  "!Evil Bless!",
  "!Satiate!",
  "!Animate Dead!",
  "!Great Miracle",
  "!Flamestrike!",
  "!Spirit Levy!",
  "!Lethal Fire!",
  "You can move quickly now.",
  "Your golden sphere disappears.",
  "You appear out of thin air.",
  "Your powerful sphere disappears.",
  "!Fear!",
  "!Forget!",
  "You calm down.",
  "You feel less shielded.",
  "!Blindness dust!",
  "poison smoke!",
  "!hell fire!",
  "!hypnotize!",
  "!recover mana!",
  "!thunderball!",
  "!electric shock!",
  "The paralyzation wears off, you can move again",
  "!remove paralysis!",
  "!dispel good!",
  "!evil word!",
  "!reappear!",
  "!reveal!",
  "!relocation!",
  "!locate character!",
  "!super harm!",
  "!SKILL_PUMMEL!",         /* pummel */
  "!legend lore!",
  "!great mana!",
  "You finally get your hands on some mouthwash!",
  "Your depth of perception diminishes.",/*perceive*/
  "! !",
  "!increase mana!",
  "The speed of your movements slows down to normal.",   /* Quack - Feb 96 */
  "!power word kill!",
  "!dispel magic!",
  "!conflagration!",
  "!mass invisibility!",
  "!convergence!",
  "!enchant armour!",
  "!disintegrate!",
  "Your head clears and you no longer feel confused.",
  "!vampiric touch!",
  "!searing orb!",
  "!clairvoyance!",
  "!firebreath!",
  "!lay hands!",
  "!dispel sanctuary!", /* Ranger - Sept 96 */
  "!disenchant!",
  "!petrify!",
  "!backflip!",
  "The chaotic aura around your body begins to weaken.", /* Sumo - Dec 97 */
  "!remove impy!",
  "!shroud existence!",
  "!quad!",
  "Your thoughts slow.",
  "You feel as if the gods have forsaken you!",
  "Your pulse slows.",
  "The gnawing hunger slowly fades.",
  "!cover!",
  "Your hands slow down.",
  "","","","","", /*155-159*/
  "","","",/*160-162*/
  "!twist!",
  "!cunning!",
  "!wind slash!",
  "","","","","","", /* 166-171 */
  "You feel a little less weak.", /* 172 Debilitate */
  "!Mana Heal!",
  "",/*174*/
  "Your movements become less of a blur.", /* 175 Blur */
  "",/*176*/
  "Your sense of tranquility fades away.", /* 177 Tranquility */
  "!execute!",/*178*/
  "!tremor!", /*179*/
  "You shadow dissipates.",/*180 shadow wraith*/
  "!devastation!",/*181*/
  "The cloud of fire surrounding you dies out.",/*182*/
  "!close combat!",/*183*/
  "!riposte!",/*184*/
  "!scalp!",/*185*/
  "!frenzy!",/*186*/
  "!power of faith!",/*187*/
  "Your focus lessens.",/*188*/
  "The bright white aura around your body fades.",/*189*/
  "!wrath of god!",/*190*/
  "The force disrupting your white aura disappears.",/*191*/
  "Your feeling of fortification diminishes.",/*192*/
  "You feel strong enough to sacrifice your health for mana.",/*193*/
  "Your magic armament fades.",/*194*/
  "You feel strong enough to leave the physical plane.",/*195*/
  "!engage!",/*196*/
  "Your slowly slip out of your regenerative trance.",/*197*/
  "!banzai!", /*198*/
  "!headbutt!",/* 199 */
  "!skip!",/*200*/
  "!charge!",/*201*/
  "You don't feel as powerful.",/*202*/
  "!demonic thunder!",/*203*/
  "!shadow-walk!",/*204*/
  "!batter!",/*205*/
  "You feel the need to desecrate another corpse.",/*206*/
  "You place yourself in a normal stance.", /*207 - defend*/
  "You place yourself in a normal stance.", /*208 - hostile*/
  "The paralyzation wears off, you can move again", /*209 - rimefang*/
  "Your sinister presence diminishes.",/*210*/
  "Your mantle of darkness fades away.",/*211*/
  "!divine wind!",/*212*/
  "!sweep!",/*213*/
  "!impair!",/*214*/
  "!flank!",/*215*/
  "!rejuvenation!",/*216*/
  "!wall of thorns!",/*217*/
  "!meteor!",/*218*/
  "!berserk!",/*219*/
  "!awareness!",/*220*/
  "!feint!",/*221*/
  "!evade!",/*222*/
  "You break camp.",/*223*/
  "You don't feel as lucky.",/*224*/
  "You don't feel as safe.",/*225*/
  "Your sense of unity disappears.",/*226*/
  "!scan!",/*227*/
  "!tigerkick!",/*228*/
  "!trip!",/*229*/
  "!dirty tricks!",/*230*/
  "!switch!",/*231*/
  "You stop praying.",/*232*/
  "!backfist!",/*233*/
  "!retreat!",/*234*/
  "You seem more visible.",/*235*/
  "You feel less disoriented.",/*236*/
  "!lunge!",/*237*/
  "You calm down and feel less angry.",/*238*/
  "Your sense of righteousness disappears.",/*239*/
  "!protect!",/*240*/
  "!wrath of ancients!",/*241*/
  "", /*242*/
  "You feel less focused.",/*243*/
  "!sanctify!",/*244*/
  "!blitz!",/*245*/
  "Your little orb disappears.",/*246*/
  "!assassinate!",/*247*/
  "!frostbolt!",/*248*/
  "Your skin turns softer.",/*249*/
  "Your sphere returns to normal.",/*250*/
  "!passdoor!",/*251*/
  "One by one, the blades in your barrier fall to the ground.",/*252*/
  "You feel less powerful.",/*253*/
  "Your sense of precision disappears.",/*254*/
  "\n",
};


const int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4
};

const int movement_loss[]=
{
  1,  /* Inside     */
  2,  /* City       */
  2,  /* Field      */
  3,  /* Forest     */
  4,  /* Hills      */
  6,  /* Mountains  */
  4,  /* Swimming   */
  1,  /* Unswimmable */
  2,  /* Desert      */
  5   /* Arctic      */
};

const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};

const char *weekdays[7] = {
  "the Day of the Moon",
  "the Day of the Bull",
  "the Day of the Deception",
  "the Day of Thunder",
  "the Day of Freedom",
  "the day of the Great Gods",
  "the Day of the Sun"
};
const char *wpn_spc[] = {
"None",					/* 0 */
"Blind",				/* 1 */
"Poison",				/* 2 */
"Vampiric Touch",		/* 3 */
"Chill Touch",			/* 4 */
"Forget",				/* 5 */
"Curse",				/* 6 */
"Drain Mana",			/* 7 */
"None", 				/* 8 - Was Energy drain */
"Power Word Kill",					/* 9 */
"None",					/* 10 */
"None",					/* 11 */
"None",					/* 12 */
"None",					/* 13 */
"None",					/* 14 */
"None",					/* 15 */
"None",					/* 16 */
"None",					/* 17 */
"None",					/* 18 */
"None",					/* 19 */
"None",					/* 20 */
"Slay Evil Beings",		/* 21 */
"Slay Neutral Beings",	/* 22 */
"Slay Good Beings",		/* 23 */
"None",					/* 24 */
"None",					/* 25 */
"None",					/* 26 */
"None",					/* 27 */
"None",					/* 28 */
"None",					/* 29 */
"Chaotic",				/* 30 */
"Slay Magic-Users",		/* 31 */
"Slay Clerics",			/* 32 */
"Slay Thieves",			/* 33 */
"Slay Warriors",		/* 34 */
"Slay Ninjas",			/* 35 */
"Slay Nomads",			/* 36 */
"Slay Paladins",		/* 37 */
"Slay Anti-Paladins",	/* 38 */
"Slay Avatars",			/* 39 */
"Slay Bards",			/* 40 */
"Slay Commandos",		/* 41 */
"None",					/* 42 */
"None",					/* 43 */
"None",					/* 44 */
"None",					/* 45 */
"None",					/* 46 */
"None",					/* 47 */
"None",					/* 48 */
"None",					/* 49 */
"None",					/* 50 */
"Slay Liches",			/* 51 */	/* 51-100 sync'd with MOB CLASSES "see structs.h" */
"Slay Lesser Undead",	/* 52 */
"Slay Greater Undead",	/* 53 */
"Slay Lesser Vampires",	/* 54 */
"Slay Greater Vampires",/* 55 */
"Slay Lesser Dragons",	/* 56 */
"Slay Greater Dragons",	/* 57 */
"Slay Lesser Giants",	/* 58 */
"Slay Greater Giants",	/* 59 */
"Slay Lesser Lycanthropes",	/* 60 */
"Slay Greater Lycanthropes",/* 61 */
"Slay Lesser Demons",	/* 62 */
"Slay Greater Demons",	/* 63 */
"Slay Lesser Elementals",	/* 64 */
"Slay Greater Elementals",	/* 65 */
"Slay Lesser Planars",	/* 66 */
"Slay Greater Planars",	/* 67 */
"Slay Humanoids",		/* 68 */
"Slay Humans",			/* 69 */
"Slay Halflings",		/* 70 */
"Slay Dwarfs",			/* 71 */
"Slay Elves",			/* 72 */
"Slay Berserkers",		/* 73 */
"Slay Kenders",			/* 74 */
"Slay Trolls",			/* 75 */
"Slay Insectoids",		/* 76 */
"Slay Arachnoids",		/* 77 */
"Slay Fungi",			/* 78 */
"Slay Golems",			/* 79 */
"Slay Reptiles",		/* 80 */
"Slay Amphibians",		/* 81 */
"Slay Dinosaurs",		/* 82 */
"Slay Avians",			/* 83 */
"Slay Fish",			/* 84 */
"Slay Doppelgangers",	/* 85 */
"Slay Animals",			/* 86 */
"Slay Automatons",		/* 87 */
"Slay Simians",			/* 88 */
"Slay Canines",			/* 89 */
"Slay Felines",			/* 90 */
"Slay Bovines",			/* 91 */
"Slay Plants",			/* 92 */
"Slay Rodents",			/* 93 */
"Slay Blobs",			/* 94 */
"Slay Ghosts",			/* 95 */
"Slay Orcs",			/* 96 */
"Slay Gargoyles",		/* 97 */
"Slay Invertibrates",	/* 98 */
"Slay Drows",			/* 99 */
"Slay Statues"			/* 100 */
};
const char *weapon_type[15] = {
  "Whip",       /* 0 */
  "Whip",
  "Whip",
  "Slashing",   /* 3 */
  "Whip",
  "Sting",      /* 5 */
  "Crush",      /* 6 */
  "Bludgeon",   /* 7 */
  "Claw",       /* 8 */
  "Pierce",     /* 9 */
  "Pierce",
  "Pierce",
  "Hack",
  "Chop",
  "Slice"
  };
const char *month_name[17] = {
  "Month of Winter",           /* 0 */
  "Month of the Winter Wolf",
  "Month of the Frost Giant",
  "Month of the Old Forces",
  "Month of the Grand Struggle",
  "Month of the Spring",
  "Month of Nature",
  "Month of Futility",
  "Month of the Dragon",
  "Month of the Sun",
  "Month of the Heat",
  "Month of the Battle",
  "Month of the Dark Shades",
  "Month of the Shadows",
  "Month of the Long Shadows",
  "Month of the Ancient Darkness",
  "Month of the Great Evil"
};

const int sharp[] = {
   0,
   0,
   0,
   1,    /* Slashing */
   0,
   0,
   0,
   0,    /* Bludgeon */
   0,
   0,
   0,
   0
 };  /* Pierce   */

const char *where[] = {
  "<used as light>      ",
  "<worn on finger>     ",
  "<worn on finger>     ",
  "<worn around neck>   ",
  "<worn around neck>   ",
  "<worn on body>       ",
  "<worn on head>       ",
  "<worn on legs>       ",
  "<worn on feet>       ",
  "<worn on hands>      ",
  "<worn on arms>       ",
  "<worn as shield>     ",
  "<worn about body>    ",
  "<worn about waist>   ",
  "<worn around wrist>  ",
  "<worn around wrist>  ",
  "<wielded>            ",
  "<held>               "
};

const char *drinks[]=
{
"water",
"beer",
"wine",
"ale",
"dark ale",
"whisky",
"lemonade",
"firebreather",
"local speciality",
"slime mold juice",
"milk",
"tea",
"coffee",
"blood",
"salt water",
"coca cola",
"stout",
"vodka",
"rum",
"liquer",
"champagne",
"bourbon",
"tequila",
"hard cider",
"urine",
"gin",
"merlot",
"schnapps",
"moonshine",
"pus",
"sherbet",
"cognac",
"brandy",
"scotch",
"kefir",
"ouzo",
"saki",
"lager",
"\n"
};

const char *drinknames[]=
{
"water",
"beer",
"wine",
"ale",
"ale",
"whisky",
"lemonade",
"firebreather",
"local",
"juice",
"milk",
"tea",
"coffee",
"blood",
"salt",
"cola",
"stout",
"vodka",
"rum",
"liquer",
"champagne",
"bourbon",
"tequila",
"cider",
"urine",
"gin",
"merlot",
"schnapps",
"moonshine",
"pus",
"sherbet",
"cognac",
"brandy",
"scotch",
"kefir",
"ouzo",
"saki",
"lager",
"\n"
};

/*Here are the affects. In the {A,B,C},
A=Amount your made drunk
B=Amount your made full
C=Amount your made non-thirsty*/

const int drink_aff[][3] = {
{ 0,0,10 }, /* Water */
{ 3,2,5 }, /* beer */
{ 5,2,5 }, /* wine */
{ 2,2,5 }, /* ale */
{ 1,2,5 }, /* ale */
{ 6,0,4 }, /* Whiskey */
{ 0,0,8 }, /* lemonade */
{ 10,0,1 }, /* firebr */
{ 3,3,3 }, /* local */
{ 0,2,-8 }, /* slime */
{ 0,3,6 }, /* milk */
{-1,1,6 }, /* tea */
{-2,1,6 }, /* coffee */
{ 0,2,-1 }, /* blood */
{ 0,1,-2 }, /* saltwater*/
{ 0,1,5 }, /* coke */
{ 2,5,2 }, /* stout */
{ 8,0,0 }, /* vodka */
{ 7,1,0 }, /* rum */
{ 4,2,1 }, /* liquer */
{ 1,0,8 }, /* champagne */
{ 8,0,1 }, /* bourbon */
{ 9,0,0 }, /* tequila */
{ 4,0,3 }, /* cider */
{ 0,0,1 }, /* urine */
{ 8,0,1 }, /* gin */
{ 3,1,5 }, /* merlot */
{ 6,0,2 }, /*schnapps */
{ 10,0,-1 }, /* moonshine */
{ 1,3,-1 }, /* pus */
{ 0,2,9 }, /* sherbet */
{ 7,0,1 }, /* cognac */
{ 7,1,0 }, /* brandy */
{ 8,0,2 }, /* scotch */
{ 4,4,1 }, /* kefir */
{ 6,1,2 }, /* ouzo */
{ 4,1,1 }, /* saki */
{ 3,2,2 } /* lager */
};

const char *color_liquid[]=
{
"clear",
"brown",
"clear",
"brown",
"dark",
"golden",
"red",
"green",
"clear",
"light green",
"white",
"brown",
"black",
"red",
"clear",
"black",
"black",
"clear",
"red",
"chartreuse",
"pink",
"golden",
"golden",
"brown",
"yellow",
"clear",
"burgundy",
"clear",
"clear",
"foul",
"orange",
"brown",
"brown",
"golden",
"white",
"clear",
"clear",
"gold",
"\n"
};


const char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};

const struct title_type titles[11][58] =
{
  {
    {"the Man","the Woman"},
    {"the Novice","the Lady Novice"},
    {"the Holder of Books","the Lady Holder of Books"},
    {"the User of Cantrips","the Lady User of Cantrips"},
    {"the Apprentice of Magic","the Lady Apprentice of Magic"},
    {"the Scribe","the Scribess"},
    {"the Seer","the Seeress"},
    {"the Devotee of Magic","the Lady Devotee of Magic"},
    {"the Scholar","the Lady Scholar"},
    {"the Alchemist","the Lady Alchemist"},
    {"the Illusionist","the Lady Illusionist"},
    {"the Abjurer","the Abjuress"},
    {"the Invoker","the Invoker"},
    {"the Enchanter","the Enchantress"},
    {"the Transmuter","the Lady Transmuter"},
    {"the Spellsurger","the Lady Spellsurger"},
    {"the Evoker","the Lady Evoker"},
    {"the Mage","the Lady Mage"},
    {"the Conjurer","the Lady Conjurer"},
    {"the Slinger of Spells","the Lady Slinger of Spells"},
    {"the Blaster","the Lady Blaster"},
    {"the Arcane Caster","the Lady Arcane Caster"},
    {"the Wizard","the Lady Wizard"},
    {"the Weaver of Spells","the Lady Weaver of Spells"},
    {"the Mastermind","the Lady Mastermind"},
    {"the Visionary","the Lady Visionary"},
    {"the Magician","the Lady Magician"},
    {"the Warlock","the Witch"},
    {"the Tempest","the Lady Tempest"},
    {"the Elementalist","the Lady Elementalist"},
    {"the Explosive","the Lady Explosive"},
    {"the Mage","the Lady Mage"},
    {"the Deathmage","the Lady Deathmage"},
    {"the Raging Storm","the Lady Raging Storm"},
    {"the Bender of Reality","the Lady Bender of Reality"},
    {"the Sorcerer","the Sorceress"},
    {"the Warmage","the Lady Warmage"},
    {"the Savant of Magic","the Lady Savant of Magic"},
    {"the Shadowcaster","the Lady Shadowcaster"},
    {"the Weapon of Mass Destruction","the Lady Weapon of Mass Destruction"},
    {"the Master Sorcerer","the Master Sorceress"},
    {"the Force of Nature","the Lady Force of Nature"},
    {"the Master of Disaster","the Mistress of Disaster"},
    {"the Exhalted Mage","the Lady Exhalted Mage"},
    {"the Arcane Torrent","the Lady Arcane Torrent"},
    {"the Grand Master Sorcerer","the Grand Master Sorceress"},
    {"the Oncoming Storm","the Lady Oncoming Storm"},
    {"the Doctor of Strange","the Lady Doctor of Strange"},
    {"the Doctor of Fate","the Lady Doctor of Fate"},
    {"the Doctor of Doom","the Lady Doctor of Doom"},
    {"the Sorcerer Supreme","the Sorceress Supreme"},
    {"the Immortal Warlock","the Immortal Enchantress"},
    {"the Deity of Magic","the Deity Lady of Magic"},
    {"the Temporal of Magic","the Lady Temporal of Magic"},
    {"the Wizard of Magic","the Lady Wizard of Magic"},
    {"the Eternal of Magic","the Lady Eternal of Magic"},
    {"the Supreme Wizard of Magic","the Supreme Lady Wizard of Magic"} ,
    {"the God","the Goddess"} },
  {
    {"the Man","the Woman"},
    {"the Believer","the Believer"},
    {"the Attendant","the Attendant"},
    {"the Acolyte","the Acolyte"},
    {"the Novice","the Novice"},
    {"the Missionary","the Missionary"},
    {"the Adept","the Adept"},
    {"the Deacon","the Deaconess"},
    {"the Vicar","the Vicaress"},
    {"the Priest","the Priestess"},
    {"the Minister","the Lady Minister"},
    {"the Canon","the Canon"},
    {"the Levite","the Levitess"},
    {"the Curate","the Curess"},
    {"the Monk","the Nunne"},
    {"the Healer","the Healess"},
    {"the Chaplain","the Chaplain"},
    {"the Expositor","the Expositress"},
    {"the Bishop to be","the Bishop in seeking"},
    {"the Arch Bishop to be","the Arch Lady of the Church to be"},
    {"the Patriarch to be","the Matriarch in seeking"},
    {"the Excommunicator","the Excommunicatress"},
    {"the Benedictine","the Lady of the Benedictine"},
    {"the Patriarch","the Matriarch"},
    {"the Inquisitor","the Inquisitress"},
    {"the Expositor","the Expositress"},
    {"the Bishop","the Lady Bishop"},
    {"the Arch Bishop","the Arch Lady of the Church"},
    {"the Pope","the Lady Pope"},
    {"the Prophet","the Lady Prophet"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Hand of God","the Lady Hand of God"},
    {"the Immortal Cardinal","the Immortal Priestess"},
    {"the Holy Deity","the Holy Lady Deity"},
    {"the Holy Temporal","the Holy Lady Temporal"},
    {"the Holy Wizard","the Holy Lady Wizard"},
    {"the Holy Eternal","the Holy Lady Eternal"},
    {"the Supreme Holy Man","the Supreme Holy Lady"},
    {"the God","the Goddess"} },

  {
    {"the Man","the Woman"},
    {"the Urchin","the Lady Urchin"},
    {"the Hooligan","the Lady Hooligan"},
    {"the Pilferer","the Pilferess"},
    {"the Footpad","the Lady Footpad"},
    {"the Vagabond","the Lady Vagabond"},
    {"the Pick-Pocket","the Lady Pick-Pocket"},
    {"the Cheat","the Lady Cheat"},
    {"the Cut-Purse","the Lady Cut-Purse"},
    {"the Rogue","the Lady Rogue"},
    {"the Thief","the Lady Thief"},
    {"the Rapscallion","the Lady Rapscallion"},
    {"the Robber","the Robberess"},
    {"the Mugger","the Muggeress"},
    {"the Burglar","the Burglaress"},
    {"the Cloak","the Lady Cloak"},
    {"the Dagger","the Lady Dagger"},
    {"the Quick-Blade","the Lady Quick-Blade"},
    {"the Sharper","the Sharpress"},
    {"the Charlatan","the Lady Charlatan"},
    {"the Trickster","the Lady Trickster"},
    {"the Swindler","the Lady Swindler"},
    {"the Silver Tongue","the Lady Silver Tongue"},
    {"the Crook","the Lady Crook"},
    {"the Raider","the Raideress"},
    {"the Criminal","the Lady Criminal"},
    {"the Outlaw","the Lady Outlaw"},
    {"the Brigand","the Lady Brigand"},
    {"the Desperado","the Desperada"},
    {"the Nightstalker","the Lady Nightstalker"},
    {"the Obscure","the Lady Obscure"},
    {"the Invisible","the Lady Invisible"},
    {"the Unknown","the Lady Unknown"},
    {"the Anonymous","the Lady Anonymous"},
    {"the Cut-Throat","the Lady Cut-Throat"},
    {"the Death-Dealer","the Lady Death-Dealer"},
    {"the Killer","the Murderess"},
    {"the Assassin","the Lady Assassin"},
    {"the Executioner","the Lady Executioner"},
    {"the Terminator","the Lady Terminator"},
    {"the Master Thief","the Lady Master Thief"},
    {"the King of Shadows","the Queen of Shadows"},
    {"the Sultan of Spin","the Sultana of Spin"},
    {"the Exhalted Rogue","the Lady Exhaulted Rogue"},
    {"the Duke of Deception","the Duchess of Deception"},
    {"the Grand Master of Thieves","the Lady Grand Master of Thieves"},
    {"the Man Above the Law","the Woman Above the Law"},
    {"the Lord of Lies","the Lady of Lies"},
    {"the Prince of Guile","the Princess of Guile"},
    {"the Legend of the Night","the Legend of the Night"},
    {"Ronin's Most Wanted","Ronin's Most Wanted"},
    {"the Immortal Assassin","the Immortal Lady Assassin"},
    {"the Deity of Thieves","the Lady Deity of Thieves"},
    {"the Temporal of Thieves","the Lady Temporal of Thieves"},
    {"the Wizard of Thieves","the Lady Wizard of Thieves"},
    {"the Eternal of Thieves","the Lady Eternal of Thieves"},
    {"the Supreme of Thieves","the Lady Supreme of Thieves "},
    {"the God","the Goddess"} },

  {
    {"the Man","the Woman"},
    {"the Swordpupil","the Swordpupil"},
    {"the Recruit","the Recruit"},
    {"the Sentry","the Sentress"},
    {"the Fighter","the Fighter"},
    {"the Soldier","the Soldier"},
    {"the Warrior","the Warrior"},
    {"the Veteran","the Veteran"},
    {"the Swordsman","the Swordswoman"},
    {"the Fencer","the Fenceress"},
    {"the Combatant","the Combatess"},
    {"the Hero","the Heroine"},
    {"the Myrmidon","the Myrmidon"},
    {"the Swashbuckler","the Swashbuckleress"},
    {"the Mercenary","the Mercenaress"},
    {"the Swordmaster","the Swordmistress"},
    {"the Lieutenant","the Lieutenant"},
    {"the Champion","the Lady Champion"},
    {"the Dragoon","the Lady Dragoon"},
    {"the Cavalier","the Cavalier"},
    {"the Knight Hopeful","the Lady Knight Hopeful"},
    {"the Warlord","the Lady of War"},
    {"the Extirpator","the Queen of Destruction"},
    {"the Scourge","the Lady Scourge"},
    {"the Master Fighter","the Mistress Fighter"},
    {"the Swordmaster in Training","the Swordmistress in Training"},
    {"the Swordmaster","the Swordmistress"},
    {"the Grand Swordmaster","the Grand Swordmistress"},
    {"the Lesser Knight","the Lady Swordmistress"},
    {"the Knight","the Lesser Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Grand Knight","the Lady Knight"},
    {"the Immortal Warlord","the Immortal Lady of War"},
    {"the Deity of War","the Lady Deity of War"},
    {"the Temporal of War","the Lady Temporal of War"},
    {"the Wizard of War","the Lady Wizard of War"},
    {"the Eternal of War","the Lady Eternal of War"},
    {"the Supreme Warrior","the Lady Supreme Warrior "},
    {"the God","the Goddess"} },

  {
    {"the Man","the Woman"},
    {"the Ninja Pupil","the Lady Ninja Pupil"},
    {"the Ninja Trainee","the Lady Ninja Trainee"},
    {"the Tetsubishi Thrower","the Tetsubishi Thrower"},
    {"the Wall Climber","the Wall Climber"},
    {"the Spy In Training","the Lady Spy In Training"},
    {"the Genin Member","the Lady Genin Member"},
    {"the Ninja Swordsman","the Ninja Swordswoman"},
    {"the Castle Guard","the Lady Castle Guard"},
    {"the Spy","the Lady Spy"},
    {"the Genin Leader","the Genin Leader"},
    {"the Assailant","the Lady Assailant"},
    {"the Chunin","the Lady Chunin"},
    {"the Shinobi","the Lady Shinobi"},
    {"the Chunin Leader","the Chunin Leader"},
    {"the Ninja Teacher","the Lady Ninja Teacher"},
    {"the Black-garbed Ninja","the Lady Black-garbed Ninja"},
    {"the Yamabushi in Training","the Lady Yamabushi in Training"},
    {"the Joinin","the Lady Joinin"},
    {"the Ninja Master","the Ninja Master"},
    {"the Yamabushi","the Lady Yamabushi"},
    {"the Shadow Man","the Shadow Woman"},
    {"the Sennin","the Lady Sennin"},
    {"the Gyoja","the Lady Gyoja"},
    {"the Tsuba","the Lady Tsuba"},
    {"the Shogun","the Lady Shogun"},
    {"the Grand Sennin","the Grand Sennin"},
    {"the Shadow Master","the Shadow Master"},
    {"the Grand Tsuba","the Grand Tsuba"},
    {"the Grand Shogun","the Grand Shogun"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Grand Ninja","the Lady Grand Ninja"},
    {"the Immortal Ninja","the Immortal Lady of Ninja"},
    {"the Deity of Ninja","the Lady Deity of Ninja"},
    {"the Temporal of Ninja","the Lady Temporal of Ninja"},
    {"the Wizard of Ninja","the Lady Wizard of Ninja"},
    {"the Eternal of Ninja","the Lady Eternal of Ninja"},
    {"the Supreme Ninja","the Lady Supreme Ninja"},
    {"the God","the Goddess"} },

{
  {"the Man","the Woman"},
  {"the Nomad","the Lady Nomad"},
  {"the Peasant","the Lady Peasant"},
  {"the Ostler","the Lady Ostler"},
  {"the Farmer","the Lady Farmer"},
  {"the Master Peasant","the Master Peasant"},
  {"the Rider in Training","the Lady Rider in Training"},
  {"the Head of the Farmers","the Lady Head of the Farmers"},
  {"the Rider","the Lady Rider"},
  {"the Grand Rider","the Grand Lady Rider"},
  {"the Lumberjack","the Lady Lumberjack"},
  {"the Trap Maker","the Lady Trap Maker"},
  {"the Traveller","the Lady Traveller"},
  {"the Grand Lumberjack","the Grand Lumberjack"},
  {"the Forester","the Lady Forester"},
  {"the Butcher","the Lady Butcher"},
  {"the Bandit","the Lady Bandit"},
  {"the Botanist","the Lady Botanist"},
  {"the Grand Bandit","the Grand Lady Bandit"},
  {"the Hunter in Training","the Lady Hunter in Training"},
  {"the Hunter","the Lady Hunter"},
  {"the Forest Walker","the Lady Forest Walker"},
  {"the Tamer in Training","the Lady Tamer in Training"},
  {"the Forest Protector","the Lady Forest Protector"},
  {"the Tamer","the Lady Tamer"},
  {"the Great Tamer","the Great Lady Tamer"},
  {"the Great Hunter","the Great Lady Hunter"},
  {"the Great Forest Protector","the Great Lady Forest Protector"},
  {"the Forest Ruler","the Forest Ruler"},
  {"the Nomadic Leader","the Nomadic Leader"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Grand Nomad","the Grand Lady Nomad"},
  {"the Immortal Nomad","the Immortal Lady Nomads"},
  {"the Deity of Nomads","the Lady Deity of Nomads"},
  {"the Temporal of Nomads","the Lady Temporal of Nomads"},
  {"the Wizard of Nomads","the Lady Wizard of Nomads"},
  {"the Eternal of Nomads","the Lady Eternal of Nomads"},
  {"the Supreme of Nomads","the Lady Supreme of Nomads "},
  {"the God","the Goddess"} },

{
  {"the Man","the Woman"},
  {"the Paladin Pupil","the Paladin Pupil"},
  {"the Pious Novice","the Pious Novice"},
  {"the Gallant","the Lady Gallant"},
  {"the Seeker","the Lady Seeker"},
  {"the Quester","the Lady Quester"},
  {"the Keeper","the Lady Keeper"},
  {"the Protector","the Lady Protector"},
  {"the Defender","the Lady Defender"},
  {"the Warder","the Lady Warder"},
  {"the Guardian","the Lady Guardian"},
  {"the Chevalier","the Lady Chevalier"},
  {"the Justiciar","the Lady Justiciar"},
  {"the Templar","the Lady Templar"},
  {"the Crusader","the Lady Crusader"},
  {"the Knight Noble","the Lady Knight Noble"},
  {"the Lord Defender","the Lord Lady Defender"},
  {"the High Warder","the High Lady Warder"},
  {"the Holy Guardian","the Holy Lady Guardian"},
  {"the Holy Chevalier","the Holy Lady Chevalier"},
  {"the Holy Justiciar","the Holy Lady Justiciar"},
  {"the Holy Knight Templar","the Holy Lady Knight Templar"},
  {"the High Crusader","the High Lady Crusader"},
  {"the Lord of the Church","the Lady Lord of the Church"},
  {"the Paladin","the Lady Paladin"},
  {"the High Paladin","the High Lady Paladin"},
  {"the Knight Paladin","the Lady Knight Paladin"},
  {"the Paladin Lord","the Lady Paladin Lord"},
  {"the Paladin Duke","the Lady Paladin Duke"},
  {"the Paladin Earl","the Lady Paladin Earl"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Grand Paladin","the Lady Grand Paladin"},
  {"the Immortal Paladin","the Immortal Lady of Paladin"},
  {"the Deity of Paladin","the Lady Deity of Paladin"},
  {"the Temporal of Paladin","the Lady Temporal of Paladin"},
  {"the Wizard of Paladin","the Lady Wizard of Paladin"},
  {"the Eternal of Paladin","the Lady Eternal of Paladin"},
  {"the Supreme Paladin","the Lady Supreme Paladin"},
  {"the God","the Goddess"} },

{
  {"the Man","the Woman"},
  {"the Anti-Paladin Pupil","the Anti-Paladin Pupil"},
  {"the Pious Novice","the Pious Novice"},
  {"the Evil Gallant","the Evil Lady Gallant"},
  {"the Evil Seeker","the Evil Lady Seeker"},
  {"the Evil Quester","the Evil Lady Quester"},
  {"the Anti-Keeper","the Lady Anti-Keeper"},
  {"the Anti-Protector","the Lady Anti-Protector"},
  {"the Evil Defender","the Lady Evil Defender"},
  {"the Dark Warder","the Lady Dark Warder"},
  {"the Dark Guardian","the Lady Dark Guardian"},
  {"the Dark Chevalier","the Lady Dark Chevalier"},
  {"the Dark Justiciar","the Lady Dark Justiciar"},
  {"the Dark Templar","the Lady Dark Templar"},
  {"the Dark Crusader","the Lady Dark Crusader"},
  {"the Dark Knight","the Lady Dark Knight"},
  {"the High Defender","the High Lady Defender"},
  {"the High Dark Warder","the High Dark Lady Warder"},
  {"the Evil Guardian","the Evil Lady Guardian"},
  {"the Evil Chevalier","the Lady Evil Chevalier"},
  {"the Evil Justiciar","the Lady Evil Justiciar"},
  {"the Dark Knight Templar","the Lady Dark Knight Templar"},
  {"the High Dark Crusader","the High Lady Dark Crusader"},
  {"the Lord of the Evil Church","the Lady Lord of the Evil Church"},
  {"the Anti-Paladin","the Lady Anti-Paladin"},
  {"the High Anti-Paladin","the High Lady Anti-Paladin"},
  {"the Dark Knight Anti-Paladin","the Lady Dark Knight Anti-Paladin"},
  {"the Anti-Paladin Dark Lord","the Lady Anti-Paladin Dark Lord"},
  {"the Anti-Paladin Dark Duke","the Lady Anti-Paladin Dark Duke"},
  {"the Anti-Paladin Dark Earl","the Lady Anti-Paladin Dark Earl"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Grand Anti-Paladin","the Lady Grand Anti-Paladin"},
  {"the Immortal Anti-Paladin","the Immortal Lady of Anti-Paladin"},
  {"the Deity of Anti-Paladin","the Lady Deity of Anti-Paladin"},
  {"the Temporal of Anti-Paladin","the Lady Temporal of Anti-Paladin"},
  {"the Wizard of Anti-Paladin","the Lady Wizard of Anti-Paladin"},
  {"the Eternal of Anti-Paladin","the Lady Eternal of Anti-Paladin"},
  {"the Supreme Anti-Paladin","the Lady Supreme Anti-Paladin"},
  {"the God","the Goddess"} },

{ {"the Man","the Woman"},
  {"the New Avatar","the New Lady Avatar"},
  {"the Not So Clever","the Not So Clever"},
  {"the Quite Clever","the Quite Clever"},
  {"the Avatar Who Start To Be Clever","the Lady Avatar Who Start To Be Clever"},
  {"the Not So Brave","the Not So Brave"},
  {"the Quite Brave","the Quite Brave"},
  {"the Avatar Who Start To Be Brave","the Lady Avatar Who Start To Be Brave"},
  {"the Wimpy","the Lady Wimpy"},
  {"the Wimpy Again","the Lady Wimpy Again"},
  {"the Carry On Wimpy","the Carry On Wimpy"},
  {"the No More Wimpy","the No More Wimpy"},
  {"the Very Brave","the Very Brave"},
  {"the Very Brave But Still Wimpy","the Very Brave But Still Wimpy"},
  {"the Helper","the Lady Helper"},
  {"the Forever Helper","the Lady Forever Helper"},
  {"the Recaller","the Lady Recaller"},
  {"the Great Avatar","the Great Lady Avatar"},
  {"the Super Avatar","the Super Lady Avatar"},
  {"the Powerful One","the Lady Powerful One"},
  {"the Very Powerful One","the Very Powerful One"},
  {"the Immortal Left Hand","the Immortal Left Hand"},
  {"the Immortal Right Hand","the Immortal Right Hand"},
  {"the Immortal Left Food","the Immortal Left Foot"},
  {"the Immortal Right Food","the Immortal Right Foot"},
  {"the Avatar Who Say Neek","the Lady Avatar Who Say Neek"},
  {"the Avatar Who Say It","the Lady Avatar Who Say It"},
  {"the Arrrggghhh Avatar","the Arrrggghhh Lady Avatar"},
  {"the Quite Brave And Clever Avatar","the Quite Brave And Clever Lady Avatar"},
  {"the Brave And Clever Avatar","the Brave And Clever Lady Avatar"},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Grand Avatar","the Lady Grand Avatar",},
  {"the Immortal Avatar","the Immortal Lady of Avatar"},
  {"the Deity of Avatar","the Lady Deity of Avatar"},
  {"the Temporal of Avatar","the Lady Temporal of Avatar"},
  {"the Wizard of Avatar","the Lady Wizard of Avatar"},
  {"the Eternal of Avatar","the Lady Eternal of Avatar"},
  {"the Supreme Avatar","the Lady Supreme Avatar"},
  {"the God","the Goddess"} },

{ {"the Man","the Woman"},
  {"the Bard Newbie","the Lady Bard Newbie"},
  {"the Violin Pupil","the Violin Pupil"},
  {"the Viola Pupil","the Viola Pupil"},
  {"the Guitar Pupil","the Guitar Pupil"},
  {"the Flute Pupil","the Flute Pupil"},
  {"the Double-Bass Pupil","the Double-Bass Pupil"},
  {"the Piano Pupil","the Piano Pupil"},
  {"the Singer","the Singer"},
  {"the Banjo Player","the Banjo Pupil"},
  {"the Violinist","the Violinist"},
  {"the Flutist","the Flutist"},
  {"the Drummer","the Drummer"},
  {"the Guitar Player","the Guitar Player"},
  {"the Poet","the Poetess"},
  {"the Cello Player","the Cello Player"},
  {"the Piano Player","the Piano Player"},
  {"the Double-Bass Player","the Double-Bass Player"},
  {"the Musician","the Lady Musician"},
  {"the Poetic Traveler","the Poetic Traveler"},
  {"the Harpist","the Lady Harpist"},
  {"the Trumpeter","the Lady Trumpeter"},
  {"the Percussist","the Lady Percussist"},
  {"the Conductor","the Lady Conductor"},
  {"the Music Teacher","the Music Teacher"},
  {"the Lord Of Poet","the Lady Lord Of Poet"},
  {"the Super Singer","the Lady Super Singer"},
  {"the Grand Musician","the Grand Musician"},
  {"the Super Conductor","the Super Conductor"},
  {"the Lord Of Bard","the Lady Lord Of Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Grand Bard","the Lady Grand Bard"},
  {"the Immortal Bard","the Immortal Lady of Bard"},
  {"the Deity of Bard","the Lady Deity of Bard"},
  {"the Temporal of Bard","the Lady Temporal of Bard"},
  {"the Wizard of Bard","the Lady Wizard of Bard"},
  {"the Eternal of Bard","the Lady Eternal of Bard"},
  {"the Supreme Bard","the Lady Supreme Bard"},
  {"the God","the Goddess"} },

{ {"the Man","the Woman"},
  {"the Commando Pupil","the Lady Commando Pupil"},
  {"the Soldier In Training","the Lady Soldier In Training"},
  {"the Shooter In Training","the Lady Shooter In Training"},
  {"the Soldier","the Lady Soldier"},
  {"the Shooter","the Lady Shooter"},
  {"the Officer","the Lady Officer"},
  {"the Combatant","the Combantant"},
  {"the Bomber","the Lady Bomber"},
  {"the Army Member","the Army Lady Member"},
  {"the General Lieutenant","the General Lieutenant"},
  {"the Brave Soldier","the Brave Soldier"},
  {"the Assassin","the Lady Assassin"},
  {"the Rifle Man","the Rifle Woman"},
  {"the Brave Warrior","the Brave Warrior"},
  {"the Lieutenant","the Lady Lieutenant"},
  {"the Killer","the Killer"},
  {"the Soldier Leader","the Soldier Leader"},
  {"the Assassin Leader","the Assassin Leader"},
  {"the Ready-To-Die","the Ready-To-Die"},
  {"the Commander","the Lady Commander"},
  {"the Battle Master","the Battle Master"},
  {"the Army Leader","the Army Leader"},
  {"the Gunman","the Gunwoman"},
  {"the Commandant","the Lady Commandant"},
  {"the Commando Leader","the Commando Leader"},
  {"the Grand Gunman","the Grand Gunwoman"},
  {"the Lethal Weapon","the Lethal Weapon"},
  {"the Commodore","the Lady Commodore"},
  {"the Commander-In-Chief","the Lady Commando-In-Chief"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Grand Commando","the Lady Grand Commando"},
  {"the Immortal Commando","the Immortal Lady of Commando"},
  {"the Deity of Commando","the Lady Deity of Commando"},
  {"the Temporal of Commando","the Lady Temporal of Commando"},
  {"the Wizard of Commando","the Lady Wizard of Commando"},
  {"the Eternal of Commando","the Lady Eternal of Commando"},
  {"the Supreme Commando","the Lady Supreme Commando"},
  {"the God","the Goddess"} }
};

int exp_table[58] = {
  0,
  1,
  1850,
  1850,
  3700,
  7400,
  13800,
  25600,
  53800,
  97500,
  179000,/*10*/
  194500,
  234000,
  259000,
  285000,
  339000,
  349000,
  410000,
  455000,
  510000,
  530000,/*20*/
  575000,
  610000,
  715000,
  735000,
  825000,
  1150000,
  1400000,
  1950000,
  2450000,
  3375000,/*30*/
  5000000,
  7000000,
  9000000,
  12000000,
  15000000,
  18000000,
  22000000,
  26000000,
  30000000,
  35000000,/*40*/
  40000000,
  45000000,
  50000000,
  55000000,
  60000000,
  65000000,
  80000000,
  96000000,
  130000000,
  200000000,/*50*/
  300000000,
  400000000,
  500000000,
  600000000,
  700000000,
  800000000,
  1000000000  /*57*/
};

const char *item_types[] = {
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FIRE WEAPON",
  "MISSILE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "TRAP",
  "CONTAINER",
  "NOTE",
  "LIQUID CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "BULLET",
  "MUSICAL",
  "LOCKPICK",
  "2H-WEAPON",
  "BOARD",
  "TICKET",
  "SC_TOKEN",
  "SKIN",
  "SCALP",
  "RECIPE",
  "\n"
};

const char *wear_bits[] = {
  "TAKE",
  "FINGER",
  "NECK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "SHIELD",
  "ABOUT",
  "WAISTE",
  "WRIST",
  "WIELD",
  "HOLD",
  "THROW",
  "LIGHT-SOURCE",
  "NO_REMOVE",
  "NO_SCAVENGR",
  "QUESTWEAR",
  "\n"
};

const char *extra_bits[] = {
  "GLOW",
  "HUM",
  "DARK",
  "CLONED",
  "EVIL",
  "INVISIBLE",
  "MAGICAL",
  "NODROP",
  "BLESSED",
  "ANTI-GOOD",
  "ANTI-EVIL",
  "ANTI-NEUTRAL",
  "ANTI-WARRIOR",
  "ANTI-THIEF",
  "ANTI-CLERIC",
  "ANTI-MAGIC_USER",
  "ANTI-MORTAL",
  "ANTI-MALE",
  "ANTI-FEMALE",
  "ANTI-RENT",
  "ANTI-NINJA",
  "ANTI-NOMAD",
  "ANTI-PALADIN",
  "ANTI-ANTI-PALADIN",
  "ANTI-AVATAR",
  "ANTI-BARD",
  "ANTI-COMMANDO",
  "LIMITED",
  "ANTI-AUCTION",
  "CHAOTIC",
  "\n"
};

const char *extra_bits2[] = {
  "RANDOM",
  "ALL_DECAY",
  "EQ_DECAY",
  "NO_GIVE",
  "NO_GIVE_MOB",
  "NO_PUT",
  "NO_TAKE_MOB",
  "NO_SCALP",
  "NO_LOCATE",
  "RANDOM_0",
  "RANDOM_1",
  "RANDOM_2",
  "\n"
};

const char *subclass_res_bits[] = {
	"ANTI_ENCHANTER",
	"ANTI_ARCHMAGE",
	"ANTI_DRUID",
	"ANTI_TEMPLAR",
	"ANTI_ROGUE",
	"ANTI_BANDIT",
	"ANTI_WARLORD",
	"ANTI_GLADIATOR",
	"ANTI_RONIN",
	"ANTI_MYSTIC",
	"ANTI_RANGER",
	"ANTI_TRAPPER",
	"ANTI_CAVALIER",
	"ANTI_CRUSADER",
	"ANTI_DEFILER",
	"ANTI_INFIDEL",
	"ANTI_BLADESINGER",
	"ANTI_CHANTER",
	"ANTI_LEGIONNAIRE",
	"ANTI_MERCENARY",
  "\n"
};

const char *room_bits[] = {
  "DARK",
  "DEATH",
  "NO_MOB",
  "INDOORS",
  "LAWFUL",
  "CHAOTIC",
  "SAFE",
  "NO_MAGIC",
  "TUNNEL",
  "PRIVATE",
  "LOCK",
  "TRAP",
  "ARENA",
  "CLUB",
  "QUIET",  /* Ranger - July 96 */
  "NO_BEAM",
  "HAZARD",
  "MOVETRAP",
  "FLYING",
  "NO_PEEK",
  "NO_SONG",
  "NO_REGEN",
  "NO_QUAFF",
  "REV_REGEN",
  "DOUBLE_MANA",
  "HALF_CONC",
  "LIT",
  "NO_ORB",
  "QRTR_CONC",
  "MANADRAIN",
  "NO_SUM",
  "\n"
};

/* Removed RSCLOSED and RSLOCKED - added LOCK 1-5 - Ranger July 96 */

const char *exit_bits[] = {
  "IS-DOOR",
  "CLOSED",
  "LOCKED",
  "PICKPROOF",
  "LOCK 10",
  "LOCK 15",
  "LOCK 20",
  "LOCK 25",
  "LOCK 30",
  "CLIMB",
  "JUMP",
  "MOVE",
  "CRAWL",
  "ENTER",
 "\n"
};

const char *sector_types[] = {
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water Swim",
  "Water NoSwim",
  "Desert",
  "Arctic",
  "\n"
};

const char *equipment_types[] = {
  "Special",
  "Worn on right finger",
  "Worn on left finger",
  "First worn around Neck",
  "Second worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on legs",
  "Worn on feet",
  "Worn on hands",
  "Worn on arms",
  "Worn as shield",
  "Worn about body",
  "Worn around waiste",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Held",
  "\n"
};

const char *affected_bits[] =  {
  "BLIND",
  "INVISIBLE",
  "DETECT-ALIGNMENT",
  "DETECT-INVISIBLE",
  "DETECT-MAGIC",
  "SENSE-LIFE",
  "HOLD",
  "SANCTUARY",
  "GROUP",
  "UNUSED",
  "CURSE",
  "SPHERE",
  "POISON",
  "PROTECT-EVIL",
  "PARALYSIS",
  "INFRAVISION",
  "STATUE",
  "SLEEP",
  "DODGE",
  "SNEAK",
  "HIDE",
  "ANIMATE",
  "CHARM",
  "PROTECT-GOOD",
  "FLY",
  "SUBDUE",
  "IMINV",
  "INVUL",
  "DUAL",
  "FURY",
  "\n"
};

const char *affected_bits2[] =  {
  "TRIPLE",
  "IMMINENT-DEATH",
  "SEVERED",
  "QUAD",
  "\n"
};

const char *apply_types[] = {
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "SEX",
  "CLASS",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MANA",
  "HIT",
  "MOVE",
  "GOLD",
  "EXP",
  "ARMOR",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "SKILL_SNEAK",
  "SKILL_HIDE",
  "SKILL_STEAL",
  "SKILL_BACKSTAB",
  "SKILL_PICKLOCK",
  "SKILL_KICK",
  "SKILL_BASH",
  "SKILL_RESCUE",
  "SKILL_BLOCK",
  "SKILL_KNOCK",
  "SKILL_PUNCH",
  "SKILL_PARRY",
  "SKILL_DUAL",
  "SKILL_THROW",
  "SKILL_DODGE",
  "SKILL_PEEK",
  "SKILL_BUTCHER",
  "SKILL_TRAP",
  "SKILL_DISARM",
  "SKILL_SUBDUE",
  "SKILL_CIRCLE",
  "SKILL_TRIPLE",
  "SKILL_PUMMEL",
  "SKILL_AMBUSH",
  "SKILL_ASSAULT",
  "SKILL_DISEMBOWEL",
  "SKILL_BACKFLIP",
  "HP_REGEN",
  "MANA_REGEN",
  "\n"
};

const char *immortal_types[] = {
  "Immortal",
  "Deity",
  "Temporal",
  "Wizard",
  "Eternal",
  "Supreme",
  "Implementor",
  "\n"
};

const char *pc_class_types[] = {
  "UNDEFINED",
  "Magic User",
  "Cleric",
  "Thief",
  "Warrior",
  "Ninja",
  "Nomad",
  "Paladin",
  "Anti-Paladin",
  "Avatar",
  "Bard",
  "Commando",
  "\n"
};

const char *npc_class_types[] = {
  "Normal",
  "Magic User",
  "Cleric",
  "Thief",
  "Warrior",
  "Ninja",
  "Nomad",
  "Paladin",
  "Anti-Paladin",
  "Avatar",
  "Bard",
  "Commando",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",    /* 20 - Max Classes possible */
  "","","","","","","","","","","","","","","","","","","","","","","","","","","","","","",
  "Lich",  /* 51 - Start of mob classes */
  "Lesser Undead",
  "Greater Undead",
  "Lesser Vampire",
  "Greater Vampire",
  "Lesser Dragon",
  "Greater Dragon",
  "Lesser Giant",
  "Greater Giant",
  "Lesser Lycanthrope",
  "Greater Lycanthrope",
  "Lesser Demon",
  "Greater Demon",
  "Lesser Elemental",
  "Greater Elemental",
  "Lesser Planar",
  "Greater Planar",
  "Humanoid",
  "Human",
  "Halfling",
  "Dwarf",
  "Elf",
  "Berserker",
  "Kender",
  "Troll",
  "Insectoid",
  "Arachnoid",
  "Fungus",
  "Golem",
  "Reptile",
  "Amphibian",
  "Dinosaur",
  "Avian",
  "Fish",
  "Doppelganger",
  "Animal",
  "Automaton",
  "Simian",
  "Canine",
  "Feline",
  "Bovine",
  "Plant",
  "Rodent",
  "Blob",
  "Ghost",
  "Orc",
  "Gargoyle",
  "Invertibrate",
  "Drow",
  "Statue",
  "\n"
};

const char *action_bits[] = {
  "SPEC",
  "SENTINEL",
  "SCAVENGER",
  "ISNPC",
  "NICE-THIEF",
  "AGGRESSIVE",
  "STAY-ZONE",
  "WIMPY",
  "SUBDUE",
  "OPEN-DOOR",
  "AGGWA",
  "AGGTH",
  "AGGCL",
  "AGGMU",
  "MEMORY",
  "AGGNI",
  "AGGNO",
  "ARM",
  "MOUNT", /* Mount and fly added for stables - Ranger April 96 */
  "FLY",
  "AGGPA", /* Four AGG flags added - Ranger April 96 */
  "AGGAP",
  "AGGBA",
  "AGGCO",
  "SHIELD", /* New mob magic shield added - Ranger May 96 */
  "AGGEVIL", /* 5 more aggressive acts - Ranger May 96 */
  "AGGGOOD",
  "AGGNEUT",
  "AGGLEADER",
  "AGGRANDOM",
  "\n"
};

const char *action_bits2[] = {
  "NO-TOKEN",
  "IGNORE-SPHERE",
  "\n"
};

const char *resist_bits[] = {
  "POISON",
  "PHYSICAL",
  "MAGICAL",
  "FIRE",
  "COLD",
  "ELECTRIC",
  "SOUND",
  "CHEMICAL",
  "ACID",
  "\n"
};

const char *immune_bits[] =  {
  "FIRE",
  "ELECTRIC",
  "POISON",
  "PUMMEL",
  "KICK",
  "PUNCH",
  "SLEEP",
  "CHARM",
  "BLINDNESS",
  "PARALYSIS",
  "DRAIN",
  "DISEMBOWEL",
  "DISINTEGRATE",
  "CLAIR",
  "SUMMON",
  "HIT",
  "BLUDGEON",
  "PIERCE",
  "SLASH",
  "WHIP",
  "CLAW",
  "BITE",
  "STING",
  "CRUSH",
  "HACK",
  "CHOP",
  "SLICE",
  "BACKSTAB",
  "AMBUSH",
  "ASSAULT",
  "\n"
};

const char *immune_bits2[] =  {
  "LOCATE",
  "COLD",
  "SOUND",
  "CHEMICAL",
  "ACID",
  "FEAR",
  "<UNUSED>",
  "CIRCLE",
  "EXECUTE",
  "STEAL",
  "\n"
};

const char *att_targets[] =  {
  "NONE",
  "BUFFER",
  "RAN_GROUP",
  "RAN_ROOM",
  "GROUP",
  "ROOM",
  "SELF",
  "LEADER",
  "\n"
};

const char *att_types[] =  {
  "NONE",
  "SPELL CAST",
  "KICK",
  "PUMMEL",
  "PUNCH",
  "BITE",
  "CLAW",
  "BASH",
  "TAILSLAM",
  "DISARM",
  "TRAMPLE",
  "SPELL SKILL",
  "\n"
};


const char *sector_types_i[] = {
  "INSIDE",
  "CITY",
  "FIELD",
  "FOREST",
  "HILLS",
  "MOUNTAIN",
  "SWIMMABLE WATER",
  "NOSWIM WATER",
  "DESERT",
  "ARCTIC",
  "\n"
};

const char *player_bits[] = {
  "BRIEF",
  "NOSHOUT",
  "COMPACT",
  "WRITING",
  "KILL",
  "THIEF",
  "NOKILL",
  "UNUSED",/*to be used for something else*/
  "NOSUMMON",
  "GOSSIP",
  "AUCTION",
  "BLOCK",
  "NOMESSAGE",
  "NONAME-CLUB",
  "D.ADDER-CLUB",
  "CASTER-CLUB",
  "AZOUN-CLUB",
  "Q-PLAYER", /* For IMM+ run quest - Ranger June 96 */
  "CHAOS",
  "DEPUTY",
  "Q-QUIET",
  "Q-CHANNEL",
  "SUPERBRIEF",
  "FIGHTBRIEF",
  "SKIPTITLE",
  "VICIOUS",
  "NOYELL",
  "EMAIL",
  "\n"
};

const char *wiz_bits[] = {
  "TRUST",
  "QUEST",
  "FREEZE",
  "LOAD",
  "NO WIZNET",
  "LOG_1",
  "LOG_2",
  "LOG_3",
  "LOG_4",
  "LOG_5",
  "CREATE",
  "LOG_6",
  "QUESTLOG",
  "JUDGE",
  "ACTIVE",
  "WIZNET",
  "CHAOS",
  "SNOOP_BRIEF",
  "\n"
};

const char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "Flying",
  "Riding",
  "Swimming",
  "\n"
};

const char *connected_types[]	=	{
  "Playing",
  "Get name",
  "Confirm name",
  "Read Password",
  "Get new password",
  "Confirm new password",
  "Get sex",
  "Read Motd",
  "Read Menu",
  "Enter description",
  "Get class",
  "Link Dead",
  "Change password",
  "Confirm new password",
  "Waiting to allowin",
  "\n"
};

/* [class], [level] (all) */
const int thaco[11][58] = {
{ 100, /* Mage */
    20,20,20,19,19,19,18,18,18,17,
    17,17,16,16,16,15,15,15,14,14,
    14,13,13,13,12,12,12,11,11,10,
    10,10,10,10, 9, 9, 9, 9, 9, 8,
     8, 8, 8, 8, 7, 7, 7, 7, 7, 6,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Cleric */
    20,20,20,19,19,19,18,18,18,17,
    17,17,16,16,16,15,15,15,14,14,
    14,13,13,13,12,12,12,11,11,10,
    10,10,10,10, 9, 9, 9, 9, 9, 8,
     8, 8, 8, 8, 7, 7, 7, 7, 7, 6,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Thief */
    20,20,20,18,18,18,16,16,16,14,
    14,14,12,12,12,11,11,11,10,10,
    10, 9, 9, 9, 8, 8, 8, 8, 8, 7,
     7, 7, 7, 7, 6, 6, 6, 6, 6, 5,
     5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Warrior */
    20,20,20,18,18,18,16,16,16,14,
    14,14,12,12,12,11,11,11,10,10,
    10, 9, 9, 9, 8, 8, 8, 8, 8, 7,
     7, 7, 7, 7, 6, 6, 6, 6, 6, 5,
     5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Ninja */
    20,20,20,18,18,18,16,16,16,14,
    14,14,12,12,11,11,10,10, 9, 9,
     8, 8, 8, 7, 7, 7, 6, 6, 6, 5,
     5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
     3, 3, 3, 3, 2, 2, 2, 2, 2, 1,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Nomad */
    20,20,20,18,18,18,16,16,16,14,
    14,14,12,12,12,11,11,11,10,10,
    10, 9, 9, 9, 8, 8, 8, 8, 8, 7,
     7, 7, 7, 7, 6, 6, 6, 6, 6, 5,
     5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Paladin */
    20,20,20,18,18,18,16,16,16,14,
    14,14,12,12,11,11,10,10, 9, 9,
     8, 8, 8, 7, 7, 7, 6, 6, 6, 5,
     5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
     3, 3, 3, 3, 2, 2, 2, 2, 2, 1,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Antipaladin */
    20,20,20,18,18,18,16,16,16,14,
    14,14,12,12,11,11,10,10, 9, 9,
     8, 8, 8, 7, 7, 7, 6, 6, 6, 5,
     5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
     3, 3, 3, 3, 2, 2, 2, 2, 2, 1,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Avatar */
    20,20,20,18,18,18,16,16,16,14,
    14,14,12,12,11,11,10,10, 9, 9,
     8, 8, 8, 7, 7, 7, 6, 6, 6, 5,
     5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
     3, 3, 3, 3, 2, 2, 2, 2, 2, 1,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Bard */
    20,20,20,18,18,18,16,16,16,14,
    14,14,12,12,12,11,11,11,10,10,
    10, 9, 9, 9, 8, 8, 8, 7, 7, 6,
     6, 6, 6, 6, 5, 5, 5, 5, 5, 4,
     3, 3, 3, 3, 3, 3, 3, 3, 3, 2,
    1,1,1,1,1,1,1}, /*51-57*/
{ 100, /* Commando */
    20,20,20,18,18,18,16,16,16,14,
    14,14,12,12,11,11,10,10, 9, 9,
     8, 8, 8, 7, 7, 7, 6, 6, 6, 5,
     5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
     3, 3, 3, 3, 2, 2, 2, 2, 2, 1,
    1,1,1,1,1,1,1} /*51-57*/
};

/* [ch] strength apply (all) */
/* hitroll, dam, carry weight, wield_weight */
const struct str_app_type str_app[31] = {
{ -5,-4,   0,  0 },  /* 0  */
{ -5,-4,   3,  1 },  /* 1  */
{ -3,-2,   3,  2 },
{ -3,-1,  10,  3 },  /* 3  */
{ -2,-1,  25,  4 },
{ -2,-1,  55,  5 },  /* 5  */
{ -1, 0,  80,  6 },
{ -1, 0,  90,  7 },
{  0, 0, 100,  8 },
{  0, 0, 100,  9 },
{  0, 0, 115, 10 }, /* 10  */
{  0, 0, 115, 11 },
{  0, 0, 140, 12 },
{  0, 0, 140, 13 },
{  0, 0, 170, 14 },
{  0, 0, 170, 15 }, /* 15  */
{  0, 1, 195, 16 },
{  1, 1, 220, 18 },
{  1, 2, 255, 20 }, /* 18  */
{  3, 7, 640, 31 },
{  3, 8, 700, 32 }, /* 20  */
{  4, 9, 810, 33 },
{  4,10, 970, 34 },
{  5,11,1130, 35 },
{  6,12,1440, 36 },
{  7,14,1750, 37 }, /* 25            */
{  1, 3, 280, 22 }, /* 18/01-50      */
{  2, 3, 305, 24 }, /* 18/51-75      */
{  2, 4, 330, 26 }, /* 18/76-90      */
{  2, 5, 380, 28 }, /* 18/91-99      */
{  3, 6, 480, 30 }  /* 18/100   (30) */
};

/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[26] = {
{-99,-99,-60},   /* 0 */
{-90,-90,-50},   /* 1 */
{-80,-80,-45},
{-70,-70,-40},
{-60,-60,-35},
{-50,-50,-30},   /* 5 */
{-40,-40,-25},
{-30,-30,-20},
{-20,-20,-15},
{-15,-20,-10},
{-10,-15, -5},   /* 10 */
{ -5,-10,  0},
{  0, -5,  0},
{  0,  0,  0},
{  0,  0,  0},
{  0,  0,  0},   /* 15 */
{  0,  0,  0},
{  5,  5,  5},
{ 10, 10, 10},
{ 15, 15, 15},
{ 15, 15, 15},   /* 20 */
{ 20, 15, 20},
{ 20, 20, 20},
{ 25, 20, 20},
{ 25, 25, 25},
{ 25, 25, 25}    /* 25 */
};

/* [level] backstab multiplyer (thieves only) */
const byte backstab_mult[58] =
{
  1,   /* 0 */
  2,   /* 1 */
  2,
  2,
  2,
  2,   /* 5 */
  2,
  2,
  3,
  3,
  3,   /* 10 */
  3,
  3,
  3,
  4,
  4,   /* 15 */
  4,
  4,
  4,
  4,
  5,   /* 20 */
  5,
  5,
  5,
  6,
  6,   /* 25 */
  6,
  7,
  7,
  7,
  8,   /* 30 */
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  9,   /* 40 */
  9,
  9,
  9,
  9,
  9,
  9,
  9,
  9,
  9,
  10, /* 50 */
  10,
  10,
  10,
  10,
  10,
  10,
  10  /* 57 */
};

/* [level] ambush multiplyer (nomads only) */
const byte ambush_mult[58] =
{
  1,   /* 0 */
  2,   /* 1 */
  2,
  2,
  2,
  2,   /* 5 */
  2,
  2,
  3,
  3,
  3,   /* 10 */
  3,
  3,
  3,
  3,
  4,   /* 15 */
  4,
  4,
  4,
  4,
  4,   /* 20 */
  4,
  4,
  4,
  5,
  5,   /* 25 */
  5,
  5,
  5,
  5,
  6,   /* 30 */
  6,
  6,
  6,
  6,
  6,   /* 35 */
  6,
  6,
  6,
  6,
  6,   /* 40 */
  6,
  6,
  6,
  6,
  7,   /* 45 */
  7,
  7,
  7,
  7,
  8,   /* 50 */
  8,
  8,
  8,
  8,
  8,
  8,
  8    /* 57 */
};

/* [level] assault multiplyer */
const byte assault_mult[58] = {
  1,   /* 0 */
  2,   /* 1 */
  2,
  2,
  2,
  2,   /* 5 */
  2,
  2,
  3,
  3,
  3,   /* 10 */
  3,
  3,
  3,
  3,
  3,   /* 15 */
  4,
  4,
  4,
  4,
  4,   /* 20 */
  4,
  4,
  4,
  4,
  5,   /* 25 */
  5,
  5,
  5,
  5,
  5,   /* 30 */
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  6,   /* 40 */
  6,
  6,
  6,
  6,
  6,
  6,
  6,
  6,
  6,
  7, /* 50*/
  7,
  7,
  7,
  7,
  7,
  7,
  7    /* 57 */
};

const byte circle_mult[58] = {
  1,   /* 0 */
  2,   /* 1 */
  2,
  2,
  2,
  2,   /* 5 */
  2,
  2,
  2,
  2,
  2,   /* 10 */
  2,
  2,
  2,
  2,
  3,   /* 15 */
  3,
  3,
  3,
  3,
  4,   /* 20 */
  4,
  4,
  4,
  4,
  4,   /* 25 */
  4,
  4,
  4,
  4,
  4,   /* 30 */
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  5,   /* 40 */
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5, /* 50 */
  7,
  7,
  7,
  7,
  7,
  7,
  7    /* 57 */
};

/* [dex] apply (all) */
/* defensive, prac_bonus */
struct dex_app_type dex_app[26] = {
  { 60, 0},   /* 0 */
  { 50, 0},   /* 1 */
  { 50, 0},
  { 40, 0},
  { 30, 0},
  { 20, 0},   /* 5 */
  { 10, 0},
  {  5, 0},
  {  0, 0},
  {  0, 0},
  {  0, 0},   /* 10 */
  {  0, 0},
  {  0, 0},
  { -2, 0},
  { -4, 0},
  { -6, 0},   /* 15 */
  { -9, 0},
  {-12, 0},
  {-15, 0},   /* 18 */
  {-18, 1},
  {-21, 2},   /* 20 */
  {-24, 3},
  {-28, 4},
  {-32, 6},
  {-36, 8},
  {-40,10}    /* 25 */
};

/* [con] apply (all) */
/* hitp, regen, reduct */
struct con_app_type con_app[26] = {
	{-4,-3,0},   /* 0 */
	{-3,-2,0},   /* 1 */
	{-2,-2,0},
	{-2,-2,0},
	{-1,-1,0},
	{-1,-1,0},   /* 5 */
	{-1,-1,0},
	{ 0,-1,0},
	{ 0, 0,0},
	{ 0, 0,0},
	{ 0, 0,0},   /* 10 */
	{ 0, 0,0},
	{ 0, 1,0},
	{ 0, 2,0},
	{ 0, 3,0},
	{ 1, 4,0},   /* 15 */
	{ 1, 5,0},
	{ 2, 6,0},
	{ 3, 7,0},
	{ 3, 8,2}, /* 2% reduction before sanct */
	{ 4, 9,4},   /* 20 */
	{ 5, 12,6},
	{ 5, 15,8},
	{ 5, 19,12},
	{ 6, 22,16},
	{ 7, 25,20}   /* 25 */ /* 20% reduction before sanct */
};

/* [int] apply (all) */
/* learn, conc */
struct int_app_type int_app[26] = {
	{3 ,-20}, /* 0 */
	{5 ,-15}, /* 1 */
	{7 ,-15},
	{8 ,-15},
	{9 ,-15},
	{10,-10}, /* 5 */
	{11,-10},
	{12,-10},
	{13,-10},
	{15,-10},
	{17, -5}, /* 10 */
	{19, -5},
	{22, -5},
	{25, -5},
	{30, -2},
	{35,  0}, /* 15 */
	{40,  5},
	{45, 10},
	{50, 15}, /* 18 - 1.5% conc */
	{53, 17},
	{55, 20}, /* 20 - 2% conc */
	{56, 22},
	{58, 24},
	{60, 26},
	{63, 28},
	{65, 30}  /* 30 - 3% conc */
};

/* [wis] apply (all) */
/* bonus, conc */
struct wis_app_type wis_app[26] = {
	{0,-20}, /* 0 */
	{0,-15}, /* 1 */
	{0,-15},
	{0,-15},
	{0,-15},
	{0,-10}, /* 5 */
	{0,-10},
	{0,-10},
	{0,-10},
	{0,-10},
	{0, -5}, /* 10 */
	{0, -5},
	{2, -5},
	{2, -5},
	{3, -2},
	{3,  0}, /* 15 */
	{3,  5},
	{4, 10},
	{5, 15}, /* 18 - 1.5% conc */
	{6, 17},
	{6, 20}, /* 20 - 2% conc */
	{6, 22},
	{6, 24},
	{6, 26},
	{6, 28},
	{6, 30}  /* 25 - 3% conc */
};

char *w_skills[] = {
  "kick",  /* No. 50 */
  "bash",
  "rescue",
  "block",
  "knock",
  "punch",
  "parry",
  "dual",
  "throw",
  "triple",
  "spin",
  "protect",
  "switch",
  "awareness",
  "flank",
  "defend",
  "hostile",
  "charge",
  "headbutt",
  "quad",
  "\n",
};

char *t_skills[] = {
  "sneak",   /* No. 45 */
  "hide",
  "steal",
  "backstab",
  "pick",
  "throw",
  "dodge",
  "peek",
  "circle",
  "trophy", /* Used to be Retreat */
  "scan", /* Used to be Evade */
  "trip",
  "evasion", /* Used to be Scan*/
  "fade",
  "dirty tricks",
  "impair",
  "close combat",
  "vehemence", /* Used to be Execute */
  "twist",
  "cunning",
  "\n"
};

char *n_skills[] = {
  "sneak",
  "hide",
  "throw",
  "dodge",
  "assault",
  "kick",
  "bash",
  "pummel",
  "backfist",
  "tigerkick",
  "banzai",
  "mantra",
  "spin",
  "\n"
};

char *nomad_skills[] = {
  "pick",
  "throw",
  "dodge",
  "peek",
  "butcher",
  "trap",
  "disarm",
  "subdue",
  "kick",
  "ambush",
  "spin",
  "rescue",
  "disembowel",
  "protect",
  "scan",
  "awareness",
  "camp",
  "sweep",
  "defend",
  "batter",
  "berserk",
  "frenzy",
  "trophy", /* Used to be Scalp */
  "evasion", /* Used to be Cover */
  "\n"
};

char *pal_skills[] = {
  "kick",
  "bash",
  "rescue",
  "block",
  "disarm",
  "spin",
  "pummel",
  "parry",
  "meditate",
  "pray",
  "switch",
  "protect",
  "\n"
};

char *cl_skills[] = {
  "meditate",
  "bash",
  "\n"
};

char *antipal_skills[] = {
  "sneak",
  "hide",
  "steal",
  "backstab",
  "pick",
  "kick",
  "bash",
  "peek",
  "pummel",
  "feint",
  "assassinate",
  "shadow-walk",
  "victimize",
  "\n"
};

char *avatar_skills[] = {
  "steal",
  "backstab",
  "pick",
  "rescue",
  "block",
  "punch",
  "parry",
  "dual",
  "throw",
  "peek",
  "trap",
  "disarm",
  "subdue",
  "circle",
  "triple",
  "pummel",
  "quad",
  "\n"
};

char *bard_skills[] = {
  "pick",
  "throw",
  "dodge",
  "peek",
  "backflip",
  "blitz",
  "\n"
};

char *commando_skills[] = {
  "kick",
  "bash",
  "dual",
  "throw",
  "disarm",
  "triple",
  "assault",
  "pummel",
  "rescue",
  "lunge",
  "clobber",
  "riposte",
  "\n"
};

char *subclass_name[] = {
	"ENCHANTER",
	"ARCHMAGE",
	"DRUID",
	"TEMPLAR",
	"ROGUE",
	"BANDIT",
	"WARLORD",
	"GLADIATOR",
	"RONIN",
	"MYSTIC",
	"RANGER",
	"TRAPPER",
	"CAVALIER",
	"CRUSADER",
	"DEFILER",
	"INFIDEL",
	"",/* Avatar*/
	"",
	"BLADESINGER",
	"CHANTER",
	"LEGIONNAIRE",
	"MERCENARY",
	"\n"
};

/* #define MOB_ID_HP_ARRAY_MAX 10 - in magic.c */
const int mob_id_hp_int[] =
{
	0,100,250,500,1000,3000,7000,15000,35000,75000
};

const char *mob_id_hp_text[] =
{
	"microscopic",
	"miniscule",
	"puny",
	"tiny",
	"small",
	"sizable",
	"hefty",
	"large",
	"enormous",
	"gargantuan"
};

/* #define MOB_ID_DMG_ARRAY_MAX 9 - in magic.c */
const int mob_id_dmg_int[] =
{
	0,15,30,60,100,200,500,1000,1800
};

const char *mob_id_dmg_text[] =
{
	"friendly",
	"gentle",
	"feeble",
	"weak",
	"strong",
	"powerful",
	"destructive",
	"calamitous",
	"slaughterous"
};

/* #define MOB_ID_LEVEL_ARRAY_MAX 6 - in magic.c */
const int mob_id_level_int[] =
{
	0,15,32,42,53,65
};

const char *mob_id_level_text[] =
{
	"newbish",
	"sophomoric",
	"inexperienced",
	"experienced",
	"wise",
	"godly"
};

/* #define MOB_ID_MATT_ARRAY_MAX 4 - in magic.c */
const int mob_id_matt_int[] =
{
	0,1,3,5
};

const char *mob_id_matt_text[] =
{
	"",
	"crafty, ",
	"shrewd, ",
	"diabolical, "
};

/* #define MOB_ID_SPEC_ARRAY_MAX 2 - in magic.c */
const char *mob_id_spec[] =
{
	"",
	"extraordinary, "
};
