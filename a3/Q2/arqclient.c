#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
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

int fid = 0, sockfd, ackd = 1;
frame sendf, recvf;

void alarm_handler(int signum) {
    printf("Timeout\n");
    write(sockfd, &sendf, sizeof(sendf));
    printf("Resent SEQ: %d\n", sendf.seq);
    alarm(1);
}

void arq_write()
{
    clock_t t;
    if (ackd) {
        bzero(&sendf, sizeof(sendf));
        sendf.type = SEQ;
        sendf.seq = fid;
        sendf.ack = 0;
        write(sockfd, &sendf, sizeof(sendf));
        t = clock();
        alarm(1);
        printf("Sent SEQ: %d\n", sendf.seq);
    }
    bzero(&recvf, sizeof(recvf));
    int n = read(sockfd, &recvf, sizeof(recvf));
    if (n>0 && recvf.seq == 0 && recvf.ack == fid+1) {
        alarm(0);
        t = clock() - t;
        printf("Received ACK. RTT: %d\n", t);
        ackd = 1;
    }
    fid++;
}

void func()
{
	int n1, n2;
    FILE *fp = fopen("data.mp4", "rb");
    printf("Message size limits: ");
    scanf("%d%d", &n1, &n2);
	for (int i = n1;i <= n2; i+=200) {
		// if (n1 == -1) {
		// 	printf("Client Exit...\n");
        //     arq_write("exit", 4);
		// 	break;
		// }
        // else {
            // for(int i = n1; i <= n2; i+=200) {
                printf("Messsage size: %d\n", i);
                fread(sendf.data, 1, i, fp);
                arq_write();
            // }
        // }
	}
    fclose(fp);
}

int main()
{
	struct sockaddr_in servaddr, cli;

    signal(SIGALRM, (void (*)(int)) alarm_handler);

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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// function for chat
	func();

    sendf.seq = fid;
    sendf.type = FIN;
    sendf.ack = 0;
    sendf.data[0] = 0;
    write(sockfd, &sendf, sizeof(sendf));

	// close the socket
	close(sockfd);
}

