/* ************************************************************************
*  file: limits.h , Limit/Gain control module             Part of DIKUMUD *
*  Usage: declaration of title type                                       *
************************************************************************* */

#ifndef _LIMITS_H_
#define _LIMITS_H_

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

#endif /* _LIMITS_H_ */
