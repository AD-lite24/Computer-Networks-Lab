#include <arpa/inet.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>

#define BUFLEN 512 // Max length of buffer
#define PORT 8882
#define PACKET_DROP_RATE 0.1

typedef struct Packet {

    int payload_size;
    int seq_no;
    int is_last_packet;
    int is_data;
} PKT;



