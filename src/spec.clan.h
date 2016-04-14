
/*
$Author: ronin $
$Date: 2004/02/05 16:13:23 $
$Header: /home/ronin/cvs/ronin/spec.clan.h,v 2.0.0.1 2004/02/05 16:13:23 ronin Exp $
$Id: spec.clan.h,v 2.0.0.1 2004/02/05 16:13:23 ronin Exp $
$Name:  $
$Log: spec.clan.h,v $
Revision 2.0.0.1  2004/02/05 16:13:23  ronin
Reinitialization of cvs archives

Revision 1.3  2003/01/18 00:42:15  ronin
Tentatively changing MAX_CLANS to 30, we're over 20.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

#define MAX_CLANS 40

struct clan_data
{
	char *name;
	long monthly_rent;
	int paid_to;
  char *head1;
  char *head2;
  char *head3;
  char *head4;
  char *head5;
};

void load_clan_file(void);
void save_clan_file(void);
void list_clan_file(struct char_data *ch,int clan);
int check_clan_board(int board,struct char_data *ch);
int check_clan_access(int room,struct char_data *ch);

