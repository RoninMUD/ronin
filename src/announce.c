/*
 *	announce - sits listening on a port, and whenever anyone connects
 *		   announces a message and disconnects them
 *
 *	Usage:	announce [port] < message_file
 *
 *	Author:	Lawrence Brown <lpb@cs.adfa.oz.au> 	Aug 90
 *
 *	Bits of code are adapted from the Berkeley telnetd sources
 */

/*
$Author: ronin $
$Date: 2004/02/05 16:08:48 $
$Header: /home/ronin/cvs/ronin/announce.c,v 2.0.0.1 2004/02/05 16:08:48 ronin Exp $
$Id: announce.c,v 2.0.0.1 2004/02/05 16:08:48 ronin Exp $
$Name:  $
$Log: announce.c,v $
Revision 2.0.0.1  2004/02/05 16:08:48  ronin
Reinitialization of cvs archives

Revision 1.3  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/


#define PORT	5000

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/signal.h>
#include <stdio.h>
#include <ctype.h>

extern	char **environ;
extern	int errno;
char	*Name;			/* name of this program for error messages */
char	msg[2048];

int
  main(argc, argv)
char *argv[];
{
  int s, ns, foo, opt, n;
  static struct sockaddr_in sin = { AF_INET };
  char *host, *inet_ntoa();
  char tmp[80];
  long ct;

  Name = argv[0];		/* save name of program for error messages  */
  sin.sin_port = htons((u_short)PORT);  /* Assume PORT */
  argc--, argv++;
  if (argc > 0) {		/*   unless specified on command-line       */
	sin.sin_port = atoi(*argv);
    sin.sin_port = htons((u_short)sin.sin_port);
  }

  strcpy(msg, "");
  strcpy(tmp, "");
  while (1) {
    if ((gets(tmp)) == NULL) break;
    strcat(tmp, "\r\n");
    strcat(msg, tmp);
  }
  msg[2048] = '\0';
  signal(SIGHUP, SIG_IGN);	/* get socket, bind port to it      */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("announce: socket");;
    exit(1);
  }
  opt = 1;
  setsockopt(s,SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt));
  if (bind(s, &sin, sizeof sin) < 0) {
    perror("bind");
    exit(1);
  }
  if ((foo = fork()) != 0) {
    fprintf(stderr, "announce: pid %d\t%d\n", foo,
	    ntohs((u_short)sin.sin_port));
    exit(0);
  } else {
/*
    setpriority(PRIO_PROCESS, getpid(), 10);
*/
  }
  if (listen(s, 1) < 0) {		/* start listening on port */
    perror("announce: listen");
    exit(1);
  }
  foo = sizeof sin;
  for(;;) {	/* loop forever, accepting requests & printing msg */
    ns = accept(s, &sin, &foo);
    if (ns < 0) {
      perror("announce: accept");
      exit(1);
    }
	 host = "Unknown";
    /* host = inet_ntoa(sin.sin_addr.s_addr); */
#ifdef NOTSUN
#else
    /* host = inet_ntoa(sin.sin_addr.S_un.S_addr); */

#endif /* NOTSUN */


    ct = time(0L);
    fprintf(stderr, "CONNECTION made from %s at %s",
	    host, ctime(&ct));
    write(ns, msg, strlen(msg));
    sleep(5);
    close(ns);
    sleep(5);
  }
}/* main */
