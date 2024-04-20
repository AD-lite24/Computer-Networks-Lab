#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 90 // Max length of buffer
// #define PORT 8882 // The port on which to send data
#define PACKET_SIZE ;

typedef struct packet {
    int pkt_size;
    int seq_no;
    char is_last_packet; // it was required that these fields be one byte
    char type;
    char data[90]; // since total packet size is 100 bytes
} PKT;

void die(char *s) {
    perror(s);
    exit(1);
}

int main(void) {
    struct sockaddr_in si_other;
    int s1, i1, slen1 = sizeof(si_other);
    int s2, i2, slen2 = sizeof(si_other);

    int sent_byte = 0;

    fd_set fdset;

    int unacked = 2; // keep track of unacked packets
    int acked_channel[2] = {1, 1};
    int send_seq_no[2] = {0, 0};

    char buf[BUFLEN];

    PKT send_pkt0, rcv_ack0, send_pkt1, rcv_ack1;

    if ((s1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        die("socket 1");
    }

    if ((s2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        die("socket 2");
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);

    serverAddr.sin_addr.s_addr =
        inet_addr("127.0.0.1"); // Specify server's address here
    printf("Address assigned\n");

    int c1 = connect(s1, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    int c2 = connect(s2, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    printf("%d\n", c1);
    printf("%d\n", c2);

    if (c1 < 0 || c2 < 0) {
        printf("Error while establishing connection");
        exit(0);
    }
    printf("Connection Established\n");

    FILE *fp = fopen("input.txt", "r");

    int bytes_read;

    while (1) {

        if (acked_channel[0]) { // can send packet across channel 0
            memset(send_pkt0.data, 0, BUFLEN);
            if ((bytes_read = fread(send_pkt0.data, 1, BUFLEN, fp)) > 0) {
                send_pkt0.seq_no = sent_byte;
                send_pkt0.pkt_size = BUFLEN + 10;

                int bytes_sent = send(s1, &send_pkt0, bytes_read + 10, 0);
                if (bytes_sent != bytes_read + 10) {
                    printf("Error while sending the message");
                    exit(0);
                }
                printf("SENT PKT: Seq. No. = %d, Size = %d Bytes, CH=%d\n",
                       send_pkt0.seq_no, bytes_sent, 0);
                acked_channel[0] = 0;
                sent_byte += bytes_sent - 10;

                if (feof(fp)) {
                    printf("All packets sent");
                    return 1;
                }
            }

            else {
                printf("Error reading file");
            }
        }

        if (acked_channel[1]) { // can send packets accross channel 1
            memset(send_pkt1.data, 0, BUFLEN);
            if ((bytes_read = fread(send_pkt1.data, 1, BUFLEN, fp)) > 0) {
                send_pkt1.seq_no = sent_byte;
                send_pkt1.pkt_size = BUFLEN + 10;

                int bytes_sent = send(s2, &send_pkt1, bytes_read + 10, 0);
                if (bytes_sent != bytes_read + 10) {
                    printf("Error while sending the message");
                    exit(0);
                }
                printf("SENT PKT: Seq. No. = %d, Size = %d Bytes, CH=%d\n",
                       send_pkt1.seq_no, bytes_sent, 1);
                acked_channel[1] = 0;
                sent_byte += bytes_sent - 10;

                if (feof(fp)) {
                    printf("All packets sent");
                    return 1;
                }
            }

            else {
                printf("Error reading file");
            }
        }

        else {
            FD_ZERO(&fdset);
            int max_sd = s1 > s2 ? s1 : s2;
            FD_SET(s1, &fdset);
            FD_SET(s2, &fdset);

            int activity = select(max_sd + 1, &fdset, NULL, NULL, NULL);

            if (activity == -1) {
                perror("select");
                exit(EXIT_FAILURE);
            }

            if (FD_ISSET(s1, &fdset)) {
                if (recvfrom(s1, &rcv_ack0, sizeof(rcv_ack0), 0,
                             (struct sockaddr *)&si_other, &slen1) == -1)
                    die("recvfrom()");

                if (rcv_ack0.type == 0) {
                    printf("RCVD ACK: for PKT with Seq.No. %d, CH = %d\n",
                           rcv_ack0.seq_no, 0);
                    acked_channel[0] = 1;
                }
            }

            if (FD_ISSET(s2, &fdset)) {
                if (recvfrom(s2, &rcv_ack1, sizeof(rcv_ack0), 0,
                             (struct sockaddr *)&si_other, &slen2) == -1)
                    die("recvfrom()");

                if (rcv_ack1.type == 1) {
                    printf("RCVD ACK: for PKT with Seq.No. %d, CH = %d\n",
                           rcv_ack0.seq_no, 1);
                    acked_channel[1] = 1;
                }
            }
        }
    }

    close(s1);
    close(s2);
}
