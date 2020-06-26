#ifndef __CHAR_SPEC_H__
#define __CHAR_SPEC_H__

void adrenaline_rush_spec(CHAR *ch, CHAR *signaler, int cmd, const char *arg);
void dirty_tricks_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg);
void blood_lust_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg);
void shadowstep_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg);
void victimize_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg);
void shadow_wraith_spec(CHAR *ch, CHAR *signaler, int cmd, const char *arg);
void taunt_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg);
void mimicry_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg);
void sidearm_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg);
void snipe_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg);

int char_spec(CHAR *ch, CHAR *signaler, int cmd, const char *arg);

#endif /* __CHAR_SPEC_H__ */
