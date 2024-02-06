#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>

#define MAX_SIZE 20
#define MAX_CHILDREN MAX_SIZE * MAX_SIZE

// Task structure
typedef struct {
    int m, n, r; // Dimensions of matrices
    int A[MAX_SIZE][MAX_SIZE];
    int B[MAX_SIZE][MAX_SIZE];
    int C[MAX_SIZE][MAX_SIZE];
    bool compute_flag[MAX_CHILDREN]; // Flags for each child process
    bool exit_flag; // Flag to signal children to exit
} Task;

// Function to compute the sum of products formed by row of the first matrix and column of the second matrix
int multiply(int n, int *row, int *col) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += row[i] * col[i];
    }
    return sum;
}

// Child process function
void child_process(int child_id, Task *task) {
    while (true) {
        if (task->exit_flag) {
            exit(0);
        }
        if (task->compute_flag[child_id]) {
            int row_index = child_id / task->r;
            int col_index = child_id % task->r;
            int col[MAX_SIZE];
            for (int k = 0; k < task->n; k++) {
                col[k] = task->B[k][col_index];
            }
            task->C[row_index][col_index] = multiply(task->n, task->A[row_index], col);
            task->compute_flag[child_id] = false;
        }
        usleep(1000); // Prevents busy waiting
    }
}

// Main function
int main() {
    int shmid;
    Task *task;
    pid_t children[MAX_CHILDREN];

    // Allocate shared memory for task
    shmid = shmget(IPC_PRIVATE, sizeof(Task), IPC_CREAT | 0777);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory
    task = (Task *)shmat(shmid, NULL, 0);
    if (task == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Initialize task
    task->exit_flag = false;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        task->compute_flag[i] = false;
    }

    // Create child processes
    for (int i = 0; i < MAX_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            child_process(i, task);
        } else {
            children[i] = pid;
        }
    }

    // Main loop for multiple matrix multiplications
    while (true) {
        printf("Enter the dimensions of the matrices (m n r) or 0 0 0 to exit: ");
        scanf("%d %d %d", &task->m, &task->n, &task->r);
        if (task->m == 0 && task->n == 0 && task->r == 0) {
            task->exit_flag = true;
            break;
        }

        if (task->m > MAX_SIZE || task->n > MAX_SIZE || task->r > MAX_SIZE) {
            printf("Dimensions exceed the maximum limit of %d.\n", MAX_SIZE);
            continue;
        }

        printf("Enter elements of first matrix in row-major order:\n");
        for (int i = 0; i < task->m; i++)
            for (int j = 0; j < task->n; j++)
                scanf("%d", &task->A[i][j]);

        printf("Enter elements of second matrix in row-major order:\n");
        for (int i = 0; i < task->n; i++)
            for (int j = 0; j < task->r; j++)
                scanf("%d", &task->B[i][j]);

        // Signal children to start computation
        for (int i = 0; i < task->m * task->r; i++) {
            task->compute_flag[i] = true;
        }

        // Wait for all child processes to complete current task
        bool all_done = false;
        while (!all_done) {
            all_done = true;
            for (int i = 0; i < task->m * task->r; i++) {
                if (task->compute_flag[i]) {
                    all_done = false;
                    break;
                }
            }
            usleep(10000); // Check every 10ms
        }

        // Print the result matrix
        printf("Result matrix:\n");
        for (int i = 0; i < task->m; i++) {
            for (int j = 0; j < task->r; j++) {
                printf("%d ", task->C[i][j]);
            }
            printf("\n");
        }
    }

    // Wait for all child processes to terminate
    for (int i = 0; i < MAX_CHILDREN; i++) {
        waitpid(children[i], NULL, 0);
    }

    // Detach and deallocate shared memory
    shmdt(task);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

