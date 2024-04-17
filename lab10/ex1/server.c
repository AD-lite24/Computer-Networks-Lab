#include <arpa/inet.h>
#include <stdio.h>  //printf
#include <stdlib.h> //exit(0);
#include <string.h> //memset
#include <sys/socket.h>

#define BUFLEN 1000 // Max length of buffer
#define PORT 8882  // The port on which to listen for incoming data
#define PACKET_DROP_RATE 10

int drop_packet() { return (rand() % 100 < PACKET_DROP_RATE) ? 1 : 0; }

void die(char *s) {
    perror(s);
    exit(1);
}

typedef struct Packet {
    int payload_size;
    int seq_no;
    int is_last_packet;
    int is_data;
    char data[512];
} PKT;

int main(void) {
    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other), recv_len;
    // char buf[BUFLEN];
    PKT rcv_pkt, ack_pkt;

    printf("Starting server...\n");
    FILE *fp = fopen("output.txt", "w");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    memset((char *)&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1) {
        die("bind");
    }

    int state = 0;

    while (1) {
        switch (state) {
        case 0:
            if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0,
                                     (struct sockaddr *)&si_other, &slen)) ==
                -1) {
                die("recvfrom()");
            }

            if (drop_packet()) {
                printf("DROPPED PACKET\n");
                break; // drop packet
            }

            if (rcv_pkt.seq_no == 0 && rcv_pkt.is_data == 1) {
                printf("RCVD DATA: Seq.No. %d of size %d\n", rcv_pkt.seq_no,
                       rcv_pkt.payload_size);
                fputs(rcv_pkt.data, fp);
                fflush(fp);

                ack_pkt.seq_no = 0;
                ack_pkt.is_data = 0;

                if (sendto(s, &ack_pkt, recv_len, 0,
                           (struct sockaddr *)&si_other, slen) == -1) {
                    die("sendto()");
                }
                state = 1;
            }
            break;

        case 1:
            if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0,
                                     (struct sockaddr *)&si_other, &slen)) ==
                -1) {
                die("recvfrom()");
            }

            if (drop_packet()) {
                printf("DROPPED PACKET\n");
                break; // drop packet
            }

            if (rcv_pkt.seq_no == 1 && rcv_pkt.is_data == 1) {
                printf("RCVD DATA: Seq.No. %d of size %d\n", rcv_pkt.seq_no,
                       rcv_pkt.payload_size);
                fputs(rcv_pkt.data, fp);
                fflush(fp);

                ack_pkt.seq_no = 1;
                ack_pkt.is_data = 0;

                if (sendto(s, &ack_pkt, recv_len, 0,
                           (struct sockaddr *)&si_other, slen) == -1) {
                    die("sendto()");
                }
                state = 0;
            }
            break;
        }
    }
    fclose(fp);
}