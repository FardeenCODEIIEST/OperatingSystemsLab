#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Function to compute the element of the product matrix
int matrix_multiply_element(int row, int col, int n, int *A, int *B) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        result += A[row * n + i] * B[i * n + col];
    }
    return result;
}

// Function for the child process
void child_process(int row, int col, int n, int *A, int *B, int *pipe_fd) {
    close(pipe_fd[0]);  // Close the reading end of the pipe in the child process

    int result = matrix_multiply_element(row, col, n, A, B);
    write(pipe_fd[1], &result, sizeof(result));

    close(pipe_fd[1]);  // Close the writing end of the pipe in the child process
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s m n r A00 A01 ... Aij ... B00 B01 ... Bij ...\n", argv[0]);
        exit(1);
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int r = atoi(argv[3]);

    int *A = malloc(m * n * sizeof(int));
    int *B = malloc(n * r * sizeof(int));

    for (int i = 0; i < m * n; i++) {
        A[i] = atoi(argv[4 + i]);
    }

    for (int i = 0; i < n * r; i++) {
        B[i] = atoi(argv[4 + m * n + i]);
    }

    int **product_matrix = malloc(m * sizeof(int *));
    for (int i = 0; i < m; i++) {
        product_matrix[i] = malloc(r * sizeof(int));
    }

    int pipe_fd[2];

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            pipe(pipe_fd);

            pid_t pid = fork();

            if (pid == 0) {  // Child process
                child_process(i, j, n, A, B, pipe_fd);
            } else if (pid > 0) {  // Parent process
                close(pipe_fd[1]);  // Close the writing end of the pipe in the parent process
                wait(NULL);

                read(pipe_fd[0], &product_matrix[i][j], sizeof(int));
                close(pipe_fd[0]);  // Close the reading end of the pipe in the parent process
            } else {
                fprintf(stderr, "Error creating child process\n");
                exit(1);
            }
        }
    }

    // Print the product matrix
    printf("Product Matrix (A X B):\n");
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            printf("%d ", product_matrix[i][j]);
        }
        printf("\n");
    }

    // Free allocated memory
    for (int i = 0; i < m; i++) {
        free(product_matrix[i]);
    }
    free(product_matrix);
    free(A);
    free(B);

    return 0;
}

