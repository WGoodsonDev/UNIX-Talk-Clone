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
#define OUTPUT_LINE 11
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
    int lineCharCount = 0;

    std::stringstream inputStream;
    std::vector<std::string> topWindow;
    std::vector<std::string> bottomWindow;
    topWindow.resize(0);
    bottomWindow.resize(0);

    // Main loop
    while(c != QUIT){
         c = get_char();
     //     clear();
     //     move(OUTPUT_LINE, 0);
         // Each new character gets added to inputStream, and the current horizontal cursor position increments
         inputStream << c;
         lineCharCount++;
         // Check for backspace NOT WORKING
     //     if(c == 127){
     //          std::string temp = inputStream.str();
     //          temp.pop_back();
     //          inputStream.str(temp);
     //          lineCharCount--;
     //     } else {
     //          lineCharCount++;
     //     }
         
         // If we get to the end of the line, need to shift everything up
         if(lineCharCount > HORIZ_CUTOFF || c == 10){
              clear();
              topWindow.push_back(inputStream.str());
              inputStream.str("");

              lineCharCount = 0;
         }
         // Display past lines above output line
         int strIdx = 0;
          for(int line = OUTPUT_LINE - 1; line > 0; line--){
               if(strIdx < topWindow.size()){
                    const char *currentLine = topWindow[strIdx].c_str();
                    mvaddstr(line, 0, currentLine);
               }
               strIdx++;
          }
          // Move to output line and display the string so far
          const char *output = inputStream.str().c_str();
          mvaddstr(OUTPUT_LINE, 0, output);
         for(int i = 0; i < NUM_COLS; i++){
          //     std::string lineNum = std::to_string(i);
          //     mvaddch(i, 0, lineNum[0]);
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
