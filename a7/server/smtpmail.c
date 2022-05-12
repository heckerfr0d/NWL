#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include <dirent.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 4096
#define PORT 4035
#define MAX_CLIENTS 100
#define SA struct sockaddr
#define MIN(a,b) ((a)<(b)?(a):(b))

typedef struct Client {
    int sockfd, id;
} client;

typedef struct User {
    char name[40], pass[40];
} user;
user *users[MAX_CLIENTS];

client *clients[MAX_CLIENTS];
static int cnum = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int authenticate(client *c, char *username, char *password)
{
    for (int i=0; users[i]; i++) {
        if (!strncmp(username, users[i]->name, strlen(users[i]->name)) && !strncmp(password, users[i]->pass, strlen(users[i]->pass))) {
            write(c->sockfd, "OK", 2);
            return 1;
        }
    }
    write(c->sockfd, "NO", 2);
    return 0;
}




// Function designed for chat between client and server.
void *func(void *cli)
{
    char buff[MAX], username[40], password[40];
    client *c = (client*) cli;

    memset(buff, 0, MAX);

    read(c->sockfd, buff, MAX);
    sscanf(buff, "%s%s", username, password);
    int auth = authenticate(c, username, password);

    // infinite loop for chat
    while(auth) {
        memset(buff, 0, MAX);

        // read the message from client and copy it in buffer
        int n = read(c->sockfd, buff, MAX);

        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        if (!n) {
            break;
        }

        char filename[80], buff2[MAX], *line, *rest = buff;
        memset(buff2, 0, MAX);

        int i = 0;
        int flag = 0;
        while (line = strtok_r(rest, "\n", &rest)) {
            if (i == 1) {
                sscanf(line, "To: %[^@]", filename);
                for (int i=0; users[i]; i++) {
                    if (!strncmp(filename, users[i]->name, strlen(users[i]->name))) {
                        flag = 1;
                        strcat(filename, "/mymailbox");
                    }
                }
                if (!flag) {
                    break;
                }
            }
            if (i == 3) {
                char timebuf[70];
                strftime(timebuf, 80, "Received: %d %b %Y : %I:%M%p\n", timeinfo);
                strcat(buff2, timebuf);
            }
            strcat(buff2, line);
            strcat(buff2, "\n");
            if (line[0] == '.') {
                break;
            }
            i++;
        }

        if (!flag) {
            write(c->sockfd, "Invalid mailbox. Mail not sent.", 32);
            break;
        }

        FILE *mb = fopen(filename, "a");
        fputs(buff2, mb);
        fflush(mb);
        fclose(mb);
        write(c->sockfd, "Mail sent successfully", 23);

    }
    close(c->sockfd);
    clients[c->id] = NULL;
    free(c);
    // --cnum;
    pthread_detach(pthread_self());
    return 0;
}

// Driver function
int main(int argc, char* argv[])
{
    int sockfd, connfd, len, option=1, my_port;
    struct sockaddr_in servaddr, cli;

    sscanf(argv[1], "%d", &my_port);

    pthread_t tid;
    signal(SIGPIPE, SIG_IGN);

    FILE *lc = fopen("userlogincred.txt", "r");

    int i = 0;
    while (!feof(lc)) {
        users[i] = (user*)malloc(sizeof(user));
        fscanf(lc, "%s%s", users[i]->name, users[i]->pass);
        i++;
    }

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("[-]Socket creation failed...\n");
        exit(1);
    }
    else
        printf("[+]Socket created...\n");
    memset(&servaddr, 0, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(my_port);

    if(setsockopt(sockfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		printf("[-]setsockopt failed");
        exit(1);
	}

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("[-]Socket bind failed...\n");
        exit(1);
    }
    else
        printf("[+]Socket binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 10)) != 0) {
        printf("[-]Listen failed...\n");
        exit(1);
    }
    else
        printf("[+]SMTPServer is live!\n");
    len = sizeof(cli);


    while(1)
    {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (cnum == MAX_CLIENTS-1 || connfd < 0) {
            printf("[-]Error connecting %d.%d.%d.%d:%d\n", cli.sin_addr.s_addr & 0xff,
                (cli.sin_addr.s_addr & 0xff00) >> 8,
                (cli.sin_addr.s_addr & 0xff0000) >> 16,
                (cli.sin_addr.s_addr & 0xff000000) >> 24, ntohs(cli.sin_port));
            close(connfd);
            continue;
        }
        else {
            printf("[+]New client: %d.%d.%d.%d:%d\n", cli.sin_addr.s_addr & 0xff,
                (cli.sin_addr.s_addr & 0xff00) >> 8,
                (cli.sin_addr.s_addr & 0xff0000) >> 16,
                (cli.sin_addr.s_addr & 0xff000000) >> 24, ntohs(cli.sin_port));

            clients[cnum] = (client*)malloc(sizeof(client));
            clients[cnum]->sockfd = connfd;
            clients[cnum]->id = cnum;

            pthread_create(&tid, NULL, (void*)&func, (void*)clients[cnum]);

            cnum++;
        }
    }

    // After chatting close the socket
    close(connfd);

    return 0;
}
