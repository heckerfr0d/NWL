#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>

#define MAX 2048
#define PORT 8080
#define SA struct sockaddr

int sockfd;
volatile sig_atomic_t flag = 0;
char name[20];

void read_func()
{
	char buff[MAX];
	int n;
	while(1) {
		memset(buff, 0, MAX);
		if (!read(sockfd, buff, MAX)) {
			printf("Server disconnected\n");
			fflush(stdout);
			flag = 1;
			break;
		}
		printf("\r%s\n[%s]: ", buff, name);
		fflush(stdout);
		memset(buff, 0, MAX);
	}
}

void write_func()
{
	char buff[MAX], msg[MAX];
	int n;
	memset(buff, 0, MAX);
	while(1) {
		printf("\r[%s]: ", name);
		fflush(stdout);
		fflush(stdin);
		fgets(buff, MAX, stdin);
		buff[strlen(buff)-1] = 0;
		if ((strncmp(buff, "bye", 3)) == 0) {
			write(sockfd, buff, strlen(buff));
			printf("logged out\n");
			break;
		}
		write(sockfd, buff, strlen(buff));
		memset(buff, 0, MAX);
		// memset(msg, 0, MAX);
	}
	flag = 1;
}

int main()
{
	struct sockaddr_in servaddr;
	int password;
	char buff[40];
	memset(buff, 0, 40);

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("[-]Socket creation failed...\n");
		exit(0);
	}

	printf("Enter name: ");
	fgets(name, 20, stdin);
	name[strlen(name)-1] = 0;

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

	printf("Password: ");
	scanf("%d%c", &password, &buff[0]);
	fflush(stdin);
	fflush(stdout);
	// write(sockfd, &password, sizeof(password));
	sprintf(buff, "%s %d", name, password);

	write(sockfd, buff, strlen(buff));

	pthread_t read_t, write_t;
	pthread_create(&read_t, NULL, (void *)read_func, NULL);
	pthread_create(&write_t, NULL, (void *)write_func, NULL);

	while(1)
		if(flag)
			break;

	// close the socket
	close(sockfd);

	return 0;
}

