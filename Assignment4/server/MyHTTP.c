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

char *recieve_big_line(int sockfd, char *line, int *line_size, char *buf, const int BUFF_SIZE);
void send_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);
void parse_words(char *line, int line_size, char *arr[], int number_of_words);
void free_array_of_words(char **arr, int number_of_words);
char *make_message(char *line, int *line_size, const char *msg);

int main(int argc, char *argv[])
{
    const char *ACCESS_LOG = "AccessLog.txt";
    const int BUFF_SIZE = 50, IP_ADRESS_SIZE = 100, NUMBER_OF_WORDS = 3;
    char buf[BUFF_SIZE], *line = NULL, client_ip_address[IP_ADRESS_SIZE], *words[NUMBER_OF_WORDS];
    int line_size = 0, client_port_number;

    assert(argc > 1);

    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr, serv_addr;

    int i;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

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

        if (fork() == 0)
        {
            close(sockfd);

            line = recieve_big_line(newsockfd, line, &line_size, buf, BUFF_SIZE);
            strcpy(client_ip_address, inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip_address, sizeof(client_ip_address)));
            client_port_number = ntohs(cli_addr.sin_port);

            parse_words(line, line_size, words, NUMBER_OF_WORDS);

            int to_append = 0;
            if (strcmp(words[0], "GET") == 0)
            {
                line = make_message(line, &line_size, "Get Command");
                to_append = 1;
            }
            else if (strcmp(words[0], "PUT") == 0)
            {
                line = make_message(line, &line_size, "Put Command");
                to_append = 1;
            }
            else
            {
                line = make_message(line, &line_size, "Error : Unknown Command");
            }

            send_big_line(newsockfd, line, line_size, buf, BUFF_SIZE);

            if (to_append)
            {
                FILE *fp = fopen(ACCESS_LOG, "a+");
                if (fp == NULL)
                {
                    perror("Error Opening Access Log File");
                    exit(EXIT_FAILURE);
                }
                time_t rawtime;
                struct tm *timeinfo;

                time(&rawtime);
                timeinfo = localtime(&rawtime);
                fprintf(fp, "<%02d%02d%02d>:<%02d%02d%02d>:<%s>:<%d>:<%s>:<%s>\n", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year % 100, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, client_ip_address, client_port_number, words[0], words[1]);
                fclose(fp);
            }

            free_array_of_words(words, NUMBER_OF_WORDS);
            close(newsockfd);
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}

char *make_message(char *line, int *line_size, const char *msg)
{
    free(line);
    *line_size = strlen(msg);
    line = (char *)malloc(sizeof(char) * (*line_size));
    strcpy(line, msg);
    return line;
}

void parse_words(char *line, int line_size, char *arr[], int number_of_words)
{
    int idx = 0, len = 0;
    for (int i = 0; i < line_size; i++)
    {
        if (idx >= number_of_words)
            break;
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\0')
            continue;
        len = 0;
        while ((i + len) < line_size && line[i + len] != ' ' && line[i + len] != '\t')
            len++;
        arr[idx] = (char *)malloc(sizeof(char) * (len + 1));
        for (int j = 0; j < len; j++)
            arr[idx][j] = line[i + j];
        arr[idx][len] = '\0';
        i += len - 1;
        idx++;
    }
}

void free_array_of_words(char *arr[], int number_of_words)
{
    for (int i = 0; i < number_of_words && arr[i]; i++)
    {
        free(arr[i]);
        arr[i] = NULL;
    }
}

char *increase_size(char *line, int *line_size)
{
    const int increment = 20;
    if (*line_size == 0)
        line = (char *)malloc(sizeof(char) * increment);
    else
        line = (char *)realloc(line, sizeof(char) * (increment + (*line_size)));
    for (int i = 0; i < increment; i++)
        line[i + (*line_size)] = '\0';
    *line_size += increment;
    return line;
}

void init(char *buf, const int BUFF_SIZE)
{
    for (int i = 0; i < BUFF_SIZE; i++)
        buf[i] = '\0';
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

char *recieve_big_line(int sockfd, char *line, int *line_size, char *buf, const int BUFF_SIZE)
{
    *line_size = 0;
    free(line);
    int i, flag = 0, lp = 0, n;

    while (1)
    {
        if (flag)
            break;
        init(buf, BUFF_SIZE);
        n = recv(sockfd, buf, BUFF_SIZE, 0);
        for (i = 0; i < n; i++)
        {
            if (buf[i] == '\0')
            {
                flag = 1;
                break;
            }
            if (lp >= (*line_size))
                line = increase_size(line, line_size);
            line[lp++] = buf[i];
        }
    }
    if (*line_size == 0 || line[*line_size - 1] != '\0')
    {
        line = increase_size(line, line_size);
    }
    return line;
}