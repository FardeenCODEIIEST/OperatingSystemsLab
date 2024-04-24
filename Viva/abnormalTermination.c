#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		fprintf(stderr,"Usage:- %s <num1> <num2>\n",argv[0]);
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
	if(pid==0)
	{
		// child
		int res=a/b;
		exit(res);

	}
	else
	{
		// parent
		int status;
		pid_t p=wait(&status);
		printf("status is %d\n",status);
		if(WIFEXITED(status))
		{
			printf("%d divided by %d is %d\n",a,b,WEXITSTATUS(status));
		}
	}
	return 0;
}
