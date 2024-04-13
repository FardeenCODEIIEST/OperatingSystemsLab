#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

int main(int argc,char* argv[])
{
	key_t key=ftok("/tmp/",'A');
	if(key==-1)
	{
		perror("ftok failed\n");
		return -1;
	}
	int shmid=shmget(key,sizeof(int),IPC_CREAT|0666);
	if(shmid==-1)
	{
		perror("shmget error\n");
		return -1;
	}
	int *res=(int *)shmat(shmid,NULL,0);
	if(res==(void*)-1)
	{
		perror("shmat failed\n");
		return -1;
	}
	*res=0;
	while(*res==0);
	printf("The computed result is %d\n",*res);

	return 0;
}
