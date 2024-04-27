#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *func(void* args)
{
	printf("pid is:- %d,Thread id:- %d\n",getpid(),gettid());
	return NULL;
}

int main()
{
	pthread_t t;
	pthread_create(&t,NULL,func,NULL);
	pthread_join(t,NULL);
	printf("pid is:- %d,Thread id:- %d\n",getpid(),gettid());
	return 0;
}
