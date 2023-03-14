#include "mysocket.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    // Create socket
    int sockfd = mysocket(AF_INET, SOCK_MyTCP, 0);
    if (sockfd == -1)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    // Create client address
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(20000);

    // Connect the socket
    int connect_ret = myconnect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (connect_ret == -1)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    char buff[100];
    for (int i = 0; i < 100; i++)
    {
        buff[i] = 0;
    }

    // Send and recieve call
    myrecv(sockfd, buff, 100, 0);
    printf("%s\n", buff);
    mysend(sockfd, buff, strlen(buff) + 1, 0);

    myclose(sockfd); 

    return 0;
}