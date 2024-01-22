#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(){
	fork(); // creates a child process
	printf("Hello World\n");
}

