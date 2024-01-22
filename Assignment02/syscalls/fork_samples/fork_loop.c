#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
	pid_t p;
	int i;
	int c=0;
	for(i=0;i<5;i++){
		p=fork();
		printf("%d\n",c);
		c++;
	}
	exit(0);
}
