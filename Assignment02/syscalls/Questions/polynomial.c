#include <stdio.h>
#include <stdlib.h>
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
double evaluatePolynomialTerm(int n, double v, double coefficients[]) {
    double result = coefficients[0];
    for (int i = 1; i<=n; i++) {
        result = result * v + coefficients[i];
    }
    return result;
}

// Print each polynomial term
void  printPolynomialTerm(int index, double x, double coefficients[],int n){
    int res=1;
    for (int i = n-index-1; i>=0; i--) {
        res*= x;
    }
    printf("P(%d): Result for term %d = %.0f * %d = %.0f\n",n-index,n-index,coefficients[index],res,coefficients[index]*res);
}
int main(int argc, char *argv[]) {
    double v = atof(argv[1]);	// Value of v
    int n = argc - 3;		// Number of terms
    double totalSum=0;
    double coefficients[n+1];	
    for (int i = 0; i <= n; i++) {
        coefficients[i] = atof(argv[i + 2]);
    }
    printf("Details of Polynomial Chosen: \nDegree: %d\nValue of X: %.0f\n\n",n,v);

    for (int i = 0; i <= n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            printPolynomialTerm(i, v, coefficients,n);
            exit(0);
        } else {
           //wait(NULL);
           sleep(n*0.2);
	    if (i == n) {
               totalSum = evaluatePolynomialTerm(n, v, coefficients);
                printf("Total sum of the polynomial: %.0f\n", totalSum);
            }
        }
    }

    return 0;
}

