#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage:- %s <input file name> <output file name>\n", argv[0]);
		return -1;
	}
	// Open the files
	int inputFd = open(argv[1], O_RDONLY | O_CREAT, 0666);
	if (inputFd == -1)
	{
		perror("open error\n");
		return -1;
	}
	int outputFd = open(argv[2], O_WRONLY | O_CREAT, 0666);
	if (outputFd == -1)
	{
		perror("open error\n");
		return -1;
	}

	// Change the streams
	int newInputFD = dup2(inputFd, STDIN_FILENO);
	if (newInputFD == -1)
	{
		perror("dup2 failed\n");
		return -1;
	}
	int newOutputFD = dup2(outputFd, STDOUT_FILENO);
	if (newOutputFD == -1)
	{
		perror("dup2 failed\n");
		return -1;
	}
	printf("%d %d\n", newInputFD, inputFd);
	printf("%d %d\n", newOutputFD, outputFd);

	printf("What is your name?\n");
	char buff[256];
	scanf("%s", buff);
	printf("OK, %s\n", buff);
	return 0;
}
