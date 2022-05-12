#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
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

// Function designed for chat between client and server.
void func(int connfd)
{
    char buff[MAX];
    char *random[] = {"ok bro\n", "shereenna\n", "okda\n", "hmmmm\n", "k\n", "riight\n", "aaha, ennitt\n", "ok bei\n", "mmhmmm\n", "lol\n", "xD\n", "noice\n", "kekw\n"};

    struct itimerval it_val;

    // infinite loop for chat
    for (;;) {

        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        read(connfd, buff, MAX);

        printf(buff);

        if (!strncasecmp(buff, "givemeyourvideo", 15)) {
            bzero(buff, MAX);
            FILE *fp = fopen("data.mp4", "rb");
            fseek(fp, 0, SEEK_END);
            long filesize = ftell(fp);
            rewind(fp);
            write(connfd, &filesize, sizeof(filesize));
            // filesize = ntohl(filesize);
            int n;
            it_val.it_value.tv_sec = 0;
            it_val.it_value.tv_usec = 10000;
            it_val.it_interval = it_val.it_value;
            setitimer(ITIMER_REAL, &it_val, NULL);
            do {
                bzero(buff, MAX);
                n = MIN(filesize, MAX);
                n = fread(buff, 1, n, fp);
                filesize -= n;
                char* p = buff;
                while(n>0) {
                    int num = write(connfd, p, n);
                    data += num;
                    p += num;
                    n -= num;
                }
            } while(filesize > 0);
            fclose(fp);
            bzero(buff, MAX);
            char * commandsForGnuplot[] = {"set title \"Server Data Transmission Rate\"", "set xlabel \"seconds\"", "set ylabel \"gbps\"", "plot '-' smooth csplines\n"};
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
            printf("Video sent\n");
        }

        // if msg contains "Exit" then server exit and chat ended.
        else if (!strncasecmp("bye", buff, 3)) {
            printf("bye gn :p\n");
            break;
        }
        else {
            // get a random message from the array
            int i = rand() % 13;
            write(connfd, random[i], strlen(random[i]));
        }
    }
}

// Driver function
int main()
{
    int sockfd, connfd, len;
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
