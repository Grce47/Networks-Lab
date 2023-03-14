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

    // Iterative TCP server
    int newsockfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    while (1)
    {
        // Accept call
        clilen = sizeof(cliaddr);
        newsockfd = myaccept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
        if (newsockfd == -1)
        {
            perror("Accept error");
            exit(0);
        }

        // Send and Recieve call
        char buff[100];
        sprintf(buff, "hello"); 
        mysend(newsockfd, buff, strlen(buff) + 1, 0);
        myrecv(newsockfd, buff, 100, 0); 
        printf("%s\n", buff);

        myclose(newsockfd);
    }

    return 0;
}