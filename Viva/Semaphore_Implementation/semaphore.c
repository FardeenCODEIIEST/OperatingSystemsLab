#include <stdio.h>	// for printf(),scanf(),..
#include <pthread.h>	// for mutex,pthread_create(),...
#include <stdatomic.h>	// for atomic operations
#include <unistd.h>     // for sleep()
#include <stdlib.h>	// for atoi()

// Semaphore Struct
typedef struct 
{
    atomic_uint value;		// Semaphore initial value
    pthread_mutex_t mutex;	// For locking of threads
    pthread_cond_t cond;	// For signal and wait
} Semaphore;

// Initialize the semaphore
void semaphore_init(Semaphore *sem, unsigned int value) 
{
    atomic_init(&sem->value, value);		// must be an atomic operation
    // Initialise the mutex and conditional variables
    pthread_mutex_init(&sem->mutex, NULL);	
    pthread_cond_init(&sem->cond, NULL);
}

// Wait (P) operation
void semaphore_wait(Semaphore *sem) 
{
    // Make a lock 
    pthread_mutex_lock(&sem->mutex);
    while (atomic_load(&sem->value) == 0) 
    {
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }
    atomic_fetch_sub(&sem->value, 1);
    // Release Lock
    pthread_mutex_unlock(&sem->mutex);
}

// Signal (V) operation
void semaphore_signal(Semaphore *sem) 
{
    pthread_mutex_lock(&sem->mutex);
    atomic_fetch_add(&sem->value, 1);
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
}

// Destroy the semaphore
void semaphore_destroy(Semaphore *sem) 
{
    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->cond);
}

// Thread function to use the semaphore
void* thread_func(void* arg) 
{
    Semaphore *sem = (Semaphore*) arg;

    printf("Thread %ld: Waiting to enter critical section...\n", pthread_self());
    semaphore_wait(sem);
    printf("Thread %ld: Entered critical section.\n", pthread_self());

    // Simulate work by sleeping
    sleep(1);

    printf("Thread %ld: Leaving critical section.\n", pthread_self());
    semaphore_signal(sem);

    return NULL;
}

int main(int argc,char* argv[]) {
    if(argc!=3)
    {
    	fprintf(stderr,"Usage:- %s <number of threads> <Initial Value of semaphore>\n",argv[0]);
	return -1;
    }
    int num_threads=atoi(argv[1]);
    pthread_t threads[num_threads];
    Semaphore sem;

    semaphore_init(&sem, atoi(argv[2])); // Initialise the semaphore

    // Create multiple threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &sem) != 0) {
            perror("Failed to create thread");
        }
    }

    // Join all threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    semaphore_destroy(&sem);

    return 0;
}

