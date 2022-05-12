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
#define PORT 8080
#define SA struct sockaddr

typedef struct fruitType{
    char name[20];
    int count;
} ftype;

// Function designed for chat between client and server.
void func(int connfd)
{
    char buff[MAX];
    ftype fruits[5], fruit;
    strcpy(fruits[0].name, "apple");
    fruits[0].count = 10;
    strcpy(fruits[1].name, "mango");
    fruits[1].count = 9;
    strcpy(fruits[2].name, "banana");
    fruits[2].count = 8;
    strcpy(fruits[3].name, "chikoo");
    fruits[3].count = 7;
    strcpy(fruits[4].name, "papaya");
    fruits[4].count = 6;

    // infinite loop for chat
    for (;;) {

        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        read(connfd, buff, MAX);

        printf(buff);
        if (!strncasecmp(buff, "fruits", 6)) {
            write(connfd, "Enter the name of the fruit: ", 30);
            bzero(buff, MAX);
            read(connfd, buff, sizeof(buff));
            printf(buff);
            sscanf(buff, "%s%d", fruit.name, &fruit.count);
            int i;
            for(i=0; i<5; i++) {
                if (tolower(fruit.name[0]) == tolower(fruits[i].name[0])) {
                    if (fruit.count > fruits[i].count) {
                        write(connfd, "Not available\n", 15);
                    }
                    else {
                        fruits[i].count -= fruit.count;
                        sprintf(buff, "%ss remaining: %d\n", fruits[i].name, fruits[i].count);
                        write(connfd, buff, sizeof(buff));
                        bzero(buff, MAX);
                    }
                    break;
                }
            }
            if (i == 5) {
                write(connfd, "Not available\n", 15);
            }
        }
        else if (!strncasecmp(buff, "SendInventory", 13)) {
            bzero(buff, MAX);
            strcat(buff, "Inventory\n");
            for(int i=0; i<5; i++) {
                char fruit[22];
                sprintf(fruit, "%s: %d\n", fruits[i].name, fruits[i].count);
                strcat(buff, fruit);
            }
            write(connfd, buff, MAX);
            bzero(buff, MAX);
        }

        // if msg contains "Exit" then server exit and chat ended.
        else if (!strncasecmp("exit", buff, 4)) {
            printf("Server Exit...\n");
            break;
        }
        else {
            write(connfd, "Invalid Command\n", 15);
        }
    }
}

// Driver function
int main()
{
    int sockfd, connfd, len;
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
