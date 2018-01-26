/***************************************************************************
 *  file: spec_assign.c , Special module.                  Part of DIKUMUD *
 *  Usage: Procedures assigning function pointers.                         *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include "structs.h"
#include "db.h"
#include "utility.h"
#include "utils.h"
#include "spec_assign.h"

void assign_the_shopkeepers();

/* ********************************************************************
 *  Assignments                                                        *
 ******************************************************************** */

/* assign special procedures to mobiles */

void assign_mob(int virtual, int (*func)(CHAR*, CHAR*, int, char*))
{
   int i;
   i = real_mobile(virtual);
   if(i!= -1)
     mob_proto_table[i].func = func;
}

void assign_obj(int virtual, int (*func)(OBJ*, CHAR*, int, char*))
{
   int i;
   i = real_object(virtual);
   if(i!= -1)
     obj_proto_table[i].func=func;
}

void assign_room(int virtual, int (*func)(int, CHAR*, int, char*))
{
   int i;
   i = real_room(virtual);
   if(i!= -1)
      world[i].funct=func;
}

void assign_monkey(void);
void assign_pagoda(void);
void assign_helventia(void);
void assign_nergal(void);
void assign_sphinx(void);
void assign_trollvillage(void);
void assign_trollcastle_moat(void);
void assign_monastery(void);
void assign_marikith(void);
void assign_elf(void);
void assign_topknot(void);
void assign_chaos(void);
void assign_wolf(void);
void assign_zyca(void);
void assign_welmar(void);
void assign_misc(void);
void assign_barovia(void);
void assign_grail(void);
void assign_pirate(void);
void assign_house(void);
void assign_abyss(void);
void assign_wyvern(void);
void assign_quests(void);
void assign_elmuseo(void);
void assign_stables(void);
void assign_druid(void);
void assign_lostworld(void);
void assign_desert(void);
void assign_cafeteria(void);
void assign_boat(void);
void assign_wsewer(void);
void assign_dasharr(void);
void assign_jungle(void);
void assign_theldon(void);
void assign_labyrinth(void);
void assign_daimyo(void);
void assign_goblin(void);
void assign_ctower(void);
void assign_clan(void);
void assign_midgaard(void);
void assign_vanity(void);
void assign_vermillion(void);
void assign_eryndlyn(void);
void assign_moria(void);
void assign_subclass(void);
void assign_feach(void);
void assign_newt(void);
void assign_gamble(void);
void assign_cavern(void);
void assign_zoo(void);
void assign_golden_plaza(void);
void assign_rank_objects(void);
void assign_newspaper(void);
void assign_enchanted(void);
void assign_underworld(void);
void assign_keening(void);
void assign_tarioncity(void);
void assign_deathplay(void);
void assign_hell(void);
void assign_invasion(void);
void assign_emith(void);
void assign_questyvaderII(void);
void assign_newbie(void);
void assign_questgearII(void);
void assign_meta(void);
void assign_remortv2(void);
void assign_ershteep(void);
void assign_remorhaz(void);
void assign_norway(void);
void assign_zan(void);
void assign_dionysus(void);
void assign_haondor(void);
void assign_mapmaker(void);
void assign_vagabond(void);
void assign_red_dragons(void);
void assign_tweefmanor(void);
void assign_aquest_special(void);
void assign_swordoftruth(void);
void assign_WOT(void);
//void assign_wbw(void);
#ifdef TEST_SITE
void assign_questyvaderIV(void);
void assign_digsite(void);
void assign_luthienIV(void);
void assign_workbench(void);
void assign_ecanyon(void);
#endif

void assign_mobiles(void)
{
  assign_monkey();
  assign_pagoda();
  assign_helventia();
  assign_nergal();
  assign_sphinx();
  assign_trollvillage();
  assign_trollcastle_moat();
  assign_monastery();
  assign_marikith();
  assign_elf();
  assign_topknot();
  assign_chaos();
  assign_wolf();
  assign_zyca();
  assign_welmar();
  assign_misc();
  assign_barovia();
  assign_grail();
  assign_pirate();
  assign_house();
  assign_abyss();
  assign_wyvern();
  assign_quests();
  assign_elmuseo();
  assign_stables();
  assign_druid();
  assign_lostworld();
  assign_desert();
  assign_cafeteria();
  assign_boat();
  assign_wsewer();
  assign_dasharr();
  assign_jungle();
  assign_theldon();
  assign_labyrinth();
  assign_daimyo();
  assign_goblin();
  assign_ctower();
  assign_clan();
  assign_midgaard();
  assign_vanity();
  assign_vermillion();
  assign_eryndlyn();
  assign_moria();
  assign_cavern();
  assign_feach();
  assign_newt();
  assign_the_shopkeepers();
  assign_subclass();
  assign_gamble();
  assign_rank_objects();
  assign_newspaper();
  assign_zoo();
  assign_golden_plaza();
  assign_enchanted();
  assign_underworld();
  assign_keening();
  assign_tarioncity();
  assign_deathplay();
  assign_hell();
  assign_invasion();
  assign_emith();
  assign_questyvaderII();
  assign_newbie();
  assign_questgearII();
  assign_meta();
  assign_remortv2();
  assign_ershteep();
  assign_haondor();
  assign_remorhaz();
  assign_norway();
  assign_zan();
  assign_dionysus();
  assign_mapmaker();
  assign_vagabond();
  assign_red_dragons();
  assign_tweefmanor();
  assign_aquest_special();
  assign_swordoftruth();
  assign_WOT();
//  assign_wbw();
#ifdef TEST_SITE
  assign_digsite();
  assign_luthienIV();
  assign_workbench();
  assign_ecanyon();
#endif
}

/* assign special procedures to objects */
void assign_objects(void)
{
  int vote (OBJ*, CHAR*, int, char*);
  assign_obj(15,vote);
}

/* assign special procedures to rooms */
void assign_rooms(void)

{
#ifdef TEST_SITE
  //assign_questyvaderIV();
#endif
}
