
/*
$Author: ronin $
$Date: 2004/02/05 16:13:15 $
$Header: /home/ronin/cvs/ronin/memory.h,v 2.0.0.1 2004/02/05 16:13:15 ronin Exp $
$Id: memory.h,v 2.0.0.1 2004/02/05 16:13:15 ronin Exp $
$Name:  $
$Log: memory.h,v $
Revision 2.0.0.1  2004/02/05 16:13:15  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

void remember (struct char_data *ch, struct char_data *mob);
void forget (struct char_data *ch, struct char_data *mob) ;
void clearMemory(struct char_data *ch);
