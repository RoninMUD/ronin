 /*
///   Special Procedure Module                   Orig. Date 11-07-1995
///
///   spec.elmuseo.c --- Specials for El Museo
///
///   Designed by Zorak and Ai of RoninMUD (sputnik@u.washington.edu) and
///   (qxotl@u.washington.edu) coded by Ai of RoninMUD (qxotl@u.washington.edu)
///   Using this code is not allowed without permission from originator.
///
///   Museo II Addon specs by Ranger - Jan 99
///
///   Museo III Addon specs by Skeena - Dec 2010
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:30 $
$Header: /home/ronin/cvs/ronin/spec.elmuseo.c,v 2.0.0.1 2004/02/05 16:10:30 ronin Exp $
$Id: spec.elmuseo.c,v 2.0.0.1 2004/02/05 16:10:30 ronin Exp $
$Name:  $
$Log: spec.elmuseo.c,v $
Revision 2.0.0.1  2004/02/05 16:10:30  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "regex.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"
#include "cmd.h"
#include "enchant.h"
#include "spec_assign.h"
#include "subclass.h"
#include "act.h"
#include "mob.spells.h"
#include "interpreter.h"

//
// External prototypes
//
extern struct descriptor_data *descriptor_list;
extern struct time_info_data time_info;

extern int CHAOSMODE;

extern char *spells[];
extern char *songs[];
extern struct idname_struct idname[MAX_ID];

extern void death_list(CHAR *ch);
extern bool is_caster(CHAR *CH);
extern int drop_object(CHAR *ch, OBJ *obj);

//command_interpreter() assumes the char buffer is mutable, so you can't call it with a string constant
//This function copies it to a temp buffer, so it can be called with string constant
void mob_do(CHAR* mob, char *arg)
{
  char mutable_buffer[MAX_INPUT_LENGTH];
  strcpy( mutable_buffer, arg );
  command_interpreter(mob,mutable_buffer);
}

//
// #defines for El Museo
//

// Miscellaneous strings
#define ENCH_NAIAD_FROZEN "The icy spectre of the naiad"
#define ENCH_NAIAD_CHILLING "The chilling spectre of the naiad"
#define ENCH_NAIAD_COWARDICE "Cowardice"
#define ENCH_FEEBLE1 "The enfeebling affects of age"
#define ENCH_FEEBLE2 "The enfeebling affects of senility"
#define ENCH_FOREST_FRIEND "Lem's rugged good looks"
#define ENCH_LOVELORN "The feeling of being Lovelorn"
#define ENCH_LOVESTRUCK "The feeling of being Lovestruck"
#define ENCH_TRUELOVE "The feeling of True Love"
#define ENCH_WAXEN    "The all-consuming cloak of the chandler"

#define WAX_DURATION 10

#define ZONE_MUSEO              16500
#define ZONE_SHIP               7500
#define ZONE_MAR                2700

// Other zones
//    Mobs
#define PIRATE_CAPTAIN      ITEM(ZONE_SHIP,0)
#define NERGAL_LARVAE       6801
#define NERGAL_BODYGUARD    6802
#define MAR_GHOST           ITEM (ZONE_MAR,5)

#define HELL_IRON_BOOTS     25302
#define HELL_IRON_BOOTS_B   25402
//    Rooms
#define LA_SOUDURE_DAIMYO_EAST  7220
#define DAIMYO_DOCKS            500
#define SKELOS_PORTAL           30032

//    Objects
#define DIAMOND_BAR             3018
#define WALL_THORNS             34
#define MAR_QUEEN_SHIELD        ITEM (ZONE_MAR,20)
#define MAR_QUEEN_STING         ITEM(ZONE_MAR,16)

// Mobs
#define MUS_FAZE                16507
#define MUS_BILL                16508
#define MUS_JENNY               ITEM(ZONE_MUSEO, 26)
#define MUS_GORGO               ITEM(ZONE_MUSEO, 15)
#define MUS_HAKEEM              ITEM(ZONE_MUSEO, 27)
#define MUS_HAKEEM_GHOST        16528
#define MUS_TOMEK               16542
#define MUS_FIRE                16558
#define MUS_VAPOR               16564
#define MUS_CURATOR             16569
#define MUS_KHALLAQ             16532
#define MUS_QUARATT             16535
#define MUS_TIMUR               16555
#define MUS_MAMA                16554
#define MUS_NAIAD               16551
#define MUS_HORSE               16513

#define MUS_PANTHER1            16538
#define MUS_PANTHER2            16566

#define MUS_FLAYED_KIMAGEE      16547


#define MUS_LEM                 16557

// Objects
#define MUS_GORGO_CANINE         16516
#define MUS_GORGO_MOLAR          16501
#define MUS_GORGO_NAMETAG        16533
#define MUS_TOMEK_CANINE         16542
#define MUS_TOMEK_NAMETAG        16543
#define MUS_TOMEK_GOLDEN_NAMETAG 16510
#define MUS_WAX_POT_HAKEEM       16520
#define MUS_WAX_POT_PC           16519
#define MUS_KEYCHAIN             16539
#define MUS_CLOAK                16521
#define MUS_LEM_CLOAKS1          16522
#define MUS_LEM_CLOAKS2          16523
#define MUS_LEM_CLOAKS3          16524
#define MUS_CORNELIUS_CAPE       16524
#define MUS_WAX_BONE_SHIELD      16526
#define MUS_BATTERING_RAM        16540
#define MUS_BRIDLE1              16513
#define MUS_BRIDLE2              16562
#define MUS_LEAF_SPEAR           16561
#define MUS_MUKLUKS              16545
#define MUS_TOMTOM               16537
#define MUS_KIMAGEE_HIDE         16569
#define MUS_KIMAGEE_HIDE_B       16570
#define MUS_WHITE_BAND           16558
#define MUS_BLACK_BAND           16559
#define MUS_BLACK_LEGGINGS       16560
#define MUS_LEAF_SPEAR           16561
#define MUS_ELAR_SKULL           16566
#define MUS_ANTLERS              16567
#define MUS_MITTENS              16568

// Rooms
#define MUS_BOUDOIR              16526
#define MUS_UP_FROM_HAKEEM       16525
#define MUS_SOUTH_FROM_HAKEEM    16528
#define MUS_NORTH_FROM_GORGO     16514
#define MUS_DROWNING             16569
#define MUS_SANCTUARY            16598
#define MUS_RED_MIST             16532
#define MUS_DJINN_FIRE_ROOM      16558
#define MUS_DJINN_VAPOR_ROOM     16564
#define MUS_HAKEEM_ROOM          16527
#define MUS_GEMBAL_ROOM          16536
#define MUS_TOMEK_ROOM           16542
#define MUS_NAIAD_ROOM           16551
#define MUS_QUARATT_ROOM         16535
#define MUS_UP_FROM_GEMBAL_SNAKE 16531
#define MUS_GEMBAL_SNAKE_ROOM    16536
#define MUS_LEM_ROOM             16557
#define MUS_ESCAPE_TUNNEL        16571
#define MUS_LIVING_ROOM          16553
#define MUS_HALLWAY_NORTH_OF_WYVERN 16511
#define MUS_HALL_OF_WYVERNS      16568
#define MUS_ENCASED_IN_WAX       16570

// Tracks an object that has been waxified.
struct wax_object
{
  struct obj_data *obj;
  int ticks_left;
  char *description;
  char *short_description;
  struct wax_object *next;
};

// Head of linked list of waxified objects.
struct wax_object *wax_objects = NULL;

bool tried_to_find_skeena = FALSE;

// List of killers of Lem`s mobs.
CHAR *khallaq_killer = NULL;
CHAR *quaratt_killer = NULL;
CHAR *mama_killer = NULL;
CHAR *timur_killer = NULL;
CHAR *tomek_killer = NULL;

CHAR *lem_instance = NULL;
CHAR *hakeem_instance = NULL;
CHAR *timur_instance = NULL;
CHAR *faze_instance = NULL;

bool timur_soothed = FALSE;
bool hakeem_died = FALSE;

int (*real_tomek_func)(struct char_data*, struct char_data*, int, char*) = NULL;

int skeena_id = -1;
int ench_naiad_cowardice(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int extract_spell(char *arg);
int extract_song(char *arg);

// We use Skeena's ID as an ownerid for waxed items so we can recover in the event of a reboot/crash
int get_skeena_id()
{
  int i = -1;
  for( i = 0; i < MAX_ID; i++ )
    if( !strcasecmp("skeena", idname[i].name) ) return i;

  log_f("ELWAXO: could not find Skeena's ID");
  return -1;
}

void mus_remeq(CHAR *victim, int pos) {
  char buf[MAX_INPUT_LENGTH];
  OBJ *o;
  struct descriptor_data *d;

  if(!victim) return;
  d=victim->desc;
  if(d && d->connected==CON_PLYNG) {

    if(IS_NPC(victim)) return;
    o = EQ(victim, pos);
    if(!o) return;
    unequip_char (victim, pos);
    obj_to_room (o, CHAR_REAL_ROOM(victim));
    sprintf(buf,"ELWAXO: [ %s had %s removed at %d ]",GET_NAME(victim),OBJ_SHORT(o),world[CHAR_REAL_ROOM(victim)].number);
    log_s(buf);
    o->log=1;
    save_char(victim,NOWHERE);
  }
}


//
// Enchantment-related functions
//
ENCH* find_enchantment(CHAR *ch, char *enchantment_name)
{
  struct enchantment_type_5 *ench     =NULL;
  struct enchantment_type_5 *next_ench=NULL;
  for(ench=ch->enchantments;ench;ench=next_ench)
  {
    next_ench = ench->next;
    if(!strcmp(ench->name,enchantment_name))
    {
      return ench;
    }
  }
  return NULL;
}


// At end of enchantment, or when Jenny and her master are in different rooms,
// send Jenny home.
void jenny_home( CHAR *jenny )
{
    stop_follower( jenny );
    act("$n is suddenly pulled away!", FALSE, jenny, 0, NULL, TO_ROOM);
    act("You feel the pull of the museum.", FALSE, jenny, 0, NULL, TO_CHAR);
    char_from_room( jenny );
    char_to_room( jenny, real_room( MUS_BOUDOIR ) );
}

// Enchantment on waxified players.
int ench_waxen(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  if(cmd==MSG_REMOVE_ENCH)
    send_to_char("Your waxen prison crumbles.	\r\n", ench_ch);

  if( ench_ch == ch && (cmd==CMD_RENT || cmd == CMD_QUIT) )
  {
    send_to_char("The receptionist looks at your wax-coated form and smirks.\r\n", ench_ch);
    send_to_char("You get the feeling they don't need your custom while you're\r\n", ench_ch);
    send_to_char("coated in wax.\r\n", ench_ch);
    return TRUE;
  }
  return FALSE;
}

int ench_waxen1(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
  {
    strcat(arg,"A wax-covered ");
    return FALSE;
  }

  return ench_waxen(ench, ench_ch, ch, cmd, arg);
}

int ench_waxen2(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
  {
    strcat(arg,"A wax-encrusted ");
    return FALSE;
  }

  return ench_waxen(ench, ench_ch, ch, cmd, arg);
}

int ench_waxen3(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
  {
    strcat(arg,"A waxen ");
    return FALSE;
  }

  return ench_waxen(ench, ench_ch, ch, cmd, arg);
}

// Enchantment on characters who unsuccessfully flirt with Jenny.
int ench_lovelorn(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  if(cmd==MSG_REMOVE_ENCH)
    send_to_char("The pain of Jenny, the Sorcerer's Consort's rejection subsides.\r\n", ench_ch);

  if( ench_ch == ch && (cmd == CMD_RENT || cmd == CMD_QUIT) )
  {
    send_to_char("Jenny's rejection is crushing, but that's no reason to do this!\r\n", ench_ch );
    return TRUE;
  }
  return FALSE;
}

// Enchantment on mage who successfully flirts with Jenny.
int ench_lovestruck(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  //Verify NPC and PC are in the same room, if not, lose charm bit.
  CHAR* jenny = NULL;
  struct follow_type* followers = NULL;

  // If you do it on actual movement commands, the msg is sent in the time when they are in different rooms.
  // So, do it on MSG_MOBACT.
  if( cmd == MSG_MOBACT )
  {
    for( followers = ench_ch->followers; followers; followers = followers->next )
    {
      if( followers->follower && IS_NPC( followers->follower ) && followers->follower->nr_v == MUS_JENNY )
      {
        jenny = followers->follower;
        if( CHAR_REAL_ROOM(ench_ch) != CHAR_REAL_ROOM(jenny) )
        {
          jenny_home( jenny );
        }
        break;
      }
    }
  }

  return FALSE;
}

int ench_forest_friend(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg );

// Enchantment on Jenny when she is successfully flirted with.
int ench_truelove(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg )
{
  char buf[MAX_INPUT_LENGTH];
  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    if( ch == ench_ch->master )
    {
      act("......$n is madly in love with you!", FALSE, ench_ch, NULL, ch, TO_VICT );
    }
    else if( ench_ch->master )
    {
      sprintf(buf, "......$n is madly in love with %s!", ench_ch->master->player.name );
      act(buf, FALSE, ench_ch, NULL, ch, TO_VICT );
    }
  }
  else if( cmd == MSG_REMOVE_ENCH )
  {
    // May not have a master if player has broken her heart and been in different room.
    if( ench_ch->master )
    {
      CHAR *vict = ench_ch->master;
      stop_follower( ench_ch );
      act("$n is struck suddenly with remorse and claws your eyes!",
          FALSE,ench_ch,0,vict,TO_VICT);
      act("$n is struck with remorse for Hakeem and claws at $N's eyes!",
          FALSE,ench_ch,0,vict,TO_NOTVICT);
      act("You are struck with remorse and claw at $N's eyes!",
          FALSE,ench_ch,0,vict,TO_CHAR);
      WAIT_STATE(vict, 2*PULSE_VIOLENCE);
      damage (ench_ch, vict, 600, TYPE_UNDEFINED,DAM_NO_BLOCK);
      spell_blindness(50, ench_ch, vict, NULL );
      jenny_home( ench_ch );
    }
  }

  return FALSE;
}

// Applies one of the enchantments related to Jenny.
void jenny_enchant( CHAR *ch, char *enchantment_name )
{
  ENCH *tmp_enchantment;
  CREATE(tmp_enchantment, ENCH, 1);
  tmp_enchantment->name     = str_dup( enchantment_name );
  tmp_enchantment->duration = 30;            /* Define the enchantment */

  if(!strcmp(enchantment_name, ENCH_TRUELOVE ) )
    tmp_enchantment->func     = ench_truelove;

  if(!strcmp(enchantment_name, ENCH_LOVELORN))
    tmp_enchantment->func     = ench_lovelorn;

  if(!strcmp(enchantment_name, ENCH_LOVESTRUCK))
    tmp_enchantment->func     = ench_lovestruck;

  if(!strcmp(enchantment_name, ENCH_FOREST_FRIEND))
    tmp_enchantment->func     = ench_forest_friend;

  enchantment_to_char(ch, tmp_enchantment, FALSE);
}

// Removes a waxified item from the list of waxified items.
// This is called when the tick counter reaches 0.
void delete_wax_node( struct wax_object *node )
{
  if( node == wax_objects )
  {
    wax_objects = node->next;
  }
  else
  {
    struct wax_object *tmp = wax_objects;
    while( tmp != NULL )
    {
      if( tmp->next == node )
      {
        tmp->next = node->next;
        break;
      }

      tmp = tmp->next;
    }
  }

  free( node );
}

// Count the ticks on wax equipment. When it hits 0, put the affects back and delete it from
// the wax_objects list.
//
// NOTE: This is incredibly slow: we do an O(N) search through the list of waxified items
// for _each_ waxified item, so we`re O(N^2).
//
// In practice, this is probably OK, but if we see big slowdowns with groups doing Hakeem,
// we can revisit this to be less stupid.
int mus_waxen_equipment(OBJ *eq, CHAR *ch,int cmd,char *arg)
{
  int pos = 0, iEq;
  if( cmd == MSG_TICK )
  {
    struct wax_object *wax = wax_objects;
    while(wax != NULL )
    {
      struct wax_object *tmp = wax->next;

      if( wax->obj == eq )
      {
        wax->ticks_left--;

        if( wax->ticks_left <= 0 )
        {
          struct obj_data *eq = wax->obj;

          ch = eq->equipped_by;
          if (ch)
          {
            pos = -1;
            for (iEq = 0; iEq < MAX_WEAR; ++iEq)
            {
              if (eq->equipped_by->equipment[iEq] == eq)
              {
                pos = iEq;
                break;
              }
            }
            unequip_char(ch, pos);
          }
          int i;
          for(i = 0; i < MAX_OBJ_AFFECT; i++ )
            if(!( eq->affected[i].location >= 25 && eq->affected[i].location <= 51 )) // don't affect SKILL_ slots
              eq->affected[i].modifier *= -1;

          eq->ownerid[7] = 0;
          eq->func = NULL;

          free( eq->short_description );
          free( eq->description );

          eq->short_description = wax->short_description;
          eq->description = wax->description;

          if (ch)
            equip_char(ch, eq, pos);
          delete_wax_node( wax );
          break;
        }
      }

      wax = tmp;
    }
  }

  return FALSE;
}

// Returns true if item is in Paktat-language.
bool is_paktat_eq( int num )
{
  return (num >= 12800 && num <= 12818) || num == 12824 || num == 12826;
}

void waxify( CHAR* victim )
{
  // If they're already waxed, don't do it again...
  if( enchanted_by( victim, ENCH_WAXEN ) )
    return;

  if( skeena_id == -1 )
  {
    log_f("ELWAXO: waxify cannot function, skeena_id == -1" );
    return;
  }

  // Overall intent: Person and their gear becomes wax-covered.
  //
  // Their title is prefaced with A wax-covered, A waxen, A wax-encrusted
  // Their gear is prefaced with "A wax-covered"
  //   ...unless it is in range 12800-12818, 12824, 12826 (paktat items)
  //   ...then it is "a maz-kufaemnaet"
  //
  // The bonuses on their gear are inverted.
  ENCH *tmp_enchantment;
  CREATE(tmp_enchantment, ENCH, 1);
  tmp_enchantment->name     = str_dup( ENCH_WAXEN );
  tmp_enchantment->duration = WAX_DURATION;

  // Pick a random title.
  int title = number(0,2);
  tmp_enchantment->func     = ench_waxen1;
  if(title==1)
    tmp_enchantment->func = ench_waxen2;

  if(title==2)
    tmp_enchantment->func = ench_waxen3;

  //enchantment_to_char(victim, tmp_enchantment, FALSE);
  //Can't use enchantment_to_char: we need to be the first enchantment so we stack nicely with rank pretitles
  ENCH *player_enchantments = victim->enchantments;
  victim->enchantments = tmp_enchantment;
  victim->enchantments->next = player_enchantments;

  // Screw with their gear.  Only affect gear where func == NULL. Otherwise, there`s
  // the chance that a clever individual could wax twice to get perma inversion (e.g.,
  // take voodoo doll from being -15 dam to being +15 dam).
  int pos = 0;
  char buf[MAX_INPUT_LENGTH];
  for(pos = 0; pos <= HOLD; pos++ )
  {
    struct obj_data *obj = victim->equipment[pos];
    if( !obj || obj->func )
      continue;

    // Bone shields are destroyed and replaced permanently with a waxen version that does not have the spec.
    if( obj->item_number_v == MAR_QUEEN_SHIELD )
    {
      log_f("ELWAXO: %s had bone shield destroyed", victim->player.name );
      extract_obj( unequip_char( victim, WEAR_SHIELD ) );
      struct obj_data *wax_shield = read_object( MUS_WAX_BONE_SHIELD, VIRTUAL );
      obj_to_char( wax_shield, victim );
      equip_char( victim, wax_shield, WEAR_SHIELD );
      continue;
    }

    int i;
    if( obj->ownerid[7] == skeena_id )
    {
      log_f("ELWAXO: !waxing player %s, item %s (%d), has Skeena owner", victim->player.name, OBJ_SHORT(obj), obj->item_number_v );
      continue;
    }
    else if( obj->ownerid[7] != 0 )
    {
      log_f("ELWAXO: !waxing player %s, item %s (%d), has non-Skeena owner (%d)", victim->player.name, OBJ_SHORT(obj), obj->item_number_v, obj->ownerid[7] );
      continue;
    }

    log_f("ELWAXO: waxing player %s, item %s (%d)", victim->player.name, OBJ_SHORT(obj), obj->item_number_v );
    unequip_char(victim, pos);
    struct wax_object *wax_entry;
    CREATE(wax_entry, struct wax_object, 1);
    wax_entry->next = wax_objects;
    wax_objects = wax_entry;
    wax_entry->obj = obj;
    wax_entry->ticks_left = WAX_DURATION + 1;
    wax_entry->short_description = obj->short_description;
    wax_entry->description = obj->description;

    for(i = 0;i < MAX_OBJ_AFFECT;i++) {
      if(!( obj->affected[i].location >= 25 && obj->affected[i].location <= 51 )) // don't affect SKILL_ slots
        obj->affected[i].modifier *= -1;
    }

    obj->ownerid[7] = skeena_id;
    sprintf(buf, is_paktat_eq( obj->item_number_v ) ? "A maz-kuatzaet %s" : "A wax-coated %s",
        obj_proto_table[obj->item_number].short_description );
    obj->short_description = strdup(buf);

    sprintf(buf, is_paktat_eq( obj->item_number_v ) ? "A maz-kuatzaet %s is here." : "A wax-coated %s is here.",
        obj_proto_table[obj->item_number].short_description );
    obj->description = strdup(buf);

    obj->func = mus_waxen_equipment;
    equip_char(victim, obj, pos);
  }
}

//
// Mob specs
//

int mus_jenny (CHAR *jen, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  char buf[MAX_INPUT_LENGTH];

  if(!jen)
    return FALSE;

  if( ch == jen )
    return FALSE;

  if(cmd == MSG_UNKNOWN)
  {
    arg = one_argument(arg, buf);
    if(!strcasecmp("flirt", buf))
    {
      arg = one_argument(arg, buf );
      if( !*buf)
      {
        if( !jen->specials.fighting && !enchanted_by( jen, ENCH_TRUELOVE) )
          send_to_char( "Jenny, the Sorcerer's Consort perks up at your playful acts.\r\n", ch);
        return FALSE;
      }
      else if( !strcasecmp("girl", buf) || !strcasecmp("jenny", buf ) )
      {
        if( jen->specials.fighting )
        {
          send_to_char( "Jenny, the Sorcerer's Consort seems a little busy right now.", ch);
          return FALSE;
        }

        //Jenny is not going to cheat on Hakeem...while he`s alive.
        if( mob_proto_table[real_mobile(MUS_HAKEEM)].number > 0)
        {
          do_say(jen, "I'm flattered, but I am Hakeem's... til death do us part!", CMD_SAY );
          return FALSE;
        }

        //Jenny is not going to cheat on her new found love, either.
        if( enchanted_by(jen, ENCH_TRUELOVE) )
        {
          if( ch == jen->master )
          {
            do_say(jen, "Oh, you!", CMD_SAY );
          }
          else if( jen->master )
          {
            sprintf(buf, "I'm %s's!", jen->master->player.name );
            do_say(jen, buf, CMD_SAY );
          }
          else
          {
            do_say(jen, "Twice scorned by men I considered true loves... thank you, but I will pass!", CMD_SAY );
          }
          return FALSE;
        }

        //Jenny doesn`t like being bothered repeatedly.
        if( enchanted_by( ch, ENCH_LOVELORN ) || enchanted_by( ch, ENCH_LOVESTRUCK ) )
        {
          return FALSE;
        }

        //Jenny has a particular kind of taste.
        if( ch->player.class != CLASS_MAGIC_USER )
        {
          do_say(jen, "You're just not my type!", CMD_SAY );
          return FALSE;
        }

        //Jenny is a traditional sort of girl.
        if( ch->player.sex == SEX_FEMALE )
        {
          jenny_enchant( ch, ENCH_LOVELORN );
          act("$n smiles warmly at you and you feel funny inside.",
              FALSE,jen,0,ch,TO_VICT);
          act("$n smiles warmly at $N, who looks a little lovestruck.",
              FALSE,jen,0,ch,TO_NOTVICT);
          act("You smile at $N, leaving them lovestruck.",
              FALSE,jen,0,ch,TO_CHAR);

          switch(number(1,10))
          {
          case 1:
          case 2:
            spell_recover_mana( 0, jen, ch, NULL );
            break;
          case 3:
          case 4:
          case 5:
            spell_miracle( 0, jen, ch, NULL );
            break;
          default:
            spell_vitality( 0, jen, ch, NULL );
            break;
          }
          return FALSE;
        }

        // Jenny`s a bit of a gold-digger.
        if( enchanted_by(ch, "The title of Sorcerer" ) )
        {
          if( number(1,2) == 1 )
          {
            // Jenny sometimes has changes of hearts about her personal relationships.
            jenny_enchant( ch, ENCH_LOVELORN );
            do_say(jen, "How can you even think of such a thing after Hakeem's death?!?", CMD_SAY);
            spell_wrath_ancients(50, jen, ch, NULL );
            return FALSE;
          }

          // True love!
          jenny_enchant( ch, ENCH_LOVESTRUCK );
          jenny_enchant( jen, ENCH_TRUELOVE );
          REMOVE_BIT( jen->specials.act, ACT_SENTINEL ); // otherwise she won`t follow player
          SET_BIT(jen->specials.affected_by, AFF_CHARM);
          add_follower( jen, ch );
          return FALSE;
        }
        else if( enchanted_by(ch, "The title of Warlock" ) || enchanted_by(ch, "The title of Apprentice" ))
        {
          sprintf(buf, "Do I look like Jenny, the %s's Consort to you?!?", enchanted_by(ch, "The title of Warlock" ) ? "Warlock" : "Apprentice" );
          do_say(jen, buf, CMD_SOCIAL );

          sprintf(buf, "slap %s", ch->player.name );
          command_interpreter( jen, buf);
          damage (jen, ch, 400, TYPE_UNDEFINED,DAM_NO_BLOCK);
          return FALSE;
        }
        else
        {
          sprintf(buf, "Just %s? I like my men to have titles.", ch->player.name );
          do_say( jen, buf, CMD_SOCIAL );
          return FALSE;
        }
      }
    }
    return FALSE;
  }
  else if(cmd == MSG_MOBACT && !jen->specials.fighting && !enchanted_by( jen, ENCH_TRUELOVE ) )
  {
    if( chance(50) )
    {
      switch(number(0,6))
      {
        case 0:
          do_say(jen, "I'm so lonely...", CMD_SOCIAL );
          break;
        case 1:
          do_say(jen, "Sometimes I wonder if Hakeem ever really loved me.", CMD_SOCIAL );
          break;
        case 2:
          mob_do( jen, "sigh" );
          break;
        case 3: mob_do( jen, "shy" ); break;
        case 4: mob_do( jen, "cry" ); break;
        default: break;
      }
    }
    return FALSE;
  }

  if(cmd)
    return FALSE;

  if (number(0,8))
    return FALSE;

  if (!(vict = get_random_victim(jen)))
    return FALSE;

  if(vict == jen)
    return FALSE;

  if( jen->specials.fighting && !enchanted_by( jen, ENCH_TRUELOVE) ) {
    act("$n's hair whips like the wind and cuts your flesh, leaving a huge gash.",
        FALSE,jen,0,vict,TO_VICT);
    act("$n whips $s hair at $N leaving a huge, bleeding gash.",
        FALSE,jen,0,vict,TO_NOTVICT);
    act("You whip your long hair around and leave a huge gash on $N's face.",
        FALSE,jen,0,vict,TO_CHAR);
    damage (jen, vict, 300, TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }

  return FALSE;
}

// Hurts people wearing gear that came from a Warek; causes them to remove it.
int warek_spec( CHAR *gor )
{
  CHAR *vict = NULL;
	CHAR *next_vict = NULL;

  // If anyone`s using gear from Gorgo/Tomek, remove it.
  for(vict = world[gor->in_room_r].people; vict; vict = next_vict)
  {
		next_vict = vict->next_in_room;
    // Gorgo/Tomek are exempt, of course.
    if( vict == gor )
      continue;

    int pos = -1;
    if( is_wearing_obj( vict, MUS_GORGO_CANINE, WIELD )
        || is_wearing_obj( vict, MUS_GORGO_MOLAR, WIELD )
        || is_wearing_obj( vict, MUS_TOMEK_CANINE, WIELD ) )
      pos = WIELD;

    if( is_wearing_obj( vict, MUS_GORGO_NAMETAG, WEAR_NECK_1 )
        || is_wearing_obj( vict, MUS_TOMEK_NAMETAG, WEAR_NECK_1 )
        || is_wearing_obj( vict, MUS_TOMEK_GOLDEN_NAMETAG, WEAR_NECK_1 ) )
      pos = WEAR_NECK_1;

    if( is_wearing_obj( vict, MUS_GORGO_NAMETAG, WEAR_NECK_2 )
        || is_wearing_obj( vict, MUS_TOMEK_NAMETAG, WEAR_NECK_2 )
        || is_wearing_obj( vict, MUS_TOMEK_GOLDEN_NAMETAG, WEAR_NECK_2 ) )
      pos = WEAR_NECK_2;

    if( pos != -1 )
    {
      mus_remeq( vict, pos );
      WAIT_STATE(vict, 5*PULSE_VIOLENCE);
      if( pos == WIELD )
      {
        act ("$n stares in confusion at your hands momentarily...",
             FALSE, gor, 0, vict, TO_VICT);
        act ("...then launches himself at you and claws at your weapon!",
             FALSE, gor, 0, vict, TO_VICT);
        act ("$n claws $N, causing $M to drop $S weapon.",
             FALSE, gor, 0, vict, TO_NOTVICT);
        act ("You are very confused...what is $N wielding?!?",
             FALSE, gor, 0, vict, TO_CHAR);
      }
      else
      {
        act ("$n growls as he rips the nametag from your throat.",
             FALSE, gor, 0, vict, TO_VICT);
        act ("$n throws $N against the wall and rips the nametag from around $S neck.",
             FALSE, gor, 0, vict, TO_NOTVICT);
        act ("You are very confused...what does $N have around $S neck?",
             FALSE, gor, 0, vict, TO_CHAR);
      }
      damage(gor, vict, number(350,450), TYPE_UNDEFINED,DAM_NO_BLOCK);
      return TRUE;
    }
  }

  return FALSE;
}

int mus_tomek_hemp(CHAR *mek, CHAR *ch, int cmd, char *arg) {
  CHAR *vict = NULL, *vict1 = NULL, *vict2 = NULL, *vict3 = NULL, *vict4 = NULL, *vict5 = NULL, *next_vict = NULL;
  int i = 0, j = 0, f = 0, specnum = 0, num = 0;
  OBJ *wield = NULL;

  if( !mek )
    return FALSE;

  if((cmd==CMD_FLEE || (cmd==CMD_WEST && !ch->specials.fighting) || cmd==CMD_RECITE) && IS_MORTAL(ch))
  {
    vict = ch;
    if(chance(20))
    {
      act("$n growls 'Gruuma gruuma lahlimonga!'\n",FALSE,mek,0,0,TO_ROOM);
    }
    else if( chance(20) )
    {
      act("$n growls 'Ruuga monga orgolahli!'\n",FALSE,mek,0,0,TO_ROOM);
    }
    act("$n rolls towards you in a vicious whirling ball of teeth, claws, and fur!",0,mek,0,vict,TO_VICT);
    act("$n rolls towards $N in a whirling ball of teeth, claws, and fur!",0,mek,0,vict,TO_NOTVICT);
    act("You curl into a tight ball and fling yourself at $N!",0,mek,0,vict,TO_CHAR);

    if(chance(5) & !IS_SET(mek->specials.affected_by,AFF_FURY))
    {
      SET_BIT(mek->specials.affected_by,AFF_FURY);
      act("$n starts snarling and fuming with rage!",FALSE,mek,0,0,TO_ROOM);
		}
    else if(chance(60))
    {
      act("$n screeches as he swipes his claws across your back.",FALSE,mek,0,vict,TO_VICT);
      act("$n screeches as he swipes his claws across $N's back.",FALSE,mek,0,vict,TO_NOTVICT);
      act("You screech loudly as you swipe your claws across $N's back.",FALSE,mek,0,vict,TO_CHAR);
      damage(mek,vict,number(100,200),TYPE_UNDEFINED,DAM_PHYSICAL);
    }
    else
    {
      act("$n roars in savage triumph as he bites into your throat!",FALSE,mek,0,vict,TO_VICT);
      act("$n roars in savage triumph as he bites into $N's throat!",FALSE,mek,0,vict,TO_NOTVICT);
      act("You roar in savage triumph as you bite into $N's throat!",FALSE,mek,0,vict,TO_CHAR);
      damage(mek,vict,number(200,300),TYPE_UNDEFINED,DAM_PHYSICAL);
    }
    return FALSE;
  }

  if(cmd==MSG_TICK && !(mek->specials.fighting))
  {
    REMOVE_BIT(mek->specials.affected_by,AFF_FURY);
		return FALSE;
  }

  if(cmd!=MSG_MOBACT) return FALSE;

  if((2*GET_MAX_HIT(mek)/5) <= GET_HIT(mek))
  {   /* if is > 40% hp */
    REMOVE_BIT(mek->specials.act,ACT_SHIELD);
  }
  else
  {
    SET_BIT(mek->specials.act,ACT_SHIELD);
  }

  if(mek->specials.fighting)
  {
    specnum = 0;
    f=number(1,100);
    if(f < 10) specnum = 2;
    else if( f < 30) specnum = 1;

    for(i=0; i<specnum; i++)
    {
      num=number(0,4);
      switch(num)
      {
        case 0:
          for(j=0; j<3; j++)
          {
            vict=get_random_victim_fighting(mek);
            if(vict && vict!=mek && CHAR_REAL_ROOM(vict)==CHAR_REAL_ROOM(mek))
            {
              act("$n snarls ferociously at you, and you feel your knees go weak.",FALSE,mek,0,vict,TO_VICT);
              act("$n snarls ferociously at a panic-stricken $N.",FALSE,mek,0,vict,TO_NOTVICT);
              act("You snarl ferociously at $N, causing $S to soil $Sself.",FALSE,mek,0,vict,TO_CHAR);
              damage(mek,vict,number(50,150),TYPE_UNDEFINED,DAM_NO_BLOCK);

              if(EQ(vict,WIELD) && IS_MORTAL(vict))
              {
                wield = EQ(vict,WIELD);
                act("Tomek's bone-chilling snarl causes you to drop your weapon.",FALSE,mek,0,vict,TO_VICT);
                act("Tomek's bone-chilling snarl causes $N to drop $S weapon.",FALSE,mek,0,vict,TO_NOTVICT);
  							log_f("ELWAXO: %s's %s disarmed by Tomek.", vict->player.name, OBJ_SHORT(wield) );
  							obj_to_room(unequip_char(vict,WIELD), CHAR_REAL_ROOM( vict ) );
              }
            }
          }
          break;
        case 1:
          act("$n hurls himself at the cavern walls causing icicles to rain down from the ceiling.",FALSE,mek,0,vict,TO_ROOM);
          act("You hurl yourself at the wall causing icicles to rain down from the ceiling.",FALSE,mek,0,vict,TO_CHAR);
          for (vict = world[CHAR_REAL_ROOM(mek)].people; vict; vict = next_vict)
          {
            next_vict = vict->next_in_room;
            if(saves_spell(vict, SAVING_SPELL,GET_LEVEL(mek)))
            {
              damage(mek,vict,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
            }
            else
            {
              damage(mek,vict,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
            }
          }
          break;
        case 2:
          act("$n exhales a huge blast of arctic air!",FALSE,mek,0,vict,TO_ROOM);
          act("You exhale a huge blast of arctic air!",FALSE,mek,0,vict,TO_CHAR);
          vict1=get_random_victim_fighting(mek);
          vict2=get_random_victim_fighting(mek);
          vict3=get_random_victim_fighting(mek);
					vict4=get_random_victim_fighting(mek);
					vict5=get_random_victim_fighting(mek);

          act("You are frozen by $n's icy breath!",FALSE,mek,0,vict1,TO_VICT);
          act("$N is frozen by $n's icy breath!",FALSE,mek,0,vict1,TO_NOTVICT);
          act("$N is frozen by your icy breath!",FALSE,mek,0,vict1,TO_CHAR);
          damage(mek,vict1,175,TYPE_UNDEFINED,DAM_COLD);
          WAIT_STATE(vict1,number(1,3)*PULSE_VIOLENCE);

          if( vict1 != vict2 )
          {
            act("You are frozen by $n's icy breath!",FALSE,mek,0,vict2,TO_VICT);
            act("$N is frozen by $n's icy breath!",FALSE,mek,0,vict2,TO_NOTVICT);
            act("$N is frozen by your icy breath!",FALSE,mek,0,vict2,TO_CHAR);
            damage(mek,vict2,150,TYPE_UNDEFINED,DAM_COLD);
            WAIT_STATE(vict2,number(1,3)*PULSE_VIOLENCE);
          }

          if( vict1 != vict3 && vict2 != vict3 )
          {
            act("You are frozen by $n's icy breath!",FALSE,mek,0,vict3,TO_VICT);
            act("$N is frozen by $n's icy breath!",FALSE,mek,0,vict3,TO_NOTVICT);
            act("$N is frozen by your icy breath!",FALSE,mek,0,vict3,TO_CHAR);
            damage(mek,vict3,125,TYPE_UNDEFINED,DAM_COLD);
            WAIT_STATE(vict3,number(1,3)*PULSE_VIOLENCE);
          }

          if( vict1 != vict4 && vict2 != vict4 && vict3 != vict4 )
          {
            act("You are frozen by $n's icy breath!",FALSE,mek,0,vict3,TO_VICT);
            act("$N is frozen by $n's icy breath!",FALSE,mek,0,vict3,TO_NOTVICT);
            act("$N is frozen by your icy breath!",FALSE,mek,0,vict3,TO_CHAR);
            damage(mek,vict4,100,TYPE_UNDEFINED,DAM_COLD);
            WAIT_STATE(vict4,number(1,3)*PULSE_VIOLENCE);
          }

          if( vict1 != vict5 && vict2 != vict5 && vict3 != vict5 && vict4 != vict5 )
          {
            act("You are frozen by $n's icy breath!",FALSE,mek,0,vict3,TO_VICT);
            act("$N is frozen by $n's icy breath!",FALSE,mek,0,vict3,TO_NOTVICT);
            act("$N is frozen by your icy breath!",FALSE,mek,0,vict3,TO_CHAR);
            damage(mek,vict5,75,TYPE_UNDEFINED,DAM_COLD);
            WAIT_STATE(vict5,number(1,3)*PULSE_VIOLENCE);
          }
          break;
        case 3:
          vict1=get_random_victim_fighting(mek);
          vict2=get_random_victim_fighting(mek);

          int failcount = 0;
          while(failcount < 10 && (vict2==vict1 || vict2==mek->specials.fighting))
          {
            vict2=get_random_victim_fighting(mek);
            failcount++;
          }

          if( (vict1 != vict2) && (vict1 != mek->specials.fighting) )
          {
            act("Tomek picks up $n and throws $s at $N!",FALSE,vict1,0,vict2,TO_NOTVICT);
            act("Tomek picks you up and throws you at $N!",FALSE,vict1,0,vict2,TO_CHAR);
            act("Tomek picks up $n and throws $s at you!",FALSE,vict1,0,vict2,TO_VICT);
            damage(mek,vict1,200,TYPE_UNDEFINED,DAM_PHYSICAL);
            damage(mek,vict2,400,TYPE_UNDEFINED,DAM_NO_BLOCK);
            stop_fighting(vict2);
            WAIT_STATE(vict1,2*PULSE_VIOLENCE);
            WAIT_STATE(vict2,3*PULSE_VIOLENCE);
          }
          break;
        case 4:
					vict=get_random_victim_fighting(mek);
			    act("$n grabs you suddenly and hurls you at the glacial wall!",0,mek,0,vict,TO_VICT);
			    act("$n grabs $N suddenly and hurls $S limp body at the glacial wall!",0,mek,0,vict,TO_NOTVICT);
			    act("$N startles you! You grab $S fragile body and hurl it at the wall!",0,mek,0,vict,TO_CHAR);
			    WAIT_STATE(vict,3*PULSE_VIOLENCE);
			    damage(mek,vict,250,TYPE_UNDEFINED,DAM_NO_BLOCK);
			    break;
        default:
          break;
      } //end switch
    } //end for
  } //end if(mek->specials.fighting)

  return FALSE;
}

/*
int mus_tomek_skeena (CHAR *tom, CHAR *ch, int cmd, char *arg) {
  if (!tom)
    return FALSE;

  // Prevent climbing up: stun and 200 damage.
  if( cmd == CMD_CLIMB )
  {
    act ("$n thumps you solidly and prevents you from climbing.",
         FALSE, tom, 0, ch, TO_VICT);
    act ("Tomek thumps $N soundly, preventing $m from climbing.",
         FALSE, tom, 0, ch, TO_NOTVICT);
    act ("You thump $N soundly, preventing $M from climbing.",
         FALSE, tom, 0, ch, TO_CHAR);
    WAIT_STATE(ch, 3*PULSE_VIOLENCE);
    damage (tom, ch, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
    GET_POS(ch) = POSITION_SITTING;
    return TRUE;
  }

  if((3*GET_MAX_HIT(tom)/5) <= GET_HIT(tom))
  {   // if > 60% hp
    REMOVE_BIT(tom->specials.act,ACT_SHIELD);
  }
  else
  { // if < 60% hp
    SET_BIT(tom->specials.act,ACT_SHIELD);
  }

  if (cmd)
    return FALSE;


  CHAR *vict;
  CHAR *next_vict;
  // Short-circuit if there is no-one in room or only Tomek in room.
  if( !count_mortals_room(tom, TRUE ) ) return FALSE;

  if(!(vict=world[tom->in_room_r].people))
    return FALSE;

  if(vict == tom) return FALSE;

  // Punish PCs who are wearing Gorgo/Tomek gear.
  if( warek_spec( tom ) )
    return FALSE;

  // Room spec 500-1000 damage and lag.
  // Tomek`s job is to be a royal pain in the ass.
  if (number(0,4))
    return FALSE;

  act ("$n screeches 'Monga monga lahliglah!!!'\n", FALSE, tom, 0, vict, TO_ROOM);
  act ("You feel as if you've lost a little weight as Tomek claws you open!\n",
       FALSE, tom, 0, vict, TO_ROOM);
  act ("You claw the abdomens of everyone in the room, cutting them wide open!!!\n",
       FALSE, tom, 0, vict, TO_CHAR);
  for( vict = world[tom->in_room_r].people; vict; vict = next_vict )
  {
		next_vict = vict->next_in_room;
    if( vict == tom || GET_LEVEL(vict) >= LEVEL_IMM)
      continue;

    damage (tom, vict, number(500,1000), TYPE_UNDEFINED,DAM_NO_BLOCK);
    WAIT_STATE(vict, number(1,4)*PULSE_VIOLENCE);
  }

  return FALSE;
}
*/


int mus_tomek (CHAR *tom, CHAR *ch, int cmd, char *arg) {
  // Just to screw with the PCs a little bit, each boot we`ll pick a different
  // spec func for Tomek.

  if( real_tomek_func == NULL ) {
    //if(chance(50)) {
    //  real_tomek_func = mus_tomek_skeena;
    //  GET_ALIGNMENT(tom) = 2;
    //} else {
    real_tomek_func = mus_tomek_hemp;
    GET_ALIGNMENT(tom) = 1;
    //}
  }

  if( cmd == MSG_DIE )
  {
    tomek_killer = ch;
    act ("$n screeches 'Monga omekla $N!' with his last breath.",
        FALSE, tom, 0, ch, TO_ROOM);
    return FALSE;
  }

  return real_tomek_func( tom, ch, cmd, arg );
}

int mus_gorgo (CHAR *gor, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  if (cmd || !gor )
    return FALSE;

  if(!(gor->specials.fighting)) {

    if(!(vict=world[gor->in_room_r].people))
      return FALSE;

    if(vict == gor) return FALSE;
    if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) return FALSE;

    act ("$n whirls at you in a ball of teeth, fur, and fury!",
         FALSE, gor, 0, vict, TO_VICT);
    act ("Gorgo curls up into a ball of teeth and fur and whirls towards $N!",
         FALSE, gor, 0, vict, TO_NOTVICT);
    act ("You curl up into a tight ball and fling yourself at $N!",
         FALSE, gor, 0, vict, TO_CHAR);
    WAIT_STATE(vict, 3*PULSE_VIOLENCE);
    act ("$n screeches as he digs his claws into your throat.",
         FALSE, gor, 0, vict, TO_VICT);
    act ("$n grabs $N by the throat and shakes $S like a rag doll.",
         FALSE, gor, 0, vict, TO_NOTVICT);
    act ("You are very afraid of $N, so you must KILL $S!!!",
         FALSE, gor, 0, vict, TO_CHAR);
    damage(gor, vict, 700, TYPE_UNDEFINED,DAM_NO_BLOCK);
    return FALSE;
  }


  if( warek_spec( gor ) )
    return FALSE;

  if (number(0,4))
    return FALSE;

  if (!(vict = get_random_victim(gor)))
    return FALSE;

  if (vict == gor)
    return FALSE;

  act ("$n growls 'Monga monga lahliglah!'\n", FALSE, gor, 0, vict, TO_ROOM);

  act ("You feel as if you've lost a little weight as Gorgo claws you open!\n",
       FALSE, gor, 0, vict, TO_VICT);
  act ("Gorgo rakes $s claws across $N's belly in an attempt to disembowel!\n",
       FALSE, gor, 0, vict, TO_NOTVICT);
  act ("You claw $N's abdomen and cut $M wide open!\n",
       FALSE, gor, 0, vict, TO_CHAR);
  damage (gor, vict, 420, TYPE_UNDEFINED,DAM_NO_BLOCK);
  return FALSE;
}

int hakeem_waxify( CHAR* hak, CHAR* victim )
{
  if( enchanted_by( victim, ENCH_WAXEN ) )
  {
    return FALSE;
  }

  //Is there a pot of wax here and a player who is not yet waxed?
  struct obj_data *room_contents = NULL;

  for( room_contents = world[CHAR_REAL_ROOM(hak)].contents; room_contents; room_contents = room_contents->next_content )
  {
    if( room_contents->item_number_v == MUS_WAX_POT_HAKEEM )
    {
      break;
    }
  }

  // No pot of wax.
  if( !room_contents )
    return FALSE;

  // Oh, dear.
  extract_obj( room_contents );
  act ("$n throws a small pot of rapidly-cooling wax at you!  You are drenched in the wax, which quickly hardens around you and your gear.",
    FALSE, hak, 0, victim, TO_VICT);
  act ("$n throws a small pot of rapidly-cooling wax at $N who is quickly encased in hardened wax!",
    FALSE, hak, 0, victim, TO_NOTVICT);
  act ("You throw a small pot of rapidly-cooling wax at $N with uncanny accuracy. That seems reasonable, since Webster's defines uncanny as \"too unlikely to seem natural or human\", and well, let's face it, you're made of wax.",
    FALSE, hak, 0, victim, TO_CHAR);

  waxify( victim );

  // Wyverns are banished to the hall of wyverns
  if( enchanted_by( victim, "Wyvern lycanthropy" ) )
  {
    do_say( hak, "You will make an excellent specimen for my hall of wyverns!", CMD_SAY );
    act ("$n makes a hand gesture and you feel yourself being pulled upwards!",
      FALSE, hak, 0, victim, TO_VICT);
    act ("$n makes a hand gesture causing $N to disappear in a shower of wax clumps!",
      FALSE, hak, 0, victim, TO_NOTVICT);
    act ("You make a hand gesture, causing $N to be summoned to the hall of wyverns!",
      FALSE, hak, 0, victim, TO_CHAR);

    char_from_room(victim);
    char_to_room(victim,real_room(MUS_HALL_OF_WYVERNS));
    do_look(victim,"",CMD_LOOK);
    act("$n appears in the middle of the room.", FALSE, victim, 0, 0, TO_ROOM);
  }

  return TRUE;
}

// Faze helps Bill if he gets attacked.
int mus_faze( CHAR *faze, CHAR *ch, int cmd, char *arg )
{
  if( cmd == MSG_DIE ) faze_instance = NULL; else faze_instance = faze;
  return FALSE;
}

// Bill, the Knight Janitor, periodically drops off new pots of wax for Hakeem until he has at least 5.
int mus_bill( CHAR *bill, CHAR *ch, int cmd, char *arg )
{
  if( !bill || cmd != MSG_MOBACT )
    return FALSE;

  // If fighting, Faze will try to come assist.
  if( GET_POS( bill ) == POSITION_FIGHTING && faze_instance && GET_POS( faze_instance ) == POSITION_STANDING )
  {
    mob_do( faze_instance, "op door" );
    mob_do( faze_instance, "north" );
    mob_do( faze_instance, "assist bill" );
  }

  // Sleeping or fighting => no wax for you!
  if( GET_POS( bill ) != POSITION_STANDING )
    return FALSE;

  int count = 0;
  struct obj_data *room_contents = NULL;
  for( room_contents = world[real_room(MUS_HAKEEM_ROOM)].contents; room_contents; room_contents = room_contents->next_content )
    if( room_contents->item_number_v == MUS_WAX_POT_HAKEEM )
      count++;

  if( count < 5 )
  {
    OBJ* tmp_obj = read_object( MUS_WAX_POT_HAKEEM, VIRTUAL );
    obj_to_room( tmp_obj, real_room(MUS_HAKEEM_ROOM ) );
    send_to_room( "Bill, the Knight Janitor arrives.\r\n", real_room(MUS_HAKEEM_ROOM) );
    send_to_room( "Bill, the Knight Janitor drops a small pot of rapidly-cooling wax.\r\n", real_room(MUS_HAKEEM_ROOM) );
    send_to_room( "Bill, the Knight Janitor departs.\r\n", real_room(MUS_HAKEEM_ROOM) );
  }

  return FALSE;
}

int mus_leaf_spear(OBJ *spear, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL;
  char buf[MAX_STRING_LENGTH];

  if (cmd == MSG_BEING_REMOVED) {
    if (ch != spear->equipped_by) return FALSE;
    if (!ch) return FALSE;
    if (spear == EQ(ch,WIELD)) {
      unequip_char(ch, WIELD);
      spear->obj_flags.value[3] = 14; // revert to Slice
      equip_char(ch, spear, WIELD);
    }
    return FALSE;
  } else if (cmd == MSG_OBJ_WORN) {
    if (GET_CLASS(ch) == CLASS_THIEF) {
      sprintf(buf, "You alter your grip slightly on the %s.\n\r", OBJ_SHORT(spear));
      send_to_char(buf, ch);
      unequip_char(ch, WIELD);
      spear->obj_flags.value[3] = 10; // change to Pierce
      equip_char(ch, spear, WIELD);
    } else { // just to be safe and for disarms
      unequip_char(ch, WIELD);
      spear->obj_flags.value[3] = 14; // revert to Slice
      equip_char(ch, spear, WIELD);
    }
    return FALSE;
  }

  CHAR *owner = spear->equipped_by;
  if(cmd != MSG_MOBACT || !owner || !owner->specials.fighting) return FALSE;
  //Only spec in forests
  vict = owner->specials.fighting;
  if((GET_HIT(owner)>-1) && (world[CHAR_REAL_ROOM(owner)].sector_type == SECT_FOREST) && vict) {
    act("Guided through the forest by your spear, you leap toward $N.", 1, owner, 0, vict, TO_CHAR);
    act("$n disappears into the forest momentarily before leaping toward you.", 1, owner, 0, vict, TO_VICT);
    act("$n disappears into the forest momentarily before leaping toward $N.", 1, owner, 0, vict, TO_NOTVICT);
    hit(owner, vict, TYPE_UNDEFINED);
  }
  return FALSE;
}

bool keys_to_ring( OBJ* chain );
// If the keychain is carried/equipped by someone who is carrying keys,
// the player will put them in the keychain.
//
// This avoids the nightmare of parsing all.foo and whatnot, at the price of
// some flexibility. It seems like a reasonable price to pay, though.
int mus_keychain(OBJ *chain, CHAR *ch,int cmd,char *arg)
{
  if( cmd == CMD_PRACTICE && ch == chain->equipped_by )
  {
    send_to_char( "You feel a little funny practicing while holding that key chain.\r\n", ch );
    return TRUE;
  }

  if( !chain || cmd != MSG_TICK || (!chain->carried_by && !chain->equipped_by) )
    return FALSE;

  CHAR *victim = chain->carried_by;
  if( !victim ) victim = chain->equipped_by;
  if( keys_to_ring( chain ) )
  {
    act ("$n fiddles with $s keys.\r\n", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char( "You fiddle idly with your keys.\r\n", victim );
  }

  return FALSE;
}

// Allows a player to throw a pot of wax at another player.
// Differs from hakeem`s version because it has a chance of failure -- player
// douses themself with wax.
int mus_pc_pot(OBJ *pot, CHAR *ch,int cmd,char *arg)
{
  if( !ch || cmd != CMD_THROW )
    return FALSE;

  if( EQ(ch, HOLD) != pot )
    return FALSE;

  char buf[MAX_INPUT_LENGTH];

  if( GET_CLASS(ch) != CLASS_NOMAD && GET_CLASS(ch) != CLASS_NINJA && GET_CLASS(ch) != CLASS_THIEF && GET_CLASS(ch) != CLASS_WARRIOR )
    return FALSE;

  arg = one_argument(arg,buf);
  if( !isname(buf, OBJ_NAME(pot) ) )
    return FALSE;

  one_argument(arg, buf );

  if( !buf[0] )
    return FALSE;

  CHAR *victim;
  if (!(victim = get_char_room_vis(ch, buf)))
  {
    send_to_char("Your target isn't here.\r\n", ch);
    return TRUE;
  }

  // Based on how well they`ve practiced throw, they`ll either hit their target, or hit themselves.
  // If throwing at themselves, they`ll always succeed.
  if( victim != ch )
  {
    int percent = number(1,101);
    if( ROOM_CHAOTIC( CHAR_REAL_ROOM( ch ) ) ) //Skelos + Chaos night, greater chance to fail
      percent = number(1,200);

    if( percent > ch->skills[SKILL_THROW].learned )
    {
      act ("$n attempts to throw a pot of wax at you...",
          FALSE, ch, 0, victim, TO_VICT);
      act ("...but stumbles, dousing $mself with molten wax!",
          FALSE, ch, 0, victim, TO_VICT);
      act ("$n stumbles while throwing a pot of wax at $N, causing $n to be doused in molten wax!",
          FALSE, ch, 0, victim, TO_NOTVICT);
      act ("You attempt to throw a pot of wax at $N...",
          FALSE, ch, 0, victim, TO_CHAR);
      act ("...but stumble, dousing yourself with molten wax!",
          FALSE, ch, 0, victim, TO_CHAR);
      victim = ch;
    }
    else
    {
      act ("$n attempts to throw a pot of wax at you...",
          FALSE, ch, 0, victim, TO_VICT);
      act ("...and succeeds, dousing you with molten wax!",
          FALSE, ch, 0, victim, TO_VICT);
      act ("$n douses $N with molten wax!",
          FALSE, ch, 0, victim, TO_NOTVICT);
      act ("You attempt to throw a pot of wax at $N...",
          FALSE, ch, 0, victim, TO_CHAR);
      act ("...and succeed, dousing $M with molten wax!",
          FALSE, ch, 0, victim, TO_CHAR);

    }
  }
  else
  {
      act ("$n nonchalantly pours a small pot of wax over $s entire body. Ouch.",
          FALSE, ch, 0, ch, TO_ROOM);
      act ("You smoothly pour the wax all over your body. Ouch.",
          FALSE, ch, 0, ch, TO_CHAR);
  }

  extract_obj( pot );
  waxify( victim );
  return TRUE;
}

// The pots of wax in Hakeem`s room are special: can`t pick them up.
int mus_hakeem_pot(OBJ *pot, CHAR *ch,int cmd,char *arg)
{
  if(cmd == MSG_GET && pot && ch && IS_MORTAL(ch))
  {
    act("$n attempts to pick up $p, but succeeds only in spilling molten wax on $mself.",
     FALSE,ch,pot,0,TO_ROOM);
    act("Your attempt to pick up $p spills molten wax all over yourself!",
     FALSE,ch,pot,0,TO_CHAR);
    damage(ch, ch, number(600,1000), TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
  }
  return FALSE;
}

// Goes away when not in El Museo zone. Can't rent while enchanted.
int ench_feeble(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  if(cmd==CMD_RENT || cmd==CMD_QUIT)
  {
    send_to_char("In your enfeebled state, to sleep may be perchance to die. Better not risk it.\r\n", ench_ch );
    return TRUE;
  }

  if( cmd == MSG_DIE  || cmd == MSG_AUTORENT || cmd == MSG_STONE || cmd == MSG_DEAD )
  {
    enchantment_remove( ench_ch, ench, 0 );
    return FALSE;
  }

  if(cmd==MSG_ENTER)
  {
    if( GET_ZONE(ench_ch) != 165 )
    {
      send_to_char("As the distance between you and Hakeem's wraith increases, his hold over\r\n you weakens.\r\n", ench_ch );
      enchantment_remove( ench_ch, ench, 0 );
    }
    return FALSE;
  }
  return FALSE;
}

void mus_enfeeble( CHAR *vict )
{
  if(GET_LEVEL(vict) >= LEVEL_IMM)
    return;
  int nat_hp = vict->specials.org_hit;
  int nat_mana = vict->specials.org_mana+100;
  int orig_hp = GET_MAX_HIT(vict);
  int orig_mana = GET_MAX_MANA(vict);
  int new_hp = -(2*orig_hp/3);
  int new_mana = -(2*orig_mana/3);
  vict->new.been_killed += 1;
  if(!IS_NPC(vict) && vict->ver3.death_limit) vict->ver3.death_limit++;
  death_list(vict);
  log_f("ELWAXO: Enfeebling %s, %d hp (was %d: nat %d), %d mana (was %d: nat %d)", vict->player.name, new_hp, orig_hp, nat_hp, new_mana, orig_mana, nat_mana );
  ENCH *tmp_enchantment;
  CREATE(tmp_enchantment, ENCH, 1);
  tmp_enchantment->name     = str_dup( ENCH_FEEBLE1 );
  tmp_enchantment->duration = 720;            /* Define the enchantment */
  tmp_enchantment->func     = ench_feeble;
  tmp_enchantment->modifier = new_hp;
  tmp_enchantment->location = APPLY_HIT;
  enchantment_to_char(vict, tmp_enchantment, FALSE);
  CREATE(tmp_enchantment, ENCH, 1);
  tmp_enchantment->name     = str_dup( ENCH_FEEBLE2 );
  tmp_enchantment->duration = 720;            /* Define the enchantment */
  tmp_enchantment->func     = ench_feeble;
  tmp_enchantment->modifier = new_mana;
  tmp_enchantment->location = APPLY_MANA;
  enchantment_to_char(vict, tmp_enchantment, FALSE);
  //Skeena 6/12/11: Enfeeblement should take effect immediately, not at tick
  GET_MANA (vict) = MIN (GET_MAX_MANA (vict), GET_MANA (vict));
  GET_HIT (vict) = MIN (GET_MAX_HIT (vict), GET_HIT (vict));
  send_to_char( "You feel weak.\r\n", vict );
}

void mus_ghost_curse( CHAR *ghost )
{
  act( " ", FALSE, ghost, 0, 0, TO_ROOM );
  act( "As the body hits the ground, $n materializes.", FALSE, ghost, 0, 0, TO_ROOM );
  act( "$n utters the words, 'ofzah mrz'", FALSE, ghost, 0, 0, TO_ROOM );

  CHAR *vict = NULL;
  for( vict = world[ghost->in_room_r].people; vict; vict = vict->next_in_room )
  {
    if( !IS_NPC( vict ) && !enchanted_by( vict, ENCH_FEEBLE1 ) )
    {
      mus_enfeeble( vict );
    }
  }
}

// Curses those who enter the room; disappears at zone reset
int mus_hakeem_ghost( CHAR *ghost, CHAR *ch, int cmd, char *arg )
{
  if( cmd == MSG_ENTER && ch && !IS_NPC(ch) && !enchanted_by(ch, ENCH_FEEBLE1 ) )
  {
    send_to_char( "The wraith of Hakeem utters the words 'mrz'\r\n", ch );
    mus_enfeeble(ch);
  }

  if( cmd == MSG_TICK && mob_proto_table[real_mobile(MUS_HAKEEM)].number > 0 )
  {
    extract_char( ghost );
    return FALSE;
  }

  if(cmd==CMD_KILL || cmd==CMD_HIT || cmd==CMD_KICK ||
     cmd==CMD_AMBUSH || cmd==CMD_ASSAULT || cmd==CMD_BACKSTAB) {
    act("$n says 'I no longer reside in the mortal plane.'",0,ghost,0,0,TO_ROOM);
    return TRUE;
  }

  CHAR *vict = NULL;
  if(cmd==MSG_MOBACT) {
    if(ghost->specials.fighting) {
      act("$n says 'I no longer reside in the mortal plane.'",0,ghost,0,0,TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(ghost)].people; vict; vict = vict->next_in_room)
        if(vict->specials.fighting==ghost) stop_fighting(vict->specials.fighting);
      stop_fighting(ghost);
      GET_HIT(ghost)=GET_MAX_HIT(ghost);
      return FALSE;
    }
  }

  return FALSE;
}

// Always blocks travel south.
// Every mobact will attempt to waxify someone.
// Then, 1/3rd of the time, will force all waxed players to do something to another player.
// Then, 1/4th of the time, will damage spec.
int
mus_hakeem (CHAR *hak, CHAR *ch, int cmd, char *arg) {
  if( !tried_to_find_skeena )
  {
    tried_to_find_skeena = TRUE;
    skeena_id = get_skeena_id();
  }

  CHAR *vict;

  hakeem_instance = hak;
  if (cmd == MSG_DIE || cmd == MSG_DEAD || cmd == MSG_CORPSE )
  {
    if (ch == hak)
    {
      hakeem_instance = NULL;
      hakeem_died = TRUE;
      int rmob = real_mobile(MUS_HAKEEM_GHOST);
      if (rmob > -1)
      {
         if (mob_proto_table[rmob].number > 0)
            return FALSE; /* we've already got one here */
      }
      CHAR *ghost = read_mobile( MUS_HAKEEM_GHOST, VIRTUAL );
      char_to_room(ghost, real_room(MUS_HAKEEM_ROOM));
      mus_ghost_curse( ghost );
      return FALSE;
    }
  }

  if (!hak)
    return FALSE;

  if( cmd == CMD_SOUTH )
  {
    do_say( hak, "You will need to defeat me first!", CMD_SAY );
    return TRUE;
  }

  if( cmd == CMD_CAST && ch)
  {
    int spl = extract_spell( arg );
    if (spl == SPELL_PASSDOOR && check_sc_access( ch, spl ))
    {
      do_say( hak, "You'll pass through my wall of wax over my dead body!", CMD_SAY );
      return TRUE;
    }
    return FALSE;
  }
  if(cmd || !AWAKE(hak) || !GET_OPPONENT(hak ) )
    return FALSE;

  if (!(vict=get_random_victim(hak)))
    return FALSE;

  if (vict == hak)
    return FALSE;

  if( hakeem_waxify( hak, vict ) )
    return FALSE;


  // Waxen PCs pick another PC and have a little fun at their expense.
  if(!number(0,2))
  {
    // Try to find a non-waxed person to target.
    CHAR *candidate = NULL;
    char buf[MAX_INPUT_LENGTH];
    bool found_waxed = FALSE;
    bool found_victim = FALSE;
    for( candidate = world[hak->in_room_r].people; candidate; candidate = candidate->next_in_room )
    {
      if( candidate == hak )
        continue;

      if( !found_waxed && enchanted_by( candidate, ENCH_WAXEN ) )
      {
        found_waxed = TRUE;
      }

      if( !found_victim && !enchanted_by( candidate, ENCH_WAXEN ) )
      {
        found_victim = TRUE;
        vict = candidate;
      }
    }

    if( found_waxed )
    {
      do_say( hak, "Waxen minions! Do as I command!", CMD_SAY );
      for( candidate = world[hak->in_room_r].people; candidate; candidate = candidate->next_in_room )
      {
        if( candidate == hak )
          continue;

        if( enchanted_by( candidate, ENCH_WAXEN ) )
        {
          if( GET_CLASS(candidate) == CLASS_MAGIC_USER || GET_CLASS(candidate) == CLASS_COMMANDO || GET_CLASS(candidate) == CLASS_ANTI_PALADIN )
          {
            sprintf(buf, "cast 'iceball' %s", vict->player.name );
            command_interpreter (candidate, buf);
          }
          else if( GET_CLASS(candidate) == CLASS_BARD )
          {
            if(number(0,1))
            {
              sprintf(buf, "song 'smoke'");
              command_interpreter (candidate, buf);
            }
            else
            {
              sprintf(buf, "song 'a view to'" );
              command_interpreter (candidate, buf);
            }
          }
          else if( GET_CLASS( candidate) == CLASS_NINJA || GET_CLASS(candidate) == CLASS_PALADIN )
          {
            sprintf(buf, "pummel %s", vict->player.name );
            command_interpreter( candidate, buf );
          }
          else if( GET_CLASS( candidate ) == CLASS_THIEF )
          {
            sprintf(buf, "circle %s", vict->player.name );
            command_interpreter( candidate, buf );
          }
          else if( GET_CLASS( candidate ) == CLASS_CLERIC )
          {
            sprintf(buf, "cast 'searing orb'" );
            command_interpreter( candidate, buf );
          }
          else
          {
            sprintf(buf, "spin");
            command_interpreter( candidate, buf );
          }
        }
      }

      return FALSE;
    }
  }

  if (number(0,3))
    return FALSE;


  act ("$n says 'I am the Master of the Universe, Hakeem!'", FALSE, hak, 0,
       vict, TO_ROOM);
  act ("You feel your life force sucked away as $n's hand reaches out for you.",
       FALSE, hak, 0, vict, TO_VICT);
  act ("Sparks fly from $n's clawed hand to $N!",
       FALSE, hak, 0, vict, TO_NOTVICT);
  act ("You use your mental energy to drain $N's life force away.",
       FALSE, hak, 0, vict, TO_CHAR);
  damage (hak, vict, 400, TYPE_UNDEFINED,DAM_NO_BLOCK);

  return FALSE;

}

/*
Fire/ice djinns block movement and can only be damaged by fire/ice damage spells respectively
*/
int mus_djinn(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH],thing[MAX_INPUT_LENGTH];
  int block;

  if( !mob ) return FALSE;

  int unshielded_spell3 = 9999;
  int unshielded_spell2 = 9999;
  int unshielded_spell = 9999;
  if( V_MOB(mob)==MUS_VAPOR )
  {
    unshielded_spell = SPELL_ICEBALL;
    unshielded_spell2 = SPELL_FROSTBOLT;
    unshielded_spell3 = SPELL_RIMEFANG;
  }

  if( V_MOB(mob)==MUS_FIRE )
  {
    unshielded_spell = SPELL_FIREBALL;
    unshielded_spell2 = SPELL_FLAMESTRIKE;
    unshielded_spell3 = SPELL_HELL_FIRE;
  }

  if(cmd==CMD_THROW) {
    if(GET_CLASS(ch) == CLASS_CLERIC || GET_CLASS(ch) == CLASS_PALADIN || GET_CLASS(ch) == CLASS_ANTI_PALADIN || GET_CLASS(ch) == CLASS_MAGIC_USER) return FALSE;

    act("You reach down to lob something at $n, but a magical force stops you.", 0, mob, 0, ch,TO_VICT);
    return TRUE;
  }

  // Skeena 6/12/11: can't use boots of iron to kill Djinns
  if( cmd == CMD_KICK && ch && (is_wearing_obj(ch, HELL_IRON_BOOTS, WEAR_FEET) || is_wearing_obj(ch, HELL_IRON_BOOTS_B, WEAR_FEET) )) {
    log_f("ELWAXO: player %s's Boots of Iron destroyed at Djinn", ch->player.name );
    extract_obj(unequip_char(ch, WEAR_FEET));
    if(V_MOB(mob)==MUS_FIRE) {
      act("As your Boots of Iron touch $n, they turn into molten slag and fall off, taking most of your feet with them!",0,mob,0,ch,TO_VICT);
      act("As $N's Boots of Iron touch $n, they turn into molten slag and fall off, taking most of $S feet with them!",0,mob,0,ch,TO_NOTVICT);
      act("$N's Boots of Iron turn into molten slag when they touch you.",0,mob,0,ch,TO_CHAR);
      damage(mob,ch,number(120,360),TYPE_UNDEFINED,DAM_NO_BLOCK);
    } else {
      act("As your Boots of Iron touch $n, they freeze and shatter, taking most of your feet with them!",0,mob,0,ch,TO_VICT);
      act("As $N's Boots of Iron touch $n, they freeze and shatter, taking most of $S feet with them!",0,mob,0,ch,TO_NOTVICT);
      act("$N's Boots of Iron freeze and shatter when they touch you.",0,mob,0,ch,TO_CHAR);
      damage(mob,ch,number(120,360),TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return TRUE;
  }

  //Skeena 6/21/11: can't throw things at them

  //Skeena 6/12/11: Fire djinn can be hurt by fire breath
  if( cmd == CMD_FIREBREATH ) {
    if(V_MOB(mob)==MUS_FIRE) {
      REMOVE_BIT( mob->specials.act, ACT_SHIELD );
    } else {
      SET_BIT(mob->specials.act, ACT_SHIELD);
    }
  }

  //Skeena 6/12/11: Lethal fire song works on fire djinn
  if( cmd == CMD_SONG ) {
    if( extract_song(arg) == 15 && V_MOB(mob) == MUS_FIRE ) {
      REMOVE_BIT( mob->specials.act, ACT_SHIELD );
    } else {
      SET_BIT(mob->specials.act, ACT_SHIELD);
    }
  }

  if( cmd == CMD_CAST )
  {
    int spell = extract_spell( arg );
    SET_BIT( mob->specials.act, ACT_SHIELD );
    if( spell == unshielded_spell || spell == unshielded_spell2 || spell == unshielded_spell3 )
      REMOVE_BIT( mob->specials.act, ACT_SHIELD );
    return FALSE;
  }

  if(V_MOB(mob)==MUS_VAPOR) block=CMD_NORTH;
  else block=CMD_WEST;

  if(cmd!=block) return FALSE;
  if(!ch) return FALSE;

  if(V_MOB(mob)==MUS_VAPOR) sprintf(thing,"piece of frozen air");
  else sprintf(thing,"bolt of fire");

  sprintf(buf,"$n stops you in your tracks and throws a %s at you.",thing);
  act(buf,0,mob,0,ch,TO_VICT);
  sprintf(buf,"$n stops $N in $S tracks and throws a %s at $M.",thing);
  act(buf,0,mob,0,ch,TO_NOTVICT);
  sprintf(buf,"You stop $N in $S tracks and throw a %s at $M.",thing);
  act(buf,0,mob,0,ch,TO_CHAR);
  damage(mob,ch,number(60,120),TYPE_UNDEFINED,DAM_NO_BLOCK);
  return TRUE;
}

/*16569 (the Waxen Curator)
   specs: #1 and #2, blocks movement east and west (Listlessly
raises an arm to prevent you from moving <dir>)*/

enum CURATE_STATE {
  CURATE_NAME = 0,
  CURATE_BOSS,
  CURATE_HELP,
  CURATE_LAST,
  CURATE_FIGHTING
} ;

char* curator_hints_name[] = {
  "If only I could escape my waxen captivity.",
  "Would you help me?",
  "A true friend would call me by name."
  };

char* curator_hints_boss[] = {
  "My master has chained me here for eternity... please, end his cruel reign!",
  "He is a vile man--you must recognize him!",
  "My master's existence is a blight against all mankind! Watch for him!"
  };

char* curator_hints_help[] = {
  "Killing him is only the beginning. To truly end his reign, you must break through...",
  "...but I fear I have already said too much, and will feel his wrath.",
  "I see no sturdy adventurers among you. There is something more you will need."
  };

char* curator_hints_last[] = {
  "There is nothing more I can give you.",
  "I await my release.",
  "Please, end my suffering."
  };

char* curator_passwords_name[] = {
"whatdowecallyou",
"howdowecallyou",
"whatshouldwecallyou",
"howshouldwecallyou",
"whatcanwecallyou",
"howcanwecallyou",
"whatdoweaddressyou",
"howdoweaddressyou",
"whatshouldweaddressyou",
"howshouldweaddressyou",
"whatcanweaddressyou",
"howcanweaddressyou",
"whatdoyouliketobecalled",
"howdoyouliketobecalled",
"whatdoyouliketobeaddressed",
"howdoyouliketobeaddressed",
"whatdoicallyou",
"howdoicallyou",
"whatshouldicallyou",
"howshouldicallyou",
"whatcanicallyou",
"howcanicallyou",
"whatdoiaddressyou",
"howdoiaddressyou",
"whatshouldiaddressyou",
"howshouldiaddressyou",
"whatcaniaddressyou",
"howcaniaddressyou",

};

char* curator_passwords_boss[] =
{
"whatisyourbosscalled","whatisyoursupervisorcalled","whatisyourleadercalled","whatisyoursuperiorcalled","howisyourbosscalled",
"howisyoursupervisorcalled","howisyourleadercalled","howisyoursuperiorcalled","whatisyourbossaddressed","whatisyoursupervisoraddressed",
"whatisyourleaderaddressed","whatisyoursuperioraddressed","howisyourbossaddressed","howisyoursupervisoraddressed","howisyourleaderaddressed",
"howisyoursuperioraddressed","howshouldwecallyourboss","howshouldwecallyoursupervisor","howshouldwecallyourleader","howshouldwecallyoursuperior",
"howshouldweaddressyourboss","howshouldweaddressyoursupervisor","howshouldweaddressyourleader","howshouldweaddressyoursuperior","howshouldweknowyourboss",
"howshouldweknowyoursupervisor","howshouldweknowyourleader","howshouldweknowyoursuperior","howdowecallyourboss","howdowecallyoursupervisor",
"howdowecallyourleader","howdowecallyoursuperior","howdoweaddressyourboss","howdoweaddressyoursupervisor","howdoweaddressyourleader",
"howdoweaddressyoursuperior","howdoweknowyourboss","howdoweknowyoursupervisor","howdoweknowyourleader","howdoweknowyoursuperior",
"howwillweknowyourboss","howwillweknowyoursupervisor","howwillweknowyourleader","howwillweknowyoursuperior","howshouldicallyourboss",
"howshouldicallyoursupervisor","howshouldicallyourleader","howshouldicallyoursuperior","howshouldiaddressyourboss","howshouldiaddressyoursupervisor",
"howshouldiaddressyourleader","howshouldiaddressyoursuperior","howshouldiknowyourboss","howshouldiknowyoursupervisor","howshouldiknowyourleader",
"howshouldiknowyoursuperior","howdoicallyourboss","howdoicallyoursupervisor","howdoicallyourleader","howdoicallyoursuperior",
"howdoiaddressyourboss","howdoiaddressyoursupervisor","howdoiaddressyourleader","howdoiaddressyoursuperior","howdoiknowyourboss",
"howdoiknowyoursupervisor","howdoiknowyourleader","howdoiknowyoursuperior","howwilliknowyourboss","howwilliknowyoursupervisor",
"howwilliknowyourleader","howwilliknowyoursuperior", "whatshouldicallyourboss", "whatshouldicallyourleader", "whatshouldwecallyourleader",
"whatshouldwecallyourboss", "howwillwerecognizehim", "howwillirecognizehim", "howwilliknowhim", "howwillweknowhim"
};

char* curator_passwords_help[] =
{
"whatdowehavetobreak", "whatdowehavetobreakthrough", "whatdoweneedtobreak", "whatdoweneedtobreakthrough",
"whatdoihavetobreak", "whatdoihavetobreakthrough", "whatdoineedtobreak", "whatdoineedtobreakthrough",
"whatmustwebreak", "whatmustwebreakthrough", "whatmustibreak", "whatmustibreakthrough"
};

#define NUM_OF(x) (sizeof(x)/sizeof*(x))
char** curator_state_passwords[] = {curator_passwords_name, curator_passwords_boss, curator_passwords_help};
int curator_state_password_lengths[] = {NUM_OF(curator_passwords_name), NUM_OF(curator_passwords_boss),  NUM_OF(curator_passwords_help) };

char** curator_state_hints[] = {curator_hints_name, curator_hints_boss, curator_hints_help, curator_hints_last };
char * curator_responses[] = {"You may call me Werner.", "You will know him when he introduces himself as Hakeem, the master of the universe!", "An imposing wall...you will know it when you see it!  Take this, it may come in handy." };


unsigned long mus_get_ip( CHAR *ch )
{
  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next)
    if (d->character == ch )
      return d->addr;

  return 0;
}

// Returns true if there are characters with frmo at least 2 different IPs in the room
bool room_has_multiple_multis( CHAR *ch )
{
  CHAR *vict;
  unsigned long ip = 0;

  for(vict = world[ch->in_room_r].people; vict; vict = vict->next_in_room)
  {
    if( !IS_NPC( vict ) )
    {
      if( ip == 0 )
        ip = mus_get_ip( vict );
      else if( ip != 0 && ip != mus_get_ip( vict ) )
         return TRUE;
    }
  }

  return FALSE;
}

// Lowercases, removes non-alpha characters and removes the substrings "please" and "werner" from src
// Dest must be a preallocated string of length MAX_INPUT_LENGTH
void normalize_string(char *src, char*dest)
{
  char *orig_dest = dest;

  while( *src != 0 )
  {
    if( isalpha(*src ) )
    {
      *dest = tolower(*src);
      dest++;
    }
    src++;
  }

  while( (dest=strstr(orig_dest, "please" )) || (dest=strstr(orig_dest, "werner" )) )
  {
    while(dest != orig_dest + (MAX_INPUT_LENGTH - 7) )
    {
      *dest = *(dest+6);
      dest++;
    }
  }
}

enum CURATE_STATE curate_state = CURATE_NAME;
unsigned long last_ip = 0;
int mus_curator(CHAR *mob,CHAR *ch,int cmd,char *arg) {

  if(!mob) return FALSE;

  // Give a random hint.
  if(cmd == MSG_MOBACT )
  {
    if( mob->specials.fighting )
    {
      if( curate_state != CURATE_FIGHTING )
      {
        curate_state = CURATE_FIGHTING;
        do_say( mob, "Death! I welcome it!", CMD_SAY );
        return FALSE;
      }
    }
    else if( curate_state != CURATE_FIGHTING && chance(50) )
    {
      do_say( mob, curator_state_hints[curate_state][number(0,2)], MSG_RECONNECT );
      return FALSE;
    }
  }


  if( !ch )
    return FALSE;

  if(cmd==CMD_EAST) {
    act("$n listlessly raises an arm to prevent you from moving east.",0,mob,0,ch,TO_VICT);
    act("$n listlessly raises an arm to prevent $N from moving east.",0,mob,0,ch,TO_NOTVICT);
    act("You listlessly raise an arm to prevent $N from moving east.",0,mob,0,ch,TO_CHAR);
    return TRUE;
  }
  if(cmd==CMD_WEST) {
    act("$n listlessly raises an arm to prevent you from moving west.",0,mob,0,ch,TO_VICT);
    act("$n listlessly raises an arm to prevent $N from moving west.",0,mob,0,ch,TO_NOTVICT);
    act("You listlessly raise an arm to prevent $N from moving west.",0,mob,0,ch,TO_CHAR);
    return TRUE;
  }

  if(cmd == MSG_ENTER )
  {
    send_to_char( "\r\nThe Waxen Curator sighs heavily.\r\n", ch );
    curate_state = CURATE_NAME;
    return FALSE;
  }

  if(cmd == MSG_SAID)
  {
    if( ch == mob ) return FALSE;

    // Enforce arbitrary rules.
    // Rule 1: players must take turns speaking to Werner
    if( last_ip == mus_get_ip( ch ) && GET_LEVEL( ch ) < LEVEL_IMM )
    {
      if( room_has_multiple_multis( ch ) )
      {
        if( number(0,1) )
        {
          do_say( mob, "Stop hogging the conversation.", MSG_RECONNECT );
        }
        else
        {
          do_say( mob, "Maybe someone else would like to chat, too.", MSG_RECONNECT );
        }
      }
      else
      {
        do_say( mob, "Oh, dear... are there more people coming to help?", MSG_RECONNECT );
        act("$n peers around, looking as if $e has trouble seeing everything clearly..",0,mob,0,0,TO_ROOM);
      }
      return FALSE;
    }
    last_ip = mus_get_ip( ch );

    // Rule 2: Players must never use these forbidden words.
    if( strstr( arg, "name" ) || strstr( arg, "location" ) || strstr( arg, "where" )
        || strstr(arg, "who" ) || strstr(arg, "master" ) )
    {
      if( GET_SEX(ch) == SEX_FEMALE )
      {
        do_say(mob, "Bah! Such a dirty mouth for such a pretty girl!", MSG_RECONNECT );
      }
      else
      {
        do_say(mob, "Bah! Such foul language! Were you raised in a barn?", MSG_RECONNECT );
      }

      return FALSE;
    }

    // Rule 2b: Or these forbidden words.
    if( strstr( arg, "curator" ) || strstr(arg, "Curator" ) )
    {
      do_say(mob, "I did not choose this job...and I don't like to be reminded of it!", MSG_RECONNECT );
      return FALSE;
    }

    // Rule 3: Once they've learned his name, they must say please
    if( curate_state >= CURATE_BOSS )
    {
      if( !strstr( arg, "please" ) )
      {
        do_say(mob, "I don't talk with rude people.", MSG_RECONNECT );

        return FALSE;
      }

      // Rule 4: Once they've learned his boss's name, they must address Werner by name
      if( curate_state >= CURATE_HELP )
      {
        if( !strstr(arg, "Werner" ) && !strstr(arg, "werner" ) )
        {
          do_say( mob, "Sorry, was that addressed to me?", MSG_RECONNECT );

          return FALSE;
        }
      }
    }

    // If we got here,  they've passed the arbitrary rules and we can advance them if they say the right things
    // I'm not masochistic enough to learn the GNU regex lib for this, so we're doing a half-baked string comparison
    char normal[MAX_INPUT_LENGTH];
    memset(normal,0,MAX_INPUT_LENGTH);
    normalize_string(arg, normal);
    if( curate_state >= CURATE_NAME && curate_state <= CURATE_HELP )
    {
      int i = 0;
      for( i = 0; i < curator_state_password_lengths[curate_state]; i++ )
      {
        if( !strcmp( normal, curator_state_passwords[curate_state][i] ) )
        {
          do_say( mob, curator_responses[curate_state], MSG_RECONNECT) ;
          curate_state++;

          if( curate_state == CURATE_LAST )
          {
            struct obj_data *wax_ram = read_object( MUS_BATTERING_RAM, VIRTUAL );
            obj_to_char( wax_ram, mob );
            mob_do( mob, "drop ram" );
          }
          break;
        }
      }
    }
    return FALSE;
  }


  // If they want to get past Hakeem and don't have a level 50 warrior, they can talk
  // a batterin ram out of Werner, but, uh, it's involved.
  //
  // Example dialogue end to end:
  //
  // hint text:
  // WERNER: sigh
  // WERNER: If only I could escape my waxen captivity.
  // WERNER: Hello, %s. Would you help me?
  // WERNER: A true friend would call me by name.
  // response to "curator" or "waxen curator"
  //    WERNER: A job is not a name!
  //    WERNER: I didn`t choose this job... and I don`t like to be reminded of it!
  //
  //   "(what|how) (do|should|can) we (address|call) you?"
  //   "(what|how|) do you like to be (called|addressed)?"
  //
  //  WERNER: Thank you, you may call me Werner.
  //
  //  "What is your name?"
  //  WERNER: Such foul language, were you raised in a barn?!
  //
  //  WERNER: My master has chained me here for eternity... please, end his cruel reign!
  //  WERNER: He is a vile creature, my master.
  //
  //  "please, (what|how) is your (superior|supervisor) (called|addressed)"
  //  "please, how (do|should) we (call|address|know) your (supervisor|superior)"
  //
  //  WERNER: You will know him when he introduces himself as Hakeem, the master of the universe!
  //  WERNER: Centuries of playing god have made him a bit full of himself.
  //
  //  WERNER: Killing him is only the beginning.  To truly end his reign, you must break through...
  //  WERNER: ...but I fear I have already said too much.
  //  WERNER: But... alas, I see no sturdy adventurers among you.  There is something more you will need.
  //
  //  "what do we have to (break through|breakthrough|break)"
  //
  // WERNER: You will know it when you see it.
  // WERNER reveals a waxen battering ram!
  //
  // WERNER: Release me.
  //
  return FALSE;
}

// Drinking Khallaq`s waste water is hazardous to your health: poison and blindness may result.
void drink_the_koolaid( CHAR* ch )
{
  int affect = number(0,4);

  if( affect == 0 )
  {
    spell_blindness(50, ch, ch, 0 );
  }
  else if( affect == 1 )
  {
    spell_poison(50, ch, ch, 0 );
  }
}

// When the wall in Hakeem's room is broken down, free anyone who is stuck in wax and trans
// them to Hakeem's room.
void free_encased_in_wax()
{
  int real = real_room( MUS_ENCASED_IN_WAX );

  CHAR *vict;
  CHAR *next_in_room;

  if( world[real].people )
  {
    send_to_room( "The destruction of the wall frees your trapped comrades!\r\n", real_room(MUS_HAKEEM_ROOM) );
  }

  for(vict = world[real].people; vict; vict = next_in_room )
  {
    next_in_room = vict->next_in_room;

    act( "The wall is destroyed with brute force!\r\n", FALSE, vict, 0, 0, TO_CHAR );
    if( GET_LEVEL(vict) < LEVEL_IMM )
    {
      GET_HIT(vict) = -1;
      GET_POS(vict) = POSITION_INCAP;
    }
    char_from_room(vict);
    char_to_room(vict,real_room(MUS_HAKEEM_ROOM));
    do_look(vict,"",CMD_LOOK);
  }
}

// Only level 50 warriors can knock down the wall.
int mus_hakeem_room(int room, CHAR *ch, int cmd, char *arg)
{
  if( ch && cmd == CMD_KNOCK && GET_CLASS( ch ) == CLASS_WARRIOR )
  {
    char buf[MAX_INPUT_LENGTH];
    arg = one_argument( arg, buf );

    if( buf[0] == 0 )
      return FALSE;

    if( !strcasecmp(buf, "wall" ) )
    {
      if( !IS_SET(EXIT(ch, SOUTH)->exit_info, EX_LOCKED))
      {
        send_to_char( "There is nothing to knock there.\r\n", ch );
        return TRUE;
      }

      if( GET_LEVEL( ch ) < 50 )
      {
        act("You run headlong into the wall, but only succeed in giving yourself a concussion.",
             FALSE, ch, 0, 0, TO_CHAR);
        act("$n runs headlong into the wall. Ouch.",
            FALSE, ch, 0, 0, TO_ROOM );
        damage (ch, ch, 300, TYPE_UNDEFINED,DAM_NO_BLOCK);
        WAIT_STATE(ch, 2*PULSE_VIOLENCE);
      }
      else
      {
        act("You strain against the wall until it crumbles in a shower of waxen fragments!",
             FALSE, ch, 0, 0, TO_CHAR);
        act("$n strains against the wall until it crumbles in a shower of waxen fragments!",
            FALSE, ch, 0, 0, TO_ROOM );
        WAIT_STATE(ch, 3*PULSE_VIOLENCE);

        REMOVE_BIT(EXIT(ch, SOUTH)->exit_info, EX_LOCKED);
        REMOVE_BIT(EXIT(ch, SOUTH)->exit_info, EX_CLOSED);
        free_encased_in_wax();
      }

      return TRUE;
    }

    return FALSE;
  }
  else if( cmd == MSG_ENTER && ch && hakeem_instance )
  {
    send_to_char( "Hakeem the Sorcerer drains you of your magical abilities.\r\n", ch );
    if( is_caster(ch ) )
    {
      int current_mana = GET_MANA(ch);
      int new_mana = GET_MAX_MANA(ch) / 10;

      GET_MANA(ch)= (current_mana < new_mana ? current_mana : new_mana);
    }
    else
    {
      // What the hell is a non-caster doing with mana anyway? Disgraceful.
      GET_MANA(ch) = 0;
    }
  }
  else if( cmd == CMD_CAST && ch)
  {
    int spl = extract_spell( arg );
    if (!hakeem_instance && spl == SPELL_PASSDOOR && check_sc_access( ch, spl ))
    {
      act( "You get distracted while casting.", FALSE, ch, 0, 0, TO_CHAR );
      act( "$n utters the words 'saggeaff'", FALSE, ch, 0, 0, TO_ROOM );
      act( "$n hurriedly tries to phase $s body through the wall.", FALSE, ch, 0, 0, TO_ROOM );

      char_from_room(ch);
      char_to_room(ch,real_room(MUS_ENCASED_IN_WAX));
      do_look(ch,"",CMD_LOOK);
      act("$n appears in the middle of the room.", FALSE, ch, 0, 0, TO_ROOM);

      return TRUE;
    }
  }
  return FALSE;
}

// Only ninjas (and non junk ones) can swim down from Khallaq`s room to get corpses/render assistance.
int mus_red_mist(int room, CHAR *ch, int cmd, char *arg)
{
  if( ch && cmd == CMD_DOWN && GET_CLASS(ch) != CLASS_NINJA )
  {
    act("You attempt to dive into the depths of the pool, but your swimming abilities are not up to the task.",
         FALSE, ch, 0, 0, TO_CHAR);
    act("$n attempts to dive into the depths of the pool, but $s swimming abilities are not up to the task.",
        FALSE, ch, 0, 0, TO_ROOM );
    return TRUE;
  }
  else if( ch && cmd == CMD_DOWN && GET_CLASS( ch ) == CLASS_NINJA && GET_LEVEL( ch ) < 35 )
  {
    // The penalty for junk ninjas is them + a real player.
    CHAR *vict = get_random_victim( ch );
    if( vict && !IS_NPC( vict ) && vict == ch )
    {
      act("You attempt to dive into the depths of the pool, but your swimming abilities are not up to the task.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n attempt to dive into the depths of the pool, but $s swimming abilities are not up to the task.",
          FALSE, ch, 0, 0, TO_ROOM );
    }
    else if( vict && vict != ch && !IS_NPC(vict) )
    {
      act ("$n dives into the water, but surfaces in a panic and drags you down with $m!",
          FALSE, ch, 0, vict, TO_VICT);
      act ("$n dives into the water, but surfaces in a panic and drags $N into the depths!",
          FALSE, ch, 0, vict, TO_NOTVICT);
      act ("You panic in the murky water and grab onto the nearest person -- $N!",
          FALSE, ch, 0, vict, TO_CHAR);

      char_from_room(ch);
      char_to_room(ch,real_room(MUS_DROWNING));
      do_look(ch,"",CMD_LOOK);
      act("$n appears in the middle of the room.", FALSE, ch, 0, 0, TO_ROOM);

      char_from_room(vict);
      char_to_room(vict,real_room(MUS_DROWNING));
      do_look(vict,"",CMD_LOOK);
      act("$n appears in the middle of the room.", FALSE, vict, 0, 0, TO_ROOM);
    }
    else
    {
      // No-one else around, insta kill.
      send_to_char( "You flail wildly in the water until exhaustion sets in...\r\n", ch);
      send_to_char( "...as the water flows up over your face, your only thought\r\n", ch);
      send_to_char( "   is that you wish you were a better swimmer.\r\n", ch);
      damage(ch, ch, 10000, TYPE_UNDEFINED, DAM_NO_BLOCK );
    }
    return TRUE;
  }
  else if( ch && cmd == CMD_DOWN && GET_CLASS( ch ) == CLASS_NINJA && GET_LEVEL( ch ) < 45 )
  {
    // The penalty for level 35-45 ninjas is damage.
    if( chance(66) )
    {
      act("You have misjudged your swimming capabilities.",
           FALSE, ch, 0, 0, TO_CHAR);
      act("$n foolishly dives into the depths of the pool, forgetting that $s swimming abilities aren't really all that good.",
          FALSE, ch, 0, 0, TO_ROOM );
      damage (ch, ch, number(1,2000), TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
    }
  }

  return FALSE;
}

// Purge any NPCs that end up in here
int mus_sanctuary(int room, CHAR *ch, int cmd, char *arg) {
  if(cmd != MSG_MOBACT) return FALSE;
  CHAR *vict = NULL;
  CHAR *next_vict = NULL;
  for (vict = world[room].people; vict; vict = next_vict) {
    next_vict = vict->next_in_room;
    if(IS_NPC(vict))
      extract_char(vict);
  }
  return FALSE;
}

// Maintenance specs for catching when waxed gear has been rented
int mus_encased_in_wax(int room, CHAR *ch, int cmd, char *arg)
{
  if( cmd == MSG_ZONE_RESET )
  {
    if( skeena_id == -1 )
      skeena_id = get_skeena_id();
  }

  if( cmd == MSG_TICK )
  {
    if( skeena_id == -1 ) return FALSE;

    //Check all logged in players for equipped gear that has ownerid == Skeena
    struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next)
      if (d->character)
      {
        int i;
        for(i = 0; i <= HOLD; i++)
        {
          OBJ* obj = EQ(d->character, i);
          if( obj && obj->ownerid[7] == skeena_id && obj->func == NULL )
          {
            log_f("ELWAXO: orphaned wax item fixed: player %s had %s (%d)", d->character->player.name, OBJ_SHORT(obj), obj->item_number_v );
            int j;
            for(j = 0; j < MAX_OBJ_AFFECT; j++ )
              if(!( obj->affected[j].location >= 25 && obj->affected[j].location <= 51 )) // don't affect SKILL_ slots
                obj->affected[j].modifier *= -1;

            obj->ownerid[7] = 0;
          }
        }
      }
  }

  return FALSE;
}

// Sanity check we`re in an arena room, and then encourage people to be true to their
// alignments -- assist/rescue does not work for people of opposing aligns
int mus_arena_rooms(int room, CHAR *ch, int cmd, char *arg)
{
  if( !ch )
    goto chain;

  if( !IS_SET( world[room].room_flags, ARENA ) )
    goto chain;

  if( is_wearing_obj(ch,MAR_QUEEN_STING,WIELD) && cmd == CMD_BACKSTAB && (GET_CLASS(ch) == CLASS_ANTI_PALADIN || GET_CLASS(ch) == CLASS_THIEF)) {
    act("$n grasps $s weapon, but it refuses to serve $s vile intentions!", FALSE,ch,0,0,TO_ROOM);
    act("Your weapon trembles in your grasp, refusing to submit to your vile intentions!", FALSE,ch,0,0,TO_CHAR);
    WAIT_STATE(ch, 3*PULSE_VIOLENCE);
    return TRUE;
  }

  struct char_data *victim;
  // If not in room, default behaviour.
  char victim_name[240];
  victim_name[0] = 0;

  if( cmd == CMD_ASSIST )
  {
    if( arg )
      strcpy(victim_name, arg + 1 ); // arg has a leading space, this removes it

    if (!(victim = get_char_room_vis(ch, victim_name)))
      goto chain;

    // If self, default behaviour.
    if( victim == ch )
      goto chain;

    // If alignment of either party is neutral, or if alignment matches, default behaviour.
    if( IS_NEUTRAL( ch ) || IS_NEUTRAL( victim ) || (IS_GOOD( victim ) && IS_GOOD( ch ) )
        || (IS_EVIL(victim) && IS_EVIL(ch ) ) )
      goto chain;

    // Oh, dear.
    act ("Your base urges overtake you and you lunge at $N!",
         FALSE, ch, 0, victim, TO_CHAR);
    act ("$n gets a funny look in $s eye and lunges at $N unexpectedly!",
         FALSE, ch, 0, victim, TO_NOTVICT);
    act ("$n gets a funny look in $s eye and lunges straight at you!",
         FALSE, ch, 0, victim, TO_VICT);

    char kill_command[MAX_INPUT_LENGTH];
    sprintf(kill_command, "kill %s", victim_name );
    command_interpreter( ch, kill_command );
    return TRUE;
  }
  else if( cmd == CMD_RESCUE )
  {
    if( arg )
      strcpy(victim_name, arg + 1 ); // arg has a leading space, this removes it

    if (!(victim = get_char_room_vis(ch, victim_name)))
      goto chain;

    // If self, default behaviour.
    if( victim == ch )
      goto chain;

    // If alignment of either party is neutral, or if alignment matches, default behaviour.
    if( IS_NEUTRAL( ch ) || IS_NEUTRAL( victim ) || (IS_GOOD( victim ) && IS_GOOD( ch ) )
        || (IS_EVIL(victim) && IS_EVIL(ch ) ) )
      goto chain;

   send_to_char("You cannot stand the thought of rendering them aid!\r\n", ch);
   return TRUE;
  }

chain:
  // Seena 6/12/11: Mat likes lame speedwalk hit and run tactics.
  if( chance(10) && ch && GET_LEVEL(ch) < LEVEL_IMM) {
    if( cmd == CMD_EAST || cmd == CMD_WEST || cmd == CMD_NORTH || cmd == CMD_SOUTH ) {
      // If following your leader, it shouldn't happen
      if( EXIT(ch, cmd-1) && (!ch->master || (EXIT(ch, cmd-1)->to_room_r != CHAR_REAL_ROOM(ch->master))) ) {
        act("You grow momentarily faint and pause to regain your bearings.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n grows momentarily faint and pauses to regain $s bearings.", FALSE, ch, 0, 0, TO_ROOM);
        return TRUE;
      }
    }
  }
  // If we`re in 16531 and you look down, and no-one else is in gembal snake room,
  // you will be teleported there.
  if( world[room].number == MUS_UP_FROM_GEMBAL_SNAKE )
  {
    if( count_mortals_real_room( real_room( MUS_GEMBAL_SNAKE_ROOM ) ) == 0 && cmd == CMD_LOOK && arg && (!strcasecmp(arg, " down" ) || !strcasecmp(arg, " d")))
    {
      act ("You feel yourself pulled downwards into the wax.",
           FALSE, ch, 0, 0, TO_CHAR);
      act ("As $n looks at the wax patterns on the floor, $e becomes entranced and is sucked into them.",
           FALSE, ch, 0, 0, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch,real_room(MUS_GEMBAL_SNAKE_ROOM));
      do_look(ch,"",CMD_LOOK);
      act("$n appears in the middle of the room.", FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
    }
    return FALSE;
  }

  // If you`re entering Lem`s room and he is alive, you will be disenchanted.
  if( world[room].number == MUS_LEM_ROOM )
  {
    if( cmd == MSG_ENTER  && lem_instance )
    {
      if( ch && !IS_NPC( ch ) )
      {
        send_to_char( "\r\nAn oppressive presence suffocates your magic.\r\n", ch );
        spell_disenchant( 50, lem_instance, ch, 0 );
      }
    }

    return FALSE;
  }
  if( world[room].number == MUS_LIVING_ROOM )
  {
    if( cmd == CMD_EAST )
    {
      if( IS_SET(EXIT(ch, EAST)->exit_info, EX_CLOSED)) return FALSE;

      send_to_char("The door is too small for you.\r\n", ch );
      act("$n contemplates the small door to the east.", FALSE, ch, 0, 0, TO_ROOM );
      return TRUE;
    }
  }

  if( world[room].number == MUS_RED_MIST )
  {
    return mus_red_mist( room, ch, cmd, arg );
  }

  return FALSE;
}

int gorgo_door_state = 0; // 0: closed, 1: open and sent message
// Hallway north of Gorgo: after you open the door, you'll hear noises from Gorgo
int mus_north_from_gorgo( int room, CHAR *ch, int cmd, char *arg )
{
  if( cmd == MSG_MOBACT )
  {
    unsigned int door_status = world[room].dir_option[SOUTH]->exit_info;

    if( IS_SET( door_status, EX_CLOSED ) )
    {
      gorgo_door_state = 0;
    }
    else if( gorgo_door_state == 0 && mob_proto_table[real_mobile(MUS_GORGO)].number>0)
    {
      gorgo_door_state = 1;
      send_to_room("You hear a deep snuffling noise from the south.\r\n", room );
    }
  }
  return FALSE;
}

int hakeem_door_state = 0; // 0: closed, 1: open and sent message
// Hallway up from Hakeem: Hakeem sends a geyser of hot wax up, spraying people in room
// with probability related to how many players there are
int mus_up_from_hakeem( int room, CHAR *ch, int cmd, char *arg)
{
  if( cmd == MSG_MOBACT )
  {
    unsigned int door_status = world[room].dir_option[DOWN]->exit_info;

    if( IS_SET( door_status, EX_CLOSED ) )
    {
      hakeem_door_state = 0;
    }
    else if( hakeem_instance && hakeem_door_state == 0 )
    {
      hakeem_door_state = 1;
      // odds of being ganked with wax rise in proportion to how many players are in room
      // presumably it's easier to dodge if there's less people => more room
      int chance_to_hit = count_mortals_real_room( room );
      chance_to_hit = (chance_to_hit * 100) / (chance_to_hit + 1);
      do_shout( hakeem_instance, "Stay back, pitiful fools, or accept your death with grace!", 0 );
      send_to_room("A geyser of molten wax explodes upwards from the trapdoor!\r\n", room );
      CHAR *vict = NULL;
      CHAR *next_vict = NULL;
      for(vict = world[room].people; vict; vict = next_vict)
      {
				next_vict = vict->next_in_room;
        if( chance(chance_to_hit) )
        {
          act ("You are hit with hot wax!",
            FALSE, vict, 0, 0, TO_CHAR);
          act ("$n is hit with hot wax!",
            FALSE, vict, 0, 0, TO_ROOM);
          damage(vict, vict, number(300,500), TYPE_UNDEFINED,DAM_NO_BLOCK);
        }
      }
    }
  }
  return FALSE;
}


// Hallway north of wyvern: can`t go south.
int mus_hallway_north_of_wyvern(int room, CHAR *ch, int cmd, char *arg)
{
  if( !ch || cmd != CMD_SOUTH )
    return FALSE;

  act ("A mound of wax prevents your entrance.",
      FALSE, ch, 0, 0, TO_CHAR);
  act ("The wax-filled doorjamb stops $n from going south.",
      FALSE, ch, 0, 0, TO_NOTVICT);
  return TRUE;
}

// Specs damage every mobact, damages and prevents songs/spells/scrolls.
// Only way out is to be a ninja or to be flying.
int mus_drowning(int room, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict = NULL;
  CHAR *next_vict = NULL;
  if( ch && GET_LEVEL(ch) >= LEVEL_IMM )
    return FALSE;

  if( cmd == MSG_MOBACT )
  {
    // Damage everyone for 300 except ninjas.
    for( vict = world[room].people; vict; vict = next_vict )
    {
			next_vict = vict->next_in_room;
      if( IS_NPC(vict) || GET_CLASS(vict) == CLASS_NINJA)
        continue;

      act ("You suffocate in the murk and the filth.",
         FALSE, vict, 0, 0, TO_CHAR);

      drink_the_koolaid( vict );
      damage (vict, vict, 300, TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    return FALSE;
  }

  bool affected = FALSE;

  vict = ch;
  if( !ch )
    return FALSE;
  else if( cmd == CMD_SONG )
  {
    act ("You clear your throat to warm up your singing voice and swallow the most foul bilgewater imaginable!",
         FALSE, vict, 0, 0, TO_CHAR);
    act ("$n's attempt to sing is rewarded with a taste of disgusting swampwater!",
         FALSE, vict, 0, 0, TO_ROOM);
    damage(vict, vict, number(300,500), TYPE_UNDEFINED,DAM_NO_BLOCK);
    affected = TRUE;
  }
  else if( cmd == CMD_CAST )
  {
    act ("When you open your mouth to speak the incantation, the murky water forces itself down your throat!",
         FALSE, vict, 0, 0, TO_CHAR);
    act ("When $n tries to speak a magical incantation, the murky water forces itself down $s throat!",
         FALSE, vict, 0, 0, TO_ROOM);
    damage(vict, vict, number(300,500), TYPE_UNDEFINED,DAM_NO_BLOCK);

    affected = TRUE;
  }
  else if( cmd == CMD_RECITE )
  {
    act ("As you open your mouth to recite the magical words, you swallow a mouthful of ooze.",
         FALSE, vict, 0, 0, TO_CHAR);
    act ("As $n opens $s mouth to recite the magical words of a scroll, $e swallows a mouthful of ooze.",
         FALSE, vict, 0, 0, TO_ROOM);
    damage(vict, vict, number(300,500), TYPE_UNDEFINED,DAM_NO_BLOCK);
    affected = TRUE;
  }
  else if( cmd == CMD_UP )
  {
    // Are you flying or a ninja?
    if( GET_CLASS( ch ) == CLASS_NINJA || IS_AFFECTED(ch, AFF_FLY) )
      return FALSE;

    act ("You struggle mightily against the ooze and the muck, but are dragged down.",
         FALSE, vict, 0, 0, TO_CHAR);
    act ("$n struggles mightily against the ooze and the muck, but is dragged down into the depths.",
         FALSE, vict, 0, 0, TO_ROOM);
    damage(vict, vict, number(300,500), TYPE_UNDEFINED,DAM_NO_BLOCK);

    affected = TRUE;
  }
  else if( cmd == CMD_SHOUT || cmd == CMD_YELL || cmd == CMD_SAY || cmd == CMD_WHISPER )
  {
    act("You open your mouth and swallow some foul water!", FALSE, vict, 0, 0, TO_CHAR );
    act("$n foolishly opens $s mouth and swallows the foul water!", FALSE, vict, 0, 0, TO_ROOM );
    damage(vict, vict, number(100,200), TYPE_UNDEFINED,DAM_NO_BLOCK);
    affected = TRUE;
  }
  else if( cmd == CMD_GOSSIP || cmd == CMD_AUCTION || cmd == CMD_QUEST || cmd == CMD_CHAOS ||
      cmd == CMD_TELL || cmd == CMD_GROUPTELL )
  {
    // It`d be a little too cruel to punish them for unfortunate timing of triggers, but
    // let`s not make it too easy to communicate, either.
    act( "The oppressively foul water encourages you to keep your peace.", FALSE, vict, 0, 0, TO_CHAR );
    return TRUE;
  }

  if( affected )
  {
    drink_the_koolaid( ch );
  }
  return affected;
}

int timur_cmd_index = -1;
char* timur_cmds[] = {
  "open door",
  "west",
  "north",
  "say MAMA!!!!" };


// Furies; can be tamed by a tamer or the presence of his mother's skin.
int mus_timur(CHAR *mob, CHAR* ch, int cmd, char *arg )
{
  if( cmd == MSG_DIE )
  {
    timur_killer = ch;
    timur_soothed = FALSE;
    timur_instance = NULL;
    act ("You have done the world a service by ridding it of this wretched abomination.",
        FALSE, mob, 0, ch, TO_VICT);
    act ("Nonetheless, you cannot help but feel as though you may have enraged its master.",
        FALSE, mob, 0, ch, TO_VICT);
    return FALSE;
  }

  timur_instance = mob;

  if( !mob )
    return FALSE;

  if( cmd == MSG_MOBACT )
  {
		if((3*GET_MAX_HIT(mob)/5) <= GET_HIT(mob))
		{   /* if > 60% hp */
			REMOVE_BIT(mob->specials.act,ACT_SHIELD);
		}
		else
		{ /* if <  60% hp */
			SET_BIT(mob->specials.act,ACT_SHIELD);
		}

    if( chance(50) && GET_POS( mob ) == POSITION_FIGHTING && !timur_soothed && !IS_SET(mob->specials.affected_by, AFF_FURY ) )
    {
      act( "$n misses his mama's soothing presence!", FALSE, mob, 0, mob, TO_ROOM );
      SET_BIT(mob->specials.affected_by,AFF_FURY);
      act("$n starts snarling and fuming with rage!\n",FALSE,mob,0,mob,TO_ROOM);
      return FALSE;
    }
    else if( timur_cmd_index >= 0 )
    {
      mob_do( mob, timur_cmds[timur_cmd_index] );
      timur_cmd_index++;
      if( timur_cmd_index == NUM_OF(timur_cmds)) timur_cmd_index = -1;
    }
  }
  else if( cmd == CMD_UNKNOWN )
  {
    char buf[MAX_INPUT_LENGTH];
    arg = one_argument( arg, buf );
    if( !*buf )
      return FALSE;

    if( strcasecmp( buf, "soothe" ) )
      return FALSE;

    one_argument( arg, buf );
    if( !*buf )
    {
      send_to_char( "Who do you want to soothe?\r\n", ch );
      return TRUE;
    }

    if( !strcasecmp( buf, "timur" ) )
    {
      // Needs to be wearing skin of scratch mama or be a Tamer

      bool wearing_skin = FALSE;
      if( EQ(ch, WEAR_ABOUT) &&
          !strcasecmp( EQ(ch, WEAR_ABOUT)->short_description, "Skin of Scratch Mama" ) )
        wearing_skin = TRUE;

      if( !(GET_SEX(ch) == SEX_FEMALE && wearing_skin) )
      {
        if( GET_CLASS( ch ) != CLASS_NOMAD )
        {
          send_to_char( "You should probably leave that to the nomads.\r\n", ch );
          return TRUE;
        }
        else if( !enchanted_by( ch, "The title of Tamer" ) )
        {
          send_to_char( "You do not think your taming skills are up the task.\r\n", ch );
          return TRUE;
        }
      }

      // OK, they can tame either because they are wearing her skin or are a Tamer.
      timur_soothed = TRUE;
      act ("You speak to $n in a soothing tone of voice.",
           FALSE, timur_instance, 0, ch, TO_VICT);
      act ("$N speaks to $n in a soothing tone of voice.",
           FALSE, timur_instance, 0, ch, TO_NOTVICT);
      act ("$N speaks to you reassuringly.",
           FALSE, timur_instance, 0, ch, TO_CHAR);

      if( IS_SET(mob->specials.affected_by, AFF_FURY ) )
      {
        act( "$n calms down.", FALSE, mob, 0, mob, TO_ROOM );
        REMOVE_BIT(mob->specials.affected_by,AFF_FURY);
      }
    }
    else
    {
      send_to_char( "You cannot soothe that!\r\n", ch );
    }

    return TRUE;
  }

  return FALSE;
}

bool mama_fighting = FALSE;
// On death, summons 1d3 panthers, 1d3 Panters and Timur. Fun!
int mus_mama(CHAR *mob, CHAR* ch, int cmd, char *arg )
{
  if( cmd == MSG_MOBACT && !mama_fighting && mob->specials.fighting )
  {
    mama_fighting = TRUE;
    do_say(mob, "Without me, who will soothe Timur's ferocity?", CMD_SAY );
  }
  else if( cmd == MSG_MOBACT && mama_fighting && !mob->specials.fighting )
  {
    mama_fighting = FALSE;
  }

	if((3*GET_MAX_HIT(mob)/5) <= GET_HIT(mob))
	{   /* if > 60% hp */
		REMOVE_BIT(mob->specials.act,ACT_SHIELD);
	}
	else
	{ /* if <  60% hp */
		SET_BIT(mob->specials.act,ACT_SHIELD);
	}

  if( cmd == MSG_DIE && ch )
  {
    mama_killer = ch;
    act ("$n says 'Lemouro shall avenge me, $N!'",
        FALSE, mob, 0, ch, TO_ROOM);

    send_to_room("The high priestess's faithful subjects appear at her death!\r\n", mob->in_room_r );
    int panthers=number(1,3);
    int i = 0;
    CHAR *panther;
    for(i=0;i<panthers;i++)
    {
      panther = read_mobile( MUS_PANTHER1, VIRTUAL );
      char_to_room(panther, mob->in_room_r);
      set_fighting( panther, ch );
    }

    panthers = number(1,3);
    for(i=0;i<panthers;i++)
    {
      panther = read_mobile( MUS_PANTHER2, VIRTUAL );
      char_to_room(panther, mob->in_room_r);
      set_fighting( panther, ch );
    }


    if( timur_instance && GET_POS( timur_instance ) == POSITION_STANDING )
    {
      timur_cmd_index = 0;
    }
    return FALSE;
  }

  return FALSE;
}

// Given cast    'thu'  queen returns "queen"
// Assumes buffer is an allocated buffer large enough to store result
void extract_victim( char *arg, char *buffer )
{
  int apostrophes = 0;
  while( apostrophes != 2 && *arg )
  {
    if( *arg == '\'' ) apostrophes++;
    arg++;
  }

  while( *arg )
  {
    if( *arg != ' ' )
    {
      *buffer = tolower(*arg);
      buffer++;
    }
    arg++;
  }
}

// Given "cast     `thu`", returns SPELL_THUNDERBALL
// If no spell found, returns -1.
//
// Adapted from spec.marikith.c; fixes memory leak.
int extract_from_table( char *arg, char**table )
{
  char *skip_spaces(char *);

  char *tmp_string;
  char *origi_string = NULL;

  int spell = -1;
  int qend;

  tmp_string = origi_string = str_dup (arg);
  tmp_string = skip_spaces(tmp_string);

  if (!(*tmp_string)) goto extract_spell_end;
  if (*tmp_string != '\'') goto extract_spell_end;

  for (qend=1; *(tmp_string+qend) && (*(tmp_string+qend) != '\'') ; qend++)
  *(tmp_string+qend) = LOWER(*(tmp_string+qend));

  if (*(tmp_string+qend) != '\'') goto extract_spell_end;

  spell = old_search_block(tmp_string, 1, qend-1,table, 0);

extract_spell_end:
  free(origi_string);
  return spell;
}

int extract_spell( char *arg ) {
  return extract_from_table(arg, spells);
}

int extract_song( char *arg ) {
  return extract_from_table(arg, songs);
}

// Summons black panthers; causes trouble to people who would cast spells on her.
int mus_quaratt(CHAR *mob, CHAR* ch, int cmd, char *arg )
{
  if( cmd == MSG_DIE )
  {
    quaratt_killer = ch;
    act ("$n lashes you with $s tail, marking you as $s executioner!",
        FALSE, mob, 0, ch, TO_VICT);
    return FALSE;
  }

  if( cmd == MSG_MOBACT && mob && GET_POS( mob ) == POSITION_FIGHTING )
  {
    // If there's not 6 panthers, summon one.
    struct room_data *quaratt_room = &world[CHAR_REAL_ROOM( mob )];

    int panther_count = 0;
    struct char_data *people = quaratt_room->people;
    for( people = quaratt_room->people; people; people = people->next )
      if( IS_NPC( people ) && (people->nr_v == MUS_PANTHER1 || people->nr_v == MUS_PANTHER2 ) )
        panther_count++;

    if( panther_count < 6 )
    {
      if( number(0,1) )
      {
        do_say( mob, "Kahnila mijala!", CMD_SAY );
        CHAR *panther1 = read_mobile(MUS_PANTHER1, VIRTUAL);
        char_to_room(panther1, mob->in_room_r);
      }
      else
      {
        do_say( mob, "Kahnila mi!", CMD_SAY );
        CHAR *panther2 = read_mobile(MUS_PANTHER2, VIRTUAL);
        char_to_room(panther2, mob->in_room_r);
      }
    }

    return FALSE;
  }

  if( cmd == CMD_CAST && ch )
  {
    int spl = extract_spell( arg );
    char wouldbevictim[256];
    memset(wouldbevictim,0,256);
    extract_victim( arg, wouldbevictim );
    if( !(ch->specials.fighting == mob && wouldbevictim[0] == 0 ))
      if( strcasecmp( wouldbevictim, "quaratt" ) && strcasecmp( wouldbevictim, "queen" ) )
        return FALSE; // if not spelling quaratt/queen, do whatever

    // It`d be cute if we could just fiddle the arg command to replace quaratt with
    // the victim`s name and let the system handle casting it.  However, I suspect
    // all of these will trigger the "You cannot cast such a powerful spell on a player!"
    // warning, so we`ll do it by hand.

    void (*cast_func) (ubyte level, CHAR *ch, CHAR *victim, OBJ *obj) = NULL;

    if( spl == SPELL_THUNDERBALL )
      cast_func = spell_thunderball;
    else if( spl == SPELL_FROSTBOLT )
      cast_func = spell_frostbolt;
    else if( spl == SPELL_ELECTRIC_SHOCK )
      cast_func = spell_electric_shock;
    else if( spl == SPELL_DEMONIC_THUNDER )
      cast_func = spell_demonic_thunder;
    else if( spl == SPELL_DEVASTATION )
      cast_func = spell_devastation;
    else if( spl == SPELL_WRATH_ANCIENTS )
      cast_func = spell_wrath_ancients;
    else if( spl == SPELL_WRATH_OF_GOD )
      cast_func = spell_wrath_of_god;
    else if( spl == SPELL_DIVINE_WIND )
      cast_func = spell_divine_wind;
    else if( spl == SPELL_DEMONIC_THUNDER )
      cast_func = spell_demonic_thunder;

    if( cast_func == NULL )
      return FALSE;

    CHAR *victim = get_random_victim( mob );
    if( victim == NULL || victim == ch)
    {
      act ("$n deftly dodges your attempt to use wizardry.",
          FALSE, mob, 0, ch, TO_VICT);
      act ("$n deftly dodges $N's attempt to use wizardry.",
          FALSE, mob, 0, ch, TO_NOTVICT);
      act ("You deftly dodge $N's attempt to use wizardry.",
          FALSE, mob, 0, ch, TO_CHAR);
    }
    else
    {
      char msg[512];

      sprintf(msg, "$n dodges your spell, causing it to hit %s with full force!", victim->player.name );
      act (msg, FALSE, mob, 0, ch, TO_VICT);

      sprintf(msg, "$n dodges $N's spell, causing it to hit %s with full force!", victim->player.name );
      act (msg, FALSE, mob, 0, ch, TO_NOTVICT);

      sprintf(msg, "You dodge $N's spell, causing it to hit %s with full force!", victim->player.name );
      act (msg, FALSE, mob, 0, victim, TO_CHAR);

      cast_func( 50, mob, victim, 0 );
    }

    return TRUE;
  }

  return FALSE;
}


char* lem_switch_warnings[] = {
  "glare %s",
  "snarl %s",
  "accuse %s" };

CHAR *lem_next_victim = NULL;
int lem_next_victim_timer = 0;

bool lem_ignore = FALSE;

int mus_lem(CHAR *mob, CHAR* ch, int cmd, char *arg )
{
  if( lem_ignore )  return FALSE;

  if( cmd == MSG_DIE )
  {
    lem_instance = khallaq_killer = quaratt_killer = mama_killer = timur_killer = tomek_killer = NULL;
    return FALSE;
  }
  else
  {
    lem_instance = mob;
  }

  if( !lem_instance )
    return FALSE;

  if( cmd == MSG_MOBACT && hakeem_died )
  {
    do_shout( lem_instance, "Rot in hell, Hakeem!", CMD_SHOUT );
    hakeem_died = FALSE;
    return FALSE;
  }

  // If there are player corpses in Lem`s room, scalp and skin them.
  struct obj_data *contents = NULL;

  for( contents = world[CHAR_REAL_ROOM(lem_instance)].contents; contents; contents = contents->next_content )
  {
    if( contents->obj_flags.cost == PC_CORPSE && strstr( OBJ_SHORT( contents ), "Corpse of" ) && contents->obj_flags.timer == MAX_PC_CORPSE_TIME )
    {
      lem_ignore = TRUE;
      mob_do( lem_instance, "trophy corpse" );
      mob_do( lem_instance, "skin corpse" );
      empty_container( contents );
      mob_do( lem_instance, "butcher corpse" );
      lem_ignore = FALSE;
      break;
    }

    if( strstr( OBJ_SHORT( contents ), "Corpse of " ) )
      break;
  }
  if( lem_instance->specials.fighting && cmd == MSG_MOBACT &&IS_SET(EXIT(lem_instance, NORTH)->exit_info, EX_CLOSED))
    mob_do(lem_instance, "open door" );

  //6/12/11: flee when not in fight, else a very lucky person can kill him before he flees
  if(cmd==MSG_MOBACT && ((100*GET_HIT(lem_instance)) / GET_MAX_HIT(lem_instance)) < 20 )
  {
    world[real_room(MUS_LEM_ROOM)].dir_option[DOWN]->to_room_r = real_room(MUS_ESCAPE_TUNNEL);
    act ("$n panics, and attempts to flee.", FALSE, lem_instance, 0, 0, TO_ROOM);
    if( get_obj_room(WALL_THORNS,CHAR_VIRTUAL_ROOM(lem_instance)))
    {
      act ("In his panic, some of $n's clothes snag on the thorns!", FALSE, lem_instance, 0, 0, TO_ROOM );
      OBJ* tmp_obj = read_object( MUS_CLOAK, VIRTUAL );
      obj_to_room( tmp_obj, CHAR_REAL_ROOM(lem_instance) );
      int lemobj;
      for( lemobj = 16558; lemobj < 16562; lemobj++ )
      {
        if( chance(20) )
        {
          tmp_obj = read_object( lemobj, VIRTUAL );
          obj_to_room( tmp_obj, CHAR_REAL_ROOM( lem_instance ) );
        }
      }
      for( lemobj = 16566; lemobj < 16569; lemobj++ )
      {
        if( chance(20) )
        {
          tmp_obj = read_object( lemobj, VIRTUAL );
          obj_to_room( tmp_obj, CHAR_REAL_ROOM( lem_instance ) );
        }
      }
    }
    act( "$n leaves down.", FALSE, lem_instance, 0, 0, TO_ROOM );
    int wearpos;
    for( wearpos = WEAR_LIGHT; wearpos <= HOLD; wearpos++ )
    {
      if( EQ( lem_instance, wearpos ) )  extract_obj( unequip_char( lem_instance, wearpos ) );
    }

    // Any shields Lem has stolen will get dropped to floor by extract_char
    extract_char( lem_instance );
    lem_instance = NULL;
    return FALSE;
  }

  if( cmd == MSG_MOBACT && lem_instance->specials.fighting )
  {
    if((3*GET_MAX_HIT(lem_instance)/5) <= GET_HIT(lem_instance))
    {   /* if > 60% hp */
      REMOVE_BIT(lem_instance->specials.act,ACT_SHIELD);
    }
    else
    { /* if <  60% hp */
      SET_BIT(lem_instance->specials.act,ACT_SHIELD);
    }

    CHAR *vict = NULL;
    for(vict = world[lem_instance->in_room_r].people; vict; vict = vict->next_in_room)
    {
      if(!IS_NPC(vict) && affected_by_spell(vict, SKILL_EVASION) && !enchanted_by(vict, ENCH_NAIAD_COWARDICE) )
      {
          char buf[512];
          sprintf(buf, "Fight me like a man, not a cowering wimp, %s!", vict->player.name );
          do_say( lem_instance, buf, MSG_RECONNECT );
          ENCH *tmp_enchantment;
          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup( ENCH_NAIAD_COWARDICE );
          tmp_enchantment->duration = 2;            /* Define the enchantment */
          tmp_enchantment->func     = ench_naiad_cowardice;
          enchantment_to_char(vict, tmp_enchantment, FALSE);
          send_to_char( "You feel ashamed at your cowardice.\r\n", vict );
      }
    }



    if( lem_next_victim && lem_next_victim_timer <= 0 )
    {
      CHAR *vict = NULL;
      bool lem_next_victim_in_room = FALSE;

      // have to iterate over room contents, can't just do CHAR_REAL_ROOM(lem_next_victim) because that
      // pointer could be bogus
      for(vict = world[lem_instance->in_room_r].people; vict; vict = vict->next_in_room)
      {
        if( vict == lem_next_victim )
        {
          lem_next_victim_in_room = TRUE;
          break;
        }
      }

      if( lem_next_victim_in_room )
      {
        stop_fighting( lem_instance );
        set_fighting( lem_instance, lem_next_victim );
        lem_next_victim = NULL;
      }
      else
      {
        // They ran away / died.
        lem_next_victim = NULL;
        lem_next_victim_timer = 0;
      }
    }
    else if( !lem_next_victim )
    {
      // Pick a random person and warn them.
      lem_next_victim = get_random_victim( lem_instance );

      if( lem_next_victim )
      {
        lem_next_victim_timer = number(1,3);
        char buf[200];
        sprintf(buf, lem_switch_warnings[number(0,2)], lem_next_victim->player.name );
        mob_do( lem_instance, buf );
      }
    }
    else
    {
      lem_next_victim_timer--;

      // Pick a random person and batter their shield away.
      CHAR *victim = get_random_victim( lem_instance );
      if( victim && !IS_NPC( victim ) && EQ( victim, WEAR_SHIELD ) )
      {
        struct obj_data *char_shield = unequip_char( victim, WEAR_SHIELD );
        log_f("ELWAXO: Lem stole from player %s, item %s (%d)", victim->player.name, OBJ_SHORT(char_shield), char_shield->item_number_v );
        obj_to_char( char_shield, lem_instance );

        act ("$n punches your wrist, causing you to drop your shield.",
             FALSE, lem_instance, 0, victim, TO_VICT);
        act ("$n punches $N, causing $M to drop $S shield.",
             FALSE, lem_instance, 0, victim, TO_NOTVICT);
        act ("You steal $N's shield.",
             FALSE, lem_instance, 0, victim, TO_CHAR);
      }
    }
  }
  return FALSE;
}

void mus_khallaq_dunk(CHAR *mob, CHAR* victim) {
  // There`s a bug here in that if the victim is on a flying mount, we`ll
  // treat them as not flying.  I can live with that...

  // If victim is flying, damage spec only.
  if( IS_AFFECTED( victim, AFF_FLY ) )
  {
    if( affected_by_spell( victim, SPELL_FLY ) )
      affect_from_char( victim, SPELL_FLY );

    act ("$n grabs you and dunks you into the fetid waters.",
        FALSE, mob, 0, victim, TO_VICT);
    act ("$n dunks $N into the fetid waters.  Yuck!",
        FALSE, mob, 0, victim, TO_NOTVICT);
    act ("You grab $N and dunk $M into the fetid waters below!",
        FALSE, mob, 0, victim, TO_CHAR);
    damage(mob, victim, number(200,300), TYPE_UNDEFINED,DAM_NO_BLOCK);

    return;
  }

  // If victim has a boat, remove it to floor.
  bool has_boat = FALSE;
  struct obj_data *obj;
  for (obj=victim->carrying; obj; obj=obj->next_content)
  {
    if (obj->obj_flags.type_flag == ITEM_BOAT)
    {
      has_boat = TRUE;
      break;
    }
  }

  if( has_boat )
  {
    act ("$n knocks you off your boat, submerging you in the murky depths!",
        FALSE, mob, 0, victim, TO_VICT);
    act ("$n knocks $N off their boat and submerges them into the fetid waters!",
        FALSE, mob, 0, victim, TO_NOTVICT);
    act ("You knock $N off their boat and into the waters below.",
        FALSE, mob, 0, victim, TO_CHAR);
    damage(mob, victim, number(300,400), TYPE_UNDEFINED,DAM_NO_BLOCK);
    has_boat = TRUE;
    while(has_boat) {
      has_boat = FALSE;
      for (obj=victim->carrying; obj; obj=obj->next_content)
      {
        if (obj->obj_flags.type_flag == ITEM_BOAT)
        {
          log_f("ELWAXO: Khallaq knocked %s's boat to ground, item %s (%d)", victim->player.name, OBJ_SHORT(obj), obj->item_number_v );
          drop_object(victim, obj);
          has_boat = TRUE;
          break;
        }
      }
    }
  }
  else
  {
    act ("$n suddenly grabs you, submerging you in the murky depths!",
        FALSE, mob, 0, victim, TO_VICT);
    act ("$n suddenly grabs $N and submerges them into the fetid waters!",
        FALSE, mob, 0, victim, TO_NOTVICT);
    act ("You submerge $N into the waters below.",
        FALSE, mob, 0, victim, TO_CHAR);
    damage(mob, victim, number(200,300), TYPE_UNDEFINED,DAM_NO_BLOCK);
  }

  char_from_room(victim);
  char_to_room(victim,real_room(MUS_DROWNING));
  do_look(victim,"",CMD_LOOK);
  act("$n appears in the middle of the room.", FALSE, victim, 0, 0, TO_ROOM);
}

// Dunks people into the depths unless they are flying.
int mus_khallaq(CHAR *mob, CHAR* ch, int cmd, char *arg )
{
  if( cmd == MSG_DIE )
  {
    khallaq_killer = ch;
    act ("$n claws you, marking you as $s executioner!",
        FALSE, mob, 0, ch, TO_VICT);
    return FALSE;
  }

  //Skeena 6/12/11: defend against hit-and-run tactics
  if(ch && (cmd == CMD_BACKSTAB  || cmd == CMD_ASSAULT || cmd == CMD_AMBUSH || cmd == CMD_CIRCLE) && !ch->specials.fighting) {
    if(chance(20)) return FALSE;

    send_to_char("Your attempts at stealth do not go unnoticed...\r\n", ch);
    mus_khallaq_dunk(mob, ch);
    WAIT_STATE(ch, 2*PULSE_VIOLENCE);
    return TRUE;
  }

  if( ch || cmd != MSG_MOBACT || !mob)
    return FALSE;

	if((3*GET_MAX_HIT(mob)/5) <= GET_HIT(mob))
	{   /* if > 60% hp */
		REMOVE_BIT(mob->specials.act,ACT_SHIELD);
	}
	else
	{ /* if <  60% hp */
		SET_BIT(mob->specials.act,ACT_SHIELD);
	}

  // Pick a random victim.
  CHAR *victim = get_random_victim( mob );
  if( !victim )
    return FALSE;

  mus_khallaq_dunk(mob, victim);
  return FALSE;
}

// Kimagee hide becomes a living imagee if its wearer is miracled
int mus_kimagee_hide(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if(!obj->equipped_by) return FALSE;

  CHAR *victim = obj->equipped_by;

  if(cmd == MSG_MIRACLE) {
    act("It's a miracle! The Kimagee hide on your arms leaps to life!", 1, victim, 0, 0, TO_CHAR);
    act("It's a miracle! The hide on $n's arms leaps to life!", 1, victim, 0, 0, TO_ROOM);
  } else if(cmd == CMD_PRACTICE && ch == obj->equipped_by) {
    send_to_char("You can't bear the thought of scuffing your Kimagee hide while practicing.\r\n", ch);
    return TRUE;
  } else {
    return FALSE;
  }
  char buf[MAX_INPUT_LENGTH];
  sprintf(buf,"ELWAXO: %s had Kimagee hide miracled at %d",GET_NAME(victim),world[CHAR_REAL_ROOM(victim)].number);
   log_s(buf);
  extract_obj( unequip_char( victim, WEAR_ARMS ) );
  CHAR *ghost = read_mobile( MUS_FLAYED_KIMAGEE, VIRTUAL );
  char_to_room(ghost, CHAR_REAL_ROOM(victim));
  return FALSE;
}

char* msgs_band_char[] = {
  "", // no benefit inside
  "", // no benefit in city
  "Stalks of grass shoot from your wristband, ensnaring $N!",  //field
  "Your wristband pulsates and tree roots silently leap to your aid against $N!", //forest
  "Years of tracking experience enables you to gain the high ground against $N!", //hills
  "You leap nimbly and cuff $N with your wristband.", //mountain
  "You call upon the naiads, who punish $N with a jet of water.", //water swimmable
  "You call upon the naiads, who punish $N with a jet of water.", //water noswimmable
  "You summon a sand dervish to buffet $N with a hail of sand!", //desert
  "You focus intently on the arctic ice, blasting $N with hoarfrost." //arctic
 };
char* msgs_band_vict[] = {
  "", // no benefit inside
  "", // no benefit in city
  "Stalks of grass shoot from $n's wristband, ensnaring you!",
  "$n's wristband pulsates and tree roots silently leap to $s aid against you!",
  "Years of tracking experience enables $n to gain the high ground against you!",
  "$n leaps nimbly and cuffs you with $s wristband.",
  "$n calls upon the naiads, who punish you with a jet of water.",
  "$n calls upon the naiads, who punish you with a jet of water.",
  "$n summons a sand dervish to buffet you with a hail of sand!",
  "$n focuses intently on the arctic ice, blasting you with hoarfrost."
 };
char* msgs_band_notvict[] = {
  "", // no benefit inside
  "", // no benefit in city
  "Stalks of grass shoot from $n's wristband, ensnaring $N!",
  "$n's wristband pulsates and tree roots silently leap to $s aid against $N!",
  "Years of tracking experience enables $n to gain the high ground against $N!",
  "$n leaps nimbly and cuffs $N with $s wristband.",
  "$n calls upon the naiads, who punish $N with a jet of water.",
  "$n calls upon the naiads, who punish $N with a jet of water.",
  "$n summons a sand dervish to buffet $N with a hail of sand!",
  "$n focuses intently on the arctic ice, blasting $N with hoarfrost."
 };

//White wristband gives 8-13 hp/mana extra per tick in fight
int mus_white_band(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *owner = obj->equipped_by;
  if(cmd != MSG_TICK || !owner || !owner->specials.fighting) return FALSE;
  //If they're wearing two wrist bands, only one should spec.
  if( EQ(owner, WEAR_WRIST_R) && EQ(owner, WEAR_WRIST_L) && V_OBJ(EQ(owner, WEAR_WRIST_R)) == MUS_WHITE_BAND &&
      V_OBJ(EQ(owner, WEAR_WRIST_L)) == MUS_WHITE_BAND && EQ(owner, WEAR_WRIST_R) != obj) return FALSE;

  int bonus = number(8, 13);
  CHAR *vict = owner->specials.fighting;
  //Only spec outdoors
  int sector =  world[CHAR_REAL_ROOM(owner)].sector_type;
  if(GET_HIT(owner)>-1 && sector > 1) {
    GET_HIT(owner) = MIN(GET_MAX_HIT(owner), GET_HIT(owner)+bonus);
    GET_MANA(owner) = MIN(GET_MAX_MANA(owner), GET_MANA(owner)+bonus);

    GET_MANA(vict) = MAX(0, GET_MANA(vict)-bonus);
    damage(owner, vict, bonus, TYPE_UNDEFINED,DAM_NO_BLOCK);

    act(msgs_band_char[sector], 1, owner, 0, vict, TO_CHAR);
    act(msgs_band_vict[sector], 1, owner, 0, vict, TO_VICT);
    act(msgs_band_notvict[sector], 1, owner, 0, vict, TO_NOTVICT);
  }

  return FALSE;
}

int ench_forest_friend(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg ) {
  if(cmd != MSG_DIE) return FALSE;
  //Ok, this is really a pain in the ass.
  //Is there someone in the room fighting this mob wearing black fur leggings?

  CHAR *furry = NULL;
  CHAR *vict;
  OBJ *obj = NULL;
  for(vict = world[CHAR_REAL_ROOM(ench_ch)].people; vict; vict = vict->next) {
    if(vict->specials.fighting == ench_ch && EQ(vict, WEAR_LEGS) && V_OBJ(EQ(vict, WEAR_LEGS)) == MUS_BLACK_LEGGINGS) {
      furry = vict;
      obj = EQ(vict, WEAR_LEGS);
      break;
    }
  }


  if(chance(80)) return FALSE;
  if(furry == NULL) return FALSE;

  act("As you move to deal $N a death blow, you are struck with mercy.", 1, furry, 0, ench_ch, TO_CHAR);
  act("As $n is about to deal the death blow, $e is struck with mercy.", 1, furry, 0, ench_ch, TO_VICT);
  act("As $n is about to deal the death blow, $e is struck with mercy.", 1, furry, 0, ench_ch, TO_NOTVICT);
  mob_do(furry, "say Go free, my friend!");

  for(vict = world[CHAR_REAL_ROOM(ench_ch)].people; vict; vict = vict->next_in_room)
    stop_fighting(vict);
  obj->spec_value = V_MOB(ench_ch);
  obj->obj_flags.timer = 120 + number(0, 120);
  //HACKHACK: We can't just extract char here.  The game code expects the char to
  //be around.  What we'll do is mira him, shuffle his ass off to the animal sanctuary
  //...where he will get purged on the next mobact
  GET_HIT(ench_ch) = GET_MAX_HIT(ench_ch);
  //So, this kind of sucks, can't make them stop fighting, but we can make them miss maybe
  ench_ch->points.armor = -500;
  char_from_room(ench_ch);
  char_to_room(ench_ch,real_room(MUS_SANCTUARY));

  return TRUE;
}

//When fighting a forest animal, chance to save it
//If saved, mob remembers and may assist in another fight
int mus_black_leggings(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *owner = obj->equipped_by;

  if(!owner || !owner->specials.fighting || cmd != MSG_MOBACT) return FALSE;

  //Have we previously spared someone's life? If yes, run the timer down.
  if(obj->spec_value != 0) {
    if(obj->obj_flags.timer > 0) {
      obj->obj_flags.timer--;
      return FALSE;
    }

    CHAR *fr = read_mobile( obj->spec_value, VIRTUAL );
    if(fr == NULL) { obj->spec_value = 0; return FALSE; }

    char_to_room(fr, CHAR_REAL_ROOM(owner));
    act("Your mercy is rewarded!", 1, owner, 0, fr, TO_CHAR);
    act("$n's mercy is rewarded as $N enters the fray!", 1, owner, 0, fr, TO_NOTVICT);
    set_fighting(fr, owner->specials.fighting);
    obj->spec_value = 0;
    return FALSE;
  }
  CHAR *vict = owner->specials.fighting;
  if(CHAOSMODE) return FALSE;
  /* Find the person who is getting killed ... */
  if(!IS_MOB(vict)) return FALSE;
  if(!(GET_CLASS(vict) == CLASS_AVIAN ||  GET_CLASS(vict) == CLASS_FISH || GET_CLASS(vict) == CLASS_ANIMAL || GET_CLASS(vict) == CLASS_SIMIAN || GET_CLASS(vict) == CLASS_CANINE || GET_CLASS(vict) == CLASS_FELINE || GET_CLASS(vict) == CLASS_RODENT))
    return FALSE;
  if(GET_LEVEL(vict) >= LEVEL_IMM-1) return FALSE;
  if (IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, CHAOTIC)) return FALSE;

  if(enchanted_by(vict, ENCH_FOREST_FRIEND)) return FALSE;
  //Give enchantment to him.
  jenny_enchant(vict, ENCH_FOREST_FRIEND);

  return FALSE;
}

char* mitten_taunts[] = {
  "say Nice mittens, dork!",
  "say Did your mom make those mittens for you?",
  "say You really seem attached to those mittens.",
  "say Isn't it a little warm for mittens, nerd?" };
// Mittens cause you to sweat and drop your weapon in desert sectors
// Inside/in summer, drunks/guards will taunt you
int mus_mittens(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *owner = obj->equipped_by;
  if(owner && cmd == MSG_MOBACT && world[CHAR_REAL_ROOM(owner)].sector_type == SECT_DESERT && EQ(owner, WIELD)) {
    act("Your hands sweat inside the mittens from the harsh desert sun.\r\nYou drop your weapon.", 1, owner, 0, 0, TO_CHAR);
    act("$n's hands sweat inside $s mittens, causing $m to drop $s weapon.", 1, owner, 0, 0, TO_ROOM);
    mus_remeq(owner, WIELD);
    return FALSE;
  }

  if(!owner || cmd != MSG_MOBACT || (world[CHAR_REAL_ROOM(owner)].sector_type != SECT_INSIDE && (time_info.month<=4 || time_info.month>10) && !OUTSIDE(owner))) return FALSE;
  if(ROOM_SAFE(CHAR_REAL_ROOM(owner))) return FALSE;
  if(owner->specials.fighting) return FALSE;

  //Any drunks?
  CHAR *punk = get_char_room_vis(owner, "drunk");
  if(punk == NULL) punk = get_char_room_vis(owner, "guard");
  if(punk == NULL) return FALSE;
  if(!chance(1)) return FALSE;
  mob_do(punk, mitten_taunts[number(0,NUM_OF(mitten_taunts)-1)]);
  set_fighting(owner, punk);
  return FALSE;
}
// Antlers snag and fall off when you do a climb/crawl/jump
// In fight, small chance to knock cloak/hold off of someone
int mus_antlers(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if(!obj->equipped_by) return FALSE;

  if( (cmd == CMD_CLIMB || cmd == CMD_CRAWL || cmd == CMD_JUMP) && obj->equipped_by == ch) {
    char buf[512];
    sprintf(buf, "As you begin to %s, your antlers tumble to the ground.", cmd == CMD_CLIMB ? "climb" : (cmd == CMD_CRAWL ? "crawl" : "jump"));
    act(buf, 1, ch, 0, 0, TO_CHAR);
    sprintf(buf, "As $n begins to %s, $s antlers tumble to the ground.", cmd == CMD_CLIMB ? "climb" : (cmd == CMD_CRAWL ? "crawl" : "jump"));
    act(buf, 1, ch, 0, 0, TO_ROOM);
    mus_remeq(ch, WEAR_SHIELD);
    return TRUE;
  }

  CHAR *wearer = obj->equipped_by;
  if(cmd != MSG_MOBACT || !wearer->specials.fighting) return FALSE;
  if(number(1,300) != 1) return FALSE;
  int attempts = 0;
  int positions[] = {WEAR_ABOUT, HOLD};
  for(attempts = 0; attempts < 5; attempts++) {
    CHAR *vict = get_random_victim(wearer);
    if(vict == NULL) continue;
    int pos = positions[chance(50) ? 1 : 0];
    if(!EQ(vict, pos)) continue;
    act("You stumble from the massive weight of your antlers, snagging $N's $o!", 1, wearer, EQ(vict, pos), vict, TO_CHAR);
    act("$n stumbles from the massive weight of $s antlers, snagging your $o!", 1, wearer, EQ(vict, pos), vict, TO_VICT);
    act("$n stumbles from the massive weight of $s antlers, snagging $N's $o!", 1, wearer, EQ(vict, pos), vict, TO_NOTVICT);
    mus_remeq(vict, pos);
    break;
  }
  return FALSE;
}

// Bard skull causes terror in other bards, who will flee
// Bards equipping it must be drunk
// On death, 10% chance it total songs you
int mus_elar_skull(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if(cmd == MSG_OBJ_WORN) {
    if( GET_CLASS(ch) == CLASS_BARD ) {
      if(GET_COND(ch,DRUNK)<10) {
        act("You are repulsed by your actions and immediately extinguish it.", 1, ch, 0, 0, TO_CHAR);
        act("$n is repulsed by $s actions and immediately extinguishes it.", 1, ch, 0, 0, TO_ROOM);
        return TRUE;
      }
    }

    CHAR *vict = NULL;
    CHAR *next_vict = NULL;
    for(vict = world[CHAR_REAL_ROOM(ch)].people; vict; vict = next_vict) {
      next_vict = vict->next_in_room;
      if(vict != ch && GET_CLASS(vict) == CLASS_BARD && GET_POS(vict) >= POSITION_FIGHTING && GET_LEVEL(vict) < 56) {
        act("Lighting the skull scares $N!", 1, ch, 0, vict, TO_CHAR);
        act("Did you just see $n light a bard's skull on fire?!", 1, ch, 0, vict, TO_VICT);
        act("$N looks at $n fearfully!", 1, ch, 0, vict, TO_NOTVICT);
        mob_do(vict, "flee");
      }
    }
    return FALSE;
  }


  CHAR *tmp, *tch;
  char buf[1000];

  if(CHAOSMODE) return FALSE;
  if(cmd != MSG_DIE) return FALSE;
  /* Find the person who is getting killed ... */
  if(!obj->equipped_by) return FALSE;
  tch = obj->equipped_by;
  if(IS_MOB(tch)) return FALSE;

  if (IS_SET(world[CHAR_REAL_ROOM(tch)].room_flags, CHAOTIC)) return FALSE;
  if(number(1,10) != 1) return FALSE;
  act ("$n's skull screams 'NOT AGAIN!'", TRUE, tch, obj, 0, TO_ROOM);
  act ("$n's skull screeches 'we are the world, we are the children....'.", TRUE, tch, obj, 0, TO_ROOM);
  send_to_char ("As you prepare to die, your skull screams 'NOT AGAIN!'\r\nand frantically sings a song.\n\r",tch);

  //if a skull is 6dam, sorry, it is now capped at 5
  if (obj->affected[0].modifier > 5) {
    send_to_char("The piercing screech from the skull cracks itself slightly.", tch);
    obj->affected[0].modifier = 5;
  }

  GET_HIT (tch) = 1;
  GET_MANA (tch) = 0;
  GET_MOVE (tch) = 0;

  for (tmp = world[CHAR_REAL_ROOM(tch)].people;tmp;tmp=tmp->next_in_room)
    stop_fighting (tmp);

  sprintf (buf, "ELWAXO: %s rescued %s from %s [%d].", OBJ_SHORT(obj),
            GET_NAME(tch), world[CHAR_REAL_ROOM(tch)].name,
            CHAR_VIRTUAL_ROOM(tch));
  wizlog(buf, LEVEL_WIZ, 6);
  log_s(buf);

  stop_fighting (tch);

  for (tmp = world[CHAR_REAL_ROOM(tch)].people;tmp;tmp=tch) {
    tch = tmp->next_in_room;
    spell_word_of_recall (GET_LEVEL(tmp), tmp, tmp, 0);
  }

  return TRUE;
}

// Tomtom has a small chance to sing invuln song when backflipping
int mus_tomtom(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
  if(!obj->equipped_by) return FALSE;
  if(ch != obj->equipped_by) return FALSE;
  CHAR *holder = obj->equipped_by;
  CHAR *tmp_victim = NULL;
  CHAR *temp = NULL;
  if(cmd != CMD_BACKFLIP) return FALSE;
  if(GET_CLASS(holder)!=CLASS_BARD) return FALSE;
  if(!holder->specials.fighting) return FALSE;

  // Roughly once every 2 hours of backflipping
  if(number(0,1000) != 1) return FALSE;

  act("You use your tomtom to beat out an invigorating tattoo on $N's head.", 1, ch, 0, holder->specials.fighting, TO_CHAR);
  act("$n beats you senseless with $s tomtom!", 1, ch, 0, holder->specials.fighting, TO_VICT);
  act("$n uses $s tomtom to beat out an invigorating tattoo on $N's head.", 1, ch, 0, holder->specials.fighting, TO_NOTVICT);
  for(tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = temp) {
    temp=tmp_victim->next_in_room;
    if (ch != tmp_victim)
      spell_haste(50, ch, tmp_victim, 0);
   }
   spell_haste(50, ch, ch, 0);

  return FALSE;
}

// Mukluks are 4/4 hp/mana regen in arctic, or 150mv outside of arctic
// Also, 2dam.
int mus_mukluks(OBJ *obj, CHAR *ch,int cmd,char *arg)
{
  if( !obj || !obj->equipped_by || cmd != MSG_TICK )
    return FALSE;

  CHAR *owner = obj->equipped_by;
  bool is_arctic = obj->affected[0].location == APPLY_MANA_REGEN;
  bool is_room_arctic = world[CHAR_REAL_ROOM(owner)].sector_type == SECT_ARCTIC;

  if( is_arctic == is_room_arctic ) return FALSE;

  if( is_room_arctic )
  {
    send_to_char( "You burrow into your mukluks for warmth.\r\n", owner );
    obj->affected[0].location = APPLY_MANA_REGEN;
    obj->affected[0].modifier = number(14,20);
    obj->affected[1].location = APPLY_HP_REGEN;
    obj->affected[1].modifier = number(14,20);
    obj->affected[2].modifier = 2;
    obj->affected[2].location = APPLY_DAMROLL;
  }
  else
  {
    send_to_char( "You knock the snow off your mukluks.\r\n", owner );
    obj->affected[0].location = APPLY_NONE;
    obj->affected[0].modifier = 0;
    obj->affected[1].location = APPLY_MOVE;
    obj->affected[1].modifier = 150;
    obj->affected[2].modifier = 2;
    obj->affected[2].location = APPLY_DAMROLL;
  }
  return FALSE;
}

char* cloak_conditions[] = {
  "pristine",
  "lightly soiled",
  "scuffed",
  "dirty",
  "filthy",
  "threadbare and filthy",
  "tattered and filthy"
};


// Cloak takes 60 ticks of solid fighting to degrade
// Degrades faster if you are fighting with multiple people.
#define NUM_TICKS 60

#define CTHORR_VALUE (NUM_TICKS+1)
#define SOUDURE_VALUE (NUM_TICKS+2)
#define CORNELIUS_VALUE (NUM_TICKS+3)
#define TICKS_PER_DEGRADE 10

void change_cloak_condition( OBJ *cloak )
{
  CHAR *victim = cloak->equipped_by;
  if( !victim )
    victim = cloak->carried_by;

  char buf[200];
  sprintf(buf, "The cloak is in a %s condition.\r\n", cloak_conditions[cloak->spec_value / TICKS_PER_DEGRADE] );
  send_to_char( buf, victim );

  sprintf(buf, "a %s traveller's cloak", cloak_conditions[cloak->spec_value / TICKS_PER_DEGRADE ] );
  cloak->short_description = str_dup( buf );
}

void convert_to_la_soudure( struct obj_data *cloak );
void convert_to_cornelius( struct obj_data *cloak );
void convert_to_cthorr( struct obj_data *cloak );

int mus_cloak(OBJ *cloak, CHAR *ch,int cmd,char *arg)
{
  CHAR *owner = cloak->equipped_by;
  if( cmd == MSG_OBJ_ENTERING_GAME && cloak->spec_value > 0 )
  {
    // short/long descs aren't saved to file, so we'll do some magic here
    if( cloak->spec_value <= NUM_TICKS )
      change_cloak_condition( cloak );
    else if( cloak->spec_value == CTHORR_VALUE )
      convert_to_cthorr( cloak );
    else if( cloak->spec_value == SOUDURE_VALUE )
      convert_to_la_soudure( cloak );
    else if( cloak->spec_value == CORNELIUS_VALUE )
      convert_to_cornelius( cloak );
    else if( cloak->spec_value > CORNELIUS_VALUE )
      change_cloak_condition( cloak );
    return FALSE;
  }

  if( cmd == MSG_MOBACT && owner && !IS_NPC( owner ) && cloak->spec_value == 0 )
  {
    // First player to wear it over a tick becomes quest owner; no other players
    // can use this cloak.
    SET_BIT( cloak->obj_flags.wear_flags, ITEM_QUESTWEAR );
    cloak->ownerid[0] = cloak->equipped_by->ver3.id;
    cloak->spec_value = 1;
    send_to_char( "The cloak fits like it was made for you alone!\r\n", owner );
    change_cloak_condition( cloak );
    return FALSE;
  }
  if( cmd == MSG_TICK && cloak->spec_value < NUM_TICKS && cloak->equipped_by && cloak->equipped_by->specials.fighting )
  {
    int modifier = 1;
    int mortal_count = 0;
    CHAR *dude, *next_dude;

    for(dude = world[CHAR_REAL_ROOM(owner)].people; dude; dude = next_dude)
    {
			next_dude = dude->next_in_room;
			if(IS_MORTAL(dude) && dude->specials.fighting)
				mortal_count++;
		}

    if( mortal_count > 15 )
    {
      modifier = 100;
      send_to_char( "The enormous amount of people in the fight causes your cloak to wear out extremely quickly!\r\n", owner );
    }
    else if( mortal_count > 10 )
    {
      modifier = 50;
      send_to_char( "The large amount of people in the fight causes your cloak to wear out quickly!\r\n", owner );
    }
    else if( mortal_count > 3 )
    {
      modifier = 10;
      send_to_char( "The amount of people in the fight causes your cloak to wear out!\r\n", owner );
    }
    else
      modifier = 1;

    int old_value = cloak->spec_value;
    cloak->spec_value += modifier;
    if( cloak->spec_value > NUM_TICKS )
      cloak->spec_value = NUM_TICKS;

    if( (cloak->spec_value / TICKS_PER_DEGRADE) > ( old_value / TICKS_PER_DEGRADE ) )
    {
      change_cloak_condition( cloak );
    }

    return FALSE;
  }

  // Life cycle of cloak:
  // Every minute it is equipped in battle, it accumulates wear and tear.
  // Every 7th of the way, its short desc changes.  First tick will go from
  // a traveller`s cloak -> a pristine traveller`s cloak
  // to hint to the user that something is up.
  // Once it is tattered, filthy you can do something with Pirate Captain, larvae in Cthorr or
  // girl in Marikith to get a shiny new cape.

  if( !owner ) return FALSE;
  if( cloak->spec_value == CTHORR_VALUE )
  {
    if( cmd == MSG_TICK && owner->specials.fighting && IS_NPC(owner->specials.fighting ) && chance( 65 ) )
    {
      act ("$n retches from the smell of $s foul cloak, spraying bile all over you!",
          FALSE, owner, 0, owner->specials.fighting, TO_VICT);
      act ("$n's foul-smelling cloak causes $m to retch, spraying bile all over $N!",
          FALSE, owner, 0, owner->specials.fighting, TO_NOTVICT);
      act ("You retch bile all over $N!",
          FALSE, owner, 0, owner->specials.fighting, TO_CHAR);

      spell_acid_breath( GET_LEVEL( owner ), owner, owner->specials.fighting, NULL );
    }
  }
  else if( cloak->spec_value == CORNELIUS_VALUE )
  {
    if( cmd == MSG_TICK && chance( 20 ) )
    {
      // Flip invisibility bit.
      if( IS_SET( cloak->obj_flags.extra_flags, ITEM_INVISIBLE ) )
        REMOVE_BIT( cloak->obj_flags.extra_flags, ITEM_INVISIBLE );
      else
        SET_BIT( cloak->obj_flags.extra_flags, ITEM_INVISIBLE );
    }

    if( owner->specials.fighting && cmd == MSG_TICK && chance( 3 ) )
    {
      act ("You are blinded by fire coming from $n's cape!",
          FALSE, owner, 0, owner->specials.fighting, TO_VICT);
      act ("$n's cape explodes a fiery display of warmth, burning $N!",
          FALSE, owner, 0, owner->specials.fighting, TO_NOTVICT);
      act ("Your cape explodes in a fiery display of warmth.",
          FALSE, owner, 0, owner->specials.fighting, TO_CHAR);

      spell_conflagration( GET_LEVEL( owner ), owner, owner->specials.fighting, NULL );
    }
  }

  return FALSE;
}

// Lem`s cloaks in chest are there to tantalize the user, but cannot be worn.
int mus_cloak_lem(OBJ *cloak, CHAR *ch,int cmd,char *arg)
{
  if( !ch || cmd != CMD_WEAR || !cloak )
    return FALSE;

  if( IS_NPC(ch)) return FALSE;
  char buf[MAX_INPUT_LENGTH];
  one_argument(arg, buf);

  if( !buf[0])
    return FALSE;

  if( ch != cloak->carried_by )
    return FALSE;

  if( isname(buf, "all" ) || isname(buf, "cloak" ) || isname(buf, "robe") || isname(buf, "cthorr" ) || isname(buf, "cornelius") || isname(buf, "cape" ))
  {
    send_to_char("You don't think Lemouro would like you to wear that.\r\nPerhaps you should get your own?\r\n", ch);
    return TRUE;
  }

  return FALSE;
}


int mus_la_soudure_east(int room, CHAR *ch, int cmd, char *arg)
{
  if( !ch )
    return FALSE;

  struct obj_data* cloak = EQ(ch, WEAR_ABOUT );
  if( !cloak ) return FALSE;

  // Classes that could summon cannot speedwalk to Daimyo
  if( GET_CLASS(ch) == CLASS_PALADIN || GET_CLASS(ch) == CLASS_CLERIC )
    return FALSE;

  if( obj_proto_table[cloak->item_number].virtual == MUS_CLOAK && cloak->spec_value == SOUDURE_VALUE )
  {
    if( cmd == MSG_ENTER )
      send_to_char( "Your cloak sways in the gentle breeze.\r\n", ch );
    else if( cmd == CMD_EAST )
    {
      act ("$n navigates the watery depths of La Soudure eastwards.", TRUE, ch, 0, 0, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch,real_room(DAIMYO_DOCKS));
      do_look(ch,"",CMD_LOOK);
      return TRUE;
    }
  }

  return FALSE;
}

int Captain(CHAR *mob, CHAR* ch, int cmd, char *arg );
int nergal_larvae(CHAR *mob, CHAR* ch, int cmd, char *arg );
int nergal_bodyguard(CHAR *mob, CHAR* ch, int cmd, char *arg );
int mar_ghost(CHAR *mob, CHAR* ch, int cmd, char *arg );

int count_obj_in_carry( CHAR* ch, int virtual )
{
  int count = 0;
  struct obj_data* carrying = ch->carrying;

  for( carrying = ch->carrying; carrying; carrying = carrying->next_content )
    if( carrying->item_number_v == virtual )
      count++;

  return count;
}


struct obj_data* find_obj_in_carry( CHAR* ch, int virtual )
{
  struct obj_data* carrying = ch->carrying;

  for( carrying = ch->carrying; carrying; carrying = carrying->next_content )
  {
    if( carrying->item_number_v == virtual )
      return carrying;
  }
  return NULL;
}

#define SET_OBJ_STAT(obj,stat) (obj)->obj_flags.extra_flags = (stat)

// Converts a tattered, filthy traveller`s cloak to a Cornelius cape
void convert_to_cornelius( struct obj_data *cloak )
{
  cloak->spec_value = CORNELIUS_VALUE;
  cloak->short_description = str_dup( "a sienna-coloured cape from Cornelius" );
  cloak->name = str_dup( "cape cornelius" );
  cloak->description = str_dup( "Flickering in and out of existence is a deep sienna cape." );

  struct extra_descr_data* extradesc;
  CREATE(extradesc, struct extra_descr_data, 1 );
  extradesc->keyword = str_dup( "cape cornelius" );
  extradesc->description = str_dup( "You are warmed just looking at the thick fabric of this deep sienna-\r\ncoloured cape. It looks to have endured the test of time, as it\r\nis stitched with runes that you had thought to have been lost to the\r\nsands of time.\r\n\r\nWait, where did it go?" );
  cloak->ex_description = extradesc;
  cloak->affected[0].location = APPLY_DAMROLL;
  cloak->affected[0].modifier = 3;
  cloak->affected[1].location = APPLY_HITROLL;
  cloak->affected[1].modifier = 1;
  cloak->affected[2].location = APPLY_MANA_REGEN;
  cloak->affected[2].modifier = 7;

  cloak->obj_flags.value[0] = 5;
  SET_OBJ_STAT(cloak, ITEM_ANTI_EVIL|ITEM_ANTI_WARRIOR|ITEM_ANTI_THIEF|ITEM_ANTI_CLERIC|ITEM_ANTI_NOMAD|ITEM_ANTI_PALADIN|ITEM_ANTI_ANTIPALADIN|ITEM_ANTI_BARD);
}

// Converts a tattered, filthy traveller`s cloak to a Cthorr cloak
void convert_to_cthorr( struct obj_data *cloak )
{
  cloak->spec_value = CTHORR_VALUE;
  cloak->short_description = str_dup( "the acid-washed cloak of Cthorr" );
  cloak->name = str_dup( "cloak cthorr" );
  cloak->description = str_dup( "Ewww! A foul stench emanates from a cloak with many tears and holes." );

  struct extra_descr_data* extradesc;
  CREATE(extradesc, struct extra_descr_data, 1 );
  extradesc->keyword = str_dup( "cloak cthorr" );
  extradesc->description = str_dup( "A sour smell of acid invades your nostrils as you regard this cloak.\r\nSinged holes signify parts of the cloak that have met their demise due\r\nto the acidic environment of their master.  The people of Cthorr aren't\r\nknown for their hospitality, leaving you wondering how Lem laid his\r\nhands on this valuable cloak." );
  cloak->ex_description = extradesc;

  cloak->affected[0].location = APPLY_DAMROLL;
  cloak->affected[0].modifier = 4;

  cloak->affected[1].location = APPLY_SAVING_BREATH;
  cloak->affected[1].modifier = -30;

  cloak->affected[2].location = APPLY_HP_REGEN;
  cloak->affected[2].modifier = 19;

  cloak->obj_flags.value[0] = 11;
  SET_OBJ_STAT(cloak, ITEM_ANTI_EVIL|ITEM_ANTI_CLERIC|ITEM_ANTI_MAGIC_USER|ITEM_ANTI_NINJA|ITEM_ANTI_NOMAD|ITEM_ANTI_ANTIPALADIN|ITEM_ANTI_BARD|ITEM_ANTI_COMMANDO);

}

// Converts a tattered, filthy traveller`s cloak to a La Soudure robe
void convert_to_la_soudure( struct obj_data *cloak )
{
  cloak->spec_value = SOUDURE_VALUE;
  cloak->short_description = str_dup( "a La Soudure wanderer's robe" );
  cloak->name = str_dup( "robe" );
  cloak->description = str_dup( "A salt-caked robe sits on the ground here." );

  struct extra_descr_data* extradesc;
  CREATE(extradesc, struct extra_descr_data, 1 );
  extradesc->keyword = str_dup( "robe" );
  extradesc->description = str_dup( "Caked in a crust of sea-salt, this robe is one of the few items in this\r\nworld that can claim to have seen the eye of the maelstrom and come out\r\nthe other side. It is rumoured that the captain of the pirates himself\r\ngave this robe to Lemouro after he drank him under the table one night.\r\nAt least, that's what Lemouro keeps saying...");
  cloak->ex_description = extradesc;

  cloak->obj_flags.value[0] = 15;
  cloak->affected[0].location = APPLY_DAMROLL;
  cloak->affected[0].modifier = 4;

  SET_OBJ_STAT(cloak, ITEM_ANTI_EVIL|ITEM_ANTI_WARRIOR|ITEM_ANTI_THIEF|ITEM_ANTI_CLERIC|ITEM_ANTI_MAGIC_USER|ITEM_ANTI_PALADIN|ITEM_ANTI_ANTIPALADIN|ITEM_ANTI_COMMANDO);
}

// Captain will swap your worn out cloak if you give him 5 diamonds.
int mus_Captain(CHAR *mob, CHAR* ch, int cmd, char *arg )
{
  char buf[200];

  if( cmd == MSG_GAVE_OBJ )
  {
    if( mob->points.gold > 0 )
    {
      do_say( mob, "Coin o' the realm never really interested me.", CMD_SAY );
      mob_do( mob, "drop all.coins" );
      mob_do( mob, "drop cloak" );
      return FALSE;
    }

    struct obj_data *cloak = find_obj_in_carry( mob, MUS_CLOAK );

    if( cloak )
    {
      if( cloak->spec_value < NUM_TICKS )
      {
        do_say( mob, "Landlubbers! Ye'd never survive at sea. Plenty o' use left in that cloak!", CMD_SAY );
        mob_do( mob, "bah" );
        mob_do( mob, "drop cloak" );
      }
      else if( cloak->spec_value > NUM_TICKS )
      {
        // Nothing happens if you give him one that`s already been transformed.
        mob_do( mob, "wear all" );
        do_say( mob, "That's a fine cloak. I think I'll keep it.", CMD_SAY );
        sprintf(buf, "chortle %s", ch->player.name );
        command_interpreter( mob, buf );
      }
      else
      {
        if( count_obj_in_carry( mob, DIAMOND_BAR ) < 5 )
        {
          do_say( mob, "I don't know what stories Lem told you...", CMD_SAY);
          do_say( mob, "...but you can pay the same price he did. Twenty five million.", CMD_SAY);
          sprintf(buf, "give cloak %s", ch->player.name );
          command_interpreter( mob, buf );
        }
        else
        {
          // Junk the diamonds, convert the cloak, give it back.
          do_say( mob, "Yeh, I can help you with that.", CMD_SAY );
          bool found_diamond = FALSE;
          int diamonds_nuked = 0;
          do
          {
            found_diamond = FALSE;
            struct obj_data *carrying;
            for(carrying = mob->carrying; carrying; carrying = carrying->next_content )
            {
              if( carrying->item_number_v == DIAMOND_BAR )
              {
                diamonds_nuked++;
                extract_obj( carrying );
                found_diamond = TRUE;
                break;
              }
            }
          } while( found_diamond && diamonds_nuked < 5);

          convert_to_la_soudure( cloak );

          char buf[200];
          sprintf(buf, "give robe %s", ch->player.name );
          command_interpreter( mob, buf );
        }
      }
    }
  }

  return Captain(mob,ch,cmd,arg);
}

int mus_nergal_spec(CHAR *mob, CHAR* ch, int cmd, char *arg)
{
  // If buffer is wearing traveller`s cloak, we`ll do our own acid breath specs.
  if( ( cmd == MSG_MOBACT || cmd == MSG_DIE ) && mob && mob->specials.fighting )
  {
    CHAR *victim = mob->specials.fighting;

    struct obj_data* about = EQ(victim, WEAR_ABOUT);

    if( about && obj_proto_table[about->item_number].virtual == MUS_CLOAK )
    {
      if( cmd == MSG_MOBACT && about->spec_value == NUM_TICKS )
      {
        if( chance(20 ) )
        {
          act("$n vomits on $N!", 1, mob, 0, victim,TO_NOTVICT);
          act("$n vomits on you!", 1, mob, 0, victim ,TO_VICT);
          convert_to_cthorr( about );
        }
      }

      return TRUE;
    }
  }

  return FALSE;
}

// If a wornout cloak is barfed on, it gets made into a real cloak.
// The astute reader will also note that Lem`s cloak protects them from all other acid specs, too.
int mus_nergal_larvae(CHAR *mob, CHAR* ch, int cmd, char *arg )
{
  if( mus_nergal_spec( mob, ch, cmd, arg ) ) return FALSE;

  return nergal_larvae(mob,ch,cmd,arg);
}

int mus_nergal_bodyguard(CHAR *mob, CHAR *ch, int cmd, char *arg )
{
  if( mus_nergal_spec( mob, ch, cmd, arg ) ) return FALSE;

  return nergal_bodyguard(mob,ch,cmd,arg);
}

// If you give her your worn out cloak, she`ll give you her fancy one.
int mus_mar_ghost(CHAR *mob, CHAR* ch, int cmd, char *arg )
{
  if( cmd == MSG_MOBACT )
  {
    //If not wearing cape of Cornelius, equip it.
    if( !EQ(mob, WEAR_ABOUT))
    {
      struct obj_data *cape = read_object( MUS_LEM_CLOAKS3, VIRTUAL );
      obj_to_char( cape, mob );
      mob_do( mob, "wear all" );
    }
  }

  if( cmd == MSG_DIE )
  {
    // If she's wearing the cape of Cornelius, extract it.
    if( EQ(mob, WEAR_ABOUT) )
    {
      act ("$n's cape bursts into bright flames and disappears before your eyes.", TRUE, mob, 0, 0, TO_ROOM);
      act ("Your cape bursts into bright flames and disappears.", TRUE, mob, 0, 0, TO_CHAR);

      extract_obj( unequip_char( mob, WEAR_ABOUT ) );
    }
  }
  else if( cmd == MSG_GAVE_OBJ )
  {
    if( ch && is_carrying_obj( mob, MUS_CLOAK ) )
    {
      struct obj_data* cloak = find_obj_in_carry( mob, MUS_CLOAK );

      if( cloak->spec_value < NUM_TICKS )
      {
        do_say( mob, "I could never accept a cloak in such fine condition!", CMD_SAY );
        char buf[200];
        sprintf( buf, "give cloak %s", ch->player.name );
        command_interpreter( mob, buf );
      }
      else if( cloak->spec_value > NUM_TICKS )
      {
        do_say( mob, "I don't want that!", CMD_SAY );
        mob_do( mob, "drop all" );
      }
      else
      {
        // This gets kind of gross: we actually want to
        //   remove her cape, extract it
        //   convert his cloak to a cape (so it keeps quest flag) and give that back
        //   create a new cloak, make it tattered and filthy, and wear it

        do_say( mob, "Thank you...this cloak fits my mourning perfectly.", CMD_SAY );
        do_say( mob, "Please, take my old cape.", CMD_SAY );
        mob_do( mob, "remove cape" );
        struct obj_data *cape2 = find_obj_in_carry( mob, MUS_CORNELIUS_CAPE );
        extract_obj( cape2 );

        convert_to_cornelius( cloak );
        char buf[200];
        sprintf(buf, "give cape %s", ch->player.name );
        command_interpreter( mob, buf );

        cape2 = read_object( MUS_CLOAK, VIRTUAL );
        cape2->spec_value = NUM_TICKS;
        obj_to_char( cape2, mob );
        change_cloak_condition( cape2 );

        mob_do( mob, "wear cloak" );
     }
    }
  }

  return mar_ghost(mob,ch,cmd,arg);
}

// Load either of the rotting bridles
int mus_horse( CHAR *mob, CHAR *ch, int cmd, char *arg )
{
  if( mob && cmd == MSG_ZONE_RESET )
  {
    int obj_load = MUS_BRIDLE1;
    if( chance(50) ) obj_load = MUS_BRIDLE2;

    if( !EQ(mob, WEAR_WAIST ) )
    {
      OBJ * obj = read_object( obj_load, VIRTUAL );
      obj_to_char(obj,mob);
      mob_do(mob, "wear all" );
    }
  }

  return FALSE;
}

// 50% chance to flee on mobact
int ench_naiad_cowardice(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  if(cmd==CMD_RENT || cmd==CMD_QUIT)
  {
    send_to_char("Go to sleep? Someone might hurt you! You're too afraid to do that.\r\n", ench_ch );
    return TRUE;
  }
  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n cowers at the slightest noise!", FALSE, ench_ch, NULL, ch, TO_VICT );
    return FALSE;
  }
  if(cmd==MSG_REMOVE_ENCH)
  {
    send_to_char("You regain your courage..\r\n", ench_ch);
    return FALSE;
  }

  if( cmd == MSG_MOBACT && ench_ch->specials.fighting && chance(50) )
  {
    mob_do( ench_ch, "rem all" );
    mob_do( ench_ch, "flee" );
  }

  return FALSE;
}

// Marker enchantment to note that the player was in a fight against naiad
// Used to determine if they should get cowardice enchantment
int ench_naiad_chilling(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  if( ch==ench_ch && (cmd==CMD_RENT || cmd==CMD_QUIT))
  {
    send_to_char( "You seem to be unable to do that right now.\r\n", ench_ch );
    return TRUE;
  }
  return FALSE;
}

// 50% chance to fail song/spell/recite. Can't rent while enchanted.
int ench_naiad_frozen(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  if(ch==ench_ch && (cmd==CMD_RENT || cmd==CMD_QUIT))
  {
    send_to_char("In your hypothermic state, sleeping might kill you! Better not risk it.\r\n", ench_ch );
    return TRUE;
  }
  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n shivers with hypothermia!", FALSE, ench_ch, NULL, ch, TO_VICT );
    return FALSE;
  }
  if(cmd==MSG_REMOVE_ENCH)
  {
    send_to_char("Warmth returns to your extremities.\r\n", ench_ch);
    return FALSE;
  }

  if( ch==ench_ch && chance(50) )
  {
    if( cmd == CMD_CAST || cmd == CMD_SONG || cmd == CMD_RECITE)
    {
      act("$n's teeth chatter as $e tries to speak some magical words.", FALSE,ench_ch,0,0,TO_ROOM);
      act("Your teeth chatter from coldness, choking off your words.", FALSE,ench_ch,0,0,TO_CHAR);
      WAIT_STATE(ench_ch, 1*PULSE_VIOLENCE);
      return TRUE;
    }
  }

  return FALSE;
}

// Block north movement while alive. Every mob act, freeze a different member of the party.
int mus_naiad(CHAR *mob, CHAR* ch, int cmd, char *arg )
{
  if( mob && cmd == CMD_NORTH )
  {
    mob_do( mob, "disagree" );
    return TRUE;
  }

  if( cmd == MSG_DIE )
  {
    // People who recalled before she died get cowardice enchantment
    // If she was equipping staff, everyone in room gets a chance of petrify. Younger
    //   people have a better chance to avoid petrification.
    struct descriptor_data *d;

    bool stone = EQ(mob,HOLD) != NULL;
    if( stone )
    {
      send_to_room("\r\nYour eyes dart to the Icy Staff as it slips from the Naiad's grasp.\r\n", CHAR_REAL_ROOM(mob) );
      obj_to_room(unequip_char(mob,HOLD), CHAR_REAL_ROOM(mob ) );
    }
    for (d = descriptor_list; d; d = d->next)
      if (d->character && enchanted_by( d->character, ENCH_NAIAD_CHILLING ) )
      {
        ENCH* to_remove = find_enchantment(d->character, ENCH_NAIAD_CHILLING );
        enchantment_remove(d->character, to_remove, 0);

        if( CHAR_REAL_ROOM(d->character) != CHAR_REAL_ROOM(mob) )
        {
          send_to_char("You are filled with shame at your cowardice.\r\n", d->character );
          ENCH *tmp_enchantment;
          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup( ENCH_NAIAD_COWARDICE );
          tmp_enchantment->duration = 30;            /* Define the enchantment */
          tmp_enchantment->func     = ench_naiad_cowardice;
          enchantment_to_char(d->character, tmp_enchantment, FALSE);
        }
        else
        {
          if(GET_LEVEL(d->character) < LEVEL_IMM && chance(GET_AGE(d->character)))
          {
            spell_petrify( GET_LEVEL(mob), mob, d->character, NULL );
          }
        }
      }
    return FALSE;
  }

  CHAR *vict = NULL;
  if( cmd == MSG_MOBACT )
  {
    for(vict = world[mob->in_room_r].people; vict; vict = vict->next_in_room)
    {
      if( !IS_NPC(vict) && !enchanted_by( vict, ENCH_NAIAD_CHILLING ) )
      {
          ENCH *tmp_enchantment;
          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup( ENCH_NAIAD_CHILLING );
          tmp_enchantment->duration = 20;            /* Define the enchantment */
          tmp_enchantment->func = ench_naiad_chilling;
          enchantment_to_char(vict, tmp_enchantment, FALSE);
      }
    }
  }

  if( mob && mob->specials.fighting )
  {
    if( cmd == MSG_MOBACT && chance(67) )
    {
      CHAR *victim = get_random_victim( mob );
      if( victim && GET_LEVEL(victim) < LEVEL_IMM && !IS_NPC(victim) && !enchanted_by(victim, ENCH_NAIAD_FROZEN))
      {
        if( chance(33))
        {
          act ("$n exhales an icy stream of vapour at you. How invigorating!",
               FALSE, mob, 0, victim, TO_VICT);
          act ("$n exhales an invigorating blast of icy vapour at $N.",
               FALSE, mob, 0, victim, TO_NOTVICT);
          act ("You breathe frost all over $N, to little effect.",
               FALSE, mob, 0, victim, TO_CHAR);
        }
        else
        {
          act ("$n exhales a blast of frost all over you, freezing your limbs instantly upon contact!",
               FALSE, mob, 0, victim, TO_VICT);
          act ("$n freezes $N with an icy blast of frost!",
               FALSE, mob, 0, victim, TO_NOTVICT);
          act ("You breathe frost all over $N, freezing them in place!",
               FALSE, mob, 0, victim, TO_CHAR);

          ENCH *tmp_enchantment;
          CREATE(tmp_enchantment, ENCH, 1);
          tmp_enchantment->name     = str_dup( ENCH_NAIAD_FROZEN );
          tmp_enchantment->duration = 2;            /* Define the enchantment */
          tmp_enchantment->func     = ench_naiad_frozen;
          enchantment_to_char(victim, tmp_enchantment, FALSE);
        }
      }
    }
  }


  return FALSE;
}

#define NUM_GROUPIES 10

// You can use this ram on the wall in Hakeem`s room to knock it down. Requires 10 PCs
// to sacrifice 2M XP each.
int mus_battering_ram(OBJ *ram, CHAR *ch,int cmd,char *arg)
{
  // Must be character holding the item who calls use.
  if( !ch || !ram || cmd != CMD_USE || EQ( ch, HOLD ) != ram )
    return FALSE;

  char buf[MAX_INPUT_LENGTH];

  arg = one_argument( arg, buf );
  if( !strcasecmp( buf, " ram" ) )
  {
    send_to_char( "Use what?\r\n", ch );
    return TRUE;
  }

  if( strlen(arg) == 0 )
  {
    send_to_char( "Use the ram on what?\r\n", ch );
    return TRUE;
  }
  else if( !strcasecmp( arg, "wall" ) )
  {
    send_to_char( "I'm not sure how you would do that.\r\n", ch );
    return TRUE;
  }
  else if( CHAR_VIRTUAL_ROOM( ch ) != MUS_HAKEEM_ROOM )
  {
    send_to_char( "I'm not sure how you would do that.\r\n", ch );
    return TRUE;
  }
  else if( !IS_SET(EXIT(ch, SOUTH)->exit_info, EX_LOCKED))
  {
    send_to_char("What wall?\r\n", ch );
    return TRUE;
  }

  //Need to have 10 people, each with 2 M XP.
  CHAR* groupies[NUM_GROUPIES];
  int i;
  for(i = 0; i < NUM_GROUPIES; i++)
    groupies[i] = NULL;

  if( !IS_AFFECTED( ch, AFF_GROUP ) )
  {
    send_to_char( "You feel like it would take a group of people to use it.\r\n", ch );
    return TRUE;
  }

  if( GET_EXP( ch ) < 2000000 )
  {
    send_to_char( "You don't feel like someone of your limited experience can use that correctly.\r\n", ch );
    act ("$n ponders the battering ram in $s hand.", FALSE, ch, 0, ch, TO_NOTVICT );
    return TRUE;
  }

  CHAR *master = ch->master;
  if( !master )
    master = ch;

  struct follow_type *f = NULL;

  groupies[0] = ch;
  i = 1;

  int num_groupies = 1;
  for( f = master->followers; f; f = f->next )
  {
    if( f->follower == ch )
      continue;

    if( CHAR_VIRTUAL_ROOM( f->follower ) != CHAR_VIRTUAL_ROOM( ch ) )
      continue;

    if( !IS_AFFECTED( f->follower, AFF_GROUP ) )
      continue;

    num_groupies++;
    if( GET_EXP( f->follower ) < 2000000 )
      continue;

    groupies[i] = f->follower;
    i++;

    if( i == NUM_GROUPIES )
      break;
  }

  if( i < NUM_GROUPIES )
  {
    if( num_groupies < NUM_GROUPIES )
    {
      send_to_char( "You feel like it would take a bigger group of people to use it.\r\n", ch );
      return TRUE;
    }

    send_to_char( "You don't feel like your group has enough experience to use that correctly.\r\n", ch );
    return TRUE;
  }

  for( i = 0; i < NUM_GROUPIES; i++ )
  {
    GET_EXP( groupies[i] ) -= 2000000;
    act("You heave the ram against the wall until it crumbles in a shower of waxen fragments!",
        FALSE, groupies[i], 0, 0, TO_CHAR);
    WAIT_STATE(groupies[i], 3*PULSE_VIOLENCE);
  }

  CHAR *vict = NULL;
  for(vict = world[ch->in_room_r].people; vict; vict = vict->next_in_room)
  {
    bool is_grouped = FALSE;
    for( i = 0; i < NUM_GROUPIES; i++ )
    {
      if( vict == groupies[i] )
        is_grouped = TRUE;
    }

    if( !is_grouped )
    {
      act("Your group members batter the wall into a shower of waxen fragments!",
        FALSE, vict, 0, 0, TO_CHAR);
    }
  }

  REMOVE_BIT(EXIT(ch, SOUTH)->exit_info, EX_LOCKED);
  REMOVE_BIT(EXIT(ch, SOUTH)->exit_info, EX_CLOSED);
  free_encased_in_wax();
  return TRUE;
}

void
assign_elmuseo (void) {
  // Rooms
  assign_room( MUS_DROWNING, mus_drowning );
  //assign_room( MUS_RED_MIST, mus_red_mist );  // <-- chained from mus_arena_rooms
  assign_room( MUS_HAKEEM_ROOM, mus_hakeem_room );
  assign_room( MUS_HALLWAY_NORTH_OF_WYVERN, mus_hallway_north_of_wyvern );
  assign_room( MUS_NORTH_FROM_GORGO, mus_north_from_gorgo );
  assign_room( MUS_UP_FROM_HAKEEM, mus_up_from_hakeem );
  assign_room( LA_SOUDURE_DAIMYO_EAST, mus_la_soudure_east );
  assign_room( MUS_ENCASED_IN_WAX, mus_encased_in_wax );
  assign_room( MUS_SANCTUARY, mus_sanctuary );

  // Arena rooms.
  int i;
  for( i = 16528; i <= 16567; i++ )
    assign_room( i, mus_arena_rooms );


  // Mobs
  assign_mob(MUS_BILL,  mus_bill);
  assign_mob(MUS_FAZE,  mus_faze);
  assign_mob(MUS_JENNY,  mus_jenny);
  assign_mob(MUS_GORGO,  mus_gorgo);
  assign_mob(MUS_HAKEEM, mus_hakeem);
  assign_mob(MUS_HAKEEM_GHOST, mus_hakeem_ghost );
  assign_mob(MUS_CURATOR, mus_curator );
  assign_mob(MUS_FIRE, mus_djinn );
  assign_mob(MUS_VAPOR, mus_djinn );
  assign_mob(MUS_TOMEK, mus_tomek );
  assign_mob(MUS_KHALLAQ, mus_khallaq );
  assign_mob(MUS_QUARATT, mus_quaratt );
  assign_mob(MUS_LEM, mus_lem );
  assign_mob(MUS_TIMUR, mus_timur );
  assign_mob(MUS_MAMA, mus_mama );
  assign_mob(MUS_NAIAD, mus_naiad );
  assign_mob(MUS_HORSE, mus_horse );

  assign_mob( PIRATE_CAPTAIN, mus_Captain );
  assign_mob( NERGAL_LARVAE, mus_nergal_larvae );
  assign_mob( NERGAL_BODYGUARD, mus_nergal_bodyguard );
  assign_mob( MAR_GHOST, mus_mar_ghost );

  // Objects
  assign_obj( MUS_WAX_POT_HAKEEM, mus_hakeem_pot );
  assign_obj( MUS_WAX_POT_PC, mus_pc_pot );
  assign_obj( MUS_KEYCHAIN, mus_keychain );
  assign_obj( MUS_CLOAK, mus_cloak );
  assign_obj( MUS_LEM_CLOAKS1, mus_cloak_lem );
  assign_obj( MUS_LEM_CLOAKS2, mus_cloak_lem );
  assign_obj( MUS_LEM_CLOAKS3, mus_cloak_lem );
  assign_obj( MUS_BATTERING_RAM, mus_battering_ram );
  assign_obj( MUS_LEAF_SPEAR, mus_leaf_spear );
  assign_obj( MUS_MUKLUKS, mus_mukluks );
  assign_obj( MUS_TOMTOM, mus_tomtom );
  assign_obj( MUS_KIMAGEE_HIDE, mus_kimagee_hide );
  assign_obj( MUS_KIMAGEE_HIDE_B, mus_kimagee_hide );
  assign_obj( MUS_ELAR_SKULL, mus_elar_skull );
  assign_obj( MUS_ANTLERS, mus_antlers );
  assign_obj( MUS_MITTENS, mus_mittens );
  assign_obj( MUS_BLACK_LEGGINGS, mus_black_leggings );
  assign_obj( MUS_WHITE_BAND, mus_white_band );
}


