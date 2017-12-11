/* quest.h
**
** Declares the quest/chaos module interface
*/

#ifndef _QUEST_H_
#define _QUEST_H_

struct scoreboard_data
{
  char killer[80];
  char killed[80];
  char location[80];
  char time_txt[80];
};

extern struct scoreboard_data scores[101];
extern int number_of_kills;

#endif /* _QUEST_H_ */
