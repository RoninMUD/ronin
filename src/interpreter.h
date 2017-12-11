/* ************************************************************************
*  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

void command_interpreter(struct char_data *ch, char *argument);
int search_block(const char *arg, const char * const *list, bool exact);
int old_search_block(const char *argument,int begin,int length, const char * const * const list,int mode);
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

extern int max_connects;
extern int total_connects;
extern char *unknownCMD[];
extern struct command_info cmd_info[];
extern char last_command[MSL];

#endif /* __INTERPRETER_H__ */
