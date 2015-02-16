/* ************************************************************************
*  file: db.h , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars booting world.                             *
************************************************************************* */

/*
$Author: ronin $
$Date: 2004/06/02 13:40:11 $
$Header: /home/ronin/cvs/ronin/db.h,v 2.1 2004/06/02 13:40:11 ronin Exp $
$Id: db.h,v 2.1 2004/06/02 13:40:11 ronin Exp $
$Name:  $
$Log: db.h,v $
Revision 2.1  2004/06/02 13:40:11  ronin
Added mult_(stat) to zone_data.

Revision 2.0.0.1  2004/02/05 16:13:05  ronin
Reinitialization of cvs archives


Revision 5-Feb-04 Ranger
Addition of MAX_ID

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/



/* data files used by the game system */

#define DFLT_DIR          "lib"           /* default data directory     */
#define WORLD_LIST_FILE   "tinyworld.idx" /* the list of zones to include */
#define CREDITS_FILE      "credits"       /* for the 'credits' command  */
#define HEROES_FILE       "heroes"        /* for the 'heroes' command  */
#define NEWS_FILE         "news"          /* for the 'news' command     */
#define MOTD_FILE         "motd"          /* messages of today          */
#define NEWBIEMOTD_FILE   "newbiemotd"
#define GODMOTD_FILE      "godmotd"
#define TIME_FILE         "time"          /* game calendar information  */
#define IDEA_FILE         "ideas"         /* for the 'idea'-command     */
#define TYPO_FILE         "typos"         /*         'typo'             */
#define BUG_FILE          "bugs"          /*         'bug'              */
#define MESS_FILE         "messages"      /* damage message             */
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
int distribute_token(void);
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
int real_room(int virtual);
char *fread_string(FILE *fl);
int real_object(int virtual);
int real_mobile(int virtual);
int real_zone(int virtual);
void update_time(void);
int allocate_zone(int virtual_number);
int allocate_room(int virtual_number);
int allocate_obj(int virtual_number);
int allocate_mob(int virtual_number);
void renum_world(void);
void reset_zone(int zone, int full);
#define REAL 0
#define VIRTUAL 1

struct obj_data *read_object(int nr, int type);
struct char_data *read_mobile(int nr, int type);

#define MENU         \
"\n\rWelcome to Ronin Diku\n\r\n\
0) Exit from DikuMud.\n\r\
1) Enter the game.\n\r\
2) Enter description.\n\r\
3) Read the background story.\n\r\n\r\
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
"\n\rWelcome to the land of DikuMUD. May your visit here be... Interesting.\
\n\r\n\r"


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
brilliant idea:  to bring the strongest adventurers from across the\n\r\
land into the town of Midgaard in hopes that they would band together,\n\r\
restoring order to their world with their selfless bravery.  Only the\n\r\
most divine and gifted warriors were to be chosen.\n\r\
\n\r\
You have been chosen...\n\r\n\r"


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
};

struct help_index_element
{
	char *keyword;
	long pos;
};
