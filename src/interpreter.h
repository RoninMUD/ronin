/* ************************************************************************
*  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

void nanny(struct descriptor_data *d, char *arg);

void command_interpreter(struct char_data *ch, char *argument);

bool is_number(char *string);
bool is_abbrev(char *str1, char *str2);
bool is_fill_word(char *string);
char *skip_spaces(char *string);
int search_block(const char *string, const char * const *list, bool exact);
int new_search_block(const char *string, const char * const *list, bool exact, bool case_sensitive);
int old_search_block(const char *string, int begin, int length, const char * const * const list, int mode);
char *one_argument(char *string, char *arg1);
void argument_interpreter(char *string, char *arg1, char *arg2);
void two_arguments(char *string, char *arg1, char *arg2);
void half_chop(char *string, char *arg1, int len1, char *substring, int substring_len);
void chop_string(char *string, char *arg1, int len1, char *substring, int substring_len);

struct command_info {
  void(*command_pointer) (struct char_data *ch, char *argument, int cmd);
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
