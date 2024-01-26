#include <stdio.h>     // For printf() ..
#include <unistd.h>    // For fork() ..
#include <sys/types.h> // For pid_t struct ..
#include <sys/wait.h>  // for wait() ..
#include <errno.h>     // for perror() ..
#include <stdlib.h>    // for exit(),atoi() ..
#include <sys/ipc.h>   // for ftok()
#include <sys/shm.h>   // for shmget(),shmat() ..

/* Function that computes the sum of products formed by row of the first matrix and column of the second matrix
 *
 * @params:- (i:index of row ,n:Length of the row/col,r: length of col in product matrix ,int[]: row, int [][]:col,int[]: shared_memory)
 * @return:- void
 */

void multiply(int i,int n,int r,int row[n], int col[n][r],int* shared_memory)
{	
    for(int c1=0;c1<r;c1++){
	int s=0;
	for(int k=0;k<n;k++){
		s+=(row[k]*col[k][c1]);
	}
	shared_memory[i*r+c1]=s;
    }
    return;
}

/*
 * Accepts arguments in the form of -->
 * 	"./[name].exe m n r A0 A1 A2 A3 A4 ... B0 B1 B2 B3 B4 ...."
 * 	Here
 * 	m	---->	number of rows of the first matrix
 * 	n	---->   number of columns of the first matrix, which is equal to number of row of the second matrix
 * 	r	---->   number of columns of the second matrix
 *
 * 	A1,A2,A3,A4.... ------>  Represent the elements of the first matrix in row major order
 * 	B1,B2,B3,B4.... ------>  Represent the elemnets of the second matrix in row major order
 *
 */

int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        /* Least possible input will be "./[name].exe 1 1 1 1 1" ---> argc=6*/
        fprintf(stderr, "Usage :- %s m n r matrix elements in row major order\n", argv[0]);
        exit(0);
    }
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int r = atoi(argv[3]);
    int A[m][n];
    int B[n][r];
    int B_transpose[r][n]; // calculating the transpose to get the elements from the columns without any complexity

    int c = 4; // elements are starting from the 4th index of the argv[] array

    // Initialize matrices A and B
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            A[i][j] = atoi(argv[c++]);
        }
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < r; j++)
        {
            B[i][j] = atoi(argv[c++]);
        }
    }

    for (int i = 0; i < r; i++)
    {
        for (int j = 0; j < n; j++)
        {
            B_transpose[i][j] = B[j][i];
        }
    }
    // Creating shared memory segment
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * m * r, IPC_CREAT | 0777); // generating shmid for allocation/attachment of shared memory segment using shmget()
    if (shmid == -1)
    {
        perror("shmget error:\n");
        exit(0);
    }
    printf("Generated shmid using shmget() is : %d\n", shmid);
    int status; // keeping track of child process exit status
    //  Calculation of product matrix elements
    for (int i = 0; i < m; i++)
    {
            pid_t pid = fork();
            if (pid == -1)
            { // fork failed
                perror("fork failed:\n");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            { // child process
                printf("Child process with pid= %d is created and is trying to calculate the elements at row %d in the product matrix\n", getpid(),i);

                int *shared_memory = shmat(shmid, NULL, 0); // Allocating/Attaching the shared memory segment using shmat()
                if (shared_memory == (void *)-1)
                {
                    perror("shmat() fails for child\n");
                    exit(0);
                }
                // Since shared memory is one-dimensional we need to multiply the row_index with the number of columns in the resultant matrix and add it to the column_index, so that the order remains intact
                //shared_memory[i * r + j] = res;
                multiply(i,n,r,A[i],B,shared_memory);
		int err = shmdt(shared_memory); // detaching the shared memory
                if (err == -1)
                {
                    perror("shmdt() fails for child\n");
                }
                exit(EXIT_SUCCESS);
            }
            else
            { // parent process should wait for child process completion
                pid_t childPid = wait(&status);
                if (childPid == -1)
                {
                    perror("wait failed:\n");
                    exit(0);
                }
                else
                {
                    fprintf(stderr, "Child process with pid=%d is terminating normally\n", childPid);
                }
            }
    }

    // Read from shared memory and print the result matrix
    int *shared_memory = shmat(shmid, NULL, 0); // attaching shared memory segment to parent process
    printf("Within parent: Final matrix is:\n");
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < r; j++)
        {
            if (shared_memory == (void *)-1)
            {
                perror("shmat() fails for parent\n");
                exit(0);
            }
            printf("%d ", shared_memory[i * r + j]);
        }
        printf("\n");
    }

    // Detaching the shared memory
    shmdt(shared_memory);
    // Marking the segment to be destroyed
    shmctl(shmid, IPC_RMID, NULL);

    exit(0);
}
