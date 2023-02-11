int connect_to_server(int port_number)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port_number);

    if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }
    return sockfd;
}

void init(char *buf, const int BUFF_SIZE)
{
    for (int i = 0; i < BUFF_SIZE; i++)
        buf[i] = '\0';
}

void send_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE)
{
    init(buf, BUFF_SIZE);
    int i, bp = 0;
    for (i = 0; line[i] != '\0'; i++)
    {
        if (bp == BUFF_SIZE)
        {
            send(sockfd, buf, BUFF_SIZE, 0);
            bp = 0;
            init(buf, BUFF_SIZE);
        }
        buf[bp++] = line[i];
    }
    send(sockfd, buf, strlen(buf) + 1, 0);
}

char *increase_size(char *line, int *line_size)
{
    const int increment = 20;
    if (*line_size == 0)
        line = (char *)malloc(sizeof(char) * increment);
    else
        line = (char *)realloc(line, sizeof(char) * (increment + (*line_size)));
    for (int i = 0; i < increment; i++)
        line[i + (*line_size)] = '\0';
    *line_size += increment;
    return line;
}

void recieve_big_line(int sockfd, char *line, int *LINE_SIZE, char *buf, const int BUFF_SIZE)
{
    *LINE_SIZE = 0;
    free(line);
    int i, flag = 0, lp = 0, n;

    while (1)
    {
        if (flag)
            break;
        init(buf, BUFF_SIZE);
        n = recv(sockfd, buf, BUFF_SIZE, 0);
        for (i = 0; i < n; i++)
        {
            if (buf[i] == '\0')
            {
                flag = 1;
                break;
            }
            if (lp >= (*LINE_SIZE))
                line = increase_size(line, LINE_SIZE);
            line[lp++] = buf[i];
        }
    }
}

char *take_line_input(char *line, int *line_size)
{
    char c;
    int i;
    while ((c = getchar()) != '\n')
    {
        if (i >= *line_size)
            line = increase_size(line, line_size);
        line[i++] = c;
    }
    return line;
}