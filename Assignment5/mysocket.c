#include "mysocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

struct Message_Table *Send_Message, *Recieve_Message;

void *runner_R()
{
    return 0;
}

void *runner_S()
{
    return 0;
}

int mysocket(int domain, int type, int protocol)
{
    if (type != SOCK_MyTCP)
    {
        return -1;
    }
    // Create the socket
    int sockfd = socket(domain, SOCK_STREAM, protocol);
    if (sockfd == -1)
    {
        return -1;
    }

    // Create R and S thread
    pthread_t tid_R, tid_S;
    pthread_create(&tid_R, NULL, runner_R, NULL);
    pthread_create(&tid_S, NULL, runner_S, NULL);

    // Allocate and initialize space for tables
    Send_Message = (struct Message_Table *)malloc(sizeof(struct Message_Table));
    Recieve_Message = (struct Message_Table *)malloc(sizeof(struct Message_Table));

    Send_Message->msgs = (struct Message *)malloc(MAX_TABLE_LENGTH * sizeof(struct Message));
    Recieve_Message->msgs = (struct Message *)malloc(MAX_TABLE_LENGTH * sizeof(struct Message));

    for (int i = 0; i < MAX_TABLE_LENGTH; i++)
    {
        Send_Message->msgs[i].msg = (char *)malloc(MAX_MSG_LENGTH * sizeof(char));
        Recieve_Message->msgs[i].msg = (char *)malloc(MAX_MSG_LENGTH * sizeof(char));
        Send_Message->msgs[i].msg_size = 0;
        Recieve_Message->msgs[i].msg_size = 0;
    }

    Send_Message->count = 0; 
    Send_Message->in = 0;
    Send_Message->out = 0;
    Recieve_Message->count = 0; 
    Recieve_Message->in = 0;
    Recieve_Message->out = 0;

    return sockfd;
}

int mybind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int bind_ret = bind(sockfd, addr, addrlen);
    return bind_ret;
}

int mylisten(int socket, int backlog)
{
    int listen_ret = listen(socket, backlog);
    return listen_ret;
}

int myaccept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len)
{
    int accept_ret = accept(socket, address, address_len);
    return accept_ret;
}

int myconnect(int socket, const struct sockaddr *address, socklen_t address_len)
{
    int connect_ret = connect(socket, address, address_len);
    return connect_ret;
}

/**
 * @brief Producer for the Send_Message table
 * 
 * @param socket 
 * @param buffer 
 * @param length 
 * @param flags 
 * @return ssize_t 
 */
ssize_t mysend(int socket, const void *buffer, size_t len, int flags)
{
    while(1)
    {   
        // Check if the table has free entry
        if(Send_Message->count != MAX_TABLE_LENGTH)
        {
            for(int i = 0; i < len; i++)
            {
                Send_Message->msgs[Send_Message->in].msg[i] = buffer[i];
            }
            return len; 
        }
        sleep(1); 
    }
}

ssize_t myrecv(int socket, void *buffer, size_t lenh, int flags)
{
}

int myclose(int fildes)
{
    int ret_close = close(fildes);
    return ret_close;
}