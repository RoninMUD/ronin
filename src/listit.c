/* listit.c: this program reads a text file and prints a listing
   of the file with line numbers prepended to the source. */

/*
$Author: ronin $
$Date: 2004/02/05 16:09:37 $
$Header: /home/ronin/cvs/ronin/listit.c,v 2.0.0.1 2004/02/05 16:09:37 ronin Exp $
$Id: listit.c,v 2.0.0.1 2004/02/05 16:09:37 ronin Exp $
$Name:  $
$Log: listit.c,v $
Revision 2.0.0.1  2004/02/05 16:09:37  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>

void display_usage(void);

int line;

main( int argc, char *argv[] )
{
  char buf[256];
  FILE *fp;
  if( argc < 2 )
    {
      display_usage();
      exit(1);
    }

  if (( fp = fopen( argv[1], "r" )) == NULL )
    {
      fprintf( stderr, "Error opening file %s\n", argv[1] );
      exit(1);
    }

  line = 1;

  while (fgets(buf, 256, fp ) != NULL )
    fprintf( stdout, "%4d:\t%s", line++, buf );
    fclose(fp);
    printf("\n            ********************  EOF  ********************\n");
    return 0;
}

void display_usage(void)
{
  fprintf(stderr, "Proper usage is listit <filename>\n");
}

