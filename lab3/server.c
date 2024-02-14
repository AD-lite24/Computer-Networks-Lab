#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAXPENDING 5
#define BUFFERSIZE 32

// Accepts 3 simultaneous connections 

int main() {

    /*CREATE A TCP SOCKET*/
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) {
        printf("Error while server socket creation");
        exit(0);
    }
    printf("Server Socket Created\n");

    /*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
    struct sockaddr_in serverAddress, clientAddress1, clientAddress2, clientAddress3;
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

    char msg1[BUFFERSIZE];
    char msg2[BUFFERSIZE];
    char msg3[BUFFERSIZE];

    int clientLength1 = sizeof(clientAddress1);
    int clientLength2 = sizeof(clientAddress2);
    int clientLength3 = sizeof(clientAddress3);

    int clientSocket1 =
        accept(serverSocket, (struct sockaddr *)&clientAddress1, &clientLength1);

    int clientSocket2 =
        accept(serverSocket, (struct sockaddr *)&clientAddress2, &clientLength2);

    int clientSocket3 =
        accept(serverSocket, (struct sockaddr *)&clientAddress3, &clientLength3);

    if (clientLength1 < 0 || clientLength2 < 0 || clientLength3 < 0) {
        printf("Error in client socket");
        exit(0);
    }


    printf("Handling Client %s\n", inet_ntoa(clientAddress1.sin_addr));
    int temp2 = recv(clientSocket1, msg1, BUFFERSIZE, 0);
    int temp3 = recv(clientSocket2, msg2, BUFFERSIZE, 0);
    int temp4 = recv(clientSocket3, msg3, BUFFERSIZE, 0);

    if (temp2 < 0) {
        printf("problem in temp 2");
        exit(0);
    }

    if (temp3 < 0) {
        printf("problem in temp 3");
        exit(0);
    }

    if (temp4 < 0) {
        printf("problem in temp 4");
    }

    printf("%s\n", msg1);
    printf("%s\n", msg2);
    printf("%s\n", msg3);

    printf("ENTER MESSAGE FOR CLIENT1\n");
    gets(msg1);
    printf("SENDING MESSAGE %s \n", msg1);

    printf("ENTER MESSAGE FOR CLIENT2\n");
    gets(msg2);
    printf("SENDING MESSAGE %s \n", msg2);

    printf("ENTER MESSAGE FOR CLIENT3\n");
    gets(msg3);
    printf("SENDING MESSAGE %s \n", msg3);

    int bytesSent1 = send(clientSocket1, msg1, strlen(msg1), 0);
    int bytesSent2 = send(clientSocket2, msg2, strlen(msg2), 0);
    int bytesSent3 = send(clientSocket3, msg3, strlen(msg3), 0);

    if (bytesSent1 != strlen(msg1)) {
        printf("Error while sending message to client1");
        exit(0);
    }

    if (bytesSent2 != strlen(msg2)) {
        printf("Error while sending message to client2");
        exit(0);
    }

    if (bytesSent3 != strlen(msg3)) {
        printf("Error while sending message to client3");
        exit(0);
    }

    close(serverSocket);
    close(clientSocket1);
    close(clientSocket2);
    close(clientSocket3);
}