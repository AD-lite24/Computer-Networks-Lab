#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFLEN 512 // Max length of buffer
#define PORT 8882
#define RETRANSMISSION_T 2

typedef struct Packet {
    int payload_size;
    int seq_no;
    int is_last_packet;
    int is_data;
    char data[512];
} PKT;

void die(char *s) {
    fprintf(stderr, "Error: %s\n", s);
    exit(1);
}

int main(void) {
    printf("starting client\n");
    struct sockaddr_in si_other;
    int s, i, slen = sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    PKT send_pkt, rcv_ack, prev_pkt;

    FILE *fp = fopen("input.txt", "r");
    char file_buffer[512];

    send_pkt.is_data = 1;
    rcv_ack.is_data = 0;
    prev_pkt.is_data = 1;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");

    fd_set fdset;
    struct timeval t; // to implement timeout
    t.tv_sec = RETRANSMISSION_T;
    t.tv_usec = 0;

    int state = 0;
    while (1) {
        
        int data_is_available;

        switch (state) {
        case 0: // send state seq 0
            if (fgets(file_buffer, sizeof(file_buffer), fp)) {
                size_t line_size = strlen(file_buffer);
                send_pkt.payload_size = line_size;
                send_pkt.seq_no = 0;
                send_pkt.is_last_packet = feof(fp) ? 1 : 0;
                send_pkt.is_data = 1;
                // free(send_pkt.data);
                // send_pkt.data = malloc(line_size);
                strcpy(send_pkt.data, file_buffer);

                if (sendto(s, &send_pkt, sizeof(send_pkt), 0,
                           (struct sockaddr *)&si_other, slen) == -1) {
                    die("sendto()");
                }
                printf("SENT DATA:Seq.N %d of size %d Bytes\n", send_pkt.seq_no,
                       send_pkt.payload_size);

                state = 1;
            } else {
                state = -1; // enter default state
            }

            break;

        case 1: // recieve state seq 0

            FD_ZERO(&fdset);
            FD_SET(s, &fdset);

            data_is_available = select(s + 1, &fdset, NULL, NULL, &t);

            if (!data_is_available) {
                if (sendto(s, &send_pkt, sizeof(send_pkt), 0,
                           (struct sockaddr *)&si_other, slen) == -1) {
                    die("sendto()");
                }
                printf("RESENT DATA: Seq.No %d of size %d Bytes\n",
                       send_pkt.seq_no, send_pkt.payload_size);
                break;
            }

            else if (data_is_available == -1)
                die("select");

            else {
                if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0,
                             (struct sockaddr *)&si_other, &slen) == -1) {
                    die("recvfrom()");
                }
                if (rcv_ack.seq_no == 0 && rcv_ack.is_data == 0) {
                    printf("RCVD ACK: for PKT with Seq.No. %d\n", rcv_ack.seq_no);
                    state = 2;
                }
                break;
            }

        case 2: // send state seq 1
            if (fgets(file_buffer, sizeof(file_buffer), fp)) {
                size_t line_size = strlen(file_buffer);
                send_pkt.payload_size = line_size;
                send_pkt.seq_no = 1;
                send_pkt.is_last_packet = feof(fp) ? 1 : 0;
                send_pkt.is_data = 1;
                // free(send_pkt.data);
                // send_pkt.data = malloc(line_size);
                strcpy(send_pkt.data, file_buffer);

                if (sendto(s, &send_pkt, sizeof(send_pkt), 0,
                           (struct sockaddr *)&si_other, slen) == -1) {
                    die("sendto()");
                }
                printf("SENT DATA:Seq.N %d of size %d Bytes\n", send_pkt.seq_no,
                       send_pkt.payload_size);
                state = 3;
            } else {
                state = -1; // enter default state
            }
            break;

        case 3: // recieve state seq 1
            FD_ZERO(&fdset);
            FD_SET(s, &fdset);

            data_is_available = select(s + 1, &fdset, NULL, NULL, &t);

            if (!data_is_available) {
                if (sendto(s, &send_pkt, sizeof(send_pkt), 0,
                           (struct sockaddr *)&si_other, slen) == -1) {
                    die("sendto()");
                }
                printf("RESENT DATA: Seq.No %d of size %d Bytes\n",
                       send_pkt.seq_no, send_pkt.payload_size);
                break;
            }

            else if (data_is_available == -1)
                die("select");

            else {
                if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0,
                             (struct sockaddr *)&si_other, &slen) == -1) {
                    die("recvfrom()");
                }
                if (rcv_ack.seq_no == 1 && rcv_ack.is_data == 0) {
                    printf("RCVD ACK: for PKT with Seq.No. %d\n", rcv_ack.seq_no);
                    state = 0;
                }
                break;
            }
            break;

        default:
            printf("Done sending all lines\n");
            close(s);
            return 0;
        }
    }

    close(s);
    return 0;
}
