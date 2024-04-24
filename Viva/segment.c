#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char* argv[])
{
	pid_t pid=fork();
	if(pid==-1)
	{
		perror("fork failed\n");
		return -1;
	}
	if(pid==0)
	{
		// child
		char *buff;
		buff[0]='F';
		buff[1]='B';
		exit(2);

	}
	else
	{
		// parent
		int status;
		pid_t p=wait(&status);
		printf("status is %d\n",status);
		if(WIFEXITED(status))
		{
			printf("Normal status is %d\n",WEXITSTATUS(status));
		}
	}
	return 0;
}
