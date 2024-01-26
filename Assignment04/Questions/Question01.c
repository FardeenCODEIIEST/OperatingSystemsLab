#include <stdio.h>
#include <unistd.h>    // for fork()
#include <sys/wait.h>  // for wait()
#include <sys/types.h> // for wait(), kill(2)
#include <sys/ipc.h>   // for shmget() shmctl()
#include <sys/shm.h>   // for shmget() shmctl()
#include <errno.h>     // for perror()
#include <stdlib.h>    // for exit()
#include <time.h>      //

#define RANGE 12 // max factorial range
#define ROUNDS 5 // number of rounds factorial will be printed

int shmid; /* stores the id of the  shared memory segment as returned by shmget(). this variable is defined globally so that signal handler can access it for releasing the shared memory segment. */

/*
 *  @brief:- returns factorial of a number
    @input:- number (n)
    @returns:- factorial of the number
*/

int factorial(int n)
{
    int f = 1;
    for (int i = 2; i <= n; i++)
    {
        f *= i;
    }
    return f;
}

int main()
{
    int status;
    pid_t pid = 0;
    pid_t p1 = 0;
    srand((unsigned)(time(NULL)));
    shmid = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0777); // stores [n,n!]

    if (shmid == -1)
    {
        perror("shmget() failed\n");
        exit(1);
    }

    printf("shmget() returns shmid= %d\n", shmid);

    for (int k = 0; k < ROUNDS; k++)
    {
        p1 = fork();

        if (p1 == -1)
        {
            perror("Fork failed\n");
            exit(1);
        }

        if (p1 == 0)
        {
            // child process
            // sleep(1);
            int i;
            int *pi_child;
            pi_child = shmat(shmid, NULL, 0);
            if (pi_child == (void *)-1)
            {
                perror("shmat() fails for child\n");
                exit(1);
            }
            int n = *pi_child;
            printf("Inside child: Value of number read is %d\n", n);
            int res = factorial(n);
            pi_child++;
            *pi_child = res;
            printf("Inside child: Value of factorial computed is %d\n", res);
            shmdt(pi_child);
            exit(EXIT_SUCCESS);
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
                int n = rand() % RANGE;
                printf("Within parent: Random number generated is %d\n", n);
                *pi_parent = n;
                // sleep(2);
                int status;
                pi_parent++;
                wait(&status);
                if (WIFEXITED(status))
                {
                    printf("Within Parent: Value of the factorial of %d is %d\n", n, *pi_parent);
                }
                shmdt(pi_parent);
            }
        }
    }
    shmctl(shmid, IPC_RMID, NULL);

    exit(EXIT_SUCCESS);
}
