#include <stdio.h> /* for printf(), etc. */
#include <unistd.h> /* for fork(), etc. */
#include <stdlib.h> /* for atoi(), etc. */
#include <sys/types.h> /* for wait(), etc. */
#include <sys/wait.h>  /* for wait(), etc. */

#include <errno.h> /* for perror(), etc. */

int fact( int n); /* defined later at the end of this file */


int main(int argc, char *argv[]) { 
	/* if the program is executed as "./a.out 5 9", 
		argc would become 3,
	        argv[0]    =      "./a.out", 
		argv[1]    =      "5", 
		argv[2]    =      "9". 
	*/

	int i;
        int status;
        pid_t pid=0;

	if (argc < 2) { 
		/* the program might have been executed as "./a.out", that is, without any command line argument. */
		fprintf(stderr, "Usage: %s d1 d2 d3 ...!\n", argv[0]);
		exit(5); /* terminates the present process executing this program */
			 /* the process which created this process would get 5 * 256 = 1280 in the variable "s", 
			    had it waited for this process by "wait(&s)". */
	}

	for (i = 1; i < argc; i++) {
		/* argc number of child process would be created */

		pid = fork(); /* a child process is created */

		if (pid == -1) { /* fork() has failed */
			perror("Within parent process: fork() failed: ");
		} else if (pid == 0) {
			/* this part will be executed only by the child process */

			printf("Within Child process: Child Process No. %d computes factorial of %s: %d!\n", i, argv[i], fact(atoi(argv[i])));
			exit(0); /* child process terminates and wont execute the outer for-loop*/
				/* child process exits with 0, parent process, if waits, would get 0*256 = 0 */
		} else {
			/* this part will be executed only by the parent process */
			printf("Within parent process: Child process No. %d is created with process id %d.\n", i, pid);
		}
	}

	/* argc number of child processes have been created and running in the above for-loop */
	/* let the parent process wait here for termination of all child processes */
	/* what happens if the parent process does not wait and terminates before all the child processes terminate???? */


	for (i = 1; i < argc; i++) { /* parent process waits for termination of argc number of child processes */

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

}

int fact(int n) {
	int i;
	int p = 1;
	for (i = 1; i<=n; i++) {
		p = p*i;
	}
	return (p);
}

