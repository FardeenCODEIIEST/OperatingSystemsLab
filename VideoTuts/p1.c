#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		fprintf(stderr,"Usage:- %s <num1> <num2>\n",argv[0]);
		return -1;
	}
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
	int *sum;
	sum=(int*)shmat(shmid,NULL,0);
	if(sum==(void*)-1)
	{
		perror("shmat error\n");
		return -1;
	}
	int a=atoi(argv[1]);
	int b=atoi(argv[2]);
	*sum=(a+b);
	printf("The sum of %d and %d is %d\n",a,b,*sum);
	return 0;
}
