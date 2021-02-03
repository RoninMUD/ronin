/**************************************************************************
*  file: structs.h , Structures                           Part of DIKUMUD *
*  Usage: Declarations of central data structures                         *
***************************************************************************/

#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#pragma pack (push, 4)

/* Typedefs */
typedef int8_t bool;
typedef int8_t byte;
typedef int8_t sbyte;
typedef uint8_t ubyte;
typedef int16_t sh_int;
typedef uint16_t ush_int;

typedef struct descriptor_data    DESC;
typedef struct follow_type        FOL;
typedef struct char_data          CHAR;
typedef struct obj_data           OBJ;
typedef struct room_data          RM;
typedef struct affected_type_5    AFF;
typedef struct enchantment_t      ENCH;
typedef struct social_messg       SOC;

typedef int (*ENCH_FUNC)(ENCH *enchantment, struct char_data *ch, struct char_data *other, int cmd, char *arg);


/* Time Data */

#define OPT_USEC              100000
#define PASSES_PER_SEC        (1000000 / OPT_USEC)
#define RL_SEC                PASSES_PER_SEC

#define PULSE_ZONE            (60 * RL_SEC)
#define PULSE_TICK            (60 * RL_SEC)
#define PULSE_MOBILE          (10 * RL_SEC)
#define PULSE_VIOLENCE        (3 * RL_SEC)

#define SECS_PER_REAL_MIN     60
#define SECS_PER_REAL_HOUR    (60 * SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY     (24 * SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR    (365 * SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR     (60 * 4)
#define SECS_PER_MUD_DAY      (24 * SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH    (28 * SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR     (17 * SECS_PER_MUD_MONTH)


/* Various Game Limits */

#define MAX_STRING_LENGTH     4096
#define MSL                   MAX_STRING_LENGTH
#define MAX_INPUT_LENGTH      255
#define MIL                   MAX_INPUT_LENGTH

#define MAX_ATTS              10 /* Mob Attacks */

#define MAX_MESSAGES          60 /* Damage Messages */

#define MAX_DNAME_LENGTH      20 /* Max Player Name Length */

#define MAX_USERID            50 /* Used in get_slave_result() */

#define MAX_COLORS            16 /* Max Player Colors */


/* Misc. Special Data */

/* Breakthrough Flags */
#define BT_INVUL              1
#define BT_SPHERE             2


/* Character Data */

/* Mortal and Immortal Levels */
#define LEVEL_MORT            50
#define LEVEL_IMM             51
#define LEVEL_DEI             52
#define LEVEL_TEM             53
#define LEVEL_WIZ             54
#define LEVEL_ETE             55
#define LEVEL_SUP             56
#define LEVEL_IMP             57
#define LEVEL_MAX             70

/* Prestige Data */
#define PRESTIGE_MAX          255
#define PRESTIGE_HIT_GAIN     8
#define PRESTIGE_MANA_GAIN    4

/* Quest Status */
#define QUEST_NONE            0
#define QUEST_RUNNING         1
#define QUEST_COMPLETED       2
#define QUEST_FAILED          3


/* Object Data */

/* Special Object VNUMs */
#define WALL_THORNS           34
#define ICE_WALL              27750
#define ICE_BLOCK             27751

/* Subclass Token and Token Mob */
#define TOKEN_OBJ_VNUM        5
#define TOKEN_MOB_VNUM        11

/* Corpse and Statue Timer Data */
#define MAX_NPC_CORPSE_TIME   10
#define MAX_PC_CORPSE_TIME    60
#define MAX_NPC_STATUE_TIME   30
#define MAX_PC_STATUE_TIME    60
#define MAX_CHAOS_CORPSE_TIME 20

/* Object Bits */

/* Object type numbers -- used in 'type_flag' */
#define ITEM_LIGHT            1
#define ITEM_SCROLL           2
#define ITEM_WAND             3
#define ITEM_STAFF            4
#define ITEM_WEAPON           5
#define ITEM_FIREARM          6
#define ITEM_MISSILE          7
#define ITEM_TREASURE         8
#define ITEM_ARMOR            9
#define ITEM_POTION           10
#define ITEM_WORN             11
#define ITEM_OTHER            12
#define ITEM_TRASH            13
#define ITEM_TRAP             14
#define ITEM_CONTAINER        15
#define ITEM_NOTE             16
#define ITEM_DRINKCON         17
#define ITEM_KEY              18
#define ITEM_FOOD             19
#define ITEM_MONEY            20
#define ITEM_PEN              21
#define ITEM_BOAT             22
#define ITEM_BULLET           23
#define ITEM_MUSICAL          24
#define ITEM_LOCKPICK         25
#define ITEM_2H_WEAPON        26
#define ITEM_BOARD            27
#define ITEM_TICKET           28
#define ITEM_SC_TOKEN         29
#define ITEM_SKIN             30
#define ITEM_TROPHY           31
#define ITEM_RECIPE           32
#define ITEM_TYPE_33          33
#define ITEM_TYPE_34          34
#define ITEM_TYPE_35          35
#define ITEM_AQ_ORDER         36

/* Object bit flags-- used in 'wear_flags' */
#define ITEM_TAKE             (1 << 0)  /* 1 */
#define ITEM_WEAR_FINGER      (1 << 1)  /* 2 */
#define ITEM_WEAR_NECK        (1 << 2)  /* 4 */
#define ITEM_WEAR_BODY        (1 << 3)  /* 8 */
#define ITEM_WEAR_HEAD        (1 << 4)  /* 16 */
#define ITEM_WEAR_LEGS        (1 << 5)  /* 32 */
#define ITEM_WEAR_FEET        (1 << 6)  /* 64 */
#define ITEM_WEAR_HANDS       (1 << 7)  /* 128 */
#define ITEM_WEAR_ARMS        (1 << 8)  /* 256 */
#define ITEM_WEAR_SHIELD      (1 << 9)  /* 512 */
#define ITEM_WEAR_ABOUT       (1 << 10) /* 1024 */
#define ITEM_WEAR_WAIST       (1 << 11) /* 2048 */
#define ITEM_WEAR_WRIST       (1 << 12) /* 4096 */
#define ITEM_WIELD            (1 << 13) /* 8192 */
#define ITEM_HOLD             (1 << 14) /* 16384 */
#define ITEM_THROW            (1 << 15) /* 32768 */
#define ITEM_LIGHT_SOURCE     (1 << 16) /* 65536 */
#define ITEM_NO_REMOVE        (1 << 17) /* 131072 */
#define ITEM_NO_SCAVENGE      (1 << 18) /* 262144 */
#define ITEM_QUESTWEAR        (1 << 19) /* 524288 */
#define ITEM_WEAR_2NECK       (1 << 20) /* 1048576 */

/* Object bit flags -- used in 'extra_flags' */
#define ITEM_GLOW             (1 << 0)  /* 1 */
#define ITEM_HUM              (1 << 1)  /* 2 */
#define ITEM_DARK             (1 << 2)  /* 4 */
#define ITEM_CLONE            (1 << 3)  /* 8 */
#define ITEM_EVIL             (1 << 4)  /* 16 */
#define ITEM_INVISIBLE        (1 << 5)  /* 32 */
#define ITEM_MAGIC            (1 << 6)  /* 64 */
#define ITEM_NO_DROP          (1 << 7)  /* 128 */
#define ITEM_BLESS            (1 << 8)  /* 256 */
#define ITEM_ANTI_GOOD        (1 << 9)  /* 512 */
#define ITEM_ANTI_EVIL        (1 << 10) /* 1024 */
#define ITEM_ANTI_NEUTRAL     (1 << 11) /* 2048 */
#define ITEM_ANTI_WARRIOR     (1 << 12) /* 4096 */
#define ITEM_ANTI_THIEF       (1 << 13) /* 8192 */
#define ITEM_ANTI_CLERIC      (1 << 14) /* 16384 */
#define ITEM_ANTI_MAGIC_USER  (1 << 15) /* 32768 */
#define ITEM_ANTI_MORTAL      (1 << 16) /* 65536 */
#define ITEM_NO_DISINTEGRATE  (1 << 17) /* 131072 */
#define ITEM_DISPELLED        (1 << 18) /* 262144 */
#define ITEM_ANTI_RENT        (1 << 19) /* 524288 */
#define ITEM_ANTI_NINJA       (1 << 20) /* 1048576 */
#define ITEM_ANTI_NOMAD       (1 << 21) /* 2097152 */
#define ITEM_ANTI_PALADIN     (1 << 22) /* 4194304 */
#define ITEM_ANTI_ANTIPALADIN (1 << 23) /* 8388608 */
#define ITEM_ANTI_AVATAR      (1 << 24) /* 16777216 */
#define ITEM_ANTI_BARD        (1 << 25) /* 33554432 */
#define ITEM_ANTI_COMMANDO    (1 << 26) /* 67108864 */
#define ITEM_LIMITED          (1 << 27) /* 134217728 */
#define ITEM_ANTI_AUCTION     (1 << 28) /* 268435456 */
#define ITEM_CHAOTIC          (1 << 29) /* 536870912 */

/* Object bit flags -- used in 'extra_flags2' */
#define ITEM_RANDOM           (1 << 0)  /* 1 */
#define ITEM_ALL_DECAY        (1 << 1)  /* 2 */
#define ITEM_EQ_DECAY         (1 << 2)  /* 4 */
#define ITEM_NO_GIVE          (1 << 3)  /* 8 */
#define ITEM_NO_GIVE_MOB      (1 << 4)  /* 16 */
#define ITEM_NO_PUT           (1 << 5)  /* 32 */
#define ITEM_NO_TAKE_MOB      (1 << 6)  /* 64 */
#define ITEM_EXTRA2_128       (1 << 7)  /* 128 */
#define ITEM_NO_LOCATE        (1 << 8)  /* 256 */
#define ITEM_RANDOM_AFF0      (1 << 9)  /* 512 */
#define ITEM_RANDOM_AFF1      (1 << 10) /* 1024 */
#define ITEM_RANDOM_AFF2      (1 << 11) /* 2048 */

/* Object bit flags -- used in 'subclass_res' */
#define ITEM_ANTI_SC_ENCHANTER   (1 << 0)  /* 1 */
#define ITEM_ANTI_SC_ARCHMAGE    (1 << 1)  /* 2 */
#define ITEM_ANTI_SC_DRUID       (1 << 2)  /* 4 */
#define ITEM_ANTI_SC_TEMPLAR     (1 << 3)  /* 8 */
#define ITEM_ANTI_SC_ROGUE       (1 << 4)  /* 16 */
#define ITEM_ANTI_SC_BANDIT      (1 << 5)  /* 32 */
#define ITEM_ANTI_SC_WARLORD     (1 << 6)  /* 64 */
#define ITEM_ANTI_SC_GLADIATOR   (1 << 7)  /* 128 */
#define ITEM_ANTI_SC_RONIN       (1 << 8)  /* 256 */
#define ITEM_ANTI_SC_MYSTIC      (1 << 9)  /* 512 */
#define ITEM_ANTI_SC_RANGER      (1 << 10) /* 1024 */
#define ITEM_ANTI_SC_TRAPPER     (1 << 11) /* 2048 */
#define ITEM_ANTI_SC_CAVALIER    (1 << 12) /* 4096 */
#define ITEM_ANTI_SC_CRUSADER    (1 << 13) /* 8192 */
#define ITEM_ANTI_SC_DEFILER     (1 << 14) /* 16384 */
#define ITEM_ANTI_SC_INFIDEL     (1 << 15) /* 32768 */
#define ITEM_ANTI_SC_BLADESINGER (1 << 16) /* 65536 */
#define ITEM_ANTI_SC_CHANTER     (1 << 17) /* 131072 */
#define ITEM_ANTI_SC_LEGIONNAIRE (1 << 18) /* 262144 */
#define ITEM_ANTI_SC_MERCENARY   (1 << 19) /* 524288 */

/* Corpse and Statue flags -- used in 'cost' */
#define PC_CORPSE             1
#define NPC_CORPSE            2
#define PC_STATUE             3
#define NPC_STATUE            4
#define CHAOS_CORPSE          5

/* Container flags -- used in value[1] */
#define CONT_CLOSEABLE        1
#define CONT_PICKPROOF        2
#define CONT_CLOSED           4
#define CONT_LOCKED           8
#define CONT_NO_REMOVE        16

/* Liquid Types */
#define LIQ_WATER             0
#define LIQ_BEER              1
#define LIQ_WINE              2
#define LIQ_ALE               3
#define LIQ_DARKALE           4
#define LIQ_WHISKY            5
#define LIQ_LEMONADE          6
#define LIQ_FIREBREATHER      7
#define LIQ_LOCAL             8
#define LIQ_SLIME             9
#define LIQ_MILK              10
#define LIQ_TEA               11
#define LIQ_COFFE             12
#define LIQ_BLOOD             13
#define LIQ_SALTWATER         14
#define LIQ_COKE              15
#define LIQ_STOUT             16
#define LIQ_VODKA             17
#define LIQ_RUM               18
#define LIQ_LIQUOR            19
#define LIQ_CHAMPAGNE         20
#define LIQ_BOURBON           21
#define LIQ_TEQUILA           22
#define LIQ_CIDER             23
#define LIQ_URINE             24
#define LIQ_GIN               25
#define LIQ_MERLOT            26
#define LIQ_SCHNAPPS          27
#define LIQ_MOONSHINE         28
#define LIQ_PUS               29
#define LIQ_SHERBET           30
#define LIQ_COGNAC            31
#define LIQ_BRANDY            32
#define LIQ_SCOTCH            33
#define LIQ_KEFIR             34
#define LIQ_OUZO              35
#define LIQ_SAKI              36
#define LIQ_LAGER             37


/* Room Data */

/* Reserved Room VNUMs */
#define ROOM_VOID                0
#define ROOM_LIMBO               1
#define ROOM_PRISON              10
#define ROOM_CHAT_ROOM           1200
#define ROOM_LOST_TREASURE_TROVE 1201
#define ROOM_QUESTERS_FORUM      1204
#define ROOM_TEMPLE_OF_MIDGAARD  3001
#define ROOM_MARKET_SQUARE       3014
#define ROOM_SCRAPYARD           3030
#define ROOM_AUCTION_HALL        3072
#define ROOM_MORGUE              3088
#define ROOM_DONATION            3084


struct string_block {
  int size;
  char *data;
};

struct extra_descr_data
{
  char *keyword;                 /* Keyword in look/examine          */
  char *description;             /* What to see                      */
  struct extra_descr_data *next; /* Next in list                     */
};

struct tagline_data
{
  char *desc;
  struct tagline_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT       3         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OFILE_MAX_OBJ_AFFECT 2         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define MAX_OBJ_VALUE        4
#define MAX_OBJ_SKIN         6

struct obj_flag_data
{
  int value[MAX_OBJ_VALUE];       /* Values of the item (see list)    */
  int skin_vnum[MAX_OBJ_SKIN];   /* skin loading items - Ranger Feb 2001 */
  ubyte type_flag;    /* Type of item                     */
  int wear_flags;     /* Where you can wear it            */
  int extra_flags;    /* If it hums,glows etc             */
  int weight;         /* Weight what else                  */
  int cost;           /* Value when sold (gp.)            */
  int cost_per_day;   /* Cost to keep pr. real day        */
  int timer;          /* Timer for object                 */
  long bitvector;     /* To set chars bits                */
  long bitvector2;    /* To set chars bits                */
  ubyte repop_percent;
  int extra_flags2;   /* More extras */
  int subclass_res;   /* Subclass restrictions */
  int material; /* Made of */
  int popped;         /* Date stamp when object popped    */
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type_ver0 {
  ubyte location;      /* Which ability to change (APPLY_XXX) */
  sbyte modifier;     /* How much it changes by              */
};

/* Newest obj_affected_type used in Obj_file_elem do not change */
struct obj_affected_type {
  ush_int location;      /* Which ability to change (APPLY_XXX) */
  sh_int modifier;     /* How much it changes by              */
};

#define MAX_OBJ_OWNER_ID 8

/* ======================== Structure for object ========================= */
struct obj_data
{
  sh_int item_number;            /* Where in data-base               */
  sh_int item_number_v;          /* Where in data-base               */
  sh_int in_room;                /* In what room -1 when conta/carr  */
  sh_int in_room_v;              /* In what room -1 when conta/carr  */
  byte log;                      /* To see if/how the item needs to be logged */
  int spec_value;                /* tmp value for spec progs         */
  int ownerid[MAX_OBJ_OWNER_ID]; /* Id of owners */
  struct obj_flag_data obj_flags;/* Object information               */
  struct obj_affected_type
      affected[MAX_OBJ_AFFECT];  /* Which abilities in PC to change  */

  char *name;                    /* Title of object :get etc.        */
  char *description ;            /* When in room                     */
  char *short_description;       /* when worn/carry/in cont.         */
  char *action_description;      /* What to write when used          */
  char *action_description_nt;   /* What to write when used - no target */
  char *char_wear_desc;          /* What to write when worn */
  char *room_wear_desc;          /* What to write when worn */
  char *char_rem_desc;           /* What to write when removed */
  char *room_rem_desc;           /* What to write when removed */
  struct extra_descr_data *ex_description; /* extra descriptions     */
  struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
  struct char_data *equipped_by;
  struct char_data *owned_by;    /* For special things like quest objs,
                                    or items that can return to owner. */
  struct obj_data *in_obj;       /* In what object NULL when none    */
  struct obj_data *contains;     /* Contains objects                 */

  struct obj_data *next_content; /* For 'contains' lists             */
  struct obj_data *next;         /* For the object list              */
  int (*func)(struct obj_data*, struct char_data*, int, char*);
};
/* ======================================================================= */

/* The following defs are for room_data  */
#define NOWHERE    -1    /* nil reference for room-database    */

/* Bitvector for 'room_flags' */
#define DARK        1
#define DEATH       2
#define NO_MOB      4
#define INDOORS     8
#define LAWFUL      16
#define CHAOTIC     32
#define SAFE        64
#define NO_MAGIC    128
#define TUNNEL      256
#define PRIVATE     512
#define LOCK        1024
#define TRAP        2048
#define ARENA       4096
#define CLUB        8192
#define QUIET       16384
#define NO_BEAM     32768
#define HAZARD      65536
#define MOVE_TRAP   131072
#define FLYING      262144
#define NO_PEEK     524288
#define NO_SONG     1048576
#define NO_REGEN    2097152
#define NO_QUAFF    4194304
#define REV_REGEN   8388608
#define DOUBLE_MANA 16777216
#define HALF_CONC   33554432
#define LIT         67108864
#define NO_ORB      134217728
#define QRTR_CONC   268435456
#define MANA_DRAIN  536870912
#define NO_SUM      1073741824
#define ROOM_FINAL  2147483648u

/* MANABURN - see commented in spell_parser.c */

/* Directions for 'dir_option' */
#define NORTH 0
#define EAST  1
#define SOUTH 2
#define WEST  3
#define UP    4
#define DOWN  5

/* Exit Flags for 'exit_info' */
#define EX_ISDOOR    1
#define EX_CLOSED    2
#define EX_LOCKED    4
#define EX_PICKPROOF 8
#define EX_LOCK_10   16
#define EX_LOCK_15   32
#define EX_LOCK_20   64
#define EX_LOCK_25   128
#define EX_LOCK_30   256
#define EX_CLIMB     512
#define EX_JUMP      1024
#define EX_MOVE      2048
#define EX_CRAWL     4096
#define EX_ENTER     8192

/* Sector Types for 'sector_type' */
#define SECT_INSIDE       0
#define SECT_CITY         1
#define SECT_FIELD        2
#define SECT_FOREST       3
#define SECT_HILLS        4
#define SECT_MOUNTAIN     5
#define SECT_WATER_SWIM   6
#define SECT_WATER_NOSWIM 7
#define SECT_DESERT       8
#define SECT_ARCTIC       9
#define SECT_FIRST        SECT_INSIDE
#define SECT_LAST         SECT_ARCTIC

struct room_direction_data
{
  char *general_description;       /* When look DIR.                  */
  char *keyword;                   /* for open/close                  */
  sh_int exit_info;                /* Exit info                       */
  sh_int key;                       /* Key's number (-1 for no key)    */
  sh_int to_room_v;                  /* Where direction leeds (NOWHERE) */
  sh_int to_room_r;                  /* Where direction leeds (NOWHERE) */
};

/* ========================= Structure for room ========================== */
struct room_data
{
  sh_int number;               /* Rooms number                       */
  sh_int zone;                 /* Room zone (for resetting)          */
  int    spec_tmp;
  int sector_type;             /* sector type (move/hide)            */
  char *name;                  /* Rooms name 'You are ...'           */
  char *description;           /* Shown when entered                 */
  struct extra_descr_data *ex_description; /* for examine/look       */
  struct room_direction_data *dir_option[6]; /* Directions           */
  unsigned long room_flags;              /* DEATH,DARK ... etc                 */
  ubyte light;                  /* Number of lightsources in room     */
  byte blood;
  int (*funct)(int, struct char_data*, int, char*);              /* special procedure                  */

  struct obj_data *contents;   /* List of items in room              */
  struct char_data *people;    /* List of NPC / PC in room           */
};
/* ======================================================================== */

/* The following defs and structures are related to char_data   */

/* For 'equipment' */

/* If adding a position, check char_to_store in reception.c - Ranger Aug 96 */
#define WEAR_LIGHT    0
#define WEAR_FINGER_R 1
#define WEAR_FINGER_L 2
#define WEAR_NECK_1   3
#define WEAR_NECK_2   4
#define WEAR_BODY     5
#define WEAR_HEAD     6
#define WEAR_LEGS     7
#define WEAR_FEET     8
#define WEAR_HANDS    9
#define WEAR_ARMS     10
#define WEAR_SHIELD   11
#define WEAR_ABOUT    12
#define WEAR_WAIST    13
#define WEAR_WRIST_R  14
#define WEAR_WRIST_L  15
#define WIELD         16
#define WEAR_WIELD    WIELD
#define HOLD          17
#define WEAR_HOLD     HOLD

/* For 'char_payer_data' */
#define MAX_TONGUE  3   /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_SKILLS4 255 /* All pfile versions up to 4 */
#define MAX_SKILLS5 500 /* pfile version 5 */
#define MAX_WEAR    18
#define MAX_AFFECT  40  /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_COND    4

/* Predifined conditions */
#define DRUNK  0
#define FULL   1
#define THIRST 2
#define QUAFF  3

/* Bitvector for 'affected_by' */
/* Don't forget to update do_affect if you add something here. */
#define AFF_NONE             0
#define AFF_BLIND            1
#define AFF_INVISIBLE        2
#define AFF_DETECT_ALIGNMENT 4
#define AFF_DETECT_INVISIBLE 8
#define AFF_DETECT_MAGIC     16
#define AFF_SENSE_LIFE       32
#define AFF_HOLD             64
#define AFF_SANCTUARY        128
#define AFF_GROUP            256
#define AFF_DETECT_POISON    512
#define AFF_CURSE            1024
#define AFF_SPHERE           2048
#define AFF_POISON           4096
#define AFF_PROTECT_EVIL     8192
#define AFF_PARALYSIS        16384
#define AFF_INFRAVISION      32768
#define AFF_STATUE           65536
#define AFF_SLEEP            131072
#define AFF_DODGE            262144
#define AFF_SNEAK            524288
#define AFF_HIDE             1048576
#define AFF_ANIMATE          2097152
#define AFF_CHARM            4194304
#define AFF_PROTECT_GOOD     8388608
#define AFF_FLY              16777216
#define AFF_SUBDUE           33554432
#define AFF_IMINV            67108864
#define AFF_INVUL            134217728
#define AFF_DUAL             268435456
#define AFF_FURY             536870912
#define AFF_FINAL            1073741824

/* Bitvector for 'affected_by2' */
/* Don't forget to update do_affect if you add something here. */
#define AFF2_TRIPLE           1
#define AFF2_IMMINENT_DEATH   2
#define AFF2_SEVERED          4
#define AFF2_QUAD             8
#define AFF2_FORTIFICATION   16
#define AFF2_PERCEIVE        32
#define AFF2_RAGE            64
#define AFF2_FINAL           128
/* WIP - Night
#define AFF2_RIPOSTE         128
#define AFF2_FEINT           256
#define AFF2_FINAL           512
*/

/* modifiers to char's abilities */
#define APPLY_NONE             0
#define APPLY_STR              1
#define APPLY_DEX              2
#define APPLY_INT              3
#define APPLY_WIS              4
#define APPLY_CON              5
#define APPLY_6                6
#define APPLY_7                7
#define APPLY_8                8
#define APPLY_AGE              9
#define APPLY_10               10
#define APPLY_11               11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_SAVING_ALL       15
#define APPLY_16               16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24
#define APPLY_SKILL_SNEAK      25
#define APPLY_SKILL_HIDE       26
#define APPLY_SKILL_STEAL      27
#define APPLY_SKILL_BACKSTAB   28
#define APPLY_SKILL_PICK_LOCK  29
#define APPLY_SKILL_KICK       30
#define APPLY_SKILL_BASH       31
#define APPLY_SKILL_RESCUE     32
#define APPLY_SKILL_BLOCK      33
#define APPLY_SKILL_KNOCK      34
#define APPLY_SKILL_PUNCH      35
#define APPLY_SKILL_PARRY      36
#define APPLY_SKILL_DUAL       37
#define APPLY_SKILL_THROW      38
#define APPLY_SKILL_DODGE      39
#define APPLY_SKILL_PEEK       40
#define APPLY_SKILL_BUTCHER    41
#define APPLY_SKILL_TRAP       42
#define APPLY_SKILL_DISARM     43
#define APPLY_SKILL_SUBDUE     44
#define APPLY_SKILL_CIRCLE     45
#define APPLY_SKILL_TRIPLE     46
#define APPLY_SKILL_PUMMEL     47
#define APPLY_SKILL_AMBUSH     48
#define APPLY_SKILL_ASSAULT    49
#define APPLY_SKILL_DISEMBOWEL 50
#define APPLY_SKILL_TAUNT      51
#define APPLY_HP_REGEN         52
#define APPLY_MANA_REGEN       53
#define APPLY_DMG_BONUS_PCT    54

/* subclasses - Ranger March 98 */
#define SC_NONE        0
#define SC_ENCHANTER   1  /* mage */
#define SC_ARCHMAGE    2
#define SC_DRUID       3  /* cleric */
#define SC_TEMPLAR     4
#define SC_ROGUE       5  /* thief */
#define SC_BANDIT      6
#define SC_WARLORD     7  /* warrior */
#define SC_GLADIATOR   8
#define SC_RONIN       9  /* ninja */
#define SC_MYSTIC      10
#define SC_RANGER      11 /* nomad */
#define SC_TRAPPER     12
#define SC_CAVALIER    13 /* paladin */
#define SC_CRUSADER    14
#define SC_DEFILER     15 /* anti-paladin */
#define SC_INFIDEL     16
#define SC_AVATAR_1    17 /* avatar */
#define SC_AVATAR_2    18
#define SC_BLADESINGER 19 /* bard */
#define SC_CHANTER     20
#define SC_LEGIONNAIRE 21 /* commando */
#define SC_MERCENARY   22
#define SC_FIRST       SC_ENCHANTER
#define SC_LAST        SC_MERCENARY

/* 'class' for PC's */
#define CLASS_NONE         0
#define CLASS_MAGIC_USER   1
#define CLASS_CLERIC       2
#define CLASS_THIEF        3
#define CLASS_WARRIOR      4
#define CLASS_NINJA        5
#define CLASS_NOMAD        6
#define CLASS_PALADIN      7
#define CLASS_ANTI_PALADIN 8
#define CLASS_AVATAR       9
#define CLASS_BARD         10
#define CLASS_COMMANDO     11
#define CLASS_FIRST        CLASS_MAGIC_USER
#define CLASS_LAST         CLASS_COMMANDO

/* 'class' for NPC's */
#define CLASS_OTHER               0
#define CLASS_LICH                51
#define CLASS_LESSER_UNDEAD       52
#define CLASS_GREATER_UNDEAD      53
#define CLASS_LESSER_VAMPIRE      54
#define CLASS_GREATER_VAMPIRE     55
#define CLASS_LESSER_DRAGON       56
#define CLASS_GREATER_DRAGON      57
#define CLASS_LESSER_GIANT        58
#define CLASS_GREATER_GIANT       59
#define CLASS_LESSER_LYCANTHROPE  60
#define CLASS_GREATER_LYCANTHROPE 61
#define CLASS_LESSER_DEMON        62
#define CLASS_GREATER_DEMON       63
#define CLASS_LESSER_ELEMENTAL    64
#define CLASS_GREATER_ELEMENTAL   65
#define CLASS_LESSER_PLANAR       66
#define CLASS_GREATER_PLANAR      67
#define CLASS_HUMANOID            68
#define CLASS_HUMAN               69
#define CLASS_HALFLING            70
#define CLASS_DWARF               71
#define CLASS_ELF                 72
#define CLASS_BERSERKER           73
#define CLASS_KENDER              74
#define CLASS_TROLL               75
#define CLASS_INSECTOID           76
#define CLASS_ARACHNOID           77
#define CLASS_FUNGUS              78
#define CLASS_GOLEM               79
#define CLASS_REPTILE             80
#define CLASS_AMPHIBIAN           81
#define CLASS_DINOSAUR            82
#define CLASS_AVIAN               83
#define CLASS_FISH                84
#define CLASS_DOPPELGANGER        85
#define CLASS_ANIMAL              86
#define CLASS_AUTOMATON           87
#define CLASS_SIMIAN              88
#define CLASS_CANINE              89
#define CLASS_FELINE              90
#define CLASS_BOVINE              91
#define CLASS_PLANT               92
#define CLASS_RODENT              93
#define CLASS_BLOB                94
#define CLASS_GHOST               95
#define CLASS_ORC                 96
#define CLASS_GARGOYLE            97
#define CLASS_INVERTIBRATE        98
#define CLASS_DROW                99
#define CLASS_STATUE              100
#define CLASS_MOB_FIRST           CLASS_LICH
#define CLASS_MOB_LAST            CLASS_STATUE

/* sex */
#define SEX_NEUTRAL           0
#define SEX_MALE              1
#define SEX_FEMALE            2

/* positions */
#define POSITION_DEAD         0
#define POSITION_MORTALLYW    1
#define POSITION_INCAP        2
#define POSITION_STUNNED      3
#define POSITION_SLEEPING     4
#define POSITION_RESTING      5
#define POSITION_SITTING      6
#define POSITION_FIGHTING     7
#define POSITION_STANDING     8
#define POSITION_FLYING       9
#define POSITION_RIDING       10
#define POSITION_SWIMMING     11

/* New mob attacks - specials.att_type[] */
#define ATT_UNDEFINED         0  /* Never use undefined */
#define ATT_SPELL_CAST        1
#define ATT_KICK              2
#define ATT_PUMMEL            3
#define ATT_PUNCH             4
#define ATT_BITE              5
#define ATT_CLAW              6
#define ATT_BASH              7
#define ATT_TAILSLAM          8
#define ATT_DISARM            9
#define ATT_TRAMPLE           10
#define ATT_SPELL_SKILL       11
#define ATT_MAX               12

/* Targets for attacks - special.att_target[] */
#define TAR_UNDEFINED         0 /* doesn't attack anyone */
#define TAR_BUFFER            1 /* the usual, attacks the one hitting the mob */
#define TAR_RAN_GROUP         2 /* attacks at random from the buffers group   */
#define TAR_RAN_ROOM          3 /* random char from the room                  */
#define TAR_GROUP             4 /* whole group                                */
#define TAR_ROOM              5 /* whole room                                 */
#define TAR_SELF              6
#define TAR_LEADER            7
#define TAR_MAX               8

/* for mobile actions: specials.act */
#define ACT_SPEC              (1 << 0)  /* 1 */
#define ACT_SENTINEL          (1 << 1)  /* 2 */
#define ACT_SCAVENGER         (1 << 2)  /* 4 */
#define ACT_ISNPC             (1 << 3)  /* 8 */
#define ACT_NICE_THIEF        (1 << 4)  /* 16 */
#define ACT_AGGRESSIVE        (1 << 5)  /* 32 */
#define ACT_STAY_ZONE         (1 << 6)  /* 64 */
#define ACT_WIMPY             (1 << 7)  /* 128 */
#define ACT_SUBDUE            (1 << 8)  /* 256 */
#define ACT_OPEN_DOOR         (1 << 9)  /* 512 */
#define ACT_AGGWA             (1 << 10) /* 1024 */
#define ACT_AGGTH             (1 << 11) /* 2048 */
#define ACT_AGGCL             (1 << 12) /* 4096 */
#define ACT_AGGMU             (1 << 13) /* 8192 */
#define ACT_MEMORY            (1 << 14) /* 16384 */
#define ACT_AGGNI             (1 << 15) /* 32768 */
#define ACT_AGGNO             (1 << 16) /* 65536 */
#define ACT_ARM               (1 << 17) /* 131072 */
#define ACT_MOUNT             (1 << 18) /* 262144 */
#define ACT_FLY               (1 << 19) /* 524288 */
#define ACT_AGGPA             (1 << 20) /* 1048576 */
#define ACT_AGGAP             (1 << 21) /* 2097152 */
#define ACT_AGGBA             (1 << 22) /* 4194304 */
#define ACT_AGGCO             (1 << 23) /* 8388608 */
#define ACT_SHIELD            (1 << 24) /* 16777216 */
#define ACT_AGGEVIL           (1 << 25) /* 33554432 */
#define ACT_AGGGOOD           (1 << 26) /* 67108864 */
#define ACT_AGGNEUT           (1 << 27) /* 134217728 */
#define ACT_AGGLEADER         (1 << 28) /* 268435456 */
#define ACT_AGGRANDOM         (1 << 29) /* 536870912 */
#define ACT_FINAL             (1 << 30) /* 1073741824 */

/* More mobile actions specials.act2 */
#define ACT2_NO_TOKEN         (1 << 0)  /* 1 */
#define ACT2_IGNORE_SPHERE    (1 << 1)  /* 2 */
#define ACT2_FINAL            (1 << 2)  /* 4 */

/* Mob resistance  specials.resist */
#define RESIST_POISON         (1 << 0)  /* 1 */
#define RESIST_PHYSICAL       (1 << 1)  /* 2 */
#define RESIST_MAGICAL        (1 << 2)  /* 4 */
#define RESIST_FIRE           (1 << 3)  /* 8 */
#define RESIST_COLD           (1 << 4)  /* 16 */
#define RESIST_ELECTRIC       (1 << 5)  /* 32 */
#define RESIST_SOUND          (1 << 6)  /* 64 */
#define RESIST_CHEMICAL       (1 << 7)  /* 128 */
#define RESIST_ACID           (1 << 8)  /* 256 */

/* Immunities -- used in specials.immune */
#define IMMUNE_FIRE           (1 << 0)  /* 1 */
#define IMMUNE_ELECTRIC       (1 << 1)  /* 2 */
#define IMMUNE_POISON         (1 << 2)  /* 4 */
#define IMMUNE_PUMMEL         (1 << 3)  /* 8 */
#define IMMUNE_KICK           (1 << 4)  /* 16 */
#define IMMUNE_PUNCH          (1 << 5)  /* 32 */
#define IMMUNE_SLEEP          (1 << 6)  /* 64 */
#define IMMUNE_CHARM          (1 << 7)  /* 128 */
#define IMMUNE_BLINDNESS      (1 << 8)  /* 256 */
#define IMMUNE_PARALYSIS      (1 << 9)  /* 512 */
#define IMMUNE_DRAIN          (1 << 10) /* 1024 */
#define IMMUNE_DISEMBOWEL     (1 << 11) /* 2048 */
#define IMMUNE_DISINTEGRATE   (1 << 12) /* 4096 */
#define IMMUNE_CLAIR          (1 << 13) /* 8192 */
#define IMMUNE_SUMMON         (1 << 14) /* 16384 */
#define IMMUNE_HIT            (1 << 15) /* 32768 */
#define IMMUNE_BLUDGEON       (1 << 16) /* 65536 */
#define IMMUNE_PIERCE         (1 << 17) /* 131072 */
#define IMMUNE_SLASH          (1 << 18) /* 262144 */
#define IMMUNE_WHIP           (1 << 19) /* 524288 */
#define IMMUNE_CLAW           (1 << 20) /* 1048576 */
#define IMMUNE_BITE           (1 << 21) /* 2097152 */
#define IMMUNE_STING          (1 << 22) /* 4194304 */
#define IMMUNE_CRUSH          (1 << 23) /* 8388608 */
#define IMMUNE_HACK           (1 << 24) /* 16777216 */
#define IMMUNE_CHOP           (1 << 25) /* 33554432 */
#define IMMUNE_SLICE          (1 << 26) /* 67108864 */
#define IMMUNE_BACKSTAB       (1 << 27) /* 134217728 */
#define IMMUNE_AMBUSH         (1 << 28) /* 268435456 */
#define IMMUNE_ASSAULT        (1 << 29) /* 536870912 */
#define IMMUNE_PHYSICAL       (1 << 30) /* 1073741824 */
#define IMMUNE_MAGICAL        (1 << 31) /* 2147483648 */

/* Immunities 2 -- used in specials.immune2 */
#define IMMUNE2_LOCATE        (1 << 0)  /* 1 */
#define IMMUNE2_COLD          (1 << 1)  /* 2 */
#define IMMUNE2_SOUND         (1 << 2)  /* 4 */
#define IMMUNE2_CHEMICAL      (1 << 3)  /* 8 */
#define IMMUNE2_ACID          (1 << 4)  /* 16 */
#define IMMUNE2_FEAR          (1 << 5)  /* 32 */
#define IMMUNE2_BASH          (1 << 6)  /* 64 */
#define IMMUNE2_CIRCLE        (1 << 7)  /* 128 */
#define IMMUNE2_TAUNT         (1 << 8)  /* 256 */
#define IMMUNE2_STEAL         (1 << 9)  /* 512 */
#define IMMUNE2_CURSE         (1 << 10) /* 1024 */
#define IMMUNE2_HOLD          (1 << 11) /* 2048 */

/* Player flags -- used in specials.pflag */
#define PLR_BRIEF             (1 << 0)  /* 1 */
#define PLR_NOSHOUT           (1 << 1)  /* 2 */
#define PLR_COMPACT           (1 << 2)  /* 4 */
#define PLR_WRITING           (1 << 3)  /* 8 */
#define PLR_KILL              (1 << 4)  /* 16 */
#define PLR_THIEF             (1 << 5)  /* 32 */
#define PLR_NOKILL            (1 << 6)  /* 64 */
#define PLR_SECTOR_BRIEF      (1 << 7)  /* 128 */
#define PLR_NOSUMMON          (1 << 8)  /* 256 */
#define PLR_GOSSIP            (1 << 9)  /* 512 */
#define PLR_AUCTION           (1 << 10) /* 1024 */
#define PLR_SHOUT_OFF         (1 << 11) /* 2048 */
#define PLR_NOMESSAGE         (1 << 12) /* 4096 */
#define PLR_SANES_VOCAL_CLUB  (1 << 13) /* 8192 */
#define PLR_LEMS_LIQOUR_ROOM  (1 << 14) /* 16384 */
#define PLR_LINERS_LOUNGE     (1 << 15) /* 32768 */
#define PLR_RANGERS_RELIQUARY (1 << 16) /* 65536 */
#define PLR_QUEST             (1 << 17) /* 131072 */
#define PLR_CHAOS             (1 << 18) /* 262144 */
#define PLR_DEPUTY            (1 << 19) /* 524288 */
#define PLR_QUIET             (1 << 20) /* 1048576 */
#define PLR_QUESTC            (1 << 21) /* 2097152 */
#define PLR_SUPERBRF          (1 << 22) /* 4194304 */
#define PLR_FIGHTBRF          (1 << 23) /* 8388608 */
#define PLR_SKIPTITLE         (1 << 24) /* 16777216 */
#define PLR_VICIOUS           (1 << 25) /* 33554432 */
#define PLR_YELL_OFF          (1 << 26) /* 67108864 */
#define PLR_EMAIL             (1 << 27) /* 134217728 */
#define PLR_TAGBRF            (1 << 28) /* 268435456 */
#define PLR_PROMPT_NEWLINE    (1 << 29) /* 536870912 */
#define PLR_TICKER            (1 << 30) /* 1073741824 */

/* Toggles -- used in ver3.toggles */
#define TOG_BLOCK             (1 << 0)  /* 1 */
#define TOG_CUNNING           (1 << 1)  /* 2 */
#define TOG_DIRTY_TRICKS      (1 << 2)  /* 4 */
#define TOG_VEHEMENCE         (1 << 3)  /* 8 */
#define TOG_TRIP              (1 << 4)  /* 16 */
#define TOG_EVASION           (1 << 5)  /* 32 */
#define TOG_AWARENESS         (1 << 6)  /* 64 */
#define TOG_HOSTILE           (1 << 7)  /* 128 */
#define TOG_TRUSTY_STEED      (1 << 8)  /* 256 */
#define TOG_SHADOWSTEP        (1 << 9)  /* 512 */
#define TOG_VICTIMIZE         (1 << 10) /* 1024 */
#define TOG_SNIPE             (1 << 11) /* 2048 */

/* Wizard flags -- used in specials.wizflags */
#define WIZ_TRUST             (1 << 0)  /* 1 */
#define WIZ_QUEST             (1 << 1)  /* 2 */
#define WIZ_FREEZE            (1 << 2)  /* 4 */
#define WIZ_LOAD              (1 << 3)  /* 8 */
#define WIZ_NO_NET            (1 << 4)  /* 16 */
#define WIZ_LOG_ONE           (1 << 5)  /* 32 */
#define WIZ_LOG_TWO           (1 << 6)  /* 64 */
#define WIZ_LOG_THREE         (1 << 7)  /* 128 */
#define WIZ_LOG_FOUR          (1 << 8)  /* 256 */
#define WIZ_LOG_FIVE          (1 << 9)  /* 512 */
#define WIZ_CREATE            (1 << 10) /* 1024 */
#define WIZ_LOG_SIX           (1 << 11) /* 2048 */
#define WIZ_QUEST_INFO        (1 << 12) /* 4096 */
#define WIZ_JUDGE             (1 << 13) /* 8192 */
#define WIZ_ACTIVE            (1 << 14) /* 16384 */
#define WIZ_WIZNET            (1 << 15) /* 32768 */
#define WIZ_CHAOS             (1 << 16) /* 65536 */
#define WIZ_SNOOP_BRIEF       (1 << 17) /* 131072 */


/* Character Structs */

typedef struct memory_record_t {
  int id;
  struct memory_record_t *next;
} memory_record_t;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
  ubyte hours, day, month;
  sh_int year;
};

/* These data contain information about a players time data */
struct time_data {
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  int played;      /* This is the total accumulated time played in secs */
};

struct char_player_data
{
  char *name;          /* PC / NPC s name (kill ...  )         */
  char *short_descr;  /* for 'actions'                        */
  char *long_descr;   /* for 'look'.. Only here for testing   */
  char *description;  /* Extra descriptions                   */

  struct tagline_data *tagline;

  char *title;        /* PC / NPC s title                     */
  char *poofin;
  char *poofout;

  ubyte sex;           /* PC / NPC s sex                       */
  ubyte class;         /* PC  class or NPC alignment          */
  ubyte level;         /* PC / NPC s level                     */

  int hometown;       /* PC s Hometown (zone)                 */

  bool talks[MAX_TONGUE]; /* PC s Tongues 0 for NPC           */

  struct time_data time; /* PC s AGE in days                 */

  ubyte weight;       /* PC / NPC s weight                    */
  ubyte height;       /* PC / NPC s height                    */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data
{
  sbyte str;
  sbyte str_add;      /* 000 - 100 if strength 18             */
  sbyte intel;
  sbyte wis;
  sbyte dex;
  sbyte con;
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data_pfile
{
  sh_int mana;
  sh_int max_mana;     /* Not useable may be erased upon player file renewal */
  sh_int hit;
  sh_int max_hit;      /* Max hit for NPC                         */
  sh_int move;
  sh_int max_move;     /* Max move for NPC                        */

  sh_int armor;        /* Internal -100..100, external -10..10 AC */
  int32_t gold;            /* Money carried                           */
  int32_t exp;             /* The experience of the player            */
  int32_t bank;

  sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
  sbyte damroll;       /* Any bonus or penalty to the damage roll */
};

struct char_point_data_all
{
  sh_int mana;
  sh_int max_mana;     /* Not useable may be erased upon player file renewal */
  int hit;
  int max_hit;      /* Max hit for NPC                         */
  sh_int move;
  sh_int max_move;     /* Max move for NPC                        */

  int mana_regen_tmp;

  sh_int armor;     /* Internal -100..100, external -10..10 AC */
  int gold;         /* Money carried                           */
  int exp;          /* The experience of the player            */
  int bank;

  sbyte hitroll;   /* Any bonus or penalty to the hit roll    */
  sh_int damroll;   /* Any bonus or penalty to the damage roll */
};

struct char_special_data
{
  byte position;           /* Standing or ...                         */
  byte default_pos;        /* Default position for NPC                */
  unsigned long pflag;     /* flags for PLR flags                     */
  unsigned long act;       /* flags for NPC behavior                  */
  unsigned long act2;      /* flags for NPC behavior                  */
  unsigned long immune;    /* New mob immunities - Ranger Sept 96 */
  unsigned long immune2;
  unsigned long resist;
  int death_timer;         /* Countdown for imminent_death */

  byte spells_to_learn;    /* How many can you learn yet this level   */

  /* New mob attack specials - Ranger Sept 96 */
  sh_int no_att;
  sh_int att_type[10];
  sh_int att_target[10];
  sh_int att_percent[10];
  sh_int att_spell[10];
  int att_timer;

  sh_int org_mana;         /* Used for orginal stats in dlist */
  sh_int org_hit;
  sh_int org_move;

  sh_int prev_max_mana;        /* Used for stat check in save_char */
  sh_int prev_max_hit;
  sh_int prev_max_move;

/*  int carry_weight;*/      /* Carried weight                          */
  ubyte carry_items;        /* Number of items carried                 */
  int timer;               /* Timer for update                        */
  sh_int was_in_room;      /* storage of location for linkdead people */
  sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)             */
  sbyte conditions[4];      /* Drunk full etc.                        */

  byte damnodice;           /* The number of damage dice's            */
  byte damsizedice;         /* The size of the damage dice's          */
  byte last_direction;      /* The last direction the monster went    */
  int attack_type;          /* The Attack Type Bitvector for NPC's    */
  int alignment;            /* +-1000 for alignments                  */
  memory_record_t *memory;     /* List of attackers to remember ...     */

  long affected_by;        /* Bitvector for spells/skills affected by */
  long affected_by2;        /* Bitvector for spells/skills affected by */

  struct char_data *fighting; /* Opponent                             */
  int num_fighting;
  int max_num_fighting;
  struct char_data *riding;
  struct char_data *rider;
  struct char_data *protecting;
  struct char_data *protect_by;
  int reply_to;
  char vaultname[20];
  int vaultaccess;
  int message;
  int zone;
  int wiznetlvl;
};

/* Used in-memory only. */
struct enchantment_t {
  /* Stored to pfile as AFF. */
  int    type;       /* SPELL_* and SKILL_* - AFF->type */
  sh_int duration;   /* Duration            - AFF->duration */
  int    modifier;   /* Modifier            - AFF->duration (truncated to sbyte) */
  byte   location;   /* APPLY_*             - AFF->location */
  long   bitvector;  /* AFF_*               - AFF->bitvector */
  long   bitvector2; /* AFF2_*              - AFF->bitvector2 */

  struct enchantment_t *next;

  /* Not stored to pfile. */
  char   *name;     /* Used for enchantment list lookups. */
  int    interval;  /* Duration interval (e.g. round, mobact, tick, etc.) */
  int    temp[4];   /* Temporary integer storage. */
  char   *metadata; /* Temporary string storage. */

  ENCH_FUNC func;
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data
{
  byte learned;           /* % chance for success 0 = not learned   */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_new_data
{
  int32_t wizinv;
  int32_t wimpy;
  int32_t  killed;
  int32_t been_killed;
  char host[50];
  uint32_t prompt;
  int32_t imm_flags;
};

/* As these get used, update to store_to_char_2 in reception */
/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ver3_data {
  int32_t death_limit;
  int32_t bleed_limit;
  int32_t ranking; /* How char compares with others */
  int32_t quest_points;
  byte clan_num;
  byte house_num; /* Not used yet */
  byte race; /* Not used yet */
  char email_addr[80]; /* Ranger Dec 03 */
  uint64_t toggles; /* Toggles */
  uint32_t extra_bitvect; /* Not used yet */
  byte extra_byte[5]; /* Not used yet */
  byte affect_style;
  ubyte rank;
  ubyte prestige;
  ubyte who_filter; /* Filters specified for 'who' output. */
  byte sc_style; /* Which score style to display - Ranger Sept 2000 */
  int32_t created; /* Date of creation mmddyyyy - Ranger June 98 */
  int32_t subclass;
  int32_t subclass_level;
  int32_t subclass_points;
  int32_t time_to_quest; /* Ranger Feb 99 */
  int32_t id; /* Ranger - Sep 00 */
  int32_t death_timer; /* Used for imminent death */
  int64_t remort_exp; /* Remort EXP Pool */
  uint32_t death_exp; /* Death Pool */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

#if 0
struct enchantment_type_4 /* All pfile versions up to 4 */
{
  char   *name;        /* Used to keep track of the enchantment.
                        * required to match a name in the 'enchantment'
                        * list in order to associate a function with
                        * it.                                     */
  sh_int temps[5];        /* Temporary storage */
  ubyte  type;            /* The type of spell that caused this      */
  sh_int duration;        /* For how long its effects will last      */
  sbyte  modifier;        /* This is added to apropriate ability     */
  byte   location;        /* Tells which ability to change(APPLY_XXX)*/
  long   bitvector;       /* Tells which bits to set (AFF_XXX)       */
  int    (*func)(struct enchantment_type_4 *enchantment,struct char_data *ch, struct char_data *other, int CMD, char* arg);
  struct enchantment_type_4 *next;
};
#endif

/* Version 5 pfile */
struct affected_type_5
{
  int type;            /* The type of spell that caused this      */
  sh_int duration;      /* For how long its effects will last      */
  sbyte modifier;       /* This is added to apropriate ability     */
  byte location;        /* Tells which ability to change(APPLY_XXX)*/
  long bitvector;       /* Tells which bits to set (AFF_XXX)       */
  long bitvector2;       /* Tells which bits to set (AFF_XXX)       */
  struct affected_type_5 *next;
};

struct affected_type_5_pfile
{
  int32_t type;            /* The type of spell that caused this      */
  sh_int duration;      /* For how long its effects will last      */
  sbyte modifier;       /* This is added to apropriate ability     */
  byte location;        /* Tells which ability to change(APPLY_XXX)*/
  int32_t bitvector;       /* Tells which bits to set (AFF_XXX)       */
  int32_t bitvector2;       /* Tells which bits to set (AFF_XXX)       */
  uint32_t dummy;
};

/* Version 3 & 4 pfile - current affected type */
struct affected_type_4
{
  int type;            /* The type of spell that caused this      */
  sh_int duration;      /* For how long its effects will last      */
  sbyte modifier;       /* This is added to apropriate ability     */
  byte location;        /* Tells which ability to change(APPLY_XXX)*/
  long bitvector;       /* Tells which bits to set (AFF_XXX)       */
  uint32_t dummy; //  struct affected_type_4 *next;
};

/* Version 2 pfile affected type */
struct affected_type_2
{
  byte type;            /* The type of spell that caused this      */
  sh_int duration;      /* For how long its effects will last      */
  sbyte modifier;       /* This is added to apropriate ability     */
  byte location;        /* Tells which ability to change(APPLY_XXX)*/
  long bitvector;       /* Tells which bits to set (AFF_XXX)       */
  uint32_t dummy; //struct affected_type_2 *next;
};


/* ================== Structure for player/non-player ===================== */

struct follow_type {
  CHAR *follower;
  FOL  *next;
};

struct bot_check {
  unsigned long meta_update;
  int           meta_amount;
  int           meta_number;
  int           misses;
};

struct idname_struct {
  char name[20];
};

/* Player data. Used in-memory only. */
struct char_data {
  sh_int nr;                               /* Mobile RNUM */
  sh_int nr_v;                             /* Mobile VNUM */

  sh_int in_room_r;                        /* In Room RNUM */
  sh_int in_room_v;                        /* In Room VNUM */

  DESC *desc;                              /* Descriptor (Player Only) */

  struct char_player_data    player;       /* Character Data */
  struct char_ability_data   abilities;    /* Ability Data */
  struct char_ability_data   tmpabilities; /* Temporary Ability Data */
  struct char_point_data_all points;       /* Point Data */
  struct char_special_data   specials;     /* Special Data */
  struct char_new_data       new;          /* New Data */
  struct char_ver3_data      ver3;         /* Version 3 Data */

  struct char_skill_data *skills;          /* Skill Data (Player Only)*/

  int  quest_status;                       /* Auto-Quest Status */
  int  quest_level;                        /* Auto-Quest Level */
  int  quest_room_v;                       /* Auto-Quest Initial Target Room VNUM */
  bool questmob_ineligible;                /* Auto-Quest Mobile Ineligible Flag */
  CHAR *questgiver;                        /* Auto-Quest Giver (Mobile) */
  CHAR *questowner;                        /* Auto-Quest Owner (Player) */
  CHAR *questmob;                          /* Auto-Quest Mobile Target */
  OBJ  *questobj;                          /* Auto-Quest Object Target */

  AFF  *affected;                          /* Affect List */
  ENCH *enchantments;                      /* Enchantment List */

  OBJ  *equipment[MAX_WEAR];               /* Equipment Array */
  OBJ  *carrying;                          /* Inventory List */

  CHAR *next;                              /* Next Character in Character List */
  CHAR *next_in_room;                      /* Next Character in Room List */
  CHAR *next_fighting;                     /* Next Character in Combat List */

  CHAR *master;                            /* Master Character Pointer */
  FOL  *followers;                         /* Follower List */

  CHAR *switched;                          /* Mobswitch Character Pointer */

  int colors[MAX_COLORS];                  /* Color Array */

  struct bot_check bot;                    /* Anti-bot Data */

  char pwd[11];                            /* Player Password */

  int (*func)(CHAR *, CHAR *, int, const char *); /* Special character function. */
};


/* ======================================================================== */

/* How much light is in the land ? */

#define SUN_DARK   0
#define SUN_RISE   1
#define SUN_LIGHT  2
#define SUN_SET    3

/* And how is the sky ? */

#define SKY_CLOUDLESS  0
#define SKY_CLOUDY     1
#define SKY_RAINING    2
#define SKY_LIGHTNING  3

struct weather_data
{
  int pressure; /* How is the pressure ( Mb ) */
  int change;   /* How fast and what way does it change. */
  int sky;      /* How is the sky. */
  int sunlight; /* And how much sun. */
};

struct corpsefile_name{
  char name[30];
};

/* New dlist structure - Ranger May 97 */
struct death_file_check {
  sbyte flag; /* 1 - death_file_u, 2 - obj data, 9 - end */
};

struct death_file_u
{
  int32_t number;
  char name[20];
  int32_t time_death;
  int16_t location;
  byte level;
  int32_t gold;
  int32_t exp;
  sh_int hp;
  sh_int mana;
  sh_int move;
  sbyte str;
  sbyte add;
  sbyte intel;
  sbyte wis;
  sbyte dex;
  sbyte con;
};

/* ***********************************************************************
*  file element for object file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

#define WPN_SLAY_EVIL_BEINGS    21
#define WPN_SLAY_NEUTRAL_BEINGS 22
#define WPN_SLAY_GOOD_BEINGS    23
#define WPN_CHAOTIC             30

#define WPN_SLAY_MAGIC_USERS    30 + CLASS_MAGIC_USER
#define WPN_SLAY_CLERICS        30 + CLASS_CLERIC
#define WPN_SLAY_THIEVES        30 + CLASS_THIEF
#define WPN_SLAY_WARRIORS       30 + CLASS_WARRIOR
#define WPN_SLAY_NINJAS         30 + CLASS_NINJA
#define WPN_SLAY_NOMADS         30 + CLASS_NOMAD
#define WPN_SLAY_PALADINS       30 + CLASS_PALADIN
#define WPN_SLAY_ANTI_PALADINS  30 + CLASS_ANTI_PALADIN
#define WPN_SLAY_AVATARS        30 + CLASS_AVATAR
#define WPN_SLAY_BARDS          30 + CLASS_BARD
#define WPN_SLAY_COMMANDOS      30 + CLASS_COMMANDO
#define WPN_SLAY_FIRST          WPN_SLAY_MAGIC_USERS
#define WPN_SLAY_LAST           WPN_SLAY_COMMANDOS

#define WPN_CLASS_MAGIC_USER    300 + CLASS_MAGIC_USER
#define WPN_CLASS_CLERIC        300 + CLASS_CLERIC
#define WPN_CLASS_THIEF         300 + CLASS_THIEF
#define WPN_CLASS_WARRIOR       300 + CLASS_WARRIOR
#define WPN_CLASS_NINJA         300 + CLASS_NINJA
#define WPN_CLASS_NOMAD         300 + CLASS_NOMAD
#define WPN_CLASS_PALADIN       300 + CLASS_PALADIN
#define WPN_CLASS_ANTI_PALADIN  300 + CLASS_ANTI_PALADIN
#define WPN_CLASS_AVATAR        300 + CLASS_AVATAR
#define WPN_CLASS_BARD          300 + CLASS_BARD
#define WPN_CLASS_COMMANDO      300 + CLASS_COMMANDO
#define WPN_CLASS_FIRST         WPN_CLASS_MAGIC_USER
#define WPN_CLASS_LAST          WPN_CLASS_COMMANDO

#define MAX_OBJ_VALUE 4

struct obj_file_version {
  sh_int version;
};

struct obj_file_elem_ver0
{
  sh_int item_number;

  int32_t value[MAX_OBJ_VALUE];
  int32_t extra_flags;
  int32_t weight;
  int32_t timer;
  int32_t bitvector;
  struct obj_affected_type_ver0 affected[OFILE_MAX_OBJ_AFFECT];
  byte position;
};

struct obj_file_elem_ver1
{
  sh_int version;
  sh_int item_number;

  ubyte type_flag; /* new saveable */
  int32_t value[MAX_OBJ_VALUE];
  int32_t wear_flags; /* new saveable */
  int32_t extra_flags;
  int32_t extra_flags2; /* new saveable */
  int32_t subclass_res; /* new saveable */
  int32_t weight;
  int32_t timer;
  int32_t material; /* new saveable */
  int32_t bitvector;
  int32_t spec_value; /* new saveable OBJ_SPEC() */
  struct obj_affected_type affected[MAX_OBJ_AFFECT]; /* new saveable */
  byte position;
};

struct obj_file_elem_ver2
{
  sh_int version;
  sh_int item_number;

  ubyte type_flag; /* new saveable */
  int32_t value[MAX_OBJ_VALUE];
  int32_t ownerid[8];  /* Owner Ids */
  int32_t wear_flags; /* new saveable */
  int32_t extra_flags;
  int32_t extra_flags2; /* new saveable */
  int32_t subclass_res; /* new saveable */
  int32_t weight;
  int32_t timer;
  int32_t material; /* new saveable */
  int32_t bitvector;
  int32_t spec_value; /* new saveable OBJ_SPEC() */
  struct obj_affected_type affected[MAX_OBJ_AFFECT]; /* new saveable */
  byte position;
};

struct obj_file_elem_ver3 /* Addition of bitvector2  */
{
  sh_int version;
  sh_int item_number;

  ubyte type_flag;
  int32_t value[MAX_OBJ_VALUE];
  int32_t ownerid[MAX_OBJ_OWNER_ID];
  int32_t wear_flags;
  int32_t extra_flags;
  int32_t extra_flags2;
  int32_t subclass_res;
  int32_t weight;
  int32_t timer;
  int32_t material;
  int32_t bitvector;
  int32_t bitvector2;
  int32_t spec_value;
  struct obj_affected_type affected[MAX_OBJ_AFFECT];
  int32_t popped;
  byte unused[116];
  byte position;
};

/* ***********************************************************************
*  file element for player file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

struct char_file_version {
  byte version;
};

/* Char file version 5 - current
** Version 5 adds bitvector2 functionality to enchantments and spells
** Version 4 is used to automatically convert from lvl 30 to lvl 50 in reception.c
**    additions from version 2
**      - byte version as first field
**      - struct char_ver3_data ver3
**      - removed special[10]
** Ranger May 97
*/

/* Version 5 */
struct char_file_u_5
{
  byte version;
  byte sex;
  byte class;
  byte level;
  uint32_t birth; //time_t birth;  /* Time of birth of character     */
  int32_t played;    /* Number of secs played in total */

  ubyte weight;
  ubyte height;

  char title[80];
  sh_int hometown;
  char description[240];
  char poofin[150];
  char poofout[150];
  bool talks[MAX_TONGUE];
  int32_t colors[MAX_COLORS];
  sh_int load_room;            /* Which room to place char in           */

  struct char_ability_data abilities;

  struct char_point_data_pfile points;

  struct char_new_data new;
  struct char_ver3_data ver3;

  struct char_skill_data skills[MAX_SKILLS5];

  struct affected_type_5_pfile affected[MAX_AFFECT];
  struct affected_type_5_pfile enchantments[MAX_AFFECT];
  /* specials */

  byte spells_to_learn;
  int32_t alignment;

  int32_t last_logon; //time_t last_logon;  /* Time (in secs) of last logon */
  uint32_t pflag;          /* PLR Flags                    */

  /* char data */
  char name[20];
  char pwd[11];
  sh_int apply_saving_throw[5];
  int32_t conditions[3];

  int32_t total_cost;    /* The cost for all items, per day    */
  int32_t last_update;  /* Time in seconds, when last updated */
};

/* Versions 3 & 4 */
struct char_file_u_4
{
  byte version;
  byte sex;
  byte class;
  byte level;
  int32_t birth; //time_t birth;  /* Time of birth of character     */
  int32_t played;    /* Number of secs played in total */

  ubyte weight;
  ubyte height;

  char title[80];
  sh_int hometown;
  char description[240];
  char poofin[150];
  char poofout[150];
  bool talks[MAX_TONGUE];
  int32_t colors[MAX_COLORS];
  sh_int load_room;            /* Which room to place char in           */

  struct char_ability_data abilities;

  struct char_point_data_pfile points;

  struct char_new_data new;
  struct char_ver3_data ver3;

  struct char_skill_data skills[MAX_SKILLS4];

  struct affected_type_4 affected[MAX_AFFECT];
  struct affected_type_4 enchantments[MAX_AFFECT];
  /* specials */

  byte spells_to_learn;
  int32_t alignment;

  int32_t last_logon;  //time_t last_logon;  /* Time (in secs) of last logon */
  uint32_t pflag;          /* PLR Flags                    */

  /* char data */
  char name[20];
  char pwd[11];
  sh_int apply_saving_throw[5];
  int32_t conditions[3];

  int32_t total_cost;    /* The cost for all items, per day    */
  int32_t last_update;  /* Time in seconds, when last updated */
};

/* Char file version 2 - Superceeded May 97 - Ranger */
struct char_file_u_2
{
  byte sex;
  byte class;
  byte level;
  int32_t birth; //time_t birth;  /* Time of birth of character     */
  int32_t played;    /* Number of secs played in total */

  ubyte weight;
  ubyte height;

  char title[80];
  sh_int hometown;
  char description[240];
  char poofin[150];
  char poofout[150];
  bool talks[MAX_TONGUE];
  int32_t colors[MAX_COLORS];
  int32_t special[10];

  sh_int load_room;            /* Which room to place char in           */

  struct char_ability_data abilities;

  struct char_point_data_pfile points;

  struct char_new_data new;

  struct char_skill_data skills[MAX_SKILLS4];

  struct affected_type_2 affected[MAX_AFFECT];
  struct affected_type_2 enchantments[MAX_AFFECT];

  /* specials */

  byte spells_to_learn;
  int32_t alignment;

  int32_t last_logon; //time_t last_logon;  /* Time (in secs) of last logon */
  uint32_t pflag;          /* PLR Flags                    */

  /* char data */
  char name[20];
  char pwd[11];
  sh_int apply_saving_throw[5];
  int32_t conditions[3];

  int32_t total_cost;    /* The cost for all items, per day    */
  int32_t last_update;  /* Time in seconds, when last updated */
};

/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */

struct txt_block {
  char *text;
  struct txt_block *next;
};

struct txt_q {
  struct txt_block *head;
  struct txt_block *tail;
};

/* modes of connectedness */

#define CON_CLOSE     -1
#define CON_PLYNG      0
#define CON_NME        1
#define CON_NMECNF     2
#define CON_PWDNRM     3
#define CON_PWDGET     4
#define CON_PWDCNF     5
#define CON_QSEX       6
#define CON_RMOTD      7
#define CON_SLCT       8
#define CON_EXDSCR     9
#define CON_QCLASS    10
#define CON_LDEAD     11
#define CON_UNUSED_13 12
#define CON_UNUSED_12 13
#define CON_AUTH      14
#define CON_UNUSED_15 15
#define CON_QCOLOR    16

struct snoop_data
{
  struct char_data *snooping;
    /* Who is this char snooping */
  struct char_data *snoop_by;
    /* And who is snooping on this char */
};

#define PROMPT_HP         00000001
#define PROMPT_HP_MAX     00000002
#define PROMPT_HP_TEX     00000004
#define PROMPT_MANA       00000010
#define PROMPT_MANA_MAX   00000020
#define PROMPT_MANA_TEX   00000040
#define PROMPT_MOVE       00000100
#define PROMPT_MOVE_MAX   00000200
#define PROMPT_MOVE_TEX   00000400
#define PROMPT_BUFFER     00001000
#define PROMPT_BUFFER_A   00002000
#define PROMPT_BUFFER_TEX 00004000
#define PROMPT_VICTIM     00010000
#define PROMPT_VICTIM_A   00020000
#define PROMPT_VICTIM_TEX 00040000
#define PROMPT_NAME       00100000

struct descriptor_data
{
  int descriptor;               /* file descriptor for socket */
  int timer;                    /* for menu items not entered game */
  char *name;                   /* copy of the player name    */
  char host[50];                /* hostname                   */
  char userid[50];              /* hostname                   */
  int  port;
  uint32_t addr;
  char pwd[12];                 /* password                   */
  char wizinfo;                 /* wizinfo level              */
  int free_rent;
  int pos;                      /* position in player-file    */
  int connected;                /* mode of 'connectedness'    */
  int wait;                     /* wait for how many loops    */
  char *showstr_head;           /* for paging through texts  */
  char *showstr_point;          /*       -                    */
  char **str;                   /* for the modify-str system  */
  int max_str;                  /* -                          */
  unsigned long prompt;
  int prompt_mode;              /* control of prompt-printing */
  char buf[MAX_STRING_LENGTH];  /* buffer for raw input       */
  char last_input[MAX_INPUT_LENGTH];/* the last input         */
  struct txt_q output;          /* q of strings to send       */
  struct txt_q input;           /* q of unprocessed input     */
  struct char_data *character;  /* linked to char             */
  struct char_data *original;   /* original char              */
  struct snoop_data snoop;      /* to snoop people.           */
  struct descriptor_data *next; /* link to next descriptor    */
};

struct msg_type {
  char *attacker_msg;
  char *victim_msg;
  char *room_msg;
};

struct message_type {
  struct msg_type die_msg;
  struct msg_type miss_msg;
  struct msg_type hit_msg;
  struct msg_type god_msg;

  struct message_type *next;
};

struct message_list {
  int attack_type;
  int number_of_attacks;

  struct message_type *msg;
};

struct dex_skill_type
{
  sh_int p_pocket;
  sh_int sneak;
  sh_int hide;
};

struct dex_app_type
{
  sh_int defensive;
  int prac_bonus;
};

struct con_app_type
{
  int hitp;
  int regen;
  int reduct;
};

struct str_app_type
{
  sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
  sh_int todam;    /* Damage Bonus/Penalty                */
  sh_int carry_w;  /* Maximum weight that can be carrried */
  sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
  byte bonus;       /* how many bonus skills a player can practice pr. level */
  int conc;         /* concentration bonus */
};

struct int_app_type
{
  byte learn;       /* how many % a player learns a spell/skill */
  int conc;         /* concentration bonus */
};


struct obj_proto
{
  char *name;
  int  virtual;/*virtual number*/
  int  number;/*instances*/
  int (*func)(struct obj_data*, struct char_data*, int, char*);

  struct obj_flag_data obj_flags;

  char *short_description;
  char *description;
  char *action_description;
  struct extra_descr_data *ex_description;

  struct obj_affected_type affected[MAX_OBJ_AFFECT];
  char *action_description_nt;
  char *char_wear_desc;          /* What to write when worn */
  char *room_wear_desc;          /* What to write when worn */
  char *char_rem_desc;           /* What to write when removed */
  char *room_rem_desc;           /* What to write when removed */
};

struct mob_proto
{
  char *name;        /* NPC s name (kill ...  )                  */
  char *short_descr;        /* for 'actions'                            */
  char *long_descr;         /* for 'look'.. Only here for testing       */
  char *description;        /* Extra descriptions                       */
  struct tagline_data *tagline;
  int skin_value;           /* Skin - Ranger Feb 2001 */
  int skin_vnum[6];
  byte level;               /* NPC s level                              */
  byte sex;                 /* NPC s sex                                */
  byte class;               /* Ranger - Sept 96 */

  sbyte hitroll;            /* Any bonus or penalty to the hit roll     */
  sh_int damroll;            /* Any bonus or penalty to the damage roll  */
  int gold;                 /* Money carried                            */
  int exp;                  /* The experience of the player             */

  unsigned long act;        /* flags for NPC behavior                   */
  unsigned long affected_by; /* Bitvector for spells/skills affected by */
  unsigned long immune;     /* New mob immunities - Ranger Sept 96 */
/* More room for acts, etc - Ranger March 99 */
  unsigned long act2;
  unsigned long affected_by2;
  unsigned long immune2;
  unsigned long resist;
  int hit_type;             /* Future addition of hit_type */

  sh_int alignment;         /* +-1000 for alignments                    */
  byte damnodice;           /* The number of damage dice's              */
  byte damsizedice;         /* The size of the damage dice's            */
  byte position;            /* Standing or ...                          */
  byte default_pos;         /* Default position for NPC                 */

  sh_int armor;             /* NPC armor                                */
  sh_int hp_nodice;         /* number of hp dice's                      */
  sh_int hp_sizedice;       /* size of hp dice's                        */
  int hp_add;               /* hp add                                   */


  /* New mob mana - Ranger Sept 96 */
  sh_int mana_nodice;
  sh_int mana_sizedice;
  sh_int mana_add;

  /* New mob attacks - Ranger Sept 96 */
  sh_int no_att;
  sh_int att_type[10];
  sh_int att_target[10];
  sh_int att_percent[10];
  sh_int att_spell[10];

  int  virtual;/*virtual number*/
  int  number;/*instances*/
  int loads; /* # items loaded */
  int (*func)(struct char_data*, struct char_data*, int, char*);
};

#pragma pack (pop)

#endif /* __STRUCTS_H__ */
