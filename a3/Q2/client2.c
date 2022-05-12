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

#define MAX 500
#define PORT 8080
#define SA struct sockaddr
#define MIN(a,b) ((a)<(b)?(a):(b))

int data = 0, etime = 0;
float rate[MAX];


void thread_handler() {
    etime += 1;
    rate[etime] = (float)data/(etime*0.01);
}

void func(int sockfd)
{
	char buff[MAX];
	int n;

    struct itimerval it_val;

	for (;;) {
		bzero(buff, MAX);
		printf("$ ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n');
		write(sockfd, buff, sizeof(buff));
		if ((strncmp(buff, "bye", 3)) == 0) {
            write(sockfd, "bye", 3);
			printf("bye bye\n");
			break;
		}
        else if (!(strncmp(buff, "givemeyourvideo", 15))) {
            long filesize;
            read(sockfd, &filesize, sizeof(filesize));
            // filesize = ntohl(filesize);
            FILE *fp = fopen("downloaded.mp4", "wb");
            it_val.it_value.tv_sec = 0;
            it_val.it_value.tv_usec = 10000;
            it_val.it_interval = it_val.it_value;
            setitimer(ITIMER_REAL, &it_val, NULL);
            do {
                bzero(buff, MAX);
                int num = MIN(filesize, MAX);
                char* p = buff;
                int temp = num;
                while (num > 0) {
                    int n = read(sockfd, p, num);
                    data += n;
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
            it_val.it_value.tv_usec = 0;
            setitimer(ITIMER_REAL, &it_val, NULL);
            bzero(buff, MAX);
            fclose(fp);
            printf("Video Downloaded\n");
            bzero(buff, MAX);
            char * commandsForGnuplot[] = {"set title \"Client Data Transmission Rate\"", "set xlabel \"seconds\"", "set ylabel \"gbps\"", "plot '-' smooth csplines\n"};
            FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
            for(int i=0; i<4; i++) {
                fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]);
            }
            for(int i=0; i<etime; i++) {
                // printf(gnuplotPipe, "%.2f %.2f\n", i*0.01, rate[i]/131072);
                fprintf(gnuplotPipe, "%.2f %.2f\n", i*0.01, rate[i]/134217728);
            }
            fprintf(gnuplotPipe, "e");
            fflush(gnuplotPipe);
            pclose(gnuplotPipe);
            // system("vlc downloaded.mp4&");
            continue;
        }
		bzero(buff, MAX);
		read(sockfd, buff, MAX);
		printf(buff);
	}
}

int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

    signal(SIGALRM, (void (*)(int)) thread_handler);

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
	func(sockfd);

	// close the socket
	close(sockfd);
}

