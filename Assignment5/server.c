#include "mysocket.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    // Create socket
    int sockfd = mysocket(AF_INET, SOCK_MyTCP, 0);
    if (sockfd == -1)
    {
        perror("Cannot create socket\n");
        exit(0);
    }

    // Create server address
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(20000);

    // Bind the socket
    int bind_ret = mybind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (bind_ret == -1)
    {
        perror("Cannot bind the socket");
        exit(0);
    }

    // Listen to the socket
    int listen_ret = mylisten(sockfd, 5);

    int newsockfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr;

    clilen = sizeof(cliaddr);
    newsockfd = myaccept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
    if (newsockfd == -1)
    {
        perror("Accept error");
        exit(0);
    }

    // Send and recieve calls
    for (int i = 0; i < 20; i++)
    {
        char sbuff[100];
        for (int j = 0; j < 100; j++)
            sbuff[j] = 0;
        sprintf(sbuff, "My message %d", i);
        mysend(newsockfd, sbuff, strlen(sbuff) + 1, 0);
    }

    for (int i = 0; i < 20; i++)
    {
        char rbuff[100];
        for (int j = 0; j < 100; j++)
            rbuff[j] = 0;
        myrecv(newsockfd, rbuff, 100, 0);
        printf("%s\n", rbuff);
    }

    myclose(newsockfd);
    myclose(sockfd);

    return 0;
}