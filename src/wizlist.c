
/*
$Author: ronin $
$Date: 2004/02/05 16:12:02 $
$Header: /home/ronin/cvs/ronin/wizlist.c,v 2.0.0.1 2004/02/05 16:12:02 ronin Exp $
$Id: wizlist.c,v 2.0.0.1 2004/02/05 16:12:02 ronin Exp $
$Name:  $
$Log: wizlist.c,v $
Revision 2.0.0.1  2004/02/05 16:12:02  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "utility.h"

#define IMM_NAMES_I       "names-inact.imm"
#define IMM_NAMES_A       "names-act.imm"
#define DEI_NAMES_I       "names-inact.dei"
#define DEI_NAMES_A       "names-act.dei"
#define TEM_NAMES_I       "names-inact.tem"
#define TEM_NAMES_A       "names-act.tem"
#define WIZ_NAMES_I       "names-inact.wiz"
#define WIZ_NAMES_A       "names-act.wiz"
#define ETE_NAMES_I       "names-inact.ete"
#define ETE_NAMES_A       "names-act.ete"
#define SUP_NAMES_I       "names-inact.sup"
#define SUP_NAMES_A       "names-act.sup"
#define IMP_NAMES_I       "names-inact.imp"
#define IMP_NAMES_A       "names-act.imp"

#define WIZLIST_START_MSG "\
==========================================================================\n\
The following people have achieved immortality and superior powers in this\n\
realm.  They should be treated with respect, and occasional prayers are\n\
advisable.  Stealing from these people is punishable by death.\n\r"

#define WIZLIST_INACTIVE_START_MSG "\
==========================================================================\n\
The following people were once active in this realm, but have since\n\
moved on.\n\r"

#define WIZLIST_END_MSG   "\
==========================================================================\n\r"

const char *wizfiles[] = {
  IMM_NAMES_I,
  IMM_NAMES_A,
  DEI_NAMES_I,
  DEI_NAMES_A,
  TEM_NAMES_I,
  TEM_NAMES_A,
  WIZ_NAMES_I,
  WIZ_NAMES_A,
  ETE_NAMES_I,
  ETE_NAMES_A,
  SUP_NAMES_I,
  SUP_NAMES_A,
  IMP_NAMES_I,
  IMP_NAMES_A
};

int
check_wizlist_name (char *name, const char *filename) {
  char tmp_name[64];
  FILE *fl;

  if(!(fl = fopen (filename, "r"))) return 0;

  while (!feof (fl)) {
    fscanf (fl, "%s\n", tmp_name);

    if (!strcmp (name, tmp_name)) {
      fclose(fl);
      return 1;
    }
  }
  fclose(fl);
  return 0;
}

void
remove_wizlist_name (char *name) {
  FILE *fl, *real_fl;
  char buf[80];
  char tmpname[80];
  int ind;

  for (ind = 0; ind < 14; ind++) {
    sprintf(tmpname," ");
    if (!(fl = fopen (wizfiles[ind], "r")))
      return;

    real_fl = fopen ("names.tmp", "w");

    while (!feof (fl)) {
      fscanf (fl, "%s\n", tmpname);
      if (strcmp(name, tmpname))
     fprintf (real_fl, "%s\n", tmpname);
    }

    fclose (fl);
    fclose (real_fl);

    sprintf (buf, "mv names.tmp %s", wizfiles[ind]);
    system (buf);
  }
}

void
insert_wizlist_name (char *name, const char *filename) {
  FILE *fl;

  remove_wizlist_name (name);
  if (!(fl = fopen (filename, "r+"))) {
    if (!(fl = fopen(filename, "w+"))) {
      return;
    }
  }

  fseek (fl, 0L, SEEK_END);
  fprintf (fl, "%s\n", name);
  fclose (fl);
}

int
insert_char_wizlist (struct char_data *ch) {
  int plus,ind;
  if (!ch) {
    log_f("   Nothing to insert in insert_name (wizlist.c)");
    return (-1);
  }
  plus=0;
  if( IS_SET(ch->new.imm_flags, WIZ_ACTIVE) ) plus=1;
  ind=(GET_LEVEL(ch)-LEVEL_IMM)*2 + plus;
  if (!check_wizlist_name (GET_NAME(ch), wizfiles[ind])) {
    insert_wizlist_name (GET_NAME(ch), wizfiles[ind]);
    return 1;
  }

  return 0;
}

int
create_wizlist (FILE *wizlist) {
  FILE *name_fl;
  char *wiznames[] = {
    " ",
    "Immortals:\n   ",
    " ",
    "Deities:\n   ",
    " ",
    "Temporals:\n   ",
    " ",
    "Wizards:\n   ",
    " ",
    "Eternals:\n   ",
    " ",
    "Supremes:\n   ",
    " ",
    "\nImplementors:\n   "
  };

  int line_len = 0;
  int ind;
  char tmp_name[64];

  fprintf (wizlist, WIZLIST_START_MSG);

  for (ind = 0; ind < 7; ind++) {
    sprintf(tmp_name," ");
    log_f("   Read wizlist source files...");
    if (!(name_fl = fopen (wizfiles[13 - ind*2], "r")))
      continue;

    fprintf (wizlist, "%s", wiznames[13 - ind*2]);

    while (!feof (name_fl)) {
      fscanf (name_fl, "%s\n", tmp_name);

      line_len += (strlen(tmp_name) + 2);
      if (line_len > 60) {
     line_len = 0;
     fprintf (wizlist, "\n   ");
      }
      fprintf (wizlist, "%s  ", tmp_name);
    }

    line_len = 0;
    fclose (name_fl);

    fprintf (wizlist, "\n\n");
  }

  fprintf (wizlist, WIZLIST_END_MSG);
  fclose (wizlist);
  return 1;
}

int
create_inactive_wizlist (FILE *wizlist) {
  FILE *name_fl;
  char *wiznames[] = {
    "Immortals:\n   ",
    " ",
    "Deities:\n   ",
    " ",
    "Temporals:\n   ",
    " ",
    "Wizards:\n   ",
    " ",
    "Eternals:\n   ",
    " ",
    "Supremes:\n   ",
    " ",
    "\nImplementors:\n   ",
    " "
  };

  int line_len = 0;
  int ind;
  char tmp_name[64];

  fprintf (wizlist, WIZLIST_INACTIVE_START_MSG);

  for (ind = 0; ind < 7; ind++) {
    sprintf(tmp_name," ");
    log_f("   Read wizlist source files...");
    if (!(name_fl = fopen (wizfiles[12 - ind*2], "r")))
      continue;

    fprintf (wizlist, "%s", wiznames[12 - ind*2]);

    while (!feof (name_fl)) {
      fscanf (name_fl, "%s\n", tmp_name);

      line_len += (strlen(tmp_name) + 2);
      if (line_len > 60) {
        line_len = 0;
        fprintf (wizlist, "\n   ");
      }
      fprintf (wizlist, "%s  ", tmp_name);
    }

    line_len = 0;
    fclose (name_fl);

    fprintf (wizlist, "\n\n");
  }

  fprintf (wizlist, WIZLIST_END_MSG);
  fclose (wizlist);
  return 1;
}
