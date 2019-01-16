#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "defs.c"


void error(char *msg)
{
    perror(msg);
    exit(1);
}
//********************************************
// function name: flush_socket
// Description: cleans socket from old transactions.
// Parameters: socket description
// Returns: N/A
//**************************************************************************************
void flush_socket(int sock)
{
    int res = 0;
    if (ioctl(sock, FIONREAD, &res) < 0) //changed to fionread (I_NREAD does not compile)
    {
        close(sock);
        error("TTFTP_ERROR: FAILED TO RUN IOCTL CMD\n");
    }
    else if(res > 0) //if res >0 it means we still have things in socket, need to clean
    {
        if (recv(sock, NULL, res, 0) < 0)
        {
            close(sock);
            error("TTFTP_ERROR: FAILED TO RUN RECV CMD WHILE CLEANING SOCKET\n");
        }
    }
}

int main(int argc, char *argv[]) {
    int sock;
    /* Socket */
    FILE* file_to_print;
    char file_name[MAX_FILE_NAME];
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen; /* Length of incoming message */
    char echoBuffer[MAX_DATA_SIZE]; /* Buffer for echo string */
    unsigned short echoServPort; /* Server port */
    int recvMsgSize; /* Size of received message */
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    echoServPort = argv[1];
    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        error("socket() failed");
/* Construct local address structure */
/* Zero out structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));
/* Internet address family */
    echoServAddr.sin_family = AF_INET;
/* Any incoming interface */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
/* Local port */
    echoServAddr.sin_port = htons(echoServPort);
/* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
        close(sock);
        error("bind() failed");
    }

    WRQ wrq;
    while () {
/* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
/* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, echoBuffer, MAX_DATA_SIZE, 0,(struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0) {
            close(sock);
            error("recvfrom() failed");
        }
        wrq = (WRQ)echoBuffer;
        if(ntohs(wrq.opcode) != 2) {
            flush_socket(sock);
            continue;
        }
        else if(strcmp(wrq.mode, "octet") != 0)
        {
            flush_socket(sock);
            continue;
        }
        printf("IN:WRQ, %s, %s", wrq.fname, wrq.mode);
        if ((file_to_print = fopen(wrq.fname, "w")) != NULL) {
            close(sock);
            error("open %s", wrq.fname);
        }



        printf("Handling client %s\n",
               inet_ntoa(echoClntAddr.sin_addr));
/* Send received datagram back to the client */
        if (sendto(sock, echoBuffer, recvMsgSize, 0,
                   (struct sockaddr *) &echoClntAddr,
                   sizeof(echoClntAddr)) != recvMsgSize)
            error("sendto() sent a different number of bytes than expected");

        do
        {
            do
            {
                do
                {
                    // TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears
                    //for us at the socket (we are waiting for DATA)
                    wait(WAIT_FOR_PACKET_TIMEOUT);
                    if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
                                                (struct sockaddr *) &echoClntAddr, &cliAddrLen)) >= 4)
                        // TODO: if there was something at the socket and
                        //we are here not because of a timeout
                    {
                        // TODO: Read the DATA packet from the socket (at
                        //least we hope this is a DATA packet)
                    }
                    if (...) // TODO: Time out expired while waiting for data
//to appear at the socket
                    {
//TODO: Send another ACK for the last packet
                        timeoutExpiredCount++;
                    }
                    if (timeoutExpiredCount>= NUMBER_OF_FAILURES)
                    {
// FATAL ERROR BAIL OUT
                    }
                }while (...) // TODO: Continue while some socket was ready
//
                but recvfrom somehow failed to read the data
                if (...) // TODO: We got something else but DATA
                {
// FATAL ERROR BAIL OUT
                }
                if (...) // TODO: The incoming block number is not what we have
//
                expected, i.e. this is a DATA pkt but the block number
//
                in DATA was wrong (not last ACK’s block number + 1)
                {
// FATAL ERROR BAIL OUT
                }
            }while (FALSE);
            timeoutExpiredCount = 0;
            lastWriteSize = fwrite(...); // write next bulk of data
// TODO: send ACK packet to the client
        }while (...); // Have blocks left to be read from client (not end of transmission)
    }
    close(sock);
}