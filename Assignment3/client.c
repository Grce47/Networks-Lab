#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

void recieve_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);
void init(char *buf, const int BUFF_SIZE);

int main(int argc, char *argv[])
{
    const int BUFF_SIZE = 50, LINE_SIZE = 2000;
    char buf[BUFF_SIZE], line[LINE_SIZE];

    assert(argc >= 2);
    int L_port = atoi(argv[1]);

    int i;
    
    int sockfd;
    struct sockaddr_in serv_addr;


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    // CONNECTING WITH L PORT
    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(L_port);

    if ((connect(sockfd, (struct sockaddr *)&serv_addr,
                 sizeof(serv_addr))) < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    recieve_big_line(sockfd, line, LINE_SIZE, buf, BUFF_SIZE);
    printf("%s", line);

    close(sockfd);
    return 0;
}

void init(char *buf, const int BUFF_SIZE)
{
    for (int i = 0; i < BUFF_SIZE; i++)
        buf[i] = '\0';
}

void recieve_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE)
{
    init(line, LINE_SIZE);
    int lp = 0, i, flag = 0;
    while (1)
    {
        if (flag)
            break;
        init(buf, BUFF_SIZE);
        recv(sockfd, buf, BUFF_SIZE, 0);
        for (i = 0; i < BUFF_SIZE; i++)
        {
            if (buf[i] == '\0')
            {
                flag = 1;
                break;
            }
            line[lp++] = buf[i];
        }
    }
}