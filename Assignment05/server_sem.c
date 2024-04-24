
#include <stdio.h>     /* for printf(),..*/
#include <stdlib.h>    /* for rand(), exit(), .. */
#include <time.h>      /* for random time seed*/
#include <sys/types.h> /* for predefined structs like pid_t ...*/
#include <sys/ipc.h>   /* for shmget(), shmat() ,...*/
#include <sys/shm.h>   /* for shmget(), shmat() ,....*/
#include <unistd.h>    /* for usleep(), ...*/
#include <signal.h>    /* for signal hanlder to respond to ctrl-C*/
#include <sys/sem.h>   /* for semget(),semctl(),...*/

#define RANGE 500 // Range of array elements is from [0,499]
#define SIZE 20   // Size of array is 20

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

task *t;

int shmid;
int semid;

union semnum
{
    int val;
    struct semid_ds *buffer;
    short *array;
} arg;

/* following is a signal handler for the keypress ctrl-c*/
typedef void (*sighandler_t)(int);

void releaseSHM(int signum)
{
    int status;
    // int shmctl(int shmid, int cmd, struct shmid_ds *buf); /* Read the manual for shmctl() */
    status = shmctl(shmid, IPC_RMID, NULL); /* IPC_RMID is the command for destroying the shared memory*/
    t->status = -1;                         // Server and workers should stop working
    if (status == 0)
    {
        fprintf(stderr, "Remove shared memory with id=%d\n", shmid);
    }
    else if (status == -1)
    {
        fprintf(stderr, "Cannot remove shared memory with id=%d\n", shmid);
    }
    else
    {
        fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id=%d\n", shmid);
    }

    // int kill(pid_t pid,int sig)
    status = kill(0, SIGKILL);
    if (status == 0)
    {
        fprintf(stderr, "kill successful\n");
    }
    else if (status == -1)
    {
        perror("kill failed\n");
        fprintf(stderr, "Cannot remove shared memory with id=%d\n", shmid);
    }
    else
    {
        fprintf(stderr, "kill(2) returned wrong value\n");
    }
}

void my_lock()
{
    struct sembuff sbuf = (0, -1, 0);
    if (semop(semid, &sbuf, 1) == -1)
    {
        perror("semop error\n");
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

int main(int argc, char *argv[])
{
    int start = 1;     // maintaining sequence number of array, to keep track which worker is sorting which array
    srand(time(NULL)); // For random seed
    sighandler_t shandler;
    /* install signal handler */
    // sighandler_t signal(int signum, sighandler_t handler);
    shandler = signal(SIGINT, releaseSHM); /* should we call this seperately in parent and child process */
    key_t key = ftok("/tmp/", 4);
    if (key == -1)
    {
        perror("ftok() error at server \n");
        exit(0);
    }
    shmid = shmget(key, sizeof(task), IPC_CREAT | 0777);
    if (shmid == -1)
    {
        perror("Server---> shmget(): error\n");
        exit(0);
    }
    t = (task *)shmat(shmid, NULL, 0);
    if (t == (void *)-1)
    {
        perror("shmat() error at server");
        exit(0);
    }
    key_t k1 = ftok("/tmp/", 3);
    if (key == -1)
    {
        perror("ftok() error at server\n");
        exit(0);
    }

    if ((semid == semget(k1, 1, IPC_CREAT | 0777)) == -1)
    {
        perror("semget error\n");
        exit(0);
    }

    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) < 0)
    {
        perror("semctl error\n");
        exit(0);
    }

    /* Initialise task */
    t->status = 0;

    while (t->status != -1)
    {
        // Put values
        if (t->status == 0)
        {
            // int totalNumbers = rand() % 101;
            printf("The number of elements in the data array is %d\n", SIZE);
            t->size = SIZE;
            t->sequenceNo = start++;
            for (int i = 0; i < SIZE; i++)
            {
                t->data[i] = rand() % RANGE;
            }
            t->status = 1;
            printf("The initial unsorted array [%d] is:\n", t->sequenceNo);
            for (int i = 0; i < SIZE; i++)
            {
                printf("%d ", t->data[i]);
            }
            printf("\n");
        }
        // Wait for the status, i.e from worker side, status should be put to 3
        else if (t->status == 3)
        {
            t->status = 4;
            printf("The array has been sorted by worker with pid %d\n", t->worker_pid);
            printf("The sorted array [%d] is :\n", t->sequenceNo);
            for (int i = 0; i < t->size; i++)
            {
                printf("%d ", t->data[i]);
            }
            printf("\n\n");
        }
        else if (t->status == 4)
        {
            t->status = 0;
        }
        //    usleep(10000); // 10 ms --> server feeds data fastly
    }
    return 0;
}
