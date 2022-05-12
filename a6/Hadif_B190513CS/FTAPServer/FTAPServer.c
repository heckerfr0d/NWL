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
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 2048
#define PORT 4035
#define MAX_CLIENTS 100
#define SA struct sockaddr
#define MIN(a,b) ((a)<(b)?(a):(b))

typedef struct Client {
    int sockfd, id;
} client;

typedef struct User {
    char name[50], pass[50];
} user;
user *users[MAX_CLIENTS];

typedef struct Args {
    int sockfd;
    char filename[50];
} args;

client *clients[MAX_CLIENTS];
static int cnum = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int authenticate(client *c, char *username)
{
    for (int i=0; users[i]; i++) {
        if (!strncmp(username, users[i]->name, strlen(users[i]->name))) {
            write(c->sockfd, "300 Correct Username; Need password", 36);
            while(1) {
                char buff[50];
                int n = read(c->sockfd, buff, 50);
                if (!n) {
                    return 0;
                }
                char cmd[10], arg[50];
                sscanf(buff, "%s%s", cmd, arg);
                if (!strncasecmp(cmd, "PASSWD", 6)) {
                    if (!strncmp(arg, users[i]->pass, strlen(users[i]->pass))) {
                        sprintf(buff, "305 User Authenticated with password\nWelcome, %s!", username);
                        write(c->sockfd, buff, strlen(buff));
                        return 1;
                    }
                    else {
                        write(c->sockfd, "310 Incorrect password", 23);
                    }
                }
                else if (!strncasecmp(cmd, "QUIT", 5)) {
                    return -1;
                }
                else {
                    write(c->sockfd, "505 Command not supported", 26);
                }
            }
        }
    }
    write(c->sockfd, "301 Incorrect Username", 23);
    return 0;
}


void *storeFile(void *a)
{
    pthread_mutex_lock(&mutex);
    char buff[MAX];
    int sockfd = ((args*)a)->sockfd;
    char *arg = ((args*)a)->filename;
    FILE *fp = fopen(arg, "rb");
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);
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
    fclose(fp);
    pthread_mutex_unlock(&mutex);
    pthread_detach(pthread_self());
}


// Function designed for chat between client and server.
void *func(void *cli)
{
    char buff[MAX];
    client *c = (client*) cli;

    pthread_t tid;

    int auth = 0;

    // infinite loop for chat
    while(1) {
        memset(buff, 0, MAX);

        // read the message from client and copy it in buffer
        int n = read(c->sockfd, buff, MAX);

        if (!n) {
            break;
        }

        char cmd[10], arg[50];
        sscanf(buff, "%s%s", cmd, arg);

        if (!strncasecmp(cmd, "USERN", 5)) {
            auth = authenticate(cli, arg);
            if (auth == -1) {
                break;
            }
        }

        else if (auth) {
            if (!strncasecmp(cmd, "CREATEFILE", 10)) {
                FILE *cf = fopen(arg, "w");
                fflush(cf);
                fclose(cf);
            }
            else if (!strncasecmp(cmd, "LISTDIR", 7)) {
                struct dirent *d;
                DIR *dr = opendir(".");
                memset(buff, 0, MAX);
                while ((d = readdir(dr))) {
                    if (d->d_name[0] != '.' && strncmp(d->d_name, "FTAPServer", 10)) {
                        strcat(buff, d->d_name);
                        strcat(buff, "\n");
                    }
                }
                write(c->sockfd, buff, strlen(buff));
            }
            else if (!strncasecmp(cmd, "STOREFILE", 9)) {
                pthread_mutex_lock(&mutex);
                char buff[MAX];
                long filesize;
                read(c->sockfd, &filesize, sizeof(filesize));
                FILE *fp = fopen(arg, "wb");
                do {
                    bzero(buff, MAX);
                    int num = MIN(filesize, MAX);
                    char* p = buff;
                    int temp = num;
                    while (num > 0) {
                        int n = read(c->sockfd, p, num);
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
                fclose(fp);
                pthread_mutex_unlock(&mutex);
            }
            else if (!strncasecmp(cmd, "GETFILE", 7)) {
                args a;
                a.sockfd = c->sockfd;
                strcpy(a.filename, arg);
                pthread_create(&tid, NULL, (void*)&storeFile, (void*)&a);
            }
            else if (!strncasecmp(cmd, "QUIT", 5)) {
                break;
            }
            else {
                write(c->sockfd, "505 Command not supported", 26);
            }
        }
        else if (!strncasecmp(cmd, "QUIT", 5)) {
            break;
        }
        else {
            write(c->sockfd, "505 Command not supported", 26);
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
    signal(SIGPIPE, SIG_IGN);

    FILE *lc = fopen("../logincred.txt", "r");

    int i = 0;
    while (!feof(lc)) {
        users[i] = (user*)malloc(sizeof(user));
        fscanf(lc, "%s", users[i]->name);
        int k = 0;
        for(k = 0; users[i]->name[k]!=','; k++);
        users[i]->name[k++] = 0;
        for(int j = 0; users[i]->name[k]; j++) {
            users[i]->pass[j] = users[i]->name[k];
            users[i]->name[k++] = 0;
        }
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
        printf("[+]FTAPServer is live!\n");
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

            write(connfd, "200 OK Connection is set up", 28);
            cnum++;
        }
    }

    // After chatting close the socket
    close(connfd);

    return 0;
}
