#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *str;
    int size, capacity;
} Mystring;

Mystring *init()
{
    Mystring *str = (Mystring *)malloc(sizeof(Mystring));
    str->str = NULL;
    str->size = str->capacity = 0;
    return str;
}

Mystring *increase_size(Mystring *str, const int increment)
{
    if (str->capacity == 0)
        str->str = (char *)malloc(increment);
    else
        str->str = (char *)realloc(str->str, increment + str->capacity);

    for (int i = 0; i < increment; i++)
        str->str[i + str->capacity] = '\0';

    str->capacity += increment;
    return str;
}

Mystring *clear(Mystring *str)
{
    free(str->str);
    str->capacity = str->size = 0;
    return str;
}

Mystring *push_back(Mystring *str, char *msg, int msg_len)
{
    const int increment = 20;
    for (int i = 0; i < msg_len; i++)
    {
        if (str->capacity <= str->size)
        {
            str = increase_size(str, increment);
        }
        str->str[str->size] = msg[i];
        str->size++;
    }
    if (str->size == str->capacity)
        str = increase_size(str, increment);
    return str;
}

int main()
{
    Mystring *str = init();
    char s[100];
    scanf("%s", s);
    str = push_back(str, s, strlen(s));
    printf("%s %d %d\n", str->str, str->size, str->capacity);
    str = push_back(str, s, strlen(s));
    printf("%s %d %d\n", str->str, str->size, str->capacity);
    str = push_back(str, s, strlen(s));
    printf("%s %d %d\n", str->str, str->size, str->capacity);
    str = push_back(str, s, strlen(s));
    printf("%s %d %d\n", str->str, str->size, str->capacity);

    str = clear(str);

    printf("ok\n");
    printf("%d\n", (int)sizeof(str));
    printf("%d %d %d\n", (int)sizeof(str->str), str->size, str->capacity);
    if (str->str != NULL)
        printf("%s\n", str->str);

    str = push_back(str, s, strlen(s));
    printf("%s %d %d\n", str->str, str->size, str->capacity);
    str = push_back(str, s, strlen(s));
    printf("%s %d %d\n", str->str, str->size, str->capacity);
    str = push_back(str, s, strlen(s));
    printf("%s %d %d\n", str->str, str->size, str->capacity);
    str = push_back(str, s, strlen(s));
    printf("%s %d %d\n", str->str, str->size, str->capacity);
    return 0;
}
