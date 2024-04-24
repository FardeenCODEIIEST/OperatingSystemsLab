#include <stdio.h>     /* for printf(),..*/
#include <stdlib.h>    /* for rand(), exit(), .. */
#include <time.h>      /* for random time seed*/
#include <sys/types.h> /* for predefined structs like pid_t ...*/
#include <sys/ipc.h>   /* for shmget(), shmat() ,...*/
#include <sys/shm.h>   /* for shmget(), shmat() ,....*/
#include <unistd.h>    /* for usleep(), ...*/
#include <sys/sem.h>

typedef struct
{
    int data[100]; /* The array to hold the numbers put by the server process and sorted by one worker process */

    int size; /* The number of elements of the array data to be sorted. The server process updates this field when putting the numbers in the array data. */

    pid_t worker_pid; /* The pid of the worker process that is sorting (or has sorted)  the size number of elements of the array data. */

    int status; /* status = 0 means that at present there is nothing in the array data[ ] that needs to be sorted.
                   status = 1 means that the server process has put some numbers in the array data[ ] that need to be sorted.
                   status = 2 means that the worker process having pid = worker_pid has started sorting size number of elements of the array data[ ].
                   status = 3 means that the worker process having pid = worker_pid has sorted size number of elements of the array data[ ].
                   status = 4 means that the server process is using the sorted elements of the array data[ ].
                   status = -1 means that the server process has ended. */

    int sequenceNo; /* stores the sequence number of the array */
} task;

task *t; // pointer to a task type structure

int shmid; // shmid used to attach and release shared memory
int semid;

struct sembuff
{
    unsigned short sem_num; // Semaphore number in the set
    short sem_op;           // Semaphore operation
    short sem_flg;          // Operation flags
};

union semnum
{
    int val;
    struct semid_ds *buffer;
    short *array;
} arg;

void my_lock()
{
    struct sembuff sbuf = (0, -1, 0);
    if (semop(semid, &sbuf, 1) == -1)
    {
        perror("semop() error\n");
        exit(0);
    }
}

void my_unlock()
{
    struct sembuff sbuf = (0, 1, 0);
    if (semop(semid, &sbuf, 1) == -1)
    {
        perror("semop error\n");
        exit(0);
    }
}

// Comparator function for sorting
int cmp(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int main(int argc, char *argv[])
{
    task *t;

    // Key generation using ftok() to be used to generate shmid using shmget
    key_t key = ftok("/tmp/", 4);
    key_t k1 = ftok("/tmp/", 3);
    if (key == -1 || k1 == -1)
    {
        perror("ftok() error at worker\n");
        exit(0);
    }
    shmid = shmget(key, sizeof(task), 0777);
    semid = semget(k1, 1, 0777);
    if (shmid == -1 || semid == -1)
    {
        perror("Worker---> shmget(): error\n");
        exit(0);
    }
    // Attaching shared memory
    t = (task *)shmat(shmid, NULL, 0);
    if (t == (void *)-1)
    {
        perror("shmat() error at client");
        exit(-1);
    }

    while (t->status != -1) // Monitoring status of the shared segment
    {
        my_lock();
        if (t->status == 1) // Server has already put the data in the segment, so worker can do its job
        {
            printf("Worker with pid %d is sorting the array [%d]\n", getpid(), t->sequenceNo);
            t->status = 2; // before sort
            qsort(t->data, t->size, sizeof(int), cmp);
            t->status = 3; // done sorting
            t->worker_pid = getpid();
        }
        my_unlock();
        // usleep(200000); // 200 ms --> worker responds slowly to give chance to other workers
    }

    // t->status = -1 , i.e, server has released shared memory segment
    printf("Server has stopped responding\n");
    return 0;
}
