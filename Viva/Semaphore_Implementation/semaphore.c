#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>  // For sleep()
#include <stdlib.h>

typedef struct {
    atomic_uint value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Semaphore;

// Initialize the semaphore
void semaphore_init(Semaphore *sem, unsigned int value) {
    atomic_init(&sem->value, value);
    pthread_mutex_init(&sem->mutex, NULL);
    pthread_cond_init(&sem->cond, NULL);
}

// Wait (P) operation
void semaphore_wait(Semaphore *sem) {
    pthread_mutex_lock(&sem->mutex);
    while (atomic_load(&sem->value) == 0) {
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }
    atomic_fetch_sub(&sem->value, 1);
    pthread_mutex_unlock(&sem->mutex);
}

// Signal (V) operation
void semaphore_signal(Semaphore *sem) {
    pthread_mutex_lock(&sem->mutex);
    atomic_fetch_add(&sem->value, 1);
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
}

// Destroy the semaphore
void semaphore_destroy(Semaphore *sem) {
    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->cond);
}

// Thread function to use the semaphore
void* thread_func(void* arg) {
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
    if(argc!=2)
    {
    	fprintf(stderr,"Usage:- %s <number of threads>\n",argv[0]);
	return -1;
    }
    int num_threads=atoi(argv[1]);
    pthread_t threads[num_threads];
    Semaphore sem;

    semaphore_init(&sem, 2);  // Initialize semaphore with 2 resources

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

