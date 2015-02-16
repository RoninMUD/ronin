/* ************************************************************************
*  file: limits.h , Limit/Gain control module             Part of DIKUMUD *
*  Usage: declaration of title type                                       *
************************************************************************* */

/*
$Author: ronin $
$Date: 2004/02/05 16:13:11 $
$Header: /home/ronin/cvs/ronin/limits.h,v 2.0.0.1 2004/02/05 16:13:11 ronin Exp $
$Id: limits.h,v 2.0.0.1 2004/02/05 16:13:11 ronin Exp $
$Name:  $
$Log: limits.h,v $
Revision 2.0.0.1  2004/02/05 16:13:11  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


/* Public Procedures */
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
void set_title(struct char_data *ch, char *title);
void gain_condition(struct char_data *ch, int condition, int value);

struct title_type
{
	char *title_m;
	char *title_f;
};
