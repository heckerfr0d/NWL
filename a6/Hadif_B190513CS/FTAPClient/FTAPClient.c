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

#define MAX 2048
#define PORT 4035
#define SA struct sockaddr
#define MIN(a,b) ((a)<(b)?(a):(b))

typedef struct Args {
    int sockfd;
    char filename[50];
} args;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *getFile(void *a)
{
    pthread_mutex_lock(&mutex);
    char buff[MAX];
    int sockfd = ((args*)a)->sockfd;
    char *arg = ((args*)a)->filename;
    long filesize, fs;
    clock_t t = clock();
    read(sockfd, &filesize, sizeof(filesize));
    fs = filesize;
    FILE *fp = fopen(arg, "wb");
    do {
        bzero(buff, MAX);
        int num = MIN(filesize, MAX);
        char* p = buff;
        int temp = num;
        while (num > 0) {
            int n = read(sockfd, p, num);
            p += n;
            num -= n;
        }
        num = temp;
        int off = 0;
        do {
            int written = fwrite(&buff[off], 1, num-off, fp);
            off += written;
        } while (off < num);
        filesize -= num;
    } while (filesize > 0);
    t = clock() - t;
    bzero(buff, MAX);
    fclose(fp);
    printf("'%s' (%d bytes) transferred in %ldμs\n", arg, fs, t);
    pthread_mutex_unlock(&mutex);
    pthread_detach(pthread_self());
}

void *storeFile(void *a)
{
    pthread_mutex_lock(&mutex);
    char buff[MAX];
    int sockfd = ((args*)a)->sockfd;
    char *arg = ((args*)a)->filename;
    clock_t t = clock();
    FILE *fp = fopen(arg, "rb");
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);
    int fs = filesize;
    write(sockfd, &filesize, sizeof(filesize));
    int n;
    do {
        bzero(buff, MAX);
        n = MIN(filesize, MAX);
        n = fread(buff, 1, n, fp);
        filesize -= n;
        char* p = buff;
        while(n>0) {
            int num = write(sockfd, p, n);
            p += num;
            n -= num;
        }
    } while(filesize > 0);
    t = clock() - t;
    printf("'%s' (%d bytes) transferred in %ldμs\n", arg, fs, t);
    fclose(fp);
    pthread_mutex_unlock(&mutex);
    pthread_detach(pthread_self());
}

void func(int sockfd)
{
	char buff[MAX];
	int n;
    pthread_t tid;

	for (;;) {
		bzero(buff, MAX);
		// printf("$ ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n');
        if (buff[0] == '\n')
            continue;
		write(sockfd, buff, n);
        char cmd[20], arg[20];
        sscanf(buff, "%s %s", cmd, arg);
		if ((strncasecmp(cmd, "QUIT", 4)) == 0) {
            pthread_mutex_lock(&mutex);
            pthread_mutex_unlock(&mutex);
			break;
		}
        else if (!strncasecmp(cmd, "GETFILE", 7)) {
            args a;
            a.sockfd = sockfd;
            strcpy(a.filename, arg);
            pthread_create(&tid, NULL, (void*)&getFile, (void*)&a);
            continue;
        }
        else if (!strncasecmp(cmd, "STOREFILE", 9)) {
            args a;
            a.sockfd = sockfd;
            strcpy(a.filename, arg);
            pthread_create(&tid, NULL, (void*)&storeFile, (void*)&a);
            continue;
        }
		bzero(buff, MAX);
		read(sockfd, buff, MAX);
		printf("%s\n", buff);
	}
}

int main()
{
	int sockfd, connfd;
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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

    char buff[100];
    while(1) {
        fgets(buff, 6, stdin);
        if (!strncasecmp(buff, "START", 5)) {
            fflush(stdin);
            // connect the client socket to server socket
            if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
                printf("connection with the server failed...\n");
                exit(0);
            }
            memset(buff, 0, 100);
            read(sockfd, buff, 100);
            printf("%s\n", buff);
            // function for chat
            func(sockfd);
            break;
        }
        else {
            printf("505 Command not supported\n");
        }
    }


	// close the socket
	close(sockfd);
}

