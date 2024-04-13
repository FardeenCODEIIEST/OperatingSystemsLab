#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <stdbool.h>

int main(int argc,char* argv[])
{
	if(argc!=2)
	{
		fprintf(stderr,"Usage:- %s <Semaphore initial value>\n",argv[0]);
		return -1;
	}
	int sem_value_initial = atoi(argv[1]);
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
	atomic_uint *sem_value=(atomic_uint *)shmat(shmid,NULL,0);
	int *value=(int *)shmat(shmid_test,NULL,0);
	if(sem_value==(void *)-1||value==(void *)-1)
	{
		perror("shmat() error\n");
		return -1;
	}
	// initial the sem value
	atomic_init(sem_value,sem_value_initial);
	*value = 0;
	printf("Initial semaphore value is %d\n", *sem_value);
	// do the job
	while(1)
	{
		//printf("Current semaphore value is:%d\n",atomic_load(sem_value));
		sleep(1);
	}

	return 0;
	
}
