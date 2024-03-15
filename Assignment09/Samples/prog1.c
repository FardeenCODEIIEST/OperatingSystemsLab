#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

char buf1[] = "LAB ";

char buf2[] = "OS Linux";

int main(void)

{

    int fd;

    if ((fd = creat("file.gol", 0666)) < 0)
    {

        perror("Creation error");

        exit(1);
    }

    if (write(fd, buf1, sizeof(buf1)) < 0)

        perror("Writing error");

    exit(2);

    if (lseek(fd, 4096, SEEK_SET) < 0)

        perror("Positioning error");

    exit(3);

    if (write(fd, buf2, sizeof(buf2)) < 0)

        perror("Writing error");

    exit(2);
}
