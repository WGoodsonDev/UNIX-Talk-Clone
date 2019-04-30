#include <iostream>
#include <sstream>
#include <curses.h>
#include <ncurses.h>
#include "prototypes.hpp"
#include "getChar.hpp"
#include <sys/types.h>	
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <vector>
#include <string.h>

// PORT NUMBERS: 8028, 8029

#define SERVER_PORT 8028
#define CLIENT_PORT  8029
#define MAXLINE 200
#define NUM_ROWS 25
#define NUM_COLS 80
#define HORIZ_CUTOFF 65
#define QUIT '`'

void startup(void);
void terminate(void);

int main(int argc, char *argv[]){
    int socketfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if(argc != 2){
         fprintf(stderr, "Usage: %s <IPaddress>\n", argv[0]);
         exit(1);
    }

    // Create socket endpoint using IPv4 and a stream socket
    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
         fprintf(stderr, "Socket error. %s\n", strerror(errno));
         exit(2);
    }

    // Build a profile for the server
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){
         fprintf(stderr, "inet_pton error for %s", argv[1]);
         exit(3);
    }

    // Attempt to connect to the server
    if(connect(socketfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
         fprintf(stderr, "Connect error: %x\n", strerror(errno));
         exit(4);
    }
    // Get curses going
    startup();

    char c = get_char();
    int breakLine = NUM_ROWS / 2;

    std::vector<std::string> topWindow[breakLine];
    std::vector<std::string> bottomWindow[breakLine];

    // Main loop
    while(c != QUIT){
         c = get_char();
         clear();
     //     for(int i = breakLine - 1; i >= 0; i--){
     //          char * str = new char[]
     //          const char* str = topWindow[i].c_str();
     //          mvaddstr(i, 0, str);
     //     }
     //     for(int i = NUM_ROWS - 1; i > breakLine; i--){
     //          const char* str = bottomWindow[i].c_str();
     //          mvaddstr(i, 0, str);
     //     }
         for(int i = 0; i < NUM_COLS; i++){
              move(breakLine, i);
              addch('_');
         }
         refresh();
    }
    terminate();

    // Talk to the server
    

    return 0;
}

void startup( void )
{
     initscr();     // activate curses
     curs_set(0);   // prevent the cursor to be displayed
     clear();	     // clear the screen that curses provides 
     noecho();      // prevent the input chars to be echoed to the screen
     cbreak();      // change the stty so that characters are delivered to the program as they are typed--no need to hit the return key!
}

void terminate( void )
{
     mvcur( 0, COLS - 1, LINES - 1, 0 );
     clear();
     refresh();
     endwin();
}
