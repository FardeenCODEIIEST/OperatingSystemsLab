#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
// #include <sys/wait.h> --> Tried to implement without using wait()

/*
 * We have a coefficients array that that stores the coefficient of the polynomial
 * in order, eg:- coefficients[0]=an-1, coefficients[1]=an-2.... coefficients[n-1]=a0 , n being the degree
 * We will use this array to evaluate the final value of polynomial for a given value v
 *
 */


// Evaluate the value of the polynomial for a given value of v
int evaluatePolynomialTerm(int n, int v, int coefficients[]) {
    int result = coefficients[0];
    for (int i = 1; i<=n; i++) {
        result = result * v + coefficients[i];
    }
    return result;
}

// Print each polynomial term
int  printPolynomialTerm(int index, int x, int coefficients[],int n){
    int res=1;
    for (int i = n-index-1; i>=0; i--) {
        res*= x;
    }
    printf("P(%d): Result for term %d = %d * %d = %d\n",n-index,n-index,coefficients[index],res,coefficients[index]*res);
    return coefficients[index]*res;
}
int main(int argc, char *argv[]) {
    if(argc<3)
    {
    	fprintf(stderr,"Usage:- %s <Value of V> <Terms>\n",argv[0]);
	return -1;
    }
    int v = atoi(argv[1]);	// Value of v
    int n = argc - 3;		// Number of terms
    int totalSum=0;
    int coefficients[n+1];	
    for (int i = 0; i <= n; i++) {
        coefficients[i] = atof(argv[i + 2]);
    }
    printf("Details of Polynomial Chosen: \nDegree: %d\nValue of X: %d\n\n",n,v);
    pid_t pidArray[n+1];
    for (int i = 0; i <= n; i++) {
        pid_t pid = fork();
	if(pid==-1)
	{
		perror("fork error\n");
		return -1;
	}
        if (pid == 0) 
	{
            double res = printPolynomialTerm(i, v, coefficients,n);
            return res;
        } 
	else 
	{
        	pidArray[i]=pid;  
        }
    }
    int status;
    for(int i=0;i<=n;i++)
    {
    	pid_t p = waitpid(pidArray[i],&status,0);
	if(p==-1)
	{
		perror("waitpid error\n");
		return -1;
	}
	printf("Process with pid %d returned successfully with answer %d\n",pidArray[i],status>>8);
    	if (i == n) 
	{
             //totalSum = evaluatePolynomialTerm(n, v, coefficients);
             	totalSum+=(status>>8);
		printf("Total sum of the polynomial: %d\n", totalSum);
        }
	else
	{
		totalSum += (status>>8);
	}
    }


    return 0;
}

