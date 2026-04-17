#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "structs.h"
#include "utility.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "enchant.h"

#include "aff_ench.h"

#define INC_SCHAR(orig, amount) \
do { \
  if ((orig > 0) && ((SCHAR_MAX - orig) < amount)) orig = SCHAR_MAX; \
  else if ((orig < 0) && ((SCHAR_MIN - orig) > amount)) orig = SCHAR_MIN; \
  else orig += amount; \
} while (0)

extern ENCH *enchantments;

struct aff_priority_t {
  int type;
};

struct ench_priority_t {
  char *name;
  int type;
};

const struct aff_priority_t aff_priority_table[] = {
};

const struct ench_priority_t ench_priority_table[] = {
  { "Mantra", SKILL_MANTRA }
};

//Set minimum and maximum values for skills.  This will stop looping with gear.  
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
//GET_LEARNED(ch, SKILL_BACKSTAB) = CLAMP(GET_LEARNED(ch, SKILL_BACKSTAB) + modifier, 0, 127);


int calculate_new_skill_value(int old_val,int modifier,bool wasNegative){
	
	int new_val = old_val + modifier;

	/* Always cap upper bound */
	if (new_val > 127)
		new_val = 127;

	/* Conditional floor logic */
	if (old_val > 85 && new_val < 85){
		new_val = 85;
	}
	else if (old_val < 85 && new_val < 0){
		new_val = 0;
	}
	//Add a check to ensure that negative modifiers dont give additional boosts.
	
	//If was Negative is true, it means its trying to add values.   
	//If the current value is 85, and the modifier puts it above, then cap to 85	
	
	if(wasNegative){
		new_val = old_val;
	}
	return new_val;
	
}

void aff_modify_char(CHAR *ch, int modifier, int location, long bitvector, long bitvector2, bool add) {
  bool wasNegative = FALSE;
  if (add) {
    SET_BIT(GET_AFF(ch), bitvector);
    SET_BIT(GET_AFF2(ch), bitvector2);
	if(modifier < 0){
		wasNegative = TRUE;
	}
  }
  else {
    REMOVE_BIT(GET_AFF(ch), bitvector);
    REMOVE_BIT(GET_AFF2(ch), bitvector2);
    
    if(modifier < 0){
		wasNegative = TRUE;
	}
	
	modifier = -modifier;

  }

  switch (location) {
    case APPLY_STR:
      if (GET_STR(ch) >= 30 && modifier > 0) {modifier = 0;}
      INC_SCHAR(GET_STR(ch), modifier);
      break;
    case APPLY_DEX:
      GET_DEX(ch) += modifier;
      break;
    case APPLY_CON:
      GET_CON(ch) += modifier;
      break;
    case APPLY_INT:
      GET_INT(ch) += modifier;
      break;
    case APPLY_WIS:
      GET_WIS(ch) += modifier;
      break;
    
	//Clamping values to ensure we dont hit the max allowed value.
    case APPLY_MANA:
      //GET_MAX_MANA_POINTS(ch) += modifier;
	  GET_MAX_MANA_POINTS(ch) = CLAMP(GET_MAX_MANA_POINTS(ch) + modifier, 0, 32760);
      break;
    case APPLY_HIT:
      //GET_MAX_HIT_POINTS(ch) += modifier;
	  GET_MAX_HIT_POINTS(ch) = CLAMP(GET_MAX_HIT_POINTS(ch) + modifier, 0, 32760);
      break;
    case APPLY_MOVE:
      //GET_MAX_MOVE_POINTS(ch) += modifier;
	  GET_MAX_MOVE_POINTS(ch) = CLAMP(GET_MAX_MOVE_POINTS(ch)+ modifier, 0, 32760);
      break;

    case APPLY_ARMOR:
      GET_AC(ch) += modifier;
      break;

    case APPLY_HITROLL:
      GET_HITROLL(ch) += modifier;
      break;
    case APPLY_DAMROLL:
      GET_DAMROLL(ch) += modifier;
      break;

    case APPLY_SAVING_ALL:
      GET_SAVING_THROW(ch, SAVING_PARA) += modifier;
      GET_SAVING_THROW(ch, SAVING_ROD) += modifier;
      GET_SAVING_THROW(ch, SAVING_PETRI) += modifier;
      GET_SAVING_THROW(ch, SAVING_BREATH) += modifier;
      GET_SAVING_THROW(ch, SAVING_SPELL) += modifier;
      break;
    case APPLY_SAVING_PARA:
      GET_SAVING_THROW(ch, SAVING_PARA) += modifier;
      break;
    case APPLY_SAVING_ROD:
      GET_SAVING_THROW(ch, SAVING_ROD) += modifier;
      break;
    case APPLY_SAVING_PETRI:
      GET_SAVING_THROW(ch, SAVING_PETRI) += modifier;
      break;
    case APPLY_SAVING_BREATH:
      GET_SAVING_THROW(ch, SAVING_BREATH) += modifier;
      break;
    case APPLY_SAVING_SPELL:
      GET_SAVING_THROW(ch, SAVING_SPELL) += modifier;
      break;

    case APPLY_AGE:
      GET_BIRTH(ch) -= ((long)SECS_PER_MUD_YEAR * (long)modifier);
      break;
  }

  if (GET_SKILLS(ch)) {
	  switch (location) {
		case APPLY_SKILL_SNEAK:
		  GET_LEARNED(ch, SKILL_SNEAK) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_SNEAK),modifier,wasNegative);
		  break;
		case APPLY_SKILL_HIDE:
		  GET_LEARNED(ch, SKILL_HIDE) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_HIDE),modifier,wasNegative);
		  break;
		case APPLY_SKILL_STEAL:
		  GET_LEARNED(ch, SKILL_STEAL) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_STEAL),modifier,wasNegative);
		  break;
		case APPLY_SKILL_BACKSTAB:
		  GET_LEARNED(ch, SKILL_BACKSTAB) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_BACKSTAB),modifier,wasNegative);
		  break;
		case APPLY_SKILL_PICK_LOCK:
		  GET_LEARNED(ch, SKILL_PICK_LOCK) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_PICK_LOCK),modifier,wasNegative);
		  break;
		case APPLY_SKILL_KICK:
		  GET_LEARNED(ch, SKILL_KICK) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_KICK),modifier,wasNegative);
		  break;
		case APPLY_SKILL_BASH:
		  GET_LEARNED(ch, SKILL_BASH) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_BASH),modifier,wasNegative);
		  break;
		case APPLY_SKILL_RESCUE:
		  GET_LEARNED(ch, SKILL_RESCUE) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_RESCUE),modifier,wasNegative);
		  break;
		case APPLY_SKILL_BLOCK:
		  GET_LEARNED(ch, SKILL_BLOCK) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_BLOCK),modifier,wasNegative);
		  break;
		case APPLY_SKILL_KNOCK:
		  GET_LEARNED(ch, SKILL_KNOCK) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_KNOCK),modifier,wasNegative);
		  break;
		case APPLY_SKILL_PUNCH:
		  GET_LEARNED(ch, SKILL_PUNCH) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_PUNCH),modifier,wasNegative);
		  break;
		case APPLY_SKILL_PARRY:
		  GET_LEARNED(ch, SKILL_PARRY) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_PARRY),modifier,wasNegative);
		  break;
		case APPLY_SKILL_DUAL:
		  GET_LEARNED(ch, SKILL_DUAL) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_DUAL),modifier,wasNegative);
		  break;
		case APPLY_SKILL_THROW:
		  GET_LEARNED(ch, SKILL_THROW) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_THROW),modifier,wasNegative);
		  break;
		case APPLY_SKILL_DODGE:		  
		   GET_LEARNED(ch, SKILL_DODGE) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_DODGE),modifier,wasNegative);
		  break;
		case APPLY_SKILL_PEEK:
		  GET_LEARNED(ch, SKILL_PEEK) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_PEEK),modifier,wasNegative);
		  break;
		case APPLY_SKILL_BUTCHER:
		  GET_LEARNED(ch, SKILL_BUTCHER) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_BUTCHER),modifier,wasNegative);
		  break;
		case APPLY_SKILL_TRAP:
		  GET_LEARNED(ch, SKILL_TRAP) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_TRAP),modifier,wasNegative);
		  break;
		case APPLY_SKILL_DISARM:
		  GET_LEARNED(ch, SKILL_DISARM) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_DISARM),modifier,wasNegative);
		  break;
		case APPLY_SKILL_SUBDUE:
		  GET_LEARNED(ch, SKILL_SUBDUE) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_SUBDUE),modifier,wasNegative);
		  break;
		case APPLY_SKILL_CIRCLE:
		  GET_LEARNED(ch, SKILL_CIRCLE) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_CIRCLE),modifier,wasNegative);
		  break;
		case APPLY_SKILL_TRIPLE:
		  GET_LEARNED(ch, SKILL_TRIPLE) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_TRIPLE),modifier,wasNegative);
		  break;
		case APPLY_SKILL_PUMMEL:
		  GET_LEARNED(ch, SKILL_PUMMEL) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_PUMMEL),modifier,wasNegative);
		  break;
		case APPLY_SKILL_AMBUSH:
		  GET_LEARNED(ch, SKILL_AMBUSH) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_AMBUSH),modifier,wasNegative);
		  break;
		case APPLY_SKILL_ASSAULT:
		  GET_LEARNED(ch, SKILL_ASSAULT) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_ASSAULT),modifier,wasNegative);
		  break;
		case APPLY_SKILL_DISEMBOWEL:
		  GET_LEARNED(ch, SKILL_DISEMBOWEL) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_DISEMBOWEL),modifier,wasNegative);
		  break;
		case APPLY_SKILL_TAUNT:
		  GET_LEARNED(ch, SKILL_TAUNT) = calculate_new_skill_value(GET_LEARNED(ch, SKILL_TAUNT),modifier,wasNegative);
		  break;
	  }
	}
}

void aff_total_char(CHAR *ch) {
  for (int i = 0; i < MAX_WEAR; i++) {
    if (EQ(ch, i)) {
      for (int j = 0; j < MAX_OBJ_AFFECT; j++) {
        aff_modify_char(ch, OBJ_AFF_MOD(EQ(ch, i), j), OBJ_AFF_LOC(EQ(ch, i), j), OBJ_BITS(EQ(ch, i)), OBJ_BITS2(EQ(ch, i)), FALSE);
      }
    }
  }

  for (AFF *aff = ch->affected; aff; aff = aff->next) {
    aff_modify_char(ch, aff->modifier, aff->location, aff->bitvector, aff->bitvector2, FALSE);
  }

  for (ENCH *ench = ch->enchantments; ench; ench = ench->next) {
    aff_modify_char(ch, ench->modifier, ench->location, ench->bitvector, ench->bitvector2, FALSE);
  }

  GET_TMP_ABILITIES(ch) = GET_ABILITIES(ch);

  GET_OHIT(ch) = GET_MAX_HIT_POINTS(ch);
  GET_OMANA(ch) = GET_MAX_MANA_POINTS(ch);
  GET_OMOVE(ch) = GET_MAX_MOVE_POINTS(ch);

  for (int i = 0; i < MAX_WEAR; i++) {
    if (EQ(ch, i)) {
      for (int j = 0; j < MAX_OBJ_AFFECT; j++) {
        aff_modify_char(ch, OBJ_AFF_MOD(EQ(ch, i), j), OBJ_AFF_LOC(EQ(ch, i), j), OBJ_BITS(EQ(ch, i)), OBJ_BITS2(EQ(ch, i)), TRUE);
      }
    }
  }

  for (AFF *aff = ch->affected; aff; aff = aff->next) {
    aff_modify_char(ch, aff->modifier, aff->location, aff->bitvector, aff->bitvector2, TRUE);
  }

  for (ENCH *ench = ch->enchantments; ench; ench = ench->next) {
    aff_modify_char(ch, ench->modifier, ench->location, ench->bitvector, ench->bitvector2, TRUE);
  }

  if (GET_STR(ch) > 18) {
    if (GET_OSTR(ch) <= 18) {
      int i = GET_ADD(ch) + ((GET_STR(ch) - 18) * 10);

      GET_STR(ch) = 18;
      GET_ADD(ch) = MIN(i, 100);
    }
    else {
      GET_STR(ch) = MIN(GET_STR(ch), GET_OSTR(ch));
    }
  }

  if (GET_STR(ch) < 18) {
    int i = GET_ADD(ch) / 10;

    GET_STR(ch) += i;
    GET_ADD(ch) -= i * 10;

    if (GET_STR(ch) > 18) {
      i = GET_ADD(ch) + ((GET_STR(ch) - 18) * 10);

      GET_STR(ch) = 18;
      GET_ADD(ch) = MIN(i, 100);
    }
  }

  if (GET_ODEX(ch) <= 18) {
    GET_DEX(ch) = MAX(0, MIN(GET_DEX(ch), 18));
  }
  else {
    GET_DEX(ch) = MAX(0, MIN(GET_DEX(ch), GET_ODEX(ch)));
  }

  if (GET_OCON(ch) <= 18) {
    GET_CON(ch) = MAX(0, MIN(GET_CON(ch), 18));
  }
  else {
    GET_CON(ch) = MAX(0, MIN(GET_CON(ch), GET_OCON(ch)));
  }

  if (GET_OINT(ch) <= 18) {
    GET_INT(ch) = MAX(0, MIN(GET_INT(ch), 18));
  }
  else {
    GET_INT(ch) = MAX(0, MIN(GET_INT(ch), GET_OINT(ch)));
  }

  if (GET_OWIS(ch) <= 18) {
    GET_WIS(ch) = MAX(0, MIN(GET_WIS(ch), 18));
  }
  else {
    GET_WIS(ch) = MAX(0, MIN(GET_WIS(ch), GET_OWIS(ch)));
  }
}

AFF *aff_dup(AFF *aff) {
  if (!aff) return NULL;

  AFF *dup_aff;

  CREATE(dup_aff, AFF, 1);

  dup_aff->type = aff->type;
  dup_aff->duration = aff->duration;
  dup_aff->modifier = aff->modifier;
  dup_aff->location = aff->location;
  dup_aff->bitvector = aff->bitvector;
  dup_aff->bitvector2 = aff->bitvector2;

  return dup_aff;
}

AFF *aff_get_from_char(CHAR *ch, int type) {
  if (!ch || !type) return NULL;

  AFF *aff = NULL;

  for (AFF *temp_aff = ch->affected; !aff && temp_aff; temp_aff = temp_aff->next) {
    if (temp_aff->type == type) {
      aff = temp_aff;
    }
  }

  return aff;
}

bool aff_affected_by(CHAR *ch, int type) {
  if (!ch || !type) return FALSE;

  return aff_get_from_char(ch, type) ? TRUE : FALSE;
}

bool aff_affected_by_aff(CHAR *ch, AFF *aff) {
  if (!ch || !aff) return FALSE;

  bool has_aff = FALSE;

  for (AFF *temp_aff = ch->affected; !has_aff && temp_aff; temp_aff = temp_aff->next) {
    if (temp_aff == aff) {
      has_aff = TRUE;
    }
  }

  return has_aff;
}

int aff_duration(CHAR *ch, int type) {
  if (!ch || !type) return 0;

  int duration = 0;

  AFF *aff = aff_get_from_char(ch, type);

  if (aff) {
    duration = aff->duration;
  }

  return duration;
}

int aff_calc_priority(int type) {
  int priority = 0;

  for (int i = 0; i < NUMELEMS(aff_priority_table); i++) {
    if (!((type > 0) && (aff_priority_table[i].type == type))) {
      priority++;
    }
  }

  return priority;
}

void aff_to_char(CHAR *ch, AFF *aff) {
  if (!ch || !aff || !aff->type) return;

  AFF *new_aff = aff_dup(aff);

  if (!ch->affected) {
    ch->affected = new_aff;
  }
  else {
    AFF *temp_aff = ch->affected, *prev_aff = NULL;

    int new_aff_priority = aff_calc_priority(new_aff->type);

    while (temp_aff && (aff_calc_priority(temp_aff->type) <= new_aff_priority)) {
      prev_aff = temp_aff;
      temp_aff = temp_aff->next;
    }

    if (prev_aff) {
      prev_aff->next = new_aff;
      new_aff->next = temp_aff;
    }
    else {
      new_aff->next = ch->affected;
      ch->affected = new_aff;
    }
  }

  aff_modify_char(ch, new_aff->modifier, new_aff->location, new_aff->bitvector, new_aff->bitvector2, TRUE);

  aff_total_char(ch);

  check_equipment(ch);
}

void aff_join(CHAR *ch, AFF *aff, bool avg_dur, bool avg_mod) {
  if (!ch || !aff) return;

  AFF *existing_aff = aff_get_from_char(ch, aff->type);

  if (existing_aff) {
    INC_SCHAR(aff->duration, existing_aff->duration);

    if (avg_dur) {
      aff->duration /= 2;
    }

    INC_SCHAR(aff->modifier, existing_aff->modifier);

    if (avg_mod) {
      aff->modifier /= 2;
    }

    aff_remove(ch, existing_aff);
  }

  aff_to_char(ch, aff);
}

void aff_apply(CHAR *ch, int type, sh_int duration, sbyte modifier, byte location, long bitvector, long bitvector2) {
  if (!ch) return;

  AFF aff = { 0 };

  aff.type = type;
  aff.duration = duration;
  aff.modifier = modifier;
  aff.location = location;
  aff.bitvector = bitvector;
  aff.bitvector2 = bitvector2;

  aff_to_char(ch, &aff);
}

void aff_remove(CHAR *ch, AFF *aff) {
  if (!ch || !aff || !(ch->affected)) return;

  if (aff_affected_by_aff(ch, aff)) {
    aff_modify_char(ch, aff->modifier, aff->location, aff->bitvector, aff->bitvector2, FALSE);

    if (ch->affected == aff) {
      ch->affected = aff->next;
    }
    else {
      AFF *aff_ptr = ch->affected;

      while (aff_ptr->next && (aff_ptr->next != aff)) {
        aff_ptr = aff_ptr->next;
      }

      aff_ptr->next = aff->next;
    }

    DESTROY(aff);

    aff_total_char(ch);

    check_equipment(ch);
  }
}

void aff_remove_all(CHAR *ch) {
  if (!ch) return;

  for (AFF *temp_aff = ch->affected, *next_aff; temp_aff; temp_aff = next_aff) {
    next_aff = temp_aff->next;

    aff_remove(ch, temp_aff);
  }
}

void aff_from_char(CHAR *ch, int type) {
  if (!ch || !type) return;

  for (AFF *temp_aff = ch->affected, *next_aff; temp_aff; temp_aff = next_aff) {
    next_aff = temp_aff->next;

    if (temp_aff->type == type) {
      aff_remove(ch, temp_aff);
    }
  }
}

ENCH *ench_dup(ENCH *ench) {
  if (!ench || !ench->name || !(*ench->name)) return NULL;

  ENCH *dup_ench;

  CREATE(dup_ench, ENCH, 1);

  dup_ench->type = ench->type;
  dup_ench->duration = ench->duration;
  dup_ench->modifier = ench->modifier;
  dup_ench->location = ench->location;
  dup_ench->bitvector = ench->bitvector;
  dup_ench->bitvector2 = ench->bitvector2;

  dup_ench->name = strdup(ench->name);
  dup_ench->interval = ench->interval;

  for (int i = 0; (i < NUMELEMS(dup_ench->temp)) && (i < NUMELEMS(ench->temp)); i++) {
    dup_ench->temp[i] = ench->temp[i];
  }

  if (ench->metadata) {
    dup_ench->metadata = strdup(ench->metadata);
  }

  dup_ench->func = ench->func;

  return dup_ench;
}

ENCH *ench_get_from_global(const char *name, int type) {
  if (!enchantments) return NULL;

  ENCH *ench = NULL;

  if (name && *name) {
    for (int i = 0; !ench && (i < TOTAL_ENCHANTMENTS); i++) {
      if (enchantments[i].name && !strcasecmp(enchantments[i].name, name)) {
        ench = &enchantments[i];
      }
    }
  }
  else if ((type > 0) && (type < TOTAL_ENCHANTMENTS) && enchantments[type].name) {
    ench = &enchantments[type];
  }

  return ench;
}

ENCH *ench_get_from_char(CHAR *ch, const char *name, int type) {
  if (!ch || !(ch->enchantments)) return NULL;

  ENCH *ench = NULL;

  if (name && *name) {
    for (ENCH *temp_ench = ch->enchantments; !ench && temp_ench; temp_ench = temp_ench->next) {
      if (temp_ench->name && !strcasecmp(temp_ench->name, name)) {
        ench = temp_ench;
      }
    }
  }
  else if (type >= 0) {
    for (ENCH *temp_ench = ch->enchantments; !ench && temp_ench; temp_ench = temp_ench->next) {
      if (temp_ench->type == type) {
        ench = temp_ench;
      }
    }
  }

  return ench;
}

bool ench_enchanted_by(CHAR *ch, const char *name, int type) {
  if (!ch) return FALSE;

  return ench_get_from_char(ch, name, type) ? TRUE : FALSE;
}

bool ench_enchanted_by_ench(CHAR *ch, ENCH *ench) {
  if (!ch || !ench) return FALSE;

  bool has_ench = FALSE;

  for (ENCH *temp_ench = ch->enchantments; !has_ench && temp_ench; temp_ench = temp_ench->next) {
    if (temp_ench == ench) {
      has_ench = TRUE;
    }
  }

  return has_ench;
}

int ench_duration(CHAR *ch, const char *name, int type) {
  if (!ch) return 0;

  int duration = 0;

  ENCH *ench = ench_get_from_char(ch, name, type);

  if (ench) {
    duration = ench->duration;
  }

  return duration;
}

int ench_calc_priority(const char *name, int type) {
  int priority = 0;

  for (int i = 0; i < NUMELEMS(ench_priority_table); i++) {
    if (!(name && *name && !strcasecmp(ench_priority_table[i].name, name)) && !((type > 0) && (ench_priority_table[i].type == type))) {
      priority++;
    }
  }

  return priority;
}

void ench_to_char(CHAR *ch, ENCH *ench, bool overwrite) {
  if (!ch || !ench || !ench->name || !(*ench->name)) return;

  ENCH *existing_ench = ench_get_from_char(ch, ench->name, ench->type);

  if (existing_ench && !overwrite) return;

  if (overwrite) {
    ench_remove(ch, existing_ench, FALSE);
  }

  ENCH *new_ench = ench_dup(ench);

  if (!ch->enchantments) {
    ch->enchantments = new_ench;
  }
  else {
    ENCH *temp_ench = ch->enchantments, *prev_ench = NULL;

    int new_ench_priority = ench_calc_priority(new_ench->name, new_ench->type);

    while (temp_ench && (ench_calc_priority(temp_ench->name, temp_ench->type) <= new_ench_priority)) {
      prev_ench = temp_ench;
      temp_ench = temp_ench->next;
    }

    if (prev_ench) {
      prev_ench->next = new_ench;
      new_ench->next = temp_ench;
    }
    else {
      new_ench->next = ch->enchantments;
      ch->enchantments = new_ench;
    }
  }

  if (!ench_get_from_global(new_ench->name, new_ench->type)) {
    new_ench->type = 0;
  }

  aff_modify_char(ch, new_ench->modifier, new_ench->location, new_ench->bitvector, new_ench->bitvector2, TRUE);

  aff_total_char(ch);

  check_equipment(ch);
}

void ench_apply(CHAR *ch, bool overwrite, const char *name, int type, sh_int duration, byte interval, int modifier, byte location, long bitvector, long bitvector2, int(*func)(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg)) {
  if (!ch || !name || !(*name)) return;

  ENCH ench = { 0 };

  ench.name = strdup(name);
  ench.type = type;
  ench.duration = duration;
  ench.interval = interval;
  ench.modifier = modifier;
  ench.location = location;
  ench.bitvector = bitvector;
  ench.bitvector2 = bitvector2;
  ench.func = func;

  ench_to_char(ch, &ench, overwrite);

  free(ench.name);
}

void ench_remove(CHAR *ch, ENCH *ench, bool to_log) {
  if (!ch || !ench || !(ch->enchantments)) return;

  if (ench_enchanted_by_ench(ch, ench)) {
    if (to_log) {
      log_f("PLRINFO: %s just had enchantment %s removed.", GET_DISP_NAME(ch), ench->name);
    }

    aff_modify_char(ch, ench->modifier, ench->location, ench->bitvector, ench->bitvector2, FALSE);

    if (ch->enchantments == ench) {
      ch->enchantments = ench->next;
    }
    else {
      ENCH *ench_ptr = ch->enchantments;

      while (ench_ptr->next && (ench_ptr->next != ench)) {
        ench_ptr = ench_ptr->next;
      }

      ench_ptr->next = ench->next;
    }

    DESTROY(ench->name);
    DESTROY(ench->metadata);
    DESTROY(ench);

    aff_total_char(ch);

    check_equipment(ch);
  }
}

void ench_remove_all(CHAR *ch, bool to_log) {
  if (!ch) return;

  for (ENCH *temp_ench = ch->enchantments, *next_ench; temp_ench; temp_ench = next_ench) {
    next_ench = temp_ench->next;

    ench_remove(ch, temp_ench, to_log);
  }
}

void ench_from_char(CHAR *ch, const char *name, int type, bool to_log) {
  if (!ch || ((!name || !(*name)) && !type)) return;

  for (ENCH *temp_ench = ch->enchantments, *next_ench; temp_ench; temp_ench = next_ench) {
    next_ench = temp_ench->next;

    if ((temp_ench->name && *temp_ench->name && !strcasecmp(temp_ench->name, name)) || ((type > 0) && (temp_ench->type == type))) {
      ench_remove(ch, temp_ench, to_log);
    }
  }
}
