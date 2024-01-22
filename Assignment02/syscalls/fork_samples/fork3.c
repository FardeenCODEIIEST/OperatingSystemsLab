#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
	pid_t p;
	p=fork();
	if(p==0){
	// Program segment B1 to be executed by the child process
		printf("I am a child process!\n");
	}
	else{
	// Program segment B2 to be executed by the parent process
		printf("I am a parent process!\n");
	}
	// Program segment B3 executed by both parent and child process
	printf("Hello there!!\n");
	exit(0);
}
