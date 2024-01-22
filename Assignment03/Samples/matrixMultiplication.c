#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>



// Function to split the command line arguments into parts of three
char** splitArgumentsIntoParts(int n, char* argv[]) {
    // Calculate the number of substrings needed
    int numSubstrings = n;

    // Allocate memory for array of pointers to char
    char** result = (char**)malloc(numSubstrings * sizeof(char*));

    // Iterate through each argument
    for (int i = 1; i <=n; i++) {
        // Allocate memory for each substring
        result[i - 1] = (char*)malloc(4 * sizeof(char)); // Each part can have 3 digits and a space

        // Copy 3 characters (or less if at the end of the string)
        strncpy(result[i - 1], argv[i], 3);

        // Null-terminate the substring
        result[i - 1][3] = '\0';
    }

    return result;
}

// Function to free memory allocated for the array of strings
void freeStringArray(int numStrings, char** array) {
    for (int i = 0; i < numStrings; i++) {
        free(array[i]);
    }
    free(array);
}

int main(int argc,char* argv[]){
	// argv[]={m,n,r,element of 1st matrix,element of 2nd matrix}
	if(argc<=4){
		fprintf(stderr,"Less arguments!\n");
		exit(0);
	}
	int m=atoi(argv[1]);
	int n=atoi(argv[2]);
	int r=atoi(argv[3]);
	int ptr=4;
	int matrix1[m][n];
	int matrix2[n][r];
	int matrix2_transpose[r][n];
	int result[m*r];
	pid_t pid;
	int status;
	
	char* rows1[m];
	for(int i=0;i<m;i++){
		char *s=argv[]
	}

	for(int i=0;i<m;i++){
		for(int j=0;j<r;j++){
			pid=fork();
			if(pid==0){
				char* argNew[]={"./matrixHelper",argv[2],rows1[i],rows2[j],NULL};
				int f=execve("./matrixHelper",argNew,NULL);
				if(f==-1){
					perror("execve failed:");
				}
			}
			else if(pid==-1){
				perror("fork failed:");
			}
			else{
				printf("Within parent child process created with pid %d\n",pid);
			}
		}
	}
	for(int i=0;i<m*r;i++){
		wait(&status);
		result[i]=WEXITSTATUS(status);
	}
	for(int i=0;i<m*r;i++){
		printf("%d\n",result[i]);
	}
	exit(0);
}
