#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
	pid_t p;
	p=fork();
	if(p==0) // child process
	{
		printf("Child process!\n");
	}
	else{
		printf("Parent process!\n");
	}
 //	exit(0);
}

