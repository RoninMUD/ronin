/* ************************************************************************
*  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
*  Usage : Signal Trapping.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "utility.h"

#include "utils.h"

void write_last_command(void);

void checkpointing(int x);
void shutdown_request(int x);
void logsig(int x);
void hupsig(int x);

void handler_sigsegv (int signum, siginfo_t *info, void *context) {
  void *array[10];
  size_t size = 0;
/*
  struct sigaction action = {
    .sa_handler = SIG_DFL,
    .sa_sigaction = NULL,
    .sa_mask = {{0}},
    .sa_flags = 0,
    .sa_restorer = NULL
  };
*/
  fprintf(stderr, "SIGSEGV received at address: %p - ", info->si_addr);

  switch (info->si_code) {
    case SEGV_MAPERR:
      fprintf(stderr, "Address not mapped.\n");
      break;

    case SEGV_ACCERR:
      fprintf(stderr, "Access to this address is not allowed.\n");
      break;

    default:
      fprintf(stderr, "Unknown reason = %d.\n", info->si_code);
      break;
  }

  size = backtrace(array, 10);
  backtrace_symbols_fd(array, size, STDERR_FILENO);

  write_last_command();

  /* unregister and let the default action occur */
  //sigaction(SIGSEGV, &action, NULL);
  signal(signum, SIG_DFL);
  kill(getpid(), signum);
}


void signal_setup(void)
{
  struct itimerval itime;
  struct timeval interval;

  struct sigaction sigsegv_action = {
    .sa_handler = NULL,
    .sa_sigaction = handler_sigsegv,
    .sa_mask = {{0}},
    .sa_flags = SA_SIGINFO,
    .sa_restorer = NULL
  };


  if (sigaction(SIGSEGV, &sigsegv_action, NULL) < 0) {
    perror("sigaction");
  }

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
  if (!tics) {
    log_f("CHECKPOINT shutdown: tics not updated");
    abort();
  }
  else
    tics = 0;
}

void shutdown_request(int x)
{
  log_f("Received USR2 - shutdown request");
  cleanshutdown = 1;
}

/* kick out players etc */
void hupsig(int x)
{
  cleanshutdown=1;
  last_command[0] = '\0';
  log_f("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
  exit(0);    /*something more elegant should perhaps be substituted */
}

void logsig(int x)
{
  log_f("Signal received. Ignoring.");
}
