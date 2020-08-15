/* ************************************************************************
*  file: db.h , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars booting world.                             *
************************************************************************* */

#ifndef _DB_H_
#define _DB_H_

#include <stdio.h>

#include "structs.h"

/* data files used by the game system */

#define DFLT_DIR          "lib"           /* default data directory     */
#define WORLD_LIST_FILE   "tinyworld.idx" /* the list of zones to include */
#define CREDITS_FILE      "credits"       /* for the 'credits' command  */
#define HEROES_FILE       "heroes"        /* for the 'heroes' command  */
#define NEWS_FILE         "news"          /* for the 'news' command     */
#define MOTD_FILE         "motd"          /* messages of today          */
#define NEWBIEMOTD_FILE   "motd.newbie"
#define GODMOTD_FILE      "motd.god"
#define TIME_FILE         "time"          /* game calendar information  */
#define IDEA_FILE         "ideas"         /* for the 'idea'-command     */
#define TYPO_FILE         "typos"         /*         'typo'             */
#define BUG_FILE          "bugs"          /*         'bug'              */
#define MESSAGES_FILE     "messages"      /* damage messages            */
#define SOCMESS_FILE      "actions"       /* messgs for social acts     */
#define HELP_KWRD_FILE    "help_table"    /* for HELP <keywrd>          */
#define HELP_PAGE_FILE    "help"          /* for HELP <CR>              */
#define WIZLIST_FILE      "wizlist"       /* for WIZLIST                */
#define WIZLIST_INACTIVE_FILE "wizlist_inact"
#define POSEMESS_FILE     "poses"         /* for 'pose'-command         */
#define WIZHELP_FILE      "wizhelp"       /* Thanks Clear for that      */
#define OLCHELP_FILE      "olchelp"
#define MAX_ID            15000 /* Maximum number of id's stored in name/id list */

/* public procedures in db.c */
void distribute_tokens(const int num_tokens, bool rebuild_table);
int inzone(int);
void boot_db(void);
void save_char(struct char_data *ch, sh_int load_room);
int create_entry(char *name);
void zone_update(void);
void init_char(struct char_data *ch);
void clear_char(struct char_data *ch);
void clear_skills(struct char_skill_data *skills);
void clear_object(struct obj_data *obj);
void reset_char(struct char_data *ch);
void free_char(struct char_data *ch);
char *fread_string(FILE *fl);
void update_time(void);
int search_zone_table(int l, int r, int vnum);
int search_world_table(int l, int r, int vnum);
int search_mob_table(int l, int r, int vnum);
int search_obj_table(int l, int r, int vnum);
int search_shop_table(int l, int r, int vnum);
int search_fight_messages_list(int l, int r, int attack_type);
int allocate_zone(int vnum);
int allocate_room(int vnum);
int allocate_mob(int vnum);
int allocate_obj(int vnum);
int allocate_shop(int vnum);
int allocate_fight_message(int attack_type);
void load_messages(void);
void renum_world(void);
void reset_zone(int zone, int full);
int real_zone(int vnum);
int real_room(int vnum);
int real_mobile(int vnum);
int real_object(int vnum);
int real_shop(int vnum);

#define REAL 0
#define VIRTUAL 1

struct obj_data *read_object(int nr, int type);
struct char_data *read_mobile(int nr, int type);

#define SITE_BAN_MESSAGE \
"Sorry...  Your site has been banned.  One or more members that operate from the\n\r\
site or subnet that you have logged in from has been a habitual problem causer\n\r\
and has forced us to take extreme measures to keep the peace.\n\r"

#define MENU \
"\n\r\
Welcome to Ronin Diku\n\r\n\
0) Exit from DikuMud.\n\r\
1) Enter the game.\n\r\
2) Enter description.\n\r\
3) Read the background story.\n\r\
\n\r\
   Make your choice: "


#define GREETINGS \
"\n\
             +@@@@@@@@@@\n\r\
             @@         @@\n\r\
            @@          @@\n\r\
           @@          @@\n\r\
          @@. @@@@@@@@@      @@@@@@@@@@     @@      @@    @@    @@      @@\n\r\
         @@    @            @@      @@     @@@@    @@    @@    @@@@    @@\n\r\
        @@      @@         @@      @@     @@  @@  @@    @@    @@  @@  @@\n\r\
       @@        @@       @@      @@     @@    @@@@    @@    @@    @@@@\n\r\
      @@          @@     @@@@@@@@@@     @@      @@    @@    @@      @@\n\r\
\n\r\
\n\r\
       =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r\
       || Based on DikuMUD I (GAMMA 2.5) by Hans Henrik Staerfeldt, ||\n\r\
       || Katja Nyboe, Tom Madsen, Michael Seifert, and Sebastian   ||\n\r\
       ||                        Hammer.                            ||\n\r\
       =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r\
\n\r\
If you have trouble creating a character when using a client, please try\n\r\
using straight telnet before giving up.\n\r\
\n\r"

#define WELC_MESSG \
"\n\r\
Welcome to the land of DikuMUD. May your visit here be... Interesting.\n\r\
\n\r"


#define STORY     \
"IN THE BEGINNING.....\n\r\
\n\r\
     Before the dawn of human civilization, Chaos ruled the land.\n\r\
Emptiness spanned existence, a sprawling collection of fleeting hopes.\n\r\
One day the Chaos coalesced and the four Elements: Earth, Fire, Air,\n\r\
and Water, unified to create a habitable planet.  Life was spawned at\n\r\
its Golden Age, making the planet ideal for living.  As time drifted,\n\r\
the land became increasingly corrupted.  Its taint shattered harmony\n\r\
and imperfections began to arise.  The Elements battled one another\n\r\
again and the once-peaceful humans began to steal and murder.\n\r\
\n\r\
      For many years the slaughtering continued.  Eventually, a town\n\r\
was created based on the idea of policing the towns around it,\n\r\
bringing peace and order to the world once more.  This town was given\n\r\
the name of the creator of its universe... Midgaard.  As the years\n\r\
passed, the loyal cityguards of Midgaard tried their hardest to\n\r\
maintain peace throughout the world.\n\r\
\n\r\
      But in due time, demons and monsters with incredible strength\n\r\
appeared.  By slaying these monsters, the town of Midgaard was barely\n\r\
able to quell the force of Chaos.  The increasing difficulty in\n\r\
defeating these rogue legions soon wearied the cityguards, so the city\n\r\
of Midgaard was forced to appoint a Mayor to lead them.  The Mayor's\n\r\
first act of duty was to contrive a way to bring about peace again.\n\r\
\n\r\
      After much toiling and thought, the Mayor concocted his most\n\r\
brilliant idea: to bring the strongest adventurers from across the\n\r\
land into the town of Midgaard in hopes that they would band together,\n\r\
restoring order to their world with their selfless bravery.  Only the\n\r\
most divine and gifted warriors were to be chosen.\n\r\
\n\r\
You have been chosen...\n\r\
\n\r"


/* structure for the reset commands */
struct reset_com
{
	char command;   /* current command                      */
	bool if_flag;   /* if TRUE: exe only if preceding exe'd */
	int arg1;       /*                                      */
	int arg2;       /* Arguments to the command             */
	int arg3;       /*                                      */

	/*
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
	+more
	*/
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
	char *name;             /* name of this zone                  */
	int virtual;            /* virtual index for this zone */
	int lifespan;           /* how long between resets (minutes)  */
	int age;                /* current age of this zone (minutes) */
	int top;                /* upper limit for rooms in this zone */
	int bottom;             /* lower limit for rooms in this zone */
	char *create_date;
	char *mod_date;
	int climate;
	char *creators;

      int mult_hp;
      int mult_mana;
      int mult_hitroll;
      int mult_damage;
      int mult_armor;
      int mult_xp;
      int mult_gold;
      int mult_level;

	int reset_mode;         /* conditions for reset (see below)   */
	struct reset_com *cmd;  /* command table for reset	           */

	/*
	  Reset mode:
	  0: Don't reset, and don't update age.
	  1: Reset if no PC's are located in zone.
	  2: Just reset.
    3: Block reset
    4: Lock Zone
    5: Only reset doors
	*/

	/*ubyte terrain_type;
	struct weather_info_t weather;*/
};

#define ZRESET_MODE_NEVER  0
#define ZRESET_MODE_EMPTY  1
#define ZRESET_MODE_ALWAYS 2
#define ZRESET_MODE_BLOCK  3
#define ZRESET_MODE_LOCK   4
#define ZRESET_MODE_DOORS  5

struct help_index_element
{
	char *keyword;
	long pos;
};


extern int WIZLOCK;
extern int CHAOSMODE;
extern int BAMDAY;
extern int PULSECHECK;
extern int GAMELOCK;
extern int GAMECHECK;
extern int GAMEHALT;
extern int BOOTFULL;
extern int DOUBLEXP;
extern int CHAOSDEATH;

extern struct idname_struct idname[MAX_ID];

extern struct char_data *character_list;

extern int top_of_zone_table;
extern struct zone_data *zone_table;

extern int top_of_world;
extern struct room_data *world;

extern int top_of_mobt;
extern struct mob_proto *mob_proto_table;

extern int top_of_objt;
extern struct obj_proto *obj_proto_table;
extern struct obj_data  *object_list;

extern int number_of_shops;
extern struct shop_data *shop_index;

extern int top_of_fight_messages_list;
extern struct message_list *fight_messages_list;

extern struct time_info_data time_info;
extern struct weather_data weather_info;

extern int top_of_helpt;
extern struct help_index_element *help_index;
extern char helpcontents[3*MSL];
extern FILE *help_fl;

extern int top_of_wizhelpt;
extern struct help_index_element *wizhelp_index;
extern char wizhelpcontents[3*MSL];
extern FILE *wizhelp_fl;

extern int top_of_olchelpt;
extern struct help_index_element *olchelp_index;
extern char olchelpcontents[3*MSL];
extern FILE *olchelp_fl;

extern char credits[MAX_STRING_LENGTH];
extern char heroes[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH];
extern char motd[MAX_STRING_LENGTH];
extern char newbiemotd[MAX_STRING_LENGTH];
extern char godmotd[MAX_STRING_LENGTH];
extern char help[MAX_STRING_LENGTH];
extern char info[];
extern char wizlist[];
extern char wizlist_inactive[];

extern int top_of_flying;
extern int *flying_rooms;

#endif /* _DB_H_ */
