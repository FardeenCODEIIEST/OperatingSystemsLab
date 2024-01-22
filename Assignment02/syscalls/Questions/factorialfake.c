#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>


int main(int argc,char* argv[]){
	int l=atoi(argv[1]);
	int h=atoi(argv[2]); // using atoi() converting string to integer
	int fact=1;
	for(int i=1;i<=l-1;i++){
		fact*=i;
	}
	pid_t p;
	for(int i=l;i<=h;i++){
		p=fork();
		if(p==0){
			fact*=i;
			printf("Child process :%d factorial of %d is %d\n",i,i,fact);
			exit(0);
		}
	}
	exit(0);
}
