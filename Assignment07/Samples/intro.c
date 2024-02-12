/* A simple program that uses a pipe (IPC tool) from within the same process */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

//       void perror(const char *s);

//       int pipe(int pipefd[2]);

int main()
{

    int pipefds[2]; /* to store the file descriptors pipe() returns */
    int flag;       /* to store what system calls (pipe/read/write) return */

    char buf[100];                    /* see later how it is used in read() to store the data to be read */
    int toWrite[5] = {1, 2, 3, 4, 5}; /* see later how it is used in write() for the data to be written */
    int *readMe;                      /* pointer to traverse a chaacter array in terms of integers */

    flag = pipe(pipefds);

    if (flag == -1)
    {
        /* The following commented out part is to demosntrte how perror() makes things easier */
        /*
        fprintf( stderr, "pipe() failed:");
        switch (errno) {
                case EFAULT: fprintf( stderr, "pipefd is not valid.\n");
                        break;

                case EINVAL: fprintf(stderr, "(pipe2()) Invalid value in flags.\n");

                EMFILE The per-process  limit  on  the  number  of  open  file descriptors has been reached.

                ENFILE The system-wide limit on the total number of open files has been reached.
        }
        */

        perror("pipe() failed:");
        exit(1);
    }

    printf("Pipefds: %d %d!\n", pipefds[0], pipefds[1]);

    // flag = pipe(pipefds);

    // ssize_t read(int fd, void *buf, size_t count);

    // size_t write(int fd, const void *buf, size_t count);

    flag = write(pipefds[1], toWrite, sizeof(int) * 5);
    if (flag == -1)
    {

        perror("write() Failed!:");
        exit(2);
    }

    flag = read(pipefds[0], buf, sizeof(int) * 5);
    if (flag == -1)
    {

        perror("read() Failed!:");
        exit(4);
    }

    readMe = (int *)buf;

    printf("%d %d %d\n", readMe[0], readMe[1], readMe[2]);
}