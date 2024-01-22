// main_prog.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s m n r [elements of matrix A] [elements of matrix B]\n", argv[0]);
        return 1;
    }

        // Extract dimensions
    int m = atoi(argv[1]);  // Number of rows for matrix A
    int n = atoi(argv[2]);  // Number of columns for matrix A, Number of rows for matrix B
    int r = atoi(argv[3]);  // Number of columns for matrix B

    if (argc != (4 + m * n + n * r)) {
        fprintf(stderr, "Invalid number of elements provided.\n");
        return 1;
    }

    int matrixA[m][n];
    int matrixB[n][r];

    // Populate matrix A
    int index = 4;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            matrixA[i][j] = atoi(argv[index++]);
        }
    }

    // Populate matrix B
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < r; j++) {
            matrixB[i][j] = atoi(argv[index++]);
        }
    }

    // Result matrix (m x r)
    int resultMatrix[m][r];

    // Create m * r child processes
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {  // Child process
                char rowAStr[10];
		char colBStr[10];

		sprintf(rowAStr,"%d",matrixA[i][0]);
                sprintf(colBStr,"%d",matrixB[0][j]);
	//	char *prog1Args[] = {
        //            "./matrix",
        //            // pass arguments to prog1
	//	    argv[2], // n
        //            rowAStr, // rowA
        //            colBStr, // colB
        //            NULL
         //       };

               // execve(prog1Args[0], prog1Args, NULL);
               // perror("execve");
                execlp("./matrix","./matrix",rowAStr,colBStr,argv[2],(char*)NULL);
		perror("execlp");
		exit(EXIT_FAILURE);
            }
        }
    }

    // Wait for all child processes to complete
    for (int i = 0; i < m * r; i++) {
        int status;
        wait(&status);
        resultMatrix[i / r][i % r] = WEXITSTATUS(status);
    }

    // Print the resulting product matrix
    printf("Product Matrix (A X B):\n");
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            printf("%d\t", resultMatrix[i][j]);
        }
        printf("\n");
    }

    return 0;
}

