/*
///   Special Procedure Module                   Orig. Date    19-07-1994
///                                              Last Modified 04-09-1994
///   Spec.vote.c --- Special for voting machine
///
///   Designed and coded by Loner (loner@cs.tut.fi)
///   Using this code is not allowed without permission from originator.
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:11:32 $
$Header: /home/ronin/cvs/ronin/spec.vote.c,v 2.0.0.1 2004/02/05 16:11:32 ronin Exp $
$Id: spec.vote.c,v 2.0.0.1 2004/02/05 16:11:32 ronin Exp $
$Name:  $
$Log: spec.vote.c,v $
Revision 2.0.0.1  2004/02/05 16:11:32  ronin
Reinitialization of cvs archives

Revision 1.3  2002/04/18 18:45:54  ronin
Addition of reboot_type for normal or hotboot.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "cmd.h"

char *one_argument (char*, char*);
void argument_interpreter (char*, char*, char*);

#include "spec.vote.h"

/* --------------------------------------------------------------- */

void initialize_vote(void) {
  int i;

  for (i = 0;i<MAX_IDEA;i++) {
    strcpy (vote_info[i].player, "");
    strcpy (vote_info[i].idea, "");
    vote_info[i].v_for = 0;
    vote_info[i].against = 0;
    vote_info[i].is_used = 0;
  }
}

void
save_vote (void) {
  FILE *fl;
  char buf[MAX_INPUT_LENGTH];
  int ind=0, num=0;

  if (!(fl = fopen (VOTE_FILE, "w"))) {
    sprintf (buf, "Error in saving votes (%s)...", VOTE_FILE);
    log_f(buf);
    return;
  }

  for (ind = 0;ind < MAX_IDEA;ind++)
    if (vote_info[ind].is_used) {
      fprintf (fl, "#%d\n%s~\n%s~\n%d %d\n",
	       num, vote_info[ind].player,
	       vote_info[ind].idea,
	       vote_info[ind].v_for,
	       vote_info[ind].against);
      num++;
    }

  fprintf (fl, "#100\n");  /* The End Sign */
  fclose (fl);

  sprintf (buf, "Saved all votes to %s ...", VOTE_FILE);
  log_f(buf);
}

void read_vote (void) {
  FILE *fl;
  char buf[MAX_INPUT_LENGTH];
  char *str;
  int tmp, tmp1, tmp2;

  initialize_vote ();

  if (!(fl = fopen (VOTE_FILE, "r"))) {
    sprintf (buf, "Didn't file %s in current directory...",VOTE_FILE);
    log_f(buf);
    return;
  }

  fscanf (fl, "#%d\n", &tmp);

  while (tmp != 100 && !feof(fl)) {
    str = fread_string (fl);
    strcpy (vote_info[tmp].player, str);
    free (str);

    str = fread_string (fl);
    strcpy (vote_info[tmp].idea, str);
    free (str);

    fscanf (fl, "%d %d\n", &tmp1, &tmp2);
    vote_info[tmp].v_for = tmp1;
    vote_info[tmp].against = tmp2;
    vote_info[tmp].is_used = 1;

    fscanf (fl, "#%d\n", &tmp);
  }

  fclose (fl);

  sprintf (buf, "Reading %s to update Voting Machine...",VOTE_FILE);
  log_f(buf);
}

void
save_voters (void) {
  FILE *fl;
  char buf[MAX_INPUT_LENGTH];
  struct voters_data *tmp;

  if (!(fl = fopen (VOTERS_FILE, "w"))) {
    sprintf (buf, "Error in saving voters (%s)...", VOTERS_FILE);
    log_f(buf);
    return;
  }

  tmp = &(voters);
  tmp = tmp->next;

  for (;tmp;tmp = tmp->next)
    fprintf (fl, "%s %ld\n", tmp->player, tmp->voted_bits);

  fprintf (fl, "$$ 0\n");
  fclose (fl);

  sprintf (buf, "Saved all voters to %s ...", VOTERS_FILE);
  log_f(buf);
}

void
read_voters (void) {
  FILE *fl;
  char buf[MAX_INPUT_LENGTH];
  struct voters_data *tmp = NULL, *tmp2 = NULL;
  long vote;

  if (!(fl = fopen (VOTERS_FILE, "r"))) {
    sprintf (buf, "Didn't find %s in current directory ...", VOTERS_FILE);
    log_f(buf);
    return;
  }

  fscanf (fl, "%s %ld",buf,&vote);

  tmp2 = &voters;

  while (strcmp(buf,"$$") && !feof(fl)) {
    CREATE(tmp, struct voters_data, 1);
    strcpy (tmp->player, buf);
    tmp->voted_bits = vote;
    tmp->next = NULL;

    tmp2->next = tmp;
    tmp2 = tmp;

    fscanf (fl, "%s %ld",buf,&vote);
  }

  sprintf (buf, "Reading %s to update Voting Machine...",VOTERS_FILE);
  log_f(buf);
  fclose(fl);
}

/* --------------------------------------------------------------- */

void add_voter(char *player) {
  struct voters_data *tmp, *new_v;
  for (tmp = &(voters);tmp->next;tmp = tmp->next);

  CREATE(new_v, struct voters_data,1);
  strcpy (new_v->player, player);
  new_v->voted_bits = 0;
  new_v->next = NULL;

  tmp->next = new_v;
}

int empty_slot (void) {
  int i;
  for (i = 0; i < MAX_IDEA; i++)
    if (!vote_info[i].is_used)
      return i;
  return -1;
}

void insert_idea(CHAR *ch, char *arg) {
  int empty = empty_slot();

  if (*arg == '\0') {
    send_to_char ("Usage: idea <text> \n\r", ch);
    return;
  }

  if (empty == -1)
    send_to_char("No more slots available for ideas.\n\r",ch);
  else {
    strcpy (vote_info[empty].player,GET_NAME(ch));
    strncpy (vote_info[empty].idea, arg, 139);
    vote_info[empty].idea[139] = 0;
    vote_info[empty].v_for   = 0;
    vote_info[empty].against = 0;
    vote_info[empty].is_used = 1;
    send_to_char("OK.\n\r",ch);
  }
}

/* --------------------------------------------------------------*/

int check_list (char *player) {
  struct voters_data *tmp;
  for (tmp = &(voters);tmp;tmp = tmp->next) {
    if (!strcmp(tmp->player,player))
      return tmp->voted_bits;
  }
  return -1;
}

int check_vote(CHAR *ch,int vote) {
  long int bits = check_list(GET_NAME(ch));
  if (bits == -1) {
    add_voter(GET_NAME(ch));
    return FALSE;
  }
  else
    return ((1 << (vote-1)) & bits);       /* checking vote flag */
}

/* --------------------------------------------------------------*/

void set_voted (CHAR *ch, int number) {
  struct voters_data *tmp;
  for (tmp = &(voters);tmp;tmp = tmp->next) {
    if (!strcmp(tmp->player,GET_NAME(ch))) {
      tmp->voted_bits |= (1 << number);
    }
  }
}

void insert_vote(CHAR *ch, int number, char *arg) {
  struct voters_data *tmp;
  char item[MAX_STRING_LENGTH];

  number--;

  if (!vote_info[number].is_used) {
    send_to_char ("Nothing to vote there...\n\r",ch);
    return;
  }

  for (tmp = &(voters);tmp;tmp = tmp->next) {
    if (!strcmp(tmp->player,GET_NAME(ch))) {
      one_argument(arg,item);
      if (!strcmp(item,"yes")) {
	vote_info[number].v_for++;
      } else if (!strcmp(item,"no")) {
	vote_info[number].against++;
      } else {
	send_to_char("Voting machine tells you, 'WHAT ?!?'\n\r", ch);
	return;
      }
      set_voted (ch, number);
      send_to_char ("OK.\n\r",ch);
    }
  }
}

void idea_remove(CHAR *ch,char *arg) {
  char tmp[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  int number;
  struct voters_data *tmp_v;

  if (GET_LEVEL(ch) < LEVEL_WIZ) {
    send_to_char("Voting machine tells you, 'What are you trying to do? You should be WIZ or higher to remove ideas from machine.'\n\r",ch);
    return;
  } else {
    one_argument (arg,tmp);
    number = atoi (tmp);
    number--;

    if ((number < 0 || number > MAX_IDEA-1) || !vote_info[number].is_used) {
      send_to_char ("Voting Machine tells you, 'There's nothing to remove there.'\n\r",ch);
      return;
    }

    for (tmp_v = &(voters);tmp_v;tmp_v = tmp_v->next)
      if (tmp_v->voted_bits & (1 << number))
	tmp_v->voted_bits ^= (1 << number);

    strcpy (vote_info[number].idea, "");
    strcpy (vote_info[number].player, "");
    vote_info[number].v_for = 0;
    vote_info[number].against = 0;
    vote_info[number].is_used = 0;
    sprintf (buf, "OK. Removed idea number %d.\n\r",number+1);
    send_to_char(buf,ch);
  }
}

void vote_report(CHAR *ch, char *arg) {
  int number, i, found = FALSE;
  char foo[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  struct string_block sb;

  init_string_block (&sb);

  one_argument(arg,foo);

  if (!*foo) {
    for (i = 0; i < MAX_IDEA; i++)
      if (vote_info[i].is_used) {
	found = TRUE;
	sprintf(buf,"(%2d) : %s\n\r", i+1, vote_info[i].idea);
	if (GET_LEVEL(ch) >= LEVEL_IMM) {
	  sprintf (buf2, "Idea by %s\n\r", vote_info[i].player);
	  strcat (buf, buf2);
	}
	sprintf (buf2, "votes: for %d, against %d\n\r\n\r",
		vote_info[i].v_for, vote_info[i].against);
	strcat (buf, buf2);
	append_to_string_block(&sb, buf);
      }
    page_string_block(&sb, ch);
    destroy_string_block(&sb);


    if (!found) {
      sprintf (buf, "Nothing to report. The Machine is empty of ideas.\n\r");
      send_to_char (buf,ch);
      return;
    }
  } else {
    number = atoi(foo);

    if ((number > MAX_IDEA) || (number < 1)) {
      send_to_char("The voting machine tells you, 'There is no such thing to report.'\n\r",ch);
      return;
    }

    number--;
    if (vote_info[number].is_used) {
      sprintf(buf,"(%2d) : %s\n\rvotes: for %2d, against %2d.\n\r\n\r",
	      number+1,vote_info[number].idea,vote_info[number].v_for,
	      vote_info[number].against);
      send_to_char(buf,ch);
    } else
      send_to_char("The voting machine tells you, 'There is no subject and nothing to report.'\n\r",ch);
  }
}

/* ----------------------------------------------------------------- */

int vote (OBJ *machine, CHAR *ch, int cmd, char *arg) {
  char valid[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int number;

  if (IS_NPC(ch))
    return FALSE;

  if (cmd != CMD_USEIDEA && cmd != CMD_VOTE && cmd != CMD_REMOVE &&
      cmd != CMD_REPORT)
    return FALSE;

  if (GET_LEVEL(ch) < UNDERAGED_VOTER) {
    sprintf (buf2,"You are a too low-level, try again after level %d...\n\r",UNDERAGED_VOTER-1);
    send_to_char(buf2,ch);
    return FALSE;
  }

  switch(cmd) {
  case CMD_USEIDEA :
    insert_idea(ch,arg);
    return TRUE;

  case CMD_VOTE :
    while (*arg == ' ') arg++;
    argument_interpreter (arg, valid, buf);
    number = atoi(valid);

    if ((number > MAX_IDEA) || (number < 1)) {
      send_to_char("Voting machine tells you, 'You can't vote that!'\n\r",ch);
      return TRUE;
    }

    if (check_vote(ch,number)) {     /* check voters existance and voting */
      send_to_char("Voting machine tells you, 'You have only one vote in that subject.'\n\r",ch);
      return TRUE;
    } else {
      insert_vote (ch, number, buf);
      save_vote();
      save_voters();
      return TRUE;
    }

  case CMD_REMOVE:
    idea_remove(ch,arg);
    save_vote();
    save_voters();
    return TRUE;

  case CMD_REPORT:
    vote_report(ch,arg);
    return TRUE;
  }
  return FALSE;
}
