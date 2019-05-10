#include <iostream>
#include <sstream>
#include <curses.h>
#include <ncurses.h>
#include "prototypes.hpp"
#include "getChar.hpp"
#include <sys/types.h>	
#include <sys/socket.h>	
#include <netinet/in.h>	
#include <arpa/inet.h>
#include <thread>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <algorithm>

// PORT NUMBERS: 8028, 8029

#define SERVER_PORT 8028
#define CLIENT_PORT 8029
#define MAXLINE 200
#define LISTENQ 5
#define NUM_ROWS 25
#define OUTPUT_LINE 11
#define NUM_COLS 80
#define HORIZ_CUTOFF 65
#define QUIT '`'

void startup(void);
void terminate(void);

void receiveThread(int recFD);
void sendThread(int sendFD);

int listenForClient(int argc, char *argv[]);
void sendToClient(int connectionFD, char msg);
int connectToClient(int argc, char *argv[]);
char receiveFromClient(int connectionfd);


int main(int argc, char *argv[]){

    startup();
    move(NUM_ROWS / 2, 0);
    hline(ACS_HLINE, NUM_COLS);
    refresh();

    int sendFD = listenForClient(argc, argv);    
    int receiveFD = connectToClient(argc, argv);

    std::thread sndThread(&sendThread, sendFD);
    std::thread recThread(&receiveThread, receiveFD);
    
    sndThread.join();
    recThread.join();
    terminate();

    return 0;
}

void receiveThread(int receiveFD){
    int charPos = 0;
    std::stringstream inputStream;
    std::vector<std::string> windowStrings;
    windowStrings.resize(0);

    char c;
    while(c != '`'){
        c = receiveFromClient(receiveFD);
        inputStream << c;
        // If we get to the end of the line, need to shift everything up
        if(charPos > HORIZ_CUTOFF || c == 10){
            clear();
            windowStrings.insert(windowStrings.begin(), inputStream.str());
            inputStream.str("");
            charPos = 0;
        }
        // Display past lines above output line
        int strIdx = 0;
        for(int line = NUM_ROWS / 2 - 2; line >= 0; line--){
            if(strIdx < windowStrings.size()){
                const char *currentLine = windowStrings[strIdx].c_str();
                mvaddstr(line, 0, currentLine);
            }
            strIdx++;
        }
        // Move to output line and display the string so far
        const char *output = inputStream.str().c_str();
        mvaddstr(NUM_ROWS / 2 - 1, 0, output);
        // Draw separating line
        move(NUM_ROWS / 2, 0);
        hline(ACS_HLINE, NUM_COLS);
        refresh();
        charPos++;
    }
    close(receiveFD);
}
void sendThread(int sendFD){

    int charPos = 0;

    std::stringstream inputStream;
    std::vector<std::string> windowStrings;
    windowStrings.resize(0);

    char c;
    c = get_char();
    while(c != '`'){
        c = get_char();
        sendToClient(sendFD, c);

        inputStream << c;
        // If we get to the end of the line, need to shift everything up
        if(charPos > HORIZ_CUTOFF || c == 10){
            clear();
            windowStrings.insert(windowStrings.begin(), inputStream.str());
            inputStream.str("");
            charPos = 0;
        }
        // Display past lines above output line
        int strIdx = 0;
        for(int line = NUM_ROWS - 2; line > NUM_ROWS / 2 ; line--){
            if(strIdx < windowStrings.size()){
                const char *currentLine = windowStrings[strIdx].c_str();
                mvaddstr(line, 0, currentLine);
            }
            strIdx++;
        }
        // Move to output line and display the string so far
        const char *output = inputStream.str().c_str();
        mvaddstr(NUM_ROWS - 1, 0, output);
        // Draw separating line
        move(NUM_ROWS / 2, 0);
        hline(ACS_HLINE, NUM_COLS);
        refresh();
        charPos++;
    }
    close(sendFD);
}

void sendToClient(int cListenFD, char c){
    char msgbuff[10];
    msgbuff[0] = c;
    if( write(cListenFD, msgbuff, strlen(msgbuff)) < 0 ) {
	    fprintf( stderr, "Write failed.  %s\n", strerror( errno ) );
	    exit( 1 );
	}
}

char receiveFromClient(int connectionfd){
    int n;
    char recvline[10];
	n = read(connectionfd, recvline, 1);
     recvline[n] = 0;	/* null terminate */
		// if (fputs(recvline, stdout) == EOF) {
          //      terminate();
		// 	fprintf( stderr, "fputs error: %s", strerror( errno ) );
		// 	exit( 5 );
		// }

	if (n < 0) {
          terminate();
	    fprintf( stderr, "read error: %s", strerror( errno ) );
	    exit( 6 );
	}
     return recvline[0];
}

int connectToClient(int argc, char *argv[]){
    int socketfd;
    struct sockaddr_in servaddr;


    // Create socket endpoint using IPv4 and a stream socket
    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
         fprintf(stderr, "Socket error. %s\n", strerror(errno));
         exit(2);
    }

    // Build a profile for the server
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(CLIENT_PORT);
    std::string ip = "127.0.0.1";
    const char *ip_addr = ip.c_str();
    if(inet_pton(AF_INET, ip_addr , &servaddr.sin_addr) <= 0){
         fprintf(stderr, "inet_pton error for 127.0.0.1");
         exit(3);
    }

    // Attempt to connect to the server
    if(connect(socketfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
         fprintf(stderr, "Connect error: %x\n", strerror(errno));
         exit(4);
    }

    return socketfd;
}

int listenForClient(int argc, char *argv[]){
    int listenfd, connectionfd;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    char buff[MAXLINE];

    // Create an endpoint for IPv4 connection
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Socket failed. %s\n", strerror(errno));
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // Communicate using the Internet domain (AF_INET)
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Who should we accept connections from?
    servaddr.sin_port = htons(SERVER_PORT); // Which port should the server listen on?

    // Bind the server endpoint using the specs stored in "serveraddr"
    if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        fprintf(stderr, "Bind failed. %s\n", strerror(errno));
        exit(1);
    }

    // Listen for the incoming connections, pile up at most LISTENQ # of connections
    if(listen(listenfd, LISTENQ) < 0){
        fprintf(stderr, "Listen failed. %s\n", strerror(errno));
        exit(1);
    }

    for(;;){
        len = sizeof(cliaddr);
        // Establish connection with incoming client
        if((connectionfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len)) < 0){
            fprintf(stderr, "Accept failed. %s\n", strerror(errno));
            exit(1);
        }
        // printf("Connection from %s, port %d\n",
        //         inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
        //         ntohs(cliaddr.sin_port));

        break; 
    }
    return connectionfd;
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