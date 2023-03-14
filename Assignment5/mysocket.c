#include "mysocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

struct Message_Table *Send_Message, *Recieve_Message;
pthread_mutex_t mutex_send_message, mutex_recv_message;
pthread_cond_t cond_send_message, cond_recv_message; 

/**
 * @brief Used for communication
 * Global socket pointer, assigned dusing mysocket and myaccept call
 * For client we need communication via sockfd returned by mysocket call
 * For server we need communication via newsockfd returned by myaccepy call
 */
int glob_sockfd = SOCK_INIT;

/**
 * @brief Producer for Recieve_Message table
 *
 * @param args
 * @return void*
 */
void *runner_R(void *args)
{
    while (1)
    {
        if(glob_sockfd == SOCK_INIT) continue;
        // Waits on a recv call on the TCP socket,
        // receives  data  that  comes  in,  and  interpretes  the  data  to  form  the  message  (the  complete
        // data for which may come in multiple recv calls)
        char header[4];
        int header_pointer = 0;
        while (header_pointer < 4)
        {
            int recv_len = recv(glob_sockfd, header + header_pointer, 4 - header_pointer, 0);
            header_pointer += recv_len;
        }
        int msg_len = 0;
        for (int i = 0; i < 4; i++)
        {
            msg_len *= 10;
            msg_len += header[i] - '0';
        }
        int content_pointer = 0;
        while (content_pointer < msg_len)
        {
            int recv_len = recv(glob_sockfd, Recieve_Message->msgs[Recieve_Message->in].msg + content_pointer, msg_len - content_pointer, 0);
            content_pointer += recv_len;
        }
        Recieve_Message->msgs[Recieve_Message->in].msg_size = msg_len;
        Recieve_Message->in = (Recieve_Message->in + 1) % MAX_TABLE_LENGTH;
        Recieve_Message->count = Recieve_Message->count + 1;
    }
    return 0;
}

/**
 * @brief Consumer for Send_Message Table
 *
 * @param args
 * @return void*
 */
void *runner_S(void *args)
{
    while (1)
    {
        if(glob_sockfd == SOCK_INIT) continue;
        // Check if there is some message to be sent
        if (Send_Message->count != 0)
        {
            // Send the message using one or more send calls on the TCP socket. You can only send a maximum of 1000 bytes in a single send call.
            char header[4];
            // Fill the header
            int content_len = Send_Message->msgs[Send_Message->out].msg_size;
            int temp_len = content_len; 
            for (int i = 0; i < 4; i++)
            {
                header[3 - i] = '0' + temp_len % 10;
                temp_len /= 10;
            }
            // Send the header
            send(glob_sockfd, header, 4, 0);
            // Send the message content
            int buff_pointer = 0;
            while (content_len > 0)
            {
                int len_to_send = content_len;
                if (len_to_send > MAX_SEND_BYTES)
                    len_to_send = MAX_SEND_BYTES;
                content_len -= len_to_send;
                send(glob_sockfd, Send_Message->msgs[Send_Message->out].msg + buff_pointer, len_to_send, 0);
                buff_pointer += len_to_send;
            }
            Send_Message->out = (Send_Message->out + 1) % MAX_TABLE_LENGTH;
            Send_Message->count = Send_Message->count - 1;
        }
        sleep(S_THREAD_SLEEP);
    }
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

    // Assign the global sockfd for communicatio
    glob_sockfd = sockfd;

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

    pthread_mutex_init(&mutex_send_message, NULL);
    pthread_mutex_init(&mutex_recv_message, NULL);
    pthread_cond_init(&cond_send_message, NULL);
    pthread_cond_init(&cond_recv_message, NULL);

    // Create R and S thread
    pthread_t tid_R, tid_S;
    pthread_create(&tid_R, NULL, runner_R, NULL);
    pthread_create(&tid_S, NULL, runner_S, NULL);

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
    // Assign the global sockfd
    glob_sockfd = accept_ret;
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
    // TODO implement using mutex
    while (1)
    {
        // Check if the table has free entry
        if (Send_Message->count != MAX_TABLE_LENGTH)
        {
            for (int i = 0; i < len; i++)
            {
                Send_Message->msgs[Send_Message->in].msg[i] = ((char *)buffer)[i];
            }
            Send_Message->msgs[Send_Message->in].msg_size = len;
            Send_Message->in = (Send_Message->in + 1) % MAX_TABLE_LENGTH;
            Send_Message->count = Send_Message->count + 1;
            // TODO what to return
            return len;
        }
        sleep(1);
    }
}

/**
 * @brief Consumer for the Recieve_Message table
 *
 * @param socket
 * @param buffer
 * @param leng
 * @param flags
 * @return ssize_t
 */
ssize_t myrecv(int socket, void *buffer, size_t len, int flags)
{
    // TODO implement using mutex
    while (1)
    {
        if (Recieve_Message->count != 0)
        {
            int msg_len = Recieve_Message->msgs[Recieve_Message->out].msg_size;
            if (len < msg_len)
                msg_len = len;
            for (int i = 0; i < msg_len; i++)
            {
                ((char *)buffer)[i] = Recieve_Message->msgs[Recieve_Message->out].msg[i];
            }
            Recieve_Message->out = (Recieve_Message->out + 1) % MAX_TABLE_LENGTH;
            Recieve_Message->count = Recieve_Message->count - 1;
            // TODO what to return
            return msg_len;
        }
        sleep(1);
    }
}

int myclose(int fildes)
{
    sleep(5); 
    pthread_mutex_destroy(&mutex_send_message);
    pthread_mutex_destroy(&mutex_recv_message);
    pthread_cond_destroy(&cond_send_message);
    pthread_cond_destroy(&cond_recv_message);
    int ret_close = close(fildes);
    return ret_close;
}