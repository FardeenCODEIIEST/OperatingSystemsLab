#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc,char* argv[])
{
	if(argc!=4)
	{
		fprintf(stderr,"Usage :- %s %s [num1] [num2]\n",argv[0],argv[1]);
		return -1;
	}
	pid_t p=fork();
	if(p==-1)
	{
		perror("fork() failed\n");
		return -1;
	}
	else if(p==0)
	{
		// child process executes
		char* argList[]={argv[1],argv[2],argv[3],NULL};

		int flag=execve(argv[1],argList,NULL);
		if(flag==-1)
		{
			perror("execve failed\n");
			return -1;
		}
	}
	//else
	//{
		// parent process waits
//		int status;
//		wait(&status);
//		printf("Child process execution completed\n");
//	}
	return 0;
}
