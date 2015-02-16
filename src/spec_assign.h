
/*
$Author: ronin $
$Date: 2004/02/05 16:13:36 $
$Header: /home/ronin/cvs/ronin/spec_assign.h,v 2.0.0.1 2004/02/05 16:13:36 ronin Exp $
$Id: spec_assign.h,v 2.0.0.1 2004/02/05 16:13:36 ronin Exp $
$Name:  $
$Log: spec_assign.h,v $
Revision 2.0.0.1  2004/02/05 16:13:36  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/

void assign_mob(int virtual, int (*func)(CHAR*,CHAR*, int, char*));
void assign_obj(int virtual, int (*func)(OBJ*, CHAR*,int, char*));
void assign_room(int virtual,int (*func)(int, CHAR*,int, char*));
