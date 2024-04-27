#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

void *func(void *args)
{
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
	pid_t p=fork();
	if(p==-1)
	{
		perror("fork:");
		return -1;
	}
	else if(p==0)
	{
		pthread_t t;
		pthread_create(&t,NULL,func,NULL);
		pthread_join(t,NULL);
		printf("Inside child, pid:- %d\n",getpid());
		return 0;
	}
	else
	{
		int s;
		wait(&s);
		pthread_t th;
		pthread_create(&th,NULL,func,NULL);
		pthread_join(th,NULL);
		printf("Inside parent, pid is:- %d\n",getpid());
	}
	return 0;
}
