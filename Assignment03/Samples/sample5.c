#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char* argv[]){
	int m=atoi(argv[1]);
	// int m=atoi(argv[2]);
	// int r=atoi(argv[3]);
	int ptr=2;
	int arr[m];
	for(int i=0;i<m;i++){
		arr[i]=atoi(argv[ptr++]);
	}
	char* numbers[m+2];
	numbers[0]=argv[1];
	for(int i=0;i<m;i++){
		numbers[i+1]=(char*)malloc(12);
		sprintf(numbers[i+1],"%d",arr[i]);
	}
	numbers[m+1]=NULL;
	printf("%s\n",numbers[0]);
	//char* argNew[]={argv[1],numbers,NULL};
	execve("./printNumbers",numbers,NULL);
	exit(0);

}
