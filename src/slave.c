/*
    This slave does iptoname conversions, and identquery lookups.

    The philosophy is to keep this program as simple/small as possible.
    It does normal fork()s, so the smaller it is, the faster it goes.
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:09 $
$Header: /home/ronin/cvs/ronin/slave.new.c,v 2.0.0.1 2004/02/05 16:10:09 ronin Exp $
$Id: slave.new.c,v 2.0.0.1 2004/02/05 16:10:09 ronin Exp $
$Name:  $
$Log: slave.new.c,v $
Revision 2.0.0.1  2004/02/05 16:10:09  ronin
Reinitialization of cvs archives

Revision 1.7  2002/07/21 20:53:16  test
Modified read to read up to 35 characters at once instead of looping
in a while loop until it reached a line feed.  Now the slave is
returning hostname lookups to the game.

Revision 1.6  2002/07/07 20:42:05  ronin
Modified char * cast to be a sockaddr cast instead, thus
eliminating another set of warnings.  Completely removed
declarations for some vars that weren't used. -- Lem

Revision 1.5  2002/07/07 20:03:30  ronin
Took out the #ifdef BCHS shit since it was causing
warnings.  Also commented some unused var declarations,
also causing warnings.  Now the slave actually compiles
on this machine.  Go figure.  Still some errors with
the hostname lookup, which I will tackle next. --Lem

Revision 1.4  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#define TRUE 1
#define FALSE 0
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include "slave.h"
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#define MAX_STRING 4096

pid_t child_PID;
char *arg_for_errors;

int my_write( int s, char *buf, size_t len )
{
/*    fprintf(stderr,"{SLAVE} Sent %s\r\n", buf);*/
    return( 1 ? write( s, buf, len ) : send( s, buf, len, 0 ) );
}


int my_read( int s, char *buf, size_t len )
{
    return( 1 ? read( s, buf, len ) : recv( s, buf, len, 0 ) );
}

void term_signal(int x)
{
   fprintf(stdout, "{SLAVE} Got SIGTERM\n\r");
   if (child_PID)
       {
       fprintf(stdout, "{SLAVE} Sending SIGTERM to child pid#%d\n\r", child_PID);
       kill(child_PID, SIGTERM);
       }
   exit(0);
}
void wake_up(int x)
{
   if(getppid() == 1)
     {
     fprintf(stderr, "{SLAVE} Parent has died :( \n\r");
     exit(0);
     }
   signal(SIGALRM, wake_up);
   alarm(30);
}

void timeout_signal(int x)
{
    /* fprintf( stderr, "{SLAVE}'%s' 5 minute timeout\r\n", arg_for_errors ); */
    exit( 1 );
}

int query( char *orig_arg )
{
    struct hostent *hp;
    struct sockaddr_in sin;
    int s;
    FILE *f;
    char result[ MAX_STRING ];
    char buf[ MAX_STRING+128 ];
    char arg[ MAX_STRING ];
    char *host;
    char *port_pair;
    size_t len;
    char *p;

    arg_for_errors = orig_arg;
    strcpy( arg, orig_arg );
    host = strtok(arg, ",");
    port_pair = strtok(NULL, "\n");

    hp = gethostbyname(host);
    if( hp == NULL ) {
       static struct hostent def;
       static struct in_addr defaddr;
       static char *alist[1];
       static char namebuf[128];

       defaddr.s_addr = inet_addr(arg);
       if (defaddr.s_addr == -1) {
           fprintf( stderr, "'%s': unknown host", orig_arg );
           return( -1 );
       }
       strcpy(namebuf, arg);
       def.h_name = namebuf;
       def.h_addr_list = alist;
       def.h_addr = (char *)&defaddr;
       def.h_length = sizeof (struct in_addr);
       def.h_addrtype = AF_INET;
       def.h_aliases = 0;
       hp = &def;
    }
    sin.sin_family = hp->h_addrtype;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons( 113 );      /* ident port */
    s = socket(hp->h_addrtype, SOCK_STREAM, 0);
    if( s < 0 ) {
       fprintf( stderr, "'%s': socket: %s", orig_arg, strerror( errno ) );
       return (-1);
    }
    if( connect(s, (struct sockaddr *)&sin, sizeof (sin)) < 0 ) {
       if( errno != ECONNREFUSED && errno != ETIMEDOUT ) {
           fprintf( stderr, "'%s': connect: %s", orig_arg, strerror( errno ) );
       }
       close(s);
      sprintf(buf, "%c%s CONNECTION REFUSED\n", SLAVE_IDENTQ, inet_ntoa(sin.sin_addr));
      my_write( 1, buf, strlen(buf) );
       return( -1 );
    }
    sprintf(buf, "%s\r\n", port_pair);
    len = strlen( buf );
    if( write(s, buf, len) != len ) {
       fprintf( stderr, "'%s': write: %s", orig_arg, strerror( errno ) );
       close(s);
       return( -1 );
    }
    f = fdopen(s, "r");
    {
       int c;

       p = result;
       while( ( c = fgetc( f ) ) != EOF ) {
           if( c == '\n' ) break;
           if( isprint( c ) ) {
               *p++ = c;
               if( p - result == MAX_STRING - 1 ) break;
           }
       }
       *p = 0;
    }
    (void)fclose(f);
    len = strlen(result);
    buf[0] = SLAVE_IDENTQ;
    sprintf(buf, "%c%s %s\n", SLAVE_IDENTQ, inet_ntoa(sin.sin_addr), result);
    my_write( 1, buf, strlen(buf) );
    return( 0 );
}


int iptoname( char *arg )
{
    unsigned long addr;
    struct hostent *hp;
    char buf[ MAX_STRING ];

    arg_for_errors = arg;

    addr = inet_addr( arg );
    if( addr == -1 ) {
       fprintf( stderr, "%s is not a valid decimal ip address", arg );
       return( -1 );
    }

    hp = gethostbyaddr( &addr, sizeof(addr), AF_INET );
    sprintf(buf, "%c%s %s\n", SLAVE_IPTONAME, arg, (hp ? hp->h_name : arg));
    my_write( 1, buf, strlen(buf) );

    return( 0 );
}


void  child_signal(int x)
{
    /* collect any children */
    while( waitpid( 0, NULL, WNOHANG ) > 0 )
       ;
    signal( SIGCHLD, child_signal );
}


int main( int argc, char **argv )
{
    char arg[ MAX_STRING +1 ];
    char tmp[31];
    int len;

    fprintf( stderr, "{OUCH} slave booted\r\n" );
    signal( SIGCHLD, child_signal );
    signal( SIGTERM, term_signal );
    signal( SIGPIPE, SIG_DFL );

    signal(SIGALRM, wake_up);
    alarm(30);

    for(;;) {
       memset(arg, 0, MAX_STRING);
       memset(tmp, 0, 31);
       len = my_read( 0, tmp, 31);

       if( len < 0 ) {
         if( errno == EINTR )
           errno = 0;
         else
           fprintf( stderr, "read: %s\r\n", strerror( errno ) );
       }
       else {
         strncpy(arg, tmp,len);
       }

       /* fprintf(stderr,"\r\n{SLAVE}received: '%s'\r\n", arg ); */

       child_PID = fork();
       switch( child_PID ) {
       case -1:

          fprintf( stderr, "\r\n{SLAVE}'%s': fork: %s\r\n", arg, strerror( errno ) );
           exit( 1 );

       case 0: /* child */
           signal(SIGALRM, timeout_signal);
           alarm(120);
           switch( arg[0] ) {
           case SLAVE_IDENTQ:
               exit( query( arg+1 ) != 0 );
           case SLAVE_IPTONAME:
               exit( iptoname( arg+1 ) != 0 );
           default:
               fprintf( stderr, "{SLAVE}invalid arg: %s\r\n", arg );
               break;
           }
           break;
        default:
           break;
       }
       /* collect any children */
       while( waitpid( 0, NULL, WNOHANG ) > 0 )
           ;
    } /* for */
    fprintf( stderr, "{SLAVE}exiting\r\n" );
    exit( 0 );
}
