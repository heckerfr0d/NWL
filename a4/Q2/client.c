#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 80
#define PORT 8081
#define SA struct sockaddr

void func(int sockfd)
{
	char buff[MAX];
	int n;
	while(1) {
		memset(buff, 0, MAX);
		printf("$ ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n');
		write(sockfd, buff, sizeof(buff));
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("[-]Client exits...\n");
			break;
		}
		memset(buff, 0, MAX);
		read(sockfd, buff, MAX);
		printf("[Server]: %s\n", buff);
	}
}

int main()
{
	int sockfd;
	struct sockaddr_in servaddr;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("[-]Socket creation failed...\n");
		exit(0);
	}
	else
		printf("[+]Socket successfully created..\n");
	memset(&servaddr, 0, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("[-]Connection with the server failed...\n");
		exit(0);
	}
	else
		printf("[+]Connected to the server..\n");

	// function for chat
	func(sockfd);

	// close the socket
	close(sockfd);
}

