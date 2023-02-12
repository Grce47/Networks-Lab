#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mystring Start Defination
typedef struct
{
    char *str;
    int size, capacity;
} Mystring;

Mystring *init();
Mystring *clear(Mystring *str);
Mystring *push_back_character(Mystring *str, char c);
Mystring *push_back(Mystring *str, char *msg, int msg_len);
Mystring *take_line_input(Mystring *str);
Mystring **parse_words(Mystring *str, int *number_of_words);
// Defination End

// Some useful functions
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

void send_big_lines(int sockfd, Mystring *str)
{
    const int BUFF_SIZE = 50;
    char *buf[BUFF_SIZE];

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

Mystring *push_back(Mystring *str, char *msg, int msg_len)
{
    for (int i = 0; i < msg_len; i++)
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
        if (str->str[i] == ' ' || str->str[i] == '\t')
            continue;
        int len = 0;
        while ((i + len) < str->size && str->str[i + len] != ' ' && str->str[i + len] != '\t')
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
        if (str->str[i] == ' ' || str->str[i] == '\t')
            continue;
        int len = 0;
        while ((i + len) < str->size && str->str[i + len] != ' ' && str->str[i + len] != '\t')
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
