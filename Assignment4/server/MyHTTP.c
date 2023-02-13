#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <time.h>
#include <assert.h>

// Mystring Start Defination
typedef struct
{
    char *str;
    int size, capacity;
} Mystring;

Mystring *init();
Mystring *clear(Mystring *str);
Mystring *push_back_character(Mystring *str, char c);
Mystring *push_back(Mystring *str, const char *msg);
Mystring **parse_words(Mystring *str, int *number_of_words);
// Defination End

// Some useful functions
Mystring *get_extension(const Mystring *str);
Mystring *recieve_big_line(int sockfd, Mystring *str);
void send_big_line(int sockfd, Mystring *str);

void send_file(int sockfd, FILE *fp)
{
    const int BUFF_SIZE = 50;
    int read_length;
    char read_line[BUFF_SIZE];
    while ((read_length = fread(read_line, 1, BUFF_SIZE, fp)) > 0)
    {
        send(sockfd, read_line, read_length, 0);
        memset(read_line, 0, BUFF_SIZE);
    }
}

void recieve_file(int sockfd, FILE *fp, int content_length)
{
    const int BUFF_SIZE = 50;
    int read_size;
    char buf[BUFF_SIZE];
    memset(buf, 0, BUFF_SIZE);
    while (content_length > 0)
    {
        read_size = recv(sockfd, buf, BUFF_SIZE, 0);
        if (read_size == 0)
            break;
        content_length -= read_size;
        fwrite(buf, 1, read_size, fp);
        memset(buf, 0, BUFF_SIZE);
    }
}

int get_file_size(Mystring **words, int n)
{
    for (int i = 1; i < n; i++)
    {
        if (strcmp(words[i - 1]->str, "Content-Length:") == 0)
        {
            return atoi(words[i]->str);
        }
    }
    return -1;
}

int main(int argc, char *argv[])
{
    const char *ACCESS_LOG = "AccessLog.txt";
    printf("Please enter the port number: ");
    int port_number;
    scanf("%d", &port_number);

    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr, serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_number);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Unable to bind local address\n");
        exit(0);
    }

    listen(sockfd, 5);

    const int IP_ADDRESS_SIZE = 100;
    int client_port_number = 0, number_of_words = 0, is_get, is_put;
    char client_ip_address[IP_ADDRESS_SIZE];
    int file_size;
    Mystring *str = init(), **words = NULL, *response = init();

    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (newsockfd < 0)
        {
            perror("Accept error\n");
            exit(0);
        }

        if (fork() == 0)
        {
            Mystring *extension = NULL;
            FILE *recfp; // file pointer to save the file

            close(sockfd);

            // receiving request
            str = recieve_big_line(newsockfd, str);

            printf("-------------------- REQUEST RECIEVED --------------------\n");
            for (int i = 0; i < str->size; i++)
            {
                printf("%c", str->str[i]);
            }
            printf("\n----------------------------------------------------------\n");

            // parsing request
            words = parse_words(str, &number_of_words);

            assert(number_of_words >= 3);

            // push back HTTP/1.1 to response
            response = push_back(response, words[2]->str);

            is_get = (strcmp(words[0]->str, "GET") == 0 ? 1 : 0);
            is_put = (strcmp(words[0]->str, "PUT") == 0 ? 1 : 0);

            extension = get_extension(words[1]);

            int status_code = 400;
            if (!is_get && !is_put)
            {
                push_back(response, " 400 Bad Request");
            }
            else
            {
                // AccessLog.txt handle
                strcpy(client_ip_address, inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip_address, sizeof(client_ip_address)));
                client_port_number = ntohs(cli_addr.sin_port);
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
                assert(number_of_words >= 2);
                fprintf(fp, "<%02d%02d%02d>:<%02d%02d%02d>:<%s>:<%d>:<%s>:<%s>\n", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year % 100, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, client_ip_address, client_port_number, words[0]->str, words[1]->str);
                fclose(fp);

                // RESPONSE OF THE REQUEST
                if (is_get)
                {
                    // FILE
                    char *file_name = words[1]->str;

                    // Check if file exists
                    if (access(file_name, F_OK) == -1)
                    {
                        status_code = 404;
                        push_back(response, " 404 Not Found");
                    }
                    else
                    {
                        if (access(file_name, R_OK) == -1)
                        {
                            status_code = 403;
                            push_back(response, " 403 Forbidden");
                        }
                        else
                        {
                            status_code = 200;
                            push_back(response, " 200 OK");

                            FILE *readfp = fopen(file_name, "r");
                            Mystring *file_content = init();
                            fseek(readfp, 0, SEEK_END);
                            file_size = ftell(readfp);
                            fseek(readfp, 0, SEEK_SET);
                            char c;
                            for (int i = 0; i < file_size; i++)
                            {
                                c = fgetc(readfp);
                                file_content = push_back_character(file_content, c);
                            }
                            fclose(readfp);

                            // Expire header
                            struct tm *expire_timeinfo;
                            expire_timeinfo = timeinfo;
                            expire_timeinfo->tm_mday += 3; // 3 days expire
                            mktime(expire_timeinfo);
                            push_back(response, "\nExpires: ");
                            push_back(response, asctime(expire_timeinfo));

                            // Cache-Control header
                            push_back(response, "Cache-control: no-store");

                            // Content-Language header
                            push_back(response, "\nContent-Language: en-us");

                            // Content-length header
                            push_back(response, "\nContent-Length: ");
                            char content_length[100];
                            sprintf(content_length, "%d", file_size);
                            push_back(response, content_length);

                            // Content-Type header
                            push_back(response, "\nContent-Type: ");
                            push_back(response, extension->str);

                            // Last Modified header
                            push_back(response, "\nLast-Modified: ");
                            // Find the last modified time
                            struct stat file_stat;
                            stat(file_name, &file_stat);
                            struct tm *last_modified_timeinfo;
                            last_modified_timeinfo = localtime(&file_stat.st_mtime);
                            push_back(response, asctime(last_modified_timeinfo));

                            push_back(response, "\n\n");
                            // Body
                        }
                    }
                }
                if (is_put)
                {

                    // Check if the file permission is correct
                    if (access(words[1]->str, F_OK) != -1 && access(words[1]->str, W_OK) == -1)
                    {
                        status_code = 403;
                        push_back(response, " 403 Forbidden");
                    }
                    else
                    {
                        status_code = 200;
                        push_back(response, " 200 OK");
                        //  Save the file
                        recfp = fopen(words[1]->str, "w");
                        file_size = get_file_size(words, number_of_words);
                        recieve_file(newsockfd, recfp, file_size);

                        fclose(recfp);
                    }
                }
            }

            send_big_line(newsockfd, response);
            if (is_get && status_code == 200)
            {
                // send file
                FILE *readfp = fopen(words[1]->str, "r");
                send_file(newsockfd, readfp);
                fclose(readfp);
            }

            printf("-------------------- RESPONSE SENT --------------------\n");
            for (int i = 0; i < response->size; i++)
            {
                printf("%c", response->str[i]);
            }
            printf("\n-------------------------------------------------------\n");

            close(newsockfd);
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}

Mystring *get_extension(const Mystring *str)
{
    int colon_index = str->size;
    for (int i = str->size - 1; i >= 0; i--)
    {
        if (str->str[i] == ':')
        {
            colon_index = i;
            break;
        }
    }
    int start_index = -1;
    for (int i = colon_index - 1; i >= 0; i--)
    {
        if (str->str[i] == '.')
        {
            start_index = i + 1;
            break;
        }
        if (!isalpha(str->str[i]))
            break;
    }

    Mystring *res = init();
    if (start_index == -1)
        res = push_back(res, "text/*");
    else
    {
        char *ext = (char *)malloc(colon_index - start_index + 1);
        for (int i = start_index, j = 0; i < colon_index; i++, j++)
            ext[j] = str->str[i];
        ext[colon_index - start_index] = '\0';
        if (strcmp(ext, "html") == 0)
            res = push_back(res, "text/html");
        else if (strcmp(ext, "pdf") == 0)
            res = push_back(res, "application/pdf");
        else if (strcmp(ext, "jpg") == 0)
            res = push_back(res, "image/jpeg");
        else
            res = push_back(res, "text/*");
        free(ext);
    }
    return res;
}

Mystring *recieve_big_line(int sockfd, Mystring *str)
{
    str = clear(str);
    const int BUFF_SIZE = 50;
    char buf[BUFF_SIZE];
    int flag = 0, n, i;

    while (1)
    {
        if (flag)
            break;

        for (i = 0; i < BUFF_SIZE; i++)
            buf[i] = '\0';

        n = recv(sockfd, buf, BUFF_SIZE, MSG_PEEK);
        for (i = 0; i < n; i++)
        {
            if (buf[i] == '\0')
            {
                flag = 1;
                break;
            }
            str = push_back_character(str, buf[i]);
        }
        n = recv(sockfd, buf, (flag ? i + 1 : BUFF_SIZE), 0);
    }
    return str;
}

void send_big_line(int sockfd, Mystring *str)
{
    const int BUFF_SIZE = 50;
    char buf[BUFF_SIZE];

    int i, bp = 0, j;
    for (j = 0; j < BUFF_SIZE; j++)
        buf[j] = '\0';

    for (i = 0; i < str->size; i++)
    {
        if (bp == BUFF_SIZE)
        {
            send(sockfd, buf, BUFF_SIZE, 0);
            bp = 0;
            for (j = 0; j < BUFF_SIZE; j++)
                buf[j] = '\0';
        }
        buf[bp++] = str->str[i];
    }
    send(sockfd, buf, strlen(buf) + 1, 0);
}

// Implement of Mystring functions
Mystring *init()
{
    Mystring *str = (Mystring *)malloc(sizeof(Mystring));
    str->str = NULL;
    str->size = str->capacity = 0;
    return str;
}

Mystring *clear(Mystring *str)
{
    free(str->str);
    str->capacity = str->size = 0;
    return str;
}

Mystring *push_back_character(Mystring *str, char c)
{
    const int increment = 20;
    if (str->capacity <= str->size + 1)
    {
        if (str->capacity == 0)
            str->str = (char *)malloc(increment);
        else
            str->str = (char *)realloc(str->str, increment + str->capacity);

        for (int i = 0; i < increment; i++)
            str->str[i + str->capacity] = '\0';

        str->capacity += increment;
    }
    str->str[str->size] = c;
    str->size++;
    return str;
}

Mystring *push_back(Mystring *str, const char *msg)
{
    for (int i = 0; i < (int)strlen(msg); i++)
        str = push_back_character(str, msg[i]);
    return str;
}

Mystring **parse_words(Mystring *str, int *number_of_words)
{
    *number_of_words = 0;
    for (int i = 0; i < str->size; i++)
    {
        if (str->str[i] == ' ' || str->str[i] == '\t' || str->str[i] == '\n')
            continue;
        int len = 0;
        while ((i + len) < str->size && str->str[i + len] != ' ' && str->str[i + len] != '\t' && str->str[i + len] != '\n')
            len++;
        i += len - 1;
        *number_of_words = *number_of_words + 1;
    }
    Mystring **arr = (Mystring **)malloc(sizeof(Mystring *) * (*number_of_words));

    for (int i = 0; i < (*number_of_words); i++)
        arr[i] = init();

    int idx = 0;
    for (int i = 0; i < str->size; i++)
    {
        if (str->str[i] == ' ' || str->str[i] == '\t' || str->str[i] == '\n')
            continue;
        int len = 0;
        while ((i + len) < str->size && str->str[i + len] != ' ' && str->str[i + len] != '\t' && str->str[i + len] != '\n')
        {
            arr[idx] = push_back_character(arr[idx], str->str[i + len]);
            len++;
        }
        idx++;
        i += len - 1;
    }
    return arr;
}
// End of Implementation