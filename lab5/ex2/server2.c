#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAXPENDING 5
#define BUFFERSIZE 32

int main() {

    FILE* fp = fopen("database.txt", "w");
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) {
        printf("Error while server socket creation");
        exit(0);
    }
    printf("Server Socket Created\n");


    /*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
    struct sockaddr_in serverAddress, clientAddress, clientAddress2, clientAddress3;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12341);
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



    while(1) {
        int newSocket;
        while(1) {
            newSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);

            if (newSocket < 0) {
                exit(1);
            }

            printf("Connection accepted from %s:%d\n",inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
            int childpid;
            int childtest = fork();
            printf("test: %d", childtest);
            if (childtest == 0) {
                
                close(serverSocket);
                while(1) {
                    printf("test");
                    int temp = recv(newSocket, msg, 1024, 0);
                    printf("Given msg: %s", msg);
                    if (temp < 0) printf("Problem in recieving data");
                    
                    if(strcmp(msg, ":exit") == 0){
                        printf("Disconnected from %s:%d\n",inet_ntoa(clientAddress.sin_addr),
                        ntohs(clientAddress.sin_port));
                        break;
                    }

                    else {
                        if (strncmp(msg, "put", 3) == 0) {
                            printf("Putting key value");
                        }

                        else if (strncmp(msg, "get", 3) == 0) {
                            printf("getting key value");
                        }

                        else if (strncmp(msg, "del", 3) == 0) {
                            printf("deleting key value");
                        }

                        else {
                            printf("Incorrect command");
                        }

                    }
                }
            }
        }
        close(newSocket);
    }

}