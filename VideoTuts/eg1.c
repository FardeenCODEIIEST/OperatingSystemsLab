#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		fprintf(stderr,"Usage:- %s [num1] [num2]\n",argv[0]);
		return -1;
	}
	int a=atoi(argv[1]);
	int b=atoi(argv[2]);

	pid_t pid=fork();
	if(pid==-1)
	{
		perror("fork failed\n");
		return -1;
	}
	else if(pid==0)
	{
		// child
		int sum=a+b;
		printf("Computed sum %d , giving it to parent\n",sum);
		return sum;
	}
	else
	{	// parent
		int status;
		pid_t p=wait(&status);
		printf("The sum computed by %d is %d\n",pid,status>>8);
	}
	return 0;
}
