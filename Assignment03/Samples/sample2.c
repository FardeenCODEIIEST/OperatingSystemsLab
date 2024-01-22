#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char* argv[]){
	pid_t pid;
	int status;
	pid=fork();
	if(pid==0){
		char* argNew[]={"./sample1",argv[1],NULL};
		int f=execve("./sample1",argNew,NULL);
		if(f==-1){
			perror("execve failed:");
			exit(0);
		}
	}
	else{
		wait(&status);
		printf("The answer is:%d\n",WEXITSTATUS(status));
	}
	exit(0);

}
