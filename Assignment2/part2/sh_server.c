#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int username_exists(const char *username, const char *filename);
void distribute(const char *buf, char *first_part, char *second_part, const int BUFF_SIZE);
void recieve_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);
void send_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE);

void init(char *buf, const int BUFF_SIZE)
{
   for (int i = 0; i < BUFF_SIZE; i++)
      buf[i] = '\0';
}

int main()
{
   int sockfd, newsockfd;
   int clilen;
   struct sockaddr_in cli_addr, serv_addr;

   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      printf("Cannot create socket\n");
      exit(0);
   }

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(20000);

   if (bind(sockfd, (struct sockaddr *)&serv_addr,
            sizeof(serv_addr)) < 0)
   {
      printf("Unable to bind local address\n");
      exit(0);
   }

   listen(sockfd, 5);

   int i;
   const char *filename = "users.txt";
   const int BUFF_SIZE = 50, LINE_SIZE = 1024, USERNAME_SIZE = 25;
   char buf[BUFF_SIZE], username[USERNAME_SIZE], line[LINE_SIZE], first_part[LINE_SIZE], second_part[LINE_SIZE], tmp_line[LINE_SIZE];

   while (1)
   {

      clilen = sizeof(cli_addr);
      newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                         &clilen);

      if (newsockfd < 0)
      {
         printf("Accept error\n");
         exit(0);
      }

      if (fork() == 0)
      {
         close(sockfd);
         printf("-----NEW CLIENT CONNECTED-----\n");

         strcpy(buf, "LOGIN:");
         send(newsockfd, buf, strlen(buf) + 1, 0);
         printf("SENDING : \'LOGIN:\'\n");

         init(username, USERNAME_SIZE);

         recv(newsockfd, username, USERNAME_SIZE, 0);
         printf("RECIEVED: %s\n", username);

         if (username_exists(username, filename))
         {
            strcpy(buf, "FOUND");
            send(newsockfd, buf, strlen(buf) + 1, 0);
            printf("SENDING : \'FOUND\'\n");

            while (1)
            {
               recieve_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
               printf("RECIEVED : %s\n", line);

               distribute(line, first_part, second_part, LINE_SIZE);

               if (strcmp(first_part, "exit") == 0)
               {
                  break;
               }

               init(line, LINE_SIZE);

               if (strcmp(first_part, "cd") == 0)
               {
                  if (strlen(second_part) == 0 || strcmp(second_part, "~") == 0)
                  {
                     // HOME DIRECTORY
                     char *home = getenv("HOME");
                     if (home != NULL)
                     {
                        if (chdir(home) != 0)
                        {
                           strcpy(line, "####");
                        }
                        else
                        {
                           strcpy(line, "Directory Changed to ");
                           if (getcwd(tmp_line, sizeof(tmp_line)) == NULL)
                           {
                              strcpy(line, "####");
                           }
                           else
                           {
                              strcat(line, tmp_line);
                           }
                        }
                     }
                     else
                     {
                        strcpy(line, "####");
                     }
                  }
                  else if (chdir(second_part) != 0)
                  {
                     stpcpy(line, "####");
                  }
                  else
                  {
                     strcpy(line, "Directory Changed to ");
                     if (getcwd(tmp_line, sizeof(tmp_line)) == NULL)
                     {
                        strcpy(line, "####");
                     }
                     else
                     {
                        strcat(line, tmp_line);
                     }
                  }
               }
               else if (strcmp(first_part, "pwd") == 0)
               {
                  if (getcwd(line, sizeof(line)) == NULL)
                  {
                     strcpy(line, "####");
                  }
               }
               else if (strcmp(first_part, "dir") == 0)
               {
                  DIR *dir;
                  struct dirent *ent;
                  if (strlen(second_part) == 0)
                  {
                     strcpy(second_part, ".");
                  }
                  if (strcmp(second_part, "~") == 0)
                  {
                     char *home = getenv("HOME");
                     if (home != NULL)
                     {
                        strcpy(second_part, home);
                     }
                     else
                     {
                        strcpy(line, "####");
                     }
                  }

                  if ((dir = opendir(second_part)) != NULL)
                  {
                     /* print all the files and directories within directory */
                     while ((ent = readdir(dir)) != NULL)
                     {
                        strcpy(tmp_line, ent->d_name);
                        if (strcmp(tmp_line, ".") == 0 || strcmp(tmp_line, "..") == 0)
                           continue;
                        if (strlen(line) != 0)
                           strcat(line, " ");
                        strcat(line, tmp_line);
                     }
                     closedir(dir);
                  }
                  else
                  {
                     /* could not open directory */
                     strcpy(line, "####");
                  }
               }
               else
               {
                  strcpy(line, "$$$$");
               }
               printf("SENDING : %s\n", line);
               send_big_line(newsockfd, line, LINE_SIZE, buf, BUFF_SIZE);
            }
         }
         else
         {
            strcpy(buf, "NOT-FOUND");
            send(newsockfd, buf, strlen(buf) + 1, 0);
            printf("SENDING : \'NOT-FOUND\'\n");
         }

         printf("-----CLIENT DISCONNECTED-----\n");
         close(newsockfd);
         exit(0);
      }

      close(newsockfd);
   }
   return 0;
}

void recieve_big_line(int sockfd, char *line, const int LINE_SIZE, char *buf, const int BUFF_SIZE)
{
   init(line, LINE_SIZE);
   int lp = 0, i, flag = 0;
   while (1)
   {
      if (flag)
         break;
      init(buf, BUFF_SIZE);
      recv(sockfd, buf, BUFF_SIZE, 0);
      for (i = 0; i < BUFF_SIZE; i++)
      {
         if (buf[i] == '\0')
         {
            flag = 1;
            break;
         }
         line[lp++] = buf[i];
      }
   }
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

void distribute(const char *buf, char *first_part, char *second_part, const int BUFF_SIZE)
{
   int i, j, fp = 0, sp = 0, flag = 0;
   init(first_part, BUFF_SIZE);
   init(second_part, BUFF_SIZE);

   for (i = 0; i < BUFF_SIZE; i++)
   {
      if (buf[i] == '\0')
         break;
      if (buf[i] == ' ' && flag)
         break;
      if (buf[i] == ' ')
      {
         flag = 1;
         for (j = i; j < BUFF_SIZE; j++)
         {
            if (buf[j] != buf[i])
               break;
         }
         i = j - 1;
         continue;
      }
      if (flag)
         second_part[sp++] = buf[i];
      else
         first_part[fp++] = buf[i];
   }
}

int username_exists(const char *username, const char *filename)
{
   const int USERNAME_SIZE = 25;
   char line[USERNAME_SIZE];
   FILE *fp = fopen(filename, "r");
   if (fp == NULL)
   {
      perror("Error opening file");
      exit(1);
   }
   int flag = 0;
   while (fgets(line, sizeof(line), fp) != NULL)
   {
      char *ptr_to_newline_char = strchr(line, '\n');
      if (ptr_to_newline_char)
         *ptr_to_newline_char = '\0';

      if (strcmp(line, username) == 0)
      {
         flag = 1;
         break;
      }
   }
   fclose(fp);
   return flag;
}