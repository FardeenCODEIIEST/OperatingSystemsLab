#include <stdio.h>
#include <stdlib.h>
int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		fprintf(stderr,"Usage:- %s [num1] [num2]\n",argv[0]);
		return -1;
	}
	int n1=atoi(argv[1]);
	int n2=atoi(argv[2]);

	printf("The sum of %d and %d is %d\n",n1,n2,(n1+n2));

	return 0;
}
