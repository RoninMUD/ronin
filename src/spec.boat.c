/*
** spec.boat.c  - Specials for "Sailing Vessel, by Thyas"
**
** Written for RoninMUD by Thyas
**
** Date:            October 3, 1996
** Last Modified:   October 26, 1996
**
** Using this code is not allowed without permission from originator.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"

#include "utility.h"
#include "cmd.h"
#include "spec_assign.h"

#define SAILING_CAPTAIN        7601  /* captain of the boat */
#define default_height         198   /* captains default height */
#define BOAT                   7601
#define BY_LEVEE               3049  /* Levee */
#define BY_WOLFCAVE            16670 /* outside Wolf Cave */
#define BY_DROWCITY            16671 /* Drow City */
#define BY_STNMNKY             7241  /* Stone Monkey Island */
#define BY_DAIMYO               500

#define STOP_LEVEE        1
#define LEAVE_LEVEE       3
#define STOP_WOLFCAVE     5
#define LEAVE_WOLFCAVE    7
#define STOP_DROWCITY     9
#define LEAVE_DROWCITY    11
#define STOP_STONEMNKY    17
#define LEAVE_STONEMNKY   19
#define STOP_DAIMYO       25
#define LEAVE_DAIMYO      27
#define STOP_STONEMNKY2   33 /* return trip */
#define LEAVE_STONEMNKY2  35 /* return trip */
#define STOP_DROWCITY2    41 /* return trip */
#define LEAVE_DROWCITY2   43 /* return trip */
#define STOP_WOLFCAVE2    45 /* return trip */
#define LEAVE_WOLFCAVE2   47 /* return trip */

const char *position[]= {
  "to the Wolf Cave from the Levee",
  "to Old Drow City from the Wolf Cave",
  "to Stone Monkey Island from Old Drow City",
  "to the Daimyo from Stone Monkey Island",
  "to Stone Monkey Island from the Daimyo",
  "to Old Drow City from Stone Monkey Island",
  "to the Wolf Cave from Old Drow City",
  "to the Levee from the Wolf Cave",
  "\n"
};

int sailing_vessel(CHAR *captain, CHAR *ch, int cmd, char *arg) {
   char buf[MAX_STRING_LENGTH / 2];
   char buf2[MAX_STRING_LENGTH];

   if(!AWAKE(captain))return FALSE;
   if(CHAR_REAL_ROOM(captain) != real_room(BOAT)) return FALSE;

   switch (cmd) {
    case MSG_MOBACT:

      captain->specials.timer++;

      if (captain->specials.timer < STOP_STONEMNKY) { captain->specials.timer = STOP_STONEMNKY; }

      if ((((captain->specials.timer+4)%4) == 0)) {
        sprintf(buf,"The boat is currently enroute %s.\n\r",position[captain->player.height-default_height]);
        sprintf(buf2,"The Old Captain says '%s'.\n\r",buf);
        send_to_room(buf2,real_room(BY_LEVEE));
        //send_to_room(buf,real_room(BY_WOLFCAVE));
        //send_to_room(buf,real_room(BY_DROWCITY));
        send_to_room(buf,real_room(BY_STNMNKY));
        send_to_room(buf,real_room(BY_DAIMYO));
        break;
      }
      switch(captain->specials.timer) {

        case STOP_LEVEE:
          send_to_room("A sturdy looking wooden sailing vessel pulls up along side the dock.\n\r", real_room(BY_LEVEE));
          world[real_room(BOAT)].dir_option[NORTH]->to_room_r = real_room(BY_LEVEE);
          send_to_room("Your vessel pulls up along side the dock at the Levee.\n\r",real_room(BOAT));
          world[real_room(BY_LEVEE)].dir_option[SOUTH]->to_room_r = real_room(BOAT);
          act("$n says 'Batten down the hatches and hold on laddies, she's no smooth ride.'",FALSE,captain,0,0,TO_ROOM);
          break;

        case LEAVE_LEVEE:
          send_to_room("A large wooden vessel pulls away from the dock.\n\r",real_room(BY_LEVEE));
          world[real_room(BOAT)].dir_option[NORTH]->to_room_r = -1;
          send_to_room("Your vessel slowly pulls away from the dock at the Levee.\n\r",real_room(BOAT));
          world[real_room(BY_LEVEE)].dir_option[SOUTH]->to_room_r = real_room(16645);
          act("$n says 'I hope ye got yer sea legs on ye bloody hangashores, cause the ride is rough.'",FALSE,captain,0,0,TO_ROOM);
          act("$n says 'With these winds we'll be at the Wolf Cave in no time.'",FALSE,captain,0,0,TO_ROOM);
          captain->player.height = 198;
          break;

        case STOP_WOLFCAVE:
          send_to_room("A sturdy looking wooden sailing vessel pulls up along side the dock.\n\r", real_room(BY_WOLFCAVE));
          world[real_room(BOAT)].dir_option[EAST]->to_room_r = real_room(BY_WOLFCAVE);
          send_to_room("Your vessel pulls up along side the dock near the Wolf Cave.\n\r",real_room(BOAT));
          world[real_room(BY_WOLFCAVE)].dir_option[WEST]->to_room_r = real_room(BOAT);
          act("$n says 'This is as close to the Wolf Cave as I can bring ye.'",FALSE,captain,0,0,TO_ROOM);
          act("$n says 'Good luck on yer journeys, I will return to this spot in a bit.'",FALSE,captain,0,0,TO_ROOM);
          break;

        case LEAVE_WOLFCAVE:
          send_to_room("A large wooden vessel pulls away from the dock.\n\r",real_room(BY_WOLFCAVE));
          world[real_room(BY_WOLFCAVE)].dir_option[WEST]->to_room_r = real_room(16651);
          send_to_room("Your vessel slowly pulls away from the dock near the Wolf Cave.\n\r",real_room(BOAT));
          world[real_room(BOAT)].dir_option[EAST]->to_room_r = -1;
          act("$n says 'The steam to Drow City should be a swift one in these southerly gales.'",FALSE,captain,0,0,TO_ROOM);
          captain->player.height = 199;
          break;

        case STOP_DROWCITY:
          send_to_room("A sturdy looking wooden sailing vessel pulls up along side the dock.\n\r", real_room(BY_DROWCITY));
          world[real_room(BOAT)].dir_option[WEST]->to_room_r = real_room(BY_DROWCITY);
          send_to_room("Your vessel pulls up along side the dock near Old Drow City.\n\r",real_room(BOAT));
          world[real_room(BY_DROWCITY)].dir_option[EAST]->to_room_r = real_room(BOAT);
          act("'Drow City was a short trot from here, luckily them drows moved on.' says $n",FALSE,captain,0,0,TO_ROOM);
          break;

        case LEAVE_DROWCITY:
          send_to_room("A large wooden vessel pulls away from the dock.\n\r",real_room(BY_DROWCITY));
          world[real_room(BY_DROWCITY)].dir_option[EAST]->to_room_r = real_room(16654);
          send_to_room("Your vessel slowly pulls away from the dock near Old Drow City.\n\r",real_room(BOAT));
          world[real_room(BOAT)].dir_option[WEST]->to_room_r = -1;
          act("$n says 'The skys tell of a storm to come, with some luck we can\n\rbe at Stone Monkey Island before it hits.'",FALSE,captain,0,0,TO_ROOM);
          captain->player.height = 200;
          break;

        case STOP_STONEMNKY:
          send_to_room("A large wooden vessel pulls up along side the dock.\n\r",real_room(BY_STNMNKY));
          world[real_room(BY_STNMNKY)].dir_option[NORTH]->to_room_r = real_room(BOAT);
          send_to_room("Your vessel pulls up along side the dock near Stone Monkey Island.\n\r",real_room(BOAT));
          world[real_room(BOAT)].dir_option[SOUTH]->to_room_r = real_room(BY_STNMNKY);
          act("$n says 'This is as close to this blimmey place as I will venture.'",FALSE,captain,0,0,TO_ROOM);
          break;

        case LEAVE_STONEMNKY:
          send_to_room("A large wooden vessel pulls away from the dock.\n\r",real_room(BY_STNMNKY));
          world[real_room(BOAT)].dir_option[SOUTH]->to_room_r = -1;
          send_to_room("Your vessel slowly pulls away from the dock at Stone Monkey Island.\n\r",real_room(BOAT));
          world[real_room(BY_STNMNKY)].dir_option[NORTH]->to_room_r = real_room(7232);
          act("$n says 'Get aboard and get settled in mates the trip ahead is a rough one.'",FALSE,captain,0,0,TO_ROOM);
          captain->player.height = 201;
          break;

        case STOP_DAIMYO:
          send_to_room("A large wooden vessel pulls up along side the dock.\n\r",real_room(BY_DAIMYO));
          world[real_room(BY_DAIMYO)].dir_option[WEST]->to_room_r = real_room(BOAT);
          send_to_room("Your vessel pulls up along side the dock near the Daimyo.\n\r",real_room(BOAT));
          world[real_room(BOAT)].dir_option[EAST]->to_room_r = real_room(BY_DAIMYO);
          act("$n says 'Watch out, these parts are real dangerous.'",FALSE,captain,0,0,TO_ROOM);
          break;

        case LEAVE_DAIMYO:
          send_to_room("A large wooden vessel pulls away from the dock.\n\r",real_room(BY_DAIMYO));
          world[real_room(BOAT)].dir_option[EAST]->to_room_r = -1;
          send_to_room("Your vessel slowly pulls away from the dock at the Daimyo.\n\r",real_room(BOAT));
          world[real_room(BY_DAIMYO)].dir_option[WEST]->to_room_r = -1;
          act("$n says 'Get aboard, I want to get away from here as soon as possible.'",FALSE,captain,0,0,TO_ROOM);
          captain->player.height = 202;
          break;

        /* return trip begins here */

        case STOP_STONEMNKY2:
          send_to_room("A large wooden vessel pulls up along side the dock.\n\r",real_room(BY_STNMNKY));
          world[real_room(BY_STNMNKY)].dir_option[NORTH]->to_room_r = real_room(BOAT);
          send_to_room("Your vessel pulls up along side the dock near Stone Monkey Island.\n\r",real_room(BOAT));
          world[real_room(BOAT)].dir_option[SOUTH]->to_room_r = real_room(BY_STNMNKY);
          act("$n says 'Here we are again, jump out if ye wish.'",FALSE,captain,0,0,TO_ROOM);
          captain->specials.timer = STOP_STONEMNKY + 1;
          break;

        case LEAVE_STONEMNKY2:
          send_to_room("A large wooden vessel pulls away from the dock.\n\r",real_room(BY_STNMNKY));
          world[real_room(BOAT)].dir_option[SOUTH]->to_room_r = -1;
          send_to_room("Your vessel slowly pulls away from the dock at Stone Monkey Island.\n\r",real_room(BOAT));
          world[real_room(BY_STNMNKY)].dir_option[NORTH]->to_room_r = real_room(7232);
          act("$n says 'Lets get going, the wind is freshin.'",FALSE,captain,0,0,TO_ROOM);
          captain->player.height = 203;
          break;

        case STOP_DROWCITY2:
          send_to_room("A sturdy looking wooden sailing vessel pulls up along side the dock.\n\r", real_room(BY_DROWCITY));
          world[real_room(BOAT)].dir_option[WEST]->to_room_r = real_room(BY_DROWCITY);
          send_to_room("Your vessel pulls up along side the dock near Old Drow City.\n\r",real_room(BOAT));
          world[real_room(BY_DROWCITY)].dir_option[EAST]->to_room_r = real_room(BOAT);
          act("$n says 'You can get off here if you like, the drows are gone though.'",FALSE,captain,0,0,TO_ROOM);
          break;

        case LEAVE_DROWCITY2:
          send_to_room("A large wooden vessel pulls away from the dock.\n\r",real_room(BY_DROWCITY));
          world[real_room(BY_DROWCITY)].dir_option[EAST]->to_room_r = real_room(16654);
          send_to_room("Your vessel slowly pulls away from the dock near Old Drow City.\n\r",real_room(BOAT));
          world[real_room(BOAT)].dir_option[WEST]->to_room_r = -1;
          act("$n says 'She'll be a slow trip if this head wind doesn't let up a little.'",FALSE,captain,0,0,TO_ROOM);
          captain->player.height = 204;
          break;

        case STOP_WOLFCAVE2:
          send_to_room("A sturdy looking wooden sailing vessel pulls up along side the dock.\n\r", real_room(BY_WOLFCAVE));
          world[real_room(BOAT)].dir_option[EAST]->to_room_r = real_room(BY_WOLFCAVE);
          send_to_room("Your vessel pulls up along side the dock near the Wolf Cave.\n\r",real_room(BOAT));
          world[real_room(BY_WOLFCAVE)].dir_option[WEST]->to_room_r = real_room(BOAT);
          act("$n says 'Here ya go lads, the Wolf cave isn't far from here.'",FALSE,captain,0,0,TO_ROOM);
          break;

        case LEAVE_WOLFCAVE2:
          send_to_room("A large wooden vessel pulls away from the dock.\n\r",real_room(BY_WOLFCAVE));
          world[real_room(BY_WOLFCAVE)].dir_option[WEST]->to_room_r = real_room(16651);
          send_to_room("Your vessel slowly pulls away from the dock near the Wolf Cave.\n\r",real_room(BOAT));
          world[real_room(BOAT)].dir_option[EAST]->to_room_r = -1;
          act("$n says 'Next stop, the Levee.  Hope I get to see me old brother.'",FALSE,captain,0,0,TO_ROOM);
          captain->specials.timer = -1;
          captain->player.height = 205;
          /* restart trip route */
          break;
        }
      break;
    }
  return FALSE;
}

void assign_boat (void) {
  assign_mob(SAILING_CAPTAIN, sailing_vessel);
}


