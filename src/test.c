 #include <stdio.h>
 #include <string.h>
 #include <ctype.h>
 #include <stdlib.h>
 #include <unistd.h>

 #include "structs.h"
 #include "utils.h"
 #include "comm.h"
 #include "interpreter.h"
 #include "handler.h"
 #include "db.h"
 #include "spells.h"
 #include "limits.h"
 #include "cmd.h"
 #include "utility.h"
 #include "fight.h"
 #include "act.h"
 #include "reception.h"
 #include "spec_assign.h"
 #include "mob.spells.h"
 /*   external vars  */
extern CHAR *character_list;
extern is_caster(CHAR *CH);
extern struct descriptor_data *descriptor_list;
extern struct spell_info_type spell_info[MAX_SPL_LIST];


void main() {
  printf("obj_file_elem_ver3: %d\n", sizeof(struct obj_file_elem_ver3));

}
