#include <stdio.h>
#include <stdlib.h>

int main(int argc,char* argv[]){
	int ans=0;
	int n=atoi(argv[1]);
	int A[n],B[n];
	int c=2;
	for(int i=0;i<n;i++){
		A[i]=atoi(argv[c]);
		B[i]=atoi(argv[c+n]);
		ans+=(A[i]*B[i]);
		c++;
	}
//	int i=atoi(argv[c++]);
//	int j=atoi(argv[c]);
//	int res=((ans<<16)|(i<<8)|j);
//	exit(res);
	exit(ans);
}
