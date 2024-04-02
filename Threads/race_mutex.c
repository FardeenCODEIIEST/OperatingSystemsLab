#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int mails = 0;
pthread_mutex_t mutex;
void *routine()
{
    for (int i = 0; i < 10000000; i++)
    {
        pthread_mutex_lock(&mutex);
        mails++;
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[])
{
    pthread_t p1, p2;
    pthread_mutex_init(&mutex, NULL);
    if (pthread_create(&p1, NULL, &routine, NULL))
    {
        return 1;
    }
    if (pthread_create(&p2, NULL, &routine, NULL))
    {
        return 2;
    }
    if (pthread_join(p1, NULL))
    {
        return 3;
    }
    if (pthread_join(p2, NULL))
    {
        return 4;
    }
    pthread_mutex_destroy(&mutex);
    printf("Number of mailes: %d\n", mails);
}
