#include <stdio.h>     /* for printf(),..*/
#include <stdlib.h>    /* for rand(), exit(), .. */
#include <time.h>      /* for random time seed*/
#include <sys/types.h> /* for predefined structs like pid_t ...*/
#include <sys/ipc.h>   /* for shmget(), shmat() ,...*/
#include <sys/shm.h>   /* for shmget(), shmat() ,....*/
#include <unistd.h>    /* for usleep(), ...*/

typedef struct
{
    char question[100];
    char answer[100];
    time_t tm;
    int status;
    int rollno;
} task;

task *t; // pointer to a task type structure

int shmid; // shmid used to attach and release shared memory

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
    if (key == -1)
    {
        perror("ftok() error at worker\n");
        exit(0);
    }
    shmid = shmget(key, sizeof(task), 0777);
    if (shmid == -1)
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
        if (t->status == 1) // Server has already put the data in the segment, so worker can do its job
        {
            printf("Worker with pid %d is answering the question :- %s \n", getpid(), t->question);
            t->status = 2; // before sort
            int roll;
            printf("Enter the rollno\n");
            scanf("%d", &roll);
            t->rollno = roll;
            printf("Enter the answer:\n");
            // fgets(t->answer, sizeof(t->answer), stdin);
            getchar();
            scanf("%s", t->answer);
            t->tm = time(NULL);
            t->status = 3; // done sorting
        }
        goto exit_here;
    }
    // t->status = -1 , i.e, server has released shared memory segment
exit_here:
    printf("Student with rollno %d is exiting\n", t->rollno);
    return 0;
}
