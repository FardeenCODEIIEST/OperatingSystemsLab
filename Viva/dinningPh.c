#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT (phnum+4)%5
#define RIGHT (phnum+1)%5

sem_t mutex;	// mutex to enter the section of eating
sem_t S[5];	// mutex for each fork
int phil[5]={0,1,2,3,4};
int state[5];

void test(int phnum)
{
	if(state[phnum]==HUNGRY&&state[LEFT]!=EATING&&state[RIGHT]!=EATING)
	{
		state[phnum]=EATING;
		sleep(2);
		printf("Philosopher %d is now eating\n",phnum+1);
		sem_post(&S[phnum]);
	}
}

void put_fork(int phnum)
{
	sem_wait(&mutex);
	state[phnum]=THINKING;
	printf("Philosopher %d is putting forks %d and %d down\n",phnum+1,LEFT+1,RIGHT+1);
	printf("Philosopher %d is now thinking\n",phnum+1);
	test(LEFT);
	test(RIGHT);
	sem_post(&mutex);
}

void take_fork(int phnum)
{
	sem_wait(&mutex);
	state[phnum]=HUNGRY;

	printf("Philosopher %d is hungry\n",phnum+1);
	test(phnum);
	sem_post(&mutex);
	sem_wait(&S[phnum]);
	sleep(1);
}

void* philosopher(void* args)
{
	while(1)
	{
		int* phnum=(int*)args;
		sleep(1);
		take_fork(*phnum);
		sleep(0);
		put_fork(*phnum);
	}
}

int main(int argc,char* argv[])
{
	sem_init(&mutex,0,1);
	for(int i=0;i<5;i++)
	{
		sem_init(&S[i],0,0);
	}
	pthread_t threads[5];
	for(int i=0;i<5;i++)
	{
		pthread_create(&threads[i],NULL,philosopher,&phil[i]);
		printf("Philosopher %d is thinking\n",(i+1));
	}
	for(int i=0;i<5;i++)
	{
		pthread_join(threads[i],NULL);
	}
	return 0;
}
