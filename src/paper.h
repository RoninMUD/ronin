/*
 * board.c - Patched board.c
 *
 */

/*
$Author: ronin $
$Date: 2004/02/05 16:13:19 $
$Header: /home/ronin/cvs/ronin/paper.h,v 2.0.0.1 2004/02/05 16:13:19 ronin Exp $
$Id: paper.h,v 2.0.0.1 2004/02/05 16:13:19 ronin Exp $
$Name:  $
$Log: paper.h,v $
Revision 2.0.0.1  2004/02/05 16:13:19  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/



void paper_write_msg(struct char_data *ch, char *arg);
void paper_reset_board();
void board_save_paper();
int paper_remove_msg(struct char_data *ch, char *arg);
void board_load_paper();
int newspaperboy(struct char_data *p,struct char_data *ch, int cmd, char *arg);
int paperboard(struct obj_data *p,struct char_data *ch, int cmd, char *arg);
void paper_write_msg(struct char_data *ch, char *arg);
int paper_remove_msg(struct char_data *ch, char *arg);
int paper_display_msg(struct char_data *ch, char *arg);
int paper_show_board(struct char_data *ch, char *arg);
