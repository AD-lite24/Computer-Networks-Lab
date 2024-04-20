#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define MAXPENDING 2
#define BUFFERSIZE 100

typedef struct packet {
    int pkt_size;
    int seq_no;
    char is_last_packet; // it was required that these fields be one byte
    char type;
    char data[90]; // since total packet size is 100 bytes
} PKT;

int main(void) {

    PKT queue_buffer[2];
    int empty_buffer[2] = {1, 1};
    int expected_byte = 0;
    char buffers[2][100];
    FILE *fp = fopen("output.txt", "w");

    PKT rcv_pkts[2];
    PKT send_acks[2];

    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int clientSockets[2];
    if (serverSocket < 0) {
        printf("Error while socket creation\n");
        return 0;
    }

    printf("Server socket created\n");
    struct sockaddr_in serverAddress, clientAddress, clientAddress2;
    int clientLength = sizeof(clientAddress);
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

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

    if ((clientSockets[0] =
             accept(serverSocket, (struct sockaddr *)&clientAddress,
                    &clientLength)) == -1) {
        perror("accept");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    printf("Client %d connected\n", 1);

    if ((clientSockets[1] =
             accept(serverSocket, (struct sockaddr *)&clientAddress,
                    &clientLength)) == -1) {
        perror("accept");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    printf("Client %d connected\n", 2);

    fd_set fdset;
    int max_sd = 0;
    int activity;
    while (1) {

        for (int j = 0; j < 2; j++) {
            if (empty_buffer[j] == 0 &&
                queue_buffer[j].seq_no == expected_byte) {
                fputs(queue_buffer[j].data, fp);
                expected_byte += rcv_pkts[j].pkt_size - 10;
                fflush(fp);
                empty_buffer[j] = 1;
            }
        }

        FD_ZERO(&fdset);
        FD_SET(serverSocket, &fdset);
        max_sd = serverSocket;

        for (int i = 0; i < 2; ++i) {
            FD_SET(clientSockets[i], &fdset);
            if (clientSockets[i] > max_sd) {
                max_sd = clientSockets[i];
            }
        }

        activity = select(max_sd + 1, &fdset, NULL, NULL, NULL);
        if (activity == -1) {
            perror("select");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        if (activity) {
            for (int i = 0; i < 2; ++i) {
                int sd = clientSockets[i];
                if (FD_ISSET(sd, &fdset)) {
                    // Read data from client
                    memset(&(rcv_pkts[i]), 0, sizeof(rcv_pkts[i]));
                    int bytes_received =
                        recv(sd, &(rcv_pkts[i]), sizeof(rcv_pkts[i]), 0);
                    if (bytes_received == -1) {
                        perror("recv");
                    } else if (bytes_received == 0) {
                        // Client disconnected
                        printf("Client %d disconnected\n", i + 1);
                        close(sd);
                        clientSockets[i] = 0;
                    } else {

                        printf("RCVD PKT: Seq. No. = %d, Size = %d Bytes, "
                               "CH=%d\n",
                               rcv_pkts[i].seq_no, rcv_pkts[i].pkt_size, i);
                        if (rcv_pkts[i].seq_no == expected_byte) {
                            // fputs(rcv_pkts[i].data, fp);
                            fwrite(rcv_pkts[i].data, 1, 90, fp);
                            expected_byte += rcv_pkts[i].pkt_size - 10;
                            fflush(fp);
                        }

                        else {
                            queue_buffer[i] = rcv_pkts[i];
                            empty_buffer[i] = 0;
                        }

                        send_acks[i].seq_no = rcv_pkts[i].seq_no;
                        send_acks[i].type = 0;

                        int temp = send(clientSockets[i], &(send_acks[i]),
                             sizeof(send_acks[i]), 0);

                        printf("SENT ACK: Seq. No. = %d, CH=%d\n",
                               send_acks[i].seq_no, i);
                    }
                }
            }
        }
    }
    close(serverSocket);
    fclose(fp);
}