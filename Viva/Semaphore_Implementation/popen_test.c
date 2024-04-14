#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc,char* argv[])
{
	FILE* fp;
	int status;
	char path[256];
	fp=popen("ls *","r");
	if(fp==NULL)
	{
		perror("popen error\n");
		return -1;
	}
	while(fgets(path,256,fp)!=NULL)
	{
		printf("%s\n",path);
	}
	status=pclose(fp);
	if(status==-1)
	{
		perror("pclose error\n");
		return -1;
	}
	/*pid_t p=wait(&status);
	if(p==-1)
	{
		perror("wait error\n");
		return -1;
	}
	printf("Process %d returned with exit status %d successfully\n",p,status);
	*/
	return 0;
}
