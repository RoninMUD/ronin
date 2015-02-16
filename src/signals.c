/* ************************************************************************
*  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
*  Usage : Signal Trapping.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2004/02/05 16:10:07 $
$Header: /home/ronin/cvs/ronin/signals.c,v 2.0.0.1 2004/02/05 16:10:07 ronin Exp $
$Id: signals.c,v 2.0.0.1 2004/02/05 16:10:07 ronin Exp $
$Name:  $
$Log: signals.c,v $
Revision 2.0.0.1  2004/02/05 16:10:07  ronin
Reinitialization of cvs archives

Revision 1.4  2002/03/31 09:22:16  ronin
Changed <time.h> back to <sys/time.h> in #include.

Revision 1.3  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "structs.h"
#include "utility.h"

#include "utils.h"

void checkpointing(int x);
void shutdown_request(int x);
void logsig(int x);
void hupsig(int x);

void signal_setup(void)
{
  struct itimerval itime;
  struct timeval interval;

  signal(SIGUSR2, shutdown_request);

  /* just to be on the safe side: */

  signal(SIGHUP, hupsig);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, hupsig);
  signal(SIGALRM, logsig);
  signal(SIGTERM, hupsig);

  /* set up the deadlock-protection */

  interval.tv_sec = 900;    /* 15 minutes */
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, 0);
  signal(SIGVTALRM, checkpointing);
}

void checkpointing(int x)
{
  extern int tics;

  if (!tics) {
    log_f("CHECKPOINT shutdown: tics not updated");
    abort();
  }
  else
    tics = 0;
}

void shutdown_request(int x)
{
  extern int cleanshutdown;

  log_f("Received USR2 - shutdown request");
  cleanshutdown = 1;
}

/* kick out players etc */
void hupsig(int x)
{
  extern int cleanshutdown;
  extern char last_command[MSL];
  cleanshutdown=1;
  last_command[0] = '\0';
  log_f("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
  exit(0);    /*something more elegant should perhaps be substituted */
}

void logsig(int x)
{
  log_f("Signal received. Ignoring.");
}
