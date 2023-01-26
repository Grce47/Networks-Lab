#include <stdio.h>
#include <time.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void recieve_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);
void init(char *buf, const int BUFF_SIZE);
void send_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);
int connect_to_server(int port_number);

long long timeInMilliseconds()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

int main(int argc, char *argv[])
{
    const int BUFF_SIZE = 50, LINE_SIZE = 2000;
    const int TIMEOUT = 5000;
    char buf[BUFF_SIZE], line[LINE_SIZE];

    srand(time(0));

    assert(argc >= 4);
    int l_port = atoi(argv[1]), server1 = atoi(argv[2]), server2 = atoi(argv[3]);

    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in lb_addr, cli_addr;

    int i;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket\n");
        exit(0);
    }

    lb_addr.sin_family = AF_INET;
    lb_addr.sin_addr.s_addr = INADDR_ANY;
    lb_addr.sin_port = htons(l_port);

    if (bind(sockfd, (struct sockaddr *)&lb_addr,
             sizeof(lb_addr)) < 0)
    {
        perror("Unable to bind local address\n");
        exit(0);
    }

    listen(sockfd, 5);

    struct pollfd fds;
    fds.fd = sockfd;
    fds.events = POLLIN;
    int wait_time = TIMEOUT;

    int load1 = 0, load2 = 0;
    long long start, end;

    while (1)
    {
        start = timeInMilliseconds();
        int ret = poll(&fds, 1, wait_time);
        end = timeInMilliseconds();
        wait_time -= (end - start);

        if (ret == -1)
        {
            perror("Error in poll()");
            exit(1);
        }
        else if (ret == 0)
        {
            printf("Timeout!\n");

            newsockfd = connect_to_server(server1);

            strcpy(line, "Send Load");
            send_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
            recieve_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);

            load1 = atoi(line);
            printf("Load recieved from <127.0.0.1:%d> : <%d>\n", server1, load1);

            close(newsockfd);

            newsockfd = connect_to_server(server2);

            strcpy(line, "Send Load");
            send_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
            recieve_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);

            load2 = atoi(line);
            printf("Load recieved from <127.0.0.1:%d> : <%d>\n", server2, load2);

            close(newsockfd);
            wait_time = TIMEOUT;
        }
        else
        {
            printf("Sending client request to 127.0.0.1:%d\n", load1 > load2 ? server2 : server1);
            newsockfd = connect_to_server(load1 > load2 ? server2 : server1);
            strcpy(line, "Send Time");
            send_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
            recieve_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
            close(newsockfd);

            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                               &clilen);

            if (newsockfd < 0)
            {
                perror("Accept error\n");
                exit(0);
            }
            send_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
            close(newsockfd);
        }
    }

    return 0;
}

void send_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE)
{
    init(buf, BUFF_SIZE);
    int i, bp = 0;
    for (i = 0; line[i] != '\0'; i++)
    {
        if (bp == BUFF_SIZE)
        {
            send(sockfd, buf, BUFF_SIZE, 0);
            bp = 0;
            init(buf, BUFF_SIZE);
        }
        buf[bp++] = line[i];
    }
    send(sockfd, buf, strlen(buf) + 1, 0);
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

int connect_to_server(int port_number)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port_number);

    if ((connect(sockfd, (struct sockaddr *)&serv_addr,
                 sizeof(serv_addr))) < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    return sockfd;
}