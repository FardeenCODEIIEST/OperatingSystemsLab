#include <stdio.h>
#include <unistd.h>	   // for fork()
#include <sys/wait.h>  // for wait()
#include <sys/types.h> // for wait(), kill(2)
#include <sys/ipc.h>   // for shmget() shmctl()
#include <sys/shm.h>   // for shmget() shmctl()
#include <signal.h>	   // for signal(2) kill(2)
#include <errno.h>	   // for perror()
#include <stdlib.h>	   // for exit()

int shmid; /* stores the id of the  shared memory segment as returned by shmget(). this variable is defined globally so that signal handler can access it for releasing the shared memory segment. */

/* following is a signal handler for the keypress ctrl-c*/
typedef void (*sighandler_t)(int);

void releaseSHM(int signum)
{
	int status;
	// int shmctl(int shmid, int cmd, struct shmid_ds *buf); /* Read the manual for shmctl() */
	status = shmctl(shmid, IPC_RMID, NULL); /* IPC_RMID is the command for destroying the shared memory*/
	if (status == 0)
	{
		fprintf(stderr, "Remove shared memory with id=%d\n", shmid);
	}
	else if (status == -1)
	{
		fprintf(stderr, "Cannot remove shared memory with id=%d\n", shmid);
	}
	else
	{
		fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id=%d\n", shmid);
	}

	// int kill(pid_t pid,int sig)
	status = kill(0, SIGKILL);
	if (status == 0)
	{
		fprintf(stderr, "kill successful\n");
	}
	else if (status == -1)
	{
		perror("kill failed\n");
		fprintf(stderr, "Cannot remove shared memory with id=%d\n", shmid);
	}
	else
	{
		fprintf(stderr, "kill(2) returned wrong value\n");
	}
}

int main()
{
	int status;
	pid_t pid = 0;
	pid_t p1 = 0;

	sighandler_t shandler;
	/* install signal handler */
	// sighandler_t signal(int signum, sighandler_t handler);
	shandler = signal(SIGINT, releaseSHM); /* should we call this seperately in parent and child process */

	shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0777);

	if (shmid == -1)
	{
		perror("shmget() failed\n");
		exit(1);
	}

	printf("shmget() returns shmid= %d\n", shmid);
	p1 = fork();

	if (p1 == 0)
	{
		// child process
		sleep(1);
		int i;
		int *pi_child;
		pi_child = shmat(shmid, NULL, 0);
		if (pi_child == (void *)-1)
		{
			perror("shmat() fails for child\n");
			exit(1);
		}
		for (int i = 0; i < 50; i++)
		{
			printf("Child reads %d\n", *pi_child);
			getchar();
		}
		exit(0);
	}
	else
	{
		// Parent process
		int i;
		int *pi_parent;

		pi_parent = shmat(shmid, NULL, 0);
		if (pi_parent == (void *)-1)
		{
			perror("shmat() fails for parent\n");
			exit(1);
		}
		else
		{
			for (int i = 0; i < 50; i++)
			{
				*pi_parent = i;
				printf("Parent writes %d\n", *pi_parent);
				getchar();
			}
			pid = wait(&status);
			printf("pid=%d status=%d\n", pid, status);
			exit(0);
		}
	}
}
