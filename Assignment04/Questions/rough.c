#include <stdio.h>	// For printf() ..
#include <unistd.h>	// For fork(),usleep(),...
#include <sys/types.h>	// For pid_t struct ..
#include <sys/wait.h>	// for wait() ..
#include <errno.h>	// for perror() ..
#include <stdlib.h>	// for exit(),atoi() ..
#include <sys/ipc.h>	// for ftok()
#include <sys/shm.h>	// for shmget(),shmat() ..
#include <stdbool.h>	// for true,false ,...

#define MAX_SIZE 50	// Maximum dimension of row and column of any matrix
#define MAX_CHILDREN 50	// Maximum number of child process created in the process pool

/* Structure that will be resonsible for each matrix multiplication, i.e one task(type Task) handles one matrix multiplication */
typedef struct{
	int m;					// Number of rows in the first matrix
	int n;					// Number of columns in the first matrix
	int r;					// Number of columns in the second matrix
	int A[MAX_SIZE][MAX_SIZE];		// First Matrix
	int B[MAX_SIZE][MAX_SIZE];		// Second Matrix
	bool computing[MAX_CHILDREN];		// Computation flag for each child prcoess
	bool exiting;				// Flag to indicate that the child processes will exit
}Task;

/* Function that computes the sum of products formed by row of the first matrix and column of the second matrix
 *
 * @params:- (i:index of row ,n:Length of the row/col,r: length of col in product matrix ,int[]: row, int [][]:col,int[]: shared_memory)
 * @return:- void
 *
 */

void multiply(int i,int n,int r,int row[n], int col[n][r],int* shared_memory)
{	
    for(int c1=0;c1<r;c1++){
	int s=0;
	for(int k=0;k<n;k++){
		s+=(row[k]*col[k][c1]);
	}
	shared_memory[i*r+c1]=s;
    }
    return;
}


/* Function that computes a multiplication of two matrices using childProcesses
 *
 *  @params:- (task: Pointer to Task type struct corresponding to a given matrix multiplication,childNumber: Sequence number of child process)
 *  @return:- void
 *
 */

void process_child(Task* task,int childNumber, int shmid2){
  	while(true){
		if(task->exiting){
			// terminate the child process
			exit(0);
		}
		if(task->computing[childNumber]){
			int row=childNumber/(task->r);
			int* sharedMemory=(int *)shmat(shmid2,NULL,0);					// Attach child process to the segment
			if(sharedMemory==(void *)-1){
				perror("shmat: error\n");
				exit(0);
			}
			multiply(row,task->n,task->r,task->A[row],task->B,sharedMemory);
			task->computing[childNumber]=false;						// Finished execution of multiplication
			shmdt(sharedMemory);								// detach child process from the segment
		}
		usleep(1000);	// wait for 1 ms
	}
}

/*
 * Accepts no arguments since we want to perform the multiplication as long as the user wants
 *
 */

int main(int argc, char *argv[])
{
    Task* task;
    pid_t child_process[MAX_CHILDREN];
    // Creating shared memory segment for tasks
    int shmid = shmget(IPC_PRIVATE, sizeof(Task), IPC_CREAT | 0777); // generating shmid for allocation/attachment of shared memory segment using shmget()
    if (shmid == -1)
    {
        perror("shmget error for Task:\n");
        exit(0);
    }
    // Result Matrix ---> shared memory segment
    key_t key=ftok("/tmp/",0);
    int shmid2=shmget(key,sizeof(int)*MAX_SIZE*MAX_SIZE,IPC_CREAT|0777);
    if(shmid2==-1){
	perror("shmget error for result matrix\n");
	exit(0);
    }
    printf("Generated shmid using shmget() for tasks is : %d\n", shmid);
    printf("Generated shmid using shmget() for result matrix is : %d\n", shmid2);
    // Attach the shared memory using shmat()
    task=(Task*)shmat(shmid,NULL,0);
    if(task==(void*)-1){
    	perror("shmat error:\n");
	exit(0);
    }
    int* sharedMemory=(int *)shmat(shmid2,NULL,0);
    if(sharedMemory==(void*)-1){
	 	perror("shmat: error\n");
		exit(0);
    }
    // Task initialization
    task->exiting=false;
    for(int i=0;i<MAX_CHILDREN;i++){
	task->computing[i]=false;
    }
//    task->result=(int **)malloc(sizeof(int*)*MAX_SIZE);
//    for(int i=0;i<MAX_SIZE;i++){
//	task->result[i]=(int *)malloc(sizeof(int)*MAX_SIZE);
//    }
    // create a process pool
    for(int i=0;i<MAX_CHILDREN;i++){
   	pid_t pid=fork();
	if(pid==-1){
		perror("fork error:\n");
		exit(0);
	}
	else if(pid==0){
		// Process child processes
		process_child(task,i,shmid2);
	}
	else{
		// Parent process
		child_process[i]=pid;
	}
    }
    // Loop for matrix multiplication

     while (true) {
        printf("Enter the dimensions of the matrices (m n r) or 0 0 0 to exit: ");
        scanf("%d %d %d", &task->m, &task->n, &task->r);
        if (task->m == 0 && task->n == 0 && task->r == 0) {
            task->exiting = true;
	    // Deallocate memory for result matrix 
//	    for(int i=0;i<MAX_SIZE;i++){
//		free(task->result[i]);
//	    }
//	    free(task->result);
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

        // Indicate the child processes to start computation
        for (int i = 0; i < task->m * task->r; i++) {
            task->computing[i] = true;
        }

        // Wait for all child processes to complete current task
        bool all_done = false;
        while (!all_done) {
            all_done = true;
            for (int i = 0; i < task->m; i++) {	// Total m child processes
                if (task->computing[i]) {
                    all_done = false;
                    break;
                }
            }
            usleep(10000); // Check the status every 10ms
        }

        // Print the result matrix
	printf("Result matrix:\n");
        for (int i = 0; i < task->m; i++) {
            for (int j = 0; j < task->r; j++) {
                printf("%d ", sharedMemory[i*(task->r)+j]);
            }
            printf("\n");
        }
    }

    // Wait for all child processes to terminate
    for(int i=0; i<MAX_CHILDREN; i++){
	waitpid(child_process[i],NULL,0);
    }

    // Detaching the shared memory
    shmdt(task);
    shmdt(sharedMemory);
    // Marking the segment to be destroyed
    shmctl(shmid, IPC_RMID, NULL);
    shmctl(shmid2,IPC_RMID,NULL);
    exit(0);
}
