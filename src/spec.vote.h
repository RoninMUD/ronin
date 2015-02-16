
/*
$Author: ronin $
$Date: 2004/02/05 16:13:28 $
$Header: /home/ronin/cvs/ronin/spec.vote.h,v 2.0.0.1 2004/02/05 16:13:28 ronin Exp $
$Id: spec.vote.h,v 2.0.0.1 2004/02/05 16:13:28 ronin Exp $
$Name:  $
$Log: spec.vote.h,v $
Revision 2.0.0.1  2004/02/05 16:13:28  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

#define MAX_IDEA 30         /* Do not change until you get a 64-bit machine */
#define UNDERAGED_VOTER 10  /* This is the minimum level, when you CAN vote */

struct vote_data {
  char player[20];               /* player_name                         */
  char idea[140];                 /* the thing they vote for and against */
  unsigned short int v_for;      /* votes for                           */
  unsigned short int against;    /* votes against                       */
  unsigned short int is_used;    /* is this vector slot used            */
};

struct voters_data {
  char player[20];                /* player_name                  */
  long int voted_bits;            /* bit vector for already voted */
  struct voters_data *next;       /* pointer to next              */
};

struct voters_file_elem {
  char player[20];                /* player_name                  */
  long int voted_bits;            /* bit vector for already voted */
};

struct vote_data vote_info[MAX_IDEA];
struct voters_data voters = { "Dummy player",0,0 };

#define VOTERS_FILE           "voters.dat"
#define VOTE_FILE             "vote.dat"

void initialize_vote(void);
void save_vote (void);
void save_voters (void);
void read_vote (void);
void read_voters (void);
void add_voter(char *player);
int empty_slot (void);
void insert_idea(CHAR *ch, char *arg);
int check_list (char *player);
int check_vote(CHAR *ch,int vote);
void set_voted (CHAR *ch, int number);
void insert_vote(CHAR *ch, int number, char *arg);
void idea_remove(CHAR *ch,char *arg);
void vote_report(CHAR *ch, char *arg);
