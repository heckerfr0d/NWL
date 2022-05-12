#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
#define ACK 0
#define SEQ 1
#define FIN 2

typedef struct Frame {
    int type;
    int seq;
    int ack;
    char data[4096];
} frame;

int fid = 0;
frame sendf, recvf;


void arq_read(int connfd)
{
    bzero(&recvf, sizeof(recvf));
    int n = read(connfd, &recvf, sizeof(recvf));
    if (n>0 && recvf.type == SEQ && recvf.seq == fid) {
        printf("Received SEQ: %d\n", recvf.seq);
        bzero(&sendf, sizeof(sendf));
        sendf.type = ACK;
        sendf.seq = 0;
        sendf.ack = recvf.seq + 1;
        write(connfd, &sendf, sizeof(sendf));
        printf("Sent ACK: %d\n", sendf.ack);
        fid++;
    }
    else if(recvf.type != FIN) {
        printf("Frame dropped\n");
        printf("%d %d %d %d %d", recvf.type, recvf.seq, recvf.ack, fid, n);
    }
}


// Function designed for chat between client and server.
void func(int connfd)
{

    // infinite loop for chat
    for (;;) {

        // read the message from client and copy it in buffer
        arq_read(connfd);

        // if msg contains "Exit" then server exit and chat ended.
        if (recvf.type == FIN) {
            printf("Server Exit...\n");
            break;
        }
    }
}

// Driver function
int main()
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");

    // Function for chatting between client and server
    func(connfd);

    // After chatting close the socket
    close(sockfd);
}
