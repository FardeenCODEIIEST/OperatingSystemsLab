#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
	if(argc!=2)
	{
		fprintf(stderr,"Usage:- %s <fileName>\n",argv[0]);
		return -1;
	}
	int filefd=open(argv[1],O_RDWR|O_CREAT,0666);
	if(filefd==-1)
	{
		perror("open error\n");
		return -1;
	}
	pid_t p=fork();
	if(p==-1)
	{
		perror("fork error\n");
		return -1;
	}
	if(p==0)
	{
		// child
		// read some bytes from the file
		off_t bytes_read=12;
		off_t res=lseek(filefd,bytes_read,SEEK_SET);
		if(res==(off_t)-1)
		{
			perror("lseek error\n");
			return -1;
		}
		return 0;
	}
	else
	{
		// parent
		int status;
		pid_t pid=wait(&status);
		// read again from the file
		char buf[30];
		sprintf(buf," %s","is the world champion");
		int b=write(filefd,buf,sizeof(char)*30);
		if(b==-1)
		{
			perror("write error\n");
			return -1;
		}
		printf("Inside parent, wrote this :- %s\n",buf);
	}
	return 0;
}
