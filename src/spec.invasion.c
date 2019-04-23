/*
  spec.invasion.c - Specs for Demon Invasion, by Night

  Written by Alan K. Miles for RoninMUD
*/

/* Ronin Includes */
#include "constants.h"
#include "structs.h"
#include "utils.h"
#include "act.h"
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
#include "subclass.h"
#include "utility.h"

/* Defines */

/* Rooms*/
#define TEMPLE            3001
#define MARKET_SQUARE     3014

#define ICE_CRYSTALS_ROOM 27760
#define JUNCTION_ROOM     27762
#define GLYPH_ROOM        27765
#define STAIRCASE_ROOM    27770
#define STORAGE_ROOM      27778
#define LAVA_HAZARD       27787
#define SEARING_ROOM      27789
#define PENTAGRAM_ROOM    27792
#define TEAR_ROOM         27793

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

    /* 3% Petrify random victim. */
    if (chance(3)) {
      CHAR *vict = get_random_victim_fighting(mob);

      /* Don't spec the tank. */
      if (!vict || IS_IMMORTAL(vict)) return FALSE;

      /* Reduced chance to petrify the tank. */
      if ((vict != GET_OPPONENT(vict)) || ((vict == GET_OPPONENT(vict)) && chance(25))) {
        act("$n gazes deep into $N's eyes and turns $M to stone!", FALSE, mob, 0, vict, TO_NOTVICT);
        act("$n gazes deep into your eyes and turns you to stone!", FALSE, mob, 0, vict, TO_VICT);

        spell_petrify(GET_LEVEL(mob), mob, vict, 0);
      }
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
    if (!GET_OPPONENT(mob)) {
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

    /* 15% Coin Toss to all attackers in the room. */
    if (rnd <= 15) {
      act("$n tosses a hail of coins about $m with deadly precision.", FALSE, mob, 0, 0, TO_ROOM);

      for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        /* Ignore non-mortals and non-attackers. */
        if (!IS_MORTAL(vict) || (GET_OPPONENT(vict) != mob)) continue;

        act("You are battered by a hail of coins thrown by $n!", FALSE, mob, 0, vict, TO_VICT);

        damage(mob, vict, number(50, 100), TYPE_UNDEFINED, DAM_PHYSICAL);
      }
    }
    /* 15% Kick dirt and blind a random attacker. */
    else if (rnd <= 30) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      if (!IS_AFFECTED(vict, AFF_BLIND)) {
        act("$n kicks dirt in $N's eyes, effectively blinding $M!", FALSE, mob, 0, vict, TO_NOTVICT);
        act("$n kicks dirt in your eyes, effectively blinding you!", FALSE, mob, 0, vict, TO_VICT);

        affect_apply(vict, SPELL_BLINDNESS, APPLY_HITROLL, -4, AFF_BLIND, 0, 0);
        affect_apply(vict, SPELL_BLINDNESS, APPLY_AC, 40, AFF_BLIND, 0, 0);
      }
      else {
        act("$n kicks dirt in $N's eyes, but $E is already blind!", FALSE, mob, 0, vict, TO_NOTVICT);
        act("$n kicks dirt in your eyes, but you are already blind!", FALSE, mob, 0, vict, TO_VICT);
      }
    }
    /* 50% Circle the tank. */
    else if (rnd <= 80) {
      CHAR *vict = GET_OPPONENT(mob);

      if (!vict) return FALSE;

      act("$n circles behind $N's back and stabs $s weapon into $S back!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n circles behind you and stabs $s weapon into your back!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, (calc_hit_damage(mob, vict, NULL, 0, RND_NRM) * 4), TYPE_UNDEFINED, DAM_PHYSICAL);
    }
    /* 20% Circle fail. */
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

    /* 30% Throwing stars at random attacker. */
    if (chance(30)) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n makes a series of quick movements and $N is pelted with throwing stars!", TRUE, mob, 0, vict, TO_NOTVICT);
      act("$n makes a series of quick movements and you are pelted with throwing stars!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, number(100, 200), TYPE_UNDEFINED, DAM_PHYSICAL);
    }

    return FALSE;
  }

  return FALSE;
}


/* Nomad-like specs. */
int shomed_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    /* 10% Stop Berserking and remove Dual bit. */
    if (chance(10)) {
      if (IS_SET(GET_AFF(mob), AFF_DUAL)) {
        act("$n seems exhausted from $s berserk frenzy and calms down.", TRUE, mob, 0, 0, TO_ROOM);

        REMOVE_BIT(GET_AFF(mob), AFF_DUAL);
      }

      return FALSE;
    }

    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    int rnd = number(1, 100);

    /* 30% Berserk and set Dual bit. */
    if (rnd <= 30) {
      if (!IS_SET(GET_AFF(mob), AFF_DUAL) && GET_OPPONENT(mob)) {
        act("$n works $mself into a berserk frenzy!", TRUE, mob, 0, 0, TO_ROOM);

        SET_BIT(GET_AFF(mob), AFF_DUAL);
      }
    }
    /* 50% Batter the tank. */
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

    int rnd = number(1, 100);

    /* 5% Total Recall */
    if (rnd <= 5) {
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
    else if (rnd <= 20) {
      act("$n sings 'Quen nwalme nin...'", FALSE, mob, 0, 0, TO_ROOM);
      send_to_room("The sounds of war echo in your mind!\n\r", CHAR_REAL_ROOM(mob));

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
        send_to_char("You grow weak with panic!\n\r", vict);
      }

      return FALSE;
    }
    /* 20% Blindness */
    else if (rnd <= 40) {
      act("$n sings 'Lumbo mi olos...'", FALSE, mob, 0, 0, TO_ROOM);
      send_to_room("Flashes of light fill the room!\n\r", CHAR_REAL_ROOM(mob));

      for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict) {
        next_vict = vict->next_in_room;

        /* Don't blind immortals or mobs that aren't attackers. */
        if (IS_IMMORTAL(vict) || (IS_NPC(vict) && (GET_OPPONENT(vict) != mob))) continue;

        /* Don't blind characters that are already blind or that are immune to blindness. */
        if (IS_SET(GET_AFF(vict), AFF_BLIND) || IS_IMMUNE(vict, IMMUNE_BLINDNESS)) continue;

        affect_apply(vict, SPELL_BLINDNESS, 2, -4, APPLY_HITROLL, AFF_BLIND, 0);
        affect_apply(vict, SPELL_BLINDNESS, 2, 40, APPLY_AC, AFF_BLIND, 0);

        act("$n seems to be blinded!", TRUE, vict, 0, 0, TO_ROOM);
        send_to_char("You have been blinded!\n\r", vict);
      }

      return FALSE;
    }
    /* 35% Lethal Fire */
    else if (rnd <= 75) {
      act("$n sings 'Nuruhuine...'", FALSE, mob, 0, 0, TO_ROOM);
      send_to_room("Blue flames erupt from the aether!\n\r", CHAR_REAL_ROOM(mob));

      for (CHAR *vict = world[CHAR_REAL_ROOM(mob)].people, *next_vict; vict; vict = next_vict)
      {
        next_vict = vict->next_in_room;

        /* Don't burn immortals or mobs that aren't attackers. */
        if (IS_IMMORTAL(vict) || (IS_NPC(vict) && (GET_OPPONENT(vict) != mob))) continue;

        act("$n is burned by the azure fire!", TRUE, vict, 0, 0, TO_ROOM);
        send_to_char("You are burned by the azure fire!\n\r", vict);

        damage(mob, vict, (GET_LEVEL(mob) / 2) + number(180, 230), TYPE_UNDEFINED, DAM_MAGICAL);
      }

      return FALSE;
    }

    /* Backflip every mobact. */
    CHAR *vict = GET_OPPONENT(mob);

    if (!vict) return FALSE;

    act("$n flips into the air and lands behind $N!", TRUE, mob, 0, vict, TO_NOTVICT);
    act("$n flips into the air and lands behind you!", FALSE, mob, 0, vict, TO_VICT);

    hit(mob, vict, TYPE_HIT);

    return FALSE;
  }

  return FALSE;
}


/* Velxok heals indefinitely to 1/2 HP until hit by Dart.  He also will teleport combatants. */
int velxok_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Note: Mob state is tracked using its Bank variable. */

  /* Attack the first mortal that enters the room. */
  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch) || GET_OPPONENT(mob) || (V_ROOM(mob) != GLYPH_ROOM)) return FALSE;

    do_say(mob, "Intruder!", CMD_SAY);

    hit(mob, ch, TYPE_HIT);

    return FALSE;
  }

  if (cmd == MSG_MOBACT) {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);

    /* Set HP to 1/2 if not affected by Dart spec. */
    if (!GET_BANK(mob) && (GET_HIT(mob) < (GET_MAX_HIT(mob) / 2))) {
      do_say(mob, "My magic can sustain me indefinitely!", CMD_SAY);

      GET_HIT(mob) = (GET_MAX_HIT(mob) / 2);
    }

    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    /* Taunt players if not affected by Dart spec. */
    if (!GET_BANK(mob)) {
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
    const int teleport_blacklist_table[] = {
      27738, 27748, 27749, 27767, 27779, 27785, 27787
    };

    /* 10% Teleport random player to a random room in the zone, except for blacklisted rooms. */
    if (chance(10)) {
      CHAR *vict = get_random_victim_fighting(mob);

      /* Reduced chance to teleport the tank. */
      if (!vict || ((vict == GET_OPPONENT(mob)) && chance(50))) return FALSE;

      int teleport_room_rnum = NOWHERE; 

      int count = 0;
      do {
        count++;

        /* Note: Rooms beyond 27788 should not be used and are also not in the blacklist table. */
        int teleport_room_vnum = number(27700, 27788);

        /* Find a room that isn't blacklisted. */
        bool is_blacklist_room = FALSE;
        for (int i = 0; i < NUMELEMS(teleport_blacklist_table); i++) {
          if (teleport_room_vnum == teleport_blacklist_table[i]) {
            is_blacklist_room = TRUE;
            break;
          }
        }

        if (!is_blacklist_room) {
          teleport_room_rnum = real_room(teleport_room_vnum);
        }
      } while ((teleport_room_rnum == NOWHERE) && (teleport_room_rnum == CHAR_REAL_ROOM(mob)) && (count < 20));

      /* Somehow we failed to find a suitable teleport room; abort. */
      if (!teleport_room_rnum) return FALSE;

      do_say(mob, "Be gone with you!", CMD_SAY);

      act("$n points a gnarled finger at $N and $E disappears!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n points a gnarled finger at you and you are teleported elsewhere!", FALSE, mob, 0, vict, TO_VICT);

      char_from_room(vict);
      char_to_room(vict, teleport_room_rnum);

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
int stram_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    int val = number(1, 100);

    /* 10% Incendiary Cloud */
    if (val <= 10) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n makes an arcane gesture and $N is engulfed in a cloud of flames!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n makes an arcane gesture and you are engulfed in a cloud of flames!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, 600, TYPE_UNDEFINED, DAM_FIRE);

      if (!affected_by_spell(vict, SPELL_INCENDIARY_CLOUD)) {
        affect_apply(vict, SPELL_INCENDIARY_CLOUD, 8, 0, 0, 0, 0);
      }
    }
    /* 15% Electric Shock */
    else if (val <= 25) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n makes an arcane gesture and sends bolts of electricity at $N!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n makes an arcane gesture and sends bolts of electricity at you!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, number(450, 500), TYPE_UNDEFINED, DAM_ELECTRIC);
    }
    /* 25% Vampiric Touch */
    else if (val <= 50) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n reaches out and touches $N, draining some of $S life away!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n reaches out and touches you, draining some of your life away!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, GET_LEVEL(mob) * 8, TYPE_UNDEFINED, DAM_MAGICAL);

      GET_HIT(mob) += (GET_LEVEL(mob) * 8);
    }
    /* 30% Iceball */
    else if (val <= 80) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n makes an arcane gesture and sends a ball of ice at $N!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n makes an arcane gesture and sends a ball of ice at you!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, (GET_LEVEL(mob) / 2) + number(100, 160), TYPE_UNDEFINED, DAM_COLD);
    }
    /* 20% Snipe random attacker. */
    else {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      snipe_action(mob, vict);
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
    if (!GET_BANK(mob) && (GET_HIT(mob) < GET_MAX_HIT(mob))) {
      act("$n prays to $s gods for a miracle and it is granted!", FALSE, mob, 0, 0, TO_ROOM);

      GET_HIT(mob) = GET_MAX_HIT(mob);
    }
    /* Otherwise, remove the Flute spec state. */
    else if (GET_BANK(mob)) {
      act("$n's look of tranquility fades as $e comes to $s senses.", FALSE, mob, 0, 0, TO_ROOM);

      GET_BANK(mob) = 0;
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
    if (!ch || IS_IMMORTAL(ch) || !GET_BANK(mob) || GET_OPPONENT(mob)) return FALSE;

    do_say(mob, "My brother is dead, and soon you shall join him!", CMD_SAY);

    hit(mob, ch, TYPE_HIT);

    return FALSE;
  }

  if (cmd == MSG_MOBACT) {
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

    /* Only continue with specs if in Searing Room. */
    if (V_ROOM(mob) != SEARING_ROOM) return FALSE;

    /* Recall all mortals in the room if not affected by Locket spec. */
    if (!GET_BANK(mob)) {
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

    /* 10% Force a character into an adjacent hazard. */
    if (chance(10)) {
      /* Get a victim to push off the bridge. */
      CHAR *vict = get_random_victim_fighting(mob);

      /* Don't push the tank. */
      if (!vict || (vict == GET_OPPONENT(mob))) return FALSE;

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
          send_to_char("Your paralyzation prevents your escape from a fiery death!\n\r", vict);
        }

        damage(vict, vict, 30000, TYPE_UNDEFINED, DAM_NO_BLOCK);
      }

      return FALSE;
    }

    return FALSE;
  }

  /* Don't fight unless affected by Locket spec. */
  if ((cmd == CMD_KILL) || (cmd == CMD_HIT)) {
    if (!ch || IS_IMMORTAL(ch) || GET_BANK(mob) || (V_ROOM(mob) != SEARING_ROOM)) return FALSE;

    char buf[MIL];

    one_argument(arg, buf);

    if (!*buf || !isname(buf, MOB_NAME(mob))) return FALSE;

    do_say(mob, "My duty is clear, I cannot be distracted!", CMD_SAY);

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

    do_say(mob, "You killed my brother... prepare to DIE!", CMD_SAY);

    act("$n drops the locket, which tumbles from the bridge and into the magma below.", FALSE, mob, 0, 0, TO_ROOM);

    extract_obj(obj);

    SET_BIT(GET_ACT(mob), ACT_AGGRESSIVE);

    GET_BANK(mob) = 1;

    hit(mob, ch, TYPE_HIT);

    return TRUE;
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


/* Anti-Paladin-like specs. */
int chaos_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Attack the first mortal that enters Pentagram Room, unless already engaged in combat. */
  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch) || GET_OPPONENT(mob) || (V_ROOM(mob) != PENTAGRAM_ROOM)) return FALSE;

    act("$n plunges $s weapon deep into $N's back!", FALSE, mob, 0, ch, TO_NOTVICT);
    act("As you enter the room, $n plunges $s weapon deep into your back!", FALSE, mob, 0, ch, TO_VICT);

    damage(mob, ch, number(2000, 4000), TYPE_UNDEFINED, DAM_NO_BLOCK);

    return FALSE;
  }

  if (cmd == MSG_TICK) {
    /* Cast Blackmantle if not already affected by it. */
    if (!affected_by_spell(mob, SPELL_BLACKMANTLE)) {
      cast_blackmantle(50, mob, "", SPELL_TYPE_SPELL, mob, 0);
    }

    return FALSE;
  }

  if (cmd == MSG_MOBACT) {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);

    /* 'Sprit Levy' corpses in the room. */
    for (OBJ *temp_obj = world[CHAR_REAL_ROOM(mob)].contents, *next_content; temp_obj; temp_obj = next_content) {
      next_content = temp_obj->next_content;

      /* Make sure it's a corpse. */
      if ((GET_ITEM_TYPE(temp_obj) == ITEM_CONTAINER) && OBJ_VALUE(temp_obj, 3) && (OBJ_COST(temp_obj) != PC_STATUE) && (OBJ_COST(temp_obj) != NPC_STATUE)) {
        act("$n absorbs life energy from the dead.", FALSE, mob, temp_obj, 0, TO_ROOM);

        /* Get all the stuff from the corpse and put it on the ground. */
        for (OBJ *temp_obj2 = temp_obj->contains, *next_content2; temp_obj2; temp_obj2 = next_content2) {
          next_content2 = temp_obj2->next_content;

          obj_from_obj(temp_obj2);
          obj_to_room(temp_obj2, CHAR_REAL_ROOM(mob));
        }

        extract_obj(temp_obj);

        GET_HIT(mob) += number(3333, 6666);
      }
    }

    return FALSE;
  }

  return FALSE;
}


/*   Throw people out of the room, gate in an ally, and taunt people. */
int xykloqtium_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  /* Note: Mob state is tracked using its Bank variable. */

  /* He take the Orb or the Cracked Orb. */
  if (cmd == CMD_GIVE) {
    if (!ch || !AWAKE(ch)) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    OBJ *obj = get_obj_in_list_vis(ch, buf, ch->carrying);

    if (!*buf || ((V_OBJ(obj) != ORB) && (V_OBJ(obj) != CRACKED_ORB))) return FALSE;

    one_argument(arg, buf);

    if (!*buf || !isname(buf, MOB_NAME(mob))) return FALSE;

    if (!IS_MORTAL(ch)) {
      act("$n glares at $N and refuses to take what is being offered.", FALSE, mob, 0, ch, TO_NOTVICT);
      act("$n glares at you and refuses to take what is being offered.", FALSE, mob, 0, ch, TO_VICT);

      return TRUE;
    }

    do_say(mob, "Arrgh!  I will not take that foul thing!", CMD_SAY);

    act("$n drops $p upon the round.", FALSE, mob, obj, 0, TO_ROOM);

    obj_to_room(obj_from_char(obj), CHAR_REAL_ROOM(ch));

    return TRUE;
  }

  if (cmd == MSG_MOBACT) {
    /* Refill mana. */
    GET_MANA(mob) = GET_MAX_MANA(mob);

    /* Combat specs below. */
    if (!GET_OPPONENT(mob)) return FALSE;

    /* Slay random character if not affected by Orb spec. */
    if (!GET_BANK(mob) && chance(20)) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      act("$n points $s finger at $N and utters a purely evil word.", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n points $s finger at you and utters a purely evil word.", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, 30000, TYPE_UNDEFINED, DAM_NO_BLOCK);

      return FALSE;
    }

    int rnd = number(1, 100);

    /* 10% Throw an attacker into a random Abyss room and load a Demon Spawn to attack them. */
    if (rnd <= 10) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      /* Don't throw the tank. */
      if (vict == GET_OPPONENT(mob)) return FALSE;

      /* Sanity check. */
      if ((MAZE_MAX - MAZE_MIN) <= 0) return FALSE;

      /* Find a maze room other than the one the mob is currently in. */
      int maze_room_rnum = NOWHERE;

      int count = 0;
      do {
        count++;
        maze_room_rnum = real_room(number(MAZE_MIN, MAZE_MAX));
      } while ((maze_room_rnum == NOWHERE) && (maze_room_rnum != CHAR_REAL_ROOM(mob)) && (count < 10));

      /* Somehow we failed to find a suitable maze room; abort. */
      if (!maze_room_rnum) return FALSE;

      act("$n reaches out, grabs $N, and throws $M into the swirling mists!", FALSE, mob, 0, vict, TO_NOTVICT);
      act("$n reaches out, grabs you, and throws you into the swirling mists!", FALSE, mob, 0, vict, TO_VICT);

      damage(mob, vict, number(80, 160), TYPE_UNDEFINED, DAM_NO_BLOCK);

      char_from_room(vict);
      char_to_room(vict, maze_room_rnum);

      CHAR *spawn = read_mobile(DEMON_SPAWN, VIRTUAL);

      /* Sanity check. */
      if (!spawn) return FALSE;

      char_to_room(spawn, CHAR_REAL_ROOM(vict));

      do_look(vict, "", CMD_LOOK);

      hit(spawn, vict, TYPE_HIT);
    }
    /* 15% Load a random demon ally to fight a random mortal. */
    else if (rnd <= 25) {
      CHAR *vict = get_random_victim_fighting(mob);

      if (!vict) return FALSE;

      /* Sanity check. */
      if ((GATE_MIN - GATE_MAX) <= 0) return FALSE;

      int ally_vnum = number(GATE_MIN, GATE_MAX);

      /* Replace Bebilith with Demon Spawn. */
      ally_vnum = ((ally_vnum == BEBILITH) ? DEMON_SPAWN : ally_vnum);

      CHAR *ally = read_mobile(ally_vnum, VIRTUAL);

      /* Sanity check. */
      if (!ally) return FALSE;

      act("$n utters a word of command and gates in a demonic ally!", FALSE, mob, 0, 0, TO_ROOM);

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


/* Chance to spec Miracle, Heal, Cure Critic, or Bless to all mortals in the room during combat. */
int horn_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_TICK) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner || IS_IMMORTAL(owner) || !GET_OPPONENT(owner)) return FALSE;

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

    send_to_room("The tranquil music of the sea fills the air!\n\r", CHAR_REAL_ROOM(owner));

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

  /* Set APPLY_DAMAGE between 1 and 3 at tick. */
  if (cmd == MSG_TICK) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner) return FALSE;

    /* Sanity check to make sure AFF1 is APPLY_DAMROLL. */
    if (OBJ_AFF_LOC(obj, 1) != APPLY_DAMROLL) return FALSE;

    /* Sanity check to make sure it's worn in the correct slot. */
    if (obj != EQ(owner, WEAR_ABOUT)) return FALSE;

    unequip_char(owner, WEAR_ABOUT);

    int rnd = number(1, 100);

    /* 5% 1 damage. */
    if (rnd <= 5) {
      OBJ_AFF_MOD(obj, 1) = 1;
    }
    /* 70% 2 damage. */
    else if (rnd <= 75) {
      OBJ_AFF_MOD(obj, 1) = 2;
    }
    /* 25% 3 damage. */
    else {
      OBJ_AFF_MOD(obj, 1) = 3;
    }

    equip_char(owner, obj, WEAR_ABOUT);

    return FALSE;
  }

  return FALSE;
}


int circlet_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  /* Set APPLY_MANA to 20 when removed or when the character dies. */
  if ((cmd == MSG_BEING_REMOVED) || (cmd == MSG_DIE)) {
    if (!ch || !OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj))) return FALSE;

    /* Sanity check to make sure AFF0 is APPLY_MANA. */
    if (OBJ_AFF_LOC(obj, 0) != APPLY_MANA) return FALSE;

    /* Sanity check to make sure its worn in the correct slot. */
    if (obj != EQ(ch, WEAR_HEAD)) return FALSE;

    unequip_char(ch, WEAR_HEAD);

    OBJ_AFF_MOD(obj, 0) = 20;

    equip_char(ch, obj, WEAR_HEAD);

    return FALSE;
  }

  /* Set APPLY_MANA based on the wearer's alignment at tick. */
  if (cmd == MSG_TICK) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner) return FALSE;

    /* Sanity check to make sure AFF0 is APPLY_MANA. */
    if (OBJ_AFF_LOC(obj, 0) != APPLY_MANA) return FALSE;

    /* Sanity check to make sure its worn in the correct slot. */
    if (obj != EQ(owner, WEAR_HEAD)) return FALSE;

    unequip_char(owner, WEAR_HEAD);

    if (GET_ALIGNMENT(owner) < 500) {
      OBJ_AFF_MOD(obj, 0) = 10;
    }
    else if (GET_ALIGNMENT(owner) < 600) {
      OBJ_AFF_MOD(obj, 0) = 15;
    }
    else if (GET_ALIGNMENT(owner) < 750) {
      OBJ_AFF_MOD(obj, 0) = 20;
    }
    else if (GET_ALIGNMENT(owner) < 900) {
      OBJ_AFF_MOD(obj, 0) = 25;
    }
    else {
      OBJ_AFF_MOD(obj, 0) = 30;
    }

    equip_char(owner, obj, WEAR_HEAD);

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
        send_to_char("Behave yourself here please!\n\r", ch);

        return TRUE;
      }

      if (GET_MOUNT(ch)) {
        send_to_char("Dismount first.\n\r", ch);

        return TRUE;
      }

      act("$n whirls about the room kicking, everyone in sight!", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You whirl about the room kicking, everyone in sight!\n\r", ch);

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
    if (!ch || !OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj)) || !AWAKE(ch) || !IS_MORTAL(ch) || GET_OPPONENT(ch) ) return FALSE;

    char buf[MIL];

    one_argument(arg, buf);

    if (!*buf) return FALSE;

    if (chance(9)) {
      CHAR *vict = get_char_room_vis(ch, buf);

      if (!vict || IS_IMMORTAL(vict) || (IS_MORTAL(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(vict)))) return FALSE;

      act("$n's frostbrand drains the warmth from $N as $e strikes $M!", TRUE, ch, 0, vict, TO_NOTVICT);
      act("$n's frostbrand drains the warmth from you as $e strikes you!", FALSE, ch, 0, vict, TO_VICT);
      act("Your frostbrand drains the warmth from $N as you strike $M!", FALSE, ch, 0, vict, TO_CHAR);

      damage(ch, vict, 100, TYPE_UNDEFINED, DAM_COLD);

      if (!affected_by_spell(vict, SPELL_CHILL_TOUCH)) {
        affect_apply(vict, SPELL_CHILL_TOUCH, 1, -2, APPLY_STR, 0, 0);
      }
    }

    return TRUE;
  }

  /* Chill Touch-like spec during combat. */
  if (cmd == MSG_MOBACT) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner) return FALSE;

    if (chance(9)) {
      CHAR *vict = GET_OPPONENT(owner);

      if (!vict || IS_IMMORTAL(vict) || (IS_MORTAL(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(vict)))) return FALSE;

      act("$n's frostbrand drains the warmth from $N as $e strikes $M!", TRUE, owner, 0, vict, TO_NOTVICT);
      act("$n's frostbrand drains the warmth from you as $e strikes you!", FALSE, owner, 0, vict, TO_VICT);
      act("Your frostbrand drains the warmth from $N as you strike $M!", FALSE, owner, 0, vict, TO_CHAR);

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
  if ((cmd == MSG_OBJ_WORN) || (cmd == MSG_BEING_REMOVED)) {
    if (!ch || !OBJ_CARRIED_BY(obj) || (ch != OBJ_CARRIED_BY(obj)) || !AWAKE(ch) || IS_IMMORTAL(ch)) return FALSE;

    damage(ch, ch, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);

    return FALSE;
  }

  if (cmd == MSG_MOBACT) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner) return FALSE;

    /* Combat specs below. */
    if (!GET_OPPONENT(owner)) return FALSE;

    int val = number(1, 100);

    /* 1% Cut self. */
    if (val <= 1) {
      act("$n winces in pain as $s shield slices into $s wrist!", TRUE, owner, 0, 0, TO_ROOM);
      send_to_char("You wince in pain as your shield slices into your wrist!\n\r", owner);

      damage(owner, owner, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK_NO_FLEE);
    }
    /* 5% Cut opponent. */
    else if (val <= 6) {
      CHAR *vict = GET_OPPONENT(owner);

      if (!vict || IS_IMMORTAL(vict) || (IS_MORTAL(vict) && !ROOM_CHAOTIC(CHAR_REAL_ROOM(vict)))) return FALSE;

      act("$n lashes out with $s shield and slices $N with it!", TRUE, owner, 0, vict, TO_NOTVICT);
      act("$n lashes out with $s shield and slices you with it!", FALSE, owner, 0, vict, TO_VICT);
      act("You lash out with your shield and slice $N with it!", FALSE, owner, 0, vict, TO_CHAR);

      damage(owner, vict, number(33, 99), TYPE_UNDEFINED, DAM_NO_BLOCK);
    }

    return FALSE;
  }

  return FALSE;
}


int lantern_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  /* Set ch's HP to 1 when lantern is held. */
  if (cmd == CMD_GRAB) {
    if (!ch || !OBJ_CARRIED_BY(obj) || (ch != OBJ_CARRIED_BY(obj)) || !AWAKE(ch) || IS_IMMORTAL(ch)) return FALSE;

    char buf[MIL];

    one_argument(arg, buf);

    OBJ *temp_obj = get_obj_in_list_vis(ch, buf, ch->carrying);

    if (!temp_obj || (temp_obj != obj)) return FALSE;

    GET_HIT(ch) = 1;

    return FALSE;
  }

  /* Chance to burn non-evil characters with sparks during combat. */
  if (cmd == MSG_TICK) {
    CHAR *owner = OBJ_EQUIPPED_BY(obj);

    if (!owner) return FALSE;

    /* Combat specs below. */
    if (!GET_OPPONENT(owner)) return FALSE;

    if (chance(5)) {
      act("A shower of burning sparks rains from $n's lantern!", FALSE, owner, 0, 0, TO_ROOM);
      send_to_char("A shower of burning sparks rains from your lantern!\n\r", owner);

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

  if (cmd == CMD_UNKNOWN) {
    if (!ch || !AWAKE(ch) || !IS_MORTAL(ch)) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!*buf || str_cmp(buf, "stoke")) return FALSE;

    one_argument(arg, buf);

    if (!*buf || !isname(buf, OBJ_NAME(obj))) {
      send_to_char("Stoke what?\n\r", ch);

      return TRUE;
    }

    if (!OBJ_EQUIPPED_BY(obj) || (ch != OBJ_EQUIPPED_BY(obj))) {
      send_to_char("You must equip the lantern before you can stoke its embers.\n\r", ch);

      return TRUE;
    }

    act("$n stokes the embers in $s lantern, causing them to white-hot for a brief moment.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You stoke the embers in your lantern, causing them to glow white-hot for a brief moment.\n\r", ch);

    if (GET_ALIGNMENT(ch) <= -1000) return TRUE;

    act("$n is scorched by the lantern's malevolent heat!", FALSE, ch, obj, 0, TO_ROOM);
    send_to_char("You are scorched by the lantern's malevolent heat!\n\r", ch);

    GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - 200);

    GET_HIT(ch) = MAX(1, ((GET_HIT(ch) * 2) / 3));

    WAIT_STATE(ch, PULSE_VIOLENCE);

    return TRUE;
  }

  return FALSE;
}


/* 'Picks' a normally un-pickable door. */
int lockpicks_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_PICK) {
    if (!ch || !OBJ_CARRIED_BY(obj) || (ch != OBJ_CARRIED_BY(obj)) || !AWAKE(ch) || IS_NPC(ch)) return FALSE;

    if (V_ROOM(ch) != STORAGE_ROOM) {
      send_to_char("The lockpicks don't seem to be intended to pick anything here.\n\r", ch);

      return TRUE;
    }

    char door_name[MIL];

    arg = one_argument(arg, door_name);

    if (!*door_name) return FALSE;

    char dir_name[MIL];

    one_argument(arg, dir_name);

    if (!*dir_name) return FALSE;

    int door_num = find_door(ch, string_to_lower(door_name), string_to_lower(dir_name));

    if (door_num < 0) return FALSE;

    REMOVE_BIT(EXIT(ch, door_num)->exit_info, EX_LOCKED);

    act("$n manages to unlock the mechanism with a set of magical lockpicks.", TRUE, ch, 0, 0, TO_ROOM);
    send_to_char("You manage to unlock the mechanism with a set of magical lockpicks.\n\r", ch);

    send_to_room("The magic within the lockpicks fades and they crumble to dust.\n\r", CHAR_REAL_ROOM(ch));

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
      act("You need to hold $p in order to throw it.", FALSE, ch, obj, 0, TO_CHAR);

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
      act("Throw $p at who?", FALSE, ch, obj, 0, TO_CHAR);

      return TRUE;
    }

    if (!IS_NPC(vict) || (V_MOB(vict) != VELXOK)) {
      act("$n throws $p at $N, but it seems to have no effect.", FALSE, ch, obj, vict, TO_NOTVICT);
      act("$n throws $p at you, but it seems to have no effect.", FALSE, ch, obj, vict, TO_VICT);
      act("You throw $p at $N, but it seems to have no effect.", FALSE, ch, obj, vict, TO_CHAR);

      obj_to_char(unequip_char(ch, HOLD), vict);

      return TRUE;
    }

    /* Only wounds Velxok if he's at <= 1/2 max_hps. */
    if (GET_HIT(vict) > (GET_MAX_HIT(vict) / 2)) {
      act("$n throws $p at $N, but it is deflected by $S mana shield!", FALSE, ch, obj, vict, TO_NOTVICT);
      act("You throw $p at $N, but it is deflected by $S mana shield!", FALSE, ch, obj, vict, TO_CHAR);

      act("$p clatters to the floor, having no effect!", FALSE, ch, obj, 0, TO_ROOM);

      obj_to_room(unequip_char(ch, HOLD), CHAR_REAL_ROOM(vict));

      return TRUE;
    }

    act("$n throws $p at $N which strikes $M in the chest and dissolves!", FALSE, ch, obj, vict, TO_NOTVICT);
    act("You throw $p at $N which strikes $M in the chest and dissolves!", FALSE, ch, obj, vict, TO_CHAR);

    act("$n shudders in agony and clutches at $s chest!", FALSE, vict, 0, 0, TO_ROOM);

    REMOVE_BIT(GET_ACT(vict), ACT_SHIELD);

    /* Bank is used to store mob state. */
    GET_BANK(vict) = 1;

    extract_obj(obj);

    WAIT_STATE(ch, PULSE_VIOLENCE);

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

    if (!*buf || str_cmp(buf, "break")) return FALSE;

    if (obj != EQ(ch, WIELD)) {
      act("You feel the insatiable urge to wield $p and break something with it!", FALSE, ch, obj, 0, TO_CHAR);

      return TRUE;
    }

    one_argument(arg, buf);

    if (!*buf) {
      send_to_char("Break what?\n\r", ch);

      return TRUE;
    }

    OBJ *ice_wall = get_obj_by_vnum_in_room(ICE_WALL, CHAR_REAL_ROOM(ch));

    if (!ice_wall) {
      send_to_char("There's nothing here to break that would satisfy your rage!\n\r", ch);

      return TRUE;
    }

    if (!isname(buf, OBJ_NAME(ice_wall))) {
      send_to_char("Breaking that wouldn't satisfy your rage!\n\r", ch);

      return TRUE;
    }

    act("$n slams $p into the $P!", FALSE, ch, obj, ice_wall, TO_ROOM);
    act("You slam $p into the $P!", FALSE, ch, obj, ice_wall, TO_CHAR);

    act("$p, and the $P, shatter to pieces!", FALSE, ch, obj, ice_wall, TO_ROOM);
    act("$p, and the $P, shatter to pieces!", FALSE, ch, obj, ice_wall, TO_CHAR);

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
      snprintf(buf, sizeof(buf), "The %s is suddenly shattered from the other side!\n\r", OBJ_SHORT(ice_wall) ? OBJ_SHORT(ice_wall) : "something");

      send_to_room(buf, opposite_rnum);

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

    if (!*buf || str_cmp(buf, "play")) return FALSE;

    one_argument(arg, buf);

    if (!*buf || !isname(buf, OBJ_NAME(obj))) {
      send_to_char("Play what?\n\r", ch);

      return TRUE;
    }

    if (!EQ(ch, HOLD) || (V_OBJ(EQ(ch, HOLD)) != FLUTE)) {
      act("You need hold $p in order to play it.", FALSE, ch, obj, 0, TO_CHAR);

      return TRUE;
    }

    if (GET_MANA(ch) < 25) {
      act("The magic within $p demands more energy than you can spare.", FALSE, ch, obj, 0, TO_CHAR);

      return TRUE;
    }

    act("A tranquil melody from $n's flute fills the room for a brief moment...", FALSE, ch, 0, 0, TO_ROOM);
    act("A tranquil melody from your flute fills the room for a brief moment...", FALSE, ch, 0, 0, TO_CHAR);

    act("The magic within $p drains you of some energy.", FALSE, ch, obj, 0, TO_CHAR);

    GET_MANA(ch) -= 25;

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);

    CHAR *vict = get_mob_by_vnum_in_room(TOHIRI, CHAR_REAL_ROOM(ch));

    if (!vict) return TRUE;

    act("$n seems to forget about $s worries for the moment as $e is filled with a sense of tranquility.", TRUE, vict, 0, 0, TO_ROOM);

    /* Bank is used to store mob state. */
    GET_BANK(vict) = 1;

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
      act("You need to hold $p in order to throw it.", FALSE, ch, obj, 0, TO_CHAR);

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
      act("Throw $p at who?", FALSE, ch, obj, 0, TO_CHAR);

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

    extract_obj(obj);

    act("$p cracks and falls to the ground!", FALSE, ch, obj, 0, TO_ROOM);
    act("$p cracks and falls to the ground!", FALSE, ch, obj, 0, TO_CHAR);

    OBJ *cracked_orb = read_object(CRACKED_ORB, VIRTUAL);

    if (cracked_orb) {
      obj_to_room(cracked_orb, CHAR_REAL_ROOM(ch));
    }

    /* Xykloqtium gets very angry when he is weakened. */
    act("$n snarls in uncontrolled rage and doubles $s resolve to destroy everything in sight!", FALSE, vict, 0, 0, TO_ROOM);

    SET_BIT(GET_AFF2(vict), AFF2_QUAD);

    /* Bank is used to store mob state. */
    GET_BANK(vict) = 1;

    WAIT_STATE(ch, PULSE_VIOLENCE);

    return TRUE;
  }

  return FALSE;
}


/* Ice Wall blocks passage north/south until broken with Barbarian's Rage. */
int ice_wall_spec(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (((cmd == CMD_NORTH) && (OBJ_IN_ROOM_V(obj) != ICE_CRYSTALS_ROOM)) || ((cmd == CMD_SOUTH) && (OBJ_IN_ROOM_V(obj) != JUNCTION_ROOM))) {
    if (!ch || !AWAKE(ch) || IS_IMMORTAL(ch)) return FALSE;

    send_to_char("A wall of ice and stone blocks your way.\n\r", ch);

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
      send_to_char("Dismount first.\n\r", ch);

      return TRUE;
    }

    act("$n enters the vortex and disappears!", TRUE, ch, 0, 0, TO_ROOM);
    send_to_char("You enter the vortex and are teleported!\n\r", ch);

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
      send_to_room("A jagged wall of ice and stone forms in the tunnel, blocking the way north.\n\r", room);
    }
    else if (OBJ_IN_ROOM_V(broken_ice_wall) == JUNCTION_ROOM) {
      send_to_room("A jagged wall of ice and stone forms in the tunnel, blocking the way south.\n\r", room);
    }

    extract_obj(broken_ice_wall);

    return FALSE;
  }

  return FALSE;
}


/* Burn objects in room/inventory, similar to Searing Orb, and inflict some fire damage to characters. */
int searing_room_spec(int room, CHAR *ch, int cmd, char *arg) {
  if (cmd == MSG_MOBACT) {
    send_to_room("Searing flames rise from the fiery depths below, super-heating the room!\n\r", real_room(SEARING_ROOM));

    /* Burn up stuff on the ground. */
    for (OBJ *temp_obj = world[real_room(SEARING_ROOM)].contents, *next_obj; temp_obj; temp_obj = next_obj) {
      next_obj = temp_obj->next_content;

      if (((GET_ITEM_TYPE(temp_obj) == ITEM_SCROLL) || (GET_ITEM_TYPE(temp_obj) == ITEM_RECIPE)) && number(0, 5)) {
        act("$p burns in bright and hot flames...", FALSE, ch, temp_obj, 0, TO_ROOM);
        act("$p burns in bright and hot flames...", FALSE, ch, temp_obj, 0, TO_CHAR);

        extract_obj(temp_obj);
      }

      if ((GET_ITEM_TYPE(temp_obj) == ITEM_POTION) && number(0, 5)) {
        act("$p boils up in steam...", FALSE, ch, temp_obj, 0, TO_ROOM);
        act("$p boils up in steam...", FALSE, ch, temp_obj, 0, TO_CHAR);

        extract_obj(temp_obj);
      }
    }

    /* Burn up stuff in people's inventory, and burn them too for good measure. */
    for (CHAR *temp_ch = world[real_room(SEARING_ROOM)].people, *next_ch; temp_ch; temp_ch = next_ch) {
      next_ch = temp_ch->next_in_room;

      if (!IS_MORTAL(temp_ch)) continue;

      for (OBJ *temp_obj = temp_ch->carrying, *next_obj; temp_obj; temp_obj = next_obj) {
        next_obj = temp_obj->next_content;

        if (((GET_ITEM_TYPE(temp_obj) == ITEM_SCROLL) || (GET_ITEM_TYPE(temp_obj) == ITEM_RECIPE)) && !number(0, 5)) {
          act("$p burns in bright and hot flames...", FALSE, ch, temp_obj, 0, TO_ROOM);
          act("$p burns in bright and hot flames...", FALSE, ch, temp_obj, 0, TO_CHAR);

          extract_obj(temp_obj);
        }

        if ((GET_ITEM_TYPE(temp_obj) == ITEM_POTION) && !number(0, 5)) {
          act("$p boils up in steam...", FALSE, ch, temp_obj, 0, TO_ROOM);
          act("$p boils up in steam...", FALSE, ch, temp_obj, 0, TO_CHAR);

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

    send_to_room("The breach into reality seals itself closed!\n\r", real_room(TEAR_ROOM));

    return FALSE;
  }

  /* Block the use of items that might transport players out of the room. */
  if ((cmd == CMD_QUIT) || (cmd == CMD_USE) || (cmd == CMD_QUAFF) || (cmd == CMD_RECITE) || (cmd == CMD_DONATE)) {
    if (!ch || !AWAKE(ch) || !IS_MORTAL(ch)) return FALSE;

    send_to_char("That action has no meaning here...\n\r", ch);

    return TRUE;
  }

  if (cmd == MSG_ENTER) {
    if (!ch || !IS_MORTAL(ch)) return FALSE;

    /* Stun players entering the room if the breach is open. */
    if (breach_is_open) {
      send_to_char("You feel disoriented for a few moments as the rift bends and warps in space and time...\n\r", ch);

      WAIT_STATE(ch, PULSE_VIOLENCE * 2);

      return FALSE;
    }

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

      send_to_room("A breach into reality has been created!\n\r", real_room(TEAR_ROOM));
    }

    return FALSE;
  }

  /* Slowly and randomly 'rescue' any characters stranded in the Tear Room by teleporting them somewhere far away. */
  if (cmd == MSG_MOBACT) {
    const int warp_room_table[] = {
       5401, /* An Old Path */
       5973, /* Deep, descending passage */
      16640, /* The Abyssian Walk */
      16921, /* Descending into the Darkness */
      25040, /* The Central Courtyard */
    };

    if (breach_is_open) return FALSE;

    for (CHAR *temp_ch = world[room].people, *next_ch; temp_ch; temp_ch = next_ch) {
      next_ch = temp_ch->next_in_room;

      if (!IS_MORTAL(temp_ch) || !chance(1)) continue;

      int warp_room_rnum = real_room(warp_room_table[number(0, NUMELEMS(warp_room_table) - 1)]);

      if (!warp_room_rnum) continue;

      act("$n vanishes slowly into the aether...", TRUE, temp_ch, 0, 0, TO_ROOM);
      send_to_char("Reality fades and distorts as you travel to another plane of existence...\n\r", temp_ch);

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
