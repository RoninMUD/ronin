#ifndef __AFF_ENCH_H__
#define __AFF_ENCH_H__

void aff_modify_char(CHAR *ch, int modifier, int location, long bitvector, long bitvector2, bool add);
void aff_total_char(CHAR *ch);

AFF *aff_dup(AFF *aff);
AFF *aff_get_from_char(CHAR *ch, int type);
bool aff_affected_by(CHAR *ch, int type);
bool aff_affected_by_aff(CHAR *ch, AFF *aff);
int aff_duration(CHAR *ch, int type);
void aff_to_char(CHAR *ch, AFF *aff);
void aff_join(CHAR *ch, AFF *aff, bool avg_dur, bool avg_mod);
void aff_apply(CHAR *ch, int type, sh_int duration, sbyte modifier, byte location, long bitvector, long bitvector2);
void aff_remove(CHAR *ch, AFF *aff);
void aff_remove_all(CHAR *ch);
void aff_from_char(CHAR *ch, int type);

ENCH *ench_dup(ENCH *ench);
ENCH *ench_get_from_global(const char *name, int type);
ENCH *ench_get_from_char(CHAR *ch, const char *name, int type);
bool ench_enchanted_by(CHAR *ch, const char *name, int type);
bool ench_enchanted_by_ench(CHAR *ch, ENCH *ench);
int ench_duration(CHAR *ch, const char *name, int type);
void ench_to_char(CHAR *ch, ENCH *ench, bool overwrite);
void ench_apply(CHAR *ch, bool overwrite, const char *name, int type, sh_int duration, byte interval, int modifier, byte location, long bitvector, long bivector2, int(*func)(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg));
void ench_remove(CHAR *ch, ENCH *ench, bool to_log);
void ench_remove_all(CHAR *ch, bool to_log);
void ench_from_char(CHAR *ch, const char *name, int type, bool to_log);

#endif /* __AFF_ENCH_H__ */
