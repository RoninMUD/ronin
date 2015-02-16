/* ========================================================================== */
/*                                                                            */
/*   spec.workbench.c                                                         */
/*   (c) 2013 Parashurama                                                     */
/*                                                                            */
/*   This spec will combine three herbs placed on the workbench               */
/*   to a potion based on which herbs are used                                */
/*                                                                            */
/* ========================================================================== */

/* Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"

#include "act.h"
#include "cmd.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spec_assign.h"

/* Definitoins */

#define WORKBENCH      18900

#define HERB1          18901
#define HERB2          18902
#define HERB3          18903
#define HERB4          18904
#define HERB5          18905
#define HERB6          18906
#define HERB7          18907
#define HERB8          18908
#define HERB9          18909
#define HERB10         18910
#define HERB11         18911
#define HERB12         18912
#define HERB13         18913
#define HERB14         18914
#define HERB15         18915
#define HERB16         18916
#define HERB17         18917
#define HERB18         18918
#define HERB19         18919
#define HERB20         18920


#define FHERB1         0x00000001
#define FHERB2         0x00000002
#define FHERB3         0x00000004
#define FHERB4         0x00000008
#define FHERB5         0x00000010
#define FHERB6         0x00000020
#define FHERB7         0x00000040
#define FHERB8         0x00000080
#define FHERB9         0x00000100
#define FHERB10        0x00000200
#define FHERB11        0x00000400
#define FHERB12        0x00000800
#define FHERB13        0x00001000
#define FHERB14        0x00002000
#define FHERB15        0x00004000
#define FHERB16        0x00008000
#define FHERB17        0x00010000
#define FHERB18        0x00020000
#define FHERB19        0x00040000
#define FHERB20        0x00080000


#define WEAKBLESS                18999
#define NORMALBLESS              18998
#define POWERFULBLESS            18997

#define WEAKDETECT               18996
#define NORMALDETECT             18995
#define POWERFULDETECT           18994

#define WEAKARMAMENT             18993
#define NORMALARMAMENT           18992
#define POWERFULARMAMENT         18991

#define WEAKPURIFICATION         18990
#define NORMALPURIFICATION       18989
#define POWERFULPURIFICATION     18988

#define WEAKPROTECTION           18987
#define NORMALPROTECTION         18986
#define POWERFULPROTECTION       18985

#define WEAKSPECTRAL             18984
#define NORMALSPECTRAL           18983
#define POWERFULSPECTRAL         18982

#define WEAKMIGHTY               18981
#define NORMALMIGHTY             18980
#define POWERFULMIGHTY           18979

#define WEAKFLOATING             18978
#define NORMALFLOATING           18977
#define POWERFULFLOATING         18976

#define WEAKARROGANT             18975
#define NORMALARROGANT           18974
#define POWERFULARROGANT         18973

#define WEAKLIGHT                18972
#define NORMALLIGHT              18971
#define POWERFULLIGHT            18970

#define WEAKDARK                 18969
#define NORMALDARK               18968
#define POWERFULDARK             18967

#define WEAKHEALING              18966
#define NORMALHEALING            18965
#define POWERFULHEALING          18964

#define WEAKMAGIC                18963
#define NORMALMAGIC              18962
#define POWERFULMAGIC            18961

#define WEAKDEFENCIVE            18960
#define NORMALDEFENCIVE          18959
#define POWERFULDEFENCIVE        18958

#define WEAKSOLID                18957
#define NORMALSOLID              18956
#define POWERFULSOLID            18955

#define WEAKROARING              18954
#define NORMALROARING            18953
#define POWERFULROARING          18952

#define WEAKSHADOWY              18951
#define NORMALSHADOWY            18950
#define POWERFULSHADOWY          18949

#define WEAKDIVINE               18948
#define NORMALDIVINE             18947
#define POWERFULDIVINE           18946

#define WEAKPIERCING             18945
#define NORMALPIERCING           18944
#define POWERFULPIERCING         18943


int spec_workbench(OBJ *workbench,CHAR *ch,int cmd,char *argument)
{
    int herbs[] = {0, 0, 0}; /* used to hold the VNUMS of the three herbs used */
    int i = 0, combo = 0, clear = FALSE;
    OBJ *obj = NULL, *obj_next = NULL;
    char arg[MIL];

    if (cmd==CMD_UNKNOWN) {                               //craft potion trigger

        argument = one_argument (argument, arg);

        if (!*arg) return FALSE;                          //Check that a command is entered
        if (!is_abbrev(arg,"craft")) return FALSE;        //Check that "craft" CMD is entered

        argument = one_argument (argument, arg);

        if (!*arg) {                                      //Check that there is entered word after "craft"
           send_to_char("Craft what?\n\r",ch);
           return TRUE;
        }


        if(!strcmp(arg,"potion")) { // Check that second argument is "potion"

          /* Putting the herb VNUMS in the herbs[] array. */
          for (obj = workbench->contains; obj && i < 3; obj = obj_next) {
              herbs[i++] = V_OBJ(obj);
              obj_next = obj->next_content;
          }

          if (i == 3) {
              /* Checking what herbs are combined */
              for (i = 0; i < 3; i++) {
                  switch(herbs[i]) {
                      case(HERB1):
                          combo |= FHERB1;
                          break;
                      case(HERB2):
                          combo |= FHERB2;
                          break;
                      case(HERB3):
                          combo |= FHERB3;
                          break;
                      case(HERB4):
                          combo |= FHERB4;
                          break;
                      case(HERB5):
                          combo |= FHERB5;
                          break;
                      case(HERB6):
                          combo |= FHERB6;
                          break;
                      case(HERB7):
                          combo |= FHERB7;
                          break;
                      case(HERB8):
                          combo |= FHERB8;
                          break;
                      case(HERB9):
                          combo |= FHERB9;
                          break;
                      case(HERB10):
                          combo |= FHERB10;
                          break;
                      case(HERB11):
                          combo |= FHERB11;
                          break;
                      case(HERB12):
                          combo |= FHERB12;
                          break;
                      case(HERB13):
                          combo |= FHERB13;
                          break;
                      case(HERB14):
                          combo |= FHERB14;
                          break;
                      case(HERB15):
                          combo |= FHERB15;
                          break;
                      case(HERB16):
                          combo |= FHERB16;
                          break;
                      case(HERB17):
                          combo |= FHERB17;
                          break;
                      case(HERB18):
                          combo |= FHERB18;
                          break;
                      case(HERB19):
                          combo |= FHERB19;
                          break;
                      case(HERB20):
                          combo |= FHERB20;
                          break;
                      
                  }
              }

              /* Crafting the Blessing potion combining herbs 1,2 and 6 */
              switch (combo) {

                case (FHERB1 | FHERB2 | FHERB6):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKBLESS, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALBLESS, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULBLESS, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
              /* Crafting the Detecting potion combining herbs 1,4 and 5 */
              case (FHERB1 | FHERB4 | FHERB5):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKDETECT, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALDETECT, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULDETECT, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
              
              
              /* Crafting the Armament potion combining herbs 1,6 and 7 */
              
                case (FHERB1 | FHERB6 | FHERB7):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKARMAMENT, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALARMAMENT, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULARMAMENT, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
              /* Crafting the Purification potion combining herbs 1,3 and 8 */
              case (FHERB1 | FHERB3 | FHERB8):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKPURIFICATION, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALPURIFICATION, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULPURIFICATION, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                
                /* Crafting the Mighty potion combining herbs 1,5 and 7 */              
                case (FHERB1 | FHERB5 | FHERB7):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKMIGHTY, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALMIGHTY, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULMIGHTY, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
                /* Crafting the Floating potion combining herbs 1,3 and 4 */              
                case (FHERB1 | FHERB3 | FHERB4):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKFLOATING, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALFLOATING, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULFLOATING, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
                /* Crafting the Spectral potion combining herbs 1,2 and 8 */              
                case (FHERB1 | FHERB2 | FHERB8):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKSPECTRAL, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALSPECTRAL, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULSPECTRAL, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
                /* Crafting the Protection potion combining herbs 1,9 and 13 */              
                case (FHERB1 | FHERB9 | FHERB13):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKPROTECTION, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALPROTECTION, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULPROTECTION, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
                /* Crafting the Arrogant potion combining herbs 2,9 and 13 */              
                case (FHERB2 | FHERB9 | FHERB13):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKARROGANT, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALARROGANT, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULARROGANT, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
                /* Crafting the Light potion combining herbs 3,10 and 14 */              
                case (FHERB3 | FHERB10 | FHERB14):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKLIGHT, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALLIGHT, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULLIGHT, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
                /* Crafting the Dark potion combining herbs 4,10 and 14 */              
                case (FHERB4 | FHERB10 | FHERB14):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKDARK, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALDARK, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULDARK, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
              /* Crafting the Healing potion combining herbs 5,11 and 15 */              
                case (FHERB5 | FHERB11 | FHERB15):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKHEALING, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALHEALING, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULHEALING, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
              /* Crafting the Magic potion combining herbs 6,11 and 15 */              
                case (FHERB6 | FHERB11 | FHERB15):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKMAGIC, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALMAGIC, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULMAGIC, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
              /* Crafting the Piercing potion combining herbs 7,12 and 16 */              
                case (FHERB7 | FHERB12 | FHERB16):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKPIERCING, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALPIERCING, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULPIERCING, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                 
              /* Crafting the Defencive potion combining herbs 8,12 and 16 */              
                case (FHERB8 | FHERB12 | FHERB16):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKDEFENCIVE, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALDEFENCIVE, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULDEFENCIVE, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
              /* Crafting the Defencive potion combining herbs 9,13 and 17 */              
                case (FHERB9 | FHERB13 | FHERB17):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKSOLID, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALSOLID, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULSOLID, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                 
              /* Crafting the Roaring potion combining herbs 10,14 and 18 */              
                case (FHERB10 | FHERB14 | FHERB18):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKROARING, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALROARING, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULROARING, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
              /* Crafting the Shadowy potion combining herbs 11,15 and 19 */              
                case (FHERB11 | FHERB15 | FHERB19):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKSHADOWY, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALSHADOWY, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULSHADOWY, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                 
              /* Crafting the Divine potion combining herbs 12,16 and 20 */              
                case (FHERB12 | FHERB16 | FHERB20):

                  clear = TRUE;

                  switch(number(0,9)) {
                    case(0):
                    case(1):
                    case(2):
                    case(3):
                    case(4):
                         obj = read_object(WEAKDIVINE, VIRTUAL);
                         act("You succesfully made a weak potion!",0,ch,0,0,TO_CHAR);
                         break;
                    case(5):
                    case(6):
                    case(7):
                        obj = read_object(NORMALDIVINE, VIRTUAL);
                        act("You succesfully made a potion!",0,ch,0,0,TO_CHAR);
                        break;
                    case(8):
                    case(9):
                        obj = read_object(POWERFULDIVINE, VIRTUAL);
                        act("You succesfully made a powerful potion!",0,ch,0,0,TO_CHAR);
                        break;
                  } /* end switch */

                  obj_to_char(obj,ch);

                  break;
                  
                  
              } /* END switch (combo) */
              
          } /* END if workbench contains 3 items */

          if (clear) {
            for (obj = workbench->contains, i = 0; obj && i < 3; obj = obj_next, i++) {
                obj_next = obj->next_content;
                extract_obj(obj);
            }
          }
          else {
            act("Nothing happens.",0,ch,0,0,TO_CHAR);
          }

        } /* END if(!strcmp(arg,"potion")) */
        else {
           act("You don't know how to craft that.",0,ch,0,0,TO_CHAR);
        }

        return TRUE;
      }  /* END if(cmd==CMD_UNKNOWN) */

   return FALSE;
} /* END SPEC */

void assign_workbench (void) {
  assign_obj(WORKBENCH, spec_workbench);
}
