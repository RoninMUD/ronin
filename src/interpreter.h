/* ************************************************************************
*  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

/*
$Author: ronin $
$Date: 2004/02/05 16:13:10 $
$Header: /home/ronin/cvs/ronin/interpreter.h,v 2.0.0.1 2004/02/05 16:13:10 ronin Exp $
$Id: interpreter.h,v 2.0.0.1 2004/02/05 16:13:10 ronin Exp $
$Name:  $
$Log: interpreter.h,v $
Revision 2.0.0.1  2004/02/05 16:13:10  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

void command_interpreter(struct char_data *ch, char *argument);
int search_block(char *arg, char **list, bool exact);
int old_search_block(char *argument,int begin,int length,char **list,int mode);
char lower( char c );
void argument_interpreter(char *argument, char *first_arg, char *second_arg);
char *one_argument(char *argument,char *first_arg);
int fill_word(char *argument);
void half_chop(char *string, char *arg1,int len1,char *arg2,int len2);
void nanny(struct descriptor_data *d, char *arg);
int is_abbrev(char *arg1, char *arg2);
int is_number(char *str);
int is_big_number(char *str);

struct command_info
{
  void (*command_pointer) (struct char_data *ch, char *argument, int cmd);
  ubyte minimum_position;
  ubyte minimum_level;
        char *cmd_text;
        int  num;
};

#endif /* __INTERPRETER_H__ */
