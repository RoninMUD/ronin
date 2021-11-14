/* ************************************************************************
*  file: limits.h , Limit/Gain control module             Part of DIKUMUD *
*  Usage: declaration of title type                                       *
************************************************************************* */

#ifndef _LIMITS_H_
#define _LIMITS_H_

/* Public Procedures */
int mana_limit(CHAR *ch);
int hit_limit(CHAR *ch);
int move_limit(CHAR *ch);
void advance_level(CHAR *ch);
void set_title(CHAR *ch, char *title);
void gain_condition(CHAR *ch, int condition, int value);

struct title_type {
	char *title_m;
	char *title_f;
};

#endif /* _LIMITS_H_ */
