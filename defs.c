/*

File: def.h

Abstract:
includes and defines for ttftp server.

Main functions:
N/A
*/
#ifndef def
#define def

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define WAIT_FOR_PACKET_TIMEOUT 3
#define NUMBER_OF_FAILURES 7
#define MAX_DATA_SIZE 516
#define MAX_FILE_NAME 255
#define FALSE 0

typedef struct sockaddr_in sock_addr;
typedef struct timeval tval;
typedef struct _WRQ
{
    unsigned short opcode;
    char fname[MAX_FILE_NAME];
    char mode[MAX_FILE_NAME];

} __attribute__((packed)) WRQ;

typedef struct _ACK
{
    unsigned short opcode;
    unsigned short block_num;
} __attribute__((packed)) ACK;

typedef struct _DATA
{
    unsigned short opcode;
    unsigned short block_id;
    char data[MAX_DATA_SIZE];
} __attribute__((packed)) DATA;

#endif //def