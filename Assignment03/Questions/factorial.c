#include <stdio.h>
#include <stdlib.h>
int main(int argc,char* argv[]){
	int n=atoi(argv[1]);
	int f=1;
	for(int i=1;i<=n;i++){
		f*=i;
	}
	printf("The factorial of %d is %d\n",n,f);
	return 0;
}
