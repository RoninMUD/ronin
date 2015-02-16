/* ************************************************************************
*  file: modify.h                                         Part of DIKUMUD *
*  Usage: Run-time modification (by users) of game variables              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************ */

/*
$Author: ronin $
$Date: 2004/02/05 16:13:17 $
$Header: /home/ronin/cvs/ronin/modify.h,v 2.0.0.1 2004/02/05 16:13:17 ronin Exp $
$Id: modify.h,v 2.0.0.1 2004/02/05 16:13:17 ronin Exp $
$Name:  $
$Log: modify.h,v $
Revision 2.0.0.1  2004/02/05 16:13:17  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

/*

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
*/
void show_string(struct descriptor_data *d, char *input);
void string_add(struct descriptor_data *d, char *str);
#undef MAX_STR
void quad_arg(char *arg, int *type, char *name, int *field, char *string);
void do_string(struct char_data *ch, char *arg, int cmd);
void do_setskill(struct char_data *ch, char *arg, int cmd);
char *one_word(char *argument, char *first_arg );
struct help_index_element *build_help_index(FILE *fl, int *num);
void page_string(struct descriptor_data *d, char *str, int keep_internal);
void show_string(struct descriptor_data *d, char *input);
void night_watchman(void);
void check_reboot(void);
/*#define GR*/
#define NEW
#ifdef GR
int workhours();
int load(void);
char *nogames(void);
#ifdef OLD_COMA
void comatose(void);
#endif
/* emulate the game regulator */
void gr(int s);
#endif
