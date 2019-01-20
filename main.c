#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "defs.c"


void error(char *msg)
{
    char* full_str;
    full_str = malloc(strlen(msg) + 20);
    const char* pre = "TTFTP_ERROR: ";
    strcpy(full_str, pre);
    strcat(full_str, msg);
    perror(full_str);
    free(full_str);
    exit(-1);
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
        error(" FAILED TO RUN IOCTL\n");
    }
    else if(res > 0) //if res >0 it means we still have things in socket, need to clean
    {
        if (recv(sock, NULL, res, 0) < 0)
        {
            close(sock);
            error("FAILED TO RUN RECV CMD WHILE CLEANING SOCKET\n");
        }
    }
}

int main(int argc, char *argv[])
{
    int sock;
    FILE* file_to_print;
    //char file_name[MAX_FILE_NAME];
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen; /* Length of incoming message */
    unsigned short echoServPort; /* Server port */
    int recvMsgSize; /* Size of received message */
    int state; //return value of select
    unsigned int written;
    WRQ wrq;
    ACK ack;
    DATA data;
    unsigned short block_num;
    fd_set read_fds;
    tval timeout;
    int timeoutExpiredCount = NUMBER_OF_FAILURES;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    echoServPort = (unsigned short)(atoi(argv[1]));
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

    while (1)
    {
/* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
/* Block until receive message from a client */
        if ((recvMsgSize =
                recvfrom(sock, &wrq, MAX_DATA_SIZE, 0, (struct sockaddr*)&echoClntAddr, &cliAddrLen)) < 0)
        {   //error in receive message
            close(sock);
            error("recvfrom() failed");
        }
        if (recvMsgSize == 0)
        {//no actual message
            flush_socket(sock);
            continue;
        }
        if(ntohs(wrq.opcode) != 2)
        {//wrong opcode
            flush_socket(sock);
            continue;
        }
        else if(strcmp(wrq.mode, "octet") != 0)
        {//wrong mode
            flush_socket(sock);
            continue;
        }
        printf("IN:WRQ, %s, %s", wrq.fname, wrq.mode);
        if ((file_to_print = fopen(wrq.fname, "w")) == NULL)
        {//failed to open file
            close(sock);
            error("failed to open file");
        }

        //everything looks good, sending ack and flush the socket
        block_num = 0;
        ack.opcode = htons(ACK_OPCODE);
        ack.block_num = htons(block_num);
        flush_socket(sock);

        if (sendto(sock, (void*)&ack , sizeof(ack), 0, (struct sockaddr*) &echoClntAddr,
                   sizeof(echoClntAddr)) != sizeof(ack))
        { // failed to send ack packet. terminating server
            close(sock);
            if (fclose(file_to_print) != 0)
                error("sendto() and fclose() failed");
            error("sendto() sent a different number of bytes than expected");
        }
        printf("OUT:ACK, %u", block_num);

        //receive DATA
        do  //while receiving packets
        {
            do {
                FD_ZERO(&read_fds);
                FD_CLR(sock, &read_fds);
                FD_SET(sock, &read_fds);
                // for us at the socket (we are waiting for DATA)
                timeout.tv_sec = WAIT_FOR_PACKET_TIMEOUT;
                timeout.tv_usec = 0;
                if ((state = select(sock + 1, &read_fds, NULL, NULL, &timeout)) < 0) {
                    printf("RECVFAIL\n");
                    close(sock);
                    if (fclose(file_to_print) != 0)
                        error("select() and fclose() failed");
                    error("select() failed");
                }
                if (state > 0) {
                    if ((recvMsgSize = recvfrom(sock, &data, MAX_DATA_SIZE, 0,
                                                (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 4) {
                        printf("RECVFAIL\n");
                        close(sock);
                        if (fclose(file_to_print) != 0)
                            error("recvfrom() and fclose() failed");
                        error("recvfrom() failed");
                    }
                    if (ntohs(data.opcode) != 3 || ntohs(data.block_id) != block_num + 1) {
                        printf("RECVFAIL\n");
                        close(sock);
                        if (fclose(file_to_print) != 0)
                            error("unexpected DATA and fclose() failed");
                        error("unexpected DATA");
                    }
                    block_num++;
                    printf("IN:DATA, %u, %d", block_num, recvMsgSize);
                    if ((written = fwrite(data.data, sizeof(char),
                                          (unsigned int) (recvMsgSize - 4), file_to_print)) != recvMsgSize - 4) {
                        printf("RECVFAIL\n");
                        close(sock);
                        if (fclose(file_to_print) != 0)
                            error("writing to file and fclose() failed");
                        error("writing to file failed");
                    }
                    printf("WRITING: %d\n", written);

                    ack.block_num = htons(block_num);
                    flush_socket(sock);

                    if (sendto(sock, (void *) &ack, sizeof(ack), 0, (struct sockaddr *) &echoClntAddr,
                               sizeof(echoClntAddr)) !=
                        sizeof(ack)) { // failed to send ack packet. terminating server
                        printf("RECVFAIL\n");
                        close(sock);
                        if (fclose(file_to_print) != 0)
                            error("sendto() and fclose() failed");
                        error("sendto() sent a different number of bytes than expected");
                    }
                    printf("OUT:ACK, %u", block_num);


                } else if (state == 0) {   //sending ack again
                    printf("FLOWERROR: didn't receive any packet");
                    if (sendto(sock, (void *) &ack, sizeof(ack), 0, (struct sockaddr *) &echoClntAddr,
                               sizeof(echoClntAddr)) !=
                        sizeof(ack)) { // failed to send ack packet. terminating server
                        printf("RECVFAIL\n");
                        close(sock);
                        if (fclose(file_to_print) != 0)
                            error("sendto() and fclose() failed");
                        error("sendto() sent a different number of bytes than expected");
                    }
                    printf("OUT:ACK, %u", block_num);
                    timeoutExpiredCount++;
                }
                if (timeoutExpiredCount >= NUMBER_OF_FAILURES) {
                    printf("RECVFAIL\n");
                    close(sock);
                    if (fclose(file_to_print) != 0)
                        printf("TTFTP_ERROR: failed to close file and timeoutExpiredCount has reached %d - GAME OVER!!!!",
                               NUMBER_OF_FAILURES);
                    printf("TTFTP_ERROR: timeoutExpiredCount has reached %d - GAME OVER!!!!", NUMBER_OF_FAILURES);
                    exit(-1);
                }

            }
            while (FALSE);
            timeoutExpiredCount = 0;
        }while (recvMsgSize == MAX_DATA_SIZE); // Have blocks left to be read from client (not end of transmission)
        if (fclose(file_to_print) != 0)
        {
            printf("RECVFAIL\n");
            close(sock);
            error("error closing file");
        }
        else {
            printf("RECVOK\n");
            break;
        }
    }
    close(sock);
    return (0);
}