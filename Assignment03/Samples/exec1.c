#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc,char* argv[]){
	//        int execve(const char *pathname, char *const argv[],char *const envp[]);
	int status;
	char* argv1[]={"./forkN","2","4",NULL};
	status= execve("./forkN", argv1,NULL);
	if(status==-1){
		perror("Exec fails\n");
	}
	exit(0);
}
