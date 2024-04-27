#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

void *func(void *args)
{
	printf("Thread id is :- %d\n",gettid());
	pid_t pid=fork();
	if(pid==-1)
	{
		perror("fork error\n");
		exit(-1);
	}
	if(pid==0)
	{
		// child
		printf("Inside child:- pid is :- %d, tid is:- %d\n",getpid(),gettid());
		exit(0);
	}
	else
	{
		int s;
		wait(&s);
		// parent
		printf("Inside parent, pid is:- %d, tid is:- %d\n",getpid(),gettid());
		return NULL;
	}
}

int main()
{
	pthread_t child;
	pthread_create(&child,NULL,func,NULL);
	pthread_join(child,NULL);
	printf("Inside the main process, pid:- %d, tid:- %d\n",getpid(),gettid());
	return 0;
}
