#ifndef MYSOCKET_LIB
#define MYSOCKET_LIB

#include <sys/socket.h>
#include <sys/types.h>

#define SOCK_MyTCP 47
#define MAX_MSG_LENGTH 5000
#define MAX_TABLE_LENGTH 10
#define MAX_SEND_BYTES 1000 

struct Message
{
    char *msg;
    int msg_size; 
};

struct Message_Table
{
    struct Message *msgs;
    int count; 
    int in, out; 
};

int mysocket(int domain, int type, int protocol);
int mybind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int mylisten(int socket, int backlog); 
int myaccept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len); 
int myconnect(int socket, const struct sockaddr *address, socklen_t address_len);
ssize_t mysend(int socket, const void *buffer, size_t length, int flags); 
ssize_t myrecv(int socket, void *buffer, size_t length, int flags);
int myclose(int fildes);

#endif  