#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#define MAX 4096
#define PORT 4035
#define SA struct sockaddr
#define MIN(a,b) ((a)<(b)?(a):(b))

char username[40], password[40];

typedef struct Mail {
    char from[50], to[50], subject[80], time[25], body[50][80];
} mail;


void manage(int sockfd)
{
	char buff[MAX];
	char sender[50];
	int n;
	bzero(buff, MAX);
	read(sockfd, buff, MAX);
	printf("%s\nsender: ", buff);
	scanf("%s", sender);
	bzero(buff, MAX);
	sprintf(buff, "FLTR %s", sender);
	write(sockfd, buff, strlen(buff));
	for (;;) {
        int ip;
        char c;
		
	
	bzero(buff, MAX);
	read(sockfd, buff, MAX);
	printf("%s\n", buff);
	if(!strncmp(buff, "INVALID", 6)) {
		printf("sender: ", buff);
		scanf("%s", sender);
		write(sockfd, sender, strlen(sender));
		bzero(buff, MAX);
		read(sockfd, buff, MAX);
		if(!strncmp(buff, "INVALID", 6))
			break;	
	}
	if (!strncmp(buff, "NO MAILS TO SHOW", 17))
		break;
	
	scanf("%d", &ip);
        if ((char)ip == 'q')
            break;
        else {
            bzero(buff, MAX);
            sprintf(buff, "GET %d", ip);
            write(sockfd, buff, strlen(buff));
            bzero(buff, MAX);
            read(sockfd, buff, MAX);
            printf("%s\n", buff);
            while((c = getc(stdin)) == '\n');
            if (c == 'q')
                break;
            else if (c == 'd') {
                bzero(buff, MAX);
                sprintf(buff, "DEL %d", ip);
                write(sockfd, buff, strlen(buff));
                read(sockfd, buff, MAX);
		printf("%s\n", buff);
                bzero(buff, MAX);
            }
            // else {
            //     write(sockfd, "hehe", 4);
            // }
        }
	}
}

int find(char *s, char c)
{
    for(int i=0; s[i]; i++)
        if (s[i] == c)
            return i;
    return 0;
}

void send_mail(int sockfd)
{
    while(1) {
        char from[50], to[50], subject[80], line[80], buff[MAX];
        system("clear");
        printf("From: ");
        scanf("%s", from);
        printf("To: ");
        scanf("%s", to);
        getc(stdin);
        printf("Subject: ");
        fgets(subject, 50, stdin);
        // getc(stdin);
        sprintf(buff, "From: %s\nTo: %s\nSubject: %s", from, to, subject);
        do {
            fgets(line, 80, stdin);
            strcat(buff, line);
        } while(line[0]!='.');
        if (!strncmp(username, from, strlen(username)) && from[strlen(username)] == '@' && find(to, '@')) {
            write(sockfd, buff, strlen(buff));
            break;
        }
        else {
            printf("Incorrect format\n");
            // getc(stdin);
        }
    }
}

int main(int argc, char* argv[])
{
	int sockfd, connfd, my_port, pop3_port, opt;
	struct sockaddr_in servaddr, cli;

    sscanf(argv[1], "%d", &my_port);
    sscanf(argv[2], "%d", &pop3_port);

    bzero(&servaddr, sizeof(servaddr));
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);

    char buff[MAX];
    while(1) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            printf("socket creation failed...\n");
            exit(0);
        }
        // system("clear");
        printf("1. Manage Mail\n2. Send Mail\n3. Quit\n\n");

        scanf("%d", &opt);

        switch (opt)
        {
        case 1:
            servaddr.sin_port = htons(pop3_port);
            if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
                printf("connection with the server failed...\n");
                exit(1);
            }
            memset(buff, 0, MAX);
            sprintf(buff, "%s %s", username, password);
            write(sockfd, buff, strlen(buff));
            memset(buff, 0, MAX);
            read(sockfd, buff, 2);
            if (buff[0] == 'O') {
                printf("Welcome, %s\n", username);
                manage(sockfd);
                close(sockfd);
            }
            else {
                printf("Invalid Credentials\n");
                exit(1);
            }
            break;
        case 2:
            bzero(&servaddr, sizeof(servaddr));
            // assign IP, PORT
            servaddr.sin_family = AF_INET;
            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
            servaddr.sin_port = htons(my_port);
            if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
                printf("connection with the server failed...\n");
                exit(1);
            }
            memset(buff, 0, MAX);
            sprintf(buff, "%s %s", username, password);
            write(sockfd, buff, strlen(buff));
            memset(buff, 0, MAX);
            read(sockfd, buff, 2);
            if (buff[0] == 'O') {
                printf("Welcome, %s\n", username);
                send_mail(sockfd);
                read(sockfd, buff, 80);
                printf("%s\n", buff);
                // getc(stdin);
                close(sockfd);
            }
            else {
                printf("Invalid Credentials\n");
                exit(1);
            }
            break;
        case 3:
            close(sockfd);
            exit(0);
        default:
            break;
        }
        // function for chat

    }

}

