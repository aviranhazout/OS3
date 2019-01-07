#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

void error(char *msg) {
    perror(msg);
    exit(0);
}

#define ECHOMAX 255
/* Longest string to echo */
int main(int argc, char *argv[]) {
    int sock;
/* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen; /* Length of incoming message */
    char echoBuffer[ECHOMAX]; /* Buffer for echo string */
    unsigned short echoServPort; /* Server port */
    int recvMsgSize; /* Size of received message */

    int portno = atoi(argv[1]);
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = INADDR_ANY;
    echoServAddr.sin_port = htons(portno);
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
    if (bind(sock, (struct sockaddr *) &echoServAddr,
             sizeof(echoServAddr)) < 0)
        error("bind() failed");
    for (;;) {
/* Run forever */
/* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
/* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
                                    (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            error("recvfrom() failed");
        printf("Handling client %s\n",
               inet_ntoa(echoClntAddr.sin_addr));
/* Send received datagram back to the client */
        if (sendto(sock, echoBuffer, recvMsgSize, 0,
                   (struct sockaddr *) &echoClntAddr,
                   sizeof(echoClntAddr)) != recvMsgSize)
            error("sendto() sent a different number of bytes than expected");
    }
/* NOT REACHED */
}
