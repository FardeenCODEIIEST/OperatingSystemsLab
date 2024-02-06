#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int mails = 0;

void *routine()
{
    for (int i = 0; i < 1000000; i++)
    {
        mails++; // race condition
    }
}

int main(int argc, char *argv[])
{
    pthread_t p1, p2;
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
    printf("Number of mailes: %d\n", mails);
}