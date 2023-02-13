#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
Mystring *take_line_input(Mystring *str);
Mystring **parse_words(Mystring *str, int *number_of_words);
// Defination End

// Some useful functions
Mystring *recieve_big_line(int sockfd, Mystring *str);
void send_big_line(int sockfd, Mystring *str);

// Question Specific
int connect_to_server(const Mystring *ip_address, int port_number);
Mystring *get_ip_address(const Mystring *str);
Mystring *get_url(const Mystring *str);
Mystring *get_extension(const Mystring *str);
int get_port_number(const Mystring *str);

int main()
{
    Mystring *str = init(), **words = NULL, *ip_address = NULL, *url = NULL, *http_request = init();
    Mystring *extension = NULL, *response = init(), **response_words;
    Mystring *filedata = init();
    FILE *sendfp; // file pointer to send the file
    FILE *recfp; // file pointer to save the file
    char date[20];
    int number_of_words = 0, port_number, is_get, is_put, response_number_of_words = 0;

    while (1)
    {
        printf("MyOwnBrowser> ");
        str = take_line_input(str);
        words = parse_words(str, &number_of_words);

        if (strcmp(words[0]->str, "QUIT") == 0)
            break;

        is_get = (strcmp(words[0]->str, "GET") == 0 ? 1 : 0);
        is_put = (strcmp(words[0]->str, "PUT") == 0 ? 1 : 0);

        // for put need 3 words and for get need 2 words
        assert(number_of_words > (is_get ? 1 : 2));

        ip_address = get_ip_address(words[1]);
        port_number = get_port_number(words[1]);
        url = get_url(words[1]);

        // for put -> extension from words[2], and for get -> extension from words[1]
        extension = get_extension(words[is_get ? 1 : 2]);

        int sockfd = connect_to_server(ip_address, port_number);

        http_request = clear(http_request);    

        if (is_get)
        {
            // GET/SET header along with Host, Connection
            http_request = push_back(http_request, words[0]->str);
            http_request = push_back(http_request, " ");
            http_request = push_back(http_request, url->str);
            http_request = push_back(http_request, " HTTP/1.1\n");
            http_request = push_back(http_request, "Host: ");
            http_request = push_back(http_request, ip_address->str);
            http_request = push_back(http_request, "\nConnection: Close\nDate: ");

            // Date header
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            sprintf(date, "%02d%02d%02d", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year % 100);    

            http_request = push_back(http_request, date);
            http_request = push_back(http_request, "\nAccept: ");
            http_request = push_back(http_request, extension->str);

            http_request = push_back(http_request, "\nAccept-Language: ");
            // TODO : Find out Accept Language and push back

            // if modified since header
            timeinfo->tm_mday -= 2;
            mktime(timeinfo);
            sprintf(date, "%02d%02d%02d", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year % 100);
            http_request = push_back(http_request, "\nIf-Modified-Since: ");
        }
        if (is_put)
        {
            // GET/SET header along with Host, Connection
            http_request = push_back(http_request, words[0]->str);
            http_request = push_back(http_request, " ");
            http_request = push_back(http_request, url->str);
            http_request = push_back(http_request, "/");
            http_request = push_back(http_request, words[2]->str);
            http_request = push_back(http_request, " HTTP/1.1\n");
            http_request = push_back(http_request, "Host: ");
            http_request = push_back(http_request, ip_address->str);
            http_request = push_back(http_request, "\nConnection: Close\nDate: ");

            // Date header
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            sprintf(date, "%02d%02d%02d", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year % 100);    

            // content language header
            http_request = push_back(http_request, date);
            http_request = push_back(http_request, "\nContent-language: en-us");

            http_request = push_back(http_request, "\nContent-length: ");
            // TODO : push back content length

            http_request = push_back(http_request, "\nContent-type: ");
            http_request = push_back(http_request, extension->str);http_request = push_back(http_request, "\n\n");
            
            // Body of the request
            sendfp = fopen(words[2]->str, "r");
            fseek(sendfp, 0, SEEK_END);
            int file_size = ftell(sendfp);
            fseek(sendfp, 0, SEEK_SET);
            char c; 
            for(int i = 0; i < file_size; i++)
            {
                c = fgetc(sendfp);
                http_request = push_back_character(http_request, c);
            }
            fclose(sendfp);
        }

        send_big_line(sockfd, http_request);

        // ----------------------------------- RECIEVE THE RESPONSE -----------------------------------
        // Recieve from server
        response = recieve_big_line(sockfd, response);
        // parsing response
        response_words = parse_words(response, &response_number_of_words);
        printf("\nResponse: %s\n", response->str);

        if(is_get)
        {
            // Get the file data
            int getting_file_data = 0;
            for(int i = 0; i < response->size; i++)
            {
                if(getting_file_data)
                {
                    filedata = push_back_character(filedata, response->str[i]);
                }
                else if(response->str[i] == '\n' && response->str[i-1] == '\n')
                {
                    getting_file_data = 1;
                }
            }

            // // Get the file name
            char *file_name = strrchr(url->str, '/');
            file_name++;

            //  Save the file
            recfp = fopen(file_name, "w");
            fprintf(recfp, "%s", filedata->str);
            fclose(recfp);

            // CHECK
            filedata = clear(filedata); 

            if(fork() == 0) // Child process to open the file in application
            {
                char* args[] = {"xdg-open", NULL, NULL};
                args[1] = file_name;
                execvp(args[0], args);
                exit(0); 
            }
        }
        if(is_put)
        {

        }


        close(sockfd);

        // freeing up Mystrings
        free(extension->str);
        free(extension);
        free(ip_address->str);
        free(ip_address);
        free(url->str);
        free(url);

        str = clear(str);
        for (int i = 0; i < number_of_words; i++)
        {
            free(words[i]->str);
            free(words[i]);
        }
        free(words);
        for (int i = 0; i < response_number_of_words; i++)
        {
            free(response_words[i]->str);
            free(response_words[i]);
        }
        free(response_words);
    }
    printf("Exiting\n");
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

Mystring *get_url(const Mystring *str)
{
    Mystring *res = init();
    const int start_index = 7;
    int flag = 0;
    for (int i = start_index; i < str->size; i++)
    {
        if (str->str[i] == '/')
            flag = 1;
        else if (str->str[i] == ':')
            break;
        if (flag)
            res = push_back_character(res, str->str[i]);
    }
    return res;
}
Mystring *get_ip_address(const Mystring *str)
{
    Mystring *res = init();
    const int start_index = 7;
    for (int i = start_index; i < str->size; i++)
    {
        if (str->str[i] == '/')
            break;
        res = push_back_character(res, str->str[i]);
    }
    return res;
}

int get_port_number(const Mystring *str)
{
    const int DEFAULT_RESULT = 80;
    int colon_flag = 1, number_flag = 1;
    int res = 0, ten_pow = 1;
    for (int i = str->size - 1; i >= 0 && number_flag && colon_flag; i--)
    {
        if (str->str[i] == ':')
        {
            colon_flag = 0;
            break;
        }
        int ascii = str->str[i] - '0';
        if (ascii >= 0 && ascii <= 9)
        {
            res += (ascii * ten_pow);
            ten_pow *= 10;
        }
        else
            number_flag = 0;
    }
    if (number_flag && !colon_flag)
        return res;
    return DEFAULT_RESULT;
}

int connect_to_server(const Mystring *ip_address, int port_number)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    inet_aton(ip_address->str, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port_number);

    if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        perror("Unable to connect to server\n");
        return -1;
    }
    return sockfd;
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

        n = recv(sockfd, buf, BUFF_SIZE, 0);
        for (i = 0; i < n; i++)
        {
            if (buf[i] == '\0')
            {
                flag = 1;
                break;
            }
            str = push_back_character(str, buf[i]);
        }
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
    if (str->str)
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

Mystring *take_line_input(Mystring *str)
{
    char c;
    while ((c = getchar()) != '\n')
        str = push_back_character(str, c);
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
