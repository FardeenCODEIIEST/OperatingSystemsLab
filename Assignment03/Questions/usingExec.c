#include <stdio.h>     // For printf() ..
#include <unistd.h>    // For fork() ..
#include <sys/types.h> // For pid_t struct ..
#include <sys/wait.h>  // for wait() ..
#include <errno.h>     // for perror() ..
#include <stdlib.h>    // for exit(),atoi() ..

/* Function that computes the sum of products formed by row of the first matrix and column of the second matrix
 *
 * @params:- (n:Length of the row/col, int[]: row, int []:col)
 * @return:- The consequent sum of product terms
 */

int multiply(int n, int row[n], int col[n])
{
    int sum = 0;
    for (int i = 0; i < n; i++)
    {
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
    char* A[m][n];
    char* B[n][r];
    char* B_transpose[r][n]; // calculating the transpose to get the elements from the columns without any complexity
    int result[m][r];
    int c = 4;  // elements are starting from the 4th index of the argv[] array 
    pid_t pid;
    // Initialize matrices A and B
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            A[i][j] = argv[c++];
        }
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < r; j++)
        {
            B[i][j] = argv[c++];
        }
    }

    for (int i = 0; i < r; i++)
    {
        for (int j = 0; j < n; j++)
        {
            B_transpose[i][j] = B[j][i];
        }
    }

    // Creating child processes
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < r; j++)
        {
            // fork
            pid = fork();
            if (pid == -1)
            {
                perror("fork failed:\n");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            { // child
		//char* argNew[2*n+4];
		char* argNew[2*n+2];
		argNew[0]="./multiply";
		argNew[1]=argv[2];
		int c=2;
		for(int k=0;k<n;k++){
		   argNew[c]=A[i][k];
		   argNew[c+n]=B_transpose[j][k];
		   c++;
		}
		//char row[8];
		//char col[8];
		//sprintf(row,"%d",i);
		//sprintf(col,"%d",j);
		//argNew[2*n+2]=row;
		//argNew[2*n+3]=col;
                int err=execve("./multiply",argNew,NULL);
		if(err==-1){
			perror("execve failed\n");
		}
            }
        }
    }

    for(int i=0;i<m;i++){
	for(int j=0;j<r;j++){
		int status;
		wait(&status);
		int res=WEXITSTATUS(status);
		//int res_value=res>>16;
		//int row=(res>>8)&0xFF;
		//int col=res&0xFF;
		//result[row][col]=res_value;
		result[i][j]=res;
	}
    }

    // Print the result matrix
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < r; j++)
        {
            printf("%d ", result[i][j]);
        }
        printf("\n");
    }

    exit(EXIT_SUCCESS);
}
