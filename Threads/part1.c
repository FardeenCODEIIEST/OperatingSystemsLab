#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *routine()
{
    printf("Test from threads\n");
    sleep(3);
    printf("Ending threads\n");
}

int main(int argc, char *argv[])
{
    pthread_t t1, t2;
    pthread_create(&t1, NULL, &routine, NULL);
    pthread_create(&t2, NULL, &routine, NULL);
    // wait for thread before fininshing execution
    pthread_join(t1, NULL); // wait for thread
    return 0;
}
/**
 *  Security context, memory and all the declarations are shared among threads
 */
