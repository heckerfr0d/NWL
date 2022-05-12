#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 2048
#define PORT 8080
#define MAX_CLIENTS 100
#define SA struct sockaddr

typedef struct Client {
    int sockfd, id;
    char name[20];
} client;

client *clients[MAX_CLIENTS];
static int cnum = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void send_all(char *msg, int id) {
    pthread_mutex_lock(&mutex);
    int i;
    for (i = 0; i < cnum; i++) {
        if (clients[i] && clients[i]->id != id) {
            write(clients[i]->sockfd, msg, strlen(msg));
        }
    }
    pthread_mutex_unlock(&mutex);
}

int find_send(char *name, char* msg) {
    pthread_mutex_lock(&mutex);
    int i;
    for (i = 0; i < cnum; i++) {
        if (clients[i] && strcmp(clients[i]->name, name) == 0) {
            write(clients[i]->sockfd, msg, strlen(msg));
        }
    }
    pthread_mutex_unlock(&mutex);
    return i;
}


// Function designed for chat between client and server.
void *func(void *cli)
{
    char buff[MAX];
    int flag = 0;
    client *c = (client*) cli;

    int n = read(c->sockfd, c->name, sizeof(c->name));
    if(!n)
        flag = 1;
    memset(buff, 0, MAX);
    sprintf(buff, "%s slid into the chat.", c->name);
    printf("%s\n", buff);
    // write(c->sockfd, buff, strlen(buff));
    send_all(buff, c->id);

    // infinite loop for chat
    while(1) {
        memset(buff, 0, MAX);

        // read the message from client and copy it in buffer
        n = read(c->sockfd, buff, MAX);

        // if msg contains "Exit" then server exit and chat ended.
        if (!n) {
            sprintf(buff, "%s went bye-bye", c->name);
            send_all(buff, c->id);
            printf("%s\n", buff);
            break;
        }
        else if (buff[0] == '@') {
            char to[20], msg[MAX], pvt[MAX];
            sscanf(buff, "@%s %[^\n]", to, msg);
            sprintf(pvt, "%s (private): %s", c->name, msg);
            printf("%s -> %s: %s\n", c->name, to, msg);
            int id = find_send(to, pvt);
            if (id != cnum) {
                send_all(buff, c->id);
                memset(buff, 0, MAX);
                sprintf(buff, "Warning: user %s not found.", to);
                write(c->sockfd, buff, strlen(buff));
                printf("%s\n", buff);
            }
        }
        // else send msg to all
        else {
            char msg[MAX];
            sprintf(msg, "%s: %s", c->name, buff);
            send_all(msg, c->id);
            printf("%s\n", msg);
        }
    }
    close(c->sockfd);
    clients[c->id] = NULL;
    free(c);
    // --cnum;
    pthread_detach(pthread_self());
    return 0;
}

// Driver function
int main()
{
    int sockfd, connfd, len, option=1;
    struct sockaddr_in servaddr, cli;

    pthread_t tid;
    // signal(SIGPIPE, SIG_IGN);

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
    servaddr.sin_port = htons(PORT);

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
        printf("[+]Kammattipadam is live!\n");
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
