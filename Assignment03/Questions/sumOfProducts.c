#include <stdio.h>	// for printf() ..
#include <stdlib.h>	// for atoi() ..

int main(int argc,char* argv[]){
	int n=atoi(argv[1]);
	int ans=0;
	int c1=2,c2=n+2;
	for(int i=0;i<n;i++){
		int element01=atoi(argv[c1++]);
		int element02=atoi(argv[c2++]);
		ans+=element01*element02;
	}
	printf("%d\n",ans);
	return 0;
}
