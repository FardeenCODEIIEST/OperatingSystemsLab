// prog1.c
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {

    int n=atoi(argv[1]);
    // Parse rowA
    //for (int i = 0; i < atoi(argv[2]); i++) {
    //    rowA[i] = atoi(argv[i + 1]);
    //}

    // Parse colB
    //for (int i = 0; i < atoi(argv[2]); i++) {
    //    colB[i] = atoi(argv[i + 1 + atoi(argv[2])]);
    //}

    int sum=0;
    int r1=2,r2=2+n;
    for(int i=0;i<n;i++){
	int rowEl=atoi(argv[r1++]);
	int colEl=atoi(argv[r2++]);
	sum+=(rowEl*colEl);
    }
    printf("%d\n",sum);
}

