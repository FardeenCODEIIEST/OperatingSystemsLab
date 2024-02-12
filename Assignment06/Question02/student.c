#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> /* for semget(2) ftok(3) semop(2) semctl(2) */
#include <sys/ipc.h>   /* for semget(2) ftok(3) semop(2) semctl(2) */
#include <sys/sem.h>   /* for semget(2) semop(2) semctl(2) */
#include <unistd.h>    /* for fork(2) */
#include <signal.h>    /*for signal()*/
#include <sys/shm.h>   /* for shmget()*/
#include <stdlib.h>    /* for exit(3) */

#define NO_SEM 1

#define P(s) semop(s, &Pop, 1);
#define V(s) semop(s, &Vop, 1);

struct sembuf Pop; // P operation buffer
struct sembuf Vop; // V operation buffer

union semun
{
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO (Linux-specific) */
} setvalArg;

int semid;
int shmid;
int *status;

/* following is a signal handler for the keypress ctrl-c*/
typedef void (*sighandler_t)(int);

void releaseSHM(int signum)
{
    int statusChecker, p;
    statusChecker = semctl(semid, 0, IPC_RMID, setvalArg); /* IPC_RMID is the command for destroying the semaphore*/
    p = shmctl(shmid, IPC_RMID, NULL);
    *status = 0;
    Vop.sem_num = 0;
    V(semid); // release the chair
    if (statusChecker == 0 || p == 0)
    {
        fprintf(stderr, "Remove shared memory with id=%d\n", semid);
    }
    else if (statusChecker == -1)
    {
        fprintf(stderr, "Cannot remove shared memory with id=%d\n", semid);
    }
    else
    {
        fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id=%d\n", semid);
    }

    // int kill(pid_t pid,int sig)
    statusChecker = kill(0, SIGKILL);
    if (statusChecker == 0)
    {
        fprintf(stderr, "kill successful\n");
    }
    else if (statusChecker == -1)
    {
        perror("kill failed\n");
        fprintf(stderr, "Cannot remove semaphore with id=%d\n", semid);
    }
    else
    {
        fprintf(stderr, "kill(2) returned wrong value\n");
    }
}

int main()
{
    key_t mykey;
    pid_t pid;

    setvalArg.val = 3;

    /* struct sembuf has the following fields */
    // unsigned short sem_num;  /* semaphore number */
    // short          sem_op;   /* semaphore operation */
    // short          sem_flg;  /* operation flags */

    sighandler_t shandler;
    /* install signal handler */
    // sighandler_t signal(int signum, sighandler_t handler);
    shandler = signal(SIGINT, releaseSHM);
    key_t k = ftok("/tmp/", 3);
    if (k == -1)
    {
        perror("ftok() error\n");
        exit(0);
    }
    shmid = shmget(k, sizeof(int), IPC_CREAT | 0666);
    status = (int *)shmat(shmid, NULL, 0);
    if (status == (void *)-1)
    {
        perror("shmat() failed\n");
        exit(0);
    }
    *status = 1;
    // setvalArg.val = 3;

    /* struct sembuf has the following fields */
    // unsigned short sem_num;  /* semaphore number */
    // short          sem_op;   /* semaphore operation */
    // short          sem_flg;  /* operation flags */

    Pop.sem_flg = SEM_UNDO;
    Pop.sem_op = -1;

    Vop.sem_flg = SEM_UNDO;
    Vop.sem_op = 1;

    // key_t ftok(const char *pathname, int proj_id);
    mykey = ftok("/tmp/", 4);

    if (mykey == -1)
    {
        perror("ftok() failed");
        exit(1);
    }

    // int semget(key_t key, int nsems, int semflg);
    semid = semget(mykey, NO_SEM, IPC_CREAT | 0777);
    if (semid == -1)
    {
        perror("semget() failed");
        exit(1);
    }

    Pop.sem_num = 0;
    int c = semctl(semid, 0, GETVAL, setvalArg);
    if (c == -1)
    {
        perror("semctl() failed");
        exit(1);
    }

    printf("Waiting for chair\n");
    P(semid);
    printf("Please! Enter the classroom\n");
    getchar();
    V(semid);
    return 0;
}