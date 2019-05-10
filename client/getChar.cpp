#include <iostream>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <Termios.h>
#include <ncurses.h>
#include <signal.h>

#include "getChar.hpp"

const int SEC_DELAY = 5;
const int USEC_DELAY = 100000;
static char lastc = ' ';
void die( void );

char get_char( )
{
     char	c;
     struct timeval tvec;
     fd_set rfds;

     tvec.tv_sec = SEC_DELAY;
     tvec.tv_usec = USEC_DELAY;
     FD_ZERO(&rfds);
     FD_SET(0, &rfds);

     int retval = select(1, &rfds, NULL, NULL, &tvec);

     if ( retval < 0 )
	 die();
     
     if ( retval ) {
         if ( read(0, &c, 1) < 0)
             die();
         lastc = c;
     }
     return( lastc );
}

void die( void )
{
     signal( SIGINT, SIG_IGN );
     mvcur( 0, COLS - 1, LINES - 1, 0 );
     endwin();
     exit( 1 );
}
