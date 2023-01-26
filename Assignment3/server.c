#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void recieve_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);
void init(char *buf, const int BUFF_SIZE);
void send_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);

int main(int argc, char *argv[])
{
    const int BUFF_SIZE = 50, LINE_SIZE = 2000;
    char buf[BUFF_SIZE], line[LINE_SIZE];

    srand(time(0));

    assert(argc >= 2);
    int server_port = atoi(argv[1]);

    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;

    int i;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(server_port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        perror("Unable to bind local address\n");
        exit(0);
    }

    listen(sockfd, 5);

    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                           &clilen);

        if (newsockfd < 0)
        {
            perror("Accept error\n");
            exit(0);
        }

        recieve_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);

        if (strcmp(line, "Send Load") == 0)
        {
            int random_number = 1 + (rand() % 100);
            sprintf(line, "%d", random_number);
            printf("Load Sent : <%d>\n",random_number);
            send_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
        }
        else if (strcmp(line, "Send Time") == 0)
        {
            time_t rawtime;
            struct tm *timeinfo;

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            strcpy(line, "Server time and date: ");
            strcat(line, asctime(timeinfo));

            printf("Time Sent : %s",line);
            send_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
        }
        else
        {
            strcpy(line, "Unknown Command");
            send_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
        }

        close(newsockfd);
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
