
/* ************************************************************************
*  File: reception.h, Special module for Inn's.           Part of DIKUMUD *
*  Usage: Procedures handling saving/loading of player objects            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2005/04/27 17:13:30 $
$Header: /home/ronin/cvs/ronin/reception.h,v 2.3 2005/04/27 17:13:30 ronin Exp $
$Id: reception.h,v 2.3 2005/04/27 17:13:30 ronin Exp $
$Name:  $
$Log: reception.h,v $
Revision 2.3  2005/04/27 17:13:30  ronin
Minor changes needed to compile on Slackware 10 for the new machine.

Revision 2.2  2005/01/21 14:55:28  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.1  2004/03/04 17:23:58  ronin
Addition of object file version 2 which includes 8 ownerid fields
for addition of some objects only being able to be used by those
owners.

Revision 2.0.0.1  2004/02/05 16:13:20  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


struct obj_cost
    {
    int total_cost;
    int no_carried;
    bool ok;
    } ;
int char_version(FILE *fl);
void store_to_char_5(struct char_file_u_5 *st, struct char_data *ch);
void store_to_char_4(struct char_file_u_4 *st, struct char_data *ch);
void store_to_char_2(struct char_file_u_2 *st, struct char_data *ch);
void char_to_store(struct char_data *ch, struct char_file_u_5 *st);
void clear_char(struct char_data *ch);
int total_cost_of_obj(struct obj_data *obj);
void add_obj_cost(struct char_data *ch, struct obj_data *obj,
                  struct obj_cost *cost);
bool recep_offer(struct char_data *ch,	struct char_data *receptionist,
		 struct obj_cost *cost);
int test_char(char *name, char *pwd);
char *string_to_lower(char *string);
void load_char(struct char_data *ch);
void strip_char(struct char_data *ch);
void obj_to_store(struct obj_data *obj, FILE *fl, struct char_data * ch, char pos, bool includeNoRent);
int cost_need(struct obj_cost *cost);
void save_char(struct char_data *ch, sh_int load_room);
int receptionist(struct char_data *r,struct char_data *ch, int cmd, char *arg);
void auto_rent(struct char_data *ch);
void autorent_all(void);
void do_crent(struct char_data *ch, char *argument, int cmd);
struct obj_data *store_to_obj_ver3(FILE *fl,struct char_data *ch);
struct obj_data *store_to_obj_ver2(FILE *fl,struct char_data *ch);
struct obj_data *store_to_obj_ver1(FILE *fl,struct char_data *ch);
struct obj_data *store_to_obj_ver0(FILE *fl,struct char_data *ch);
int obj_version(FILE *fl);
