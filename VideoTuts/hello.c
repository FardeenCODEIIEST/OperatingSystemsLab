#include <stdio.h>
#include <stdlib.h>
int main(int argc,char* argv[]){
	// ./hello N 
	//  argc ---> Total number of command line words---> 2
	//  argv[0]= "./hello"
	//  argv[1]= "N"
	int n=atoi(argv[1]);
	int f=1;
	for(int i=1;i<=n;i++){
		f*=i;
	}
	printf("The factorial is :%d\n",f);
	return 0;
}





