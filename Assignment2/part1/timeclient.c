// A Simple Client Implementation
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netinet/in.h>

int main()
{
    const int timeout = 3000, MAXLINE = 50;
    int sockfd;
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8181);
    inet_aton("127.0.0.1", &servaddr.sin_addr);

    struct pollfd fds;
    fds.fd = sockfd;
    fds.events = POLLIN;

    int success = 0;
    int n;
    socklen_t len;
    char *hello = "CLIENT:HELLO";
    char buffer[MAXLINE];

    for (int i = 0; i < 5; i++)
    {
        // Sending Message to Server
        sendto(sockfd, (const char *)hello, strlen(hello), 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));

        int ret = poll(&fds, 1, timeout);
        if (ret == -1)
        {
            perror("Error in poll()");
            exit(1);
        }
        else if (ret == 0)
        {
            printf("Timeout: Waiting for data\n");
        }
        else
        {
            // Receive data from server
            len = sizeof(servaddr);
            n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0,
                         (struct sockaddr *)&servaddr, &len);
            buffer[n] = '\0';

            if (n < 0)
            {
                perror("Error receiving data");
                exit(1);
            }
            else
            {
                printf("Received : %s\n", buffer);
                success = 1;
                break;
            }
        }
    }

    if (!success)
    {
        printf("Failed 5 Times!!\n");
    }

    close(sockfd);
    return 0;
}