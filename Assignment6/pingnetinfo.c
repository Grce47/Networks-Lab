#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>

int main()
{
    struct hostent *host;
    host = gethostbyname("www.google.com");
    printf("IP = %s\n%s\n", host->h_name, inet_ntoa(*(struct in_addr *)host->h_name));
}