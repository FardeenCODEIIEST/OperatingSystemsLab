#include <stdio.h>     // for printf(),scanf()
#include <stdlib.h>    // for malloc(),exit()
#include <unistd.h>    // for pipe(),usleep()
#include <string.h>    // for strlen(), strcspn()
#include <sys/types.h> // for pid_t..

#define MAX_LEN 100 // max length of the string

typedef struct
{
    float temperature;
    char place[MAX_LEN];
} Weather;

// Generic enqueue
void enqueue(int pipefd[2], void *data, size_t size)
{
    write(pipefd[1], data, size);
}

// Generic dequeue
void *dequeue(int pipefd[2], size_t size)
{
    void *data = malloc(size);
    read(pipefd[0], data, size);
    return data;
}

// Function to enqueue an integer
void enqueueInt(int pipefd[2])
{
    printf("Enter the integer\n");
    int n;
    scanf("%d", &n);
    enqueue(pipefd, &n, sizeof(n));
}

// Function to dequeue an integer
void dequeueInt(int pipefd[2])
{
    int *dequeued = (int *)dequeue(pipefd, sizeof(int));
    printf("Dequeued int: %d\n", *dequeued);
    free(dequeued);
}

// Function to enqueue a double
void enqueueDouble(int pipefd[2])
{
    double n;
    printf("Enter the double data\n");
    scanf("%lf", &n);
    enqueue(pipefd, &n, sizeof(n));
}

// Function to dequeue a double
void dequeueDouble(int pipefd[2])
{
    double *dequeued = (double *)dequeue(pipefd, sizeof(double));
    printf("Dequeued double: %0.4lf\n", *dequeued);
    free(dequeued);
}

// Function to enqueue a float
void enqueueFloat(int pipefd[2])
{
    float n;
    printf("Enter the float type data\n");
    scanf("%f", &n);
    enqueue(pipefd, &n, sizeof(n));
}

// Function to dequeue a float
void dequeueFloat(int pipefd[2])
{
    float *dequeued = (float *)dequeue(pipefd, sizeof(float));
    printf("Dequeued float: %0.4f\n", *dequeued);
    free(dequeued);
}

// Function to enqueue a string
void enqueueString(int pipefd[2])
{
    char str[MAX_LEN];
    printf("Enter a string: ");
    scanf(" %[^\n]", str); // read until a newline character
    enqueue(pipefd, str, sizeof(str));
}

// Function to dequeue a string
void dequeueString(int pipefd[2])
{
    char *dequeued = (char *)dequeue(pipefd, sizeof(char) * MAX_LEN);
    printf("Dequeued string: %s\n", dequeued);
    free(dequeued);
}

// Function to enqueue a Weather type data
void enqueueWeather(int pipefd[2])
{
    Weather wt;
    printf("Enter the temperature\n");
    scanf("%f", &wt.temperature);
    printf("Enter the place: ");
    scanf(" %[^\n]", wt.place); // read until a newline character
    enqueue(pipefd, &wt, sizeof(wt));
}

// Function to dequeue a Weather type data
void dequeueWeather(int pipefd[2])
{
    Weather *dequeued = (Weather *)dequeue(pipefd, sizeof(Weather));
    printf("Dequeued Weather temperature is : %0.4f\n", dequeued->temperature);
    printf("Dequeued Weather place is : %s\n", dequeued->place);
    free(dequeued);
}

// Demonstrate usage
int main()
{
    int pipefd[2];
    int c = pipe(pipefd); // pipe syscall() giving read and write fd
    if (c == -1)
    {
        perror("pipe() failed\n");
        exit(EXIT_FAILURE);
    }
    int operations;
    printf("Enter the number of operations\n");
    scanf("%d", &operations);

    // operations on various types
    for (int i = 0; i < operations; i++)
    {

        printf("Enter the type of Data to be enqueued:\n 1. Integer \n 2. Double \n 3. Float \n 4. String \n 5. Weather Type Struct\n ");
        int choice;
        scanf("%d", &choice);
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork failed\n");
            exit(EXIT_FAILURE);
        }
        if (pid != 0)
        { // parent does enqueing
            printf("Within parent: Only enqueing\n");
            usleep(200); // Maintaining the strict order of the statements of enqueuing and dequeing

            if (choice == 1)
            {
                enqueueInt(pipefd);
            }
            else if (choice == 2)
            {
                enqueueDouble(pipefd);
            }
            else if (choice == 3)
            {
                enqueueFloat(pipefd);
            }
            else if (choice == 4)
            {
                enqueueString(pipefd);
            }
            else if (choice == 5)
            {
                enqueueWeather(pipefd);
            }
            else
            {
                printf("Wrong choice\n");
            }

            usleep(200); // For maintaining the strict order of dequeue followed by enqueue
        }
        else
        {
            // child does dequeing

            printf("Within Child: Only dequeing\n");
            if (choice == 1)
            {
                dequeueInt(pipefd);
            }
            else if (choice == 2)
            {
                dequeueDouble(pipefd);
            }
            else if (choice == 3)
            {
                dequeueFloat(pipefd);
            }
            else if (choice == 4)
            {
                dequeueString(pipefd);
            }
            else if (choice == 5)
            {
                dequeueWeather(pipefd);
            }
            else
            {
                printf("Wrong choice\n");
            }

            _exit(0); // child flushes all the existing file descriptors asscoiated with it before exiting.
        }
    }

    return 0;
}