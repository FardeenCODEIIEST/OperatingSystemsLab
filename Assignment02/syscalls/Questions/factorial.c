#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>


/*
 * @params:-(int) n-> number for which factorial is to be calculated
 * @return:- (int) returns the factorial of the number
 * */
int factorial(int n){
	int ans=1;
	for(int i=2;i<=n;i++){
		ans*=i;
	}
	return ans;
}

int main(int argc,char* argv[]){
	int n=atoi(argv[1]); // takes in the command line argument converts it into integer using atoi defined in <stdlib.h>
	pid_t p;
	for(int i=1;i<=n;i++){
		p=fork();
		if(p==0){
			// if p==0 then it is a child process, thus print the factorial
			printf("Child process :- %d, factorial of %d is %d\n",(i),(i),factorial(i)); 
			exit(0);
		}
	}
}

