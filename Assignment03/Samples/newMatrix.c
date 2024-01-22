// main_program.c

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE sizeof(int) * m * r

int main(int argc, char *argv[]) {
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int r = atoi(argv[3]);
    int A[m][n];
    int B[n][r];
    int B_transpose[r][n];

    int c = 4;

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

    // Create shared memory segment
    key_t key = ftok("/tmp", 'x');
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    int *shared_memory = shmat(shmid, NULL, 0);

    // Fork and calculate products
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork failed:\n");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Child process
                char index_i[10], index_j[10], shmid_str[10];
                snprintf(index_i, sizeof(index_i), "%d", i);
                snprintf(index_j, sizeof(index_j), "%d", j);
                snprintf(shmid_str, sizeof(shmid_str), "%d", shmid);

                char *args[] = {"./multiply", index_i, index_j, shmid_str, NULL};
                execve("./multiply", args, NULL);
                perror("execve failed:\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Parent process waits for all child processes
    for (int i = 0; i < m * r; i++) {
        int status;
        wait(&status);
    }

    // Read from shared memory and print the result matrix
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            printf("%d ", shared_memory[i * r + j]);
        }
        printf("\n");
    }

    // Detach and remove shared memory
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);

    exit(0);
}

