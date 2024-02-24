#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAXPENDING 5
#define BUFFERSIZE 32

int main() {

    /*CREATE A TCP SOCKET*/
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) {
        printf("Error while server socket creation");
        exit(0);
    }
    printf("Server Socket Created\n");

    /*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
    struct sockaddr_in serverAddress, clientAddress, clientAddress2,
        clientAddress3;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address assigned\n");

    int temp = bind(serverSocket, (struct sockaddr *)&serverAddress,
                    sizeof(serverAddress));
    if (temp < 0) {
        printf("Error while binding\n");
        exit(0);
    }
    printf("Binding successful\n");

    int temp1 = listen(serverSocket, MAXPENDING);
    if (temp1 < 0) {
        printf("Error in listen");
        exit(0);
    }
    printf("Now Listening\n");

    char msg[BUFFERSIZE];

    int clientLength = sizeof(clientAddress);

    while (1) {
        int newSocket;
        while (1) {
            newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress,
                               &clientLength);

            if (newSocket < 0) {
                exit(1);
            }

            printf("Connection accepted from %s:%d\n",
                   inet_ntoa(clientAddress.sin_addr),
                   ntohs(clientAddress.sin_port));
            int childpid;
            if ((childpid = fork()) == 0) {
                close(serverSocket); // because the child is not going to accept
                                     // new connections, it will only handle the
                                     // ones it needs
                while (1) {
                    printf("test");
                    int temp = recv(newSocket, msg, 1024, 0);

                    if (temp < 0)
                        printf("Problem in recieving data");

                    if (strcmp(msg, ":exit") == 0) {
                        printf("Disconnected from %s:%d\n",
                               inet_ntoa(clientAddress.sin_addr),
                               ntohs(clientAddress.sin_port));
                        break;
                    }

                    else {
                        printf("Client %s: %s\n",
                               inet_ntoa(clientAddress.sin_addr), msg);
                        send(newSocket, msg, strlen(msg), 0);
                        bzero(msg, sizeof(msg));
                    }
                }
            }
        }
        close(newSocket);
    }
}