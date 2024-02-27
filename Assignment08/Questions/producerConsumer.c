#include <stdio.h>	 // for printf(), scanf(), ..
#include <stdlib.h>	 // for atoi(), exit(), ..
#include <pthread.h> /* for thread functions */
#include <errno.h>	 /* For the macros used here - EAGAIN, EINVAL, EPERM. etc.*/
#include <stdbool.h> // for boolean values
#include <unistd.h>	 // for sleep()

#define MAX_SIZE 10

/* Following functions (producer() and consumer()) will be executed by the threads.
   producer() will be executed by producer threads and
   consumer() will be executed by consumer threads */
/* This signatures of producer()  and consumer() are fixed as imposed by pthread_create().
   That is, they must take 'void *' as parameter and return 'void *' */
void *producer(void *param);
void *consumer(void *param);

/* Defining mutexes */
pthread_mutex_t p_mutex; /* producer mutex */
pthread_mutex_t c_mutex; /* consumer mutex */

// Defining the conditional variables
pthread_cond_t full;
pthread_cond_t empty;

// Circular Queue
typedef struct
{
	int arr[MAX_SIZE]; // circular queue array
	int size;		   // size of the queue
	int ind;		   // points to head
	int tail;		   // points to tail
} Queue;

// Enqueues element in the array
void enqueue(Queue *q, int val)
{
	q->ind = (q->ind + 1) % MAX_SIZE;
	q->arr[q->ind] = val;
	q->size++;
	printf("Enqueued %d in the queue at position %d\n", val, q->ind);
}

// Dequeues element from the array
void dequeue(Queue *q)
{
	q->tail = (q->tail + 1) % MAX_SIZE;
	int val = q->arr[q->tail];
	q->arr[q->tail] = -1; // signifying empty
	q->size--;
	printf("Dequeued %d from the queue \n", val);
}

// Checks the fullness of the queue
bool isFull(Queue *q)
{
	return q->size == MAX_SIZE;
}

// Checks the emptiness of the queue
bool isEmpty(Queue *q)
{
	return q->size == 0;
}

// Displays appropriate error message
void error_message(int status, char *str, int type)
{
	if (type == 0)
	{
		// pthread_create()
		fprintf(stderr, "%s: %s\n", str, status == EAGAIN ? "Insufficient resources to create another thread" : status == EINVAL ? "Invalid settings in attr"
																											: status == EPERM	 ? "No permission to set the scheduling policy and parameters specified in attr"
																																 : "Unknown Error");
	}
	else if (type == 1)
	{
		// pthread_join
		fprintf(stderr, "%s: %s\n", str, status == EDEADLK ? "A deadlock was detected (e.g., two threads tried to join with each other); or  thread  specifies  the calling thread." : status == EINVAL ? "Not a joinable thread"
																																												   : status == ESRCH	? "No thread with the ID thread could be found"
																																																		: "Unknown Error");
	}
	else if (type == 2)
	{
		// pthread_mutex_lock
		fprintf(stderr, "%s :%s\n", str, status == EINVAL ? "The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and the calling thread's priority is higher than the mutex's current priority ceiling" : status == EBUSY ? "The mutex could not be acquired because it was already locked"
																																																										  : status == EAGAIN  ? "The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded"
																																																										  : status == EDEADLK ? "The current thread already owns the mutex"
																																																										  : status == EPERM	  ? "The current thread does not own the mutex"
																																																															  : "Unknown error");
	}
	else if (type == 3)
	{
		// pthread_mutex_unlock
		fprintf(stderr, "%s :%s\n", str, status == EINVAL ? "The value specified by mutex does not refer to an initialized mutex object" : status == EAGAIN ? "The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded"
																																	   : status == EPERM	? "The current thread does not own the mutex"
																																							: "Unknown error");
	}
	else if (type == 4)
	{
		// pthread_mutex_init
		fprintf(stderr, "%s :%s\n", str, status == EAGAIN ? "The system lacked the necessary resources (other than memory) to initialise another mutex" : status == ENOMEM ? "Insufficient memory exists to initialise the mutex"
																																					  : status == EPERM	   ? "The caller does not have the privilege to perform the operation"
																																					  : status == EBUSY	   ? "The implementation has detected an attempt to re-initialise the object referenced by mutex, a previously initialised, but not yet destroyed, mutex"
																																					  : status == EINVAL   ? "The value specified by attr is invalid"
																																					  : status == EBUSY	   ? "The implementation has detected an attempt to destroy the object referenced by mutex while it is locked or referenced (for example, while being used in a pthread_cond_wait() or pthread_cond_timedwait()) by another thread"
																																										   : "Unknown Error");
	}
	else if (type == 5)
	{
		// pthread_cond_init
		fprintf(stderr, "%s :%s\n", str, status == EAGAIN ? "The system lacked the necessary resources (other than memory) to initialise another condition variable" : status == ENOMEM ? "Insufficient memory exists to initialise the condition variable"
																																								   : status == EBUSY	? "The implementation has detected an attempt to re-initialise the object referenced by cond, a previously initialised, but not yet destroyed, condition variable"
																																								   : status == EINVAL	? "The value specified by attr is invalid"
																																														: "Unknown Error");
	}
	else if (type == 6)
	{
		// pthread_cond_wait
		fprintf(stderr, "%s :%s\n", str, status == ETIMEDOUT ? "The time specified by abstime to pthread_cond_timedwait() has passed" : status == EINVAL ? "The value specified by cond, mutex, or abstime is invalid"
																																						 : "Unknown Error");
	}
	else if (type == 7)
	{
		// pthread_cond_signal
		fprintf(stderr, "%s :%s\n", str, status == EINVAL ? "The value cond does not refer to an initialised condition variable" : "Unknown Error");
	}
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s m n\n", argv[0]);
		return 0;
	}
	int m = atoi(argv[1]); // number of producers
	if (m == 0)
	{
		fprintf(stderr, "Number of producers should not be zero\n");
		return 0;
	}
	int n = atoi(argv[2]); // number of consumers
	int status;			   // Error status
	pthread_t ptids[m];	   // Producer thread tids
	pthread_t ctids[n];	   // Consumer thread tids

	// Initialise the attributes of the threads
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	// Initialise mutextes
	status = pthread_mutex_init(&p_mutex, NULL);

	if (status != 0)
	{
		error_message(status, "pthread_mutex_init() failed\n", 4);
		return 0;
	}
	status = pthread_mutex_init(&c_mutex, NULL);

	if (status != 0)
	{
		error_message(status, "pthread_mutex_init() failed\n", 4);
		return 0;
	}

	// Initialise conditional variables
	status = pthread_cond_init(&full, NULL);

	if (status != 0)
	{
		error_message(status, "pthread_cond_init() failed\n", 5);
		return 0;
	}
	status = pthread_cond_init(&empty, NULL);

	if (status != 0)
	{
		error_message(status, "pthread_cond_init() failed\n", 5);
		return 0;
	}

	// Initialise queue
	Queue *q = (Queue *)malloc(sizeof(Queue));
	q->ind = -1;
	q->tail = -1;
	q->size = 0;

	// 	Initialise threads
	for (int i = 0; i < m; i++)
	{
		// threadData=(Queue**)malloc(sizeof(Queue*));
		// threadData=&q;

		status = pthread_create(&ptids[i], &attr, producer, q);

		if (status != 0)
		{
			error_message(status, "pthread_create() failed\n", 0);
			return 0;
		}
	}

	for (int i = 0; i < n; i++)
	{
		// threadData=(Queue**)malloc(sizeof(Queue*));
		// threadData=&q;

		status = pthread_create(&ctids[i], &attr, consumer, q);

		if (status != 0)
		{
			error_message(status, "pthread_create() failed\n", 0);
			return 0;
		}
	}

	// Wait for all the threads --> pthread_join()

	for (int i = 0; i < m; i++)
	{
		status = pthread_join(ptids[i], NULL);
		if (status != 0)
		{
			error_message(status, "pthread_join() failed\n", 1);
			return 0;
		}
	}

	for (int i = 0; i < n; i++)
	{
		status = pthread_join(ctids[i], NULL);
		if (status != 0)
		{
			error_message(status, "pthread_join() failed\n", 1);
			return 0;
		}
	}

	return 0;
}
/* The producer() function is executed by all producer threads */
/* Please note that the signature (return type and parameter type) of such functions (the ones to be executed
   by a thread created by pthread_create() is fixed. That means all the "parameters" that a thread may need
   should be "grouped" and its address should be passed */
void *producer(void *param)
{
	int status;		/* used to store return value from pthread functions */
	Queue *alldata; // Queue

	alldata = (Queue *)param;

	while (1)
	{
		int value;
		printf("Enter the value \n");
		scanf("%d", &value);
		status = pthread_mutex_lock(&p_mutex);
		if (status != 0)
		{
			error_message(status, "pthread_mutex_lock()\n", 2);
			return 0;
		}
		while (isFull(alldata))
		{
			printf("Queue is full cannot enqueue\n");
			status = pthread_cond_wait(&full, &p_mutex);
			if (status != 0)
			{
				error_message(status, "pthread_cond_wait()\n", 6);
				return 0;
			}
		}
		enqueue(alldata, value);

		status = pthread_cond_signal(&empty); // signal consumer threads
		if (status != 0)
		{
			error_message(status, "pthread_cond_signal()\n", 7);
			return 0;
		}
		status = pthread_mutex_unlock(&p_mutex);
		if (status != 0)
		{
			error_message(status, "pthread_mutex_unlock()\n", 3);
			return 0;
		}
		sleep(1);
	}

	/* Free the data malloced in the main thread */
	free(param);

	/* Exit the thread */
	pthread_exit(0);
}

/* The consumer() function is executed by all consumer threads */
/* Please note that the signature (return type and parameter type) of such functions (the ones to be executed
   by a thread created by pthread_create() is fixed. That means all the "parameters" that a thread may need
   should be "grouped" and its address should be passed */
void *consumer(void *param)
{
	int status;		/* used to store return value from pthread functions */
	Queue *alldata; // Queue

	alldata = (Queue *)param;

	while (1)
	{
		status = pthread_mutex_lock(&c_mutex);
		if (status != 0)
		{
			error_message(status, "pthread_mutex_lock()\n", 2);
			return 0;
		}
		while (isEmpty(alldata))
		{
			printf("Queue is empty cannot dequeue\n");
			status = pthread_cond_wait(&empty, &c_mutex);
			if (status != 0)
			{
				error_message(status, "pthread_cond_wait()\n", 6);
				return 0;
			}
		}

		dequeue(alldata);
		status = pthread_cond_signal(&full); // signal producr threads
		if (status != 0)
		{
			error_message(status, "pthread_cond_control()\n", 7);
			return 0;
		}
		status = pthread_mutex_unlock(&c_mutex);
		if (status != 0)
		{
			error_message(status, "pthread_mutex_unlock()\n", 3);
			return 0;
		}
		sleep(1);
	}

	/* Free the data malloced in the main thread */
	free(param);

	/* Exit the thread */
	pthread_exit(0);
}
