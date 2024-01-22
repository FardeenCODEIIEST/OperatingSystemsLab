#include <stdio.h>	// For printf() ..
#include <unistd.h>	// For fork() ..
#include <sys/types.h>	// For pid_t struct ..
#include <sys/wait.h>	// for wait() ..
#include <errno.h>	// for perror() ..
#include <stdlib.h>	// for exit(),atoi() ..
#include <sys/ipc.h>	// for ftok()
#include <sys/shm.h>	// for shmget(),shmat() ..

/* Function that computes the sum of products formed by row of the first matrix and column of the second matrix
 *
 * @params:- (n:Length of the row/col, int[]: row, int []:col)
 * @return:- The consequent sum of product terms
 */

int multiply(int n, int row[n], int col[n]) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (row[i] * col[i]);
    }
    return sum;
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

int main(int argc, char *argv[]) {
    if(argc<6){
	/* Least possible input will be "./[name].exe 1 1 1 1 1" ---> argc=6*/
	fprintf(stderr,"Usage :- %s m n r matrix elements in row major order\n",argv[0]);
	exit(0);
    }
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int r = atoi(argv[3]);
    int A[m][n];
    int B[n][r];
    int B_transpose[r][n];	// calculating the transpose to get the elements from the columns without any complexity
    
    int c = 4;	// elements are starting from the 4th index of the argv[] array 

    // Initialize matrices A and B
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = atoi(argv[c++]);
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < r; j++) {
            B[i][j] = atoi(argv[c++]);
        }
    }

    for (int i = 0; i < r; i++) {
        for (int j = 0; j < n; j++) {
            B_transpose[i][j] = B[j][i];
        }
    }
    // Creating shared memory segment
    key_t key = ftok("/tmp", 'x');						// generating key needed to generate shmid using ftok()
    int shmid = shmget(key, sizeof(int)*m*r, IPC_CREAT | 0666);			// generating shmid for allocation/attachment of shared memory segment using shmget()
    if(shmid==-1){
	perror("shmget error:\n");
	exit(0);
    }
    int *shared_memory = shmat(shmid, NULL, 0);					// Allocating/Attaching the shared memory segment using shmat()
    int status;									// keeping track of child process exit status
    //  Calculation of product matrix elements
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            pid_t pid = fork();
            if (pid == -1) {	// fork failed
                perror("fork failed:\n");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {	// child process
                printf("Child process with pid= %d is created and is trying to calculate the element at [%d,%d] position in the product matrix\n",getpid(),i,j);
		int res = multiply(n, A[i], B_transpose[j]);
		// Since shared memory is one-dimensional we need to multiply the row_index with the number of columns in the resultant matrix and add it to the column_index, so that the order remains intact
                shared_memory[i * r + j] = res; 				
                exit(EXIT_SUCCESS);
            }
	    else{	// parent process should wait for child process completion 
		pid_t childPid=wait(&status);
		if(childPid==-1){
			perror("wait failed:\n");
			exit(0);
		}
		else{
			fprintf(stderr,"Child process with pid=%d is terminating normally\n",childPid);
		}
	    }
        }
    }

    // Read from shared memory and print the result matrix
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
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

