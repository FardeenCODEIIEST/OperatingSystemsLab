#include <stdlib.h>
#include <stdio.h>

int main(int argc,char* argv[]){
	int n=atoi(argv[1]);
	int c=2;
	int numbers[n];
	for(int i=0;i<n;i++){
		numbers[i]=atoi(argv[c++]);
	}
	for(int i=0;i<n;i++){
		printf("%d\n", numbers[i]);
	}
	exit(0);
}
