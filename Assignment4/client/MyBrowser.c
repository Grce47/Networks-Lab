#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>


char *recieve_big_line(int sockfd, char *line, int *line_size, char *buf, const int BUFF_SIZE);
char *take_line_input(char *line, int *line_size);
void send_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);
int connect_to_server(const char *ip_address, int port_number);
void parse_words(char *line, int line_size, char *arr[], int number_of_words);
char *parse_port_number(char *word, int *port_number);
void free_array_of_words(char **arr, int number_of_words);

int main()
{
    const int BUFF_SIZE = 50, INCREMENT = 10, NUMBER_OF_WORDS = 3;
    char buf[BUFF_SIZE], *line = NULL, *words[NUMBER_OF_WORDS], *ip_address = NULL;
    int line_size = 0, port_number = 0;
    int sockfd;
    while (1)
    {
        printf("MyOwnBrowser> ");

        line = take_line_input(line, &line_size);

        parse_words(line, line_size, words, NUMBER_OF_WORDS);

        if (strcmp(words[0], "QUIT") == 0)
            break;
        if (words[1] != NULL)
        {
            ip_address = parse_port_number(words[1], &port_number);
            sockfd = connect_to_server(ip_address, port_number);
            if (sockfd != -1)
            {
                send_big_line(sockfd, line, line_size, buf, BUFF_SIZE);
                line = recieve_big_line(sockfd,line,&line_size,buf,BUFF_SIZE);
                printf("%s\n",line);
                close(sockfd);
            }
        }
        free_array_of_words(words, NUMBER_OF_WORDS);
    }
    printf("Exiting\n");
    return 0;
}

/////////////////////////////////////////////////////////////
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

int connect_to_server(const char *ip_address, int port_number)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    inet_aton(ip_address, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port_number);

    if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        perror("Unable to connect to server\n");
        return -1;
    }
    return sockfd;
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

char *parse_port_number(char *word, int *port_number)
{
    *port_number = 80;
    char *ip_address = NULL;
    const int START_INDEX = 7;
    int n = strlen(word), colon_idx = -1, number_flag = 1;
    for (int i = START_INDEX; i < n; i++)
    {
        if (word[i] == '/')
        {
            ip_address = (char *)malloc(sizeof(char) * (i - START_INDEX));
            strncpy(ip_address, word + START_INDEX, i - START_INDEX);
            break;
        }
    }
    for (int i = n - 1; i >= 0; i--)
    {
        if (word[i] == ':')
        {
            colon_idx = i;
            break;
        }
        int ascii = word[i] - '0';
        if (ascii < 0 || ascii > 9)
        {
            number_flag = 0;
            break;
        }
    }
    if (colon_idx == -1 || number_flag == 0)
        return ip_address;
    if (colon_idx == n - 1)
    {
        *port_number = 0;
        return ip_address;
    }

    char *sub_string = (char *)malloc(sizeof(char) * (n - colon_idx));
    strncpy(sub_string, word + colon_idx + 1, n - colon_idx);
    *port_number = atoi(sub_string);
    free(sub_string);
    return ip_address;
}

char *take_line_input(char *line, int *line_size)
{
    *line_size = 0;
    free(line);
    char c;
    int i = 0;
    while ((c = getchar()) != '\n')
    {
        if (i >= *line_size)
            line = increase_size(line, line_size);
        line[i++] = c;
    }
    return line;
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