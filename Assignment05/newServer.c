
#include <stdio.h>     /* for printf(),..*/
#include <stdlib.h>    /* for rand(), exit(), .. */
#include <time.h>      /* for random time seed*/
#include <sys/types.h> /* for predefined structs like pid_t ...*/
#include <sys/ipc.h>   /* for shmget(), shmat() ,...*/
#include <sys/shm.h>   /* for shmget(), shmat() ,....*/
#include <unistd.h>    /* for usleep(), ...*/
#include <signal.h>    /* for signal hanlder to respond to ctrl-C*/
#include <string.h>

#define RANGE 500 // Range of array elements is from [0,499]
#define SIZE 20   // Size of array is 20

typedef struct
{
    char question[100];
    char answer[100];
    time_t tm;
    int status;
    int rollno;
    pid_t worker_id;
} task;

task *t;

int shmid;

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
    /* Initialise task */
    t->status = 0;

    char question[100] = "What is your Name?\n";

    printf("The question is \n%s\n", question);

    while (t->status != -1)
    {
        // Put values
        if (t->status == 0)
        {
            // int totalNumbers = rand() % 101;
            bzero(t->question, 100);
            bzero(t->answer, 100);
            bcopy(question, t->question, strlen(question));
            t->status = 1;
        }
        // Wait for the status, i.e from worker side, status should be put to 3
        else if (t->status == 3)
        {
            printf("The answer given by worker with rollno %d is \n%s\n ", t->rollno, t->answer);
            char buff[20];
            time_t now = t->tm;
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
            printf("Answer was given at : %s\n", buff);
            t->status = 4;
        }
        else if (t->status == 4)
        {
            t->status = 0;
        }
        usleep(10000); // 10 ms --> server feeds data fastly
    }
    return 0;
}
