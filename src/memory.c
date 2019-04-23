/**************************************************************************
*  File: memory.c                                         Part of DikuMud *
*  Usage: Memory for MOBs.						  *
*  Author : Dionysos, the God of Wine. 					  *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

/*
$Author: ronin $
$Date: 2004/02/05 16:09:45 $
$Header: /home/ronin/cvs/ronin/memory.c,v 2.0.0.1 2004/02/05 16:09:45 ronin Exp $
$Id: memory.c,v 2.0.0.1 2004/02/05 16:09:45 ronin Exp $
$Name:  $
$Log: memory.c,v $
Revision 2.0.0.1  2004/02/05 16:09:45  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"

/*******************************************************************/
/* FUNCTIONS                                                       */
/*******************************************************************/

void remember (struct char_data *ch, struct char_data *mob) {
   memory_record_t *new, *curr;
   bool present = FALSE;

   if(IS_NPC(ch)) return;
   /* See if the person is already in the list (no duplicates) */

   curr = (memory_record_t *)mob->specials.memory;
   while (curr) {
      if (ch->ver3.id==curr->id) {
         present = TRUE;
         break;
      }
      else {
         curr = curr->next;
      }
   }

   if (!present) {
      CREATE(new, memory_record_t, 1);
      new->next = mob->specials.memory;
      new->id = (int)ch->ver3.id;
      mob->specials.memory = new;
   }

} /* remember */

/*******************************************************************/

void forget (struct char_data *ch, struct char_data *mob)
{
   memory_record_t *prev, *curr;

   prev = (memory_record_t *)mob->specials.memory;
   curr = prev;

   while ((curr) && curr->id!=ch->ver3.id) {
      prev = curr;
      curr = curr->next;
   }

   if (curr == NULL) return;
   if (curr == mob->specials.memory) {
      mob->specials.memory = curr->next;
   }
   else {
      prev->next = curr->next;
   }

   free(curr);
} /* forget */

/********************************************************************/

void clearMemory(struct char_data *ch)
{
   memory_record_t *curr, *prev;

   prev = (memory_record_t *)ch->specials.memory;

   while (prev) {
      curr = prev->next;
      free(prev);
      prev = curr;
   }

   ch->specials.memory = NULL;
} /* clearMemory */


