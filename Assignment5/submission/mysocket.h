/**
 * @file mysocket.h
 * @author Grace Sharma (20CS30022) and Mradul Agarwal (20CS30034)
 * @brief Networks Assignment5 Header File
 * @version 0.1
 * @date 2023-03-15
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef MYSOCKET_LIB
#define MYSOCKET_LIB

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define SOCK_MyTCP 47
#define SOCK_INIT -47

#define MAX_MSG_LENGTH 5000
#define MAX_TABLE_LENGTH 10
#define MAX_SEND_BYTES 1000
#define S_THREAD_SLEEP 1

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

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_listen(int socket, int backlog);
int my_accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
int my_connect(int socket, const struct sockaddr *address, socklen_t address_len);
ssize_t my_send(int socket, const void *buffer, size_t length, int flags);
ssize_t my_recv(int socket, void *buffer, size_t length, int flags);
int my_close(int fildes);

#endif