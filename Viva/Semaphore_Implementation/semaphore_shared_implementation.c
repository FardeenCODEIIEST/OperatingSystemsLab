#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <stdbool.h>

// Assuming counting semaphore so there is no upper bound
atomic_uint* sem_value;

void P()
{
 	while(atomic_load(sem_value)==0)
	{
		printf("Trying to enter the critical section\n");
	}
	atomic_fetch_sub(sem_value,1);
	return ;
}

void V()
{
	atomic_fetch_add(sem_value,1);
	return;
}

int main(int argc,char* argv[])
{
	
	// Make room for the shared variable
	key_t key = ftok("./file.txt",'B');
	key_t key_test=ftok("./file.txt",'R');
	if(key==-1||key_test==-1)
	{
		perror("ftok() error\n");
		return -1;
	}
	int shmid=shmget(key,sizeof(atomic_uint),IPC_CREAT|0777);
	int shmid_test=shmget(key_test,sizeof(int),IPC_CREAT|0777);
	if(shmid==-1||shmid_test==-1)
	{
		perror("shmget() error\n");
		return -1;
	}
	sem_value=(atomic_uint *)shmat(shmid,NULL,0);
	int *value=(int *)shmat(shmid_test,NULL,0);
	if(sem_value==(void *)-1||value==(void *)-1)
	{
		perror("shmat() error\n");
		return -1;
	}
	
	// do stuff	
	P();
	printf("Enter the critical section\n");
	// increment the test value and display it
	*value=*value+1;
	printf("The value of the test variable is :%d\n",*value);
	V();
	return 0;
	
}
