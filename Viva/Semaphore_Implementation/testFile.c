#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char* argv[])
{

	if(argc!=2)
	{
		fprintf(stderr,"Usage:- %s <number of running process>\n",argv[0]);
		return -1;
	}
	int n=atoi(argv[1]);
	for(int i=1;i<=n;i++)
	{
		pid_t pid=fork();
		if(pid==-1)
		{
			perror("fork error:\n");
			return -1;
		}
		if(pid==0)
		{
			// child
			char* argList[]={"./semaphore_shared_implementation",NULL};
			int status=execve("./semaphore_shared_implementation",argList,NULL);
			if(status==-1)
			{
				perror("execve failed\n");
				return -1;
			}
		}
		else
		{
			int stat;
			wait(&stat);
		}
	}
	return 0;
}
