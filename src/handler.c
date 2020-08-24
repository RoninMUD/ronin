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
#include "aff_ench.h"

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

int isname(char *name, char *list) {
  if (!name || !*name || !list || !*list) return FALSE;

  if (!str_cmp(name, list)) return TRUE;

  char *temp_list = strdup(list);

  for (char *save = NULL, *token = strtok_r(temp_list, " \t\r\n", &save); token; token = strtok_r(NULL, " \t\r\n", &save)) {
    if (token && !str_cmp(name, token)) {
      free(temp_list);

      return TRUE;
    }
  }

  free(temp_list);

  return FALSE;
}

void affect_modify(CHAR *ch, int loc, int mod, long bitv, long bitv2, bool add) {
  aff_modify_char(ch, mod, loc, bitv, bitv2, add);
}

void affect_total(CHAR *ch) {
  aff_total_char(ch);
}

void affect_to_char(CHAR *ch, AFF *af) {
  aff_to_char(ch, af);
}

void affect_remove(CHAR *ch, AFF *af) {
  aff_remove(ch, af);
}

void affect_from_char(CHAR *ch, int type) {
  aff_from_char(ch, type);
}

bool affected_by_spell(CHAR *ch, int type) {
  return aff_affected_by(ch, type);
}

int duration_of_spell(CHAR *ch, int type) {
  return aff_duration(ch, type);
}

void affect_join(CHAR *ch, AFF *af, bool avg_dur, bool avg_mod) {
  aff_join(ch, af, avg_dur, avg_mod);
}

void remove_all_affects(CHAR *ch) {
  aff_remove_all(ch);
}

AFF *get_affect_from_char(CHAR *ch, int type) {
  return aff_get_from_char(ch, type);
}

void affect_apply(CHAR *ch, int type, sh_int duration, sbyte modifier, byte location, long bitvector, long bitvector2) {
  aff_apply(ch, type, duration, modifier, location, bitvector, bitvector2);
}


void char_from_room(CHAR *ch) {
  /* Keep this ordered so we can use a binary search on the list.*/
  const int club_rooms[] = {
    3076, 3079, 3081, 3083
  };

  if (!ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return;

  if (affected_by_spell(ch, SKILL_CAMP)) {
    printf_to_char(ch, "You quickly break camp.\n\r");
    act("$n quickly breaks camp.", TRUE, ch, 0, 0, TO_ROOM);

    affect_from_char(ch, SKILL_CAMP);

    if (binary_search_int_array(club_rooms, 0, NUMELEMS(club_rooms) - 1, CHAR_VIRTUAL_ROOM(ch)) == -1) {
      REMOVE_BIT(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CLUB);
    }
  }

  if (EQ(ch, WEAR_LIGHT) && (OBJ_TYPE(EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT) && (OBJ_VALUE(EQ(ch, WEAR_LIGHT), 2) != 0)) {
    ROOM_LIGHT(CHAR_REAL_ROOM(ch))--;
  }

  if (ROOM_PEOPLE(CHAR_REAL_ROOM(ch)) == ch) {
    ROOM_PEOPLE(CHAR_REAL_ROOM(ch)) = CHAR_NEXT_IN_ROOM(ch);
  }
  else {
    CHAR *tmp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch));

    while (tmp_ch && (CHAR_NEXT_IN_ROOM(tmp_ch) != ch)) {
      tmp_ch = CHAR_NEXT_IN_ROOM(tmp_ch);
    }

    CHAR_NEXT_IN_ROOM(tmp_ch) = CHAR_NEXT_IN_ROOM(ch);
  }

  CHAR_NEXT_IN_ROOM(ch) = NULL;
  ch->in_room_r = NOWHERE;
  ch->in_room_v = NOWHERE;
}


void char_to_room(CHAR *ch, int room) {
  const int immortal_room_vnum = 1212;
  const int mortal_room_vnum = 3054;

  if (!ch) return;

  if (room < 0) {
    room = 0;
  }

  if (IS_SET(ROOM_FLAGS(room), LOCK) && (GET_LEVEL(ch) < LEVEL_SUP) && (room != real_room(immortal_room_vnum)) && (room != real_room(mortal_room_vnum)) && !isname(ROOM_NAME(room), GET_NAME(ch))) {
    printf_to_char(ch, "The room is locked.  There may be a private conversation there.\n\r");

    IS_IMMORTAL(ch) ? char_to_room(ch, real_room(immortal_room_vnum)) : char_to_room(ch, real_room(mortal_room_vnum));

    return;
  }

  CHAR_NEXT_IN_ROOM(ch) = ROOM_PEOPLE(room);
  ROOM_PEOPLE(room) = ch;
  ch->in_room_r = room;
  ch->in_room_v = ROOM_VNUM(room);

  if (EQ(ch, WEAR_LIGHT) && (OBJ_TYPE(EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT) && (OBJ_VALUE(EQ(ch, WEAR_LIGHT), 2) != 0)) {
    ROOM_LIGHT(CHAR_REAL_ROOM(ch))++;
  }

  if ((GET_CLASS(ch) == CLASS_NINJA) && (GET_POS(ch) == POSITION_SWIMMING) && (ROOM_SECTOR_TYPE(room) != SECT_WATER_NOSWIM)) {
    GET_POS(ch) = POSITION_STANDING;
  }
}


void obj_to_char(OBJ *obj, CHAR *ch) {
  const int quest_card_vnum = 35;

  if (!obj || !ch) return;

  adjust_obj_list(obj, GET_CARRYING(ch));

  GET_CARRYING(ch) = obj;

  OBJ_CARRIED_BY(obj) = ch;
  OBJ_EQUIPPED_BY(obj) = NULL;
  OBJ_IN_ROOM(obj) = NOWHERE;
  OBJ_IN_ROOM_V(obj) = NOWHERE;

  if (GET_QUEST_STATUS(ch) == QUEST_RUNNING) {
    if (GET_QUEST_OBJ(ch) && (GET_QUEST_OBJ(ch) == obj)) {
      if (V_OBJ(obj) != quest_card_vnum) {
        printf_to_char(ch, "You have the quest item!  Return to the quest giver to complete the quest.\n\r");

        GET_QUEST_STATUS(ch) = QUEST_COMPLETED;
      }
      else {
        printf_to_char(ch, "You found a quest item!  When you have enough, return to the quest giver to complete the quest.\n\r");
      }
    }
  }

  if (OBJ_LOG(obj)) {
    char buf[MIL];

    snprintf(buf, sizeof(buf), "QSTINFO: %s has item %s (%d).", GET_DISP_NAME(ch), OBJ_SHORT(obj), V_OBJ(obj));

    switch (OBJ_LOG(obj)) {
      case 1:
        if (!IS_NPC(ch)) OBJ_LOG(obj) = 0;

        log_f("%s", buf);
        break;
      case 2:
        wizlog(buf, GET_LEVEL(ch), 4);
        break;
    }
  }

  if ((OBJ_TYPE(obj) == ITEM_SC_TOKEN) && !IS_NPC(ch)) {
    log_f("SUBLOG: %s has a token.", GET_NAME(ch));
  }
}


OBJ *obj_from_char(OBJ *obj) {
  if (!obj || OBJ_EQUIPPED_BY(obj)) return NULL;

  if ((OBJ_TYPE(obj) == ITEM_SC_TOKEN) && !IS_NPC(OBJ_CARRIED_BY(obj))) {
    log_f("SUBLOG: %s loses a token.", GET_DISP_NAME(OBJ_CARRIED_BY(obj)));
  }

  if (GET_CARRYING(OBJ_CARRIED_BY(obj)) == obj) {
    GET_CARRYING(OBJ_CARRIED_BY(obj)) = OBJ_NEXT_CONTENT(obj);
  }
  else {
    OBJ *tmp_obj = GET_CARRYING(OBJ_CARRIED_BY(obj));

    while (tmp_obj && (OBJ_NEXT_CONTENT(tmp_obj) != obj)) {
      tmp_obj = OBJ_NEXT_CONTENT(tmp_obj);
    }

    OBJ_NEXT_CONTENT(tmp_obj) = OBJ_NEXT_CONTENT(obj);
  }

  OBJ_CARRIED_BY(obj) = NULL;
  OBJ_EQUIPPED_BY(obj) = NULL;
  OBJ_NEXT_CONTENT(obj) = NULL;

  return obj;
}


int apply_ac(CHAR *ch, int pos) {
  if (!ch) return 0;

  OBJ *eq = EQ(ch, pos);

  if (!eq || (OBJ_TYPE(eq) != ITEM_ARMOR)) return 0;

  int ac = OBJ_VALUE(eq, 0);
  int multi = 1;

  switch (pos) {
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


bool equip_bits_zap(CHAR *ch, OBJ *obj) {
  if (!ch || !obj || !IS_MORTAL(ch)) return FALSE;

  const int WARMAGE_DAGGER = 17308;

  struct allow_info_t {
    int vnum;
    long bits;
    long bits2;
    bool classes[CLASS_LAST + 1];
  };

  const struct allow_info_t allow_list[] = {
    /*          vnum,     bits, bits2,   --, mu, cl, th, wa, ni, no, pa, ap, av, ba, co  */
    { WARMAGE_DAGGER, AFF_DUAL,     0, {  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } }
  };

  int bits[] = {
    AFF_DODGE, AFF_DUAL, AFF_FURY
  };

  int bits2[] = {
    AFF2_TRIPLE, AFF2_QUAD, AFF2_FORTIFICATION, AFF2_RAGE
  };

  bool zap = FALSE;

  for (int bit = 0; bit < NUMELEMS(bits); bit++) {
    if (IS_SET(OBJ_BITS(obj), bits[bit])) {
      zap = TRUE;

      for (int idx = 0; idx < NUMELEMS(allow_list); idx++) {
        if ((V_OBJ(obj) == allow_list[idx].vnum) && IS_SET(allow_list[idx].bits, bits[bit]) && allow_list[idx].classes[GET_CLASS(ch)]) {
          zap = FALSE;
        }
      }
    }
  }

  for (int bit2 = 0; bit2 < NUMELEMS(bits2); bit2++) {
    if (IS_SET(OBJ_BITS2(obj), bits2[bit2])) {
      zap = TRUE;

      for (int idx = 0; idx < NUMELEMS(allow_list); idx++) {
        if ((V_OBJ(obj) == allow_list[idx].vnum) && IS_SET(allow_list[idx].bits2, bits2[bit2]) && allow_list[idx].classes[GET_CLASS(ch)]) {
          zap = FALSE;
        }
      }
    }
  }

  return zap;
}


bool equip_char_ex(CHAR *ch, OBJ *obj, int pos, bool zap) {
  if (!ch || !obj || (pos < 0) || (pos > (MAX_WEAR - 1))) return FALSE;

  if (EQ(ch, pos) || OBJ_EQUIPPED_BY(obj) || OBJ_CARRIED_BY(obj) || (OBJ_IN_ROOM(obj) != NOWHERE) || (OBJ_IN_ROOM_V(obj) != NOWHERE)) return FALSE;

  bool owner_zap = FALSE, align_zap = FALSE, class_zap = FALSE, sc_zap = FALSE, affect_zap = FALSE, bits_zap = FALSE;

  if (IS_MORTAL(ch)) {
    if (IS_SET(OBJ_WEAR_FLAGS(obj), ITEM_QUESTWEAR)) {
      owner_zap = TRUE;

      for (int i = 0; owner_zap && (i < MAX_OBJ_OWNER_ID); i++) {
        if (OBJ_OWNER_ID(obj, i) == GET_ID(ch)) {
          owner_zap = FALSE;
        }
      }
    }

    if ((IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
      align_zap = TRUE;
    }

    if ((IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_MAGIC_USER) && (GET_CLASS(ch) == CLASS_MAGIC_USER)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_CLERIC) && (GET_CLASS(ch) == CLASS_CLERIC)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_THIEF) && (GET_CLASS(ch) == CLASS_THIEF)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_WARRIOR) && (GET_CLASS(ch) == CLASS_WARRIOR)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_NINJA) && (GET_CLASS(ch) == CLASS_NINJA)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_NOMAD) && (GET_CLASS(ch) == CLASS_NOMAD)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_PALADIN) && (GET_CLASS(ch) == CLASS_PALADIN)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_ANTIPALADIN) && (GET_CLASS(ch) == CLASS_ANTI_PALADIN)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_AVATAR) && (GET_CLASS(ch) == CLASS_AVATAR)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_BARD) && (GET_CLASS(ch) == CLASS_BARD)) ||
        (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_COMMANDO) && (GET_CLASS(ch) == CLASS_COMMANDO))) {
      class_zap = TRUE;
    }

    if (IS_WEAPON(obj)) {
      /* Templar SC2: Martial Training - Bypass class restrictions for weapons. */
      if (check_subclass(ch, SC_TEMPLAR, 2)) {
        class_zap = FALSE;
      }
    }
    else {
      /* Druid SC2: Adaptation - Bypass alignment and class restrictions for non-weapons. */
      if (check_subclass(ch, SC_DRUID, 2)) {
        align_zap = FALSE;
        class_zap = FALSE;
      }
    }

    if ((IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_ENCHANTER) && (GET_SC(ch) == SC_ENCHANTER)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_ARCHMAGE) && (GET_SC(ch) == SC_ARCHMAGE)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_DRUID) && (GET_SC(ch) == SC_DRUID)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_TEMPLAR) && (GET_SC(ch) == SC_TEMPLAR)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_ROGUE) && (GET_SC(ch) == SC_ROGUE)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_BANDIT) && (GET_SC(ch) == SC_BANDIT)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_WARLORD) && (GET_SC(ch) == SC_WARLORD)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_GLADIATOR) && (GET_SC(ch) == SC_GLADIATOR)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_RONIN) && (GET_SC(ch) == SC_RONIN)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_MYSTIC) && (GET_SC(ch) == SC_MYSTIC)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_RANGER) && (GET_SC(ch) == SC_RANGER)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_TRAPPER) && (GET_SC(ch) == SC_TRAPPER)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_CAVALIER) && (GET_SC(ch) == SC_CAVALIER)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_CRUSADER) && (GET_SC(ch) == SC_CRUSADER)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_DEFILER) && (GET_SC(ch) == SC_DEFILER)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_INFIDEL) && (GET_SC(ch) == SC_INFIDEL)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_BLADESINGER) && (GET_SC(ch) == SC_BLADESINGER)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_CHANTER) && (GET_SC(ch) == SC_CHANTER)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_LEGIONNAIRE) && (GET_SC(ch) == SC_LEGIONNAIRE)) ||
        (IS_SET(OBJ_SC_RES(obj), ITEM_ANTI_SC_MERCENARY) && (GET_SC(ch) == SC_MERCENARY))) {
      sc_zap = TRUE;
    }

    for (int i = 0; !affect_zap && (i < MAX_OBJ_AFFECT); i++) {
      if ((OBJ_AFF_LOC(obj, i) == APPLY_HIT) && ((hit_limit(ch) + OBJ_AFF_MOD(obj, i)) <= 0)) {
        affect_zap = TRUE;
      }
    }

    if ((!IS_SET(OBJ_WEAR_FLAGS(obj), ITEM_QUESTWEAR) || owner_zap) &&
        !IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_RENT) &&
        !IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_ALL_DECAY) &&
        !IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_EQ_DECAY)) {
      bits_zap = equip_bits_zap(ch, obj);
    }

    if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_MORTAL) || owner_zap || class_zap || sc_zap || align_zap || affect_zap || bits_zap) {
      if (zap) {
        act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);

        obj_to_char(obj, ch);
      }

      return FALSE;
    }

    if (IS_WEAPON(obj) && ((pos == WIELD) || (pos == HOLD)) && (GETOBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)) {
      if (zap) {
        act("$p falls from your grasp because of your weakness.", FALSE, ch, obj, 0, TO_CHAR);
        act("$p falls from $n's grasp because $e's too weak.", FALSE, ch, obj, 0, TO_ROOM);

        obj_to_char(obj, ch);
      }

      return FALSE;
    }
  }

  EQ(ch, pos) = obj;

  OBJ_EQUIPPED_BY(obj) = ch;
  OBJ_CARRIED_BY(obj) = NULL;
  OBJ_IN_ROOM(obj) = NOWHERE;
  OBJ_IN_ROOM_V(obj) = NOWHERE;

  if (OBJ_TYPE(obj) == ITEM_ARMOR) {
    GET_AC(ch) -= apply_ac(ch, pos);
  }

  for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
    affect_modify(ch, OBJ_AFF_LOC(obj, i), OBJ_AFF_MOD(obj, i), OBJ_BITS(obj), OBJ_BITS2(obj), TRUE);
  }

  affect_total(ch);

  return TRUE;
}


void equip_char(CHAR *ch, OBJ *obj, int pos) {
  equip_char_ex(ch, obj, pos, TRUE);
}


bool rent_equip_char(CHAR *ch, OBJ *obj, int pos) {
  return equip_char_ex(ch, obj, pos, FALSE);
}


OBJ *unequip_char(CHAR *ch, int pos) {
  if (!ch) return NULL;

  OBJ *obj = EQ(ch, pos);

  if (obj) {
    if (OBJ_TYPE(obj) == ITEM_ARMOR) {
      GET_AC(ch) += apply_ac(ch, pos);
    }

    EQ(ch, pos) = NULL;

    OBJ_CARRIED_BY(obj) = NULL;
    OBJ_EQUIPPED_BY(obj) = NULL;
    OBJ_IN_ROOM(obj) = NOWHERE;
    OBJ_IN_ROOM_V(obj) = NOWHERE;

    for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
      affect_modify(ch, OBJ_AFF_LOC(obj, i), OBJ_AFF_MOD(obj, i), OBJ_BITS(obj), OBJ_BITS2(obj), FALSE);
    }

    affect_total(ch);
  }

  return obj;
}


/*
A non-destructive method of getting the "dot number" in a string.
Returns the "dot number" and points the sub variable to the substring.
*/
int dot_number(char *str, char **sub_ptr) {
  char *dot_pos;

  if ((dot_pos = strchr(str, '.'))) {
    *sub_ptr = (dot_pos + 1);

    int dot_num = 0;

    for (int i = 0; i < (dot_pos - str); i++) {
      char c = *((dot_pos - 1) - i);

      if (!isdigit(c)) {
        return 0;
      }

      dot_num += (c - '0') * ((i > 0) ? (10 * i) : 1);
    }

    return dot_num;
  }

  *sub_ptr = str;

  return 1;
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
void obj_to_room(OBJ *object, int room)
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


void update_object(OBJ *obj, bool equipped) {
  if (!obj) return;

  CHAR *ch = equipped ? OBJ_EQUIPPED_BY(obj) : OBJ_CARRIED_BY(obj);

  if (ch && equipped && EQ(ch, WEAR_LIGHT) && (EQ(ch, WEAR_LIGHT) == obj)) {
    if (OBJ_VALUE(obj, 2) > 0) {
      OBJ_VALUE(obj, 2)--;

      if (OBJ_VALUE(obj, 2) == 0) {
        printf_to_char(ch, "The light from your %s flickers and fades as it goes dark.\n\r", fname(OBJ_NAME(obj)));

        ROOM_LIGHT(CHAR_REAL_ROOM(ch))--;
      }
    }
  }

  if (OBJ_TIMER(obj) > 0) {
    bool decay = TRUE;

    /* Druid SC2: Adaptation - Decay worn items only half of the time. */
    if (ch && equipped && !IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_RENT) && IS_MORTAL(ch) && check_subclass(ch, SC_DRUID, 2) && chance(50)) {
      decay = FALSE;
    }

    // Prestige Perk 22
    if (ch && !IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_RENT) && ((GET_PRESTIGE_PERK(ch) >= 22) && chance(10))) {
      decay = FALSE;
    }

    if (OBJ_IN_OBJ(obj) && (OBJ_TYPE(OBJ_IN_OBJ(obj)) == ITEM_AQ_ORDER)) {
      decay = FALSE;
    }

    if (decay) {
      if (IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_ALL_DECAY)) {
        if (equipped) {
          OBJ_TIMER(obj) -= 2;
        }
        else {
          OBJ_TIMER(obj) -= 1;
        }
      }
      else if (IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_EQ_DECAY)) {
        if (equipped) {
          OBJ_TIMER(obj) -= 1;
        }
      }
    }
  }

  if (OBJ_CONTAINS(obj)) {
    update_object(OBJ_CONTAINS(obj), FALSE);
  }

  if (OBJ_NEXT_CONTENT(obj)) {
    update_object(OBJ_NEXT_CONTENT(obj), FALSE);
  }
}

void update_char_objects(CHAR *ch) {
  for (int eq_pos = 0; eq_pos < MAX_WEAR; eq_pos++) {
    if (EQ(ch, eq_pos)) {
      update_object(EQ(ch, eq_pos), TRUE);
    }
  }

  if (GET_CARRYING(ch)) {
    update_object(GET_CARRYING(ch), FALSE);
  }
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

CHAR *get_char_ex(CHAR *ch, char *name, bool must_see) {
  if (!ch || !name) return NULL;

  char buf[MIL];

  /* Copy the name to buf and point to it with temp_name, since get_number() is destructive. */
  snprintf(buf, sizeof(buf), "%s", name);

  char *temp_name = buf;

  /* Pointer to the character we're interested in. */
  CHAR *temp_ch = get_char_room_ex(ch, temp_name, must_see);

  /* Check current room. */
  if (temp_ch) {
    return temp_ch;
  }

  /* Get the number of the character. */
  int number = get_number(&temp_name);

  /* Was the number input invalid? */
  if (!number) {
    return NULL;
  }

  /* Check players first. */
  temp_ch = character_list;
  for (int i = 1; temp_ch && (i <= number); temp_ch = temp_ch->next) {
    if (!IS_NPC(temp_ch) && isname(temp_name, GET_NAME(temp_ch)) && (!must_see || CAN_SEE(ch, temp_ch))) {
      if (i == number) {
        return temp_ch;
      }

      i++;
    }
  }

  /* Check NPCs. */
  temp_ch = character_list;
  for (int i = 1; temp_ch && (i <= number); temp_ch = temp_ch->next) {
    if (IS_NPC(temp_ch) && isname(temp_name, GET_NAME(temp_ch)) && (!must_see || CAN_SEE(ch, temp_ch))) {
      if (i == number) {
        return temp_ch;
      }

      i++;
    }
  }

  return NULL;
}

CHAR *get_char_vis(CHAR *ch, char *name) {
  return get_char_ex(ch, name, TRUE);
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

OBJ* get_obj_equipped_by_name(CHAR *ch, char *obj_name)
{
  int number = 0;
  int item_count = 1;
  char* sub_ptr = NULL;
  OBJ* target_obj = NULL;

  if ((number = dot_number(obj_name, &sub_ptr)))
  {
    for (int i = 0; i < MAX_WEAR; i++)
    {
      if (EQ(ch, i) && isname(sub_ptr, OBJ_NAME(EQ(ch, i))))
      {
        if (item_count == number)
        {
          target_obj = EQ(ch, i);
          break;
        }
        else
        {
          ++item_count;
        }
      }
    }
  }
  return target_obj;
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
  char name[MIL];

  argument = skip_spaces(one_argument(argument, name));

  if (!*name || *argument) return FIND_NOT_FOUND;

  if (target_ch) {
    if (IS_SET(bitvector, FIND_CHAR_ROOM) && (*target_ch = get_char_room_vis(ch, name))) {
      return FIND_CHAR_ROOM;
    }

    if (IS_SET(bitvector, FIND_CHAR_WORLD) && (*target_ch = get_char_vis(ch, name))) {
      return FIND_CHAR_WORLD;
    }
  }

  if (target_obj) {
    if (IS_SET(bitvector, FIND_OBJ_INV) && (*target_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
      return FIND_OBJ_INV;
    }

    if (IS_SET(bitvector, FIND_OBJ_EQUIP))
    {
      *target_obj = get_obj_equipped_by_name(ch, name);
      return FIND_OBJ_EQUIP;
    }

    if (IS_SET(bitvector, FIND_OBJ_ROOM) && (*target_obj = get_obj_in_list_vis(ch, name, world[CHAR_REAL_ROOM(ch)].contents))) {
      return FIND_OBJ_ROOM;
    }

    if (IS_SET(bitvector, FIND_OBJ_WORLD) && (*target_obj = get_obj_vis(ch, name))) {
      return FIND_OBJ_WORLD;
    }
  }

  return FIND_NOT_FOUND;
}

/* Returns a pointer to the first MOB in the specified room with the specified VNUM, or NULL if not found. */
CHAR *get_mob_by_vnum_in_room(int mob_vnum, int rm) {
  if (((rm < 0) || (rm > top_of_world)) || (mob_vnum < 0)) return NULL;

  for (CHAR *temp_vict = world[rm].people, *next_in_room; temp_vict; temp_vict = next_in_room) {
    next_in_room = temp_vict->next_in_room;

    if (!IS_NPC(temp_vict)) continue;

    if (V_MOB(temp_vict) == mob_vnum) return temp_vict;
  }

  return NULL;
}

/* Returns a pointer to the first OBJ in the specified room with the specified VNUM, or NULL if not found. */
OBJ *get_obj_by_vnum_in_room(int obj_vnum, int rm) {
  if (((rm < 0) || (rm > top_of_world)) || (obj_vnum < 0)) return NULL;

  for (OBJ *temp_obj = world[rm].contents, *next_content; temp_obj; temp_obj = next_content) {
    next_content = temp_obj->next_content;

    if (V_OBJ(temp_obj) == obj_vnum) return temp_obj;
  }

  return NULL;
}

OBJ *get_obj_in_list_ex(CHAR *ch, char *name, OBJ *list, bool must_see) {
  if (!ch || !name || !list) return NULL;

  int number = 0;
  char *sub_ptr = NULL;

  if ((number = dot_number(name, &sub_ptr))) {
    OBJ *obj = list;

    for (int i = 1; obj && (i <= number); obj = obj->next_content) {
      if (isname(sub_ptr, OBJ_NAME(obj))) {
        if (!must_see || CAN_SEE_OBJ(ch, obj)) {
          if (i == number) {
            return obj;
          }

          i++;
        }
      }
    }
  }

  return NULL;
}

OBJ *get_obj_in_list_vis(CHAR *ch, char *name, OBJ *list) {
  return get_obj_in_list_ex(ch, name, list, TRUE);
}

CHAR *get_char_room_ex(CHAR *ch, char *name, bool must_see) {
  if (!ch || !name) return NULL;

  int number = 0;
  char *sub_ptr = NULL;

  if ((number = dot_number(name, &sub_ptr))) {
    CHAR *vict = ROOM_PEOPLE(CHAR_REAL_ROOM(ch));

    for (int i = 1; vict && (i <= number); vict = vict->next_in_room) {
      if (isname(sub_ptr, GET_NAME(vict))) {
        if (!must_see || CAN_SEE(ch, vict)) {
          if (i == number) {
            return vict;
          }

          i++;
        }
      }
    }
  }

  return NULL;
}

CHAR *get_char_room_vis(CHAR *ch, char *name) {
  return get_char_room_ex(ch, name, TRUE);
}
