#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void recieve_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);
void send_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);

void init(char *buf, const int BUFF_SIZE)
{
    for (int i = 0; i < BUFF_SIZE; i++)
        buf[i] = '\0';
}

int main()
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
    serv_addr.sin_port = htons(20000);

    if ((connect(sockfd, (struct sockaddr *)&serv_addr,
                 sizeof(serv_addr))) < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    const int USERNAME_SIZE = 25, BUFF_SIZE = 50, LINE_SIZE = 1024;
    int i, read_line_size;
    size_t len;
    char *username = (char *)malloc(USERNAME_SIZE * sizeof(char)), buf[BUFF_SIZE], line[LINE_SIZE];

    init(buf, BUFF_SIZE);

    recv(sockfd, buf, BUFF_SIZE, 0);
    printf("%s ", buf);

    init(username, USERNAME_SIZE);

    read_line_size = getline(&username, &len, stdin);
    if (read_line_size != -1 && username[read_line_size - 1] == '\n')
        username[read_line_size - 1] = '\0';

    char *token = strtok(username, " ");
    if (token != NULL)
        strcpy(username, token);

    send(sockfd, username, strlen(username) + 1, 0);

    init(buf, BUFF_SIZE);

    recv(sockfd, buf, BUFF_SIZE, 0);
    printf("%s\n", buf);

    if (strcmp(buf, "FOUND") == 0)
    {
        int c;
        while (1)
        {
            init(line, LINE_SIZE);
            i = 0;
            while ((c = getchar()) != '\n' && c != EOF)
            {
                line[i++] = c;
            }

            send_big_line(sockfd, line, LINE_SIZE, buf, BUFF_SIZE);

            char *token = strtok(line, " ");
            if (token != NULL && strcmp(token, "exit") == 0)
            {
                break;
            }
            if (strcmp(line, "exit") == 0)
            {
                break;
            }

            recieve_big_line(sockfd, line, LINE_SIZE, buf, BUFF_SIZE);
            printf("%s\n", line);
        }
    }

    close(sockfd);
    return 0;
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
