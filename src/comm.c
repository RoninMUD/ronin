/*************************************************************************
*  File: comm.c , Communication module.                   Part of DIKUMUD *
*  Usage: Communication, central game loop.                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Using *any* part of DikuMud without having read license.doc is         *
*  violating our copyright.
************************************************************************* */

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <assert.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "weather.h"
#include "utility.h"
#include "modify.h"
#include "limits.h"
#include "act.h"
#include "cmd.h"
#include "slave.h"
#include "spells.h"
#include "reception.h"
#include "subclass.h"
#include "fight.h"
#include "enchant.h"
#include "aff_ench.h"
#include "char_spec.h"
#include "mcheck.h"

#define DFLT_PORT 5000        /* default port */
#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME   50

#define STATE(d) ((d)->connected)
#define LAST_COMMAND_FILE "last_command.txt"

extern int errno;    /* Why isn't this done in errno.h on alfa??? */
int special(CHAR *ch, int cmd, char *arg);
int obj_special(OBJ *obj, CHAR *ch, int cmd, char *arg);
int mob_special(CHAR *mob, CHAR *ch, int cmd, char *arg);
int room_special(int room, CHAR *ch, int cmd, char *arg);
int enchantment_special(ENCH *enchantment,CHAR *mob,CHAR *ch,int cmd,char *arg);
/* externs */

void init_descriptor (struct descriptor_data *newd, int desc);
void game_sleep(struct timeval *timeout);
void heartbeat(int pulse);
void write_last_command(void);

/* local globals */
struct descriptor_data *descriptor_list, *next_to_process;

int lawful = 0;            /* work like the game regulator */
int slow_death = 0;  /* Shut her down, Martha, she's sucking mud */
int cleanshutdown = 0;    /* clean shutdown */
int cleanreboot = 0;      /* reboot the game after a shutdown */
int disablereboot = 0;
int reboot_type=0;
int no_specials = 0; /* Suppress ass. of special routines */
int noroomdesc = 0;
int noextradesc = 0;
int uptime;
int chreboot=0;
int maxdesc=0, avail_descs;
int tics = 0;        /* for extern checkpointing */
pid_t slave_pid;
static int slave_socket = -1;
static bool fCopyOver;
int port,mother_desc;
struct timeval null_time;
char* g_datadir = NULL;

int get_from_q(struct txt_q *queue, char *dest);
/* write_to_q is in comm.h for the macro */
int run_the_game(int port);
int game_loop(int mother_desc);
int init_socket(int port);
int new_connection(int s);
int new_descriptor(int s);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void close_sockets(int s);
void close_socket(struct descriptor_data *d);
void timediff(struct timeval *diff, struct timeval *a, struct timeval *b);
void timeadd(struct timeval *sum, struct timeval *a, struct timeval *b);
void flush_queues(struct descriptor_data *d);
void nonblock(int s);
void parse_name(struct descriptor_data *desc, char *arg);
void autorent_all(void);
void give_prompt(struct descriptor_data *point);
void do_parry(CHAR*,CHAR* ,OBJ*);
void do_dodge(CHAR*,CHAR* ,OBJ*);
int board(OBJ *obj, CHAR *ch, int cmd, char *arg);
/* extern fcnts */

struct char_data *make_char(char *name, struct descriptor_data *desc);
void boot_db(void);
void zone_update(void);
void affect_update( void ); /* In spells.c */
void point_update( void );  /* In limits.c */
void free_char(struct char_data *ch);
void mobile_activity(CHAR *ch);
void string_add(struct descriptor_data *d, char *str);
void perform_violence(void);
void perform_mob_attack(void);
void stop_fighting(struct char_data *ch);
void show_string(struct descriptor_data *d, char *input);
void gr(int s);

void check_reboot(void);

FILE *logfile = NULL;    /* Where to send the log messages. */
void open_logfile(void);
void close_logfile(void);
void backup_logfile(void);

char* Color[] = {
  "\x1B[0m\x1B[30m", "Black",
  "\x1B[0m\x1B[31m", "Drk Red",
  "\x1B[0m\x1B[32m", "Drk Green",
  "\x1B[0m\x1B[33m", "Brown",
  "\x1B[0m\x1B[34m", "Drk Blue",
  "\x1B[0m\x1B[35m", "Drk Magenta",
  "\x1B[0m\x1B[36m", "Drk Cyan",
  "\x1B[0m\x1B[37m", "Grey",
  "\x1B[1m\x1B[31m", "Red",
  "\x1B[1m\x1B[32m", "Green",
  "\x1B[1m\x1B[33m", "Yellow",
  "\x1B[1m\x1B[34m", "Blue",
  "\x1B[1m\x1B[35m", "Magenta",
  "\x1B[1m\x1B[36m", "Cyan",
  "\x1B[1m\x1B[37m", "White",
  "\x1B[1m\x1B[30m", "Drk Grey",
};

char* BKColor[] = {
  "\x1B[0m",
  "\x1B[40m",
  "\x1B[41m",
  "\x1B[42m",
  "\x1B[43m",
  "\x1B[44m",
  "\x1B[45m",
  "\x1B[46m",
  "\x1B[47m",
};

char* Color2[] = {
  "\x1B[0;30",
  "\x1B[0;31",
  "\x1B[0;32",
  "\x1B[0;33",
  "\x1B[0;34",
  "\x1B[0;35",
  "\x1B[0;36",
  "\x1B[0;37",
  "\x1B[1;31",
  "\x1B[1;32",
  "\x1B[1;33",
  "\x1B[1;34",
  "\x1B[1;35",
  "\x1B[1;36",
  "\x1B[1;37",
  "\x1B[1;30",
};

char* BKColor2[] = {
  "0m",
  ";40m",
  ";41m",
  ";42m",
  ";43m",
  ";44m",
  ";45m",
  ";46m",
  ";47m",
};


/* *********************************************************************
*  main game loop and related stuff                               *
********************************************************************* */
int WIZLOCK;
int GAMELOCK;
int CHAOSMODE;
int GAMECHECK;
int GAMEHALT;
int REBOOT_AT;
int BAMDAY;
int BOOTFULL;
int PULSECHECK;
int CHAOSDEATH;
int TOKENCOUNT;
int DOUBLEXP;
#define SZ_OPT_CREATEIMP "CREATEIMP:"
char CREATEIMP[21];
int FREEMORT;

void update_reboot(void) {
  FILE *fl;
  int next_boot;

  fl=fopen("reboot_time", "r");
  if(!fl) REBOOT_AT=20;
  else {
    fscanf(fl,"%d\n",&REBOOT_AT);
    fclose(fl);
  }
  next_boot=REBOOT_AT-2;
  if(next_boot<0) next_boot+=24;
  fl=fopen("reboot_time","w");
  fprintf(fl,"%d\n",next_boot);
  fclose(fl);
}

int main(int argc, char **argv)
{
  int pos = 1,tmp_num;
  char tmp_txt[40];
  FILE *fl;

  port = DFLT_PORT;
  g_datadir = DFLT_DIR;

  CHAOSMODE=0;
  GAMECHECK=0;
  BAMDAY=0;
  BOOTFULL=0;
  PULSECHECK=0;
  CHAOSDEATH=0;
  TOKENCOUNT=0;
  DOUBLEXP=0;
  FREEMORT=0;
#ifdef TEST_SITE
  WIZLOCK=1;
  GAMELOCK=1;
  GAMEHALT=0;
  disablereboot=1;
#else
  WIZLOCK = 0;
  GAMELOCK=0;
  GAMEHALT=0;
  disablereboot=0;
#endif

  if(mcheck(0)) fprintf(stderr, "Cannot set mcheck");

  memset(CREATEIMP, 0, sizeof(CREATEIMP));

  open_logfile();

  /* Option file addition - Ranger Sept 2000 */
  if((fl=fopen("start_options", "r"))) {
    while(!feof(fl)) {
      fscanf(fl,"%d - %s\n",&tmp_num,tmp_txt);
      if(!strcmp("GAMECHECK",tmp_txt))     GAMECHECK=tmp_num;
      if(!strcmp("GAMELOCK",tmp_txt))      GAMELOCK=tmp_num;
      if(!strcmp("WIZLOCK",tmp_txt))       WIZLOCK=tmp_num;
      if(!strcmp("GAMEHALT",tmp_txt))      GAMEHALT=tmp_num;
      if(!strcmp("PULSECHECK",tmp_txt))    PULSECHECK=tmp_num;
      if(!strcmp("disablereboot",tmp_txt)) disablereboot=tmp_num;
      if(!strcmp("BAMDAY",tmp_txt))        BAMDAY=tmp_num;
      if(!strcmp("BOOTFULL",tmp_txt))      BOOTFULL=tmp_num;
      if(!strcmp("CHAOSMODE",tmp_txt))     CHAOSMODE=tmp_num;
      if(!strcmp("reboottype",tmp_txt))    reboot_type=tmp_num;
      if(!strcmp("CHAOSDEATH",tmp_txt))    CHAOSDEATH=tmp_num;
      if(!strcmp("TOKENCOUNT",tmp_txt))    TOKENCOUNT=tmp_num;
      if(!strcmp("DOUBLEXP",tmp_txt))      DOUBLEXP=tmp_num;
      if(!strcmp("FREEMORT",tmp_txt))      FREEMORT=tmp_num;

      if(!strncmp(SZ_OPT_CREATEIMP, tmp_txt, sizeof(SZ_OPT_CREATEIMP) - 1)) {
        if (tmp_num) {
          size_t len = strlen(tmp_txt) - (sizeof(SZ_OPT_CREATEIMP) - 1);
          if ((len > 0) && (len < sizeof(CREATEIMP))) {
            strncpy(CREATEIMP, tmp_txt + (sizeof(SZ_OPT_CREATEIMP) - 1), len);
          }
        }
      }
    }
    fclose(fl);
  }

  while ((pos < argc) && (*(argv[pos]) == '-')) {
   switch (*(argv[pos] + 1)) {
    case 'C':
      fCopyOver = TRUE;
      mother_desc = atoi(argv[pos]+2);
      log_f("Booting in copyover mode, mother desc = %d.", mother_desc);
      break;
    case 'l':
      lawful = 1;
      log_s("Lawful mode selected.");
      break;
    case 'd':
      if (*(argv[pos] + 2)) {
        g_datadir = argv[pos] + 2;
      }
      else if (++pos < argc) {
        g_datadir = argv[pos];
      }
      else {
        log_s("Directory arg expected after option -d.");
        produce_core();
      }
      break;
      case 's':
      no_specials = 1;
      log_s("Suppressing assignment of special routines.");
      break;
      case 'r':
      noroomdesc = TRUE;
      log_s("No room descriptions.");
      break;
      case 'e':
      noextradesc = TRUE;
      log_s("No extra descriptions.");
      break;
    default:
      log_f("Unknown option -%c in argument string.",
            *(argv[pos] + 1));
      break;
   }
   pos++;
  }

  if (pos < argc) {
   if (!isdigit(*argv[pos])) {
    log_f("Usage: %s [-l] [-s] [-d pathname] [ port # ]\n",
          argv[0]);
    produce_core();
   }
   else if ((port = atoi(argv[pos])) <= 1024) {
    log_s("Illegal port #\n");
    produce_core();
   }
  }

  uptime = time(0);

  log_f("Running game on port %d.", port);

  if (chdir(g_datadir) < 0) {
   log_s("chdir");
   produce_core();
  }

  log_f("Using %s as data directory.", g_datadir);

  run_the_game(port);
  return(0);
}


#define PROFILE(x)

void boot_slave( void )
{
    FILE *pidFile;
    int sv[2];
    int i;

    if( slave_socket != -1 ) {
       close( slave_socket );
       slave_socket = -1;
    }

    log_s("{BOOT} booting slave");
    log_s("{BOOT} slave: ouch");

    if( socketpair( AF_UNIX, SOCK_DGRAM, 0, sv ) < 0 ) {
       log_s("boot_slave: socketpair: ");
       return;
    }
    /* set to nonblocking */
    if( fcntl( sv[0], F_SETFL, FNDELAY ) == -1 ) {
       log_s("boot_slave: fcntl( F_SETFL, FNDELAY ): ");
       close(sv[0]);
       close(sv[1]);
       return;
    }
    slave_pid = vfork();
    switch( slave_pid ) {
    case -1: /* error*/
       log_s("boot_slave: vfork");
       close( sv[0] );
       close( sv[1] );
       return;

    case 0: /* child */
       close( sv[0] );
       close( 0 );
       close( 1 );
       if( dup2( sv[1], 0 ) == -1 ) {
           log_s("boot_slave: child: unable to dup stdin: ");
           produce_core();
       }
       if( dup2( sv[1], 1 ) == -1 ) {
           log_s("boot_slave: child: unable to dup stdout ");
           produce_core();
       }
       for( i = 3; i < avail_descs; ++i ) {
           close( i );
       }

#ifdef BCHS
       execlp( "roninslave", "roninslave", NULL );
#else
       execlp( "../bin/roninslave", "../bin/roninslave", NULL );
#endif

       log_s("boot_slave: child: unable to exec: ");
       exit(1);
    }
    pidFile = fopen("slave.pid", "w");
    if(pidFile)
        {
        fprintf(pidFile, "%d", slave_pid);
        fclose(pidFile);
        }
    close( sv[1] );

    if( fcntl(sv[0], F_SETFL, FNDELAY ) == -1 ) {
       log_s("boot_slave: fcntl");
       close( sv[0] );
       return;
    }
    slave_socket = sv[0];
}

void name_to_corpsefile(struct obj_data *corpse,FILE *fl) {
  struct corpsefile_name cname;
  memset(&cname,0,sizeof(cname));
  sprintf(cname.name,"%s",OBJ_NAME(corpse));
  fwrite(&cname,sizeof(cname),1,fl);
}

/* Saves items in PC corpses and statues at reboot - will be read into
   1201 immortal only room at restart - Ranger Sept 98*/
void obj_to_corpsefile(struct obj_data *obj, FILE *fl) {
  int j;
  struct obj_data *tmp;
  struct obj_file_elem_ver3 object;
  memset(&object,0,sizeof(object));

  if (!obj) return;
  if(!IS_RENTABLE(obj)) return;

  object.position   = -1;
  object.item_number= obj_proto_table[obj->item_number].virtual;
  object.value[0]   = obj->obj_flags.value[0];
  object.value[1]   = obj->obj_flags.value[1];
  object.value[2]   = obj->obj_flags.value[2];
  object.value[3]   = obj->obj_flags.value[3];
  if(obj->obj_flags.type_flag == ITEM_CONTAINER) {
    if(obj->item_number_v!=16)
      object.value[3]=COUNT_RENTABLE_CONTENTS(obj);
    else if(obj->contains)
      object.value[3]=COUNT_RENTABLE_CONTENTS(obj);
  }
  object.extra_flags= obj->obj_flags.extra_flags;
  object.weight     = obj->obj_flags.weight;
  object.timer      = obj->obj_flags.timer;
  object.bitvector  = obj->obj_flags.bitvector;
/* new obj saves */
  object.version=32003;
  object.type_flag=obj->obj_flags.type_flag;
  object.wear_flags=obj->obj_flags.wear_flags;
  object.extra_flags2=obj->obj_flags.extra_flags2;
  object.subclass_res=obj->obj_flags.subclass_res;
  object.material=obj->obj_flags.material;
  object.spec_value =OBJ_SPEC(obj);
  for(j=0;j<MAX_OBJ_AFFECT;j++)
    object.affected[j]=obj->affected[j];
/* end new obj saves */

/* new obj saves for obj ver3 */
      object.bitvector2  =obj->obj_flags.bitvector2;
/* end new ver3 obj saves */

/* New owner id */
  object.ownerid[0] =obj->ownerid[0];
  object.ownerid[1] =obj->ownerid[1];
  object.ownerid[2] =obj->ownerid[2];
  object.ownerid[3] =obj->ownerid[3];
  object.ownerid[4] =obj->ownerid[4];
  object.ownerid[5] =obj->ownerid[5];
  object.ownerid[6] =obj->ownerid[6];
  object.ownerid[7] =obj->ownerid[7];

  fwrite(&object, sizeof(object),1,fl);

  if(obj->contains) {
    for (tmp = obj->contains;tmp;tmp = tmp->next_content)
       obj_to_corpsefile(tmp, fl);
  }
}
void check_corpses(void) {
  OBJ *j,*jj,*obj;
  FILE *fl;

  fl=fopen("corpselist","wb");
  for (j = object_list; j ; j = j->next) {
    if(j->in_room==NOWHERE) continue;
    /* If this is a corpse */
    if(j->in_room==real_room(1201) && V_OBJ(j)==16) {
      name_to_corpsefile(j,fl);
      obj_to_corpsefile(j,fl);
    }
    else if((OBJ_TYPE(j) == ITEM_CONTAINER) && (j->obj_flags.value[3])) {
      if(!COUNT_RENTABLE_CONTENTS(j)) continue;
      if(j->obj_flags.cost!=PC_STATUE && j->obj_flags.cost!=PC_CORPSE) continue;
      obj=0;
      if(!(obj=read_object(16,VIRTUAL))) {
        log_s("ERROR: Can't create Vobj 16");
        return;
      }
      obj->obj_flags.value[3]=COUNT_RENTABLE_CONTENTS(j);
      name_to_corpsefile(j,fl);
      obj_to_corpsefile(obj,fl);
      for(jj=j->contains; jj; jj = jj->next_content)
        obj_to_corpsefile(jj,fl);
    }
  }
  fclose(fl);
}

/* Init sockets, run game, and cleanup sockets */
void copyover_recover(void);
int run_the_game(int port) {
  PROFILE(extern etext();)

  void signal_setup(void);
  int load(void);
  void comatose(int s);

  PROFILE(monstartup((int) 2, etext);)

  descriptor_list = NULL;

  log_s("Signal trapping.");
  signal_setup();

/* infinte loop signal */
/* Why trap this - checkpointing in signals.c kills game if inf loop */
  log_s("Trapping Signal 26 SIGVTALRM.");
  signal(SIGVTALRM, SIG_IGN);

  if (!fCopyOver) /* If copyover mother_desc is already set up */
  {
    log_s("Opening mother connection.");
    mother_desc = init_socket (port);
    boot_slave();
  }
  update_reboot();

  boot_db();

  maxdesc = mother_desc; // initialize the top descriptor to the mother_desc before full boot and copyover

  if (fCopyOver)
    copyover_recover();

  game_loop(mother_desc);

  autorent_all();

  close_sockets(mother_desc);

  PROFILE(monitor(0);)
  if (cleanreboot) {
   check_corpses();
   log_f("Rebooting with %d boot time high and %d connections",max_connects,total_connects);
   close_logfile();
   exit(52);            /* what's so great about HHGTTG, anyhow? */
  }

  log_s("Normal termination of game.");
  close_logfile();
  return 0;
}

/* logs #of people connected every 10 mins to a file plrlog
Ranger July 98 */
void plrlog(void) {
  FILE *fl;
  struct descriptor_data *d;
  int count=0;
  long ct;
  char *tmstr;

  fl = fopen("plrlog", "a");
  if (!fl) {
    log_s("Error cannot open plrlog file");
    return;
  }
  for(d=descriptor_list; d; d=d->next)
    if (!d->connected) count++;

  ct=time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr+strlen(tmstr)-1)='\0';
  fprintf(fl,"Time: %s Count: %d\n\r",tmstr,count);
  fclose(fl);
}


int get_slave_result(void);
void flying_room(int room);
void process_program_output(void);

/* Big change to make pulse change every 1/10 of a second
and to add checks for missed pulses, etc - Ranger - Dec 99 */


int game_loop(int mother_desc)
{
  fd_set input_set, output_set, exc_set, null_set;
  struct timeval last_time, opt_time, process_time, temp_time;
  struct timeval before_sleep, now, timeout;
  char comm[MAX_INPUT_LENGTH],buf[MIL];
  struct descriptor_data *point, *next_point;
  int pulse = 0,missed_pulses;
  sigset_t mask;

  /* initialize various time values */
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  opt_time.tv_usec = OPT_USEC;
  opt_time.tv_sec = 0;
  FD_ZERO(&null_set);

  gettimeofday(&last_time,(struct timezone *) 0);

  // 20170222 - Commented out, because maxdesc must be set for both copyover and full boot -- done in run_the_game
  //if (!fCopyOver)
  //  maxdesc = mother_desc;
  avail_descs = getdtablesize() - 2; /* !! Change if more needed !! */

  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGPIPE);
  sigaddset(&mask, SIGALRM);
  sigaddset(&mask, SIGTERM);
  sigaddset(&mask, SIGURG);
  sigaddset(&mask, SIGXCPU);
  sigaddset(&mask, SIGHUP);

  /* Logging of last command or signal from a crash - Ranger - Jan 2001 */
  last_command[0] = '\0';
  if(atexit(write_last_command) != 0) {
    log_s("install_other_handlers:atexit");
    exit (1);
  }
  /* This is the seg fault signal - if the write_last_command itself creates a
    sig fault, it will result in an infinite loop. */

  /* Main loop */
  while (!cleanshutdown) {
    /* Check what's happening out there */
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(mother_desc, &input_set);
    if(slave_socket != -1)
      FD_SET(slave_socket, &input_set);
    for (point = descriptor_list; point; point = point->next) {
      FD_SET(point->descriptor, &input_set);
      FD_SET(point->descriptor, &output_set);
      FD_SET(point->descriptor, &exc_set);
    }

    /*
     * At this point, we have completed all input, output and heartbeat
     * activity from the previous iteration, so we have to put ourselves
     * to sleep until the next 0.1 second tick.  The first step is to
     * calculate how long we took processing the previous iteration.
     */

    gettimeofday(&before_sleep, (struct timezone *) 0); /* current time */
    timediff(&process_time, &before_sleep, &last_time);

    /*
     * If we were asleep for more than one pass, count missed pulses and sleep
     * until we're resynchronized with the next upcoming pulse.
     */
    if (process_time.tv_sec == 0 && process_time.tv_usec < OPT_USEC) {
      missed_pulses = 0;
    } else {
      missed_pulses = process_time.tv_sec * PASSES_PER_SEC;
      missed_pulses += process_time.tv_usec / OPT_USEC;
      process_time.tv_sec = 0;
      process_time.tv_usec = process_time.tv_usec % OPT_USEC;
    }

    /* Calculate the time we should wake up */
    timediff(&temp_time, &opt_time, &process_time);
    timeadd(&last_time, &before_sleep, &temp_time);

    /* Now keep sleeping until that time has come */
    gettimeofday(&now, (struct timezone *) 0);
    timediff(&timeout, &last_time, &now);

    /* Go to sleep */
    do {
      game_sleep(&timeout);
      gettimeofday(&now, (struct timezone *) 0);
      timediff(&timeout, &last_time, &now);
    } while (timeout.tv_usec || timeout.tv_sec);

    sigprocmask(SIG_SETMASK,&mask,NULL);

    /* Poll (without blocking) for new input, output, and exceptions */
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      log_s("SYSERR: Select poll");
      return(-1);
    }

    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK,&mask,NULL);

    /* Respond to whatever might be happening */
    process_program_output();

    /* New connection? */
    if (FD_ISSET(mother_desc, &input_set))
      if (new_descriptor(mother_desc) < 0)
        log_s("New connection");

    /* kick out the freaky folks */
    for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      if (FD_ISSET(point->descriptor, &exc_set)) {
        FD_CLR(point->descriptor, &input_set);
        FD_CLR(point->descriptor, &output_set);
        close_socket(point);
      }
    }
    if( slave_socket != -1 && FD_ISSET( slave_socket, &input_set ) )
      while( get_slave_result() == 0 );

    for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      if (FD_ISSET(point->descriptor, &input_set))
        if (process_input(point) < 0)
          close_socket(point);
    }

    /* process_commands; */
    for (point = descriptor_list; point; point = next_to_process) {
      next_to_process = point->next;
/*
   Here is the check for command lag
*/
      if ((--(point->wait) <= 0) && get_from_q(&point->input, comm)) {
/*
   This check brings a player back if it had been moved due to idleness.
*/
        if (point->character && point->connected == CON_PLYNG &&
            point->character->specials.was_in_room != NOWHERE) {
          if (CHAR_REAL_ROOM(point->character) != NOWHERE)
            char_from_room(point->character);
          char_to_room(point->character,point->character->specials.was_in_room);
          point->character->specials.was_in_room = NOWHERE;
          act("$n has returned.",TRUE, point->character, 0, 0, TO_ROOM);
        }

        point->wait = 1; /* reset */

        if (point->character)
          point->character->specials.timer = 0; /* reset idle timer */
        point->prompt_mode = 1;

        if (point->str) {/* writing a string */
          string_add(point, comm);
        }
        else if (point->connected==CON_PLYNG) {/* if playing */
/*
   If being shown a string work it out, or check the command.
*/
          if (point->showstr_point) {
            show_string(point, comm);
          }
          else {
            command_interpreter(point->character, comm);
          }
        }
        else {
          nanny(point, comm); /* start a connection */
        }
      } /* if not lagged */
    } /* for loop */

/*
 Notes for addition of commands that can be processed even if lagged
 (socials, communication, other no-combat related commands).

 The inability to communicate is imo part of the disadvantage to using
skills and spells which lag a player.  However, this is a possible way
of allowing non-combat commands in the middle of a skill lag.  This will
not allow a player to jump in the middle of a series of 30 pummels
already sent to the game however, perhaps making it a little less
of an impact.

Bringing the player back and reseting the idle timer can be done regardless
of the wait amount with no impact, thus these can be removed to be outside
of any lag check.  However, for both string add and show string, the
wait should be checked.  With command interpreter, the wait would have
to be checked within that procedure and all commands would require an
additional variable - Run_With_Lag or something similar.  Within
command interpreter, combat commands would only be ran if the wait was
0 and other commands could be ran any time.  Command interpreter would
not reset the wait variable or manipulate it in any way.

some pseudo code loop thing

check for input
do -- on wait
return player
reset idle timer
if no lag
  reset wait to 1
  check for string input
  continue
  if char playing
    check for show string
    continue
  else
    goto connection nanny
    continue
  end of char playing if
end of no lag if
jump into command
  if wait is 1 process any
  otherwise process select commands
*/

    for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      point->wait-=missed_pulses;
      if (FD_ISSET(point->descriptor, &output_set) && point->output.head) {
        if (process_output(point) < 0)
          close_socket(point);
        else
          point->prompt_mode = 1;
      }
    }

    /* Kick out folks in the CON_CLOSE state */
    for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      if (point->connected == CON_CLOSE)
        close_socket(point);
    }

    /*
     * Now, we execute as many pulses as necessary--just one if we haven't
     * missed any pulses, or make up for lost time if we missed a few
     * pulses by sleeping for too long.
     */
    missed_pulses++;

    if(PULSECHECK && missed_pulses>1) {
      sprintf(buf,"WIZINFO: Missed %d pulses (10/sec).",missed_pulses);
      wizlog(buf,LEVEL_IMP, 6);
    }

    if (missed_pulses <= 0) {
      log_f("SYSERR: **BAD** MISSED_PULSES NONPOSITIVE (%d), TIME GOING BACKWARDS!!", missed_pulses);
      log_s("SYSERR: GAME BEING REBOOTED");
      send_to_world("EMERGENCY REBOOT, be back in a few minutes.\n\r");
      cleanreboot = 1;
      missed_pulses = 1;
    }

    /* If we missed more than 3 seconds worth of pulses, just do 3 secs */
    if (missed_pulses > (3 * PASSES_PER_SEC)) {
      log_f("SYSERR: Missed %d seconds worth of pulses.", missed_pulses / PASSES_PER_SEC);
      missed_pulses = 3 * PASSES_PER_SEC;
    }

    /* Now execute the heartbeat functions */
    while (missed_pulses--)
      heartbeat(++pulse);

    /* Roll pulse over after 10 hours */
    if (pulse >= (600 * PULSE_TICK))
      pulse = 0;

    tics++;        /* tics since last checkpoint signal */
  }
  last_command[0]='\0';
  return 0;
}

void check_token_mob(void);
void heartbeat(int pulse) {
  if ((pulse % PULSE_VIOLENCE == 0) && !GAMEHALT) {
    perform_mob_attack();
  }

  for (DESC *desc = descriptor_list, *next = NULL; desc; desc = next) {
    next = desc->next;

    if (DESC_PROMPT_MODE(desc)) {
      give_prompt(desc);
    }

    if ((pulse % PULSE_TICK == 0) && (STATE(desc) != CON_PLYNG) && !GAMEHALT) {
      DESC_TIMER(desc)++;

      if (DESC_TIMER(desc) > 30) {
        char buf[MSL];

        snprintf(buf, sizeof(buf), "WIZINFO: Closing socket %d - idle at menu.", DESC_DESCRIPTOR(desc));

        wizlog(buf, LEVEL_IMM, 5);
        log_s(buf);

        close_socket(desc);
      }
    }
  }

  /* 60 seconds */
  if ((pulse % PULSE_ZONE == 0) && !GAMEHALT) {
    zone_update();
  }

  /* 10 seconds */
  if ((pulse % PULSE_MOBILE == 0) && !GAMEHALT) {
    signal_world(0, MSG_MOBACT, "");
  }

  /* 3 seconds */
  if ((pulse % PULSE_VIOLENCE == 0) && !GAMEHALT) {
    /* Process flying rooms */
    for (int i = 0; i < top_of_flying; i++) {
      flying_room(*(flying_rooms + i));
    }

    /* 3 second pulse signaling for objects.
       Used as an override, since objects don't normally get this signal below. */
    const int msg_3_sec_pulse_objects[] = {
      18 // Lottery Machine
    };

    for (int i = 0; i < NUMELEMS(msg_3_sec_pulse_objects); i++) {
      if (obj_proto_table[real_object(msg_3_sec_pulse_objects[i])].number >= 1) {
        for (OBJ *obj = object_list; obj; obj = obj->next) {
          if (OBJ_VNUM(obj) == msg_3_sec_pulse_objects[i]) {
            signal_object(obj, 0, MSG_ROUND, "");
          }
        }
      }
    }

    /* 3 second pulse signaling for the world.
       This is filtered in the signal_* functions to send only enchantments for now. */
    signal_world(0, MSG_ROUND, "");

    /* Process combat */
    perform_violence();
  }

  /* 1 minute */
  if ((pulse % PULSE_TICK == 0) && !GAMEHALT) {
    signal_world(0, MSG_TICK, "");

    /* Update character points (hp, mana, move, ...) */
    point_update();

    /* Update character affects. */
    affect_update();

    /* Update weather and time. */
    static int weather_and_time_counter = 0;

    /* Every 4 real minutes is equal to one game hour. */
    if (++weather_and_time_counter == 4) {
      weather_and_time();

      weather_and_time_counter = 0;
    }

    if (!CHAOSMODE) {
      check_token_mob();
    }
  }

  /* 10 minutes */
  if ((pulse % (10 * PULSE_TICK) == 0)) {
    plrlog();

    check_reboot();
  }

  /* 2 hours */
  if ((pulse % (120 * PULSE_TICK) == 0) && !GAMEHALT) {
#ifndef TEST_SITE
    log_s("SUBLOG: Re-distributing subclass tokens.");

    /* Extract existing tokens. */
    for (OBJ *token = object_list, *next_token = NULL; token; token = next_token) {
      next_token = token->next;

      if ((V_OBJ(token) == TOKEN_OBJ_VNUM) && OBJ_CARRIED_BY(token) && IS_NPC(OBJ_CARRIED_BY(token)) && (V_MOB(OBJ_CARRIED_BY(token)) != TOKEN_MOB_VNUM)) {
        extract_obj(token);
      }
    }

    distribute_tokens(CHAOSMODE ? 0 : TOKENCOUNT, FALSE);
#endif
  }

  /* free disposed characters */
  for (CHAR* ch = disposed_chars, *next = NULL; ch; ch = next) {
    next = ch->next;
    free(ch);
  }
  disposed_chars = NULL;

  /* free disposed objs */
  for (OBJ* obj = disposed_objs, *next = NULL; obj; obj = next) {
    next = obj->next;
    free(obj);
  }
  disposed_objs = NULL;
}

void timediff(struct timeval *rslt, struct timeval *a, struct timeval *b)
{
  if (a->tv_sec < b->tv_sec)
    *rslt = null_time;
  else if (a->tv_sec == b->tv_sec) {
    if (a->tv_usec < b->tv_usec)
      *rslt = null_time;
    else {
      rslt->tv_sec = 0;
      rslt->tv_usec = a->tv_usec - b->tv_usec;
    }
  } 
  else /* a->tv_sec > b->tv_sec */
  { 
    rslt->tv_sec = a->tv_sec - b->tv_sec;
    if (a->tv_usec < b->tv_usec) 
  {
      rslt->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
      rslt->tv_sec--;
    } 
  else
      rslt->tv_usec = a->tv_usec - b->tv_usec;
  }
}

void timeadd(struct timeval *rslt, struct timeval *a, struct timeval *b)
{
  rslt->tv_sec = a->tv_sec + b->tv_sec;
  rslt->tv_usec = a->tv_usec + b->tv_usec;

  while (rslt->tv_usec >= 1000000) {
    rslt->tv_usec -= 1000000;
    rslt->tv_sec++;
  }
}


void autosave_all(void);
int copyover_write(int same_room) {
  struct descriptor_data *front = NULL,*middle = NULL,*back = NULL;
  char buf[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH];
  int room;
  CHAR *i;
  FILE *fl;

  if(!(fl=fopen(COPYOVER_FILE, "w"))) {
    log_s("Copyover file not writeable, aborted.");
    return FALSE;
  }

  autosave_all();
  check_corpses();
  log_f("Rebooting with %d boot time high and %d connections",max_connects,total_connects);
  sprintf(buf, "The world freezes as the gods rend it asunder....\n\r");

  fprintf(fl,"%d %d\n",slave_socket,slave_pid);

  /* Reverse the descriptor list */
  if(descriptor_list) {
    middle = descriptor_list;
    front = middle->next;
    back = NULL;
    while (1) {
      middle->next = back;
      if(front==NULL) break;
      back = middle;
      middle = front;
      front = front->next;
    }
    descriptor_list = middle;
  }

  for (DESC *d = descriptor_list, *next = NULL; d; d = next) {
    next = d->next;
    i=d->character;
    if (!d->character || (d->connected > CON_PLYNG)) {
      write_to_descriptor(d->descriptor, "\r\nSorry, the world is being rebooted, Come back in a minute.\r\n");
      close_socket(d);
    }
    else {
      if(!d->host) strcpy(d->host,"none");
      if(!same_room) room=0;
      else room=world[i->in_room_r].number;
      fprintf(fl, "%d %s %d %d %d %s\n", d->descriptor, string_to_lower(GET_NAME(i)), d->port, d->addr,room,d->host);
      write_to_descriptor(d->descriptor,buf);
    }
  }

  fprintf(fl, "-1\n");
  fclose(fl);


  sprintf(buf, "%d", port);
  sprintf(buf2, "-C%d", mother_desc);
  close_logfile();
  fclose(help_fl);
  fclose(wizhelp_fl);
  fclose(olchelp_fl);
  chdir("..");
/*
#ifdef TEST_SITE
  system("cp -pf ../imperial/ronintest ../bin/ronintest");
  execl("../bin/ronintest", "/bin/ronintest", buf2, buf, (char *) NULL);
#else
  system("cp -pf ../ronin/ronin ../bin/ronin");
  execl("../bin/ronin", "/bin/ronin", buf2, buf, (char *) NULL);
#endif
*/
  system("cp -pf obj/ronin bin/ronin");
  execl("bin/ronin", "/bin/ronin", buf2, buf, (char *) NULL);
  log_s("hotboot:execl");
  cleanshutdown = cleanreboot = 1;
  open_logfile();
  chdir(g_datadir);
  log_s("Hotboot failed - Doing normal reboot");
  return FALSE;
}

void copyover_recover(void) {
  struct descriptor_data *d;
  struct char_file_u_5 char_data_5;
  struct char_file_u_4 char_data_4;
  struct char_file_u_2 char_data_2;
  int desc,room,port,addr,version=0,last_desc=0;
  long last_up = 0;
  FILE *fp,*fl;
  bool fOld,close_desc;
  char name[MAX_INPUT_LENGTH],buf[MSL];

  log_s("Copyover recovery initiated");

  fp = fopen (COPYOVER_FILE, "r");

  if (!fp) {
    log_s("Copyover file not found. Exiting.\n\r");
    exit (1);
  }

  unlink(COPYOVER_FILE); /* In case something crashes - doesn't prevent reading  */

  max_connects=0;
  fscanf(fp,"%d %d\n",&slave_socket,&slave_pid);
  for (;;) {
    fOld = TRUE;
    fscanf (fp, "%d %s %d %d %d %s\n", &desc, name, &port, &addr, &room,buf);
    if (desc == -1) break;
    if(last_desc==desc) break;
    last_desc=desc;

    /* Write something, and check if it goes error-free */
    if (write_to_descriptor (desc, "\r") < 0) {
      log_f("Error writing to descriptor %d for character %s. Closing.", desc, name);
      close (desc);
      continue;
    }

    /* create a new descriptor */
    CREATE (d, struct descriptor_data, 1);
    memset ((char *) d, 0, sizeof (struct descriptor_data));
    init_descriptor (d,desc); /* set up various stuff */
    d->addr = addr;
    d->port = port;
    if(strcmp(buf,"none"))
      strcpy(d->host,buf);
    d->connected = CON_CLOSE;

    /* Now, find the pfile */
    sprintf(buf,"cp rent/%c/%s.dat rent/%c/%s.bak",UPPER(name[0]),name,UPPER(name[0]),name);
    system(buf);
    sprintf(buf,"rent/%c/%s.dat",UPPER(name[0]),name);
    if (!(fl = fopen(buf, "rb"))) fOld=FALSE;

    if(fOld) {
      version=char_version(fl);
      switch(version) {
        case 2:
          if((fread(&char_data_2,sizeof(struct char_file_u_2),1,fl))!=1)
          {log_s("Error Reading rent file(copyover)");fclose(fl);fOld=FALSE;}
          last_up=char_data_2.last_update;
          break;
        case 3:
          if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
          {log_s("Error Reading rent file(copyover)");fclose(fl);fOld=FALSE;}
          last_up=char_data_4.last_update;
          break;
        case 4:
          if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1)
          {log_s("Error Reading rent file(copyover)");fclose(fl);fOld=FALSE;}
          last_up=char_data_4.last_update;
          break;
        case 5:
          if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1)
          {log_s("Error Reading rent file(copyover)");fclose(fl);fOld=FALSE;}
          last_up=char_data_5.last_update;
          break;
        default:
          log_s("Error getting pfile version (copyover)");fclose(fl);
          fOld=FALSE;
      }
    }

    if(fOld) {
      CREATE(d->character, CHAR, 1);
      clear_char(d->character);
      d->character->desc=d;
      CREATE(d->character->skills, struct char_skill_data, MAX_SKILLS5);
      clear_skills(d->character->skills);
      reset_char(d->character);
      d->prompt = d->character->new.prompt;
      if(!IS_SET(d->prompt, PROMPT_VICTIM_TEX) && GET_LEVEL(d->character) < LEVEL_SUP)
        SET_BIT(d->prompt, PROMPT_VICTIM_TEX);
      if(!IS_SET(d->prompt, PROMPT_BUFFER_TEX) && GET_LEVEL(d->character) < LEVEL_SUP)
        SET_BIT(d->prompt, PROMPT_BUFFER_TEX);
      d->character->new.prompt = d->prompt;

      switch (version) {
        case 2:
          store_to_char_2(&char_data_2,d->character);
          break;
        case 3:
          store_to_char_4(&char_data_4,d->character);
          /* Changes for 50 level update */
          GET_EXP(d->character)=0;
          if(GET_LEVEL(d->character)==35 && IS_SET(d->character->new.imm_flags, WIZ_ACTIVE)) GET_LEVEL(d->character)=LEVEL_IMP;
          else if(GET_LEVEL(d->character)>=31) GET_LEVEL(d->character)=LEVEL_IMM;
          break;
        case 4:
          store_to_char_4(&char_data_4,d->character);
          break;
        case 5:
          store_to_char_5(&char_data_5,d->character);
          break;
        default:
          log_s("Version number corrupted? (copyover)");
          fOld=FALSE;
      }
    }

    if(fOld) {
      while (!feof(fl)) {
        switch(obj_version(fl)) {
          case 3:
            obj_to_char(store_to_obj_ver3(fl,d->character),d->character);
            break;
          case 2:
            obj_to_char(store_to_obj_ver2(fl,d->character),d->character);
            break;
          case 1:
            obj_to_char(store_to_obj_ver1(fl,d->character),d->character);
            break;
          case 0:
            obj_to_char(store_to_obj_ver0(fl,d->character),d->character);
            break;
        }
      }
    }

    if(fl) fclose(fl);

    d->character->next = character_list;
    character_list = d->character;

    if (!fOld) {
      write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r");
      close_socket (d);
    }
    else {
      max_connects++;
      if(desc>maxdesc) maxdesc=desc;
      write_to_descriptor (desc, "\n\r...the world reforms under divine direction.\n\r");
      if(!room) {
        if(GET_LEVEL(d->character)<LEVEL_IMM) room=3001;
        else room=1212;
      }
      if(real_room(room)!=NOWHERE)
        char_to_room(d->character, real_room(room));
      else
        char_to_room(d->character, real_room(0));

      if(GET_LEVEL(d->character) >= LEVEL_IMM)
         d->wizinfo = GET_LEVEL(d->character);

      sprintf(buf,"%ld",time(0)-last_up);
      if(signal_char(d->character,d->character,MSG_OBJ_ENTERING_GAME,buf))
        log_s("Error: Return TRUE from MSG_OBJ_ENTERING_GAME");

      d->connected = CON_PLYNG;
      do_look(d->character,"",CMD_LOOK);
      log_f("WIZINFO: Restored %s to room %d.",GET_NAME(d->character),room);

      /* Assign special character function (act.other.c). */
      d->character->func = char_spec;
    }
  }
  total_connects=max_connects;
  fclose (fp);
  for(desc=9;desc<256;desc++) {
    close_desc=TRUE;
    for(d=descriptor_list;d;d=d->next)
      if(desc==d->descriptor) close_desc=FALSE;
    if(close_desc)
      close(desc);
  }
}

/* ****************************************************************
*  general utility stuff (for local use)                          *
******************************************************************/

int get_from_q(struct txt_q *queue, char *dest)
{
  struct txt_block *tmp = NULL;

  /* Q empty? */
  if (!queue->head)
    return(0);

  strcpy(dest, queue->head->text ? queue->head->text : "");
  tmp = queue->head;
  queue->head = queue->head->next;

  if (tmp->text) {
    free(tmp->text);
    tmp->text = NULL;
  }
  free(tmp);
  tmp = NULL;

  return(1);
}

void write_to_q(char *txt, struct txt_q *queue) {
  struct txt_block *new = NULL;

  CREATE(new, struct txt_block, 1);
  CREATE(new->text, char, strlen(txt) + 1);

  strcpy(new->text, txt);

  /* Q empty? */
  if (!queue->head) {
    new->next = NULL;
    queue->head = queue->tail = new;
  }
  else {
    queue->tail->next = new;
    queue->tail = new;
    new->next = NULL;
  }
}

/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
  char dummy[MSL];

  while (get_from_q(&d->output, dummy));
  while (get_from_q(&d->input, dummy));
}

/* ******************************************************************
*  socket handling                                        *
****************************************************************** */

/* init_socket ripped out of circle code
   for testing purposes with Linux 2.0
   - Ranger Sept 97
   Additions made to allow the mud to reboot and not
   have the port be already in use. (SO_REUSEADDR)
   - Ranger Dec 97/Jan 98?
*/

int init_socket(int port)
{
  int s,opt;
  struct sockaddr_in sa;
  struct linger ld;

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    log_s("Error creating socket");
    exit(1);
  }

  opt = 1;

  if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
      (char *) &opt, sizeof (opt)) < 0) {
   log_s("setsockopt REUSEADDR");
   exit (1);
  }

  ld.l_onoff = 0; /* was 1 Ranger 19-Jul-02 */
  ld.l_linger = 0;

  if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
   log_s("setsockopt LINGER");
   produce_core();
  }
  /* Clear the structure */
  memset((char *)&sa, 0, sizeof(sa));

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    log_s("bind");
    close(s);
    exit(1);
  }
  nonblock(s);
  listen(s, 5); /* was 128 Ranger 19-Jul-02 */
  return s;
}

int new_connection(int s) {
  char buf[255];
  socklen_t i;
  int t = -1;
  fd_set   ready_to_read;
  struct sockaddr_in isa;

  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  i = sizeof(isa);
  getsockname(s, (struct sockaddr*)&isa, &i);

  FD_ZERO(&ready_to_read);
  FD_SET(s,&ready_to_read);

  // log_f("{ACCEPT} new connection from %s, re-selecting for read on all sockets", inet_ntoa(isa.sin_addr));

  if(select(s+1,&ready_to_read,NULL,NULL,&null_time)<0) {
    log_s("select error");
    produce_core();
  }

  // log_s("{ACCEPT} checking new socket for read ready state");

  if(FD_ISSET(s,&ready_to_read)) {
    if ((t = accept(s, (struct sockaddr*)&isa, &i)) < 0) {
      log_s("Accept");
      return(-1);
    }

    nonblock(t);

    // log_f("{ACCEPT} accepted new connection from ip: %s", inet_ntoa(isa.sin_addr));
  }

  /* this effectively sprintfs %u.%u.%u.%u with the inet addr */
  if (slave_socket != -1) {
    sprintf(buf, "%c%s\n", SLAVE_IPTONAME, inet_ntoa(isa.sin_addr));

    log_f("{ACCEPT} asking slave to resolve %s", inet_ntoa(isa.sin_addr));

    /* ask slave to do a hostname lookup for us */
    if (write( slave_socket, buf, strlen(buf) ) != strlen(buf)) {
      log_s("{SLAVE} losing slave on write: ");
      close( slave_socket );
      slave_socket = -1;
    }

    /* ask slave to do an identquery for us */

/*  Disabled - Virtually no machine now accepts username lookups.  Pointless
 to waste the cpu - Ranger

       buf[0] = SLAVE_IDENTQ;
       sprintf(buf, "%c%s,%d,%d\n", SLAVE_IDENTQ, inet_ntoa(isa.sin_addr), ntohs(isa.sin_port), port);
       if( write( slave_socket, buf, strlen(buf) ) != strlen(buf) ) {
           log_f( "{SLAVE} losing slave on write");
           close( slave_socket );
           slave_socket = -1;
       }
*/
  }

  return(t);
}


/* get a result from the slave */
 int get_slave_result( void )
{
    char buf[ MSL +1 ]="";
    char *userid, *token, *tmp;
    char *ascii_addr;
    int remote_port;
    struct descriptor_data *d;
    uint32_t addr;
    int len;

    len = read(slave_socket, buf, 100);
    if( len < 0 ) {
       if( errno == EAGAIN || errno == EWOULDBLOCK ) return( -1 );
       log_s("{SLAVE} losing slave on read: ");
       close( slave_socket );
       slave_socket = -1;
       return( -1 );
    } else if( len == 0 ) {
       return( -1 );
    }

    switch( buf[0] ) {
    case SLAVE_IDENTQ:
       ascii_addr = strtok(buf+1, " ");
       if(ascii_addr)
         {
         tmp = strtok(NULL, ",");
         if(tmp)
           {
           remote_port = atoi(tmp);
           tmp = strtok(NULL, ":");
           if(tmp)
             {
             tmp = strtok(NULL, ":");
             tmp = strtok(NULL, ":");
             userid = strtok(NULL, "\n");
             if(userid)
               {
               addr = inet_addr(ascii_addr);
               for( d = descriptor_list; d; d = d->next ) {
                  if( d->port == remote_port && d->addr == addr)
                    {
                    strncpy( d->userid, userid, MAX_USERID );
                    d->userid[MAX_USERID-1] = 0;
                    }
                 }
               }
             else
               log_s("{SLAVE}: userid not returned");
             }
           }
         }
       return(0);
       break;

    case SLAVE_IPTONAME:
       ascii_addr = strtok(buf+1, " ");
       token      = strtok(NULL, "\n");
       if(ascii_addr)
         {
         addr = inet_addr(ascii_addr);
         for( d = descriptor_list; d; d = d->next ) {
             if( d->addr != addr ) continue;
             strncpy( d->host, token, MAX_HOSTNAME );
             d->host[MAX_HOSTNAME-1] = 0;
           }
         }
       return(0);
       break;
    default:
       log_s("{SLAVE} invalid: ");
       break;
    }
    return( -1 );
}

int new_descriptor(int s)
{
  int desc;
  struct descriptor_data *newd;
  socklen_t size;
  bool total_ban=0;
  char buf[255],ip_txt[50],tmp[50];
  FILE *file;
  struct sockaddr_in sock;

  if ((desc = new_connection(s)) < 0)
   return (-1);

  if ((desc + 1) >= avail_descs) { /* maxdesc changed to desc */
   write_to_descriptor(desc, "Sorry.. The game is full...\n\r");
   close(desc);
   return(0);
  }
  else
   if (desc > maxdesc)
    maxdesc = desc;

  size = sizeof(sock);
  if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
   log_s("getpeername");
  }

  sprintf(tmp,"%s", inet_ntoa(sock.sin_addr));
  /* more ban stuff*/
  file = fopen("total-ban", "r");
  if (file) {
     while (!feof(file)) {
       fscanf(file, "%s - %s\n",ip_txt,buf);
       if(!strncmp(tmp,ip_txt, strlen(ip_txt))) total_ban=TRUE;
     }
     fclose(file);
  }

  if(total_ban) {
        write_to_descriptor(desc, "Sorry.. Your site has been banned.\
A member or members that operates from \r\n\
the site or subnet that you have logged\r\n\
in from has been a habitual problem\r\n\
causer and has caused us to take extreme\r\n\
measures to keep the peace.\n\r\
\r\n\
The admin of RoninMUD.");
        close(desc);
        return(0);
        }


  CREATE(newd, struct descriptor_data, 1);
  memset((char *) newd, 0, sizeof(struct descriptor_data));
  /* Initialize and assign addresses */
  init_descriptor(newd,desc);
  newd->addr = sock.sin_addr.s_addr;
  newd->port = ntohs(sock.sin_port);

  SEND_TO_Q(GREETINGS, newd);
  SEND_TO_Q("By what name do you wish to be known? ", newd);

  return(0);
}

void init_descriptor (struct descriptor_data *newd, int desc)
{
  newd->descriptor = desc;
  newd->connected  = 1;
  newd->wizinfo = 0;
  newd->free_rent = 0;
  newd->wait = 1;
  newd->timer=0;
  newd->prompt = 0xffffffff;
  newd->prompt_mode = 0;
  *newd->buf = '\0';
  newd->str = 0;
  newd->showstr_head = 0;
  newd->showstr_point = 0;
  *newd->last_input= '\0';
  newd->output.head = NULL;
  newd->input.head = NULL;
  newd->character = 0;
  newd->original = 0;
  newd->snoop.snooping = 0;
  newd->snoop.snoop_by = 0;
  newd->next = descriptor_list;
  descriptor_list = newd;
}


int process_output(struct descriptor_data *desc) {
  char buf[4 * MSL + 1];

  if (!desc->prompt_mode && !desc->connected) {
    if (write_to_descriptor(desc->descriptor, "\n\r") < 0) return -1;
  }

  while (get_from_q(&desc->output, buf)) {
    if (write_to_descriptor(desc->descriptor, buf)) return -1;
  }

  if (!desc->connected && !(desc->character && !IS_NPC(desc->character) && IS_SET(GET_PFLAG(desc->character), PLR_COMPACT))) {
    if (write_to_descriptor(desc->descriptor, "\n\r") < 0) return -1;
  }

  return 1;
}


int write_to_descriptor(int desc, char *txt) {
  int so_far = 0, this_round = 0, total = 0;

  total = strlen(txt);

  do {
    this_round = write(desc, txt + so_far, total - so_far);

    if (this_round < 0) {
      log_f("Write to socket error, %d.", errno);

      return -1;
    }

    so_far += this_round;
  } while (so_far < total);

  return 0;
}

int process_input(struct descriptor_data *t)
{
  int sofar, thisround, begin, squelch, i, k, flag;
  char tmp[MAX_INPUT_LENGTH+50], buffer[MAX_INPUT_LENGTH * 2];

  sofar = 0;
  flag = 0;
  begin = strlen(t->buf);

  /* Read in some stuff */
  do
  {
   if ((thisround = read(t->descriptor, t->buf + begin + sofar,
       MSL - (begin + sofar) - 1)) > 0)
    sofar += thisround;
   else
    if (thisround < 0)
     if (errno != EWOULDBLOCK) {
      log_s("Read1 - ERROR");
      return(-1);
     } else
      break;
    else {
     log_s("EOF encountered on socket read.");
     return(-1);
    }
  } while (!ISNEWL(*(t->buf + begin + sofar - 1)));

  *(t->buf + begin + sofar) = 0;

  /* if no newline is contained in input, return without proc'ing */
  for (i = begin; !ISNEWL(*(t->buf + i)); i++)
   if (!*(t->buf + i))
    return(0);

  /* input contains 1 or more newlines; process the stuff */
  for (i = 0, k = 0; *(t->buf + i);) {
   if (!ISNEWL(*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))))
    if (*(t->buf + i) == '\b')       /* backspace */
     if (k)  /* more than one char ? */ {
      if (*(tmp + --k) == '$')
       k--;
      i++;
     } else
      i++;  /* no or just one char.. Skip backsp */
    else
     if (isascii(*(t->buf + i)) && isprint(*(t->buf + i))) {
      /* trans char, double for '$' (printf)      */
      if ((*(tmp + k) = *(t->buf + i)) == '$')
       *(tmp + ++k) = '$';
      k++;
      i++;
     } else
      i++;
   else {
    *(tmp + k) = 0;
    if (*tmp == '!')
     strcpy(tmp,t->last_input);
    else
     strcpy(t->last_input,tmp);

    write_to_q(tmp, &t->input);

    if(t->snoop.snoop_by && strncmp("pass",tmp,4)) {
      printf_to_char(t->snoop.snoop_by,"%% %s\n\r",tmp);
    }

    if (flag) {
     sprintf(buffer,
      "Line too long. Truncated to:\n\r%s\n\r", tmp);
     if (write_to_descriptor(t->descriptor, buffer) < 0)
      return(-1);

     /* skip the rest of the line */
     for (; !ISNEWL(*(t->buf + i)); i++);
    }

    /* find end of entry */
    for (; ISNEWL(*(t->buf + i)); i++);
    /* squelch the entry from the buffer */
    for (squelch = 0;; squelch++)
      if ((*(t->buf + squelch) = *(t->buf + i + squelch)) == '\0')
        break;

    k = 0;
    i = 0;

   }
  }
  return(1);
}


void close_sockets(int s)
{
  log_s("Closing all sockets.");

  while (descriptor_list)
   close_socket(descriptor_list);
  if(slave_socket != -1)
     kill(slave_pid, SIGKILL);
  close(s);
}

void close_socket(struct descriptor_data *d)
{
  struct descriptor_data *tmp;
  struct sockaddr_in isa;
  char buf[100];

  if (!d) return;

  close(d->descriptor);
  flush_queues(d);
  if (d->descriptor == maxdesc)
   --maxdesc;

  /* Forget snooping */
  if (d->snoop.snooping)
   d->snoop.snooping->desc->snoop.snoop_by = 0;

  if (d->snoop.snoop_by) {
   send_to_char("Your victim is no longer among us.\n\r",d->snoop.snoop_by);
   d->snoop.snoop_by->desc->snoop.snooping = 0;
  }

  if (d->character) {
   if(d->character->specials.protecting) {
     act("You are no longer protected by $N.",0,d->character->specials.protecting,0,d->character,TO_CHAR);
     d->character->specials.protecting->specials.protect_by=0;
     d->character->specials.protecting=0;
   }
   if(d->character->specials.protect_by) {
     act("You are no longer protecting $N.",0,d->character->specials.protect_by,0,d->character,TO_CHAR);
     d->character->specials.protect_by->specials.protecting=0;
     d->character->specials.protect_by=0;
   }
   if (d->connected == CON_PLYNG) {
      if (IS_SET(d->character->specials.pflag,PLR_WRITING))
       REMOVE_BIT(d->character->specials.pflag,PLR_WRITING);
      if(GET_LEVEL(d->character)<LEVEL_IMM) save_char(d->character, NOWHERE);;
      act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
      sprintf(buf, "Closing link to: %s(%d).", GET_NAME(d->character), GET_LEVEL(d->character));
      log_s(buf);
      wizlog(buf, GET_LEVEL(d->character), 1);
      d->character->desc = 0;
      d->character->switched=0;
    } else {
      isa.sin_addr.s_addr = d->addr;
      sprintf(buf, "Losing player: %s[%s].", GET_NAME(d->character), (d->host && d->host[0] != '\0') ? d->host : inet_ntoa(isa.sin_addr));
      log_s(buf);
      wizlog(buf, GET_LEVEL(d->character), 1);

      if (d->character == character_list) {
        character_list = d->character;
      }
      else {
        CHAR *ch = NULL;
        for (ch = character_list; ch && ch->next != d->character; ch = ch->next);
        if (ch) {
          ch->next = d->character->next;
        }
      }
      free_char(d->character);
      d->character = NULL;
    }
  }
  else
   log_s("Losing descriptor without char.");

  if (next_to_process == d)            /* to avoid crashing the process loop */
   next_to_process = next_to_process->next;

  if (d == descriptor_list) /* this is the head of the list */
   descriptor_list = descriptor_list->next;
  else  /* This is somewhere inside the list */
  {
    /* Locate the previous element */
   for (tmp = descriptor_list; (tmp->next != d) && tmp;
    tmp = tmp->next);

   tmp->next = d->next;
  }

  if (d->showstr_head) {
    free(d->showstr_head);
    d->showstr_head = NULL;
  }

  free(d);
  d = NULL;
}

void nonblock(int s)
{
  int flags;

  flags = fcntl(s, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (fcntl(s, F_SETFL, flags) < 0) {
   log_s("Noblock");
   produce_core();
  }
}




#define COMA_SIGN \
"\n\r\
DikuMUD is currently inactive due to excessive load on the host machine.\n\r\
Please try again later.\n\r\n\n\r\
   Sadly,\n\r\
\n\r\
    the DikuMUD system operators\n\r\n\r"

#ifdef GR
/* sleep while the load is too high */
void comatose(int s)
{
  fd_set input_set;
  static struct timeval timeout =
  {
   60,
    0
  };
  int conn;
  int workhours(void);
  int load(void);

  log_s("Entering comatose state.");

  sigsetmask(sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
           sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
           sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP));


  while (descriptor_list)
   close_socket(descriptor_list);

  FD_ZERO(&input_set);
  do {
   FD_SET(s, &input_set);
   if (select(64, &input_set, 0, 0, &timeout) < 0) {
    log_s("coma select");
    produce_core();
   }
   if (FD_ISSET(s, &input_set)) {
    if (load() < 6) {
     log_s("Leaving coma with visitor.");
     sigsetmask(0);
     return;
    }
    if ((conn = new_connection(s)) >= 0) {
     write_to_descriptor(conn, COMA_SIGN);
     sleep(2);
     close(conn);
    }
   }

   tics = 1;
   if (workhours()) {
    log_s("Working hours collision during coma. Exit.");
    produce_core();
   }
  }
  while (load() >= 6);

  log_s("Leaving coma.");
  sigsetmask(0);
}

#endif

/* ****************************************************************
*      Public routines for system-to-player-communication         *
******************************************************************/

/* Only types so far are 0 - no type and 1 - combat.  For snoop brief
   Ranger Jan 99 */
void send_to_char_by_type(char *message, CHAR *ch, int type) {
  if (!message || !*message || !ch || !GET_DESCRIPTOR(ch)) return;

#ifdef TYPECOLOR
  char buf[MSL * 4];

  bool color_active = FALSE;

  for (char *str_ptr = message, *buf_ptr = buf, *sub = NULL;;) {
    if (*str_ptr == '`' &&
        ((*(str_ptr + 1) == '`') ||
         (*(str_ptr + 1) >= 'a' && *(str_ptr + 1) <= 'q') ||
         (*(str_ptr + 1) >= 'A' && *(str_ptr + 1) <= 'Q'))) {
      char c = *(++str_ptr);
      char color_num = LOWER(c) - '`';

      if (c == '`') {
        *(buf_ptr++) = '`';
      }
      else if ((GET_COMM_COLOR(ch, COMM_COLOR_ENABLED) == TRUE) && (color_num >= COMM_COLOR_CODE_FIRST) && (color_num <= COMM_COLOR_CODE_LAST)) {
        if (color_num == COMM_COLOR_CODE_LAST) {
          color_num = (GET_COMM_COLOR(ch, COMM_COLOR_FOREGROUND) ? GET_COMM_COLOR(ch, COMM_COLOR_FOREGROUND) : COMM_COLOR_CODE_GREY);
        }

        sub = Color2[color_num - 1];
        while ((*buf_ptr = *(sub++))) {
          buf_ptr++;
        }

        sub = BKColor2[GET_COMM_COLOR(ch, COMM_COLOR_BACKGROUND)];
        while ((*buf_ptr = *(sub++))) {
          buf_ptr++;
        }

        color_active = TRUE;
      }

      str_ptr++;
    }
    else if (!(*(buf_ptr++) = *(str_ptr++))) {
      *(--buf_ptr) = '\0';
      break;
    }
  }

  write_to_q(buf, &ch->desc->output);

  if (color_active && GET_COMM_COLOR(ch, COMM_COLOR_ENABLED) && GET_COMM_COLOR(ch, COMM_COLOR_FOREGROUND)) {
    write_to_q(Color2[GET_COMM_COLOR(ch, COMM_COLOR_FOREGROUND) - 1], &ch->desc->output);
    write_to_q(BKColor2[GET_COMM_COLOR(ch, COMM_COLOR_BACKGROUND)], &ch->desc->output);
  }

  CHAR *snooper = ch->desc->snoop.snoop_by;

  if (snooper) {
    if (!IS_SET(GET_IMM_FLAGS(snooper), WIZ_SNOOP_BRIEF) || (type == 0)) {
      printf_to_char(snooper, "%% %s", buf);
    }

    if (color_active && GET_COMM_COLOR(snooper, COMM_COLOR_ENABLED) && GET_COMM_COLOR(snooper, COMM_COLOR_FOREGROUND)) {
      printf_to_char(snooper, "%s%s", Color2[GET_COMM_COLOR(snooper, COMM_COLOR_FOREGROUND) - 1], BKColor2[GET_COMM_COLOR(snooper, COMM_COLOR_BACKGROUND)]);
    }
  }
#else
  write_to_q(message, &ch->desc->output);

  CHAR *snooper = ch->desc->snoop.snoop_by;

  if (snooper) {
    if (!IS_SET(GET_IMM_FLAGS(snooper), WIZ_SNOOP_BRIEF) || (type == 0)) {
      printf_to_char(snooper, "%% %s", message);
    }
  }
#endif
}

void act_by_type(char *message, int hide, CHAR *ch, void *other_or_obj, void *vict_or_obj, int type, int type2) {
  if (!message || !*message || !ch || (CHAR_REAL_ROOM(ch) == NOWHERE)) return;

  char buf[MSL * 2];

  CHAR *to_ch = NULL;

  switch (type) {
    case TO_VICT:
      to_ch = (CHAR *)vict_or_obj;
      break;

    case TO_CHAR:
      to_ch = ch;
      break;

    case TO_OTHER:
      to_ch = (CHAR *)other_or_obj;
      break;

    default:
      to_ch = world[CHAR_REAL_ROOM(ch)].people;
      break;
  }

  for (; to_ch; to_ch = to_ch->next_in_room) {
    if ((GET_DESCRIPTOR(to_ch) &&
        ((hide < COMM_ACT_HIDE_CANT_SEE) || (IS_SET(hide, COMM_ACT_HIDE_NON_MORT)) || CAN_SEE(to_ch, ch)) &&
        ((!IS_SET(hide, COMM_ACT_HIDE_SUPERBRF)) || !IS_SET(GET_PFLAG(to_ch), PLR_SUPERBRF)) &&
        ((type == TO_VICT) || (type == TO_CHAR) || (GET_POS(to_ch) != POSITION_SLEEPING)) &&
        ((type == TO_CHAR) || (to_ch != ch)) &&
        ((type != TO_NOTVICT) || (to_ch != (CHAR *)vict_or_obj)) &&
        ((type != TO_GROUP) || SAME_GROUP(to_ch, ch)))) {
      for (char *str_ptr = message, *buf_ptr = buf, *sub = NULL;;) {
        if (*str_ptr == '$') {
          char c = *(++str_ptr);

          switch (c) {
            case 'z':
              if (to_ch == ch) {
                sub = "";
              }
              else {
                sub = "s";
              }
              break;
            case 'r':
              if (to_ch == ch) {
                sub = "your";
              }
              else {
                sub = POSSESS_ex(ch, to_ch, hide);
              }
              break;
            case 'R':
              if (!vict_or_obj) {
                sub = "$R";
              }
              else {
                if (vict_or_obj == to_ch) {
                  sub = "your";
                }
                else {
                  sub = POSSESS_ex((CHAR *)vict_or_obj, to_ch, hide);
                }
              }
              break;
            case 'i':
              if (!ch) {
                sub = "$i";
              }
              else {
                sub = PERS_ex(ch, to_ch, hide);
              }
              break;
            case 'I':
              if (!vict_or_obj) {
                sub = "$I";
              }
              else {
                sub = PERS_ex((CHAR *)vict_or_obj, to_ch, hide);
              }
              break;
            case 'n':
              if (to_ch == ch) {
                sub = "you";
              }
              else {
                sub = PERS_ex(ch, to_ch, hide);
              }
              break;
            case 'N':
              if (!vict_or_obj) {
                sub = "$N";
              }
              else {
                if (to_ch == vict_or_obj) {
                  sub = "you";
                }
                else {
                  sub = PERS_ex((CHAR *)vict_or_obj, to_ch, hide);
                }
              }
              break;
            case 'm':
              if (to_ch == ch) {
                sub = "you";
              }
              else {
                sub = HMHR(ch);
              }
              break;
            case 'M':
              if (!vict_or_obj) {
                sub = "$M";
              }
              else {
                if (to_ch == vict_or_obj) {
                  sub = "you";
                }
                else {
                  sub = HMHR((CHAR *)vict_or_obj);
                }
              }
              break;
            case 's':
              if (to_ch == ch) {
                sub = "your";
              }
              else {
                sub = HSHR(ch);
              }
              break;
            case 'S':
              if (!vict_or_obj) {
                sub = "$S";
              }
              else {
                if (to_ch == vict_or_obj) {
                  sub = "you";
                }
                else {
                  sub = HSHR((CHAR *)vict_or_obj);
                }
              }
              break;
            case 'e':
              if (to_ch == ch) {
                sub = "you";
              }
              else {
                sub = HSSH(ch);
              }
              break;
            case 'E':
              if (!vict_or_obj) {
                sub = "$E";
              }
              else {
                if (to_ch == vict_or_obj) {
                  sub = "you";
                }
                else {
                  sub = HSSH((CHAR *)vict_or_obj);
                }
              }
              break;
            case 'o':
              if (!other_or_obj) {
                sub = "$o";
              }
              else {
                sub = OBJN((OBJ *)other_or_obj, to_ch);
              }
              break;
            case 'O':
              if (!vict_or_obj) {
                sub = "$O";
              }
              else {
                sub = OBJN((OBJ *)vict_or_obj, to_ch);
              }
              break;
            case 'q':
              if (!other_or_obj) {
                sub = "$q";
              }
              else {
                sub = OBJS2((OBJ *)other_or_obj, to_ch);
              }
              break;
            case 'Q':
              if (!vict_or_obj) {
                sub = "$Q";
              }
              else {
                sub = OBJS2((OBJ *)vict_or_obj, to_ch);
              }
              break;
            case 'p':
              if (!other_or_obj) {
                sub = "$p";
              }
              else {
                sub = OBJS((OBJ *)other_or_obj, to_ch);
              }
              break;
            case 'P':
              if (!vict_or_obj) {
                sub = "$P";
              }
              else {
                sub = OBJS((OBJ *)vict_or_obj, to_ch);
              }
              break;
            case 'a':
              if (!other_or_obj) {
                sub = "$a";
              }
              else {
                sub = SANA((OBJ *)other_or_obj);
              }
              break;
            case 'A':
              if (!vict_or_obj) {
                sub = "$A";
              }
              else {
                sub = SANA((OBJ *)vict_or_obj);
              }
              break;
            case 'T':
              if (!vict_or_obj) {
                sub = "$T";
              }
              else {
                sub = (char *)vict_or_obj;
              }
              break;
            case 'f':
              if (!other_or_obj) {
                sub = "$f";
              }
              else {
                sub = fname((char *)other_or_obj);
              }
              break;
            case 'F':
              if (!vict_or_obj) {
                sub = "$F";
              }
              else {
                sub = fname((char *)vict_or_obj);
              }
              break;
            case 'V':
              if (!vict_or_obj) {
                sub = "no-one";
              }
              else {
                if (to_ch == vict_or_obj) {
                  sub = "you";
                }
                else {
                  sub = PERS_ex((CHAR *)vict_or_obj, to_ch, hide);
                }
              }
              break;
            case '$':
              sub = "$";
              break;
            default:
              log_f("WARNING: Invalid $flag used in act(): %s", message);
              break;
          }

          while ((*buf_ptr = *(sub++))) {
            buf_ptr++;
          }

          str_ptr++;
        }
        else if (!(*(buf_ptr++) = *(str_ptr++))) {
          *(--buf_ptr) = '\n';
          *(++buf_ptr) = '\r';
          *(++buf_ptr) = '\0';
          break;
        }
      }

      send_to_char_by_type(CAP(buf), to_ch, type2);
    }

    if ((type == TO_VICT) || (type == TO_CHAR) || (type == TO_OTHER)) break;
  }
}

void act(char *message, int hide, CHAR * ch, void *other_or_obj, void *vict_or_obj, int type/*, ...*/) {
  if (!message) return;

  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  act_by_type(message, hide, ch, other_or_obj, vict_or_obj, type, 0);
}

void act_f(int hide, CHAR *ch, void *other_or_obj, void *vict_or_obj, int type, char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  act(buf, hide, ch, other_or_obj, vict_or_obj, type);
}

void send_to_char(char *message, CHAR *ch/*, ...*/) {
  if (!message || !ch) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  send_to_char_by_type(message, ch, 0);
}

void send_to_group(char *message, CHAR *ch, bool same_room/*, ...*/) {
  if (!message || !ch || !GET_MASTER(ch)) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  if (GET_MASTER(ch) != ch) {
    send_to_char(message, ch);
  }

  for (FOL *follower = GET_FOLLOWERS(GET_MASTER(ch)); follower; follower = follower->next) {
    CHAR *group_member = follower->follower;

    if (group_member && (group_member != ch) && SAME_GROUP(ch, group_member) && (!same_room || SAME_ROOM(group_member, ch))) {
      send_to_char(message, group_member);
    }
  }
}

void send_to_room(char *message, int room/*, ...*/) {
  if (!message || (room < 0) || (room > top_of_world)) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  for (CHAR *temp_ch = world[room].people; temp_ch; temp_ch = temp_ch->next_in_room) {
    if (temp_ch->desc) {
      send_to_char(message, temp_ch);
    }
  }
}

void send_to_room_except(char *message, int room, CHAR *ch/*, ...*/) {
  if (!message || (room < 0) || (room > top_of_world) || !ch) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  for (CHAR *temp_ch = world[room].people; temp_ch; temp_ch = temp_ch->next_in_room) {
    if (temp_ch->desc && (temp_ch != ch)) {
      send_to_char(message, temp_ch);
    }
  }
}

void send_to_room_except_two(char *message, int room, CHAR *ch1, CHAR *ch2/*, ...*/) {
  if (!message || (room < 0) || (room > top_of_world) || !ch1 || !ch2) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  for (CHAR *temp_ch = world[room].people; temp_ch; temp_ch = temp_ch->next_in_room) {
    if (temp_ch->desc && (temp_ch != ch1) && (temp_ch != ch2)) {
      send_to_char(message, temp_ch);
    }
  }
}

void send_to_world(char *message/*, ...*/) {
  if (!message) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  for (int room = 0; room < top_of_world; room++) {
    send_to_room(message, room);
  }
}

void send_to_world_except(char *message, CHAR *ch/*, ...*/) {
  if (!message || !ch) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  for (int room = 0; room < top_of_world; room++) {
    send_to_room_except(message, room, ch);
  }
}

void send_to_outdoor(char *message/*, ...*/) {
  if (!message) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  for (DESC *desc = descriptor_list; desc; desc = desc->next) {
    if ((desc->connected == CON_PLYNG) && IS_OUTSIDE(desc->character) && AWAKE(desc->character)) {
      send_to_char(message, desc->character);
    }
  }
}

void send_to_all(char *message/*, ...*/) {
  if (!message) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  for (DESC *desc = descriptor_list; desc; desc = desc->next) {
    if (desc->connected == CON_PLYNG) {
      send_to_char(message, desc->character);
    }
  }
}

void send_to_all_except(char *message, CHAR *ch/*, ...*/) {
  if (!message || !ch) return;
  /*
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);
  */
  for (DESC *desc = descriptor_list; desc; desc = desc->next) {
    if ((desc->connected == CON_PLYNG) && (desc->character != ch)) {
      send_to_char(message, desc->character);
    }
  }
}

void printf_to_char(CHAR *ch, char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_char(buf, ch);
}

void printf_to_group(CHAR *ch, bool same_room, char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_group(buf, ch, same_room);
}

void printf_to_room(int room, char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_room(buf, room);
}

void printf_to_room_except(int room, CHAR *ch, char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_room_except(buf, room, ch);
}

void printf_to_room_except_two(int room, CHAR *ch1, CHAR *ch2, char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_room_except_two(buf, room, ch1, ch2);
}

void printf_to_world(char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_world(buf);
}

void printf_to_world_except(CHAR *ch, char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_world_except(buf, ch);
}

void printf_to_outdoor(char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_outdoor(buf);
}

void printf_to_all(char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_all(buf);
}

void printf_to_all_except(CHAR *ch, char *message, ...) {
  char buf[2 * MSL];
  va_list args;

  va_start(args, message);
  vsnprintf(buf, sizeof(buf), message, args);
  va_end(args);

  send_to_all_except(buf, ch);
}


void give_prompt(DESC *desc) {
  const char *condition[] = {
    "bleeding",  /*   1% -  9% */
    "terrible",  /*  10% - 19% */
    "awful",     /*  20% - 29% */
    "bad",       /*  30% - 39% */
    "hurt",      /*  40% - 49% */
    "wounded",   /*  50% - 59% */
    "fair",      /*  60% - 69% */
    "good",      /*  70% - 79% */
    "fine",      /*  80% - 89% */
    "excellent", /*  90% - 99% */
    "full"       /* 100%       */
  };

  assert(desc);

  if (desc->str) {
    write_to_descriptor(desc->descriptor, "] ");
    desc->prompt_mode = 0;

    // When writing on a board or postcard, we only want the ]
    return;
  }

  if (desc->connected) return;

  if (desc->showstr_point) {
    write_to_descriptor(desc->descriptor, "*** Press return ***");
    desc->prompt_mode = 0;

    return;
  }

  CHAR *ch = desc->character;

  assert(ch);

  char prompt[MIL], buf[MIL];

  prompt[0] = '\0';

  bool color_enabled = GET_COMM_COLOR(ch, 0);
  char *prompt_color = Color[((GET_COMM_COLOR(ch, COMM_COLOR_PROMPT) * 2) - 2)];
  char *fg_color = Color[((GET_COMM_COLOR(ch, COMM_COLOR_FOREGROUND) * 2) - 2)];
  char *bg_color = BKColor[GET_COMM_COLOR(ch, COMM_COLOR_BACKGROUND)];

  if (color_enabled) {
    if (prompt_color) {
      str_cat(prompt, sizeof(prompt), prompt_color);
    }

    if (bg_color) {
      str_cat(prompt, sizeof(prompt), bg_color);
    }
  }

  str_cat(prompt, sizeof(prompt), "<");

  if (IS_SET(desc->prompt, PROMPT_NAME)) {
    str_cat(prompt, sizeof(prompt), " ");
  }

  if (IS_SET(desc->prompt, PROMPT_NAME)) {
    str_cat(prompt, sizeof(prompt), GET_NAME(ch));
  }

  if (IS_SET(desc->prompt, PROMPT_HP | PROMPT_HP_MAX | PROMPT_HP_TEX)) {
    str_cat(prompt, sizeof(prompt), " ");
  }

  if (IS_SET(desc->prompt, PROMPT_HP)) {
    snprintf(buf, sizeof(buf), "%d", GET_HIT(ch));
    str_cat(prompt, sizeof(prompt), buf);
  }

  if (IS_SET(desc->prompt, PROMPT_HP_MAX)) {
    snprintf(buf, sizeof(buf), "(%d)", GET_MAX_HIT(ch));
    str_cat(prompt, sizeof(prompt), buf);
  }

  if (IS_SET(desc->prompt, PROMPT_HP_TEX)) {
    str_cat(prompt, sizeof(prompt), "hp");
  }

  if (IS_SET(desc->prompt, PROMPT_MANA | PROMPT_MANA_MAX | PROMPT_MANA_TEX)) {
    str_cat(prompt, sizeof(prompt), " ");
  }

  if (IS_SET(desc->prompt, PROMPT_MANA)) {
    snprintf(buf, sizeof(buf), "%d", GET_MANA(ch));
    str_cat(prompt, sizeof(prompt), buf);
  }

  if (IS_SET(desc->prompt, PROMPT_MANA_MAX)) {
    snprintf(buf, sizeof(buf), "(%d)", GET_MAX_MANA(ch));
    str_cat(prompt, sizeof(prompt), buf);
  }

  if (IS_SET(desc->prompt, PROMPT_MANA_TEX)) {
    str_cat(prompt, sizeof(prompt), "mana");
  }

  if (IS_SET(desc->prompt, PROMPT_MOVE | PROMPT_MOVE_MAX | PROMPT_MOVE_TEX)) {
    str_cat(prompt, sizeof(prompt), " ");
  }

  if (IS_SET(desc->prompt, PROMPT_MOVE)) {
    snprintf(buf, sizeof(buf), "%d", GET_MOVE(ch));
    str_cat(prompt, sizeof(prompt), buf);
  }

  if (IS_SET(desc->prompt, PROMPT_MOVE_MAX)) {
    snprintf(buf, sizeof(buf), "(%d)", GET_MAX_MOVE(ch));
    str_cat(prompt, sizeof(prompt), buf);
  }

  if (IS_SET(desc->prompt, PROMPT_MOVE_TEX)) {
    str_cat(prompt, sizeof(prompt), "mv");
  }

  if (IS_SET(desc->prompt, PROMPT_BUFFER)) {
    CHAR *opponent = ((GET_OPPONENT(ch) && SAME_ROOM(ch, GET_OPPONENT(ch))) ? GET_OPPONENT(ch) : NULL);
    CHAR *buffer = ((opponent && GET_OPPONENT(opponent) && SAME_ROOM(ch, GET_OPPONENT(opponent)) && (GET_OPPONENT(opponent) != ch)) ? GET_OPPONENT(opponent) : NULL);

    if (buffer || IS_SET(desc->prompt, PROMPT_BUFFER_A)) {
      str_cat(prompt, sizeof(prompt), " Buf:");
    }

    if (buffer) {
      if (IS_SET(desc->prompt, PROMPT_BUFFER_TEX)) {
        snprintf(buf, sizeof(buf), "%s", condition[MAX(MIN((GET_MAX_HIT(buffer) ? ((100 * GET_HIT(buffer)) / GET_MAX_HIT(buffer)) : 0) / 10, 10), 0)]);
        str_cat(prompt, sizeof(prompt), buf);
      }
      else {
        snprintf(buf, sizeof(buf), "%d%%", GET_MAX_HIT(buffer) ? (int)(((double)GET_HIT(buffer) / (double)GET_MAX_HIT(buffer)) * 100.0) : 0);
        str_cat(prompt, sizeof(prompt), buf);
      }
    }
    else if (buffer || IS_SET(desc->prompt, PROMPT_BUFFER_A)) {
      str_cat(prompt, sizeof(prompt), "*");
    }
  }

  if (IS_SET(desc->prompt, PROMPT_VICTIM)) {
    CHAR *opponent = ((GET_OPPONENT(ch) && SAME_ROOM(ch, GET_OPPONENT(ch))) ? GET_OPPONENT(ch) : NULL);

    if (opponent || IS_SET(desc->prompt, PROMPT_VICTIM_A)) {
      str_cat(prompt, sizeof(prompt), " Vic:");
    }

    if (opponent) {
      if (IS_SET(desc->prompt, PROMPT_VICTIM_TEX)) {
        snprintf(buf, sizeof(buf), "%s", condition[MAX(MIN((GET_MAX_HIT(opponent) ? (100 * GET_HIT(opponent)) / GET_MAX_HIT(opponent) : 0) / 10, 10), 0)]);
        str_cat(prompt, sizeof(prompt), buf);
      }
      else {
        snprintf(buf, sizeof(buf), "%d%%", GET_MAX_HIT(opponent) ? (int)(((double)GET_HIT(opponent) / (double)GET_MAX_HIT(opponent)) * 100.0) : 0);
        str_cat(prompt, sizeof(prompt), buf);
      }
    }
    else if (opponent || IS_SET(desc->prompt, PROMPT_VICTIM_A)) {
      str_cat(prompt, sizeof(prompt), "*");
    }
  }

  str_cat(prompt, sizeof(prompt), " > ");

  if (color_enabled) {
    if (fg_color) {
      str_cat(prompt, sizeof(prompt), fg_color);
    }

    if (bg_color) {
      str_cat(prompt, sizeof(prompt), bg_color);
    }
  }

  if (IS_SET(GET_PFLAG(ch), PLR_PROMPT_NEWLINE)) {
    str_cat(prompt, sizeof(prompt), "\n\r");
  }

  write_to_descriptor(desc->descriptor, prompt);

  desc->prompt_mode = 0;
}


int signal_world(CHAR *signaler, int cmd, char *arg) {
  bool stop = FALSE;

  for (int zone_rnum = 0; !stop && (zone_rnum <= top_of_zone_table); zone_rnum++) {
    stop = signal_zone(zone_rnum, signaler, cmd, arg);
  }

  return stop;
}


int signal_zone(int zone_rnum, CHAR *signaler, int cmd, char *arg) {
  if (zone_table[zone_rnum].reset_mode == ZRESET_MODE_BLOCK) {
    if (cmd == MSG_ZONE_RESET) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  int start = 0;

  if ((zone_table[zone_rnum].bottom == -1) || (real_room(zone_table[zone_rnum].bottom) == -1)) {
    start = 0;
  }
  else {
    start = real_room(zone_table[zone_rnum].bottom);
  }

  int finish = 0;

  if ((zone_table[zone_rnum].top == -1) || (real_room(zone_table[zone_rnum].top) == -1)) {
    finish = top_of_world;
  }
  else {
    finish = real_room(zone_table[zone_rnum].top);
  }

  for (int room_rnum = start, started = FALSE; room_rnum <= finish; room_rnum++) {
    if (world[room_rnum].zone == zone_table[zone_rnum].virtual) {
      if (zone_table[zone_rnum].bottom == -1) {
        zone_table[zone_rnum].bottom = world[room_rnum].number;
      }

      started = TRUE;

      if (signal_room(room_rnum, signaler, cmd, arg)) {
        return TRUE;
      }
    }
    else if (started) {
      if (room_rnum > 0) {
        zone_table[zone_rnum].top = world[room_rnum - 1].number;
      }

      break;
    }
  }

  return FALSE;
}

int signal_room(int room_rnum, CHAR *signaler, int cmd, char *arg) {
  if (room_rnum == NOWHERE) return FALSE;

  if (cmd == MSG_TICK) {
    if (ROOM_BLOOD(room_rnum) > 0) {
      ROOM_BLOOD(room_rnum)--;
    }
  }

  bool stop = FALSE;

  if (!stop && world[room_rnum].funct) {
    stop = room_special(room_rnum, signaler, cmd, arg);
  }

  for (CHAR *temp_ch = world[room_rnum].people, *next_ch; !stop && temp_ch; temp_ch = next_ch) {
    next_ch = temp_ch->next_in_room;

    if ((cmd == MSG_TICK) && !IS_NPC(temp_ch) && IS_SET(GET_PFLAG(temp_ch), PLR_TICKER)) {
      send_to_char("TICK!\n\r", temp_ch);
    }

    stop = signal_char(temp_ch, signaler, cmd, arg);
  }

  for (OBJ *temp_obj = world[room_rnum].contents, *next_obj = NULL; !stop && temp_obj; temp_obj = next_obj) {
    next_obj = temp_obj->next_content;

    stop = signal_object(temp_obj, signaler, cmd, arg);
  }

  if (stop && ((cmd == MSG_MOBACT) || (cmd == MSG_TICK) || (cmd == MSG_ZONE_RESET))) {
    char buf[MSL];

    snprintf(buf, sizeof(buf), "WIZINFO: signal_room(%d, %s, %d, %s) returned TRUE.",
      world[room_rnum].number, signaler ? GET_DISP_NAME(signaler) : "(null)", cmd, arg ? arg : "(null)");
    wizlog(buf, 1, 6);

    stop = FALSE;
  }

  return stop;
}

int signal_char(CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  assert(ch);

  /* Sanity check. */
  if ((CHAR_REAL_ROOM(ch) == NOWHERE) || ((ch->nr >= 0) && (ch->nr <= top_of_mobt) && (ch->nr_v && (ch->nr_v != mob_proto_table[ch->nr].virtual)))) {
    return FALSE;
  }

  bool stop = FALSE;

  /* Call special character function (char_spec.c). */
  if (ch->func) {
    stop = (*ch->func)(ch, signaler, cmd, arg);
  }

  /* Signal equipped objects. */
  for (int eq_pos = 0; !stop && (eq_pos < MAX_WEAR); eq_pos++) {
    OBJ *temp_obj = EQ(ch, eq_pos);

    if (temp_obj) {
      stop = signal_object(temp_obj, signaler, cmd, arg);
    }
  }

  /* Signal carried objects. */
  for (OBJ *temp_obj = ch->carrying, *next_obj; !stop && temp_obj; temp_obj = next_obj) {
    next_obj = temp_obj->next_content;

    stop = signal_object(temp_obj, signaler, cmd, arg);
  }

  /* Signal enchantments. */
  for (ENCH *temp_ench = ch->enchantments, *next_ench; !stop && temp_ench; temp_ench = next_ench) {
    next_ench = temp_ench->next;

    stop = enchantment_special(temp_ench, ch, signaler, cmd, arg);
  }

  /* Signal NPCs. */
  if (IS_NPC(ch)) {
    if (cmd == MSG_MOBACT) {
      mobile_activity(ch);
    }

    if (mob_proto_table[ch->nr].func) {
      stop = mob_special(ch, signaler, cmd, arg);
    }
  }

  if (stop) {
    switch (cmd) {
      case MSG_VIOLENCE:
      case MSG_VIOLENCE_POST_HIT:
      case MSG_ROUND:
      case MSG_TICK:
      case MSG_MOBACT:
      case MSG_ZONE_RESET:
      stop = FALSE;

      wizlog_f(1, 6, "WIZINFO: signal_char(%s, %s, %d, %s) returned TRUE.",
        ch ? GET_DISP_NAME(ch) : "(null)", signaler ? GET_DISP_NAME(signaler) : "(null)", cmd, arg ? arg : "(null)");
      break;
    }
  }

  return stop;
}


int signal_object(OBJ *obj, CHAR *signaler, int cmd, char *arg) {
  assert(obj);

  bool stop = FALSE;

  if (!stop && (OBJ_RNUM(obj) >= 0) && (OBJ_FUNC(obj) || obj_proto_table[obj->item_number].func)) {
    stop = obj_special(obj, signaler, cmd, arg);
  }

  if (!stop && (OBJ_TYPE(obj) == ITEM_BOARD)) {
    stop = board(obj, signaler, cmd, arg);
  }

  switch (cmd) {
    case MSG_TICK:
      for (OBJ *temp_obj = obj->contains, *next_obj; !stop && temp_obj; temp_obj = next_obj) {
        next_obj = temp_obj->next_content;

        stop = signal_object(temp_obj, signaler, cmd, arg);
      }
      break;
  }

  if (stop) {
    switch (cmd) {
      case MSG_VIOLENCE:
      case MSG_VIOLENCE_POST_HIT:
      case MSG_ROUND:
      case MSG_TICK:
      case MSG_MOBACT:
      case MSG_ZONE_RESET:
        stop = FALSE;

        wizlog_f(1, 6, "WIZINFO: signal_object(%s, %s, %d, %s) returned TRUE.",
          obj ? OBJ_SHORT(obj) : "(null)", signaler ? GET_DISP_NAME(signaler) : "(null)", cmd, arg ? arg : "(null)");
        break;
    }
  }

  return stop;
}


void flying_room(int room) {
  CHAR *vict,*next_vict;
  OBJ *obj,*next_obj;
  int fall_room,v_room;
  char buf[MSL];

  if(room==-1 || room==0) return;
  if(!IS_SET(world[room].room_flags,FLYING)) return;

  /* Check for a down exit */
  v_room=world[room].number;
  if(world[room].dir_option[DOWN]) {
    fall_room=world[room].dir_option[DOWN]->to_room_r;
    if(fall_room==room) {
      sprintf(buf, "WIZINFO: FLYING: Room %d has a loop to itself.", v_room);
      wizlog(buf, LEVEL_SUP, 6);
      return;
    }
    if(fall_room==0 || fall_room==-1) {
      sprintf(buf, "WIZINFO: FLYING: Room %d has a NOWHERE or VOID exit.", v_room);
      wizlog(buf, LEVEL_SUP, 6);
      return;
    }
  } else {
    sprintf(buf, "WIZINFO: FLYING: Room %d has no down exit.", v_room);
    wizlog(buf, LEVEL_SUP, 6);
    return;
  }

  /* Check characters in the room */
  for(vict=world[room].people;vict;vict=next_vict) {
    next_vict=vict->next_in_room;
    if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM) continue;

    if(!vict->specials.riding) {
      if(IS_AFFECTED(vict,AFF_FLY) && vict->specials.fighting) continue;
      if(!vict->specials.fighting && GET_POS(vict)==POSITION_FLYING) continue;
      if (IS_SET(GET_ACT(vict), ACT_FLY)) continue;
      if(vict->specials.rider) {
        /* Mount and rider fall */
        act("You and your mount, fall to the area below.",0,vict->specials.rider,0,0,TO_CHAR);
        act("$n and $s mount fall to the area below.",1,vict->specials.rider,0,0,TO_ROOM);

        /* Check fighting */
        if(vict->specials.fighting) {
          if(vict->specials.fighting->specials.fighting == vict)
            stop_fighting(vict->specials.fighting);
          stop_fighting(vict);
        }
        if (vict->specials.rider->specials.fighting) {
          if(vict->specials.rider->specials.fighting->specials.fighting == vict->specials.rider)
            stop_fighting(vict->specials.rider->specials.fighting);
          stop_fighting(vict->specials.rider);
        }

        char_from_room(vict->specials.rider);
        char_to_room(vict->specials.rider,fall_room);
        char_from_room(vict);
        char_to_room(vict,fall_room);
        GET_HIT(vict->specials.rider)=GET_HIT(vict->specials.rider)*3/4;
        GET_MOVE(vict->specials.rider)=GET_MOVE(vict->specials.rider)*3/4;
        GET_HIT(vict)=GET_HIT(vict)/2;
        GET_MOVE(vict)=GET_MOVE(vict)/2;
        act("$n and $s mount fall from above.",1,vict->specials.rider,0,0,TO_ROOM);
        act("You fall to the area below.",0,vict,0,0,TO_CHAR);
        do_look(vict->specials.rider,"",CMD_LOOK);
        do_look(vict,"",CMD_LOOK);
      } else {
        /* Just vict falls */
        act("$n falls to the area below.",1,vict,0,0,TO_ROOM);
        act("You fall to the area below.",0,vict,0,0,TO_CHAR);
        /* Check fighting */
        if(vict->specials.fighting) {
          if(vict->specials.fighting->specials.fighting == vict)
            stop_fighting(vict->specials.fighting);
          stop_fighting(vict);
        }
        char_from_room(vict);
        char_to_room(vict,fall_room);
        GET_HIT(vict)=GET_HIT(vict)/2;
        GET_MOVE(vict)=GET_MOVE(vict)/2;
        act("$n falls from above.",1,vict,0,0,TO_ROOM);
        do_look(vict,"",CMD_LOOK);
      }
      continue;
    }

    /* Must be riding something */
    if(IS_AFFECTED(vict->specials.riding,AFF_FLY) && vict->specials.riding->specials.fighting) continue;
    if(!vict->specials.riding->specials.fighting && GET_POS(vict->specials.riding)==POSITION_FLYING) continue;
    /* char and mount fall to down exit */
    act("You and your mount, fall to the area below.",0,vict,0,0,TO_CHAR);
    act("$n and $s mount fall to the area below.",1,vict,0,0,TO_ROOM);

    /* Check Fighting */
    if(vict->specials.riding->specials.fighting) {
      if(vict->specials.riding->specials.fighting->specials.fighting == vict->specials.riding)
        stop_fighting(vict->specials.riding->specials.fighting);
      stop_fighting(vict->specials.riding);
    }

    if(vict->specials.fighting) {
      if(vict->specials.fighting->specials.fighting == vict)
        stop_fighting(vict->specials.fighting);
      stop_fighting(vict);
    }

    char_from_room(vict);
    char_to_room(vict,fall_room);
    char_from_room(vict->specials.riding);
    char_to_room(vict->specials.riding,fall_room);
    GET_HIT(vict)=GET_HIT(vict)*3/4;
    GET_MOVE(vict)=GET_MOVE(vict)*3/4;
    GET_HIT(vict->specials.riding)=GET_HIT(vict->specials.riding)/2;
    GET_MOVE(vict->specials.riding)=GET_MOVE(vict->specials.riding)/2;
    act("$n and $s mount fall from above.",1,vict,0,0,TO_ROOM);
    act("You fall to the area below.",0,vict->specials.riding,0,0,TO_CHAR);
    do_look(vict,"",CMD_LOOK);
    do_look(vict->specials.riding,"",CMD_LOOK);
  }

  /* Check for objects in the room */

  for(obj=world[room].contents;obj;obj=next_obj) {
    next_obj=obj->next_content;
    if(!CAN_WEAR(obj,ITEM_TAKE)) continue;
    /* Make obj fall to room below */
    sprintf(buf, "The %s falls to the area below.\n\r",rem_prefix(OBJ_SHORT(obj)));
    send_to_room(buf,room);
    sprintf(buf,"The %s falls from above.\n\r",rem_prefix(OBJ_SHORT(obj)));
    send_to_room(buf,fall_room);
    obj_from_room(obj);
    obj_to_room(obj,fall_room);
  }
}

void game_sleep(struct timeval *timeout) {
  if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, timeout) < 0) {
    if (errno != EINTR) {
      log_s("SYSERR: Select sleep");

      exit(1);
    }
  }
}

void write_last_command() {
    if (!last_command[0]) return;

    int fd = open(LAST_COMMAND_FILE, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd < 0) return;

    write(fd, last_command, strlen(last_command));
    write(fd, "\n", 1);

    close(fd);
}

void write_board(int vnum, char *heading, char *message);
void examine_last_command () {
    FILE *fp = fopen(LAST_COMMAND_FILE, "r");

    if (!fp) return;

    char buf[MSL];

    fscanf (fp, "%[^\n]", buf);

    fclose (fp);

    unlink(LAST_COMMAND_FILE);

    write_board(3097, "Last recorded command before crash", buf);
}

void open_logfile() {
  backup_logfile();

  if (!(logfile = fopen("syslog", "w"))) {
    printf("Error: syslog file could not be opened.\n\r");
  }
}

void close_logfile() {
  fclose(logfile);

  backup_logfile();
}

void backup_logfile() {
  if (access("syslog", F_OK) == 0) {
    system("tail -2 syslog >> runlog");
    system("cp syslog syslogs/syslog.`date +%y%m%d.%H%M%S`");
    system("mv syslog syslog.last");
  }

  if (access("lib/plrlog", F_OK) == 0) {
    system("mv lib/plrlog syslogs/plrlog.`date +%y%m%d.%H%M%S`");
  }
}
