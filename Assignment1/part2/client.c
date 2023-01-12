/*
    Assignment1 - Part2 - Client File
    Grace Sharma
    20CS30022
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    const int BUF_SIZE = 10, RES_SIZE = 100;
    int sockfd;
    struct sockaddr_in serv_addr;

    int i;
    char buf[BUF_SIZE], res[RES_SIZE];
    for (i = 0; i < BUF_SIZE; i++)
        buf[i] = '\0';
    for (i = 0; i < RES_SIZE; i++)
        res[i] = '\0';

    /* Opening a socket is exactly similar to the server process */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    /* Recall that we specified INADDR_ANY when we specified the server
       address in the server. Since the client can run on a different
       machine, we must specify the IP address of the server.

       In this program, we assume that the server is running on the
       same machine as the client. 127.0.0.1 is a special address
       for "localhost" (this machine)

    /* IF YOUR SERVER RUNS ON SOME OTHER MACHINE, YOU MUST CHANGE
           THE IP ADDRESS SPECIFIED BELOW TO THE IP ADDRESS OF THE
           MACHINE WHERE YOU ARE RUNNING THE SERVER.
        */

    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(20000);

    /* With the information specified in serv_addr, the connect()
       system call establishes a connection with the server process.
    */
    if ((connect(sockfd, (struct sockaddr *)&serv_addr,
                 sizeof(serv_addr))) < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    /* After connection, the client can send or receive messages.
       However, please note that recv() will block when the
       server is not sending and vice versa. Similarly send() will
       block when the server is not receiving and vice versa. For
       non-blocking modes, refer to the online man pages.
    */

    while (1)
    {
        char c;
        int buf_ptr = 0, sent_cnt = 0;
        while ((c = getchar()) != '\n')
        {
            buf[buf_ptr] = c;
            buf_ptr++;
            if (buf_ptr == BUF_SIZE)
            {
                send(sockfd, buf, BUF_SIZE, 0);
                sent_cnt++;
                for (i = 0; i < BUF_SIZE; i++)
                    buf[i] = '\0';
                buf_ptr = 0;
            }
        }
        send(sockfd, buf, BUF_SIZE, 0);
        if (buf[0] == '-' && buf[1] == '1' && strlen(buf) == 2 && sent_cnt == 0)
            break;
        for (i = 0; i < RES_SIZE; i++)
            res[i] = '\0';
        recv(sockfd, res, RES_SIZE, 0);
        printf("%s\n", res);
        for (i = 0; i < BUF_SIZE; i++)
            buf[i] = '\0';
    }

    close(sockfd);
    return 0;
}
