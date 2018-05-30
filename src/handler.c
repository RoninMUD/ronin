/**************************************************************************
*  file: handler.c , Handler module.                      Part of DIKUMUD *
*  Usage: Various routines for moving about objects/players               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "spells.h"
#include "utility.h"
#include "modify.h"
#include "interpreter.h"
#include "cmd.h"
#include "limits.h"
#include "enchant.h"
#include "subclass.h"
#include "fight.h"

/* External procedures */

int str_cmp(char *arg1, char *arg2);
void free_char(struct char_data *ch);
void stop_fighting(struct char_data *ch);
void remove_follower(struct char_data *ch);

extern char *index(const char *s, int c);
void clearMemory(struct char_data *ch);

void init_string_block(struct string_block *sb)
{
  sb->data = (char*)malloc(sb->size=128);
  *sb->data = '\0';
}

void append_to_string_block(struct string_block *sb, char *str)
{
  int   len;
  len = strlen(sb->data) + strlen(str) + 1;
  if (len > sb->size) {
    if ( len > (sb->size*=2))
      sb->size = len;
    sb->data = (char*)realloc(sb->data, sb->size);
  }
  strcat(sb->data, str);
}

void page_string_block(struct string_block *sb, struct char_data *ch)
{
  page_string(ch->desc, sb->data, 1);
}

void destroy_string_block(struct string_block *sb)
{
  free(sb->data);
  sb->data = NULL;
}

char *fname(char *namelist)
{
  static char holder[30];
  register char *point;

  for (point = holder; isalpha(*namelist); namelist++, point++)
   *point = *namelist;

  *point = '\0';

  return(holder);
}

/* Remove prefixed a, an or the from object short Ranger Oct 98 */
char *rem_prefix(char *str) {
  char *obuf;
  int loc,amount=0;

  if (!str) return str;
  if(is_abbrev("an ",str))  amount=3;
  if(is_abbrev("a ",str))   amount=2;
  if(is_abbrev("the ",str)) amount=4;
  if(!amount) return str;
  loc=0;
  for (obuf = str; *obuf && loc<amount; ++obuf) {loc++;}
  return obuf;
}

int isname(char *name, char *list)
{
  char *tmp_list = NULL;
  char *token = NULL;
  char *last = NULL;

  if (!name || !*name || !list || !*list) return FALSE;

  if (!str_cmp(name, list)) return TRUE;

  tmp_list = strdup(list);

  for (token = strtok_r(tmp_list, " \t\r\n", &last); token; token = strtok_r(NULL, " \t\r\n", &last))
  {
    if (token && !str_cmp(name, token))
    {
      free(tmp_list);
      return TRUE;
    }
  }

  free(tmp_list);
  return FALSE;
}

#define SCHAR_MIN -127
#define SCHAR_MAX 127

#define INC_SCHAR(orig,amount) \
  do { \
    if (orig > 0 && SCHAR_MAX-orig < amount) orig = SCHAR_MAX; \
    else if (orig < 0 && SCHAR_MIN-orig > amount ) orig = SCHAR_MIN; \
    else orig += amount; \
  } while (0)

void affect_modify(struct char_data *ch, int loc, int mod, long bitv,long bitv2, bool add)
{
  if (add) {
   SET_BIT(ch->specials.affected_by, bitv);
   SET_BIT(ch->specials.affected_by2, bitv2);
  } else {
   REMOVE_BIT(ch->specials.affected_by, bitv);
   REMOVE_BIT(ch->specials.affected_by2, bitv2);
   mod = -mod;
  }

  switch(loc) {
    case APPLY_NONE:
      break;
    case APPLY_STR:
      INC_SCHAR(GET_STR(ch),mod);
      break;
    case APPLY_DEX:
      GET_DEX(ch) += mod;
      break;
    case APPLY_INT:
      GET_INT(ch) += mod;
      break;
    case APPLY_WIS:
      GET_WIS(ch) += mod;
      break;
    case APPLY_CON:
      GET_CON(ch) += mod;
      break;
    case APPLY_SEX:
      /*GET_SEX(ch) += mod;*/
      break;
    case APPLY_CLASS:
      /* ??? GET_CLASS(ch) += mod; */
      break;
    case APPLY_LEVEL:
      /*GET_LEVEL(ch) += mod;
      Never reactivate this unless you want a potential security breach
      with a level 1 wearing a -level item.  There are other problems as
      well. - Ranger*/
      break;
   case APPLY_AGE:
    ch->player.time.birth -= ((long)SECS_PER_MUD_YEAR*(long)mod);
    break;
   case APPLY_CHAR_WEIGHT:
    GET_WEIGHT(ch) += mod;
    break;
   case APPLY_CHAR_HEIGHT:
    GET_HEIGHT(ch) += mod;
    break;
   case APPLY_MANA:
    ch->points.max_mana += mod;
    break;
   case APPLY_HIT:
    ch->points.max_hit += mod;
    break;
   case APPLY_MOVE:
    ch->points.max_move += mod;
    break;
   case APPLY_GOLD:
    break;
   case APPLY_EXP:
    break;
   case APPLY_AC:
    GET_AC(ch) += mod;
    break;
   case APPLY_HITROLL:
    GET_HITROLL(ch) += mod;
    break;
   case APPLY_DAMROLL:
    GET_DAMROLL(ch) += mod;
    break;
   case APPLY_SAVING_PARA:
    ch->specials.apply_saving_throw[0] += mod;
    break;
   case APPLY_SAVING_ROD:
    ch->specials.apply_saving_throw[1] += mod;
    break;
   case APPLY_SAVING_PETRI:
    ch->specials.apply_saving_throw[2] += mod;
    break;
   case APPLY_SAVING_BREATH:
    ch->specials.apply_saving_throw[3] += mod;
    break;
   case APPLY_SAVING_SPELL:
    ch->specials.apply_saving_throw[4] += mod;
    break;
   case APPLY_SKILL_SNEAK:
    if(ch->skills)
      ch->skills[SKILL_SNEAK].learned += mod;
    break;
   case APPLY_SKILL_HIDE:
    if(ch->skills)
      ch->skills[SKILL_HIDE].learned += mod;
    break;
   case APPLY_SKILL_STEAL:
    if(ch->skills)
      ch->skills[SKILL_STEAL].learned += mod;
    break;
   case APPLY_SKILL_BACKSTAB:
    if(ch->skills)
      ch->skills[SKILL_BACKSTAB].learned += mod;
    break;
   case APPLY_SKILL_PICK_LOCK:
    if(ch->skills)
      ch->skills[SKILL_PICK_LOCK].learned += mod;
    break;
   case APPLY_SKILL_KICK:
    if(ch->skills)
      ch->skills[SKILL_KICK].learned += mod;
    break;
   case APPLY_SKILL_BASH:
    if(ch->skills)
      ch->skills[SKILL_BASH].learned += mod;
    break;
   case APPLY_SKILL_RESCUE:
    if(ch->skills)
      ch->skills[SKILL_RESCUE].learned += mod;
    break;
   case APPLY_SKILL_BLOCK:
    if(ch->skills)
      ch->skills[SKILL_BLOCK].learned += mod;
    break;
   case APPLY_SKILL_KNOCK:
    if(ch->skills)
      ch->skills[SKILL_KNOCK].learned += mod;
    break;
   case APPLY_SKILL_PUNCH:
    if(ch->skills)
      ch->skills[SKILL_PUNCH].learned += mod;
    break;
   case APPLY_SKILL_PARRY:
    if(ch->skills)
      ch->skills[SKILL_PARRY].learned += mod;
    break;
   case APPLY_SKILL_DUAL:
    if(ch->skills)
      ch->skills[SKILL_DUAL].learned += mod;
    break;
   case APPLY_SKILL_THROW:
    if(ch->skills)
      ch->skills[SKILL_THROW].learned += mod;
    break;
   case APPLY_SKILL_DODGE:
    if(ch->skills)
      ch->skills[SKILL_DODGE].learned += mod;
    break;
   case APPLY_SKILL_PEEK:
    if(ch->skills)
      ch->skills[SKILL_PEEK].learned += mod;
    break;
   case APPLY_SKILL_BUTCHER:
    if(ch->skills)
      ch->skills[SKILL_BUTCHER].learned += mod;
    break;
   case APPLY_SKILL_TRAP:
    if(ch->skills)
      ch->skills[SKILL_TRAP].learned += mod;
    break;
   case APPLY_SKILL_DISARM:
    if(ch->skills)
      ch->skills[SKILL_DISARM].learned += mod;
    break;
   case APPLY_SKILL_SUBDUE:
    if(ch->skills)
      ch->skills[SKILL_SUBDUE].learned += mod;
    break;
   case APPLY_SKILL_CIRCLE:
    if(ch->skills)
      ch->skills[SKILL_CIRCLE].learned += mod;
    break;
   case APPLY_SKILL_TRIPLE:
    if(ch->skills)
      ch->skills[SKILL_TRIPLE].learned += mod;
    break;
   case APPLY_SKILL_PUMMEL:
    if(ch->skills)
      ch->skills[SKILL_PUMMEL].learned += mod;
    break;
   case APPLY_SKILL_AMBUSH:
    if(ch->skills)
      ch->skills[SKILL_AMBUSH].learned += mod;
    break;
   case APPLY_SKILL_ASSAULT:
    if(ch->skills)
      ch->skills[SKILL_ASSAULT].learned += mod;
    break;
   case APPLY_SKILL_DISEMBOWEL:
    if(ch->skills)
      ch->skills[SKILL_DISEMBOWEL].learned += mod;
    break;
   case APPLY_SKILL_BACKFLIP:
    if(ch->skills)
      ch->skills[SKILL_BACKFLIP].learned += mod;
    break;

   default:
/*  log_f("Unknown apply adjust attempt (handler.c, affect_modify).");*/
    break;

  } /* switch */
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data *ch)
{
  struct affected_type_5 *af;
  struct enchantment_type_5 *ench;
  int i,j;

  for(i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i])
      for(j=0; j<MAX_OBJ_AFFECT; j++)
        affect_modify(ch, ch->equipment[i]->affected[j].location,
                      ch->equipment[i]->affected[j].modifier,
                      ch->equipment[i]->obj_flags.bitvector,ch->equipment[i]->obj_flags.bitvector2, FALSE);
  }


  for(af = ch->affected; af; af=af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector,af->bitvector2, FALSE);
  for(ench = ch->enchantments; ench; ench=ench->next)
    affect_modify(ch, ench->location, ench->modifier, ench->bitvector,ench->bitvector2, FALSE);
  ch->tmpabilities = ch->abilities;

  ch->specials.org_hit=ch->points.max_hit;
  ch->specials.org_mana=ch->points.max_mana;
  ch->specials.org_move=ch->points.max_move;

  for(i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i])
      for(j=0; j<MAX_OBJ_AFFECT; j++)
        affect_modify(ch, ch->equipment[i]->affected[j].location,
                      ch->equipment[i]->affected[j].modifier,
                      ch->equipment[i]->obj_flags.bitvector,ch->equipment[i]->obj_flags.bitvector2, TRUE);
  }


  for(af = ch->affected; af; af=af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector,af->bitvector2, TRUE);
  for(ench = ch->enchantments; ench; ench=ench->next)
    affect_modify(ch, ench->location, ench->modifier, ench->bitvector,ench->bitvector2, TRUE);

  if(GET_OCON(ch)<19) GET_CON(ch)=MAX(0,MIN(GET_CON(ch),18));
  else GET_CON(ch) = MAX(0,MIN(GET_CON(ch),GET_OCON(ch)));

  if(GET_OINT(ch)<19) GET_INT(ch)=MAX(0,MIN(GET_INT(ch),18));
  else GET_INT(ch) = MAX(0,MIN(GET_INT(ch),GET_OINT(ch)));

  if(GET_ODEX(ch)<19) GET_DEX(ch)=MAX(0,MIN(GET_DEX(ch),18));
  else GET_DEX(ch) = MAX(0,MIN(GET_DEX(ch),GET_ODEX(ch)));

  if(GET_OWIS(ch)<19) GET_WIS(ch)=MAX(0,MIN(GET_WIS(ch),18));
  else GET_WIS(ch) = MAX(0,MIN(GET_WIS(ch),GET_OWIS(ch)));

  if(GET_STR(ch)>18) {
    if(GET_OSTR(ch)<19) {
      i=GET_ADD(ch)+((GET_STR(ch)-18)*10);
      GET_ADD(ch)=MIN(i,100);
      GET_STR(ch)=18;
    }
    else {
      GET_STR(ch)=MIN(GET_STR(ch),GET_OSTR(ch));
    }
  }

  if(GET_STR(ch)<18) {
    i=GET_ADD(ch)/10;
    GET_STR(ch)+=i;
    GET_ADD(ch)-=i*10;
    if(GET_STR(ch)>18) {
      i=GET_ADD(ch)+((GET_STR(ch)-18)*10);
      GET_ADD(ch)=MIN(i, 100);
      GET_STR(ch)=18;
    }
  }
}

/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(CHAR *ch, AFF *af) {
  if (!ch || !af) return;

  AFF *tmp_af;

  CREATE(tmp_af, AFF, 1);

  *tmp_af = *af;

  tmp_af->next = ch->affected;
  ch->affected = tmp_af;

  affect_modify(ch, af->location, af->modifier, af->bitvector, af->bitvector2, TRUE);

  affect_total(ch);
  check_equipment(ch);
}

/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NIL! Frees mem and calls
   affect_location_apply                                                */
void affect_remove(CHAR *ch, AFF *af) {
  if (!ch || !af || !(ch->affected)) return;

  affect_modify(ch, af->location, af->modifier, af->bitvector, af->bitvector2, FALSE);

  if (ch->affected == af) {
    ch->affected = af->next;
  }
  else {
    AFF *tmp_af;

    for (tmp_af = ch->affected; tmp_af->next && (tmp_af->next != af); tmp_af = tmp_af->next);

    tmp_af->next = af->next;
  }

  free(af);

  affect_total(ch);
  check_equipment(ch);
}

void affect_from_char(CHAR *ch, int type) {
  if (!ch) return;

  for (AFF *aff = ch->affected, *tmp_aff = NULL; aff; aff = tmp_aff) {
    tmp_aff = aff->next;

    if (aff->type == type) {
      affect_remove(ch, aff);
    }
  }
}

bool affected_by_spell(CHAR *ch, int type) {
  if (!ch) return FALSE;

  for (AFF *aff = ch->affected; aff; aff = aff->next) {
    if (aff->type == type) {
      return TRUE;
    }
  }

  return FALSE;
}

int duration_of_spell(CHAR *ch, int type) {
  if (!ch) return 0;

  for (AFF *aff = ch->affected; aff; aff = aff->next) {
    if (aff->type == type) {
      return aff->duration;
    }
  }

  return 0;
}

void affect_join(CHAR *ch, AFF *af, bool avg_dur, bool avg_mod) {
  if (!ch || !af) return;

  bool found = FALSE;

  for (AFF *aff = ch->affected; aff && !found; aff = aff->next) {
    if (aff->type == af->type) {
      INC_SCHAR(af->duration, aff->duration);
      if (avg_dur) {
        af->duration /= 2;
      }

      INC_SCHAR(af->modifier, aff->modifier);
      if (avg_mod) {
        af->modifier /= 2;
      }

      affect_remove(ch, aff);
      affect_to_char(ch, af);

      found = TRUE;
    }
  }

  if (!found) {
    affect_to_char(ch, af);
  }
}

void remove_all_affects(CHAR *ch) {
  if (!ch) return;

  for (AFF *aff = ch->affected, *tmp_aff = NULL; aff; aff = tmp_aff) {
    tmp_aff = aff->next;

    affect_from_char(ch, aff->type);
  }
}

AFF * get_affect_from_char(CHAR *ch, int type) {
  if (!ch) return NULL;

  for (AFF *aff = ch->affected; aff; aff = aff->next) {
    if (aff->type == type) return aff;
  }

  return NULL;
}

void affect_apply(CHAR *ch, int af_type, int af_dur, int af_mod, int af_loc, long af_bitv, long af_bitv2) {
  if (!ch) return;

  AFF af;

  af.type = af_type;
  af.duration = af_dur;
  af.modifier = af_mod;
  af.location = af_loc;
  af.bitvector = af_bitv;
  af.bitvector2 = af_bitv2;

  affect_to_char(ch, &af);
}

/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
  struct char_data *i;

  if (CHAR_REAL_ROOM(ch) == NOWHERE) {
    log_s("NOWHERE extracting char from room (handler.c, char_from_room)");
    produce_core();
  }

  if(affected_by_spell(ch, SKILL_CAMP)) {
    act("You quickly break camp.\n\r",1,ch,0,0,TO_CHAR);
    act("$n quickly breaks camp.",1,ch,0,0,TO_ROOM);
    affect_from_char(ch,SKILL_CAMP);
    if(CHAR_VIRTUAL_ROOM(ch)!=3076 &&
       CHAR_VIRTUAL_ROOM(ch)!=3079 &&
       CHAR_VIRTUAL_ROOM(ch)!=3081 &&
       CHAR_VIRTUAL_ROOM(ch)!=3083)
      REMOVE_BIT(world[CHAR_REAL_ROOM(ch)].room_flags,CLUB);
  }

  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light is ON */
        world[CHAR_REAL_ROOM(ch)].light--;

  if (ch == world[CHAR_REAL_ROOM(ch)].people)  /* head of list */
     world[CHAR_REAL_ROOM(ch)].people = ch->next_in_room;

  else    /* locate the previous element */
  {
    for (i = world[CHAR_REAL_ROOM(ch)].people;
       i->next_in_room != ch; i = i->next_in_room);

     i->next_in_room = ch->next_in_room;
  }

  ch->in_room_r = NOWHERE;
  ch->in_room_v = NOWHERE;
  ch->next_in_room = 0;
}


/* place a character in a room */
void char_to_room(struct char_data *ch, int room)
{
  if(room<0)
     room = 0;
  if (IS_SET(world[room].room_flags, LOCK) &&
      (GET_LEVEL(ch) < LEVEL_SUP) &&
      real_room(1212) != room &&
      real_room(3054) != room &&
      !isname(GET_NAME(ch),world[room].name)) {
    send_to_char("The room is locked. There may be a private conversation there.\n\r", ch);
    if(GET_LEVEL(ch)<LEVEL_IMM) char_to_room(ch, real_room(3054));
    else char_to_room(ch,real_room(1212));
    return;
  }

  ch->next_in_room = world[room].people;
  world[room].people = ch;
  ch->in_room_r = room;
  ch->in_room_v = world[room].number;

  if (GET_CLASS(ch) == CLASS_NINJA && GET_POS(ch) == POSITION_SWIMMING
      && world[room].sector_type < SECT_WATER_NOSWIM)
    GET_POS(ch) = POSITION_STANDING;

  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light is ON */
  world[room].light++;
}


/* give an object to a char   */

void obj_to_char(struct obj_data *object, struct char_data *ch) {
  char buf[MAX_INPUT_LENGTH];
  if (object) {
    adjust_obj_list(object,ch->carrying);
    ch->carrying = object;
    object->carried_by = ch;
    object->equipped_by = 0;
    object->in_room = NOWHERE;
    if(ch->questobj && ch->quest_status==QUEST_RUNNING)
    {
      if(ch->questobj==object && V_OBJ(object)!=35)
      {
        send_to_char("You have the quest item! Return it to the quest giver for credit.\n\r",ch);
        ch->quest_status=QUEST_COMPLETED;
      }
      if(V_OBJ(ch->questobj) == V_OBJ(object) && OBJ_SPEC(object) == ch->ver3.id)
      {/* for auto-questcard hunting */
        send_to_char("You found one! When you have enough, return to your guildmaster and complete.\n\r", ch);
      }
    }
    if(object->log) {
      sprintf(buf,"QSTINFO: %s has item %s (%d).",GET_NAME(ch),OBJ_SHORT(object),V_OBJ(object));
      if(object->log==1) {
        if(!IS_NPC(ch)) object->log=0;
        log_f("%s",buf);
      }
      if(object->log==2) wizlog(buf,GET_LEVEL(ch),4);
    }

    if(object->obj_flags.type_flag==ITEM_SC_TOKEN && !IS_NPC(ch))
      log_f("SUBLOG: %s has a token.",GET_NAME(ch));
  }
}


/* take an object from a char */
struct obj_data *obj_from_char(struct obj_data *object)
{
  struct obj_data *tmp;

  if (object->equipped_by) {
    log_f("Obj in more than one place.");
    abort();
  }

  if(object->obj_flags.type_flag==ITEM_SC_TOKEN && !IS_NPC(object->carried_by))
    log_f("SUBLOG: %s loses a token.",GET_NAME(object->carried_by));

  if (object->carried_by->carrying == object)   /* head of list */
     object->carried_by->carrying = object->next_content;

  else
  {
    for (tmp = object->carried_by->carrying;
       tmp && (tmp->next_content != object);
          tmp = tmp->next_content); /* locate previous */

    tmp->next_content = object->next_content;
  }
  object->equipped_by = 0;
  object->carried_by = 0;
  object->next_content = 0;
        return object;
}


int apply_ac(struct char_data *ch, int eq_pos) {
  if (!ch || (eq_pos < 0)) return 0;

  struct obj_data *eq = EQ(ch, eq_pos);

  if (!eq || (OBJ_TYPE(eq) != ITEM_ARMOR)) return 0;

  int ac = OBJ_VALUE0(eq);

  int multi = 1;

  switch (eq_pos) {
    case WEAR_BODY:
      multi = 3;
      break;
    case WEAR_SHIELD:
      multi = 2;

      /* Protect */
      if (IS_MORTAL(ch) && check_subclass(ch, SC_WARLORD, 2)) {
        multi += 1;
      }
      break;
  }

  return (ac * multi);
}

void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
  int j,hit,owner_ok=0;
  char buf[MAX_INPUT_LENGTH];

  assert(pos>=0 && pos<MAX_WEAR);
  assert(!(ch->equipment[pos]));

  if (!obj) return;
  if (obj->carried_by) {
   log_f("EQUIP: Obj is carried_by when equip.");
   return;
  }

  if (obj->in_room!=NOWHERE) {
   log_f("EQUIP: Obj is in_room when equip.");
   return;
  }

  if(!IS_NPC(ch) && GET_LEVEL(ch)<LEVEL_IMM) {
   if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)       && IS_EVIL(ch))         ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)        && IS_GOOD(ch))         ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL)     && IS_NEUTRAL(ch))      ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR)     && (GET_CLASS(ch) == CLASS_WARRIOR))      ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF)       && (GET_CLASS(ch) == CLASS_THIEF))        ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC)      && (GET_CLASS(ch) == CLASS_CLERIC))       ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER)  && (GET_CLASS(ch) == CLASS_MAGIC_USER))   ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NINJA)       && (GET_CLASS(ch) == CLASS_NINJA))        ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NOMAD)       && (GET_CLASS(ch) == CLASS_NOMAD))        ||
      /*
      (IS_OBJ_STAT(obj, ITEM_ANTI_MALE)        && (GET_SEX(ch)   == SEX_MALE))           ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_FEMALE)      && (GET_SEX(ch)   == SEX_FEMALE))         ||
      */
      (IS_OBJ_STAT(obj, ITEM_ANTI_PALADIN)     && (GET_CLASS(ch) == CLASS_PALADIN))      ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_ANTIPALADIN) && (GET_CLASS(ch) == CLASS_ANTI_PALADIN)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_AVATAR)      && (GET_CLASS(ch) == CLASS_AVATAR))       ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_BARD)        && (GET_CLASS(ch) == CLASS_BARD))         ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_COMMANDO)    && (GET_CLASS(ch) == CLASS_COMMANDO))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_ENCHANTER  )   && check_subclass(ch,SC_ENCHANTER  ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_ARCHMAGE   )   && check_subclass(ch,SC_ARCHMAGE   ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_DRUID      )   && check_subclass(ch,SC_DRUID      ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_TEMPLAR    )   && check_subclass(ch,SC_TEMPLAR    ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_ROGUE      )   && check_subclass(ch,SC_ROGUE      ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_BANDIT     )   && check_subclass(ch,SC_BANDIT     ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_WARLORD    )   && check_subclass(ch,SC_WARLORD    ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_GLADIATOR  )   && check_subclass(ch,SC_GLADIATOR  ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_RONIN      )   && check_subclass(ch,SC_RONIN      ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_MYSTIC     )   && check_subclass(ch,SC_MYSTIC     ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_RANGER     )   && check_subclass(ch,SC_RANGER     ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_TRAPPER    )   && check_subclass(ch,SC_TRAPPER    ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_CAVALIER   )   && check_subclass(ch,SC_CAVALIER   ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_CRUSADER   )   && check_subclass(ch,SC_CRUSADER   ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_DEFILER    )   && check_subclass(ch,SC_DEFILER    ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_INFIDEL    )   && check_subclass(ch,SC_INFIDEL    ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_BLADESINGER)   && check_subclass(ch,SC_BLADESINGER,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_CHANTER    )   && check_subclass(ch,SC_CHANTER    ,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_LEGIONNAIRE)   && check_subclass(ch,SC_LEGIONNAIRE,1))     ||
      (IS_SET(obj->obj_flags.subclass_res, ITEM_ANTI_MERCENARY  )   && check_subclass(ch,SC_MERCENARY  ,1))     ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_MORTAL)      && (GET_LEVEL(ch) < LEVEL_IMM) && !CHAOSMODE)       )          {
     if (CHAR_REAL_ROOM(ch) != NOWHERE) {
       act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
       act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
       sprintf(buf,"%s zapped by %s (Room %d)",GET_NAME(ch),OBJ_SHORT(obj),CHAR_VIRTUAL_ROOM(ch));
       log_f("%s",buf);
       obj->log = TRUE;
       wizlog(buf,LEVEL_WIZ,6);
       obj_to_room(obj, CHAR_REAL_ROOM(ch));
       save_char(ch, NOWHERE);
       return;
     } else {
       log_f("CHAR_REAL_ROOM(ch) = NOWHERE when equipping char.");
     }
    }
  }

  if((GET_ITEM_TYPE(obj)==ITEM_WEAPON || GET_ITEM_TYPE(obj)==ITEM_2HWEAPON) && (!IS_NPC(ch)) && /* Ranger - July 96 */
      GETOBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
    {
    act("$p falls from your grasp, because of your weakness.", FALSE, ch, obj, 0, TO_CHAR);
    act("$p falls from $n's grasp because $e's too weak.", FALSE, ch, obj, 0, TO_ROOM);
    obj_to_char(obj, ch);
    return;
    }
/* Insert check for - hps here */
  if(GET_LEVEL(ch)<21) {
   for (j=0; j<MAX_OBJ_AFFECT; j++) {
    if(obj->affected[j].location==APPLY_HIT) {
      hit=hit_limit(ch)+obj->affected[j].modifier;
      if(hit<1) {
        if (CHAR_REAL_ROOM(ch) != NOWHERE) {
          act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
          act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
          sprintf(buf,"%s zapped by %s (Room %d)",GET_NAME(ch),OBJ_SHORT(obj),CHAR_VIRTUAL_ROOM(ch));
          log_f("%s",buf);
          obj->log = TRUE;
          wizlog(buf,LEVEL_WIZ,6);
          obj_to_room(obj, CHAR_REAL_ROOM(ch));
          save_char(ch, NOWHERE);
          return;
        } else {
          log_f("CHAR_REAL_ROOM(ch) = NOWHERE when equipping char.");
        }
      }
    }
   }
  }

/* Check for equipment ownership */
  if(IS_SET(obj->obj_flags.wear_flags,ITEM_QUESTWEAR)) {
    for(j=0;j<8;j++) {
      if(ch->ver3.id==obj->ownerid[j]) owner_ok=1;
    }
    if(!owner_ok) {
      if (CHAR_REAL_ROOM(ch) != NOWHERE) {
        act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
        sprintf(buf,"%s zapped by %s (Room %d)",GET_NAME(ch),OBJ_SHORT(obj),CHAR_VIRTUAL_ROOM(ch));
        log_f("%s",buf);
        obj->log = TRUE;
        wizlog(buf,LEVEL_WIZ,6);
        obj_to_room(obj, CHAR_REAL_ROOM(ch));
        save_char(ch, NOWHERE);
        return;
      } else {
        log_f("CHAR_REAL_ROOM(ch) = NOWHERE when equipping char.");
      }
    }
  }

  ch->equipment[pos] = obj;
  obj->equipped_by = ch;
  obj->in_room = NOWHERE;
  obj->in_room_v = NOWHERE;
  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) -= apply_ac(ch, pos);

  for (j=0; j<MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
    obj->affected[j].modifier,
    obj->obj_flags.bitvector,obj->obj_flags.bitvector2, TRUE);
   affect_total(ch);
}

int rent_equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
  int j;

  if (!obj)
     return(FALSE);
  if ( pos < 0 || pos >= MAX_WEAR)
    return(FALSE);

  if(ch->equipment[pos]){
    log_f("BUG: double equipment same position!!!!(rentequip)");
    return(FALSE);
  }

  if (obj->carried_by) {
    log_f("BUG: Obj is carried_by when rentequip.");
    return(FALSE);
  }

  if (obj->in_room!=NOWHERE) {
    log_f("BUG: Obj is in_room when rentequip.");
    return(FALSE);
  }

  if (obj->equipped_by) {
    log_f("BUG: Obj is equipped when rentequip");
    return(FALSE);
  }

  if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && (GET_CLASS(ch) == CLASS_WARRIOR)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) && (GET_CLASS(ch) == CLASS_THIEF)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && (GET_CLASS(ch) == CLASS_CLERIC)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER) && (GET_CLASS(ch) == CLASS_MAGIC_USER)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NINJA) && (GET_CLASS(ch) == CLASS_NINJA)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NOMAD) && (GET_CLASS(ch) == CLASS_NOMAD)) ||
      /*
      (IS_OBJ_STAT(obj, ITEM_ANTI_MALE) && (GET_SEX(ch) == SEX_MALE)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_FEMALE) && (GET_SEX(ch) == SEX_FEMALE)) ||
      */
      (IS_OBJ_STAT(obj, ITEM_ANTI_PALADIN) && (GET_CLASS(ch) == CLASS_PALADIN)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_ANTIPALADIN) && (GET_CLASS(ch) == CLASS_ANTI_PALADIN)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_AVATAR) && (GET_CLASS(ch) == CLASS_AVATAR)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_BARD) && (GET_CLASS(ch) == CLASS_BARD)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_COMMANDO) && (GET_CLASS(ch) == CLASS_COMMANDO)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_MORTAL) && (GET_LEVEL(ch) < LEVEL_IMM))) {
    return(FALSE);
  }

  if(obj->obj_flags.type_flag == ITEM_WEAPON){
    if((GET_CLASS(ch) != CLASS_NINJA && pos == HOLD) ||
       (pos != HOLD && pos != WIELD) ||
       (GETOBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w))
      return(FALSE);
  }

  ch->equipment[pos] = obj;
  obj->equipped_by = ch;
  obj->in_room = NOWHERE;
  obj->in_room_v = NOWHERE;
  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) -= apply_ac(ch, pos);

  for(j=0; j<MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
      obj->affected[j].modifier,
      obj->obj_flags.bitvector,obj->obj_flags.bitvector2, TRUE);

  affect_total(ch);
  return(TRUE);
}


struct obj_data *unequip_char(struct char_data *ch, int pos)
{
  int j;
  struct obj_data *obj;

  assert(pos>=0 && pos<MAX_WEAR);

  if (!ch->equipment[pos])
       return NULL;
  obj = ch->equipment[pos];
  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
   GET_AC(ch) += apply_ac(ch, pos);

  obj->equipped_by = 0;
  ch->equipment[pos] = 0;
  obj->in_room = NOWHERE;
  obj->in_room_v = NOWHERE;

  for(j=0; j<MAX_OBJ_AFFECT; j++)
   affect_modify(ch, obj->affected[j].location,
  obj->affected[j].modifier,
  obj->obj_flags.bitvector,obj->obj_flags.bitvector2, FALSE);

  affect_total(ch);

  return(obj);
}

int get_number(char **name)
{
  char *ppos = NULL;
  char number[MIL] = "";
  int i = 0;

  if ((ppos = index(*name, '.')))
  {
    *ppos++ = '\0';

    strcpy(number, *name);
    strcpy(*name, ppos);

    for (i = 0; *(number + i); i++)
    {
      if (!isdigit(*(number + i)))
      {
        return 0;
      }
    }

    return atoi(number);
  }

  return 1;
}

/* Search a given list for an object, and return a pointer to that object. */
OBJ *get_obj_in_list(char *name, OBJ *list)
{
  OBJ *obj = NULL;
  char tmp_name[MIL] = "";
  char *tmp = NULL;
  int number = 0;
  int i = 0;

  strcpy(tmp_name, name);
  tmp = tmp_name;

  if ((number = get_number(&tmp)))
  {
    for (obj = list, i = 1; obj && i <= number; obj = obj->next_content)
    {
      if (isname(tmp, OBJ_NAME(obj)))
      {
        if (i == number)
        {
          return obj;
        }

        i++;
      }
    }
  }

  return NULL;
}

/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
  struct obj_data *i;

  for (i = list; i; i = i->next_content)
    if (i->item_number == num)
      return(i);

  return(0);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(char *name)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  for (i = object_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, OBJ_NAME(i))) {
      if (j == number)
        return(i);
      j++;
    }

  return(0);
}

/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
  struct obj_data *i;
  for (i = object_list; i; i = i->next)
    if (i->item_number == nr)
      return(i);

  return(0);
}





/* search a room for a char, and return a pointer if found..  */
CHAR *get_char_room(char *name, int real_room)
{
  CHAR *ch = NULL;
  char tmp_name[MIL] = "";
  char *tmp = NULL;
  int number = 0;
  int i = 0;

  strcpy(tmp_name, name);
  tmp = tmp_name;

  if ((number = get_number(&tmp)))
  {
    for (ch = world[real_room].people, i = 1; ch && i <= number; ch = ch->next_in_room)
    {
      if (isname(tmp, GET_NAME(ch)))
      {
        if (i == number)
        {
          return ch;
        }

        i++;
      }
    }
  }

  return NULL;
}

/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(char *name)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  for (i = character_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, GET_NAME(i))) {
      if (j == number)
        return(i);
      j++;
    }

  return(0);
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    if (i->nr == nr)
      return(i);

  return(0);
}

/* put an object in a room */
void obj_to_room(struct obj_data *object, int room)
{
  int fall_room,v_room;
  char buf[MSL];

  if(room==NOWHERE) {
    sprintf(buf,"ERROR: Trying to place %s NOWHERE",OBJ_SHORT(object));
    log_f("%s",buf);
    return;
  }

  adjust_obj_list(object,world[room].contents);
  world[room].contents = object;
  object->equipped_by = 0;
  object->in_room = room;
  object->in_room_v = world[room].number;
  object->carried_by = 0;
  if(object->log) {
    sprintf(buf,"QSTINFO: Item %s (%d) placed in room %d.",OBJ_SHORT(object),V_OBJ(object),object->in_room_v);
    if(object->log==1) log_f("%s",buf);
    if(object->log==2) wizlog(buf,LEVEL_IMM,4);
  }
  if(object->obj_flags.type_flag==ITEM_SC_TOKEN)
    log_f("SUBLOG: Token placed in room %d.",object->in_room_v);

  if(IS_SET(world[room].room_flags,FLYING)) {
    /* Check for a down exit */
    v_room=world[room].number;
    if(world[room].dir_option[DOWN]) {
      fall_room=world[room].dir_option[DOWN]->to_room_r;
      if(fall_room==room) {
        sprintf(buf, "WIZINFO: FLYING: Room %d has a loop to itself.", v_room);
        wizlog(buf, LEVEL_SUP, 6);
        return;
      }
      if(fall_room==0 || fall_room==-1) {
        sprintf(buf, "WIZINFO: FLYING: Room %d has a NOWHERE or VOID exit.", v_room);
        wizlog(buf, LEVEL_SUP, 6);
        return;
      }
    } else {
      sprintf(buf, "WIZINFO: FLYING: Room %d has no down exit.", v_room);
      wizlog(buf, LEVEL_SUP, 6);
      return;
    }
    if(CAN_WEAR(object,ITEM_TAKE)) {
      /* Make obj fall to room below */
      sprintf(buf, "The %s falls to the area below.\n\r",rem_prefix(OBJ_SHORT(object)));
      send_to_room(buf,room);
      sprintf(buf,"The %s falls from above.\n\r",rem_prefix(OBJ_SHORT(object)));
      send_to_room(buf,fall_room);
      obj_from_room(object);
      obj_to_room(object,fall_room);
    }
  }
}

void adjust_obj_list(struct obj_data *object, struct obj_data *list)
{
  int i;
  int f1,f2,f3;
  struct obj_data *x,*back,*back2;
  bool found=FALSE,c=TRUE;
  f1=f2=f3=0;i=0;
  x=NULL;back=NULL;back2=NULL;
  x=list;
  if(x)
    {
    while(c)
    {
    i++;
    if((object->item_number==x->item_number)
           &&(IS_OBJ_STAT(object,ITEM_CLONE)==IS_OBJ_STAT(x,ITEM_CLONE))
        &&(IS_OBJ_STAT(object,ITEM_INVISIBLE)==IS_OBJ_STAT(x,ITEM_INVISIBLE))
        &&(!str_cmp(OBJ_NAME(object),OBJ_NAME(x)))
        &&(!str_cmp(OBJ_SHORT(object),OBJ_SHORT(x)))
        &&(!str_cmp(OBJ_DESCRIPTION(object),OBJ_DESCRIPTION(x))))
      {
      c=FALSE;found=TRUE;
      if(i==1) f1=0;
      else f1=1;
      object->next_content=x;
      while(found)
        {
        back2=x;
        x=x->next_content;
        if(x)  {
          if((object->item_number==x->item_number)
              &&(IS_OBJ_STAT(object,ITEM_CLONE)==IS_OBJ_STAT(x,ITEM_CLONE))
              &&(IS_OBJ_STAT(object,ITEM_INVISIBLE)==IS_OBJ_STAT(x,ITEM_INVISIBLE))
              &&(!str_cmp(OBJ_NAME(object),OBJ_NAME(x)))
              &&(!str_cmp(OBJ_SHORT(object),OBJ_SHORT(x)))
              &&(!str_cmp(OBJ_DESCRIPTION(object),OBJ_DESCRIPTION(x))))
            {
            found=TRUE;
            }
          else  {
            if(f1)  {
               back->next_content=x;
              back2->next_content=list;
              }
            found=FALSE;
            }
          }
        else  {
          if(f1)  {
            back->next_content=NULL;
            back2->next_content=list;
            }
          found=FALSE;
          }
        }
      }
    else  {
      back=x;
      x=x->next_content;
      if(!x)  {
        c=FALSE;
        object->next_content=list;
        }
      }
    }
    }
  else
          if(object)
            object->next_content=NULL;
/*------------------------------------------------------*/

}


/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
  struct obj_data *i;

  /* remove object from room */

  if (object->in_room <= NOWHERE) {
    if (object->carried_by || object->equipped_by) {
       log_f("Eek.. an object was just taken from a char, instead of a room");
       abort();
    }
    return;  /* its not in a room */
  }
  if(object->obj_flags.type_flag==ITEM_SC_TOKEN)
    log_f("SUBLOG: Token removed from room %d.",object->in_room_v);


  if (object == world[object->in_room].contents)  /* head of list */
     world[object->in_room].contents = object->next_content;

  else     /* locate previous element in list */
  {
    for (i = world[object->in_room].contents; i &&
       (i->next_content != object); i = i->next_content);

    i->next_content = object->next_content;
   }

  object->in_room = NOWHERE;
  object->in_room_v = NOWHERE;
  object->next_content = 0;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to) {
  char buf[MAX_INPUT_LENGTH];

  if (obj && obj_to) {
    adjust_obj_list(obj,obj_to->contains);
    obj_to->contains = obj;
    obj->in_obj = obj_to;
    obj->carried_by = 0;
    obj->equipped_by = 0;
    obj->in_room = -1;
    obj->in_room_v = -1;
    if(obj->log) {
      sprintf(buf,"QSTINFO: %s (%d) placed in item %s (%d).",OBJ_SHORT(obj),V_OBJ(obj),OBJ_SHORT(obj_to),V_OBJ(obj_to));
      if(obj->log==1) log_f("%s",buf);
      if(obj->log==2) wizlog(buf,LEVEL_IMM,4);
    }
    if(obj->obj_flags.type_flag==ITEM_SC_TOKEN)
      log_f("SUBLOG: Token placed in item %s (%d).",OBJ_SHORT(obj_to),V_OBJ(obj_to));
  }
}


/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
  struct obj_data *tmp, *obj_from;

  if (obj->in_obj) {
    if(obj->obj_flags.type_flag==ITEM_SC_TOKEN)
      log_f("SUBLOG: Token removed from item %s (%d).",OBJ_SHORT(obj->in_obj),V_OBJ(obj->in_obj));

    obj_from = obj->in_obj;
    if (obj == obj_from->contains)   /* head of list */
       obj_from->contains = obj->next_content;
    else {
      for (tmp = obj_from->contains;
        tmp && (tmp->next_content != obj);
        tmp = tmp->next_content); /* locate previous */

      if (!tmp) {
        log_f("Fatal error in object structures.");
        abort();
      }

      tmp->next_content = obj->next_content;
    }


    obj->in_obj = 0;
    obj->next_content = 0;
  } else {
    log_f("Trying to object from object when in no object.");
    abort();
  }
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
  if (list) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}


extern void free_obj(struct obj_data *obj);
/* Extract an object from the world */
void extract_obj(struct obj_data *obj) {
  char buf[MAX_INPUT_LENGTH];
  struct obj_data *temp1, *temp2;
  int i;

  if(!obj) return;
  if(obj->owned_by)
    obj->owned_by->questobj=0;

  if(obj->in_room != NOWHERE) {
    if(obj->log) {
      sprintf(buf,"QSTINFO: %s (%d) extracted from room %d.",OBJ_SHORT(obj),V_OBJ(obj),obj->in_room_v);
      if(obj->log==1) log_f("%s",buf);
      if(obj->log==2) wizlog(buf,LEVEL_IMM,4);
    }
    if(obj->obj_flags.type_flag==ITEM_SC_TOKEN)
      log_f("SUBLOG: Token extracted from room %d.",obj->in_room_v);
    obj_from_room(obj);
  }
  else if(obj->carried_by) {
    if(obj->log) {
      sprintf(buf,"QSTINFO: %s (%d) extracted from %s.",OBJ_SHORT(obj),V_OBJ(obj),GET_NAME(obj->carried_by));
      if(obj->log==1) log_f("%s",buf);
      if(obj->log==2) wizlog(buf,LEVEL_IMM,4);
    }
    obj_from_char(obj);
  }
  else if(obj->equipped_by) {
    if(obj->log) {
      sprintf(buf,"QSTINFO: %s (%d) extracted from %s.",OBJ_SHORT(obj),V_OBJ(obj),GET_NAME(obj->equipped_by));
      if(obj->log==1) log_f("%s",buf);
      if(obj->log==2) wizlog(buf,LEVEL_IMM,4);
    }

    for (i = 0; i < MAX_WEAR; i++)
      if (obj->equipped_by->equipment[i]) {
        if(obj->equipped_by->equipment[i]==obj) {
          obj_to_char(unequip_char(obj->equipped_by,i), obj->equipped_by);
          break;
        }
      }
    obj_from_char(obj);
  }
  else if(obj->in_obj)
  {
    if(obj->log) {
      sprintf(buf,"QSTINFO: %s (%d) extracted from item %s (%d).",OBJ_SHORT(obj),V_OBJ(obj),OBJ_SHORT(obj->in_obj),V_OBJ(obj->in_obj));
      if(obj->log==1) log_f("%s",buf);
      if(obj->log==2) wizlog(buf,LEVEL_IMM,4);
    }
    if(obj->obj_flags.type_flag==ITEM_SC_TOKEN)
      log_f("SUBLOG: Token extracted from item %s (%d).",OBJ_SHORT(obj->in_obj),V_OBJ(obj->in_obj));

    temp1 = obj->in_obj;
    if(temp1->contains == obj)   /* head of list */
      temp1->contains = obj->next_content;
    else
    {
      for( temp2 = temp1->contains ;
        temp2 && (temp2->next_content != obj);
        temp2 = temp2->next_content );

      if(temp2) {
        temp2->next_content =
          obj->next_content; }
    }
  }

  for( ; obj->contains; extract_obj(obj->contains));
    /* leaves nothing ! */

  if (object_list == obj )       /* head of list */
    object_list = obj->next;
  else
  {
    for(temp1 = object_list;
      temp1 && (temp1->next != obj);
      temp1 = temp1->next);

    if(temp1)
      temp1->next = obj->next;
  }

  if(obj->item_number>=0)
    (obj_proto_table[obj->item_number].number)--;
  free_obj(obj);
}


void update_object(struct obj_data *obj, int equipped) {
  if ((obj->obj_flags.timer > 0) && (!(obj->in_obj) || (OBJ_TYPE(obj->in_obj) != ITEM_AQ_ORDER))) {
    if (IS_SET(obj->obj_flags.extra_flags2, ITEM_ALL_DECAY)) {
      if (equipped) obj->obj_flags.timer -= 2;
      else obj->obj_flags.timer -= 1;
    }
    else if (IS_SET(obj->obj_flags.extra_flags2, ITEM_EQ_DECAY)) {
      if (equipped) obj->obj_flags.timer -= 1;
    }
  }

  if (obj->contains) update_object(obj->contains, FALSE);
  if (obj->next_content) update_object(obj->next_content, FALSE);
}


void update_char_objects( struct char_data *ch )
{

  int i;

  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] > 0)
        (ch->equipment[WEAR_LIGHT]->obj_flags.value[2])--;

  for(i = 0;i < MAX_WEAR;i++)
    if(ch->equipment[i])
      update_object(ch->equipment[i],TRUE);

  if(ch->carrying) update_object(ch->carrying,FALSE);
}



/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data *ch)
{
  struct obj_data *i;
  struct char_data *k, *next_char;
  struct descriptor_data *t_desc;
  int l, was_in;
  char buf[MAX_INPUT_LENGTH];

  void do_return(struct char_data *ch, char *argument, int cmd);
  void affect_remove(struct char_data *ch, struct affected_type_5 *af);
  void die_follower(struct char_data *ch);
  void stop_riding(struct char_data *ch,struct char_data *vict);

  /* Re-randomize Rashgugh time if for some reason he wasn't extracted
  from a regular style death. Ranger Dec 2000*/
  if(IS_NPC(ch) && V_MOB(ch)==11 && token_mob_time<2)
    token_mob_time=number(50,80);

  if(ch->specials.rider) {
    if(IS_AFFECTED(ch->specials.rider, AFF_FLY))
      GET_POS(ch->specials.rider) = POSITION_FLYING;
    else
      GET_POS(ch->specials.rider) = POSITION_STANDING;
    stop_riding(ch->specials.rider,ch);
  }

  if(ch->questowner)
    ch->questowner->questmob=0;
  if(ch->questmob)
    ch->questmob->questowner=0;
  if(ch->questobj)
    ch->questobj->owned_by=0;


  if(ch->specials.riding) stop_riding(ch,ch->specials.riding);

  /*  Removing affects  */
  while (ch->affected) affect_remove(ch,ch->affected);
  while (ch->enchantments) enchantment_remove(ch,ch->enchantments,0);

  if(!IS_NPC(ch) && !ch->desc)
  {
    for(t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
      if(t_desc->original==ch)
        do_return(t_desc->character, "", 0);
  }

  if (CHAR_REAL_ROOM(ch) == NOWHERE) {
    sprintf(buf,"ERROR: NOWHERE when extracting char %s",GET_NAME(ch));
    log_f("%s",buf);
    if (ch == character_list)
      character_list = ch->next;
    else {
      for(k = character_list; (k) && (k->next != ch); k = k->next);
      if(k)
        k->next = ch->next;
      else {
        log_f("Trying to remove ?? from character_list. (handler.c, extract_char)");
        abort();
      }
    }
    return;
  }

  if (ch->followers || ch->master)
    die_follower(ch);

   if(ch->desc) {
     /* Forget snooping */
     if (ch->desc->snoop.snooping)
  ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

     if (ch->desc->snoop.snoop_by) {
  send_to_char("Your victim is no longer among us.\n\r",
           ch->desc->snoop.snoop_by);
  ch->desc->snoop.snoop_by->desc->snoop.snooping = 0;
     }
     ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;

   }

     if (ch->carrying)
  {
    /* transfer ch's objects to room */

    if (world[CHAR_REAL_ROOM(ch)].contents)  /* room nonempty */
    {
      /* locate tail of room-contents */
      for (i = world[CHAR_REAL_ROOM(ch)].contents; i->next_content;
         i = i->next_content);

      /* append ch's stuff to room-contents */
      i->next_content = ch->carrying;
    }
    else
       world[CHAR_REAL_ROOM(ch)].contents = ch->carrying;

    /* connect the stuff to the room */
    for (i = ch->carrying; i; i = i->next_content)
    {
      i->carried_by = 0;
      i->in_room = CHAR_REAL_ROOM(ch);
      i->in_room_v = CHAR_VIRTUAL_ROOM(ch);
    }
  }
        ch->carrying = NULL;
  if (ch->specials.fighting)
    stop_fighting(ch);

  for (k = combat_list; k ; k = next_char)
  {
    next_char = k->next_fighting;
    if (k->specials.fighting == ch)
      stop_fighting(k);
  }

  /* Must remove from room before removing the equipment! */
  was_in = CHAR_REAL_ROOM(ch);
  char_from_room(ch);

  /* clear equipment_list */
  for (l = 0; l < MAX_WEAR; l++)
    if (ch->equipment[l])
      obj_to_room(unequip_char(ch,l), was_in);

       /* pull the char from the list */

  if (ch == character_list)
     character_list = ch->next;
  else
  {
    for(k = character_list; (k) && (k->next != ch); k = k->next);
    if(k)
      k->next = ch->next;
    else {
      log_f("Trying to remove ?? from character_list. (handler.c, extract_char)");
      abort();
    }
  }

  GET_AC(ch) = 100;

  if (ch->desc)
  {
    if (ch->desc->original)
      do_return(ch, "", 0);
  }

  if (IS_NPC(ch))
  {
    if (ch->nr > -1) /* if mobile */
      mob_proto_table[ch->nr].number--;
    clearMemory(ch);
    free_char(ch);
    return;
  }

  if (ch->desc) {
    ch->desc->connected = CON_SLCT;
    SEND_TO_Q(MENU, ch->desc);
  }
}

/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functions
   which incorporate the actual player-data.
   *********************************************************************** */


struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  for (i = world[CHAR_REAL_ROOM(ch)].people, j = 1; i && (j <= number); i = i->next_in_room)
    if (isname(tmp, GET_NAME(i)))
      if (CAN_SEE(ch, i))  {
        if (j == number)
          return(i);
        j++;
      }

  return(0);
}

struct char_data *get_mortal_room_vis(struct char_data *ch, char *name)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  for (i = world[CHAR_REAL_ROOM(ch)].people, j = 1; i && (j <= number); i = i->next_in_room)
    if (isname(tmp, GET_NAME(i)))
      if (CAN_SEE(ch, i) && IS_MORTAL(i))  {
        if (j == number)
          return(i);
        j++;
      }

  return(0);
}

CHAR *get_char_vis(CHAR *ch, char *name)
{
  CHAR *tmp_char = NULL;
  char buf[MIL];
  char *tmp_name;
  int number = 0;
  int i = 0;

  /* check location */
  if ((tmp_char = get_char_room_vis(ch, name)))
    return tmp_char;

  strcpy(buf, name);
  tmp_name = buf;

  if (!(number = get_number(&tmp_name)))
    return NULL;

  /* Check for players first. */
  for (tmp_char = character_list, i = 1; tmp_char && (i <= number); tmp_char = tmp_char->next)
    if (isname(tmp_name, GET_NAME(tmp_char)) && !IS_NPC(tmp_char))
      if (CAN_SEE(ch, tmp_char))
      {
        if (i == number)
          return tmp_char;
        i++;
      }

  for (tmp_char = character_list, i = 1; tmp_char && (i <= number); tmp_char = tmp_char->next)
    if (isname(tmp_name, GET_NAME(tmp_char)))
      if (CAN_SEE(ch, tmp_char))
      {
        if (i == number)
          return tmp_char;
        i++;
      }

  return NULL;
}

struct char_data *get_char_vis_zone(struct char_data *ch, char *name)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  /* check location */
  if ((i = get_char_room_vis(ch, name)))
    return(i);

  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  /* Check for players first */
  for (i = character_list; i ; i = i->next)
    if (isname(tmp, GET_NAME(i)) && !IS_NPC(i) &&
        world[CHAR_REAL_ROOM(ch)].zone==world[CHAR_REAL_ROOM(i)].zone )
      if (CAN_SEE(ch, i))  return(i);


  for (i = character_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, GET_NAME(i)) &&
        world[CHAR_REAL_ROOM(ch)].zone==world[CHAR_REAL_ROOM(i)].zone )
      if (CAN_SEE(ch, i))  {
        if (j == number)
          return(i);
        j++;
      }

  return(0);
}

struct char_data *get_mob_vis(struct char_data *ch, char *name)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  /* check location */
  if ((i = get_char_room_vis(ch, name)))
    return(i);

  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  for (i = character_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, GET_NAME(i)))
      if (CAN_SEE(ch, i) && IS_NPC(i))  {
        if (j == number)
          return(i);
        j++;
      }

  return(0);
}

struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
        struct obj_data *list)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  for (i = list, j = 1; i && (j <= number); i = i->next_content)
    if (isname(tmp, OBJ_NAME(i)))
      if (CAN_SEE_OBJ(ch, i)) {
        if (j == number)
          return(i);
        j++;
      }
  return(0);
}


/*
Search a given equipment for an object, and return a pointer to that object
*/
struct obj_data *get_obj_in_equip_vis( struct char_data *ch, char *name,
                                       struct obj_data *equipment[], int *i)
{
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname, name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  for ((*i) = 0, j = 1; (*i) < MAX_WEAR; (*i)++)
    if (equipment[(*i)])
      if (CAN_SEE_OBJ(ch, equipment[(*i)]))
        if (isname(tmp, OBJ_NAME(equipment[(*i)]))) {
      if (j == number)
            return(equipment[(*i)]);
          j++;
    }

  return(0);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  /* scan items carried */
  if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
    return(i);

  /* scan room */
  if ((i = get_obj_in_list_vis(ch, name, world[CHAR_REAL_ROOM(ch)].contents)))
    return(i);

  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, OBJ_NAME(i)))
      if (CAN_SEE_OBJ(ch, i)) {
        if (j == number)
          return(i);
        j++;
      }
  return(0);
}

struct obj_data *get_obj_vis_in_rooms(struct char_data *ch, char *name)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, OBJ_NAME(i)))
      if (CAN_SEE_OBJ(ch, i) && i->in_room != NOWHERE) {
        if (j == number)
          return(i);
        j++;
      }
  return(0);
}

struct obj_data *create_money( int amount )
{
  struct obj_data *obj;
  struct extra_descr_data *new_descr;
  char buf[80];

  if(amount<=0)
  {
    log_f("ERROR: Try to create negative money.");
                str_dup(NULL);
  }

  if(real_object(33) > 0) {
    obj = read_object(33,VIRTUAL);
  }
  else {
    CREATE(obj, struct obj_data, 1);
    clear_object(obj);
    obj->item_number = -1;
    obj->next = object_list;
    object_list = obj;
  }
  CREATE(new_descr, struct extra_descr_data, 1);

  if(amount==1)
  {
    obj->name = str_dup("coin gold");
    obj->short_description = str_dup("a gold coin");
    obj->description = str_dup("One miserable gold coin.");

    new_descr->keyword = str_dup("coin gold");
    new_descr->description = str_dup("One miserable gold coin.");
  }
  else
  {
    obj->name = str_dup("coins gold");
    obj->short_description = str_dup("gold coins");
    obj->description = str_dup("A pile of gold coins.");

    new_descr->keyword = str_dup("coins gold");
    if(amount<10) {
      sprintf(buf,"There is %d coins.",amount);
      new_descr->description = str_dup(buf);
    }
    else if (amount<100) {
      sprintf(buf,"There is about %d coins",10*(amount/10));
      new_descr->description = str_dup(buf);
    }
    else if (amount<1000) {
      sprintf(buf,"It looks like something round %d coins",100*(amount/100));
      new_descr->description = str_dup(buf);
    }
    else if (amount<100000) {
      sprintf(buf,"You guess there is %d coins",1000*((amount/1000)+ number(0,(amount/1000))));
      new_descr->description = str_dup(buf);
    }
    else
      new_descr->description = str_dup("There is A LOT of coins");
  }

  new_descr->next = 0;
  obj->ex_description = new_descr;

  obj->obj_flags.type_flag = ITEM_MONEY;
  obj->obj_flags.wear_flags = ITEM_TAKE;
  obj->obj_flags.value[0] = amount;
  obj->obj_flags.cost = amount;

  return(obj);
}



/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
int generic_find(char *argument, int bitvector, CHAR *ch, CHAR **target_ch, OBJ **target_obj) {
  const char * ignore[] = {
    "the",
    "in",
    "on",
    "at",
    "\n"
  };

  char name[MIL];

  /* Eliminate spaces and "ignore" words. */
  while (*argument) {
    for (; *argument == ' '; argument++);

    int i = 0;

    for (; (name[i] = *(argument + i)) && (name[i] != ' '); i++);

    name[i] = 0;

    argument += i;

    if (search_block(name, ignore, TRUE) > -1) break;
  }

  if (!name[0]) return 0;

  if (target_ch && IS_SET(bitvector, FIND_CHAR_ROOM)) {
    if ((*target_ch = get_char_room_vis(ch, name))) {
      return FIND_CHAR_ROOM;
    }
  }

  if (target_ch && IS_SET(bitvector, FIND_CHAR_WORLD)) {
    if ((*target_ch = get_char_vis(ch, name))) {
      return FIND_CHAR_WORLD;
    }
  }

  if (target_obj && IS_SET(bitvector, FIND_OBJ_INV)) {
    if ((*target_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
      return FIND_OBJ_INV;
    }
  }

  if (target_obj && IS_SET(bitvector, FIND_OBJ_EQUIP)) {
    for (int i = 0; i < MAX_WEAR; i++) {
      OBJ *tmp_obj = EQ(ch, i);

      if (tmp_obj && isname(name, OBJ_NAME(tmp_obj))) {
        *target_obj = tmp_obj;

        return FIND_OBJ_EQUIP;
      }
    }
  }

  if (target_obj && IS_SET(bitvector, FIND_OBJ_ROOM)) {
    if ((*target_obj = get_obj_in_list_vis(ch, name, world[CHAR_REAL_ROOM(ch)].contents))) {
      return FIND_OBJ_ROOM;
    }
  }

  if (target_obj && IS_SET(bitvector, FIND_OBJ_WORLD)) {
    if ((*target_obj = get_obj_vis(ch, name))) {
      return FIND_OBJ_WORLD;
    }
  }

  return 0;
}
