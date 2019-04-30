#include<sys/types.h>	
#include<sys/socket.h>	
#include<arpa/inet.h>
#include<netinet/in.h>	
#include<sys/time.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>

// PORT NUMBERS: 8028, 8029

#define SERVER_PORT 8028
#define CLIENT_PORT 8029
#define MAXLINE 200
#define LISTENQ 5



int main(int argc, char *argv[]){
    int listenfd, connectionfd;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    char buff[MAXLINE];
    time_t ticks;

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
        printf("Connection from %s, port %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
                ntohs(cliaddr.sin_port));

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\n", ctime(&ticks));
        printf("Connection from %s, port %d\n", ctime(&ticks));
        if(write(connectionfd, buff, strlen(buff)) < 0){
            fprintf( stderr, "Write failed.  %s\n", strerror( errno ) );
            exit( 1 );
        }
        // Finished talking to this client. Close the connection
        close(connectionfd);
    }
    

    return 0;
}