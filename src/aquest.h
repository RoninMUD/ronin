/*
  aquest.h - Autoquest related functions

  Written by Shun for RoninMUD
*/

#ifndef _AQUEST_H_
#define _AQUEST_H_

#include "structs.h"

bool is_order_item(int vnum);

/* Flat cooldown for quest interruptions that aren't the player's fault
   (death, quest mob/guildmaster killed by someone else). */
#define AQ_INNOCENT_COOLDOWN 2

void aq_fail_quest(CHAR *ch, int new_status);
void aq_fail_quest_flat(CHAR *ch, int new_status, int cooldown);

#endif /* _AQUEST_H_ */
