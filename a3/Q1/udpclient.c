#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void func(int sockfd, SA* servaddr)
{
	char buff[MAX];
	int n, len;
	for (;;) {
		bzero(buff, MAX);
		printf("$ ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n');

		sendto(sockfd, buff, sizeof(buff), MSG_CONFIRM, servaddr, sizeof(*servaddr));
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
		bzero(buff, MAX);
		recvfrom(sockfd, buff, MAX, MSG_WAITALL, servaddr, &len);
		printf(buff);
	}
}

int main()
{
	int sockfd;
	struct sockaddr_in servaddr;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);


	// function for chat
	func(sockfd, (SA*)&servaddr);

	// close the socket
	close(sockfd);
}

