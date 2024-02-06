#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>

#define MAX_SIZE 50
#define MAX_CHILDREN 50

typedef struct {
    int m, n, r;
    int A[MAX_SIZE][MAX_SIZE];
    int B[MAX_SIZE][MAX_SIZE];
    int result[MAX_SIZE][MAX_SIZE];
    int next_task;
    bool exiting;
} Task;

void multiply(Task* task, int task_id) {
    int row = task_id / task->r;
    int col = task_id % task->r;
    int sum = 0;
    for (int k = 0; k < task->n; k++) {
        sum += task->A[row][k] * task->B[k][col];
    }
    task->result[row][col] = sum;
}

void process_child(Task* task) {
    while (true) {
        if (task->exiting) {
            exit(0);
        }

        int task_id = __sync_fetch_and_add(&task->next_task, 1);
        if (task_id < task->m * task->r) {
            multiply(task, task_id);
        } else {
            usleep(1000);
        }
    }
}

int main() {
    Task* task;
    pid_t child_process[MAX_CHILDREN];
    int shmid = shmget(IPC_PRIVATE, sizeof(Task), IPC_CREAT | 0777);
    if (shmid == -1) {
        perror("shmget error:\n");
        exit(1);
    }
    task = (Task*)shmat(shmid, NULL, 0);
    if (task == (void*)-1) {
        perror("shmat error:\n");
        exit(1);
    }

    task->exiting = false;
    task->next_task = 0;

    for (int i = 0; i < MAX_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork error:\n");
            exit(1);
        } else if (pid == 0) {
            process_child(task);
        } else {
            child_process[i] = pid;
        }
    }

    while (true) {
        printf("Enter the dimensions of the matrices (m n r) or 0 0 0 to exit: ");
        scanf("%d %d %d", &task->m, &task->n, &task->r);
        if (task->m == 0 && task->n == 0 && task->r == 0) {
            task->exiting = true;
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

        task->next_task = 0;

        while (__sync_fetch_and_add(&task->next_task, 0) < task->m * task->r) {
            usleep(1000);
        }

        printf("Result matrix:\n");
        for (int i = 0; i < task->m; i++) {
            for (int j = 0; j < task->r; j++) {
                printf("%d ", task->result[i][j]);
            }
            printf("\n");
        }
    }

    for (int i = 0; i < MAX_CHILDREN; i++) {
        waitpid(child_process[i], NULL, 0);
    }

    shmdt(task);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

