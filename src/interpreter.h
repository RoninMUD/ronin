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
int search_block_ex(const char *string, const char *const *list, bool exact, bool case_sensitive);
int search_block(const char *string, const char * const *list, bool exact);
int old_search_block(const char *string, int begin, int length, const char * const * const list, int mode);
char *one_argument_ex(char *string, char *arg, size_t arg_size, bool include_fill_words);
void two_arguments_ex(char *string, char *arg1, size_t arg1_size, char *arg2, size_t arg2_size, bool include_fill_words);
char *one_argument(char *string, char *arg);
void two_arguments(char *string, char *arg1, char *arg2);
void argument_interpreter(char *string, char *arg1, char *arg2);
void chop_string(char *string, char *arg, size_t arg_len, char *sub, size_t sub_len);
void half_chop(char *string, char *arg, size_t arg_len, char *sub, size_t sub_len);

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
int determine_command(char *command, int length);

#endif /* __INTERPRETER_H__ */
