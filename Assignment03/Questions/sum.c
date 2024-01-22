#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
	int n=atoi(argv[1]);
	// Sum of natural numbers till n
	int sum=0;
	for(int i=1;i<=n;i++){
		sum+=i;
	}
	printf("The sum of natural numbers till %d is %d\n",n,sum);
	return 0;
}

