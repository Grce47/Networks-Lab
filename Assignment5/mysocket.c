#include "mysocket.h"
#include <stdio.h>
#include <pthread.h>

void *func(void *params)
{
    printf("trwd\n");
    pthread_exit(0);
}

void *func1(void *params)
{
    printf("new\n");
    pthread_exit(0);
}

int thread_check()
{
    pthread_t tid[2];
    pthread_create(&tid[0], NULL, func, NULL);
    pthread_create(&tid[1], NULL, func1, NULL);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    return 0;
}
