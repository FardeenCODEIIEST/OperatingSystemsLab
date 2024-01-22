#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
void main(){
	int pipefds[2];
	char buffer[256]={0};

	printf("Parent pid :%d\n",getpid());
	pipe(pipefds);
	write(pipefds[1],"msg",3);
	if(fork()!=0){
		return;
	}
	read(pipefds[0],buffer,3);
	printf("%s\n",buffer);
	printf("PID is :%d\n",getpid());
}
