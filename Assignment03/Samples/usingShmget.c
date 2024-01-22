#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_SIZE 100

void matrixProduct(int row, int col, int *matrixA, int *matrixB, int *result) {
    for (int i = 0; i < col; ++i) {
        result[i] = 0;
        for (int j = 0; j < row; ++j) {
            result[i] += matrixA[j] * matrixB[i + j * col];
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s m,n,r A_elements B_elements\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int m, n, r;
    m=atoi(argv[1]);
    n=atoi(argv[2]);
    r=atoi(argv[3]);

    if (n <= 0 || m <= 0 || r <= 0) {
        fprintf(stderr, "Invalid dimensions for matrices.\n");
        exit(EXIT_FAILURE);
    }

    int matrixA[m][n], matrixB[n][r], result[m][r];

    // Parse A matrix elements
    int index = 4;
    printf("Matrix A:\n");
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            matrixA[i][j] = atoi(argv[index++]);
            printf("%d ", matrixA[i][j]);
        }
        printf("\n");
    }

    // Parse B matrix elements
    printf("Matrix B:\n");
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < r; ++j) {
            matrixB[i][j] = atoi(argv[index++]);
            printf("%d ", matrixB[i][j]);
        }
        printf("\n");
    }

    // Shared memory setup
    int shmid;
    key_t key = ftok(".", 'a');

    shmid = shmget(key, sizeof(int) * m * r, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    int *sharedResult = (int *)shmat(shmid, NULL, 0);

    // Create m*r child processes
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < r; ++j) {
            pid_t child_pid = fork();

            if (child_pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (child_pid == 0) { // Child process
                matrixProduct(n, r, matrixA[i], matrixB[j], result[i*r+j]);
                // Communicate result to parent using status
                exit(result[i][j]);
            }
        }
    }

    // Wait for all child processes to complete
    int status;
    for (int i = 0; i < m * r; ++i) {
        wait(&status);
        // Retrieve result from status and store in shared memory
        sharedResult[i] = WEXITSTATUS(status);
    }

    // Print the product matrix
    printf("\nProduct Matrix (A X B):\n");
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < r; ++j) {
            printf("%d ", sharedResult[i * r + j]);
        }
        printf("\n");
    }

    // Detach and remove shared memory
    shmdt(sharedResult);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

