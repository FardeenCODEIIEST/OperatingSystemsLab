#include <stdio.h> 	 // for printf(),...
#include <unistd.h>	 // for fork()
#include <stdlib.h>      // for exit()
#include <sys/types.h>	 // for structs pid_t
#include <sys/wait.h>    // for wait()
#include <errno.h>	 // for perror()

int main() {
        int status;
        pid_t pid=0;
        pid_t p1=0;
        printf("Hello World!\n"); /* printing "Hello =World!\n" just like that */

        p1 = fork();
	if(p1==-1){
		perror("Fork Failed:\n");
		exit(0);
	}
        if (p1 == 0) {
                /* this part is executed by child process */
                int i;

                /* the following part is just to keep the child executing something
                    so that it is live for some period and does not terminate */

                for (i=0; i<5; i++) {
                        printf("%d\n", i++);
                        getchar(); /* child process waits for user input, remains alive and does not terminate */
                }
                exit(12); /* the child process terminates with status 12, note that 12 is just a number,
                                 this status (12) is communicated to the parent process (in some form) and the process
                                 may use it (12) through wait() if it wishes to do so.
                              */

        }

       /* this part will be executed only  by the parent  process */
        pid = wait(&status);
	if(pid==-1)
	{
		perror("wait failed :\n");
		exit(0);
	}
	printf("pid = %d status = %d!\n", pid, status); /* Look the value of status that the  parent process prints! */
}
