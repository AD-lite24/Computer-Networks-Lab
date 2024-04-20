#include <arpa/inet.h> // different address structures are declared
#include <stdio.h>
#include <stdlib.h> // atoi() which convert string to integer
#include <string.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv()
#include <unistd.h>     // close() function
#define BUFSIZE 100
#define PACKET_SIZE 100

typedef struct packet {
    int payload_size;
    int seq_no;
    int is_last_packet;
    int is_data;
    int channnel_id;
} PKT;

int main() {
    int sock1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int sock2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock1 < 0 || sock2 < 0) {
        printf("Error in opening a socket");
        exit(0);
    }
    printf("Client Sockets Created\n");

    PKT send_pkt, rcv_ack;

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);

    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int c1 = connect(sock1, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    int c2 = connect(sock2, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (c1 < 0 || c2 < 0) {
        printf("Error while establishing connection");
        exit(0);
    }
    printf("Connections Established\n");

    FILE *fp = fopen("input.txt", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    char buffer[BUFSIZ];
    int state = 0;
    while (1) {

        switch (state) {
        case 0: // transmit seq 0 packet
            int bytes_read;
            if (bytes_read = fread(buffer, sizeof(char), PACKET_SIZE, fp) > 0) {
                int bytes_sent = send(sock1, buffer, strlen(buffer), 0);
                if (bytes_sent != strlen(buffer)) {
                    printf("Error while sending the message");
                    exit(0);
                }
            }
            break;

        case 1:
            break;

        case 2:
            break;

        case 3:
            break;
        }
    }
}