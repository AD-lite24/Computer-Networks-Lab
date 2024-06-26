#include <arpa/inet.h> // different address structures are declared
#include <stdio.h>
#include <stdlib.h> // atoi() which convert string to integer
#include <string.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions here
#include <unistd.h>     // close() function
#define BUFSIZE 32

int main() {

    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock < 0) {
        printf("Error in opening a socket");
        exit(0);
    }

    printf("Client Socket Created\n");

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));


    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); 
    serverAddr.sin_addr.s_addr =
        inet_addr("127.0.0.1"); // Specify server's IP address here (Use
                                // ifconfig command for macos)
    printf("Address assigned\n");


    int c = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    printf("%d\n", c);
    if (c < 0) {
        printf("Error while establishing connection");
        exit(0);
    }

    printf("Connection Established\n");


    while(1) {
        printf("ENTER MESSAGE FOR SERVER with max 32 characters\n");
        char msg[BUFSIZE];
        gets(msg);
        int bytesSent = send(sock, msg, strlen(msg), 0);
        if (bytesSent != strlen(msg)) {
            printf("Error while sending the message");
            exit(0);
        }
        if (strcmp(msg, ":exit") == 0) break;
        printf("Data Sent\n");
    }
}
