#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 80
#define PORT 8081
#define SA struct sockaddr


// Function designed for chat between client and server.
void func(int connfd, int num)
{
    char buff[MAX];
    int op1, op2;
    char op;
    // infinite loop for chat
    for (;;) {

        memset(buff, 0, MAX);

        // read the message from client and copy it in buffer
        read(connfd, buff, MAX);

        printf("[Client %d]: %s", num, buff);

        // if msg contains "Exit" then server exit and chat ended.
        if (!strncasecmp("exit", buff, 4)) {
            printf("[-]Client %d disconnected...\n", num);
            return;
        }
        sscanf(buff, "%d %c %d", &op1, &op, &op2);
        memset(buff, 0, MAX);
        switch (op) {
        case '+':
            sprintf(buff, "%d", op1 + op2);
            break;
        case '-':
            sprintf(buff, "%d", op1 - op2);
            break;
        case '*':
            sprintf(buff, "%d", op1 * op2);
            break;
        case '/':
            sprintf(buff, "%0.1f", (float)op1 / (float)op2);
            break;
        default:
            sprintf(buff, "[-]Invalid operation");
        }
        printf("[Server]: %s\n", buff);
        write(connfd, buff, strlen(buff));
    }
}

// Driver function
int main()
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    pid_t pid;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("[-]Socket creation failed...\n");
        exit(0);
    }
    else
        printf("[+]Socket created...\n");
    memset(&servaddr, 0, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("[-]Socket bind failed...\n");
        exit(0);
    }
    else
        printf("[+]Socket binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("[-]Listen failed...\n");
        exit(0);
    }
    else
        printf("[+]Listening..\n");
    len = sizeof(cli);

    int num = 0;

    while(1)
    {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("[-]Connection failed...\n");
            exit(0);
        }
        else
            printf("[+]Connected to client %d (:%d)\n", ++num, ntohs(cli.sin_port));

        if ((pid = fork()) == 0) {
            close(sockfd);
            func(connfd, num);
            break;
        }
    }

    // After chatting close the socket
    close(connfd);

    return 0;
}
