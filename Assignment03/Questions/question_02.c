#include <stdio.h>	// for printf() ..i
#include <unistd.h>	// for execve() ..
#include <errno.h>  	// for perror() ..
#include <stdlib.h>	// for exit() ..
#include <sys/types.h>	// for structs pid_t
#include <sys/wait.h>

// Considering all the process require only one command line argument

int main(int argc,char* argv[]){
	// numberOfProcess=argc-1;
	if (argc < 2) { 
		/* the program might have been executed as "./a.out", that is, without any command line argument. */
		fprintf(stderr, "Usage: %s d1 d2 d3 ...!\n", argv[0]);
		exit(5); /* terminates the present process executing this program */
			 /* the process which created this process would get 5 * 256 = 1280 in the variable "s", 
			  * had it waited for this process by "wait(&s)". */
	}

	for(int i=1;i<argc;i++){
		printf("%s\n",argv[i]);
	}

	int status;
	pid_t pid=0;
	for(int i=1;i<argc;i+=2){
		pid=fork();
		if(pid==-1){
			perror("fork error:!!\n");
		}
		else if(pid==0){
			// within child process
			char* argNew[]={argv[i],argv[i+1],NULL};
			status=execve(argv[i],argNew,NULL);
			printf("%s\n",argv[i]);
			printf("%s\n",argNew[0]);
			if(status==-1){
				perror("Execve failed:\n");
			}
			exit(0);
		}
		else{
			printf("\nWithin parent process , child process %d is created\n",pid);
		}

	}
	for (int i = 1; i < argc; i+=2) { /* parent process waits for termination of argc number of child processes */

		pid = wait(&status); /* waiting for termination of 1 child process */
		
		if (pid == -1) { /* wait() has failed */
			perror("Within parent process: wait() failed: ");
		} else if (status != 0) { /* child process did not terminate properly */
			fprintf(stderr, "Within parent process: Child process %d terminated abnormally!\n", pid);
		} else {
			fprintf(stderr, "Within parent process: Child process %d terminated normally!\n", pid);
		}

		

	}

	fprintf(stderr, "Within parent process: Parent  process is going to terminate normally!\n");


	return 0;
}

