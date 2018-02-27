/*
  spec.ecanyon.c - Specs for ElementalCanyonDyingWorld and ElementalCanyonDifficultWorld, by Geldrin

  Written for RoninMUD
*/

/* Includes */
/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ronin Includes */
#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "db.h"
#include "cmd.h"
#include "comm.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "mob.spells.h"
#include "spec_assign.h"
#include "spells.h"
#include "act.h"
#include "utility.h"

/* Rooms VNums */
#define ECANYON_ROOM_ORIGINAL_ECANYON_PORTAL  9400
#define ECANYON_ROOM_DYING_ECANYON_PORTAL     19400
#define ECANYON_ROOM_DIFFICULT_ECANYON_PORTAL 19500
#define ECANYON_ROOM_DYING_FIRE_CHAMBER       19445

/* Mob VNums */
#define ECANYON_MOB_DIFFICULT_EARTH_RULER     19508

/* Object VNums */
#define ECANYON_OBJ_SHIELD                    19400
#define ECANYON_OBJ_RING                      19501
#define ECANYON_OBJ_BREASTPLATE               19502
#define ECANYON_OBJ_JUG                       19524

/* Misc */
#define ECANYON_FIRE_CHAMBER_DAMAGE           200

#define ECANYON_PORTAL_STONE_DAMAGE           48
#define ECANYON_PORTAL_STONE_MANACOST         200

#define ECANYON_SHIELD_SPEC_CHANCE            3
#define ECANYON_SHIELD_SPEC_DURATION          3
#define ECANYON_SHIELD_NORMAL_AC_APPLY        0
#define ECANYON_SHIELD_SPEC_AC_APPLY          -10

#define ECANYON_RING_HP_AFFECT_SLOT           2
#define ECANYON_RING_BASE_HP_APPLY            0
#define ECANYON_RING_MIN_HP_GROWTH            20
#define ECANYON_RING_MAX_HP_GROWTH            100
#define ECANYON_RING_MIN_GROWTH_DAMAGE        20
#define ECANYON_RING_MAX_GROWTH_DAMAGE        40

#define ECANYON_BREASTPLATE_AC_AFFECT_SLOT    0
#define ECANYON_BREASTPLATE_BASE_AC_APPLY     0
#define ECANYON_BREASTPLATE_MIN_AC_GROWTH     -5
#define ECANYON_BREASTPLATE_MAX_AC_GROWTH     -18
#define ECANYON_BREASTPLATE_MIN_GROWTH_DMG    70
#define ECANYON_BREASTPLATE_MAX_GROWTH_DMG    120
#define ECANYON_BREASTPLATE_REMOVE_DMG_MULTI  50

#define ECANYON_JUG_MIN_MANA_RESTORED         2
#define ECANYON_JUG_MAX_MANA_RESTORED         12

/* Room Specs */

/* Teleport players into one of the elemental canyon zones randomly. */
int ecanyon_room_ECANYON_PORTAL_STONE(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch)) return FALSE;

    if (GET_MANA(ch) < ECANYON_PORTAL_STONE_MANACOST) return FALSE;

    act("As you approach, you feel the portal stones tugging at your life force.", FALSE, ch, 0, 0, TO_CHAR);

    return FALSE;
  }

  if (cmd == CMD_USE) {
    char buf[MIL];

    one_argument(arg, buf);

    if (!*buf) return FALSE;

    if (!isname(buf, "stones portal")) return FALSE;

    act("You place your hands on the portal stones.", FALSE, ch, 0, 0, TO_CHAR);
    act("The stone grows hot as energy is pulled from your body.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n places $s hands on the portal stones.", TRUE, ch, 0, 0, TO_ROOM);

    if (GET_MANA(ch) >= ECANYON_PORTAL_STONE_MANACOST) {
      act("The stones are blazing under your hands!", FALSE, ch, 0, 0, TO_CHAR);
      act("Suddenly, as if the sun had gone out for the blink of an eye, the world flickers.", TRUE, ch, 0, 0, TO_ROOM);
      act("Flicker... flicker... flicker...  flicker-flicker-flicker...", TRUE, ch, 0, 0, TO_ROOM);

      int teleport_room = NOWHERE;

      switch (CHAR_VIRTUAL_ROOM(ch)) {
        case ECANYON_ROOM_ORIGINAL_ECANYON_PORTAL:
          if (chance(50)) {
            teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;
          }
          else {
            teleport_room = ECANYON_ROOM_DIFFICULT_ECANYON_PORTAL;
          }
          break;
        case ECANYON_ROOM_DYING_ECANYON_PORTAL:
          if (chance(50)) {
            teleport_room = ECANYON_ROOM_ORIGINAL_ECANYON_PORTAL;
          }
          else {
            teleport_room = ECANYON_ROOM_DIFFICULT_ECANYON_PORTAL;
          }
          break;
        case ECANYON_ROOM_DIFFICULT_ECANYON_PORTAL:
          if (chance(50)) {
            teleport_room = ECANYON_ROOM_DYING_ECANYON_PORTAL;
          }
          else {
            teleport_room = ECANYON_ROOM_ORIGINAL_ECANYON_PORTAL;
          }
          break;
      }

      if (teleport_room == NOWHERE) return FALSE;

      for (CHAR *tel_ch = world[CHAR_REAL_ROOM(ch)].people, *tel_next = NULL; tel_ch; tel_ch = tel_next) {
        tel_next = tel_ch->next_in_room;

        if (IS_NPC(tel_ch)) continue;

        char_from_room(tel_ch);
        char_to_room(tel_ch, real_room(teleport_room));

        do_look(tel_ch, "\0", CMD_LOOK);
      }

      GET_MANA(ch) -= ECANYON_PORTAL_STONE_MANACOST;

      damage(ch, ch, ECANYON_PORTAL_STONE_DAMAGE, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

      return TRUE;
    }
    else {
      GET_MANA(ch) = 0;

      act("You feel drained, but the stones are lifeless under your hands.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n looks tired as $e removes $s hands from the stones.", TRUE, ch, 0, 0, TO_ROOM);

      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

/* Inflict damage when players enter the Fire Chamber. */
int ecanyon_room_fire_chamber(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch)) return FALSE;

    act("Your flesh begins to sizzle as it's seared by the overwhelming heat!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n begins to sizzle as $s flesh is seared by the overwhelming heat!", TRUE, ch, 0, 0, TO_ROOM);

    damage(ch, ch, ECANYON_FIRE_CHAMBER_DAMAGE, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

    return FALSE;
  }

  return FALSE;
}

/* Object Specs */

/* Shield spec. */
int ecanyon_obj_shield(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_TICK) {
    CHAR *owner = obj->equipped_by;

    if (!owner || obj != EQ(owner, WEAR_SHIELD)) return FALSE;

    if (OBJ_TIMER(obj) < 0 || OBJ_TIMER(obj) > ECANYON_SHIELD_SPEC_DURATION) {
      unequip_char(owner, WEAR_SHIELD);
      OBJ_TIMER(obj) = 0;
      if (OBJ_AFF0(obj).location == APPLY_ARMOR) {
        OBJ_AFF0(obj).modifier = ECANYON_SHIELD_NORMAL_AC_APPLY;
      }
      equip_char(owner, obj, WEAR_SHIELD);
    }

    if (OBJ_TIMER(obj)) {
      OBJ_TIMER(obj)--;

      if (!OBJ_TIMER(obj)) {
        act("You struggle with the immense weight of your beak shield as it tilts to the side.", FALSE, owner, obj, 0, TO_CHAR);
        act("$n struggles with $s beak shield as it tilts to the side.", FALSE, owner, obj, 0, TO_ROOM);

        unequip_char(owner, WEAR_SHIELD);
        OBJ_TIMER(obj) = 0;
        if (OBJ_AFF0(obj).location == APPLY_ARMOR) {
          OBJ_AFF0(obj).modifier = ECANYON_SHIELD_NORMAL_AC_APPLY;
        }
        equip_char(owner, obj, WEAR_SHIELD);
      }
    }

    if (!OBJ_TIMER(obj) && GET_OPPONENT(owner) && chance(ECANYON_SHIELD_SPEC_CHANCE)) {
      act("You struggle with the immense weight of your beak shield, shifting it into the perfect position.", FALSE, owner, obj, 0, TO_CHAR);
      act("$n struggles with $s beak shield, shifting it into the perfect position.", FALSE, owner, obj, 0, TO_ROOM);

      unequip_char(owner, WEAR_SHIELD);
      OBJ_TIMER(obj) = ECANYON_SHIELD_SPEC_DURATION;
      if (OBJ_AFF0(obj).location == APPLY_ARMOR) {
        OBJ_AFF0(obj).modifier = ECANYON_SHIELD_SPEC_AC_APPLY;
      }
      equip_char(owner, obj, WEAR_SHIELD);
    }

    return FALSE;
  }

  if (cmd == MSG_BEING_REMOVED) {
    CHAR *owner = obj->equipped_by;

    if (!owner || obj != EQ(owner, WEAR_SHIELD)) return FALSE;

    unequip_char(owner, WEAR_SHIELD);
    OBJ_TIMER(obj) = 0;
    if (OBJ_AFF0(obj).location == APPLY_ARMOR) {
      OBJ_AFF0(obj).modifier = ECANYON_SHIELD_NORMAL_AC_APPLY;
    }
    equip_char(owner, obj, WEAR_SHIELD);

    return FALSE;
  }

  return FALSE;
}

/* Ring growth spec. */
int ecanyon_obj_ring(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_TICK) {
    if (!OBJ_SPEC(obj)) {
      OBJ_SPEC(obj) = number(ECANYON_RING_MIN_HP_GROWTH, ECANYON_RING_MAX_HP_GROWTH);
    }

    if (time_info.hours != 6) {
      if (OBJ_TIMER(obj)) OBJ_TIMER(obj) = 0;

      return FALSE;
    }

    if (OBJ_TIMER(obj)) return FALSE;
    if (obj->affected[ECANYON_RING_HP_AFFECT_SLOT].location != APPLY_HIT) return FALSE;
    if (obj->affected[ECANYON_RING_HP_AFFECT_SLOT].modifier >= OBJ_SPEC(obj)) return FALSE;

    CHAR *owner = obj->equipped_by;

    if (!owner) return FALSE;

    int eq_pos = -1;

    if (obj == EQ(owner, WEAR_FINGER_R)) {
      eq_pos = WEAR_FINGER_R;
    }
    else if (obj == EQ(owner, WEAR_FINGER_L)) {
      eq_pos = WEAR_FINGER_L;
    }
    else {
      return FALSE;
    }

    bool inflict_damage = FALSE;

    unequip_char(owner, eq_pos);
    if (obj->affected[ECANYON_RING_HP_AFFECT_SLOT].modifier < OBJ_SPEC(obj)) {
      obj->affected[ECANYON_RING_HP_AFFECT_SLOT].modifier++;
      inflict_damage = TRUE;
    }
    OBJ_TIMER(obj) = 1;
    equip_char(owner, obj, eq_pos);

    if (inflict_damage) {
      act("Your stone ring rumbles as it painfully burrows into your finger.", FALSE, owner, 0, 0, TO_CHAR);
      act("$n's stone ring rumbles as it burrows into $s finger.", TRUE, owner, 0, 0, TO_ROOM);

      damage(owner, owner, number(ECANYON_RING_MIN_GROWTH_DAMAGE, ECANYON_RING_MAX_GROWTH_DAMAGE), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
    }
  }

  if (cmd == MSG_BEING_REMOVED) {
    if (obj->affected[ECANYON_RING_HP_AFFECT_SLOT].location != APPLY_HIT) return FALSE;

    CHAR *owner = obj->equipped_by;

    if (!owner) return FALSE;

    int eq_pos = -1;

    if (obj == EQ(owner, WEAR_FINGER_R)) {
      eq_pos = WEAR_FINGER_R;
    }
    else if (obj == EQ(owner, WEAR_FINGER_L)) {
      eq_pos = WEAR_FINGER_L;
    }

    if (eq_pos != WEAR_FINGER_R && eq_pos != WEAR_FINGER_L) return FALSE;

    int remove_damage = obj->affected[ECANYON_RING_HP_AFFECT_SLOT].modifier * 2;

    unequip_char(owner, eq_pos);
    obj->affected[ECANYON_RING_HP_AFFECT_SLOT].modifier = ECANYON_RING_BASE_HP_APPLY;
    OBJ_TIMER(obj) = 0;
    equip_char(owner, obj, eq_pos);

    if (remove_damage) {
      act("You strain to tear the ring free from your flesh, causing yourself great pain.", FALSE, owner, 0, 0, TO_CHAR);
      act("$n seems to be in pain as $e labors to separate a stone ring from $s flesh.", TRUE, owner, 0, 0, TO_ROOM);

      damage(owner, owner, remove_damage, TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
    }
    else {
      act("The stone ring shifts gently in your hands as you remove it.", FALSE, owner, 0, 0, TO_CHAR);
      act("$n's stone ring shifts gently in $s hands as $e removes it.", TRUE, owner, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}

/* Breastplate growth spec. */
int ecanyon_obj_breastplate(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_TICK) {
    if (!OBJ_SPEC(obj)) {
      OBJ_SPEC(obj) = number(ECANYON_BREASTPLATE_MIN_AC_GROWTH, ECANYON_BREASTPLATE_MAX_AC_GROWTH);
    }

    if (time_info.hours != 6) {
      if (OBJ_TIMER(obj)) OBJ_TIMER(obj) = 0;

      return FALSE;
    }

    if (OBJ_TIMER(obj)) return FALSE;
    if (obj->affected[ECANYON_BREASTPLATE_AC_AFFECT_SLOT].location != APPLY_ARMOR) return FALSE;
    if (obj->affected[ECANYON_BREASTPLATE_AC_AFFECT_SLOT].modifier <= OBJ_SPEC(obj)) return FALSE;

    CHAR *owner = obj->equipped_by;

    if (!owner || obj != EQ(owner, WEAR_BODY)) return FALSE;

    bool inflict_damage = FALSE;

    unequip_char(owner, WEAR_BODY);
    if (obj->affected[ECANYON_BREASTPLATE_AC_AFFECT_SLOT].modifier > OBJ_SPEC(obj)) {
      obj->affected[ECANYON_BREASTPLATE_AC_AFFECT_SLOT].modifier--;
      inflict_damage = TRUE;
    }
    OBJ_TIMER(obj) = 1;
    equip_char(owner, obj, WEAR_BODY);

    if (inflict_damage) {
      act("Your stone breastplate rumbles as it painfully burrows into your flesh.", FALSE, owner, 0, 0, TO_CHAR);
      act("$n's stone breastplate rumbles as it burrows into $s flesh.", TRUE, owner, 0, 0, TO_ROOM);

      damage(owner, owner, number(ECANYON_BREASTPLATE_MIN_GROWTH_DMG, ECANYON_BREASTPLATE_MAX_GROWTH_DMG), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
    }
  }

  if (cmd == MSG_BEING_REMOVED) {
    if (obj->affected[ECANYON_BREASTPLATE_AC_AFFECT_SLOT].location != APPLY_ARMOR) return FALSE;

    CHAR *owner = obj->equipped_by;

    if (!owner || obj != EQ(owner, WEAR_BODY)) return FALSE;

    bool inflict_damage = FALSE;

    if (obj->affected[ECANYON_BREASTPLATE_AC_AFFECT_SLOT].modifier < ECANYON_BREASTPLATE_BASE_AC_APPLY) {
      inflict_damage = TRUE;
    }

    int remove_damage = obj->affected[ECANYON_BREASTPLATE_AC_AFFECT_SLOT].modifier;

    unequip_char(owner, WEAR_BODY);
    obj->affected[ECANYON_BREASTPLATE_AC_AFFECT_SLOT].modifier = ECANYON_BREASTPLATE_BASE_AC_APPLY;
    OBJ_TIMER(obj) = 0;
    equip_char(owner, obj, WEAR_BODY);

    if (inflict_damage) {
      act("You strain to tear the breastplate free from your flesh, causing yourself great pain.", FALSE, owner, 0, 0, TO_CHAR);
      act("$n seems to be in pain as $e labors to separate a stone breastplate from $s flesh. ", TRUE, owner, 0, 0, TO_ROOM);

      damage(owner, owner, abs(remove_damage * ECANYON_BREASTPLATE_REMOVE_DMG_MULTI), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
    }
    else {
      act("The stone breastplate shifts in your hands as you remove it.", FALSE, owner, 0, 0, TO_CHAR);
      act("$n's stone breastplate shifts in $s hands as $e removes it.", TRUE, owner, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}

/* Restores a small amount of mana when a player drinks from it. */
int ecanyon_obj_jug(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_DRINK) {
    CHAR *owner = OBJ_CARRIED_BY(obj) != NULL ? OBJ_CARRIED_BY(obj) : OBJ_EQUIPPED_BY(obj) != NULL ? OBJ_EQUIPPED_BY(obj) : NULL;

    if (!ch || !IS_MORTAL(ch) || !owner || owner != ch) return FALSE;

    int condition = GET_COND(ch, THIRST);

    do_drink(owner, arg, cmd);

    if (GET_COND(owner, THIRST) > condition) {
      GET_MANA(owner) = MIN(GET_MAX_MANA(owner), (GET_MANA(owner) + number(ECANYON_JUG_MIN_MANA_RESTORED, ECANYON_JUG_MAX_MANA_RESTORED)));

      act("You feel energized after drinking from $p.", FALSE, owner, obj, 0, TO_CHAR);
    }

    return TRUE;
  }

  return FALSE;
}

/* Assign Specs */
void assign_ecanyon(void) {
  assign_room(ECANYON_ROOM_ORIGINAL_ECANYON_PORTAL, ecanyon_room_ECANYON_PORTAL_STONE);
  assign_room(ECANYON_ROOM_DYING_ECANYON_PORTAL, ecanyon_room_ECANYON_PORTAL_STONE);
  assign_room(ECANYON_ROOM_DIFFICULT_ECANYON_PORTAL, ecanyon_room_ECANYON_PORTAL_STONE);
  assign_room(ECANYON_ROOM_DYING_FIRE_CHAMBER, ecanyon_room_fire_chamber);
  assign_obj(ECANYON_OBJ_SHIELD, ecanyon_obj_shield);
  assign_obj(ECANYON_OBJ_RING, ecanyon_obj_ring);
  assign_obj(ECANYON_OBJ_BREASTPLATE, ecanyon_obj_breastplate);
  assign_obj(ECANYON_OBJ_JUG, ecanyon_obj_jug);
}