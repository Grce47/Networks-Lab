/*
    Assignment1 - Part2 - Server File
    Grace Sharma
    20CS30022
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* EVALUATE FUNCTION DECLARATIONS */
double do_op(double ele1, char op, double ele2, int *has_error, char *res);
void eval(char *buf, char *res);

/* THE SERVER PROCESS */

int main()
{
    const int BUF_SIZE = 10, RES_SIZE = 100;
    int sockfd, newsockfd; /* Socket descriptors */
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;

    int i;
    char buf[BUF_SIZE], res[RES_SIZE]; /* We will use this buffer for communication */

    int expression_size = 0;
    char *expression;

    /* The following system call opens a socket. The first parameter
       indicates the family of the protocol to be followed. For internet
       protocols we use AF_INET. For TCP sockets the second parameter
       is SOCK_STREAM. The third parameter is set to 0 for user
       applications.
    */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket\n");
        exit(0);
    }

    /* The structure "sockaddr_in" is defined in <netinet/in.h> for the
       internet family of protocols. This has three main fields. The
       field "sin_family" specifies the family and is therefore AF_INET
       for the internet family. The field "sin_addr" specifies the
       internet address of the server. This field is set to INADDR_ANY
       for machines having a single IP address. The field "sin_port"
       specifies the port number of the server.
    */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    /* With the information provided in serv_addr, we associate the server
       with its port using the bind() system call.
    */
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        perror("Unable to bind local address\n");
        exit(0);
    }

    listen(sockfd, 5); /* This specifies that up to 5 concurrent client
                  requests will be queued up while the system is
                  executing the "accept" system call below.
               */

    /* In this program we are illustrating an iterative server -- one
       which handles client connections one by one.i.e., no concurrency.
       The accept() system call returns a new socket descriptor
       which is used for communication with the server. After the
       communication is over, the process comes back to wait again on
       the original socket descriptor.
    */
    while (1)
    {

        /* The accept() system call accepts a client connection.
           It blocks the server until a client request comes.

           The accept() system call fills up the client's details
           in a struct sockaddr which is passed as a parameter.
           The length of the structure is noted in clilen. Note
           that the new socket descriptor returned by the accept()
           system call is stored in "newsockfd".
        */
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                           &clilen);

        if (newsockfd < 0)
        {
            perror("Accept error\n");
            exit(0);
        }
        printf("New Client Connected\n");
        while (1)
        {
            for (i = 0; i < BUF_SIZE; i++)
                buf[i] = '\0';

            recv(newsockfd, buf, BUF_SIZE, 0);
            if (buf[0] == '-' && buf[1] == '1' && strlen(buf) == 2 && expression_size == 0)
            {
                printf("Client Disconnected\n");
                break;
            }

            int flag = 1;
            for (i = 0; i < BUF_SIZE; i++)
                if (buf[i] == '\0')
                    flag = 0;

            if (expression_size == 0)
                expression = (char *)malloc(BUF_SIZE * sizeof(char));
            else
                expression = realloc(expression, (expression_size + BUF_SIZE) * sizeof(char));
            for (i = expression_size; i < expression_size + BUF_SIZE; i++)
                expression[i] = '\0';
            for (i = 0; i < BUF_SIZE; i++)
                expression[expression_size + i] = buf[i];
            expression_size += BUF_SIZE;

            if (!flag)
            {
                for (i = 0; i < RES_SIZE; i++)
                    res[i] = '\0';
                eval(expression, res);
                printf("Sending = %s\n",res);
                send(newsockfd, res, RES_SIZE, 0);
                free(expression);
                expression_size = 0;
            }
        }
        close(newsockfd);
    }
    return 0;
}

/* EVALUATE FUNCTION */
double do_op(double ele1, char op, double ele2, int *has_error, char *res)
{
    const double ZERO = 0;
    double ans = 0;
    switch (op)
    {
    case '+':
        ans = ele1 + ele2;
        break;
    case '-':
        ans = ele1 - ele2;
        break;
    case '*':
        ans = ele1 * ele2;
        break;
    case '/':
        if (ele2 == ZERO)
        {
            *has_error = 1;
            strcpy(res, "Error Message: Cant Divide by Zero");
        }
        else
            ans = ele1 / ele2;
        break;
    default:
        *has_error = 1;
        strcpy(res, "Error Message: Invalid Operation");
        break;
    }
    return ans;
}

int isSpace(char c)
{
    return (c == ' ' || c == '\t');
}

void eval(char *buf, char *res)
{
    printf("RECV = %s\n",buf);
    const int SUB_LEN = 25;
    char substr[SUB_LEN], *tmp_ptr;
    double ans = 0, prev_ans = 0;
    int has_prev = 0, bracket_started = 0, has_error = 0;
    char op = '+', prev_op = '+';
    for (int i = 0; buf[i] != '\0'; i++)
    {
        if (has_error)
            break;
        if (isSpace(buf[i]))
            continue;
        if (buf[i] == '(')
        {
            bracket_started = 1;
            prev_ans = ans;
            ans = 0;
            has_prev = 1;
            prev_op = op;
            op = '+';
        }
        else if (buf[i] == ')')
        {
            if(!bracket_started)
            {
                has_error = 1;
                strcpy(res,"Error Message: Unbalanced Brackets");
                break;
            }
            bracket_started = 0;
            ans = do_op(prev_ans, prev_op, ans, &has_error, res);
            op = '+';
            has_prev = 0;
        }
        else if (buf[i] == '+')
        {
            op = '+';
        }
        else if (buf[i] == '-')
        {
            op = '-';
        }
        else if (buf[i] == '*')
        {
            op = '*';
        }
        else if (buf[i] == '/')
        {
            op = '/';
        }
        else
        {
            int len = 0, points = 0, has_space = 0;
            for (int j = i; buf[j] != '\0'; j++)
            {
                int ascii = buf[j] - '0';
                if (buf[j] == ')' || buf[j] == '+' || buf[j] == '-' || buf[j] == '*' || buf[j] == '/')
                    break;
                if (!(buf[j] == '.' || (ascii >= 0 && ascii <= 9) || isSpace(buf[j])))
                {
                    has_error = 1;
                    strcpy(res, "Error Message: Number Error");
                    break;
                }
                if (buf[j] == '.')
                {
                    if (has_space)
                    {
                        has_error = 1;
                        strcpy(res, "Error Message: Number Error");
                        break;
                    }
                    points++;
                    len++;
                }
                else if (ascii >= 0 && ascii <= 9)
                {
                    if (has_space)
                    {
                        has_error = 1;
                        strcpy(res, "Error Message: Number Error");
                        break;
                    }
                    len++;
                }
                else if (buf[j] == ' ')
                {
                    has_space = 1;
                    len++;
                }
                else
                    break;
            }
            if (points >= 2)
            {
                has_error = 1;
                strcpy(res, "Error Message: Number Error");
                break;
            }
            for (int j = 0; j < SUB_LEN; j++)
                substr[j] = '\0';
            stpncpy(substr, buf + i, len);
            double number = strtod(substr, &tmp_ptr);
            ans = do_op(ans, op, number, &has_error, res);
            i += len - 1;
        }
    }
    if (bracket_started)
    {
        has_error = 1;
        strcpy(res, "Error Message: Unbalanced Brackets");
    }
    if (has_error == 0)
    {
        sprintf(res, "%f", ans);
    }
}