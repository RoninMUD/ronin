/*
  spec.invasion.c - Specs for Demon Invasion, by Night

  Written by Alan K. Miles for RoninMUD
*/

/* System Includes */
#include <string.h>

/* Ronin Includes */
#include "constants.h"
#include "structs.h"
#include "utils.h"
#include "act.h"
#include "db.h"
#include "cmd.h"
#include "comm.h"
#include "enchant.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "mob.spells.h"
#include "spec_assign.h"
#include "spells.h"
#include "subclass.h"
#include "utility.h"


/* Defines */

/* Rooms*/
#define TEMPLE            3001
#define MARKET_SQUARE     3014

#define OLD_PATH          5401  /* An Old Path */
#define DEEP_PASSAGE      5973  /* Deep, descending passage */
#define ABYSSIAN_WALK     16640 /* The Abyssian Walk */
#define DECENDING_DARK    16921 /* Descending into the Darkness */
#define CENTRAL_COURTYARD 25040 /* The Central Courtyard */

#define ICE_CRYSTALS_ROOM 27760
#define JUNCTION_ROOM     27762
#define STAIRCASE_ROOM    27770
#define STORAGE_ROOM      27778
#define LAVA_HAZARD       27787
#define SEARING_ROOM      27789
#define TEAR_ROOM         27793

#define BEBILITH_ROOM     27738
#define MYRDON_ROOM       27740
#define SHADOWRAITH_ROOM  27748
#define TRYSTA_ROOM       27763
#define SHOMED_ROOM       27761
#define VELXOK_ROOM       27765
#define STRAM_ROOM        27779
#define TOHIRI_ROOM       27785
#define ANISTON_ROOM      27789
#define LAW_ROOM          27791
#define CHAOS_ROOM        27792

#define DT_ROOM           27749
#define HAZARD_ROOM       27787

#define TELEPORT_START    27700
#define TELEPORT_END      27788

#define MAZE_MIN          27794
#define MAZE_MAX          27799

/* Mobiles */
#define YETI              27700
#define TROLL             27701
#define BASILISK          27702
#define IMP               27710
#define LEMURE            27711
#define BEBILITH          27712
#define OSYLUTH           27713
#define HELLCAT           27714
#define GELUGON           27715
#define BAATEZU           27716
#define DEMON_SPAWN       27717

#define MYRDON            27720
#define SHADOWRAITH       27721
#define SHOMED            27722
#define TRYSTA            27723
#define VELXOK            27724
#define STRAM             27725
#define TOHIRI            27726
#define ANISTON           27727
#define LAW               27728
#define CHAOS             27729
#define XYKLOQTIUM        27730

#define GATE_MIN          IMP
#define GATE_MAX          DEMON_SPAWN

/* Objects */
#define WRISTBAND         27721
#define HORN              27723
#define MANTLE            27725
#define CIRCLET           27726
#define TACTICAL          27727
#define FROSTBRAND        27728
#define BLADED_SHIELD     27729
#define LANTERN           27730

#define LOCKPICKS         27740
#define DART              27741
#define BARBARIANS_RAGE   27742
#define FLUTE             27743
#define MAP               27744
#define LOCKET            27745
#define ORB               27746
#define CRACKED_ORB       27747
#define MANDATE           27748
#define SLIVER            27749

#define ICE_WALL          27750
#define BROKEN_ICE_WALL   27751
#define VORTEX            27752

/* Other */
#define WEAKENED         (1 << 0)
#define ATTACKED         (1 << 1)

/* Strings */
#define STRAM_SNIPE_ENCH_NAME    "Sniped by Stram"
#define ANISTON_CRUSH_ENCH_NAME  "Crushed Armor"
#define CHAOS_BITE_ENCH_NAME     "Gaping Neck Wound"
#define CHAOS_TENDRILS_ENCH_NAME "Shadowy Tendrils"

#define HORN_ENCH_NAME           "Tranquility of the Sea"


/* Mobile Specs */


int yeti_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    int rnd = number(1, 100);

    /* Simple message spec to make the mob more interesting. */
    if (rnd <= 10) {
      act("$n sniffs the air and gnashes $s teeth hungrily.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 20) {
      act("$n bellows out a primal roar!", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}


int troll_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    int rnd = number(1, 100);

    /* Simple message spec to make the mob more interesting. */
    if (rnd <= 10) {
      act("$n grinds and chatters $s icicle-like teeth.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 20) {
      act("$n looks around, seeking a hapless victim to torment.", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}


int basilisk_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    /* 3% Petrify attacker. */
    if (chance(3)) {
      CHAR *vict = GET_OPPONENT(mob);

      if (!vict) return FALSE;

      act("$n gazes deep into $N's eyes and turns $M to stone!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n gazes deep into your eyes and turns you to stone!", FALSE, mob, 0, vict, TO_VICT);

      spell_petrify(GET_LEVEL(mob), mob, vict, 0);
    }

    return FALSE;
  }

  return FALSE;
}


int imp_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    int rnd = number(1, 100);

    /* Simple message spec to make the mob more interesting. */
    if (rnd <= 10) {
      act("$n cackles maniacally to $mself.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 20) {
      act("$n swears profusely in $s own demonic tongue.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 30) {
      act("$n flicks $s tail about, seeking another victim.", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}


int lemure_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    int rnd = number(1, 100);

    /* Simple message spec to make the mob more interesting. */
    if (rnd <= 10) {
      act("$n drools blood from $s gaping mouth.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 20) {
      act("$n groans in the agony of $s twisted body.", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}


int bebilith_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    int rnd = number(1, 100);

    /* Simple message spec to make the mob more interesting. */
    if (rnd <= 10) {
      act("$n clacks $s massive bladed appendages together in a threatening gesture.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 20) {
      act("$n shifts $s bulk about on $s six multi-jointed legs.", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}


int osyluth_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    int rnd = number(1, 100);

    /* Simple message spec to make the mob more interesting. */
    if (rnd <= 10) {
      act("$n rakes $s sharp claws across the floor.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 20) {
      act("$n shudders and twiches violently, $s body wracked with pain.", FALSE, mob, 0, 0, TO_ROOM);
    }

    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    /* 25% Sting random victim. */
    if (chance(25)) {
      CHAR *vict = get_random_victim_fighting(mob);

      /* Don't spec if there's no valid target. */
      if (!vict) return FALSE;

      act("$n stings $N with $s tail!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n stings you with $s tail!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, number(8, 24), TYPE_UNDEFINED, DAM_PHYSICAL);
    }

    return FALSE;
  }

  return FALSE;
}


int hellcat_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    int rnd = number(1, 100);

    /* Simple message spec to make the mob more interesting. */
    if (rnd <= 10) {
      act("$n growls menacingly.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 20) {
      act("$n sniffs the air, seeking fresh prey.", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}


int gelugon_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Block and attack characters trying to exit Down in Staircase Room. */
  if (cmd == CMD_DOWN) {
    if (!ch || IS_IMMORTAL(ch) || (V_ROOM(mob) != STAIRCASE_ROOM)) return FALSE;

    /* Attack the character if not engaged in combat. */
    if (!GET_OPPONENT(mob) && IS_MORTAL(ch)) {
      act("$n blocks $N's way and attacks!", FALSE, mob, 0, ch, TO_NOTVICT);
      act("$n blocks your way and attacks!", FALSE, mob, 0, ch, TO_VICT);

      hit(mob, ch, TYPE_HIT);
    }
    /* Otherwise just block them. */
    else {
      act("$n blocks $N's way!", FALSE, mob, 0, ch, TO_NOTVICT);
      act("$n blocks your way!", FALSE, mob, 0, ch, TO_VICT);
    }

    return TRUE;
  }

  if (cmd == MSG_MOBACT) {
    int rnd = number(1, 100);

    /* Simple message spec to make the mob more interesting. */
    if (rnd <= 10) {
      act("$n clacks and chitters as $e searches for intruders.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 20) {
      act("$n changes $s stance slightly, chitinous plates scraping together.", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}


int baatezu_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    int rnd = number(1, 100);

    /* Simple message spec to make the mob more interesting. */
    if (rnd <= 10) {
      act("$n flexes $s bat-like wings, $s corded muscles rippling with the act.", FALSE, mob, 0, 0, TO_ROOM);
    }
    else if (rnd <= 20) {
      act("$n slams $s tail against the ground threateningly.", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}


/* Thief-like specs. */
int myrdon_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    int rnd = number(1, 100);

    /* 20% Coin Toss to all attackers in the room. */
    if (rnd <= 20) {
      act("$n tosses a hail of coins about $m with deadly precision.", FALSE, mob, 0, 0, TO_ROOM);

      for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        /* Ignore non-mortals and non-attackers. */
        if (!IS_MORTAL(vict) || (GET_OPPONENT(vict) != mob)) continue;

        act("You are battered by a hail of coins thrown by $n!", FALSE, mob, 0, vict, TO_VICT);

        damage(mob, vict, number(50, 100), TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
    /* 20% Kick dirt and blind a random attacker. */
    else if (rnd <= 40) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      if (!IS_AFFECTED(vict, AFF_BLIND)) {
        act("$n kicks dirt in $N's eyes, effectively blinding $M!", FALSE, mob, 0, vict, TO_NOTVICT);
        act("$n kicks dirt in your eyes, effectively blinding you!", FALSE, mob, 0, vict, TO_VICT);

        affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_HITROLL, -4, AFF_BLIND, 0);
        affect_apply(vict, SPELL_BLINDNESS, 0, APPLY_AC, 40, AFF_BLIND, 0);
      }
      else {
        act("$n kicks dirt in $N's eyes, but $E is already blind!", FALSE, mob, 0, vict, TO_NOTVICT);
        act("$n kicks dirt in your eyes, but you are already blind!", FALSE, mob, 0, vict, TO_VICT);
      }
    }
    /* 45% Circle the tank. */
    else if (rnd <= 85) {
      CHAR *vict = GET_OPPONENT(mob);

      if (!vict) return FALSE;

      act("$n circles behind $N's back and stabs $s weapon into $S back!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n circles behind you and stabs $s weapon into your back!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, (calc_hit_damage(mob, vict, NULL, 0, RND_NRM) * 4), TYPE_UNDEFINED, DAM_PHYSICAL);
    }
    /* 15% Circle fail. */
    else {
      CHAR *vict = GET_OPPONENT(mob);

      if (!vict) return FALSE;

      act("$n attempts to circle behind $N's back, but fails.", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n attempts to circle behind your back, but fails.", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, 0, TYPE_UNDEFINED, DAM_NO_BLOCK);
    }

    return FALSE;
  }

  return FALSE;
}


/* Ninja-like specs. */
int shadowraith_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Simulate a 2nd attack per round. */
  if (cmd == MSG_VIOLENCE) {
    CHAR *vict = GET_OPPONENT(mob);

    if (!vict) return FALSE;

    hit(mob, vict, TYPE_HIT);

    return FALSE;
  }

  if (cmd == MSG_MOBACT) {
    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    /* 50% Throwing Stars at random attacker. */
    if (chance(50)) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n makes a series of quick movements and $N is pelted with throwing stars!", TRUE, mob, 0, vict, TO_NOTVICT);
      act("$n makes a series of quick movements and you are pelted with throwing stars!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, number(100, 200), TYPE_UNDEFINED, DAM_PHYSICAL);
    }

    return FALSE;
  }

  /* Apply random affect to Wristband of the Assassin when Shadowraith dies and is wearing it (e.g. she was loading it). */
  if (cmd == MSG_DIE) {
    OBJ *wristband = EQ(mob, WEAR_WRIST_R) ? EQ(mob, WEAR_WRIST_R) : EQ(mob, WEAR_WRIST_L);

    if (!wristband || (V_OBJ(wristband) != WRISTBAND) || OBJ_AFF_LOC(wristband, 2) || OBJ_AFF_MOD(wristband, 2)) return FALSE;

    const int wristband_skill_list[] = {
      APPLY_SKILL_ASSAULT,
      APPLY_SKILL_BACKSTAB,
      APPLY_SKILL_PUMMEL
    };

    int wristband_location = 0;

    wristband_location = wristband_skill_list[number(1, NUMELEMS(wristband_skill_list)) - 1];

    OBJ_AFF_LOC(wristband, 2) = wristband_location;
    OBJ_AFF_MOD(wristband, 2) = 5;

    return FALSE;
  }

  return FALSE;
}


/* Nomad-like specs. */
int shomed_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    bool stopped_berserking = FALSE;

    /* 10% Stop Berserking and remove Dual bit. */
    if (IS_SET(GET_AFF(mob), AFF_DUAL) && chance(10)) {
      act("$n seems exhausted from $s berserk frenzy and calms down.", TRUE, mob, 0, 0, TO_ROOM);

      REMOVE_BIT(GET_AFF(mob), AFF_DUAL);

      stopped_berserking = TRUE;
    }

    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    int rnd = number(1, 100);

    /* Don't Berserk this MOBACT if we just stopped. */
    if (stopped_berserking) {
      rnd += 30;
    }

    /* 30% Berserk and set Dual bit. */
    if (rnd <= 30) {
      if (!IS_SET(GET_AFF(mob), AFF_DUAL)) {
        act("$n works $mself into a berserk frenzy!", TRUE, mob, 0, 0, TO_ROOM);

        SET_BIT(GET_AFF(mob), AFF_DUAL);
      }
    }
    /* 50% Batter a random target. */
    else if (rnd <= 80) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n pounds on $N with $s fists.", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$N pounds on you with $s fists.", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, calc_position_damage(GET_POS(vict), GET_LEVEL(mob) * 2), SKILL_BATTER, DAM_PHYSICAL);

      if ((CHAR_REAL_ROOM(vict) != NOWHERE) && (GET_POS(vict) > POSITION_SITTING))
      {
        GET_POS(vict) = POSITION_SITTING;
      }
    }
    /* 20% Batter fail. */
    else {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n tried to batter $N, but misses.", FALSE, mob, NULL, vict, TO_NOTVICT);
      act("$N tried to batter you, but misses.", FALSE, mob, NULL, vict, TO_VICT);

      damage(mob, vict, 0, TYPE_UNDEFINED, DAM_NO_BLOCK);
    }

    return FALSE;
  }

  return FALSE;
}


/* Bard-like specs. */
int trysta_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    /* Backflip every mobact. */
    CHAR *vict = GET_OPPONENT(mob);

    if (vict) {
      act("$n flips into the air and lands behind $N!", TRUE, mob, 0, vict, TO_NOTVICT);
      act("$n flips into the air and lands behind you!", FALSE, mob, 0, vict, TO_VICT);

      hit(mob, vict, TYPE_HIT);
    }

    int rnd = number(1, 100);

    /* 10% Total Recall */
    if (rnd <= 10) {
      act("$n sings 'Ilu lme ier, hini lme ier...'", FALSE, mob, 0, 0, TO_ROOM);

      for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        /* Don't teleport non-mortals. */
        if (!IS_MORTAL(vict)) continue;

        act("$n disappears!", TRUE, vict, 0, 0, TO_ROOM);

        char_from_room(vict);
        char_to_room(vict, real_room(TEMPLE));

        act("$n appears in the room suddenly!", TRUE, vict, 0, 0, TO_ROOM);

        do_look(vict, "", CMD_LOOK);
      }

      return FALSE;
    }
    /* 15% Warchant */
    else if (rnd <= 25) {
      act("$n sings 'Quen nwalme nin...'", FALSE, mob, 0, 0, TO_ROOM);
      printf_to_room(CHAR_REAL_ROOM(mob), "The sounds of war echo in your mind!\n\r");

      for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        /* Don't warchant immortals or mobs that aren't attackers. */
        if (IS_IMMORTAL(vict) || (IS_NPC(vict) && (GET_OPPONENT(vict) != mob))) continue;

        bool has_warchant = FALSE;

        /* Iterate through the victim's affects and see if they already have the bad Warchant affect. */
        for (AFF *temp_aff = vict->affected; temp_aff; temp_aff = temp_aff->next) {
          if ((temp_aff->type == SPELL_WARCHANT) && (temp_aff->location == APPLY_HITROLL) && (temp_aff->modifier < 0)) {
            has_warchant = TRUE;

            break;
          }
        }

        if (has_warchant) continue;

        affect_apply(vict, SPELL_WARCHANT, 5, -4, APPLY_HITROLL, 0, 0);

        act("$n grows weak with panic!", FALSE, vict, 0, 0, TO_ROOM);
        printf_to_char(vict, "You grow weak with panic!\n\r");
      }

      return FALSE;
    }
    /* 30% Blindness */
    else if (rnd <= 55) {
      act("$n sings 'Lumbo mi olos...'", FALSE, mob, 0, 0, TO_ROOM);
      printf_to_room(CHAR_REAL_ROOM(mob), "Flashes of light fill the room!\n\r");

      for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        /* Don't blind immortals or mobs that aren't attackers. */
        if (IS_IMMORTAL(vict) || (IS_NPC(vict) && (GET_OPPONENT(vict) != mob))) continue;

        /* Don't blind characters that are already blind or that are immune to blindness. */
        if (IS_SET(GET_AFF(vict), AFF_BLIND) || IS_IMMUNE(vict, IMMUNE_BLINDNESS)) continue;

        affect_apply(vict, SPELL_BLINDNESS, 2, -4, APPLY_HITROLL, AFF_BLIND, 0);
        affect_apply(vict, SPELL_BLINDNESS, 2, 40, APPLY_AC, AFF_BLIND, 0);

        act("$n seems to be blinded!", TRUE, vict, 0, 0, TO_ROOM);
        printf_to_char(vict, "You have been blinded!\n\r");
      }

      return FALSE;
    }
    /* 30% Lethal Fire */
    else if (rnd <= 85) {
      act("$n sings 'Nuruhuine...'", FALSE, mob, 0, 0, TO_ROOM);
      printf_to_room(CHAR_REAL_ROOM(mob), "Blue flames erupt from the aether!\n\r");

      for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        /* Don't burn immortals or mobs that aren't attackers. */
        if (IS_IMMORTAL(vict) || (IS_NPC(vict) && (GET_OPPONENT(vict) != mob))) continue;

        act("$n is burned by the azure fire!", TRUE, vict, 0, 0, TO_ROOM);
        printf_to_char(vict, "You are burned by the azure fire!\n\r");

        damage(mob, vict, (GET_LEVEL(mob) / 2) + number(180, 230), TYPE_UNDEFINED, DAM_MAGICAL);
      }

      return FALSE;
    }

    return FALSE;
  }

  return FALSE;
}


/* Velxok heals indefinitely to 1/2 HP until hit by Dart.  He also will teleport combatants. */
int velxok_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Note: Mob state is tracked using its Bank variable. */

  if (cmd == MSG_VIOLENCE) {
    if (!IS_SET(GET_BANK(mob), ATTACKED)) {
      SET_BIT(GET_BANK(mob), ATTACKED);
    }

    /* If fighting, and not wimpy, set wimpy. */
    if (GET_OPPONENT(mob) && !IS_SET(GET_ACT(mob), ACT_WIMPY)) {
      SET_BIT(GET_ACT(mob), ACT_WIMPY);
    }

    return FALSE;
  }

  /* Attack the first mortal that enters the room. */
  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch) || GET_OPPONENT(mob)) return FALSE;

    if ((V_ROOM(mob) == VELXOK_ROOM) && !IS_SET(GET_BANK(mob), ATTACKED)) {
      do_say(mob, "Intruder!", CMD_SAY);
    }
    else if (IS_SET(GET_BANK(mob), ATTACKED)) {
      do_say(mob, "I'll not be taken by surprise!", CMD_SAY);
    }

    hit(mob, ch, TYPE_HIT);

    return FALSE;
  }

  if (cmd == MSG_MOBACT) {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);

    /* If not fighting, and wimpy, remove wimpy. */
    if (!GET_OPPONENT(mob) && IS_SET(GET_ACT(mob), ACT_WIMPY)) {
      REMOVE_BIT(GET_ACT(mob), ACT_WIMPY);

      return FALSE;
    }

    /* Set HP to 1/2 if not affected by Dart spec. */
    if (!IS_SET(GET_BANK(mob), WEAKENED) && (GET_HIT(mob) < (GET_MAX_HIT(mob) / 2))) {
      do_say(mob, "My magic can sustain me indefinitely!", CMD_SAY);

      GET_HIT(mob) = (GET_MAX_HIT(mob) / 2);
    }

    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    /* Taunt players if not affected by Dart spec. */
    if (!IS_SET(GET_BANK(mob), WEAKENED)) {
      int rnd = number(1, 100);

      if (rnd <= 20) {
        do_say(mob, "Hahaha, you fools.  You cannot defeat one so powerful as me!", CMD_SAY);
      }
      else if (rnd <= 40) {
        do_say(mob, "So, you have chosen the death...", CMD_SAY);
      }
      else if (rnd <= 60) {
        do_say(mob, "Pathetic.  Watch me rain fire upon your corpses!", CMD_SAY);
      }
    }
    /* Otherwise, print some different messages. */
    else {
      int rnd = number(1, 100);

      if (rnd <= 20) {
        do_say(mob, "What?!  My powers are waning!", CMD_SAY);
      }
      else if (rnd <= 40) {
        do_say(mob, "This cannot be!  I will not let you defeat me!", CMD_SAY);
      }
      else if (rnd <= 60) {
        do_say(mob, "If you strike me down, I shall become more powerful than you can possibly imagine!", CMD_SAY);
      }
    }

    /* Don't teleport players to these rooms. */
    const int teleport_blacklist[] = {
       BEBILITH_ROOM,
       MYRDON_ROOM,
       SHADOWRAITH_ROOM,
       TRYSTA_ROOM,
       SHOMED_ROOM,
       VELXOK_ROOM,
       STRAM_ROOM,
       TOHIRI_ROOM,
       DT_ROOM,
       HAZARD_ROOM
    };

    /* 15% Teleport random player to a random room in the zone, except for blacklisted rooms. */
    if (chance(15)) {
      CHAR *vict = get_random_victim_fighting(mob);

      /* Reduced chance to teleport the tank. */
      if (!vict || ((vict == GET_OPPONENT(mob)) && chance(50))) return FALSE;

      size_t teleport_list_size = TELEPORT_END - TELEPORT_START - NUMELEMS(teleport_blacklist);

      /* Sanity check. */
      if (teleport_list_size < 1) return FALSE;

      int teleport_list[teleport_list_size];

      for (int i = 0, j = 0; (i < (TELEPORT_END - TELEPORT_START)) && (j < teleport_list_size); i++) {
        if (in_int_array(TELEPORT_START + i, teleport_blacklist, NUMELEMS(teleport_blacklist))) continue;

        teleport_list[j] = TELEPORT_START + i;
        j++;
      }

      shuffle_int_array(teleport_list, NUMELEMS(teleport_list));

      int teleport_room = real_room(teleport_list[0]);

      /* Somehow we failed to find a suitable teleport room; abort. */
      if (!teleport_room) return FALSE;

      do_say(mob, "Be gone with you!", CMD_SAY);

      act("$n points a gnarled finger at $N and $E disappears!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n points a gnarled finger at you and you are teleported elsewhere!", FALSE, mob, 0, vict, TO_VICT);

      char_from_room(vict);
      char_to_room(vict, teleport_room);

      act("$n appears in the room suddenly!", TRUE, vict, 0, 0, TO_ROOM);

      do_look(vict, "", CMD_LOOK);

      return FALSE;
    }

    return FALSE;
  }

  return FALSE;
}


/* Commando-like specs. */
void snipe_action(CHAR *ch, CHAR *victim); /* From fight.c */
int incendiary_cloud_enchant(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg); /* From subclass.spells.c */
int stram_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    int rnd = number(1, 100);

    /* 15% Incendiary Cloud */
    if (rnd <= 15) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n makes an arcane gesture and $N is engulfed in a cloud of flames!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n makes an arcane gesture and you are engulfed in a cloud of flames!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, 500, SPELL_INCENDIARY_CLOUD_NEW, DAM_FIRE);

      enchantment_apply(vict, TRUE, "Incendiary Cloud", SPELL_INCENDIARY_CLOUD_NEW, 20, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, incendiary_cloud_enchant);
    }
    /* 15% Electric Shock */
    else if (rnd <= 30) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n makes an arcane gesture and sends bolts of electricity at $N!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n makes an arcane gesture and sends bolts of electricity at you!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, number(450, 500), TYPE_UNDEFINED, DAM_ELECTRIC);
    }
    /* 25% Vampiric Touch */
    else if (rnd <= 55) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n reaches out and touches $N, draining some of $S life away!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n reaches out and touches you, draining some of your life away!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, GET_LEVEL(mob) * 8, TYPE_UNDEFINED, DAM_MAGICAL);

      GET_HIT(mob) += (GET_LEVEL(mob) * 8);
    }
    /* 25% Iceball */
    else if (rnd <= 80) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n makes an arcane gesture and sends a ball of ice at $N!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n makes an arcane gesture and sends a ball of ice at you!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, (GET_LEVEL(mob) / 2) + number(100, 160), TYPE_UNDEFINED, DAM_COLD);
    }
    /* 20% Death Spray */
    else {
      act("$n makes an arcane gesture and many dark rays shoot out from $s hand.", TRUE, mob, 0, 0, TO_ROOM);

      for (CHAR *vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)), *temp_vict; vict; vict = temp_vict) {
        temp_vict = vict->next_in_room;

        if (!IS_MORTAL(vict)) continue;

        damage(ch, vict, 120 + (GET_LEVEL(mob) / 2), TYPE_UNDEFINED, DAM_MAGICAL);
      }
    }

    /* Try to snipe one weakened player per MOBACT. */
    for (CHAR *vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)), *temp_vict; vict; vict = temp_vict) {
      temp_vict = vict->next_in_room;

      if (!IS_MORTAL(vict)) continue;

      snipe_action(mob, vict);

      /* If he sniped someone, break out of the loop. */
      if (enchanted_by(vict, STRAM_SNIPE_ENCH_NAME)) break;
    }

    return FALSE;
  }

  return FALSE;
}


/* Tohiri is healed to full each mobact unless affected by Flute spec. */
int tohiri_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Note: Mob state is tracked using its Bank variable. */

  if (cmd == MSG_MOBACT) {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);

    /* Set HP to full if not affected by Flute spec. */
    if (!IS_SET(GET_BANK(mob), WEAKENED) && (GET_HIT(mob) < GET_MAX_HIT(mob))) {
      act("$n prays to $s gods for a miracle and it is granted!", FALSE, mob, 0, 0, TO_ROOM);

      GET_HIT(mob) = GET_MAX_HIT(mob);
    }
    /* Otherwise, remove the Flute spec state. */
    else if (IS_SET(GET_BANK(mob), WEAKENED)) {
      act("$n's look of tranquility fades as $e comes to $s senses.", FALSE, mob, 0, 0, TO_ROOM);

      REMOVE_BIT(GET_BANK(mob), WEAKENED);
    }

    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    int rnd = number(1, 100);

    /* 20% Fortification */
    if (rnd <= 20) {
      if (!affected_by_spell(mob, SPELL_FORTIFICATION)) {
        spell_fortification(GET_LEVEL(mob), mob, mob, 0);

        return FALSE;
      }
    }

    /* 40% Haste */
    if (rnd <= 60) {
      if (!affected_by_spell(mob, SPELL_HASTE)) {
        spell_haste(GET_LEVEL(mob), mob, mob, 0);

        return FALSE;
      }
    }

    return FALSE;
  }

  return FALSE;
}


int aniston_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Block characters trying to exit north in Searing Room. */
  if (cmd == CMD_NORTH) {
    if (!ch || IS_IMMORTAL(ch) || (V_ROOM(mob) != SEARING_ROOM)) return FALSE;

    act("$n pushes $N back!", FALSE, mob, 0, ch, TO_NOTVICT);
    act("$n pushes you back!", FALSE, mob, 0, ch, TO_VICT);

    do_say(mob, "I will not let you interfere!  Mere mortals are not strong enough to face what lies beyond!", CMD_SAY);

    return TRUE;
  }

  /* Attack first character to enter Searing Room if Locket has been given, unless already engaged in combat. */
  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch) || !IS_SET(GET_BANK(mob), WEAKENED) || GET_OPPONENT(mob)) return FALSE;

    do_say(mob, "My brother is dead, and soon you shall join him!", CMD_SAY);

    hit(mob, ch, TYPE_HIT);

    return FALSE;
  }

  if (cmd == MSG_VIOLENCE) {
    if (!ch || !GET_OPPONENT(ch) || (V_ROOM(mob) != SEARING_ROOM)) return FALSE;

    /* Recall all mortals in the room if not affected by Locket spec. */
    if (!IS_SET(GET_BANK(mob), WEAKENED)) {
      do_say(mob, "I will not fight you!  Go away!", CMD_SAY);

      act("$n recites a scroll of total recall!", TRUE, mob, 0, 0, TO_ROOM);

      for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        /* Only teleport mortals. */
        if (!IS_MORTAL(vict)) continue;

        act("$n disappears!", FALSE, vict, 0, 0, TO_ROOM);

        char_from_room(vict);
        char_to_room(vict, real_room(TEMPLE));

        act("$n appears in the room suddenly!", TRUE, vict, 0, 0, TO_ROOM);

        do_look(vict, "", CMD_LOOK);
      }

      return FALSE;
    }
  }

  if (cmd == MSG_MOBACT) {
    /* Full heal if not in combat. */
    if (!GET_OPPONENT(mob) && (GET_HIT(mob) < GET_MAX_HIT(mob))) {
      act("$n quaffs a crimson colored potion and looks completely rejuvenated.", TRUE, mob, 0, 0, TO_ROOM);

      GET_HIT(mob) = GET_MAX_HIT(mob);
    }

    /* Set ACT_SHIELD above 1/3 HP. */
    if (!IS_SET(GET_ACT(mob), ACT_SHIELD) && (GET_HIT(mob) > (GET_MAX_HIT(mob) / 3))) {
      act("$n regains $s composure and sets $mself in a more defensive stance.", FALSE, mob, 0, 0, TO_ROOM);

      SET_BIT(GET_ACT(mob), ACT_SHIELD);
    }
    /* Remove ACT_SHIELD at or below 1/3 HP. */
    else if (IS_SET(GET_ACT(mob), ACT_SHIELD) && (GET_HIT(mob) <= (GET_MAX_HIT(mob) / 3))) {
      act("$n's composure falters and $s defenses seem to weaken.", FALSE, mob, 0, 0, TO_ROOM);

      REMOVE_BIT(GET_ACT(mob), ACT_SHIELD);
    }

    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    if (V_ROOM(mob) != ANISTON_ROOM) return FALSE;

    /* 25% Force a character into an adjacent hazard. */
    if (chance(25)) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      if (vict == GET_OPPONENT(mob)) {
        act("$n charges $N and crushes $S armor with a devastating blow!", TRUE, mob, 0, vict, TO_NOTVICT);
        act("$n charges you and crushes your armor with a devastating blow!", FALSE, mob, 0, vict, TO_VICT);

        enchantment_apply(vict, TRUE, ANISTON_CRUSH_ENCH_NAME, TYPE_UNDEFINED, 20, ENCH_INTERVAL_ROUND, 150, APPLY_AC, 0, 0, 0);

        damage(mob, vict, number(300, 600), TYPE_UNDEFINED, DAM_NO_BLOCK);

        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
      }
      else {
        act("$n charges $N and pushes $M off the narrow bridge!", TRUE, mob, 0, vict, TO_NOTVICT);
        act("$n charges you and pushes you off the narrow bridge!", FALSE, mob, 0, vict, TO_VICT);

        stop_fighting(vict);

        /* Make a mortal walk off the bridge if they're not paralyzed. */
        if (IS_MORTAL(vict) && !IS_AFFECTED(vict, AFF_PARALYSIS)) {
          GET_POS(vict) = POSITION_STANDING;

          char buf[MIL];

          snprintf(buf, sizeof(buf), "%s", chance(50) ? "east" : "west");

          command_interpreter(vict, buf);
        }
        /* Otherwise, they fall off the bridge and die instead. */
        else {
          char_from_room(vict);
          char_to_room(vict, real_room(LAVA_HAZARD));

          do_look(vict, "", CMD_LOOK);

          if (IS_AFFECTED(vict, AFF_PARALYSIS)) {
            printf_to_char(vict, "Your paralyzation prevents your escape from a fiery death!\n\r");
          }

          /* 3 times for good measure. */
          damage(vict, vict, 30000, TYPE_UNDEFINED, DAM_NO_BLOCK);
          damage(vict, vict, 30000, TYPE_UNDEFINED, DAM_NO_BLOCK);
          damage(vict, vict, 30000, TYPE_UNDEFINED, DAM_NO_BLOCK);
        }
      }

      return FALSE;
    }

    return FALSE;
  }

  /* Don't fight unless affected by Locket spec. */
  if ((cmd == CMD_KILL) || (cmd == CMD_HIT)) {
    if (!ch || IS_IMMORTAL(ch) || IS_SET(GET_BANK(mob), WEAKENED) || (V_ROOM(mob) != SEARING_ROOM)) return FALSE;

    char buf[MIL];

    one_argument(arg, buf);

    if (!*buf || !isname(buf, MOB_NAME(mob))) return FALSE;

    do_say(mob, "My duty is clear.  I cannot be distracted!", CMD_SAY);

    return TRUE;
  }

  /* Set to aggressive if given Locket and make him attack the giver. */
  if (cmd == CMD_GIVE) {
    if (!ch) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!*buf) return FALSE;

    OBJ *obj = get_obj_in_list_vis(ch, buf, ch->carrying);

    if (!obj || (V_OBJ(obj) != LOCKET)) return FALSE;

    one_argument(arg, buf);

    if (!*buf || !isname(buf, MOB_NAME(mob))) return FALSE;

    /* Refuse the object if not given by a mortal. */
    if (!IS_MORTAL(ch)) {
      act("$n glares at $N and refuses to take what is being offered.", FALSE, mob, 0, ch, TO_NOTVICT);
      act("$n glares at you and refuses to take what is being offered.", FALSE, mob, 0, ch, TO_VICT);

      return TRUE;
    }

    do_say(mob, "You killed my brother...  Prepare to DIE!", CMD_SAY);

    act("$n drops the locket, which tumbles from the bridge and into the magma below.", FALSE, mob, 0, 0, TO_ROOM);

    extract_obj(obj);

    SET_BIT(GET_ACT(mob), ACT_AGGRESSIVE);

    /* Bank is used to store mob state. */
    SET_BIT(GET_BANK(mob), WEAKENED);

    hit(mob, ch, TYPE_HIT);

    return TRUE;
  }

  /* Override RANDOM flag so that APPLY_DAMROLL can only be +1 higher than base, instead of +2. */
  if (cmd == MSG_DIE) {
    if (!mob) return FALSE;

    OBJ *tactical = EQ(mob, WEAR_LEGS);

    const int tactical_max_damroll = 3;

    if (!tactical || (V_OBJ(tactical) != TACTICAL) || OBJ_AFF_LOC(tactical, 1) != APPLY_DAMROLL) return FALSE;

    OBJ_AFF_MOD(tactical, 1) = MIN(OBJ_AFF_MOD(tactical, 1), tactical_max_damroll);

    return FALSE;
  }

  return FALSE;
}


/* Paladin-like specs. */
int law_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);

    return FALSE;
  }

  return FALSE;
}


/* Enchantment that prevents casting. */
int chaos_bite_ench(ENCH *ench, CHAR *ch, CHAR *char_in_room, int cmd, char *arg) {
  if (cmd == CMD_CAST) {
    if (!ch || !IS_MORTAL(ch) || (GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) return FALSE;

    act("$n tries to cast a spell, but $e can only choke up blood from the wound in $s neck.", FALSE, ch, 0, 0, TO_ROOM);
    act("You try to cast a spell, but you can only choke up blood from the wound in your neck.", FALSE, ch, 0, 0, TO_CHAR);

    return TRUE;
  }

  if (cmd == MSG_REMOVE_ENCH) {
    if (!ch || IS_MORTAL(ch)) return FALSE;

    act("You manage to stem the flow of blood from the wound on your neck.", FALSE, ch, 0, 0, TO_CHAR);

    return FALSE;
  }

  return FALSE;
}


/* Anti-Paladin-like specs. */
int chaos_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Attack the first mortal that enters Pentagram Room, unless already engaged in combat. */
  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch) || GET_OPPONENT(mob) || (V_ROOM(mob) != CHAOS_ROOM)) return FALSE;

    act("$n plunges $s weapon deep into $N's back!", FALSE, mob, 0, ch, TO_NOTVICT);
    act("As you enter the room, $n plunges $s weapon deep into your back!", FALSE, mob, 0, ch, TO_VICT);

    damage(mob, ch, number(2000, 4000), TYPE_UNDEFINED, DAM_NO_BLOCK);

    return FALSE;
  }

  if (cmd == MSG_MOBACT) {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);

    /* Cast Blackmantle if not already affected by it. */
    if (!affected_by_spell(mob, SPELL_BLACKMANTLE)) {
      spell_blackmantle(GET_LEVEL(mob), mob, mob, 0);
    }

    int rnd = number(1, 100);

    /* 20% Shadowstep a random target. */
    if (rnd <= 20) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n steps into the shadows and attacks $N by surprise!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n steps into the shadows and attacks you by surprise!", FALSE, mob, 0, vict, TO_VICT);

      hit(mob, vict, TYPE_HIT);
    }
    /* 20% Anti-cast enchant. */
    else if (rnd <= 40) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n lunges at $N and savagely tears at $S neck!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n lunges at you and savagely tears at your neck!", FALSE, mob, 0, vict, TO_VICT);

      enchantment_apply(vict, TRUE, CHAOS_BITE_ENCH_NAME, TYPE_UNDEFINED, 20, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, chaos_bite_ench);

      damage(mob, vict, number(200, 400), TYPE_UNDEFINED, DAM_NO_BLOCK);
    }
    /* 20% Non-dispellable blindness enchant. */
    else if (rnd <= 60) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n sends a black mass of shadowy tendrils that envelop $N!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n sends a black mass of shadowy tendrils that envelop you!", FALSE, mob, 0, vict, TO_VICT);

      enchantment_apply(vict, TRUE, CHAOS_TENDRILS_ENCH_NAME, TYPE_UNDEFINED, 10, ENCH_INTERVAL_ROUND, -8, APPLY_HITROLL, AFF_BLIND, 0, 0);

      damage(mob, vict, number(50, 100), TYPE_UNDEFINED, DAM_NO_BLOCK);
    }

    bool spirit_levy = FALSE;

    /* 'Sprit Levy' corpses in the room. */
    for (OBJ *temp_obj = world[CHAR_REAL_ROOM(mob)].contents, *next_content; temp_obj; temp_obj = next_content) {
      next_content = temp_obj->next_content;

      /* Make sure it's a corpse. */
      if ((OBJ_TYPE(temp_obj) == ITEM_CONTAINER) && OBJ_VALUE(temp_obj, 3) && (OBJ_COST(temp_obj) != PC_STATUE) && (OBJ_COST(temp_obj) != NPC_STATUE)) {
        if (!spirit_levy) {
          act("$n performs a dark ritual.", FALSE, mob, 0, 0, TO_ROOM);

          spirit_levy = TRUE;
        }

        act("$n absorbs life energy from the dead.", FALSE, mob, temp_obj, 0, TO_ROOM);

        /* Get all the stuff from the corpse and put it on the ground. */
        for (OBJ *temp_obj2 = temp_obj->contains, *next_content2; temp_obj2; temp_obj2 = next_content2) {
          next_content2 = temp_obj2->next_content;

          obj_from_obj(temp_obj2);
          obj_to_room(temp_obj2, CHAR_REAL_ROOM(mob));
        }

        extract_obj(temp_obj);

        GET_HIT(mob) += number(9999, 13333);
      }
    }

    return FALSE;
  }

  /* Apply random affect to Bladed Shield when Chaos dies and is wearing it (e.g. he was loading it). */
  if (cmd == MSG_DIE) {
    if (!mob) return FALSE;

    OBJ *shield = EQ(mob, WEAR_SHIELD);

    if (!shield || (V_OBJ(shield) != BLADED_SHIELD) || OBJ_AFF_LOC(shield, 2) || OBJ_AFF_MOD(shield, 2)) return FALSE;

    const int shield_skill_list[] = {
      APPLY_SKILL_ASSAULT,
      APPLY_SKILL_BACKSTAB,
      APPLY_SKILL_BACKFLIP,
      APPLY_SKILL_CIRCLE,
      APPLY_SKILL_DODGE,
      APPLY_SKILL_DUAL,
      APPLY_SKILL_KICK,
      APPLY_SKILL_PARRY,
      APPLY_SKILL_PUMMEL,
      APPLY_SKILL_PUNCH,
      APPLY_SKILL_TRIPLE
    };

    const int shield_regen_list[] = {
      APPLY_HP_REGEN,
      APPLY_MANA_REGEN
    };

    const int shield_skill_apply_min = 4, shield_skill_apply_max = 8;
    const int shield_regen_apply_min = 4, shield_regen_apply_max = 8;

    bool use_skill_list = chance(50) ? TRUE : FALSE;

    int shield_location = 0, shield_modifier = 0;

    if (use_skill_list) {
      shield_location = shield_skill_list[number(1, NUMELEMS(shield_skill_list)) - 1];
      shield_modifier = number(shield_skill_apply_min, shield_skill_apply_max);
    }
    else {
      shield_location = shield_regen_list[number(1, NUMELEMS(shield_regen_list)) - 1];
      shield_modifier = number(shield_regen_apply_min, shield_regen_apply_max);

      switch (shield_location) {
        case APPLY_HP_REGEN:
          shield_modifier *= 5;
          break;
      }
    }

    OBJ_AFF_LOC(shield, 2) = shield_location;
    OBJ_AFF_MOD(shield, 2) = shield_modifier;

    return FALSE;
  }

  return FALSE;
}


/* Throw people out of the room, gate in an ally, and taunt people. */
int xykloqtium_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Note: Mob state is tracked using its Bank variable. */

  /* He won't take the Orb or the Cracked Orb. */
  if (cmd == CMD_GIVE) {
    if (!ch || !AWAKE(ch)) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!*buf) return FALSE;

    OBJ *obj = get_obj_in_list_vis(ch, buf, ch->carrying);

    if ((V_OBJ(obj) != ORB) && (V_OBJ(obj) != CRACKED_ORB)) return FALSE;

    one_argument(arg, buf);

    if (!*buf || !isname(buf, MOB_NAME(mob))) return FALSE;

    if (!IS_MORTAL(ch)) {
      act("$n glares at $N and refuses to take what is being offered.", FALSE, mob, 0, ch, TO_NOTVICT);
      act("$n glares at you and refuses to take what is being offered.", FALSE, mob, 0, ch, TO_VICT);

      return TRUE;
    }

    do_say(mob, "Arrgh!  I will not take that foul thing!", CMD_SAY);

    act("$n drops $p upon the ground.", FALSE, mob, obj, 0, TO_ROOM);

    obj_to_room(obj_from_char(obj), CHAR_REAL_ROOM(ch));

    hit(mob, ch, TYPE_HIT);

    return TRUE;
  }

  if (cmd == MSG_MOBACT) {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);

    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    /* Slay random character if not affected by Orb spec. */
    if (!IS_SET(GET_BANK(mob), WEAKENED) && chance(20)) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n points $s finger at $N and utters a purely evil word.", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n points $s finger at you and utters a purely evil word.", FALSE, mob, 0, vict, TO_VICT);

      /* 3 times for good measure. */
      damage(mob, vict, 30000, TYPE_UNDEFINED, DAM_NO_BLOCK);
      damage(mob, vict, 30000, TYPE_UNDEFINED, DAM_NO_BLOCK);
      damage(mob, vict, 30000, TYPE_UNDEFINED, DAM_NO_BLOCK);
    }

    int rnd = number(1, 100);

    /* 15% Throw an attacker into a random Abyss room and load a Demon Spawn to attack them. */
    if (rnd <= 15) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      /* Throw the tank to the ground instead, causing damage. */
      if (vict == GET_OPPONENT(mob)) {
        act("$n grabs $N and throws $M to the ground!", FALSE, mob, 0, vict, TO_NOTVICT);
        act("$n grabs you and throws you to the ground!", FALSE, mob, 0, vict, TO_VICT);

        damage(mob, vict, number(900, 1200), TYPE_UNDEFINED, DAM_NO_BLOCK);

        GET_POS(vict) = POSITION_SITTING;

        WAIT_STATE(vict, PULSE_VIOLENCE);
      }
      else {
        size_t maze_list_size = MAZE_MAX - MAZE_MIN;

        /* Don't count the current room if the mob is in a maze room. */
        if ((CHAR_VIRTUAL_ROOM(mob) >= MAZE_MIN) || (CHAR_VIRTUAL_ROOM(mob) <= MAZE_MAX)) {
          maze_list_size -= 1;
        }

        /* Sanity check. */
        if (maze_list_size < 1) return FALSE;

        int maze_list[maze_list_size];

        for (int i = 0, j = 0; (i < (MAZE_MAX - MAZE_MIN)) && (j < maze_list_size); i++) {
          if (CHAR_VIRTUAL_ROOM(mob) == MAZE_MIN + i) continue;

          maze_list[i] = MAZE_MIN + i;
          j++;
        }

        shuffle_int_array(maze_list, NUMELEMS(maze_list));

        int maze_room = real_room(maze_list[0]);

        /* Somehow we failed to find a suitable maze room; abort. */
        if (!maze_room) return FALSE;

        act("$n reaches out, grabs $N, and throws $M into the swirling mists!", FALSE, mob, 0, vict, TO_NOTVICT);
        act("$n reaches out, grabs you, and throws you into the swirling mists!", FALSE, mob, 0, vict, TO_VICT);

        damage(mob, vict, number(80, 160), TYPE_UNDEFINED, DAM_NO_BLOCK);

        char_from_room(vict);
        char_to_room(vict, maze_room);

        GET_POS(vict) = POSITION_SITTING;

        WAIT_STATE(vict, PULSE_VIOLENCE);

        CHAR *spawn = read_mobile(DEMON_SPAWN, VIRTUAL);

        /* Sanity check. */
        if (!spawn) return FALSE;

        char_to_room(spawn, CHAR_REAL_ROOM(vict));

        do_look(vict, "", CMD_LOOK);

        hit(spawn, vict, TYPE_HIT);
      }
    }
    /* 25% Load a random demon ally to fight a random mortal. */
    else if (rnd <= 40) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      /* Sanity check. */
      if ((GATE_MAX - GATE_MIN) <= 0) return FALSE;

      int ally_vnum = number(GATE_MIN, GATE_MAX);

      /* Replace Bebilith with Demon Spawn. */
      ally_vnum = ((ally_vnum == BEBILITH) ? DEMON_SPAWN : ally_vnum);

      CHAR *ally = read_mobile(ally_vnum, VIRTUAL);

      /* Sanity check. */
      if (!ally) return FALSE;

      act("$n utters a word of command and gates in a demonic ally!", FALSE, mob, 0, 0, TO_ROOM);

      act("$n steps through the gate.", FALSE, mob, 0, vict, TO_ROOM);

      char_to_room(ally, CHAR_REAL_ROOM(mob));

      hit(ally, vict, TYPE_HIT);
    }

    /* Taunt attackers. */
    rnd = number(1, 100);

    if (rnd <= 15) {
      do_say(mob, "Your bones shall fill my belly after I break your soul!", CMD_SAY);
    }
    else if (rnd <= 30) {
      do_say(mob, "Hahaha... You fools!  You cannot defeat me!", CMD_SAY);
    }
    else if (rnd <= 45) {
      do_say(mob, "Is that the best you can do?  Pitiful mortals!", CMD_SAY);
    }
    else if (rnd <= 60) {
      do_say(mob, "I will squeeze the life from your world with my bare hands!", CMD_SAY);
    }
    else if (rnd <= 75) {
      do_say(mob, "You shall all die a most painfully slow death!", CMD_SAY);
    }
    else if (rnd <= 90) {
      do_say(mob, "Bow down and serve the true masters of the multiverse!", CMD_SAY);
    }
    else {
      act("$n snorts smoke and flames from $s nostrils!", FALSE, mob, 0, 0, TO_ROOM);
    }

    return FALSE;
  }

  return FALSE;
}


/* Object Specs */


int horn_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  /* Apply an enchantment that gives an appropriate skill bonus when the horn is being equipped. */
  if ((cmd == MSG_OBJ_WORN) || (cmd == MSG_OBJ_ENTERING_GAME)) {
    if (!ch || !IS_MORTAL(ch)) return FALSE;
    if ((cmd == MSG_OBJ_ENTERING_GAME) && ((ch != OBJ_EQUIPPED_BY(obj)) || (ch != OBJ_EQUIPPED_BY(obj)))) return FALSE;
    if ((cmd == MSG_OBJ_WORN) && ((ch != OBJ_CARRIED_BY(obj)) || (ch != OBJ_CARRIED_BY(obj)))) return FALSE;

    int horn_location = 0, horn_modifier = 0;

    switch (GET_CLASS(ch)) {
      case CLASS_CLERIC:
        horn_location = APPLY_SKILL_BASH;
        horn_modifier = 10;
        break;
      case CLASS_BARD:
        horn_location = APPLY_SKILL_BACKFLIP;
        horn_modifier = 10;
        break;
    }

    if (!enchanted_by(ch, HORN_ENCH_NAME)) {
      enchantment_apply(ch, TRUE, HORN_ENCH_NAME, TYPE_UNDEFINED, -1, ENCH_INTERVAL_USER, horn_modifier, horn_location, 0, 0, 0);
    }

    return FALSE;
  }

  /* Remove the enchantment if the horn is being removed. */
  if (cmd == MSG_OBJ_REMOVED) {
    if (!ch || !OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj))) return FALSE;

    ENCH *horn_enchant = get_enchantment_by_name(ch, HORN_ENCH_NAME);

    if (horn_enchant) {
      enchantment_remove(ch, horn_enchant, FALSE);
    }

    return FALSE;
  }

  /* Chance to spec Miracle, Heal, Cure Critic, or Bless to all mortals in the room during combat. */
  if (cmd == MSG_TICK) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner || !IS_MORTAL(owner) || !GET_OPPONENT(owner)) return FALSE;

    /* 90% No spec. */
    if (chance(90)) return FALSE;

    int rnd = number(1, 10);

    /* 1% Miracle */
    if (rnd <= 1) {
      if (GET_MANA(owner) < 100) return FALSE;

      GET_MANA(owner) -= 100;

      spell_miracle(GET_LEVEL(owner), owner, owner, 0);

      if (ROOM_CHAOTIC(CHAR_REAL_ROOM(owner))) return FALSE;

      for (CHAR *vict = world[CHAR_REAL_ROOM(owner)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        if (!IS_MORTAL(vict) || (vict == owner)) continue;

        spell_miracle(GET_LEVEL(owner), owner, vict, 0);
      }
    }
    /* 2% Heal */
    else if (rnd <= 3) {
      if (GET_MANA(owner) < 50) return FALSE;

      GET_MANA(owner) -= 50;

      spell_heal(GET_LEVEL(owner), owner, owner, 0);

      if (ROOM_CHAOTIC(CHAR_REAL_ROOM(owner))) return FALSE;

      for (CHAR *vict = world[CHAR_REAL_ROOM(owner)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        if (!IS_MORTAL(vict) || (vict == owner)) continue;

        spell_heal(GET_LEVEL(owner), owner, vict, 0);
      }
    }
    /* 3% Cure Critic */
    else if (rnd <= 6) {
      if (GET_MANA(owner) < 25) return FALSE;

      GET_MANA(owner) -= 25;

      spell_cure_critic(GET_LEVEL(owner), owner, owner, 0);

      if (ROOM_CHAOTIC(CHAR_REAL_ROOM(owner))) return FALSE;

      for (CHAR *vict = world[CHAR_REAL_ROOM(owner)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        if (!IS_MORTAL(vict) || (vict == owner)) continue;

        spell_cure_critic(GET_LEVEL(owner), owner, vict, 0);
      }
    }
    /* 4% Bless */
    else if (rnd <= 10) {
      if (GET_MANA(owner) < 5) return FALSE;

      GET_MANA(owner) -= 5;

      spell_bless(GET_LEVEL(owner), owner, owner, 0);

      if (ROOM_CHAOTIC(CHAR_REAL_ROOM(owner))) return FALSE;

      for (CHAR *vict = world[CHAR_REAL_ROOM(owner)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        if (!IS_MORTAL(vict) || (vict == owner)) continue;

        spell_bless(GET_LEVEL(owner), owner, vict, 0);
      }
    }

    printf_to_room(CHAR_REAL_ROOM(owner), "The tranquil music of the sea fills the air!\n\r");

    return FALSE;
  }

  return FALSE;
}


int mantle_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  /* Set APPLY_DAMAGE to 2 when removed or when the character dies. */
  if ((cmd == MSG_BEING_REMOVED) || (cmd == MSG_DIE)) {
    if (!ch || !OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj))) return FALSE;

    /* Sanity check to make sure AFF1 is APPLY_DAMROLL. */
    if (OBJ_AFF_LOC(obj, 1) != APPLY_DAMROLL) return FALSE;

    /* Sanity check to make sure its worn in the correct slot. */
    if (obj != EQ(ch, WEAR_ABOUT)) return FALSE;

    unequip_char(ch, WEAR_ABOUT);

    OBJ_AFF_MOD(obj, 1) = 2;

    equip_char(ch, obj, WEAR_ABOUT);

    return FALSE;
  }

  /* Set APPLY_DAMAGE between 1 and 4 at tick. */
  if (cmd == MSG_TICK) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner || !IS_MORTAL(owner)) return FALSE;

    /* Sanity check to make sure AFF1 is APPLY_DAMROLL. */
    if (OBJ_AFF_LOC(obj, 1) != APPLY_DAMROLL) return FALSE;

    /* Sanity check to make sure it's worn in the correct slot. */
    if (obj != EQ(owner, WEAR_ABOUT)) return FALSE;

    unequip_char(owner, WEAR_ABOUT);

    int rnd = number(1, 100);

    /* 10% 1 damage. */
    if (rnd <= 20) {
      OBJ_AFF_MOD(obj, 1) = 1;
    }
    /* 30% 2 damage. */
    else if (rnd <= 50) {
      OBJ_AFF_MOD(obj, 1) = 2;
    }
    /* 40% 3 damage. */
    else if (rnd <= 80) {
      OBJ_AFF_MOD(obj, 1) = 3;
    }
    /* 20% 4 damage. */
    else {
      OBJ_AFF_MOD(obj, 1) = 4;
    }

    equip_char(owner, obj, WEAR_ABOUT);

    return FALSE;
  }

  return FALSE;
}


int circlet_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  /* Set APPLY_MANA_REGEN to 5 when removed or when the character dies. */
  if ((cmd == MSG_BEING_REMOVED) || (cmd == MSG_DIE)) {
    if (!ch || !OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj))) return FALSE;

    /* Reset the timer for the pray command. */
    OBJ_SPEC(obj) = 0;

    /* Sanity check to make sure AFF0 is APPLY_MANA_REGEN. */
    if (OBJ_AFF_LOC(obj, 0) != APPLY_MANA_REGEN) return FALSE;

    /* Sanity check to make sure its worn in the correct slot. */
    if (obj != EQ(ch, WEAR_HEAD)) return FALSE;

    unequip_char(ch, WEAR_HEAD);

    OBJ_AFF_MOD(obj, 0) = 5;

    equip_char(ch, obj, WEAR_HEAD);

    return FALSE;
  }

  /* Set APPLY_MANA_REGEN based on the wearer's alignment at tick. */
  if (cmd == MSG_TICK) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner || !IS_MORTAL(owner)) return FALSE;

    if (OBJ_SPEC(obj) < 5) {
      OBJ_SPEC(obj)++;

      if (OBJ_SPEC(obj) >= 5) {
        printf_to_char(owner, "You feel as if the gods will reward your devotion if you pray.\n\r");
      }
    }

    /* Sanity check to make sure AFF0 is APPLY_MANA_REGEN. */
    if (OBJ_AFF_LOC(obj, 0) != APPLY_MANA_REGEN) return FALSE;

    /* Sanity check to make sure its worn in the correct slot. */
    if (obj != EQ(owner, WEAR_HEAD)) return FALSE;

    unequip_char(owner, WEAR_HEAD);

    if (GET_ALIGNMENT(owner) < 350) {
      OBJ_AFF_MOD(obj, 0) = 5;
    }
    else if (GET_ALIGNMENT(owner) < 500) {
      OBJ_AFF_MOD(obj, 0) = 6;
    }
    else if (GET_ALIGNMENT(owner) < 600) {
      OBJ_AFF_MOD(obj, 0) = 7;
    }
    else if (GET_ALIGNMENT(owner) < 750) {
      OBJ_AFF_MOD(obj, 0) = 8;
    }
    else if (GET_ALIGNMENT(owner) < 900) {
      OBJ_AFF_MOD(obj, 0) = 9;
    }
    else {
      OBJ_AFF_MOD(obj, 0) = 10;
    }

    equip_char(owner, obj, WEAR_HEAD);

    return FALSE;
  }

  /* The wearer can improve their alignment by 200 after wearing the circlet for 10 ticks. */
  if (cmd == CMD_PRAY) {
    if (!ch || !OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj)) || !AWAKE(ch) || !IS_MORTAL(ch)) return FALSE;

    if (OBJ_SPEC(obj) >= 10) {
      if (GET_ALIGNMENT(ch) >= 1000) {
        /* Defer to Paladin pray command for members of that class. */
        if (GET_CLASS(ch) == CLASS_PALADIN) return FALSE;

        printf_to_char(ch, "You are already saintly.\n\r");
      }
      else {
        OBJ_SPEC(obj) = 0;

        printf_to_char(ch, "You bow your head in prayer and your devotion is rewarded by the gods!\n\r");

        GET_ALIGNMENT(ch) = MIN(1000, GET_ALIGNMENT(ch) + 200);

        /* Allow paladins to continue to pray as normal */
        if ((GET_ALIGNMENT(ch) < 1000) && (GET_CLASS(ch) == CLASS_PALADIN)) return FALSE;

        WAIT_STATE(ch, PULSE_VIOLENCE);
      }

      return TRUE;
    }
    else {
      /* Defer to Paladin pray command for members of that class. */
      if (GET_CLASS(ch) == CLASS_PALADIN) return FALSE;

      printf_to_char(ch, "You must demonstrate your devotion to the gods for a while longer.\n\r");

      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}


int tactical_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  /* Powerful Spin-kick spec. */
  if (cmd == CMD_SPIN) {
    if (!ch || !OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj)) || !AWAKE(ch) || !IS_MORTAL(ch)) return FALSE;

    if (chance(25)) {
      if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && !CHAOSMODE) {
        printf_to_char(ch, "Behave yourself here please!\n\r");

        return TRUE;
      }

      if (GET_MOUNT(ch)) {
        printf_to_char(ch, "Dismount first.\n\r");

        return TRUE;
      }

      act("$n whirls about the room kicking, everyone in sight!", TRUE, ch, 0, 0, TO_ROOM);
      printf_to_char(ch, "You whirl about the room kicking, everyone in sight!\n\r");

      for (CHAR *vict = world[CHAR_REAL_ROOM(ch)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        if ((vict == ch) || (GET_RIDER(vict) == ch) || (IS_MORTAL(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(vict))) || IS_IMMORTAL(vict)) continue;

        act("You have been kicked by $n.", FALSE, ch, 0, vict, TO_VICT);

        damage(ch, vict, calc_position_damage(GET_POS(vict), (GET_LEVEL(ch) * 4)), TYPE_UNDEFINED, DAM_PHYSICAL);
      }

      WAIT_STATE(ch, PULSE_VIOLENCE * 2);

      return TRUE;
    }
  }

  return FALSE;
}


int frostbrand_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  /* Chill Touch-like spec on kill. */
  if (cmd == CMD_KILL) {
    if (!ch || !OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj)) || !AWAKE(ch) || !IS_MORTAL(ch) || GET_OPPONENT(ch)) return FALSE;

    char buf[MIL];

    one_argument(arg, buf);

    if (!*buf) return FALSE;

    if (chance(9)) {
      CHAR *vict = get_char_room_vis(ch, buf);

      if (!vict || IS_IMMORTAL(vict) || (IS_MORTAL(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(vict))) || IS_IMMUNE2(vict, IMMUNE2_COLD)) return FALSE;

      if (OBJ_ACTION(obj)) {
        snprintf(buf, sizeof(buf), "%s", OBJ_ACTION(obj));

        act(buf, TRUE, ch, 0, vict, TO_NOTVICT);
        act(buf, TRUE, ch, 0, vict, TO_CHAR);
      }

      act("$n's $f drains the warmth from $N as it strikes $M!", TRUE, ch, OBJ_NAME(obj), vict, TO_NOTVICT);
      act("$n's $f drains the warmth from you as it strikes you!", TRUE, ch, OBJ_NAME(obj), vict, TO_VICT);
      act("Your $f drains the warmth from $N as it strikes $M!", TRUE, ch, OBJ_NAME(obj), vict, TO_CHAR);

      damage(ch, vict, 100, TYPE_UNDEFINED, DAM_COLD);

      if (!affected_by_spell(vict, SPELL_CHILL_TOUCH)) {
        affect_apply(vict, SPELL_CHILL_TOUCH, 1, -2, APPLY_STR, 0, 0);
      }
    }

    return FALSE;
  }

  /* Chill Touch-like spec during combat. */
  if (cmd == MSG_MOBACT) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner || !IS_MORTAL(owner)) return FALSE;

    if (chance(9)) {
      CHAR *vict = GET_OPPONENT(owner);

      if (!vict || IS_IMMORTAL(vict) || (IS_MORTAL(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(vict)))) return FALSE;

      act("$n's $f drains the warmth from $N as it strikes $M!", TRUE, owner, OBJ_NAME(obj), vict, TO_NOTVICT);
      act("$n's $f drains the warmth from you as it strikes you!", TRUE, owner, OBJ_NAME(obj), vict, TO_VICT);
      act("Your $f drains the warmth from $N as it strikes $M!", TRUE, owner, OBJ_NAME(obj), vict, TO_CHAR);

      damage(owner, vict, 100, TYPE_UNDEFINED, DAM_COLD);

      if (!affected_by_spell(vict, SPELL_CHILL_TOUCH)) {
        affect_apply(vict, SPELL_CHILL_TOUCH, 1, -2, APPLY_STR, 0, 0);
      }
    }

    return FALSE;
  }

  return FALSE;
}


/* Damages wearer on wear, remove, and sometimes in combat. */
int bladed_shield_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if ((cmd == MSG_OBJ_WORN) || (cmd == MSG_OBJ_REMOVED)) {
    if (!ch || !AWAKE(ch) || IS_IMMORTAL(ch)) return FALSE;

    damage(ch, ch, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

    return FALSE;
  }

  if (cmd == MSG_MOBACT) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner || IS_NPC(owner)) return FALSE;

    /* Combat specs below. */
    if (!GET_OPPONENT(owner)) return FALSE;

    int val = number(1, 100);

    /* 1% Cut self. */
    if (val <= 1) {
      act("$n winces in pain as $s $F slices into $s wrist!", TRUE, owner, 0, OBJ_NAME(obj), TO_ROOM);
      act("You wince in pain as your $F slices into your wrist!", FALSE, owner, 0, OBJ_NAME(obj), TO_CHAR);

      damage(owner, owner, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
    }
    /* 5% Cut opponent. */
    else if (val <= 6) {
      CHAR *vict = GET_OPPONENT(owner);

      if (!vict || IS_IMMORTAL(vict) || (IS_MORTAL(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(vict)))) return FALSE;

      act("$n lashes out with $s $f and slices $N with it!", TRUE, owner, OBJ_NAME(obj), vict, TO_NOTVICT);
      act("$n lashes out with $s $f and slices you with it!", TRUE, owner, OBJ_NAME(obj), vict, TO_VICT);
      act("You lash out with your $f and slice $N with it!", TRUE, owner, OBJ_NAME(obj), vict, TO_CHAR);

      damage(owner, vict, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK);
    }

    return FALSE;
  }

  return FALSE;
}


int lantern_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  /* Set the player's HP to 1 when lantern is held and stun them. */
  if (cmd == MSG_OBJ_WORN) {
    if (!ch || !OBJ_CARRIED_BY(obj) || (ch != OBJ_CARRIED_BY(obj)) || !AWAKE(ch) || IS_IMMORTAL(ch)) return FALSE;

    GET_HIT(ch) = 1;

    WAIT_STATE(ch, PULSE_VIOLENCE);

    return FALSE;
  }

  if (cmd == MSG_TICK) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner) return FALSE;

    /* Inform the player that they might want to 'stoke' the lantern's embers. */
    if (GET_ALIGNMENT(owner) > -500) {
      act("Your $F gutters and smokes, and you suddenly desire to stoke its embers...", FALSE, owner, 0, OBJ_NAME(obj), TO_CHAR);
    }

    /* Combat specs below. */
    if (!GET_OPPONENT(owner)) return FALSE;

    /* Chance to burn non-evil characters with sparks during combat. */
    if (chance(5)) {
      act("A shower of burning sparks rains from $n's $F!", TRUE, owner, 0, OBJ_NAME(obj), TO_ROOM);
      act("A shower of burning sparks rains from your $F!", FALSE, owner, 0, OBJ_NAME(obj), TO_CHAR);

      for (CHAR *vict = world[CHAR_REAL_ROOM(owner)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        if (IS_EVIL(vict) || IS_IMMORTAL(vict)) continue;

        act("$N is burned the rain of sparks!", TRUE, owner, 0, vict, TO_NOTVICT);
        act("You are burned by the rain of sparks!", FALSE, owner, 0, vict, TO_VICT);
        act("$N is burned by the rain of sparks!", TRUE, owner, 0, vict, TO_CHAR);

        if (IS_NPC(vict)) {
          damage(owner, vict, 100, TYPE_UNDEFINED, DAM_FIRE);
        }
        else {
          GET_HIT(vict) = MAX(1, GET_HIT(vict) - 100);
        }
      }
    }

    return FALSE;
  }

  /* Stoke the lantern to decrease alignment by 200, at the cost of 10% of the user's hit points. */
  if (cmd == CMD_UNKNOWN) {
    if (!ch || !OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj)) || !AWAKE(ch) || !IS_MORTAL(ch)) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!*buf || (strcmp(buf, "stoke") != 0)) return FALSE;

    one_argument(arg, buf);

    if (!*buf || !isname(buf, OBJ_NAME(obj))) {
      printf_to_char(ch, "Stoke what?\n\r");

      return TRUE;
    }

    if (!OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj))) {
      act("You must equip the $F before you can stoke its embers.", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

      return TRUE;
    }

    act("$n stokes the embers in $s $F, causing them to glow white-hot for a brief moment.", FALSE, ch, 0, OBJ_NAME(obj), TO_ROOM);
    act("You stoke the embers in your $F, causing them to glow white-hot for a brief moment.", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

    if (GET_ALIGNMENT(ch) > -1000) {
      act("$n is scorched by the $F's malevolent heat!", FALSE, ch, 0, OBJ_NAME(obj), TO_ROOM);
      act("You are scorched by the $F's malevolent heat!", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

      GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - 200);

      GET_HIT(ch) = MAX(1, (int)(GET_HIT(ch) * 0.9));

      WAIT_STATE(ch, PULSE_VIOLENCE);
    }

    return TRUE;
  }

  return FALSE;
}


/* 'Picks' a normally un-pickable door. */
int lockpicks_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_PICK) {
    if (!ch || (ch != OBJ_CARRIED_BY(obj)) || (ch != OBJ_CARRIED_BY(obj)) || !AWAKE(ch) || IS_NPC(ch)) return FALSE;

    if (V_ROOM(ch) != STORAGE_ROOM) {
      act("The $F don't seem to be intended to pick anything here.", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

      return TRUE;
    }

    char door_name[MIL];

    arg = one_argument(arg, door_name);

    if (!*door_name) return FALSE;

    char dir_name[MIL];

    one_argument(arg, dir_name);

    if (!*dir_name) return FALSE;

    int door_num = find_door(ch, string_to_lower(door_name), string_to_lower(dir_name));

    if (door_num < 0) return TRUE;

    REMOVE_BIT(EXIT(ch, door_num)->exit_info, EX_LOCKED);

    act("$n manages to unlock the mechanism with $p.", FALSE, ch, obj, 0, TO_ROOM);
    act("You manage to unlock the mechanism with $p.", FALSE, ch, obj, 0, TO_CHAR);

    act("The magic within the $F fades and they crumble to dust.", FALSE, ch, 0, OBJ_NAME(obj), TO_ROOM);
    act("The magic within the $F fades and they crumble to dust.", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

    extract_obj(obj);

    return TRUE;
  }

  return FALSE;
}


/* Used to wound Velxok when he's below 1/2 HP, making him killable. */
int dart_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_THROW) {
    if (!ch || ((ch != OBJ_CARRIED_BY(obj)) && (ch != OBJ_EQUIPPED_BY(obj))) || !AWAKE(ch) || IS_NPC(ch)) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!isname(buf, OBJ_NAME(obj))) return FALSE;

    if (obj != EQ(ch, HOLD)) {
      act("You need to hold the $F in order to throw it.", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

      return TRUE;
    }

    one_argument(arg, buf);

    CHAR *vict = NULL;

    if (GET_OPPONENT(ch) && (!*buf || isname(buf, GET_NAME(GET_OPPONENT(ch))))) {
      vict = GET_OPPONENT(ch);
    }
    else {
      vict = get_char_room_vis(ch, buf);
    }

    if (!vict) {
      act("Throw the $F at who?", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

      return TRUE;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);

    if (!IS_NPC(vict) || (V_MOB(vict) != VELXOK)) {
      act("$n throws $p at $N, but it seems to have no effect.", FALSE, ch, obj, vict, TO_NOTVICT);
      act("$n throws $p at you, but it seems to have no effect.", FALSE, ch, obj, vict, TO_VICT);
      act("You throw $p at $N, but it seems to have no effect.", FALSE, ch, obj, vict, TO_CHAR);

      obj_to_char(unequip_char(ch, HOLD), vict);

      return TRUE;
    }

    extract_obj(unequip_char(ch, HOLD));

    /* Only wounds Velxok if he's at <= 1/2 max_hps. */
    if (GET_HIT(vict) > (GET_MAX_HIT(vict) / 2)) {
      act("$n throws $p at $N, but it is vaporized by $S mana shield!", FALSE, ch, obj, vict, TO_NOTVICT);
      act("You throw $p at $N, but it is vaporized by $S mana shield!", FALSE, ch, obj, vict, TO_CHAR);

      return TRUE;
    }
    else {
      act("$n throws $p at $N which strikes $M in the chest and dissolves!", FALSE, ch, obj, vict, TO_NOTVICT);
      act("You throw $p at $N which strikes $M in the chest and dissolves!", FALSE, ch, obj, vict, TO_CHAR);

      act("$n shudders in agony and clutches at $s chest!", FALSE, vict, 0, 0, TO_ROOM);

      REMOVE_BIT(GET_ACT(vict), ACT_SHIELD);

      /* Bank is used to store mob state. */
      SET_BIT(GET_BANK(vict), WEAKENED);
    }

    return TRUE;
  }

  return FALSE;
}


/* Barbarian's Rage breaks Ice Wall. */
int barbarians_rage_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_UNKNOWN) {
    if (!ch || ((ch != OBJ_CARRIED_BY(obj)) && (ch != OBJ_EQUIPPED_BY(obj))) || !AWAKE(ch) || IS_NPC(ch)) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!*buf || (strcmp(buf, "break") != 0)) return FALSE;

    if ((obj != EQ(ch, WIELD)) && chance(20)) {
      act("You feel the insatiable urge to wield $p and break something with it!", FALSE, ch, obj, 0, TO_CHAR);

      return FALSE;
    }

    one_argument(arg, buf);

    if (!*buf) {
      printf_to_char(ch, "Break what?\n\r");

      return TRUE;
    }

    OBJ *ice_wall = get_obj_by_vnum_in_room(ICE_WALL, CHAR_REAL_ROOM(ch));

    if (!ice_wall) {
      printf_to_char(ch, "There's nothing here to break that would satisfy your rage!\n\r");

      return TRUE;
    }

    if (!isname(buf, OBJ_NAME(ice_wall))) {
      printf_to_char(ch, "Breaking that wouldn't satisfy your rage!\n\r");

      return TRUE;
    }

    act("$n slams $p into the wall of ice!", FALSE, ch, obj, 0, TO_ROOM);
    act("You slam $p into the wall of ice!", FALSE, ch, obj, 0, TO_CHAR);

    act("Both $p and the wall of ice shatter to pieces!", FALSE, ch, obj, 0, TO_ROOM);
    act("Both $p and the wall of ice shatter to pieces!", FALSE, ch, obj, 0, TO_CHAR);

    extract_obj(obj);
    extract_obj(ice_wall);

    WAIT_STATE(ch, PULSE_VIOLENCE);

    /* Place a Broken Ice Wall in the current room. */
    OBJ *broken_ice_wall = read_object(BROKEN_ICE_WALL, VIRTUAL);

    /* Sanity check. */
    if (!broken_ice_wall) return TRUE;

    obj_to_room(broken_ice_wall, CHAR_REAL_ROOM(ch));

    /* Also need place a Broken Ice Wall in the opposite room. */
    int opposite_rnum = NOWHERE;

    if (V_ROOM(ch) == ICE_CRYSTALS_ROOM) {
      opposite_rnum = real_room(JUNCTION_ROOM);
    }
    else if (V_ROOM(ch) == JUNCTION_ROOM) {
      opposite_rnum = real_room(ICE_CRYSTALS_ROOM);
    }

    /* Sanity check. */
    if (!opposite_rnum) return TRUE;

    /* Extract the Ice Wall from the opposite room. */
    ice_wall = get_obj_by_vnum_in_room(ICE_WALL, opposite_rnum);

    if (ice_wall) {
      printf_to_room(opposite_rnum, "The wall of ice is suddenly shattered from the other side!");

      extract_obj(ice_wall);
    }

    broken_ice_wall = read_object(BROKEN_ICE_WALL, VIRTUAL);

    /* Sanity check. */
    if (!broken_ice_wall) return TRUE;

    /* Place a Broken Ice Wall in the opposite room. */
    obj_to_room(broken_ice_wall, opposite_rnum);

    return TRUE;
  }

  return FALSE;
}


/* Coninuously playing the flute will prevent Tohiri from healing herself to full. */
int flute_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_UNKNOWN) {
    if (!ch || ((ch != OBJ_CARRIED_BY(obj)) && (ch != OBJ_EQUIPPED_BY(obj))) || !AWAKE(ch) || IS_NPC(ch)) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!*buf || (strcmp(buf, "play") != 0)) return FALSE;

    one_argument(arg, buf);

    if (!*buf || !isname(buf, OBJ_NAME(obj))) {
      printf_to_char(ch, "Play what?\n\r");

      return TRUE;
    }

    if (!EQ(ch, HOLD) || (V_OBJ(EQ(ch, HOLD)) != FLUTE)) {
      act("You need hold the $F in order to play it.", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

      return TRUE;
    }

    if (GET_MANA(ch) < 50) {
      act("The magic within the $F demands more energy than you can spare.", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

      return TRUE;
    }

    act("A tranquil melody from $n's $F fills the room for a brief moment...", FALSE, ch, 0, OBJ_NAME(obj), TO_ROOM);
    act("A tranquil melody from your $F fills the room for a brief moment...", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

    act("The magic within the $F drains you of some energy.", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

    GET_MANA(ch) -= 50;

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);

    CHAR *vict = get_mob_by_vnum_in_room(TOHIRI, CHAR_REAL_ROOM(ch));

    if (!vict) return TRUE;

    act("$n seems to forget about $s worries for the moment as $e is filled with a sense of tranquility.", TRUE, vict, 0, 0, TO_ROOM);

    /* Bank is used to store mob state. */
    SET_BIT(GET_BANK(vict), WEAKENED);

    return TRUE;
  }

  return FALSE;
}


/* The Orb disables Xykloqtium's instant kill power, but makes him more dangerous. */
int orb_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_THROW) {
    if (!ch || ((ch != OBJ_CARRIED_BY(obj)) && (ch != OBJ_EQUIPPED_BY(obj))) || !AWAKE(ch) || IS_NPC(ch)) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!isname(buf, OBJ_NAME(obj))) return FALSE;

    if (obj != EQ(ch, HOLD)) {
      act("You need to hold the $F in order to throw it.", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

      return TRUE;
    }

    one_argument(arg, buf);

    CHAR *vict = NULL;

    if (GET_OPPONENT(ch) && (!*buf || isname(buf, GET_NAME(GET_OPPONENT(ch))))) {
      vict = GET_OPPONENT(ch);
    }
    else {
      vict = get_char_room_vis(ch, buf);
    }

    if (!vict) {
      act("Throw the $F at who?", FALSE, ch, 0, OBJ_NAME(obj), TO_CHAR);

      return TRUE;
    }

    if (!IS_NPC(vict) || (V_MOB(vict) != XYKLOQTIUM)) {
      act("$n throws $p at $N, but it seems to have no effect.", FALSE, ch, obj, vict, TO_NOTVICT);
      act("$n throws $p at you, but it seems to have no effect.", FALSE, ch, obj, vict, TO_VICT);
      act("You throw $p at $N, but it seems to have no effect.", FALSE, ch, obj, vict, TO_CHAR);

      obj_to_char(unequip_char(ch, HOLD), vict);

      WAIT_STATE(ch, PULSE_VIOLENCE);

      return TRUE;
    }

    act("$p flies from $n's hand and strikes $N, shattering $S demonic powers!", FALSE, ch, obj, vict, TO_NOTVICT);
    act("$p flies from your hand and strikes $N, shattering $S demonic powers!", FALSE, ch, obj, vict, TO_CHAR);

    extract_obj(unequip_char(ch, HOLD));

    act("$p cracks and falls to the ground!", FALSE, ch, obj, 0, TO_ROOM);
    act("$p cracks and falls to the ground!", FALSE, ch, obj, 0, TO_CHAR);

    OBJ *cracked_orb = read_object(CRACKED_ORB, VIRTUAL);

    if (cracked_orb) {
      obj_to_room(cracked_orb, CHAR_REAL_ROOM(ch));
    }

    /* Xykloqtium gets very angry when he is weakened. */
    act("$n snarls in uncontrolled rage and doubles $s resolve to destroy everything in sight!", FALSE, vict, 0, 0, TO_ROOM);

    SET_BIT(GET_AFF2(vict), AFF2_QUAD | AFF2_RAGE);

    /* Bank is used to store mob state. */
    SET_BIT(GET_BANK(vict), WEAKENED);

    WAIT_STATE(ch, PULSE_VIOLENCE);

    return TRUE;
  }

  return FALSE;
}


/* Ice Wall blocks passage north/south until broken with Barbarian's Rage. */
int ice_wall_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (((cmd == CMD_NORTH) && (OBJ_IN_ROOM_V(obj) == ICE_CRYSTALS_ROOM)) || ((cmd == CMD_SOUTH) && (OBJ_IN_ROOM_V(obj) == JUNCTION_ROOM))) {
    if (!ch || !AWAKE(ch) || IS_IMMORTAL(ch)) return FALSE;

    printf_to_char(ch, "A wall of ice and stone blocks your way.\n\r");

    return TRUE;
  }

  return FALSE;
}


/* Teleport characters to Market Square if they enter the Vortex. */
int vortex_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_ENTER) {
    if (!ch || IS_NPC(ch)) return FALSE;

    char buf[MIL];

    one_argument(arg, buf);

    if (!*buf || !isname(buf, OBJ_NAME(obj))) return FALSE;

    if (GET_MOUNT(ch)) {
      printf_to_char(ch, "Dismount first.\n\r");

      return TRUE;
    }

    act("$n enters the vortex and disappears!", TRUE, ch, 0, 0, TO_ROOM);
    printf_to_char(ch, "You enter the vortex and are teleported!\n\r");

    char_from_room(ch);
    char_to_room(ch, real_room(MARKET_SQUARE));

    act("$n appears in the room suddenly!", TRUE, ch, 0, 0, TO_ROOM);

    do_look(ch, "", CMD_LOOK);

    return TRUE;
  }

  return FALSE;
}


/* Room Specs */


/* Remove the Broken Ice Wall on zone reset. */
int ice_wall_room_spec(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_ZONE_RESET) {
    OBJ *broken_ice_wall = get_obj_by_vnum_in_room(BROKEN_ICE_WALL, room);

    if (!broken_ice_wall) return FALSE;

    if (OBJ_IN_ROOM_V(broken_ice_wall) == ICE_CRYSTALS_ROOM) {
      printf_to_room(room, "A jagged wall of ice and stone forms in the tunnel, blocking the way north.\n\r");
    }
    else if (OBJ_IN_ROOM_V(broken_ice_wall) == JUNCTION_ROOM) {
      printf_to_room(room, "A jagged wall of ice and stone forms in the tunnel, blocking the way south.\n\r");
    }

    extract_obj(broken_ice_wall);

    return FALSE;
  }

  return FALSE;
}


/* Burn objects in room/inventory, similar to Searing Orb, and inflict some fire damage to characters. */
int searing_room_spec(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    printf_to_room(room, "Searing flames rise from the fiery depths below, superheating the room!\n\r");

    /* Burn up stuff on the ground. */
    for (OBJ *temp_obj = world[room].contents, *next_obj; temp_obj; temp_obj = next_obj) {
      next_obj = temp_obj->next_content;

      CHAR *dummy = world[room].people;

      if (((OBJ_TYPE(temp_obj) == ITEM_SCROLL) || (OBJ_TYPE(temp_obj) == ITEM_RECIPE)) && number(0, 5)) {
        act("$p burns in bright and hot flames...", FALSE, dummy, temp_obj, 0, TO_ROOM);
        act("$p burns in bright and hot flames...", FALSE, dummy, temp_obj, 0, TO_CHAR);

        extract_obj(temp_obj);
      }

      if ((OBJ_TYPE(temp_obj) == ITEM_POTION) && number(0, 5)) {
        act("$p boils up in steam...", FALSE, dummy, temp_obj, 0, TO_ROOM);
        act("$p boils up in steam...", FALSE, dummy, temp_obj, 0, TO_CHAR);

        extract_obj(temp_obj);
      }
    }

    /* Burn up stuff in people's inventory, and burn them too for good measure. */
    for (CHAR *temp_ch = world[room].people, *next_ch; temp_ch; temp_ch = next_ch) {
      next_ch = temp_ch->next_in_room;

      if (!IS_MORTAL(temp_ch)) continue;

      for (OBJ *temp_obj = temp_ch->carrying, *next_obj; temp_obj; temp_obj = next_obj) {
        next_obj = temp_obj->next_content;

        if (((OBJ_TYPE(temp_obj) == ITEM_SCROLL) || (OBJ_TYPE(temp_obj) == ITEM_RECIPE)) && !number(0, 5)) {
          act("$p burns in bright and hot flames...", FALSE, temp_ch, temp_obj, 0, TO_ROOM);
          act("$p burns in bright and hot flames...", FALSE, temp_ch, temp_obj, 0, TO_CHAR);

          extract_obj(temp_obj);
        }

        if ((OBJ_TYPE(temp_obj) == ITEM_POTION) && !number(0, 5)) {
          act("$p boils up in steam...", FALSE, temp_ch, temp_obj, 0, TO_ROOM);
          act("$p boils up in steam...", FALSE, temp_ch, temp_obj, 0, TO_CHAR);

          extract_obj(temp_obj);
        }
      }

      damage(temp_ch, temp_ch, number(50, 100), SPELL_SEARING_ORB, DAM_FIRE);
    }

    return FALSE;
  }

  return FALSE;
}


/* Transport characters to the Abyss if they have the required objects. */
int tear_room_spec(int room, CHAR *ch, int cmd, char *arg) {
  static bool breach_is_open = FALSE;

  /* Close the breach at zone reset if it's open. */
  if (cmd == MSG_ZONE_RESET) {
    for (int i = NORTH; i <= DOWN; i++) {
      world[room].dir_option[i]->to_room_r = real_room(0);
    }

    breach_is_open = FALSE;

    printf_to_room(room, "The breach into reality seals itself closed!\n\r");

    return FALSE;
  }

  /* Block the use of items that might transport players out of the room. */
  if ((cmd == CMD_QUIT) || (cmd == CMD_USE) || (cmd == CMD_QUAFF) || (cmd == CMD_RECITE) || (cmd == CMD_DONATE)) {
    if (!ch || !AWAKE(ch) || IS_IMMORTAL(ch)) return FALSE;

    printf_to_char(ch, "That action has no meaning here...\n\r");

    return TRUE;
  }

  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch) || breach_is_open) return FALSE;

    /* Stun players entering the room. */
    printf_to_char(ch, "You feel disoriented for a few moments as the rift bends and warps in space and time...\n\r");

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);

    /* Open the breach if the character is carrying the Map, Mandate, and Sliver. */
    if (is_carrying_obj(ch, MAP) && is_carrying_obj(ch, MANDATE) && is_carrying_obj(ch, SLIVER)) {
      /* Extract the objects. */
      for (OBJ *temp_obj = ch->carrying, *next_obj; temp_obj; temp_obj = next_obj) {
        next_obj = temp_obj->next_content;

        if (V_OBJ(temp_obj) == MAP || V_OBJ(temp_obj) == MANDATE || V_OBJ(temp_obj) == SLIVER) {
          act("$p dissolves!", TRUE, ch, temp_obj, 0, TO_ROOM);
          act("$p dissolves!", TRUE, ch, temp_obj, 0, TO_CHAR);

          obj_from_char(temp_obj);
          extract_obj(temp_obj);
        }
      }

      /* Link Tear Room to a random maze room in a random direction. */
      int dest_room_rnum = real_room(number(MAZE_MIN, MAZE_MAX));

      if (!dest_room_rnum) return FALSE;

      world[room].dir_option[number(NORTH, DOWN)]->to_room_r = dest_room_rnum;

      breach_is_open = TRUE;

      printf_to_room(room, "A breach into reality has been created!\n\r");
    }

    return FALSE;
  }

  /* Slowly and randomly 'rescue' any characters stranded in the Tear Room by teleporting them somewhere far away. */
  if (cmd == MSG_MOBACT) {
    const int warp_room_table[] = {
      OLD_PATH,         /* An Old Path */
      DEEP_PASSAGE,     /* Deep, descending passage */
      ABYSSIAN_WALK,    /* The Abyssian Walk */
      DECENDING_DARK,   /* Descending into the Darkness */
      CENTRAL_COURTYARD /* The Central Courtyard */
    };

    if (breach_is_open) return FALSE;

    for (CHAR *temp_ch = world[room].people, *next_ch; temp_ch; temp_ch = next_ch) {
      next_ch = temp_ch->next_in_room;

      if (!IS_MORTAL(temp_ch) || !chance(10)) continue;

      int warp_room_rnum = real_room(warp_room_table[number(0, NUMELEMS(warp_room_table) - 1)]);

      /* Sanity check. */
      if (!warp_room_rnum) continue;

      act("$n vanishes slowly into the aether...", TRUE, temp_ch, 0, 0, TO_ROOM);
      printf_to_char(temp_ch, "Reality fades and distorts as you travel to another plane of existence...\n\r");

      char_from_room(temp_ch);
      char_to_room(temp_ch, warp_room_rnum);

      act("$n slowly materializes in the room...", TRUE, temp_ch, 0, 0, TO_ROOM);

      do_look(temp_ch, "", CMD_LOOK);
    }

    return FALSE;
  }

  return FALSE;
}


/* Assign Specs */
void assign_invasion(void) {
  assign_mob(YETI, yeti_spec);
  assign_mob(TROLL, troll_spec);
  assign_mob(BASILISK, basilisk_spec);

  assign_mob(IMP, imp_spec);
  assign_mob(LEMURE, lemure_spec);
  assign_mob(BEBILITH, bebilith_spec);
  assign_mob(OSYLUTH, osyluth_spec);
  assign_mob(HELLCAT, hellcat_spec);
  assign_mob(GELUGON, gelugon_spec);
  assign_mob(BAATEZU, baatezu_spec);

  assign_mob(MYRDON, myrdon_spec);
  assign_mob(SHADOWRAITH, shadowraith_spec);
  assign_mob(SHOMED, shomed_spec);
  assign_mob(TRYSTA, trysta_spec);
  assign_mob(VELXOK, velxok_spec);
  assign_mob(TOHIRI, tohiri_spec);
  assign_mob(STRAM, stram_spec);
  assign_mob(ANISTON, aniston_spec);
  assign_mob(LAW, law_spec);
  assign_mob(CHAOS, chaos_spec);

  assign_mob(XYKLOQTIUM, xykloqtium_spec);

  assign_obj(HORN, horn_spec);
  assign_obj(MANTLE, mantle_spec);
  assign_obj(CIRCLET, circlet_spec);
  assign_obj(TACTICAL, tactical_spec);
  assign_obj(FROSTBRAND, frostbrand_spec);
  assign_obj(BLADED_SHIELD, bladed_shield_spec);

  assign_obj(LANTERN, lantern_spec);

  assign_obj(LOCKPICKS, lockpicks_spec);
  assign_obj(DART, dart_spec);
  assign_obj(BARBARIANS_RAGE, barbarians_rage_spec);
  assign_obj(FLUTE, flute_spec);
  assign_obj(ORB, orb_spec);

  assign_obj(ICE_WALL, ice_wall_spec);
  assign_obj(VORTEX, vortex_spec);

  assign_room(ICE_CRYSTALS_ROOM, ice_wall_room_spec);
  assign_room(JUNCTION_ROOM, ice_wall_room_spec);
  assign_room(SEARING_ROOM, searing_room_spec);
  assign_room(TEAR_ROOM, tear_room_spec);
}
